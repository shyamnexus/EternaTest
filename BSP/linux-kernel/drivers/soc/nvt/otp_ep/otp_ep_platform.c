


#if defined __UITRON || defined __ECOS

#include "ddr_arb.h"
#include "interrupt.h"
#include "top.h"
#include "dma.h"
static const DRV_SEM v_sem[] = {SEMID_ARB};

#elif defined(__FREERTOS)
#include <kwrap/error_no.h>
#include <kwrap/semaphore.h>
#include <kwrap/nvt_type.h>
#include <kwrap/flag.h>
#include <kwrap/spinlock.h>

#include <kwrap/task.h>
#include <string.h>
#include "rcw_macro.h"
#include "io_address.h"
#include "dma_protected.h"
#include "cache_protected.h"
#include "interrupt.h"


static SEM_HANDLE *v_sem[1];
static SEM_HANDLE SEMID_OTP;
static vk_spinlock_t	v_otp_spin_locks;
#else
#include "otp_ep_platform.h"
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include <linux/soc/nvt/fmem.h>

void __iomem *IOADDR_EFUSE_EP_REG_BASE[OTP_EP_CNT];
static struct semaphore otp_sem[OTP_EP_CNT];
static spinlock_t v_otp_spin_locks[OTP_EP_CNT];
#endif


ER otp_ep_platform_sem_wait(UINT32 ep_id)
{
#if defined __UITRON || defined __ECOS
	return wai_sem(v_sem[0]);
#elif defined(__FREERTOS)
	return SEM_WAIT(*v_sem[0]);
#else
	down(&otp_sem[ep_id]);
	return E_OK;
#endif
}

ER otp_ep_platform_sem_signal(UINT32 ep_id)
{
#if defined __UITRON || defined __ECOS
	return sig_sem(v_sem[0]);
#elif defined(__FREERTOS)
	SEM_SIGNAL(*v_sem[0]);
	return E_OK;
#else
	up(&otp_sem[ep_id]);
	return E_OK;
#endif
}

unsigned long otp_ep_platform_spin_lock(UINT32 ep_id)
{
#if defined __UITRON || defined __ECOS
	loc_cpu();
#elif defined(__FREERTOS)
	unsigned long flags;
	vk_spin_lock_irqsave(&v_otp_spin_locks, flags);
	return flags;
#else
	unsigned long flags;
	spin_lock_irqsave(&v_otp_spin_locks[ep_id], flags);
	return flags;
#endif
}

void otp_ep_platform_spin_unlock(UINT32 ep_id, unsigned long flags)
{
#if defined __UITRON || defined __ECOS
	unl_cpu();
#elif defined(__FREERTOS)
	vk_spin_unlock_irqrestore(&v_otp_spin_locks, flags);
#else
	spin_unlock_irqrestore(&v_otp_spin_locks[ep_id], flags);
#endif
}




void otp_ep_platform_delay_ms(UINT32 ms)
{
#if defined __UITRON || defined __ECOS
	Delay_DelayMs(ms);
#elif defined __FREERTOS
	vos_task_delay_ms(ms);
#else
	msleep(ms);
#endif
}
#if defined __FREERTOS
void otp_ep_platform_create_resource(void)
#else
void otp_ep_platform_create_resource(MODULE_INFO *pmodule_info, UINT32 ep_id)
#endif
{
#if defined __FREERTOS
	SEM_CREATE(SEMID_OTP, 1);
	v_sem[0] = &SEMID_OTP;
#else
	IOADDR_EFUSE_EP_REG_BASE[ep_id] = pmodule_info->io_addr[ep_id];
	sema_init(&otp_sem[ep_id], 1);
	spin_lock_init(&v_otp_spin_locks[ep_id]);
#endif
}

void otp_ep_platform_release_resource(void)
{
#if defined __FREERTOS
	SEM_DESTROY(SEMID_OTP);
#endif
}
