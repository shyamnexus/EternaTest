#ifndef _MD_DRV_DBG_INT_H_
#define _MD_DRV_DBG_INT_H_

#if defined(__LINUX)
#include "comm/util/log.h"
#define md_pr_warn pr_warn
#define md_printm printm
extern void md_drv_proc_seq_printf(void *sfile, const char *fmt, ...);
#elif defined(__FREERTOS)
#include <kwrap/debug.h>
#include <stdarg.h>
#define md_pr_warn debug_msg_isr
#define md_drv_proc_seq_printf(sfile, fmt, args...) md_pr_warn(fmt, ##args)
#define md_printm(module, fmt, ...)
#endif

#define MD_DRV_DBG_FATAL     0
#define MD_DRV_DBG_ERR       1
#define MD_DRV_DBG_WRN       2
#define MD_DRV_DBG_MSG       3
#define MD_DRV_DBG_IND       4
#define MD_DRV_DBG_FUNC      5

extern unsigned int kdrv_md_debug_level;
#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (MD_DRV_DBG_##level <= kdrv_md_debug_level) { \
		md_pr_warn("%s:" fmt, __func__, ##args); \
	} \
} while (0)

#define MD_DRV_LOG_TAG "VD"
#define nvt_md_dbg_log(fmt, args...) \
do { \
	md_pr_warn(fmt, ##args); \
	md_printm(MD_DRV_LOG_TAG, fmt, ##args); \
} while (0)

#define md_drv_uti_log md_drv_proc_seq_printf
#define md_drv_log nvt_md_dbg_log
#define md_drv_dump(fmt, args...) md_pr_warn(fmt, ##args)
#define md_drv_err(fmt, args...) nvt_dbg_level(ERR, fmt, ##args)
#define md_drv_warn(fmt, args...) nvt_dbg_level(WRN, fmt, ##args)
#define md_drv_info(fmt, args...) nvt_dbg_level(IND, fmt, ##args)

#define md_drv_dbgd(x)   md_pr_warn("%s=%d\n", #x, x)
#define md_drv_dbgh(x)   md_pr_warn("%s=0x%.8x\n", #x, x)

#endif  //_MD_DRV_DBG_INT_H_
