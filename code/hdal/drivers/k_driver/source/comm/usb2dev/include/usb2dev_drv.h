#ifndef __USB_CHRGDET_DRV_H__
#define __USB_CHRGDET_DRV_H__
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/clk.h>

#define MODULE_IRQ_NUM          1
#define MODULE_REG_NUM          1
#define MODULE_CLK_NUM          1

#define WRITE_REG(value, addr)  iowrite32(value, addr)
#define READ_REG(addr)          ioread32(addr)

#define USB_PHY_ADDR_1 0x2F05E1000
#define USB_PHY_SIZE_1 0x1000
#define USB3_PHY_ADDR_1 0x2F05F2000
#define USB3_PHY_SIZE_1 0x1000
#define USB3_UTMI_PHY_ADDR_1 0x2F05F1000
#define USB3_UTMI_PHY_SIZE_1 0x1000
#define USB3_CTRL_ADDR_1 0x2F05F0000
#define USB3_CTRL_SIZE_1 0x100



typedef struct usb2dev_module_info {
	void __iomem *io_addr[MODULE_REG_NUM];
	void __iomem *io_phy_addr[MODULE_REG_NUM];
	void __iomem *io_u3phy_addr[MODULE_REG_NUM];
	void __iomem *io_u3utmi_phy_addr[MODULE_REG_NUM];
	void __iomem *io_u3_ctrl_addr[MODULE_REG_NUM];
	int iinterrupt_id[MODULE_IRQ_NUM];
	int vbus_gpio_pin;
	int vbus_gpio_pol;
	struct clk *pclk[MODULE_CLK_NUM];
	unsigned char *ctrl_buff;
} USB2DEV_MODULE_INFO, *PUSB2DEV_MODULE_INFO;

int nvt_usb2dev_drv_open(PUSB2DEV_MODULE_INFO pmodule_info, unsigned char uc_if);
int nvt_usb2dev_drv_release(PUSB2DEV_MODULE_INFO pmodule_info, unsigned char uc_if);
int nvt_usb2dev_drv_init(PUSB2DEV_MODULE_INFO pmodule_info);
int nvt_usb2dev_drv_remove(PUSB2DEV_MODULE_INFO pmodule_info);
//int nvt_usb2dev_drv_suspend(USB2DEV_MODULE_INFO *pmodule_info);
//int nvt_usb2dev_drv_resume(USB2DEV_MODULE_INFO *pmodule_info);
int nvt_usb2dev_drv_ioctl(unsigned char uc_if, USB2DEV_MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg);
#endif

