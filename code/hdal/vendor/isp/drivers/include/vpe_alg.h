#ifndef _VPE_ALG_H_
#define _VPE_ALG_H_

#if defined(__KERNEL__) || defined(__FREERTOS)
#include "kwrap/type.h"
#endif

//=============================================================================
// struct & definition
//=============================================================================
#define VPE_2DLUT_NUM            260 * 257
#define VPE_GEO_LUT_NUMS         65

#define VPE_DRT_PATH_NUM         4
#define VPE_SHARPEN_DBS_GAIN_NUM 17

/**
	VPE process id
*/
typedef enum _VPE_ID {
	VPE_ID_1 = 0,                       ///< vpe id 1
	VPE_ID_2,                           ///< vpe id 2
	VPE_ID_3,                           ///< vpe id 3
	VPE_ID_4,                           ///< vpe id 4
	VPE_ID_5,                           ///< vpe id 5
	VPE_ID_6,                           ///< vpe id 6
	VPE_ID_7,                           ///< vpe id 7
	VPE_ID_8,                           ///< vpe id 8
	VPE_ID_9,                           ///< vpe id 9
	VPE_ID_10,                          ///< vpe id 10
	VPE_ID_11,                          ///< vpe id 11
	VPE_ID_12,                          ///< vpe id 12
	VPE_ID_13,                          ///< vpe id 13
	VPE_ID_14,                          ///< vpe id 14
	VPE_ID_15,                          ///< vpe id 15
	VPE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(VPE_ID)
} VPE_ID;

typedef enum _VPE_ISP_DCE_MODE {
	VPE_ISP_DCE_MODE_2DLUT_ONLY = 1,
	VPE_ISP_DCE_MODE_2DLUT_DCTG,
	VPE_ISP_DCE_MODE_MAX,
	ENUM_DUMMY4WORD(VPE_ISP_DCE_MODE)
} VPE_ISP_DCE_MODE;

typedef enum _VPE_ISP_2DLUT_SZ {
	VPE_ISP_2DLUT_SZ_2X2 =     2,
	VPE_ISP_2DLUT_SZ_9X9 =     9,
	VPE_ISP_2DLUT_SZ_65X65 =   65,
	VPE_ISP_2DLUT_SZ_129X129 = 129,
	VPE_ISP_2DLUT_SZ_257X257 = 257,
	VPE_ISP_2DLUT_SZ_MAX_NUM,
	ENUM_DUMMY4WORD(VPE_ISP_2DLUT_SZ)
} VPE_ISP_2DLUT_SZ;

typedef enum _VPE_YUV_CVT {
	VPE_YUV_CVT_NONE = 0,
	VPE_YUV_CVT_PC,
	VPE_YUV_CVT_TV,
	VPE_YUV_CVT_MAX,
	ENUM_DUMMY4WORD(VPE_YUV_CVT)
} VPE_YUV_CVT;

typedef enum _VPE_DRT_SEL {
	VPE_DRT_NONE = 0,
	VPE_DRT_PC2TV,
	VPE_DRT_TV2PC,
	VPE_DRT_MAX,
	ENUM_DUMMY4WORD(VPE_DRT_SEL)
} VPE_DRT_SEL;

typedef enum _VPE_ISP_DCTG_MODE {
	VPE_ISP_DCTG_MODE_DISABLE = 0,
	VPE_ISP_DCTG_MODE_DCTG_ONLY,
	VPE_ISP_DCTG_MODE_PTZ,
	VPE_ISP_DCTG_MODE_MAX,
	ENUM_DUMMY4WORD(VPE_ISP_DCTG_MODE)
} VPE_ISP_DCTG_MODE;

typedef enum _VPE_ISP_DCTG_MOUNT_TYPE {
	VPE_ISP_DCTG_MOUNT_CEILING = 0,
	VPE_ISP_DCTG_MOUNT_FLOOR,
	VPE_ISP_DCTG_MOUNT_WALL,
	ENUM_DUMMY4WORD(VPE_ISP_DCTG_MOUNT_TYPE)
} VPE_ISP_DCTG_MOUNT_TYPE;

typedef enum _VPE_ISP_DCTG_2DLUT_SZ {
	VPE_ISP_DCTG_2DLUT_SZ_9X9 =   0,
	VPE_ISP_DCTG_2DLUT_SZ_65X65 = 3,
	VPE_ISP_DCTG_2DLUT_SZ_MAX_NUM,
	ENUM_DUMMY4WORD(VPE_ISP_DCTG_2DLUT_SZ)
} VPE_ISP_DCTG_2DLUT_SZ;

typedef enum _VPE_ISP_PTZ_PROJ_TYPE {
	VPE_ISP_PTZ_PROJ_EQUIRECTANGULAR = 0,
	VPE_ISP_PTZ_PROJ_CYLINDRICAL,
	VPE_ISP_PTZ_PROJ_MAX,
	ENUM_DUMMY4WORD(VPE_ISP_PTZ_PROJ_TYPE)
} VPE_ISP_PTZ_PROJ_TYPE;

typedef enum _VPE_ISP_FLIP_TYPE {
	VPE_ISP_FLIP_NONE,
	VPE_ISP_FLIP_H,
	VPE_ISP_FLIP_V,
	VPE_ISP_FLIP_HV,
	VPE_ISP_FLIP_MAX,
	ENUM_DUMMY4WORD(VPE_ISP_FLIP_TYPE)
} VPE_ISP_FLIP_TYPE;

typedef enum _VPE_ISP_MANUAL_ROT_RATIO {
	VPE_ISP_ROT_RAT_FIX_ASPECT_RATIO_UP,
	VPE_ISP_ROT_RAT_FIX_ASPECT_RATIO_DOWN,
	VPE_ISP_ROT_RAT_FIT_ROI,
	VPE_ISP_ROT_RAT_NORMAL,
	VPE_ISP_ROT_RAT_MAX,
	ENUM_DUMMY4WORD(VPE_ISP_MANUAL_ROT_RATIO)
} VPE_ISP_MANUAL_ROT_RATIO;

typedef enum _VPE_ISP_FLIP_ROT_MODE {
	VPE_ISP_ROTATE_0 = 0,       //rotate 0 degree
	VPE_ISP_ROTATE_90,          //rotate 90 degree
	VPE_ISP_ROTATE_180,         //rotate 180 degree
	VPE_ISP_ROTATE_270,         //rotate 270 degree
	VPE_ISP_H_FLIP_ROTATE_0,    //horizontal flip + rotate 0 degree
	VPE_ISP_H_FLIP_ROTATE_90,   //horizontal flip + rotate 90 degree
	VPE_ISP_H_FLIP_ROTATE_180,  //horizontal flip + rotate 180 degree
	VPE_ISP_H_FLIP_ROTATE_270,  //horizontal flip + rotate 270 degree
	VPE_ISP_ROTATE_MANUAL,      //manual mode, need set CTL_VPE_ISP_MANUAL_ROT_PARAM
	VPE_ISP_FLIP_ROTATE_MAX,
	ENUM_DUMMY4WORD(VPE_ISP_FLIP_ROT_MODE)
} VPE_ISP_FLIP_ROT_MODE;

typedef enum _VPE_DCOUT_MODE {
	VPE_DCOUT_AUTO = 0,
	VPE_DCOUT_OUTPUT,
	VPE_DCOUT_INPUT,
	ENUM_DUMMY4WORD(VPE_DCOUT_MODE)
} VPE_DCOUT_MODE;

typedef struct _VPE_DCE_CTL_PARAM {
	UINT8 enable;
	UINT8 lsb_rand;
	VPE_ISP_DCE_MODE dce_mode;
	UINT8 fovbound;
	UINT16 boundy;
	UINT16 boundu;
	UINT16 boundv;
	UINT16 geo_lut[VPE_GEO_LUT_NUMS];
} VPE_DCE_CTL_PARAM;

typedef struct _VPE_SHARPEN_PARAM {
	UINT8 enable;
	UINT8 edge_weight_gain;
	UINT8 edge_sharp_str1;
	UINT8 edge_sharp_str2;
	UINT8 flat_sharp_str;
	UINT8 dbs_gain_en;
	UINT8 dbs_gain[VPE_SHARPEN_DBS_GAIN_NUM];
	UINT32 quad_area_clamping; 
} VPE_SHARPEN_PARAM;

typedef struct _VPE_2DLUT_EXPAND_PARAM {
	UINT8 xofs_i;
	UINT32 xofs_f;
	UINT8 yofs_i;
	UINT32 yofs_f;
	UINT16 lut2d_width;
	UINT16 lut2d_height;
	UINT16 lut2d_lofs;
	ULONG lut_addr_pa;
	UINT8 lut2d_precision;
} VPE_2DLUT_EXPAND_PARAM;

typedef struct _VPE_2DLUT_PARAM {
	VPE_ISP_2DLUT_SZ lut_sz;
	UINT32 lut[VPE_2DLUT_NUM];
	UINT8 lut2d_precision;
} VPE_2DLUT_PARAM;

typedef struct _VPE_DRT_PARAM {
	VPE_YUV_CVT cvt_sel[VPE_DRT_PATH_NUM];
	UINT16 pc2tv_weight[VPE_DRT_PATH_NUM]; // 539A only
	VPE_DRT_SEL src_drt_sel;               //src drt select, 539A only
	UINT16 src_pc2tv_weight;               //src pc2tv weighting, 539A only
} VPE_DRT_PARAM;

typedef struct _VPE_ISP_DCTG_PARAM {
	VPE_ISP_DCTG_MOUNT_TYPE mount_type;
	UINT16 lut2d_width;
	UINT16 lut2d_height;
	UINT16 lens_r;
	UINT16 lens_cent_x;
	UINT16 lens_cent_y;
	UINT32 long_aov;
	UINT32 lati_aov;
	INT32 pan_angle;
	INT32 tilt_angle;
	INT32 rot_angle;
	INT32 adj_v_angle;
	UINT8 distor_lut_en;
	UINT32 max_diag_dist;
	UINT8 adj_fov_aspect_ratio_en;
} VPE_ISP_DCTG_PARAM;

typedef struct _VPE_ISP_PTZ_PARAM {
	VPE_ISP_PTZ_PROJ_TYPE proj_type;
	UINT32 long_aov;
	UINT32 lati_aov;
	INT32 pan_angle;
	INT32 tilt_angle;
	INT32 rot_angle;
	UINT16 zoom_step;
	UINT32 cam_long_aov;
	UINT32 cam_lati_aov;
	UINT16 cam_w;
	UINT16 cam_h;
	INT32 stitch_overlap_angle;
} VPE_ISP_PTZ_PARAM;

typedef struct _VPE_DCTG_CTRL {
	VPE_ISP_DCTG_MODE mode_sel;
	VPE_ISP_DCTG_PARAM dctg_param;
	VPE_ISP_PTZ_PARAM ptz_param;
} VPE_DCTG_CTRL;

typedef struct _VPE_MANUAL_ROT_PARAM {
	UINT32 rot_degree;      //rot degree, 1 degree = 10, range:0~360 degree, val: 0 ~ 3600
	VPE_ISP_FLIP_TYPE flip;
	VPE_ISP_MANUAL_ROT_RATIO ratio_mode;
	UINT8  fovbound;
	UINT16 boundy;
	UINT16 boundu;
	UINT16 boundv;
} VPE_MANUAL_ROT_PARAM;

typedef struct _VPE_FLIP_ROT_PARAM {
	VPE_ISP_FLIP_ROT_MODE flip_rot_mode;
	VPE_MANUAL_ROT_PARAM rot_manual_param;   //valid only when flip_rot_mode = CTL_VPE_ISP_ROTATE_MANUAL
} VPE_FLIP_ROT_PARAM;

#endif
