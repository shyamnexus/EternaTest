#ifndef __MODULE_DBG_H_
#define __MODULE_DBG_H_

#define __MODULE__	ctl_dre
#define __DBGLVL__	8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#include "kwrap/debug.h"
#include "kwrap/perf.h"
#include "comm/hwclock.h"
#include "comm/util/log.h"

extern unsigned int ctl_dre_debug_level;

typedef enum _CTL_DRE_DBG_LVL {
	CTL_DRE_DBG_LVL_NONE = 0,
	CTL_DRE_DBG_LVL_ERR,
	CTL_DRE_DBG_LVL_WRN,
	CTL_DRE_DBG_LVL_UNIT,
	CTL_DRE_DBG_LVL_IND,
	CTL_DRE_DBG_LVL_FUNC,
	CTL_DRE_DBG_LVL_MAX,
	ENUM_DUMMY4WORD(CTL_DRE_DBG_LVL)
} CTL_DRE_DBG_LVL;

#if defined(__KERNEL__)
#define CTL_DRE_LOG_BUFFER          1
#else
#define CTL_DRE_LOG_BUFFER          0
#endif

#if CTL_DRE_LOG_BUFFER
#define ctl_dre_dbg_err(fmt, args...) \
	printm2("[ctl_dre]ERR: "   fmt , ##args);    \
	{ if (ctl_dre_debug_level >= CTL_DRE_DBG_LVL_ERR) { DBG_ERR(fmt, ##args);  }}
#define ctl_dre_dbg_wrn(fmt, args...) \
	printm2("[ctl_dre]WRN: "   fmt , ##args);    \
	{ if (ctl_dre_debug_level >= CTL_DRE_DBG_LVL_WRN) { DBG_WRN(fmt, ##args);  }}
#define ctl_dre_dbg_ind(fmt, args...) \
	printm2("[ctl_dre]IND: "   fmt , ##args);    \
	{ if (ctl_dre_debug_level >= CTL_DRE_DBG_LVL_IND) { DBG_DUMP("[ctl_dre]IND: " fmt, ##args); }}
#define ctl_dre_dbg_func(fmt, args...) \
	{ if (ctl_dre_debug_level >= CTL_DRE_DBG_LVL_FUNC) { printm2("[ctl_dre]FUNC: "  fmt , ##args); }}	\
	{ if (ctl_dre_debug_level >= CTL_DRE_DBG_LVL_FUNC) { DBG_DUMP("[ctl_dre]FUNC: " fmt, ##args); }}
#else
#define ctl_dre_dbg_err(fmt, args...) \
	{ if (ctl_dre_debug_level >= CTL_DRE_DBG_LVL_ERR) { DBG_ERR(fmt, ##args);  }}
#define ctl_dre_dbg_wrn(fmt, args...) \
	{ if (ctl_dre_debug_level >= CTL_DRE_DBG_LVL_WRN) { DBG_WRN(fmt, ##args);  }}
#define ctl_dre_dbg_ind(fmt, args...) \
	{ if (ctl_dre_debug_level >= CTL_DRE_DBG_LVL_IND) { DBG_DUMP("[ctl_dre]IND: " fmt, ##args); }}
#define ctl_dre_dbg_func(fmt, args...) \
	{ if (ctl_dre_debug_level >= CTL_DRE_DBG_LVL_FUNC) { DBG_DUMP("[ctl_dre]FUNC: " fmt, ##args); }}
#endif
#define ctl_dre_dbg_dump(fmt, args...)   DBG_DUMP(fmt, ##args);

#endif



