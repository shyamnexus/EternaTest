/**
    NVT PCIE edma 

    @file nvt_edma.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2023. All rights reserved.

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
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/dmaengine.h>
#include <linux/of_dma.h>
#include <linux/io.h>
#include <linux/delay.h>
#include "dmaengine.h"

#define DBI_DMA_BASE			0x0
#define DMA_MAX_SIZE			0xFFFFFFFF
#define DMA_LINKED_LIST_ERR_MASK	0xFF00FF

#define DMA_ENABLE			0x1
#define DMA_DISABLE			0x0

#define DMA_STOP			0x60

#define PCIE_IRQ_EN_OFF_OFS		(0x24)
#define PCIE_EDMA_IRQ_OFS		(22)

#define DMA_WRITE_ENGINE_EN_OFF_OFS             (DBI_DMA_BASE + 0x0C)
#define DMA_WRITE_DOORBELL_OFF_OFS              (DBI_DMA_BASE + 0x10)
#define DMA_WRITE_CHAN_WEIGHT_LOW_OFF_OFS       (DBI_DMA_BASE + 0x18)
#define DMA_WRITE_CHAN_WEIGHT_HIGH_OFF_OFS      (DBI_DMA_BASE + 0x1C)
#define DMA_READ_ENGINE_EN_OFF_OFS              (DBI_DMA_BASE + 0x2C)
#define DMA_READ_DOORBELL_OFF_OFS               (DBI_DMA_BASE + 0x30)
#define DMA_READ_CHAN_WEIGHT_LOW_OFF_OFS        (DBI_DMA_BASE + 0x38)
#define DMA_READ_CHAN_WEIGHT_HIGH_OFF_OFS       (DBI_DMA_BASE + 0x3C)
#define DMA_WRITE_INT_STATUS_OFF_OFS            (DBI_DMA_BASE + 0x4C)
#define DMA_WRITE_INT_MASK_OFF_OFS              (DBI_DMA_BASE + 0x54)
#define DMA_WRITE_INT_CLEAR_OFF_OFS             (DBI_DMA_BASE + 0x58)
#define DMA_WRITE_ERR_STATUS_OFF_OFS            (DBI_DMA_BASE + 0x5C)
#define DMA_WRITE_DONE_IMWR_LOW_OFF_OFS         (DBI_DMA_BASE + 0x60)
#define DMA_WRITE_DONE_IMWR_HIGH_OFF_OFS        (DBI_DMA_BASE + 0x64)
#define DMA_WRITE_CH01_IMWR_DATA_OFF_OFS        (DBI_DMA_BASE + 0x70)
#define DMA_WRITE_CH23_IMWR_DATA_OFF_OFS        (DBI_DMA_BASE + 0x74)
#define DMA_WRITE_CH45_IMWR_DATA_OFF_OFS        (DBI_DMA_BASE + 0x78)
#define DMA_WRITE_CH67_IMWR_DATA_OFF_OFS        (DBI_DMA_BASE + 0x7C)
#define DMA_WRITE_LINKED_LIST_ERR_EN_OFF_OFS	(DBI_DMA_BASE + 0x90)
#define DMA_READ_INT_STATUS_OFF_OFS             (DBI_DMA_BASE + 0xA0)
#define DMA_READ_INT_MASK_OFF_OFS               (DBI_DMA_BASE + 0xA8)
#define DMA_READ_INT_CLEAR_OFF_OFS              (DBI_DMA_BASE + 0xAC)
#define DMA_READ_ERR_STATUS_LOW_OFF_OFS         (DBI_DMA_BASE + 0xB4)
#define DMA_READ_ERR_STATUS_HIGH_OFF_OFS        (DBI_DMA_BASE + 0xB8)
#define DMA_READ_LINKED_LIST_ERR_EN_OFF_OFS	(DBI_DMA_BASE + 0xC4)
#define DMA_CH_CONTROL1_OFF_WRCH_OFS(i)         (DBI_DMA_BASE + 0x200 + (i)*0x200)
#define DMA_TRANSFER_SIZE_OFF_WRCH_OFS(i)       (DBI_DMA_BASE + 0x208 + (i)*0x200)
#define DMA_SAR_LOW_OFF_WRCH_OFS(i)             (DBI_DMA_BASE + 0x20C + (i)*0x200)
#define DMA_SAR_HIGH_OFF_WRCH_OFS(i)            (DBI_DMA_BASE + 0x210 + (i)*0x200)
#define DMA_DAR_LOW_OFF_WRCH_OFS(i)             (DBI_DMA_BASE + 0x214 + (i)*0x200)
#define DMA_DAR_HIGH_OFF_WRCH_OFS(i)            (DBI_DMA_BASE + 0x218 + (i)*0x200)
#define DMA_LLP_LOW_OFF_WRCH_OFS(i)             (DBI_DMA_BASE + 0x21c + (i)*0x200)
#define DMA_LLP_HIGH_OFF_WRCH_OFS(i)            (DBI_DMA_BASE + 0x220 + (i)*0x200)
#define DMA_CH_CONTROL1_OFF_RDCH_OFS(i)         (DBI_DMA_BASE + 0x300 + (i)*0x200)
#define DMA_TRANSFER_SIZE_OFF_RDCH_OFS(i)       (DBI_DMA_BASE + 0x308 + (i)*0x200)
#define DMA_SAR_LOW_OFF_RDCH_OFS(i)             (DBI_DMA_BASE + 0x30C + (i)*0x200)
#define DMA_SAR_HIGH_OFF_RDCH_OFS(i)            (DBI_DMA_BASE + 0x310 + (i)*0x200)
#define DMA_DAR_LOW_OFF_RDCH_OFS(i)             (DBI_DMA_BASE + 0x314 + (i)*0x200)
#define DMA_DAR_HIGH_OFF_RDCH_OFS(i)            (DBI_DMA_BASE + 0x318 + (i)*0x200)
#define DMA_LLP_LOW_OFF_RDCH_OFS(i)             (DBI_DMA_BASE + 0x31c + (i)*0x200)
#define DMA_LLP_HIGH_OFF_RDCH_OFS(i)            (DBI_DMA_BASE + 0x320 + (i)*0x200)

#define DRV_VER "0.0.3"

enum DMA_CTRL_MODE {
	CB = 0,
	TCB,
	LLP,
	LIE,
	RIE,
	CS,
	DMA_RESERVED0 = 7,
	CCS,
	LLE,
};

enum DMA_ERR_MODE {
	APP_WR_ERR                      = 0,
	LINK_LIST_ELEMENT_FETCH_ERR     = 16,
	UNSUPPORTED_REQ                 = 32,
	CPL_ABORT                       = 40,
	CPL_TIMEOUT                     = 48,
	DATA_POISIONING                 = 56,
};

#pragma pack(push, 1)
struct nvt_edma_descriptor {
	u32     chan_ctrl;
	u32     len;
	u64     src;
	u64     dest;
	u32     llp_ctrl;
	u32     reserve;
	u64     llp;
};

struct nvt_edma_desc {
	struct nvt_edma_descriptor		element;
	dma_addr_t                              phys;
	struct list_head                        desc_node;
	struct list_head                        tx_list;
	struct dma_descriptor_metadata_ops	ops;
	struct dma_async_tx_descriptor          *tx_dma_desc;
} __attribute__((aligned(L1_CACHE_BYTES)));
#pragma pack(pop)

struct nvt_edma_chan {
	struct tasklet_struct           tasklet;
	struct dma_chan                 chan;
	struct dma_slave_config         sconfig;
	struct nvt_edma_desc		*read_desc;
	struct nvt_edma_desc		*write_desc;
	struct list_head                xfers_list;
	struct list_head                queue;
	spinlock_t                      lock;
	unsigned int                    maxburst;
	unsigned int                    ch_id;
	unsigned int                    status;
	enum dma_transfer_direction	direction;
	bool                            active;
	bool                            read_pcs;
	bool                            write_pcs;
};

struct nvt_edma {
	struct dma_device       dma;
	spinlock_t              lock;
	void __iomem            *pci_va;
	void __iomem            *dma_va;
	struct nvt_edma_chan    *chan;
	unsigned int            max_chan;
	int                     irq;
};

struct nvt_edma *g_nvt_edma = NULL;

static inline struct nvt_edma_chan *to_nvt_edma_chan(struct dma_chan *dchan)
{
	return container_of(dchan, struct nvt_edma_chan, chan);
}

static inline struct nvt_edma *to_nvt_edma(struct dma_device *ddev)
{
	return container_of(ddev, struct nvt_edma, dma);
}

static inline struct nvt_edma_desc *tx_dma_desc_to_at_desc(struct dma_descriptor_metadata_ops *metadata_ops)
{
	return container_of(metadata_ops, struct nvt_edma_desc, ops);
}

static struct device *chan2dev(struct dma_chan *chan)
{
	return &chan->dev->device;
}

static void nvt_desc_put(struct nvt_edma_chan *nvtchan, struct nvt_edma_desc *desc)
{
	struct nvt_edma      *nvtdmac = to_nvt_edma(nvtchan->chan.device);
	struct nvt_edma_desc *child, *_next;

	dev_dbg(nvtdmac->dma.dev, "%s\n", __func__);
	if (unlikely(!desc))
		return;

	list_for_each_entry_safe(child, _next, &desc->tx_list, desc_node) {
		list_del(&child->desc_node);
		if (child->tx_dma_desc)
			kfree(child->tx_dma_desc);
		dma_free_coherent(nvtdmac->dma.dev, sizeof(struct nvt_edma_desc), child, child->phys);
	}

	if (desc->tx_dma_desc)
		kfree(desc->tx_dma_desc);
	dma_free_coherent(nvtdmac->dma.dev, sizeof(struct nvt_edma_desc), desc, desc->phys);
}


static void nvt_handle_err(struct nvt_edma_chan *nvtchan)
{
	u32 status = 0;
	struct nvt_edma *nvtdmac = to_nvt_edma(nvtchan->chan.device);

	dev_dbg(nvtdmac->dma.dev, "%s\n", __func__);
	if (nvtchan->direction == DMA_MEM_TO_DEV) {
		status = readl(nvtdmac->dma_va + DMA_WRITE_ERR_STATUS_OFF_OFS);
		if (status & 1 << (nvtchan->ch_id + APP_WR_ERR)) {
			dev_err(nvtdmac->dma.dev, "Application Read Error on eDMA channel[%d]\n", nvtchan->ch_id);
		}
		if (status & 1 << (nvtchan->ch_id + LINK_LIST_ELEMENT_FETCH_ERR)) {
			dev_err(nvtdmac->dma.dev, "Linked List Element Fetch Error on eDMA channel[%d]\n", nvtchan->ch_id);
		}
	} else {
		status = readl(nvtdmac->dma_va + DMA_READ_ERR_STATUS_LOW_OFF_OFS);
		if (status & 1 << (nvtchan->ch_id + APP_WR_ERR)) {
			dev_err(nvtdmac->dma.dev, "Application Read Error on eDMA channel[%d]\n", nvtchan->ch_id);
		}
		if (status & 1 << (nvtchan->ch_id + LINK_LIST_ELEMENT_FETCH_ERR)) {
			dev_err(nvtdmac->dma.dev, "Linked List Element Fetch Error on eDMA channel[%d]\n", nvtchan->ch_id);
		}

		status = readl(nvtdmac->dma_va + DMA_READ_ERR_STATUS_HIGH_OFF_OFS);
		if (status & 1 << (nvtchan->ch_id + UNSUPPORTED_REQ)) {
			dev_err(nvtdmac->dma.dev, "Unsupported Request on eDMA channel[%d]\n", nvtchan->ch_id);
		}
		if (status & 1 << (nvtchan->ch_id + CPL_ABORT)) {
			dev_err(nvtdmac->dma.dev, "Completer Abort on eDMA channel[%d]\n", nvtchan->ch_id);
		}
		if (status & 1 << (nvtchan->ch_id + CPL_TIMEOUT)) {
			dev_err(nvtdmac->dma.dev, "Completion Time Out on eDMA channel[%d]\n", nvtchan->ch_id);
		}
		if (status & 1 << (nvtchan->ch_id + DATA_POISIONING)) {
			dev_err(nvtdmac->dma.dev, "Data Poisoning on eDMA channel[%d]\n", nvtchan->ch_id);
		}
	}
}

irqreturn_t nvt_pcie_drv_isr(int irq, void *data)
{
	unsigned long flags;
	u32 status_read = 0, status_write = 0, i;
	struct nvt_edma *nvtdmac = (struct nvt_edma *)data;

	dev_dbg(nvtdmac->dma.dev, "%s\n", __func__);

	spin_lock_irqsave(&nvtdmac->lock, flags);

	status_read = readl(nvtdmac->dma_va + DMA_READ_INT_STATUS_OFF_OFS);
	status_write = readl(nvtdmac->dma_va + DMA_WRITE_INT_STATUS_OFF_OFS);

	/* Returns if not eDMA ISR */
	if (!status_read && !status_write) {
		spin_unlock_irqrestore(&nvtdmac->lock, flags);
		return IRQ_NONE;
	}

	/*
	 * Disable the interrupts. 
	 * We'll turn them back on in the softirq handler.
	 */
	writel(status_read, nvtdmac->dma_va + DMA_READ_INT_CLEAR_OFF_OFS);
	writel(status_write, nvtdmac->dma_va + DMA_WRITE_INT_CLEAR_OFF_OFS);

	writel(status_read, nvtdmac->dma_va + DMA_READ_INT_MASK_OFF_OFS);
	writel(status_write, nvtdmac->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);

	spin_unlock_irqrestore(&nvtdmac->lock, flags);

	for (i = 0; i < nvtdmac->max_chan; i++) {
		struct nvt_edma_chan *nvtchan = &nvtdmac->chan[i];
		if ((status_write & BIT(nvtchan->ch_id)) || (status_write & BIT(nvtchan->ch_id + 16))) {
			nvtchan->status = status_write;
			nvtchan->write_pcs = nvtchan->write_pcs ? false:true;
			tasklet_schedule(&nvtchan->tasklet);
		} else if ((status_read & BIT(nvtchan->ch_id)) || (status_read & BIT(nvtchan->ch_id + 16))) {
			nvtchan->status = status_read;
			nvtchan->read_pcs = nvtchan->read_pcs ? false:true;
			tasklet_schedule(&nvtchan->tasklet);
		}
	}

	return IRQ_HANDLED;
}

static dma_cookie_t nvt_edma_tx_submit(struct dma_async_tx_descriptor *tx)
{
	struct nvt_edma_desc *desc = tx_dma_desc_to_at_desc(tx->metadata_ops);
	struct nvt_edma_chan *nvtchan = to_nvt_edma_chan(tx->chan);
	dma_cookie_t        cookie;

	dev_dbg(chan2dev(&nvtchan->chan), "%s\n", __func__);

	spin_lock(&nvtchan->lock);
	cookie = dma_cookie_assign(tx);

	list_add_tail(&desc->desc_node, &nvtchan->queue);

	spin_unlock(&nvtchan->lock);

	return cookie;
}

static struct nvt_edma_desc *nvt_edma_setup_desc(struct dma_chan *chan, dma_addr_t dest, dma_addr_t src, size_t len)
{
	struct nvt_edma_chan *nvtchan = to_nvt_edma_chan(chan);
	struct nvt_edma      *nvtdmac = to_nvt_edma(chan->device);
	dma_addr_t phys;
	struct nvt_edma_desc *desc;
	struct dma_async_tx_descriptor *txd = kzalloc(sizeof(struct dma_async_tx_descriptor), GFP_NOWAIT);

	dev_dbg(chan2dev(&nvtchan->chan), "%s: d%pad s%pad l0x%zx\n", __func__, &dest, &src, len);
	if (!txd) {
		dev_err(nvtdmac->dma.dev, "eDMA channel[%d] alloc tx descriptor fail!!!\n", nvtchan->ch_id);
		return NULL;
	}

	desc = dma_alloc_coherent(nvtdmac->dma.dev, sizeof(struct nvt_edma_desc), &phys, GFP_NOWAIT);
	if (!desc) {
		dev_err(nvtdmac->dma.dev, "eDMA channel[%d] alloc dma memory fail!!!\n", nvtchan->ch_id);
		kfree(txd);
		return NULL;
	}

	memset(desc, 0, sizeof(struct nvt_edma_desc));

	/* Avoid txd_lock stuck */
	txd->metadata_ops = &desc->ops;
	desc->tx_dma_desc = txd;

	INIT_LIST_HEAD(&desc->tx_list);
	dma_async_tx_descriptor_init(desc->tx_dma_desc, chan);
	desc->tx_dma_desc->tx_submit = nvt_edma_tx_submit;
	desc->tx_dma_desc->flags = DMA_CTRL_ACK;
	desc->tx_dma_desc->phys = phys;

	/* Setup Descriptor */
	desc->phys = phys;
	desc->element.len = len;
	desc->element.src = src;
	desc->element.dest = dest;
	desc->element.chan_ctrl = BIT(CB);

	desc->element.llp_ctrl = BIT(TCB) | BIT(LLP);
	desc->element.llp = phys;

	return desc;
}

static void
nvt_descriptor_complete(struct nvt_edma_chan *nvtchan, struct nvt_edma_desc *desc,
		                bool callback_required)
{
	struct dma_async_tx_descriptor *tx_dma_desc = desc->tx_dma_desc;
	struct nvt_edma_desc *child, *_desc;
	struct dmaengine_desc_callback cb;

	if (tx_dma_desc->cookie <= 0)
		return;

	dma_cookie_complete(tx_dma_desc);

	if (callback_required)
		dmaengine_desc_get_callback(tx_dma_desc, &cb);
	else
		memset(&cb, 0, sizeof(cb));

	/* async_tx_ack */
	list_for_each_entry_safe(child, _desc, &desc->tx_list, desc_node) {
		if (async_tx_test_ack(child->tx_dma_desc))
			async_tx_ack(child->tx_dma_desc);
	}

	if (async_tx_test_ack(desc->tx_dma_desc))
		async_tx_ack(desc->tx_dma_desc);
	nvt_desc_put(nvtchan, desc);

	dmaengine_desc_callback_invoke(&cb, NULL);
}

static int nvt_edma_device_terminate_all(struct dma_chan *chan)
{
	struct nvt_edma_desc *desc, *_desc;
	struct nvt_edma_chan *nvtchan = to_nvt_edma_chan(chan);
	unsigned long flags;

	LIST_HEAD(list);

	spin_lock_irqsave(&nvtchan->lock, flags);

	dev_dbg(chan2dev(&nvtchan->chan), "%s\n", __func__);

	/* xfers_list entries will end up before queued entries */
	list_splice_init(&nvtchan->queue, &list);
	list_splice_init(&nvtchan->xfers_list, &list);

	if (list_empty(&list)) {
		spin_unlock_irqrestore(&nvtchan->lock, flags);
		return 0;
	}

	/* Flush all pending and queued descriptors */
	list_for_each_entry_safe(desc, _desc, &list, desc_node)
		nvt_descriptor_complete(nvtchan, desc, false);

	spin_unlock_irqrestore(&nvtchan->lock, flags);

	return 0;
}

static struct dma_async_tx_descriptor *
nvt_edma_prep_dma_memcpy(struct dma_chan *chan, dma_addr_t dest, dma_addr_t src,
		size_t len, unsigned long flags)
{
	struct nvt_edma_desc *desc;
	struct nvt_edma_chan *nvtchan = to_nvt_edma_chan(chan);

	dev_dbg(chan2dev(&nvtchan->chan), "%s: d%pad s%pad l0x%zx f0x%lx\n", __func__, &dest, &src, len, flags);

	spin_lock(&nvtchan->lock);
	if (unlikely(!len) || len > DMA_MAX_SIZE) {
		dev_err(chan2dev(&nvtchan->chan), "%s: length is over size(%x) or 0!\n", __func__, DMA_MAX_SIZE);
		return NULL;
	}

	desc = nvt_edma_setup_desc(chan, dest, src, len);
	if (!desc) {
		goto err_desc_get;
	}

	/* Trigger interrupt after last block */
	desc->element.chan_ctrl |= BIT(LIE);

	desc->tx_dma_desc->flags = flags;
	spin_unlock(&nvtchan->lock);

	return desc->tx_dma_desc;

err_desc_get:
	spin_unlock(&nvtchan->lock);
	nvt_desc_put(nvtchan, desc);
	return NULL;
}

static int nvt_edma_device_config(struct dma_chan *chan, struct dma_slave_config *config)
{
	struct nvt_edma_chan *nvtchan = to_nvt_edma_chan(chan);
	struct nvt_edma *nvtdmac = to_nvt_edma(nvtchan->chan.device);
	unsigned int width = 5;
	u32 val;

	dev_dbg(chan2dev(&nvtchan->chan), "%s\n", __func__);

	spin_lock(&nvtchan->lock);

	if (nvtchan->active) {
		dev_err(chan2dev(&nvtchan->chan), "channel is busy, config failed\n");
		spin_unlock(&nvtchan->lock);
		return -EBUSY;
	}

	if (!is_slave_direction(config->direction)) {
		dev_err(chan2dev(&nvtchan->chan), "direction is not support, config failed\n");
		spin_unlock(&nvtchan->lock);
		return -EINVAL;
	}

	if (config->src_maxburst != config->dst_maxburst || config->src_maxburst > 31) {
		dev_err(chan2dev(&nvtchan->chan), "MaxBurst should be in the range [0~31], SRC DST must have the same value, config failed\n");
		spin_unlock(&nvtchan->lock);
		return -EINVAL;
	}

	nvtchan->direction = config->direction;
	nvtchan->maxburst = config->src_maxburst;

	if (nvtchan->direction == DMA_MEM_TO_DEV) {
		if (nvtchan->ch_id <= 3) {
			unsigned int offset = nvtchan->ch_id * 5;
			val = readl(nvtdmac->dma_va + DMA_WRITE_CHAN_WEIGHT_LOW_OFF_OFS);
			/* Clear burst bits */
			val &= ~(GENMASK(width, 0) << offset);
			/* Set burst bits */
			val |= nvtchan->maxburst << offset;
			writel(val, nvtdmac->dma_va + DMA_WRITE_CHAN_WEIGHT_LOW_OFF_OFS);
		} else {
			unsigned int offset = (nvtchan->ch_id - 4) * 5;
			val = readl(nvtdmac->dma_va + DMA_WRITE_CHAN_WEIGHT_HIGH_OFF_OFS);
			/* Clear burst bits */
			val &= ~(GENMASK(width, 0) << offset);
			/* Set burst bits */
			val |= nvtchan->maxburst << offset;
			writel(val, nvtdmac->dma_va + DMA_WRITE_CHAN_WEIGHT_HIGH_OFF_OFS);
		}
	} else {
		if (nvtchan->ch_id <= 3) {
			unsigned int offset = nvtchan->ch_id * 5;
			val = readl(nvtdmac->dma_va + DMA_READ_CHAN_WEIGHT_LOW_OFF_OFS);
			/* Clear burst bits */
			val &= ~(GENMASK(width, 0) << offset);
			/* Set burst bits */
			val |= nvtchan->maxburst << offset;
			writel(val, nvtdmac->dma_va + DMA_READ_CHAN_WEIGHT_LOW_OFF_OFS);
		} else {
			unsigned int offset = (nvtchan->ch_id - 4) * 5;
			val = readl(nvtdmac->dma_va + DMA_READ_CHAN_WEIGHT_HIGH_OFF_OFS);
			/* Clear burst bits */
			val &= ~(GENMASK(width, 0) << offset);
			/* Set burst bits */
			val |= nvtchan->maxburst << offset;
			writel(val, nvtdmac->dma_va + DMA_READ_CHAN_WEIGHT_HIGH_OFF_OFS);
		}
	}

	memcpy(&nvtchan->sconfig, config, sizeof(nvtchan->sconfig));

	spin_unlock(&nvtchan->lock);

	return 0;
}

void nvt_edma_reinit(void)
{
	int idx;
	struct nvt_edma *nvt_edma_ptr = g_nvt_edma;
	if (!nvt_edma_ptr) {
		pr_err("%s: nvt_edma_ptr is NULL\n", __func__);
		return;
	}

	/* Disable eDMA IRQ Mask */
	writel(0, nvt_edma_ptr->dma_va + DMA_READ_INT_MASK_OFF_OFS);
	writel(0, nvt_edma_ptr->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);
	writel(DMA_LINKED_LIST_ERR_MASK, nvt_edma_ptr->dma_va + DMA_WRITE_LINKED_LIST_ERR_EN_OFF_OFS);
	writel(DMA_LINKED_LIST_ERR_MASK, nvt_edma_ptr->dma_va + DMA_READ_LINKED_LIST_ERR_EN_OFF_OFS);
	writel(DMA_ENABLE, nvt_edma_ptr->dma_va + DMA_WRITE_ENGINE_EN_OFF_OFS);
	writel(DMA_ENABLE, nvt_edma_ptr->dma_va + DMA_READ_ENGINE_EN_OFF_OFS);

	for (idx = 0; idx < nvt_edma_ptr->max_chan; idx++) {
		unsigned int val;
		struct nvt_edma_chan *nvtchan = &nvt_edma_ptr->chan[idx];
		writel(DMA_STOP, nvt_edma_ptr->dma_va + DMA_CH_CONTROL1_OFF_WRCH_OFS(nvtchan->ch_id));
		writel(DMA_STOP, nvt_edma_ptr->dma_va + DMA_CH_CONTROL1_OFF_RDCH_OFS(nvtchan->ch_id));
		writel(0x0, nvt_edma_ptr->dma_va + DMA_TRANSFER_SIZE_OFF_WRCH_OFS(nvtchan->ch_id));
		writel(0x0, nvt_edma_ptr->dma_va + DMA_TRANSFER_SIZE_OFF_RDCH_OFS(nvtchan->ch_id));
		/* Setup Write channel */
		nvtchan->write_pcs = true;
		writel(nvtchan->write_desc->phys & 0xFFFFFFFF, nvt_edma_ptr->dma_va + DMA_LLP_LOW_OFF_WRCH_OFS(nvtchan->ch_id));
		writel(nvtchan->write_desc->phys >> 32, nvt_edma_ptr->dma_va + DMA_LLP_HIGH_OFF_WRCH_OFS(nvtchan->ch_id));
		val = BIT(CCS) | BIT(LLE);
		writel(val, nvt_edma_ptr->dma_va + DMA_CH_CONTROL1_OFF_WRCH_OFS(nvtchan->ch_id));

		/* Setup Read channel */
		nvtchan->read_pcs = true;
		writel(nvtchan->read_desc->phys & 0xFFFFFFFF, nvt_edma_ptr->dma_va + DMA_LLP_LOW_OFF_RDCH_OFS(nvtchan->ch_id));
		writel(nvtchan->read_desc->phys >> 32, nvt_edma_ptr->dma_va + DMA_LLP_HIGH_OFF_RDCH_OFS(nvtchan->ch_id));
		val = BIT(CCS) | BIT(LLE);
		writel(val, nvt_edma_ptr->dma_va + DMA_CH_CONTROL1_OFF_RDCH_OFS(nvtchan->ch_id));
	}
}
EXPORT_SYMBOL(nvt_edma_reinit);

static void nvt_dma_start_xfer(struct nvt_edma_chan *nvtchan, struct nvt_edma_desc *desc)
{
	unsigned long flags;
	struct nvt_edma *nvtdmac = to_nvt_edma(nvtchan->chan.device);

	dev_dbg(chan2dev(&nvtchan->chan), "%s\n", __func__);
	spin_lock_irqsave(&nvtdmac->lock, flags);

	nvtchan->active = true;
	/* DMA Engine Enable */
	if (nvtchan->direction == DMA_MEM_TO_DEV) {
		nvtchan->write_desc->tx_dma_desc = desc->tx_dma_desc;
		nvtchan->write_desc->element.len = desc->element.len;
		nvtchan->write_desc->element.src = desc->element.src;
		nvtchan->write_desc->element.dest = desc->element.dest;
		nvtchan->write_desc->element.llp = nvtchan->write_desc->phys;
		if (nvtchan->write_pcs) {
			nvtchan->write_desc->element.chan_ctrl = BIT(CB) | BIT(LIE);
			nvtchan->write_desc->element.llp_ctrl = BIT(TCB) | BIT(LLP);
		} else {
			nvtchan->write_desc->element.chan_ctrl = BIT(LIE);
			nvtchan->write_desc->element.llp_ctrl = BIT(TCB) | BIT(LLP) | BIT(CB);
		}
		dma_sync_single_for_device(chan2dev(&nvtchan->chan), nvtchan->write_desc->phys, sizeof(struct nvt_edma_desc), DMA_TO_DEVICE);
		writel(nvtchan->ch_id, nvtdmac->dma_va + DMA_WRITE_DOORBELL_OFF_OFS);
	} else {
		nvtchan->read_desc->tx_dma_desc = desc->tx_dma_desc;
		nvtchan->read_desc->element.len = desc->element.len;
		nvtchan->read_desc->element.src = desc->element.src;
		nvtchan->read_desc->element.dest = desc->element.dest;
		nvtchan->read_desc->element.llp = nvtchan->read_desc->phys;
		if (nvtchan->read_pcs) {
			nvtchan->read_desc->element.chan_ctrl = BIT(CB) | BIT(LIE);
			nvtchan->read_desc->element.llp_ctrl = BIT(TCB) | BIT(LLP);
		} else {
			nvtchan->read_desc->element.chan_ctrl = BIT(LIE);
			nvtchan->read_desc->element.llp_ctrl = BIT(TCB) | BIT(LLP) | BIT(CB);
		}
		dma_sync_single_for_device(chan2dev(&nvtchan->chan), nvtchan->read_desc->phys, sizeof(struct nvt_edma_desc), DMA_TO_DEVICE);
		writel(nvtchan->ch_id, nvtdmac->dma_va + DMA_READ_DOORBELL_OFF_OFS);
	}
	spin_unlock_irqrestore(&nvtdmac->lock, flags);
}

static void nvt_start_first_queued(struct nvt_edma_chan *nvtchan)
{
	struct nvt_edma_desc *desc;

	if (list_empty(nvtchan->queue.next))
		return;

	list_move(nvtchan->queue.next, &nvtchan->xfers_list);

	desc = list_first_entry(&nvtchan->xfers_list, struct nvt_edma_desc, desc_node);
	nvt_dma_start_xfer(nvtchan, desc);
}

static void nvt_edma_issue_pending(struct dma_chan *chan)
{
	unsigned long flags;
	struct nvt_edma_chan *nvtchan = to_nvt_edma_chan(chan);

	dev_dbg(chan2dev(&nvtchan->chan), "%s\n", __func__);

	spin_lock_irqsave(&nvtchan->lock, flags);

	if (!is_slave_direction(nvtchan->direction)) {
		dev_err(chan2dev(&nvtchan->chan), "dma channel direction is not config, %s failed\n", __func__);
		spin_unlock_irqrestore(&nvtchan->lock, flags);
		return;
	}

	if (list_empty(&nvtchan->xfers_list) && !nvtchan->active)
		nvt_start_first_queued(nvtchan);

	spin_unlock_irqrestore(&nvtchan->lock, flags);
}

static enum dma_status
nvt_edma_tx_status(struct dma_chan *chan, dma_cookie_t cookie, struct dma_tx_state *txstate) {
	return dma_cookie_status(chan, cookie, txstate);
}

static int nvt_edma_alloc_chan_resources(struct dma_chan *chan)
{
	dma_cookie_init(chan);
	return 0;
}

static void nvt_edma_free_chan_resources(struct dma_chan *chan)
{
	struct nvt_edma_chan *nvtchan = to_nvt_edma_chan(chan);
	dev_dbg(chan2dev(&nvtchan->chan), "%s\n", __func__);
	nvt_edma_device_terminate_all(chan);
	return;
}

static struct dma_chan *nvt_edma_xlate(struct of_phandle_args *dma_spec, struct of_dma *of_dma)
{
	struct nvt_edma     *nvtdmac = of_dma->of_dma_data;
	struct dma_chan *chan;
	unsigned int ch_id;

	dev_dbg(nvtdmac->dma.dev, "%s\n", __func__);
	if (dma_spec->args_count != 3) {
		dev_err(nvtdmac->dma.dev, "dma phandler args: bad number of args\n");
		return NULL;
	}

	ch_id = dma_spec->args[0];
	if (dma_spec->args[1] == 0)
		nvtdmac->chan[ch_id].direction = DMA_MEM_TO_DEV;
	else
		nvtdmac->chan[ch_id].direction = DMA_DEV_TO_MEM;
	nvtdmac->chan[ch_id].maxburst = dma_spec->args[2];

	chan = dma_get_slave_channel(&nvtdmac->chan[ch_id].chan);
	if (!chan) {
		dev_err(nvtdmac->dma.dev, "can't get a dma channel\n");
		return NULL;
	}

	return chan;
}

static void nvt_edma_tasklet(struct tasklet_struct *t)
{
	unsigned long flags;
	u32 val;
	struct nvt_edma_desc *child, *desc, *_desc;
	struct nvt_edma_chan *nvtchan = from_tasklet(nvtchan, t, tasklet);
	struct nvt_edma *nvtdmac = to_nvt_edma(nvtchan->chan.device);

	spin_lock(&nvtchan->lock);

	nvtchan->active = false;

	if (list_empty(&nvtchan->xfers_list)) {
		spin_unlock(&nvtchan->lock);
		goto exit_takelet;
	}

	desc = list_first_entry(&nvtchan->xfers_list, struct nvt_edma_desc, desc_node);
	if (!desc) {
		spin_unlock(&nvtchan->lock);
		goto exit_takelet;
	}

	dma_cookie_complete(desc->tx_dma_desc);

	/* async_tx_ack */
	list_for_each_entry_safe(child, _desc, &desc->tx_list, desc_node) {
		async_tx_ack(child->tx_dma_desc);
	}
	async_tx_ack(desc->tx_dma_desc);

	if (desc->tx_dma_desc->flags & DMA_PREP_INTERRUPT) {
		dmaengine_desc_get_callback_invoke(desc->tx_dma_desc, NULL);
	}

	if (nvtchan->status & 0xFFFF0000) {
		nvt_desc_put(nvtchan, desc);
		nvt_handle_err(nvtchan);
		spin_unlock(&nvtchan->lock);
		nvt_edma_device_terminate_all(&nvtchan->chan);
		goto exit_takelet;
	}

	/* Clear the xfers list if is the last descriptor currectlly */
	if (list_is_last(nvtchan->xfers_list.next, &nvtchan->xfers_list)) {
		list_del_init(&nvtchan->xfers_list);
	}

	nvt_desc_put(nvtchan, desc);
	if (list_empty(&nvtchan->xfers_list)){
		nvt_start_first_queued(nvtchan);
	} else {
		list_for_each_entry_safe(child, _desc, &nvtchan->xfers_list, desc_node) {
			nvt_dma_start_xfer(nvtchan, child);
			break;
		}
	}

	spin_unlock(&nvtchan->lock);

exit_takelet:
	/* Disable eDMA IRQ Mask */
	spin_lock_irqsave(&nvtdmac->lock, flags);
	val = readl(nvtdmac->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);
	val = val & ~BIT(nvtchan->ch_id);
	writel(val, nvtdmac->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);

	val = readl(nvtdmac->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);
	val = val & ~BIT(nvtchan->ch_id + 16);
	writel(val, nvtdmac->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);

	val = readl(nvtdmac->dma_va + DMA_READ_INT_MASK_OFF_OFS);
	val = val & ~BIT(nvtchan->ch_id);
	writel(val, nvtdmac->dma_va + DMA_READ_INT_MASK_OFF_OFS);

	val = readl(nvtdmac->dma_va + DMA_READ_INT_MASK_OFF_OFS);
	val = val & ~BIT(nvtchan->ch_id + 16);
	writel(val, nvtdmac->dma_va + DMA_READ_INT_MASK_OFF_OFS);
	spin_unlock_irqrestore(&nvtdmac->lock, flags);
}


static int nvt_edma_probe(struct platform_device *pdev)
{
	int idx, ret;
	struct resource *res = NULL;
	struct nvt_edma *nvt_edma_ptr = NULL;
#ifdef CONFIG_NVT_NT98690_PCI_EP_EVB
	unsigned int reg;
#endif

	nvt_edma_ptr = devm_kzalloc(&pdev->dev, sizeof(struct nvt_edma), GFP_KERNEL);
	if (!nvt_edma_ptr) {
		pr_err("Alloc memory for nvt_edma fail\n");
		return -ENOMEM;
	}

	nvt_edma_ptr->dma.dev = &pdev->dev;

	if (device_property_read_u32(&pdev->dev, "dma-channels", &nvt_edma_ptr->max_chan)) {
		dev_err(nvt_edma_ptr->dma.dev, "Can not find dma-channels on device tree\n");
		return -EINVAL;
	}

	nvt_edma_ptr->chan = devm_kcalloc(&pdev->dev, nvt_edma_ptr->max_chan, sizeof(struct nvt_edma_chan), GFP_KERNEL);
	if (!nvt_edma_ptr->chan) {
		dev_err(nvt_edma_ptr->dma.dev, "Alloc memory for edma channel fail\n");
		return -ENOMEM;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pci");
	if (unlikely(!res)) {
		dev_err(nvt_edma_ptr->dma.dev, "platform_get_resource fail on pci\n");
		return -ENXIO;
	}

	nvt_edma_ptr->pci_va = devm_ioremap(&pdev->dev, res->start, res->end - res->start);
	if (unlikely(nvt_edma_ptr->pci_va == 0)) {
		dev_err(nvt_edma_ptr->dma.dev, "ioremap fail on pci\n");
		return -EIO;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dma");
	if (unlikely(!res)) {
		dev_err(nvt_edma_ptr->dma.dev, "platform_get_resource fail on dma\n");
		return -ENXIO;
	}

	nvt_edma_ptr->dma_va = devm_ioremap(&pdev->dev, res->start, res->end - res->start);
	if (unlikely(nvt_edma_ptr->dma_va == 0)) {
		dev_err(nvt_edma_ptr->dma.dev, "ioremap fail on dma\n");
		return -EIO;
	}

	// coverity[side_effect_free]
	spin_lock_init(&nvt_edma_ptr->lock);
	nvt_edma_ptr->irq = platform_get_irq(pdev, 0);
	if (nvt_edma_ptr->irq < 0) {
		dev_err(nvt_edma_ptr->dma.dev, "get irq fail(%d)\n", nvt_edma_ptr->irq);
		return -EINVAL;
	}

	if (devm_request_irq(&pdev->dev, nvt_edma_ptr->irq, nvt_pcie_drv_isr, IRQF_TRIGGER_HIGH, "pci_dma_irq", nvt_edma_ptr)) {
		dev_err(nvt_edma_ptr->dma.dev, "request irq fail\n");
		return -ENODEV;
	}

	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(64);
	dma_cap_set(DMA_SLAVE, nvt_edma_ptr->dma.cap_mask);
	nvt_edma_ptr->dma.dev 				= &pdev->dev;
	nvt_edma_ptr->dma.device_alloc_chan_resources	= nvt_edma_alloc_chan_resources;
	nvt_edma_ptr->dma.device_free_chan_resources	= nvt_edma_free_chan_resources;
	nvt_edma_ptr->dma.device_tx_status		= nvt_edma_tx_status;
	nvt_edma_ptr->dma.device_prep_dma_memcpy	= nvt_edma_prep_dma_memcpy;
	nvt_edma_ptr->dma.device_issue_pending		= nvt_edma_issue_pending;
	nvt_edma_ptr->dma.device_config			= nvt_edma_device_config;
	nvt_edma_ptr->dma.device_terminate_all		= nvt_edma_device_terminate_all;
	nvt_edma_ptr->dma.src_addr_widths		= 1;
	nvt_edma_ptr->dma.dst_addr_widths		= 1;
	nvt_edma_ptr->dma.directions			= BIT(DMA_DEV_TO_MEM) | BIT(DMA_MEM_TO_DEV);
	nvt_edma_ptr->dma.residue_granularity		= DMA_RESIDUE_GRANULARITY_BURST;

	/* Init channels. */
	INIT_LIST_HEAD(&nvt_edma_ptr->dma.channels);
	for (idx = 0; idx < nvt_edma_ptr->max_chan; idx++) {
		struct nvt_edma_chan *nvtchan = &nvt_edma_ptr->chan[idx];
		// coverity[side_effect_free]
		spin_lock_init(&nvtchan->lock);

		nvtchan->chan.device = &nvt_edma_ptr->dma;
		list_add_tail(&nvtchan->chan.device_node,
				&nvt_edma_ptr->dma.channels);
		nvtchan->ch_id = idx;
		nvtchan->active = false;

		INIT_LIST_HEAD(&nvtchan->xfers_list);
		INIT_LIST_HEAD(&nvtchan->queue);
		tasklet_setup(&nvtchan->tasklet, nvt_edma_tasklet);
	}

#ifdef CONFIG_NVT_NT98690_PCI_EP_EVB
	/* Enable eDMA IRQ */
	reg = readl(nvt_edma_ptr->pci_va + PCIE_IRQ_EN_OFF_OFS);
	reg = reg & ~BIT(PCIE_EDMA_IRQ_OFS);
	dev_dbg(nvt_edma_ptr->dma.dev, "Enable eDMA IRQ : 0x%x\n",reg);
	writel(reg, nvt_edma_ptr->pci_va + PCIE_IRQ_EN_OFF_OFS);
#endif

	ret = dma_async_device_register(&nvt_edma_ptr->dma);
	if (ret) {
		dev_err(&pdev->dev, "fail to register DMA engine device\n");
		return ret;
	}

	ret = of_dma_controller_register(pdev->dev.of_node,
			nvt_edma_xlate, nvt_edma_ptr);
	if (ret) {
		dev_err(&pdev->dev, "could not register of dma controller\n");
		return ret;
	}

	/* Disable eDMA IRQ Mask */
	writel(0, nvt_edma_ptr->dma_va + DMA_READ_INT_MASK_OFF_OFS);
	writel(0, nvt_edma_ptr->dma_va + DMA_WRITE_INT_MASK_OFF_OFS);
	writel(DMA_LINKED_LIST_ERR_MASK, nvt_edma_ptr->dma_va + DMA_WRITE_LINKED_LIST_ERR_EN_OFF_OFS);
	writel(DMA_LINKED_LIST_ERR_MASK, nvt_edma_ptr->dma_va + DMA_READ_LINKED_LIST_ERR_EN_OFF_OFS);
	writel(DMA_ENABLE, nvt_edma_ptr->dma_va + DMA_WRITE_ENGINE_EN_OFF_OFS);
	writel(DMA_ENABLE, nvt_edma_ptr->dma_va + DMA_READ_ENGINE_EN_OFF_OFS);

	for (idx = 0; idx < nvt_edma_ptr->max_chan; idx++) {
		unsigned int val;
		dma_addr_t phys;
		struct nvt_edma_chan *nvtchan = &nvt_edma_ptr->chan[idx];
		writel(DMA_STOP, nvt_edma_ptr->dma_va + DMA_CH_CONTROL1_OFF_WRCH_OFS(nvtchan->ch_id));
		writel(DMA_STOP, nvt_edma_ptr->dma_va + DMA_CH_CONTROL1_OFF_RDCH_OFS(nvtchan->ch_id));
		writel(0x0, nvt_edma_ptr->dma_va + DMA_TRANSFER_SIZE_OFF_WRCH_OFS(nvtchan->ch_id));
		writel(0x0, nvt_edma_ptr->dma_va + DMA_TRANSFER_SIZE_OFF_RDCH_OFS(nvtchan->ch_id));
		/* Setup Write channel */
		nvtchan->write_pcs = true;
		nvtchan->write_desc = dma_alloc_coherent(nvt_edma_ptr->dma.dev, sizeof(struct nvt_edma_desc), &phys, GFP_KERNEL);
		if (!nvtchan->write_desc) {
			dev_err(&pdev->dev, "fail to alloc write dma buffer\n");
			return -ENOMEM;
		}
		nvtchan->write_desc->phys = phys;
		writel(nvtchan->write_desc->phys & 0xFFFFFFFF, nvt_edma_ptr->dma_va + DMA_LLP_LOW_OFF_WRCH_OFS(nvtchan->ch_id));
		writel(nvtchan->write_desc->phys >> 32, nvt_edma_ptr->dma_va + DMA_LLP_HIGH_OFF_WRCH_OFS(nvtchan->ch_id));
		val = BIT(CCS) | BIT(LLE);
		writel(val, nvt_edma_ptr->dma_va + DMA_CH_CONTROL1_OFF_WRCH_OFS(nvtchan->ch_id));

		/* Setup Read channel */
		nvtchan->read_pcs = true;
		nvtchan->read_desc = dma_alloc_coherent(nvt_edma_ptr->dma.dev, sizeof(struct nvt_edma_desc), &phys, GFP_KERNEL);
		if (!nvtchan->read_desc) {
			dev_err(&pdev->dev, "fail to alloc read dma buffer\n");
			return -ENOMEM;
		}
		nvtchan->read_desc->phys = phys;
		writel(nvtchan->read_desc->phys & 0xFFFFFFFF, nvt_edma_ptr->dma_va + DMA_LLP_LOW_OFF_RDCH_OFS(nvtchan->ch_id));
		writel(nvtchan->read_desc->phys >> 32, nvt_edma_ptr->dma_va + DMA_LLP_HIGH_OFF_RDCH_OFS(nvtchan->ch_id));
		val = BIT(CCS) | BIT(LLE);
		writel(val, nvt_edma_ptr->dma_va + DMA_CH_CONTROL1_OFF_RDCH_OFS(nvtchan->ch_id));
	}

	platform_set_drvdata(pdev, nvt_edma_ptr);
	g_nvt_edma = nvt_edma_ptr;

	dev_info(&pdev->dev, "Register %s successfully\n", __func__);
	return 0;
}

static int nvt_edma_remove(struct platform_device *pdev)
{
	struct nvt_edma *nvt_edma_ptr = platform_get_drvdata(pdev);
	int idx;

	dma_async_device_unregister(&nvt_edma_ptr->dma);

	platform_set_drvdata(pdev, NULL);

	for (idx = 0; idx < nvt_edma_ptr->max_chan; idx++) {
		struct nvt_edma_chan *nvtchan = &nvt_edma_ptr->chan[idx];
		nvt_edma_device_terminate_all(&nvtchan->chan);
		tasklet_kill(&nvtchan->tasklet);
	}

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

