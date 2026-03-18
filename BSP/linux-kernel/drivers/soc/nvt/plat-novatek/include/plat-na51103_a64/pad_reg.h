/*
	PAD controller register header

	PAD controller register header

	@file       pad_reg.h
	@ingroup    mIDrvSys_PAD
	@note       Nothing

	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _PAD_REG_H
#define _PAD_REG_H

#include <linux/soc/nvt/rcw_macro.h>

// PAD Pull UP/DOWN Register 0
#define PAD_PUPD0_REG_OFS                    0x00

#if 0
// PAD Power Control Register 1
#define PAD_PWR1_REG_OFS                     0x74
union PAD_PWR1_REG {
	uint32_t reg;
	struct {
	unsigned int MC_POWER0:1;       // power source. 0: 3.3V, 1: 1.8V
	unsigned int MC_SEL:1;          // regulator bias current
	unsigned int MC_OPSEL:1;        // regulator OPA gain/phase option
	unsigned int MC_PULLDOWN:1;     // regulator output pull down control
	unsigned int MC_REGULATOR_EN:1; // regulator enable
	unsigned int reserved0:27;
	} bit;
};

// PAD Power Control Register 2
#define PAD_PWR2_REG_OFS                     0x78
union PAD_PWR2_REG {
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

// PAD LCD Delay Chain Control Register
#define PAD_LCDDLY_REG_OFS                  0xFC
union PAD_LCDDLY_REG {
	uint32_t reg;
	struct {
	unsigned int lcd_dly_vo_clk:5;
	unsigned int rev0:2;
	unsigned int lcd_inv_vo_clk:1;
	unsigned int rev1:8;
	unsigned int lcd_dly_vga_hs:5;
	unsigned int rev2:2;
	unsigned int lcd_inv_vga_hs:1;
	unsigned int lcd_dly_vga_vs:5;
	unsigned int rev3:2;
	unsigned int lcd_inv_vga_vs:1;
	} bit;
};

// PAD Power Control Register 3
#define PAD_PWR3_REG_OFS                     0x108
union PAD_PWR3_REG {
	uint32_t reg;
	struct {
	unsigned int reserved0:1;
	unsigned int CSI_SEL:1;         // regulator bias current
	unsigned int CSI_OPSEL:1;       // regulator OPA gain/phase option
	unsigned int CSI_PULLDOWN:1;    // regulator output pull down control
	unsigned int CSI_REGULATOR_EN:1;// regulator enable
	unsigned int CSI_VAD:2;
	unsigned int reserved1:25;
	} bit;
};
#endif
