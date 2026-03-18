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
#include <linux/soc/nvt/nvt-info.h>

#include <plat/hardware.h>

#define DRV_VERSION             "1.00.002"

#define NR_HW_GIC_IRQ           1   /* To define the source GIC irq number which is used to expend the GIC and pcie msix interrupts */
#define NR_HW_BITS_PER_GROUP    32
#define NR_HW_GROUP             32

#define RCV_PENDING_GROUP_BITMAP_REG_OFS            0x4     //Receive Pending Word Register
#define RCV_AXI_CMD_GROUP_N_BIT_M_SET_EN_OFS        0x100
#define RCV_AXI_CMD_GROUP_N_BIT_M_CLR_EN_OFS        0x200
#define RCV_AXI_CMD_GROUP_N_BIT_M_SET_PENDING_OFS   0x300
#define RCV_AXI_CMD_GROUP_N_BIT_M_CLR_PENDING_OFS   0x400

#define DOORBELL_SIMULATION 1 //0: real doorbell, 1: simulation by 336

#if 0
#define DBG_IND(fmt, args...) printk("%s() "fmt, __func__, ##args)
#else
#define DBG_IND(fmt, args...)
#endif

struct nvt_doorbell_irq_data {
	struct device *dev;

	void __iomem *base;
	struct irq_domain *domain;
	struct irq_chip irq_chip;

	int gic_irq_num[NR_HW_GIC_IRQ];
	unsigned int *reg_data;

	spinlock_t lock;
};

/* functions for doorbell receive control */
static u32 __doorbell_get_pending_group_bitmap(struct nvt_doorbell_irq_data *chip_data)
{
	u32 val;
	val = readl(chip_data->base + RCV_PENDING_GROUP_BITMAP_REG_OFS);
	return val;
}

static u32 __doorbell_get_pending_irq(struct nvt_doorbell_irq_data *chip_data, int group)
{
	u32 val;
	val = readl(chip_data->base + RCV_AXI_CMD_GROUP_N_BIT_M_SET_PENDING_OFS + (group << 2));
	return val;
}

static void __doorbell_enable_irq(struct nvt_doorbell_irq_data *chip_data, int group, int number)
{
	writel(number, chip_data->base + RCV_AXI_CMD_GROUP_N_BIT_M_SET_EN_OFS + (group << 2));
}

static void __doorbell_clr_pending_irq(struct nvt_doorbell_irq_data *chip_data, int group, int number)
{
	writel(number, chip_data->base + RCV_AXI_CMD_GROUP_N_BIT_M_CLR_PENDING_OFS + (group << 2));
}

static void __doorbell_disable_irq(struct nvt_doorbell_irq_data *chip_data, int group, int number)
{
	writel(number, chip_data->base + RCV_AXI_CMD_GROUP_N_BIT_M_CLR_EN_OFS + (group << 2));
}

/* callbacks registered to kernel */

static void nvt_doorbell_summary_irq_handler(struct irq_desc *desc)
{
	unsigned int irq = irq_desc_get_irq(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct nvt_doorbell_irq_data *irq_data;
	unsigned int group_idx;
	unsigned int bit_idx;
	unsigned int group_bit; //pending group bitmap
	unsigned int intr_bit; //pending interrupt bitmap
	irq_hw_number_t hwirq;
	unsigned int virq;

	irq_data = irq_get_handler_data(irq);
	if (NULL == irq_data) {
		DBG_IND("irq_get_handler_data failed, irq %d\r\n", irq);
		return;
	}

	group_bit = __doorbell_get_pending_group_bitmap(irq_data);
	DBG_IND("group_bit 0x%x\r\n", group_bit);

	for (group_idx = 0; group_idx < NR_HW_GROUP; group_idx++) {
		if (0 == (group_bit & (1U << group_idx)))
			continue;

		intr_bit = __doorbell_get_pending_irq(irq_data, group_idx);
		DBG_IND("group_idx[%d] intr_bit 0x%x\r\n", group_idx, intr_bit);

		for (bit_idx = 0; bit_idx < 32; bit_idx++) {
			if (0 == (intr_bit & (1U << bit_idx)))
				continue;

			hwirq = bit_idx + (group_idx * NR_HW_BITS_PER_GROUP);

			chained_irq_enter(chip, desc);

			virq = irq_find_mapping(irq_data->domain, hwirq);
			DBG_IND("hwirq(%ld) -> virq(%d) found\r\n", hwirq, virq);

			generic_handle_irq(virq);

			chained_irq_exit(chip, desc);
		}
	}
}

static void doorbell_unmask_irq(struct irq_data *data)
{
	struct nvt_doorbell_irq_data *chip_data = irq_data_get_irq_chip_data(data);
	unsigned long flags;
	int group;
	int bit;

	spin_lock_irqsave(&chip_data->lock, flags);

	group = data->hwirq / NR_HW_BITS_PER_GROUP;
	bit = 1 << (data->hwirq % NR_HW_BITS_PER_GROUP);
	__doorbell_enable_irq(chip_data, group, bit);

	spin_unlock_irqrestore(&chip_data->lock, flags);
}

static void doorbell_mask_irq(struct irq_data *data)
{
	struct nvt_doorbell_irq_data *chip_data = irq_data_get_irq_chip_data(data);
	unsigned long flags;
	int group;
	int bit;

	spin_lock_irqsave(&chip_data->lock, flags);

	group = data->hwirq / NR_HW_BITS_PER_GROUP;
	bit = 1 << (data->hwirq % NR_HW_BITS_PER_GROUP);
	__doorbell_disable_irq(chip_data, group, bit);

	spin_unlock_irqrestore(&chip_data->lock, flags);
}

static void doorbell_irq_ack(struct irq_data *data)
{
	struct nvt_doorbell_irq_data *chip_data = irq_data_get_irq_chip_data(data);
	unsigned long flags;
	int group;
	int bit;

	spin_lock_irqsave(&chip_data->lock, flags);

	group = data->hwirq / NR_HW_BITS_PER_GROUP;
	bit = 1 << (data->hwirq % NR_HW_BITS_PER_GROUP);
	__doorbell_clr_pending_irq(chip_data, group, bit);

	spin_unlock_irqrestore(&chip_data->lock, flags);
}

static struct irq_chip nvt_doorbell_irq_chip = {
	.name = "nvt_doorbell_irqc",
	.irq_enable = doorbell_unmask_irq,
	.irq_disable = doorbell_mask_irq,
	.irq_mask = doorbell_mask_irq,
	.irq_unmask = doorbell_unmask_irq,
	.irq_ack = doorbell_irq_ack,
};

static int nvt_doorbell_intx_map(struct irq_domain *domain, unsigned int irq,
			     irq_hw_number_t hwirq)
{
	irq_set_chip_and_handler(irq, &nvt_doorbell_irq_chip, handle_edge_irq);
	irq_set_chip_data(irq, domain->host_data);

	return 0;
}

static int nvt_doorbell_intx_translate(struct irq_domain *d,
				      struct irq_fwspec *fwspec,
				      unsigned long *hwirq,
				      unsigned int *type)
{
	struct nvt_doorbell_irq_data *data = d->host_data;

	/* We only support in RC mode: interrupts = <CHIP_RC 0 IRQ_TYPE_LEVEL_HIGH>; */
	if (is_of_node(fwspec->fwnode)) {

		// compatible with GIC 3-tupples description
		if (fwspec->param_count < 3) {
			dev_err(data->dev,"param_count %d < 3\r\n", fwspec->param_count);
			return -EINVAL;
		}

		DBG_IND("param[0] %d, param[1] %d, param[2] %d\r\n",
			fwspec->param[0], fwspec->param[1], fwspec->param[2]);

		if (fwspec->param[0] != 0) {
			dev_err(data->dev,"param[0] %d != 0\r\n", fwspec->param[0]);
			return -EINVAL;
		}

		*hwirq = fwspec->param[1];
		*type = IRQ_TYPE_NONE;
	}

	return 0;
}

static const struct irq_domain_ops nvt_doorbell_domain_ops = {
	.map = nvt_doorbell_intx_map,
	.translate = nvt_doorbell_intx_translate,
};

static int nvt_doorbell_install_irq_chip(struct platform_device *pdev, struct nvt_doorbell_irq_data *data)
{
	int n = 0;

	/* To create IRQ domain with total expended irq number */
	data->domain = irq_domain_add_tree(data->dev->of_node,
						  &nvt_doorbell_domain_ops, data);
	if (!data->domain)
		return -ENOMEM;

	/*
	 * Assign handler to hw irq which is used to connect ARM GIC
	 * Once HWIRQ is triggered, this handler will be called.
	 */
	for (n = 0; n < NR_HW_GIC_IRQ; n++) {
		dev_info(&pdev->dev, "install %d isr\r\n", data->gic_irq_num[n]);
		irq_set_chained_handler_and_data(data->gic_irq_num[n], nvt_doorbell_summary_irq_handler, data);
	}

	return 0;
}

static int nvt_doorbell_probe(struct platform_device *pdev)
{
	struct nvt_doorbell_irq_data *data;
	struct resource *res;
	int i;

	dev_info(&pdev->dev, "probe start\r\n");

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		return -ENOMEM;
	}
	data->dev = &pdev->dev;

	//coverity[side_effect_free]: spin_lock_init is kernel API, do not change
	spin_lock_init(&data->lock);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "get res failed\r\n");
		return -ENXIO;
	}
	dev_info(&pdev->dev, "res start 0x%llx\r\n", res->start);

	data->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(data->base)) {
		return PTR_ERR(data->base);
	}

	for (i = 0; i < NR_HW_GIC_IRQ; i++) {
		data->gic_irq_num[i] = platform_get_irq(pdev, i);

		if (data->gic_irq_num[i] < 0) {
			dev_err(&pdev->dev, "failed to platform_get_irq[%d]\n", i);
			return -EINVAL;
		}
	}

	nvt_doorbell_install_irq_chip(pdev, data);

	platform_set_drvdata(pdev, data);

	dev_info(&pdev->dev, "probe end\r\n");

	return 0;
}

static int nvt_doorbell_remove(struct platform_device *pdev)
{
	struct nvt_doorbell_irq_data *data = platform_get_drvdata(pdev);

	if (data->reg_data)
		kfree(data->reg_data);

	irq_domain_remove(data->domain);

	return 0;
}

#ifdef CONFIG_PM
static int nvt_doorbell_suspend(struct device *ddev)
{
	int index;
	struct platform_device *pdev;
	struct resource *res;
	struct nvt_doorbell_irq_data *data = dev_get_drvdata(ddev);

	if (!nvt_get_suspend_mode())
		return 0;

	pdev = container_of(ddev, struct platform_device, dev);
	if (!pdev) {
		pr_err("%s, get pdev fail\n", __func__);
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(!res)) {
		dev_err(&pdev->dev, "%s fails: platform_get_resource IORESOURCE_MEM not OK", __FUNCTION__);
		return -ENXIO;
	}

	if (!data->reg_data) {
		data->reg_data = kzalloc(resource_size(res), GFP_KERNEL);
		if (!data->reg_data) {
			dev_err(&pdev->dev, "alloc fail");
			return -ENOMEM;
		}
	}

	for (index = 0; index < resource_size(res); index += sizeof(unsigned int)) {
		data->reg_data[index] = readl((void*)(data->base + index));
	}

	return 0;
}

static int nvt_doorbell_resume(struct device *ddev)
{
	int index;
	struct platform_device *pdev;
	struct resource *res;
	struct nvt_doorbell_irq_data *data = dev_get_drvdata(ddev);

	if (!nvt_get_suspend_mode())
		return 0;

	pdev = container_of(ddev, struct platform_device, dev);
	if (!pdev) {
		pr_err("%s, get pdev fail\n", __func__);
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(!res)) {
		dev_err(&pdev->dev, "%s fails: platform_get_resource IORESOURCE_MEM not OK", __FUNCTION__);
		return -ENXIO;
	}

	if (!data->reg_data) {
		dev_err(&pdev->dev, "gpio_data is NULL\n");
		return -EINVAL;
	}

	for (index = 0; index < resource_size(res); index += sizeof(unsigned int)) {
		writel(data->reg_data[index], (void*)(data->base + index));
	}

	return 0;
}

static const struct dev_pm_ops nvt_doorbell_pm_ops = {
	.suspend  = nvt_doorbell_suspend,
	.resume = nvt_doorbell_resume,
};
#endif

static const struct of_device_id nvt_doorbell_of_match[] = {
	{ .compatible = "nvt,nvt_doorbell", },
	{},
};
MODULE_DEVICE_TABLE(of, nvt_doorbell_of_match);

static struct platform_driver nvt_doorbell_driver = {
	.probe  = nvt_doorbell_probe,
	.remove = nvt_doorbell_remove,
	.driver = {
		.name = "nvt_doorbell_platdrv",
		.of_match_table = nvt_doorbell_of_match,
#ifdef CONFIG_PM
		.pm = &nvt_doorbell_pm_ops,
#endif
	},
};
module_platform_driver(nvt_doorbell_driver);

MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL v2");
