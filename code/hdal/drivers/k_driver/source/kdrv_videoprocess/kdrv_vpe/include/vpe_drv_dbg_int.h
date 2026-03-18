#ifndef _VPE_DRV_DBG_INT_H_
#define _VPE_DRV_DBG_INT_H_

#if defined(__LINUX)
#include "comm/util/log.h"
#define vpe_pr_warn pr_warn
#define vpe_printm printm
extern void vpe_drv_proc_seq_printf(void *sfile, const char *fmt, ...);
#elif defined(__FREERTOS)
#include <kwrap/debug.h>
#include <stdarg.h>
#define vpe_pr_warn debug_msg_isr
#define vpe_drv_proc_seq_printf(sfile, fmt, args...) vpe_pr_warn(fmt, ##args)
#define vpe_printm(module, fmt, ...)
#endif

#define VPE_DRV_DBG_FATAL     0
#define VPE_DRV_DBG_ERR       1
#define VPE_DRV_DBG_WRN       2
#define VPE_DRV_DBG_MSG       3
#define VPE_DRV_DBG_IND       4
#define VPE_DRV_DBG_FUNC      5

extern unsigned int kdrv_vpe_debug_level;
#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (VPE_DRV_DBG_##level <= kdrv_vpe_debug_level) { \
		vpe_pr_warn("%s:" fmt, __func__, ##args); \
	} \
} while (0)

#define VPE_DRV_LOG_TAG "VD"
#define nvt_vpe_dbg_log(fmt, args...) \
do { \
	vpe_pr_warn(fmt, ##args); \
	vpe_printm(VPE_DRV_LOG_TAG, fmt, ##args); \
} while (0)

#define vpe_drv_uti_log vpe_drv_proc_seq_printf
#define vpe_drv_log nvt_vpe_dbg_log
#define vpe_drv_dump(fmt, args...) vpe_pr_warn(fmt, ##args)
#define vpe_drv_err(fmt, args...) nvt_dbg_level(ERR, fmt, ##args)
#define vpe_drv_warn(fmt, args...) nvt_dbg_level(WRN, fmt, ##args)
#define vpe_drv_info(fmt, args...) nvt_dbg_level(IND, fmt, ##args)

#define vpe_drv_dbgd(x)   vpe_pr_warn("%s=%d\n", #x, x)
#define vpe_drv_dbgh(x)   vpe_pr_warn("%s=0x%.8x\n", #x, x)
#endif  //_VPE_DRV_DBG_INT_H_
