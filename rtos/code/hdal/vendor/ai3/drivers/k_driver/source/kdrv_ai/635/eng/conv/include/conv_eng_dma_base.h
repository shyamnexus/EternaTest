#ifndef _CONV_ENG_DMA_BASE_H_
#define _CONV_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "conv_eng_handle.h"

extern INT32 conv_eng_set_dma_base_addr(CONV_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx, UINT32 ll_idx);

#ifdef __cplusplus
}
#endif


#endif //

