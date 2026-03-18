#ifndef _ISE_DRV_DBG_INT_H_
#define _ISE_DRV_DBG_INT_H_

#if defined(__LINUX)
#include "comm/util/log.h"
#define ise_pr_warn pr_warn
#define ise_printm printm
extern void ise_drv_proc_seq_printf(void *sfile, const char *fmt, ...);
#elif defined(__FREERTOS)
#include <kwrap/debug.h>
#include <stdarg.h>
#define ise_pr_warn debug_msg_isr
#define ise_drv_proc_seq_printf(sfile, fmt, args...) ise_pr_warn(fmt, ##args)
#define ise_printm(module, fmt, ...)
#endif

#define ISE_DRV_DBG_FATAL     0
#define ISE_DRV_DBG_ERR       1
#define ISE_DRV_DBG_WRN       2
#define ISE_DRV_DBG_MSG       3
#define ISE_DRV_DBG_IND       4
#define ISE_DRV_DBG_FUNC      5

extern unsigned int kdrv_ise_debug_level;
#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (ISE_DRV_DBG_##level <= kdrv_ise_debug_level) { \
		ise_pr_warn("%s:" fmt, __func__, ##args); \
	} \
} while (0)

#define ISE_DRV_LOG_TAG "ID"
#define nvt_ise_dbg_log(fmt, args...) \
do { \
	ise_pr_warn(fmt, ##args); \
	ise_printm(ISE_DRV_LOG_TAG, fmt, ##args); \
} while (0)

#define ise_drv_uti_log ise_drv_proc_seq_printf
#define ise_drv_log nvt_ise_dbg_log
#define ise_drv_dump(fmt, args...) ise_pr_warn(fmt, ##args)
#define ise_drv_err(fmt, args...) nvt_dbg_level(ERR, fmt, ##args)
#define ise_drv_warn(fmt, args...) nvt_dbg_level(WRN, fmt, ##args)
#define ise_drv_info(fmt, args...) nvt_dbg_level(IND, fmt, ##args)

#define ise_drv_dbgd(x)   ise_pr_warn("%s=%d\n", #x, x)
#define ise_drv_dbgh(x)   ise_pr_warn("%s=0x%.8x\n", #x, x)

#endif  //_ISE_DRV_DBG_INT_H_
