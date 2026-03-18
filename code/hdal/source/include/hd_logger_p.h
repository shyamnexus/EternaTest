/**
	@brief Source code of logger function.\n
	This file contains the logger function, and logger menu entry point.

	@file hd_debug_menu.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _HD_LOGGER_P_H_
#define _HD_LOGGER_P_H_

typedef enum _HD_FLOW_DBG {
	HD_FLOW_DBG_COMM = 0,
	HD_FLOW_DBG_VCAP,
	HD_FLOW_DBG_VPRC,
	HD_FLOW_DBG_VOUT,
	HD_FLOW_DBG_VENC,
	HD_FLOW_DBG_VDEC,
	HD_FLOW_DBG_ACAP,
	HD_FLOW_DBG_AOUT,
	HD_FLOW_DBG_AENC,
	HD_FLOW_DBG_ADEC,
	HD_FLOW_DBG_GFX,
	HD_FLOW_DBG_MAX,
	E_HD_FLOW_DBG = 0x10000000
} HD_FLOW_DBG;

extern unsigned int *hd_flow_dbg_lvl[HD_FLOW_DBG_MAX];


void hd_logger_init_p(unsigned int cpu_cfg);
void hd_logger_uninit_p(void);
void hdal_flow_log_p(const char *fmt, ...); //this api name synced from 313
void vprintm_p(const char *fmt, va_list ap);

#endif
