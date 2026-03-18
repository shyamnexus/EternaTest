/**
    NVT clock management module for NA51090 SoC
    @file na51090-clk-ep.c

    Copyright Novatek Microelectronics Corp. 2024. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/spinlock.h>
#include <linux/of_address.h>
#include <plat/cg-reg.h>
#include "../v2/nvt-im-clk-fr.h"
#include "na51090-maxfreq.h"
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <plat/top.h>

#define IOADDR_CG_REG_SIZE 0x12A0

#define DEFAULT_OSC_IN_CLK_FREQ 12000000
#define DEFAULT_PLL_DIV_VALUE   131072
#define DEFAULT_PLL0_RATIO    5461333  /* 0x535555 (500MHz for AXI0) */
#define DEFAULT_PLL2_RATIO    4369066  /* 0x42AAAA (400MHz for AXI1) */
#define DEFAULT_PLL3_RATIO    2184533  /* 0x215555 (200MHz for DRAM) */
#define DEFAULT_PLL4_RATIO    3276800  /* 0x320000 (300MHz for AHB) */
#define DEFAULT_PLL5_RATIO    1092266  /* 0x10AAAA (100MHz for SATA_PHY) */
#define DEFAULT_PLL6_RATIO    2184533  /* 0x215555 (200MHz for SDC) */
#define DEFAULT_PLL7_RATIO    2730666  /* 0x29AAAA (250MHz for MAC) */
#define DEFAULT_PLL8_RATIO    6553600  /* 0x640000 (600MHz for CNN) */
#define DEFAULT_PLL9_RATIO    3244032  /* 0x318000 (297MHz for DISPLAY0) */
#define DEFAULT_PLL10_RATIO   2730666  /* 0x29AAAA (250MHz for DISPALY1) */
#define DEFAULT_PLL11_RATIO    589824  /* 0x90000  (54MHz for DISPLAY2) */
#define DEFAULT_PLL12_RATIO   4026531  /* 0x3D70A3 (368.64MHz for SSP) */
#define DEFAULT_PLL13_RATIO   3822933  /* 0x3A5555 (350MHz for AXI2) */
#define DEFAULT_PLL14_RATIO   4150613  /* 0x3F5555 (380MHz for VENC) */
#define DEFAULT_PLL15_RATIO   5461333  /* 0x535555 (500MHz for IPP, Codec bakup) */
#define DEFAULT_PLL16_RATIO   1911466  /* 0x1D2AAA (175MHz for CPU) */
#define DEFAULT_PLL17_RATIO   2184533  /* 0x215555 (200MHz for DRAM2) */
#define DEFAULT_PLL18_RATIO   4915200  /* 0x4B0000 (450MHz for VDEC) */
#define DEFAULT_PLL19_RATIO   4915200  /* 0x4B0000 (450MHz for VCAP) */
#define DEFAULT_PLL20_RATIO   5461333  /* 0x535555 (500MHz for USB3_PHY) */

static void __iomem *remap_ep_base = NULL;
static spinlock_t cg_lock;

static struct nvt_pll_clk novatek_pll_ep[] = {
	PLL_CONF("pll0_ep0", DEFAULT_PLL0_RATIO, CG_PLL0_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT0,
		 CG_EP0_BASE),
	PLL_CONF("pll2_ep0", DEFAULT_PLL0_RATIO, CG_PLL2_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT2,
		 CG_EP0_BASE),
	PLL_CONF("pll3_ep0", DEFAULT_PLL3_RATIO, CG_PLL3_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT3,
		 CG_EP0_BASE),
	PLL_CONF("pll4_ep0", DEFAULT_PLL4_RATIO, CG_PLL4_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT4,
		 CG_EP0_BASE),
	PLL_CONF("pll5_ep0", DEFAULT_PLL5_RATIO, CG_PLL5_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT5,
		 CG_EP0_BASE),
	PLL_CONF("pll6_ep0", DEFAULT_PLL6_RATIO, CG_PLL6_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT6,
		 CG_EP0_BASE),
	PLL_CONF("pll7_ep0", DEFAULT_PLL7_RATIO, CG_PLL7_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT7,
		 CG_EP0_BASE),
	PLL_CONF("pll8_ep0", DEFAULT_PLL8_RATIO, CG_PLL8_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT8,
		 CG_EP0_BASE),
	PLL_CONF("pll9_ep0", DEFAULT_PLL9_RATIO, CG_PLL9_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT9,
		 CG_EP0_BASE),
	PLL_CONF("pll10_ep0", DEFAULT_PLL10_RATIO, CG_PLL10_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT10,
		 CG_EP0_BASE),
	PLL_CONF("pll11_ep0", DEFAULT_PLL11_RATIO, CG_PLL11_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT11,
		 CG_EP0_BASE),
	PLL_CONF("pll12_ep0", DEFAULT_PLL12_RATIO, CG_PLL12_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT12,
		 CG_EP0_BASE),
	PLL_CONF("pll13_ep0", DEFAULT_PLL13_RATIO, CG_PLL13_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT13,
		 CG_EP0_BASE),
	PLL_CONF("pll14_ep0", DEFAULT_PLL14_RATIO, CG_PLL14_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT14,
		 CG_EP0_BASE),
	PLL_CONF("pll15_ep0", DEFAULT_PLL15_RATIO, CG_PLL15_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT15,
		 CG_EP0_BASE),
	PLL_CONF("pll16_ep0", DEFAULT_PLL16_RATIO, CG_PLL16_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT16,
		 CG_EP0_BASE),
	PLL_CONF("pll17_ep0", DEFAULT_PLL17_RATIO, CG_PLL17_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT17,
		 CG_EP0_BASE),
	PLL_CONF("pll18_ep0", DEFAULT_PLL18_RATIO, CG_PLL18_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT18,
		 CG_EP0_BASE),
	PLL_CONF("pll19_ep0", DEFAULT_PLL19_RATIO, CG_PLL19_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT19,
		 CG_EP0_BASE),
	PLL_CONF("pll20_ep0", DEFAULT_PLL20_RATIO, CG_PLL20_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT20,
		 CG_EP0_BASE),
};

static struct nvt_composite_gate_clk novatek_cgate_clk_ep[] = {
	COMP_GATE_CONF("h264d_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT15, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("h265d_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT16, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("osg_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT17, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("jpeg_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT8, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("venc_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT13, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("vecn2_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT14, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("nue_pclk_ep", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT27, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("nue2_pclk_ep", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT28, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("cnn_pclk_ep", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT2, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("cnn2_pclk_ep", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT3, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("cnn3_pclk_ep", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT4, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620280000.uart", "fix480m", 48000000,
		       CG_PERI_CLK_DIV_REG0_OFFSET, BIT0, WID8,
		       CG_CLK_EN_REG2_OFFSET, BIT24, DO_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT4, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT24, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("uart6_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT20, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("6201c0000.i2c", "fix480m", 48000000,
		       0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT16, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT28, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT16, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("6201d0000.i2c2", "fix480m", 48000000,
		       0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT17, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT29, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT17, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("6201e0000.i2c3", "fix480m", 48000000,
		       0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT18, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT30, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT18, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("6201f0000.i2c4", "fix480m", 48000000,
		       0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT19, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT31, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT19, DO_AUTOGATING,
		       CG_EP0_BASE),

	COMP_GATE_CONF("dei_mclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT4, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("dei2_mclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT5, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("vpe_mclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT6, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("vpel_mclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT7, NOT_AUTOGATING,
		       CG_EP0_BASE),

	COMP_GATE_CONF("620510000.tmep0", "osc_in", 12000000,
		       0, 0, 0,				// clk div
		       CG_CLK_EN_REG2_OFFSET, BIT27, NOT_ENABLE,		// 0x78:CG_CLK_EN_REG2_OFFSET bit[27] module gating
		       CG_SYS_RESET_REG3_OFFSET, BIT7, NOT_RESET,		// module reset
		       0, 0, 0,				// auto gating
		       CG_EP0_BASE),
	COMP_GATE_CONF("ssca_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT19, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("age_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT20, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620330000.hwcp", "pll2", 400000000,
		       0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT9, DO_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT0, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT21, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620120000.pwm0", "fix120m", 30000000,
		       CG_PWM_CLK_DIV_REG0_OFFSET, BIT0, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT0, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT9, NOT_RESET,
		       0, 0, 0,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620120000.pwm1", "fix120m", 30000000,
		       CG_PWM_CLK_DIV_REG0_OFFSET, BIT16, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT1, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT9, NOT_RESET,
		       0, 0, 0,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620120000.pwm2", "fix120m", 30000000,
		       CG_PWM_CLK_DIV_REG1_OFFSET, BIT0, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT2, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT9, NOT_RESET,
		       0, 0, 0,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620120000.pwm3", "fix120m", 30000000,
		       CG_PWM_CLK_DIV_REG1_OFFSET, BIT16, WID14,
		       CG_CLK_EN_REG3_OFFSET, BIT3, NOT_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT9, NOT_RESET,
		       0, 0, 0,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620100000.timer", "fix480m", 48000000,
		       0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT26, DO_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT6, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT26, DO_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("2f0200000.eth0e", "pll7", 250000000,
		       0, 0, 0,
		       CG_CLK_EN_REG0_OFFSET, BIT27, DO_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT16, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT8, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0210000.eth1e", "pll7", 250000000,
		       0, 0, 0,
		       CG_CLK_EN_REG0_OFFSET, BIT28, DO_ENABLE,
		       CG_SYS_RESET_REG1_OFFSET, BIT18, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT9, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("eth0_extphy_clk_ep0", "fix480m", 2500000,
		       0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT7, DO_ENABLE,
		       0, 0, 0,
		       0, 0, 0,
		       0),
	COMP_GATE_CONF("eth1_extphy_clk_ep0", "fix480m", 2500000,
		       0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT8, DO_ENABLE,
		       0, 0, 0,
		       0, 0, 0,
		       0),
	COMP_GATE_CONF("lcd310_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT10, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("lcd310_2_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT11, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("lcd210_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT12, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("hdmi_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT18, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620380000.usb20", "pll20_ep0", 48000000, 0, 0, 0,
		       0, 0, 0,
		       CG_SYS_RESET_REG2_OFFSET, BIT15, DO_RESET,
		       0, 0, 0,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620390000.usb20", "fix48m", 48000000, 0, 0, 0,
		       0, 0, 0,
		       CG_SYS_RESET_REG2_OFFSET, BIT16, DO_RESET,
		       0, 0, 0,
		       CG_EP0_BASE),
	COMP_GATE_CONF("620580000.u3hst", "pll20_ep0", 150000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT26, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT18, DO_RESET,
		       0, 0, 0,
		       CG_EP0_BASE),
	COMP_GATE_CONF("u3phy_ep0", "pll20_ep0", 150000000, 0, 0, 0,
		       CG_SYS_RESET_REG3_OFFSET, BIT15, NOT_ENABLE,
		       0, 0, 0,
		       0, 0, 0,
		       CG_EP0_BASE),
	COMP_GATE_CONF("pll20_ss_ep0", "osc_in", 0,
		       CG_PLL20_CLK_BASE_REG_OFFSET + CG_PLL_SPREAD_SPECTRUM_REG0_OFFSET, BIT20, 0,
		       CG_PLL20_CLK_BASE_REG_OFFSET + CG_PLL_SPREAD_SPECTRUM_REG1_OFFSET, BIT20, 0,
		       CG_PLL20_CLK_BASE_REG_OFFSET + CG_PLL_SPREAD_SPECTRUM_REG2_OFFSET, BIT20, 0,
		       CG_PLL20_CLK_BASE_REG_OFFSET + CG_PLL_SPREAD_SPECTRUM_REG3_OFFSET, BIT20, 0,
		       CG_EP0_BASE),

	COMP_GATE_CONF("dai_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT0, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("dai2_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT1, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("dai3_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT2, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("dai4_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT3, NOT_AUTOGATING,
		       CG_EP0_BASE),
	COMP_GATE_CONF("dai5_pclk_ep0", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT4, NOT_AUTOGATING,
		       CG_EP0_BASE),
};

/* osg */
static const char *osg_clk_parent_ep0[] = {
	"pll2_ep0", "pll13_ep0"
};

/* h264dec */
static const char *h264d_mclk_parent_ep0[] = {
	"pll18_ep0", "pll0_ep0"
};
static const char *h264d_aclk_parent_ep0[] = {
	"pll18_ep0", "pll13_ep0", "pll0_ep0"
};

/* h265dec */
static const char *h265d_mclk_parent_ep0[] = {
	"pll18_ep0", "pll0_ep0"
};
static const char *h265d_aclk_parent_ep0[] = {
	"pll13_ep0", "pll2_ep0"
};

static const char *venc_clk_parent_ep0[] = {
	"pll14_ep0", "reserved", "pll15_ep0", "reserved"
};

static const char *jpeg_clk_parent_ep0[] = {
	"fix480m", "fix320m", "pll15_ep0", "reserved"
};
static const char *nue_clk_parent_ep0[] = {
	"pll8_ep0", "pll15_ep0"
};

static const char *nue2_clk_parent_ep0[] = {
	"pll8_ep0", "pll15_ep0"
};
static const char *cnn_clk_parent_ep0[] = {
	"pll8_ep0", "pll15_ep0"
};

static const char *cnn2_clk_parent_ep0[] = {
	"pll8_ep0", "pll15_ep0"
};

static const char *cnn3_clk_parent_ep0[] = {
	"pll8_ep0", "pll15_ep0"
};
static const char *ive_clk_parent_ep0[] = {
	"fix480m", "fix320m", "fix240m", "reserved"
};
/* vcap */
static const char *vcap_aclk_m_parent_ep0[] = {
	"pll0_ep0",
};
static const char *vcap_mclk_parent_ep0[] = {
	"pll19_ep0", "pll0_ep0"
};
static const char *vcap_patclk_parent_ep0[] = {
	"pll9_ep0", "pll6_ep0", "pll11_ep0"
};

static const char *vpe_clk_parent_ep0[] = {
	"pll2_ep0", "fix320m", "pll15_ep0", "reserved"
};
static const char *vpel_clk_parent_ep0[] = {
	"pll2_ep0", "fix320m", "pll15_ep0", "reserved"
};
static const char *dei_clk_parent_ep0[] = {
	"pll2_ep0", "fix320m", "pll15_ep0", "reserved"
};

static const char *dei2_clk_parent_ep0[] = {
	"pll2_ep0", "fix320m", "pll15_ep0", "reserved"

};

static const char *age_clk_parent_ep0[] = {
	"pll4_ep0",
};

static const char *ssca_clk_parent_ep0[] = {
	"pll4_ep0", "pll2_ep0"
};

static const char *ext_clk_parent_ep0[] = {
	"pll9_ep0", "pll7_ep0", "pll12_ep0", "osc_in"
};

static const char *audio_clk_parent[] = {
	"pll12",
};

static const char *audio_clk_parent_ep0[] = {
	"pll12_ep0",
};

static const char *root_clk_parent_ep0[] = {
	"osc_in",
};

static const char *sata_aclk_parent_ep0[] = {
	"pll4_ep0",
};

static const char *lcd210_clk_parent_ep0[] = {
	"pll9_ep0", "pll10_ep0", "pll11_ep0",
};

static const char *lcd310_clk_parent_ep0[] = {
	"pll9_ep0", "pll10_ep0", "pll11_ep0", "pll15_ep0",
};

static const char *lcd310_mclk_parent_ep0[] = {
	"pll0_ep0", "pll2_ep0",
};

static const char *lcd310lite_mclk_parent_ep0[] = {
	"pll0_ep0", "pll2_ep0", "pll2_ep0",
};

static const char *extclk_form_parent[] = {
	"ext", "ext2", "ext3", "ext4"
};
static const char *extclk2_form_parent[] = {
	"ext2", "ext3", "ext4", "ext"
};
static const char *extclk3_form_parent[] = {
	"ext3", "ext4", "ext", "ext2"
};
static const char *extclk4_form_parent[] = {
	"ext4", "ext", "ext2", "ext3"
};
static const char *csi_clk_parent_ep0[] = {
	"fix60m", "fix120m"
};

static const char *csi_pxclk_parent_ep0[] = {
	"pll4_ep0", "pll2_ep0", "pll13_ep0"
};

static struct nvt_composite_mux_clk novatek_cmux_clk_ep[] = {
	COMP_MUX_CONF("h264d_mclk_ep0", h264d_mclk_parent_ep0, 45000000, 0, 	/* mclk default is pll18 with 450M */
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT12, WID1, 	    /* 0x1c, clock select */
		      0, 0, 0,											/* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT2, NOT_ENABLE, 			/* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG1_OFFSET, BIT26, DO_RESET, 		/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT18, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("h264d_aclk_ep0", h264d_aclk_parent_ep0, 45000000, 0, 	/* mclk default is pll18 with 450M */
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT8, WID2, 	    /* 0x1c, clock select */
		      0, 0, 0,											/* div_reg */
		      0, 0, 0, 											/* 0x74, Module Clock Enable Register 1 */
		      0, 0, 0, 											/* 0x94, System Reset Register1 */
		      0, 0, 0, 										/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("h265d_mclk_ep0", h265d_mclk_parent_ep0, 45000000, 0, 	/* mclk default is pll18 with 450M */
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT14, WID1, 	    /* 0x1c, clock select */
		      0, 0, 0,											/* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT3, NOT_ENABLE, 			/* 0x74, Module Clock Enable Register 1 */
		      0, 0, 0, 											/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT19, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("h265d_aclk_ep0", h265d_aclk_parent_ep0, 35000000, 0, /* mclk default is pll12 with 350M */
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT10, WID1, 	    /* 0x1c, clock select */
		      0, 0, 0,											/* div_reg */
		      0, 0, 0, 											/* 0x74, Module Clock Enable Register 1 */
		      0, 0, 0, 											/* 0x94, System Reset Register1 */
		      0, 0, 0, 										/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("620500000.venc", venc_clk_parent_ep0, 38000000, 0,		/* mclk default is pll14 with 380M */
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT4, WID2,					/* 0x4, clock select */
		      0, 0, 0,													/* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT0, NOT_ENABLE,					/* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG1_OFFSET, BIT24, DO_RESET,					/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT16, NOT_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("620a00000.venc", venc_clk_parent_ep0, 38000000, 0,		/* mclk default is pll14 with 380M */
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT4, WID2,					/* 0x4, clock select */
		      0, 0, 0,													/* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT1, NOT_ENABLE,					/* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG1_OFFSET, BIT25, DO_RESET,					/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT17, NOT_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("620320000.jpeg", jpeg_clk_parent_ep0, 48000000, 0,		/* mclk default is fix480m with 480M */
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT0, WID2,					/* 0x0, clock select */
		      0, 0, 0,													/* div_reg */
		      CG_CLK_EN_REG0_OFFSET, BIT22, NOT_ENABLE,					/* 0x70, Module Clock Enable Register 0 */
		      CG_SYS_RESET_REG1_OFFSET, BIT10, DO_RESET,					/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT6, NOT_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("620300000.osg", osg_clk_parent_ep0, 40000000, 0, 	/* mclk default is pll2 with 400M */
		      CG_CODEC_CLK_RATE_REG_OFFSET, BIT16, WID1, 				/* 0x1c, clock select */
		      0, 0, 0, 													/* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT4, NOT_ENABLE, 					/* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG1_OFFSET, BIT28, DO_RESET, 				/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT20, NOT_AUTOGATING,	/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("vcap_aclk_m0.1", vcap_aclk_m_parent_ep0, 500000000, 0,   /* MCLK default is PLL0 with 500MHz               */
		      0, 0, 0,                                                      /* mux_reg,                                       */
		      0, 0, 0,                                                      /* div_reg                                        */
		      CG_CLK_EN_REG2_OFFSET, BIT0, NOT_ENABLE,                      /* gate_reg  0x78, Module Clock Enable Register 2 */
		      0, 0, 0,                                                      /* reset_reg                                      */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("vcap_aclk_m1.1", vcap_aclk_m_parent_ep0, 500000000, 0,   /* MCLK default is PLL0 with 500MHz               */
		      0, 0, 0,                                                      /* mux_reg,                                       */
		      0, 0, 0,                                                      /* div_reg                                        */
		      CG_CLK_EN_REG2_OFFSET, BIT1, NOT_ENABLE,                      /* gate_reg  0x78, Module Clock Enable Register 2 */
		      0, 0, 0,                                                      /* reset_reg                                      */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("vcap_aclk_m2.1", vcap_aclk_m_parent_ep0, 500000000, 0,   /* MCLK default is PLL0 with 500MHz               */
		      0, 0, 0,                                                      /* mux_reg,                                       */
		      0, 0, 0,                                                      /* div_reg                                        */
		      CG_CLK_EN_REG2_OFFSET, BIT2, NOT_ENABLE,                      /* gate_reg  0x78, Module Clock Enable Register 2 */
		      0, 0, 0,                                                      /* reset_reg                                      */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("vcap_mclk.1", vcap_mclk_parent_ep0, 400000000, 0,        /* MCLK default is PLL19 with 400MHz              */
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT20, WID1,                     /* mux_reg   0x14, IPP Clock Rate Register        */
		      0, 0, 0,                                                      /* div_reg                                        */
		      CG_CLK_EN_REG2_OFFSET, BIT3, NOT_ENABLE,                      /* gate_reg  0x78, Module Clock Enable Register 2 */
		      CG_SYS_RESET_REG2_OFFSET, BIT24, DO_RESET,                    /* reset_reg 0x98, System Reset Register 2        */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("vcap_patclk.1", vcap_patclk_parent_ep0, 74250000, 0,     /* MCLK default is PLL9 with 74.25MHz             */
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT16, WID2,                     /* mux_reg   0x14, IPP Clock Rate Register        */
		      CG_IPP_CLK_DIV_REG0_OFFSET, BIT0, WID6,                       /* div_reg   0x30, IPP Clock Divider Register     */
		      CG_CLK_EN_REG2_OFFSET, BIT4, NOT_ENABLE,                      /* gate_reg  0x78, Module Clock Enable Register 2 */
		      0, 0, 0,                                                      /* reset_reg                                      */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("vcap2_aclk_m0.1", vcap_aclk_m_parent_ep0, 500000000, 0,  /* MCLK default is PLL0 with 500MHz               */
		      0, 0, 0,                                                      /* mux_reg,                                       */
		      0, 0, 0,                                                      /* div_reg                                        */
		      CG_CLK_EN_REG2_OFFSET, BIT8, NOT_ENABLE,                      /* gate_reg  0x78, Module Clock Enable Register 2 */
		      0, 0, 0,                                                      /* reset_reg                                      */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("vcap2_aclk_m1.1", vcap_aclk_m_parent_ep0, 500000000, 0,  /* MCLK default is PLL0 with 500MHz               */
		      0, 0, 0,                                                      /* mux_reg,                                       */
		      0, 0, 0,                                                      /* div_reg                                        */
		      CG_CLK_EN_REG2_OFFSET, BIT9, NOT_ENABLE,                      /* gate_reg  0x78, Module Clock Enable Register 2 */
		      0, 0, 0,                                                      /* reset_reg                                      */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("vcap2_aclk_m2.1", vcap_aclk_m_parent_ep0, 500000000, 0,  /* MCLK default is PLL0 with 500MHz               */
		      0, 0, 0,                                                      /* mux_reg,                                       */
		      0, 0, 0,                                                      /* div_reg                                        */
		      CG_CLK_EN_REG2_OFFSET, BIT10, NOT_ENABLE,                     /* gate_reg  0x78, Module Clock Enable Register 2 */
		      0, 0, 0,                                                      /* reset_reg                                      */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("vcap2_mclk.1", vcap_mclk_parent_ep0, 400000000, 0,       /* MCLK default is PLL19 with 400M                */
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT22, WID1,                     /* mux_reg   0x14, IPP Clock Rate Register        */
		      0, 0, 0,                                                      /* div_reg                                        */
		      CG_CLK_EN_REG2_OFFSET, BIT11, NOT_ENABLE,                     /* gate_reg  0x78, Module Clock Enable Register 2 */
		      CG_SYS_RESET_REG2_OFFSET, BIT25, DO_RESET,                    /* reset_reg 0x98, System Reset Register 2        */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("vcap2_patclk.1", vcap_patclk_parent_ep0, 74250000, 0,    /* MCLK default is PLL9 with 74.25MHz             */
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT18, WID2,                     /* mux_reg   0x14, IPP Clock Rate Register        */
		      CG_IPP_CLK_DIV_REG0_OFFSET, BIT8, WID6,                       /* div_reg   0x30, IPP Clock Divider Register     */
		      CG_CLK_EN_REG2_OFFSET, BIT12, NOT_ENABLE,                     /* gate_reg  0x78, Module Clock Enable Register 2 */
		      0, 0, 0,                                                      /* reset_reg                                      */
		      0, 0, 0,                                                      /* autogating_reg                                 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("620410000.vpe", vpe_clk_parent_ep0, 400000000, 0,       /* mclk default is pll2 with 400M */
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT24, WID2,         /* 0x14, clock select */
		      0, 0, 0,                                          /* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT30, NOT_ENABLE,         /* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG2_OFFSET, BIT21, DO_RESET,        /* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG1_OFFSET, BIT8, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("620420000.vpel", vpel_clk_parent_ep0, 400000000, 0,       /* mclk default is pll2 with 400M */
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT26, WID2,         /* 0x14, clock select */
		      0, 0, 0,                                          /* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT31, NOT_ENABLE,         /* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG2_OFFSET, BIT22, DO_RESET,        /* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG1_OFFSET, BIT9, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("620610000.dei", dei_clk_parent_ep0, 400000000, 0,       /* mclk default is pll2 with 400M */
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT28, WID2,         /* 0x14, clock select */
		      0, 0, 0,                                          /* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT28, NOT_ENABLE,         /* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG2_OFFSET, BIT19, DO_RESET,        /* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG1_OFFSET, BIT6, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("620620000.dei2", dei2_clk_parent_ep0, 400000000, 0,       /* mclk default is pll2 with 400M */
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT30, WID2,         /* 0x14, clock select */
		      0, 0, 0,                                          /* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT29, NOT_ENABLE,         /* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG2_OFFSET, BIT20, DO_RESET,        /* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG1_OFFSET, BIT7, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("2f0920000_ep0.ai", nue_clk_parent_ep0, 600000000, 0,	/* mclk default is pll8 with 600M*/
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT12, WID1,						/* 0x0, clock select */
		      0, 0, 0,														/* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT15, NOT_ENABLE,						/* 0x74, Module Clock Enable Register 0 */
		      CG_SYS_RESET_REG2_OFFSET, BIT6, DO_RESET,						/* 0x98, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT30, DO_AUTOGATING,			/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("2f0930000_ep0.ai", nue2_clk_parent_ep0, 600000000, 0,	/* mclk default is pll8 with 600M*/
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT14, WID1,						/* 0x0, clock select */
		      0, 0, 0,														/* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT16, NOT_ENABLE,						/* 0x74, Module Clock Enable Register 0 */
		      CG_SYS_RESET_REG2_OFFSET, BIT7, DO_RESET,                   	/* 0x98, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT31, DO_AUTOGATING,			/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("2f0400000_ep0.ai", cnn_clk_parent_ep0, 600000000, 0,	/* mclk default is pll8 with 600M*/
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT6, WID1,						/* 0x0, clock select */
		      0, 0, 0,														/* div_reg */
		      CG_CLK_EN_REG0_OFFSET, BIT16, NOT_ENABLE,						/* 0x70, Module Clock Enable Register 0 */
		      CG_SYS_RESET_REG1_OFFSET, BIT4, DO_RESET,						/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT0, DO_AUTOGATING,			/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("2f0900000_ep0.ai", cnn2_clk_parent_ep0, 600000000, 0,	/* mclk default is pll8 with 600M*/
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT8, WID1,						/* 0x0, clock select */
		      0, 0, 0,														/* div_reg */
		      CG_CLK_EN_REG0_OFFSET, BIT17, NOT_ENABLE,						/* 0x70, Module Clock Enable Register 0 */
		      CG_SYS_RESET_REG1_OFFSET, BIT5, DO_RESET,                   	/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT1, DO_AUTOGATING,			/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("2f0910000_ep0.ai", cnn3_clk_parent_ep0, 600000000, 0,	/* mclk default is pll8 with 600M*/
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT10, WID1,						/* 0x0, clock select */
		      0, 0, 0,														/* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT18, NOT_ENABLE,						/* 0x70, Module Clock Enable Register 0 */
		      CG_SYS_RESET_REG1_OFFSET, BIT6, DO_RESET,                   	/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT2, DO_AUTOGATING,			/* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),
	COMP_MUX_CONF("2f0940000_ep0.ive", ive_clk_parent_ep0, 480000000, 0,		/* mclk default is fix480m with 480M*/
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT0, WID2,					/* 0x0, clock select */
		      0, 0, 0,													/* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT18, NOT_ENABLE,					/* 0x70, Module Clock Enable Register 0 */
		      CG_SYS_RESET_REG2_OFFSET, BIT9, DO_RESET,                   /* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG1_OFFSET, BIT1, DO_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
		      CG_RC_BASE),
	COMP_MUX_CONF("6203a0000.age", age_clk_parent_ep0, 300000000, 0,
		      0, 0, 0,         									/* 0x14, clock select */
		      0, 0, 0,									        /* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT6, DO_ENABLE, 		   /* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG1_OFFSET, BIT31, DO_RESET, 		/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT23, DO_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("620310000.ssca", ssca_clk_parent_ep0, 300000000, 0,       /* mclk default is pll4 with 300M */
		      CG_IPP_CLK_RATE_REG0_OFFSET, BIT4, WID2,         /* 0x14, clock select */
		      0, 0, 0,									        /* div_reg */
		      CG_CLK_EN_REG1_OFFSET, BIT5, DO_ENABLE, 		   /* 0x74, Module Clock Enable Register 1 */
		      CG_SYS_RESET_REG1_OFFSET, BIT30, DO_RESET, 		/* 0x94, System Reset Register1 */
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT22, DO_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
		      CG_EP0_BASE),

	COMP_MUX_CONF("ext0_clk_ep0", ext_clk_parent_ep0, 27000000, 0,
		      CG_PERI_CLK_RATE_REG1_OFFSET, BIT0, WID2,
		      CG_PERI_CLK_DIV_REG3_OFFSET, BIT0, WID5,
		      CG_CLK_EN_REG4_OFFSET, BIT12, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0,
		      CG_EP0_BASE),
	COMP_MUX_CONF("ext1_clk_ep0", ext_clk_parent_ep0, 27000000, 0,
		      CG_PERI_CLK_RATE_REG1_OFFSET, BIT2, WID2,
		      CG_PERI_CLK_DIV_REG3_OFFSET, BIT8, WID5,
		      CG_CLK_EN_REG4_OFFSET, BIT13, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0,
		      CG_EP0_BASE),
	COMP_MUX_CONF("ext2_clk_ep0", ext_clk_parent_ep0, 27000000, 0,
		      CG_PERI_CLK_RATE_REG1_OFFSET, BIT4, WID2,
		      CG_PERI_CLK_DIV_REG3_OFFSET, BIT16, WID5,
		      CG_CLK_EN_REG4_OFFSET, BIT14, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0,
		      CG_EP0_BASE),
	COMP_MUX_CONF("ext3_clk_ep0", ext_clk_parent_ep0, 27000000, 0,
		      CG_PERI_CLK_RATE_REG1_OFFSET, BIT6, WID2,
		      CG_PERI_CLK_DIV_REG3_OFFSET, BIT24, WID5,
		      CG_CLK_EN_REG4_OFFSET, BIT15, NOT_ENABLE,
		      0, 0, 0,
		      0, 0, 0,
		      CG_EP0_BASE),

	COMP_MUX_CONF("ext_from_ep0", extclk_form_parent, 0, 0,
		      CG_EXT_CLK_OUT_SRC_REG_OFFSET, BIT0, WID2,
		      0, 0, 0,
		      0, 0, 0, 0, 0,
		      0, 0, 0, 0, CG_EP0_BASE),

	COMP_MUX_CONF("ext2_from_ep0", extclk2_form_parent, 0, 0,
		      CG_EXT_CLK_OUT_SRC_REG_OFFSET, BIT2, WID2,
		      0, 0, 0,
		      0, 0, 0, 0, 0,
		      0, 0, 0, 0, CG_EP0_BASE),

	COMP_MUX_CONF("ext3_from_ep0", extclk3_form_parent, 0, 0,
		      CG_EXT_CLK_OUT_SRC_REG_OFFSET, BIT4, WID2,
		      0, 0, 0,
		      0, 0, 0, 0, 0,
		      0, 0, 0, 0, CG_EP0_BASE),

	COMP_MUX_CONF("ext4_from_ep0", extclk4_form_parent, 0, 0,
		      CG_EXT_CLK_OUT_SRC_REG_OFFSET, BIT6, WID2,
		      0, 0, 0,
		      0, 0, 0, 0, 0,
		      0, 0, 0, 0, CG_EP0_BASE),

	COMP_MUX_CONF("620b80000.sata", sata_aclk_parent_ep0, 300000000, 0,
		      0, 0, 0,
		      0, 0, 0,
		      CG_CLK_EN_REG1_OFFSET, BIT19, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT10, DO_RESET,
		      0, 0, 0,
		      CG_EP0_BASE),

	COMP_MUX_CONF("620b90000.sata", sata_aclk_parent_ep0, 300000000, 0,
		      0, 0, 0,
		      0, 0, 0,
		      CG_CLK_EN_REG1_OFFSET, BIT20, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT11, DO_RESET,
		      0, 0, 0,
		      CG_EP0_BASE),

	COMP_MUX_CONF("620880000.hdmi", root_clk_parent_ep0, DEFAULT_PLL9_RATIO, 0,
		      0, 0, 0,
		      0, 0, 0,
		      CG_CLK_EN_REG1_OFFSET, BIT14, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT5, DO_RESET,
		      0, 0, 0,
		      CG_EP0_BASE),

	COMP_MUX_CONF("620840000.tve", root_clk_parent_ep0, DEFAULT_PLL11_RATIO, 0,
		      0, 0, 0,
		      0, 0, 0,
		      CG_CLK_EN_REG1_OFFSET, BIT13, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT4, DO_RESET,
		      0, 0, 0,
		      CG_EP0_BASE),

	COMP_MUX_CONF("620800000.lcd3", lcd310_clk_parent_ep0, 297000000, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT12, WID2,
		      CG_PERI_CLK_DIV_REG1_OFFSET, BIT8, WID6,
		      CG_CLK_EN_REG0_OFFSET, BIT24, DO_ENABLE,
		      CG_SYS_RESET_REG1_OFFSET, BIT12, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT10, NOT_AUTOGATING,
		      CG_EP0_BASE),

	COMP_MUX_CONF("620810000.lcd3", lcd310_clk_parent_ep0, 297000000, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT16, WID2,
		      CG_PERI_CLK_DIV_REG1_OFFSET, BIT16, WID6,
		      CG_CLK_EN_REG0_OFFSET, BIT25, DO_ENABLE,
		      CG_SYS_RESET_REG1_OFFSET, BIT13, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT11, NOT_AUTOGATING,
		      CG_EP0_BASE),

	COMP_MUX_CONF("620820000.lcd2", lcd210_clk_parent_ep0, 27000000, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT8, WID2,
		      CG_PERI_CLK_DIV_REG1_OFFSET, BIT0, WID6,
		      CG_CLK_EN_REG0_OFFSET, BIT26, DO_ENABLE,
		      CG_SYS_RESET_REG1_OFFSET, BIT14, DO_RESET,
		      CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT12, NOT_AUTOGATING,
		      CG_EP0_BASE),

	COMP_MUX_CONF("620810000.lc3m", lcd310lite_mclk_parent_ep0, 400000000, 1,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT21, WID2,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0,
		      CG_EP0_BASE),

	COMP_MUX_CONF("620800000.lc3m", lcd310_mclk_parent_ep0, 400000000, 1,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT20, WID1,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0,
		      0, 0, 0,
		      CG_EP0_BASE),
	COMP_MUX_CONF("620850000.dai", audio_clk_parent_ep0, 12288000, 0,
		      0, 0, 0,
		      CG_DAI_CLK_DIV_REG0_OFFSET, BIT0, WID8,
		      CG_CLK_EN_REG3_OFFSET, BIT16, 0, 0, 0,
		      0, 0, 0, 0, CG_EP0_BASE),

	COMP_MUX_CONF("620860000.dai2", audio_clk_parent_ep0, 12288000, 0,
		      0, 0, 0,
		      CG_DAI_CLK_DIV_REG0_OFFSET, BIT8, WID8,
		      CG_CLK_EN_REG3_OFFSET, BIT17, 0, 0, 0,
		      0, 0, 0, 0, CG_EP0_BASE),

	COMP_MUX_CONF("620220000.dai3", audio_clk_parent_ep0, 12288000, 0,
		      0, 0, 0,
		      CG_DAI_CLK_DIV_REG0_OFFSET, BIT16, WID8,
		      CG_CLK_EN_REG3_OFFSET, BIT18, 0, 0, 0,
		      0, 0, 0, 0, CG_EP0_BASE),

	COMP_MUX_CONF("620230000.dai4", audio_clk_parent_ep0, 12288000, 0,
		      0, 0, 0,
		      CG_DAI_CLK_DIV_REG0_OFFSET, BIT24, WID8,
		      CG_CLK_EN_REG3_OFFSET, BIT19, 0, 0, 0,
		      0, 0, 0, 0, CG_EP0_BASE),

	COMP_MUX_CONF("620870000.dai5", audio_clk_parent_ep0, 12288000, 0,
		      0, 0, 0,
		      CG_DAI_CLK_DIV_REG1_OFFSET, BIT0, WID8,
		      CG_CLK_EN_REG3_OFFSET, BIT20, 0, 0, 0,
		      0, 0, 0, 0, CG_EP0_BASE),

    COMP_MUX_CONF("dai_mclk_ep0", audio_clk_parent_ep0, 12288000, 0,
              0, 0, 0,
			  CG_DAI_MCLK_DIV_REG_OFFSET, BIT0, WID8,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0),

    COMP_MUX_CONF("dai2_mclk_ep0", audio_clk_parent_ep0, 12288000, 0,
              0, 0, 0,
			  CG_DAI_MCLK_DIV_REG_OFFSET, BIT8, WID8,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0),

    COMP_MUX_CONF("dai3_mclk_ep0", audio_clk_parent_ep0, 12288000, 0,
              0, 0, 0,
			  CG_DAI_MCLK_DIV_REG_OFFSET, BIT16, WID8,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0),

    COMP_MUX_CONF("dai4_mclk_ep0", audio_clk_parent_ep0, 12288000, 0,
              0, 0, 0,
			  CG_DAI_MCLK_DIV_REG_OFFSET, BIT24, WID8,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0),

	COMP_MUX_CONF("620780000.csi", csi_clk_parent_ep0, 60000000, 0,        /* MCLK default is fix60m with 60Mhz  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT0, WID1,                    /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG3_OFFSET, BIT24, NOT_ENABLE,                    /* 0x7C, gate_reg                     */
		      CG_SYS_RESET_REG4_OFFSET, BIT0, NOT_RESET,                   /* 0xA0, reset_reg                    */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT8, DO_AUTOGATING,     /* 0xD8, autogating_reg               */
		      CG_EP0_BASE),
	COMP_MUX_CONF("620790000.csi", csi_clk_parent_ep0, 60000000, 0,        /* MCLK default is fix60m with 60Mhz  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT1, WID1,                    /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG3_OFFSET, BIT25, NOT_ENABLE,                    /* 0x7C, gate_reg                     */
		      CG_SYS_RESET_REG4_OFFSET, BIT1, NOT_RESET,                   /* 0xA0, reset_reg                    */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT9, DO_AUTOGATING,     /* 0xD8, autogating_reg               */
		      CG_EP0_BASE),
	COMP_MUX_CONF("6207a0000.csi", csi_clk_parent_ep0, 60000000, 0,        /* MCLK default is fix60m with 60Mhz  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT2, WID1,                    /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG3_OFFSET, BIT26, NOT_ENABLE,                    /* 0x7C, gate_reg                     */
		      CG_SYS_RESET_REG4_OFFSET, BIT2, NOT_RESET,                   /* 0xA0, reset_reg                    */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT10, NOT_AUTOGATING,   /* 0xD8, autogating_reg               */
		      CG_EP0_BASE),
	COMP_MUX_CONF("6207b0000.csi", csi_clk_parent_ep0, 60000000, 0,        /* MCLK default is fix60m with 60Mhz  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT3, WID1,                    /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG3_OFFSET, BIT27, NOT_ENABLE,                    /* 0x7C, gate_reg                     */
		      CG_SYS_RESET_REG4_OFFSET, BIT3, NOT_RESET,                   /* 0xA0, reset_reg                    */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT11, DO_AUTOGATING,    /* 0xD8, autogating_reg               */
		      CG_EP0_BASE),
	COMP_MUX_CONF("6207c0000.csi", csi_clk_parent_ep0, 60000000, 0,        /* MCLK default is fix60m with 60Mhz  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT4, WID1,                    /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG3_OFFSET, BIT28, NOT_ENABLE,                    /* 0x7C, gate_reg                     */
		      CG_SYS_RESET_REG4_OFFSET, BIT4, NOT_RESET,                   /* 0xA0, reset_reg                    */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT12, NOT_AUTOGATING,   /* 0xD8, autogating_reg               */
		      CG_EP0_BASE),
	COMP_MUX_CONF("6207d0000.csi", csi_clk_parent_ep0, 60000000, 0,        /* MCLK default is fix60m with 60Mhz  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT5, WID1,                    /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG3_OFFSET, BIT29, NOT_ENABLE,                    /* 0x7C, gate_reg                     */
		      CG_SYS_RESET_REG4_OFFSET, BIT5, NOT_RESET,                   /* 0xA0, reset_reg                    */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT13, DO_AUTOGATING,    /* 0xD8, autogating_reg               */
		      CG_EP0_BASE),
	COMP_MUX_CONF("6207e0000.csi", csi_clk_parent_ep0, 60000000, 0,        /* MCLK default is fix60m with 60Mhz  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT6, WID1,                    /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG3_OFFSET, BIT30, NOT_ENABLE,                    /* 0x7C, gate_reg                     */
		      CG_SYS_RESET_REG4_OFFSET, BIT6, NOT_RESET,                   /* 0xA0, reset_reg                    */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT14, NOT_AUTOGATING,   /* 0xD8, autogating_reg               */
		      CG_EP0_BASE),
	COMP_MUX_CONF("6207f0000.csi", csi_clk_parent_ep0, 60000000, 0,        /* MCLK default is fix60m with 60Mhz  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT7, WID1,                    /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG3_OFFSET, BIT31, NOT_ENABLE,                    /* 0x7C, gate_reg                     */
		      CG_SYS_RESET_REG4_OFFSET, BIT7, NOT_RESET,                   /* 0xA0, reset_reg                    */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT15, DO_AUTOGATING,    /* 0xD8, autogating_reg               */
		      CG_EP0_BASE),
	COMP_MUX_CONF("csi_pxclk_ep0", csi_pxclk_parent_ep0, 300000000, 0,     /* MCLK default is is pll4 with 300M  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT16, WID2,                   /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG4_OFFSET, BIT16, NOT_ENABLE,                    /* 0x80, gate_reg                     */
		      0, 0, 0,                                                     /* reset_reg                          */
		      0, 0, 0,                                                     /* autogating_reg                     */
		      CG_EP0_BASE),
	COMP_MUX_CONF("csi2_pxclk_ep0", csi_pxclk_parent_ep0, 300000000, 0,    /* MCLK default is is pll4 with 300M  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT18, WID2,                   /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG4_OFFSET, BIT17, NOT_ENABLE,                    /* 0x80, gate_reg                     */
		      0, 0, 0,                                                     /* reset_reg                          */
		      0, 0, 0,                                                     /* autogating_reg                     */
		      CG_EP0_BASE),
	COMP_MUX_CONF("csi3_pxclk_ep0", csi_pxclk_parent_ep0, 300000000, 0,    /* MCLK default is is pll4 with 300M  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT20, WID2,                   /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG4_OFFSET, BIT18, NOT_ENABLE,                    /* 0x80, gate_reg                     */
		      0, 0, 0,                                                     /* reset_reg                          */
		      0, 0, 0,                                                     /* autogating_reg                     */
		      CG_EP0_BASE),
	COMP_MUX_CONF("csi4_pxclk_ep0", csi_pxclk_parent_ep0, 300000000, 0,    /* MCLK default is is pll4 with 300M  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT22, WID2,                   /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG4_OFFSET, BIT19, NOT_ENABLE,                    /* 0x80, gate_reg                     */
		      0, 0, 0,                                                     /* reset_reg                          */
		      0, 0, 0,                                                     /* autogating_reg                     */
		      CG_EP0_BASE),
	COMP_MUX_CONF("csi5_pxclk_ep0", csi_pxclk_parent_ep0, 300000000, 0,    /* MCLK default is is pll4 with 300M  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT24, WID2,                   /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG4_OFFSET, BIT20, NOT_ENABLE,                    /* 0x80, gate_reg                     */
		      0, 0, 0,                                                     /* reset_reg                          */
		      0, 0, 0,                                                     /* autogating_reg                     */
		      CG_EP0_BASE),
	COMP_MUX_CONF("csi6_pxclk_ep0", csi_pxclk_parent_ep0, 300000000, 0,    /* MCLK default is is pll4 with 300M  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT26, WID2,                   /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG4_OFFSET, BIT21, NOT_ENABLE,                    /* 0x80, gate_reg                     */
		      0, 0, 0,                                                     /* reset_reg                          */
		      0, 0, 0,                                                     /* autogating_reg                     */
		      CG_EP0_BASE),
	COMP_MUX_CONF("csi7_pxclk_ep0", csi_pxclk_parent_ep0, 300000000, 0,    /* MCLK default is is pll4 with 300M  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT28, WID2,                   /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG4_OFFSET, BIT22, NOT_ENABLE,                    /* 0x80, gate_reg                     */
		      0, 0, 0,                                                     /* reset_reg                          */
		      0, 0, 0,                                                     /* autogating_reg                     */
		      CG_EP0_BASE),
	COMP_MUX_CONF("csi8_pxclk_ep0", csi_pxclk_parent_ep0, 300000000, 0,    /* MCLK default is is pll4 with 300M  */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT30, WID2,                   /* 0x28, mux_reg                      */
		      0, 0, 0,                                                     /* drv_reg                            */
		      CG_CLK_EN_REG4_OFFSET, BIT23, NOT_ENABLE,                    /* 0x80, gate_reg                     */
		      0, 0, 0,                                                     /* reset_reg                          */
		      0, 0, 0,                                                     /* autogating_reg                     */
		      CG_EP0_BASE),
};

static struct nvt_clk_gating_init novatek_init_gating_clk_ep[] = {
};

static int clk_nvt_ep_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct resource res;
	int ret;

	spin_lock_init(&cg_lock);

	if (of_address_to_resource(node, 0, &res)) {
		pr_err("%s: Failed to get resource of cg_ep\n", __func__);
		return -ENXIO;
	} else {
		remap_ep_base = ioremap(res.start, resource_size(&res));
		if (IS_ERR((void *)remap_ep_base))
			pr_err("%s: Failed to remap cg_ep register\n", __func__);
		else
			nvt_cg_ep_base_remap(remap_ep_base, 0);
	}

	pr_info("%s: cg_ep(0x%llx)\n", __func__, res.start);

	nvt_init_clk_auto_gating(novatek_init_gating_clk_ep, ARRAY_SIZE(novatek_init_gating_clk_ep), &cg_lock);

	ret = nvt_pll_clk_register(novatek_pll_ep, ARRAY_SIZE(novatek_pll_ep), DEFAULT_PLL_DIV_VALUE, &cg_lock);
	if (ret < 0) {
		pr_err("%s: Failed to register ep pll clk!\n", __func__);
	}

	ret = nvt_composite_gate_clk_register(novatek_cgate_clk_ep, ARRAY_SIZE(novatek_cgate_clk_ep), &cg_lock);
	if (ret < 0) {
		pr_err("%s: Failed to register ep composite gate clk!\n", __func__);
	}

	ret = nvt_composite_mux_clk_register(novatek_cmux_clk_ep, ARRAY_SIZE(novatek_cmux_clk_ep), &cg_lock);
	if (ret < 0) {
		pr_err("%s: Failed to register ep composite mux clk!\n", __func__);
	}

	return 0;
}

static const struct of_device_id of_match_clk_nvt_ep[] = {
	{
		.compatible = "nvt,ep_clk",
	}, {
		/* sentinel */
	}
};

static struct platform_driver clk_nvt_ep_drv = {
	.probe = clk_nvt_ep_probe,
	.driver = {
		.name = "clk-nvt-ep",
		.owner = THIS_MODULE,
		.of_match_table = of_match_clk_nvt_ep,
	},
};

static int __init clk_nvt_ep_init(void)
{
	return platform_driver_register(&clk_nvt_ep_drv);
}

arch_initcall(clk_nvt_ep_init);

static void __exit clk_nvt_ep_exit(void)
{
	platform_driver_unregister(&clk_nvt_ep_drv);
}

module_exit(clk_nvt_ep_exit);

MODULE_AUTHOR("Novatek");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CLK EP driver for NOVATEK CG controller");
MODULE_ALIAS("nvt_clk_ep");
MODULE_VERSION("1.00.001");
