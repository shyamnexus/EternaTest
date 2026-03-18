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
#define _EMULATION_             (0)	// no emu macro in linux
// manually defined here
#define _FPGA_EMULATION_	(0)
#define _EMULATION_ON_CPU2_	(0)
#define DRV_SUPPORT_IST		(1)

#define DDR_ARB_BOTTOMHALF_KTHREAD	(0)
#define DDR_ARB_BOTTOMHALF_TASKLET	(1)
#define DDR_ARB_BOTTOMHALF_SEL		(DDR_ARB_BOTTOMHALF_TASKLET)

#define DMA_PRI_BIT_MASK        (0x03)

#else
#define OTP_EP_0					(0)
#define OTP_EP_CNT					(1)

#include "otp_ep_drv.h"
#include "otp_ep_dbg.h"
extern void __iomem *IOADDR_EFUSE_EP_REG_BASE[OTP_EP_CNT];
#endif
extern ER otp_ep_platform_sem_wait(UINT32 ep_id);
extern ER otp_ep_platform_sem_signal(UINT32 ep_id);
extern unsigned long otp_ep_platform_spin_lock(UINT32 ep_id);
extern void otp_ep_platform_spin_unlock(UINT32 ep_id, unsigned long flags);
extern void otp_ep_platform_delay_ms(UINT32 ms);
#if !(defined __UITRON || defined __ECOS)
#if defined __FREERTOS
extern void otp_ep_platform_create_resource(void);
#else
extern void otp_ep_platform_create_resource(MODULE_INFO *pmodule_info, UINT32 ep_id);
extern void otp_ep_platform_release_resource(void);
#endif
#endif
#endif
