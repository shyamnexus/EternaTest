#ifndef __SENPHY_DRV_H__
#define __SENPHY_DRV_H__
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/clk.h>

#define MODULE_IRQ_NUM          0
#define MODULE_REG_NUM          1
#define MODULE_CLK_NUM          0

#define WRITE_REG(value, addr)  iowrite32(value, addr)
#define READ_REG(addr)          ioread32(addr)

typedef struct dsiphy_module_info {
	void __iomem *io_addr[MODULE_REG_NUM];
} DSIPHY_MODULE_INFO, *PDSIPHY_MODULE_INFO;

int nvt_dsiphy_drv_open(PDSIPHY_MODULE_INFO pmodule_info, unsigned char uc_if);
int nvt_dsiphy_drv_release(PDSIPHY_MODULE_INFO pmodule_info, unsigned char uc_if);
int nvt_dsiphy_drv_init(PDSIPHY_MODULE_INFO pmodule_info);
int nvt_dsiphy_drv_remove(PDSIPHY_MODULE_INFO pmodule_info);
int nvt_dsiphy_drv_suspend(DSIPHY_MODULE_INFO *pmodule_info);
int nvt_dsiphy_drv_resume(DSIPHY_MODULE_INFO *pmodule_info);
int nvt_dsiphy_drv_ioctl(unsigned char uc_if, DSIPHY_MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg);
int nvt_dsiphy_drv_write_reg(PDSIPHY_MODULE_INFO pmodule_info, unsigned long addr, unsigned long value);
int nvt_dsiphy_drv_read_reg(PDSIPHY_MODULE_INFO pmodule_info, unsigned long addr);

#endif
