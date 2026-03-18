/**
    NVT PCIE cascade driver
    To handle NVT PCIE cascade driver: RC/EP connection and isr handling
    @file nvt-pcie-drv.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/soc/nvt/nvt-io.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/msi.h>
#include <linux/dma-mapping.h>

#include "nvt_pcie_int.h"
#include "pcie-drv-wrap.h"

#define DRV_VERSION		"0.00.019"

#define loc_cpu(lock, flags) spin_lock_irqsave(lock, flags)
#define unl_cpu(lock, flags) spin_unlock_irqrestore(lock, flags)

static int is_init = 0;

static void nvt_pcie_int_get_status(struct nvt_pcie_chip *chip, unsigned long *pending_int)
{
	unsigned int n;

	for (n = 0; n < NR_HW_IRQS; n++) {
		pending_int[n] = msix_recv_cmd_get_pending_irq(chip, n) & msix_recv_cmd_get_enabled_irq(chip, n);
	}

	return;
}

static void nvt_pcie_summary_irq_handler(struct irq_desc *desc)
{
	unsigned int irq = irq_desc_get_irq(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = irq_get_handler_data(irq);
	unsigned int virq = 0, bit = 0, n = 0;
	unsigned long pending_int[NR_HW_IRQS];
	const unsigned int gic_offset = irq - nvt_pcie_chip_ptr->gic_irq_num[0];

//	printk("%s: enter, parent irq %d, data irq %d, base irq %d\r\n", __func__, desc->parent_irq, irq, nvt_pcie_chip_ptr->gic_irq_num[0]);
	/* To read the interrupt status to find which pin is trigger: msi_int_status */
	nvt_pcie_int_get_status(nvt_pcie_chip_ptr, pending_int);

	/* mack irq */
	chained_irq_enter(chip, desc);

	/* To find PCIE MSIX which pin is triggered */
	bit = gic_offset;
	for (n = 0; n < NR_HW_IRQS; n++)
	{
//		printk("%s: HW GROUP %d, pending 0x%lx\r\n", __func__, n, pending_int[n]);
		if (pending_int[n] & (1<<bit)) {
//			printk("%s: HW GROUP %d, bit %d\r\n", __func__, n, bit);
			msix_recv_cmd_chk_pending_dummy_data(nvt_pcie_chip_ptr, bit + (n * NR_HW_IRQ_GROP_NUM));
			/* To find the virtual irq number */
			virq = irq_find_mapping(nvt_pcie_chip_ptr->domain, bit + (n * NR_HW_IRQ_GROP_NUM));
//			printk("%s: virq %d found\r\n", __func__, virq);
			/* Assign virq to handle irq isr */
			generic_handle_irq(virq);
			/* Clear Interrupt Source */
			msix_clear_msix_tx_irq_status(nvt_pcie_chip_ptr, bit + (n * NR_HW_IRQ_GROP_NUM));
		}
	}

	/* unmask irq */
	chained_irq_exit(chip, desc);
}

static void msix_unmask_irq(struct irq_data *data)
{
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = irq_data_get_irq_chip_data(data);
	unsigned long flags;

//	printk("%s: irq %ld, group %ld, bit %lu\r\n", __func__, data->hwirq, data->hwirq/NR_HW_IRQ_GROP_NUM, data->hwirq%NR_HW_IRQ_GROP_NUM);

	spin_lock_irqsave(&nvt_pcie_chip_ptr->lock, flags);

	// unmask remote EP irq
	// (because msix_enable_irq() will dec INT_GIC_SPI_START_ID, add it here)
	msix_enable_irq(nvt_pcie_chip_ptr, data->hwirq + INT_GIC_SPI_START_ID);

	// unmask local msix-rx irq
	msix_recv_cmd_set_en_irq(nvt_pcie_chip_ptr, data->hwirq/NR_HW_IRQ_GROP_NUM, 1<<(data->hwirq%NR_HW_IRQ_GROP_NUM));

	spin_unlock_irqrestore(&nvt_pcie_chip_ptr->lock, flags);
}

static void msix_mask_irq(struct irq_data *data)
{
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = irq_data_get_irq_chip_data(data);
	unsigned long flags;

//	printk("%s: irq %ld, group %ld, bit %lu\r\n", __func__, data->hwirq, data->hwirq/NR_HW_IRQ_GROP_NUM, data->hwirq%NR_HW_IRQ_GROP_NUM);

	spin_lock_irqsave(&nvt_pcie_chip_ptr->lock, flags);

	// mask remote EP irq
	// (because msix_disable_irq() will dec INT_GIC_SPI_START_ID, add it here)
	msix_disable_irq(nvt_pcie_chip_ptr, data->hwirq + INT_GIC_SPI_START_ID);

	// mask local msix-rx irq
	msix_recv_cmd_clr_en_irq(nvt_pcie_chip_ptr, data->hwirq/NR_HW_IRQ_GROP_NUM, 1<<(data->hwirq%NR_HW_IRQ_GROP_NUM));

	spin_unlock_irqrestore(&nvt_pcie_chip_ptr->lock, flags);
}

#if 1
static void msix_irq_ack(struct irq_data *data)
{
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = irq_data_get_irq_chip_data(data);
	unsigned long flags;

//	printk("%s: hwirq %ld\r\n", __func__, data->hwirq);
//	printk("%s: HW GROUP %lu, bit %lu\r\n", __func__, data->hwirq/NR_HW_IRQ_GROP_NUM, data->hwirq%NR_HW_IRQ_GROP_NUM);

	spin_lock_irqsave(&nvt_pcie_chip_ptr->lock, flags);

	msix_recv_cmd_clr_pending_irq(nvt_pcie_chip_ptr,
		data->hwirq/NR_HW_IRQ_GROP_NUM,
		1<<(data->hwirq%NR_HW_IRQ_GROP_NUM));

	spin_unlock_irqrestore(&nvt_pcie_chip_ptr->lock, flags);
}
#endif

static struct irq_chip nvt_pcie_msi_irq_chip = {
	.name = "Novatek PCIe MSIX",
#if 1
	.irq_enable = msix_unmask_irq,
	.irq_disable = msix_mask_irq,
	.irq_mask = msix_mask_irq,
	.irq_unmask = msix_unmask_irq,
	.irq_ack = msix_irq_ack,
#endif
};

static int nvt_pcie_intx_map(struct irq_domain *domain, unsigned int irq,
			     irq_hw_number_t hwirq)
{
	printk("%s: map %d\r\n", __func__, irq);
	irq_set_chip_and_handler(irq, &nvt_pcie_msi_irq_chip, handle_edge_irq);
	irq_set_chip_data(irq, domain->host_data);

	return 0;
}

static int nvt_pcie_intx_translate(struct irq_domain *d,
				      struct irq_fwspec *fwspec,
				      unsigned long *hwirq,
				      unsigned int *type)
{
	printk("%s: enter\r\n", __func__);
	if (is_of_node(fwspec->fwnode)) {
		// compatible with GIC 3-tupples description
		if (fwspec->param_count < 3)
			return -EINVAL;

		*hwirq = fwspec->param[1];
		*type = IRQ_TYPE_NONE;
	}

	return 0;
}

#if 0
static int nvt_pcie_intx_match(struct irq_domain *d, struct device_node *node,
		     enum irq_domain_bus_token bus_token)
{
	printk("%s: enter. node %s, irq_domain %s\r\n", __func__, node->name, d->name);
	return 0;
}
#endif

static const struct irq_domain_ops nvt_pcie_domain_ops = {
	.map = nvt_pcie_intx_map,
	.translate = nvt_pcie_intx_translate,
//	.match = nvt_pcie_intx_match,
};

static int nvt_pcie_install_irq_chip(struct nvt_pcie_chip *chip)
{
	int n = 0;

	/* To create PCIE MSIX IRQ domain with total expend irq number */
	chip->domain = irq_domain_add_tree(chip->dev->of_node,
						  &nvt_pcie_domain_ops, chip);
	if (!chip->domain)
		return -ENOMEM;

	/*
	 * Assign handler to PCIE MSIX hw irq which is used to connect ARM GIC
	 * Once HWIRQ is triggered by PCIE MSIX, this handler will be called.
	 */
	for (n = 0; n < NR_HW_IRQS; n++) {
		irq_set_chained_handler_and_data(chip->gic_irq_num[n], nvt_pcie_summary_irq_handler, chip);
	}

	return 0;
}

#if 0
static irqreturn_t nvt_pcie_irq(int irq, void *device_id)
{

	return IRQ_HANDLED;
}
#endif

static void nvt_pcie_gen_info(struct nvt_pcie_chip *chip)
{
	unsigned int reg;

	reg = chip->ep.read_cfg(&chip->ep, 0);
	if (reg != 0xFFFFFFFF) {
		int i;
		struct nvt_pcie_info *p_info;

		p_info = nvt_pcie_get_info();
		if (p_info) {
			p_info->link_masks |= 1<<CHIP_EP0;
			p_info->dbi_ptr = chip->PCIE_REG_BASE[MEM_PCIE_DBI];

			// RC
			for (i=0; i<ATU_MAP_COUNT; i++) {
				printk("%s: addr 0x%llx, size 0x%lx\r\n", __func__,
					chip->rc.phys_map[i].addr,
					chip->rc.phys_map[i].size);
				p_info->pcie_chip_map[CHIP_RC][i].type = chip->rc.phys_map[i].type;
				p_info->pcie_chip_map[CHIP_RC][i].addr = chip->rc.phys_map[i].addr;
				p_info->pcie_chip_map[CHIP_RC][i].size = chip->rc.phys_map[i].size;
			}
			// EP
			for (i=0; i<ATU_MAP_COUNT; i++) {
				printk("%s: addr 0x%llx, size 0x%lx\r\n", __func__,
					chip->ep.phys_map[i].addr,
					chip->ep.phys_map[i].size);
				p_info->pcie_chip_map[CHIP_EP0][i].type = chip->ep.phys_map[i].type;
				p_info->pcie_chip_map[CHIP_EP0][i].addr = chip->ep.phys_map[i].addr;
				p_info->pcie_chip_map[CHIP_EP0][i].size = chip->ep.phys_map[i].size;
			}
		}
	}
}

// additionl region beside dts
static struct nvt_pcie_region ep_regions[MEM_PRIVATE_COUNT] = {
	// MEM_EP_MSIX_TBL
	{"EP_MSIX_TBL",	0x610020000,	0x10000},
	// MEM_EP_ATU
	{"EP_ATU",	0x610000000,	0x20000},
	// MEM_EP_CFG
	{"EP_CFG",	0x600000000,	0x10000},
};

static int load_rc_inbound_dts(struct nvt_pcie_chip *chip)
{
	int rlen, offset, end;
	const __be32 *cell;
	const int CELL_SIZE = 2;

	cell = of_get_property(chip->dev->of_node, "dma-ranges", &rlen);
	if (cell == NULL) {
		printk("%s: property dma-ranges NOT found\r\n", __func__);
	} else {
		printk("%s: property dma-ranges len %d\r\n", __func__, rlen);
	}

	end = rlen / sizeof(__be32);

	for (offset=0; offset<end; offset+=(1+CELL_SIZE*3)) {
		u32 flag;
		u64 pci_addr;
		u64 cpu_addr;
		u64 range_size;

		flag = of_read_number(cell+offset, 1);
		pci_addr = of_read_number(cell+offset+1, CELL_SIZE);
		cpu_addr = of_read_number(cell+offset+1+CELL_SIZE, CELL_SIZE);
		range_size = of_read_number(cell+offset+1+CELL_SIZE*2, CELL_SIZE);
		printk("%s: offset %d, flag 0x%x, pci addr 0x%llx, cpu addr 0x%llx, size 0x%llx\r\n",
			__func__, offset, flag, pci_addr, cpu_addr, range_size);

		if (flag & 0x01) {	// APB
			chip->rc.phys_map[ATU_MAP_APB].type = ATU_MAP_APB;
			chip->rc.phys_map[ATU_MAP_APB].addr = pci_addr;
			chip->rc.phys_map[ATU_MAP_APB].size = range_size;
		} else {		// MAU
			chip->rc.phys_map[ATU_MAP_MAU].type = ATU_MAP_MAU;
			chip->rc.phys_map[ATU_MAP_MAU].addr = pci_addr;
			chip->rc.phys_map[ATU_MAP_MAU].size = range_size;
		}
	}

	return 0;
}

static int load_ep_inbound_dts(struct nvt_pcie_chip *chip)
{
	int idx;
	int rlen, offset, end;
#ifdef CONFIG_OF
        struct device_node* of_node = of_find_node_by_path("/nvt_ep0");
#endif
	const __be32 *cell;
	const int CELL_SIZE = 2;

#ifdef CONFIG_OF
        if (!of_node) {
		printk("%s: ep dts not found\r\n", __func__);
		return -ENOMEM;
        }
#endif

	cell = of_get_property(of_node, "reg", &rlen);
	if (cell == NULL) {
		printk("%s: property reg NOT found\r\n", __func__);
	} else {
		printk("%s: property reg len %d\r\n", __func__, rlen);
	}

	end = rlen / sizeof(__be32);

	for (offset=0, idx=0; offset<end; offset+=(CELL_SIZE*2), idx++) {
		u64 pci_addr;
		u64 cpu_addr = 0;
		u64 range_size;

		pci_addr = of_read_number(cell+offset, CELL_SIZE);
		range_size = of_read_number(cell+offset+CELL_SIZE, CELL_SIZE);
		printk("%s: offset %d, pci addr 0x%llx, cpu addr 0x%llx, size 0x%llx\r\n",
			__func__, offset, pci_addr, cpu_addr, range_size);

		switch (idx) {
		case 0:
			chip->ep.phys_map[ATU_MAP_MAU].type = ATU_MAP_MAU;
			chip->ep.phys_map[ATU_MAP_MAU].addr = pci_addr;
			chip->ep.phys_map[ATU_MAP_MAU].size = range_size;
			break;
		case 2:
			chip->ep.phys_map[ATU_MAP_APB].type = ATU_MAP_APB;
			chip->ep.phys_map[ATU_MAP_APB].addr = pci_addr;
			chip->ep.phys_map[ATU_MAP_APB].size = range_size;
			break;
		default:
			break;
		}
	}

	return 0;
}

static int nvt_pcie_probe(struct platform_device *pdev)
{
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = NULL;
	struct resource *res = NULL;
	int ret = 0, i = 0;

	nvt_pcie_chip_ptr = kzalloc(sizeof(struct nvt_pcie_chip), GFP_KERNEL);
	if (!nvt_pcie_chip_ptr)
		goto out_free;

	nvt_pcie_chip_ptr->is_rc =	true;
	nvt_pcie_chip_ptr->dev =	&pdev->dev;

	if (load_rc_inbound_dts(nvt_pcie_chip_ptr) != 0) {
		printk("%s: load rc inbound dts fail\r\n", __func__);
		goto out_free;
	}
	if (load_ep_inbound_dts(nvt_pcie_chip_ptr) != 0) {
		printk("%s: load ep inbound dts fail\r\n", __func__);
		goto out_free;
	}

	nvt_pcie_chip_ptr->va_msix_dummy_addr = dma_alloc_coherent(&pdev->dev,
			NR_HW_IRQ_TOTAL*4,
			&nvt_pcie_chip_ptr->dma_msix_dummy_addr, GFP_KERNEL);
	if (nvt_pcie_chip_ptr->va_msix_dummy_addr == NULL) {
		printk("%s: alloc msix dummy buffer fail\r\n", __func__);
		goto out_free;
	}

	for (i=0; i<MEM_DTS_COUNT; i++) {
		if (pdev->resource->flags == IORESOURCE_MEM) {
		/* setup resource */
			printk("%s: IOMEM found\r\n", __func__);
			res = platform_get_resource(pdev, IORESOURCE_MEM, i);
			if (unlikely(!res)) {
				printk("%s, %s fails: platform_get_resource IORESOURCE_MEM not OK", pdev->name, __FUNCTION__);
				ret = -ENXIO;
				goto out_free;
			}
		}
		nvt_pcie_chip_ptr->res[i] = res;
		printk("%s: res %d start 0x%llx\n", __func__, i, res->start);

		/* reserve the memory region */
		if (!request_mem_region(res->start, resource_size(res),
								pdev->name)) {
				dev_err(&pdev->dev,
						"Unable to get memory/io address region 0x%08llx\n",
						res->start);
				ret = -EBUSY;
				goto req_mem_err;
		}

		nvt_pcie_chip_ptr->PCIE_REG_BASE[i] = ioremap(res->start, resource_size(res));
		if (unlikely(nvt_pcie_chip_ptr->PCIE_REG_BASE[i] == 0)) {
			printk("%s fails: ioremap fail\n", __FUNCTION__);
			ret = -EIO;
			goto remap_err;
		}
	}

	for (i=0; i<MEM_PRIVATE_COUNT; i++) {
		/* reserve the memory region */
		printk("%s: priv %s start 0x%llx\n", __func__, ep_regions[i].name, ep_regions[i].addr);
		if (!request_mem_region(ep_regions[i].addr, ep_regions[i].size,
								pdev->name)) {
				dev_err(&pdev->dev,
						"Unable to get %s memory/io address region 0x%08llx\n",
						ep_regions[i].name,
						ep_regions[i].addr);
				ret = -EBUSY;
				goto req_mem_err;
		}

		nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i] = ioremap(ep_regions[i].addr, ep_regions[i].size);
		if (unlikely(nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i] == 0)) {
			printk("%s fails: ioremap fail\n", __FUNCTION__);
			ret = -EIO;
			goto remap_err;
		}
	}

	setup_controller_hdl(nvt_pcie_chip_ptr);

	nvt_pcie_chip_ptr->irq = platform_get_irq(pdev, 0);
	printk("%s: get irq %d\n", __func__, nvt_pcie_chip_ptr->irq);
#if PCIE_EDMA_INT_EN
	/* register IRQ here*/
	if (request_irq(nvt_pcie_chip_ptr->irq , nvt_pcie_drv_isr, IRQF_TRIGGER_HIGH, "pcie_irq", nvt_pcie_chip_ptr)) {
		//nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[0]);
		printk("failregister an IRQ Int:%d\n", nvt_pcie_chip_ptr->irq);
		ret = -ENODEV;
		goto out_free_irq;
	}
	printk("%s: get irq %d done\n", __func__, nvt_pcie_chip_ptr->irq);
#endif
	/* Basic data structure initialization */
	/* irq structure init. */
	nvt_pcie_chip_ptr->irq_summary	= NR_HW_IRQ_TOTAL;
	/* To store the GIC IRQ num */
	for (i = 0; i < NR_HW_IRQS; i++)
	{
		nvt_pcie_chip_ptr->gic_irq_num[i] = platform_get_irq(pdev, i+1);
		printk("%s: get msix INT %d\r\n", __func__, nvt_pcie_chip_ptr->gic_irq_num[i]);
		if (nvt_pcie_chip_ptr->gic_irq_num[i] < 0) {
			dev_err(&pdev->dev, "Cannot translate IRQ index %d\n",
				nvt_pcie_chip_ptr->gic_irq_num[i]);
			ret = nvt_pcie_chip_ptr->gic_irq_num[i];
			goto getirq_err;
		}
	}

	/* Initialize the lock before use to match the new kernel flow */
	spin_lock_init(&nvt_pcie_chip_ptr->lock);

	/* Initialize proc files */
	ret = nvt_pcie_proc_init(nvt_pcie_chip_ptr);
	if (ret < 0)
		goto proc_err;

	/* Initialize controllers */
	init_msix_table(nvt_pcie_chip_ptr);	// init PCIE MSIX table

	msix_open(nvt_pcie_chip_ptr, MSIx_SEL_TYPE_AXICMD, MSIX_PCIE_MATCH_ADDR&(BIT_MASK(32)-1), MSIX_PCIE_MATCH_ADDR>>32);

	/* To install PCIE IRQ chip */
	ret = nvt_pcie_install_irq_chip(nvt_pcie_chip_ptr);
	if (ret < 0)
		goto out_free_irq;

	// fill info
	nvt_pcie_gen_info(nvt_pcie_chip_ptr);

       //edma init
	nvt_pcie_edma_init(nvt_pcie_chip_ptr,0, 0);

	// tba init
	nvt_pcie_tba_init(nvt_pcie_chip_ptr,0, 0);

	platform_set_drvdata(pdev, nvt_pcie_chip_ptr);

	dev_info(&pdev->dev, "Register %s successfully\n", __func__);
	return 0;

out_free_irq:
proc_err:
	nvt_pcie_proc_remove(nvt_pcie_chip_ptr);
getirq_err:
	for (i=0; i<MEM_COUNT; i++) {
		if (nvt_pcie_chip_ptr->PCIE_REG_BASE[i] != NULL) {
			iounmap(nvt_pcie_chip_ptr->PCIE_REG_BASE[i]);
			nvt_pcie_chip_ptr->PCIE_REG_BASE[i] = NULL;
		}
	}

remap_err:
	for (i=0; i<MEM_DTS_COUNT; i++) {
		res = nvt_pcie_chip_ptr->res[i];
		if (res != NULL) {
			release_mem_region(res->start, (res->end - res->start + 1));
			nvt_pcie_chip_ptr->res[i]= NULL;
		}
	}

req_mem_err:
out_free:
	if (nvt_pcie_chip_ptr->va_msix_dummy_addr) {
		dma_free_coherent(&pdev->dev, NR_HW_IRQ_TOTAL*4,
				nvt_pcie_chip_ptr->va_msix_dummy_addr,
				nvt_pcie_chip_ptr->dma_msix_dummy_addr);
	}
	kfree(nvt_pcie_chip_ptr);

	return ret;
}

static int nvt_pcie_remove(struct platform_device *pdev)
{
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = platform_get_drvdata(pdev);
	struct resource *res;
	int i;

	nvt_pcie_proc_remove(nvt_pcie_chip_ptr);

	for (i=0; i<MEM_DTS_COUNT; i++) {
		if (nvt_pcie_chip_ptr->PCIE_REG_BASE[i] != NULL) {
			iounmap(nvt_pcie_chip_ptr->PCIE_REG_BASE[i]);
			nvt_pcie_chip_ptr->PCIE_REG_BASE[i] = NULL;
		}

		res = nvt_pcie_chip_ptr->res[i];
		if (res != NULL) {
			release_mem_region(res->start, (res->end - res->start + 1));
			nvt_pcie_chip_ptr->res[i] = NULL;
		}
	}
	for (i=0; i<MEM_PRIVATE_COUNT; i++) {
		if (nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i] != NULL) {
			iounmap(nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i]);
			nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i] = NULL;
		}
	}
	if (nvt_pcie_chip_ptr->va_msix_dummy_addr) {
		dma_free_coherent(&pdev->dev, NR_HW_IRQ_TOTAL*4,
				nvt_pcie_chip_ptr->va_msix_dummy_addr,
				nvt_pcie_chip_ptr->dma_msix_dummy_addr);
	}
	platform_set_drvdata(pdev, NULL);
	kfree(nvt_pcie_chip_ptr);
	return 0;
}

#ifdef CONFIG_PM
static int nvt_pcie_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = platform_get_drvdata(pdev);

	return 0;
}
static int nvt_pcie_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = platform_get_drvdata(pdev);

	return 0;
}
static const struct dev_pm_ops nvt_pcie_pmops = {
	.suspend	= nvt_pcie_suspend,
	.resume		= nvt_pcie_resume,
};
#define NVT_PCIE_PMOPS &nvt_pcie_pmops
#else
#define NVT_PCIE_PMOPS NULL
#endif /* CONFIG_PM */

#ifdef CONFIG_OF
static const struct of_device_id nvt_pcie_pin_match[] = {
	{ .compatible = "nvt,nvt_pcie_drv" },
	{},
};

MODULE_DEVICE_TABLE(of, nvt_pcie_pin_match);
#endif

// report status to pcie-drv-wrap
int is_nvt_pcie_init(void)
{
	return is_init;
}

static struct platform_driver nvt_pcie_drv = {
	.probe		= nvt_pcie_probe,
	.remove		= nvt_pcie_remove,
	.driver		= {
		.name	= "nvt_pcie_drv",
		.owner	= THIS_MODULE,
		.pm = NVT_PCIE_PMOPS,
#ifdef CONFIG_OF
		.of_match_table = nvt_pcie_pin_match,
#endif
	},
};

static int __init nvt_pcie_init(void)
{
	struct nvt_pcie_info *p_info;

	p_info = nvt_pcie_get_info();
	if (p_info) {
		memset(p_info, 0, sizeof(struct nvt_pcie_info));
	}

	is_init = 1;

	return platform_driver_register(&nvt_pcie_drv);
}

static void __exit nvt_pcie_exit(void)
{
	platform_driver_unregister(&nvt_pcie_drv);
}

arch_initcall(nvt_pcie_init);
module_exit(nvt_pcie_exit);

MODULE_AUTHOR("Novatek Microelectronics Corp.");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("NVT PCIE driver for nvt na51090 SOC");
MODULE_VERSION(DRV_VERSION);
