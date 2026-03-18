/**
    NVT clock management module for NA51090 SoC
    @file na51090-clk.c

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

#define CPU_CLK_SEL_PLL16 1
#define CPU_CLK_SEL_480M 2

static void __iomem *remap_base = NULL;
static spinlock_t cg_lock;
#if 0
static int fastboot_determination(void)
{
	u32 m_fastboot = 0x0;
	struct device_node* of_node = of_find_node_by_path("/fastboot");

	if (of_node) {
		of_property_read_u32(of_node, "enable", &m_fastboot);
	}

	return m_fastboot;
}
#endif

/**
 * Get frequency of peripheral clock
 */
unsigned long get_periph_clk(void)
{
	int cpu_clk_sel;
	u32 cpu_clk;

	cpu_clk_sel = readl(remap_base + CG_SYS_CLK_RATE_REG_OFFSET) & 0x3;

	switch (cpu_clk_sel) {
	case CPU_CLK_SEL_PLL16: {
#ifdef CONFIG_NVT_FPGA_EMULATION
		cpu_clk = 24000000;
#else
		struct clk *pll16_clk = clk_get(NULL, "cpu_clk");

		cpu_clk = clk_get_rate(pll16_clk);

		cpu_clk *= 8;
#endif
		break;
	}
	case CPU_CLK_SEL_480M:
#ifdef CONFIG_NVT_FPGA_EMULATION
		cpu_clk = 48000000;
#else
		cpu_clk = 480000000;
#endif
		break;
	default:
#ifdef CONFIG_NVT_FPGA_EMULATION
		cpu_clk = 24000000;
#else
		cpu_clk = 80000000;
#endif
		break;
	}

	return (cpu_clk/8);
}

static void __init nvt_periph_clk_init(struct device_node *node)
{
	struct clk *periph_clk;

	periph_clk = clk_register_fixed_rate(NULL,
			"periph_clk",
			NULL,
			0,
			get_periph_clk());
	clk_register_clkdev(periph_clk, NULL, "periph_clk_dev");
	of_clk_add_provider(node, of_clk_src_simple_get, periph_clk);
}

CLK_OF_DECLARE(nvt_periph_clk, "nvt,periph_clk", nvt_periph_clk_init);

static struct nvt_fixed_rate_clk novatek_fixed_rate_clk[] __initdata = {
	FIXED_RATE_CONF("osc_in", 12000000),
    FIXED_RATE_CONF("osc_rtc", 32768),
#ifdef CONFIG_NVT_FPGA_EMULATION
	FIXED_RATE_CONF("fix480m", 48000000),
	FIXED_RATE_CONF("fix320m", 24000000),
	FIXED_RATE_CONF("fix192m", 12000000),
#else
	FIXED_RATE_CONF("fix480m", 480000000),
	FIXED_RATE_CONF("fix320m", 320000000),
	FIXED_RATE_CONF("fix192m", 192000000),
#endif
	FIXED_RATE_CONF("fix240m", 240000000),
	FIXED_RATE_CONF("fix160m", 160000000),
	FIXED_RATE_CONF("fix150m", 150000000),
	FIXED_RATE_CONF("fix120m", 120000000),
	FIXED_RATE_CONF("fix60m", 60000000),
	FIXED_RATE_CONF("fix48m", 48000000),
	FIXED_RATE_CONF("fix3m", 3000000),
	FIXED_RATE_CONF("PAUTOGATING", 0),
	FIXED_RATE_CONF("fix32.768k", 32768),
	FIXED_RATE_CONF("fix32k", 32000),
	FIXED_RATE_CONF("cnn_sram_cpu", 0),
	FIXED_RATE_CONF("cnn_sram_cnn", 0),
	FIXED_RATE_CONF("ext", 0),
	FIXED_RATE_CONF("ext2", 0),
	FIXED_RATE_CONF("ext3", 0),
	FIXED_RATE_CONF("ext4", 0),
};

static struct nvt_pll_clk novatek_pll[] __initdata = {
	PLL_CONF("pll0", DEFAULT_PLL0_RATIO, CG_PLL0_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT0,
		 CG_RC_BASE),
	PLL_CONF("pll2", DEFAULT_PLL0_RATIO, CG_PLL2_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT2,
		 CG_RC_BASE),
	PLL_CONF("pll3", DEFAULT_PLL3_RATIO, CG_PLL3_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT3,
		 CG_RC_BASE),
	PLL_CONF("pll4", DEFAULT_PLL4_RATIO, CG_PLL4_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT4,
		 CG_RC_BASE),
	PLL_CONF("pll5", DEFAULT_PLL5_RATIO, CG_PLL5_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT5,
		 CG_RC_BASE),
	PLL_CONF("pll6", DEFAULT_PLL6_RATIO, CG_PLL6_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT6,
		 CG_RC_BASE),
	PLL_CONF("pll7", DEFAULT_PLL7_RATIO, CG_PLL7_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT7,
		 CG_RC_BASE),
	PLL_CONF("pll8", DEFAULT_PLL8_RATIO, CG_PLL8_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT8,
		 CG_RC_BASE),
	PLL_CONF("pll9", DEFAULT_PLL9_RATIO, CG_PLL9_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT9,
		 CG_RC_BASE),
	PLL_CONF("pll10", DEFAULT_PLL10_RATIO, CG_PLL10_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT10,
		 CG_RC_BASE),
	PLL_CONF("pll11", DEFAULT_PLL11_RATIO, CG_PLL11_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT11,
		 CG_RC_BASE),
	PLL_CONF("pll12", DEFAULT_PLL12_RATIO, CG_PLL12_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT12,
		 CG_RC_BASE),
	PLL_CONF("pll13", DEFAULT_PLL13_RATIO, CG_PLL13_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT13,
		 CG_RC_BASE),
	PLL_CONF("pll14", DEFAULT_PLL14_RATIO, CG_PLL14_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT14,
		 CG_RC_BASE),
	PLL_CONF("pll15", DEFAULT_PLL15_RATIO, CG_PLL15_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT15,
		 CG_RC_BASE),
	PLL_CONF("cpu_clk", DEFAULT_PLL16_RATIO, CG_PLL16_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT16,
		 CG_RC_BASE),
	PLL_CONF("pll17", DEFAULT_PLL17_RATIO, CG_PLL17_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT17,
		 CG_RC_BASE),
	PLL_CONF("pll18", DEFAULT_PLL18_RATIO, CG_PLL18_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT18,
		 CG_RC_BASE),
	PLL_CONF("pll19", DEFAULT_PLL19_RATIO, CG_PLL19_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT19,
		 CG_RC_BASE),
	PLL_CONF("pll20", DEFAULT_PLL20_RATIO, CG_PLL20_CLK_RATE0_REG_OFFSET,
		 CG_PLL_EN_REG_OFFSET, BIT20,
		 CG_RC_BASE),
};

static struct nvt_composite_gate_clk novatek_cgate_clk[] __initdata = {
#ifdef CONFIG_NVT_FPGA_EMULATION
#ifdef CONFIG_MTD_SPINAND
	COMP_GATE_CONF("2f0180000.nand", "fix480m", 6000000,
		       CG_PERI_CLK_DIV_REG0_OFFSET, BIT16, WID6,
		       CG_CLK_EN_REG1_OFFSET, BIT10, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT1, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT25, NOT_AUTOGATING,
		       CG_RC_BASE),
#else
	COMP_GATE_CONF("2f0180000.nor", "fix480m", 6000000,
		       CG_PERI_CLK_DIV_REG0_OFFSET, BIT16, WID6,
		       CG_CLK_EN_REG1_OFFSET, BIT10, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT1, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT25, NOT_AUTOGATING,
		       CG_RC_BASE),
#endif
#else
#if defined(CONFIG_MTD_SPINAND) && defined(CONFIG_MTD_SPINOR)
	COMP_GATE_CONF("2f0180000.nand", "fix480m", 48000000,
		       CG_PERI_CLK_DIV_REG0_OFFSET, BIT16, WID6,
		       CG_CLK_EN_REG1_OFFSET, BIT10, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT1, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT25, NOT_AUTOGATING,
		       CG_RC_BASE),

	COMP_GATE_CONF("2f0180000.nor", "fix480m", 48000000,
		       CG_PERI_CLK_DIV_REG0_OFFSET, BIT16, WID6,
		       CG_CLK_EN_REG1_OFFSET, BIT10, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT1, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT25, NOT_AUTOGATING,
		       CG_RC_BASE),
#else
#ifdef CONFIG_MTD_SPINAND
	COMP_GATE_CONF("2f0180000.nand", "fix480m", 48000000,
		       CG_PERI_CLK_DIV_REG0_OFFSET, BIT16, WID6,
		       CG_CLK_EN_REG1_OFFSET, BIT10, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT1, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT25, NOT_AUTOGATING,
		       CG_RC_BASE),
#else
	COMP_GATE_CONF("2f0180000.nor", "fix480m", 48000000,
		       CG_PERI_CLK_DIV_REG0_OFFSET, BIT16, WID6,
		       CG_CLK_EN_REG1_OFFSET, BIT10, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT1, NOT_RESET,
		       CG_CLK_AUTO_GATING_REG0_OFFSET, BIT25, NOT_AUTOGATING,
		       CG_RC_BASE),
#endif
#endif
#endif
	COMP_GATE_CONF("2f0280000.uart", "fix480m", 48000000,
			   CG_UART_CLK_DIV_REG_OFFSET, BIT0, WID8,
		       CG_CLK_EN_REG2_OFFSET, BIT20, DO_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT0, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT20, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0290000.uart", "fix480m", 48000000,
			   CG_UART_CLK_DIV_REG_OFFSET, BIT8, WID8,
		       CG_CLK_EN_REG2_OFFSET, BIT21, DO_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT1, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT21, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f02a0000.uart", "fix480m", 48000000,
			   CG_UART_CLK_DIV_REG_OFFSET, BIT16, WID8,
		       CG_CLK_EN_REG2_OFFSET, BIT22, DO_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT2, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT22, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f02b0000.uart", "fix480m", 48000000,
			   CG_UART_CLK_DIV_REG_OFFSET, BIT24, WID8,
		       CG_CLK_EN_REG2_OFFSET, BIT23, DO_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT3, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT23, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f02c0000.uart", "fix480m", 48000000,
			   CG_PERI_CLK_DIV_REG0_OFFSET, BIT0, WID8,
		       CG_CLK_EN_REG2_OFFSET, BIT24, DO_ENABLE,
		       CG_SYS_RESET_REG3_OFFSET, BIT4, NOT_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT24, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0380000.usbh", "pll20", 48000000, 0, 0, 0,
		       0, 0, 0,
		       CG_SYS_RESET_REG2_OFFSET, BIT15, DO_RESET,
		       0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0380000.usbd", "pll20", 48000000, 0, 0, 0,
		       0, 0, 0,
		       CG_SYS_RESET_REG2_OFFSET, BIT15, DO_RESET,
		       0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0390000.usbh", "fix48m", 48000000, 0, 0, 0,
		       0, 0, 0,
		       CG_SYS_RESET_REG2_OFFSET, BIT16, DO_RESET,
		       0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0390000.usbd", "fix48m", 48000000, 0, 0, 0,
		       0, 0, 0,
		       CG_SYS_RESET_REG2_OFFSET, BIT16, DO_RESET,
		       0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0580000.u3hst", "pll20", 150000000, 0, 0, 0,
		       CG_CLK_EN_REG1_OFFSET, BIT26, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT18, DO_RESET,
		       0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("u3phy", "pll20", 150000000, 0, 0, 0,
		       CG_SYS_RESET_REG3_OFFSET, BIT15, NOT_ENABLE,
		       0, 0, 0,
		       0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0110000.wdt", "osc_in", 12000000,
			0, 0, 0,				// clk div
			CG_CLK_EN_REG3_OFFSET, BIT4, DO_ENABLE,	// module gating
			CG_SYS_RESET_REG3_OFFSET, BIT12, NOT_RESET,// module reset
			0, 0, 0,				// auto gating
			CG_RC_BASE),

    COMP_GATE_CONF("2f0a50000.rtc", "osc_rtc", 32768,
                0, 0, 0,                                                    // clk div
                0, 0, 0,                                        // module gating
                CG_SYS_RESET_REG3_OFFSET, BIT24, NOT_RESET,// module reset
                0, 0, 0,        // auto gating
                CG_RC_BASE),

	COMP_GATE_CONF("2f0510000.thmal", "osc_in", 12000000,
			0, 0, 0,				// clk div
			CG_CLK_EN_REG2_OFFSET, BIT27, NOT_ENABLE,		// 0x78:CG_CLK_EN_REG2_OFFSET bit[27] module gating
			CG_SYS_RESET_REG3_OFFSET, BIT7, NOT_RESET,		// module reset
			0, 0, 0,				// auto gating
			CG_RC_BASE),

	COMP_GATE_CONF("2f01c0000.i2c", "fix480m", 48000000,
			   0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT16, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT28, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT16, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f01d0000.i2c2", "fix480m", 48000000,
			   0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT17, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT29, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT17, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f01e0000.i2c3", "fix480m", 48000000,
			   0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT18, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT30, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT18, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f01f0000.i2c4", "fix480m", 48000000,
			   0, 0, 0,
		       CG_CLK_EN_REG2_OFFSET, BIT19, NOT_ENABLE,
		       CG_SYS_RESET_REG2_OFFSET, BIT31, DO_RESET,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT19, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0330000.hwcp", "pll2", 400000000,
			0, 0, 0,
			CG_CLK_EN_REG1_OFFSET, BIT9, DO_ENABLE,
			CG_SYS_RESET_REG2_OFFSET, BIT0, NOT_RESET,
			CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT21, DO_AUTOGATING,
			CG_RC_BASE),
	COMP_GATE_CONF("2f0100000.timer", "fix480m", 48000000,
                        0, 0, 0,
                        CG_CLK_EN_REG2_OFFSET, BIT26, DO_ENABLE,
                        CG_SYS_RESET_REG3_OFFSET, BIT6, NOT_RESET,
                        CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT26, DO_AUTOGATING,
                        CG_RC_BASE),

    COMP_GATE_CONF("2f0200000.eth0", "pll7", 250000000,
            0, 0, 0,
            CG_CLK_EN_REG0_OFFSET, BIT27, DO_ENABLE,
            CG_SYS_RESET_REG1_OFFSET, BIT16, NOT_RESET,
            CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT8, DO_AUTOGATING,
            CG_RC_BASE),

	COMP_GATE_CONF("2f0210000.eth1", "pll7", 250000000,
            0, 0, 0,
            CG_CLK_EN_REG0_OFFSET, BIT28, DO_ENABLE,
            CG_SYS_RESET_REG1_OFFSET, BIT18, NOT_RESET,
            CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT9, DO_AUTOGATING,
            CG_RC_BASE),

	COMP_GATE_CONF("eth0_extphy_clk", "fix480m", 2500000,
            0, 0, 0,
            CG_CLK_EN_REG1_OFFSET, BIT7, DO_ENABLE,
            0, 0, 0,
            0, 0, 0,
            0),

	COMP_GATE_CONF("eth1_extphy_clk", "fix480m", 2500000,
            0, 0, 0,
            CG_CLK_EN_REG1_OFFSET, BIT8, DO_ENABLE,
            0, 0, 0,
            0, 0, 0,
            0),

	COMP_GATE_CONF("2f0140000.msi", "pll0", 500000000,
			0, 0, 0,				// clk div
			CG_CLK_EN_REG3_OFFSET, BIT11, DO_ENABLE,// module gating
			CG_SYS_RESET_REG1_OFFSET, BIT3, NOT_RESET,// module reset
			CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT9, NOT_AUTOGATING,// auto gating
			CG_RC_BASE),
	COMP_GATE_CONF("pll20_ss", "osc_in", 0,
		       CG_PLL20_CLK_BASE_REG_OFFSET + CG_PLL_SPREAD_SPECTRUM_REG0_OFFSET, BIT20, 0,
		       CG_PLL20_CLK_BASE_REG_OFFSET + CG_PLL_SPREAD_SPECTRUM_REG1_OFFSET, BIT20, 0,
		       CG_PLL20_CLK_BASE_REG_OFFSET + CG_PLL_SPREAD_SPECTRUM_REG2_OFFSET, BIT20, 0,
		       CG_PLL20_CLK_BASE_REG_OFFSET + CG_PLL_SPREAD_SPECTRUM_REG3_OFFSET, BIT20, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("cnn_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT2, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("cnn2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT3, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("cnn3_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT4, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("gpdec_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT5, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("gpenc_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT6, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("gpenc2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT7, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("jpeg_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT8, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("msi_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT9, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("lcd310_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT10, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("lcd310_2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT11, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("lcd210_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT12, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("venc_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT13, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("vecn2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT14, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("h264d_pclk", "fix150m", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT15, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("h265d_pclk", "fix150m", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT16, NOT_AUTOGATING,
		       CG_RC_BASE),

	COMP_GATE_CONF("osg_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT17, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("hdmi_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT18, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("ssca_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT19, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("age_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT20, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("hwcp_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT21, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("nand_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT22, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("sdio_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT23, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("sdio2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT24, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("crypto_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT25, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("hash_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT26, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("nue_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT27, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("nue2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT28, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("ive_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT30, DO_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("dei_mclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT4, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("dei2_mclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT5, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("vpe_mclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT6, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("vpel_mclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT7, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("i2c_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT16, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("i2c2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT17, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("i2c3_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT18, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("i2c4_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT19, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("uart_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT20, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("uart2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT21, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("uart3_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT22, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("uart4_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT23, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("uart5_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT24, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("remote_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT25, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("timer_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT26, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("temp_sensor_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT27, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("pwm_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT28, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0120000.pwm0", "fix120m", 30000000,
			   CG_PWM_CLK_DIV_REG0_OFFSET, BIT0, WID14,
	           CG_CLK_EN_REG3_OFFSET, BIT0, NOT_ENABLE,
	           CG_SYS_RESET_REG3_OFFSET, BIT9, NOT_RESET,
	           0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0120000.pwm1", "fix120m", 30000000,
	           CG_PWM_CLK_DIV_REG0_OFFSET, BIT16, WID14,
	           CG_CLK_EN_REG3_OFFSET, BIT1, NOT_ENABLE,
	           CG_SYS_RESET_REG3_OFFSET, BIT9, NOT_RESET,
	           0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0120000.pwm2", "fix120m", 30000000,
	           CG_PWM_CLK_DIV_REG1_OFFSET, BIT0, WID14,
	           CG_CLK_EN_REG3_OFFSET, BIT2, NOT_ENABLE,
	           CG_SYS_RESET_REG3_OFFSET, BIT9, NOT_RESET,
	           0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("2f0120000.pwm3", "fix120m", 30000000,
	           CG_PWM_CLK_DIV_REG1_OFFSET, BIT16, WID14,
	           CG_CLK_EN_REG3_OFFSET, BIT3, NOT_ENABLE,
	           CG_SYS_RESET_REG3_OFFSET, BIT9, NOT_RESET,
	           0, 0, 0,
		       CG_RC_BASE),
	COMP_GATE_CONF("pcie_refclk", "pll5", 100000000,
			0, 0, 0,
			0, 0, 0,
			0, 0, NOT_ENABLE,
			0, 0, NOT_AUTOGATING,
			CG_RC_BASE),
	COMP_GATE_CONF("pcie_clk", "reserved", 0,
			0, 0, 0,
			0, 0, 0,
			CG_CLK_EN_REG1_OFFSET, 22, NOT_ENABLE,
			0, 0, NOT_AUTOGATING,
			CG_RC_BASE),
	COMP_GATE_CONF("PHY_PORN", "reserved", 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, NOT_ENABLE,
			0, 0, NOT_AUTOGATING,
			CG_RC_BASE),
	COMP_GATE_CONF("wdt_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT29, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("gpio_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT30, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("dai_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT0, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("dai2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT1, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("dai3_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT2, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("dai4_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT3, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("dai5_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT4, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("csi_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT8, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("csi2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT9, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("csi3_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT10, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("csi4_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT11, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("csi5_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT12, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("csi6_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT13, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("csi7_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT14, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("csi8_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT15, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("senphy_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT16, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("senphy2_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT17, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("senphy3_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT18, NOT_AUTOGATING,
		       CG_RC_BASE),
	COMP_GATE_CONF("senphy4_pclk", "PAUTOGATING", 0,
		       0, 0, 0,
		       0, 0, 0,
		       0, 0, 0,
		       CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT19, NOT_AUTOGATING,
		       CG_RC_BASE),
};

static const char *sdio_clk_parent[] __initconst = {
	"fix192m", "fix480m", "pll6", "reserved"
};

static const char *apb_clk_parent[] __initconst = {
	"fix240m", "pll4"
};

static const char *csi_clk_parent[] __initconst = {
	"fix60m", "fix120m"
};

static const char *csi_pxclk_parent[] __initconst = {
	"pll4", "pll2", "pll13"
};

static const char *root_clk_parent[] __initconst = {
	"osc_in",
};

static const char *rtc_clk_parent[] __initconst = {
    "osc_rtc",
};

static const char *sata_aclk_parent[] __initconst = {
	"pll4",
};

static const char *lcd210_clk_parent[] __initconst = {
	"pll9", "pll10", "pll11",
};

static const char *lcd310_clk_parent[] __initconst = {
	"pll9", "pll10", "pll11", "pll15",
};

static const char *lcd310_mclk_parent[] __initconst = {
	"pll0", "pll2",
};

static const char *lcd310lite_mclk_parent[] __initconst = {
	"pll0", "pll2", "pll2",
};

/* osg */
static const char *osg_clk_parent[] __initconst = {
	"pll2", "pll13"
};
/* h264dec */
static const char *h264d_mclk_parent[] __initconst = {
	"pll18", "pll0"
};
static const char *h264d_aclk_parent[] __initconst = {
	"pll18", "pll13", "pll0"
};
/* h265dec */
static const char *h265d_mclk_parent[] __initconst = {
	"pll18", "pll0"
};
static const char *h265d_aclk_parent[] __initconst = {
	"pll13", "pll2"
};
static const char *vpe_clk_parent[] __initconst = {
	"pll2", "fix320m", "pll15", "reserved"
};
static const char *vpel_clk_parent[] __initconst = {
	"pll2", "fix320m", "pll15", "reserved"
};
static const char *dei_clk_parent[] __initconst = {
	"pll2", "fix320m", "pll15", "reserved"
};

static const char *dei2_clk_parent[] __initconst = {
	"pll2", "fix320m", "pll15", "reserved"

};

static const char *venc_clk_parent[] __initconst = {
	"pll14", "reserved", "pll15", "reserved"
};

static const char *age_clk_parent[] __initconst = {
	"pll4",
};

static const char *ssca_clk_parent[] __initconst = {
	"pll4", "pll2"
};

/* vcap */
static const char *vcap_aclk_m_parent[] __initconst = {
	"pll0",
};
static const char *vcap_mclk_parent[] __initconst = {
	"pll19", "pll0"
};
static const char *vcap_patclk_parent[] __initconst = {
	"pll9", "pll6", "pll11"
};

static const char *crypto_clk_parent[] __initconst = {
	"pll2", "pll13"
};

static const char *rsa_clk_parent[] __initconst = {
	"pll2", "pll13"
};

static const char *hash_clk_parent[] __initconst = {
	"pll2", "pll13"
};

static const char *jpeg_clk_parent[] __initconst = {
	"fix480m", "fix320m", "pll15", "reserved"
};
static const char *nue_clk_parent[] __initconst = {
	"pll8", "pll15"
};

static const char *nue2_clk_parent[] __initconst = {
	"pll8", "pll15"
};
static const char *cnn_clk_parent[] __initconst = {
	"pll8", "pll15"
};

static const char *cnn2_clk_parent[] __initconst = {
	"pll8", "pll15"
};


static const char *cnn3_clk_parent[] __initconst = {
	"pll8", "pll15"
};
static const char *ive_clk_parent[] __initconst = {
	"fix480m", "fix320m", "fix240m", "reserved"
};
static const char *ext_clk_parent[] __initconst = {
	"pll9", "pll7", "pll12", "osc_in"
};

static const char *extclk_form_parent[] __initconst = {
	"ext", "ext2", "ext3", "ext4"
};
static const char *extclk2_form_parent[] __initconst = {
	"ext2", "ext3", "ext4", "ext"
};
static const char *extclk3_form_parent[] __initconst = {
	"ext3", "ext4", "ext", "ext2"
};
static const char *extclk4_form_parent[] __initconst = {
	"ext4", "ext", "ext2", "ext3"
};


static const char *cnn_shared_sram_parent[] __initconst = {
	"cnn_sram_cpu", "cnn_sram_cnn"
};

static const char *remote_clk_parent[] __initconst = {
	"fix32.768k", "fix32k", "reserved", "fix3m"
};

static const char *rng_clk_parent[] __initconst = {
	"fix160m", "fix240m"
};

static const char *rngro_clk_parent[] __initconst = {
	"osc_in", "pll11"
};

static const char *audio_clk_parent[] __initconst = {
	"pll12",
};

static struct nvt_composite_mux_clk novatek_cmux_clk[] __initdata = {
	COMP_MUX_CONF("remote_clk", remote_clk_parent, 32768, 0,
		      CG_PERI_CLK_RATE_REG1_OFFSET, BIT16, WID2,
		      0, 0, 0,
		      CG_CLK_EN_REG2_OFFSET, BIT25, NOT_ENABLE,
		      CG_SYS_RESET_REG3_OFFSET, BIT5, NOT_RESET,
		      CG_APB_CLK_AUTO_GATING_REG1_OFFSET, BIT25, DO_AUTOGATING,
		      CG_RC_BASE),

	COMP_MUX_CONF("2f0190000.mmc", sdio_clk_parent, 312500, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT0, WID2,
		      CG_SDIO_CLK_DIV_REG_OFFSET, BIT0, WID11,
		      CG_CLK_EN_REG1_OFFSET, BIT11, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT2, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT26, DO_AUTOGATING,
		      CG_RC_BASE),

	COMP_MUX_CONF("2f01a0000.mmc", sdio_clk_parent, 312500, 0,
		      CG_PERI_CLK_RATE_REG0_OFFSET, BIT2, WID2,
		      CG_SDIO_CLK_DIV_REG_OFFSET, BIT16, WID11,
		      CG_CLK_EN_REG1_OFFSET, BIT12, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT3, DO_RESET,
		      CG_CLK_AUTO_GATING_REG0_OFFSET, BIT27, NOT_AUTOGATING,
		      CG_RC_BASE),

	COMP_MUX_CONF("2f0b80000.sata", sata_aclk_parent, 300000000, 0,
	          0, 0, 0,
	          0, 0, 0,
	          CG_CLK_EN_REG1_OFFSET, BIT19, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT10, DO_RESET,
		      0, 0, 0,
		      CG_RC_BASE),

	COMP_MUX_CONF("2f0b90000.sata", sata_aclk_parent, 300000000, 0,
	          0, 0, 0,
	          0, 0, 0,
	          CG_CLK_EN_REG1_OFFSET, BIT20, NOT_ENABLE,
		      CG_SYS_RESET_REG2_OFFSET, BIT11, DO_RESET,
		      0, 0, 0,
		      CG_RC_BASE),

	COMP_MUX_CONF("2f0480000.sata", sata_aclk_parent, 300000000, 0,
	          0, 0, 0,
	          0, 0, 0,
	          CG_CLK_EN_REG1_OFFSET, BIT21, NOT_ENABLE,
	          CG_SYS_RESET_REG2_OFFSET, BIT12, DO_RESET,
		      0, 0, 0,
		      CG_RC_BASE),

    COMP_MUX_CONF("2f0A50000.rtc", rtc_clk_parent, 32768,0,
              0, 0, 0,                                               // clk sel
              0, 0, 0,                                   // module gating
              0, 0, 0,                                   // clk en
              CG_SYS_RESET_REG3_OFFSET, BIT24, NOT_RESET,// module reset
              0, 0, 0,                                               // auto gating
              CG_RC_BASE),

	COMP_MUX_CONF("2f0880000.hdmi", root_clk_parent, DEFAULT_PLL9_RATIO, 0,
			  0, 0, 0,
			  0, 0, 0,
			  CG_CLK_EN_REG1_OFFSET, BIT14, NOT_ENABLE,
			  CG_SYS_RESET_REG2_OFFSET, BIT5, DO_RESET,
			  0, 0, 0,
			  CG_RC_BASE),
	COMP_MUX_CONF("2f0840000.tve", root_clk_parent, DEFAULT_PLL11_RATIO, 0,
			  0, 0, 0,
			  0, 0, 0,
			  CG_CLK_EN_REG1_OFFSET, BIT13, NOT_ENABLE,
			  CG_SYS_RESET_REG2_OFFSET, BIT4, DO_RESET,
			  0, 0, 0,
			  CG_RC_BASE),

	COMP_MUX_CONF("2f0800000.lcd3", lcd310_clk_parent, 297000000, 0,
	      CG_PERI_CLK_RATE_REG0_OFFSET, BIT12, WID2,
          CG_PERI_CLK_DIV_REG1_OFFSET, BIT8, WID6,
          CG_CLK_EN_REG0_OFFSET, BIT24, DO_ENABLE,
		  CG_SYS_RESET_REG1_OFFSET, BIT12, DO_RESET,
		  CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT10, NOT_AUTOGATING,
		  CG_RC_BASE),

	COMP_MUX_CONF("2f0810000.lcd3", lcd310_clk_parent, 297000000, 0,
	      CG_PERI_CLK_RATE_REG0_OFFSET, BIT16, WID2,
          CG_PERI_CLK_DIV_REG1_OFFSET, BIT16, WID6,
          CG_CLK_EN_REG0_OFFSET, BIT25, DO_ENABLE,
		  CG_SYS_RESET_REG1_OFFSET, BIT13, DO_RESET,
		  CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT11, NOT_AUTOGATING,
		  CG_RC_BASE),

	COMP_MUX_CONF("2f0820000.lcd2", lcd210_clk_parent, 27000000, 0,
	      CG_PERI_CLK_RATE_REG0_OFFSET, BIT8, WID2,
          CG_PERI_CLK_DIV_REG1_OFFSET, BIT0, WID6,
          CG_CLK_EN_REG0_OFFSET, BIT26, DO_ENABLE,
		  CG_SYS_RESET_REG1_OFFSET, BIT14, DO_RESET,
		  CG_APB_CLK_AUTO_GATING_REG0_OFFSET, BIT12, NOT_AUTOGATING,
		  CG_RC_BASE),

	COMP_MUX_CONF("2f0800000.lc3m", lcd310_mclk_parent, 400000000, 1,
	      CG_PERI_CLK_RATE_REG0_OFFSET, BIT20, WID1,
          0, 0, 0,
          0, 0, 0,
		  0, 0, 0,
		  0, 0, 0,
		  CG_RC_BASE),

	COMP_MUX_CONF("2f0810000.lc3m", lcd310lite_mclk_parent, 400000000, 1,
	      CG_PERI_CLK_RATE_REG0_OFFSET, BIT21, WID2,
          0, 0, 0,
          0, 0, 0,
		  0, 0, 0,
		  0, 0, 0,
		  CG_RC_BASE),

	COMP_MUX_CONF("2f0300000.osg", osg_clk_parent, 400000000, 0, 	/* mclk default is pll2 with 400M */
			  CG_CODEC_CLK_RATE_REG_OFFSET, BIT16, WID1, 		/* 0x1c, clock select */
			  0, 0, 0, 											/* div_reg */
			  CG_CLK_EN_REG1_OFFSET, BIT4, NOT_ENABLE, 			/* 0x74, Module Clock Enable Register 1 */
			  CG_SYS_RESET_REG1_OFFSET, BIT28, DO_RESET, 		/* 0x94, System Reset Register1 */
			  CG_CLK_AUTO_GATING_REG0_OFFSET, BIT20, NOT_AUTOGATING,	/* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),
	COMP_MUX_CONF("h264d_mclk", h264d_mclk_parent, 450000000, 0, 	/* mclk default is pll18 with 450M */
			  CG_CODEC_CLK_RATE_REG_OFFSET, BIT12, WID1, 	    /* 0x1c, clock select */
			  0, 0, 0,											/* div_reg */
			  CG_CLK_EN_REG1_OFFSET, BIT2, NOT_ENABLE, 			/* 0x74, Module Clock Enable Register 1 */
			  CG_SYS_RESET_REG1_OFFSET, BIT26, DO_RESET, 		/* 0x94, System Reset Register1 */
			  CG_CLK_AUTO_GATING_REG0_OFFSET, BIT18, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),
	COMP_MUX_CONF("h264d_aclk", h264d_aclk_parent, 450000000, 0, 	/* mclk default is pll18 with 450M */
			  CG_CODEC_CLK_RATE_REG_OFFSET, BIT8, WID2, 	    /* 0x1c, clock select */
			  0, 0, 0,											/* div_reg */
			  0, 0, 0, 											/* 0x74, Module Clock Enable Register 1 */
			  0, 0, 0, 											/* 0x94, System Reset Register1 */
			  0, 0, 0, 											/* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),
	COMP_MUX_CONF("h265d_mclk", h265d_mclk_parent, 450000000, 0, 	/* mclk default is pll18 with 450M */
			  CG_CODEC_CLK_RATE_REG_OFFSET, BIT14, WID1, 	    /* 0x1c, clock select */
			  0, 0, 0,											/* div_reg */
			  CG_CLK_EN_REG1_OFFSET, BIT3, NOT_ENABLE, 			/* 0x74, Module Clock Enable Register 1 */
			  0, 0, 0, 											/* 0x94, System Reset Register1 */
			  CG_CLK_AUTO_GATING_REG0_OFFSET, BIT19, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),
	COMP_MUX_CONF("h265d_aclk", h265d_aclk_parent, 350000000, 0, /* mclk default is pll12 with 350M */
			  CG_CODEC_CLK_RATE_REG_OFFSET, BIT10, WID1, 	    /* 0x1c, clock select */
			  0, 0, 0,											/* div_reg */
			  0, 0, 0, 											/* 0x74, Module Clock Enable Register 1 */
			  0, 0, 0, 											/* 0x94, System Reset Register1 */
			  0, 0, 0, 											/* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),
	COMP_MUX_CONF("2f0410000.vpe", vpe_clk_parent, 400000000, 0,       /* mclk default is pll2 with 400M */
           	  CG_IPP_CLK_RATE_REG0_OFFSET, BIT24, WID2,         /* 0x14, clock select */
			  0, 0, 0,									        /* div_reg */
			  CG_CLK_EN_REG1_OFFSET, BIT30, NOT_ENABLE, 		   /* 0x74, Module Clock Enable Register 1 */
			  CG_SYS_RESET_REG2_OFFSET, BIT21, DO_RESET, 		/* 0x94, System Reset Register1 */
			  CG_CLK_AUTO_GATING_REG1_OFFSET, BIT8, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),

	COMP_MUX_CONF("2f0420000.vpel", vpel_clk_parent, 400000000, 0,       /* mclk default is pll2 with 400M */
           	  CG_IPP_CLK_RATE_REG0_OFFSET, BIT26, WID2,         /* 0x14, clock select */
			  0, 0, 0,									        /* div_reg */
			  CG_CLK_EN_REG1_OFFSET, BIT31, NOT_ENABLE, 		   /* 0x74, Module Clock Enable Register 1 */
			  CG_SYS_RESET_REG2_OFFSET, BIT22, DO_RESET, 		/* 0x94, System Reset Register1 */
			  CG_CLK_AUTO_GATING_REG1_OFFSET, BIT9, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),

	COMP_MUX_CONF("2f0610000.dei", dei_clk_parent, 400000000, 0,       /* mclk default is pll2 with 400M */
           	  CG_IPP_CLK_RATE_REG0_OFFSET, BIT28, WID2,         /* 0x14, clock select */
			  0, 0, 0,									        /* div_reg */
			  CG_CLK_EN_REG1_OFFSET, BIT28, NOT_ENABLE, 		   /* 0x74, Module Clock Enable Register 1 */
			  CG_SYS_RESET_REG2_OFFSET, BIT19, DO_RESET, 		/* 0x94, System Reset Register1 */
			  CG_CLK_AUTO_GATING_REG1_OFFSET, BIT6, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),

	COMP_MUX_CONF("2f0620000.dei2", dei2_clk_parent, 400000000, 0,       /* mclk default is pll2 with 400M */
           	  CG_IPP_CLK_RATE_REG0_OFFSET, BIT30, WID2,         /* 0x14, clock select */
			  0, 0, 0,									        /* div_reg */
			  CG_CLK_EN_REG1_OFFSET, BIT29, NOT_ENABLE, 		   /* 0x74, Module Clock Enable Register 1 */
			  CG_SYS_RESET_REG2_OFFSET, BIT20, DO_RESET, 		/* 0x94, System Reset Register1 */
			  CG_CLK_AUTO_GATING_REG1_OFFSET, BIT7, NOT_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),

	COMP_MUX_CONF("2f0500000.venc", venc_clk_parent, 38000000, 0,		/* mclk default is pll14 with 380M */
			CG_CODEC_CLK_RATE_REG_OFFSET, BIT4, WID2,					/* 0x4, clock select */
			0, 0, 0,													/* div_reg */
			CG_CLK_EN_REG1_OFFSET, BIT0, NOT_ENABLE,					/* 0x74, Module Clock Enable Register 1 */
			CG_SYS_RESET_REG1_OFFSET, BIT24, DO_RESET,					/* 0x94, System Reset Register1 */
			CG_CLK_AUTO_GATING_REG0_OFFSET, BIT16, NOT_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
			CG_RC_BASE),
	COMP_MUX_CONF("2f0a00000.venc", venc_clk_parent, 38000000, 0,		/* mclk default is pll14 with 380M */
			CG_CODEC_CLK_RATE_REG_OFFSET, BIT4, WID2,					/* 0x4, clock select */
			0, 0, 0,													/* div_reg */
			CG_CLK_EN_REG1_OFFSET, BIT1, NOT_ENABLE,					/* 0x74, Module Clock Enable Register 1 */
			CG_SYS_RESET_REG1_OFFSET, BIT25, DO_RESET,					/* 0x94, System Reset Register1 */
			CG_CLK_AUTO_GATING_REG0_OFFSET, BIT17, NOT_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
			CG_RC_BASE),
	COMP_MUX_CONF("2f03a0000.age", age_clk_parent, 300000000, 0,
           	  0, 0, 0,         									/* 0x14, clock select */
			  0, 0, 0,									        /* div_reg */
			  CG_CLK_EN_REG1_OFFSET, BIT6, DO_ENABLE, 		   /* 0x74, Module Clock Enable Register 1 */
			  CG_SYS_RESET_REG1_OFFSET, BIT31, DO_RESET, 		/* 0x94, System Reset Register1 */
			  CG_CLK_AUTO_GATING_REG0_OFFSET, BIT23, DO_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),

	COMP_MUX_CONF("2f0310000.ssca", ssca_clk_parent, 300000000, 0,       /* mclk default is pll4 with 300M */
        CG_IPP_CLK_RATE_REG0_OFFSET, BIT4, WID2,         /* 0x14, clock select */
			  0, 0, 0,									        /* div_reg */
			  CG_CLK_EN_REG1_OFFSET, BIT5, DO_ENABLE, 		   /* 0x74, Module Clock Enable Register 1 */
			  CG_SYS_RESET_REG1_OFFSET, BIT30, DO_RESET, 		/* 0x94, System Reset Register1 */
			  CG_CLK_AUTO_GATING_REG0_OFFSET, BIT22, DO_AUTOGATING, /* 0xC0, Clock Auto Gating Register0 */
			  CG_RC_BASE),

	COMP_MUX_CONF("vcap_aclk_m0", vcap_aclk_m_parent, 500000000, 0,     /* MCLK default is PLL0 with 500MHz               */
			  0, 0, 0,                                                  /* mux_reg,                                       */
			  0, 0, 0,                                                  /* div_reg                                        */
			  CG_CLK_EN_REG2_OFFSET, BIT0, NOT_ENABLE,                  /* gate_reg  0x78, Module Clock Enable Register 2 */
			  0, 0, 0,                                                  /* reset_reg                                      */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),
	COMP_MUX_CONF("vcap_aclk_m1", vcap_aclk_m_parent, 500000000, 0,     /* MCLK default is PLL0 with 500MHz               */
			  0, 0, 0,                                                  /* mux_reg,                                       */
			  0, 0, 0,                                                  /* div_reg                                        */
			  CG_CLK_EN_REG2_OFFSET, BIT1, NOT_ENABLE,                  /* gate_reg  0x78, Module Clock Enable Register 2 */
			  0, 0, 0,                                                  /* reset_reg                                      */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),
	COMP_MUX_CONF("vcap_aclk_m2", vcap_aclk_m_parent, 500000000, 0,     /* MCLK default is PLL0 with 500MHz               */
			  0, 0, 0,                                                  /* mux_reg,                                       */
			  0, 0, 0,                                                  /* div_reg                                        */
			  CG_CLK_EN_REG2_OFFSET, BIT2, NOT_ENABLE,                  /* gate_reg  0x78, Module Clock Enable Register 2 */
			  0, 0, 0,                                                  /* reset_reg                                      */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),
	COMP_MUX_CONF("vcap_mclk", vcap_mclk_parent, 400000000, 0,          /* MCLK default is PLL19 with 400MHz              */
			  CG_IPP_CLK_RATE_REG0_OFFSET, BIT20, WID1,                 /* mux_reg   0x14, IPP Clock Rate Register        */
			  0, 0, 0,                                                  /* div_reg                                        */
			  CG_CLK_EN_REG2_OFFSET, BIT3, NOT_ENABLE,                  /* gate_reg  0x78, Module Clock Enable Register 2 */
			  CG_SYS_RESET_REG2_OFFSET, BIT24, DO_RESET,                /* reset_reg 0x98, System Reset Register 2        */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),
	COMP_MUX_CONF("vcap_patclk", vcap_patclk_parent, 74250000, 0,       /* MCLK default is PLL9 with 74.25MHz             */
			  CG_IPP_CLK_RATE_REG0_OFFSET, BIT16, WID2,                 /* mux_reg   0x14, IPP Clock Rate Register        */
			  CG_IPP_CLK_DIV_REG0_OFFSET, BIT0, WID6,                   /* div_reg   0x30, IPP Clock Divider Register     */
			  CG_CLK_EN_REG2_OFFSET, BIT4, NOT_ENABLE,                  /* gate_reg  0x78, Module Clock Enable Register 2 */
			  0, 0, 0,                                                  /* reset_reg                                      */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),
	COMP_MUX_CONF("vcap2_aclk_m0", vcap_aclk_m_parent, 500000000, 0,    /* MCLK default is PLL0 with 500MHz               */
			  0, 0, 0,                                                  /* mux_reg,                                       */
			  0, 0, 0,                                                  /* div_reg                                        */
			  CG_CLK_EN_REG2_OFFSET, BIT8, NOT_ENABLE,                  /* gate_reg  0x78, Module Clock Enable Register 2 */
			  0, 0, 0,                                                  /* reset_reg                                      */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),
	COMP_MUX_CONF("vcap2_aclk_m1", vcap_aclk_m_parent, 500000000, 0,    /* MCLK default is PLL0 with 500MHz               */
			  0, 0, 0,                                                  /* mux_reg,                                       */
			  0, 0, 0,                                                  /* div_reg                                        */
			  CG_CLK_EN_REG2_OFFSET, BIT9, NOT_ENABLE,                  /* gate_reg  0x78, Module Clock Enable Register 2 */
			  0, 0, 0,                                                  /* reset_reg                                      */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),
	COMP_MUX_CONF("vcap2_aclk_m2", vcap_aclk_m_parent, 500000000, 0,    /* MCLK default is PLL0 with 500MHz               */
			  0, 0, 0,                                                  /* mux_reg,                                       */
			  0, 0, 0,                                                  /* div_reg                                        */
			  CG_CLK_EN_REG2_OFFSET, BIT10, NOT_ENABLE,                 /* gate_reg  0x78, Module Clock Enable Register 2 */
			  0, 0, 0,                                                  /* reset_reg                                      */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),
	COMP_MUX_CONF("vcap2_mclk", vcap_mclk_parent, 400000000, 0,         /* MCLK default is PLL19 with 400M                */
			  CG_IPP_CLK_RATE_REG0_OFFSET, BIT22, WID1,                 /* mux_reg   0x14, IPP Clock Rate Register        */
			  0, 0, 0,                                                  /* div_reg                                        */
			  CG_CLK_EN_REG2_OFFSET, BIT11, NOT_ENABLE,                 /* gate_reg  0x78, Module Clock Enable Register 2 */
			  CG_SYS_RESET_REG2_OFFSET, BIT25, DO_RESET,                /* reset_reg 0x98, System Reset Register 2        */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),
	COMP_MUX_CONF("vcap2_patclk", vcap_patclk_parent, 74250000, 0,      /* MCLK default is PLL9 with 74.25MHz             */
			  CG_IPP_CLK_RATE_REG0_OFFSET, BIT18, WID2,                 /* mux_reg   0x14, IPP Clock Rate Register        */
			  CG_IPP_CLK_DIV_REG0_OFFSET, BIT8, WID6,                   /* div_reg   0x30, IPP Clock Divider Register     */
			  CG_CLK_EN_REG2_OFFSET, BIT12, NOT_ENABLE,                 /* gate_reg  0x78, Module Clock Enable Register 2 */
			  0, 0, 0,                                                  /* reset_reg                                      */
			  0, 0, 0,                                                  /* autogating_reg                                 */
			  CG_RC_BASE),

	COMP_MUX_CONF("2f0950000.crypt", crypto_clk_parent, 400000000, 0,  /* MCLK default is PLL2 with 400MHz                 */
			  CG_PERI_CLK_RATE_REG1_OFFSET, BIT10, WID1,                /* mux_reg   0x24, Peripheral Clock Rate Register 1 */
			  0, 0, 0,                                                  /* div_reg                                          */
			  CG_CLK_EN_REG0_OFFSET, BIT31, NOT_ENABLE,                 /* gate_reg  0x70, Module Clock Enable Register 0   */
			  CG_SYS_RESET_REG1_OFFSET, BIT22, DO_RESET,                /* reset_reg 0x94, System Reset Register 1          */
			  0, 0, 0,                                                  /* autogating_reg                                   */
			  CG_RC_BASE),
	COMP_MUX_CONF("2f0960000.hash", hash_clk_parent, 400000000, 0,      /* MCLK default is PLL2 with 400MHz                 */
			  CG_PERI_CLK_RATE_REG1_OFFSET, BIT8, WID1,                 /* mux_reg   0x24, Peripheral Clock Rate Register 1 */
			  0, 0, 0,                                                  /* div_reg                                          */
			  CG_CLK_EN_REG0_OFFSET, BIT29, NOT_ENABLE,                 /* gate_reg  0x70, Module Clock Enable Register 0   */
			  CG_SYS_RESET_REG1_OFFSET, BIT20, DO_RESET,                /* reset_reg 0x94, System Reset Register 1          */
			  0, 0, 0,                                                  /* autogating_reg                                   */
			  CG_RC_BASE),
	COMP_MUX_CONF("2f0a80000.rsa", rsa_clk_parent, 400000000, 0,        /* MCLK default is PLL2 with 400MHz                 */
			  CG_PERI_CLK_RATE_REG1_OFFSET, BIT9, WID1,                 /* mux_reg   0x24, Peripheral Clock Rate Register 1 */
			  0, 0, 0,                                                  /* div_reg                                          */
			  CG_CLK_EN_REG0_OFFSET, BIT30, NOT_ENABLE,                 /* gate_reg  0x70, Module Clock Enable Register 0   */
			  CG_SYS_RESET_REG1_OFFSET, BIT21, DO_RESET,                /* reset_reg 0x94, System Reset Register 1          */
			  0, 0, 0,                                                  /* autogating_reg                                   */
			  CG_RC_BASE),

	COMP_MUX_CONF("2f0320000.jpeg", jpeg_clk_parent, 48000000, 0,		/* mclk default is fix480m with 480M */
			CG_CODEC_CLK_RATE_REG_OFFSET, BIT0, WID2,					/* 0x0, clock select */
			0, 0, 0,													/* div_reg */
			CG_CLK_EN_REG0_OFFSET, BIT22, NOT_ENABLE,					/* 0x70, Module Clock Enable Register 0 */
			CG_SYS_RESET_REG1_OFFSET, BIT10, DO_RESET,					/* 0x94, System Reset Register1 */
			CG_CLK_AUTO_GATING_REG0_OFFSET, BIT6, NOT_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
			CG_RC_BASE),

	COMP_MUX_CONF("2f0920000.ai", nue_clk_parent, 600000000, 0,		/* mclk default is pll8 with 600M*/
			CG_IPP_CLK_RATE_REG0_OFFSET, BIT12, WID1,					/* 0x0, clock select */
			0, 0, 0,													/* div_reg */
			CG_CLK_EN_REG1_OFFSET, BIT15, NOT_ENABLE,					/* 0x74, Module Clock Enable Register 0 */
			CG_SYS_RESET_REG2_OFFSET, BIT6, DO_RESET,					/* 0x98, System Reset Register1 */
			CG_CLK_AUTO_GATING_REG0_OFFSET, BIT30, DO_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
			CG_RC_BASE),

	COMP_MUX_CONF("2f0930000.ai", nue2_clk_parent, 600000000, 0,		/* mclk default is pll8 with 600M*/
			CG_IPP_CLK_RATE_REG0_OFFSET, BIT14, WID1,					/* 0x0, clock select */
			0, 0, 0,													/* div_reg */
			CG_CLK_EN_REG1_OFFSET, BIT16, NOT_ENABLE,					/* 0x74, Module Clock Enable Register 0 */
			CG_SYS_RESET_REG2_OFFSET, BIT7, DO_RESET,                   /* 0x98, System Reset Register1 */
			CG_CLK_AUTO_GATING_REG0_OFFSET, BIT31, DO_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
			CG_RC_BASE),
	COMP_MUX_CONF("2f0400000.ai", cnn_clk_parent, 600000000, 0,		/* mclk default is pll8 with 600M*/
			CG_IPP_CLK_RATE_REG0_OFFSET, BIT6, WID1,					/* 0x0, clock select */
			0, 0, 0,													/* div_reg */
			CG_CLK_EN_REG0_OFFSET, BIT16, NOT_ENABLE,					/* 0x70, Module Clock Enable Register 0 */
			CG_SYS_RESET_REG1_OFFSET, BIT4, DO_RESET,					/* 0x94, System Reset Register1 */
			CG_CLK_AUTO_GATING_REG0_OFFSET, BIT0, DO_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
			CG_RC_BASE),

	COMP_MUX_CONF("2f0900000.ai", cnn2_clk_parent, 600000000, 0,		/* mclk default is pll8 with 600M*/
			CG_IPP_CLK_RATE_REG0_OFFSET, BIT8, WID1,					/* 0x0, clock select */
			0, 0, 0,													/* div_reg */
			CG_CLK_EN_REG0_OFFSET, BIT17, NOT_ENABLE,					/* 0x70, Module Clock Enable Register 0 */
			CG_SYS_RESET_REG1_OFFSET, BIT5, DO_RESET,                   /* 0x94, System Reset Register1 */
			CG_CLK_AUTO_GATING_REG0_OFFSET, BIT1, DO_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
			CG_RC_BASE),
	COMP_MUX_CONF("2f0910000.ai", cnn3_clk_parent, 600000000, 0,		/* mclk default is pll8 with 600M*/
			CG_IPP_CLK_RATE_REG0_OFFSET, BIT10, WID1,					/* 0x0, clock select */
			0, 0, 0,													/* div_reg */
			CG_CLK_EN_REG0_OFFSET, BIT18, NOT_ENABLE,					/* 0x70, Module Clock Enable Register 0 */
			CG_SYS_RESET_REG1_OFFSET, BIT6, DO_RESET,                   /* 0x94, System Reset Register1 */
			CG_CLK_AUTO_GATING_REG0_OFFSET, BIT2, DO_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
			CG_RC_BASE),
	COMP_MUX_CONF("2f0940000.ive", ive_clk_parent, 480000000, 0,		/* mclk default is fix480m with 480M*/
			CG_IPP_CLK_RATE_REG0_OFFSET, BIT0, WID2,					/* 0x0, clock select */
			0, 0, 0,													/* div_reg */
			CG_CLK_EN_REG1_OFFSET, BIT18, NOT_ENABLE,					/* 0x70, Module Clock Enable Register 0 */
			CG_SYS_RESET_REG2_OFFSET, BIT9, DO_RESET,                   /* 0x94, System Reset Register1 */
			CG_CLK_AUTO_GATING_REG1_OFFSET, BIT1, DO_AUTOGATING,		/* 0xC0, Clock Auto Gating Register0 */
			CG_RC_BASE),

	COMP_MUX_CONF("2f0780000.csi", csi_clk_parent, 60000000, 0,         /* MCLK default is fix60m with 60Mhz              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT0, WID1,                 /* mux_reg   0x28, b[0x0]                         */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG3_OFFSET, BIT24, NOT_ENABLE,                 /* gate_reg  0x7C, b[0x24]                        */
		      CG_SYS_RESET_REG4_OFFSET, BIT0, NOT_RESET,                /* reset_reg 0xA0, b[0x0]                         */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT8, DO_AUTOGATING,  /* autogating_reg 0xD8, b[0x8]                    */
		      CG_RC_BASE),
	COMP_MUX_CONF("2f0790000.csi", csi_clk_parent, 60000000, 0,         /* MCLK default is fix60m with 60Mhz              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT1, WID1,                 /* mux_reg   0x28, b[0x1]                         */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG3_OFFSET, BIT25, NOT_ENABLE,                 /* gate_reg  0x7C, b[0x25]                        */
		      CG_SYS_RESET_REG4_OFFSET, BIT1, NOT_RESET,                /* reset_reg 0xA0, b[0x1]                         */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT9, DO_AUTOGATING,  /* autogating_reg 0xD8, b[0x9]                    */
		      CG_RC_BASE),
	COMP_MUX_CONF("2f07a0000.csi", csi_clk_parent, 60000000, 0,         /* MCLK default is fix60m with 60Mhz              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT2, WID1,                 /* mux_reg   0x28, b[0x2]                         */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG3_OFFSET, BIT26, NOT_ENABLE,                 /* gate_reg  0x7C, b[0x26]                        */
		      CG_SYS_RESET_REG4_OFFSET, BIT2, NOT_RESET,                /* reset_reg 0xA0, b[0x2]                         */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT10, NOT_AUTOGATING, /* autogating_reg 0xD8, b[0x10]                   */
		      CG_RC_BASE),
	COMP_MUX_CONF("2f07b0000.csi", csi_clk_parent, 60000000, 0,         /* MCLK default is fix60m with 60Mhz              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT3, WID1,                 /* mux_reg   0x28, b[0x3]                         */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG3_OFFSET, BIT27, NOT_ENABLE,                 /* gate_reg  0x7C, b[0x27]                        */
		      CG_SYS_RESET_REG4_OFFSET, BIT3, NOT_RESET,                /* reset_reg 0xA0, b[0x3]                         */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT11, DO_AUTOGATING, /* autogating_reg 0xD8, b[0x11]                   */
		      CG_RC_BASE),
	COMP_MUX_CONF("2f07c0000.csi", csi_clk_parent, 60000000, 0,         /* MCLK default is fix60m with 60Mhz              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT4, WID1,                 /* mux_reg   0x28, b[0x4]                         */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG3_OFFSET, BIT28, NOT_ENABLE,                 /* gate_reg  0x7C, b[0x28]                        */
		      CG_SYS_RESET_REG4_OFFSET, BIT4, NOT_RESET,                /* reset_reg 0xA0, b[0x4]                         */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT12, NOT_AUTOGATING, /* autogating_reg 0xD8, b[0x12]                   */
		      CG_RC_BASE),
	COMP_MUX_CONF("2f07d0000.csi", csi_clk_parent, 60000000, 0,         /* MCLK default is fix60m with 60Mhz              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT5, WID1,                 /* mux_reg   0x28, b[0x5]                         */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG3_OFFSET, BIT29, NOT_ENABLE,                 /* gate_reg  0x7C, b[0x29]                        */
		      CG_SYS_RESET_REG4_OFFSET, BIT5, NOT_RESET,                /* reset_reg 0xA0, b[0x5]                         */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT13, DO_AUTOGATING, /* autogating_reg 0xD8, b[0x13]                   */
		      CG_RC_BASE),
	COMP_MUX_CONF("2f07e0000.csi", csi_clk_parent, 60000000, 0,         /* MCLK default is fix60m with 60Mhz              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT6, WID1,                 /* mux_reg   0x28, b[0x6]                         */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG3_OFFSET, BIT30, NOT_ENABLE,                 /* gate_reg  0x7C, b[0x30]                        */
		      CG_SYS_RESET_REG4_OFFSET, BIT6, NOT_RESET,                /* reset_reg 0xA0, b[0x6]                         */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT14, NOT_AUTOGATING, /* autogating_reg 0xD8, b[0x14]                   */
		      CG_RC_BASE),
	COMP_MUX_CONF("2f07f0000.csi", csi_clk_parent, 60000000, 0,         /* MCLK default is fix60m with 60Mhz              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT7, WID1,                 /* mux_reg   0x28, b[0x7]                         */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG3_OFFSET, BIT31, NOT_ENABLE,                 /* gate_reg  0x7C, b[0x31]                        */
		      CG_SYS_RESET_REG4_OFFSET, BIT7, NOT_RESET,                /* reset_reg 0xA0, b[0x7]                         */
		      CG_APB_CLK_AUTO_GATING_REG2_OFFSET, BIT15, DO_AUTOGATING, /* autogating_reg 0xD8, b[0x15]                   */
		      CG_RC_BASE),
	COMP_MUX_CONF("csi_pxclk", csi_pxclk_parent, 300000000, 0,          /* MCLK default is is pll4 with 300M              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT16, WID2,                /* mux_reg   0x28, b[0x17..16]                    */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG4_OFFSET, BIT16, NOT_ENABLE,                 /* gate_reg  0x80, b[0x16]                        */
		      0, 0, 0,                                                  /* reset_reg                                      */
		      0, 0, 0,                                                  /* autogating_reg                                 */
		      CG_RC_BASE),
	COMP_MUX_CONF("csi2_pxclk", csi_pxclk_parent, 300000000, 0,         /* MCLK default is is pll4 with 300M              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT18, WID2,                /* mux_reg   0x28, b[0x19..18]                    */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG4_OFFSET, BIT17, NOT_ENABLE,                 /* gate_reg  0x80, b[0x17]                        */
		      0, 0, 0,                                                  /* reset_reg                                      */
		      0, 0, 0,                                                  /* autogating_reg                                 */
		      CG_RC_BASE),
	COMP_MUX_CONF("csi3_pxclk", csi_pxclk_parent, 300000000, 0,         /* MCLK default is is pll4 with 300M              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT20, WID2,                /* mux_reg   0x28, b[0x21..20]                    */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG4_OFFSET, BIT18, NOT_ENABLE,                 /* gate_reg  0x80, b[0x18]                        */
		      0, 0, 0,                                                  /* reset_reg                                      */
		      0, 0, 0,                                                  /* autogating_reg                                 */
		      CG_RC_BASE),
	COMP_MUX_CONF("csi4_pxclk", csi_pxclk_parent, 300000000, 0,         /* MCLK default is is pll4 with 300M              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT22, WID2,                /* mux_reg   0x28, b[0x23..22]                    */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG4_OFFSET, BIT19, NOT_ENABLE,                 /* gate_reg  0x80, b[0x19]                        */
		      0, 0, 0,                                                  /* reset_reg                                      */
		      0, 0, 0,                                                  /* autogating_reg                                 */
		      CG_RC_BASE),
	COMP_MUX_CONF("csi5_pxclk", csi_pxclk_parent, 300000000, 0,         /* MCLK default is is pll4 with 300M              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT24, WID2,                /* mux_reg   0x28, b[0x25..24]                    */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG4_OFFSET, BIT20, NOT_ENABLE,                 /* gate_reg  0x80, b[0x20]                        */
		      0, 0, 0,                                                  /* reset_reg                                      */
		      0, 0, 0,                                                  /* autogating_reg                                 */
		      CG_RC_BASE),
	COMP_MUX_CONF("csi6_pxclk", csi_pxclk_parent, 300000000, 0,         /* MCLK default is is pll4 with 300M              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT26, WID2,                /* mux_reg   0x28, b[0x27..26]                    */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG4_OFFSET, BIT21, NOT_ENABLE,                 /* gate_reg  0x80, b[0x21]                        */
		      0, 0, 0,                                                  /* reset_reg                                      */
		      0, 0, 0,                                                  /* autogating_reg                                 */
		      CG_RC_BASE),
	COMP_MUX_CONF("csi7_pxclk", csi_pxclk_parent, 300000000, 0,         /* MCLK default is is pll4 with 300M              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT28, WID2,                /* mux_reg   0x28, b[0x29..28]                    */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG4_OFFSET, BIT22, NOT_ENABLE,                 /* gate_reg  0x80, b[0x22]                        */
		      0, 0, 0,                                                  /* reset_reg                                      */
		      0, 0, 0,                                                  /* autogating_reg                                 */
		      CG_RC_BASE),
	COMP_MUX_CONF("csi8_pxclk", csi_pxclk_parent, 300000000, 0,         /* MCLK default is is pll4 with 300M              */
		      CG_PERI_CLK_RATE_REG2_OFFSET, BIT30, WID2,                /* mux_reg   0x28, b[0x31..30]                    */
		      0, 0, 0,                                                  /* drv_reg                                        */
		      CG_CLK_EN_REG4_OFFSET, BIT23, NOT_ENABLE,                 /* gate_reg  0x80, b[0x23]                        */
		      0, 0, 0,                                                  /* reset_reg                                      */
		      0, 0, 0,                                                  /* autogating_reg                                 */
		      CG_RC_BASE),

	COMP_MUX_CONF("ext0_clk", ext_clk_parent, 27000000, 0,
	          CG_PERI_CLK_RATE_REG1_OFFSET, BIT0, WID2,
	          CG_PERI_CLK_DIV_REG3_OFFSET, BIT0, WID5,
	          CG_CLK_EN_REG4_OFFSET, BIT12, NOT_ENABLE,
	          0, 0, 0,
	          0, 0, 0,
	          CG_RC_BASE),
	COMP_MUX_CONF("ext1_clk", ext_clk_parent, 27000000, 0,
	          CG_PERI_CLK_RATE_REG1_OFFSET, BIT2, WID2,
	          CG_PERI_CLK_DIV_REG3_OFFSET, BIT8, WID5,
	          CG_CLK_EN_REG4_OFFSET, BIT13, NOT_ENABLE,
	          0, 0, 0,
	          0, 0, 0,
	          CG_RC_BASE),
	COMP_MUX_CONF("ext2_clk", ext_clk_parent, 27000000, 0,
	          CG_PERI_CLK_RATE_REG1_OFFSET, BIT4, WID2,
	          CG_PERI_CLK_DIV_REG3_OFFSET, BIT16, WID5,
	          CG_CLK_EN_REG4_OFFSET, BIT14, NOT_ENABLE,
	          0, 0, 0,
	          0, 0, 0,
	          CG_RC_BASE),
	COMP_MUX_CONF("ext3_clk", ext_clk_parent, 27000000, 0,
	          CG_PERI_CLK_RATE_REG1_OFFSET, BIT6, WID2,
	          CG_PERI_CLK_DIV_REG3_OFFSET, BIT24, WID5,
	          CG_CLK_EN_REG4_OFFSET, BIT15, NOT_ENABLE,
	          0, 0, 0,
	          0, 0, 0,
	          CG_RC_BASE),

	COMP_MUX_CONF("extclk_from", extclk_form_parent, 0, 0,
	          CG_EXT_CLK_OUT_SRC_REG_OFFSET, BIT0, WID2,
	          0, 0, 0,
	          0, 0, 0, 0, 0,
	          0, 0, 0, 0, 0),

	COMP_MUX_CONF("extclk2_from", extclk2_form_parent, 0, 0,
	          CG_EXT_CLK_OUT_SRC_REG_OFFSET, BIT2, WID2,
	          0, 0, 0,
	          0, 0, 0, 0, 0,
	          0, 0, 0, 0, 0),

	COMP_MUX_CONF("extclk3_from", extclk3_form_parent, 0, 0,
	          CG_EXT_CLK_OUT_SRC_REG_OFFSET, BIT4, WID2,
	          0, 0, 0,
	          0, 0, 0, 0, 0,
	          0, 0, 0, 0, 0),

	COMP_MUX_CONF("extclk4_from", extclk4_form_parent, 0, 0,
	          CG_EXT_CLK_OUT_SRC_REG_OFFSET, BIT6, WID2,
	          0, 0, 0,
	          0, 0, 0, 0, 0,
	          0, 0, 0, 0, 0),

	COMP_MUX_CONF("cnn_shared_sram", cnn_shared_sram_parent, 0, 0,
		      CG_SYS_CLK_RATE_REG_OFFSET, BIT4, WID1,
		      0, 0, 0,
		      0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0),

	COMP_MUX_CONF("2f0850000.dai", audio_clk_parent, 12288000, 0,
	          0, 0, 0,
	          CG_DAI_CLK_DIV_REG0_OFFSET, BIT0, WID8,
	          CG_CLK_EN_REG3_OFFSET, BIT16, 0, 0, 0,
	          0, 0, 0, 0, 0),

	COMP_MUX_CONF("2f0860000.dai2", audio_clk_parent, 12288000, 0,
	          0, 0, 0,
	          CG_DAI_CLK_DIV_REG0_OFFSET, BIT8, WID8,
	          CG_CLK_EN_REG3_OFFSET, BIT17, 0, 0, 0,
	          0, 0, 0, 0, 0),

	COMP_MUX_CONF("2f0220000.dai3", audio_clk_parent, 12288000, 0,
	          0, 0, 0,
	          CG_DAI_CLK_DIV_REG0_OFFSET, BIT16, WID8,
	          CG_CLK_EN_REG3_OFFSET, BIT18, 0, 0, 0,
	          0, 0, 0, 0, 0),

	COMP_MUX_CONF("2f0230000.dai4", audio_clk_parent, 12288000, 0,
	          0, 0, 0,
	          CG_DAI_CLK_DIV_REG0_OFFSET, BIT24, WID8,
	          CG_CLK_EN_REG3_OFFSET, BIT19, 0, 0, 0,
	          0, 0, 0, 0, 0),

	COMP_MUX_CONF("2f0870000.dai5", audio_clk_parent, 12288000, 0,
	          0, 0, 0,
	          CG_DAI_CLK_DIV_REG1_OFFSET, BIT0, WID8,
	          CG_CLK_EN_REG3_OFFSET, BIT20, 0, 0, 0,
	          0, 0, 0, 0, 0),

    COMP_MUX_CONF("dai_mclk", audio_clk_parent, 12288000, 0,
              0, 0, 0,
			  CG_DAI_MCLK_DIV_REG_OFFSET, BIT0, WID8,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0),

    COMP_MUX_CONF("dai2_mclk", audio_clk_parent, 12288000, 0,
              0, 0, 0,
			  CG_DAI_MCLK_DIV_REG_OFFSET, BIT8, WID8,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0),

    COMP_MUX_CONF("dai3_mclk", audio_clk_parent, 12288000, 0,
              0, 0, 0,
			  CG_DAI_MCLK_DIV_REG_OFFSET, BIT16, WID8,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0),

    COMP_MUX_CONF("dai4_mclk", audio_clk_parent, 12288000, 0,
              0, 0, 0,
			  CG_DAI_MCLK_DIV_REG_OFFSET, BIT24, WID8,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0),

	COMP_MUX_CONF("2f0150000.rng", rng_clk_parent, 160000000, 0,
			0, 0, 0,
			CG_PERI_CLK_DIV_REG2_OFFSET, BIT24, WID8,
			CG_CLK_EN_REG2_OFFSET, BIT28, NOT_ENABLE,
			CG_SYS_RESET_REG3_OFFSET, BIT8, NOT_RESET,
			0, 0, 0,
			CG_RC_BASE),

	COMP_MUX_CONF("2f0150000.rngro", rngro_clk_parent, 24000000, 0,
			0, 0, 0,
			CG_PERI_CLK_DIV_REG2_OFFSET, BIT0, WID8,
			CG_CLK_EN_REG2_OFFSET, BIT29, NOT_ENABLE,
			0, 0, 0,
			0, 0, 0,
			CG_RC_BASE),
};

static struct nvt_clk_gating_init novatek_init_gating_clk[] __initdata = {
	COMP_GATING_CONF(CG_CLK_AUTO_GATING_REG0_OFFSET, 0x00000000,
			CG_RC_BASE),
	COMP_GATING_CONF(CG_CLK_AUTO_GATING_REG1_OFFSET, 0x00000000,
			CG_RC_BASE),
	COMP_GATING_CONF(CG_APB_CLK_AUTO_GATING_REG0_OFFSET, 0x00000000,
			CG_RC_BASE),
	COMP_GATING_CONF(CG_APB_CLK_AUTO_GATING_REG1_OFFSET, 0x00000000,
			CG_RC_BASE),
	COMP_GATING_CONF(CG_APB_CLK_AUTO_GATING_REG2_OFFSET, 0x00000000,
			CG_RC_BASE),
};

static int novatek_misc_clk_register(void)
{
	struct clk *clk;
	struct clk_hw *clk_hw;
	int ret = 0;


	clk =
	    clk_register_mux(NULL, "apb_clk", apb_clk_parent,
			     ARRAY_SIZE(apb_clk_parent),
			     CLK_SET_RATE_PARENT | CLK_SET_RATE_NO_REPARENT |
			     CLK_GET_RATE_NOCACHE,
			     remap_base + CG_SYS_CLK_RATE_REG_OFFSET, 8, 1, 0,
			     &cg_lock);
	if (IS_ERR(clk)) {
		pr_err("%s: failed to register clock hardware \"apb_clk\"\n",
		       __func__);
		ret = -EPERM;
	}
	if (clk_register_clkdev(clk, "apb_clk", NULL)) {
		pr_err("%s: failed to register clock device \"apb_clk\"\n",
		       __func__);
		ret = -EPERM;
	} else {
		clk_hw = __clk_get_hw(clk);
		ret = clk_prepare_enable(clk);
		if (ret < 0)
			pr_err("apb_clk prepare & enable failed!\n");
	}

	clk = clk_register_fixed_factor(NULL, "pll6div6", "pll6", 0, 1, 6);
	if (IS_ERR(clk)) {
		pr_err("%s: failed to register clock hardware \"pll6div6\"\n",
		       __func__);
		ret = -EPERM;
	} else {
		ret = clk_prepare_enable(clk);
		if (ret < 0)
			pr_err("pll6div6 prepare & enable failed!\n");
	}

	return ret;
}

static int max_freq_node = 0;
static int freq_index = 0;
static void nvt_get_max_frequency(struct nvt_composite_mux_clk composite_mux_clks[], int cmux_array_size,
				struct nvt_composite_gate_clk composite_gate_clks[], int cgate_array_size)
{
	int i = 0, j = 0;

	max_freq_node = ARRAY_SIZE(nvt_fixed_max_freq);

	for (i = 0; i < cmux_array_size; i++) {
		composite_mux_clks[i].max_rate = 0;
		for (j = 0; j < max_freq_node; j++) {
			if (strncmp(composite_mux_clks[i].name, nvt_fixed_max_freq[j].name, strlen(composite_mux_clks[i].name)) == 0) {
				composite_mux_clks[i].max_rate = nvt_fixed_max_freq[j].max_freq[freq_index];
			}
		}
	}

	for (i = 0; i < cgate_array_size; i++) {
		composite_gate_clks[i].max_rate = 0;
		for (j = 0; j < max_freq_node; j++) {
			if (strncmp(composite_gate_clks[i].name, nvt_fixed_max_freq[j].name, strlen(composite_gate_clks[i].name)) == 0) {
				composite_gate_clks[i].max_rate = nvt_fixed_max_freq[j].max_freq[freq_index];
			}
		}
	}
}

int nvt_get_max_freq_node(void)
{
	return max_freq_node;
}

void nvt_get_max_freq_info(int index, char *name, int *freq)
{
	strncpy(name, nvt_fixed_max_freq[index].name, CLK_NAME_STR_SIZE);
	*freq = nvt_fixed_max_freq[index].max_freq[freq_index];
}

static void nvt_get_keeper_array(struct nvt_composite_mux_clk composite_mux_clks[], int cmux_array_size,
			struct nvt_composite_gate_clk composite_gate_clks[], int cgate_array_size)
{
	struct device_node *node, *keeper_node;
	int i, j, keeper_count = 0;
	struct nvt_clk_rate_keeper *keeper;
	const char *name_ptr = NULL;
	unsigned int rate;
	unsigned int parent_idx;
	unsigned int do_reset;

	keeper = kzalloc(CLK_NAME_STR_CNT * sizeof(struct nvt_clk_rate_keeper), GFP_KERNEL);
	node = of_find_node_by_path("/cg_keeper@0");
	if (node) {
		for_each_child_of_node(node, keeper_node) {
			name_ptr = keeper[keeper_count].name;
			if (!of_property_read_string(keeper_node, "keeper_name", &name_ptr)) {
				strncpy(keeper[keeper_count].name, name_ptr, CLK_NAME_STR_SIZE);
				
				if (of_property_read_u32(keeper_node, "do_reset",&do_reset))
                    do_reset = UINT_MAX;
                keeper[keeper_count].do_reset = do_reset;

				if (of_property_read_u32(keeper_node, "parent_idx",&parent_idx))
                	parent_idx = UINT_MAX;
				keeper[keeper_count].parent_idx = parent_idx;

            	if (of_property_read_u32(keeper_node, "rate",&rate))
                	rate = UINT_MAX;
                keeper[keeper_count].rate = rate;
				
				keeper_count++;
				if (keeper_count == CLK_NAME_STR_CNT) {
					pr_err("exceed keeper limitation");
					break;
				}
			}

		}
	}

	for (i = 0; i < cmux_array_size; i++) {
		composite_mux_clks[i].keep_rate = 0;
		for (j = 0; j < keeper_count; j++) {
			if (strncmp(composite_mux_clks[i].name, keeper[j].name, strlen(composite_mux_clks[i].name)) == 0) {
				if(keeper[j].do_reset != UINT_MAX)
					composite_mux_clks[i].do_reset = (keeper[j].do_reset>0)?DO_RESET:NOT_RESET;
				
				if(keeper[j].parent_idx != UINT_MAX)
					composite_mux_clks[i].parent_idx = keeper[j].parent_idx;

				if(keeper[j].rate != UINT_MAX)
					composite_mux_clks[i].current_rate = keeper[j].rate;
				else
					composite_mux_clks[i].keep_rate = 1;
	
			}
		}
	}

	for (i = 0; i < cgate_array_size; i++) {
		composite_gate_clks[i].keep_rate = 0;
		for (j = 0; j < keeper_count; j++) {
			if (strncmp(composite_gate_clks[i].name, keeper[j].name, strlen(composite_gate_clks[i].name)) == 0) {
				composite_gate_clks[i].keep_rate = 1;
			}
		}
	}
}

static void __init clk_nvt_init(struct device_node *node)
{
	struct resource res;
	int ret;
	//int fastboot_init = 0;

	//fastboot_init = fastboot_determination();
	//nvt_fastboot_detect(fastboot_init);

	spin_lock_init(&cg_lock);

	if (of_address_to_resource(node, 0, &res)) {
		pr_err("%s: Failed to get resource of cg_rc\n", __func__);
	} else {
		remap_base = ioremap(res.start, resource_size(&res));
		if (IS_ERR((void *)remap_base))
			pr_err("%s: Failed to remap cg_rc register\n", __func__);
		else
			nvt_cg_base_remap(remap_base);
	}

	pr_info("%s: cg_rc(0x%llx)\n", __func__, res.start);

	nvt_init_clk_auto_gating(novatek_init_gating_clk, ARRAY_SIZE(novatek_init_gating_clk), &cg_lock);

	ret = nvt_fixed_rate_clk_register(novatek_fixed_rate_clk, ARRAY_SIZE(novatek_fixed_rate_clk));
	if (ret < 0) {
		pr_err("%s: Failed to register fixed rate clk!\n", __func__);
	}

	ret = nvt_pll_clk_register(novatek_pll, ARRAY_SIZE(novatek_pll), DEFAULT_PLL_DIV_VALUE, &cg_lock);
	if (ret < 0) {
		pr_err("%s: Failed to register pll clk!\n", __func__);
	}

	ret = novatek_misc_clk_register();
	if (ret < 0) {
		pr_err("%s: Failed to register misc clk!\n", __func__);
	}

	nvt_get_max_frequency(novatek_cmux_clk, ARRAY_SIZE(novatek_cmux_clk),
		novatek_cgate_clk, ARRAY_SIZE(novatek_cgate_clk));

	nvt_get_keeper_array(novatek_cmux_clk, ARRAY_SIZE(novatek_cmux_clk),
		novatek_cgate_clk, ARRAY_SIZE(novatek_cgate_clk));

	ret = nvt_composite_gate_clk_register(novatek_cgate_clk, ARRAY_SIZE(novatek_cgate_clk), &cg_lock);
	if (ret < 0) {
		pr_err("%s: Failed to register composite gate clk!\n", __func__);
	}

	ret = nvt_composite_mux_clk_register(novatek_cmux_clk, ARRAY_SIZE(novatek_cmux_clk), &cg_lock);
	if (ret < 0) {
		pr_err("%s: Failed to register composite mux clk!\n", __func__);
	}

	// suppress pll noise
	writel(0x7F, remap_base + CG_CHARGE_PUMP_REG_OFFSET);

	/* Shift SATA frequency */
	writel(0xd0, remap_base + CG_SHIFT_SATA_FREQ_REG0_OFFSET);
	writel(0xa9, remap_base + CG_SHIFT_SATA_FREQ_REG1_OFFSET);
	writel(0x10, remap_base + CG_SHIFT_SATA_FREQ_REG2_OFFSET);

/*
	ret = novatek_eth_clk_register();
	if (ret < 0) {
		pr_err("%s: Failed to setup eth clk!\n", __func__);
	}
*/
}

CLK_OF_DECLARE(nvt_core_clk, "nvt,core_clk", clk_nvt_init);
