#ifndef __USB_CHRGDET_DRV_H__
#define __USB_CHRGDET_DRV_H__
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/clk.h>

#define MODULE_IRQ_NUM          1
#define MODULE_REG_NUM          3
#define MODULE_CLK_NUM          0

#define WRITE_REG(value, addr)  iowrite32(value, addr)
#define READ_REG(addr)          ioread32(addr)

typedef struct usb3dev_module_info {
	void __iomem *io_addr[MODULE_REG_NUM];
	int iinterrupt_id[MODULE_IRQ_NUM];
	int vbus_gpio_pin;
	int vbus_gpio_pol;
} USB3DEV_MODULE_INFO, *PUSB3DEV_MODULE_INFO;

int nvt_usb3dev_drv_open(PUSB3DEV_MODULE_INFO pmodule_info, unsigned char uc_if);
int nvt_usb3dev_drv_release(PUSB3DEV_MODULE_INFO pmodule_info, unsigned char uc_if);
int nvt_usb3dev_drv_init(PUSB3DEV_MODULE_INFO pmodule_info, struct device *dev);
int nvt_usb3dev_drv_remove(PUSB3DEV_MODULE_INFO pmodule_info, struct device *dev);
//int nvt_usb2dev_drv_suspend(USB3DEV_MODULE_INFO *pmodule_info);
//int nvt_usb2dev_drv_resume(USB3DEV_MODULE_INFO *pmodule_info);
int nvt_usb3dev_drv_ioctl(unsigned char uc_if, USB3DEV_MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg);
void nvt_usb3dev_phy_reset(void);
void nvt_usb3dev_u2phy_reset(void);
void nvt_usb3dev_check_ssc(void);

#ifdef CONFIG_PM
/**
  Support nvt_usb3dev suspend/resume API
  */
extern void usb3dev_suspend(void);
extern void usb3dev_resume(void);

int nvt_usb3dev_drv_suspend(USB3DEV_MODULE_INFO *pmodule_info);
int nvt_usb3dev_drv_resume(USB3DEV_MODULE_INFO *pmodule_info);
#endif

#endif

