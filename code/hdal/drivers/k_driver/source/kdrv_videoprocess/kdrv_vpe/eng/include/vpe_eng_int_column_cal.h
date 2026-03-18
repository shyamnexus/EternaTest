#ifndef _VPE_ENG_INT_COLUMN_CAL_H_
#define _VPE_ENG_INT_COLUMN_CAL_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"

#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000


#define VPE_REG_OFFSET               4
#define VPE_REG_MAXBIT              32
#define VPE_COL_MST_DBG   0

#define VPE_ENG_RES_MAX                 4
#define VPE_ENG_DUP_MAX                 3
#define VPE_ENG_COL_MAX                 32//24//8
#define VPE_ENG_HW_COL_MAX              1
#define VPE_ENG_SCALE_PRECISION         65536
#define VPE_ENG_SCALE_PRECISION_BIT     16 //VPE_ENG_SCALE_PRECISION = 2 ^ 16
#define VPE_ENG_HW_LIMIT_SCA_CROP_H     16384
#define VPE_ENG_HW_LIMIT_PROC_H         16384
#define VPE_ENG_COL_2D_LUT_W_LIMIT      257
#define VPE_ENG_COL_PTZ_LUT_W_LIMIT     128
#define VPE_ENG_SEG_MAX                 3

//========================================================

#define VPE_ALIGN_FLOOR(value, base)  ((value) & ~((base)-1))                   ///< Align Floor
#define VPE_ALIGN_ROUND(value, base)  VPE_ALIGN_FLOOR((value) + ((base)/2), base)   ///< Align Round
#define VPE_ALIGN_CEIL(value, base)   VPE_ALIGN_FLOOR((value) + ((base)-1), base)   ///< Align Ceil


#define VPE_ALIGN_ROUND_128(a)      VPE_ALIGN_ROUND(a, 128)  ///< Round Off to 128
#define VPE_ALIGN_ROUND_64(a)       VPE_ALIGN_ROUND(a, 64)  ///< Round Off to 64
#define VPE_ALIGN_ROUND_32(a)       VPE_ALIGN_ROUND(a, 32)  ///< Round Off to 32
#define VPE_ALIGN_ROUND_16(a)       VPE_ALIGN_ROUND(a, 16)  ///< Round Off to 16
#define VPE_ALIGN_ROUND_8(a)        VPE_ALIGN_ROUND(a, 8)   ///< Round Off to 8
#define VPE_ALIGN_ROUND_4(a)        VPE_ALIGN_ROUND(a, 4)   ///< Round Off to 4
#define VPE_ALIGN_ROUND_2(a)        VPE_ALIGN_ROUND(a, 2)   ///< Round Off to 2

#define VPE_ALIGN_CEIL_128(a)       VPE_ALIGN_CEIL(a, 128)  ///< Round Up to 128
#define VPE_ALIGN_CEIL_64(a)        VPE_ALIGN_CEIL(a, 64)  ///< Round Up to 64
#define VPE_ALIGN_CEIL_32(a)        VPE_ALIGN_CEIL(a, 32)   ///< Round Up to 32
#define VPE_ALIGN_CEIL_16(a)        VPE_ALIGN_CEIL(a, 16)   ///< Round Up to 16
#define VPE_ALIGN_CEIL_8(a)         VPE_ALIGN_CEIL(a, 8)    ///< Round Up to 8
#define VPE_ALIGN_CEIL_4(a)         VPE_ALIGN_CEIL(a, 4)    ///< Round Up to 4
#define VPE_ALIGN_CEIL_2(a)         VPE_ALIGN_CEIL(a, 2)    ///< Round Up to 2

#define VPE_ALIGN_FLOOR_128(a)       VPE_ALIGN_FLOOR(a, 128)  ///< Round down to 128
#define VPE_ALIGN_FLOOR_64(a)        VPE_ALIGN_FLOOR(a, 64)  ///< Round down to 64
#define VPE_ALIGN_FLOOR_32(a)       VPE_ALIGN_FLOOR(a, 32)  ///< Round down to 32
#define VPE_ALIGN_FLOOR_16(a)       VPE_ALIGN_FLOOR(a, 16)  ///< Round down to 16
#define VPE_ALIGN_FLOOR_8(a)        VPE_ALIGN_FLOOR(a, 8)   ///< Round down to 8
#define VPE_ALIGN_FLOOR_4(a)        VPE_ALIGN_FLOOR(a, 4)   ///< Round down to 4
#define VPE_ALIGN_FLOOR_2(a)        VPE_ALIGN_FLOOR(a, 2)   ///< Round down to 2

//========================================================

typedef enum {
	STPMODE_SST     = 0,  ///< single stripe mode
	STPMODE_MST     = 1,  ///< horizontal multiple stripe mode
} STP_MODE;


typedef struct _POS_INFO_ {
	UINT16 start_x;
	UINT16 end_x;
} POS_INFO;


typedef struct _VPE_RES_EN_PARAM_ {
	UINT32 res_en[VPE_ENG_RES_MAX];
	UINT32 res_out_size[VPE_ENG_RES_MAX];
} VPE_RES_EN_PARAM;

typedef struct _STRIPE_CHK_ {
	UINT16 sw_stp_size_min_limit;
	UINT16 col_2dlut_limit;
	UINT16 isd_col_out_size_limit;
	UINT8 path_en_cnt;
    UINT8 dce_en;
} STRIPE_CHK;

typedef struct _STRIPE_PARAM_ {
	UINT32 stp_hn;
	UINT32 stp_hl;
	UINT32 stp_hm;

	UINT32 stp_overlap;
} STRIPE_PARAM;

typedef struct _VPE_IMG_SCALE_PARAM_ {
	UINT32 path_en;

	UINT32 in_size_h;
	UINT32 in_size_v;
	//UINT32 in_lofs;

	UINT32 out_size_h;
	UINT32 out_size_v;
	//UINT32 out_lofs;

	UINT32 stp_nums;
} VPE_IMG_SCALE_PARAM;



typedef struct _VPE_IN_PATH_INFO_ { // view scene

	UINT8 rotation;
	UINT16 src_width;
	UINT16 src_height;
	UINT16 dce_width;
	UINT16 dce_height;
	UINT16 proc_x_pos;
	UINT16 proc_y_pos;
	UINT16 proc_width;
	UINT16 proc_height;
	UINT16 dce_lens_radius;
} VPE_IN_PATH_INFO;

typedef struct _VPE_OUT_PATH_INFO_ {
    UINT8 out_dup_num;
    UINT8 path_en;
	UINT8 sca_comps_en[VPE_ENG_DUP_MAX];   //max duplicate 3
	UINT8 sca_mb_en[VPE_ENG_DUP_MAX];      //max duplicate 3

	UINT16 sca_proc_crop_x_pos;  // target crop x position
	UINT16 sca_proc_crop_y_pos;  // target crop y position
	UINT16 sca_proc_crop_width;  // target crop width
	UINT16 sca_proc_crop_height;   // target crop height

	UINT16 sca_width;
	UINT16 sca_height;

	UINT16 sca_tag_crop_x_pos;   // crop x position after scaling
	UINT16 sca_tag_crop_y_pos;   // crop y position after scaling
	UINT16 sca_tag_crop_width;   // crop width after scaling
	UINT16 sca_tag_crop_height;  // crop height after scaling


	UINT16 des_width;
	UINT16 des_height;

	UINT16 out_x_pos;
	UINT16 out_y_pos;
	UINT16 out_width;
	UINT16 out_height;

	UINT16 rlt_x_pos;
	UINT16 rlt_y_pos;
	UINT16 rlt_width;
	UINT16 rlt_height;

	UINT16 pip_x_pos;
	UINT16 pip_y_pos;
	UINT16 pip_width;
	UINT16 pip_height;
	UINT16 sca_method;//0: bilinear, 1: isd2	
} VPE_OUT_PATH_INFO;

typedef struct _VPE_SCALE_FACTORS_ {
	UINT32 sca_factor_h;
	UINT32 sca_factor_v;
	UINT32 sca_size_h;
	UINT32 sca_size_v;
	UINT32 sca_pos_x;
	UINT32 sca_pos_y;
} VPE_SCALE_FACTORS;

typedef struct _VPE_MD_PARAM_ {
	UINT16  md_in_x;
	UINT16  md_in_y;
	UINT8  md_win_size;
} VPE_MD_PARAM;


typedef struct _VPE_BGL_STRIPE_PARAM_ {
	VPE_IN_PATH_INFO    vpe_in_param;
	VPE_OUT_PATH_INFO   vpe_out_param[VPE_ENG_RES_MAX];
	VPE_SCALE_FACTORS   vpe_sca_factor_param[VPE_ENG_RES_MAX];
    //VPE_MD_PARAM        vpe_md_param;
	UINT8 stp_num_multiply_base;
	UINT8 stp_size_reduce_rate;
	UINT8            vpe_dce_en;
	UINT8            vpe_ptz_en;//1: ptz
    UINT32            vpe_dce_hfact;
	UINT8             bdc_rot_limit;
	
    //UINT8             ne_blk_sample_step;
    //UINT8             tmnr_refr_dec_en;

} VPE_BGL_STRIPE_PARAM;

typedef struct {
    UINT16  proc_width;
    UINT16  proc_x_start;
	/* TMNR */
	//UINT16  refr_x_start;
	//UINT16  refw_x_start;

	//UINT16  refw_crop_x_start;
	//UINT16  refw_crop_width;
    //#20210125-cliff_modify_start
	//UINT16  refw_crop_skip;
    //#20210125-cliff_modify_end
	//UINT16  tmnr_info_xnum;
	//UINT16  tmnr_info_xoft;

	/* MD */
	//UINT16  md_win_x;
	//UINT16  md_win_x_num;
	//UINT16  md_win_x_start_num;

	//UINT16  vc_idx_mode;
	/* MASK & DCE */
	UINT16  col_x_start;

} COLUMN_SIZE_INFO;

typedef struct _VPE_HW_IN_COL_PARAM_ {
	COLUMN_SIZE_INFO in_col_info[VPE_ENG_COL_MAX];
	UINT32 col_num;
    BOOL fill_col[VPE_ENG_COL_MAX];
} VPE_HW_IN_COL_PARAM;


typedef struct _VPE_OUT_MST_INFO_ {
	UINT32 sca_in_crop_size_h;
	UINT32 sca_in_crop_size_v;
	UINT32 sca_in_crop_pos_x;
	UINT32 sca_in_crop_pos_y;

	UINT32 sca_out_size_h;    // scaling size h
	UINT32 sca_out_size_v;    // scaling size v

	UINT32 sca_out_crop_size_h;  // target crop width
	UINT32 sca_out_crop_size_v;  // target crop height
	UINT32 sca_out_crop_pos_x;   // target crop start position x
	UINT32 sca_out_crop_pos_y;   // target crop start position y

	UINT32 rlt_pos_x;     // result output position x
	UINT32 rlt_pos_y;     // result output position y
	UINT32 rlt_size_h;    // result output width
	UINT32 rlt_size_v;    // result output height

	UINT32 out_pos_x;     // final output position x after target crop
	UINT32 out_pos_y;     // final output position y after target crop
	UINT32 out_size_h;    // final output width after target crop
	UINT32 out_size_v;    // final output height after target crop

	UINT32 pip_pos_x;     // final pip position x after target crop
	UINT32 pip_pos_y;     // final pip position y after target crop
	UINT32 pip_size_h;    // final pip width after target crop
	UINT32 pip_size_v;    // final pip height after target crop



	INT32 scl_h_init_ofs_int;
	INT32 scl_h_init_ofs_dec;
	INT32 vpe_scl_h_ftr_init;

	INT32 scl_v_init_ofs_int;
	INT32 scl_v_init_ofs_dec;
	INT32 vpe_scl_v_ftr_init;

	UINT32 skip_en;
} VPE_OUT_MST_INFO;

typedef struct _VPE_SCA_SRC_IN_STRIPE_PARAM_ {
	VPE_OUT_MST_INFO sca_stp_info[VPE_ENG_COL_MAX];  // sw output stripe parameters

	UINT32 sca_factor_h;//((in_h-1)* VPE_ENG_SCALE_PRECISION)/(out_h-1)
	UINT32 sca_factor_v;//((in_v-1)* VPE_ENG_SCALE_PRECISION)/(out_v-1)
	UINT32 sca_drate_h; //(in_h-1)/(out_h-1)
	UINT32 sca_drate_v; //(in_v-1)/(out_v-1)
	UINT32 sca_drate_decimal_h; //(((in_v-1)/(out_v-1))  -  sca_drate_v) * VPE_ENG_SCALE_PRECISION
	UINT32 sca_drate_decimal_v; //(((in_v-1)/(out_v-1))  -  sca_drate_v) * VPE_ENG_SCALE_PRECISION
	UINT32 isd_norm_h;  //for isd method only
	UINT32 isd_norm_v; //for isd method only
	/*
	UINT32 sca_hsca_div;
	UINT32 sca_havg_pxl_msk;
	UINT32 sca_vsca_div;
	UINT32 sca_vavg_pxl_msk;
	*/
	UINT32 stp_overlap_size;
	UINT32 valid_stp_num;
	UINT32 valid_stp_start_idx;
	UINT32 valid_stp_end_idx;

    UINT32 des_yuv420;

} VPE_SCA_SRC_IN_STRIPE_PARAM;

typedef struct _VPE_GLO_PARAM_ {
	VPE_BGL_STRIPE_PARAM  vpe_user_in_out_img_param;
	VPE_IMG_SCALE_PARAM scl_info[VPE_ENG_RES_MAX];
	STRIPE_PARAM get_src_in_sw_stp_info;
	VPE_HW_IN_COL_PARAM in_hw_col_info;
	VPE_SCA_SRC_IN_STRIPE_PARAM get_sca_in_stp_info[VPE_ENG_RES_MAX];
} VPE_GLO_PARAM;

typedef struct _RES_SCA_DATA_ {
	UINT8 path_en;
	UINT16 sca_in_size_h;
	UINT16 sca_in_pos_start_x;
	UINT16 sca_in_pos_end_x;
	UINT16 sca_out_size_h;
	UINT32 sca_factor_h;
	UINT8 sca_proc_done;
} RES_SCA_DATA;


typedef struct _VPE_SCA_WET_PARAM_ {
	UINT8 sca_luma_wet;
	UINT8 sca_chroma_wet;
	UINT8 sca_chroma_hlpf;
	UINT8 sca_chroma_vlpf;
} VPE_SCA_WET_PARAM;


typedef enum{
    VPE_STP_NONE = 0,
    VPE_STP_SINGLE,
    VPE_STP_MULTI,

} VPE_STP_TYPE;


extern UINT32 vpe_eng_int_to_2comp(INT32 val, INT32 bits);
//extern int vpe_eng_get_scale_out_crop_info(UINT32 res_num, VPE_BGL_STRIPE_PARAM *p_stp_gbl_info, VPE_SCA_SRC_IN_STRIPE_PARAM *p_sca_in_stp_info);
extern int vpe_eng_get_scale_out_crop_info(UINT32 res_num, VPE_BGL_STRIPE_PARAM *p_stp_gbl_info, VPE_SCA_SRC_IN_STRIPE_PARAM *p_sca_in_stp_info, VPE_HW_IN_COL_PARAM *p_stp_hw_info);


extern int vpe_eng_cal_src_in_stripe_v3(VPE_BGL_STRIPE_PARAM *p_gbl_sca_info, STRIPE_PARAM *p_get_stp_info, BOOL bprtSpecErrMsg);
extern int vpe_eng_cal_src_in_stripe_v4(VPE_BGL_STRIPE_PARAM *p_gbl_sca_info, STRIPE_PARAM *p_get_stp_info, BOOL bprtSpecErrMsg);

extern void vpe_eng_get_in_column_info(VPE_BGL_STRIPE_PARAM *p_stp_gbl_info, STRIPE_PARAM *p_stp_sw_info, VPE_HW_IN_COL_PARAM *p_stp_hw_info);
//#20210120-cliff_modify_start
extern int vpe_eng_refine_in_column_info(VPE_BGL_STRIPE_PARAM *p_stp_gbl_info, VPE_HW_IN_COL_PARAM *p_stp_hw_info);
//#20210120-cliff_modify_end
extern int vpe_eng_cal_sca_in_stripe(UINT32 res_num, VPE_BGL_STRIPE_PARAM *p_stp_gbl_info, STRIPE_PARAM *p_src_in_stp_info, VPE_SCA_SRC_IN_STRIPE_PARAM *p_sca_in_stp_info);
extern int vpe_eng_get_out_column_info(UINT32 res_num, VPE_BGL_STRIPE_PARAM *p_stp_gbl_info, VPE_SCA_SRC_IN_STRIPE_PARAM *p_sca_in_stp_info);
extern void vpe_eng_refine_out_column_info(VPE_SCA_SRC_IN_STRIPE_PARAM *p_sca_in_stp_info);
extern void vpe_sca_get_filter_coefs(UINT32 sca_factor, INT32 *p_get_coefs);
extern void vpe_sca_get_weight(uint32_t sca_hfactor, uint32_t sca_vfactor, VPE_SCA_WET_PARAM *p_sca_weight_param);

#ifdef __cplusplus
}
#endif


#endif //

