#if defined(__FREERTOS)
#include <stdio.h>
#endif

#include "iq_alg.h"
#include "isp_dev.h"

#include "iq_dbg.h"
#include "iq_version.h"
#include "iqt_cfg.h"

static IQT_CFG iqt_cfg[IQT_CFG_ITEM_MAX_NUM] = {
//   section name                  key name                                              length
	{"INFO",                       "iq_version",                                              1     },

	{"OB",                         "enable",                                                  1     },
	{"OB",                         "mode",                                                    1     },
	{"OB_M",                       "cofs",                                            IQ_OB_LEN     },

	{"NR",                         "outl_enable",                                             1     },
	{"NR",                         "gbal_enable",                                             1     },
	{"NR",                         "filter_enable",                                           1     },
	{"NR",                         "lca_enable",                                              1     },
	{"NR",                         "dbcs_enable",                                             1     },
	{"NR",                         "mode",                                                    1     },
	{"NR_M",                       "outl_ord_protect_th",                                     1     },
	{"NR_M",                       "outl_ord_blend_w",                                        1     },
	{"NR_M",                       "outl_avg_mode",                                           1     },
	{"NR_M",                       "outl_sel",                                                1     },
	{"NR_M",                       "outl_bright_th",                             IQ_NR_OUTL_LEN     },
	{"NR_M",                       "outl_dark_th",                               IQ_NR_OUTL_LEN     },
	{"NR_M",                       "gbal_diff_th_str",                                        1     },
	{"NR_M",                       "gbal_edge_protect_th",                                    1     },
	{"NR_M",                       "gbal_str_luma_low_bnd",                                   1     },
	{"NR_M",                       "gbal_edge_luma_low_bnd",                                  1     },
	{"NR_M",                       "filter_th",                                    IQ_NR_TH_NUM     },
	{"NR_M",                       "filter_lut",                                   IQ_NR_TH_LUT     },
	{"NR_M",                       "filter_th_b",                                  IQ_NR_TH_NUM     },
	{"NR_M",                       "filter_lut_b",                                 IQ_NR_TH_LUT     },
	{"NR_M",                       "filter_th_1",                                  IQ_NR_TH_NUM     },
	{"NR_M",                       "filter_lut_1",                                 IQ_NR_TH_LUT     },
	{"NR_M",                       "filter_th_b_1",                                IQ_NR_TH_NUM     },
	{"NR_M",                       "filter_lut_b_1",                               IQ_NR_TH_LUT     },
	{"NR_M",                       "filter_th_2",                                  IQ_NR_TH_NUM     },
	{"NR_M",                       "filter_lut_2",                                 IQ_NR_TH_LUT     },
	{"NR_M",                       "filter_th_b_2",                                IQ_NR_TH_NUM     },
	{"NR_M",                       "filter_lut_b_2",                               IQ_NR_TH_LUT     },
	{"NR_M",                       "filter_blend_w",                                          1     },
	{"NR_M",                       "filter_clamp_th",                                         1     },
	{"NR_M",                       "filter_clamp_mul",                                        1     },
	{"NR_M",                       "lca_edge_th",                                             1     },
	{"NR_M",                       "lca_y_filter_level",                          IQ_NR_LCA_NUM     },
	{"NR_M",                       "lca_y_coring_gain",                           IQ_NR_LCA_NUM     },
	{"NR_M",                       "lca_y_coring_cutoff",                         IQ_NR_LCA_NUM     },
	{"NR_M",                       "lca_c_filter_level",                          IQ_NR_LCA_NUM     },
	{"NR_M",                       "lca_c_coring_gain",                           IQ_NR_LCA_NUM     },
	{"NR_M",                       "lca_c_coring_cutoff",                         IQ_NR_LCA_NUM     },
	{"NR_M",                       "dbcs_step_y",                                             1     },
	{"NR_M",                       "dbcs_step_c",                                             1     },
	{"NR",                         "dbcs_proc_location",                                      1     },

	{"CFA",                        "mode",                                                    1     },
	{"CFA_M",                      "edge_dth",                                                1     },
	{"CFA_M",                      "edge_dth2",                                               1     },
	{"CFA_M",                      "freq_th",                                                 1     },
	{"CFA_M",                      "fcs_weight",                                              1     },
	{"CFA_M",                      "fcs_strength",                               IQ_CFA_FCS_NUM     },

	{"RAW_VA",                     "pre_filter_enable",                                       1     },
	{"RAW_VA",                     "mode",                                                    1     },
	{"RAW_VA",                     "g1_h_filter_sel",                                         1     },
	{"RAW_VA",                     "g1_fir_sym_sel",                                          1     },
	{"RAW_VA",                     "g1_fir",                                                  5     },
	{"RAW_VA",                     "g2_h_filter_sel",                                         1     },
	{"RAW_VA",                     "g2_fir_sym_sel",                                          1     },
	{"RAW_VA",                     "g2_fir",                                                  5     },
	{"RAW_VA",                     "ldg",                                                     6     },
	{"RAW_VA",                     "energy_w",                                                1     },
	{"RAW_VA",                     "win_cnt_out_sel",                                         1     },
	{"RAW_VA",                     "high_luma_th",                                            1     },
	{"RAW_VA_0",                   "g1_iir",                                                  1     },
	{"RAW_VA_M",                   "g1_iir1",                                                 5     },
	{"RAW_VA_M",                   "g1_iir2",                                                 5     },
	{"RAW_VA_M",                   "g1_iir3",                                                 5     },
	{"RAW_VA_M",                   "g1_th",                                                   2     },
	{"RAW_VA_0",                   "g2_iir",                                                  1     },
	{"RAW_VA_M",                   "g2_iir1",                                                 5     },
	{"RAW_VA_M",                   "g2_iir2",                                                 5     },
	{"RAW_VA_M",                   "g2_iir3",                                                 5     },
	{"RAW_VA_M",                   "g2_th",                                                   2     },

	{"VA",                         "pre_filter_enable",                                       1     },
	{"VA",                         "mode",                                                    1     },
	{"VA",                         "g1_sym_sel",                                              1     },
	{"VA",                         "g1",                                                      5     },
	{"VA",                         "g2_sym_sel",                                              1     },
	{"VA",                         "g2",                                                      5     },
	{"VA",                         "ldg",                                                     6     },
	{"VA",                         "energy_w",                                                1     },
	{"VA",                         "win_cnt_out_sel",                                         1     },
	{"VA",                         "high_luma_th",                                            1     },
	{"VA_M",                       "g1_th",                                                   2     },
	{"VA_M",                       "g2_th",                                                   2     },

	{"TONE",                       "enable",                                                  1     },
	{"TONE",                       "mode",                                                    1     },
	{"TONE",                       "auto_sel",                                                1     },
	{"TONE",                       "tone_in_yv_blend_lut",             IQ_TONE_INPUT_YV_BLD_NUM     },
	{"TONE",                       "manual_lut_left",                          IQ_TONE_LEFT_NUM     },
	{"TONE",                       "manual_lut_right",                        IQ_TONE_RIGHT_NUM     },
	{"TONE",                       "auto_set0_level",                                         1     },
	{"TONE",                       "auto_set0_lut_left",                       IQ_TONE_LEFT_NUM     },
	{"TONE",                       "auto_set0_lut_right",                     IQ_TONE_RIGHT_NUM     },
	{"TONE",                       "auto_set1_level",                                         1     },
	{"TONE",                       "auto_set1_lut_left",                       IQ_TONE_LEFT_NUM     },
	{"TONE",                       "auto_set1_lut_right",                     IQ_TONE_RIGHT_NUM     },
	{"TONE",                       "auto_set2_level",                                         1     },
	{"TONE",                       "auto_set2_lut_left",                       IQ_TONE_LEFT_NUM     },
	{"TONE",                       "auto_set2_lut_right",                     IQ_TONE_RIGHT_NUM     },
	{"TONE",                       "auto_set3_level",                                         1     },
	{"TONE",                       "auto_set3_lut_left",                       IQ_TONE_LEFT_NUM     },
	{"TONE",                       "auto_set3_lut_right",                     IQ_TONE_RIGHT_NUM     },
	{"TONE",                       "auto_set4_level",                                         1     },
	{"TONE",                       "auto_set4_lut_left",                       IQ_TONE_LEFT_NUM     },
	{"TONE",                       "auto_set4_lut_right",                     IQ_TONE_RIGHT_NUM     },
	{"TONE_0",                     "lv",                                                      1     },
	{"TONE_0",                     "tone_level",                                              1     },

	{"GAMMA",                      "enable",                                                  1     },
	{"GAMMA",                      "mode",                                                    1     },
	{"GAMMA",                      "auto_sel",                                                1     },
	{"GAMMA",                      "manual_lut",                                   IQ_GAMMA_LEN     },
	{"GAMMA",                      "auto_set0_level",                                         1     },
	{"GAMMA",                      "auto_set0_lut",                                IQ_GAMMA_LEN     },
	{"GAMMA",                      "auto_set1_level",                                         1     },
	{"GAMMA",                      "auto_set1_lut",                                IQ_GAMMA_LEN     },
	{"GAMMA",                      "auto_set2_level",                                         1     },
	{"GAMMA",                      "auto_set2_lut",                                IQ_GAMMA_LEN     },
	{"GAMMA",                      "auto_set3_level",                                         1     },
	{"GAMMA",                      "auto_set3_lut",                                IQ_GAMMA_LEN     },
	{"GAMMA",                      "auto_set4_level",                                         1     },
	{"GAMMA",                      "auto_set4_lut",                                IQ_GAMMA_LEN     },
	{"GAMMA_M",                    "lv",                                                      1     },
	{"GAMMA_M",                    "gamma_level",                                             1     },

	{"CCM",                        "enable",                                                  1     },
	{"CCM",                        "mode",                                                    1     },
	{"CCM_M",                      "ct",                                                      1     },
	{"CCM_M",                      "coef",                                           IQ_CCM_LEN     },
	{"CCM_M",                      "hue_tab",                                 IQ_CCM_HUETAB_LEN     },
	{"CCM_M",                      "sat_tab",                                 IQ_CCM_SATTAB_LEN     },
	{"CCM_M",                      "int_tab",                                 IQ_CCM_INTTAB_LEN     },

	{"COLOR",                      "enable",                                                  1     },
	{"COLOR",                      "mode",                                                    1     },
	{"COLOR_M",                    "c_con",                                                   1     },
	{"COLOR_M",                    "fstab",                                  IQ_COLOR_FSTAB_LEN     },
	{"COLOR_M",                    "fdtab",                                  IQ_COLOR_FDTAB_LEN     },
	{"COLOR_M",                    "cconlut",                                 IQ_COLOR_CCON_LEN     },

	{"CONTRAST",                   "enable",                                                  1     },
	{"CONTRAST",                   "lce_enable",                                              1     },
	{"CONTRAST",                   "mode",                                                    1     },
	{"CONTRAST_M",                 "y_con",                                                   1     },
	{"CONTRAST_M",                 "lce_lum_wt_lut",                        IQ_CONTRAST_LCE_LEN     },

	{"EDGE",                       "enable",                                                  1     },
	{"EDGE",                       "mode",                                                    1     },
	{"EDGE",                       "th_overshoot",                                            1     },
	{"EDGE",                       "th_undershoot",                                           1     },
	{"EDGE",                       "blending_th",                                             1     },
	{"EDGE",                       "blending_low_luma_w",                                     1     },
	{"EDGE",                       "blending_high_luma_w",                                    1     },
	{"EDGE",                       "edge_map_lut",                              IQ_EDGE_LUT_LEN     },
	{"EDGE",                       "es_map_lut",                                IQ_EDGE_LUT_LEN     },
	{"EDGE",                       "edge_tab",                                  IQ_EDGE_HUE_NUM     },
	{"EDGE",                       "reduce_ratio",                                            1     },
	{"EDGE_M",                     "edge_enh",                                                2     },
	{"EDGE_M",                     "thin_freq",                                               1     },
	{"EDGE_M",                     "robust_freq",                                             1     },
	{"EDGE_M",                     "wt",                                                      2     },
	{"EDGE_M",                     "th_flat_low",                                             1     },
	{"EDGE_M",                     "th_flat_high",                                            1     },
	{"EDGE_M",                     "th_edge_low",                                             1     },
	{"EDGE_M",                     "th_edge_high",                                            1     },
	{"EDGE_M",                     "str_flat",                                                1     },
	{"EDGE_M",                     "str_edge",                                                1     },
	{"EDGE_M",                     "overshoot_str",                                           1     },
	{"EDGE_M",                     "undershoot_str",                                          1     },
	{"EDGE_M",                     "edge_map_th",                                             4     },
	{"EDGE_M",                     "es_map_th",                                               4     },
	{"EDGE_M",                     "dir_eng_blend_w",                                         1     },
	{"EDGE_M",                     "motion_str",                             IQ_EDGE_OBJECT_NUM     },

	{"3DNR",                       "enable",                                                  1     },
	{"3DNR",                       "fcvg_enable",                                             1     },
	{"3DNR",                       "mode",                                                    1     },
	{"3DNR_M",                     "pf_str",                                                  1     },
	{"3DNR_M",                     "cost_blend",                                              1     },
	{"3DNR_M",                     "sad_penalty",                           IQ_3DNR_PENALTY_LEN     },
	{"3DNR_M",                     "detail_penalty",                        IQ_3DNR_PENALTY_LEN     },
	{"3DNR_M",                     "switch_th",                              IQ_3DNR_SWITCH_LEN     },
	{"3DNR_M",                     "switch_rto",                                              1     },
	{"3DNR_M",                     "probability",                                             1     },
	{"3DNR_M",                     "sad_base",                                 IQ_3DNR_BASE_LEN     },
	{"3DNR_M",                     "sad_coefa",                               IQ_3DNR_COEFA_LEN     },
	{"3DNR_M",                     "sad_coefb",                               IQ_3DNR_COEFB_LEN     },
	{"3DNR_M",                     "sad_std",                                   IQ_3DNR_STD_LEN     },
	{"3DNR_M",                     "fth",                                        IQ_3DNR_TH_NUM     },
	{"3DNR_M",                     "mv_th",                                                   1     },
	{"3DNR_M",                     "mix_ratio",                                IQ_3DNR_NODE_NUM     },
	{"3DNR_M",                     "ds_th",                                                   1     },
	{"3DNR_M",                     "blur_eth",                                                1     },
	{"3DNR_M",                     "luma_residue_th",                        IQ_3DNR_OBJECT_NUM     },
	{"3DNR_M",                     "chroma_residue_th",                                       1     },
	{"3DNR_M",                     "tf0_blur_str",                           IQ_3DNR_OBJECT_NUM     },
	{"3DNR_M",                     "tf0_blur_estr",                                           1     },
	{"3DNR_M",                     "tf0_y_str",                              IQ_3DNR_OBJECT_NUM     },
	{"3DNR_M",                     "tf0_c_str",                              IQ_3DNR_OBJECT_NUM     },
	{"3DNR_M",                     "tf0_u_th",                                                1     },
	{"3DNR_M",                     "tf0_v_th",                                                1     },
	{"3DNR_M",                     "tf0_uv_ratio",                       IQ_3DNR_TF0_OBJECT_NUM     },
	{"3DNR_M",                     "pre_filter_str",                           IQ_3DNR_FREQ_NUM     },
	{"3DNR_M",                     "pre_filter_rto",                          IQ_3DNR_RATIO_NUM     },
	{"3DNR_M",                     "snr_base_th",                                             1     },
	{"3DNR_M",                     "tnr_base_th",                                             1     },
	{"3DNR_M",                     "freq_wet",                                 IQ_3DNR_FREQ_NUM     },
	{"3DNR_M",                     "luma_wet",                               IQ_3DNR_F3_LUT_LEN     },
	{"3DNR_M",                     "snr_str",                                IQ_3DNR_OBJECT_NUM     },
	{"3DNR_M",                     "tnr_str",                                IQ_3DNR_OBJECT_NUM     },
	{"3DNR_M",                     "luma_3d_lut",                            IQ_3DNR_F4_LUT_LEN     },
	{"3DNR_M",                     "luma_3d_rto",                             IQ_3DNR_RATIO_NUM     },
	{"3DNR_M",                     "chroma_3d_lut",                          IQ_3DNR_F4_LUT_LEN     },
	{"3DNR_M",                     "chroma_3d_rto",                           IQ_3DNR_RATIO_NUM     },
	{"3DNR_M",                     "luma_comp_str",                                           1     },
	{"3DNR_M",                     "fcvg_start_point",                                        1     },
	{"3DNR_M",                     "fcvg_step_size",                                          1     },
	{"3DNR_M",                     "motion_sat_ratio",                                        1     },
	{"3DNR_M",                     "cshk_th",                                  IQ_3DNR_LUMA_LEN     },
	{"3DNR_M",                     "cshk_val",                                 IQ_3DNR_LUMA_LEN     },
	{"3DNR_M",                     "dc_ratio0",                                               1     },
	{"3DNR_M",                     "dc_ratio1",                                               1     },

	{"DPC",                        "enable",                                                  1     },
	{"DPC",                        "bin_file",                                                1     },
	{"DPC",                        "def_same_ch_only_en",                                     1     },

	{"DPC_EXPAND",                 "enable",                                                  1     },
	{"DPC_EXPAND",                 "size",                                                    1     },
	{"DPC_EXPAND",                 "bin_file",                                                1     },

	{"SHADING",                    "ecs_enable",                                              1     },
	{"SHADING",                    "vig_enable",                                              1     },
	{"SHADING",                    "mode",                                                    1     },
	{"SHADING",                    "ecs_dthr_enable",                                         1     },
	{"SHADING",                    "ecs_smooth_l_m_ct_lower",                                 1     },
	{"SHADING",                    "ecs_smooth_l_m_ct_upper",                                 1     },
	{"SHADING",                    "ecs_smooth_m_h_ct_lower",                                 1     },
	{"SHADING",                    "ecs_smooth_m_h_ct_upper",                                 1     },
	{"SHADING",                    "ecs_bin_file",                                            1     },
	{"SHADING",                    "vig_center_x",                                            1     },
	{"SHADING",                    "vig_center_y",                                            1     },
	{"SHADING",                    "vig_reduce_th",                                           1     },
	{"SHADING",                    "vig_zero_th",                                             1     },
	{"SHADING",                    "vig_lut",                                IQ_SHADING_VIG_LEN     },

	{"SHADING_EXT",                "ecs_ext_0_bin_file",                                      1     },
	{"SHADING_EXT",                "ecs_ext_1_bin_file",                                      1     },
	{"SHADING_EXT",                "ecs_ext_2_bin_file",                                      1     },

	{"PFR",                        "enable",                                                  1     },
	{"PFR",                        "mode",                                                    1     },
	{"PFR",                        "luma_lut",                                  IQ_PFR_LUMA_LEN     },
	{"PFR",                        "set0_en",                                                 1     },
	{"PFR",                        "set0_color_u",                                            1     },
	{"PFR",                        "set0_color_v",                                            1     },
	{"PFR",                        "set1_en",                                                 1     },
	{"PFR",                        "set1_color_u",                                            1     },
	{"PFR",                        "set1_color_v",                                            1     },
	{"PFR",                        "set2_en",                                                 1     },
	{"PFR",                        "set2_color_u",                                            1     },
	{"PFR",                        "set2_color_v",                                            1     },
	{"PFR",                        "set3_en",                                                 1     },
	{"PFR",                        "set3_color_u",                                            1     },
	{"PFR",                        "set3_color_v",                                            1     },
	{"PFR_M",                      "pfr_strength",                                            1     },
	{"PFR_M",                      "luma_th",                                                 1     },

	{"WDR",                        "enable",                                                  1     },
	{"WDR",                        "mode",                                                    1     },
	{"WDR",                        "subimg_size_h",                                           1     },
	{"WDR",                        "subimg_size_v",                                           1     },
	{"WDR",                        "max_gain",                                                1     },
	{"WDR",                        "min_gain",                                                1     },
	{"WDR",                        "halo_ratio",                                              1     },
	{"WDR",                        "halo_slope",                                              1     },
	{"WDR",                        "fbc_ratio",                                               1     },
	{"WDR",                        "gain_protect_str",                                        1     },
	{"WDR",                        "lut_left",                                  IQ_WDR_LEFT_NUM     },
	{"WDR",                        "lut_right",                                IQ_WDR_RIGHT_NUM     },
	{"WDR_M",                      "strength",                                                1     },
	{"WDR_0",                      "level",                                                   1     },
	{"WDR_0",                      "strength_min",                                            1     },
	{"WDR_0",                      "strength_max",                                            1     },

	{"WDR_ENH",                    "enable",                                                  1     },
	{"WDR_ENH",                    "enh_ratio",                                  IQ_WDR_STR_LEN     },

	{"DEFOG",                      "enable",                                                  1     },
	{"DEFOG",                      "mode",                                                    1     },
	{"DEFOG",                      "outbld_local_en",                                         1     },
	{"DEFOG",                      "outbld_diff_wt",                                          1     },
	{"DEFOG",                      "min_diff_ratio",                                          1     },
	{"DEFOG_M",                    "fog_level",                                               1     },
	{"DEFOG_M",                    "fog_ratio",                                               1     },
	{"DEFOG_M",                    "gain_th",                                                 1     },
	{"DEFOG_M",                    "outbld_lum_wt",                     IQ_DEFOG_OUTPUT_BLD_LEN     },
	{"DEFOG_0",                    "dr_th",                                                   1     },
	{"DEFOG_0",                    "fog_level_max",                                           1     },
	{"DEFOG_0",                    "fog_ratio",                                               1     },
	{"DEFOG_0",                    "outbld_wt",                                               1     },

	{"SHDR",                       "nrs_enable",                                              1     },
	{"SHDR",                       "auto_ev_enable",                                          1     },
	{"SHDR",                       "mode",                                                    1     },
	{"SHDR",                       "fusion_nor_sel",                                          1     },
	{"SHDR",                       "fusion_l_nor_knee",                                       1     },
	{"SHDR",                       "fusion_l_nor_range",                                      1     },
	{"SHDR",                       "fusion_s_nor_knee",                                       1     },
	{"SHDR",                       "fusion_s_nor_range",                                      1     },
	{"SHDR",                       "fusion_dif_sel",                                          1     },
	{"SHDR",                       "fusion_l_dif_knee",                                       1     },
	{"SHDR",                       "fusion_l_dif_range",                                      1     },
	{"SHDR",                       "fusion_s_dif_knee",                                       1     },
	{"SHDR",                       "fusion_s_dif_range",                                      1     },
	{"SHDR",                       "fusion_lum_th",                                           1     },
	{"SHDR",                       "fusion_diff_w",                   IQ_SHDR_FUSION_DIFF_W_NUM     },
	{"SHDR",                       "fcurve_y_mean_sel",                                       1     },
	{"SHDR",                       "fcurve_yv_w",                                             1     },
	{"SHDR",                       "fcurve_y_w_lut",                     IQ_SHDR_FCURVE_Y_W_NUM     },
	{"SHDR",                       "fcurve_left_lut",                   IQ_SHDR_FCURVE_LEFT_NUM     },
	{"SHDR",                       "fcurve_right_lut",                 IQ_SHDR_FCURVE_RIGHT_NUM     },
	{"SHDR",                       "fcurve_end_lut",                     IQ_SHDR_FCURVE_END_NUM     },
	{"SHDR_M",                     "nrs_s_str",                             IQ_SHDR_NRS_STR_NUM     },

	{"COMPANDING",                 "decomp_kpx",                       IQ_DECOMPANDING_KNEE_NUM     },
	{"COMPANDING",                 "decomp_kpy",                       IQ_DECOMPANDING_KNEE_NUM     },
	{"COMPANDING",                 "decomp_gain",                      IQ_DECOMPANDING_KNEE_NUM     },
	{"COMPANDING",                 "decomp_sb",                        IQ_DECOMPANDING_KNEE_NUM     },
	{"COMPANDING",                 "comp_fcurve_l",                      IQ_COMPANDING_LEFT_NUM     },
	{"COMPANDING",                 "comp_fcurve_m",                    IQ_COMPANDING_MIDDLE_NUM     },
	{"COMPANDING",                 "comp_fcurve_r",                     IQ_COMPANDING_RIGHT_NUM     },
	{"COMPANDING",                 "comp_fcurve_ev_fmt",                                      1     },

	{"RGBIR",                      "enable",                                                  1     },
	{"RGBIR",                      "mode",                                                    1     },
	{"RGBIR",                      "manual_irsub_r_weight",                                   1     },
	{"RGBIR",                      "manual_irsub_g_weight",                                   1     },
	{"RGBIR",                      "manual_irsub_b_weight",                                   1     },
	{"RGBIR",                      "manual_ir_sat_gain",                                      1     },
	{"RGBIR",                      "auto_irsub_r_weight",                                     1     },
	{"RGBIR",                      "auto_irsub_g_weight",                                     1     },
	{"RGBIR",                      "auto_irsub_b_weight",                                     1     },
	{"RGBIR",                      "auto_irsub_reduce_th",                                    1     },
	{"RGBIR",                      "auto_night_mode_th",                                      1     },

	{"RGBIR_ENH",                  "enable",                                                  1     },
	{"RGBIR_ENH",                  "mode",                                                    1     },
	{"RGBIR_ENH",                  "min_ir_th",                                               1     },
	{"RGBIR_ENH_M",                "ir_th",                                                   1     },
	{"RGBIR_ENH_M",                "enh_ratio",                              IQ_GAIN_ID_MAX_NUM     },
	{"RGBIR_ENH_M",                "outl_rgbir_rb_w",                                         1     },
	{"RGBIR_ENH_M",                "outl_ord_rgbir_rb_w",                                     1     },

	{"POST_SHARPEN_1",             "enable",                                                  1     },
	{"POST_SHARPEN_1",             "mode",                                                    1     },
	{"POST_SHARPEN_1_M",           "noise_level",                                             1     },
	{"POST_SHARPEN_1_M",           "noise_curve",                     IQ_POST_SHARPEN_CURVE_NUM     },
	{"POST_SHARPEN_1_M",           "edge_weight_th",                                          1     },
	{"POST_SHARPEN_1_M",           "edge_weight_gain",                                        1     },
	{"POST_SHARPEN_1_M",           "th_flat",                                                 1     },
	{"POST_SHARPEN_1_M",           "th_edge",                                                 1     },
	{"POST_SHARPEN_1_M",           "flat_region_str",                                         1     },
	{"POST_SHARPEN_1_M",           "edge_region_str",                                         1     },
	{"POST_SHARPEN_1_M",           "motion_edge_w_str",                                       1     },
	{"POST_SHARPEN_1_M",           "tarnsition_edge_w_str",                                   1     },
	{"POST_SHARPEN_1_M",           "static_edge_w_str",                                       1     },
	{"POST_SHARPEN_1_M",           "coring_th",                                               1     },
	{"POST_SHARPEN_1_M",           "blend_inv_gamma",                                         1     },
	{"POST_SHARPEN_1_M",           "edge_filt_sel",                                           1     },
	{"POST_SHARPEN_1_M",           "sharp_str",                                               1     },
	{"POST_SHARPEN_1_M",           "bright_halo_clip",                                        1     },
	{"POST_SHARPEN_1_M",           "dark_halo_clip",                                          1     },

	{"POST_SHARPEN_2",             "enable",                                                  1     },
	{"POST_SHARPEN_2",             "mode",                                                    1     },
	{"POST_SHARPEN_2_M",           "noise_level",                                             1     },
	{"POST_SHARPEN_2_M",           "noise_curve",                     IQ_POST_SHARPEN_CURVE_NUM     },
	{"POST_SHARPEN_2_M",           "edge_weight_th",                                          1     },
	{"POST_SHARPEN_2_M",           "edge_weight_gain",                                        1     },
	{"POST_SHARPEN_2_M",           "th_flat",                                                 1     },
	{"POST_SHARPEN_2_M",           "th_edge",                                                 1     },
	{"POST_SHARPEN_2_M",           "flat_region_str",                                         1     },
	{"POST_SHARPEN_2_M",           "edge_region_str",                                         1     },
	{"POST_SHARPEN_2_M",           "motion_edge_w_str",                                       1     },
	{"POST_SHARPEN_2_M",           "tarnsition_edge_w_str",                                   1     },
	{"POST_SHARPEN_2_M",           "static_edge_w_str",                                       1     },
	{"POST_SHARPEN_2_M",           "coring_th",                                               1     },
	{"POST_SHARPEN_2_M",           "blend_inv_gamma",                                         1     },
	{"POST_SHARPEN_2_M",           "sharp_str",                                               1     },
	{"POST_SHARPEN_2_M",           "bright_halo_clip",                                        1     },
	{"POST_SHARPEN_2_M",           "dark_halo_clip",                                          1     },

	{"YCURVE",                     "enable",                                                  1     },
	{"YCURVE",                     "ycurve_lut",                                  IQ_YCURVE_LEN     },

	{"CST",                        "mode",                                                    1     },
	{"CST",                        "cst_coef",                                       IQ_CST_LEN     },
	{"CST",                        "y_ofs",                                                   1     },
	{"CST",                        "cb_ofs",                                                  1     },
	{"CST",                        "cr_ofs",                                                  1     },
	{"CST_M",                      "cstp_ratio",                                              1     },

	{"FPN",                        "enable",                                                  1     },
	{"FPN",                        "mode",                                                    1     },
	{"FPN_M",                      "buf_phyaddr",                                             1     },
	{"FPN_M",                      "gain",                                                    1     },

	{"3DCC",                       "enable",                                                  1     },
	{"3DCC",                       "mode",                                                    1     },
	{"3DCC",                       "3dcc_bin_file",                                           1     },

	{"3DCC_EXT",                   "3dcc_ext_0_bin_file",                                     1     },
	{"3DCC_EXT",                   "3dcc_ext_1_bin_file",                                     1     },
	{"3DCC_EXT",                   "3dcc_ext_2_bin_file",                                     1     },
	{"3DCC_EXT",                   "3dcc_ext_3_bin_file",                                     1     },
	{"3DCC_EXT",                   "3dcc_ext_4_bin_file",                                     1     },

	{"BNR",                        "enable",                                                  1     },
	{"BNR",                        "err_comp_en",                                             1     },
	{"BNR",                        "mode",                                                    1     },
	{"BNR_M",                      "prefilter_str",                                           1     },
	{"BNR_M",                      "err_comp_l",                          IQ_BNR_ERR_COMP_L_MAX     },
	{"BNR_M",                      "err_comp_r",                          IQ_BNR_ERR_COMP_R_MAX     },
	{"BNR_M",                      "err_sft",                                                 1     },
	{"BNR_M",                      "coef_a",                                                  1     },
	{"BNR_M",                      "coef_b",                                                  1     },
	{"BNR_M",                      "downsample_th1",                                          1     },
	{"BNR_M",                      "downsample_th2",                                          1     },
	{"BNR_M",                      "static_region",                                           1     },
	{"BNR_M",                      "transision_region",                                       1     },
	{"BNR_M",                      "motion_region",                                           1     },
	{"BNR_M",                      "residue_th",                                              1     },
	{"BNR_M",                      "md_th_l",                                    IQ_BNR_MD_TH_L     },
	{"BNR_M",                      "md_th_r",                                    IQ_BNR_MD_TH_R     },
	{"BNR_M",                      "md_base",                                                 1     },
	{"BNR_M",                      "md_k1",                                                   1     },
	{"BNR_M",                      "md_k2",                                                   1     },

	{"AIISP",                      "path_id",                                                 1     },
	{"AIISP",                      "version",                                                 1     },
	{"AIISP",                      "param_num",                                               1     },
	{"AIISP",                      "param_size",                                AIISP_PARAM_MAX     },
	{"AIISP",                      "param_name",                                             32     },
	{"AIISP",                      "mode",                                                    1     },
	{"AIISP_M",                    "enable",                                                  1     },
	{"AIISP_M",                    "effect",                                                  1     },
	{"AIISP_M",                    "param",                                     AIISP_PARAM_MAX     },
};

static char dpc_tbl_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char dpc_expand_tbl_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char ecs_tbl_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char ecs_tbl_ext_0_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char ecs_tbl_ext_1_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char ecs_tbl_ext_2_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char _3dcc_tbl_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char _3dcc_tbl_ext_0_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char _3dcc_tbl_ext_1_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char _3dcc_tbl_ext_2_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char _3dcc_tbl_ext_3_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};
static char _3dcc_tbl_ext_4_path[MAX_PATH_NAME_LENGTH] = {CHAR_NULL};

static SECTION_MAP iq_cfg_sec_map[MAX_SECTION_MAP_NUM] = {{ .sec_name[0] = CHAR_NULL, .name_len = 0, .start_pos = 0, .sec_len = 0 } };

CFG_FILE_FMT *iqt_cfg_open(INT8 *pfile_name)
{
	CFG_FILE_FMT *pcfg_file;
	CHAR cfg_buf[LINE_LEN];

	pcfg_file = isp_cfg_open(pfile_name, O_RDONLY);

	if (pcfg_file != NULL) {
		isp_cfg_build_section_map(pcfg_file, cfg_buf, iq_cfg_sec_map);
	}

	return pcfg_file;
}

void iqt_cfg_close(CFG_FILE_FMT *pcfg_file)
{
	isp_cfg_close(pcfg_file);
}

static ER iqt_cfg_parsing_param(CFG_FILE_FMT *pcfg_file, IQT_CFG *cfg, void *target_addr, IQT_CFG_DATA_TYPE data_type)
{
	INT8 cfg_buf[LINE_LEN];
	UINT32 count = 0;
	BOOL dbg_en = ((iq_dbg_get_dbg_mode(0) & IQ_DBG_CFG) ? TRUE : FALSE);

	if (isp_cfg_get_field_str(iq_cfg_sec_map, cfg->section_name, cfg->key_name, cfg_buf, pcfg_file, 0) > 0) {
		if (data_type == IQT_CFG_DATA_TYPE_UINT64) {
			count = isp_cfg_str2tab_u64(cfg_buf, (UINT64 *)target_addr, cfg->length);
		} else if (data_type == IQT_CFG_DATA_TYPE_INT64) {
			count = isp_cfg_str2tab_s64(cfg_buf, (INT64 *)target_addr, cfg->length);
		} else if (data_type == IQT_CFG_DATA_TYPE_UINT32) {
			count = isp_cfg_str2tab_u32(cfg_buf, (UINT32 *)target_addr, cfg->length);
		} else if (data_type == IQT_CFG_DATA_TYPE_INT32) {
			count = isp_cfg_str2tab_s32(cfg_buf, (INT32 *)target_addr, cfg->length);
		} else if (data_type == IQT_CFG_DATA_TYPE_UINT16) {
			count = isp_cfg_str2tab_u16(cfg_buf, (UINT16 *)target_addr, cfg->length);
		} else if (data_type == IQT_CFG_DATA_TYPE_INT16) {
			count = isp_cfg_str2tab_s16(cfg_buf, (INT16 *)target_addr, cfg->length);
		} else if (data_type == IQT_CFG_DATA_TYPE_UINT8) {
			count = isp_cfg_str2tab_u8(cfg_buf, (UINT8 *)target_addr, cfg->length);
		} else if (data_type == IQT_CFG_DATA_TYPE_INT8) {
			count = isp_cfg_str2tab_s8(cfg_buf, (INT8 *)target_addr, cfg->length);
		} else if (data_type == IQT_CFG_DATA_TYPE_CHAR) {
			count = isp_cfg_str2tab_char(cfg_buf, (CHAR *)target_addr, cfg->length);
		} else {
			PRINT_IQ(dbg_en, "%s data type not support (%d) \r\n", cfg->key_name, data_type);
			return E_SYS;
		}

		if (count != cfg->length) {
			PRINT_IQ(dbg_en, "%s(%d), key number not match (%d) \r\n", cfg->key_name, count, cfg->length);
			return E_PAR;
		}
	} else {
		PRINT_IQ(dbg_en, "miss section_name(%s) key_name(%s) \r\n", cfg->section_name, cfg->key_name);
		return E_NOEXS;
	}

	return E_OK;
}

static void iqt_cfg_clean_string(CHAR *str)
{
	UINT32 i;

	for (i = 0; i < MAX_PATH_NAME_LENGTH; i++) {
		str[i] = CHAR_NULL;
	}
}

static UINT32 iqt_cfg_parsing_bin(INT8 *pfile_path, INT8 *pdata_des, UINT32 data_size)
{
	UINT32 data_cnt = 0;
	CFG_FILE_FMT *pbin_file = isp_cfg_open(pfile_path, O_RDONLY);
	BOOL dbg_en = ((iq_dbg_get_dbg_mode(0) & IQ_DBG_CFG) ? TRUE : FALSE);

	if (pbin_file == NULL) {
		PRINT_IQ(dbg_en, "fail to read param bin: %s!! \r\n", (CHAR *)pfile_path);
	} else {
		data_cnt = vos_file_read(pbin_file->filp, pdata_des, data_size);
		if (data_cnt == 0) {
			PRINT_IQ(dbg_en, "%s: zero size for param bin file!! \r\n", (CHAR *)pfile_path);
		}
		isp_cfg_close(pbin_file);
	}

	return data_size;
}

void iqt_cfg_load(CFG_FILE_FMT *pcfg_file, IQALG_INFO *iq_info, void *param)
{
	IQ_PARAM_PTR *iq_param = (IQ_PARAM_PTR *)param;
	IQ_OB_TUNE_PARAM *ob_auto;
	IQ_NR_TUNE_PARAM *nr_auto;
	IQ_NR_EXT_TUNE_PARAM *nr_ext_auto;
	IQ_CFA_TUNE_PARAM *cfa_auto;
	IQ_RAW_VA_MANUAL_PARAM *raw_va_manual;
	IQ_RAW_VA_AUTO_PARAM *raw_va_auto;
	IQ_VA_TUNE_PARAM *va_auto;
	IQ_TONE_AUTO_PARAM *tone_auto;
	IQ_GAMMA_AUTO_PARAM *gamma_auto;
	IQ_CCM_AUTO_PARAM *ccm_auto;
	IQ_COLOR_TUNE_PARAM *color_auto;
	IQ_CONTRAST_TUNE_PARAM *contrast_auto;
	IQ_EDGE_TUNE_PARAM *edge_auto;
	IQ_EDGE_EXT_TUNE_PARAM *edge_ext_auto;
	IQ_3DNR_TUNE_PARAM *_3dnr_auto;
	IQ_3DNR_EXT_TUNE_PARAM *_3dnr_ext_auto;
	IQ_PFR_TUNE_PARAM *pfr_auto;
	IQ_WDR_AUTO_PARAM *wdr_auto;
	IQ_DEFOG_AUTO_PARAM *defog_auto;
	IQ_SHDR_TUNE_PARAM *shdr_auto;
	IQ_RGBIR_MANUAL_PARAM *rgbir_manual;
	IQ_RGBIR_AUTO_PARAM *rgbir_auto;
	IQ_POST_SHARPEN_1_TUNE_PARAM *post_sharpen_1_auto;
	IQ_POST_SHARPEN_2_TUNE_PARAM *post_sharpen_2_auto;
	IQ_RGBIR_ENH_MANUAL_PARAM *rgbir_enh_manual;
	IQ_RGBIR_ENH_AUTO_PARAM *rgbir_enh_auto;
	IQ_CST_TUNE_PARAM *cst_auto;
	IQ_FPN_TUNE_PARAM *fpn_auto;
	IQ_BNR_TUNE_PARAM *bnr_auto;
	IQ_AIISP_TUNE_PARAM *aiisp_auto;
	UINT32 i, j;
	UINT32 version = 0;
	UINT32 rd_bin_size;
	UINT32 temp[16] = {0};
	INT32 int_temp[16] = {0};
	BOOL dbg_en = ((iq_dbg_get_dbg_mode(0) & IQ_DBG_CFG) ? TRUE : FALSE);
	ER rt = E_OK, rt_temp = E_OK;

	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VERSION], &version, IQT_CFG_DATA_TYPE_UINT32);
	if (version == iq_get_version()) {
		PRINT_IQ(dbg_en, "iq cfg version: 0x%x \r\n", version);
	} else {
		DBG_WRN("iq cfg version not match. (cfg: 0x%X) (iq module: 0x%X) \r\n", version, iq_get_version());
	}

	// OB
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_OB_ENABLE], &(iq_param->ob->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_OB_MODE], &(iq_param->ob->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_OB_OFS].section_name, "OB_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_OB_OFS], &iq_param->ob->manual_param.cofs[0], IQT_CFG_DATA_TYPE_UINT32);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_OB_OFS].section_name, "OB_%d", (int)i);
		ob_auto = iq_param->ob->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_OB_OFS], &(ob_auto->cofs[0]), IQT_CFG_DATA_TYPE_UINT32);
	}

	// NR
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_ENABLE], &(iq_param->nr->outl_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_GBAL_ENABLE], &(iq_param->nr->gbal_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_ENABLE], &(iq_param->nr->filter_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_ENABLE], &(iq_param->nr->lca_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_DBCS_ENABLE], &(iq_param->nr->dbcs_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_MODE], &(iq_param->nr->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_DBCS_PROC_LOCATION], &(iq_param->nr->dbcs_proc_location), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_ORD_PROTECT_TH].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_ORD_BLEND_W].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_AVG_MODE].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_SEL].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_BRIGHT_TH].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_DARK_TH].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_GBAL_DIFF_TH_STR].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_GBAL_EDGE_PROTECT_TH].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_GBAL_STR_LUMA_LOW_BND].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_GBAL_EDGE_LUMA_LOW_BND].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_B].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_B].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_1].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_1].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_B_1].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_B_1].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_2].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_2].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_B_2].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_B_2].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_BLEND].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_CLAMP_TH].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_CLAMP_MUL].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_EDGE_TH].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_Y_FILTER_LEVEL].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_Y_CORING_GAIN].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_Y_CORING_CUTOFF].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_C_FILTER_LEVEL].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_C_CORING_GAIN].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_C_CORING_CUTOFF].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_DBCS_STEP_Y].section_name, "NR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_DBCS_STEP_C].section_name, "NR_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_ORD_PROTECT_TH], &(iq_param->nr->manual_param.outl_ord_protect_th), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_ORD_BLEND_W], &(iq_param->nr->manual_param.outl_ord_blend_w), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_AVG_MODE], &(iq_param->nr->manual_param.outl_avg_mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_SEL], &(iq_param->nr->manual_param.outl_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_BRIGHT_TH], &(iq_param->nr->manual_param.outl_bright_th[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_DARK_TH], &(iq_param->nr->manual_param.outl_dark_th[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_GBAL_DIFF_TH_STR], &(iq_param->nr->manual_param.gbal_diff_th_str), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_GBAL_EDGE_PROTECT_TH], &(iq_param->nr->manual_param.gbal_edge_protect_th), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_GBAL_STR_LUMA_LOW_BND], &(iq_param->nr->manual_param.gbal_str_luma_low_bnd), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_GBAL_EDGE_LUMA_LOW_BND], &(iq_param->nr->manual_param.gbal_edge_luma_low_bnd), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH], &(iq_param->nr->manual_param.filter_th[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT], &(iq_param->nr->manual_param.filter_lut[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_B], &(iq_param->nr->manual_param.filter_th_b[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_B], &(iq_param->nr->manual_param.filter_lut_b[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_1], &(iq_param->nr->ext_manual_param.filter_th_1[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_1], &(iq_param->nr->ext_manual_param.filter_lut_1[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_B_1], &(iq_param->nr->ext_manual_param.filter_th_b_1[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_B_1], &(iq_param->nr->ext_manual_param.filter_lut_b_1[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_2], &(iq_param->nr->ext_manual_param.filter_th_2[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_2], &(iq_param->nr->ext_manual_param.filter_lut_2[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_B_2], &(iq_param->nr->ext_manual_param.filter_th_b_2[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_B_2], &(iq_param->nr->ext_manual_param.filter_lut_b_2[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_BLEND], &(iq_param->nr->manual_param.filter_blend_w), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_CLAMP_TH], &(iq_param->nr->manual_param.filter_clamp_th), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_CLAMP_MUL], &(iq_param->nr->manual_param.filter_clamp_mul), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_EDGE_TH], &(iq_param->nr->manual_param.lca_edge_th), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_Y_FILTER_LEVEL], &(iq_param->nr->manual_param.lca_y_filter_level), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_Y_CORING_GAIN], &(iq_param->nr->manual_param.lca_y_coring_gain), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_Y_CORING_CUTOFF], &(iq_param->nr->manual_param.lca_y_coring_cutoff), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_C_FILTER_LEVEL], &(iq_param->nr->manual_param.lca_c_filter_level), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_C_CORING_GAIN], &(iq_param->nr->manual_param.lca_c_coring_gain), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_C_CORING_CUTOFF], &(iq_param->nr->manual_param.lca_c_coring_cutoff), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_DBCS_STEP_Y], &(iq_param->nr->manual_param.dbcs_step_y), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_DBCS_STEP_C], &(iq_param->nr->manual_param.dbcs_step_c), IQT_CFG_DATA_TYPE_UINT32);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_ORD_PROTECT_TH].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_ORD_BLEND_W].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_AVG_MODE].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_SEL].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_BRIGHT_TH].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_OUTL_DARK_TH].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_GBAL_DIFF_TH_STR].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_GBAL_EDGE_PROTECT_TH].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_GBAL_STR_LUMA_LOW_BND].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_GBAL_EDGE_LUMA_LOW_BND].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_B].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_B].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_1].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_1].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_B_1].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_B_1].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_2].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_2].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_TH_B_2].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_LUT_B_2].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_BLEND].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_CLAMP_TH].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_FILTER_CLAMP_MUL].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_EDGE_TH].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_Y_FILTER_LEVEL].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_Y_CORING_GAIN].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_Y_CORING_CUTOFF].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_C_FILTER_LEVEL].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_C_CORING_GAIN].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_LCA_C_CORING_CUTOFF].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_DBCS_STEP_Y].section_name, "NR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_NR_DBCS_STEP_C].section_name, "NR_%d", (int)i);
		nr_auto = iq_param->nr->auto_param + i;
		nr_ext_auto = iq_param->nr->ext_auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_ORD_PROTECT_TH], &(nr_auto->outl_ord_protect_th), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_ORD_BLEND_W], &(nr_auto->outl_ord_blend_w), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_AVG_MODE], &(nr_auto->outl_avg_mode), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_SEL], &(nr_auto->outl_sel), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_BRIGHT_TH], &(nr_auto->outl_bright_th[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_OUTL_DARK_TH], &(nr_auto->outl_dark_th[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_GBAL_DIFF_TH_STR], &(nr_auto->gbal_diff_th_str), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_GBAL_EDGE_PROTECT_TH], &(nr_auto->gbal_edge_protect_th), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_GBAL_STR_LUMA_LOW_BND], &(nr_auto->gbal_str_luma_low_bnd), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_GBAL_EDGE_LUMA_LOW_BND], &(nr_auto->gbal_edge_luma_low_bnd), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH], &(nr_auto->filter_th[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT], &(nr_auto->filter_lut[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_B], &(nr_auto->filter_th_b[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_B], &(nr_auto->filter_lut_b[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_1], &(nr_ext_auto->filter_th_1[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_1], &(nr_ext_auto->filter_lut_1[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_B_1], &(nr_ext_auto->filter_th_b_1[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_B_1], &(nr_ext_auto->filter_lut_b_1[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_2], &(nr_ext_auto->filter_th_2[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_2], &(nr_ext_auto->filter_lut_2[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_TH_B_2], &(nr_ext_auto->filter_th_b_2[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_LUT_B_2], &(nr_ext_auto->filter_lut_b_2[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_BLEND], &(nr_auto->filter_blend_w), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_CLAMP_TH], &(nr_auto->filter_clamp_th), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_FILTER_CLAMP_MUL], &(nr_auto->filter_clamp_mul), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_EDGE_TH], &(nr_auto->lca_edge_th), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_Y_FILTER_LEVEL], &(nr_auto->lca_y_filter_level), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_Y_CORING_GAIN], &(nr_auto->lca_y_coring_gain), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_Y_CORING_CUTOFF], &(nr_auto->lca_y_coring_cutoff), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_C_FILTER_LEVEL], &(nr_auto->lca_c_filter_level), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_C_CORING_GAIN], &(nr_auto->lca_c_coring_gain), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_LCA_C_CORING_CUTOFF], &(nr_auto->lca_c_coring_cutoff), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_DBCS_STEP_Y], &(nr_auto->dbcs_step_y), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_NR_DBCS_STEP_C], &(nr_auto->dbcs_step_c), IQT_CFG_DATA_TYPE_UINT32);
	}

	// CFA
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_MODE], &(iq_param->cfa->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_EDGE_DTH].section_name, "CFA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_EDGE_DTH2].section_name, "CFA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_FREQ_TH].section_name, "CFA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_FCS_WEIGHT].section_name, "CFA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_FCS_STRENGTH].section_name, "CFA_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_EDGE_DTH], &(iq_param->cfa->manual_param.edge_dth), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_EDGE_DTH2], &(iq_param->cfa->manual_param.edge_dth2), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_FREQ_TH], &(iq_param->cfa->manual_param.freq_th), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_FCS_WEIGHT], &(iq_param->cfa->manual_param.fcs_weight), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_FCS_STRENGTH], &(iq_param->cfa->manual_param.fcs_strength[0]), IQT_CFG_DATA_TYPE_UINT8);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_EDGE_DTH].section_name, "CFA_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_EDGE_DTH2].section_name, "CFA_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_FREQ_TH].section_name, "CFA_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_FCS_WEIGHT].section_name, "CFA_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CFA_FCS_STRENGTH].section_name, "CFA_%d", (int)i);
		cfa_auto = iq_param->cfa->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_EDGE_DTH], &(cfa_auto->edge_dth), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_EDGE_DTH2], &(cfa_auto->edge_dth2), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_FREQ_TH], &(cfa_auto->freq_th), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_FCS_WEIGHT], &(cfa_auto->fcs_weight), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CFA_FCS_STRENGTH], &(cfa_auto->fcs_strength[0]), IQT_CFG_DATA_TYPE_UINT8);
	}

	// RAW_VA
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_PRE_FILTER_ENABLE], &(iq_param->raw_va->pre_filter_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_MODE], &(iq_param->raw_va->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G1_H_FILTER_SEL], &(iq_param->raw_va->g1_h_filter_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G1_FIR_SYM_SEL], &(iq_param->raw_va->g1_fir_sym_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G1_FIR], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		iq_param->raw_va->g1_fir_tap_a = (UINT8)int_temp[0];
		iq_param->raw_va->g1_fir_tap_b = (INT8)int_temp[1];
		iq_param->raw_va->g1_fir_tap_c = (INT8)int_temp[2];
		iq_param->raw_va->g1_fir_tap_d = (INT8)int_temp[3];
		iq_param->raw_va->g1_fir_div = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G2_H_FILTER_SEL], &(iq_param->raw_va->g2_h_filter_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G2_FIR_SYM_SEL], &(iq_param->raw_va->g2_fir_sym_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G2_FIR], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		iq_param->raw_va->g2_fir_tap_a = (UINT8)int_temp[0];
		iq_param->raw_va->g2_fir_tap_b = (INT8)int_temp[1];
		iq_param->raw_va->g2_fir_tap_c = (INT8)int_temp[2];
		iq_param->raw_va->g2_fir_tap_d = (INT8)int_temp[3];
		iq_param->raw_va->g2_fir_div = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_LDG], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		iq_param->raw_va->ldg_low_th = (UINT8)temp[0];
		iq_param->raw_va->ldg_high_th = (UINT8)temp[1];
		iq_param->raw_va->ldg_low_gain = (UINT8)temp[2];
		iq_param->raw_va->ldg_high_gain = (UINT8)temp[3];
		iq_param->raw_va->ldg_low_slope = (UINT8)temp[4];
		iq_param->raw_va->ldg_high_slope = (UINT8)temp[5];
	} else {
		rt |= rt_temp;
	}
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_ENERGY_W], &(iq_param->raw_va->energy_w), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_CNT_SEL], &(iq_param->raw_va->win_cnt_out_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_HIGH_LUMA_TH], &(iq_param->raw_va->high_luma_th), IQT_CFG_DATA_TYPE_UINT8);
	// manual_param
	raw_va_manual = &(iq_param->raw_va->manual_param);
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G1_MANUAL_IIR1].section_name, "RAW_VA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G1_MANUAL_IIR2].section_name, "RAW_VA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G1_MANUAL_IIR3].section_name, "RAW_VA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G1_TH].section_name, "RAW_VA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G2_MANUAL_IIR1].section_name, "RAW_VA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G2_MANUAL_IIR2].section_name, "RAW_VA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G2_MANUAL_IIR3].section_name, "RAW_VA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G2_TH].section_name, "RAW_VA_M");
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G1_MANUAL_IIR1], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		raw_va_manual->g1_iir1_tap_a = (INT16)int_temp[0];
		raw_va_manual->g1_iir1_tap_b = (INT16)int_temp[1];
		raw_va_manual->g1_iir1_tap_e = (INT16)int_temp[2];
		raw_va_manual->g1_iir1_tap_f = (INT16)int_temp[3];
		raw_va_manual->g1_iir1_shift_bit = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G1_MANUAL_IIR2], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		raw_va_manual->g1_iir2_tap_a = (INT16)int_temp[0];
		raw_va_manual->g1_iir2_tap_b = (INT16)int_temp[1];
		raw_va_manual->g1_iir2_tap_e = (INT16)int_temp[2];
		raw_va_manual->g1_iir2_tap_f = (INT16)int_temp[3];
		raw_va_manual->g1_iir2_shift_bit = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G1_MANUAL_IIR3], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		raw_va_manual->g1_iir3_tap_a = (INT16)int_temp[0];
		raw_va_manual->g1_iir3_tap_b = (INT16)int_temp[1];
		raw_va_manual->g1_iir3_tap_e = (INT16)int_temp[2];
		raw_va_manual->g1_iir3_tap_f = (INT16)int_temp[3];
		raw_va_manual->g1_iir3_shift_bit = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G1_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		raw_va_manual->g1_th_l = (UINT16)temp[0];
		raw_va_manual->g1_th_u = (UINT16)temp[1];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G2_MANUAL_IIR1], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		raw_va_manual->g2_iir1_tap_a = (INT16)int_temp[0];
		raw_va_manual->g2_iir1_tap_b = (INT16)int_temp[1];
		raw_va_manual->g2_iir1_tap_e = (INT16)int_temp[2];
		raw_va_manual->g2_iir1_tap_f = (INT16)int_temp[3];
		raw_va_manual->g2_iir1_shift_bit = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G2_MANUAL_IIR2], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		raw_va_manual->g2_iir2_tap_a = (INT16)int_temp[0];
		raw_va_manual->g2_iir2_tap_b = (INT16)int_temp[1];
		raw_va_manual->g2_iir2_tap_e = (INT16)int_temp[2];
		raw_va_manual->g2_iir2_tap_f = (INT16)int_temp[3];
		raw_va_manual->g2_iir2_shift_bit = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G2_MANUAL_IIR3], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		raw_va_manual->g2_iir3_tap_a = (INT16)int_temp[0];
		raw_va_manual->g2_iir3_tap_b = (INT16)int_temp[1];
		raw_va_manual->g2_iir3_tap_e = (INT16)int_temp[2];
		raw_va_manual->g2_iir3_tap_f = (INT16)int_temp[3];
		raw_va_manual->g2_iir3_shift_bit = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G2_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		raw_va_manual->g2_th_l = (UINT16)temp[0];
		raw_va_manual->g2_th_u = (UINT16)temp[1];
	} else {
		rt |= rt_temp;
	}
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G1_AUTO_IIR].section_name, "RAW_VA_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G1_TH].section_name, "RAW_VA_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G2_AUTO_IIR].section_name, "RAW_VA_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_RAW_VA_G2_TH].section_name, "RAW_VA_%d", (int)i);
		raw_va_auto = iq_param->raw_va->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G1_AUTO_IIR], &(raw_va_auto->g1_iir), IQT_CFG_DATA_TYPE_UINT32);
		rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G1_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
		if (rt_temp == E_OK) {
			raw_va_auto->g1_th_l = (UINT16)temp[0];
			raw_va_auto->g1_th_u = (UINT16)temp[1];
		} else {
			rt |= rt_temp;
		}
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G2_AUTO_IIR], &(raw_va_auto->g2_iir), IQT_CFG_DATA_TYPE_UINT32);
		rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RAW_VA_G2_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
		if (rt_temp == E_OK) {
			raw_va_auto->g2_th_l = (UINT16)temp[0];
			raw_va_auto->g2_th_u = (UINT16)temp[1];
		} else {
			rt |= rt_temp;
		}
	}

	// VA
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_PRE_FILTER_ENABLE], &(iq_param->va->pre_filter_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_MODE], &(iq_param->va->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_G1_SYM_SEL], &(iq_param->va->g1_sym_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_G1], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		iq_param->va->g1_tap_a = (UINT8)int_temp[0];
		iq_param->va->g1_tap_b = (INT8)int_temp[1];
		iq_param->va->g1_tap_c = (INT8)int_temp[2];
		iq_param->va->g1_tap_d = (INT8)int_temp[3];
		iq_param->va->g1_div = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_G2_SYM_SEL], &(iq_param->va->g2_sym_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_G2], &(int_temp[0]), IQT_CFG_DATA_TYPE_INT32);
	if (rt_temp == E_OK) {
		iq_param->va->g2_tap_a = (UINT8)int_temp[0];
		iq_param->va->g2_tap_b = (INT8)int_temp[1];
		iq_param->va->g2_tap_c = (INT8)int_temp[2];
		iq_param->va->g2_tap_d = (INT8)int_temp[3];
		iq_param->va->g2_div = (UINT8)int_temp[4];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_LDG], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		iq_param->va->ldg_low_th = (UINT8)temp[0];
		iq_param->va->ldg_high_th = (UINT8)temp[1];
		iq_param->va->ldg_low_gain = (UINT8)temp[2];
		iq_param->va->ldg_high_gain = (UINT8)temp[3];
		iq_param->va->ldg_low_slope = (UINT8)temp[4];
		iq_param->va->ldg_high_slope = (UINT8)temp[5];
	} else {
		rt |= rt_temp;
	}
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_ENERGY_W], &(iq_param->va->energy_w), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_CNT_SEL], &(iq_param->va->win_cnt_out_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_HIGH_LUMA_TH], &(iq_param->va->high_luma_th), IQT_CFG_DATA_TYPE_UINT8);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_VA_G1_TH].section_name, "VA_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_VA_G2_TH].section_name, "VA_M");
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_G1_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		iq_param->va->manual_param.g1_th_l = (UINT16)temp[0];
		iq_param->va->manual_param.g1_th_u = (UINT16)temp[1];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_G2_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		iq_param->va->manual_param.g2_th_l = (UINT16)temp[0];
		iq_param->va->manual_param.g2_th_u = (UINT16)temp[1];
	} else {
		rt |= rt_temp;
	}
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_VA_G1_TH].section_name, "VA_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_VA_G2_TH].section_name, "VA_%d", (int)i);
		va_auto = iq_param->va->auto_param + i;
		rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_G1_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
		if (rt_temp == E_OK) {
			va_auto->g1_th_l = (UINT16)temp[0];
			va_auto->g1_th_u = (UINT16)temp[1];
		} else {
			rt |= rt_temp;
		}
		rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_VA_G2_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
		if (rt_temp == E_OK) {
			va_auto->g2_th_l = (UINT16)temp[0];
			va_auto->g2_th_u = (UINT16)temp[1];
		} else {
			rt |= rt_temp;
		}
	}

	// TONE
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_ENABLE], &(iq_param->tone->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_MODE], &(iq_param->tone->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SEL], &(iq_param->tone->auto_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_IN_YV_BLEND_LUT], &(iq_param->tone->tone_in_yv_blend_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_MANUAL_LUT_LEFT], &(iq_param->tone->manual_lut_left[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_MANUAL_LUT_RIGHT], &(iq_param->tone->manual_lut_right[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET0_LEVEL], &(iq_param->tone->auto_set0_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET0_LUT_LEFT], &(iq_param->tone->auto_set0_lut_left[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET0_LUT_RIGHT], &(iq_param->tone->auto_set0_lut_right[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET1_LEVEL], &(iq_param->tone->auto_set1_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET1_LUT_LEFT], &(iq_param->tone->auto_set1_lut_left[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET1_LUT_RIGHT], &(iq_param->tone->auto_set1_lut_right[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET2_LEVEL], &(iq_param->tone->auto_set2_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET2_LUT_LEFT], &(iq_param->tone->auto_set2_lut_left[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET2_LUT_RIGHT], &(iq_param->tone->auto_set2_lut_right[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET3_LEVEL], &(iq_param->tone->auto_set3_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET3_LUT_LEFT], &(iq_param->tone->auto_set3_lut_left[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET3_LUT_RIGHT], &(iq_param->tone->auto_set3_lut_right[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET4_LEVEL], &(iq_param->tone->auto_set4_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET4_LUT_LEFT], &(iq_param->tone->auto_set4_lut_left[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_AUTO_SET4_LUT_RIGHT], &(iq_param->tone->auto_set4_lut_right[0]), IQT_CFG_DATA_TYPE_UINT16);
	// auto_param
	for (i = 0; i < IQ_TONE_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_TONE_LV].section_name, "TONE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_TONE_LEVEL].section_name, "TONE_%d", (int)i);
		tone_auto = iq_param->tone->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_LV], &(tone_auto->lv), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_TONE_LEVEL], &(tone_auto->tone_level), IQT_CFG_DATA_TYPE_UINT32);
	}

	// GAMMA
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_ENABLE], &(iq_param->gamma->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_MODE], &(iq_param->gamma->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SEL], &(iq_param->gamma->auto_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_MANUAL_LUT], &(iq_param->gamma->manual_lut[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET0_LEVEL], &(iq_param->gamma->auto_set0_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET0_LUT], &(iq_param->gamma->auto_set0_lut[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET1_LEVEL], &(iq_param->gamma->auto_set1_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET1_LUT], &(iq_param->gamma->auto_set1_lut[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET2_LEVEL], &(iq_param->gamma->auto_set2_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET2_LUT], &(iq_param->gamma->auto_set2_lut[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET3_LEVEL], &(iq_param->gamma->auto_set3_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET3_LUT], &(iq_param->gamma->auto_set3_lut[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET4_LEVEL], &(iq_param->gamma->auto_set4_level), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_SET4_LUT], &(iq_param->gamma->auto_set4_lut[0]), IQT_CFG_DATA_TYPE_UINT32);
	// auto_param
	for (i = 0; i < IQ_GAMMA_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_GAMMA_AUTO_LV].section_name, "GAMMA_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_GAMMA_AUTO_GAMMA_LEVEL].section_name, "GAMMA_%d", (int)i);
		gamma_auto = iq_param->gamma->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_LV], &(gamma_auto->lv), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_GAMMA_AUTO_GAMMA_LEVEL], &(gamma_auto->gamma_level), IQT_CFG_DATA_TYPE_UINT32);
	}

	// CCM
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_ENABLE], &(iq_param->ccm->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_MODE], &(iq_param->ccm->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CCM_COEF].section_name, "CCM_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CCM_HUE_TAB].section_name, "CCM_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CCM_SAT_TAB].section_name, "CCM_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CCM_INT_TAB].section_name, "CCM_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_COEF], &(iq_param->ccm->manual_param.coef[0]), IQT_CFG_DATA_TYPE_INT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_HUE_TAB], &(iq_param->ccm->manual_param.hue_tab[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_SAT_TAB], &(iq_param->ccm->manual_param.sat_tab[0]), IQT_CFG_DATA_TYPE_INT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_INT_TAB], &(iq_param->ccm->manual_param.int_tab[0]), IQT_CFG_DATA_TYPE_INT8);
	// auto_param
	for (i = 0; i < IQ_COLOR_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CCM_CT].section_name, "CCM_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CCM_COEF].section_name, "CCM_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CCM_HUE_TAB].section_name, "CCM_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CCM_SAT_TAB].section_name, "CCM_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CCM_INT_TAB].section_name, "CCM_%d", (int)i);
		ccm_auto = iq_param->ccm->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_CT], &(ccm_auto->ct), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_COEF], &(ccm_auto->coef[0]), IQT_CFG_DATA_TYPE_INT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_HUE_TAB], &(ccm_auto->hue_tab[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_SAT_TAB], &(ccm_auto->sat_tab[0]), IQT_CFG_DATA_TYPE_INT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CCM_INT_TAB], &(ccm_auto->int_tab[0]), IQT_CFG_DATA_TYPE_INT8);
	}

	// COLOR
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_ENABLE], &iq_param->color->enable, IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_MODE], &iq_param->color->mode, IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_COLOR_C_CON].section_name, "COLOR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_COLOR_FSTAB].section_name, "COLOR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_COLOR_FDTAB].section_name, "COLOR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_COLOR_CCONLUT].section_name, "COLOR_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_C_CON], &(iq_param->color->manual_param.c_con), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_FSTAB], &(iq_param->color->manual_param.fstab[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_FDTAB], &(iq_param->color->manual_param.fdtab[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_CCONLUT], &(iq_param->color->manual_param.cconlut[0]), IQT_CFG_DATA_TYPE_UINT16);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_COLOR_C_CON].section_name, "COLOR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_COLOR_FSTAB].section_name, "COLOR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_COLOR_FDTAB].section_name, "COLOR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_COLOR_CCONLUT].section_name, "COLOR_%d", (int)i);
		color_auto = iq_param->color->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_C_CON], &(color_auto->c_con), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_FSTAB], &(color_auto->fstab[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_FDTAB], &(color_auto->fdtab[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COLOR_CCONLUT], &(color_auto->cconlut[0]), IQT_CFG_DATA_TYPE_UINT16);
	}

	// CONTRAST
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CONTRAST_ENABLE], &(iq_param->contrast->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CONTRAST_LCE_ENABLE], &(iq_param->contrast->lce_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CONTRAST_MODE], &(iq_param->contrast->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CONTRAST_Y_CON].section_name, "CONTRAST_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CONTRAST_LCE_LUM_WT_LUT].section_name, "CONTRAST_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CONTRAST_Y_CON], &(iq_param->contrast->manual_param.y_con), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CONTRAST_LCE_LUM_WT_LUT], &(iq_param->contrast->manual_param.lce_lum_wt_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CONTRAST_Y_CON].section_name, "CONTRAST_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_CONTRAST_LCE_LUM_WT_LUT].section_name, "CONTRAST_%d", (int)i);
		contrast_auto = iq_param->contrast->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CONTRAST_Y_CON], &(contrast_auto->y_con), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CONTRAST_LCE_LUM_WT_LUT], &(contrast_auto->lce_lum_wt_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
	}

	// EDGE
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_ENABLE], &(iq_param->edge->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_MODE], &(iq_param->edge->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_OVERSHOOT], &(iq_param->edge->th_overshoot), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_UNDERSHOOT], &(iq_param->edge->th_undershoot), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_BLENDING_TH], &(iq_param->edge->blending_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_BLENDING_LOW_LUMA_W], &(iq_param->edge->blending_low_luma_w), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_BLENDING_HIGH_LUMA_W], &(iq_param->edge->blending_high_luma_w), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_EDGE_MAP_LUT], &(iq_param->edge->edge_map_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_ES_MAP_LUT], &(iq_param->edge->es_map_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_EDGE_TAB], &(iq_param->edge->edge_tab[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_REDUCE_RATIO], &(iq_param->edge->reduce_ratio), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_ENH].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_THIN_FREQ].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_ROBUST_FREQ].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_WT].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_TH_FLAT_LOW].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_TH_FLAT_HIGH].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_TH_EDGE_LOW].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_TH_EDGE_HIGH].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_STR_FLAT].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_STR_EDGE].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_OVERSHOOT_STR].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_UNDERSHOOT_STR].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_EDGE_MAP_TH].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_ES_MAP_TH].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_DIR_ENG_BLEND_W].section_name, "EDGE_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_MOTION_STR].section_name, "EDGE_M");
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_ENH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		iq_param->edge->manual_param.edge_enh_p = temp[0];
		iq_param->edge->manual_param.edge_enh_n = temp[1];
	} else {
		rt |= rt_temp;
	}
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_THIN_FREQ], &(iq_param->edge->manual_param.thin_freq), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_ROBUST_FREQ], &(iq_param->edge->manual_param.robust_freq), IQT_CFG_DATA_TYPE_UINT32);
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_WT], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		iq_param->edge->manual_param.wt_low = temp[0];
		iq_param->edge->manual_param.wt_high = temp[1];
	} else {
		rt |= rt_temp;
	}
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_FLAT_LOW], &(iq_param->edge->manual_param.th_flat_low), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_FLAT_HIGH], &(iq_param->edge->manual_param.th_flat_high), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_EDGE_LOW], &(iq_param->edge->manual_param.th_edge_low), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_EDGE_HIGH], &(iq_param->edge->manual_param.th_edge_high), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_STR_FLAT], &(iq_param->edge->manual_param.str_flat), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_STR_EDGE], &(iq_param->edge->manual_param.str_edge), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_OVERSHOOT_STR], &(iq_param->edge->manual_param.overshoot_str), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_UNDERSHOOT_STR], &(iq_param->edge->manual_param.undershoot_str), IQT_CFG_DATA_TYPE_UINT32);
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_EDGE_MAP_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		iq_param->edge->manual_param.edge_ethr_low = temp[0];
		iq_param->edge->manual_param.edge_ethr_high = temp[1];
		iq_param->edge->manual_param.edge_etab_low = temp[2];
		iq_param->edge->manual_param.edge_etab_high = temp[3];
	} else {
		rt |= rt_temp;
	}
	rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_ES_MAP_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
	if (rt_temp == E_OK) {
		iq_param->edge->manual_param.es_ethr_low = temp[0];
		iq_param->edge->manual_param.es_ethr_high = temp[1];
		iq_param->edge->manual_param.es_etab_low = temp[2];
		iq_param->edge->manual_param.es_etab_high = temp[3];
	} else {
		rt |= rt_temp;
	}
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_DIR_ENG_BLEND_W], &(iq_param->edge->manual_param.dir_eng_blend_w), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_MOTION_STR], &(iq_param->edge->ext_manual_param.motion_str), IQT_CFG_DATA_TYPE_UINT8);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_ENH].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_THIN_FREQ].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_ROBUST_FREQ].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_WT].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_TH_FLAT_LOW].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_TH_FLAT_HIGH].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_TH_EDGE_LOW].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_TH_EDGE_HIGH].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_STR_FLAT].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_STR_EDGE].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_OVERSHOOT_STR].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_UNDERSHOOT_STR].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_EDGE_MAP_TH].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_ES_MAP_TH].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_DIR_ENG_BLEND_W].section_name, "EDGE_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_EDGE_MOTION_STR].section_name, "EDGE_%d", (int)i);
		edge_auto = iq_param->edge->auto_param + i;
		edge_ext_auto = iq_param->edge->ext_auto_param + i;
		rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_ENH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
		if (rt_temp == E_OK) {
			edge_auto->edge_enh_p = temp[0];
			edge_auto->edge_enh_n = temp[1];
		} else {
			rt |= rt_temp;
		}
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_THIN_FREQ], &(edge_auto->thin_freq), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_ROBUST_FREQ], &(edge_auto->robust_freq), IQT_CFG_DATA_TYPE_UINT32);
		rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_WT], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
		if (rt_temp == E_OK) {
			edge_auto->wt_low = temp[0];
			edge_auto->wt_high = temp[1];
		} else {
			rt |= rt_temp;
		}
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_FLAT_LOW], &(edge_auto->th_flat_low), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_FLAT_HIGH], &(edge_auto->th_flat_high), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_EDGE_LOW], &(edge_auto->th_edge_low), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_TH_EDGE_HIGH], &(edge_auto->th_edge_high), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_STR_FLAT], &(edge_auto->str_flat), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_STR_EDGE], &(edge_auto->str_edge), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_OVERSHOOT_STR], &(edge_auto->overshoot_str), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_UNDERSHOOT_STR], &(edge_auto->undershoot_str), IQT_CFG_DATA_TYPE_UINT32);
		rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_EDGE_MAP_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
		if (rt_temp == E_OK) {
			edge_auto->edge_ethr_low = temp[0];
			edge_auto->edge_ethr_high = temp[1];
			edge_auto->edge_etab_low = temp[2];
			edge_auto->edge_etab_high = temp[3];
		} else {
			rt |= rt_temp;
		}
		rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_ES_MAP_TH], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
		if (rt_temp == E_OK) {
			edge_auto->es_ethr_low = temp[0];
			edge_auto->es_ethr_high = temp[1];
			edge_auto->es_etab_low = temp[2];
			edge_auto->es_etab_high = temp[3];
		} else {
			rt |= rt_temp;
		}
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_DIR_ENG_BLEND_W], &(edge_auto->dir_eng_blend_w), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EDGE_MOTION_STR], &(edge_ext_auto->motion_str), IQT_CFG_DATA_TYPE_UINT8);
	}

	// 3DNR
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_ENABLE], &(iq_param->_3dnr->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_FCVG_ENABLE], &(iq_param->_3dnr->fcvg_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_MODE], &(iq_param->_3dnr->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_PF_STR].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_COST_BLEND].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_PENALITY].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_DETAIL_PENALITY].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SWITCH_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SWITCH_RTO].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_PROBABILITY].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_BASE].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_COEFA].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_COEFB].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_STD].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_FTH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_MV_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_MIX_RATIO].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_DS_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_BLUR_ETH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_RESIDUE_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CHROMA_RESIDUE_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_BLUR_STR].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_BLUR_ESTR].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_Y_STR].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_C_STR].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_U_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_V_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_UV_RATIO].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_PRE_FILTER_STR].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_PRE_FILTER_RTO].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_DC_RATIO0].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_DC_RATIO1].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SNR_BASE_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TNR_BASE_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_FREQ_WET].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_WET].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SNR_STR].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TNR_STR].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_3D_LUT].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_3D_RTO].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CHROMA_3D_LUT].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CHROMA_3D_RTO].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_COMP_STR].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_FCVG_START_POINT].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_FCVG_STEP_SIZE].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_MOTION_SAT_RATIO].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CSHK_TH].section_name, "3DNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CSHK_VAL].section_name, "3DNR_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_PF_STR], &(iq_param->_3dnr->manual_param.pf_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_COST_BLEND], &(iq_param->_3dnr->manual_param.cost_blend), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_PENALITY], &(iq_param->_3dnr->manual_param.sad_penalty[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_DETAIL_PENALITY], &(iq_param->_3dnr->manual_param.detail_penalty[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SWITCH_TH], &(iq_param->_3dnr->manual_param.switch_th[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SWITCH_RTO], &(iq_param->_3dnr->manual_param.switch_rto), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_PROBABILITY], &(iq_param->_3dnr->manual_param.probability), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_BASE], &(iq_param->_3dnr->manual_param.sad_base[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_COEFA], &(iq_param->_3dnr->manual_param.sad_coefa[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_COEFB], &(iq_param->_3dnr->manual_param.sad_coefb[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_STD], &(iq_param->_3dnr->manual_param.sad_std[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_FTH], &(iq_param->_3dnr->manual_param.fth[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_MV_TH], &(iq_param->_3dnr->manual_param.mv_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_MIX_RATIO], &(iq_param->_3dnr->manual_param.mix_ratio[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_DS_TH], &(iq_param->_3dnr->manual_param.ds_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_BLUR_ETH], &(iq_param->_3dnr->manual_param.blur_eth), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_RESIDUE_TH], &(iq_param->_3dnr->manual_param.luma_residue_th[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CHROMA_RESIDUE_TH], &(iq_param->_3dnr->manual_param.chroma_residue_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_BLUR_STR], &(iq_param->_3dnr->manual_param.tf0_blur_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_BLUR_ESTR], &(iq_param->_3dnr->manual_param.tf0_blur_estr), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_Y_STR], &(iq_param->_3dnr->manual_param.tf0_y_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_C_STR], &(iq_param->_3dnr->manual_param.tf0_c_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_U_TH], &(iq_param->_3dnr->manual_param.tf0_u_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_V_TH], &(iq_param->_3dnr->manual_param.tf0_v_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_UV_RATIO], &(iq_param->_3dnr->manual_param.tf0_uv_ratio), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_PRE_FILTER_STR], &(iq_param->_3dnr->manual_param.pre_filter_str[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_PRE_FILTER_RTO], &(iq_param->_3dnr->manual_param.pre_filter_rto[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SNR_BASE_TH], &(iq_param->_3dnr->manual_param.snr_base_th), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TNR_BASE_TH], &(iq_param->_3dnr->manual_param.tnr_base_th), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_FREQ_WET], &(iq_param->_3dnr->manual_param.freq_wet[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_WET], &(iq_param->_3dnr->manual_param.luma_wet[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SNR_STR], &(iq_param->_3dnr->manual_param.snr_str[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TNR_STR], &(iq_param->_3dnr->manual_param.tnr_str[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_3D_LUT], &(iq_param->_3dnr->manual_param.luma_3d_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_3D_RTO], &(iq_param->_3dnr->manual_param.luma_3d_rto[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CHROMA_3D_LUT], &(iq_param->_3dnr->manual_param.chroma_3d_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CHROMA_3D_RTO], &(iq_param->_3dnr->manual_param.chroma_3d_rto[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_COMP_STR], &(iq_param->_3dnr->manual_param.luma_comp_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_FCVG_START_POINT], &(iq_param->_3dnr->manual_param.fcvg_start_point), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_FCVG_STEP_SIZE], &(iq_param->_3dnr->manual_param.fcvg_step_size), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_MOTION_SAT_RATIO], &(iq_param->_3dnr->manual_param.motion_sat_ratio), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CSHK_TH], &(iq_param->_3dnr->manual_param.cshk_th), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CSHK_VAL], &(iq_param->_3dnr->manual_param.cshk_val), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_DC_RATIO0], &(iq_param->_3dnr->ext_manual_param.dc_ratio0), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_DC_RATIO1], &(iq_param->_3dnr->ext_manual_param.dc_ratio1), IQT_CFG_DATA_TYPE_UINT8);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_PF_STR].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_COST_BLEND].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_PENALITY].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_DETAIL_PENALITY].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SWITCH_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SWITCH_RTO].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_PROBABILITY].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_BASE].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_COEFA].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_COEFB].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SAD_STD].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_FTH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_MV_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_MIX_RATIO].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_DS_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_BLUR_ETH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_RESIDUE_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CHROMA_RESIDUE_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_BLUR_STR].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_BLUR_ESTR].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_Y_STR].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_C_STR].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_U_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_V_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TF0_UV_RATIO].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_PRE_FILTER_STR].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_PRE_FILTER_RTO].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_DC_RATIO0].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_DC_RATIO1].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SNR_BASE_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TNR_BASE_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_FREQ_WET].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_WET].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_SNR_STR].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_TNR_STR].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_3D_LUT].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_3D_RTO].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CHROMA_3D_LUT].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CHROMA_3D_RTO].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_LUMA_COMP_STR].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_FCVG_START_POINT].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_FCVG_STEP_SIZE].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_MOTION_SAT_RATIO].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CSHK_TH].section_name, "3DNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_3DNR_CSHK_VAL].section_name, "3DNR_%d", (int)i);
		_3dnr_auto = iq_param->_3dnr->auto_param + i;
		_3dnr_ext_auto = iq_param->_3dnr->ext_auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_PF_STR], &(_3dnr_auto->pf_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_COST_BLEND], &(_3dnr_auto->cost_blend), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_PENALITY], &(_3dnr_auto->sad_penalty[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_DETAIL_PENALITY], &(_3dnr_auto->detail_penalty[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SWITCH_TH], &(_3dnr_auto->switch_th[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SWITCH_RTO], &(_3dnr_auto->switch_rto), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_PROBABILITY], &(_3dnr_auto->probability), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_BASE], &(_3dnr_auto->sad_base[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_COEFA], &(_3dnr_auto->sad_coefa[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_COEFB], &(_3dnr_auto->sad_coefb[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SAD_STD], &(_3dnr_auto->sad_std[0]), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_FTH], &(_3dnr_auto->fth[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_MV_TH], &(_3dnr_auto->mv_th), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_MIX_RATIO], &(_3dnr_auto->mix_ratio[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_DS_TH], &(_3dnr_auto->ds_th), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_BLUR_ETH], &(_3dnr_auto->blur_eth), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_RESIDUE_TH], &(_3dnr_auto->luma_residue_th[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CHROMA_RESIDUE_TH], &(_3dnr_auto->chroma_residue_th), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_BLUR_STR], &(_3dnr_auto->tf0_blur_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_BLUR_ESTR], &(_3dnr_auto->tf0_blur_estr), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_Y_STR], &(_3dnr_auto->tf0_y_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_C_STR], &(_3dnr_auto->tf0_c_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_U_TH], &(_3dnr_auto->tf0_u_th), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_V_TH], &(_3dnr_auto->tf0_v_th), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TF0_UV_RATIO], &(_3dnr_auto->tf0_uv_ratio), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_PRE_FILTER_STR], &(_3dnr_auto->pre_filter_str[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_PRE_FILTER_RTO], &(_3dnr_auto->pre_filter_rto[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SNR_BASE_TH], &(_3dnr_auto->snr_base_th), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TNR_BASE_TH], &(_3dnr_auto->tnr_base_th), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_FREQ_WET], &(_3dnr_auto->freq_wet[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_WET], &(_3dnr_auto->luma_wet[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_SNR_STR], &(_3dnr_auto->snr_str[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_TNR_STR], &(_3dnr_auto->tnr_str[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_3D_LUT], &(_3dnr_auto->luma_3d_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_3D_RTO], &(_3dnr_auto->luma_3d_rto[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CHROMA_3D_LUT], &(_3dnr_auto->chroma_3d_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CHROMA_3D_RTO], &(_3dnr_auto->chroma_3d_rto[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_LUMA_COMP_STR], &(_3dnr_auto->luma_comp_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_FCVG_START_POINT], &(_3dnr_auto->fcvg_start_point), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_FCVG_STEP_SIZE], &(_3dnr_auto->fcvg_step_size), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_MOTION_SAT_RATIO], &(_3dnr_auto->motion_sat_ratio), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CSHK_TH], &(_3dnr_auto->cshk_th), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_CSHK_VAL], &(_3dnr_auto->cshk_val), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_DC_RATIO0], &(_3dnr_ext_auto->dc_ratio0), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DNR_DC_RATIO1], &(_3dnr_ext_auto->dc_ratio1), IQT_CFG_DATA_TYPE_UINT8);
	}

	// DPC
	if (iq_info->final_sie.sie_dpc != NULL) {
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DPC_ENABLE], &(iq_info->final_sie.sie_dpc->enable), IQT_CFG_DATA_TYPE_UINT32);
		iqt_cfg_clean_string(&dpc_tbl_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DPC_TABLE], &(dpc_tbl_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "dpc_tbl_path = %s \r\n", dpc_tbl_path);
		if (dpc_tbl_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)dpc_tbl_path, (INT8 *)iq_info->final_sie.sie_dpc->table, sizeof(UINT32) * IQ_DPC_MAX_NUM);
			if (rd_bin_size != sizeof(UINT32) * IQ_DPC_MAX_NUM) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[DPC] dpc_tbl_path size mis-match!! \r\n");
				}
			}
		}
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DPC_DEF_SAME_CH_ONLY_EN], &(iq_info->final_sie.sie_dpc->def_same_ch_only_en), IQT_CFG_DATA_TYPE_UINT32);
	}

	// DPC_EXPAND
	if (iq_info->final_sie.sie_dpc != NULL) {
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EXPAND_DPC_ENABLE], &(iq_info->final_sie.sie_dpc->expand_en), IQT_CFG_DATA_TYPE_UINT32);
		rt_temp = iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EXPAND_DPC_SIZE], &(temp[0]), IQT_CFG_DATA_TYPE_UINT32);
		if ((rt_temp == E_OK) && (iq_info->final_sie.sie_dpc->expand_table_viraddr != 0) && (iq_info->final_sie.sie_dpc->dp_buffer_size >= temp[0])) {
			iq_info->final_sie.sie_dpc->dp_total_size = temp[0];
			iqt_cfg_clean_string(&dpc_expand_tbl_path[0]);
			rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_EXPAND_DPC_TABLE], &(dpc_expand_tbl_path[0]), IQT_CFG_DATA_TYPE_CHAR);
			PRINT_IQ(dbg_en, "dpc_expand_tbl_path = %s \r\n", dpc_expand_tbl_path);
			if (dpc_expand_tbl_path[0] != CHAR_NULL) {
				rd_bin_size = iqt_cfg_parsing_bin((INT8 *)dpc_expand_tbl_path, (INT8 *)iq_info->final_sie.sie_dpc->expand_table_viraddr, iq_info->final_sie.sie_dpc->dp_total_size);
				if (rd_bin_size != iq_info->final_sie.sie_dpc->dp_total_size) {
					if (rd_bin_size != 0) {
						PRINT_IQ(dbg_en, "[DPC] dpc_expand_tbl_path size mis-match!! \r\n");
					}
				}
			}
		} else {
			if (rt_temp != E_OK) {
				rt |= rt_temp;
			} else if (iq_info->final_sie.sie_dpc->dp_buffer_size < temp[0]) {
				PRINT_IQ(dbg_en, "[DPC] dpc_expand_tbl_path size overflow!! buffer size %d, dpc_expand_tbl_path size %d \r\n", iq_info->final_sie.sie_dpc->dp_buffer_size, temp[0]);
			}
		}
		if (iq_info->final_sie.sie_dpc->expand_table_viraddr == 0) {
			iq_info->final_sie.sie_dpc->expand_en = FALSE;
			iq_info->final_sie.sie_dpc->dp_total_size = IQ_DPC_MAX_NUM * sizeof(UINT32);
			PRINT_IQ(dbg_en, "[DPC] expand_table_viraddr is not available, expand_en force to 0 !! \r\n");
		}
		if (iq_info->final_sie.sie_dpc->expand_en == FALSE) {
			iq_info->final_sie.sie_dpc->dp_total_size = IQ_DPC_MAX_NUM * sizeof(UINT32);
		}
	}

	// SHADING
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_MODE], &(iq_info->iq_ref_set.ecs_mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_ECS_L_M_CT_LOWER], &(iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_ECS_L_M_CT_UPPER], &(iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_ECS_M_H_CT_LOWER], &(iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_ECS_M_H_CT_UPPER], &(iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper), IQT_CFG_DATA_TYPE_UINT32);
	if (iq_info->iq_ref_set.ecs_ext != NULL) {
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_ECS_ENABLE], &(iq_info->final_sie.sie_ecs->enable), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_ECS_DTHR_ENABLE], &(iq_info->final_sie.sie_ecs->dthr_enable), IQT_CFG_DATA_TYPE_UINT32);
		iqt_cfg_clean_string(&ecs_tbl_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_ECS_MAP_TBL], &(ecs_tbl_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "ecs_tbl_path = %s \r\n", ecs_tbl_path);
		if (ecs_tbl_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)ecs_tbl_path, (INT8 *)iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl, sizeof(UINT32) * IQ_SHADING_ECS_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_SHADING_ECS_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[SHADING] ecs_map_tbl size mis-match!! \r\n");
				}
			}
		}
	}
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_VIG_ENABLE], &(iq_info->final_ipp.ife_vig.enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_VIG_CENTER_X], &(iq_info->final_ipp.ife_cent_ratio.ch0.x), IQT_CFG_DATA_TYPE_UINT32);
	iq_info->final_ipp.ife_cent_ratio.ch1.x = iq_info->final_ipp.ife_cent_ratio.ch0.x;
	iq_info->final_ipp.ife_cent_ratio.ch2.x = iq_info->final_ipp.ife_cent_ratio.ch0.x;
	iq_info->final_ipp.ife_cent_ratio.ch3.x = iq_info->final_ipp.ife_cent_ratio.ch0.x;
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_VIG_CENTER_Y], &(iq_info->final_ipp.ife_cent_ratio.ch0.y), IQT_CFG_DATA_TYPE_UINT32);
	iq_info->final_ipp.ife_cent_ratio.ch1.y = iq_info->final_ipp.ife_cent_ratio.ch0.y;
	iq_info->final_ipp.ife_cent_ratio.ch2.y = iq_info->final_ipp.ife_cent_ratio.ch0.y;
	iq_info->final_ipp.ife_cent_ratio.ch3.y = iq_info->final_ipp.ife_cent_ratio.ch0.y;
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_VIG_REDUCE_TH], &(iq_info->iq_ref_set.vig_reduce_th), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_VIG_ZERO_TH], &(iq_info->iq_ref_set.vig_zero_th), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_VIG_LUT], &(iq_info->iq_ref_set.vig_lut[0]), IQT_CFG_DATA_TYPE_UINT16);

	// SHADING EXT
	if (iq_info->iq_ref_set.ecs_ext != NULL) {
		iqt_cfg_clean_string(&ecs_tbl_ext_0_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_EXT_ECS_MAP_TBL_H], &(ecs_tbl_ext_0_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "ecs_tbl_ext_0_path = %s \r\n", ecs_tbl_ext_0_path);
		if (ecs_tbl_ext_0_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)ecs_tbl_ext_0_path, (INT8 *)iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_H], sizeof(UINT32) * IQ_SHADING_ECS_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_SHADING_ECS_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[SHADING] ecs_tbl_ext_0_path size mis-match!! \r\n");
				}
			}
		}
		iqt_cfg_clean_string(&ecs_tbl_ext_1_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_EXT_ECS_MAP_TBL_M], &(ecs_tbl_ext_1_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "ecs_tbl_ext_1_path = %s \r\n", ecs_tbl_ext_1_path);
		if (ecs_tbl_ext_1_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)ecs_tbl_ext_1_path, (INT8 *)iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_M], sizeof(UINT32) * IQ_SHADING_ECS_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_SHADING_ECS_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[SHADING] ecs_tbl_ext_1_path size mis-match!! \r\n");
				}
			}
		}
		iqt_cfg_clean_string(&ecs_tbl_ext_2_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHADING_EXT_ECS_MAP_TBL_L], &(ecs_tbl_ext_2_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "ecs_tbl_ext_2_path = %s \r\n", ecs_tbl_ext_2_path);
		if (ecs_tbl_ext_2_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)ecs_tbl_ext_2_path, (INT8 *)iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_L], sizeof(UINT32) * IQ_SHADING_ECS_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_SHADING_ECS_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[SHADING] ecs_tbl_ext_2_path size mis-match!! \r\n");
				}
			}
		}
	}

	// PFR
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_ENABLE], &(iq_param->pfr->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_MODE], &(iq_param->pfr->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_LUMA_LUT], &(iq_param->pfr->luma_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET0_EN], &(iq_param->pfr->set0_en), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET0_COLOR_U], &(iq_param->pfr->set0_color_u), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET0_COLOR_V], &(iq_param->pfr->set0_color_v), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET1_EN], &(iq_param->pfr->set1_en), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET1_COLOR_U], &(iq_param->pfr->set1_color_u), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET1_COLOR_V], &(iq_param->pfr->set1_color_v), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET2_EN], &(iq_param->pfr->set2_en), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET2_COLOR_U], &(iq_param->pfr->set2_color_u), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET2_COLOR_V], &(iq_param->pfr->set2_color_v), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET3_EN], &(iq_param->pfr->set3_en), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET3_COLOR_U], &(iq_param->pfr->set3_color_u), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_SET3_COLOR_V], &(iq_param->pfr->set3_color_v), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_PFR_STRENGTH].section_name, "PFR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_PFR_LUMA_TH].section_name, "PFR_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_STRENGTH], &(iq_param->pfr->manual_param.pfr_strength), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_LUMA_TH], &(iq_param->pfr->manual_param.luma_th), IQT_CFG_DATA_TYPE_UINT32);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_PFR_STRENGTH].section_name, "PFR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_PFR_LUMA_TH].section_name, "PFR_%d", (int)i);
		pfr_auto = iq_param->pfr->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_STRENGTH], &(pfr_auto->pfr_strength), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_PFR_LUMA_TH], &(pfr_auto->luma_th), IQT_CFG_DATA_TYPE_UINT32);
	}

	// WDR
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_ENABLE], &(iq_param->wdr->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_MODE], &(iq_param->wdr->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_SUBIMG_SIZE_H], &(iq_param->wdr->subimg_size_h), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_SUBIMG_SIZE_V], &(iq_param->wdr->subimg_size_v), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_MAX_GAIN], &(iq_param->wdr->max_gain), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_MIN_GAIN], &(iq_param->wdr->min_gain), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_HALO_RATIO], &(iq_param->wdr->halo_ratio), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_HALO_SLOPE], &(iq_param->wdr->halo_slope), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_FBC_RATIO], &(iq_param->wdr->fbc_ratio), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_GAIN_PROTECT_STR], &(iq_param->wdr->gain_protect_str), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_LUT_LEFT], &(iq_param->wdr->lut_left[0]), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_LUT_RIGHT], &(iq_param->wdr->lut_right[0]), IQT_CFG_DATA_TYPE_UINT16);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_WDR_MANUAL_STRENGTH].section_name, "WDR_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_MANUAL_STRENGTH], &(iq_param->wdr->manual_param.strength), IQT_CFG_DATA_TYPE_UINT32);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)(CHAR *)iqt_cfg[IQT_CFG_WDR_AUTO_LEVEL].section_name, "WDR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_WDR_AUTO_STRENGTH_MIN].section_name, "WDR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_WDR_AUTO_STRENGTH_MAX].section_name, "WDR_%d", (int)i);
		wdr_auto = iq_param->wdr->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_AUTO_LEVEL], &(wdr_auto->level), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_AUTO_STRENGTH_MIN], &(wdr_auto->strength_min), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_AUTO_STRENGTH_MAX], &(wdr_auto->strength_max), IQT_CFG_DATA_TYPE_UINT32);
	}

	// WDR_ENH
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_ENH_ENABLE], &(iq_param->wdr_enh->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_WDR_ENH_RATIO], &(iq_param->wdr_enh->enh_ratio[0]), IQT_CFG_DATA_TYPE_UINT32);

	// DEFOG
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_ENABLE], &(iq_param->defog->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_MODE], &(iq_param->defog->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_OUTBLD_LOCAL_EN], &(iq_param->defog->outbld_local_en), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_OUTBLD_DIFF_WT], &(iq_param->defog->outbld_diff_wt), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_MIN_DIFF_RATIO], &(iq_param->defog->min_diff_ratio), IQT_CFG_DATA_TYPE_UINT8);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_DEFOG_MANUAL_FOG_LEVEL].section_name, "DEFOG_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_DEFOG_MANUAL_FOG_RATIO].section_name, "DEFOG_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_DEFOG_MANUAL_GAIN_TH].section_name, "DEFOG_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_DEFOG_MANUAL_OUTBLD_LUM_WT].section_name, "DEFOG_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_MANUAL_FOG_LEVEL], &(iq_param->defog->manual_param.fog_level), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_MANUAL_FOG_RATIO], &(iq_param->defog->manual_param.fog_ratio), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_MANUAL_GAIN_TH], &(iq_param->defog->manual_param.gain_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_MANUAL_OUTBLD_LUM_WT], &(iq_param->defog->manual_param.outbld_lum_wt[0]), IQT_CFG_DATA_TYPE_UINT8);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_DEFOG_AUTO_DR_TH].section_name, "DEFOG_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_DEFOG_AUTO_FOG_LEVEL_MAX].section_name, "DEFOG_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_DEFOG_AUTO_FOG_RATIO].section_name, "DEFOG_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_DEFOG_AUTO_OUTBLD_WT].section_name, "DEFOG_%d", (int)i);
		defog_auto = iq_param->defog->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_AUTO_DR_TH], &(defog_auto->dr_th), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_AUTO_FOG_LEVEL_MAX], &(defog_auto->fog_level_max), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_AUTO_FOG_RATIO], &(defog_auto->fog_ratio), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DEFOG_AUTO_OUTBLD_WT], &(defog_auto->outbld_wt), IQT_CFG_DATA_TYPE_UINT8);
	}

	// SHDR
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_NRS_ENABLE], &(iq_param->shdr->nrs_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_AUTO_EV_ENABLE], &(iq_param->shdr->auto_ev_enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_MODE], &(iq_param->shdr->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_NOR_SEL], &(iq_param->shdr->fusion_nor_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_L_NOR_KNEE], &(iq_param->shdr->fusion_l_nor_knee), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_L_NOR_RANGE], &(iq_param->shdr->fusion_l_nor_range), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_S_NOR_KNEE], &(iq_param->shdr->fusion_s_nor_knee), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_S_NOR_RANGE], &(iq_param->shdr->fusion_s_nor_range), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_DIF_SEL], &(iq_param->shdr->fusion_dif_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_L_DIF_KNEE], &(iq_param->shdr->fusion_l_dif_knee), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_L_DIF_RANGE], &(iq_param->shdr->fusion_l_dif_range), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_S_DIF_KNEE], &(iq_param->shdr->fusion_s_dif_knee), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_S_DIF_RANGE], &(iq_param->shdr->fusion_s_dif_range), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_LUM_TH], &(iq_param->shdr->fusion_lum_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FUSION_DIFF_W], &(iq_param->shdr->fusion_diff_w[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FCURVE_Y_MEAN_SEL], &(iq_param->shdr->fcurve_y_mean_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FCURVE_YV_W], &(iq_param->shdr->fcurve_yv_w), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FCURVE_Y_W_LUT], &(iq_param->shdr->fcurve_y_w_lut[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FCURVE_LEFT_LUT], &(iq_param->shdr->fcurve_left_lut[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FCURVE_RIGHT_LUT], &(iq_param->shdr->fcurve_right_lut[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_FCURVE_END_LUT], &(iq_param->shdr->fcurve_end_lut[0]), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_SHDR_NRS_S_STR].section_name, "SHDR_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_NRS_S_STR], &(iq_param->shdr->manual_param.nrs_s_str[0]), IQT_CFG_DATA_TYPE_UINT16);
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_SHDR_NRS_S_STR].section_name, "SHDR_%d", (int)i);
		shdr_auto = iq_param->shdr->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_SHDR_NRS_S_STR], &(shdr_auto->nrs_s_str[0]), IQT_CFG_DATA_TYPE_UINT16);
	}

	// COMPANDING
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DECOMPANDING_KPX], &(iq_param->companding->decomp_kpx[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DECOMPANDING_KPY], &(iq_param->companding->decomp_kpy[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DECOMPANDING_GAIN], &(iq_param->companding->decomp_gain[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_DECOMPANDING_SB], &(iq_param->companding->decomp_sb[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COMPANDING_FCURVE_L], &(iq_param->companding->comp_fcurve_l[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COMPANDING_FCURVE_M], &(iq_param->companding->comp_fcurve_m[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COMPANDING_FCURVE_R], &(iq_param->companding->comp_fcurve_r[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_COMPANDING_FCURVE_EV_FMT], &(iq_param->companding->comp_fcurve_ev_fmt), IQT_CFG_DATA_TYPE_UINT32);

	// RGBIR
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENABLE], &(iq_param->rgbir->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_MODE], &(iq_param->rgbir->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	rgbir_manual = &iq_param->rgbir->manual_param;
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_MANUAL_IRSUB_R_WEIGHT], &(rgbir_manual->irsub_r_weight), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_MANUAL_IRSUB_G_WEIGHT], &(rgbir_manual->irsub_g_weight), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_MANUAL_IRSUB_B_WEIGHT], &(rgbir_manual->irsub_b_weight), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_MANUAL_IR_SAT_GAIN], &(rgbir_manual->ir_sat_gain), IQT_CFG_DATA_TYPE_UINT32);
	// auto_param
	rgbir_auto = &iq_param->rgbir->auto_param;
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_AUTO_IRSUB_R_WEIGHT], &(rgbir_auto->irsub_r_weight), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_AUTO_IRSUB_G_WEIGHT], &(rgbir_auto->irsub_g_weight), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_AUTO_IRSUB_B_WEIGHT], &(rgbir_auto->irsub_b_weight), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_AUTO_IRSUB_REDUCE_TH], &(rgbir_auto->irsub_reduce_th), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_AUTO_NIGHT_MODE_TH], &(rgbir_auto->night_mode_th), IQT_CFG_DATA_TYPE_UINT32);

	// RGBIR_ENH
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_ENABLE], &(iq_param->rgbir_enh->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_MODE], &(iq_param->rgbir_enh->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_MIN_IR_TH], &(iq_param->rgbir_enh->min_ir_th), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	rgbir_enh_manual = &iq_param->rgbir_enh->manual_param;
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RGBIR_ENH_RATIO].section_name, "RGBIR_ENH_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RGBIR_ENH_OUTL_RB_W].section_name, "RGBIR_ENH_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_RGBIR_ENH_OUTL_ORD_RB_W].section_name, "RGBIR_ENH_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_RATIO], &(rgbir_enh_manual->enh_ratio[0]), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_OUTL_RB_W], &(rgbir_enh_manual->outl_rgbir_rb_w), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_OUTL_ORD_RB_W], &(rgbir_enh_manual->outl_ord_rgbir_rb_w), IQT_CFG_DATA_TYPE_UINT8);
	// auto_param
	for (i = 0; i < IQ_RGBIR_LIGHT_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_RGBIR_ENH_IR_TH].section_name, "RGBIR_ENH_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_RGBIR_ENH_RATIO].section_name, "RGBIR_ENH_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_RGBIR_ENH_OUTL_RB_W].section_name, "RGBIR_ENH_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_RGBIR_ENH_OUTL_ORD_RB_W].section_name, "RGBIR_ENH_%d", (int)i);
		rgbir_enh_auto = iq_param->rgbir_enh->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_IR_TH], &(rgbir_enh_auto->ir_th), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_RATIO], &(rgbir_enh_auto->enh_ratio[0]), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_OUTL_RB_W], &(rgbir_enh_auto->outl_rgbir_rb_w), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_RGBIR_ENH_OUTL_ORD_RB_W], &(rgbir_enh_auto->outl_ord_rgbir_rb_w), IQT_CFG_DATA_TYPE_UINT8);
	}

	// POST_SHARPEN_1
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_ENABLE], &(iq_param->post_sharpen_1->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_MODE], &(iq_param->post_sharpen_1->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_NOISE_LEVEL].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_NOISE_CURVE].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_WEIGHT_TH].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_WEIGHT_GAIN].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_TH_FLAT].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_TH_EDGE].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_FLAT_REGION_STR].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_REGION_STR].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_MOTION_EDGE_W_STR].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_TRANS_EDGE_W_STR].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_STATIC_EDGE_W_STR].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_CORING_TH].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_BLEND_INV_GAMMA].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_FILT_SEL].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_SHARP_STR].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_BRIGHT_HALO_CLIP].section_name, "POST_SHARPEN_1_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_DARK_HALO_CLIP].section_name, "POST_SHARPEN_1_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_NOISE_LEVEL], &(iq_param->post_sharpen_1->manual_param.noise_level), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_NOISE_CURVE], &(iq_param->post_sharpen_1->manual_param.noise_curve[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_WEIGHT_TH], &(iq_param->post_sharpen_1->manual_param.edge_region_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_WEIGHT_GAIN], &(iq_param->post_sharpen_1->manual_param.edge_weight_gain), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_TH_FLAT], &(iq_param->post_sharpen_1->manual_param.th_flat), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_TH_EDGE], &(iq_param->post_sharpen_1->manual_param.th_edge), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_FLAT_REGION_STR], &(iq_param->post_sharpen_1->manual_param.flat_region_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_REGION_STR], &(iq_param->post_sharpen_1->manual_param.edge_region_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_MOTION_EDGE_W_STR], &(iq_param->post_sharpen_1->manual_param.motion_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_TRANS_EDGE_W_STR], &(iq_param->post_sharpen_1->manual_param.tarnsition_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_STATIC_EDGE_W_STR], &(iq_param->post_sharpen_1->manual_param.static_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_CORING_TH], &(iq_param->post_sharpen_1->manual_param.coring_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_BLEND_INV_GAMMA], &(iq_param->post_sharpen_1->manual_param.blend_inv_gamma), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_FILT_SEL], &(iq_param->post_sharpen_1->manual_param.edge_filt_sel), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_SHARP_STR], &(iq_param->post_sharpen_1->manual_param.sharp_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_BRIGHT_HALO_CLIP], &(iq_param->post_sharpen_1->manual_param.bright_halo_clip), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_DARK_HALO_CLIP], &(iq_param->post_sharpen_1->manual_param.dark_halo_clip), IQT_CFG_DATA_TYPE_UINT8);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_NOISE_LEVEL].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_NOISE_CURVE].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_WEIGHT_TH].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_WEIGHT_GAIN].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_TH_FLAT].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_TH_EDGE].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_FLAT_REGION_STR].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_REGION_STR].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_MOTION_EDGE_W_STR].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_TRANS_EDGE_W_STR].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_STATIC_EDGE_W_STR].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_CORING_TH].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_BLEND_INV_GAMMA].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_FILT_SEL].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_SHARP_STR].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_BRIGHT_HALO_CLIP].section_name, "POST_SHARPEN_1_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_1_DARK_HALO_CLIP].section_name, "POST_SHARPEN_1_%d", (int)i);
		post_sharpen_1_auto = iq_param->post_sharpen_1->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_NOISE_LEVEL], &(post_sharpen_1_auto->noise_level), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_NOISE_CURVE], &(post_sharpen_1_auto->noise_curve[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_WEIGHT_TH], &(post_sharpen_1_auto->edge_region_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_WEIGHT_GAIN], &(post_sharpen_1_auto->edge_weight_gain), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_TH_FLAT], &(post_sharpen_1_auto->th_flat), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_TH_EDGE], &(post_sharpen_1_auto->th_edge), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_FLAT_REGION_STR], &(post_sharpen_1_auto->flat_region_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_REGION_STR], &(post_sharpen_1_auto->edge_region_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_MOTION_EDGE_W_STR], &(post_sharpen_1_auto->motion_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_TRANS_EDGE_W_STR], &(post_sharpen_1_auto->tarnsition_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_STATIC_EDGE_W_STR], &(post_sharpen_1_auto->static_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_CORING_TH], &(post_sharpen_1_auto->coring_th), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_BLEND_INV_GAMMA], &(post_sharpen_1_auto->blend_inv_gamma), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_EDGE_FILT_SEL], &(post_sharpen_1_auto->edge_filt_sel), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_SHARP_STR], &(post_sharpen_1_auto->sharp_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_BRIGHT_HALO_CLIP], &(post_sharpen_1_auto->bright_halo_clip), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_1_DARK_HALO_CLIP], &(post_sharpen_1_auto->dark_halo_clip), IQT_CFG_DATA_TYPE_UINT8);
	}

	// POST_SHARPEN_2
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_ENABLE], &(iq_param->post_sharpen_2->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_MODE], &(iq_param->post_sharpen_2->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_NOISE_LEVEL].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_NOISE_CURVE].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_WEIGHT_TH].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_WEIGHT_GAIN].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_TH_FLAT].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_TH_EDGE].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_FLAT_REGION_STR].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_REGION_STR].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_MOTION_EDGE_W_STR].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_TRANS_EDGE_W_STR].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_STATIC_EDGE_W_STR].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_CORING_TH].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_BLEND_INV_GAMMA].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_SHARP_STR].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_BRIGHT_HALO_CLIP].section_name, "POST_SHARPEN_2_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_DARK_HALO_CLIP].section_name, "POST_SHARPEN_2_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_NOISE_LEVEL], &(iq_param->post_sharpen_2->manual_param.noise_level), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_NOISE_CURVE], &(iq_param->post_sharpen_2->manual_param.noise_curve[0]), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_WEIGHT_TH], &(iq_param->post_sharpen_2->manual_param.edge_region_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_WEIGHT_GAIN], &(iq_param->post_sharpen_2->manual_param.edge_weight_gain), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_TH_FLAT], &(iq_param->post_sharpen_2->manual_param.th_flat), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_TH_EDGE], &(iq_param->post_sharpen_2->manual_param.th_edge), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_FLAT_REGION_STR], &(iq_param->post_sharpen_2->manual_param.flat_region_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_REGION_STR], &(iq_param->post_sharpen_2->manual_param.edge_region_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_MOTION_EDGE_W_STR], &(iq_param->post_sharpen_2->manual_param.motion_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_TRANS_EDGE_W_STR], &(iq_param->post_sharpen_2->manual_param.tarnsition_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_STATIC_EDGE_W_STR], &(iq_param->post_sharpen_2->manual_param.static_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_CORING_TH], &(iq_param->post_sharpen_2->manual_param.coring_th), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_BLEND_INV_GAMMA], &(iq_param->post_sharpen_2->manual_param.blend_inv_gamma), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_SHARP_STR], &(iq_param->post_sharpen_2->manual_param.sharp_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_BRIGHT_HALO_CLIP], &(iq_param->post_sharpen_2->manual_param.bright_halo_clip), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_DARK_HALO_CLIP], &(iq_param->post_sharpen_2->manual_param.dark_halo_clip), IQT_CFG_DATA_TYPE_UINT8);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_NOISE_LEVEL].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_NOISE_CURVE].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_WEIGHT_TH].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_WEIGHT_GAIN].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_TH_FLAT].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_TH_EDGE].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_FLAT_REGION_STR].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_REGION_STR].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_MOTION_EDGE_W_STR].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_TRANS_EDGE_W_STR].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_STATIC_EDGE_W_STR].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_CORING_TH].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_BLEND_INV_GAMMA].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_SHARP_STR].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_BRIGHT_HALO_CLIP].section_name, "POST_SHARPEN_2_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_POST_SHARPEN_2_DARK_HALO_CLIP].section_name, "POST_SHARPEN_2_%d", (int)i);
		post_sharpen_2_auto = iq_param->post_sharpen_2->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_NOISE_LEVEL], &(post_sharpen_2_auto->noise_level), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_NOISE_CURVE], &(post_sharpen_2_auto->noise_curve[0]), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_WEIGHT_TH], &(post_sharpen_2_auto->edge_region_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_WEIGHT_GAIN], &(post_sharpen_2_auto->edge_weight_gain), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_TH_FLAT], &(post_sharpen_2_auto->th_flat), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_TH_EDGE], &(post_sharpen_2_auto->th_edge), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_FLAT_REGION_STR], &(post_sharpen_2_auto->flat_region_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_EDGE_REGION_STR], &(post_sharpen_2_auto->edge_region_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_MOTION_EDGE_W_STR], &(post_sharpen_2_auto->motion_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_TRANS_EDGE_W_STR], &(post_sharpen_2_auto->tarnsition_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_STATIC_EDGE_W_STR], &(post_sharpen_2_auto->static_edge_w_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_CORING_TH], &(post_sharpen_2_auto->coring_th), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_BLEND_INV_GAMMA], &(post_sharpen_2_auto->blend_inv_gamma), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_SHARP_STR], &(post_sharpen_2_auto->sharp_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_BRIGHT_HALO_CLIP], &(post_sharpen_2_auto->bright_halo_clip), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_POST_SHARPEN_2_DARK_HALO_CLIP], &(post_sharpen_2_auto->dark_halo_clip), IQT_CFG_DATA_TYPE_UINT8);
	}

	// YCURVE
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_YCURVE_ENABLE], &(iq_param->ycurve->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_YCURVE_LUT], &(iq_param->ycurve->ycurve_lut), IQT_CFG_DATA_TYPE_UINT32);

	// CST
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CST_MODE], &(iq_param->cst->mode), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CST_COEF], &(iq_param->cst->cst_coef[0]), IQT_CFG_DATA_TYPE_INT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CST_Y_OFS], &(iq_param->cst->y_ofs), IQT_CFG_DATA_TYPE_INT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CST_CB_OFS], &(iq_param->cst->cb_ofs), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CST_CR_OFS], &(iq_param->cst->cr_ofs), IQT_CFG_DATA_TYPE_UINT8);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_CST_CSTP_RATIO].section_name, "CST_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CST_CSTP_RATIO], &(iq_param->cst->manual_param.cstp_ratio), IQT_CFG_DATA_TYPE_UINT8);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
			sprintf((CHAR *)iqt_cfg[IQT_CFG_CST_CSTP_RATIO].section_name, "CST_%d", (int)i);
			cst_auto = iq_param->cst->auto_param + i;
			rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_CST_CSTP_RATIO], &(cst_auto->cstp_ratio), IQT_CFG_DATA_TYPE_UINT8);
	}

	// FPN
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_FPN_ENABLE], &(iq_param->fpn->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_FPN_MODE], &(iq_param->fpn->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_FPN_BUF_PHYADDR].section_name, "FPN_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_FPN_GAIN].section_name, "FPN_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_FPN_BUF_PHYADDR], &(iq_param->fpn->manual_param.buf_phyaddr), IQT_CFG_DATA_TYPE_UINT64);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_FPN_GAIN], &(iq_param->fpn->manual_param.gain), IQT_CFG_DATA_TYPE_UINT32);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
			sprintf((CHAR *)iqt_cfg[IQT_CFG_FPN_BUF_PHYADDR].section_name, "FPN_%d", (int)i);
			sprintf((CHAR *)iqt_cfg[IQT_CFG_FPN_GAIN].section_name, "FPN_%d", (int)i);
			fpn_auto = iq_param->fpn->auto_param + i;
			rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_FPN_BUF_PHYADDR], &(fpn_auto->buf_phyaddr), IQT_CFG_DATA_TYPE_UINT64);
			rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_FPN_GAIN], &(fpn_auto->gain), IQT_CFG_DATA_TYPE_UINT32);
	}

	// 3DCC
	if (iq_info->final_ipp.ipe_3dcc != NULL) {
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DCC_ENABLE], &(iq_info->final_ipp.ipe_3dcc->enable), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DCC_MODE], &(iq_info->final_ipp.ipe_3dcc->mode), IQT_CFG_DATA_TYPE_UINT32);
	}
	if (iq_info->iq_ref_set._3dcc_ext != NULL) {
		iqt_cfg_clean_string(&_3dcc_tbl_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DCC_MANUAL_LUT], &(_3dcc_tbl_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "_3dcc_tbl_path = %s \r\n", _3dcc_tbl_path);
		if (_3dcc_tbl_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)_3dcc_tbl_path, (INT8 *)&(iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut[0]), sizeof(UINT32) * IQ_3DCC_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_3DCC_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[3DCC] _3dcc_tbl_path size mis-match!! \r\n");
				}
			}
		}
	}

	// 3DCC EXT
	if (iq_info->iq_ref_set._3dcc_ext != NULL) {
		iqt_cfg_clean_string(&_3dcc_tbl_ext_0_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DCC_EXT_AUTO_LUT_0], &(_3dcc_tbl_ext_0_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "_3dcc_tbl_ext_0_path = %s \r\n", _3dcc_tbl_ext_0_path);
		if (_3dcc_tbl_ext_0_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)_3dcc_tbl_ext_0_path, (INT8 *)&(iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_0][0]), sizeof(UINT32) * IQ_3DCC_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_3DCC_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[3DCC] _3dcc_tbl_ext_0_path size mis-match!! \r\n");
				}
			}
		}
		iqt_cfg_clean_string(&_3dcc_tbl_ext_1_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DCC_EXT_AUTO_LUT_1], &(_3dcc_tbl_ext_1_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "_3dcc_tbl_ext_1_path = %s \r\n", _3dcc_tbl_ext_1_path);
		if (_3dcc_tbl_ext_1_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)_3dcc_tbl_ext_1_path, (INT8 *)&(iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_1][0]), sizeof(UINT32) * IQ_3DCC_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_3DCC_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[3DCC] _3dcc_tbl_ext_1_path size mis-match!! \r\n");
				}
			}
		}
		iqt_cfg_clean_string(&_3dcc_tbl_ext_2_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DCC_EXT_AUTO_LUT_2], &(_3dcc_tbl_ext_2_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "_3dcc_tbl_ext_2_path = %s \r\n", _3dcc_tbl_ext_2_path);
		if (_3dcc_tbl_ext_2_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)_3dcc_tbl_ext_2_path, (INT8 *)&(iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_2][0]), sizeof(UINT32) * IQ_3DCC_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_3DCC_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[3DCC] _3dcc_tbl_ext_2_path size mis-match!! \r\n");
				}
			}
		}
		iqt_cfg_clean_string(&_3dcc_tbl_ext_3_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DCC_EXT_AUTO_LUT_3], &(_3dcc_tbl_ext_3_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "_3dcc_tbl_ext_3_path = %s \r\n", _3dcc_tbl_ext_3_path);
		if (_3dcc_tbl_ext_3_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)_3dcc_tbl_ext_3_path, (INT8 *)&(iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_3][0]), sizeof(UINT32) * IQ_3DCC_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_3DCC_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[3DCC] _3dcc_tbl_ext_3_path size mis-match!! \r\n");
				}
			}
		}
		iqt_cfg_clean_string(&_3dcc_tbl_ext_4_path[0]);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_3DCC_EXT_AUTO_LUT_4], &(_3dcc_tbl_ext_4_path[0]), IQT_CFG_DATA_TYPE_CHAR);
		PRINT_IQ(dbg_en, "_3dcc_tbl_ext_4_path = %s \r\n", _3dcc_tbl_ext_4_path);
		if (_3dcc_tbl_ext_4_path[0] != CHAR_NULL) {
			rd_bin_size = iqt_cfg_parsing_bin((INT8 *)_3dcc_tbl_ext_4_path, (INT8 *)&(iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_4][0]), sizeof(UINT32) * IQ_3DCC_LEN);
			if (rd_bin_size != sizeof(UINT32) * IQ_3DCC_LEN) {
				if (rd_bin_size != 0) {
					PRINT_IQ(dbg_en, "[3DCC] _3dcc_tbl_ext_4_path size mis-match!! \r\n");
				}
			}
		}
	}

	//BNR
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_ENABLE], &(iq_param->bnr->enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_ERR_COMP_EN], &(iq_param->bnr->err_comp_en), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MODE], &(iq_param->bnr->mode), IQT_CFG_DATA_TYPE_UINT32);

	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_PREFILTER_STR].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_ERR_COMP_L].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_ERR_COMP_R].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_ERR_SFT].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_COEF_A].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_COEF_B].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_DOWNSAMPLE_TH1].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_DOWNSAMPLE_TH2].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_STATIC_REGION].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_TRANSISION_REGION].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MOTION_REGION].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_RESIDUE_TH].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_TH_L].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_TH_R].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_BASE].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_K1].section_name, "BNR_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_K2].section_name, "BNR_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_PREFILTER_STR], &(iq_param->bnr->manual_param.prefilter_str), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_ERR_COMP_L], &(iq_param->bnr->manual_param.err_comp_l), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_ERR_COMP_R], &(iq_param->bnr->manual_param.err_comp_r), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_ERR_SFT], &(iq_param->bnr->manual_param.err_sft), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_COEF_A], &(iq_param->bnr->manual_param.coef_a), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_COEF_B], &(iq_param->bnr->manual_param.coef_b), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_DOWNSAMPLE_TH1], &(iq_param->bnr->manual_param.downsample_th1), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_DOWNSAMPLE_TH2], &(iq_param->bnr->manual_param.downsample_th2), IQT_CFG_DATA_TYPE_UINT8);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_STATIC_REGION], &(iq_param->bnr->manual_param.static_region), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_TRANSISION_REGION], &(iq_param->bnr->manual_param.transision_region), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MOTION_REGION], &(iq_param->bnr->manual_param.motion_region), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_RESIDUE_TH], &(iq_param->bnr->manual_param.residue_th), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_TH_L], &(iq_param->bnr->manual_param.md_th_l), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_TH_R], &(iq_param->bnr->manual_param.md_th_r), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_BASE], &(iq_param->bnr->manual_param.md_base), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_K1], &(iq_param->bnr->manual_param.md_k1), IQT_CFG_DATA_TYPE_UINT16);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_K2], &(iq_param->bnr->manual_param.md_k2), IQT_CFG_DATA_TYPE_UINT16);

	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_PREFILTER_STR].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_ERR_COMP_L].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_ERR_COMP_R].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_ERR_SFT].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_COEF_A].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_COEF_B].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_DOWNSAMPLE_TH1].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_DOWNSAMPLE_TH2].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_STATIC_REGION].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_TRANSISION_REGION].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MOTION_REGION].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_RESIDUE_TH].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_TH_L].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_TH_R].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_BASE].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_K1].section_name, "BNR_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_BNR_MD_K2].section_name, "BNR_%d", (int)i);

		bnr_auto = iq_param->bnr->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_PREFILTER_STR], &(bnr_auto->prefilter_str), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_ERR_COMP_L], &(bnr_auto->err_comp_l), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_ERR_COMP_R], &(bnr_auto->err_comp_r), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_ERR_SFT], &(bnr_auto->err_sft), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_COEF_A], &(bnr_auto->coef_a), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_COEF_B], &(bnr_auto->coef_b), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_DOWNSAMPLE_TH1], &(bnr_auto->downsample_th1), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_DOWNSAMPLE_TH2], &(bnr_auto->downsample_th2), IQT_CFG_DATA_TYPE_UINT8);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_STATIC_REGION], &(bnr_auto->static_region), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_TRANSISION_REGION], &(bnr_auto->transision_region), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MOTION_REGION], &(bnr_auto->motion_region), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_RESIDUE_TH], &(bnr_auto->residue_th), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_TH_L], &(bnr_auto->md_th_l), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_TH_R], &(bnr_auto->md_th_r), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_BASE], &(bnr_auto->md_base), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_K1], &(bnr_auto->md_k1), IQT_CFG_DATA_TYPE_UINT16);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_BNR_MD_K2], &(bnr_auto->md_k2), IQT_CFG_DATA_TYPE_UINT16);
	}

	//AIISP
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_PATH_ID], &(iq_param->aiisp->path_id), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_VERSION], &(iq_param->aiisp->version), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_PARAM_NUM], &(iq_param->aiisp->param_num), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_PARAM_SIZE], &(iq_param->aiisp->param_size), IQT_CFG_DATA_TYPE_UINT32);
	for (i = 0; i < iq_param->aiisp->param_num; i++) {
		for (j = 0; j < sizeof(iq_param->aiisp->param_name[i]); j++) {
			iq_param->aiisp->param_name[i][j] = CHAR_NULL;
		}
		sprintf((CHAR *)iqt_cfg[IQT_CFG_AIISP_PARAM_NAME].key_name, "param_name_%d", (int)i);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_PARAM_NAME], &(iq_param->aiisp->param_name[i]), IQT_CFG_DATA_TYPE_CHAR);
	}
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_MODE], &(iq_param->aiisp->mode), IQT_CFG_DATA_TYPE_UINT32);
	// manual_param
	sprintf((CHAR *)iqt_cfg[IQT_CFG_AIISP_ENABLE].section_name, "AIISP_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_AIISP_EFFECT].section_name, "AIISP_M");
	sprintf((CHAR *)iqt_cfg[IQT_CFG_AIISP_PARAM].section_name, "AIISP_M");
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_ENABLE], &(iq_param->aiisp->manual_param.enable), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_EFFECT], &(iq_param->aiisp->manual_param.effect), IQT_CFG_DATA_TYPE_UINT32);
	rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_PARAM], &(iq_param->aiisp->manual_param.param), IQT_CFG_DATA_TYPE_UINT32);
	// auto_param
	for (i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
		sprintf((CHAR *)iqt_cfg[IQT_CFG_AIISP_ENABLE].section_name, "AIISP_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_AIISP_EFFECT].section_name, "AIISP_%d", (int)i);
		sprintf((CHAR *)iqt_cfg[IQT_CFG_AIISP_PARAM].section_name, "AIISP_%d", (int)i);

		aiisp_auto = iq_param->aiisp->auto_param + i;
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_ENABLE], &(aiisp_auto->enable), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_EFFECT], &(aiisp_auto->effect), IQT_CFG_DATA_TYPE_UINT32);
		rt |= iqt_cfg_parsing_param(pcfg_file, &iqt_cfg[IQT_CFG_AIISP_PARAM], &(aiisp_auto->param), IQT_CFG_DATA_TYPE_UINT32);
	}

	if (rt & E_NOEXS) {
		PRINT_IQ(dbg_en, "iq cfg miss settings \r\n");
		rt &= ~E_NOEXS;
	}
	if (rt != E_OK) {
		PRINT_IQ(dbg_en, "iq cfg parsing error!! \r\n");
	}
	return;
}
