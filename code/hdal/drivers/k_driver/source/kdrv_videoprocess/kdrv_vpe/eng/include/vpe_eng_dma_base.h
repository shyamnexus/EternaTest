#ifndef _VPE_ENG_DMA_BASE_H_
#define _VPE_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "vpe_eng_handle.h"
#include "vpe_eng_ctrl_base.h"


extern VOID vpe_eng_set_dma_link_list_addr_buf_reg(VPE_ENG_HANDLE *p_eng, uintptr_t llc_addr);
extern ER vpe_eng_set_dma_src_addr_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DATA_CH data_ch, uintptr_t src_addr);
extern ER vpe_eng_set_dma_llc_side_addr_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DATA_CH data_ch, uintptr_t llc_side_addr);
extern ER vpe_eng_set_res_x_dma_out_addr_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 seg_id, VPE_ENG_DATA_CH data_ch, uintptr_t addr);
extern VOID vpe_eng_set_dma_dce_2dlut_addr_buf_reg(VPE_ENG_HANDLE *p_eng, uintptr_t dctg_2dlut_addr);
extern ER vpe_eng_set_pixel_mask_addr_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 seg_id, uintptr_t addr);
#ifdef __cplusplus
}
#endif


#endif //

