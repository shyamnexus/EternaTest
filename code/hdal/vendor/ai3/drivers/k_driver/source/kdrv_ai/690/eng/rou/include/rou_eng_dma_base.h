#ifndef _ROU_ENG_DMA_BASE_H_
#define _ROU_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "rou_eng_handle.h"


extern INT32 rou_eng_set_dma_base_addr(ROU_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx);


#ifdef __cplusplus
}
#endif


#endif //

