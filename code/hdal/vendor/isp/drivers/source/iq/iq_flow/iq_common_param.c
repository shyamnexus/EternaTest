#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "iq_ui.h"
#include "iq_alg_int.h"

CTL_SIE_OB_PARAM ctl_sie_ob_init = {
	.bypass_enable = TRUE,
	.ob_ofs = 0,
};

CTL_SIE_CA_PARAM ctl_sie_ca_init = {
	.enable = TRUE,
	.win_num = {
		.w = 32,
		.h = 32, },
	.th_enable = FALSE,
	.g_th_l = 0,
	.g_th_u = 4095,
	.r_th_l = 0,
	.r_th_u = 4095,
	.b_th_l = 0,
	.b_th_u = 4095,
	.p_th_l = 0,
	.p_th_u = 4095,
	.irsub_r_weight = 0,
	.irsub_g_weight = 0,
	.irsub_b_weight = 0,
	.ca_ob_ofs = 0,
	.ca_src = CTL_SIE_CA_SRC_BEFORE_OB,
};

CTL_SIE_LA_PARAM ctl_sie_la_init = {
	.enable = TRUE,
	.win_num = {
		.w = 32,
		.h = 32, },
	.la_src = CTL_SIE_LA_SRC_POST_CG,
	.la_rgb2y1mod = CTL_SIE_STCS_LA_RGB,
	.la_rgb2y2mod = CTL_SIE_STCS_LA_RGB,
	.cg_enable = TRUE,
	.r_gain = 128,
	.g_gain = 128,
	.b_gain = 128,
	.gamma_enable = TRUE,
	.gamma_tbl = {
		0, 73, 139, 198, 241, 273, 304, 333, 360, 386,
		411, 433, 455, 475, 493, 510, 525, 539, 553, 566,
		580, 594, 607, 621, 634, 647, 659, 672, 684, 697,
		709, 721, 733, 744, 756, 767, 777, 787, 797, 806,
		816, 826, 835, 845, 854, 863, 872, 882, 891, 900,
		908, 917, 926, 934, 943, 951, 959, 968, 976, 984,
		992, 1000, 1007, 1015, 1023},
	.hist_enable = TRUE,
	.histo_src = CTL_SIE_LA_HIST_SRC_PRE_GMA,
	.irsub_r_weight = 0,
	.irsub_g_weight = 0,
	.irsub_b_weight = 0,
	.la_ob_ofs = 240,
	.lath_enable = TRUE,
	.lathy1lower = 0,
	.lathy1upper = 255,
	.lathy2lower = 0,
	.lathy2upper = 255,
};

CTL_SIE_CGAIN ctl_sie_cgain_init = {
	.enable = TRUE,
	.sel_37_fmt = TRUE,
	.r_gain = 128,
	.gr_gain = 128,
	.gb_gain = 128,
	.b_gain = 128,
	.ir_gain =  128
};

CTL_SIE_COMPANDING ctl_sie_companding_init = {
	.enable = FALSE,
	.decomp_info = {
		.decomp_kpx = {
			0, 512, 768, 2560, 2560, 2560, 2560, 2560, 2560, 2560,
			2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560,
			2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560},
		.decomp_kpy = {
			0, 1024, 2048, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
			16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
			16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384},
		.decomp_gain = {
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		.decomp_sb = {
			4, 5, 6, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	},
	.comp_info = {
		.comp_fcurve_l = {
			0, 6712, 13424, 20135, 26847, 29730, 32613, 35495, 38378, 39982,
			41585, 43189, 44792, 45813, 46834, 47855, 48876, 49584, 50291, 50999,
			51706, 52225, 52745, 53264, 53783, 54181, 54579, 54976, 55374, 55689,
			56003, 56318, 56632, 56888, 57143, 57399, 57654, 57866, 58078, 58289,
			58501, 58680, 58859, 59038, 59217, 59371, 59524, 59678, 59831, 59965,
			60098, 60232, 60365, 60483, 60600, 60718, 60835, 60940, 61045, 61149,
			61254, 61349, 61443, 61538, 61632},
		.comp_fcurve_m = {
			61632, 61976, 62292, 62585, 62858, 63117, 63363, 63599, 63826, 64047,
			64264, 64478, 64689, 64900, 65110, 65322, 65535},
		.comp_fcurve_r = {
			65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
			65535, 65535, 65535, 65535, 65535, 65535},
		.comp_fcurve_ev_fmt = 0,
	},
	.comp_shift = 0,
};

CTL_SIE_DGAIN ctl_sie_dgain_init = {
	.enable = TRUE,
	.enable_stcs = TRUE,
	.gain = 256,
};

CTL_SIE_ISP_ROI_RATIO ctl_sie_roi_ratio_init = {
	.ratio_base = 1000,
	.ca_crop_win_roi = {
		.x = 0,
		.y = 0,
		.w = 1000,
		.h = 1000, },
	.la_crop_win_roi = {
		.x = 0,
		.y = 0,
		.w = 1000,
		.h = 1000, },
};

CTL_IFE_ISP_NRS ctl_ife_nrs_0_init = {
	.enable = TRUE,
	.str = {0, 0, 0, 0, 0, 0},
};

CTL_IFE_ISP_FCURVE ctl_ife_fcurve_init = {
	.enable = TRUE,
	.fcur_ctrl = {
		.y_mean_sel = CTL_IFE_ISP_F_CURVE_Y_8G4R4B,
		.yv_w = 8,
		.ev_fmt = 4, },
	.y_weight = {
		.y_w_lut = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, },
	.fcurve_l = {
		.fcur_l_lut = {
			1, 9886, 15473, 19385, 22398, 24850, 26550, 27967, 28912, 29772,
			30566, 31303, 31993, 32644, 33261, 33848, 34408, 34946, 35462, 35961,
			36442, 36909, 37362, 37802, 38231, 38649, 39057, 39456, 39846, 40229,
			40604, 40972, 41334, 41690, 42040, 42385, 42724, 43059, 43389, 43715,
			44036, 44354, 44668, 44978, 45285, 45588, 45889, 46186, 46481, 46772,
			47062, 47348, 47632, 47914, 48193, 48471, 48746, 49019, 49290, 49560,
			49828, 50093, 50357, 50619, 50880}, },
	.fcurve_r = {
		.fcur_r_lut = {
			50880, 51908, 52913, 53898, 54866, 55817, 56754, 57677, 58589, 59488,
			60378, 61258, 62129, 62992, 63846, 64694, 65535}, },
};

CTL_IFE_ISP_FUSION ctl_ife_fusion_init = {
	.fu_ctrl = {
		.y_mean_sel = CTL_IFE_ISP_Y_SEL_8G4R4B,
		.mode = CTL_IFE_ISP_MODE_FUSION,
		.ev_ratio = 256, },
	.bld_cur = {
		.nor_sel = CTL_IFE_ISP_FUSION_BY_S_EXP,
		.dif_sel = CTL_IFE_ISP_FUSION_BOTH_EXP,
		.l_nor_knee = {3072},
		.l_nor_range = 512,
		.l_nor_slope = 2048, // = (256*4096)/l_nor_range
		.s_nor_knee = {192},
		.s_nor_range = 64,
		.s_nor_slope = 16384,
		.l_dif_knee = {3072},
		.l_dif_range = 512,
		.l_dif_slope = 2048,
		.s_dif_knee = {192},
		.s_dif_range = 64,
		.s_dif_slope = 16384, },
	.mc_para = {
		.lum_th = 5,
		.diff_ratio = 0,
		.diff_w = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16},
		.dwd = 0, },
	.dk_sat = {
		.th = {4095, 4095},
		.step = {16, 16},
		.low_bound = {255, 255}, },
	.fu_cgain = {
		.enable = TRUE,
		.bit_field = CTL_IFE_ISP__8_8,
		.fcgain_s = {256, 256, 256, 256, 256},
		.fcofs_s = {240, 240, 240, 240, 240},
		.fcgain_l = {256, 256, 256, 256, 256},
		.fcofs_l = {240, 240, 240, 240, 240}, },
	.dbg = {
		.enable = FALSE,
		.mode = CTL_IFE_ISP_DISABLE_DBG_MODE, },
};

CTL_IFE_ISP_OUTL ctl_ife_outl_init = {
	.enable = TRUE,
	.bright_th = {4095, 4095, 4095, 4095, 4095},
	.dark_th = {4095, 4095, 4095, 4095, 4095},
	.outl_cnt = {1, 0},
	.outl_weight = 0,
	.dark_ofs = 0,
	.bright_ofs = 0,
	.outl_comp_mode = FALSE,
	.ord_rng_bri = 3,
	.ord_rng_dark = 3,
	.ord_protect_th = 256,
	.ord_blend_w = 0,
	.ord_bri_w = {0, 1, 2, 3, 4, 5, 6, 8},
	.ord_dark_w = {0, 1, 2, 3, 4, 5, 6, 8},
	.outl_rgbir_rb_w = 255,
	.ord_rgbir_rb_w = 255,
};

CTL_IFE_ISP_FILTER ctl_ife_filter_init = {
	.enable = TRUE,
	.spatial = {
		.weight = {31, 23, 17, 9, 7, 3}, },
	.rng_filt_r = {
		.a_th = {0, 0, 0, 0, 0, 0},
		.a_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.b_th = {0, 0, 0, 0, 0, 0},
		.b_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, },
	.rng_filt_gr = {
		.a_th = {0, 0, 0, 0, 0, 0},
		.a_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.b_th = {0, 0, 0, 0, 0, 0},
		.b_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, },
	.rng_filt_gb = {
		.a_th = {0, 0, 0, 0, 0, 0},
		.a_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.b_th = {0, 0, 0, 0, 0, 0},
		.b_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, },
	.rng_filt_b = {
		.a_th = {0, 0, 0, 0, 0, 0},
		.a_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.b_th = {0, 0, 0, 0, 0, 0},
		.b_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, },
	.rng_filt_ir = {
		.a_th = {0, 0, 0, 0, 0, 0},
		.a_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.b_th = {0, 0, 0, 0, 0, 0},
		.b_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, },
	.center_mod = {
		.enable = FALSE,
		.th1 = 0,
		.th2 = 0,
		.cen_sel = CTL_IFE_ISP_BILAT_CEN_DPC, },
	.clamp = {
		.th = 0,
		.mul = 128,
		.dlt = 0, },
	.rbfill = {
		.enable = TRUE,
		.luma = {0, 16, 24, 27, 27, 28, 30, 31, 31, 30, 29, 28, 28, 27, 26, 12, 4},
		.ratio = {31, 31, 31, 31, 31, 31, 30, 30, 29, 28, 24, 20, 16, 12, 10, 8, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 3, 2, 1, 0},
		.ratio_mode = 1, },
	.blend_w = 0,
	.rng_th_w = 0,
	.bin = 0,
};

CTL_IFE_ISP_DGAIN ctl_ife_dgain_init = {
	.enable = TRUE,
	.dgain = 256,
};

CTL_IFE_ISP_CGAIN ctl_ife_cgain_init = {
	.enable = TRUE,
	.inv = FALSE,
	.hinv = FALSE,
	.bit_field = CTL_IFE_ISP__2_8,
	.mask = 4095,
	.cgain_r = 256,
	.cgain_gr = 256,
	.cgain_gb = 256,
	.cgain_b = 256,
	.cgain_ir = 256,
	.cofs_r = 240,
	.cofs_gr = 240,
	.cofs_gb = 240,
	.cofs_b = 240,
	.cofs_ir = 240,
};

CTL_IFE_ISP_VIG ctl_ife_vig_init = {
	.enable = TRUE,
	.dist_th = 0,
	.ch_r_lut = {0, 1, 4, 7, 10, 13, 23, 36, 57, 82, 112, 144, 198, 259, 375, 586, 1023},
	.ch_gr_lut = {0, 1, 4, 7, 10, 13, 23, 36, 57, 82, 112, 144, 198, 259, 375, 586, 1023},
	.ch_gb_lut = {0, 1, 4, 7, 10, 13, 23, 36, 57, 82, 112, 144, 198, 259, 375, 586, 1023},
	.ch_b_lut = {0, 1, 4, 7, 10, 13, 23, 36, 57, 82, 112, 144, 198, 259, 375, 586, 1023},
	.ch_ir_lut = {0, 1, 4, 7, 10, 13, 23, 36, 57, 82, 112, 144, 198, 259, 375, 586, 1023},
	.dither_enable = TRUE,
	.dither_rst_enable = FALSE,
};

CTL_IFE_ISP_GBAL ctl_ife_gbal_init = {
	.enable = TRUE,
	.protect_enable = TRUE,
	.diff_th_str = 256,
	.diff_w_max = 15,
	.edge_protect_th1 = 160,
	.edge_protect_th0 = 128,
	.edge_w_max = 255,
	.edge_w_min = 16,
	.gbal_ofs = {1, 3, 6, 9, 13, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48},
	.str_luma_low_bnd = 0,
	.edge_luma_low_bnd = 0,
};

CTL_IFE_ISP_WDR ctl_ife_wdr_init = {
	.wdr_enable = FALSE,
	.tonecurve_enable = TRUE,
	.ftrcoef = {4, 2, 1},
	.input_bld = {
		.bld_sel = CTL_IFE_ISP_INPUT_BLD_3x3_Y,
		.blend_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.in_yv_blend_lut = {0, 0, 0, 63, 63, 63, 63, 63, 63}, },
	.tonecurve = {
		.left_table = {
			0, 142, 284, 426, 568, 681, 795, 908, 1021, 1113,
			1205, 1297, 1389, 1466, 1542, 1619, 1695, 1760, 1824, 1889,
			1953, 2008, 2063, 2118, 2173, 2221, 2269, 2316, 2364, 2406,
			2448, 2489, 2531, 2568, 2605, 2641, 2678, 2711, 2743, 2776,
			2808, 2837, 2867, 2896, 2925, 2952, 2978, 3005, 3031, 3055,
			3079, 3102, 3126, 3148, 3170, 3191, 3213, 3233, 3253, 3273,
			3293, 3311, 3330, 3348, 3366},
		.right_table = {
			3366, 3434, 3496, 3555, 3610, 3662, 3710, 3756, 3800, 3842,
			3882, 3920, 3957, 3993, 4028, 4062, 4095}, },
	.wdr_str = {
		.wdr_coeff = {-1732, -2715, -2548, -2548},
		.strength = 128,
		.wdr_mode = CTL_IFE_ISP_WDR_ANTI_HALO_MODE,
		.wdr_anti_halo_opt = CTL_IFE_ISP_ANTI_HALO_ADAPTIVE,
		.wdr_halo_ratio = 128,
		.wdr_halo_slope = 64,
		.wdr_b2p_var = 512, },
	.gainctrl = {
		.gainctrl_en = TRUE,
		.max_gain = 20,
		.min_gain = 5, },
	.outbld = {
		.outbld_en = TRUE,
		.outbld_lut = {
			.left_table = {
				4021, 4016, 4011, 4005, 4000, 3994, 3987, 3980, 3973, 3965,
				3956, 3948, 3938, 3928, 3918, 3907, 3895, 3882, 3869, 3855,
				3840, 3825, 3808, 3791, 3772, 3753, 3732, 3711, 3688, 3664,
				3639, 3612, 3585, 3556, 3525, 3493, 3460, 3426, 3389, 3352,
				3312, 3272, 3229, 3185, 3140, 3093, 3044, 2994, 2943, 2890,
				2836, 2780, 2724, 2666, 2607, 2547, 2486, 2424, 2362, 2299,
				2236, 2172, 2108, 2044, 1980},
			.right_table = {
				1980, 1727, 1484, 1258, 1052, 870, 713, 579, 467, 374,
				299, 238, 188, 149, 118, 93, 73}, }, },
	.sat_reduct = {
		.sat_th = 0,
		.sat_wt_low = 0,
		.sat_delta = 255, },
	.dither = {
		.wdr_dithering_en = TRUE,
		.wdr_rand_rst = TRUE,
		.wdr_rand_sel = CTL_IFE_ISP_WDR_RANDOMLSB, },
	.fbc = {
		.fbc_en = TRUE,
		.fbc_ratio = 128,
		.fbc_th = {1536, 2048, 2560, 3072, 3200, 3500}, },
	.wdr_gain_prot_str = 256,
};

CTL_IFE_ISP_WDR_SUBIMG ctl_ife_wdr_subimg_init = {
	.subimg_size_h = 16,
	.subimg_size_v = 9,
};

CTL_IFE_ISP_HIST ctl_ife_hist_init = {
	.hist_enable = TRUE,
	.hist_sel = CTL_IFE_ISP_BEFORE_WDR,
	.step_h = 6,
	.step_v = 6,
};

CTL_IFE_ISP_VA ctl_ife_va_init = {
	.enable = TRUE,
	.indep_va_enable = TRUE,
	.ldg_enable = TRUE,
	.gamma_enable = FALSE,
	.group_1 = {
		.h_filt = {
			.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE,
			.filter_size = CTL_IFE_ISP_VA_FILTER_SIZE_3,
			.tap_a = 0,
			.tap_b = 44,
			.tap_c = 0,
			.tap_d = 0,
			.div = 0,
			.th_l = 0,
			.th_u = 0, },
		.v_filt = {
			.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE,
			.filter_size = CTL_IFE_ISP_VA_FILTER_SIZE_3,
			.tap_a = 0,
			.tap_b = 0,
			.tap_c = -1,
			.tap_d = 0,
			.div = 2,
			.th_l = 0,
			.th_u = 0, },
		.iir_filt = {
			.symmetry_iir2 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE,
			.symmetry_iir3 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE,
			.tap_iir1_e = -343,
			.tap_iir1_f = 202,
			.iir1_shift_bit = 6,
			.iir2_enable = TRUE,
			.tap_iir2_a = 0,
			.tap_iir2_b = 44,
			.tap_iir2_e = -443,
			.tap_iir2_f = 223,
			.iir2_shift_bit = 8,
			.iir3_enable = TRUE,
			.tap_iir3_a = 0,
			.tap_iir3_b = 41,
			.tap_iir3_e = -370,
			.tap_iir3_f = 174,
			.iir3_shift_bit = 8, },
		.count_enable = TRUE, },
	.group_2 = {
		.h_filt = {
			.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_MIRROR,
			.filter_size = CTL_IFE_ISP_VA_FILTER_SIZE_3,
			.tap_a = 2,
			.tap_b = 0,
			.tap_c = -1,
			.tap_d = 0,
			.div = 3,
			.th_l = 0,
			.th_u = 0, },
		.v_filt = {
			.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_MIRROR,
			.filter_size = CTL_IFE_ISP_VA_FILTER_SIZE_3,
			.tap_a = 2,
			.tap_b = 0,
			.tap_c = -1,
			.tap_d = 0,
			.div = 3,
			.th_l = 0,
			.th_u = 0, },
		.iir_filt = {
			.symmetry_iir2 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE,
			.symmetry_iir3 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE,
			.tap_iir1_e = -343,
			.tap_iir1_f = 202,
			.iir1_shift_bit = 6,
			.iir2_enable = TRUE,
			.tap_iir2_a = 0,
			.tap_iir2_b = 44,
			.tap_iir2_e = -443,
			.tap_iir2_f = 223,
			.iir2_shift_bit = 8,
			.iir3_enable = TRUE,
			.tap_iir3_a = 0,
			.tap_iir3_b = 41,
			.tap_iir3_e = -370,
			.tap_iir3_f = 174,
			.iir3_shift_bit = 8, },
		.count_enable = TRUE, },
	.fltr_sel = {
		.vdetgh1_filter_sel = CTL_IFE_ISP_VA_VDETGH_IIR,
		.vdetgh1_iir_input_sel = CTL_IFE_ISP_VA_VDETGH_IIR_INPUT_JUMP_PIXEL,
		.vdetgh2_filter_sel = CTL_IFE_ISP_VA_VDETGH_IIR,
		.vdetgh2_iir_input_sel = CTL_IFE_ISP_VA_VDETGH_IIR_INPUT_JUMP_PIXEL,
	},
	.va_out_grp1_2 = TRUE,
	.win_num = {
		.w = 8,
		.h = 8, },
	.indep_win[0] = {
		.enable = TRUE, },
	.indep_win[1] = {
		.enable = TRUE, },
	.indep_win[2] = {
		.enable = TRUE, },
	.indep_win[3] = {
		.enable = TRUE, },
	.indep_win[4] = {
		.enable = TRUE, },
	.ldg_para = {
		.ldg_low_th = 10,
		.ldg_high_th = 150,
		.ldg_low_gain = 255,
		.ldg_high_gain = 255,
		.ldg_low_slope = 2,
		.ldg_high_slope = 6, },
	.pre_filter_mode = CTL_IFE_ISP_PRE_FILTER_3x3_2,
	.va_gamma_lut = {
		0, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304,
		2560, 2816, 3072, 3328, 3584, 3840, 4095},
	.win_cnt_out_sel = CTL_IFE_ISP_VA_ENERGY_COUNT,
	.high_luma_th = 150,
	.energy_w = 16,
};

CTL_IFE_ISP_SUBISP_IQ ctl_ife_subisp_init = {
	.subisp_cst_en = TRUE,
	.coef = {256, 0, 0, 0, 256, 0, 0, 0, 256},
	.subisp_gamma_en = TRUE,
	.subisp_gamma_lut = {
		0, 60, 90, 114, 131, 145, 158, 171, 183, 194,
		204, 213, 223, 241, 240, 248, 255},
	.subisp_cfa_en = TRUE,
};

CTL_IFE_ISP_FPN ctl_ife_fpn_init = {
	.fpn_en = FALSE,
	.fpn_cgain_range = 0, // 0: 2_8; 1: 3_7
	.fpn_cgain_r = {256},
	.fpn_cgain_gr = {256},
	.fpn_cgain_gb = {256},
	.fpn_cgain_b = {256},
	.fpn_cgain_ir = {256},
	.fpn_cofs_r = {240},
	.fpn_cofs_gr = {240},
	.fpn_cofs_gb = {240},
	.fpn_cofs_b = {240},
	.fpn_cofs_ir = {240},
};

CTL_IPE_ISP_EEXT ctl_ipe_eext_init = {
	.edge_ker = {380, 42, -80, -30, -35, 4, 10, 10, 4, 1, 10, 5},
	.eext_kerstrength = {
		.ker_freq0 = {.eext_enh = 24, .eext_div = 3},
		.ker_freq1 = {.eext_enh = 24, .eext_div = 3},
		.ker_freq2 = {.eext_enh = 24, .eext_div = 3}, },
	.eext_engcon = {
		.eext_div_con = 0,
		.eext_div_eng = 0,
		.wt_con_eng = 4, },
	.ker_thickness = {
		.wt_ker_thin = 0,
		.wt_ker_robust = 0,
		.iso_ker_thin = 8,
		.iso_ker_robust = 0, },
	.ker_thickness_hld = {
		.wt_ker_thin = 0,
		.wt_ker_robust = 0,
		.iso_ker_thin = 8,
		.iso_ker_robust = 0, },
	.eext_region = {
		.reg_th = {
			.th_flat = 40,
			.th_edge = 240,
			.th_flat_hld = 40,
			.th_edge_hld = 240,
			.th_lum_hld = 1023, },
		.reg_wt = {
			.wt_low = 0,
			.wt_high = 16,
			.wt_low_hld = 0,
			.wt_high_hld = 16, }, },
	.eext_blending = {
		.eext_blending_th = 50,
		.eext_blending_w1 = 224,
		.eext_blending_w2 = 16,
		.eext_blending_slope = 8, },
	.dir_ker_strength = {
		.ker_h = {
			.eext_enh = 2,
			.eext_div = 0, },
		.ker_v = {
			.eext_enh = 2,
			.eext_div = 0, },
		.ker_d1 = {
			.eext_enh = 2,
			.eext_div = 0, },
		.ker_d2 = {
			.eext_enh = 2,
			.eext_div = 0, }, },
	.dir_ker_para = {
		.eext_dir_min_s_low_bound = 80,
		.eext_dir_s_th_mul = 1,
		.eext_dir_s_th_shift = 0,
		.dir_eng_blend_w1 = 16,
		.dir_eng_blend_w2 = 8,
		.dir_eng_blend_w3 = 4,
		.dir_eng_blend_w4 = 0,
		.s_count_th = 3, },
};

CTL_IPE_ISP_EEXT_TONEMAP ctl_ipe_eext_tone_init = {
	.gamma_sel = CTL_IPE_ISP_EEXT_POST_GAM,
	.tone_map_lut = {
		0, 73, 139, 198, 241, 273, 304, 333, 360, 386,
		411, 433, 455, 475, 493, 510, 525, 539, 553, 566,
		580, 594, 607, 621, 634, 647, 659, 672, 684, 697,
		709, 721, 733, 744, 756, 767, 777, 787, 797, 806,
		816, 826, 835, 845, 854, 863, 872, 882, 891, 900,
		908, 917, 926, 934, 943, 951, 959, 968, 976, 984,
		992, 1000, 1007, 1015, 1023},
};

CTL_IPE_ISP_EDGE_OVERSHOOT ctl_ipe_edge_overshoot_init = {
	.overshoot_en = TRUE,
	.wt_overshoot = 128,
	.wt_undershoot = 128,
	.th_overshoot = 192,
	.th_undershoot = 0,
	.th_undershoot_lum = 255,
	.th_undershoot_eng = 255,
	.slope_overshoot = 256,
	.slope_undershoot = 256,
	.slope_undershoot_lum = 96,
	.slope_undershoot_eng = 96,
	.clamp_wt_mod_lum = 0,
	.clamp_wt_mod_eng = 0,
	.strength_lum_eng = 1,
	.norm_lum_eng = 0,
};

CTL_IPE_ISP_EPROC ctl_ipe_eproc_init = {
	.edge_map_th = {
		.ethr_low = 0,
		.ethr_high = 128,
		.etab_low = 0,
		.etab_high = 2,
		.map_sel = CTL_IPE_ISP_EIN_EEXT, },
	.edge_map_lut = {0, 36, 72, 108, 144, 180, 216, 255, 255, 224, 192, 170, 154, 144, 136, 128},
	.es_map_th = {
		.ethr_low = 0,
		.ethr_high = 512,
		.etab_low = 0,
		.etab_high = 5, },
	.es_map_lut = {32, 42, 53, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 52, 44, 38},
};

CTL_IPE_ISP_RGBLPF ctl_ipe_rgblpf_init = {
	.enable = TRUE,
	.lpf_param_r = {
		.lpf_w = 4,
		.s_only_w = 0,
		.range_th0 = 8,
		.range_th1 = 16,
		.filt_size = CTL_IPE_ISP_LPFSIZE_5X5, },
	.lpf_param_g = {
		.lpf_w = 4,
		.s_only_w = 0,
		.range_th0 = 8,
		.range_th1 = 16,
		.filt_size = CTL_IPE_ISP_LPFSIZE_5X5, },
	.lpf_param_b = {
		.lpf_w = 4,
		.s_only_w = 0,
		.range_th0 = 8,
		.range_th1 = 16,
		.filt_size = CTL_IPE_ISP_LPFSIZE_5X5, },
};

CTL_IPE_ISP_PFR ctl_ipe_pfr_init = {
	.enable = FALSE,
	.uv_filt_en = TRUE,
	.luma_level_en = TRUE,
	.out_wet = 224,
	.pfr_strength = 224,
	.edge_th = 0,
	.edge_str = 255,
	.luma_th = 300,
	.luma_lut = {0, 0, 0, 8, 112, 232, 255, 255, 255, 255, 255, 255, 255},
	.color_wet_g = 255,
	.color_wet_set[0] = {
		.enable = TRUE,
		.color_u = 175,
		.color_v = 145,
		.color_wet_r = 255,
		.color_wet_b = 255,
		.cdiff_th = 16,
		.cdiff_step = 4,
		.cdiff_lut = {255, 132, 12, 0, 0},
		},
	.color_wet_set[1] = {
		.enable = TRUE,
		.color_u = 145,
		.color_v = 175,
		.color_wet_r = 255,
		.color_wet_b = 255,
		.cdiff_th = 16,
		.cdiff_step = 4,
		.cdiff_lut = {255, 132, 12, 0, 0},
		},
	.color_wet_set[2] = {
		.enable = FALSE,
		.color_u = 80,
		.color_v = 116,
		.color_wet_r = 255,
		.color_wet_b = 255,
		.cdiff_th = 16,
		.cdiff_step = 4,
		.cdiff_lut = {255, 132, 12, 0, 0},
		},
	.color_wet_set[3] = {
		.enable = FALSE,
		.color_u = 116,
		.color_v = 80,
		.color_wet_r = 255,
		.color_wet_b = 255,
		.cdiff_th = 16,
		.cdiff_step = 4,
		.cdiff_lut = {255, 132, 12, 0, 0},
		},
};

CTL_IPE_ISP_CC ctl_ipe_cc_init = {
	.enable = TRUE,
	.cc2_sel = CTL_IPE_ISP_CC2_IDENTITY,
	.cc_stab_sel = CTL_IPE_ISP_CC_MAX,
	.fstab = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	.fdtab = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

CTL_IPE_ISP_CCM ctl_ipe_ccm_init = {
	.cc_range = CTL_IPE_ISP_CCRANGE_3_8,
	.cc_gamma_sel = CTL_IPE_ISP_CC_PRE_GAM,
	.coef = {256, 0, 0, 0, 256, 0, 0, 0, 256},
};

CTL_IPE_ISP_CCTRL ctl_ipe_cctrl_init = {
	.enable = TRUE,
	.int_ofs = 0,
	.sat_ofs = 0,
	.hue_c2g = FALSE,
	.cctrl_sel = CTL_IPE_ISP_CCTRL_E7,
	.vdet_div = 40,
	.edge_tab = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
	.dds_tab = {32, 32, 32, 32, 32, 32, 32, 32},
	.hue_rotate_en = FALSE,
	.hue_tab = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
	.sat_tab = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	.int_tab = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

CTL_IPE_ISP_CADJ_EE ctl_ipe_cadj_ee_init = {
	.enable = TRUE,
	.edge_enh_p = 128,
	.edge_enh_n = 128,
	.edge_inv_p_en = FALSE,
	.edge_inv_n_en = FALSE,
};

CTL_IPE_ISP_CADJ_YCCON ctl_ipe_cadj_yccon_init = {
	.enable = TRUE,
	.y_con = 128,
	.c_con = 128,
	.cconlut = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
	.ccontab_sel = CTL_IPE_ISP_SATURATION,
};

CTL_IPE_ISP_CADJ_COFS ctl_ipe_cadj_cofs_init = {
	.enable = TRUE,
	.cb_ofs = 128,
	.cr_ofs = 128,
};

CTL_IPE_ISP_CADJ_RAND ctl_ipe_cadj_rand_init = {
	.enable = FALSE,
	.rand_en_y = FALSE,
	.rand_en_c = FALSE,
	.rand_level_y = 0,
	.rand_level_c = 0,
	.rand_reset = FALSE,
};

CTL_IPE_ISP_CADJ_HUE ctl_ipe_cadj_hue_init = {
	.enable = TRUE,
};

CTL_IPE_ISP_CADJ_FIXTH ctl_ipe_cadj_fixth_init = {
	.enable = FALSE,
	.yth1 = {
		.y_th = 0,
		.edge_th = 0,
		.ycth_sel_hit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE,
		.ycth_sel_nonhit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE,
		.value_hit = 0,
		.nonvalue_hit = 0, },
	.yth2 = {
		.y_th = 0,
		.ycth_sel_hit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE,
		.ycth_sel_nonhit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE,
		.value_hit = 0,
		.nonvalue_hit = 0, },
	.cth = {
		.edge_th = 0,
		.y_th_low = 0,
		.y_th_high = 0,
		.cb_th_low = 0,
		.cb_th_high = 0,
		.cr_th_low = 0,
		.cr_th_high = 0,
		.ycth_sel_hit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE,
		.ycth_sel_nonhit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE,
		.cb_value_hit = 0,
		.cb_value_nonhit = 0,
		.cr_value_hit = 0,
		.cr_value_nonhit = 0, },
};

CTL_IPE_ISP_CADJ_MASK ctl_ipe_cadj_mask_init = {
	.enable = FALSE,
	.y_mask = 255,
	.cb_mask = 255,
	.cr_mask = 255,
};

CTL_IPE_ISP_CST ctl_ipe_cst_init = {
	.enable = TRUE,
	.cst_coef = {77, 150, 29, -43, -85, 128, 128, -107, -21},
	.cst_off_sel = CTL_IPE_ISP_CST_MINUS128,
};

CTL_IPE_ISP_CSTP ctl_ipe_cstp_init = {
	.enable = TRUE,
	.cstp_ratio = 2,
};

CTL_IPE_ISP_GAMYRAND ctl_ipe_gamyrand_init = {
	.enable = FALSE,
	.rand_en = FALSE,
	.rst_en = FALSE,
	.rand_shift = 0,
};

CTL_IPE_ISP_GAMMA ctl_ipe_gamma_init = {
	.enable = TRUE,
	.option = CTL_IPE_ISP_GAMMA_RGB_COMBINE,
	.lut.gamma_lut[CTL_IPE_ISP_RGB_R] = {
		0, 37, 73, 107, 139, 169, 198, 225, 241, 257,
		273, 289, 304, 318, 333, 347, 360, 373, 386, 399,
		411, 422, 433, 444, 455, 465, 475, 484, 493, 502,
		510, 518, 525, 532, 539, 546, 553, 560, 566, 573,
		580, 587, 594, 601, 607, 614, 621, 627, 634, 640,
		647, 653, 659, 666, 672, 678, 684, 691, 697, 703,
		709, 715, 721, 727, 733, 739, 744, 750, 756, 762,
		767, 772, 777, 782, 787, 792, 797, 801, 806, 811,
		816, 821, 826, 830, 835, 840, 845, 849, 854, 859,
		863, 868, 872, 877, 882, 886, 891, 895, 900, 904,
		908, 913, 917, 921, 926, 930, 934, 939, 943, 947,
		951, 955, 959, 964, 968, 972, 976, 980, 984, 988,
		992, 996, 1000, 1003, 1007, 1011, 1015, 1019, 1023},
	.lut.gamma_lut[CTL_IPE_ISP_RGB_G] = {
		0, 37, 73, 107, 139, 169, 198, 225, 241, 257,
		273, 289, 304, 318, 333, 347, 360, 373, 386, 399,
		411, 422, 433, 444, 455, 465, 475, 484, 493, 502,
		510, 518, 525, 532, 539, 546, 553, 560, 566, 573,
		580, 587, 594, 601, 607, 614, 621, 627, 634, 640,
		647, 653, 659, 666, 672, 678, 684, 691, 697, 703,
		709, 715, 721, 727, 733, 739, 744, 750, 756, 762,
		767, 772, 777, 782, 787, 792, 797, 801, 806, 811,
		816, 821, 826, 830, 835, 840, 845, 849, 854, 859,
		863, 868, 872, 877, 882, 886, 891, 895, 900, 904,
		908, 913, 917, 921, 926, 930, 934, 939, 943, 947,
		951, 955, 959, 964, 968, 972, 976, 980, 984, 988,
		992, 996, 1000, 1003, 1007, 1011, 1015, 1019, 1023},
	.lut.gamma_lut[CTL_IPE_ISP_RGB_B] = {
		0, 37, 73, 107, 139, 169, 198, 225, 241, 257,
		273, 289, 304, 318, 333, 347, 360, 373, 386, 399,
		411, 422, 433, 444, 455, 465, 475, 484, 493, 502,
		510, 518, 525, 532, 539, 546, 553, 560, 566, 573,
		580, 587, 594, 601, 607, 614, 621, 627, 634, 640,
		647, 653, 659, 666, 672, 678, 684, 691, 697, 703,
		709, 715, 721, 727, 733, 739, 744, 750, 756, 762,
		767, 772, 777, 782, 787, 792, 797, 801, 806, 811,
		816, 821, 826, 830, 835, 840, 845, 849, 854, 859,
		863, 868, 872, 877, 882, 886, 891, 895, 900, 904,
		908, 913, 917, 921, 926, 930, 934, 939, 943, 947,
		951, 955, 959, 964, 968, 972, 976, 980, 984, 988,
		992, 996, 1000, 1003, 1007, 1011, 1015, 1019, 1023},
};

CTL_IPE_ISP_YCURVE ctl_ipe_ycurve_init = {
	.enable = FALSE,
	.ycurve_sel = CTL_IPE_ISP_YCURVE_YUV,
	.lut.y_curve_lut = {
		0, 2, 4, 6, 8, 10, 12, 14, 16, 18,
		20, 22, 24, 26, 28, 30, 32, 34, 36, 38,
		40, 42, 44, 46, 48, 50, 52, 54, 56, 58,
		60, 62, 64, 66, 68, 70, 72, 74, 76, 78,
		80, 82, 84, 86, 88, 90, 92, 94, 96, 98,
		100, 102, 104, 106, 108, 110, 112, 114, 116, 118,
		120, 122, 124, 126, 128, 130, 132, 134, 136, 138,
		140, 142, 144, 146, 148, 150, 152, 154, 156, 158,
		160, 162, 164, 166, 168, 170, 172, 174, 176, 178,
		180, 182, 184, 186, 188, 190, 192, 194, 196, 198,
		200, 202, 204, 206, 208, 210, 212, 214, 216, 218,
		220, 222, 224, 226, 228, 230, 232, 234, 236, 238,
		240, 242, 244, 246, 248, 250, 252, 254, 255},
};

CTL_IPE_ISP_DEFOG ctl_ipe_defog_init = {
	.enable = FALSE,
	.scalup_param = {
		.interp_diff_lut = {63, 58, 43, 27, 13, 6, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.interp_wdist = 0,
		.interp_wout = 8,
		.interp_wcenter = 1,
		.interp_wsrc = 26, },
	.input_bld = {
		.in_blend_wt = {0, 32, 64, 96, 128, 160, 192, 224, 255}, },
	.env_estimation = {
		.dfg_self_comp_en = FALSE,
		.dfg_min_diff = 480,
		.dfg_airlight = {1023, 1023, 1023},
		.fog_mod_lut = {0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1023}, },
	.dfg_strength = {
		.str_mode_sel = CTL_IPE_ISP_DEFOG_METHOD_B,
		.target_lut = {0, 0, 0, 0, 0, 0, 0, 0, 0},
		.fog_ratio = 255,
		.dgain_ratio = 255,
		.gain_th = 96, },
	.dfg_outbld = {
		.outbld_ref_sel = CTL_IPE_ISP_DEFOG_OUTBLD_REF_AFTER,
		.outbld_local_en = TRUE,
		.outbld_lum_wt = {192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192},
		.outbld_diff_wt = {0, 0, 24, 36, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48}, },
	.dfg_stcs = {
		.airlight_stcs_ratio = 8, },
	.dfg_round = {
		.rand_opt = CTL_IPE_ISP_ROUNDING,
		.rand_rst = CTL_IPE_ISP_B_DISABLE, }
};

CTL_IPE_ISP_LCE ctl_ipe_lce_init = {
	.enable = TRUE,
	.diff_wt_pos = 0,
	.diff_wt_neg = 0,
	.diff_wt_avg = 128,
	.lum_wt_lut = {64, 64, 64, 64, 64, 64, 64, 64, 64},
};

CTL_IPE_ISP_SUBIMG ctl_ipe_subimg_init = {
	.subimg_size = {
		.h_size = 32,
		.v_size = 18, },
	.subimg_ftrcoef = {4, 2, 1},
};

CTL_IPE_ISP_EDGEDBG ctl_ipe_edgedbg_init = {
	.enable = FALSE,
	.mode_sel = CTL_IPE_ISP_DBG_EDGE_REGION,
};

CTL_IPE_ISP_VA ctl_ipe_va_init = {
	.enable = TRUE,
	.indep_va_enable = TRUE,
	.ldg_enable = TRUE,
	.group_1 = {
		.h_filt = {
			.symmetry = CTL_IPE_ISP_VA_FILTER_SYM_INVERSE,
			.filter_size = CTL_IPE_ISP_VA_FILTER_SIZE_3,
			.tap_a = 0,
			.tap_b = 0,
			.tap_c = -1,
			.tap_d = 0,
			.div = 2,
			.th_l = 0,
			.th_u = 0, },
		.v_filt = {
			.symmetry = CTL_IPE_ISP_VA_FILTER_SYM_INVERSE,
			.filter_size = CTL_IPE_ISP_VA_FILTER_SIZE_3,
			.tap_a = 0,
			.tap_b = 0,
			.tap_c = -1,
			.tap_d = 0,
			.div = 2,
			.th_l = 0,
			.th_u = 0, },
		.count_enable = TRUE, },
	.group_2 = {
		.h_filt = {
			.symmetry = CTL_IPE_ISP_VA_FILTER_SYM_MIRROR,
			.filter_size = CTL_IPE_ISP_VA_FILTER_SIZE_3,
			.tap_a = 2,
			.tap_b = 0,
			.tap_c = -1,
			.tap_d = 0,
			.div = 3,
			.th_l = 0,
			.th_u = 0, },
		.v_filt = {
			.symmetry = CTL_IPE_ISP_VA_FILTER_SYM_MIRROR,
			.filter_size = CTL_IPE_ISP_VA_FILTER_SIZE_3,
			.tap_a = 2,
			.tap_b = 0,
			.tap_c = -1,
			.tap_d = 0,
			.div = 3,
			.th_l = 0,
			.th_u = 0, },
		.count_enable = TRUE, },
	.va_out_grp1_2 = TRUE,
	.win_num = {
		.w = 8,
		.h = 8, },
	.indep_win[0] = {
		.enable = TRUE, },
	.indep_win[1] = {
		.enable = TRUE, },
	.indep_win[2] = {
		.enable = TRUE, },
	.indep_win[3] = {
		.enable = TRUE, },
	.indep_win[4] = {
		.enable = TRUE, },
	.ldg_para = {
		.ldg_low_th = 10,
		.ldg_high_th = 150,
		.ldg_low_gain = 255,
		.ldg_high_gain = 255,
		.ldg_low_slope = 2,
		.ldg_high_slope = 6, },
	.pre_filter_mode = CTL_IPE_ISP_PRE_FILTER_3x3_2,
	.win_cnt_out_sel = CTL_IPE_ISP_VA_ENERGY_COUNT,
	.high_luma_th = 150,
	.energy_w = 16,
};

CTL_IPE_ISP_EDGE_REGION_STR ctl_ipe_edge_region_str_init = {
	.enable = TRUE,
	.enh_thin = 64,
	.enh_robust = 64,
	.slope_flat = -1024,
	.slope_edge = 1024,
	.str_flat = 64,
	.str_edge = 80,
};

CTL_IPE_ISP_CFA ctl_ipe_cfa_init = {
	.cfa_enable = TRUE,
	.cfa_interp = {
		.edge_dth = 10,
		.edge_dth2 = 40,
		.freq_th = 160,
		.freq_lut = {0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 15},
		.luma_wt = {128, 120, 95, 72, 58, 48, 40, 36, 32, 32, 32, 32, 32, 32, 32, 32, 32}, },
	.cfa_correction = {
		.rb_corr_enable = TRUE,
		.rb_corr_th1 = 0,
		.rb_corr_th2 = 0, },
	.cfa_fcs = {
		.fcs_dirsel = TRUE,
		.fcs_coring = 8,
		.fcs_weight = 64,
		.fcs_strength = {15, 15, 7, 4, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0}, },
	.cfa_ir_hfc = {
		.cl_check_enable = TRUE,
		.hf_check_enable = TRUE,
		.average_mode = TRUE,
		.cl_sel = FALSE,
		.cl_th = 16,
		.hf_gth = 3,
		.hf_diff = 128,
		.hf_eth = 64,
		.ir_g_edge_th = 8,
		.ir_rb_cstrength = 4, },
	.cfa_ir_sub = {
		.ir_sub_r = 204,
		.ir_sub_g = 204,
		.ir_sub_b = 230,
		.ir_sub_wt_lb = 0,
		.ir_sub_th = 168,
		.ir_sub_range = 2,
		.ir_sat_gain = 256, },
	.cfa_pink_reduc = {
		.pink_rd_en = FALSE,
		.pink_rd_mode = CTL_IPE_ISP_PINKR_MOD_G,
		.pink_rd_th1 = 192,
		.pink_rd_th2 = 224,
		.pink_rd_th3 = 255,
		.pink_rd_th4 = 255, },
	.cfa_cgain = {
		.r_gain = 256,
		.g_gain = 256,
		.b_gain = 256,
		.gain_range = CTL_IPE_ISP_CGAIN_2_8, },
};

CTL_IPE_ISP_3DCC ctl_ipe_3dcc_init = {
	.enable = FALSE,
	#if 1
	.lut.rgb_3d_lut = {
		0x00000000, 0x00000020, 0x00000040, 0x00000060, 0x00000080, 0x000000a0, 0x000000c0, 0x000000e0, 0x000000ff,
		0x00002000, 0x00002020, 0x00002040, 0x00002060, 0x00002080, 0x000020a0, 0x000020c0, 0x000020e0, 0x000020ff,
		0x00004000, 0x00004020, 0x00004040, 0x00004060, 0x00004080, 0x000040a0, 0x000040c0, 0x000040e0, 0x000040ff,
		0x00006000, 0x00006020, 0x00006040, 0x00006060, 0x00006080, 0x000060a0, 0x000060c0, 0x000060e0, 0x000060ff,
		0x00008000, 0x00008020, 0x00008040, 0x00008060, 0x00008080, 0x000080a0, 0x000080c0, 0x000080e0, 0x000080ff,
		0x0000a000, 0x0000a020, 0x0000a040, 0x0000a060, 0x0000a080, 0x0000a0a0, 0x0000a0c0, 0x0000a0e0, 0x0000a0ff,
		0x0000c000, 0x0000c020, 0x0000c040, 0x0000c060, 0x0000c080, 0x0000c0a0, 0x0000c0c0, 0x0000c0e0, 0x0000c0ff,
		0x0000e000, 0x0000e020, 0x0000e040, 0x0000e060, 0x0000e080, 0x0000e0a0, 0x0000e0c0, 0x0000e0e0, 0x0000e0ff,
		0x0000ff00, 0x0000ff20, 0x0000ff40, 0x0000ff60, 0x0000ff80, 0x0000ffa0, 0x0000ffc0, 0x0000ffe0, 0x0000ffff,

		0x00200000, 0x00200020, 0x00200040, 0x00200060, 0x00200080, 0x002000a0, 0x002000c0, 0x002000e0, 0x002000ff,
		0x00202000, 0x00202020, 0x00202040, 0x00202060, 0x00202080, 0x002020a0, 0x002020c0, 0x002020e0, 0x002020ff,
		0x00204000, 0x00204020, 0x00204040, 0x00204060, 0x00204080, 0x002040a0, 0x002040c0, 0x002040e0, 0x002040ff,
		0x00206000, 0x00206020, 0x00206040, 0x00206060, 0x00206080, 0x002060a0, 0x002060c0, 0x002060e0, 0x002060ff,
		0x00208000, 0x00208020, 0x00208040, 0x00208060, 0x00208080, 0x002080a0, 0x002080c0, 0x002080e0, 0x002080ff,
		0x0020a000, 0x0020a020, 0x0020a040, 0x0020a060, 0x0020a080, 0x0020a0a0, 0x0020a0c0, 0x0020a0e0, 0x0020a0ff,
		0x0020c000, 0x0020c020, 0x0020c040, 0x0020c060, 0x0020c080, 0x0020c0a0, 0x0020c0c0, 0x0020c0e0, 0x0020c0ff,
		0x0020e000, 0x0020e020, 0x0020e040, 0x0020e060, 0x0020e080, 0x0020e0a0, 0x0020e0c0, 0x0020e0e0, 0x0020e0ff,
		0x0020ff00, 0x0020ff20, 0x0020ff40, 0x0020ff60, 0x0020ff80, 0x0020ffa0, 0x0020ffc0, 0x0020ffe0, 0x0020ffff,

		0x00400000, 0x00400020, 0x00400040, 0x00400060, 0x00400080, 0x004000a0, 0x004000c0, 0x004000e0, 0x004000ff,
		0x00402000, 0x00402020, 0x00402040, 0x00402060, 0x00402080, 0x004020a0, 0x004020c0, 0x004020e0, 0x004020ff,
		0x00404000, 0x00404020, 0x00404040, 0x00404060, 0x00404080, 0x004040a0, 0x004040c0, 0x004040e0, 0x004040ff,
		0x00406000, 0x00406020, 0x00406040, 0x00406060, 0x00406080, 0x004060a0, 0x004060c0, 0x004060e0, 0x004060ff,
		0x00408000, 0x00408020, 0x00408040, 0x00408060, 0x00408080, 0x004080a0, 0x004080c0, 0x004080e0, 0x004080ff,
		0x0040a000, 0x0040a020, 0x0040a040, 0x0040a060, 0x0040a080, 0x0040a0a0, 0x0040a0c0, 0x0040a0e0, 0x0040a0ff,
		0x0040c000, 0x0040c020, 0x0040c040, 0x0040c060, 0x0040c080, 0x0040c0a0, 0x0040c0c0, 0x0040c0e0, 0x0040c0ff,
		0x0040e000, 0x0040e020, 0x0040e040, 0x0040e060, 0x0040e080, 0x0040e0a0, 0x0040e0c0, 0x0040e0e0, 0x0040e0ff,
		0x0040ff00, 0x0040ff20, 0x0040ff40, 0x0040ff60, 0x0040ff80, 0x0040ffa0, 0x0040ffc0, 0x0040ffe0, 0x0040ffff,

		0x00600000, 0x00600020, 0x00600040, 0x00600060, 0x00600080, 0x006000a0, 0x006000c0, 0x006000e0, 0x006000ff,
		0x00602000, 0x00602020, 0x00602040, 0x00602060, 0x00602080, 0x006020a0, 0x006020c0, 0x006020e0, 0x006020ff,
		0x00604000, 0x00604020, 0x00604040, 0x00604060, 0x00604080, 0x006040a0, 0x006040c0, 0x006040e0, 0x006040ff,
		0x00606000, 0x00606020, 0x00606040, 0x00606060, 0x00606080, 0x006060a0, 0x006060c0, 0x006060e0, 0x006060ff,
		0x00608000, 0x00608020, 0x00608040, 0x00608060, 0x00608080, 0x006080a0, 0x006080c0, 0x006080e0, 0x006080ff,
		0x0060a000, 0x0060a020, 0x0060a040, 0x0060a060, 0x0060a080, 0x0060a0a0, 0x0060a0c0, 0x0060a0e0, 0x0060a0ff,
		0x0060c000, 0x0060c020, 0x0060c040, 0x0060c060, 0x0060c080, 0x0060c0a0, 0x0060c0c0, 0x0060c0e0, 0x0060c0ff,
		0x0060e000, 0x0060e020, 0x0060e040, 0x0060e060, 0x0060e080, 0x0060e0a0, 0x0060e0c0, 0x0060e0e0, 0x0060e0ff,
		0x0060ff00, 0x0060ff20, 0x0060ff40, 0x0060ff60, 0x0060ff80, 0x0060ffa0, 0x0060ffc0, 0x0060ffe0, 0x0060ffff,

		0x00800000, 0x00800020, 0x00800040, 0x00800060, 0x00800080, 0x008000a0, 0x008000c0, 0x008000e0, 0x008000ff,
		0x00802000, 0x00802020, 0x00802040, 0x00802060, 0x00802080, 0x008020a0, 0x008020c0, 0x008020e0, 0x008020ff,
		0x00804000, 0x00804020, 0x00804040, 0x00804060, 0x00804080, 0x008040a0, 0x008040c0, 0x008040e0, 0x008040ff,
		0x00806000, 0x00806020, 0x00806040, 0x00806060, 0x00806080, 0x008060a0, 0x008060c0, 0x008060e0, 0x008060ff,
		0x00808000, 0x00808020, 0x00808040, 0x00808060, 0x00808080, 0x008080a0, 0x008080c0, 0x008080e0, 0x008080ff,
		0x0080a000, 0x0080a020, 0x0080a040, 0x0080a060, 0x0080a080, 0x0080a0a0, 0x0080a0c0, 0x0080a0e0, 0x0080a0ff,
		0x0080c000, 0x0080c020, 0x0080c040, 0x0080c060, 0x0080c080, 0x0080c0a0, 0x0080c0c0, 0x0080c0e0, 0x0080c0ff,
		0x0080e000, 0x0080e020, 0x0080e040, 0x0080e060, 0x0080e080, 0x0080e0a0, 0x0080e0c0, 0x0080e0e0, 0x0080e0ff,
		0x0080ff00, 0x0080ff20, 0x0080ff40, 0x0080ff60, 0x0080ff80, 0x0080ffa0, 0x0080ffc0, 0x0080ffe0, 0x0080ffff,

		0x00a00000, 0x00a00020, 0x00a00040, 0x00a00060, 0x00a00080, 0x00a000a0, 0x00a000c0, 0x00a000e0, 0x00a000ff,
		0x00a02000, 0x00a02020, 0x00a02040, 0x00a02060, 0x00a02080, 0x00a020a0, 0x00a020c0, 0x00a020e0, 0x00a020ff,
		0x00a04000, 0x00a04020, 0x00a04040, 0x00a04060, 0x00a04080, 0x00a040a0, 0x00a040c0, 0x00a040e0, 0x00a040ff,
		0x00a06000, 0x00a06020, 0x00a06040, 0x00a06060, 0x00a06080, 0x00a060a0, 0x00a060c0, 0x00a060e0, 0x00a060ff,
		0x00a08000, 0x00a08020, 0x00a08040, 0x00a08060, 0x00a08080, 0x00a080a0, 0x00a080c0, 0x00a080e0, 0x00a080ff,
		0x00a0a000, 0x00a0a020, 0x00a0a040, 0x00a0a060, 0x00a0a080, 0x00a0a0a0, 0x00a0a0c0, 0x00a0a0e0, 0x00a0a0ff,
		0x00a0c000, 0x00a0c020, 0x00a0c040, 0x00a0c060, 0x00a0c080, 0x00a0c0a0, 0x00a0c0c0, 0x00a0c0e0, 0x00a0c0ff,
		0x00a0e000, 0x00a0e020, 0x00a0e040, 0x00a0e060, 0x00a0e080, 0x00a0e0a0, 0x00a0e0c0, 0x00a0e0e0, 0x00a0e0ff,
		0x00a0ff00, 0x00a0ff20, 0x00a0ff40, 0x00a0ff60, 0x00a0ff80, 0x00a0ffa0, 0x00a0ffc0, 0x00a0ffe0, 0x00a0ffff,

		0x00c00000, 0x00c00020, 0x00c00040, 0x00c00060, 0x00c00080, 0x00c000a0, 0x00c000c0, 0x00c000e0, 0x00c000ff,
		0x00c02000, 0x00c02020, 0x00c02040, 0x00c02060, 0x00c02080, 0x00c020a0, 0x00c020c0, 0x00c020e0, 0x00c020ff,
		0x00c04000, 0x00c04020, 0x00c04040, 0x00c04060, 0x00c04080, 0x00c040a0, 0x00c040c0, 0x00c040e0, 0x00c040ff,
		0x00c06000, 0x00c06020, 0x00c06040, 0x00c06060, 0x00c06080, 0x00c060a0, 0x00c060c0, 0x00c060e0, 0x00c060ff,
		0x00c08000, 0x00c08020, 0x00c08040, 0x00c08060, 0x00c08080, 0x00c080a0, 0x00c080c0, 0x00c080e0, 0x00c080ff,
		0x00c0a000, 0x00c0a020, 0x00c0a040, 0x00c0a060, 0x00c0a080, 0x00c0a0a0, 0x00c0a0c0, 0x00c0a0e0, 0x00c0a0ff,
		0x00c0c000, 0x00c0c020, 0x00c0c040, 0x00c0c060, 0x00c0c080, 0x00c0c0a0, 0x00c0c0c0, 0x00c0c0e0, 0x00c0c0ff,
		0x00c0e000, 0x00c0e020, 0x00c0e040, 0x00c0e060, 0x00c0e080, 0x00c0e0a0, 0x00c0e0c0, 0x00c0e0e0, 0x00c0e0ff,
		0x00c0ff00, 0x00c0ff20, 0x00c0ff40, 0x00c0ff60, 0x00c0ff80, 0x00c0ffa0, 0x00c0ffc0, 0x00c0ffe0, 0x00c0ffff,

		0x00e00000, 0x00e00020, 0x00e00040, 0x00e00060, 0x00e00080, 0x00e000a0, 0x00e000c0, 0x00e000e0, 0x00e000ff,
		0x00e02000, 0x00e02020, 0x00e02040, 0x00e02060, 0x00e02080, 0x00e020a0, 0x00e020c0, 0x00e020e0, 0x00e020ff,
		0x00e04000, 0x00e04020, 0x00e04040, 0x00e04060, 0x00e04080, 0x00e040a0, 0x00e040c0, 0x00e040e0, 0x00e040ff,
		0x00e06000, 0x00e06020, 0x00e06040, 0x00e06060, 0x00e06080, 0x00e060a0, 0x00e060c0, 0x00e060e0, 0x00e060ff,
		0x00e08000, 0x00e08020, 0x00e08040, 0x00e08060, 0x00e08080, 0x00e080a0, 0x00e080c0, 0x00e080e0, 0x00e080ff,
		0x00e0a000, 0x00e0a020, 0x00e0a040, 0x00e0a060, 0x00e0a080, 0x00e0a0a0, 0x00e0a0c0, 0x00e0a0e0, 0x00e0a0ff,
		0x00e0c000, 0x00e0c020, 0x00e0c040, 0x00e0c060, 0x00e0c080, 0x00e0c0a0, 0x00e0c0c0, 0x00e0c0e0, 0x00e0c0ff,
		0x00e0e000, 0x00e0e020, 0x00e0e040, 0x00e0e060, 0x00e0e080, 0x00e0e0a0, 0x00e0e0c0, 0x00e0e0e0, 0x00e0e0ff,
		0x00e0ff00, 0x00e0ff20, 0x00e0ff40, 0x00e0ff60, 0x00e0ff80, 0x00e0ffa0, 0x00e0ffc0, 0x00e0ffe0, 0x00e0ffff,

		0x00ff0000, 0x00ff0020, 0x00ff0040, 0x00ff0060, 0x00ff0080, 0x00ff00a0, 0x00ff00c0, 0x00ff00e0, 0x00ff00ff,
		0x00ff2000, 0x00ff2020, 0x00ff2040, 0x00ff2060, 0x00ff2080, 0x00ff20a0, 0x00ff20c0, 0x00ff20e0, 0x00ff20ff,
		0x00ff4000, 0x00ff4020, 0x00ff4040, 0x00ff4060, 0x00ff4080, 0x00ff40a0, 0x00ff40c0, 0x00ff40e0, 0x00ff40ff,
		0x00ff6000, 0x00ff6020, 0x00ff6040, 0x00ff6060, 0x00ff6080, 0x00ff60a0, 0x00ff60c0, 0x00ff60e0, 0x00ff60ff,
		0x00ff8000, 0x00ff8020, 0x00ff8040, 0x00ff8060, 0x00ff8080, 0x00ff80a0, 0x00ff80c0, 0x00ff80e0, 0x00ff80ff,
		0x00ffa000, 0x00ffa020, 0x00ffa040, 0x00ffa060, 0x00ffa080, 0x00ffa0a0, 0x00ffa0c0, 0x00ffa0e0, 0x00ffa0ff,
		0x00ffc000, 0x00ffc020, 0x00ffc040, 0x00ffc060, 0x00ffc080, 0x00ffc0a0, 0x00ffc0c0, 0x00ffc0e0, 0x00ffc0ff,
		0x00ffe000, 0x00ffe020, 0x00ffe040, 0x00ffe060, 0x00ffe080, 0x00ffe0a0, 0x00ffe0c0, 0x00ffe0e0, 0x00ffe0ff,
		0x00ffff00, 0x00ffff20, 0x00ffff40, 0x00ffff60, 0x00ffff80, 0x00ffffa0, 0x00ffffc0, 0x00ffffe0, 0x00ffffff, }
#else
	.rgb_3d_lut = { // refine
		// SRAM0, num = 125
		0x00000000, 0x00000040, 0x00000080, 0x000000c0, 0x000000ff,
		0x00004000, 0x00004040, 0x00004080, 0x000040c0, 0x000040ff,
		0x00008000, 0x00008040, 0x00008080, 0x000080c0, 0x000080ff,
		0x0000c000, 0x0000c040, 0x0000c080, 0x0000c0c0, 0x0000c0ff,
		0x0000ff00, 0x0000ff40, 0x0000ff80, 0x0000ffc0, 0x0000ffff,
		0x00400000, 0x00400040, 0x00400080, 0x004000c0, 0x004000ff,
		0x00404000, 0x00404040, 0x00404080, 0x004040c0, 0x004040ff,
		0x00408000, 0x00408040, 0x00408080, 0x004080c0, 0x004080ff,
		0x0040c000, 0x0040c040, 0x0040c080, 0x0040c0c0, 0x0040c0ff,
		0x0040ff00, 0x0040ff40, 0x0040ff80, 0x0040ffc0, 0x0040ffff,
		0x00800000, 0x00800040, 0x00800080, 0x008000c0, 0x008000ff,
		0x00804000, 0x00804040, 0x00804080, 0x008040c0, 0x008040ff,
		0x00808000, 0x00808040, 0x00808080, 0x008080c0, 0x008080ff,
		0x0080c000, 0x0080c040, 0x0080c080, 0x0080c0c0, 0x0080c0ff,
		0x0080ff00, 0x0080ff40, 0x0080ff80, 0x0080ffc0, 0x0080ffff,
		0x00c00000, 0x00c00040, 0x00c00080, 0x00c000c0, 0x00c000ff,
		0x00c04000, 0x00c04040, 0x00c04080, 0x00c040c0, 0x00c040ff,
		0x00c08000, 0x00c08040, 0x00c08080, 0x00c080c0, 0x00c080ff,
		0x00c0c000, 0x00c0c040, 0x00c0c080, 0x00c0c0c0, 0x00c0c0ff,
		0x00c0ff00, 0x00c0ff40, 0x00c0ff80, 0x00c0ffc0, 0x00c0ffff,
		0x00ff0000, 0x00ff0040, 0x00ff0080, 0x00ff00c0, 0x00ff00ff,
		0x00ff4000, 0x00ff4040, 0x00ff4080, 0x00ff40c0, 0x00ff40ff,
		0x00ff8000, 0x00ff8040, 0x00ff8080, 0x00ff80c0, 0x00ff80ff,
		0x00ffc000, 0x00ffc040, 0x00ffc080, 0x00ffc0c0, 0x00ffc0ff,
		0x00ffff00, 0x00ffff40, 0x00ffff80, 0x00ffffc0, 0x00ffffff,

		// SRAM1, num = 125
		0x00000020, 0x00000060, 0x000000a0, 0x000000e0, 0x00000000,
		0x00004020, 0x00004060, 0x000040a0, 0x000040e0, 0x00000000,
		0x00008020, 0x00008060, 0x000080a0, 0x000080e0, 0x00000000,
		0x0000c020, 0x0000c060, 0x0000c0a0, 0x0000c0e0, 0x00000000,
		0x0000ff20, 0x0000ff60, 0x0000ffa0, 0x0000ffe0, 0x00000000,
		0x00400020, 0x00400060, 0x004000a0, 0x004000e0, 0x00000000,
		0x00404020, 0x00404060, 0x004040a0, 0x004040e0, 0x00000000,
		0x00408020, 0x00408060, 0x004080a0, 0x004080e0, 0x00000000,
		0x0040c020, 0x0040c060, 0x0040c0a0, 0x0040c0e0, 0x00000000,
		0x0040ff20, 0x0040ff60, 0x0040ffa0, 0x0040ffe0, 0x00000000,
		0x00800020, 0x00800060, 0x008000a0, 0x008000e0, 0x00000000,
		0x00804020, 0x00804060, 0x008040a0, 0x008040e0, 0x00000000,
		0x00808020, 0x00808060, 0x008080a0, 0x008080e0, 0x00000000,
		0x0080c020, 0x0080c060, 0x0080c0a0, 0x0080c0e0, 0x00000000,
		0x0080ff20, 0x0080ff60, 0x0080ffa0, 0x0080ffe0, 0x00000000,
		0x00c00020, 0x00c00060, 0x00c000a0, 0x00c000e0, 0x00000000,
		0x00c04020, 0x00c04060, 0x00c040a0, 0x00c040e0, 0x00000000,
		0x00c08020, 0x00c08060, 0x00c080a0, 0x00c080e0, 0x00000000,
		0x00c0c020, 0x00c0c060, 0x00c0c0a0, 0x00c0c0e0, 0x00000000,
		0x00c0ff20, 0x00c0ff60, 0x00c0ffa0, 0x00c0ffe0, 0x00000000,
		0x00ff0020, 0x00ff0060, 0x00ff00a0, 0x00ff00e0, 0x00000000,
		0x00ff4020, 0x00ff4060, 0x00ff40a0, 0x00ff40e0, 0x00000000,
		0x00ff8020, 0x00ff8060, 0x00ff80a0, 0x00ff80e0, 0x00000000,
		0x00ffc020, 0x00ffc060, 0x00ffc0a0, 0x00ffc0e0, 0x00000000,
		0x00ffff20, 0x00ffff60, 0x00ffffa0, 0x00ffffe0, 0x00000000,

		// SRAM2, num = 125
		0x00002000, 0x00002040, 0x00002080, 0x000020c0, 0x000020ff,
		0x00006000, 0x00006040, 0x00006080, 0x000060c0, 0x000060ff,
		0x0000a000, 0x0000a040, 0x0000a080, 0x0000a0c0, 0x0000a0ff,
		0x0000e000, 0x0000e040, 0x0000e080, 0x0000e0c0, 0x0000e0ff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00402000, 0x00402040, 0x00402080, 0x004020c0, 0x004020ff,
		0x00406000, 0x00406040, 0x00406080, 0x004060c0, 0x004060ff,
		0x0040a000, 0x0040a040, 0x0040a080, 0x0040a0c0, 0x0040a0ff,
		0x0040e000, 0x0040e040, 0x0040e080, 0x0040e0c0, 0x0040e0ff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00802000, 0x00802040, 0x00802080, 0x008020c0, 0x008020ff,
		0x00806000, 0x00806040, 0x00806080, 0x008060c0, 0x008060ff,
		0x0080a000, 0x0080a040, 0x0080a080, 0x0080a0c0, 0x0080a0ff,
		0x0080e000, 0x0080e040, 0x0080e080, 0x0080e0c0, 0x0080e0ff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00c02000, 0x00c02040, 0x00c02080, 0x00c020c0, 0x00c020ff,
		0x00c06000, 0x00c06040, 0x00c06080, 0x00c060c0, 0x00c060ff,
		0x00c0a000, 0x00c0a040, 0x00c0a080, 0x00c0a0c0, 0x00c0a0ff,
		0x00c0e000, 0x00c0e040, 0x00c0e080, 0x00c0e0c0, 0x00c0e0ff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00ff2000, 0x00ff2040, 0x00ff2080, 0x00ff20c0, 0x00ff20ff,
		0x00ff6000, 0x00ff6040, 0x00ff6080, 0x00ff60c0, 0x00ff60ff,
		0x00ffa000, 0x00ffa040, 0x00ffa080, 0x00ffa0c0, 0x00ffa0ff,
		0x00ffe000, 0x00ffe040, 0x00ffe080, 0x00ffe0c0, 0x00ffe0ff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,

		// SRAM3, num = 125
		0x00002020, 0x00002060, 0x000020a0, 0x000020e0, 0x00000000,
		0x00006020, 0x00006060, 0x000060a0, 0x000060e0, 0x00000000,
		0x0000a020, 0x0000a060, 0x0000a0a0, 0x0000a0e0, 0x00000000,
		0x0000e020, 0x0000e060, 0x0000e0a0, 0x0000e0e0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00402020, 0x00402060, 0x004020a0, 0x004020e0, 0x00000000,
		0x00406020, 0x00406060, 0x004060a0, 0x004060e0, 0x00000000,
		0x0040a020, 0x0040a060, 0x0040a0a0, 0x0040a0e0, 0x00000000,
		0x0040e020, 0x0040e060, 0x0040e0a0, 0x0040e0e0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00802020, 0x00802060, 0x008020a0, 0x008020e0, 0x00000000,
		0x00806020, 0x00806060, 0x008060a0, 0x008060e0, 0x00000000,
		0x0080a020, 0x0080a060, 0x0080a0a0, 0x0080a0e0, 0x00000000,
		0x0080e020, 0x0080e060, 0x0080e0a0, 0x0080e0e0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00c02020, 0x00c02060, 0x00c020a0, 0x00c020e0, 0x00000000,
		0x00c06020, 0x00c06060, 0x00c060a0, 0x00c060e0, 0x00000000,
		0x00c0a020, 0x00c0a060, 0x00c0a0a0, 0x00c0a0e0, 0x00000000,
		0x00c0e020, 0x00c0e060, 0x00c0e0a0, 0x00c0e0e0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00ff2020, 0x00ff2060, 0x00ff20a0, 0x00ff20e0, 0x00000000,
		0x00ff6020, 0x00ff6060, 0x00ff60a0, 0x00ff60e0, 0x00000000,
		0x00ffa020, 0x00ffa060, 0x00ffa0a0, 0x00ffa0e0, 0x00000000,
		0x00ffe020, 0x00ffe060, 0x00ffe0a0, 0x00ffe0e0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,

		// SRAM4, num = 100
		0x00200000, 0x00200040, 0x00200080, 0x002000c0, 0x002000ff,
		0x00204000, 0x00204040, 0x00204080, 0x002040c0, 0x002040ff,
		0x00208000, 0x00208040, 0x00208080, 0x002080c0, 0x002080ff,
		0x0020c000, 0x0020c040, 0x0020c080, 0x0020c0c0, 0x0020c0ff,
		0x0020ff00, 0x0020ff40, 0x0020ff80, 0x0020ffc0, 0x0020ffff,
		0x00600000, 0x00600040, 0x00600080, 0x006000c0, 0x006000ff,
		0x00604000, 0x00604040, 0x00604080, 0x006040c0, 0x006040ff,
		0x00608000, 0x00608040, 0x00608080, 0x006080c0, 0x006080ff,
		0x0060c000, 0x0060c040, 0x0060c080, 0x0060c0c0, 0x0060c0ff,
		0x0060ff00, 0x0060ff40, 0x0060ff80, 0x0060ffc0, 0x0060ffff,
		0x00a00000, 0x00a00040, 0x00a00080, 0x00a000c0, 0x00a000ff,
		0x00a04000, 0x00a04040, 0x00a04080, 0x00a040c0, 0x00a040ff,
		0x00a08000, 0x00a08040, 0x00a08080, 0x00a080c0, 0x00a080ff,
		0x00a0c000, 0x00a0c040, 0x00a0c080, 0x00a0c0c0, 0x00a0c0ff,
		0x00a0ff00, 0x00a0ff40, 0x00a0ff80, 0x00a0ffc0, 0x00a0ffff,
		0x00e00000, 0x00e00040, 0x00e00080, 0x00e000c0, 0x00e000ff,
		0x00e04000, 0x00e04040, 0x00e04080, 0x00e040c0, 0x00e040ff,
		0x00e08000, 0x00e08040, 0x00e08080, 0x00e080c0, 0x00e080ff,
		0x00e0c000, 0x00e0c040, 0x00e0c080, 0x00e0c0c0, 0x00e0c0ff,
		0x00e0ff00, 0x00e0ff40, 0x00e0ff80, 0x00e0ffc0, 0x00e0ffff,

		// SRAM5, num = 100
		0x00200020, 0x00200060, 0x002000a0, 0x002000e0, 0x00000000,
		0x00204020, 0x00204060, 0x002040a0, 0x002040e0, 0x00000000,
		0x00208020, 0x00208060, 0x002080a0, 0x002080e0, 0x00000000,
		0x0020c020, 0x0020c060, 0x0020c0a0, 0x0020c0e0, 0x00000000,
		0x0020ff20, 0x0020ff60, 0x0020ffa0, 0x0020ffe0, 0x00000000,
		0x00600020, 0x00600060, 0x006000a0, 0x006000e0, 0x00000000,
		0x00604020, 0x00604060, 0x006040a0, 0x006040e0, 0x00000000,
		0x00608020, 0x00608060, 0x006080a0, 0x006080e0, 0x00000000,
		0x0060c020, 0x0060c060, 0x0060c0a0, 0x0060c0e0, 0x00000000,
		0x0060ff20, 0x0060ff60, 0x0060ffa0, 0x0060ffe0, 0x00000000,
		0x00a00020, 0x00a00060, 0x00a000a0, 0x00a000e0, 0x00000000,
		0x00a04020, 0x00a04060, 0x00a040a0, 0x00a040e0, 0x00000000,
		0x00a08020, 0x00a08060, 0x00a080a0, 0x00a080e0, 0x00000000,
		0x00a0c020, 0x00a0c060, 0x00a0c0a0, 0x00a0c0e0, 0x00000000,
		0x00a0ff20, 0x00a0ff60, 0x00a0ffa0, 0x00a0ffe0, 0x00000000,
		0x00e00020, 0x00e00060, 0x00e000a0, 0x00e000e0, 0x00000000,
		0x00e04020, 0x00e04060, 0x00e040a0, 0x00e040e0, 0x00000000,
		0x00e08020, 0x00e08060, 0x00e080a0, 0x00e080e0, 0x00000000,
		0x00e0c020, 0x00e0c060, 0x00e0c0a0, 0x00e0c0e0, 0x00000000,
		0x00e0ff20, 0x00e0ff60, 0x00e0ffa0, 0x00e0ffe0, 0x00000000,

		// SRAM6, num = 100
		0x00202000, 0x00202040, 0x00202080, 0x002020c0, 0x002020ff,
		0x00206000, 0x00206040, 0x00206080, 0x002060c0, 0x002060ff,
		0x0020a000, 0x0020a040, 0x0020a080, 0x0020a0c0, 0x0020a0ff,
		0x0020e000, 0x0020e040, 0x0020e080, 0x0020e0c0, 0x0020e0ff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00602000, 0x00602040, 0x00602080, 0x006020c0, 0x006020ff,
		0x00606000, 0x00606040, 0x00606080, 0x006060c0, 0x006060ff,
		0x0060a000, 0x0060a040, 0x0060a080, 0x0060a0c0, 0x0060a0ff,
		0x0060e000, 0x0060e040, 0x0060e080, 0x0060e0c0, 0x0060e0ff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00a02000, 0x00a02040, 0x00a02080, 0x00a020c0, 0x00a020ff,
		0x00a06000, 0x00a06040, 0x00a06080, 0x00a060c0, 0x00a060ff,
		0x00a0a000, 0x00a0a040, 0x00a0a080, 0x00a0a0c0, 0x00a0a0ff,
		0x00a0e000, 0x00a0e040, 0x00a0e080, 0x00a0e0c0, 0x00a0e0ff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00e02000, 0x00e02040, 0x00e02080, 0x00e020c0, 0x00e020ff,
		0x00e06000, 0x00e06040, 0x00e06080, 0x00e060c0, 0x00e060ff,
		0x00e0a000, 0x00e0a040, 0x00e0a080, 0x00e0a0c0, 0x00e0a0ff,
		0x00e0e000, 0x00e0e040, 0x00e0e080, 0x00e0e0c0, 0x00e0e0ff,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,

		// SRAM7, num = 100
		0x00202020, 0x00202060, 0x002020a0, 0x002020e0, 0x00000000,
		0x00206020, 0x00206060, 0x002060a0, 0x002060e0, 0x00000000,
		0x0020a020, 0x0020a060, 0x0020a0a0, 0x0020a0e0, 0x00000000,
		0x0020e020, 0x0020e060, 0x0020e0a0, 0x0020e0e0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00602020, 0x00602060, 0x006020a0, 0x006020e0, 0x00000000,
		0x00606020, 0x00606060, 0x006060a0, 0x006060e0, 0x00000000,
		0x0060a020, 0x0060a060, 0x0060a0a0, 0x0060a0e0, 0x00000000,
		0x0060e020, 0x0060e060, 0x0060e0a0, 0x0060e0e0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00a02020, 0x00a02060, 0x00a020a0, 0x00a020e0, 0x00000000,
		0x00a06020, 0x00a06060, 0x00a060a0, 0x00a060e0, 0x00000000,
		0x00a0a020, 0x00a0a060, 0x00a0a0a0, 0x00a0a0e0, 0x00000000,
		0x00a0e020, 0x00a0e060, 0x00a0e0a0, 0x00a0e0e0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00e02020, 0x00e02060, 0x00e020a0, 0x00e020e0, 0x00000000,
		0x00e06020, 0x00e06060, 0x00e060a0, 0x00e060e0, 0x00000000,
		0x00e0a020, 0x00e0a060, 0x00e0a0a0, 0x00e0a0e0, 0x00000000,
		0x00e0e020, 0x00e0e060, 0x00e0e0a0, 0x00e0e0e0, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}
	#endif
};

CTL_IME_ISP_LCA ctl_ime_lca_init = {
	.rf = {
		.edge_ker_size_sel = CTL_IME_ISP_EDG_KER_SIZE_3X3,
		.edge_th = {256, 384},
		.sr_ker_size_sel = CTL_IME_ISP_KER_SIZE_9X9,
		.mr_ker_size_sel = CTL_IME_ISP_KER_SIZE_15X9,
		.ctr_3x3_y_th = {2, 4, 6},
		.ctr_3x3_u_th = {4, 6, 8},
		.ctr_3x3_v_th = {4, 6, 8},
		.sr_y_wet = 32,
		.sr_uv_wet = 32,
		.er_y_wet = 8,
		.er_uv_wet = 8,
		.ss_region_y_th = {2, 4, 6, 8, 10},
		.ss_region_uv_th = {36, 64, 121, 196, 289},
		.edge_region_y_th = {2, 4, 6, 8, 10},
		.edge_region_uv_th = {9, 25, 64, 121, 196},
		.motion_region_y_th = {4, 8, 12, 16, 20},
		.motion_region_uv_th = {324, 576, 1089, 1764, 2601}, },
	.coring_gain = {
		.still_y_gain = {32, 32, 32},
		.motion_y_gain = {32, 32, 32},
		.still_u_gain = {2, 29, 31},
		.motion_u_gain = {1, 1, 1},
		.still_v_gain = {2, 29, 31},
		.motion_v_gain = {1, 1, 1}, },
	.cutout_coff = {
		.still_y_coff = {0, 0, 0},
		.motion_y_coff = {0, 0, 0},
		.still_u_coff = {1, 3, 3},
		.motion_u_coff = {1, 1, 1},
		.still_v_coff = {1, 3, 3},
		.motion_v_coff = {1, 1, 1}, },
	.final_y_out_wt = 8,
	.final_uv_out_wt = 32,
	.dbg = {
		.enable = FALSE,
		.ch_sel = CTL_IME_ISP_DBG_COLOR_MODE,
		.ch_ofs = 0, },
	.set_proc_location = CTL_IME_ISP_LCA_PROC_LOCATION_POST_3DNR,
};

CTL_IME_ISP_DBCS ctl_ime_dbcs_init = {
	.enable = TRUE,
	.op_mode = CTL_IME_ISP_DBCS_DK_MODE,
	.cent_u = 128,
	.cent_v = 128,
	.step_y = 0,
	.step_c = 0,
	.wt_y = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
	.wt_c = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
};

CTL_IME_ISP_TMNR ctl_ime_tmnr_init = {
	.enable = TRUE,
	.me_param = {
		.update_mode = CTL_IME_ISP_ME_UPDATE_RAND,
		.boundary_set = TRUE,
		.sad_shift = 0,
		.cost_blend = 15,
		.rand_bit_x = 3,
		.rand_bit_y = 3,
		.min_detail = 16,
		.sad_penalty = {64, 64, 0, 128, 128, 128, 200, 200},
		.switch_th = {5, 5, 6, 6, 7, 7, 8, 8},
		.switch_rto = 128,
		.detail_penalty = {4, 4, 3, 4, 4, 4, 4, 4},
		.probability = {0, 0, 0, 0, 1, 1, 1, 1}, },
	.md_param = {
		.sad_coefa = {0, 0, 0, 0, 0, 0, 0, 0},
		.sad_coefb = {0, 0, 0, 0, 0, 0, 0, 0},
		.sad_std = {0, 0, 0, 0, 0, 0, 0, 0},
		.fth = {8, 16}, },
	.md_roi_param = {
		.fth = {8, 16}, },
	.mc_param = {
		.sad_base = {0, 0, 0, 0, 0, 0, 0, 0},
		.sad_coefa = {0, 0, 0, 0, 0, 0, 0, 0},
		.sad_coefb = {0, 0, 0, 0, 0, 0, 0, 0},
		.sad_std = {0, 0, 0, 0, 0, 0, 0, 0},
		.fth = {8, 16}, },
	.mc_roi_param = {
		.fth = {8, 16}, },
	.ps_param = {
		.smart_roi_ctrl_en = FALSE,
		.mv_check_en = TRUE,
		.roi_mv_check_en = TRUE,
		.mv_info_mode = CTL_IME_ISP_MV_INFO_MODE_AVERAGE,
		.mv_th = 4,
		.roi_mv_th = 4,
		.mix_ratio = {16, 32},
		.ds_th = 6,
		.ds_th_roi = 6,
		.fs_th = 16383,
		.blur_eth = 0, },
	.nr_param = {
		.luma_ch_en = TRUE,
		.chroma_ch_en = TRUE,
		.center_wzeros_y = TRUE,
		.luma_residue_th = {1, 1, 1},
		.chroma_residue_th = 1,
		.freq_wet = {16, 16, 16, 16},
		.luma_wet = {16, 16, 16, 16, 16, 16, 16, 16},
		.pre_y_blur_str = CTL_IME_ISP_PRE_FILTER_Y_STR_2,
		.pf_str = 0,
		.pre_filter_str = {16, 32, 48, 64},
		.pre_filter_rto = {128, 64},
		.snr_str = {8, 16, 64},
		.tnr_str = {64, 16, 8},
		.snr_base_th = 256,
		.tnr_base_th = 256,
		.luma_3d_lut = {0, 6, 12, 24, 40, 76, 96, 112},
		.luma_3d_rto = {128, 64},
		.luma_comp_str = 0,
		.chroma_3d_lut = {0, 6, 12, 24, 40, 76, 96, 112},
		.chroma_3d_rto = {128, 64},
		.tf0_blur_str = {0, 0, 0},
		.tf0_blur_estr = 0,
		.tf0_y_str = {0, 0, 0},
		.tf0_c_str = {0, 0, 0},
		.u_tf0_md_th = 255,
		.v_tf0_md_th = 255,
		.c_tf0_ratio = {90, 180},
		.motion_sat_ratio = 255,
		.c_tf0_tprot_th = 1,
		.cshk_th = {64, 64, 64, 64, 64, 64, 64, 64},
		.cshk_val = {0, 0, 0, 0, 0, 0, 0, 0}, },
	.dbg_param = {
		.dbg_mv0 = FALSE,
		.dbg_mode = 0, },
	.fcvg_param = {
		.en = FALSE,
		.start_point = 4,
		.step_size = 1, }
};

CTL_IME_ISP_YCC_CVT ctl_ime_ycc_cvt_init = {
	.enable = TRUE,
	.cvt_sel = CTL_IME_ISP_YCC_CVT_BT709,
};

CTL_IME_ISP_SHARPEN ctl_ime_sharpen_init = {
	.shp_en = TRUE,
	.motion_bit_en = TRUE,
	.dbg_en = FALSE,
	.shp_src_sel = CTL_IME_ISP_SHP_EDG_WT_SRC_Y,
	.jnd_filt_sel = CTL_IME_ISP_SHP_JND_FILT_3x3,
	.weight_th = 2,
	.weight_gain = 255,
	.noise_lv = 25,
	.flat_th = 150,
	.edge_th = 250,
	.wet = 4,
	.noise_curve = {35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35},
	.ewg_curve = {0, 4, 8, 40, 128, 312, 648, 1200, 2016},
	.slope = 0,
	.flat_region_str = 32,
	.edge_region_str = 255,
	.motion_str = 128,
	.static_str = 255,
	.trans_str = 128,
	.blend_inv_gamma = 64,
	.coring_th = 0,
	.edge_str = 80,
	.bright_halo_clip = 128,
	.dark_halo_clip = 128,
};

CTL_IPP_ISP_IFE_VIG_CENT_RATIO ctrl_ipp_ife_vig_cent_ratio_init = {
	.ratio_base = 1000,
	.ch0 = {
		.x = 500,
		.y = 500
	},
	.ch1 = {
		.x = 500,
		.y = 500
	},
	.ch2 = {
		.x = 500,
		.y = 500
	},
	.ch3 = {
		.x = 500,
		.y = 500
	},
};

CTL_IPP_ISP_IFE_VA_WIN_SIZE_RATIO ctrl_ipp_ife_va_win_size_init = {
	.ratio_base = 1000,
	.winsz_ratio = {
		.w = 1000,
		.h = 1000 },
	.indep_roi_ratio[0] = {
		.x = 450,
		.y = 450,
		.w = 100,
		.h = 100 },
	.indep_roi_ratio[1] = {
		.x = 200,
		.y = 200,
		.w = 100,
		.h = 100 },
	.indep_roi_ratio[2] = {
		.x = 700,
		.y = 200,
		.w = 100,
		.h = 100 },
	.indep_roi_ratio[3] = {
		.x = 200,
		.y = 700,
		.w = 100,
		.h = 100 },
	.indep_roi_ratio[4] = {
		.x = 700,
		.y = 700,
		.w = 100,
		.h = 100 },
};

CTL_IPP_ISP_IPE_VA_WIN_SIZE_RATIO ctrl_ipp_ipe_va_win_size_init = {
	.ratio_base = 1000,
	.winsz_ratio = {
		.w = 1000,
		.h = 1000 },
	.indep_roi_ratio[0] = {
		.x = 450,
		.y = 450,
		.w = 100,
		.h = 100 },
	.indep_roi_ratio[1] = {
		.x = 200,
		.y = 200,
		.w = 100,
		.h = 100 },
	.indep_roi_ratio[2] = {
		.x = 700,
		.y = 200,
		.w = 100,
		.h = 100 },
	.indep_roi_ratio[3] = {
		.x = 200,
		.y = 700,
		.w = 100,
		.h = 100 },
	.indep_roi_ratio[4] = {
		.x = 700,
		.y = 700,
		.w = 100,
		.h = 100 },
};

CTL_IPP_ISP_IME_LCA_DBG_X_POS_RATIO ctrl_ipp_ime_lca_dbg_x_pos_init = {
	.ratio_base = 1000,
	.x_pos_ratio = 500,
};

KDRV_H26XENC_SPN kdrv_enc_post_sharpen_init = {
	.bEnable = TRUE,
	.ucConEng = 4,
	.usSlopConEng = 0,
	.ucBHC = 128,
	.ucDHC = 128,
	.ucEWT = 2,
	.ucEWG = 255,
	.ucEdgeSharpStr1 = 32,
	.ucCT = 0,
	.ucNL = 25,
	.ucBIG = 64,
	.usFlatTh = 150,
	.usEdgeTh = 250,
	.ucEdgeStr = 255,
	.ucTransitionStr = 128,
	.ucMotionStr = 128,
	.ucStaticStr = 255,
	.ucFlatStr = 32,
	.ucNC = {35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35},
	.usEWG = {0, 4, 8, 40, 128, 312, 648, 1200, 2016},
	.bShowSharpInfo = FALSE,
};

CTL_IPE_ISP_CADJ_FIXTH iq_effect_param_color_pencil = {
	TRUE,
	{0, 0, CTL_IPE_ISP_YCTH_REGISTER_VALUE, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, 128, 0},
	{0, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, 0, 0, 0, 0}
};

CTL_IPE_ISP_CADJ_FIXTH iq_effect_param_sketch = {
	TRUE,
	{0, 0, CTL_IPE_ISP_YCTH_REGISTER_VALUE, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, 128, 0},
	{0, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE,CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, 0, 0, 0, 0}
};

CTL_IPE_ISP_CADJ_FIXTH iq_effect_param_rock = {
	TRUE,
	{0, 0, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, CTL_IPE_ISP_YCTH_ORIGINAL_VALUE, 0, 0},
	{128, CTL_IPE_ISP_YCTH_REGISTER_VALUE, CTL_IPE_ISP_YCTH_REGISTER_VALUE, 112, 0},
	{0, 128, 255, 0, 255, 0, 255, CTL_IPE_ISP_YCTH_REGISTER_VALUE, CTL_IPE_ISP_YCTH_REGISTER_VALUE, 165, 128, 235, 128}
};

INT8 iq_effect_param_select_cc[IQ_UI_CCID_MAX_CNT][IQ_CCM_SATTAB_LEN] = {
	{    0,    0,    0,    0,    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},          //CCID_NO_EFFECT,
	{   99,   99,   99,    0,    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  99},          //CCID_ENHANCE_RED,
	{    0,    0,    0,   99,   99,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},          //CCID_ENHANCE_YELLOW,
	{    0,    0,    0,    0,    0,    0,    0,  99,  99,  99,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},          //CCID_ENHANCE_GREEN,
	{    0,    0,    0,    0,    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,  99,  99,  99,   0,   0,   0,   0,   0,   0,   0},          //CCID_ENHANCE_BLUE,
	{    0,    0,    0,    0,    0,    0,    0,   0,   0,   0,   0,   0,   0,  99,  99,   0,   0,   0,   0,   0,   0,   0,   0,   0},          //CCID_ENHANCE_CYAN,
	{    0,    0,   99,   99,    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},          //CCID_ENHANCE_ORANGE,
	{    0,    0,    0,    0,    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  99,  99,  99,   0},          //CCID_ENHANCE_MAGENTA,
	{    0,    0,    0, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,   0}, //CCID_REMOVAL_RED,
	{ -128, -128, -128,    0,    0, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128}, //CCID_REMOVAL_YELLOW,
	{ -128, -128, -128, -128, -128, -128, -128,    0,   0,   0, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128}, //CCID_REMOVAL_GREEN,
	{ -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,   0,   0,   0, -128, -128, -128, -128, -128, -128, -128}, //CCID_REMOVAL_BLUE,
	{ -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,   0,   0, -128, -128, -128, -128, -128, -128, -128, -128, -128}, //CCID_REMOVAL_CYAN,
	{ -128, -128,    0,    0, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128}, //CCID_REMOVAL_ORANGE,
	{ -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,   0,   0,   0, -128}, //CCID_REMOVAL_MAGENTA
};

UINT32 iq_effect_param_negative[IQ_GAMMA_LEN] = {
	1023, 1003, 967, 932, 899, 869, 835, 800, 768, 728,
	687, 648, 612, 576, 542, 510, 483, 467, 452, 437,
	422, 406, 385, 366, 346, 327, 309, 292, 278, 264,
	251, 237, 225, 212, 200, 188, 179, 170, 162, 153,
	145, 137, 128, 121, 113, 106, 100, 93, 88, 82,
	76, 70, 64, 59, 54, 49, 44, 38, 34, 31,
	29, 28, 26, 25, 23, 22, 20, 20, 19, 18,
	17, 17, 16, 16, 15, 14, 14, 13, 13, 12,
	12, 11, 11, 10, 10, 9, 9, 8, 8, 8,
	7, 7, 7, 7, 6, 6, 6, 6, 6, 5,
	5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
	3, 3, 3, 3, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 0, 0
};

UINT32 iq_effect_param_thermal_red[3][IQ_GAMMA_LEN] = {
	{ // R
		0, 24, 48, 72, 96, 120, 144, 168, 192, 216,
		240, 260, 288, 312, 336, 360, 384, 408, 432, 456,
		480, 504, 528, 552, 576, 600, 624, 648, 672, 696,
		720, 744, 768, 792, 816, 840, 864, 888, 912, 936,
		960, 984, 1008, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023
	},
	{ // G
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 24, 48, 72, 96, 120, 144, 168, 192,
		216, 240, 260, 288, 312, 336, 360, 384, 408, 432,
		456, 480, 504, 528, 552, 576, 600, 624, 648, 672,
		696, 720, 744, 768, 792, 816, 840, 864, 888, 912,
		936, 960, 984, 1008, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023
	},
	{ // B
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 24, 48, 72, 96, 120, 144,
		168, 192, 216, 240, 260, 288, 312, 336, 360, 384,
		408, 432, 456, 480, 504, 528, 552, 576, 600, 624,
		648, 672, 696, 720, 744, 768, 792, 816, 840, 864,
		888, 912, 936, 960, 984, 1008, 1023, 1023, 1023
	},
};

UINT32 iq_effect_param_thermal_color[3][IQ_GAMMA_LEN] = {
	{ // R
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 32,
		64, 96, 128, 160, 192, 224, 256, 288, 320, 352,
		384, 416, 448, 480, 512, 544, 576, 608, 640, 672,
		704, 736, 768, 800, 832, 864, 896, 928, 960, 992,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 992, 960, 928, 896, 864, 832, 800,
		768, 736, 704, 672, 640, 608, 576, 544, 512
	},
	{ // G
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 32, 64, 96, 128,
		160, 192, 224, 256, 288, 320, 352, 384, 416, 448,
		480, 512, 544, 576, 608, 640, 672, 704, 736, 768,
		800, 832, 864, 896, 928, 960, 992, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		992, 960, 928, 896, 864, 832, 800, 768, 736, 704,
		672, 640, 608, 576, 544, 512, 480, 448, 416, 384,
		352, 320, 288, 256, 224, 192, 160, 128, 96, 64,
		32, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{ // B
		512, 544, 576, 608, 640, 672, 704, 736, 768, 800,
		832, 864, 896, 928, 960, 992, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
		1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 992,
		960, 928, 896, 864, 832, 800, 768, 736, 704, 672,
		640, 608, 576, 544, 512, 480, 448, 416, 384, 352,
		320, 288, 256, 224, 192, 160, 128, 96, 64, 32,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0
	},
};

CTL_IPE_ISP_CCTRL iq_nnsc_param_enh_green_cctrl_ct = {
	// idx          0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23
	.hue_tab = {    0,   0,   0,   0,   0,   0,   0, -15, -30, -30, -15,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, //shift setting
	.sat_tab = {    0,   0,   0,   0,  64, 127, 127, 127, 127, 127,  64,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	.int_tab = {    0,   0,   0,   0,   5,  10,  10,  10,  10,  10,   5,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
};

CTL_IPE_ISP_CCTRL iq_nnsc_param_enh_skin_cctrl = {
	.edge_tab = { -48, -64, -64, -48,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, //shift setting
};

CTL_IPE_ISP_CCTRL iq_nnsc_param_enh_skin_white_cctrl_ct = {
	.int_tab = {    8,  15,  15,   8,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
};

CTL_IPE_ISP_CCTRL iq_nnsc_param_enh_skin_tan_cctrl_ct = {
	.int_tab = {   -8, -15, -15,  -8,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
};

CTL_IME_ISP_TMNR iq_tmnr_still = {
	.enable = TRUE,
	.me_param = {
		.update_mode = CTL_IME_ISP_ME_UPDATE_RAND,
		.boundary_set = TRUE,
		.sad_shift = 0,
		.cost_blend = 15,
		.rand_bit_x = 3,
		.rand_bit_y = 3,
		.min_detail = 16,
		.sad_penalty = {1023, 1023, 0, 1023, 1023, 1023, 1023, 1023},
		.switch_th = {15, 15, 0, 15, 15, 15, 15, 15},
		.switch_rto = 128,
		.detail_penalty = {4, 4, 3, 4, 4, 4, 4, 4},
		.probability = {0, 0, 0, 0, 1, 1, 1, 1}, },
	.md_param = {
		.sad_coefa = {63, 63, 63, 63, 63, 63, 63, 63},
		.sad_coefb = {0, 0, 0, 0, 0, 0, 0, 0},
		.sad_std = {0, 0, 0, 0, 0, 0, 0, 0},
		.fth = {63, 63}, },
	.md_roi_param = {
		.fth = {8, 16}, },
	.mc_param = {
		.sad_base = {63, 63, 63, 63, 63, 63, 63, 63},
		.sad_coefa = {0, 0, 0, 0, 0, 0, 0, 0},
		.sad_coefb = {0, 0, 0, 0, 0, 0, 0, 0},
		.sad_std = {0, 0, 0, 0, 0, 0, 0, 0},
		.fth = {63, 63}, },
	.mc_roi_param = {
		.fth = {8, 16}, },
	.ps_param = {
		.smart_roi_ctrl_en = FALSE,
		.mv_check_en = TRUE,
		.roi_mv_check_en = TRUE,
		.mv_info_mode = CTL_IME_ISP_MV_INFO_MODE_AVERAGE,
		.mv_th = 63,
		.roi_mv_th = 4,
		.mix_ratio = {16, 32},
		.ds_th = 31,
		.ds_th_roi = 6,
		.fs_th = 16383,
		.blur_eth = 0, },
	.nr_param = {
		.luma_ch_en = TRUE,
		.chroma_ch_en = TRUE,
		.center_wzeros_y = TRUE,
		.luma_residue_th = {1, 1, 1},
		.chroma_residue_th = 1,
		.freq_wet = {16, 16, 16, 16},
		.luma_wet = {16, 16, 16, 16, 16, 16, 16, 16},
		.pre_y_blur_str = CTL_IME_ISP_PRE_FILTER_Y_STR_2,
		.pf_str = 0,
		.pre_filter_str = {16, 32, 48, 64},
		.pre_filter_rto = {128, 64},
		.snr_str = {8, 16, 64},
		.tnr_str = {64, 16, 8},
		.snr_base_th = 256,
		.tnr_base_th = 256,
		.luma_3d_lut = {0, 16, 32, 48, 64, 80, 96, 112},
		.luma_3d_rto = {128, 64},
		.chroma_3d_lut = {0, 16, 32, 48, 64, 80, 96, 112},
		.chroma_3d_rto = {128, 64},
		.luma_comp_str = 0,
		.tf0_blur_str = {0, 0, 0},
		.tf0_blur_estr = 0,
		.tf0_y_str = {0, 0, 0},
		.tf0_c_str = {0, 0, 0},
		.u_tf0_md_th = 255,
		.v_tf0_md_th = 255,
		.c_tf0_ratio = {90, 180},
		.motion_sat_ratio = 255,
		.c_tf0_tprot_th = 1,
		.cshk_th = {64, 64, 64, 64, 64, 64, 64, 64},
		.cshk_val = {0, 0, 0, 0, 0, 0, 0, 0}, },
	.fcvg_param = {
		.en = FALSE,
		.start_point = 0,
		.step_size = 0, }
};

