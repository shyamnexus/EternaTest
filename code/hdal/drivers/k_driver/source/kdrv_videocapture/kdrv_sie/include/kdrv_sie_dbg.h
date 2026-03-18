#ifndef __KDRV_SIE_DBG_H_
#define __KDRV_SIE_DBG_H_

#include "kwrap/debug.h"
#include "comm/util/log.h"

#define KDRV_SIE_E_OK  			(0)
#define KDRV_SIE_E_ID 			(-100)	//illegal sie handle
#define KDRV_SIE_E_SYS 			(-200)	//sysem error
#define KDRV_SIE_E_HDL 			(-300)	//null handle
#define KDRV_SIE_E_NULL_FP 		(-400)	//null fp
#define KDRV_SIE_E_PAR 			(-500)	//parameter error
#define KDRV_SIE_E_NOSPT 		(-600)	//no support function
#define KDRV_SIE_E_NODEV 		(-700)	//no device found
#define KDRV_SIE_E_NOMEM 		(-800)	//no memory
#define KDRV_SIE_E_STATE 		(-900)	//illegal state

typedef enum _KDRV_SIE_DBG_LVL {
	KDRV_SIE_DBG_LVL_NONE = 0,
	KDRV_SIE_DBG_LVL_ERR,
	KDRV_SIE_DBG_LVL_WRN,
	KDRV_SIE_DBG_LVL_UNIT,
	KDRV_SIE_DBG_LVL_IND,
	KDRV_SIE_DBG_LVL_FUNC,
	KDRV_SIE_DBG_LVL_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_DBG_LVL)
} KDRV_SIE_DBG_LVL;

extern KDRV_SIE_DBG_LVL kdrv_sie_dbg_rt_lvl;

#if defined(__KERNEL__)
#define KDRV_SIE_LOG_BUFFER          1
#else
#define KDRV_SIE_LOG_BUFFER          0
#endif

#if KDRV_SIE_LOG_BUFFER
#define kdrv_sie_dbg_err(fmt, args...) \
	printm2("[kdrv_sie]ERR: "   fmt , ##args);    \
	{ if (kdrv_sie_dbg_rt_lvl >= KDRV_SIE_DBG_LVL_ERR) { DBG_ERR(fmt, ##args);  }}
#define kdrv_sie_dbg_wrn(fmt, args...) \
	printm2("[kdrv_sie]WRN: "   fmt , ##args);    \
	{ if (kdrv_sie_dbg_rt_lvl >= KDRV_SIE_DBG_LVL_WRN) { DBG_WRN(fmt, ##args);  }}
#define kdrv_sie_dbg_ind(fmt, args...) \
	printm2("[kdrv_sie]IND: "   fmt , ##args);    \
	{ if (kdrv_sie_dbg_rt_lvl >= KDRV_SIE_DBG_LVL_IND) { DBG_DUMP("[kdrv_sie]IND: " fmt, ##args); }}
#define kdrv_sie_dbg_func(fmt, args...) \
	{ if (kdrv_sie_dbg_rt_lvl >= KDRV_SIE_DBG_LVL_FUNC) { printm2("[kdrv_sie]FUNC: "   fmt , ##args); }}	    \
	{ if (kdrv_sie_dbg_rt_lvl >= KDRV_SIE_DBG_LVL_FUNC) { DBG_DUMP("[kdrv_sie]FUNC: "  fmt, ##args); }}
#else
#define kdrv_sie_dbg_err(fmt, args...) \
	{ if (kdrv_sie_dbg_rt_lvl >= KDRV_SIE_DBG_LVL_ERR) { DBG_ERR(fmt, ##args);  }}
#define kdrv_sie_dbg_wrn(fmt, args...) \
	{ if (kdrv_sie_dbg_rt_lvl >= KDRV_SIE_DBG_LVL_WRN) { DBG_WRN(fmt, ##args);  }}
#define kdrv_sie_dbg_ind(fmt, args...) \
	{ if (kdrv_sie_dbg_rt_lvl >= KDRV_SIE_DBG_LVL_IND) { DBG_DUMP("[kdrv_sie]IND: " fmt, ##args); }}
#define kdrv_sie_dbg_func(fmt, args...) \
	{ if (kdrv_sie_dbg_rt_lvl >= KDRV_SIE_DBG_LVL_FUNC) { DBG_DUMP("[kdrv_sie]FUNC: "  fmt, ##args); }}
#endif
#define kdrv_sie_dbg_dump(fmt, args...)   DBG_DUMP(fmt, ##args);

#endif
