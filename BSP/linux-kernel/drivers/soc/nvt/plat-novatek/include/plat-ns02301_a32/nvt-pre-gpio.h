/**
	GPIO header file
	This file will setup gpio number
	@file       nvt-pre-gpio.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/
#ifndef __NVT_PRE_GPIO_H
#define __NVT_PRE_GPIO_H

#include <linux/types.h>
#include <linux/irq.h>
#include "hardware.h"

#define NVT_PRE_GPIO_STG_DATA_0             0x00
#define NVT_PRE_GPIO_STG_DIR_0              0x0C
#define NVT_PRE_GPIO_STG_SET_0              0x18
#define NVT_PRE_GPIO_STG_CLR_0              0x24
#define NVT_PRE_GPIO_STS_CPU                0x30
#define NVT_PRE_GPIO_STS_CPU2               0x48
#define NVT_PRE_GPIO_INTEN_CPU              0x38
#define NVT_PRE_GPIO_INTEN_CPU2             0x50
#define NVT_PRE_GPIO_INT_TYPE               0x78
#define NVT_PRE_GPIO_INT_POL                0x80
#define NVT_PRE_GPIO_EDGE_TYPE              0x88
#define NVT_PRE_GPIO_TO_CPU                 0x40
#define NVT_PRE_GPIO_TO_CPU2                0x58
#define NVT_PRE_GPIO_TOTAL_SIZE             0xA0

#define NVT_PRE_GPIO_BASE_NUM    (0xa0)

#endif /* __NVT_PRE_GPIO_H */
