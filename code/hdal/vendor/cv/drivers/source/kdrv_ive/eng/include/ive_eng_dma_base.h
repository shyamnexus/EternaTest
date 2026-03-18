#ifndef _IVE_ENG_DMA_BASE_H_
#define _IVE_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "ive_eng_handle.h"

extern INT32 ive_eng_set_dma_in_addr_buf_reg(IVE_ENG_HANDLE *p_eng, UINT8 in_idx, uintptr_t in_addr);
extern INT32 ive_eng_set_dma_in_lnoft_buf_reg(IVE_ENG_HANDLE *p_eng, UINT8 in_idx, UINT32 ln_oft);
extern INT32 ive_eng_set_dma_out_addr_buf_reg(IVE_ENG_HANDLE *p_eng, UINT8 out_idx, uintptr_t out_addr);
extern INT32 ive_eng_set_dma_out_lnoft_buf_reg(IVE_ENG_HANDLE *p_eng, UINT8 out_idx, UINT32 ln_oft);
extern INT32 ive_eng_set_dma_in_enable_reg(IVE_ENG_HANDLE *p_eng, UINT8 in_idx, UINT8 enable);
extern INT32 ive_eng_set_dma_out_enable_reg(IVE_ENG_HANDLE *p_eng, UINT8 in_idx, UINT8 enable);
#ifdef __cplusplus
}
#endif


#endif //

