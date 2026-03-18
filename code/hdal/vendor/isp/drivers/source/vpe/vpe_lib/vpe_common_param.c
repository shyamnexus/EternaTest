#include "ctl_vpe_isp.h"

CTL_VPE_ISP_DCE_CTL ctl_vpe_dce_ctl_init = {
	.enable = FALSE,
	.lsb_rand = 0,
	.dce_mode = CTL_VPE_ISP_DCE_MODE_2DLUT_ONLY,
	.lens_radius = 0,
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

CTL_VPE_ISP_SHARPEN_PARAM ctl_vpe_sharpen_init = {
	.enable = FALSE,
	.edge_weight_src_sel = FALSE,
	.edge_weight_th = 2,
	.edge_weight_gain = 175,
	.noise_level = 15,
	.noise_curve = {0, 38, 46, 51, 54, 57, 59, 61, 62, 62, 62, 62, 62, 62, 62, 62, 62},
	.blend_inv_gamma = 128,
	.edge_sharp_str1 = 40,
	.edge_sharp_str2 = 40,
	.flat_sharp_str = 20,
	.coring_th = 0,
	.bright_halo_clip = 128,
	.dark_halo_clip = 128,
	.sharpen_out_sel = 0,
	.dbs_gain_en = FALSE,
	.dbs_gain = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	.quad_area_clamping = 65536
};

CTL_VPE_ISP_DCE_2DLUT_PARAM ctl_vpe_2dlut_init = {
	.xofs_i = 0,
	.xofs_f = 0,
	.yofs_i = 0,
	.yofs_f = 0,
	.lut2d_width = 0,
	.lut2d_height = 0,
	.lut2d_lofs = 0,
	.lut_addr_pa = 0,
	.out_size = {
		.w = 0,
		.h = 0
	},
	.lut2d_precision = 0
};

CTL_VPE_ISP_YUV_CVT_PARAM ctl_vpe_cvt_init = {
	.cvt_sel = {0},
	.pc2tv_weight = {0},
	.src_drt_sel = CTL_VPE_ISP_DRT_NONE,
	.src_pc2tv_weight = 0
};

CTL_VPE_ISP_DCTG_CTL ctl_vpe_dctg_init = {
	.mode_sel = CTL_VPE_ISP_DCTG_MODE_DISABLE,
	.dctg_param = {
		.mount_type = CTL_VPE_ISP_DCTG_MOUNT_WALL,
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
		.adj_v_perspect = 1,
		.adj_v_angle = 0,
		.distor_lut_en = 0,
		.max_diag_dist = 1620,
		.max_inci_angle = 120,
		.adj_fov_aspect_ratio_en = 0
	},
	.ptz_param = {
		.proj_type = CTL_VPE_ISP_PTZ_PROJ_EQUIRECTANGULAR,
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
	},
	.out_size = {0, 0}
};

CTL_VPE_ISP_FLIP_ROT_CTL ctl_vpe_flip_rot_init = {
	.flip_rot_mode = CTL_VPE_ISP_ROTATE_0,
	.rot_manual_param = {
		.rot_degree = 0,
		.flip = CTL_VPE_ISP_FLIP_NONE,
		.ratio_mode = CTL_VPE_ISP_ROT_RAT_MANUAL,
		.ratio = 100,
		.fovbound = 1,
		.boundy = 128,
		.boundu = 0,
		.boundv = 0
	}
};

