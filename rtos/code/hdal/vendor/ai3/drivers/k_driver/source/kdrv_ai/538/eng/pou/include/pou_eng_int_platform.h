/*
    POU module driver

    NT98539A POU internal header file.

    @file       pou_eng_platform.h
    @ingpoup    mIIPPPOU
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/
#ifndef _POU_ENG_INT_PLATFORM_H_
#define _POU_ENG_INT_PLATFORM_H_

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
#include "pou_eng_handle.h"

#define FLGPTN_POU_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_POU_LL_END			FLGPTN_BIT(24)


void *POU_ENG_MALLOC(UINT32 size);
void POU_ENG_FREE(void *ptr);

void POU_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 POU_ENG_GETREG(uintptr_t oft);

extern VOID pou_eng_platform_disable_sram_shutdown(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_platform_enable_sram_shutdown(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_platform_prepare_clk(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_platform_unprepare_clk(POU_ENG_HANDLE *p_eng);
extern INT32 pou_eng_platform_enable_clk(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_platform_disable_clk(POU_ENG_HANDLE *p_eng);
extern INT32 pou_eng_platform_set_clk_rate(POU_ENG_HANDLE *p_eng);
extern ER pou_eng_platform_flg_clear(POU_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER pou_eng_platform_flg_wait(POU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER pou_eng_platform_flg_wait_timeout(POU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER pou_eng_platform_flg_set(POU_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID pou_eng_platform_request_irq(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_platform_release_irq(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_platform_create_resource(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_platform_release_resource(POU_ENG_HANDLE *p_eng);
extern INT32 pou_eng_platform_dma_idle(POU_ENG_HANDLE *p_eng);
extern UINT32 pou_eng_platform_get_clk_rate(POU_ENG_HANDLE *p_eng);

#ifdef __cplusplus
}
#endif

#endif // _POU_ENG_INT_PLATFORM_H_

