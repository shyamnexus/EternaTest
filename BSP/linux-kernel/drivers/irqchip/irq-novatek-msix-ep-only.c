/*
 * MSIX driver for Novatek RC/EP interworking
 *
 * Copyright (c) 2021 - Novatek Microelectronics Corp.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/bitfield.h>

#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/soc/nvt/nvt-pci.h>

#include <plat/hardware.h>

#include "irq-novatek-msix-ep-only.h"

#define DRV_VERSION		"1.00.000"

#define NR_HW_IRQS		32				/* To define the source GIC irq number which is used to expend the GIC and pcie msix interrupts */
#define NR_HW_IRQ_GROP_NUM	32
#define NR_HW_IRQ_TOTAL		(NR_HW_IRQS * NR_HW_IRQ_GROP_NUM)

#define MAX_INT_PER_EP		(128)

#define IRQ_MASK        0x4
#define IRQ_STATUS      0x8

struct nvt_msix_irq_data {
	struct device		*dev;

	void __iomem		*base;
	struct irq_domain	*domain;
	struct irq_chip		irq_chip;

	int			gic_irq_num[NR_HW_IRQS];

	spinlock_t		lock;
};

struct nvt_ep_data {
	struct pci_dev *pci_dev;
	void __iomem *apb_va;		// RC outbound to access EP APB (VA)
	unsigned long ep_out_apb_pa;	// EP outbound to access RC APB (PA)
};

/* functions for local (RC) msix recive control */
static u32 local_msix_get_pending_irq(struct nvt_msix_irq_data *chip_data, int group)
{
	u32 val;

	val = readl(chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_PENDING_OFS + (group<<2));
	return val;
}

static u32 local_msix_get_enabled_irq(struct nvt_msix_irq_data *chip_data, int group)
{
	u32 val;

	val = readl(chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_EN_OFS + (group<<2));
	return val;
}

static void local_msix_enable_irq(struct nvt_msix_irq_data *chip_data, int group, int number)
{
	writel(number, chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_EN_OFS + (group<<2));
}

static void local_msix_clr_pending_irq(struct nvt_msix_irq_data *chip_data, int group, int number)
{
	writel(number, chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_PENDING_OFS + (group<<2));
}

static void local_msix_disable_irq(struct nvt_msix_irq_data *chip_data, int group, int number)
{
	writel(number, chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_EN_OFS + (group<<2));
}

/* callbacks registered to kernel */

static void nvt_msix_int_get_status(struct nvt_msix_irq_data *chip_data, unsigned long *pending_int)
{
	int n;

	for (n = 0; n < NR_HW_IRQS; n++) {
		pending_int[n] = local_msix_get_pending_irq(chip_data, n) & local_msix_get_enabled_irq(chip_data, n);
	}

	return;
}

static int nvt_msix_chk_pending_dummy_data(struct nvt_msix_irq_data *chip_data, int number)
{
	return 1;
}

static void nvt_msix_summary_irq_handler(struct irq_desc *desc)
{
	unsigned int irq = irq_desc_get_irq(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct nvt_msix_irq_data *chip_data = irq_get_handler_data(irq);
	unsigned int virq = 0, bit = 0, n = 0;
	unsigned long pending_int[NR_HW_IRQS];
	const unsigned int gic_offset = irq - chip_data->gic_irq_num[0];

//	printk("%s: enter, parent irq %d, data irq %d, base irq %d\r\n", __func__, desc->parent_irq, irq, chip_data->gic_irq_num[0]);
	/* To read the interrupt status to find which pin is trigger: msi_int_status */
	nvt_msix_int_get_status(chip_data, pending_int);

	/* mack irq */
	chained_irq_enter(chip, desc);

	/* To find PCIE MSIX which pin is triggered */
	bit = gic_offset;
	for (n = 0; n < NR_HW_IRQS; n++)
	{
//		printk("%s: HW GROUP %d, pending 0x%lx\r\n", __func__, n, pending_int[n]);
		if (pending_int[n] & (1<<bit)) {
//			printk("%s: HW GROUP %d, bit %d\r\n", __func__, n, bit);
			nvt_msix_chk_pending_dummy_data(chip_data, bit + (n * NR_HW_IRQ_GROP_NUM));
			/* To find the virtual irq number */
			virq = irq_find_mapping(chip_data->domain, bit + (n * NR_HW_IRQ_GROP_NUM));
//			printk("%s: virq %d found\r\n", __func__, virq);
			/* Assign virq to handle irq isr */
			generic_handle_irq(virq);
		}
	}

	/* unmask irq */
	chained_irq_exit(chip, desc);
}

static void msix_unmask_irq(struct irq_data *data)
{
	int ep_id;
	struct nvt_msix_irq_data *chip_data = irq_data_get_irq_chip_data(data);
	unsigned long flags;

	ep_id = data->hwirq / MAX_INT_PER_EP;

//	printk("%s: ep %d irq %ld, group %ld, bit %lu\r\n", __func__, ep_id, data->hwirq, data->hwirq/NR_HW_IRQ_GROP_NUM, data->hwirq%NR_HW_IRQ_GROP_NUM);

	spin_lock_irqsave(&chip_data->lock, flags);

	// unmask local msix-rx irq
	local_msix_enable_irq(chip_data, data->hwirq/NR_HW_IRQ_GROP_NUM, 1<<(data->hwirq%NR_HW_IRQ_GROP_NUM));

	spin_unlock_irqrestore(&chip_data->lock, flags);
}

static void msix_mask_irq(struct irq_data *data)
{
	int ep_id;
	struct nvt_msix_irq_data *chip_data = irq_data_get_irq_chip_data(data);
	unsigned long flags;

	ep_id = data->hwirq / MAX_INT_PER_EP;

//	printk("%s: ep %d irq %ld, group %ld, bit %lu\r\n", __func__, ep_id, data->hwirq, data->hwirq/NR_HW_IRQ_GROP_NUM, data->hwirq%NR_HW_IRQ_GROP_NUM);

	spin_lock_irqsave(&chip_data->lock, flags);

	// mask local msix-rx irq
	local_msix_disable_irq(chip_data, data->hwirq/NR_HW_IRQ_GROP_NUM, 1<<(data->hwirq%NR_HW_IRQ_GROP_NUM));

	spin_unlock_irqrestore(&chip_data->lock, flags);
}

static void msix_irq_ack(struct irq_data *data)
{
	int ep_id;
	struct nvt_msix_irq_data *chip_data = irq_data_get_irq_chip_data(data);
	unsigned long flags;

	ep_id = data->hwirq / MAX_INT_PER_EP;

//	printk("%s: ep %d hwirq %ld\r\n", __func__, ep_id, data->hwirq);
//	printk("%s: HW GROUP %lu, bit %lu\r\n", __func__, data->hwirq/NR_HW_IRQ_GROP_NUM, data->hwirq%NR_HW_IRQ_GROP_NUM);

	spin_lock_irqsave(&chip_data->lock, flags);

	local_msix_clr_pending_irq(chip_data,
		data->hwirq/NR_HW_IRQ_GROP_NUM,
		1<<(data->hwirq%NR_HW_IRQ_GROP_NUM));

	spin_unlock_irqrestore(&chip_data->lock, flags);
}

static struct irq_chip nvt_msix_irq_chip = {
	.name = "Novatek PCIe MSIX",
	.irq_enable = msix_unmask_irq,
	.irq_disable = msix_mask_irq,
	.irq_mask = msix_mask_irq,
	.irq_unmask = msix_unmask_irq,
	.irq_ack = msix_irq_ack,
};

static int nvt_msix_intx_map(struct irq_domain *domain, unsigned int irq,
			     irq_hw_number_t hwirq)
{
	irq_set_chip_and_handler(irq, &nvt_msix_irq_chip, handle_edge_irq);
	irq_set_chip_data(irq, domain->host_data);

	return 0;
}

static int nvt_msix_intx_translate(struct irq_domain *d,
				      struct irq_fwspec *fwspec,
				      unsigned long *hwirq,
				      unsigned int *type)
{
	struct nvt_msix_irq_data *data = d->host_data;

	/* We only support in RC mode: interrupts = <CHIP_RC 0 IRQ_TYPE_LEVEL_HIGH>; */
	if (is_of_node(fwspec->fwnode)) {

		// compatible with GIC 3-tupples description
		if (fwspec->param_count < 3)
			return -EINVAL;

		if (fwspec->param[0] != CHIP_RC) {
			dev_err(data->dev,"We only support first parameter with RC mode %d\r\n", fwspec->param[0]);
			return -EINVAL;
		}

		*hwirq = fwspec->param[1];
		*type = IRQ_TYPE_NONE;
	}

	return 0;
}

static const struct irq_domain_ops nvt_msix_domain_ops = {
	.map = nvt_msix_intx_map,
	.translate = nvt_msix_intx_translate,
};

static int nvt_pcie_install_irq_chip(struct nvt_msix_irq_data *data)
{
	int n = 0;

	/* To create PCIE MSIX IRQ domain with total expend irq number */
	data->domain = irq_domain_add_tree(data->dev->of_node,
						  &nvt_msix_domain_ops, data);
	if (!data->domain)
		return -ENOMEM;

	/*
	 * Assign handler to PCIE MSIX hw irq which is used to connect ARM GIC
	 * Once HWIRQ is triggered by PCIE MSIX, this handler will be called.
	 */
	for (n = 0; n < NR_HW_IRQS; n++) {
		printk("%s: install %d isr\r\n", __func__, data->gic_irq_num[n]);
		irq_set_chained_handler_and_data(data->gic_irq_num[n], nvt_msix_summary_irq_handler, data);
	}

	return 0;
}

static int nvt_msix_probe(struct platform_device *pdev)
{
	struct nvt_msix_irq_data *data;
	struct resource *res;
	int i;

	dev_info(&pdev->dev, "NVT MSIX Probing...\r\n");

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dev_info(&pdev->dev, "%s: base 0x%llx\r\n", __func__, res->start);
	data->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(data->base))
		return PTR_ERR(data->base);

	spin_lock_init(&data->lock);

	for (i = 0; i < NR_HW_IRQS; i++) {
		data->gic_irq_num[i] = platform_get_irq(pdev, i);

		if (data->gic_irq_num[i] < 0) {
			dev_err(&pdev->dev, "failed to get parent IRQ\n");
			return -EINVAL;
		}
	}

	/* To install PCIE IRQ chip */
	nvt_pcie_install_irq_chip(data);

	platform_set_drvdata(pdev, data);

	dev_info(&pdev->dev, "NVT MSIX probe done\r\n");

	return 0;
}

static int nvt_msix_remove(struct platform_device *pdev)
{
	struct nvt_msix_irq_data *data = platform_get_drvdata(pdev);

	irq_domain_remove(data->domain);

	return 0;
}

static const struct of_device_id nvt_msix_of_match[] = {
	{ .compatible = "nvt,pcie-msix-ep", },
	{},
};
MODULE_DEVICE_TABLE(of, nvt_msix_of_match);

static struct platform_driver nvt_msix_driver = {
	.probe  = nvt_msix_probe,
	.remove = nvt_msix_remove,
	.driver = {
		.name = "nvt-pcie-msix-irqchip-ep",
		.of_match_table = nvt_msix_of_match,
	},
};
module_platform_driver(nvt_msix_driver);

MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL v2");