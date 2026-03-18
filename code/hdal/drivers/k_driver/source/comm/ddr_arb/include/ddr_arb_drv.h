#ifndef __MODULE_DRV_H__
#define __MODULE_DRV_H__
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include "kwrap/task.h"
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/clk.h>
#include <kwrap/nvt_type.h>

#define MODULE_IRQ_NUM          1
#define MODULE_REG_NUM          6
#define MODULE_CLK_NUM          1

#define WRITE_REG(VALUE, ADDR)  iowrite32(VALUE, ADDR)
#define READ_REG(ADDR)          ioread32(ADDR)


typedef struct _DBGUT_CTRL_DMA_USAGE {
	UINT32			interval_ms; ///< time Interval in Ms
	UINT32			cycle_us;
	UINT32			usage;        ///< reported usage, by acc_usage/nResetCnt
	UINT32			efficiency;
	INT32			is_start;
	UINT32          reset_cnt;     ///< when remain_cnt is 0, reset remain_cnt to nResetCnt
	INT32           remain_cnt;     ///< if <10 ms 1 time count 1 hits, >10ms 1 time count 10 hits
	UINT64          acc_usage;     ///< account for sum of 10 times usage
	UINT64          acc_efficiency;
	UINT32          interval_cnt;  ///< a count for each interval occured

	INT32           monitor_is_start[4];
	UINT64          acc_monitor[4];
	UINT32          monitor[4];
	UINT32			mon_ch[4];
} DBGUT_CTRL_DMA_USAGE;

typedef struct module_info {
	struct completion xxx_completion;
	struct semaphore xxx_sem;
	struct clk *pclk[MODULE_CLK_NUM];
	struct tasklet_struct xxx_tasklet;
	void __iomem *io_addr[MODULE_REG_NUM];
	int iinterrupt_id[MODULE_IRQ_NUM];
	wait_queue_head_t xxx_wait_queue;
	spinlock_t xxx_spinlock;


	// monitor
	FLGPTN nvt_ddr_proc_flag_id[1];
	THREAD_HANDLE nvt_ddr_proc_tsk_id[1];
	DBGUT_CTRL_DMA_USAGE nvt_ddr_proc_cfg[1];
	UINT64 data_cnt[1][4];
} MODULE_INFO, *PMODULE_INFO;

int nvt_ddr_arb_drv_open(PMODULE_INFO pmodule_info, unsigned char if_id);
int nvt_ddr_arb_drv_release(PMODULE_INFO pmodule_info, unsigned char if_id);
int nvt_ddr_arb_drv_init(PMODULE_INFO pmodule_info);
int nvt_ddr_arb_drv_remove(PMODULE_INFO pmodule_info);
int nvt_ddr_arb_drv_suspend(MODULE_INFO *pmodule_info);
int nvt_ddr_arb_drv_resume(MODULE_INFO *pmodule_info);
int nvt_ddr_arb_drv_ioctl(unsigned char if_id, MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long argc);
//int nvt_ddr_arb_drv_write_reg(PMODULE_INFO pmodule_info, unsigned long addr, unsigned long value);
//int nvt_ddr_arb_drv_read_reg(PMODULE_INFO pmodule_info, unsigned long addr);
#endif

