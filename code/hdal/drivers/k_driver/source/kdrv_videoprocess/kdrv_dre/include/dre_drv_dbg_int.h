#ifndef _DRE_DRV_DBG_INT_H_
#define _DRE_DRV_DBG_INT_H_

#if defined(__LINUX)
#include "comm/util/log.h"
#define dre_pr_warn pr_warn
#define dre_printm printm
extern void dre_drv_proc_seq_printf(void *sfile, const char *fmt, ...);
#elif defined(__FREERTOS)
#include <kwrap/debug.h>
#include <stdarg.h>
#define dre_pr_warn debug_msg_isr
#define dre_drv_proc_seq_printf(sfile, fmt, args...) dre_pr_warn(fmt, ##args)
#define dre_printm(module, fmt, ...)
#endif

#define DRE_DRV_DBG_FATAL     0
#define DRE_DRV_DBG_ERR       1
#define DRE_DRV_DBG_WRN       2
#define DRE_DRV_DBG_MSG       3
#define DRE_DRV_DBG_IND       4
#define DRE_DRV_DBG_FUNC      5

extern unsigned int kdrv_dre_debug_level;
#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (DRE_DRV_DBG_##level <= kdrv_dre_debug_level) { \
		dre_pr_warn("%s:" fmt, __func__, ##args); \
	} \
} while (0)

#define DRE_DRV_LOG_TAG "VD"
#define nvt_dre_dbg_log(fmt, args...) \
do { \
	dre_pr_warn(fmt, ##args); \
	dre_printm(DRE_DRV_LOG_TAG, fmt, ##args); \
} while (0)

#define dre_drv_uti_log dre_drv_proc_seq_printf
#define dre_drv_log nvt_dre_dbg_log
#define dre_drv_dump(fmt, args...) dre_pr_warn(fmt, ##args)
#define dre_drv_err(fmt, args...) nvt_dbg_level(ERR, fmt, ##args)
#define dre_drv_warn(fmt, args...) nvt_dbg_level(WRN, fmt, ##args)
#define dre_drv_info(fmt, args...) nvt_dbg_level(IND, fmt, ##args)

#define dre_drv_dbgd(x)   dre_pr_warn("%s=%d\n", #x, x)
#define dre_drv_dbgh(x)   dre_pr_warn("%s=0x%.8x\n", #x, x)

#endif  //_DRE_DRV_DBG_INT_H_
