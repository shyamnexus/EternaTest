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
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/soc/nvt/nvt-io.h>
#include <linux/soc/nvt/rcw_macro.h>
#include <plat/nvt-gpio.h>

#define DRV_VERSION		"2.00.006"

#define loc_cpu(lock, flags) spin_lock_irqsave(lock, flags)
#define unl_cpu(lock, flags) spin_unlock_irqrestore(lock, flags)

#define GPIO_SETREG(chip, ofs,value)	   OUTW(chip->reg_base +(ofs),(value))
#define GPIO_GETREG(chip, ofs)		   INW(chip->reg_base+(ofs))

/*GPIO number in na51090 SoC*/
#define GPIO_DATA_NUM           10
#define GPIO_IRQ_NUM            4
#define NR_GPIO_IRQ_GROP_NUM	32
#define GPIO_IRQ_TOTAL			(GPIO_IRQ_NUM * NR_GPIO_IRQ_GROP_NUM)
#define GPIO_IRQ_CON_NUM        12
#define NVT_GPIO_TO_CPU_TOTAL           (NVT_GPIO_INTEN_CPU_NUM * GPIO_IRQ_NUM)

struct nvt_gpio_pm_data {
	unsigned long gpio_data[GPIO_DATA_NUM];
	unsigned long gpio_dir[GPIO_DATA_NUM];
	unsigned long gpio_irq_en[GPIO_IRQ_NUM];
	unsigned long gpio_irq_config[GPIO_IRQ_CON_NUM];
};

/* GPIO pin and interrupt control structures */
struct nvt_gpio_chip {
	struct device                  *dev;
	void __iomem                   *reg_base;
	struct irq_domain				*domain;
	unsigned int                   irq_base[GPIO_IRQ_NUM];           /* Shared IRQ: GIC to GPIO IRQ */
	unsigned int                   irq_summary;                      /* Total IRQ numbers */
	unsigned long                  irq_request_record[GPIO_IRQ_NUM]; /* To record GPIO irq domain int ID 0~127 */
	struct irq_chip                irq_chip;
	unsigned long                  int_status[GPIO_IRQ_NUM];         /* Interrupt status GPIO_1, GPIO_2, GPIO_3 and GPIO_4 */
	spinlock_t                     lock;
	struct gpio_chip               gpio_chip;                        /* To abstract gpio controller */
	struct nvt_gpio_pm_data        pm_data;                          /* GPIO data used for power management */
	unsigned long                  cpu_num;                          /* Force interrupt target*/
	int                            is_ep;
	int                            ep_num;
	unsigned int		       gpio_data[NVT_GPIO_TOTAL_SIZE];
};

static int nvt_gpio_irq_table[GPIO_IRQ_TOTAL] = {
	C_GPIO(0),      C_GPIO(1),      C_GPIO(2),      C_GPIO(3),      C_GPIO(4),     C_GPIO(5),
	C_GPIO(6),      C_GPIO(7),      C_GPIO(8),      C_GPIO(9),      C_GPIO(10),    C_GPIO(11),
	C_GPIO(12),     C_GPIO(13),     J_GPIO(0),      J_GPIO(1),      J_GPIO(2),     J_GPIO(3),
	J_GPIO(4),      P_GPIO(0),      P_GPIO(1),      P_GPIO(2),      P_GPIO(3),     P_GPIO(4),
	P_GPIO(5),      P_GPIO(6),      P_GPIO(7),      P_GPIO(8),      P_GPIO(9),     P_GPIO(10),
	P_GPIO(11),     P_GPIO(12),     P_GPIO(13),     P_GPIO(14),     P_GPIO(15),    P_GPIO(16),
	P_GPIO(17),     P_GPIO(18),     P_GPIO(19),     P_GPIO(20),     P_GPIO(21),    P_GPIO(22),
	P_GPIO(23),     P_GPIO(24),     P_GPIO(25),     P_GPIO(26),     P_GPIO(27),    P_GPIO(28),
	P_GPIO(29),     P_GPIO(30),     P_GPIO(31),     P_GPIO(32),     P_GPIO(33),    P_GPIO(34),
	D_GPIO(0),      D_GPIO(1),      D_GPIO(2),      D_GPIO(3),      D_GPIO(4),     D_GPIO(5),
	D_GPIO(6),      D_GPIO(7),      D_GPIO(8),      D_GPIO(9),      D_GPIO(10),    D_GPIO(11),
	E_GPIO(0),      E_GPIO(1),      E_GPIO(2),      E_GPIO(3),      E_GPIO(4),     E_GPIO(5),
	E_GPIO(6),      E_GPIO(7),      E_GPIO(8),      E_GPIO(9),      E_GPIO(10),    E_GPIO(11),
	E_GPIO(12),     E_GPIO(13),     E_GPIO(14),     E_GPIO(15),     S_GPIO(0),     S_GPIO(1),
	S_GPIO(2),      S_GPIO(3),      S_GPIO(4),      S_GPIO(5),      S_GPIO(6),     S_GPIO(7),
	S_GPIO(8),      S_GPIO(9),      S_GPIO(10),     S_GPIO(11),     S_GPIO(12),    S_GPIO(13),
	S_GPIO(14),     S_GPIO(15),     S_GPIO(16),     S_GPIO(17),     S_GPIO(18),    S_GPIO(19),
	S_GPIO(20),     S_GPIO(21),     S_GPIO(22),     S_GPIO(23),     S_GPIO(24),    S_GPIO(25),
	S_GPIO(26),     S_GPIO(27),     S_GPIO(28),     S_GPIO(29),     S_GPIO(30),    S_GPIO(31),
	S_GPIO(32),     S_GPIO(33),     S_GPIO(34),     S_GPIO(35),     P_GPIO(35),    P_GPIO(36),
	E_GPIO(19),     E_GPIO(20),     E_GPIO(25),     P_GPIO(40),     P_GPIO(41),    B_GPIO(2),
	B_GPIO(3),      B_GPIO(11)
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

	cpu = cpumask_any_and(mask, cpu_online_mask);
	for (i = 0; i < GPIO_IRQ_NUM; i++) {
		if (!chip->irq_base[i])
			break;
		chip->int_status[i] = GPIO_GETREG(chip, NVT_GPIO_STS_CPU + (cpu * NVT_GPIO_CPU_REG_OFFSET) + i * 4);
		/* reuested record is used to save which pin is called by request_irq
		 * and operation with this pin
		 */
		chip->int_status[i] &= chip->irq_request_record[i];
	}
}

static void nvt_gpio_int_set_status(struct nvt_gpio_chip *chip, unsigned long hw_irq_num)
{
	unsigned long tmp, i;
	unsigned long ofs = (hw_irq_num >> 5) << 2;

	hw_irq_num &= (32 - 1);
	tmp = (1 << hw_irq_num);

	/* Clear the status of edge trigger and level trigger */
	for (i = 0; i < NVT_GPIO_INTEN_CPU_NUM; i++)  {
		GPIO_SETREG(chip, NVT_GPIO_STS_CPU + (i * NVT_GPIO_CPU_REG_OFFSET) + ofs, tmp);
	}
}

static void nvt_gpio_int_pin_en(struct nvt_gpio_chip *chip, u8 hw_irq_num, u8 en, const struct cpumask *mask)
{
	int cpu;
	unsigned long reg = 0;
	unsigned long ofs = (hw_irq_num >> 5) << 2;

	hw_irq_num &= (32 - 1);

	cpu = cpumask_any_and(mask, cpu_online_mask);
	if (en) {
		reg = GPIO_GETREG(chip, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_CPU_REG_OFFSET) + ofs);
		reg |= 1 << hw_irq_num;
		GPIO_SETREG(chip, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_CPU_REG_OFFSET) + ofs, reg);
	} else {
		reg = GPIO_GETREG(chip, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_CPU_REG_OFFSET) + ofs);
		reg &= ~(1 << hw_irq_num);
		GPIO_SETREG(chip, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_CPU_REG_OFFSET) + ofs, reg);
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
	for (i = 0; i < NVT_GPIO_INTEN_CPU_NUM; i++)  {
		reg = GPIO_GETREG(nvt_gpio_chip_ptr, NVT_GPIO_INTEN_CPU + (i * NVT_GPIO_CPU_REG_OFFSET) + ofs);
		reg &= ~(1 << hw_irq_num);
		GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_INTEN_CPU + (i * NVT_GPIO_CPU_REG_OFFSET) + ofs, reg);
	}

	reg = GPIO_GETREG(nvt_gpio_chip_ptr, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_CPU_REG_OFFSET) + ofs);
	reg |= 1 << hw_irq_num;
	GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_INTEN_CPU + (cpu * NVT_GPIO_CPU_REG_OFFSET) + ofs, reg);

	spin_unlock_irqrestore(&nvt_gpio_chip_ptr->lock, flags);

	irq_data_update_effective_affinity(d, cpumask_of(cpu));

	return IRQ_SET_MASK_OK;
}

static void nvt_summary_irq_handler(struct irq_desc *desc)
{
	unsigned int irq = irq_desc_get_irq(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct nvt_gpio_chip *nvt_gpio_chip_ptr = irq_get_handler_data(irq);
	struct cpumask *mask = irq_get_affinity_mask(irq);
	unsigned int virq = 0, bit = 0, n = 0;

	/* To read the interrupt status to find which pin is trigger: int_status */
	nvt_gpio_int_get_status(nvt_gpio_chip_ptr, mask);

	/* mack irq */
	chained_irq_enter(chip, desc);

	/* To find GPIO interrupt which pin is triggered
	 * We have several gpio groups to bind irq domain.
	 */
	for (n = 0; n < GPIO_IRQ_NUM; n++) {
		if (!nvt_gpio_chip_ptr->irq_base[n])
			break;
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
		if (!chip->irq_base[n])
			break;
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
	if ((pin < C_GPIO(0)) || \
	    (pin >= C_GPIO(C_GPIO_NUM) && pin < J_GPIO(0)) || \
	    (pin >= J_GPIO(J_GPIO_NUM) && pin < P_GPIO(0)) || \
	    (pin >= P_GPIO(P_GPIO_NUM) && pin < E_GPIO(0)) || \
	    (pin >= E_GPIO(E_GPIO_NUM) && pin < D_GPIO(0)) || \
	    (pin >= D_GPIO(D_GPIO_NUM) && pin < S_GPIO(0)) || \
	    (pin >= S_GPIO(S_GPIO_NUM) && pin < B_GPIO(0)) || \
	    (pin >= B_GPIO(B_GPIO_NUM))) {
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
	unsigned int virq, hw_irq, i;

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
	int ret = 0, i = 0, irq;
	u32 cpu_num = 0, ep_num = 0;

	nvt_gpio_chip_ptr = kzalloc(sizeof(struct nvt_gpio_chip), GFP_KERNEL);
	if (!nvt_gpio_chip_ptr)
		return -ENOMEM;

	if (pdev->resource->flags == IORESOURCE_MEM) {
		/* setup resource */
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (unlikely(!res)) {
			printk("%s, %s fails: platform_get_resource IORESOURCE_MEM not OK", pdev->name, __FUNCTION__);
			ret = -ENXIO;
			goto out_free;
		}
	}

	nvt_gpio_chip_ptr->reg_base = ioremap(res->start, resource_size(res));
	if (unlikely(nvt_gpio_chip_ptr->reg_base == 0)) {
		printk("%s fails: ioremap fail\n", __FUNCTION__);
		ret = -1;
		goto remap_err;
	}

	/* Basic data structure initialization */
	/* irq structure init. */
	nvt_gpio_chip_ptr->dev			= &pdev->dev;
	nvt_gpio_chip_ptr->irq_summary	= GPIO_IRQ_TOTAL;		/* The number of the total gpio irq number  */
	/* To store the GIC IRQ num: GIC IRQ will map to gpio domain IRQ */
	for (i = 0; i < GPIO_IRQ_NUM; i++)
		nvt_gpio_chip_ptr->irq_base[i] = 0;

	for (i = 0; i < platform_irq_count(pdev); i++) {
		irq = platform_get_irq(pdev, i);
		if (irq < 0) {
			dev_err(&pdev->dev, "Cannot translate IRQ index %d\n",
					nvt_gpio_chip_ptr->irq_base[i]);
			ret = nvt_gpio_chip_ptr->irq_base[i];
			goto getirq_err;
		}
 		nvt_gpio_chip_ptr->irq_base[i] = irq;
	}

	for (i = 0; i < NVT_GPIO_TO_CPU_TOTAL; i++)  {
		GPIO_SETREG(nvt_gpio_chip_ptr, NVT_GPIO_TO_CPU + (i * 0x4), ~0UL);
	}

	for (i = 0; i < GPIO_IRQ_NUM; i++) {
		nvt_gpio_chip_ptr->irq_request_record[i] = 0;
		nvt_gpio_chip_ptr->int_status[i] = 0;
	}

	if (of_property_read_u32(pdev->dev.of_node, "ep_num", &ep_num))
		nvt_gpio_chip_ptr->is_ep = 0;
	else {
		nvt_gpio_chip_ptr->is_ep = 1;
		nvt_gpio_chip_ptr->ep_num = ep_num;
	}

	if (!nvt_gpio_chip_ptr->is_ep) {
		for (i = 0; i < NVT_GPIO_INTEN_CPU_NUM; i++) {
			ret = irq_set_affinity_hint(nvt_gpio_chip_ptr->irq_base[i], get_cpu_mask(i));
			if (ret < 0) {
				dev_err(&pdev->dev, "Cannot set affinity hint on %d\n",
						nvt_gpio_chip_ptr->irq_base[i]);
				goto getirq_err;
			}
		}
	}

	/* gpio structure init. */
	nvt_gpio_chip_ptr->gpio_chip.get = nvt_gpio_get;
	nvt_gpio_chip_ptr->gpio_chip.set = nvt_gpio_set;
	nvt_gpio_chip_ptr->gpio_chip.direction_input = nvt_gpio_dir_input;
	nvt_gpio_chip_ptr->gpio_chip.direction_output = nvt_gpio_dir_output;
	nvt_gpio_chip_ptr->gpio_chip.get_direction = nvt_gpio_get_dir;
	nvt_gpio_chip_ptr->gpio_chip.ngpio = NVT_GPIO_NUMBER;
	nvt_gpio_chip_ptr->gpio_chip.to_irq = nvt_gpio_to_irq;
	if (nvt_gpio_chip_ptr->is_ep) {
		nvt_gpio_chip_ptr->gpio_chip.base = (nvt_gpio_chip_ptr->ep_num + 1) * NVT_GPIO_BASE_NUM;
	} else
		nvt_gpio_chip_ptr->gpio_chip.base = 0;
#ifdef CONFIG_OF_GPIO
	nvt_gpio_chip_ptr->gpio_chip.of_gpio_n_cells = 2;
	nvt_gpio_chip_ptr->gpio_chip.of_node = pdev->dev.of_node;

	of_property_read_u32(pdev->dev.of_node, "cpu-num", &cpu_num);
	nvt_gpio_chip_ptr->cpu_num = cpu_num;
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
	int n = 0, irq_counts;

	irq_domain_remove(nvt_gpio_chip_ptr->domain);
	irq_counts = platform_irq_count(pdev);

	for (n = 0; n < irq_counts; n++) {
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
MODULE_DESCRIPTION("GPIO driver for nvt na51090 SOC");
MODULE_VERSION(DRV_VERSION);
