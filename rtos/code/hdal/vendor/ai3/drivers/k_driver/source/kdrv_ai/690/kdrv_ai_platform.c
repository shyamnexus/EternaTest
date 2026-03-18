/**
	@brief Source file of vendor net flow sample.

	@file net_flow_platform.c

	@ingroup net_flow_platform

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "kwrap/cpu.h"
#include "kdrv_ai_platform.h"
#include "../../../k_driver/include/kdrv_ai_version.h"


#if defined(__LINUX)
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/slab.h>
#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h> //for fmem_lookup_pa, PAGE_ALIGN
#else
#include <mach/fmem.h> //for fmem_lookup_pa, PAGE_ALIGN
#endif
#include <asm/io.h>  /* for ioremap and iounmap */
#include <plat/top.h>
//#include "kflow_common/nvtmpp.h"
#elif defined(__FREERTOS)
#include "rtos_na51090/nvt-sramctl.h"
#include "rtos_na51090/top.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#else
#endif


uintptr_t kdrv_ai_pa2va_remap(uintptr_t pa, UINT32 sz)
{
	uintptr_t va = 0;
#if defined(__LINUX)
	if (sz == 0) {
		return va;
	}
	if (pfn_valid(__phys_to_pfn(pa))) {
		va = (uintptr_t)__va(pa);
	} else {
		va = (uintptr_t)ioremap(pa, PAGE_ALIGN(sz));
	}
#else
	va = pa;
#endif
	if (va > 0) {
		vos_cpu_dcache_sync(va, sz, VOS_DMA_TO_DEVICE);
	}
	
	return va;
}

uintptr_t kdrv_ai_pa2va_remap_wo_sync(uintptr_t pa, UINT32 sz)
{
	uintptr_t va = 0;
#if defined(__LINUX)
	if (sz == 0) {
		return va;
	}
	if (pfn_valid(__phys_to_pfn(pa))) {
		va = (uintptr_t)__va(pa);
	} else {
		va = (uintptr_t)ioremap(pa, PAGE_ALIGN(sz));
	}
#else
	va = pa;
#endif

	return va;
}

VOID kdrv_ai_pa2va_unmap(uintptr_t va, uintptr_t pa)
{
	if (va == 0) {
		return;
	}
#if defined(__LINUX)
	if (!pfn_valid(__phys_to_pfn(pa))) {
		iounmap((VOID *)va);
	}
#endif
}

void *kdrv_ai_alloc_mem(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void kdrv_ai_free_mem(void *p_buf)
{
#if defined(__LINUX)
	kfree(p_buf);
#elif defined(__FREERTOS)
	free(p_buf);
#endif
}

#if defined(__LINUX)
MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("AI kdrv");
MODULE_LICENSE("GPL");
#endif