/*
    IFE module driver

    NT98520 IFE internal header file.

    @file       ife_eng_platform.h
    @ingroup    mIIPPIFE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _IFE_ENG_INT_PLATFORM_H_
#define _IFE_ENG_INT_PLATFORM_H_

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



//#define   _IFE_REG_BASE_ADDR      (p_eng->p_ife_reg_st)
//#define IFE_REG_ADDR(ofs)     (_IFE_REG_BASE_ADDR+(ofs))
//#define IFE_SETREG(ofs, value)   iowrite32(value, (void*)(_IFE_REG_BASE_ADDR + ofs))
//#define IFE_GETREG(ofs)          ioread32((void*)(_IFE_REG_BASE_ADDR + ofs))

#define IFE_ENG_GETREG(ofs)          ioread32((void*)(ofs))

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

//#define    _IFE_REG_BASE_ADDR       (p_eng->p_ife_reg_st)
//#define  IFE_REG_ADDR(ofs)        (_IFE_REG_BASE_ADDR+(ofs))
//#define  IFE_SETREG(ofs, value)   OUTW((_IFE_REG_BASE_ADDR + ofs), value);
//#define  IFE_GETREG(ofs)          INW((_IFE_REG_BASE_ADDR + ofs))

#define IFE_ENG_GETREG(ofs)          INW(ofs)

#endif

//extern uintptr_t _ife_reg_io_base;

#include "plat/top.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/cpu.h"
#include "kwrap/nvt_type.h"
#include "ife_eng.h"

extern void (*IFE_ENG_SETREG)(uintptr_t ofs, UINT32 value);
extern NVT_API_CHK_DECLARE(ife);

#define FLGPTN_IFE_FRAMEEND      FLGPTN_BIT(0)
#define FLGPTN_IFE_DEC1_ERR      FLGPTN_BIT(1)
#define FLGPTN_IFE_DEC2_ERR      FLGPTN_BIT(2)
#define FLGPTN_IFE_LLEND         FLGPTN_BIT(3)
#define FLGPTN_IFE_LLERR         FLGPTN_BIT(4)
#define FLGPTN_IFE_LLERR2        FLGPTN_BIT(5)
#define FLGPTN_IFE_LLJOBEND      FLGPTN_BIT(6)
#define FLGPTN_IFE_BUFOVFL       FLGPTN_BIT(7)
#define FLGPTN_IFE_RING_BUF_ERR  FLGPTN_BIT(8)
#define FLGPTN_IFE_FRAME_ERR     FLGPTN_BIT(9)
#define FLGPTN_IFE_SIE_FRM_START FLGPTN_BIT(12)
#define FLGPTN_IFE_SIE2_FRM_START   FLGPTN_BIT(13)
#define FLGPTN_IFE_FRM_START     FLGPTN_BIT(14)

#define FLGPTN_IFE_NN_ISP_P0_SLICE_READY  FLGPTN_BIT(16)
#define FLGPTN_IFE_NN_ISP_P0_SLICE_CLEAR  FLGPTN_BIT(17)
#define FLGPTN_IFE_NN_ISP_P1_SLICE_READY  FLGPTN_BIT(18)
#define FLGPTN_IFE_NN_ISP_P1_SLICE_CLEAR  FLGPTN_BIT(19)

#define IFE_CHIP_A_PLL_13_MAX               450
#define IFE_CHIP_B_PLL_17_MAX               600

//-------------------------------------------------------------------------

extern ER ife_eng_platform_sem_signal(IFE_ENG_HANDLE *p_eng);
extern ER ife_eng_platform_sem_wait(IFE_ENG_HANDLE *p_eng);

extern VOID ife_eng_platform_int_enable(VOID);
extern VOID ife_eng_platform_int_disable(VOID);
extern VOID ife_eng_platform_enable_clk(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_disable_clk(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_prepare_clk(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_unprepare_clk(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_disable_sram_shutdown(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_enable_sram_shutdown(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_request_irq(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_platform_release_irq(IFE_ENG_HANDLE *p_eng);
extern INT32 ife_eng_platform_set_clk_rate(IFE_ENG_HANDLE *p_eng);
extern UINT32 ife_eng_platform_get_clk_rate(VOID);
extern UINT32 ife_eng_platform_get_chip_id(VOID);

extern ER ife_eng_platform_flg_set(UINT32 eng_id, FLGPTN flg);
extern ER ife_eng_platform_flg_clear(UINT32 eng_id, FLGPTN flg);
extern ER ife_eng_platform_flg_wait(UINT32 eng_id, PFLGPTN p_flgptn, FLGPTN flg);
extern VOID ife_eng_platform_create_resource(VOID);
extern VOID ife_eng_platform_release_resource(VOID);

#if defined (_NVT_EMULATION_)
extern BOOL ife_end_time_out_status;
#endif

void *IFE_ENG_MALLOC(UINT32 size);
void IFE_ENG_FREE(void *ptr);
#ifdef __cplusplus
}
#endif


#endif // _IFE_ENG_INT_PLATFORM_H_


