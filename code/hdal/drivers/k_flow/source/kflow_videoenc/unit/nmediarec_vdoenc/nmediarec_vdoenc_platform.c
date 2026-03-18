#include "nmediarec_vdoenc_platform.h"
#include "kwrap/spinlock.h"
#include <kwrap/task.h>
#if defined(__LINUX)
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/io.h>
#if defined (__aarch64__)
#include <linux/soc/nvt/fmem.h> //for fmem_lookup_pa, PAGE_ALIGN
#else
#include <mach/fmem.h>
#endif
#elif defined(__FREERTOS)
#include <malloc.h>
#include "dma_protected.h"
#endif

static VK_DEFINE_SPINLOCK(my_lock);
static VK_DEFINE_SPINLOCK(my_lock2);

void NMR_VdoEnc_Lock_cpu(unsigned long *in_flags)
{
	unsigned long flags=0;
	vk_spin_lock_irqsave(&my_lock, flags);
	*in_flags = flags;
}

void NMR_VdoEnc_Unlock_cpu(unsigned long *in_flags)
{
	vk_spin_unlock_irqrestore(&my_lock, *in_flags);
}

void NMR_VdoEnc_Lock_cpu2(unsigned long *in_flags)
{
	unsigned long flags=0;
	vk_spin_lock_irqsave(&my_lock2, flags);
	*in_flags = flags;
}

void NMR_VdoEnc_Unlock_cpu2(unsigned long *in_flags)
{
	vk_spin_unlock_irqrestore(&my_lock2, *in_flags);
}

void* nmr_vdoenc_alloc(int size)
{
#if defined(__LINUX)
	return vmalloc(size);
#elif defined(__FREERTOS)
	return malloc(size);
#endif
}

void nmr_vdoenc_free(void *buf)
{
#if defined(__LINUX)
	vfree(buf);
#elif defined(__FREERTOS)
	free(buf);
#endif
}

void nmr_vdoenc_delay_m_sec(UINT32 m_sec)
{
#if defined(__LINUX)
	msleep(m_sec);
#elif defined(__FREERTOS)
	vos_task_delay_ms(m_sec);
#endif
}

uintptr_t nmr_vdoenc_va2pa(uintptr_t va)
{
	uintptr_t pa = 0;
#if defined(__LINUX)
	pa = fmem_lookup_pa(va);
#elif defined(__FREERTOS)
	pa = dma_getPhyAddr(va);
#endif
	return pa;
}

void *nmr_vdoenc_ioremap_nocache_wrap(phys_addr_t offset, size_t size)
{
//#if defined(__LINUX)
//	return ioremap_nocache(offset, size);
//#elif defined(__FREERTOS)
//	return (void *)offset;
//#endif
	return (void *)offset;
}

void nmr_vdoenc_iounmap_wrap(void *addr)
{
//#if defined(__LINUX)
//	iounmap(addr);
//#endif
	return;
}