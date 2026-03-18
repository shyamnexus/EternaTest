#ifndef __VIE_DBG_H_
#define __VIE_DBG_H_

#include "kwrap/debug.h"
#include "comm/util/log.h"

#define KDRV_VIE_E_OK  			(0)
#define KDRV_VIE_E_ID 			(-100)	//illegal vie handle
#define KDRV_VIE_E_SYS 			(-200)	//sysem error
#define KDRV_VIE_E_HDL 			(-300)	//null handle
#define KDRV_VIE_E_NULL_FP 		(-400)	//null fp
#define KDRV_VIE_E_PAR 			(-500)	//parameter error
#define KDRV_VIE_E_NOSPT 		(-600)	//no support function
#define KDRV_VIE_E_NODEV 		(-700)	//no device found
#define KDRV_VIE_E_NOMEM 		(-800)	//no memory
#define KDRV_VIE_E_STATE 		(-900)	//illegal state

typedef enum _KDRV_VIE_DBG_LVL {
	KDRV_VIE_DBG_LVL_NONE = 0,
	KDRV_VIE_DBG_LVL_ERR,
	KDRV_VIE_DBG_LVL_WRN,
	KDRV_VIE_DBG_LVL_UNIT,
	KDRV_VIE_DBG_LVL_IND,
	KDRV_VIE_DBG_LVL_FUNC,
	KDRV_VIE_DBG_LVL_MAX,
	ENUM_DUMMY4WORD(KDRV_VIE_DBG_LVL)
} KDRV_VIE_DBG_LVL;

extern KDRV_VIE_DBG_LVL kdrv_vie_dbg_rt_lvl;

#if defined(__KERNEL__)
#define KDRV_VIE_LOG_BUFFER          1
#else
#define KDRV_VIE_LOG_BUFFER          0
#endif

#if KDRV_VIE_LOG_BUFFER
#define kdrv_vie_dbg_err(fmt, args...) \
	printm2("[kdrv_vie]ERR: "   fmt , ##args);    \
	{ if (kdrv_vie_dbg_rt_lvl >= KDRV_VIE_DBG_LVL_ERR) { DBG_ERR(fmt, ##args);  }}
#define kdrv_vie_dbg_wrn(fmt, args...) \
	printm2("[kdrv_vie]WRN: "   fmt , ##args);    \
	{ if (kdrv_vie_dbg_rt_lvl >= KDRV_VIE_DBG_LVL_WRN) { DBG_WRN(fmt, ##args);  }}
#define kdrv_vie_dbg_ind(fmt, args...) \
	printm2("[kdrv_vie]IND: "   fmt , ##args);    \
	{ if (kdrv_vie_dbg_rt_lvl >= KDRV_VIE_DBG_LVL_IND) { DBG_DUMP("[kdrv_vie]IND: " fmt, ##args); }}
#define kdrv_vie_dbg_func(fmt, args...) \
	{ if (kdrv_vie_dbg_rt_lvl >= KDRV_VIE_DBG_LVL_FUNC) { printm2("[kdrv_vie]FUNC: " fmt , ##args); }}	    \
	{ if (kdrv_vie_dbg_rt_lvl >= KDRV_VIE_DBG_LVL_FUNC) { DBG_DUMP("[kdrv_vie]FUNC: " fmt, ##args); }}
#else
#define kdrv_vie_dbg_err(fmt, args...) \
	{ if (kdrv_vie_dbg_rt_lvl >= KDRV_VIE_DBG_LVL_ERR) { DBG_ERR(fmt, ##args);  }}
#define kdrv_vie_dbg_wrn(fmt, args...) \
	{ if (kdrv_vie_dbg_rt_lvl >= KDRV_VIE_DBG_LVL_WRN) { DBG_WRN(fmt, ##args);  }}
#define kdrv_vie_dbg_ind(fmt, args...) \
	{ if (kdrv_vie_dbg_rt_lvl >= KDRV_VIE_DBG_LVL_IND) { DBG_DUMP("[kdrv_vie]IND: " fmt, ##args); }}
#define kdrv_vie_dbg_func(fmt, args...) \
	{ if (kdrv_vie_dbg_rt_lvl >= KDRV_VIE_DBG_LVL_FUNC) { DBG_DUMP("[kdrv_vie]FUNC: " fmt, ##args); }}
#endif
#define kdrv_vie_dbg_dump(fmt, args...)   DBG_DUMP(fmt, ##args);

#endif
