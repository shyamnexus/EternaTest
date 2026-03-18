// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Novatek, Inc
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <generic-phy.h>
#include <power-domain.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/arch/efuse_protected.h>
#include <asm/arch/IOAddress.h>

#include "pcie_dw_common.h"
#include "pcie_nvt.h"

#define PCIE_FIX_PRESET			(4)	// possible value 0~10

/* TOP defines */
#define TOP_BS_REG_OFS			0x00
#define TOP_BS_PCIE_EP_MSK		(1<<13)
#define TOP_BS_PCIE_LANE_MODE_MSK	(0x3<<14)
#define TOP_BS_PCIE_LANE_MODE_ONE_X4	(0)
#define TOP_BS_PCIE_LANE_MODE_TWO_X2	(1<<14)
#define TOP_BS_PCIE_LANE_MODE_FOUR_X1	(2<<14)
#define TOP_BS_PCIE_REFCLK_MSK		(1<<16)
#define TOP_BS_PCIE_REFCLK_MPLL		(0)
#define TOP_BS_PCIE_REFCLK_PAD		(1<<16)

#define TOP_CTRL_REG14_OFS		0x60
#define TOP_PCIE_RSTN_MSK		(1<<4)
#define TOP_PCIE_RSTN_LOW		(0)
#define TOP_PCIE_RSTN_HIGHZ		(1<<4)
#define TOP_PCIE_REFCLK_SRC_MSK		(1<<8)
#define TOP_PCIE_REFCLK_SRC_MPLL	(0)
#define TOP_PCIE_REFCLK_SRC_PAD		(1<<8)
#define TOP_PCIE_REFCLK_OUT_MSK		(1<<9)
#define TOP_PCIE_REFCLK_OUT_DIS		(0)
#define TOP_PCIE_REFCLK_OUT_EN		(1<<9)


/* CG defines */
#define CG_PLL_EN_REG_OFS		0x00
#define CG_PLL_STS_REG_OFS		0x04
#define CG_CLK_EN_REG3_OFS		0x7C
#define CG_RESET_REG6_OFS		0xA8

#define PCIE_CLKEN_MSK			(1<<24)		// PCIE1
#define PCIE2_CLKEN_MSK			(1<<25)		// PCIE2
#define PCIE3_CLKEN_MSK			(1<<26)		// PCIE3
#define PCIE4_CLKEN_MSK			(1<<27)		// PCIE4

#define PCIE_PHY_PORN_MSK		(1<<20)


static int bootstrap_read(struct pcie_nvt *pci)
{
	u32 val;

	val = readl(IOADDR_TOP_REG_BASE + TOP_BS_REG_OFS);
	if (val & TOP_BS_PCIE_EP_MSK) {
		printf("%s: RC driver probe, but bootstrap is EP\r\n", __func__);
		return -1;
	}
	switch (val&TOP_BS_PCIE_LANE_MODE_MSK) {
	case TOP_BS_PCIE_LANE_MODE_FOUR_X1:
		if (pci->num_lanes > 1) {
			printf("%s: DTS x%d , but bootstrap x1 ==> conflict\n", __func__, pci->num_lanes);
			return -1;
		}
		break;
	case TOP_BS_PCIE_LANE_MODE_ONE_X4:
		if (pci->num_lanes != 4) {
			printf("%s: DTS x%d , but bootstrap x4 ==> conflict\n", __func__, pci->num_lanes);
			return -1;
		}
		break;
	default:
		// x2 * 2
		// x2*1 + x1*2
		if (pci->num_lanes != 2) {
			printf("%s: DTS x%d , but bootstrap x2 ==> conflict\n", __func__, pci->num_lanes);
			return -1;
		}
		break;
	}

	if ((val&TOP_BS_PCIE_REFCLK_MSK) == TOP_BS_PCIE_REFCLK_PAD) {
		if (pci->refclk_out == 1) {
			printf("%s: DTS refclk out , but bootstrap refclk from pad ==> conflict\n", __func__);
			return -1;
		}
	} else {
		if (pci->refclk_out == 0) {
			printf("%s: DTS refclk from pad , but bootstrap refclk from MPLL ==> conflict\n", __func__);
			return -1;
		}
	}

	return 0;
}

static int pcie_clk_setup(struct pcie_nvt *pci)
{
	int i;
	u32 val;
	const int PLL_MSK = (1<<24) | (1<<13) | (1<<2);

	// REFCLK PLL enable
	val = readl(IOADDR_CG_REG_BASE + CG_PLL_EN_REG_OFS);
	val |= PLL_MSK;
	writel(val, IOADDR_CG_REG_BASE + CG_PLL_EN_REG_OFS);

	for (i=0; i<1000; i++) {
		val = readl(IOADDR_CG_REG_BASE + CG_PLL_STS_REG_OFS);
		if (val & PLL_MSK) {
			break;
		}
		udelay(3);
	}
	if (i >= 1000) {
		pr_err("%s: PLL enable timeout\r\n", __func__);
		return -1;
	}

	// clock enable
	val = readl(IOADDR_CG_REG_BASE + CG_CLK_EN_REG3_OFS);
	switch (pci->slot) {
	case 0:
		val |= PCIE_CLKEN_MSK;
		break;
	case 1:
		val |= PCIE2_CLKEN_MSK;
		break;
	case 2:
		val |= PCIE3_CLKEN_MSK;
		break;
	default:
		val |= PCIE4_CLKEN_MSK;
		break;
	}
	writel(val, IOADDR_CG_REG_BASE + CG_CLK_EN_REG3_OFS);

	// refclk control
	val = readl(IOADDR_TOP_REG_BASE + TOP_CTRL_REG14_OFS);
	if (pci->refclk_out) {
		val &= ~TOP_PCIE_REFCLK_SRC_MSK;
		val |= TOP_PCIE_REFCLK_OUT_EN;
	} else {
		val |= TOP_PCIE_REFCLK_SRC_PAD;
		val &= ~TOP_PCIE_REFCLK_OUT_MSK;
	}
	writel(val, IOADDR_TOP_REG_BASE + TOP_CTRL_REG14_OFS);

	return 0;
}

static int pcie_power_on(struct pcie_nvt *pci)
{
	u32 val;

	// assert PCIe controller reset signals
	val = readl(pci->top_base + NVT_PCIE_TOP_CTRL0_OFS);
	val &= ~NVT_PCIE_TOP_RESET_MSK;
	writel(val, pci->top_base + NVT_PCIE_TOP_CTRL0_OFS);

	// disable train_en while reset asserted
	val = readl(pci->top_base + NVT_PCIE_TOP_CTRL1_OFS);
	val &= ~NVT_PCIE_LINK_TRAIN_MSK;
	writel(val, pci->top_base + NVT_PCIE_TOP_CTRL1_OFS);

	// release PCIe phy register reset
	val = readl(pci->top_base + NVT_PCIE_TOP_CTRL0_OFS);
	val |= NVT_PCIE_TOP_PHY_APBRST_MSK;
	writel(val, pci->top_base + NVT_PCIE_TOP_CTRL0_OFS);

	// apply phy setting while PHY PORn
	if (pci->is_phy_init_done == 0) {
		generic_phy_reset(pci->p_phy);
		generic_phy_init(pci->p_phy);
		generic_phy_power_on(pci->p_phy);

		pci->is_phy_init_done = 1;
	}

	// release phy POR
	val = readl(IOADDR_CG_REG_BASE + CG_RESET_REG6_OFS);
	val |= PCIE_PHY_PORN_MSK;
	writel(val, IOADDR_CG_REG_BASE + CG_RESET_REG6_OFS);

	// release PCIe controller reset signals
	val = readl(pci->top_base + NVT_PCIE_TOP_CTRL0_OFS);
	val |= NVT_PCIE_TOP_RESET_MSK;
	writel(val, pci->top_base + NVT_PCIE_TOP_CTRL0_OFS);

	// Force EQ preset
	val = readl(pci->dw.dbi_base + GEN3_EQ_CONTROL_OFS);
	val &= ~GEN3_EQ_PSET_REQ_VEC_MSK;
	val |= FIELD_PREP(GEN3_EQ_PSET_REQ_VEC_MSK, GEN3_EQ_PSET_REQ_VEC_NVT);
	val &= ~GEN3_EQ_FOM_INC_INITIAL_MSK;
	writel(val, pci->dw.dbi_base + GEN3_EQ_CONTROL_OFS);

	// CTS 4.2.6.7.2. L1.Idle
	val = readl(pci->dw.dbi_base + GEN3_RELATED_OFS);
	val &= ~GEN3_ZRXDC_NONCOMPL_MSK;
	writel(val, pci->dw.dbi_base + GEN3_RELATED_OFS);

	val = readl(pci->dw.dbi_base + MISC_CONTROL_1_OFF);
	val |= DBI_RO_WR_EN_MSK;
	writel(val, pci->dw.dbi_base + MISC_CONTROL_1_OFF);

	val = readl(pci->dw.dbi_base + PORT_FORCE_OFF_OFS);
	val |= SUPPORT_PART_LANES_MSK;
	writel(val, pci->dw.dbi_base + PORT_FORCE_OFF_OFS);

	// Force detect
	switch (pci->num_lanes) {
	case 1:
		val = readl(pci->dw.dbi_base + GEN2_CTRL_OFF_OFS);
		val &= ~NUM_OF_LANES_MSK;
		val |= FIELD_PREP(NUM_OF_LANES_MSK, 1);
		writel(val, pci->dw.dbi_base + GEN2_CTRL_OFF_OFS);

		val = readl(pci->dw.dbi_base + PORT_LINK_CTRL_OFS);
		val &= ~LINK_CAPABLE_MSK;
		val |= FIELD_PREP(LINK_CAPABLE_MSK, LINK_CAPABLE_x1);
		writel(val, pci->dw.dbi_base + PORT_LINK_CTRL_OFS);
		break;
	case 2:
		val = readl(pci->dw.dbi_base + GEN2_CTRL_OFF_OFS);
		val &= ~NUM_OF_LANES_MSK;
		val |= FIELD_PREP(NUM_OF_LANES_MSK, 2);
		writel(val, pci->dw.dbi_base + GEN2_CTRL_OFF_OFS);

		val = readl(pci->dw.dbi_base + PORT_LINK_CTRL_OFS);
		val &= ~LINK_CAPABLE_MSK;
		val |= FIELD_PREP(LINK_CAPABLE_MSK, LINK_CAPABLE_x2);
		writel(val, pci->dw.dbi_base + PORT_LINK_CTRL_OFS);
		break;
	case 4:
	default:
		val = readl(pci->dw.dbi_base + GEN2_CTRL_OFF_OFS);
		val &= ~NUM_OF_LANES_MSK;
		val |= FIELD_PREP(NUM_OF_LANES_MSK, 4);
		writel(val, pci->dw.dbi_base + GEN2_CTRL_OFF_OFS);

		val = readl(pci->dw.dbi_base + PORT_LINK_CTRL_OFS);
		val &= ~LINK_CAPABLE_MSK;
		val |= FIELD_PREP(LINK_CAPABLE_MSK, LINK_CAPABLE_x4);
		writel(val, pci->dw.dbi_base + PORT_LINK_CTRL_OFS);
		break;
	}
#if 0
	if (pci->num_lanes >= 2) {
		val = readl(pci->dw.dbi_base + SD_CONTROL1_REG_OFS);
		val |= FORCE_DETECT_LANE_EN_MSK | FORCE_DETECT_LANE_X2_MSK;
		writel(val, pci->dw.dbi_base + SD_CONTROL1_REG_OFS);
	}
#endif

	// release PRESET (to EP)
	val = readl(IOADDR_TOP_REG_BASE + TOP_CTRL_REG14_OFS);
	val |= TOP_PCIE_RSTN_HIGHZ;
	writel(val, IOADDR_TOP_REG_BASE + TOP_CTRL_REG14_OFS);

	mdelay(200);


	return 0;
}

const struct nvt_pcie_plat nvt_plat_ns02401 = {
	.bootstrap_read =	bootstrap_read,
	.clk_setup =		pcie_clk_setup,
	.power_on =		pcie_power_on,
};

