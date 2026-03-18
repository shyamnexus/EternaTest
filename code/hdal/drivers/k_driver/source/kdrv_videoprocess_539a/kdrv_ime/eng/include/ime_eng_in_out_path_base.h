
#ifndef _IME_ENG_IN_OUT_PATH_BASE_H_
#define _IME_ENG_IN_OUT_PATH_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif



#include "kwrap/type.h"
#include "ime_eng_int_comm.h"
#include "ime_eng_comm.h"

typedef enum _IME_ENG_P0_MB_BUF_EN_ {
	IME_ENG_P0_MB_BUF_ENABLE  = 0,
	IME_ENG_P0_MB_BUF_DISABLE = 1,
	ENUM_DUMMY4WORD(IME_ENG_P0_MB_BUF_EN)
} IME_ENG_P0_MB_BUF_EN;


typedef struct _IME_ENG_SCL_PARAM_ {
	UINT32 in_size_h;   ///< input image width
	UINT32 in_size_v;   ///< input image height
	UINT32 scl_size_h;  ///< scaling image width
	UINT32 scl_size_v;  ///< scaling image height
	UINT32 scl_method;
} IME_ENG_SCL_PARAM;


extern IME_ENG_OPMODE g_ime_op_mode;

//--------------------------------------------------------------------------------
// input path
extern ER ime_eng_set_op_mode_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_OPMODE set_mode);

extern VOID ime_eng_set_input_image_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_size_h, UINT32 set_size_v);
//extern VOID ime_eng_set_dual_input_image_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_size_h, UINT32 set_size_v);

extern VOID ime_eng_set_input_source_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 in_src_sel);
extern VOID ime_eng_set_input_image_format_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 in_fmt_sel);

extern VOID ime_eng_set_stripe_size_mode_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_mode);
extern VOID ime_eng_set_horizontal_fixed_stripe_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_hn, UINT32 set_hl);
extern VOID ime_eng_set_horizontal_stripe_number_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_hm);
extern VOID ime_eng_set_horizontal_varied_stripe_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 *p_set_hns);
extern VOID ime_eng_set_vertical_stripe_info_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_vn, UINT32 set_vl, UINT32 set_vm);
extern VOID ime_eng_set_horizontal_overlap_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 h_overlap_sel, UINT32 h_overlap, UINT32 h_prt_sel, UINT32 h_prt_size);


/**
    set input Y/UV lineoffset
*/
extern VOID ime_eng_set_input_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_y_lofs, UINT32 set_c_lofs, UINT32 set_start_y_ofs, UINT32 set_start_uv_ofs);

/**
    set input Y/U/V buffer address
*/
//extern VOID ime_eng_set_input_addr_buf_reg(IME_ENG_HANDLE *p_eng, ULONG addr_y, ULONG addr_u, ULONG addr_v);

extern VOID ime_eng_set_input_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_BUF_SEL buf_sel, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);


//extern VOID ime_eng_st_input_scale_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
//extern VOID ime_eng_set_input_scale_image_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_size_h, UINT32 set_size_v);

//extern VOID ime_eng_set_p0_mbbuf_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_P0_MB_BUF_EN set_en);

#if (IME_GET_API_EN == 1)

extern VOID ime_eng_get_input_image_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 *p_get_size_h, UINT32 *p_get_size_v);
extern UINT32 ime_eng_get_input_source_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern UINT32 ime_eng_get_input_image_format_buf_reg(IME_ENG_HANDLE *p_eng, VOID);

extern VOID ime_eng_get_horizontal_stripe_info_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 *p_get_hn, UINT32 *p_get_hl, UINT32 *p_get_hm);
extern VOID ime_eng_get_vertical_stripe_info_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 *p_get_vn, UINT32 *p_get_vl, UINT32 *p_get_vm);
extern VOID ime_eng_get_horizontal_overlap_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 *p_get_h_overlap_sel, UINT32 *p_get_h_overlap);
extern UINT32 ime_eng_get_stripe_mode_buf_reg(IME_ENG_HANDLE *p_eng, VOID);

extern UINT32 ime_eng_get_input_lineoffset_y_buf_reg(IME_ENG_HANDLE *p_eng, VOID);
extern UINT32 ime_eng_get_input_lineoffset_c_buf_reg(IME_ENG_HANDLE *p_eng, VOID);

extern VOID ime_eng_get_input_channel_addr1_buf_reg(IME_ENG_HANDLE *p_eng, ULONG *p_get_addr_y, ULONG *p_get_addr_u, ULONG *p_get_addr_v);

#endif



//--------------------------------------------------------------------------------
// output path register APIs

/**
    set output path enable
*/
extern VOID ime_eng_set_output_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_en);

/**
    set output path flip enable
*/
extern VOID ime_eng_set_flip_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_en);

/**
    set output path dram-out enable
*/
extern VOID ime_eng_set_output_dram_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_en);

/**
    set output path image data format
*/
extern VOID ime_eng_set_output_image_format_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 out_format_sel);

/**
    set output path image data type
*/
extern VOID ime_eng_set_output_type_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_out_fmt_type);


/**
    set scaling interpolation method
*/
extern VOID ime_eng_set_scale_interpolation_method_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_scl_method);

/**
    set scaling size
*/
extern VOID ime_eng_set_scaling_size_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 h_scale_size, UINT32 v_scale_size);


/**
    set output encoder enable
*/
extern VOID ime_eng_set_encode_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_en);

/**
    set scaling output enhance factor
*/
extern VOID ime_eng_set_scaling_enhance_factor_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 scl_enh_ftr, UINT32 scl_enh_sht);

/**
    set scale pre-crop position
*/
//extern VOID ime_eng_set_scale_precrop_coordinate_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 crop_pos_x, UINT32 crop_pos_y);

/**
    set output crop position
*/
extern VOID ime_eng_set_output_crop_coordinate_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 crop_pos_x, UINT32 crop_pos_y);

/**
    set scale pre_crop size
*/
//extern VOID ime_eng_set_scale_precrop_size_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_size_h, UINT32 set_size_v);


/**
    set output size
*/
extern VOID ime_eng_set_output_size_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_size_h, UINT32 set_size_v);

/**
    set output Y/UV buffer lineoffset
*/
extern VOID ime_eng_set_output_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_y_lofs, UINT32 set_c_lofs, UINT32 set_start_y_ofs, UINT32 set_start_uv_ofs);

/**
    set output Y/U/V buffer address
*/
//extern VOID ime_eng_set_output_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, ULONG out_addr_y, ULONG out_addr_cb, ULONG out_addr_cr);

extern VOID ime_eng_set_output_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, IME_ENG_BUF_SEL buf_sel, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);



/**
    set output path Y/U/V range clamp
*/
extern VOID ime_eng_set_clamp_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 min_y, UINT32 max_y, UINT32 min_uv, UINT32 max_uv);


/**
    set ISD2 user coefficients
*/
extern VOID ime_eng_set_integration_scaling_factor_base_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_base_h_factor, UINT32 set_base_v_factor);

//extern VOID ime_eng_set_integration_scaling_coefs_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, INT32 *hcoefs);
//extern VOID ime_eng_set_integration_scaling_ctrl_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 isd_mode, UINT32 isd_h_coef_adj, UINT32 isd_v_coef_adj);
extern VOID ime_eng_set_integration_scaling_factor0_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_isd_h_factor, UINT32 set_isd_v_factor);
extern VOID ime_eng_set_integration_scaling_factor1_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_isd_h_factor, UINT32 set_isd_v_factor);
extern VOID ime_eng_set_integration_scaling_factor2_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_isd_h_factor, UINT32 set_isd_v_factor);


//-------------------------------------------------------------------------------
// stitching

#if 1
/**
    set output path stitching enable
*/
extern VOID ime_eng_set_stitching_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_en);

/**
    set output path stitching horizontal crop position
*/
extern VOID ime_eng_set_stitching_base_position_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 base_pos);

/**
    set output path stitching Y/UV buffer lineoffset
*/
extern VOID ime_eng_set_stitching_output_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 set_y_lofs, UINT32 set_c_lofs);

/**
    set output path stitching Y/UV buffer address
*/
extern VOID ime_eng_set_stitching_output_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, ULONG out_addr_y, ULONG out_addr_cb, ULONG out_addr_cr);
#endif
//-------------------------------------------------------------------------------

/**
    set output path image scaling factors
*/
extern ER ime_eng_set_scaling_param_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, IME_ENG_SCL_PARAM *p_set_scl_param);

//-------------------------------------------------------------------------------

#if (IME_GET_API_EN == 1)

extern VOID ime_eng_get_scaling_size_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 *p_get_size_h, UINT32 *p_get_size_v);
extern VOID ime_eng_get_output_size_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 *p_get_size_h, UINT32 *p_get_size_v);
extern VOID ime_eng_get_output_crop_coordinate_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 *p_get_crop_pos_x, UINT32 *p_get_crop_pos_y);
extern VOID ime_eng_get_output_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, UINT32 *p_get_y_lofs, UINT32 *p_get_c_lofs);

extern VOID ime_eng_get_output_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, ULONG *p_get_y_addr, ULONG *p_get_u_addr, ULONG *p_get_v_addr);

extern UINT32 ime_eng_get_output_path_enable_status_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel);
extern UINT32 ime_eng_get_output_format_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel);

extern UINT32 ime_eng_get_output_format_type_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel);
extern UINT32 ime_eng_get_scaling_method_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel);



extern UINT32 ime_eng_get_stitching_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel);
extern VOID ime_eng_get_stitching_output_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel UINT32 *p_get_y_lofs, UINT32 *p_get_c_lofs);
extern VOID ime_eng_get_stitching_output_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_ENG_PATH_SEL path_sel, ULONG *p_get_y_addr, ULONG *p_get_u_addr, ULONG *p_get_v_addr);


#endif

#ifdef __cplusplus
}
#endif


#endif


