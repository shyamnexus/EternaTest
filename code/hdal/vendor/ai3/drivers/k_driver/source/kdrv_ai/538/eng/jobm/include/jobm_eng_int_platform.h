/*
    JOBM module driver

    NT98530 JOBM internal header file.

    @file       jobm_eng_platform.h
    @ingjobmp    mIIPPJOBM
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _JOBM_ENG_INT_PLATFORM_H_
#define _JOBM_ENG_INT_PLATFORM_H_

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
#include "jobm_eng_handle.h"

#define FLGPTN_JOBM_JLA_END			FLGPTN_BIT(0)
#define FLGPTN_JOBM_JLB_END			FLGPTN_BIT(1)
#define FLGPTN_JOBM_JLC_END			FLGPTN_BIT(2)
#define FLGPTN_JOBM_JLD_END			FLGPTN_BIT(3)
#define FLGPTN_JOBM_JLE_END			FLGPTN_BIT(4)

void *JOBM_ENG_MALLOC(UINT32 size);
void JOBM_ENG_FREE(void *ptr);

void JOBM_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 JOBM_ENG_GETREG(uintptr_t oft);
/*
extern VOID jobm_eng_platform_disable_sram_shutdown(JOBM_ENG_HANDLE *p_eng);
extern VOID jobm_eng_platform_enable_sram_shutdown(JOBM_ENG_HANDLE *p_eng);
extern VOID jobm_eng_platform_prepare_clk(JOBM_ENG_HANDLE *p_eng);
extern VOID jobm_eng_platform_unprepare_clk(JOBM_ENG_HANDLE *p_eng);
extern VOID jobm_eng_platform_enable_clk(JOBM_ENG_HANDLE *p_eng);
extern VOID jobm_eng_platform_disable_clk(JOBM_ENG_HANDLE *p_eng);
extern INT32 jobm_eng_platform_set_clk_rate(JOBM_ENG_HANDLE *p_eng);
*/
extern ER jobm_eng_platform_flg_clear(JOBM_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER jobm_eng_platform_flg_wait(JOBM_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER jobm_eng_platform_flg_wait_timeout(JOBM_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER jobm_eng_platform_flg_set(JOBM_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID jobm_eng_platform_request_irq(JOBM_ENG_HANDLE *p_eng);
extern VOID jobm_eng_platform_release_irq(JOBM_ENG_HANDLE *p_eng);
extern VOID jobm_eng_platform_create_resource(JOBM_ENG_HANDLE *p_eng);
extern VOID jobm_eng_platform_release_resource(JOBM_ENG_HANDLE *p_eng);
extern INT32 jobm_eng_platform_dma_idle(JOBM_ENG_HANDLE *p_eng);

#ifdef __cplusplus
}
#endif


#endif // _JOBM_ENG_INT_PLATFORM_H_


