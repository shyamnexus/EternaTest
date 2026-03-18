/*
    JMISP module driver

    NT98530 JMISP internal header file.

    @file       jmisp_eng_platform.h
    @ingjmispp    mIIPPJMISP
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _JMISP_ENG_INT_PLATFORM_H_
#define _JMISP_ENG_INT_PLATFORM_H_

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
#include "jmisp_eng_handle.h"

#define FLGPTN_JMISP_PL0_END			FLGPTN_BIT(0)
#define FLGPTN_JMISP_PL1_END			FLGPTN_BIT(1)
#define FLGPTN_JMISP_PL2_END			FLGPTN_BIT(2)
#define FLGPTN_JMISP_PL3_END			FLGPTN_BIT(3)

void *JMISP_ENG_MALLOC(UINT32 size);
void JMISP_ENG_FREE(void *ptr);

void JMISP_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 JMISP_ENG_GETREG(uintptr_t oft);
/*
extern VOID jmisp_eng_platform_disable_sram_shutdown(JMISP_ENG_HANDLE *p_eng);
extern VOID jmisp_eng_platform_enable_sram_shutdown(JMISP_ENG_HANDLE *p_eng);
extern VOID jmisp_eng_platform_prepare_clk(JMISP_ENG_HANDLE *p_eng);
extern VOID jmisp_eng_platform_unprepare_clk(JMISP_ENG_HANDLE *p_eng);
extern VOID jmisp_eng_platform_enable_clk(JMISP_ENG_HANDLE *p_eng);
extern VOID jmisp_eng_platform_disable_clk(JMISP_ENG_HANDLE *p_eng);
extern INT32 jmisp_eng_platform_set_clk_rate(JMISP_ENG_HANDLE *p_eng);
*/
extern ER jmisp_eng_platform_flg_clear(JMISP_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER jmisp_eng_platform_flg_wait(JMISP_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER jmisp_eng_platform_flg_wait_timeout(JMISP_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER jmisp_eng_platform_flg_set(JMISP_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID jmisp_eng_platform_request_irq(JMISP_ENG_HANDLE *p_eng);
extern VOID jmisp_eng_platform_release_irq(JMISP_ENG_HANDLE *p_eng);
extern VOID jmisp_eng_platform_create_resource(JMISP_ENG_HANDLE *p_eng);
extern VOID jmisp_eng_platform_release_resource(JMISP_ENG_HANDLE *p_eng);
extern INT32 jmisp_eng_platform_dma_idle(JMISP_ENG_HANDLE *p_eng);

#ifdef __cplusplus
}
#endif


#endif // _JMISP_ENG_INT_PLATFORM_H_


