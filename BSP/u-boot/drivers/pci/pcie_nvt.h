/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 *
 * PCIe DM U-Boot driver for Novatek SoCs
 */

#ifndef _PCIE_NVT_H_
#define _PCIE_NVT_H_

#include "pcie_dw_common.h"

/**
 * struct pcie_nvt - Novatek DW PCIe controller state
 *
 * @pci: The common PCIe DW structure
 * @app_base: The base address of application register space
 */
struct pcie_nvt {
	/* Must be first member of the struct */
	struct pcie_dw dw;
	struct phy *p_phy;
	void *top_base;
	int link_gen;
	u32 refclk_out;
	u32 num_lanes;
	u32 slot;

	int is_phy_init_done;		// record if phy ever inited

	const struct nvt_pcie_plat *p_nvt_plat;
};

struct nvt_pcie_plat {
	int (*bootstrap_read)(struct pcie_nvt *pci);
	int (*clk_setup)(struct pcie_nvt *pci);
	int (*power_on)(struct pcie_nvt *pci);
};

/* PCIE TOP registers */
#define NVT_PCIE_TOP_CTRL0_OFS		0x300
#define NVT_PCIE_TOP_RESET_MSK		GENMASK(2, 0)
#define NVT_PCIE_TOP_PHY_APBRST_MSK	GENMASK(3, 3)

#define NVT_PCIE_TOP_CTRL1_OFS				0x304
#define NVT_PCIE_LINK_DOWN_RST_ACK_MSK			GENMASK(0, 0)
#define NVT_PCIE_LINK_DOWN_RST_REQ_MSK			GENMASK(1, 1)
#define NVT_PCIE_LINK_TRAIN_MSK				GENMASK(2, 2)
#define NVT_PCIE_LINK_TRAIN_EN				(1)
#define NVT_PCIE_LINK_TRAIN_DIS				(0)

/* PCIE DBI registers */
#define SD_CONTROL1_REG_OFS		0x274
#define FORCE_DETECT_LANE_EN_MSK	GENMASK(16, 16)
#define FORCE_DETECT_LANE_X2_MSK	GENMASK(1, 0)

#define PORT_FORCE_OFF_OFS		0x708
#define SUPPORT_PART_LANES_MSK		GENMASK(22, 22)

#define PORT_LINK_CTRL_OFS		0x710
#define HOT_RESET_MSK			GENMASK(3, 3)
#define LINK_CAPABLE_MSK		GENMASK(21, 16)
#define LINK_CAPABLE_x1			(1)
#define LINK_CAPABLE_x2			(3)
#define LINK_CAPABLE_x4			(7)

#define GEN2_CTRL_OFF_OFS		0x80C
#define NUM_OF_LANES_MSK		GENMASK(12, 8)

#define GEN3_RELATED_OFS		0x890
#define GEN3_ZRXDC_NONCOMPL_MSK		GENMASK(0, 0)

#define MISC_CONTROL_1_OFF		0x8BC
#define DBI_RO_WR_EN_MSK		GENMASK(0, 0)

#define GEN3_EQ_CONTROL_OFS		0x8a8
#define GEN3_EQ_FOM_INC_INITIAL_MSK	GENMASK(24, 24)
#define GEN3_EQ_FOM_INC_INITIAL_YES	(1)
#define GEN3_EQ_FOM_INC_INITIAL_NO	(0)
#define GEN3_EQ_PSET_REQ_VEC_MSK	GENMASK(23, 8)
#define GEN3_EQ_PSET_REQ_VEC_NVT		(1<<PCIE_FIX_PRESET)
#define GEN3_EQ_AUTO_PSET_REQ_VEC_NVT	(0x1f) // auto preset P0~P4

#define BAR0_MASK_OFS			(0x100010)

// exported from various nvt soc platform
#if defined(CONFIG_TARGET_NS02401_A64)
extern const struct nvt_pcie_plat nvt_plat_ns02401;
#endif

#endif	// _PCIE_NVT_H_
