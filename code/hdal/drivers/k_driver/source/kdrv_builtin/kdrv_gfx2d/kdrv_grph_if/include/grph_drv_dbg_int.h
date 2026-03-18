#ifndef _GRAPH_DRV_DBG_INT_H_
#define _GRAPH_DRV_DBG_INT_H_

#if defined(__LINUX)
#include "comm/util/log.h"
#define graph_pr_warn pr_warn
#define graph_printm(module, fmt, ...)
extern void graph_drv_proc_seq_printf(void *sfile, const char *fmt, ...);
#elif defined(__FREERTOS)
#include <kwrap/debug.h>
#include <stdarg.h>
#define graph_pr_warn debug_msg_isr
#define graph_drv_proc_seq_printf(sfile, fmt, args...) graph_pr_warn(fmt, ##args)
#define graph_printm(module, fmt, ...)
#endif

#define GRAPH_DRV_DBG_FATAL     0
#define GRAPH_DRV_DBG_ERR       1
#define GRAPH_DRV_DBG_WRN       2
#define GRAPH_DRV_DBG_MSG       3
#define GRAPH_DRV_DBG_IND       4
#define GRAPH_DRV_DBG_FUNC      5

extern unsigned int kdrv_graph_debug_level;
#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (GRAPH_DRV_DBG_##level <= kdrv_graph_debug_level) { \
		graph_pr_warn("%s:" fmt, __func__, ##args); \
	} \
} while (0)

#define GRAPH_DRV_LOG_TAG "GD"
#define nvt_graph_dbg_log(fmt, args...) \
do { \
	graph_pr_warn(fmt, ##args); \
	graph_printm(GRAPH_DRV_LOG_TAG, fmt, ##args); \
} while (0)

#define graph_drv_uti_log graph_drv_proc_seq_printf
#define graph_drv_log nvt_graph_dbg_log
#define graph_drv_dump(fmt, args...) graph_pr_warn(fmt, ##args)
#define graph_drv_err(fmt, args...) nvt_dbg_level(ERR, fmt, ##args)
#define graph_drv_warn(fmt, args...) nvt_dbg_level(WRN, fmt, ##args)
#define graph_drv_info(fmt, args...) nvt_dbg_level(IND, fmt, ##args)

#define graph_drv_dbgd(x)   graph_pr_warn("%s=%d\n", #x, x)
#define graph_drv_dbgh(x)   graph_pr_warn("%s=0x%.8x\n", #x, x)

#endif  //_GRAPH_DRV_DBG_INT_H_
