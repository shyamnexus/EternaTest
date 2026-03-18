#ifndef _IQ_MSG_H_
#define _IQ_MSG_H_

//=============================================================================
// struct & definition
//=============================================================================
#if defined(__KERNEL__)
#include "linux/soc/nvt/rcw_macro.h"

#define IOREMAP(addr, size) ioremap(addr, size)
#define IOUNMAP(addr) iounmap(addr)
#define SETREG(value, addr) nvt_writel(value, addr)
#define GETREG(addr) nvt_readl(addr)
#else
#include "rcw_macro.h"

#define IOREMAP(addr, size) addr
#define IOUNMAP(addr)
#define SETREG(value, addr) OUTW(addr, value)
#define GETREG(addr) INW(addr)
#endif

#if defined(__KERNEL__)
#define SIE_BASE_ADDR   0x2F0310000
#define PRE_BASE_ADDR   0x2F0341000
#define IFE_BASE_ADDR   0x2F0340000
#define IPE_BASE_ADDR   0x2F0400000
#define IME_BASE_ADDR   0x2F0410000
#else
#define SIE_BASE_ADDR   0xF0310000
#define PRE_BASE_ADDR   0xF0341000
#define IFE_BASE_ADDR   0xF0340000
#define IPE_BASE_ADDR   0xF0400000
#define IME_BASE_ADDR   0xF0410000
#endif
#define SIE_SIZE        0xBEC
#define PRE_SIZE        0xEDC
#define IFE_SIZE        0xD88
#define IPE_SIZE        0x18BC
#define IME_SIZE        0xC8C

// SIE
#define R4_ENGINE_FUNCTION_OFS 0x0004
REGDEF_BEGIN(R4_ENGINE_FUNCTION)
REGDEF_BIT(,                           1)
REGDEF_BIT(PATGEN_EN,                  1)
REGDEF_BIT(DVI_EN,                     1)
REGDEF_BIT(OB_AVG_EN,                  1)
REGDEF_BIT(OB_SUB_SEL,                 1)
REGDEF_BIT(OB_BYPASS_EN,               1)
REGDEF_BIT(MASK0,                      1)
REGDEF_BIT(MASK1,                      1)
REGDEF_BIT(MASK2,                      1)
REGDEF_BIT(MASK3,                      1)
REGDEF_BIT(DVS_DECOMP_EN,              1)
REGDEF_BIT(DPC_EN,                     1)
REGDEF_BIT(,                           1)
REGDEF_BIT(PFPC_EN,                    1)
REGDEF_BIT(,                           1)
REGDEF_BIT(ECS_EN,                     1)
REGDEF_BIT(DGAIN_EN,                   1)
REGDEF_BIT(BS_H_EN,                    1)
REGDEF_BIT(BS_V_EN,                    1)
REGDEF_BIT(RAWENC_EN,                  1)
REGDEF_BIT(CGAIN_EN,                   1)
REGDEF_BIT(STCS_HISTO_Y_EN,            1)
REGDEF_BIT(,                           1)
REGDEF_BIT(STCS_LA_EN,                 1)
REGDEF_BIT(STCS_CA_EN,                 1)
REGDEF_BIT(STCS_DGAIN_EN,              1)
REGDEF_BIT(EMBDATA_EXTRACT_EN,         1)
REGDEF_BIT(DRAM_OUT0_EN,               1)
REGDEF_BIT(COMPANDING_EN,              1)
REGDEF_BIT(BAYER_FORMAT,               2)
REGDEF_BIT(ECS_BAYER_MODE,             1)
REGDEF_END(R4_ENGINE_FUNCTION)

// PRE
#define PRE_CONTROL_REGISTER_OFS 0x0004
REGDEF_BEGIN(PRE_CONTROL_REGISTER)
REGDEF_BIT(pre_mode,                   2)
REGDEF_BIT(pre_input_format,           1)
REGDEF_BIT(pre_subout_sel,             1)
REGDEF_BIT(inbit_16_fmt_sel,           1)
REGDEF_BIT(inbit_depth,                2)
REGDEF_BIT(outbit_depth,               2)
REGDEF_BIT(cfapat,                     3)
REGDEF_BIT(,                           1)
REGDEF_BIT(outl_en,                    1)
REGDEF_BIT(filter_en,                  1)
REGDEF_BIT(cgain_en,                   1)
REGDEF_BIT(vig_en,                     1)
REGDEF_BIT(gbal_en,                    1)
REGDEF_BIT(binning,                    3)
REGDEF_BIT(bayer_fmt,                  1)
REGDEF_BIT(rgbir_rb_nrfill,            1)
REGDEF_BIT(bilat_th_en,                1)
REGDEF_BIT(dgain_en,                   1)
REGDEF_BIT(f_cg_en,                    1)
REGDEF_BIT(f_fusion_en,                1)
REGDEF_BIT(f_fusion_fnum,              2)
REGDEF_BIT(f_fc_en,                    1)
REGDEF_BIT(mirror_en,                  1)
REGDEF_BIT(pre_thermal_mode,           1)
REGDEF_END(PRE_CONTROL_REGISTER)

#define PRE_SETTING_REGISTER_OFS 0x0018
REGDEF_BEGIN(PRE_SETTING_REGISTER)
REGDEF_BIT(cfapat_2,                   3)
REGDEF_BIT(,                           5)
REGDEF_BIT(pre_hdr_sram_shutdown_en,   1)
REGDEF_BIT(pre_va_sram_shutdown_en,    1)
REGDEF_BIT(pre_bnr_sram_shutdown_en,   1)
REGDEF_BIT(pre_outl_sram_shutdown_en,  1)
REGDEF_BIT(,                          12)
REGDEF_BIT(dram_saill_msb,             4)
REGDEF_BIT(pre_flip0_en,               1)
REGDEF_BIT(pre_flip1_en,               1)
REGDEF_END(PRE_SETTING_REGISTER)

#define _3DNR_REGISTER_OFS 0x0DA0
REGDEF_BEGIN(_3DNR_REGISTER)
REGDEF_BIT(pre_bnr_en,                 1)
REGDEF_BIT(pre_bnr_ref_out_en,         1)
REGDEF_BIT(pre_bnr_sta_out_en,         1)
REGDEF_BIT(pre_bnr_sg_en,              1)
REGDEF_BIT(pre_bnr_gamma_map_out_en,   1)
REGDEF_BIT(pre_bnr_err_comp_en,        1)
REGDEF_BIT(pre_bnr_ref_enc_en,         1)
REGDEF_BIT(pre_bnr_ref_dec_en,         1)
REGDEF_BIT(pre_bnr_enc_gamma_en,       1)
REGDEF_BIT(pre_bnr_dec_degamma_en,     1)
REGDEF_BIT(pre_bnr_dec_dith_en,        1)
REGDEF_BIT(pre_bnr_debug_mode,         1)
REGDEF_BIT(pre_bnr_sg_opt,             1)
REGDEF_BIT(pre_bnr_gamma_out_wait_en,  1)
REGDEF_BIT(pre_bnr_aided_map_direct_en,1)
REGDEF_BIT(,                           5)
REGDEF_BIT(pre_bnr_residue_th,         4)
REGDEF_BIT(pre_bnr_pf_str,             8)
REGDEF_END(_3DNR_REGISTER)

#define VA_REGISTER1_OFS 0x0c00
REGDEF_BEGIN(VA_REGISTER1)
REGDEF_BIT(vacc_en,                    1)
REGDEF_BIT(win0_vaen,                  1)
REGDEF_BIT(win1_vaen,                  1)
REGDEF_BIT(win2_vaen,                  1)
REGDEF_BIT(win3_vaen,                  1)
REGDEF_BIT(win4_vaen,                  1)
REGDEF_BIT(va_pre_filter_mode,         2)
REGDEF_BIT(va_gamma_sel,               1)
REGDEF_BIT(va_win_ldg_en,              1)
REGDEF_BIT(va_in_sel,                  2)
REGDEF_BIT(va_vertical_fir_en,         1)
REGDEF_END(VA_REGISTER1)

#define VA_REGISTER10_OFS 0x0c24
REGDEF_BEGIN(VA_REGISTER10)
REGDEF_BIT(vacc_outsel,                1)
REGDEF_BIT(,                           2)
REGDEF_BIT(va_stx,                    13)
REGDEF_BIT(va_sty,                    13)
REGDEF_END(VA_REGISTER10)

// IFE
#define IFE_CONTROL_REGISTER_OFS 0x0004
REGDEF_BEGIN(IFE_CONTROL_REGISTER)
REGDEF_BIT(ife_mode,                   2)
REGDEF_BIT(input_format,               1)
REGDEF_BIT(output_sel,                 1)
REGDEF_BIT(inbit_16_fmt_sel,           1)
REGDEF_BIT(inbit_depth,                2)
REGDEF_BIT(outbit_depth,               2)
REGDEF_BIT(cfapat,                     3)
REGDEF_BIT(filter_range,               1)
REGDEF_BIT(,                           1)
REGDEF_BIT(filter_en,                  1)
REGDEF_BIT(cgain_en,                   1)
REGDEF_BIT(vig_en,                     1)
REGDEF_BIT(gbal_en,                    1)
REGDEF_BIT(ife_binning,                3)
REGDEF_BIT(bayer_format,               1)
REGDEF_BIT(rgbir_rb_nrfill,            1)
REGDEF_BIT(bilat_th_en,                1)
REGDEF_BIT(dgain_en,                   1)
REGDEF_END(IFE_CONTROL_REGISTER)

#define IFE_SETTING_REGISTER_OFS 0x0018
REGDEF_BEGIN(IFE_SETTING_REGISTER)
REGDEF_BIT(,                           3)
REGDEF_BIT(ife_output_sel,             1)
REGDEF_BIT(,                           8)
REGDEF_BIT(ife_2dnr_sram_shutdown_en,  1)
REGDEF_BIT(ife_gbal_sram_shutdown_en,  1)
REGDEF_BIT(ife_wdr_sram_shutdown_en,   1)
REGDEF_BIT(,                           9)
REGDEF_BIT(dram_saill_msb,             4)
REGDEF_BIT(,                           3)
REGDEF_BIT(ife_input_sel,              1)
REGDEF_END(IFE_SETTING_REGISTER)

#define IFE_MOTION_CONTROL_REGISTER_OFS 0x00fc
REGDEF_BEGIN(IFE_MOTION_CONTROL_REGISTER)
REGDEF_BIT(ife_motion_en,              1)
REGDEF_BIT(ife_motion_type,            2)
REGDEF_BIT(ife_motion_in_sel,          1)
REGDEF_BIT(,                           4)
REGDEF_BIT(ife_static_symbo,           8)
REGDEF_BIT(ife_transition_symbol,      8)
REGDEF_BIT(ife_motion_symbol,          8)
REGDEF_END(IFE_MOTION_CONTROL_REGISTER)

#define WDR_CONTROL_REGISTER0_OFS 0x0900
REGDEF_BEGIN(WDR_CONTROL_REGISTER0)
REGDEF_BIT(ife_wdr_en,                 1)
REGDEF_BIT(ife_wdr_subimg_out_en,      1)
REGDEF_BIT(ife_wdr_histogram_en,       1)
REGDEF_BIT(ife_wdr_histogram_sel,      1)
REGDEF_BIT(ife_wdr_tcurve_en,          1)
REGDEF_BIT(ife_wdr_d2d_rand,           1)
REGDEF_BIT(ife_wdr_d2d_rand_rst,       1)
REGDEF_BIT(,                          24)
REGDEF_BIT(ife_wdr_bypass_mode,        1)
REGDEF_END(WDR_CONTROL_REGISTER0)

#define IFE_WDR_SUBIMAGE_REGISTER_0_OFS 0x0904
REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_0)
REGDEF_BIT(ife_wdr_subimg_width,       6)
REGDEF_BIT(,                           2)
REGDEF_BIT(ife_wdr_subimg_height,      6)
REGDEF_BIT(,                           2)
REGDEF_BIT(ife_wdr_dithering_en,       1)
REGDEF_BIT(ife_wdr_rand_reset,         1)
REGDEF_BIT(ife_wdr_rand_sel,           2)
REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_0)

#define IFE_WDR_CONTROL_REGISTER1_OFS 0x0948
REGDEF_BEGIN(IFE_WDR_CONTROL_REGISTER1)
REGDEF_BIT(ife_wdr_outbld_table_en,    1)
REGDEF_BIT(ife_wdr_gainctrl_en,        1)
REGDEF_BIT(,                          14)
REGDEF_BIT(ife_wdr_maxgain,            8)
REGDEF_BIT(ife_wdr_mingain,            8)
REGDEF_END(IFE_WDR_CONTROL_REGISTER1)

#define IFE_WDR_TONE_MAPPING_REGISTER_0_OFS 0x0aac
REGDEF_BEGIN(IFE_WDR_TONE_MAPPING_REGISTER_0)
REGDEF_BIT(ife_wdr_mode,               1)
REGDEF_BIT(ife_wdr_anti_halo_opt,      1)
REGDEF_BIT(,                           6)
REGDEF_BIT(ife_wdr_halo_ratio,         8)
REGDEF_BIT(ife_wdr_halo_slope,         8)
REGDEF_END(IFE_WDR_TONE_MAPPING_REGISTER_0)

#define IFE_WDR_FBC_REGISTER_0_OFS 0x0bb8
REGDEF_BEGIN(IFE_WDR_FBC_REGISTER_0)
REGDEF_BIT(ife_wdr_fbc_en,            1)
REGDEF_BIT(,                          7)
REGDEF_BIT(ife_wdr_fbc_rto,           8)
REGDEF_BIT(ife_wdr_fbc_th0,          13)
REGDEF_END(IFE_WDR_FBC_REGISTER_0)

// IPE
#define IPE_MODE_REGISTER_1_OFS 0x0008
REGDEF_BEGIN(IPE_MODE_REGISTER_1)
REGDEF_BIT(rgblpf_en,                  1)
REGDEF_BIT(rgbgamma_en,                1)
REGDEF_BIT(ycurve_en,                  1)
REGDEF_BIT(cr_en,                      1)
REGDEF_BIT(defog_subimg_out_en,        1)
REGDEF_BIT(defog_en,                   1)
REGDEF_BIT(  ,                         1)
REGDEF_BIT(lce_en,                     1)
REGDEF_BIT(cst_en,                     1)
REGDEF_BIT(ctrl_en,                    1)
REGDEF_BIT(hadj_en,                    1)
REGDEF_BIT(cadj_en,                    1)
REGDEF_BIT(cadj_yenh_en,               1)
REGDEF_BIT(cadj_ycon_en,               1)
REGDEF_BIT(cadj_ccon_en,               1)
REGDEF_BIT(cadj_ycth_en,               1)
REGDEF_BIT(cstprotect_en,              1)
REGDEF_BIT(edge_dbg_en,                1)
REGDEF_BIT(,                           1)
REGDEF_BIT(vacc_en,                    1)
REGDEF_BIT(win0_vaen,                  1)
REGDEF_BIT(win1_vaen,                  1)
REGDEF_BIT(win2_vaen,                  1)
REGDEF_BIT(win3_vaen,                  1)
REGDEF_BIT(win4_vaen,                  1)
REGDEF_BIT(pfr_en,                     1)
REGDEF_BIT(va_pre_filter_mode,         2)
REGDEF_BIT(va_win_ldg_en,              1)
REGDEF_BIT(cc3d_en,                    1)
REGDEF_BIT(ipe_ycc_shift_mode_en,      1)
REGDEF_BIT(edge_ref_motion_en,         1)
REGDEF_END(IPE_MODE_REGISTER_1)

#define IPE_POWER_SAVE_MODE_REGISTER_OFS 0x02e0
REGDEF_BEGIN(IPE_POWER_SAVE_MODE_REGISTER)
REGDEF_BIT(ipe_power_save_mode_edge_ker,                2)
REGDEF_BIT(ipe_power_save_mode_edge,                    1)
REGDEF_BIT(,                                            5)
REGDEF_BIT(ipe_power_save_mode_defog_sram_sd_en,        1)
REGDEF_BIT(ipe_power_save_mode_gamma_sram_sd_en,        1)
REGDEF_BIT(ipe_power_save_mode_ycurve_sram_sd_en,       1)
REGDEF_BIT(ipe_power_save_mode_cc3d_sram_sd_en,         1)
REGDEF_BIT(ipe_power_save_mode_cfa_sram_sd_en,          1)
REGDEF_END(IPE_POWER_SAVE_MODE_REGISTER)

#define DEFOG_AIRLIGHT_REGISTER_1_OFS 0x0394
REGDEF_BEGIN(DEFOG_AIRLIGHT_REGISTER_1)
REGDEF_BIT(defog_air2,                10)
REGDEF_BIT(,                           2)
REGDEF_BIT(defog_air_en,               1)
REGDEF_END(DEFOG_AIRLIGHT_REGISTER_1)

#define PURPLE_FRINGE_REDUCTION_REGISTER0_OFS 0x0440
REGDEF_BEGIN(PURPLE_FRINGE_REDUCTION_REGISTER0)
	REGDEF_BIT(pfr_uv_filt_en,         1)
	REGDEF_BIT(pfr_luma_level_en,      1)
	REGDEF_BIT(pfr_set0_en,            1)
	REGDEF_BIT(pfr_set1_en,            1)
	REGDEF_BIT(pfr_set2_en,            1)
	REGDEF_BIT(pfr_set3_en,            1)
	REGDEF_BIT(,                      10)
	REGDEF_BIT(pfr_out_wet,            8)
REGDEF_END(PURPLE_FRINGE_REDUCTION_REGISTER0)

#define IPE_MODE_REGISTER_2_OFS 0x0900
REGDEF_BEGIN(IPE_MODE_REGISTER_2)
REGDEF_BIT(cfa_en,                     1)
REGDEF_BIT(cfa_subimg_out_en,          1)
REGDEF_BIT(cfa_subimg_out_flip_en,     1)
REGDEF_BIT(cfa_pinkr_en,               1)
REGDEF_BIT(cfa_fcs_en,                 1)
REGDEF_END(IPE_MODE_REGISTER_2)

//IME
#define IME_FUNCTION_CONTROL_REGISTER0_OFS 0x0004
REGDEF_BEGIN(IME_FUNCTION_CONTROL_REGISTER0)
REGDEF_BIT(ime_src,                    1)
REGDEF_BIT(ime_dir_ctrl,               1)
REGDEF_BIT(ime_p0_en,                  1)
REGDEF_BIT(ime_p1_en,                  1)
REGDEF_BIT(ime_p2_en,                  1)
REGDEF_BIT(ime_p3_en,                  1)
REGDEF_BIT(,                           4)
REGDEF_BIT(ime_lca_en,                 1)
REGDEF_BIT(,                           2)
REGDEF_BIT(ime_dbcs_en,                1)
REGDEF_BIT(,                           1)
REGDEF_BIT(ime_fisheye_mask_en,        1)
REGDEF_BIT(ime_nn_isp_p2_3dnr_en,      1)
REGDEF_BIT(ime_nn_isp_p3_2dnr_en,      1)
REGDEF_BIT(,                           6)
REGDEF_BIT(ime_post_shp_en,            1)
REGDEF_BIT(,                           2)
REGDEF_BIT(ime_3dnr_en,                1)
REGDEF_BIT(ime_3dnr_ref_out_sel,       1)
REGDEF_BIT(ime_3dnr_ref_in_dec_en,     1)
REGDEF_BIT(ime_3dnr_ref_out_en,        1)
REGDEF_BIT(ime_3dnr_ref_out_enc_en,    1)
REGDEF_END(IME_FUNCTION_CONTROL_REGISTER0)

#define LOCAL_CHROMA_ADAPTATION_REGISTER0_OFS 0x0b70
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER0)
REGDEF_BIT(lca_proc_lcon_sel,          1)
REGDEF_BIT(lca_edge_ker_size,          1)
REGDEF_BIT(,                           2)
REGDEF_BIT(lca_sr_rf_ker_size,         3)
REGDEF_BIT(,                           1)
REGDEF_BIT(lca_mr_rf_ker_size,         3)
REGDEF_BIT(,                          18)
REGDEF_BIT(lca_edge_detect_en,         1)
REGDEF_BIT(lca_ref_center_en,          1)
REGDEF_BIT(lca_motion_status_en,       1)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER0)

#define IME_TMNR_CONTROL_REGISTER0_OFS 0x0700
REGDEF_BEGIN(IME_TMNR_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_pre_y_blur_str,        2)
REGDEF_BIT(ime_3dnr_pf_type,               2)
REGDEF_BIT(ime_3dnr_me_update_mode,        1)
REGDEF_BIT(ime_3dnr_me_boundary_set,       1)
REGDEF_BIT(,                               2)
REGDEF_BIT(ime_3dnr_ps_smart_roi_ctrl,     1)
REGDEF_BIT(ime_3dnr_nr_center_wzero_y_3d,  1)
REGDEF_BIT(ime_3dnr_ps_mv_check_en,        1)
REGDEF_BIT(ime_3dnr_ps_mv_check_roi_en,    1)
REGDEF_BIT(ime_3dnr_ps_mv_info_mode,       2)
REGDEF_BIT(,                               2)
REGDEF_BIT(ime_3dnr_me_sad_shift,          4)
REGDEF_BIT(ime_3dnr_nr_y_ch_en,            1)
REGDEF_BIT(ime_3dnr_nr_c_ch_en,            1)
REGDEF_BIT(,                               1)
REGDEF_BIT(ime_3dnr_seed_reset_en,         1)
REGDEF_BIT(ime_3dnr_nr_tf0_y_e,            1)
REGDEF_BIT(ime_3dnr_nr_tf0_c_e,            1)
REGDEF_BIT(ime_3dnr_mot_est_en,            1)
REGDEF_END(IME_TMNR_CONTROL_REGISTER0)

//=============================================================================
// extern functions
//=============================================================================
extern void iq_msg_sie(IQALG_INFO *iq_info);
extern void iq_msg_pre(IQALG_INFO *iq_info);
extern void iq_msg_aiisp(IQALG_INFO *iq_info);
extern void iq_msg_ife(IQALG_INFO *iq_info);
extern void iq_msg_ipe(IQALG_INFO *iq_info);
extern void iq_msg_ime(IQALG_INFO *iq_info);
extern void iq_msg_enc(IQALG_INFO *iq_info);

#endif

