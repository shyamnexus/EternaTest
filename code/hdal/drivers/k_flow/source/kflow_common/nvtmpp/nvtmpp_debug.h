/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       nvtmpp_debug.h

    @brief      nvtmpp debug api header file

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2017/02/13
*/
#ifndef _NVTMPP_DEBUG_H
#define _NVTMPP_DEBUG_H

#define THIS_DBGLVL         NVT_DBG_WRN
#define __MODULE__          nvtmpp
#define __DBGLVL__          8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass
#include "kwrap/debug.h"
#include "comm/util/log.h"

#if defined(__KERNEL__)
#define LOG_BUFFER          1
#else
#define LOG_BUFFER          0
#endif


extern unsigned int nvtmpp_debug_level;

#if LOG_BUFFER
#define NVTMPP_ERR(fmt, args...) \
	{ printm2("[nvtmpp]ERR: "   fmt , ##args);    \
	DBG_ERR(fmt, ##args); }
#define NVTMPP_WRN(fmt, args...) \
	{ printm2("[nvtmpp]WRN: "   fmt , ##args);    \
	DBG_ERR(fmt, ##args); }

#else
#define NVTMPP_ERR(fmt, args...) \
	DBG_ERR(fmt, ##args);
#define NVTMPP_WRN(fmt, args...) \
	DBG_WRN(fmt, ##args);
#endif

#endif
