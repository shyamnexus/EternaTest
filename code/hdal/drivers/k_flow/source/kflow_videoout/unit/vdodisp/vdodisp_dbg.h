#ifndef __ISF_VDOOUT_DBG_H
#define __ISF_VDOOUT_DBG_H

#include "comm/util/log.h"

#if defined(__KERNEL__)
#define LOG_BUFFER          1
#else
#define LOG_BUFFER          0
#endif

#if LOG_BUFFER
#define VDOOUT_DBG_ERR(fmt, args...) \
	printm2(fmt, ##args);    \
	DBG_ERR(fmt, ##args);
#define VDOOUT_DBG_WRN(fmt, args...) \
	printm2(fmt, ##args);    \
	DBG_WRN(fmt, ##args);
#define VDOOUT_DBG_DUMP(fmt, args...) \
	printm2(fmt, ##args);    \
	DBG_DUMP(fmt, ##args);
#else
#define VDOOUT_DBG_ERR(fmt, args...) \
	DBG_ERR(fmt, ##args);
#define VDOOUT_DBG_WRN(fmt, args...) \
	DBG_WRN(fmt, ##args);
#define VDOOUT_DBG_DUMP(fmt, args...) \
	DBG_DUMP(fmt, ##args);
#endif

#endif


