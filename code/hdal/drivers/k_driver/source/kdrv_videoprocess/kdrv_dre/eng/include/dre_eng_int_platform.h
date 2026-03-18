/*
    DRE module driver

    NT98520 DRE internal header file.

    @file       dre_eng_platform.h
    @ingroup    mIIPPDRE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _DRE_ENG_INT_PLATFORM_H_
#define _DRE_ENG_INT_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define pmc_turnonPower(a)
#define pmc_turnoffPower(a)
#include "nvt_api_ver.h"

//-------------------------------------------------------------------------
#if defined (__LINUX)

#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/dma-mapping.h> // header file Dma(cache handle)
#include <linux/slab.h>
#include <linux/interrupt.h>

#include "linux/soc/nvt/rcw_macro.h"
#include "linux/soc/nvt/nvt-io.h"
#include "linux/soc/nvt/fmem.h"
#include "plat/nvt-sramctl.h"
#include "kwrap/type.h"


#define DRE_ENG_GETREG(ofs)          ioread32((void*)(ofs))

//=========================================================================
#elif defined (__FREERTOS)

#include "string.h"
#include <stdlib.h>
#include "rcw_macro.h"
#include "io_address.h"
#include "malloc.h"
#include "interrupt.h"
#include "nvt-sramctl.h"
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#include "dma_protected.h"
#include "kwrap/type.h"


#define DRE_ENG_GETREG(ofs)          INW(ofs)

#endif

extern void (*DRE_ENG_SETREG)(uintptr_t ofs, UINT32 value);
extern NVT_API_CHK_DECLARE(dre);


#include "plat/top.h"
//#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/cpu.h"
#include "kwrap/nvt_type.h"
#include "dre_eng.h"

// interrupt flag
#define FLGPTN_DRE_FRAME_END	FLGPTN_BIT(0)
#define FLGPTN_DRE_TIME_OUT		FLGPTN_BIT(1)
#define FLGPTN_DRE_LL_END		FLGPTN_BIT(2)
#define FLGPTN_DRE_LL_ERROR	    FLGPTN_BIT(3)
#define FLGPTN_DRE_LL_JOBEND	FLGPTN_BIT(4)

//-------------------------------------------------------------------------
extern VOID dre_eng_platform_int_enable(VOID);
extern VOID dre_eng_platform_int_disable(VOID);
extern VOID dre_eng_platform_enable_clk(DRE_ENG_HANDLE *p_eng);
extern VOID dre_eng_platform_disable_clk(DRE_ENG_HANDLE *p_eng);
extern VOID dre_eng_platform_prepare_clk(DRE_ENG_HANDLE *p_eng);
extern VOID dre_eng_platform_unprepare_clk(DRE_ENG_HANDLE *p_eng);
extern VOID dre_eng_platform_disable_sram_shutdown(DRE_ENG_HANDLE *p_eng);
extern VOID dre_eng_platform_enable_sram_shutdown(DRE_ENG_HANDLE *p_eng);
extern INT32 dre_eng_platform_set_clk_rate(DRE_ENG_HANDLE *p_eng);
extern UINT32 dre_eng_platform_get_clk_rate(VOID);
extern UINT32 dre_eng_platform_get_chip_id(VOID);
extern UINT64 dre_eng_do_64b_div(UINT64 dividend, UINT64 divisor);

extern VOID dre_eng_platform_create_resource(VOID);
extern VOID dre_eng_platform_release_resource(VOID);
extern ER dre_eng_platform_sem_wait(VOID);
extern ER dre_eng_platform_sem_signal(VOID);
extern ER dre_eng_platform_flg_clear(FLGPTN flg);
extern ER dre_eng_platform_flg_wait(PFLGPTN p_flgptn, FLGPTN flg);
extern ER dre_eng_platform_flg_set(FLGPTN flg);
extern VOID dre_eng_platform_request_irq(DRE_ENG_HANDLE *p_eng);
extern VOID dre_eng_platform_release_irq(DRE_ENG_HANDLE *p_eng);

#if defined (_NVT_EMULATION_)
extern BOOL dre_end_time_out_status;
#endif

extern void *dre_eng_malloc(UINT32 size);
extern void dre_eng_free(void *ptr);

//#endif
#ifdef __cplusplus
}
#endif


#endif // _DRE_ENG_PLATFORM_H_


