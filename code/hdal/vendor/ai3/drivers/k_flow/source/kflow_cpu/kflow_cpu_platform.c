/**
	@brief Source file of vendor net flow sample.

	@file kflow_cpu_platform.c

	@ingroup kflow ai net mem map

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include"kflow_cpu/kflow_cpu_platform.h"

#if defined (__LINUX)
#include <linux/vmalloc.h>
#define mem_alloc	vmalloc
#define mem_free	vfree
#else
#include <malloc.h>
#define mem_alloc	malloc
#define mem_free	free
#endif

#define DEBUG_MEMORY_CPU 0

#if DEBUG_MEMORY_CPU
UINT32 alloc_count_cpu = 0;
#endif

void* nvt_ai_mem_alloc_cpu(unsigned size)
{
#if DEBUG_MEMORY_CPU
    void* addr = NULL;
    addr = mem_alloc(size);
    alloc_count_cpu += 1;
    printk("nvt_ai_mem_alloc_cpu for size = %u, addr = %p, alloc_count_cpu = %u\n",size, addr, alloc_count_cpu);
    return addr;
#else
	return mem_alloc(size);
#endif
}
//EXPORT_SYMBOL(nvt_ai_mem_alloc);

void nvt_ai_mem_free_cpu(void* addr)
{
#if DEBUG_MEMORY_CPU
    alloc_count_cpu -= 1;
    printk("nvt_ai_mem_free_cpu for addr = %p, alloc_count_cpu = %u\n",addr,alloc_count_cpu);
#endif
	mem_free(addr);
}
//EXPORT_SYMBOL(nvt_ai_mem_free);