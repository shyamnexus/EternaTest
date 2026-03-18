/**
	GPIO header file
	This file will setup gpio related base address
	@file       nvt-gpio-internal.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include "nvt-gpio.h"

/* GPIO total pin number */
#define NVT_GPIO_NUMBER              (S_GPIO(S_GPIO_NUM))

/* GPIO Storage Data Register */
#define GPIO_DATA_NUM           7

/* Interrupt to GIC or other IP */
#define GPIO_IRQ_NUM                    2

/* GPIO_STS_CPU0_X */
#define NVT_GPIO_STS_CPU_NUM            4
#define NVT_GPIO_STS_CPU_OFFSET         (NVT_GPIO_STS_CPU2 - NVT_GPIO_STS_CPU)
#define NVT_GPIO_INTEN_CPU_NUM          4
#define NVT_GPIO_INTEN_CPU_OFFSET       (NVT_GPIO_INTEN_CPU2 - NVT_GPIO_INTEN_CPU)
#define NVT_GPIO_TO_CPU_NUM             4
#define NVT_GPUI_TO_CPU_OFFSET          (NVT_GPIO_TO_CPU2 - NVT_GPIO_TO_CPU)

#define NR_GPIO_IRQ_GROP_NUM            32
#define GPIO_IRQ_TOTAL                  (NVT_GPIO_INTEN_CPU_NUM * NR_GPIO_IRQ_GROP_NUM)

#define NVT_GPIO_TO_CPU_TOTAL           (GPIO_IRQ_NUM * NVT_GPIO_INTEN_CPU_NUM)

/* GPIO_INT_TYPE + GPIO_INT_POL + GPIO_EDGE_TYPE */
#define GPIO_IRQ_CON_NUM                ((NVT_GPIO_TO_CPU - NVT_GPIO_INT_TYPE) / 0x4)

#define INVALID_GPIO(pin) ( (pin < C_GPIO(0)) || \
	    (pin >= C_GPIO(C_GPIO_NUM) && pin < J_GPIO(0)) || \
	    (pin >= J_GPIO(J_GPIO_NUM) && pin < P_GPIO(0)) || \
	    (pin >= P_GPIO(P_GPIO_NUM) && pin < E_GPIO(0)) || \
	    (pin >= E_GPIO(E_GPIO_NUM) && pin < D_GPIO(0)) || \
	    (pin >= D_GPIO(D_GPIO_NUM) && pin < S_GPIO(0)) || \
	    (pin >= S_GPIO(S_GPIO_NUM)) )

int nvt_gpio_irq_table[GPIO_IRQ_TOTAL] = {
	C_GPIO(3),	C_GPIO(5),	C_GPIO(7),
	J_GPIO(0),	J_GPIO(1),	J_GPIO(2),	J_GPIO(3),	J_GPIO(4),
	P_GPIO(0),	P_GPIO(1),	P_GPIO(2),	P_GPIO(3),	P_GPIO(5),
	P_GPIO(6),	P_GPIO(7),	P_GPIO(8),	P_GPIO(9),	P_GPIO(10),	P_GPIO(11),
	P_GPIO(12),	P_GPIO(14),	P_GPIO(15),	P_GPIO(16),	P_GPIO(17),
	P_GPIO(18),	P_GPIO(19),	P_GPIO(20),	P_GPIO(21),	P_GPIO(23),
	P_GPIO(24),	P_GPIO(25),	P_GPIO(26),
	D_GPIO(0),	D_GPIO(1),	D_GPIO(2),	D_GPIO(3),	D_GPIO(4),	D_GPIO(5),
	D_GPIO(6),	D_GPIO(7),	D_GPIO(8),	D_GPIO(9),
	E_GPIO(0),	E_GPIO(1),	E_GPIO(2),	E_GPIO(3),	E_GPIO(4),	E_GPIO(5),
	E_GPIO(6),	E_GPIO(7),	E_GPIO(8),	E_GPIO(9),	E_GPIO(10),	E_GPIO(11),
	E_GPIO(12),	E_GPIO(13),	E_GPIO(14),	E_GPIO(15),
	S_GPIO(0),	S_GPIO(1),	S_GPIO(2),	S_GPIO(3),	S_GPIO(4),	S_GPIO(5),
	S_GPIO(6),	S_GPIO(7),	S_GPIO(8),	S_GPIO(9),	S_GPIO(10),	S_GPIO(11),
	S_GPIO(12),	S_GPIO(13),	S_GPIO(14),	S_GPIO(15),	S_GPIO(16),	S_GPIO(17),
	S_GPIO(18),	S_GPIO(19),	S_GPIO(20),	S_GPIO(21),	S_GPIO(22),	S_GPIO(23),
	S_GPIO(24),	S_GPIO(25),	S_GPIO(26),	S_GPIO(27),	S_GPIO(28),	S_GPIO(29),
	S_GPIO(30),	S_GPIO(31),	S_GPIO(32),	S_GPIO(33),	S_GPIO(34),	S_GPIO(35),
	P_GPIO(27),	P_GPIO(28),	P_GPIO(29),	P_GPIO(30),	P_GPIO(31),
};
