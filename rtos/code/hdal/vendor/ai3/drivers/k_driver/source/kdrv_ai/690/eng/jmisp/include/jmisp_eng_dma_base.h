#ifndef _JMISP_ENG_DMA_BASE_H_
#define _JMISP_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "jmisp_eng_handle.h"


extern INT32 jmisp_eng_set_dma_base_addr(JMISP_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx, UINT32 pl_id);



#ifdef __cplusplus
}
#endif


#endif //

