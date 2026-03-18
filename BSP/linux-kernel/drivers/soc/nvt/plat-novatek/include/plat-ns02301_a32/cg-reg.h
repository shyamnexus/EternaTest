/*
	Register definition header file for CG module of NS02301.

	This file is the header file that define the address offset
	definition of registers of CG module.

	@file       cg-reg.h

	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _CG_REG_H
#define _CG_REG_H

#include "hardware.h"

#define NVT_PLL_MAX_NUMBER 12
#define CG_CPU_RATIO  0x8

/* Reset or not during clk_prepare() */
#define DO_RESET        1
#define NOT_RESET       0

/* Enable or not during clock tree construction */
#define DO_DISABLE	2
#define DO_ENABLE	1
#define NOT_ENABLE	0 // keep previous value

/* Enable or not during clock tree construction */
#define DO_AUTOGATING   1
#define NOT_AUTOGATING  0

#define DEFAULT_OSC_IN_CLK_FREQ 12000000
#define DEFAULT_PLL_DIV_VALUE   131072
#define DEFAULT_PLL3_RATIO    5133653  /* 0x4E5555 (470MHz for DMA1) */
#define DEFAULT_PLL5_RATIO    2621440  /* 2621440  (240MHz for SENSOR1) */
#define DEFAULT_PLL6_RATIO    3276800  /* 0x320000 (300MHz for IDE_ETH) */
#define DEFAULT_PLL7_RATIO    3758096  /* 0x395810 (344.064MHz for AUDIO) */
#define DEFAULT_PLL8_RATIO    0        /* 0x0      (CPU clock is determined by Loader) */
#define DEFAULT_PLL9_RATIO    6553600  /* 0x640000 (600MHz for IDE_ETH_BACKUP) */
#define DEFAULT_PLL11_RATIO   5242880  /* 0x500000 (480MHz for DIS) */
#define DEFAULT_PLL12_RATIO   2621440  /* 2621440  (240MHz for SENSOR2) */

/* PLL */
#define CG_PLL_EN_REG_OFFSET			0x0
#define CG_PLL_STAT_REG_OFFSET			0x4

/* Clock source control */
#define CG_SYS_CLK_RATE_REG_OFFSET			0x10
#define CG_IPP_CLK_RATE_REG0_OFFSET			0x14
#define CG_IPP_CLK_RATE_REG1_OFFSET			0x18
#define CG_CODEC_CLK_RATE_REG_OFFSET		0x1C
#define CG_PERI_CLK_RATE_REG0_OFFSET		0x20
#define CG_PERI_CLK_RATE_REG1_OFFSET		0x24
#define CG_PERI_CLK_RATE_REG2_OFFSET		0x28
#define CG_PERI_CLK_RATE_REG3_OFFSET		0x2C

/* Clock rate control */
#define CG_IPP_CLK_DIV_REG0_OFFSET			0x30
#define CG_VIDEO_CLK_DIV_REG0_OFFSET			0x34
#define CG_PERI_CLK_DIV_REG0_OFFSET			0x38
#define CG_SDIO_CLK_DIV_REG0_OFFSET			0x3C
#define CG_PERI_CLK_DIV_REG1_OFFSET			0x40
#define CG_SPI_CLK_DIV_REG0_OFFSET			0x44
#define CG_SPI_CLK_DIV_REG1_OFFSET			0x48
#define CG_UART_CLK_DIV_REG0_OFFSET			0x4C
#define CG_PWM_CLK_DIV_REG0_OFFSET			0x50
#define CG_PWM_CLK_DIV_REG1_OFFSET			0x54
#define CG_PWM_CLK_DIV_REG2_OFFSET			0x58
#define CG_PERI_CLK_DIV_REG2_OFFSET			0x60
#define CG_PERI_CLK_DIV_REG3_OFFSET			0x64

/* Clock enable/disable control */
#define CG_CLK_EN_REG0_OFFSET				0x70
#define CG_CLK_EN_REG1_OFFSET				0x74
#define CG_CLK_EN_REG2_OFFSET				0x78
#define CG_CLK_EN_REG3_OFFSET				0x7C

/* Module reset control */
#define CG_SYS_RESET_REG0_OFFSET			0x80
#define CG_SYS_RESET_REG1_OFFSET			0x84
#define CG_SYS_RESET_REG2_OFFSET			0x88
#define CG_SYS_RESET_REG3_OFFSET			0x8C

/* Low power control */
#define CG_SLEEP_MODE_REG_OFFSET			0xA0
#define CG_CLK_AUTO_GATING_REG0_OFFSET		0xB0
#define CG_CLK_AUTO_GATING_REG1_OFFSET		0xB4

#define CG_APB_CLK_AUTO_GATING_REG0_OFFSET	0xC0
#define CG_APB_CLK_AUTO_GATING_REG1_OFFSET	0xC4
#define CG_APB_CLK_AUTO_GATING_REG2_OFFSET	0xC8

/* Program Clock enable/disable control */
#define CG_PCLK_EN_REG0_OFFSET				0xE0
#define CG_PCLK_EN_REG1_OFFSET				0xE4

/* RO_32K Calibration Control Register */
#define CG_RO32K_CLK_CTRL_REG_OFFSET		0x100
/* RO_32K Calibration Result Register */
#define CG_RO32K_CLK_RESULT_REG_OFFSET		0x104
/* RO_32K Calibration Debug Register */
#define CG_RO32K_CLK_DEBUG_REG_OFFSET		0x108

/* Glitch Free MUX Type Register */
#define CG_GLITCH_FREE_CLK_REG_OFFSET			0x120

/* Sleep Mode Timing Register */
#define CG_SLEEP_MODE_TIM_CLK_REG_OFFSET		0x124

/* PLL clock rate control */
#define CG_PLL8_CLK_RATE0_REG_OFFSET			0x4420
#define CG_PLL11_CLK_RATE0_REG_OFFSET			0x4460
#define CG_PLL3_CLK_RATE0_REG_OFFSET			0x44A0
#define CG_PLL6_CLK_RATE0_REG_OFFSET			0x44E0
#define CG_PLL9_CLK_RATE0_REG_OFFSET			0x4520
#define CG_PLL5_CLK_RATE0_REG_OFFSET			0x4560
#define CG_PLL12_CLK_RATE0_REG_OFFSET			0x45A0
#define CG_PLL7_CLK_RATE0_REG_OFFSET			0x45E0

/* Preroll part */
#define PREROLL_OFFSET                                  0x100000

#define PRE_CG_FLL_EN_REG0_OFFSET                       PREROLL_OFFSET
#define PRE_CG_SLEEP_MODE_REG0_OFFSET                   (PREROLL_OFFSET + 0x4)
#define PRE_CG_PERI_WAKE_CTRL_REG0_OFFSET               (PREROLL_OFFSET + 0x8)
#define PRE_CG_RESERVED_REG0_OFFSET                     (PREROLL_OFFSET + 0xC)
#define PRE_CG_SYSTEM_CLK_RATE_REG0_OFFSET              (PREROLL_OFFSET + 0x10)
#define PRE_CG_IPP_CLOCK_RATE_REG0_OFFSET               (PREROLL_OFFSET + 0x14)
#define PRE_CG_PERI_CLOCK_DIV_REG0_OFFSET               (PREROLL_OFFSET + 0x18)
#define PRE_CG_PWM_CLOCK_RATE_REG0_OFFSET               (PREROLL_OFFSET + 0x1C)
#define PRE_CG_CLOCK_EN_REG0_OFFSET                     (PREROLL_OFFSET + 0x20)
#define PRE_CG_PROG_CLOCK_EN_REG0_OFFSET                (PREROLL_OFFSET + 0x24)
#define PRE_CG_SYSTEM_RESET_REG0_OFFSET                 (PREROLL_OFFSET + 0x28)
#define PRE_CG_CLK_AUTO_GATING_REG0_OFFSET              (PREROLL_OFFSET + 0x2C)
#define PRE_CG_APB_CLK_AUTO_GATING_REG0_OFFSET          (PREROLL_OFFSET + 0x30)
#define PRE_CG_SPI_DIV_REG0_OFFSET                      (PREROLL_OFFSET + 0x34)
#define PRE_CG_PERI_CLOCK_DIV_REG1_OFFSET               (PREROLL_OFFSET + 0x10C)
#define PRE_CG_MISC_CTRL_REG0_OFFSET                    (PREROLL_OFFSET + 0x120)
#define PRE_CG_FLL_ALT_CTRL_REG0_OFFSET                 (PREROLL_OFFSET + 0x124)

#define SPREAD_SPECTRUM_REG0_MINUS_OFFSET               (0x14) /* MPLL + 0xC */
#define SPREAD_SPECTRUM_REG1_MINUS_OFFSET               (0x10) /* MPLL + 0x10 */
#define SPREAD_SPECTRUM_REG2_MINUS_OFFSET               (0xC)  /* MPLL + 0x14 */
#define SPREAD_SPECTRUM_REG3_MINUS_OFFSET               (0x4)  /* MPLL + 0x1C */
#define SPREAD_SPECTRUM_STEP_OFFSET                     (0x4)
#define SPREAD_SPECTRUM_NEW_MODE_OFFSET                 (0x2)
#define SPREAD_SPECTRUM_DSSC_OFFSET                     (0x7)
#define SPREAD_SPECTRUM_PERIOD_VALUE                    (0xFF)

#endif
