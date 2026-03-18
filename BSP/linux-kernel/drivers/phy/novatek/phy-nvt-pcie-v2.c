/*
 * Copyright (C) 2023 Novatek Microelectronics Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/bitfield.h>

#include <plat/hardware.h>
#include <plat/efuse_protected.h>
#include <linux/soc/nvt/rcw_macro.h>

#define DRV_VERSION             "1.00.007"

/* TOP registers */
#define TOP_BOOT_CTRL_REG       0x00
#define PCIE_SATA_COMBO_MASK    GENMASK(9, 9)
#define PCIE_SATA_COMBO_SATA    0
#define PCIE_SATA_COMBO_PCIE    1

/* PHY registers */
#define PHY_RXODT_REG           0x0000
#define PHY_RXODT_CAL_MSK       GENMASK(23, 16)
#define PHY_RXODT_CAL_LONGER    0xFF

#define PHY_TXODT_REG           0x000C
#define PHY_TXODT_CAL_MSK       GENMASK(23, 22)
#define PHY_TXODT_CAL_LONGER    2
#define PHY_NODT_INI_MSK        GENMASK(21, 21)
#define PHY_NODT_INI_MID        1
#define PHY_PODT_INI_MSK        GENMASK(20, 20)
#define PHY_PODT_INI_MID        1

#define PHY_DFE_CTRL_REG        0x00A0
#define PHY_VREF_REL_MSK        GENMASK(30, 30)
#define PHY_VREF_REL_AUTO       1
#define PHY_VREF_INITIAL_MSK    GENMASK(29, 24)
#define PHY_VREF_INIT_VAL       0x3F
#define PHY_AGC_ACC_TH_MSK	GENMASK(18, 10)
#define PHY_AGC_ACC_TH_VAL	0x100

#define PHY_DFE_TH1_REG		0x00A4
#define PHY_DFE_TH1_MSK		GENMASK(21, 12)
#define PHY_DFE_TH1_VAL		0x100

#define PHY_DFE_TH2_REG		0x00A8
#define PHY_DFE_TH2_MSK		GENMASK(21, 12)
#define PHY_DFE_TH2_VAL		0x100

#define PHY_CTLE_ACC_REG        0x00B8
#define PHY_CTLE_ACC_MSK        GENMASK(10, 0)
#define PHY_CTLE_ACC_VAL        0x400

#define PHY_RXQEC_REG           0x0108
#define PHY_CLKQ_DLY_MSK        GENMASK(19, 17)
#define PHY_CLKQ_DLY_SETTING    1
#define PHY_CLKQ_DLY_ENA_MSK    GENMASK(9, 9)
#define PHY_CLKQ_DLY_EN         1

#define PHY_0218_REG            0x0218
#define PHY_T2R_PATH_MSK	GENMASK(5, 5)
#define PHY_T2R_PATH_EN		1

#define PHY_LOS_REG             0x0834
#define PHY_LOS_THSHLD_MSK      GENMASK(15, 12)
#define PHY_LOS_THSHLD_PCIE     0x04

#define PHY_085C_REG            0x085C
#define PHY_RX_MUX_SEL_MSK	GENMASK(30, 30)
#define PHY_TX_CLK_DEGLITCH_MSK	GENMASK(26, 26)

#define PHY_0918_REG            0x0918
#define PHY_RX_DEMUX_MSK	GENMASK(22, 22)
#define PHY_RX_DEMUX_ECO	1

#define PHY_1068_REG            0x1068
#define PHY_1068_B21_MSK        GENMASK(21, 21)
#define PHY_TX_ELEC_PH_MSK	GENMASK(20, 20)
#define PHY_TX_ELEC_PH_EN	0

#define PHY_1070_REG            0x1070
#define PHY_PTEN_RST_CTRL_MSK   GENMASK(8, 8)
#define PHY_PTEN_RST_BYPASS     1

#define PHY_AEQ_AGC_REG		0x1080
#define PHY_AEQ_TIME_MSK	GENMASK(13, 0)
#define PHY_AGC_TIME_MSK	GENMASK(29, 16)

#define PHY_PIPE_CTRL_REG       0x3010
#define PHY_EIOS_CHK_HDR_MSK    GENMASK(21, 21)
#define PHY_EIOS_CHK_HDR_NEW	1
#define PHY_PIPE_MODE_MSK       GENMASK(14, 14)
#define PHY_PIPE_MODE_CHG       1
#define PHY_EIOS_DET_MSK        GENMASK(3, 3)
#define PHY_EIOS_DET_DIS        1
#define PHY_EIOS_DET_EN         0

#define PHY_3014_REG            0x3014
#define PHY_3014_B7_MSK         GENMASK(7, 7)
#define PHY_RX_MUX_RST_MSK	GENMASK(3, 3)
#define PHY_RX_MUX_RST_EN	1

#define PHY_CMM_REG             0x4060
#define PHY_CMLR_PDB_MSK        GENMASK(3, 3)
#define PHY_CMLR_PDB_OFF        0
#define PHY_CMLR_PDB_ON         1

#define PHY_PHASE_CALIBRATION_REG  0x1210
#define PHY_PHASE_CALIBRATION_MSK  GENMASK(4, 4)
#define PHY_PHASE_CALIBRATION_VAL  0x1

#define PHY_PHASE_CALIBRATION_FOM_REG  0x1210
#define PHY_PHASE_CALIBRATION_FOM_MSK  GENMASK(19, 19)

#define PHY_FOM_CTRL_REG  0x1028
#define PHY_FOM_CTRL_MSK  GENMASK(11, 11)
#define PHY_FOM_CTRL_VAL  0x1

#define PHY_FOM_PARAM_REG  0x102C
#define PHY_FOM_PARAM_MSK  GENMASK(16, 16)
#define PHY_FOM_PARAM_VAL  0x1

#define PHY_PRESET_REG  0x1304
#define PHY_PRESET_MSK  GENMASK(25, 22)
#define PHY_PRESET_VAL  0x7
#define PHY_PRESET_MANUAL_MSK  GENMASK(21, 21)
#define PHY_PRESET_MANUAL_VAL  0x1


#define PHY_AEQ_REG  0x091C
#define PHY_AEQ_MSK  GENMASK(16, 16)

#define PHY_TUNE_R_REG  0x00AC
#define PHY_TUNE_R_MSK  GENMASK(22, 22)

#define PHY_MANUAL_C_REG  0x00AC
#define PHY_MANUAL_C_MSK  GENMASK(23, 23)
#define PHY_MANUAL_C_VAL  0x1

#define PHY_FIX_C_REG  0x00A8
#define PHY_FIX_C_MSK  GENMASK(27, 25)
#define PHY_FIX_C_VAL  0x4

#define PHY_40DC_REG            0x40DC


enum nvt_pcie_phy_id {
	NVT_PHY_PCIE0 = 0,
	NVT_PHY_PCIE1 = 1,
	MAX_NUM_PHYS,
};

enum nvt_pcie_phy_mode {
	NVT_PHY_ONE_x2 = 0,
	NVT_PHY_TWO_x1 = 1,
	MAX_NUM_PHY_MODE,
};

struct nvt_pcie_phy_core;

/**
 * struct nvt_pcie_phy - Novatek PCIe PHY device
 * @core: pointer to the Novatek PCIe PHY core control
 * @id: internal ID to identify the Novatek PCIe PHY
 * @phy: pointer to the kernel PHY device
 */
struct nvt_pcie_phy {
	struct nvt_pcie_phy_core *core;
	enum nvt_pcie_phy_id id;
	struct phy *phy;

	u32 is_rint;	// 0: use rext, 1: use rint
	u32 rint;
	u32 tx_odt_p;
	u32 tx_odt_n;
	u32 tx_odt_s;
};

/**
 * struct nvt_pcie_phy_core - Novatek PCIe PHY core control
 * @dev: pointer to device
 * @base: base register
 * @lock: mutex to protect access to individual PHYs
 * @phys: pointer to Novatek PHY device
 */
struct nvt_pcie_phy_core {
	struct device *dev;
	void __iomem *v_base[2];
	int en_l1_detect;
	enum nvt_pcie_phy_mode mode;
	struct mutex lock;
	struct nvt_pcie_phy phys[MAX_NUM_PHYS];
};

static void rc_apbm_srmw(void __iomem *addr, u32 data, u32 start, u32 width) {
	const u32 MASK = (1<<width) - 1;
	u32 data_tmp;

	data_tmp = readl(addr);
	data_tmp = (data_tmp & (~(MASK << start))) | ((data & MASK) << start);
	writel(data_tmp, addr);
}

static int nvt_pcie_phy_init(struct phy *p)
{
//	const u32 TRIM = 0x030B09;
	struct nvt_pcie_phy *phy = phy_get_drvdata(p);
	struct nvt_pcie_phy_core *core = phy->core;
	int idx;

	if (core->phys[NVT_PHY_PCIE0].is_rint) {
		rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x5064, 0, 5, 1);	// CMM_CTRL_IREXT = 1'b1
		rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x5004, 0, 0, 1);	// 0: RINT, 1: REXT

		rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x5004,
			core->phys[NVT_PHY_PCIE0].rint & 0x1f,
			1, 5); // RINT
	} else {
		rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x5064, 1, 5, 1);	// CMM_CTRL_IREXT = 1'b1
		rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x5004, 1, 0, 1);	// 0: RINT, 1: REXT
	}

	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x0008,
		core->phys[NVT_PHY_PCIE0].tx_odt_p & 0xf,
		8, 4); // TX_ODT_P
	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x0008,
		core->phys[NVT_PHY_PCIE0].tx_odt_n & 0xf,
		4, 4); // TX_ODT_N
	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x0008,
		core->phys[NVT_PHY_PCIE0].tx_odt_s & 0xf,
		12, 4); // TX_ODT_S
	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE1]+0x0008,
		core->phys[NVT_PHY_PCIE1].tx_odt_p & 0xf,
		8, 4); // TX_ODT_P
	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE1]+0x0008,
		core->phys[NVT_PHY_PCIE1].tx_odt_n & 0xf,
		4, 4); // TX_ODT_N
	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE1]+0x0008,
		core->phys[NVT_PHY_PCIE1].tx_odt_s & 0xf,
		12, 4); // TX_ODT_S

	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x4080, 0x1, 0, 5); //REG_CP
	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x404C, 0x2, 4, 3); //REG_VCO_LDOSEL
	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x4070, 0x1, 7, 1); //CMM_TX_SWCTRL
	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x4070, 0x2, 4, 2); //CMM_TX_SWCTRL

	//for690 eco Lane0
	rc_apbm_srmw(core->v_base[NVT_PHY_PCIE0]+0x40dc, 0x6, 0, 4); //REG_LPF_RSW[3:0]

	for(idx=0;idx<MAX_NUM_PHYS;idx++){
		rc_apbm_srmw(core->v_base[idx]+0x10fc, 0x7d0, 0, 11); // CMM_CAL
		rc_apbm_srmw(core->v_base[idx]+0x1070, 0x1, 8, 1); // cfg_pt_en_byp_reset
		rc_apbm_srmw(core->v_base[idx]+0x948, 0x1, 17, 1); // PHY_L RX_DCC_new
		rc_apbm_srmw(core->v_base[idx]+0x0c0, 0x0, 18, 1); // TXEQ from pipe tx preset sel
		rc_apbm_srmw(core->v_base[idx]+0xb8, 0x400, 0, 11); //AEQ parameter  ctle accu th
		rc_apbm_srmw(core->v_base[idx]+0x1080, 0x3fff, 0, 14); // AEQ parameter aeq time period
		rc_apbm_srmw(core->v_base[idx]+0xa0, 0x100, 10, 9); // AGC/VTH parameter agc acc th
		rc_apbm_srmw(core->v_base[idx]+0x1080, 0x3fff, 16, 14); // AGC/VTH parameter agc time period/dfe time period
		{
			rc_apbm_srmw(core->v_base[idx]+0x850, 0x1, 1, 1); //RX_CDR_DIV64_SEL
			rc_apbm_srmw(core->v_base[idx]+0x3020, 0x1, 15, 1); //reg_pcie_rx_halfrate_ECO
			rc_apbm_srmw(core->v_base[idx]+0x948, 0x1, 24, 1); //cfg_tc_clk_PLL_CDR_sel_ECO
			rc_apbm_srmw(core->v_base[idx]+0x948, 0xF, 20, 4); //cfg_dlpf_clk_tc_div_PLLMODE
		}
		rc_apbm_srmw(core->v_base[idx]+0x820, 0x0, 30, 2); // TX_AMP_BOOST
		rc_apbm_srmw(core->v_base[idx]+0x3014, 0xA, 24, 4); // TX demp -6dB
		rc_apbm_srmw(core->v_base[idx]+0x3014, 0x7, 28, 4); // TX demp -3.5dB
		if(core->mode == NVT_PHY_TWO_x1){
			rc_apbm_srmw(core->v_base[idx]+0x81c, 0x2, 12, 2); // TXPUB_CKBUF8G_IS 5G
			rc_apbm_srmw(core->v_base[idx]+0x858, 0x2, 17, 2); // RXPUB_CKBUF8G_IS 5G
		}
		rc_apbm_srmw(core->v_base[idx]+0x228, 0x1, 4, 1); // RX_EN_ACFB_PK1 =1
		rc_apbm_srmw(core->v_base[idx]+0x228, 0x1, 6, 1); // RX_EN_ACFB_PK2 = 1
		rc_apbm_srmw(core->v_base[idx]+0x228, 0x1, 7, 1); // RX_ACFB_RSEL_PK2 = 1
		rc_apbm_srmw(core->v_base[idx]+0x228, 0x1, 8, 3); // RX_LFEQ_RFB[2:0] = 3’b001
		rc_apbm_srmw(core->v_base[idx]+0x228, 0x3, 12, 4); // RX_LFEQ_CFB[3:0] = 4’b0011
		rc_apbm_srmw(core->v_base[idx]+0x228, 0x4, 16, 4); // RX_RLFEQ[3:0] = 4’b0100
		rc_apbm_srmw(core->v_base[idx]+0x4054, 0xb, 0, 4); // REG_CALRESAH_CTL
		rc_apbm_srmw(core->v_base[idx]+0x4054, 0x6, 4, 4); // REG_CALRESAL_CTL
		rc_apbm_srmw(core->v_base[idx]+0x4058, 0xd, 0, 4); // REG_CALRESBH_CTL
		rc_apbm_srmw(core->v_base[idx]+0x4058, 0x1, 4, 4); // REG_CALRESBL_CTL
		rc_apbm_srmw(core->v_base[idx]+0x3010, 0x1, 14, 1); // L1CPM mode
		if(core->mode == NVT_PHY_ONE_x2){
			rc_apbm_srmw(core->v_base[idx]+0x3010, 0x0, 8, 1); // add/remove skip for L0/L1
		}
		//for690 ECO
		rc_apbm_srmw(core->v_base[idx]+0x3020, 0x1, 12, 1); // reg_dec128b_cnt_clr
		rc_apbm_srmw(core->v_base[idx]+0x3004, 0x1, 0, 1); // reg_bypass_rxPLL
		rc_apbm_srmw(core->v_base[idx]+0x1210, 0x1, 4, 1); // bypass phase k (FOM)
		rc_apbm_srmw(core->v_base[idx]+0x3020, 0x1, 15, 1); // reg_pcie_rx_halfrate_ECO
		rc_apbm_srmw(core->v_base[idx]+0x3020, 0x1, 16, 1); // reg_dec128b_sym_cnt_clr_sel
		rc_apbm_srmw(core->v_base[idx]+0x00, 0xff, 16, 8); // RXODT
		rc_apbm_srmw(core->v_base[idx]+0x3010, 0x1, 31, 1); // FIFO start read index
		rc_apbm_srmw(core->v_base[idx]+0x3010, 0x10, 24, 7); // FIFO start read index
		rc_apbm_srmw(core->v_base[idx]+0x1208, 0x1, 11, 1); // dphy rst
	}
	return 0;
}

static int nvt_pcie_phy_power_on(struct phy *p)
{
//	struct nvt_pcie_phy *phy = phy_get_drvdata(p);

	return 0;
}

static int nvt_pcie_phy_power_off(struct phy *p)
{
//	struct nvt_pcie_phy *phy = phy_get_drvdata(p);

	return 0;
}

static int nvt_pcie_set_mode(struct phy *p, enum phy_mode mode, int submode)
{
	struct nvt_pcie_phy *phy = phy_get_drvdata(p);
	struct nvt_pcie_phy_core *core = phy->core;

	if (mode == PHY_MODE_PCIE) {
		switch (submode) {
		case NVT_PHY_ONE_x2:
			core->mode = NVT_PHY_ONE_x2;
			break;
		case NVT_PHY_TWO_x1:
			core->mode = NVT_PHY_TWO_x1;
			break;
		default:
			break;
		}
	}

	return 0;
}

static int nvt_pcie_phy_reset(struct phy *p)
{
	//NT98690 don't need this patch.
	return 0;
}

static const struct phy_ops nvt_pcie_phy_ops = {
	.init = nvt_pcie_phy_init,
	.power_on = nvt_pcie_phy_power_on,
	.power_off = nvt_pcie_phy_power_off,
	.set_mode = nvt_pcie_set_mode,
	.reset = nvt_pcie_phy_reset,
	.owner = THIS_MODULE,
};

static int load_efuse_trim(struct nvt_pcie_phy_core *core)
{
	s32 addr;
	s32 trim = EFUSE_INVALIDATE_ERR;
	u16 value;
	u32 PCIe_internal_resistor;
	u32 PHY0_TX_ODT_P, PHY0_TX_ODT_N, PHY0_TX_ODT_S;
	u32 PHY1_TX_ODT_P, PHY1_TX_ODT_N, PHY1_TX_ODT_S;

	if (core == NULL) {
		pr_err("core NULL\n");
		return -1;
	}

	addr = 0x30;
#if IS_ENABLED(CONFIG_NVT_OTP_SUPPORT)
	trim = efuse_readParamOps(EFUSE_PCIE_RINT_TRIM_DATA, &value);
#else
	pr_alert("CONFIG_NVT_OTP_SUPPORT is disabled, use default trim\r\n");
#endif
	if (trim == EFUSE_SUCCESS) {
		pr_info("   PCIe internal resistor trim success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
		//bit[4..0]
		PCIe_internal_resistor = (value & 0x1F);           //0x7 ~ 0x19
		if (PCIe_internal_resistor < 0x7) {
			pr_info("   PCIe internal resistor 0x7 <= PCIe_internal_resistor[0x%04x] <= 0x19 error\r\n", (int)PCIe_internal_resistor);
		} else if (PCIe_internal_resistor > 0x19) {
			pr_info("   PCIe internal resistor 0x7 <= PCIe_internal_resistor[0x%04x] <= 0x19 error\r\n", (int)PCIe_internal_resistor);
		} else {
			pr_info("    PCIe internal resistor Trim data range success 0x7 <= PCIe_internal_resistor[0x%04x] <= 0x19\r\n", (int)PCIe_internal_resistor);
		}

		core->phys[0].is_rint = 1;
		core->phys[0].rint = PCIe_internal_resistor;
	} else {
		pr_info("addr[%02x] error %d\r\n", (int)addr, (int)value);
		core->phys[0].is_rint = 0;
	}

	addr = 0x31;
#if IS_ENABLED(CONFIG_NVT_OTP_SUPPORT)
	trim = efuse_readParamOps(EFUSE_PCIE0_ODT_TRIM_DATA, &value);
#endif
	if (trim == EFUSE_SUCCESS) {
		pr_info("   PCIe0 ODT trim success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
		//bit[ 3..0]    0x4 <= xx <= 0xE
		PHY0_TX_ODT_P = (value & 0xF);
		//bit[ 7..4]    0x4 <= xx <= 0xE
		PHY0_TX_ODT_N = ((value >> 4) & 0xF);
		//bit[11..8]    0x0 <= xx <= 0xF
		PHY0_TX_ODT_S = ((value >> 8) & 0xF);
		//0x4 <= xx <= 0xE
		if (PHY0_TX_ODT_P < 0x4 || PHY0_TX_ODT_P > 0xE) {
			pr_info("   PCIe0 TX ODT P 0x4 <= PHY0_TX_ODT_P[0x%04x] <= 0xE error\r\n", (int)PHY0_TX_ODT_P);
		} else {
			pr_info("    PCIe0 TX ODT P Trim data range success 0x4 <= PHY0_TX_ODT_P[0x%04x] <= 0xE\r\n", (int)PHY0_TX_ODT_P);
		}
		//0x4 <= xx <= 0xE
		if (PHY0_TX_ODT_N < 0x4 || PHY0_TX_ODT_N > 0xE) {
			pr_info("   PCIe0 TX ODT N 0x4 <= PHY0_TX_ODT_N[0x%04x] <= 0xE error\r\n", (int)PHY0_TX_ODT_N);
		} else {
			pr_info("    PCIe0 TX ODT N Trim data range success 0x4 <= PHY0_TX_ODT_N[0x%04x] <= 0xE\r\n", (int)PHY0_TX_ODT_N);
		}

		//0x0 <= xx <= 0xF
		if (PHY0_TX_ODT_S < 0x0 || PHY0_TX_ODT_S > 0xF) {
			pr_info("   PCIe0 TX ODT S 0x0 <= PHY0_TX_ODT_S[0x%04x] <= 0xF error\r\n", (int)PHY0_TX_ODT_S);
		} else {
			pr_info("    PCIe0 TX ODT S Trim data range success 0x0 <= PHY0_TX_ODT_S[0x%04x] <= 0xF\r\n", (int)PHY0_TX_ODT_S);
		}

		core->phys[0].tx_odt_p = PHY0_TX_ODT_P;
		core->phys[0].tx_odt_n = PHY0_TX_ODT_N;
		core->phys[0].tx_odt_s = PHY0_TX_ODT_S;
	} else {
		pr_info("addr[%02x] error %d\r\n", (int)addr, (int)value);

		core->phys[0].tx_odt_p = 0x9;
		core->phys[0].tx_odt_n = 0xB;
		core->phys[0].tx_odt_s = 0x3;
	}

	addr = 0x32;
#if IS_ENABLED(CONFIG_NVT_OTP_SUPPORT)
	trim = efuse_readParamOps(EFUSE_PCIE1_ODT_TRIM_DATA, &value);
#endif
	if (trim == EFUSE_SUCCESS) {
		pr_info("   PCIe1 ODT trim success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
		//bit[ 3..0]    0x4 <= xx <= 0xE
		PHY1_TX_ODT_P = (value & 0xF);
		//bit[ 7..4]    0x4 <= xx <= 0xE
		PHY1_TX_ODT_N = ((value >> 4) & 0xF);
		//bit[11..8]    0x0 <= xx <= 0xF
		PHY1_TX_ODT_S = ((value >> 8) & 0xF);
		//0x4 <= xx <= 0xE
		if (PHY1_TX_ODT_P < 0x4 || PHY1_TX_ODT_P > 0xE) {
			pr_info("   PCIe1 TX ODT P 0x4 <= PHY1_TX_ODT_P[0x%04x] <= 0xE error\r\n", (int)PHY1_TX_ODT_P);
		} else {
			pr_info("    PCIe1 TX ODT P Trim data range success 0x4 <= PHY1_TX_ODT_P[0x%04x] <= 0xE\r\n", (int)PHY1_TX_ODT_P);
		}

		//0x4 <= xx <= 0xE
		if (PHY1_TX_ODT_N < 0x7 || PHY1_TX_ODT_N > 0xE) {
			pr_info("   PCIe1 TX ODT N 0x4 <= PHY1_TX_ODT_N[0x%04x] <= 0xE error\r\n", (int)PHY1_TX_ODT_N);
		} else {
			pr_info("    PCIe1 TX ODT N Trim data range success 0x4 <= PHY1_TX_ODT_N[0x%04x] <= 0xE\r\n", (int)PHY1_TX_ODT_N);
		}
		//0x0 <= xx <= 0xF
		if (PHY1_TX_ODT_S < 0x0 || PHY1_TX_ODT_S > 0xF) {
			pr_info("   PCIe1 TX ODT S 0x0 <= PHY1_TX_ODT_S[0x%04x] <= 0xF error\r\n", (int)PHY1_TX_ODT_S);
		} else {
			pr_info("    PCIe1 TX ODT S Trim data range success 0x0 <= PHY1_TX_ODT_S[0x%04x] <= 0xF\r\n", (int)PHY1_TX_ODT_S);
		}

		core->phys[1].tx_odt_p = PHY1_TX_ODT_P;
		core->phys[1].tx_odt_n = PHY1_TX_ODT_N;
		core->phys[1].tx_odt_s = PHY1_TX_ODT_S;
	} else {
		pr_info("addr[%02x] error %d\r\n", (int)addr, (int)value);

		core->phys[1].tx_odt_p = 0x9;
		core->phys[1].tx_odt_n = 0xB;
		core->phys[1].tx_odt_s = 0x3;
	}

	return 0;
}

static int nvt_pcie_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node, *child;
	struct nvt_pcie_phy_core *core;
	struct phy_provider *provider;
	struct resource *res;
	unsigned cnt = 0;
	int ret;


	if (of_get_child_count(node) == 0) {
		dev_err(dev, "PHY no child node\n");
		return -ENODEV;
	}

	core = devm_kzalloc(dev, sizeof(*core), GFP_KERNEL);
	if (!core)
		return -ENOMEM;

	core->dev = dev;
	core->mode = NVT_PHY_ONE_x2;	// default x2 * 1

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	core->v_base[NVT_PHY_PCIE0] = devm_ioremap_resource(dev, res);
	if (IS_ERR(core->v_base[NVT_PHY_PCIE0]))
		return PTR_ERR(core->v_base[NVT_PHY_PCIE0]);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	core->v_base[NVT_PHY_PCIE1] = devm_ioremap_resource(dev, res);
	if (IS_ERR(core->v_base[NVT_PHY_PCIE1]))
		return PTR_ERR(core->v_base[NVT_PHY_PCIE1]);

	core->en_l1_detect = 1;

	mutex_init(&core->lock);

	for_each_available_child_of_node(node, child) {
		unsigned int id;
		struct nvt_pcie_phy *p;

		if (id >= MAX_NUM_PHYS) {
			dev_err(dev, "invalid PHY id: %u\n", id);
			ret = -EINVAL;
			goto put_child;
		}

		if (core->phys[id].phy) {
			dev_err(dev, "duplicated PHY id: %u\n", id);
			ret = -EINVAL;
			goto put_child;
		}

		p = &core->phys[id];
		p->phy = devm_phy_create(dev, child, &nvt_pcie_phy_ops);
		if (IS_ERR(p->phy)) {
			dev_err(dev, "failed to create PHY\n");
			ret = PTR_ERR(p->phy);
			goto put_child;
		}

		p->core = core;
		p->id = id;
		phy_set_drvdata(p->phy, p);
		cnt++;
	}

	dev_set_drvdata(dev, core);

	if (load_efuse_trim(core)) {
		goto put_child;
	}

	provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	if (IS_ERR(provider)) {
		dev_err(dev, "failed to register PHY provider\n");
		return PTR_ERR(provider);
	}

	dev_dbg(dev, "registered %u PCIe PHY(s)\n", cnt);

	dev_info(dev, "PCIe PHY probe done\r\n");

	return 0;
put_child:
	of_node_put(child);
	return ret;
}

static const struct of_device_id nvt_pcie_phy_match_table[] = {
	{ .compatible = "nvt,pcie-v2-phy" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, nvt_pcie_phy_match_table);

static struct platform_driver nvt_pcie_phy_driver = {
	.driver = {
		.name = "nvt-pcie-phy",
		.of_match_table = nvt_pcie_phy_match_table,
	},
	.probe = nvt_pcie_phy_probe,
};
module_platform_driver(nvt_pcie_phy_driver);

MODULE_DESCRIPTION("Novatek PCIe-v2 PHY driver");
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL v2");

