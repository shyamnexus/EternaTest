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
#define GPIO_DATA_NUM                   8

/* Interrupt to GIC or other IP */
#define GPIO_IRQ_NUM                    8

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

#define C_GPIO_NUM                      18
#define P_GPIO_NUM                      48
#define D_GPIO_NUM                      12
#define J_GPIO_NUM                      6
#define E_GPIO_NUM                      32
#define B_GPIO_NUM                      17
#define A_GPIO_NUM                      4
#define INTERNAL_GPIO_NUM               1

#define INVALID_GPIO(pin) ( (pin >= C_GPIO(C_GPIO_NUM) && pin < P_GPIO(0)) || \
		(pin >= P_GPIO(P_GPIO_NUM) && pin < D_GPIO(0)) || \
		(pin >= D_GPIO(D_GPIO_NUM) && pin < J_GPIO(0)) || \
		(pin >= J_GPIO(J_GPIO_NUM) && pin < E_GPIO(0)) || \
		(pin >= E_GPIO(E_GPIO_NUM) && pin < B_GPIO(0)) || \
		(pin >= B_GPIO(B_GPIO_NUM) && pin < A_GPIO(0)) || \
		(pin >= A_GPIO(A_GPIO_NUM) && pin < INTERNAL_GPIO(0)) || \
	        (pin >= INTERNAL_GPIO(INTERNAL_GPIO_NUM)) )

int nvt_gpio_irq_table[GPIO_IRQ_TOTAL] = {
	C_GPIO(0),     C_GPIO(1),     C_GPIO(2),     C_GPIO(3),     C_GPIO(4),    C_GPIO(5),
	C_GPIO(6),     C_GPIO(7),     C_GPIO(8),     C_GPIO(9),     C_GPIO(10),   C_GPIO(11),
	C_GPIO(12),    C_GPIO(13),    C_GPIO(14),    C_GPIO(15),    C_GPIO(16),   C_GPIO(17),

	P_GPIO(0),     P_GPIO(1),     P_GPIO(2),     P_GPIO(3),     P_GPIO(4),    P_GPIO(5),
	P_GPIO(6),     P_GPIO(7),     P_GPIO(8),     P_GPIO(9),     P_GPIO(10),   P_GPIO(11),
	P_GPIO(12),    P_GPIO(13),    P_GPIO(14),    P_GPIO(15),    P_GPIO(16),   P_GPIO(17),
	P_GPIO(18),    P_GPIO(19),    P_GPIO(20),    P_GPIO(21),    P_GPIO(22),   P_GPIO(23),
	P_GPIO(24),    P_GPIO(25),    P_GPIO(26),    P_GPIO(27),    P_GPIO(28),   P_GPIO(29),
	P_GPIO(30),    P_GPIO(31),    P_GPIO(32),    P_GPIO(33),    P_GPIO(34),   P_GPIO(35),
	P_GPIO(36),    P_GPIO(37),    P_GPIO(38),    P_GPIO(39),    P_GPIO(40),   P_GPIO(41),
	P_GPIO(42),    P_GPIO(43),    P_GPIO(44),    P_GPIO(45),    P_GPIO(46),   P_GPIO(47),

	D_GPIO(0),     D_GPIO(1),     D_GPIO(2),     D_GPIO(3),     D_GPIO(4),    D_GPIO(5),
	D_GPIO(6),     D_GPIO(7),     D_GPIO(8),     D_GPIO(9),     D_GPIO(10),   D_GPIO(11),

	J_GPIO(0),     J_GPIO(1),     J_GPIO(2),     J_GPIO(3),     J_GPIO(4),    J_GPIO(5),

	E_GPIO(0),     E_GPIO(1),     E_GPIO(2),     E_GPIO(3),     E_GPIO(4),     E_GPIO(5),
	E_GPIO(6),     E_GPIO(7),     E_GPIO(8),     E_GPIO(9),     E_GPIO(10),    E_GPIO(11),
	E_GPIO(12),    E_GPIO(13),    E_GPIO(14),    E_GPIO(15),    E_GPIO(16),    E_GPIO(17),
	E_GPIO(18),    E_GPIO(19),    E_GPIO(20),    E_GPIO(21),    E_GPIO(22),    E_GPIO(23),
	E_GPIO(24),    E_GPIO(25),    E_GPIO(26),    E_GPIO(27),    E_GPIO(28),    E_GPIO(29),
	E_GPIO(30),    E_GPIO(31),

	B_GPIO(0),     B_GPIO(1),     B_GPIO(2),     B_GPIO(3),     B_GPIO(4),     B_GPIO(5),
	B_GPIO(6),     B_GPIO(7),     B_GPIO(8),     B_GPIO(9),     B_GPIO(10),    B_GPIO(11),
	B_GPIO(12),    B_GPIO(13),    B_GPIO(14),    B_GPIO(15),    B_GPIO(16),

	A_GPIO(0),     A_GPIO(1),     A_GPIO(2),     A_GPIO(3),

	INTERNAL_GPIO(0),
};
