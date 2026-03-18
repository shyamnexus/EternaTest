#ifndef _IVE_DRV_DBG_INT_H_
#define _IVE_DRV_DBG_INT_H_

#if defined(__LINUX)
#include "comm/util/log.h"
#define ive_pr_warn pr_warn
#define ive_printm printm
extern void ive_drv_proc_seq_printf(void *sfile, const char *fmt, ...);
#elif defined(__FREERTOS)
#include <kwrap/debug.h>
#include <stdarg.h>
#define ive_pr_warn debug_msg_isr
#define ive_drv_proc_seq_printf(sfile, fmt, args...) ive_pr_warn(fmt, ##args)
#define ive_printm(module, fmt, ...)
#endif

#define IVE_DRV_DBG_FATAL     0
#define IVE_DRV_DBG_ERR       1
#define IVE_DRV_DBG_WRN       2
#define IVE_DRV_DBG_MSG       3
#define IVE_DRV_DBG_IND       4
#define IVE_DRV_DBG_FUNC      5

extern unsigned int kdrv_ive_debug_level;
#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (IVE_DRV_DBG_##level <= kdrv_ive_debug_level) { \
		ive_pr_warn("%s:" fmt, __func__, ##args); \
	} \
} while (0)

#define IVE_DRV_LOG_TAG "VD"
#define nvt_ive_dbg_log(fmt, args...) \
do { \
	ive_pr_warn(fmt, ##args); \
	ive_printm(IVE_DRV_LOG_TAG, fmt, ##args); \
} while (0)

#define ive_drv_uti_log ive_drv_proc_seq_printf
#define ive_drv_log nvt_ive_dbg_log
#define ive_drv_dump(fmt, args...) ive_pr_warn(fmt, ##args)
#define ive_drv_err(fmt, args...) nvt_dbg_level(ERR, fmt, ##args)
#define ive_drv_warn(fmt, args...) nvt_dbg_level(WRN, fmt, ##args)
#define ive_drv_info(fmt, args...) nvt_dbg_level(IND, fmt, ##args)

#define ive_drv_dbgd(x)   ive_pr_warn("%s=%d\n", #x, x)
#define ive_drv_dbgh(x)   ive_pr_warn("%s=0x%.8x\n", #x, x)
#endif  //_IVE_DRV_DBG_INT_H_
