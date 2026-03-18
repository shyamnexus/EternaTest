/*
    MDBC module driver

    NT98690 MDBC internal header file.

    @file       mdbc_eng_platform.h
    @ingroup    mIIPPMDBC
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _MD_ENG_INT_PLATFORM_H_
#define _MD_ENG_INT_PLATFORM_H_

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
#include "md_eng_handle.h"

#define FLGPTN_MD_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_MD_LL_END			FLGPTN_BIT(8)



void *MD_ENG_MALLOC(UINT32 size);
void MD_ENG_FREE(void *ptr);

void MD_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 MD_ENG_GETREG(uintptr_t oft);

extern VOID md_eng_platform_disable_sram_shutdown(MD_ENG_HANDLE *p_eng);
extern VOID md_eng_platform_enable_sram_shutdown(MD_ENG_HANDLE *p_eng);
extern VOID md_eng_platform_prepare_clk(MD_ENG_HANDLE *p_eng);
extern VOID md_eng_platform_unprepare_clk(MD_ENG_HANDLE *p_eng);
extern VOID md_eng_platform_enable_clk(MD_ENG_HANDLE *p_eng);
extern VOID md_eng_platform_disable_clk(MD_ENG_HANDLE *p_eng);
extern INT32 md_eng_platform_set_clk_rate(MD_ENG_HANDLE *p_eng);
extern ER md_eng_platform_flg_clear(MD_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER md_eng_platform_flg_wait(MD_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER md_eng_platform_flg_wait_timeout(MD_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER md_eng_platform_flg_set(MD_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID md_eng_platform_request_irq(MD_ENG_HANDLE *p_eng);
extern VOID md_eng_platform_release_irq(MD_ENG_HANDLE *p_eng);
extern VOID md_eng_platform_create_resource(MD_ENG_HANDLE *p_eng);
extern VOID md_eng_platform_release_resource(MD_ENG_HANDLE *p_eng);


#ifdef __cplusplus
}
#endif


#endif // _MD_ENG_INT_PLATFORM_H_


