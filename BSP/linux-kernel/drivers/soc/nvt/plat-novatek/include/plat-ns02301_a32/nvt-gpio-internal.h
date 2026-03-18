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
#include "nvt-gpio.h"

/* GPIO total pin number */
#define NVT_GPIO_NUMBER                 (INTERNAL_GPIO(0) + 1)

/* GPIO Storage Data Register */
#define GPIO_DATA_NUM                   4

/* Interrupt to GIC or other IP */
#define GPIO_IRQ_NUM                    2

/* GPIO_STS_CPU0_X */
#define NVT_GPIO_STS_CPU_NUM		3
#define NVT_GPIO_STS_CPU_OFFSET		(NVT_GPIO_STS_CPU2 - NVT_GPIO_STS_CPU)

/* GPIO_INTEN_CPU0_X */
#define NVT_GPIO_INTEN_CPU_NUM		3
#define NVT_GPIO_INTEN_CPU_OFFSET	(NVT_GPIO_INTEN_CPU2 - NVT_GPIO_INTEN_CPU)

/* GPIO_TO_CPU0_X */
#define NVT_GPIO_TO_CPU_NUM		3
#define NVT_GPUI_TO_CPU_OFFSET		(NVT_GPIO_TO_CPU2 - NVT_GPIO_TO_CPU)

#define NR_GPIO_IRQ_GROP_NUM            32
#define GPIO_IRQ_TOTAL                  (NVT_GPIO_INTEN_CPU_NUM * NR_GPIO_IRQ_GROP_NUM)

#define NVT_GPIO_TO_CPU_TOTAL		(GPIO_IRQ_NUM * NVT_GPIO_INTEN_CPU_NUM)

/* GPIO_INT_TYPE + GPIO_INT_POL + GPIO_EDGE_TYPE */
#define GPIO_IRQ_CON_NUM                ((NVT_GPIO_TO_CPU - NVT_GPIO_INT_TYPE) / 0x4)

#define C_GPIO_NUM			25
#define P_GPIO_NUM			25
#define HSI_GPIO_NUM			12
#define D_GPIO_NUM			8
#define INTERNAL_GPIO_NUM		1

#define INVALID_GPIO(pin) ( (pin >= C_GPIO(C_GPIO_NUM) && pin < P_GPIO(0)) || \
		(pin >= P_GPIO(P_GPIO_NUM) && pin < HSI_GPIO(0)) || \
		(pin >= HSI_GPIO(HSI_GPIO_NUM) && pin < D_GPIO(0)) || \
		(pin >= D_GPIO(D_GPIO_NUM) && pin < INTERNAL_GPIO(0)) || \
	        (pin >= INTERNAL_GPIO(INTERNAL_GPIO_NUM)) )

int nvt_gpio_irq_table[GPIO_IRQ_TOTAL] = {
	C_GPIO(0),     C_GPIO(1),     C_GPIO(2),     C_GPIO(3),     C_GPIO(4),    C_GPIO(5),
	C_GPIO(6),     C_GPIO(7),     C_GPIO(8),     C_GPIO(9),     C_GPIO(10),   C_GPIO(11),
	C_GPIO(12),    C_GPIO(13),    C_GPIO(14),    C_GPIO(15),    C_GPIO(16),   C_GPIO(17),
	C_GPIO(18),    C_GPIO(19),    C_GPIO(20),    C_GPIO(21),    C_GPIO(22),   C_GPIO(23),
	C_GPIO(24),

	P_GPIO(0),     P_GPIO(1),     P_GPIO(2),     P_GPIO(3),     P_GPIO(4),    P_GPIO(5),
	P_GPIO(6),     P_GPIO(7),     P_GPIO(8),     P_GPIO(9),     P_GPIO(10),   P_GPIO(11),
	P_GPIO(12),    P_GPIO(13),    P_GPIO(14),    P_GPIO(15),    P_GPIO(16),   P_GPIO(17),
	P_GPIO(18),    P_GPIO(19),    P_GPIO(20),    P_GPIO(21),    P_GPIO(22),   P_GPIO(23),
	P_GPIO(24),    P_GPIO(39),

	HSI_GPIO(0),   HSI_GPIO(1),   HSI_GPIO(2),   HSI_GPIO(3),   HSI_GPIO(4),  HSI_GPIO(5),
	HSI_GPIO(6),   HSI_GPIO(7),   HSI_GPIO(8),   HSI_GPIO(9),   HSI_GPIO(10), HSI_GPIO(11),

	D_GPIO(0),     D_GPIO(1),     D_GPIO(2),     D_GPIO(3),     D_GPIO(4),     D_GPIO(5),
	D_GPIO(6),     D_GPIO(7),

	-1, -1, -1, -1, -1,

	INTERNAL_GPIO(0),
};
