// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023 Novatek
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <asm/io.h>
#include <asm/arch/efuse_protected.h>
#include <linux/bitops.h>

#define DRV_VERSION                     "1.00.01"

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

/* we have up to 8 PAXB based RC. The 9th one is always PAXC */
#define SR_NR_PCIE_PHYS               8

#define PCIE_PIPEMUX_CFG_OFFSET       0x10c
#define PCIE_PIPEMUX_SELECT_STRAP     GENMASK(3, 0)

#define CDRU_STRAP_DATA_LSW_OFFSET    0x5c
#define PCIE_PIPEMUX_SHIFT            19
#define PCIE_PIPEMUX_MASK             GENMASK(3, 0)

struct nvt_pcie_phy {
	struct nvt_pcie_phy_core *core;
	enum nvt_pcie_phy_id id;
//	struct phy *phy;

	u32 is_rint;	// 0: use rext, 1: use rint
	u32 rint;
	u32 tx_odt_p;
	u32 tx_odt_n;
	u32 tx_odt_s;
};

/**
 * struct nvt_pcie_phy_core - Novatek PCIe PHY core control
 *
 * @dev: pointer to device
 * @base: base register of PCIe SS
 * @cdru: CDRU base address
 * @pipemux: pipemuex strap
 */
struct nvt_pcie_phy_core {
	struct udevice *dev;
	void __iomem *v_base[2];

	enum nvt_pcie_phy_mode mode;
	struct nvt_pcie_phy phys[MAX_NUM_PHYS];
};

static void rc_apbm_srmw(void __iomem *addr, u32 data, u32 start, u32 width) {
	const u32 MASK = (1<<width) - 1;
	u32 data_tmp;

	data_tmp = readl(addr);
	data_tmp = (data_tmp & (~(MASK << start))) | ((data & MASK) << start);
	writel(data_tmp, addr);
}

static int pcie_phy_init(struct phy *phy)
{
	struct nvt_pcie_phy_core *core = dev_get_priv(phy->dev);
	int idx;

	debug("%s %lx\n", __func__, phy->id);
	if(core->mode == NVT_PHY_TWO_x1){
		debug("TWO x1\r\n");
	} else {
		debug("ONE x2\r\n");
	}

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
		rc_apbm_srmw(core->v_base[idx]+0x228, 0x1, 8, 3); // RX_LFEQ_RFB[2:0] = 3b001
		rc_apbm_srmw(core->v_base[idx]+0x228, 0x3, 12, 4); // RX_LFEQ_CFB[3:0] = 4b0011
		rc_apbm_srmw(core->v_base[idx]+0x228, 0x4, 16, 4); // RX_RLFEQ[3:0] = 4b0100
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

static int pcie_phy_xlate(struct phy *phy, struct ofnode_phandle_args *args)
{
	debug("%s %d\n", __func__, args->args[0]);
	if (args->args_count && args->args[0] < SR_NR_PCIE_PHYS)
		phy->id = args->args[0];
	else
		return -ENODEV;

	return 0;
}

static int pcie_phy_configure(struct phy *phy, void *params)
{
	struct nvt_pcie_phy_core *core = dev_get_priv(phy->dev);
	int phy_mode;

	if (params == NULL) {
		pr_err("%s: params is NULL\n", __func__);
		return -ENODEV;
	}

	phy_mode = *((int*)params);
	core->mode = phy_mode;

	return 0;
}

static const struct phy_ops pcie_phy_ops = {
	.of_xlate = pcie_phy_xlate,
	.init = pcie_phy_init,
	.configure = pcie_phy_configure,
};

static int load_efuse_trim(struct nvt_pcie_phy_core *core)
{
	u32 trim;
	u32 addr;
	int code;
	int is_found;

	if (core == NULL) {
		pr_err("core NULL\n");
		return -1;
	}

	// PCIe
	addr = 0x30;	// RINT
	code = otp_key_manager(addr);
	if (code == -33) {
		//!!!Please apply default value here!!!
		printf("Read PCIe RINT trim error\r\n");
		core->phys[NVT_PHY_PCIE0].is_rint = 0;
	} else {
		is_found = extract_trim_valid(code, (u32 *)&trim);
		if (is_found) {
			int is_valid;

			printf("PCIe RINT Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
			is_valid = (trim>>5)&0x01;
			if (is_valid) {
				core->phys[NVT_PHY_PCIE0].is_rint = 1;
				core->phys[NVT_PHY_PCIE0].rint = trim & 0x1F;
			} else {
				core->phys[NVT_PHY_PCIE0].is_rint = 0;
			}
		} else {
			//!!!Please apply default value here!!!
			pr_err("[690] PCIe Trim data raw(0x30) = NULL\n");
			printf("is found [%d][PCIe RINT] = 0x%08x\r\n", is_found, (int)code);
			core->phys[NVT_PHY_PCIE0].is_rint = 0;
		}
	}

	core->phys[NVT_PHY_PCIE0].id = NVT_PHY_PCIE0;
	addr = 0x31;	// lane0 ODT
	code = otp_key_manager(addr);
	if (code == -33) {
		//!!!Please apply default value here!!!
		printf("Read PCIe lane0 ODT trim error\r\n");
		core->phys[NVT_PHY_PCIE0].tx_odt_p = 0x9;
		core->phys[NVT_PHY_PCIE0].tx_odt_n = 0xB;
		core->phys[NVT_PHY_PCIE0].tx_odt_s = 0x3;
	} else {
		is_found = extract_trim_valid(code, (u32 *)&trim);
		if (is_found) {
			printf("PCIe lane0 ODT Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
			core->phys[NVT_PHY_PCIE0].tx_odt_p = (trim>>0)&0x0F;
			core->phys[NVT_PHY_PCIE0].tx_odt_n = (trim>>4)&0x0F;
			core->phys[NVT_PHY_PCIE0].tx_odt_s = (trim>>8)&0x0F;
		} else {
			pr_err("[690] PCIe Trim data raw(0x31) = NULL\n");
			printf("is found [%d][PCIe lane0 ODT] = 0x%08x\r\n", is_found, (int)code);
			core->phys[NVT_PHY_PCIE0].tx_odt_p = 0x9;
			core->phys[NVT_PHY_PCIE0].tx_odt_n = 0xB;
			core->phys[NVT_PHY_PCIE0].tx_odt_s = 0x3;
		}
	}

	core->phys[NVT_PHY_PCIE1].id = NVT_PHY_PCIE1;
	addr = 0x32;	// lane1 ODT
	code = otp_key_manager(addr);
	if (code == -33) {
		//!!!Please apply default value here!!!
		printf("Read PCIe lane1 ODT trim error\r\n");
		core->phys[NVT_PHY_PCIE1].tx_odt_p = 0x9;
		core->phys[NVT_PHY_PCIE1].tx_odt_n = 0xB;
		core->phys[NVT_PHY_PCIE1].tx_odt_s = 0x3;
	} else {
		is_found = extract_trim_valid(code, (u32 *)&trim);
		if (is_found) {
			printf("PCIe lane1 ODT Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
			core->phys[NVT_PHY_PCIE1].tx_odt_p = (trim>>0)&0x0F;
			core->phys[NVT_PHY_PCIE1].tx_odt_n = (trim>>4)&0x0F;
			core->phys[NVT_PHY_PCIE1].tx_odt_s = (trim>>8)&0x0F;
		} else {
			pr_err("[690] PCIe Trim data raw(0x32) = NULL\n");
			printf("is found [%d][PCIe lane1 ODT] = 0x%08x\r\n", is_found, (int)code);
			core->phys[NVT_PHY_PCIE1].tx_odt_p = 0x9;
			core->phys[NVT_PHY_PCIE1].tx_odt_n = 0xB;
			core->phys[NVT_PHY_PCIE1].tx_odt_s = 0x3;
		}
	}

	return 0;
};

static int nvt_pcie_phy_probe(struct udevice *dev)
{
	struct nvt_pcie_phy_core *core = dev_get_priv(dev);

	printf("%s: ver %s\r\n", __func__, DRV_VERSION);

	core->dev = dev;

	core->v_base[NVT_PHY_PCIE0] = (void __iomem *)devfdt_get_addr_index(dev, 0);
	if ((fdt_addr_t)core->v_base[NVT_PHY_PCIE0] == FDT_ADDR_T_NONE) {
		pr_err("PHY0 reg not found in dts\r\n");
		return -EINVAL;
	}
	core->v_base[NVT_PHY_PCIE1] = (void __iomem *)devfdt_get_addr_index(dev, 1);
	if ((fdt_addr_t)core->v_base[NVT_PHY_PCIE1] == FDT_ADDR_T_NONE) {
		pr_err("PHY1 reg not found in dts\r\n");
		return -EINVAL;
	}
	debug("ip base %p %p\n", core->v_base[NVT_PHY_PCIE0], core->v_base[NVT_PHY_PCIE1]);

	core->mode = NVT_PHY_ONE_x2;	// default x2 * 1
	load_efuse_trim(core);

	pr_info("Novatek PCIe PHY driver initialized\n");

	return 0;
}

static const struct udevice_id pcie_phy_match_table[] = {
	{ .compatible = "nvt,pcie-v2-phy" },
	{ }
};

U_BOOT_DRIVER(nvt_pcie_phy) = {
	.name = "nvt-pcie-phy",
	.id = UCLASS_PHY,
	.probe = nvt_pcie_phy_probe,
	.of_match = pcie_phy_match_table,
	.ops = &pcie_phy_ops,
	.plat_auto	= sizeof(struct nvt_pcie_phy_core),
	.priv_auto	= sizeof(struct nvt_pcie_phy_core),
};

