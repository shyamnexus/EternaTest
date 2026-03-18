#include "../stbc_platform.h"
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include <linux/soc/nvt/fmem.h>

#include <plat/hardware.h>
void __iomem *STBC_IOADDR_CG_REG_BASE;
void __iomem *STBC_IOADDR_STBC_REG_BASE;
void __iomem *STBC_IOADDR_STBC_CG_REG_BASE;
void __iomem *STBC_IOADDR_CORE_REG_BASE;
void __iomem *STBC_IOADDR_CC_REG_BASE;

static struct semaphore otp_sem;
static spinlock_t v_otp_spin_locks;


ER stbc_platform_sem_wait(void)
{
#if defined __UITRON || defined __ECOS
	return wai_sem(v_sem[0]);
#elif defined(__FREERTOS)
	return SEM_WAIT(*v_sem[0]);
#else
	down(&otp_sem);
	return E_OK;
#endif
}

ER stbc_platform_sem_signal(void)
{
#if defined __UITRON || defined __ECOS
	return sig_sem(v_sem[0]);
#elif defined(__FREERTOS)
	SEM_SIGNAL(*v_sem[0]);
	return E_OK;
#else
	up(&otp_sem);
	return E_OK;
#endif
}

unsigned long stbc_platform_spin_lock(void)
{
#if defined __UITRON || defined __ECOS
	loc_cpu();
#elif defined(__FREERTOS)
	unsigned long flags;
	vk_spin_lock_irqsave(&v_otp_spin_locks, flags);
	return flags;
#else
	unsigned long flags;
	spin_lock_irqsave(&v_otp_spin_locks, flags);
	return flags;
#endif
}

void stbc_platform_spin_unlock(unsigned long flags)
{
#if defined __UITRON || defined __ECOS
	unl_cpu();
#elif defined(__FREERTOS)
	vk_spin_unlock_irqrestore(&v_otp_spin_locks, flags);
#else
	spin_unlock_irqrestore(&v_otp_spin_locks, flags);
#endif
}


void stbc_platform_delay_ms(UINT32 ms)
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
void stbc_platform_create_resource(void)
#else
void stbc_platform_create_resource(MODULE_INFO *pmodule_info)
#endif
{
#if defined __FREERTOS
	SEM_CREATE(SEMID_OTP, 1);
	v_sem[0] = &SEMID_OTP;
#else
	UINT32 reg;
	//STBC_IOADDR_CG_REG_BASE   = pmodule_info->io_addr[IOADDR_CG_REG_BASE_ENUM];
	STBC_IOADDR_STBC_REG_BASE   = pmodule_info->io_addr[IOADDR_STBC_REG_BASE_ENUM];
	//STBC_IOADDR_STBC_CG_REG_BASE= pmodule_info->io_addr[IOADDR_STBC_CG_REG_BASE_ENUM];
	//STBC_IOADDR_CORE_REG_BASE = pmodule_info->io_addr[IOADDR_CORE_REG_BASE_ENUM];
	reg = readl(STBC_IOADDR_STBC_REG_BASE);
	reg &= ~(0x1);
	writel(reg, STBC_IOADDR_STBC_REG_BASE);

	if (!STBC_IOADDR_CG_REG_BASE) {
		STBC_IOADDR_CG_REG_BASE = ioremap(NVT_CG_BASE_PHYS, 0x100);
	}

	if (!STBC_IOADDR_STBC_CG_REG_BASE) {
		STBC_IOADDR_STBC_CG_REG_BASE = ioremap(NVT_STBC_CG_BASE_PHYS, 0x100);
	}

	if (!STBC_IOADDR_CORE_REG_BASE) {
		STBC_IOADDR_CORE_REG_BASE = ioremap(CORE_REG_BASE_PHYS, 0x100);
	}

	if (!STBC_IOADDR_CC_REG_BASE) {
		STBC_IOADDR_CC_REG_BASE = ioremap(NVT_CC_BASE_PHYS, 0x1000);
	}

	sema_init(&otp_sem, 1);
	spin_lock_init(&v_otp_spin_locks);
#endif
}

void stbc_platform_release_resource(void)
{
#if defined __FREERTOS
	SEM_DESTROY(SEMID_OTP);
#endif
}
