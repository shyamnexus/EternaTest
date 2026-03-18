#ifndef _CAL_ENG_DMA_BASE_H_
#define _CAL_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "cal_eng_handle.h"


extern INT32 cal_eng_set_dma_base_addr(CAL_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx, UINT32 ll_idx);


#ifdef __cplusplus
}
#endif


#endif //

