#ifndef __CTL_VIE_MODULE_DBG_H_
#define __CTL_VIE_MODULE_DBG_H_

#define __MODULE__ ctl_vie
#define __DBGLVL__ 5          // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__ "*"      //*=All

#include "kwrap/debug.h"
#include "kwrap/error_no.h"
#include <kwrap/spinlock.h>
#include <kwrap/stdio.h>
#include "comm/util/log.h"

#define debug_msg			vk_printk

typedef enum _CTL_VIE_DBG_LVL {
	CTL_VIE_DBG_LVL_NONE = 0,
	CTL_VIE_DBG_LVL_ERR,
	CTL_VIE_DBG_LVL_WRN,
	CTL_VIE_DBG_LVL_UNIT,
	CTL_VIE_DBG_LVL_IND,
	CTL_VIE_DBG_LVL_FUNC,
	CTL_VIE_DBG_LVL_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_DBG_LVL)
} CTL_VIE_DBG_LVL;

extern CTL_VIE_DBG_LVL ctl_vie_dbg_rt_lvl;

#if defined(__KERNEL__)
#define CTL_VIE_LOG_BUFFER          1
#else
#define CTL_VIE_LOG_BUFFER          0
#endif

#if CTL_VIE_LOG_BUFFER
#define ctl_vie_dbg_err(fmt, args...) \
	printm2("[ctl_vie]ERR: "   fmt , ##args);    \
	{ if (ctl_vie_dbg_rt_lvl >= CTL_VIE_DBG_LVL_ERR) { DBG_ERR(fmt, ##args);  }}
#define ctl_vie_dbg_wrn(fmt, args...) \
	printm2("[ctl_vie]WRN: "   fmt , ##args);    \
	{ if (ctl_vie_dbg_rt_lvl >= CTL_VIE_DBG_LVL_WRN) { DBG_WRN(fmt, ##args);  }}
#define ctl_vie_dbg_ind(fmt, args...) \
	printm2("[ctl_vie]IND: "   fmt , ##args);    \
	{ if (ctl_vie_dbg_rt_lvl >= CTL_VIE_DBG_LVL_IND) { DBG_DUMP("[ctl_vie]IND: " fmt, ##args); }}
#define ctl_vie_dbg_func(fmt, args...) \
	{ if (ctl_vie_dbg_rt_lvl >= CTL_VIE_DBG_LVL_FUNC) { printm2("[ctl_vie]FUNC: "   fmt , ##args); }}	    \
	{ if (ctl_vie_dbg_rt_lvl >= CTL_VIE_DBG_LVL_FUNC) { DBG_DUMP("[ctl_vie]FUNC: " fmt, ##args); }}
#else
#define ctl_vie_dbg_err(fmt, args...) \
	{ if (ctl_vie_dbg_rt_lvl >= CTL_VIE_DBG_LVL_ERR) { DBG_ERR(fmt, ##args);  }}
#define ctl_vie_dbg_wrn(fmt, args...) \
	{ if (ctl_vie_dbg_rt_lvl >= CTL_VIE_DBG_LVL_WRN) { DBG_WRN(fmt, ##args);  }}
#define ctl_vie_dbg_ind(fmt, args...) \
	{ if (ctl_vie_dbg_rt_lvl >= CTL_VIE_DBG_LVL_IND) { DBG_DUMP("[ctl_vie]IND: "fmt, ##args); }}
#define ctl_vie_dbg_func(fmt, args...) \
	{ if (ctl_vie_dbg_rt_lvl >= CTL_VIE_DBG_LVL_FUNC) { DBG_DUMP("[ctl_vie]FUNC: "fmt, ##args); }}
#endif
#define ctl_vie_dbg_dump(fmt, args...)   DBG_DUMP(fmt, ##args);

#endif


