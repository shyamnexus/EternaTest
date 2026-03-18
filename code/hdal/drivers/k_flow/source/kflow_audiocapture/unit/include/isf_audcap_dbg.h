#ifndef __ISF_AUDCAP_DBG_H_
#define __ISF_AUDCAP_DBG_H_

#define THIS_DBGLVL         NVT_DBG_WRN
#define __MODULE__          isf_audcap
#define __DBGLVL__          8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass
#if defined (__UITRON)
#include "DebugModule.h"
#else
#include "kwrap/debug.h"
#endif
#include "comm/util/log.h"

extern unsigned int isf_audcap_debug_level;

#if defined(__KERNEL__)
#define LOG_BUFFER          1
#else
#define LOG_BUFFER          0
#endif

#if LOG_BUFFER
#define AUDCAP_DBG_ERR(fmt, args...) \
	{printm2("[audcap]ERR: "   fmt , ##args);    \
	DBG_ERR(fmt, ##args);}
#define AUDCAP_DBG_WRN(fmt, args...) \
	{printm2("[audcap]WRN: "   fmt , ##args);    \
	DBG_WRN(fmt, ##args);}
#define AUDCAP_DBG_DUMP(fmt, args...) \
	{printm2("[audcap] "   fmt , ##args);    \
	DBG_DUMP(fmt, ##args);}

#define AUDCAP_LOG_ERR(fmt, args...) \
	{printm2("[audcap]ERR: "   fmt , ##args);}
#define AUDCAP_LOG_WRN(fmt, args...) \
	{printm2("[audcap]WRN: "   fmt , ##args);}
#define AUDCAP_LOG_DUMP(fmt, args...) \
	{printm2("[audcap] "   fmt , ##args);}
#else
#define AUDCAP_DBG_ERR(fmt, args...) \
	{DBG_ERR(fmt, ##args);}
#define AUDCAP_DBG_WRN(fmt, args...) \
	{DBG_WRN(fmt, ##args);}
#define AUDCAP_DBG_DUMP(fmt, args...) \
	{DBG_DUMP(fmt, ##args);}

#define AUDCAP_LOG_ERR(fmt, args...)
#define AUDCAP_LOG_WRN(fmt, args...)
#define AUDCAP_LOG_DUMP(fmt, args...)
#endif

int isf_audcap_panic_printout_handler(uintptr_t data);


#endif

