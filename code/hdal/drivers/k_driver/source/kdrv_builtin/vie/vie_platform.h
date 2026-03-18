#ifndef __VIE_PLATFORM_H_
#define __VIE_PLATFORM_H__

#include <nvt_api_ver.h>

#if defined(__FREERTOS)
#include "string.h"
#include <stdlib.h>
#include "rcw_macro.h"
#include "io_address.h"
#define EXPORT_SYMBOL(a)
//#if defined(_BSP_NA51089_)
#include "nvt-sramctl.h"
//#endif
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#include "dma_protected.h"
#define VIE_REG_ADDR(addr,ofs)        (addr+(ofs))
//#define VIE_SETREG(addr,ofs, value)   OUTW((addr + ofs), (value))
#define VIE_GETREG(addr,ofs)          INW(addr + ofs)
#define MODULE_CLK_NUM          2
#else
#include <linux/slab.h>
#include <plat/nvt-sramctl.h>
#include <linux/clk-provider.h>
#include <linux/soc/nvt/fmem.h> //<mach/fmem.h>
#include "vie_drv.h"
#define VIE_REG_ADDR(addr,ofs)        (addr+(ofs))
//#define VIE_SETREG(addr,ofs, value)   vie_platform_set_reg(addr, ofs, value)
#define VIE_GETREG(addr,ofs)          ioread32((void*)(addr + ofs))
#endif
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/util.h"
#include "kwrap/cpu.h"
#include "viep_lib.h"
#include "kwrap/debug.h"

#define VIE_MAX_CLK_FREQ        400000000

#if (defined(_NVT_EMULATION_) || defined(_NVT_FPGA_) || defined(CONFIG_NVT_FPGA_EMULATION))
#define SIE_DRVWAIT_FLG_TIMEOUT_MS 	1000000000
#else
#define SIE_DRVWAIT_FLG_TIMEOUT_MS 	100
#endif

extern ULONG _VIE_REG_BASE_ADDR_SET[MODULE_CLK_NUM];


#define FLGPTN_VIE_CH0_FRAMEEND     FLGPTN_BIT(0)
#define FLGPTN_VIE_CH1_FRAMEEND     FLGPTN_BIT(1)
#define FLGPTN_VIE_CH2_FRAMEEND     FLGPTN_BIT(2)
#define FLGPTN_VIE_CH3_FRAMEEND     FLGPTN_BIT(3)
#define FLGPTN_VIE_CH0_BP           FLGPTN_BIT(4)
#define FLGPTN_VIE_CH1_BP           FLGPTN_BIT(5)
#define FLGPTN_VIE_CH2_BP           FLGPTN_BIT(6)
#define FLGPTN_VIE_CH3_BP           FLGPTN_BIT(7)
#define FLGPTN_VIE_CH0_DO0_END      FLGPTN_BIT(8)
#define FLGPTN_VIE_CH1_DO0_END      FLGPTN_BIT(9)
#define FLGPTN_VIE_CH2_DO0_END      FLGPTN_BIT(10)
#define FLGPTN_VIE_CH3_DO0_END      FLGPTN_BIT(11)
#define FLGPTN_VIE_CH0_DO1_END      FLGPTN_BIT(12)
#define FLGPTN_VIE_CH1_DO1_END      FLGPTN_BIT(13)
#define FLGPTN_VIE_CH2_DO1_END      FLGPTN_BIT(14)
#define FLGPTN_VIE_CH3_DO1_END      FLGPTN_BIT(15)
#define FLGPTN_VIE_CH0_CRPSTR       FLGPTN_BIT(16)
#define FLGPTN_VIE_CH1_CRPSTR       FLGPTN_BIT(17)
#define FLGPTN_VIE_CH2_CRPSTR       FLGPTN_BIT(18)
#define FLGPTN_VIE_CH3_CRPSTR       FLGPTN_BIT(19)
#define FLGPTN_VIE_CH0_CRPEND       FLGPTN_BIT(20)
#define FLGPTN_VIE_CH1_CRPEND       FLGPTN_BIT(21)
#define FLGPTN_VIE_CH2_CRPEND       FLGPTN_BIT(22)
#define FLGPTN_VIE_CH3_CRPEND       FLGPTN_BIT(23)
#define FLGPTN_VIE_CH0_YCC_OVFL     FLGPTN_BIT(24)
#define FLGPTN_VIE_CH1_YCC_OVFL     FLGPTN_BIT(25)
#define FLGPTN_VIE_CH2_YCC_OVFL     FLGPTN_BIT(26)
#define FLGPTN_VIE_CH3_YCC_OVFL     FLGPTN_BIT(27)
#define FLGPTN_VIE_DBGOUT_END       FLGPTN_BIT(28)
#define FLGPTN_VIE_LINEBUF_OVFL     FLGPTN_BIT(29)
#define FLGPTN_VIE_CCIR656_HEADER_HIT FLGPTN_BIT(30)
#define FLGPTN_VIE_CRPEND_VDLATISR  FLGPTN_BIT(31) // crop end sts after vie_reg_vdlatch_isr_cb


extern void vie_platform_set_clk_rate(VIE_ENGINE_ID id, VIE_OPENOBJ *pObjCB);
extern ER vie_setPxClock(VIE_ENGINE_ID id, VIE_PXCLKSRC PxClkSel);
extern ER vie_sram_switch(void);
extern ER vie1_setPxClock(VIE_PXCLKSRC PxClkSel);
extern ER vie2_setPxClock(VIE_PXCLKSRC PxClkSel);
extern ER vie_setClock(VIE_ENGINE_ID id, VIE_CLKSRC_SEL ClkSrc, UINT32 uiClkRate);
extern ER vie_getClock(VIE_ENGINE_ID id, VIE_CLKSRC_SEL *ClkSrc, UINT32 *uiClkRate);
extern ER vie1_setClock(VIE_CLKSRC_SEL ClkSrc, UINT32 uiClkRate);
extern ER vie2_setClock(VIE_CLKSRC_SEL ClkSrc, UINT32 uiClkRate);

extern void vie_isr(VIE_ENGINE_ID id);
#if defined __FREERTOS
extern void vie_platform_create_resource(void);
#else
extern void vie_platform_create_resource(VIE_MODULE_INFO *pmodule_info);
#endif
extern void vie_platform_release_resource(void);

extern ER vie_platform_flg_clear(VIE_ENGINE_ID id, FLGPTN flg);
extern ER vie_platform_flg_set(VIE_ENGINE_ID id, FLGPTN flg);
extern ER vie_platform_flg_wait(VIE_ENGINE_ID id, PFLGPTN p_flgptn, FLGPTN flg);
extern ER vie_platform_sem_wait(VIE_ENGINE_ID id);
extern ER vie_platform_sem_signal(VIE_ENGINE_ID id);
extern void vie_platform_prepare_clk(VIE_ENGINE_ID id);
extern void vie_platform_unprepare_clk(VIE_ENGINE_ID id);
extern void vie_platform_enable_clk(VIE_ENGINE_ID id);
extern void vie_platform_disable_clk(VIE_ENGINE_ID id);
extern void vie_platform_disable_sram_shutdown(VIE_ENGINE_ID id);
extern void vie_platform_enable_sram_shutdown(VIE_ENGINE_ID id);
extern unsigned long vie_platform_spin_lock(void);
extern void vie_platform_spin_unlock(unsigned long flags);
extern ULONG vie_platform_va2pa(ULONG addr);
extern UINT32 vie_platform_dma_flush_dev2mem(ULONG addr, UINT32 size, UINT32 type);

#ifndef __VIE_PLATFORM_REG_LOG_API__
#define __VIE_PLATFORM_REG_LOG_API__

/*
	log register api
*/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} VIE_REG_INFO;

typedef struct {
	UINT32 cnt;
	VIE_REG_INFO *p_reg;
} VIE_REG_LOG_RESULT;

void vie_platform_set_reg(ULONG addr, UINT32 ofs, UINT32 val);
INT32 vie_platform_set_reg_log_enable(UINT32 id, UINT32 en);
INT32 vie_platform_set_reg_log_disable(UINT32 id, UINT32 b_freebuf);
VIE_REG_LOG_RESULT vie_platform_get_reg_log(UINT32 id);
#endif

extern void (*VIE_SETREG)(ULONG addr, UINT32 offset, REGVALUE value);
extern NVT_API_CHK_DECLARE(vie);


#endif
