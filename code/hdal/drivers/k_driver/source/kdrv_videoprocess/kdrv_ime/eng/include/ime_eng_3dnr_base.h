
#ifndef _IME_ENG_3DNR_BASE_H_
#define _IME_ENG_3DNR_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "ime_eng_int_comm.h"


/**
    IME enum - system flag clear selection
*/
typedef enum _IME_TMNR_ENC_FMT_SEL_ {
	IME_TMNR_ENC_FMT_PGSV  = 0,    ///< progressive mode
	IME_TMNR_ENC_FMT_COMP_MB    = 1,    ///< YCC maco-block mode
	IME_TMNR_ENC_FMT_MB    = 2,    ///< maco-block mode
	ENUM_DUMMY4WORD(IME_TMNR_ENC_FMT_SEL)
} IME_TMNR_ENC_FMT_SEL;

/**
    IME enum - system flag clear selection
*/
typedef enum _IME_TMNR_REF_OUT_LOCATION_SEL_ {
	IME_TMNR_REF_OUT_LOCATION_POST_3DNR  = 0,    ///< POST_3DNR
	IME_TMNR_REF_OUT_LOCATION_POST_LCA   = 1,    ///< POST_3DNR
	ENUM_DUMMY4WORD(IME_TMNR_REF_OUT_LOCATION_SEL)
} IME_TMNR_REF_OUT_LOCATION_SEL;



extern VOID ime_eng_set_tmnr_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_out_ref_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_out_ref_encoder_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_in_ref_decoder_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_in_ref_flip_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_in_ref_decoder_fmt_buf_reg(IME_ENG_HANDLE *p_eng, IME_TMNR_ENC_FMT_SEL set_fmt);

extern VOID ime_eng_set_tmnr_out_ref_flip_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_ms_roi_output_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_ms_roi_flip_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_nr_luma_channel_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_nr_chroma_channel_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_statistic_data_output_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_debug_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mode, UINT32 set_mv0);

extern VOID ime_eng_set_tmnr_y_pre_blur_strength_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 pre_y_blur_str);

extern VOID ime_eng_set_tmnr_me_control_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_update_mode, UINT32 set_bndy, UINT32 set_ds_mode);

extern VOID ime_eng_set_tmnr_me_sad_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_sad_type, UINT32 set_sad_shift);

extern VOID ime_eng_set_tmnr_nr_false_color_control_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_nr_false_color_control_strength_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_val);

extern VOID ime_eng_set_tmnr_nr_center_zero_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_ps_mv_check_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_ps_mv_roi_check_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_tmnr_ps_control_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mv_info_mode, UINT32 set_ps_mode);

extern VOID ime_eng_set_tmnr_ne_sampling_horizontal_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_start_pos, UINT32 set_step, UINT32 set_num);

extern VOID ime_eng_set_tmnr_ne_sampling_vertical_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_start_pos, UINT32 set_step, UINT32 set_num);

extern VOID ime_eng_set_tmnr_me_sad_penalty_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 *p_sad_penalty);

extern VOID ime_eng_set_tmnr_me_switch_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_me_switch_th, UINT8 me_switch_ratio);
extern VOID ime_eng_set_tmnr_me_cost_blending_ratio_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 me_cost_blend);

extern VOID ime_eng_set_tmnr_me_detail_penalty_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_me_detail_penalty);

extern VOID ime_eng_set_tmnr_me_probability_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_me_probability);

extern VOID ime_eng_set_tmnr_me_rand_bit_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 me_rand_bitx, UINT32 me_rand_bity);

extern VOID ime_eng_set_tmnr_me_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 me_min_detail, UINT32 me_periodic_th);

extern VOID ime_eng_set_tmnr_md_final_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_md_k1, UINT32 set_md_k2);

extern VOID ime_eng_set_tmnr_md_roi_final_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_md_roi_k1, UINT32 set_md_roi_k2);

extern VOID ime_eng_set_tmnr_md_sad_coefs_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_coefa, UINT16 *p_set_coefb);

extern VOID ime_eng_set_tmnr_md_sad_standard_deviation_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 *p_set_std);

extern VOID ime_eng_set_tmnr_mc_noise_base_level_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 *p_set_base_level);

extern VOID ime_eng_set_tmnr_mc_edge_coefs_offset_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_coefa, UINT16 *p_set_coefb);

extern VOID ime_eng_set_tmnr_mc_sad_standard_deviation_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 *p_set_std);

extern VOID ime_eng_set_tmnr_mc_final_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mc_k1, UINT32 set_mc_k2);

extern VOID ime_eng_set_tmnr_mc_roi_final_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mc_roi_k1, UINT32 set_mc_roi_k2);

extern VOID ime_eng_set_tmnr_ps_mv_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mv_th, UINT32 set_roi_mv_th);

extern VOID ime_eng_set_tmnr_ps_mix_ratio_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mix_rto0, UINT32 set_mix_rto1);

extern VOID ime_eng_set_tmnr_ps_mix_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mix_th0, UINT32 set_mix_th1);

extern VOID ime_eng_set_tmnr_ps_mix_slope_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mix_slp0, UINT32 set_mix_slp1);

extern VOID ime_eng_set_tmnr_ps_down_sample_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_ds_th);

extern VOID ime_eng_set_tmnr_ps_roi_down_sample_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_ds_roi_th);

extern VOID ime_eng_set_tmnr_ps_edge_control_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_start_point, UINT32 set_th0, UINT32 set_th1, UINT32 set_slope);

extern VOID ime_eng_set_tmnr_ps_path_error_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_th);

extern VOID ime_eng_set_tmnr_ps_threshold_of_tf0_blur_for_smooth_region_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 set_th);

extern VOID ime_eng_set_tmnr_nr_type_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_type);

extern VOID ime_eng_set_tmnr_nr_luma_residue_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_th0, UINT8 set_th1, UINT8 set_th2);

extern VOID ime_eng_set_tmnr_nr_chroma_residue_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_th);

extern VOID ime_eng_set_tmnr_nr_varied_frequency_filter_weight_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_wets);

extern VOID ime_eng_set_tmnr_nr_luma_filter_weight_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_wets);

//extern VOID ime_eng_set_tmnr_nr_prefiltering_type_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 pf_type);
extern VOID ime_eng_set_tmnr_nr_prefiltering_strength_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 pf_str);

extern VOID ime_eng_set_tmnr_nr_prefiltering_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_strs, UINT8 *p_set_rto);

extern VOID ime_eng_set_tmnr_nr_snr_control_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_strs, UINT32 set_th);

extern VOID ime_eng_set_tmnr_nr_tnr_control_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_strs, UINT32 set_th);

extern VOID ime_eng_set_tmnr_nr_luma_noise_reduction_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_lut, UINT8 *p_set_rto);
extern VOID ime_eng_set_tmnr_nr_luma_compensation_strength_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_str);

extern VOID ime_eng_set_tmnr_nr_chroma_noise_reduction_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_lut, UINT8 *p_set_rto);
extern VOID ime_eng_set_tmnr_nr_tf0_filter_strength_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_str_blur, UINT8 *p_set_str_y, UINT8 *p_set_str_c);
extern VOID ime_eng_set_tmnr_nr_y_tf0_blur_estr_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_str);

extern VOID ime_eng_set_tmnr_statistic_data_output_start_position_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_start_x, UINT32 set_start_y);
extern VOID ime_eng_set_tmnr_statistic_data_output_sample_numbers_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_num_x, UINT32 set_num_y);
extern VOID ime_eng_set_tmnr_statistic_data_output_sample_steps_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_step_x, UINT32 set_step_y);



extern VOID ime_eng_set_tmnr_in_ref_y_uv_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_ofs_y, UINT32 set_ofs_uv, UINT32 set_start_y_ofs, UINT32 set_start_uv_ofs);
//extern VOID ime_eng_set_tmnr_in_ref_uv_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_ofs_uv);
extern VOID ime_eng_set_tmnr_in_ref_y_addr_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);
extern VOID ime_eng_set_tmnr_in_ref_uv_addr_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);

extern VOID ime_eng_set_tmnr_out_ref_y_uv_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_ofs_y, UINT32 set_ofs_uv, UINT32 set_start_y_ofs, UINT32 set_start_uv_ofs);
//extern VOID ime_eng_set_tmnr_out_ref_uv_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_ofs_uv);
extern VOID ime_eng_set_tmnr_out_ref_y_addr_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);
extern VOID ime_eng_set_tmnr_out_ref_uv_addr_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);

extern VOID ime_eng_set_tmnr_in_motion_status_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_lofs, UINT32 set_in_motio_start_ofs, UINT32 set_out_motio_start_ofs);
extern VOID ime_eng_set_tmnr_in_motion_status_address_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);
extern VOID ime_eng_set_tmnr_out_motion_status_address_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);

extern VOID ime_eng_set_tmnr_out_roi_motion_status_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_lofs, UINT32 set_start_ofs);
extern VOID ime_eng_set_tmnr_out_roi_motion_status_address_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);

extern VOID ime_eng_set_tmnr_in_motion_vector_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_lofs, UINT32 set_in_motion_start_ofs, UINT32 set_out_motion_start_ofs);
extern VOID ime_eng_set_tmnr_in_motion_vector_address_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);
extern VOID ime_eng_set_tmnr_out_motion_vector_address_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);

extern VOID ime_eng_set_tmnr_out_statistic_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_lofs, UINT32 set_start_ofs);
extern VOID ime_eng_set_tmnr_out_statistic_address_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);


extern VOID ime_eng_set_tmnr_fast_converge_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_en);
extern VOID ime_eng_set_tmnr_fast_converge_control_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_sp, UINT8 set_step);
extern VOID ime_eng_set_tmnr_fast_converge_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_lofs, UINT32 set_in_fcg_start_ofs, UINT32 set_out_fcg_start_ofs);
extern VOID ime_eng_set_tmnr_in_fast_converge_address_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);
extern VOID ime_eng_set_tmnr_out_fast_converge_address_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);

extern VOID ime_eng_set_tmnr_random_seed_reset_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_en);


//extern VOID ime_eng_set_tmnr_out_ref_crop_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
//extern VOID ime_eng_set_tmnr_out_ref_crop_position_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 pos_x, UINT32 pos_y);
//extern VOID ime_eng_set_tmnr_out_ref_crop_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 size_h, UINT32 size_v);


extern VOID ime_eng_set_tmnr_motion_detection_chroma_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 th_u, UINT32 th_v);
extern VOID ime_eng_set_tmnr_temporal_filter_tf0_ratio_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 str0, UINT32 str1);
extern VOID ime_eng_set_tmnr_motion_sat_ratio_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 sat_ratio);
extern VOID ime_eng_set_tmnr_temporal_filter_tf0_reesidual_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 th);
extern VOID ime_eng_set_tmnr_color_dithering_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT16 *th);
extern VOID ime_eng_set_tmnr_color_dithering_bit_number_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *bnum);

extern VOID ime_eng_set_tmnr_reference_output_localtion_selection_buf_reg(IME_ENG_HANDLE *p_eng, IME_TMNR_REF_OUT_LOCATION_SEL set_sel);
extern VOID ime_eng_set_tmnr_reference_output_clamp_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 min_y, UINT32 max_y, UINT32 min_uv, UINT32 max_uv);


#if (IME_GET_API_EN == 1)

extern UINT32 ime_eng_get_tmnr_out_ref_encoder_enable_buf_reg(IME_ENG_HANDLE *p_eng, void);
extern UINT32 ime_eng_get_tmnr_enable_buf_reg(IME_ENG_HANDLE *p_eng, VOID);

extern ULONG ime_eng_get_tmnr_in_ref_y_addr_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern UINT32 ime_eng_get_tmnr_in_ref_uv_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern UINT32 ime_eng_get_tmnr_in_ref_y_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern ULONG ime_eng_get_tmnr_in_ref_uv_addr_buf_reg(IME_ENG_HANDLE *p_eng, VOID);

extern UINT32 ime_eng_get_tmnr_out_ref_uv_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern ULONG ime_eng_get_tmnr_out_ref_y_addr_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern UINT32 ime_eng_get_tmnr_out_ref_y_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern ULONG ime_eng_get_tmnr_out_ref_uv_addr_buf_reg(IME_ENG_HANDLE *p_eng, VOID);

extern UINT32 ime_eng_get_tmnr_in_motion_status_address_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern UINT32 ime_eng_get_tmnr_in_motion_status_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern ULONG ime_eng_get_tmnr_out_motion_status_address_buf_reg(IME_ENG_HANDLE *p_eng, VOID);

extern UINT32 ime_eng_get_tmnr_out_roi_motion_status_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern ULONG ime_eng_get_tmnr_out_roi_motion_status_address_buf_reg(IME_ENG_HANDLE *p_eng, VOID);

extern UINT32 ime_eng_get_tmnr_in_motion_vector_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern ULONG ime_eng_get_tmnr_in_motion_vector_address_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern ULONG ime_eng_get_tmnr_out_motion_vector_address_buf_reg(IME_ENG_HANDLE *p_eng, VOID);

extern UINT32 ime_eng_get_tmnr_out_statistic_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern ULONG ime_eng_get_tmnr_out_statistic_address_buf_reg(IME_ENG_HANDLE *p_eng, VOID);



#endif


#ifdef __cplusplus
}
#endif


#endif // _IME_3DNR_REG_


