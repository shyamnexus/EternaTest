#ifndef __ISF_VDOOUT_DBG_H
#define __ISF_VDOOUT_DBG_H

#define THIS_DBGLVL         NVT_DBG_USER
#define __MODULE__          isf_vdoout
#define __DBGLVL__          2 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#include "kwrap/debug.h"
extern unsigned int isf_vdoout_debug_level;

#include "comm/util/log.h"

#if defined(__KERNEL__)
#define LOG_BUFFER          1
#else
#define LOG_BUFFER          0
#endif

#if LOG_BUFFER
#define VDOOUT_DBG_ERR(fmt, args...) \
	{	\
		printm2(fmt, ##args);    \
		DBG_ERR(fmt, ##args);	\
	}
#define VDOOUT_DBG_WRN(fmt, args...) \
	{	\
		printm2(fmt, ##args);    \
		DBG_WRN(fmt, ##args);	\
	}
#define VDOOUT_DBG_DUMP(fmt, args...) \
	{	\
		printm2(fmt, ##args); \
		DBG_DUMP(fmt, ##args);	\
	}
#else
#define VDOOUT_DBG_ERR(fmt, args...) \
	DBG_ERR(fmt, ##args);
#define VDOOUT_DBG_WRN(fmt, args...) \
	DBG_WRN(fmt, ##args);
#define VDOOUT_DBG_DUMP(fmt, args...) \
	DBG_DUMP(fmt, ##args);
#endif

#endif


