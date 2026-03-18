/**
	GPIO header file
	This file will setup gpio related base address
	@file       nvt-gpio.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/
#ifndef __ASM_ARCH_NA51103_NVT_GPIO_H
#define __ASM_ARCH_NA51103_NVT_GPIO_H

#include <linux/types.h>
#include <linux/irq.h>
#include "hardware.h"

#define NVT_GPIO_STG_DATA_0  0x00
#define NVT_GPIO_STG_DIR_0   0x30
#define NVT_GPIO_STG_SET_0   0x60
#define NVT_GPIO_STG_CLR_0   0x90
#define NVT_GPIO_STS_CPU     0xC0
#define NVT_GPIO_STS_CPU2    0xD0
#define NVT_GPIO_INTEN_CPU   0x100
#define NVT_GPIO_INTEN_CPU2  0x110
#define NVT_GPIO_INT_TYPE    0x120
#define NVT_GPIO_INT_POL     0x130
#define NVT_GPIO_EDGE_TYPE   0x140
#define NVT_GPIO_TO_CPU      0x150
#define NVT_GPIO_TO_CPU2     0x160
#define NVT_GPIO_TOTAL_SIZE  0x170

/* GPIO pin number translation  */
#define C_GPIO(pin)                  (pin)
#define J_GPIO(pin)                  (pin + 0x20)
#define P_GPIO(pin)                  (pin + 0x40)
#define E_GPIO(pin)                  (pin + 0x60)
#define D_GPIO(pin)                  (pin + 0x80)
#define S_GPIO(pin)                  (pin + 0xA0)

#define C_GPIO_NUM                   8
#define J_GPIO_NUM                   5
#define P_GPIO_NUM                   32
#define E_GPIO_NUM                   16
#define D_GPIO_NUM                   10
#define S_GPIO_NUM                   36

/* GPIO total pin number */
#define NVT_GPIO_NUMBER              (S_GPIO(S_GPIO_NUM))

#endif /* __ASM_ARCH_NA51103_NVT_GPIO_H */
