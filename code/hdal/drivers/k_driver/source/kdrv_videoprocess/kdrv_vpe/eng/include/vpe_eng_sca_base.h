#ifndef _VPE_ENG_SCA_BASE_H_
#define _VPE_ENG_SCA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "vpe_eng_ctrl_base.h"
#include "vpe_eng_int_column_cal.h"
#include "vpe_eng_handle.h"
#include "vpe_eng_dce_base.h"


#define ISD_USER_COEF_NUMS         32
#define VPE_ALIGN(n,x) ((n + (x-1)) & ~(x-1))
#define VPE_ALIGN64(n) VPE_ALIGN(n,64)
#define VPE_ALIGN128(n) VPE_ALIGN(n,128)

#define VPE_ENG_STPNUM_MUL_SEARCH   1 //for multi engine strp num use 

typedef struct {
	//dst information
	UINT8 path_en;
	//UINT8 sca_bypass;
	UINT8 sca_crop_en;
	UINT8 tc_en;
	UINT8 seg_op;
	//UINT8 dst_uv_swap;
 	VPE_ENG_DRT dst_drt;
	UINT16 drt_pc2tv_weight; //0~256
    VPE_ENG_DES_FMT dst_fmt;
	//UINT8 dst_420;
	//UINT8 dst_dup;
    UINT8 dst_bg_sel;
	VPE_ENG_SCA_METHOD sca_method;
} VPE_RES_CTL_INFO;


typedef struct  {
    /* information from LIB*/
    UINT16 src_width;
    UINT16 src_height;
    UINT16 src_in_width; // src in width
    UINT16 src_in_height;
    UINT16 src_in_x;
    UINT16 src_in_y;
    UINT16 out_width[VPE_ENG_RES_MAX];
    UINT8  des_dup_num[VPE_ENG_RES_MAX];
    VPE_ENG_DES_FMT des_type[VPE_ENG_RES_MAX][3];
    //UINT8  md_win_size; // 0: md window size =16, 1: md window size =32
    VPE_ENG_ROT rotation;
    VPE_ENG_SRC_FMT src_type;
    //UINT8  ne_blk_sample_step; //tmnr sample_len = sample_size(2) * (ne_blk_sample_step)
    /* information to LIB */
    UINT8  col_num; // 0: one column
    COLUMN_SIZE_INFO col_size_info[VPE_ENG_COL_MAX];
}COL_INFO_GET;


typedef enum {
	VPE_ENG_DC_ROT_NONE = 0,
	VPE_ENG_DC_ROT_90,
	VPE_ENG_DC_ROT_180,
	VPE_ENG_DC_ROT_270,
	VPE_ENG_DC_ROT_0_H_FLIP,
	VPE_ENG_DC_ROT_90_H_FLIP,
	VPE_ENG_DC_ROT_180_H_FLIP,
	VPE_ENG_DC_ROT_270_H_FLIP,
	VPE_ENG_DC_ROT_MAX 
}VPE_ENG_DC_ROT;



typedef struct  {
    UINT16 src_in_width;
    UINT16 src_in_height;
    UINT16 sca_crop_x[VPE_ENG_RES_MAX]; //source in crop x start before scaling
    UINT16 sca_crop_y[VPE_ENG_RES_MAX]; //source in crop y start before scaling
    UINT16 sca_crop_w[VPE_ENG_RES_MAX]; //source in crop width before scaling
    UINT16 sca_crop_h[VPE_ENG_RES_MAX]; //source in crop height before scaling
    UINT16 rlt_width[VPE_ENG_RES_MAX];  //result width after scaling (rlt_width)
    VPE_ENG_DES_FMT des_type[VPE_ENG_RES_MAX];
    VPE_ENG_ROT rotation;
	VPE_ENG_SRC_FMT src_type;
	VPE_ENG_DC_ROT dc_rot_sel;
    UINT8  src_path_sel;//0:dec off, 1:dce on
	UINT16  dewarp_in_width;
	UINT16  dewarp_in_height;
	UINT16  dewarp_out_width;
	UINT16  dewarp_out_height;
    UINT32 dce_lens_radius;
}COL_INFO_CHK;

typedef struct{
	VPE_ENG_SCA_METHOD sca_method;
	//UINT32 coef_mode;	//for isd method only
	//UINT32 filt_adj_h;  //for isd method only
	//UINT32 filt_adj_v;  //for isd method only
	//UINT8 isd2_user_coef[33];
}RES_SCA_INFO;

typedef struct{
	UINT32 		des_y_ofso; //bg line offset	
	UINT32 		des_uv_ofso; //bg line offset	
	UINT32 		des_y1_ofso; //bg line offset	
	UINT32 		des_uv1_ofso; //bg line offset	
	UINT32 		des_y2_ofso; //bg line offset	
	UINT32 		des_uv2_ofso; //bg line offset
    UINT16      des_width;
    UINT16      des_height;

    UINT16      des_out_w;
	UINT16      des_out_h;
    UINT16      des_out_x;
	UINT16      des_out_y;
    //UINT16      des_out_x1;

    UINT16      des_rlt_w;
	UINT16      des_rlt_h;
    UINT16      des_rlt_x;
	UINT16      des_rlt_y;

    UINT16      hole_w; //set 0 if don't use pip
	UINT16      hole_h; //set 0 if don't use pip
    UINT16      hole_x;
	UINT16      hole_y;

    UINT16      seg_pos;	//seg_op==0, seg_st for x; seg_op==1, seg_st for y
    UINT16      seg_pos1;	//seg_op==0, seg_st for x; seg_op==1, seg_st for y
    UINT16      seg_out_en[VPE_ENG_SEG_MAX];	//seg out en 
    UINT16      sca_crop_w;
	UINT16      sca_crop_h;
    UINT16      sca_crop_x;
	UINT16      sca_crop_y;

	UINT16      sca_width;
    UINT16      sca_height;
	UINT16      des_crop_out_x;
	UINT16      des_crop_out_y;
	UINT16      des_crop_out_w;
	UINT16      des_crop_out_h;

}RES_DIM_INFO;

typedef struct{
    VPE_ENG_SRC_FMT     src_type;
    VPE_ENG_ROT         rotation;
    UINT16              src_width;
	UINT16              src_height;
    UINT16              src_in_w;
	UINT16              src_in_h;
    UINT16              src_in_x;
	UINT16              src_in_y;
	UINT32 				src_y_ofsi; //bg line offset	
	UINT32 				src_uv_ofsi; //bg line offset	
	//UINT32 				src_y_side_info_ofsi; //bg line offset	
	//UINT32 				src_uv_side_info_ofsi; //bg line offset	

	
}GLB_IMG_INFO;

typedef struct{
    VPE_ENG_DES_FMT     des_type;
}OUT_DUP_INFO;

typedef struct{
    UINT8               out_dup_num;
    OUT_DUP_INFO        out_dup_info[3];
    RES_DIM_INFO        res_dim_info;
    RES_SCA_INFO        res_sca_info;
    UINT8               out_bg_sel;
    UINT8               res_des_drt;	
    UINT8               out_seg_op; //0: disable, 1: left/right, 2: up/down
}OUT_IMG_INFO;




typedef enum {
	DCE_DEFORM_NONE = 0,
	DCE_DEFORM_MF,
	DCE_DEFORM_ROT,
	DCE_DEFORM_TYPE_MAX 
}DCE_DEFORM_TYPE;


typedef struct _DCE_PARAM_ {
	UINT16  dewarp_in_width;
	UINT16  dewarp_in_height;
	UINT16  dewarp_out_width;
	UINT16  dewarp_out_height;
	UINT32  lens_radius;
	UINT32  lut2d_width;
	UINT32  lut2d_mode;//1:ptz_mode
	VPE_ENG_DC_MODE dc_mode;
	VPE_ENG_DC_ROT  dc_rot_sel;
	UINT8  pxlmsk_op;
	UINT16 pxlmsk_pos;
	UINT16 pxlmsk_pos1;
	UINT16 pxlmsk_en[VPE_ENG_SEG_MAX];
	UINT32 pxlmsk_ofso[VPE_ENG_SEG_MAX];	
} DCE_PARAM;

typedef struct _MD_PARAM_ {
	UINT16  md_in_x;
	UINT16  md_in_y;
	UINT8  md_win_size;
} MD_PARAM;

typedef enum _VPE_STPNUM_BASE {
	VPE_STPNUM_MUL_BASE_1=0,
	VPE_STPNUM_MUL_BASE_2,
	VPE_STPNUM_MUL_BASE_3,
     ENUM_DUMMY4WORD(VPE_STPNUM_BASE)
} VPE_STPNUM_BASE;

typedef enum {
	SEG_OP_OFF = 0,
	SEG_OP_H,
	SEG_OP_V,
		
	ENUM_DUMMY4WORD(SEG_OPERATION)
}SEG_OPERATION;

typedef struct{
    GLB_IMG_INFO        glb_img_info;
    OUT_IMG_INFO        out_img_info[VPE_ENG_RES_MAX];
    DCE_PARAM           dce_param;
    //MD_PARAM            md_param;
    UINT8               src_path_sel;//0:dec off, 1:dce on
    VPE_STPNUM_BASE vpe_stpnum_base;
    //UINT8               ne_blk_sample_step;
    //UINT8               tmnr_refr_dec_en;
}VPE_CONFIG;

typedef struct{
    UINT16 sca_width;
    UINT32 sca_hstep_oft;
    UINT16 out_x_start;
    UINT16 out_width;
    UINT16 rlt_x_start;
    UINT16 rlt_width;
    UINT16 pip_x_start;
    UINT16 pip_width;
    UINT16 out_x1_start;
    UINT16 out_x2_start;	
    UINT16 out_seg_pos;	
    UINT16 out_seg1_pos;
	UINT8  out_seg_en[VPE_ENG_SEG_MAX];
    UINT16 sca_crop_x_start;
    UINT16 sca_crop_width;
    UINT16 tc_crop_x_start;
    UINT16 tc_crop_width;
    UINT16 tc_crop_skip;
}RES_COL_SIZE_CFG;

typedef struct{
    UINT8 path_en;
    //UINT8 sca_bypass_en;//no use for 690
    UINT8 sca_crop_en;
    UINT8 tc_en;
    //UINT8 des_yuv420;
    UINT8 dst_fmt;
    UINT8 out_bg_sel;
    //UINT8 des_dp_scee_en[3];
    //VPE_ENG_SCA_METHOD sca_method;
}RES_CTL_CFG;

typedef struct{
    UINT16 sca_height;
    UINT16 des_width;
	UINT16 des_y_dram_ofso;
	UINT16 des_uv_dram_ofso;
	UINT16 des_y1_dram_ofso;
	UINT16 des_uv1_dram_ofso;	
	UINT16 des_y2_dram_ofso;
	UINT16 des_uv2_dram_ofso;	
    UINT16 des_height;
    UINT16 out_y_start;
    UINT16 out_y1_start;
    UINT16 out_y2_start;
    UINT16 out_height;
    UINT16 rlt_y_start;
    UINT16 rlt_height;
    UINT16 pip_y_start;
    UINT16 pip_height;
}RES_SIZE_CFG;

typedef struct{

    UINT16 src_width;
    UINT16 src_height;
	UINT32 src_y_dram_ofsi;	
	UINT32 src_uv_dram_ofsi;	
    UINT16 proc_height;
    UINT16 presca_merge_width;
    UINT16 proc_y_start;
	UINT16 dce_width;
	UINT16 dce_height;
	UINT32 src_y_info_dram_ofsi; //for llc_8x4 y side info ofsi	
	UINT32 src_uv_info_dram_ofsi;//for llc8x4 uv siee info ofsi	

}GLO_SZ_CFG;

typedef struct{
    UINT16 sca_crop_y_start;
    UINT16 sca_crop_height;
}RES_SCA_CROP_CFG;

typedef struct{
	/*
	UINT16 sca_hstep;//no use for 690
	UINT16 sca_vstep;//no use for 690
    UINT16 sca_hsca_divisor;//no use for 690
    UINT16 sca_havg_pxl_msk;//no use for 690
    UINT16 sca_vsca_divisor;//no use for 690
    UINT16 sca_vavg_pxl_msk;//no use for 690
	*/
    UINT16 sca_factor_h;
    UINT16 sca_factor_v;
	UINT8 sca_drate_h;
	UINT8 sca_drate_v;
	UINT32 isd_norm_h;  //for isd method only
	UINT32 isd_norm_v; //for isd method only
	UINT16 coeffHorizontal[4];
	UINT16 coeffVertical[4];
	UINT8 sca_luma_wet;
	UINT8 sca_chroma_wet;
	UINT8 sca_chroma_hlpf;
	UINT8 sca_chroma_vlpf;
}RES_SCA_CFG;

typedef struct{
    UINT16 tc_crop_y_start;
    UINT16 tc_crop_height;
}RES_TC_CFG;

typedef struct{
    UINT8 min_y;
    UINT8 max_y;
    UINT8 min_uv;
    UINT8 max_uv;
}RES_CLAMP_CFG;

typedef struct{
    UINT8 col_num;
    //UINT8 md_win_y_num;
}GLO_CTL_CFG;

typedef struct{
    UINT16 proc_width;
    UINT16 proc_x_start;
    //UINT16 refr_x_start;
    //UINT16 refw_x_start;
    //UINT16 refw_crop_x_start;
    //UINT16 refw_crop_width;
	//UINT16 refw_crop_skip;
    //UINT16 md_win_x;
    //UINT16 md_win_x_num;
    //UINT16 md_win_x_start_num;
    //UINT16 tmnr_info_xnum;
    //UINT16 tmnr_info_xoft;
    UINT16 col_x_start;
    UINT16 lut2d_col_st_int;
    UINT32 lut2d_col_st_frac;
	UINT16 pxlmsk_seg_pos; 
	UINT16 pxlmsk_seg1_pos;
	UINT8  pxlmsk_seg_en[VPE_ENG_SEG_MAX];
    UINT16 pxlmsk_x_start;	
    UINT16 pxlmsk_x1_start;
    UINT16 pxlmsk_x2_start;	
    UINT16 pxlmsk_y_start;	
    UINT16 pxlmsk_y1_start;
    UINT16 pxlmsk_y2_start;
	UINT32 pxlmsk_dram_ofso[VPE_ENG_SEG_MAX];	
    //UINT16 bch_pfet_xlen;
}COL_SZ_CFG;

typedef struct{
    RES_CTL_CFG         res_ctl;
    RES_SIZE_CFG        res_size;
    RES_COL_SIZE_CFG    res_col_size[VPE_ENG_COL_MAX];
    RES_SCA_CROP_CFG    res_sca_crop;
    RES_SCA_CFG         res_sca;
    RES_TC_CFG          res_tc;
	RES_CLAMP_CFG       clamp; 
}RES_CFG;


typedef struct{
    GLO_CTL_CFG         glo_ctl;
    GLO_SZ_CFG          glo_sz;
    COL_SZ_CFG          col_sz[VPE_ENG_COL_MAX];
    RES_CFG             res[4];
	VPE_ENG_CACHE_CFG   cache_cfg;
}VPE_DRV_CFG;


typedef struct{
	UINT8 coef[ISD_USER_COEF_NUMS];
} VPE_ENG_ISD_CFG;


extern ER vpe_eng_set_res_x_sca_crop_vsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 sca_crop_y, UINT32 sca_crop_h);
extern void vpe_eng_set_col_x_proc_hsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 proc_x, UINT32 proc_w);
extern void vpe_eng_set_col_x_hsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 col_x);
extern ER vpe_eng_set_res_x_dst_ctrl_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, VPE_RES_CTL_INFO *param);
extern ER vpe_eng_set_res_x_sca_height_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 sca_h);
extern ER vpe_eng_set_res_x_dst_size_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 dst_w, UINT32 dst_h);
extern ER vpe_eng_set_res_x_dst_lineoft_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 seg_id, VPE_ENG_DATA_CH data_ch, UINT32 dst_lineoft);
extern ER vpe_eng_set_res_x_out_vsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 out_h);
extern ER vpe_eng_set_res_x_rlt_vsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 rlt_y, UINT32 rlt_h);
extern ER vpe_eng_set_res_x_pip_vsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 pip_y, UINT32 pip_h);
extern ER vpe_eng_set_res_x_col_x_sca_hsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 sca_w, UINT32 hstep_oft);
extern ER vpe_eng_set_res_x_col_x_out_start_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 seg_id, UINT32 out_x);
extern ER vpe_eng_set_res_x_col_x_out_hsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 out_w);
extern ER vpe_eng_set_res_x_col_x_seg_pos_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 seg_pos_id, UINT32 seg_pos);

extern ER vpe_eng_set_res_x_col_x_rlt_hsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 rlt_x, UINT32 rlt_w);
extern ER vpe_eng_set_res_x_col_x_pip_hsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 pip_x, UINT32 pip_w);
extern ER vpe_eng_set_res_x_col_x_sca_crop_hsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 sca_crop_x, UINT32 sca_crop_w);
extern ER vpe_eng_set_res_x_col_x_tc_crop_hsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 tc_crop_x, UINT32 tc_crop_w, UINT32 tc_crop_skip);
extern ER vpe_eng_set_res_x_tc_crop_vsize_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 tc_crop_y, UINT32 tc_crop_h);
extern ER vpe_eng_set_res_x_sca_factor_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 factor_h, UINT32 drate_h, UINT32 factor_v, UINT32 drate_v);
extern ER vpe_eng_set_res_x_sca_norm_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 norm_h, UINT32 norm_v);
extern INT32 vpe_eng_column_chk(VPE_CONFIG *p_vpe_info);

extern int vpe_eng_column_sca_config(VPE_ENG_HANDLE *p_eng, VPE_CONFIG *vpe321_info, VPE_DRV_CFG *drv_cfg);
extern ER vpe_eng_set_res_x_sca_wet_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 luma_wet, UINT32 chroma_wet, UINT32 chroma_hlpf, UINT32 chroma_vlpf);
extern ER vpe_eng_set_res_x_sca_lpf_hcoef_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT16 *h_p);
extern ER vpe_eng_set_res_x_sca_lpf_vcoef_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT16 *v_p);
extern ER vpe_eng_set_res_x_coef_mode_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 coef_mode);
extern ER vpe_eng_set_res_x_filt_adj_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, UINT32 filt_adj_h, UINT32 filt_adj_v);
extern void vpe_eng_set_isd_user_coef_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_ISD_CFG *isd_cfg);
extern ER vpe_eng_set_res_x_clamp_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id, RES_CLAMP_CFG clamp);
extern ER vpe_eng_set_res_x_seg_out_ystart_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id,  UINT32 seg_id, UINT32 out_y);
extern ER vpe_eng_set_res_x_seg_out_en_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 res_id,  UINT32 seg_id, UINT8 seg_out_en);

#ifdef __cplusplus
}
#endif


#endif //
