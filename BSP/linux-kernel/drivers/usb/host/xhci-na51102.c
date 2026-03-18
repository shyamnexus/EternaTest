/*
 * Copyright (C) 2017 Novatek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <asm/io.h>
//#include <mach/hardware.h>
//#include <mach/nvt-gpio.h>
#include <plat/efuse_protected.h>
#include <plat/top.h>
#include <linux/of_address.h>

#include "xhci.h"
#include <linux/soc/nvt/nvt-info.h>

static unsigned int usb3_txen = 1;
// module_param_named(usb3_txen, usb3_txen, int, S_IRUGO | S_IWUSR);
// MODULE_PARM_DESC(usb3_txen, "Set usb3 ss tx default force-enable or disable. default is force-enable.");

void __iomem *U3PHY_REG;
void __iomem *U2PHY_REG;
void __iomem *U3CTRL_REG;
void __iomem *cg_regs;

#define USB3PHY_SETREG(ofs,value)   writel((value), (volatile void __iomem *)(U3PHY_REG+((ofs)<<2)))
#define USB3PHY_GETREG(ofs)         readl((volatile void __iomem *)(U3PHY_REG+((ofs)<<2)))

#define USB3U2PHY_SETREG(ofs,value) writel((value), (volatile void __iomem *)(U2PHY_REG+((ofs)<<2)))
#define USB3U2PHY_GETREG(ofs)       readl((volatile void __iomem *)(U2PHY_REG+((ofs)<<2)))

#define USB3CTRL_SETREG(ofs,value)   writel((value), (volatile void __iomem *)(U3CTRL_REG+((ofs))))
#define USB3CTRL_GETREG(ofs)         readl((volatile void __iomem *)(U3CTRL_REG+((ofs))))

#ifdef CONFIG_POWER_SAVE_OPTION
#define USB3PDPHY_SETREG(ofs,value)   writel((value), (volatile void __iomem *)(U3PHY_REG+((ofs))))
#define USB3PDPHY_GETREG(ofs)         readl((volatile void __iomem *)(U3PHY_REG+((ofs))))

void xhci_u2phy_pd(void)
{
	unsigned long    temp;
	//printk("U2 PD start\n");

	// dmpulldown = 0, dppulldown = 0, suspendm = 1, termselect = 1, xcvrselect = 01, opmode = 00
	//sendln 'mem w 0x2F05F10d4 0xb4'
	//sendln 'mem w 0x2F05E10d4 0xb4'
	//OUTREG32(0xF05F10d4, 0xb4);
	//OUTREG32(0xF05E10d4, 0xb4);
	USB3U2PHY_SETREG(0xD4>>2 , 0xb4);

	//; Enable sw reg 0x00d4
	//sendln 'mem w 0x2F05F10d0 0xff'
	//sendln 'mem w 0x2F05E10d0 0xff'
	//OUTREG32(0xF05F10d0, 0xff);
	//OUTREG32(0xF05E10d0, 0xff);
	USB3U2PHY_SETREG(0xD0>>2 , 0xff);

	//; pll_pdn = 0, pll_rstn = 0
	//sendln 'mem w 0x2F05F1008 0x14'
	//sendln 'mem w 0x2F05E1008 0x14'
	//OUTREG32(0xF05F1008, 0x14);
	//OUTREG32(0xF05E1008, 0x14);
	USB3U2PHY_SETREG(0x8>>2 , 0xff);

	//; bias_pdn = 0, ldo_pdn = 0
	//sendln 'mem w 0x2F05F1000 0x8a'
	//sendln 'mem w 0x2F05E1000 0x8a'
	//OUTREG32(0xF05F1000, 0x8a);
	//OUTREG32(0xF05E1000, 0x8a);
	USB3U2PHY_SETREG(0x0>>2 , 0x8a);

	//; datarpu1 = 0, datarpu2 = 1, spsel = 0, datarpusel = 0, datarpdp = 0, datarpdm = 1
	//;sendln 'mem w 0x2F05F1030 0x13'
	//;sendln 'mem w 0x2F05E1030 0x13'
	//sendln 'mem w 0x2F05F1030 0x35'
	//sendln 'mem w 0x2F05E1030 0x35'
	//OUTREG32(0xF05F1030, 0x35);
	//OUTREG32(0xF05E1030, 0x35);
	USB3U2PHY_SETREG(0x30>>2 , 0x35);

	//; Enable sw reg 0x0030
	//sendln 'mem w 0x2F05F102c 0xff'
	//sendln 'mem w 0x2F05E102c 0xff'
	//OUTREG32(0xF05F102C, 0xff);
	//OUTREG32(0xF05E102C, 0xff);
	USB3U2PHY_SETREG(0x2C>>2 , 0xFF);

	//; usb_pwr_ok = 0
	//sendln 'mem w 0x2F05F1028 0x20'
	//sendln 'mem w 0x2F05E1028 0x20'
	//OUTREG32(0xF05F1028, 0x20);
	//OUTREG32(0xF05E1028, 0x20);
	USB3U2PHY_SETREG(0x28>>2 , 0x20);

	//; u11_rx_pwd = 1
	//sendln 'mem w 0x2F05F1024 0xb3'
	//sendln 'mem w 0x2F05E1024 0xb3'
	//OUTREG32(0xF05F1024, 0xb3);
	//OUTREG32(0xF05E1024, 0xb3);
	USB3U2PHY_SETREG(0x24>>2 , 0xb3);

	//; tx_pdn = 0
	//sendln 'mem w 0x2F05F1034 0x90'
	//sendln 'mem w 0x2F05E1034 0x90'
	//OUTREG32(0xF05F1034, 0x90);
	//OUTREG32(0xF05E1034, 0x90);
	USB3U2PHY_SETREG(0x34>>2 , 0x90);

	temp = USB3CTRL_GETREG(0x28);
	temp &= 0x7F;
	USB3CTRL_SETREG(0x28, temp);

	printk("U2 PD\n");
}


void xhci_u3phy_pd(void)
{
	unsigned long    temp;

	//printk("U3 PD start\n");
	// BIAS powerdown
	// to  bank0
	USB3PDPHY_SETREG(0x3FC, 0x00);
	//  0xec074000 bit0  = PDN
	//USB3PDPHY_SETREG(0x00, 0xF8);
	USB3PDPHY_SETREG(0x00, 0xF9);
	// TX powedown
	// to  bank0
	USB3PDPHY_SETREG(0x3FC, 0x00);
	USB3PDPHY_SETREG(0xC4, 0x01);
	USB3PDPHY_SETREG(0x44, 0x10);
	// TXPLL disable
	// to  bank0
	USB3PDPHY_SETREG(0x3FC, 0x00);
	// 0xF05F21a4 bit 5 = TXPLL_PDB force mode
	USB3PDPHY_SETREG(0x1A4, 0x20);
	// 0xF05F2190 bit 6 = TXPLL_PDB force mode
	USB3PDPHY_SETREG(0x190, 0x45);


	// TXPLL 312.5MHz
	// to  bank0
	USB3PDPHY_SETREG(0x3FC, 0x00);
	//bit1 : TXPLL_VCO_BAND_SEL manual mode
	//bit3 : TXPLL_CLKMULT_CTL manual mode
	//bit5 : TXPLL_DIV_PRE manual mode
	//bit6 : TXPLL_DIV_CNT manual mode
	USB3PDPHY_SETREG(0x1A8, 0x6A);


	//bit[7:5] = TX_PLL_CLKMULT_CTL[2:0]
	//sendln 'bit set 0xF05F2194 7 5 0x5'
	//bitmask_set(0xF05F2194, 7, 5, 0x5);
	//bit[3] = TXPLL_VCO_BAND_SEL
	//sendln 'bit set 0xF05F2194 3 3 0'
	//bitmask_set(0xF05F2194, 3, 3, 0);
	//bit[1:0] = TXPLL_CP[1:0]
	//sendln 'bit set 0xF05F2194 1 0 0x0'
	//bitmask_set(0xF05F2194, 1, 0, 0);

	//bitmask_set(0xF05F2194, 7, 5, 0x5);
	temp = USB3PDPHY_GETREG(0x194);
	//temp &= ~(0xEB);
	temp &= ~(0xE0);
	temp |= 0xA0;
	USB3PDPHY_SETREG(0x194 , temp);

	//bitmask_set(0xF05F2194, 3, 3, 0);
	temp = USB3PDPHY_GETREG(0x194);
	temp &= (0xF7);
	USB3PDPHY_SETREG(0x194 , temp);

	//bitmask_set(0xF05F2194, 1, 0, 0);
	temp = USB3PDPHY_GETREG(0x194);
	temp &= (0xFC);
	USB3PDPHY_SETREG(0x194 , temp);

	//bit[7:5] = TX_PLL_DIV_PRE[2:0]
	//sendln 'bit set 0xF05F2198 7 5 0x1'
	//bitmask_set(0xF05F2198, 7, 5, 0x1);
	temp = USB3PDPHY_GETREG(0x198);
	temp &= ~(0xE0);
	temp |= 0x20;
	USB3PDPHY_SETREG(0x198 , temp);

	//bit[4:0] = TX_PLL_DIV_CNT[4:0]
	//sendln 'bit set 0xF05F219C 4 0 0x9'
	//bitmask_set(0xF05F219C, 4, 0, 0x9);
	temp = USB3PDPHY_GETREG(0x19C);
	temp &= ~(0x1F);
	temp |= 0x9;
	USB3PDPHY_SETREG(0x19C , temp);

	// LDO disable
	// to  bank0
	USB3PDPHY_SETREG(0x3FC, 0x00);
	temp = USB3PDPHY_GETREG(0x1A4);
	temp |= 0x2;
	USB3PDPHY_SETREG(0x1A4 , temp);
	// 0xF05F2190 bit 3 = EN_LDO_18 force value
	//sendln 'bit set 0xF05F2190 2 2 1'
	temp = USB3PDPHY_GETREG(0x190);
	temp |= 0x4;
	USB3PDPHY_SETREG(0x190 , temp);

	// Disable BIASGEN
	USB3PDPHY_SETREG(0x3FC, 0x02);

	temp = USB3PDPHY_GETREG(0x9C);
	temp &= ~(0x1);
	USB3PDPHY_SETREG(0x9C , temp);

	// LOD 1.8v power down
	// to bank 2
	USB3PDPHY_SETREG(0x3FC, 0x02);
	temp = USB3PDPHY_GETREG(0xA8);
	temp &= ~(0x80);
	USB3PDPHY_SETREG(0xA8 , temp);
	// LFPS comparator disable
	// to bank 2
	USB3PDPHY_SETREG(0x3FC, 0x02);
	temp = USB3PDPHY_GETREG(0x104);
	temp |= 0x2;
	USB3PDPHY_SETREG(0x104 , temp);

	temp = USB3PDPHY_GETREG(0xF8);
	temp &= ~(0x2);
	USB3PDPHY_SETREG(0xF8 , temp);

	// TX_LFPS_CLKIN
	// to bank 0
	USB3PDPHY_SETREG(0x3FC, 0x00);
	temp = USB3PDPHY_GETREG(0xC8);
	temp |= 0x4;
	USB3PDPHY_SETREG(0xC8 , temp);
	// EQ_RC_CLK
	// to bank 2
	USB3PDPHY_SETREG(0x3FC, 0x02);
	temp = USB3PDPHY_GETREG(0xC8);
	temp |= 0x20;
	USB3PDPHY_SETREG(0xC8 , temp);

	// CDR_DCO_BA_CLK
	// to bank 2
	USB3PDPHY_SETREG(0x3FC, 0x02);
	temp = USB3PDPHY_GETREG(0xE0);
	temp |= 0x80;
	USB3PDPHY_SETREG(0xE0 , temp);
	USB3PDPHY_SETREG(0x3FC, 0x02);
	temp = USB3PDPHY_GETREG(0xE4);
	temp |= 0x4;
	USB3PDPHY_SETREG(0xE4 , temp);

	// TXPLL_CLKIN
	//USB3PDPHY_SETREG(0x3FC, 0x00);
	//temp = USB3PDPHY_GETREG(0x458);
	//temp |= 0x2;
	//USB3PDPHY_SETREG(0x458 , temp);

	// BIAS_EN_CDR_CST
	USB3PDPHY_SETREG(0x3FC, 0x02);
	temp = USB3PDPHY_GETREG(0xA0);
	temp &= ~(0x1);
	USB3PDPHY_SETREG(0xA0 , temp);
	temp = USB3PDPHY_GETREG(0xA0);
	temp &= ~(0x2);
	USB3PDPHY_SETREG(0xA0 , temp);

	// BIAS_EN_DFE_PTAT_100U
	USB3PDPHY_SETREG(0x3FC, 0x02);
	temp = USB3PDPHY_GETREG(0x9C);
	temp &= ~(0x2);
	USB3PDPHY_SETREG(0x9C , temp);

	//BIAS_EN_DFE_VBGR_50U
	USB3PDPHY_SETREG(0x3FC, 0x02);
	temp = USB3PDPHY_GETREG(0x9C);
	temp &= ~(0x4);
	USB3PDPHY_SETREG(0x9C , temp);

	// CDR_EN* and DFE_EN
	// 0xF05F20d4 CDR_EN* and DFE_NE force mode
	USB3PDPHY_SETREG(0xD4, 0xBF);
	// 0xF05F2034
	// bit 0 : DFE_EN
	// bit 1 : DFE_EN_EG
	// bit 2 : DFE_PTAT_EN
	// bit 3 : DFE_EN_PI
	// bit 4 : DFE_EN_ER
	// bit 5 : CDR_EN
	// bit 7 : CDR_EN_LPF
	//sendln 'mem w 0xF05F2034 0x00'
	USB3PDPHY_SETREG(0x34, 0x00);

	// CDR_EN_VCO
	temp = USB3PDPHY_GETREG(0x9C);
	temp |= 0x10;
	USB3PDPHY_SETREG(0x9C , temp);
	temp = USB3PDPHY_GETREG(0x7C);
	temp &= ~(0x8);
	USB3PDPHY_SETREG(0x7C , temp);
	// DFE_EN, DFE_EOC_EN
	// 0xF05F20c0 bit[5:4] = DFE_ECO_EN, DFE_EN force mode
	USB3PDPHY_SETREG(0xC0, 0x30);

	temp = USB3PDPHY_GETREG(0x8);
	temp &= ~(0x20);
	USB3PDPHY_SETREG(0x8 , temp);
	temp = USB3PDPHY_GETREG(0x8);
	temp &= ~(0x40);
	USB3PDPHY_SETREG(0x8 , temp);

	temp = USB3PDPHY_GETREG(0xC4);
	temp |= 0x80;
	USB3PDPHY_SETREG(0xC4 , temp);

	temp = USB3PDPHY_GETREG(0x14);
	temp &= ~(0x20);
	USB3PDPHY_SETREG(0x14 , temp);

	temp = USB3PDPHY_GETREG(0xd0);
	temp |= 0x2;
	USB3PDPHY_SETREG(0xd0 , temp);

	temp = USB3PDPHY_GETREG(0x2c);
	temp &= ~(0x20);
	USB3PDPHY_SETREG(0x2c , temp);
	// TX off
	// to bank0
	USB3PDPHY_SETREG(0x3FC, 0x00);
	// bit 2=DET1_EN =0
	USB3PDPHY_SETREG(0x4C, 0x10);
	// bit7 = PDB_CUR = 0
	USB3PDPHY_SETREG(0x48, 0x58);

	//USB3PDPHY_SETREG(0x3FC, 0x02);

	//temp = USB3PDPHY_GETREG(0xd4);
	//temp |= 0x40;
	//USB3PDPHY_SETREG(0xd4 , temp);
	//temp = USB3PDPHY_GETREG(0x34);
	//temp |= 0x40;
	//USB3PDPHY_SETREG(0x34 , temp);

	printk("U3 PD\n");
}
#endif

void nvt_usb3dev_u2phy_reset(void)
{
	struct clk       *clk;

	clk = clk_get(NULL, "u2phy");
	if (!IS_ERR(clk)) {
		clk_prepare_enable(clk);
		clk_disable_unprepare(clk);
	} else
		pr_err("%s: clk u2phy not found\n", __func__);
}

#ifdef CONFIG_POWER_SAVE_OPTION
void nvt_usb3dev_pdphy_reset(void)
{
#if 0
	void __iomem *io_addr;
	UINT32 reg;

	io_addr = ioremap_nocache(0x2F002009C,4);
	reg = *(UINT32 *)io_addr;
	iounmap(io_addr);

	*(UINT32 *)io_addr = reg&0xFFFFFF5F;
	mdelay(10);
	printk("%s: Reset U2/U3 PHY \n", __func__);
	*(UINT32 *)io_addr = reg |0xA0;
#else
	UINT32 reg;
	void __iomem *U3_CGREG;

	U3_CGREG = cg_regs+0x9C;

	reg = *(UINT32 *)U3_CGREG;

	*(UINT32 *)U3_CGREG = reg&0xFFFFFF5F;
	mdelay(10);
	printk("%s: Reset U2/U3 PHY \n", __func__);
	*(UINT32 *)U3_CGREG = reg |0xA0;

#endif
}
#endif

static ssize_t trim_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	writel(0x0, U3CTRL_REG + 0x23fc);
	return sprintf(buf, "0x%x\n", readl(U3CTRL_REG + 0x2680));
}

static DEVICE_ATTR_RO(trim);

static struct attribute *nvt_sysfs_attributes[] = {
	&dev_attr_trim.attr,
	NULL,
};

static const struct attribute_group nvt_sysfs_attr_group = {
	.attrs = nvt_sysfs_attributes,
};

static int nvt_xhci_sysfs_init(struct device *dev)
{
	return sysfs_create_group(&dev->kobj, &nvt_sysfs_attr_group);
}

static void nvt_xhci_sysfs_remove(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &nvt_sysfs_attr_group);
}

UINT32 __weak nvt_get_chip_ver(void)
{
	return 0;
}

int xhci_nvt_host_resume(struct usb_hcd *hcd)
{
	struct xhci_hcd  *xhci = hcd_to_xhci(hcd);
	unsigned long    temp;
	UINT8 u3_trim_swctrl=2;
	UINT8 u2_trim_swctrl = 4;

	temp = USB3CTRL_GETREG(0x28);//readl((volatile unsigned long *)xhci->phy_reg + 0x28);
	//Using 0x28 bit7 to check u2phy status
	// 0: u2 phy not reset
	// 1: u2 phy has been reset
	if((temp&0x80) == 0x00)
	{
		//pr_err("reset u2 phy\r\n");
		nvt_usb3dev_u2phy_reset();
		temp = 0x80;
		USB3CTRL_SETREG(0x28, temp);
	}

	if(nvt_get_chip_ver() == CHIPVER_C) {
		USB3CTRL_SETREG(0x24, 0x1);
	}

	temp = USB3CTRL_GETREG(0x14);
	temp &= 0xFFFFFDFF;
	temp |= 0x2;
	USB3CTRL_SETREG(0x14, temp);

	temp = USB3CTRL_GETREG(0x28);
	temp |= 0x1;
	USB3CTRL_SETREG(0x28, temp);

	temp = USB3CTRL_GETREG(0x10);
	temp &= ~0x6; /*Set Host mode & No Force Suspend*/
	temp |= 0x1;

	if (xhci->force_hs)
		temp |= 0x8;
	else
		temp &= ~(0x1 << 11);

	temp &= ~((0x7 << 16) | (0x7 << 24) | (0x1 << 4));
	temp |= ((0x4 << 16) | (0x4 << 24));
	USB3CTRL_SETREG(0x10, temp);
	udelay(100);

	if ((xhci->u2_tx_swing >= 0) && (xhci->u2_tx_swing <= 7)) {
		u2_trim_swctrl = xhci->u2_tx_swing;
	}

	temp = USB3PHY_GETREG(0x1A1);
	temp &= ~(0x80);
	USB3PHY_SETREG(0x1A1 , temp);

	//usb3_validateTrim();
	temp = USB3PHY_GETREG(0x1A0);
	if ((temp & 0xF) == 0) {
		USB3PHY_SETREG(0x1A0, 0x40+xhci->u3_trim_rint_sel);
		USB3PHY_SETREG(0x1A3, 0x60);
	}

	if (!xhci->force_hs) {
		//========================================================
		//add additonal setting for 0.85V
		//========================================================
		// Bank 2
		USB3PHY_SETREG(0xFF, 0x02);
		// bit[5:4]  = LDO_VLDO_SEL_DCO[1:0] ; 00 = 0.85v
		USB3PHY_SETREG(0x2A, 0x80);
		// Bank 0
		USB3PHY_SETREG(0xFF, 0x00);
		// bit[1:0]  = LDO_SEL_18[1:0] ; 00 = 0.85v
		USB3PHY_SETREG(0x64, 0x44);
		// bit[1:0] = TXPLL_CP[1:0] = 01
		USB3PHY_SETREG(0x65, 0X99);
		//========================================================
		//swing
		//========================================================
		// Bank 0
		USB3PHY_SETREG(0xFF, 0x00);
		// bit[5:4] = V_VCHSEL[1:0] = 11
		USB3PHY_SETREG(0x56, 0X74);
		// TX_AMP_CTL=2, TX_DEC_EM_CTL=8
		USB3PHY_SETREG(0x14, 0X8A);
		// bit[5:4] = TX_AMP_CTL_LFPS[1:0] (from 10 -> 11)
		//USB3PHY_SETREG(0x12, 0XE8);
		USB3PHY_SETREG(0x12, 0XF8);
		// bit3 : TX_DE_EN manual mode
		// bit[7:4]: TX_DEC_EM_CTL manual mode
		USB3PHY_SETREG(0x34, 0Xfc);

		if ((xhci->tx_swing >= 0) && (xhci->tx_swing <= 3)) {
			u3_trim_swctrl = xhci->tx_swing;

			//printk("u3_trim_swctrl = 0x%x\n",u3_trim_swctrl);
			temp = USB3PHY_GETREG(0x14);
			temp &= ~(0x3);
			temp |= (u3_trim_swctrl);
			//printk("temp = 0x%x\n",temp);
			USB3PHY_SETREG(0x14 , temp);
		}

		//======================================================
		//(4) TX termination trim bits								[NEW]
		//======================================================
		// 0xF05F2040[7:4] = tx_term_trim_val[3:0]
		// Bank 0
		USB3PHY_SETREG(0xFF, 0x00);

		temp = USB3PHY_GETREG(0x10);
		temp &= ~(0xF0);
		temp |= (xhci->tx_term_trim_val<<4);
		USB3PHY_SETREG(0x10 , temp);

		//======================================================
		//(5) RX termination trim bits								[NEW]
		//======================================================
		// Bank 2
		USB3PHY_SETREG(0xFF, 0x02);

		temp = USB3PHY_GETREG(0x0);
		temp &= ~(0xF);
		temp |= (xhci->rx_term_trim_val);
		USB3PHY_SETREG(0x00 , temp);

		// Bank 0
		USB3PHY_SETREG(0xFF, 0x00);

		//======================================================
		//(6) T12 USB30 APHY setting								[NEW]
		//======================================================
		USB3PHY_SETREG(0xFF ,0x00);
		USB3PHY_SETREG(0x149,0xFF);
		USB3PHY_SETREG(0x145,0xFF);
		USB3PHY_SETREG(0xFF ,0x01);
		USB3PHY_SETREG(0x168,0xEF);
		USB3PHY_SETREG(0x108,0x02);
		USB3PHY_SETREG(0x1A0,0x5F);
		USB3PHY_SETREG(0x1A2,0xF4);
		USB3PHY_SETREG(0x15E,0xFF);
		USB3PHY_SETREG(0x22 ,0x00);
		USB3PHY_SETREG(0x16C,0x00);
		USB3PHY_SETREG(0x170,0x00);
		USB3PHY_SETREG(0x174,0xB9);
		USB3PHY_SETREG(0x23 ,0x13);
		USB3PHY_SETREG(0x1A5,0x2C);
		USB3PHY_SETREG(0x5A ,0x08);
		USB3PHY_SETREG(0xB9 ,0xC0);
		USB3PHY_SETREG(0xFF ,0x02);
		USB3PHY_SETREG(0x28 ,0xF7);
		USB3PHY_SETREG(0x3	,0x21);
		USB3PHY_SETREG(0x2	,0xEC);
		USB3PHY_SETREG(0x1E ,0x10);
		USB3PHY_SETREG(0x33 ,0xA0);
		USB3PHY_SETREG(0xA	,0x0F);
		USB3PHY_SETREG(0xB	,0x3B);
		USB3PHY_SETREG(0x1F ,0x7C);
		//USB3PHY_SETREG(0x2A ,0x90);
		USB3PHY_SETREG(0xFF ,0x00);

		/* asd_mode=1 (toggle rate) */
		USB3PHY_SETREG(0x198, 0x04);
		/* RX_ICTRL's offset = 0 */
		//USB3PHY_SETREG(0x1BF, 0x40);
		/* TX_AMP_CTL=2, TX_DEC_EM_CTL=8 */
		USB3PHY_SETREG(0x014, 0x8a);
		/* TX_LFPS_AMP_CTL = 1 */
		USB3PHY_SETREG(0x034, 0xfc);//default 0xfc
		/* PHY Power Mode Change ready reponse time. (3 is 1ms.)(4 is 1.3ms.) */

		//USB3PHY_SETREG(0x114, 0x0B);
		USB3PHY_SETREG(0x114, 0x04);
		//USB3PHY_SETREG(0x152, 0x2E);
		//USB3PHY_SETREG(0x153, 0x01);
		//USB3PHY_SETREG(0x1B0, 0xC0);
		//USB3PHY_SETREG(0x1B1, 0x91);
		//USB3PHY_SETREG(0x1B2, 0x00);
		//USB3PHY_SETREG(0x135, 0x88);
		//USB3PHY_SETREG(0x12A, 0x50);
		//USB3PHY_SETREG(0x1F0, 0x80);
		//USB3PHY_SETREG(0x1F5, 0x01|(u3_trim_icdr<<4));//0xB1
		//USB3PHY_SETREG(0x105, 0x01);
		USB3PHY_SETREG(0x056, 0x74);//disconnect level to 0x3
		udelay(2);

		USB3PHY_SETREG(0x102, 0xF0);
		udelay(10);
		USB3PHY_SETREG(0x102, 0x00);
		udelay(300);
	}

	//USB3PHY_SETREG(0x103, 0x01);
	//udelay(100);

	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	if (!xhci->force_hs) {
		if(usb3_txen) {
			temp = USB3PHY_GETREG(0x011);
			temp |= 0x1;
			USB3PHY_SETREG(0x011, temp);
			temp = USB3PHY_GETREG(0x031);
			temp |= 0x1;
			USB3PHY_SETREG(0x031, temp);
		}

		USB3PHY_SETREG(0x029, 0x1);
	}

	USB3U2PHY_SETREG(0x09, 0xb0);
	USB3U2PHY_SETREG(0x12, 0x31);

	//Set u2 phy swing
	temp = USB3U2PHY_GETREG(0x6);
	temp &= ~(0x7 << 1);
	temp |= (u2_trim_swctrl << 0x1);
	USB3U2PHY_SETREG(0x6, temp);

	return 0;
}


int xhci_nvt_host_init(struct usb_hcd *hcd)
{
	struct xhci_hcd  *xhci = hcd_to_xhci(hcd);
	struct device *dev = hcd->self.controller;
	void __iomem     *regs;
	struct clk       *clk;
	unsigned long    temp;
	//UINT8 u3_trim_rint_sel=8,u3_trim_swctrl=4, u3_trim_sqsel=4,u3_trim_icdr=0xB;
	UINT8 u3_trim_rint_sel=8;
	UINT8 tx_term_trim_val = 6;
	UINT8 rx_term_trim_val = 12;
	UINT8 u3_trim_swctrl=2;
	UINT8 u2_trim_swctrl = 4;
	u32 force_hs = 0;
	u32 get_power_gpio[2];
	int power_gpio[2];
	u32 get_tx_swing;
	int tx_swing;
	u32 u2_get_tx_swing;
	int u2_tx_swing;

	if (of_property_read_u32(dev->of_node, "nvt_stop_ep", &xhci->nvt_stop_ep)) {
		xhci->nvt_stop_ep = 0;
	} else {
		printk("NVT stop EP patch!\n");
	}

	if(!of_property_read_u32(dev->of_node, "force-hs", &force_hs)) {
		pr_debug("force-hs!\n");
	}

	if (of_property_read_u32_array(dev->of_node, "power_gpio", get_power_gpio, 2)) {
		power_gpio[0] = -1;
		power_gpio[1] = -1;
	} else {
		power_gpio[0] = get_power_gpio[0];
		power_gpio[1] = get_power_gpio[1];

		gpio_direction_output(power_gpio[0], !power_gpio[1]);
		mdelay(200);
	}

	if (of_property_read_u32(dev->of_node, "tx_swing", &get_tx_swing)) {
		tx_swing = -1;
	} else {
		tx_swing = get_tx_swing;
	}

	if (of_property_read_u32(dev->of_node, "u2_tx_swing", &u2_get_tx_swing)) {
		u2_tx_swing = -1;
	} else {
		u2_tx_swing = u2_get_tx_swing;
	}

	xhci->tx_swing = tx_swing;
	xhci->u2_tx_swing = u2_tx_swing;

	clk = clk_get(NULL, "pll20");
	if (!IS_ERR(clk)) {
		xhci->phyclk = clk;
		//clk_set_rate(xhci->phyclk, 498750000);
		clk_set_rate(xhci->phyclk, 500000000);
	} else {
		xhci->phyclk = NULL;
		pr_err("%s: u3phyclk not found\n", __func__);
	}
#if 1
	clk = clk_get(NULL, "pll20_ss");
	if (!IS_ERR(clk)) {
		xhci->phyclk_ss = clk;
		clk_set_phase(xhci->phyclk_ss, 0x6);
	} else {
		pr_err("%s: pll20ss not found\n", __func__);
	}
#endif
	clk = clk_get(dev, dev_name(dev));
	if (!IS_ERR(clk)) {
		clk_prepare_enable(clk);
		clk_disable_unprepare(clk);
		clk_prepare_enable(clk);
	} else
		pr_err("%s: %s not found\n", __func__, dev_name(dev));

#if 0
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(regs)) {
		pr_err("map err1\r\n");
	}
#else
       regs = of_iomap(dev->of_node, 1);
       if (!regs) {
              pr_err("map err1\r\n");
       }
#endif
	cg_regs = of_iomap(dev->of_node, 2);
	if (!cg_regs) {
		pr_err("cg regs map err\r\n");
	}

	U3PHY_REG = regs + 0x2000;
	U2PHY_REG = regs + 0x1000;
	U3CTRL_REG = regs;

	temp = readl(regs + 0x28);
	//pr_err("%s: 0x28 = 0x%x\n", __func__,temp);

	//u3phy check
	{
		clk = clk_get(NULL, "u3phy");
		if (!IS_ERR(clk)) {
			pr_err("%s: u3phy reset\n", __func__);
			clk_prepare_enable(clk);
			clk_disable(clk);
			clk_enable(clk);
		} else
			pr_err("%s: u3phy not found\n", __func__);
	}

#ifdef CONFIG_POWER_SAVE_OPTION
	nvt_usb3dev_pdphy_reset();
#endif

	//keep original setting
	writel(temp, regs + 0x28);

	//Using 0x28 bit7 to check u2phy status
	// 0: u2 phy not reset
	// 1: u2 phy has been reset
	if((temp&0x80) == 0x00)
	{
		//pr_err("reset u2 phy\r\n");
		nvt_usb3dev_u2phy_reset();
		temp = 0x80;
		writel(temp, regs + 0x28);
	}

	if(nvt_get_chip_ver() == CHIPVER_C) {
		writel(0x01, regs + 0x24);
	}

	temp = readl(regs + 0x14);
	temp &= 0xFFFFFDFF;
	temp |= 0x2;
	writel(temp, regs + 0x14);

	temp = readl(regs + 0x28);
	temp |= 0x1;
	writel(temp, regs + 0x28);

	temp = readl(regs + 0x10);
	temp &= ~0x6; /*Set Host mode & No Force Suspend*/
	temp |= 0x1;
	if (force_hs)
		temp |= 0x8;
	else
		temp &= ~(0x1 << 11);

	temp &= ~((0x7 << 16) | (0x7 << 24) | (0x1 << 4));
	temp |= ((0x4 << 16) | (0x4 << 24));

	writel(temp, regs + 0x10);
	udelay(100);

	xhci->force_hs = force_hs;

	xhci->phy_reg = (unsigned long)regs;

	xhci->ctrl_reg = hcd->regs;

	xhci->power_gpio[0] = power_gpio[0];

	xhci->power_gpio[1] = power_gpio[1];

	if ((u2_tx_swing >= 0) && (u2_tx_swing <= 7)) {
		u2_trim_swctrl = u2_tx_swing;
	}

#if 1
	//rxdatak and rxdata use pclk posedge/negedge to controller
	// 0 : used pclk posedge
	// 1 : used pclk negedge
	//USB3PHY_SETREG(0x0D, 0x01);
	{
		UINT16 data=0;
		//INT32 result;

		if (efuse_readParamOps(EFUSE_USB_SATA_TRIM_DATA, &data) != 0) {
			pr_err("[%d] data = NULL\n", EFUSE_USB_SATA_TRIM_DATA);
		} else {
			//12K resistor 1<= x <= 20(0x14)
			pr_info("  USB(SATA) Trim data = 0x%04x\r\n", (int)data);
			u3_trim_rint_sel = data & 0x1F;
			tx_term_trim_val = ((data >> 5) & 0xF);
			rx_term_trim_val = ((data >> 9) & 0xF);
			if (u3_trim_rint_sel > 0x14) {
				pr_info("12K_resistor Trim data error 0x%04x > 20(0x14)\r\n", (int)u3_trim_rint_sel);
				u3_trim_rint_sel=8;
			} else if (u3_trim_rint_sel < 0x1) {
				pr_info("12K_resistor Trim data error 0x%04x < 1(0x1)\r\n", (int)u3_trim_rint_sel);
				u3_trim_rint_sel=8;
			} else {
				pr_info("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)u3_trim_rint_sel);
			}

			//TX term bit[8..5] 0x2 <= x <= 0xE
			if (tx_term_trim_val > 0xE) {
				pr_info("USB(SATA) TX Trim data error 0x%04x > 0xE\r\n", (int)tx_term_trim_val);
				tx_term_trim_val = 6;
			} else if (tx_term_trim_val < 0x2) {
				pr_info("USB(SATA) TX Trim data error 0x%04x < 0x2\r\n", (int)tx_term_trim_val);
				tx_term_trim_val = 6;
			} else {
				pr_info("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)tx_term_trim_val);
			}
			//RX term bit[12..9] 0x8 <= x <= 0xE
			if (rx_term_trim_val > 0xE) {
				pr_info("USB(SATA) RX Trim data error 0x%04x > 0xE\r\n", (int)rx_term_trim_val);
				rx_term_trim_val = 12;
			} else if (rx_term_trim_val < 0x8) {
				pr_info("USB(SATA) RX Trim data error 0x%04x < 0x8\r\n", (int)rx_term_trim_val);
				rx_term_trim_val = 12;
			} else {
				pr_info("  *USB(SATA) RX Trim data 0x8 <= [0x%04x] <= 0xE\r\n", (int)rx_term_trim_val);
			}
		}

		xhci->u3_trim_rint_sel = u3_trim_rint_sel;
		xhci->tx_term_trim_val = tx_term_trim_val;
		xhci->rx_term_trim_val = rx_term_trim_val;


		//result= efuse_readParamOps(0x100, &data);
		//result= 1;
		//if(result >= 0) {
		//	u3_trim_rint_sel = data&0x1F;
		//	//u3_trim_icdr     = (data>>5)&0xF;
		//}

		temp = USB3PHY_GETREG(0x1A1);
		temp &= ~(0x80);
		USB3PHY_SETREG(0x1A1 , temp);

		//usb3_validateTrim();
		temp = USB3PHY_GETREG(0x1A0);
		if ((temp & 0xF) == 0) {
			USB3PHY_SETREG(0x1A0, 0x40+u3_trim_rint_sel);
			USB3PHY_SETREG(0x1A3, 0x60);
		}

		//temp = USB3U2PHY_GETREG(0x06);
		//temp &= ~(0x7<<1);
		//temp |= (u3_trim_swctrl<<1);
		//USB3U2PHY_SETREG(0x06, temp);

		//temp = USB3U2PHY_GETREG(0x05);
		//temp &= ~(0x7<<2);
		//temp |= (u3_trim_sqsel<<2);
		//USB3U2PHY_SETREG(0x05, temp);
	}

	if (!force_hs) {
		//========================================================
		//add additonal setting for 0.85V
		//========================================================
		// Bank 2
		USB3PHY_SETREG(0xFF, 0x02);
		// bit[5:4]  = LDO_VLDO_SEL_DCO[1:0] ; 00 = 0.85v
		USB3PHY_SETREG(0x2A, 0x80);
		// Bank 0
		USB3PHY_SETREG(0xFF, 0x00);
		// bit[1:0]  = LDO_SEL_18[1:0] ; 00 = 0.85v
		USB3PHY_SETREG(0x64, 0x44);
		// bit[1:0] = TXPLL_CP[1:0] = 01
		USB3PHY_SETREG(0x65, 0X99);

		//========================================================
		//swing
		//========================================================
		// Bank 0
		USB3PHY_SETREG(0xFF, 0x00);
		// bit[5:4] = V_VCHSEL[1:0] = 11
		USB3PHY_SETREG(0x56, 0X74);
		// TX_AMP_CTL=2, TX_DEC_EM_CTL=8
		USB3PHY_SETREG(0x14, 0X8A);
		// bit[5:4] = TX_AMP_CTL_LFPS[1:0] (from 10 -> 11)
		USB3PHY_SETREG(0x12, 0XE8);
		//USB3PHY_SETREG(0x12, 0XF8);
		// bit3 : TX_DE_EN manual mode
		// bit[7:4]: TX_DEC_EM_CTL manual mode
		USB3PHY_SETREG(0x34, 0Xfc);

		if ((tx_swing >= 0) && (tx_swing <= 3)) {
			u3_trim_swctrl = tx_swing;

			//printk("u3_trim_swctrl = 0x%x\n",u3_trim_swctrl);
			temp = USB3PHY_GETREG(0x14);
			temp &= ~(0x3);
			temp |= (u3_trim_swctrl);
			//printk("temp = 0x%x\n",temp);
			USB3PHY_SETREG(0x14 , temp);
		}

		//======================================================
		//(4) TX termination trim bits								[NEW]
		//======================================================
		// 0xF05F2040[7:4] = tx_term_trim_val[3:0]
		// Bank 0
		USB3PHY_SETREG(0xFF, 0x00);

		temp = USB3PHY_GETREG(0x10);
		temp &= ~(0xF0);
		temp |= (tx_term_trim_val<<4);
		USB3PHY_SETREG(0x10 , temp);

		//======================================================
		//(5) RX termination trim bits								[NEW]
		//======================================================
		// Bank 2
		USB3PHY_SETREG(0xFF, 0x02);

		temp = USB3PHY_GETREG(0x0);
		temp &= ~(0xF);
		temp |= (rx_term_trim_val);
		USB3PHY_SETREG(0x00 , temp);

		// Bank 0
		USB3PHY_SETREG(0xFF, 0x00);

		//======================================================
		//(6) T12 USB30 APHY setting								[NEW]
		//======================================================
		USB3PHY_SETREG(0xFF ,0x00);
		USB3PHY_SETREG(0x149,0xFF);
		USB3PHY_SETREG(0x145,0xFF);
		USB3PHY_SETREG(0xFF ,0x01);
		USB3PHY_SETREG(0x168,0xEF);
		USB3PHY_SETREG(0x108,0x02);
		USB3PHY_SETREG(0x1A0,0x5F);
		USB3PHY_SETREG(0x1A2,0xF4);
		USB3PHY_SETREG(0x15E,0xFF);
		USB3PHY_SETREG(0x22 ,0x00);
		USB3PHY_SETREG(0x16C,0x00);
		USB3PHY_SETREG(0x170,0x00);
		USB3PHY_SETREG(0x174,0xB9);
		USB3PHY_SETREG(0x23 ,0x13);
		USB3PHY_SETREG(0x1A5,0x2C);
		USB3PHY_SETREG(0x5A ,0x08);
		USB3PHY_SETREG(0xB9 ,0xC0);
		USB3PHY_SETREG(0xFF ,0x02);
		USB3PHY_SETREG(0x28 ,0xF7);
		USB3PHY_SETREG(0x3  ,0x21);
		USB3PHY_SETREG(0x2  ,0xEC);
		USB3PHY_SETREG(0x1E ,0x10);
		USB3PHY_SETREG(0x33 ,0xA0);
		USB3PHY_SETREG(0xA  ,0x0F);
		USB3PHY_SETREG(0xB  ,0x3B);
		USB3PHY_SETREG(0x1F ,0x7C);
		//USB3PHY_SETREG(0x2A ,0x90);
		USB3PHY_SETREG(0xFF ,0x00);

		/* asd_mode=1 (toggle rate) */
		USB3PHY_SETREG(0x198, 0x04);
		/* RX_ICTRL's offset = 0 */
		//USB3PHY_SETREG(0x1BF, 0x40);
		/* TX_AMP_CTL=2, TX_DEC_EM_CTL=8 */
		USB3PHY_SETREG(0x014, 0x8a);
		/* TX_LFPS_AMP_CTL = 1 */
		USB3PHY_SETREG(0x034, 0xfc);//default 0xfc
		/* PHY Power Mode Change ready reponse time. (3 is 1ms.)(4 is 1.3ms.) */

		//USB3PHY_SETREG(0x114, 0x0B);
		USB3PHY_SETREG(0x114, 0x04);
		//USB3PHY_SETREG(0x152, 0x2E);
		//USB3PHY_SETREG(0x153, 0x01);
		//USB3PHY_SETREG(0x1B0, 0xC0);
		//USB3PHY_SETREG(0x1B1, 0x91);
		//USB3PHY_SETREG(0x1B2, 0x00);
		//USB3PHY_SETREG(0x135, 0x88);
		//USB3PHY_SETREG(0x12A, 0x50);
		//USB3PHY_SETREG(0x1F0, 0x80);
		//USB3PHY_SETREG(0x1F5, 0x01|(u3_trim_icdr<<4));//0xB1
		//USB3PHY_SETREG(0x105, 0x01);
		USB3PHY_SETREG(0x056, 0x74);//disconnect level to 0x3
		udelay(2);

		USB3PHY_SETREG(0x102, 0xF0);
		udelay(10);
		USB3PHY_SETREG(0x102, 0x00);
		udelay(300);
	}
	//USB3PHY_SETREG(0x103, 0x01);
	//udelay(100);

	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	if (!force_hs) {
		if(usb3_txen) {
			temp = USB3PHY_GETREG(0x011);
			temp |= 0x1;
			USB3PHY_SETREG(0x011, temp);

			temp = USB3PHY_GETREG(0x031);
			temp |= 0x1;
			USB3PHY_SETREG(0x031, temp);
		}

		USB3PHY_SETREG(0x029, 0x1);
	}
	USB3U2PHY_SETREG(0x09, 0xb0);

	USB3U2PHY_SETREG(0x12, 0x31);

	//Set u2 phy swing
	temp = USB3U2PHY_GETREG(0x6);
	temp &= ~(0x7 << 1);
	temp |= (u2_trim_swctrl << 0x1);
	USB3U2PHY_SETREG(0x6, temp);
#endif
	if (power_gpio[0] > 0) {
		gpio_direction_output(power_gpio[0], power_gpio[1]);
		mdelay(200);
	}

	nvt_xhci_sysfs_init(dev);

	pr_info("%s with %s mode, apb 0x10= 0x%08lX, ver %s\r\n", __func__,
							force_hs ? "HS": "SS",
							temp, DRV_VERSION);
	return 0;
}

void xhci_nvt_host_uninit(struct platform_device *pdev, struct usb_hcd *hcd)
{
	struct xhci_hcd  *xhci = hcd_to_xhci(hcd);

	nvt_xhci_sysfs_remove(&pdev->dev);

	if (xhci->power_gpio[0] > 0)
		gpio_direction_output(xhci->power_gpio[0], !xhci->power_gpio[1]);
#ifdef CONFIG_POWER_SAVE_OPTION
	xhci_u2phy_pd();
	xhci_u3phy_pd();
#endif
	{
		struct clk       *clk;

		/* controller clk */
		clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
		if (!IS_ERR(clk)) {
			clk_disable_unprepare(clk);
			clk_put(clk);
		} else
			pr_err("%s: %s not found\n", __func__, dev_name(&pdev->dev));


		/* u3phy clk */
		clk = clk_get(NULL, "u3phy");
		if (!IS_ERR(clk)) {
			pr_err("%s: u3phy reset\n", __func__);
			clk_disable_unprepare(clk);
			clk_put(clk);
		} else
			pr_err("%s: u3phy not found\n", __func__);
	}
}

