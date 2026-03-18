#ifndef __CTL_SEN_MODULE_DBG_H_
#define __CTL_SEN_MODULE_DBG_H_

#define __MODULE__ ctl_sen
#define __DBGLVL__ 5          // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__ "*"      //*=All

#include "ctl_sen.h"

#if defined(__FREERTOS) || defined(__ECOS) || defined(__UITRON)
#else
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define debug_msg			printk
#endif

#include "kwrap/debug.h"
#include "comm/util/log.h"

// for dump all log to file
#if defined(__KERNEL__) && !defined(_BSP_NS02301_) // 567 reduce codesize
#define CTL_SEN_DBG_LOG_ERR(fmt, args...) printm2("ERR:%s() " fmt, __func__, ##args)
#define CTL_SEN_DBG_LOG_WRN(fmt, args...) printm2("WRN:%s() " fmt, __func__, ##args)
#define CTL_SEN_DBG_LOG_IND(fmt, args...) printm2("%s() " fmt, __func__, ##args)
#define CTL_SEN_DBG_LOG_DUMP(fmt, args...) printm2(fmt, ##args)
#else
#define CTL_SEN_DBG_LOG_ERR(fmt, args...)
#define CTL_SEN_DBG_LOG_WRN(fmt, args...)
#define CTL_SEN_DBG_LOG_IND(fmt, args...)
#define CTL_SEN_DBG_LOG_DUMP(fmt, args...)
#endif

/* dbg level */
typedef enum  {
	CTL_SEN_DBG_LV_NONE = 0,
	CTL_SEN_DBG_LV_ERR,
	CTL_SEN_DBG_LV_WRN,
	CTL_SEN_DBG_LV_IND,
	CTL_SEN_DBG_LV_TRC,
	CTL_SEN_DBG_LV_MAX,
	ENUM_DUMMY4WORD(CTL_SEN_DBG_LV)
} CTL_SEN_DBG_LV;

// for CTL_SEN_DBG_LV_TRC log type
typedef enum {
	CTL_SEN_DBG_TRC_NONE,
	CTL_SEN_DBG_TRC_GET_SET,
	CTL_SEN_DBG_TRC_REG,
	CTL_SEN_DBG_TRC_ALL,
} CTL_SEN_DBG_TRC_TYPE;

extern CTL_SEN_DBG_LV ctl_sen_dbg_level;
extern CTL_SEN_DBG_TRC_TYPE ctl_sen_dbg_trc_type;
#define ctl_sen_dbg_err(fmt, args...) { CTL_SEN_DBG_LOG_ERR(fmt, ##args); if (ctl_sen_dbg_level >= CTL_SEN_DBG_LV_ERR) { DBG_ERR(fmt, ##args); } }
#define ctl_sen_dbg_wrn(fmt, args...) { CTL_SEN_DBG_LOG_WRN(fmt, ##args); if (ctl_sen_dbg_level >= CTL_SEN_DBG_LV_WRN) { DBG_WRN(fmt, ##args); } }
#define ctl_sen_dbg_ind(fmt, args...) { CTL_SEN_DBG_LOG_IND(fmt, ##args); if (ctl_sen_dbg_level >= CTL_SEN_DBG_LV_IND) { DBG_IND(fmt, ##args); } }
#define ctl_sen_dbg_dump(fmt, args...) { CTL_SEN_DBG_LOG_DUMP(fmt, ##args); DBG_DUMP(fmt, ##args); }
#define ctl_sen_dbg_trc(type, fmt, args...)									\
	do {																	\
		if ((ctl_sen_dbg_level >= CTL_SEN_DBG_LV_TRC) &&					\
			((ctl_sen_dbg_trc_type == type) || 								\
			(ctl_sen_dbg_trc_type == CTL_SEN_DBG_TRC_ALL))) {				\
			CTL_SEN_DBG_LOG_IND(fmt, ##args); 								\
			DBG_IND(fmt, ##args);											\
		}																	\
	} while(0)

#endif

