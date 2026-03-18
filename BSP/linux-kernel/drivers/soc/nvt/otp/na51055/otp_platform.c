


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
static vk_spinlock_t    v_otp_spin_locks;
#else
#include <plat/hardware.h>
#include "otp_platform.h"
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include <linux/soc/nvt/fmem.h>
//Toggle GPIO sample
#include <linux/gpio.h>
#include <plat/nvt-gpio.h>
#include <linux/timekeeping.h>

void __iomem *IOADDR_EFUSE_REG_BASE;
void __iomem *IOADDR_TZPC_REG_BASE;
void __iomem *IOADDR_DDR_ARB_REG_BASE;

static u64 curr_t, curr_t2;
POWERON_EFUSE   poweron_efuse_callback = NULL;
POWERDOWN_EFUSE powerdown_efuse_callback = NULL;

DEFINE_SEMAPHORE(otp_sem);
//static struct semaphore otp_sem;
DEFINE_SPINLOCK(v_otp_spin_locks);
#endif


ER otp_platform_sem_wait(void)
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

ER otp_platform_sem_signal(void)
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

unsigned long otp_platform_spin_lock(void)
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

void otp_platform_spin_unlock(unsigned long flags)
{
#if defined __UITRON || defined __ECOS
	unl_cpu();
#elif defined(__FREERTOS)
	vk_spin_unlock_irqrestore(&v_otp_spin_locks, flags);
#else
	spin_unlock_irqrestore(&v_otp_spin_locks, flags);
#endif
}


void otp_platform_delay_ms(UINT32 ms)
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
void otp_platform_create_resource(void)
#else
void otp_platform_create_resource(MODULE_INFO *pmodule_info)
#endif
{
#if defined __FREERTOS
	SEM_CREATE(SEMID_OTP, 1);
	v_sem[0] = &SEMID_OTP;
#else
	IOADDR_EFUSE_REG_BASE = pmodule_info->io_addr[0];
	if(!IOADDR_TZPC_REG_BASE) {
		IOADDR_TZPC_REG_BASE = ioremap(NVT_TZPC_PHY_BASE_PHYS, 0x100);
	}

	if(!IOADDR_DDR_ARB_REG_BASE) {
		IOADDR_DDR_ARB_REG_BASE = ioremap(NVT_DDR_ARB_PHY_BASE_PHYS, 0x100);
	}
	//sema_init(&otp_sem, 1);
	//spin_lock_init(&v_otp_spin_locks);
	//Need porting depend on customer's GPIO number
	gpio_direction_output(P_GPIO(12), 0);
	otp_platform_register_poweron_efuse_callback(powerup_efuse);
	otp_platform_register_powerdown_efuse_callback(powerdown_efuse);
#endif
}

void otp_platform_release_resource(void)
{
#if defined __FREERTOS
	SEM_DESTROY(SEMID_OTP);
#endif
}

#if defined __KERNEL__
void otp_platform_earily_create_resource(void)
{
	if(!IOADDR_EFUSE_REG_BASE) {
		IOADDR_EFUSE_REG_BASE = ioremap(NVT_EFUSE_BASE_PHYS, 0x100);
	}
}

void otp_platform_earily_release_resource(void)
{
	iounmap((volatile void __iomem *)IOADDR_EFUSE_REG_BASE);
}

EXPORT_SYMBOL(otp_platform_earily_create_resource);
EXPORT_SYMBOL(otp_platform_earily_release_resource);
#endif
void otp_platform_register_poweron_efuse_callback(POWERON_EFUSE poweron_efuse)
{
	if (poweron_efuse) {
		poweron_efuse_callback = poweron_efuse;
	}
}

void otp_platform_register_powerdown_efuse_callback(POWERON_EFUSE powerdown_efuse)
{
	if (powerdown_efuse) {
		powerdown_efuse_callback = powerdown_efuse;
	}
}

//Need call otp_platform_register_poweron_efuse_callback(powerup_efuse) to register to OTP driver
void powerup_efuse(void)
{
	pr_info("powerup_efuse\n");
	//Sample code use P_GPIO24 as PAD pull up pin
	gpio_direction_output(P_GPIO(24), 1);
	//Remember need delay for PAD pull up
	mdelay(800);
	curr_t = ktime_get_real_ns();
}
//Need call otp_platform_register_powerdown_efuse_callback(powerdown_efuse) to register to OTP driver
void powerdown_efuse(void)
{
	curr_t2 = ktime_get_real_ns();
	pr_info("powerdown_efuse %lld ns\n", (curr_t2 - curr_t));
	//Sample code use P_GPIO24 as PAD pull up pin
	gpio_direction_output(P_GPIO(24), 0);
	//Remember need delay for PAD pull down
	mdelay(800);
}

