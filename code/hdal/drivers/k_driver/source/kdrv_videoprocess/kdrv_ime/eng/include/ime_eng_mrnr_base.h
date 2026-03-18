
#ifndef _IME_ENG_MRNR_BASE_H_
#define _IME_ENG_MRNR_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


//#include "kwrap/type.h"
#include "ime_eng_int_comm.h"

extern VOID ime_eng_set_mrnr_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_mrnr_motion_texture_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mot_en, UINT32 set_txt_en);

extern VOID ime_eng_set_mrnr_y_edge_detection_th_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 *set_l1, UINT16 *set_l2);

extern VOID ime_eng_set_mrnr_uv_edge_detection_th_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_u, UINT32 set_v);

extern VOID ime_eng_set_mrnr_y_edge_smooth_th_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *set_l1, UINT8 *set_l2);

extern VOID ime_eng_set_mrnr_uv_edge_smooth_th_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_u, UINT32 set_v);

extern VOID ime_eng_set_mrnr_y_nr_strength_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_str0, UINT32 set_str1);

extern VOID ime_eng_set_mrnr_uv_nr_strength_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_str);

extern VOID ime_eng_set_mrnr_debug_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_sel, UINT32 set_y_th1, UINT32 set_y_th2, UINT32 set_uv_th);

extern VOID ime_eng_set_mrnr_nr_region_strength_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mot_str, UINT32 set_trans_str, UINT32 set_still_str);

extern VOID ime_eng_set_mrnr_texture_strength_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_th1, UINT32 set_th2, UINT32 set_min, UINT32 set_max, UINT32 set_slop);

#ifdef __cplusplus
}
#endif


#endif // _IME_DBCS_REG_
