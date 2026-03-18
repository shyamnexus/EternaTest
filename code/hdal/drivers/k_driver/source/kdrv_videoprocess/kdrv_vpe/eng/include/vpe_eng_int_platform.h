/*
    VPE module driver

    NT98636 VPE internal header file.

    @file       vpe_eng_int_platform.h
    @ingroup    mIVPE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _VPE_ENG_INT_PLATFORM_H_
#define _VPE_ENG_INT_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvt_api_ver.h"

//-------------------------------------------------------------------------
#if defined (__LINUX)

#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/dma-mapping.h> // header file Dma(cache handle)
#include <linux/slab.h>
//#include <plat-na51102/nvt-sramctl.h>


#if defined(__aarch64__)
#include "linux/soc/nvt/rcw_macro.h"
#else
#include "rcw_macro.h"
#endif
//#include "mach/nvt-io.h"
//#include <mach/fmem.h>
//#include <mach/rcw_macro.h>
//#include <plat-na51068/nvt-sramctl.h>

#include "plat/top.h"
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/cpu.h"
#include "kwrap/nvt_type.h"
#include "kwrap/mem.h"

//#include "kwrap/error_no.h"

//extern uintptr_t _vpe_reg_io_base[VPE_ID_MAX_NUM];

//#define _VPE_REG_BASE_ADDR(eng_id)  _vpe_reg_io_base[eng_id]
//#define VPE_REG_ADDR(eng_id, ofs)           (_VPE_REG_BASE_ADDR(eng_id)+(ofs))
//#define VPE_SETREG(eng_id, ofs, value)      iowrite32(value, (void*)(_VPE_REG_BASE_ADDR(eng_id) + ofs))
//#define VPE_GETREG(eng_id, ofs)             ioread32((void*)(_VPE_REG_BASE_ADDR(eng_id) + ofs))

#define VPE_ENG_GETREG(ofs)          ioread32((void*)(ofs))

#define VPE_ENG_MALLOC(size)	kmalloc((size), GFP_KERNEL)
#define VPE_ENG_FREE(ptr)		kfree(ptr)


//=========================================================================
#elif defined (__FREERTOS)

#include "string.h"
#include <stdlib.h>
#include "rcw_macro.h"
#include "io_address.h"

#if 1//defined(_BSP_NA51068_)
#include "nvt-sramctl.h"
#endif

#include "interrupt.h"
//#include "pll.h"
#include "pll_protected.h"
#include "dma_protected.h"
#include "top.h"
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/cpu.h"
#include "kwrap/nvt_type.h"
#include "kwrap/mem.h"

#define VPE_ENG_GETREG(ofs)          INW(ofs)

#define VPE_ENG_MALLOC(size)	malloc((size))
#define VPE_ENG_FREE(ptr)		free(ptr)

#endif

extern void (*VPE_ENG_SETREG)(uintptr_t ofs, UINT32 value);
extern NVT_API_CHK_DECLARE(vpe);

//-------------------------------------------------------------------------
#include "vpe_eng.h"
#include "vpe_eng_int_dbg.h"

#define FLGPTN_VPE_FRM_END			FLGPTN_BIT(0)
#define FLGPTN_VPE_LL_END			FLGPTN_BIT(2)
#define VPE_ENG_538_STANDARD_RATE        450 //450Mhz
#define VPE_ENG_539A_STANDARD_RATE       500 //500Mhz
#define VPE_ENG_539A_OVER_STANDARD_RATE       600 //600Mhz

/*
#define VPE_DEF(RCW)            t##RCW
#define VPE_INIT_VAL(RCW)       t##RCW.reg = 0
#define VPE_OF(RCW)             t##RCW.bit
#define VPE_VAL(RCW)            t##RCW.reg
#define VPE_ST(RCW)             OUTW(_VPE_REG_BASE_ADDR+RCW##_OFS, t##RCW.reg)
#define VPE_OF(RCW)             t##RCW.bit
*/

extern VOID vpe_eng_platform_disable_sram_shutdown(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_enable_sram_shutdown(VPE_ENG_HANDLE *p_eng);

extern VOID vpe_eng_platform_enable_clk(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_disable_clk(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_prepare_clk(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_unprepare_clk(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_enable_gating(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_disable_gating(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_request_irq(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_release_irq(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_create_resource(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_platform_release_resource(VPE_ENG_HANDLE *p_eng);

extern ER vpe_eng_platform_flg_clear(VPE_ENG_HANDLE *p_eng, FLGPTN flg);
extern ER vpe_eng_platform_flg_wait(VPE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern ER vpe_eng_platform_flg_wait_timeout(VPE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick);
extern ER vpe_eng_platform_flg_set(VPE_ENG_HANDLE *p_eng, FLGPTN flg);
extern UINT32 vpe_eng_platform_get_chip_id(VOID);
extern INT32 vpe_eng_platform_set_clk_rate(VPE_ENG_HANDLE *p_eng);
extern void *vpe_eng_malloc(UINT32 size);
extern void vpe_eng_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // _VPE_ENG_INT_PLATFORM_H_

