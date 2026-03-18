/*
    ISE module driver

    NT98520 ISE internal header file.

    @file       ise_eng_platform.h
    @ingroup    mIIPPISE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _ISE_ENG_INT_PLATFORM_H_
#define _ISE_ENG_INT_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nvt_api_ver.h>

//-------------------------------------------------------------------------
#if defined (__LINUX)

#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/dma-mapping.h> // header file Dma(cache handle)
#include <linux/slab.h>
/*
#include "mach/rcw_macro.h"
#include "mach/nvt-io.h"
#include <mach/fmem.h>
#include <mach/rcw_macro.h>
#include <plat-na51055/nvt-sramctl.h>
*/
#include "plat/top.h"
#include "kwrap/type.h"
#include "linux/soc/nvt/rcw_macro.h"
#include "linux/soc/nvt/nvt-io.h"
#include "linux/soc/nvt/fmem.h"
#include "plat/nvt-sramctl.h"


//#define	_ISE_REG_BASE_ADDR		_ise_reg_io_base

//#define ISE_SETREG(ofs, value)       iowrite32(value, (void*)(_ISE_REG_BASE_ADDR + ofs))
#define ISE_ENG_GETREG(ofs)          ioread32((void*)(ofs))

//=========================================================================
#elif defined (__FREERTOS)

#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "rcw_macro.h"
#include "io_address.h"
#include "malloc.h"
#include "nvt-sramctl.h"
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#include "dma_protected.h"
#include "plat/top.h"
#include "kwrap/type.h"

//#define	_ISE_REG_BASE_ADDR		_ise_reg_io_base

//#define ISE_SETREG(ofs, value)       OUTW((_ISE_REG_BASE_ADDR + ofs), value);
#define ISE_ENG_GETREG(ofs)          INW(ofs)

#endif

#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/cpu.h"
#include "kwrap/nvt_type.h"
#include "ise_eng_int_dbg.h"
#include "ise_eng.h"


//-------------------------------------------------------------------------


#define FLGPTN_ISE_LLEND      FLGPTN_BIT(0)
#define FLGPTN_ISE_LLERR      FLGPTN_BIT(1)
#define FLGPTN_ISE_FRAMEEND   FLGPTN_BIT(31)

extern void (*ISE_ENG_SETREG)(uintptr_t ofs, UINT32 value);
extern NVT_API_CHK_DECLARE(ise);

extern UINT32 _ise_reg_io_base;


extern ER ise_eng_platform_sem_signal(UINT32 eng_id);

extern ER ise_eng_platform_sem_wait(UINT32 eng_id);

extern void ise_eng_platform_int_enable(VOID);
extern void ise_eng_platform_int_disable(VOID);
extern void ise_eng_platform_enable_clk(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_platform_disable_clk(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_platform_prepare_clk(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_platform_unprepare_clk(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_platform_disable_sram_shutdown(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_platform_enable_sram_shutdown(ISE_ENG_HANDLE *p_eng);
extern INT32 ise_eng_platform_set_clk_rate(ISE_ENG_HANDLE *p_eng);
extern UINT32 ise_eng_platform_get_clk_rate(VOID);

extern ER ise_eng_platform_flg_clear(FLGPTN flg, UINT32 eng_id);
extern ER ise_eng_platform_flg_wait(PFLGPTN p_flgptn, FLGPTN flg, UINT32 eng_id);
extern ER ise_eng_platform_flg_set(FLGPTN flg, UINT32 eng_id);
extern void ise_eng_platform_request_irq(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_platform_release_irq(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_platform_create_resource(VOID);
extern void ise_eng_platform_release_resource(VOID);
extern UINT32 ise_eng_platform_get_chip_id(VOID);

void *ISE_ENG_MALLOC(UINT32 size);
void ISE_ENG_FREE(void *ptr);


#ifdef __cplusplus
}
#endif


#endif // _ISE_ENG_PLATFORM_H_


