#include "nmediaplay_vdodec_platform.h"
#include "kwrap/spinlock.h"
#include <kwrap/task.h>
#if defined(__LINUX)
#include <linux/delay.h>
#if defined (__aarch64__)
#include <linux/soc/nvt/fmem.h> //for fmem_lookup_pa, PAGE_ALIGN
#else
#include <mach/fmem.h>
#endif
#elif defined(__FREERTOS)
#include "dma_protected.h"
#endif

static VK_DEFINE_SPINLOCK(vdec_lock);

void NMP_VdoDec_Lock_cpu(unsigned long *in_flags)
{
	unsigned long flags=0;
	vk_spin_lock_irqsave(&vdec_lock, flags);
	*in_flags = flags;
}

void NMP_VdoDec_Unlock_cpu(unsigned long *in_flags)
{
	vk_spin_unlock_irqrestore(&vdec_lock, *in_flags);
}

void nmp_vdodec_delay_m_sec(UINT32 m_sec)
{
#if defined(__LINUX)
	msleep(m_sec);
#elif defined(__FREERTOS)
	vos_task_delay_ms(m_sec);
#endif
}

uintptr_t nmp_vdodec_va2pa(uintptr_t va)
{
	uintptr_t pa = 0;
#if defined(__LINUX)
	pa = fmem_lookup_pa(va);
#elif defined(__FREERTOS)
	pa = dma_getPhyAddr(va);
#endif
	return pa;
}

