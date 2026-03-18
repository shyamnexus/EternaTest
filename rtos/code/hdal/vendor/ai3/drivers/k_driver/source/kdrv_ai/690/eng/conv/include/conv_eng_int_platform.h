/*
    CONV module driver

    NT98690 CONV internal header file.

    @file       conv_eng_platform.h
    @ingconvp    mIIPPCONV
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _CONV_ENG_INT_PLATFORM_H_
#define _CONV_ENG_INT_PLATFORM_H_

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
#include "conv_eng_handle.h"

#define FLGPTN_CONV_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_CONV_LL_END			FLGPTN_BIT(24)


void *CONV_ENG_MALLOC(UINT32 size);
void CONV_ENG_FREE(void *ptr);

void CONV_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 CONV_ENG_GETREG(uintptr_t oft);

extern VOID conv_eng_platform_disable_sram_shutdown(CONV_ENG_HANDLE *p_eng);
extern VOID conv_eng_platform_enable_sram_shutdown(CONV_ENG_HANDLE *p_eng);
extern VOID conv_eng_platform_prepare_clk(CONV_ENG_HANDLE *p_eng);
extern VOID conv_eng_platform_unprepare_clk(CONV_ENG_HANDLE *p_eng);
extern INT32 conv_eng_platform_enable_clk(CONV_ENG_HANDLE *p_eng);
extern VOID conv_eng_platform_disable_clk(CONV_ENG_HANDLE *p_eng);
extern UINT32 conv_eng_platform_get_clk_rate(CONV_ENG_HANDLE *p_eng);
extern INT32 conv_eng_platform_set_clk_rate(CONV_ENG_HANDLE *p_eng);
extern ER conv_eng_platform_flg_clear(CONV_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER conv_eng_platform_flg_wait(CONV_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER conv_eng_platform_flg_wait_timeout(CONV_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER conv_eng_platform_flg_set(CONV_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID conv_eng_platform_request_irq(CONV_ENG_HANDLE *p_eng);
extern VOID conv_eng_platform_release_irq(CONV_ENG_HANDLE *p_eng);
extern VOID conv_eng_platform_create_resource(CONV_ENG_HANDLE *p_eng);
extern VOID conv_eng_platform_release_resource(CONV_ENG_HANDLE *p_eng);
extern INT32 conv_eng_platform_dma_idle(CONV_ENG_HANDLE *p_eng);


#ifdef __cplusplus
}
#endif


#endif // _CONV_ENG_INT_PLATFORM_H_


