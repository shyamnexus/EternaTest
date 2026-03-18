#ifndef __ISF_AUDOUT_DBG_H_
#define __ISF_AUDOUT_DBG_H_

#define THIS_DBGLVL         NVT_DBG_WRN
#define __MODULE__          isf_audout
#define __DBGLVL__          8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass
#if defined (__UITRON)
#include "DebugModule.h"
#else
#include "kwrap/debug.h"
#endif
#include "comm/util/log.h"

extern unsigned int wavstudio_debug_level;

#if defined(__KERNEL__)
#define LOG_BUFFER          1
#else
#define LOG_BUFFER          0
#endif

#if LOG_BUFFER
#define AUDOUT_DBG_ERR(fmt, args...) \
	{ printm2("[audout]ERR: "   fmt , ##args);    \
	DBG_ERR(fmt, ##args); }
#define AUDOUT_DBG_WRN(fmt, args...) \
	{ printm2("[audout]WRN: "   fmt , ##args);    \
	DBG_WRN(fmt, ##args); }
#define AUDOUT_DBG_DUMP(fmt, args...) \
	{ printm2("[audout] "   fmt , ##args);    \
	DBG_DUMP(fmt, ##args); }

#define AUDOUT_LOG_ERR(fmt, args...) \
	{ printm2("[audout]ERR: "   fmt , ##args); }
#define AUDOUT_LOG_WRN(fmt, args...) \
	{ printm2("[audout]WRN: "   fmt , ##args); }
#define AUDOUT_LOG_DUMP(fmt, args...) \
	{ printm2("[audout] "   fmt , ##args); }
#else
#define AUDOUT_DBG_ERR(fmt, args...) \
	{ DBG_ERR(fmt, ##args); }
#define AUDOUT_DBG_WRN(fmt, args...) \
	{ DBG_WRN(fmt, ##args); }
#define AUDOUT_DBG_DUMP(fmt, args...) \
	{ DBG_DUMP(fmt, ##args); }

#define AUDOUT_LOG_ERR(fmt, args...)
#define AUDOUT_LOG_WRN(fmt, args...)
#define AUDOUT_LOG_DUMP(fmt, args...)
#endif

int isf_audout_panic_printout_handler(uintptr_t data);

#endif

