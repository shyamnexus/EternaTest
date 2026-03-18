#ifndef _TRKE_ENG_DMA_BASE_H_
#define _TRKE_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "trke_eng_handle.h"


//extern INT32 trke_eng_set_dma_in_addr_buf_reg(TRKE_ENG_HANDLE *p_eng, UINT8 in_idx, uintptr_t in_addr);
//extern INT32 trke_eng_set_dma_in_lnoft_buf_reg(TRKE_ENG_HANDLE *p_eng, UINT8 in_idx, UINT32 ln_oft);
//extern INT32 trke_eng_set_dma_out_addr_buf_reg(TRKE_ENG_HANDLE *p_eng, UINT8 out_idx, uintptr_t out_addr);
//extern INT32 trke_eng_set_dma_out_lnoft_buf_reg(TRKE_ENG_HANDLE *p_eng, UINT8 out_idx, UINT32 ln_oft);
extern INT32 trke_eng_set_prv_pyr_dma_in_addr_buf_reg(TRKE_ENG_HANDLE *p_eng, uintptr_t in_addr, UINT32 ln_oft);
extern INT32 trke_eng_set_nxt_pyr_dma_in_addr_buf_reg(TRKE_ENG_HANDLE *p_eng, uintptr_t in_addr, UINT32 ln_oft);
extern INT32 trke_eng_set_prv_pnt_dma_in_addr_buf_reg(TRKE_ENG_HANDLE *p_eng, uintptr_t in_addr);
extern INT32 trke_eng_set_init_flow_dma_in_addr_buf_reg(TRKE_ENG_HANDLE *p_eng, uintptr_t in_addr);
extern INT32 trke_eng_set_nxt_pnt_dma_out_addr_buf_reg(TRKE_ENG_HANDLE *p_eng, uintptr_t out_addr);
extern INT32 trke_eng_set_err_dma_out_addr_buf_reg(TRKE_ENG_HANDLE *p_eng, uintptr_t out_addr);
extern INT32 trke_eng_set_dma_in_enable_reg(TRKE_ENG_HANDLE *p_eng, UINT8 enable);
extern INT32 trke_eng_set_dma_out_enable_reg(TRKE_ENG_HANDLE *p_eng, UINT8 enable);


#ifdef __cplusplus
}
#endif


#endif //

