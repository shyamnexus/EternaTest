/**
    Public header file for DRE module.

    @file       dre_eng.h
    @ingroup    mIIPPDRE

    @brief

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/
#ifndef _DRE_REG_H
#define _DRE_REG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __KERNEL__
#include "linux/soc/nvt/rcw_macro.h"
#include "plat/top.h"
#include "kwrap/type.h"
//#include "kwrap/nvt_type.h"
#elif defined(__FREERTOS)
#include "rcw_macro.h"
#include "plat/top.h"
#include "kwrap/type.h"
#include "kwrap/nvt_type.h"

#else
#endif


/*
    dre_rst  :    [0x0, 0x1],			bits : 0
    dre_start:    [0x0, 0x1],			bits : 1
    ll_fire  :    [0x0, 0x1],			bits : 28
*/
#define ENGINE_CONTROL_REGISTER_OFS 0x0000
REGDEF_BEGIN(ENGINE_CONTROL_REGISTER)
    REGDEF_BIT(dre_rst  ,        1)
    REGDEF_BIT(dre_start,        1)
    REGDEF_BIT(         ,        26)
    REGDEF_BIT(ll_fire  ,        1)
REGDEF_END(ENGINE_CONTROL_REGISTER)


/*
    dre_process_sel       :    [0x0, 0x1],			bits : 0
    dre_dcm_in_img_num_sel:    [0x0, 0x1],			bits : 1
    dre_dcm_work_mode_sel :    [0x0, 0x7],			bits : 4_2
    dre_dcm_filt_en       :    [0x0, 0x1],			bits : 6
    dre_dcm_nr_lut_y_en   :    [0x0, 0x1],			bits : 7
    dre_dcm_nr_lut_u_en   :    [0x0, 0x1],			bits : 8
    dre_dcm_nr_lut_v_en   :    [0x0, 0x1],			bits : 9
    dre_rcs_work_mode_sel :    [0x0, 0x7],			bits : 12_10
    ycmod_en              :    [0x0, 0x1],			bits : 13
    weight_map_src_sel    :    [0x0, 0x1],			bits : 14
    hard_weight_en        :    [0x0, 0x1],			bits : 15
    src_image_patching_mode:    [0x0, 0x1],			bits : 16
*/
#define FUNCTION_CONTROL_REGISTER_OFS 0x0004
REGDEF_BEGIN(FUNCTION_CONTROL_REGISTER)
    REGDEF_BIT(dre_process_sel       ,        1)
    REGDEF_BIT(dre_dcm_in_img_num_sel,        1)
    REGDEF_BIT(dre_dcm_work_mode_sel ,        3)
    REGDEF_BIT(                      ,        1)
    REGDEF_BIT(dre_dcm_filt_en       ,        1)
    REGDEF_BIT(dre_dcm_nr_lut_y_en   ,        1)
    REGDEF_BIT(dre_dcm_nr_lut_u_en   ,        1)
    REGDEF_BIT(dre_dcm_nr_lut_v_en   ,        1)
    REGDEF_BIT(dre_rcs_work_mode_sel ,        3)
    REGDEF_BIT(ycmod_en              ,        1)
    REGDEF_BIT(weight_map_src_sel    ,        1)
    REGDEF_BIT(hard_weight_en        ,        1)
    REGDEF_BIT(src_image_patching_mode ,        1)
REGDEF_END(FUNCTION_CONTROL_REGISTER)


/*
    dre_inte_end         :    [0x0, 0x1],			bits : 0
    dre_inte_timeout     :    [0x0, 0x1],			bits : 1
    dre_inte_llend       :    [0x0, 0x1],			bits : 2
    dre_inte_ll_cmd_error:    [0x0, 0x1],			bits : 3
    dre_inte_lljobend    :    [0x0, 0x1],			bits : 4
*/
#define DRE_INTERRUPT_ENABLE_REGISTER_OFS 0x0008
REGDEF_BEGIN(DRE_INTERRUPT_ENABLE_REGISTER)
    REGDEF_BIT(dre_inte_end         ,        1)
    REGDEF_BIT(dre_inte_timeout     ,        1)
    REGDEF_BIT(dre_inte_llend       ,        1)
    REGDEF_BIT(dre_inte_ll_cmd_error,        1)
    REGDEF_BIT(dre_inte_lljobend    ,        1)
REGDEF_END(DRE_INTERRUPT_ENABLE_REGISTER)


/*
    dre_int_end         :    [0x0, 0x1],			bits : 0
    dre_int_timeout     :    [0x0, 0x1],			bits : 1
    dre_int_llend       :    [0x0, 0x1],			bits : 2
    dre_int_ll_cmd_error:    [0x0, 0x1],			bits : 3
    dre_int_lljobend    :    [0x0, 0x1],			bits : 4
*/
#define DRE_INTERRUPT_STATUS_REGISTER_OFS 0x000c
REGDEF_BEGIN(DRE_INTERRUPT_STATUS_REGISTER)
    REGDEF_BIT(dre_int_end         ,        1)
    REGDEF_BIT(dre_int_timeout     ,        1)
    REGDEF_BIT(dre_int_llend       ,        1)
    REGDEF_BIT(dre_int_ll_cmd_error,        1)
    REGDEF_BIT(dre_int_lljobend    ,        1)
REGDEF_END(DRE_INTERRUPT_STATUS_REGISTER)


/*
    dre_time_limit:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DRE_DEBUG_REGISTER_1_OFS 0x0010
REGDEF_BEGIN(DRE_DEBUG_REGISTER_1)
    REGDEF_BIT(dre_time_limit,        32)
REGDEF_END(DRE_DEBUG_REGISTER_1)


/*
    dre_proc_time:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DRE_DEBUG_REGISTER_2_OFS 0x0014
REGDEF_BEGIN(DRE_DEBUG_REGISTER_2)
    REGDEF_BIT(dre_proc_time,        32)
REGDEF_END(DRE_DEBUG_REGISTER_2)


/*
    dre_in_ch0_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define INPUT_CHANNEL_0_ADDRESS_REGISTER_OFS 0x0018
REGDEF_BEGIN(INPUT_CHANNEL_0_ADDRESS_REGISTER)
    REGDEF_BIT(               ,        2)
    REGDEF_BIT(dre_in_ch0_addr,        30)
REGDEF_END(INPUT_CHANNEL_0_ADDRESS_REGISTER)


/*
    dre_in_ch1_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define INPUT_CHANNEL_1_ADDRESS_REGISTER_OFS 0x001c
REGDEF_BEGIN(INPUT_CHANNEL_1_ADDRESS_REGISTER)
    REGDEF_BIT(               ,        2)
    REGDEF_BIT(dre_in_ch1_addr,        30)
REGDEF_END(INPUT_CHANNEL_1_ADDRESS_REGISTER)


/*
    dre_in_ch2_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define INPUT_CHANNEL_2_ADDRESS_REGISTER_OFS 0x0020
REGDEF_BEGIN(INPUT_CHANNEL_2_ADDRESS_REGISTER)
    REGDEF_BIT(               ,        2)
    REGDEF_BIT(dre_in_ch2_addr,        30)
REGDEF_END(INPUT_CHANNEL_2_ADDRESS_REGISTER)


/*
    dre_in_ch3_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define INPUT_CHANNEL_3_ADDRESS_REGISTER_OFS 0x0024
REGDEF_BEGIN(INPUT_CHANNEL_3_ADDRESS_REGISTER)
    REGDEF_BIT(               ,        2)
    REGDEF_BIT(dre_in_ch3_addr,        30)
REGDEF_END(INPUT_CHANNEL_3_ADDRESS_REGISTER)


/*
    dre_in_weight_map_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define WEIGHT_MAP_INPUT_ADDRESS_REGISTER_OFS 0x0028
REGDEF_BEGIN(WEIGHT_MAP_INPUT_ADDRESS_REGISTER)
    REGDEF_BIT(                      ,        2)
    REGDEF_BIT(dre_in_weight_map_addr,        30)
REGDEF_END(WEIGHT_MAP_INPUT_ADDRESS_REGISTER)


/*
    dre_out_ch0_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define OUTPUT_CHANNEL_0_ADDRESS_REGISTER_OFS 0x002c
REGDEF_BEGIN(OUTPUT_CHANNEL_0_ADDRESS_REGISTER)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(dre_out_ch0_addr,        30)
REGDEF_END(OUTPUT_CHANNEL_0_ADDRESS_REGISTER)


/*
    dre_out_ch1_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define OUTPUT_CHANNEL_1_ADDRESS_REGISTER_OFS 0x0030
REGDEF_BEGIN(OUTPUT_CHANNEL_1_ADDRESS_REGISTER)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(dre_out_ch1_addr,        30)
REGDEF_END(OUTPUT_CHANNEL_1_ADDRESS_REGISTER)


/*
    dre_out_ch2_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define OUTPUT_CHANNEL_2_ADDRESS_REGISTER_OFS 0x0034
REGDEF_BEGIN(OUTPUT_CHANNEL_2_ADDRESS_REGISTER)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(dre_out_ch2_addr,        30)
REGDEF_END(OUTPUT_CHANNEL_2_ADDRESS_REGISTER)


/*
    dre_out_weight_map_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define WEIGHT_MAP_OUTPUT_ADDRESS_REGISTER_OFS 0x0038
REGDEF_BEGIN(WEIGHT_MAP_OUTPUT_ADDRESS_REGISTER)
    REGDEF_BIT(                       ,        2)
    REGDEF_BIT(dre_out_weight_map_addr,        30)
REGDEF_END(WEIGHT_MAP_OUTPUT_ADDRESS_REGISTER)


/*
    dre_l0_width :    [0x0, 0xffff],			bits : 15_0
    dre_l0_height:    [0x0, 0xffff],			bits : 31_16
*/
#define IMAGE_SIZE_OF_LAYER_0_REGISTER_OFS 0x003c
REGDEF_BEGIN(IMAGE_SIZE_OF_LAYER_0_REGISTER)
    REGDEF_BIT(dre_l0_width ,        16)
    REGDEF_BIT(dre_l0_height,        16)
REGDEF_END(IMAGE_SIZE_OF_LAYER_0_REGISTER)


/*
    dre_l1_width :    [0x0, 0xffff],			bits : 15_0
    dre_l1_height:    [0x0, 0xffff],			bits : 31_16
*/
#define IMAGE_SIZE_OF_LAYER_1_REGISTER_OFS 0x0040
REGDEF_BEGIN(IMAGE_SIZE_OF_LAYER_1_REGISTER)
    REGDEF_BIT(dre_l1_width ,        16)
    REGDEF_BIT(dre_l1_height,        16)
REGDEF_END(IMAGE_SIZE_OF_LAYER_1_REGISTER)


/*
    dre_in_ch0_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define INPUT_CHANNEL_0_LINE_OFFSET_REGISTER_OFS 0x0044
REGDEF_BEGIN(INPUT_CHANNEL_0_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(dre_in_ch0_lofst,        18)
REGDEF_END(INPUT_CHANNEL_0_LINE_OFFSET_REGISTER)


/*
    dre_in_ch1_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define INPUT_CHANNEL_1_LINE_OFFSET_REGISTER_OFS 0x0048
REGDEF_BEGIN(INPUT_CHANNEL_1_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(dre_in_ch1_lofst,        18)
REGDEF_END(INPUT_CHANNEL_1_LINE_OFFSET_REGISTER)


/*
    dre_in_ch2_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define INPUT_CHANNEL_2_LINE_OFFSET_REGISTER_OFS 0x004c
REGDEF_BEGIN(INPUT_CHANNEL_2_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(dre_in_ch2_lofst,        18)
REGDEF_END(INPUT_CHANNEL_2_LINE_OFFSET_REGISTER)


/*
    dre_in_ch3_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define INPUT_CHANNEL_3_LINE_OFFSET_REGISTER_OFS 0x0050
REGDEF_BEGIN(INPUT_CHANNEL_3_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(dre_in_ch3_lofst,        18)
REGDEF_END(INPUT_CHANNEL_3_LINE_OFFSET_REGISTER)


/*
    dre_in_weight_map_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define WEIGHT_MAP_INPUT_LINE_OFFSET_REGISTER_OFS 0x0054
REGDEF_BEGIN(WEIGHT_MAP_INPUT_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                       ,        2)
    REGDEF_BIT(dre_in_weight_map_lofst,        18)
REGDEF_END(WEIGHT_MAP_INPUT_LINE_OFFSET_REGISTER)


/*
    dre_out_ch0_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define OUTPUT_CHANNEL_0_LINE_OFFSET_REGISTER_OFS 0x0058
REGDEF_BEGIN(OUTPUT_CHANNEL_0_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                 ,        2)
    REGDEF_BIT(dre_out_ch0_lofst,        18)
REGDEF_END(OUTPUT_CHANNEL_0_LINE_OFFSET_REGISTER)


/*
    dre_out_ch1_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define OUTPUT_CHANNEL_1_LINE_OFFSET_REGISTER_OFS 0x005c
REGDEF_BEGIN(OUTPUT_CHANNEL_1_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                 ,        2)
    REGDEF_BIT(dre_out_ch1_lofst,        18)
REGDEF_END(OUTPUT_CHANNEL_1_LINE_OFFSET_REGISTER)


/*
    dre_out_ch2_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define OUTPUT_CHANNEL_2_LINE_OFFSET_REGISTER_OFS 0x0060
REGDEF_BEGIN(OUTPUT_CHANNEL_2_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                 ,        2)
    REGDEF_BIT(dre_out_ch2_lofst,        18)
REGDEF_END(OUTPUT_CHANNEL_2_LINE_OFFSET_REGISTER)


/*
    inch0_burst_len_sel         :    [0x0, 0x3],			bits : 1_0
    inch1_burst_len_sel         :    [0x0, 0x3],			bits : 3_2
    inch2_burst_len_sel         :    [0x0, 0x3],			bits : 5_4
    inch3_burst_len_sel         :    [0x0, 0x3],			bits : 7_6
    in_weight_map_burst_len_sel :    [0x0, 0x3],			bits : 9_8
    outch0_burst_len_sel        :    [0x0, 0x3],			bits : 11_10
    outch1_burst_len_sel        :    [0x0, 0x3],			bits : 13_12
    outch2_burst_len_sel        :    [0x0, 0x3],			bits : 15_14
    out_weight_map_burst_len_sel:    [0x0, 0x3],			bits : 17_16
    out_weight_map_burst_len_sel        :    [0x0, 0x3],			bits : 17_16
    disable_hw_input_data_align         :    [0x0, 0x1],			bits : 18
    disable_hw_output_data_align        :    [0x0, 0x1],			bits : 19
    in_patch_pixel_mask_burst_len_sel:    [0x0, 0x3],			bits : 21_20
    dre_dbg_port_sel            :    [0x0, 0x7],			bits : 30_28
    check_sum_enable            :    [0x0, 0x1],			bits : 31
*/
#define DMA_BURST_LENGTH_REGISTER_OFS 0x0064
REGDEF_BEGIN(DMA_BURST_LENGTH_REGISTER)
    REGDEF_BIT(inch0_burst_len_sel         ,        2)
    REGDEF_BIT(inch1_burst_len_sel         ,        2)
    REGDEF_BIT(inch2_burst_len_sel         ,        2)
    REGDEF_BIT(inch3_burst_len_sel         ,        2)
    REGDEF_BIT(in_weight_map_burst_len_sel ,        2)
    REGDEF_BIT(outch0_burst_len_sel        ,        2)
    REGDEF_BIT(outch1_burst_len_sel        ,        2)
    REGDEF_BIT(outch2_burst_len_sel        ,        2)
    REGDEF_BIT(out_weight_map_burst_len_sel,        2)
    REGDEF_BIT(disable_hw_input_data_align ,         1)
    REGDEF_BIT(disable_hw_output_data_align,         1)
    REGDEF_BIT(in_patch_pixel_mask_burst_len_sel   , 2)
    REGDEF_BIT(                            ,         6)
    REGDEF_BIT(dre_dbg_port_sel            ,        3)
    REGDEF_BIT(check_sum_enable            ,        1)
REGDEF_END(DMA_BURST_LENGTH_REGISTER)


/*
    dre_h_sdn_factor:    [0x0, 0x3ffff],			bits : 17_0
*/
#define HORIZONTAL_SCALING_FACTOR_REGISTER_OFS 0x0068
REGDEF_BEGIN(HORIZONTAL_SCALING_FACTOR_REGISTER)
    REGDEF_BIT(dre_h_sdn_factor,        18)
REGDEF_END(HORIZONTAL_SCALING_FACTOR_REGISTER)


/*
    dre_v_sdn_factor:    [0x0, 0x3ffff],			bits : 17_0
*/
#define VERTICAL_SCALING_FACTOR_REGISTER_OFS 0x006c
REGDEF_BEGIN(VERTICAL_SCALING_FACTOR_REGISTER)
    REGDEF_BIT(dre_v_sdn_factor,        18)
REGDEF_END(VERTICAL_SCALING_FACTOR_REGISTER)


/*
    dre_h_sup_factor:    [0x0, 0xffff],			bits : 15_0
    dre_v_sup_factor:    [0x0, 0xffff],			bits : 31_16
*/
#define SCALING_UP_FACTOR_REGISTER_OFS 0x0070
REGDEF_BEGIN(SCALING_UP_FACTOR_REGISTER)
    REGDEF_BIT(dre_h_sup_factor,        16)
    REGDEF_BIT(dre_v_sup_factor,        16)
REGDEF_END(SCALING_UP_FACTOR_REGISTER)


/*
    dre_out_weight_map_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define WEIGHT_MAP_OUTPUT_LINE_OFFSET_REGISTER_OFS 0x0074
REGDEF_BEGIN(WEIGHT_MAP_OUTPUT_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                        ,        2)
    REGDEF_BIT(dre_out_weight_map_lofst,        18)
REGDEF_END(WEIGHT_MAP_OUTPUT_LINE_OFFSET_REGISTER)


/*
    dre_h_sdn_offset:    [0x0, 0xffff],			bits : 15_0
    dre_v_sdn_offset:    [0x0, 0xffff],			bits : 31_16
*/
#define SAMPLING_OFFSET_OF_SCALING_DOWN_REGISTER_OFS 0x0078
REGDEF_BEGIN(SAMPLING_OFFSET_OF_SCALING_DOWN_REGISTER)
    REGDEF_BIT(dre_h_sdn_offset,        16)
    REGDEF_BIT(dre_v_sdn_offset,        16)
REGDEF_END(SAMPLING_OFFSET_OF_SCALING_DOWN_REGISTER)


/*
    ll_terminate:    [0x0, 0x1],			bits : 0
    reserved    :    [0x0, 0x7fffffff],			bits : 31_1
*/
#define LINKED_LIST_CONTROL_REGISTER_OFS 0x007c
REGDEF_BEGIN(LINKED_LIST_CONTROL_REGISTER)
    REGDEF_BIT(ll_terminate,        1)
    REGDEF_BIT(reserved    ,        31)
REGDEF_END(LINKED_LIST_CONTROL_REGISTER)


/*
    reserved      :    [0x0, 0x3],			bits : 1_0
    dre_in_ll_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define LINKED_LIST_COMMAND_ADDRESS_REGISTER_OFS 0x0080
REGDEF_BEGIN(LINKED_LIST_COMMAND_ADDRESS_REGISTER)
    REGDEF_BIT(reserved      ,        2)
    REGDEF_BIT(dre_in_ll_addr,        30)
REGDEF_END(LINKED_LIST_COMMAND_ADDRESS_REGISTER)


/*
    fusion_wt_table_val00:    [0x0, 0xff],			bits : 7_0
    fusion_wt_table_val01:    [0x0, 0xff],			bits : 15_8
    fusion_wt_table_val02:    [0x0, 0xff],			bits : 23_16
    fusion_wt_table_val03:    [0x0, 0xff],			bits : 31_24
*/
#define FUSION_WEIGHT_TABLE_0_REGISTER_OFS 0x0084
REGDEF_BEGIN(FUSION_WEIGHT_TABLE_0_REGISTER)
    REGDEF_BIT(fusion_wt_table_val00,        8)
    REGDEF_BIT(fusion_wt_table_val01,        8)
    REGDEF_BIT(fusion_wt_table_val02,        8)
    REGDEF_BIT(fusion_wt_table_val03,        8)
REGDEF_END(FUSION_WEIGHT_TABLE_0_REGISTER)


/*
    fusion_wt_table_val04:    [0x0, 0xff],			bits : 7_0
    fusion_wt_table_val05:    [0x0, 0xff],			bits : 15_8
    fusion_wt_table_val06:    [0x0, 0xff],			bits : 23_16
    fusion_wt_table_val07:    [0x0, 0xff],			bits : 31_24
*/
#define FUSION_WEIGHT_TABLE_1_REGISTER_OFS 0x0088
REGDEF_BEGIN(FUSION_WEIGHT_TABLE_1_REGISTER)
    REGDEF_BIT(fusion_wt_table_val04,        8)
    REGDEF_BIT(fusion_wt_table_val05,        8)
    REGDEF_BIT(fusion_wt_table_val06,        8)
    REGDEF_BIT(fusion_wt_table_val07,        8)
REGDEF_END(FUSION_WEIGHT_TABLE_1_REGISTER)


/*
    fusion_wt_table_val08:    [0x0, 0xff],			bits : 7_0
    fusion_wt_table_val09:    [0x0, 0xff],			bits : 15_8
    fusion_wt_table_val10:    [0x0, 0xff],			bits : 23_16
    fusion_wt_table_val11:    [0x0, 0xff],			bits : 31_24
*/
#define FUSION_WEIGHT_TABLE_2_REGISTER_OFS 0x008c
REGDEF_BEGIN(FUSION_WEIGHT_TABLE_2_REGISTER)
    REGDEF_BIT(fusion_wt_table_val08,        8)
    REGDEF_BIT(fusion_wt_table_val09,        8)
    REGDEF_BIT(fusion_wt_table_val10,        8)
    REGDEF_BIT(fusion_wt_table_val11,        8)
REGDEF_END(FUSION_WEIGHT_TABLE_2_REGISTER)


/*
    fusion_wt_table_val12:    [0x0, 0xff],			bits : 7_0
    fusion_wt_table_val13:    [0x0, 0xff],			bits : 15_8
    fusion_wt_table_val14:    [0x0, 0xff],			bits : 23_16
    fusion_wt_table_val15:    [0x0, 0xff],			bits : 31_24
*/
#define FUSION_WEIGHT_TABLE_3_REGISTER_OFS 0x0090
REGDEF_BEGIN(FUSION_WEIGHT_TABLE_3_REGISTER)
    REGDEF_BIT(fusion_wt_table_val12,        8)
    REGDEF_BIT(fusion_wt_table_val13,        8)
    REGDEF_BIT(fusion_wt_table_val14,        8)
    REGDEF_BIT(fusion_wt_table_val15,        8)
REGDEF_END(FUSION_WEIGHT_TABLE_3_REGISTER)


/*
    fusion_wt_table_val16:    [0x0, 0xff],			bits : 7_0
*/
#define FUSION_WEIGHT_TABLE_4_REGISTER_OFS 0x0094
REGDEF_BEGIN(FUSION_WEIGHT_TABLE_4_REGISTER)
    REGDEF_BIT(fusion_wt_table_val16,        8)
REGDEF_END(FUSION_WEIGHT_TABLE_4_REGISTER)


/*
    dma_disable:    [0x0, 0x1],			bits : 0
    dre_idle   :    [0x0, 0x1],			bits : 15
*/
#define DMA_DISABLE_REGISER_0_OFS 0x0098
REGDEF_BEGIN(DMA_DISABLE_REGISER_0)
    REGDEF_BIT(dma_disable,        1)
    REGDEF_BIT(           ,        14)
    REGDEF_BIT(dre_idle   ,        1)
REGDEF_END(DMA_DISABLE_REGISER_0)


/*
    img0_spa_coeff0:    [0x0, 0xff],			bits : 7_0
    img0_spa_coeff1:    [0x0, 0xff],			bits : 15_8
    img0_spa_coeff2:    [0x0, 0xff],			bits : 23_16
*/
#define FUSION_IMAGE_0_3X3_SPATIAL_FILTER_REGISTER_OFS 0x009c
REGDEF_BEGIN(FUSION_IMAGE_0_3X3_SPATIAL_FILTER_REGISTER)
    REGDEF_BIT(img0_spa_coeff0,        8)
    REGDEF_BIT(img0_spa_coeff1,        8)
    REGDEF_BIT(img0_spa_coeff2,        8)
REGDEF_END(FUSION_IMAGE_0_3X3_SPATIAL_FILTER_REGISTER)


/*
    dre_in_patch_pixel_mask_addr:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define PATCH_PIXEL_MASK_INPUT_ADDRESS_REGISTER_OFS 0x00a0
REGDEF_BEGIN(PATCH_PIXEL_MASK_INPUT_ADDRESS_REGISTER)
    REGDEF_BIT(                             ,        2)
    REGDEF_BIT(dre_in_patch_pixel_mask_addr,        30)
REGDEF_END(PATCH_PIXEL_MASK_INPUT_ADDRESS_REGISTER)

/*
    dre_in_patch_pixel_mask_lofst:    [0x0, 0x3ffff],			bits : 19_2
*/
#define PATCH_PIXEL_MASK_LINE_OFFSET_REGISTER_OFS 0x00a4
REGDEF_BEGIN(PATCH_PIXEL_MASK_LINE_OFFSET_REGISTER)
    REGDEF_BIT(                             ,        2)
    REGDEF_BIT(dre_in_patch_pixel_mask_lofst,        18)
REGDEF_END(PATCH_PIXEL_MASK_LINE_OFFSET_REGISTER)


/*
    img_patch_user_color_y:    [0x0, 0xff],		bits : 7_0
    img_patch_user_color_y:    [0x0, 0xff], 		bits : 15_8
    img_patch_user_color_v:    [0x0, 0xff], 		bits : 23_16   
*/
#define IMG_PATCH_USER_COLOR_REGISTER_OFS 0x00a8
REGDEF_BEGIN(IMG_PATCH_MODE_USER_COLOR_REGISTER)
    REGDEF_BIT(img_patch_user_color_y,        8)
    REGDEF_BIT(img_patch_user_color_u,        8)
    REGDEF_BIT(img_patch_user_color_v,        8)
REGDEF_END(IMG_PATCH_MODE_USER_COLOR_REGISTER)



/*
    img1_spa_coeff0:    [0x0, 0xff],			bits : 7_0
    img1_spa_coeff1:    [0x0, 0xff],			bits : 15_8
    img1_spa_coeff2:    [0x0, 0xff],			bits : 23_16
*/
#define FUSION_IMAGE_1_3X3_SPATIAL_FILTER_REGISTER_OFS 0x00ac
REGDEF_BEGIN(FUSION_IMAGE_1_3X3_SPATIAL_FILTER_REGISTER)
    REGDEF_BIT(img1_spa_coeff0,        8)
    REGDEF_BIT(img1_spa_coeff1,        8)
    REGDEF_BIT(img1_spa_coeff2,        8)
REGDEF_END(FUSION_IMAGE_1_3X3_SPATIAL_FILTER_REGISTER)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER_3_OFS 0x00b0
REGDEF_BEGIN(RESERVED_REGISTER_3)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER_3)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER_4_OFS 0x00b4
REGDEF_BEGIN(RESERVED_REGISTER_4)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER_4)


/*
    s_weight0:    [0x0, 0xff],			bits : 7_0
    s_weight1:    [0x0, 0xff],			bits : 15_8
    s_weight2:    [0x0, 0xff],			bits : 23_16
    s_weight3:    [0x0, 0xff],			bits : 31_24
*/
#define NR_5X5_SPATIAL_FILTER_REGISTER0_OFS 0x00b8
REGDEF_BEGIN(NR_5X5_SPATIAL_FILTER_REGISTER0)
    REGDEF_BIT(s_weight0,        8)
    REGDEF_BIT(s_weight1,        8)
    REGDEF_BIT(s_weight2,        8)
    REGDEF_BIT(s_weight3,        8)
REGDEF_END(NR_5X5_SPATIAL_FILTER_REGISTER0)


/*
    s_weight4:    [0x0, 0xff],			bits : 7_0
    s_weight5:    [0x0, 0xff],			bits : 15_8
*/
#define NR_5X5_SPATIAL_FILTER_REGISTER1_OFS 0x00bc
REGDEF_BEGIN(NR_5X5_SPATIAL_FILTER_REGISTER1)
    REGDEF_BIT(s_weight4,        8)
    REGDEF_BIT(s_weight5,        8)
REGDEF_END(NR_5X5_SPATIAL_FILTER_REGISTER1)


/*
    rth_y_lut00:    [0x0, 0xff],			bits : 7_0
    rth_y_lut01:    [0x0, 0xff],			bits : 15_8
    rth_y_lut02:    [0x0, 0xff],			bits : 23_16
    rth_y_lut03:    [0x0, 0xff],			bits : 31_24
*/
#define NR_RANGE_THRESHOLD_LUT_REGISTER0_OFS 0x00c0
REGDEF_BEGIN(NR_RANGE_THRESHOLD_LUT_REGISTER0)
    REGDEF_BIT(rth_y_lut00,        8)
    REGDEF_BIT(rth_y_lut01,        8)
    REGDEF_BIT(rth_y_lut02,        8)
    REGDEF_BIT(rth_y_lut03,        8)
REGDEF_END(NR_RANGE_THRESHOLD_LUT_REGISTER0)


/*
    rth_y_lut04:    [0x0, 0xff],			bits : 7_0
    rth_y_lut05:    [0x0, 0xff],			bits : 15_8
    rth_y_lut06:    [0x0, 0xff],			bits : 23_16
    rth_y_lut07:    [0x0, 0xff],			bits : 31_24
*/
#define NR_RANGE_THRESHOLD_LUT_REGISTER1_OFS 0x00c4
REGDEF_BEGIN(NR_RANGE_THRESHOLD_LUT_REGISTER1)
    REGDEF_BIT(rth_y_lut04,        8)
    REGDEF_BIT(rth_y_lut05,        8)
    REGDEF_BIT(rth_y_lut06,        8)
    REGDEF_BIT(rth_y_lut07,        8)
REGDEF_END(NR_RANGE_THRESHOLD_LUT_REGISTER1)


/*
    rth_u_lut00:    [0x0, 0xff],			bits : 7_0
    rth_u_lut01:    [0x0, 0xff],			bits : 15_8
    rth_u_lut02:    [0x0, 0xff],			bits : 23_16
    rth_u_lut03:    [0x0, 0xff],			bits : 31_24
*/
#define NR_RANGE_THRESHOLD_LUT_REGISTER2_OFS 0x00c8
REGDEF_BEGIN(NR_RANGE_THRESHOLD_LUT_REGISTER2)
    REGDEF_BIT(rth_u_lut00,        8)
    REGDEF_BIT(rth_u_lut01,        8)
    REGDEF_BIT(rth_u_lut02,        8)
    REGDEF_BIT(rth_u_lut03,        8)
REGDEF_END(NR_RANGE_THRESHOLD_LUT_REGISTER2)


/*
    rth_u_lut04:    [0x0, 0xff],			bits : 7_0
    rth_u_lut05:    [0x0, 0xff],			bits : 15_8
    rth_u_lut06:    [0x0, 0xff],			bits : 23_16
    rth_u_lut07:    [0x0, 0xff],			bits : 31_24
*/
#define NR_RANGE_THRESHOLD_LUT_REGISTER3_OFS 0x00cc
REGDEF_BEGIN(NR_RANGE_THRESHOLD_LUT_REGISTER3)
    REGDEF_BIT(rth_u_lut04,        8)
    REGDEF_BIT(rth_u_lut05,        8)
    REGDEF_BIT(rth_u_lut06,        8)
    REGDEF_BIT(rth_u_lut07,        8)
REGDEF_END(NR_RANGE_THRESHOLD_LUT_REGISTER3)


/*
    rth_v_lut00:    [0x0, 0xff],			bits : 7_0
    rth_v_lut01:    [0x0, 0xff],			bits : 15_8
    rth_v_lut02:    [0x0, 0xff],			bits : 23_16
    rth_v_lut03:    [0x0, 0xff],			bits : 31_24
*/
#define NR_RANGE_THRESHOLD_LUT_REGISTER4_OFS 0x00d0
REGDEF_BEGIN(NR_RANGE_THRESHOLD_LUT_REGISTER4)
    REGDEF_BIT(rth_v_lut00,        8)
    REGDEF_BIT(rth_v_lut01,        8)
    REGDEF_BIT(rth_v_lut02,        8)
    REGDEF_BIT(rth_v_lut03,        8)
REGDEF_END(NR_RANGE_THRESHOLD_LUT_REGISTER4)


/*
    rth_v_lut04:    [0x0, 0xff],			bits : 7_0
    rth_v_lut05:    [0x0, 0xff],			bits : 15_8
    rth_v_lut06:    [0x0, 0xff],			bits : 23_16
    rth_v_lut07:    [0x0, 0xff],			bits : 31_24
*/
#define NR_RANGE_THRESHOLD_LUT_REGISTER5_OFS 0x00d4
REGDEF_BEGIN(NR_RANGE_THRESHOLD_LUT_REGISTER5)
    REGDEF_BIT(rth_v_lut04,        8)
    REGDEF_BIT(rth_v_lut05,        8)
    REGDEF_BIT(rth_v_lut06,        8)
    REGDEF_BIT(rth_v_lut07,        8)
REGDEF_END(NR_RANGE_THRESHOLD_LUT_REGISTER5)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER_5_OFS 0x00d8
REGDEF_BEGIN(RESERVED_REGISTER_5)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER_5)


/*
    joint_yy:    [0x0, 0x1],			bits : 0
    joint_yu:    [0x0, 0x1],			bits : 1
    joint_yv:    [0x0, 0x1],			bits : 2
    joint_uy:    [0x0, 0x1],			bits : 3
    joint_uu:    [0x0, 0x1],			bits : 4
    joint_uv:    [0x0, 0x1],			bits : 5
    joint_vy:    [0x0, 0x1],			bits : 6
    joint_vu:    [0x0, 0x1],			bits : 7
    joint_vv:    [0x0, 0x1],			bits : 8
    outly_en:    [0x0, 0x1],			bits : 9
    outlu_en:    [0x0, 0x1],			bits : 10
    outlv_en:    [0x0, 0x1],			bits : 11
*/
#define NR_JOINT_BILATERAL_REGISTER_OFS 0x00dc
REGDEF_BEGIN(NR_JOINT_BILATERAL_REGISTER)
    REGDEF_BIT(joint_yy,        1)
    REGDEF_BIT(joint_yu,        1)
    REGDEF_BIT(joint_yv,        1)
    REGDEF_BIT(joint_uy,        1)
    REGDEF_BIT(joint_uu,        1)
    REGDEF_BIT(joint_uv,        1)
    REGDEF_BIT(joint_vy,        1)
    REGDEF_BIT(joint_vu,        1)
    REGDEF_BIT(joint_vv,        1)
    REGDEF_BIT(outly_en,        1)
    REGDEF_BIT(outlu_en,        1)
    REGDEF_BIT(outlv_en,        1)
REGDEF_END(NR_JOINT_BILATERAL_REGISTER)


/*
    msnr_ylut0:    [0x0, 0xff],			bits : 7_0
    msnr_ylut1:    [0x0, 0xff],			bits : 15_8
    msnr_ylut2:    [0x0, 0xff],			bits : 23_16
    msnr_ylut3:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER0_OFS 0x00e0
REGDEF_BEGIN(Y_NR_REGISTER0)
    REGDEF_BIT(msnr_ylut0,        8)
    REGDEF_BIT(msnr_ylut1,        8)
    REGDEF_BIT(msnr_ylut2,        8)
    REGDEF_BIT(msnr_ylut3,        8)
REGDEF_END(Y_NR_REGISTER0)


/*
    msnr_ylut4:    [0x0, 0xff],			bits : 7_0
    msnr_ylut5:    [0x0, 0xff],			bits : 15_8
    msnr_ylut6:    [0x0, 0xff],			bits : 23_16
    msnr_ylut7:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER1_OFS 0x00e4
REGDEF_BEGIN(Y_NR_REGISTER1)
    REGDEF_BIT(msnr_ylut4,        8)
    REGDEF_BIT(msnr_ylut5,        8)
    REGDEF_BIT(msnr_ylut6,        8)
    REGDEF_BIT(msnr_ylut7,        8)
REGDEF_END(Y_NR_REGISTER1)


/*
    msnr_ylut8 :    [0x0, 0xff],			bits : 7_0
    msnr_ylut9 :    [0x0, 0xff],			bits : 15_8
    msnr_ylut10:    [0x0, 0xff],			bits : 23_16
    msnr_ylut11:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER2_OFS 0x00e8
REGDEF_BEGIN(Y_NR_REGISTER2)
    REGDEF_BIT(msnr_ylut8 ,        8)
    REGDEF_BIT(msnr_ylut9 ,        8)
    REGDEF_BIT(msnr_ylut10,        8)
    REGDEF_BIT(msnr_ylut11,        8)
REGDEF_END(Y_NR_REGISTER2)


/*
    msnr_ylut12:    [0x0, 0xff],			bits : 7_0
    msnr_ylut13:    [0x0, 0xff],			bits : 15_8
    msnr_ylut14:    [0x0, 0xff],			bits : 23_16
    msnr_ylut15:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER3_OFS 0x00ec
REGDEF_BEGIN(Y_NR_REGISTER3)
    REGDEF_BIT(msnr_ylut12,        8)
    REGDEF_BIT(msnr_ylut13,        8)
    REGDEF_BIT(msnr_ylut14,        8)
    REGDEF_BIT(msnr_ylut15,        8)
REGDEF_END(Y_NR_REGISTER3)


/*
    msnr_ylut16:    [0x0, 0xff],			bits : 7_0
    msnr_ylut17:    [0x0, 0xff],			bits : 15_8
    msnr_ylut18:    [0x0, 0xff],			bits : 23_16
    msnr_ylut19:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER4_OFS 0x00f0
REGDEF_BEGIN(Y_NR_REGISTER4)
    REGDEF_BIT(msnr_ylut16,        8)
    REGDEF_BIT(msnr_ylut17,        8)
    REGDEF_BIT(msnr_ylut18,        8)
    REGDEF_BIT(msnr_ylut19,        8)
REGDEF_END(Y_NR_REGISTER4)


/*
    msnr_ylut20:    [0x0, 0xff],			bits : 7_0
    msnr_ylut21:    [0x0, 0xff],			bits : 15_8
    msnr_ylut22:    [0x0, 0xff],			bits : 23_16
    msnr_ylut23:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER5_OFS 0x00f4
REGDEF_BEGIN(Y_NR_REGISTER5)
    REGDEF_BIT(msnr_ylut20,        8)
    REGDEF_BIT(msnr_ylut21,        8)
    REGDEF_BIT(msnr_ylut22,        8)
    REGDEF_BIT(msnr_ylut23,        8)
REGDEF_END(Y_NR_REGISTER5)


/*
    msnr_ylut24:    [0x0, 0xff],			bits : 7_0
    msnr_ylut25:    [0x0, 0xff],			bits : 15_8
    msnr_ylut26:    [0x0, 0xff],			bits : 23_16
    msnr_ylut27:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER6_OFS 0x00f8
REGDEF_BEGIN(Y_NR_REGISTER6)
    REGDEF_BIT(msnr_ylut24,        8)
    REGDEF_BIT(msnr_ylut25,        8)
    REGDEF_BIT(msnr_ylut26,        8)
    REGDEF_BIT(msnr_ylut27,        8)
REGDEF_END(Y_NR_REGISTER6)


/*
    msnr_ylut28:    [0x0, 0xff],			bits : 7_0
    msnr_ylut29:    [0x0, 0xff],			bits : 15_8
    msnr_ylut30:    [0x0, 0xff],			bits : 23_16
    msnr_ylut31:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER7_OFS 0x00fc
REGDEF_BEGIN(Y_NR_REGISTER7)
    REGDEF_BIT(msnr_ylut28,        8)
    REGDEF_BIT(msnr_ylut29,        8)
    REGDEF_BIT(msnr_ylut30,        8)
    REGDEF_BIT(msnr_ylut31,        8)
REGDEF_END(Y_NR_REGISTER7)


/*
    msnr_ylut32:    [0x0, 0xff],			bits : 7_0
    msnr_ylut33:    [0x0, 0xff],			bits : 15_8
    msnr_ylut34:    [0x0, 0xff],			bits : 23_16
    msnr_ylut35:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER8_OFS 0x0100
REGDEF_BEGIN(Y_NR_REGISTER8)
    REGDEF_BIT(msnr_ylut32,        8)
    REGDEF_BIT(msnr_ylut33,        8)
    REGDEF_BIT(msnr_ylut34,        8)
    REGDEF_BIT(msnr_ylut35,        8)
REGDEF_END(Y_NR_REGISTER8)


/*
    msnr_ylut36:    [0x0, 0xff],			bits : 7_0
    msnr_ylut37:    [0x0, 0xff],			bits : 15_8
    msnr_ylut38:    [0x0, 0xff],			bits : 23_16
    msnr_ylut39:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER9_OFS 0x0104
REGDEF_BEGIN(Y_NR_REGISTER9)
    REGDEF_BIT(msnr_ylut36,        8)
    REGDEF_BIT(msnr_ylut37,        8)
    REGDEF_BIT(msnr_ylut38,        8)
    REGDEF_BIT(msnr_ylut39,        8)
REGDEF_END(Y_NR_REGISTER9)


/*
    msnr_ylut40:    [0x0, 0xff],			bits : 7_0
    msnr_ylut41:    [0x0, 0xff],			bits : 15_8
    msnr_ylut42:    [0x0, 0xff],			bits : 23_16
    msnr_ylut43:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER10_OFS 0x0108
REGDEF_BEGIN(Y_NR_REGISTER10)
    REGDEF_BIT(msnr_ylut40,        8)
    REGDEF_BIT(msnr_ylut41,        8)
    REGDEF_BIT(msnr_ylut42,        8)
    REGDEF_BIT(msnr_ylut43,        8)
REGDEF_END(Y_NR_REGISTER10)


/*
    msnr_ylut44:    [0x0, 0xff],			bits : 7_0
    msnr_ylut45:    [0x0, 0xff],			bits : 15_8
    msnr_ylut46:    [0x0, 0xff],			bits : 23_16
    msnr_ylut47:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER11_OFS 0x010c
REGDEF_BEGIN(Y_NR_REGISTER11)
    REGDEF_BIT(msnr_ylut44,        8)
    REGDEF_BIT(msnr_ylut45,        8)
    REGDEF_BIT(msnr_ylut46,        8)
    REGDEF_BIT(msnr_ylut47,        8)
REGDEF_END(Y_NR_REGISTER11)


/*
    msnr_ylut48:    [0x0, 0xff],			bits : 7_0
    msnr_ylut49:    [0x0, 0xff],			bits : 15_8
    msnr_ylut50:    [0x0, 0xff],			bits : 23_16
    msnr_ylut51:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER12_OFS 0x0110
REGDEF_BEGIN(Y_NR_REGISTER12)
    REGDEF_BIT(msnr_ylut48,        8)
    REGDEF_BIT(msnr_ylut49,        8)
    REGDEF_BIT(msnr_ylut50,        8)
    REGDEF_BIT(msnr_ylut51,        8)
REGDEF_END(Y_NR_REGISTER12)


/*
    msnr_ylut52:    [0x0, 0xff],			bits : 7_0
    msnr_ylut53:    [0x0, 0xff],			bits : 15_8
    msnr_ylut54:    [0x0, 0xff],			bits : 23_16
    msnr_ylut55:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER13_OFS 0x0114
REGDEF_BEGIN(Y_NR_REGISTER13)
    REGDEF_BIT(msnr_ylut52,        8)
    REGDEF_BIT(msnr_ylut53,        8)
    REGDEF_BIT(msnr_ylut54,        8)
    REGDEF_BIT(msnr_ylut55,        8)
REGDEF_END(Y_NR_REGISTER13)


/*
    msnr_ylut56:    [0x0, 0xff],			bits : 7_0
    msnr_ylut57:    [0x0, 0xff],			bits : 15_8
    msnr_ylut58:    [0x0, 0xff],			bits : 23_16
    msnr_ylut59:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER14_OFS 0x0118
REGDEF_BEGIN(Y_NR_REGISTER14)
    REGDEF_BIT(msnr_ylut56,        8)
    REGDEF_BIT(msnr_ylut57,        8)
    REGDEF_BIT(msnr_ylut58,        8)
    REGDEF_BIT(msnr_ylut59,        8)
REGDEF_END(Y_NR_REGISTER14)


/*
    msnr_ylut60:    [0x0, 0xff],			bits : 7_0
    msnr_ylut61:    [0x0, 0xff],			bits : 15_8
    msnr_ylut62:    [0x0, 0xff],			bits : 23_16
    msnr_ylut63:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER15_OFS 0x011c
REGDEF_BEGIN(Y_NR_REGISTER15)
    REGDEF_BIT(msnr_ylut60,        8)
    REGDEF_BIT(msnr_ylut61,        8)
    REGDEF_BIT(msnr_ylut62,        8)
    REGDEF_BIT(msnr_ylut63,        8)
REGDEF_END(Y_NR_REGISTER15)


/*
    msnr_ylut64:    [0x0, 0xff],			bits : 7_0
    msnr_ylut65:    [0x0, 0xff],			bits : 15_8
    msnr_ylut66:    [0x0, 0xff],			bits : 23_16
    msnr_ylut67:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER16_OFS 0x0120
REGDEF_BEGIN(Y_NR_REGISTER16)
    REGDEF_BIT(msnr_ylut64,        8)
    REGDEF_BIT(msnr_ylut65,        8)
    REGDEF_BIT(msnr_ylut66,        8)
    REGDEF_BIT(msnr_ylut67,        8)
REGDEF_END(Y_NR_REGISTER16)


/*
    msnr_ylut68:    [0x0, 0xff],			bits : 7_0
    msnr_ylut69:    [0x0, 0xff],			bits : 15_8
    msnr_ylut70:    [0x0, 0xff],			bits : 23_16
    msnr_ylut71:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER17_OFS 0x0124
REGDEF_BEGIN(Y_NR_REGISTER17)
    REGDEF_BIT(msnr_ylut68,        8)
    REGDEF_BIT(msnr_ylut69,        8)
    REGDEF_BIT(msnr_ylut70,        8)
    REGDEF_BIT(msnr_ylut71,        8)
REGDEF_END(Y_NR_REGISTER17)


/*
    msnr_ylut72:    [0x0, 0xff],			bits : 7_0
    msnr_ylut73:    [0x0, 0xff],			bits : 15_8
    msnr_ylut74:    [0x0, 0xff],			bits : 23_16
    msnr_ylut75:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER18_OFS 0x0128
REGDEF_BEGIN(Y_NR_REGISTER18)
    REGDEF_BIT(msnr_ylut72,        8)
    REGDEF_BIT(msnr_ylut73,        8)
    REGDEF_BIT(msnr_ylut74,        8)
    REGDEF_BIT(msnr_ylut75,        8)
REGDEF_END(Y_NR_REGISTER18)


/*
    msnr_ylut76:    [0x0, 0xff],			bits : 7_0
    msnr_ylut77:    [0x0, 0xff],			bits : 15_8
    msnr_ylut78:    [0x0, 0xff],			bits : 23_16
    msnr_ylut79:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER19_OFS 0x012c
REGDEF_BEGIN(Y_NR_REGISTER19)
    REGDEF_BIT(msnr_ylut76,        8)
    REGDEF_BIT(msnr_ylut77,        8)
    REGDEF_BIT(msnr_ylut78,        8)
    REGDEF_BIT(msnr_ylut79,        8)
REGDEF_END(Y_NR_REGISTER19)


/*
    msnr_ylut80:    [0x0, 0xff],			bits : 7_0
    msnr_ylut81:    [0x0, 0xff],			bits : 15_8
    msnr_ylut82:    [0x0, 0xff],			bits : 23_16
    msnr_ylut83:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER20_OFS 0x0130
REGDEF_BEGIN(Y_NR_REGISTER20)
    REGDEF_BIT(msnr_ylut80,        8)
    REGDEF_BIT(msnr_ylut81,        8)
    REGDEF_BIT(msnr_ylut82,        8)
    REGDEF_BIT(msnr_ylut83,        8)
REGDEF_END(Y_NR_REGISTER20)


/*
    msnr_ylut84:    [0x0, 0xff],			bits : 7_0
    msnr_ylut85:    [0x0, 0xff],			bits : 15_8
    msnr_ylut86:    [0x0, 0xff],			bits : 23_16
    msnr_ylut87:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER21_OFS 0x0134
REGDEF_BEGIN(Y_NR_REGISTER21)
    REGDEF_BIT(msnr_ylut84,        8)
    REGDEF_BIT(msnr_ylut85,        8)
    REGDEF_BIT(msnr_ylut86,        8)
    REGDEF_BIT(msnr_ylut87,        8)
REGDEF_END(Y_NR_REGISTER21)


/*
    msnr_ylut88:    [0x0, 0xff],			bits : 7_0
    msnr_ylut89:    [0x0, 0xff],			bits : 15_8
    msnr_ylut90:    [0x0, 0xff],			bits : 23_16
    msnr_ylut91:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER22_OFS 0x0138
REGDEF_BEGIN(Y_NR_REGISTER22)
    REGDEF_BIT(msnr_ylut88,        8)
    REGDEF_BIT(msnr_ylut89,        8)
    REGDEF_BIT(msnr_ylut90,        8)
    REGDEF_BIT(msnr_ylut91,        8)
REGDEF_END(Y_NR_REGISTER22)


/*
    msnr_ylut92:    [0x0, 0xff],			bits : 7_0
    msnr_ylut93:    [0x0, 0xff],			bits : 15_8
    msnr_ylut94:    [0x0, 0xff],			bits : 23_16
    msnr_ylut95:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER23_OFS 0x013c
REGDEF_BEGIN(Y_NR_REGISTER23)
    REGDEF_BIT(msnr_ylut92,        8)
    REGDEF_BIT(msnr_ylut93,        8)
    REGDEF_BIT(msnr_ylut94,        8)
    REGDEF_BIT(msnr_ylut95,        8)
REGDEF_END(Y_NR_REGISTER23)


/*
    msnr_ylut96:    [0x0, 0xff],			bits : 7_0
    msnr_ylut97:    [0x0, 0xff],			bits : 15_8
    msnr_ylut98:    [0x0, 0xff],			bits : 23_16
    msnr_ylut99:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER24_OFS 0x0140
REGDEF_BEGIN(Y_NR_REGISTER24)
    REGDEF_BIT(msnr_ylut96,        8)
    REGDEF_BIT(msnr_ylut97,        8)
    REGDEF_BIT(msnr_ylut98,        8)
    REGDEF_BIT(msnr_ylut99,        8)
REGDEF_END(Y_NR_REGISTER24)


/*
    msnr_ylut100:    [0x0, 0xff],			bits : 7_0
    msnr_ylut101:    [0x0, 0xff],			bits : 15_8
    msnr_ylut102:    [0x0, 0xff],			bits : 23_16
    msnr_ylut103:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER25_OFS 0x0144
REGDEF_BEGIN(Y_NR_REGISTER25)
    REGDEF_BIT(msnr_ylut100,        8)
    REGDEF_BIT(msnr_ylut101,        8)
    REGDEF_BIT(msnr_ylut102,        8)
    REGDEF_BIT(msnr_ylut103,        8)
REGDEF_END(Y_NR_REGISTER25)


/*
    msnr_ylut104:    [0x0, 0xff],			bits : 7_0
    msnr_ylut105:    [0x0, 0xff],			bits : 15_8
    msnr_ylut106:    [0x0, 0xff],			bits : 23_16
    msnr_ylut107:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER26_OFS 0x0148
REGDEF_BEGIN(Y_NR_REGISTER26)
    REGDEF_BIT(msnr_ylut104,        8)
    REGDEF_BIT(msnr_ylut105,        8)
    REGDEF_BIT(msnr_ylut106,        8)
    REGDEF_BIT(msnr_ylut107,        8)
REGDEF_END(Y_NR_REGISTER26)


/*
    msnr_ylut108:    [0x0, 0xff],			bits : 7_0
    msnr_ylut109:    [0x0, 0xff],			bits : 15_8
    msnr_ylut110:    [0x0, 0xff],			bits : 23_16
    msnr_ylut111:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER27_OFS 0x014c
REGDEF_BEGIN(Y_NR_REGISTER27)
    REGDEF_BIT(msnr_ylut108,        8)
    REGDEF_BIT(msnr_ylut109,        8)
    REGDEF_BIT(msnr_ylut110,        8)
    REGDEF_BIT(msnr_ylut111,        8)
REGDEF_END(Y_NR_REGISTER27)


/*
    msnr_ylut112:    [0x0, 0xff],			bits : 7_0
    msnr_ylut113:    [0x0, 0xff],			bits : 15_8
    msnr_ylut114:    [0x0, 0xff],			bits : 23_16
    msnr_ylut115:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER28_OFS 0x0150
REGDEF_BEGIN(Y_NR_REGISTER28)
    REGDEF_BIT(msnr_ylut112,        8)
    REGDEF_BIT(msnr_ylut113,        8)
    REGDEF_BIT(msnr_ylut114,        8)
    REGDEF_BIT(msnr_ylut115,        8)
REGDEF_END(Y_NR_REGISTER28)


/*
    msnr_ylut116:    [0x0, 0xff],			bits : 7_0
    msnr_ylut117:    [0x0, 0xff],			bits : 15_8
    msnr_ylut118:    [0x0, 0xff],			bits : 23_16
    msnr_ylut119:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER29_OFS 0x0154
REGDEF_BEGIN(Y_NR_REGISTER29)
    REGDEF_BIT(msnr_ylut116,        8)
    REGDEF_BIT(msnr_ylut117,        8)
    REGDEF_BIT(msnr_ylut118,        8)
    REGDEF_BIT(msnr_ylut119,        8)
REGDEF_END(Y_NR_REGISTER29)


/*
    msnr_ylut120:    [0x0, 0xff],			bits : 7_0
    msnr_ylut121:    [0x0, 0xff],			bits : 15_8
    msnr_ylut122:    [0x0, 0xff],			bits : 23_16
    msnr_ylut123:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER30_OFS 0x0158
REGDEF_BEGIN(Y_NR_REGISTER30)
    REGDEF_BIT(msnr_ylut120,        8)
    REGDEF_BIT(msnr_ylut121,        8)
    REGDEF_BIT(msnr_ylut122,        8)
    REGDEF_BIT(msnr_ylut123,        8)
REGDEF_END(Y_NR_REGISTER30)


/*
    msnr_ylut124:    [0x0, 0xff],			bits : 7_0
    msnr_ylut125:    [0x0, 0xff],			bits : 15_8
    msnr_ylut126:    [0x0, 0xff],			bits : 23_16
    msnr_ylut127:    [0x0, 0xff],			bits : 31_24
*/
#define Y_NR_REGISTER31_OFS 0x015c
REGDEF_BEGIN(Y_NR_REGISTER31)
    REGDEF_BIT(msnr_ylut124,        8)
    REGDEF_BIT(msnr_ylut125,        8)
    REGDEF_BIT(msnr_ylut126,        8)
    REGDEF_BIT(msnr_ylut127,        8)
REGDEF_END(Y_NR_REGISTER31)


/*
    msnr_ulut0:    [0x0, 0xff],			bits : 7_0
    msnr_ulut1:    [0x0, 0xff],			bits : 15_8
    msnr_ulut2:    [0x0, 0xff],			bits : 23_16
    msnr_ulut3:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER0_OFS 0x0160
REGDEF_BEGIN(U_NR_REGISTER0)
    REGDEF_BIT(msnr_ulut0,        8)
    REGDEF_BIT(msnr_ulut1,        8)
    REGDEF_BIT(msnr_ulut2,        8)
    REGDEF_BIT(msnr_ulut3,        8)
REGDEF_END(U_NR_REGISTER0)


/*
    msnr_ulut4:    [0x0, 0xff],			bits : 7_0
    msnr_ulut5:    [0x0, 0xff],			bits : 15_8
    msnr_ulut6:    [0x0, 0xff],			bits : 23_16
    msnr_ulut7:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER1_OFS 0x0164
REGDEF_BEGIN(U_NR_REGISTER1)
    REGDEF_BIT(msnr_ulut4,        8)
    REGDEF_BIT(msnr_ulut5,        8)
    REGDEF_BIT(msnr_ulut6,        8)
    REGDEF_BIT(msnr_ulut7,        8)
REGDEF_END(U_NR_REGISTER1)


/*
    msnr_ulut8 :    [0x0, 0xff],			bits : 7_0
    msnr_ulut9 :    [0x0, 0xff],			bits : 15_8
    msnr_ulut10:    [0x0, 0xff],			bits : 23_16
    msnr_ulut11:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER2_OFS 0x0168
REGDEF_BEGIN(U_NR_REGISTER2)
    REGDEF_BIT(msnr_ulut8 ,        8)
    REGDEF_BIT(msnr_ulut9 ,        8)
    REGDEF_BIT(msnr_ulut10,        8)
    REGDEF_BIT(msnr_ulut11,        8)
REGDEF_END(U_NR_REGISTER2)


/*
    msnr_ulut12:    [0x0, 0xff],			bits : 7_0
    msnr_ulut13:    [0x0, 0xff],			bits : 15_8
    msnr_ulut14:    [0x0, 0xff],			bits : 23_16
    msnr_ulut15:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER3_OFS 0x016c
REGDEF_BEGIN(U_NR_REGISTER3)
    REGDEF_BIT(msnr_ulut12,        8)
    REGDEF_BIT(msnr_ulut13,        8)
    REGDEF_BIT(msnr_ulut14,        8)
    REGDEF_BIT(msnr_ulut15,        8)
REGDEF_END(U_NR_REGISTER3)


/*
    msnr_ulut16:    [0x0, 0xff],			bits : 7_0
    msnr_ulut17:    [0x0, 0xff],			bits : 15_8
    msnr_ulut18:    [0x0, 0xff],			bits : 23_16
    msnr_ulut19:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER4_OFS 0x0170
REGDEF_BEGIN(U_NR_REGISTER4)
    REGDEF_BIT(msnr_ulut16,        8)
    REGDEF_BIT(msnr_ulut17,        8)
    REGDEF_BIT(msnr_ulut18,        8)
    REGDEF_BIT(msnr_ulut19,        8)
REGDEF_END(U_NR_REGISTER4)


/*
    msnr_ulut20:    [0x0, 0xff],			bits : 7_0
    msnr_ulut21:    [0x0, 0xff],			bits : 15_8
    msnr_ulut22:    [0x0, 0xff],			bits : 23_16
    msnr_ulut23:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER5_OFS 0x0174
REGDEF_BEGIN(U_NR_REGISTER5)
    REGDEF_BIT(msnr_ulut20,        8)
    REGDEF_BIT(msnr_ulut21,        8)
    REGDEF_BIT(msnr_ulut22,        8)
    REGDEF_BIT(msnr_ulut23,        8)
REGDEF_END(U_NR_REGISTER5)


/*
    msnr_ulut24:    [0x0, 0xff],			bits : 7_0
    msnr_ulut25:    [0x0, 0xff],			bits : 15_8
    msnr_ulut26:    [0x0, 0xff],			bits : 23_16
    msnr_ulut27:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER6_OFS 0x0178
REGDEF_BEGIN(U_NR_REGISTER6)
    REGDEF_BIT(msnr_ulut24,        8)
    REGDEF_BIT(msnr_ulut25,        8)
    REGDEF_BIT(msnr_ulut26,        8)
    REGDEF_BIT(msnr_ulut27,        8)
REGDEF_END(U_NR_REGISTER6)


/*
    msnr_ulut28:    [0x0, 0xff],			bits : 7_0
    msnr_ulut29:    [0x0, 0xff],			bits : 15_8
    msnr_ulut30:    [0x0, 0xff],			bits : 23_16
    msnr_ulut31:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER7_OFS 0x017c
REGDEF_BEGIN(U_NR_REGISTER7)
    REGDEF_BIT(msnr_ulut28,        8)
    REGDEF_BIT(msnr_ulut29,        8)
    REGDEF_BIT(msnr_ulut30,        8)
    REGDEF_BIT(msnr_ulut31,        8)
REGDEF_END(U_NR_REGISTER7)


/*
    msnr_ulut32:    [0x0, 0xff],			bits : 7_0
    msnr_ulut33:    [0x0, 0xff],			bits : 15_8
    msnr_ulut34:    [0x0, 0xff],			bits : 23_16
    msnr_ulut35:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER8_OFS 0x0180
REGDEF_BEGIN(U_NR_REGISTER8)
    REGDEF_BIT(msnr_ulut32,        8)
    REGDEF_BIT(msnr_ulut33,        8)
    REGDEF_BIT(msnr_ulut34,        8)
    REGDEF_BIT(msnr_ulut35,        8)
REGDEF_END(U_NR_REGISTER8)


/*
    msnr_ulut36:    [0x0, 0xff],			bits : 7_0
    msnr_ulut37:    [0x0, 0xff],			bits : 15_8
    msnr_ulut38:    [0x0, 0xff],			bits : 23_16
    msnr_ulut39:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER9_OFS 0x0184
REGDEF_BEGIN(U_NR_REGISTER9)
    REGDEF_BIT(msnr_ulut36,        8)
    REGDEF_BIT(msnr_ulut37,        8)
    REGDEF_BIT(msnr_ulut38,        8)
    REGDEF_BIT(msnr_ulut39,        8)
REGDEF_END(U_NR_REGISTER9)


/*
    msnr_ulut40:    [0x0, 0xff],			bits : 7_0
    msnr_ulut41:    [0x0, 0xff],			bits : 15_8
    msnr_ulut42:    [0x0, 0xff],			bits : 23_16
    msnr_ulut43:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER10_OFS 0x0188
REGDEF_BEGIN(U_NR_REGISTER10)
    REGDEF_BIT(msnr_ulut40,        8)
    REGDEF_BIT(msnr_ulut41,        8)
    REGDEF_BIT(msnr_ulut42,        8)
    REGDEF_BIT(msnr_ulut43,        8)
REGDEF_END(U_NR_REGISTER10)


/*
    msnr_ulut44:    [0x0, 0xff],			bits : 7_0
    msnr_ulut45:    [0x0, 0xff],			bits : 15_8
    msnr_ulut46:    [0x0, 0xff],			bits : 23_16
    msnr_ulut47:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER11_OFS 0x018c
REGDEF_BEGIN(U_NR_REGISTER11)
    REGDEF_BIT(msnr_ulut44,        8)
    REGDEF_BIT(msnr_ulut45,        8)
    REGDEF_BIT(msnr_ulut46,        8)
    REGDEF_BIT(msnr_ulut47,        8)
REGDEF_END(U_NR_REGISTER11)


/*
    msnr_ulut48:    [0x0, 0xff],			bits : 7_0
    msnr_ulut49:    [0x0, 0xff],			bits : 15_8
    msnr_ulut50:    [0x0, 0xff],			bits : 23_16
    msnr_ulut51:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER12_OFS 0x0190
REGDEF_BEGIN(U_NR_REGISTER12)
    REGDEF_BIT(msnr_ulut48,        8)
    REGDEF_BIT(msnr_ulut49,        8)
    REGDEF_BIT(msnr_ulut50,        8)
    REGDEF_BIT(msnr_ulut51,        8)
REGDEF_END(U_NR_REGISTER12)


/*
    msnr_ulut52:    [0x0, 0xff],			bits : 7_0
    msnr_ulut53:    [0x0, 0xff],			bits : 15_8
    msnr_ulut54:    [0x0, 0xff],			bits : 23_16
    msnr_ulut55:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER13_OFS 0x0194
REGDEF_BEGIN(U_NR_REGISTER13)
    REGDEF_BIT(msnr_ulut52,        8)
    REGDEF_BIT(msnr_ulut53,        8)
    REGDEF_BIT(msnr_ulut54,        8)
    REGDEF_BIT(msnr_ulut55,        8)
REGDEF_END(U_NR_REGISTER13)


/*
    msnr_ulut56:    [0x0, 0xff],			bits : 7_0
    msnr_ulut57:    [0x0, 0xff],			bits : 15_8
    msnr_ulut58:    [0x0, 0xff],			bits : 23_16
    msnr_ulut59:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER14_OFS 0x0198
REGDEF_BEGIN(U_NR_REGISTER14)
    REGDEF_BIT(msnr_ulut56,        8)
    REGDEF_BIT(msnr_ulut57,        8)
    REGDEF_BIT(msnr_ulut58,        8)
    REGDEF_BIT(msnr_ulut59,        8)
REGDEF_END(U_NR_REGISTER14)


/*
    msnr_ulut60:    [0x0, 0xff],			bits : 7_0
    msnr_ulut61:    [0x0, 0xff],			bits : 15_8
    msnr_ulut62:    [0x0, 0xff],			bits : 23_16
    msnr_ulut63:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER15_OFS 0x019c
REGDEF_BEGIN(U_NR_REGISTER15)
    REGDEF_BIT(msnr_ulut60,        8)
    REGDEF_BIT(msnr_ulut61,        8)
    REGDEF_BIT(msnr_ulut62,        8)
    REGDEF_BIT(msnr_ulut63,        8)
REGDEF_END(U_NR_REGISTER15)


/*
    msnr_ulut64:    [0x0, 0xff],			bits : 7_0
    msnr_ulut65:    [0x0, 0xff],			bits : 15_8
    msnr_ulut66:    [0x0, 0xff],			bits : 23_16
    msnr_ulut67:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER16_OFS 0x01a0
REGDEF_BEGIN(U_NR_REGISTER16)
    REGDEF_BIT(msnr_ulut64,        8)
    REGDEF_BIT(msnr_ulut65,        8)
    REGDEF_BIT(msnr_ulut66,        8)
    REGDEF_BIT(msnr_ulut67,        8)
REGDEF_END(U_NR_REGISTER16)


/*
    msnr_ulut68:    [0x0, 0xff],			bits : 7_0
    msnr_ulut69:    [0x0, 0xff],			bits : 15_8
    msnr_ulut70:    [0x0, 0xff],			bits : 23_16
    msnr_ulut71:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER17_OFS 0x01a4
REGDEF_BEGIN(U_NR_REGISTER17)
    REGDEF_BIT(msnr_ulut68,        8)
    REGDEF_BIT(msnr_ulut69,        8)
    REGDEF_BIT(msnr_ulut70,        8)
    REGDEF_BIT(msnr_ulut71,        8)
REGDEF_END(U_NR_REGISTER17)


/*
    msnr_ulut72:    [0x0, 0xff],			bits : 7_0
    msnr_ulut73:    [0x0, 0xff],			bits : 15_8
    msnr_ulut74:    [0x0, 0xff],			bits : 23_16
    msnr_ulut75:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER18_OFS 0x01a8
REGDEF_BEGIN(U_NR_REGISTER18)
    REGDEF_BIT(msnr_ulut72,        8)
    REGDEF_BIT(msnr_ulut73,        8)
    REGDEF_BIT(msnr_ulut74,        8)
    REGDEF_BIT(msnr_ulut75,        8)
REGDEF_END(U_NR_REGISTER18)


/*
    msnr_ulut76:    [0x0, 0xff],			bits : 7_0
    msnr_ulut77:    [0x0, 0xff],			bits : 15_8
    msnr_ulut78:    [0x0, 0xff],			bits : 23_16
    msnr_ulut79:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER19_OFS 0x01ac
REGDEF_BEGIN(U_NR_REGISTER19)
    REGDEF_BIT(msnr_ulut76,        8)
    REGDEF_BIT(msnr_ulut77,        8)
    REGDEF_BIT(msnr_ulut78,        8)
    REGDEF_BIT(msnr_ulut79,        8)
REGDEF_END(U_NR_REGISTER19)


/*
    msnr_ulut80:    [0x0, 0xff],			bits : 7_0
    msnr_ulut81:    [0x0, 0xff],			bits : 15_8
    msnr_ulut82:    [0x0, 0xff],			bits : 23_16
    msnr_ulut83:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER20_OFS 0x01b0
REGDEF_BEGIN(U_NR_REGISTER20)
    REGDEF_BIT(msnr_ulut80,        8)
    REGDEF_BIT(msnr_ulut81,        8)
    REGDEF_BIT(msnr_ulut82,        8)
    REGDEF_BIT(msnr_ulut83,        8)
REGDEF_END(U_NR_REGISTER20)


/*
    msnr_ulut84:    [0x0, 0xff],			bits : 7_0
    msnr_ulut85:    [0x0, 0xff],			bits : 15_8
    msnr_ulut86:    [0x0, 0xff],			bits : 23_16
    msnr_ulut87:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER21_OFS 0x01b4
REGDEF_BEGIN(U_NR_REGISTER21)
    REGDEF_BIT(msnr_ulut84,        8)
    REGDEF_BIT(msnr_ulut85,        8)
    REGDEF_BIT(msnr_ulut86,        8)
    REGDEF_BIT(msnr_ulut87,        8)
REGDEF_END(U_NR_REGISTER21)


/*
    msnr_ulut88:    [0x0, 0xff],			bits : 7_0
    msnr_ulut89:    [0x0, 0xff],			bits : 15_8
    msnr_ulut90:    [0x0, 0xff],			bits : 23_16
    msnr_ulut91:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER22_OFS 0x01b8
REGDEF_BEGIN(U_NR_REGISTER22)
    REGDEF_BIT(msnr_ulut88,        8)
    REGDEF_BIT(msnr_ulut89,        8)
    REGDEF_BIT(msnr_ulut90,        8)
    REGDEF_BIT(msnr_ulut91,        8)
REGDEF_END(U_NR_REGISTER22)


/*
    msnr_ulut92:    [0x0, 0xff],			bits : 7_0
    msnr_ulut93:    [0x0, 0xff],			bits : 15_8
    msnr_ulut94:    [0x0, 0xff],			bits : 23_16
    msnr_ulut95:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER23_OFS 0x01bc
REGDEF_BEGIN(U_NR_REGISTER23)
    REGDEF_BIT(msnr_ulut92,        8)
    REGDEF_BIT(msnr_ulut93,        8)
    REGDEF_BIT(msnr_ulut94,        8)
    REGDEF_BIT(msnr_ulut95,        8)
REGDEF_END(U_NR_REGISTER23)


/*
    msnr_ulut96:    [0x0, 0xff],			bits : 7_0
    msnr_ulut97:    [0x0, 0xff],			bits : 15_8
    msnr_ulut98:    [0x0, 0xff],			bits : 23_16
    msnr_ulut99:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER24_OFS 0x01c0
REGDEF_BEGIN(U_NR_REGISTER24)
    REGDEF_BIT(msnr_ulut96,        8)
    REGDEF_BIT(msnr_ulut97,        8)
    REGDEF_BIT(msnr_ulut98,        8)
    REGDEF_BIT(msnr_ulut99,        8)
REGDEF_END(U_NR_REGISTER24)


/*
    msnr_ulut100:    [0x0, 0xff],			bits : 7_0
    msnr_ulut101:    [0x0, 0xff],			bits : 15_8
    msnr_ulut102:    [0x0, 0xff],			bits : 23_16
    msnr_ulut103:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER25_OFS 0x01c4
REGDEF_BEGIN(U_NR_REGISTER25)
    REGDEF_BIT(msnr_ulut100,        8)
    REGDEF_BIT(msnr_ulut101,        8)
    REGDEF_BIT(msnr_ulut102,        8)
    REGDEF_BIT(msnr_ulut103,        8)
REGDEF_END(U_NR_REGISTER25)


/*
    msnr_ulut104:    [0x0, 0xff],			bits : 7_0
    msnr_ulut105:    [0x0, 0xff],			bits : 15_8
    msnr_ulut106:    [0x0, 0xff],			bits : 23_16
    msnr_ulut107:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER26_OFS 0x01c8
REGDEF_BEGIN(U_NR_REGISTER26)
    REGDEF_BIT(msnr_ulut104,        8)
    REGDEF_BIT(msnr_ulut105,        8)
    REGDEF_BIT(msnr_ulut106,        8)
    REGDEF_BIT(msnr_ulut107,        8)
REGDEF_END(U_NR_REGISTER26)


/*
    msnr_ulut108:    [0x0, 0xff],			bits : 7_0
    msnr_ulut109:    [0x0, 0xff],			bits : 15_8
    msnr_ulut110:    [0x0, 0xff],			bits : 23_16
    msnr_ulut111:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER27_OFS 0x01cc
REGDEF_BEGIN(U_NR_REGISTER27)
    REGDEF_BIT(msnr_ulut108,        8)
    REGDEF_BIT(msnr_ulut109,        8)
    REGDEF_BIT(msnr_ulut110,        8)
    REGDEF_BIT(msnr_ulut111,        8)
REGDEF_END(U_NR_REGISTER27)


/*
    msnr_ulut112:    [0x0, 0xff],			bits : 7_0
    msnr_ulut113:    [0x0, 0xff],			bits : 15_8
    msnr_ulut114:    [0x0, 0xff],			bits : 23_16
    msnr_ulut115:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER28_OFS 0x01d0
REGDEF_BEGIN(U_NR_REGISTER28)
    REGDEF_BIT(msnr_ulut112,        8)
    REGDEF_BIT(msnr_ulut113,        8)
    REGDEF_BIT(msnr_ulut114,        8)
    REGDEF_BIT(msnr_ulut115,        8)
REGDEF_END(U_NR_REGISTER28)


/*
    msnr_ulut116:    [0x0, 0xff],			bits : 7_0
    msnr_ulut117:    [0x0, 0xff],			bits : 15_8
    msnr_ulut118:    [0x0, 0xff],			bits : 23_16
    msnr_ulut119:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER29_OFS 0x01d4
REGDEF_BEGIN(U_NR_REGISTER29)
    REGDEF_BIT(msnr_ulut116,        8)
    REGDEF_BIT(msnr_ulut117,        8)
    REGDEF_BIT(msnr_ulut118,        8)
    REGDEF_BIT(msnr_ulut119,        8)
REGDEF_END(U_NR_REGISTER29)


/*
    msnr_ulut120:    [0x0, 0xff],			bits : 7_0
    msnr_ulut121:    [0x0, 0xff],			bits : 15_8
    msnr_ulut122:    [0x0, 0xff],			bits : 23_16
    msnr_ulut123:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER30_OFS 0x01d8
REGDEF_BEGIN(U_NR_REGISTER30)
    REGDEF_BIT(msnr_ulut120,        8)
    REGDEF_BIT(msnr_ulut121,        8)
    REGDEF_BIT(msnr_ulut122,        8)
    REGDEF_BIT(msnr_ulut123,        8)
REGDEF_END(U_NR_REGISTER30)


/*
    msnr_ulut124:    [0x0, 0xff],			bits : 7_0
    msnr_ulut125:    [0x0, 0xff],			bits : 15_8
    msnr_ulut126:    [0x0, 0xff],			bits : 23_16
    msnr_ulut127:    [0x0, 0xff],			bits : 31_24
*/
#define U_NR_REGISTER31_OFS 0x01dc
REGDEF_BEGIN(U_NR_REGISTER31)
    REGDEF_BIT(msnr_ulut124,        8)
    REGDEF_BIT(msnr_ulut125,        8)
    REGDEF_BIT(msnr_ulut126,        8)
    REGDEF_BIT(msnr_ulut127,        8)
REGDEF_END(U_NR_REGISTER31)


/*
    msnr_vlut0:    [0x0, 0xff],			bits : 7_0
    msnr_vlut1:    [0x0, 0xff],			bits : 15_8
    msnr_vlut2:    [0x0, 0xff],			bits : 23_16
    msnr_vlut3:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER0_OFS 0x01e0
REGDEF_BEGIN(V_NR_REGISTER0)
    REGDEF_BIT(msnr_vlut0,        8)
    REGDEF_BIT(msnr_vlut1,        8)
    REGDEF_BIT(msnr_vlut2,        8)
    REGDEF_BIT(msnr_vlut3,        8)
REGDEF_END(V_NR_REGISTER0)


/*
    msnr_vlut4:    [0x0, 0xff],			bits : 7_0
    msnr_vlut5:    [0x0, 0xff],			bits : 15_8
    msnr_vlut6:    [0x0, 0xff],			bits : 23_16
    msnr_vlut7:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER1_OFS 0x01e4
REGDEF_BEGIN(V_NR_REGISTER1)
    REGDEF_BIT(msnr_vlut4,        8)
    REGDEF_BIT(msnr_vlut5,        8)
    REGDEF_BIT(msnr_vlut6,        8)
    REGDEF_BIT(msnr_vlut7,        8)
REGDEF_END(V_NR_REGISTER1)


/*
    msnr_vlut8 :    [0x0, 0xff],			bits : 7_0
    msnr_vlut9 :    [0x0, 0xff],			bits : 15_8
    msnr_vlut10:    [0x0, 0xff],			bits : 23_16
    msnr_vlut11:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER2_OFS 0x01e8
REGDEF_BEGIN(V_NR_REGISTER2)
    REGDEF_BIT(msnr_vlut8 ,        8)
    REGDEF_BIT(msnr_vlut9 ,        8)
    REGDEF_BIT(msnr_vlut10,        8)
    REGDEF_BIT(msnr_vlut11,        8)
REGDEF_END(V_NR_REGISTER2)


/*
    msnr_vlut12:    [0x0, 0xff],			bits : 7_0
    msnr_vlut13:    [0x0, 0xff],			bits : 15_8
    msnr_vlut14:    [0x0, 0xff],			bits : 23_16
    msnr_vlut15:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER3_OFS 0x01ec
REGDEF_BEGIN(V_NR_REGISTER3)
    REGDEF_BIT(msnr_vlut12,        8)
    REGDEF_BIT(msnr_vlut13,        8)
    REGDEF_BIT(msnr_vlut14,        8)
    REGDEF_BIT(msnr_vlut15,        8)
REGDEF_END(V_NR_REGISTER3)


/*
    msnr_vlut16:    [0x0, 0xff],			bits : 7_0
    msnr_vlut17:    [0x0, 0xff],			bits : 15_8
    msnr_vlut18:    [0x0, 0xff],			bits : 23_16
    msnr_vlut19:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER4_OFS 0x01f0
REGDEF_BEGIN(V_NR_REGISTER4)
    REGDEF_BIT(msnr_vlut16,        8)
    REGDEF_BIT(msnr_vlut17,        8)
    REGDEF_BIT(msnr_vlut18,        8)
    REGDEF_BIT(msnr_vlut19,        8)
REGDEF_END(V_NR_REGISTER4)


/*
    msnr_vlut20:    [0x0, 0xff],			bits : 7_0
    msnr_vlut21:    [0x0, 0xff],			bits : 15_8
    msnr_vlut22:    [0x0, 0xff],			bits : 23_16
    msnr_vlut23:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER5_OFS 0x01f4
REGDEF_BEGIN(V_NR_REGISTER5)
    REGDEF_BIT(msnr_vlut20,        8)
    REGDEF_BIT(msnr_vlut21,        8)
    REGDEF_BIT(msnr_vlut22,        8)
    REGDEF_BIT(msnr_vlut23,        8)
REGDEF_END(V_NR_REGISTER5)


/*
    msnr_vlut24:    [0x0, 0xff],			bits : 7_0
    msnr_vlut25:    [0x0, 0xff],			bits : 15_8
    msnr_vlut26:    [0x0, 0xff],			bits : 23_16
    msnr_vlut27:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER6_OFS 0x01f8
REGDEF_BEGIN(V_NR_REGISTER6)
    REGDEF_BIT(msnr_vlut24,        8)
    REGDEF_BIT(msnr_vlut25,        8)
    REGDEF_BIT(msnr_vlut26,        8)
    REGDEF_BIT(msnr_vlut27,        8)
REGDEF_END(V_NR_REGISTER6)


/*
    msnr_vlut28:    [0x0, 0xff],			bits : 7_0
    msnr_vlut29:    [0x0, 0xff],			bits : 15_8
    msnr_vlut30:    [0x0, 0xff],			bits : 23_16
    msnr_vlut31:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER7_OFS 0x01fc
REGDEF_BEGIN(V_NR_REGISTER7)
    REGDEF_BIT(msnr_vlut28,        8)
    REGDEF_BIT(msnr_vlut29,        8)
    REGDEF_BIT(msnr_vlut30,        8)
    REGDEF_BIT(msnr_vlut31,        8)
REGDEF_END(V_NR_REGISTER7)


/*
    msnr_vlut32:    [0x0, 0xff],			bits : 7_0
    msnr_vlut33:    [0x0, 0xff],			bits : 15_8
    msnr_vlut34:    [0x0, 0xff],			bits : 23_16
    msnr_vlut35:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER8_OFS 0x0200
REGDEF_BEGIN(V_NR_REGISTER8)
    REGDEF_BIT(msnr_vlut32,        8)
    REGDEF_BIT(msnr_vlut33,        8)
    REGDEF_BIT(msnr_vlut34,        8)
    REGDEF_BIT(msnr_vlut35,        8)
REGDEF_END(V_NR_REGISTER8)


/*
    msnr_vlut36:    [0x0, 0xff],			bits : 7_0
    msnr_vlut37:    [0x0, 0xff],			bits : 15_8
    msnr_vlut38:    [0x0, 0xff],			bits : 23_16
    msnr_vlut39:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER9_OFS 0x0204
REGDEF_BEGIN(V_NR_REGISTER9)
    REGDEF_BIT(msnr_vlut36,        8)
    REGDEF_BIT(msnr_vlut37,        8)
    REGDEF_BIT(msnr_vlut38,        8)
    REGDEF_BIT(msnr_vlut39,        8)
REGDEF_END(V_NR_REGISTER9)


/*
    msnr_vlut40:    [0x0, 0xff],			bits : 7_0
    msnr_vlut41:    [0x0, 0xff],			bits : 15_8
    msnr_vlut42:    [0x0, 0xff],			bits : 23_16
    msnr_vlut43:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER10_OFS 0x0208
REGDEF_BEGIN(V_NR_REGISTER10)
    REGDEF_BIT(msnr_vlut40,        8)
    REGDEF_BIT(msnr_vlut41,        8)
    REGDEF_BIT(msnr_vlut42,        8)
    REGDEF_BIT(msnr_vlut43,        8)
REGDEF_END(V_NR_REGISTER10)


/*
    msnr_vlut44:    [0x0, 0xff],			bits : 7_0
    msnr_vlut45:    [0x0, 0xff],			bits : 15_8
    msnr_vlut46:    [0x0, 0xff],			bits : 23_16
    msnr_vlut47:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER11_OFS 0x020c
REGDEF_BEGIN(V_NR_REGISTER11)
    REGDEF_BIT(msnr_vlut44,        8)
    REGDEF_BIT(msnr_vlut45,        8)
    REGDEF_BIT(msnr_vlut46,        8)
    REGDEF_BIT(msnr_vlut47,        8)
REGDEF_END(V_NR_REGISTER11)


/*
    msnr_vlut48:    [0x0, 0xff],			bits : 7_0
    msnr_vlut49:    [0x0, 0xff],			bits : 15_8
    msnr_vlut50:    [0x0, 0xff],			bits : 23_16
    msnr_vlut51:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER12_OFS 0x0210
REGDEF_BEGIN(V_NR_REGISTER12)
    REGDEF_BIT(msnr_vlut48,        8)
    REGDEF_BIT(msnr_vlut49,        8)
    REGDEF_BIT(msnr_vlut50,        8)
    REGDEF_BIT(msnr_vlut51,        8)
REGDEF_END(V_NR_REGISTER12)


/*
    msnr_vlut52:    [0x0, 0xff],			bits : 7_0
    msnr_vlut53:    [0x0, 0xff],			bits : 15_8
    msnr_vlut54:    [0x0, 0xff],			bits : 23_16
    msnr_vlut55:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER13_OFS 0x0214
REGDEF_BEGIN(V_NR_REGISTER13)
    REGDEF_BIT(msnr_vlut52,        8)
    REGDEF_BIT(msnr_vlut53,        8)
    REGDEF_BIT(msnr_vlut54,        8)
    REGDEF_BIT(msnr_vlut55,        8)
REGDEF_END(V_NR_REGISTER13)


/*
    msnr_vlut56:    [0x0, 0xff],			bits : 7_0
    msnr_vlut57:    [0x0, 0xff],			bits : 15_8
    msnr_vlut58:    [0x0, 0xff],			bits : 23_16
    msnr_vlut59:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER14_OFS 0x0218
REGDEF_BEGIN(V_NR_REGISTER14)
    REGDEF_BIT(msnr_vlut56,        8)
    REGDEF_BIT(msnr_vlut57,        8)
    REGDEF_BIT(msnr_vlut58,        8)
    REGDEF_BIT(msnr_vlut59,        8)
REGDEF_END(V_NR_REGISTER14)


/*
    msnr_vlut60:    [0x0, 0xff],			bits : 7_0
    msnr_vlut61:    [0x0, 0xff],			bits : 15_8
    msnr_vlut62:    [0x0, 0xff],			bits : 23_16
    msnr_vlut63:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER15_OFS 0x021c
REGDEF_BEGIN(V_NR_REGISTER15)
    REGDEF_BIT(msnr_vlut60,        8)
    REGDEF_BIT(msnr_vlut61,        8)
    REGDEF_BIT(msnr_vlut62,        8)
    REGDEF_BIT(msnr_vlut63,        8)
REGDEF_END(V_NR_REGISTER15)


/*
    msnr_vlut64:    [0x0, 0xff],			bits : 7_0
    msnr_vlut65:    [0x0, 0xff],			bits : 15_8
    msnr_vlut66:    [0x0, 0xff],			bits : 23_16
    msnr_vlut67:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER16_OFS 0x0220
REGDEF_BEGIN(V_NR_REGISTER16)
    REGDEF_BIT(msnr_vlut64,        8)
    REGDEF_BIT(msnr_vlut65,        8)
    REGDEF_BIT(msnr_vlut66,        8)
    REGDEF_BIT(msnr_vlut67,        8)
REGDEF_END(V_NR_REGISTER16)


/*
    msnr_vlut68:    [0x0, 0xff],			bits : 7_0
    msnr_vlut69:    [0x0, 0xff],			bits : 15_8
    msnr_vlut70:    [0x0, 0xff],			bits : 23_16
    msnr_vlut71:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER17_OFS 0x0224
REGDEF_BEGIN(V_NR_REGISTER17)
    REGDEF_BIT(msnr_vlut68,        8)
    REGDEF_BIT(msnr_vlut69,        8)
    REGDEF_BIT(msnr_vlut70,        8)
    REGDEF_BIT(msnr_vlut71,        8)
REGDEF_END(V_NR_REGISTER17)


/*
    msnr_vlut72:    [0x0, 0xff],			bits : 7_0
    msnr_vlut73:    [0x0, 0xff],			bits : 15_8
    msnr_vlut74:    [0x0, 0xff],			bits : 23_16
    msnr_vlut75:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER18_OFS 0x0228
REGDEF_BEGIN(V_NR_REGISTER18)
    REGDEF_BIT(msnr_vlut72,        8)
    REGDEF_BIT(msnr_vlut73,        8)
    REGDEF_BIT(msnr_vlut74,        8)
    REGDEF_BIT(msnr_vlut75,        8)
REGDEF_END(V_NR_REGISTER18)


/*
    msnr_vlut76:    [0x0, 0xff],			bits : 7_0
    msnr_vlut77:    [0x0, 0xff],			bits : 15_8
    msnr_vlut78:    [0x0, 0xff],			bits : 23_16
    msnr_vlut79:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER19_OFS 0x022c
REGDEF_BEGIN(V_NR_REGISTER19)
    REGDEF_BIT(msnr_vlut76,        8)
    REGDEF_BIT(msnr_vlut77,        8)
    REGDEF_BIT(msnr_vlut78,        8)
    REGDEF_BIT(msnr_vlut79,        8)
REGDEF_END(V_NR_REGISTER19)


/*
    msnr_vlut80:    [0x0, 0xff],			bits : 7_0
    msnr_vlut81:    [0x0, 0xff],			bits : 15_8
    msnr_vlut82:    [0x0, 0xff],			bits : 23_16
    msnr_vlut83:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER20_OFS 0x0230
REGDEF_BEGIN(V_NR_REGISTER20)
    REGDEF_BIT(msnr_vlut80,        8)
    REGDEF_BIT(msnr_vlut81,        8)
    REGDEF_BIT(msnr_vlut82,        8)
    REGDEF_BIT(msnr_vlut83,        8)
REGDEF_END(V_NR_REGISTER20)


/*
    msnr_vlut84:    [0x0, 0xff],			bits : 7_0
    msnr_vlut85:    [0x0, 0xff],			bits : 15_8
    msnr_vlut86:    [0x0, 0xff],			bits : 23_16
    msnr_vlut87:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER21_OFS 0x0234
REGDEF_BEGIN(V_NR_REGISTER21)
    REGDEF_BIT(msnr_vlut84,        8)
    REGDEF_BIT(msnr_vlut85,        8)
    REGDEF_BIT(msnr_vlut86,        8)
    REGDEF_BIT(msnr_vlut87,        8)
REGDEF_END(V_NR_REGISTER21)


/*
    msnr_vlut88:    [0x0, 0xff],			bits : 7_0
    msnr_vlut89:    [0x0, 0xff],			bits : 15_8
    msnr_vlut90:    [0x0, 0xff],			bits : 23_16
    msnr_vlut91:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER22_OFS 0x0238
REGDEF_BEGIN(V_NR_REGISTER22)
    REGDEF_BIT(msnr_vlut88,        8)
    REGDEF_BIT(msnr_vlut89,        8)
    REGDEF_BIT(msnr_vlut90,        8)
    REGDEF_BIT(msnr_vlut91,        8)
REGDEF_END(V_NR_REGISTER22)


/*
    msnr_vlut92:    [0x0, 0xff],			bits : 7_0
    msnr_vlut93:    [0x0, 0xff],			bits : 15_8
    msnr_vlut94:    [0x0, 0xff],			bits : 23_16
    msnr_vlut95:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER23_OFS 0x023c
REGDEF_BEGIN(V_NR_REGISTER23)
    REGDEF_BIT(msnr_vlut92,        8)
    REGDEF_BIT(msnr_vlut93,        8)
    REGDEF_BIT(msnr_vlut94,        8)
    REGDEF_BIT(msnr_vlut95,        8)
REGDEF_END(V_NR_REGISTER23)


/*
    msnr_vlut96:    [0x0, 0xff],			bits : 7_0
    msnr_vlut97:    [0x0, 0xff],			bits : 15_8
    msnr_vlut98:    [0x0, 0xff],			bits : 23_16
    msnr_vlut99:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER24_OFS 0x0240
REGDEF_BEGIN(V_NR_REGISTER24)
    REGDEF_BIT(msnr_vlut96,        8)
    REGDEF_BIT(msnr_vlut97,        8)
    REGDEF_BIT(msnr_vlut98,        8)
    REGDEF_BIT(msnr_vlut99,        8)
REGDEF_END(V_NR_REGISTER24)


/*
    msnr_vlut100:    [0x0, 0xff],			bits : 7_0
    msnr_vlut101:    [0x0, 0xff],			bits : 15_8
    msnr_vlut102:    [0x0, 0xff],			bits : 23_16
    msnr_vlut103:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER25_OFS 0x0244
REGDEF_BEGIN(V_NR_REGISTER25)
    REGDEF_BIT(msnr_vlut100,        8)
    REGDEF_BIT(msnr_vlut101,        8)
    REGDEF_BIT(msnr_vlut102,        8)
    REGDEF_BIT(msnr_vlut103,        8)
REGDEF_END(V_NR_REGISTER25)


/*
    msnr_vlut104:    [0x0, 0xff],			bits : 7_0
    msnr_vlut105:    [0x0, 0xff],			bits : 15_8
    msnr_vlut106:    [0x0, 0xff],			bits : 23_16
    msnr_vlut107:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER26_OFS 0x0248
REGDEF_BEGIN(V_NR_REGISTER26)
    REGDEF_BIT(msnr_vlut104,        8)
    REGDEF_BIT(msnr_vlut105,        8)
    REGDEF_BIT(msnr_vlut106,        8)
    REGDEF_BIT(msnr_vlut107,        8)
REGDEF_END(V_NR_REGISTER26)


/*
    msnr_vlut108:    [0x0, 0xff],			bits : 7_0
    msnr_vlut109:    [0x0, 0xff],			bits : 15_8
    msnr_vlut110:    [0x0, 0xff],			bits : 23_16
    msnr_vlut111:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER27_OFS 0x024c
REGDEF_BEGIN(V_NR_REGISTER27)
    REGDEF_BIT(msnr_vlut108,        8)
    REGDEF_BIT(msnr_vlut109,        8)
    REGDEF_BIT(msnr_vlut110,        8)
    REGDEF_BIT(msnr_vlut111,        8)
REGDEF_END(V_NR_REGISTER27)


/*
    msnr_vlut112:    [0x0, 0xff],			bits : 7_0
    msnr_vlut113:    [0x0, 0xff],			bits : 15_8
    msnr_vlut114:    [0x0, 0xff],			bits : 23_16
    msnr_vlut115:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER28_OFS 0x0250
REGDEF_BEGIN(V_NR_REGISTER28)
    REGDEF_BIT(msnr_vlut112,        8)
    REGDEF_BIT(msnr_vlut113,        8)
    REGDEF_BIT(msnr_vlut114,        8)
    REGDEF_BIT(msnr_vlut115,        8)
REGDEF_END(V_NR_REGISTER28)


/*
    msnr_vlut116:    [0x0, 0xff],			bits : 7_0
    msnr_vlut117:    [0x0, 0xff],			bits : 15_8
    msnr_vlut118:    [0x0, 0xff],			bits : 23_16
    msnr_vlut119:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER29_OFS 0x0254
REGDEF_BEGIN(V_NR_REGISTER29)
    REGDEF_BIT(msnr_vlut116,        8)
    REGDEF_BIT(msnr_vlut117,        8)
    REGDEF_BIT(msnr_vlut118,        8)
    REGDEF_BIT(msnr_vlut119,        8)
REGDEF_END(V_NR_REGISTER29)


/*
    msnr_vlut120:    [0x0, 0xff],			bits : 7_0
    msnr_vlut121:    [0x0, 0xff],			bits : 15_8
    msnr_vlut122:    [0x0, 0xff],			bits : 23_16
    msnr_vlut123:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER30_OFS 0x0258
REGDEF_BEGIN(V_NR_REGISTER30)
    REGDEF_BIT(msnr_vlut120,        8)
    REGDEF_BIT(msnr_vlut121,        8)
    REGDEF_BIT(msnr_vlut122,        8)
    REGDEF_BIT(msnr_vlut123,        8)
REGDEF_END(V_NR_REGISTER30)


/*
    msnr_vlut124:    [0x0, 0xff],			bits : 7_0
    msnr_vlut125:    [0x0, 0xff],			bits : 15_8
    msnr_vlut126:    [0x0, 0xff],			bits : 23_16
    msnr_vlut127:    [0x0, 0xff],			bits : 31_24
*/
#define V_NR_REGISTER31_OFS 0x025c
REGDEF_BEGIN(V_NR_REGISTER31)
    REGDEF_BIT(msnr_vlut124,        8)
    REGDEF_BIT(msnr_vlut125,        8)
    REGDEF_BIT(msnr_vlut126,        8)
    REGDEF_BIT(msnr_vlut127,        8)
REGDEF_END(V_NR_REGISTER31)


/*
    ycmod_ylut00:    [0x0, 0x1f],			bits : 4_0
    ycmod_ylut01:    [0x0, 0x1f],			bits : 12_8
    ycmod_ylut02:    [0x0, 0x1f],			bits : 20_16
    ycmod_ylut03:    [0x0, 0x1f],			bits : 28_24
*/
#define YCMOD_REGISTER0_OFS 0x0260
REGDEF_BEGIN(YCMOD_REGISTER0)
    REGDEF_BIT(ycmod_ylut00,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut01,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut02,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut03,        5)
REGDEF_END(YCMOD_REGISTER0)


/*
    ycmod_ylut04:    [0x0, 0x1f],			bits : 4_0
    ycmod_ylut05:    [0x0, 0x1f],			bits : 12_8
    ycmod_ylut06:    [0x0, 0x1f],			bits : 20_16
    ycmod_ylut07:    [0x0, 0x1f],			bits : 28_24
*/
#define YCMOD_REGISTER1_OFS 0x0264
REGDEF_BEGIN(YCMOD_REGISTER1)
    REGDEF_BIT(ycmod_ylut04,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut05,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut06,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut07,        5)
REGDEF_END(YCMOD_REGISTER1)


/*
    ycmod_ylut08:    [0x0, 0x1f],			bits : 4_0
    ycmod_ylut09:    [0x0, 0x1f],			bits : 12_8
    ycmod_ylut10:    [0x0, 0x1f],			bits : 20_16
    ycmod_ylut11:    [0x0, 0x1f],			bits : 28_24
*/
#define YCMOD_REGISTER2_OFS 0x0268
REGDEF_BEGIN(YCMOD_REGISTER2)
    REGDEF_BIT(ycmod_ylut08,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut09,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut10,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut11,        5)
REGDEF_END(YCMOD_REGISTER2)


/*
    ycmod_ylut12:    [0x0, 0x1f],			bits : 4_0
    ycmod_ylut13:    [0x0, 0x1f],			bits : 12_8
    ycmod_ylut14:    [0x0, 0x1f],			bits : 20_16
    ycmod_ylut15:    [0x0, 0x1f],			bits : 28_24
*/
#define YCMOD_REGISTER3_OFS 0x026c
REGDEF_BEGIN(YCMOD_REGISTER3)
    REGDEF_BIT(ycmod_ylut12,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut13,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut14,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_ylut15,        5)
REGDEF_END(YCMOD_REGISTER3)


/*
    ycmod_clut00:    [0x0, 0x1f],			bits : 4_0
    ycmod_clut01:    [0x0, 0x1f],			bits : 12_8
    ycmod_clut02:    [0x0, 0x1f],			bits : 20_16
    ycmod_clut03:    [0x0, 0x1f],			bits : 28_24
*/
#define YCMOD_REGISTER4_OFS 0x0270
REGDEF_BEGIN(YCMOD_REGISTER4)
    REGDEF_BIT(ycmod_clut00,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut01,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut02,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut03,        5)
REGDEF_END(YCMOD_REGISTER4)


/*
    ycmod_clut04:    [0x0, 0x1f],			bits : 4_0
    ycmod_clut05:    [0x0, 0x1f],			bits : 12_8
    ycmod_clut06:    [0x0, 0x1f],			bits : 20_16
    ycmod_clut07:    [0x0, 0x1f],			bits : 28_24
*/
#define YCMOD_REGISTER5_OFS 0x0274
REGDEF_BEGIN(YCMOD_REGISTER5)
    REGDEF_BIT(ycmod_clut04,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut05,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut06,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut07,        5)
REGDEF_END(YCMOD_REGISTER5)


/*
    ycmod_clut08:    [0x0, 0x1f],			bits : 4_0
    ycmod_clut09:    [0x0, 0x1f],			bits : 12_8
    ycmod_clut10:    [0x0, 0x1f],			bits : 20_16
    ycmod_clut11:    [0x0, 0x1f],			bits : 28_24
*/
#define YCMOD_REGISTER6_OFS 0x0278
REGDEF_BEGIN(YCMOD_REGISTER6)
    REGDEF_BIT(ycmod_clut08,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut09,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut10,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut11,        5)
REGDEF_END(YCMOD_REGISTER6)


/*
    ycmod_clut12:    [0x0, 0x1f],			bits : 4_0
    ycmod_clut13:    [0x0, 0x1f],			bits : 12_8
    ycmod_clut14:    [0x0, 0x1f],			bits : 20_16
    ycmod_clut15:    [0x0, 0x1f],			bits : 28_24
*/
#define YCMOD_REGISTER7_OFS 0x027c
REGDEF_BEGIN(YCMOD_REGISTER7)
    REGDEF_BIT(ycmod_clut12,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut13,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut14,        5)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(ycmod_clut15,        5)
REGDEF_END(YCMOD_REGISTER7)


/*
    ycmod_cbofs :    [0x0, 0xff],			bits : 7_0
    ycmod_crofs :    [0x0, 0xff],			bits : 15_8
    ycmod_stepy :    [0x0, 0x3],			bits : 17_16
    ycmod_stepc :    [0x0, 0x3],			bits : 19_18
    ycmod_sel   :    [0x0, 0x1],			bits : 20
    ycmod_lutsel:    [0x0, 0x1],			bits : 21
*/
#define YCMOD_REGISTER8_OFS 0x0280
REGDEF_BEGIN(YCMOD_REGISTER8)
    REGDEF_BIT(ycmod_cbofs ,        8)
    REGDEF_BIT(ycmod_crofs ,        8)
    REGDEF_BIT(ycmod_stepy ,        2)
    REGDEF_BIT(ycmod_stepc ,        2)
    REGDEF_BIT(ycmod_sel   ,        1)
    REGDEF_BIT(ycmod_lutsel,        1)
REGDEF_END(YCMOD_REGISTER8)


/*
    write_ch_outstanding_num:    [0x0, 0xff],			bits : 7_0
    read_ch_outstanding_num :    [0x0, 0xff],			bits : 15_8
*/
#define AXI_OUTSTRANDING_REGISTER_OFS 0x0284
REGDEF_BEGIN(AXI_OUTSTRANDING_REGISTER)
    REGDEF_BIT(write_ch_outstanding_num,        8)
    REGDEF_BIT(read_ch_outstanding_num ,        8)
REGDEF_END(AXI_OUTSTRANDING_REGISTER)


/*
    in0_ch_en              :    [0x0, 0x1],			bits : 0
    in1_ch_en              :    [0x0, 0x1],			bits : 1
    in2_ch_en              :    [0x0, 0x1],			bits : 2
    in3_ch_en              :    [0x0, 0x1],			bits : 3
    in_weight_ch_en        :    [0x0, 0x1],			bits : 4
    out1_ch_en             :    [0x0, 0x1],			bits : 5
    out2_ch_en             :    [0x0, 0x1],			bits : 6
    out3_ch_en             :    [0x0, 0x1],			bits : 7
    out_weight_ch_en       :    [0x0, 0x1],			bits : 8
    llc_in_ch_en           :    [0x0, 0x1],			bits : 9
    in5_ch_en              :    [0x0, 0x1],			bits : 10
    Reserverd              :    [0x0, 0x3f],			bits : 15_11
    in0_lock_disable       :    [0x0, 0x1],			bits : 16
    in1_lock_disable       :    [0x0, 0x1],			bits : 17
    in2_lock_disable       :    [0x0, 0x1],			bits : 18
    in3_lock_disable       :    [0x0, 0x1],			bits : 19
    in_weight_lock_disable :    [0x0, 0x1],			bits : 20
    out1_lock_disable      :    [0x0, 0x1],			bits : 21
    out2_lock_disable      :    [0x0, 0x1],			bits : 22
    out3_lock_disable      :    [0x0, 0x1],			bits : 23
    out_weight_lock_disable:    [0x0, 0x1],			bits : 24
    llc_lock_disable       :    [0x0, 0x1],			bits : 25
    in5_lock_disable	   :	  [0x0, 0x1], 		      bits : 26
    
*/
#define AXI_REGISTER_0_OFS 0x0288
REGDEF_BEGIN(AXI_REGISTER_0)
    REGDEF_BIT(in0_ch_en              ,        1)
    REGDEF_BIT(in1_ch_en              ,        1)
    REGDEF_BIT(in2_ch_en              ,        1)
    REGDEF_BIT(in3_ch_en              ,        1)
    REGDEF_BIT(in_weight_ch_en        ,        1)
    REGDEF_BIT(out1_ch_en             ,        1)
    REGDEF_BIT(out2_ch_en             ,        1)
    REGDEF_BIT(out3_ch_en             ,        1)
    REGDEF_BIT(out_weight_ch_en       ,        1)
    REGDEF_BIT(llc_in_ch_en           ,        1)
    REGDEF_BIT(in_patch_pixel_mask_ch_en ,        1)
    REGDEF_BIT(reserverd                 ,        5)
    REGDEF_BIT(in0_lock_disable       ,        1)
    REGDEF_BIT(in1_lock_disable       ,        1)
    REGDEF_BIT(in2_lock_disable       ,        1)
    REGDEF_BIT(in3_lock_disable       ,        1)
    REGDEF_BIT(in_weight_lock_disable ,        1)
    REGDEF_BIT(out1_lock_disable      ,        1)
    REGDEF_BIT(out2_lock_disable      ,        1)
    REGDEF_BIT(out3_lock_disable      ,        1)
    REGDEF_BIT(out_weight_lock_disable,        1)
    REGDEF_BIT(llc_lock_disable       ,        1)
    REGDEF_BIT(in_patch_pixel_mask_lock_disable,        1)    
REGDEF_END(AXI_REGISTER_0)


/*
    axi_bus_disable:    [0x0, 0x1],			bits : 0
    Reserverd      :    [0x0, 0x7fff],			bits : 15_1
    axi_bus_idle   :    [0x0, 0x1],			bits : 16
*/
#define AXI_REGISTER_1_OFS 0x028c
REGDEF_BEGIN(AXI_REGISTER_1)
    REGDEF_BIT(axi_bus_disable,        1)
    REGDEF_BIT(reserverd      ,        15)
    REGDEF_BIT(axi_bus_idle   ,        1)
REGDEF_END(AXI_REGISTER_1)


/*
    axi_bus_status:    [0x0, 0xffffffff],			bits : 31_0
*/
#define AXI_REGISTER_2_OFS 0x0290
REGDEF_BEGIN(AXI_REGISTER_2)
    REGDEF_BIT(axi_bus_status,        32)
REGDEF_END(AXI_REGISTER_2)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER_10_OFS 0x0294
REGDEF_BEGIN(RESERVED_REGISTER_10)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER_10)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER_11_OFS 0x0298
REGDEF_BEGIN(RESERVED_REGISTER_11)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER_11)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER_12_OFS 0x029c
REGDEF_BEGIN(RESERVED_REGISTER_12)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER_12)


/*
    llc_chksum:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LLC_DEBUG_RESISTER_0_OFS 0x02a0
REGDEF_BEGIN(LLC_DEBUG_RESISTER_0)
    REGDEF_BIT(llc_chksum,        32)
REGDEF_END(LLC_DEBUG_RESISTER_0)


/*
    llc_err_cmd_cnt:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LLC_DEBUG_RESISTER_1_OFS 0x02a4
REGDEF_BEGIN(LLC_DEBUG_RESISTER_1)
    REGDEF_BIT(llc_err_cmd_cnt,        32)
REGDEF_END(LLC_DEBUG_RESISTER_1)


/*
    llc_err_cmd_adr:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LLC_DEBUG_RESISTER_2_OFS 0x02a8
REGDEF_BEGIN(LLC_DEBUG_RESISTER_2)
    REGDEF_BIT(llc_err_cmd_adr,        32)
REGDEF_END(LLC_DEBUG_RESISTER_2)


/*
    ll_table_idx0:    [0x0, 0xff],			bits : 7_0
    ll_table_idx1:    [0x0, 0xff],			bits : 15_8
    ll_table_idx2:    [0x0, 0xff],			bits : 23_16
    ll_table_idx3:    [0x0, 0xff],			bits : 31_24
*/
#define LL_FRAME_REGISTER_0_OFS 0x02ac
REGDEF_BEGIN(LL_FRAME_REGISTER_0)
    REGDEF_BIT(ll_table_idx0,        8)
    REGDEF_BIT(ll_table_idx1,        8)
    REGDEF_BIT(ll_table_idx2,        8)
    REGDEF_BIT(ll_table_idx3,        8)
REGDEF_END(LL_FRAME_REGISTER_0)


/*
    ll_table_idx4:    [0x0, 0xff],			bits : 7_0
    ll_table_idx5:    [0x0, 0xff],			bits : 15_8
    ll_table_idx6:    [0x0, 0xff],			bits : 23_16
    ll_table_idx7:    [0x0, 0xff],			bits : 31_24
*/
#define LL_FRAME_REGISTER_1_OFS 0x02b0
REGDEF_BEGIN(LL_FRAME_REGISTER_1)
    REGDEF_BIT(ll_table_idx4,        8)
    REGDEF_BIT(ll_table_idx5,        8)
    REGDEF_BIT(ll_table_idx6,        8)
    REGDEF_BIT(ll_table_idx7,        8)
REGDEF_END(LL_FRAME_REGISTER_1)


/*
    ll_table_idx8 :    [0x0, 0xff],			bits : 7_0
    ll_table_idx9 :    [0x0, 0xff],			bits : 15_8
    ll_table_idx10:    [0x0, 0xff],			bits : 23_16
    ll_table_idx11:    [0x0, 0xff],			bits : 31_24
*/
#define LL_FRAME_REGISTER_2_OFS 0x02b4
REGDEF_BEGIN(LL_FRAME_REGISTER_2)
    REGDEF_BIT(ll_table_idx8 ,        8)
    REGDEF_BIT(ll_table_idx9 ,        8)
    REGDEF_BIT(ll_table_idx10,        8)
    REGDEF_BIT(ll_table_idx11,        8)
REGDEF_END(LL_FRAME_REGISTER_2)


/*
    ll_table_idx12:    [0x0, 0xff],			bits : 7_0
    ll_table_idx13:    [0x0, 0xff],			bits : 15_8
    ll_table_idx14:    [0x0, 0xff],			bits : 23_16
    ll_table_idx15:    [0x0, 0xff],			bits : 31_24
*/
#define LL_FRAME_REGISTER_3_OFS 0x02b8
REGDEF_BEGIN(LL_FRAME_REGISTER_3)
    REGDEF_BIT(ll_table_idx12,        8)
    REGDEF_BIT(ll_table_idx13,        8)
    REGDEF_BIT(ll_table_idx14,        8)
    REGDEF_BIT(ll_table_idx15,        8)
REGDEF_END(LL_FRAME_REGISTER_3)



/*
    dre_in_ll_addr_msb          :    [0x0, 0xf],			bits : 3_0
    reserved                    :    [0x0, 0x0fffffff],		bits : 31_4
*/
#define LINKED_LIST_COMMAND_ADDRESS_MSB_REGISTER_OFS 0x02F4
REGDEF_BEGIN(LINKED_LIST_COMMAND_ADDRESS_MSB_REGISTER)
    REGDEF_BIT(dre_in_ll_addr_msb,        4)
    REGDEF_BIT(reserved      ,        28)
REGDEF_END(LINKED_LIST_COMMAND_ADDRESS_MSB_REGISTER)




//for register buffer
typedef struct
{

  union
  {
    struct
    {
      unsigned dre_rst          : 1;		// bits : 0
      unsigned dre_start        : 1;		// bits : 1
      unsigned                  : 26;
      unsigned ll_fire          : 1;		// bits : 28
    } bit;
    UINT32 word;
  } reg_0; // 0x0000

  union
  {
    struct
    {
      unsigned dre_process_sel               : 1;		// bits : 0
      unsigned dre_dcm_in_img_num_sel        : 1;		// bits : 1
      unsigned dre_dcm_work_mode_sel         : 3;		// bits : 4_2
      unsigned                               : 1;
      unsigned dre_dcm_filt_en               : 1;		// bits : 6
      unsigned dre_dcm_nr_lut_y_en           : 1;		// bits : 7
      unsigned dre_dcm_nr_lut_u_en           : 1;		// bits : 8
      unsigned dre_dcm_nr_lut_v_en           : 1;		// bits : 9
      unsigned dre_rcs_work_mode_sel         : 3;		// bits : 12_10
      unsigned ycmod_en                      : 1;		// bits : 13
      unsigned weight_map_src_sel            : 1;		// bits : 14
      unsigned hard_weight_en                : 1;		// bits : 15
	  unsigned src_image_patching_mode       : 1;		// bits : 16
    } bit;
    UINT32 word;
  } reg_1; // 0x0004

  union
  {
    struct
    {
      unsigned dre_inte_end                 : 1;		// bits : 0
      unsigned dre_inte_timeout             : 1;		// bits : 1
      unsigned dre_inte_llend               : 1;		// bits : 2
      unsigned dre_inte_ll_cmd_error        : 1;		// bits : 3
      unsigned dre_inte_lljobend            : 1;		// bits : 4
    } bit;
    UINT32 word;
  } reg_2; // 0x0008

  union
  {
    struct
    {
      unsigned dre_int_end                 : 1;		// bits : 0
      unsigned dre_int_timeout             : 1;		// bits : 1
      unsigned dre_int_llend               : 1;		// bits : 2
      unsigned dre_int_ll_cmd_error        : 1;		// bits : 3
      unsigned dre_int_lljobend            : 1;		// bits : 4
    } bit;
    UINT32 word;
  } reg_3; // 0x000c

  union
  {
    struct
    {
      unsigned dre_time_limit        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_4; // 0x0010

  union
  {
    struct
    {
      unsigned dre_proc_time        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_5; // 0x0014

  union
  {
    struct
    {
      unsigned                        : 2;
      unsigned dre_in_ch0_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_6; // 0x0018

  union
  {
    struct
    {
      unsigned                        : 2;
      unsigned dre_in_ch1_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_7; // 0x001c

  union
  {
    struct
    {
      unsigned                        : 2;
      unsigned dre_in_ch2_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_8; // 0x0020

  union
  {
    struct
    {
      unsigned                        : 2;
      unsigned dre_in_ch3_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_9; // 0x0024

  union
  {
    struct
    {
      unsigned                               : 2;
      unsigned dre_in_weight_map_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_10; // 0x0028

  union
  {
    struct
    {
      unsigned                         : 2;
      unsigned dre_out_ch0_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_11; // 0x002c

  union
  {
    struct
    {
      unsigned                         : 2;
      unsigned dre_out_ch1_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_12; // 0x0030

  union
  {
    struct
    {
      unsigned                         : 2;
      unsigned dre_out_ch2_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_13; // 0x0034

  union
  {
    struct
    {
      unsigned                                : 2;
      unsigned dre_out_weight_map_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_14; // 0x0038

  union
  {
    struct
    {
      unsigned dre_l0_width         : 16;		// bits : 15_0
      unsigned dre_l0_height        : 16;		// bits : 31_16
    } bit;
    UINT32 word;
  } reg_15; // 0x003c

  union
  {
    struct
    {
      unsigned dre_l1_width         : 16;		// bits : 15_0
      unsigned dre_l1_height        : 16;		// bits : 31_16
    } bit;
    UINT32 word;
  } reg_16; // 0x0040

  union
  {
    struct
    {
      unsigned                         : 2;
      unsigned dre_in_ch0_lofst        : 18;		// bits : 19_2
    } bit;
    UINT32 word;
  } reg_17; // 0x0044

  union
  {
    struct
    {
      unsigned                         : 2;
      unsigned dre_in_ch1_lofst        : 18;		// bits : 19_2
    } bit;
    UINT32 word;
  } reg_18; // 0x0048

  union
  {
    struct
    {
      unsigned                         : 2;
      unsigned dre_in_ch2_lofst        : 18;		// bits : 19_2
    } bit;
    UINT32 word;
  } reg_19; // 0x004c

  union
  {
    struct
    {
      unsigned                         : 2;
      unsigned dre_in_ch3_lofst        : 18;		// bits : 19_2
    } bit;
    UINT32 word;
  } reg_20; // 0x0050

  union
  {
    struct
    {
      unsigned                                : 2;
      unsigned dre_in_weight_map_lofst        : 18;		// bits : 19_2
    } bit;
    UINT32 word;
  } reg_21; // 0x0054

  union
  {
    struct
    {
      unsigned                          : 2;
      unsigned dre_out_ch0_lofst        : 18;		// bits : 19_2
    } bit;
    UINT32 word;
  } reg_22; // 0x0058

  union
  {
    struct
    {
      unsigned                          : 2;
      unsigned dre_out_ch1_lofst        : 18;		// bits : 19_2
    } bit;
    UINT32 word;
  } reg_23; // 0x005c

  union
  {
    struct
    {
      unsigned                          : 2;
      unsigned dre_out_ch2_lofst        : 18;		// bits : 19_2
    } bit;
    UINT32 word;
  } reg_24; // 0x0060

  union
  {
    struct
    {
      unsigned inch0_burst_len_sel                 : 2;		// bits : 1_0
      unsigned inch1_burst_len_sel                 : 2;		// bits : 3_2
      unsigned inch2_burst_len_sel                 : 2;		// bits : 5_4
      unsigned inch3_burst_len_sel                 : 2;		// bits : 7_6
      unsigned in_weight_map_burst_len_sel         : 2;		// bits : 9_8
      unsigned outch0_burst_len_sel                : 2;		// bits : 11_10
      unsigned outch1_burst_len_sel                : 2;		// bits : 13_12
      unsigned outch2_burst_len_sel                : 2;		// bits : 15_14
      unsigned out_weight_map_burst_len_sel        : 2;		// bits : 17_16
	  unsigned disable_hw_input_data_align         : 1;		// bits : 19
	  unsigned disable_hw_output_data_align        : 1;		// bits : 20
	  unsigned in_patch_pixel_mask_burst_len_sel   : 2;		// bits : 21_20
      unsigned                                     : 6;     // bits : 27_22
      unsigned dre_dbg_port_sel                    : 3;		// bits : 30_28
      unsigned check_sum_enable                    : 1;		// bits : 31
    } bit;
    UINT32 word;
  } reg_25; // 0x0064

  union
  {
    struct
    {
      unsigned dre_h_sdn_factor        : 18;		// bits : 17_0
    } bit;
    UINT32 word;
  } reg_26; // 0x0068

  union
  {
    struct
    {
      unsigned dre_v_sdn_factor        : 18;		// bits : 17_0
    } bit;
    UINT32 word;
  } reg_27; // 0x006c

  union
  {
    struct
    {
      unsigned dre_h_sup_factor        : 16;		// bits : 15_0
      unsigned dre_v_sup_factor        : 16;		// bits : 31_16
    } bit;
    UINT32 word;
  } reg_28; // 0x0070

  union
  {
    struct
    {
      unsigned                                 : 2;
      unsigned dre_out_weight_map_lofst        : 18;		// bits : 19_2
    } bit;
    UINT32 word;
  } reg_29; // 0x0074

  union
  {
    struct
    {
      unsigned dre_h_sdn_offset        : 16;		// bits : 15_0
      unsigned dre_v_sdn_offset        : 16;		// bits : 31_16
    } bit;
    UINT32 word;
  } reg_30; // 0x0078

  union
  {
    struct
    {
      unsigned ll_terminate        : 1;		// bits : 0
      unsigned reserved            : 31;		// bits : 31_1
    } bit;
    UINT32 word;
  } reg_31; // 0x007c

  union
  {
    struct
    {
      unsigned reserved              : 2;		// bits : 1_0
      unsigned dre_in_ll_addr        : 30;		// bits : 31_2
    } bit;
    UINT32 word;
  } reg_32; // 0x0080

  union
  {
    struct
    {
      unsigned fusion_wt_table_val00        : 8;		// bits : 7_0
      unsigned fusion_wt_table_val01        : 8;		// bits : 15_8
      unsigned fusion_wt_table_val02        : 8;		// bits : 23_16
      unsigned fusion_wt_table_val03        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_33; // 0x0084

  union
  {
    struct
    {
      unsigned fusion_wt_table_val04        : 8;		// bits : 7_0
      unsigned fusion_wt_table_val05        : 8;		// bits : 15_8
      unsigned fusion_wt_table_val06        : 8;		// bits : 23_16
      unsigned fusion_wt_table_val07        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_34; // 0x0088

  union
  {
    struct
    {
      unsigned fusion_wt_table_val08        : 8;		// bits : 7_0
      unsigned fusion_wt_table_val09        : 8;		// bits : 15_8
      unsigned fusion_wt_table_val10        : 8;		// bits : 23_16
      unsigned fusion_wt_table_val11        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_35; // 0x008c

  union
  {
    struct
    {
      unsigned fusion_wt_table_val12        : 8;		// bits : 7_0
      unsigned fusion_wt_table_val13        : 8;		// bits : 15_8
      unsigned fusion_wt_table_val14        : 8;		// bits : 23_16
      unsigned fusion_wt_table_val15        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_36; // 0x0090

  union
  {
    struct
    {
      unsigned fusion_wt_table_val16        : 8;		// bits : 7_0
    } bit;
    UINT32 word;
  } reg_37; // 0x0094

  union
  {
    struct
    {
      unsigned dma_disable        : 1;		// bits : 0
      unsigned                    : 14;
      unsigned dre_idle           : 1;		// bits : 15
    } bit;
    UINT32 word;
  } reg_38; // 0x0098

  union
  {
    struct
    {
      unsigned img0_spa_coeff0        : 8;		// bits : 7_0
      unsigned img0_spa_coeff1        : 8;		// bits : 15_8
      unsigned img0_spa_coeff2        : 8;		// bits : 23_16
    } bit;
    UINT32 word;
  } reg_39; // 0x009c

  union
  {
    struct
    {
		unsigned						            : 2;
		unsigned dre_in_patch_pixel_mask_addr		: 30;	  // bits : 31_2
    } bit;
    UINT32 word;
  } reg_40; // 0x00a0

  union
  {
    struct
    {
		unsigned						             : 2;
		unsigned dre_in_patch_pixel_mask_lofst		 : 18;		  // bits : 19_2
    } bit;
    UINT32 word;
  } reg_41; // 0x00a4

  union
  {
    struct
    {
		unsigned img_patch_user_color_y		: 8;	  // bits : 7_0
		unsigned img_patch_user_color_u		: 8;	  // bits : 15_8
		unsigned img_patch_user_color_v  	: 8;	  // bits : 23_16
    } bit;
    UINT32 word;
  } reg_42; // 0x00a8

  union
  {
    struct
    {
      unsigned img1_spa_coeff0        : 8;		// bits : 7_0
      unsigned img1_spa_coeff1        : 8;		// bits : 15_8
      unsigned img1_spa_coeff2        : 8;		// bits : 23_16
    } bit;
    UINT32 word;
  } reg_43; // 0x00ac

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_44; // 0x00b0

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_45; // 0x00b4

  union
  {
    struct
    {
      unsigned s_weight0        : 8;		// bits : 7_0
      unsigned s_weight1        : 8;		// bits : 15_8
      unsigned s_weight2        : 8;		// bits : 23_16
      unsigned s_weight3        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_46; // 0x00b8

  union
  {
    struct
    {
      unsigned s_weight4        : 8;		// bits : 7_0
      unsigned s_weight5        : 8;		// bits : 15_8
    } bit;
    UINT32 word;
  } reg_47; // 0x00bc

  union
  {
    struct
    {
      unsigned rth_y_lut00        : 8;		// bits : 7_0
      unsigned rth_y_lut01        : 8;		// bits : 15_8
      unsigned rth_y_lut02        : 8;		// bits : 23_16
      unsigned rth_y_lut03        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_48; // 0x00c0

  union
  {
    struct
    {
      unsigned rth_y_lut04        : 8;		// bits : 7_0
      unsigned rth_y_lut05        : 8;		// bits : 15_8
      unsigned rth_y_lut06        : 8;		// bits : 23_16
      unsigned rth_y_lut07        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_49; // 0x00c4

  union
  {
    struct
    {
      unsigned rth_u_lut00        : 8;		// bits : 7_0
      unsigned rth_u_lut01        : 8;		// bits : 15_8
      unsigned rth_u_lut02        : 8;		// bits : 23_16
      unsigned rth_u_lut03        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_50; // 0x00c8

  union
  {
    struct
    {
      unsigned rth_u_lut04        : 8;		// bits : 7_0
      unsigned rth_u_lut05        : 8;		// bits : 15_8
      unsigned rth_u_lut06        : 8;		// bits : 23_16
      unsigned rth_u_lut07        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_51; // 0x00cc

  union
  {
    struct
    {
      unsigned rth_v_lut00        : 8;		// bits : 7_0
      unsigned rth_v_lut01        : 8;		// bits : 15_8
      unsigned rth_v_lut02        : 8;		// bits : 23_16
      unsigned rth_v_lut03        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_52; // 0x00d0

  union
  {
    struct
    {
      unsigned rth_v_lut04        : 8;		// bits : 7_0
      unsigned rth_v_lut05        : 8;		// bits : 15_8
      unsigned rth_v_lut06        : 8;		// bits : 23_16
      unsigned rth_v_lut07        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_53; // 0x00d4

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_54; // 0x00d8

  union
  {
    struct
    {
      unsigned joint_yy        : 1;		// bits : 0
      unsigned joint_yu        : 1;		// bits : 1
      unsigned joint_yv        : 1;		// bits : 2
      unsigned joint_uy        : 1;		// bits : 3
      unsigned joint_uu        : 1;		// bits : 4
      unsigned joint_uv        : 1;		// bits : 5
      unsigned joint_vy        : 1;		// bits : 6
      unsigned joint_vu        : 1;		// bits : 7
      unsigned joint_vv        : 1;		// bits : 8
      unsigned outly_en        : 1;		// bits : 9
      unsigned outlu_en        : 1;		// bits : 10
      unsigned outlv_en        : 1;		// bits : 11
    } bit;
    UINT32 word;
  } reg_55; // 0x00dc

  union
  {
    struct
    {
      unsigned msnr_ylut0        : 8;		// bits : 7_0
      unsigned msnr_ylut1        : 8;		// bits : 15_8
      unsigned msnr_ylut2        : 8;		// bits : 23_16
      unsigned msnr_ylut3        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_56; // 0x00e0

  union
  {
    struct
    {
      unsigned msnr_ylut4        : 8;		// bits : 7_0
      unsigned msnr_ylut5        : 8;		// bits : 15_8
      unsigned msnr_ylut6        : 8;		// bits : 23_16
      unsigned msnr_ylut7        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_57; // 0x00e4

  union
  {
    struct
    {
      unsigned msnr_ylut8         : 8;		// bits : 7_0
      unsigned msnr_ylut9         : 8;		// bits : 15_8
      unsigned msnr_ylut10        : 8;		// bits : 23_16
      unsigned msnr_ylut11        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_58; // 0x00e8

  union
  {
    struct
    {
      unsigned msnr_ylut12        : 8;		// bits : 7_0
      unsigned msnr_ylut13        : 8;		// bits : 15_8
      unsigned msnr_ylut14        : 8;		// bits : 23_16
      unsigned msnr_ylut15        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_59; // 0x00ec

  union
  {
    struct
    {
      unsigned msnr_ylut16        : 8;		// bits : 7_0
      unsigned msnr_ylut17        : 8;		// bits : 15_8
      unsigned msnr_ylut18        : 8;		// bits : 23_16
      unsigned msnr_ylut19        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_60; // 0x00f0

  union
  {
    struct
    {
      unsigned msnr_ylut20        : 8;		// bits : 7_0
      unsigned msnr_ylut21        : 8;		// bits : 15_8
      unsigned msnr_ylut22        : 8;		// bits : 23_16
      unsigned msnr_ylut23        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_61; // 0x00f4

  union
  {
    struct
    {
      unsigned msnr_ylut24        : 8;		// bits : 7_0
      unsigned msnr_ylut25        : 8;		// bits : 15_8
      unsigned msnr_ylut26        : 8;		// bits : 23_16
      unsigned msnr_ylut27        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_62; // 0x00f8

  union
  {
    struct
    {
      unsigned msnr_ylut28        : 8;		// bits : 7_0
      unsigned msnr_ylut29        : 8;		// bits : 15_8
      unsigned msnr_ylut30        : 8;		// bits : 23_16
      unsigned msnr_ylut31        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_63; // 0x00fc

  union
  {
    struct
    {
      unsigned msnr_ylut32        : 8;		// bits : 7_0
      unsigned msnr_ylut33        : 8;		// bits : 15_8
      unsigned msnr_ylut34        : 8;		// bits : 23_16
      unsigned msnr_ylut35        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_64; // 0x0100

  union
  {
    struct
    {
      unsigned msnr_ylut36        : 8;		// bits : 7_0
      unsigned msnr_ylut37        : 8;		// bits : 15_8
      unsigned msnr_ylut38        : 8;		// bits : 23_16
      unsigned msnr_ylut39        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_65; // 0x0104

  union
  {
    struct
    {
      unsigned msnr_ylut40        : 8;		// bits : 7_0
      unsigned msnr_ylut41        : 8;		// bits : 15_8
      unsigned msnr_ylut42        : 8;		// bits : 23_16
      unsigned msnr_ylut43        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_66; // 0x0108

  union
  {
    struct
    {
      unsigned msnr_ylut44        : 8;		// bits : 7_0
      unsigned msnr_ylut45        : 8;		// bits : 15_8
      unsigned msnr_ylut46        : 8;		// bits : 23_16
      unsigned msnr_ylut47        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_67; // 0x010c

  union
  {
    struct
    {
      unsigned msnr_ylut48        : 8;		// bits : 7_0
      unsigned msnr_ylut49        : 8;		// bits : 15_8
      unsigned msnr_ylut50        : 8;		// bits : 23_16
      unsigned msnr_ylut51        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_68; // 0x0110

  union
  {
    struct
    {
      unsigned msnr_ylut52        : 8;		// bits : 7_0
      unsigned msnr_ylut53        : 8;		// bits : 15_8
      unsigned msnr_ylut54        : 8;		// bits : 23_16
      unsigned msnr_ylut55        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_69; // 0x0114

  union
  {
    struct
    {
      unsigned msnr_ylut56        : 8;		// bits : 7_0
      unsigned msnr_ylut57        : 8;		// bits : 15_8
      unsigned msnr_ylut58        : 8;		// bits : 23_16
      unsigned msnr_ylut59        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_70; // 0x0118

  union
  {
    struct
    {
      unsigned msnr_ylut60        : 8;		// bits : 7_0
      unsigned msnr_ylut61        : 8;		// bits : 15_8
      unsigned msnr_ylut62        : 8;		// bits : 23_16
      unsigned msnr_ylut63        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_71; // 0x011c

  union
  {
    struct
    {
      unsigned msnr_ylut64        : 8;		// bits : 7_0
      unsigned msnr_ylut65        : 8;		// bits : 15_8
      unsigned msnr_ylut66        : 8;		// bits : 23_16
      unsigned msnr_ylut67        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_72; // 0x0120

  union
  {
    struct
    {
      unsigned msnr_ylut68        : 8;		// bits : 7_0
      unsigned msnr_ylut69        : 8;		// bits : 15_8
      unsigned msnr_ylut70        : 8;		// bits : 23_16
      unsigned msnr_ylut71        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_73; // 0x0124

  union
  {
    struct
    {
      unsigned msnr_ylut72        : 8;		// bits : 7_0
      unsigned msnr_ylut73        : 8;		// bits : 15_8
      unsigned msnr_ylut74        : 8;		// bits : 23_16
      unsigned msnr_ylut75        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_74; // 0x0128

  union
  {
    struct
    {
      unsigned msnr_ylut76        : 8;		// bits : 7_0
      unsigned msnr_ylut77        : 8;		// bits : 15_8
      unsigned msnr_ylut78        : 8;		// bits : 23_16
      unsigned msnr_ylut79        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_75; // 0x012c

  union
  {
    struct
    {
      unsigned msnr_ylut80        : 8;		// bits : 7_0
      unsigned msnr_ylut81        : 8;		// bits : 15_8
      unsigned msnr_ylut82        : 8;		// bits : 23_16
      unsigned msnr_ylut83        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_76; // 0x0130

  union
  {
    struct
    {
      unsigned msnr_ylut84        : 8;		// bits : 7_0
      unsigned msnr_ylut85        : 8;		// bits : 15_8
      unsigned msnr_ylut86        : 8;		// bits : 23_16
      unsigned msnr_ylut87        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_77; // 0x0134

  union
  {
    struct
    {
      unsigned msnr_ylut88        : 8;		// bits : 7_0
      unsigned msnr_ylut89        : 8;		// bits : 15_8
      unsigned msnr_ylut90        : 8;		// bits : 23_16
      unsigned msnr_ylut91        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_78; // 0x0138

  union
  {
    struct
    {
      unsigned msnr_ylut92        : 8;		// bits : 7_0
      unsigned msnr_ylut93        : 8;		// bits : 15_8
      unsigned msnr_ylut94        : 8;		// bits : 23_16
      unsigned msnr_ylut95        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_79; // 0x013c

  union
  {
    struct
    {
      unsigned msnr_ylut96        : 8;		// bits : 7_0
      unsigned msnr_ylut97        : 8;		// bits : 15_8
      unsigned msnr_ylut98        : 8;		// bits : 23_16
      unsigned msnr_ylut99        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_80; // 0x0140

  union
  {
    struct
    {
      unsigned msnr_ylut100        : 8;		// bits : 7_0
      unsigned msnr_ylut101        : 8;		// bits : 15_8
      unsigned msnr_ylut102        : 8;		// bits : 23_16
      unsigned msnr_ylut103        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_81; // 0x0144

  union
  {
    struct
    {
      unsigned msnr_ylut104        : 8;		// bits : 7_0
      unsigned msnr_ylut105        : 8;		// bits : 15_8
      unsigned msnr_ylut106        : 8;		// bits : 23_16
      unsigned msnr_ylut107        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_82; // 0x0148

  union
  {
    struct
    {
      unsigned msnr_ylut108        : 8;		// bits : 7_0
      unsigned msnr_ylut109        : 8;		// bits : 15_8
      unsigned msnr_ylut110        : 8;		// bits : 23_16
      unsigned msnr_ylut111        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_83; // 0x014c

  union
  {
    struct
    {
      unsigned msnr_ylut112        : 8;		// bits : 7_0
      unsigned msnr_ylut113        : 8;		// bits : 15_8
      unsigned msnr_ylut114        : 8;		// bits : 23_16
      unsigned msnr_ylut115        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_84; // 0x0150

  union
  {
    struct
    {
      unsigned msnr_ylut116        : 8;		// bits : 7_0
      unsigned msnr_ylut117        : 8;		// bits : 15_8
      unsigned msnr_ylut118        : 8;		// bits : 23_16
      unsigned msnr_ylut119        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_85; // 0x0154

  union
  {
    struct
    {
      unsigned msnr_ylut120        : 8;		// bits : 7_0
      unsigned msnr_ylut121        : 8;		// bits : 15_8
      unsigned msnr_ylut122        : 8;		// bits : 23_16
      unsigned msnr_ylut123        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_86; // 0x0158

  union
  {
    struct
    {
      unsigned msnr_ylut124        : 8;		// bits : 7_0
      unsigned msnr_ylut125        : 8;		// bits : 15_8
      unsigned msnr_ylut126        : 8;		// bits : 23_16
      unsigned msnr_ylut127        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_87; // 0x015c

  union
  {
    struct
    {
      unsigned msnr_ulut0        : 8;		// bits : 7_0
      unsigned msnr_ulut1        : 8;		// bits : 15_8
      unsigned msnr_ulut2        : 8;		// bits : 23_16
      unsigned msnr_ulut3        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_88; // 0x0160

  union
  {
    struct
    {
      unsigned msnr_ulut4        : 8;		// bits : 7_0
      unsigned msnr_ulut5        : 8;		// bits : 15_8
      unsigned msnr_ulut6        : 8;		// bits : 23_16
      unsigned msnr_ulut7        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_89; // 0x0164

  union
  {
    struct
    {
      unsigned msnr_ulut8         : 8;		// bits : 7_0
      unsigned msnr_ulut9         : 8;		// bits : 15_8
      unsigned msnr_ulut10        : 8;		// bits : 23_16
      unsigned msnr_ulut11        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_90; // 0x0168

  union
  {
    struct
    {
      unsigned msnr_ulut12        : 8;		// bits : 7_0
      unsigned msnr_ulut13        : 8;		// bits : 15_8
      unsigned msnr_ulut14        : 8;		// bits : 23_16
      unsigned msnr_ulut15        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_91; // 0x016c

  union
  {
    struct
    {
      unsigned msnr_ulut16        : 8;		// bits : 7_0
      unsigned msnr_ulut17        : 8;		// bits : 15_8
      unsigned msnr_ulut18        : 8;		// bits : 23_16
      unsigned msnr_ulut19        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_92; // 0x0170

  union
  {
    struct
    {
      unsigned msnr_ulut20        : 8;		// bits : 7_0
      unsigned msnr_ulut21        : 8;		// bits : 15_8
      unsigned msnr_ulut22        : 8;		// bits : 23_16
      unsigned msnr_ulut23        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_93; // 0x0174

  union
  {
    struct
    {
      unsigned msnr_ulut24        : 8;		// bits : 7_0
      unsigned msnr_ulut25        : 8;		// bits : 15_8
      unsigned msnr_ulut26        : 8;		// bits : 23_16
      unsigned msnr_ulut27        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_94; // 0x0178

  union
  {
    struct
    {
      unsigned msnr_ulut28        : 8;		// bits : 7_0
      unsigned msnr_ulut29        : 8;		// bits : 15_8
      unsigned msnr_ulut30        : 8;		// bits : 23_16
      unsigned msnr_ulut31        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_95; // 0x017c

  union
  {
    struct
    {
      unsigned msnr_ulut32        : 8;		// bits : 7_0
      unsigned msnr_ulut33        : 8;		// bits : 15_8
      unsigned msnr_ulut34        : 8;		// bits : 23_16
      unsigned msnr_ulut35        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_96; // 0x0180

  union
  {
    struct
    {
      unsigned msnr_ulut36        : 8;		// bits : 7_0
      unsigned msnr_ulut37        : 8;		// bits : 15_8
      unsigned msnr_ulut38        : 8;		// bits : 23_16
      unsigned msnr_ulut39        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_97; // 0x0184

  union
  {
    struct
    {
      unsigned msnr_ulut40        : 8;		// bits : 7_0
      unsigned msnr_ulut41        : 8;		// bits : 15_8
      unsigned msnr_ulut42        : 8;		// bits : 23_16
      unsigned msnr_ulut43        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_98; // 0x0188

  union
  {
    struct
    {
      unsigned msnr_ulut44        : 8;		// bits : 7_0
      unsigned msnr_ulut45        : 8;		// bits : 15_8
      unsigned msnr_ulut46        : 8;		// bits : 23_16
      unsigned msnr_ulut47        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_99; // 0x018c

  union
  {
    struct
    {
      unsigned msnr_ulut48        : 8;		// bits : 7_0
      unsigned msnr_ulut49        : 8;		// bits : 15_8
      unsigned msnr_ulut50        : 8;		// bits : 23_16
      unsigned msnr_ulut51        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_100; // 0x0190

  union
  {
    struct
    {
      unsigned msnr_ulut52        : 8;		// bits : 7_0
      unsigned msnr_ulut53        : 8;		// bits : 15_8
      unsigned msnr_ulut54        : 8;		// bits : 23_16
      unsigned msnr_ulut55        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_101; // 0x0194

  union
  {
    struct
    {
      unsigned msnr_ulut56        : 8;		// bits : 7_0
      unsigned msnr_ulut57        : 8;		// bits : 15_8
      unsigned msnr_ulut58        : 8;		// bits : 23_16
      unsigned msnr_ulut59        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_102; // 0x0198

  union
  {
    struct
    {
      unsigned msnr_ulut60        : 8;		// bits : 7_0
      unsigned msnr_ulut61        : 8;		// bits : 15_8
      unsigned msnr_ulut62        : 8;		// bits : 23_16
      unsigned msnr_ulut63        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_103; // 0x019c

  union
  {
    struct
    {
      unsigned msnr_ulut64        : 8;		// bits : 7_0
      unsigned msnr_ulut65        : 8;		// bits : 15_8
      unsigned msnr_ulut66        : 8;		// bits : 23_16
      unsigned msnr_ulut67        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_104; // 0x01a0

  union
  {
    struct
    {
      unsigned msnr_ulut68        : 8;		// bits : 7_0
      unsigned msnr_ulut69        : 8;		// bits : 15_8
      unsigned msnr_ulut70        : 8;		// bits : 23_16
      unsigned msnr_ulut71        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_105; // 0x01a4

  union
  {
    struct
    {
      unsigned msnr_ulut72        : 8;		// bits : 7_0
      unsigned msnr_ulut73        : 8;		// bits : 15_8
      unsigned msnr_ulut74        : 8;		// bits : 23_16
      unsigned msnr_ulut75        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_106; // 0x01a8

  union
  {
    struct
    {
      unsigned msnr_ulut76        : 8;		// bits : 7_0
      unsigned msnr_ulut77        : 8;		// bits : 15_8
      unsigned msnr_ulut78        : 8;		// bits : 23_16
      unsigned msnr_ulut79        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_107; // 0x01ac

  union
  {
    struct
    {
      unsigned msnr_ulut80        : 8;		// bits : 7_0
      unsigned msnr_ulut81        : 8;		// bits : 15_8
      unsigned msnr_ulut82        : 8;		// bits : 23_16
      unsigned msnr_ulut83        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_108; // 0x01b0

  union
  {
    struct
    {
      unsigned msnr_ulut84        : 8;		// bits : 7_0
      unsigned msnr_ulut85        : 8;		// bits : 15_8
      unsigned msnr_ulut86        : 8;		// bits : 23_16
      unsigned msnr_ulut87        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_109; // 0x01b4

  union
  {
    struct
    {
      unsigned msnr_ulut88        : 8;		// bits : 7_0
      unsigned msnr_ulut89        : 8;		// bits : 15_8
      unsigned msnr_ulut90        : 8;		// bits : 23_16
      unsigned msnr_ulut91        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_110; // 0x01b8

  union
  {
    struct
    {
      unsigned msnr_ulut92        : 8;		// bits : 7_0
      unsigned msnr_ulut93        : 8;		// bits : 15_8
      unsigned msnr_ulut94        : 8;		// bits : 23_16
      unsigned msnr_ulut95        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_111; // 0x01bc

  union
  {
    struct
    {
      unsigned msnr_ulut96        : 8;		// bits : 7_0
      unsigned msnr_ulut97        : 8;		// bits : 15_8
      unsigned msnr_ulut98        : 8;		// bits : 23_16
      unsigned msnr_ulut99        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_112; // 0x01c0

  union
  {
    struct
    {
      unsigned msnr_ulut100        : 8;		// bits : 7_0
      unsigned msnr_ulut101        : 8;		// bits : 15_8
      unsigned msnr_ulut102        : 8;		// bits : 23_16
      unsigned msnr_ulut103        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_113; // 0x01c4

  union
  {
    struct
    {
      unsigned msnr_ulut104        : 8;		// bits : 7_0
      unsigned msnr_ulut105        : 8;		// bits : 15_8
      unsigned msnr_ulut106        : 8;		// bits : 23_16
      unsigned msnr_ulut107        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_114; // 0x01c8

  union
  {
    struct
    {
      unsigned msnr_ulut108        : 8;		// bits : 7_0
      unsigned msnr_ulut109        : 8;		// bits : 15_8
      unsigned msnr_ulut110        : 8;		// bits : 23_16
      unsigned msnr_ulut111        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_115; // 0x01cc

  union
  {
    struct
    {
      unsigned msnr_ulut112        : 8;		// bits : 7_0
      unsigned msnr_ulut113        : 8;		// bits : 15_8
      unsigned msnr_ulut114        : 8;		// bits : 23_16
      unsigned msnr_ulut115        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_116; // 0x01d0

  union
  {
    struct
    {
      unsigned msnr_ulut116        : 8;		// bits : 7_0
      unsigned msnr_ulut117        : 8;		// bits : 15_8
      unsigned msnr_ulut118        : 8;		// bits : 23_16
      unsigned msnr_ulut119        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_117; // 0x01d4

  union
  {
    struct
    {
      unsigned msnr_ulut120        : 8;		// bits : 7_0
      unsigned msnr_ulut121        : 8;		// bits : 15_8
      unsigned msnr_ulut122        : 8;		// bits : 23_16
      unsigned msnr_ulut123        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_118; // 0x01d8

  union
  {
    struct
    {
      unsigned msnr_ulut124        : 8;		// bits : 7_0
      unsigned msnr_ulut125        : 8;		// bits : 15_8
      unsigned msnr_ulut126        : 8;		// bits : 23_16
      unsigned msnr_ulut127        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_119; // 0x01dc

  union
  {
    struct
    {
      unsigned msnr_vlut0        : 8;		// bits : 7_0
      unsigned msnr_vlut1        : 8;		// bits : 15_8
      unsigned msnr_vlut2        : 8;		// bits : 23_16
      unsigned msnr_vlut3        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_120; // 0x01e0

  union
  {
    struct
    {
      unsigned msnr_vlut4        : 8;		// bits : 7_0
      unsigned msnr_vlut5        : 8;		// bits : 15_8
      unsigned msnr_vlut6        : 8;		// bits : 23_16
      unsigned msnr_vlut7        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_121; // 0x01e4

  union
  {
    struct
    {
      unsigned msnr_vlut8         : 8;		// bits : 7_0
      unsigned msnr_vlut9         : 8;		// bits : 15_8
      unsigned msnr_vlut10        : 8;		// bits : 23_16
      unsigned msnr_vlut11        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_122; // 0x01e8

  union
  {
    struct
    {
      unsigned msnr_vlut12        : 8;		// bits : 7_0
      unsigned msnr_vlut13        : 8;		// bits : 15_8
      unsigned msnr_vlut14        : 8;		// bits : 23_16
      unsigned msnr_vlut15        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_123; // 0x01ec

  union
  {
    struct
    {
      unsigned msnr_vlut16        : 8;		// bits : 7_0
      unsigned msnr_vlut17        : 8;		// bits : 15_8
      unsigned msnr_vlut18        : 8;		// bits : 23_16
      unsigned msnr_vlut19        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_124; // 0x01f0

  union
  {
    struct
    {
      unsigned msnr_vlut20        : 8;		// bits : 7_0
      unsigned msnr_vlut21        : 8;		// bits : 15_8
      unsigned msnr_vlut22        : 8;		// bits : 23_16
      unsigned msnr_vlut23        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_125; // 0x01f4

  union
  {
    struct
    {
      unsigned msnr_vlut24        : 8;		// bits : 7_0
      unsigned msnr_vlut25        : 8;		// bits : 15_8
      unsigned msnr_vlut26        : 8;		// bits : 23_16
      unsigned msnr_vlut27        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_126; // 0x01f8

  union
  {
    struct
    {
      unsigned msnr_vlut28        : 8;		// bits : 7_0
      unsigned msnr_vlut29        : 8;		// bits : 15_8
      unsigned msnr_vlut30        : 8;		// bits : 23_16
      unsigned msnr_vlut31        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_127; // 0x01fc

  union
  {
    struct
    {
      unsigned msnr_vlut32        : 8;		// bits : 7_0
      unsigned msnr_vlut33        : 8;		// bits : 15_8
      unsigned msnr_vlut34        : 8;		// bits : 23_16
      unsigned msnr_vlut35        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_128; // 0x0200

  union
  {
    struct
    {
      unsigned msnr_vlut36        : 8;		// bits : 7_0
      unsigned msnr_vlut37        : 8;		// bits : 15_8
      unsigned msnr_vlut38        : 8;		// bits : 23_16
      unsigned msnr_vlut39        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_129; // 0x0204

  union
  {
    struct
    {
      unsigned msnr_vlut40        : 8;		// bits : 7_0
      unsigned msnr_vlut41        : 8;		// bits : 15_8
      unsigned msnr_vlut42        : 8;		// bits : 23_16
      unsigned msnr_vlut43        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_130; // 0x0208

  union
  {
    struct
    {
      unsigned msnr_vlut44        : 8;		// bits : 7_0
      unsigned msnr_vlut45        : 8;		// bits : 15_8
      unsigned msnr_vlut46        : 8;		// bits : 23_16
      unsigned msnr_vlut47        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_131; // 0x020c

  union
  {
    struct
    {
      unsigned msnr_vlut48        : 8;		// bits : 7_0
      unsigned msnr_vlut49        : 8;		// bits : 15_8
      unsigned msnr_vlut50        : 8;		// bits : 23_16
      unsigned msnr_vlut51        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_132; // 0x0210

  union
  {
    struct
    {
      unsigned msnr_vlut52        : 8;		// bits : 7_0
      unsigned msnr_vlut53        : 8;		// bits : 15_8
      unsigned msnr_vlut54        : 8;		// bits : 23_16
      unsigned msnr_vlut55        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_133; // 0x0214

  union
  {
    struct
    {
      unsigned msnr_vlut56        : 8;		// bits : 7_0
      unsigned msnr_vlut57        : 8;		// bits : 15_8
      unsigned msnr_vlut58        : 8;		// bits : 23_16
      unsigned msnr_vlut59        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_134; // 0x0218

  union
  {
    struct
    {
      unsigned msnr_vlut60        : 8;		// bits : 7_0
      unsigned msnr_vlut61        : 8;		// bits : 15_8
      unsigned msnr_vlut62        : 8;		// bits : 23_16
      unsigned msnr_vlut63        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_135; // 0x021c

  union
  {
    struct
    {
      unsigned msnr_vlut64        : 8;		// bits : 7_0
      unsigned msnr_vlut65        : 8;		// bits : 15_8
      unsigned msnr_vlut66        : 8;		// bits : 23_16
      unsigned msnr_vlut67        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_136; // 0x0220

  union
  {
    struct
    {
      unsigned msnr_vlut68        : 8;		// bits : 7_0
      unsigned msnr_vlut69        : 8;		// bits : 15_8
      unsigned msnr_vlut70        : 8;		// bits : 23_16
      unsigned msnr_vlut71        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_137; // 0x0224

  union
  {
    struct
    {
      unsigned msnr_vlut72        : 8;		// bits : 7_0
      unsigned msnr_vlut73        : 8;		// bits : 15_8
      unsigned msnr_vlut74        : 8;		// bits : 23_16
      unsigned msnr_vlut75        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_138; // 0x0228

  union
  {
    struct
    {
      unsigned msnr_vlut76        : 8;		// bits : 7_0
      unsigned msnr_vlut77        : 8;		// bits : 15_8
      unsigned msnr_vlut78        : 8;		// bits : 23_16
      unsigned msnr_vlut79        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_139; // 0x022c

  union
  {
    struct
    {
      unsigned msnr_vlut80        : 8;		// bits : 7_0
      unsigned msnr_vlut81        : 8;		// bits : 15_8
      unsigned msnr_vlut82        : 8;		// bits : 23_16
      unsigned msnr_vlut83        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_140; // 0x0230

  union
  {
    struct
    {
      unsigned msnr_vlut84        : 8;		// bits : 7_0
      unsigned msnr_vlut85        : 8;		// bits : 15_8
      unsigned msnr_vlut86        : 8;		// bits : 23_16
      unsigned msnr_vlut87        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_141; // 0x0234

  union
  {
    struct
    {
      unsigned msnr_vlut88        : 8;		// bits : 7_0
      unsigned msnr_vlut89        : 8;		// bits : 15_8
      unsigned msnr_vlut90        : 8;		// bits : 23_16
      unsigned msnr_vlut91        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_142; // 0x0238

  union
  {
    struct
    {
      unsigned msnr_vlut92        : 8;		// bits : 7_0
      unsigned msnr_vlut93        : 8;		// bits : 15_8
      unsigned msnr_vlut94        : 8;		// bits : 23_16
      unsigned msnr_vlut95        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_143; // 0x023c

  union
  {
    struct
    {
      unsigned msnr_vlut96        : 8;		// bits : 7_0
      unsigned msnr_vlut97        : 8;		// bits : 15_8
      unsigned msnr_vlut98        : 8;		// bits : 23_16
      unsigned msnr_vlut99        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_144; // 0x0240

  union
  {
    struct
    {
      unsigned msnr_vlut100        : 8;		// bits : 7_0
      unsigned msnr_vlut101        : 8;		// bits : 15_8
      unsigned msnr_vlut102        : 8;		// bits : 23_16
      unsigned msnr_vlut103        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_145; // 0x0244

  union
  {
    struct
    {
      unsigned msnr_vlut104        : 8;		// bits : 7_0
      unsigned msnr_vlut105        : 8;		// bits : 15_8
      unsigned msnr_vlut106        : 8;		// bits : 23_16
      unsigned msnr_vlut107        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_146; // 0x0248

  union
  {
    struct
    {
      unsigned msnr_vlut108        : 8;		// bits : 7_0
      unsigned msnr_vlut109        : 8;		// bits : 15_8
      unsigned msnr_vlut110        : 8;		// bits : 23_16
      unsigned msnr_vlut111        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_147; // 0x024c

  union
  {
    struct
    {
      unsigned msnr_vlut112        : 8;		// bits : 7_0
      unsigned msnr_vlut113        : 8;		// bits : 15_8
      unsigned msnr_vlut114        : 8;		// bits : 23_16
      unsigned msnr_vlut115        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_148; // 0x0250

  union
  {
    struct
    {
      unsigned msnr_vlut116        : 8;		// bits : 7_0
      unsigned msnr_vlut117        : 8;		// bits : 15_8
      unsigned msnr_vlut118        : 8;		// bits : 23_16
      unsigned msnr_vlut119        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_149; // 0x0254

  union
  {
    struct
    {
      unsigned msnr_vlut120        : 8;		// bits : 7_0
      unsigned msnr_vlut121        : 8;		// bits : 15_8
      unsigned msnr_vlut122        : 8;		// bits : 23_16
      unsigned msnr_vlut123        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_150; // 0x0258

  union
  {
    struct
    {
      unsigned msnr_vlut124        : 8;		// bits : 7_0
      unsigned msnr_vlut125        : 8;		// bits : 15_8
      unsigned msnr_vlut126        : 8;		// bits : 23_16
      unsigned msnr_vlut127        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_151; // 0x025c

  union
  {
    struct
    {
      unsigned ycmod_ylut00        : 5;		// bits : 4_0
      unsigned                     : 3;
      unsigned ycmod_ylut01        : 5;		// bits : 12_8
      unsigned                     : 3;
      unsigned ycmod_ylut02        : 5;		// bits : 20_16
      unsigned                     : 3;
      unsigned ycmod_ylut03        : 5;		// bits : 28_24
    } bit;
    UINT32 word;
  } reg_152; // 0x0260

  union
  {
    struct
    {
      unsigned ycmod_ylut04        : 5;		// bits : 4_0
      unsigned                     : 3;
      unsigned ycmod_ylut05        : 5;		// bits : 12_8
      unsigned                     : 3;
      unsigned ycmod_ylut06        : 5;		// bits : 20_16
      unsigned                     : 3;
      unsigned ycmod_ylut07        : 5;		// bits : 28_24
    } bit;
    UINT32 word;
  } reg_153; // 0x0264

  union
  {
    struct
    {
      unsigned ycmod_ylut08        : 5;		// bits : 4_0
      unsigned                     : 3;
      unsigned ycmod_ylut09        : 5;		// bits : 12_8
      unsigned                     : 3;
      unsigned ycmod_ylut10        : 5;		// bits : 20_16
      unsigned                     : 3;
      unsigned ycmod_ylut11        : 5;		// bits : 28_24
    } bit;
    UINT32 word;
  } reg_154; // 0x0268

  union
  {
    struct
    {
      unsigned ycmod_ylut12        : 5;		// bits : 4_0
      unsigned                     : 3;
      unsigned ycmod_ylut13        : 5;		// bits : 12_8
      unsigned                     : 3;
      unsigned ycmod_ylut14        : 5;		// bits : 20_16
      unsigned                     : 3;
      unsigned ycmod_ylut15        : 5;		// bits : 28_24
    } bit;
    UINT32 word;
  } reg_155; // 0x026c

  union
  {
    struct
    {
      unsigned ycmod_clut00        : 5;		// bits : 4_0
      unsigned                     : 3;
      unsigned ycmod_clut01        : 5;		// bits : 12_8
      unsigned                     : 3;
      unsigned ycmod_clut02        : 5;		// bits : 20_16
      unsigned                     : 3;
      unsigned ycmod_clut03        : 5;		// bits : 28_24
    } bit;
    UINT32 word;
  } reg_156; // 0x0270

  union
  {
    struct
    {
      unsigned ycmod_clut04        : 5;		// bits : 4_0
      unsigned                     : 3;
      unsigned ycmod_clut05        : 5;		// bits : 12_8
      unsigned                     : 3;
      unsigned ycmod_clut06        : 5;		// bits : 20_16
      unsigned                     : 3;
      unsigned ycmod_clut07        : 5;		// bits : 28_24
    } bit;
    UINT32 word;
  } reg_157; // 0x0274

  union
  {
    struct
    {
      unsigned ycmod_clut08        : 5;		// bits : 4_0
      unsigned                     : 3;
      unsigned ycmod_clut09        : 5;		// bits : 12_8
      unsigned                     : 3;
      unsigned ycmod_clut10        : 5;		// bits : 20_16
      unsigned                     : 3;
      unsigned ycmod_clut11        : 5;		// bits : 28_24
    } bit;
    UINT32 word;
  } reg_158; // 0x0278

  union
  {
    struct
    {
      unsigned ycmod_clut12        : 5;		// bits : 4_0
      unsigned                     : 3;
      unsigned ycmod_clut13        : 5;		// bits : 12_8
      unsigned                     : 3;
      unsigned ycmod_clut14        : 5;		// bits : 20_16
      unsigned                     : 3;
      unsigned ycmod_clut15        : 5;		// bits : 28_24
    } bit;
    UINT32 word;
  } reg_159; // 0x027c

  union
  {
    struct
    {
      unsigned ycmod_cbofs         : 8;		// bits : 7_0
      unsigned ycmod_crofs         : 8;		// bits : 15_8
      unsigned ycmod_stepy         : 2;		// bits : 17_16
      unsigned ycmod_stepc         : 2;		// bits : 19_18
      unsigned ycmod_sel           : 1;		// bits : 20
      unsigned ycmod_lutsel        : 1;		// bits : 21
    } bit;
    UINT32 word;
  } reg_160; // 0x0280

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_161; // 0x0284

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_162; // 0x0288

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_163; // 0x028c

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_164; // 0x0290

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_165; // 0x0294

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_166; // 0x0298

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_167; // 0x029c

  union
  {
    struct
    {
      unsigned llc_chksum        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_168; // 0x02a0

  union
  {
    struct
    {
      unsigned llc_err_cmd_cnt        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_169; // 0x02a4

  union
  {
    struct
    {
      unsigned llc_err_cmd_adr        : 32;		// bits : 31_0
    } bit;
    UINT32 word;
  } reg_170; // 0x02a8

  union
  {
    struct
    {
      unsigned ll_table_idx0        : 8;		// bits : 7_0
      unsigned ll_table_idx1        : 8;		// bits : 15_8
      unsigned ll_table_idx2        : 8;		// bits : 23_16
      unsigned ll_table_idx3        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_171; // 0x02ac

  union
  {
    struct
    {
      unsigned ll_table_idx4        : 8;		// bits : 7_0
      unsigned ll_table_idx5        : 8;		// bits : 15_8
      unsigned ll_table_idx6        : 8;		// bits : 23_16
      unsigned ll_table_idx7        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_172; // 0x02b0

  union
  {
    struct
    {
      unsigned ll_table_idx8         : 8;		// bits : 7_0
      unsigned ll_table_idx9         : 8;		// bits : 15_8
      unsigned ll_table_idx10        : 8;		// bits : 23_16
      unsigned ll_table_idx11        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_173; // 0x02b4

  union
  {
    struct
    {
      unsigned ll_table_idx12        : 8;		// bits : 7_0
      unsigned ll_table_idx13        : 8;		// bits : 15_8
      unsigned ll_table_idx14        : 8;		// bits : 23_16
      unsigned ll_table_idx15        : 8;		// bits : 31_24
    } bit;
    UINT32 word;
  } reg_174; // 0x02b8
  
  union {
  	struct {
  		unsigned chksum_ich0		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_175; // 0x02bc
  union {
  	struct {
  		unsigned chksum_ich1		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_176; // 0x02c0
  union {
  	struct {
  		unsigned chksum_ich2		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_177; // 0x02c4
  union {
  	struct {
  		unsigned chksum_ich3		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_178; // 0x02c8
  union {
  	struct {
  		unsigned chksum_ich4		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_179; // 0x02cc
  union {
  	struct {
  		unsigned chksum_och0		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_180; // 0x02d0
  union {
  	struct {
  		unsigned chksum_och1		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_181; // 0x02d4
  
  union {
  	struct {
  		unsigned chksum_och2		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_182; // 0x02d8
  union {
  	struct {
  		unsigned chksum_och3		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_183; // 0x02dc
  union {
  	struct {
  		unsigned in_ch0_addr_msb    : 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_184; // 0x02e0
  union {
  	struct {
  		unsigned in_ch1_addr_msb    : 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_185; // 0x02e4
  union {
  	struct {
  		unsigned in_ch2_addr_msb    : 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_186; // 0x02e8
  union {
  	struct {
  		unsigned in_ch3_addr_msb    : 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_187; // 0x02ec
  union {
  	struct {
  		unsigned in_weight_addr_msb  	: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_188; // 0x02f0
  union {
  	struct {
  		unsigned in_ll_addr_msb 	: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_189; // 0x02f4
  union {
  	struct {
  		unsigned out_ch0_addr_msb   : 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_190; // 0x02f8
  union {
  	struct {
  		unsigned out_ch1_addr_msb   : 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_191; // 0x02fc
  union {
  	struct {
  		unsigned out_ch2_addr_msb  : 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_192; // 0x0300
  union {
  	struct {
  		unsigned out_weight_addr_msb : 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_193; // 0x0304
  union {
  	struct {
  		unsigned dre_in_patch_pixel_mask_addr_msb    : 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_194; // 0x02e0
  union {
  	struct {
  		unsigned chksum_ich5		: 32;		// bits : 31_0
  	} bit;
  	UINT32 word;
  } reg_195; // 0x02c8


} NT98690_DRE_ENG_REG_STRUCT;



#ifdef __cplusplus
}
#endif
#endif
