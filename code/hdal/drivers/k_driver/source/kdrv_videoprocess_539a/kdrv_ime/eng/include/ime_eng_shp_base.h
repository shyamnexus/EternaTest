

#ifndef _IME_ENG_SHARPEN_BASE_H_
#define _IME_ENG_SHARPEN_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif



#include "kwrap/type.h"
#include "ime_eng_int_comm.h"

extern VOID ime_eng_set_sharpen_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_en);
extern VOID ime_eng_set_sharpen_motion_bit_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_en);

extern VOID ime_eng_set_sharpen_debug_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_en);
extern VOID ime_eng_set_sharpen_jnd_filter_size_sel_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_sel);
extern VOID ime_eng_set_sharpen_edge_weight_cal_source_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_sel);

extern VOID ime_eng_set_sharpen_edge_weight_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_th, UINT8 set_gain);
extern VOID ime_eng_set_sharpen_noise_level_curve_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_level, UINT8 *p_set_noise_curve);
extern VOID ime_eng_set_sharpen_region_classification_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 set_flat_th, UINT16 set_edge_th, UINT8 set_wet);

extern VOID ime_eng_set_sharpen_edge_weighting_gain_curve_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 *p_set_wet);

extern VOID ime_eng_set_sharpen_strength_for_varied_regions_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 set_slope, UINT8 set_flat_str, UINT8 set_edge_str);
extern VOID ime_eng_set_sharpen_motion_based_strength_for_varied_regions_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_mstr, UINT8 set_sstr, UINT8 set_tstr);

extern VOID ime_eng_set_sharpen_edge_enh_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_bgma, UINT8 set_cor_th, UINT8 set_estr);
extern VOID ime_eng_set_sharpen_halo_clip_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_bhc, UINT8 set_dhc);


#if (IME_GET_API_EN == 1)



#endif

#ifdef __cplusplus
}
#endif


#endif

