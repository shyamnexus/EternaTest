/*
    IPE module driver

    NT98520 IPE internal header file.

    @file       kdrv_ipe_int.h
    @ingroup    mIIPPIPE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _KDRV_IPE_INT_H_
#define _KDRV_IPE_INT_H_

#if defined(__LINUX)
#elif defined(__FREERTOS)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "kwrap/task.h"
#include "kwrap/type.h"
#include "kwrap/spinlock.h"
#include "kwrap/semaphore.h"
#include "kwrap/error_no.h"
#include "kwrap/cpu.h"
#include "ipe_eng.h"
#include "kdrv_ipe_int_dbg.h"
#include "kdrv_ipp_int_util.h"
#include "kdrv_videoprocess/kdrv_ipe_539a.h"


#ifdef CONFIG_PM
#if defined(__LINUX)
#include <linux/soc/nvt/nvt-info.h>
#include <linux/of.h>
#include <linux/clk.h>
#else
#include <comm/hwclock.h>
#include <pll.h>
#endif

#endif

#include "ipe_eng_int_platform.h"

#define KDRV_IPE_TIMEOUT_MS (1000)

typedef enum {
	KDRV_IPE_ISR_STATE_UNKNOWN = 0,
	KDRV_IPE_ISR_STATE_START,
	KDRV_IPE_ISR_STATE_END,
	KDRV_IPE_ISR_STATE_MAX
} KDRV_IPE_ISR_STATE;

typedef struct {
	UINT32 to_dma;
	UINT32 to_ime;
	IPE_ENG_DRAM_OUT_SEL out_sel;
	IPE_ENG_FMT in_fmt;
	IPE_ENG_FMT out_fmt;
} KDRV_IPE_IO_INFO;

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	BOOL auto_gating;
	SEM_HANDLE sem;
	vk_spinlock_t lock;
	IPE_ENG_HANDLE *p_eng;
	void *p_eng_reg_buf;	/* working buffer for ipe_eng */
	void *p_eng_flg_buf;	/* working buffer for ipe_eng */
	KDRV_IPP_ISR_CB cb;
	KDRV_IPE_ISR_STATE isr_state;
	KDRV_IPE_DBG_INFO *p_ipe_dbg_info;
	BOOL ycurve_en;			// 690 patch. use to check if ycurve is enabled
	UINT32 ycurve_msb;		// 690 patch. use to check if ll blk address's msb is same as ycurve lut address's msb
	UINT32 *reg_data;
} KDRV_IPE_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	KDRV_IPE_HANDLE *p_hdl;
} KDRV_IPE_CTL;

typedef struct {
	INT32(*set_fp)(KDRV_IPE_HANDLE *p_hdl, void *p_data);
	BOOL set_chk_data;

	INT32(*get_fp)(KDRV_IPE_HANDLE *p_hdl, void *p_data);
	BOOL get_chk_data;

	CHAR *msg;
} KDRV_IPE_FUNC_ITEM;

typedef struct {
	INT32(*query_fp)(UINT32 id, void *p_data);
	CHAR *msg;
} KDRV_IPE_QUERY_ITEM;

typedef struct {
	UINT32 in_w_min;
	UINT32 in_w_max;
	UINT32 in_w_align;
	UINT32 in_h_min;
	UINT32 in_h_max;
	UINT32 in_h_align;
	UINT32 in_y_addr_align;
	UINT32 in_uv_addr_align;
	UINT32 in_y_lofs_align;
	UINT32 in_uv_lofs_align;

	UINT32 out_w_min;
	UINT32 out_w_max;
	UINT32 out_w_align;
	UINT32 out_h_min;
	UINT32 out_h_max;
	UINT32 out_h_align;
	UINT32 out_y_addr_align;
	UINT32 out_uv_addr_align;
	UINT32 out_y_lofs_align;
	UINT32 out_uv_lofs_align;

	UINT32 defog_subimg_w_min;
	UINT32 defog_subimg_w_max;
	UINT32 defog_subimg_w_align;
	UINT32 defog_subimg_h_min;
	UINT32 defog_subimg_h_max;
	UINT32 defog_subimg_h_align;
	UINT32 defog_subimg_addr_align;
	UINT32 defog_subimg_lofs_align;

	UINT32 lce_subimg_w_min;
	UINT32 lce_subimg_w_max;
	UINT32 lce_subimg_w_align;
	UINT32 lce_subimg_h_min;
	UINT32 lce_subimg_h_max;
	UINT32 lce_subimg_h_align;
	UINT32 lce_subimg_addr_align;
	UINT32 lce_subimg_lofs_align;

} KDRV_IPE_LMT;

INT32 kdrv_ipe_sys_init(UINT32 chip_num, UINT32 eng_num);
INT32 kdrv_ipe_sys_uninit(void);
void kdrv_ipe_dump_info(void);
void *kdrv_ipe_os_malloc_wrap(UINT32 want_size);
void kdrv_ipe_os_mfree_wrap(void *p_buf);
void kdrv_ipe_eng_setreg(uintptr_t ofs, UINT32 value);
UINT32 kdrv_ipe_eng_getreg(uintptr_t ofs);

#endif
