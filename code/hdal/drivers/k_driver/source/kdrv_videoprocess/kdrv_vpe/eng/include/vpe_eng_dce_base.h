#ifndef __VPE_ENG_DCE_BASE_H__
#define __VPE_ENG_DCE_BASE_H__

#include "kwrap/type.h"
#include "vpe_eng_handle.h"

#define KDRV_VPE_GEO_LUT_NUMS         65
#define KDRV_VPE_DBS_GAIN_NUMS        17
#define KDRV_VPE_ZOOM_LUT_NUMS        17
#define VPE_2DLUT_FRAC_BIT_NUM        24
#define VPE_PI        31415926
#define VPE_PI_PRECISION_BIT        7 //
#define DCE_2DLUT_DCTG_LUT_OFSI_MAX	    (1024)
#define DCE_PTZ_LUT_OFSI_MAX		    (513)


typedef struct _VPE_ENG_DCE_PARAM_ {
	UINT8   dce_mode;
	//UINT8   lut2d_sz;
	UINT16  lut2d_width;
	UINT16  lut2d_height;
	UINT16  lut2d_width_ofsi;
	UINT8   lsb_rand;
	UINT8   fovbound;
	UINT16  boundy;
	UINT16  boundu;
	UINT16  boundv;
	//UINT16  cent_x_s;
	//UINT16  cent_y_s;
	//UINT16  xdist_a1;
	//UINT16  ydist_a1;
	//UINT16  normfact;
	//UINT8   normbit;
	//UINT16  fovgain;
	//UINT32  hfact;
	//UINT32  vfact;
	UINT8   xofs_i;
	UINT32  xofs_f;
	UINT8   yofs_i;
	UINT32  yofs_f;
	//UINT16  geo_lut[KDRV_VPE_GEO_LUT_NUMS];
	//UINT8   dce_2d_lut_en;
	//UINT32  dce_l2d_addr;
	UINT16  dewarp_in_width;
	UINT16  dewarp_in_height;
	UINT16  dewarp_out_width;
	UINT16  dewarp_out_height;
	//UINT32  lens_radius;
	UINT16  lut2d_mode;//0:2dlut only or dctg, 1:ptz_mode
} VPE_ENG_DCE_PARAM;

typedef struct _DCE_NORM_PARAM_ {
	UINT16  cent_x_s;
	UINT16  cent_y_s;
	UINT16  xdist_a1;
	UINT16  ydist_a1;
	UINT16  normfact;
	UINT8   normbit;
	UINT16  dewarp_in_width;
	UINT16  dewarp_in_height;
	UINT16  dewarp_out_width;
	UINT16  dewarp_out_height;
} DCE_NORM_PARAM;


typedef struct _VPE_ENG_DCE_GEO_CFG_ {
	UINT16 geo_lut[KDRV_VPE_GEO_LUT_NUMS];
} VPE_ENG_DCE_GEO_CFG;

typedef struct _VPE_ENG_DCE_DBS_CFG_ {
	UINT8 dbs_gain[KDRV_VPE_DBS_GAIN_NUMS];
} VPE_ENG_DCE_DBS_CFG;
typedef struct _VPE_ENG_DCTG_PARAM_ {
	UINT8  mount_type;
	UINT16 lens_r;
	UINT16 lens_x_st;
	UINT16 lens_y_st;
    INT32 long_aov;
    INT32 lati_aov;
    INT32 pan;
    INT32 tilt;
    INT32 roll;
	UINT8 auto_corr_v_perspect;
    INT32 v_perspect;
	UINT8 fov_correct_en;
} VPE_ENG_DCTG_PARAM;


typedef struct _VPE_ENG_DCTG_DISTOR_CFG_ {
    UINT8 lut_max_inci_angle_deg;
	UINT16 max_diag_distance;
} VPE_ENG_DCTG_DISTOR_CFG;


typedef struct _VPE_ENG_PTZ_PARAM_{
	//dce
	UINT8 proj_type;
	INT32 stitch_shift_overlap_angle;
	UINT32 single_cam_long_aov;
	UINT32 single_cam_lati_aov;
	UINT16 single_cam_img_width;
	UINT16 single_cam_img_height;
	
	//dctg
	UINT32 ptz_long_aov;
	UINT32 ptz_lati_aov;
	INT32 pan_angle;
	INT32 tilt_angle;
	INT32 rotate_angle;
	UINT16 zoom_in_step;
	//UINT8 max_zoom_rate_order;
	UINT16 zoom_rate_lut[KDRV_VPE_ZOOM_LUT_NUMS];

} VPE_ENG_PTZ_PARAM;

typedef enum _VPE_ENG_DC_MODE {
	MODE_Reserved0  = 0,
	MODE_2DLUT_ONLY,
	ENUM_DUMMY4WORD(VPE_ENG_DC_MODE)
} VPE_ENG_DC_MODE;

extern ER vpe_eng_set_dce_param_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DCE_PARAM *p_dce_param);
extern void vpe_eng_set_dctg_param_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DCTG_PARAM *p_dctg_param);
extern void vpe_eng_set_dctg_distor_param_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DCTG_DISTOR_CFG *p_dctg_distor_cfg);

extern void vpe_eng_set_dce_geo_lut_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DCE_GEO_CFG *geo_cfg);
extern ER vpe_eng_set_ptz_param_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_PTZ_PARAM *p_ptz_param);
extern void vpe_eng_set_dce_dbs_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DCE_DBS_CFG *dbs_cfg);
extern void vpe_eng_set_dce_quad_clamp_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 quad_clamp);
extern UINT64 vpe_eng_do_64b_div(UINT64 dividend, UINT64 divisor);

#endif
