#ifndef _JOBM_ENG_DMA_BASE_H_
#define _JOBM_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "jobm_eng_handle.h"


extern INT32 jobm_eng_set_dma_base_addr(JOBM_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx, UINT32 jl_id);

#ifdef __cplusplus
}
#endif


#endif //

