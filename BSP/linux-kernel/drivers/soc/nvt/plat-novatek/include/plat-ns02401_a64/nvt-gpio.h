/**
	GPIO header file
	This file will setup gpio number
	@file       nvt-gpio.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/
#ifndef __NVT_GPIO_H
#define __NVT_GPIO_H

#include <linux/types.h>
#include <linux/irq.h>
#include "hardware.h"

#define NVT_GPIO_STG_DATA_0             0x00
#define NVT_GPIO_STG_DIR_0              0x20
#define NVT_GPIO_STG_SET_0              0x40
#define NVT_GPIO_STG_CLR_0              0x60
#define NVT_GPIO_STS_CPU                0x80
#define NVT_GPIO_STS_CPU2               0x94
#define NVT_GPIO_INTEN_CPU              0x120
#define NVT_GPIO_INTEN_CPU2             0x134
#define NVT_GPIO_TO_CPU                 0x1C0
#define NVT_GPIO_TO_CPU2                0x1D4
#define NVT_GPIO_INT_TYPE               0x260
#define NVT_GPIO_INT_POL                0x274
#define NVT_GPIO_EDGE_TYPE              0x288
#define NVT_GPIO_TOTAL_SIZE             0x2D0


#define NVT_GPIO_BASE_NUM    0x160

/* GPIO pin number translation  */
#define C_GPIO(pin)			(pin)
#define P_GPIO(pin)			(pin + 0x20)
#define D_GPIO(pin)			(pin + 0x60)
#define J_GPIO(pin)			(pin + 0x80)
#define E_GPIO(pin)			(pin + 0xA0)
#define B_GPIO(pin)			(pin + 0xC0)
#define A_GPIO(pin)			(pin + 0xE0)
#define INTERNAL_GPIO(pin)		(pin + 0x100)

#define C_GPIO_NUM			18
#define P_GPIO_NUM			48
#define D_GPIO_NUM			12
#define J_GPIO_NUM			6
#define E_GPIO_NUM			32
#define B_GPIO_NUM			17
#define A_GPIO_NUM			4
#define INTERNAL_GPIO_NUM		1

#endif /* __NVT_GPIO_H */
