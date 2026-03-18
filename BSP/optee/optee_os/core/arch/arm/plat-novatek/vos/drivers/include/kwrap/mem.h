#ifndef _VOS_MEM_H_
#define _VOS_MEM_H_
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------*/
/* Including Files                                                             */
/*-----------------------------------------------------------------------------*/
#include <kwrap/nvt_type.h>

//for backward compatible, do NOT use these macros anymore --- begin
#define vos_mem_alloc_from_cma(a) vos_mem_alloc_contiguous(a)
#define vos_mem_release_from_cma(a) vos_mem_release_contiguous(a)
#define vos_mem_init_cma_info(a,b,c) vos_mem_init_info(a,b,c,NULL)
#define vos_mem_cma_info_t vos_mem_info_t
#define VOS_MEM_CMA_TYPE VOS_MEM_TYPE
#define VOS_MEM_CMA_TYPE_CACHE VOS_MEM_TYPE_CACHE
#define VOS_MEM_CMA_TYPE_NONCACHE VOS_MEM_TYPE_NONCACHE
#define VOS_MEM_CMA_TYPE_BUFFER VOS_MEM_TYPE_BUFFER
#define VOS_MEM_CMA_HDL VOS_MEM_HDL
//for backward compatible, do NOT use these macros anymore --- end

typedef void* VOS_MEM_HDL;

typedef enum _VOS_MEM_TYPE {
	VOS_MEM_TYPE_CACHE = 1,         ///< linux: dma_alloc_from_contiguous / freertos: malloc
	VOS_MEM_TYPE_NONCACHE,          ///< linux: dma_alloc_coherent        / freertos: Not supported
	VOS_MEM_TYPE_BUFFER,            ///< linux: dma_alloc_writecombine    / freertos: Not supported
}VOS_MEM_TYPE;

struct vos_mem_info_t {
    VOS_MEM_CMA_TYPE        type;       ///< allocate type, refer to #VOS_FMEM_ALLOC_TYPE
    ULONG                   reserved1;
    ULONG                   paddr;      ///< physical address
    ULONG                   reserved2;
    ULONG                   reserved3;
    ULONG                   vaddr;      ///< virtual address
    ULONG                   size;       ///< size
};

void rtos_mem_init(void *param);
void rtos_mem_exit(void);

int vos_mem_init_info(struct vos_mem_info_t *p_info, VOS_MEM_TYPE type, UINT32 size, void *dev);
	//Note:
	//About parameter "void *dev", the real type is "struct device *dev", which is used by fmem statistics
	//If NULL is passed, vos will use its own struct device for fmem
	//A Linux driver should pass its own struct device to be correctly gather statistics by fmem.

VOS_MEM_HDL vos_mem_alloc_cma(struct vos_mem_info_t *p_info, unsigned int index);
int vos_mem_release_cma(VOS_MEM_HDL cma_hdl, unsigned int index);

VOS_MEM_HDL vos_mem_alloc_contiguous(struct vos_mem_info_t *p_info);
int vos_mem_release_contiguous(VOS_MEM_HDL p_hdl);

#ifdef __cplusplus
}
#endif

#endif /* _VOS_MEM_H_ */

