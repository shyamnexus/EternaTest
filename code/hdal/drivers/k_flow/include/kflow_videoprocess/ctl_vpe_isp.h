/**
    IPL Ctrl Layer, ISP Interface

    @file       ctl_vpe_isp.h
    @ingroup    mIVPE_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_VPE_ISP_H
#define _CTL_VPE_ISP_H

#include "kflow_common/isp_if.h"
#include "kflow_common/type_vdo.h"

#define CTL_VPE_ISP_NOISE_CURVE_NUMS		17
#define CTL_VPE_ISP_SHARPEN_DBS_GAIN_NUMS	17
#define CTL_VPE_ISP_GEO_LUT_NUMS			65
#define CTL_VPE_ISP_OUT_PATH_NUMS			(4)

#define CTL_VPE_ISP_DCTG_PTZ_PARAM		ENABLE

typedef enum {
	CTL_VPE_ISP_FLOW_VPE,		// vpe auto select VPE1/2/3 by func/fmt
	CTL_VPE_ISP_FLOW_VPE_PTZ,	// force use VPE3 + ptz function
	CTL_VPE_ISP_FLOW_MAX
} CTL_VPE_ISP_FLOW_TYPE;	//should the same with CTL_VPE_FLOW_TYPE

typedef enum {
	CTL_VPE_ISP_ITEM_IQ_PARAM = 0,					///< [SET], 	data_type: CTL_VPE_ISP_IQ_ALL
	CTL_VPE_ISP_ITEM_MAX,
} CTL_VPE_ISP_ITEM;

typedef struct {
	UINT8  enable;
	UINT8  edge_weight_src_sel; 					///< Select source of edge weight calculation, 0~1
	UINT8  edge_weight_th; 							///< Edge weight coring threshold, 0~255
	UINT8  edge_weight_gain; 						///< Edge weight gain, 0~255
	UINT8  noise_level; 							///< Noise Level, 0~255
	UINT8  noise_curve[CTL_VPE_ISP_NOISE_CURVE_NUMS]; ///< 17 control points of noise modulation curve, 0~255
	UINT8  blend_inv_gamma; 						///< Blending ratio of HPF results, 0~128
	UINT8  edge_sharp_str1; 						///< Sharpen strength1 of edge region, 0~255
	UINT8  edge_sharp_str2; 						///< Sharpen strength2 of edge region, 0~255
	UINT8  flat_sharp_str; 							///< Sharpen strength of flat region,0~255
	UINT8  coring_th; 								///< Coring threshold, 0~255
	UINT8  bright_halo_clip; 						///< Bright halo clip ratio, 0~255
	UINT8  dark_halo_clip; 							///< Dark halo clip ratio, 0~255
	UINT8  sharpen_out_sel;

	UINT8  dbs_gain_en;	   								// only valid at dce_en = 1
	UINT8  dbs_gain[CTL_VPE_ISP_SHARPEN_DBS_GAIN_NUMS]; // 17 control points of noise modulation curve, 0~255(u3.5)
	UINT32 quad_area_clamping; //17 bits (u1.16)
} CTL_VPE_ISP_SHARPEN_PARAM;

typedef enum {
	CTL_VPE_ISP_DCE_MODE_2DLUT_ONLY,	//2dlut tbl input from isp
	CTL_VPE_ISP_DCE_MODE_2DLUT_DCTG,	//2dlut tbl gen by vpe
	CTL_VPE_ISP_DCE_MODE_MAX,
} CTL_VPE_ISP_DCE_MODE;

typedef enum {
	CTL_VPE_ISP_FLIP_NONE,
	CTL_VPE_ISP_FLIP_H,
	CTL_VPE_ISP_FLIP_V,
	CTL_VPE_ISP_FLIP_HV,
	CTL_VPE_ISP_FLIP_MAX,
} CTL_VPE_ISP_FLIP_TYPE;

typedef enum {
	CTL_VPE_ISP_ROT_RAT_FIX_ASPECT_RATIO_UP,
	CTL_VPE_ISP_ROT_RAT_FIX_ASPECT_RATIO_DOWN,
	CTL_VPE_ISP_ROT_RAT_FIT_ROI,
	CTL_VPE_ISP_ROT_RAT_MANUAL,
	CTL_VPE_ISP_ROT_RAT_MAX
} CTL_VPE_ISP_MANUAL_ROT_RATIO;

typedef struct {
	UINT32 rot_degree; 		//rot degree, 1 degree = 10, range:0~360 degree, val: 0 ~ 3600
	CTL_VPE_ISP_FLIP_TYPE flip;
	CTL_VPE_ISP_MANUAL_ROT_RATIO ratio_mode;
	UINT16 ratio; 			//100:1x 1 ~ max(depend image size & rot deg)
	UINT8  fovbound;
	UINT16 boundy;
	UINT16 boundu;
	UINT16 boundv;
}  CTL_VPE_ISP_MANUAL_ROT_PARAM;

typedef enum {
	CTL_VPE_ISP_ROTATE_0 = 0,		//rotate 0 degree
	CTL_VPE_ISP_ROTATE_90,			//rotate 90 degree
	CTL_VPE_ISP_ROTATE_180,			//rotate 180 degree
	CTL_VPE_ISP_ROTATE_270,			//rotate 270 degree
	CTL_VPE_ISP_H_FLIP_ROTATE_0,    //horizontal flip + rotate 0 degree
	CTL_VPE_ISP_H_FLIP_ROTATE_90,   //horizontal flip + rotate 90 degree
	CTL_VPE_ISP_H_FLIP_ROTATE_180,  //horizontal flip + rotate 180 degree
	CTL_VPE_ISP_H_FLIP_ROTATE_270,  //horizontal flip + rotate 270 degree
	CTL_VPE_ISP_ROTATE_MANUAL,		//manual mode, need set CTL_VPE_ISP_MANUAL_ROT_PARAM
	CTL_VPE_ISP_FLIP_ROTATE_MAX,
} CTL_VPE_ISP_FLIP_ROT_MODE;

typedef struct {
	CTL_VPE_ISP_FLIP_ROT_MODE flip_rot_mode;
	CTL_VPE_ISP_MANUAL_ROT_PARAM rot_manual_param;	//valid only when flip_rot_mode = CTL_VPE_ISP_ROTATE_MANUAL
}  CTL_VPE_ISP_FLIP_ROT_CTL;

typedef struct {
	UINT8 	xofs_i;
	UINT32 	xofs_f;
	UINT8 	yofs_i;
	UINT32 	yofs_f;
    UINT16 lut2d_width; //2~1024
    UINT16 lut2d_height; //2~1024
    UINT16 lut2d_lofs; //align 4
	ULONG  lut_addr_pa;
	USIZE 	out_size;	//2dlut output size
	UINT8 lut2d_precision;		//0:abs(s14.2), 1:abs(s15.1), 2:abs(s16.0), 3:rela(s2.14), 4:rela(s3.13), 5:rela(s4.12), 6:rela(s5.11)
} CTL_VPE_ISP_DCE_2DLUT_PARAM;

typedef struct {
	UINT8 enable;
	UINT8 lsb_rand;
	CTL_VPE_ISP_DCE_MODE dce_mode;
	UINT32  lens_radius;
	UINT8 	fovbound;
	UINT16 	boundy;
	UINT16 	boundu;
	UINT16	boundv;
	UINT16 	geo_lut[CTL_VPE_ISP_GEO_LUT_NUMS];
} CTL_VPE_ISP_DCE_CTL;

typedef enum {
	CTL_VPE_ISP_YUV_CVT_NONE = 0,
	CTL_VPE_ISP_YUV_CVT_PC,
	CTL_VPE_ISP_YUV_CVT_TV,
	CTL_VPE_ISP_YUV_CVT_MAX,
} CTL_VPE_ISP_YUV_CVT;

typedef enum {
	CTL_VPE_ISP_DCTG_MODE_DISABLE = 0,
	CTL_VPE_ISP_DCTG_MODE_DCTG_ONLY,
	CTL_VPE_ISP_DCTG_MODE_PTZ,
	CTL_VPE_ISP_DCTG_MODE_MAX,
} CTL_VPE_ISP_DCTG_MODE;

typedef enum {
	CTL_VPE_ISP_DCTG_MOUNT_CEILING = 0,
	CTL_VPE_ISP_DCTG_MOUNT_FLOOR,
	CTL_VPE_ISP_DCTG_MOUNT_WALL,
} CTL_VPE_ISP_DCTG_MOUNT_TYPE;

typedef enum {
	CTL_VPE_ISP_DCTG_2DLUT_SZ_9X9 = 0,
	CTL_VPE_ISP_DCTG_2DLUT_SZ_65X65 = 3,
} CTL_VPE_ISP_DCTG_2DLUT_SZ;

typedef struct _CTL_VPE_ISP_DCTG_PARAM_ {
	CTL_VPE_ISP_DCTG_MOUNT_TYPE mount_type;	 // 0:ceiling 1:floor 2:wall
    UINT16 lut2d_width; 	//2~1024
    UINT16 lut2d_height; 	//2~1024
    UINT16 lens_r;
    UINT16 lens_cent_x;
    UINT16 lens_cent_y;
    UINT32 long_aov;		//1 degree = 10, range:0~180 degree, val: 0~1800
    UINT32 lati_aov;		//1 degree = 10, range:0~180 degree, val: 0~1800
    INT32 pan_angle;   		//1 degree = 10, range:-180~180 degree, val: -1800~1800
    INT32 tilt_angle;   	//1 degree = 10, range:-180~180 degree, val: -1800~1800
    INT32 rot_angle;   		//1 degree = 10, range:-180~180 degree, val: -1800~1800
    UINT8 adj_v_perspect;	//0:disable, 1:manual 2:auto
    INT32 adj_v_angle;		//1 degree = 10, range:-80~80 degree, val: -800~800 (only valid at adj_v_perspect = manual mode)

	UINT8 distor_lut_en;	// ref. gdc geo_lut table
	UINT32 max_diag_dist;
	UINT32 max_inci_angle;
	UINT8 adj_fov_aspect_ratio_en;	//538:not support, 539A:support
} CTL_VPE_ISP_DCTG_PARAM;

typedef enum {
	CTL_VPE_ISP_PTZ_PROJ_EQUIRECTANGULAR = 0,
	CTL_VPE_ISP_PTZ_PROJ_CYLINDRICAL,
	CTL_VPE_ISP_PTZ_PROJ_MAX
} CTL_VPE_ISP_PTZ_PROJ_TYPE;

/* ptz ctrl */
typedef struct {
	CTL_VPE_ISP_PTZ_PROJ_TYPE proj_type;
    UINT32 long_aov;		//1 degree = 10, range:0~180 degree, val: 0 ~ 1800
    UINT32 lati_aov;		//1 degree = 10, range:0~180 degree, val: 0 ~ 1800
    INT32 pan_angle;   		//1 degree = 10, range:-180~180 degree, val: -1800 ~ 1800
    INT32 tilt_angle;   	//1 degree = 10, range:-180~180 degree, val: -1800 ~ 1800
    INT32 rot_angle;   		//1 degree = 10, range:-180~180 degree, val: -1800 ~ 1800
    UINT16 zoom_step; 		//val: 0~1023, 128 segments each 2x, 1x:0, 2x:128, 4x:256
    UINT32 cam_long_aov;	//single camera long aov, 1 degree = 10, range:0~180 degree, val: 0 ~ 1800
    UINT32 cam_lati_aov;	//single camera lati aov, 1 degree = 10, range:0~180 degree, val: 0 ~ 1800
    UINT16 cam_w;			//single camera image width in a stitch image
	UINT16 cam_h;			//single camera image height in a stitch image
    INT32 stitch_overlap_angle;   //1 degree = 10, range:-180~180 degree, val: -1800 ~ 1800
} CTL_VPE_ISP_PTZ_PARAM;

typedef struct _CTL_VPE_ISP_DCTG_CTL_ {
	CTL_VPE_ISP_DCTG_MODE mode_sel;
	CTL_VPE_ISP_DCTG_PARAM dctg_param;
	CTL_VPE_ISP_PTZ_PARAM ptz_param;
	USIZE 	out_size;	//dctg output size
} CTL_VPE_ISP_DCTG_CTL;

typedef struct {
	UINT8 cgofs_en;
	UINT16 y_gain;		//1x = 64 max:7.99x val: 0 ~ 511
	INT16 y_ofs;		//signed s9bit -511 ~ 512
	UINT16 u_gain;		//1x = 64 max:7.99x val: 0 ~ 511
	INT16 u_ofs;		//signed s9bit -511 ~ 512
	UINT16 v_gain;		//1x = 64 max:7.99x val: 0 ~ 511
	INT16 v_ofs;		//signed s9bit -511 ~ 512
	UINT8 is_tv_range;  //if in drt = VPE_DRV_DRT_NONE, data range must be set(base on dram data range)
}CTL_VPE_ISP_COLOR_OFS_PARAM;

typedef enum {
	CTL_VPE_ISP_DRT_NONE = 0,
	CTL_VPE_ISP_DRT_PC2TV,
	CTL_VPE_ISP_DRT_TV2PC,
	CTL_VPE_ISP_DRT_MAX
} CTL_VPE_ISP_DRT_SEL;

typedef struct {
	CTL_VPE_ISP_YUV_CVT cvt_sel[CTL_VPE_ISP_OUT_PATH_NUMS];
	UINT16 pc2tv_weight[CTL_VPE_ISP_OUT_PATH_NUMS]; //out weightint, 538:not support, 539A:support
	CTL_VPE_ISP_DRT_SEL src_drt_sel;	//src drt select
	UINT16 src_pc2tv_weight;			//src pc2tv weighting
} CTL_VPE_ISP_YUV_CVT_PARAM;

typedef struct {
	UINT8 adj_en;			//adj on/off, off default 0x20
	UINT8 filt_weight;		//0~63 1x=32
} CTL_VPE_ISP_ISD2_PARAM;

typedef struct {
	CTL_VPE_ISP_SHARPEN_PARAM	*p_sharpen_param;
	CTL_VPE_ISP_DCE_CTL			*p_dce_ctl;
	CTL_VPE_ISP_DCE_2DLUT_PARAM	*p_dce_2dlut_param;
	CTL_VPE_ISP_YUV_CVT_PARAM   *p_yuv_cvt_param;
	CTL_VPE_ISP_DCTG_CTL 	    *p_dctg_ctl;
	CTL_VPE_ISP_COLOR_OFS_PARAM *p_color_ofs_param;
	CTL_VPE_ISP_ISD2_PARAM		*p_isd2_param;
	CTL_VPE_ISP_FLIP_ROT_CTL	*p_flip_rot_ctl;
} CTL_VPE_ISP_IQ_ALL;

typedef enum {
	CTL_VPE_ISP_CB_MSG_NONE = 0x00000000,
	CTL_VPE_ISP_CB_MAX_MAX,
} CTL_VPE_ISP_CB_MSG;

typedef struct {
	CTL_VPE_ISP_FLOW_TYPE isp_flow;
	UINT32 vsp_frm_idx;
	UINT32 vsp_frm_num;
	USIZE dce_in_size;
	USIZE out_size;
} CTL_VPE_ISP_CB_PARAM;

ER ctl_vpe_isp_evt_fp_reg(CHAR *name, ISP_EVENT_FP fp, ISP_EVENT evt, CTL_VPE_ISP_CB_MSG cb_msg);
ER ctl_vpe_isp_evt_fp_unreg(CHAR *name);
ER ctl_vpe_isp_set(ISP_ID id, CTL_VPE_ISP_FLOW_TYPE isp_flow, CTL_VPE_ISP_ITEM item, void *data);
ER ctl_vpe_isp_get(ISP_ID id, CTL_VPE_ISP_FLOW_TYPE isp_flow, CTL_VPE_ISP_ITEM item, void *data);

#endif