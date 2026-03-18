#ifndef _KDRV_VIE_DEBUG_INT_H_
#define _KDRV_VIE_DEBUG_INT_H_
#include "vie_dbg.h"
#include "vie_platform.h"
#include "kdrv_vie.h"
#include "plat/top.h"
#include "kdrv_vie_int.h"
/**
    kdrv_vie_debug_int.h


    @file       kdrv_vie_debug_int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

typedef enum {
	KDRV_VIE_DBG_MSG_OFF = 0,		//disable all debug msg
	KDRV_VIE_DBG_MSG_CTL_INFO,		//dump kdrv_vie parameters info
	KDRV_VIE_DBG_MSG_ALL,			//enable all ctl vie debug msg
	KDRV_VIE_DBG_MSG_MAX
} KDRV_VIE_DBG_MSG_TYPE;

typedef enum {
	KDRV_VIE_DBG_CNT_TYPE_CURR, // clear in open
	KDRV_VIE_DBG_CNT_TYPE_ACCU, // will not clear
	KDRV_VIE_DBG_CNT_TYPE_MAX
} KDRV_VIE_DBG_CNT_TYPE;

#define KDRV_VIE_DFT_LOG_RATE 300

typedef enum {
	KDRV_VIE_DBG_CNT_VD,

	KDRV_VIE_DBG_CNT_ERR_CH0_LINBUF_OVFL, // video intput channel 0: line buffer overflow
	KDRV_VIE_DBG_CNT_ERR_CH1_LINBUF_OVFL, // video intput channel 1: line buffer overflow
	KDRV_VIE_DBG_CNT_ERR_CH2_LINBUF_OVFL, // video intput channel 2: line buffer overflow
	KDRV_VIE_DBG_CNT_ERR_CH3_LINBUF_OVFL, // video intput channel 3: line buffer overflow

	KDRV_VIE_DBG_CNT_ERR_CH0_YCCENC, // video intput channel 0: ycc encode err
	KDRV_VIE_DBG_CNT_ERR_CH1_YCCENC, // video intput channel 1: ycc encode err
	KDRV_VIE_DBG_CNT_ERR_CH2_YCCENC, // video intput channel 2: ycc encode err
	KDRV_VIE_DBG_CNT_ERR_CH3_YCCENC, // video intput channel 3: ycc encode err

	KDRV_VIE_DBG_CNT_MAX

} KDRV_VIE_DBG_CNT_SEL;

typedef struct {
	KDRV_VIE_DBG_CNT_SEL dbg_cnt_sel;
	CHAR msg[18];
} KDRV_VIE_DBG_MSG;

typedef struct {
	KDRV_VIE_DBG_MSG_TYPE dbg_msg_type;

	/* debug */
	BOOL		init;
	atomic_t	cnt[KDRV_VIE_DBG_CNT_MAX][KDRV_VIE_DBG_CNT_TYPE_MAX];
	UINT32		err_log_rate;

} KDRV_VIE_DBG_INFO;

void kdrv_vie_dump_info(int (*dump)(const char *fmt, ...));
void kdrv_vie_dbg_set_dbg_level(KDRV_VIE_DBG_LVL dbg_level);
void kdrv_vie_dbg_set_msg_type(KDRV_VIE_PROC_ID id, KDRV_VIE_DBG_MSG_TYPE type);

void kdrv_vie_dbg_init(KDRV_VIE_PROC_ID id);
void kdrv_vie_dbg_uninit(KDRV_VIE_PROC_ID id);
void kdrv_vie_upd_vd_intrpt_sts(KDRV_VIE_PROC_ID id, VIE_ENGINE_STATUS_INFO_CB *info);
void kdrv_vie_upd_intrpt_err_sts(KDRV_VIE_PROC_ID id, UINT32 status);
int kdrv_vie_panic_printout_handler(uintptr_t data);

#endif //_KDRV_VIE_DEBUG_INT_H_
