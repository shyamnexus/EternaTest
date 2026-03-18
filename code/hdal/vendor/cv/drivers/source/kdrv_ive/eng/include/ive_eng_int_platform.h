/*
    IVE module driver

    NT98530 IVE internal header file.

    @file       ive_eng_platform.h
    @ingroup    mIIPPIVE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _IVE_ENG_INT_PLATFORM_H_
#define _IVE_ENG_INT_PLATFORM_H_

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
#include "ive_eng_handle.h"

#define FLGPTN_IVE_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_IVE_LL_END			FLGPTN_BIT(8)



void *IVE_ENG_MALLOC(UINT32 size);
void IVE_ENG_FREE(void *ptr);

void IVE_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 IVE_ENG_GETREG(uintptr_t oft);

extern VOID ive_eng_platform_disable_sram_shutdown(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_platform_enable_sram_shutdown(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_platform_prepare_clk(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_platform_unprepare_clk(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_platform_enable_clk(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_platform_disable_clk(IVE_ENG_HANDLE *p_eng);
extern INT32 ive_eng_platform_set_clk_rate(IVE_ENG_HANDLE *p_eng);
extern ER ive_eng_platform_flg_clear(IVE_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER ive_eng_platform_flg_wait(IVE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER ive_eng_platform_flg_wait_timeout(IVE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER ive_eng_platform_flg_set(IVE_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID ive_eng_platform_request_irq(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_platform_release_irq(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_platform_create_resource(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_platform_release_resource(IVE_ENG_HANDLE *p_eng);


#ifdef __cplusplus
}
#endif


#endif // _IVE_ENG_INT_PLATFORM_H_


