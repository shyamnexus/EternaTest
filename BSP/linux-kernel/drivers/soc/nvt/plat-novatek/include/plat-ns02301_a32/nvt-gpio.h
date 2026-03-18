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
#define NVT_GPIO_STG_DIR_0              0x10
#define NVT_GPIO_STG_SET_0              0x20
#define NVT_GPIO_STG_CLR_0              0x30
#define NVT_GPIO_STS_CPU                0x40
#define NVT_GPIO_STS_CPU2               0x64
#define NVT_GPIO_INTEN_CPU              0x4C
#define NVT_GPIO_INTEN_CPU2             0x70
#define NVT_GPIO_INT_TYPE               0x88
#define NVT_GPIO_INT_POL                0x94
#define NVT_GPIO_EDGE_TYPE              0xA0
#define NVT_GPIO_TO_CPU                 0x58
#define NVT_GPIO_TO_CPU2                0x7C
#define NVT_GPIO_TOTAL_SIZE             0xBC

/* GPIO pin number translation  */
#define NVT_GPIO_BASE_NUM    (0xa0)

/* GPIO pin number translation  */
#define C_GPIO(pin)                  (pin)
#define P_GPIO(pin)                  ((pin >= 25 && pin < 39) ? (pin + NVT_GPIO_BASE_NUM - 25) : ((pin == 39) ? (pin + 0x20 - 14) : (pin + 0x20)))
#define HSI_GPIO(pin)                (pin + 0x40)
#define D_GPIO(pin)                  (pin + 0x60)
#define INTERNAL_GPIO(pin)           (pin >= 1 ? ((pin - 1) + 0x60 + NVT_GPIO_BASE_NUM) : (pin + 0x80))

#define C_GPIO_NUM                      25
#define P_GPIO_NUM                      25
#define HSI_GPIO_NUM                    12
#define D_GPIO_NUM                      8
#define INTERNAL_GPIO_NUM               1

#define S_GPIO(pin)                  (pin + 0x20 + NVT_GPIO_BASE_NUM)
#define A_GPIO(pin)                  (pin + 0x40 + NVT_GPIO_BASE_NUM)

#endif /* __NVT_GPIO_H */
