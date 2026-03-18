/*
    PRE module driver

    NT98520 PRE internal header file.

    @file       pre_eng_platform.h
    @ingroup    mIIPPPRE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _PRE_ENG_INT_PLATFORM_H_
#define _PRE_ENG_INT_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nvt_api_ver.h>

#define pmc_turnonPower(a)
#define pmc_turnoffPower(a)

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
#include "linux/soc/nvt/rcw_macro.h"
#include "linux/soc/nvt/nvt-io.h"
#include "linux/soc/nvt/fmem.h"
#include "plat/nvt-sramctl.h"
#include "kwrap/type.h"

#define PRE_ENG_GETREG(ofs)          ioread32((void*)(ofs))

//=========================================================================
#elif defined (__FREERTOS)

#include "string.h"
#include <stdlib.h>
#include "rcw_macro.h"
#include "io_address.h"
#include "malloc.h"
#include "nvt-sramctl.h"
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#include "dma_protected.h"

#define PRE_ENG_GETREG(ofs)          INW(ofs)

#endif

//extern uintptr_t _pre_reg_io_base;

#include "plat/top.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/cpu.h"
#include "kwrap/nvt_type.h"
#include "pre_eng.h"

extern void (*PRE_ENG_SETREG)(uintptr_t ofs, UINT32 value);
extern NVT_API_CHK_DECLARE(pre);

#define FLGPTN_PRE_FRAMEEND      FLGPTN_BIT(0)
#define FLGPTN_PRE_DEC1_ERR      FLGPTN_BIT(1)
#define FLGPTN_PRE_DEC2_ERR      FLGPTN_BIT(2)
#define FLGPTN_PRE_LLEND         FLGPTN_BIT(3)
#define FLGPTN_PRE_LLERR         FLGPTN_BIT(4)
#define FLGPTN_PRE_LLERR2        FLGPTN_BIT(5)
#define FLGPTN_PRE_LLJOBEND      FLGPTN_BIT(6)
#define FLGPTN_PRE_BUFOVFL       FLGPTN_BIT(7)
#define FLGPTN_PRE_RING_BUF_ERR  FLGPTN_BIT(8)
#define FLGPTN_PRE_FRAME_ERR     FLGPTN_BIT(9)
#define FLGPTN_PRE_SIE_FRM_START FLGPTN_BIT(12)
#define FLGPTN_PRE_SIE2_FRM_START   FLGPTN_BIT(13)
#define FLGPTN_PRE_FRM_START     FLGPTN_BIT(14)



//-------------------------------------------------------------------------

extern ER pre_eng_platform_sem_signal(PRE_ENG_HANDLE *p_eng);
extern ER pre_eng_platform_sem_wait(PRE_ENG_HANDLE *p_eng);

extern VOID pre_eng_platform_int_enable(VOID);
extern VOID pre_eng_platform_int_disable(VOID);
extern VOID pre_eng_platform_enable_clk(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_disable_clk(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_prepare_clk(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_unprepare_clk(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_disable_sram_shutdown(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_enable_sram_shutdown(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_request_irq(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_platform_release_irq(PRE_ENG_HANDLE *p_eng);
extern INT32 pre_eng_platform_set_clk_rate(PRE_ENG_HANDLE *p_eng);
extern UINT32 pre_eng_platform_get_clk_rate(VOID);
extern UINT32 pre_eng_platform_get_chip_id(VOID);

extern ER pre_eng_platform_flg_set(UINT32 eng_id, FLGPTN flg);
extern ER pre_eng_platform_flg_clear(UINT32 eng_id, FLGPTN flg);
extern ER pre_eng_platform_flg_wait(UINT32 eng_id, PFLGPTN p_flgptn, FLGPTN flg);
extern VOID pre_eng_platform_create_resource(VOID);
extern VOID pre_eng_platform_release_resource(VOID);

#if defined (_NVT_EMULATION_)
extern BOOL pre_end_time_out_status;
#endif

void *PRE_ENG_MALLOC(UINT32 size);
void PRE_ENG_FREE(void *ptr);
#ifdef __cplusplus
}
#endif


#endif // _PRE_ENG_INT_PLATFORM_H_


