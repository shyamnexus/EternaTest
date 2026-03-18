#ifndef _MD_ENG_DMA_BASE_H_
#define _MD_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "md_eng_handle.h"
extern INT32 md_eng_set_dma_inaddr_reg(MD_ENG_HANDLE *p_eng,  UINT8 in_idx ,uintptr_t in_addr);
extern INT32 md_eng_set_dma_in_lnoft_reg(MD_ENG_HANDLE *p_eng, UINT8 in_idx, UINT32 ln_oft);
extern INT32 md_eng_set_dma_outaddr_reg(MD_ENG_HANDLE *p_eng,  UINT8 out_idx ,uintptr_t out_addr);
extern INT32 md_eng_set_dma_axi_enable_reg(MD_ENG_HANDLE *p_eng, UINT8 enable);
#ifdef __cplusplus
}
#endif


#endif //

