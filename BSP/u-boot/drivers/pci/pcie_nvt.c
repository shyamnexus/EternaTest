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

#define DRV_VERSION                     "1.01.00"

DECLARE_GLOBAL_DATA_PTR;

#define PCIE_LINK_CAPABILITY		0x7c
#define PCIE_LINK_CTL_2			0xa0
#define TARGET_LINK_SPEED_MASK		0xf
#define LINK_SPEED_GEN_1		0x1
#define LINK_SPEED_GEN_2		0x2
#define LINK_SPEED_GEN_3		0x3

#define PLR_OFFSET			0x700
#define PCIE_PORT_DEBUG0		(PLR_OFFSET + 0x28)
#define PORT_LOGIC_LTSSM_STATE_MASK	0x1f
#define PORT_LOGIC_LTSSM_STATE_L0	0x11

#define PCIE_LINK_UP_TIMEOUT_MS		1000

//
// Novatek definitions
//

#define PCIE_FIX_PRESET			(4)	// possible value 0~10

/* TOP defines */
#define TOP_BS_REG_OFS			0x00
#define TOP_BS_PCIE_EP_MSK		(1<<15)
#define TOP_BS_PCIE_LANE_MODE_MSK	(1<<16)
#define TOP_BS_PCIE_LANE_MODE_ONE_X2	(0)
#define TOP_BS_PCIE_LANE_MODE_TWO_X1	(1<<16)
#define TOP_BS_PCIE_REFCLK_MSK		(1<<17)
#define TOP_BS_PCIE_REFCLK_MPLL		(0)
#define TOP_BS_PCIE_REFCLK_PAD		(1<<17)

#define TOP_CTRL_REG22_OFS		0x60
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
#define CG_CLK_EN_REG5_OFS		0x84
#define CG_RESET_REG7_OFS		0xAC

#define PCIE_CLKEN_MSK			(1<<25)		// PCIE1: x2 controller
#define PCIE2_CLKEN_MSK			(1<<26)		// PCIE2: x1 controller

#define PCIE_PHY_PORN_MSK		(1<<13)

enum dw_pcie_device_mode {
	DW_PCIE_UNKNOWN_TYPE,
	DW_PCIE_EP_TYPE,
	DW_PCIE_LEG_EP_TYPE,
	DW_PCIE_RC_TYPE,
};

static void pcie_enable_ltssm(struct pcie_nvt *pci);

/**
 * pcie_dw_configure() - Configure link capabilities and speed
 *
 * @regs_base: A pointer to the PCIe controller registers
 * @cap_speed: The capabilities and speed to configure
 *
 * Configure the link capabilities and speed in the PCIe root complex.
 */
static void pcie_dw_configure(struct pcie_nvt *pci, u32 cap_speed)
{
	u32 val;

	dw_pcie_dbi_write_enable(&pci->dw, true);

	val = readl(pci->dw.dbi_base + PCIE_LINK_CAPABILITY);
	val &= ~TARGET_LINK_SPEED_MASK;
	val |= cap_speed;
	writel(val, pci->dw.dbi_base + PCIE_LINK_CAPABILITY);

	val = readl(pci->dw.dbi_base + PCIE_LINK_CTL_2);
	val &= ~TARGET_LINK_SPEED_MASK;
	val |= cap_speed;
	writel(val, pci->dw.dbi_base + PCIE_LINK_CTL_2);

	dw_pcie_dbi_write_enable(&pci->dw, false);
}

/**
 * is_link_up() - Return the link state
 *
 * @regs_base: A pointer to the PCIe DBICS registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link
 */
static int is_link_up(struct pcie_nvt *pci)
{
	u32 val;

	val = readl(pci->dw.dbi_base + PCIE_PORT_DEBUG0);
	val &= PORT_LOGIC_LTSSM_STATE_MASK;

	return (val == PORT_LOGIC_LTSSM_STATE_L0);
}

/**
 * wait_link_up() - Wait for the link to come up
 *
 * @regs_base: A pointer to the PCIe controller registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link (timeout)
 */
static int wait_link_up(struct pcie_nvt *pci)
{
	unsigned long timeout;

	timeout = get_timer(0) + PCIE_LINK_UP_TIMEOUT_MS;
	while (!is_link_up(pci)) {
		if (get_timer(0) > timeout)
			return 0;
	};

	return 1;
}

static int pcie_dw_pcie_link_up(struct pcie_nvt *pci, u32 cap_speed)
{
	if (is_link_up(pci)) {
		printf("PCI Link already up before configuration!\n");
		return 1;
	}

	/* DW pre link configurations */
	pcie_dw_configure(pci, cap_speed);

	/* Initiate link training */
	pcie_enable_ltssm(pci);

	/* Check that link was established */
	if (!wait_link_up(pci))
		return 0;

	/*
	 * Link can be established in Gen 1. still need to wait
	 * till MAC nagaotiation is completed
	 */
	udelay(100);

	return 1;
}

#if !defined(CONFIG_TARGET_NS02401_A64)
static int bootstrap_read(struct pcie_nvt *pci)
{
	u32 val;

	val = readl(IOADDR_TOP_REG_BASE + TOP_BS_REG_OFS);
	if (val & TOP_BS_PCIE_EP_MSK) {
		printf("%s: RC driver probe, but bootstrap is EP\r\n", __func__);
		return -1;
	}
	if ((val&TOP_BS_PCIE_LANE_MODE_MSK) == TOP_BS_PCIE_LANE_MODE_TWO_X1) {
		// x1 * 2
		if (pci->num_lanes == 2) {
			printf("%s: DTS x2 , but bootstrap x1 ==> conflict\n", __func__);
			return -1;
		}
	} else {
		// x2 * 1
		if (pci->num_lanes == 1) {
			printf("%s: DTS x1 , but bootstrap x2 ==> conflict\n", __func__);
			return -1;
		}
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
	val = readl(IOADDR_CG_REG_BASE + CG_CLK_EN_REG5_OFS);
	if (pci->slot == 0) {
		val |= PCIE_CLKEN_MSK;
	} else {
		val |= PCIE2_CLKEN_MSK;
	}
	writel(val, IOADDR_CG_REG_BASE + CG_CLK_EN_REG5_OFS);

	// refclk control
	val = readl(IOADDR_TOP_REG_BASE + TOP_CTRL_REG22_OFS);
	if (pci->refclk_out) {
		val &= ~TOP_PCIE_REFCLK_SRC_MSK;
		val |= TOP_PCIE_REFCLK_OUT_EN;
	} else {
		val |= TOP_PCIE_REFCLK_SRC_PAD;
		val &= ~TOP_PCIE_REFCLK_OUT_MSK;
	}
	writel(val, IOADDR_TOP_REG_BASE + TOP_CTRL_REG22_OFS);

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
	val = readl(IOADDR_CG_REG_BASE + CG_RESET_REG7_OFS);
	val |= PCIE_PHY_PORN_MSK;
	writel(val, IOADDR_CG_REG_BASE + CG_RESET_REG7_OFS);

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
	if (pci->num_lanes == 2) {
		val = readl(pci->dw.dbi_base + SD_CONTROL1_REG_OFS);
		val |= FORCE_DETECT_LANE_EN_MSK | FORCE_DETECT_LANE_X2_MSK;
		writel(val, pci->dw.dbi_base + SD_CONTROL1_REG_OFS);
	}

	// release PRESET (to EP)
	val = readl(IOADDR_TOP_REG_BASE + TOP_CTRL_REG22_OFS);
	val |= TOP_PCIE_RSTN_HIGHZ;
	writel(val, IOADDR_TOP_REG_BASE + TOP_CTRL_REG22_OFS);

	mdelay(200);


	return 0;
}
#endif

static void pcie_enable_ltssm(struct pcie_nvt *pci)
{
	u32 val;

	val = readl(pci->top_base + NVT_PCIE_TOP_CTRL1_OFS);
	val |= FIELD_PREP(NVT_PCIE_LINK_TRAIN_MSK, NVT_PCIE_LINK_TRAIN_EN);
	writel(val, pci->top_base + NVT_PCIE_TOP_CTRL1_OFS);
}

static void nvt_plat_set_rc_bar0(struct pcie_nvt *pci)
{
	//Note: set RESBAR_CTRL_REG_0_REG
	//RESBAR_CTRL_REG_BAR_SIZE: 0x0 (bit 13:8) = 2^0 MB
	//RESBAR_CTRL_REG_NUM_BARS: 0x1 (bit 7:5)
	dw_pcie_dbi_write_enable(&pci->dw, true);
	writel(0x00, pci->dw.dbi_base + BAR0_MASK_OFS);
	debug("Disable RC BAR 0\n");
}

/**
 * pcie_nvt_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int pcie_nvt_probe(struct udevice *dev)
{
	struct pcie_nvt *pci = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	struct phy phy0;//, phy1;
	int phy_mode;
	int ret;

	if (pci == NULL) {
		dev_err(dev, "NULL priv\n");
		return -ENODEV;
	}

	if (pci->p_nvt_plat == NULL) {
		dev_err(dev, "NULL nvt plat callbacks\n");
		return -ENODEV;
	}

	ret = pci->p_nvt_plat->bootstrap_read(pci);
	if (ret) {
		dev_err(dev, "Bootstrap conflict with DTS\n");
		return ret;
	}

	pci->p_nvt_plat->clk_setup(pci);

	ret = generic_phy_get_by_name(dev,  "pcie-phy", &phy0);
	if (ret) {
		dev_err(dev, "Unable to get pcie-phy\n");
		return ret;
	}
	pci->p_phy = &phy0;

	if (pci->num_lanes == 1) {
		phy_mode = 1;	// x1 * 2
	} else {
		phy_mode = 0;	// x2 * 1
	}
	generic_phy_configure(&phy0, &phy_mode);
	pci->dw.first_busno = dev_seq(dev);
	pci->dw.dev = dev;


	pci->p_nvt_plat->power_on(pci);

	// link up
	if (!pcie_dw_pcie_link_up(pci, pci->link_gen)) {
		printf("PCIE-%d: Link down\n", dev_seq(dev));
		return -ENODEV;
	}

	nvt_plat_set_rc_bar0(pci);
	pcie_dw_setup_host(&pci->dw);

	// ensure speed change complete
	/* Check that link was established */
	if (!wait_link_up(pci)) {
		printf("PCIE-%d: Link up timeout after speed change\n", dev_seq(dev));
		return -ENODEV;
	}

	printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n", dev_seq(dev),
	       pcie_dw_get_link_speed(&pci->dw),
	       pcie_dw_get_link_width(&pci->dw),
	       hose->first_busno);

	pcie_dw_prog_outbound_atu_unroll(&pci->dw, PCIE_ATU_REGION_INDEX0,
					 PCIE_ATU_TYPE_MEM,
					 pci->dw.mem.phys_start,
					 pci->dw.mem.bus_start, pci->dw.mem.size);

	return 0;
}

#if !defined(CONFIG_TARGET_NS02401_A64)
static const struct nvt_pcie_plat nvt_plat_ns02201 = {
	.bootstrap_read =	bootstrap_read,
	.clk_setup =		pcie_clk_setup,
	.power_on =		pcie_power_on,
};
#endif

/**
 * pcie_nvt_of_to_plat() - Translate from DT to device state
 *
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pcie_nvt_of_to_plat(struct udevice *dev)
{
	int ret;
	int link_gen;
	u32 refclk_out;
	u32 num_lanes;
	u32 slot;
	struct pcie_nvt *pcie = dev_get_priv(dev);

	printf("%s: ver %s\r\n", __func__, DRV_VERSION);

	/* Get the controller base address */
	pcie->dw.dbi_base = (void *)dev_read_addr_name(dev, "dbi");
	if ((fdt_addr_t)pcie->dw.dbi_base == FDT_ADDR_T_NONE) {
		pr_err("DTS reg dbi not found\n");
		return -EINVAL;
	}

	/* Get the config space base address and size */
	pcie->dw.cfg_base = (void *)dev_read_addr_size_name(dev, "config",
							 &pcie->dw.cfg_size);
	if ((fdt_addr_t)pcie->dw.cfg_base == FDT_ADDR_T_NONE) {
		pr_err("DTS reg config not found\n");
		return -EINVAL;
	}

	/* Get the iATU base address and size */
	pcie->dw.atu_base = pcie->dw.dbi_base + DEFAULT_DBI_ATU_OFFSET;

	/* Get the app base address and size */
	pcie->top_base = (void *)dev_read_addr_name(dev, "nvt-pcie-top");
	if ((fdt_addr_t)pcie->top_base == FDT_ADDR_T_NONE) {
		pr_err("DTS reg nvt-pcie-top not found\n");
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "refclk-out", &refclk_out);
	if (ret) {
		pr_err("DTS param refclk-out not found\n");
		return -EINVAL;
	}
	pcie->refclk_out = refclk_out;

	ret = dev_read_u32(dev, "num-lanes", &num_lanes);
	if (ret) {
		pr_err("DTS param num-lanes not found\n");
		return -EINVAL;
	}
	pcie->num_lanes = num_lanes;

	ret = dev_read_u32(dev, "linux,pci-domain", &slot);
	if (ret) {
		pr_err("DTS param linux,pci-domain not found\n");
		return -EINVAL;
	}
	pcie->slot = slot;

	ret = dev_read_u32(dev, "max-link-speed", &link_gen);
	if (ret) {
		pr_err("DTS param max-link-speed not found\n");
		return -EINVAL;
	}
	if (link_gen < LINK_SPEED_GEN_1) {
		pr_err("max link speed gen %d not supported, force to gen1\r\n", link_gen);
		link_gen = LINK_SPEED_GEN_1;
	} else if (link_gen > LINK_SPEED_GEN_3) {
		pr_err("max link speed gen %d not supported, force to gen3\r\n", link_gen);
		link_gen = LINK_SPEED_GEN_3;
	}
	pcie->link_gen = link_gen;
	printf("%s: max link gen %d\r\n", __func__, link_gen);

#if defined(CONFIG_TARGET_NS02401_A64)
	pcie->p_nvt_plat = &nvt_plat_ns02401;
#else
	pcie->p_nvt_plat = &nvt_plat_ns02201;
#endif

	return 0;
}

static const struct dm_pci_ops pcie_nvt_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id pcie_nvt_ids[] = {
	{ .compatible = "nvt,nvt-dwc-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_nvt) = {
	.name			= "nvt-pcit",
	.id			= UCLASS_PCI,
	.of_match		= pcie_nvt_ids,
	.ops			= &pcie_nvt_ops,
	.of_to_plat		= pcie_nvt_of_to_plat,
	.probe			= pcie_nvt_probe,
	.priv_auto		= sizeof(struct pcie_nvt),
};

