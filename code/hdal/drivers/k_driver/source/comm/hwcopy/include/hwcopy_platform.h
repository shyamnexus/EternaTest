#ifndef __HWCOPY_PLATFORM_H__
#define __HWCOPY_PLATFORM_H__

//#include "interrupt.h"

#if (defined __UITRON || defined __ECOS)
#include "hwcopy.h"
#include "top.h"
#include "dma.h"
#include "nvtDrvProtected.h"
#include "Perf.h"
#include "DrvCommon.h"
#include "cache.h"

#else // ---- __LINUX || __FREERTOS ---

//#include "frammap/frammap_if.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/type.h"
#include "hwcopy_compatible.h"
#include "kwrap/spinlock.h"
#include "comm/driver.h"
#include "kwrap/error_no.h"
#include "dma_protected.h"
#include "io_address.h"
#include "hwcopy_dbg.h"
//#include "kwrap/debug.h"
// ---- __LINUX || __FREERTOS ---

#if defined(_BSP_NA51000_)
#include <mach/rcw_macro.h>
#else
#include "rcw_macro.h"

#endif

#if defined __LINUX //Linux
#include "hwcopy_drv.h"
//#include "hwcopy_dbg.h"
#include "kwrap/cpu.h"
#include <linux/string.h>

#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h>
#else
#include <mach/fmem.h>
#endif

#elif defined __FREERTOS //RTOS
#include "pll_protected.h"
#include "plat/interrupt.h"
//#include "sys.h" 530 do't have the file.
#include <string.h>
#include <stdio.h>

#endif
#endif

#define _EMULATION_             (0)	// no emu macro in linux
					// manually defined here

#define FLGPTN_HWCOPY  0x01

#if defined __FREERTOS
#if defined(_PCIE_EMU_ON_)
#define HWCOPY_GETREG(ofs)          INW(APB_LOCAL2PCIE(IOADDR_HWCP_REG_BASE)+(ofs))               ///< get hwcopy reg
#define HWCOPY_SETREG(ofs,value)    OUTW(APB_LOCAL2PCIE(IOADDR_HWCP_REG_BASE)+(ofs), (value))     ///< set hwcopy reg
#else
#define HWCOPY_GETREG(ofs)          INW(IOADDR_HWCP_REG_BASE+(ofs))               ///< get hwcopy reg
#define HWCOPY_SETREG(ofs,value)    OUTW(IOADDR_HWCP_REG_BASE+(ofs), (value))     ///< set hwcopy reg
#endif
#elif defined __LINUX
#define HWCOPY_GETREG(id, ofs)          INW(IOADDR_HWCP_REG_BASE[id]+(ofs))               ///< get hwcopy reg
#define HWCOPY_SETREG(id, ofs,value)    OUTW(IOADDR_HWCP_REG_BASE[id]+(ofs), (value))     ///< set hwcopy reg
#endif

#if (defined __UITRON || defined __ECOS)
#else // __LINUX || __FREERTOS
#if defined __LINUX
//#define CHKPNT    printk("\033[37mCHK: %d, %s\033[0m\r\n",__LINE__,__func__)
extern void __iomem *IOADDR_HWCP_REG_BASE[HWCOPY_ID_MAX];
#elif defined __FREERTOS
#endif
#endif

extern KDRV_CALLBACK_FUNC v_hwcopy_callback[HWCOPY_ID_1+1];

extern void hwcopy_platform_clk_enable(HWCOPY_ID id);
extern void hwcopy_platform_clk_disable(HWCOPY_ID id);
extern void hwcopy_platform_clk_set_freq(HWCOPY_ID id, UINT32 freq);
extern void hwcopy_platform_clk_get_freq(HWCOPY_ID id, UINT32 *p_freq);
extern void hwcopy_platform_flg_clear(HWCOPY_ID id, FLGPTN flg);
extern void hwcopy_platform_flg_set(HWCOPY_ID id, FLGPTN flg);
extern void hwcopy_platform_flg_wait(HWCOPY_ID id, FLGPTN flg);
extern ER hwcopy_platform_sem_wait(HWCOPY_ID id);
extern ER hwcopy_platform_sem_signal(HWCOPY_ID id);
extern ULONG hwcopy_platform_spin_lock(HWCOPY_ID id);
extern void hwcopy_platform_spin_unlock(HWCOPY_ID id, ULONG flags);
extern void hwcopy_platform_sram_enable(HWCOPY_ID id);
extern void hwcopy_platform_int_enable(HWCOPY_ID id);
extern void hwcopy_platform_int_disable(HWCOPY_ID id);
extern UINT32 hwcopy_platform_dma_is_cacheable(uintptr_t addr);
extern UINT32 hwcopy_platform_dma_flush_dev2mem_width_neq_loff(uintptr_t addr, UINT32 size);
extern UINT32 hwcopy_platform_dma_flush_dev2mem(uintptr_t addr, UINT32 size);
extern UINT32 hwcopy_platform_dma_flush_mem2dev(uintptr_t addr, UINT32 size);
extern BOOL hwcopy_platform_is_valid_va(uintptr_t addr);
extern UINT64 hwcopy_platform_va2pa(uintptr_t addr);
extern ER hwcopy_platform_oc_sem_wait(HWCOPY_ID id);
extern ER hwcopy_platform_oc_sem_signal(HWCOPY_ID id);

extern void (*hwcopy_set_reg)(HWCOPY_ID id, UINT32 offset, REGVALUE value);
extern REGVALUE hwcopy_get_reg(HWCOPY_ID id, UINT32 offset);

extern void copy_isr(HWCOPY_ID id);


extern BOOL hwcopy_platform_list_empty(HWCOPY_ID id);
extern ER hwcopy_platform_add_list(HWCOPY_ID id, KDRV_HWCOPY_TRIGGER_PARAM *p_param,
                        KDRV_CALLBACK_FUNC *p_callback);
extern struct _HWCOPY_REQ_LIST_NODE* hwcopy_platform_get_head(HWCOPY_ID id);
extern ER hwcopy_platform_del_list(HWCOPY_ID id);

extern void hwcopy_platform_set_ist_event(HWCOPY_ID id);
extern int  hwcopy_platform_ist(HWCOPY_ID id, UINT32 event);

extern ER hwcopy_platform_create_resource(void *pmodule_info);
extern void hwcopy_platform_release_resource(void);

#endif
