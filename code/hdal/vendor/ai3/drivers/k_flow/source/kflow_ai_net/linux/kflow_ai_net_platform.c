/**
	@brief Source file of vendor net flow sample.

	@file kflow_ai_net_platform.c

	@ingroup kflow ai net mem map

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kwrap/cpu.h"

#include "kflow_ai_net/kflow_ai_net_platform.h"

#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h> //for fmem_lookup_pa, PAGE_ALIGN
#else
#include <mach/fmem.h> //for fmem_lookup_pa, PAGE_ALIGN
#endif
#include <asm/io.h>  /* for ioremap and iounmap, pfn_valid, __phys_to_pfn */

#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
//#include "kflow_common/nvtmpp.h"
#endif

int nvt_ai_user_va_cacheable(uintptr_t user_va)
{
    struct vm_area_struct *vma = NULL;
    pgprot_t nc_prot = {0}, wb_prot = {0};

    if (current && current->mm) {
        vma = find_vma(current->mm, user_va);
        if (vma) {
            //DBG_DUMP("user_va = 0x%x, vm_start = 0x%x, vm_end = 0x%x, vm_page_prot = 0x%x\r\n",
            //    (int)user_va, (int)vma->vm_start, (int)vma->vm_end, vma->vm_page_prot);
            nc_prot = pgprot_noncached(vma->vm_page_prot);
            wb_prot = pgprot_writecombine(vma->vm_page_prot);
            if (memcmp(&nc_prot, &vma->vm_page_prot, sizeof(pgprot_t)) == 0) {
                // DBG_IND("This is non-cache\r\n");
                return EFAULT;
            } else if (memcmp(&wb_prot, &vma->vm_page_prot, sizeof(pgprot_t)) == 0) {
                // DBG_IND("This is non-cache\r\n");
                return EFAULT;
            } else {
                // DBG_IND("This is cache\r\n");
                return 0;
            }
        } else {
            // DBG_ERR("find vma_fail for user_va 0x%lx\r\n", user_va);
            return ENXIO;
        }
    } else {
        // DBG_ERR("This is not user task\r\n");
        return ESPIPE;
    }
}

uintptr_t nvt_ai_va2pa(uintptr_t addr)
{
	if (addr == 0) {
		return addr;
	} else {
		return fmem_lookup_pa(addr);
	}
}

uintptr_t nvt_ai_pa2va_remap(uintptr_t pa, UINT32 sz)
{
	uintptr_t va = 0;
	if (sz == 0) {
		return va;
	}
	if (pfn_valid(__phys_to_pfn(pa))) {
        va = (uintptr_t)__va(pa);
    } else {
#if defined(__aarch64__)
		va = (uintptr_t)ioremap_cache(pa, PAGE_ALIGN(sz));
#else
		va = (uintptr_t)ioremap_cached(pa, PAGE_ALIGN(sz));
#endif
		if (va == 0) {
			return va;
		}
	}
	//fmem_dcache_sync((UINT32 *)va, sz, DMA_BIDIRECTIONAL);
	vos_cpu_dcache_sync(va, sz, VOS_DMA_TO_DEVICE); ///< cache clean - output to engine's input
	return va;
}
EXPORT_SYMBOL(nvt_ai_pa2va_remap);
uintptr_t nvt_ai_pa2va_remap_wo_sync(uintptr_t pa, UINT32 sz)
{
	uintptr_t va = 0;
#if defined(__LINUX)
	if (sz == 0) {
		return va;
	}
	if (pfn_valid(__phys_to_pfn(pa))) {
		va = (uintptr_t)__va(pa);
	} else {
		va = (uintptr_t)ioremap_wc(pa, PAGE_ALIGN(sz));
		if (va == 0) {
			printk("ioremap fail. pa(%#lx)\n", pa);
		}
	}
#else
	va = pa;
#endif

	return va;
}
EXPORT_SYMBOL(nvt_ai_pa2va_remap_wo_sync);

VOID nvt_ai_pa2va_unmap(uintptr_t va, uintptr_t pa)
{
	if (va == 0) {
		return;
	}
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_))
	if (!pfn_valid(__phys_to_pfn(pa))) {
		iounmap((VOID *)va);
	}
#else
	if (!pfn_valid(__phys_to_pfn(pa))) {
		iounmap((VOID *)va);
	}
#endif
}
EXPORT_SYMBOL(nvt_ai_pa2va_unmap);
#if defined (__LINUX)
#include <linux/vmalloc.h>
#define mem_alloc	vmalloc
#define mem_free	vfree
#else
#include <malloc.h>
#define mem_alloc	malloc
#define mem_free	free
#endif

#define DEBUG_MEMORY 0

#if DEBUG_MEMORY
UINT32 alloc_count = 0;
#endif

VOID* nvt_ai_mem_alloc(UINT32 size)
{
#if DEBUG_MEMORY
    VOID* addr = NULL;
    addr = mem_alloc(size);
    alloc_count += 1;
    printk("nvt_ai_mem_alloc for size = %u, addr = %p, alloc_count = %u\n",size, addr, alloc_count);
    return addr;
#else
	return mem_alloc(size);
#endif
}
EXPORT_SYMBOL(nvt_ai_mem_alloc);

VOID nvt_ai_mem_free(VOID* addr)
{
#if DEBUG_MEMORY
    alloc_count -= 1;
    printk("nvt_ai_mem_free for addr = %p, alloc_count = %u\n",addr,alloc_count);
#endif
	mem_free(addr);
}
EXPORT_SYMBOL(nvt_ai_mem_free);

int nvt_ai_get_config_from_dtsi(char* path, char* name, UINT32* num, UINT32 size)
{
	struct device_node *dt_node;

	dt_node = of_find_node_by_path(path);
	if (dt_node) {
		if (of_property_read_u32_array(dt_node, name, (UINT32 *)num, size)) {
			return EFAULT; 
		}
		return 0 ; 
	}	
	return EFAULT; 
	
}
EXPORT_SYMBOL(nvt_ai_get_config_from_dtsi);

int nvt_ai_has_config_from_dtsi(char* path)
{
	struct device_node *dt_node;

	dt_node = of_find_node_by_path(path);
	if (dt_node) {
		return 0 ; 
	}	
	return EFAULT; 
}