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

static void __iomem *IOADDR_USB_PHY_REG_BASE=NULL;
static void usb_pd(void)
{
	if(!IOADDR_USB_PHY_REG_BASE) {
		IOADDR_USB_PHY_REG_BASE = ioremap(NVT_USB1_BASE_PHYS, 0x3000);
	}

	if (INW(IOADDR_USB_PHY_REG_BASE + 0x1000) & (0x1 << USB_PD_DONE_BIT)) { //phy reg 0x0[7] will be used to determined if USB power save mode already done or not.
		//already enter usb_pd
	} else {
		printk("do usb_pd\n");		
		OUTW(IOADDR_USB_PHY_REG_BASE + 0x10D4, 0x000000B4);
		OUTW(IOADDR_USB_PHY_REG_BASE + 0x10D0, 0x000000FF);
		OUTW(IOADDR_USB_PHY_REG_BASE + 0x1008, 0x00000014);
		OUTW(IOADDR_USB_PHY_REG_BASE + 0x1000, 0x0000008A);
		OUTW(IOADDR_USB_PHY_REG_BASE + 0x1030, 0x00000013);
		OUTW(IOADDR_USB_PHY_REG_BASE + 0x102C, 0x000000FF);
		OUTW(IOADDR_USB_PHY_REG_BASE + 0x1028, 0x00000020);
		OUTW(IOADDR_USB_PHY_REG_BASE + 0x1024, 0x000000B3);		
		OUTW(IOADDR_USB_PHY_REG_BASE + 0x1034, 0x00000090);
	}
	iounmap((volatile void __iomem *)IOADDR_USB_PHY_REG_BASE);
	IOADDR_USB_PHY_REG_BASE=NULL;
}
int __init nvt_pd_init(void)
{
	int ret = 0;
	printk("NVT_PD version:%s\n", PD_DRV_VERSION);
	usb_pd();
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
