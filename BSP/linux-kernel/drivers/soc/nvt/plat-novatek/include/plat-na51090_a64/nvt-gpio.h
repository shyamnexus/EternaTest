/**
	GPIO header file
	This file will setup gpio related base address
	@file       nvt-gpio.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/
#ifndef __ASM_ARCH_NA51090_NVT_GPIO_H
#define __ASM_ARCH_NA51090_NVT_GPIO_H

#include <linux/types.h>
#include <linux/irq.h>
#include "hardware.h"

#define NVT_GPIO_STG_DATA_0  0x00
#define NVT_GPIO_STG_DIR_0   0x30
#define NVT_GPIO_STG_SET_0   0x60
#define NVT_GPIO_STG_CLR_0   0x90
#define NVT_GPIO_STS_CPU     0xC0
#define NVT_GPIO_STS_CPU2    0xD0
#define NVT_GPIO_STS_CPU3    0xE0
#define NVT_GPIO_STS_CPU4    0xF0
#define NVT_GPIO_INTEN_CPU   0x100
#define NVT_GPIO_INTEN_CPU2  0x110
#define NVT_GPIO_INTEN_CPU3  0x120
#define NVT_GPIO_INTEN_CPU4  0x130
#define NVT_GPIO_INT_TYPE    0x140
#define NVT_GPIO_INT_POL     0x150
#define NVT_GPIO_EDGE_TYPE   0x160
#define NVT_GPIO_TO_CPU      0x170

#define NVT_GPIO_BASE_NUM    0x160
#define NVT_GPIO_TOTAL_SIZE  0x1B0

/* GPIO pin number translation  */
#define C_GPIO(pin)                  (pin)
#define J_GPIO(pin)                  (pin + 0x20)
#define P_GPIO(pin)                  (pin + 0x40)
#define E_GPIO(pin)                  (pin + 0x80)
#define D_GPIO(pin)                  (pin + 0xA0)
#define S_GPIO(pin)                  (pin + 0xC0)
#define B_GPIO(pin)                  (pin + 0x120)

#define EP_C_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin)
#define EP_J_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0x20)
#define EP_P_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0x40)
#define EP_E_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0x80)
#define EP_D_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0xA0)
#define EP_S_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0xC0)
#define EP_B_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0x120)

#define C_GPIO_NUM                   14
#define J_GPIO_NUM                   6
#define P_GPIO_NUM                   45
#define E_GPIO_NUM                   32
#define D_GPIO_NUM                   12
#define S_GPIO_NUM                   84
#define B_GPIO_NUM                   17

#define NVT_GPIO_INTEN_CPU_NUM       4
#define NVT_GPIO_CPU_REG_OFFSET      (NVT_GPIO_INTEN_CPU2 - NVT_GPIO_INTEN_CPU)

#define IRQ_SPI_START                32
#define IRQ_SPI_END                  256

#define NVT_GPIO_INT_EXT_TOTAL       128

#ifndef CONFIG_SPARSE_IRQ
#define NR_IRQS (256 + NVT_GPIO_INT_EXT_TOTAL)
#endif

#define NVT_IRQ_GPIO_EXT_START       (IRQ_SPI_END)

/* GPIO total pin number */
#define NVT_GPIO_NUMBER              (320)
#endif /* __ASM_ARCH_NA51090_NVT_GPIO_H */
