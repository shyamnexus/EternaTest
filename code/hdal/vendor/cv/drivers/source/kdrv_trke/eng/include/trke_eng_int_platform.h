/*
    TRKE module driver

    NT98530 TRKE internal header file.

    @file       trke_eng_platform.h
    @ingroup    mIIPPTRKE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _TRKE_ENG_INT_PLATFORM_H_
#define _TRKE_ENG_INT_PLATFORM_H_

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
#include "nvt-sramctl.h"
#include "pll_protected.h"
#endif

#include "plat/top.h"
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/cpu.h"
#include "kwrap/nvt_type.h"
#include "kwrap/mem.h"
#include "trke_eng_handle.h"

#define FLGPTN_TRKE_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_TRKE_LL_END			FLGPTN_BIT(8)



void *TRKE_ENG_MALLOC(UINT32 size);
void TRKE_ENG_FREE(void *ptr);

void TRKE_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 TRKE_ENG_GETREG(uintptr_t oft);

extern VOID trke_eng_platform_disable_sram_shutdown(TRKE_ENG_HANDLE *p_eng);
extern VOID trke_eng_platform_enable_sram_shutdown(TRKE_ENG_HANDLE *p_eng);
extern VOID trke_eng_platform_prepare_clk(TRKE_ENG_HANDLE *p_eng);
extern VOID trke_eng_platform_unprepare_clk(TRKE_ENG_HANDLE *p_eng);
extern VOID trke_eng_platform_enable_clk(TRKE_ENG_HANDLE *p_eng);
extern VOID trke_eng_platform_disable_clk(TRKE_ENG_HANDLE *p_eng);
extern INT32 trke_eng_platform_set_clk_rate(TRKE_ENG_HANDLE *p_eng);
extern ER trke_eng_platform_flg_clear(TRKE_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER trke_eng_platform_flg_wait(TRKE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER trke_eng_platform_flg_wait_timeout(TRKE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER trke_eng_platform_flg_set(TRKE_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID trke_eng_platform_request_irq(TRKE_ENG_HANDLE *p_eng);
extern VOID trke_eng_platform_release_irq(TRKE_ENG_HANDLE *p_eng);
extern VOID trke_eng_platform_create_resource(TRKE_ENG_HANDLE *p_eng);
extern VOID trke_eng_platform_release_resource(TRKE_ENG_HANDLE *p_eng);


#ifdef __cplusplus
}
#endif


#endif // _TRKE_ENG_INT_PLATFORM_H_


