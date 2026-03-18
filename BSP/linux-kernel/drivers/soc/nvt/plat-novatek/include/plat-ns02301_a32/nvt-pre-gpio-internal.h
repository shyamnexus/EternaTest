/**
	GPIO header file
	This file will setup gpio related base address
	@file       nvt-gpio-internal.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include "nvt-pre-gpio.h"

/* gpio_chip_hw_gpio will subtract the base, so internally only the offset is checked */
#define P_GPIO_NUM                      14
#define S_GPIO_NUM                      17
#define A_GPIO_NUM                      4
#define INTERNAL_GPIO_NUM               1

#define P_GPIO(pin)                  (pin)
#define S_GPIO(pin)                  (pin + 0x20)
#define A_GPIO(pin)                  (pin + 0x40)
#define INTERNAL_GPIO(pin)           (pin + 0x60)

/* GPIO total pin number */
#define NVT_PRE_GPIO_NUMBER                 (INTERNAL_GPIO(INTERNAL_GPIO_NUM))

/* GPIO Storage Data Register */
#define GPIO_DATA_NUM                   3

/* Interrupt to GIC or other IP */
#define GPIO_IRQ_NUM                    2

/* GPIO_STS_CPU0_X */
#define NVT_PRE_GPIO_STS_CPU_NUM		2
#define NVT_PRE_GPIO_STS_CPU_OFFSET		(NVT_PRE_GPIO_STS_CPU2 - NVT_PRE_GPIO_STS_CPU)

/* GPIO_INTEN_CPU0_X */
#define NVT_PRE_GPIO_INTEN_CPU_NUM		2
#define NVT_PRE_GPIO_INTEN_CPU_OFFSET	(NVT_PRE_GPIO_INTEN_CPU2 - NVT_PRE_GPIO_INTEN_CPU)

/* GPIO_TO_CPU0_X */
#define NVT_GPIO_TO_CPU_NUM		2
#define NVT_GPUI_TO_CPU_OFFSET		(NVT_PRE_GPIO_TO_CPU2 - NVT_PRE_GPIO_TO_CPU)

#define NR_GPIO_IRQ_GROP_NUM            32
#define GPIO_IRQ_TOTAL                  (NVT_PRE_GPIO_INTEN_CPU_NUM * NR_GPIO_IRQ_GROP_NUM)

#define NVT_GPIO_TO_CPU_TOTAL		(GPIO_IRQ_NUM * NVT_PRE_GPIO_INTEN_CPU_NUM)

/* GPIO_INT_TYPE + GPIO_INT_POL + GPIO_EDGE_TYPE */
#define GPIO_IRQ_CON_NUM                ((NVT_PRE_GPIO_TO_CPU - NVT_PRE_GPIO_INT_TYPE) / 0x4)

#define INVALID_GPIO(pin) ((pin >= P_GPIO(P_GPIO_NUM) && pin < S_GPIO(0)) || \
		(pin >= S_GPIO(S_GPIO_NUM) && pin < A_GPIO(0)) || \
		(pin >= A_GPIO(A_GPIO_NUM) && pin < INTERNAL_GPIO(0)) || \
		(pin >= INTERNAL_GPIO(INTERNAL_GPIO_NUM)) )

int nvt_pre_gpio_irq_table[GPIO_IRQ_TOTAL] = {
	P_GPIO(0),  P_GPIO(1),  P_GPIO(2),  P_GPIO(3),  P_GPIO(4),   P_GPIO(5),
	P_GPIO(6),  P_GPIO(7),  P_GPIO(8),  P_GPIO(9),  P_GPIO(10),   P_GPIO(11),
	P_GPIO(12),  P_GPIO(13),

	S_GPIO(0),   S_GPIO(1),   S_GPIO(2),   S_GPIO(3),   S_GPIO(4),  S_GPIO(5),
	S_GPIO(6),   S_GPIO(7),   S_GPIO(8),   S_GPIO(9),   S_GPIO(10), S_GPIO(11),
	S_GPIO(12),  S_GPIO(13),  S_GPIO(14),  S_GPIO(15),  S_GPIO(16),

	A_GPIO(0),   A_GPIO(1),   A_GPIO(2),   A_GPIO(3),

	-1,

	INTERNAL_GPIO(0),
};
