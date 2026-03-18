/**
	GPIO header file
	This file will setup gpio number
	@file       nvt-gpio.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

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
#define NVT_GPIO_STG_DIR_0              0x30
#define NVT_GPIO_STG_SET_0              0x60
#define NVT_GPIO_STG_CLR_0              0x90
#define NVT_GPIO_STS_CPU                0xC0
#define NVT_GPIO_STS_CPU2               0xD8
#define NVT_GPIO_INTEN_CPU              0x120
#define NVT_GPIO_INTEN_CPU2             0x138
#define NVT_GPIO_INT_TYPE               0x180
#define NVT_GPIO_INT_POL                0x198
#define NVT_GPIO_EDGE_TYPE              0x1B0
#define NVT_GPIO_TO_CPU                 0x1C8
#define NVT_GPIO_TO_CPU2                0x1E0
#define NVT_GPIO_TOTAL_SIZE		0x230

#define NVT_GPIO_BASE_NUM    0x160

/* GPIO pin number translation  */
#define C_GPIO(pin)                  (pin)
#define P_GPIO(pin)                  (pin + 0x20)
#define D_GPIO(pin)                  (pin + 0x60)
#define L_GPIO(pin)                  (pin + 0x80)
#define S_GPIO(pin)                  (pin + 0xA0)
#define DSI_GPIO(pin)                (pin + 0xC0)
#define A_GPIO(pin)                  (pin + 0xE0)
#define INTERNAL_GPIO(pin)           (pin + 0x100)

#define C_GPIO_NUM			26
#define P_GPIO_NUM			47
#define D_GPIO_NUM			15
#define L_GPIO_NUM			31
#define S_GPIO_NUM			32
#define DSI_GPIO_NUM			14
#define A_GPIO_NUM			4
#define INTERNAL_GPIO_NUM		1

#endif /* __NVT_GPIO_H */
