/*
    NUE2 module driver

    NT98690 NUE2 internal header file.

    @file       nue2_eng_platform.h
    @ingnue2p    mIIPPNUE2
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _NUE2_ENG_INT_PLATFORM_H_
#define _NUE2_ENG_INT_PLATFORM_H_

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
#include "nue2_eng_handle.h"

#define FLGPTN_NUE2_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_NUE2_LL_END			FLGPTN_BIT(8)



void *NUE2_ENG_MALLOC(UINT32 size);
void NUE2_ENG_FREE(void *ptr);

void NUE2_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 NUE2_ENG_GETREG(uintptr_t oft);

extern VOID nue2_eng_platform_disable_sram_shutdown(NUE2_ENG_HANDLE *p_eng);
extern VOID nue2_eng_platform_enable_sram_shutdown(NUE2_ENG_HANDLE *p_eng);
extern VOID nue2_eng_platform_prepare_clk(NUE2_ENG_HANDLE *p_eng);
extern VOID nue2_eng_platform_unprepare_clk(NUE2_ENG_HANDLE *p_eng);
extern INT32 nue2_eng_platform_enable_clk(NUE2_ENG_HANDLE *p_eng);
extern VOID nue2_eng_platform_disable_clk(NUE2_ENG_HANDLE *p_eng);
extern INT32 nue2_eng_platform_set_clk_rate(NUE2_ENG_HANDLE *p_eng);
extern ER nue2_eng_platform_flg_clear(NUE2_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER nue2_eng_platform_flg_wait(NUE2_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER nue2_eng_platform_flg_wait_timeout(NUE2_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER nue2_eng_platform_flg_set(NUE2_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID nue2_eng_platform_request_irq(NUE2_ENG_HANDLE *p_eng);
extern VOID nue2_eng_platform_release_irq(NUE2_ENG_HANDLE *p_eng);
extern VOID nue2_eng_platform_create_resource(NUE2_ENG_HANDLE *p_eng);
extern VOID nue2_eng_platform_release_resource(NUE2_ENG_HANDLE *p_eng);
extern INT32 nue2_eng_platform_dma_idle(NUE2_ENG_HANDLE *p_eng);
extern UINT32 nue2_eng_platform_get_clk_rate(NUE2_ENG_HANDLE *p_eng);

#ifdef __cplusplus
}
#endif


#endif // _NUE2_ENG_INT_PLATFORM_H_


