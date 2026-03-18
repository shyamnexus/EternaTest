#ifndef _KDRV_SIE_DEBUG_INT_H_
#define _KDRV_SIE_DEBUG_INT_H_

#if defined(__FREERTOS)
#include "string.h"
#include <stdlib.h>
#include "rcw_macro.h"
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#include "dma_protected.h"
#else
#include <linux/slab.h>
#include <plat/nvt-sramctl.h>
#include <linux/clk-provider.h>
#include <linux/soc/nvt/fmem.h>
#endif
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/util.h"
#include "kwrap/cpu.h"
#include "plat/top.h"
#include "kdrv_sie_dbg.h"
#include "kdrv_sie.h"
#include "kdrv_sie_int.h"


/**
    kdrv_sie_debug_int.h


    @file       kdrv_sie_debug_int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

typedef enum {
	KDRV_SIE_DBG_MSG_OFF = 0,		//disable all debug msg
	KDRV_SIE_DBG_MSG_CTL_INFO,		//dump kdrv_sie parameters info
	KDRV_SIE_DBG_MSG_ALL,			//enable all ctl sie debug msg
	KDRV_SIE_DBG_MSG_MAX
} KDRV_SIE_DBG_MSG_TYPE;

typedef enum {
	KDRV_SIE_DBG_CNT_TYPE_CURR, // clear in open
	KDRV_SIE_DBG_CNT_TYPE_ACCU, // will not clear
	KDRV_SIE_DBG_CNT_TYPE_MAX
} KDRV_SIE_DBG_CNT_TYPE;

#define KDRV_SIE_DFT_LOG_RATE 300

typedef enum {
	KDRV_SIE_DBG_CNT_VD,
	KDRV_SIE_DBG_CNT_ERR_DRMIN1_UDFL,
	KDRV_SIE_DBG_CNT_ERR_DRMIN2_UDFL,
	KDRV_SIE_DBG_CNT_ERR_DRMOUT0_OVFL,
	KDRV_SIE_DBG_CNT_ERR_DRMOUT1_OVFL,
	KDRV_SIE_DBG_CNT_ERR_DRMOUT2_OVFL,
	KDRV_SIE_DBG_CNT_ERR_DRMOUT0_SZ,
	KDRV_SIE_DBG_CNT_ERR_DPC,
	KDRV_SIE_DBG_CNT_ERR_RAWENC,
	KDRV_SIE_DBG_CNT_ERR_SIECLK,
	KDRV_SIE_DBG_CNT_ERR_BEHAVIOR,
	KDRV_SIE_DBG_CNT_ERR_SRC_BUF_OVFL,
	KDRV_SIE_DBG_CNT_ERR_RAWENC_FIFO_OVFL,
	KDRV_SIE_DBG_CNT_ERR_RAWENC_DATA_OVFL,
	KDRV_SIE_DBG_CNT_ERR_RAWENC_DATA_BEHAVIOR,
	KDRV_SIE_DBG_CNT_MAX
} KDRV_SIE_DBG_CNT_SEL;

typedef struct {
	KDRV_SIE_DBG_CNT_SEL dbg_cnt_sel;
	CHAR msg[30];
} KDRV_SIE_DBG_MSG;

typedef struct {
	KDRV_SIE_DBG_MSG_TYPE dbg_msg_type;

	/* debug */
	BOOL		init;
	atomic_t	cnt[KDRV_SIE_DBG_CNT_MAX][KDRV_SIE_DBG_CNT_TYPE_MAX];
	UINT32		err_log_rate;
} KDRV_SIE_DBG_INFO;

void kdrv_sie_dump_info(int (*dump)(const char *fmt, ...));
void kdrv_sie_dbg_set_dbg_level(KDRV_SIE_DBG_LVL dbg_level);
void kdrv_sie_dbg_set_eng_dbglv(int dbglv, BOOL isr_en);
void kdrv_sie_dbg_set_msg_type(KDRV_SIE_PROC_ID id, KDRV_SIE_DBG_MSG_TYPE type);
UINT32 kdrv_sie_set_dbg_func_en(UINT32 id, UINT32 dbg_func_en, BOOL en);

void kdrv_sie_dbg_init(KDRV_SIE_PROC_ID id);
void kdrv_sie_dbg_uninit(KDRV_SIE_PROC_ID id);
void kdrv_sie_upd_vd_intrpt_sts(KDRV_SIE_PROC_ID id, SIE_ENGINE_STATUS_INFO *info);
void kdrv_sie_upd_intrpt_err_sts(KDRV_SIE_PROC_ID id, UINT32 status);
int kdrv_sie_panic_printout_handler(uintptr_t data);

#endif //_KDRV_SIE_DEBUG_INT_H_
