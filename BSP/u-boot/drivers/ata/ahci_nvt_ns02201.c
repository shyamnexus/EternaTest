/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Novatek Inc.
 */

#include "ahci_nvt.h"
#include <asm/arch/IOAddress.h>
#include <asm/arch/efuse_protected.h>
#include <linux/delay.h>

#define msleep(a) udelay(a * 1000)

//static int nvt_sata_tune_en = 0;
//static int rxclkinv;
//static int nvt_sata_dma_opt = 0;
//static int nvt_ahci_hflag = AHCI_HFLAG_YES_NCQ;// | AHCI_HFLAG_YES_FBS;// // 0x4000
//static int nvt_sata_ddrabt[4] = {[0 ...(3)] = 0x0};
static bool debug_write = false;

static u32 nvt_sata_phy_read(void __iomem *phy_reg)
{
	u32 tmp = readl(phy_reg);
	return tmp;
}

static void nvt_sata_phy_write(u32 val, void __iomem *phy_reg)
{
	writel(val, phy_reg);

	if (debug_write) {
		printk("[DBG_W] val(0x%x), 0x%llx(0x%x)\r\n", val, (u64)phy_reg, nvt_sata_phy_read(phy_reg));
	}
}

static void nvt_sata_phy_masked_write(u32 hi, u32 lo, u32 val, void __iomem *phy_reg)
{
	u32 mask, tmp;

	mask = GENMASK(hi, lo);
	tmp = readl(phy_reg);

	tmp &= ~(mask);
	tmp |= ((val << lo) & mask);

	writel(tmp, phy_reg);

	if (debug_write) {
		printk("[DBG_W] mask(0x%x), val(0x%x), 0x%llx(0x%x)\r\n", mask, val, (u64)phy_reg, nvt_sata_phy_read(phy_reg));
	}
}

static void nvt_sata_phy_trim(struct udevice *dev)
{
	struct ahci_nvt_plat_data *sata_phy = dev_get_priv(dev);
	u32		trim;
	u32		addr;
	int		code;
	BOOL	is_found;
	u32		internal_12k_trim = 0x8, tx_trim = 0x6, rx_trim = 0xc;
	u32		SATA1_Internal_12K_resistor;                          //0x00
	u32		SATA1_TX_TRIM, SATA1_RX_TRIM;                         //0x00
	u32		EFUSE_TRIM_DATA_SATA;

	if (sata_phy->idx == 1) {
		addr = 0x0;
		EFUSE_TRIM_DATA_SATA = EFUSE_TRIM_DATA_SATA1_PRI;
	} else if (sata_phy->idx == 2) {
		addr = 0x5;
		EFUSE_TRIM_DATA_SATA = EFUSE_TRIM_DATA_USB_SATA_PRI;
	} else {
		printf("Invalid SATA index\r\n");
		return;
	}

	code = otp_key_manager(EFUSE_TRIM_DATA_SATA);

	if (code == -33) {
		//!!!Please apply default value here!!!
		printf("Read SATA1 trim error\r\n");
		return;
	} else {
		is_found = extract_trim_valid(code, (u32 *)&trim);
		if (is_found) {
			// printf("  SATA1 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			printf("  SATA1 12K_resistor[4..0] valid from 1(0x1) <-> 20(0x14)\r\n");
			SATA1_Internal_12K_resistor = trim & 0x1F;
			SATA1_TX_TRIM = ((trim >> 5) & 0xF);
			SATA1_RX_TRIM = ((trim >> 9) & 0xF);

			//12K resistor 1<= x <= 20(0x14)
			if (SATA1_Internal_12K_resistor > 0x14) {
				printf("SATA1 12K_resistor Trim data error [0x%02x]=0x%04x > 20(0x14)\r\n", (int)addr, (int)SATA1_Internal_12K_resistor);
			} else if (SATA1_Internal_12K_resistor < 0x1) {
				printf("SATA1 12K_resistor Trim data error [0x%02x]=0x%04x < 1(0x1)\r\n", (int)addr, (int)SATA1_Internal_12K_resistor);
			} else {
				printf("  *SATA1 12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)SATA1_Internal_12K_resistor);
				internal_12k_trim = SATA1_Internal_12K_resistor;
			}

			//TX term bit[8..5] 0x2 <= x <= 0xE
			if (SATA1_TX_TRIM > 0xE) {
				printf("SATA1 TX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)SATA1_TX_TRIM);
			} else if (SATA1_TX_TRIM < 0x2) {
				printf("SATA1 TX Trim data error [0x%02x]=0x%04x < 0x2\r\n", (int)addr, (int)SATA1_TX_TRIM);
			} else {
				printf("  *SATA1 TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)SATA1_TX_TRIM);
				tx_trim = SATA1_TX_TRIM;
			}
			//RX term bit[12..9] 0x8 <= x <= 0xE
			if (SATA1_RX_TRIM > 0xE) {
				printf("SATA1 RX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)SATA1_RX_TRIM);
			} else if (SATA1_RX_TRIM < 0x8) {
				printf("SATA1 RX Trim data error [0x%02x]=0x%04x < 0x8\r\n", (int)addr, (int)SATA1_RX_TRIM);
			} else {
				printf("  *SATA1 RX Trim data range success 0x8 <= [0x%04x] <= 0xE\r\n", (int)SATA1_RX_TRIM);
				rx_trim = SATA1_RX_TRIM;
			}
		} else {
			//!!!Please apply default value here!!!
			printf("is found [%d][SATA1 trim] = 0x%08x\r\n", is_found, (int)code);
		}
	}

	//12k trim value
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_masked_write(5, 5, 1, (sata_phy->phy_va_base + 0x68c));
	nvt_sata_phy_masked_write(7, 7, 0, (sata_phy->phy_va_base + 0x684));
	nvt_sata_phy_masked_write(4, 0, internal_12k_trim, (sata_phy->phy_va_base + 0x680));

	//Tx trim value
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_masked_write(7, 4, tx_trim, (sata_phy->phy_va_base + 0x040));

	// RX trim value
	nvt_sata_phy_write(0x02, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_masked_write(3, 0, rx_trim, (sata_phy->phy_va_base + 0x000));

	msleep(1000);
}

static inline void nvt_sata100_phy_parameter_setting(struct udevice *dev)
{
	struct ahci_nvt_plat_data *sata_phy = dev_get_priv(dev);
	void __iomem *cg_base = (void __iomem *)IOADDR_CG_REG_BASE;

	//#MPLL Setting : USB3_PHY_CK
	//#Freqeuncy Config 500MHz
	//#page B
#if 1
	// nvt_sata_phy_write(0xb, (cg_base + 0x130));

	nvt_sata_phy_write(0x55, (cg_base + 0x4054));
	nvt_sata_phy_write(0x55, (cg_base + 0x4058));
	nvt_sata_phy_write(0x53, (cg_base + 0x405c));
#endif
	//#PHY internal clk en
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_masked_write(1, 0, 0x3, (sata_phy->phy_va_base + 0x25c));

	//#U3SATA_PHY_CLKSEL
	if (sata_phy->idx == 2) {
		nvt_sata_phy_masked_write(4, 4, 0x1, (cg_base + 0x24));
	}

	//##boundary clk en
	//#refclk pll enable
	nvt_sata_phy_masked_write(17, 17, 0x1, (cg_base + 0x0));
#if 1
	//#RX_MPLL_CLK pll eanble
	nvt_sata_phy_masked_write(25, 25, 0x1, (cg_base + 0x0));
#endif

	//#SATA DPHY setting =============================
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	//#bit[2:0] : u30p_PHYMODE
	nvt_sata_phy_write(0x09, (sata_phy->phy_va_base + 0x18c));

	//#bit0 : TurnoMode
	nvt_sata_phy_masked_write(0, 0, 0x0, (sata_phy->phy_va_base + 0xcc8));

	//#bit7 : DATABUS20_40 => 1: 20bits / 0: 40bits
	nvt_sata_phy_masked_write(7, 7, 0x1, (sata_phy->phy_va_base + 0xcc4));
	//#bit7 : HOST
	nvt_sata_phy_masked_write(0, 0, 0x1, (sata_phy->phy_va_base + 0xcc0));
	//#bit0 : cfg_byps_oob_r
	nvt_sata_phy_masked_write(0, 0, 0x0, (sata_phy->phy_va_base + 0xcf8));

	//#to bank0
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	//#bit7 : u30p_txelecidle_md
	//#bit6 : u30p_txdetectrx_loopback_md
	//#bit4 : u30p_powerdown_md
	//#bit3 : u30p_txelecidle
	//#bit2 : u30p_txdetectrx_loopback
	//#bit[1:0] : u30p_powerdown
	nvt_sata_phy_write(0xc0, (sata_phy->phy_va_base + 0x944));

	//#bit7 : u30p_rxtermination_md = 1
	//#bit4 : u30p_rxeqtraining_md = 0
	//#bit3 : u30p_rxtermination = 1
	nvt_sata_phy_write(0x88, (sata_phy->phy_va_base + 0x94c));

	//#LPM CDR
	//#bit0 : u30p_pll_ready_sync_en
	nvt_sata_phy_masked_write(0, 0, 0x1, (sata_phy->phy_va_base + 0x9f0));
	//#bit[3:0] : u30p_tx_sp_en_dly_cnt
	nvt_sata_phy_masked_write(3, 0, 0xf, (sata_phy->phy_va_base + 0x9cc));

	//#bit3 : u30p_rxanaif_all_skp_recalib_en
	nvt_sata_phy_masked_write(3, 3, 0x0, (sata_phy->phy_va_base + 0x504));
	//#bit[3:0] : u30p_SATA_modeChg_ack_interval
	nvt_sata_phy_write(0x0f, (sata_phy->phy_va_base + 0x1e8));

	//#for auto LPM Partial
	nvt_sata_phy_write(0x33, (sata_phy->phy_va_base + 0x418));

	//### Vcontrol
	//#p_rmw 0xf0250004 17 17 1
	//#bit[4:0] : Vcontrol[4:0]
	nvt_sata_phy_masked_write(4, 0, 0x0, (sata_phy->top_va_base + 0x10));

#if 1
	nvt_sata_phy_masked_write(0, 0, 0x0, (sata_phy->top_va_base + 0x10));
	nvt_sata_phy_masked_write(16, 16, 0x0, (sata_phy->top_va_base + 0x10));
	nvt_sata_phy_masked_write(16, 16, 0x0, (sata_phy->top_va_base + 0x14));
#endif

	/* New */
	//##postpne CDR_EN enable timing
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_write(0x0a, (sata_phy->phy_va_base + 0x518));
    nvt_sata_phy_masked_write(1, 0, 0x2, (sata_phy->phy_va_base + 0x540));

	/* New */
	nvt_sata_phy_masked_write(1, 1, 0x0, (sata_phy->phy_va_base + 0x428));

	//#Reset ===============================================
	//#clock mux for usb30 controller
	//#p_rmw 0xec055a04 5 5 1
	//#ponrst for usb20phy
	//#mww 0xec070000 0x4a0
	//#mww 0xe0218000 0x4a0

	if (sata_phy->idx == 1) {
		//#rstb for usb30phy
		nvt_sata_phy_masked_write(7, 7, 0x0, (cg_base + 0xa4));
		nvt_sata_phy_masked_write(7, 7, 0x1, (cg_base + 0xa4));
	} else if (sata_phy->idx == 2) {
		//#rstb for usb30phy
		nvt_sata_phy_masked_write(9, 9, 0x0, (cg_base + 0xa4));
		nvt_sata_phy_masked_write(9, 9, 0x1, (cg_base + 0xa4));
	}
	//#hi_rstb for usb30phy
	///nvt_sata_phy_masked_write(5, 5, 0x0, (cg_base + 0x9c));
	///nvt_sata_phy_masked_write(5, 5, 0x1, (cg_base + 0x9c));

	//#RESETn for SATAPHY
	///nvt_sata_phy_masked_write(8, 8, 0x1, (sata_phy->top_va_base + 0x04));
	///nvt_sata_phy_masked_write(24, 24, 0x0, (sata_phy->top_va_base + 0x04));
	///nvt_sata_phy_masked_write(24, 24, 0x1, (sata_phy->top_va_base + 0x04));

	//### disalbe ABJ ------
	//#to bank1
	nvt_sata_phy_write(0x01, (sata_phy->phy_va_base + 0x3fc));
	//#0xf0251168 bit5 : abj_en force mode ; bit4 : abj_en force value
	nvt_sata_phy_masked_write(5, 4, 0x2, (sata_phy->phy_va_base + 0x168));

	//#USB30 SATA PHY common setting =============================
	//#1. RX VBS clock source selection
	//#to bank1
	nvt_sata_phy_write(0x01, (sata_phy->phy_va_base + 0x3fc));
	//#bit4 : r_afref_clk_sel
	nvt_sata_phy_masked_write(4, 4, 0x1, (sata_phy->phy_va_base + 0x410));
	//#bit5 : r_avco_clk_sel
	nvt_sata_phy_masked_write(5, 5, 0x1, (sata_phy->phy_va_base + 0x410));
	//#back to bank0
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));

	//#2. extend TXPLL settle time
	//#bit[7:0] : u30p_rxanaif_vco_rst_pwidth
	nvt_sata_phy_write(0xff, (sata_phy->phy_va_base + 0x524));
	//#bit[7:0] : u30p_rxanaif_cdr_init_dly
	nvt_sata_phy_write(0xff, (sata_phy->phy_va_base + 0x514));
	//#to bank1
	nvt_sata_phy_write(0x01, (sata_phy->phy_va_base + 0x3fc));
	//#0xf02515a0 bit[3:0] : r_ofs_time_period
	nvt_sata_phy_masked_write(0, 0, 0x1, (sata_phy->phy_va_base + 0x5a0));
	nvt_sata_phy_masked_write(1, 1, 0x1, (sata_phy->phy_va_base + 0x5a0));
	nvt_sata_phy_masked_write(2, 2, 0x1, (sata_phy->phy_va_base + 0x5a0));
	nvt_sata_phy_masked_write(3, 3, 0x1, (sata_phy->phy_va_base + 0x5a0));
	//#back to bank0
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));

	//#to bank1
	nvt_sata_phy_write(0x01, (sata_phy->phy_va_base + 0x3fc));
	//#0xf0251420 bit2 : r_lms1_en
	nvt_sata_phy_masked_write(2, 2, 0x0, (sata_phy->phy_va_base + 0x420));
	//#back to bank0
	//#mww 0xf02513fc 0x00
	//#bit[4:0] : r_ba_cnt_ftime[4:0]
	nvt_sata_phy_write(0x5f, (sata_phy->phy_va_base + 0x680));
	//#bit[7:4] : r_ba_abj_time
	nvt_sata_phy_write(0xf4, (sata_phy->phy_va_base + 0x688));

	//#HDMI DPHY RX setting =============================
	//#to bank1
	nvt_sata_phy_write(0x01, (sata_phy->phy_va_base + 0x3fc));
	//#0xf0251578 bit[7:0] : r_aeq_timeout_period[7:0]
	nvt_sata_phy_write(0xff, (sata_phy->phy_va_base + 0x578));
	//#0xf0251088 bit2 : r_eq_c_freerun_en
	nvt_sata_phy_masked_write(2, 2, 0x0, (sata_phy->phy_va_base + 0x088));
	//#0xf02515b0 bit[3:0] : r_ofs_sa_eq_cft[3:0]
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x5b0));
	//#0xf02515c0 bit[3:0] : r_ofs_sa_eq_rct[3:0] ; bit[7:4] : r_ofs_sa_eq_rvga[3:0]
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x5c0));
	//#0xf02515d0 bit[1:0] : r_ofs_eq_eq_spsel0[1:0]
	//#           bit[3:2] : r_ofs_eq_eq_spsel1[1:0]
	//#           bit[5:4] : r_ofs_eq_eq_spsel2[1:0]
	nvt_sata_phy_write(0xb9, (sata_phy->phy_va_base + 0x5d0));
	//#0xf025108c bit[7:4] : r_aeq_cfine_man[3:0]
	//#           bit[3:0] : r_aeq_ccoarse_man[3:0]
	nvt_sata_phy_masked_write(7, 4, 0x2, (sata_phy->phy_va_base + 0x08c));
	nvt_sata_phy_masked_write(3, 0, 0x3, (sata_phy->phy_va_base + 0x08c));

	//#to bank1
	nvt_sata_phy_write(0x01, (sata_phy->phy_va_base + 0x3fc));
	//#0xf0251694 bit[5:4] : r_ki_fctl[1:0]
	//#           bit[2:0] : r_ki_frate[2:0]
	nvt_sata_phy_masked_write(5, 4, 0x1, (sata_phy->phy_va_base + 0x694));
	nvt_sata_phy_masked_write(2, 0, 0x4, (sata_phy->phy_va_base + 0x694));
	//#0xf0251168 bit3 : r_ki_clk_inv
	nvt_sata_phy_masked_write(3, 3, 0x1, (sata_phy->phy_va_base + 0x168));

	//#0xf02512e4 bit7 : r_eom_offset_swen
	//#           bit6 : r_ie_use_eom_offset
	//#           bit[5:0] : r_eom_offset_sw
	nvt_sata_phy_write(0xc0, (sata_phy->phy_va_base + 0x2e4));

	//#HDMI APHY RX setting =============================
	//#to bank2
	nvt_sata_phy_write(0x02, (sata_phy->phy_va_base + 0x3fc));
	//#0xf02510a0 bit[1:0] : BIAS_EN_CDR_CST
	//#           bit[3:2] : BIAS_CDR_SEL_DCO
	//#           bit[7:4] : BIAS_CDR_SEL_PI[3:0]
	nvt_sata_phy_write(0xf7, (sata_phy->phy_va_base + 0x0a0));
	//#0xf025102c bit[1:0] : EQ_RS_TUNE[1:0]
	nvt_sata_phy_masked_write(1, 0, 0x3, (sata_phy->phy_va_base + 0x02c));
	//#0xf025100c bit[2:0] : EQ_EOC_R_SW[2:0]
	nvt_sata_phy_masked_write(2, 0, 0x1, (sata_phy->phy_va_base + 0x00c));
	//#0xf0251008 bit7 : EQ_EOC_STEP
	nvt_sata_phy_masked_write(7, 7, 0x1, (sata_phy->phy_va_base + 0x008));
	//#0xf025100c bit[7:4] : EQ_EOC_VSEL[3:0]
	nvt_sata_phy_masked_write(7, 4, 0x2, (sata_phy->phy_va_base + 0x00c));
	//#0xf0251078 bit4 : DFE_SUM_GAIN
	//#           bit5 : DFE_EN_SEL1_H1_X2
	//#           bit6 : DFE_EN_SEL3_H1_X2
	nvt_sata_phy_masked_write(4, 4, 0x1, (sata_phy->phy_va_base + 0x078));
	nvt_sata_phy_masked_write(5, 5, 0x0, (sata_phy->phy_va_base + 0x078));
	nvt_sata_phy_masked_write(6, 6, 0x1, (sata_phy->phy_va_base + 0x078));
	//#0xf02510cc bit5 : SM_EQ_RVGA
	//#           bit7 : SM_EQ_SPSE
	nvt_sata_phy_masked_write(5, 5, 0x1, (sata_phy->phy_va_base + 0x0cc));
	nvt_sata_phy_masked_write(7, 7, 0x1, (sata_phy->phy_va_base + 0x0cc));
	//#0xf0251028 bit[7:4] : EQ_RVGA[3:0]
	nvt_sata_phy_masked_write(7, 4, 0x0, (sata_phy->phy_va_base + 0x028));
	//#0xf025102c bit[3:2] : EQ_SPSEL[1:0]
	nvt_sata_phy_masked_write(3, 2, 0x2, (sata_phy->phy_va_base + 0x02c));

	//#0xf0251024 bit[6:4] : EQ_RFB[2:0]
	nvt_sata_phy_masked_write(6, 4, 0x1, (sata_phy->phy_va_base + 0x024));
	//#0xf0251004 bit[3:0] : EQ_CFB[3:0]
	nvt_sata_phy_masked_write(3, 0, 0x0, (sata_phy->phy_va_base + 0x004));

	//#0xf025107c bit[6:4] : CDR_P_CTL[2:0]
	nvt_sata_phy_masked_write(6, 4, 0x7, (sata_phy->phy_va_base + 0x07c));

	//#SATA APHY Setting=============================
	//#TXPLL Gen3
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	//#bit3 : TXPLL_CLKMULT_CTL manual mode
	nvt_sata_phy_masked_write(3, 3, 0x1, (sata_phy->phy_va_base + 0x1a8));
	//#bit5 : TXPLL_DIV_PRE manual mode
	nvt_sata_phy_masked_write(5, 5, 0x1, (sata_phy->phy_va_base + 0x1a8));
	//#bit[7:5] : TXPLL_CLKMULT_CTL[2:0]
	nvt_sata_phy_masked_write(7, 5, 0x4, (sata_phy->phy_va_base + 0x194));
	//#bit[7:5] : TXPLL_DIV_PRE[2:0]
	nvt_sata_phy_masked_write(7, 5, 0x2, (sata_phy->phy_va_base + 0x198));

	//#SATA DPHY setting =============================
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	//#bit2:0 u30p_PHYMODE
	nvt_sata_phy_write(0x09, (sata_phy->phy_va_base + 0x18c));

	//#bit0 : TurnoMode
	nvt_sata_phy_masked_write(0, 0, 0x0, (sata_phy->phy_va_base + 0xcc8));
	//#bit7 : DATABUS20_40 => 1: 20bits / 0: 40bits
	nvt_sata_phy_masked_write(7, 7, 0x1, (sata_phy->phy_va_base + 0xcc4));
	//#bit7 : HOST
	nvt_sata_phy_masked_write(0, 0, 0x1, (sata_phy->phy_va_base + 0xcc0));
	//#bit0 : cfg_byps_oob_r
	nvt_sata_phy_masked_write(0, 0, 0x0, (sata_phy->phy_va_base + 0xcf8));

	//#Tx OOB waveform modify
	//#TX VCM stabling function for SATA TX OOB
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_masked_write(2, 2, 0x1, (sata_phy->phy_va_base + 0x160));

	//#reduce TX idle to OOB interval
	//#delay time = value *128us , default = 1ms
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_masked_write(3, 0, 0x1, (sata_phy->phy_va_base + 0x9ec));

	//#avoid LOS toggle issue
	//#LOS -> pt_en -> SATA DPHY RSTn_rx
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_masked_write(1, 1, 0x1, (sata_phy->phy_va_base + 0xd08));

	//# USB30 APHY setting =============================
	//#back to bank0
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	//#bit[5:4] = V_VCHSEL[1:0] = 11
	nvt_sata_phy_write(0x74, (sata_phy->phy_va_base + 0x158));
	//#bit[5:4] = TX_AMP_CTL_LFPS[1:0]
	nvt_sata_phy_write(0xf8, (sata_phy->phy_va_base + 0x048));

	//#1.TX_AMP_CTL=3, TX_DEC_EM_CTL=f
	//#*(volatile u32 *)(USB30_PHY_BASE + 0x50) = (0xfb);
	//#bit3 = TX_DE_EN
	//#mww 0xf0251050 0xfb
	nvt_sata_phy_write(0x8a, (sata_phy->phy_va_base + 0x050));

	//#nable software control
	//#TX_LFPS_AMP_CTL=0x1
	//#*(volatile u32 *)(USB30_PHY_BASE + 0xd0) = (0xff);
	nvt_sata_phy_write(0xfc, (sata_phy->phy_va_base + 0x0d0));

	//#Power mode change ready response time = 3 ms
	nvt_sata_phy_write(0x03, (sata_phy->phy_va_base + 0x450));

	nvt_sata_phy_trim(dev);

	//#anaif_pc_rst=1
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_write(0xf0, (sata_phy->phy_va_base + 0x408));

	msleep(1);

	//#anaif_pc_rst=0
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x408));
}

int nvtsata_platform_init(struct udevice *dev)
{
	struct ahci_nvt_plat_data *plat_data = dev_get_priv(dev);

	if (!plat_data->base) {
		pr_err("Error base addr\r\n");
		return -1;
	}

	if (plat_data->idx == 1) {
		writel(readl(IOADDR_CG_REG_BASE + 0x00) | (1 << 17),				IOADDR_CG_REG_BASE + 0x00);			//Enable SATA phy(pll17) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x00) | (1 << 25),				IOADDR_CG_REG_BASE + 0x00);			//Enable SATA phy(pll25) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x74) | (1 << 25),				IOADDR_CG_REG_BASE + 0x74);			//Enable SATA0 clk
	} else if (plat_data->idx == 2) {
		writel(readl(IOADDR_CG_REG_BASE + 0x00) | (1 << 17),				IOADDR_CG_REG_BASE + 0x00);			//Enable SATA phy(pll17) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x00) | (1 << 25),				IOADDR_CG_REG_BASE + 0x00);			//Enable SATA phy(pll25) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x74) | (1 << 26),				IOADDR_CG_REG_BASE + 0x74);			//Enable SATA1 clk
	} else {
		pr_err("Wrong base addr\r\n");
		return -1;
	}

	nvt_sata100_phy_parameter_setting(dev);

	writel(readl(plat_data->base + 0x00) | (1 << 27), plat_data->base + 0x00);
	writel(readl(plat_data->base + 0x0C) | 0x1, plat_data->base + 0x0C);
	writel(readl(plat_data->base + 0x100 + 0x18) | (1 << 22), plat_data->base + 0x100 + 0x18);
	writel(readl(plat_data->base + 0xa4) | (1 << 14), plat_data->base + 0xa4);

	return 0;
}
