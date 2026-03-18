/**
	@brief Source file of vendor net flow sample.

	@file kflow_dsp_platform.c

	@ingroup kflow ai net mem map

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include"kflow_dsp/kflow_dsp_platform.h"

#if defined (__LINUX)
#include <linux/vmalloc.h>
#define mem_alloc	vmalloc
#define mem_free	vfree
#else
#include <malloc.h>
#define mem_alloc	malloc
#define mem_free	free
#endif

#define DEBUG_MEMORY_DSP 0

#if DEBUG_MEMORY_DSP
UINT32 alloc_count_dsp = 0;
#endif

void* nvt_ai_mem_alloc_dsp(unsigned size)
{
#if DEBUG_MEMORY_DSP
    void* addr = NULL;
    addr = mem_alloc(size);
    alloc_count_dsp += 1;
    printk("nvt_ai_mem_alloc_dsp for size = %u, addr = %p, alloc_count_dsp = %u\n",size, addr, alloc_count_dsp);
    return addr;
#else
	return mem_alloc(size);
#endif
}
//EXPORT_SYMBOL(nvt_ai_mem_alloc_dsp);

void nvt_ai_mem_free_dsp(void* addr)
{
#if DEBUG_MEMORY_DSP
    alloc_count_dsp -= 1;
    printk("nvt_ai_mem_free_dsp for addr = %p, alloc_count_dsp = %u\n",addr,alloc_count_dsp);
#endif
	mem_free(addr);
}
//EXPORT_SYMBOL(nvt_ai_mem_free_dsp);
