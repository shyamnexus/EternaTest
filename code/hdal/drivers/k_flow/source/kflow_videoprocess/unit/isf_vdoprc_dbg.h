#ifndef __ISF_VDOPRC_DBG_H_
#define __ISF_VDOPRC_DBG_H_

#define THIS_DBGLVL         NVT_VPRC_DBG_WRN
#define __MODULE__          isf_vdoprc
#define __DBGLVL__          8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass
#if defined (__UITRON)
#include "DebugModule.h"
#else
#include "kwrap/debug.h"
#endif
#include "comm/util/log.h"

#define LOG_UART  0x2
#define LOG_FILE  0x1
#define LOG_ALL  (LOG_UART|LOG_FILE)

#if defined(__KERNEL__)
#define LOG_BUFFER          1
#else
#define LOG_BUFFER          0
#endif

extern unsigned int isf_vdoprc_debug_level;

#if LOG_BUFFER
#define VPRC_DBG_ERR(fmt, args...) \
	{printm2("ERR:%s() "   fmt, __func__, ##args);    \
	DBG_ERR(fmt, ##args);}
#define VPRC_DBG_WRN(fmt, args...) \
	{printm2("WRN:%s() "   fmt, __func__, ##args);    \
	DBG_WRN(fmt, ##args);}
#define VPRC_DBG_UNIT(fmt, args...) \
	{printm2("%s() "  fmt, __func__, ##args);    \
	DBG_UNIT(fmt, ##args);}
#define VPRC_DBG_MSG(fmt, args...) \
	{printm2(fmt, ##args);    \
	DBG_MSG(fmt, ##args);}
#define VPRC_DBG_DUMP(fmt, args...) \
	{printm2(fmt, ##args);    \
	DBG_DUMP(fmt, ##args);}

#define VPRC_DUMP(type,fmt, args...) \
	{if((ULONG)type & LOG_FILE) printm2(fmt, ##args);    \
	 if((ULONG)type & LOG_UART) DBG_DUMP(fmt, ##args);}
#else
#define VPRC_DBG_ERR(fmt, args...) \
	DBG_ERR(fmt, ##args);
#define VPRC_DBG_WRN(fmt, args...) \
	DBG_WRN(fmt, ##args);
#define VPRC_DBG_UNIT(fmt, args...) \
	DBG_UNIT(fmt, ##args);
#define VPRC_DBG_MSG(fmt, args...) \
	DBG_MSG(fmt, ##args);
#define VPRC_DBG_DUMP(fmt, args...) \
	DBG_DUMP(fmt, ##args);
#define VPRC_DUMP(type,fmt, args...) \
	DBG_MSG(fmt, ##args);
#endif



#endif

