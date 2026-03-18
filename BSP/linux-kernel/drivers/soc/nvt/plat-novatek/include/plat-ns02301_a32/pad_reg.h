/*
	PAD controller register header

	PAD controller register header

	@file       pad_reg.h
	@ingroup    mIDrvSys_PAD
	@note       Nothing

	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _PAD_REG_H
#define _PAD_REG_H

#include <linux/soc/nvt/rcw_macro.h>

//0x00 PAD Pull UP/DOWN Register 0
#define PAD_PUPD0_REG_OFS                   0x00
#define PAD_PUPD1_REG_OFS                   0x04
#define PAD_PUPD2_REG_OFS                   0x08
#define PAD_PUPD3_REG_OFS                   0x0C
#define PAD_PUPD4_REG_OFS                   0x10
#define PAD_PUPD5_REG_OFS                   0x14
#define PAD_PUPD6_REG_OFS                   0x18
#define PAD_PUPD7_REG_OFS                   0x1C
#define PAD_PUPD8_REG_OFS                   0x20
#define PAD_PUPD9_REG_OFS                   0x24
#define PAD_DS_REG_OFS                      0x40
#define PAD_DS1_REG_OFS                     0x44
#define PAD_DS2_REG_OFS                     0x48
#define PAD_DS3_REG_OFS                     0x4C
#define PAD_DS4_REG_OFS                     0x50
#define PAD_DS5_REG_OFS                     0x54
#define PAD_DS6_REG_OFS                     0x58
#define PAD_DS7_REG_OFS                     0x5C
#define PAD_DS8_REG_OFS                     0x60
#define PAD_DS9_REG_OFS                     0x64
#define PAD_DS10_REG_OFS                    0x68
#define PAD_DS11_REG_OFS                    0x90

//0x78 PAD Power Control Register 0
#define PAD_PWR0_REG_OFS                     0x78
union PAD_PWR0_REG {
	uint32_t reg;
	struct {
	unsigned int ADC_POWER0:1;      // ADC power source. 0: 3.3V, 1: 1.8V
	unsigned int ADC_SEL:1;         // regulator bias current
	unsigned int ADC_OPSEL:1;       // regulator OPA gain/phase option
	unsigned int ADC_PULLDOWN:1;    // regulator output pull down control
	unsigned int ADC_REGULATOR_EN:1;// regulator enable
	unsigned int ADC_VAD:2;
	unsigned int reserved0:25;
	} bit;
};

#endif
