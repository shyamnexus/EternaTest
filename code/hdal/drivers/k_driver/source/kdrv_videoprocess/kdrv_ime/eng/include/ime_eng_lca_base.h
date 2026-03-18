

#ifndef _IME_ENG_LCA_BASE_H_
#define _IME_ENG_LCA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif



#include "kwrap/type.h"
#include "ime_eng_int_comm.h"

typedef enum _IME_LCA_PROC_LOCATION_ {
	IME_LCA_PROC_LOCATION_PRE_3DNR  = 0,
	IME_LCA_PROC_LOCATION_POST_3DNR = 1,
	ENUM_DUMMY4WORD(IME_LCA_PROC_LOCATION)
} IME_LCA_PROC_LOCATION;

//-------------------------------------------------------------------------------
// local chroma adaption

//extern VOID ime_eng_set_lca_chroma_adj_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
//extern VOID ime_eng_set_lca_la_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
//extern VOID ime_eng_set_lca_bypass_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);



//extern VOID ime_eng_set_lca_bilateral_center_modified_wet_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_wet);
//extern VOID ime_eng_set_lca_bilateral_spatial_ker_wet_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_coefs);
//extern VOID ime_eng_set_lca_bilateral_range_ker_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_ths);


//extern VOID ime_eng_set_lca_directional_range_filter_uv_refto_y_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_wet);
//extern VOID ime_eng_set_lca_directional_range_filter_edge_flat_region_blending_wet_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_y_wet, UINT8 set_uv_wet);


//extern VOID ime_eng_set_lca_y_channel_high_freq_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_in, UINT8 *p_set_out);
//extern VOID ime_eng_set_lca_y_channel_middle_freq_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_in, UINT8 *p_set_out);
//extern VOID ime_eng_set_lca_y_channel_low_freq_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_in, UINT8 *p_set_out);

//extern VOID ime_eng_set_lca_uv_channel_high_freq_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_in, UINT8 *p_set_out);
//extern VOID ime_eng_set_lca_uv_channel_low_freq_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_in, UINT8 *p_set_out);

extern VOID ime_eng_set_lca_debug_mode_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_en, UINT8 set_sel, UINT8 set_ofs, UINT32 set_pos);


//---------------------
extern VOID ime_eng_set_lca_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ime_eng_set_lca_processing_location(IME_ENG_HANDLE *p_eng, IME_LCA_PROC_LOCATION set_proc_location);
extern VOID ime_eng_set_lca_image_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 size_h, UINT32 size_v);
extern VOID ime_eng_set_lca_scale_factor_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 factor_h, UINT32 factor_v);
extern VOID ime_eng_set_lca_scale_factor_initial_offset_buf_reg(IME_ENG_HANDLE *p_eng, INT32 init_ofs_h, INT32 init_ofs_v);
extern VOID ime_eng_set_lca_edge_detection_ker_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_ker_size, UINT32 *set_th);
extern VOID ime_eng_set_lca_range_filter_ker_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_sr_ker_size, UINT8 set_mr_ker_size);
extern VOID ime_eng_set_lca_range_filter_center_3x3_ker_coefs_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_y_th, UINT8 *p_set_u_th, UINT8 *p_set_v_th);

extern VOID ime_eng_set_lca_range_filter_center_mod_smooth_region_weight_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_sr_y_wet, UINT8 set_sr_uv_wet);
extern VOID ime_eng_set_lca_range_filter_center_mod_edge_region_weight_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_er_y_wet, UINT8 set_er_uv_wet);
extern VOID ime_eng_set_lca_range_filter_still_smootion_region_coefs_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_y_th, UINT16 *p_set_uv_th);
extern VOID ime_eng_set_lca_range_filter_still_edge_region_coefs_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_y_th, UINT16 *p_set_uv_th);
extern VOID ime_eng_set_lca_range_filter_motion_region_coefs_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_y_th, UINT16 *p_set_uv_th);
extern VOID ime_eng_set_lca_out_wet_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_y_wet, UINT8 set_uv_wet);

extern VOID ime_eng_set_lca_y_channel_still_region_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_gain);
extern VOID ime_eng_set_lca_y_channel_motion_region_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_gain);
extern VOID ime_eng_set_lca_u_channel_still_region_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_gain);
extern VOID ime_eng_set_lca_u_channel_motion_region_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_gain);
extern VOID ime_eng_set_lca_v_channel_still_region_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_gain);
extern VOID ime_eng_set_lca_v_channel_motion_region_coring_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_gain);


extern VOID ime_eng_set_lca_y_channel_still_region_cut_off_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_th);
extern VOID ime_eng_set_lca_y_channel_motion_region_cut_off_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_th);
extern VOID ime_eng_set_lca_u_channel_still_region_cut_off_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_th);
extern VOID ime_eng_set_lca_u_channel_motion_region_cut_off_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_th);
extern VOID ime_eng_set_lca_v_channel_still_region_cut_off_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_th);
extern VOID ime_eng_set_lca_v_channel_motion_region_cut_off_threshold_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 *p_set_th);


#if (IME_GET_API_EN == 1)
extern UINT32 ime_eng_get_lca_enable_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern VOID ime_eng_get_lca_image_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 *p_get_size_h, UINT32 *p_get_size_v);


#endif

#ifdef __cplusplus
}
#endif


#endif

