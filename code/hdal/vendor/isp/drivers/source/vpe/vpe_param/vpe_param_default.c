#include "kwrap/type.h"

#include "vpe_alg_int.h"
#include "vpe_param_default.h"

static VPE_DCE_CTL_PARAM vpe_dce_ctl_param = {
	.enable = TRUE,
	.lsb_rand = 0,
	.dce_mode = VPE_ISP_DCE_MODE_2DLUT_ONLY,
	.fovbound = 1,
	.boundy = 0,
	.boundu = 128,
	.boundv = 128,
	.geo_lut = {
			65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
			65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
			65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
			65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
			65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
			65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
			65535, 65535, 65535, 65535, 65535}
};

static VPE_SHARPEN_PARAM vpe_sharpen_param = {
	.enable = FALSE,
	.edge_weight_gain = 175,
	.edge_sharp_str1 = 40,
	.edge_sharp_str2 = 40,
	.flat_sharp_str = 20,
	.dbs_gain_en = 0,
	.dbs_gain = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	.quad_area_clamping = 65536
};

static VPE_2DLUT_INT_PARAM vpe_2dlut_int_param = {
	.lut_sz = VPE_ISP_2DLUT_SZ_65X65,
	.lut_addr = 0,
	.lut2d_precision = 0
};

static VPE_2DLUT_EXPAND_PARAM vpe_2dlut_expand_param = {
	.xofs_i = 0,
	.xofs_f = 0,
	.yofs_i = 0,
	.yofs_f = 0,
	.lut2d_width = 0,
	.lut2d_height = 0,
	.lut2d_lofs = 0,
	.lut_addr_pa = 0,
	.lut2d_precision = 0
};

static VPE_DRT_PARAM vpe_drt_param = {
	.cvt_sel = {0},
	.pc2tv_weight = {0},
	.src_drt_sel = VPE_DRT_NONE,
	.src_pc2tv_weight = 0
};

static VPE_DCTG_CTRL vpe_dctg_ctrl = {
	.mode_sel = VPE_ISP_DCTG_MODE_DCTG_ONLY,
	.dctg_param = {
		.mount_type = VPE_ISP_DCTG_MOUNT_WALL,
		.lut2d_width = 65,
		.lut2d_height = 65,
		.lens_r = 1256,
		.lens_cent_x = 1296,
		.lens_cent_y = 972,
		.long_aov = 900,
		.lati_aov = 700,
		.pan_angle = 0,
		.tilt_angle = 0,
		.rot_angle = 0,
		.adj_v_angle = 0,
		.distor_lut_en = 0,
		.max_diag_dist = 1620,
		.adj_fov_aspect_ratio_en = 0
	},
	.ptz_param = {
		.proj_type = VPE_ISP_PTZ_PROJ_EQUIRECTANGULAR,
		.long_aov = 900,
		.lati_aov = 700,
		.pan_angle = 0,
		.tilt_angle = 0,
		.rot_angle = 0,
		.zoom_step = 0,
		.cam_long_aov = 0,
		.cam_lati_aov = 0,
		.cam_w = 1296,
		.cam_h = 972,
		.stitch_overlap_angle = 0
	}
};

static VPE_FLIP_ROT_PARAM vpe_flip_rot_param = {
	.flip_rot_mode = VPE_ISP_ROTATE_0,
	.rot_manual_param = {
		.rot_degree = 0,
		.flip = VPE_ISP_FLIP_NONE,
		.ratio_mode = VPE_ISP_ROT_RAT_NORMAL,
		.fovbound = 1,
		.boundy = 0,
		.boundu = 128,
		.boundv = 128
	}
};

static VPE_PARAM_PTR vpe_param_default = {
	&vpe_dce_ctl_param,
	&vpe_sharpen_param,
	&vpe_2dlut_int_param,
	&vpe_2dlut_expand_param,
	&vpe_drt_param,
	&vpe_dctg_ctrl,
	&vpe_flip_rot_param
};

ULONG vpe_get_param_default(void)
{
	return (ULONG)(&vpe_param_default);
}
