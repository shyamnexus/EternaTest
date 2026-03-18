/*
    ROU module driver

    NT98690 ROU internal header file.

    @file       rou_eng_platform.h
    @ingroup    mIIPPROU
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef _ROU_ENG_INT_PLATFORM_H_
#define _ROU_ENG_INT_PLATFORM_H_

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
#include "rou_eng_handle.h"

#define FLGPTN_ROU_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_ROU_LL_END			FLGPTN_BIT(24)



void *ROU_ENG_MALLOC(UINT32 size);
void ROU_ENG_FREE(void *ptr);

void ROU_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 ROU_ENG_GETREG(uintptr_t oft);

extern VOID rou_eng_platform_disable_sram_shutdown(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_platform_enable_sram_shutdown(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_platform_prepare_clk(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_platform_unprepare_clk(ROU_ENG_HANDLE *p_eng);
extern INT32 rou_eng_platform_enable_clk(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_platform_disable_clk(ROU_ENG_HANDLE *p_eng);
extern INT32 rou_eng_platform_set_clk_rate(ROU_ENG_HANDLE *p_eng);
extern ER rou_eng_platform_flg_clear(ROU_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER rou_eng_platform_flg_wait(ROU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER rou_eng_platform_flg_wait_timeout(ROU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER rou_eng_platform_flg_set(ROU_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID rou_eng_platform_request_irq(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_platform_release_irq(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_platform_create_resource(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_platform_release_resource(ROU_ENG_HANDLE *p_eng);
extern INT32 rou_eng_platform_dma_idle(ROU_ENG_HANDLE *p_eng);
extern UINT32 rou_eng_platform_get_clk_rate(ROU_ENG_HANDLE *p_eng);

#ifdef __cplusplus
}
#endif


#endif // _ROU_ENG_INT_PLATFORM_H_


