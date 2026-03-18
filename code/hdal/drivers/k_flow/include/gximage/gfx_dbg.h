/**
    Header file of GFX debug header

    @file       gfx_dbg.h

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _ISF_GFX_DBG_H
#define _ISF_GFX_DBG_H

#include "comm/util/log.h"

#if defined(__KERNEL__)
#define LOG_BUFFER          1
#else
#define LOG_BUFFER          0
#endif

#if LOG_BUFFER
#define GFX_DBG_ERR(fmt, args...) \
	{ \
		printm2(fmt, ##args);    \
		DBG_ERR(fmt, ##args);    \
	}
#define GFX_DBG_WRN(fmt, args...) \
	{	\
		printm2(fmt, ##args);    \
		DBG_WRN(fmt, ##args);    \
	}
#define GFX_DBG_DUMP(fmt, args...) \
	{	\
		printm2(fmt, ##args);    \
		DBG_DUMP(fmt, ##args);    \
	}
#else
#define GFX_DBG_ERR(fmt, args...) \
	DBG_ERR(fmt, ##args);
#define GFX_DBG_WRN(fmt, args...) \
	DBG_WRN(fmt, ##args);
#define GFX_DBG_DUMP(fmt, args...) \
	DBG_DUMP(fmt, ##args);
#endif
#endif//_ISF_GFX_DBG_H

//@}
