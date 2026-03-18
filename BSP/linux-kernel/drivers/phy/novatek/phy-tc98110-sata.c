/*
 * Copyright (C) 2022 Novatek Microelectronics Corp.
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

#define DRV_VERSION        "1.00.000"

struct nvt_sata_phy {
	void __iomem *top_va_base;
	void __iomem *phy_va_base;
	struct phy *phy;
};

static int debug_write = 0;
module_param(debug_write, int, 0444);
MODULE_PARM_DESC(debug_write, "debug write to the PHY");

static u32 nvt_sata_phy_read(void __iomem *phy_reg)
{
	u32 tmp = readl(phy_reg);
	return tmp;
}

static void nvt_sata_phy_write(u32 val, void __iomem *phy_reg)
{
	writel(val, phy_reg);

	if (debug_write) {
		printk("[DBG_W] val(0x%x), 0x%x(0x%x)\r\n", val, (u32)phy_reg, nvt_sata_phy_read(phy_reg));
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
		printk("[DBG_W] mask(0x%x), val(0x%x), 0x%x(0x%x)\r\n", mask, val, (u32)phy_reg, nvt_sata_phy_read(phy_reg));
	}
}

static int nvt_sata_phy_init(struct phy *phy)
{
	struct nvt_sata_phy *sata_phy = phy_get_drvdata(phy);
	void __iomem *cg_base = ioremap(NVT_CG_BASE_PHYS, 0x5000);

	//#MPLL Setting : USB3_PHY_CK
	//#Freqeuncy Config 500MHz
	//#page B
	nvt_sata_phy_write(0xb, (cg_base + 0x130));

	nvt_sata_phy_write(0x55, (cg_base + 0x4054));
	nvt_sata_phy_write(0x55, (cg_base + 0x4058));
	nvt_sata_phy_write(0x53, (cg_base + 0x405c));

	//#PHY internal clk en
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	nvt_sata_phy_masked_write(1, 0, 0x3, (sata_phy->phy_va_base + 0x25c));

	//##boundary clk en
	//#refclk pll enable
	nvt_sata_phy_masked_write(20, 20, 0x1, (cg_base + 0x0));

	//#RX_MPLL_CLK pll eanble
	nvt_sata_phy_masked_write(22, 22, 0x1, (cg_base + 0x0));

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

	//#to bank2
	nvt_sata_phy_write(0x02, (sata_phy->phy_va_base + 0x3fc));
	//#0x428[1] = 0 // cfg_rxanaifFSM_ON2WSS_via_pt_en
	nvt_sata_phy_masked_write(1, 1, 0x0, (sata_phy->phy_va_base + 0x428));

#if 0
	/* Test to extend comreset time */
	//#to bank0
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x3fc));
	//#0xc14[1:0] // 00: 10ms; 01:  15ms; 10: 20ms; 11: 25ms
	nvt_sata_phy_masked_write(1, 0, 0x3, (sata_phy->phy_va_base + 0xc14));
#endif

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
	//#bit[5:0] : u30p_lfps_clk_cnt[5:0]
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x9dc));

	//#bit3 : u30p_rxanaif_all_skp_recalib_en
	nvt_sata_phy_masked_write(3, 3, 0x0, (sata_phy->phy_va_base + 0x504));
	//#bit[3:0] : u30p_SATA_modeChg_ack_interval
	nvt_sata_phy_write(0x0f, (sata_phy->phy_va_base + 0x1e8));

#if 0
	/* Test to postpone CDR_EN enable timing */
	#if 0
	nvt_sata_phy_write(0x14, (sata_phy->phy_va_base + 0x518));
	#else
	nvt_sata_phy_write(0x0a, (sata_phy->phy_va_base + 0x518));
	#endif
	nvt_sata_phy_masked_write(1, 0, 0x2, (sata_phy->phy_va_base + 0x540));
#endif

	//#for auto LPM Partial
	nvt_sata_phy_write(0x33, (sata_phy->phy_va_base + 0x418));

	//### Vcontrol
	//#p_rmw 0xf0250004 17 17 1
	//#bit[4:0] : Vcontrol[4:0]
	nvt_sata_phy_masked_write(4, 0, 0x0, (sata_phy->top_va_base + 0x10));

	//#Reset ===============================================
	//#clock mux for usb30 controller
	//#p_rmw 0xec055a04 5 5 1
	//#ponrst for usb20phy
	//#mww 0xec070000 0x4a0
	//#mww 0xe0218000 0x4a0

	//#rstb for usb30phy
	nvt_sata_phy_masked_write(9, 9, 0x0, (cg_base + 0x98));
	nvt_sata_phy_masked_write(9, 9, 0x1, (cg_base + 0x98));

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

	//#Power mode change ready response time = 1 ms
	nvt_sata_phy_write(0x03, (sata_phy->phy_va_base + 0x450));
#if 0
	/* Test Power mode change ready response time = 4 ms */
	nvt_sata_phy_write(0x0f, (sata_phy->phy_va_base + 0x450));
#endif

	//#anaif_pc_rst=1
	nvt_sata_phy_write(0xf0, (sata_phy->phy_va_base + 0x408));

	msleep(1);

	//#anaif_pc_rst=0
	nvt_sata_phy_write(0x00, (sata_phy->phy_va_base + 0x408));

	return 0;
}

static const struct phy_ops nvt_sata_phy_ops = {
	.init		= nvt_sata_phy_init,
	.owner		= THIS_MODULE,
};

static int nvt_sata_phy_probe(struct platform_device *pdev)
{
	struct phy_provider *phy_provider;
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct phy *phy;
	struct nvt_sata_phy *sata_phy;

	sata_phy = devm_kzalloc(dev, sizeof(*sata_phy), GFP_KERNEL);
	if (!sata_phy)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -EINVAL;

	sata_phy->top_va_base = devm_ioremap(dev, res->start, resource_size(res));
	if (!sata_phy->top_va_base)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res)
		return -EINVAL;

	sata_phy->phy_va_base = devm_ioremap(dev, res->start, resource_size(res));
	if (!sata_phy->phy_va_base)
		return -ENOMEM;

	phy = devm_phy_create(dev, NULL, &nvt_sata_phy_ops);
	if (IS_ERR(phy)) {
		dev_err(dev, "failed to create PHY\n");
		return PTR_ERR(phy);
	}

	phy_set_drvdata(phy, sata_phy);

	phy_provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	if (IS_ERR(phy_provider)) {
		dev_err(dev, "failed to register PHY provider\n");
		return PTR_ERR(phy_provider);
	}

	dev_info(dev, "SATA PHY probe done\r\n");

	return 0;
}

static const struct of_device_id nvt_sata_phy_of_match[] = {
	{.compatible = "nvt,nvt-sata-phy",},
	{ },
};
MODULE_DEVICE_TABLE(of, nvt_sata_phy_of_match);

static struct platform_driver nvt_sata_phy_driver = {
	.probe	= nvt_sata_phy_probe,
	.driver = {
		.name	= "nvt-sata-phy",
		.of_match_table	= nvt_sata_phy_of_match,
	}
};
module_platform_driver(nvt_sata_phy_driver);

MODULE_DESCRIPTION("Novatek TC98110 SATA PHY driver");
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL v2");
