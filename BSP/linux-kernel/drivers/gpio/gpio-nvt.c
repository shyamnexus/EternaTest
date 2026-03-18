/**
    NVT GPIO driver
    To handle NVT GPIO interrupt extender and GPIO driver
    @file nvt-gpio.c
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
#include <linux/soc/nvt/nvt-io.h>
#include <linux/soc/nvt/rcw_macro.h>
#include <linux/soc/nvt/nvt-info.h>
#include <plat/nvt-gpio-internal.h>

#define DRV_VERSION		"2.00.012"

#ifdef CONFIG_NVT_CC
#include <plat/nvt_cc.h>
#define loc_cpu(lock, flags)							\
	do {									\
		spin_lock_irqsave(lock, flags);					\
		cc_hwlock_resource(CC_CORE_CPU1, CC_RESOURCE_GPIO);	\
	} while (0)

#define unl_cpu(lock, flags)							\
	do {									\
		cc_hwunlock_resource(CC_CORE_CPU1, CC_RESOURCE_GPIO);	\
		spin_unlock_irqrestore(lock, flags);				\
	} while (0)

#else
#define loc_cpu(lock, flags)							\
	do {									\
		spin_lock_irqsave(lock, flags);					\
	} while (0)

#define unl_cpu(lock, flags)							\
	do {									\
		spin_unlock_irqrestore(lock, flags);				\
	} while (0)
#endif /* CONFIG_NVT_CC */


#define GPIO_SETREG(chip, ofs,value)	OUTW(chip->reg_base +(ofs),(value))
#define GPIO_GETREG(chip, ofs)		INW(chip->reg_base+(ofs))

/* GPIO pin and interrupt control structures */
struct nvt_gpio_chip {
	struct device                   *dev;
	void __iomem                    *reg_base;
	struct irq_domain               *domain;
	unsigned int                    irq_base[GPIO_IRQ_NUM];           /* Shared IRQ: GIC to GPIO IRQ */
	unsigned int                    irq_summary;                      /* Total IRQ numbers */
	unsigned long                   irq_request_record[NVT_GPIO_TO_CPU_TOTAL]; /* To record GPIO irq domain int ID 0~127 */
	struct irq_chip                 irq_chip;
	unsigned long                   int_status[NVT_GPIO_TO_CPU_TOTAL];         /* Interrupt status GPIO_STS_CPUx_x */
	spinlock_t                      lock;
	struct gpio_chip                gpio_chip;                        /* To abstract gpio controller */
	int                             is_ep;
	int                             ep_num;
	unsigned int			gpio_data[NVT_GPIO_TOTAL_SIZE];
};

static void nvt_gpio_int_pin_init(struct nvt_gpio_chip *chip, u8 hw_irq_num, u8 type, u8 pol, u8 edge_type)
{
	unsigned long reg = 0;
	unsigned long ofs = (hw_irq_num >> 5) << 2;

	hw_irq_num &= (32 - 1);

	/* To configure the int type (Edge trigger=0 or Level trigger=1) */
	if (type) {
		reg = GPIO_GETREG(chip, NVT_GPIO_INT_TYPE + ofs);
		reg |= (1 << hw_irq_num);
		GPIO_SETREG(chip, NVT_GPIO_INT_TYPE + ofs, reg);
	} else {
		reg = GPIO_GETREG(chip, NVT_GPIO_INT_TYPE + ofs);
		reg &= ~(1 << hw_irq_num);
		GPIO_SETREG(chip, NVT_GPIO_INT_TYPE + ofs, reg);
	}

	/* To configure the polarity (Positive=0 or Negative=1) */
	if (pol) {
		reg = GPIO_GETREG(chip, NVT_GPIO_INT_POL + ofs);
		reg |= (1 << hw_irq_num);
		GPIO_SETREG(chip, NVT_GPIO_INT_POL + ofs, reg);
	} else {
		reg = GPIO_GETREG(chip, NVT_GPIO_INT_POL + ofs);
		reg &= ~(1 << hw_irq_num);
		GPIO_SETREG(chip, NVT_GPIO_INT_POL + ofs, reg);
	}

	/* To configure the polarity (Both edge) */
	if (edge_type) {
		reg = GPIO_GETREG(chip, NVT_GPIO_EDGE_TYPE + ofs);
		reg |= (1 << hw_irq_num);
		GPIO_SETREG(chip, NVT_GPIO_EDGE_TYPE + ofs, reg);
	} else {
		reg = GPIO_GETREG(chip, NVT_GPIO_EDGE_TYPE + ofs);
		reg &= ~(1 << hw_irq_num);
		GPIO_SETREG(chip, NVT_GPIO_EDGE_TYPE + ofs, reg);
	}

	/* To clear the interrupt status */
	reg = GPIO_GETREG(chip, NVT_GPIO_STS_CPU + ofs);
	reg |= 1 << hw_irq_num;
	GPIO_SETREG(chip, NVT_GPIO_STS_CPU + ofs, reg);

}

static void nvt_gpio_int_get_status(struct nvt_gpio_chip *chip, const struct cpumask *mask)
{
	int i = 0, cpu;

	/* We should check this by gpio irq group number */
	for (i = 0; i < NVT_GPIO_STS_CPU_NUM; i++) {
		for (cpu = 0; cpu < GPIO_IRQ_NUM; cpu++) {
			chip->int_status[i + (cpu * NVT_GPIO_STS_CPU_NUM)] = GPIO_GETREG(chip, NVT_GPIO_STS_CPU + (cpu * NVT_GPIO_STS_CPU_OFFSET) + i * 4);
			/* reuested record is used to save which pin is called by request_irq
			 * and operation with this pin
			 */
			chip->int_status[i + (cpu * NVT_GPIO_STS_CPU_NUM)] &= chip->irq_request_record[i + (cpu * NVT_GPIO_STS_CPU_NUM)];
		}
	}
}

static void nvt_gpio_int_set_status(struct nvt_gpio_chip *chip, unsigned long hw_irq_num)
{
	unsigned long tmp, i;
	unsigned long ofs = (hw_irq_num >> 5) << 2;

	hw_irq_num &= (32 - 1);
	tmp = (1 << hw_irq_num);

	/* Clear the status of edge trigger and level trigger */
	for (i = 0; i < GPIO_IRQ_NUM; i++)  {
		GPIO_SETREG(chip, NVT_GPIO_STS_CPU + (i * NVT_GPIO_STS_CPU_OFFSET) + ofs, tmp);
	}
}

static void nvt_gpio_int_pin_en(struct nvt_gpio_chip *chip, u8 hw_irq_num, u8 en, const struct cpumask *mask)
{
	int cpu, i;
	unsigned long reg = 0;
	unsigned long ofs = (hw_irq_num >> 5) << 2;

	hw_irq_num &= (32 - 1);

	cpu = cpumask_any_and(mask, cpu_online_mask);
	if (en) {
		/* Clear STS before enable IRQ */
		for (i = 0; i < GPIO_IRQ_NUM; i++)  {
			GPIO_SETREG(chip, NVT_GPIO_STS_CPU + (i * NVT_GPIO_STS_CPU_OFFSET) + ofs, (1 << hw_irq_num));
		}

		reg = GPIO_GETREG(chip, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_INTEN_CPU_OFFSET) + ofs);
		reg |= 1 << hw_irq_num;
		GPIO_SETREG(chip, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_INTEN_CPU_OFFSET) + ofs, reg);
	} else {
		reg = GPIO_GETREG(chip, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_INTEN_CPU_OFFSET) + ofs);
		reg &= ~(1 << hw_irq_num);
		GPIO_SETREG(chip, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_INTEN_CPU_OFFSET) + ofs, reg);
	}
}

static void nvt_gpio_irq_mask(struct irq_data *d)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = irq_data_get_irq_chip_data(d);
	struct cpumask *mask = irq_data_get_affinity_mask(d);
	unsigned int hw_irq_num = d->hwirq;
	unsigned long flags = 0;
	unsigned long ofs = (hw_irq_num >> 5);
	hw_irq_num &= (32 - 1);

	spin_lock_irqsave(&nvt_gpio_chip_ptr->lock, flags);
	/* To clear the irq id flag */
	nvt_gpio_chip_ptr->irq_request_record[ofs] &= ~(1 << hw_irq_num);
	/* Disable IRQ */
	nvt_gpio_int_pin_en(nvt_gpio_chip_ptr, d->hwirq, 0, mask);
	spin_unlock_irqrestore(&nvt_gpio_chip_ptr->lock, flags);
}

static void nvt_gpio_irq_unmask(struct irq_data *d)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = irq_data_get_irq_chip_data(d);
	struct cpumask *mask = irq_data_get_affinity_mask(d);
	unsigned int hw_irq_num = d->hwirq;
	unsigned long flags = 0;
	unsigned long ofs = (hw_irq_num >> 5);
	hw_irq_num &= (32 - 1);

	spin_lock_irqsave(&nvt_gpio_chip_ptr->lock, flags);
	/* To set the irq id flag */
	nvt_gpio_chip_ptr->irq_request_record[ofs] |= (1 << hw_irq_num);
	/* Enable IRQ */
	nvt_gpio_int_pin_en(nvt_gpio_chip_ptr, d->hwirq, 1, mask);
	spin_unlock_irqrestore(&nvt_gpio_chip_ptr->lock, flags);
}

static int nvt_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = irq_data_get_irq_chip_data(d);
	/* To get hw irq number (virq [Virtual irq number] <--> hwirq [Related to gpio controller]) */
	int hw_irq_num = d->hwirq;
	unsigned long flags = 0;

	if (type == IRQ_TYPE_LEVEL_LOW) {
		irq_set_handler_locked(d, handle_level_irq);
		loc_cpu(&nvt_gpio_chip_ptr->lock, flags);
		nvt_gpio_int_pin_init(nvt_gpio_chip_ptr, hw_irq_num, 1, 1, 0);
		unl_cpu(&nvt_gpio_chip_ptr->lock, flags);
		dev_info(nvt_gpio_chip_ptr->dev,
			 "HWIRQ: %d IRQ: %d ### Level Low ###\n",
			 hw_irq_num, d->irq);
	} else if (type == IRQ_TYPE_LEVEL_HIGH) {
		irq_set_handler_locked(d, handle_level_irq);
		loc_cpu(&nvt_gpio_chip_ptr->lock, flags);
		nvt_gpio_int_pin_init(nvt_gpio_chip_ptr, hw_irq_num, 1, 0, 0);
		unl_cpu(&nvt_gpio_chip_ptr->lock, flags);
		dev_info(nvt_gpio_chip_ptr->dev,
			 "HWIRQ: %d IRQ: %d ### Level High ###\n",
			 hw_irq_num, d->irq);
	} else if (type == IRQ_TYPE_EDGE_FALLING) {
		irq_set_handler_locked(d, handle_edge_irq);
		loc_cpu(&nvt_gpio_chip_ptr->lock, flags);
		nvt_gpio_int_pin_init(nvt_gpio_chip_ptr, hw_irq_num, 0, 1, 0);
		unl_cpu(&nvt_gpio_chip_ptr->lock, flags);
		dev_info(nvt_gpio_chip_ptr->dev,
			 "HWIRQ: %d IRQ: %d ### Edge falling ###\n",
			 hw_irq_num, d->irq);
	} else if (type == IRQ_TYPE_EDGE_RISING) {
		irq_set_handler_locked(d, handle_edge_irq);
		loc_cpu(&nvt_gpio_chip_ptr->lock, flags);
		nvt_gpio_int_pin_init(nvt_gpio_chip_ptr, hw_irq_num, 0, 0, 0);
		unl_cpu(&nvt_gpio_chip_ptr->lock, flags);
		dev_info(nvt_gpio_chip_ptr->dev,
			 "HWIRQ: %d IRQ: %d ### Edge rising ###\n",
			 hw_irq_num, d->irq);
	} else {
		irq_set_handler_locked(d, handle_edge_irq);
		loc_cpu(&nvt_gpio_chip_ptr->lock, flags);
		nvt_gpio_int_pin_init(nvt_gpio_chip_ptr, hw_irq_num, 0, 0, 1);
		unl_cpu(&nvt_gpio_chip_ptr->lock, flags);
		dev_info(nvt_gpio_chip_ptr->dev,
			 "HWIRQ: %d IRQ: %d ### Both Edge ###\n",
			 hw_irq_num, d->irq);
	}

	return 0;
}

static void nvt_gpio_irq_ack(struct irq_data *d)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = irq_data_get_irq_chip_data(d);

	/* Clear hw irq status */
	nvt_gpio_int_set_status(nvt_gpio_chip_ptr, d->hwirq);
}

static int nvt_gpio_irq_set_affinity(struct irq_data *d, const struct cpumask *mask, bool force)
{

	struct nvt_gpio_chip *nvt_gpio_chip_ptr = irq_data_get_irq_chip_data(d);
	unsigned int hw_irq_num = d->hwirq;
	unsigned long flags = 0;
	int i;
	unsigned int cpu;
	unsigned long reg = 0;
	unsigned long ofs = (hw_irq_num >> 5);
	hw_irq_num &= (32 - 1);

	if (force)
		cpu = cpumask_first(mask);
	else
		cpu = cpumask_any_and(mask, cpu_online_mask);

	if (cpu >= nr_cpu_ids)
		return -EINVAL;

	spin_lock_irqsave(&nvt_gpio_chip_ptr->lock, flags);

	/* To set the irq id flag */
	nvt_gpio_chip_ptr->irq_request_record[ofs] |= (1 << hw_irq_num);

	/* Enable IRQ */
	ofs = (d->hwirq >> 5) << 2;
	for (i = 0; i < GPIO_IRQ_NUM; i++)  {
		reg = GPIO_GETREG(nvt_gpio_chip_ptr, NVT_GPIO_INTEN_CPU + (i * NVT_GPIO_INTEN_CPU_OFFSET) + ofs);
		reg &= ~(1 << hw_irq_num);
		GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_INTEN_CPU + (i * NVT_GPIO_INTEN_CPU_OFFSET) + ofs, reg);
	}
	/* Clear STS before enable IRQ */
	for (i = 0; i < GPIO_IRQ_NUM; i++)  {
		GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_STS_CPU + (i * NVT_GPIO_STS_CPU_OFFSET) + ofs, (1 << hw_irq_num));
	}

	reg = GPIO_GETREG(nvt_gpio_chip_ptr, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_INTEN_CPU_OFFSET) + ofs);
	reg |= 1 << hw_irq_num;
	GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_INTEN_CPU_OFFSET) + ofs, reg);

	spin_unlock_irqrestore(&nvt_gpio_chip_ptr->lock, flags);

	irq_data_update_effective_affinity(d, cpumask_of(cpu));

	return IRQ_SET_MASK_OK;
}

static int nvt_gpio_irq_set_wake(struct irq_data *d, unsigned int state)
{
	return irq_set_irq_wake(d->hwirq, state);
}

static void nvt_summary_irq_handler(struct irq_desc *desc)
{
	unsigned int irq = irq_desc_get_irq(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = irq_get_handler_data(irq);
	struct cpumask *mask = irq_get_affinity_mask(irq);
	unsigned int virq = 0, bit = 0, n = 0;

	/* To read the interrupt status to find which pin is trigger: int_status */
	if (nvt_gpio_chip_ptr == NULL) {
		pr_err("%s: handler data not find, return fail\n", __func__);
		return;
	}
	nvt_gpio_int_get_status(nvt_gpio_chip_ptr, mask);

	/* mack irq */
	chained_irq_enter(chip, desc);

	/* To find GPIO interrupt which pin is triggered
	 * We have several gpio groups to bind irq domain.
	 */
	for (n = 0; n < NVT_GPIO_TO_CPU_TOTAL; n++) {
		for_each_set_bit(bit, &nvt_gpio_chip_ptr->int_status[n], NR_GPIO_IRQ_GROP_NUM) {
			/* To find the virtual irq number */
			virq = irq_find_mapping(nvt_gpio_chip_ptr->domain, bit + (n * NR_GPIO_IRQ_GROP_NUM));
			/* Assign virq to handle irq isr */
			generic_handle_irq(virq);
		}
	}

	/* unmask irq */
	chained_irq_exit(chip, desc);
}

static struct irq_chip nvt_gpio_irq_chip = {
	.name = "Novatek GPIO IRQ",
	.irq_enable = nvt_gpio_irq_unmask,
	.irq_disable = nvt_gpio_irq_mask,
	.irq_mask = nvt_gpio_irq_mask,
	.irq_unmask = nvt_gpio_irq_unmask,
	.irq_ack = nvt_gpio_irq_ack,
	.irq_set_type = nvt_gpio_irq_set_type,
	.irq_set_affinity = nvt_gpio_irq_set_affinity,
	.irq_set_wake = nvt_gpio_irq_set_wake,
};

static int nvt_gpio_int_map(struct irq_domain *domain, unsigned int irq,
			    irq_hw_number_t hwirq)
{
	irq_set_chip_and_handler(irq, &nvt_gpio_irq_chip, handle_simple_irq);
	irq_set_chip_data(irq, domain->host_data);

	return 0;
}

static int nvt_gpio_int_map_translate(struct irq_domain *d,
				      struct irq_fwspec *fwspec,
				      unsigned long *hwirq,
				      unsigned int *type)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = d->host_data;

	if (is_of_node(fwspec->fwnode)) {
		// compatible with GIC 3-tupples description
		if (fwspec->param_count < 3)
			return -EINVAL;

		*hwirq = fwspec->param[1];
		*type = fwspec->param[2];
		dev_info(nvt_gpio_chip_ptr->dev, "HWIRQ: %ld Type: %d\n", *hwirq, *type);
	}

	return 0;
}

static const struct irq_domain_ops nvt_gpio_domain_ops = {
	.map = nvt_gpio_int_map,
	.translate = nvt_gpio_int_map_translate,
};

static int nvt_install_irq_chip(struct nvt_gpio_chip *chip)
{
	int n = 0;

	/* To create GPIO IRQ domain with total expend irq number */
	chip->domain = irq_domain_add_linear(NULL, GPIO_IRQ_TOTAL,
					     &nvt_gpio_domain_ops, chip);
	if (!chip->domain)
		return -ENOMEM;

	/* To create virq <-> hwirq mapping */
	for (n = 0; n < GPIO_IRQ_TOTAL; n++)
		irq_create_mapping(chip->domain, n);

	/*
	 * Assign handler to gpio hw irq which is used to connect ARM GIC
	 * Once HWIRQ is triggered by gpio controller, this handler will be called.
	 */
	for (n = 0; n < GPIO_IRQ_NUM; n++) {
		irq_set_chained_handler_and_data(chip->irq_base[n], nvt_summary_irq_handler, chip);
	}

	return 0;
}

static inline struct nvt_gpio_chip *to_nvt_gpio(struct gpio_chip *chip)
{
	return container_of(chip, struct nvt_gpio_chip, gpio_chip);
}

static int gpio_validation(unsigned pin)
{
	if (INVALID_GPIO(pin)) {
		pr_devel("The gpio number is out of range\n");
		return -ENOENT;
	} else
		return 0;
}

static int nvt_gpio_get(struct gpio_chip *gc, unsigned offset)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = to_nvt_gpio(gc);
	unsigned long tmp;
	unsigned long ofs;

	if (gpio_validation(offset) < 0)
		return -ENOENT;

	ofs = (offset >> 5) << 2;
	offset &= (32 - 1);
	tmp = (1 << offset);

	return (GPIO_GETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_DATA_0 + ofs) & tmp) != 0;
}

static void nvt_gpio_set(struct gpio_chip *gc, unsigned offset, int value)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = to_nvt_gpio(gc);
	unsigned long tmp;
	unsigned long ofs;
	unsigned long flags = 0;

	if (gpio_validation(offset) == 0) {

		ofs = (offset >> 5) << 2;
		offset &= (32 - 1);
		tmp = (1 << offset);

		/*race condition protect. enter critical section*/
		loc_cpu(&nvt_gpio_chip_ptr->lock, flags);

		if (value)
			GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_SET_0 + ofs, tmp);
		else
			GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_CLR_0 + ofs, tmp);

		/*race condition protect. leave critical section*/
		unl_cpu(&nvt_gpio_chip_ptr->lock, flags);

	}

}

static int nvt_gpio_dir_input(struct gpio_chip *gc, unsigned offset)
{
	unsigned long reg_data;
	unsigned long ofs;
	unsigned long flags = 0;
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = to_nvt_gpio(gc);

	if (gpio_validation(offset) < 0)
		return -ENOENT;

	ofs = (offset >> 5) << 2;
	offset &= (32 - 1);

	/*race condition protect. enter critical section*/
	loc_cpu(&nvt_gpio_chip_ptr->lock, flags);

	//reg_data = GPIO_GETREG(GPIO_STRG_DIR0_REG_OFS + ofs);
	reg_data = GPIO_GETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_DIR_0 + ofs);
	reg_data &= ~(1 << offset); /*input*/
	GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_DIR_0 + ofs, reg_data);
	//GPIO_SETREG(GPIO_STRG_DIR0_REG_OFS + ofs, reg_data);

	/*race condition protect. leave critical section*/
	unl_cpu(&nvt_gpio_chip_ptr->lock, flags);

	return 0;
}

static int nvt_gpio_dir_output(struct gpio_chip *gc,
			       unsigned offset, int value)
{
	unsigned long reg_data, tmp;
	unsigned long ofs;
	unsigned long flags = 0;
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = to_nvt_gpio(gc);

	if (gpio_validation(offset) < 0)
		return -ENOENT;

	ofs = (offset >> 5) << 2;
	offset &= (32 - 1);
	tmp = (1 << offset);

	/*race condition protect. enter critical section*/
	loc_cpu(&nvt_gpio_chip_ptr->lock, flags);

	reg_data = GPIO_GETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_DIR_0 + ofs);
	reg_data |= (1 << offset);  /*output*/
	GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_DIR_0 + ofs, reg_data);

	if (value)
		GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_SET_0 + ofs, tmp);
	else
		GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_CLR_0 + ofs, tmp);

	/*race condition protect. leave critical section*/
	unl_cpu(&nvt_gpio_chip_ptr->lock, flags);

	return 0;
}


static int nvt_gpio_get_dir(struct gpio_chip *gc, unsigned offset)
{
	unsigned long reg_data, tmp;
	unsigned long ofs;
	unsigned long flags = 0;
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = to_nvt_gpio(gc);

	if (gpio_validation(offset) < 0)
		return -ENOENT;

	ofs = (offset >> 5) << 2;
	offset &= (32 - 1);
	tmp = (1 << offset);
	/*race condition protect. enter critical section*/
	loc_cpu(&nvt_gpio_chip_ptr->lock, flags);
	reg_data = GPIO_GETREG(nvt_gpio_chip_ptr, NVT_GPIO_STG_DIR_0 + ofs);
	/*race condition protect. leave critical section*/
	unl_cpu(&nvt_gpio_chip_ptr->lock, flags);

	return !(reg_data & (1 << offset));
}


static int nvt_gpio_to_irq(struct gpio_chip *gc, unsigned offset)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = to_nvt_gpio(gc);
	unsigned int virq, i, hw_irq = nvt_gpio_chip_ptr->irq_summary;

	/* Find the hw irq number through macro translation */
	for (i = 0; i < nvt_gpio_chip_ptr->irq_summary; i++) {
		if (nvt_gpio_irq_table[i] == offset) {
			hw_irq = i;
			break;
		}
	}

	virq = irq_find_mapping(nvt_gpio_chip_ptr->domain, hw_irq);
	if (virq == 0)
		return -ENOENT;
	else
		return virq;
}

#ifdef CONFIG_PM
static int nvt_gpio_suspend(struct device *ddev)
{
	int index;
	struct nvt_gpio_chip *nvt_gpio_chip_ptr;

	if (!nvt_get_suspend_mode())
		return 0;

	nvt_gpio_chip_ptr = dev_get_drvdata(ddev);
	if (!nvt_gpio_chip_ptr) {
		pr_err("%s, dev_get_drvdata fail\n", __func__);
		return -EINVAL;
	}

	for (index = 0; index < NVT_GPIO_TOTAL_SIZE; index += sizeof(unsigned int)) {
		nvt_gpio_chip_ptr->gpio_data[index] = readl((void*)(nvt_gpio_chip_ptr->reg_base + index));
	}

	return 0;
}

static int nvt_gpio_resume(struct device *ddev)
{
	int index;
	struct nvt_gpio_chip *nvt_gpio_chip_ptr;

	if (!nvt_get_suspend_mode())
		return 0;

	nvt_gpio_chip_ptr = dev_get_drvdata(ddev);
	if (!nvt_gpio_chip_ptr) {
		pr_err("%s, dev_get_drvdata fail\n", __func__);
		return -EINVAL;
	}

	/* Set the direction first to avoid setting the value in input mode */
	for (index = NVT_GPIO_STG_DIR_0; index < (NVT_GPIO_STG_DIR_0 + (NVT_GPIO_STG_SET_0 - NVT_GPIO_STG_DIR_0)); index += sizeof(unsigned int)) {
		writel(nvt_gpio_chip_ptr->gpio_data[index], (void*)(nvt_gpio_chip_ptr->reg_base + index));
	}

	for (index = 0; index < NVT_GPIO_TOTAL_SIZE; index += sizeof(unsigned int)) {
		writel(nvt_gpio_chip_ptr->gpio_data[index], (void*)(nvt_gpio_chip_ptr->reg_base + index));
	}

	return 0;
}

static const struct dev_pm_ops nvt_gpio_pm_ops = {
	.suspend  = nvt_gpio_suspend,
	.resume = nvt_gpio_resume,
};
#endif

static int nvt_gpio_probe(struct platform_device *pdev)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = NULL;
	struct resource *res = NULL;
	int ret = 0, i = 0, cpu = 0;

	nvt_gpio_chip_ptr = kzalloc(sizeof(struct nvt_gpio_chip), GFP_KERNEL);
	if (!nvt_gpio_chip_ptr)
		return -ENOMEM;

	/* setup resource */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(!res)) {
		printk("%s, %s fails: platform_get_resource IORESOURCE_MEM not OK", pdev->name, __FUNCTION__);
		ret = -ENXIO;
		goto out_free;
	}

	nvt_gpio_chip_ptr->reg_base = ioremap(res->start, resource_size(res));
	if (!nvt_gpio_chip_ptr->reg_base) {
		printk("%s fails: ioremap_nocache fail\n", __FUNCTION__);
		ret = -1;
		goto remap_err;
	}

	/* Basic data structure initialization */
	/* irq structure init. */
	nvt_gpio_chip_ptr->dev			= &pdev->dev;
	nvt_gpio_chip_ptr->irq_summary	= GPIO_IRQ_TOTAL;		/* The number of the total gpio irq number  */
	/* To store the GIC IRQ num: GIC IRQ will map to gpio domain IRQ */
	for (i = 0; i < GPIO_IRQ_NUM; i++) {
		int irq;
		irq = platform_get_irq(pdev, i);
		if (irq < 0) {
			dev_err(&pdev->dev, "Cannot translate IRQ index %d\n", irq);
			ret = irq;
			goto getirq_err;
		}
		nvt_gpio_chip_ptr->irq_base[i] = irq;
	}

#ifdef CONFIG_SMP
	for (i = 0; i < GPIO_IRQ_NUM; i++) {
		ret = irq_set_affinity_hint(nvt_gpio_chip_ptr->irq_base[i], get_cpu_mask(i));
		if (ret < 0) {
			dev_err(&pdev->dev, "Cannot set affinity hint on %d\n",
					nvt_gpio_chip_ptr->irq_base[i]);
			goto getirq_err;
		}
	}
#endif

	for (i = 0; i < NVT_GPIO_TO_CPU_NUM; i++) {
		for (cpu = 0; cpu < GPIO_IRQ_NUM; cpu++) {
			GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_TO_CPU + (i * 0x4) + (cpu * NVT_GPUI_TO_CPU_OFFSET), ~0UL);
		}
	}

	for (i = 0; i < NVT_GPIO_TO_CPU_TOTAL; i++) {
		nvt_gpio_chip_ptr->irq_request_record[i] = 0;
		nvt_gpio_chip_ptr->int_status[i] = 0;
	}

	/* gpio structure init. */
	nvt_gpio_chip_ptr->gpio_chip.get = nvt_gpio_get;
	nvt_gpio_chip_ptr->gpio_chip.set = nvt_gpio_set;
	nvt_gpio_chip_ptr->gpio_chip.direction_input = nvt_gpio_dir_input;
	nvt_gpio_chip_ptr->gpio_chip.direction_output = nvt_gpio_dir_output;
	nvt_gpio_chip_ptr->gpio_chip.get_direction = nvt_gpio_get_dir;
	nvt_gpio_chip_ptr->gpio_chip.ngpio = NVT_GPIO_NUMBER;
	nvt_gpio_chip_ptr->gpio_chip.to_irq = nvt_gpio_to_irq;
	nvt_gpio_chip_ptr->gpio_chip.base = 0;

#ifdef CONFIG_OF_GPIO
	ret = of_property_read_u32(pdev->dev.of_node, "#gpio-cells", &nvt_gpio_chip_ptr->gpio_chip.of_gpio_n_cells);
	if (ret) {
		dev_err(&pdev->dev, "#gpio-cells not found\n");
		goto gpiochip_add_err;
	}
	nvt_gpio_chip_ptr->gpio_chip.of_node = pdev->dev.of_node;
#endif

#ifdef CONFIG_PCI
	if (nvt_gpio_chip_ptr->is_ep) {
		nvt_gpio_chip_ptr->gpio_chip.base = (nvt_gpio_chip_ptr->ep_num + 1) * NVT_GPIO_BASE_NUM;
	}

	if (of_property_read_u32(pdev->dev.of_node, "ep_num", &nvt_gpio_chip_ptr->ep_num)) {
		nvt_gpio_chip_ptr->is_ep = 0;
		nvt_gpio_chip_ptr->ep_num = 0;
	} else {
		nvt_gpio_chip_ptr->is_ep = 1;
	}
#endif

	/* Initialize the lock before use to match the new kernel flow */
	spin_lock_init(&nvt_gpio_chip_ptr->lock);

	ret = gpiochip_add(&nvt_gpio_chip_ptr->gpio_chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not register gpiochip, %d\n", ret);
		goto gpiochip_add_err;
	}

	/* To install GPIO IRQ extender chip */
	ret = nvt_install_irq_chip(nvt_gpio_chip_ptr);
	if (ret < 0)
		goto out_free_irq;

	platform_set_drvdata(pdev, nvt_gpio_chip_ptr);

	dev_info(&pdev->dev, "Register %s successfully\n", __func__);
	return 0;

out_free_irq:
	gpiochip_remove(&nvt_gpio_chip_ptr->gpio_chip);

gpiochip_add_err:
getirq_err:
	iounmap(nvt_gpio_chip_ptr->reg_base);

remap_err:
	release_mem_region(res->start, (res->end - res->start + 1));
out_free:
	kfree(nvt_gpio_chip_ptr);

	return ret;
}

static int nvt_gpio_remove(struct platform_device *pdev)
{
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = platform_get_drvdata(pdev);
	int n = 0;

	irq_domain_remove(nvt_gpio_chip_ptr->domain);

	for (n = 0; n < GPIO_IRQ_NUM; n++) {
		free_irq(nvt_gpio_chip_ptr->irq_base[n], nvt_gpio_chip_ptr);
	}

	gpiochip_remove(&nvt_gpio_chip_ptr->gpio_chip);
	platform_set_drvdata(pdev, NULL);
	kfree(nvt_gpio_chip_ptr);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id nvt_gpio_pin_match[] = {
	{ .compatible = "nvt,nvt_gpio" },
	{},
};

MODULE_DEVICE_TABLE(of, nvt_gpio_pin_match);
#endif

static struct platform_driver nvt_gpio_drv = {
	.probe		= nvt_gpio_probe,
	.remove		= nvt_gpio_remove,
	.driver		= {
		.name	= "nvt_gpio",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm = &nvt_gpio_pm_ops,
#endif
#ifdef CONFIG_OF
		.of_match_table = nvt_gpio_pin_match,
#endif
	},
};

static int __init nvt_gpio_init(void)
{
	return platform_driver_register(&nvt_gpio_drv);
}

static void __exit nvt_gpio_exit(void)
{
	platform_driver_unregister(&nvt_gpio_drv);
}

subsys_initcall(nvt_gpio_init);
module_exit(nvt_gpio_exit);

MODULE_AUTHOR("Novatek Microelectronics Corp.");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("GPIO driver for nvt SOC");
MODULE_VERSION(DRV_VERSION);
