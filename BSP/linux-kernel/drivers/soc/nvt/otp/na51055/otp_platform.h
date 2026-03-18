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
#include "otp_drv.h"
#include "otp_dbg.h"
typedef void(*POWERON_EFUSE)(void);
typedef void(*POWERDOWN_EFUSE)(void);
extern void __iomem *IOADDR_EFUSE_REG_BASE;
extern void __iomem *IOADDR_CG_REG_BASE;
extern void __iomem *IOADDR_TZPC_REG_BASE;
extern void __iomem *IOADDR_EFUSE_REG_BASE;
extern void __iomem *IOADDR_TZPC_REG_BASE;
extern void __iomem *IOADDR_DDR_ARB_REG_BASE;
extern POWERON_EFUSE 	poweron_efuse_callback;
extern POWERDOWN_EFUSE	powerdown_efuse_callback;
#endif

extern ER otp_platform_sem_wait(void);
extern ER otp_platform_sem_signal(void);
extern unsigned long otp_platform_spin_lock(void);
extern void otp_platform_spin_unlock(unsigned long flags);
extern void otp_platform_delay_ms(UINT32 ms);
extern void otp_platform_register_poweron_efuse_callback(POWERON_EFUSE poweron_efuse);
extern void otp_platform_register_powerdown_efuse_callback(POWERON_EFUSE powerdown_efuse);
//Inhouse test
extern void powerup_efuse(void) ;
extern void powerdown_efuse(void);

#if !(defined __UITRON || defined __ECOS)
#if defined __FREERTOS
extern void otp_platform_create_resource(void);
#else
extern void otp_platform_create_resource(MODULE_INFO *pmodule_info);
extern void otp_platform_release_resource(void);
#endif
#endif
#endif
