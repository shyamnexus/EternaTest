/**
    @file       pcie-nvt.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _PCIE_NVT_H
#define _PCIE_NVT_H

#include <plat/hardware.h>

#define NVT_PCIE_CONTROLLER_VERSION	"0.1.07"

#define NVT_PCIE_PIPE_LOOPBACK		(1)

#define PCI_VENDOR_ID_NOVATEK       0x1e09
#define PCI_DEVICE_ID_NOVATEK_98336 0x0005
#define PCI_DEVICE_ID_NOVATEK_98690 0x0006
#define PCI_DEVICE_ID_NOVATEK_98635 0x0007
#define PCI_GET_VENDOR_ID(sign)     (sign & 0xffff)

#define PCIE_FIX_PRESET			(4)	// possible value 0~10

#define to_nvt_plat_pcie(x)	dev_get_drvdata((x)->dev)

struct nvt_plat_linkdown_hdl_data {
	spinlock_t			lock;
	int				footprint;
};

enum {
    EOM_SM_IDLE,
    EOM_SM_SCAN_ONGOING,
    EOM_SM_SCAN_DONE,
};

struct nvt_plat_pcie {
	struct dw_pcie			*pci;
	struct regmap			*regmap;
	enum dw_pcie_device_mode	mode;
	void __iomem			*top_base;
	void __iomem			*ptr_phy;
	struct phy			*phy;
	struct clk			*p_clk_phy_porn;
	struct clk			*p_clk_ref;	// PCIE REFCLK
	struct clk			*p_clk_top;	// module clock
	unsigned int			link_stat;
	u32				slot;
	int				dts_count;

	struct delayed_work		deffered_linkdown;
	struct nvt_plat_linkdown_hdl_data	linkdown_hdl_data;

	/* dts controlled setting */
	int				refclk_out;
	int				link_gen;
	int 				auto_preset;
	int 				exclude_ib;	// exclude access outside inbound regions

	/* To record for ep device drivers list */
	struct list_head		ep_info_list_root;      // To record all of the ep devices info
	struct list_head		ep_info_notifier_head;  // To notify all registered functions
	struct semaphore		sema_ep_info;

	/* peer link partner info */
	u8				pcie_cap;	/* PCIe capability offset */

	/* nvt proprietary */
	int				refclk_from_pad;
	int				bs_lane_width;

	/* hw setting */
	int				bs_lane_mode;

	/* pcie top irq controller */
	struct irq_domain		*top_irq_domain;

	spinlock_t              lock_eom;
	spinlock_t              lock_ras;
	spinlock_t              lock_int;

	struct delayed_work     eom_work;                       /* work queue for scan rx eye */
	int                     state_eom;                      /* sw state machine for scan rx eye */
	int                     eom_result[64][128];
	unsigned long           eom_start_jiffies;
	unsigned long           eom_end_jiffies;

	struct nvt_plat_pcie		*p_next;
};

struct nvt_plat_pcie_of_data {
	enum dw_pcie_device_mode	mode;
};

#define PRESET_COUNT	(5+1+1)		// initial preset + P0~4 + final

struct nvt_plat_pcie_link_eq {
	void __iomem		*ptr_phy;
	u32 v_fom_history[PRESET_COUNT];
	u32 v_history_R[PRESET_COUNT];
	u32 v_history_C[PRESET_COUNT];
};

#ifdef CONFIG_PROC_FS
int nvt_plat_pcie_proc_init(struct device *dev, struct dw_pcie *pci);
#else
static int inline nvt_plat_pcie_proc_init(struct device *dev, struct dw_pcie *pci)
{
	return 0;
}
#endif /* CONFIG_PROC_FS */

/* exported from pcie-nvt-util.c */
extern int nvt_pcie_get_speed_cap(struct pcie_port *pp);

typedef enum {
	PCIE_EC_EN_PEROFF = 0x1,
	PCIE_EC_EN_PERON = 0x3,
	PCIE_EC_EN_ALLOFF = 0x5,
	PCIE_EC_EN_ALLON = 0x7,
	PCIE_EC_EN_COUNT = 4
} PCIE_EC_EN_SEL;

typedef enum {
	PCIE_EC_CLEAR_NOCHANGE = 0x0,
	PCIE_EC_CLEAR_PERCLEAR,
	PCIE_EC_CLEAR_NOCHANGE2,
	PCIE_EC_CLEAR_ALLCLEAR,
	PCIE_EC_CLEAR_COUNT
} PCIE_EC_CLEAR_SEL;

typedef enum {
	PCIE_LINK_STAT_ERR = 0x0,
	PCIE_LINK_STAT_RC_INB_ERR,
	PCIE_LINK_STAT_OK = 0x1000
} PCIE_LINK_STAT;

typedef enum {
	PCIE_EQ_CURSOR_P0 = 0x0A780,
	PCIE_EQ_CURSOR_P1 = 0x07840,
	PCIE_EQ_CURSOR_P2 = 0x08800,
	PCIE_EQ_CURSOR_P3 = 0x058C0,
	PCIE_EQ_CURSOR_P4 = 0x00A00,
	PCIE_EQ_CURSOR_P5 = 0x00904,
	PCIE_EQ_CURSOR_P6 = 0x008C5,
	PCIE_EQ_CURSOR_P7 = 0x08704,
	PCIE_EQ_CURSOR_P8 = 0x05785,
	PCIE_EQ_CURSOR_P9 = 0x00847,
	PCIE_EQ_CURSOR_P10 = 0x0D6C0,
} PCIE_EQ_CURSOR;

#if 0
/* TOP registers */
#define TOP_BOOT_CTRL_OFS		(0x00)
#define TOP_PCIE_REFCLK_SRC_MSK		GENMASK(16, 16)
#define TOP_PCIE_REFCLK_SRC_MPLL	(0)
#define TOP_PCIE_REFCLK_SRC_PAD		(1)
#define TOP_PCIE_LANEMODE_MSK		GENMASK(15, 15)
#define TOP_PCIE_LANEMODE_ONE_x2	(0)
#define TOP_PCIE_LANEMODE_TWO_x1	(1)
#define TOP_PCIE_BOOT_MSK		GENMASK(14, 14)
#define TOP_PCIE_BOOT_RC		(0)
#define TOP_PCIE_BOOT_EP		(1)

#define TOP_PCIE_CTRL_OFS		(0x60)
#define TOP_PCIE_REFCLK_MSK		GENMASK(9, 9)
#define TOP_PCIE_REFCLK_OUE_EN		(1)
#define TOP_PCIE_REFCLK_OUE_DIS		(0)
#define TOP_PCIE_PRESETN_MSK		GENMASK(4, 4)
#define TOP_PCIE_PRESETN_HIGHZ		(1)
#define TOP_PCIE_PRESETN_LOW		(0)
#define TOP_PCIE_CTRL2_MODE_MSK		GENMASK(1, 1)
#define TOP_PCIE_MODE_EP		(1)
#define TOP_PCIE_MODE_RC		(0)
#endif

/* NVT PCIE Top offset group */
#define NVT_PCIE_TOP_DMC_STS0_OFS			0x08
#define NVT_PCIE_TOP_DMC_LINK_MSK			GENMASK(0, 0)

#define NVT_PCIE_TOP_INT_STS_OFS			0x20
#define NVT_PCIE_TOP_INT_MASK_OFS			0x24
#define NVT_PCIE_TOP_INT_ALL_MSK			(0xFFFFFFFF)
#define NVT_PCIE_TOP_INT_LINK_DONW_MSK			GENMASK(29, 29)
#define NVT_PCIE_TOP_INT_MSI_MSK			GENMASK(26, 26)
#define NVT_PCIE_TOP_INT_EDMA_MSK			GENMASK(22, 22)

#define NVT_PCIE_TOP_DEBUG_PIPE_CTL1_REG_OFS		0x25C
#define NVT_PCIE_TOP_FAKE_PHY_MSK			GENMASK(23, 23)

#define NVT_PCIE_TOP_CTRL0_OFS				0x300
#define NVT_PCIE_TOP_RESET_MSK				GENMASK(3, 0)
#define NVT_PCIE_TOP_PHY_APBRST_MSK			GENMASK(3, 3)

#define NVT_PCIE_TOP_CTRL1_OFS				0x304
#define NVT_PCIE_LINK_DOWN_RST_ACK_MSK			GENMASK(0, 0)
#define NVT_PCIE_LINK_DOWN_RST_REQ_MSK			GENMASK(1, 1)
#define NVT_PCIE_LINK_TRAIN_MSK				GENMASK(2, 2)
#define NVT_PCIE_LINK_TRAIN_EN				(1)
#define NVT_PCIE_LINK_TRAIN_DIS				(0)

#define NVT_PCIE_SOC_CTRL0_OFS				0x310
#define NVT_PCIE_AXIM_NON_BUFF_MSK			GENMASK(9, 9)
#define NVT_PCIE_AXIM_NON_BUFF_EN			(1)
#define NVT_PCIE_AXIM_NON_BUFF_DIS			(0)

#define NVT_PCIE_TOP_LOCAL_CLK_OFS			0x318
#define NVT_PCIE_REFCLK_MSK				GENMASK(22, 22)
#define NVT_PCIE_REFCLK_OUT_EN				(1)
#define NVT_PCIE_REFCLK_OUT_DIS				(0)

/* NVT PCIE DBI offset group */
#define LINK_CAPABILITIES_REG		0x7C
#define PCIE_CAP_MAX_LINK_WIDTH_OFS	4
#define PCIE_CAP_MAX_LINK_WIDTH_MSK	0x3F

#define LINK_CONTROL_LINK_STATUS_REG 	0x80
#define PCIE_CAP_DLL_ACTIVE_BIT 	(1 << 29)

#define LINK_CONTROL2_LINK_STATUS2_REG	0xA0

#define PCIE_EQ_STATUS_CURSOR_MSK	(0x3FFFF)


#define AER_UNCORR_STATUS_OFS           0x104
#define AER_CORR_STATUS_OFS             0x110

#define EVENT_COUNTER_CONTROL_OFS       0x1dc
#define EVENT_COUNTER_DATA_OFS          0x1e0

#define SD_CONTROL1_REG_OFS		0x274
#define FORCE_DETECT_LANE_EN_MSK	GENMASK(16, 16)
#define FORCE_DETECT_LANE_X2_MSK	GENMASK(1, 0)

#define EQ_STATUS2_REG_OFS       	0x2b8
#define EQ_STATUS3_REG_OFS       	0x2bc

#define RESBAR_CTRL_REG_0_REG_OFS       0x2f4
#define RESBAR_CTRL636_REG_0_REG_OFS    0x2bc
#define RESBAR_CTRL635_REG_0_REG_OFS    0x304

#define PORT_FORCE_OFF_OFS		0x708
#define SUPPORT_PART_LANES_MSK		GENMASK(22, 22)

#define PORT_LINK_CTRL_OFF_OFS		0x710
#define LOOPBACK_ENABLE_MSK		GENMASK(2, 2)
#define HOT_RESET_MSK			GENMASK(3, 3)
#define LINK_CAPABLE_MSK		GENMASK(21, 16)

#define GEN3_RELATED_OFS		0x890
#define GEN3_ZRXDC_NONCOMPL_MSK		GENMASK(0, 0)

#define GEN3_EQ_CONTROL_OFS		0x8a8
#define GEN3_EQ_FOM_INC_INITIAL_MSK	GENMASK(24, 24)
#define GEN3_EQ_FOM_INC_INITIAL_YES	(1)
#define GEN3_EQ_FOM_INC_INITIAL_NO	(0)
#define GEN3_EQ_PSET_REQ_VEC_MSK	GENMASK(23, 8)
#define GEN3_EQ_PSET_REQ_VEC_NVT		(1<<PCIE_FIX_PRESET)
#define GEN3_EQ_AUTO_PSET_REQ_VEC_NVT	(0x1f) // auto preset P0~P4

#define PIPE_LOOPBACK_CONTROL_OFF_OFS	0x8b8
#define PIPE_LOOPBACK_MSK		GENMASK(31, 31)

#define ELBI_BASE_OFS			0xF00

#define BAR0_MASK_OFS			(0x100010)

/* iATU nvt management */
#define PCIE_ATU_REGION_INDEX_IO	(0x2 << 0)
#define PCIE_ATU_REGION_INDEX_CFG	(0x1 << 0)
#define PCIE_ATU_REGION_INDEX_MEM	(0x0 << 0)

/* NVT PCIE DBI2 offset group */

#endif /* _PCIE_NVT_H */
