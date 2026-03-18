#ifndef __IME_ENG_CAL_DUAL_PARAMS_H__
#define __IME_ENG_CAL_DUAL_PARAMS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"

#include "ime_eng_int_comm.h"


typedef enum _SCALE_METHOD_SEL_ {
	SCALE_METHOD_BICUBIC    = 0,
	SCALE_METHOD_BILINEAR   = 1,
	SCALE_METHOD_NEAREST    = 2,
	SCALE_METHOD_ISD        = 3,
	ENUM_DUMMY4WORD(SCALE_METHOD_SEL)
} SCALE_METHOD_SEL;



typedef struct _IMAGE_SIZE_ {
	UINT32 size_h;
	UINT32 size_v;
} IMAGE_SIZE;


typedef struct _DUAL_STRIPE_INFO_ {
	UINT32 *stp_size;
	UINT32 *stp_pos;

	UINT32 stp_num;
} DUAL_STRIPE_INFO;



typedef struct _DUAL_OUTPUT_PATH_PARAMS_ {
	UINT32              sca_path_en;
	UINT32              sca_in_size_h;  // scale input image width
	UINT32              sca_in_size_v;  // scale input image height

	SCALE_METHOD_SEL    sca_method;     // scale method

	UINT32              sca_size_h;     // scale image width
	UINT32              sca_size_v;     // scale image height
	UINT32              sca_init_ofs_h; // scale initial offset for horizontal direction

	UINT32              sca_out_crop_pos_x;
	UINT32              sca_out_crop_pos_y;
	UINT32              sca_out_crop_size_h;
	UINT32              sca_out_crop_size_v;
} DUAL_OUTPUT_PATH_PARAMS;

typedef struct _DUAL_GBL_PARAMS_ {
	UINT32 dual_in_size_h;
	UINT32 dual_in_size_v;
	UINT32 dual_overlap_size;

	UINT32 dual_stp_size[2];
	UINT32 dual_stp_pos[2];
	UINT32 dual_stp_num;

	DUAL_OUTPUT_PATH_PARAMS path0;
	DUAL_OUTPUT_PATH_PARAMS path1;
	DUAL_OUTPUT_PATH_PARAMS path2;
	DUAL_OUTPUT_PATH_PARAMS path3;
} DUAL_GBL_PARAMS;


typedef struct _INPUT_PATH_REG_PARAMS_ {
	UINT32 in_size_h;
	UINT32 in_size_v;

	UINT32 in_sca_size_h;
	UINT32 in_sca_size_v;
	UINT32 in_sca_init_ofs;

	UINT32 in_sca_crop_pos_x;
	UINT32 in_sca_crop_pos_y;
	UINT32 in_sca_crop_size_h;
	UINT32 in_sca_crop_size_v;

	UINT32 in_buf_start_ofs_y;
	UINT32 in_buf_start_ofs_uv;
} INPUT_PATH_REG_PARAMS;


typedef struct _OUTPUT_PATH_REG_PARAMS_ {
	UINT32 sca_path_en;         // not for 3DNR
	UINT32 sca_dram_out_en;     // not for 3DNR

	UINT32 sca_pre_crop_pos_x;
	UINT32 sca_pre_crop_pos_y;
	UINT32 sca_pre_crop_size_h;
	UINT32 sca_pre_crop_size_v;

	UINT32 sca_size_h;
	UINT32 sca_size_v;
	UINT32 sca_init_ofs_h;

	UINT32 sca_post_crop_pos_x;
	UINT32 sca_post_crop_pos_y;
	UINT32 sca_post_crop_size_h;
	UINT32 sca_post_crop_size_v;

	UINT32 out_buf_start_ofs_y;
	UINT32 out_buf_start_ofs_uv;
} OUTPUT_PATH_REG_PARAMS;


typedef struct _IME_ENG_REG_PARAMS_ {
	INPUT_PATH_REG_PARAMS  in_path_reg;
	INPUT_PATH_REG_PARAMS  in_tmnr_ref_reg;

	OUTPUT_PATH_REG_PARAMS out_path0_reg;
	OUTPUT_PATH_REG_PARAMS out_path1_reg;
	OUTPUT_PATH_REG_PARAMS out_path2_reg;
	OUTPUT_PATH_REG_PARAMS out_path3_reg;

	OUTPUT_PATH_REG_PARAMS out_tmnr_ref_reg;
} IME_ENG_REG_PARAMS;



typedef struct _va_win_info_ {
	UINT32 VA_DUAL_LEFT_OVERLAP;
	UINT32 VA_DUAL_RIGHT_OVERLAP;

	UINT32 VA_DUAL_START_OFFSET;
} va_win_info;


extern void dual_cal_stripe_params(UINT32 dual_in_size_h, UINT32 dual_ovlap_size, DUAL_STRIPE_INFO *p_get_stp);

extern void ime_eng_cal_dual_params(DUAL_GBL_PARAMS *p_gbl_param, IME_ENG_REG_PARAMS *p_get_left_param, IME_ENG_REG_PARAMS *p_get_right_param);

extern void dual_cal_va_params(DUAL_GBL_PARAMS *p_dual_img_info, va_win_info *p_left_va_info, va_win_info *p_right_va_info);


#ifdef __cplusplus
}
#endif


#endif



