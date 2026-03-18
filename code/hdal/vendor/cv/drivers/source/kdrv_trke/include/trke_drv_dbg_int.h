#ifndef _TRKE_DRV_DBG_INT_H_
#define _TRKE_DRV_DBG_INT_H_

#if defined(__LINUX)
#include "comm/util/log.h"
#define trke_pr_warn pr_warn
#define trke_printm printm
extern void trke_drv_proc_seq_printf(void *sfile, const char *fmt, ...);
#elif defined(__FREERTOS)
#include <kwrap/debug.h>
#include <stdarg.h>
#define trke_pr_warn debug_msg_isr
#define trke_drv_proc_seq_printf(sfile, fmt, args...) trke_pr_warn(fmt, ##args)
#define trke_printm(module, fmt, ...)
#endif

#define TRKE_DRV_DBG_FATAL     0
#define TRKE_DRV_DBG_ERR       1
#define TRKE_DRV_DBG_WRN       2
#define TRKE_DRV_DBG_MSG       3
#define TRKE_DRV_DBG_IND       4
#define TRKE_DRV_DBG_FUNC      5

extern unsigned int kdrv_trke_debug_level;
#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (TRKE_DRV_DBG_##level <= kdrv_trke_debug_level) { \
		trke_pr_warn("%s:" fmt, __func__, ##args); \
	} \
} while (0)

#define TRKE_DRV_LOG_TAG "VD"
#define nvt_trke_dbg_log(fmt, args...) \
do { \
	trke_pr_warn(fmt, ##args); \
	trke_printm(TRKE_DRV_LOG_TAG, fmt, ##args); \
} while (0)

#define trke_drv_uti_log trke_drv_proc_seq_printf
#define trke_drv_log nvt_trke_dbg_log
#define trke_drv_dump(fmt, args...) trke_pr_warn(fmt, ##args)
#define trke_drv_err(fmt, args...) nvt_dbg_level(ERR, fmt, ##args)
#define trke_drv_warn(fmt, args...) nvt_dbg_level(WRN, fmt, ##args)
#define trke_drv_info(fmt, args...) nvt_dbg_level(IND, fmt, ##args)

#define trke_drv_dbgd(x)   trke_pr_warn("%s=%d\n", #x, x)
#define trke_drv_dbgh(x)   trke_pr_warn("%s=0x%.8x\n", #x, x)

#endif  //_TRKE_DRV_DBG_INT_H_
