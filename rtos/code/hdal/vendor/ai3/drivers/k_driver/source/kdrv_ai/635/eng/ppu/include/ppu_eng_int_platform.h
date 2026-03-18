/*
    PPU module driver

    NT98538 PPU internal header file.

    @file       ppu_eng_platform.h
    @ingroup    mIIPPPPU
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _PPU_ENG_INT_PLATFORM_H_
#define _PPU_ENG_INT_PLATFORM_H_

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
#include "ppu_eng_handle.h"

#define FLGPTN_PPU_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_PPU_LL_END			FLGPTN_BIT(24)



void *PPU_ENG_MALLOC(UINT32 size);
void PPU_ENG_FREE(void *ptr);

void PPU_ENG_SETREG(uintptr_t ofs, UINT32 value);
UINT32 PPU_ENG_GETREG(uintptr_t oft);

extern VOID ppu_eng_platform_disable_sram_shutdown(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_platform_enable_sram_shutdown(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_platform_prepare_clk(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_platform_unprepare_clk(PPU_ENG_HANDLE *p_eng);
extern INT32 ppu_eng_platform_enable_clk(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_platform_disable_clk(PPU_ENG_HANDLE *p_eng);
extern INT32 ppu_eng_platform_set_clk_rate(PPU_ENG_HANDLE *p_eng);
extern ER ppu_eng_platform_flg_clear(PPU_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER ppu_eng_platform_flg_wait(PPU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER ppu_eng_platform_flg_wait_timeout(PPU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER ppu_eng_platform_flg_set(PPU_ENG_HANDLE *p_eng, FLGPTN flg);
extern VOID ppu_eng_platform_request_irq(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_platform_release_irq(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_platform_create_resource(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_platform_release_resource(PPU_ENG_HANDLE *p_eng);
extern INT32 ppu_eng_platform_dma_idle(PPU_ENG_HANDLE *p_eng);
extern UINT32 ppu_eng_platform_get_clk_rate(PPU_ENG_HANDLE *p_eng);

#ifdef __cplusplus
}
#endif


#endif // _PPU_ENG_INT_PLATFORM_H_


