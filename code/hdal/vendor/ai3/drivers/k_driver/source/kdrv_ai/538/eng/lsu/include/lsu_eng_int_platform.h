/*
    LSU module driver

    NT98538 LSU internal header file.

    @file       lsu_eng_platform.h
    @inglsup    mIIPPLSU
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _LSU_ENG_INT_PLATFORM_H_
#define _LSU_ENG_INT_PLATFORM_H_

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
#include "lsu_eng_handle.h"

#define FLGPTN_LSU_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_LSU_LL_END			FLGPTN_BIT(24)



void *LSU_ENG_MALLOC(UINT32 size);
void LSU_ENG_FREE(void *ptr);

void LSU_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 LSU_ENG_GETREG(uintptr_t oft);

extern VOID lsu_eng_platform_disable_sram_shutdown(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_platform_enable_sram_shutdown(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_platform_prepare_clk(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_platform_unprepare_clk(LSU_ENG_HANDLE *p_eng);
extern INT32 lsu_eng_platform_enable_clk(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_platform_disable_clk(LSU_ENG_HANDLE *p_eng);
extern INT32 lsu_eng_platform_set_clk_rate(LSU_ENG_HANDLE *p_eng);
extern ER lsu_eng_platform_flg_clear(LSU_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER lsu_eng_platform_flg_wait(LSU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER lsu_eng_platform_flg_wait_timeout(LSU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER lsu_eng_platform_flg_set(LSU_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID lsu_eng_platform_request_irq(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_platform_release_irq(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_platform_create_resource(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_platform_release_resource(LSU_ENG_HANDLE *p_eng);
extern INT32 lsu_eng_platform_dma_idle(LSU_ENG_HANDLE *p_eng);
extern UINT32 lsu_eng_platform_get_clk_rate(LSU_ENG_HANDLE *p_eng);

#ifdef __cplusplus
}
#endif


#endif // _LSU_ENG_INT_PLATFORM_H_

