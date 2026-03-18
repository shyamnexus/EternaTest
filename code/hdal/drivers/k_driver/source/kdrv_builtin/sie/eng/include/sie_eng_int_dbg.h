/*
    Internal header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _SIE_ENG_INT_DBG_H_
#define _SIE_ENG_INT_DBG_H_

#define __MODULE__  sie_eng_dbg
#define __DBGLVL__  NVT_DBG_USER
#include "kwrap/debug.h"

#define sie_printf(fmt, args...)    vk_pr_warn(fmt, ##args)
#define sie_err(fmt, args...)       nvt_dbg(ERR,   DBG_COLOR_ERR "[SIE_ERR]: " fmt DBG_COLOR_END, ##args)
#define sie_dbg(fmt, args...)       nvt_dbg(WRN,   DBG_COLOR_WRN "[SIE_DBG]: " fmt DBG_COLOR_END, ##args)
#define sie_warn(fmt, args...)      nvt_dbg(WRN,   DBG_COLOR_WRN "[SIE_WRN]: " fmt DBG_COLOR_END, ##args)
#define sie_info(fmt, args...)      nvt_dbg(MSG,   DBG_COLOR_MSG "[SIE_INF]: " fmt DBG_COLOR_END, ##args)

#if defined (__LINUX)
#define sie_isr_err(dbg_lvl,  fmt, args...)  do { if (dbg_lvl && printk_ratelimit()) nvt_dbg(ERR, DBG_COLOR_ERR "ERR:%s() " fmt DBG_COLOR_END, __func__, ##args); } while(0)
#define sie_isr_warn(dbg_lvl, fmt, args...)  do { if (dbg_lvl && printk_ratelimit()) nvt_dbg(WRN, DBG_COLOR_WRN "WRN:%s() " fmt DBG_COLOR_END, __func__, ##args); } while(0)
#else
#define sie_isr_err(dbg_lvl,  fmt, args...)  do { if (dbg_lvl) nvt_dbg(ERR, DBG_COLOR_ERR "ERR:%s() " fmt DBG_COLOR_END, __func__, ##args); } while(0)
#define sie_isr_warn(dbg_lvl, fmt, args...)  do { if (dbg_lvl) nvt_dbg(WRN, DBG_COLOR_WRN "WRN:%s() " fmt DBG_COLOR_END, __func__, ##args); } while(0)
#endif

#endif /* _SIE_ENG_INT_DBG_H_ */
