
#ifndef _IME_ENG_COMPRESSION_BASE_H_
#define _IME_ENG_COMPRESSION_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif



#include "kwrap/type.h"
#include "ime_eng_int_comm.h"
#include "ime_eng_comm.h"

#if 0
extern const UINT32 ime_yuvcomp_dct_dec_qtbl[28][3];
extern const UINT32 ime_yuvcomp_dct_enc_qtbl[28][3];
extern const UINT32 ime_yuvcomp_dct_max[28];

extern UINT32 const ime_yuvcomp_dct_th_start[3];

extern UINT32 yuv_dct_qtab_encp[16][3];
extern UINT32 yuv_dct_qtab_decp[16][3];
extern UINT32 yuv_dct_qtab_dc[16];
#endif

extern UINT8 ime_yuvcomp_q_tbl_idx[16];
extern UINT8 yuv_dct_qtab_cth[8];

extern VOID ime_eng_set_comps_path_encoder_shift_mode_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_en);

extern VOID ime_eng_set_comps_tmnr_out_ref_encoder_shift_mode_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_comps_tmnr_in_ref_decoder_shift_mode_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_comps_tmnr_in_ref_decoder_dithering_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_comps_tmnr_in_ref_decoder_dithering_initial_seed_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_seed0, UINT32 set_seed1);

extern VOID ime_eng_set_comps_dct_index_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_dct_idx);

extern VOID ime_eng_set_comps_dct_complexity_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_dct_cth);


extern VOID ime_eng_set_comps_in_path_decoder_shift_mode_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_comps_in_path_decoder_dithering_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

#if 0

extern VOID ime_eng_set_comps_encoder_parameters_buf_reg(IME_ENG_HANDLE *p_eng, UINT32(*p_enc_tab)[3]);

extern VOID ime_eng_set_comps_decoder_parameters_buf_reg(IME_ENG_HANDLE *p_eng, UINT32(*p_dec_tab)[3]);

extern VOID ime_eng_set_comps_dc_max_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 *p_dc_max);
#endif


#ifdef __cplusplus
}
#endif

#endif

