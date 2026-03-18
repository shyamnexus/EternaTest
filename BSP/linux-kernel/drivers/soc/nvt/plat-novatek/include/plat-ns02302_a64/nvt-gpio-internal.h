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
#define NVT_GPIO_NUMBER                 (INTERNAL_GPIO(INTERNAL_GPIO_NUM))

/* GPIO Storage Data Register */
#define GPIO_DATA_NUM                   7

/* Interrupt to GIC or other IP */
#define GPIO_IRQ_NUM                    2

/* GPIO_STS_CPU0_X */
#define NVT_GPIO_STS_CPU_NUM		5
#define NVT_GPIO_STS_CPU_OFFSET		(NVT_GPIO_STS_CPU2 - NVT_GPIO_STS_CPU)

/* GPIO_INTEN_CPU0_X */
#define NVT_GPIO_INTEN_CPU_NUM		5
#define NVT_GPIO_INTEN_CPU_OFFSET	(NVT_GPIO_INTEN_CPU2 - NVT_GPIO_INTEN_CPU)

/* GPIO_TO_CPU0_X */
#define NVT_GPIO_TO_CPU_NUM		5
#define NVT_GPUI_TO_CPU_OFFSET		(NVT_GPIO_TO_CPU2 - NVT_GPIO_TO_CPU)

#define NR_GPIO_IRQ_GROP_NUM            32
#define GPIO_IRQ_TOTAL                  (NVT_GPIO_INTEN_CPU_NUM * NR_GPIO_IRQ_GROP_NUM)

#define NVT_GPIO_TO_CPU_TOTAL		(GPIO_IRQ_NUM * NVT_GPIO_INTEN_CPU_NUM)

/* GPIO_INT_TYPE + GPIO_INT_POL + GPIO_EDGE_TYPE */
#define GPIO_IRQ_CON_NUM                ((NVT_GPIO_TO_CPU - NVT_GPIO_INT_TYPE) / 0x4)

#define C_GPIO_NUM			24
#define P_GPIO_NUM			26
#define S_GPIO_NUM			16
#define L_GPIO_NUM			25
#define D_GPIO_NUM			15
#define HSI_GPIO_NUM			24
#define A_GPIO_NUM			3
#define INTERNAL_GPIO_NUM		2 // interal 1 is for usb

#define INVALID_GPIO(pin) ( (pin >= C_GPIO(C_GPIO_NUM) && pin < P_GPIO(0)) || \
		(pin >= P_GPIO(P_GPIO_NUM) && pin < S_GPIO(0)) || \
		(pin >= S_GPIO(S_GPIO_NUM) && pin < L_GPIO(0)) || \
		(pin >= L_GPIO(L_GPIO_NUM) && pin < D_GPIO(0)) || \
		(pin >= D_GPIO(D_GPIO_NUM) && pin < HSI_GPIO(0)) || \
		(pin >= HSI_GPIO(HSI_GPIO_NUM) && pin < A_GPIO(0)) || \
		(pin >= A_GPIO(A_GPIO_NUM) && pin < INTERNAL_GPIO(0)) || \
	        (pin >= INTERNAL_GPIO(INTERNAL_GPIO_NUM)) )

int nvt_gpio_irq_table[GPIO_IRQ_TOTAL] = {
	C_GPIO(0),     C_GPIO(1),     C_GPIO(2),     C_GPIO(3),     C_GPIO(4),    C_GPIO(5),
	C_GPIO(6),     C_GPIO(7),     C_GPIO(8),     C_GPIO(9),     C_GPIO(10),   C_GPIO(11),
	C_GPIO(12),    C_GPIO(13),    C_GPIO(14),    C_GPIO(15),    C_GPIO(16),   C_GPIO(17),
	C_GPIO(18),    C_GPIO(19),    C_GPIO(20),    C_GPIO(21),    C_GPIO(22),   C_GPIO(23),

	P_GPIO(0),     P_GPIO(1),     P_GPIO(2),     P_GPIO(3),     P_GPIO(4),    P_GPIO(5),
	P_GPIO(6),     P_GPIO(7),     P_GPIO(8),     P_GPIO(9),     P_GPIO(10),   P_GPIO(11),
	P_GPIO(12),    P_GPIO(13),    P_GPIO(14),    P_GPIO(15),    P_GPIO(16),   P_GPIO(17),
	P_GPIO(18),    P_GPIO(19),    P_GPIO(20),    P_GPIO(21),    P_GPIO(22),   P_GPIO(23),
	P_GPIO(24),    P_GPIO(25),

	S_GPIO(0),     S_GPIO(1),     S_GPIO(2),     S_GPIO(3),     S_GPIO(4),    S_GPIO(5),
	S_GPIO(6),     S_GPIO(7),     S_GPIO(8),     S_GPIO(9),     S_GPIO(10),   S_GPIO(11),
	S_GPIO(12),    S_GPIO(13),    S_GPIO(14),    S_GPIO(15),

	L_GPIO(0),     L_GPIO(1),     L_GPIO(2),     L_GPIO(3),     L_GPIO(4),    L_GPIO(5),
	L_GPIO(6),     L_GPIO(7),     L_GPIO(8),     L_GPIO(9),     L_GPIO(10),   L_GPIO(11),
	L_GPIO(12),    L_GPIO(13),    L_GPIO(14),    L_GPIO(15),    L_GPIO(16),   L_GPIO(17),
	L_GPIO(18),    L_GPIO(19),    L_GPIO(20),    L_GPIO(21),    L_GPIO(22),   L_GPIO(23),
	L_GPIO(24),

	D_GPIO(0),     D_GPIO(1),     D_GPIO(2),     D_GPIO(3),     D_GPIO(4),     D_GPIO(5),
	D_GPIO(6),     D_GPIO(7),     D_GPIO(8),     D_GPIO(9),     D_GPIO(10),    D_GPIO(11),
	D_GPIO(12),    D_GPIO(13),    D_GPIO(14),

	HSI_GPIO(0),   HSI_GPIO(1),   HSI_GPIO(2),   HSI_GPIO(3),   HSI_GPIO(4),  HSI_GPIO(5),
	HSI_GPIO(6),   HSI_GPIO(7),   HSI_GPIO(8),   HSI_GPIO(9),   HSI_GPIO(10), HSI_GPIO(11),
	HSI_GPIO(12),  HSI_GPIO(13),  HSI_GPIO(14),  HSI_GPIO(15),  HSI_GPIO(16), HSI_GPIO(17),
	HSI_GPIO(18),  HSI_GPIO(19),  HSI_GPIO(20),  HSI_GPIO(21),  HSI_GPIO(22), HSI_GPIO(23),

	A_GPIO(0),     A_GPIO(1),     A_GPIO(2),

	INTERNAL_GPIO(0), INTERNAL_GPIO(1),
};
