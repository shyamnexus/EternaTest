#ifndef __ISF_VDOCAP_DBG_H
#define __ISF_VDOCAP_DBG_H

#define THIS_DBGLVL         NVT_DBG_WRN
#define __MODULE__          isf_vdocap
#define __DBGLVL__          8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass
#include "kwrap/debug.h"
#include "comm/util/log.h"

#if defined(__KERNEL__)
#define LOG_BUFFER          1
#else
#define LOG_BUFFER          0
#endif

#define VCAP_TRACE(opclass, fmtstr, args...) p_thisunit->p_base->do_trace(p_thisunit, opclass, fmtstr, ##args)

extern unsigned int isf_vdocap_debug_level;

#if LOG_BUFFER
#define VCAP_DBG_ERR(fmt, args...) \
	{printm2("ERR:%s() "   fmt, __func__, ##args);    \
	DBG_ERR(fmt, ##args);}
#define VCAP_DBG_WRN(fmt, args...) \
	{printm2("WRN:%s() "   fmt, __func__, ##args);    \
	DBG_WRN(fmt, ##args);}
#define VCAP_DBG_UNIT(fmt, args...) \
	{printm2("%s() "  fmt, __func__, ##args);    \
	DBG_UNIT(fmt, ##args);}
#define VCAP_DBG_MSG(fmt, args...) \
	{printm2(fmt, ##args);    \
	DBG_MSG(fmt, ##args);}
#define VCAP_DBG_DUMP(fmt, args...) \
	{printm2(fmt, ##args);    \
	DBG_DUMP(fmt, ##args);}
#define VCAP_DBG_DUMP(fmt, args...) \
	{printm2(fmt, ##args);    \
	DBG_DUMP(fmt, ##args);}

#else
#define VCAP_DBG_ERR(fmt, args...) \
	DBG_ERR(fmt, ##args);
#define VCAP_DBG_WRN(fmt, args...) \
	DBG_WRN(fmt, ##args);
#define VCAP_DBG_UNIT(fmt, args...) \
	DBG_UNIT(fmt, ##args);
#define VCAP_DBG_MSG(fmt, args...) \
	DBG_MSG(fmt, ##args);
#define VCAP_DBG_DUMP(fmt, args...) \
	DBG_DUMP(fmt, ##args);
#endif

#endif
