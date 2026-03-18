/**
    NVT PCIE edma 

    @file nvt_pcie_edma.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/of.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/semaphore.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>

#include "nvt_pcie_reg.h"
#include "pcie-nvt-edma-internal.h"

#define TIMEOUT 180000000 // Timeout if no response for more than 3 minutes
struct nvt_dma *nvt_dma_ptr = NULL;

static void nvt_dma_read_tasklet(unsigned long data)
{
	//struct nvt_dma *nvt_dma_ptr = (struct nvt_dma *)data;
	u32 chan;
	u32 status, event;
	unsigned long flags;
	u64 curr_t, after_t;

	spin_lock_irqsave(&nvt_dma_ptr->lock, flags);

	/* DMA_READ_INT_STATUS_OFF, [7:0]RD_DONE_INT_STATUS, [23:16]RD_ABORT_INT_STATUS */
	status = readl(nvt_dma_ptr->dma_va + DMA_READ_INT_STATUS_OFF_OFS);
	/* clear status */
	writel(status, nvt_dma_ptr->dma_va + DMA_READ_INT_CLEAR_OFF_OFS);

	if (status) {
		for (chan = 0; chan < nvt_dma_ptr->max_chan; chan ++) {
			event = 0;
			if (status & (0x1 << chan)) {
				event = DMA_FINISH;
			}
			if ((status >> 16) & (0x1 << chan)) {
				event = DMA_ABORT;
			}

			if (event) {
				nvt_dma_ptr->channel[chan].status = event;
				wake_up(&nvt_dma_ptr->wait_queue[chan]);
				if (nvt_dma_ptr->channel[chan].callback) {
					curr_t = ktime_get_real_ns();
					nvt_dma_ptr->channel[chan].callback(chan, nvt_dma_ptr->channel[chan].status);
					after_t = ktime_get_real_ns();
					if ((after_t - curr_t) > CALLBACK_MAX_TIME) {
						dev_err(nvt_dma_ptr->dev, "Callback func:[%pF] is taking too much time(%lld us)\n", nvt_dma_ptr->channel[chan].callback, (after_t - curr_t) / 1000);
					}
				}
			}
		}
	}

	/* Disable eDMA IRQ Mask */
	writel(0, nvt_dma_ptr->dma_va + DMA_READ_INT_MASK_OFF_OFS);

	spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);
}

static void nvt_dma_write_tasklet(unsigned long data)
{
	//struct nvt_dma *nvt_dma_ptr = (struct nvt_dma *)data;
	u32 chan;
	u32 status, event;
	unsigned long flags;
	u64 curr_t, after_t;

	spin_lock_irqsave(&nvt_dma_ptr->lock, flags);

	/* DMA_WRITE_INT_STATUS_OFF, [7:0]RD_DONE_INT_STATUS, [23:16]RD_ABORT_INT_STATUS */
	status = readl(nvt_dma_ptr->dma_va + DMA_WRITE_INT_STATUS_OFF_OFS);
	/* clear status */
	writel(status, nvt_dma_ptr->dma_va + DMA_WRITE_INT_CLEAR_OFF_OFS);

	if (status) {
		for (chan = 0; chan < nvt_dma_ptr->max_chan; chan ++) {
			event = 0;
			if (status & (0x1 << chan)) {
				event = DMA_FINISH;
			}
			if ((status >> 16) & (0x1 << chan)) {
				event = DMA_ABORT;
			}

			if (event) {
				nvt_dma_ptr->channel[chan].status = event;
				wake_up(&nvt_dma_ptr->wait_queue[chan]);
				if (nvt_dma_ptr->channel[chan].callback) {
					curr_t = ktime_get_real_ns();
					nvt_dma_ptr->channel[chan].callback(chan, nvt_dma_ptr->channel[chan].status);
					after_t = ktime_get_real_ns();
					if ((after_t - curr_t) > CALLBACK_MAX_TIME) {
						dev_err(nvt_dma_ptr->dev, "Callback func:[%pF] is taking too much time(%lld us)\n", nvt_dma_ptr->channel[chan].callback, (after_t - curr_t) / 1000);
					}
				}
			}
		}
	}

	/* Disable eDMA IRQ Mask */
	writel(0, nvt_dma_ptr->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);

	spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);
}

irqreturn_t nvt_pcie_drv_isr(int irq, void *data)
{
	unsigned long flags;
	u32 status;
	bool match = false;

	status = readl(nvt_dma_ptr->dma_va + DMA_READ_INT_STATUS_OFF_OFS);

	if (status) {
		match = true;
		/*
		 * Disable the interrupts. 
		 * We'll turn them back on in the softirq handler.
		 */
		spin_lock_irqsave(&nvt_dma_ptr->lock, flags);

		writel(~0, nvt_dma_ptr->dma_va + DMA_READ_INT_MASK_OFF_OFS);

		spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);

		tasklet_schedule(&nvt_dma_ptr->read_tasklet);
	}

	status = readl(nvt_dma_ptr->dma_va + DMA_WRITE_INT_STATUS_OFF_OFS);
	if (status) {
		match = true;
		/*
		 * Disable the interrupts. 
		 * We'll turn them back on in the softirq handler.
		 */
		spin_lock_irqsave(&nvt_dma_ptr->lock, flags);

		writel(~0, nvt_dma_ptr->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);

		spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);

		tasklet_schedule(&nvt_dma_ptr->write_tasklet);
	}

	if (match)
		return IRQ_HANDLED;

	return IRQ_NONE;
}
EXPORT_SYMBOL(nvt_pcie_drv_isr);

int nvt_alloc_edmac_chan(void)
{
	int chan;
	unsigned long flags;

	spin_lock_irqsave(&nvt_dma_ptr->lock, flags);

	for (chan = 0; chan < nvt_dma_ptr->max_chan; chan ++) {
		if (nvt_dma_ptr->in_use & (1 << chan))
			continue;

		nvt_dma_ptr->in_use |= 1 << chan;
		memset(&nvt_dma_ptr->channel[chan], 0x0,  sizeof(struct dmac_info));

		spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);

		return chan;
	}

	spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);

	dev_err(nvt_dma_ptr->dev, "All eDMA channels are busy\n");

	return -EBUSY;
}
EXPORT_SYMBOL(nvt_alloc_edmac_chan);

void nvt_free_edmac_chan(int chan)
{
	unsigned long flags;

	spin_lock_irqsave(&nvt_dma_ptr->lock, flags);

	if (nvt_dma_ptr->channel[chan].status != DMA_IN_PROCESS) {
		nvt_dma_ptr->in_use &= ~(1 << chan);
	} else {
		dev_info(nvt_dma_ptr->dev, "eDMA channel[%d] is busy, force abort it\n", chan);
		//FIXME ABORT
	}

	spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);
}
EXPORT_SYMBOL(nvt_free_edmac_chan);

int nvt_pcie_config_edma_chan(int chan, unsigned char max_packet)
{
	unsigned long flags;
	T_DMA_WRITE_CHAN_WEIGHT_LOW_OFF write_weight_low;
	T_DMA_WRITE_CHAN_WEIGHT_HIGH_OFF write_weight_high;
	T_DMA_READ_CHAN_WEIGHT_LOW_OFF read_weight_low;
	T_DMA_READ_CHAN_WEIGHT_HIGH_OFF read_weight_high;
	u64 packet_size = max_packet << (chan * 5);

	if (max_packet > 0x1F || max_packet == 0) {
		dev_err(nvt_dma_ptr->dev, "Incorrect packet size, should be [1~31]\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&nvt_dma_ptr->lock, flags);

	if (nvt_dma_ptr->channel[chan].status == DMA_IN_PROCESS) {
		spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);
		dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] in process, config fail\n", chan);
		return -EBUSY;
	}

	write_weight_low.reg = packet_size & 0xFFFFFFFF;
	writel(write_weight_low.reg, nvt_dma_ptr->dma_va + DMA_WRITE_CHAN_WEIGHT_LOW_OFF_OFS);

	write_weight_high.reg = packet_size >> 32;
	writel(write_weight_high.reg, nvt_dma_ptr->dma_va + DMA_WRITE_CHAN_WEIGHT_HIGH_OFF_OFS);

	read_weight_low.reg = packet_size & 0xFFFFFFFF;
	writel(read_weight_low.reg, nvt_dma_ptr->dma_va + DMA_READ_CHAN_WEIGHT_LOW_OFF_OFS);

	read_weight_high.reg = packet_size >> 32;
	writel(read_weight_high.reg, nvt_dma_ptr->dma_va + DMA_READ_CHAN_WEIGHT_HIGH_OFF_OFS);

	spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);
	return 0;
}
EXPORT_SYMBOL(nvt_pcie_config_edma_chan);

/* Read Transfer On Read Channel#N. Initated by Local CPU(Non LL mode)
 * src_paddr or dst_paddr is through pcie address translation already and 16 bytes alignment.
 * len: bytes. len is 16 bytes alignment
 */
int nvt_pcie_edma_read(int chan, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len, dma_callback callback, bool blocking)
{
	int ret = 0;
	T_DMA_READ_ENGINE_EN_OFF read_engine_en = {0};
	T_DMA_CH_CONTROL1_OFF_RDCH ctrl1;
	T_DMA_READ_DOORBELL_OFF doorbell = {0};
	T_DMA_LLP_LOW_OFF_RDCH llp_low;
	T_DMA_LLP_HIGH_OFF_RDCH llp_high;
	unsigned long flags;
	dma_addr_t handle;
	struct nvt_desc_list *desc_list;
	enum DMA_STATUS status;

	if (nvt_dma_ptr->channel[chan].status == DMA_IN_PROCESS) {
		dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] is busy !!!\n", chan);
		ret = -EBUSY;
		status = DMA_BUSY;
		goto read_fail;
	}

	desc_list = dma_pool_zalloc(nvt_dma_ptr->dma_pool, GFP_KERNEL, &handle);
	if (!desc_list) {
		dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] alloc dma memory fail!!!\n", chan);
		ret = -ENOMEM;
		status = ALLOC_POOL_FAIL;
		goto read_fail;
	}

	spin_lock_irqsave(&nvt_dma_ptr->lock, flags);

	nvt_dma_ptr->channel[chan].callback = callback;

	// DMA Read Engine Enable
	read_engine_en.bit.DMA_READ_ENGINE = 1;
	writel(read_engine_en.reg, nvt_dma_ptr->dma_va + DMA_READ_ENGINE_EN_OFF_OFS);

	/* Setup Descriptor */
	ctrl1.reg = 0;
	ctrl1.bit.CB = 1;
	ctrl1.bit.LIE = 1;
	desc_list->desc[0].element.len = len;
	desc_list->desc[0].element.src_paddr = src_paddr;
	desc_list->desc[0].element.dst_paddr = dst_paddr;
	desc_list->desc[0].element.chan_ctrl = ctrl1.reg;

	/* Setup End Element */
	ctrl1.reg = 0;
	ctrl1.bit.TCB = 1;
	ctrl1.bit.LLP = 1;
	desc_list->desc[1].enddesc.chan_ctrl = ctrl1.reg;
	desc_list->desc[1].enddesc.llp = handle;

	/* Setup eDMA Channel Status */
	ctrl1.reg = 0;
	ctrl1.bit.CCS = 1;
	ctrl1.bit.LLE = 1;
	writel(ctrl1.reg, nvt_dma_ptr->dma_va + DMA_CH_CONTROL1_OFF_RDCH_OFS(chan));

	llp_low.bit.LLP_ADDR_REG_LOW = handle & 0xFFFFFFFF;
	writel(llp_low.reg, nvt_dma_ptr->dma_va + DMA_LLP_LOW_OFF_RDCH_OFS(chan));
	llp_high.bit.LLP_ADDR_REG_HIGH = handle >> 32;
	writel(llp_high.reg, nvt_dma_ptr->dma_va + DMA_LLP_HIGH_OFF_RDCH_OFS(chan));

	nvt_dma_ptr->channel[chan].status = DMA_IN_PROCESS;

	// DMA Read Doorbell
	doorbell.bit.RD_DOORBELL_NUM = chan;
	writel(doorbell.reg, nvt_dma_ptr->dma_va + DMA_READ_DOORBELL_OFF_OFS);

	spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);

	if (blocking) {
		if (!wait_event_timeout(nvt_dma_ptr->wait_queue[chan], nvt_dma_ptr->channel[chan].status != DMA_IN_PROCESS, TIMEOUT)) {
			dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] write timeout !!!\n", chan);
			ret = -ETIMEDOUT;
			status = DMA_TIMEOUT;
			if (callback)
				callback(chan, status);
		}
		if (nvt_dma_ptr->channel[chan].status == DMA_ABORT) {
			dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] read abort !!!\n", chan);
			ret = -ECONNABORTED;
		}

		if (nvt_dma_ptr->channel[chan].status == DMA_ERR) {
			dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] read error !!!\n", chan);
			ret = -EIO;
		}
	}

	dma_pool_free(nvt_dma_ptr->dma_pool, desc_list, handle);

	return ret;

read_fail:
	if (callback)
		callback(chan, status);
	return ret;
}

EXPORT_SYMBOL(nvt_pcie_edma_read);

/* Write Transfer On Write Channel#N. Initated by Local CPU(Non LL mode)
 * src_paddr or dst_paddr is through pcie address translation already and 16 bytes alignment.
 * len: bytes. len is 16 bytes alignment
 */
int nvt_pcie_edma_write(int chan, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len, dma_callback callback, bool blocking)
{
	int ret = 0;
	T_DMA_WRITE_ENGINE_EN_OFF write_engine_en = {0};
	T_DMA_WRITE_DOORBELL_OFF doorbell = {0};
	T_DMA_CH_CONTROL1_OFF_WRCH ctrl1;
	T_DMA_LLP_LOW_OFF_WRCH llp_low;
	T_DMA_LLP_HIGH_OFF_WRCH llp_high;
	unsigned long flags;
	dma_addr_t handle;
	struct nvt_desc_list *desc_list;
	enum DMA_STATUS status;

	if (nvt_dma_ptr->channel[chan].status == DMA_IN_PROCESS) {
		dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] is busy !!!\n", chan);
		ret = -EBUSY;
		status = DMA_BUSY;
		goto write_fail;
	}

	desc_list = dma_pool_zalloc(nvt_dma_ptr->dma_pool, GFP_KERNEL, &handle);
	if (!desc_list) {
		dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] alloc dma memory fail!!!\n", chan);
		ret = -ENOMEM;
		status = ALLOC_POOL_FAIL;
		goto write_fail;
	}

	spin_lock_irqsave(&nvt_dma_ptr->lock, flags);

	nvt_dma_ptr->channel[chan].callback = callback;

	/* Setup Descriptor */
	ctrl1.reg = 0;
	ctrl1.bit.CB = 1;
	ctrl1.bit.LIE = 1;
	desc_list->desc[0].element.len = len;
	desc_list->desc[0].element.src_paddr = src_paddr;
	desc_list->desc[0].element.dst_paddr = dst_paddr;
	desc_list->desc[0].element.chan_ctrl = ctrl1.reg;

	/* Setup End Element */
	ctrl1.reg = 0;
	ctrl1.bit.TCB = 1;
	ctrl1.bit.LLP = 1;
	desc_list->desc[1].enddesc.chan_ctrl = ctrl1.reg;
	desc_list->desc[1].enddesc.llp = handle;

	ctrl1.reg = 0;
	ctrl1.bit.CCS = 1;
	ctrl1.bit.LLE = 1;
	writel(ctrl1.reg, nvt_dma_ptr->dma_va + DMA_CH_CONTROL1_OFF_WRCH_OFS(chan));
	llp_low.bit.LLP_ADDR_REG_LOW = handle & 0xFFFFFFFF;
	writel(llp_low.reg, nvt_dma_ptr->dma_va + DMA_LLP_LOW_OFF_WRCH_OFS(chan));
	llp_high.bit.LLP_ADDR_REG_HIGH = handle >> 32;
	writel(llp_high.reg, nvt_dma_ptr->dma_va + DMA_LLP_HIGH_OFF_WRCH_OFS(chan));

	nvt_dma_ptr->channel[chan].status = DMA_IN_PROCESS;

	// DMA Write Engine Enable
	write_engine_en.bit.DMA_WRITE_ENGINE = 1;
	writel(write_engine_en.reg, nvt_dma_ptr->dma_va + DMA_WRITE_ENGINE_EN_OFF_OFS);

	// DMA Write Doorbell
	doorbell.bit.WR_DOORBELL_NUM = chan;
	writel(doorbell.reg, nvt_dma_ptr->dma_va + DMA_WRITE_DOORBELL_OFF_OFS);

	spin_unlock_irqrestore(&nvt_dma_ptr->lock, flags);

	if (blocking) {
		if (!wait_event_timeout(nvt_dma_ptr->wait_queue[chan], nvt_dma_ptr->channel[chan].status != DMA_IN_PROCESS, TIMEOUT)) {
			dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] write timeout !!!\n", chan);
			ret = -ETIMEDOUT;
			status = DMA_TIMEOUT;
			if (callback)
				callback(chan, status);
		}

		if (nvt_dma_ptr->channel[chan].status == DMA_ABORT) {
			dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] write abort !!!\n", chan);
			ret = -ECONNABORTED;
		}

		if (nvt_dma_ptr->channel[chan].status == DMA_ERR) {
			dev_err(nvt_dma_ptr->dev, "eDMA channel[%d] write error !!!\n", chan);
			ret = -EIO;
		}
	}

	dma_pool_free(nvt_dma_ptr->dma_pool, desc_list, handle);

	return ret;

write_fail:
	if (callback)
		callback(chan, status);
	return ret;
}
EXPORT_SYMBOL(nvt_pcie_edma_write);

static int nvt_edma_probe(struct platform_device *pdev)
{
	u32 reg;
	int chan, ret;
	struct resource *res = NULL;

	pr_info("%s: start \n", __func__);

	nvt_dma_ptr = devm_kzalloc(&pdev->dev, sizeof(struct nvt_dma), GFP_KERNEL);
	if (!nvt_dma_ptr) {
		pr_err("Alloc memory for nvt_dma fail\n");
		ret = -ENOMEM;
		goto out_free;
	}

	nvt_dma_ptr->dev = &pdev->dev;

	if (device_property_read_u32(&pdev->dev, "dma-channels", &nvt_dma_ptr->max_chan)) {
		dev_err(nvt_dma_ptr->dev, "Can not find dma-channels on device tree\n");
		ret = -EINVAL;
		goto out_free;
	}

	nvt_dma_ptr->channel = devm_kcalloc(&pdev->dev, nvt_dma_ptr->max_chan, sizeof(struct dmac_info), GFP_KERNEL);
	if (!nvt_dma_ptr->channel) {
		dev_err(nvt_dma_ptr->dev, "Alloc memory for edma channel fail\n");
		ret = -ENOMEM;
		goto out_free;
	}

	nvt_dma_ptr->wait_queue = devm_kcalloc(&pdev->dev, nvt_dma_ptr->max_chan, sizeof(wait_queue_head_t), GFP_KERNEL);
	if (!nvt_dma_ptr->wait_queue) {
		dev_err(nvt_dma_ptr->dev, "Alloc memory for edma wait_queue fail\n");
		ret = -ENOMEM;
		goto out_free;
	}

	nvt_dma_ptr->dma_pool = dma_pool_create("edma", &pdev->dev, sizeof(struct nvt_desc_list), 0, 0);
	if (!nvt_dma_ptr->dma_pool) {
		dev_err(nvt_dma_ptr->dev, "Alloc memory for dma pool fail\n");
		ret = -ENOMEM;
		goto out_free;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pci");
	if (unlikely(!res)) {
		dev_err(nvt_dma_ptr->dev, "platform_get_resource fail on pci\n");
		ret = -ENXIO;
		goto out_free;
	}

	nvt_dma_ptr->pci_va = ioremap(res->start, resource_size(res));
	if (unlikely(nvt_dma_ptr->pci_va == 0)) {
		dev_err(nvt_dma_ptr->dev, "ioremap fail on pci\n");
		ret = -EIO;
		goto remap_err;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dma");
	if (unlikely(!res)) {
		dev_err(nvt_dma_ptr->dev, "platform_get_resource fail on dma\n");
		ret = -ENXIO;
		goto remap_err;
	}

	nvt_dma_ptr->dma_va = ioremap(res->start, resource_size(res));
	if (unlikely(nvt_dma_ptr->dma_va == 0)) {
		dev_err(nvt_dma_ptr->dev, "ioremap fail on dma\n");
		ret = -EIO;
		goto remap_err;
	}

	spin_lock_init(&nvt_dma_ptr->lock);

	for (chan = 0; chan < nvt_dma_ptr->max_chan; chan ++) {
		init_waitqueue_head(&nvt_dma_ptr->wait_queue[chan]);
		nvt_dma_ptr->channel[chan].status = DMA_IDLE;
	}

	/* Enable eDMA IRQ */
	reg = readl(nvt_dma_ptr->pci_va + 0x24);
	writel(reg & ~BIT(22), nvt_dma_ptr->pci_va + 0x24);

	nvt_dma_ptr->irq = platform_get_irq(pdev, 0);
	if (nvt_dma_ptr->irq > 0) {
		if (request_irq(nvt_dma_ptr->irq, nvt_pcie_drv_isr, IRQF_TRIGGER_HIGH, "pci_dma_irq", nvt_dma_ptr)) {
			dev_err(nvt_dma_ptr->dev, "request irq fail\n");
			ret = -ENODEV;
			goto remap_err;
		}
	}

	/* Disable eDMA IRQ Mask */
	writel(0, nvt_dma_ptr->dma_va + DMA_READ_INT_MASK_OFF_OFS);
	writel(0, nvt_dma_ptr->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);

	platform_set_drvdata(pdev, nvt_dma_ptr);

	tasklet_init(&nvt_dma_ptr->read_tasklet, nvt_dma_read_tasklet, (unsigned long)nvt_dma_ptr);
	tasklet_init(&nvt_dma_ptr->write_tasklet, nvt_dma_write_tasklet, (unsigned long)nvt_dma_ptr);

	dev_info(&pdev->dev, "Register %s successfully\n", __func__);
	return 0;

remap_err:
	if (nvt_dma_ptr->dma_va)
		iounmap(nvt_dma_ptr->dma_va);
	if (nvt_dma_ptr->pci_va)
		iounmap(nvt_dma_ptr->pci_va);
out_free:
	if (nvt_dma_ptr->dma_pool)
		dma_pool_destroy(nvt_dma_ptr->dma_pool);
	if (nvt_dma_ptr->channel)
		kfree(nvt_dma_ptr->channel);
	if (nvt_dma_ptr)
		kfree(nvt_dma_ptr);
	return ret;
}

static int nvt_edma_remove(struct platform_device *pdev)
{
	struct nvt_dma *nvt_dma_ptr = platform_get_drvdata(pdev);
	int chan;

	platform_set_drvdata(pdev, NULL);

	tasklet_kill(&nvt_dma_ptr->read_tasklet);
	tasklet_kill(&nvt_dma_ptr->write_tasklet);

	if (nvt_dma_ptr->irq)
		free_irq(nvt_dma_ptr->irq, NULL);

	if (nvt_dma_ptr->dma_va)
		iounmap(nvt_dma_ptr->dma_va);

	if (nvt_dma_ptr->pci_va)
		iounmap(nvt_dma_ptr->pci_va);

	if (nvt_dma_ptr->dma_pool)
		dma_pool_destroy(nvt_dma_ptr->dma_pool);

	if (nvt_dma_ptr->channel) {
		for (chan = 0; chan < nvt_dma_ptr->max_chan; chan ++) {
			kfree(nvt_dma_ptr->channel);
		}
	}

	if (nvt_dma_ptr)
		kfree(nvt_dma_ptr);

	dev_info(&pdev->dev, "Remove %s successfully\n", __func__);
	return 0;
}

static const struct of_device_id nvt_edma_match[] = {
	{
		.compatible = "nvt,nvt_pci_dma",
	},
	{ }
};
MODULE_DEVICE_TABLE(of, nvt_edma_match);

static struct platform_driver nvt_pcie_drv = {
	.probe          = nvt_edma_probe,
	.remove         = nvt_edma_remove,
	.driver         = {
		.name   = "nvt_pci_dma",
		.owner  = THIS_MODULE,
		.of_match_table = nvt_edma_match,
	},
};

static int __init nvt_edma_init(void)
{
	return platform_driver_register(&nvt_pcie_drv);
}

static void __exit nvt_edma_exit(void)
{
	platform_driver_unregister(&nvt_pcie_drv);
}

module_init(nvt_edma_init);
module_exit(nvt_edma_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Novatek Microelectronics Corp.");
MODULE_DESCRIPTION("NVT PCI DMA driver for nvt SOC");
MODULE_VERSION(DRV_VER);

