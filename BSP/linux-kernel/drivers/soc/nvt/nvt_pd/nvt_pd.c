#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <plat/hardware.h>
#include <linux/time.h>
#include <linux/io.h>
//#include <nvt/nvt_type.h>
//#include "nvt_pd.h"
#include <linux/delay.h>

#if defined(__FREERTOS)
#include "rcw_macro.h"
#include "kwrap/type.h"
#else
//#include <mach/rcw_macro.h>
#include <linux/soc/nvt/rcw_macro.h>
#endif


#define PD_DRV_VERSION "1.00.000"
#define USB_PD_DONE_BIT 7

static void __iomem *IOADDR_USB3_PHY_REG_BASE=NULL;
static void usb_pd(void)
{
	if(!IOADDR_USB3_PHY_REG_BASE) {
		IOADDR_USB3_PHY_REG_BASE = ioremap(NVT_USB3_BASE_PHYS, 0x3000);
	}

	if (INW(IOADDR_USB3_PHY_REG_BASE + 0x1000) & (0x1 << USB_PD_DONE_BIT)) { //phy reg 0x0[7] will be used to determined if USB power save mode already done or not.
		//already enter usb_pd
	} else {
		printk("do usb_pd\n");
		OUTW(IOADDR_USB3_PHY_REG_BASE + 0x10D0, 0x000000FF);
		mdelay(10);
		OUTW(IOADDR_USB3_PHY_REG_BASE + 0x10D4, 0x00000034);
		mdelay(10);
		OUTW(IOADDR_USB3_PHY_REG_BASE + 0x1008, 0x00000014);
		mdelay(10);
		OUTW(IOADDR_USB3_PHY_REG_BASE + 0x1000, 0x00000088);
		mdelay(10);
		OUTW(IOADDR_USB3_PHY_REG_BASE + 0x1030, 0x00000035);
		mdelay(10);
		OUTW(IOADDR_USB3_PHY_REG_BASE + 0x102C, 0x000000FF);
		mdelay(10);
		OUTW(IOADDR_USB3_PHY_REG_BASE + 0x1028, 0x00000020);
		mdelay(10);
		OUTW(IOADDR_USB3_PHY_REG_BASE + 0x1024, 0x00000093);
	}
	iounmap((volatile void __iomem *)IOADDR_USB3_PHY_REG_BASE);
	IOADDR_USB3_PHY_REG_BASE=NULL;
}

#define USB3PDPHY_SETREG(ofs,value)   writel((value), (volatile void __iomem *)(IOADDR_USB3_PHY_REG_BASE+((ofs))))
#define USB3PDPHY_GETREG(ofs)         readl((volatile void __iomem *)(IOADDR_USB3_PHY_REG_BASE+((ofs))))

static void usb3_pd(void)
{
	unsigned long    temp;
	if(!IOADDR_USB3_PHY_REG_BASE) {
		IOADDR_USB3_PHY_REG_BASE = ioremap(NVT_USB3_BASE_PHYS, 0x3000);
	}
	printk("do usb3_pd\n");	
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

	iounmap((volatile void __iomem *)IOADDR_USB3_PHY_REG_BASE);
	IOADDR_USB3_PHY_REG_BASE=NULL;
}

int __init nvt_pd_init(void)
{
	int ret = 0;
	printk("NVT_PD version:%s\n", PD_DRV_VERSION);
	usb_pd();
	usb3_pd();
	return ret;
}

void __exit nvt_pd_exit(void)
{
}

module_init(nvt_pd_init);
module_exit(nvt_pd_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("nvt pd");
MODULE_LICENSE("Proprietary");
