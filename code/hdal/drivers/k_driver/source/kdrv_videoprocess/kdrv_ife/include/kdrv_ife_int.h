/*
    IFE module driver

    NT98520 IFE internal header file.

    @file       kdrv_ife_int.h
    @ingroup    mIIPPIFE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _KDRV_IFE_INT_H_
#define _KDRV_IFE_INT_H_

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
#include "ife_eng.h"
#include "kdrv_ife_int_dbg.h"
#include "kdrv_ipp_int_util.h"
#include "kdrv_videoprocess/kdrv_ife.h"

#include "ife_eng_int_platform.h"


#define KDRV_IFE_TIMEOUT_MS (1000)
#define KDRV_IFE_RNG_TH_MAX 1023

typedef enum {
	KDRV_IFE_ISR_STATE_UNKNOWN = 0,
	KDRV_IFE_ISR_STATE_START,
	KDRV_IFE_ISR_STATE_END,
	KDRV_IFE_ISR_STATE_MAX
} KDRV_IFE_ISR_STATE;

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	BOOL auto_gating;
	SEM_HANDLE sem;
	vk_spinlock_t lock;
	IFE_ENG_HANDLE *p_eng;
	void *p_eng_reg_buf;	/* working buffer for ife_eng */
	void *p_eng_flg_buf;	/* working buffer for ife_eng */
	KDRV_IPP_ISR_CB cb;
	KDRV_IFE_ISR_STATE isr_state;
	KDRV_IFE_DBG_INFO *p_ife_dbg_info;
	KDRV_IFE_DBG_CTL ife_dbg_ctl;
	UINT32 *reg_data;
} KDRV_IFE_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	KDRV_IFE_HANDLE *p_hdl;
} KDRV_IFE_CTL;

typedef struct {
	INT32(*set_fp)(KDRV_IFE_HANDLE *p_hdl, void *p_data);
	BOOL set_chk_data;

	INT32(*get_fp)(KDRV_IFE_HANDLE *p_hdl, void *p_data);
	BOOL get_chk_data;

	CHAR *msg;
} KDRV_IFE_FUNC_ITEM;

typedef struct {
	INT32(*query_fp)(UINT32 id, void *p_data);
	CHAR *msg;
} KDRV_IFE_QUERY_ITEM;


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

} KDRV_IFE_LMT;

INT32 kdrv_ife_sys_init(UINT32 chip_num, UINT32 eng_num);
INT32 kdrv_ife_sys_uninit(void);
void *kdrv_ife_os_malloc_wrap(UINT32 want_size);
void kdrv_ife_os_mfree_wrap(void *p_buf);
UINT32 kdrv_ife_do_div(UINT64 n, UINT64 base);
void kdrv_ife_eng_setreg(uintptr_t ofs, UINT32 value);
UINT32 kdrv_ife_eng_getreg(uintptr_t ofs);


#if 0
extern VOID ife_eng_platform_int_enable(VOID);
extern VOID ife_eng_platform_int_disable(VOID);
extern VOID ife_eng_platform_enable_clk(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_disable_clk(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_prepare_clk(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_unprepare_clk(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_disable_sram_shutdown(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_enable_sram_shutdown(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_request_irq(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_release_irq(IFE_ENG_HANDLE *p_eng);
extern INT32 ife_eng_platform_set_clk_rate(IFE_ENG_HANDLE *p_eng);
extern UINT32 ife_eng_platform_get_clk_rate(VOID);
extern UINT32 ife_eng_platform_get_chip_id(VOID);
#endif

#endif

