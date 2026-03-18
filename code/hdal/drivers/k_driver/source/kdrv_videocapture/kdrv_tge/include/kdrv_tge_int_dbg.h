#ifndef _KDRV_TGE_INT_DBG_H_
#define _KDRV_TGE_INT_DBG_H_

// should remove in the future
//#include "tge_dbg.h"
#include "comm/util/log.h"

// 690 only
#define __MODULE__          kdrv_tge
#define __DBGLVL__          NVT_DBG_USER

extern unsigned int kdrv_tge_debug_level;

// dbg level
typedef enum  {
	KDRV_TGE_DBG_LV_NONE =  0,
	KDRV_TGE_DBG_LV_ERR  =  1,
	KDRV_TGE_DBG_LV_WRN  =  2,
	KDRV_TGE_DBG_LV_FUNC =  4,
	KDRV_TGE_DBG_LV_IND  =  5,
	ENUM_DUMMY4WORD(KDRV_TGE_DBG_LV)
} KDRV_TGE_DBG_LV;

#if defined(__LINUX)
#include "kwrap/debug.h"

#elif defined(__FREERTOS)
#include <stdio.h>
#include <kwrap/debug.h>

#ifndef unlikely
#define unlikely(x) (x)
#endif

#else
#error Not supported OS
#endif

#if defined(__KERNEL__)
#define KDRV_TGE_LOG_BUFFER (1)
#else
#define KDRV_TGE_LOG_BUFFER (0)
#endif

#if KDRV_TGE_LOG_BUFFER
#define KDRV_TGE_ERR(fmt, args...)	\
	do{	\
    	printm2("[kdrv_tge]ERR: "   fmt , ##args);	\
		if (kdrv_tge_debug_level >= KDRV_TGE_DBG_LV_ERR) { DBG_ERR(fmt, ##args);  }	\
	}while(0)
#define KDRV_TGE_WRN(fmt, args...) \
	do{	\
    	printm2("[kdrv_tge]WRN: "   fmt , ##args);	\
    	if (kdrv_tge_debug_level >= KDRV_TGE_DBG_LV_WRN) { DBG_WRN(fmt, ##args);  }	\
	}while(0)
#define KDRV_TGE_FUNC(fmt, args...)	\
	do{	\
    	printm2("[kdrv_tge]FUNC: "   fmt , ##args);	\
    	if (kdrv_tge_debug_level >= KDRV_TGE_DBG_LV_FUNC) { DBG_FUNC(fmt, ##args);  }	\
	}while(0)
#define KDRV_TGE_IND(fmt, args...)	\
	do{	\
    	printm2("[kdrv_tge]IND: "   fmt , ##args);	\
    	if (kdrv_tge_debug_level >= KDRV_TGE_DBG_LV_IND) { DBG_IND(fmt, ##args);  }	\
	}while(0)
#define KDRV_TGE_DUMP(fmt, args...)	\
	do{	\
		printm2("[kdrv_tge]DUMP: "   fmt , ##args);	\
    	DBG_DUMP(fmt, ##args);	\
	}while(0)
#else
#define KDRV_TGE_ERR(fmt, args...)	\
	do{ if (kdrv_tge_debug_level >= KDRV_TGE_DBG_LV_ERR) { DBG_ERR(fmt, ##args);  }}while(0)
#define KDRV_TGE_WRN(fmt, args...)	\
    do{ if (kdrv_tge_debug_level >= KDRV_TGE_DBG_LV_WRN) { DBG_WRN(fmt, ##args);  }}while(0)
#define KDRV_TGE_FUNC(fmt, args...)	\
    do{ if (kdrv_tge_debug_level >= KDRV_TGE_DBG_LV_FUNC) { DBG_FUNC(fmt, ##args);  }}while(0)
#define KDRV_TGE_IND(fmt, args...)	\
    do{ if (kdrv_tge_debug_level >= KDRV_TGE_DBG_LV_IND) { DBG_IND(fmt, ##args);  }}while(0)
#define KDRV_TGE_DUMP(fmt, args...)	\
    DBG_DUMP(fmt, ##args);
#endif

#endif //_KDRV_TGE_INT_DBG_H_



