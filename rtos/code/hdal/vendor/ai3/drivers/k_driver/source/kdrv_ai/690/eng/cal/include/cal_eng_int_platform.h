/*
    CAL module driver

    NT98690 CAL internal header file.

    @file       cal_eng_platform.h
    @ingroup    mIIPPCAL
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _CAL_ENG_INT_PLATFORM_H_
#define _CAL_ENG_INT_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__LINUX)
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/dma-mapping.h> // header file Dma(cache handle)
#include <linux/slab.h>
#elif defined (__FREERTOS)
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "malloc.h"
#endif

#include "plat/top.h"
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/cpu.h"
#include "kwrap/nvt_type.h"
#include "kwrap/mem.h"
#include "cal_eng_handle.h"

#define FLGPTN_CAL_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_CAL_LL_END			FLGPTN_BIT(24)



void *CAL_ENG_MALLOC(UINT32 size);
void CAL_ENG_FREE(void *ptr);

void CAL_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 CAL_ENG_GETREG(uintptr_t oft);

extern VOID cal_eng_platform_disable_sram_shutdown(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_platform_enable_sram_shutdown(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_platform_prepare_clk(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_platform_unprepare_clk(CAL_ENG_HANDLE *p_eng);
extern INT32 cal_eng_platform_enable_clk(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_platform_disable_clk(CAL_ENG_HANDLE *p_eng);
extern INT32 cal_eng_platform_set_clk_rate(CAL_ENG_HANDLE *p_eng);
extern ER cal_eng_platform_flg_clear(CAL_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER cal_eng_platform_flg_wait(CAL_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER cal_eng_platform_flg_wait_timeout(CAL_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER cal_eng_platform_flg_set(CAL_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID cal_eng_platform_request_irq(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_platform_release_irq(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_platform_create_resource(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_platform_release_resource(CAL_ENG_HANDLE *p_eng);
extern INT32 cal_eng_platform_dma_idle(CAL_ENG_HANDLE *p_eng);
extern UINT32 cal_eng_platform_get_clk_rate(CAL_ENG_HANDLE *p_eng);

#ifdef __cplusplus
}
#endif


#endif // _CAL_ENG_INT_PLATFORM_H_
