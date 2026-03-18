/*
 * AHCI SATA platform driver for NOVATECH IVOT SOCs
 *
 * Copyright (C) 2022 NOVATECH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/soc/nvt/fmem.h>
#include <linux/timekeeping.h>
#include "ahci_nvt_ivot_na51103.h"

//void __iomem *globalmem_temp;
static struct ahci_host_priv *nvt_hpriv[5];

static const struct ata_port_info ahci_port_info = {
	.flags		= AHCI_FLAG_COMMON,     /* AHCI_FLAG_COMMON | AHCI_HFLAG_IGN_IRQ_IF_ERR, */
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA6,
	.port_ops	= &ahci_pmp_retry_srst_ops,
};

static int ahci_nvt_hardreset(struct ata_link *link, unsigned int *class, unsigned long deadline);
static struct ata_port_operations ahci_nvt_ops = {
	.inherits				= &ahci_pmp_retry_srst_ops,
	.qc_prep				= nvt_ahci_qc_prep,
	.prereset				= nvt_ata_std_prereset,
	.hardreset 				= ahci_nvt_hardreset,
	.softreset				= nvt_ahci_pmp_retry_softreset,
	.pmp_softreset			= nvt_ahci_softreset,
	.pmp_attach				= nvt_ahci_pmp_attach,
	.pmp_detach				= nvt_ahci_pmp_detach,
#ifdef CONFIG_PM
	.port_resume			= nvt_ahci_port_resume,
#endif
	.port_start				= nvt_ahci_port_start,
	.error_handler      	= nvt_ahci_error_handler,
};

static const struct ata_port_info ahci_port_nvt_info = {
	.flags		= AHCI_FLAG_COMMON | ATA_FLAG_EM ,
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA6,
	.port_ops	= &ahci_nvt_ops,
};

static int nvt_sata_tune_en = 0;
module_param(nvt_sata_tune_en, int, 0444);
MODULE_PARM_DESC(nvt_sata_tune_en, "nvt sata tune enable(0=off [default], 1:on)");

static int nvt_sata_scope_gpio = 256;
module_param(nvt_sata_scope_gpio, int, 0444);
MODULE_PARM_DESC(nvt_sata_scope_gpio, "nvt sata scope gpio number (0~127 vaild)");

static int nvt_sata_scope_event = (PORT_IRQ_COLD_PRES | PORT_IRQ_PHYRDY | PORT_IRQ_CONNECT);
module_param(nvt_sata_scope_event, int, 0444);
MODULE_PARM_DESC(nvt_sata_scope_event, "nvt sata scope monitor event");

static int rxclkinv;
module_param_named(rxclkinv, rxclkinv, int, 0444);
MODULE_PARM_DESC(rxclkinv, "enable RX CLK inv (0=disable, 1=enable)");

static int nvt_skip_hba_reset = 0;
module_param_named(nvt_skip_hba_reset, nvt_skip_hba_reset, int, 0444);
MODULE_PARM_DESC(nvt_skip_hba_reset, "skip hba reset on 331B");

#ifdef CONFIG_AHCI_NVT_IVOT_FBS
static int nvt_ahci_hflag = AHCI_HFLAG_YES_NCQ | AHCI_HFLAG_YES_FBS;// // 0x4200
module_param_named(nvt_ahci_hflag, nvt_ahci_hflag, int, 0444);
MODULE_PARM_DESC(nvt_ahci_hflag, "nvt_ahci_hflag");
#else
static int nvt_ahci_hflag = AHCI_HFLAG_YES_NCQ;// | AHCI_HFLAG_YES_FBS;// // 0x4000
module_param_named(nvt_ahci_hflag, nvt_ahci_hflag, int, 0444);
MODULE_PARM_DESC(nvt_ahci_hflag, "nvt_ahci_hflag");
#endif


static int nvt_sata_dma_opt = 0;
module_param_named(nvt_sata_dma_opt, nvt_sata_dma_opt, int, 0444);
MODULE_PARM_DESC(nvt_sata_dma_opt, "nvt_sata_dma_opt");

static int nvt_sata_ddrabt[5] = {[0 ... (4)] = 0x0};
module_param_array(nvt_sata_ddrabt, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(nvt_sata_ddrabt, "nvt_sata_ddrabt array");

static int ahci_nvt_dev_hardreset_en = 0;
module_param(ahci_nvt_dev_hardreset_en, int, 0444);
MODULE_PARM_DESC(ahci_nvt_dev_hardreset_en, "ahci nvt device hwreset enable");

static int nvt_sata_en_bmap = 0;
module_param(nvt_sata_en_bmap, int, 0444);
MODULE_PARM_DESC(nvt_sata_en_bmap, "nvt sata enable bitmap");

static int nvt_sata_auto_detect_en = 1;
module_param(nvt_sata_auto_detect_en, int, 0444);
MODULE_PARM_DESC(nvt_sata_auto_detect_en, "nvt sata auto detect");

static int force_gen = 2;
module_param(force_gen, int, 0444);
MODULE_PARM_DESC(force_gen, "SATA speed limit (1=gen1, 2=gen2");

static int ssc;
static int pd_off;
static int force_powoff_bmap = 0;
static int phy_txffepost1 = 0x6;//R-0x820[3:0]
static int phy_txffepost2 = 0x0;//R-0x820[5:4]
static int phy_txffepre   = 0x0;//R-0x820[8:6]
static int phy_txampboost = 0x0;//R-0x820[10:9]
static int phy_txampredu  = 0xC;//R-0x820[15:11]
static int phy_txslewadj  = 0x7;//R-0x820[18:16]
static int phy_losrefgendecoderbinin = 0x3;//LOS_REFGEN_DECODER_BIN_IN[15:12]

module_param_named(ssc, ssc, int, 0444);
MODULE_PARM_DESC(ssc, "enable spread spectrum (0=disable<default>, 1=enable)");

module_param_named(pd_off, pd_off, int, 0444);
MODULE_PARM_DESC(pd_off, "pd_off (0: turn on pd<default>, 1=no pd)");

module_param(force_powoff_bmap, int, 0444);
MODULE_PARM_DESC(force_powoff_bmap, "this option can power off non-connected sata port bitmap after boot up. Set 0 to DISABLE");

module_param(phy_txffepost1, int, 0444);
MODULE_PARM_DESC(phy_txffepost1, "PHY REG 0x820 bit[3:0] default 0x4");

module_param(phy_txffepost2, int, 0444);
MODULE_PARM_DESC(phy_txffepost2, "PHY REG 0x820 bit[5:4] default 0x0");

module_param(phy_txffepre, int, 0444);
MODULE_PARM_DESC(phy_txffepre, "PHY REG 0x820 bit[8:6] default 0x0");

module_param(phy_txampboost, int, 0444);
MODULE_PARM_DESC(phy_txampboost, "PHY REG 0x820 bit[10:9] default 0x0");

module_param(phy_txampredu, int, 0444);
MODULE_PARM_DESC(phy_txampredu, "PHY REG 0x820 bit[15:11] default 0x8");

module_param(phy_txslewadj, int, 0444);
MODULE_PARM_DESC(phy_txslewadj, "PHY REG 0x820 bit[18:16] default 0x7");

module_param(phy_losrefgendecoderbinin, int, 0444);
MODULE_PARM_DESC(phy_losrefgendecoderbinin, "PHY REG 0x834 bit[15:12] default 0x4");

static int ki_mode = 1;
module_param(ki_mode, int, 0444);
MODULE_PARM_DESC(ki_mode, "Enable CDR reset mechanism; the negotiation timeout is 873.8 us");

static unsigned long ki_mode_rst_delay = 55000;
module_param(ki_mode_rst_delay, ulong, 0444);
MODULE_PARM_DESC(ki_mode_rst_delay, "Set the delay(ns) for CDR reset; the ALIGN timeout is 54.6 us)");

static unsigned long ki_mode_oob_delay = 10000;
module_param(ki_mode_oob_delay, ulong, 0444);
MODULE_PARM_DESC(ki_mode_oob_delay, "Set the delay(ns) for OOB");

static int ki_dump = 0;
module_param(ki_dump, int, 0444);
MODULE_PARM_DESC(ki_dump, "Enable ki dump");

static int aeq_man_en = 1;
module_param(aeq_man_en, int, 0444);
MODULE_PARM_DESC(aeq_man_en, "Enable manual AEQ");

static unsigned long aeq_en_delay = 5000;
module_param(aeq_en_delay, ulong, 0444);
MODULE_PARM_DESC(aeq_en_delay, "Set the delay(ns) for AEQ reset");

static unsigned long aeq_reset_delay = 5000;
module_param(aeq_reset_delay, ulong, 0444);
MODULE_PARM_DESC(aeq_reset_delay, "Set the delay(ns) for AEQ reset");

static unsigned long aeq_stable_delay = 1000000;
module_param(aeq_stable_delay, ulong, 0444);
MODULE_PARM_DESC(aeq_stable_delay, "Set the delay(ns) for AEQ stable");

static void nvt_sata_phy_write(u32 val, void __iomem *phy_reg)
{
	if (nvt_sata_tune_en != 0) {
		printk("write phyreg:0x%x val:0x%08x\r\n", fmem_lookup_pa((uintptr_t)phy_reg), val);
	}
	writel(val, phy_reg);
}

static u32 nvt_sata_phy_read(void __iomem *phy_reg)
{
	u32 val = 0;

	val = readl(phy_reg);
	if (nvt_sata_tune_en != 0) {
		printk("read phyreg:0x%x val:0x%08x\r\n", fmem_lookup_pa((uintptr_t)phy_reg), val);
	}
	return val;
}

static inline void nvt_sata100_phy_parameter_setting(
	struct platform_device *pdev, struct ahci_host_priv *hpriv)
{
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
	u32 val = 0;//, scontrol = 0;
	u16 sata_trim_data[2];
	s32 result = 0;

	if (rxclkinv) {
		//nvt_sata_phy_write(0x11, (plat_data->top_va_base + 0x30));
		val =  nvt_sata_phy_read((plat_data->top_va_base + 0x0));
		val |= (0x1 << 6);
		nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0));
	}
	nvt_sata_phy_read((plat_data->top_va_base + 0x30));

	/* axi ch dis*/
	val = nvt_sata_phy_read((plat_data->top_va_base + 0x40));
	val &= ~(0x1 << 16);
	nvt_sata_phy_write(val, (plat_data->top_va_base + 0x40));

	nvt_sata_phy_write(0x40000000, (plat_data->top_va_base + 0x1C));

	nvt_sata_phy_write(0x0, (plat_data->top_va_base + 0x18));			/*mode0 */
	//nvt_sata_phy_write(0x80004C26, (plat_data->top_va_base + 0x18));	/*mode1 */
	//nvt_sata_phy_write(0x40004C26,  (plat_data->top_va_base + 0x18));	/*mode2 */

	if (force_gen != 0) {
		val = nvt_sata_phy_read(hpriv->mmio + 0x100 + 0x2c);	//read Port SControl
		if ((val & (0xF << 4)) == 0) {
			dev_info(&pdev->dev, "%s: force_gen detected and set (=%d)\r\n", __func__, force_gen);
			val |= (force_gen << 4);
			nvt_sata_phy_write(val, hpriv->mmio + 0x100 + 0x2c);	//write speed limit to SControl
		}
	}

	if (nvt_sata_dma_opt) {
		nvt_sata_phy_write(0x100, (plat_data->top_va_base + 0x00));
	} else {
		nvt_sata_phy_write(0x0, (plat_data->top_va_base + 0x00));
	}


	nvt_sata_phy_write(nvt_sata_phy_read((plat_data->top_va_base + 0x20)) & ~(0xF << 20), (plat_data->top_va_base + 0x20));

	nvt_sata_phy_write(nvt_sata_phy_read((plat_data->top_va_base + 0x04)) & ~0x43, (plat_data->top_va_base + 0x04));
	msleep(5);

	// Release PHY_I2C_APB_RESETN
	nvt_sata_phy_write(nvt_sata_phy_read((plat_data->top_va_base + 0x04)) | 0x2, (plat_data->top_va_base + 0x04));
	msleep(5);
#if 0
	scontrol = readl((hpriv->mmio + 0x12C));

	if((scontrol & 0xF0) != 0x00) {
		nvt_sata_phy_write(0x04040000, (plat_data->phy_va_base + 0x850));
	}

	if((scontrol & 0xF0) == 0x10) {
		// RATE_SET_1P5G
		nvt_sata_phy_write(0x0006c7d0, (plat_data->phy_va_base + 0x0C0));
		nvt_sata_phy_write(0x01430000, (plat_data->phy_va_base + 0x81C));
		nvt_sata_phy_write(0x04040008, (plat_data->phy_va_base + 0x850));
	}

	if((scontrol & 0xF0) == 0x20) {
		// RATE_SET_3G
		nvt_sata_phy_write(0x0006c7d0, (plat_data->phy_va_base + 0x0C0));
		nvt_sata_phy_write(0x01420000, (plat_data->phy_va_base + 0x81C));
		nvt_sata_phy_write(0x04040004, (plat_data->phy_va_base + 0x850));
	}

	if((scontrol & 0xF0) == 0x30) {
		// RATE_SET_6G
		nvt_sata_phy_write(0x0006c7d0, (plat_data->phy_va_base + 0x0C0));
		nvt_sata_phy_write(0x01410000, (plat_data->phy_va_base + 0x81C));
		nvt_sata_phy_write(0x04040000, (plat_data->phy_va_base + 0x850));
	}
	msleep(1);

	if(ssc) {
		//printk("TURN ON SSC\r\n");
		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x406C));
		val &= ~(0xFF << 0);
		val |= (0x79 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x406C));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x40AC));
		val &= ~(0xFF << 0);
		val |= (0xC8 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40AC));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x40A8));
		val &= ~(0xFF << 0);
		val |= (0x0D << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40A8));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4098));
		val &= ~(0xFF << 0);
		val |= (0xC7 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4098));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4098));
		val &= ~(0xFF << 0);
		val |= (0xCF << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4098));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4098));
		val &= ~(0xFF << 0);
		val |= (0xC7 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4098));
	}

	nvt_sata_phy_write((0x06400000 + ((phy_txffepost1 & 0xF) << 0) +
	                  ((phy_txffepost2 & 0x3) << 4) + ((phy_txffepre & 0x7) << 6) +
	                  ((phy_txampboost & 0x3) << 9) + ((phy_txampredu & 0x1F) << 11) +
					  ((phy_txslewadj & 0x7) << 16)), (plat_data->phy_va_base + 0x820));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x834));
	val &= ~(0xF << 12);
	val |= (phy_losrefgendecoderbinin << 12);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x834));
#endif

#if IS_ENABLED(CONFIG_NVT_OTP_SUPPORT)
	result = efuse_readParamOps(EFUSE_SATA_TRIM_DATA, &sata_trim_data[0]);
#else
	result = -1;
#endif
	if (result == 0) {
		/* Apply efuse trim data */
		u32 BIAS_VLDOSEL_PI, BIAS_LDOSEL_PLL, BIAS_VREF_SEL, BIAS_RX_VLDO_SEL, TX_ZTX_CTL;
		BIAS_VLDOSEL_PI    = (sata_trim_data[0]) & 0xF;
		BIAS_RX_VLDO_SEL   = (sata_trim_data[0] >> 4) & 0xF;
		BIAS_LDOSEL_PLL    = (sata_trim_data[0] >> 8) & 0xF;
		BIAS_VREF_SEL      = (sata_trim_data[0] >> 12) & 0xF;
		TX_ZTX_CTL         = (sata_trim_data[1]) & 0xF;
		dev_dbg(&pdev->dev, "%s: [1][5]: sata_trim_data[0]=0x%08x, sata_trim_data[1]=0x%08x\r\n", __func__, (int)(sata_trim_data[0]), (int)(sata_trim_data[1]));
		dev_dbg(&pdev->dev, "BIAS_VLDOSEL_PI  = 0x%04x, apply to 0x1B8[3:0]\r\n", (int)BIAS_VLDOSEL_PI);
		dev_dbg(&pdev->dev, "BIAS_RX_VLDO_SEL = 0x%04x, apply to 0x174[3:0]\r\n", (int)BIAS_RX_VLDO_SEL);
		dev_dbg(&pdev->dev, "BIAS_LDOSEL_PLL  = 0x%04x, apply to 0x1B8[7:4]\r\n", (int)BIAS_LDOSEL_PLL);
		dev_dbg(&pdev->dev, "BIAS_VREF_SEL    = 0x%04x, apply to 0x1BC[3:0]\r\n", (int)BIAS_VREF_SEL);
		dev_dbg(&pdev->dev, "TX_ZTX_CTL       = 0x%04x, apply to 0x40[7:4]\r\n", (int)TX_ZTX_CTL);

		//0x1B8[3:0] BIAS_VLDOSEL_PI  = sata_trim_data[0]=>[3:0]
		//0x1B8[7:4] BIAS_LDOSEL_PLL  = sata_trim_data[0]=>[11:8]
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1B8));
		val &= ~(0xFF << 0);
		val |= (BIAS_VLDOSEL_PI << 0);
		val |= (BIAS_LDOSEL_PLL << 4);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1B8));
		//0x1BC[3:0] BIAS_VREF_SEL    = sata_trim_data[0]=>[15:12]
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1BC));
		val &= ~(0xF << 0);
		val |= (BIAS_VREF_SEL << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1BC));
		//0x174[3:0] BIAS_RX_VLDO_SEL = sata_trim_data[0]=>[7:4]
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x174));
		val &= ~(0xF << 0);
		val |= (BIAS_RX_VLDO_SEL << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x174));
		//TX_ZTX_CTL
		//SM: 0x00C0[7:4] = 4'b1111
		//SW: 0x0040[7:4] = sata_trim_data[1]=>[3:0]
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0xC0));
		val &= ~(0xF << 4);
		val |= (0xF << 4);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xC0));
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x40));
		val &= ~(0xF << 4);
		val |= (TX_ZTX_CTL << 4);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40));
	} else {
		/* Apply default */
		dev_dbg(&pdev->dev, "%s: [1][5]: sata_trim_data not found\r\n", __func__);

		//0x1B8[3:0] BIAS_VLDOSEL_PI  = 0xA
		//0x1B8[7:4] BIAS_LDOSEL_PLL  = 0xA
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1B8));
		val &= ~(0xFF << 0);
		val |= (0xAA << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1B8));
		//0x1BC[3:0] BIAS_VREF_SEL    = 0xA
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1BC));
		val &= ~(0xF << 0);
		val |= (0xA << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1BC));
		//0x174[3:0] BIAS_RX_VLDO_SEL = 0x4
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x174));
		val &= ~(0xF << 0);
		val |= (0x4 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x174));
		//TX_ZTX_CTL
		//SM: 0x00C0[7:4] = 4'b1111
		//SW: 0x0040[7:4] = 0x8
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0xC0));
		val &= ~(0xF << 4);
		val |= (0xF << 4);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xC0));
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x40));
		val &= ~(0xF << 4);
		val |= (0x8 << 4);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40));
	}

	// BEST_SETTING
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2004));
	val &= ~(0xFF << 0);
	val |= (0x30 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2004));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1E8));
	val &= ~(0xFF << 0);
	val |= (0x02 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1E8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x9F0));
	val &= ~(0x1 << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x9F0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x94C));
	val &= ~(0xFF << 0);
	val |= (0x88 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x94C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1CC));
	val &= ~(0xFF << 0);
	val |= (0x3F << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1CC));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x504));
	val &= ~(0xFF  << 0);
	val |= (0x08 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x504));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1000));
	val &= ~(0xFF  << 0);
	val |= (0x07 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1000));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x18C));
	val &= ~(0xFF  << 0);
	val |= (0x08 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x18C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x94));
	val &= ~(0xFF  << 0);
	val |= (0x07 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x94));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x8));
	val &= ~(0xFF  << 0);
	val |= (0x44 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1BC));
	val &= ~(0x1  << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1BC));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x3010));
	val &= ~(0x7  << 12);
	val |= (0x1 << 12);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x3010));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x5E4));
	val &= ~(0xFF  << 0);
	val |= (0x3F << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x5E4));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x5E8));
	val &= ~(0xFF  << 0);
	val |= (0x00 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x5E8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x3010));
	val &= ~(0xFF  << 0);
	val |= (0x22 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x3010));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x5C));
	val &= ~(0xF  << 4);
	val |= (0x2 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x5C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x3010));
	val &= ~(0xF  << 24);
	val |= (0x0 << 24);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x3010));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x64));
	val &= ~(0x3  << 5);
	val |= (0x0 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x64));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x3010));
	val &= ~(0xF  << 28);
	val |= (0x5 << 28);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x3010));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x68));
	val &= ~(0x3  << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x68));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x10D8));
	val &= ~(0xFF  << 0);
	val |= (0x58 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x10D8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x10DC));
	val &= ~(0x3  << 0);
	val |= (0x2 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x10DC));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x10D0));
	val &= ~(0xFF  << 0);
	val |= (0x2c << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x10D0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x10D4));
	val &= ~(0x3  << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x10D4));


	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x24C));
	val &= ~(0x1  << 6);
	val |= (0x1 << 6);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x24C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x208));
	val &= ~(0x3  << 2);
	val |= (0x0 << 2);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x208));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x240));
	val &= ~(0x3  << 0);
	val |= (0x0 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x240));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x244));
	val &= ~(0x3  << 0);
	val |= (0x0 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x244));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x248));
	val &= ~(0x3  << 0);
	val |= (0x0 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x248));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x250));
	val &= ~(0xF  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x250));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x240));
	val &= ~(0xF  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x240));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x244));
	val &= ~(0xF  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x244));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x248));
	val &= ~(0xF  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x248));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4C));
	val &= ~(0x1  << 2);
	val |= (0x0 << 2);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x24C));
	val &= ~(0x1  << 7);
	val |= (0x1 << 7);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x24C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x240));
	val &= ~(0x1  << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x240));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x244));
	val &= ~(0x1  << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x244));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x248));
	val &= ~(0x1  << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x248));
/*
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0xCC));
	val &= ~(0x1  << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xCC));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4C));
	val &= ~(0x1  << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4C));
*/
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x208));
	val &= ~(0x7  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x208));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x160));
	val &= ~(0x1  << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x160));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x250));
	val &= ~(0xF  << 0);
	val |= (0xF << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x250));

	/* TX_amp_DEC_EM_CTRL*/
	/* #manual on */
	nvt_sata_phy_write(0xFF, (plat_data->phy_va_base + 0x00d0));

	/* #for gen3 tx cts */
	nvt_sata_phy_write(0x88, (plat_data->phy_va_base + 0x0050));

	/* Acclerate the speed of AEQ convergence */
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x05CC));
	val &= ~(0xFF  << 0);
	val |= (0x04 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x05CC));

	/* Suppress SATA pll bandwidth */
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1130));
	val &= ~(0xFF  << 0);
	val |= (0x35 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1130));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1134));
	val &= ~(0xFF  << 0);
	val |= (0x06 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1134));

	/* Decrease TXPLL lock timer */
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x0450));
	val &= ~(0xFF  << 0);
	val |= (0x01 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x0450));

	/* RX VBS delay timer */
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x0534));
	val &= ~(0xFF  << 0);
	val |= (0x0a << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x0534));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x055C));
	val &= ~(0xFF  << 0);
	val |= (0x02 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x055C));

	/* TXELCIDLE timer */
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x09ec));
	val &= ~(0xFF  << 0);
	val |= (0x51 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x09ec));

	/* avoid LOS toggle issue */
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2214));
	val &= ~(0x1 << 17);
	val |= (0x1 << 17);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2214));

#if 0
	if (!pd_off) {
		// MANUAL_TURN_OFF_PD
		//printk("MANUAL_TURN_OFF_PD\r\n");
		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x854));
		val |= ((0x1 << 29));
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x854));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x818));
		val &= ~(0x7 << 11);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x818));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4084));
		val &= ~((0x7 << 0) | (0x7 << 4));
		val |= ((0x2 << 0) | (0x3 << 4));
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4084));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x404C));
		val &= ~( (0x7 << 4));
		val |= ((0x1 << 4));
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x404C));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x928));
		val |= ((0x1 << 1) | (0x1 << 5) | (0x1 << 9));
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x928));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x908));
		val |= ((0x1 << 1) | (0x1 << 5) | (0x1 << 9));
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x908));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x924));
		val |= ((0x1 << 2));
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x924));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x904));
		val |= ((0x1 << 2));
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x904));
	}
#endif

	// Release PORn
	nvt_sata_phy_write((nvt_sata_phy_read(plat_data->top_va_base + 0x04) | 0x41), (plat_data->top_va_base + 0x04));

	msleep(10);
	dev_info(&pdev->dev, "%s: done\r\n", __func__);
}

static int ahci_nvt_autodetect(struct ahci_host_priv *hpriv)
{
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
	int timeout_cnt = 20;

	if (!nvt_sata_auto_detect_en) {
		return -1;
	}

	do {
		if (((u32)readl((void __iomem *) (hpriv->mmio + 0x128)) & 0xF) != 0x0) {
			break;
		}
		msleep(10);
	} while(timeout_cnt--);

	if(((u32)readl((void __iomem *) (hpriv->mmio + 0x128)) & 0xF) == 0x0) {
		//hotplug detect
		//printk("PD with OOB\r\n");
		nvt_sata_phy_write(0x3f, (plat_data->phy_va_base + 0x24c));
		nvt_sata_phy_write(0x03, (plat_data->phy_va_base + 0x240));
		nvt_sata_phy_write(0x03, (plat_data->phy_va_base + 0x244));
		nvt_sata_phy_write(0x03, (plat_data->phy_va_base + 0x248));

		nvt_sata_phy_write(0xff, (plat_data->phy_va_base + 0x0bc));
		nvt_sata_phy_write(0x03, (plat_data->phy_va_base + 0x0c0));
		nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x040));
		nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x03c));

		nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x098));
		nvt_sata_phy_write(0x04, (plat_data->phy_va_base + 0x018));

		nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x0a8));
		nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x028));

		nvt_sata_phy_write(0x90, (plat_data->phy_va_base + 0x1a4));

		nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x0d4));
		nvt_sata_phy_write(0x24, (plat_data->phy_va_base + 0x054));

		nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0xc4));
		nvt_sata_phy_write(0x10, (plat_data->phy_va_base + 0x44));

		nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0xc8));
		nvt_sata_phy_write(0x10, (plat_data->phy_va_base + 0x48));

		nvt_sata_phy_write(0x07, (plat_data->phy_va_base + 0xc4));
		nvt_sata_phy_write(0x10, (plat_data->phy_va_base + 0x44));

		nvt_sata_phy_write(0x03, (plat_data->phy_va_base + 0xc8));
		nvt_sata_phy_write(0x10, (plat_data->phy_va_base + 0x48));

		nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0xac));
		nvt_sata_phy_write(0x0a, (plat_data->phy_va_base + 0x2c));

		nvt_sata_phy_write(0x04, (plat_data->phy_va_base + 0xcc));
		nvt_sata_phy_write(0x10, (plat_data->phy_va_base + 0x4c));

		nvt_sata_phy_write(0x05, (plat_data->phy_va_base + 0xcc));
		nvt_sata_phy_write(0x10, (plat_data->phy_va_base + 0x4c));

		nvt_sata_phy_write(0x15, (plat_data->phy_va_base + 0xcc));
		nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x4c));

		nvt_sata_phy_write(0x35, (plat_data->phy_va_base + 0xcc));
		nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x4c));

		nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x1cc));
		nvt_sata_phy_write(0x11, (plat_data->phy_va_base + 0x080));
		nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x000));

		nvt_sata_phy_write(0x31, (plat_data->phy_va_base + 0x080));
		nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x000));

		nvt_sata_phy_write(0x0f, (plat_data->phy_va_base + 0x174));
		nvt_sata_phy_write(0x1f, (plat_data->phy_va_base + 0x174));

		nvt_sata_phy_write(0x31, (plat_data->phy_va_base + 0x080));
		nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x000));
	} else {
	}
	return 0;
}

static int ahci_nvt_poweron(struct ahci_host_priv *hpriv)
{
#if 0
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
	u32 val;

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x928));
	val &= ~((0xFFF << 1) | (0x7 << 16));
	val |= ((0x1 << 1) | (0x1 << 5) | (0x1 << 9));
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x928));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x908));
	val &= ~((0xFFF << 1) | (0x7 << 16));
	val |= ((0x1 << 1) | (0x1 << 5) | (0x1 << 9));
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x908));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x924));
	val &= ~((0xFF << 1));
	val |= ((0x1 << 2));
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x924));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x904));
	val &= ~((0xFF << 1));
	val |= ((0x1 << 2));
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x904));

	force_powoff_bmap &= ~(0x1 << (plat_data->sata_id - 1));
#endif

	return 0;
}

static int ahci_nvt_poweroff(struct ahci_host_priv *hpriv)// struct platform_device *pdev,
{
#if 0
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
	int timeout_cnt = 20;
	u32 val;

	do {
		if (((u32)readl((void __iomem *) (hpriv->mmio + 0x128)) & 0xF) != 0x0) {
			break;
		}
		msleep(10);
	} while(timeout_cnt--);


	if(((u32)readl((void __iomem *) (hpriv->mmio + 0x128)) & 0xF) == 0x0) {
		if (force_powoff_bmap & (0x1 << (plat_data->sata_id - 1))) {
			printk("detect no devive force power off port: 0x%d\n",  plat_data->sata_id);
			val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x928));
			val |= ((0xFFF << 1) | (0x7 << 16));
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x928));

			val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x908));
			val &= ~((0xFFF << 1) | (0x7 << 16));
			val |= ((0x9FF << 1) | (0x7 << 16));
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x908));

			val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x924));
			val |= ((0xFF << 1));
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x924));

			val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x904));
			val &= ~((0xFF << 1));
			val |= ((0x3F << 1));
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x904));
		}
	}
#endif
	return 0;
}

static int nvt_sata_poweron_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "currect force poweroff bitmap 0x%x \n", force_powoff_bmap);
    return 0;
}

static int nvt_sata_poweron_open(struct inode *inode, struct file *file) {
	return single_open(file, nvt_sata_poweron_show, PDE_DATA(inode));
}

static ssize_t nvt_sata_poweron_write(struct file *file, const char __user *buffer,
                                                        size_t count, loff_t *ppos)
{
    unsigned int value_in;
    char value_str[32] = {'\0'};
	int i;

    if(copy_from_user(value_str, buffer, count))
        return -EFAULT;

    value_str[count] = '\0';
    sscanf(value_str, "%x\n", &value_in);
	printk("\n set force poweroff bitmap:0x%X  before force poweroff bitmap:0x%X\n", value_in, force_powoff_bmap);

	if ((value_in & 0xFF) == (force_powoff_bmap & 0xFF)) {
		return -EINVAL;
	}

	for (i = 0; i < 5; i++) {
		if ((force_powoff_bmap & (0x1 << i)) !=  (value_in & (0x1 << i))) {
			if ((value_in & (0x1 << i)) == 0x0) {
				ahci_nvt_poweron(nvt_hpriv[i]);
			}
		}
	}

    return count;
}

static struct proc_ops nvt_sata_poweron_ops = {
	.proc_open    = nvt_sata_poweron_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_sata_poweron_write
};


static struct proc_dir_entry *nvt_sata_proc_root	= NULL;
static struct proc_dir_entry *nvt_sata_proc_poweron   = NULL;

int nvt_sata_proc_init(void)
{
	const char name[16] = "nvt_sata";
	struct proc_dir_entry *root;
	int ret = 0;

	root = proc_mkdir(name, NULL);
	 if (!root) {
	 	ret = -ENOMEM;
		goto end;
	}

	nvt_sata_proc_root = root;

	nvt_sata_proc_poweron = proc_create("nvt_sata_poweron", S_IRUGO|S_IXUGO, nvt_sata_proc_root, &nvt_sata_poweron_ops);
	if (!nvt_sata_proc_poweron) {
		printk("create proc node 'nvt_sata_poweron' failed!\n");
		ret = -EINVAL;
		goto err1;
	}

	return ret;

err1:
	proc_remove(nvt_sata_proc_root);

end:
	return ret;

}
EXPORT_SYMBOL(nvt_sata_proc_init);

void nvt_sata_proc_exit(void)
{

	//if (nvt_sata_proc_poweron) {
	//	remove_proc_entry(nvt_sata_proc_poweron->name, nvt_sata_proc_root);
	//}

	if (nvt_sata_proc_root) {
		proc_remove(nvt_sata_proc_root);
		nvt_sata_proc_root = NULL;
	}
}
EXPORT_SYMBOL(nvt_sata_proc_exit);

static void nvt_sata100_dump_reg(struct ata_port *ap, unsigned int tag)
{
    struct platform_device *pdev = to_platform_device(ap->host->dev);
	struct ahci_host_priv *hpriv = ap->host->private_data;
	void __iomem *mmio = hpriv->mmio;
	struct ahci_port_priv *pp = ap->private_data;
	struct ahci_sg *sata100_sg;
	u32 *pfis, i, rec_n_sg;

	printk(KERN_WARNING "SATA%u: dump register contents\n", pdev->id);
	printk("   CAP(0x00)  = %#x\n", readl(mmio+0x00));
	printk("   CAP(0x04)  = %#x\n", readl(mmio+0x04));
	printk("    IS(0x08)  = %#x\n", readl(mmio+0x08));
	printk("    PI(0x0C)  = %#x\n", readl(mmio+0x0c));
	printk("TCOUNT(0xA0)  = %#x\n", readl(mmio+0xa0));
	printk("   FEA(0xA4)  = %#x\n", readl(mmio+0xa4));
	printk(" P0CLB(0x100) = %#x\n", readl(mmio+0x100));
	printk("  P0FB(0x108) = %#x\n", readl(mmio+0x108));
	printk("  P0IS(0x110) = %#x\n", readl(mmio+0x110));
	printk("  P0IE(0x114) = %#x\n", readl(mmio+0x114));
	printk(" P0CMD(0x118) = %#x\n", readl(mmio+0x118));
	printk(" P0TFD(0x120) = %#x\n", readl(mmio+0x120));
	printk("P0SSTS(0x128) = %#x\n", readl(mmio+0x128));
	printk("P0SCTL(0x12C) = %#x\n", readl(mmio+0x12c));
	printk("P0SERR(0x130) = %#x\n", readl(mmio+0x130));
	printk("P0SACT(0x134) = %#x\n", readl(mmio+0x134));
	printk("  P0CI(0x138) = %#x\n", readl(mmio+0x138));
	printk(" P0FCR(0x170) = %#x\n", readl(mmio+0x170));
	printk("P0BIST(0x174) = %#x\n", readl(mmio+0x174));
	printk(" P0FSM(0x17C) = %#x\n", readl(mmio+0x17c));

	printk(KERN_WARNING "ata%u: dump command header %d\n", ap->print_id, tag);
	if (tag !=0 ) {
		printk("DW0 = %#x\n", pp->cmd_slot[tag].opts);
		printk("DW1 = %#x\n", pp->cmd_slot[tag].status);
		printk("DW2 = %#x\n", pp->cmd_slot[tag].tbl_addr);
		printk("DW3 = %#x\n", pp->cmd_slot[tag].tbl_addr_hi);
		rec_n_sg = (pp->cmd_slot[tag].opts >> 16) & 0xFFFF;
	}
	else {
		printk("DW0 = %#x\n", pp->cmd_slot[0].opts);
		printk("DW1 = %#x\n", pp->cmd_slot[0].status);
		printk("DW2 = %#x\n", pp->cmd_slot[0].tbl_addr);
		printk("DW3 = %#x\n", pp->cmd_slot[0].tbl_addr_hi);
		rec_n_sg = (pp->cmd_slot[0].opts >> 16) & 0xFFFF;
	}
	printk("\n");

	printk(KERN_WARNING "ata%u: dump command table 0\n", ap->print_id);
	pfis = (u32 *) pp->cmd_tbl;
	printk("DW0 = %#x\n", *(pfis+0));
	printk("DW1 = %#x\n", *(pfis+1));
	printk("DW2 = %#x\n", *(pfis+2));
	printk("DW3 = %#x\n", *(pfis+3));
	printk("DW4 = %#x\n", *(pfis+4));
	printk("\n");

	printk(KERN_WARNING "ata%u: dump PRD table\n", ap->print_id);
	sata100_sg = pp->cmd_tbl + AHCI_CMD_TBL_HDR_SZ;
	for (i=0; i<rec_n_sg; i++) {
		printk("-- item %d --\n", i);
		printk("DW0 = %#x\n", sata100_sg->addr);
		printk("DW1 = %#x\n", sata100_sg->addr_hi);
		printk("DW3 = %#x\n", sata100_sg->flags_size);
		sata100_sg++;
	}
	printk("\n");

}

static void ahci_nvt_cg_reset(struct ata_link *link);

const struct ata_port_info *ahci_nvt_get_port_info(
		struct platform_device *pdev, struct ahci_host_priv *hpriv)
{
	struct device *dev = &pdev->dev;
	struct ahci_nvt_plat_data *plat_data;
    struct resource *mem;
	int val = 0xffff;
	//void __iomem *apb_va;
	char buf[16];
	//void __iomem *mmio = hpriv->mmio;
	static int first = 1;
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	int err = 0;
#endif

	if (pdev->id < 1 || pdev->id > 5) {
		return &ahci_port_info;
	}

	nvt_hpriv[(pdev->id - 1)] = hpriv;
    hpriv->flags |= nvt_ahci_hflag;//AHCI_HFLAG_YES_NCQ;// | AHCI_HFLAG_YES_FBS

	plat_data = devm_kzalloc(dev, sizeof(*plat_data), GFP_KERNEL);
	if (!plat_data)
		return &ahci_port_info;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (!mem) {
        return &ahci_port_info;
    }

	plat_data->top_va_base= devm_ioremap_resource(dev, mem);
	if (IS_ERR(plat_data->top_va_base))
		return &ahci_port_info;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    if (!mem) {
        return &ahci_port_info;
    }

	plat_data->phy_va_base= devm_ioremap_resource(dev, mem);
	if (IS_ERR(plat_data->phy_va_base))
		return &ahci_port_info;

	/* alloc gpio resource*/
	of_property_read_s32(dev->of_node,"gpio-id", &val);
	if (val != 0xffff) {
		plat_data->gpio_num = val;
	}

#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	plat_data->use_fbs = 1;		//use_fbs should follow global fbs setting by default
	err = of_property_read_s32(dev->of_node,"use-fbs", &val);
	if (!err) {
		plat_data->use_fbs = val;
		if (val) {
			dev_info(dev, "use-fbs is set 1, enable FBS\n");
		} else {
			dev_info(dev, "use-fbs is set 0, disable FBS\n");
		}
	} else {
		dev_info(dev, "use-fbs is not set, enable FBS by default\n");
	}
#endif

	//sprintf(buf, "gpio_sata%d\n", plat_data->gpio_num);
	//gpio_request(plat_data->gpio_num, buf);
	//gpio_direction_output(plat_data->gpio_num, 1);

	plat_data->dump_reg = &nvt_sata100_dump_reg;
	plat_data->cg_reset = &ahci_nvt_cg_reset;
	//plat_data->logmem = globalmem_temp;
	//apb_va = ioremap_nocache((u32)0xfc800000, PAGE_ALIGN(0x1000));
	//writel( 0xfc000000, apb_va);
	//iounmap(apb_va);
	plat_data->sata_id = pdev->id;
	plat_data->ddrabt = nvt_sata_ddrabt[plat_data->sata_id - 1];
	nvt_sata_en_bmap |= (0x1 << (pdev->id - 1));
	plat_data->id_bitmap = &nvt_sata_en_bmap;
	plat_data->sata_tune_en = &nvt_sata_tune_en;
	plat_data->nvt_sata_scope_gpio = &nvt_sata_scope_gpio;
	plat_data->nvt_sata_scope_event = &nvt_sata_scope_event;

	if ((nvt_sata_tune_en != 0) && (nvt_sata_scope_gpio < 0x80)) {
		printk("plat: bmap:0x%x en:0x%x io:0x%x event:0x%x\r\n", (u32) *plat_data->id_bitmap, (u32) *plat_data->sata_tune_en, (u32) *plat_data->nvt_sata_scope_gpio, (u32) *plat_data->nvt_sata_scope_event);
		if (first) {
			int ret;

			ret = gpio_request(nvt_sata_scope_gpio, buf);
			if (ret) {
				printk("failed to gpio_request nvt_sata_scope_gpio\n");
			} else  {
				gpio_direction_output(nvt_sata_scope_gpio, 1);
			}

			first = 0;
		}
	}

	hpriv->em_loc = 0;
	hpriv->em_buf_sz = 4;
	hpriv->em_msg_type = EM_MSG_TYPE_LED;

	hpriv->plat_data = plat_data;

	nvt_sata100_phy_parameter_setting(pdev, hpriv);

	dev_info(dev, "AHCI NVT GPIO LED control is enabled.\n");
	return &ahci_port_nvt_info;
}
EXPORT_SYMBOL(ahci_nvt_get_port_info);

void nvt_sata100_clk_enable(void)
{
    return;
}
EXPORT_SYMBOL(nvt_sata100_clk_enable);

int ahci_nvt_sata100_init(struct platform_device *pdev)
{
	//struct device *dev = &pdev->dev;
	//struct resource *mem;
	struct clk				*clk;
    static int print_msg = 1;
	int 					err;
	//int val;

    if (print_msg) {
		printk(DRV_NAME " driver version " DRV_VERSION "\r\n");
		print_msg = 0;
	}

	clk = clk_get(&pdev->dev, dev_name(&pdev->dev));//"2f0b80000.sata"
	if (!IS_ERR(clk)) {
		err = clk_prepare_enable(clk);
		if (err < 0) {
			dev_err(&pdev->dev, "clk_prepare_enable failed\n");
			return -ENODEV;
		}
	} else {
		dev_err(&pdev->dev, "%s not found, clk_get failed\n", dev_name(&pdev->dev));
		return -ENODEV;
	}

    return 0;
}
EXPORT_SYMBOL(ahci_nvt_sata100_init);

int ahci_nvt_sata100_post_init(struct platform_device *pdev, struct ahci_host_priv *hpriv)
{
	int val = 0;

	val = readl(hpriv->mmio + 0x16C);
	val &= ~(0x3 << 19);
	if (nvt_sata_dma_opt) {
		val |= (0x3 << 19);
	}
	else {
		val |= (0x2 << 19);
	}

	val |= (0x1 << 30);
	writel( val, (hpriv->mmio + 0x16C));

	writel(300000, (hpriv->mmio + 0xA0));

	if (nvt_get_chip_ver(NVT_PERIPHERAL_PHYS_BASE) == CHIPVER_B) {

		val = readl(hpriv->mmio + 0x17C);
		val &= ~(0xBF << 0);
		val |= (0xBF << 0);
		writel(val, (hpriv->mmio + 0x17C));

		nvt_skip_hba_reset = 1;
	}

	val = readl(hpriv->mmio + 0x174);
	val &= ~(0x1 << 31);
	val |= (0x1 << 31);
	writel( val, (hpriv->mmio + 0x174));

	return 0;
}
EXPORT_SYMBOL(ahci_nvt_sata100_post_init);

int ahci_nvt_sata100_exit(struct platform_device *pdev)
{
	//struct ahci_host_priv *hpriv = nvt_hpriv[pdev->id];
	//struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;

    return 0;
}
EXPORT_SYMBOL(ahci_nvt_sata100_exit);

#ifdef CONFIG_AHCI_NVT_IVOT_FBS
static int ahci_nvt_reset(struct ata_link *link)
{
	struct ata_port *ap = link->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	u32 reg_16C;

#define reg_num 0x150
#define reg_sz reg_num >> 2
	int i, reg_val[reg_sz] = {0};

	/* SATA read ori register(0x0~0x140, 0x16C) */
	for (i = 0; i < reg_sz; i++) {
		reg_val[i] = readl(hpriv->mmio + i*4);
	}
	reg_16C = readl(hpriv->mmio + 0x16C);

	for(i = 0; i < 32; i++) {
		ata_link_dbg(link, "old> %03x: %x %x %x %x\r\n", i * 16, readl(hpriv->mmio + i * 16), readl(hpriv->mmio + i * 16 + 4), readl(hpriv->mmio + i * 16 + 8), readl(hpriv->mmio + i * 16 + 12));
	}

	/* HBA reset */
	ahci_reset_controller(ap->host);

	/* SATA write ori register(0x0~0x140, 0x16C, 0x17C) */
	for (i = 0; i < reg_sz; i++) {
		if(i == (0x114 >> 2) || (i == (0x110 >> 2))) {
			continue;
		}

		writel(reg_val[i], hpriv->mmio + i*4);
		readl(hpriv->mmio + i*4);  /* flush */
	}
	writel(reg_16C | (0x1 << 30), hpriv->mmio + 0x16C);
	/* verB should not be here, there is no reason to refill */
	//writel((0xBF << 0), hpriv->mmio + 0x17C);

	for(i = 0; i < 32; i++) {
		ata_link_dbg(link, "new> %03x: %x %x %x %x\r\n", i * 16, readl(hpriv->mmio + i * 16), readl(hpriv->mmio + i * 16 + 4), readl(hpriv->mmio + i * 16 + 8), readl(hpriv->mmio + i * 16 + 12));
	}

	msleep(100);

	return 0;
}
#endif

static void ahci_nvt_cg_reset(struct ata_link *link)
{
	struct ata_port *ap = link->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	u32 reg_16C;
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
	struct platform_device *pdev = to_platform_device(ap->host->dev);
	u32 val;
	struct clk *clk;
	int err;

#define reg_num 0x150
#define reg_sz reg_num >> 2
	int i, reg_val[reg_sz] = {0};
#define top_reg_num 0x44
#define top_reg_sz top_reg_num >> 2
	int top_reg_val[top_reg_sz] = {0};

	/* SATA read ori register(0x0~0x140, 0x16C) */
	for (i = 0; i < reg_sz; i++) {
		reg_val[i] = readl(hpriv->mmio + i*4);
	}
	reg_16C = readl(hpriv->mmio + 0x16C);

	for(i = 0; i < 32; i++) {
		ata_link_dbg(link, "old> %03x: %x %x %x %x\r\n", i * 16, readl(hpriv->mmio + i * 16), readl(hpriv->mmio + i * 16 + 4), readl(hpriv->mmio + i * 16 + 8), readl(hpriv->mmio + i * 16 + 12));
	}

	/* TOP read ori register(0x0~0x44) */
	for (i = 0; i < top_reg_sz; i++) {
		top_reg_val[i] = readl(plat_data->top_va_base + i*4);
	}

	for(i = 0; i < 5; i++) {
		ata_link_dbg(link, "old> %03x: %x %x %x %x\r\n", i * 16, readl(plat_data->top_va_base + i * 16), readl(plat_data->top_va_base + i * 16 + 4), readl(plat_data->top_va_base + i * 16 + 8), readl(plat_data->top_va_base + i * 16 + 12));
	}

	/* select eco option */
	val = readl(hpriv->mmio + 0x174);
	val &= ~(0x1 << 31);
	writel(val, hpriv->mmio + 0x174);

	/* AXI flush */
	val = readl(hpriv->mmio + 0x17C);
	writel(val | (0x1 << 8), hpriv->mmio + 0x17C);

	/* Polling AXI flush valid */
	while (1) {
		val = readl(hpriv->mmio + 0x17C);
		if (val & (0x1 << 8)) {
			break;
		} else {
			ata_link_err(link, "opt> 17c: %x, AXI flush check failed...\r\n", readl(hpriv->mmio + 0x17C));
		}
	}

	ata_link_dbg(link, "opt> 17c: %x\r\n", readl(hpriv->mmio + 0x17C));

	/* CG reset */
	clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (!IS_ERR(clk)) {
		clk_disable_unprepare(clk);
		err = clk_prepare_enable(clk);
		if (err < 0) {
			dev_err(&pdev->dev, "clk_prepare_enable failed\n");
			return;
		}
	} else {
		dev_err(&pdev->dev, "%s not found, clk_get failed\n", dev_name(&pdev->dev));
		return;
	}

	/* SATA write ori register(0x0~0x140, 0x16C, 0x17C) */
	for (i = 0; i < reg_sz; i++) {
		if(i == (0x114 >> 2) || (i == (0x110 >> 2))) {
			continue;
		}

		writel(reg_val[i], hpriv->mmio + i*4);
		readl(hpriv->mmio + i*4);  /* flush */
	}
	writel(reg_16C | (0x1 << 30), hpriv->mmio + 0x16C);
	if (nvt_get_chip_ver(NVT_PERIPHERAL_PHYS_BASE) == CHIPVER_B) {
		writel((0xBF << 0), hpriv->mmio + 0x17C);
	}

	for(i = 0; i < 32; i++) {
		ata_link_dbg(link, "new> %03x: %x %x %x %x\r\n", i * 16, readl(hpriv->mmio + i * 16), readl(hpriv->mmio + i * 16 + 4), readl(hpriv->mmio + i * 16 + 8), readl(hpriv->mmio + i * 16 + 12));
	}

	/* TOP write ori register(0x0~0x44) */
	for (i = 0; i < top_reg_sz; i++) {
		writel(top_reg_val[i], plat_data->top_va_base + i*4);
		readl(plat_data->top_va_base + i*4);  /* flush */
	}

	for(i = 0; i < 5; i++) {
		ata_link_dbg(link, "new> %03x: %x %x %x %x\r\n", i * 16, readl(plat_data->top_va_base + i * 16), readl(plat_data->top_va_base + i * 16 + 4), readl(plat_data->top_va_base + i * 16 + 8), readl(plat_data->top_va_base + i * 16 + 12));
	}

	msleep(100);

	nvt_sata100_phy_parameter_setting(pdev, hpriv);
}

/************************************
 *    Override for customization    *
 ************************************/
static int ahci_nvt_hardreset(struct ata_link *link, unsigned int *class,
				unsigned long deadline)
{
	static const unsigned long timing[] = { 5, 100, 300};
	struct ata_port *ap = link->ap;
	struct ahci_port_priv *pp = ap->private_data;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct platform_device *pdev = to_platform_device(ap->host->dev);
	u8 *d2h_fis = pp->rx_fis + RX_FIS_D2H_REG;
	struct ata_taskfile tf;
	bool online;
	u32 sstatus, scontrol;
	int rc;
	int retry = 50;
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
	//u32 val;
	u32 val_fsm, val_evt;
	plat_data->escape_occurred = false;

	/* select internal FSM */
	val_fsm = readl(hpriv->mmio + 0x174);
	val_fsm &= ~(0x1 << 31);
	val_fsm |= (0x1 << 31);
	writel(val_fsm, hpriv->mmio + 0x174);

	/* check SYNC escape */
	val_evt = readl(hpriv->mmio + 0x15C);
	if ((val_evt & 0xff) == 0x45) {
		plat_data->escape_occurred = true;
	}

	hpriv->stop_engine(ap);

#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	/* HBA reset to clear HW flag (caused by CRC error) */
	if (!nvt_skip_hba_reset && plat_data->use_fbs)
		ahci_nvt_reset(link);
#endif

	/* clear D2H reception area to properly wait for D2H FIS */
	ata_tf_init(link->device, &tf);
	tf.command = ATA_BUSY;
	ata_tf_to_fis(&tf, 0, 0, d2h_fis);

	do {
		if (plat_data->skip_one == 0) {
			plat_data->skip_one = 1;
			plat_data->ata_hrst = ahci_nvt_dev_hardreset_en;
		} else {
			if (ata_is_host_link(link)) {
				nvt_sata100_phy_parameter_setting(pdev, hpriv);
			}
		}

		ahci_nvt_autodetect(hpriv);
		ahci_nvt_poweroff(hpriv);

		rc = nvt_sata_link_hardreset(link, timing, deadline, &online, NULL);

		if (sata_scr_read(link, SCR_STATUS, &sstatus))
			break;

		if ((sstatus & 0x3) == 0x0)
			break;

		if ((sstatus & 0x3) == 0x1)
			continue;

		if (sata_scr_read(link, SCR_CONTROL, &scontrol))
			break;

		/* connection establishment */
		//if (((sstatus & 0x3) == 0x3) && (((scontrol & 0x0f0) == 0x0) || ((scontrol & 0x0f0) == (sstatus & 0x0f0)))) {
		if ((sstatus & 0x3) == 0x3) {
			if(!pd_off) {
				#if 0
				val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x928));
				val |= ((0x1 << 3) | (0x1 << 7) | (0x1 << 4));
				nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x928));

				val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x908));
				val |= ((0x1 << 3) | (0x1 << 7) | (0x1 << 4));
				nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x908));
				#endif
			}
			break;
		}
	} while (retry--);

	if (plat_data->cg_reset && plat_data->escape_occurred == true && plat_data->ifs_occurred == true) {
		/* delay 100ms to ensure d2h coming */
		msleep(100);

		/* select internal FSM */
		val_fsm = readl(hpriv->mmio + 0x174);
		val_fsm &= ~(0x1 << 31);
		val_fsm |= (0x1 << 31);
		writel(val_fsm, hpriv->mmio + 0x174);

		/* check FSM */
		val_fsm = readl(hpriv->mmio + 0x17C);

		if ((val_fsm & 0x5000c00) == 0x5000c00) {
			ata_link_err(link, "[AXI_PATCH] trigger cg reset at hardreset, [0x17c(%x), 0x15c(%x), 0x118(%x)]\n", val_fsm, readl(hpriv->mmio + 0x15C), readl(hpriv->mmio + 0x118));
			plat_data->cg_reset(link);

			/* retrigger oob */
			retry = 50;

			ata_tf_init(link->device, &tf);
			tf.command = ATA_BUSY;
			ata_tf_to_fis(&tf, 0, 0, d2h_fis);

			do {
				if (plat_data->skip_one == 0) {
					plat_data->skip_one = 1;
					plat_data->ata_hrst = ahci_nvt_dev_hardreset_en;
				} else {
					if (ata_is_host_link(link)) {
						nvt_sata100_phy_parameter_setting(pdev, hpriv);
					}
				}

				ahci_nvt_autodetect(hpriv);
				ahci_nvt_poweroff(hpriv);

				rc = nvt_sata_link_hardreset(link, timing, deadline, &online, NULL);

				if (sata_scr_read(link, SCR_STATUS, &sstatus))
					break;

				if ((sstatus & 0x3) == 0x0)
					break;

				if ((sstatus & 0x3) == 0x1)
					continue;

				if (sata_scr_read(link, SCR_CONTROL, &scontrol))
					break;

				/* connection establishment */
				//if (((sstatus & 0x3) == 0x3) && (((scontrol & 0x0f0) == 0x0) || ((scontrol & 0x0f0) == (sstatus & 0x0f0)))) {
				if ((sstatus & 0x3) == 0x3) {
					if(!pd_off) {
						#if 0
						val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x928));
						val |= ((0x1 << 3) | (0x1 << 7) | (0x1 << 4));
						nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x928));

						val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x908));
						val |= ((0x1 << 3) | (0x1 << 7) | (0x1 << 4));
						nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x908));
						#endif
					}
					break;
				}
			} while (retry--);
		}
	}

#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	if (plat_data->ifs_occurred == true) {
		if ((((0x1 << (plat_data->sata_id - 1)) & plat_data->fbsen_map) > 0)) {
			if (ap->ops->pmp_detach) {
				ap->ops->pmp_detach(ap);
			}
		}
	}
#endif

	hpriv->start_engine(ap);
	if (nvt_sata_tune_en != 0) {
		printk("sata%d oob sw count %d\r\n", pdev->id, (50 - retry));
	}

	if (online) {
		*class = ahci_dev_classify(ap);
	}

	DPRINTK("EXIT, rc=%d, class=%u\n", rc, *class);
	return rc;
}

int nvt_sata_link_hardreset(struct ata_link *link, const unsigned long *timing,
			    unsigned long deadline,
			    bool *online, int (*check_ready)(struct ata_link *))
{
	u32 scontrol;
	int rc;

	DPRINTK("ENTER\n");

	if (online)
		*online = false;

	if (nvt_sata_set_spd_needed(link)) {
		/* SATA spec says nothing about how to reconfigure
		 * spd.  To be on the safe side, turn off phy during
		 * reconfiguration.  This works for at least ICH7 AHCI
		 * and Sil3124.
		 */
		if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
			goto out;

		scontrol = (scontrol & 0x0f0) | 0x304;

		if ((rc = sata_scr_write(link, SCR_CONTROL, scontrol)))
			goto out;

		sata_set_spd(link);
	}

	/* issue phy wake/reset */
	if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
		goto out;

	scontrol = (scontrol & 0x0f0) | 0x301;

	if ((rc = sata_scr_write_flush(link, SCR_CONTROL, scontrol)))
		goto out;

	/* Couldn't find anything in SATA I/II specs, but AHCI-1.1
	 * 10.4.2 says at least 1 ms.
	 */
	ata_msleep(link->ap, 1);

	/* bring link back */
	rc = nvt_sata_link_resume(link, timing, deadline);
	if (rc)
		goto out;
	/* if link is offline nothing more to do */
	if (nvt_ata_phys_link_offline(link))
		goto out;

	/* Link is online.  From this point, -ENODEV too is an error. */
	if (online)
		*online = true;

	if (sata_pmp_supported(link->ap) && ata_is_host_link(link)) {
		/* If PMP is supported, we have to do follow-up SRST.
		 * Some PMPs don't send D2H Reg FIS after hardreset if
		 * the first port is empty.  Wait only for
		 * ATA_TMOUT_PMP_SRST_WAIT.
		 */
		if (check_ready) {
			unsigned long pmp_deadline;

			pmp_deadline = ata_deadline(jiffies,
						    ATA_TMOUT_PMP_SRST_WAIT);
			if (time_after(pmp_deadline, deadline))
				pmp_deadline = deadline;
			nvt_ata_wait_ready(link, pmp_deadline, check_ready);
		}
		rc = -EAGAIN;
		goto out;
	}

	rc = 0;
	if (check_ready)
		rc = nvt_ata_wait_ready(link, deadline, check_ready);
out:
	if (rc && rc != -EAGAIN) {
		/* online is set iff link is online && reset succeeded */
		if (online)
			*online = false;
		ata_link_err(link, "COMRESET failed (errno=%d)\n", rc);
	}
	DPRINTK("EXIT, rc=%d\n", rc);
	return rc;
}

#define KI_MEM_SIZE  1000
u32 ki_mem[2][KI_MEM_SIZE] = {{0}, {0}};
u64 wake_t = 0;

static DEFINE_SPINLOCK(link_resume_lock);
int nvt_sata_link_resume(struct ata_link *link, const unsigned long *params,
		     unsigned long deadline)
{
	int tries = ATA_LINK_RESUME_TRIES;
	u32 scontrol, serror;
	int rc;
	unsigned long flags;

	if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
		return rc;

	/*
	 * Writes to SControl sometimes get ignored under certain
	 * controllers (ata_piix SIDPR).  Make sure DET actually is
	 * cleared.
	 */
	do {
		if (ki_mode && ata_is_host_link(link)) {
			/* Lock for atomic */
			spin_lock_irqsave(&link_resume_lock, flags);
		}

		scontrol = (scontrol & 0x0f0) | 0x300;

		if ((rc = sata_scr_write(link, SCR_CONTROL, scontrol))) {
			if (ki_mode && ata_is_host_link(link)) {
				/* Unlock */
				spin_unlock_irqrestore(&link_resume_lock, flags);
			}
			return rc;
		}

		/*
		 * Some PHYs react badly if SStatus is pounded
		 * immediately after resuming.  Delay 200ms before
		 * debouncing.
		 */
		if (ki_mode) {
			struct ata_port *ap = link->ap;
			struct ahci_host_priv *hpriv = ap->host->private_data;
			struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
			int ki_idx = 0;
			bool aeq_en_done = false;
			bool aeq_reset_done = false;
			bool aeq_rel_done = false;
			u32 val = 0;
			u32 round = 0;
			u32 sstatus;
			u64 curr_t = 0;
			u64 rc_t = 0;
			u32 cal_count = 0;

			if (ata_is_host_link(link)) {
				/* The delay for OOB */
				curr_t = ktime_get_real_ns();
				rc_t = curr_t;
				writel(0x1 << 18, (hpriv->mmio + 0x130));
				while ((ktime_get_real_ns() - curr_t) < ki_mode_oob_delay) {
					if ((ktime_get_real_ns() - rc_t) >= 1000){
						rc_t = ktime_get_real_ns();
						val = readl(hpriv->mmio + 0x130);

						if (val & (0x1 << 18)) {
							wake_t = ktime_get_real_ns() - curr_t;
							break;
						}
					}
				}

				do {
					/* The delay for CDR reset */
					curr_t = ktime_get_real_ns();
					while ((ktime_get_real_ns() - curr_t) < ki_mode_rst_delay) {

						if (aeq_man_en) {
							u64 aeq_t = ktime_get_real_ns() - curr_t;

							if (aeq_t <= aeq_reset_delay && !aeq_reset_done) {
								/* AEQ reset */
								val = readl((plat_data->phy_va_base + 0x05C0));
								val &= ~(0x1 << 1);
								val |= (0x1 << 1);
								writel(val,(plat_data->phy_va_base + 0x05C0));
								aeq_reset_done = true;
							}

							if (aeq_t >= aeq_reset_delay && !aeq_rel_done) {
								/* AEQ reset release */
								val = readl((plat_data->phy_va_base + 0x05C0));
								val &= ~(0x1 << 1);
								writel(val,(plat_data->phy_va_base + 0x05C0));
								aeq_rel_done = true;

								if (round == 0) {
									if (!aeq_en_done) {
										/* manual AEQ enable */
										val = readl((plat_data->phy_va_base + 0x05C0));
										val &= ~(0x1 << 0);
										val |= (0x1 << 0);
										writel(val,(plat_data->phy_va_base + 0x05C0));
										aeq_en_done = true;
									}
								}
							}
						}

						val = readl(plat_data->phy_va_base + 0x5b4);
						if (val) {
							if (ki_idx < KI_MEM_SIZE) {
								/* Record ki variation */
								if (ki_idx == 0 || (val != ki_mem[0][ki_idx - 1])) {
									ki_mem[0][ki_idx] = val;
									ki_mem[1][ki_idx] = round;
									ki_idx++;
								}
							}
						}
						cal_count++;
					}

					if (aeq_man_en) {
						aeq_reset_done = false;
						aeq_rel_done = false;
					}

					round++;

					if (sata_scr_read(link, SCR_STATUS, &sstatus))
						break;

					/* Check link */
					if ((sstatus & 0x103) == 0x103)
						break;

					/* CDR reset */
					val = readl((plat_data->phy_va_base + 0xA8));
					val &= ~(0x1 << 1);
					val |= (0x1 << 1);
					writel(val,(plat_data->phy_va_base + 0xA8));
					val = readl((plat_data->phy_va_base + 0x28));
					val &= ~(0x1 << 1);
					val |= (0x1 << 1);
					writel(val,(plat_data->phy_va_base + 0x28));
					val = readl((plat_data->phy_va_base + 0x28));
					val &= ~(0x1 << 1);
					val |= (0x0 << 1);
					writel(val,(plat_data->phy_va_base + 0x28));
					val = readl((plat_data->phy_va_base + 0xA8));
					val &= ~(0x1 << 1);
					val |= (0x0 << 1);
					writel(val,(plat_data->phy_va_base + 0xA8));


				} while (round < 15);
			}

			if (aeq_man_en) {
				/* Delay 1ms for resistor(R) stable */
				curr_t = ktime_get_real_ns();
				while ((ktime_get_real_ns() - curr_t) < aeq_stable_delay);

				/* Disable manual AEQ */
				val = readl((plat_data->phy_va_base + 0x05C0));
				val &= ~(0x1 << 0);
				writel(val,(plat_data->phy_va_base + 0x05C0));
			}

			if (ata_is_host_link(link)) {
				/* Unlock */
				spin_unlock_irqrestore(&link_resume_lock, flags);
			}

			if (ki_dump) {
				int j = 0;
				for (j = 0; j < min(ki_idx, KI_MEM_SIZE); j++) {
					ata_link_err(link, "[KI_DUMP] sata phy 5b4[%d]: 0x%08x, round: %u\r\n", j, ki_mem[0][j], ki_mem[1][j]);
				}
				ata_link_err(link, "[KI_DUMP] cal_count: %u, oob_delay: %llu , rst_delay: %lu\r\n", cal_count, wake_t, ki_mode_rst_delay);
			}
		}

		if (!(link->flags & ATA_LFLAG_NO_DB_DELAY))
			ata_msleep(link->ap, 200);

		/* is SControl restored correctly? */
		if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
			return rc;

	} while ((scontrol & 0xf0f) != 0x300 && --tries);

	if ((scontrol & 0xf0f) != 0x300) {
		ata_link_warn(link, "failed to resume link (SControl %X)\n",
			     scontrol);
		return 0;
	}

	if (tries < ATA_LINK_RESUME_TRIES)
		ata_link_warn(link, "link resume succeeded after %d retries\n",
			      ATA_LINK_RESUME_TRIES - tries);

	if ((rc = sata_link_debounce(link, params, deadline)))
		return rc;

	/* clear SError, some PHYs require this even for SRST to work */
	if (!(rc = sata_scr_read(link, SCR_ERROR, &serror)))
		rc = sata_scr_write(link, SCR_ERROR, serror);

	return rc != -EINVAL ? rc : 0;
}

int nvt_ata_std_prereset(struct ata_link *link, unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	struct ata_eh_context *ehc = &link->eh_context;
	const unsigned long *timing = sata_ehc_deb_timing(ehc);
	int rc;

	/* if we're about to do hardreset, nothing more to do */
	if (ehc->i.action & ATA_EH_HARDRESET)
		return 0;

	/* if SATA, resume link */
	if (ap->flags & ATA_FLAG_SATA) {
		rc = nvt_sata_link_resume(link, timing, deadline);
		/* whine about phy resume failure but proceed */
		if (rc && rc != -EOPNOTSUPP)
			ata_link_warn(link,
				      "failed to resume link for reset (errno=%d)\n",
				      rc);
	}

	/* no point in trying softreset on offline link */
	if (nvt_ata_phys_link_offline(link))
		ehc->i.action &= ~ATA_EH_SOFTRESET;

	return 0;
}