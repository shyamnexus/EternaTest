/*
	Register definition header file for CG module of NA51055.

	This file is the header file that define the address offset
	definition of registers of CG module.

	@file       cg-reg.h

	Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _CG_REG_H
#define _CG_REG_H

#include "hardware.h"

/* PLL */
#define CG_PLL_EN_REG_OFFSET				0x00
#define CG_PLL_STAT_REG_OFFSET				0x04

/* Clock source control */
#define CG_SYS_CLK_RATE_REG_OFFSET			0x10
#define CG_IPP_CLK_RATE_REG0_OFFSET			0x14
//#define CG_IPP_CLK_RATE_REG1_OFFSET			0x18
#define CG_CODEC_CLK_RATE_REG_OFFSET			0x1C
#define CG_PERI_CLK_RATE_REG0_OFFSET			0x20
#define CG_PERI_CLK_RATE_REG1_OFFSET			0x24
#define CG_PERI_CLK_RATE_REG2_OFFSET			0x28
#define CG_PERI_CLK_RATE_REG3_OFFSET			0x2C

/* Clock rate control */
#define CG_IPP_CLK_DIV_REG0_OFFSET			0x30
#define CG_PWM_CLK_DIV_REG0_OFFSET			0x34
#define CG_PWM_CLK_DIV_REG1_OFFSET			0x38
#define CG_DAI_MCLK_DIV_REG_OFFSET			0x3C
#define CG_SDIO_CLK_DIV_REG_OFFSET			0x40
#define CG_UART_CLK_DIV_REG_OFFSET			0x44
#define CG_PERI_CLK_DIV_REG0_OFFSET			0x48
#define CG_PERI_CLK_DIV_REG1_OFFSET			0x4C
#define CG_PERI_CLK_DIV_REG2_OFFSET			0x50
#define CG_PERI_CLK_DIV_REG3_OFFSET			0x54
#define CG_DAI_CLK_DIV_REG0_OFFSET			0x58
#define CG_DAI_CLK_DIV_REG1_OFFSET			0x5C

/* Clock enable/disable control */
#define CG_CLK_EN_REG0_OFFSET				0x70
#define CG_CLK_EN_REG1_OFFSET				0x74
#define CG_CLK_EN_REG2_OFFSET				0x78
#define CG_CLK_EN_REG3_OFFSET				0x7C
#define CG_CLK_EN_REG4_OFFSET				0x80

/* Module reset control */
#define CG_SYS_RESET_REG0_OFFSET			0x90
#define CG_SYS_RESET_REG1_OFFSET			0x94
#define CG_SYS_RESET_REG2_OFFSET			0x98
#define CG_SYS_RESET_REG3_OFFSET			0x9C
#define CG_SYS_RESET_REG4_OFFSET			0xA0

/* Low power control */
#define CG_SLEEP_MODE_REG_OFFSET			0xB0
#define CG_CLK_AUTO_GATING_REG0_OFFSET			0xC0
#define CG_CLK_AUTO_GATING_REG1_OFFSET			0xC4
#define CG_APB_CLK_AUTO_GATING_REG0_OFFSET		0xD0
#define CG_APB_CLK_AUTO_GATING_REG1_OFFSET		0xD4
#define CG_APB_CLK_AUTO_GATING_REG2_OFFSET		0xD8

/* Extclk source control */
#define CG_EXT_CLK_OUT_SRC_REG_OFFSET		0xF0

#define CG_PLL_MCU_FLAG_REG_OFFSET			0x1180

/* PLL clock rate control */
#define CG_PLL0_CLK_RATE0_REG_OFFSET 			0x4420
#define CG_PLL2_CLK_RATE0_REG_OFFSET 			0x44A0
#define CG_PLL3_CLK_RATE0_REG_OFFSET			0x44E0
#define CG_PLL4_CLK_RATE0_REG_OFFSET			0x4520
#define CG_PLL5_CLK_RATE0_REG_OFFSET			0x4560
#define CG_PLL6_CLK_RATE0_REG_OFFSET			0x45A0
#define CG_PLL7_CLK_RATE0_REG_OFFSET			0x45E0
#define CG_PLL8_CLK_RATE0_REG_OFFSET			0x4620
#define CG_PLL9_CLK_RATE0_REG_OFFSET			0x4660
#define CG_PLL10_CLK_RATE0_REG_OFFSET			0x46A0
#define CG_PLL11_CLK_RATE0_REG_OFFSET			0x46E0
#define CG_PLL12_CLK_RATE0_REG_OFFSET			0x4720
#define CG_PLL13_CLK_RATE0_REG_OFFSET			0x4760
#define CG_PLL14_CLK_RATE0_REG_OFFSET			0x47A0
#define CG_PLL15_CLK_RATE0_REG_OFFSET			0x47E0
#define CG_PLL16_CLK_RATE0_REG_OFFSET			0x4820
#define CG_PLL17_CLK_RATE0_REG_OFFSET			0x4860
#define CG_PLL18_CLK_RATE0_REG_OFFSET			0x48A0
#define CG_PLL19_CLK_RATE0_REG_OFFSET			0x48E0
#define CG_PLL20_CLK_RATE0_REG_OFFSET			0x4920

#endif
