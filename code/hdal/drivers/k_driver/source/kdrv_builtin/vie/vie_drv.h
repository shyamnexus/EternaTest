#ifndef __VIE_DRV_H__
#define __VIE_DRV_H__

#if defined (__LINUX)
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/completion.h>
#endif
#include <kwrap/spinlock.h>
#include <kwrap/semaphore.h>

#define MODULE_IRQ_NUM          1
#define MODULE_REG_NUM          1
#define MODULE_CLK_NUM          1

#define WRITE_REG(Value, Addr)  iowrite32(Value, Addr)
#define READ_REG(Addr)          ioread32(Addr)

typedef struct vie_module_info {
	struct completion vie_completion;
	struct vk_semaphore vie_sem;
	struct clk* vie_clk[MODULE_CLK_NUM];
	struct clk* vie_pxclkA[MODULE_CLK_NUM];
	struct clk* vie_pxclkB[MODULE_CLK_NUM];
	struct clk* vie_pxclkpad;
	struct clk* vie_sram;
	struct tasklet_struct vie_tasklet;
	void __iomem* io_addr[MODULE_REG_NUM];
    int iinterrupt_id[MODULE_IRQ_NUM];
    wait_queue_head_t vie_wait_queue;
	vk_spinlock_t vie_spinlock;
}VIE_MODULE_INFO, *PVIE_MODULE_INFO;

int nvt_vie_drv_open(PVIE_MODULE_INFO pmodule_info, unsigned char ucIF);
int nvt_vie_drv_release(PVIE_MODULE_INFO pmodule_info, unsigned char ucIF);
int nvt_vie_drv_init(PVIE_MODULE_INFO pmodule_info, unsigned char ucIF);
int nvt_vie_drv_remove(PVIE_MODULE_INFO pmodule_info);
int nvt_vie_drv_suspend(VIE_MODULE_INFO* pmodule_info);
int nvt_vie_drv_resume(VIE_MODULE_INFO* pmodule_info);
int nvt_vie_drv_write_reg(PVIE_MODULE_INFO pmodule_info, unsigned long addr, unsigned long value);
int nvt_vie_drv_read_reg(PVIE_MODULE_INFO pmodule_info, unsigned long addr);
#endif

