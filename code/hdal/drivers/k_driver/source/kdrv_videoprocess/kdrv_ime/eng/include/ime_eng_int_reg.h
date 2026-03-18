
#ifndef _IME_ENG_INT_REG_H_
#define _IME_ENG_INT_REG_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__LINUX)
#include <linux/version.h>

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
#include <linux/soc/nvt/rcw_macro.h>
#else
#include "mach/rcw_macro.h"
#endif



#include "plat/top.h"
#include "kwrap/type.h"

//=========================================================================
#elif defined (__FREERTOS)


#include "rcw_macro.h"
#include "plat/top.h"
#include "kwrap/type.h"

#else


#endif



/*
    ime_rst                       :    [0x0, 0x1],          bits : 0
    ime_start                       :    [0x0, 0x1],            bits : 1
    ime_start_load                 :    [0x0, 0x1],         bits : 2
    ime_frameend_load               :    [0x0, 0x1],            bits : 3
    ime_drt_start_load   :    [0x0, 0x1],           bits : 4
    ime_gbl_load_en      :    [0x0, 0x1],           bits : 5
    ime_dmach_dis        :    [0x0, 0x1],           bits : 27
    ime_ll_fire          :    [0x0, 0x1],           bits : 28
*/
#define IME_ENGINE_CONTROL_REGISTER_OFS 0x0000
REGDEF_BEGIN(IME_ENGINE_CONTROL_REGISTER)
REGDEF_BIT(ime_rst,        1)
REGDEF_BIT(ime_start,        1)
REGDEF_BIT(ime_start_tload,        1)
REGDEF_BIT(ime_frameend_tload,        1)
REGDEF_BIT(ime_drt_start_load,        1)
REGDEF_BIT(ime_gbl_load_en,        1)
REGDEF_BIT(,        21)
REGDEF_BIT(ime_dmach_dis,        1)
REGDEF_BIT(ime_ll_fire,        1)
REGDEF_END(IME_ENGINE_CONTROL_REGISTER)


/*
    ime_src                :    [0x0, 0x1],         bits : 0
    ime_dir_ctrl           :    [0x0, 0x1],         bits : 1
    ime_p0_en              :    [0x0, 0x1],         bits : 2
    ime_p1_en              :    [0x0, 0x1],         bits : 3
    ime_p2_en              :    [0x0, 0x1],         bits : 4
    ime_p3_en              :    [0x0, 0x1],         bits : 5
    ime_lca_en             :    [0x0, 0x1],         bits : 10
    ime_dbcs_en            :    [0x0, 0x1],         bits : 13
    ime_fisheye_mask_en    :    [0x0, 0x1],         bits : 15
    ime_nn_isp_p2_3dnr_en  :    [0x0, 0x1],         bits : 16
    ime_nn_isp_p3_2dnr_en  :    [0x0, 0x1],         bits : 17
    ime_post_shp_en        :    [0x0, 0x1],         bits : 24
    ime_3dnr_en            :    [0x0, 0x1],         bits : 27
    ime_3dnr_ref_out_sel   :    [0x0, 0x1],         bits : 28
    ime_3dnr_ref_in_dec_en :    [0x0, 0x1],         bits : 29
    ime_3dnr_ref_out_en    :    [0x0, 0x1],         bits : 30
    ime_3dnr_ref_out_enc_en:    [0x0, 0x1],         bits : 31
*/
#define IME_FUNCTION_CONTROL_REGISTER0_OFS 0x0004
REGDEF_BEGIN(IME_FUNCTION_CONTROL_REGISTER0)
REGDEF_BIT(ime_src,        1)
REGDEF_BIT(ime_dir_ctrl,        1)
REGDEF_BIT(ime_p0_en,        1)
REGDEF_BIT(ime_p1_en,        1)
REGDEF_BIT(ime_p2_en,        1)
REGDEF_BIT(ime_p3_en,        1)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_lca_en,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_dbcs_en,        1)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_fisheye_mask_en,        1)
REGDEF_BIT(ime_nn_isp_p2_3dnr_en,        1)
REGDEF_BIT(ime_nn_isp_p3_2dnr_en,        1)
REGDEF_BIT(,        6)
REGDEF_BIT(ime_post_shp_en,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_en,        1)
REGDEF_BIT(ime_3dnr_ref_out_sel,        1)
REGDEF_BIT(ime_3dnr_ref_in_dec_en,        1)
REGDEF_BIT(ime_3dnr_ref_out_en,        1)
REGDEF_BIT(ime_3dnr_ref_out_enc_en,        1)
REGDEF_END(IME_FUNCTION_CONTROL_REGISTER0)


/*
    ime_pm0_en             :    [0x0, 0x1],         bits : 0
    ime_pm1_en             :    [0x0, 0x1],         bits : 1
    ime_pm2_en             :    [0x0, 0x1],         bits : 2
    ime_pm3_en             :    [0x0, 0x1],         bits : 3
    ime_pm4_en             :    [0x0, 0x1],         bits : 4
    ime_pm5_en             :    [0x0, 0x1],         bits : 5
    ime_pm6_en             :    [0x0, 0x1],         bits : 6
    ime_pm7_en             :    [0x0, 0x1],         bits : 7
    ime_low_dly_en         :    [0x0, 0x1],         bits : 8
    ime_low_dly_sel        :    [0x0, 0x7],         bits : 11_9
    ime_p0_flip_en         :    [0x0, 0x1],         bits : 12
    ime_p1_flip_en         :    [0x0, 0x1],         bits : 13
    ime_p2_flip_en         :    [0x0, 0x1],         bits : 14
    ime_3dnr_inref_flip_en :    [0x0, 0x1],         bits : 16
    ime_3dnr_outref_flip_en:    [0x0, 0x1],         bits : 17
    ime_3dnr_ms_roi_flip_en:    [0x0, 0x1],         bits : 18
    ime_pm_pxl_subout_en   :    [0x0, 0x1],         bits : 19
    ime_ycc_cvt_en         :    [0x0, 0x1],         bits : 20
    ime_ycc_cvt_sel        :    [0x0, 0x1],         bits : 21
    ime_p0_enc_en          :    [0x0, 0x1],         bits : 22
*/
#define IME_FUNCTION_CONTROL_REGISTER1_OFS 0x0008
REGDEF_BEGIN(IME_FUNCTION_CONTROL_REGISTER1)
REGDEF_BIT(ime_pm0_en,        1)
REGDEF_BIT(ime_pm1_en,        1)
REGDEF_BIT(ime_pm2_en,        1)
REGDEF_BIT(ime_pm3_en,        1)
REGDEF_BIT(ime_pm4_en,        1)
REGDEF_BIT(ime_pm5_en,        1)
REGDEF_BIT(ime_pm6_en,        1)
REGDEF_BIT(ime_pm7_en,        1)
REGDEF_BIT(ime_low_dly_en,        1)
REGDEF_BIT(ime_low_dly_sel,        3)
REGDEF_BIT(ime_p0_flip_en,        1)
REGDEF_BIT(ime_p1_flip_en,        1)
REGDEF_BIT(ime_p2_flip_en,        1)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_inref_flip_en,        1)
REGDEF_BIT(ime_3dnr_outref_flip_en,        1)
REGDEF_BIT(ime_3dnr_ms_roi_flip_en,        1)
REGDEF_BIT(ime_pm_pxl_subout_en,        1)
REGDEF_BIT(ime_ycc_cvt_en,        1)
REGDEF_BIT(ime_ycc_cvt_sel,        1)
REGDEF_BIT(ime_p0_enc_en,        1)
REGDEF_END(IME_FUNCTION_CONTROL_REGISTER1)


/*
    ime_pxl_subout_dram_out_single_en :    [0x0, 0x1],          bits : 0
    ime_3dnr_refout_dram_out_single_en:    [0x0, 0x1],          bits : 1
    ime_3dnr_ms_dram_out_single_en    :    [0x0, 0x1],          bits : 2
    ime_3dnr_ms_roi_dram_out_single_en:    [0x0, 0x1],          bits : 3
    ime_3dnr_mv_dram_out_single_en    :    [0x0, 0x1],          bits : 4
    ime_3dnr_sta_dram_out_single_en   :    [0x0, 0x1],          bits : 5
    ime_out_p0_dram_out_single_en     :    [0x0, 0x1],          bits : 6
    ime_out_p1_dram_out_single_en     :    [0x0, 0x1],          bits : 7
    ime_out_p2_dram_out_single_en     :    [0x0, 0x1],          bits : 8
    ime_out_p3_dram_out_single_en     :    [0x0, 0x1],          bits : 9
    ime_3dnr_fc_dram_out_single_en    :    [0x0, 0x1],          bits : 10
    ime_dram_out_mode                 :    [0x0, 0x1],          bits : 31
*/
#define IME_DRAM_SINGLE_OUTPUT_CONTROL_REGISTER_OFS 0x000c
REGDEF_BEGIN(IME_DRAM_SINGLE_OUTPUT_CONTROL_REGISTER)
REGDEF_BIT(ime_pxl_subout_dram_out_single_en,        1)
REGDEF_BIT(ime_3dnr_refout_dram_out_single_en,        1)
REGDEF_BIT(ime_3dnr_ms_dram_out_single_en,        1)
REGDEF_BIT(ime_3dnr_ms_roi_dram_out_single_en,        1)
REGDEF_BIT(ime_3dnr_mv_dram_out_single_en,        1)
REGDEF_BIT(ime_3dnr_sta_dram_out_single_en,        1)
REGDEF_BIT(ime_out_p0_dram_out_single_en,        1)
REGDEF_BIT(ime_out_p1_dram_out_single_en,        1)
REGDEF_BIT(ime_out_p2_dram_out_single_en,        1)
REGDEF_BIT(ime_out_p3_dram_out_single_en,        1)
REGDEF_BIT(ime_3dnr_fc_dram_out_single_en,        1)
REGDEF_BIT(,        20)
REGDEF_BIT(ime_dram_out_mode,        1)
REGDEF_END(IME_DRAM_SINGLE_OUTPUT_CONTROL_REGISTER)


/*
    ime_ll_terminate  :    [0x0, 0x1],          bits : 0
    ime_dmach_idle    :    [0x0, 0x1],          bits : 4
    ime_sline_num     :    [0x0, 0x1ff],            bits : 15_7
    ime_sline_path_sel:    [0x0, 0x3],          bits : 17_16
*/
#define IME_LINKED_LIST_CONTROL_REGISTER1_OFS 0x0010
REGDEF_BEGIN(IME_LINKED_LIST_CONTROL_REGISTER1)
REGDEF_BIT(ime_ll_terminate,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_dmach_idle,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_sline_num,        9)
REGDEF_BIT(ime_sline_path_sel,        2)
REGDEF_END(IME_LINKED_LIST_CONTROL_REGISTER1)


/*
    ime_dram_ll_sai:    [0x0, 0x3fffffff],          bits : 31_2
*/
#define IME_LINKED_LIST_INPUT_DMA_CHANNEL_REGISTER_OFS 0x0014
REGDEF_BEGIN(IME_LINKED_LIST_INPUT_DMA_CHANNEL_REGISTER)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_dram_ll_sai,        30)
REGDEF_END(IME_LINKED_LIST_INPUT_DMA_CHANNEL_REGISTER)


/*
    ime_inte_ll_end                 :    [0x0, 0x1],            bits : 0
    ime_inte_ll_err                 :    [0x0, 0x1],            bits : 1
    ime_inte_ll_red_late            :    [0x0, 0x1],            bits : 2
    ime_inte_ll_job_end             :    [0x0, 0x1],            bits : 3
    ime_inte_in_bp0                 :    [0x0, 0x1],            bits : 4
    ime_inte_in_bp1                 :    [0x0, 0x1],            bits : 5
    ime_inte_in_bp2                 :    [0x0, 0x1],            bits : 6
    ime_inte_3dnr_slice_end         :    [0x0, 0x1],            bits : 7
    ime_inte_3dnr_mot_end           :    [0x0, 0x1],            bits : 8
    ime_inte_3dnr_mv_end            :    [0x0, 0x1],            bits : 9
    ime_inte_3dnr_statsitic_end     :    [0x0, 0x1],            bits : 10
    ime_inte_3dnr_ref_out_enc_ovfl  :    [0x0, 0x1],            bits : 12
    ime_inte_3dnr_ref_in_dec_err    :    [0x0, 0x1],            bits : 13
    ime_inte_frm_err                :    [0x0, 0x1],            bits : 14
    ime_inte_sline                  :    [0x0, 0x1],            bits : 15
    ime_inte_p0_out_enc_ovfl        :    [0x0, 0x1],            bits : 16
    ime_inte_in_dec_err             :    [0x0, 0x1],            bits : 20
    ime_inte_nn_isp2cpu_p2_out_ready:    [0x0, 0x1],            bits : 21
    ime_inte_nn_isp2cpu_p2_in_clear :    [0x0, 0x1],            bits : 22
    ime_inte_nn_isp2cpu_p3_out_ready:    [0x0, 0x1],            bits : 23
    ime_inte_nn_isp2cpu_p3_in_clear :    [0x0, 0x1],            bits : 24
    ime_inte_frm_start              :    [0x0, 0x1],            bits : 29
    ime_inte_strp_end               :    [0x0, 0x1],            bits : 30
    ime_inte_frm_end                :    [0x0, 0x1],            bits : 31
*/
#define IME_INTERRUPT_ENABLE_REGISTER_OFS 0x0018
REGDEF_BEGIN(IME_INTERRUPT_ENABLE_REGISTER)
REGDEF_BIT(ime_inte_ll_end,        1)
REGDEF_BIT(ime_inte_ll_err,        1)
REGDEF_BIT(ime_inte_ll_red_late,        1)
REGDEF_BIT(ime_inte_ll_job_end,        1)
REGDEF_BIT(ime_inte_in_bp0,        1)
REGDEF_BIT(ime_inte_in_bp1,        1)
REGDEF_BIT(ime_inte_in_bp2,        1)
REGDEF_BIT(ime_inte_3dnr_slice_end,        1)
REGDEF_BIT(ime_inte_3dnr_mot_end,        1)
REGDEF_BIT(ime_inte_3dnr_mv_end,        1)
REGDEF_BIT(ime_inte_3dnr_statsitic_end,        1)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_inte_3dnr_ref_out_enc_ovfl,        1)
REGDEF_BIT(ime_inte_3dnr_ref_in_dec_err,        1)
REGDEF_BIT(ime_inte_frm_err,        1)
REGDEF_BIT(ime_inte_sline,        1)
REGDEF_BIT(ime_inte_p0_out_enc_ovfl,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_inte_in_dec_err,        1)
REGDEF_BIT(ime_inte_nn_isp2cpu_p2_out_ready,        1)
REGDEF_BIT(ime_inte_nn_isp2cpu_p2_in_clear,        1)
REGDEF_BIT(ime_inte_nn_isp2cpu_p3_out_ready,        1)
REGDEF_BIT(ime_inte_nn_isp2cpu_p3_in_clear,        1)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_inte_frm_start,        1)
REGDEF_BIT(ime_inte_strp_end,        1)
REGDEF_BIT(ime_inte_frm_end,        1)
REGDEF_END(IME_INTERRUPT_ENABLE_REGISTER)


/*
    ime_ints_ll_end                 :    [0x0, 0x1],            bits : 0
    ime_ints_ll_err                 :    [0x0, 0x1],            bits : 1
    ime_ints_ll_red_late            :    [0x0, 0x1],            bits : 2
    ime_ints_ll_job_end             :    [0x0, 0x1],            bits : 3
    ime_ints_in_bp0                 :    [0x0, 0x1],            bits : 4
    ime_ints_in_bp1                 :    [0x0, 0x1],            bits : 5
    ime_ints_in_bp2                 :    [0x0, 0x1],            bits : 6
    ime_ints_3dnr_slice_end         :    [0x0, 0x1],            bits : 7
    ime_ints_3dnr_mot_end           :    [0x0, 0x1],            bits : 8
    ime_ints_3dnr_mv_end            :    [0x0, 0x1],            bits : 9
    ime_ints_3dnr_statsitic_end     :    [0x0, 0x1],            bits : 10
    ime_ints_3dnr_ref_out_enc_ovfl  :    [0x0, 0x1],            bits : 12
    ime_ints_3dnr_ref_in_dec_err    :    [0x0, 0x1],            bits : 13
    ime_ints_frm_err                :    [0x0, 0x1],            bits : 14
    ime_ints_sline                  :    [0x0, 0x1],            bits : 15
    ime_ints_p0_out_enc_ovfl        :    [0x0, 0x1],            bits : 16
    ime_ints_in_dec_err             :    [0x0, 0x1],            bits : 20
    ime_ints_nn_isp2cpu_p2_out_ready:    [0x0, 0x1],            bits : 21
    ime_ints_nn_isp2cpu_p2_in_clear :    [0x0, 0x1],            bits : 22
    ime_ints_nn_isp2cpu_p3_out_ready:    [0x0, 0x1],            bits : 23
    ime_ints_nn_isp2cpu_p3_in_clear :    [0x0, 0x1],            bits : 24
    ime_ints_frm_start              :    [0x0, 0x1],            bits : 29
    ime_ints_strp_end               :    [0x0, 0x1],            bits : 30
    ime_ints_frm_end                :    [0x0, 0x1],            bits : 31
*/
#define IME_STATUS_REGISTER_OFS 0x001c
REGDEF_BEGIN(IME_STATUS_REGISTER)
REGDEF_BIT(ime_ints_ll_end,        1)
REGDEF_BIT(ime_ints_ll_err,        1)
REGDEF_BIT(ime_ints_ll_red_late,        1)
REGDEF_BIT(ime_ints_ll_job_end,        1)
REGDEF_BIT(ime_ints_in_bp0,        1)
REGDEF_BIT(ime_ints_in_bp1,        1)
REGDEF_BIT(ime_ints_in_bp2,        1)
REGDEF_BIT(ime_ints_3dnr_slice_end,        1)
REGDEF_BIT(ime_ints_3dnr_mot_end,        1)
REGDEF_BIT(ime_ints_3dnr_mv_end,        1)
REGDEF_BIT(ime_ints_3dnr_statsitic_end,        1)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_ints_3dnr_ref_out_enc_ovfl,        1)
REGDEF_BIT(ime_ints_3dnr_ref_in_dec_err,        1)
REGDEF_BIT(ime_ints_frm_err,        1)
REGDEF_BIT(ime_ints_sline,        1)
REGDEF_BIT(ime_ints_p0_out_enc_ovfl,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_ints_in_dec_err,        1)
REGDEF_BIT(ime_ints_nn_isp2cpu_p2_out_ready,        1)
REGDEF_BIT(ime_ints_nn_isp2cpu_p2_in_clear,        1)
REGDEF_BIT(ime_ints_nn_isp2cpu_p3_out_ready,        1)
REGDEF_BIT(ime_ints_nn_isp2cpu_p3_in_clear,        1)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_ints_frm_start,        1)
REGDEF_BIT(ime_ints_strp_end,        1)
REGDEF_BIT(ime_ints_frm_end,        1)
REGDEF_END(IME_STATUS_REGISTER)


/*
    ime_in_h_size:    [0x0, 0x3fff],            bits : 15_2
    ime_in_v_size:    [0x0, 0x3fff],            bits : 31_18
*/
#define IME_INPUT_IMAGE_SIZE_REGISTER_OFS 0x0020
REGDEF_BEGIN(IME_INPUT_IMAGE_SIZE_REGISTER)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_in_h_size,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_in_v_size,        14)
REGDEF_END(IME_INPUT_IMAGE_SIZE_REGISTER)


/*
    ime_imat        :    [0x0, 0x3],            bits : 1_0
    ime_st_hovlp_sel:    [0x0, 0x3],            bits : 5_4
    ime_st_prt_sel  :    [0x0, 0x3],            bits : 7_6
    ime_st_size_mode:    [0x0, 0x1],            bits : 8
    ime_st_hovlp_msb:    [0x0, 0x3],            bits : 17_16
    ime_st_prt_msb  :    [0x0, 0x3],            bits : 19_18
*/
#define IME_INPUT_STRIPE_HORIZONTAL_DIMENSION_OFS 0x0024
REGDEF_BEGIN(IME_INPUT_STRIPE_HORIZONTAL_DIMENSION)
REGDEF_BIT(ime_imat,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_st_hovlp_sel,        2)
REGDEF_BIT(ime_st_prt_sel,        2)
REGDEF_BIT(ime_st_size_mode,        1)
REGDEF_BIT(,        7)
REGDEF_BIT(ime_st_hovlp_msb,        2)
REGDEF_BIT(ime_st_prt_msb,        2)
REGDEF_END(IME_INPUT_STRIPE_HORIZONTAL_DIMENSION)


/*
    ime_st_hn:    [0x0, 0x7ff],         bits : 10_0
    ime_st_hl:    [0x0, 0x7ff],         bits : 22_12
    ime_st_hm:    [0x0, 0xff],          bits : 31_24
*/
#define IME_INPUT_STRIPE_HORIZONTAL_DIMENSION1_OFS 0x0028
REGDEF_BEGIN(IME_INPUT_STRIPE_HORIZONTAL_DIMENSION1)
REGDEF_BIT(ime_st_hn,        11)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_st_hl,        11)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_st_hm,        8)
REGDEF_END(IME_INPUT_STRIPE_HORIZONTAL_DIMENSION1)


/*
    ime_st_vl   :    [0x0, 0xffff],         bits : 15_0
    ime_st_prt  :    [0x0, 0xff],           bits : 23_16
    ime_st_hovlp:    [0x0, 0x1f],           bits : 31_27
*/
#define IME_INPUT_STRIPE_VERTICAL_DIMENSION_REGISTER_OFS 0x002c
REGDEF_BEGIN(IME_INPUT_STRIPE_VERTICAL_DIMENSION_REGISTER)
REGDEF_BIT(ime_st_vl,        16)
REGDEF_BIT(ime_st_prt,        8)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_st_hovlp,        5)
REGDEF_END(IME_INPUT_STRIPE_VERTICAL_DIMENSION_REGISTER)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_1_OFS 0x0030
REGDEF_BEGIN(IME_RESERVED_REGISTER_1)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_1)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_2_OFS 0x0034
REGDEF_BEGIN(IME_RESERVED_REGISTER_2)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_2)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_3_OFS 0x0038
REGDEF_BEGIN(IME_RESERVED_REGISTER_3)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_3)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_4_OFS 0x003c
REGDEF_BEGIN(IME_RESERVED_REGISTER_4)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_4)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_5_OFS 0x0040
REGDEF_BEGIN(IME_RESERVED_REGISTER_5)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_5)


/*
    ime_y_dram_ofsi:    [0x0, 0x3ffff],         bits : 19_2
*/
#define IME_INPUT_DMA_LINEOFFSET_REGISTER0_OFS 0x0044
REGDEF_BEGIN(IME_INPUT_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_y_dram_ofsi,        18)
REGDEF_END(IME_INPUT_DMA_LINEOFFSET_REGISTER0)


/*
    ime_uv_dram_ofsi:    [0x0, 0x3ffff],            bits : 19_2
*/
#define IME_INPUT_DMA_LINEOFFSET_REGISTER1_OFS 0x0048
REGDEF_BEGIN(IME_INPUT_DMA_LINEOFFSET_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_uv_dram_ofsi,        18)
REGDEF_END(IME_INPUT_DMA_LINEOFFSET_REGISTER1)


/*
    ime_y_dram_sai:    [0x0, 0x3fffffff],           bits : 31_2
*/
#define IME_INPUT_DMA_REGISTER0_OFS 0x004c
REGDEF_BEGIN(IME_INPUT_DMA_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_y_dram_sai,        30)
REGDEF_END(IME_INPUT_DMA_REGISTER0)


/*
    ime_u_dram_sai:    [0x0, 0x3fffffff],           bits : 31_2
*/
#define IME_INPUT_DMA_REGISTER1_OFS 0x0050
REGDEF_BEGIN(IME_INPUT_DMA_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_u_dram_sai,        30)
REGDEF_END(IME_INPUT_DMA_REGISTER1)


/*
    ime_v_dram_sai:    [0x0, 0x3fffffff],           bits : 31_2
*/
#define IME_INPUT_DMA_REGISTER2_OFS 0x0054
REGDEF_BEGIN(IME_INPUT_DMA_REGISTER2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_v_dram_sai,        30)
REGDEF_END(IME_INPUT_DMA_REGISTER2)


/*
    ime_stripe_size0:    [0x0, 0x3ff],          bits : 9_0
    ime_stripe_size1:    [0x0, 0x3ff],          bits : 20_11
    ime_stripe_size2:    [0x0, 0x3ff],          bits : 31_22
*/
#define IME_VARIED_STRIPE_SIZE_REGISTER0_OFS 0x0058
REGDEF_BEGIN(IME_VARIED_STRIPE_SIZE_REGISTER0)
REGDEF_BIT(ime_stripe_size0,        10)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_stripe_size1,        10)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_stripe_size2,        10)
REGDEF_END(IME_VARIED_STRIPE_SIZE_REGISTER0)


/*
    ime_stripe_size3:    [0x0, 0x3ff],          bits : 9_0
    ime_stripe_size4:    [0x0, 0x3ff],          bits : 20_11
    ime_stripe_size5:    [0x0, 0x3ff],          bits : 31_22
*/
#define IME_VARIED_STRIPE_SIZE_REGISTER1_OFS 0x005c
REGDEF_BEGIN(IME_VARIED_STRIPE_SIZE_REGISTER1)
REGDEF_BIT(ime_stripe_size3,        10)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_stripe_size4,        10)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_stripe_size5,        10)
REGDEF_END(IME_VARIED_STRIPE_SIZE_REGISTER1)


/*
    ime_stripe_size6    :    [0x0, 0x3ff],          bits : 9_0
    ime_stripe_size7    :    [0x0, 0x3ff],          bits : 20_11
    ime_stripe_msb_size0:    [0x0, 0x1],            bits : 24
    ime_stripe_msb_size1:    [0x0, 0x1],            bits : 25
    ime_stripe_msb_size2:    [0x0, 0x1],            bits : 26
    ime_stripe_msb_size3:    [0x0, 0x1],            bits : 27
    ime_stripe_msb_size4:    [0x0, 0x1],            bits : 28
    ime_stripe_msb_size5:    [0x0, 0x1],            bits : 29
    ime_stripe_msb_size6:    [0x0, 0x1],            bits : 30
    ime_stripe_msb_size7:    [0x0, 0x1],            bits : 31
*/
#define IME_VARIED_STRIPE_SIZE_REGISTER2_OFS 0x0060
REGDEF_BEGIN(IME_VARIED_STRIPE_SIZE_REGISTER2)
REGDEF_BIT(ime_stripe_size6,        10)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_stripe_size7,        10)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_stripe_msb_size0,        1)
REGDEF_BIT(ime_stripe_msb_size1,        1)
REGDEF_BIT(ime_stripe_msb_size2,        1)
REGDEF_BIT(ime_stripe_msb_size3,        1)
REGDEF_BIT(ime_stripe_msb_size4,        1)
REGDEF_BIT(ime_stripe_msb_size5,        1)
REGDEF_BIT(ime_stripe_msb_size6,        1)
REGDEF_BIT(ime_stripe_msb_size7,        1)
REGDEF_END(IME_VARIED_STRIPE_SIZE_REGISTER2)


/*
    ime_dend_sts_en:    [0x0, 0x1],         bits : 0
    ime_dend_wbit  :    [0x0, 0x1f],            bits : 8_4
*/
#define IME_DRAM_END_OUTPUT_STATUS_REGISTER0_OFS 0x0064
REGDEF_BEGIN(IME_DRAM_END_OUTPUT_STATUS_REGISTER0)
REGDEF_BIT(ime_dend_sts_en,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_dend_wbit,        5)
REGDEF_END(IME_DRAM_END_OUTPUT_STATUS_REGISTER0)


/*
    ime_dend_status:    [0x0, 0xffffffff],          bits : 31_0
*/
#define IME_DRAM_END_OUTPUT_STATUS_REGISTER1_OFS 0x0068
REGDEF_BEGIN(IME_DRAM_END_OUTPUT_STATUS_REGISTER1)
REGDEF_BIT(ime_dend_status,        32)
REGDEF_END(IME_DRAM_END_OUTPUT_STATUS_REGISTER1)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_6_OFS 0x006c
REGDEF_BEGIN(IME_RESERVED_REGISTER_6)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_6)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_7_OFS 0x0070
REGDEF_BEGIN(IME_RESERVED_REGISTER_7)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_7)


/*
    ime_dram_ll_msb_sai:    [0x0, 0xf],         bits : 3_0
*/
#define IME_LINKED_LIST_INPUT_DMA_CHANNEL_REGISTER1_OFS 0x0074
REGDEF_BEGIN(IME_LINKED_LIST_INPUT_DMA_CHANNEL_REGISTER1)
REGDEF_BIT(ime_dram_ll_msb_sai,        4)
REGDEF_END(IME_LINKED_LIST_INPUT_DMA_CHANNEL_REGISTER1)


/*
    ime_y_dram_msb_sai:    [0x0, 0xf],          bits : 3_0
*/
#define IME_INPUT_DMA_REGISTER3_OFS 0x0078
REGDEF_BEGIN(IME_INPUT_DMA_REGISTER3)
REGDEF_BIT(ime_y_dram_msb_sai,        4)
REGDEF_END(IME_INPUT_DMA_REGISTER3)


/*
    ime_u_dram_msb_sai:    [0x0, 0xf],          bits : 3_0
*/
#define IME_INPUT_DMA_REGISTER4_OFS 0x007c
REGDEF_BEGIN(IME_INPUT_DMA_REGISTER4)
REGDEF_BIT(ime_u_dram_msb_sai,        4)
REGDEF_END(IME_INPUT_DMA_REGISTER4)


/*
    ime_v_dram_msb_sai:    [0x0, 0xf],          bits : 3_0
*/
#define IME_INPUT_DMA_REGISTER5_OFS 0x0080
REGDEF_BEGIN(IME_INPUT_DMA_REGISTER5)
REGDEF_BIT(ime_v_dram_msb_sai,        4)
REGDEF_END(IME_INPUT_DMA_REGISTER5)


/*
    ime_p0_out_type    :    [0x0, 0x1],         bits : 1
    ime_p0_scl_method  :    [0x0, 0x3],         bits : 3_2
    ime_p0_out_en      :    [0x0, 0x1],         bits : 4
    ime_p0_sprt_out_en :    [0x0, 0x1],         bits : 5
    ime_p0_omat        :    [0x0, 0x7],         bits : 10_8
    ime_p0_scl_enh_fact:    [0x0, 0xff],            bits : 19_12
    ime_p0_scl_enh_bit :    [0x0, 0xf],         bits : 23_20
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER0_OFS 0x0084
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER0)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_p0_out_type,        1)
REGDEF_BIT(ime_p0_scl_method,        2)
REGDEF_BIT(ime_p0_out_en,        1)
REGDEF_BIT(ime_p0_sprt_out_en,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p0_omat,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_p0_scl_enh_fact,        8)
REGDEF_BIT(ime_p0_scl_enh_bit,        4)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER0)


/*
    ime_p0_h_ud      :    [0x0, 0x1],           bits : 0
    ime_p0_v_ud      :    [0x0, 0x1],           bits : 1
    ime_p0_h_dnrate  :    [0x0, 0x1f],          bits : 6_2
    ime_p0_v_dnrate  :    [0x0, 0x1f],          bits : 11_7
    ime_p0_h_filtmode:    [0x0, 0x1],           bits : 15
    ime_p0_h_filtcoef:    [0x0, 0x3f],          bits : 21_16
    ime_p0_v_filtmode:    [0x0, 0x1],           bits : 22
    ime_p0_v_filtcoef:    [0x0, 0x3f],          bits : 28_23
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER1_OFS 0x0088
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER1)
REGDEF_BIT(ime_p0_h_ud,        1)
REGDEF_BIT(ime_p0_v_ud,        1)
REGDEF_BIT(ime_p0_h_dnrate,        5)
REGDEF_BIT(ime_p0_v_dnrate,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p0_h_filtmode,        1)
REGDEF_BIT(ime_p0_h_filtcoef,        6)
REGDEF_BIT(ime_p0_v_filtmode,        1)
REGDEF_BIT(ime_p0_v_filtcoef,        6)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER1)


/*
    ime_p0_h_sfact:    [0x0, 0xffff],           bits : 15_0
    ime_p0_v_sfact:    [0x0, 0xffff],           bits : 31_16
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER2_OFS 0x008c
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER2)
REGDEF_BIT(ime_p0_h_sfact,        16)
REGDEF_BIT(ime_p0_v_sfact,        16)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER2)


/*
    ime_p0_h_scl_init_ofs:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER3_OFS 0x0090
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER3)
REGDEF_BIT(ime_p0_h_scl_init_ofs,        32)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER3)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_12_OFS 0x0094
REGDEF_BEGIN(IME_RESERVED_REGISTER_12)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_12)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_13_OFS 0x0098
REGDEF_BEGIN(IME_RESERVED_REGISTER_13)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_13)


/*
    ime_p0_h_scl_size:    [0x0, 0xffff],            bits : 15_0
    ime_p0_v_scl_size:    [0x0, 0xffff],            bits : 31_16
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER6_OFS 0x009c
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER6)
REGDEF_BIT(ime_p0_h_scl_size,        16)
REGDEF_BIT(ime_p0_v_scl_size,        16)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER6)


/*
    ime_p0_cropout_x:    [0x0, 0xffff],         bits : 15_0
    ime_p0_cropout_y:    [0x0, 0xffff],         bits : 31_16
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER7_OFS 0x00a0
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER7)
REGDEF_BIT(ime_p0_cropout_x,        16)
REGDEF_BIT(ime_p0_cropout_y,        16)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER7)


/*
    ime_p0_h_osize:    [0x0, 0xffff],           bits : 15_0
    ime_p0_v_osize:    [0x0, 0xffff],           bits : 31_16
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER8_OFS 0x00a4
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER8)
REGDEF_BIT(ime_p0_h_osize,        16)
REGDEF_BIT(ime_p0_v_osize,        16)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER8)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_14_OFS 0x00a8
REGDEF_BEGIN(IME_RESERVED_REGISTER_14)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_14)


/*
    ime_p0_y_clamp_min :    [0x0, 0xff],            bits : 7_0
    ime_p0_y_clamp_max :    [0x0, 0xff],            bits : 15_8
    ime_p0_uv_clamp_min:    [0x0, 0xff],            bits : 23_16
    ime_p0_uv_clamp_max:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER9_OFS 0x00ac
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER9)
REGDEF_BIT(ime_p0_y_clamp_min,        8)
REGDEF_BIT(ime_p0_y_clamp_max,        8)
REGDEF_BIT(ime_p0_uv_clamp_min,        8)
REGDEF_BIT(ime_p0_uv_clamp_max,        8)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER9)


/*
    ime_p0_y_dram_ofso:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER0_OFS 0x00b0
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p0_y_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER0)


/*
    ime_p0_uv_dram_ofso:    [0x0, 0x3ffff],         bits : 19_2
*/
#define IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER1_OFS 0x00b4
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p0_uv_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER1)


/*
    ime_p0_y_dram_sao:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER0_OFS 0x00b8
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER0)
REGDEF_BIT(ime_p0_y_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER0)


/*
    ime_p0_u_dram_sao:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER1_OFS 0x00bc
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER1)
REGDEF_BIT(ime_p0_u_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER1)


/*
    ime_p0_v_dram_sao:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER2_OFS 0x00c0
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER2)
REGDEF_BIT(ime_p0_v_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER2)


/*
    ime_p0_y_dram_msb_sao:    [0x0, 0xf],           bits : 3_0
*/
#define IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER3_OFS 0x00c4
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER3)
REGDEF_BIT(ime_p0_y_dram_msb_sao,        4)
REGDEF_END(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER3)


/*
    ime_p0_u_dram_msb_sao:    [0x0, 0xf],           bits : 3_0
*/
#define IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER4_OFS 0x00c8
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER4)
REGDEF_BIT(ime_p0_u_dram_msb_sao,        4)
REGDEF_END(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER4)


/*
    ime_p0_v_dram_msb_sao:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER5_OFS 0x00cc
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER5)
REGDEF_BIT(ime_p0_v_dram_msb_sao,        32)
REGDEF_END(IME_OUTPUT_PATH0_DMA_BUFFER0_REGISTER5)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_18_OFS 0x00d0
REGDEF_BEGIN(IME_RESERVED_REGISTER_18)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_18)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_19_OFS 0x00d4
REGDEF_BEGIN(IME_RESERVED_REGISTER_19)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_19)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_20_OFS 0x00d8
REGDEF_BEGIN(IME_RESERVED_REGISTER_20)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_20)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_21_OFS 0x00dc
REGDEF_BEGIN(IME_RESERVED_REGISTER_21)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_21)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_22_OFS 0x00e0
REGDEF_BEGIN(IME_RESERVED_REGISTER_22)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_22)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_23_OFS 0x00e4
REGDEF_BEGIN(IME_RESERVED_REGISTER_23)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_23)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_24_OFS 0x00e8
REGDEF_BEGIN(IME_RESERVED_REGISTER_24)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_24)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_25_OFS 0x00ec
REGDEF_BEGIN(IME_RESERVED_REGISTER_25)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_25)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_26_OFS 0x00f0
REGDEF_BEGIN(IME_RESERVED_REGISTER_26)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_26)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_27_OFS 0x00f4
REGDEF_BEGIN(IME_RESERVED_REGISTER_27)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_27)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_28_OFS 0x00f8
REGDEF_BEGIN(IME_RESERVED_REGISTER_28)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_28)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_29_OFS 0x00fc
REGDEF_BEGIN(IME_RESERVED_REGISTER_29)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_29)


/*
    ime_p1_out_type    :    [0x0, 0x1],         bits : 1
    ime_p1_scl_method  :    [0x0, 0x3],         bits : 3_2
    ime_p1_out_en      :    [0x0, 0x1],         bits : 4
    ime_p1_sprt_out_en :    [0x0, 0x1],         bits : 5
    ime_p1_omat        :    [0x0, 0x7],         bits : 10_8
    ime_p1_scl_enh_fact:    [0x0, 0xff],            bits : 19_12
    ime_p1_scl_enh_bit :    [0x0, 0xf],         bits : 23_20
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER0_OFS 0x0100
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER0)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_p1_out_type,        1)
REGDEF_BIT(ime_p1_scl_method,        2)
REGDEF_BIT(ime_p1_out_en,        1)
REGDEF_BIT(ime_p1_sprt_out_en,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p1_omat,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_p1_scl_enh_fact,        8)
REGDEF_BIT(ime_p1_scl_enh_bit,        4)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER0)


/*
    ime_p1_h_ud      :    [0x0, 0x1],           bits : 0
    ime_p1_v_ud      :    [0x0, 0x1],           bits : 1
    ime_p1_h_dnrate  :    [0x0, 0x1f],          bits : 6_2
    ime_p1_v_dnrate  :    [0x0, 0x1f],          bits : 11_7
    ime_p1_h_filtmode:    [0x0, 0x1],           bits : 15
    ime_p1_h_filtcoef:    [0x0, 0x3f],          bits : 21_16
    ime_p1_v_filtmode:    [0x0, 0x1],           bits : 22
    ime_p1_v_filtcoef:    [0x0, 0x3f],          bits : 28_23
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER1_OFS 0x0104
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER1)
REGDEF_BIT(ime_p1_h_ud,        1)
REGDEF_BIT(ime_p1_v_ud,        1)
REGDEF_BIT(ime_p1_h_dnrate,        5)
REGDEF_BIT(ime_p1_v_dnrate,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p1_h_filtmode,        1)
REGDEF_BIT(ime_p1_h_filtcoef,        6)
REGDEF_BIT(ime_p1_v_filtmode,        1)
REGDEF_BIT(ime_p1_v_filtcoef,        6)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER1)


/*
    ime_p1_h_sfact:    [0x0, 0xffff],           bits : 15_0
    ime_p1_v_sfact:    [0x0, 0xffff],           bits : 31_16
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER2_OFS 0x0108
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER2)
REGDEF_BIT(ime_p1_h_sfact,        16)
REGDEF_BIT(ime_p1_v_sfact,        16)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER2)


/*
    ime_p1_isd_h_base:    [0x0, 0x1fff],            bits : 12_0
    ime_p1_isd_v_base:    [0x0, 0x1fff],            bits : 28_16
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER3_OFS 0x010c
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER3)
REGDEF_BIT(ime_p1_isd_h_base,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p1_isd_v_base,        13)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER3)


/*
    ime_p1_isd_h_sfact0:    [0x0, 0x1fff],          bits : 12_0
    ime_p1_isd_v_sfact0:    [0x0, 0x1fff],          bits : 28_16
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER4_OFS 0x0110
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER4)
REGDEF_BIT(ime_p1_isd_h_sfact0,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p1_isd_v_sfact0,        13)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER4)


/*
    ime_p1_isd_h_sfact1:    [0x0, 0x1fff],          bits : 12_0
    ime_p1_isd_v_sfact1:    [0x0, 0x1fff],          bits : 28_16
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER5_OFS 0x0114
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER5)
REGDEF_BIT(ime_p1_isd_h_sfact1,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p1_isd_v_sfact1,        13)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER5)


/*
    ime_p1_isd_h_sfact2:    [0x0, 0x1fff],          bits : 12_0
    ime_p1_isd_v_sfact2:    [0x0, 0x1fff],          bits : 28_16
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER6_OFS 0x0118
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER6)
REGDEF_BIT(ime_p1_isd_h_sfact2,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p1_isd_v_sfact2,        13)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER6)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_30_OFS 0x011c
REGDEF_BEGIN(IME_RESERVED_REGISTER_30)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_30)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_31_OFS 0x0120
REGDEF_BEGIN(IME_RESERVED_REGISTER_31)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_31)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_32_OFS 0x0124
REGDEF_BEGIN(IME_RESERVED_REGISTER_32)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_32)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_33_OFS 0x0128
REGDEF_BEGIN(IME_RESERVED_REGISTER_33)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_33)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_34_OFS 0x012c
REGDEF_BEGIN(IME_RESERVED_REGISTER_34)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_34)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_35_OFS 0x0130
REGDEF_BEGIN(IME_RESERVED_REGISTER_35)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_35)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_36_OFS 0x0134
REGDEF_BEGIN(IME_RESERVED_REGISTER_36)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_36)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_37_OFS 0x0138
REGDEF_BEGIN(IME_RESERVED_REGISTER_37)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_37)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_38_OFS 0x013c
REGDEF_BEGIN(IME_RESERVED_REGISTER_38)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_38)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_39_OFS 0x0140
REGDEF_BEGIN(IME_RESERVED_REGISTER_39)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_39)


/*
    ime_p1_h_scl_size:    [0x0, 0xffff],            bits : 15_0
    ime_p1_v_scl_size:    [0x0, 0xffff],            bits : 31_16
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER17_OFS 0x0144
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER17)
REGDEF_BIT(ime_p1_h_scl_size,        16)
REGDEF_BIT(ime_p1_v_scl_size,        16)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER17)


/*
    ime_p1_cropout_x:    [0x0, 0xffff],         bits : 15_0
    ime_p1_cropout_y:    [0x0, 0xffff],         bits : 31_16
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER18_OFS 0x0148
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER18)
REGDEF_BIT(ime_p1_cropout_x,        16)
REGDEF_BIT(ime_p1_cropout_y,        16)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER18)


/*
    ime_p1_h_osize:    [0x0, 0xffff],           bits : 15_0
    ime_p1_v_osize:    [0x0, 0xffff],           bits : 31_16
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER19_OFS 0x014c
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER19)
REGDEF_BIT(ime_p1_h_osize,        16)
REGDEF_BIT(ime_p1_v_osize,        16)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER19)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_40_OFS 0x0150
REGDEF_BEGIN(IME_RESERVED_REGISTER_40)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_40)


/*
    ime_p1_y_clamp_min :    [0x0, 0xff],            bits : 7_0
    ime_p1_y_clamp_max :    [0x0, 0xff],            bits : 15_8
    ime_p1_uv_clamp_min:    [0x0, 0xff],            bits : 23_16
    ime_p1_uv_clamp_max:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER21_OFS 0x0154
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER21)
REGDEF_BIT(ime_p1_y_clamp_min,        8)
REGDEF_BIT(ime_p1_y_clamp_max,        8)
REGDEF_BIT(ime_p1_uv_clamp_min,        8)
REGDEF_BIT(ime_p1_uv_clamp_max,        8)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER21)


/*
    ime_p1_y_dram_ofso:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER0_OFS 0x0158
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p1_y_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER0)


/*
    ime_p1_uv_dram_ofso:    [0x0, 0x3ffff],         bits : 19_2
*/
#define IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER1_OFS 0x015c
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p1_uv_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER1)


/*
    ime_p1_y_dram_sao:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER0_OFS 0x0160
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER0)
REGDEF_BIT(ime_p1_y_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER0)


/*
    ime_p1_uv_dram_sao:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER1_OFS 0x0164
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER1)
REGDEF_BIT(ime_p1_uv_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER1)


/*
    ime_p1_y_dram_msb_sao:    [0x0, 0xf],           bits : 3_0
*/
#define IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER2_OFS 0x0168
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER2)
REGDEF_BIT(ime_p1_y_dram_msb_sao,        4)
REGDEF_END(IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER2)


/*
    ime_p1_uv_dram_msb_sao:    [0x0, 0xf],          bits : 3_0
*/
#define IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER3_OFS 0x016c
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER3)
REGDEF_BIT(ime_p1_uv_dram_msb_sao,        4)
REGDEF_END(IME_OUTPUT_PATH1_DMA_BUFFER0_REGISTER3)


/*
    ime_p1_h_scl_init_ofs:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER22_OFS 0x0170
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER22)
REGDEF_BIT(ime_p1_h_scl_init_ofs,        32)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER22)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_43_OFS 0x0174
REGDEF_BEGIN(IME_RESERVED_REGISTER_43)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_43)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_44_OFS 0x0178
REGDEF_BEGIN(IME_RESERVED_REGISTER_44)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_44)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_45_OFS 0x017c
REGDEF_BEGIN(IME_RESERVED_REGISTER_45)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_45)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_46_OFS 0x0180
REGDEF_BEGIN(IME_RESERVED_REGISTER_46)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_46)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_47_OFS 0x0184
REGDEF_BEGIN(IME_RESERVED_REGISTER_47)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_47)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_48_OFS 0x0188
REGDEF_BEGIN(IME_RESERVED_REGISTER_48)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_48)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_49_OFS 0x018c
REGDEF_BEGIN(IME_RESERVED_REGISTER_49)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_49)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_50_OFS 0x0190
REGDEF_BEGIN(IME_RESERVED_REGISTER_50)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_50)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_51_OFS 0x0194
REGDEF_BEGIN(IME_RESERVED_REGISTER_51)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_51)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_52_OFS 0x0198
REGDEF_BEGIN(IME_RESERVED_REGISTER_52)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_52)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_53_OFS 0x019c
REGDEF_BEGIN(IME_RESERVED_REGISTER_53)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_53)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_54_OFS 0x01a0
REGDEF_BEGIN(IME_RESERVED_REGISTER_54)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_54)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_55_OFS 0x01a4
REGDEF_BEGIN(IME_RESERVED_REGISTER_55)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_55)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_56_OFS 0x01a8
REGDEF_BEGIN(IME_RESERVED_REGISTER_56)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_56)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_57_OFS 0x01ac
REGDEF_BEGIN(IME_RESERVED_REGISTER_57)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_57)


/*
    ime_p2_out_type    :    [0x0, 0x1],         bits : 1
    ime_p2_scl_method  :    [0x0, 0x3],         bits : 3_2
    ime_p2_out_en      :    [0x0, 0x1],         bits : 4
    ime_p2_sprt_out_en :    [0x0, 0x1],         bits : 5
    ime_p2_omat        :    [0x0, 0x7],         bits : 10_8
    ime_p2_scl_enh_fact:    [0x0, 0xff],            bits : 19_12
    ime_p2_scl_enh_bit :    [0x0, 0xf],         bits : 23_20
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER0_OFS 0x01b0
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER0)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_p2_out_type,        1)
REGDEF_BIT(ime_p2_scl_method,        2)
REGDEF_BIT(ime_p2_out_en,        1)
REGDEF_BIT(ime_p2_sprt_out_en,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p2_omat,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_p2_scl_enh_fact,        8)
REGDEF_BIT(ime_p2_scl_enh_bit,        4)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER0)


/*
    ime_p2_h_ud      :    [0x0, 0x1],           bits : 0
    ime_p2_v_ud      :    [0x0, 0x1],           bits : 1
    ime_p2_h_dnrate  :    [0x0, 0x1f],          bits : 6_2
    ime_p2_v_dnrate  :    [0x0, 0x1f],          bits : 11_7
    ime_p2_h_filtmode:    [0x0, 0x1],           bits : 15
    ime_p2_h_filtcoef:    [0x0, 0x3f],          bits : 21_16
    ime_p2_v_filtmode:    [0x0, 0x1],           bits : 22
    ime_p2_v_filtcoef:    [0x0, 0x3f],          bits : 28_23
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER1_OFS 0x01b4
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER1)
REGDEF_BIT(ime_p2_h_ud,        1)
REGDEF_BIT(ime_p2_v_ud,        1)
REGDEF_BIT(ime_p2_h_dnrate,        5)
REGDEF_BIT(ime_p2_v_dnrate,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p2_h_filtmode,        1)
REGDEF_BIT(ime_p2_h_filtcoef,        6)
REGDEF_BIT(ime_p2_v_filtmode,        1)
REGDEF_BIT(ime_p2_v_filtcoef,        6)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER1)


/*
    ime_p2_h_sfact:    [0x0, 0xffff],           bits : 15_0
    ime_p2_v_sfact:    [0x0, 0xffff],           bits : 31_16
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER2_OFS 0x01b8
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER2)
REGDEF_BIT(ime_p2_h_sfact,        16)
REGDEF_BIT(ime_p2_v_sfact,        16)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER2)


/*
    ime_p2_isd_h_base:    [0x0, 0x1fff],            bits : 12_0
    ime_p2_isd_v_base:    [0x0, 0x1fff],            bits : 28_16
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER3_OFS 0x01bc
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER3)
REGDEF_BIT(ime_p2_isd_h_base,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p2_isd_v_base,        13)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER3)


/*
    ime_p2_isd_h_sfact0:    [0x0, 0x1fff],          bits : 12_0
    ime_p2_isd_v_sfact0:    [0x0, 0x1fff],          bits : 28_16
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER4_OFS 0x01c0
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER4)
REGDEF_BIT(ime_p2_isd_h_sfact0,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p2_isd_v_sfact0,        13)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER4)


/*
    ime_p2_isd_h_sfact1:    [0x0, 0x1fff],          bits : 12_0
    ime_p2_isd_v_sfact1:    [0x0, 0x1fff],          bits : 28_16
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER5_OFS 0x01c4
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER5)
REGDEF_BIT(ime_p2_isd_h_sfact1,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p2_isd_v_sfact1,        13)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER5)


/*
    ime_p2_isd_h_sfact2:    [0x0, 0x1fff],          bits : 12_0
    ime_p2_isd_v_sfact2:    [0x0, 0x1fff],          bits : 28_16
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER6_OFS 0x01c8
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER6)
REGDEF_BIT(ime_p2_isd_h_sfact2,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p2_isd_v_sfact2,        13)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER6)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_58_OFS 0x01cc
REGDEF_BEGIN(IME_RESERVED_REGISTER_58)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_58)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_59_OFS 0x01d0
REGDEF_BEGIN(IME_RESERVED_REGISTER_59)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_59)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_60_OFS 0x01d4
REGDEF_BEGIN(IME_RESERVED_REGISTER_60)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_60)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_61_OFS 0x01d8
REGDEF_BEGIN(IME_RESERVED_REGISTER_61)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_61)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_62_OFS 0x01dc
REGDEF_BEGIN(IME_RESERVED_REGISTER_62)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_62)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_63_OFS 0x01e0
REGDEF_BEGIN(IME_RESERVED_REGISTER_63)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_63)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_64_OFS 0x01e4
REGDEF_BEGIN(IME_RESERVED_REGISTER_64)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_64)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_65_OFS 0x01e8
REGDEF_BEGIN(IME_RESERVED_REGISTER_65)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_65)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_66_OFS 0x01ec
REGDEF_BEGIN(IME_RESERVED_REGISTER_66)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_66)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_67_OFS 0x01f0
REGDEF_BEGIN(IME_RESERVED_REGISTER_67)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_67)


/*
    ime_p2_h_scl_size:    [0x0, 0xffff],            bits : 15_0
    ime_p2_v_scl_size:    [0x0, 0xffff],            bits : 31_16
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER17_OFS 0x01f4
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER17)
REGDEF_BIT(ime_p2_h_scl_size,        16)
REGDEF_BIT(ime_p2_v_scl_size,        16)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER17)


/*
    ime_p2_cropout_x:    [0x0, 0xffff],         bits : 15_0
    ime_p2_cropout_y:    [0x0, 0xffff],         bits : 31_16
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER18_OFS 0x01f8
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER18)
REGDEF_BIT(ime_p2_cropout_x,        16)
REGDEF_BIT(ime_p2_cropout_y,        16)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER18)


/*
    ime_p2_h_osize:    [0x0, 0xffff],           bits : 15_0
    ime_p2_v_osize:    [0x0, 0xffff],           bits : 31_16
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER19_OFS 0x01fc
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER19)
REGDEF_BIT(ime_p2_h_osize,        16)
REGDEF_BIT(ime_p2_v_osize,        16)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER19)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_68_OFS 0x0200
REGDEF_BEGIN(IME_RESERVED_REGISTER_68)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_68)


/*
    ime_p2_y_clamp_min :    [0x0, 0xff],            bits : 7_0
    ime_p2_y_clamp_max :    [0x0, 0xff],            bits : 15_8
    ime_p2_uv_clamp_min:    [0x0, 0xff],            bits : 23_16
    ime_p2_uv_clamp_max:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER21_OFS 0x0204
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER21)
REGDEF_BIT(ime_p2_y_clamp_min,        8)
REGDEF_BIT(ime_p2_y_clamp_max,        8)
REGDEF_BIT(ime_p2_uv_clamp_min,        8)
REGDEF_BIT(ime_p2_uv_clamp_max,        8)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER21)


/*
    ime_p2_y_dram_ofso:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER0_OFS 0x0208
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p2_y_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER0)


/*
    ime_p2_uv_dram_ofso:    [0x0, 0x3ffff],         bits : 19_2
*/
#define IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER1_OFS 0x020c
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p2_uv_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER1)


/*
    ime_p2_y_dram_sao:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER0_OFS 0x0210
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER0)
REGDEF_BIT(ime_p2_y_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER0)


/*
    ime_p2_uv_dram_sao:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER1_OFS 0x0214
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER1)
REGDEF_BIT(ime_p2_uv_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER1)


/*
    ime_p2_y_dram_msb_sao:    [0x0, 0xf],           bits : 3_0
*/
#define IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER2_OFS 0x0218
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER2)
REGDEF_BIT(ime_p2_y_dram_msb_sao,        4)
REGDEF_END(IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER2)


/*
    ime_p2_uv_dram_msb_sao:    [0x0, 0xf],          bits : 3_0
*/
#define IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER3_OFS 0x021c
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER3)
REGDEF_BIT(ime_p2_uv_dram_msb_sao,        4)
REGDEF_END(IME_OUTPUT_PATH2_DMA_BUFFER0_REGISTER3)


/*
    ime_p2_h_scl_init_ofs:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER22_OFS 0x0220
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER22)
REGDEF_BIT(ime_p2_h_scl_init_ofs,        32)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER22)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_71_OFS 0x0224
REGDEF_BEGIN(IME_RESERVED_REGISTER_71)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_71)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_72_OFS 0x0228
REGDEF_BEGIN(IME_RESERVED_REGISTER_72)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_72)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_73_OFS 0x022c
REGDEF_BEGIN(IME_RESERVED_REGISTER_73)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_73)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_74_OFS 0x0230
REGDEF_BEGIN(IME_RESERVED_REGISTER_74)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_74)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_75_OFS 0x0234
REGDEF_BEGIN(IME_RESERVED_REGISTER_75)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_75)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_76_OFS 0x0238
REGDEF_BEGIN(IME_RESERVED_REGISTER_76)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_76)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_77_OFS 0x023c
REGDEF_BEGIN(IME_RESERVED_REGISTER_77)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_77)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_78_OFS 0x0240
REGDEF_BEGIN(IME_RESERVED_REGISTER_78)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_78)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_79_OFS 0x0244
REGDEF_BEGIN(IME_RESERVED_REGISTER_79)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_79)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_80_OFS 0x0248
REGDEF_BEGIN(IME_RESERVED_REGISTER_80)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_80)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_81_OFS 0x024c
REGDEF_BEGIN(IME_RESERVED_REGISTER_81)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_81)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_82_OFS 0x0250
REGDEF_BEGIN(IME_RESERVED_REGISTER_82)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_82)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_83_OFS 0x0254
REGDEF_BEGIN(IME_RESERVED_REGISTER_83)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_83)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_84_OFS 0x0258
REGDEF_BEGIN(IME_RESERVED_REGISTER_84)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_84)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_85_OFS 0x025c
REGDEF_BEGIN(IME_RESERVED_REGISTER_85)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_85)


/*
    ime_p3_out_type    :    [0x0, 0x1],         bits : 1
    ime_p3_scl_method  :    [0x0, 0x3],         bits : 3_2
    ime_p3_out_en      :    [0x0, 0x1],         bits : 4
    ime_p3_sprt_out_en :    [0x0, 0x1],         bits : 5
    ime_p3_omat        :    [0x0, 0x7],         bits : 10_8
    ime_p3_scl_enh_fact:    [0x0, 0xff],            bits : 19_12
    ime_p3_scl_enh_bit :    [0x0, 0xf],         bits : 23_20
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER0_OFS 0x0260
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER0)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_p3_out_type,        1)
REGDEF_BIT(ime_p3_scl_method,        2)
REGDEF_BIT(ime_p3_out_en,        1)
REGDEF_BIT(ime_p3_sprt_out_en,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p3_omat,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_p3_scl_enh_fact,        8)
REGDEF_BIT(ime_p3_scl_enh_bit,        4)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER0)


/*
    ime_p3_h_ud      :    [0x0, 0x1],           bits : 0
    ime_p3_v_ud      :    [0x0, 0x1],           bits : 1
    ime_p3_h_dnrate  :    [0x0, 0x1f],          bits : 6_2
    ime_p3_v_dnrate  :    [0x0, 0x1f],          bits : 11_7
    ime_p3_h_filtmode:    [0x0, 0x1],           bits : 15
    ime_p3_h_filtcoef:    [0x0, 0x3f],          bits : 21_16
    ime_p3_v_filtmode:    [0x0, 0x1],           bits : 22
    ime_p3_v_filtcoef:    [0x0, 0x3f],          bits : 28_23
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER1_OFS 0x0264
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER1)
REGDEF_BIT(ime_p3_h_ud,        1)
REGDEF_BIT(ime_p3_v_ud,        1)
REGDEF_BIT(ime_p3_h_dnrate,        5)
REGDEF_BIT(ime_p3_v_dnrate,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_p3_h_filtmode,        1)
REGDEF_BIT(ime_p3_h_filtcoef,        6)
REGDEF_BIT(ime_p3_v_filtmode,        1)
REGDEF_BIT(ime_p3_v_filtcoef,        6)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER1)


/*
    ime_p3_h_sfact:    [0x0, 0xffff],           bits : 15_0
    ime_p3_v_sfact:    [0x0, 0xffff],           bits : 31_16
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER2_OFS 0x0268
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER2)
REGDEF_BIT(ime_p3_h_sfact,        16)
REGDEF_BIT(ime_p3_v_sfact,        16)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER2)


/*
    Reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER3_OFS 0x026c
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER3)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER3)


/*
    Reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER4_OFS 0x0270
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER4)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER4)


/*
    Reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER5_OFS 0x0274
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER5)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER5)


/*
    ime_p3_coef_mode :    [0x0, 0x1],           bits : 0
    ime_p3_h_filt_adj:    [0x0, 0x3f],          bits : 21_16
    ime_p3_v_filt_adj:    [0x0, 0x3f],          bits : 29_24
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER0_OFS 0x0278
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER0)
REGDEF_BIT(ime_p3_coef_mode,        1)
REGDEF_BIT(,        15)
REGDEF_BIT(ime_p3_h_filt_adj,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p3_v_filt_adj,        6)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER0)


/*
    ime_p3_isd2_ucoef0:    [0x0, 0xff],         bits : 7_0
    ime_p3_isd2_ucoef1:    [0x0, 0xff],         bits : 15_8
    ime_p3_isd2_ucoef2:    [0x0, 0xff],         bits : 23_16
    ime_p3_isd2_ucoef3:    [0x0, 0xff],         bits : 31_24
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER1_OFS 0x027c
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER1)
REGDEF_BIT(ime_p3_isd2_ucoef0,        8)
REGDEF_BIT(ime_p3_isd2_ucoef1,        8)
REGDEF_BIT(ime_p3_isd2_ucoef2,        8)
REGDEF_BIT(ime_p3_isd2_ucoef3,        8)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER1)


/*
    ime_p3_isd2_ucoef4:    [0x0, 0xff],         bits : 7_0
    ime_p3_isd2_ucoef5:    [0x0, 0xff],         bits : 15_8
    ime_p3_isd2_ucoef6:    [0x0, 0xff],         bits : 23_16
    ime_p3_isd2_ucoef7:    [0x0, 0xff],         bits : 31_24
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER2_OFS 0x0280
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER2)
REGDEF_BIT(ime_p3_isd2_ucoef4,        8)
REGDEF_BIT(ime_p3_isd2_ucoef5,        8)
REGDEF_BIT(ime_p3_isd2_ucoef6,        8)
REGDEF_BIT(ime_p3_isd2_ucoef7,        8)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER2)


/*
    ime_p3_isd2_ucoef8 :    [0x0, 0xff],            bits : 7_0
    ime_p3_isd2_ucoef9 :    [0x0, 0xff],            bits : 15_8
    ime_p3_isd2_ucoef10:    [0x0, 0xff],            bits : 23_16
    ime_p3_isd2_ucoef11:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER3_OFS 0x0284
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER3)
REGDEF_BIT(ime_p3_isd2_ucoef8,        8)
REGDEF_BIT(ime_p3_isd2_ucoef9,        8)
REGDEF_BIT(ime_p3_isd2_ucoef10,        8)
REGDEF_BIT(ime_p3_isd2_ucoef11,        8)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER3)


/*
    ime_p3_isd2_ucoef12:    [0x0, 0xff],            bits : 7_0
    ime_p3_isd2_ucoef13:    [0x0, 0xff],            bits : 15_8
    ime_p3_isd2_ucoef14:    [0x0, 0xff],            bits : 23_16
    ime_p3_isd2_ucoef15:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER4_OFS 0x0288
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER4)
REGDEF_BIT(ime_p3_isd2_ucoef12,        8)
REGDEF_BIT(ime_p3_isd2_ucoef13,        8)
REGDEF_BIT(ime_p3_isd2_ucoef14,        8)
REGDEF_BIT(ime_p3_isd2_ucoef15,        8)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER4)


/*
    ime_p3_isd2_ucoef16:    [0x0, 0xff],            bits : 7_0
    ime_p3_isd2_ucoef17:    [0x0, 0xff],            bits : 15_8
    ime_p3_isd2_ucoef18:    [0x0, 0xff],            bits : 23_16
    ime_p3_isd2_ucoef19:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER5_OFS 0x028c
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER5)
REGDEF_BIT(ime_p3_isd2_ucoef16,        8)
REGDEF_BIT(ime_p3_isd2_ucoef17,        8)
REGDEF_BIT(ime_p3_isd2_ucoef18,        8)
REGDEF_BIT(ime_p3_isd2_ucoef19,        8)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER5)


/*
    ime_p3_isd2_ucoef20:    [0x0, 0xff],            bits : 7_0
    ime_p3_isd2_ucoef21:    [0x0, 0xff],            bits : 15_8
    ime_p3_isd2_ucoef22:    [0x0, 0xff],            bits : 23_16
    ime_p3_isd2_ucoef23:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER6_OFS 0x0290
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER6)
REGDEF_BIT(ime_p3_isd2_ucoef20,        8)
REGDEF_BIT(ime_p3_isd2_ucoef21,        8)
REGDEF_BIT(ime_p3_isd2_ucoef22,        8)
REGDEF_BIT(ime_p3_isd2_ucoef23,        8)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER6)


/*
    ime_p3_isd2_ucoef24:    [0x0, 0xff],            bits : 7_0
    ime_p3_isd2_ucoef25:    [0x0, 0xff],            bits : 15_8
    ime_p3_isd2_ucoef26:    [0x0, 0xff],            bits : 23_16
    ime_p3_isd2_ucoef27:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER7_OFS 0x0294
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER7)
REGDEF_BIT(ime_p3_isd2_ucoef24,        8)
REGDEF_BIT(ime_p3_isd2_ucoef25,        8)
REGDEF_BIT(ime_p3_isd2_ucoef26,        8)
REGDEF_BIT(ime_p3_isd2_ucoef27,        8)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER7)


/*
    ime_p3_isd2_ucoef28:    [0x0, 0xff],            bits : 7_0
    ime_p3_isd2_ucoef29:    [0x0, 0xff],            bits : 15_8
    ime_p3_isd2_ucoef30:    [0x0, 0xff],            bits : 23_16
    ime_p3_isd2_ucoef31:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER8_OFS 0x0298
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER8)
REGDEF_BIT(ime_p3_isd2_ucoef28,        8)
REGDEF_BIT(ime_p3_isd2_ucoef29,        8)
REGDEF_BIT(ime_p3_isd2_ucoef30,        8)
REGDEF_BIT(ime_p3_isd2_ucoef31,        8)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER8)


/*
    ime_p3_isd2_h_norm:    [0x0, 0xfffff],          bits : 19_0
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER9_OFS 0x029c
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER9)
REGDEF_BIT(ime_p3_isd2_h_norm,        20)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER9)


/*
    ime_p3_isd2_v_norm:    [0x0, 0xfffff],          bits : 19_0
*/
#define IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER10_OFS 0x02a0
REGDEF_BEGIN(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER10)
REGDEF_BIT(ime_p3_isd2_v_norm,        20)
REGDEF_END(IME_OUTPUT_PATH3_ISD2_CONTROL_REGISTER10)


/*
    ime_p3_h_scl_size:    [0x0, 0xffff],            bits : 15_0
    ime_p3_v_scl_size:    [0x0, 0xffff],            bits : 31_16
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER17_OFS 0x02a4
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER17)
REGDEF_BIT(ime_p3_h_scl_size,        16)
REGDEF_BIT(ime_p3_v_scl_size,        16)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER17)


/*
    ime_p3_cropout_x:    [0x0, 0xffff],         bits : 15_0
    ime_p3_cropout_y:    [0x0, 0xffff],         bits : 31_16
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER18_OFS 0x02a8
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER18)
REGDEF_BIT(ime_p3_cropout_x,        16)
REGDEF_BIT(ime_p3_cropout_y,        16)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER18)


/*
    ime_p3_h_osize:    [0x0, 0xffff],           bits : 15_0
    ime_p3_v_osize:    [0x0, 0xffff],           bits : 31_16
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER19_OFS 0x02ac
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER19)
REGDEF_BIT(ime_p3_h_osize,        16)
REGDEF_BIT(ime_p3_v_osize,        16)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER19)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_106_OFS 0x02b0
REGDEF_BEGIN(IME_RESERVED_REGISTER_106)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_106)


/*
    ime_p3_y_clamp_min :    [0x0, 0xff],            bits : 7_0
    ime_p3_y_clamp_max :    [0x0, 0xff],            bits : 15_8
    ime_p3_uv_clamp_min:    [0x0, 0xff],            bits : 23_16
    ime_p3_uv_clamp_max:    [0x0, 0xff],            bits : 31_24
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER21_OFS 0x02b4
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER21)
REGDEF_BIT(ime_p3_y_clamp_min,        8)
REGDEF_BIT(ime_p3_y_clamp_max,        8)
REGDEF_BIT(ime_p3_uv_clamp_min,        8)
REGDEF_BIT(ime_p3_uv_clamp_max,        8)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER21)


/*
    ime_p3_y_dram_ofso:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_OUTPUT_PATH3_DMA_LINEOFFSET_REGISTER0_OFS 0x02b8
REGDEF_BEGIN(IME_OUTPUT_PATH3_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p3_y_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH3_DMA_LINEOFFSET_REGISTER0)


/*
    ime_p3_uv_dram_ofso:    [0x0, 0x3ffff],         bits : 19_2
*/
#define IME_OUTPUT_PATH3_DMA_LINEOFFSET_REGISTER1_OFS 0x02bc
REGDEF_BEGIN(IME_OUTPUT_PATH3_DMA_LINEOFFSET_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p3_uv_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH3_DMA_LINEOFFSET_REGISTER1)


/*
    ime_p3_y_dram_sao:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER0_OFS 0x02c0
REGDEF_BEGIN(IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER0)
REGDEF_BIT(ime_p3_y_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER0)


/*
    ime_p3_uv_dram_sao:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER1_OFS 0x02c4
REGDEF_BEGIN(IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER1)
REGDEF_BIT(ime_p3_uv_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER1)


/*
    ime_p3_y_dram_msb_sao:    [0x0, 0xf],           bits : 3_0
*/
#define IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER2_OFS 0x02c8
REGDEF_BEGIN(IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER2)
REGDEF_BIT(ime_p3_y_dram_msb_sao,        4)
REGDEF_END(IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER2)


/*
    ime_p3_uv_dram_msb_sao:    [0x0, 0xf],          bits : 3_0
*/
#define IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER3_OFS 0x02cc
REGDEF_BEGIN(IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER3)
REGDEF_BIT(ime_p3_uv_dram_msb_sao,        4)
REGDEF_END(IME_OUTPUT_PATH3_DMA_BUFFER0_REGISTER3)


/*
    ime_p3_h_scl_init_ofs:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER22_OFS 0x02d0
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER22)
REGDEF_BIT(ime_p3_h_scl_init_ofs,        32)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER22)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_115_OFS 0x02d4
REGDEF_BEGIN(IME_RESERVED_REGISTER_115)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_115)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_116_OFS 0x02d8
REGDEF_BEGIN(IME_RESERVED_REGISTER_116)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_116)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_117_OFS 0x02dc
REGDEF_BEGIN(IME_RESERVED_REGISTER_117)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_117)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_118_OFS 0x02e0
REGDEF_BEGIN(IME_RESERVED_REGISTER_118)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_118)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_119_OFS 0x02e4
REGDEF_BEGIN(IME_RESERVED_REGISTER_119)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_119)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_120_OFS 0x02e8
REGDEF_BEGIN(IME_RESERVED_REGISTER_120)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_120)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_121_OFS 0x02ec
REGDEF_BEGIN(IME_RESERVED_REGISTER_121)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_121)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_122_OFS 0x02f0
REGDEF_BEGIN(IME_RESERVED_REGISTER_122)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_122)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_123_OFS 0x02f4
REGDEF_BEGIN(IME_RESERVED_REGISTER_123)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_123)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_124_OFS 0x02f8
REGDEF_BEGIN(IME_RESERVED_REGISTER_124)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_124)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_125_OFS 0x02fc
REGDEF_BEGIN(IME_RESERVED_REGISTER_125)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_125)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_126_OFS 0x0300
REGDEF_BEGIN(IME_RESERVED_REGISTER_126)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_126)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_127_OFS 0x0304
REGDEF_BEGIN(IME_RESERVED_REGISTER_127)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_127)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_128_OFS 0x0308
REGDEF_BEGIN(IME_RESERVED_REGISTER_128)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_128)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_129_OFS 0x030c
REGDEF_BEGIN(IME_RESERVED_REGISTER_129)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_129)


/*
    lca_subin_h_size:    [0x0, 0xffff],         bits : 15_0
    lca_subin_v_size:    [0x0, 0xffff],         bits : 31_16
*/
#define IME_LCA_INPUT_IMAGE_REGISTER0_OFS 0x0310
REGDEF_BEGIN(IME_LCA_INPUT_IMAGE_REGISTER0)
REGDEF_BIT(lca_subin_h_size,        16)
REGDEF_BIT(lca_subin_v_size,        16)
REGDEF_END(IME_LCA_INPUT_IMAGE_REGISTER0)


/*
    lca_su_h_fact:    [0x0, 0xffff],            bits : 15_0
    lca_su_v_fact:    [0x0, 0xffff],            bits : 31_16
*/
#define IME_CHROMA_ADAPTATION_INPUT_IMAGE_REGISTER1_OFS 0x0314
REGDEF_BEGIN(IME_CHROMA_ADAPTATION_INPUT_IMAGE_REGISTER1)
REGDEF_BIT(lca_su_h_fact,        16)
REGDEF_BIT(lca_su_v_fact,        16)
REGDEF_END(IME_CHROMA_ADAPTATION_INPUT_IMAGE_REGISTER1)


/*
    lca_su_h_init_ofs:    [0x0, 0x1fffff],          bits : 20_0
*/
#define IME_CHROMA_ADAPTATION_SUBIMAGE_OUTPUT_REGISTER1_OFS 0x0318
REGDEF_BEGIN(IME_CHROMA_ADAPTATION_SUBIMAGE_OUTPUT_REGISTER1)
REGDEF_BIT(lca_su_h_init_ofs,        21)
REGDEF_END(IME_CHROMA_ADAPTATION_SUBIMAGE_OUTPUT_REGISTER1)


/*
    lca_su_v_init_ofs:    [0x0, 0x1fffff],          bits : 20_0
*/
#define IME_CHROMA_ADAPTATION_SUBIMAGE_OUTPUT_REGISTER2_OFS 0x031c
REGDEF_BEGIN(IME_CHROMA_ADAPTATION_SUBIMAGE_OUTPUT_REGISTER2)
REGDEF_BIT(lca_su_v_init_ofs,        21)
REGDEF_END(IME_CHROMA_ADAPTATION_SUBIMAGE_OUTPUT_REGISTER2)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_130_OFS 0x0320
REGDEF_BEGIN(IME_RESERVED_REGISTER_130)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_130)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_131_OFS 0x0324
REGDEF_BEGIN(IME_RESERVED_REGISTER_131)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_131)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_132_OFS 0x0328
REGDEF_BEGIN(IME_RESERVED_REGISTER_132)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_132)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_133_OFS 0x032c
REGDEF_BEGIN(IME_RESERVED_REGISTER_133)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_133)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_134_OFS 0x0330
REGDEF_BEGIN(IME_RESERVED_REGISTER_134)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_134)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_135_OFS 0x0334
REGDEF_BEGIN(IME_RESERVED_REGISTER_135)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_135)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_136_OFS 0x0338
REGDEF_BEGIN(IME_RESERVED_REGISTER_136)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_136)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_137_OFS 0x033c
REGDEF_BEGIN(IME_RESERVED_REGISTER_137)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_137)


/*
    ime_pm_pxl_sd_h_dnrate:    [0x0, 0xf],          bits : 3_0
    ime_pm_pxl_sd_v_dnrate:    [0x0, 0xf],          bits : 7_4
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER0_OFS 0x0340
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER0)
REGDEF_BIT(ime_pm_pxl_sd_h_dnrate,        4)
REGDEF_BIT(ime_pm_pxl_sd_v_dnrate,        4)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER0)


/*
    ime_pm_pxl_sd_h_fact:    [0x0, 0xffff],         bits : 15_0
    ime_pm_pxl_sd_v_fact:    [0x0, 0xffff],         bits : 31_16
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER1_OFS 0x0344
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER1)
REGDEF_BIT(ime_pm_pxl_sd_h_fact,        16)
REGDEF_BIT(ime_pm_pxl_sd_v_fact,        16)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER1)


/*
    ime_pm_pxl_sd_isd_h_base:    [0x0, 0x1fff],         bits : 12_0
    ime_pm_pxl_sd_isd_v_base:    [0x0, 0x1fff],         bits : 28_16
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER2_OFS 0x0348
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER2)
REGDEF_BIT(ime_pm_pxl_sd_isd_h_base,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm_pxl_sd_isd_v_base,        13)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER2)


/*
    ime_pm_pxl_sd_isd_h_fact0:    [0x0, 0x1fff],            bits : 12_0
    ime_pm_pxl_sd_isd_v_fact0:    [0x0, 0x1fff],            bits : 28_16
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER3_OFS 0x034c
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER3)
REGDEF_BIT(ime_pm_pxl_sd_isd_h_fact0,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm_pxl_sd_isd_v_fact0,        13)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER3)


/*
    ime_pm_pxl_sd_isd_h_fact1:    [0x0, 0x1fff],            bits : 12_0
    ime_pm_pxl_sd_isd_v_fact1:    [0x0, 0x1fff],            bits : 28_16
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER4_OFS 0x0350
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER4)
REGDEF_BIT(ime_pm_pxl_sd_isd_h_fact1,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm_pxl_sd_isd_v_fact1,        13)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER4)


/*
    ime_pm_pxl_sd_isd_h_fact2:    [0x0, 0x1fff],            bits : 12_0
    ime_pm_pxl_sd_isd_v_fact2:    [0x0, 0x1fff],            bits : 28_16
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER5_OFS 0x0354
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER5)
REGDEF_BIT(ime_pm_pxl_sd_isd_h_fact2,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm_pxl_sd_isd_v_fact2,        13)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER5)


/*
    ime_pm_subout_h_size:    [0x0, 0x7ff],          bits : 10_0
    ime_pm_subout_v_size:    [0x0, 0x7ff],          bits : 26_16
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER6_OFS 0x0358
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER6)
REGDEF_BIT(ime_pm_subout_h_size,        11)
REGDEF_BIT(,        5)
REGDEF_BIT(ime_pm_subout_v_size,        11)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER6)


/*
    ime_pm_pxl_dram_ofso:    [0x0, 0x3ffff],            bits : 19_2
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER7_OFS 0x035c
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER7)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm_pxl_dram_ofso,        18)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER7)


/*
    ime_pm_pxl_dram_sao:    [0x0, 0x3fffffff],          bits : 31_2
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER8_OFS 0x0360
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER8)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm_pxl_dram_sao,        30)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER8)


/*
    ime_pm_pxl_dram_msb_sao:    [0x0, 0xf],         bits : 3_0
*/
#define IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER9_OFS 0x0364
REGDEF_BEGIN(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER9)
REGDEF_BIT(ime_pm_pxl_dram_msb_sao,        4)
REGDEF_END(IME_PRIVACY_MASK_PIXELATION_SUBIMAGE_OUTPUT_REGISTER9)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_139_OFS 0x0368
REGDEF_BEGIN(IME_RESERVED_REGISTER_139)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_139)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_140_OFS 0x036c
REGDEF_BEGIN(IME_RESERVED_REGISTER_140)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_140)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_141_OFS 0x0370
REGDEF_BEGIN(IME_RESERVED_REGISTER_141)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_141)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_142_OFS 0x0374
REGDEF_BEGIN(IME_RESERVED_REGISTER_142)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_142)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_143_OFS 0x0378
REGDEF_BEGIN(IME_RESERVED_REGISTER_143)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_143)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_144_OFS 0x037c
REGDEF_BEGIN(IME_RESERVED_REGISTER_144)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_144)


/*
    dbcs_ctr_u  :    [0x0, 0xff],           bits : 7_0
    dbcs_ctr_v  :    [0x0, 0xff],           bits : 15_8
    dbcs_mode   :    [0x0, 0x3],            bits : 17_16
    dbcs_step_y :    [0x0, 0x3],            bits : 21_20
    dbcs_step_uv:    [0x0, 0x3],            bits : 23_22
*/
#define IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_REGISTER0_OFS 0x0380
REGDEF_BEGIN(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_REGISTER0)
REGDEF_BIT(dbcs_ctr_u,        8)
REGDEF_BIT(dbcs_ctr_v,        8)
REGDEF_BIT(dbcs_mode,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(dbcs_step_y,        2)
REGDEF_BIT(dbcs_step_uv,        2)
REGDEF_END(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_REGISTER0)


/*
    dbcs_y_wt0:    [0x0, 0x1f],         bits : 4_0
    dbcs_y_wt1:    [0x0, 0x1f],         bits : 9_5
    dbcs_y_wt2:    [0x0, 0x1f],         bits : 14_10
    dbcs_y_wt3:    [0x0, 0x1f],         bits : 19_15
    dbcs_y_wt4:    [0x0, 0x1f],         bits : 24_20
    dbcs_y_wt5:    [0x0, 0x1f],         bits : 29_25
*/
#define IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER0_OFS 0x0384
REGDEF_BEGIN(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER0)
REGDEF_BIT(dbcs_y_wt0,        5)
REGDEF_BIT(dbcs_y_wt1,        5)
REGDEF_BIT(dbcs_y_wt2,        5)
REGDEF_BIT(dbcs_y_wt3,        5)
REGDEF_BIT(dbcs_y_wt4,        5)
REGDEF_BIT(dbcs_y_wt5,        5)
REGDEF_END(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER0)


/*
    dbcs_y_wt6 :    [0x0, 0x1f],            bits : 4_0
    dbcs_y_wt7 :    [0x0, 0x1f],            bits : 9_5
    dbcs_y_wt8 :    [0x0, 0x1f],            bits : 14_10
    dbcs_y_wt9 :    [0x0, 0x1f],            bits : 19_15
    dbcs_y_wt10:    [0x0, 0x1f],            bits : 24_20
    dbcs_y_wt11:    [0x0, 0x1f],            bits : 29_25
*/
#define IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER1_OFS 0x0388
REGDEF_BEGIN(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER1)
REGDEF_BIT(dbcs_y_wt6,        5)
REGDEF_BIT(dbcs_y_wt7,        5)
REGDEF_BIT(dbcs_y_wt8,        5)
REGDEF_BIT(dbcs_y_wt9,        5)
REGDEF_BIT(dbcs_y_wt10,        5)
REGDEF_BIT(dbcs_y_wt11,        5)
REGDEF_END(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER1)


/*
    dbcs_y_wt12:    [0x0, 0x1f],            bits : 4_0
    dbcs_y_wt13:    [0x0, 0x1f],            bits : 9_5
    dbcs_y_wt14:    [0x0, 0x1f],            bits : 14_10
    dbcs_y_wt15:    [0x0, 0x1f],            bits : 19_15
*/
#define IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER2_OFS 0x038c
REGDEF_BEGIN(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER2)
REGDEF_BIT(dbcs_y_wt12,        5)
REGDEF_BIT(dbcs_y_wt13,        5)
REGDEF_BIT(dbcs_y_wt14,        5)
REGDEF_BIT(dbcs_y_wt15,        5)
REGDEF_END(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER2)


/*
    dbcs_c_wt0:    [0x0, 0x1f],         bits : 4_0
    dbcs_c_wt1:    [0x0, 0x1f],         bits : 9_5
    dbcs_c_wt2:    [0x0, 0x1f],         bits : 14_10
    dbcs_c_wt3:    [0x0, 0x1f],         bits : 19_15
    dbcs_c_wt4:    [0x0, 0x1f],         bits : 24_20
    dbcs_c_wt5:    [0x0, 0x1f],         bits : 29_25
*/
#define IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER3_OFS 0x0390
REGDEF_BEGIN(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER3)
REGDEF_BIT(dbcs_c_wt0,        5)
REGDEF_BIT(dbcs_c_wt1,        5)
REGDEF_BIT(dbcs_c_wt2,        5)
REGDEF_BIT(dbcs_c_wt3,        5)
REGDEF_BIT(dbcs_c_wt4,        5)
REGDEF_BIT(dbcs_c_wt5,        5)
REGDEF_END(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER3)


/*
    dbcs_c_wt6 :    [0x0, 0x1f],            bits : 4_0
    dbcs_c_wt7 :    [0x0, 0x1f],            bits : 9_5
    dbcs_c_wt8 :    [0x0, 0x1f],            bits : 14_10
    dbcs_c_wt9 :    [0x0, 0x1f],            bits : 19_15
    dbcs_c_wt10:    [0x0, 0x1f],            bits : 24_20
    dbcs_c_wt11:    [0x0, 0x1f],            bits : 29_25
*/
#define IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER4_OFS 0x0394
REGDEF_BEGIN(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER4)
REGDEF_BIT(dbcs_c_wt6,        5)
REGDEF_BIT(dbcs_c_wt7,        5)
REGDEF_BIT(dbcs_c_wt8,        5)
REGDEF_BIT(dbcs_c_wt9,        5)
REGDEF_BIT(dbcs_c_wt10,        5)
REGDEF_BIT(dbcs_c_wt11,        5)
REGDEF_END(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER4)


/*
    dbcs_c_wt12:    [0x0, 0x1f],            bits : 4_0
    dbcs_c_wt13:    [0x0, 0x1f],            bits : 9_5
    dbcs_c_wt14:    [0x0, 0x1f],            bits : 14_10
    dbcs_c_wt15:    [0x0, 0x1f],            bits : 19_15
*/
#define IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER5_OFS 0x0398
REGDEF_BEGIN(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER5)
REGDEF_BIT(dbcs_c_wt12,        5)
REGDEF_BIT(dbcs_c_wt13,        5)
REGDEF_BIT(dbcs_c_wt14,        5)
REGDEF_BIT(dbcs_c_wt15,        5)
REGDEF_END(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_WEIGHTING_REGISTER5)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_145_OFS 0x039c
REGDEF_BEGIN(IME_RESERVED_REGISTER_145)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_145)


/*
    ime_feye_gain_centx:    [0x0, 0x7fff],          bits : 14_0
    ime_feye_gain_centy:    [0x0, 0x7fff],          bits : 30_16
*/
#define FISHEYE_GAIN_SETTING_REGISTER_0_OFS 0x03a0
REGDEF_BEGIN(FISHEYE_GAIN_SETTING_REGISTER_0)
REGDEF_BIT(ime_feye_gain_centx,        15)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_feye_gain_centy,        15)
REGDEF_END(FISHEYE_GAIN_SETTING_REGISTER_0)


/*
    ime_feye_gain_decrase_range:    [0x0, 0xff],            bits : 7_0
    ime_feye_gain_radius       :    [0x0, 0x7fff],          bits : 30_16
*/
#define FISHEYE_GAIN_SETTING_REGISTER_1_OFS 0x03a4
REGDEF_BEGIN(FISHEYE_GAIN_SETTING_REGISTER_1)
REGDEF_BIT(ime_feye_gain_decrase_range,        8)
REGDEF_BIT(,        8)
REGDEF_BIT(ime_feye_gain_radius,        15)
REGDEF_END(FISHEYE_GAIN_SETTING_REGISTER_1)


/*
    ime_feye_gain_divide_mul  :    [0x0, 0x7fff],           bits : 14_0
    ime_feye_gain_divide_shift:    [0x0, 0xf],          bits : 19_16
*/
#define FISHEYE_GAIN_SETTING_REGISTER_2_OFS 0x03a8
REGDEF_BEGIN(FISHEYE_GAIN_SETTING_REGISTER_2)
REGDEF_BIT(ime_feye_gain_divide_mul,        15)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_feye_gain_divide_shift,        4)
REGDEF_END(FISHEYE_GAIN_SETTING_REGISTER_2)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_146_OFS 0x03ac
REGDEF_BEGIN(IME_RESERVED_REGISTER_146)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_146)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_147_OFS 0x03b0
REGDEF_BEGIN(IME_RESERVED_REGISTER_147)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_147)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_148_OFS 0x03b4
REGDEF_BEGIN(IME_RESERVED_REGISTER_148)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_148)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_149_OFS 0x03b8
REGDEF_BEGIN(IME_RESERVED_REGISTER_149)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_149)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_150_OFS 0x03bc
REGDEF_BEGIN(IME_RESERVED_REGISTER_150)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_150)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_151_OFS 0x03c0
REGDEF_BEGIN(IME_RESERVED_REGISTER_151)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_151)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_152_OFS 0x03c4
REGDEF_BEGIN(IME_RESERVED_REGISTER_152)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_152)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_153_OFS 0x03c8
REGDEF_BEGIN(IME_RESERVED_REGISTER_153)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_153)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_154_OFS 0x03cc
REGDEF_BEGIN(IME_RESERVED_REGISTER_154)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_154)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_155_OFS 0x03d0
REGDEF_BEGIN(IME_RESERVED_REGISTER_155)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_155)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_156_OFS 0x03d4
REGDEF_BEGIN(IME_RESERVED_REGISTER_156)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_156)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_157_OFS 0x03d8
REGDEF_BEGIN(IME_RESERVED_REGISTER_157)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_157)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_158_OFS 0x03dc
REGDEF_BEGIN(IME_RESERVED_REGISTER_158)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_158)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_159_OFS 0x03e0
REGDEF_BEGIN(IME_RESERVED_REGISTER_159)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_159)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_160_OFS 0x03e4
REGDEF_BEGIN(IME_RESERVED_REGISTER_160)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_160)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_161_OFS 0x03e8
REGDEF_BEGIN(IME_RESERVED_REGISTER_161)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_161)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_162_OFS 0x03ec
REGDEF_BEGIN(IME_RESERVED_REGISTER_162)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_162)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_163_OFS 0x03f0
REGDEF_BEGIN(IME_RESERVED_REGISTER_163)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_163)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_164_OFS 0x03f4
REGDEF_BEGIN(IME_RESERVED_REGISTER_164)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_164)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_165_OFS 0x03f8
REGDEF_BEGIN(IME_RESERVED_REGISTER_165)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_165)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_166_OFS 0x03fc
REGDEF_BEGIN(IME_RESERVED_REGISTER_166)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_166)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_167_OFS 0x0400
REGDEF_BEGIN(IME_RESERVED_REGISTER_167)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_167)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_168_OFS 0x0404
REGDEF_BEGIN(IME_RESERVED_REGISTER_168)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_168)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_169_OFS 0x0408
REGDEF_BEGIN(IME_RESERVED_REGISTER_169)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_169)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_170_OFS 0x040c
REGDEF_BEGIN(IME_RESERVED_REGISTER_170)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_170)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_171_OFS 0x0410
REGDEF_BEGIN(IME_RESERVED_REGISTER_171)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_171)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_172_OFS 0x0414
REGDEF_BEGIN(IME_RESERVED_REGISTER_172)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_172)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_173_OFS 0x0418
REGDEF_BEGIN(IME_RESERVED_REGISTER_173)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_173)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_174_OFS 0x041c
REGDEF_BEGIN(IME_RESERVED_REGISTER_174)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_174)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_175_OFS 0x0420
REGDEF_BEGIN(IME_RESERVED_REGISTER_175)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_175)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_176_OFS 0x0424
REGDEF_BEGIN(IME_RESERVED_REGISTER_176)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_176)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_177_OFS 0x0428
REGDEF_BEGIN(IME_RESERVED_REGISTER_177)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_177)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_178_OFS 0x042c
REGDEF_BEGIN(IME_RESERVED_REGISTER_178)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_178)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_179_OFS 0x0430
REGDEF_BEGIN(IME_RESERVED_REGISTER_179)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_179)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_180_OFS 0x0434
REGDEF_BEGIN(IME_RESERVED_REGISTER_180)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_180)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_181_OFS 0x0438
REGDEF_BEGIN(IME_RESERVED_REGISTER_181)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_181)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_182_OFS 0x043c
REGDEF_BEGIN(IME_RESERVED_REGISTER_182)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_182)


/*
    ime_p0_sprt_hbl:    [0x0, 0xffff],          bits : 15_0
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER10_OFS 0x0440
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER10)
REGDEF_BIT(ime_p0_sprt_hbl,        16)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER10)


/*
    ime_p0_y1_dram_ofso:    [0x0, 0x3ffff],         bits : 19_2
*/
#define IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER2_OFS 0x0444
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p0_y1_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER2)


/*
    ime_p0_uv1_dram_ofso:    [0x0, 0x3ffff],            bits : 19_2
*/
#define IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER3_OFS 0x0448
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER3)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p0_uv1_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH0_DMA_LINEOFFSET_REGISTER3)


/*
    ime_p0_y1_dram_sao:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_OUTPUT_PATH0_DMA_BUFFER1_REGISTER3_OFS 0x044c
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_BUFFER1_REGISTER3)
REGDEF_BIT(ime_p0_y1_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH0_DMA_BUFFER1_REGISTER3)


/*
    ime_p0_uv1_dram_sao:    [0x0, 0xffffffff],          bits : 31_0
*/
#define IME_OUTPUT_PATH0_DMA_BUFFER1_REGISTER4_OFS 0x0450
REGDEF_BEGIN(IME_OUTPUT_PATH0_DMA_BUFFER1_REGISTER4)
REGDEF_BIT(ime_p0_uv1_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH0_DMA_BUFFER1_REGISTER4)


/*
    ime_p1_sprt_hbl:    [0x0, 0xffff],          bits : 15_0
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER10_OFS 0x0454
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER10)
REGDEF_BIT(ime_p1_sprt_hbl,        16)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER10)


/*
    ime_p1_y1_dram_ofso:    [0x0, 0x3ffff],         bits : 19_2
*/
#define IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER2_OFS 0x0458
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p1_y1_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER2)


/*
    ime_p1_uv1_dram_ofso:    [0x0, 0x3ffff],            bits : 19_2
*/
#define IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER3_OFS 0x045c
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER3)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p1_uv1_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH1_DMA_LINEOFFSET_REGISTER3)


/*
    ime_p1_y1_dram_sao:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_OUTPUT_PATH1_DMA_BUFFER2_REGISTER3_OFS 0x0460
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_BUFFER2_REGISTER3)
REGDEF_BIT(ime_p1_y1_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH1_DMA_BUFFER2_REGISTER3)


/*
    ime_p1_y1_dram_sao:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_OUTPUT_PATH1_DMA_BUFFER1_REGISTER4_OFS 0x0464
REGDEF_BEGIN(IME_OUTPUT_PATH1_DMA_BUFFER1_REGISTER4)
REGDEF_BIT(ime_p1_y1_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH1_DMA_BUFFER1_REGISTER4)


/*
    ime_p2_sprt_hbl:    [0x0, 0xffff],          bits : 15_0
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER10_OFS 0x0468
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER10)
REGDEF_BIT(ime_p2_sprt_hbl,        16)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER10)


/*
    ime_p2_y1_dram_ofso:    [0x0, 0x3ffff],         bits : 19_2
*/
#define IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER2_OFS 0x046c
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p2_y1_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER2)


/*
    ime_p2_uv1_dram_ofso:    [0x0, 0x3ffff],            bits : 19_2
*/
#define IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER3_OFS 0x0470
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER3)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_p2_uv1_dram_ofso,        18)
REGDEF_END(IME_OUTPUT_PATH2_DMA_LINEOFFSET_REGISTER3)


/*
    ime_p2_y1_dram_sao:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_OUTPUT_PATH2_DMA_BUFFER2_REGISTER3_OFS 0x0474
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_BUFFER2_REGISTER3)
REGDEF_BIT(ime_p2_y1_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH2_DMA_BUFFER2_REGISTER3)


/*
    ime_p2_y1_dram_sao:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_OUTPUT_PATH2_DMA_BUFFER1_REGISTER4_OFS 0x0478
REGDEF_BEGIN(IME_OUTPUT_PATH2_DMA_BUFFER1_REGISTER4)
REGDEF_BIT(ime_p2_y1_dram_sao,        32)
REGDEF_END(IME_OUTPUT_PATH2_DMA_BUFFER1_REGISTER4)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_183_OFS 0x047c
REGDEF_BEGIN(IME_RESERVED_REGISTER_183)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_183)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_184_OFS 0x0480
REGDEF_BEGIN(IME_RESERVED_REGISTER_184)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_184)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_185_OFS 0x0484
REGDEF_BEGIN(IME_RESERVED_REGISTER_185)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_185)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_186_OFS 0x0488
REGDEF_BEGIN(IME_RESERVED_REGISTER_186)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_186)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_187_OFS 0x048c
REGDEF_BEGIN(IME_RESERVED_REGISTER_187)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_187)


/*
    ime_pm0_line4_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm0_line4_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm0_line4_signa:    [0x0, 0x1],         bits : 17
    ime_pm0_line4_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm0_line4_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET0_REGISTER9_OFS 0x0490
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER9)
REGDEF_BIT(ime_pm0_line4_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm0_line4_coefa,        13)
REGDEF_BIT(ime_pm0_line4_signa,        1)
REGDEF_BIT(ime_pm0_line4_coefb,        13)
REGDEF_BIT(ime_pm0_line4_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER9)


/*
    ime_pm0_line4_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm0_line4_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET0_REGISTER10_OFS 0x0494
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER10)
REGDEF_BIT(ime_pm0_line4_coefc,        26)
REGDEF_BIT(ime_pm0_line4_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER10)


/*
    ime_pm0_line5_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm0_line5_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm0_line5_signa:    [0x0, 0x1],         bits : 17
    ime_pm0_line5_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm0_line5_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET0_REGISTER11_OFS 0x0498
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER11)
REGDEF_BIT(ime_pm0_line5_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm0_line5_coefa,        13)
REGDEF_BIT(ime_pm0_line5_signa,        1)
REGDEF_BIT(ime_pm0_line5_coefb,        13)
REGDEF_BIT(ime_pm0_line5_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER11)


/*
    ime_pm0_line5_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm0_line5_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET0_REGISTER12_OFS 0x049c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER12)
REGDEF_BIT(ime_pm0_line5_coefc,        26)
REGDEF_BIT(ime_pm0_line5_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER12)


/*
    ime_pm2_line4_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm2_line4_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm2_line4_signa:    [0x0, 0x1],         bits : 17
    ime_pm2_line4_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm2_line4_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET2_REGISTER9_OFS 0x04a0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER9)
REGDEF_BIT(ime_pm2_line4_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm2_line4_coefa,        13)
REGDEF_BIT(ime_pm2_line4_signa,        1)
REGDEF_BIT(ime_pm2_line4_coefb,        13)
REGDEF_BIT(ime_pm2_line4_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER9)


/*
    ime_pm2_line4_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm2_line4_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET2_REGISTER10_OFS 0x04a4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER10)
REGDEF_BIT(ime_pm2_line4_coefc,        26)
REGDEF_BIT(ime_pm2_line4_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER10)


/*
    ime_pm2_line5_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm2_line5_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm2_line5_signa:    [0x0, 0x1],         bits : 17
    ime_pm2_line5_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm2_line5_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET2_REGISTER11_OFS 0x04a8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER11)
REGDEF_BIT(ime_pm2_line5_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm2_line5_coefa,        13)
REGDEF_BIT(ime_pm2_line5_signa,        1)
REGDEF_BIT(ime_pm2_line5_coefb,        13)
REGDEF_BIT(ime_pm2_line5_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER11)


/*
    ime_pm2_line5_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm2_line5_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET2_REGISTER12_OFS 0x04ac
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER12)
REGDEF_BIT(ime_pm2_line5_coefc,        26)
REGDEF_BIT(ime_pm2_line5_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER12)


/*
    ime_pm4_line4_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm4_line4_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm4_line4_signa:    [0x0, 0x1],         bits : 17
    ime_pm4_line4_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm4_line4_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET4_REGISTER9_OFS 0x04b0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER9)
REGDEF_BIT(ime_pm4_line4_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm4_line4_coefa,        13)
REGDEF_BIT(ime_pm4_line4_signa,        1)
REGDEF_BIT(ime_pm4_line4_coefb,        13)
REGDEF_BIT(ime_pm4_line4_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER9)


/*
    ime_pm4_line4_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm4_line4_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET4_REGISTER10_OFS 0x04b4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER10)
REGDEF_BIT(ime_pm4_line4_coefc,        26)
REGDEF_BIT(ime_pm4_line4_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER10)


/*
    ime_pm4_line5_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm4_line5_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm4_line5_signa:    [0x0, 0x1],         bits : 17
    ime_pm4_line5_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm4_line5_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET4_REGISTER11_OFS 0x04b8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER11)
REGDEF_BIT(ime_pm4_line5_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm4_line5_coefa,        13)
REGDEF_BIT(ime_pm4_line5_signa,        1)
REGDEF_BIT(ime_pm4_line5_coefb,        13)
REGDEF_BIT(ime_pm4_line5_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER11)


/*
    ime_pm4_line5_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm4_line5_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET4_REGISTER12_OFS 0x04bc
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER12)
REGDEF_BIT(ime_pm4_line5_coefc,        26)
REGDEF_BIT(ime_pm4_line5_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER12)


/*
    ime_pm6_line4_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm6_line4_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm6_line4_signa:    [0x0, 0x1],         bits : 17
    ime_pm6_line4_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm6_line4_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET6_REGISTER9_OFS 0x04c0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER9)
REGDEF_BIT(ime_pm6_line4_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm6_line4_coefa,        13)
REGDEF_BIT(ime_pm6_line4_signa,        1)
REGDEF_BIT(ime_pm6_line4_coefb,        13)
REGDEF_BIT(ime_pm6_line4_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER9)


/*
    ime_pm6_line4_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm6_line4_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET6_REGISTER10_OFS 0x04c4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER10)
REGDEF_BIT(ime_pm6_line4_coefc,        26)
REGDEF_BIT(ime_pm6_line4_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER10)


/*
    ime_pm6_line5_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm6_line5_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm6_line5_signa:    [0x0, 0x1],         bits : 17
    ime_pm6_line5_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm6_line5_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET6_REGISTER11_OFS 0x04c8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER11)
REGDEF_BIT(ime_pm6_line5_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm6_line5_coefa,        13)
REGDEF_BIT(ime_pm6_line5_signa,        1)
REGDEF_BIT(ime_pm6_line5_coefb,        13)
REGDEF_BIT(ime_pm6_line5_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER11)


/*
    ime_pm6_line5_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm6_line5_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET6_REGISTER12_OFS 0x04cc
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER12)
REGDEF_BIT(ime_pm6_line5_coefc,        26)
REGDEF_BIT(ime_pm6_line5_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER12)


/*
    ime_pm0_type     :    [0x0, 0x1],           bits : 0
    ime_pm_pxlsize   :    [0x0, 0x3],           bits : 2_1
    ime_pm_proc_loc  :    [0x0, 0x1],           bits : 3
    ime_pm0_color_y  :    [0x0, 0xff],          bits : 11_4
    ime_pm0_color_u  :    [0x0, 0xff],          bits : 19_12
    ime_pm0_color_v  :    [0x0, 0xff],          bits : 27_20
    ime_pm_pxl_src   :    [0x0, 0x3],           bits : 29_28
    ime_pm0_shape_sel:    [0x0, 0x1],           bits : 31
*/
#define IME_PRIVACY_MASK_SET0_REGISTER0_OFS 0x04d0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER0)
REGDEF_BIT(ime_pm0_type,        1)
REGDEF_BIT(ime_pm_pxlsize,        2)
REGDEF_BIT(ime_pm_proc_loc,        1)
REGDEF_BIT(ime_pm0_color_y,        8)
REGDEF_BIT(ime_pm0_color_u,        8)
REGDEF_BIT(ime_pm0_color_v,        8)
REGDEF_BIT(ime_pm_pxl_src,        2)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_pm0_shape_sel,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER0)


/*
    ime_pm0_line0_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm0_line0_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm0_line0_signa:    [0x0, 0x1],         bits : 17
    ime_pm0_line0_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm0_line0_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET0_REGISTER1_OFS 0x04d4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER1)
REGDEF_BIT(ime_pm0_line0_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm0_line0_coefa,        13)
REGDEF_BIT(ime_pm0_line0_signa,        1)
REGDEF_BIT(ime_pm0_line0_coefb,        13)
REGDEF_BIT(ime_pm0_line0_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER1)


/*
    ime_pm0_line0_coefc  :    [0x0, 0x3ffffff],         bits : 25_0
    ime_pm0_line0_signc  :    [0x0, 0x1],           bits : 26
    ime_pm0_comb_mode_sel:    [0x0, 0x3],           bits : 31_30
*/
#define IME_PRIVACY_MASK_SET0_REGISTER2_OFS 0x04d8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER2)
REGDEF_BIT(ime_pm0_line0_coefc,        26)
REGDEF_BIT(ime_pm0_line0_signc,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm0_comb_mode_sel,        2)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER2)


/*
    ime_pm0_line1_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm0_line1_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm0_line1_signa:    [0x0, 0x1],         bits : 17
    ime_pm0_line1_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm0_line1_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET0_REGISTER3_OFS 0x04dc
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER3)
REGDEF_BIT(ime_pm0_line1_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm0_line1_coefa,        13)
REGDEF_BIT(ime_pm0_line1_signa,        1)
REGDEF_BIT(ime_pm0_line1_coefb,        13)
REGDEF_BIT(ime_pm0_line1_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER3)


/*
    ime_pm0_line1_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm0_line1_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET0_REGISTER4_OFS 0x04e0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER4)
REGDEF_BIT(ime_pm0_line1_coefc,        26)
REGDEF_BIT(ime_pm0_line1_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER4)


/*
    ime_pm0_line2_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm0_line2_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm0_line2_signa:    [0x0, 0x1],         bits : 17
    ime_pm0_line2_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm0_line2_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET0_REGISTER5_OFS 0x04e4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER5)
REGDEF_BIT(ime_pm0_line2_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm0_line2_coefa,        13)
REGDEF_BIT(ime_pm0_line2_signa,        1)
REGDEF_BIT(ime_pm0_line2_coefb,        13)
REGDEF_BIT(ime_pm0_line2_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER5)


/*
    ime_pm0_line2_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm0_line2_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET0_REGISTER6_OFS 0x04e8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER6)
REGDEF_BIT(ime_pm0_line2_coefc,        26)
REGDEF_BIT(ime_pm0_line2_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER6)


/*
    ime_pm0_line3_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm0_line3_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm0_line3_signa:    [0x0, 0x1],         bits : 17
    ime_pm0_line3_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm0_line3_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET0_REGISTER7_OFS 0x04ec
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER7)
REGDEF_BIT(ime_pm0_line3_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm0_line3_coefa,        13)
REGDEF_BIT(ime_pm0_line3_signa,        1)
REGDEF_BIT(ime_pm0_line3_coefb,        13)
REGDEF_BIT(ime_pm0_line3_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER7)


/*
    ime_pm0_line3_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm0_line3_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET0_REGISTER8_OFS 0x04f0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET0_REGISTER8)
REGDEF_BIT(ime_pm0_line3_coefc,        26)
REGDEF_BIT(ime_pm0_line3_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET0_REGISTER8)


/*
    ime_pm1_type   :    [0x0, 0x1],         bits : 0
    ime_pm1_color_y:    [0x0, 0xff],            bits : 11_4
    ime_pm1_color_u:    [0x0, 0xff],            bits : 19_12
    ime_pm1_color_v:    [0x0, 0xff],            bits : 27_20
*/
#define IME_PRIVACY_MASK_SET1_REGISTER0_OFS 0x04f4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET1_REGISTER0)
REGDEF_BIT(ime_pm1_type,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm1_color_y,        8)
REGDEF_BIT(ime_pm1_color_u,        8)
REGDEF_BIT(ime_pm1_color_v,        8)
REGDEF_END(IME_PRIVACY_MASK_SET1_REGISTER0)


/*
    ime_pm1_line0_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm1_line0_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm1_line0_signa:    [0x0, 0x1],         bits : 17
    ime_pm1_line0_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm1_line0_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET1_REGISTER1_OFS 0x04f8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET1_REGISTER1)
REGDEF_BIT(ime_pm1_line0_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm1_line0_coefa,        13)
REGDEF_BIT(ime_pm1_line0_signa,        1)
REGDEF_BIT(ime_pm1_line0_coefb,        13)
REGDEF_BIT(ime_pm1_line0_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET1_REGISTER1)


/*
    ime_pm1_line0_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm1_line0_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET1_REGISTER2_OFS 0x04fc
REGDEF_BEGIN(IME_PRIVACY_MASK_SET1_REGISTER2)
REGDEF_BIT(ime_pm1_line0_coefc,        26)
REGDEF_BIT(ime_pm1_line0_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET1_REGISTER2)


/*
    ime_pm1_line1_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm1_line1_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm1_line1_signa:    [0x0, 0x1],         bits : 17
    ime_pm1_line1_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm1_line1_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET1_REGISTER3_OFS 0x0500
REGDEF_BEGIN(IME_PRIVACY_MASK_SET1_REGISTER3)
REGDEF_BIT(ime_pm1_line1_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm1_line1_coefa,        13)
REGDEF_BIT(ime_pm1_line1_signa,        1)
REGDEF_BIT(ime_pm1_line1_coefb,        13)
REGDEF_BIT(ime_pm1_line1_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET1_REGISTER3)


/*
    ime_pm1_line1_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm1_line1_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET1_REGISTER4_OFS 0x0504
REGDEF_BEGIN(IME_PRIVACY_MASK_SET1_REGISTER4)
REGDEF_BIT(ime_pm1_line1_coefc,        26)
REGDEF_BIT(ime_pm1_line1_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET1_REGISTER4)


/*
    ime_pm1_line2_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm1_line2_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm1_line2_signa:    [0x0, 0x1],         bits : 17
    ime_pm1_line2_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm1_line2_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET1_REGISTER5_OFS 0x0508
REGDEF_BEGIN(IME_PRIVACY_MASK_SET1_REGISTER5)
REGDEF_BIT(ime_pm1_line2_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm1_line2_coefa,        13)
REGDEF_BIT(ime_pm1_line2_signa,        1)
REGDEF_BIT(ime_pm1_line2_coefb,        13)
REGDEF_BIT(ime_pm1_line2_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET1_REGISTER5)


/*
    ime_pm1_line2_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm1_line2_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET1_REGISTER6_OFS 0x050c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET1_REGISTER6)
REGDEF_BIT(ime_pm1_line2_coefc,        26)
REGDEF_BIT(ime_pm1_line2_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET1_REGISTER6)


/*
    ime_pm1_line3_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm1_line3_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm1_line3_signa:    [0x0, 0x1],         bits : 17
    ime_pm1_line3_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm1_line3_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET1_REGISTER7_OFS 0x0510
REGDEF_BEGIN(IME_PRIVACY_MASK_SET1_REGISTER7)
REGDEF_BIT(ime_pm1_line3_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm1_line3_coefa,        13)
REGDEF_BIT(ime_pm1_line3_signa,        1)
REGDEF_BIT(ime_pm1_line3_coefb,        13)
REGDEF_BIT(ime_pm1_line3_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET1_REGISTER7)


/*
    ime_pm1_line3_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm1_line3_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET1_REGISTER8_OFS 0x0514
REGDEF_BEGIN(IME_PRIVACY_MASK_SET1_REGISTER8)
REGDEF_BIT(ime_pm1_line3_coefc,        26)
REGDEF_BIT(ime_pm1_line3_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET1_REGISTER8)


/*
    ime_pm2_type     :    [0x0, 0x1],           bits : 0
    ime_pm2_color_y  :    [0x0, 0xff],          bits : 11_4
    ime_pm2_color_u  :    [0x0, 0xff],          bits : 19_12
    ime_pm2_color_v  :    [0x0, 0xff],          bits : 27_20
    ime_pm2_shape_sel:    [0x0, 0x1],           bits : 31
*/
#define IME_PRIVACY_MASK_SET2_REGISTER0_OFS 0x0518
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER0)
REGDEF_BIT(ime_pm2_type,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm2_color_y,        8)
REGDEF_BIT(ime_pm2_color_u,        8)
REGDEF_BIT(ime_pm2_color_v,        8)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm2_shape_sel,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER0)


/*
    ime_pm2_line0_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm2_line0_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm2_line0_signa:    [0x0, 0x1],         bits : 17
    ime_pm2_line0_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm2_line0_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET2_REGISTER1_OFS 0x051c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER1)
REGDEF_BIT(ime_pm2_line0_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm2_line0_coefa,        13)
REGDEF_BIT(ime_pm2_line0_signa,        1)
REGDEF_BIT(ime_pm2_line0_coefb,        13)
REGDEF_BIT(ime_pm2_line0_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER1)


/*
    ime_pm2_line0_coefc  :    [0x0, 0x3ffffff],         bits : 25_0
    ime_pm2_line0_signc  :    [0x0, 0x1],           bits : 26
    ime_pm2_comb_mode_sel:    [0x0, 0x3],           bits : 31_30
*/
#define IME_PRIVACY_MASK_SET2_REGISTER2_OFS 0x0520
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER2)
REGDEF_BIT(ime_pm2_line0_coefc,        26)
REGDEF_BIT(ime_pm2_line0_signc,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm2_comb_mode_sel,        2)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER2)


/*
    ime_pm2_line1_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm2_line1_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm2_line1_signa:    [0x0, 0x1],         bits : 17
    ime_pm2_line1_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm2_line1_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET2_REGISTER3_OFS 0x0524
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER3)
REGDEF_BIT(ime_pm2_line1_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm2_line1_coefa,        13)
REGDEF_BIT(ime_pm2_line1_signa,        1)
REGDEF_BIT(ime_pm2_line1_coefb,        13)
REGDEF_BIT(ime_pm2_line1_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER3)


/*
    ime_pm2_line1_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm2_line1_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET2_REGISTER4_OFS 0x0528
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER4)
REGDEF_BIT(ime_pm2_line1_coefc,        26)
REGDEF_BIT(ime_pm2_line1_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER4)


/*
    ime_pm2_line2_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm2_line2_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm2_line2_signa:    [0x0, 0x1],         bits : 17
    ime_pm2_line2_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm2_line2_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET2_REGISTER5_OFS 0x052c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER5)
REGDEF_BIT(ime_pm2_line2_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm2_line2_coefa,        13)
REGDEF_BIT(ime_pm2_line2_signa,        1)
REGDEF_BIT(ime_pm2_line2_coefb,        13)
REGDEF_BIT(ime_pm2_line2_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER5)


/*
    ime_pm2_line2_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm2_line2_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET2_REGISTER6_OFS 0x0530
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER6)
REGDEF_BIT(ime_pm2_line2_coefc,        26)
REGDEF_BIT(ime_pm2_line2_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER6)


/*
    ime_pm2_line3_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm2_line3_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm2_line3_signa:    [0x0, 0x1],         bits : 17
    ime_pm2_line3_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm2_line3_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET2_REGISTER7_OFS 0x0534
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER7)
REGDEF_BIT(ime_pm2_line3_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm2_line3_coefa,        13)
REGDEF_BIT(ime_pm2_line3_signa,        1)
REGDEF_BIT(ime_pm2_line3_coefb,        13)
REGDEF_BIT(ime_pm2_line3_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER7)


/*
    ime_pm2_line3_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm2_line3_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET2_REGISTER8_OFS 0x0538
REGDEF_BEGIN(IME_PRIVACY_MASK_SET2_REGISTER8)
REGDEF_BIT(ime_pm2_line3_coefc,        26)
REGDEF_BIT(ime_pm2_line3_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET2_REGISTER8)


/*
    ime_pm3_type   :    [0x0, 0x1],         bits : 0
    ime_pm3_color_y:    [0x0, 0xff],            bits : 11_4
    ime_pm3_color_u:    [0x0, 0xff],            bits : 19_12
    ime_pm3_color_v:    [0x0, 0xff],            bits : 27_20
*/
#define IME_PRIVACY_MASK_SET3_REGISTER0_OFS 0x053c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET3_REGISTER0)
REGDEF_BIT(ime_pm3_type,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm3_color_y,        8)
REGDEF_BIT(ime_pm3_color_u,        8)
REGDEF_BIT(ime_pm3_color_v,        8)
REGDEF_END(IME_PRIVACY_MASK_SET3_REGISTER0)


/*
    ime_pm3_line0_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm3_line0_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm3_line0_signa:    [0x0, 0x1],         bits : 17
    ime_pm3_line0_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm3_line0_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET3_REGISTER1_OFS 0x0540
REGDEF_BEGIN(IME_PRIVACY_MASK_SET3_REGISTER1)
REGDEF_BIT(ime_pm3_line0_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm3_line0_coefa,        13)
REGDEF_BIT(ime_pm3_line0_signa,        1)
REGDEF_BIT(ime_pm3_line0_coefb,        13)
REGDEF_BIT(ime_pm3_line0_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET3_REGISTER1)


/*
    ime_pm3_line0_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm3_line0_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET3_REGISTER2_OFS 0x0544
REGDEF_BEGIN(IME_PRIVACY_MASK_SET3_REGISTER2)
REGDEF_BIT(ime_pm3_line0_coefc,        26)
REGDEF_BIT(ime_pm3_line0_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET3_REGISTER2)


/*
    ime_pm3_line1_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm3_line1_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm3_line1_signa:    [0x0, 0x1],         bits : 17
    ime_pm3_line1_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm3_line1_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET3_REGISTER3_OFS 0x0548
REGDEF_BEGIN(IME_PRIVACY_MASK_SET3_REGISTER3)
REGDEF_BIT(ime_pm3_line1_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm3_line1_coefa,        13)
REGDEF_BIT(ime_pm3_line1_signa,        1)
REGDEF_BIT(ime_pm3_line1_coefb,        13)
REGDEF_BIT(ime_pm3_line1_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET3_REGISTER3)


/*
    ime_pm3_line1_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm3_line1_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET3_REGISTER4_OFS 0x054c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET3_REGISTER4)
REGDEF_BIT(ime_pm3_line1_coefc,        26)
REGDEF_BIT(ime_pm3_line1_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET3_REGISTER4)


/*
    ime_pm3_line2_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm3_line2_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm3_line2_signa:    [0x0, 0x1],         bits : 17
    ime_pm3_line2_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm3_line2_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET3_REGISTER5_OFS 0x0550
REGDEF_BEGIN(IME_PRIVACY_MASK_SET3_REGISTER5)
REGDEF_BIT(ime_pm3_line2_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm3_line2_coefa,        13)
REGDEF_BIT(ime_pm3_line2_signa,        1)
REGDEF_BIT(ime_pm3_line2_coefb,        13)
REGDEF_BIT(ime_pm3_line2_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET3_REGISTER5)


/*
    ime_pm3_line2_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm3_line2_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET3_REGISTER6_OFS 0x0554
REGDEF_BEGIN(IME_PRIVACY_MASK_SET3_REGISTER6)
REGDEF_BIT(ime_pm3_line2_coefc,        26)
REGDEF_BIT(ime_pm3_line2_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET3_REGISTER6)


/*
    ime_pm3_line3_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm3_line3_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm3_line3_signa:    [0x0, 0x1],         bits : 17
    ime_pm3_line3_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm3_line3_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET3_REGISTER7_OFS 0x0558
REGDEF_BEGIN(IME_PRIVACY_MASK_SET3_REGISTER7)
REGDEF_BIT(ime_pm3_line3_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm3_line3_coefa,        13)
REGDEF_BIT(ime_pm3_line3_signa,        1)
REGDEF_BIT(ime_pm3_line3_coefb,        13)
REGDEF_BIT(ime_pm3_line3_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET3_REGISTER7)


/*
    ime_pm3_line3_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm3_line3_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET3_REGISTER8_OFS 0x055c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET3_REGISTER8)
REGDEF_BIT(ime_pm3_line3_coefc,        26)
REGDEF_BIT(ime_pm3_line3_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET3_REGISTER8)


/*
    ime_pm0_awet:    [0x0, 0xff],           bits : 7_0
    ime_pm1_awet:    [0x0, 0xff],           bits : 15_8
    ime_pm2_awet:    [0x0, 0xff],           bits : 23_16
    ime_pm3_awet:    [0x0, 0xff],           bits : 31_24
*/
#define IME_PRIVACY_MASK_ALPHA_REGISTER0_OFS 0x0560
REGDEF_BEGIN(IME_PRIVACY_MASK_ALPHA_REGISTER0)
REGDEF_BIT(ime_pm0_awet,        8)
REGDEF_BIT(ime_pm1_awet,        8)
REGDEF_BIT(ime_pm2_awet,        8)
REGDEF_BIT(ime_pm3_awet,        8)
REGDEF_END(IME_PRIVACY_MASK_ALPHA_REGISTER0)


/*
    ime_pm4_type     :    [0x0, 0x1],           bits : 0
    ime_pm4_color_y  :    [0x0, 0xff],          bits : 11_4
    ime_pm4_color_u  :    [0x0, 0xff],          bits : 19_12
    ime_pm4_color_v  :    [0x0, 0xff],          bits : 27_20
    ime_pm4_shape_sel:    [0x0, 0x1],           bits : 31
*/
#define IME_PRIVACY_MASK_SET4_REGISTER0_OFS 0x0564
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER0)
REGDEF_BIT(ime_pm4_type,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm4_color_y,        8)
REGDEF_BIT(ime_pm4_color_u,        8)
REGDEF_BIT(ime_pm4_color_v,        8)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm4_shape_sel,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER0)


/*
    ime_pm4_line0_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm4_line0_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm4_line0_signa:    [0x0, 0x1],         bits : 17
    ime_pm4_line0_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm4_line0_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET4_REGISTER1_OFS 0x0568
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER1)
REGDEF_BIT(ime_pm4_line0_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm4_line0_coefa,        13)
REGDEF_BIT(ime_pm4_line0_signa,        1)
REGDEF_BIT(ime_pm4_line0_coefb,        13)
REGDEF_BIT(ime_pm4_line0_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER1)


/*
    ime_pm4_line0_coefc  :    [0x0, 0x3ffffff],         bits : 25_0
    ime_pm4_line0_signc  :    [0x0, 0x1],           bits : 26
    ime_pm4_comb_mode_sel:    [0x0, 0x3],           bits : 31_30
*/
#define IME_PRIVACY_MASK_SET4_REGISTER2_OFS 0x056c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER2)
REGDEF_BIT(ime_pm4_line0_coefc,        26)
REGDEF_BIT(ime_pm4_line0_signc,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm4_comb_mode_sel,        2)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER2)


/*
    ime_pm4_line1_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm4_line1_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm4_line1_signa:    [0x0, 0x1],         bits : 17
    ime_pm4_line1_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm4_line1_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET4_REGISTER3_OFS 0x0570
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER3)
REGDEF_BIT(ime_pm4_line1_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm4_line1_coefa,        13)
REGDEF_BIT(ime_pm4_line1_signa,        1)
REGDEF_BIT(ime_pm4_line1_coefb,        13)
REGDEF_BIT(ime_pm4_line1_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER3)


/*
    ime_pm4_line1_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm4_line1_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET4_REGISTER4_OFS 0x0574
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER4)
REGDEF_BIT(ime_pm4_line1_coefc,        26)
REGDEF_BIT(ime_pm4_line1_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER4)


/*
    ime_pm4_line2_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm4_line2_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm4_line2_signa:    [0x0, 0x1],         bits : 17
    ime_pm4_line2_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm4_line2_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET4_REGISTER5_OFS 0x0578
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER5)
REGDEF_BIT(ime_pm4_line2_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm4_line2_coefa,        13)
REGDEF_BIT(ime_pm4_line2_signa,        1)
REGDEF_BIT(ime_pm4_line2_coefb,        13)
REGDEF_BIT(ime_pm4_line2_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER5)


/*
    ime_pm4_line2_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm4_line2_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET4_REGISTER6_OFS 0x057c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER6)
REGDEF_BIT(ime_pm4_line2_coefc,        26)
REGDEF_BIT(ime_pm4_line2_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER6)


/*
    ime_pm4_line3_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm4_line3_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm4_line3_signa:    [0x0, 0x1],         bits : 17
    ime_pm4_line3_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm4_line3_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET4_REGISTER7_OFS 0x0580
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER7)
REGDEF_BIT(ime_pm4_line3_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm4_line3_coefa,        13)
REGDEF_BIT(ime_pm4_line3_signa,        1)
REGDEF_BIT(ime_pm4_line3_coefb,        13)
REGDEF_BIT(ime_pm4_line3_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER7)


/*
    ime_pm4_line3_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm4_line3_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET4_REGISTER8_OFS 0x0584
REGDEF_BEGIN(IME_PRIVACY_MASK_SET4_REGISTER8)
REGDEF_BIT(ime_pm4_line3_coefc,        26)
REGDEF_BIT(ime_pm4_line3_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET4_REGISTER8)


/*
    ime_pm5_type   :    [0x0, 0x1],         bits : 0
    ime_pm5_color_y:    [0x0, 0xff],            bits : 11_4
    ime_pm5_color_u:    [0x0, 0xff],            bits : 19_12
    ime_pm5_color_v:    [0x0, 0xff],            bits : 27_20
*/
#define IME_PRIVACY_MASK_SET5_REGISTER0_OFS 0x0588
REGDEF_BEGIN(IME_PRIVACY_MASK_SET5_REGISTER0)
REGDEF_BIT(ime_pm5_type,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm5_color_y,        8)
REGDEF_BIT(ime_pm5_color_u,        8)
REGDEF_BIT(ime_pm5_color_v,        8)
REGDEF_END(IME_PRIVACY_MASK_SET5_REGISTER0)


/*
    ime_pm5_line0_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm5_line0_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm5_line0_signa:    [0x0, 0x1],         bits : 17
    ime_pm5_line0_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm5_line0_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET5_REGISTER1_OFS 0x058c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET5_REGISTER1)
REGDEF_BIT(ime_pm5_line0_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm5_line0_coefa,        13)
REGDEF_BIT(ime_pm5_line0_signa,        1)
REGDEF_BIT(ime_pm5_line0_coefb,        13)
REGDEF_BIT(ime_pm5_line0_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET5_REGISTER1)


/*
    ime_pm5_line0_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm5_line0_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET5_REGISTER2_OFS 0x0590
REGDEF_BEGIN(IME_PRIVACY_MASK_SET5_REGISTER2)
REGDEF_BIT(ime_pm5_line0_coefc,        26)
REGDEF_BIT(ime_pm5_line0_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET5_REGISTER2)


/*
    ime_pm5_line1_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm5_line1_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm5_line1_signa:    [0x0, 0x1],         bits : 17
    ime_pm5_line1_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm5_line1_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET5_REGISTER3_OFS 0x0594
REGDEF_BEGIN(IME_PRIVACY_MASK_SET5_REGISTER3)
REGDEF_BIT(ime_pm5_line1_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm5_line1_coefa,        13)
REGDEF_BIT(ime_pm5_line1_signa,        1)
REGDEF_BIT(ime_pm5_line1_coefb,        13)
REGDEF_BIT(ime_pm5_line1_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET5_REGISTER3)


/*
    ime_pm5_line1_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm5_line1_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET5_REGISTER4_OFS 0x0598
REGDEF_BEGIN(IME_PRIVACY_MASK_SET5_REGISTER4)
REGDEF_BIT(ime_pm5_line1_coefc,        26)
REGDEF_BIT(ime_pm5_line1_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET5_REGISTER4)


/*
    ime_pm5_line2_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm5_line2_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm5_line2_signa:    [0x0, 0x1],         bits : 17
    ime_pm5_line2_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm5_line2_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET5_REGISTER5_OFS 0x059c
REGDEF_BEGIN(IME_PRIVACY_MASK_SET5_REGISTER5)
REGDEF_BIT(ime_pm5_line2_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm5_line2_coefa,        13)
REGDEF_BIT(ime_pm5_line2_signa,        1)
REGDEF_BIT(ime_pm5_line2_coefb,        13)
REGDEF_BIT(ime_pm5_line2_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET5_REGISTER5)


/*
    ime_pm5_line2_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm5_line2_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET5_REGISTER6_OFS 0x05a0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET5_REGISTER6)
REGDEF_BIT(ime_pm5_line2_coefc,        26)
REGDEF_BIT(ime_pm5_line2_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET5_REGISTER6)


/*
    ime_pm5_line3_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm5_line3_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm5_line3_signa:    [0x0, 0x1],         bits : 17
    ime_pm5_line3_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm5_line3_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET5_REGISTER7_OFS 0x05a4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET5_REGISTER7)
REGDEF_BIT(ime_pm5_line3_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm5_line3_coefa,        13)
REGDEF_BIT(ime_pm5_line3_signa,        1)
REGDEF_BIT(ime_pm5_line3_coefb,        13)
REGDEF_BIT(ime_pm5_line3_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET5_REGISTER7)


/*
    ime_pm5_line3_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm5_line3_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET5_REGISTER8_OFS 0x05a8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET5_REGISTER8)
REGDEF_BIT(ime_pm5_line3_coefc,        26)
REGDEF_BIT(ime_pm5_line3_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET5_REGISTER8)


/*
    ime_pm6_type     :    [0x0, 0x1],           bits : 0
    ime_pm6_color_y  :    [0x0, 0xff],          bits : 11_4
    ime_pm6_color_u  :    [0x0, 0xff],          bits : 19_12
    ime_pm6_color_v  :    [0x0, 0xff],          bits : 27_20
    ime_pm6_shape_sel:    [0x0, 0x1],           bits : 31
*/
#define IME_PRIVACY_MASK_SET6_REGISTER0_OFS 0x05ac
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER0)
REGDEF_BIT(ime_pm6_type,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm6_color_y,        8)
REGDEF_BIT(ime_pm6_color_u,        8)
REGDEF_BIT(ime_pm6_color_v,        8)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm6_shape_sel,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER0)


/*
    ime_pm6_line0_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm6_line0_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm6_line0_signa:    [0x0, 0x1],         bits : 17
    ime_pm6_line0_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm6_line0_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET6_REGISTER1_OFS 0x05b0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER1)
REGDEF_BIT(ime_pm6_line0_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm6_line0_coefa,        13)
REGDEF_BIT(ime_pm6_line0_signa,        1)
REGDEF_BIT(ime_pm6_line0_coefb,        13)
REGDEF_BIT(ime_pm6_line0_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER1)


/*
    ime_pm6_line0_coefc  :    [0x0, 0x3ffffff],         bits : 25_0
    ime_pm6_line0_signc  :    [0x0, 0x1],           bits : 26
    ime_pm6_comb_mode_sel:    [0x0, 0x3],           bits : 31_30
*/
#define IME_PRIVACY_MASK_SET6_REGISTER2_OFS 0x05b4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER2)
REGDEF_BIT(ime_pm6_line0_coefc,        26)
REGDEF_BIT(ime_pm6_line0_signc,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm6_comb_mode_sel,        2)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER2)


/*
    ime_pm6_line1_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm6_line1_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm6_line1_signa:    [0x0, 0x1],         bits : 17
    ime_pm6_line1_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm6_line1_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET6_REGISTER3_OFS 0x05b8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER3)
REGDEF_BIT(ime_pm6_line1_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm6_line1_coefa,        13)
REGDEF_BIT(ime_pm6_line1_signa,        1)
REGDEF_BIT(ime_pm6_line1_coefb,        13)
REGDEF_BIT(ime_pm6_line1_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER3)


/*
    ime_pm6_line1_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm6_line1_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET6_REGISTER4_OFS 0x05bc
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER4)
REGDEF_BIT(ime_pm6_line1_coefc,        26)
REGDEF_BIT(ime_pm6_line1_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER4)


/*
    ime_pm6_line2_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm6_line2_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm6_line2_signa:    [0x0, 0x1],         bits : 17
    ime_pm6_line2_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm6_line2_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET6_REGISTER5_OFS 0x05c0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER5)
REGDEF_BIT(ime_pm6_line2_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm6_line2_coefa,        13)
REGDEF_BIT(ime_pm6_line2_signa,        1)
REGDEF_BIT(ime_pm6_line2_coefb,        13)
REGDEF_BIT(ime_pm6_line2_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER5)


/*
    ime_pm6_line2_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm6_line2_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET6_REGISTER6_OFS 0x05c4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER6)
REGDEF_BIT(ime_pm6_line2_coefc,        26)
REGDEF_BIT(ime_pm6_line2_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER6)


/*
    ime_pm6_line3_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm6_line3_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm6_line3_signa:    [0x0, 0x1],         bits : 17
    ime_pm6_line3_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm6_line3_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET6_REGISTER7_OFS 0x05c8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER7)
REGDEF_BIT(ime_pm6_line3_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm6_line3_coefa,        13)
REGDEF_BIT(ime_pm6_line3_signa,        1)
REGDEF_BIT(ime_pm6_line3_coefb,        13)
REGDEF_BIT(ime_pm6_line3_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER7)


/*
    ime_pm6_line3_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm6_line3_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET6_REGISTER8_OFS 0x05cc
REGDEF_BEGIN(IME_PRIVACY_MASK_SET6_REGISTER8)
REGDEF_BIT(ime_pm6_line3_coefc,        26)
REGDEF_BIT(ime_pm6_line3_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET6_REGISTER8)


/*
    ime_pm7_type   :    [0x0, 0x1],         bits : 0
    ime_pm7_color_y:    [0x0, 0xff],            bits : 11_4
    ime_pm7_color_u:    [0x0, 0xff],            bits : 19_12
    ime_pm7_color_v:    [0x0, 0xff],            bits : 27_20
*/
#define IME_PRIVACY_MASK_SET7_REGISTER0_OFS 0x05d0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET7_REGISTER0)
REGDEF_BIT(ime_pm7_type,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm7_color_y,        8)
REGDEF_BIT(ime_pm7_color_u,        8)
REGDEF_BIT(ime_pm7_color_v,        8)
REGDEF_END(IME_PRIVACY_MASK_SET7_REGISTER0)


/*
    ime_pm7_line0_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm7_line0_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm7_line0_signa:    [0x0, 0x1],         bits : 17
    ime_pm7_line0_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm7_line0_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET7_REGISTER1_OFS 0x05d4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET7_REGISTER1)
REGDEF_BIT(ime_pm7_line0_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm7_line0_coefa,        13)
REGDEF_BIT(ime_pm7_line0_signa,        1)
REGDEF_BIT(ime_pm7_line0_coefb,        13)
REGDEF_BIT(ime_pm7_line0_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET7_REGISTER1)


/*
    ime_pm7_line0_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm7_line0_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET7_REGISTER2_OFS 0x05d8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET7_REGISTER2)
REGDEF_BIT(ime_pm7_line0_coefc,        26)
REGDEF_BIT(ime_pm7_line0_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET7_REGISTER2)


/*
    ime_pm7_line1_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm7_line1_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm7_line1_signa:    [0x0, 0x1],         bits : 17
    ime_pm7_line1_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm7_line1_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET7_REGISTER3_OFS 0x05dc
REGDEF_BEGIN(IME_PRIVACY_MASK_SET7_REGISTER3)
REGDEF_BIT(ime_pm7_line1_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm7_line1_coefa,        13)
REGDEF_BIT(ime_pm7_line1_signa,        1)
REGDEF_BIT(ime_pm7_line1_coefb,        13)
REGDEF_BIT(ime_pm7_line1_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET7_REGISTER3)


/*
    ime_pm7_line1_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm7_line1_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET7_REGISTER4_OFS 0x05e0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET7_REGISTER4)
REGDEF_BIT(ime_pm7_line1_coefc,        26)
REGDEF_BIT(ime_pm7_line1_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET7_REGISTER4)


/*
    ime_pm7_line2_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm7_line2_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm7_line2_signa:    [0x0, 0x1],         bits : 17
    ime_pm7_line2_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm7_line2_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET7_REGISTER5_OFS 0x05e4
REGDEF_BEGIN(IME_PRIVACY_MASK_SET7_REGISTER5)
REGDEF_BIT(ime_pm7_line2_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm7_line2_coefa,        13)
REGDEF_BIT(ime_pm7_line2_signa,        1)
REGDEF_BIT(ime_pm7_line2_coefb,        13)
REGDEF_BIT(ime_pm7_line2_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET7_REGISTER5)


/*
    ime_pm7_line2_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm7_line2_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET7_REGISTER6_OFS 0x05e8
REGDEF_BEGIN(IME_PRIVACY_MASK_SET7_REGISTER6)
REGDEF_BIT(ime_pm7_line2_coefc,        26)
REGDEF_BIT(ime_pm7_line2_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET7_REGISTER6)


/*
    ime_pm7_line3_comp :    [0x0, 0x3],         bits : 1_0
    ime_pm7_line3_coefa:    [0x0, 0x1fff],          bits : 16_4
    ime_pm7_line3_signa:    [0x0, 0x1],         bits : 17
    ime_pm7_line3_coefb:    [0x0, 0x1fff],          bits : 30_18
    ime_pm7_line3_signb:    [0x0, 0x1],         bits : 31
*/
#define IME_PRIVACY_MASK_SET7_REGISTER7_OFS 0x05ec
REGDEF_BEGIN(IME_PRIVACY_MASK_SET7_REGISTER7)
REGDEF_BIT(ime_pm7_line3_comp,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm7_line3_coefa,        13)
REGDEF_BIT(ime_pm7_line3_signa,        1)
REGDEF_BIT(ime_pm7_line3_coefb,        13)
REGDEF_BIT(ime_pm7_line3_signb,        1)
REGDEF_END(IME_PRIVACY_MASK_SET7_REGISTER7)


/*
    ime_pm7_line3_coefc:    [0x0, 0x3ffffff],           bits : 25_0
    ime_pm7_line3_signc:    [0x0, 0x1],         bits : 26
*/
#define IME_PRIVACY_MASK_SET7_REGISTER8_OFS 0x05f0
REGDEF_BEGIN(IME_PRIVACY_MASK_SET7_REGISTER8)
REGDEF_BIT(ime_pm7_line3_coefc,        26)
REGDEF_BIT(ime_pm7_line3_signc,        1)
REGDEF_END(IME_PRIVACY_MASK_SET7_REGISTER8)


/*
    ime_pm4_awet:    [0x0, 0xff],           bits : 7_0
    ime_pm5_awet:    [0x0, 0xff],           bits : 15_8
    ime_pm6_awet:    [0x0, 0xff],           bits : 23_16
    ime_pm7_awet:    [0x0, 0xff],           bits : 31_24
*/
#define IME_PRIVACY_MASK_ALPHA_REGISTER1_OFS 0x05f4
REGDEF_BEGIN(IME_PRIVACY_MASK_ALPHA_REGISTER1)
REGDEF_BIT(ime_pm4_awet,        8)
REGDEF_BIT(ime_pm5_awet,        8)
REGDEF_BIT(ime_pm6_awet,        8)
REGDEF_BIT(ime_pm7_awet,        8)
REGDEF_END(IME_PRIVACY_MASK_ALPHA_REGISTER1)


/*
    ime_pm_subin_h_size:    [0x0, 0x7ff],           bits : 10_0
    ime_pm_subin_v_size:    [0x0, 0x7ff],           bits : 26_16
    ime_pm_fmt         :    [0x0, 0x3],         bits : 31_30
*/
#define IME_PRIVACY_MASK_SUB_IMAGE_REGISTER_OFS 0x05f8
REGDEF_BEGIN(IME_PRIVACY_MASK_SUB_IMAGE_REGISTER)
REGDEF_BIT(ime_pm_subin_h_size,        11)
REGDEF_BIT(,        5)
REGDEF_BIT(ime_pm_subin_v_size,        11)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_pm_fmt,        2)
REGDEF_END(IME_PRIVACY_MASK_SUB_IMAGE_REGISTER)


/*
    ime_pm_y_ofsi:    [0x0, 0x3ffff],           bits : 19_2
*/
#define IME_PRIVACY_MASK_SUB_IMAGE_Y_CHANNEL_LINEOFFSET_REGISTER_OFS 0x05fc
REGDEF_BEGIN(IME_PRIVACY_MASK_SUB_IMAGE_Y_CHANNEL_LINEOFFSET_REGISTER)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm_y_ofsi,        18)
REGDEF_END(IME_PRIVACY_MASK_SUB_IMAGE_Y_CHANNEL_LINEOFFSET_REGISTER)


/*
    ime_pm_y_sai:    [0x0, 0x3fffffff],         bits : 31_2
*/
#define IME_PRIVACY_MASK_IMAGE_INPUT_Y_CHANNEL_DMA_ADDRESS_REGISTER_OFS 0x0600
REGDEF_BEGIN(IME_PRIVACY_MASK_IMAGE_INPUT_Y_CHANNEL_DMA_ADDRESS_REGISTER)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_pm_y_sai,        30)
REGDEF_END(IME_PRIVACY_MASK_IMAGE_INPUT_Y_CHANNEL_DMA_ADDRESS_REGISTER)


/*
    ime_in_pxl_bp0:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_INPUT_PIXEL_BREAK_POINT_REGISTER0_OFS 0x0604
REGDEF_BEGIN(IME_INPUT_PIXEL_BREAK_POINT_REGISTER0)
REGDEF_BIT(ime_in_pxl_bp0,        32)
REGDEF_END(IME_INPUT_PIXEL_BREAK_POINT_REGISTER0)


/*
    ime_in_pxl_bp1:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_INPUT_PIXEL_BREAK_POINT_REGISTER1_OFS 0x0608
REGDEF_BEGIN(IME_INPUT_PIXEL_BREAK_POINT_REGISTER1)
REGDEF_BIT(ime_in_pxl_bp1,        32)
REGDEF_END(IME_INPUT_PIXEL_BREAK_POINT_REGISTER1)


/*
    ime_in_pxl_bp1:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_INPUT_PIXEL_BREAK_POINT_REGISTER2_OFS 0x060c
REGDEF_BEGIN(IME_INPUT_PIXEL_BREAK_POINT_REGISTER2)
REGDEF_BIT(ime_in_pxl_bp1,        32)
REGDEF_END(IME_INPUT_PIXEL_BREAK_POINT_REGISTER2)


/*
    ime_in_line_bp0:    [0x0, 0xffff],          bits : 15_0
    ime_in_line_bp1:    [0x0, 0xffff],          bits : 31_16
*/
#define IME_INPUT_PATH_BREAK_POINT_REGISTER0_OFS 0x0610
REGDEF_BEGIN(IME_INPUT_PATH_BREAK_POINT_REGISTER0)
REGDEF_BIT(ime_in_line_bp0,        16)
REGDEF_BIT(ime_in_line_bp1,        16)
REGDEF_END(IME_INPUT_PATH_BREAK_POINT_REGISTER0)


/*
    ime_in_line_bp2:    [0x0, 0xffff],          bits : 15_0
    ime_bp_mode    :    [0x0, 0x1],         bits : 31
*/
#define IME_INPUT_PATH_BREAK_POINT_REGISTER1_OFS 0x0614
REGDEF_BEGIN(IME_INPUT_PATH_BREAK_POINT_REGISTER1)
REGDEF_BIT(ime_in_line_bp2,        16)
REGDEF_BIT(,        15)
REGDEF_BIT(ime_bp_mode,        1)
REGDEF_END(IME_INPUT_PATH_BREAK_POINT_REGISTER1)


/*
    ime_pm_y_msb_sai:    [0x0, 0xf],            bits : 3_0
*/
#define IME_PRIVACY_MASK_IMAGE_INPUT_Y_CHANNEL_DMA_ADDRESS_REGISTER2_OFS 0x0618
REGDEF_BEGIN(IME_PRIVACY_MASK_IMAGE_INPUT_Y_CHANNEL_DMA_ADDRESS_REGISTER2)
REGDEF_BIT(ime_pm_y_msb_sai,        4)
REGDEF_END(IME_PRIVACY_MASK_IMAGE_INPUT_Y_CHANNEL_DMA_ADDRESS_REGISTER2)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_190_OFS 0x061c
REGDEF_BEGIN(IME_RESERVED_REGISTER_190)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_190)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_191_OFS 0x0620
REGDEF_BEGIN(IME_RESERVED_REGISTER_191)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_191)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_192_OFS 0x0624
REGDEF_BEGIN(IME_RESERVED_REGISTER_192)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_192)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_193_OFS 0x0628
REGDEF_BEGIN(IME_RESERVED_REGISTER_193)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_193)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_194_OFS 0x062c
REGDEF_BEGIN(IME_RESERVED_REGISTER_194)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_194)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_195_OFS 0x0630
REGDEF_BEGIN(IME_RESERVED_REGISTER_195)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_195)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_196_OFS 0x0634
REGDEF_BEGIN(IME_RESERVED_REGISTER_196)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_196)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_197_OFS 0x0638
REGDEF_BEGIN(IME_RESERVED_REGISTER_197)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_197)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_198_OFS 0x063c
REGDEF_BEGIN(IME_RESERVED_REGISTER_198)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_198)


/*
    ime_in_bst_y           :    [0x0, 0x1],         bits : 0
    ime_in_bst_u           :    [0x0, 0x1],         bits : 1
    ime_in_bst_v           :    [0x0, 0x1],         bits : 2
    ime_out_p0_bst_y       :    [0x0, 0x1],         bits : 3
    ime_out_p0_bst_u       :    [0x0, 0x1],         bits : 4
    ime_out_p0_bst_v       :    [0x0, 0x1],         bits : 5
    ime_out_p1_bst_y       :    [0x0, 0x1],         bits : 6
    ime_out_p1_bst_uv      :    [0x0, 0x1],         bits : 7
    ime_out_p2_bst_y       :    [0x0, 0x1],         bits : 8
    ime_out_p2_bst_uv      :    [0x0, 0x1],         bits : 9
    ime_out_p3_bst_y       :    [0x0, 0x1],         bits : 10
    ime_out_p3_bst_uv      :    [0x0, 0x1],         bits : 11
    ime_in_pix_bst         :    [0x0, 0x1],         bits : 12
    ime_out_sub_bst        :    [0x0, 0x1],         bits : 13
    ime_in_3dnr_bst_fcp    :    [0x0, 0x1],         bits : 14
    ime_out_3dnr_bst_fcp   :    [0x0, 0x1],         bits : 15
    ime_in_3dnr_bst_y      :    [0x0, 0x1],         bits : 16
    ime_in_3dnr_bst_c      :    [0x0, 0x1],         bits : 17
    ime_out_3dnr_bst_y     :    [0x0, 0x1],         bits : 18
    ime_out_3dnr_bst_c     :    [0x0, 0x1],         bits : 19
    ime_in_3dnr_bst_mv     :    [0x0, 0x1],         bits : 20
    ime_out_3dnr_bst_mv    :    [0x0, 0x1],         bits : 21
    ime_out_3dnr_bst_mo_roi:    [0x0, 0x1],         bits : 24
    ime_out_3dnr_bst_sta   :    [0x0, 0x1],         bits : 25
*/
#define IME_BURST_LENGTH_REGISTER0_OFS 0x0640
REGDEF_BEGIN(IME_BURST_LENGTH_REGISTER0)
REGDEF_BIT(ime_in_bst_y,        1)
REGDEF_BIT(ime_in_bst_u,        1)
REGDEF_BIT(ime_in_bst_v,        1)
REGDEF_BIT(ime_out_p0_bst_y,        1)
REGDEF_BIT(ime_out_p0_bst_u,        1)
REGDEF_BIT(ime_out_p0_bst_v,        1)
REGDEF_BIT(ime_out_p1_bst_y,        1)
REGDEF_BIT(ime_out_p1_bst_uv,        1)
REGDEF_BIT(ime_out_p2_bst_y,        1)
REGDEF_BIT(ime_out_p2_bst_uv,        1)
REGDEF_BIT(ime_out_p3_bst_y,        1)
REGDEF_BIT(ime_out_p3_bst_uv,        1)
REGDEF_BIT(ime_in_pix_bst,        1)
REGDEF_BIT(ime_out_sub_bst,        1)
REGDEF_BIT(ime_in_3dnr_bst_fcp,        1)
REGDEF_BIT(ime_out_3dnr_bst_fcp,        1)
REGDEF_BIT(ime_in_3dnr_bst_y,        1)
REGDEF_BIT(ime_in_3dnr_bst_c,        1)
REGDEF_BIT(ime_out_3dnr_bst_y,        1)
REGDEF_BIT(ime_out_3dnr_bst_c,        1)
REGDEF_BIT(ime_in_3dnr_bst_mv,        1)
REGDEF_BIT(ime_out_3dnr_bst_mv,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_out_3dnr_bst_mo_roi,        1)
REGDEF_BIT(ime_out_3dnr_bst_sta,        1)
REGDEF_END(IME_BURST_LENGTH_REGISTER0)


/*
    ime_frm_start_rst:    [0x0, 0x1],           bits : 30
    ime_chksum_en    :    [0x0, 0x1],           bits : 31
*/
#define IME_FRAME_START_RESET_CONTROL_REGISTER_OFS 0x0644
REGDEF_BEGIN(IME_FRAME_START_RESET_CONTROL_REGISTER)
REGDEF_BIT(,        30)
REGDEF_BIT(ime_frm_start_rst,        1)
REGDEF_BIT(ime_chksum_en,        1)
REGDEF_END(IME_FRAME_START_RESET_CONTROL_REGISTER)


/*
    ime_ll_cmd_start_addr_info:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_LINKED_LIST_NEXT_JOB_START_ADDRESS_INFORMATION_REGISTER_OFS 0x0648
REGDEF_BEGIN(IME_LINKED_LIST_NEXT_JOB_START_ADDRESS_INFORMATION_REGISTER)
REGDEF_BIT(ime_ll_cmd_start_addr_info,        32)
REGDEF_END(IME_LINKED_LIST_NEXT_JOB_START_ADDRESS_INFORMATION_REGISTER)


/*
    ime_ll_cmdprs_cnt:    [0x0, 0xfffff],           bits : 19_0
*/
#define IME_LINKED_LIST_COMMAND_PARSING_COUNTER_REGISTER_OFS 0x064c
REGDEF_BEGIN(IME_LINKED_LIST_COMMAND_PARSING_COUNTER_REGISTER)
REGDEF_BIT(ime_ll_cmdprs_cnt,        20)
REGDEF_END(IME_LINKED_LIST_COMMAND_PARSING_COUNTER_REGISTER)


/*
    ime_ll_tab0:    [0x0, 0xff],            bits : 7_0
    ime_ll_tab1:    [0x0, 0xff],            bits : 15_8
    ime_ll_tab2:    [0x0, 0xff],            bits : 23_16
    ime_ll_tab3:    [0x0, 0xff],            bits : 31_24
*/
#define IME_LINKED_LIST_TABLE_INDEX_REGISTER0_OFS 0x0650
REGDEF_BEGIN(IME_LINKED_LIST_TABLE_INDEX_REGISTER0)
REGDEF_BIT(ime_ll_tab0,        8)
REGDEF_BIT(ime_ll_tab1,        8)
REGDEF_BIT(ime_ll_tab2,        8)
REGDEF_BIT(ime_ll_tab3,        8)
REGDEF_END(IME_LINKED_LIST_TABLE_INDEX_REGISTER0)


/*
    ime_ll_tab4:    [0x0, 0xff],            bits : 7_0
    ime_ll_tab5:    [0x0, 0xff],            bits : 15_8
    ime_ll_tab6:    [0x0, 0xff],            bits : 23_16
    ime_ll_tab7:    [0x0, 0xff],            bits : 31_24
*/
#define IME_LINKED_LIST_TABLE_INDEX_REGISTER1_OFS 0x0654
REGDEF_BEGIN(IME_LINKED_LIST_TABLE_INDEX_REGISTER1)
REGDEF_BIT(ime_ll_tab4,        8)
REGDEF_BIT(ime_ll_tab5,        8)
REGDEF_BIT(ime_ll_tab6,        8)
REGDEF_BIT(ime_ll_tab7,        8)
REGDEF_END(IME_LINKED_LIST_TABLE_INDEX_REGISTER1)


/*
    ime_ll_tab8 :    [0x0, 0xff],           bits : 7_0
    ime_ll_tab9 :    [0x0, 0xff],           bits : 15_8
    ime_ll_tab10:    [0x0, 0xff],           bits : 23_16
    ime_ll_tab11:    [0x0, 0xff],           bits : 31_24
*/
#define IME_LINKED_LIST_TABLE_INDEX_REGISTER2_OFS 0x0658
REGDEF_BEGIN(IME_LINKED_LIST_TABLE_INDEX_REGISTER2)
REGDEF_BIT(ime_ll_tab8,        8)
REGDEF_BIT(ime_ll_tab9,        8)
REGDEF_BIT(ime_ll_tab10,        8)
REGDEF_BIT(ime_ll_tab11,        8)
REGDEF_END(IME_LINKED_LIST_TABLE_INDEX_REGISTER2)


/*
    ime_ll_tab12:    [0x0, 0xff],           bits : 7_0
    ime_ll_tab13:    [0x0, 0xff],           bits : 15_8
    ime_ll_tab14:    [0x0, 0xff],           bits : 23_16
    ime_ll_tab15:    [0x0, 0xff],           bits : 31_24
*/
#define IME_LINKED_LIST_TABLE_INDEX_REGISTER3_OFS 0x065c
REGDEF_BEGIN(IME_LINKED_LIST_TABLE_INDEX_REGISTER3)
REGDEF_BIT(ime_ll_tab12,        8)
REGDEF_BIT(ime_ll_tab13,        8)
REGDEF_BIT(ime_ll_tab14,        8)
REGDEF_BIT(ime_ll_tab15,        8)
REGDEF_END(IME_LINKED_LIST_TABLE_INDEX_REGISTER3)


/*
    shp_edge_weight_src_sel:    [0x0, 0x1],         bits : 0
    shp_motion_bit_en      :    [0x0, 0x1],         bits : 1
    shp_show_info          :    [0x0, 0x3],         bits : 3_2
    shp_jnd_filter_size    :    [0x0, 0x3],         bits : 5_4
    shp_edge_weight_th     :    [0x0, 0xff],            bits : 15_8
    shp_edge_weight_gain   :    [0x0, 0xff],            bits : 23_16
    shp_noise_level        :    [0x0, 0xff],            bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER0_OFS 0x0660
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER0)
REGDEF_BIT(shp_edge_weight_src_sel,        1)
REGDEF_BIT(shp_motion_bit_en,        1)
REGDEF_BIT(shp_show_info,        2)
REGDEF_BIT(shp_jnd_filter_size,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(shp_edge_weight_th,        8)
REGDEF_BIT(shp_edge_weight_gain,        8)
REGDEF_BIT(shp_noise_level,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER0)


/*
    shp_blend_inv_gamma:    [0x0, 0xff],            bits : 7_0
    shp_edge_str       :    [0x0, 0xff],            bits : 15_8
    shp_coring_th      :    [0x0, 0xff],            bits : 23_16
    shp_w_con_eng      :    [0x0, 0xff],            bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER1_OFS 0x0664
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER1)
REGDEF_BIT(shp_blend_inv_gamma,        8)
REGDEF_BIT(shp_edge_str,        8)
REGDEF_BIT(shp_coring_th,        8)
REGDEF_BIT(shp_w_con_eng,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER1)


/*
    shp_bright_halo_clip:    [0x0, 0xff],           bits : 7_0
    shp_dark_halo_clip  :    [0x0, 0xff],           bits : 15_8
*/
#define IME_POST_SHARPEN_REGISTER2_OFS 0x0668
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER2)
REGDEF_BIT(shp_bright_halo_clip,        8)
REGDEF_BIT(shp_dark_halo_clip,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER2)


/*
    shp_flat_th:    [0x0, 0x7ff],           bits : 10_0
    shp_edge_th:    [0x0, 0x7ff],           bits : 26_16
*/
#define IME_POST_SHARPEN_REGISTER3_OFS 0x066c
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER3)
REGDEF_BIT(shp_flat_th,        11)
REGDEF_BIT(,        5)
REGDEF_BIT(shp_edge_th,        11)
REGDEF_END(IME_POST_SHARPEN_REGISTER3)


/*
    shp_slope_con_eng  :    [0x0, 0xfff],           bits : 11_0
    shp_flat_region_str:    [0x0, 0xff],            bits : 23_16
    shp_edge_region_str:    [0x0, 0xff],            bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER4_OFS 0x0670
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER4)
REGDEF_BIT(shp_slope_con_eng,        12)
REGDEF_BIT(,        4)
REGDEF_BIT(shp_flat_region_str,        8)
REGDEF_BIT(shp_edge_region_str,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER4)


/*
    shp_motion_edge_weight_str:    [0x0, 0xff],         bits : 7_0
    shp_static_edge_weight_str:    [0x0, 0xff],         bits : 15_8
    shp_trans_edge_weight_str :    [0x0, 0xff],         bits : 23_16
*/
#define IME_POST_SHARPEN_REGISTER5_OFS 0x0674
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER5)
REGDEF_BIT(shp_motion_edge_weight_str,        8)
REGDEF_BIT(shp_static_edge_weight_str,        8)
REGDEF_BIT(shp_trans_edge_weight_str,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER5)


/*
    shp_noise_curve0:    [0x0, 0xff],           bits : 7_0
    shp_noise_curve1:    [0x0, 0xff],           bits : 15_8
    shp_noise_curve2:    [0x0, 0xff],           bits : 23_16
    shp_noise_curve3:    [0x0, 0xff],           bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER6_OFS 0x0678
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER6)
REGDEF_BIT(shp_noise_curve0,        8)
REGDEF_BIT(shp_noise_curve1,        8)
REGDEF_BIT(shp_noise_curve2,        8)
REGDEF_BIT(shp_noise_curve3,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER6)


/*
    shp_noise_curve4:    [0x0, 0xff],           bits : 7_0
    shp_noise_curve5:    [0x0, 0xff],           bits : 15_8
    shp_noise_curve6:    [0x0, 0xff],           bits : 23_16
    shp_noise_curve7:    [0x0, 0xff],           bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER7_OFS 0x067c
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER7)
REGDEF_BIT(shp_noise_curve4,        8)
REGDEF_BIT(shp_noise_curve5,        8)
REGDEF_BIT(shp_noise_curve6,        8)
REGDEF_BIT(shp_noise_curve7,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER7)


/*
    shp_noise_curve8 :    [0x0, 0xff],          bits : 7_0
    shp_noise_curve9 :    [0x0, 0xff],          bits : 15_8
    shp_noise_curve10:    [0x0, 0xff],          bits : 23_16
    shp_noise_curve11:    [0x0, 0xff],          bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER8_OFS 0x0680
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER8)
REGDEF_BIT(shp_noise_curve8,        8)
REGDEF_BIT(shp_noise_curve9,        8)
REGDEF_BIT(shp_noise_curve10,        8)
REGDEF_BIT(shp_noise_curve11,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER8)


/*
    shp_noise_curve12:    [0x0, 0xff],          bits : 7_0
    shp_noise_curve13:    [0x0, 0xff],          bits : 15_8
    shp_noise_curve14:    [0x0, 0xff],          bits : 23_16
    shp_noise_curve15:    [0x0, 0xff],          bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER9_OFS 0x0684
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER9)
REGDEF_BIT(shp_noise_curve12,        8)
REGDEF_BIT(shp_noise_curve13,        8)
REGDEF_BIT(shp_noise_curve14,        8)
REGDEF_BIT(shp_noise_curve15,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER9)


/*
    shp_noise_curve16:    [0x0, 0xff],          bits : 7_0
*/
#define IME_POST_SHARPEN_REGISTER10_OFS 0x0688
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER10)
REGDEF_BIT(shp_noise_curve16,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER10)


/*
    shp_ewg_curve0:    [0x0, 0xff],         bits : 7_0
    shp_ewg_curve1:    [0x0, 0xff],         bits : 15_8
    shp_ewg_curve2:    [0x0, 0xff],         bits : 23_16
    shp_ewg_curve3:    [0x0, 0xff],         bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER11_OFS 0x068c
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER11)
REGDEF_BIT(shp_ewg_curve0,        8)
REGDEF_BIT(shp_ewg_curve1,        8)
REGDEF_BIT(shp_ewg_curve2,        8)
REGDEF_BIT(shp_ewg_curve3,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER11)


/*
    shp_ewg_curve4:    [0x0, 0xff],         bits : 7_0
    shp_ewg_curve5:    [0x0, 0xff],         bits : 15_8
    shp_ewg_curve6:    [0x0, 0xff],         bits : 23_16
    shp_ewg_curve7:    [0x0, 0xff],         bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER12_OFS 0x0690
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER12)
REGDEF_BIT(shp_ewg_curve4,        8)
REGDEF_BIT(shp_ewg_curve5,        8)
REGDEF_BIT(shp_ewg_curve6,        8)
REGDEF_BIT(shp_ewg_curve7,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER12)


/*
    shp_ewg_curve8:    [0x0, 0xff],         bits : 7_0
*/
#define IME_POST_SHARPEN_REGISTER13_OFS 0x0694
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER13)
REGDEF_BIT(shp_ewg_curve8,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER13)


/*
    shp_ewg_curve0_msb:    [0x0, 0x7],          bits : 2_0
    shp_ewg_curve1_msb:    [0x0, 0x7],          bits : 5_3
    shp_ewg_curve2_msb:    [0x0, 0x7],          bits : 8_6
    shp_ewg_curve3_msb:    [0x0, 0x7],          bits : 11_9
    shp_ewg_curve4_msb:    [0x0, 0x7],          bits : 14_12
    shp_ewg_curve5_msb:    [0x0, 0x7],          bits : 17_15
    shp_ewg_curve6_msb:    [0x0, 0x7],          bits : 20_18
    shp_ewg_curve7_msb:    [0x0, 0x7],          bits : 23_21
    shp_ewg_curve8_msb:    [0x0, 0x7],          bits : 26_24
*/
#define IME_POST_SHARPEN_REGISTER14_OFS 0x0698
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER14)
REGDEF_BIT(shp_ewg_curve0_msb,        3)
REGDEF_BIT(shp_ewg_curve1_msb,        3)
REGDEF_BIT(shp_ewg_curve2_msb,        3)
REGDEF_BIT(shp_ewg_curve3_msb,        3)
REGDEF_BIT(shp_ewg_curve4_msb,        3)
REGDEF_BIT(shp_ewg_curve5_msb,        3)
REGDEF_BIT(shp_ewg_curve6_msb,        3)
REGDEF_BIT(shp_ewg_curve7_msb,        3)
REGDEF_BIT(shp_ewg_curve8_msb,        3)
REGDEF_END(IME_POST_SHARPEN_REGISTER14)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_214_OFS 0x069c
REGDEF_BEGIN(IME_RESERVED_REGISTER_214)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_214)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_215_OFS 0x06a0
REGDEF_BEGIN(IME_RESERVED_REGISTER_215)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_215)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_216_OFS 0x06a4
REGDEF_BEGIN(IME_RESERVED_REGISTER_216)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_216)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_217_OFS 0x06a8
REGDEF_BEGIN(IME_RESERVED_REGISTER_217)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_217)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_218_OFS 0x06ac
REGDEF_BEGIN(IME_RESERVED_REGISTER_218)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_218)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_219_OFS 0x06b0
REGDEF_BEGIN(IME_RESERVED_REGISTER_219)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_219)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_220_OFS 0x06b4
REGDEF_BEGIN(IME_RESERVED_REGISTER_220)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_220)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_221_OFS 0x06b8
REGDEF_BEGIN(IME_RESERVED_REGISTER_221)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_221)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_222_OFS 0x06bc
REGDEF_BEGIN(IME_RESERVED_REGISTER_222)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_222)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_223_OFS 0x06c0
REGDEF_BEGIN(IME_RESERVED_REGISTER_223)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_223)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_224_OFS 0x06c4
REGDEF_BEGIN(IME_RESERVED_REGISTER_224)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_224)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_225_OFS 0x06c8
REGDEF_BEGIN(IME_RESERVED_REGISTER_225)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_225)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_226_OFS 0x06cc
REGDEF_BEGIN(IME_RESERVED_REGISTER_226)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_226)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_227_OFS 0x06d0
REGDEF_BEGIN(IME_RESERVED_REGISTER_227)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_227)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_228_OFS 0x06d4
REGDEF_BEGIN(IME_RESERVED_REGISTER_228)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_228)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_229_OFS 0x06d8
REGDEF_BEGIN(IME_RESERVED_REGISTER_229)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_229)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_230_OFS 0x06dc
REGDEF_BEGIN(IME_RESERVED_REGISTER_230)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_230)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_231_OFS 0x06e0
REGDEF_BEGIN(IME_RESERVED_REGISTER_231)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_231)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_232_OFS 0x06e4
REGDEF_BEGIN(IME_RESERVED_REGISTER_232)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_232)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_233_OFS 0x06e8
REGDEF_BEGIN(IME_RESERVED_REGISTER_233)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_233)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_234_OFS 0x06ec
REGDEF_BEGIN(IME_RESERVED_REGISTER_234)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_234)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_235_OFS 0x06f0
REGDEF_BEGIN(IME_RESERVED_REGISTER_235)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_235)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_236_OFS 0x06f4
REGDEF_BEGIN(IME_RESERVED_REGISTER_236)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_236)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_237_OFS 0x06f8
REGDEF_BEGIN(IME_RESERVED_REGISTER_237)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_237)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_238_OFS 0x06fc
REGDEF_BEGIN(IME_RESERVED_REGISTER_238)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_238)


/*
    ime_3dnr_pre_y_blur_str      :    [0x0, 0x3],           bits : 1_0
    ime_3dnr_pf_type             :    [0x0, 0x3],           bits : 3_2
    ime_3dnr_me_update_mode      :    [0x0, 0x1],           bits : 4
    ime_3dnr_me_boundary_set     :    [0x0, 0x1],           bits : 5
    ime_3dnr_ps_smart_roi_ctrl   :    [0x0, 0x1],           bits : 8
    ime_3dnr_nr_center_wzero_y_3d:    [0x0, 0x1],           bits : 9
    ime_3dnr_ps_mv_check_en      :    [0x0, 0x1],           bits : 10
    ime_3dnr_ps_mv_check_roi_en  :    [0x0, 0x1],           bits : 11
    ime_3dnr_ps_mv_info_mode     :    [0x0, 0x3],           bits : 13_12
    ime_3dnr_me_sad_shift        :    [0x0, 0xf],           bits : 19_16
    ime_3dnr_nr_y_ch_en          :    [0x0, 0x1],           bits : 20
    ime_3dnr_nr_c_ch_en          :    [0x0, 0x1],           bits : 21
    ime_3dnr_seed_reset_en       :    [0x0, 0x1],           bits : 23
*/
#define IME_TMNR_CONTROL_REGISTER0_OFS 0x0700
REGDEF_BEGIN(IME_TMNR_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_pre_y_blur_str,        2)
REGDEF_BIT(ime_3dnr_pf_type,        2)
REGDEF_BIT(ime_3dnr_me_update_mode,        1)
REGDEF_BIT(ime_3dnr_me_boundary_set,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ps_smart_roi_ctrl,        1)
REGDEF_BIT(ime_3dnr_nr_center_wzero_y_3d,        1)
REGDEF_BIT(ime_3dnr_ps_mv_check_en,        1)
REGDEF_BIT(ime_3dnr_ps_mv_check_roi_en,        1)
REGDEF_BIT(ime_3dnr_ps_mv_info_mode,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_me_sad_shift,        4)
REGDEF_BIT(ime_3dnr_nr_y_ch_en,        1)
REGDEF_BIT(ime_3dnr_nr_c_ch_en,        1)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_seed_reset_en,        1)
REGDEF_END(IME_TMNR_CONTROL_REGISTER0)


/*
    ime_3dnr_ne_sample_step_x   :    [0x0, 0xff],           bits : 7_0
    ime_3dnr_ne_sample_step_y   :    [0x0, 0xff],           bits : 15_8
    ime_3dnr_statistic_output_en:    [0x0, 0x1],            bits : 16
    ime_3dnr_ps_fastc_en        :    [0x0, 0x1],            bits : 17
    ime_3dnr_dbg_mv0            :    [0x0, 0x1],            bits : 18
    ime_3dnr_dbg_mode           :    [0x0, 0xf],            bits : 31_28
*/
#define IME_TMNR_CONTROL_REGISTER1_OFS 0x0704
REGDEF_BEGIN(IME_TMNR_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_ne_sample_step_x,        8)
REGDEF_BIT(ime_3dnr_ne_sample_step_y,        8)
REGDEF_BIT(ime_3dnr_statistic_output_en,        1)
REGDEF_BIT(ime_3dnr_ps_fastc_en,        1)
REGDEF_BIT(ime_3dnr_dbg_mv0,        1)
REGDEF_BIT(,        9)
REGDEF_BIT(ime_3dnr_dbg_mode,        4)
REGDEF_END(IME_TMNR_CONTROL_REGISTER1)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_239_OFS 0x0708
REGDEF_BEGIN(IME_RESERVED_REGISTER_239)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_239)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_240_OFS 0x070c
REGDEF_BEGIN(IME_RESERVED_REGISTER_240)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_240)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_241_OFS 0x0710
REGDEF_BEGIN(IME_RESERVED_REGISTER_241)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_241)


/*
    ime_3dnr_ro_motion_sum:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_3DNR_READ_ONLY_REGISTER0_OFS 0x0714
REGDEF_BEGIN(IME_3DNR_READ_ONLY_REGISTER0)
REGDEF_BIT(ime_3dnr_ro_motion_sum,        32)
REGDEF_END(IME_3DNR_READ_ONLY_REGISTER0)


/*
    ime_3dnr_ro_edge_sum:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_3DNR_READ_ONLY_REGISTER1_OFS 0x0718
REGDEF_BEGIN(IME_3DNR_READ_ONLY_REGISTER1)
REGDEF_BIT(ime_3dnr_ro_edge_sum,        32)
REGDEF_END(IME_3DNR_READ_ONLY_REGISTER1)


/*
    ime_3dnr_ro_sad_sum:    [0x0, 0xffffffff],          bits : 31_0
*/
#define IME_3DNR_READ_ONLY_REGISTER2_OFS 0x071c
REGDEF_BEGIN(IME_3DNR_READ_ONLY_REGISTER2)
REGDEF_BIT(ime_3dnr_ro_sad_sum,        32)
REGDEF_END(IME_3DNR_READ_ONLY_REGISTER2)


/*
    ime_3dnr_ro_mv_sum:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_3DNR_READ_ONLY_REGISTER3_OFS 0x0720
REGDEF_BEGIN(IME_3DNR_READ_ONLY_REGISTER3)
REGDEF_BIT(ime_3dnr_ro_mv_sum,        32)
REGDEF_END(IME_3DNR_READ_ONLY_REGISTER3)


/*
    ime_3dnr_ro_sample_cnt:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_3DNR_READ_ONLY_REGISTER4_OFS 0x0724
REGDEF_BEGIN(IME_3DNR_READ_ONLY_REGISTER4)
REGDEF_BIT(ime_3dnr_ro_sample_cnt,        32)
REGDEF_END(IME_3DNR_READ_ONLY_REGISTER4)


/*
    ime_3dnr_me_sad_penalty_0:    [0x0, 0x3ff],         bits : 9_0
    ime_3dnr_me_sad_penalty_1:    [0x0, 0x3ff],         bits : 19_10
    ime_3dnr_me_sad_penalty_2:    [0x0, 0x3ff],         bits : 29_20
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER0_OFS 0x0728
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_me_sad_penalty_0,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_1,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_2,        10)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER0)


/*
    ime_3dnr_me_sad_penalty_3:    [0x0, 0x3ff],         bits : 9_0
    ime_3dnr_me_sad_penalty_4:    [0x0, 0x3ff],         bits : 19_10
    ime_3dnr_me_sad_penalty_5:    [0x0, 0x3ff],         bits : 29_20
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER1_OFS 0x072c
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_me_sad_penalty_3,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_4,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_5,        10)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER1)


/*
    ime_3dnr_me_sad_penalty_6:    [0x0, 0x3ff],         bits : 9_0
    ime_3dnr_me_sad_penalty_7:    [0x0, 0x3ff],         bits : 19_10
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER2_OFS 0x0730
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER2)
REGDEF_BIT(ime_3dnr_me_sad_penalty_6,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_7,        10)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER2)


/*
    ime_3dnr_me_switch_th0:    [0x0, 0xff],         bits : 7_0
    ime_3dnr_me_switch_th1:    [0x0, 0xff],         bits : 15_8
    ime_3dnr_me_switch_th2:    [0x0, 0xff],         bits : 23_16
    ime_3dnr_me_switch_th3:    [0x0, 0xff],         bits : 31_24
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER3_OFS 0x0734
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER3)
REGDEF_BIT(ime_3dnr_me_switch_th0,        8)
REGDEF_BIT(ime_3dnr_me_switch_th1,        8)
REGDEF_BIT(ime_3dnr_me_switch_th2,        8)
REGDEF_BIT(ime_3dnr_me_switch_th3,        8)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER3)


/*
    ime_3dnr_me_switch_th4:    [0x0, 0xff],         bits : 7_0
    ime_3dnr_me_switch_th5:    [0x0, 0xff],         bits : 15_8
    ime_3dnr_me_switch_th6:    [0x0, 0xff],         bits : 23_16
    ime_3dnr_me_switch_th7:    [0x0, 0xff],         bits : 31_24
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER4_OFS 0x0738
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER4)
REGDEF_BIT(ime_3dnr_me_switch_th4,        8)
REGDEF_BIT(ime_3dnr_me_switch_th5,        8)
REGDEF_BIT(ime_3dnr_me_switch_th6,        8)
REGDEF_BIT(ime_3dnr_me_switch_th7,        8)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER4)


/*
    ime_3dnr_me_switch_ratio:    [0x0, 0xff],           bits : 7_0
    ime_3dnr_me_cost_blend  :    [0x0, 0xf],            bits : 11_8
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER5_OFS 0x073c
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER5)
REGDEF_BIT(ime_3dnr_me_switch_ratio,        8)
REGDEF_BIT(ime_3dnr_me_cost_blend,        4)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER5)


/*
    ime_3dnr_me_detail_penalty0:    [0x0, 0xf],         bits : 3_0
    ime_3dnr_me_detail_penalty1:    [0x0, 0xf],         bits : 7_4
    ime_3dnr_me_detail_penalty2:    [0x0, 0xf],         bits : 11_8
    ime_3dnr_me_detail_penalty3:    [0x0, 0xf],         bits : 15_12
    ime_3dnr_me_detail_penalty4:    [0x0, 0xf],         bits : 19_16
    ime_3dnr_me_detail_penalty5:    [0x0, 0xf],         bits : 23_20
    ime_3dnr_me_detail_penalty6:    [0x0, 0xf],         bits : 27_24
    ime_3dnr_me_detail_penalty7:    [0x0, 0xf],         bits : 31_28
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER6_OFS 0x0740
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER6)
REGDEF_BIT(ime_3dnr_me_detail_penalty0,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty1,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty2,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty3,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty4,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty5,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty6,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty7,        4)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER6)


/*
    ime_3dnr_me_probability0:    [0x0, 0x1],            bits : 0
    ime_3dnr_me_probability1:    [0x0, 0x1],            bits : 1
    ime_3dnr_me_probability2:    [0x0, 0x1],            bits : 2
    ime_3dnr_me_probability3:    [0x0, 0x1],            bits : 3
    ime_3dnr_me_probability4:    [0x0, 0x1],            bits : 4
    ime_3dnr_me_probability5:    [0x0, 0x1],            bits : 5
    ime_3dnr_me_probability6:    [0x0, 0x1],            bits : 6
    ime_3dnr_me_probability7:    [0x0, 0x1],            bits : 7
    ime_3dnr_me_rand_bit_x  :    [0x0, 0x7],            bits : 10_8
    ime_3dnr_me_rand_bit_y  :    [0x0, 0x7],            bits : 14_12
    ime_3dnr_me_min_detail  :    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER7_OFS 0x0744
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER7)
REGDEF_BIT(ime_3dnr_me_probability0,        1)
REGDEF_BIT(ime_3dnr_me_probability1,        1)
REGDEF_BIT(ime_3dnr_me_probability2,        1)
REGDEF_BIT(ime_3dnr_me_probability3,        1)
REGDEF_BIT(ime_3dnr_me_probability4,        1)
REGDEF_BIT(ime_3dnr_me_probability5,        1)
REGDEF_BIT(ime_3dnr_me_probability6,        1)
REGDEF_BIT(ime_3dnr_me_probability7,        1)
REGDEF_BIT(ime_3dnr_me_rand_bit_x,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_me_rand_bit_y,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_me_min_detail,        14)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER7)


/*
    ime_3dnr_pf_str:    [0x0, 0xff],            bits : 7_0
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER8_OFS 0x0748
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER8)
REGDEF_BIT(ime_3dnr_pf_str,        8)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER8)


/*
    ime_3dnr_ne_sample_num_x:    [0x0, 0xfff],          bits : 11_0
    ime_3dnr_ne_sample_num_y:    [0x0, 0xfff],          bits : 27_16
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER9_OFS 0x074C
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER9)
REGDEF_BIT(ime_3dnr_ne_sample_num_x,        12)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_3dnr_ne_sample_num_y,        12)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER9)


/*
    ime_3dnr_ne_sample_start_x:    [0x0, 0xfff],            bits : 11_0
    ime_3dnr_ne_sample_start_y:    [0x0, 0xfff],            bits : 27_16
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER10_OFS 0x0750
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER10)
REGDEF_BIT(ime_3dnr_ne_sample_start_x,        12)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_3dnr_ne_sample_start_y,        12)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER10)


/*
    ime_3dnr_fast_converge_sp  :    [0x0, 0xf],         bits : 3_0
    ime_3dnr_fast_converge_step:    [0x0, 0xf],         bits : 7_4
*/
#define IME_3DNR_FAST_CONVERGE_CONTROL_REGISTER0_OFS 0x0754
REGDEF_BEGIN(IME_3DNR_FAST_CONVERGE_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_fast_converge_sp,        4)
REGDEF_BIT(ime_3dnr_fast_converge_step,        4)
REGDEF_END(IME_3DNR_FAST_CONVERGE_CONTROL_REGISTER0)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_242_OFS 0x0758
REGDEF_BEGIN(IME_RESERVED_REGISTER_242)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_242)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_243_OFS 0x075c
REGDEF_BEGIN(IME_RESERVED_REGISTER_243)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_243)


/*
    ime_3dnr_md_sad_coef_a0:    [0x0, 0x3f],            bits : 5_0
    ime_3dnr_md_sad_coef_a1:    [0x0, 0x3f],            bits : 13_8
    ime_3dnr_md_sad_coef_a2:    [0x0, 0x3f],            bits : 21_16
    ime_3dnr_md_sad_coef_a3:    [0x0, 0x3f],            bits : 29_24
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER0_OFS 0x0760
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_md_sad_coef_a0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_a1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_a2,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_a3,        6)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER0)


/*
    ime_3dnr_md_sad_coef_a4:    [0x0, 0x3f],            bits : 5_0
    ime_3dnr_md_sad_coef_a5:    [0x0, 0x3f],            bits : 13_8
    ime_3dnr_md_sad_coef_a6:    [0x0, 0x3f],            bits : 21_16
    ime_3dnr_md_sad_coef_a7:    [0x0, 0x3f],            bits : 29_24
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER1_OFS 0x0764
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_md_sad_coef_a4,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_a5,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_a6,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_a7,        6)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER1)


/*
    ime_3dnr_md_sad_coef_b0:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_md_sad_coef_b1:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER2_OFS 0x0768
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER2)
REGDEF_BIT(ime_3dnr_md_sad_coef_b0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_b1,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER2)


/*
    ime_3dnr_md_sad_coef_b2:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_md_sad_coef_b3:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER3_OFS 0x076c
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER3)
REGDEF_BIT(ime_3dnr_md_sad_coef_b2,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_b3,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER3)


/*
    ime_3dnr_md_sad_coef_b4:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_md_sad_coef_b5:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER4_OFS 0x0770
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER4)
REGDEF_BIT(ime_3dnr_md_sad_coef_b4,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_b5,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER4)


/*
    ime_3dnr_md_sad_coef_b6:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_md_sad_coef_b7:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER5_OFS 0x0774
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER5)
REGDEF_BIT(ime_3dnr_md_sad_coef_b6,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_b7,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER5)


/*
    ime_3dnr_md_sad_std0:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_md_sad_std1:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER6_OFS 0x0778
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER6)
REGDEF_BIT(ime_3dnr_md_sad_std0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_std1,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER6)


/*
    ime_3dnr_md_sad_std2:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_md_sad_std3:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER7_OFS 0x077c
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER7)
REGDEF_BIT(ime_3dnr_md_sad_std2,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_std3,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER7)


/*
    ime_3dnr_md_sad_std4:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_md_sad_std5:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER8_OFS 0x0780
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER8)
REGDEF_BIT(ime_3dnr_md_sad_std4,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_std5,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER8)


/*
    ime_3dnr_md_sad_std6:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_md_sad_std7:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER9_OFS 0x0784
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER9)
REGDEF_BIT(ime_3dnr_md_sad_std6,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_std7,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER9)


/*
    ime_3dnr_md_k1:    [0x0, 0x3f],         bits : 5_0
    ime_3dnr_md_k2:    [0x0, 0x3f],         bits : 13_8
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER10_OFS 0x0788
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER10)
REGDEF_BIT(ime_3dnr_md_k1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_k2,        6)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER10)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_244_OFS 0x078c
REGDEF_BEGIN(IME_RESERVED_REGISTER_244)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_244)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_245_OFS 0x0790
REGDEF_BEGIN(IME_RESERVED_REGISTER_245)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_245)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_246_OFS 0x0794
REGDEF_BEGIN(IME_RESERVED_REGISTER_246)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_246)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_247_OFS 0x0798
REGDEF_BEGIN(IME_RESERVED_REGISTER_247)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_247)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_248_OFS 0x079c
REGDEF_BEGIN(IME_RESERVED_REGISTER_248)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_248)


/*
    ime_3dnr_mc_sad_base0:    [0x0, 0x3fff],            bits : 13_0
    ime_3dnr_mc_sad_base1:    [0x0, 0x3fff],            bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER0_OFS 0x07a0
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_mc_sad_base0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_base1,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER0)


/*
    ime_3dnr_mc_sad_base2:    [0x0, 0x3fff],            bits : 13_0
    ime_3dnr_mc_sad_base3:    [0x0, 0x3fff],            bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER1_OFS 0x07a4
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_mc_sad_base2,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_base3,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER1)


/*
    ime_3dnr_mc_sad_base4:    [0x0, 0x3fff],            bits : 13_0
    ime_3dnr_mc_sad_base5:    [0x0, 0x3fff],            bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER2_OFS 0x07a8
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER2)
REGDEF_BIT(ime_3dnr_mc_sad_base4,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_base5,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER2)


/*
    ime_3dnr_mc_sad_base6:    [0x0, 0x3fff],            bits : 13_0
    ime_3dnr_mc_sad_base7:    [0x0, 0x3fff],            bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER3_OFS 0x07ac
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER3)
REGDEF_BIT(ime_3dnr_mc_sad_base6,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_base7,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER3)


/*
    ime_3dnr_mc_sad_coef_a0:    [0x0, 0x3f],            bits : 5_0
    ime_3dnr_mc_sad_coef_a1:    [0x0, 0x3f],            bits : 13_8
    ime_3dnr_mc_sad_coef_a2:    [0x0, 0x3f],            bits : 21_16
    ime_3dnr_mc_sad_coef_a3:    [0x0, 0x3f],            bits : 29_24
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER4_OFS 0x07b0
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER4)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a2,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a3,        6)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER4)


/*
    ime_3dnr_mc_sad_coef_a4:    [0x0, 0x3f],            bits : 5_0
    ime_3dnr_mc_sad_coef_a5:    [0x0, 0x3f],            bits : 13_8
    ime_3dnr_mc_sad_coef_a6:    [0x0, 0x3f],            bits : 21_16
    ime_3dnr_mc_sad_coef_a7:    [0x0, 0x3f],            bits : 29_24
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER5_OFS 0x07b4
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER5)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a4,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a5,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a6,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a7,        6)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER5)


/*
    ime_3dnr_mc_sad_coef_b0:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_mc_sad_coef_b1:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER6_OFS 0x07b8
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER6)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b1,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER6)


/*
    ime_3dnr_mc_sad_coef_b2:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_mc_sad_coef_b3:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER7_OFS 0x07bc
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER7)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b2,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b3,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER7)


/*
    ime_3dnr_mc_sad_coef_b4:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_mc_sad_coef_b5:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER8_OFS 0x07c0
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER8)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b4,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b5,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER8)


/*
    ime_3dnr_mc_sad_coef_b6:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_mc_sad_coef_b7:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER9_OFS 0x07c4
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER9)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b6,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b7,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER9)


/*
    ime_3dnr_mc_sad_std0:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_mc_sad_std1:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER10_OFS 0x07c8
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER10)
REGDEF_BIT(ime_3dnr_mc_sad_std0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_std1,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER10)


/*
    ime_3dnr_mc_sad_std2:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_mc_sad_std3:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER11_OFS 0x07cc
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER11)
REGDEF_BIT(ime_3dnr_mc_sad_std2,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_std3,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER11)


/*
    ime_3dnr_mc_sad_std4:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_mc_sad_std5:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER12_OFS 0x07d0
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER12)
REGDEF_BIT(ime_3dnr_mc_sad_std4,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_std5,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER12)


/*
    ime_3dnr_mc_sad_std6:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_mc_sad_std7:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER13_OFS 0x07d4
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER13)
REGDEF_BIT(ime_3dnr_mc_sad_std6,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_std7,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER13)


/*
    ime_3dnr_mc_k1:    [0x0, 0x3f],         bits : 5_0
    ime_3dnr_mc_k2:    [0x0, 0x3f],         bits : 13_8
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER14_OFS 0x07d8
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER14)
REGDEF_BIT(ime_3dnr_mc_k1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_k2,        6)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER14)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_249_OFS 0x07dc
REGDEF_BEGIN(IME_RESERVED_REGISTER_249)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_249)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_250_OFS 0x07e0
REGDEF_BEGIN(IME_RESERVED_REGISTER_250)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_250)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_251_OFS 0x07e4
REGDEF_BEGIN(IME_RESERVED_REGISTER_251)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_251)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_252_OFS 0x07e8
REGDEF_BEGIN(IME_RESERVED_REGISTER_252)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_252)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_253_OFS 0x07ec
REGDEF_BEGIN(IME_RESERVED_REGISTER_253)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_253)


/*
    ime_3dnr_roi_md_k1:    [0x0, 0x3f],         bits : 5_0
    ime_3dnr_roi_md_k2:    [0x0, 0x3f],         bits : 13_8
*/
#define IME_3DNR_ROI_MOTION_DETECTION_CONTROL_REGISTER0_OFS 0x07f0
REGDEF_BEGIN(IME_3DNR_ROI_MOTION_DETECTION_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_roi_md_k1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_roi_md_k2,        6)
REGDEF_END(IME_3DNR_ROI_MOTION_DETECTION_CONTROL_REGISTER0)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_254_OFS 0x07f4
REGDEF_BEGIN(IME_RESERVED_REGISTER_254)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_254)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_255_OFS 0x07f8
REGDEF_BEGIN(IME_RESERVED_REGISTER_255)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_255)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_256_OFS 0x07fc
REGDEF_BEGIN(IME_RESERVED_REGISTER_256)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_256)


/*
    ime_3dnr_roi_mc_k1  :    [0x0, 0x3f],           bits : 5_0
    ime_3dnr_roi_mc_k2  :    [0x0, 0x3f],           bits : 13_8
    ime_3dnr_ps_blur_eth:    [0x0, 0xffff],         bits : 31_16
*/
#define IME_3DNR_ROI_MOTION_COMPENSATION_CONTROL_REGISTER0_OFS 0x0800
REGDEF_BEGIN(IME_3DNR_ROI_MOTION_COMPENSATION_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_roi_mc_k1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_roi_mc_k2,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ps_blur_eth,        16)
REGDEF_END(IME_3DNR_ROI_MOTION_COMPENSATION_CONTROL_REGISTER0)


/*
    ime_3dnr_nr_y_tf0_blur_str0:    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_y_tf0_blur_str1:    [0x0, 0xff],            bits : 15_8
    ime_3dnr_nr_y_tf0_blur_str2:    [0x0, 0xff],            bits : 23_16
    ime_3dnr_nr_y_tf0_str0     :    [0x0, 0xff],            bits : 31_24
*/
#define IME_3DNR_TF0_REGISTER0_OFS 0x0804
REGDEF_BEGIN(IME_3DNR_TF0_REGISTER0)
REGDEF_BIT(ime_3dnr_nr_y_tf0_blur_str0,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_blur_str1,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_blur_str2,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_str0,        8)
REGDEF_END(IME_3DNR_TF0_REGISTER0)


/*
    ime_3dnr_nr_y_tf0_str1:    [0x0, 0xff],         bits : 7_0
    ime_3dnr_nr_y_tf0_str2:    [0x0, 0xff],         bits : 15_8
    ime_3dnr_nr_c_tf0_str0:    [0x0, 0xff],         bits : 23_16
    ime_3dnr_nr_c_tf0_str1:    [0x0, 0xff],         bits : 31_24
*/
#define IME_3DNR_TF0_REGISTER1_OFS 0x0808
REGDEF_BEGIN(IME_3DNR_TF0_REGISTER1)
REGDEF_BIT(ime_3dnr_nr_y_tf0_str1,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_str2,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_str0,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_str1,        8)
REGDEF_END(IME_3DNR_TF0_REGISTER1)


/*
    ime_3dnr_nr_c_tf0_str2     :    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_y_tf0_blur_estr:    [0x0, 0xff],            bits : 15_8
*/
#define IME_3DNR_TF0_REGISTER2_OFS 0x080c
REGDEF_BEGIN(IME_3DNR_TF0_REGISTER2)
REGDEF_BIT(ime_3dnr_nr_c_tf0_str2,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_blur_estr,        8)
REGDEF_END(IME_3DNR_TF0_REGISTER2)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_257_OFS 0x0810
REGDEF_BEGIN(IME_RESERVED_REGISTER_257)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_257)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_258_OFS 0x0814
REGDEF_BEGIN(IME_RESERVED_REGISTER_258)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_258)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_259_OFS 0x0818
REGDEF_BEGIN(IME_RESERVED_REGISTER_259)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_259)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_260_OFS 0x081c
REGDEF_BEGIN(IME_RESERVED_REGISTER_260)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_260)


/*
    ime_3dnr_ps_mv_th    :    [0x0, 0x3f],          bits : 5_0
    ime_3dnr_ps_roi_mv_th:    [0x0, 0x3f],          bits : 13_8
    ime_3dnr_ps_fs_th    :    [0x0, 0x3fff],            bits : 29_16
*/
#define IME_3DNR_PS_CONTROL_REGISTER0_OFS 0x0820
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_ps_mv_th,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ps_roi_mv_th,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ps_fs_th,        14)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER0)


/*
    ime_3dnr_ps_mix_ratio0:    [0x0, 0xff],         bits : 7_0
    ime_3dnr_ps_mix_ratio1:    [0x0, 0xff],         bits : 15_8
*/
#define IME_3DNR_PS_CONTROL_REGISTER1_OFS 0x0824
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_ps_mix_ratio0,        8)
REGDEF_BIT(ime_3dnr_ps_mix_ratio1,        8)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER1)


/*
    ime_3dnr_nr_u_tf0_md_th  :    [0x0, 0xff],          bits : 7_0
    ime_3dnr_nr_v_tf0_md_th  :    [0x0, 0xff],          bits : 15_8
    ime_3dnr_nr_c_tf0_ratio_0:    [0x0, 0xff],          bits : 23_16
    ime_3dnr_nr_c_tf0_ratio_1:    [0x0, 0xff],          bits : 31_24
*/
#define IME_3DNR_PS_CONTROL_REGISTER2_OFS 0x0828
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER2)
REGDEF_BIT(ime_3dnr_nr_u_tf0_md_th,        8)
REGDEF_BIT(ime_3dnr_nr_v_tf0_md_th,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_ratio_0,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_ratio_1,        8)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER2)


/*
    ime_3dnr_nr_motion_sat_ratio :    [0x0, 0xff],          bits : 7_0
    ime_3dnr_nr_c_tf0_residual_th:    [0x0, 0xf],           bits : 11_8
*/
#define IME_3DNR_PS_CONTROL_REGISTER3_OFS 0x082c
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER3)
REGDEF_BIT(ime_3dnr_nr_motion_sat_ratio,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_residual_th,        4)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER3)


/*
    ime_3dnr_nr_cshk_th0:    [0x0, 0x3ff],          bits : 9_0
    ime_3dnr_nr_cshk_th1:    [0x0, 0x3ff],          bits : 25_16
*/
#define IME_3DNR_COLOR_DITHERING_REGISTER0_OFS 0x0830
REGDEF_BEGIN(IME_3DNR_COLOR_DITHERING_REGISTER0)
REGDEF_BIT(ime_3dnr_nr_cshk_th0,        10)
REGDEF_BIT(,        6)
REGDEF_BIT(ime_3dnr_nr_cshk_th1,        10)
REGDEF_END(IME_3DNR_COLOR_DITHERING_REGISTER0)


/*
    ime_3dnr_nr_cshk_th2:    [0x0, 0x3ff],          bits : 9_0
    ime_3dnr_nr_cshk_th3:    [0x0, 0x3ff],          bits : 25_16
*/
#define IME_3DNR_COLOR_DITHERING_REGISTER1_OFS 0x0834
REGDEF_BEGIN(IME_3DNR_COLOR_DITHERING_REGISTER1)
REGDEF_BIT(ime_3dnr_nr_cshk_th2,        10)
REGDEF_BIT(,        6)
REGDEF_BIT(ime_3dnr_nr_cshk_th3,        10)
REGDEF_END(IME_3DNR_COLOR_DITHERING_REGISTER1)


/*
    ime_3dnr_nr_cshk_th4:    [0x0, 0x3ff],          bits : 9_0
    ime_3dnr_nr_cshk_th5:    [0x0, 0x3ff],          bits : 25_16
*/
#define IME_3DNR_COLOR_DITHERING_REGISTER2_OFS 0x0838
REGDEF_BEGIN(IME_3DNR_COLOR_DITHERING_REGISTER2)
REGDEF_BIT(ime_3dnr_nr_cshk_th4,        10)
REGDEF_BIT(,        6)
REGDEF_BIT(ime_3dnr_nr_cshk_th5,        10)
REGDEF_END(IME_3DNR_COLOR_DITHERING_REGISTER2)


/*
    ime_3dnr_nr_cshk_th6:    [0x0, 0x3ff],          bits : 9_0
    ime_3dnr_nr_cshk_th7:    [0x0, 0x3ff],          bits : 25_16
*/
#define IME_3DNR_COLOR_DITHERING_REGISTER3_OFS 0x083c
REGDEF_BEGIN(IME_3DNR_COLOR_DITHERING_REGISTER3)
REGDEF_BIT(ime_3dnr_nr_cshk_th6,        10)
REGDEF_BIT(,        6)
REGDEF_BIT(ime_3dnr_nr_cshk_th7,        10)
REGDEF_END(IME_3DNR_COLOR_DITHERING_REGISTER3)


/*
    ime_3dnr_nr_cshk_val0:    [0x0, 0x7],           bits : 2_0
    ime_3dnr_nr_cshk_val1:    [0x0, 0x7],           bits : 6_4
    ime_3dnr_nr_cshk_val2:    [0x0, 0x7],           bits : 10_8
    ime_3dnr_nr_cshk_val3:    [0x0, 0x7],           bits : 14_12
    ime_3dnr_nr_cshk_val4:    [0x0, 0x7],           bits : 18_16
    ime_3dnr_nr_cshk_val5:    [0x0, 0x7],           bits : 22_20
    ime_3dnr_nr_cshk_val6:    [0x0, 0x7],           bits : 26_24
    ime_3dnr_nr_cshk_val7:    [0x0, 0x7],           bits : 30_28
*/
#define IME_3DNR_COLOR_DITHERING_REGISTER4_OFS 0x0840
REGDEF_BEGIN(IME_3DNR_COLOR_DITHERING_REGISTER4)
REGDEF_BIT(ime_3dnr_nr_cshk_val0,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val1,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val2,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val3,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val4,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val5,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val6,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val7,        3)
REGDEF_END(IME_3DNR_COLOR_DITHERING_REGISTER4)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_261_OFS 0x0844
REGDEF_BEGIN(IME_RESERVED_REGISTER_261)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_261)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_262_OFS 0x0848
REGDEF_BEGIN(IME_RESERVED_REGISTER_262)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_262)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_263_OFS 0x084c
REGDEF_BEGIN(IME_RESERVED_REGISTER_263)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_263)


/*
    ime_3dnr_ps_ds_th    :    [0x0, 0x1f],          bits : 4_0
    ime_3dnr_ps_ds_th_roi:    [0x0, 0x1f],          bits : 12_8
*/
#define IME_3DNR_PS_CONTROL_REGISTER4_OFS 0x0850
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER4)
REGDEF_BIT(ime_3dnr_ps_ds_th,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_3dnr_ps_ds_th_roi,        5)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER4)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_3DNR_PS_CONTROL_REGISTER5_OFS 0x0854
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER5)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER5)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_3DNR_PS_CONTROL_REGISTER6_OFS 0x0858
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER6)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER6)


/*
    ime_3dnr_nr_residue_th_y0:    [0x0, 0xf],           bits : 3_0
    ime_3dnr_nr_residue_th_y1:    [0x0, 0xf],           bits : 11_8
    ime_3dnr_nr_residue_th_y2:    [0x0, 0xf],           bits : 19_16
    ime_3dnr_nr_residue_th_c :    [0x0, 0xf],           bits : 27_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER0_OFS 0x085c
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_nr_residue_th_y0,        4)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_3dnr_nr_residue_th_y1,        4)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_3dnr_nr_residue_th_y2,        4)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_3dnr_nr_residue_th_c,        4)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER0)


/*
    ime_3dnr_nr_freq_w0:    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_freq_w1:    [0x0, 0xff],            bits : 15_8
    ime_3dnr_nr_freq_w2:    [0x0, 0xff],            bits : 23_16
    ime_3dnr_nr_freq_w3:    [0x0, 0xff],            bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER1_OFS 0x0860
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_nr_freq_w0,        8)
REGDEF_BIT(ime_3dnr_nr_freq_w1,        8)
REGDEF_BIT(ime_3dnr_nr_freq_w2,        8)
REGDEF_BIT(ime_3dnr_nr_freq_w3,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER1)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_264_OFS 0x0864
REGDEF_BEGIN(IME_RESERVED_REGISTER_264)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_264)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_265_OFS 0x0868
REGDEF_BEGIN(IME_RESERVED_REGISTER_265)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_265)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_266_OFS 0x086c
REGDEF_BEGIN(IME_RESERVED_REGISTER_266)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_266)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_267_OFS 0x0870
REGDEF_BEGIN(IME_RESERVED_REGISTER_267)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_267)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_268_OFS 0x0874
REGDEF_BEGIN(IME_RESERVED_REGISTER_268)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_268)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_269_OFS 0x0878
REGDEF_BEGIN(IME_RESERVED_REGISTER_269)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_269)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_270_OFS 0x087c
REGDEF_BEGIN(IME_RESERVED_REGISTER_270)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_270)


/*
    ime_3dnr_nr_luma_w0:    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_luma_w1:    [0x0, 0xff],            bits : 15_8
    ime_3dnr_nr_luma_w2:    [0x0, 0xff],            bits : 23_16
    ime_3dnr_nr_luma_w3:    [0x0, 0xff],            bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER3_OFS 0x0880
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER3)
REGDEF_BIT(ime_3dnr_nr_luma_w0,        8)
REGDEF_BIT(ime_3dnr_nr_luma_w1,        8)
REGDEF_BIT(ime_3dnr_nr_luma_w2,        8)
REGDEF_BIT(ime_3dnr_nr_luma_w3,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER3)


/*
    ime_3dnr_nr_luma_w4:    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_luma_w5:    [0x0, 0xff],            bits : 15_8
    ime_3dnr_nr_luma_w6:    [0x0, 0xff],            bits : 23_16
    ime_3dnr_nr_luma_w7:    [0x0, 0xff],            bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER4_OFS 0x0884
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER4)
REGDEF_BIT(ime_3dnr_nr_luma_w4,        8)
REGDEF_BIT(ime_3dnr_nr_luma_w5,        8)
REGDEF_BIT(ime_3dnr_nr_luma_w6,        8)
REGDEF_BIT(ime_3dnr_nr_luma_w7,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER4)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_271_OFS 0x0888
REGDEF_BEGIN(IME_RESERVED_REGISTER_271)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_271)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_272_OFS 0x088c
REGDEF_BEGIN(IME_RESERVED_REGISTER_272)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_272)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_273_OFS 0x0890
REGDEF_BEGIN(IME_RESERVED_REGISTER_273)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_273)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_274_OFS 0x0894
REGDEF_BEGIN(IME_RESERVED_REGISTER_274)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_274)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_275_OFS 0x0898
REGDEF_BEGIN(IME_RESERVED_REGISTER_275)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_275)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_276_OFS 0x089c
REGDEF_BEGIN(IME_RESERVED_REGISTER_276)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_276)


/*
    ime_3dnr_nr_pre_filtering_str0:    [0x0, 0xff],         bits : 7_0
    ime_3dnr_nr_pre_filtering_str1:    [0x0, 0xff],         bits : 15_8
    ime_3dnr_nr_pre_filtering_str2:    [0x0, 0xff],         bits : 23_16
    ime_3dnr_nr_pre_filtering_str3:    [0x0, 0xff],         bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER6_OFS 0x08a0
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER6)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_str0,        8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_str1,        8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_str2,        8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_str3,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER6)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_277_OFS 0x08a4
REGDEF_BEGIN(IME_RESERVED_REGISTER_277)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_277)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_278_OFS 0x08a8
REGDEF_BEGIN(IME_RESERVED_REGISTER_278)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_278)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_279_OFS 0x08ac
REGDEF_BEGIN(IME_RESERVED_REGISTER_279)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_279)


/*
    ime_3dnr_nr_pre_filtering_ratio0:    [0x0, 0xff],           bits : 7_0
    ime_3dnr_nr_pre_filtering_ratio1:    [0x0, 0xff],           bits : 15_8
    ime_3dnr_nr_snr_str0            :    [0x0, 0xff],           bits : 23_16
    ime_3dnr_nr_snr_str1            :    [0x0, 0xff],           bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER8_OFS 0x08b0
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_ratio0,        8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_ratio1,        8)
REGDEF_BIT(ime_3dnr_nr_snr_str0,        8)
REGDEF_BIT(ime_3dnr_nr_snr_str1,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER8)


/*
    ime_3dnr_nr_snr_str2:    [0x0, 0xff],           bits : 7_0
    ime_3dnr_nr_tnr_str0:    [0x0, 0xff],           bits : 15_8
    ime_3dnr_nr_tnr_str1:    [0x0, 0xff],           bits : 23_16
    ime_3dnr_nr_tnr_str2:    [0x0, 0xff],           bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER9_OFS 0x08b4
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER9)
REGDEF_BIT(ime_3dnr_nr_snr_str2,        8)
REGDEF_BIT(ime_3dnr_nr_tnr_str0,        8)
REGDEF_BIT(ime_3dnr_nr_tnr_str1,        8)
REGDEF_BIT(ime_3dnr_nr_tnr_str2,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER9)


/*
    ime_3dnr_nr_base_th_snr:    [0x0, 0xffff],          bits : 15_0
    ime_3dnr_nr_base_th_tnr:    [0x0, 0xffff],          bits : 31_16
*/
#define IME_3DNR_NR_CONTROL_REGISTER10_OFS 0x08b8
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER10)
REGDEF_BIT(ime_3dnr_nr_base_th_snr,        16)
REGDEF_BIT(ime_3dnr_nr_base_th_tnr,        16)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER10)


/*
    ime_3dnr_nr_y_3d_ratio0  :    [0x0, 0xff],          bits : 7_0
    ime_3dnr_nr_y_3d_ratio1  :    [0x0, 0xff],          bits : 15_8
    ime_3dnr_nr_luma_comp_str:    [0x0, 0xff],          bits : 23_16
*/
#define IME_3DNR_NR_CONTROL_REGISTER11_OFS 0x08bc
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER11)
REGDEF_BIT(ime_3dnr_nr_y_3d_ratio0,        8)
REGDEF_BIT(ime_3dnr_nr_y_3d_ratio1,        8)
REGDEF_BIT(ime_3dnr_nr_luma_comp_str,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER11)


/*
    ime_3dnr_nr_y_3d_lut0:    [0x0, 0x7f],          bits : 6_0
    ime_3dnr_nr_y_3d_lut1:    [0x0, 0x7f],          bits : 14_8
    ime_3dnr_nr_y_3d_lut2:    [0x0, 0x7f],          bits : 22_16
    ime_3dnr_nr_y_3d_lut3:    [0x0, 0x7f],          bits : 30_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER12_OFS 0x08c0
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER12)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut0,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut1,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut2,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut3,        7)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER12)


/*
    ime_3dnr_nr_y_3d_lut4:    [0x0, 0x7f],          bits : 6_0
    ime_3dnr_nr_y_3d_lut5:    [0x0, 0x7f],          bits : 14_8
    ime_3dnr_nr_y_3d_lut6:    [0x0, 0x7f],          bits : 22_16
    ime_3dnr_nr_y_3d_lut7:    [0x0, 0x7f],          bits : 30_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER13_OFS 0x08c4
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER13)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut4,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut5,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut6,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut7,        7)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER13)


/*
    ime_3dnr_nr_c_3d_lut0:    [0x0, 0x7f],          bits : 6_0
    ime_3dnr_nr_c_3d_lut1:    [0x0, 0x7f],          bits : 14_8
    ime_3dnr_nr_c_3d_lut2:    [0x0, 0x7f],          bits : 22_16
    ime_3dnr_nr_c_3d_lut3:    [0x0, 0x7f],          bits : 30_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER14_OFS 0x08c8
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER14)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut0,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut1,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut2,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut3,        7)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER14)


/*
    ime_3dnr_nr_c_3d_lut4:    [0x0, 0x7f],          bits : 6_0
    ime_3dnr_nr_c_3d_lut5:    [0x0, 0x7f],          bits : 14_8
    ime_3dnr_nr_c_3d_lut6:    [0x0, 0x7f],          bits : 22_16
    ime_3dnr_nr_c_3d_lut7:    [0x0, 0x7f],          bits : 30_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER15_OFS 0x08cc
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER15)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut4,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut5,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut6,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut7,        7)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER15)


/*
    ime_3dnr_nr_c_3d_ratio0:    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_c_3d_ratio1:    [0x0, 0xff],            bits : 15_8
*/
#define IME_3DNR_NR_CONTROL_REGISTER16_OFS 0x08d0
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER16)
REGDEF_BIT(ime_3dnr_nr_c_3d_ratio0,        8)
REGDEF_BIT(ime_3dnr_nr_c_3d_ratio1,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER16)


/*
    ime_3dnr_ref_in_st_hinit_y_ofs:    [0x0, 0xfffff],            bits : 19_0
*/
#define IME_3DNR_REFERENCE_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08D4
REGDEF_BEGIN(IME_3DNR_REFERENCE_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_ref_in_st_hinit_y_ofs,        20)
REGDEF_END(IME_3DNR_REFERENCE_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_ref_out_st_hinit_y_ofs:    [0x0, 0xfffff],           bits : 19_0
*/
#define IME_3DNR_REFERENCE_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08D8
REGDEF_BEGIN(IME_3DNR_REFERENCE_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_ref_out_st_hinit_y_ofs,        20)
REGDEF_END(IME_3DNR_REFERENCE_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_mot_in_st_hinit_ofs:    [0x0, 0xfffff],            bits : 19_0
*/
#define IME_3DNR_MOTION_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08DC
REGDEF_BEGIN(IME_3DNR_MOTION_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_mot_in_st_hinit_ofs,        20)
REGDEF_END(IME_3DNR_MOTION_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_mot_out_st_hinit_ofs:    [0x0, 0xfffff],           bits : 19_0
*/
#define IME_3DNR_MOTION_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08E0
REGDEF_BEGIN(IME_3DNR_MOTION_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_mot_out_st_hinit_ofs,        20)
REGDEF_END(IME_3DNR_MOTION_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_mot_roi_out_st_hinit_ofs:    [0x0, 0xfffff],           bits : 19_0
*/
#define IME_3DNR_MOTION_STATUS_ROI_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08E4
REGDEF_BEGIN(IME_3DNR_MOTION_STATUS_ROI_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_mot_roi_out_st_hinit_ofs,        20)
REGDEF_END(IME_3DNR_MOTION_STATUS_ROI_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_mv_in_st_hinit_ofs:    [0x0, 0xfffff],         bits : 19_0
*/
#define IME_3DNR_MOTION_VECTOR_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08E8
REGDEF_BEGIN(IME_3DNR_MOTION_VECTOR_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_mv_in_st_hinit_ofs,        20)
REGDEF_END(IME_3DNR_MOTION_VECTOR_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_mv_out_st_hinit_ofs:    [0x0, 0xfffff],            bits : 19_0
*/
#define IME_3DNR_MOTION_VECTOR_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08EC
REGDEF_BEGIN(IME_3DNR_MOTION_VECTOR_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_mv_out_st_hinit_ofs,        20)
REGDEF_END(IME_3DNR_MOTION_VECTOR_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_statistic_out_st_hinit_ofs:    [0x0, 0xfffff],         bits : 19_0
*/
#define IME_3DNR_STATISTIC_DATA_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08F0
REGDEF_BEGIN(IME_3DNR_STATISTIC_DATA_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_statistic_out_st_hinit_ofs,        20)
REGDEF_END(IME_3DNR_STATISTIC_DATA_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_fast_converge_in_st_hinit_ofs:    [0x0, 0xfffff],          bits : 19_0
*/
#define IME_3DNR_FAST_CONVERGE_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08F4
REGDEF_BEGIN(IME_3DNR_FAST_CONVERGE_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_fast_converge_in_st_hinit_ofs,        20)
REGDEF_END(IME_3DNR_FAST_CONVERGE_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_fast_converge_out_st_hinit_ofs:    [0x0, 0xfffff],         bits : 19_0
*/
#define IME_3DNR_FAST_CONVERGE_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER_OFS 0x08F8
REGDEF_BEGIN(IME_3DNR_FAST_CONVERGE_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)
REGDEF_BIT(ime_3dnr_fast_converge_out_st_hinit_ofs,        20)
REGDEF_END(IME_3DNR_FAST_CONVERGE_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER)


/*
    ime_3dnr_ref_in_fmt:    [0x0, 0x7],         bits : 2_0
*/
#define IME_3DNR_REFERENCE_FRAME_DATA_FORMAT_REGISTER_OFS 0x08fc
REGDEF_BEGIN(IME_3DNR_REFERENCE_FRAME_DATA_FORMAT_REGISTER)
REGDEF_BIT(ime_3dnr_ref_in_fmt,        3)
REGDEF_END(IME_3DNR_REFERENCE_FRAME_DATA_FORMAT_REGISTER)


/*
    ime_3dnr_ref_dram_y_ofsi:    [0x0, 0x3ffff],            bits : 19_2
*/
#define IME_3DNR_INPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER0_OFS 0x0900
REGDEF_BEGIN(IME_3DNR_INPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ref_dram_y_ofsi,        18)
REGDEF_END(IME_3DNR_INPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER0)


/*
    ime_3dnr_ref_dram_uv_ofsi:    [0x0, 0x3ffff],           bits : 19_2
*/
#define IME_3DNR_INPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER1_OFS 0x0904
REGDEF_BEGIN(IME_3DNR_INPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ref_dram_uv_ofsi,        18)
REGDEF_END(IME_3DNR_INPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER1)


/*
    ime_3dnr_ref_dram_y_sai:    [0x0, 0x3fffffff],          bits : 31_2
*/
#define IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER0_OFS 0x0908
REGDEF_BEGIN(IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ref_dram_y_sai,        30)
REGDEF_END(IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER0)


/*
    ime_3dnr_ref_dram_uv_sai:    [0x0, 0x3fffffff],         bits : 31_2
*/
#define IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER1_OFS 0x090c
REGDEF_BEGIN(IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ref_dram_uv_sai,        30)
REGDEF_END(IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER1)


/*
    ime_3dnr_ref_dram_y_ofso:    [0x0, 0x3ffff],            bits : 19_2
*/
#define IME_3DNR_OUTPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER0_OFS 0x0910
REGDEF_BEGIN(IME_3DNR_OUTPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ref_dram_y_ofso,        18)
REGDEF_END(IME_3DNR_OUTPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER0)


/*
    ime_3dnr_ref_dram_uv_ofso:    [0x0, 0x3ffff],           bits : 19_2
*/
#define IME_3DNR_OUTPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER1_OFS 0x0914
REGDEF_BEGIN(IME_3DNR_OUTPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ref_dram_uv_ofso,        18)
REGDEF_END(IME_3DNR_OUTPUT_REFERENCE_IMAGE_LINE_OFFSET_REGISTER1)


/*
    ime_3dnr_ref_dram_y_sao:    [0x0, 0x3fffffff],          bits : 31_2
*/
#define IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER0_OFS 0x0918
REGDEF_BEGIN(IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ref_dram_y_sao,        30)
REGDEF_END(IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER0)


/*
    ime_3dnr_ref_dram_uv_sao:    [0x0, 0x3fffffff],         bits : 31_2
*/
#define IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER1_OFS 0x091C
REGDEF_BEGIN(IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ref_dram_uv_sao,        30)
REGDEF_END(IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER1)


/*
    ime_3dnr_mot_dram_ofs:    [0x0, 0x3ffff],           bits : 19_2
*/
#define IME_3DNR_MOTION_STATUS_INPUT_DMA_LINEOFFSET_REGISTER0_OFS 0x0920
REGDEF_BEGIN(IME_3DNR_MOTION_STATUS_INPUT_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mot_dram_ofs,        18)
REGDEF_END(IME_3DNR_MOTION_STATUS_INPUT_DMA_LINEOFFSET_REGISTER0)


/*
    ime_3dnr_mot_dram_sai:    [0x0, 0x3fffffff],            bits : 31_2
*/
#define IME_3DNR_MOTION_STATUS_INPUT_DMA_STARTING_ADDRESS_REGISTER0_OFS 0x0924
REGDEF_BEGIN(IME_3DNR_MOTION_STATUS_INPUT_DMA_STARTING_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mot_dram_sai,        30)
REGDEF_END(IME_3DNR_MOTION_STATUS_INPUT_DMA_STARTING_ADDRESS_REGISTER0)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_290_OFS 0x0928
REGDEF_BEGIN(IME_RESERVED_REGISTER_290)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_290)


/*
    ime_3dnr_mot_dram_sao:    [0x0, 0x3fffffff],            bits : 31_2
*/
#define IME_3DNR_MOTION_STATUS_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0_OFS 0x092c
REGDEF_BEGIN(IME_3DNR_MOTION_STATUS_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mot_dram_sao,        30)
REGDEF_END(IME_3DNR_MOTION_STATUS_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0)


/*
    ime_3dnr_mot_roi_dram_ofso:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_3DNR_MOTION_STATUS_ROI_OUTPUT_DMA_LINEOFFSET_REGISTER0_OFS 0x0930
REGDEF_BEGIN(IME_3DNR_MOTION_STATUS_ROI_OUTPUT_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mot_roi_dram_ofso,        18)
REGDEF_END(IME_3DNR_MOTION_STATUS_ROI_OUTPUT_DMA_LINEOFFSET_REGISTER0)


/*
    ime_3dnr_mot_roi_dram_sao:    [0x0, 0x3fffffff],            bits : 31_2
*/
#define IME_3DNR_MOTION_STATUS_ROI_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0_OFS 0x0934
REGDEF_BEGIN(IME_3DNR_MOTION_STATUS_ROI_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mot_roi_dram_sao,        30)
REGDEF_END(IME_3DNR_MOTION_STATUS_ROI_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0)


/*
    ime_3dnr_mv_dram_ofs:    [0x0, 0x3ffff],            bits : 19_2
*/
#define IME_3DNR_MOTION_VECTOR_INPUT_DMA_LINEOFFSET_REGISTER0_OFS 0x0938
REGDEF_BEGIN(IME_3DNR_MOTION_VECTOR_INPUT_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mv_dram_ofs,        18)
REGDEF_END(IME_3DNR_MOTION_VECTOR_INPUT_DMA_LINEOFFSET_REGISTER0)


/*
    ime_3dnr_mv_dram_sai:    [0x0, 0x3fffffff],         bits : 31_2
*/
#define IME_3DNR_MOTION_VECTOR_INPUT_DMA_STARTING_ADDRESS_REGISTER0_OFS 0x093c
REGDEF_BEGIN(IME_3DNR_MOTION_VECTOR_INPUT_DMA_STARTING_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mv_dram_sai,        30)
REGDEF_END(IME_3DNR_MOTION_VECTOR_INPUT_DMA_STARTING_ADDRESS_REGISTER0)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_291_OFS 0x0940
REGDEF_BEGIN(IME_RESERVED_REGISTER_291)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_291)


/*
    ime_3dnr_mv_dram_sao:    [0x0, 0x3fffffff],         bits : 31_2
*/
#define IME_3DNR_MOTION_VECTOR_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0_OFS 0x0944
REGDEF_BEGIN(IME_3DNR_MOTION_VECTOR_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mv_dram_sao,        30)
REGDEF_END(IME_3DNR_MOTION_VECTOR_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0)


/*
    ime_3dnr_statistic_dram_ofso:    [0x0, 0x3ffff],            bits : 19_2
*/
#define IME_3DNR_STATISTIC_DATA_OUTPUT_DMA_LINEOFFSET_REGISTER0_OFS 0x0948
REGDEF_BEGIN(IME_3DNR_STATISTIC_DATA_OUTPUT_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_statistic_dram_ofso,        18)
REGDEF_END(IME_3DNR_STATISTIC_DATA_OUTPUT_DMA_LINEOFFSET_REGISTER0)


/*
    ime_3dnr_statistic_dram_sao:    [0x0, 0x3fffffff],          bits : 31_2
*/
#define IME_3DNR_STATISTIC_DATA_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0_OFS 0x094c
REGDEF_BEGIN(IME_3DNR_STATISTIC_DATA_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_statistic_dram_sao,        30)
REGDEF_END(IME_3DNR_STATISTIC_DATA_OUTPUT_DMA_STARTING_ADDRESS_REGISTER0)


/*
    ime_3dnr_fast_converge_dram_sai:    [0x0, 0x3fffffff],          bits : 31_2
*/
#define IME_3DNR_FAST_CONVERGE_INPUT_DMA_ADDRESS_REGISTER0_OFS 0x0950
REGDEF_BEGIN(IME_3DNR_FAST_CONVERGE_INPUT_DMA_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_fast_converge_dram_sai,        30)
REGDEF_END(IME_3DNR_FAST_CONVERGE_INPUT_DMA_ADDRESS_REGISTER0)


/*
    ime_3dnr_fast_converge_dram_ofs:    [0x0, 0x3ffff],         bits : 19_2
*/
#define IME_3DNR_FAST_CONVERGE_DMA_LINEOFFSET_REGISTER0_OFS 0x0954
REGDEF_BEGIN(IME_3DNR_FAST_CONVERGE_DMA_LINEOFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_fast_converge_dram_ofs,        18)
REGDEF_END(IME_3DNR_FAST_CONVERGE_DMA_LINEOFFSET_REGISTER0)


/*
    ime_3dnr_fast_converge_dram_sao:    [0x0, 0x3fffffff],          bits : 31_2
*/
#define IME_3DNR_FAST_CONVERGE_OUTPUT_DMA_ADDRESS_REGISTER0_OFS 0x0958
REGDEF_BEGIN(IME_3DNR_FAST_CONVERGE_OUTPUT_DMA_ADDRESS_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_fast_converge_dram_sao,        30)
REGDEF_END(IME_3DNR_FAST_CONVERGE_OUTPUT_DMA_ADDRESS_REGISTER0)


/*
    ime_3dnr_ref_dram_y_msb_sai:    [0x0, 0xf],         bits : 3_0
*/
#define IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER2_OFS 0x095c
REGDEF_BEGIN(IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER2)
REGDEF_BIT(ime_3dnr_ref_dram_y_msb_sai,        4)
REGDEF_END(IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER2)


/*
    ime_3dnr_ref_dram_uv_msb_sai:    [0x0, 0xf],            bits : 3_0
*/
#define IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER3_OFS 0x0960
REGDEF_BEGIN(IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER3)
REGDEF_BIT(ime_3dnr_ref_dram_uv_msb_sai,        4)
REGDEF_END(IME_3DNR_INPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER3)


/*
    ime_3dnr_ref_dram_y_msb_sao:    [0x0, 0xf],         bits : 3_0
*/
#define IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER2_OFS 0x0964
REGDEF_BEGIN(IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER2)
REGDEF_BIT(ime_3dnr_ref_dram_y_msb_sao,        4)
REGDEF_END(IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER2)


/*
    ime_3dnr_ref_dram_uv_msb_sao:    [0x0, 0xf],            bits : 3_0
*/
#define IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER3_OFS 0x0968
REGDEF_BEGIN(IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER3)
REGDEF_BIT(ime_3dnr_ref_dram_uv_msb_sao,        4)
REGDEF_END(IME_3DNR_OUTPUT_REFERENCE_IMAGE_DMA_STARTING_ADDRESS_REGISTER3)


/*
    ime_3dnr_mot_dram_msb_sai:    [0x0, 0xf],           bits : 3_0
*/
#define IME_3DNR_MOTION_STATUS_INPUT_DMA_STARTING_ADDRESS_REGISTER1_OFS 0x096c
REGDEF_BEGIN(IME_3DNR_MOTION_STATUS_INPUT_DMA_STARTING_ADDRESS_REGISTER1)
REGDEF_BIT(ime_3dnr_mot_dram_msb_sai,        4)
REGDEF_END(IME_3DNR_MOTION_STATUS_INPUT_DMA_STARTING_ADDRESS_REGISTER1)


/*
    ime_3dnr_mot_dram_msb_sao:    [0x0, 0xf],           bits : 3_0
*/
#define IME_3DNR_MOTION_STATUS_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1_OFS 0x0970
REGDEF_BEGIN(IME_3DNR_MOTION_STATUS_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1)
REGDEF_BIT(ime_3dnr_mot_dram_msb_sao,        4)
REGDEF_END(IME_3DNR_MOTION_STATUS_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1)


/*
    ime_3dnr_mot_roi_dram_msb_sao:    [0x0, 0xf],           bits : 3_0
*/
#define IME_3DNR_MOTION_STATUS_ROI_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1_OFS 0x0974
REGDEF_BEGIN(IME_3DNR_MOTION_STATUS_ROI_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1)
REGDEF_BIT(ime_3dnr_mot_roi_dram_msb_sao,        4)
REGDEF_END(IME_3DNR_MOTION_STATUS_ROI_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1)


/*
    ime_3dnr_mv_dram_smb_sai:    [0x0, 0xf],            bits : 3_0
*/
#define IME_3DNR_MOTION_VECTOR_INPUT_DMA_STARTING_ADDRESS_REGISTER1_OFS 0x0978
REGDEF_BEGIN(IME_3DNR_MOTION_VECTOR_INPUT_DMA_STARTING_ADDRESS_REGISTER1)
REGDEF_BIT(ime_3dnr_mv_dram_smb_sai,        4)
REGDEF_END(IME_3DNR_MOTION_VECTOR_INPUT_DMA_STARTING_ADDRESS_REGISTER1)


/*
    ime_3dnr_mv_dram_msb_sao:    [0x0, 0xf],            bits : 3_0
*/
#define IME_3DNR_MOTION_VECTOR_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1_OFS 0x097c
REGDEF_BEGIN(IME_3DNR_MOTION_VECTOR_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1)
REGDEF_BIT(ime_3dnr_mv_dram_msb_sao,        4)
REGDEF_END(IME_3DNR_MOTION_VECTOR_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1)


/*
    ime_3dnr_statistic_dram_msb_sao:    [0x0, 0xf],         bits : 3_0
*/
#define IME_3DNR_STATISTIC_DATA_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1_OFS 0x0980
REGDEF_BEGIN(IME_3DNR_STATISTIC_DATA_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1)
REGDEF_BIT(ime_3dnr_statistic_dram_msb_sao,        4)
REGDEF_END(IME_3DNR_STATISTIC_DATA_OUTPUT_DMA_STARTING_ADDRESS_REGISTER1)


/*
    ime_3dnr_fast_converge_dram_msb_sai:    [0x0, 0xf],         bits : 3_0
*/
#define IME_3DNR_FAST_CONVERGE_INPUT_DMA_ADDRESS_REGISTER1_OFS 0x0984
REGDEF_BEGIN(IME_3DNR_FAST_CONVERGE_INPUT_DMA_ADDRESS_REGISTER1)
REGDEF_BIT(ime_3dnr_fast_converge_dram_msb_sai,        4)
REGDEF_END(IME_3DNR_FAST_CONVERGE_INPUT_DMA_ADDRESS_REGISTER1)


/*
    ime_3dnr_fast_converge_dram_msb_sao:    [0x0, 0xf],         bits : 3_0
*/
#define IME_3DNR_FAST_CONVERGE_OUTPUT_DMA_ADDRESS_REGISTER1_OFS 0x0988
REGDEF_BEGIN(IME_3DNR_FAST_CONVERGE_OUTPUT_DMA_ADDRESS_REGISTER1)
REGDEF_BIT(ime_3dnr_fast_converge_dram_msb_sao,        4)
REGDEF_END(IME_3DNR_FAST_CONVERGE_OUTPUT_DMA_ADDRESS_REGISTER1)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_304_OFS 0x098c
REGDEF_BEGIN(IME_RESERVED_REGISTER_304)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_304)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_305_OFS 0x0990
REGDEF_BEGIN(IME_RESERVED_REGISTER_305)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_305)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_306_OFS 0x0994
REGDEF_BEGIN(IME_RESERVED_REGISTER_306)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_306)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_307_OFS 0x0998
REGDEF_BEGIN(IME_RESERVED_REGISTER_307)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_307)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_308_OFS 0x099c
REGDEF_BEGIN(IME_RESERVED_REGISTER_308)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_308)


/*
    ime_in_st_hinit_uv_ofs:    [0x0, 0xfffff],          bits : 19_0
*/
#define IME_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER1_OFS 0x09a0
REGDEF_BEGIN(IME_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER1)
REGDEF_BIT(ime_in_st_hinit_uv_ofs,        20)
REGDEF_END(IME_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER1)


/*
    ime_p1_out_st_hinit0_uv_ofs:    [0x0, 0xfffff],         bits : 19_0
*/
#define IME_OUTPUT_PATH0_CONTROL_REGISTER33_OFS 0x09a4
REGDEF_BEGIN(IME_OUTPUT_PATH0_CONTROL_REGISTER33)
REGDEF_BIT(ime_p1_out_st_hinit0_uv_ofs,        20)
REGDEF_END(IME_OUTPUT_PATH0_CONTROL_REGISTER33)


/*
    ime_p2_out_st_hinit0_uv_ofs:    [0x0, 0xfffff],         bits : 19_0
*/
#define IME_OUTPUT_PATH1_CONTROL_REGISTER33_OFS 0x09a8
REGDEF_BEGIN(IME_OUTPUT_PATH1_CONTROL_REGISTER33)
REGDEF_BIT(ime_p2_out_st_hinit0_uv_ofs,        20)
REGDEF_END(IME_OUTPUT_PATH1_CONTROL_REGISTER33)


/*
    ime_p3_out_st_hinit0_uv_ofs:    [0x0, 0xfffff],         bits : 19_0
*/
#define IME_OUTPUT_PATH2_CONTROL_REGISTER33_OFS 0x09ac
REGDEF_BEGIN(IME_OUTPUT_PATH2_CONTROL_REGISTER33)
REGDEF_BIT(ime_p3_out_st_hinit0_uv_ofs,        20)
REGDEF_END(IME_OUTPUT_PATH2_CONTROL_REGISTER33)


/*
    ime_p4_out_st_hinit0_uv_ofs:    [0x0, 0xfffff],         bits : 19_0
*/
#define IME_OUTPUT_PATH3_CONTROL_REGISTER33_OFS 0x09b0
REGDEF_BEGIN(IME_OUTPUT_PATH3_CONTROL_REGISTER33)
REGDEF_BIT(ime_p4_out_st_hinit0_uv_ofs,        20)
REGDEF_END(IME_OUTPUT_PATH3_CONTROL_REGISTER33)


/*
    ime_3dnr_ref_in_st_hinit_uv_ofs:    [0x0, 0xfffff],         bits : 19_0
*/
#define IME_3DNR_REFERENCE_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER1_OFS 0x09b4
REGDEF_BEGIN(IME_3DNR_REFERENCE_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER1)
REGDEF_BIT(ime_3dnr_ref_in_st_hinit_uv_ofs,        20)
REGDEF_END(IME_3DNR_REFERENCE_INPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER1)


/*
    ime_3dnr_ref_out_st_hinit_uv_ofs:    [0x0, 0xfffff],            bits : 19_0
*/
#define IME_3DNR_REFERENCE_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER1_OFS 0x09b8
REGDEF_BEGIN(IME_3DNR_REFERENCE_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER1)
REGDEF_BIT(ime_3dnr_ref_out_st_hinit_uv_ofs,        20)
REGDEF_END(IME_3DNR_REFERENCE_OUTPUT_FIRST_STRIPE_HORIZONTAL_START_POSITION_REGISTER1)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_316_OFS 0x09bc
REGDEF_BEGIN(IME_RESERVED_REGISTER_316)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_316)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_317_OFS 0x09c0
REGDEF_BEGIN(IME_RESERVED_REGISTER_317)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_317)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_318_OFS 0x09c4
REGDEF_BEGIN(IME_RESERVED_REGISTER_318)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_318)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_319_OFS 0x09c8
REGDEF_BEGIN(IME_RESERVED_REGISTER_319)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_319)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_320_OFS 0x09cc
REGDEF_BEGIN(IME_RESERVED_REGISTER_320)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_320)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_321_OFS 0x09d0
REGDEF_BEGIN(IME_RESERVED_REGISTER_321)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_321)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_322_OFS 0x09d4
REGDEF_BEGIN(IME_RESERVED_REGISTER_322)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_322)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_323_OFS 0x09d8
REGDEF_BEGIN(IME_RESERVED_REGISTER_323)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_323)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_324_OFS 0x09dc
REGDEF_BEGIN(IME_RESERVED_REGISTER_324)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_324)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_325_OFS 0x09e0
REGDEF_BEGIN(IME_RESERVED_REGISTER_325)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_325)


/*
    ime_3dnr_ref_out_min_y :    [0x0, 0xff],            bits : 7_0
    ime_3dnr_ref_out_max_y :    [0x0, 0xff],            bits : 15_8
    ime_3dnr_ref_out_min_uv:    [0x0, 0xff],            bits : 23_16
    ime_3dnr_ref_out_max_uv:    [0x0, 0xff],            bits : 31_24
*/
#define IME_3DNR_REFERENCE_OUTPUT_Y_AND_UV_RANGE_CLAMP_REGISTER0_OFS 0x09e4
REGDEF_BEGIN(IME_3DNR_REFERENCE_OUTPUT_Y_AND_UV_RANGE_CLAMP_REGISTER0)
REGDEF_BIT(ime_3dnr_ref_out_min_y,        8)
REGDEF_BIT(ime_3dnr_ref_out_max_y,        8)
REGDEF_BIT(ime_3dnr_ref_out_min_uv,        8)
REGDEF_BIT(ime_3dnr_ref_out_max_uv,        8)
REGDEF_END(IME_3DNR_REFERENCE_OUTPUT_Y_AND_UV_RANGE_CLAMP_REGISTER0)


/*
    ime_axi_channel_en:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_AXI_BUS_CONTROL_REG0_OFS 0x09e8
REGDEF_BEGIN(IME_AXI_BUS_CONTROL_REG0)
REGDEF_BIT(ime_axi_channel0_en,        32)
REGDEF_END(IME_AXI_BUS_CONTROL_REG0)


/*
    ime_axi_channel_en:    [0x0, 0x3f],         bits : 5_0
    ime_axi_disable   :    [0x0, 0x1],          bits : 31
*/
#define IME_AXI_BUS_CONTROL_REG1_OFS 0x09ec
REGDEF_BEGIN(IME_AXI_BUS_CONTROL_REG1)
REGDEF_BIT(ime_axi_channel32_en,        6)
REGDEF_BIT(,        25)
REGDEF_BIT(ime_axi_disable,        1)
REGDEF_END(IME_AXI_BUS_CONTROL_REG1)


/*
    ime_r_ostd_num :    [0x0, 0xff],            bits : 7_0
    ime_w_ostd_num :    [0x0, 0xff],            bits : 15_8
    ime_axi_ch_idle:    [0x0, 0x1],         bits : 31
*/
#define IME_AXI_BUS_CONTROL_REG2_OFS 0x09f0
REGDEF_BEGIN(IME_AXI_BUS_CONTROL_REG2)
REGDEF_BIT(ime_r_ostd_num,        8)
REGDEF_BIT(ime_w_ostd_num,        8)
REGDEF_BIT(,        15)
REGDEF_BIT(ime_axi_ch_idle,        1)
REGDEF_END(IME_AXI_BUS_CONTROL_REG2)


/*
    ime_axi_lock_dis:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_AXI_BUS_CONTROL_REG3_OFS 0x09f4
REGDEF_BEGIN(IME_AXI_BUS_CONTROL_REG3)
REGDEF_BIT(ime_axi_lock0_dis,        32)
REGDEF_END(IME_AXI_BUS_CONTROL_REG3)


/*
    ime_axi_lock_dis:    [0x0, 0x3f],           bits : 5_0
*/
#define IME_AXI_BUS_CONTROL_REG4_OFS 0x09f8
REGDEF_BEGIN(IME_AXI_BUS_CONTROL_REG4)
REGDEF_BIT(ime_axi_lock32_dis,        6)
REGDEF_END(IME_AXI_BUS_CONTROL_REG4)


/*
    ime_axi_ch_sta:    [0x0, 0xffffffff],           bits : 31_0
*/
#define IME_AXI_BUS_CONTROL_REG5_OFS 0x09FC
REGDEF_BEGIN(IME_AXI_BUS_CONTROL_REG5)
REGDEF_BIT(ime_axi_ch_sta,        32)
REGDEF_END(IME_AXI_BUS_CONTROL_REG5)


/*
    ime_p0_enc_smode_en            :    [0x0, 0x1],         bits : 0
    ime_3dnr_refout_enc_smode_en   :    [0x0, 0x1],         bits : 4
    ime_3dnr_refin_dec_smode_en    :    [0x0, 0x1],         bits : 8
    ime_3dnr_refin_dec_dither_en   :    [0x0, 0x1],         bits : 10
    ime_3dnr_refin_dec_dither_seed0:    [0x0, 0x7fff],          bits : 30_16
    ime_3dnr_refin_dec_dither_reset:    [0x0, 0x1],         bits : 31
*/
#define IME_COMPRESSION_CONTROL_REGISTER0_OFS 0x0a00
REGDEF_BEGIN(IME_COMPRESSION_CONTROL_REGISTER0)
REGDEF_BIT(ime_p0_enc_smode_en,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_3dnr_refout_enc_smode_en,        1)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_3dnr_refin_dec_smode_en,        1)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_refin_dec_dither_en,        1)
REGDEF_BIT(,        5)
REGDEF_BIT(ime_3dnr_refin_dec_dither_seed0,        15)
REGDEF_BIT(ime_3dnr_refin_dec_dither_reset,        1)
REGDEF_END(IME_COMPRESSION_CONTROL_REGISTER0)


/*
    ime_3dnr_refin_dec_dither_seed1:    [0x0, 0xf],         bits : 3_0
*/
#define IME_COMPRESSION_CONTROL_REGISTER1_OFS 0x0a04
REGDEF_BEGIN(IME_COMPRESSION_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_refin_dec_dither_seed1,        4)
REGDEF_END(IME_COMPRESSION_CONTROL_REGISTER1)


/*
    dct_level_th0:    [0x0, 0xff],          bits : 7_0
    dct_level_th1:    [0x0, 0xff],          bits : 15_8
    dct_level_th2:    [0x0, 0xff],          bits : 23_16
    dct_level_th3:    [0x0, 0xff],          bits : 31_24
*/
#define IME_ENCODER_QUALITY_THRESHOLD_REGISTER0_OFS 0x0a08
REGDEF_BEGIN(IME_ENCODER_QUALITY_THRESHOLD_REGISTER0)
REGDEF_BIT(dct_level_th0,        8)
REGDEF_BIT(dct_level_th1,        8)
REGDEF_BIT(dct_level_th2,        8)
REGDEF_BIT(dct_level_th3,        8)
REGDEF_END(IME_ENCODER_QUALITY_THRESHOLD_REGISTER0)


/*
    dct_level_th4_:    [0x0, 0xff],         bits : 7_0
    dct_level_th5 :    [0x0, 0xff],         bits : 15_8
    dct_level_th6 :    [0x0, 0xff],         bits : 23_16
    dct_level_th7 :    [0x0, 0xff],         bits : 31_24
*/
#define IME_ENCODER_QUALITY_THRESHOLD_REGISTER1_OFS 0x0a0c
REGDEF_BEGIN(IME_ENCODER_QUALITY_THRESHOLD_REGISTER1)
REGDEF_BIT(dct_level_th4,        8)
REGDEF_BIT(dct_level_th5,        8)
REGDEF_BIT(dct_level_th6,        8)
REGDEF_BIT(dct_level_th7,        8)
REGDEF_END(IME_ENCODER_QUALITY_THRESHOLD_REGISTER1)


/*
    dct_qtbl0_idx:    [0x0, 0x1f],          bits : 4_0
    dct_qtbl1_idx:    [0x0, 0x1f],          bits : 12_8
    dct_qtbl2_idx:    [0x0, 0x1f],          bits : 20_16
    dct_qtbl3_idx:    [0x0, 0x1f],          bits : 28_24
*/
#define IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER0_OFS 0x0a10
REGDEF_BEGIN(IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER0)
REGDEF_BIT(dct_qtbl0_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl1_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl2_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl3_idx,        5)
REGDEF_END(IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER0)


/*
    dct_qtbl4_idx:    [0x0, 0x1f],          bits : 4_0
    dct_qtbl5_idx:    [0x0, 0x1f],          bits : 12_8
    dct_qtbl6_idx:    [0x0, 0x1f],          bits : 20_16
    dct_qtbl7_idx:    [0x0, 0x1f],          bits : 28_24
*/
#define IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER1_OFS 0x0a14
REGDEF_BEGIN(IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER1)
REGDEF_BIT(dct_qtbl4_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl5_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl6_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl7_idx,        5)
REGDEF_END(IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER1)


/*
    dct_qtbl8_idx :    [0x0, 0x1f],         bits : 4_0
    dct_qtbl9_idx :    [0x0, 0x1f],         bits : 12_8
    dct_qtbl10_idx:    [0x0, 0x1f],         bits : 20_16
    dct_qtbl11_idx:    [0x0, 0x1f],         bits : 28_24
*/
#define IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER2_OFS 0x0a18
REGDEF_BEGIN(IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER2)
REGDEF_BIT(dct_qtbl8_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl9_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl10_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl11_idx,        5)
REGDEF_END(IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER2)


/*
    dct_qtbl12_idx:    [0x0, 0x1f],         bits : 4_0
    dct_qtbl13_idx:    [0x0, 0x1f],         bits : 12_8
    dct_qtbl14_idx:    [0x0, 0x1f],         bits : 20_16
    dct_qtbl15_idx:    [0x0, 0x1f],         bits : 28_24
*/
#define IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER3_OFS 0x0a1c
REGDEF_BEGIN(IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER3)
REGDEF_BIT(dct_qtbl12_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl13_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl14_idx,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(dct_qtbl15_idx,        5)
REGDEF_END(IME_ENCODER_QUALITY_TABLE_INDEX_REGISTER3)


/*
    yrc_lncnt_lfn0:    [0x0, 0x1fff],           bits : 12_0
    yrc_lncnt_lfn1:    [0x0, 0xfff],            bits : 27_16
*/
#define IME_ENCODER_QUALITY_TABLE_CHECK_REGISTER0_OFS 0x0a20
REGDEF_BEGIN(IME_ENCODER_QUALITY_TABLE_CHECK_REGISTER0)
REGDEF_BIT(yrc_lncnt_lfn0,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(yrc_lncnt_lfn1,        12)
REGDEF_END(IME_ENCODER_QUALITY_TABLE_CHECK_REGISTER0)


/*
    yrc_lncnt_lfn2:    [0x0, 0x1fff],           bits : 12_0
    yrc_lncnt_lfn3:    [0x0, 0xfff],            bits : 27_16
*/
#define IME_ENCODER_QUALITY_TABLE_CHECK_REGISTER1_OFS 0x0a24
REGDEF_BEGIN(IME_ENCODER_QUALITY_TABLE_CHECK_REGISTER1)
REGDEF_BIT(yrc_lncnt_lfn2,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(yrc_lncnt_lfn3,        12)
REGDEF_END(IME_ENCODER_QUALITY_TABLE_CHECK_REGISTER1)


/*
    yrc_lncnt_lfn4:    [0x0, 0x1fff],           bits : 12_0
    yrc_lncnt_lfn5:    [0x0, 0xfff],            bits : 27_16
*/
#define IMR_ENCODER_QUALITY_TABLE_CHECK_REGISTER2_OFS 0x0a28
REGDEF_BEGIN(IMR_ENCODER_QUALITY_TABLE_CHECK_REGISTER2)
REGDEF_BIT(yrc_lncnt_lfn4,        13)
REGDEF_BIT(,        3)
REGDEF_BIT(yrc_lncnt_lfn5,        12)
REGDEF_END(IMR_ENCODER_QUALITY_TABLE_CHECK_REGISTER2)


/*
    dct_maxdist:    [0x0, 0xff],            bits : 7_0
*/
#define IME_ENCODER_MAX_ERROR_FOR_DCT_REGISTER_OFS 0x0a2c
REGDEF_BEGIN(IME_ENCODER_MAX_ERROR_FOR_DCT_REGISTER)
REGDEF_BIT(dct_maxdist,        8)
REGDEF_END(IME_ENCODER_MAX_ERROR_FOR_DCT_REGISTER)


/*
    ime_in_dec_smode_en    :    [0x0, 0x1],         bits : 8
    ime_in_dec_dither_en   :    [0x0, 0x1],         bits : 10
    ime_in_dec_dither_seed0:    [0x0, 0x7fff],          bits : 30_16
    ime_in_dec_dither_reset:    [0x0, 0x1],         bits : 31
*/
#define IME_COMPRESSION_CONTROL_REGISTER2_OFS 0x0a30
REGDEF_BEGIN(IME_COMPRESSION_CONTROL_REGISTER2)
REGDEF_BIT(,        8)
REGDEF_BIT(ime_in_dec_smode_en,        1)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_in_dec_dither_en,        1)
REGDEF_BIT(,        5)
REGDEF_BIT(ime_in_dec_dither_seed0,        15)
REGDEF_BIT(ime_in_dec_dither_reset,        1)
REGDEF_END(IME_COMPRESSION_CONTROL_REGISTER2)


/*
    ime_in_dec_dither_seed1:    [0x0, 0xf],         bits : 3_0
*/
#define IME_COMPRESSION_CONTROL_REGISTER3_OFS 0x0a34
REGDEF_BEGIN(IME_COMPRESSION_CONTROL_REGISTER3)
REGDEF_BIT(ime_in_dec_dither_seed1,        4)
REGDEF_END(IME_COMPRESSION_CONTROL_REGISTER3)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_327_OFS 0x0a38
REGDEF_BEGIN(IME_RESERVED_REGISTER_327)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_327)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_328_OFS 0x0a3c
REGDEF_BEGIN(IME_RESERVED_REGISTER_328)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_328)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_329_OFS 0x0a40
REGDEF_BEGIN(IME_RESERVED_REGISTER_329)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_329)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_330_OFS 0x0a44
REGDEF_BEGIN(IME_RESERVED_REGISTER_330)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_330)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_331_OFS 0x0a48
REGDEF_BEGIN(IME_RESERVED_REGISTER_331)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_331)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_332_OFS 0x0a4c
REGDEF_BEGIN(IME_RESERVED_REGISTER_332)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_332)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_333_OFS 0x0a50
REGDEF_BEGIN(IME_RESERVED_REGISTER_333)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_333)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_334_OFS 0x0a54
REGDEF_BEGIN(IME_RESERVED_REGISTER_334)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_334)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_335_OFS 0x0a58
REGDEF_BEGIN(IME_RESERVED_REGISTER_335)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_335)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_336_OFS 0x0a5c
REGDEF_BEGIN(IME_RESERVED_REGISTER_336)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_336)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_337_OFS 0x0a60
REGDEF_BEGIN(IME_RESERVED_REGISTER_337)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_337)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_338_OFS 0x0a64
REGDEF_BEGIN(IME_RESERVED_REGISTER_338)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_338)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_339_OFS 0x0a68
REGDEF_BEGIN(IME_RESERVED_REGISTER_339)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_339)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_340_OFS 0x0a6c
REGDEF_BEGIN(IME_RESERVED_REGISTER_340)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_340)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_341_OFS 0x0a70
REGDEF_BEGIN(IME_RESERVED_REGISTER_341)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_341)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_342_OFS 0x0a74
REGDEF_BEGIN(IME_RESERVED_REGISTER_342)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_342)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_343_OFS 0x0a78
REGDEF_BEGIN(IME_RESERVED_REGISTER_343)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_343)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_344_OFS 0x0a7c
REGDEF_BEGIN(IME_RESERVED_REGISTER_344)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_344)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_345_OFS 0x0a80
REGDEF_BEGIN(IME_RESERVED_REGISTER_345)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_345)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_346_OFS 0x0a84
REGDEF_BEGIN(IME_RESERVED_REGISTER_346)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_346)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_347_OFS 0x0a88
REGDEF_BEGIN(IME_RESERVED_REGISTER_347)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_347)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_348_OFS 0x0a8c
REGDEF_BEGIN(IME_RESERVED_REGISTER_348)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_348)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_349_OFS 0x0a90
REGDEF_BEGIN(IME_RESERVED_REGISTER_349)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_349)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_350_OFS 0x0a94
REGDEF_BEGIN(IME_RESERVED_REGISTER_350)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_350)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_351_OFS 0x0a98
REGDEF_BEGIN(IME_RESERVED_REGISTER_351)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_351)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_352_OFS 0x0a9c
REGDEF_BEGIN(IME_RESERVED_REGISTER_352)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_352)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_353_OFS 0x0aa0
REGDEF_BEGIN(IME_RESERVED_REGISTER_353)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_353)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_354_OFS 0x0aa4
REGDEF_BEGIN(IME_RESERVED_REGISTER_354)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_354)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_355_OFS 0x0aa8
REGDEF_BEGIN(IME_RESERVED_REGISTER_355)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_355)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_356_OFS 0x0aac
REGDEF_BEGIN(IME_RESERVED_REGISTER_356)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_356)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_357_OFS 0x0ab0
REGDEF_BEGIN(IME_RESERVED_REGISTER_357)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_357)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_358_OFS 0x0ab4
REGDEF_BEGIN(IME_RESERVED_REGISTER_358)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_358)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_359_OFS 0x0ab8
REGDEF_BEGIN(IME_RESERVED_REGISTER_359)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_359)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_360_OFS 0x0abc
REGDEF_BEGIN(IME_RESERVED_REGISTER_360)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_360)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_361_OFS 0x0ac0
REGDEF_BEGIN(IME_RESERVED_REGISTER_361)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_361)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_362_OFS 0x0ac4
REGDEF_BEGIN(IME_RESERVED_REGISTER_362)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_362)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_363_OFS 0x0ac8
REGDEF_BEGIN(IME_RESERVED_REGISTER_363)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_363)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_364_OFS 0x0acc
REGDEF_BEGIN(IME_RESERVED_REGISTER_364)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_364)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_365_OFS 0x0ad0
REGDEF_BEGIN(IME_RESERVED_REGISTER_365)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_365)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_366_OFS 0x0ad4
REGDEF_BEGIN(IME_RESERVED_REGISTER_366)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_366)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_367_OFS 0x0ad8
REGDEF_BEGIN(IME_RESERVED_REGISTER_367)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_367)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_368_OFS 0x0adc
REGDEF_BEGIN(IME_RESERVED_REGISTER_368)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_368)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_369_OFS 0x0ae0
REGDEF_BEGIN(IME_RESERVED_REGISTER_369)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_369)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_370_OFS 0x0ae4
REGDEF_BEGIN(IME_RESERVED_REGISTER_370)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_370)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_371_OFS 0x0ae8
REGDEF_BEGIN(IME_RESERVED_REGISTER_371)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_371)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_372_OFS 0x0aec
REGDEF_BEGIN(IME_RESERVED_REGISTER_372)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_372)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_373_OFS 0x0af0
REGDEF_BEGIN(IME_RESERVED_REGISTER_373)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_373)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_374_OFS 0x0af4
REGDEF_BEGIN(IME_RESERVED_REGISTER_374)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_374)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_375_OFS 0x0af8
REGDEF_BEGIN(IME_RESERVED_REGISTER_375)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_375)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_376_OFS 0x0afc
REGDEF_BEGIN(IME_RESERVED_REGISTER_376)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_376)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_377_OFS 0x0b00
REGDEF_BEGIN(IME_RESERVED_REGISTER_377)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_377)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_378_OFS 0x0b04
REGDEF_BEGIN(IME_RESERVED_REGISTER_378)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_378)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_379_OFS 0x0b08
REGDEF_BEGIN(IME_RESERVED_REGISTER_379)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_379)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_380_OFS 0x0b0c
REGDEF_BEGIN(IME_RESERVED_REGISTER_380)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_380)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_381_OFS 0x0b10
REGDEF_BEGIN(IME_RESERVED_REGISTER_381)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_381)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_382_OFS 0x0b14
REGDEF_BEGIN(IME_RESERVED_REGISTER_382)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_382)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_383_OFS 0x0b18
REGDEF_BEGIN(IME_RESERVED_REGISTER_383)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_383)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_384_OFS 0x0b1c
REGDEF_BEGIN(IME_RESERVED_REGISTER_384)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_384)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_385_OFS 0x0b20
REGDEF_BEGIN(IME_RESERVED_REGISTER_385)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_385)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_386_OFS 0x0b24
REGDEF_BEGIN(IME_RESERVED_REGISTER_386)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_386)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_387_OFS 0x0b28
REGDEF_BEGIN(IME_RESERVED_REGISTER_387)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_387)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_388_OFS 0x0b2c
REGDEF_BEGIN(IME_RESERVED_REGISTER_388)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_388)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_389_OFS 0x0b30
REGDEF_BEGIN(IME_RESERVED_REGISTER_389)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_389)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_390_OFS 0x0b34
REGDEF_BEGIN(IME_RESERVED_REGISTER_390)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_390)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_391_OFS 0x0b38
REGDEF_BEGIN(IME_RESERVED_REGISTER_391)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_391)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_392_OFS 0x0b3c
REGDEF_BEGIN(IME_RESERVED_REGISTER_392)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_392)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_393_OFS 0x0b40
REGDEF_BEGIN(IME_RESERVED_REGISTER_393)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_393)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_394_OFS 0x0b44
REGDEF_BEGIN(IME_RESERVED_REGISTER_394)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_394)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_395_OFS 0x0b48
REGDEF_BEGIN(IME_RESERVED_REGISTER_395)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_395)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_396_OFS 0x0b4c
REGDEF_BEGIN(IME_RESERVED_REGISTER_396)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_396)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_397_OFS 0x0b50
REGDEF_BEGIN(IME_RESERVED_REGISTER_397)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_397)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_398_OFS 0x0b54
REGDEF_BEGIN(IME_RESERVED_REGISTER_398)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_398)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_399_OFS 0x0b58
REGDEF_BEGIN(IME_RESERVED_REGISTER_399)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_399)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_400_OFS 0x0b5c
REGDEF_BEGIN(IME_RESERVED_REGISTER_400)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_400)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_401_OFS 0x0b60
REGDEF_BEGIN(IME_RESERVED_REGISTER_401)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_401)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_402_OFS 0x0b64
REGDEF_BEGIN(IME_RESERVED_REGISTER_402)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_402)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_403_OFS 0x0b68
REGDEF_BEGIN(IME_RESERVED_REGISTER_403)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_403)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_404_OFS 0x0b6c
REGDEF_BEGIN(IME_RESERVED_REGISTER_404)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_404)


/*
    lca_proc_lcon_sel :    [0x0, 0x1],          bits : 0
    lca_edge_ker_size :    [0x0, 0x1],          bits : 1
    lca_sr_rf_ker_size:    [0x0, 0x7],          bits : 6_4
    lca_mr_rf_ker_size:    [0x0, 0x7],          bits : 10_8
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER0_OFS 0x0b70
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER0)
REGDEF_BIT(lca_proc_lcon_sel,        1)
REGDEF_BIT(lca_edge_ker_size,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_sr_rf_ker_size,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(lca_mr_rf_ker_size,        3)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER0)


/*
    lca_ctr_y_th0:    [0x0, 0xff],          bits : 7_0
    lca_ctr_y_th1:    [0x0, 0xff],          bits : 15_8
    lca_ctr_y_th2:    [0x0, 0xff],          bits : 23_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER1_OFS 0x0b74
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER1)
REGDEF_BIT(lca_ctr_y_th0,        8)
REGDEF_BIT(lca_ctr_y_th1,        8)
REGDEF_BIT(lca_ctr_y_th2,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER1)


/*
    lca_ctr_u_th0:    [0x0, 0xff],          bits : 7_0
    lca_ctr_u_th1:    [0x0, 0xff],          bits : 15_8
    lca_ctr_u_th2:    [0x0, 0xff],          bits : 23_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER2_OFS 0x0b78
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER2)
REGDEF_BIT(lca_ctr_u_th0,        8)
REGDEF_BIT(lca_ctr_u_th1,        8)
REGDEF_BIT(lca_ctr_u_th2,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER2)


/*
    lca_ctr_v_th0:    [0x0, 0xff],          bits : 7_0
    lca_ctr_v_th1:    [0x0, 0xff],          bits : 15_8
    lca_ctr_v_th2:    [0x0, 0xff],          bits : 23_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER3_OFS 0x0b7c
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER3)
REGDEF_BIT(lca_ctr_v_th0,        8)
REGDEF_BIT(lca_ctr_v_th1,        8)
REGDEF_BIT(lca_ctr_v_th2,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER3)


/*
    lca_ref_wet_smooth_y :    [0x0, 0x3f],          bits : 5_0
    lca_ref_wet_edge_y   :    [0x0, 0x3f],          bits : 13_8
    lca_ref_wet_smooth_uv:    [0x0, 0x3f],          bits : 21_16
    lca_ref_wet_edge_uv  :    [0x0, 0x3f],          bits : 29_24
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER4_OFS 0x0b80
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER4)
REGDEF_BIT(lca_ref_wet_smooth_y,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_ref_wet_edge_y,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_ref_wet_smooth_uv,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_ref_wet_edge_uv,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER4)


/*
    lca_still_smooth_y_th0:    [0x0, 0xff],         bits : 7_0
    lca_still_smooth_y_th1:    [0x0, 0xff],         bits : 15_8
    lca_still_smooth_y_th2:    [0x0, 0xff],         bits : 23_16
    lca_still_smooth_y_th3:    [0x0, 0xff],         bits : 31_24
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER5_OFS 0x0b84
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER5)
REGDEF_BIT(lca_still_smooth_y_th0,        8)
REGDEF_BIT(lca_still_smooth_y_th1,        8)
REGDEF_BIT(lca_still_smooth_y_th2,        8)
REGDEF_BIT(lca_still_smooth_y_th3,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER5)


/*
    lca_still_smooth_y_th4:    [0x0, 0xff],         bits : 7_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER6_OFS 0x0b88
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER6)
REGDEF_BIT(lca_still_smooth_y_th4,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER6)


/*
    lca_still_smooth_uv_th0:    [0x0, 0xffff],          bits : 15_0
    lca_still_smooth_uv_th1:    [0x0, 0xffff],          bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER7_OFS 0x0b8c
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER7)
REGDEF_BIT(lca_still_smooth_uv_th0,        16)
REGDEF_BIT(lca_still_smooth_uv_th1,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER7)


/*
    lca_still_smooth_uv_th2:    [0x0, 0xffff],          bits : 15_0
    lca_still_smooth_uv_th3:    [0x0, 0xffff],          bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER8_OFS 0x0b90
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER8)
REGDEF_BIT(lca_still_smooth_uv_th2,        16)
REGDEF_BIT(lca_still_smooth_uv_th3,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER8)


/*
    lca_still_smooth_uv_th4:    [0x0, 0xffff],          bits : 15_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER9_OFS 0x0b94
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER9)
REGDEF_BIT(lca_still_smooth_uv_th4,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER9)


/*
    lca_still_edge_y_th0:    [0x0, 0xff],           bits : 7_0
    lca_still_edge_y_th1:    [0x0, 0xff],           bits : 15_8
    lca_still_edge_y_th2:    [0x0, 0xff],           bits : 23_16
    lca_still_edge_y_th3:    [0x0, 0xff],           bits : 31_24
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER10_OFS 0x0b98
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER10)
REGDEF_BIT(lca_still_edge_y_th0,        8)
REGDEF_BIT(lca_still_edge_y_th1,        8)
REGDEF_BIT(lca_still_edge_y_th2,        8)
REGDEF_BIT(lca_still_edge_y_th3,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER10)


/*
    lca_still_edge_y_th4:    [0x0, 0xff],           bits : 7_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER11_OFS 0x0b9c
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER11)
REGDEF_BIT(lca_still_edge_y_th4,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER11)


/*
    lca_still_edge_uv_th0:    [0x0, 0xffff],            bits : 15_0
    lca_still_edge_uv_th1:    [0x0, 0xffff],            bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER12_OFS 0x0ba0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER12)
REGDEF_BIT(lca_still_edge_uv_th0,        16)
REGDEF_BIT(lca_still_edge_uv_th1,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER12)


/*
    lca_still_edge_uv_th2:    [0x0, 0xffff],            bits : 15_0
    lca_still_edge_uv_th3:    [0x0, 0xffff],            bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER13_OFS 0x0ba4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER13)
REGDEF_BIT(lca_still_edge_uv_th2,        16)
REGDEF_BIT(lca_still_edge_uv_th3,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER13)


/*
    lca_still_edge_uv_th4:    [0x0, 0xffff],            bits : 15_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER14_OFS 0x0ba8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER14)
REGDEF_BIT(lca_still_edge_uv_th4,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER14)


/*
    lca_motion_y_th0:    [0x0, 0xff],           bits : 7_0
    lca_motion_y_th1:    [0x0, 0xff],           bits : 15_8
    lca_motion_y_th2:    [0x0, 0xff],           bits : 23_16
    lca_motion_y_th3:    [0x0, 0xff],           bits : 31_24
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER15_OFS 0x0bac
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER15)
REGDEF_BIT(lca_motion_y_th0,        8)
REGDEF_BIT(lca_motion_y_th1,        8)
REGDEF_BIT(lca_motion_y_th2,        8)
REGDEF_BIT(lca_motion_y_th3,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER15)


/*
    lca_motion_y_th4:    [0x0, 0xff],           bits : 7_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER16_OFS 0x0bb0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER16)
REGDEF_BIT(lca_motion_y_th4,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER16)


/*
    lca_motion_uv_th0:    [0x0, 0xffff],            bits : 15_0
    lca_motion_uv_th1:    [0x0, 0xffff],            bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER17_OFS 0x0bb4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER17)
REGDEF_BIT(lca_motion_uv_th0,        16)
REGDEF_BIT(lca_motion_uv_th1,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER17)


/*
    lca_motion_uv_th2:    [0x0, 0xffff],            bits : 15_0
    lca_motion_uv_th3:    [0x0, 0xffff],            bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER18_OFS 0x0bb8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER18)
REGDEF_BIT(lca_motion_uv_th2,        16)
REGDEF_BIT(lca_motion_uv_th3,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER18)


/*
    lca_motion_uv_th4:    [0x0, 0xffff],            bits : 15_0
    lca_out_wet_y    :    [0x0, 0x3f],          bits : 25_20
    lca_out_wet_uv   :    [0x0, 0x3f],          bits : 31_26
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER19_OFS 0x0bbc
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER19)
REGDEF_BIT(lca_motion_uv_th4,        16)
REGDEF_BIT(,        4)
REGDEF_BIT(lca_out_wet_y,        6)
REGDEF_BIT(lca_out_wet_uv,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER19)


/*
    lca_still_y_gain0:    [0x0, 0x3f],          bits : 5_0
    lca_still_y_gain1:    [0x0, 0x3f],          bits : 13_8
    lca_still_y_gain2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER20_OFS 0x0bc0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER20)
REGDEF_BIT(lca_still_y_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_y_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_y_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER20)


/*
    lca_motion_y_gain0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_y_gain1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_y_gain2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER21_OFS 0x0bc4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER21)
REGDEF_BIT(lca_motion_y_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_y_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_y_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER21)


/*
    lca_still_u_gain0:    [0x0, 0x3f],          bits : 5_0
    lca_still_u_gain1:    [0x0, 0x3f],          bits : 13_8
    lca_still_u_gain2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER22_OFS 0x0bc8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER22)
REGDEF_BIT(lca_still_u_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_u_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_u_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER22)


/*
    lca_motion_u_gain0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_u_gain1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_u_gain2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER23_OFS 0x0bcc
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER23)
REGDEF_BIT(lca_motion_u_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_u_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_u_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER23)


/*
    lca_still_v_gain0:    [0x0, 0x3f],          bits : 5_0
    lca_still_v_gain1:    [0x0, 0x3f],          bits : 13_8
    lca_still_v_gain2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER24_OFS 0x0bd0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER24)
REGDEF_BIT(lca_still_v_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_v_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_v_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER24)


/*
    lca_motion_v_gain0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_v_gain1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_v_gain2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER25_OFS 0x0bd4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER25)
REGDEF_BIT(lca_motion_v_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_v_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_v_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER25)


/*
    lca_still_y_coff0:    [0x0, 0x3f],          bits : 5_0
    lca_still_y_coff1:    [0x0, 0x3f],          bits : 13_8
    lca_still_y_coff2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER26_OFS 0x0bd8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER26)
REGDEF_BIT(lca_still_y_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_y_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_y_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER26)


/*
    lca_motion_y_coff0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_y_coff1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_y_coff2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER27_OFS 0x0bdc
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER27)
REGDEF_BIT(lca_motion_y_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_y_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_y_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER27)


/*
    lca_still_u_coff0:    [0x0, 0x3f],          bits : 5_0
    lca_still_u_coff1:    [0x0, 0x3f],          bits : 13_8
    lca_still_u_coff2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER28_OFS 0x0be0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER28)
REGDEF_BIT(lca_still_u_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_u_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_u_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER28)


/*
    lca_motion_u_coff0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_u_coff1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_u_coff2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER29_OFS 0x0be4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER29)
REGDEF_BIT(lca_motion_u_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_u_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_u_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER29)


/*
    lca_still_v_coff0:    [0x0, 0x3f],          bits : 5_0
    lca_still_v_coff1:    [0x0, 0x3f],          bits : 13_8
    lca_still_v_coff2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER30_OFS 0x0be8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER30)
REGDEF_BIT(lca_still_v_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_v_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_v_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER30)


/*
    lca_motion_v_coff0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_v_coff1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_v_coff2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER31_OFS 0x0bec
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER31)
REGDEF_BIT(lca_motion_v_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_v_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_v_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER31)


/*
    lca_dbg_en     :    [0x0, 0x1],         bits : 0
    lca_dbg_chl_sel:    [0x0, 0xf],         bits : 4_1
    lca_dbg_chl_ofs:    [0x0, 0xff],            bits : 12_5
    lca_dbg_x_pos  :    [0x0, 0xffff],          bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER32_OFS 0x0bf0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER32)
REGDEF_BIT(lca_dbg_en,        1)
REGDEF_BIT(lca_dbg_chl_sel,        4)
REGDEF_BIT(lca_dbg_chl_ofs,        8)
REGDEF_BIT(,        3)
REGDEF_BIT(lca_dbg_x_pos,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER32)


/*
    lca_edge_th0:    [0x0, 0x1ffff],            bits : 16_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER33_OFS 0x0bf4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER33)
REGDEF_BIT(lca_edge_th0,        17)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER33)


/*
    lca_edge_th1:    [0x0, 0x1ffff],            bits : 16_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER34_OFS 0x0bf8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER34)
REGDEF_BIT(lca_edge_th1,        17)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER34)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_REGISTER_412_OFS 0x0bfc
REGDEF_BEGIN(IME_RESERVED_REGISTER_412)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_REGISTER_412)


/*
    nn_isp_p2_ringbuf_height:    [0x0, 0x1fff],         bits : 12_0
    nn_isp_p2_slice_height  :    [0x0, 0x7ff],          bits : 26_16
    nn_isp_p2_fw_hs_en  :           [0x0, 0x1],          bits : 29
*/
#define IME_NN_ISP_PATH2_REGISTER0_OFS 0x0c00
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER0)
REGDEF_BIT(nn_isp_p2_ringbuf_height,        16)
REGDEF_BIT(nn_isp_p2_slice_height,        14)
REGDEF_BIT(,        1)
REGDEF_BIT(nn_isp_p2_fw_hs_en,        1)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER0)


/*
    nn_isp_p2_slice_ovlp   :    [0x0, 0x7f],            bits : 6_0
    nn_isp_p2_outbuf_height:    [0x0, 0x7ff],           bits : 26_16
    nn_isp_p2_outbuf_num   :    [0x0, 0x1],         bits : 31
*/
#define IME_NN_ISP_PATH2_REGISTER1_OFS 0x0c04
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER1)
REGDEF_BIT(nn_isp_p2_slice_ovlp,        7)
REGDEF_BIT(,        9)
REGDEF_BIT(nn_isp_p2_outbuf_height,        14)
REGDEF_BIT(,        1)
REGDEF_BIT(nn_isp_p2_outbuf_num,        1)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER1)


/*
    nn_isp_p2_yuv_ofsi:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_NN_ISP_PATH2_REGISTER2_OFS 0x0c08
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER2)
REGDEF_BIT(,        2)
REGDEF_BIT(nn_isp_p2_yuv_ofsi,        18)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER2)


/*
    nn_isp_p2_y_sai:    [0x0, 0xffffffff],          bits : 31_0
*/
#define IME_NN_ISP_PATH2_REGISTER3_OFS 0x0c0c
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER3)
REGDEF_BIT(nn_isp_p2_y_sai,        32)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER3)


/*
    nn_isp_p2_uv_sai:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_NN_ISP_PATH2_REGISTER4_OFS 0x0c10
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER4)
REGDEF_BIT(nn_isp_p2_uv_sai,        32)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER4)


/*
    nn_isp_p2_msb_y_sai :    [0x0, 0xf],            bits : 3_0
    nn_isp_p2_msb_uv_sai:    [0x0, 0xf],            bits : 19_16
*/
#define IME_NN_ISP_PATH2_REGISTER5_OFS 0x0c14
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER5)
REGDEF_BIT(nn_isp_p2_msb_y_sai,        4)
REGDEF_BIT(,        12)
REGDEF_BIT(nn_isp_p2_msb_uv_sai,        4)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER5)


/*
    nn_isp_p2_yuv_ofso:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_NN_ISP_PATH2_REGISTER6_OFS 0x0c18
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER6)
REGDEF_BIT(,        2)
REGDEF_BIT(nn_isp_p2_yuv_ofso,        18)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER6)


/*
    nn_isp_p2_y_sao:    [0x0, 0xffffffff],          bits : 31_0
*/
#define IME_NN_ISP_PATH2_REGISTER7_OFS 0x0c1c
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER7)
REGDEF_BIT(nn_isp_p2_y_sao,        32)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER7)


/*
    nn_isp_p2_uv_sao:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_NN_ISP_PATH2_REGISTER8_OFS 0x0c20
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER8)
REGDEF_BIT(nn_isp_p2_uv_sao,        32)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER8)


/*
    nn_isp_p2_msb_y_sao :    [0x0, 0xf],            bits : 3_0
    nn_isp_p2_msb_uv_sao:    [0x0, 0xf],            bits : 19_16
*/
#define IME_NN_ISP_PATH2_REGISTER9_OFS 0x0c24
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER9)
REGDEF_BIT(nn_isp_p2_msb_y_sao,        4)
REGDEF_BIT(,        12)
REGDEF_BIT(nn_isp_p2_msb_uv_sao,        4)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER9)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_NN_ISP_PATH2_REGISTER10_OFS 0x0c28
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER10)
REGDEF_BIT(,        31)
REGDEF_BIT(nn_isp_p2_cpu2isp_in_ready,        1)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER10)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_NN_ISP_PATH2_REGISTER11_OFS 0x0c2c
REGDEF_BEGIN(IME_NN_ISP_PATH2_REGISTER11)
REGDEF_BIT(,        31)
REGDEF_BIT(nn_isp_p2_cpu2isp_out_clear,        1)
REGDEF_END(IME_NN_ISP_PATH2_REGISTER11)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_NN_ISP_STATUS_REGISTER0_OFS 0x0c30
REGDEF_BEGIN(IME_NN_ISP_STATUS_REGISTER0)
REGDEF_BIT(ime_nn_isp2xpu_p2_out_ready_status,        1)
REGDEF_BIT(ime_nn_isp2xpu_p2_in_clear_status,        1)
REGDEF_BIT(ime_nn_isp2xpu_p3_out_ready_status,        1)
REGDEF_BIT(ime_nn_isp2xpu_p3_in_clear_status,        1)
REGDEF_END(IME_NN_ISP_STATUS_REGISTER0)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_256_OFS 0x0c34
REGDEF_BEGIN(IME_RESERVED_256)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_256)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_257_OFS 0x0c38
REGDEF_BEGIN(IME_RESERVED_257)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_257)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_RESERVED_258_OFS 0x0c3c
REGDEF_BEGIN(IME_RESERVED_258)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_RESERVED_258)


/*
    nn_isp_p3_ringbuf_height:       [0x0, 0x1fff],         bits : 12_0
    nn_isp_p3_slice_height  :       [0x0, 0x7ff],          bits : 26_16
    nn_isp_p3_fw_hs_en  :           [0x0, 0x1],          bits : 29
*/
#define IME_NN_ISP_PATH3_REGISTER0_OFS 0x0c40
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER0)
REGDEF_BIT(nn_isp_p3_ringbuf_height,        16)
REGDEF_BIT(nn_isp_p3_slice_height,        14)
REGDEF_BIT(,        1)
REGDEF_BIT(nn_isp_p3_fw_hs_en,        1)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER0)


/*
    nn_isp_p3_slice_ovlp   :    [0x0, 0x7f],            bits : 6_0
    nn_isp_p3_outbuf_height:    [0x0, 0x7ff],           bits : 26_16
    nn_isp_p3_outbuf_num   :    [0x0, 0x1],         bits : 31
*/
#define IME_NN_ISP_PATH3_REGISTER1_OFS 0x0c44
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER1)
REGDEF_BIT(nn_isp_p3_slice_ovlp,        7)
REGDEF_BIT(,        9)
REGDEF_BIT(nn_isp_p3_outbuf_height,        14)
REGDEF_BIT(,        1)
REGDEF_BIT(nn_isp_p3_outbuf_num,        1)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER1)


/*
    nn_isp_p3_yuv_ofsi:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_NN_ISP_PATH3_REGISTER2_OFS 0x0c48
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER2)
REGDEF_BIT(,        2)
REGDEF_BIT(nn_isp_p3_yuv_ofsi,        18)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER2)


/*
    nn_isp_p3_y_sai:    [0x0, 0xffffffff],          bits : 31_0
*/
#define IME_NN_ISP_PATH3_REGISTER3_OFS 0x0c4c
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER3)
REGDEF_BIT(nn_isp_p3_y_sai,        32)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER3)


/*
    nn_isp_p3_uv_sai:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_NN_ISP_PATH3_REGISTER4_OFS 0x0c50
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER4)
REGDEF_BIT(nn_isp_p3_uv_sai,        32)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER4)


/*
    nn_isp_p3_msb_y_sai :    [0x0, 0xf],            bits : 3_0
    nn_isp_p3_msb_uv_sai:    [0x0, 0xf],            bits : 19_16
*/
#define IME_NN_ISP_PATH3_REGISTER5_OFS 0x0c54
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER5)
REGDEF_BIT(nn_isp_p3_msb_y_sai,        4)
REGDEF_BIT(,        12)
REGDEF_BIT(nn_isp_p3_msb_uv_sai,        4)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER5)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_NN_ISP_PATH3_REGISTER6_OFS 0x0c58
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER6)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER6)


/*
    nn_isp_p3_mot_map_ofsi:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_NN_ISP_PATH3_REGISTER8_OFS 0x0c5c
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER8)
REGDEF_BIT(,        2)
REGDEF_BIT(nn_isp_p3_mot_map_ofsi,        18)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER8)


/*
    reserved:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_NN_ISP_PATH3_REGISTER7_OFS 0x0c60
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER7)
REGDEF_BIT(reserved,        32)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER7)


/*
    nn_isp_p3_mot_map_sai:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_NN_ISP_PATH3_REGISTER9_OFS 0x0c64
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER9)
REGDEF_BIT(nn_isp_p3_mot_map_sai,        32)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER9)


/*
    reserved                 :    [0x0, 0xf],           bits : 3_0
    nn_isp_p3_mot_map_msb_sai:    [0x0, 0xf],           bits : 19_16
*/
#define IME_NN_ISP_PATH3_REGISTER10_OFS 0x0c68
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER10)
REGDEF_BIT(reserved,        4)
REGDEF_BIT(,        12)
REGDEF_BIT(nn_isp_p3_mot_map_msb_sai,        4)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER10)


/*
    nn_isp_p3_yuv_ofso:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_NN_ISP_PATH3_REGISTER11_OFS 0x0c6c
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER11)
REGDEF_BIT(,        2)
REGDEF_BIT(nn_isp_p3_yuv_ofso,        18)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER11)


/*
    nn_isp_p3_y_sao:    [0x0, 0xffffffff],          bits : 31_0
*/
#define IME_NN_ISP_PATH3_REGISTER12_OFS 0x0c70
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER12)
REGDEF_BIT(nn_isp_p3_y_sao,        32)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER12)


/*
    nn_isp_p3_uv_sao:    [0x0, 0xffffffff],         bits : 31_0
*/
#define IME_NN_ISP_PATH3_REGISTER13_OFS 0x0c74
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER13)
REGDEF_BIT(nn_isp_p3_uv_sao,        32)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER13)


/*
    nn_isp_p3_y_msb_sao :    [0x0, 0xf],            bits : 3_0
    nn_isp_p3_uv_msb_sao:    [0x0, 0xf],            bits : 19_16
*/
#define IME_NN_ISP_PATH3_REGISTER14_OFS 0x0c78
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER14)
REGDEF_BIT(nn_isp_p3_y_msb_sao,        4)
REGDEF_BIT(,        12)
REGDEF_BIT(nn_isp_p3_uv_msb_sao,        4)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER14)


/*
    nn_isp_p3_mot_map_ofso:    [0x0, 0x3ffff],          bits : 19_2
*/
#define IME_NN_ISP_PATH3_REGISTER17_OFS 0x0c7c
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER17)
REGDEF_BIT(,        2)
REGDEF_BIT(nn_isp_p3_mot_map_ofso,        18)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER17)


/*
    nn_isp_p3_mot_map_sao:    [0x0, 0xffffffff],            bits : 31_0
*/
#define IME_NN_ISP_PATH3_REGISTER18_OFS 0x0c80
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER18)
REGDEF_BIT(nn_isp_p3_mot_map_sao,        32)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER18)


/*
    nn_isp_p3_mot_map_msb_sao:    [0x0, 0xf],           bits : 3_0
*/
#define IME_NN_ISP_PATH3_REGISTER19_OFS 0x0c84
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER19)
REGDEF_BIT(nn_isp_p3_mot_map_msb_sao,        4)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER19)

/*
    nn_isp_p3_cpu2isp_in_ready:    [0x0, 0x1],           bits : 31
*/
#define IME_NN_ISP_PATH3_REGISTER20_OFS 0x0c88
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER20)
REGDEF_BIT(,        31)
REGDEF_BIT(nn_isp_p3_cpu2isp_in_ready,        1)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER20)

/*
    nn_isp_p3_cpu2isp_out_clear:    [0x0, 0x1],           bits : 31
*/
#define IME_NN_ISP_PATH3_REGISTER21_OFS 0x0c8c
REGDEF_BEGIN(IME_NN_ISP_PATH3_REGISTER21)
REGDEF_BIT(,        31)
REGDEF_BIT(nn_isp_p3_cpu2isp_out_clear,        1)
REGDEF_END(IME_NN_ISP_PATH3_REGISTER21)




//-----------------------------------------------------------------



typedef struct _NT98538_IME_ENG_REG_STRUCT_ {

	union {
		struct {
			unsigned ime_rst                   : 1;     // bits : 0
			unsigned ime_start                 : 1;     // bits : 1
			unsigned ime_start_load            : 1;     // bits : 2
			unsigned ime_frmend_load           : 1;     // bits : 3
			unsigned ime_drt_start_load        : 1;     // bits : 4
			unsigned ime_gbl_load_en           : 1;     // bits : 5
			unsigned                           : 21;
			unsigned ime_dmach_dis             : 1;     // bits : 27
			unsigned ime_ll_fire               : 1;     // bits : 28
		} bit;
		UINT32 word;
	} reg_0; // 0x0000

	union {
		struct {
			unsigned ime_src                        : 1;        // bits : 0
			unsigned ime_dir_ctrl                   : 1;        // bits : 1
			unsigned ime_p0_en                      : 1;        // bits : 2
			unsigned ime_p1_en                      : 1;        // bits : 3
			unsigned ime_p2_en                      : 1;        // bits : 4
			unsigned ime_p3_en                      : 1;        // bits : 5
			unsigned                                : 4;
			unsigned ime_lca_en                     : 1;        // bits : 10
			unsigned                                : 2;
			unsigned ime_dbcs_en                    : 1;        // bits : 13
			unsigned                                : 1;
			unsigned ime_fisheye_mask_en            : 1;        // bits : 15
			unsigned ime_nn_isp_p2_3dnr_en          : 1;        // bits : 16
			unsigned ime_nn_isp_p3_2dnr_en          : 1;        // bits : 17
			unsigned                                : 6;
			unsigned ime_post_shp_en                : 1;        // bits : 24
			unsigned                                : 2;
			unsigned ime_3dnr_en                    : 1;        // bits : 27
			unsigned ime_3dnr_ref_out_sel           : 1;        // bits : 28
			unsigned ime_3dnr_ref_in_dec_en         : 1;        // bits : 29
			unsigned ime_3dnr_ref_out_en            : 1;        // bits : 30
			unsigned ime_3dnr_ref_out_enc_en        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_1; // 0x0004

	union {
		struct {
			unsigned ime_pm0_en                     : 1;        // bits : 0
			unsigned ime_pm1_en                     : 1;        // bits : 1
			unsigned ime_pm2_en                     : 1;        // bits : 2
			unsigned ime_pm3_en                     : 1;        // bits : 3
			unsigned ime_pm4_en                     : 1;        // bits : 4
			unsigned ime_pm5_en                     : 1;        // bits : 5
			unsigned ime_pm6_en                     : 1;        // bits : 6
			unsigned ime_pm7_en                     : 1;        // bits : 7
			unsigned ime_low_dly_en                 : 1;        // bits : 8
			unsigned ime_low_dly_sel                : 3;        // bits : 11_9
			unsigned ime_p0_flip_en                 : 1;        // bits : 12
			unsigned ime_p1_flip_en                 : 1;        // bits : 13
			unsigned ime_p2_flip_en                 : 1;        // bits : 14
			unsigned ime_p3_flip_en                 : 1;        // bits : 15
			unsigned ime_3dnr_inref_flip_en         : 1;        // bits : 16
			unsigned ime_3dnr_outref_flip_en        : 1;        // bits : 17
			unsigned ime_3dnr_ms_roi_flip_en        : 1;        // bits : 18
			unsigned ime_pm_pxl_subout_en           : 1;        // bits : 19
			unsigned ime_ycc_cvt_en                 : 1;        // bits : 20
			unsigned ime_ycc_cvt_sel                : 1;        // bits : 21
			unsigned ime_p0_enc_en                  : 1;        // bits : 22
		} bit;
		UINT32 word;
	} reg_2; // 0x0008

	union {
		struct {
			unsigned ime_pxl_subout_dram_out_single_en         : 1;     // bits : 0
			unsigned ime_3dnr_refout_dram_out_single_en        : 1;     // bits : 1
			unsigned ime_3dnr_ms_dram_out_single_en            : 1;     // bits : 2
			unsigned ime_3dnr_ms_roi_dram_out_single_en        : 1;     // bits : 3
			unsigned ime_3dnr_mv_dram_out_single_en            : 1;     // bits : 4
			unsigned ime_3dnr_sta_dram_out_single_en           : 1;     // bits : 5
			unsigned ime_out_p0_dram_out_single_en             : 1;     // bits : 6
			unsigned ime_out_p1_dram_out_single_en             : 1;     // bits : 7
			unsigned ime_out_p2_dram_out_single_en             : 1;     // bits : 8
			unsigned ime_out_p3_dram_out_single_en             : 1;     // bits : 9
			unsigned ime_3dnr_fc_dram_out_single_en            : 1;     // bits : 10
			unsigned                                           : 20;
			unsigned ime_dram_out_mode                         : 1;     // bits : 31
		} bit;
		UINT32 word;
	} reg_3; // 0x000c

	union {
		struct {
			unsigned ime_ll_terminate          : 1;     // bits : 0
			unsigned                           : 3;
			unsigned ime_dmach_idle            : 1;     // bits : 4
			unsigned                           : 2;
			unsigned ime_sline_num             : 9;     // bits : 15_7
			unsigned ime_sline_path_sel        : 2;     // bits : 17_16
		} bit;
		UINT32 word;
	} reg_4; // 0x0010

	union {
		struct {
			unsigned                        : 2;
			unsigned ime_dram_ll_sai        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} reg_5; // 0x0014

	union {
		struct {
			unsigned ime_inte_ll_end                         : 1;       // bits : 0
			unsigned ime_inte_ll_err                         : 1;       // bits : 1
			unsigned ime_inte_ll_red_late                    : 1;       // bits : 2
			unsigned ime_inte_ll_job_end                     : 1;       // bits : 3
			unsigned ime_inte_in_bp0                         : 1;       // bits : 4
			unsigned ime_inte_in_bp1                         : 1;       // bits : 5
			unsigned ime_inte_in_bp2                         : 1;       // bits : 6
			unsigned ime_inte_3dnr_slice_end                 : 1;       // bits : 7
			unsigned ime_inte_3dnr_mot_end                   : 1;       // bits : 8
			unsigned ime_inte_3dnr_mv_end                    : 1;       // bits : 9
			unsigned ime_inte_3dnr_statsitic_end             : 1;       // bits : 10
			unsigned                                         : 1;
			unsigned ime_inte_3dnr_ref_out_enc_ovfl          : 1;       // bits : 12
			unsigned ime_inte_3dnr_ref_in_dec_err            : 1;       // bits : 13
			unsigned ime_inte_frm_err                        : 1;       // bits : 14
			unsigned ime_inte_sline                          : 1;       // bits : 15
			unsigned ime_inte_p0_out_enc_ovfl                : 1;       // bits : 16
			unsigned                                         : 3;
			unsigned ime_inte_in_dec_err                     : 1;       // bits : 20
			unsigned ime_inte_nn_isp2cpu_p2_out_ready        : 1;       // bits : 21
			unsigned ime_inte_nn_isp2cpu_p2_in_clear         : 1;       // bits : 22
			unsigned ime_inte_nn_isp2cpu_p3_out_ready        : 1;       // bits : 23
			unsigned ime_inte_nn_isp2cpu_p3_in_clear         : 1;       // bits : 24
			unsigned                                         : 4;
			unsigned ime_inte_frm_start                      : 1;       // bits : 29
			unsigned ime_inte_strp_end                       : 1;       // bits : 30
			unsigned ime_inte_frm_end                        : 1;       // bits : 31
		} bit;
		UINT32 word;
	} reg_6; // 0x0018

	union {
		struct {
			unsigned ime_ints_ll_end                         : 1;       // bits : 0
			unsigned ime_ints_ll_err                         : 1;       // bits : 1
			unsigned ime_ints_ll_red_late                    : 1;       // bits : 2
			unsigned ime_ints_ll_job_end                     : 1;       // bits : 3
			unsigned ime_ints_in_bp0                         : 1;       // bits : 4
			unsigned ime_ints_in_bp1                         : 1;       // bits : 5
			unsigned ime_ints_in_bp2                         : 1;       // bits : 6
			unsigned ime_ints_3dnr_slice_end                 : 1;       // bits : 7
			unsigned ime_ints_3dnr_mot_end                   : 1;       // bits : 8
			unsigned ime_ints_3dnr_mv_end                    : 1;       // bits : 9
			unsigned ime_ints_3dnr_statsitic_end             : 1;       // bits : 10
			unsigned                                         : 1;
			unsigned ime_ints_3dnr_ref_out_enc_ovfl          : 1;       // bits : 12
			unsigned ime_ints_3dnr_ref_in_dec_err            : 1;       // bits : 13
			unsigned ime_ints_frm_err                        : 1;       // bits : 14
			unsigned ime_ints_sline                          : 1;       // bits : 15
			unsigned ime_ints_p0_out_enc_ovfl                : 1;       // bits : 16
			unsigned                                         : 3;
			unsigned ime_ints_in_dec_err                     : 1;       // bits : 20
			unsigned ime_ints_nn_isp2cpu_p2_out_ready        : 1;       // bits : 21
			unsigned ime_ints_nn_isp2cpu_p2_in_clear         : 1;       // bits : 22
			unsigned ime_ints_nn_isp2cpu_p3_out_ready        : 1;       // bits : 23
			unsigned ime_ints_nn_isp2cpu_p3_in_clear         : 1;       // bits : 24
			unsigned                                         : 4;
			unsigned ime_ints_frm_start                      : 1;       // bits : 29
			unsigned ime_ints_strp_end                       : 1;       // bits : 30
			unsigned ime_ints_frm_end                        : 1;       // bits : 31
		} bit;
		UINT32 word;
	} reg_7; // 0x001c

	union {
		struct {
			unsigned                      : 2;
			unsigned ime_in_h_size        : 14;     // bits : 15_2
			unsigned                      : 2;
			unsigned ime_in_v_size        : 14;     // bits : 31_18
		} bit;
		UINT32 word;
	} reg_8; // 0x0020

	union {
		struct {
			unsigned ime_imat                : 2;       // bits : 1_0
			unsigned                         : 2;
			unsigned ime_st_hovlp_sel        : 2;       // bits : 5_4
			unsigned ime_st_prt_sel          : 2;       // bits : 7_6
			unsigned ime_st_size_mode        : 1;       // bits : 8
			unsigned                         : 7;
			unsigned ime_st_hovlp_msb        : 2;       // bits : 17_16
			unsigned ime_st_prt_msb          : 2;       // bits : 19_18
		} bit;
		UINT32 word;
	} reg_9; // 0x0024

	union {
		struct {
			unsigned ime_st_hn        : 11;     // bits : 10_0
			unsigned                  : 1;
			unsigned ime_st_hl        : 11;     // bits : 22_12
			unsigned                  : 1;
			unsigned ime_st_hm        : 8;      // bits : 31_24
		} bit;
		UINT32 word;
	} reg_10; // 0x0028

	union {
		struct {
			unsigned ime_st_vl           : 16;      // bits : 15_0
			unsigned ime_st_prt          : 8;       // bits : 23_16
			unsigned                     : 3;
			unsigned ime_st_hovlp        : 5;       // bits : 31_27
		} bit;
		UINT32 word;
	} reg_11; // 0x002c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_12; // 0x0030

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_13; // 0x0034

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_14; // 0x0038

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_15; // 0x003c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_16; // 0x0040

	union {
		struct {
			unsigned                        : 2;
			unsigned ime_y_dram_ofsi        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_17; // 0x0044

	union {
		struct {
			unsigned                         : 2;
			unsigned ime_uv_dram_ofsi        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_18; // 0x0048

	union {
		struct {
			unsigned                       : 2;
			unsigned ime_y_dram_sai        : 30;        // bits : 31_2
		} bit;
		UINT32 word;
	} reg_19; // 0x004c

	union {
		struct {
			unsigned                       : 2;
			unsigned ime_u_dram_sai        : 30;        // bits : 31_2
		} bit;
		UINT32 word;
	} reg_20; // 0x0050

	union {
		struct {
			unsigned                       : 2;
			unsigned ime_v_dram_sai        : 30;        // bits : 31_2
		} bit;
		UINT32 word;
	} reg_21; // 0x0054

	union {
		struct {
			unsigned ime_stripe_size0        : 10;      // bits : 9_0
			unsigned                         : 1;
			unsigned ime_stripe_size1        : 10;      // bits : 20_11
			unsigned                         : 1;
			unsigned ime_stripe_size2        : 10;      // bits : 31_22
		} bit;
		UINT32 word;
	} reg_22; // 0x0058

	union {
		struct {
			unsigned ime_stripe_size3        : 10;      // bits : 9_0
			unsigned                         : 1;
			unsigned ime_stripe_size4        : 10;      // bits : 20_11
			unsigned                         : 1;
			unsigned ime_stripe_size5        : 10;      // bits : 31_22
		} bit;
		UINT32 word;
	} reg_23; // 0x005c

	union {
		struct {
			unsigned ime_stripe_size6            : 10;      // bits : 9_0
			unsigned                             : 1;
			unsigned ime_stripe_size7            : 10;      // bits : 20_11
			unsigned                             : 3;
			unsigned ime_stripe_msb_size0        : 1;       // bits : 24
			unsigned ime_stripe_msb_size1        : 1;       // bits : 25
			unsigned ime_stripe_msb_size2        : 1;       // bits : 26
			unsigned ime_stripe_msb_size3        : 1;       // bits : 27
			unsigned ime_stripe_msb_size4        : 1;       // bits : 28
			unsigned ime_stripe_msb_size5        : 1;       // bits : 29
			unsigned ime_stripe_msb_size6        : 1;       // bits : 30
			unsigned ime_stripe_msb_size7        : 1;       // bits : 31
		} bit;
		UINT32 word;
	} reg_24; // 0x0060

	union {
		struct {
			unsigned ime_dend_sts_en        : 1;        // bits : 0
			unsigned                        : 3;
			unsigned ime_dend_wbit          : 5;        // bits : 8_4
		} bit;
		UINT32 word;
	} reg_25; // 0x0064

	union {
		struct {
			unsigned ime_dend_status        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_26; // 0x0068

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_27; // 0x006c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_28; // 0x0070

	union {
		struct {
			unsigned ime_dram_ll_msb_sai        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_29; // 0x0074

	union {
		struct {
			unsigned ime_y_dram_msb_sai        : 4;     // bits : 3_0
		} bit;
		UINT32 word;
	} reg_30; // 0x0078

	union {
		struct {
			unsigned ime_u_dram_msb_sai        : 4;     // bits : 3_0
		} bit;
		UINT32 word;
	} reg_31; // 0x007c

	union {
		struct {
			unsigned ime_v_dram_msb_sai        : 4;     // bits : 3_0
		} bit;
		UINT32 word;
	} reg_32; // 0x0080

	union {
		struct {
			unsigned                            : 1;
			unsigned ime_p0_out_type            : 1;        // bits : 1
			unsigned ime_p0_scl_method          : 2;        // bits : 3_2
			unsigned ime_p0_out_en              : 1;        // bits : 4
			unsigned ime_p0_sprt_out_en         : 1;        // bits : 5
			unsigned                            : 2;
			unsigned ime_p0_omat                : 3;        // bits : 10_8
			unsigned                            : 1;
			unsigned ime_p0_scl_enh_fact        : 8;        // bits : 19_12
			unsigned ime_p0_scl_enh_bit         : 4;        // bits : 23_20
		} bit;
		UINT32 word;
	} reg_33; // 0x0084

	union {
		struct {
			unsigned ime_p0_h_ud              : 1;      // bits : 0
			unsigned ime_p0_v_ud              : 1;      // bits : 1
			unsigned ime_p0_h_dnrate          : 5;      // bits : 6_2
			unsigned ime_p0_v_dnrate          : 5;      // bits : 11_7
			unsigned                          : 3;
			unsigned ime_p0_h_filtmode        : 1;      // bits : 15
			unsigned ime_p0_h_filtcoef        : 6;      // bits : 21_16
			unsigned ime_p0_v_filtmode        : 1;      // bits : 22
			unsigned ime_p0_v_filtcoef        : 6;      // bits : 28_23
		} bit;
		UINT32 word;
	} reg_34; // 0x0088

	union {
		struct {
			unsigned ime_p0_h_sfact        : 16;        // bits : 15_0
			unsigned ime_p0_v_sfact        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} reg_35; // 0x008c

	union {
		struct {
			unsigned ime_p0_h_scl_init_ofs        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_36; // 0x0090

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_37; // 0x0094

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_38; // 0x0098

	union {
		struct {
			unsigned ime_p0_h_scl_size        : 16;     // bits : 15_0
			unsigned ime_p0_v_scl_size        : 16;     // bits : 31_16
		} bit;
		UINT32 word;
	} reg_39; // 0x009c

	union {
		struct {
			unsigned ime_p0_cropout_x        : 16;      // bits : 15_0
			unsigned ime_p0_cropout_y        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} reg_40; // 0x00a0

	union {
		struct {
			unsigned ime_p0_h_osize        : 16;        // bits : 15_0
			unsigned ime_p0_v_osize        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} reg_41; // 0x00a4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_42; // 0x00a8

	union {
		struct {
			unsigned ime_p0_y_clamp_min         : 8;        // bits : 7_0
			unsigned ime_p0_y_clamp_max         : 8;        // bits : 15_8
			unsigned ime_p0_uv_clamp_min        : 8;        // bits : 23_16
			unsigned ime_p0_uv_clamp_max        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_43; // 0x00ac

	union {
		struct {
			unsigned                           : 2;
			unsigned ime_p0_y_dram_ofso        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_44; // 0x00b0

	union {
		struct {
			unsigned                            : 2;
			unsigned ime_p0_uv_dram_ofso        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_45; // 0x00b4

	union {
		struct {
			unsigned ime_p0_y_dram_sao        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_46; // 0x00b8

	union {
		struct {
			unsigned ime_p0_u_dram_sao        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_47; // 0x00bc

	union {
		struct {
			unsigned ime_p0_v_dram_sao        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_48; // 0x00c0

	union {
		struct {
			unsigned ime_p0_y_dram_msb_sao        : 4;      // bits : 3_0
		} bit;
		UINT32 word;
	} reg_49; // 0x00c4

	union {
		struct {
			unsigned ime_p0_u_dram_msb_sao        : 4;      // bits : 3_0
		} bit;
		UINT32 word;
	} reg_50; // 0x00c8

	union {
		struct {
			unsigned ime_p0_v_dram_msb_sao        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_51; // 0x00cc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_52; // 0x00d0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_53; // 0x00d4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_54; // 0x00d8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_55; // 0x00dc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_56; // 0x00e0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_57; // 0x00e4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_58; // 0x00e8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_59; // 0x00ec

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_60; // 0x00f0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_61; // 0x00f4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_62; // 0x00f8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_63; // 0x00fc

	union {
		struct {
			unsigned                            : 1;
			unsigned ime_p1_out_type            : 1;        // bits : 1
			unsigned ime_p1_scl_method          : 2;        // bits : 3_2
			unsigned ime_p1_out_en              : 1;        // bits : 4
			unsigned ime_p1_sprt_out_en         : 1;        // bits : 5
			unsigned                            : 2;
			unsigned ime_p1_omat                : 3;        // bits : 10_8
			unsigned                            : 1;
			unsigned ime_p1_scl_enh_fact        : 8;        // bits : 19_12
			unsigned ime_p1_scl_enh_bit         : 4;        // bits : 23_20
		} bit;
		UINT32 word;
	} reg_64; // 0x0100

	union {
		struct {
			unsigned ime_p1_h_ud              : 1;      // bits : 0
			unsigned ime_p1_v_ud              : 1;      // bits : 1
			unsigned ime_p1_h_dnrate          : 5;      // bits : 6_2
			unsigned ime_p1_v_dnrate          : 5;      // bits : 11_7
			unsigned                          : 3;
			unsigned ime_p1_h_filtmode        : 1;      // bits : 15
			unsigned ime_p1_h_filtcoef        : 6;      // bits : 21_16
			unsigned ime_p1_v_filtmode        : 1;      // bits : 22
			unsigned ime_p1_v_filtcoef        : 6;      // bits : 28_23
		} bit;
		UINT32 word;
	} reg_65; // 0x0104

	union {
		struct {
			unsigned ime_p1_h_sfact        : 16;        // bits : 15_0
			unsigned ime_p1_v_sfact        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} reg_66; // 0x0108

	union {
		struct {
			unsigned ime_p1_isd_h_base        : 13;     // bits : 12_0
			unsigned                          : 3;
			unsigned ime_p1_isd_v_base        : 13;     // bits : 28_16
		} bit;
		UINT32 word;
	} reg_67; // 0x010c

	union {
		struct {
			unsigned ime_p1_isd_h_sfact0        : 13;       // bits : 12_0
			unsigned                            : 3;
			unsigned ime_p1_isd_v_sfact0        : 13;       // bits : 28_16
		} bit;
		UINT32 word;
	} reg_68; // 0x0110

	union {
		struct {
			unsigned ime_p1_isd_h_sfact1        : 13;       // bits : 12_0
			unsigned                            : 3;
			unsigned ime_p1_isd_v_sfact1        : 13;       // bits : 28_16
		} bit;
		UINT32 word;
	} reg_69; // 0x0114

	union {
		struct {
			unsigned ime_p1_isd_h_sfact2        : 13;       // bits : 12_0
			unsigned                            : 3;
			unsigned ime_p1_isd_v_sfact2        : 13;       // bits : 28_16
		} bit;
		UINT32 word;
	} reg_70; // 0x0118

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_71; // 0x011c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_72; // 0x0120

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_73; // 0x0124

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_74; // 0x0128

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_75; // 0x012c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_76; // 0x0130

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_77; // 0x0134

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_78; // 0x0138

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_79; // 0x013c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_80; // 0x0140

	union {
		struct {
			unsigned ime_p1_h_scl_size        : 16;     // bits : 15_0
			unsigned ime_p1_v_scl_size        : 16;     // bits : 31_16
		} bit;
		UINT32 word;
	} reg_81; // 0x0144

	union {
		struct {
			unsigned ime_p1_cropout_x        : 16;      // bits : 15_0
			unsigned ime_p1_cropout_y        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} reg_82; // 0x0148

	union {
		struct {
			unsigned ime_p1_h_osize        : 16;        // bits : 15_0
			unsigned ime_p1_v_osize        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} reg_83; // 0x014c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_84; // 0x0150

	union {
		struct {
			unsigned ime_p1_y_clamp_min         : 8;        // bits : 7_0
			unsigned ime_p1_y_clamp_max         : 8;        // bits : 15_8
			unsigned ime_p1_uv_clamp_min        : 8;        // bits : 23_16
			unsigned ime_p1_uv_clamp_max        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_85; // 0x0154

	union {
		struct {
			unsigned                           : 2;
			unsigned ime_p1_y_dram_ofso        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_86; // 0x0158

	union {
		struct {
			unsigned                            : 2;
			unsigned ime_p1_uv_dram_ofso        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_87; // 0x015c

	union {
		struct {
			unsigned ime_p1_y_dram_sao        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_88; // 0x0160

	union {
		struct {
			unsigned ime_p1_uv_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_89; // 0x0164

	union {
		struct {
			unsigned ime_p1_y_dram_msb_sao        : 4;      // bits : 3_0
		} bit;
		UINT32 word;
	} reg_90; // 0x0168

	union {
		struct {
			unsigned ime_p1_uv_dram_msb_sao        : 4;     // bits : 3_0
		} bit;
		UINT32 word;
	} reg_91; // 0x016c

	union {
		struct {
			unsigned ime_p1_h_scl_init_ofs        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_92; // 0x0170

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_93; // 0x0174

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_94; // 0x0178

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_95; // 0x017c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_96; // 0x0180

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_97; // 0x0184

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_98; // 0x0188

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_99; // 0x018c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_100; // 0x0190

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_101; // 0x0194

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_102; // 0x0198

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_103; // 0x019c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_104; // 0x01a0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_105; // 0x01a4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_106; // 0x01a8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_107; // 0x01ac

	union {
		struct {
			unsigned                            : 1;
			unsigned ime_p2_out_type            : 1;        // bits : 1
			unsigned ime_p2_scl_method          : 2;        // bits : 3_2
			unsigned ime_p2_out_en              : 1;        // bits : 4
			unsigned ime_p2_sprt_out_en         : 1;        // bits : 5
			unsigned                            : 2;
			unsigned ime_p2_omat                : 3;        // bits : 10_8
			unsigned                            : 1;
			unsigned ime_p2_scl_enh_fact        : 8;        // bits : 19_12
			unsigned ime_p2_scl_enh_bit         : 4;        // bits : 23_20
		} bit;
		UINT32 word;
	} reg_108; // 0x01b0

	union {
		struct {
			unsigned ime_p2_h_ud              : 1;      // bits : 0
			unsigned ime_p2_v_ud              : 1;      // bits : 1
			unsigned ime_p2_h_dnrate          : 5;      // bits : 6_2
			unsigned ime_p2_v_dnrate          : 5;      // bits : 11_7
			unsigned                          : 3;
			unsigned ime_p2_h_filtmode        : 1;      // bits : 15
			unsigned ime_p2_h_filtcoef        : 6;      // bits : 21_16
			unsigned ime_p2_v_filtmode        : 1;      // bits : 22
			unsigned ime_p2_v_filtcoef        : 6;      // bits : 28_23
		} bit;
		UINT32 word;
	} reg_109; // 0x01b4

	union {
		struct {
			unsigned ime_p2_h_sfact        : 16;        // bits : 15_0
			unsigned ime_p2_v_sfact        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} reg_110; // 0x01b8

	union {
		struct {
			unsigned ime_p2_isd_h_base        : 13;     // bits : 12_0
			unsigned                          : 3;
			unsigned ime_p2_isd_v_base        : 13;     // bits : 28_16
		} bit;
		UINT32 word;
	} reg_111; // 0x01bc

	union {
		struct {
			unsigned ime_p2_isd_h_sfact0        : 13;       // bits : 12_0
			unsigned                            : 3;
			unsigned ime_p2_isd_v_sfact0        : 13;       // bits : 28_16
		} bit;
		UINT32 word;
	} reg_112; // 0x01c0

	union {
		struct {
			unsigned ime_p2_isd_h_sfact1        : 13;       // bits : 12_0
			unsigned                            : 3;
			unsigned ime_p2_isd_v_sfact1        : 13;       // bits : 28_16
		} bit;
		UINT32 word;
	} reg_113; // 0x01c4

	union {
		struct {
			unsigned ime_p2_isd_h_sfact2        : 13;       // bits : 12_0
			unsigned                            : 3;
			unsigned ime_p2_isd_v_sfact2        : 13;       // bits : 28_16
		} bit;
		UINT32 word;
	} reg_114; // 0x01c8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_115; // 0x01cc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_116; // 0x01d0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_117; // 0x01d4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_118; // 0x01d8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_119; // 0x01dc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_120; // 0x01e0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_121; // 0x01e4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_122; // 0x01e8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_123; // 0x01ec

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_124; // 0x01f0

	union {
		struct {
			unsigned ime_p2_h_scl_size        : 16;     // bits : 15_0
			unsigned ime_p2_v_scl_size        : 16;     // bits : 31_16
		} bit;
		UINT32 word;
	} reg_125; // 0x01f4

	union {
		struct {
			unsigned ime_p2_cropout_x        : 16;      // bits : 15_0
			unsigned ime_p2_cropout_y        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} reg_126; // 0x01f8

	union {
		struct {
			unsigned ime_p2_h_osize        : 16;        // bits : 15_0
			unsigned ime_p2_v_osize        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} reg_127; // 0x01fc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_128; // 0x0200

	union {
		struct {
			unsigned ime_p2_y_clamp_min         : 8;        // bits : 7_0
			unsigned ime_p2_y_clamp_max         : 8;        // bits : 15_8
			unsigned ime_p2_uv_clamp_min        : 8;        // bits : 23_16
			unsigned ime_p2_uv_clamp_max        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_129; // 0x0204

	union {
		struct {
			unsigned                           : 2;
			unsigned ime_p2_y_dram_ofso        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_130; // 0x0208

	union {
		struct {
			unsigned                            : 2;
			unsigned ime_p2_uv_dram_ofso        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_131; // 0x020c

	union {
		struct {
			unsigned ime_p2_y_dram_sao        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_132; // 0x0210

	union {
		struct {
			unsigned ime_p2_uv_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_133; // 0x0214

	union {
		struct {
			unsigned ime_p2_y_dram_msb_sao        : 4;      // bits : 3_0
		} bit;
		UINT32 word;
	} reg_134; // 0x0218

	union {
		struct {
			unsigned ime_p2_uv_dram_msb_sao        : 4;     // bits : 3_0
		} bit;
		UINT32 word;
	} reg_135; // 0x021c

	union {
		struct {
			unsigned ime_p2_h_scl_init_ofs        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_136; // 0x0220

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_137; // 0x0224

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_138; // 0x0228

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_139; // 0x022c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_140; // 0x0230

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_141; // 0x0234

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_142; // 0x0238

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_143; // 0x023c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_144; // 0x0240

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_145; // 0x0244

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_146; // 0x0248

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_147; // 0x024c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_148; // 0x0250

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_149; // 0x0254

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_150; // 0x0258

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_151; // 0x025c

	union {
		struct {
			unsigned                            : 1;
			unsigned ime_p3_out_type            : 1;        // bits : 1
			unsigned ime_p3_scl_method          : 2;        // bits : 3_2
			unsigned ime_p3_out_en              : 1;        // bits : 4
			unsigned ime_p3_sprt_out_en         : 1;        // bits : 5
			unsigned                            : 2;
			unsigned ime_p3_omat                : 3;        // bits : 10_8
			unsigned                            : 1;
			unsigned ime_p3_scl_enh_fact        : 8;        // bits : 19_12
			unsigned ime_p3_scl_enh_bit         : 4;        // bits : 23_20
		} bit;
		UINT32 word;
	} reg_152; // 0x0260

	union {
		struct {
			unsigned ime_p3_h_ud              : 1;      // bits : 0
			unsigned ime_p3_v_ud              : 1;      // bits : 1
			unsigned ime_p3_h_dnrate          : 5;      // bits : 6_2
			unsigned ime_p3_v_dnrate          : 5;      // bits : 11_7
			unsigned                          : 3;
			unsigned ime_p3_h_filtmode        : 1;      // bits : 15
			unsigned ime_p3_h_filtcoef        : 6;      // bits : 21_16
			unsigned ime_p3_v_filtmode        : 1;      // bits : 22
			unsigned ime_p3_v_filtcoef        : 6;      // bits : 28_23
		} bit;
		UINT32 word;
	} reg_153; // 0x0264

	union {
		struct {
			unsigned ime_p3_h_sfact        : 16;        // bits : 15_0
			unsigned ime_p3_v_sfact        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} reg_154; // 0x0268

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_155; // 0x026c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_156; // 0x0270

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_157; // 0x0274

	union {
		struct {
			unsigned ime_p3_coef_mode         : 1;      // bits : 0
			unsigned                          : 15;
			unsigned ime_p3_h_filt_adj        : 6;      // bits : 21_16
			unsigned                          : 2;
			unsigned ime_p3_v_filt_adj        : 6;      // bits : 29_24
		} bit;
		UINT32 word;
	} reg_158; // 0x0278

	union {
		struct {
			unsigned ime_p3_isd2_ucoef0        : 8;     // bits : 7_0
			unsigned ime_p3_isd2_ucoef1        : 8;     // bits : 15_8
			unsigned ime_p3_isd2_ucoef2        : 8;     // bits : 23_16
			unsigned ime_p3_isd2_ucoef3        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_159; // 0x027c

	union {
		struct {
			unsigned ime_p3_isd2_ucoef4        : 8;     // bits : 7_0
			unsigned ime_p3_isd2_ucoef5        : 8;     // bits : 15_8
			unsigned ime_p3_isd2_ucoef6        : 8;     // bits : 23_16
			unsigned ime_p3_isd2_ucoef7        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_160; // 0x0280

	union {
		struct {
			unsigned ime_p3_isd2_ucoef8         : 8;        // bits : 7_0
			unsigned ime_p3_isd2_ucoef9         : 8;        // bits : 15_8
			unsigned ime_p3_isd2_ucoef10        : 8;        // bits : 23_16
			unsigned ime_p3_isd2_ucoef11        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_161; // 0x0284

	union {
		struct {
			unsigned ime_p3_isd2_ucoef12        : 8;        // bits : 7_0
			unsigned ime_p3_isd2_ucoef13        : 8;        // bits : 15_8
			unsigned ime_p3_isd2_ucoef14        : 8;        // bits : 23_16
			unsigned ime_p3_isd2_ucoef15        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_162; // 0x0288

	union {
		struct {
			unsigned ime_p3_isd2_ucoef16        : 8;        // bits : 7_0
			unsigned ime_p3_isd2_ucoef17        : 8;        // bits : 15_8
			unsigned ime_p3_isd2_ucoef18        : 8;        // bits : 23_16
			unsigned ime_p3_isd2_ucoef19        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_163; // 0x028c

	union {
		struct {
			unsigned ime_p3_isd2_ucoef20        : 8;        // bits : 7_0
			unsigned ime_p3_isd2_ucoef21        : 8;        // bits : 15_8
			unsigned ime_p3_isd2_ucoef22        : 8;        // bits : 23_16
			unsigned ime_p3_isd2_ucoef23        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_164; // 0x0290

	union {
		struct {
			unsigned ime_p3_isd2_ucoef24        : 8;        // bits : 7_0
			unsigned ime_p3_isd2_ucoef25        : 8;        // bits : 15_8
			unsigned ime_p3_isd2_ucoef26        : 8;        // bits : 23_16
			unsigned ime_p3_isd2_ucoef27        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_165; // 0x0294

	union {
		struct {
			unsigned ime_p3_isd2_ucoef28        : 8;        // bits : 7_0
			unsigned ime_p3_isd2_ucoef29        : 8;        // bits : 15_8
			unsigned ime_p3_isd2_ucoef30        : 8;        // bits : 23_16
			unsigned ime_p3_isd2_ucoef31        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_166; // 0x0298

	union {
		struct {
			unsigned ime_p3_isd2_h_norm        : 20;        // bits : 19_0
		} bit;
		UINT32 word;
	} reg_167; // 0x029c

	union {
		struct {
			unsigned ime_p3_isd2_v_norm        : 20;        // bits : 19_0
		} bit;
		UINT32 word;
	} reg_168; // 0x02a0

	union {
		struct {
			unsigned ime_p3_h_scl_size        : 16;     // bits : 15_0
			unsigned ime_p3_v_scl_size        : 16;     // bits : 31_16
		} bit;
		UINT32 word;
	} reg_169; // 0x02a4

	union {
		struct {
			unsigned ime_p3_cropout_x        : 16;      // bits : 15_0
			unsigned ime_p3_cropout_y        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} reg_170; // 0x02a8

	union {
		struct {
			unsigned ime_p3_h_osize        : 16;        // bits : 15_0
			unsigned ime_p3_v_osize        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} reg_171; // 0x02ac

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_172; // 0x02b0

	union {
		struct {
			unsigned ime_p3_y_clamp_min         : 8;        // bits : 7_0
			unsigned ime_p3_y_clamp_max         : 8;        // bits : 15_8
			unsigned ime_p3_uv_clamp_min        : 8;        // bits : 23_16
			unsigned ime_p3_uv_clamp_max        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_173; // 0x02b4

	union {
		struct {
			unsigned                           : 2;
			unsigned ime_p3_y_dram_ofso        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_174; // 0x02b8

	union {
		struct {
			unsigned                            : 2;
			unsigned ime_p3_uv_dram_ofso        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_175; // 0x02bc

	union {
		struct {
			unsigned ime_p3_y_dram_sao        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_176; // 0x02c0

	union {
		struct {
			unsigned ime_p3_uv_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_177; // 0x02c4

	union {
		struct {
			unsigned ime_p3_y_dram_msb_sao        : 4;      // bits : 3_0
		} bit;
		UINT32 word;
	} reg_178; // 0x02c8

	union {
		struct {
			unsigned ime_p3_uv_dram_msb_sao        : 4;     // bits : 3_0
		} bit;
		UINT32 word;
	} reg_179; // 0x02cc

	union {
		struct {
			unsigned ime_p3_h_scl_init_ofs        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_180; // 0x02d0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_181; // 0x02d4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_182; // 0x02d8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_183; // 0x02dc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_184; // 0x02e0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_185; // 0x02e4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_186; // 0x02e8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_187; // 0x02ec

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_188; // 0x02f0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_189; // 0x02f4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_190; // 0x02f8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_191; // 0x02fc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_192; // 0x0300

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_193; // 0x0304

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_194; // 0x0308

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_195; // 0x030c

	union {
		struct {
			unsigned lca_subin_h_size        : 16;      // bits : 15_0
			unsigned lca_subin_v_size        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} reg_196; // 0x0310

	union {
		struct {
			unsigned lca_su_h_fact        : 16;     // bits : 15_0
			unsigned lca_su_v_fact        : 16;     // bits : 31_16
		} bit;
		UINT32 word;
	} reg_197; // 0x0314

	union {
		struct {
			unsigned lca_su_h_init_ofs        : 21;     // bits : 20_0
		} bit;
		UINT32 word;
	} reg_198; // 0x0318

	union {
		struct {
			unsigned lca_su_v_init_ofs        : 21;     // bits : 20_0
		} bit;
		UINT32 word;
	} reg_199; // 0x031c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_200; // 0x0320

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_201; // 0x0324

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_202; // 0x0328

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_203; // 0x032c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_204; // 0x0330

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_205; // 0x0334

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_206; // 0x0338

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_207; // 0x033c

	union {
		struct {
			unsigned ime_pm_pxl_sd_h_dnrate        : 4;     // bits : 3_0
			unsigned ime_pm_pxl_sd_v_dnrate        : 4;     // bits : 7_4
		} bit;
		UINT32 word;
	} reg_208; // 0x0340

	union {
		struct {
			unsigned ime_pm_pxl_sd_h_fact        : 16;      // bits : 15_0
			unsigned ime_pm_pxl_sd_v_fact        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} reg_209; // 0x0344

	union {
		struct {
			unsigned ime_pm_pxl_sd_isd_h_base        : 13;      // bits : 12_0
			unsigned                                 : 3;
			unsigned ime_pm_pxl_sd_isd_v_base        : 13;      // bits : 28_16
		} bit;
		UINT32 word;
	} reg_210; // 0x0348

	union {
		struct {
			unsigned ime_pm_pxl_sd_isd_h_fact0        : 13;     // bits : 12_0
			unsigned                                  : 3;
			unsigned ime_pm_pxl_sd_isd_v_fact0        : 13;     // bits : 28_16
		} bit;
		UINT32 word;
	} reg_211; // 0x034c

	union {
		struct {
			unsigned ime_pm_pxl_sd_isd_h_fact1        : 13;     // bits : 12_0
			unsigned                                  : 3;
			unsigned ime_pm_pxl_sd_isd_v_fact1        : 13;     // bits : 28_16
		} bit;
		UINT32 word;
	} reg_212; // 0x0350

	union {
		struct {
			unsigned ime_pm_pxl_sd_isd_h_fact2        : 13;     // bits : 12_0
			unsigned                                  : 3;
			unsigned ime_pm_pxl_sd_isd_v_fact2        : 13;     // bits : 28_16
		} bit;
		UINT32 word;
	} reg_213; // 0x0354

	union {
		struct {
			unsigned ime_pm_subout_h_size        : 11;      // bits : 10_0
			unsigned                             : 5;
			unsigned ime_pm_subout_v_size        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} reg_214; // 0x0358

	union {
		struct {
			unsigned                             : 2;
			unsigned ime_pm_pxl_dram_ofso        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_215; // 0x035c

	union {
		struct {
			unsigned                            : 2;
			unsigned ime_pm_pxl_dram_sao        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} reg_216; // 0x0360

	union {
		struct {
			unsigned ime_pm_pxl_dram_msb_sao        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_217; // 0x0364

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_218; // 0x0368

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_219; // 0x036c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_220; // 0x0370

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_221; // 0x0374

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_222; // 0x0378

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_223; // 0x037c

	union {
		struct {
			unsigned dbcs_ctr_u          : 8;       // bits : 7_0
			unsigned dbcs_ctr_v          : 8;       // bits : 15_8
			unsigned dbcs_mode           : 2;       // bits : 17_16
			unsigned                     : 2;
			unsigned dbcs_step_y         : 2;       // bits : 21_20
			unsigned dbcs_step_uv        : 2;       // bits : 23_22
		} bit;
		UINT32 word;
	} reg_224; // 0x0380

	union {
		struct {
			unsigned dbcs_y_wt0        : 5;     // bits : 4_0
			unsigned dbcs_y_wt1        : 5;     // bits : 9_5
			unsigned dbcs_y_wt2        : 5;     // bits : 14_10
			unsigned dbcs_y_wt3        : 5;     // bits : 19_15
			unsigned dbcs_y_wt4        : 5;     // bits : 24_20
			unsigned dbcs_y_wt5        : 5;     // bits : 29_25
		} bit;
		UINT32 word;
	} reg_225; // 0x0384

	union {
		struct {
			unsigned dbcs_y_wt6         : 5;        // bits : 4_0
			unsigned dbcs_y_wt7         : 5;        // bits : 9_5
			unsigned dbcs_y_wt8         : 5;        // bits : 14_10
			unsigned dbcs_y_wt9         : 5;        // bits : 19_15
			unsigned dbcs_y_wt10        : 5;        // bits : 24_20
			unsigned dbcs_y_wt11        : 5;        // bits : 29_25
		} bit;
		UINT32 word;
	} reg_226; // 0x0388

	union {
		struct {
			unsigned dbcs_y_wt12        : 5;        // bits : 4_0
			unsigned dbcs_y_wt13        : 5;        // bits : 9_5
			unsigned dbcs_y_wt14        : 5;        // bits : 14_10
			unsigned dbcs_y_wt15        : 5;        // bits : 19_15
		} bit;
		UINT32 word;
	} reg_227; // 0x038c

	union {
		struct {
			unsigned dbcs_c_wt0        : 5;     // bits : 4_0
			unsigned dbcs_c_wt1        : 5;     // bits : 9_5
			unsigned dbcs_c_wt2        : 5;     // bits : 14_10
			unsigned dbcs_c_wt3        : 5;     // bits : 19_15
			unsigned dbcs_c_wt4        : 5;     // bits : 24_20
			unsigned dbcs_c_wt5        : 5;     // bits : 29_25
		} bit;
		UINT32 word;
	} reg_228; // 0x0390

	union {
		struct {
			unsigned dbcs_c_wt6         : 5;        // bits : 4_0
			unsigned dbcs_c_wt7         : 5;        // bits : 9_5
			unsigned dbcs_c_wt8         : 5;        // bits : 14_10
			unsigned dbcs_c_wt9         : 5;        // bits : 19_15
			unsigned dbcs_c_wt10        : 5;        // bits : 24_20
			unsigned dbcs_c_wt11        : 5;        // bits : 29_25
		} bit;
		UINT32 word;
	} reg_229; // 0x0394

	union {
		struct {
			unsigned dbcs_c_wt12        : 5;        // bits : 4_0
			unsigned dbcs_c_wt13        : 5;        // bits : 9_5
			unsigned dbcs_c_wt14        : 5;        // bits : 14_10
			unsigned dbcs_c_wt15        : 5;        // bits : 19_15
		} bit;
		UINT32 word;
	} reg_230; // 0x0398

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_231; // 0x039c

	union {
		struct {
			unsigned ime_feye_gain_centx        : 15;       // bits : 14_0
			unsigned                            : 1;
			unsigned ime_feye_gain_centy        : 15;       // bits : 30_16
		} bit;
		UINT32 word;
	} reg_232; // 0x03a0

	union {
		struct {
			unsigned ime_feye_gain_decrase_range        : 8;        // bits : 7_0
			unsigned                                    : 8;
			unsigned ime_feye_gain_radius               : 15;       // bits : 30_16
		} bit;
		UINT32 word;
	} reg_233; // 0x03a4

	union {
		struct {
			unsigned ime_feye_gain_divide_mul          : 15;        // bits : 14_0
			unsigned                                   : 1;
			unsigned ime_feye_gain_divide_shift        : 4;     // bits : 19_16
		} bit;
		UINT32 word;
	} reg_234; // 0x03a8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_235; // 0x03ac

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_236; // 0x03b0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_237; // 0x03b4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_238; // 0x03b8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_239; // 0x03bc

	union {
		struct {
			unsigned t_y_edge_detection_l1_0        : 10;       // bits : 9_0
			unsigned                                : 6;
			unsigned t_y_edge_detection_l1_1        : 10;       // bits : 25_16
			unsigned                                : 3;
			unsigned motion_map_en                  : 1;        // bits : 29
			unsigned texture_map_en                 : 1;        // bits : 30
			unsigned texture_map_src                : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_240; // 0x03c0

	union {
		struct {
			unsigned t_y_edge_detection_l1_2        : 10;       // bits : 9_0
			unsigned                                : 6;
			unsigned t_y_edge_detection_l1_3        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_241; // 0x03c4

	union {
		struct {
			unsigned t_y_edge_detection_l1_4        : 10;       // bits : 9_0
			unsigned                                : 6;
			unsigned t_y_edge_detection_l1_5        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_242; // 0x03c8

	union {
		struct {
			unsigned t_y_edge_detection_l1_6        : 10;       // bits : 9_0
			unsigned                                : 6;
			unsigned t_y_edge_detection_l1_7        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_243; // 0x03cc

	union {
		struct {
			unsigned t_y_edge_detection_l2_0        : 10;       // bits : 9_0
			unsigned                                 : 6;
			unsigned t_y_edge_detection_l2_1         : 10;      // bits : 25_16
		} bit;
		UINT32 word;
	} reg_244; // 0x03d0

	union {
		struct {
			unsigned t_y_edge_detection_l2_2        : 10;       // bits : 9_0
			unsigned                                : 6;
			unsigned t_y_edge_detection_l2_3        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_245; // 0x03d4

	union {
		struct {
			unsigned t_y_edge_detection_l2_4        : 10;       // bits : 9_0
			unsigned                                : 6;
			unsigned t_y_edge_detection_l2_5        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_246; // 0x03d8

	union {
		struct {
			unsigned t_y_edge_detection_l2_6        : 10;       // bits : 9_0
			unsigned                                : 6;
			unsigned t_y_edge_detection_l2_7        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_247; // 0x03dc

	union {
		struct {
			unsigned                                 : 16;
			unsigned t_cb_edge_detection_l2_1        : 10;      // bits : 25_16
		} bit;
		UINT32 word;
	} reg_248; // 0x03e0

	union {
		struct {
			unsigned                                 : 16;
			unsigned t_cr_edge_detection_l2_1        : 10;      // bits : 25_16
		} bit;
		UINT32 word;
	} reg_249; // 0x03e4

	union {
		struct {
			unsigned t_y_edge_smoothing_l1_0        : 8;        // bits : 7_0
			unsigned t_y_edge_smoothing_l1_1        : 8;        // bits : 15_8
			unsigned t_y_edge_smoothing_l1_2        : 8;        // bits : 23_16
			unsigned t_y_edge_smoothing_l1_3        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_250; // 0x03e8

	union {
		struct {
			unsigned t_y_edge_smoothing_l1_4        : 8;        // bits : 7_0
			unsigned t_y_edge_smoothing_l1_5        : 8;        // bits : 15_8
			unsigned t_y_edge_smoothing_l1_6        : 8;        // bits : 23_16
			unsigned t_y_edge_smoothing_l1_7        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_251; // 0x03ec

	union {
		struct {
			unsigned t_y_edge_smoothing_l2_0        : 8;        // bits : 7_0
			unsigned t_y_edge_smoothing_l2_1        : 8;        // bits : 15_8
			unsigned t_y_edge_smoothing_l2_2        : 8;        // bits : 23_16
			unsigned t_y_edge_smoothing_l2_3        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_252; // 0x03f0

	union {
		struct {
			unsigned t_y_edge_smoothing_l2_4        : 8;        // bits : 7_0
			unsigned t_y_edge_smoothing_l2_5        : 8;        // bits : 15_8
			unsigned t_y_edge_smoothing_l2_6        : 8;        // bits : 23_16
			unsigned t_y_edge_smoothing_l2_7        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_253; // 0x03f4

	union {
		struct {
			unsigned                              : 8;
			unsigned t_cb_edge_smoothing_1        : 8;      // bits : 15_8
		} bit;
		UINT32 word;
	} reg_254; // 0x03f8

	union {
		struct {
			unsigned                              : 8;
			unsigned t_cr_edge_smoothing_1        : 8;      // bits : 15_8
		} bit;
		UINT32 word;
	} reg_255; // 0x03fc

	union {
		struct {
			unsigned nr_strength_y_0        : 8;      // bits : 7_0
			unsigned nr_strength_y_1        : 8;      // bits : 15_8
			unsigned                        : 8;
			unsigned nr_strength_c_1        : 8;      // bits : 31_24
		} bit;
		UINT32 word;
	} reg_256; // 0x0400

	union {
		struct {
			unsigned mrnr_edge_out_sel        : 2;      // bits : 1_0
			unsigned edge_dbg_y1_th           : 1;      // bits : 2
			unsigned edge_dbg_y2_th           : 1;      // bits : 3
			unsigned edge_dbg_c2_th           : 1;      // bits : 4
		} bit;
		UINT32 word;
	} reg_257; // 0x0404

	union {
		struct {
			unsigned nr_strength_motion        : 8;        // bits : 7_0
			unsigned nr_strength_trans         : 8;        // bits : 15_8
			unsigned nr_strength_still         : 8;        // bits : 23_16
			unsigned                           : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_258; // 0x0408

	union {
		struct {
			unsigned texture_str_th1        : 8;        // bits : 7_0
			unsigned texture_str_th2        : 8;        // bits : 15_8
			unsigned texture_str_min        : 8;        // bits : 23_16
			unsigned texture_str_max        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_259; // 0x040c

	union {
		struct {
			unsigned texture_str_slope        : 16;        // bits : 15_0
		} bit;
		UINT32 word;
	} reg_260; // 0x0410

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_261; // 0x0414

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_262; // 0x0418

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_263; // 0x041c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_264; // 0x0420

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_265; // 0x0424

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_266; // 0x0428

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_267; // 0x042c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_268; // 0x0430

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_269; // 0x0434

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_270; // 0x0438

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_271; // 0x043c

	union {
		struct {
			unsigned ime_p0_sprt_hbl        : 16;       // bits : 15_0
		} bit;
		UINT32 word;
	} reg_272; // 0x0440

	union {
		struct {
			unsigned                            : 2;
			unsigned ime_p0_y1_dram_ofso        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_273; // 0x0444

	union {
		struct {
			unsigned                             : 2;
			unsigned ime_p0_uv1_dram_ofso        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_274; // 0x0448

	union {
		struct {
			unsigned ime_p0_y1_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_275; // 0x044c

	union {
		struct {
			unsigned ime_p0_uv1_dram_sao        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_276; // 0x0450

	union {
		struct {
			unsigned ime_p1_sprt_hbl        : 16;       // bits : 15_0
		} bit;
		UINT32 word;
	} reg_277; // 0x0454

	union {
		struct {
			unsigned                            : 2;
			unsigned ime_p1_y1_dram_ofso        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_278; // 0x0458

	union {
		struct {
			unsigned                             : 2;
			unsigned ime_p1_uv1_dram_ofso        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_279; // 0x045c

	union {
		struct {
			unsigned ime_p1_y1_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_280; // 0x0460

	union {
		struct {
			unsigned ime_p1_uv1_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_281; // 0x0464

	union {
		struct {
			unsigned ime_p2_sprt_hbl        : 16;       // bits : 15_0
		} bit;
		UINT32 word;
	} reg_282; // 0x0468

	union {
		struct {
			unsigned                            : 2;
			unsigned ime_p2_y1_dram_ofso        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_283; // 0x046c

	union {
		struct {
			unsigned                             : 2;
			unsigned ime_p2_uv1_dram_ofso        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_284; // 0x0470

	union {
		struct {
			unsigned ime_p2_y1_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_285; // 0x0474

	union {
		struct {
			unsigned ime_p2_uv1_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_286; // 0x0478

	union {
		struct {
			unsigned ime_p3_sprt_hbl        : 16;       // bits : 15_0
		} bit;
		UINT32 word;
	} reg_287; // 0x047c

	union {
		struct {
			unsigned                            : 2;
			unsigned ime_p3_y1_dram_ofso        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_288; // 0x0480

	union {
		struct {
			unsigned                             : 2;
			unsigned ime_p3_uv1_dram_ofso        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_289; // 0x0484

	union {
		struct {
			unsigned ime_p3_y1_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_290; // 0x0488

	union {
		struct {
			unsigned ime_p3_uv1_dram_sao        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_291; // 0x048c

	union {
		struct {
			unsigned ime_pm0_line4_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm0_line4_coefa        : 13;       // bits : 16_4
			unsigned ime_pm0_line4_signa        : 1;        // bits : 17
			unsigned ime_pm0_line4_coefb        : 13;       // bits : 30_18
			unsigned ime_pm0_line4_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_292; // 0x0490

	union {
		struct {
			unsigned ime_pm0_line4_coefc        : 26;       // bits : 25_0
			unsigned ime_pm0_line4_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_293; // 0x0494

	union {
		struct {
			unsigned ime_pm0_line5_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm0_line5_coefa        : 13;       // bits : 16_4
			unsigned ime_pm0_line5_signa        : 1;        // bits : 17
			unsigned ime_pm0_line5_coefb        : 13;       // bits : 30_18
			unsigned ime_pm0_line5_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_294; // 0x0498

	union {
		struct {
			unsigned ime_pm0_line5_coefc        : 26;       // bits : 25_0
			unsigned ime_pm0_line5_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_295; // 0x049c

	union {
		struct {
			unsigned ime_pm2_line4_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm2_line4_coefa        : 13;       // bits : 16_4
			unsigned ime_pm2_line4_signa        : 1;        // bits : 17
			unsigned ime_pm2_line4_coefb        : 13;       // bits : 30_18
			unsigned ime_pm2_line4_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_296; // 0x04a0

	union {
		struct {
			unsigned ime_pm2_line4_coefc        : 26;       // bits : 25_0
			unsigned ime_pm2_line4_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_297; // 0x04a4

	union {
		struct {
			unsigned ime_pm2_line5_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm2_line5_coefa        : 13;       // bits : 16_4
			unsigned ime_pm2_line5_signa        : 1;        // bits : 17
			unsigned ime_pm2_line5_coefb        : 13;       // bits : 30_18
			unsigned ime_pm2_line5_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_298; // 0x04a8

	union {
		struct {
			unsigned ime_pm2_line5_coefc        : 26;       // bits : 25_0
			unsigned ime_pm2_line5_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_299; // 0x04ac

	union {
		struct {
			unsigned ime_pm4_line4_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm4_line4_coefa        : 13;       // bits : 16_4
			unsigned ime_pm4_line4_signa        : 1;        // bits : 17
			unsigned ime_pm4_line4_coefb        : 13;       // bits : 30_18
			unsigned ime_pm4_line4_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_300; // 0x04b0

	union {
		struct {
			unsigned ime_pm4_line4_coefc        : 26;       // bits : 25_0
			unsigned ime_pm4_line4_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_301; // 0x04b4

	union {
		struct {
			unsigned ime_pm4_line5_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm4_line5_coefa        : 13;       // bits : 16_4
			unsigned ime_pm4_line5_signa        : 1;        // bits : 17
			unsigned ime_pm4_line5_coefb        : 13;       // bits : 30_18
			unsigned ime_pm4_line5_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_302; // 0x04b8

	union {
		struct {
			unsigned ime_pm4_line5_coefc        : 26;       // bits : 25_0
			unsigned ime_pm4_line5_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_303; // 0x04bc

	union {
		struct {
			unsigned ime_pm6_line4_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm6_line4_coefa        : 13;       // bits : 16_4
			unsigned ime_pm6_line4_signa        : 1;        // bits : 17
			unsigned ime_pm6_line4_coefb        : 13;       // bits : 30_18
			unsigned ime_pm6_line4_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_304; // 0x04c0

	union {
		struct {
			unsigned ime_pm6_line4_coefc        : 26;       // bits : 25_0
			unsigned ime_pm6_line4_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_305; // 0x04c4

	union {
		struct {
			unsigned ime_pm6_line5_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm6_line5_coefa        : 13;       // bits : 16_4
			unsigned ime_pm6_line5_signa        : 1;        // bits : 17
			unsigned ime_pm6_line5_coefb        : 13;       // bits : 30_18
			unsigned ime_pm6_line5_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_306; // 0x04c8

	union {
		struct {
			unsigned ime_pm6_line5_coefc        : 26;       // bits : 25_0
			unsigned ime_pm6_line5_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_307; // 0x04cc

	union {
		struct {
			unsigned ime_pm0_type             : 1;      // bits : 0
			unsigned ime_pm_pxlsize           : 2;      // bits : 2_1
			unsigned ime_pm_proc_loc          : 1;      // bits : 3
			unsigned ime_pm0_color_y          : 8;      // bits : 11_4
			unsigned ime_pm0_color_u          : 8;      // bits : 19_12
			unsigned ime_pm0_color_v          : 8;      // bits : 27_20
			unsigned ime_pm_pxl_src           : 2;      // bits : 29_28
			unsigned                          : 1;
			unsigned ime_pm0_shape_sel        : 1;      // bits : 31
		} bit;
		UINT32 word;
	} reg_308; // 0x04d0

	union {
		struct {
			unsigned ime_pm0_line0_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm0_line0_coefa        : 13;       // bits : 16_4
			unsigned ime_pm0_line0_signa        : 1;        // bits : 17
			unsigned ime_pm0_line0_coefb        : 13;       // bits : 30_18
			unsigned ime_pm0_line0_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_309; // 0x04d4

	union {
		struct {
			unsigned ime_pm0_line0_coefc          : 26;     // bits : 25_0
			unsigned ime_pm0_line0_signc          : 1;      // bits : 26
			unsigned                              : 3;
			unsigned ime_pm0_comb_mode_sel        : 2;      // bits : 31_30
		} bit;
		UINT32 word;
	} reg_310; // 0x04d8

	union {
		struct {
			unsigned ime_pm0_line1_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm0_line1_coefa        : 13;       // bits : 16_4
			unsigned ime_pm0_line1_signa        : 1;        // bits : 17
			unsigned ime_pm0_line1_coefb        : 13;       // bits : 30_18
			unsigned ime_pm0_line1_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_311; // 0x04dc

	union {
		struct {
			unsigned ime_pm0_line1_coefc        : 26;       // bits : 25_0
			unsigned ime_pm0_line1_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_312; // 0x04e0

	union {
		struct {
			unsigned ime_pm0_line2_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm0_line2_coefa        : 13;       // bits : 16_4
			unsigned ime_pm0_line2_signa        : 1;        // bits : 17
			unsigned ime_pm0_line2_coefb        : 13;       // bits : 30_18
			unsigned ime_pm0_line2_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_313; // 0x04e4

	union {
		struct {
			unsigned ime_pm0_line2_coefc        : 26;       // bits : 25_0
			unsigned ime_pm0_line2_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_314; // 0x04e8

	union {
		struct {
			unsigned ime_pm0_line3_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm0_line3_coefa        : 13;       // bits : 16_4
			unsigned ime_pm0_line3_signa        : 1;        // bits : 17
			unsigned ime_pm0_line3_coefb        : 13;       // bits : 30_18
			unsigned ime_pm0_line3_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_315; // 0x04ec

	union {
		struct {
			unsigned ime_pm0_line3_coefc        : 26;       // bits : 25_0
			unsigned ime_pm0_line3_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_316; // 0x04f0

	union {
		struct {
			unsigned ime_pm1_type           : 1;        // bits : 0
			unsigned                        : 3;
			unsigned ime_pm1_color_y        : 8;        // bits : 11_4
			unsigned ime_pm1_color_u        : 8;        // bits : 19_12
			unsigned ime_pm1_color_v        : 8;        // bits : 27_20
		} bit;
		UINT32 word;
	} reg_317; // 0x04f4

	union {
		struct {
			unsigned ime_pm1_line0_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm1_line0_coefa        : 13;       // bits : 16_4
			unsigned ime_pm1_line0_signa        : 1;        // bits : 17
			unsigned ime_pm1_line0_coefb        : 13;       // bits : 30_18
			unsigned ime_pm1_line0_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_318; // 0x04f8

	union {
		struct {
			unsigned ime_pm1_line0_coefc        : 26;       // bits : 25_0
			unsigned ime_pm1_line0_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_319; // 0x04fc

	union {
		struct {
			unsigned ime_pm1_line1_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm1_line1_coefa        : 13;       // bits : 16_4
			unsigned ime_pm1_line1_signa        : 1;        // bits : 17
			unsigned ime_pm1_line1_coefb        : 13;       // bits : 30_18
			unsigned ime_pm1_line1_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_320; // 0x0500

	union {
		struct {
			unsigned ime_pm1_line1_coefc        : 26;       // bits : 25_0
			unsigned ime_pm1_line1_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_321; // 0x0504

	union {
		struct {
			unsigned ime_pm1_line2_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm1_line2_coefa        : 13;       // bits : 16_4
			unsigned ime_pm1_line2_signa        : 1;        // bits : 17
			unsigned ime_pm1_line2_coefb        : 13;       // bits : 30_18
			unsigned ime_pm1_line2_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_322; // 0x0508

	union {
		struct {
			unsigned ime_pm1_line2_coefc        : 26;       // bits : 25_0
			unsigned ime_pm1_line2_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_323; // 0x050c

	union {
		struct {
			unsigned ime_pm1_line3_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm1_line3_coefa        : 13;       // bits : 16_4
			unsigned ime_pm1_line3_signa        : 1;        // bits : 17
			unsigned ime_pm1_line3_coefb        : 13;       // bits : 30_18
			unsigned ime_pm1_line3_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_324; // 0x0510

	union {
		struct {
			unsigned ime_pm1_line3_coefc        : 26;       // bits : 25_0
			unsigned ime_pm1_line3_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_325; // 0x0514

	union {
		struct {
			unsigned ime_pm2_type             : 1;      // bits : 0
			unsigned                          : 3;
			unsigned ime_pm2_color_y          : 8;      // bits : 11_4
			unsigned ime_pm2_color_u          : 8;      // bits : 19_12
			unsigned ime_pm2_color_v          : 8;      // bits : 27_20
			unsigned                          : 3;
			unsigned ime_pm2_shape_sel        : 1;      // bits : 31
		} bit;
		UINT32 word;
	} reg_326; // 0x0518

	union {
		struct {
			unsigned ime_pm2_line0_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm2_line0_coefa        : 13;       // bits : 16_4
			unsigned ime_pm2_line0_signa        : 1;        // bits : 17
			unsigned ime_pm2_line0_coefb        : 13;       // bits : 30_18
			unsigned ime_pm2_line0_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_327; // 0x051c

	union {
		struct {
			unsigned ime_pm2_line0_coefc          : 26;     // bits : 25_0
			unsigned ime_pm2_line0_signc          : 1;      // bits : 26
			unsigned                              : 3;
			unsigned ime_pm2_comb_mode_sel        : 2;      // bits : 31_30
		} bit;
		UINT32 word;
	} reg_328; // 0x0520

	union {
		struct {
			unsigned ime_pm2_line1_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm2_line1_coefa        : 13;       // bits : 16_4
			unsigned ime_pm2_line1_signa        : 1;        // bits : 17
			unsigned ime_pm2_line1_coefb        : 13;       // bits : 30_18
			unsigned ime_pm2_line1_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_329; // 0x0524

	union {
		struct {
			unsigned ime_pm2_line1_coefc        : 26;       // bits : 25_0
			unsigned ime_pm2_line1_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_330; // 0x0528

	union {
		struct {
			unsigned ime_pm2_line2_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm2_line2_coefa        : 13;       // bits : 16_4
			unsigned ime_pm2_line2_signa        : 1;        // bits : 17
			unsigned ime_pm2_line2_coefb        : 13;       // bits : 30_18
			unsigned ime_pm2_line2_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_331; // 0x052c

	union {
		struct {
			unsigned ime_pm2_line2_coefc        : 26;       // bits : 25_0
			unsigned ime_pm2_line2_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_332; // 0x0530

	union {
		struct {
			unsigned ime_pm2_line3_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm2_line3_coefa        : 13;       // bits : 16_4
			unsigned ime_pm2_line3_signa        : 1;        // bits : 17
			unsigned ime_pm2_line3_coefb        : 13;       // bits : 30_18
			unsigned ime_pm2_line3_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_333; // 0x0534

	union {
		struct {
			unsigned ime_pm2_line3_coefc        : 26;       // bits : 25_0
			unsigned ime_pm2_line3_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_334; // 0x0538

	union {
		struct {
			unsigned ime_pm3_type           : 1;        // bits : 0
			unsigned                        : 3;
			unsigned ime_pm3_color_y        : 8;        // bits : 11_4
			unsigned ime_pm3_color_u        : 8;        // bits : 19_12
			unsigned ime_pm3_color_v        : 8;        // bits : 27_20
		} bit;
		UINT32 word;
	} reg_335; // 0x053c

	union {
		struct {
			unsigned ime_pm3_line0_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm3_line0_coefa        : 13;       // bits : 16_4
			unsigned ime_pm3_line0_signa        : 1;        // bits : 17
			unsigned ime_pm3_line0_coefb        : 13;       // bits : 30_18
			unsigned ime_pm3_line0_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_336; // 0x0540

	union {
		struct {
			unsigned ime_pm3_line0_coefc        : 26;       // bits : 25_0
			unsigned ime_pm3_line0_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_337; // 0x0544

	union {
		struct {
			unsigned ime_pm3_line1_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm3_line1_coefa        : 13;       // bits : 16_4
			unsigned ime_pm3_line1_signa        : 1;        // bits : 17
			unsigned ime_pm3_line1_coefb        : 13;       // bits : 30_18
			unsigned ime_pm3_line1_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_338; // 0x0548

	union {
		struct {
			unsigned ime_pm3_line1_coefc        : 26;       // bits : 25_0
			unsigned ime_pm3_line1_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_339; // 0x054c

	union {
		struct {
			unsigned ime_pm3_line2_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm3_line2_coefa        : 13;       // bits : 16_4
			unsigned ime_pm3_line2_signa        : 1;        // bits : 17
			unsigned ime_pm3_line2_coefb        : 13;       // bits : 30_18
			unsigned ime_pm3_line2_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_340; // 0x0550

	union {
		struct {
			unsigned ime_pm3_line2_coefc        : 26;       // bits : 25_0
			unsigned ime_pm3_line2_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_341; // 0x0554

	union {
		struct {
			unsigned ime_pm3_line3_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm3_line3_coefa        : 13;       // bits : 16_4
			unsigned ime_pm3_line3_signa        : 1;        // bits : 17
			unsigned ime_pm3_line3_coefb        : 13;       // bits : 30_18
			unsigned ime_pm3_line3_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_342; // 0x0558

	union {
		struct {
			unsigned ime_pm3_line3_coefc        : 26;       // bits : 25_0
			unsigned ime_pm3_line3_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_343; // 0x055c

	union {
		struct {
			unsigned ime_pm0_awet        : 8;       // bits : 7_0
			unsigned ime_pm1_awet        : 8;       // bits : 15_8
			unsigned ime_pm2_awet        : 8;       // bits : 23_16
			unsigned ime_pm3_awet        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_344; // 0x0560

	union {
		struct {
			unsigned ime_pm4_type             : 1;      // bits : 0
			unsigned                          : 3;
			unsigned ime_pm4_color_y          : 8;      // bits : 11_4
			unsigned ime_pm4_color_u          : 8;      // bits : 19_12
			unsigned ime_pm4_color_v          : 8;      // bits : 27_20
			unsigned                          : 3;
			unsigned ime_pm4_shape_sel        : 1;      // bits : 31
		} bit;
		UINT32 word;
	} reg_345; // 0x0564

	union {
		struct {
			unsigned ime_pm4_line0_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm4_line0_coefa        : 13;       // bits : 16_4
			unsigned ime_pm4_line0_signa        : 1;        // bits : 17
			unsigned ime_pm4_line0_coefb        : 13;       // bits : 30_18
			unsigned ime_pm4_line0_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_346; // 0x0568

	union {
		struct {
			unsigned ime_pm4_line0_coefc          : 26;     // bits : 25_0
			unsigned ime_pm4_line0_signc          : 1;      // bits : 26
			unsigned                              : 3;
			unsigned ime_pm4_comb_mode_sel        : 2;      // bits : 31_30
		} bit;
		UINT32 word;
	} reg_347; // 0x056c

	union {
		struct {
			unsigned ime_pm4_line1_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm4_line1_coefa        : 13;       // bits : 16_4
			unsigned ime_pm4_line1_signa        : 1;        // bits : 17
			unsigned ime_pm4_line1_coefb        : 13;       // bits : 30_18
			unsigned ime_pm4_line1_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_348; // 0x0570

	union {
		struct {
			unsigned ime_pm4_line1_coefc        : 26;       // bits : 25_0
			unsigned ime_pm4_line1_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_349; // 0x0574

	union {
		struct {
			unsigned ime_pm4_line2_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm4_line2_coefa        : 13;       // bits : 16_4
			unsigned ime_pm4_line2_signa        : 1;        // bits : 17
			unsigned ime_pm4_line2_coefb        : 13;       // bits : 30_18
			unsigned ime_pm4_line2_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_350; // 0x0578

	union {
		struct {
			unsigned ime_pm4_line2_coefc        : 26;       // bits : 25_0
			unsigned ime_pm4_line2_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_351; // 0x057c

	union {
		struct {
			unsigned ime_pm4_line3_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm4_line3_coefa        : 13;       // bits : 16_4
			unsigned ime_pm4_line3_signa        : 1;        // bits : 17
			unsigned ime_pm4_line3_coefb        : 13;       // bits : 30_18
			unsigned ime_pm4_line3_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_352; // 0x0580

	union {
		struct {
			unsigned ime_pm4_line3_coefc        : 26;       // bits : 25_0
			unsigned ime_pm4_line3_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_353; // 0x0584

	union {
		struct {
			unsigned ime_pm5_type           : 1;        // bits : 0
			unsigned                        : 3;
			unsigned ime_pm5_color_y        : 8;        // bits : 11_4
			unsigned ime_pm5_color_u        : 8;        // bits : 19_12
			unsigned ime_pm5_color_v        : 8;        // bits : 27_20
		} bit;
		UINT32 word;
	} reg_354; // 0x0588

	union {
		struct {
			unsigned ime_pm5_line0_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm5_line0_coefa        : 13;       // bits : 16_4
			unsigned ime_pm5_line0_signa        : 1;        // bits : 17
			unsigned ime_pm5_line0_coefb        : 13;       // bits : 30_18
			unsigned ime_pm5_line0_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_355; // 0x058c

	union {
		struct {
			unsigned ime_pm5_line0_coefc        : 26;       // bits : 25_0
			unsigned ime_pm5_line0_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_356; // 0x0590

	union {
		struct {
			unsigned ime_pm5_line1_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm5_line1_coefa        : 13;       // bits : 16_4
			unsigned ime_pm5_line1_signa        : 1;        // bits : 17
			unsigned ime_pm5_line1_coefb        : 13;       // bits : 30_18
			unsigned ime_pm5_line1_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_357; // 0x0594

	union {
		struct {
			unsigned ime_pm5_line1_coefc        : 26;       // bits : 25_0
			unsigned ime_pm5_line1_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_358; // 0x0598

	union {
		struct {
			unsigned ime_pm5_line2_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm5_line2_coefa        : 13;       // bits : 16_4
			unsigned ime_pm5_line2_signa        : 1;        // bits : 17
			unsigned ime_pm5_line2_coefb        : 13;       // bits : 30_18
			unsigned ime_pm5_line2_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_359; // 0x059c

	union {
		struct {
			unsigned ime_pm5_line2_coefc        : 26;       // bits : 25_0
			unsigned ime_pm5_line2_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_360; // 0x05a0

	union {
		struct {
			unsigned ime_pm5_line3_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm5_line3_coefa        : 13;       // bits : 16_4
			unsigned ime_pm5_line3_signa        : 1;        // bits : 17
			unsigned ime_pm5_line3_coefb        : 13;       // bits : 30_18
			unsigned ime_pm5_line3_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_361; // 0x05a4

	union {
		struct {
			unsigned ime_pm5_line3_coefc        : 26;       // bits : 25_0
			unsigned ime_pm5_line3_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_362; // 0x05a8

	union {
		struct {
			unsigned ime_pm6_type             : 1;      // bits : 0
			unsigned                          : 3;
			unsigned ime_pm6_color_y          : 8;      // bits : 11_4
			unsigned ime_pm6_color_u          : 8;      // bits : 19_12
			unsigned ime_pm6_color_v          : 8;      // bits : 27_20
			unsigned                          : 3;
			unsigned ime_pm6_shape_sel        : 1;      // bits : 31
		} bit;
		UINT32 word;
	} reg_363; // 0x05ac

	union {
		struct {
			unsigned ime_pm6_line0_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm6_line0_coefa        : 13;       // bits : 16_4
			unsigned ime_pm6_line0_signa        : 1;        // bits : 17
			unsigned ime_pm6_line0_coefb        : 13;       // bits : 30_18
			unsigned ime_pm6_line0_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_364; // 0x05b0

	union {
		struct {
			unsigned ime_pm6_line0_coefc          : 26;     // bits : 25_0
			unsigned ime_pm6_line0_signc          : 1;      // bits : 26
			unsigned                              : 3;
			unsigned ime_pm6_comb_mode_sel        : 2;      // bits : 31_30
		} bit;
		UINT32 word;
	} reg_365; // 0x05b4

	union {
		struct {
			unsigned ime_pm6_line1_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm6_line1_coefa        : 13;       // bits : 16_4
			unsigned ime_pm6_line1_signa        : 1;        // bits : 17
			unsigned ime_pm6_line1_coefb        : 13;       // bits : 30_18
			unsigned ime_pm6_line1_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_366; // 0x05b8

	union {
		struct {
			unsigned ime_pm6_line1_coefc        : 26;       // bits : 25_0
			unsigned ime_pm6_line1_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_367; // 0x05bc

	union {
		struct {
			unsigned ime_pm6_line2_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm6_line2_coefa        : 13;       // bits : 16_4
			unsigned ime_pm6_line2_signa        : 1;        // bits : 17
			unsigned ime_pm6_line2_coefb        : 13;       // bits : 30_18
			unsigned ime_pm6_line2_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_368; // 0x05c0

	union {
		struct {
			unsigned ime_pm6_line2_coefc        : 26;       // bits : 25_0
			unsigned ime_pm6_line2_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_369; // 0x05c4

	union {
		struct {
			unsigned ime_pm6_line3_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm6_line3_coefa        : 13;       // bits : 16_4
			unsigned ime_pm6_line3_signa        : 1;        // bits : 17
			unsigned ime_pm6_line3_coefb        : 13;       // bits : 30_18
			unsigned ime_pm6_line3_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_370; // 0x05c8

	union {
		struct {
			unsigned ime_pm6_line3_coefc        : 26;       // bits : 25_0
			unsigned ime_pm6_line3_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_371; // 0x05cc

	union {
		struct {
			unsigned ime_pm7_type           : 1;        // bits : 0
			unsigned                        : 3;
			unsigned ime_pm7_color_y        : 8;        // bits : 11_4
			unsigned ime_pm7_color_u        : 8;        // bits : 19_12
			unsigned ime_pm7_color_v        : 8;        // bits : 27_20
		} bit;
		UINT32 word;
	} reg_372; // 0x05d0

	union {
		struct {
			unsigned ime_pm7_line0_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm7_line0_coefa        : 13;       // bits : 16_4
			unsigned ime_pm7_line0_signa        : 1;        // bits : 17
			unsigned ime_pm7_line0_coefb        : 13;       // bits : 30_18
			unsigned ime_pm7_line0_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_373; // 0x05d4

	union {
		struct {
			unsigned ime_pm7_line0_coefc        : 26;       // bits : 25_0
			unsigned ime_pm7_line0_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_374; // 0x05d8

	union {
		struct {
			unsigned ime_pm7_line1_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm7_line1_coefa        : 13;       // bits : 16_4
			unsigned ime_pm7_line1_signa        : 1;        // bits : 17
			unsigned ime_pm7_line1_coefb        : 13;       // bits : 30_18
			unsigned ime_pm7_line1_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_375; // 0x05dc

	union {
		struct {
			unsigned ime_pm7_line1_coefc        : 26;       // bits : 25_0
			unsigned ime_pm7_line1_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_376; // 0x05e0

	union {
		struct {
			unsigned ime_pm7_line2_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm7_line2_coefa        : 13;       // bits : 16_4
			unsigned ime_pm7_line2_signa        : 1;        // bits : 17
			unsigned ime_pm7_line2_coefb        : 13;       // bits : 30_18
			unsigned ime_pm7_line2_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_377; // 0x05e4

	union {
		struct {
			unsigned ime_pm7_line2_coefc        : 26;       // bits : 25_0
			unsigned ime_pm7_line2_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_378; // 0x05e8

	union {
		struct {
			unsigned ime_pm7_line3_comp         : 2;        // bits : 1_0
			unsigned                            : 2;
			unsigned ime_pm7_line3_coefa        : 13;       // bits : 16_4
			unsigned ime_pm7_line3_signa        : 1;        // bits : 17
			unsigned ime_pm7_line3_coefb        : 13;       // bits : 30_18
			unsigned ime_pm7_line3_signb        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_379; // 0x05ec

	union {
		struct {
			unsigned ime_pm7_line3_coefc        : 26;       // bits : 25_0
			unsigned ime_pm7_line3_signc        : 1;        // bits : 26
		} bit;
		UINT32 word;
	} reg_380; // 0x05f0

	union {
		struct {
			unsigned ime_pm4_awet        : 8;       // bits : 7_0
			unsigned ime_pm5_awet        : 8;       // bits : 15_8
			unsigned ime_pm6_awet        : 8;       // bits : 23_16
			unsigned ime_pm7_awet        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_381; // 0x05f4

	union {
		struct {
			unsigned ime_pm_subin_h_size        : 11;       // bits : 10_0
			unsigned                            : 5;
			unsigned ime_pm_subin_v_size        : 11;       // bits : 26_16
			unsigned                            : 3;
			unsigned ime_pm_fmt                 : 2;        // bits : 31_30
		} bit;
		UINT32 word;
	} reg_382; // 0x05f8

	union {
		struct {
			unsigned                      : 2;
			unsigned ime_pm_y_ofsi        : 18;     // bits : 19_2
		} bit;
		UINT32 word;
	} reg_383; // 0x05fc

	union {
		struct {
			unsigned                     : 2;
			unsigned ime_pm_y_sai        : 30;      // bits : 31_2
		} bit;
		UINT32 word;
	} reg_384; // 0x0600

	union {
		struct {
			unsigned ime_in_pxl_bp0        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_385; // 0x0604

	union {
		struct {
			unsigned ime_in_pxl_bp1        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_386; // 0x0608

	union {
		struct {
			unsigned ime_in_pxl_bp2        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_387; // 0x060c

	union {
		struct {
			unsigned ime_in_line_bp0        : 16;       // bits : 15_0
			unsigned ime_in_line_bp1        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} reg_388; // 0x0610

	union {
		struct {
			unsigned ime_in_line_bp2        : 16;       // bits : 15_0
			unsigned                        : 15;
			unsigned ime_bp_mode            : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_389; // 0x0614

	union {
		struct {
			unsigned ime_pm_y_msb_sai        : 4;       // bits : 3_0
		} bit;
		UINT32 word;
	} reg_390; // 0x0618

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_391; // 0x061c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_392; // 0x0620

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_393; // 0x0624

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_394; // 0x0628

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_395; // 0x062c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_396; // 0x0630

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_397; // 0x0634

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_398; // 0x0638

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_399; // 0x063c

	union {
		struct {
			unsigned ime_in_bst_y                   : 1;        // bits : 0
			unsigned ime_in_bst_u                   : 1;        // bits : 1
			unsigned ime_in_bst_v                   : 1;        // bits : 2
			unsigned ime_out_p0_bst_y               : 1;        // bits : 3
			unsigned ime_out_p0_bst_u               : 1;        // bits : 4
			unsigned ime_out_p0_bst_v               : 1;        // bits : 5
			unsigned ime_out_p1_bst_y               : 1;        // bits : 6
			unsigned ime_out_p1_bst_uv              : 1;        // bits : 7
			unsigned ime_out_p2_bst_y               : 1;        // bits : 8
			unsigned ime_out_p2_bst_uv              : 1;        // bits : 9
			unsigned ime_out_p3_bst_y               : 1;        // bits : 10
			unsigned ime_out_p3_bst_uv              : 1;        // bits : 11
			unsigned ime_in_pix_bst                 : 1;        // bits : 12
			unsigned ime_out_sub_bst                : 1;        // bits : 13
			unsigned ime_in_3dnr_bst_fcp            : 1;        // bits : 14
			unsigned ime_out_3dnr_bst_fcp           : 1;        // bits : 15
			unsigned ime_in_3dnr_bst_y              : 1;        // bits : 16
			unsigned ime_in_3dnr_bst_c              : 1;        // bits : 17
			unsigned ime_out_3dnr_bst_y             : 1;        // bits : 18
			unsigned ime_out_3dnr_bst_c             : 1;        // bits : 19
			unsigned ime_in_3dnr_bst_mv             : 1;        // bits : 20
			unsigned ime_out_3dnr_bst_mv            : 1;        // bits : 21
			unsigned                                : 2;
			unsigned ime_out_3dnr_bst_mo_roi        : 1;        // bits : 24
			unsigned ime_out_3dnr_bst_sta           : 1;        // bits : 25
		} bit;
		UINT32 word;
	} reg_400; // 0x0640

	union {
		struct {
			unsigned                          : 30;
			unsigned ime_frm_start_rst        : 1;      // bits : 30
			unsigned ime_chksum_en            : 1;      // bits : 31
		} bit;
		UINT32 word;
	} reg_401; // 0x0644

	union {
		struct {
			unsigned ime_ll_cmd_start_addr_info        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_402; // 0x0648

	union {
		struct {
			unsigned ime_ll_cmdprs_cnt        : 20;     // bits : 19_0
		} bit;
		UINT32 word;
	} reg_403; // 0x064c

	union {
		struct {
			unsigned ime_ll_tab0        : 8;        // bits : 7_0
			unsigned ime_ll_tab1        : 8;        // bits : 15_8
			unsigned ime_ll_tab2        : 8;        // bits : 23_16
			unsigned ime_ll_tab3        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_404; // 0x0650

	union {
		struct {
			unsigned ime_ll_tab4        : 8;        // bits : 7_0
			unsigned ime_ll_tab5        : 8;        // bits : 15_8
			unsigned ime_ll_tab6        : 8;        // bits : 23_16
			unsigned ime_ll_tab7        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_405; // 0x0654

	union {
		struct {
			unsigned ime_ll_tab8         : 8;       // bits : 7_0
			unsigned ime_ll_tab9         : 8;       // bits : 15_8
			unsigned ime_ll_tab10        : 8;       // bits : 23_16
			unsigned ime_ll_tab11        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_406; // 0x0658

	union {
		struct {
			unsigned ime_ll_tab12        : 8;       // bits : 7_0
			unsigned ime_ll_tab13        : 8;       // bits : 15_8
			unsigned ime_ll_tab14        : 8;       // bits : 23_16
			unsigned ime_ll_tab15        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_407; // 0x065c

	union {
		struct {
			unsigned shp_edge_weight_src_sel        : 1;        // bits : 0
			unsigned shp_motion_bit_en              : 1;        // bits : 1
			unsigned shp_show_info                  : 2;        // bits : 3_2
			unsigned shp_jnd_filter_size            : 2;        // bits : 5_4
			unsigned                                : 2;
			unsigned shp_edge_weight_th             : 8;        // bits : 15_8
			unsigned shp_edge_weight_gain           : 8;        // bits : 23_16
			unsigned shp_noise_level                : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_408; // 0x0660

	union {
		struct {
			unsigned shp_blend_inv_gamma        : 8;        // bits : 7_0
			unsigned shp_edge_str               : 8;        // bits : 15_8
			unsigned shp_coring_th              : 8;        // bits : 23_16
			unsigned shp_w_con_eng              : 4;        // bits : 27_24
		} bit;
		UINT32 word;
	} reg_409; // 0x0664

	union {
		struct {
			unsigned shp_bright_halo_clip        : 8;       // bits : 7_0
			unsigned shp_dark_halo_clip          : 8;       // bits : 15_8
		} bit;
		UINT32 word;
	} reg_410; // 0x0668

	union {
		struct {
			unsigned shp_flat_th        : 11;       // bits : 10_0
			unsigned                    : 5;
			unsigned shp_edge_th        : 11;       // bits : 26_16
		} bit;
		UINT32 word;
	} reg_411; // 0x066c

	union {
		struct {
			unsigned shp_slope_con_eng          : 12;       // bits : 11_0
			unsigned                            : 4;
			unsigned shp_flat_region_str        : 8;        // bits : 23_16
			unsigned shp_edge_region_str        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_412; // 0x0670

	union {
		struct {
			unsigned shp_motion_edge_weight_str        : 8;     // bits : 7_0
			unsigned shp_static_edge_weight_str        : 8;     // bits : 15_8
			unsigned shp_trans_edge_weight_str         : 8;     // bits : 23_16
		} bit;
		UINT32 word;
	} reg_413; // 0x0674

	union {
		struct {
			unsigned shp_noise_curve0        : 8;       // bits : 7_0
			unsigned shp_noise_curve1        : 8;       // bits : 15_8
			unsigned shp_noise_curve2        : 8;       // bits : 23_16
			unsigned shp_noise_curve3        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_414; // 0x0678

	union {
		struct {
			unsigned shp_noise_curve4        : 8;       // bits : 7_0
			unsigned shp_noise_curve5        : 8;       // bits : 15_8
			unsigned shp_noise_curve6        : 8;       // bits : 23_16
			unsigned shp_noise_curve7        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_415; // 0x067c

	union {
		struct {
			unsigned shp_noise_curve8         : 8;      // bits : 7_0
			unsigned shp_noise_curve9         : 8;      // bits : 15_8
			unsigned shp_noise_curve10        : 8;      // bits : 23_16
			unsigned shp_noise_curve11        : 8;      // bits : 31_24
		} bit;
		UINT32 word;
	} reg_416; // 0x0680

	union {
		struct {
			unsigned shp_noise_curve12        : 8;      // bits : 7_0
			unsigned shp_noise_curve13        : 8;      // bits : 15_8
			unsigned shp_noise_curve14        : 8;      // bits : 23_16
			unsigned shp_noise_curve15        : 8;      // bits : 31_24
		} bit;
		UINT32 word;
	} reg_417; // 0x0684

	union {
		struct {
			unsigned shp_noise_curve16        : 8;      // bits : 7_0
		} bit;
		UINT32 word;
	} reg_418; // 0x0688

	union {
		struct {
			unsigned shp_ewg_curve0        : 8;     // bits : 7_0
			unsigned shp_ewg_curve1        : 8;     // bits : 15_8
			unsigned shp_ewg_curve2        : 8;     // bits : 23_16
			unsigned shp_ewg_curve3        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_419; // 0x068c

	union {
		struct {
			unsigned shp_ewg_curve4        : 8;     // bits : 7_0
			unsigned shp_ewg_curve5        : 8;     // bits : 15_8
			unsigned shp_ewg_curve6        : 8;     // bits : 23_16
			unsigned shp_ewg_curve7        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_420; // 0x0690

	union {
		struct {
			unsigned shp_ewg_curve8        : 8;     // bits : 7_0
		} bit;
		UINT32 word;
	} reg_421; // 0x0694

	union {
		struct {
			unsigned shp_ewg_curve0_msb        : 3;     // bits : 2_0
			unsigned shp_ewg_curve1_msb        : 3;     // bits : 5_3
			unsigned shp_ewg_curve2_msb        : 3;     // bits : 8_6
			unsigned shp_ewg_curve3_msb        : 3;     // bits : 11_9
			unsigned shp_ewg_curve4_msb        : 3;     // bits : 14_12
			unsigned shp_ewg_curve5_msb        : 3;     // bits : 17_15
			unsigned shp_ewg_curve6_msb        : 3;     // bits : 20_18
			unsigned shp_ewg_curve7_msb        : 3;     // bits : 23_21
			unsigned shp_ewg_curve8_msb        : 3;     // bits : 26_24
		} bit;
		UINT32 word;
	} reg_422; // 0x0698

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_423; // 0x069c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_424; // 0x06a0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_425; // 0x06a4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_426; // 0x06a8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_427; // 0x06ac

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_428; // 0x06b0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_429; // 0x06b4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_430; // 0x06b8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_431; // 0x06bc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_432; // 0x06c0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_433; // 0x06c4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_434; // 0x06c8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_435; // 0x06cc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_436; // 0x06d0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_437; // 0x06d4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_438; // 0x06d8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_439; // 0x06dc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_440; // 0x06e0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_441; // 0x06e4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_442; // 0x06e8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_443; // 0x06ec

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_444; // 0x06f0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_445; // 0x06f4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_446; // 0x06f8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_447; // 0x06fc

	union {
		struct {
			unsigned ime_3dnr_pre_y_blur_str              : 2;      // bits : 1_0
			unsigned ime_3dnr_pf_type                     : 2;      // bits : 3_2
			unsigned ime_3dnr_me_update_mode              : 1;      // bits : 4
			unsigned ime_3dnr_me_boundary_set             : 1;      // bits : 5
			//unsigned ime_3dnr_me_mv_ds_mode               : 2;      // bits : 7_6
			unsigned                : 2;
			unsigned ime_3dnr_ps_smart_roi_ctrl           : 1;      // bits : 8
			unsigned ime_3dnr_nr_center_wzero_y_3d        : 1;      // bits : 9
			unsigned ime_3dnr_ps_mv_check_en              : 1;      // bits : 10
			unsigned ime_3dnr_ps_mv_check_roi_en          : 1;      // bits : 11
			unsigned ime_3dnr_ps_mv_info_mode             : 2;      // bits : 13_12
			//unsigned ime_3dnr_ps_mode                     : 1;      // bits : 14
			//unsigned ime_3dnr_me_sad_type                 : 1;      // bits : 15
			unsigned                : 2;
			unsigned ime_3dnr_me_sad_shift                : 4;      // bits : 19_16
			unsigned ime_3dnr_nr_y_ch_en                  : 1;      // bits : 20
			unsigned ime_3dnr_nr_c_ch_en                  : 1;      // bits : 21
			//unsigned ime_3dnr_nr_c_fsv_en                 : 1;      // bits : 22
			unsigned                : 1;
			unsigned ime_3dnr_seed_reset_en               : 1;      // bits : 23
			//unsigned ime_3dnr_nr_c_fsv                    : 8;      // bits : 31_24
		} bit;
		UINT32 word;
	} reg_448; // 0x0700

	union {
		struct {
			unsigned ime_3dnr_ne_sample_step_x           : 8;       // bits : 7_0
			unsigned ime_3dnr_ne_sample_step_y           : 8;       // bits : 15_8
			unsigned ime_3dnr_statistic_output_en        : 1;       // bits : 16
			unsigned ime_3dnr_ps_fastc_en                : 1;       // bits : 17
			unsigned ime_3dnr_dbg_mv0                    : 1;       // bits : 18
			unsigned                                     : 9;
			unsigned ime_3dnr_dbg_mode                   : 4;       // bits : 31_28
		} bit;
		UINT32 word;
	} reg_449; // 0x0704

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_450; // 0x0708

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_451; // 0x070c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_452; // 0x0710

	union {
		struct {
			unsigned ime_3dnr_ro_motion_sum        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_453; // 0x0714

	union {
		struct {
			unsigned ime_3dnr_ro_edge_sum        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_454; // 0x0718

	union {
		struct {
			unsigned ime_3dnr_ro_sad_sum        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_455; // 0x071c

	union {
		struct {
			unsigned ime_3dnr_ro_mv_sum        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_456; // 0x0720

	union {
		struct {
			unsigned ime_3dnr_ro_sample_cnt        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_457; // 0x0724

	union {
		struct {
			unsigned ime_3dnr_me_sad_penalty_0        : 10;     // bits : 9_0
			unsigned ime_3dnr_me_sad_penalty_1        : 10;     // bits : 19_10
			unsigned ime_3dnr_me_sad_penalty_2        : 10;     // bits : 29_20
		} bit;
		UINT32 word;
	} reg_458; // 0x0728

	union {
		struct {
			unsigned ime_3dnr_me_sad_penalty_3        : 10;     // bits : 9_0
			unsigned ime_3dnr_me_sad_penalty_4        : 10;     // bits : 19_10
			unsigned ime_3dnr_me_sad_penalty_5        : 10;     // bits : 29_20
		} bit;
		UINT32 word;
	} reg_459; // 0x072c

	union {
		struct {
			unsigned ime_3dnr_me_sad_penalty_6        : 10;     // bits : 9_0
			unsigned ime_3dnr_me_sad_penalty_7        : 10;     // bits : 19_10
		} bit;
		UINT32 word;
	} reg_460; // 0x0730

	union {
		struct {
			unsigned ime_3dnr_me_switch_th0        : 8;     // bits : 7_0
			unsigned ime_3dnr_me_switch_th1        : 8;     // bits : 15_8
			unsigned ime_3dnr_me_switch_th2        : 8;     // bits : 23_16
			unsigned ime_3dnr_me_switch_th3        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_461; // 0x0734

	union {
		struct {
			unsigned ime_3dnr_me_switch_th4        : 8;     // bits : 7_0
			unsigned ime_3dnr_me_switch_th5        : 8;     // bits : 15_8
			unsigned ime_3dnr_me_switch_th6        : 8;     // bits : 23_16
			unsigned ime_3dnr_me_switch_th7        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_462; // 0x0738

	union {
		struct {
			unsigned ime_3dnr_me_switch_ratio        : 8;       // bits : 7_0
			unsigned ime_3dnr_me_cost_blend          : 4;       // bits : 11_8
		} bit;
		UINT32 word;
	} reg_463; // 0x073c

	union {
		struct {
			unsigned ime_3dnr_me_detail_penalty0        : 4;        // bits : 3_0
			unsigned ime_3dnr_me_detail_penalty1        : 4;        // bits : 7_4
			unsigned ime_3dnr_me_detail_penalty2        : 4;        // bits : 11_8
			unsigned ime_3dnr_me_detail_penalty3        : 4;        // bits : 15_12
			unsigned ime_3dnr_me_detail_penalty4        : 4;        // bits : 19_16
			unsigned ime_3dnr_me_detail_penalty5        : 4;        // bits : 23_20
			unsigned ime_3dnr_me_detail_penalty6        : 4;        // bits : 27_24
			unsigned ime_3dnr_me_detail_penalty7        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_464; // 0x0740

	union {
		struct {
			unsigned ime_3dnr_me_probability0        : 1;       // bits : 0
			unsigned ime_3dnr_me_probability1        : 1;       // bits : 1
			unsigned ime_3dnr_me_probability2        : 1;       // bits : 2
			unsigned ime_3dnr_me_probability3        : 1;       // bits : 3
			unsigned ime_3dnr_me_probability4        : 1;       // bits : 4
			unsigned ime_3dnr_me_probability5        : 1;       // bits : 5
			unsigned ime_3dnr_me_probability6        : 1;       // bits : 6
			unsigned ime_3dnr_me_probability7        : 1;       // bits : 7
			unsigned ime_3dnr_me_rand_bit_x          : 3;       // bits : 10_8
			unsigned                                 : 1;
			unsigned ime_3dnr_me_rand_bit_y          : 3;       // bits : 14_12
			unsigned                                 : 1;
			unsigned ime_3dnr_me_min_detail          : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_465; // 0x0744

	union {
		struct {
			unsigned ime_3dnr_pf_str        : 8;        // bits : 7_0
		} bit;
		UINT32 word;
	} reg_466; // 0x0748

	union {
		struct {
			unsigned ime_3dnr_ne_sample_num_x        : 12;      // bits : 11_0
			unsigned                                 : 4;
			unsigned ime_3dnr_ne_sample_num_y        : 12;      // bits : 27_16
		} bit;
		UINT32 word;
	} reg_467; // 0x074c

	union {
		struct {
			unsigned ime_3dnr_ne_sample_start_x        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ime_3dnr_ne_sample_start_y        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} reg_468; // 0x0750

	union {
		struct {
			unsigned ime_3dnr_fast_converge_sp          : 4;        // bits : 3_0
			unsigned ime_3dnr_fast_converge_step        : 4;        // bits : 7_4
		} bit;
		UINT32 word;
	} reg_469; // 0x0754

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_470; // 0x0758

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_471; // 0x075c

	union {
		struct {
			unsigned ime_3dnr_md_sad_coef_a0        : 6;        // bits : 5_0
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_a1        : 6;        // bits : 13_8
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_a2        : 6;        // bits : 21_16
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_a3        : 6;        // bits : 29_24
		} bit;
		UINT32 word;
	} reg_472; // 0x0760

	union {
		struct {
			unsigned ime_3dnr_md_sad_coef_a4        : 6;        // bits : 5_0
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_a5        : 6;        // bits : 13_8
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_a6        : 6;        // bits : 21_16
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_a7        : 6;        // bits : 29_24
		} bit;
		UINT32 word;
	} reg_473; // 0x0764

	union {
		struct {
			unsigned ime_3dnr_md_sad_coef_b0        : 14;       // bits : 13_0
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_b1        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_474; // 0x0768

	union {
		struct {
			unsigned ime_3dnr_md_sad_coef_b2        : 14;       // bits : 13_0
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_b3        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_475; // 0x076c

	union {
		struct {
			unsigned ime_3dnr_md_sad_coef_b4        : 14;       // bits : 13_0
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_b5        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_476; // 0x0770

	union {
		struct {
			unsigned ime_3dnr_md_sad_coef_b6        : 14;       // bits : 13_0
			unsigned                                : 2;
			unsigned ime_3dnr_md_sad_coef_b7        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_477; // 0x0774

	union {
		struct {
			unsigned ime_3dnr_md_sad_std0        : 14;      // bits : 13_0
			unsigned                             : 2;
			unsigned ime_3dnr_md_sad_std1        : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_478; // 0x0778

	union {
		struct {
			unsigned ime_3dnr_md_sad_std2        : 14;      // bits : 13_0
			unsigned                             : 2;
			unsigned ime_3dnr_md_sad_std3        : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_479; // 0x077c

	union {
		struct {
			unsigned ime_3dnr_md_sad_std4        : 14;      // bits : 13_0
			unsigned                             : 2;
			unsigned ime_3dnr_md_sad_std5        : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_480; // 0x0780

	union {
		struct {
			unsigned ime_3dnr_md_sad_std6        : 14;      // bits : 13_0
			unsigned                             : 2;
			unsigned ime_3dnr_md_sad_std7        : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_481; // 0x0784

	union {
		struct {
			unsigned ime_3dnr_md_k1        : 6;     // bits : 5_0
			unsigned                       : 2;
			unsigned ime_3dnr_md_k2        : 6;     // bits : 13_8
		} bit;
		UINT32 word;
	} reg_482; // 0x0788

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_483; // 0x078c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_484; // 0x0790

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_485; // 0x0794

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_486; // 0x0798

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_487; // 0x079c

	union {
		struct {
			unsigned ime_3dnr_mc_sad_base0        : 14;     // bits : 13_0
			unsigned                              : 2;
			unsigned ime_3dnr_mc_sad_base1        : 14;     // bits : 29_16
		} bit;
		UINT32 word;
	} reg_488; // 0x07a0

	union {
		struct {
			unsigned ime_3dnr_mc_sad_base2        : 14;     // bits : 13_0
			unsigned                              : 2;
			unsigned ime_3dnr_mc_sad_base3        : 14;     // bits : 29_16
		} bit;
		UINT32 word;
	} reg_489; // 0x07a4

	union {
		struct {
			unsigned ime_3dnr_mc_sad_base4        : 14;     // bits : 13_0
			unsigned                              : 2;
			unsigned ime_3dnr_mc_sad_base5        : 14;     // bits : 29_16
		} bit;
		UINT32 word;
	} reg_490; // 0x07a8

	union {
		struct {
			unsigned ime_3dnr_mc_sad_base6        : 14;     // bits : 13_0
			unsigned                              : 2;
			unsigned ime_3dnr_mc_sad_base7        : 14;     // bits : 29_16
		} bit;
		UINT32 word;
	} reg_491; // 0x07ac

	union {
		struct {
			unsigned ime_3dnr_mc_sad_coef_a0        : 6;        // bits : 5_0
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_a1        : 6;        // bits : 13_8
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_a2        : 6;        // bits : 21_16
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_a3        : 6;        // bits : 29_24
		} bit;
		UINT32 word;
	} reg_492; // 0x07b0

	union {
		struct {
			unsigned ime_3dnr_mc_sad_coef_a4        : 6;        // bits : 5_0
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_a5        : 6;        // bits : 13_8
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_a6        : 6;        // bits : 21_16
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_a7        : 6;        // bits : 29_24
		} bit;
		UINT32 word;
	} reg_493; // 0x07b4

	union {
		struct {
			unsigned ime_3dnr_mc_sad_coef_b0        : 14;       // bits : 13_0
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_b1        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_494; // 0x07b8

	union {
		struct {
			unsigned ime_3dnr_mc_sad_coef_b2        : 14;       // bits : 13_0
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_b3        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_495; // 0x07bc

	union {
		struct {
			unsigned ime_3dnr_mc_sad_coef_b4        : 14;       // bits : 13_0
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_b5        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_496; // 0x07c0

	union {
		struct {
			unsigned ime_3dnr_mc_sad_coef_b6        : 14;       // bits : 13_0
			unsigned                                : 2;
			unsigned ime_3dnr_mc_sad_coef_b7        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_497; // 0x07c4

	union {
		struct {
			unsigned ime_3dnr_mc_sad_std0        : 14;      // bits : 13_0
			unsigned                             : 2;
			unsigned ime_3dnr_mc_sad_std1        : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_498; // 0x07c8

	union {
		struct {
			unsigned ime_3dnr_mc_sad_std2        : 14;      // bits : 13_0
			unsigned                             : 2;
			unsigned ime_3dnr_mc_sad_std3        : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_499; // 0x07cc

	union {
		struct {
			unsigned ime_3dnr_mc_sad_std4        : 14;      // bits : 13_0
			unsigned                             : 2;
			unsigned ime_3dnr_mc_sad_std5        : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_500; // 0x07d0

	union {
		struct {
			unsigned ime_3dnr_mc_sad_std6        : 14;      // bits : 13_0
			unsigned                             : 2;
			unsigned ime_3dnr_mc_sad_std7        : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_501; // 0x07d4

	union {
		struct {
			unsigned ime_3dnr_mc_k1        : 6;     // bits : 5_0
			unsigned                       : 2;
			unsigned ime_3dnr_mc_k2        : 6;     // bits : 13_8
		} bit;
		UINT32 word;
	} reg_502; // 0x07d8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_503; // 0x07dc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_504; // 0x07e0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_505; // 0x07e4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_506; // 0x07e8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_507; // 0x07ec

	union {
		struct {
			unsigned ime_3dnr_roi_md_k1        : 6;     // bits : 5_0
			unsigned                           : 2;
			unsigned ime_3dnr_roi_md_k2        : 6;     // bits : 13_8
		} bit;
		UINT32 word;
	} reg_508; // 0x07f0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_509; // 0x07f4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_510; // 0x07f8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_511; // 0x07fc

	union {
		struct {
			unsigned ime_3dnr_roi_mc_k1          : 6;       // bits : 5_0
			unsigned                             : 2;
			unsigned ime_3dnr_roi_mc_k2          : 6;       // bits : 13_8
			unsigned                             : 2;
			unsigned ime_3dnr_ps_blur_eth        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} reg_512; // 0x0800

	union {
		struct {
			unsigned ime_3dnr_nr_y_tf0_blur_str0        : 8;        // bits : 7_0
			unsigned ime_3dnr_nr_y_tf0_blur_str1        : 8;        // bits : 15_8
			unsigned ime_3dnr_nr_y_tf0_blur_str2        : 8;        // bits : 23_16
			unsigned ime_3dnr_nr_y_tf0_str0             : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_513; // 0x0804

	union {
		struct {
			unsigned ime_3dnr_nr_y_tf0_str1        : 8;     // bits : 7_0
			unsigned ime_3dnr_nr_y_tf0_str2        : 8;     // bits : 15_8
			unsigned ime_3dnr_nr_c_tf0_str0        : 8;     // bits : 23_16
			unsigned ime_3dnr_nr_c_tf0_str1        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_514; // 0x0808

	union {
		struct {
			unsigned ime_3dnr_nr_c_tf0_str2             : 8;        // bits : 7_0
			unsigned ime_3dnr_nr_y_tf0_blur_estr        : 8;        // bits : 15_8
		} bit;
		UINT32 word;
	} reg_515; // 0x080c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_516; // 0x0810

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_517; // 0x0814

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_518; // 0x0818

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_519; // 0x081c

	union {
		struct {
			unsigned ime_3dnr_ps_mv_th            : 6;      // bits : 5_0
			unsigned                              : 2;
			unsigned ime_3dnr_ps_roi_mv_th        : 6;      // bits : 13_8
			unsigned                              : 2;
			unsigned ime_3dnr_ps_fs_th            : 14;     // bits : 29_16
		} bit;
		UINT32 word;
	} reg_520; // 0x0820

	union {
		struct {
			unsigned ime_3dnr_ps_mix_ratio0        : 8;     // bits : 7_0
			unsigned ime_3dnr_ps_mix_ratio1        : 8;     // bits : 15_8
		} bit;
		UINT32 word;
	} reg_521; // 0x0824

	union {
		struct {
			//unsigned ime_3dnr_ps_mix_th0        : 14;       // bits : 13_0
			//unsigned                            : 2;
			//unsigned ime_3dnr_ps_mix_th1        : 14;       // bits : 29_16
			unsigned ime_3dnr_nr_u_tf0_md_th          : 8;      // bits : 7_0
			unsigned ime_3dnr_nr_v_tf0_md_th          : 8;      // bits : 15_8
			unsigned ime_3dnr_nr_c_tf0_ratio_0        : 8;      // bits : 23_16
			unsigned ime_3dnr_nr_c_tf0_ratio_1        : 8;      // bits : 31_24
		} bit;
		UINT32 word;
	} reg_522; // 0x0828

	union {
		struct {
			//unsigned ime_3dnr_ps_mix_slope0        : 16;        // bits : 15_0
			//unsigned ime_3dnr_ps_mix_slope1        : 16;        // bits : 31_16
			unsigned ime_3dnr_nr_motion_sat_ratio         : 8;      // bits : 7_0
			unsigned ime_3dnr_nr_c_tf0_residual_th        : 4;      // bits : 11_8
		} bit;
		UINT32 word;
	} reg_523; // 0x082c

	union {
		struct {
			unsigned ime_3dnr_nr_cshk_th0        : 10;      // bits : 9_0
			unsigned                             : 6;
			unsigned ime_3dnr_nr_cshk_th1        : 10;      // bits : 25_16
		} bit;
		UINT32 word;
	} reg_524; // 0x0830

	union {
		struct {
			unsigned ime_3dnr_nr_cshk_th2        : 10;      // bits : 9_0
			unsigned                             : 6;
			unsigned ime_3dnr_nr_cshk_th3        : 10;      // bits : 25_16
		} bit;
		UINT32 word;
	} reg_525; // 0x0834

	union {
		struct {
			unsigned ime_3dnr_nr_cshk_th4        : 10;      // bits : 9_0
			unsigned                             : 6;
			unsigned ime_3dnr_nr_cshk_th5        : 10;      // bits : 25_16
		} bit;
		UINT32 word;
	} reg_526; // 0x0838

	union {
		struct {
			unsigned ime_3dnr_nr_cshk_th6        : 10;      // bits : 9_0
			unsigned                             : 6;
			unsigned ime_3dnr_nr_cshk_th7        : 10;      // bits : 25_16
		} bit;
		UINT32 word;
	} reg_527; // 0x083c

	union {
		struct {
			unsigned ime_3dnr_nr_cshk_val0        : 3;      // bits : 2_0
			unsigned                              : 1;
			unsigned ime_3dnr_nr_cshk_val1        : 3;      // bits : 6_4
			unsigned                              : 1;
			unsigned ime_3dnr_nr_cshk_val2        : 3;      // bits : 10_8
			unsigned                              : 1;
			unsigned ime_3dnr_nr_cshk_val3        : 3;      // bits : 14_12
			unsigned                              : 1;
			unsigned ime_3dnr_nr_cshk_val4        : 3;      // bits : 18_16
			unsigned                              : 1;
			unsigned ime_3dnr_nr_cshk_val5        : 3;      // bits : 22_20
			unsigned                              : 1;
			unsigned ime_3dnr_nr_cshk_val6        : 3;      // bits : 26_24
			unsigned                              : 1;
			unsigned ime_3dnr_nr_cshk_val7        : 3;      // bits : 30_28
		} bit;
		UINT32 word;
	} reg_528; // 0x0840

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_529; // 0x0844

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_530; // 0x0848

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_531; // 0x084c

	union {
		struct {
			unsigned ime_3dnr_ps_ds_th            : 5;      // bits : 4_0
			unsigned                              : 3;
			unsigned ime_3dnr_ps_ds_th_roi        : 5;      // bits : 12_8
			//unsigned                              : 3;
			//unsigned ime_3dnr_ps_edge_w           : 8;      // bits : 23_16
		} bit;
		UINT32 word;
	} reg_532; // 0x0850

	union {
		struct {
			//unsigned ime_3dnr_ps_edge_th0        : 14;      // bits : 13_0
			unsigned                             : 32;
			//unsigned ime_3dnr_ps_edge_th1        : 14;      // bits : 29_16
		} bit;
		UINT32 word;
	} reg_533; // 0x0854

	union {
		struct {
			//unsigned ime_3dnr_ps_edge_slope        : 16;        // bits : 15_0
			unsigned         : 32;
		} bit;
		UINT32 word;
	} reg_534; // 0x0858

	union {
		struct {
			unsigned ime_3dnr_nr_residue_th_y0        : 4;      // bits : 3_0
			unsigned                                  : 4;
			unsigned ime_3dnr_nr_residue_th_y1        : 4;      // bits : 11_8
			unsigned                                  : 4;
			unsigned ime_3dnr_nr_residue_th_y2        : 4;      // bits : 19_16
			unsigned                                  : 4;
			unsigned ime_3dnr_nr_residue_th_c         : 4;      // bits : 27_24
		} bit;
		UINT32 word;
	} reg_535; // 0x085c

	union {
		struct {
			unsigned ime_3dnr_nr_freq_w0        : 8;        // bits : 7_0
			unsigned ime_3dnr_nr_freq_w1        : 8;        // bits : 15_8
			unsigned ime_3dnr_nr_freq_w2        : 8;        // bits : 23_16
			unsigned ime_3dnr_nr_freq_w3        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_536; // 0x0860

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_537; // 0x0864

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_538; // 0x0868

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_539; // 0x086c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_540; // 0x0870

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_541; // 0x0874

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_542; // 0x0878

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_543; // 0x087c

	union {
		struct {
			unsigned ime_3dnr_nr_luma_w0        : 8;        // bits : 7_0
			unsigned ime_3dnr_nr_luma_w1        : 8;        // bits : 15_8
			unsigned ime_3dnr_nr_luma_w2        : 8;        // bits : 23_16
			unsigned ime_3dnr_nr_luma_w3        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_544; // 0x0880

	union {
		struct {
			unsigned ime_3dnr_nr_luma_w4        : 8;        // bits : 7_0
			unsigned ime_3dnr_nr_luma_w5        : 8;        // bits : 15_8
			unsigned ime_3dnr_nr_luma_w6        : 8;        // bits : 23_16
			unsigned ime_3dnr_nr_luma_w7        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_545; // 0x0884

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_546; // 0x0888

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_547; // 0x088c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_548; // 0x0890

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_549; // 0x0894

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_550; // 0x0898

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_551; // 0x089c

	union {
		struct {
			unsigned ime_3dnr_nr_pre_filtering_str0        : 8;     // bits : 7_0
			unsigned ime_3dnr_nr_pre_filtering_str1        : 8;     // bits : 15_8
			unsigned ime_3dnr_nr_pre_filtering_str2        : 8;     // bits : 23_16
			unsigned ime_3dnr_nr_pre_filtering_str3        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_552; // 0x08a0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_553; // 0x08a4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_554; // 0x08a8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_555; // 0x08ac

	union {
		struct {
			unsigned ime_3dnr_nr_pre_filtering_ratio0        : 8;       // bits : 7_0
			unsigned ime_3dnr_nr_pre_filtering_ratio1        : 8;       // bits : 15_8
			unsigned ime_3dnr_nr_snr_str0                    : 8;       // bits : 23_16
			unsigned ime_3dnr_nr_snr_str1                    : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_556; // 0x08b0

	union {
		struct {
			unsigned ime_3dnr_nr_snr_str2        : 8;       // bits : 7_0
			unsigned ime_3dnr_nr_tnr_str0        : 8;       // bits : 15_8
			unsigned ime_3dnr_nr_tnr_str1        : 8;       // bits : 23_16
			unsigned ime_3dnr_nr_tnr_str2        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_557; // 0x08b4

	union {
		struct {
			unsigned ime_3dnr_nr_base_th_snr        : 16;       // bits : 15_0
			unsigned ime_3dnr_nr_base_th_tnr        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} reg_558; // 0x08b8

	union {
		struct {
			unsigned ime_3dnr_nr_y_3d_ratio0          : 8;      // bits : 7_0
			unsigned ime_3dnr_nr_y_3d_ratio1          : 8;      // bits : 15_8
			unsigned ime_3dnr_nr_luma_comp_str        : 8;      // bits : 23_16
		} bit;
		UINT32 word;
	} reg_559; // 0x08bc

	union {
		struct {
			unsigned ime_3dnr_nr_y_3d_lut0        : 7;      // bits : 6_0
			unsigned                              : 1;
			unsigned ime_3dnr_nr_y_3d_lut1        : 7;      // bits : 14_8
			unsigned                              : 1;
			unsigned ime_3dnr_nr_y_3d_lut2        : 7;      // bits : 22_16
			unsigned                              : 1;
			unsigned ime_3dnr_nr_y_3d_lut3        : 7;      // bits : 30_24
		} bit;
		UINT32 word;
	} reg_560; // 0x08c0

	union {
		struct {
			unsigned ime_3dnr_nr_y_3d_lut4        : 7;      // bits : 6_0
			unsigned                              : 1;
			unsigned ime_3dnr_nr_y_3d_lut5        : 7;      // bits : 14_8
			unsigned                              : 1;
			unsigned ime_3dnr_nr_y_3d_lut6        : 7;      // bits : 22_16
			unsigned                              : 1;
			unsigned ime_3dnr_nr_y_3d_lut7        : 7;      // bits : 30_24
		} bit;
		UINT32 word;
	} reg_561; // 0x08c4

	union {
		struct {
			unsigned ime_3dnr_nr_c_3d_lut0        : 7;      // bits : 6_0
			unsigned                              : 1;
			unsigned ime_3dnr_nr_c_3d_lut1        : 7;      // bits : 14_8
			unsigned                              : 1;
			unsigned ime_3dnr_nr_c_3d_lut2        : 7;      // bits : 22_16
			unsigned                              : 1;
			unsigned ime_3dnr_nr_c_3d_lut3        : 7;      // bits : 30_24
		} bit;
		UINT32 word;
	} reg_562; // 0x08c8

	union {
		struct {
			unsigned ime_3dnr_nr_c_3d_lut4        : 7;      // bits : 6_0
			unsigned                              : 1;
			unsigned ime_3dnr_nr_c_3d_lut5        : 7;      // bits : 14_8
			unsigned                              : 1;
			unsigned ime_3dnr_nr_c_3d_lut6        : 7;      // bits : 22_16
			unsigned                              : 1;
			unsigned ime_3dnr_nr_c_3d_lut7        : 7;      // bits : 30_24
		} bit;
		UINT32 word;
	} reg_563; // 0x08cc

	union {
		struct {
			unsigned ime_3dnr_nr_c_3d_ratio0        : 8;        // bits : 7_0
			unsigned ime_3dnr_nr_c_3d_ratio1        : 8;        // bits : 15_8
		} bit;
		UINT32 word;
	} reg_564; // 0x08d0

	union {
		struct {
			unsigned ime_3dnr_ref_in_st_hinit_y_ofs        : 20;        // bits : 19_0
		} bit;
		UINT32 word;
	} reg_565; // 0x08d4

	union {
		struct {
			unsigned ime_3dnr_ref_out_st_hinit_y_ofs        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_566; // 0x08d8

	union {
		struct {
			unsigned ime_3dnr_mot_in_st_hinit_ofs        : 20;      // bits : 19_0
		} bit;
		UINT32 word;
	} reg_567; // 0x08dc

	union {
		struct {
			unsigned ime_3dnr_mot_out_st_hinit_ofs        : 20;     // bits : 19_0
		} bit;
		UINT32 word;
	} reg_568; // 0x08e0

	union {
		struct {
			unsigned ime_3dnr_mot_roi_out_st_hinit_ofs        : 20;     // bits : 19_0
		} bit;
		UINT32 word;
	} reg_569; // 0x08e4

	union {
		struct {
			unsigned ime_3dnr_mv_in_st_hinit_ofs        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_570; // 0x08e8

	union {
		struct {
			unsigned ime_3dnr_mv_out_st_hinit_ofs        : 20;      // bits : 19_0
		} bit;
		UINT32 word;
	} reg_571; // 0x08ec

	union {
		struct {
			unsigned ime_3dnr_statistic_out_st_hinit_ofs        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_572; // 0x08f0

	union {
		struct {
			unsigned ime_3dnr_fast_converge_in_st_hinit_ofs        : 20;        // bits : 19_0
		} bit;
		UINT32 word;
	} reg_573; // 0x08f4

	union {
		struct {
			unsigned ime_3dnr_fast_converge_out_st_hinit_ofs        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_574; // 0x08f8

	union {
		struct {
			unsigned ime_3dnr_ref_in_fmt        : 3;        // bits : 2_0
		} bit;
		UINT32 word;
	} reg_575; // 0x08fc

	union {
		struct {
			unsigned                                 : 2;
			unsigned ime_3dnr_ref_dram_y_ofsi        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_576; // 0x0900

	union {
		struct {
			unsigned                                  : 2;
			unsigned ime_3dnr_ref_dram_uv_ofsi        : 18;     // bits : 19_2
		} bit;
		UINT32 word;
	} reg_577; // 0x0904

	union {
		struct {
			unsigned                                : 2;
			unsigned ime_3dnr_ref_dram_y_sai        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} reg_578; // 0x0908

	union {
		struct {
			unsigned                                 : 2;
			unsigned ime_3dnr_ref_dram_uv_sai        : 30;      // bits : 31_2
		} bit;
		UINT32 word;
	} reg_579; // 0x090c

	union {
		struct {
			unsigned                                 : 2;
			unsigned ime_3dnr_ref_dram_y_ofso        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_580; // 0x0910

	union {
		struct {
			unsigned                                  : 2;
			unsigned ime_3dnr_ref_dram_uv_ofso        : 18;     // bits : 19_2
		} bit;
		UINT32 word;
	} reg_581; // 0x0914

	union {
		struct {
			unsigned                                : 2;
			unsigned ime_3dnr_ref_dram_y_sao        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} reg_582; // 0x0918

	union {
		struct {
			unsigned                                 : 2;
			unsigned ime_3dnr_ref_dram_uv_sao        : 30;      // bits : 31_2
		} bit;
		UINT32 word;
	} reg_583; // 0x091c

	union {
		struct {
			unsigned                              : 2;
			unsigned ime_3dnr_mot_dram_ofs        : 18;     // bits : 19_2
		} bit;
		UINT32 word;
	} reg_584; // 0x0920

	union {
		struct {
			unsigned                              : 2;
			unsigned ime_3dnr_mot_dram_sai        : 30;     // bits : 31_2
		} bit;
		UINT32 word;
	} reg_585; // 0x0924

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_586; // 0x0928

	union {
		struct {
			unsigned                              : 2;
			unsigned ime_3dnr_mot_dram_sao        : 30;     // bits : 31_2
		} bit;
		UINT32 word;
	} reg_587; // 0x092c

	union {
		struct {
			unsigned                                   : 2;
			unsigned ime_3dnr_mot_roi_dram_ofso        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_588; // 0x0930

	union {
		struct {
			unsigned                                  : 2;
			unsigned ime_3dnr_mot_roi_dram_sao        : 30;     // bits : 31_2
		} bit;
		UINT32 word;
	} reg_589; // 0x0934

	union {
		struct {
			unsigned                             : 2;
			unsigned ime_3dnr_mv_dram_ofs        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_590; // 0x0938

	union {
		struct {
			unsigned                             : 2;
			unsigned ime_3dnr_mv_dram_sai        : 30;      // bits : 31_2
		} bit;
		UINT32 word;
	} reg_591; // 0x093c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_592; // 0x0940

	union {
		struct {
			unsigned                             : 2;
			unsigned ime_3dnr_mv_dram_sao        : 30;      // bits : 31_2
		} bit;
		UINT32 word;
	} reg_593; // 0x0944

	union {
		struct {
			unsigned                                     : 2;
			unsigned ime_3dnr_statistic_dram_ofso        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_594; // 0x0948

	union {
		struct {
			unsigned                                    : 2;
			unsigned ime_3dnr_statistic_dram_sao        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} reg_595; // 0x094c

	union {
		struct {
			unsigned                                        : 2;
			unsigned ime_3dnr_fast_converge_dram_sai        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} reg_596; // 0x0950

	union {
		struct {
			unsigned                                        : 2;
			unsigned ime_3dnr_fast_converge_dram_ofs        : 18;       // bits : 19_2
		} bit;
		UINT32 word;
	} reg_597; // 0x0954

	union {
		struct {
			unsigned                                        : 2;
			unsigned ime_3dnr_fast_converge_dram_sao        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} reg_598; // 0x0958

	union {
		struct {
			unsigned ime_3dnr_ref_dram_y_msb_sai        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_599; // 0x095c

	union {
		struct {
			unsigned ime_3dnr_ref_dram_uv_msb_sai        : 4;       // bits : 3_0
		} bit;
		UINT32 word;
	} reg_600; // 0x0960

	union {
		struct {
			unsigned ime_3dnr_ref_dram_y_msb_sao        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_601; // 0x0964

	union {
		struct {
			unsigned ime_3dnr_ref_dram_uv_msb_sao        : 4;       // bits : 3_0
		} bit;
		UINT32 word;
	} reg_602; // 0x0968

	union {
		struct {
			unsigned ime_3dnr_mot_dram_msb_sai        : 4;      // bits : 3_0
		} bit;
		UINT32 word;
	} reg_603; // 0x096c

	union {
		struct {
			unsigned ime_3dnr_mot_dram_msb_sao        : 4;      // bits : 3_0
		} bit;
		UINT32 word;
	} reg_604; // 0x0970

	union {
		struct {
			unsigned ime_3dnr_mot_roi_dram_msb_sao        : 4;      // bits : 3_0
		} bit;
		UINT32 word;
	} reg_605; // 0x0974

	union {
		struct {
			unsigned ime_3dnr_mv_dram_msb_sai        : 4;       // bits : 3_0
		} bit;
		UINT32 word;
	} reg_606; // 0x0978

	union {
		struct {
			unsigned ime_3dnr_mv_dram_msb_sao        : 4;       // bits : 3_0
		} bit;
		UINT32 word;
	} reg_607; // 0x097c

	union {
		struct {
			unsigned ime_3dnr_statistic_dram_msb_sao        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_608; // 0x0980

	union {
		struct {
			unsigned ime_3dnr_fast_converge_dram_msb_sai        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_609; // 0x0984

	union {
		struct {
			unsigned ime_3dnr_fast_converge_dram_msb_sao        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_610; // 0x0988

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_611; // 0x098c

	union {
		struct {
			//unsigned ime_3dnr_ref_out_crop_x        : 16;      // bits : 15_0
			//unsigned ime_3dnr_ref_out_crop_y        : 16;      // bits : 31_16
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_612; // 0x0990

	union {
		struct {
			//unsigned ime_3dnr_ref_out_crop_size_h        : 16;      // bits : 15_0
			//unsigned ime_3dnr_ref_out_crop_size_v        : 16;      // bits : 31_16
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_613; // 0x0994

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_614; // 0x0998

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_615; // 0x099c

	union {
		struct {
			unsigned ime_in_st_hinit_uv_ofs        : 20;        // bits : 19_0
		} bit;
		UINT32 word;
	} reg_616; // 0x09a0

	union {
		struct {
			unsigned ime_out_st_hinit0_uv_ofs_p1        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_617; // 0x09a4

	union {
		struct {
			unsigned ime_out_st_hinit0_uv_ofs_p2        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_618; // 0x09a8

	union {
		struct {
			unsigned ime_out_st_hinit0_uv_ofs_p3        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_619; // 0x09ac

	union {
		struct {
			unsigned ime_out_st_hinit0_uv_ofs_p4        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_620; // 0x09b0

	union {
		struct {
			unsigned ime_3dnr_ref_in_st_hinit_uv_ofs        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_621; // 0x09b4

	union {
		struct {
			unsigned ime_3dnr_ref_out_st_hinit_uv_ofs        : 20;      // bits : 19_0
		} bit;
		UINT32 word;
	} reg_622; // 0x09b8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_623; // 0x09bc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_624; // 0x09c0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_625; // 0x09c4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_626; // 0x09c8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_627; // 0x09cc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_628; // 0x09d0

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_629; // 0x09d4

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_630; // 0x09d8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_631; // 0x09dc

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_632; // 0x09e0

	union {
		struct {
			unsigned ime_3dnr_ref_out_min_y         : 8;        // bits : 7_0
			unsigned ime_3dnr_ref_out_max_y         : 8;        // bits : 15_8
			unsigned ime_3dnr_ref_out_min_uv        : 8;        // bits : 23_16
			unsigned ime_3dnr_ref_out_max_uv        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} reg_633; // 0x09e4

	union {
		struct {
			unsigned ime_axi_channel0_en        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_634; // 0x09e8

	union {
		struct {
			unsigned ime_axi_channel32_en   : 6;      // bits : 5_0
			unsigned                        : 25;      // bits : 30_6
			unsigned ime_axi_disable        : 1;      // bits : 5_0
		} bit;
		UINT32 word;
	} reg_635; // 0x09ec

	union {
		struct {
			unsigned ime_r_ostd_num         : 8;        // bits : 7_0
			unsigned ime_w_ostd_num         : 8;        // bits : 15_8
			unsigned                        : 15;
			unsigned ime_axi_ch_idle        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_636; // 0x09f0

	union {
		struct {
			unsigned ime_axi_lock0_dis         : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_637; // 0x09f4

	union {
		struct {
			unsigned ime_axi_lock32_dis        : 6;      // bits : 5_0
		} bit;
		UINT32 word;
	} reg_638; // 0x09f8

	union {
		struct {
			unsigned ime_axi_ch_sta        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} reg_639; // 0x09fc

	union {
		struct {
			unsigned ime_p0_enc_smode_en                    : 1;        // bits : 0
			unsigned                                        : 3;
			unsigned ime_3dnr_refout_enc_smode_en           : 1;        // bits : 4
			unsigned                                        : 3;
			unsigned ime_3dnr_refin_dec_smode_en            : 1;        // bits : 8
			unsigned                                        : 1;
			unsigned ime_3dnr_refin_dec_dither_en           : 1;        // bits : 10
			unsigned                                        : 5;
			unsigned ime_3dnr_refin_dec_dither_seed0        : 15;       // bits : 30_16
			unsigned ime_3dnr_refin_dec_dither_reset        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_640; // 0x0a00

	union {
		struct {
			unsigned ime_3dnr_refin_dec_dither_seed1        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_641; // 0x0a04

	union {
		struct {
			unsigned dct_level_th0        : 8;      // bits : 7_0
			unsigned dct_level_th1        : 8;      // bits : 15_8
			unsigned dct_level_th2        : 8;      // bits : 23_16
			unsigned dct_level_th3        : 8;      // bits : 31_24
		} bit;
		UINT32 word;
	} reg_642; // 0x0a08

	union {
		struct {
			unsigned dct_level_th4        : 8;      // bits : 7_0
			unsigned dct_level_th5        : 8;      // bits : 15_8
			unsigned dct_level_th6        : 8;      // bits : 23_16
			unsigned dct_level_th7        : 8;      // bits : 31_24
		} bit;
		UINT32 word;
	} reg_643; // 0x0a0c

	union {
		struct {
			unsigned dct_qtbl0_idx        : 5;      // bits : 4_0
			unsigned                      : 3;
			unsigned dct_qtbl1_idx        : 5;      // bits : 12_8
			unsigned                      : 3;
			unsigned dct_qtbl2_idx        : 5;      // bits : 20_16
			unsigned                      : 3;
			unsigned dct_qtbl3_idx        : 5;      // bits : 28_24
		} bit;
		UINT32 word;
	} reg_644; // 0x0a10

	union {
		struct {
			unsigned dct_qtbl4_idx        : 5;      // bits : 4_0
			unsigned                      : 3;
			unsigned dct_qtbl5_idx        : 5;      // bits : 12_8
			unsigned                      : 3;
			unsigned dct_qtbl6_idx        : 5;      // bits : 20_16
			unsigned                      : 3;
			unsigned dct_qtbl7_idx        : 5;      // bits : 28_24
		} bit;
		UINT32 word;
	} reg_645; // 0x0a14

	union {
		struct {
			unsigned dct_qtbl8_idx         : 5;     // bits : 4_0
			unsigned                       : 3;
			unsigned dct_qtbl9_idx         : 5;     // bits : 12_8
			unsigned                       : 3;
			unsigned dct_qtbl10_idx        : 5;     // bits : 20_16
			unsigned                       : 3;
			unsigned dct_qtbl11_idx        : 5;     // bits : 28_24
		} bit;
		UINT32 word;
	} reg_646; // 0x0a18

	union {
		struct {
			unsigned dct_qtbl12_idx        : 5;     // bits : 4_0
			unsigned                       : 3;
			unsigned dct_qtbl13_idx        : 5;     // bits : 12_8
			unsigned                       : 3;
			unsigned dct_qtbl14_idx        : 5;     // bits : 20_16
			unsigned                       : 3;
			unsigned dct_qtbl15_idx        : 5;     // bits : 28_24
		} bit;
		UINT32 word;
	} reg_647; // 0x0a1c

	union {
		struct {
			unsigned yrc_lncnt_lfn0        : 13;        // bits : 12_0
			unsigned                       : 3;
			unsigned yrc_lncnt_lfn1        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} reg_648; // 0x0a20

	union {
		struct {
			unsigned yrc_lncnt_lfn2        : 13;        // bits : 12_0
			unsigned                       : 3;
			unsigned yrc_lncnt_lfn3        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} reg_649; // 0x0a24

	union {
		struct {
			unsigned yrc_lncnt_lfn4        : 13;        // bits : 12_0
			unsigned                       : 3;
			unsigned yrc_lncnt_lfn5        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} reg_650; // 0x0a28

	union {
		struct {
			unsigned dct_maxdist        : 8;        // bits : 7_0
		} bit;
		UINT32 word;
	} reg_651; // 0x0a2c

	union {
		struct {
			unsigned                                : 8;
			unsigned ime_in_dec_smode_en            : 1;        // bits : 8
			unsigned                                : 1;
			unsigned ime_in_dec_dither_en           : 1;        // bits : 10
			unsigned                                : 5;
			unsigned ime_in_dec_dither_seed0        : 15;       // bits : 30_16
			unsigned ime_in_dec_dither_reset        : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_652; // 0x0a30

	union {
		struct {
			unsigned ime_in_dec_dither_seed1        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_653; // 0x0a34

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_654; // 0x0a38

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_655; // 0x0a3c

	union {
		struct {
			unsigned ime_dual_start_offset        : 16;      // bits : 15_0
			unsigned                              : 15;
			unsigned ime_va_force_write_out       : 1;       // bits: 31
		} bit;
		UINT32 word;
	} reg_656; // 0x0a40

	union {
		struct {
			unsigned ime_dual_left_overlap         : 16;      // bits : 15_0
			unsigned ime_dual_right_overlap        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} reg_657; // 0x0a44

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_658; // 0x0a48

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_659; // 0x0a4c

	union {
		struct {
			unsigned ime_va_out_st_hinit_ofs        : 20;       // bits : 19_0
		} bit;
		UINT32 word;
	} reg_660; // 0x0a50

	union {
		struct {
			unsigned                         : 2;
			unsigned ime_va_dram_ofso        : 18;      // bits : 19_2
		} bit;
		UINT32 word;
	} reg_661; // 0x0a54

	union {
		struct {
			unsigned                        : 2;
			unsigned ime_va_dram_sao        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} reg_662; // 0x0a58

	union {
		struct {
			unsigned ime_va_dram_msb_sao        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} reg_663; // 0x0a5c

	union {
		struct {
			unsigned ime_vdet_gh1_a              : 5;       // bits : 4_0
			unsigned                           : 2;
			unsigned ime_vdet_gh1_bcd_op        : 1;        // bits : 7
			unsigned ime_vdet_gh1_b              : 5;       // bits : 12_8
			unsigned                           : 3;
			unsigned ime_vdet_gh1_c              : 4;       // bits : 19_16
			unsigned ime_vdet_gh1_d              : 4;       // bits : 23_20
			unsigned ime_vdet_gh1_fsize         : 2;        // bits : 25_24
			unsigned                           : 2;
			unsigned ime_vdet_gh1_div           : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_664; // 0x0a60

	union {
		struct {
			unsigned ime_vdet_gv1_a          : 5;       // bits : 4_0
			unsigned                           : 2;
			unsigned ime_vdet_gv1_bcd_op        : 1;        // bits : 7
			unsigned ime_vdet_gv1_b              : 5;       // bits : 12_8
			unsigned                           : 3;
			unsigned ime_vdet_gv1_c              : 4;       // bits : 19_16
			unsigned ime_vdet_gv1_d              : 4;       // bits : 23_20
			unsigned ime_vdet_gv1_fsize         : 2;        // bits : 25_24
			unsigned                           : 2;
			unsigned ime_vdet_gv1_div           : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_665; // 0x0a64

	union {
		struct {
			unsigned ime_vdet_gh2_a              : 5;       // bits : 4_0
			unsigned                           : 2;
			unsigned ime_vdet_gh2_bcd_op        : 1;        // bits : 7
			unsigned ime_vdet_gh2_b              : 5;       // bits : 12_8
			unsigned                           : 3;
			unsigned ime_vdet_gh2_c              : 4;       // bits : 19_16
			unsigned ime_vdet_gh2_d              : 4;       // bits : 23_20
			unsigned ime_vdet_gh2_fsize         : 2;        // bits : 25_24
			unsigned                           : 2;
			unsigned ime_vdet_gh2_div           : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_666; // 0x0a68

	union {
		struct {
			unsigned ime_vdet_gv2_a              : 5;       // bits : 4_0
			unsigned                           : 2;
			unsigned ime_vdet_gv2_bcd_op        : 1;        // bits : 7
			unsigned ime_vdet_gv2_b              : 5;       // bits : 12_8
			unsigned                           : 3;
			unsigned ime_vdet_gv2_c              : 4;       // bits : 19_16
			unsigned ime_vdet_gv2_d              : 4;       // bits : 23_20
			unsigned ime_vdet_gv2_fsize         : 2;        // bits : 25_24
			unsigned                           : 2;
			unsigned ime_vdet_gv2_div           : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_667; // 0x0a6c

	union {
		struct {
			unsigned ime_va_out_sel     : 1;        // bits : 0
			unsigned                    : 2;
			unsigned ime_va_stx         : 14;       // bits : 16_3
			unsigned ime_va_sty         : 14;       // bits : 30_17
		} bit;
		UINT32 word;
	} reg_668; // 0x0a70

	union {
		struct {
			unsigned ime_va_g1h_thl        : 8;     // bits : 7_0
			unsigned ime_va_g1h_thh        : 8;     // bits : 15_8
			unsigned ime_va_g1v_thl        : 8;     // bits : 23_16
			unsigned ime_va_g1v_thh        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_669; // 0x0a74

	union {
		struct {
			unsigned ime_va_g2h_thl        : 8;     // bits : 7_0
			unsigned ime_va_g2h_thh        : 8;     // bits : 15_8
			unsigned ime_va_g2v_thl        : 8;     // bits : 23_16
			unsigned ime_va_g2v_thh        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_670; // 0x0a78

	union {
		struct {
			unsigned ime_va_win_szx          : 10;      // bits : 9_0
			unsigned                         : 2;
			unsigned ime_va_win_szy          : 10;      // bits : 21_12
			unsigned                         : 8;
			unsigned ime_va_g1_cnt_en        : 1;       // bits : 30
			unsigned ime_va_g2_cnt_en        : 1;       // bits : 31
		} bit;
		UINT32 word;
	} reg_671; // 0x0a7c

	union {
		struct {
			unsigned ime_va_win_numx         : 4;       // bits : 3_0
			unsigned ime_va_win_numy         : 4;       // bits : 7_4
			unsigned                         : 8;
			unsigned ime_va_win_skipx        : 6;       // bits : 21_16
			unsigned                         : 2;
			unsigned ime_va_win_skipy        : 6;       // bits : 29_24
		} bit;
		UINT32 word;
	} reg_672; // 0x0a80

	union {
		struct {
			unsigned ime_va_win0_stx        : 14;       // bits : 13_0
			unsigned                        : 2;
			unsigned ime_va_win0_sty        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_673; // 0x0a84

	union {
		struct {
			unsigned ime_va_win0_hsz        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned ime_va_win0_vsz        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_674; // 0x0a88

	union {
		struct {
			unsigned ime_va_win1_stx        : 14;       // bits : 13_0
			unsigned                        : 2;
			unsigned ime_va_win1_sty        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_675; // 0x0a8c

	union {
		struct {
			unsigned ime_va_win1_hsz        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned ime_va_win1_vsz        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_676; // 0x0a90

	union {
		struct {
			unsigned ime_va_win2_stx        : 14;       // bits : 13_0
			unsigned                        : 2;
			unsigned ime_va_win2_sty        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_677; // 0x0a94

	union {
		struct {
			unsigned ime_va_win2_hsz        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned ime_va_win2_vsz        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_678; // 0x0a98

	union {
		struct {
			unsigned ime_va_win3_stx        : 14;       // bits : 13_0
			unsigned                        : 2;
			unsigned ime_va_win3_sty        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_679; // 0x0a9c

	union {
		struct {
			unsigned ime_va_win3_hsz        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned ime_va_win3_vsz        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_680; // 0x0aa0

	union {
		struct {
			unsigned ime_va_win4_stx        : 14;       // bits : 13_0
			unsigned                        : 2;
			unsigned ime_va_win4_sty        : 14;       // bits : 29_16
		} bit;
		UINT32 word;
	} reg_681; // 0x0aa4

	union {
		struct {
			unsigned ime_va_win4_hsz        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned ime_va_win4_vsz        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} reg_682; // 0x0aa8

	union {
		struct {
			unsigned ime_va_win_cnt_out_sel         : 1;        // bits : 0
			unsigned                                : 3;
			unsigned ime_va_win_high_luma_th        : 8;        // bits : 11_4
			unsigned ime_va_energy_w                : 5;        // bits : 16_12
		} bit;
		UINT32 word;
	} reg_683; // 0x0aac

	union {
		struct {
			unsigned ime_va_win0g1h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_684; // 0x0ab0

	union {
		struct {
			unsigned ime_va_win0g1h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win0g1h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_685; // 0x0ab4

	union {
		struct {
			unsigned ime_va_win0g1v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_686; // 0x0ab8

	union {
		struct {
			unsigned ime_va_win0g1v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win0g1v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_687; // 0x0abc

	union {
		struct {
			unsigned ime_va_win0g2h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_688; // 0x0ac0

	union {
		struct {
			unsigned ime_va_win0g2h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win0g2h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_689; // 0x0ac4

	union {
		struct {
			unsigned ime_va_win0g2v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_690; // 0x0ac8

	union {
		struct {
			unsigned ime_va_win0g2v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win0g2v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_691; // 0x0acc

	union {
		struct {
			unsigned ime_va_win1g1h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_692; // 0x0ad0

	union {
		struct {
			unsigned ime_va_win1g1h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win1g1h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_693; // 0x0ad4

	union {
		struct {
			unsigned ime_va_win1g1v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_694; // 0x0ad8

	union {
		struct {
			unsigned ime_va_win1g1v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win1g1v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_695; // 0x0adc

	union {
		struct {
			unsigned ime_va_win1g2h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_696; // 0x0ae0

	union {
		struct {
			unsigned ime_va_win1g2h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win1g2h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_697; // 0x0ae4

	union {
		struct {
			unsigned ime_va_win1g2v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_698; // 0x0ae8

	union {
		struct {
			unsigned ime_va_win1g2v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win1g2v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_699; // 0x0aec

	union {
		struct {
			unsigned ime_va_win2g1h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_700; // 0x0af0

	union {
		struct {
			unsigned ime_va_win2g1h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win2g1h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_701; // 0x0af4

	union {
		struct {
			unsigned ime_va_win2g1v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_702; // 0x0af8

	union {
		struct {
			unsigned ime_va_win2g1v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win2g1v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_703; // 0x0afc

	union {
		struct {
			unsigned ime_va_win2g2h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_704; // 0x0b00

	union {
		struct {
			unsigned ime_va_win2g2h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win2g2h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_705; // 0x0b04

	union {
		struct {
			unsigned ime_va_win2g2v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_706; // 0x0b08

	union {
		struct {
			unsigned ime_va_win2g2v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win2g2v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_707; // 0x0b0c

	union {
		struct {
			unsigned ime_va_win3g1h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_708; // 0x0b10

	union {
		struct {
			unsigned ime_va_win3g1h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win3g1h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_709; // 0x0b14

	union {
		struct {
			unsigned ime_va_win3g1v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_710; // 0x0b18

	union {
		struct {
			unsigned ime_va_win3g1v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win3g1v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_711; // 0x0b1c

	union {
		struct {
			unsigned ime_va_win3g2h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_712; // 0x0b20

	union {
		struct {
			unsigned ime_va_win3g2h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win3g2h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_713; // 0x0b24

	union {
		struct {
			unsigned ime_va_win3g2v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_714; // 0x0b28

	union {
		struct {
			unsigned ime_va_win3g2v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win3g2v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_715; // 0x0b2c

	union {
		struct {
			unsigned ime_va_win4g1h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_716; // 0x0b30

	union {
		struct {
			unsigned ime_va_win4g1h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win4g1h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_717; // 0x0b34

	union {
		struct {
			unsigned ime_va_win4g1v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_718; // 0x0b38

	union {
		struct {
			unsigned ime_va_win4g1v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win4g1v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_719; // 0x0b3c

	union {
		struct {
			unsigned ime_va_win4g2h_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_720; // 0x0b40

	union {
		struct {
			unsigned ime_va_win4g2h_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win4g2h_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_721; // 0x0b44

	union {
		struct {
			unsigned ime_va_win4g2v_vacc        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_722; // 0x0b48

	union {
		struct {
			unsigned ime_va_win4g2v_vacnt           : 20;       // bits : 19_0
			unsigned                                : 8;
			unsigned ime_va_win4g2v_vacc_msb        : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_723; // 0x0b4c

	union {
		struct {
			unsigned ime_va_win_ldg_en                : 1;      // bits : 0
			unsigned                                  : 3;
			unsigned ime_va_win_ldg_low_slope         : 4;      // bits : 7_4
			unsigned ime_va_win_ldg_high_slope        : 4;      // bits : 11_8
		} bit;
		UINT32 word;
	} reg_724; // 0x0b50

	union {
		struct {
			unsigned ime_va_win_ldg_low_th           : 8;       // bits : 7_0
			unsigned ime_va_win_ldg_high_th          : 8;       // bits : 15_8
			unsigned ime_va_win_ldg_low_gain         : 8;       // bits : 23_16
			unsigned ime_va_win_ldg_high_gain        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_725; // 0x0b54

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_726; // 0x0b58

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_727; // 0x0b5c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_728; // 0x0b60

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_729; // 0x0b64

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_730; // 0x0b68

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_731; // 0x0b6c

	union {
		struct {
			unsigned lca_proc_lcon_sel         : 1;     // bits : 0
			unsigned lca_edge_ker_size         : 1;     // bits : 1
			unsigned                           : 2;
			unsigned lca_sr_rf_ker_size        : 3;     // bits : 6_4
			unsigned                           : 1;
			unsigned lca_mr_rf_ker_size        : 3;     // bits : 10_8
		} bit;
		UINT32 word;
	} reg_732; // 0x0b70

	union {
		struct {
			unsigned lca_ctr_y_th0        : 8;      // bits : 7_0
			unsigned lca_ctr_y_th1        : 8;      // bits : 15_8
			unsigned lca_ctr_y_th2        : 8;      // bits : 23_16
		} bit;
		UINT32 word;
	} reg_733; // 0x0b74

	union {
		struct {
			unsigned lca_ctr_u_th0        : 8;      // bits : 7_0
			unsigned lca_ctr_u_th1        : 8;      // bits : 15_8
			unsigned lca_ctr_u_th2        : 8;      // bits : 23_16
		} bit;
		UINT32 word;
	} reg_734; // 0x0b78

	union {
		struct {
			unsigned lca_ctr_v_th0        : 8;      // bits : 7_0
			unsigned lca_ctr_v_th1        : 8;      // bits : 15_8
			unsigned lca_ctr_v_th2        : 8;      // bits : 23_16
		} bit;
		UINT32 word;
	} reg_735; // 0x0b7c

	union {
		struct {
			unsigned lca_ref_wet_smooth_y         : 6;      // bits : 5_0
			unsigned                              : 2;
			unsigned lca_ref_wet_edge_y           : 6;      // bits : 13_8
			unsigned                              : 2;
			unsigned lca_ref_wet_smooth_uv        : 6;      // bits : 21_16
			unsigned                              : 2;
			unsigned lca_ref_wet_edge_uv          : 6;      // bits : 29_24
		} bit;
		UINT32 word;
	} reg_736; // 0x0b80

	union {
		struct {
			unsigned lca_still_smooth_y_th0        : 8;     // bits : 7_0
			unsigned lca_still_smooth_y_th1        : 8;     // bits : 15_8
			unsigned lca_still_smooth_y_th2        : 8;     // bits : 23_16
			unsigned lca_still_smooth_y_th3        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} reg_737; // 0x0b84

	union {
		struct {
			unsigned lca_still_smooth_y_th4        : 8;     // bits : 7_0
		} bit;
		UINT32 word;
	} reg_738; // 0x0b88

	union {
		struct {
			unsigned lca_still_smooth_uv_th0        : 16;       // bits : 15_0
			unsigned lca_still_smooth_uv_th1        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} reg_739; // 0x0b8c

	union {
		struct {
			unsigned lca_still_smooth_uv_th2        : 16;       // bits : 15_0
			unsigned lca_still_smooth_uv_th3        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} reg_740; // 0x0b90

	union {
		struct {
			unsigned lca_still_smooth_uv_th4        : 16;       // bits : 15_0
		} bit;
		UINT32 word;
	} reg_741; // 0x0b94

	union {
		struct {
			unsigned lca_still_edge_y_th0        : 8;       // bits : 7_0
			unsigned lca_still_edge_y_th1        : 8;       // bits : 15_8
			unsigned lca_still_edge_y_th2        : 8;       // bits : 23_16
			unsigned lca_still_edge_y_th3        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_742; // 0x0b98

	union {
		struct {
			unsigned lca_still_edge_y_th4        : 8;       // bits : 7_0
		} bit;
		UINT32 word;
	} reg_743; // 0x0b9c

	union {
		struct {
			unsigned lca_still_edge_uv_th0        : 16;     // bits : 15_0
			unsigned lca_still_edge_uv_th1        : 16;     // bits : 31_16
		} bit;
		UINT32 word;
	} reg_744; // 0x0ba0

	union {
		struct {
			unsigned lca_still_edge_uv_th2        : 16;     // bits : 15_0
			unsigned lca_still_edge_uv_th3        : 16;     // bits : 31_16
		} bit;
		UINT32 word;
	} reg_745; // 0x0ba4

	union {
		struct {
			unsigned lca_still_edge_uv_th4        : 16;     // bits : 15_0
		} bit;
		UINT32 word;
	} reg_746; // 0x0ba8

	union {
		struct {
			unsigned lca_motion_y_th0        : 8;       // bits : 7_0
			unsigned lca_motion_y_th1        : 8;       // bits : 15_8
			unsigned lca_motion_y_th2        : 8;       // bits : 23_16
			unsigned lca_motion_y_th3        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} reg_747; // 0x0bac

	union {
		struct {
			unsigned lca_motion_y_th4        : 8;       // bits : 7_0
		} bit;
		UINT32 word;
	} reg_748; // 0x0bb0

	union {
		struct {
			unsigned lca_motion_uv_th0        : 16;     // bits : 15_0
			unsigned lca_motion_uv_th1        : 16;     // bits : 31_16
		} bit;
		UINT32 word;
	} reg_749; // 0x0bb4

	union {
		struct {
			unsigned lca_motion_uv_th2        : 16;     // bits : 15_0
			unsigned lca_motion_uv_th3        : 16;     // bits : 31_16
		} bit;
		UINT32 word;
	} reg_750; // 0x0bb8

	union {
		struct {
			unsigned lca_motion_uv_th4        : 16;     // bits : 15_0
			unsigned                          : 4;
			unsigned lca_out_wet_y            : 6;      // bits : 25_20
			unsigned lca_out_wet_uv           : 6;      // bits : 31_26
		} bit;
		UINT32 word;
	} reg_751; // 0x0bbc

	union {
		struct {
			unsigned lca_still_y_gain0        : 6;      // bits : 5_0
			unsigned                          : 2;
			unsigned lca_still_y_gain1        : 6;      // bits : 13_8
			unsigned                          : 2;
			unsigned lca_still_y_gain2        : 6;      // bits : 21_16
		} bit;
		UINT32 word;
	} reg_752; // 0x0bc0

	union {
		struct {
			unsigned lca_motion_y_gain0        : 6;     // bits : 5_0
			unsigned                           : 2;
			unsigned lca_motion_y_gain1        : 6;     // bits : 13_8
			unsigned                           : 2;
			unsigned lca_motion_y_gain2        : 6;     // bits : 21_16
		} bit;
		UINT32 word;
	} reg_753; // 0x0bc4

	union {
		struct {
			unsigned lca_still_u_gain0        : 6;      // bits : 5_0
			unsigned                          : 2;
			unsigned lca_still_u_gain1        : 6;      // bits : 13_8
			unsigned                          : 2;
			unsigned lca_still_u_gain2        : 6;      // bits : 21_16
		} bit;
		UINT32 word;
	} reg_754; // 0x0bc8

	union {
		struct {
			unsigned lca_motion_u_gain0        : 6;     // bits : 5_0
			unsigned                           : 2;
			unsigned lca_motion_u_gain1        : 6;     // bits : 13_8
			unsigned                           : 2;
			unsigned lca_motion_u_gain2        : 6;     // bits : 21_16
		} bit;
		UINT32 word;
	} reg_755; // 0x0bcc

	union {
		struct {
			unsigned lca_still_v_gain0        : 6;      // bits : 5_0
			unsigned                          : 2;
			unsigned lca_still_v_gain1        : 6;      // bits : 13_8
			unsigned                          : 2;
			unsigned lca_still_v_gain2        : 6;      // bits : 21_16
		} bit;
		UINT32 word;
	} reg_756; // 0x0bd0

	union {
		struct {
			unsigned lca_motion_v_gain0        : 6;     // bits : 5_0
			unsigned                           : 2;
			unsigned lca_motion_v_gain1        : 6;     // bits : 13_8
			unsigned                           : 2;
			unsigned lca_motion_v_gain2        : 6;     // bits : 21_16
		} bit;
		UINT32 word;
	} reg_757; // 0x0bd4

	union {
		struct {
			unsigned lca_still_y_coff0        : 6;      // bits : 5_0
			unsigned                          : 2;
			unsigned lca_still_y_coff1        : 6;      // bits : 13_8
			unsigned                          : 2;
			unsigned lca_still_y_coff2        : 6;      // bits : 21_16
		} bit;
		UINT32 word;
	} reg_758; // 0x0bd8

	union {
		struct {
			unsigned lca_motion_y_coff0        : 6;     // bits : 5_0
			unsigned                           : 2;
			unsigned lca_motion_y_coff1        : 6;     // bits : 13_8
			unsigned                           : 2;
			unsigned lca_motion_y_coff2        : 6;     // bits : 21_16
		} bit;
		UINT32 word;
	} reg_759; // 0x0bdc

	union {
		struct {
			unsigned lca_still_u_coff0        : 6;      // bits : 5_0
			unsigned                          : 2;
			unsigned lca_still_u_coff1        : 6;      // bits : 13_8
			unsigned                          : 2;
			unsigned lca_still_u_coff2        : 6;      // bits : 21_16
		} bit;
		UINT32 word;
	} reg_760; // 0x0be0

	union {
		struct {
			unsigned lca_motion_u_coff0        : 6;     // bits : 5_0
			unsigned                           : 2;
			unsigned lca_motion_u_coff1        : 6;     // bits : 13_8
			unsigned                           : 2;
			unsigned lca_motion_u_coff2        : 6;     // bits : 21_16
		} bit;
		UINT32 word;
	} reg_761; // 0x0be4

	union {
		struct {
			unsigned lca_still_v_coff0        : 6;      // bits : 5_0
			unsigned                          : 2;
			unsigned lca_still_v_coff1        : 6;      // bits : 13_8
			unsigned                          : 2;
			unsigned lca_still_v_coff2        : 6;      // bits : 21_16
		} bit;
		UINT32 word;
	} reg_762; // 0x0be8

	union {
		struct {
			unsigned lca_motion_v_coff0        : 6;     // bits : 5_0
			unsigned                           : 2;
			unsigned lca_motion_v_coff1        : 6;     // bits : 13_8
			unsigned                           : 2;
			unsigned lca_motion_v_coff2        : 6;     // bits : 21_16
		} bit;
		UINT32 word;
	} reg_763; // 0x0bec

	union {
		struct {
			unsigned lca_dbg_en             : 1;        // bits : 0
			unsigned lca_dbg_chl_sel        : 4;        // bits : 4_1
			unsigned lca_dbg_chl_ofs        : 8;        // bits : 12_5
			unsigned                        : 3;
			unsigned lca_dbg_x_pos          : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} reg_764; // 0x0bf0

	union {
		struct {
			unsigned lca_edge_th0        : 17;      // bits : 16_0
		} bit;
		UINT32 word;
	} reg_765; // 0x0bf4

	union {
		struct {
			unsigned lca_edge_th1        : 17;      // bits : 16_0
		} bit;
		UINT32 word;
	} reg_766; // 0x0bf8

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_767; // 0x0bfc

	union {
		struct {
			unsigned nn_isp_p2_ringbuf_height        : 16;      // bits : 15_0
			unsigned nn_isp_p2_slice_height          : 14;      // bits : 29_16
			unsigned                                 : 1;
			unsigned nn_isp_p2_fw_hs_en              : 1;       // bits : 31
		} bit;
		UINT32 word;
	} reg_768; // 0x0c00

	union {
		struct {
			unsigned nn_isp_p2_slice_ovlp           : 7;        // bits : 6_0
			unsigned                                : 9;
			unsigned nn_isp_p2_outbuf_height        : 14;       // bits : 29_16
			unsigned                                : 1;
			unsigned nn_isp_p2_outbuf_num           : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_769; // 0x0c04

	union {
		struct {
			unsigned                           : 2;
			unsigned nn_isp_p2_yuv_ofsi        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_770; // 0x0c08

	union {
		struct {
			unsigned nn_isp_p2_y_sai        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_771; // 0x0c0c

	union {
		struct {
			unsigned nn_isp_p2_uv_sai        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_772; // 0x0c10

	union {
		struct {
			unsigned nn_isp_p2_msb_y_sai         : 4;       // bits : 3_0
			unsigned                             : 12;
			unsigned nn_isp_p2_msb_uv_sai        : 4;       // bits : 19_16
		} bit;
		UINT32 word;
	} reg_773; // 0x0c14

	union {
		struct {
			unsigned                           : 2;
			unsigned nn_isp_p2_yuv_ofso        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_774; // 0x0c18

	union {
		struct {
			unsigned nn_isp_p2_y_sao        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_775; // 0x0c1c

	union {
		struct {
			unsigned nn_isp_p2_uv_sao        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_776; // 0x0c20

	union {
		struct {
			unsigned nn_isp_p2_msb_y_sao         : 4;       // bits : 3_0
			unsigned                             : 12;
			unsigned nn_isp_p2_msb_uv_sao        : 4;       // bits : 19_16
		} bit;
		UINT32 word;
	} reg_777; // 0x0c24

	union {
		struct {
			unsigned         : 31;      // bits : 31_0
			unsigned nn_isp_p2_cpu2isp_in_ready        : 1;      // bits : 31
		} bit;
		UINT32 word;
	} reg_778; // 0x0c28

	union {
		struct {
			unsigned         : 31;      // bits : 31_0
			unsigned nn_isp_p2_cpu2isp_out_clear        : 1;      // bits : 31
		} bit;
		UINT32 word;
	} reg_779; // 0x0c2c

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_780; // 0x0c30

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_781; // 0x0c34

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_782; // 0x0c38

	union {
		struct {
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_783; // 0x0c3c

	union {
		struct {
			unsigned nn_isp_p3_ringbuf_height        : 16;      // bits : 15_0
			unsigned nn_isp_p3_slice_height          : 14;      // bits : 26_16
			unsigned                                 : 1;
			unsigned nn_isp_p3_fw_hs_en              : 1;       // bits : 29
		} bit;
		UINT32 word;
	} reg_784; // 0x0c40

	union {
		struct {
			unsigned nn_isp_p3_slice_ovlp           : 7;        // bits : 6_0
			unsigned                                : 9;
			unsigned nn_isp_p3_outbuf_height        : 14;       // bits : 26_16
			unsigned                                : 1;
			unsigned nn_isp_p3_outbuf_num           : 1;        // bits : 31
		} bit;
		UINT32 word;
	} reg_785; // 0x0c44

	union {
		struct {
			unsigned                           : 2;
			unsigned nn_isp_p3_yuv_ofsi        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_786; // 0x0c48

	union {
		struct {
			unsigned nn_isp_p3_y_sai        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_787; // 0x0c4c

	union {
		struct {
			unsigned nn_isp_p3_uv_sai        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_788; // 0x0c50

	union {
		struct {
			unsigned nn_isp_p3_msb_y_sai         : 4;       // bits : 3_0
			unsigned                             : 12;
			unsigned nn_isp_p3_msb_uv_sai        : 4;       // bits : 19_16
		} bit;
		UINT32 word;
	} reg_789; // 0x0c54

	union {
		struct {
			unsigned                               : 32;
			//unsigned nn_isp_p3_txt_map_ofsi        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_790; // 0x0c58

	union {
		struct {
			unsigned                               : 2;
			unsigned nn_isp_p3_mot_map_ofsi        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_791; // 0x0c5c

	union {
		struct {
			//unsigned nn_isp_p3_txt_map_sai        : 32;     // bits : 31_0
			unsigned reserved        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_792; // 0x0c60

	union {
		struct {
			unsigned nn_isp_p3_mot_map_sai        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_793; // 0x0c64

	union {
		struct {
			//unsigned nn_isp_p3_txt_map_msb_sai        : 4;      // bits : 3_0
			unsigned reserved                         : 4;      // bits : 3_0
			unsigned                                  : 12;
			unsigned nn_isp_p3_mot_map_msb_sai        : 4;      // bits : 19_16
		} bit;
		UINT32 word;
	} reg_794; // 0x0c68

	union {
		struct {
			unsigned                           : 2;
			unsigned nn_isp_p3_yuv_ofso        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_795; // 0x0c6c

	union {
		struct {
			unsigned nn_isp_p3_y_sao        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} reg_796; // 0x0c70

	union {
		struct {
			unsigned nn_isp_p3_uv_sao        : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} reg_797; // 0x0c74

	union {
		struct {
			unsigned nn_isp_p3_y_msb_sao         : 4;       // bits : 3_0
			unsigned                             : 12;
			unsigned nn_isp_p3_uv_msb_sao        : 4;       // bits : 19_16
		} bit;
		UINT32 word;
	} reg_798; // 0x0c78

	union {
		struct {
			unsigned                               : 2;
			unsigned nn_isp_p3_mot_map_ofso        : 18;        // bits : 19_2
		} bit;
		UINT32 word;
	} reg_799; // 0x0c7c

	union {
		struct {
			unsigned nn_isp_p3_mot_map_sao        : 32;     // bits : 31_0
		} bit;
		UINT32 word;
	} reg_800; // 0x0c80

	union {
		struct {
			unsigned nn_isp_p3_mot_map_msb_sao        : 4;      // bits : 3_0
		} bit;
		UINT32 word;
	} reg_801; // 0x0c84

	union {
		struct {
			unsigned         : 31;      // bits : 31_0
			unsigned nn_isp_p3_cpu2isp_in_ready        : 1;      // bits : 31
		} bit;
		UINT32 word;
	} reg_802; // 0x0c88

	union {
		struct {
			unsigned         : 31;      // bits : 31_0
			unsigned nn_isp_p3_cpu2isp_out_clear        : 1;      // bits : 31
		} bit;
		UINT32 word;
	} reg_803; // 0x0c8c

} NT98538_IME_ENG_REG_STRUCT;


#ifdef __cplusplus
}
#endif


#endif


