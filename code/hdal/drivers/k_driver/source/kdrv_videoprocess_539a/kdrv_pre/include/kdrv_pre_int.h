/*
    PRE module driver

    NT98520 PRE internal header file.

    @file       kdrv_pre_int.h
    @ingroup    mIIPPPRE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _KDRV_PRE_INT_H_
#define _KDRV_PRE_INT_H_

#if defined(__LINUX)
#elif defined(__FREERTOS)
#include <stdio.h>
#include <string.h>
#endif
#include "kwrap/task.h"
#include "kwrap/type.h"
#include "kwrap/spinlock.h"
#include "kwrap/semaphore.h"
#include "kwrap/error_no.h"
#include "kwrap/cpu.h"
#include "pre_eng.h"
#include "kdrv_pre_int_dbg.h"
#include "kdrv_ipp_int_util.h"
#include "kdrv_videoprocess/kdrv_pre_539a.h"

#include "pre_eng_int_platform.h"


#define KDRV_PRE_TIMEOUT_MS (1000)
#define KDRV_PRE_RNG_TH_MAX 1023

typedef enum {
	KDRV_PRE_ISR_STATE_UNKNOWN = 0,
	KDRV_PRE_ISR_STATE_START,
	KDRV_PRE_ISR_STATE_END,
	KDRV_PRE_ISR_STATE_MAX
} KDRV_PRE_ISR_STATE;

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	BOOL auto_gating;
	SEM_HANDLE sem;
	vk_spinlock_t lock;
	PRE_ENG_HANDLE *p_eng;
	void *p_eng_reg_buf;	/* working buffer for pre_eng */
	void *p_eng_flg_buf;	/* working buffer for pre_eng */
	KDRV_IPP_ISR_CB cb;
	KDRV_PRE_ISR_STATE isr_state;
	KDRV_PRE_DBG_INFO *p_pre_dbg_info;
	KDRV_PRE_DBG_CTL pre_dbg_ctl;
	UINT32 *reg_data;

	UINT32 ll_cmd_w_time;  //LL_CMD write time
} KDRV_PRE_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	KDRV_PRE_HANDLE *p_hdl;
} KDRV_PRE_CTL;

typedef struct {
	INT32(*set_fp)(KDRV_PRE_HANDLE *p_hdl, void *p_data);
	BOOL set_chk_data;

	INT32(*get_fp)(KDRV_PRE_HANDLE *p_hdl, void *p_data);
	BOOL get_chk_data;

	CHAR *msg;
} KDRV_PRE_FUNC_ITEM;

typedef struct {
	INT32(*query_fp)(UINT32 id, void *p_data);
	CHAR *msg;
} KDRV_PRE_QUERY_ITEM;


typedef struct {
	UINT32 in_w_min;
	UINT32 in_w_max;
	UINT32 in_w_align;
	UINT32 in_h_min;
	UINT32 in_h_max;
	UINT32 in_h_align;
	UINT32 in_addr_align;
	UINT32 in_lofs_align;

	UINT32 out_w_min;
	UINT32 out_w_max;
	UINT32 out_w_align;
	UINT32 out_h_min;
	UINT32 out_h_max;
	UINT32 out_h_align;
	UINT32 out_addr_align;
	UINT32 out_lofs_align;

} KDRV_PRE_LMT;

INT32 kdrv_pre_sys_init(UINT32 chip_num, UINT32 eng_num);
INT32 kdrv_pre_sys_uninit(void);
void *kdrv_pre_os_malloc_wrap(UINT32 want_size);
void kdrv_pre_os_mfree_wrap(void *p_buf);
UINT32 kdrv_pre_do_div(UINT64 n, UINT64 base);
void kdrv_pre_eng_setreg(uintptr_t ofs, UINT32 value);
UINT32 kdrv_pre_eng_getreg(uintptr_t ofs);


#if 0
extern VOID pre_eng_platform_int_enable(VOID);
extern VOID pre_eng_platform_int_disable(VOID);
extern VOID pre_eng_platform_enable_clk(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_disable_clk(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_prepare_clk(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_unprepare_clk(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_disable_sram_shutdown(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_enable_sram_shutdown(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_request_irq(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_release_irq(PRE_ENG_HANDLE *p_eng);
extern INT32 pre_eng_platform_set_clk_rate(PRE_ENG_HANDLE *p_eng);
extern UINT32 pre_eng_platform_get_clk_rate(VOID);
extern UINT32 pre_eng_platform_get_chip_id(VOID);
#endif

#endif

