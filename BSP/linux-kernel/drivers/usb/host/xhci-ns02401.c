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
#include <linux/proc_fs.h>

#include "xhci.h"

#ifndef CONFIG_NVT_FPGA_EMULATION
//static unsigned int usb3_txen = 1;
#endif
// module_param_named(usb3_txen, usb3_txen, int, S_IRUGO | S_IWUSR);
// MODULE_PARM_DESC(usb3_txen, "Set usb3 ss tx default force-enable or disable. default is force-enable.");

void __iomem *U3PHY_REG;
void __iomem *U2PHY_REG;
void __iomem *U3CTRL_REG;
void __iomem *cg_regs;

static struct proc_dir_entry *nvt_xhci_proc_root    = NULL;
static struct proc_dir_entry *nvt_xhci_proc_test   = NULL;

enum xhci_test_id {
        XHCI_TST_PWR_TOG,
        XHCI_TST_COUNT
};

#define USB3PHY_SETREG(ofs,value)   writel((value), (volatile void __iomem *)(U3PHY_REG+((ofs)<<2)))
#define USB3PHY_GETREG(ofs)         readl((volatile void __iomem *)(U3PHY_REG+((ofs)<<2)))

#define USB3U2PHY_SETREG(ofs,value) writel((value), (volatile void __iomem *)(U2PHY_REG+((ofs)<<2)))
#define USB3U2PHY_GETREG(ofs)       readl((volatile void __iomem *)(U2PHY_REG+((ofs)<<2)))

#define USB3CTRL_SETREG(ofs,value)   writel((value), (volatile void __iomem *)(U3CTRL_REG+((ofs))))
#define USB3CTRL_GETREG(ofs)         readl((volatile void __iomem *)(U3CTRL_REG+((ofs))))

#define U3_TOP_REG0     0x10
#define USB_VBUSVALID   0
#define USB_ID          1
#define USB_FORCE_HS    3
#define ELAS_BUF_MODE   4
#define RXTERM_EN_MASK  11
#define U3PHY_RST_CTL0  16
#define U3PHY_RST_CTL1  17
#define U3PHY_RST_CTL2  18
#define U2PHY_RST_CTL0  24
#define U2PHY_RST_CTL1  25
#define U2PHY_RST_CTL2  26
#define PM_U3TOU0_PATCH 21

#define U3_TOP_REG1     0x14
#define BUF_RESP_MODE	1
#define CH_DIS          9

#define NVT_SE1_REG     0xC0
#define SE1_EN_BIT      4

#define U3_CLKEN_IDX    0x7C
#define U3_CLKEN_BIT    0x2
#define U3_2_CLKEN_IDX  0x7C
#define U3_2_CLKEN_BIT  0x3
#define U3_TOP_DBGSEL	0x28
#define U2RST_FLAG_BIT  7

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

int xhci_nvt_host_resume(struct usb_hcd *hcd)
{
	struct xhci_hcd  *xhci = hcd_to_xhci(hcd);
	unsigned long    temp;
#ifndef CONFIG_NVT_FPGA_EMULATION
	UINT8 u3_trim_swctrl = 4;
#endif

	//SSC_period (Fssc:31.6kHz)
	writel(0xbe, cg_regs+0x4594);
	//SSC_mulfac (SSC:0.43%)
	writel(0x6, cg_regs+0x4590);
	//SSC_en (enable ssc down mode)
	writel(0x95, cg_regs+0x458c);

	temp = *(UINT32 *)(cg_regs+U3_CLKEN_IDX);
	//USB3_CLKEN
	temp |= (0x1 <<U3_CLKEN_BIT);
	writel(temp, cg_regs+U3_CLKEN_IDX);

	//pr_err("reset u2 phy\r\n");
	nvt_usb3dev_u2phy_reset();

	//BUF_RESP_MODE & CHANNEL_DISABLE off
	temp = USB3CTRL_GETREG(U3_TOP_REG1);
	temp &= ~(0x1 << CH_DIS);
	temp |= (0x1 << BUF_RESP_MODE);
	USB3CTRL_SETREG(U3_TOP_REG1, temp);

	temp = USB3CTRL_GETREG(U3_TOP_DBGSEL);
	temp |= 0x1;
	USB3CTRL_SETREG(U3_TOP_DBGSEL, temp);

	temp = USB3CTRL_GETREG(U3_TOP_REG0);
	temp &= ~(0x1 << USB_ID);
	temp |= (0x1 << USB_VBUSVALID);
	//force_hs = 1;
	if (xhci->force_hs)
		temp |= (0x1 << USB_FORCE_HS);
	else
		temp &= ~(0x1 << RXTERM_EN_MASK);

	temp &= ~((0x7 << U3PHY_RST_CTL0) | (0x7 << U2PHY_RST_CTL0) | (0x1 << ELAS_BUF_MODE));
	temp |= ((0x1 << U3PHY_RST_CTL2) | (0x1 << U2PHY_RST_CTL2));
	//Support PM mode
	temp |= (0x1 << PM_U3TOU0_PATCH);

	USB3CTRL_SETREG(U3_TOP_REG0, temp);
	udelay(100);

	//Open SE1 detect
	USB3CTRL_SETREG(NVT_SE1_REG, (0x1 << SE1_EN_BIT));
#ifndef CONFIG_NVT_FPGA_EMULATION
	//======================================================
	// USB30 12K & TX trim bits
	//======================================================

	temp = USB3PHY_GETREG(0x102);
	temp = USB3PHY_GETREG(0x115);

	// step1: 12K trim set
	USB3PHY_SETREG(0x1A3, 0x60);

	temp = USB3PHY_GETREG(0x1A1);
	temp &= ~(0x80);
	USB3PHY_SETREG(0x1A1 , temp);

	USB3PHY_SETREG(0x1A0, 0x40 + xhci->USB_SATA_Internal_12K_resistor);

	// step2: tx trim
	//bit[7:4] :TX_ZTX_CTL force mode
	USB3PHY_SETREG(0x30, 0xF0);
	//0x40[7:4] = tx_term_trim_val[3:0]
	temp = USB3PHY_GETREG(0x10);
	temp &= ~(0xF0);
	temp |= (xhci->tx_term_trim_val<<4);
	USB3PHY_SETREG(0x10 , temp);

	//======================================================
	// USB20 swing & sqsel trim bits
	//======================================================
	temp = USB3U2PHY_GETREG(0x06);
	temp &= ~(0x7<<1);
	temp |= (u3_trim_swctrl<<1);
	USB3U2PHY_SETREG(0x06, temp);

	temp = USB3U2PHY_GETREG(0x05);
	temp &= ~(0x7<<2);
	temp |= (xhci->u3_trim_rint_sel<<2);
	USB3U2PHY_SETREG(0x05, temp);

	//R45 Calibration
	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	//======================================================
	// USB20 Best setting on NT98538
	//======================================================
	//host mode
	USB3U2PHY_SETREG(0x60, 0x01);
	USB3U2PHY_SETREG(0xC, 0x20);
	USB3U2PHY_SETREG(0xB, 0x20);
	USB3U2PHY_SETREG(0x63, 0x60);
	USB3PHY_SETREG(0x56, 0x74);
	USB3U2PHY_SETREG(0x12, 0x31); //RX clk inverse

	//======================================================
	// USB30 Best setting on NT98538
	//======================================================
	USB3PHY_SETREG(0x1BF, 0x40); //EOC RX_ICTRL's offset = 0
	//USB3PHY_SETREG(0x12, 0x28);  //bit[5:4] : TX_AMP_CTL_LFPS[1:0]
	USB3PHY_SETREG(0x14, 0x8A);  //Follow 336 for TX CTS
	USB3PHY_SETREG(0x34, 0xFC);
	USB3PHY_SETREG(0x114, 0x04); //PHY_PWR_MODE_CHG_RDY_RSP = 1.3ms
	USB3PHY_SETREG(0x152, 0x2E); //LB of VBS TUNE FREQ CNT
	USB3PHY_SETREG(0x153, 0x01); //UB of VBS TUNE FREQ CNT
	USB3PHY_SETREG(0x1B0, 0xC0); //EOC 2-data sampler
	USB3PHY_SETREG(0x1B1, 0x91); //LB of EOC THR
	USB3PHY_SETREG(0x1B2, 0x00); //UB of EOC THR
	USB3PHY_SETREG(0x135, 0x88); //SET VGA = FORCE MODE
	USB3PHY_SETREG(0x12A, 0x50); //SET VGA = 5
	USB3PHY_SETREG(0x105, 0x0);  //Remove RX_ZRX OFS
	USB3PHY_SETREG(0x27B, 0x51);
	USB3PHY_SETREG(0x141, 0x51);
	USB3PHY_SETREG(0x140, 0x80);
	USB3PHY_SETREG(0x56, 0x74);  //bit[5:4] = BIAS_VCH_SEL[1:0] = 2'b11
	USB3PHY_SETREG(0x11, 0x11);  //bit[0] TX_PDB
	USB3PHY_SETREG(0x31, 0x1);	 //bit[0] SM_TX_PDB
	USB3PHY_SETREG(0x13, 0x14);  //bit[2] TX_DET1_EN force value = 1
	USB3PHY_SETREG(0x102, 0x20); //ANAIF_PC_RST = 1
	udelay(1);
	USB3PHY_SETREG(0x102, 0x00); //ANAIF_PC_RST = 0
#endif
	return 0;
}

/* nvt xhci test mode proc */
static int nvt_xhci_test_show(struct seq_file *sfile, void *v)
{
        seq_printf(sfile, "(0) usb 5v power off/on\n");

        return 0;
}

static int nvt_xhci_test_open(struct inode *inode, struct file *file)
{
        return single_open(file, nvt_xhci_test_show, PDE_DATA(inode));
}

static ssize_t nvt_xhci_test_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
{

	struct xhci_hcd *xhci = PDE_DATA(file_inode(file));
	unsigned int value_in;
	char value_str[32] = {'\0'};

	if (copy_from_user(value_str, buffer, count)) {
		return -EFAULT;
	}

	value_str[count] = '\0';
	sscanf(value_str, "%x\n", &value_in);

	switch (value_in) {

		case XHCI_TST_PWR_TOG:
			printk("[XCHI_TST_MODE] 5V power toggle\n");
			if (xhci->power_gpio[0] > 0) {
				gpio_direction_output(xhci->power_gpio[0], !xhci->power_gpio[1]);

				mdelay(200);

				gpio_direction_output(xhci->power_gpio[0], xhci->power_gpio[1]);
			} else { /*No power gpio case*/
			}

			break;
		default:
			printk("[XCHI_TST_MODE] wrong parameters!!!\n");
			break;
	}

	return count;
}

static struct proc_ops nvt_xhci_test_ops = {
	.proc_open   = nvt_xhci_test_open,
	.proc_write  = nvt_xhci_test_write,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int nvt_xhci_proc_init(struct xhci_hcd *xhci)
{
        const char name[16] = "nvt_xhci";
        struct proc_dir_entry *root;
        int ret = 0;
        static int init = 0;

        if (init) {
                return 0;
        }

        root = proc_mkdir(name, NULL);
        if (!root) {
                ret = -ENOMEM;
                goto end;
        }

        nvt_xhci_proc_root = root;

        nvt_xhci_proc_test = proc_create_data("xhci_test_mode", S_IRUGO | S_IXUGO, nvt_xhci_proc_root, &nvt_xhci_test_ops, xhci);
        if (!nvt_xhci_proc_test) {
                printk("create proc node 'nvt_xhci_proc_test' failed!\n");
                ret = -EINVAL;
                goto err1;
        }

        init = 1;

        return ret;

err1:
        proc_remove(nvt_xhci_proc_root);

end:
	return ret;

}

void nvt_xhci_proc_exit(void)
{

	if (nvt_xhci_proc_root) {
		static int init = 0;

		if (init) {
			return;
		}

		proc_remove(nvt_xhci_proc_root);
		nvt_xhci_proc_root = NULL;
		init = 1;
	}
}

int xhci_nvt_host_init(struct usb_hcd *hcd)
{
	struct xhci_hcd  *xhci = hcd_to_xhci(hcd);
	struct device *dev = hcd->self.controller;
	void __iomem     *regs;
	struct clk       *clk;
	unsigned long    temp;
#ifndef CONFIG_NVT_FPGA_EMULATION
	UINT8 USB_SATA_Internal_12K_resistor = 8;
	UINT8 tx_term_trim_val = 6;
	UINT8 u3_trim_swctrl = 4;
	UINT8 u3_trim_sqsel = 4;
#endif
	UINT8 u2_trim_swctrl = 4;
	u32 force_hs = 0;
	u32 get_power_gpio[2];
	int power_gpio[2];
	u32 get_tx_swing;
	int tx_swing;
	u32 u2_get_tx_swing;
	int u2_tx_swing;
	u32 u3_channel = 0;
	u32 get_ssc_ofs[3];
	u32 get_ssc_value[3];
	int ssc_ofs[3];
	int ssc_value[3];

	//======================================================
	// [STEP1] parsing device tree
	//======================================================
	if(!of_property_read_u32(dev->of_node, "force-hs", &force_hs)) {
		pr_debug("force-hs!\n");
	}
	xhci->force_hs = force_hs;

	if(!of_property_read_u32(dev->of_node, "channel", &u3_channel)) {
		pr_info("channel = 0x%x\n", u3_channel);
	}
	xhci->u3_channel = u3_channel;

	if (of_property_read_u32_array(dev->of_node, "power_gpio", get_power_gpio, 2)) {
		power_gpio[0] = -1;
		power_gpio[1] = -1;
	} else {
		power_gpio[0] = get_power_gpio[0];
		power_gpio[1] = get_power_gpio[1];

		gpio_direction_output(power_gpio[0], !power_gpio[1]);
		mdelay(200);
	}

	if (of_property_read_u32_array(dev->of_node, "ssc_ofs", get_ssc_ofs, 3)) {
		//get fail, use default
		ssc_ofs[0] = 0x4594;
		ssc_ofs[1] = 0x4590;
		ssc_ofs[2] = 0x458C;
	} else {
		ssc_ofs[0] = get_ssc_ofs[0];
		ssc_ofs[1] = get_ssc_ofs[1];
		ssc_ofs[2] = get_ssc_ofs[2];
	}
	if (of_property_read_u32_array(dev->of_node, "ssc_value", get_ssc_value, 3)) {
		//get fail, use default
		ssc_value[0] = 0xbe;
		ssc_value[1] = 0x6;
		ssc_value[2] = 0x95;
	} else {
		ssc_value[0] = get_ssc_value[0];
		ssc_value[1] = get_ssc_value[1];
		ssc_value[2] = get_ssc_value[2];
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

	//======================================================
        // [STEP2] PHY setting : Get USB30 trim
        //======================================================
	#ifndef CONFIG_NVT_FPGA_EMULATION
	#if 1
	{
		u16 data=0;
		u32 trim;
		u32 addr;

		trim = efuse_readParamOps(EFUSE_USBC_TRIM_DATA, &data);
		//trim = 1; // Be used while efuse api not ready
		addr = EFUSE_USBC_TRIM_DATA;

		if(trim == 0) {
			//12K resistor 1<= x <= 20(0x14)
			pr_info("  USB Trim data = 0x%04x\r\n", (int)data);
			USB_SATA_Internal_12K_resistor = data & 0x1F;
			tx_term_trim_val = ((data >> 5) & 0xF);

			//12K
			if (USB_SATA_Internal_12K_resistor > 0x14) {
				pr_info("12K_resistor Trim data error 0x%04x > 20(0x14)\r\n", (int)USB_SATA_Internal_12K_resistor);
				USB_SATA_Internal_12K_resistor = 8;
			} else if (USB_SATA_Internal_12K_resistor < 0x1) {
				pr_info("12K_resistor Trim data error 0x%04x < 1(0x1)\r\n", (int)USB_SATA_Internal_12K_resistor);
				USB_SATA_Internal_12K_resistor = 8;
			} else {
				pr_info("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB_SATA_Internal_12K_resistor);
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
			//RX term use auto calibration on 538
		}else {
			pr_err("[0x%x] data = NULL\n", addr);
		}
	}
	#endif

	xhci->u3_trim_rint_sel = u3_trim_sqsel;
	xhci->tx_term_trim_val = tx_term_trim_val;
	xhci->USB_SATA_Internal_12K_resistor = USB_SATA_Internal_12K_resistor;

	//======================================================
	// [STEP2] PHY setting : USB30 12K & TX trim bits
	//======================================================

	temp = USB3PHY_GETREG(0x102);
	pr_info("U3 internal block reset(0x2408): 0x%lx\n", temp);

	temp = USB3PHY_GETREG(0x115);
        pr_info("U3 RX/TX termination(0x2454): 0x%lx\n", temp);

	// step1: 12K trim set
	USB3PHY_SETREG(0x1A3, 0x60);

	temp = USB3PHY_GETREG(0x1A1);
	temp &= ~(0x80);
	USB3PHY_SETREG(0x1A1 , temp);

	USB3PHY_SETREG(0x1A0, 0x40 + USB_SATA_Internal_12K_resistor);

	// step2: tx trim
	//bit[7:4] :TX_ZTX_CTL force mode
	USB3PHY_SETREG(0x30, 0xF0);
	//0x40[7:4] = tx_term_trim_val[3:0]
	temp = USB3PHY_GETREG(0x10);
	temp &= ~(0xF0);
	temp |= (tx_term_trim_val<<4);
	USB3PHY_SETREG(0x10 , temp);



	//======================================================
	// [STEP2] PHY setting : USB20 swing & sqsel trim bits
	//======================================================
	temp = USB3U2PHY_GETREG(0x06);
	temp &= ~(0x7<<1);
	temp |= (u3_trim_swctrl<<1);
	USB3U2PHY_SETREG(0x06, temp);

	temp = USB3U2PHY_GETREG(0x05);
	temp &= ~(0x7<<2);
	temp |= (u3_trim_sqsel<<2);
	USB3U2PHY_SETREG(0x05, temp);

	//R45 Calibration
	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	//======================================================
	// [STEP2] PHY setting : USB20 Best setting on NT98538
	//======================================================
	//host mode
	USB3U2PHY_SETREG(0x60, 0x01);
	USB3U2PHY_SETREG(0xC, 0x20);
	USB3U2PHY_SETREG(0xB, 0x20);
	USB3U2PHY_SETREG(0x63, 0x60);
	USB3PHY_SETREG(0x56, 0x74);
	USB3U2PHY_SETREG(0x12, 0x31); //RX clk inverse

	//======================================================
	// [STEP2] PHY setting : USB30 Best setting on NT98538
	//======================================================
	USB3PHY_SETREG(0x1BF, 0x40); //EOC RX_ICTRL's offset = 0
	//USB3PHY_SETREG(0x12, 0x28);  //bit[5:4] : TX_AMP_CTL_LFPS[1:0]
	USB3PHY_SETREG(0x14, 0x8A);  //Follow 336 for TX CTS
	USB3PHY_SETREG(0x34, 0xFC);
	USB3PHY_SETREG(0x114, 0x04); //PHY_PWR_MODE_CHG_RDY_RSP = 1.3ms
	USB3PHY_SETREG(0x152, 0x2E); //LB of VBS TUNE FREQ CNT
	USB3PHY_SETREG(0x153, 0x01); //UB of VBS TUNE FREQ CNT
	USB3PHY_SETREG(0x1B0, 0xC0); //EOC 2-data sampler
	USB3PHY_SETREG(0x1B1, 0x91); //LB of EOC THR
	USB3PHY_SETREG(0x1B2, 0x00); //UB of EOC THR
	USB3PHY_SETREG(0x135, 0x88); //SET VGA = FORCE MODE
	USB3PHY_SETREG(0x12A, 0x50); //SET VGA = 5
	USB3PHY_SETREG(0x105, 0x0);  //Remove RX_ZRX OFS
	USB3PHY_SETREG(0x27B, 0x51);
	USB3PHY_SETREG(0x141, 0x51);
	USB3PHY_SETREG(0x140, 0x80);
	USB3PHY_SETREG(0x56, 0x74);  //bit[5:4] = BIAS_VCH_SEL[1:0] = 2'b11
	USB3PHY_SETREG(0x11, 0x11);  //bit[0] TX_PDB
	USB3PHY_SETREG(0x31, 0x1);   //bit[0] SM_TX_PDB
	USB3PHY_SETREG(0x13, 0x14);  //bit[2] TX_DET1_EN force value = 1
	USB3PHY_SETREG(0x102, 0x20); //ANAIF_PC_RST = 1
	udelay(1);
	USB3PHY_SETREG(0x102, 0x00); //ANAIF_PC_RST = 0

        USB3PHY_SETREG(0x114, 0x04);

	#endif //END of trim fetch & PHY best setting

	//======================================================
	// [STEP3] Clock reset
	//======================================================
	// 1. mpll14 500Mhz
	clk = clk_get(NULL, "pll14");
	if (!IS_ERR(clk)) {
		xhci->phyclk = clk;
		clk_set_rate(xhci->phyclk, 500000000);
	} else {
		xhci->phyclk = NULL;
		pr_err("%s: u3phyclk not found\n", __func__);
	}

	// 2. usb3_rstn
	clk = clk_get(dev, dev_name(dev));
	if (!IS_ERR(clk)) {
		clk_prepare_enable(clk);
		clk_disable_unprepare(clk);
		clk_prepare_enable(clk);
	} else
		pr_err("%s: %s not found\n", __func__, dev_name(dev));

	//3. usb3_A_rstn/usb3_glue_rstn
	if (xhci->u3_channel == 1) {
		{
			clk = clk_get(NULL, "u3phy2");
			if (!IS_ERR(clk)) {
				pr_err("%s: u3phy2 reset\n", __func__);
				clk_prepare_enable(clk);
				clk_disable(clk);
				clk_enable(clk);
			} else
				pr_err("%s: u3phy2 not found\n", __func__);
		}

		{
			clk = clk_get(NULL, "u3glue2");
			if (!IS_ERR(clk)) {
				pr_err("%s: u3glue2 reset\n", __func__);
				clk_prepare_enable(clk);
				clk_disable(clk);
				clk_enable(clk);
			} else
				pr_err("%s: u3glue2 not found\n", __func__);
		}


	} else {
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

		{
			clk = clk_get(NULL, "u3glue");
			if (!IS_ERR(clk)) {
				pr_err("%s: u3glue reset\n", __func__);
				clk_prepare_enable(clk);
				clk_disable(clk);
				clk_enable(clk);
			} else
				pr_err("%s: u3glue not found\n", __func__);
		}


	}


	#ifdef CONFIG_POWER_SAVE_OPTION
	nvt_usb3dev_pdphy_reset();
	#endif
	//SSC_period (Fssc:31.6kHz)
	writel(ssc_value[0], cg_regs + ssc_ofs[0]);
	//SSC_mulfac (SSC:0.43%)
	writel(ssc_value[1], cg_regs + ssc_ofs[1]);
	//SSC_en (enable ssc down mode)
	writel(ssc_value[2], cg_regs + ssc_ofs[2]);

	// CLKEN
	if (xhci->u3_channel == 1) {
		temp = *(UINT32 *)(cg_regs+U3_2_CLKEN_IDX);
		temp |= (0x1 <<U3_2_CLKEN_BIT);
		writel(temp, cg_regs+U3_2_CLKEN_IDX);
	} else {
		temp = *(UINT32 *)(cg_regs+U3_CLKEN_IDX);
		temp |= (0x1 <<U3_CLKEN_BIT);
		writel(temp, cg_regs+U3_CLKEN_IDX);
	}

	//Using 0x28 bit7 to check u2phy status
	// 0: u2 phy not reset
	// 1: u2 phy has been reset
	temp = readl(regs + U3_TOP_DBGSEL);

	if((temp & (0x1 << U2RST_FLAG_BIT)) == 0x00)
	{
		nvt_usb3dev_u2phy_reset();
		temp = (0x1 << U2RST_FLAG_BIT);
		writel(temp, regs + U3_TOP_DBGSEL);
	}

	//======================================================
	// [STEP4] NVT controller setting
	//======================================================
	//BUF_RESP_MODE & CHANNEL_DISABLE off
	temp = USB3CTRL_GETREG(U3_TOP_REG1);
	temp &= ~(0x1 << CH_DIS);
	temp |= (0x1 << BUF_RESP_MODE);
	USB3CTRL_SETREG(U3_TOP_REG1, temp);

	temp = readl(regs + U3_TOP_DBGSEL);
	temp |= 0x1;
	writel(temp, regs + U3_TOP_DBGSEL);

	temp = readl(regs + U3_TOP_REG0);
	temp &= ~(0x1 << USB_ID);
	temp |= (0x1 << USB_VBUSVALID);
	//force_hs = 1;
	if (force_hs)
		temp |= (0x1 << USB_FORCE_HS);
	else
		temp &= ~(0x1 << RXTERM_EN_MASK);

	temp &= ~((0x7 << U3PHY_RST_CTL0) | (0x7 << U2PHY_RST_CTL0) | (0x1 << ELAS_BUF_MODE));
	temp |= ((0x1 << U3PHY_RST_CTL2) | (0x1 << U2PHY_RST_CTL2));
	//Support PM mode
	temp |= (0x1 << PM_U3TOU0_PATCH);

	writel(temp, regs + U3_TOP_REG0);
	udelay(100);
	xhci->phy_reg = (unsigned long)regs;

	xhci->ctrl_reg = hcd->regs;

	xhci->power_gpio[0] = power_gpio[0];
	xhci->power_gpio[1] = power_gpio[1];

	xhci->ssc_ofs[0] = ssc_ofs[0];
	xhci->ssc_ofs[1] = ssc_ofs[1];
	xhci->ssc_ofs[2] = ssc_ofs[2];

	xhci->ssc_value[0] = ssc_value[0];
	xhci->ssc_value[1] = ssc_value[1];
	xhci->ssc_value[2] = ssc_value[2];

	if ((u2_tx_swing >= 0) && (u2_tx_swing <= 7)) {
		u2_trim_swctrl = u2_tx_swing;
	}
	//Open SE1 detect
	USB3CTRL_SETREG(0xC0, 0x10);

	nvt_xhci_proc_init(xhci);

	if (power_gpio[0] > 0) {
		gpio_direction_output(power_gpio[0], power_gpio[1]);
		mdelay(200);
	}

	pr_info("%s with %s mode, apb 0x10= 0x%08lX, ver %s\r\n", __func__,
							force_hs ? "HS": "SS",
							temp, DRV_VERSION);

	return 0;
}

void xhci_nvt_host_uninit(struct platform_device *pdev, struct usb_hcd *hcd)
{
	struct xhci_hcd  *xhci = hcd_to_xhci(hcd);
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

	nvt_xhci_proc_exit();
}

