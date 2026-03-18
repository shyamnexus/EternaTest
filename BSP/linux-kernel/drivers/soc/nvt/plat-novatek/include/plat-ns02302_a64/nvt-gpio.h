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
#define NVT_GPIO_STS_CPU2               0xF8
#define NVT_GPIO_INTEN_CPU              0x94
#define NVT_GPIO_INTEN_CPU2             0x10C
#define NVT_GPIO_INT_TYPE               0xA8
#define NVT_GPIO_INT_POL                0xBC
#define NVT_GPIO_EDGE_TYPE              0xD0
#define NVT_GPIO_TO_CPU                 0xE4
#define NVT_GPIO_TO_CPU2                0x120
#define NVT_GPIO_TOTAL_SIZE             0x144

#define NVT_GPIO_BASE_NUM    0x160

/* GPIO pin number translation  */
#define C_GPIO(pin)                  (pin)
#define P_GPIO(pin)                  (pin + 0x20)
#define S_GPIO(pin)                  (pin + 0x40)
#define L_GPIO(pin)                  (pin + 0x60)
#define D_GPIO(pin)                  (pin + 0x80)
#define HSI_GPIO(pin)                (pin + 0xA0)
#define A_GPIO(pin)                  (pin + 0xC0)
#define INTERNAL_GPIO(pin)           (pin + 0xE0)

#define C_GPIO_NUM			24
#define P_GPIO_NUM			26
#define S_GPIO_NUM			16
#define L_GPIO_NUM			25
#define D_GPIO_NUM			15
#define HSI_GPIO_NUM			24
#define A_GPIO_NUM			3
#define INTERNAL_GPIO_NUM		2 // interal 1 is for usb

#endif /* __NVT_GPIO_H */
