
#ifndef _IME_ENG_FISHEYE_MASK_BASE_H_
#define _IME_ENG_FISHEYE_MASK_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "ime_eng_int_comm.h"

//-------------------------------------------------------------------------------
// Fisheye mask
extern VOID ime_eng_set_fisheye_mask_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ime_eng_set_fisheye_mask_center_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 cent_x, UINT32 cent_y);
extern VOID ime_eng_set_fisheye_mask_gain_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 radius, UINT32 decrase_range, UINT32 divide_mul, UINT32 divide_shift);

#ifdef __cplusplus
}
#endif


#endif
