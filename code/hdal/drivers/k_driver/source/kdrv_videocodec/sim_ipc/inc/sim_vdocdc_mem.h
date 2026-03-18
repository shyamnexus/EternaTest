#ifndef _SIM_VDOCDC_MEM_H_
#define _SIM_VDOCDC_MEM_H_

#if defined(__LINUX)
#include <linux/soc/nvt/nvtmem.h>
#endif

#include "kwrap/type.h"

#ifdef __FREERTOS
struct nvt_fmem_mem_info_t {
	uintptr_t vaddr;
	UINT32 size;
	uintptr_t ddr_addr;
	UINT32 ddr_size;
};
#endif

typedef struct _H26XDecSIMRecBufAddr_{
	uintptr_t uiYAddr;
	uintptr_t uiCAddr;
	UINT8 bIsUsed;
} H26XDecSIMRecBufAddr;

void *vdocdc_get_mem(struct nvt_fmem_mem_info_t *pinfo, unsigned int size);
void vdocdc_free_mem(void *handle);

#endif // _SIM_VDOCDC_MEM_H_
