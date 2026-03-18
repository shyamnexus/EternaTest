#ifndef __MODULE_PLATFORM_H_
#define __MODULE_PLATFORM_H__

#if (defined __UITRON || defined __ECOS)
#include <mach/fmem.h>
#elif defined(__FREERTOS)
#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include <kwrap/spinlock.h>
#include <kwrap/nvt_type.h>
#include <kwrap/task.h>
#include <string.h>
#include "rcw_macro.h"
#include "io_address.h"
#include "dma_protected.h"
#include "cache_protected.h"
#include "interrupt.h"
//#include "ddr_arb_dbg.h"
#define _EMULATION_             (0) // no emu macro in linux
// manually defined here
#define _FPGA_EMULATION_    (0)
#define _EMULATION_ON_CPU2_ (0)
#define DRV_SUPPORT_IST     (1)

#define DDR_ARB_BOTTOMHALF_KTHREAD  (0)
#define DDR_ARB_BOTTOMHALF_TASKLET  (1)
#define DDR_ARB_BOTTOMHALF_SEL      (DDR_ARB_BOTTOMHALF_TASKLET)

#define DMA_PRI_BIT_MASK        (0x03)

#else
#include "stbc_drv.h"
#include "stbc_dbg.h"
extern void __iomem *STBC_IOADDR_CG_REG_BASE;
extern void __iomem *STBC_IOADDR_STBC_REG_BASE;
extern void __iomem *STBC_IOADDR_STBC_CG_REG_BASE;
extern void __iomem *STBC_IOADDR_CORE_REG_BASE;
extern void __iomem *STBC_IOADDR_CC_REG_BASE;
#endif
extern ER stbc_platform_sem_wait(void);
extern ER stbc_platform_sem_signal(void);
extern void stbc_platform_clk_standby(void);
extern unsigned long stbc_platform_spin_lock(void);
extern void stbc_platform_spin_unlock(unsigned long flags);

extern void stbc_platform_delay_ms(UINT32 ms);
#if !(defined __UITRON || defined __ECOS)
#if defined __FREERTOS
extern void stbc_platform_create_resource(void);
#else
extern void stbc_platform_create_resource(MODULE_INFO *pmodule_info);
extern void stbc_platform_release_resource(void);
#endif
#endif
#endif
