/*
    UTIL module driver

    NT98538 UTIL internal header file.

    @file       util_eng_platform.h
    @ingutilp    mIIPPUTIL
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _UTIL_ENG_INT_PLATFORM_H_
#define _UTIL_ENG_INT_PLATFORM_H_

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
#include "util_eng_handle.h"

#define FLGPTN_UTIL_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_UTIL_LL0_END			FLGPTN_BIT(24)
#define FLGPTN_UTIL_LL1_END			FLGPTN_BIT(28)


void *UTIL_ENG_MALLOC(UINT32 size);
void UTIL_ENG_FREE(void *ptr);

void UTIL_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 UTIL_ENG_GETREG(uintptr_t oft);

extern VOID util_eng_platform_disable_sram_shutdown(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_platform_enable_sram_shutdown(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_platform_prepare_clk(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_platform_unprepare_clk(UTIL_ENG_HANDLE *p_eng);
extern INT32 util_eng_platform_enable_clk(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_platform_disable_clk(UTIL_ENG_HANDLE *p_eng);
extern INT32 util_eng_platform_set_clk_rate(UTIL_ENG_HANDLE *p_eng);
extern ER util_eng_platform_flg_clear(UTIL_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER util_eng_platform_flg_wait(UTIL_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER util_eng_platform_flg_wait_timeout(UTIL_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER util_eng_platform_flg_set(UTIL_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID util_eng_platform_request_irq(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_platform_release_irq(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_platform_create_resource(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_platform_release_resource(UTIL_ENG_HANDLE *p_eng);
extern INT32 util_eng_platform_dma_idle(UTIL_ENG_HANDLE *p_eng);
extern UINT32 util_eng_platform_get_clk_rate(UTIL_ENG_HANDLE *p_eng);

#ifdef __cplusplus
}
#endif


#endif // _UTIL_ENG_INT_PLATFORM_H_

