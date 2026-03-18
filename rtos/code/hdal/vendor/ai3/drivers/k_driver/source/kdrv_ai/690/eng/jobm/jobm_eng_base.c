
/*
    JOBM module driver

    NT98530 JOBM module driver.

    @file       jobm_eng.c
    @ingjobmp    mIIPPJOBM
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#include "kwrap/util.h"
#include "jobm_eng_base.h"
#include "jobm_eng_int_platform.h"
#include "jobm_eng_int_reg.h"
#include "jobm_eng_int_dbg.h"


/*
VOID jobm_eng_set_img_size(IVE_ENG_HANDLE *p_eng, UINT32 img_w, UINT32 img_h)
{
    p_eng->p_ive_reg_st->reg_22.bit.img_width = img_w;
    p_eng->p_ive_reg_st->reg_22.bit.img_height = img_h;

    p_eng->p_ive_reg_chg_flag[22] = TRUE;
}


// function control API
VOID jobm_eng_set_func_mode(IVE_ENG_HANDLE *p_eng, IVE_ENG_PATH_SEL path)
{
    p_eng->p_ive_reg_st->reg_4.bit.ive_mode = path;
    p_eng->p_ive_reg_chg_flag[4] = TRUE;
}

VOID ive_eng_set_canny_func_enable(IVE_ENG_HANDLE *p_eng, UINT32 Enable_func)
{
    if (Enable_func & IVE_ENG_CANNY_GEN_FILT_EN)
        p_eng->p_ive_reg_st->reg_4.bit.gen_filt_en = 1;
    if (Enable_func & IVE_ENG_CANNY_ORST_FILT_EN)
        p_eng->p_ive_reg_st->reg_4.bit.orst_filt_en = 1;
    if (Enable_func & IVE_ENG_CANNY_EDGE_FILT_EN)
        p_eng->p_ive_reg_st->reg_4.bit.edge_filt_en = 1;
    if (Enable_func & IVE_ENG_CANNY_NON_MAX_SUP_EN)
        p_eng->p_ive_reg_st->reg_4.bit.non_max_sup_en = 1;
    if (Enable_func & IVE_ENG_CANNY_THRES_EN)
        p_eng->p_ive_reg_st->reg_4.bit.thres_en = 1;
    if (Enable_func & IVE_ENG_CANNY_POSTPROC_EN)
        p_eng->p_ive_reg_st->reg_4.bit.postproc_en = 1;

    p_eng->p_ive_reg_chg_flag[4] = TRUE;
}

VOID ive_eng_set_va_func_enable(IVE_ENG_HANDLE *p_eng, UINT32 Enable_func)
{
	if (Enable_func & IVE_ENG_VA_GEN_FILT_EN)
        p_eng->p_ive_reg_st->reg_4.bit.gen_filt_en = 1;
    if (Enable_func & IVE_ENG_VA_MAP_EN)
        p_eng->p_ive_reg_st->reg_4.bit.map_en = 1;
    if (Enable_func & IVE_ENG_VA_EDGE_FILT_EN)
        p_eng->p_ive_reg_st->reg_4.bit.edge_filt_en = 1;
    if (Enable_func & IVE_ENG_CANNY_THRES_EN)
        p_eng->p_ive_reg_st->reg_4.bit.thres_en = 1;
    if (Enable_func & IVE_ENG_VA_INTEGRAL_EN)
        p_eng->p_ive_reg_st->reg_4.bit.integral_en = 1;

    p_eng->p_ive_reg_chg_flag[4] = TRUE;
}

VOID ive_eng_set_va_input_fmt(IVE_ENG_HANDLE *p_eng, UINT32 input_fmt)
{
	p_eng->p_ive_reg_st->reg_114.bit.va_in_fmt = input_fmt;
    p_eng->p_ive_reg_chg_flag[114] = TRUE;
}


VOID ive_eng_set_gen_filt(IVE_ENG_HANDLE *p_eng, IVE_ENG_GEN_FILT gen_filt)
{
    if (gen_filt.enable)
        p_eng->p_ive_reg_st->reg_4.bit.gen_filt_en = 1;
    else 
        p_eng->p_ive_reg_st->reg_4.bit.gen_filt_en = 0;
    p_eng->p_ive_reg_chg_flag[4] = TRUE;

    p_eng->p_ive_reg_st->reg_23.bit.gen_filt_coeff0 = gen_filt.coeff[0] & IVE_GEN_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_23.bit.gen_filt_coeff1 = gen_filt.coeff[1] & IVE_GEN_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_23.bit.gen_filt_coeff2 = gen_filt.coeff[2] & IVE_GEN_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_23.bit.gen_filt_coeff3 = gen_filt.coeff[3] & IVE_GEN_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[23] = TRUE;
	
	p_eng->p_ive_reg_st->reg_24.bit.gen_filt_coeff4 = gen_filt.coeff[4] & IVE_GEN_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_24.bit.gen_filt_coeff5 = gen_filt.coeff[5] & IVE_GEN_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_24.bit.gen_filt_coeff6 = gen_filt.coeff[6] & IVE_GEN_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_24.bit.gen_filt_coeff7 = gen_filt.coeff[7] & IVE_GEN_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[24] = TRUE;

    p_eng->p_ive_reg_st->reg_25.bit.gen_filt_coeff8  = gen_filt.coeff[8] & IVE_GEN_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_25.bit.gen_filt_coeff9  = gen_filt.coeff[9] & IVE_GEN_FILT_COEF_MSK;
	p_eng->p_ive_reg_st->reg_25.bit.gen_in_fmt       = gen_filt.input_fmt;
	p_eng->p_ive_reg_st->reg_25.bit.gen_in_sub_ratio = gen_filt.input_sub_ratio;
    p_eng->p_ive_reg_chg_flag[25] = TRUE;
}

VOID ive_eng_set_orst_filt(IVE_ENG_HANDLE *p_eng, IVE_ENG_ORST_FILT orst_filt)
{
    if (orst_filt.enable) {
        p_eng->p_ive_reg_st->reg_4.bit.orst_filt_en = 1;
    } else { 
        p_eng->p_ive_reg_st->reg_4.bit.orst_filt_en = 0;
	}
	p_eng->p_ive_reg_chg_flag[4] = TRUE;
	
    p_eng->p_ive_reg_st->reg_26.bit.orst_filt_mode = orst_filt.mode;
    p_eng->p_ive_reg_chg_flag[26] = TRUE;
}


VOID ive_eng_set_edge_filt(IVE_ENG_HANDLE *p_eng, IVE_ENG_EDGE_FILT edge_filt)
{
    if (edge_filt.enable) {
        p_eng->p_ive_reg_st->reg_4.bit.edge_filt_en = 1;
    } else { 
        p_eng->p_ive_reg_st->reg_4.bit.edge_filt_en = 0;
	}
	p_eng->p_ive_reg_chg_flag[4] = TRUE;
	
    p_eng->p_ive_reg_st->reg_27.bit.edge_mode = edge_filt.mode;
	p_eng->p_ive_reg_st->reg_27.bit.edge_kernel_mode = edge_filt.ker_mode;
    if (edge_filt.shift_bit > 20) {
		DBG_ERR("edge shift is %d, over than 20!!\r\n", edge_filt.shift_bit);
    }
    p_eng->p_ive_reg_st->reg_27.bit.edge_shift_bit = edge_filt.shift_bit & IVE_EDGE_FILT_SHF_BIT_MSK;
    p_eng->p_ive_reg_st->reg_27.bit.edge_angle_slp_fact = edge_filt.angle_slp;
    p_eng->p_ive_reg_chg_flag[27] = TRUE;

    p_eng->p_ive_reg_st->reg_28.bit.edge_filt0_coeff0 = edge_filt.coeff[0][0] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_28.bit.edge_filt0_coeff1 = edge_filt.coeff[0][1] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_28.bit.edge_filt0_coeff2 = edge_filt.coeff[0][2] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_28.bit.edge_filt0_coeff3 = edge_filt.coeff[0][3] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[28] = TRUE;
	
	p_eng->p_ive_reg_st->reg_29.bit.edge_filt0_coeff4 = edge_filt.coeff[0][4] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_29.bit.edge_filt0_coeff5 = edge_filt.coeff[0][5] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_29.bit.edge_filt0_coeff6 = edge_filt.coeff[0][6] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_29.bit.edge_filt0_coeff7 = edge_filt.coeff[0][7] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_chg_flag[29] = TRUE;
	
    p_eng->p_ive_reg_st->reg_30.bit.edge_filt0_coeff8  = edge_filt.coeff[0][8]  & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_30.bit.edge_filt0_coeff9  = edge_filt.coeff[0][9]  & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_30.bit.edge_filt0_coeff10 = edge_filt.coeff[0][10] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_30.bit.edge_filt0_coeff11 = edge_filt.coeff[0][11] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[30] = TRUE;

    p_eng->p_ive_reg_st->reg_31.bit.edge_filt0_coeff12 = edge_filt.coeff[0][12] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_31.bit.edge_filt0_coeff13 = edge_filt.coeff[0][13] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_31.bit.edge_filt0_coeff14 = edge_filt.coeff[0][14] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_31.bit.edge_filt0_coeff15 = edge_filt.coeff[0][15] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_chg_flag[31] = TRUE;
	
    p_eng->p_ive_reg_st->reg_32.bit.edge_filt0_coeff16 = edge_filt.coeff[0][16] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_32.bit.edge_filt0_coeff17 = edge_filt.coeff[0][17] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_st->reg_32.bit.edge_filt0_coeff18 = edge_filt.coeff[0][18] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_32.bit.edge_filt0_coeff19 = edge_filt.coeff[0][19] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[32] = TRUE;

    p_eng->p_ive_reg_st->reg_33.bit.edge_filt0_coeff20 = edge_filt.coeff[0][20] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_33.bit.edge_filt0_coeff21 = edge_filt.coeff[0][21] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_33.bit.edge_filt0_coeff22 = edge_filt.coeff[0][22] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_33.bit.edge_filt0_coeff23 = edge_filt.coeff[0][23] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[33] = TRUE;

    p_eng->p_ive_reg_st->reg_34.bit.edge_filt0_coeff24 = edge_filt.coeff[0][24] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_chg_flag[34] = TRUE;
	
    p_eng->p_ive_reg_st->reg_35.bit.edge_filt1_coeff0 = edge_filt.coeff[1][0] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_35.bit.edge_filt1_coeff1 = edge_filt.coeff[1][1] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_35.bit.edge_filt1_coeff2 = edge_filt.coeff[1][2] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_35.bit.edge_filt1_coeff3 = edge_filt.coeff[1][3] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_chg_flag[35] = TRUE;
	
    p_eng->p_ive_reg_st->reg_36.bit.edge_filt1_coeff4 = edge_filt.coeff[1][4] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_st->reg_36.bit.edge_filt1_coeff5 = edge_filt.coeff[1][5] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_36.bit.edge_filt1_coeff6 = edge_filt.coeff[1][6] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_36.bit.edge_filt1_coeff7 = edge_filt.coeff[1][7] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[36] = TRUE;
    
    p_eng->p_ive_reg_st->reg_37.bit.edge_filt1_coeff8  = edge_filt.coeff[1][8]  & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_37.bit.edge_filt1_coeff9  = edge_filt.coeff[1][9]  & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_37.bit.edge_filt1_coeff10 = edge_filt.coeff[1][10] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_st->reg_37.bit.edge_filt1_coeff11 = edge_filt.coeff[1][11] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[37] = TRUE;
    
    p_eng->p_ive_reg_st->reg_38.bit.edge_filt1_coeff12 = edge_filt.coeff[1][12] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_38.bit.edge_filt1_coeff13 = edge_filt.coeff[1][13] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_38.bit.edge_filt1_coeff14 = edge_filt.coeff[1][14] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_38.bit.edge_filt1_coeff15 = edge_filt.coeff[1][15] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_chg_flag[38] = TRUE;
	
    p_eng->p_ive_reg_st->reg_39.bit.edge_filt1_coeff16 = edge_filt.coeff[1][16] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_st->reg_39.bit.edge_filt1_coeff17 = edge_filt.coeff[1][17] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_39.bit.edge_filt1_coeff18 = edge_filt.coeff[1][18] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_39.bit.edge_filt1_coeff19 = edge_filt.coeff[1][19] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[39] = TRUE;

    p_eng->p_ive_reg_st->reg_40.bit.edge_filt1_coeff20 = edge_filt.coeff[1][20] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_40.bit.edge_filt1_coeff21 = edge_filt.coeff[1][21] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_st->reg_40.bit.edge_filt1_coeff22 = edge_filt.coeff[1][22] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_st->reg_40.bit.edge_filt1_coeff23 = edge_filt.coeff[1][23] & IVE_EDGE_FILT_COEF_MSK;
    p_eng->p_ive_reg_chg_flag[40] = TRUE;

    p_eng->p_ive_reg_st->reg_41.bit.edge_filt1_coeff24 = edge_filt.coeff[1][24] & IVE_EDGE_FILT_COEF_MSK;
	p_eng->p_ive_reg_st->reg_41.bit.edge_alpha_blending_factor = edge_filt.alpha_blending_factor;
    p_eng->p_ive_reg_chg_flag[41] = TRUE;    
}

VOID ive_eng_set_non_max_sup(IVE_ENG_HANDLE *p_eng, IVE_ENG_NON_MAX_SUP non_max_sup)
{
    if (non_max_sup.enable)
        p_eng->p_ive_reg_st->reg_4.bit.non_max_sup_en = 1;
    else 
        p_eng->p_ive_reg_st->reg_4.bit.non_max_sup_en = 0;

    p_eng->p_ive_reg_chg_flag[4] = TRUE;

    p_eng->p_ive_reg_st->reg_27.bit.edge_mag_th = non_max_sup.edge_mag_th;
    p_eng->p_ive_reg_chg_flag[27] = TRUE;
}

VOID ive_eng_set_integral_img(IVE_ENG_HANDLE *p_eng, IVE_ENG_INTEGRAL_IMG integral_img)
{
    if (integral_img.enable) {
        p_eng->p_ive_reg_st->reg_4.bit.integral_en = 1;
    } else { 
        p_eng->p_ive_reg_st->reg_4.bit.integral_en = 0;
    }
	p_eng->p_ive_reg_chg_flag[4] = TRUE;

    p_eng->p_ive_reg_st->reg_50.bit.integral_in_fmt = integral_img.in_fmt;
	p_eng->p_ive_reg_st->reg_50.bit.integral_out_fmt = integral_img.out_fmt;
	p_eng->p_ive_reg_st->reg_50.bit.integral_out_sub_ratio = integral_img.out_sub_ratio;
    p_eng->p_ive_reg_chg_flag[50] = TRUE;
}

VOID ive_eng_set_thres(IVE_ENG_HANDLE *p_eng, IVE_ENG_THRES thres)
{
    if (thres.enable)
        p_eng->p_ive_reg_st->reg_4.bit.thres_en = 1;
    else 
        p_eng->p_ive_reg_st->reg_4.bit.thres_en = 0;

    p_eng->p_ive_reg_chg_flag[4] = TRUE;

    p_eng->p_ive_reg_st->reg_42.bit.thres_mode = thres.mode;
    p_eng->p_ive_reg_chg_flag[42] = TRUE;

    p_eng->p_ive_reg_st->reg_43.bit.thres_low_th = thres.low_th;
    p_eng->p_ive_reg_chg_flag[43] = TRUE;
	
    p_eng->p_ive_reg_st->reg_44.bit.thres_high_th = thres.high_th;
    p_eng->p_ive_reg_chg_flag[44] = TRUE;
	
    p_eng->p_ive_reg_st->reg_45.bit.thres_out_min_val = thres.th_val.th_min_val;
	p_eng->p_ive_reg_chg_flag[45] = TRUE;
	
    p_eng->p_ive_reg_st->reg_46.bit.thres_out_mid_val = thres.th_val.th_mid_val;
	p_eng->p_ive_reg_chg_flag[46] = TRUE;
	
    p_eng->p_ive_reg_st->reg_47.bit.thres_out_max_val = thres.th_val.th_max_val;
    p_eng->p_ive_reg_chg_flag[47] = TRUE;
}

VOID ive_eng_set_post_proc(IVE_ENG_HANDLE *p_eng, IVE_ENG_POST_PROC postproc)
{
    UINT32 i, mask = 0;
    if (postproc.enable) {
        p_eng->p_ive_reg_st->reg_4.bit.postproc_en = 1;
    } else {
        p_eng->p_ive_reg_st->reg_4.bit.postproc_en = 0;
	}
	
	p_eng->p_ive_reg_chg_flag[4] = TRUE;
	
    p_eng->p_ive_reg_st->reg_48.bit.postproc_mode = postproc.mode;
	p_eng->p_ive_reg_st->reg_48.bit.postproc_morph_op = postproc.morph_op;
	for (i = 0; i < 24; i++) {
        if (postproc.mask_en_bit[i])
            mask |= (1 << i);
    }
    p_eng->p_ive_reg_st->reg_48.bit.postproc_morph_mask_en = mask;
    p_eng->p_ive_reg_chg_flag[48] = TRUE;

	p_eng->p_ive_reg_st->reg_49.bit.postproc_hyst_low_th  = postproc.hyst_low_th;
	p_eng->p_ive_reg_st->reg_49.bit.postproc_hyst_high_th = postproc.hyst_high_th;
    p_eng->p_ive_reg_chg_flag[49] = TRUE;
}

VOID ive_eng_set_canny_out_fmt(IVE_ENG_HANDLE *p_eng, IVE_ENG_CANNY_OUT_SEL out_fmt)
{
    p_eng->p_ive_reg_st->reg_4.bit.canny_out_sel = out_fmt;
    p_eng->p_ive_reg_chg_flag[4] = TRUE;
}

VOID ive_eng_set_path_sel(IVE_ENG_HANDLE *p_eng, IVE_ENG_PATH_SEL path_sel)
{
    p_eng->p_ive_reg_st->reg_4.bit.ive_mode = path_sel;
    p_eng->p_ive_reg_chg_flag[4] = TRUE;
}

VOID ive_eng_set_map(IVE_ENG_HANDLE *p_eng, IVE_ENG_MAP map)
{
    if (map.enable)
        p_eng->p_ive_reg_st->reg_4.bit.map_en = 1;
    else 
        p_eng->p_ive_reg_st->reg_4.bit.map_en = 0;
    p_eng->p_ive_reg_chg_flag[4] = TRUE;

	p_eng->p_ive_reg_st->reg_115.bit.map_mode = map.mode;
	p_eng->p_ive_reg_st->reg_115.bit.map_index_shift = map.index_shift;
    p_eng->p_ive_reg_chg_flag[115] = TRUE;
}

VOID ive_eng_set_csc_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_CSC_PATH csc_path)
{
    UINT8 i;
    INT16 tmpval = 0, regoffset = 0;
    
    p_eng->p_ive_reg_st->reg_51.bit.csc_yuv_mode = csc_path.in_fmt;
    p_eng->p_ive_reg_st->reg_51.bit.csc_mode_sel = csc_path.out_fmt >> 1;
    p_eng->p_ive_reg_st->reg_51.bit.csc_out_fmt  = csc_path.out_fmt & 0x1;
    p_eng->p_ive_reg_st->reg_51.bit.csc_hue_sft  = csc_path.hue_shift;
    p_eng->p_ive_reg_st->reg_51.bit.csc_lab_fmt  = csc_path.lab_fmt;
    p_eng->p_ive_reg_st->reg_51.bit.csc_gamma_en = csc_path.gamma_en;
    p_eng->p_ive_reg_chg_flag[51] = TRUE;

	for(i = 0; i < 9; i++) {
		tmpval = 0;
        regoffset = (i * 4) + CSC_COEF_REGISTER0_OFS;
		tmpval = csc_path.coef[i];
        IVE_ENG_SETREG((uintptr_t)p_eng->p_ive_reg_st + regoffset, tmpval);
        p_eng->p_ive_reg_chg_flag[regoffset>>2] = TRUE;
	}
	for(i = 0; i < 3; i++) {
		tmpval = 0;
        regoffset = (i * 4) + CSC_BIAS_REGISTER0_OFS;
		tmpval = csc_path.bias[i];
        IVE_ENG_SETREG((uintptr_t)p_eng->p_ive_reg_st + regoffset, tmpval);
        p_eng->p_ive_reg_chg_flag[regoffset>>2] = TRUE;
	}

}

VOID ive_eng_set_histogram_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_HISTOGRAM_PATH hist_path)
{
    p_eng->p_ive_reg_st->reg_88.bit.hist_mode = hist_path.mode;
	p_eng->p_ive_reg_chg_flag[88] = TRUE;
	
	//p_eng->p_ive_reg_st->reg_89.bit.hist_cdf_min = hist_path.hist_cdf_min;
	//p_eng->p_ive_reg_chg_flag[89] = TRUE;
	
	p_eng->p_ive_reg_st->reg_100.bit.op_coef_a = hist_path.coef_a;
	p_eng->p_ive_reg_chg_flag[100] = TRUE;
	
	p_eng->p_ive_reg_st->reg_101.bit.op_coef_b = hist_path.coef_b;
	p_eng->p_ive_reg_chg_flag[101] = TRUE;
	
	p_eng->p_ive_reg_st->reg_103.bit.op_shift_bit = hist_path.shift_bit;
    p_eng->p_ive_reg_chg_flag[103] = TRUE;
}

UINT32 ive_eng_get_hist_cdf_min(IVE_ENG_HANDLE *p_eng)
{
    UINT32 hist_cdf_min;

    hist_cdf_min = IVE_ENG_GETREG((uintptr_t)p_eng->reg_io_base + IVE_HIST_REGISTER1_OFS);
    return hist_cdf_min;
}

IVE_ENG_NCC_PATH ive_eng_get_ncc_rlt(IVE_ENG_HANDLE *p_eng)
{
    IVE_ENG_NCC_PATH ncc_path;
    
    ncc_path.numerator_lsb = IVE_ENG_GETREG((uintptr_t)p_eng->reg_io_base + NCC_OUTPUT_REGISTER0_OFS);
    ncc_path.numerator_msb = IVE_ENG_GETREG((uintptr_t)p_eng->reg_io_base + NCC_OUTPUT_REGISTER1_OFS);
    ncc_path.quad_sum0_lsb = IVE_ENG_GETREG((uintptr_t)p_eng->reg_io_base + NCC_OUTPUT_REGISTER2_OFS);
    ncc_path.quad_sum0_msb = IVE_ENG_GETREG((uintptr_t)p_eng->reg_io_base + NCC_OUTPUT_REGISTER3_OFS);
    ncc_path.quad_sum1_lsb = IVE_ENG_GETREG((uintptr_t)p_eng->reg_io_base + NCC_OUTPUT_REGISTER4_OFS);
    ncc_path.quad_sum1_msb = IVE_ENG_GETREG((uintptr_t)p_eng->reg_io_base + NCC_OUTPUT_REGISTER5_OFS);

    return ncc_path;
}

VOID ive_eng_set_dma_cpy_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_DMA_CPY_PATH dma_cpy_path)
{   
    p_eng->p_ive_reg_st->reg_96.bit.dma_hor_seg_sz = dma_cpy_path.hor_seg_size;
    p_eng->p_ive_reg_st->reg_96.bit.dma_ver_seg_sz = dma_cpy_path.ver_seg_size;
    p_eng->p_ive_reg_chg_flag[96] = TRUE;

	p_eng->p_ive_reg_st->reg_97.bit.dma_element_sz  = dma_cpy_path.element_size;
    p_eng->p_ive_reg_st->reg_97.bit.dma_ver_seg_num = dma_cpy_path.ver_seg_num;
    p_eng->p_ive_reg_chg_flag[97] = TRUE;
}

VOID ive_eng_set_img_op_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_IMG_OP_PATH img_op_path)
{
    p_eng->p_ive_reg_st->reg_98.bit.imgop_mode = img_op_path.mode;
    p_eng->p_ive_reg_st->reg_98.bit.imgop_sub_mode = img_op_path.sub_mode;
    p_eng->p_ive_reg_chg_flag[98] = TRUE;
    p_eng->p_ive_reg_st->reg_100.bit.op_coef_a = img_op_path.coef_a;
    p_eng->p_ive_reg_chg_flag[100] = TRUE;
    p_eng->p_ive_reg_st->reg_101.bit.op_coef_b = img_op_path.coef_b;
    p_eng->p_ive_reg_chg_flag[101] = TRUE;
    p_eng->p_ive_reg_st->reg_103.bit.op_shift_bit = img_op_path.shift_bit;
    p_eng->p_ive_reg_chg_flag[103] = TRUE;
}

VOID ive_eng_set_bit16_to_8_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_BIT16_TO_8_PATH bit16_8_path)
{
    p_eng->p_ive_reg_st->reg_99.bit.bit16_to_bit8_mode = bit16_8_path.mode;
    p_eng->p_ive_reg_chg_flag[99] = TRUE;
    p_eng->p_ive_reg_st->reg_100.bit.op_coef_a = bit16_8_path.coef_a;
    p_eng->p_ive_reg_chg_flag[100] = TRUE;
    p_eng->p_ive_reg_st->reg_101.bit.op_coef_b = bit16_8_path.coef_b;
    p_eng->p_ive_reg_chg_flag[101] = TRUE;
    p_eng->p_ive_reg_st->reg_102.bit.op_coef_bias = bit16_8_path.coef_bias;
    p_eng->p_ive_reg_chg_flag[102] = TRUE;
    p_eng->p_ive_reg_st->reg_103.bit.op_shift_bit = bit16_8_path.shift_bit;
    p_eng->p_ive_reg_chg_flag[103] = TRUE;
}


VOID ive_eng_set_ccl_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_CCL_PATH ccl_path)
{
    p_eng->p_ive_reg_st->reg_104.bit.ccl_connect_type = ccl_path.mode;
    p_eng->p_ive_reg_st->reg_104.bit.ccl_trig_mode = ccl_path.trig_mode;
    if (ccl_path.region_x_num)
        p_eng->p_ive_reg_st->reg_104.bit.ccl_region_x_num = ccl_path.region_x_num - 1;
    else
        DBG_ERR("ccl region_x_num is 0!!\r\n");
    if (ccl_path.region_y_num)
        p_eng->p_ive_reg_st->reg_104.bit.ccl_region_y_num = ccl_path.region_y_num - 1;
    else
        DBG_ERR("ccl region_y_num is 0!!\r\n");
    p_eng->p_ive_reg_st->reg_104.bit.ccl_foregjobmnd_val = ccl_path.foregjobmnd_val;
    p_eng->p_ive_reg_chg_flag[104] = TRUE;

    p_eng->p_ive_reg_st->reg_105.bit.ccl_region_width = ccl_path.region_w;
    if (ccl_path.region_h > 1)
        p_eng->p_ive_reg_st->reg_105.bit.ccl_region_height = ccl_path.region_h;
    else
        DBG_ERR("ccl region_h (%d) should be larger than 1!!\r\n", ccl_path.region_h);
    p_eng->p_ive_reg_chg_flag[105] = TRUE;
}

VOID ive_eng_set_st_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_ST_PATH st_path)
{
    p_eng->p_ive_reg_st->reg_106.bit.st_mode = st_path.mode;
	p_eng->p_ive_reg_st->reg_106.bit.st_max_corner_num = st_path.max_corner_num;
	p_eng->p_ive_reg_st->reg_106.bit.st_sort_topn = st_path.sort_topn;
	p_eng->p_ive_reg_chg_flag[106] = TRUE;
	
    p_eng->p_ive_reg_st->reg_107.bit.st_quality_level = st_path.quality_lvl;
    p_eng->p_ive_reg_st->reg_107.bit.st_min_dist = st_path.min_dist; 
    p_eng->p_ive_reg_st->reg_107.bit.st_blk_dist = st_path.blk_dist;
    p_eng->p_ive_reg_chg_flag[107] = TRUE;
    
	p_eng->p_ive_reg_st->reg_108.bit.st_stripe_blk_x_num = st_path.stripe_blk_x_num;
    p_eng->p_ive_reg_chg_flag[108] = TRUE;
	
	p_eng->p_ive_reg_st->reg_109.bit.st_blk_x_num = st_path.blk_x_num;
	p_eng->p_ive_reg_st->reg_109.bit.st_blk_y_num = st_path.blk_y_num;
    p_eng->p_ive_reg_chg_flag[109] = TRUE;
	
	p_eng->p_ive_reg_st->reg_110.bit.st_first_stage_point_num = st_path.first_stage_point_num;
	p_eng->p_ive_reg_chg_flag[110] = TRUE;
	
    p_eng->p_ive_reg_st->reg_111.bit.st_max_eigen_value = st_path.max_eigen_val;
    p_eng->p_ive_reg_chg_flag[111] = TRUE;
}


UINT16 ive_eng_get_st_corner_num(IVE_ENG_HANDLE *p_eng)
{
    UINT16 corner_num;

    corner_num = IVE_ENG_GETREG((uintptr_t)p_eng->reg_io_base + ST_CORNER_REGISTER6_OFS) & IVE_STCORNER_NUM_MSK;
    return corner_num;
}

UINT16 ive_eng_get_st_max_eigen_value(IVE_ENG_HANDLE *p_eng)
{
    UINT16 max_eigen_value;

    max_eigen_value = IVE_ENG_GETREG((uintptr_t)p_eng->reg_io_base + ST_CORNER_REGISTER5_OFS) & IVE_STCORNER_EIGVAL_MSK;
    return max_eigen_value;
}

VOID ive_eng_set_lbp_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_LBP_PATH lbp_path)
{
	p_eng->p_ive_reg_st->reg_113.bit.lbp_cmp_mode = lbp_path.mode;
    p_eng->p_ive_reg_st->reg_113.bit.lbp_threshold = lbp_path.threshold;
	p_eng->p_ive_reg_chg_flag[113] = TRUE;
}
*/
