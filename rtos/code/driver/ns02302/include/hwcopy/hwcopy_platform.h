#ifndef __HWCOPY_PLATFORM_H__
#define __HWCOPY_PLATFORM_H__


#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/type.h"
#include "kwrap/spinlock.h"
#include "kwrap/error_no.h"
#include "kwrap/debug.h"

#include "comm/driver.h"
#include "dma_protected.h"
#include "io_address.h"
#include "pll_protected.h"
#include "plat/interrupt.h"
#include "rcw_macro.h"
#include <string.h>
#include <stdio.h>

#include "hwcopy_reg.h"
#include "hwcopy_compatible.h"
#include "hwcopy_int.h"
#include "../../../hdal/drivers/k_driver/source/include/cache_protected_ns02302.h"

#define FLGDBGUT_LL_CH0         FLGPTN_BIT(0)   
#define FLGDBGUT_LL_CH1         FLGPTN_BIT(1)
#define FLGDBGUT_LL_CH2         FLGPTN_BIT(2)   
#define FLGDBGUT_LL_CH3         FLGPTN_BIT(3)

#define FLGDBGUT_HWDOWN_CH0     FLGPTN_BIT(4)   
#define FLGDBGUT_HWDOWN_CH1     FLGPTN_BIT(5)
#define FLGDBGUT_HWDOWN_CH2     FLGPTN_BIT(6)   
#define FLGDBGUT_HWDOWN_CH3     FLGPTN_BIT(7)


extern void hwcopy_platform_clk_enable(void);
extern void hwcopy_platform_clk_disable(void);
extern void hwcopy_platform_flg_clear(HWCOPY_CHANNEL ch);
extern void hwcopy_platform_flg_set(HWCOPY_CHANNEL ch);
extern void hwcopy_platform_flg_wait(HWCOPY_CHANNEL ch);
extern void hwcopy_ll_flg_set(HWCOPY_CHANNEL ch);
extern void hwcopy_ll_flg_wait(HWCOPY_CHANNEL ch);


extern ER hwcopy_platform_sem_wait(HWCOPY_CHANNEL ch);
extern ER hwcopy_platform_sem_signal(HWCOPY_CHANNEL ch);
extern ULONG hwcopy_platform_spin_lock(void);
extern void hwcopy_platform_spin_unlock(ULONG flags);

extern void hwcopy_platform_int_enable(void);
extern UINT32 hwcopy_platform_dma_is_cacheable(uintptr_t addr);
extern UINT32 hwcopy_platform_dma_flush_dev2mem_width_neq_loff(uintptr_t addr, UINT32 size);
extern UINT32 hwcopy_platform_dma_flush_dev2mem(uintptr_t addr, UINT32 size);
extern UINT32 hwcopy_platform_dma_flush_mem2dev(uintptr_t addr, UINT32 size);

extern UINT64 hwcopy_platform_va2pa(uintptr_t addr);
extern ER hwcopy_platform_oc_sem_wait(HWCOPY_CHANNEL ch);
extern ER hwcopy_platform_oc_sem_signal(HWCOPY_CHANNEL ch);

extern void hwcopy_set_reg(HWCOPY_CHANNEL ch, UINT32 offset, REGVALUE value);
extern REGVALUE hwcopy_get_reg(HWCOPY_CHANNEL ch, UINT32 offset);

extern void copy_isr(void);


extern void hwcopy_platform_set_ist_event(HWCOPY_CHANNEL ch, HWCOPY_EVENT_TYPE event);
extern int  hwcopy_platform_ist(HWCOPY_CHANNEL ch, UINT32 event);

extern ER hwcopy_platform_create_resource(void *pmodule_info);
extern void hwcopy_platform_release_resource(void);


#endif
