#if defined(__FREERTOS)
#include "string.h"
#endif
#include "kwrap/error_no.h"
#include "kwrap/type.h"

#include "iq_alg_int.h"
#include "iq_flow.h"
#include "iq_lib.h"
#include "iq_ui_int.h"
#include "iq_nnsc_int.h"
#include "iq_common_param_int.h"
#include "iq_dbg.h"

// NOTE: aiisp aided test
extern BOOL aiisp_aided_test_en;
extern UINT32 aiisp_aided_test_clone_id;
extern UINT32 aiisp_aided_test_src_id;

ISOMAP iso_map_tab[IQ_GAIN_ID_MAX_NUM] = {
	{    100,      IQ_GAIN_1X},
	{    200,      IQ_GAIN_2X},
	{    400,      IQ_GAIN_4X},
	{    800,      IQ_GAIN_8X},
	{   1600,     IQ_GAIN_16X},
	{   3200,     IQ_GAIN_32X},
	{   6400,     IQ_GAIN_64X},
	{  12800,    IQ_GAIN_128X},
	{  25600,    IQ_GAIN_256X},
	{  51200,    IQ_GAIN_512X},
	{ 102400,   IQ_GAIN_1024X},
	{ 204800,   IQ_GAIN_2048X},
	{ 409600,   IQ_GAIN_4096X},
	{ 819200,   IQ_GAIN_8192X},
	{1638400,  IQ_GAIN_16384X},
	{3276800,  IQ_GAIN_32768X},
};

static UINT32 iq_sqrt_tab[41] = { // sqrt[x]*256, x = 0, 0.2, 0.4, ..., 8
	0, 114, 161, 198, 228, 256, 280, 302, 323, 343,
	362, 379, 396, 412, 428, 443, 457, 472, 485, 499,
	512, 524, 536, 549, 560, 572, 583, 594, 605, 616,
	627, 637, 647, 657, 667, 677, 686, 696, 705, 714,
	724
};

UINT16 de_gamma[CTL_IPE_ISP_TONE_MAP_LUT_LEN] = {
	0, 0, 0, 1, 2, 3, 4, 6, 9, 11,
	14, 18, 22, 26, 31, 36, 42, 48, 55, 63,
	70, 79, 88, 97, 107, 118, 129, 141, 153, 166,
	179, 193, 208, 223, 239, 255, 272, 290, 308, 327,
	347, 367, 388, 410, 432, 455, 479, 503, 528, 553,
	580, 607, 635, 663, 692, 722, 752, 784, 816, 848,
	882, 916, 951, 987, 1023};

static UINT32 iq_edge_ker_tab[17][2] = {
	{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8},
	{1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8} };

static UINT32 iq_edge_dir_w_tab[17][4] = {
	{ 0,  0,  0,  0},
	{ 1,  1,  0,  0},
	{ 2,  1,  1,  0},
	{ 2,  2,  1,  0},
	{ 3,  2,  1,  0},
	{ 4,  2,  1,  0},
	{ 4,  4,  2,  0},
	{ 6,  4,  2,  0},
	{ 8,  4,  2,  0},
	{10,  5,  2,  0},
	{12,  6,  3,  0},
	{14,  8,  4,  0},
	{16,  8,  4,  0},
	{16, 10,  6,  2},
	{16, 12,  8,  4},
	{16, 16, 12,  8},
	{16, 16, 16, 16} };

static UINT32 rgbir_2dnr_enh_ratio[IQ_GAIN_ID_MAX_NUM] = {160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160};

IQ_RAW_VA_IIR_PARAM iq_va_iir_tap[IQ_VA_IIR_SET_MAX_NUM] = {
	{ // Freq:0.12~0.24, H
		.iir1_tap_a = 0, .iir1_tap_b = 44, .iir1_tap_e = -343, .iir1_tap_f = 202, .iir1_div = 0, .iir1_shift = 8,
		.iir2_en = TRUE,
		.iir2_tap_a = 0, .iir2_tap_b = 44, .iir2_tap_e = -443, .iir2_tap_f = 223,                .iir2_shift = 8,
		.iir3_en = TRUE,
		.iir3_tap_a = 0, .iir3_tap_b = 41, .iir3_tap_e = -370, .iir3_tap_f = 174,                .iir3_shift = 8,
	},
	{ // Freq:0.04~0.12, M
		.iir1_tap_a = 0, .iir1_tap_b = 30, .iir1_tap_e = -440, .iir1_tap_f = 213, .iir1_div = 0, .iir1_shift = 8,
		.iir2_en = TRUE,
		.iir2_tap_a = 0, .iir2_tap_b = 30, .iir2_tap_e = -491, .iir2_tap_f = 239,                .iir2_shift = 8,
		.iir3_en = TRUE,
		.iir3_tap_a = 0, .iir3_tap_b = 29, .iir3_tap_e = -444, .iir3_tap_f = 199,                .iir3_shift = 8,
	},
	{ // Freq:0.02~0.08, L
		.iir1_tap_a = 0, .iir1_tap_b = 23, .iir1_tap_e = -464, .iir1_tap_f = 221, .iir1_div = 0, .iir1_shift = 8,
		.iir2_en = TRUE,
		.iir2_tap_a = 0, .iir2_tap_b = 23, .iir2_tap_e = -501, .iir2_tap_f = 246,                .iir2_shift = 8,
		.iir3_en = TRUE,
		.iir3_tap_a = 0, .iir3_tap_b = 22, .iir3_tap_e = -464, .iir3_tap_f = 212,                .iir3_shift = 8,
	},
	{ // Freq:0.02~0.04, L2
		.iir1_tap_a = 0, .iir1_tap_b =  8, .iir1_tap_e = -498, .iir1_tap_f = 246, .iir1_div = 0, .iir1_shift = 8,
		.iir2_en = TRUE,
		.iir2_tap_a = 0, .iir2_tap_b =  8, .iir2_tap_e = -505, .iir2_tap_f = 250,                .iir2_shift = 8,
		.iir3_en = TRUE,
		.iir3_tap_a = 0, .iir3_tap_b =  8, .iir3_tap_e = -494, .iir3_tap_f = 240,                .iir3_shift = 8,
	},
};

static void iq_intpl_tbl_cc(INT16 *l_value, INT16 *h_value, INT32 l_index, INT32 h_index, INT32 index, INT16 *target)
{
	INT32 i, E1, E2;

	for (i = 0; i < IQ_CCM_LEN; i++) {
		E1 = l_value[i];
		if (E1 > 0x800) {
			E1 = E1 - 0x1000;
		}
		E2 = h_value[i];
		if (E2 > 0x800) {
			E2 = E2 - 0x1000;
		}
		target[i] = iq_intpl(index, E1, E2, l_index, h_index);
	}
}

static void iq_intpl_tbl_3dcc(UINT32 *l_value, UINT32 *h_value, INT32 l_index, INT32 h_index, INT32 index, UINT32 *target)
{
	INT32 i;
	UINT32 R1, G1, B1;
	UINT32 R2, G2, B2;

	for (i = 0; i < IQ_3DCC_LEN; i++) {
		R1 = (l_value[i] & 0x000000ff);
		R2 = (h_value[i] & 0x000000ff);
		R1 = IQ_CLAMP(iq_intpl(index, R1, R2, l_index, h_index), 0, 0xff);

		G1 = (l_value[i] & 0x0000ff00) >> 8;
		G2 = (h_value[i] & 0x0000ff00) >> 8;
		G1 = IQ_CLAMP(iq_intpl(index, G1, G2, l_index, h_index), 0, 0xff);

		B1 = (l_value[i] & 0x00ff0000) >> 16;
		B2 = (h_value[i] & 0x00ff0000) >> 16;
		B1 = IQ_CLAMP(iq_intpl(index, B1, B2, l_index, h_index), 0, 0xff);

		target[i] = (B1 << 16) + (G1 << 8) + R1;
	}
}

static void iq_intpl_tbl_gamma(UINT32 *l_value, UINT32 *h_value, INT32 l_index, INT32 h_index, INT32 index, UINT32 *target)
{
	INT32 i;

	for (i = 0; i < CTL_IPE_ISP_GAMMA_LEN; i++) {
		target[i] = iq_intpl(index, l_value[i], h_value[i], l_index, h_index);
	}
}

static void iq_gamma_down_sample2(UINT32 *gamma, UINT16 *eext_tone)
{
	INT32 i = 0, j = 0;

	for (i = 0; i < CTL_IPE_ISP_GAMMA_LEN; i += 2) {
		eext_tone[j++] = (UINT16)gamma[i];
	}
}

static void iq_gamma_down_sample8(UINT32 *gamma, UINT8 *pfr_gamma)
{
	INT32 i = 0, j = 0;

	for (i = 0; i < CTL_IPE_ISP_GAMMA_LEN; i += 8) {
		pfr_gamma[j++] = (UINT8)IQ_CLAMP((gamma[i] + 2) >> 2, 0, 255);
	}
}

INT32 iq_cal(INT32 ratio, INT32 int_value, INT32 min_value, INT32 max_value, IQ_CAL_WAY way)
{
	switch ((UINT32)way) {
	case IQ_CAL_MULTIPLY:
		if (ratio == 100) {
			return int_value;
		}
		return IQ_CLAMP(int_value * ratio / 100, min_value, max_value);

	case IQ_CAL_INTPL:
		if (ratio == 100) {
			return int_value;
		} else if (ratio > 100) {
			return iq_intpl(ratio, int_value, max_value, 100, 200);
		} else {
			return iq_intpl(ratio, min_value, int_value, 0, 100);
		}

	case IQ_CAL_HUE:
		return (int_value + (ratio * 256) / 360) % 256;

	default:
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(0) & IQ_DBG_WRN_MSG, "unknow IQ_CAL_WAY(%d), return input value \r\n", way);
		return int_value;
	}
	return 0;
}

static void iq_operation_wdr(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, IQALG_WDR *wdr_set)
{
	BOOL dbg_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_A_WDR) ? TRUE : FALSE);
	UINT32 wdr_target_level;

	if (iq_param->wdr->mode == IQ_OP_TYPE_AUTO) {
		wdr_target_level = iq_info->dr_level;
		wdr_target_level = IQ_CLAMP(wdr_target_level, 0, 240);
		PRINT_IQ(dbg_en, "pre ui_strength wdr %d \r\n", wdr_target_level);
		wdr_target_level = (wdr_target_level * wdr_set->auto_set.level) >> 7;
		PRINT_IQ(dbg_en, "ui_level %d \r\n", wdr_set->auto_set.level);
		PRINT_IQ(dbg_en, "post ui_strength wdr %d \r\n", wdr_target_level);
		wdr_target_level = IQ_CLAMP(wdr_target_level, wdr_set->auto_set.strength_min, wdr_set->auto_set.strength_max);
		PRINT_IQ(dbg_en, "ui_clamp (%d, %d) \r\n", wdr_set->auto_set.strength_min, wdr_set->auto_set.strength_max);
		PRINT_IQ(dbg_en, "post ui_clamp wdr %d \r\n", wdr_target_level);

		iq_info->target_wdr_str = wdr_target_level;
	}
}

#define IQ_DFG_STR_UPDATE_RATE 3
#define IQ_DFG_AIRLIGHT_UPDATE_RATE 30
static void iq_operation_defog(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, IQALG_DEFOG *defog_set)
{
	BOOL dbg_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_A_DEFOG) ? TRUE : FALSE);
	UINT16 fog_mod_level;
	IQLIB_DEFOG defog_result;
	UINT16 dfg_airlight_result[CTL_IPE_ISP_DFG_AIRLIGHT_NUM];
	UINT16 air_min = 1023;
	UINT32 i;
	static BOOL first_run = TRUE;
	static IQLIB_DEFOG defog_preset[IQ_ID_MAX_NUM];
	static UINT16 dfg_airlight_preset[IQ_ID_MAX_NUM][CTL_IPE_ISP_DFG_AIRLIGHT_NUM];

	if (first_run) {
		for (i = 0; i < IQ_ID_MAX_NUM; i++) {
			defog_preset[i].fog_level = 0,
			defog_preset[i].outbld_lum_wt = 192,
			defog_preset[i].gain_th = 96,
			dfg_airlight_preset[i][0] = 1023;
			dfg_airlight_preset[i][1] = 1023;
			dfg_airlight_preset[i][2] = 1023;
		}
		first_run = FALSE;
	}

	if (iq_param->defog->mode == IQ_OP_TYPE_AUTO) {
		{
			IQLIB_DEFOG_STRENGTH defog_strength = {0};

			defog_strength.dr_th = defog_set->auto_set.dr_th;
			defog_strength.fog_mod_level_lb = (1024 - defog_set->auto_set.fog_level_max);
			defog_strength.subimg_size_w = iq_info->final_ipp.ipe_subimg.subimg_size.h_size;
			defog_strength.subimg_size_h = iq_info->final_ipp.ipe_subimg.subimg_size.v_size;
			defog_strength.cnt_ratio_th1 = IQLIB_DEFOG_CNT_RATIO_TH1;
			defog_strength.cnt_ratio_th2 = IQLIB_DEFOG_CNT_RATIO_TH2;
			defog_strength.subout_min = (UINT16 *)iq_info->dfg_subout_min;
			defog_strength.subout_avg = (UINT16 *)iq_info->dfg_subout_avg;
			defog_strength.dynamic_range = (UINT16 *)iq_info->dfg_dynamic_range;
			defog_strength.dbg_en = dbg_en;

			iq_lib_defog_strength(&defog_strength);

			defog_result.fog_level = defog_strength.result.fog_level;
			defog_result.outbld_lum_wt = defog_strength.result.outbld_lum_wt;
			defog_result.gain_th = defog_strength.result.gain_th;
		}

		if ((iq_info->ipp_trig_obj.reset != TRUE) && (iq_info->ipp_trig_obj.ipp_capture != TRUE)) {
			//prevent dramatic change (fog_mod_lut)
			if (defog_preset[iq_info->id].fog_level > defog_result.fog_level) {
				defog_preset[iq_info->id].fog_level -= IQ_MAX(1, (defog_preset[iq_info->id].fog_level - defog_result.fog_level) / IQ_DFG_STR_UPDATE_RATE);
			} else if (defog_preset[iq_info->id].fog_level < defog_result.fog_level) {
				defog_preset[iq_info->id].fog_level += IQ_MAX(1, (defog_result.fog_level - defog_preset[iq_info->id].fog_level) / IQ_DFG_STR_UPDATE_RATE);
			} else {
				defog_preset[iq_info->id].fog_level = defog_result.fog_level;
			}
		} else {
			if (iq_info->ipp_trig_obj.ipp_capture != TRUE) {
				defog_preset[iq_info->id].fog_level = defog_result.fog_level;
			}
		}
		fog_mod_level = defog_preset[iq_info->id].fog_level;

		// 0: strong, 255: weak
		defog_result.outbld_lum_wt = IQ_CLAMP(defog_result.outbld_lum_wt + (((255 - defog_result.outbld_lum_wt) * defog_set->auto_set.outbld_wt) >> 8), 0, 255);
		PRINT_IQ(dbg_en, "auto_set outbld_wt = %d\r\n", defog_set->auto_set.outbld_wt);
		if ((iq_info->ipp_trig_obj.reset != TRUE) && (iq_info->ipp_trig_obj.ipp_capture != TRUE)) {
			//prevent dramatic change (str)
			if (defog_preset[iq_info->id].outbld_lum_wt > defog_result.outbld_lum_wt) {
				defog_preset[iq_info->id].outbld_lum_wt -= IQ_MAX(1, (defog_preset[iq_info->id].outbld_lum_wt - defog_result.outbld_lum_wt) / IQ_DFG_STR_UPDATE_RATE);
			} else if (defog_preset[iq_info->id].outbld_lum_wt < defog_result.outbld_lum_wt) {
				defog_preset[iq_info->id].outbld_lum_wt += IQ_MAX(1, (defog_result.outbld_lum_wt - defog_preset[iq_info->id].outbld_lum_wt) / IQ_DFG_STR_UPDATE_RATE);
			} else {
				defog_preset[iq_info->id].outbld_lum_wt = defog_result.outbld_lum_wt;
			}
		} else {
			if (iq_info->ipp_trig_obj.ipp_capture != TRUE) {
				defog_preset[iq_info->id].outbld_lum_wt = defog_result.outbld_lum_wt;
			}
		}
		for (i = 0; i < CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN; i++) {
			iq_info->final_ipp.ipe_defog.dfg_outbld.outbld_lum_wt[i] =  IQ_CLAMP(defog_preset[iq_info->id].outbld_lum_wt, 0, 255);
		}

		if ((iq_info->ipp_trig_obj.reset != TRUE) && (iq_info->ipp_trig_obj.ipp_capture != TRUE)) {
			//prevent dramatic change (gain_th)
			if (defog_preset[iq_info->id].gain_th > defog_result.gain_th) {
				defog_preset[iq_info->id].gain_th -= IQ_MAX(1, (defog_preset[iq_info->id].gain_th - defog_result.gain_th) / IQ_DFG_STR_UPDATE_RATE);
			} else if (defog_preset[iq_info->id].gain_th < defog_result.gain_th) {
				defog_preset[iq_info->id].gain_th += IQ_MAX(1, (defog_result.gain_th - defog_preset[iq_info->id].gain_th) / IQ_DFG_STR_UPDATE_RATE);
			} else {
				defog_preset[iq_info->id].gain_th = defog_result.gain_th;
			}
		} else {
			if (iq_info->ipp_trig_obj.ipp_capture != TRUE) {
				defog_preset[iq_info->id].gain_th = defog_result.gain_th;
			}
		}
		iq_info->final_ipp.ipe_defog.dfg_strength.gain_th = IQ_CLAMP(defog_preset[iq_info->id].gain_th, 32, 255);
	} else {
		fog_mod_level = defog_set->manual_set.fog_level;
	}
	iq_lib_defog_fog_mod_level_to_lut(fog_mod_level, iq_info->final_ipp.ipe_defog.env_estimation.fog_mod_lut);
	if (dbg_en) {
		PRINT_IQ(dbg_en, "Final Defog outbld_lum_wt = ");
		PRINT_IQ_ARR(dbg_en, iq_info->final_ipp.ipe_defog.dfg_outbld.outbld_lum_wt, CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN);
		PRINT_IQ(dbg_en, "\r\n");
		PRINT_IQ(dbg_en, "Final Defog gain_th = %d\r\n", iq_info->final_ipp.ipe_defog.dfg_strength.gain_th);
		PRINT_IQ(dbg_en, "Final Defog fog_mod_lut = ");
		PRINT_IQ_ARR(dbg_en, iq_info->final_ipp.ipe_defog.env_estimation.fog_mod_lut, CTL_IPE_ISP_DFG_FOG_MOD_LEN);
		PRINT_IQ(dbg_en, "\r\n");
	}

	{
		IQLIB_DEFOG_AIRLIGHT defog_airlight = {0};

		defog_airlight.th = IQLIB_DEFOG_AIRLIGHT_TH;
		defog_airlight.diff_max_ratio = IQLIB_DEFOG_AIRLIGHT_DIFF_MAX_RATIO;
		memcpy(&defog_airlight.airlight, iq_info->dfg_airlight, sizeof(UINT16) * IQLIB_DFG_AIRLIGHT_NUM);
		defog_airlight.dbg_en = dbg_en;
		iq_lib_defog_airlight(&defog_airlight);
		dfg_airlight_result[0] = defog_airlight.r;
		dfg_airlight_result[1] = defog_airlight.g;
		dfg_airlight_result[2] = defog_airlight.b;
		PRINT_IQ(dbg_en, "Defog airlight result = {%d, %d, %d} \n", dfg_airlight_result[0], dfg_airlight_result[1], dfg_airlight_result[2]);
	}

	if ((iq_info->ipp_trig_obj.reset != TRUE) && (iq_info->ipp_trig_obj.ipp_capture != TRUE)) {
		//prevent dramatic change
		for (i = 0; i < CTL_IPE_ISP_DFG_AIRLIGHT_NUM; i++) {
			if (dfg_airlight_preset[iq_info->id][i] > dfg_airlight_result[i]) {
				dfg_airlight_preset[iq_info->id][i] -= IQ_MAX(1, (dfg_airlight_preset[iq_info->id][i] - dfg_airlight_result[i]) / IQ_DFG_AIRLIGHT_UPDATE_RATE);
			} else if (dfg_airlight_preset[iq_info->id][i] < dfg_airlight_result[i]) {
				dfg_airlight_preset[iq_info->id][i] += IQ_MAX(1, (dfg_airlight_result[i] - dfg_airlight_preset[iq_info->id][i]) / IQ_DFG_AIRLIGHT_UPDATE_RATE);
			} else {
				dfg_airlight_preset[iq_info->id][i] = dfg_airlight_result[i];
			}
		}
	} else {
		if (iq_info->ipp_trig_obj.ipp_capture != TRUE) {
			for (i = 0; i < CTL_IPE_ISP_DFG_AIRLIGHT_NUM; i++) {
				dfg_airlight_preset[iq_info->id][i] = dfg_airlight_result[i];
			}
		}
	}
	PRINT_IQ(dbg_en, "Final Defog airlight = {%d, %d, %d}\n", dfg_airlight_preset[iq_info->id][0], dfg_airlight_preset[iq_info->id][1], dfg_airlight_preset[iq_info->id][2]);

	iq_info->final_ipp.ipe_defog.env_estimation.dfg_airlight[0] = IQ_CLAMP(dfg_airlight_preset[iq_info->id][0], 0, 1023);
	iq_info->final_ipp.ipe_defog.env_estimation.dfg_airlight[1] = IQ_CLAMP(dfg_airlight_preset[iq_info->id][1], 0, 1023);
	iq_info->final_ipp.ipe_defog.env_estimation.dfg_airlight[2] = IQ_CLAMP(dfg_airlight_preset[iq_info->id][2], 0, 1023);

	for (i = 0; i < CTL_IPE_ISP_DFG_AIRLIGHT_NUM; i++) {
		if (iq_info->final_ipp.ipe_defog.env_estimation.dfg_airlight[i] < air_min) {
			air_min = iq_info->final_ipp.ipe_defog.env_estimation.dfg_airlight[i];
		}
	}
	iq_info->final_ipp.ipe_defog.env_estimation.dfg_min_diff = air_min * iq_param->defog->min_diff_ratio / 16;
	PRINT_IQ(dbg_en, "Final Defog dfg_min_diff = %d\n", iq_info->final_ipp.ipe_defog.env_estimation.dfg_min_diff);
}

static void iq_operation_shdr_fusion(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param)
{
	// sync_info.ev_ratio = 64 * (2 ^ ev_ratio), 4EV=16*64=1024, 3EV = 8*64=512, 2EV=4*64=256, 1EV=2*64=128, 0EV=1*64=64
	// fu_ctrl.ev_ratio = 16 * (2 ^ ev_ratio),   4EV=16*16= 256, 3EV = 8*16=128, 2EV=4*16= 64, 1EV=2*16= 32, 0EV=1*16=16
	UINT32 iq_ev_ratio;
	BOOL is_8ev;
	BOOL dbg_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_A_SHDR) ? TRUE : FALSE);
	UINT32 i;

	iq_info->final_ipp.pre_fusion.fu_ctrl.ev_ratio = IQ_CLAMP(iq_info->final_ipp.sync_info.shdr_ev_ratio[1] >> 2, 16, 256 * 16); // 8ev: 256 * 16

	iq_ev_ratio = IQ_CLAMP(iq_info->final_ipp.sync_info.shdr_ev_ratio[1] >> 2, 16, 256 * 16);
	is_8ev = FALSE;
	for (i = 0; i < IQ_SHDR_FCURVE_END_NUM; i++) {
		if (iq_param->shdr->fcurve_end_lut[i] > 65535) {
			is_8ev = TRUE;
			break;
		}
	}

	if (iq_param->shdr->auto_ev_enable) {
		UINT32 iq_ev_ratio_max = (is_8ev) ? 4096 : 256;

		iq_info->final_ipp.pre_fusion.bld_cur.s_nor_knee[0] = (iq_param->shdr->fusion_s_nor_knee * iq_lib_get_fcurve_y(iq_ev_ratio_max)) / iq_lib_get_fcurve_y(iq_ev_ratio);
		iq_info->final_ipp.pre_fusion.bld_cur.s_nor_range = (iq_param->shdr->fusion_s_nor_range * iq_lib_get_fcurve_y(iq_ev_ratio_max)) / iq_lib_get_fcurve_y(iq_ev_ratio);
		iq_info->final_ipp.pre_fusion.bld_cur.s_dif_knee[0] = (iq_param->shdr->fusion_s_dif_knee * iq_lib_get_fcurve_y(iq_ev_ratio_max)) / iq_lib_get_fcurve_y(iq_ev_ratio);
		iq_info->final_ipp.pre_fusion.bld_cur.s_dif_range = (iq_param->shdr->fusion_s_dif_range * iq_lib_get_fcurve_y(iq_ev_ratio_max)) / iq_lib_get_fcurve_y(iq_ev_ratio);
	} else {
		iq_info->final_ipp.pre_fusion.bld_cur.s_nor_knee[0] = iq_param->shdr->fusion_s_nor_knee;
		iq_info->final_ipp.pre_fusion.bld_cur.s_nor_range = iq_param->shdr->fusion_s_nor_range;
		iq_info->final_ipp.pre_fusion.bld_cur.s_dif_knee[0] = iq_param->shdr->fusion_s_dif_knee;
		iq_info->final_ipp.pre_fusion.bld_cur.s_dif_range = iq_param->shdr->fusion_s_dif_range;
	}

	iq_info->final_ipp.pre_fusion.bld_cur.s_nor_slope = IQ_CLAMP(256 * 4096 / iq_info->final_ipp.pre_fusion.bld_cur.s_nor_range, 0, 65535);
	iq_info->final_ipp.pre_fusion.bld_cur.s_dif_slope = IQ_CLAMP(256 * 4096 / iq_info->final_ipp.pre_fusion.bld_cur.s_dif_range, 0, 65535);

	PRINT_IQ_VAR(dbg_en, iq_param->shdr->auto_ev_enable);
	PRINT_IQ_VAR(dbg_en, iq_info->final_ipp.pre_fusion.fu_ctrl.ev_ratio);
	PRINT_IQ_VAR(dbg_en, iq_info->final_ipp.pre_fusion.bld_cur.s_nor_knee[0]);
	PRINT_IQ_VAR(dbg_en, iq_info->final_ipp.pre_fusion.bld_cur.s_nor_range);
	PRINT_IQ_VAR(dbg_en, iq_info->final_ipp.pre_fusion.bld_cur.s_nor_slope);
	PRINT_IQ_VAR(dbg_en, iq_info->final_ipp.pre_fusion.bld_cur.s_dif_knee[0]);
	PRINT_IQ_VAR(dbg_en, iq_info->final_ipp.pre_fusion.bld_cur.s_dif_range);
	PRINT_IQ_VAR(dbg_en, iq_info->final_ipp.pre_fusion.bld_cur.s_dif_slope);
}

static void iq_operation_shdr_fcurve(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param)
{
	IQLIB_SHDR_FCURVE_INFO shdr_fcurve_lib_input;
	IQLIB_SHDR_FCURVE shdr_fcurve_lib_output;
	UINT32 i;

	shdr_fcurve_lib_input.dbg_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_A_SHDR) ? TRUE : FALSE);
	shdr_fcurve_lib_input.auto_ev_en = iq_param->shdr->auto_ev_enable;
	shdr_fcurve_lib_input.frame_num = iq_info->ipp_trig_obj.frame_num;
	// sync_info.ev_ratio = 64 * (2 ^ ev_ratio), 4EV=16*64=1024, 3EV = 8*64=512, 2EV=4*64=256, 1EV=2*64=128, 0EV=1*64=64
	// fu_ctrl.ev_ratio = 16 * (2 ^ ev_ratio),   4EV=16*16= 256, 3EV = 8*16=128, 2EV=4*16= 64, 1EV=2*16= 32, 0EV=1*16=16
	shdr_fcurve_lib_input.ev_ratio = IQ_CLAMP(iq_info->final_ipp.sync_info.shdr_ev_ratio[1] >> 2, 16, 256 * 16);
	shdr_fcurve_lib_input.is_8ev = FALSE;
	for (i = 0; i < IQ_SHDR_FCURVE_END_NUM; i++) {
		if (iq_param->shdr->fcurve_end_lut[i] > 65535) {
			shdr_fcurve_lib_input.is_8ev = TRUE;
			break;
		}
	}
	shdr_fcurve_lib_input.tm_ratio = iq_info->final_ipp.sync_info.shdr_tm_ratio;
	shdr_fcurve_lib_input.hbs_param.lum_th = iq_info->final_ipp.sync_info.shdr_hbs_param.lum_th;
	shdr_fcurve_lib_input.hbs_param.w_start = iq_info->final_ipp.sync_info.shdr_hbs_param.w_start;
	shdr_fcurve_lib_input.hbs_param.w_slope = iq_info->final_ipp.sync_info.shdr_hbs_param.w_slope;

	shdr_fcurve_lib_input.left_lut = iq_param->shdr->fcurve_left_lut;
	shdr_fcurve_lib_input.right_lut = iq_param->shdr->fcurve_right_lut;
	shdr_fcurve_lib_input.end_lut = iq_param->shdr->fcurve_end_lut;

	shdr_fcurve_lib_output.ev_fmt = 0;
	shdr_fcurve_lib_output.left_lut = iq_info->final_ipp.pre_fcurve.fcurve_l.fcur_l_lut;
	shdr_fcurve_lib_output.right_lut = iq_info->final_ipp.pre_fcurve.fcurve_r.fcur_r_lut;
	shdr_fcurve_lib_output.end_lut = iq_info->final_ipp.pre_fcurve.fcurve_end.fcur_end_lut;

	iq_lib_shdr_fcurve(&shdr_fcurve_lib_input, &shdr_fcurve_lib_output);
	iq_info->final_ipp.pre_fcurve.fcur_ctrl.ev_fmt = shdr_fcurve_lib_output.ev_fmt;
}

#define IQ_RGBIR_ENH_RATIO_BASE 16
void iq_operation_iso(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param)
{
	IQ_SYNC_INFO iq_sync_info = {0};
	UINT32 ob_ratio;
	UINT32 iso_idx_l = 0, iso_idx_h = 0;
	UINT32 iso_start = 0, iso_end = 0;
	UINT32 iso_idx_l_enh = 0, iso_idx_h_enh = 0;
	UINT32 iso_start_enh = 0, iso_end_enh = 0;
	UINT32 wdr_idx_l = 0, wdr_idx_h = 0;
	UINT32 sie_cgain[3] = {0};
	UINT16 vig_zero[IQ_SHADING_VIG_LEN] = {0};
	UINT32 curr_ob[IQ_OB_LEN] = {0};
	IQ_VA_IIR_FILTER_SET curr_va_g1_iir_set = IQ_VA_IIR_SET_FREQ_H, curr_va_g2_iir_set = IQ_VA_IIR_SET_FREQ_H;
	IQ_NR_OUTL_SEL curr_outl_sel = IQ_NR_OUTL_8_NODE;
	static UINT16 curr_nr_filter_th[IQ_NR_TH_NUM] = {0}, curr_nr_filter_th_b[IQ_NR_TH_NUM] = {0};
	static UINT16 curr_nr_filter_lut[IQ_NR_TH_LUT] = {0}, curr_nr_filter_lut_b[IQ_NR_TH_LUT] = {0};
	static UINT16 curr_nr_filter_th_1[IQ_NR_TH_NUM] = {0}, curr_nr_filter_th_b_1[IQ_NR_TH_NUM] = {0};
	static UINT16 curr_nr_filter_lut_1[IQ_NR_TH_LUT] = {0}, curr_nr_filter_lut_b_1[IQ_NR_TH_LUT] = {0};
	static UINT16 curr_nr_filter_th_2[IQ_NR_TH_NUM] = {0}, curr_nr_filter_th_b_2[IQ_NR_TH_NUM] = {0};
	static UINT16 curr_nr_filter_lut_2[IQ_NR_TH_LUT] = {0}, curr_nr_filter_lut_b_2[IQ_NR_TH_LUT] = {0};
	UINT32 curr_rgain_sqrt_id = 0, curr_bgain_sqrt_id = 0;
	UINT32 curr_rgain_sqrt = 0, curr_bgain_sqrt = 0;
	UINT32 curr_thin_freq = 0, curr_robust_freq = 0;
	UINT32 curr_th_flat_low = 0, curr_th_flat_high = 0, curr_th_edge_low = 0, curr_th_edge_high = 0;
	INT32 curr_slope_flat = 0, curr_slope_edge = 0;
	UINT32 curr_dir_eng_blend_w = 0;
	UINT8 curr_lce_lum_wt_lut[IQ_CONTRAST_LCE_LEN] = {0};
	UINT8 curr_probability = 0;
	UINT32 curr_pfr_strength = 0;
	IQALG_WDR curr_wdr_set = {0};
	IQALG_DEFOG curr_defog_set = {0};
	UINT16 curr_shdr_nrs_s_str[IQ_SHDR_NRS_STR_NUM] = {0};
	UINT16 curr_fpn_gain;
	UINT32 ir_2dnr_enh_ratio;
	UINT32 ir_d_light_enh_ratio, ir_a_light_enh_ratio, ir_ir_light_enh_ratio, ir_final_enh_ratio;
	UINT32 wdr_final_enh_ratio;
	UINT32 ui_night_mode = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_NIGHT_MODE);
	BOOL dbg_rgbir_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_A_RGBIR) ? TRUE : FALSE);
	BOOL dbg_iso_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_O_ISO) ? TRUE : FALSE);
	BOOL dbg_tmnr_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_O_TMNR) ? TRUE : FALSE);
	UINT32 diff_lv;
	UINT32 i;
	UINT32 ob_ofs;
	static UINT32 pre_lv[IQ_ID_MAX_NUM];
	static UINT32 tmnr_residue_count[IQ_ID_MAX_NUM];
	static BOOL curr_tmnr_still[IQ_ID_MAX_NUM];
	static UINT32 tmnr_ae_count[IQ_ID_MAX_NUM];

	CTL_PRE_ISP_NRS                  *pre_nrs_0 =  &iq_info->final_ipp.pre_nrs_0;
	CTL_PRE_ISP_FUSION               *pre_fusion =  &iq_info->final_ipp.pre_fusion;
	CTL_PRE_ISP_BNR                  *pre_bnr = &iq_info->final_ipp.pre_bnr;
	CTL_PRE_ISP_OUTL                 *pre_outl = &iq_info->final_ipp.pre_outl;
	CTL_PRE_ISP_VA                   *pre_raw_va = &iq_info->final_ipp.pre_va;
	CTL_PRE_ISP_FPN                  *pre_fpn = &iq_info->final_ipp.pre_fpn;
	CTL_PRE_ISP_CGAIN                *pre_cgain =  &iq_info->final_ipp.pre_cgain;
	CTL_IPP_ISP_AIISP_PARAM          *aiisp = &iq_info->final_ipp.aiisp;
	CTL_IFE_ISP_FILTER               *ife_filter = &iq_info->final_ipp.ife_filter;
	CTL_IFE_ISP_DGAIN                *ife_dgain =  &iq_info->final_ipp.ife_dgain;
	CTL_IFE_ISP_CGAIN                *ife_cgain =  &iq_info->final_ipp.ife_cgain;
	CTL_IFE_ISP_WDR                  *ife_wdr = &iq_info->final_ipp.ife_wdr;
	CTL_IFE_ISP_GBAL                 *ife_gbal = &iq_info->final_ipp.ife_gbal;
	CTL_IPE_ISP_EEXT                 *ipe_eext = &iq_info->final_ipp.ipe_eext;
	CTL_IPE_ISP_EDGE_OVERSHOOT       *ipe_edge_overshoot = &iq_info->final_ipp.ipe_edge_overshoot;
	CTL_IPE_ISP_EPROC                *ipe_eproc = &iq_info->final_ipp.ipe_eproc;
	CTL_IPE_ISP_PFR                  *ipe_pfr = &iq_info->final_ipp.ipe_pfr;
	CTL_IPE_ISP_CC                   *ipe_cc = &iq_info->final_ipp.ipe_cc;
	CTL_IPE_ISP_CCTRL                *ipe_cctrl = &iq_info->final_ipp.ipe_cctrl;
	CTL_IPE_ISP_CADJ_YCCON           *ipe_cadj_yccon = &iq_info->final_ipp.ipe_cadj_yccon;
	CTL_IPE_ISP_CSTP                 *ipe_cstp = &iq_info->final_ipp.ipe_cstp;
	CTL_IPE_ISP_DEFOG                *ipe_defog = &iq_info->final_ipp.ipe_defog;
	CTL_IPE_ISP_LCE                  *ipe_lce = &iq_info->final_ipp.ipe_lce;
	CTL_IPE_ISP_EDGE_REGION_STR      *ipe_edge_region_str = &iq_info->final_ipp.ipe_edge_region_str;
	CTL_IPE_ISP_CFA                  *ipe_cfa = &iq_info->final_ipp.ipe_cfa;
	CTL_IME_ISP_LCA                  *ime_lca = &iq_info->final_ipp.ime_lca;
	CTL_IME_ISP_DBCS                 *ime_dbcs = &iq_info->final_ipp.ime_dbcs;
	CTL_IME_ISP_TMNR                 *ime_tmnr = &iq_info->final_ipp.ime_tmnr;
	CTL_IME_ISP_SHARPEN              *ime_post_sharpen_1 = &iq_info->final_ipp.ime_sharpen;
	KDRV_H26XENC_SPN                 *enc_post_sharpen_2 = &iq_info->final_enc.post_sharpen;
	CTL_IPP_ISP_FPN_INFO             *pre_fpn_info = &iq_info->final_ipp.pre_fpn_info;
	IQ_OB_TUNE_PARAM                 *ob_m, *ob_l, *ob_h;
	IQ_NR_TUNE_PARAM                 *nr_m, *nr_l, *nr_h, *nr_lca_m, *nr_lca_l, *nr_lca_h;
	IQ_NR_EXT_TUNE_PARAM             *nr_ext_m, *nr_ext_l, *nr_ext_h;
	IQ_CFA_TUNE_PARAM                *cfa_m, *cfa_l, *cfa_h;
	IQ_RAW_VA_MANUAL_PARAM           *raw_va_m;
	IQ_RAW_VA_AUTO_PARAM             *raw_va_l, *raw_va_h;
	IQ_COLOR_TUNE_PARAM              *color_m, *color_l, *color_h;
	IQ_CONTRAST_TUNE_PARAM           *contrast_m, *contrast_l, *contrast_h;
	IQ_EDGE_TUNE_PARAM               *edge_m, *edge_l, *edge_h;
	IQ_EDGE_EXT_TUNE_PARAM           *edge_ext_m, *edge_ext_l, *edge_ext_h;
	IQ_3DNR_TUNE_PARAM               *_3dnr_m, *_3dnr_l, *_3dnr_h;
	IQ_3DNR_EXT_TUNE_PARAM           *_3dnr_ext_m, *_3dnr_ext_l, *_3dnr_ext_h;
	IQ_PFR_TUNE_PARAM                *pfr_m, *pfr_l, *pfr_h;
	IQ_WDR_MANUAL_PARAM              *wdr_m;
	IQ_WDR_AUTO_PARAM                *wdr_l, *wdr_h;
	IQ_DEFOG_MANUAL_PARAM            *defog_m;
	IQ_DEFOG_AUTO_PARAM              *defog_l, *defog_h;
	IQ_FPN_TUNE_PARAM                *fpn_m, *fpn_l, *fpn_h;
	IQ_SHDR_TUNE_PARAM               *shdr_m, *shdr_l, *shdr_h;
	IQ_POST_SHARPEN_1_TUNE_PARAM     *post_sharpen_1_m, *post_sharpen_1_l, *post_sharpen_1_h;
	IQ_POST_SHARPEN_2_TUNE_PARAM     *post_sharpen_2_m, *post_sharpen_2_l, *post_sharpen_2_h;
	IQ_RGBIR_ENH_MANUAL_PARAM        *rgbir_enh_m;
	IQ_RGBIR_ENH_AUTO_PARAM          *rgbir_enh_d, *rgbir_enh_a, *rgbir_enh_ir;
	IQ_CST_TUNE_PARAM                *cst_m, *cst_l, *cst_h;
	IQ_BNR_TUNE_PARAM                *bnr_m, *bnr_l, *bnr_h;
	IQ_AIISP_TUNE_PARAM              *aiisp_m, *aiisp_l, *aiisp_h;

	switch ((UINT32)msg) {
	case ISP_TRIG_IQ_SIE:
		iq_sync_info = iq_info->final_sie.sync_info;

		if (iq_sync_info.gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
			iso_idx_l = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
			iso_idx_h = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
			iso_start = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso;
			iso_end = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso;
		}
		for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
			if (iq_sync_info.gain < iso_map_tab[i].iso) {
				iso_idx_l = iso_map_tab[i - 1].index;
				iso_idx_h = iso_map_tab[i].index;
				iso_start = iso_map_tab[i - 1].iso;
				iso_end = iso_map_tab[i].iso;
				break;
			}
		}

		ob_m = &iq_param->ob->manual_param;
		ob_l = &iq_param->ob->auto_param[iso_idx_l];
		ob_h = &iq_param->ob->auto_param[iso_idx_h];

		// AWB_CGain operation
		if (iq_param->ob->mode == IQ_OP_TYPE_AUTO) {
			iq_intpl_tbl_uint32(ob_l->cofs, ob_h->cofs, IQ_OB_LEN, curr_ob, iq_sync_info.gain, iso_start, iso_end);
		} else {
			memcpy(curr_ob, ob_m->cofs, sizeof(UINT32) * IQ_OB_LEN);
		}
		if (iq_param->ob->enable == FALSE) {
			memset(curr_ob, 0, sizeof(UINT32) * IQ_OB_LEN);
		}
		ob_ofs = ((curr_ob[1] + curr_ob[2]) >> 1);
		ob_ratio = (4096 - ob_ofs);
		sie_cgain[0] = IQ_CLAMP(((iq_sync_info.cgain[0] << 12) + (ob_ratio - 1)) / ob_ratio, 0, 2047);
		sie_cgain[1] = IQ_CLAMP(((iq_sync_info.cgain[1] << 12) + (ob_ratio - 1)) / ob_ratio, 0, 2047);
		sie_cgain[2] = IQ_CLAMP(((iq_sync_info.cgain[2] << 12) + (ob_ratio - 1)) / ob_ratio, 0, 2047);

		iq_info->final_sie.final_cgain[0] = sie_cgain[0];
		iq_info->final_sie.final_cgain[1] = sie_cgain[1];
		iq_info->final_sie.final_cgain[2] = sie_cgain[2];

		// AWB_CGain operation
		if (iq_info->cg_mode == IQ_CG_SIE) {
			if (sie_cgain[0] > 1023 || sie_cgain[1] > 1023 || sie_cgain[2] > 1023) {
				iq_info->final_sie.sie_cgain.sel_37_fmt = TRUE;
				iq_info->final_sie.sie_cgain.r_gain  = IQ_CLAMP(sie_cgain[0] >> 1, 0, 1023);
				iq_info->final_sie.sie_cgain.gr_gain = IQ_CLAMP(sie_cgain[1] >> 1, 0, 1023);
				iq_info->final_sie.sie_cgain.gb_gain = IQ_CLAMP(sie_cgain[1] >> 1, 0, 1023);
				iq_info->final_sie.sie_cgain.b_gain  = IQ_CLAMP(sie_cgain[2] >> 1, 0, 1023);
				iq_info->final_sie.sie_cgain.ir_gain = IQ_CLAMP(sie_cgain[1] >> 1, 0, 1023);
			} else {
				iq_info->final_sie.sie_cgain.sel_37_fmt = FALSE;
				iq_info->final_sie.sie_cgain.r_gain  = IQ_CLAMP(sie_cgain[0], 0, 1023);
				iq_info->final_sie.sie_cgain.gr_gain = IQ_CLAMP(sie_cgain[1], 0, 1023);
				iq_info->final_sie.sie_cgain.gb_gain = IQ_CLAMP(sie_cgain[1], 0, 1023);
				iq_info->final_sie.sie_cgain.b_gain  = IQ_CLAMP(sie_cgain[2], 0, 1023);
				iq_info->final_sie.sie_cgain.ir_gain = IQ_CLAMP(sie_cgain[1], 0, 1023);
			}
		} else {
			iq_info->final_sie.sie_cgain.sel_37_fmt = TRUE;
			iq_info->final_sie.sie_cgain.r_gain = 128;
			iq_info->final_sie.sie_cgain.gr_gain = 128;
			iq_info->final_sie.sie_cgain.gb_gain = 128;
			iq_info->final_sie.sie_cgain.b_gain = 128;
			iq_info->final_sie.sie_cgain.ir_gain = 128;
		}

		if ((iq_info->dg_mode == IQ_DG_SIE) || (iq_info->dg_mode == IQ_DG_AI)) {
			iq_info->final_sie.sie_dgain.enable = TRUE;
			iq_info->final_sie.sie_dgain.enable_stcs = TRUE;
		} else {
			iq_info->final_sie.sie_dgain.enable = FALSE;
			iq_info->final_sie.sie_dgain.enable_stcs = TRUE;
		}
		if (iq_info->dg_mode_manual.sie_enable || iq_info->dg_mode_manual.pre_enable || iq_info->dg_mode_manual.ife_enable) {
			UINT64 total_manual_gain = 0;

			if (iq_info->dg_mode_manual.sie_enable && iq_info->dg_mode_manual.pre_enable && iq_info->dg_mode_manual.ife_enable) {
				PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "IQ(%d) sie_enable/pre_enable/ife_enable cannot be enabled at the same time. \r\n", iq_info->id);
			}

			iq_info->final_sie.sie_dgain.enable = iq_info->dg_mode_manual.sie_enable ? TRUE : FALSE;
			total_manual_gain = iq_info->dg_mode_manual.sie_enable ? iq_info->dg_mode_manual.sie_value : 128;
			total_manual_gain = iq_info->dg_mode_manual.pre_enable ? total_manual_gain * iq_info->dg_mode_manual.pre_value : total_manual_gain * 128;
			total_manual_gain = iq_info->dg_mode_manual.ife_enable ? total_manual_gain * iq_info->dg_mode_manual.ife_value : total_manual_gain * 128;

			iq_info->final_sie.sie_dgain.gain = IQ_CLAMP((UINT32)((total_manual_gain << 8) >> 21), 0, 65535);
		} else {
			iq_info->final_sie.sie_dgain.gain = IQ_CLAMP(iq_sync_info.dgain << 1, 0, 65535);
		}

		if ((iq_info->dg_mode == IQ_DG_AI) && iq_info->dg_mode_manual.sie_dg_max && (iq_info->final_sie.sie_dgain.gain > (iq_info->dg_mode_manual.sie_dg_max << 1))) {
			iq_info->dg_ai = IQ_CLAMP((iq_info->final_sie.sie_dgain.gain << 7) / (iq_info->dg_mode_manual.sie_dg_max << 1), 0, 32767);
			iq_info->final_sie.sie_dgain.gain_stcs = IQ_CLAMP((iq_info->final_sie.sie_dgain.gain << 8) / (iq_info->dg_mode_manual.sie_dg_max << 1), 0, 65535);
			iq_info->final_sie.sie_dgain.gain = IQ_CLAMP((iq_info->dg_mode_manual.sie_dg_max << 1), 0, 65535);
			if (iq_info->dg_ai_enable) {
				if (iq_info->dg_ai > 512) {
					iq_info->dg_ai2 = (iq_info->dg_ai << 7) / 512;
					iq_info->dg_ai = 512;
				} else if (iq_info->dg_ai > 256) {
					iq_info->dg_ai2 = (iq_info->dg_ai << 7) / 256;
					iq_info->dg_ai = 256;
				} else {
					iq_info->dg_ai2 = (iq_info->dg_ai << 7) / 128;
					iq_info->dg_ai = 128;
				}
			} else {
				iq_info->dg_ai2 = 128;
			}
		} else {
			iq_info->dg_ai2 = 128;
			iq_info->dg_ai = (1 << 7);
			iq_info->final_sie.sie_dgain.gain_stcs = (1 << 8);
		}

		// IQ_OB_PARAM setting
		if (iq_info->ob_mode_manual.sie_enable) {
			iq_info->final_sie.sie_ob.ob_ofs = IQ_CLAMP(iq_info->ob_mode_manual.sie_value, 0, 4095);
		} else {
			if (iq_info->ob_mode == IQ_OB_AI) {
				UINT32 sie_ob_ofs;

				if (((iq_info->final_sie.sie_dgain.gain * ob_ofs) >> 8) > 1023) {
					sie_ob_ofs = ((((iq_info->final_sie.sie_dgain.gain * ob_ofs) >> 8) - 1023) << 8) / (iq_info->final_sie.sie_dgain.gain);
					iq_info->ob_ai = 1023;
				} else {
					sie_ob_ofs = 0;
					iq_info->ob_ai = IQ_CLAMP(((iq_info->final_sie.sie_dgain.gain * ob_ofs) >> 8), 0, 1023);
				}

				iq_info->final_sie.sie_ob.ob_ofs = IQ_CLAMP(sie_ob_ofs, 0, 4095);
			} else {
				iq_info->final_sie.sie_ob.ob_ofs = IQ_CLAMP(ob_ofs, 0, 4095);
			}
		}

		if ((iq_info->ob_mode == IQ_OB_SIE) || (iq_info->ob_mode == IQ_OB_AI)) {
			iq_info->final_sie.sie_ob.bypass_enable = FALSE;
		} else {
			iq_info->final_sie.sie_ob.bypass_enable = TRUE;
		}

		if (iq_info->final_sie.sie_ob.bypass_enable) {
			if (iq_info->ob_mode_manual.sie_enable) {
				if (iq_info->ob_mode_manual.sie_value == 0) {
					iq_info->final_sie.sie_ca.ca_ob_ofs = IQ_CLAMP(((ob_ofs * iq_sync_info.dgain) >> 7), 0, 4095);
				} else {
					iq_info->final_sie.sie_ca.ca_ob_ofs = IQ_CLAMP(iq_info->ob_mode_manual.sie_value, 0, 4095);
				}
			} else {
				iq_info->final_sie.sie_ca.ca_ob_ofs = IQ_CLAMP(ob_ofs, 0, 4095);
			}
		} else {
			if (iq_info->ob_mode == IQ_OB_AI) {
				iq_info->final_sie.sie_ca.ca_ob_ofs = iq_info->ob_ai;
			} else {
				iq_info->final_sie.sie_ca.ca_ob_ofs = 0;
			}
		}

		if (iq_info->final_sie.sie_ob.bypass_enable) {
			if (iq_info->ob_mode_manual.sie_enable) {
				if (iq_info->ob_mode_manual.sie_value == 0) {
					iq_info->final_sie.sie_la.la_ob_ofs = IQ_CLAMP(((ob_ofs * iq_sync_info.dgain) >> 7), 0, 4095);
				} else {
					iq_info->final_sie.sie_la.la_ob_ofs = IQ_CLAMP(iq_info->ob_mode_manual.sie_value, 0, 4095);
				}
			} else {
				iq_info->final_sie.sie_la.la_ob_ofs = IQ_CLAMP(ob_ofs, 0, 4095);
			}
		} else {
			if (iq_info->ob_mode == IQ_OB_AI) {
				iq_info->final_sie.sie_la.la_ob_ofs = iq_info->ob_ai;
			} else {
				iq_info->final_sie.sie_la.la_ob_ofs = 0;
			}
		}
		break;

	case ISP_TRIG_IQ_IPP:
		{
			IQLIB_WDR iqlib_wdr = {0};

			iqlib_wdr.dark_th = IQLIB_WDR_DARK_TH;
			iqlib_wdr.dr_th = IQLIB_WDR_DR_TH;
			iqlib_wdr.dark_compensate = IQLIB_WDR_DARK_COMPENSATE;
			iqlib_wdr.hist_stcs_pre_wdr = (UINT16 *)iq_info->hist_stcs_pre_wdr;
			iqlib_wdr.dbg_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_A_WDR) ? TRUE : FALSE);

			iq_lib_wdr_strength(&iqlib_wdr);

			iq_info->dr_level = iqlib_wdr.dr_level;
		}

		iq_sync_info = iq_info->final_ipp.sync_info;

		if (iq_sync_info.gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
			iso_idx_l = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
			iso_idx_h = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
			iso_start = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso;
			iso_end = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso;
		}
		for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
			if (iq_sync_info.gain < iso_map_tab[i].iso) {
				iso_idx_l = iso_map_tab[i - 1].index;
				iso_idx_h = iso_map_tab[i].index;
				iso_start = iso_map_tab[i - 1].iso;
				iso_end = iso_map_tab[i].iso;
				break;
			}
		}

		// get RGBIR ISO enh_ratio
		rgbir_enh_m = &iq_param->rgbir_enh->manual_param;
		rgbir_enh_d = &iq_param->rgbir_enh->auto_param[IQ_RGBIR_LIGHT_D];
		rgbir_enh_a = &iq_param->rgbir_enh->auto_param[IQ_RGBIR_LIGHT_A];
		rgbir_enh_ir = &iq_param->rgbir_enh->auto_param[IQ_RGBIR_LIGHT_IR];

		if ((iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGBIR) && (iq_param->rgbir_enh->enable == TRUE)) {
			if (iq_param->rgbir_enh->mode == IQ_OP_TYPE_AUTO) {
				ir_d_light_enh_ratio = iq_intpl(iq_sync_info.gain, rgbir_enh_d->enh_ratio[iso_idx_l], rgbir_enh_d->enh_ratio[iso_idx_h], iso_start, iso_end);
				ir_a_light_enh_ratio = iq_intpl(iq_sync_info.gain, rgbir_enh_a->enh_ratio[iso_idx_l], rgbir_enh_a->enh_ratio[iso_idx_h], iso_start, iso_end);
				ir_ir_light_enh_ratio = iq_intpl(iq_sync_info.gain, rgbir_enh_ir->enh_ratio[iso_idx_l], rgbir_enh_ir->enh_ratio[iso_idx_h], iso_start, iso_end);
				if (iq_info->ir_info.ir_level <= iq_param->rgbir_enh->min_ir_th) {
					ir_final_enh_ratio = IQ_RGBIR_ENH_RATIO_BASE;
					PRINT_IQ(dbg_iso_en, "ir_level            = %d (N: %d) \r\n", iq_info->ir_info.ir_level, iq_param->rgbir_enh->min_ir_th);
					PRINT_IQ(dbg_iso_en, "ir_final_enh_ratio  = %d (N: %d) \r\n", ir_final_enh_ratio, IQ_RGBIR_ENH_RATIO_BASE);
				} else if (iq_info->ir_info.ir_level <= rgbir_enh_d->ir_th) {
					ir_final_enh_ratio = iq_intpl(iq_info->ir_info.ir_level, IQ_RGBIR_ENH_RATIO_BASE, ir_d_light_enh_ratio, iq_param->rgbir_enh->min_ir_th, rgbir_enh_d->ir_th);
					PRINT_IQ(dbg_iso_en, "ir_level            = %d (N: %d, D: %d) \r\n", iq_info->ir_info.ir_level, iq_param->rgbir_enh->min_ir_th, rgbir_enh_d->ir_th);
					PRINT_IQ(dbg_iso_en, "ir_final_enh_ratio  = %d (N: %d, D: %d) \r\n", ir_final_enh_ratio, IQ_RGBIR_ENH_RATIO_BASE, ir_d_light_enh_ratio);
				} else if (iq_info->ir_info.ir_level <= rgbir_enh_a->ir_th) {
					ir_final_enh_ratio = iq_intpl(iq_info->ir_info.ir_level, ir_d_light_enh_ratio, ir_a_light_enh_ratio, rgbir_enh_d->ir_th, rgbir_enh_a->ir_th);
					PRINT_IQ(dbg_iso_en, "ir_level            = %d (D: %d, A: %d) \r\n", iq_info->ir_info.ir_level, rgbir_enh_d->ir_th, rgbir_enh_a->ir_th);
					PRINT_IQ(dbg_iso_en, "ir_final_enh_ratio  = %d (D: %d, A: %d) \r\n", ir_final_enh_ratio, ir_d_light_enh_ratio, ir_a_light_enh_ratio);
				} else{
					ir_final_enh_ratio = iq_intpl(iq_info->ir_info.ir_level, ir_a_light_enh_ratio, ir_ir_light_enh_ratio, rgbir_enh_a->ir_th, rgbir_enh_ir->ir_th);
					PRINT_IQ(dbg_iso_en, "ir_level            = %d (A: %d, I: %d) \r\n", iq_info->ir_info.ir_level, rgbir_enh_a->ir_th, rgbir_enh_ir->ir_th);
					PRINT_IQ(dbg_iso_en, "ir_final_enh_ratio  = %d (A: %d, I: %d) \r\n", ir_final_enh_ratio, ir_a_light_enh_ratio, ir_ir_light_enh_ratio);
				}
			} else {
				ir_final_enh_ratio = iq_intpl(iq_sync_info.gain, rgbir_enh_m->enh_ratio[iso_idx_l], rgbir_enh_m->enh_ratio[iso_idx_h], iso_start, iso_end);
			}
		} else {
			ir_final_enh_ratio = 16;
		}
		PRINT_IQ_VAR(dbg_iso_en, ir_final_enh_ratio);

		// get WDR ISO enh_ratio
		wdr_m = &iq_param->wdr->manual_param;
		wdr_l = &iq_param->wdr->auto_param[iso_idx_l];
		wdr_h = &iq_param->wdr->auto_param[iso_idx_h];

		// ++ get first WDR strength for fastboot and capture
		if ((iq_info->ipp_trig_obj.reset == TRUE) && (iq_info->ipp_trig_obj.ipp_capture == TRUE)) {
			if (iq_param->wdr->mode == IQ_OP_TYPE_AUTO) {
				curr_wdr_set.auto_set.level = iq_intpl(iq_sync_info.gain, wdr_l->level, wdr_h->level, iso_start, iso_end);
				curr_wdr_set.auto_set.strength_min = iq_intpl(iq_sync_info.gain, wdr_l->strength_min, wdr_h->strength_min, iso_start, iso_end);
				curr_wdr_set.auto_set.strength_max = iq_intpl(iq_sync_info.gain, wdr_l->strength_max, wdr_h->strength_max, iso_start, iso_end);
				iq_operation_wdr(iq_info, iq_param, &curr_wdr_set);
			} else {
				iq_info->target_wdr_str = wdr_m->strength;
			}
			ife_wdr->wdr_str.strength = iq_info->target_wdr_str;
		}
		if ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_WDR) && (iq_param->wdr->enable == TRUE) && (iq_param->wdr_enh->enable == TRUE)) {
			wdr_idx_l = ife_wdr->wdr_str.strength >> 4;
			wdr_idx_l = IQ_CLAMP(wdr_idx_l, 0, IQ_WDR_STR_LEN - 1);
			wdr_idx_h = IQ_CLAMP(wdr_idx_l + 1, 0, IQ_WDR_STR_LEN - 1);
			wdr_final_enh_ratio = iq_intpl(ife_wdr->wdr_str.strength, iq_param->wdr_enh->enh_ratio[wdr_idx_l], iq_param->wdr_enh->enh_ratio[wdr_idx_h], wdr_idx_l << 4, wdr_idx_h << 4);
			PRINT_IQ(dbg_iso_en, "wdr_strength        = %d \r\n", ife_wdr->wdr_str.strength);
			PRINT_IQ(dbg_iso_en, "wdr_final_enh_ratio = %d (L_%d: %d, H_%d: %d) \r\n", wdr_final_enh_ratio, wdr_idx_l, iq_param->wdr_enh->enh_ratio[wdr_idx_l], wdr_idx_h, iq_param->wdr_enh->enh_ratio[wdr_idx_h]);
		} else {
			wdr_final_enh_ratio = 128;
		}
		PRINT_IQ_VAR(dbg_iso_en, wdr_final_enh_ratio);

		// get enhanced ISO
		iq_info->final_ipp.enh_gain = IQ_CLAMP(iq_sync_info.gain * ir_final_enh_ratio >> 4, iso_map_tab[0].iso, iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso);
		iq_info->final_ipp.enh_gain = IQ_CLAMP(iq_info->final_ipp.enh_gain * wdr_final_enh_ratio >> 7, iso_map_tab[0].iso, iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso);
		PRINT_IQ_VAR(dbg_iso_en, iq_info->final_ipp.enh_gain);

		if (iq_info->final_ipp.enh_gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
			iso_idx_l_enh = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
			iso_idx_h_enh = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
			iso_start_enh = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso;
			iso_end_enh = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso;
		}
		for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
			if (iq_info->final_ipp.enh_gain < iso_map_tab[i].iso) {
				iso_idx_l_enh = iso_map_tab[i - 1].index;
				iso_idx_h_enh = iso_map_tab[i].index;
				iso_start_enh = iso_map_tab[i - 1].iso;
				iso_end_enh = iso_map_tab[i].iso;
				break;
			}
		}

		ob_m = &iq_param->ob->manual_param;
		ob_l = &iq_param->ob->auto_param[iso_idx_l];
		ob_h = &iq_param->ob->auto_param[iso_idx_h];
		nr_m = &iq_param->nr->manual_param;
		nr_l = &iq_param->nr->auto_param[iso_idx_l];
		nr_h = &iq_param->nr->auto_param[iso_idx_h];
		nr_ext_m = &iq_param->nr->ext_manual_param;
		nr_ext_l = &iq_param->nr->ext_auto_param[iso_idx_l];
		nr_ext_h = &iq_param->nr->ext_auto_param[iso_idx_h];
		nr_lca_m = &iq_param->nr->manual_param;
		nr_lca_l = &iq_param->nr->auto_param[iso_idx_l_enh];
		nr_lca_h = &iq_param->nr->auto_param[iso_idx_h_enh];
		cfa_m = &iq_param->cfa->manual_param;
		cfa_l = &iq_param->cfa->auto_param[iso_idx_l];
		cfa_h = &iq_param->cfa->auto_param[iso_idx_h];
		raw_va_m = &iq_param->raw_va->manual_param;
		raw_va_l = &iq_param->raw_va->auto_param[iso_idx_l];
		raw_va_h = &iq_param->raw_va->auto_param[iso_idx_h];
		color_m = &iq_param->color->manual_param;
		color_l = &iq_param->color->auto_param[iso_idx_l];
		color_h = &iq_param->color->auto_param[iso_idx_h];
		contrast_m = &iq_param->contrast->manual_param;
		contrast_l = &iq_param->contrast->auto_param[iso_idx_l];
		contrast_h = &iq_param->contrast->auto_param[iso_idx_h];
		edge_m = &iq_param->edge->manual_param;
		edge_l = &iq_param->edge->auto_param[iso_idx_l_enh];
		edge_h = &iq_param->edge->auto_param[iso_idx_h_enh];
		edge_ext_m = &iq_param->edge->ext_manual_param;
		edge_ext_l = &iq_param->edge->ext_auto_param[iso_idx_l_enh];
		edge_ext_h = &iq_param->edge->ext_auto_param[iso_idx_h_enh];
		_3dnr_m = &iq_param->_3dnr->manual_param;
		_3dnr_l = &iq_param->_3dnr->auto_param[iso_idx_l_enh];
		_3dnr_h = &iq_param->_3dnr->auto_param[iso_idx_h_enh];
		_3dnr_ext_m = &iq_param->_3dnr->ext_manual_param;
		_3dnr_ext_l = &iq_param->_3dnr->ext_auto_param[iso_idx_l_enh];
		_3dnr_ext_h = &iq_param->_3dnr->ext_auto_param[iso_idx_h_enh];
		post_sharpen_1_m = &iq_param->post_sharpen_1->manual_param;
		post_sharpen_1_l = &iq_param->post_sharpen_1->auto_param[iso_idx_l_enh];
		post_sharpen_1_h = &iq_param->post_sharpen_1->auto_param[iso_idx_h_enh];
		pfr_m = &iq_param->pfr->manual_param;
		pfr_l = &iq_param->pfr->auto_param[iso_idx_l];
		pfr_h = &iq_param->pfr->auto_param[iso_idx_h];
		defog_m = &iq_param->defog->manual_param;
		defog_l = &iq_param->defog->auto_param[iso_idx_l];
		defog_h = &iq_param->defog->auto_param[iso_idx_h];
		shdr_m = &iq_param->shdr->manual_param;
		shdr_l = &iq_param->shdr->auto_param[iso_idx_l];
		shdr_h = &iq_param->shdr->auto_param[iso_idx_h];
		fpn_m = &iq_param->fpn->manual_param;
		fpn_l = &iq_param->fpn->auto_param[iso_idx_l];
		fpn_h = &iq_param->fpn->auto_param[iso_idx_h];
		cst_m = &iq_param->cst->manual_param;
		cst_l = &iq_param->cst->auto_param[iso_idx_l];
		cst_h = &iq_param->cst->auto_param[iso_idx_h];
		bnr_m = &iq_param->bnr->manual_param;
		bnr_l = &iq_param->bnr->auto_param[iso_idx_l_enh];
		bnr_h = &iq_param->bnr->auto_param[iso_idx_h_enh];
		aiisp_m = &iq_param->aiisp->manual_param;
		aiisp_l = &iq_param->aiisp->auto_param[iso_idx_l_enh];
		aiisp_h = &iq_param->aiisp->auto_param[iso_idx_h_enh];

		// IQ_FPN_PARAM setting
		pre_fpn->fpn_en = iq_param->fpn->enable;
		if ((iq_param->fpn->mode == IQ_OP_TYPE_AUTO) && (fpn_l->buf_phyaddr != 0)) {
			pre_fpn_info->buf_phyaddr = fpn_l->buf_phyaddr;
			pre_fpn_info->buf_size = fpn_l->buf_size;
			if (fpn_h->buf_phyaddr == fpn_l->buf_phyaddr) {
				curr_fpn_gain = iq_intpl(iq_sync_info.gain, fpn_l->gain, fpn_h->gain, iso_start, iso_end);
			} else {
				curr_fpn_gain = iq_intpl(iq_sync_info.gain, fpn_l->gain, fpn_l->gain * 2, iso_start, iso_end);
			}
		} else if ((iq_param->fpn->mode == IQ_OP_TYPE_MANUAL) && (fpn_m->buf_phyaddr != 0)) {
			pre_fpn_info->buf_phyaddr = fpn_m->buf_phyaddr;
			pre_fpn_info->buf_size = fpn_m->buf_size;
			curr_fpn_gain = fpn_m->gain;
		} else {
			pre_fpn->fpn_en = FALSE;
			curr_fpn_gain = 256;
		}
		pre_fpn->fpn_cgain_r[0] = curr_fpn_gain;
		pre_fpn->fpn_cgain_gr[0] = curr_fpn_gain;
		pre_fpn->fpn_cgain_gb[0] = curr_fpn_gain;
		pre_fpn->fpn_cgain_b[0] = curr_fpn_gain;
		pre_fpn->fpn_cgain_ir[0] = curr_fpn_gain;
		pre_fpn->fpn_cofs_r[0] = 0;
		pre_fpn->fpn_cofs_gr[0] = 0;
		pre_fpn->fpn_cofs_gb[0] = 0;
		pre_fpn->fpn_cofs_b[0] = 0;
		pre_fpn->fpn_cofs_ir[0] = 0;
		iq_sync_info.cgain[0] = IQ_CLAMP(iq_sync_info.cgain[0], 0, 2047);
		iq_sync_info.cgain[1] = IQ_CLAMP(iq_sync_info.cgain[1], 0, 2047);
		iq_sync_info.cgain[2] = IQ_CLAMP(iq_sync_info.cgain[2], 0, 2047);

		if ((iq_info->cg_mode == IQ_CG_PRE) && (iq_info->dg_mode == IQ_DG_PRE)) {
			PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "IQ(%d) cg_mode and dg_mode are at the same HW(PRE) \r\n", iq_info->id);
		} else if ((iq_info->cg_mode == IQ_CG_IPE) && (iq_info->dg_mode == IQ_DG_IPE)) {
			PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "IQ(%d) cg_mode and dg_mode are at the same HW(IPE) \r\n", iq_info->id);
		}

		// AWB_CGain operation
		// AE_DGain operation
		if ((iq_sync_info.cgain[0] != 0) && (iq_sync_info.cgain[1] != 0) && (iq_sync_info.cgain[2] != 0) && (iq_sync_info.dgain != 0)) {
			if (iq_info->cg_mode == IQ_CG_PRE_F) {
				if (iq_sync_info.cgain[0] > 1023 || iq_sync_info.cgain[1] > 1023 || iq_sync_info.cgain[2] > 1023) {
					pre_fusion->fu_cgain.bit_field = CTL_PRE_ISP__9_7;
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_R]  = IQ_CLAMP(iq_sync_info.cgain[0] >> 1, 0, 65536);
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_GR] = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 65536);
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_GB] = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 65536);
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_B]  = IQ_CLAMP(iq_sync_info.cgain[2] >> 1, 0, 65536);
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_IR] = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_R]  = IQ_CLAMP(iq_sync_info.cgain[0] >> 1, 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_GR] = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_GB] = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_B]  = IQ_CLAMP(iq_sync_info.cgain[2] >> 1, 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_IR] = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 65536);
				} else {
					pre_fusion->fu_cgain.bit_field = CTL_PRE_ISP__8_8;
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_R]  = IQ_CLAMP(iq_sync_info.cgain[0], 0, 65536);
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_GR] = IQ_CLAMP(iq_sync_info.cgain[1], 0, 65536);
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_GB] = IQ_CLAMP(iq_sync_info.cgain[1], 0, 65536);
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_B]  = IQ_CLAMP(iq_sync_info.cgain[2], 0, 65536);
					pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_IR] = IQ_CLAMP(iq_sync_info.cgain[1], 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_R]  = IQ_CLAMP(iq_sync_info.cgain[0], 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_GR] = IQ_CLAMP(iq_sync_info.cgain[1], 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_GB] = IQ_CLAMP(iq_sync_info.cgain[1], 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_B]  = IQ_CLAMP(iq_sync_info.cgain[2], 0, 65536);
					pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_IR] = IQ_CLAMP(iq_sync_info.cgain[1], 0, 65536);
				}
			} else {
				pre_fusion->fu_cgain.bit_field = CTL_PRE_ISP__8_8;
				pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_R]  = 256;
				pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_GR] = 256;
				pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_GB] = 256;
				pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_B]  = 256;
				pre_fusion->fu_cgain.fcgain_s[CTL_PRE_ISP_CGAIN_CH_IR] = 256;
				pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_R]  = 256;
				pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_GR] = 256;
				pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_GB] = 256;
				pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_B]  = 256;
				pre_fusion->fu_cgain.fcgain_l[CTL_PRE_ISP_CGAIN_CH_IR] = 256;
			}

			if (iq_info->dg_mode_manual.pre_enable) {
				pre_cgain->bit_field = CTL_PRE_ISP__3_7;
				pre_cgain->cgain_r  = IQ_CLAMP(iq_info->dg_mode_manual.pre_value, 0, 1023);
				pre_cgain->cgain_gr = IQ_CLAMP(iq_info->dg_mode_manual.pre_value, 0, 1023);
				pre_cgain->cgain_gb = IQ_CLAMP(iq_info->dg_mode_manual.pre_value, 0, 1023);
				pre_cgain->cgain_b  = IQ_CLAMP(iq_info->dg_mode_manual.pre_value, 0, 1023);
				pre_cgain->cgain_ir = IQ_CLAMP(iq_info->dg_mode_manual.pre_value, 0, 1023);
			} else {
				pre_cgain->bit_field = CTL_PRE_ISP__3_7;

				if (iq_info->cg_mode == IQ_CG_PRE) {
					pre_cgain->cgain_r  = IQ_CLAMP(iq_sync_info.cgain[0] >> 1, 0, 1023);
					pre_cgain->cgain_gr = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 1023);
					pre_cgain->cgain_gb = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 1023);
					pre_cgain->cgain_b  = IQ_CLAMP(iq_sync_info.cgain[2] >> 1, 0, 1023);
					pre_cgain->cgain_ir = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 1023);
				} else if (iq_info->dg_mode == IQ_DG_PRE) {
					pre_cgain->cgain_r  = IQ_CLAMP(iq_sync_info.dgain, 0, 1023);
					pre_cgain->cgain_gr = IQ_CLAMP(iq_sync_info.dgain, 0, 1023);
					pre_cgain->cgain_gb = IQ_CLAMP(iq_sync_info.dgain, 0, 1023);
					pre_cgain->cgain_b  = IQ_CLAMP(iq_sync_info.dgain, 0, 1023);
					pre_cgain->cgain_ir = IQ_CLAMP(iq_sync_info.dgain, 0, 1023);
				} else {
					pre_cgain->cgain_r  = 128;
					pre_cgain->cgain_gr = 128;
					pre_cgain->cgain_gb = 128;
					pre_cgain->cgain_b  = 128;
					pre_cgain->cgain_ir = 128;
				}

				if ((iq_info->dg_mode == IQ_DG_AI) && !iq_info->dg_ai_enable) {
					pre_cgain->cgain_r  = IQ_CLAMP((pre_cgain->cgain_r * iq_info->dg_ai) >> 7, 0, 1023);
					pre_cgain->cgain_gr = IQ_CLAMP((pre_cgain->cgain_gr * iq_info->dg_ai) >> 7, 0, 1023);
					pre_cgain->cgain_gb = IQ_CLAMP((pre_cgain->cgain_gb * iq_info->dg_ai) >> 7, 0, 1023);
					pre_cgain->cgain_b  = IQ_CLAMP((pre_cgain->cgain_b * iq_info->dg_ai) >> 7, 0, 1023);
					pre_cgain->cgain_ir = IQ_CLAMP((pre_cgain->cgain_ir * iq_info->dg_ai) >> 7, 0, 1023);
				}
			}

			if (iq_info->dg_mode_manual.ife_enable) {
				ife_dgain->dgain = IQ_CLAMP(iq_info->dg_mode_manual.ife_value << 1, 0, 65535);
			} else {
				if (iq_info->dg_mode == IQ_DG_IFE) {
					ife_dgain->dgain = IQ_CLAMP(iq_sync_info.dgain << 1, 0, 65535);
				} else {
					ife_dgain->dgain = 256;
				}

				if ((iq_info->dg_mode == IQ_DG_AI) && iq_info->dg_ai_enable) {
					ife_dgain->dgain = IQ_CLAMP((ife_dgain->dgain * iq_info->dg_ai2) >> 7, 0, 65535);
				}
			}

			if (iq_info->cg_mode == IQ_CG_IFE) {
				if (iq_sync_info.cgain[0] > 1023 || iq_sync_info.cgain[1] > 1023 || iq_sync_info.cgain[2] > 1023) {
					ife_cgain->bit_field = CTL_IFE_ISP__3_7;
					ife_cgain->cgain_r  = IQ_CLAMP(iq_sync_info.cgain[0] >> 1, 0, 1023);
					ife_cgain->cgain_gr = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 1023);
					ife_cgain->cgain_gb = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 1023);
					ife_cgain->cgain_b  = IQ_CLAMP(iq_sync_info.cgain[2] >> 1, 0, 1023);
					ife_cgain->cgain_ir = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 1023);
				} else {
					ife_cgain->bit_field = CTL_IFE_ISP__2_8;
					ife_cgain->cgain_r  = IQ_CLAMP(iq_sync_info.cgain[0], 0, 1023);
					ife_cgain->cgain_gr = IQ_CLAMP(iq_sync_info.cgain[1], 0, 1023);
					ife_cgain->cgain_gb = IQ_CLAMP(iq_sync_info.cgain[1], 0, 1023);
					ife_cgain->cgain_b  = IQ_CLAMP(iq_sync_info.cgain[2], 0, 1023);
					ife_cgain->cgain_ir = IQ_CLAMP(iq_sync_info.cgain[1], 0, 1023);
				}
			} else {
				ife_cgain->bit_field = CTL_IFE_ISP__3_7;
				ife_cgain->cgain_r  = 128;
				ife_cgain->cgain_gr = 128;
				ife_cgain->cgain_gb = 128;
				ife_cgain->cgain_b  = 128;
				ife_cgain->cgain_ir = 128;
			}

			if (iq_info->cg_mode == IQ_CG_IPE) {
				if (iq_sync_info.cgain[0] > 1023 || iq_sync_info.cgain[1] > 1023 || iq_sync_info.cgain[2] > 1023) {
					ipe_cfa->cfa_cgain.r_gain = IQ_CLAMP(iq_sync_info.cgain[0] >> 1, 0, 1023);
					ipe_cfa->cfa_cgain.g_gain = IQ_CLAMP(iq_sync_info.cgain[1] >> 1, 0, 1023);
					ipe_cfa->cfa_cgain.b_gain = IQ_CLAMP(iq_sync_info.cgain[2] >> 1, 0, 1023);
					ipe_cfa->cfa_cgain.gain_range = CTL_IPE_ISP_CGAIN_3_7;
				} else {
					ipe_cfa->cfa_cgain.r_gain = IQ_CLAMP(iq_sync_info.cgain[0], 0, 1023);
					ipe_cfa->cfa_cgain.g_gain = IQ_CLAMP(iq_sync_info.cgain[1], 0, 1023);
					ipe_cfa->cfa_cgain.b_gain = IQ_CLAMP(iq_sync_info.cgain[2], 0, 1023);
					ipe_cfa->cfa_cgain.gain_range = CTL_IPE_ISP_CGAIN_2_8;
				}
			} else if (iq_info->dg_mode == IQ_DG_IPE) {
				ipe_cfa->cfa_cgain.gain_range = CTL_IPE_ISP_CGAIN_3_7;
				ipe_cfa->cfa_cgain.r_gain = IQ_CLAMP(iq_sync_info.dgain, 0, 1023);
				ipe_cfa->cfa_cgain.g_gain = IQ_CLAMP(iq_sync_info.dgain, 0, 1023);
				ipe_cfa->cfa_cgain.b_gain = IQ_CLAMP(iq_sync_info.dgain, 0, 1023);
			} else {
				ipe_cfa->cfa_cgain.gain_range = CTL_IPE_ISP_CGAIN_3_7;
				ipe_cfa->cfa_cgain.r_gain = 128;
				ipe_cfa->cfa_cgain.g_gain = 128;
				ipe_cfa->cfa_cgain.b_gain = 128;
			}
		} else {
			PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "IQ(%d) IPP_Gain error setting IPP_CGain(%d, %d, %d), IPP_DGain(%d)\r\n"
				, iq_info->id, iq_sync_info.cgain[0], iq_sync_info.cgain[1], iq_sync_info.cgain[2], iq_sync_info.dgain);
		}

		// IQ_OB_PARAM setting
		if (iq_param->ob->mode == IQ_OP_TYPE_AUTO) {
			iq_intpl_tbl_uint32(ob_l->cofs, ob_h->cofs, IQ_OB_LEN, curr_ob, iq_sync_info.gain, iso_start, iso_end);
		} else {
			memcpy(curr_ob, ob_m->cofs, sizeof(UINT32) * IQ_OB_LEN);
		}
		if (iq_param->ob->enable == FALSE) {
			memset(curr_ob, 0, sizeof(UINT32) * IQ_OB_LEN);
		}

		if (iq_info->ob_mode_manual.pre_f_enable) {
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_R] = iq_info->ob_mode_manual.pre_f_value[0];
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GR] = iq_info->ob_mode_manual.pre_f_value[1];
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GB] = iq_info->ob_mode_manual.pre_f_value[2];
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_B] = iq_info->ob_mode_manual.pre_f_value[3];
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_IR] = iq_info->ob_mode_manual.pre_f_value[4];
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_R] = iq_info->ob_mode_manual.pre_f_value[0];
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GR] = iq_info->ob_mode_manual.pre_f_value[1];
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GB] = iq_info->ob_mode_manual.pre_f_value[2];
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_B] = iq_info->ob_mode_manual.pre_f_value[3];
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_IR] = iq_info->ob_mode_manual.pre_f_value[4];
		} else if (iq_info->ob_mode == IQ_OB_PRE_F) {
			if (pre_fpn->fpn_en == FALSE) {
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_R] = curr_ob[0];
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GR] = curr_ob[1];
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GB] = curr_ob[2];
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_B] = curr_ob[3];
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_IR] = curr_ob[4];
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_R] = curr_ob[0];
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GR] = curr_ob[1];
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GB] = curr_ob[2];
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_B] = curr_ob[3];
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_IR] = curr_ob[4];
			} else {
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_R] = (curr_ob[0] * (256 - curr_fpn_gain)) >> 8;
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GR] = (curr_ob[1] * (256 - curr_fpn_gain)) >> 8;
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GB] = (curr_ob[2] * (256 - curr_fpn_gain)) >> 8;
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_B] = (curr_ob[3] * (256 - curr_fpn_gain)) >> 8;
				pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_IR] = (curr_ob[4] * (256 - curr_fpn_gain)) >> 8;
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_R] = pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_R];
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GR] = pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GR];
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GB] = pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GB];
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_B] = pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_B];
				pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_IR] = pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_IR];
			}
		} else {
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_R] = 0;
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GR] = 0;
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GB] = 0;
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_B] = 0;
			pre_fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_IR] = 0;
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_R] = 0;
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GR] = 0;
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GB] = 0;
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_B] = 0;
			pre_fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_IR] = 0;
		}

		if (iq_info->ob_mode_manual.pre_enable) {
			pre_cgain->cofs_r = iq_info->ob_mode_manual.pre_value[0];
			pre_cgain->cofs_gr = iq_info->ob_mode_manual.pre_value[1];
			pre_cgain->cofs_gb = iq_info->ob_mode_manual.pre_value[2];
			pre_cgain->cofs_b = iq_info->ob_mode_manual.pre_value[3];
			pre_cgain->cofs_ir = iq_info->ob_mode_manual.pre_value[4];
		} else if (iq_info->ob_mode == IQ_OB_PRE) {
			if (pre_fpn->fpn_en == FALSE) {
				pre_cgain->cofs_r = curr_ob[0];
				pre_cgain->cofs_gr = curr_ob[1];
				pre_cgain->cofs_gb = curr_ob[2];
				pre_cgain->cofs_b = curr_ob[3];
				pre_cgain->cofs_ir = curr_ob[4];
			} else {
				pre_cgain->cofs_r = (curr_ob[0] * (256 - curr_fpn_gain)) >> 8;
				pre_cgain->cofs_gr = (curr_ob[1] * (256 - curr_fpn_gain)) >> 8;
				pre_cgain->cofs_gb = (curr_ob[2] * (256 - curr_fpn_gain)) >> 8;
				pre_cgain->cofs_b = (curr_ob[3] * (256 - curr_fpn_gain)) >> 8;
				pre_cgain->cofs_ir = (curr_ob[4] * (256 - curr_fpn_gain)) >> 8;
			}
		} else if (iq_info->ob_mode == IQ_OB_AI) {
			pre_cgain->cofs_r = iq_info->ob_ai;
			pre_cgain->cofs_gr = iq_info->ob_ai;
			pre_cgain->cofs_gb = iq_info->ob_ai;
			pre_cgain->cofs_b = iq_info->ob_ai;
			pre_cgain->cofs_ir = iq_info->ob_ai;
		} else {
			pre_cgain->cofs_r = 0;
			pre_cgain->cofs_gr = 0;
			pre_cgain->cofs_gb = 0;
			pre_cgain->cofs_b = 0;
			pre_cgain->cofs_ir = 0;
		}

		if (iq_info->ob_mode_manual.ife_enable) {
			ife_cgain->cofs_r = iq_info->ob_mode_manual.ife_value[0];
			ife_cgain->cofs_gr = iq_info->ob_mode_manual.ife_value[1];
			ife_cgain->cofs_gb = iq_info->ob_mode_manual.ife_value[2];
			ife_cgain->cofs_b = iq_info->ob_mode_manual.ife_value[3];
			ife_cgain->cofs_ir = iq_info->ob_mode_manual.ife_value[4];
		} else if (iq_info->ob_mode == IQ_OB_IFE) { //((ife_fpn->fpn_en == FALSE) && (iq_info->ob_mode == IQ_OB_IFE)) {
			if (pre_fpn->fpn_en == FALSE) {
				ife_cgain->cofs_r = curr_ob[0];
				ife_cgain->cofs_gr = curr_ob[1];
				ife_cgain->cofs_gb = curr_ob[2];
				ife_cgain->cofs_b = curr_ob[3];
				ife_cgain->cofs_ir = curr_ob[4];
			} else {
				ife_cgain->cofs_r = (curr_ob[0] * (256 - curr_fpn_gain)) >> 8;
				ife_cgain->cofs_gr = (curr_ob[1] * (256 - curr_fpn_gain)) >> 8;
				ife_cgain->cofs_gb = (curr_ob[2] * (256 - curr_fpn_gain)) >> 8;
				ife_cgain->cofs_b = (curr_ob[3] * (256 - curr_fpn_gain)) >> 8;
				ife_cgain->cofs_ir = (curr_ob[4] * (256 - curr_fpn_gain)) >> 8;
			}
		} else {
			ife_cgain->cofs_r = 0;
			ife_cgain->cofs_gr = 0;
			ife_cgain->cofs_gb = 0;
			ife_cgain->cofs_b = 0;
			ife_cgain->cofs_ir = 0;
		}

		// IQ_BNR_PARAM setting
		pre_bnr->bnr_enable = iq_param->bnr->enable;
		pre_bnr->bnr_err_comp.err_comp_en = iq_param->bnr->err_comp_en;
		if (iq_param->bnr->mode == IQ_OP_TYPE_AUTO) {
			pre_bnr->bnr_prefilter_str = iq_intpl(iq_sync_info.gain, bnr_l->prefilter_str, bnr_h->prefilter_str, iso_start, iso_end);

			iq_intpl_tbl_uint16(bnr_l->err_comp_l, bnr_h->err_comp_l, CTL_PRE_ISP_BNR_ERR_COMP_L_MAX, pre_bnr->bnr_err_comp.err_comp_l, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(bnr_l->err_comp_r, bnr_h->err_comp_r, CTL_PRE_ISP_BNR_ERR_COMP_R_MAX, pre_bnr->bnr_err_comp.err_comp_r, iq_sync_info.gain, iso_start, iso_end);
			pre_bnr->bnr_err_comp.err_sft = iq_intpl(iq_sync_info.gain, bnr_l->err_sft, bnr_h->err_sft, iso_start, iso_end);

			pre_bnr->bnr_np_coef.coef_a = iq_intpl(iq_sync_info.gain, bnr_l->coef_a, bnr_h->coef_a, iso_start, iso_end);
			pre_bnr->bnr_np_coef.coef_b = iq_intpl(iq_sync_info.gain, bnr_l->coef_b, bnr_h->coef_b, iso_start, iso_end);

			pre_bnr->ds_th.downsample_th1 = iq_intpl(iq_sync_info.gain, bnr_l->downsample_th1, bnr_h->downsample_th1, iso_start, iso_end);
			pre_bnr->ds_th.downsample_th2 = iq_intpl(iq_sync_info.gain, bnr_l->downsample_th2, bnr_h->downsample_th2, iso_start, iso_end);

			pre_bnr->fusion_str.static_region = iq_intpl(iq_sync_info.gain, bnr_l->static_region, bnr_h->static_region, iso_start, iso_end);
			pre_bnr->fusion_str.transision_region = iq_intpl(iq_sync_info.gain, bnr_l->transision_region, bnr_h->transision_region, iso_start, iso_end);
			pre_bnr->fusion_str.motion_region = iq_intpl(iq_sync_info.gain, bnr_l->motion_region, bnr_h->motion_region, iso_start, iso_end);

			pre_bnr->residue_th = iq_intpl(iq_sync_info.gain, bnr_l->residue_th, bnr_h->residue_th, iso_start, iso_end);

			iq_intpl_tbl_uint16(bnr_l->md_th_l, bnr_h->md_th_l, CTL_PRE_ISP_BNR_MD_TH_L, pre_bnr->md.md_th_l, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(bnr_l->md_th_r, bnr_h->md_th_r, CTL_PRE_ISP_BNR_MD_TH_R, pre_bnr->md.md_th_r, iq_sync_info.gain, iso_start, iso_end);
			pre_bnr->md.md_base = iq_intpl(iq_sync_info.gain, bnr_l->md_base, bnr_h->md_base, iso_start, iso_end);;
			pre_bnr->md.md_k1 = iq_intpl(iq_sync_info.gain, bnr_l->md_k1, bnr_h->md_k1, iso_start, iso_end);;
			pre_bnr->md.md_k2 = iq_intpl(iq_sync_info.gain, bnr_l->md_k2, bnr_h->md_k2, iso_start, iso_end);;
		} else {
			pre_bnr->bnr_prefilter_str = bnr_m->prefilter_str;

			memcpy(pre_bnr->bnr_err_comp.err_comp_l, bnr_m->err_comp_l, sizeof(UINT16) * CTL_PRE_ISP_BNR_ERR_COMP_L_MAX);
			memcpy(pre_bnr->bnr_err_comp.err_comp_r, bnr_m->err_comp_r, sizeof(UINT16) * CTL_PRE_ISP_BNR_ERR_COMP_R_MAX);
			pre_bnr->bnr_err_comp.err_sft = bnr_m->err_sft;

			pre_bnr->bnr_np_coef.coef_a = bnr_m->coef_a;
			pre_bnr->bnr_np_coef.coef_b = bnr_m->coef_b;

			pre_bnr->ds_th.downsample_th1 = bnr_m->downsample_th1;
			pre_bnr->ds_th.downsample_th2 = bnr_m->downsample_th2;

			pre_bnr->fusion_str.static_region = bnr_m->static_region;
			pre_bnr->fusion_str.transision_region = bnr_m->transision_region;
			pre_bnr->fusion_str.motion_region = bnr_m->motion_region;

			pre_bnr->residue_th = bnr_m->residue_th;

			memcpy(pre_bnr->md.md_th_l, bnr_m->md_th_l, sizeof(UINT16) * CTL_PRE_ISP_BNR_MD_TH_L);
			memcpy(pre_bnr->md.md_th_r, bnr_m->md_th_r, sizeof(UINT16) * CTL_PRE_ISP_BNR_MD_TH_R);
			pre_bnr->md.md_base = bnr_m->md_base;
			pre_bnr->md.md_k1 = bnr_m->md_k1;
			pre_bnr->md.md_k2 = bnr_m->md_k2;
		}
		// inverse region
		pre_bnr->fusion_str.static_region = 255 - IQ_CLAMP(pre_bnr->fusion_str.static_region, 0, 255);
		pre_bnr->fusion_str.transision_region = 255 - IQ_CLAMP(pre_bnr->fusion_str.transision_region, 0, 255);
		pre_bnr->fusion_str.motion_region = 255 - IQ_CLAMP(pre_bnr->fusion_str.motion_region, 0, 255);

		// AIISP setting
		aiisp->version = iq_param->aiisp->version;
		aiisp->path_id = iq_param->aiisp->path_id;
		aiisp->param_num = iq_param->aiisp->param_num;
		memcpy(aiisp->param_size, iq_param->aiisp->param_size, sizeof(UINT32) * CTL_IPP_ISP_AIISP_PARAM_MAX);
		if (iq_param->aiisp->mode == IQ_OP_TYPE_AUTO) {
			aiisp->enable = aiisp_l->enable;
			aiisp->effect = aiisp_l->effect;
			iq_intpl_tbl_uint32(aiisp_l->param, aiisp_h->param, CTL_IPP_ISP_AIISP_PARAM_MAX, iq_info->aiisp_final_param, iq_sync_info.gain, iso_start, iso_end);
		} else {
			aiisp->enable = aiisp_m->enable;
			aiisp->effect = aiisp_m->effect;
			memcpy(iq_info->aiisp_final_param, aiisp_m->param, sizeof(UINT32) * CTL_IPP_ISP_AIISP_PARAM_MAX);
		}
		for (i = 0; i < aiisp->param_num; i++) {
			aiisp->param_phyaddr[i] = (ULONG)vos_cpu_get_phy_addr((VOS_ADDR)&iq_info->aiisp_final_param[i]);
		}

		if ((iq_info->dg_mode == IQ_DG_AI) && iq_info->dg_ai_enable) {
			iq_info->aiisp_reserved_param.dgain = (iq_info->dg_ai >> 3);
		} else {
			iq_info->aiisp_reserved_param.dgain = 16;
		}
		aiisp->reserved_param_size = sizeof(IQ_AIISP_RESERVED_PARAM);
		aiisp->reserved_param_viraddr = (ULONG)&iq_info->aiisp_reserved_param;

		// IQ_NR_PARAM setting
		if (iq_info->ai_aided_en) {
			ife_filter->motion_set.static_symbol = 0;
			ife_filter->motion_set.transition_symbol = 1;
			ife_filter->motion_set.motion_symbol = 2;
		} else {
			ife_filter->motion_set.static_symbol = pre_bnr->fusion_str.static_region;
			ife_filter->motion_set.transition_symbol = pre_bnr->fusion_str.transision_region;
			ife_filter->motion_set.motion_symbol = pre_bnr->fusion_str.motion_region;
			// NOTE: aiisp aided test
			if (aiisp_aided_test_en && (iq_info->id == aiisp_aided_test_clone_id))
			{
				ISP_MODULE *iq_module = iq_get_module();
				void *iq_private = iq_module->private;
				IQALG_INFO *iq_info_id0;

				iq_info_id0 = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(aiisp_aided_test_src_id));
				ife_filter->motion_set.static_symbol = iq_info_id0->final_ipp.pre_bnr.fusion_str.static_region;
				ife_filter->motion_set.transition_symbol = iq_info_id0->final_ipp.pre_bnr.fusion_str.transision_region;
				ife_filter->motion_set.motion_symbol = iq_info_id0->final_ipp.pre_bnr.fusion_str.motion_region;
			}
		}
		if (iq_param->nr->mode == IQ_OP_TYPE_AUTO) {
			// outl
			pre_outl->ord_protect_th = iq_intpl(iq_sync_info.gain, nr_l->outl_ord_protect_th, nr_h->outl_ord_protect_th, iso_start, iso_end);
			pre_outl->ord_blend_w = iq_intpl(iq_sync_info.gain, nr_l->outl_ord_blend_w, nr_h->outl_ord_blend_w, iso_start, iso_end);
			pre_outl->outl_comp_mode = iq_intpl(iq_sync_info.gain, nr_l->outl_avg_mode, nr_h->outl_avg_mode, iso_start, iso_end);
			curr_outl_sel = iq_intpl(iq_sync_info.gain, nr_l->outl_sel, nr_h->outl_sel, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_l->outl_bright_th, nr_h->outl_bright_th, CTL_PRE_ISP_OUTL_BRI_TH_NUM, pre_outl->bright_th, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_l->outl_dark_th, nr_h->outl_dark_th, CTL_PRE_ISP_OUTL_DARK_TH_NUM, pre_outl->dark_th, iq_sync_info.gain, iso_start, iso_end);

			// gbal
			ife_gbal->diff_th_str = iq_intpl(iq_sync_info.gain, nr_l->gbal_diff_th_str, nr_h->gbal_diff_th_str, iso_start, iso_end);
			ife_gbal->edge_protect_th0 = iq_intpl(iq_sync_info.gain, nr_l->gbal_edge_protect_th, nr_h->gbal_edge_protect_th, iso_start, iso_end);
			ife_gbal->str_luma_low_bnd = iq_intpl(iq_sync_info.gain, nr_l->gbal_str_luma_low_bnd, nr_h->gbal_str_luma_low_bnd, iso_start, iso_end);
			ife_gbal->edge_luma_low_bnd = iq_intpl(iq_sync_info.gain, nr_l->gbal_edge_luma_low_bnd, nr_h->gbal_edge_luma_low_bnd, iso_start, iso_end);

			// filter
			iq_intpl_tbl_uint16(nr_l->filter_th, nr_h->filter_th, IQ_NR_TH_NUM, curr_nr_filter_th, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_l->filter_lut, nr_h->filter_lut, IQ_NR_TH_LUT, curr_nr_filter_lut, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_l->filter_th_b, nr_h->filter_th_b, IQ_NR_TH_NUM, curr_nr_filter_th_b, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_l->filter_lut_b, nr_h->filter_lut_b, IQ_NR_TH_LUT, curr_nr_filter_lut_b, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_ext_l->filter_th_1, nr_ext_h->filter_th_1, IQ_NR_TH_NUM, curr_nr_filter_th_1, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_ext_l->filter_lut_1, nr_ext_h->filter_lut_1, IQ_NR_TH_LUT, curr_nr_filter_lut_1, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_ext_l->filter_th_b_1, nr_ext_h->filter_th_b_1, IQ_NR_TH_NUM, curr_nr_filter_th_b_1, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_ext_l->filter_lut_b_1, nr_ext_h->filter_lut_b_1, IQ_NR_TH_LUT, curr_nr_filter_lut_b_1, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_ext_l->filter_th_2, nr_ext_h->filter_th_2, IQ_NR_TH_NUM, curr_nr_filter_th_2, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_ext_l->filter_lut_2, nr_ext_h->filter_lut_2, IQ_NR_TH_LUT, curr_nr_filter_lut_2, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_ext_l->filter_th_b_2, nr_ext_h->filter_th_b_2, IQ_NR_TH_NUM, curr_nr_filter_th_b_2, iq_sync_info.gain, iso_start, iso_end);
			iq_intpl_tbl_uint16(nr_ext_l->filter_lut_b_2, nr_ext_h->filter_lut_b_2, IQ_NR_TH_LUT, curr_nr_filter_lut_b_2, iq_sync_info.gain, iso_start, iso_end);
			ife_filter->blend_w = (UINT8)iq_intpl(iq_sync_info.gain, nr_l->filter_blend_w, nr_h->filter_blend_w, iso_start, iso_end);
			ife_filter->clamp.th = (UINT16)iq_intpl(iq_sync_info.gain, nr_l->filter_clamp_th, nr_h->filter_clamp_th, iso_start, iso_end);
			ife_filter->clamp.mul = (UINT8)iq_intpl(iq_sync_info.gain, nr_l->filter_clamp_mul, nr_h->filter_clamp_mul, iso_start, iso_end);

			// lca
			ime_lca->rf.edge_th[0] = (UINT32)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_edge_th, nr_lca_h->lca_edge_th, iso_start_enh, iso_end_enh);
			ime_lca->rf.ctr_3x3_y_th[0] = (UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_filter_level[0], nr_lca_h->lca_y_filter_level[0], iso_start_enh, iso_end_enh);
			ime_lca->rf.ctr_3x3_u_th[0] = (UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_filter_level[0], nr_lca_h->lca_c_filter_level[0], iso_start_enh, iso_end_enh);
			ime_lca->rf.ctr_3x3_v_th[0] = ime_lca->rf.ctr_3x3_u_th[0];
			ime_lca->rf.edge_region_y_th[0] = (UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_filter_level[0], nr_lca_h->lca_y_filter_level[0], iso_start_enh, iso_end_enh);
			ime_lca->rf.ss_region_y_th[0] = (UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_filter_level[1], nr_lca_h->lca_y_filter_level[1], iso_start_enh, iso_end_enh);
			ime_lca->rf.motion_region_y_th[0] = (UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_filter_level[2], nr_lca_h->lca_y_filter_level[2], iso_start_enh, iso_end_enh);
			ime_lca->rf.edge_region_uv_th[0] = (UINT16)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_filter_level[0], nr_lca_h->lca_c_filter_level[0], iso_start_enh, iso_end_enh);
			ime_lca->rf.ss_region_uv_th[0] = (UINT16)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_filter_level[1], nr_lca_h->lca_c_filter_level[1], iso_start_enh, iso_end_enh);
			ime_lca->rf.motion_region_uv_th[0] = (UINT16)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_filter_level[2], nr_lca_h->lca_c_filter_level[2], iso_start_enh, iso_end_enh);
			ime_lca->coring_gain.still_y_gain[2] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_coring_gain[0], nr_lca_h->lca_y_coring_gain[0], iso_start_enh, iso_end_enh), 0, 32);
			ime_lca->coring_gain.still_y_gain[1] = ime_lca->coring_gain.still_y_gain[2];
			ime_lca->coring_gain.still_y_gain[0] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_coring_gain[1], nr_lca_h->lca_y_coring_gain[1], iso_start_enh, iso_end_enh), 0, 32);
			ime_lca->coring_gain.motion_y_gain[0] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_coring_gain[2], nr_lca_h->lca_y_coring_gain[2], iso_start_enh, iso_end_enh), 0, 32);
			ime_lca->coring_gain.motion_y_gain[1] = ime_lca->coring_gain.motion_y_gain[0];
			ime_lca->coring_gain.motion_y_gain[2] = ime_lca->coring_gain.motion_y_gain[0];
			ime_lca->coring_gain.still_u_gain[2] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_coring_gain[0], nr_lca_h->lca_c_coring_gain[0], iso_start_enh, iso_end_enh), 0, 32);
			ime_lca->coring_gain.still_u_gain[1] = ime_lca->coring_gain.still_u_gain[2];
			ime_lca->coring_gain.still_u_gain[0] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_coring_gain[1], nr_lca_h->lca_c_coring_gain[1], iso_start_enh, iso_end_enh), 0, 32);
			ime_lca->coring_gain.motion_u_gain[0] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_coring_gain[2], nr_lca_h->lca_c_coring_gain[2], iso_start_enh, iso_end_enh), 0, 32);
			ime_lca->coring_gain.motion_u_gain[1] = ime_lca->coring_gain.motion_u_gain[0];
			ime_lca->coring_gain.motion_u_gain[2] = ime_lca->coring_gain.motion_u_gain[0];
			memcpy(ime_lca->coring_gain.still_v_gain, ime_lca->coring_gain.still_u_gain, sizeof(UINT8) * CTL_IME_ISP_CORING_GAIN_LEN);
			memcpy(ime_lca->coring_gain.motion_v_gain, ime_lca->coring_gain.motion_u_gain, sizeof(UINT8) * CTL_IME_ISP_CORING_GAIN_LEN);
			ime_lca->cutout_coff.still_y_coff[2] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_coring_cutoff[0], nr_lca_h->lca_y_coring_cutoff[0], iso_start_enh, iso_end_enh), 0, 63);
			ime_lca->cutout_coff.still_y_coff[1] = ime_lca->cutout_coff.still_y_coff[2];
			ime_lca->cutout_coff.still_y_coff[0] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_coring_cutoff[1], nr_lca_h->lca_y_coring_cutoff[1], iso_start_enh, iso_end_enh), 0, 63);
			ime_lca->cutout_coff.motion_y_coff[0] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_y_coring_cutoff[2], nr_lca_h->lca_y_coring_cutoff[2], iso_start_enh, iso_end_enh), 0, 63);
			ime_lca->cutout_coff.motion_y_coff[1] = ime_lca->cutout_coff.motion_y_coff[0];
			ime_lca->cutout_coff.motion_y_coff[2] = ime_lca->cutout_coff.motion_y_coff[0];
			ime_lca->cutout_coff.still_u_coff[2] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_coring_cutoff[0], nr_lca_h->lca_c_coring_cutoff[0], iso_start_enh, iso_end_enh), 0, 63);
			ime_lca->cutout_coff.still_u_coff[1] = ime_lca->cutout_coff.still_u_coff[2];
			ime_lca->cutout_coff.still_u_coff[0] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_coring_cutoff[1], nr_lca_h->lca_c_coring_cutoff[1], iso_start_enh, iso_end_enh), 0, 63);
			ime_lca->cutout_coff.motion_u_coff[0] = IQ_CLAMP((UINT8)iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->lca_c_coring_cutoff[2], nr_lca_h->lca_c_coring_cutoff[2], iso_start_enh, iso_end_enh), 0, 63);
			ime_lca->cutout_coff.motion_u_coff[1] = ime_lca->cutout_coff.motion_u_coff[0];
			ime_lca->cutout_coff.motion_u_coff[2] = ime_lca->cutout_coff.motion_u_coff[0];
			memcpy(ime_lca->cutout_coff.still_v_coff, ime_lca->cutout_coff.still_u_coff, sizeof(UINT8) * CTL_IME_ISP_CUTOUT_LEN);
			memcpy(ime_lca->cutout_coff.motion_v_coff, ime_lca->cutout_coff.motion_u_coff, sizeof(UINT8) * CTL_IME_ISP_CUTOUT_LEN);

			// dbcs
			ime_dbcs->step_y = iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->dbcs_step_y, nr_lca_h->dbcs_step_y, iso_start, iso_end);
			ime_dbcs->step_c = iq_intpl(iq_info->final_ipp.enh_gain, nr_lca_l->dbcs_step_c, nr_lca_h->dbcs_step_c, iso_start, iso_end);
		} else {
			// outl
			pre_outl->ord_protect_th = nr_m->outl_ord_protect_th;
			pre_outl->ord_blend_w = nr_m->outl_ord_blend_w;
			pre_outl->outl_comp_mode = nr_m->outl_avg_mode;
			curr_outl_sel = nr_m->outl_sel;
			memcpy(pre_outl->bright_th, nr_m->outl_bright_th, sizeof(UINT16) * CTL_IFE_ISP_OUTL_BRI_TH_NUM);
			memcpy(pre_outl->dark_th, nr_m->outl_dark_th, sizeof(UINT16) * CTL_IFE_ISP_OUTL_DARK_TH_NUM);

			// gbal
			ife_gbal->diff_th_str = nr_m->gbal_diff_th_str;
			ife_gbal->edge_protect_th0 = nr_m->gbal_edge_protect_th;
			ife_gbal->str_luma_low_bnd = nr_m->gbal_str_luma_low_bnd;
			ife_gbal->edge_luma_low_bnd = nr_m->gbal_edge_luma_low_bnd;

			// filter
			memcpy(curr_nr_filter_th, nr_m->filter_th, sizeof(UINT16) * CTL_IFE_ISP_RANGE_A_TH_NUM);
			memcpy(curr_nr_filter_lut, nr_m->filter_lut, sizeof(UINT16) * CTL_IFE_ISP_RANGE_A_LUT_SIZE);
			memcpy(curr_nr_filter_th_b, nr_m->filter_th_b, sizeof(UINT16) * CTL_IFE_ISP_RANGE_B_TH_NUM);
			memcpy(curr_nr_filter_lut_b, nr_m->filter_lut_b, sizeof(UINT16) * CTL_IFE_ISP_RANGE_B_LUT_SIZE);
			ife_filter->blend_w = nr_m->filter_blend_w;
			ife_filter->clamp.th = nr_m->filter_clamp_th;
			ife_filter->clamp.mul = nr_m->filter_clamp_mul;
			memcpy(curr_nr_filter_th_1, nr_ext_m->filter_th_1, sizeof(UINT16) * CTL_IFE_ISP_RANGE_A_TH_NUM);
			memcpy(curr_nr_filter_lut_1, nr_ext_m->filter_lut_1, sizeof(UINT16) * CTL_IFE_ISP_RANGE_A_LUT_SIZE);
			memcpy(curr_nr_filter_th_b_1, nr_ext_m->filter_th_b_1, sizeof(UINT16) * CTL_IFE_ISP_RANGE_B_TH_NUM);
			memcpy(curr_nr_filter_lut_b_1, nr_ext_m->filter_lut_b_1, sizeof(UINT16) * CTL_IFE_ISP_RANGE_B_LUT_SIZE);
			memcpy(curr_nr_filter_th_2, nr_ext_m->filter_th_2, sizeof(UINT16) * CTL_IFE_ISP_RANGE_A_TH_NUM);
			memcpy(curr_nr_filter_lut_2, nr_ext_m->filter_lut_2, sizeof(UINT16) * CTL_IFE_ISP_RANGE_A_LUT_SIZE);
			memcpy(curr_nr_filter_th_b_2, nr_ext_m->filter_th_b_2, sizeof(UINT16) * CTL_IFE_ISP_RANGE_B_TH_NUM);
			memcpy(curr_nr_filter_lut_b_2, nr_ext_m->filter_lut_b_2, sizeof(UINT16) * CTL_IFE_ISP_RANGE_B_LUT_SIZE);

			// lca
			ime_lca->rf.edge_th[0] = nr_m->lca_edge_th;
			ime_lca->rf.ctr_3x3_y_th[0] = nr_m->lca_y_filter_level[0];
			ime_lca->rf.ctr_3x3_u_th[0] = nr_m->lca_c_filter_level[0];
			ime_lca->rf.ctr_3x3_v_th[0] = ime_lca->rf.ctr_3x3_u_th[0];
			ime_lca->rf.edge_region_y_th[0] = nr_m->lca_y_filter_level[0];
			ime_lca->rf.ss_region_y_th[0] = nr_m->lca_y_filter_level[1];
			ime_lca->rf.motion_region_y_th[0] = nr_m->lca_y_filter_level[2];
			ime_lca->rf.edge_region_uv_th[0] = nr_m->lca_c_filter_level[0];
			ime_lca->rf.ss_region_uv_th[0] = nr_m->lca_c_filter_level[1];
			ime_lca->rf.motion_region_uv_th[0] = nr_m->lca_c_filter_level[2];
			ime_lca->coring_gain.still_y_gain[2] = nr_m->lca_y_coring_gain[0];
			ime_lca->coring_gain.still_y_gain[1] = ime_lca->coring_gain.still_y_gain[2];
			ime_lca->coring_gain.still_y_gain[0] = nr_m->lca_y_coring_gain[1];
			ime_lca->coring_gain.motion_y_gain[0] = nr_m->lca_y_coring_gain[2];
			ime_lca->coring_gain.motion_y_gain[1] = ime_lca->coring_gain.motion_y_gain[0];
			ime_lca->coring_gain.motion_y_gain[2] = ime_lca->coring_gain.motion_y_gain[0];
			ime_lca->coring_gain.still_u_gain[2] = nr_m->lca_c_coring_gain[0];
			ime_lca->coring_gain.still_u_gain[1] = ime_lca->coring_gain.still_u_gain[2];
			ime_lca->coring_gain.still_u_gain[0] = nr_m->lca_c_coring_gain[1];
			ime_lca->coring_gain.motion_u_gain[0] = nr_m->lca_c_coring_gain[2];
			ime_lca->coring_gain.motion_u_gain[1] = ime_lca->coring_gain.motion_u_gain[0];
			ime_lca->coring_gain.motion_u_gain[2] = ime_lca->coring_gain.motion_u_gain[0];
			memcpy(ime_lca->coring_gain.still_v_gain, ime_lca->coring_gain.still_u_gain, sizeof(UINT8) * CTL_IME_ISP_CORING_GAIN_LEN);
			memcpy(ime_lca->coring_gain.motion_v_gain, ime_lca->coring_gain.motion_u_gain, sizeof(UINT8) * CTL_IME_ISP_CORING_GAIN_LEN);
			ime_lca->cutout_coff.still_y_coff[2] = nr_m->lca_y_coring_cutoff[0];
			ime_lca->cutout_coff.still_y_coff[1] = ime_lca->cutout_coff.still_y_coff[2];
			ime_lca->cutout_coff.still_y_coff[0] = nr_m->lca_y_coring_cutoff[1];
			ime_lca->cutout_coff.motion_y_coff[0] = nr_m->lca_y_coring_cutoff[2];
			ime_lca->cutout_coff.motion_y_coff[1] = ime_lca->cutout_coff.motion_y_coff[0];
			ime_lca->cutout_coff.motion_y_coff[2] = ime_lca->cutout_coff.motion_y_coff[0];
			ime_lca->cutout_coff.still_u_coff[2] = nr_m->lca_c_coring_cutoff[0];
			ime_lca->cutout_coff.still_u_coff[1] = ime_lca->cutout_coff.still_u_coff[2];
			ime_lca->cutout_coff.still_u_coff[0] = nr_m->lca_c_coring_cutoff[1];
			ime_lca->cutout_coff.motion_u_coff[0] = nr_m->lca_c_coring_cutoff[2];
			ime_lca->cutout_coff.motion_u_coff[1] = ime_lca->cutout_coff.motion_u_coff[0];
			ime_lca->cutout_coff.motion_u_coff[2] = ime_lca->cutout_coff.motion_u_coff[0];
			memcpy(ime_lca->cutout_coff.still_v_coff, ime_lca->cutout_coff.still_u_coff, sizeof(UINT8) * CTL_IME_ISP_CUTOUT_LEN);
			memcpy(ime_lca->cutout_coff.motion_v_coff, ime_lca->cutout_coff.motion_u_coff, sizeof(UINT8) * CTL_IME_ISP_CUTOUT_LEN);

			// dbcs
			ime_dbcs->step_y = nr_lca_m->dbcs_step_y;
			ime_dbcs->step_c = nr_lca_m->dbcs_step_c;
		}

		// Outlier internal setting
		if (curr_outl_sel == IQ_NR_OUTL_8_NODE) {
			pre_outl->outl_cnt[0] = 1;
			pre_outl->outl_cnt[1] = 0;
		} else if (curr_outl_sel == IQ_NR_OUTL_7_NODE) {
			pre_outl->outl_cnt[0] = 3;
			pre_outl->outl_cnt[1] = 2;
		} else {
			pre_outl->outl_cnt[0] = 1;
			pre_outl->outl_cnt[1] = 0;
		}

		// Gbal internal setting
		ife_gbal->edge_protect_th1 = IQ_CLAMP(ife_gbal->edge_protect_th0 * 125 / 100, 0, 4095);
		// Filter internal setting
		if (nr_ir_ratio[iq_info->id] != 0) {
			ir_2dnr_enh_ratio = nr_ir_ratio[iq_info->id];
		} else {
			ir_2dnr_enh_ratio = iq_intpl(iq_sync_info.gain, rgbir_2dnr_enh_ratio[iso_idx_l], rgbir_2dnr_enh_ratio[iso_idx_h], iso_start, iso_end);
		}
		PRINT_IQ(dbg_rgbir_en, "ir_2dnr_enh_ratio = %d \r\n", ir_2dnr_enh_ratio);

		if ((nr_r_ratio[iq_info->id] != 0) || (nr_b_ratio[iq_info->id] != 0)) {
			for (i = 0; i < CTL_IFE_ISP_RANGE_A_TH_NUM; i++) {
				// b_th is 2X of a_th, ch0 = R, ch1 = Gr, ch2 = Gb, ch3 = B
				ife_filter->rng_filt_r.a_th[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b.a_th[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir.a_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r.b_th[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b.b_th[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir.b_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r_1.a_th[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr_1.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb_1.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b_1.a_th[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir_1.a_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r_1.b_th[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr_1.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb_1.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b_1.b_th[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir_1.b_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r_2.a_th[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr_2.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb_2.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b_2.a_th[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir_2.a_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r_2.b_th[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr_2.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb_2.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b_2.b_th[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir_2.b_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
			}
			for (i = 0; i < CTL_IFE_ISP_RANGE_A_LUT_SIZE; i++) {
				// b_lut is 2X of a_lut, ch0 = R, ch1 = Gr, ch2 = Gb, ch3 = B
				ife_filter->rng_filt_r.a_lut[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b.a_lut[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir.a_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r.b_lut[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b.b_lut[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir.b_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r_1.a_lut[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr_1.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb_1.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b_1.a_lut[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir_1.a_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r_1.b_lut[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr_1.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb_1.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b_1.b_lut[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir_1.b_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r_2.a_lut[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr_2.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb_2.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b_2.a_lut[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir_2.a_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);

				ife_filter->rng_filt_r_2.b_lut[i] = (UINT16)iq_cal(100 * nr_r_ratio[iq_info->id] >> 8, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gr_2.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_gb_2.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_b_2.b_lut[i] = (UINT16)iq_cal(100 * nr_b_ratio[iq_info->id] >> 8, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				ife_filter->rng_filt_ir_2.b_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
			}
		} else {
			if (((iq_info->cg_mode == IQ_CG_SIE) || (iq_info->cg_mode == IQ_CG_IFE_F)) && (ui_night_mode != IQ_UI_NIGHT_MODE_ON)) {
				curr_rgain_sqrt_id = (iq_sync_info.cgain[0] * 5) / iq_sync_info.cgain[1];
				curr_bgain_sqrt_id = (iq_sync_info.cgain[2] * 5) / iq_sync_info.cgain[1];
				curr_rgain_sqrt = iq_intpl(iq_sync_info.cgain[0], iq_sqrt_tab[curr_rgain_sqrt_id], iq_sqrt_tab[curr_rgain_sqrt_id + 1], (curr_rgain_sqrt_id * iq_sync_info.cgain[1]) / 5, ((curr_rgain_sqrt_id + 1) * iq_sync_info.cgain[1]) / 5);
				curr_bgain_sqrt = iq_intpl(iq_sync_info.cgain[2], iq_sqrt_tab[curr_bgain_sqrt_id], iq_sqrt_tab[curr_bgain_sqrt_id + 1], (curr_bgain_sqrt_id * iq_sync_info.cgain[1]) / 5, ((curr_bgain_sqrt_id + 1) * iq_sync_info.cgain[1]) / 5);
				for (i = 0; i < CTL_IFE_ISP_RANGE_A_TH_NUM; i++) {
					// b_th is 2X of a_th, ch0 = R, ch1 = Gr, ch2 = Gb, ch3 = B
					ife_filter->rng_filt_r.a_th[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b.a_th[i] = (UINT16)iq_cal(100 * curr_bgain_sqrt >> 8, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir.a_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r.b_th[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b.b_th[i] = (UINT16)iq_cal(100  * curr_bgain_sqrt >> 8, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir.b_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_1.a_th[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_1.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_1.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_1.a_th[i] = (UINT16)iq_cal(100 * curr_bgain_sqrt >> 8, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_1.a_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					
					ife_filter->rng_filt_r_1.b_th[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_1.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_1.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_1.b_th[i] = (UINT16)iq_cal(100  * curr_bgain_sqrt >> 8, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_1.b_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_gr_2.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_2.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_2.a_th[i] = (UINT16)iq_cal(100 * curr_bgain_sqrt >> 8, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_2.a_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_2.b_th[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_2.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_2.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_2.b_th[i] = (UINT16)iq_cal(100  * curr_bgain_sqrt >> 8, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_2.b_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				}
				for (i = 0; i < CTL_IFE_ISP_RANGE_A_LUT_SIZE; i++) {
					// b_lut is 2X of a_lut, ch0 = R, ch1 = Gr, ch2 = Gb, ch3 = B
					ife_filter->rng_filt_r.a_lut[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b.a_lut[i] = (UINT16)iq_cal(100 * curr_bgain_sqrt >> 8, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir.a_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r.b_lut[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b.b_lut[i] = (UINT16)iq_cal(100 * curr_bgain_sqrt >> 8, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir.b_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_1.a_lut[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_1.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_1.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_1.a_lut[i] = (UINT16)iq_cal(100 * curr_bgain_sqrt >> 8, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_1.a_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					
					ife_filter->rng_filt_r_1.b_lut[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_1.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_1.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_1.b_lut[i] = (UINT16)iq_cal(100 * curr_bgain_sqrt >> 8, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_1.b_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_2.a_lut[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_2.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_2.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_2.a_lut[i] = (UINT16)iq_cal(100 * curr_bgain_sqrt >> 8, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_2.a_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_2.b_lut[i] = (UINT16)iq_cal(100 * curr_rgain_sqrt >> 8, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_2.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_2.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_2.b_lut[i] = (UINT16)iq_cal(100 * curr_bgain_sqrt >> 8, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_2.b_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				}
			} else {
				for (i = 0; i < CTL_IFE_ISP_RANGE_A_TH_NUM; i++) {
					// b_th is 2X of a_th, same setting on 4 channel
					ife_filter->rng_filt_r.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir.a_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir.b_th[i] = (UINT16)iq_cal(100  * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_b[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_1.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_1.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_1.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_1.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_1.a_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_1[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_1.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_1.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_1.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_1.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_1.b_th[i] = (UINT16)iq_cal(100  * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_2.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_2.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_2.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_2.a_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_2.a_th[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_2[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_2.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_2.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_2.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_2.b_th[i] = (UINT16)iq_cal(100, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_2.b_th[i] = (UINT16)iq_cal(100	* ir_2dnr_enh_ratio >> 4, curr_nr_filter_th_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				}
				for (i = 0; i < CTL_IFE_ISP_RANGE_A_LUT_SIZE; i++) {
					// b_lut is 2X of a_lut, same setting on 4 channel
					ife_filter->rng_filt_r.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir.a_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir.b_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_b[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_1.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_1.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_1.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_1.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_1.a_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_1[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_1.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_1.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_1.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_1.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_1.b_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_b_1[i], 0, 1023, IQ_CAL_MULTIPLY);

					ife_filter->rng_filt_r_2.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_2.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_2.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_2.a_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_2.a_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					
					ife_filter->rng_filt_r_2.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gr_2.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_gb_2.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_b_2.b_lut[i] = (UINT16)iq_cal(100, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
					ife_filter->rng_filt_ir_2.b_lut[i] = (UINT16)iq_cal(100 * ir_2dnr_enh_ratio >> 4, curr_nr_filter_lut_b_2[i], 0, 1023, IQ_CAL_MULTIPLY);
				}
			}
		}

		// LCA internal setting
		ime_lca->rf.edge_th[1] = ime_lca->rf.edge_th[0];
		ime_lca->rf.ctr_3x3_y_th[0] = IQ_CLAMP(ime_lca->rf.ctr_3x3_y_th[0], 0, 255);
		ime_lca->rf.ctr_3x3_y_th[1] = IQ_CLAMP(ime_lca->rf.ctr_3x3_y_th[0] * 1414 / 1000, 0, 255);
		ime_lca->rf.ctr_3x3_y_th[2] = IQ_CLAMP(ime_lca->rf.ctr_3x3_y_th[0] * 1732 / 1000, 0, 255);
		ime_lca->rf.ctr_3x3_u_th[0] = IQ_CLAMP(ime_lca->rf.ctr_3x3_u_th[0], 0, 255);
		ime_lca->rf.ctr_3x3_u_th[1] = IQ_CLAMP(ime_lca->rf.ctr_3x3_u_th[0] * 1414 / 1000, 0, 255);
		ime_lca->rf.ctr_3x3_u_th[2] = IQ_CLAMP(ime_lca->rf.ctr_3x3_u_th[0] * 1732 / 1000, 0, 255);
		memcpy(ime_lca->rf.ctr_3x3_v_th, ime_lca->rf.ctr_3x3_u_th, sizeof(UINT8) * CTL_IME_ISP_RF_CTR_TH_LEN);
		ime_lca->rf.ss_region_y_th[0] = IQ_CLAMP(ime_lca->rf.ss_region_y_th[0], 0, 255);
		ime_lca->rf.ss_region_y_th[1] = IQ_CLAMP(ime_lca->rf.ss_region_y_th[0] * 1414 / 1000, 0, 255);
		ime_lca->rf.ss_region_y_th[2] = IQ_CLAMP(ime_lca->rf.ss_region_y_th[0] * 1732 / 1000, 0, 255);
		ime_lca->rf.ss_region_y_th[3] = IQ_CLAMP(ime_lca->rf.ss_region_y_th[0] * 2000 / 1000, 0, 255);
		ime_lca->rf.ss_region_y_th[4] = IQ_CLAMP(ime_lca->rf.ss_region_y_th[0] * 2236 / 1000, 0, 255);
		ime_lca->rf.edge_region_y_th[0] = IQ_CLAMP(ime_lca->rf.edge_region_y_th[0], 0, 255);
		ime_lca->rf.edge_region_y_th[1] = IQ_CLAMP(ime_lca->rf.edge_region_y_th[0] * 1414 / 1000, 0, 255);
		ime_lca->rf.edge_region_y_th[2] = IQ_CLAMP(ime_lca->rf.edge_region_y_th[0] * 1732 / 1000, 0, 255);
		ime_lca->rf.edge_region_y_th[3] = IQ_CLAMP(ime_lca->rf.edge_region_y_th[0] * 2000 / 1000, 0, 255);
		ime_lca->rf.edge_region_y_th[4] = IQ_CLAMP(ime_lca->rf.edge_region_y_th[0] * 2236 / 1000, 0, 255);
		ime_lca->rf.motion_region_y_th[0] = IQ_CLAMP(ime_lca->rf.motion_region_y_th[0], 0, 255);
		ime_lca->rf.motion_region_y_th[1] = IQ_CLAMP(ime_lca->rf.motion_region_y_th[0] * 1414 / 1000, 0, 255);
		ime_lca->rf.motion_region_y_th[2] = IQ_CLAMP(ime_lca->rf.motion_region_y_th[0] * 1732 / 1000, 0, 255);
		ime_lca->rf.motion_region_y_th[3] = IQ_CLAMP(ime_lca->rf.motion_region_y_th[0] * 2000 / 1000, 0, 255);
		ime_lca->rf.motion_region_y_th[4] = IQ_CLAMP(ime_lca->rf.motion_region_y_th[0] * 2236 / 1000, 0, 255);
		ime_lca->rf.ss_region_uv_th[0] = IQ_CLAMP(ime_lca->rf.ss_region_uv_th[0] * ime_lca->rf.ss_region_uv_th[0], 0, 65535);
		ime_lca->rf.ss_region_uv_th[0] = IQ_CLAMP(ime_lca->rf.ss_region_uv_th[0], 0, 65535);
		ime_lca->rf.ss_region_uv_th[1] = IQ_CLAMP(ime_lca->rf.ss_region_uv_th[0] * 1414 / 1000, 0, 65535);
		ime_lca->rf.ss_region_uv_th[2] = IQ_CLAMP(ime_lca->rf.ss_region_uv_th[0] * 1732 / 1000, 0, 65535);
		ime_lca->rf.ss_region_uv_th[3] = IQ_CLAMP(ime_lca->rf.ss_region_uv_th[0] * 2000 / 1000, 0, 65535);
		ime_lca->rf.ss_region_uv_th[4] = IQ_CLAMP(ime_lca->rf.ss_region_uv_th[0] * 2236 / 1000, 0, 65535);
		ime_lca->rf.edge_region_uv_th[0] = IQ_CLAMP(ime_lca->rf.edge_region_uv_th[0] * ime_lca->rf.edge_region_uv_th[0], 0, 65535);
		ime_lca->rf.edge_region_uv_th[0] = IQ_CLAMP(ime_lca->rf.edge_region_uv_th[0], 0, 65535);
		ime_lca->rf.edge_region_uv_th[1] = IQ_CLAMP(ime_lca->rf.edge_region_uv_th[0] * 1414 / 1000, 0, 65535);
		ime_lca->rf.edge_region_uv_th[2] = IQ_CLAMP(ime_lca->rf.edge_region_uv_th[0] * 1732 / 1000, 0, 65535);
		ime_lca->rf.edge_region_uv_th[3] = IQ_CLAMP(ime_lca->rf.edge_region_uv_th[0] * 2000 / 1000, 0, 65535);
		ime_lca->rf.edge_region_uv_th[4] = IQ_CLAMP(ime_lca->rf.edge_region_uv_th[0] * 2236 / 1000, 0, 65535);
		ime_lca->rf.motion_region_uv_th[0] = IQ_CLAMP(ime_lca->rf.motion_region_uv_th[0] * ime_lca->rf.motion_region_uv_th[0], 0, 65535);
		ime_lca->rf.motion_region_uv_th[0] = IQ_CLAMP(ime_lca->rf.motion_region_uv_th[0], 0, 65535);
		ime_lca->rf.motion_region_uv_th[1] = IQ_CLAMP(ime_lca->rf.motion_region_uv_th[0] * 1414 / 1000, 0, 65535);
		ime_lca->rf.motion_region_uv_th[2] = IQ_CLAMP(ime_lca->rf.motion_region_uv_th[0] * 1732 / 1000, 0, 65535);
		ime_lca->rf.motion_region_uv_th[3] = IQ_CLAMP(ime_lca->rf.motion_region_uv_th[0] * 2000 / 1000, 0, 65535);
		ime_lca->rf.motion_region_uv_th[4] = IQ_CLAMP(ime_lca->rf.motion_region_uv_th[0] * 2236 / 1000, 0, 65535);
		for (i = 0; i < CTL_IME_ISP_CORING_GAIN_LEN; i++) {
			ime_lca->coring_gain.still_y_gain[i] = IQ_CLAMP(32 - ime_lca->coring_gain.still_y_gain[i], 0, 32);
			ime_lca->coring_gain.motion_y_gain[i] = IQ_CLAMP(32 -ime_lca->coring_gain.motion_y_gain[i], 0, 32);
			ime_lca->coring_gain.still_u_gain[i] = IQ_CLAMP(32 - ime_lca->coring_gain.still_u_gain[i], 0, 32);
			ime_lca->coring_gain.motion_u_gain[i] = IQ_CLAMP(32 -ime_lca->coring_gain.motion_u_gain[i], 0, 32);
			ime_lca->coring_gain.still_v_gain[i] = IQ_CLAMP(32 - ime_lca->coring_gain.still_v_gain[i], 0, 32);
			ime_lca->coring_gain.motion_v_gain[i] = IQ_CLAMP(32 -ime_lca->coring_gain.motion_v_gain[i], 0, 32);
		}
		if (iq_param->nr->lca_enable) {
			ime_lca->final_y_out_wt = 32;
			ime_lca->final_uv_out_wt = 32;
		} else {
			ime_lca->final_y_out_wt = 0;
			ime_lca->final_uv_out_wt = 0;
		}

		// IQ_RGBIR_ENH_PARAM setting
		if (iq_param->rgbir_enh->mode == IQ_OP_TYPE_AUTO) {
			if (iq_info->ir_info.ir_level <= rgbir_enh_a->ir_th) {
				pre_outl->outl_rgbir_rb_w = iq_intpl(iq_sync_info.gain, rgbir_enh_d->outl_rgbir_rb_w, rgbir_enh_a->outl_rgbir_rb_w, rgbir_enh_d->ir_th, rgbir_enh_a->ir_th);
				pre_outl->ord_rgbir_rb_w = iq_intpl(iq_sync_info.gain, rgbir_enh_d->outl_ord_rgbir_rb_w, rgbir_enh_a->outl_ord_rgbir_rb_w, rgbir_enh_d->ir_th, rgbir_enh_a->ir_th);
			} else{
				pre_outl->outl_rgbir_rb_w = iq_intpl(iq_sync_info.gain, rgbir_enh_a->outl_rgbir_rb_w, rgbir_enh_ir->outl_rgbir_rb_w, rgbir_enh_a->ir_th, rgbir_enh_ir->ir_th);
				pre_outl->ord_rgbir_rb_w = iq_intpl(iq_sync_info.gain, rgbir_enh_a->outl_ord_rgbir_rb_w, rgbir_enh_ir->outl_ord_rgbir_rb_w, rgbir_enh_a->ir_th, rgbir_enh_ir->ir_th);
			}
		} else {
			pre_outl->outl_rgbir_rb_w = rgbir_enh_m->outl_rgbir_rb_w;
			pre_outl->ord_rgbir_rb_w = rgbir_enh_m->outl_ord_rgbir_rb_w;
		}

		// IQ_CFA_PARAM setting
		if (iq_param->cfa->mode == IQ_OP_TYPE_AUTO) {
			ipe_cfa->cfa_interp.edge_dth = iq_intpl(iq_sync_info.gain, cfa_l->edge_dth, cfa_h->edge_dth, iso_start, iso_end);
			ipe_cfa->cfa_interp.edge_dth2 = iq_intpl(iq_sync_info.gain, cfa_l->edge_dth2, cfa_h->edge_dth2, iso_start, iso_end);
			ipe_cfa->cfa_interp.freq_th = iq_intpl(iq_sync_info.gain, cfa_l->freq_th, cfa_h->freq_th, iso_start, iso_end);
			ipe_cfa->cfa_fcs.fcs_weight = iq_intpl(iq_sync_info.gain, cfa_l->fcs_weight, cfa_h->fcs_weight, iso_start, iso_end);
			iq_intpl_tbl_uint8(cfa_l->fcs_strength, cfa_h->fcs_strength, CTL_IPE_ISP_CFA_FCS_NUM, ipe_cfa->cfa_fcs.fcs_strength, iq_sync_info.gain, iso_start, iso_end);
		} else {
			ipe_cfa->cfa_interp.edge_dth = cfa_m->edge_dth;
			ipe_cfa->cfa_interp.edge_dth2 = cfa_m->edge_dth2;
			ipe_cfa->cfa_interp.freq_th = cfa_m->freq_th;
			ipe_cfa->cfa_fcs.fcs_weight = cfa_m->fcs_weight;
			memcpy(ipe_cfa->cfa_fcs.fcs_strength, cfa_m->fcs_strength, sizeof(UINT8) * CTL_IPE_ISP_CFA_FCS_NUM);
		}

		// IQ_RAW_VA_PARAM setting
		if (iq_param->raw_va->mode == IQ_OP_TYPE_AUTO) {
			curr_va_g1_iir_set = iq_intpl(iq_sync_info.gain, raw_va_l->g1_iir, raw_va_h->g1_iir, iso_start, iso_end);
			pre_raw_va->group_1.h_filt.th_l = iq_intpl(iq_sync_info.gain, raw_va_l->g1_th_l, raw_va_h->g1_th_l, iso_start, iso_end);
			pre_raw_va->group_1.h_filt.th_u = iq_intpl(iq_sync_info.gain, raw_va_l->g1_th_u, raw_va_h->g1_th_u, iso_start, iso_end);
			pre_raw_va->group_1.v_filt.th_l = iq_intpl(iq_sync_info.gain, raw_va_l->g1_th_l, raw_va_h->g1_th_l, iso_start, iso_end);
			pre_raw_va->group_1.v_filt.th_u = iq_intpl(iq_sync_info.gain, raw_va_l->g1_th_u, raw_va_h->g1_th_u, iso_start, iso_end);
			curr_va_g2_iir_set = iq_intpl(iq_sync_info.gain, raw_va_l->g2_iir, raw_va_h->g2_iir, iso_start, iso_end);
			pre_raw_va->group_2.h_filt.th_l = iq_intpl(iq_sync_info.gain, raw_va_l->g2_th_l, raw_va_h->g2_th_l, iso_start, iso_end);
			pre_raw_va->group_2.h_filt.th_u = iq_intpl(iq_sync_info.gain, raw_va_l->g2_th_u, raw_va_h->g2_th_u, iso_start, iso_end);
			pre_raw_va->group_2.v_filt.th_l = iq_intpl(iq_sync_info.gain, raw_va_l->g2_th_l, raw_va_h->g2_th_l, iso_start, iso_end);
			pre_raw_va->group_2.v_filt.th_u = iq_intpl(iq_sync_info.gain, raw_va_l->g2_th_u, raw_va_h->g2_th_u, iso_start, iso_end);

			if (iq_param->raw_va->g1_h_filter_sel == IQ_VA_H_FILTER_IIR) {
				curr_va_g1_iir_set = IQ_CLAMP(curr_va_g1_iir_set, 0, IQ_VA_IIR_SET_MAX_NUM - 1);
				iq_info->final_ipp.pre_va.group_1.h_filt.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_1.iir_filt.symmetry_iir2 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_1.iir_filt.symmetry_iir3 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_1.h_filt.tap_a = iq_va_iir_tap[curr_va_g1_iir_set].iir1_tap_a;
				iq_info->final_ipp.pre_va.group_1.h_filt.tap_b = iq_va_iir_tap[curr_va_g1_iir_set].iir1_tap_b;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir1_e = iq_va_iir_tap[curr_va_g1_iir_set].iir1_tap_e;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir1_f = iq_va_iir_tap[curr_va_g1_iir_set].iir1_tap_f;
				iq_info->final_ipp.pre_va.group_1.h_filt.div = iq_va_iir_tap[curr_va_g1_iir_set].iir1_div;
				iq_info->final_ipp.pre_va.group_1.iir_filt.iir1_shift_bit = iq_va_iir_tap[curr_va_g1_iir_set].iir1_shift;
				iq_info->final_ipp.pre_va.group_1.iir_filt.iir2_enable = iq_va_iir_tap[curr_va_g1_iir_set].iir2_en;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir2_a = iq_va_iir_tap[curr_va_g1_iir_set].iir2_tap_a;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir2_b = iq_va_iir_tap[curr_va_g1_iir_set].iir2_tap_b;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir2_e = iq_va_iir_tap[curr_va_g1_iir_set].iir2_tap_e;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir2_f = iq_va_iir_tap[curr_va_g1_iir_set].iir2_tap_f;
				iq_info->final_ipp.pre_va.group_1.iir_filt.iir2_shift_bit = iq_va_iir_tap[curr_va_g1_iir_set].iir2_shift;
				iq_info->final_ipp.pre_va.group_1.iir_filt.iir3_enable = iq_va_iir_tap[curr_va_g1_iir_set].iir3_en;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir3_a = iq_va_iir_tap[curr_va_g1_iir_set].iir3_tap_a;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir3_b = iq_va_iir_tap[curr_va_g1_iir_set].iir3_tap_b;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir3_e = iq_va_iir_tap[curr_va_g1_iir_set].iir3_tap_e;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir3_f = iq_va_iir_tap[curr_va_g1_iir_set].iir3_tap_f;
				iq_info->final_ipp.pre_va.group_1.iir_filt.iir3_shift_bit = iq_va_iir_tap[curr_va_g1_iir_set].iir3_shift;
			}
			if (iq_param->raw_va->g2_h_filter_sel == IQ_VA_H_FILTER_IIR) {
				curr_va_g2_iir_set = IQ_CLAMP(curr_va_g2_iir_set, 0, IQ_VA_IIR_SET_MAX_NUM - 1);
				iq_info->final_ipp.pre_va.group_2.h_filt.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_2.iir_filt.symmetry_iir2 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_2.iir_filt.symmetry_iir3 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_2.h_filt.tap_a = iq_va_iir_tap[curr_va_g2_iir_set].iir1_tap_a;
				iq_info->final_ipp.pre_va.group_2.h_filt.tap_b = iq_va_iir_tap[curr_va_g2_iir_set].iir1_tap_b;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir1_e = iq_va_iir_tap[curr_va_g2_iir_set].iir1_tap_e;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir1_f = iq_va_iir_tap[curr_va_g2_iir_set].iir1_tap_f;
				iq_info->final_ipp.pre_va.group_2.h_filt.div = iq_va_iir_tap[curr_va_g2_iir_set].iir1_div;
				iq_info->final_ipp.pre_va.group_2.iir_filt.iir1_shift_bit = iq_va_iir_tap[curr_va_g2_iir_set].iir1_shift;
				iq_info->final_ipp.pre_va.group_2.iir_filt.iir2_enable = iq_va_iir_tap[curr_va_g2_iir_set].iir2_en;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir2_a = iq_va_iir_tap[curr_va_g2_iir_set].iir2_tap_a;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir2_b = iq_va_iir_tap[curr_va_g2_iir_set].iir2_tap_b;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir2_e = iq_va_iir_tap[curr_va_g2_iir_set].iir2_tap_e;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir2_f = iq_va_iir_tap[curr_va_g2_iir_set].iir2_tap_f;
				iq_info->final_ipp.pre_va.group_2.iir_filt.iir2_shift_bit = iq_va_iir_tap[curr_va_g2_iir_set].iir2_shift;
				iq_info->final_ipp.pre_va.group_2.iir_filt.iir3_enable = iq_va_iir_tap[curr_va_g2_iir_set].iir3_en;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir3_a = iq_va_iir_tap[curr_va_g2_iir_set].iir3_tap_a;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir3_b = iq_va_iir_tap[curr_va_g2_iir_set].iir3_tap_b;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir3_e = iq_va_iir_tap[curr_va_g2_iir_set].iir3_tap_e;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir3_f = iq_va_iir_tap[curr_va_g2_iir_set].iir3_tap_f;
				iq_info->final_ipp.pre_va.group_2.iir_filt.iir3_shift_bit = iq_va_iir_tap[curr_va_g2_iir_set].iir3_shift;
			}
		} else {
			if (iq_param->raw_va->g1_h_filter_sel == IQ_VA_H_FILTER_IIR) {
				iq_info->final_ipp.pre_va.group_1.h_filt.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_1.iir_filt.symmetry_iir2 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_1.iir_filt.symmetry_iir3 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_1.h_filt.tap_a = raw_va_m->g1_iir1_tap_a;
				iq_info->final_ipp.pre_va.group_1.h_filt.tap_b = raw_va_m->g1_iir1_tap_b;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir1_e = raw_va_m->g1_iir1_tap_e;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir1_f = raw_va_m->g1_iir1_tap_f;
				iq_info->final_ipp.pre_va.group_1.h_filt.div = 0;
				iq_info->final_ipp.pre_va.group_1.iir_filt.iir1_shift_bit = raw_va_m->g1_iir1_shift_bit;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir2_a = raw_va_m->g1_iir2_tap_a;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir2_b = raw_va_m->g1_iir2_tap_b;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir2_e = raw_va_m->g1_iir2_tap_e;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir2_f = raw_va_m->g1_iir2_tap_f;
				iq_info->final_ipp.pre_va.group_1.iir_filt.iir2_shift_bit = raw_va_m->g1_iir2_shift_bit;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir3_a = raw_va_m->g1_iir3_tap_a;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir3_b = raw_va_m->g1_iir3_tap_b;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir3_e = raw_va_m->g1_iir3_tap_e;
				iq_info->final_ipp.pre_va.group_1.iir_filt.tap_iir3_f = raw_va_m->g1_iir3_tap_f;
				iq_info->final_ipp.pre_va.group_1.iir_filt.iir3_shift_bit = raw_va_m->g1_iir3_shift_bit;
			}
			pre_raw_va->group_1.h_filt.th_l = raw_va_m->g1_th_l;
			pre_raw_va->group_1.h_filt.th_u = raw_va_m->g1_th_u;
			pre_raw_va->group_1.v_filt.th_l = raw_va_m->g1_th_l;
			pre_raw_va->group_1.v_filt.th_u = raw_va_m->g1_th_u;

			if (iq_param->raw_va->g2_h_filter_sel == IQ_VA_H_FILTER_IIR) {
				iq_info->final_ipp.pre_va.group_2.h_filt.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_2.iir_filt.symmetry_iir2 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_2.iir_filt.symmetry_iir3 = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				iq_info->final_ipp.pre_va.group_2.h_filt.tap_a = raw_va_m->g2_iir1_tap_a;
				iq_info->final_ipp.pre_va.group_2.h_filt.tap_b = raw_va_m->g2_iir1_tap_b;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir1_e = raw_va_m->g2_iir1_tap_e;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir1_f = raw_va_m->g2_iir1_tap_f;
				iq_info->final_ipp.pre_va.group_2.h_filt.div = 0;
				iq_info->final_ipp.pre_va.group_2.iir_filt.iir1_shift_bit = raw_va_m->g2_iir1_shift_bit;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir2_a = raw_va_m->g2_iir2_tap_a;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir2_b = raw_va_m->g2_iir2_tap_b;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir2_e = raw_va_m->g2_iir2_tap_e;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir2_f = raw_va_m->g2_iir2_tap_f;
				iq_info->final_ipp.pre_va.group_2.iir_filt.iir2_shift_bit = raw_va_m->g2_iir2_shift_bit;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir3_a = raw_va_m->g2_iir3_tap_a;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir3_b = raw_va_m->g2_iir3_tap_b;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir3_e = raw_va_m->g2_iir3_tap_e;
				iq_info->final_ipp.pre_va.group_2.iir_filt.tap_iir3_f = raw_va_m->g2_iir3_tap_f;
				iq_info->final_ipp.pre_va.group_2.iir_filt.iir3_shift_bit = raw_va_m->g2_iir3_shift_bit;
			}
			pre_raw_va->group_2.h_filt.th_l = raw_va_m->g2_th_l;
			pre_raw_va->group_2.h_filt.th_u = raw_va_m->g2_th_u;
			pre_raw_va->group_2.v_filt.th_l = raw_va_m->g2_th_l;
			pre_raw_va->group_2.v_filt.th_u = raw_va_m->g2_th_u;
		}

		// IQ_COLOR_PARAM setting
		if (iq_param->color->enable == TRUE) {
			if (iq_param->color->mode == IQ_OP_TYPE_AUTO) {
				ipe_cadj_yccon->c_con = iq_intpl(iq_sync_info.gain, color_l->c_con, color_h->c_con, iso_start, iso_end);
				iq_intpl_tbl_uint8(color_l->fstab, color_h->fstab, CTL_IPE_ISP_FTAB_LEN, ipe_cc->fstab, iq_sync_info.gain, iso_start, iso_end);
				iq_intpl_tbl_uint8(color_l->fdtab, color_h->fdtab, CTL_IPE_ISP_FTAB_LEN, ipe_cc->fdtab, iq_sync_info.gain, iso_start, iso_end);
				iq_intpl_tbl_uint16(color_l->cconlut, color_h->cconlut, CTL_IPE_ISP_CCONTAB_LEN, ipe_cadj_yccon->cconlut, iq_sync_info.gain, iso_start, iso_end);
			} else {
				ipe_cadj_yccon->c_con = color_m->c_con;
				memcpy(ipe_cc->fstab, color_m->fstab, sizeof(UINT8) * CTL_IPE_ISP_FTAB_LEN);
				memcpy(ipe_cc->fdtab, color_m->fdtab, sizeof(UINT8) * CTL_IPE_ISP_FTAB_LEN);
				memcpy(ipe_cadj_yccon->cconlut, color_m->cconlut, sizeof(UINT16) * CTL_IPE_ISP_CCONTAB_LEN);
			}
		} else {
			ipe_cadj_yccon->c_con = ctl_ipe_cadj_yccon_init.c_con;
			memcpy(ipe_cc->fstab, ctl_ipe_cc_init.fstab, sizeof(UINT8) * CTL_IPE_ISP_FTAB_LEN);
			memcpy(ipe_cc->fdtab, ctl_ipe_cc_init.fdtab, sizeof(UINT8) * CTL_IPE_ISP_FTAB_LEN);
			memcpy(ipe_cadj_yccon->cconlut, ctl_ipe_cadj_yccon_init.cconlut, sizeof(UINT16) * CTL_IPE_ISP_CCONTAB_LEN);
		}

		// IQ_CST_PARAM setting
		if (iq_param->cst->mode == IQ_OP_TYPE_AUTO) {
			ipe_cstp->cstp_ratio = iq_intpl(iq_sync_info.gain, cst_l->cstp_ratio, cst_h->cstp_ratio, iso_start, iso_end);
		} else {
			ipe_cstp->cstp_ratio = cst_m->cstp_ratio;
		}

		// IQ_CONTRAST_PARAM setting
		if (iq_param->contrast->mode == IQ_OP_TYPE_AUTO) {
			ipe_cadj_yccon->y_con = iq_intpl(iq_sync_info.gain, contrast_l->y_con, contrast_h->y_con, iso_start, iso_end);
			iq_intpl_tbl_uint8(contrast_l->lce_lum_wt_lut, contrast_h->lce_lum_wt_lut, CTL_IPE_ISP_LCE_LUMA_LEN, curr_lce_lum_wt_lut, iq_sync_info.gain, iso_start, iso_end);
		} else {
			ipe_cadj_yccon->y_con = contrast_m->y_con;
			memcpy(curr_lce_lum_wt_lut, contrast_m->lce_lum_wt_lut, sizeof(UINT8) * CTL_IPE_ISP_LCE_LUMA_LEN);
		}
		for (i = 0; i < CTL_IPE_ISP_LCE_LUMA_LEN; i++) {
			ipe_lce->lum_wt_lut[i] = IQ_CLAMP(curr_lce_lum_wt_lut[i] + 64, 0, 255);
		}
		if (iq_param->contrast->enable == FALSE) {
			ipe_cadj_yccon->y_con = ctl_ipe_cadj_yccon_init.y_con;
		}

		// IQ_EDGE_PARAM setting
		if (iq_info->ai_aided_en) {
			ipe_eext->motion_symbol.static_symbol = 0;
			ipe_eext->motion_symbol.transition_symbol = 1;
			ipe_eext->motion_symbol.motion_symbol = 2;
		} else {
			ipe_eext->motion_symbol.static_symbol = pre_bnr->fusion_str.static_region;
			ipe_eext->motion_symbol.transition_symbol = pre_bnr->fusion_str.transision_region;
			ipe_eext->motion_symbol.motion_symbol = pre_bnr->fusion_str.motion_region;
			// NOTE: aiisp aided test
			if (aiisp_aided_test_en && (iq_info->id == aiisp_aided_test_clone_id))
			{
				ISP_MODULE *iq_module = iq_get_module();
				void *iq_private = iq_module->private;
				IQALG_INFO *iq_info_id0;

				iq_info_id0 = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(aiisp_aided_test_src_id));

				ipe_eext->motion_symbol.static_symbol = iq_info_id0->final_ipp.pre_bnr.fusion_str.static_region;
				ipe_eext->motion_symbol.transition_symbol = iq_info_id0->final_ipp.pre_bnr.fusion_str.transision_region;
				ipe_eext->motion_symbol.motion_symbol = iq_info_id0->final_ipp.pre_bnr.fusion_str.motion_region;
			}
		}
		if (iq_param->edge->mode == IQ_OP_TYPE_AUTO) {
			iq_info->target_edge_enh_p = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->edge_enh_p, edge_h->edge_enh_p, iso_start_enh, iso_end_enh);
			iq_info->target_edge_enh_n = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->edge_enh_n, edge_h->edge_enh_n, iso_start_enh, iso_end_enh);
			curr_thin_freq = IQ_CLAMP(iq_intpl(iq_info->final_ipp.enh_gain, edge_l->thin_freq, edge_h->thin_freq, iso_start_enh, iso_end_enh), 0, 16);
			curr_robust_freq = IQ_CLAMP(iq_intpl(iq_info->final_ipp.enh_gain, edge_l->robust_freq, edge_h->robust_freq, iso_start_enh, iso_end_enh), 0, 16);
			curr_th_flat_low = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->th_flat_low, edge_h->th_flat_low, iso_start_enh, iso_end_enh);
			curr_th_flat_high = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->th_flat_high, edge_h->th_flat_high, iso_start_enh, iso_end_enh);
			curr_th_edge_low = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->th_edge_low, edge_h->th_edge_low, iso_start_enh, iso_end_enh);
			curr_th_edge_high = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->th_edge_high, edge_h->th_edge_high, iso_start_enh, iso_end_enh);
			curr_th_flat_low = IQ_CLAMP(curr_th_flat_low, 0, 1023);
			curr_th_flat_high = IQ_CLAMP(curr_th_flat_high, curr_th_flat_low, 1023);
			curr_th_edge_low = IQ_CLAMP(curr_th_edge_low, curr_th_flat_high, 1023);
			curr_th_edge_high = IQ_CLAMP(curr_th_edge_high, curr_th_edge_low, 1023);
			ipe_eext->eext_region.reg_wt.wt_low = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->wt_low, edge_h->wt_low, iso_start_enh, iso_end_enh);
			ipe_eext->eext_region.reg_wt.wt_high = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->wt_high, edge_h->wt_high, iso_start_enh, iso_end_enh);
			ipe_edge_region_str->str_flat = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->str_flat, edge_h->str_flat, iso_start_enh, iso_end_enh);
			ipe_edge_region_str->str_edge = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->str_edge, edge_h->str_edge, iso_start_enh, iso_end_enh);
			ipe_edge_overshoot->slope_overshoot = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->overshoot_str, edge_h->overshoot_str, iso_start_enh, iso_end_enh);
			ipe_edge_overshoot->slope_undershoot = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->undershoot_str, edge_h->undershoot_str, iso_start_enh, iso_end_enh);
			ipe_eproc->edge_map_th.ethr_low = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->edge_ethr_low, edge_h->edge_ethr_low, iso_start_enh, iso_end_enh);
			ipe_eproc->edge_map_th.ethr_high = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->edge_ethr_high, edge_h->edge_ethr_high, iso_start_enh, iso_end_enh);
			ipe_eproc->edge_map_th.etab_low = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->edge_etab_low, edge_h->edge_etab_low, iso_start_enh, iso_end_enh);
			ipe_eproc->edge_map_th.etab_high = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->edge_etab_high, edge_h->edge_etab_high, iso_start_enh, iso_end_enh);
			ipe_eproc->es_map_th.ethr_low = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->es_ethr_low, edge_h->es_ethr_low, iso_start_enh, iso_end_enh);
			ipe_eproc->es_map_th.ethr_high = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->es_ethr_high, edge_h->es_ethr_high, iso_start_enh, iso_end_enh);
			ipe_eproc->es_map_th.etab_low = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->es_etab_low, edge_h->es_etab_low, iso_start_enh, iso_end_enh);
			ipe_eproc->es_map_th.etab_high = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->es_etab_high, edge_h->es_etab_high, iso_start_enh, iso_end_enh);
			curr_dir_eng_blend_w = iq_intpl(iq_info->final_ipp.enh_gain, edge_l->dir_eng_blend_w, edge_h->dir_eng_blend_w, iso_start_enh, iso_end_enh);
			curr_dir_eng_blend_w = IQ_CLAMP(curr_dir_eng_blend_w, 0, 16);
			ipe_eext->motion_str.static_str = iq_intpl(iq_info->final_ipp.enh_gain, edge_ext_l->motion_str[0], edge_ext_h->motion_str[0], iso_start_enh, iso_end_enh);
			ipe_eext->motion_str.static_str = IQ_CLAMP(ipe_eext->motion_str.static_str, 0, 15);
			ipe_eext->motion_str.transition_str = iq_intpl(iq_info->final_ipp.enh_gain, edge_ext_l->motion_str[1], edge_ext_h->motion_str[1], iso_start_enh, iso_end_enh);
			ipe_eext->motion_str.transition_str = IQ_CLAMP(ipe_eext->motion_str.transition_str, 0, 15);
			ipe_eext->motion_str.motion_str = iq_intpl(iq_info->final_ipp.enh_gain, edge_ext_l->motion_str[2], edge_ext_h->motion_str[2], iso_start_enh, iso_end_enh);
			ipe_eext->motion_str.motion_str = IQ_CLAMP(ipe_eext->motion_str.motion_str, 0, 15);
		} else {
			iq_info->target_edge_enh_p = edge_m->edge_enh_p;
			iq_info->target_edge_enh_n = edge_m->edge_enh_n;
			curr_thin_freq = IQ_CLAMP(edge_m->thin_freq, 0, 16);
			curr_robust_freq = IQ_CLAMP(edge_m->robust_freq, 0, 16);
			curr_th_flat_low = IQ_CLAMP(edge_m->th_flat_low, 0, 1023);
			curr_th_flat_high = IQ_CLAMP(edge_m->th_flat_high, curr_th_flat_low, 1023);
			curr_th_edge_low = IQ_CLAMP(edge_m->th_edge_low, curr_th_flat_high, 1023);
			curr_th_edge_high = IQ_CLAMP(edge_m->th_edge_high, curr_th_edge_low, 1023);
			ipe_eext->eext_region.reg_wt.wt_low = edge_m->wt_low;
			ipe_eext->eext_region.reg_wt.wt_high = edge_m->wt_high;
			ipe_edge_region_str->str_flat = edge_m->str_flat;
			ipe_edge_region_str->str_edge = edge_m->str_edge;
			ipe_edge_overshoot->slope_overshoot = edge_m->overshoot_str;
			ipe_edge_overshoot->slope_undershoot = edge_m->undershoot_str;
			ipe_eproc->edge_map_th.ethr_low = edge_m->edge_ethr_low;
			ipe_eproc->edge_map_th.ethr_high = edge_m->edge_ethr_high;
			ipe_eproc->edge_map_th.etab_low = edge_m->edge_etab_low;
			ipe_eproc->edge_map_th.etab_high = edge_m->edge_etab_high;
			ipe_eproc->es_map_th.ethr_low = edge_m->es_ethr_low;
			ipe_eproc->es_map_th.ethr_high = edge_m->es_ethr_high;
			ipe_eproc->es_map_th.etab_low = edge_m->es_etab_low;
			ipe_eproc->es_map_th.etab_high = edge_m->es_etab_high;
			curr_dir_eng_blend_w = edge_m->dir_eng_blend_w;
			curr_dir_eng_blend_w = IQ_CLAMP(curr_dir_eng_blend_w, 0, 16);
			ipe_eext->motion_str.static_str = edge_ext_m->motion_str[0];
			ipe_eext->motion_str.static_str = IQ_CLAMP(ipe_eext->motion_str.static_str, 0, 15);
			ipe_eext->motion_str.transition_str = edge_ext_m->motion_str[1];
			ipe_eext->motion_str.transition_str = IQ_CLAMP(ipe_eext->motion_str.transition_str, 0, 15);
			ipe_eext->motion_str.motion_str = edge_ext_m->motion_str[2];
			ipe_eext->motion_str.motion_str = IQ_CLAMP(ipe_eext->motion_str.motion_str, 0, 15);
		}
		if (iq_sync_info.ae_status == ISP_AE_STATUS_COARSE) {
			iq_info->target_edge_enh_p = (iq_info->target_edge_enh_p * iq_param->edge->reduce_ratio) >> 7;
			iq_info->target_edge_enh_n = (iq_info->target_edge_enh_n * iq_param->edge->reduce_ratio) >> 7;
			PRINT_IQ(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_O_EDGE, "IQ(%d) reduce_ratio (%d/128), modified enh(%d, %d) \r\n"
				, iq_info->id, iq_param->edge->reduce_ratio, iq_info->target_edge_enh_p, iq_info->target_edge_enh_n);
		}
		if ((iq_info->ipp_trig_obj.reset != TRUE) && (iq_info->ipp_trig_obj.ipp_capture != TRUE)) { // do smooth while NOT reset and Capture
			if (iq_info->enc_isp_ratio.enc_edge_ratio != 256) {
				iq_info->target_edge_enh_p = IQ_CLAMP((iq_info->target_edge_enh_p * iq_info->enc_isp_ratio.enc_edge_ratio) >> 8, 0, 1023);
				iq_info->target_edge_enh_n = IQ_CLAMP((iq_info->target_edge_enh_n * iq_info->enc_isp_ratio.enc_edge_ratio) >> 8, 0, 1023);
				PRINT_IQ(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_O_EDGE, "IQ(%d) enc_edge_ratio (%d/256), modified enh(%d, %d) \r\n"
					, iq_info->id, iq_info->enc_isp_ratio.enc_edge_ratio, iq_info->target_edge_enh_p, iq_info->target_edge_enh_n);
			}
		}

		ipe_eext->ker_thickness.wt_ker_thin = iq_edge_ker_tab[curr_thin_freq][0];
		ipe_eext->ker_thickness.iso_ker_thin = iq_edge_ker_tab[curr_thin_freq][1];
		ipe_eext->ker_thickness_hld.wt_ker_thin = iq_edge_ker_tab[IQ_CLAMP(curr_thin_freq + 2, 0, 16)][0];
		ipe_eext->ker_thickness_hld.iso_ker_thin = iq_edge_ker_tab[IQ_CLAMP(curr_thin_freq + 2, 0, 16)][1];
		ipe_eext->ker_thickness.wt_ker_robust = iq_edge_ker_tab[curr_robust_freq][0];
		ipe_eext->ker_thickness.iso_ker_robust = iq_edge_ker_tab[curr_robust_freq][1];
		ipe_eext->ker_thickness_hld.wt_ker_robust = iq_edge_ker_tab[curr_robust_freq][0];
		ipe_eext->ker_thickness_hld.iso_ker_robust = iq_edge_ker_tab[curr_robust_freq][1];
		ipe_eext->eext_region.reg_th.th_flat = curr_th_flat_high;
		ipe_eext->eext_region.reg_th.th_edge = curr_th_edge_low;
		ipe_eext->eext_region.reg_th.th_flat_hld = (ipe_eext->eext_region.reg_th.th_flat * 200) >> 8;
		ipe_eext->eext_region.reg_th.th_edge_hld = ipe_eext->eext_region.reg_th.th_edge;
		ipe_eext->eext_region.reg_wt.wt_low_hld = ipe_eext->eext_region.reg_wt.wt_low;
		ipe_eext->eext_region.reg_wt.wt_high_hld = ipe_eext->eext_region.reg_wt.wt_high;
		ipe_eext->dir_ker_para.dir_eng_blend_w1 = iq_edge_dir_w_tab[curr_dir_eng_blend_w][0];
		ipe_eext->dir_ker_para.dir_eng_blend_w2 = iq_edge_dir_w_tab[curr_dir_eng_blend_w][1];
		ipe_eext->dir_ker_para.dir_eng_blend_w3 = iq_edge_dir_w_tab[curr_dir_eng_blend_w][2];
		ipe_eext->dir_ker_para.dir_eng_blend_w4 = iq_edge_dir_w_tab[curr_dir_eng_blend_w][3];
		curr_slope_flat = (curr_th_flat_high == curr_th_flat_low) ? 0x7fff : ((64 - (INT32)ipe_edge_region_str->str_flat) << 10) / ((INT32)curr_th_flat_high - (INT32)curr_th_flat_low);
		if (curr_slope_flat > 0x7fff) {
			curr_slope_flat = 0x7fff;
		}
		ipe_edge_region_str->slope_flat = 0 - (INT16)curr_slope_flat;
		curr_slope_edge = (curr_th_edge_high == curr_th_edge_low) ? 0x7fff : (((INT32)ipe_edge_region_str->str_edge - 64) << 10) / ((INT32)curr_th_edge_high - (INT32)curr_th_edge_low);
		if (curr_slope_edge > 0x7fff) {
			curr_slope_edge = 0x7fff;
		}
		ipe_edge_region_str->slope_edge = (INT16)curr_slope_edge;
		

		// IQ_3DNR_PARAM setting
		ime_tmnr->enable = iq_param->_3dnr->enable;
		ime_tmnr->nr_param.luma_ch_en = iq_param->_3dnr->enable;
		ime_tmnr->nr_param.chroma_ch_en = iq_param->_3dnr->enable;
		ime_tmnr->fcvg_param.en = iq_param->_3dnr->fcvg_enable;
		if (iq_param->_3dnr->mode == IQ_OP_TYPE_AUTO) {
			ime_tmnr->nr_param.pf_str = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->pf_str, _3dnr_h->pf_str, iso_start_enh, iso_end_enh);
			ime_tmnr->me_param.cost_blend = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->cost_blend, _3dnr_h->cost_blend, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint16(_3dnr_l->sad_penalty, _3dnr_h->sad_penalty, CTL_IME_ISP_TMNR_ME_SAD_PENALTY_TAB, ime_tmnr->me_param.sad_penalty, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->detail_penalty, _3dnr_h->detail_penalty, CTL_IME_ISP_TMNR_ME_DETAIL_PENALTY_TAB, ime_tmnr->me_param.detail_penalty, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->switch_th, _3dnr_h->switch_th, CTL_IME_ISP_TMNR_ME_SWITCH_THRESHOLD_TAB, ime_tmnr->me_param.switch_th, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			ime_tmnr->me_param.switch_rto = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->switch_rto, _3dnr_h->switch_rto, iso_start_enh, iso_end_enh);
			curr_probability = IQ_CLAMP(iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->probability, _3dnr_h->probability, iso_start_enh, iso_end_enh), 0, 8);
			iq_intpl_tbl_uint16(_3dnr_l->sad_base, _3dnr_h->sad_base, CTL_IME_ISP_TMNR_MC_SAD_BASE_TAB, ime_tmnr->mc_param.sad_base, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->sad_coefa, _3dnr_h->sad_coefa, CTL_IME_ISP_TMNR_MC_SAD_COEFA_TAB, ime_tmnr->mc_param.sad_coefa, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint16(_3dnr_l->sad_coefb, _3dnr_h->sad_coefb, CTL_IME_ISP_TMNR_MC_SAD_COEFB_TAB, ime_tmnr->mc_param.sad_coefb, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint16(_3dnr_l->sad_std, _3dnr_h->sad_std, CTL_IME_ISP_TMNR_MC_SAD_STD_TAB, ime_tmnr->mc_param.sad_std, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->fth, _3dnr_h->fth, CTL_IME_ISP_TMNR_MC_FINAL_THRESHOLD_TAB, ime_tmnr->mc_param.fth, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			memcpy(ime_tmnr->md_param.sad_coefa, ime_tmnr->mc_param.sad_coefa, sizeof(UINT8) * CTL_IME_ISP_TMNR_MD_SAD_COEFA_TAB);
			memcpy(ime_tmnr->md_param.sad_coefb, ime_tmnr->mc_param.sad_coefb, sizeof(UINT16) * CTL_IME_ISP_TMNR_MD_SAD_COEFB_TAB);
			memcpy(ime_tmnr->md_param.sad_std, ime_tmnr->mc_param.sad_std, sizeof(UINT16) * CTL_IME_ISP_TMNR_MD_SAD_STD_TAB);
			memcpy(ime_tmnr->md_param.fth, ime_tmnr->mc_param.fth, sizeof(UINT8) * CTL_IME_ISP_TMNR_MD_FINAL_THRESHOLD_TAB);
			ime_tmnr->ps_param.mv_th = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->mv_th, _3dnr_h->mv_th, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->mix_ratio, _3dnr_h->mix_ratio, CTL_IME_ISP_TMNR_PS_MIX_RATIO_TAB, ime_tmnr->ps_param.mix_ratio, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			ime_tmnr->ps_param.ds_th = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->ds_th, _3dnr_h->ds_th, iso_start_enh, iso_end_enh);
			ime_tmnr->ps_param.blur_eth = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->blur_eth, _3dnr_h->blur_eth, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->luma_residue_th, _3dnr_h->luma_residue_th, CTL_IME_ISP_TMNR_NR_LUMA_RESIDUE_TH_TAB, ime_tmnr->nr_param.luma_residue_th, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.chroma_residue_th = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->chroma_residue_th, _3dnr_h->chroma_residue_th, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->tf0_blur_str, _3dnr_h->tf0_blur_str, CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB, ime_tmnr->nr_param.tf0_blur_str, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.tf0_blur_estr = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->tf0_blur_estr, _3dnr_h->tf0_blur_estr, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->tf0_y_str, _3dnr_h->tf0_y_str, CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB, ime_tmnr->nr_param.tf0_y_str, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->tf0_c_str, _3dnr_h->tf0_c_str, CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB, ime_tmnr->nr_param.tf0_c_str, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.u_tf0_md_th = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->tf0_u_th, _3dnr_h->tf0_u_th, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.v_tf0_md_th = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->tf0_v_th, _3dnr_h->tf0_v_th, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->tf0_uv_ratio, _3dnr_h->tf0_uv_ratio, CTL_IME_ISP_TMNR_NR_C_TF0_RATIO_TAB, ime_tmnr->nr_param.c_tf0_ratio, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->pre_filter_str, _3dnr_h->pre_filter_str, CTL_IME_ISP_TMNR_NR_PRE_FILTER_STRENGTH_TAB, ime_tmnr->nr_param.pre_filter_str, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->pre_filter_rto, _3dnr_h->pre_filter_rto, CTL_IME_ISP_TMNR_NR_PRE_FILTER_RATION_TAB, ime_tmnr->nr_param.pre_filter_rto, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.snr_base_th = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->snr_base_th, _3dnr_h->snr_base_th, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.tnr_base_th = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->tnr_base_th, _3dnr_h->tnr_base_th, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->freq_wet, _3dnr_h->freq_wet, CTL_IME_ISP_TMNR_NR_FREQ_WEIGHT_TAB, ime_tmnr->nr_param.freq_wet, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->luma_wet, _3dnr_h->luma_wet, CTL_IME_ISP_TMNR_NR_LUMA_WEIGHT_TAB, ime_tmnr->nr_param.luma_wet, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->snr_str, _3dnr_h->snr_str, CTL_IME_ISP_TMNR_NR_SFILTER_STRENGTH_TAB, ime_tmnr->nr_param.snr_str, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->tnr_str, _3dnr_h->tnr_str, CTL_IME_ISP_TMNR_NR_TFILTER_STRENGTH_TAB, ime_tmnr->nr_param.tnr_str, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->luma_3d_lut, _3dnr_h->luma_3d_lut, CTL_IME_ISP_TMNR_NR_LUMA_LUT_TAB, ime_tmnr->nr_param.luma_3d_lut, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->luma_3d_rto, _3dnr_h->luma_3d_rto, CTL_IME_ISP_TMNR_NR_LUMA_RATIO_TAB, ime_tmnr->nr_param.luma_3d_rto, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->chroma_3d_lut, _3dnr_h->chroma_3d_lut, CTL_IME_ISP_TMNR_NR_CHROMA_LUT_TAB, ime_tmnr->nr_param.chroma_3d_lut, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->chroma_3d_rto, _3dnr_h->chroma_3d_rto, CTL_IME_ISP_TMNR_NR_CHROMA_RATIO_TAB, ime_tmnr->nr_param.chroma_3d_rto, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.luma_comp_str = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->luma_comp_str, _3dnr_h->luma_comp_str, iso_start_enh, iso_end_enh);
			ime_tmnr->fcvg_param.start_point = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->fcvg_start_point, _3dnr_h->fcvg_start_point, iso_start_enh, iso_end_enh);
			ime_tmnr->fcvg_param.step_size = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->fcvg_step_size, _3dnr_h->fcvg_step_size, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.motion_sat_ratio = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_l->motion_sat_ratio, _3dnr_h->motion_sat_ratio, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint16(_3dnr_l->cshk_th, _3dnr_h->cshk_th, CTL_IME_ISP_TMNR_NR_CSHK_TH_TAB, ime_tmnr->nr_param.cshk_th, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(_3dnr_l->cshk_val, _3dnr_h->cshk_val, CTL_IME_ISP_TMNR_NR_CSHK_VAL, ime_tmnr->nr_param.cshk_val, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.dc_ratio0 = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_ext_l->dc_ratio0, _3dnr_ext_h->dc_ratio0, iso_start_enh, iso_end_enh);
			ime_tmnr->nr_param.dc_ratio1 = iq_intpl(iq_info->final_ipp.enh_gain, _3dnr_ext_l->dc_ratio1, _3dnr_ext_h->dc_ratio1, iso_start_enh, iso_end_enh);
		} else {
			ime_tmnr->nr_param.pf_str = _3dnr_m->pf_str;
			ime_tmnr->me_param.cost_blend = _3dnr_m->cost_blend;
			memcpy(ime_tmnr->me_param.sad_penalty, _3dnr_m->sad_penalty, sizeof(UINT16) * CTL_IME_ISP_TMNR_ME_SAD_PENALTY_TAB);
			memcpy(ime_tmnr->me_param.detail_penalty, _3dnr_m->detail_penalty, sizeof(UINT8) * CTL_IME_ISP_TMNR_ME_DETAIL_PENALTY_TAB);
			memcpy(ime_tmnr->me_param.switch_th, _3dnr_m->switch_th, sizeof(UINT8) * CTL_IME_ISP_TMNR_ME_SWITCH_THRESHOLD_TAB);
			ime_tmnr->me_param.switch_rto = _3dnr_m->switch_rto;
			curr_probability = IQ_CLAMP(_3dnr_m->probability, 0, 8);
			memcpy(ime_tmnr->mc_param.sad_base, _3dnr_m->sad_base, sizeof(UINT16) * CTL_IME_ISP_TMNR_MC_SAD_BASE_TAB);
			memcpy(ime_tmnr->mc_param.sad_coefa, _3dnr_m->sad_coefa, sizeof(UINT8) * CTL_IME_ISP_TMNR_MC_SAD_COEFA_TAB);
			memcpy(ime_tmnr->mc_param.sad_coefb, _3dnr_m->sad_coefb, sizeof(UINT16) * CTL_IME_ISP_TMNR_MC_SAD_COEFB_TAB);
			memcpy(ime_tmnr->mc_param.sad_std, _3dnr_m->sad_std, sizeof(UINT16) * CTL_IME_ISP_TMNR_MC_SAD_STD_TAB);
			memcpy(ime_tmnr->mc_param.fth, _3dnr_m->fth, sizeof(UINT8) * CTL_IME_ISP_TMNR_MC_FINAL_THRESHOLD_TAB);
			memcpy(ime_tmnr->md_param.sad_coefa, _3dnr_m->sad_coefa, sizeof(UINT8) * CTL_IME_ISP_TMNR_MD_SAD_COEFA_TAB);
			memcpy(ime_tmnr->md_param.sad_coefb, _3dnr_m->sad_coefb, sizeof(UINT16) * CTL_IME_ISP_TMNR_MD_SAD_COEFB_TAB);
			memcpy(ime_tmnr->md_param.sad_std, _3dnr_m->sad_std, sizeof(UINT16) * CTL_IME_ISP_TMNR_MD_SAD_STD_TAB);
			memcpy(ime_tmnr->md_param.fth, _3dnr_m->fth, sizeof(UINT8) * CTL_IME_ISP_TMNR_MD_FINAL_THRESHOLD_TAB);
			ime_tmnr->ps_param.mv_th = _3dnr_m->mv_th;
			memcpy(ime_tmnr->ps_param.mix_ratio, _3dnr_m->mix_ratio, sizeof(UINT8) * CTL_IME_ISP_TMNR_PS_MIX_RATIO_TAB);
			ime_tmnr->ps_param.ds_th = _3dnr_m->ds_th;
			ime_tmnr->ps_param.blur_eth = _3dnr_m->blur_eth;
			memcpy(ime_tmnr->nr_param.luma_residue_th, _3dnr_m->luma_residue_th, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_LUMA_RESIDUE_TH_TAB);
			ime_tmnr->nr_param.chroma_residue_th = _3dnr_m->chroma_residue_th;
			memcpy(ime_tmnr->nr_param.tf0_blur_str, _3dnr_m->tf0_blur_str, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB);
			ime_tmnr->nr_param.tf0_blur_estr = _3dnr_m->tf0_blur_estr;
			memcpy(ime_tmnr->nr_param.tf0_y_str, _3dnr_m->tf0_y_str, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB);
			memcpy(ime_tmnr->nr_param.tf0_c_str, _3dnr_m->tf0_c_str, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB);
			ime_tmnr->nr_param.u_tf0_md_th = _3dnr_m->tf0_u_th;
			ime_tmnr->nr_param.v_tf0_md_th = _3dnr_m->tf0_v_th;
			memcpy(ime_tmnr->nr_param.c_tf0_ratio, _3dnr_m->tf0_uv_ratio, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_C_TF0_RATIO_TAB);
			memcpy(ime_tmnr->nr_param.pre_filter_str, _3dnr_m->pre_filter_str, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_PRE_FILTER_STRENGTH_TAB);
			memcpy(ime_tmnr->nr_param.pre_filter_rto, _3dnr_m->pre_filter_rto, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_PRE_FILTER_RATION_TAB);
			ime_tmnr->nr_param.snr_base_th = _3dnr_m->snr_base_th;
			ime_tmnr->nr_param.tnr_base_th = _3dnr_m->tnr_base_th;
			memcpy(ime_tmnr->nr_param.freq_wet, _3dnr_m->freq_wet, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_FREQ_WEIGHT_TAB);
			memcpy(ime_tmnr->nr_param.luma_wet, _3dnr_m->luma_wet, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_LUMA_WEIGHT_TAB);
			memcpy(ime_tmnr->nr_param.snr_str, _3dnr_m->snr_str, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_SFILTER_STRENGTH_TAB);
			memcpy(ime_tmnr->nr_param.tnr_str, _3dnr_m->tnr_str, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_TFILTER_STRENGTH_TAB);
			memcpy(ime_tmnr->nr_param.luma_3d_lut, _3dnr_m->luma_3d_lut, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_LUMA_LUT_TAB);
			memcpy(ime_tmnr->nr_param.luma_3d_rto, _3dnr_m->luma_3d_rto, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_LUMA_RATIO_TAB);
			memcpy(ime_tmnr->nr_param.chroma_3d_lut, _3dnr_m->chroma_3d_lut, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_CHROMA_LUT_TAB);
			memcpy(ime_tmnr->nr_param.chroma_3d_rto, _3dnr_m->chroma_3d_rto, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_CHROMA_RATIO_TAB);
			ime_tmnr->nr_param.luma_comp_str = _3dnr_m->luma_comp_str;
			ime_tmnr->fcvg_param.start_point = _3dnr_m->fcvg_start_point;
			ime_tmnr->fcvg_param.step_size = _3dnr_m->fcvg_step_size;
			ime_tmnr->nr_param.motion_sat_ratio = _3dnr_m->motion_sat_ratio;
			memcpy(ime_tmnr->nr_param.cshk_th, _3dnr_m->cshk_th, sizeof(UINT16) * CTL_IME_ISP_TMNR_NR_CSHK_TH_TAB);
			memcpy(ime_tmnr->nr_param.cshk_val, _3dnr_m->cshk_val, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_CSHK_VAL);
			ime_tmnr->nr_param.dc_ratio0 = _3dnr_ext_m->dc_ratio0;
			ime_tmnr->nr_param.dc_ratio1 = _3dnr_ext_m->dc_ratio1;
		}
		memset(ime_tmnr->me_param.probability, 0, sizeof(UINT8) * CTL_IME_ISP_TMNR_ME_PROBABILITY_TAB);
		for (i = curr_probability; i > 0; i--) {
			ime_tmnr->me_param.probability[i - 1] = 1;
			curr_probability--;
		}
		for (i = 0; i < CTL_IME_ISP_TMNR_NR_C_TF0_RATIO_TAB; i++) {
			ime_tmnr->nr_param.c_tf0_ratio[i]  = IQ_CLAMP(255 -ime_tmnr->nr_param.c_tf0_ratio[i], 0, 255);
		}

		if (residue_reset_num[iq_info->id] != 0) {
			if (tmnr_residue_count[iq_info->id] < residue_reset_num[iq_info->id]) {
				tmnr_residue_count[iq_info->id]++;
			} else {
				for (i = 0; i < CTL_IME_ISP_TMNR_NR_LUMA_RESIDUE_TH_TAB; i++) {
					if (ime_tmnr->nr_param.luma_residue_th[i] == 0) {
						ime_tmnr->nr_param.luma_residue_th[i] = 1;
					}
				}
				tmnr_residue_count[iq_info->id] = 0;
			}
		}
		PRINT_IQ_ARR(dbg_tmnr_en, ime_tmnr->nr_param.luma_residue_th, CTL_IME_ISP_TMNR_NR_LUMA_RESIDUE_TH_TAB);
		// tmnr chroma setting for Black&White, avoid chroma residue
		if ((ipe_cadj_yccon->c_con == 0) || (ipe_cctrl->sat_ofs == -128)) {
			memcpy(ime_tmnr->nr_param.chroma_3d_lut, iq_tmnr_still.nr_param.chroma_3d_lut, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_CHROMA_LUT_TAB);
		}
		// force TMNR while scene change
		if (iq_sync_info.lv > pre_lv[iq_info->id]) {
			diff_lv = iq_sync_info.lv - pre_lv[iq_info->id];
		} else {
			diff_lv = pre_lv[iq_info->id] - iq_sync_info.lv;
		}
		if (diff_lv > iq_info->final_ipp.sync_info.lv_base) {
			curr_tmnr_still[iq_info->id] = TRUE;
		}
		if ((tmnr_ae_still_en[iq_info->id] == TRUE) && (curr_tmnr_still[iq_info->id] == TRUE) && ((iq_sync_info.ae_status == ISP_AE_STATUS_COARSE) || (tmnr_ae_count[iq_info->id] > 0))) {
			memcpy(ime_tmnr->me_param.sad_penalty, iq_tmnr_still.me_param.sad_penalty, sizeof(UINT16) * CTL_IME_ISP_TMNR_ME_SAD_PENALTY_TAB);
			memcpy(ime_tmnr->me_param.detail_penalty, iq_tmnr_still.me_param.detail_penalty, sizeof(UINT8) * CTL_IME_ISP_TMNR_ME_DETAIL_PENALTY_TAB);
			memcpy(ime_tmnr->md_param.sad_coefa, iq_tmnr_still.md_param.sad_coefa, sizeof(UINT8) * CTL_IME_ISP_TMNR_MD_SAD_COEFA_TAB);
			memcpy(ime_tmnr->mc_param.sad_coefa, iq_tmnr_still.mc_param.sad_coefa, sizeof(UINT8) * CTL_IME_ISP_TMNR_MC_SAD_COEFA_TAB);
			memcpy(ime_tmnr->md_param.fth, iq_tmnr_still.md_param.fth, sizeof(UINT8) * CTL_IME_ISP_TMNR_MD_FINAL_THRESHOLD_TAB);
			memcpy(ime_tmnr->mc_param.fth, iq_tmnr_still.mc_param.fth, sizeof(UINT8) * CTL_IME_ISP_TMNR_MC_FINAL_THRESHOLD_TAB);
			ime_tmnr->ps_param.mv_th = iq_tmnr_still.ps_param.mv_th;
			ime_tmnr->ps_param.ds_th = iq_tmnr_still.ps_param.ds_th;
			memcpy(ime_tmnr->nr_param.luma_residue_th, iq_tmnr_still.nr_param.luma_residue_th, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_LUMA_RESIDUE_TH_TAB);
			memcpy(ime_tmnr->nr_param.luma_3d_lut, iq_tmnr_still.nr_param.luma_3d_lut, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_LUMA_LUT_TAB);
			PRINT_IQ(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_O_TMNR, "IQ(%d) modified all still TMNR(%d) \r\n", iq_info->id, tmnr_ae_count[iq_info->id]);
			if (iq_sync_info.ae_status == ISP_AE_STATUS_COARSE) {
				tmnr_ae_count[iq_info->id] = 10;
			} else if (tmnr_ae_count[iq_info->id] > 0) {
				tmnr_ae_count[iq_info->id]--;
				if (tmnr_ae_count[iq_info->id] == 0) {
					curr_tmnr_still[iq_info->id] = FALSE;
				}
			}
		}
		pre_lv[iq_info->id] = iq_sync_info.lv;

		// IQ_POST_SHARPEN_1_PARAM setting
		ime_post_sharpen_1->shp_en = iq_param->post_sharpen_1->enable;
		if ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_3DNR) && (iq_param->_3dnr->enable == TRUE)) {
			ime_post_sharpen_1->motion_bit_en = TRUE;
		} else {
			ime_post_sharpen_1->motion_bit_en = FALSE;
		}
		if (iq_param->post_sharpen_1->mode == IQ_OP_TYPE_AUTO) {
			ime_post_sharpen_1->noise_lv = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->noise_level, post_sharpen_1_h->noise_level, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(post_sharpen_1_l->noise_curve, post_sharpen_1_h->noise_curve, IQ_POST_SHARPEN_CURVE_NUM, ime_post_sharpen_1->noise_curve, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->weight_th = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->edge_weight_th, post_sharpen_1_h->edge_weight_th, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->weight_gain = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->edge_weight_gain, post_sharpen_1_h->edge_weight_gain, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->flat_th = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->th_flat, post_sharpen_1_h->th_flat, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->edge_th = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->th_edge, post_sharpen_1_h->th_edge, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->flat_region_str = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->flat_region_str, post_sharpen_1_h->flat_region_str, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->edge_region_str = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->edge_region_str, post_sharpen_1_h->edge_region_str, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->motion_str = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->motion_edge_w_str, post_sharpen_1_h->motion_edge_w_str, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->trans_str = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->tarnsition_edge_w_str, post_sharpen_1_h->tarnsition_edge_w_str, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->static_str = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->static_edge_w_str, post_sharpen_1_h->static_edge_w_str, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->coring_th = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->coring_th, post_sharpen_1_h->coring_th, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->blend_inv_gamma = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->blend_inv_gamma, post_sharpen_1_h->blend_inv_gamma, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->jnd_filt_sel = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->edge_filt_sel, post_sharpen_1_h->edge_filt_sel, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->edge_str = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->sharp_str, post_sharpen_1_h->sharp_str, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->bright_halo_clip = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->bright_halo_clip, post_sharpen_1_h->bright_halo_clip, iso_start_enh, iso_end_enh);
			ime_post_sharpen_1->dark_halo_clip = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_1_l->dark_halo_clip, post_sharpen_1_h->dark_halo_clip, iso_start_enh, iso_end_enh);
		} else {
			ime_post_sharpen_1->noise_lv = post_sharpen_1_m->noise_level;
			memcpy(ime_post_sharpen_1->noise_curve, post_sharpen_1_m->noise_curve, sizeof(UINT8) * IQ_POST_SHARPEN_CURVE_NUM);
			ime_post_sharpen_1->weight_th = post_sharpen_1_m->edge_weight_th;
			ime_post_sharpen_1->weight_gain = post_sharpen_1_m->edge_weight_gain;
			ime_post_sharpen_1->flat_th = post_sharpen_1_m->th_flat;
			ime_post_sharpen_1->edge_th = post_sharpen_1_m->th_edge;
			ime_post_sharpen_1->flat_region_str = post_sharpen_1_m->flat_region_str;
			ime_post_sharpen_1->edge_region_str = post_sharpen_1_m->edge_region_str;
			ime_post_sharpen_1->motion_str = post_sharpen_1_m->motion_edge_w_str;
			ime_post_sharpen_1->trans_str = post_sharpen_1_m->tarnsition_edge_w_str;
			ime_post_sharpen_1->static_str = post_sharpen_1_m->static_edge_w_str;
			ime_post_sharpen_1->coring_th = post_sharpen_1_m->coring_th;
			ime_post_sharpen_1->blend_inv_gamma = post_sharpen_1_m->blend_inv_gamma;
			ime_post_sharpen_1->jnd_filt_sel = post_sharpen_1_m->edge_filt_sel;
			ime_post_sharpen_1->edge_str = post_sharpen_1_m->sharp_str;
			ime_post_sharpen_1->bright_halo_clip = post_sharpen_1_m->bright_halo_clip;
			ime_post_sharpen_1->dark_halo_clip = post_sharpen_1_m->dark_halo_clip;
		}
		if (ime_post_sharpen_1->edge_th == ime_post_sharpen_1->flat_th) {
			ime_post_sharpen_1->slope = 4095;
		} else {
			if (ime_post_sharpen_1->edge_region_str < ime_post_sharpen_1->flat_region_str) {
				ime_post_sharpen_1->slope = 0;
			} else {
				ime_post_sharpen_1->slope = IQ_CLAMP(256 * (ime_post_sharpen_1->edge_region_str - ime_post_sharpen_1->flat_region_str) / (ime_post_sharpen_1->edge_th - ime_post_sharpen_1->flat_th), 0, 255);
			}
		}

		// IQ_SHADING_PARAM setting
		iq_intpl_tbl_uint16(iq_info->iq_ref_set.vig_lut, vig_zero, CTL_IFE_ISP_VIG_CH0_LUT_SIZE, iq_info->target_shading_vig, iq_sync_info.gain, iq_info->iq_ref_set.vig_reduce_th, iq_info->iq_ref_set.vig_zero_th);

		// IQ_PFR_PARAM setting
		if (iq_param->pfr->mode == IQ_OP_TYPE_AUTO) {
			curr_pfr_strength= iq_intpl(iq_sync_info.gain, pfr_l->pfr_strength, pfr_h->pfr_strength, iso_start, iso_end);
			ipe_pfr->luma_th = iq_intpl(iq_sync_info.gain, pfr_l->luma_th, pfr_h->luma_th, iso_start, iso_end);
		} else {
			curr_pfr_strength = pfr_m->pfr_strength;
			ipe_pfr->luma_th = pfr_m->luma_th;
		}
		if (curr_pfr_strength <= 127) {
			ipe_pfr->color_wet_g = 0;
			ipe_pfr->out_wet = IQ_CLAMP(curr_pfr_strength << 1, 0, 255);
		} else {
			ipe_pfr->color_wet_g = IQ_CLAMP((curr_pfr_strength - 127) << 1, 0, 255);
			ipe_pfr->out_wet = 255;
		}

		// IQ_WDR_PARAM setting
		if ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_WDR) && (iq_param->wdr->enable == TRUE)) {
			if (iq_param->wdr->mode == IQ_OP_TYPE_AUTO) {
				curr_wdr_set.auto_set.level = iq_intpl(iq_sync_info.gain, wdr_l->level, wdr_h->level, iso_start, iso_end);
				curr_wdr_set.auto_set.strength_min = iq_intpl(iq_sync_info.gain, wdr_l->strength_min, wdr_h->strength_min, iso_start, iso_end);
				curr_wdr_set.auto_set.strength_max = iq_intpl(iq_sync_info.gain, wdr_l->strength_max, wdr_h->strength_max, iso_start, iso_end);
				iq_operation_wdr(iq_info, iq_param, &curr_wdr_set);
			} else {
				iq_info->target_wdr_str = wdr_m->strength;
			}
		} else {
			iq_info->target_wdr_str = 0;
		}

		// IQ_DEFOG_PARAM setting
		if (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_DEFOG) {
			if (iq_param->defog->mode == IQ_OP_TYPE_AUTO) {
				curr_defog_set.auto_set.dr_th = iq_intpl(iq_sync_info.gain, defog_l->dr_th, defog_h->dr_th, iso_start, iso_end);
				curr_defog_set.auto_set.fog_level_max = iq_intpl(iq_sync_info.gain, defog_l->fog_level_max, defog_h->fog_level_max, iso_start, iso_end);
				curr_defog_set.auto_set.outbld_wt = iq_intpl(iq_sync_info.gain, defog_l->outbld_wt, defog_h->outbld_wt, iso_start, iso_end);

				ipe_defog->dfg_strength.fog_ratio = iq_intpl(iq_sync_info.gain, defog_l->fog_ratio, defog_h->fog_ratio, iso_start, iso_end);
			} else {
				curr_defog_set.manual_set.fog_level = defog_m->fog_level;

				ipe_defog->dfg_strength.fog_ratio = defog_m->fog_ratio;
				ipe_defog->dfg_strength.gain_th = IQ_CLAMP(defog_m->gain_th, 32, 255);
				memcpy(ipe_defog->dfg_outbld.outbld_lum_wt, defog_m->outbld_lum_wt, sizeof(UINT8) * CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN);
			}
			iq_operation_defog(iq_info, iq_param, &curr_defog_set);
		}

		// IQ_SHDR_PARAM setting
		// NOTE: SHDR
		if ((iq_info->ipp_flow_mode == IQ_FLOW_SHDR) && (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_SHDR)) {
			if (iq_param->shdr->mode == IQ_OP_TYPE_AUTO) {
				iq_intpl_tbl_uint16(shdr_l->nrs_s_str, shdr_h->nrs_s_str, IQ_SHDR_NRS_STR_NUM, curr_shdr_nrs_s_str, iq_sync_info.gain, iso_start, iso_end);
			} else {
				memcpy(curr_shdr_nrs_s_str, shdr_m->nrs_s_str, sizeof(UINT16) * IQ_SHDR_NRS_STR_NUM);
			}
			memcpy(pre_nrs_0->str, curr_shdr_nrs_s_str, sizeof(UINT16) * IQ_SHDR_NRS_STR_NUM);
			iq_operation_shdr_fusion(iq_info, iq_param);
			iq_operation_shdr_fcurve(iq_info, iq_param);
			iq_dbg_clr_dbg_mode(iq_info->id, IQ_DBG_A_SHDR);
		} else {
			memcpy(iq_info->final_ipp.pre_fcurve.fcurve_l.fcur_l_lut, iq_param->shdr->fcurve_left_lut, sizeof(UINT32) * IQ_SHDR_FCURVE_LEFT_NUM);
			memcpy(iq_info->final_ipp.pre_fcurve.fcurve_r.fcur_r_lut, iq_param->shdr->fcurve_right_lut, sizeof(UINT32) * IQ_SHDR_FCURVE_RIGHT_NUM);
		}
		break;

	case ISP_TRIG_IQ_ENC:
		iq_sync_info = iq_info->final_enc.sync_info;

		if (iq_info->final_ipp.enh_gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
			iso_idx_l_enh = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
			iso_idx_h_enh = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
			iso_start_enh = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso;
			iso_end_enh = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso;
		}
		for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
			if (iq_info->final_ipp.enh_gain < iso_map_tab[i].iso) {
				iso_idx_l_enh = iso_map_tab[i - 1].index;
				iso_idx_h_enh = iso_map_tab[i].index;
				iso_start_enh = iso_map_tab[i - 1].iso;
				iso_end_enh = iso_map_tab[i].iso;
				break;
			}
		}

		post_sharpen_2_m = &iq_param->post_sharpen_2->manual_param;
		post_sharpen_2_l = &iq_param->post_sharpen_2->auto_param[iso_idx_l_enh];
		post_sharpen_2_h = &iq_param->post_sharpen_2->auto_param[iso_idx_h_enh];

		// IQ_POST_SHARPEN_2_PARAM setting
		enc_post_sharpen_2->bEnable = iq_param->post_sharpen_2->enable;
		if (iq_param->post_sharpen_2->mode == IQ_OP_TYPE_AUTO) {
			enc_post_sharpen_2->ucNL = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->noise_level, post_sharpen_2_h->noise_level, iso_start_enh, iso_end_enh);
			iq_intpl_tbl_uint8(post_sharpen_2_l->noise_curve, post_sharpen_2_h->noise_curve, IQ_POST_SHARPEN_CURVE_NUM, enc_post_sharpen_2->ucNC, iq_info->final_ipp.enh_gain, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucEWT = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->edge_weight_th, post_sharpen_2_h->edge_weight_th, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucEWG = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->edge_weight_gain, post_sharpen_2_h->edge_weight_gain, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->usFlatTh = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->th_flat, post_sharpen_2_h->th_flat, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->usEdgeTh = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->th_edge, post_sharpen_2_h->th_edge, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucFlatStr = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->flat_region_str, post_sharpen_2_h->flat_region_str, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucEdgeStr = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->edge_region_str, post_sharpen_2_h->edge_region_str, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucMotionStr = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->motion_edge_w_str, post_sharpen_2_h->motion_edge_w_str, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucTransitionStr = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->tarnsition_edge_w_str, post_sharpen_2_h->tarnsition_edge_w_str, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucStaticStr = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->static_edge_w_str, post_sharpen_2_h->static_edge_w_str, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucCT = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->coring_th, post_sharpen_2_h->coring_th, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucBIG = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->blend_inv_gamma, post_sharpen_2_h->blend_inv_gamma, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucEdgeSharpStr1 = IQ_CLAMP(iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->sharp_str, post_sharpen_2_h->sharp_str, iso_start_enh, iso_end_enh), 0 , 128);
			enc_post_sharpen_2->ucBHC = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->bright_halo_clip, post_sharpen_2_h->bright_halo_clip, iso_start_enh, iso_end_enh);
			enc_post_sharpen_2->ucDHC = iq_intpl(iq_info->final_ipp.enh_gain, post_sharpen_2_l->dark_halo_clip, post_sharpen_2_h->dark_halo_clip, iso_start_enh, iso_end_enh);
		} else {
			enc_post_sharpen_2->ucNL = post_sharpen_2_m->noise_level;
			memcpy(enc_post_sharpen_2->ucNC, post_sharpen_2_m->noise_curve, sizeof(UINT8) * IQ_POST_SHARPEN_CURVE_NUM);
			enc_post_sharpen_2->ucEWT = post_sharpen_2_m->edge_weight_th;
			enc_post_sharpen_2->ucEWG = post_sharpen_2_m->edge_weight_gain;
			enc_post_sharpen_2->usFlatTh = post_sharpen_2_m->th_flat;
			enc_post_sharpen_2->usEdgeTh = post_sharpen_2_m->th_edge;
			enc_post_sharpen_2->ucFlatStr = post_sharpen_2_m->flat_region_str;
			enc_post_sharpen_2->ucEdgeStr = post_sharpen_2_m->edge_region_str;
			enc_post_sharpen_2->ucMotionStr = post_sharpen_2_m->motion_edge_w_str;
			enc_post_sharpen_2->ucTransitionStr = post_sharpen_2_m->tarnsition_edge_w_str;
			enc_post_sharpen_2->ucStaticStr = post_sharpen_2_m->static_edge_w_str;
			enc_post_sharpen_2->ucCT = post_sharpen_2_m->coring_th;
			enc_post_sharpen_2->ucBIG = post_sharpen_2_m->blend_inv_gamma;
			enc_post_sharpen_2->ucEdgeSharpStr1 = IQ_CLAMP(post_sharpen_2_m->sharp_str, 0 , 128);
			enc_post_sharpen_2->ucBHC = post_sharpen_2_m->bright_halo_clip;
			enc_post_sharpen_2->ucDHC = post_sharpen_2_m->dark_halo_clip;
		}
		if (enc_post_sharpen_2->usEdgeTh == enc_post_sharpen_2->usFlatTh) {
			enc_post_sharpen_2->usSlopConEng = 4095;
		} else {
			if (enc_post_sharpen_2->ucEdgeStr < enc_post_sharpen_2->ucFlatStr) {
				enc_post_sharpen_2->usSlopConEng = 0;
			} else {
				enc_post_sharpen_2->usSlopConEng = IQ_CLAMP(256 * (enc_post_sharpen_2->ucEdgeStr - enc_post_sharpen_2->ucFlatStr) / (enc_post_sharpen_2->usEdgeTh - enc_post_sharpen_2->usFlatTh), 0, 255);
			}
		}
		break;

	default:
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "iq_operation_iso get unknow msg(%d) \r\n", msg);
		break;
	}
}

#define IQ_RGBIR_SAT_MAX 512
void iq_operation_rgbir(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param)
{
	CTL_IFE_ISP_WDR               *ife_wdr = &iq_info->final_ipp.ife_wdr;
	CTL_IPE_ISP_CFA               *ipe_cfa = &iq_info->final_ipp.ipe_cfa;
	IQ_RGBIR_AUTO_PARAM           *rgbir_a;
	BOOL dbg_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_A_RGBIR) ? TRUE : FALSE);
	UINT32 new_irsub_r, new_irsub_g, new_irsub_b;
	UINT32 new_irsub_sat;
	UINT32 i;
	static BOOL first_run = TRUE;
	static UINT32 old_irsub_r[IQ_ID_MAX_NUM] = {0}, old_irsub_g[IQ_ID_MAX_NUM] = {0}, old_irsub_b[IQ_ID_MAX_NUM] = {0}; //16x
	static UINT32 old_irsub_sat[IQ_ID_MAX_NUM] = {0}; //16x
	static UINT32 curr_irsub_r[IQ_ID_MAX_NUM] = {0}, curr_irsub_g[IQ_ID_MAX_NUM] = {0}, curr_irsub_b[IQ_ID_MAX_NUM] = {0}; //16x
	static UINT32 curr_irsub_sat[IQ_ID_MAX_NUM] = {0}; //16x

	if (first_run) {
		for (i = 0; i < IQ_ID_MAX_NUM; i++) {
			old_irsub_sat[i] = ctl_ipe_cfa_init.cfa_ir_sub.ir_sat_gain * 16;
			curr_irsub_sat[i] = ctl_ipe_cfa_init.cfa_ir_sub.ir_sat_gain * 16;
		}
		first_run = FALSE;
	}

	switch ((UINT32)msg) {
	case ISP_TRIG_IQ_SIE:
		PRINT_IQ(dbg_en, "=======new RGBIR operation=======\r\n");
		if (iq_param->rgbir->mode == IQ_OP_TYPE_AUTO) {
			rgbir_a = &iq_param->rgbir->auto_param;
			if (iq_info->sie_trig_obj.reset == TRUE) {
				old_irsub_r[iq_info->id] = rgbir_a->irsub_r_weight << 4;
				old_irsub_g[iq_info->id] = rgbir_a->irsub_g_weight << 4;
				old_irsub_b[iq_info->id] = rgbir_a->irsub_b_weight << 4;
				old_irsub_sat[iq_info->id] = 256 << 4;
			}
			PRINT_IQ(dbg_en, "ir_level = %d, th = (%d, %d)\r\n", iq_info->ir_info.ir_level, rgbir_a->irsub_reduce_th, rgbir_a->night_mode_th);
			new_irsub_r = iq_intpl(iq_info->ir_info.ir_level, rgbir_a->irsub_r_weight, 0, rgbir_a->irsub_reduce_th, rgbir_a->night_mode_th);
			new_irsub_g = iq_intpl(iq_info->ir_info.ir_level, rgbir_a->irsub_g_weight, 0, rgbir_a->irsub_reduce_th, rgbir_a->night_mode_th);
			new_irsub_b = iq_intpl(iq_info->ir_info.ir_level, rgbir_a->irsub_b_weight, 0, rgbir_a->irsub_reduce_th, rgbir_a->night_mode_th);
			#if 0
			if (iq_info->ipp_trig_obj.ir_level <= 128) {
				new_irsub_sat = 256;
			} else if (iq_info->ipp_trig_obj.ir_level <= 152) {
				new_irsub_sat = iq_intpl(iq_info->ipp_trig_obj.ir_level, 256, IQ_RGBIR_SAT_MAX, 128, 152);
			} else if (iq_info->ipp_trig_obj.ir_level <= rgbir_a->night_mode_th) {
				new_irsub_sat = iq_intpl(iq_info->ipp_trig_obj.ir_level, IQ_RGBIR_SAT_MAX, 256, 152, rgbir_a->night_mode_th);
			} else {
				new_irsub_sat = 256;
			}
			#else
			new_irsub_sat = iq_info->ir_info.saturation;
			#endif
			PRINT_IQ(dbg_en, "new ir_sub = (%d, %d, %d)\r\n", new_irsub_r, new_irsub_g, new_irsub_b);
			PRINT_IQ(dbg_en, "new irsub_sat = %d\r\n", new_irsub_sat);
			// smooth (new:old) = (1:31)
			curr_irsub_r[iq_info->id] = (1 * (new_irsub_r << 4) + 31 * old_irsub_r[iq_info->id] + 16) >> 5;
			curr_irsub_g[iq_info->id] = (1 * (new_irsub_g << 4) + 31 * old_irsub_g[iq_info->id] + 16) >> 5;
			curr_irsub_b[iq_info->id] = (1 * (new_irsub_b << 4) + 31 * old_irsub_b[iq_info->id] + 16) >> 5;
			curr_irsub_sat[iq_info->id] = (1 * (new_irsub_sat << 4) + 31 * old_irsub_sat[iq_info->id] + 16) >> 5;
			old_irsub_r[iq_info->id] = curr_irsub_r[iq_info->id];
			old_irsub_g[iq_info->id] = curr_irsub_g[iq_info->id];
			old_irsub_b[iq_info->id] = curr_irsub_b[iq_info->id];
			old_irsub_sat[iq_info->id] = curr_irsub_sat[iq_info->id];
			PRINT_IQ(dbg_en, "smooth ir_sub = (%d, %d, %d)\r\n", curr_irsub_r[iq_info->id] >> 4, curr_irsub_g[iq_info->id] >> 4, curr_irsub_b[iq_info->id] >> 4);
			PRINT_IQ(dbg_en, "smooth irsub_sat = %d\r\n", curr_irsub_sat[iq_info->id] >> 4);
		} else {
			curr_irsub_r[iq_info->id] = iq_param->rgbir->manual_param.irsub_r_weight << 4;
			curr_irsub_g[iq_info->id] = iq_param->rgbir->manual_param.irsub_g_weight << 4;
			curr_irsub_b[iq_info->id] = iq_param->rgbir->manual_param.irsub_b_weight << 4;
			curr_irsub_sat[iq_info->id] = iq_param->rgbir->manual_param.ir_sat_gain << 4;
			PRINT_IQ(dbg_en, "manual ir_sub = (%d, %d, %d)\r\n", curr_irsub_r[iq_info->id] >> 4, curr_irsub_g[iq_info->id] >> 4, curr_irsub_b[iq_info->id] >> 4);
			PRINT_IQ(dbg_en, "manual irsub_sat = %d\r\n", curr_irsub_sat[iq_info->id]  >> 4);
		}
		if (iq_param->rgbir->enable == FALSE) {
			curr_irsub_r[iq_info->id] = 0;
			curr_irsub_g[iq_info->id] = 0;
			curr_irsub_b[iq_info->id] = 0;
			curr_irsub_sat[iq_info->id] = 256;
		}

		// IQ_RGBIR_PARAM setting
		iq_info->final_sie.sie_ca.irsub_r_weight = IQ_CLAMP(curr_irsub_r[iq_info->id] >> 4, 0, 255);
		iq_info->final_sie.sie_ca.irsub_g_weight = IQ_CLAMP(curr_irsub_g[iq_info->id] >> 4, 0, 255);
		iq_info->final_sie.sie_ca.irsub_b_weight = IQ_CLAMP(curr_irsub_b[iq_info->id] >> 4, 0, 255);
		iq_info->final_sie.sie_la.irsub_r_weight = 0;
		iq_info->final_sie.sie_la.irsub_g_weight = 0;
		iq_info->final_sie.sie_la.irsub_b_weight = 0;
		if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGBIR) {
			iq_info->final_sie.sie_la.la_rgb2y1mod = CTL_SIE_STCS_LA_G;
			iq_info->final_sie.sie_la.la_rgb2y2mod = CTL_SIE_STCS_LA_G;
		} else {
			iq_info->final_sie.sie_la.la_rgb2y1mod = CTL_SIE_STCS_LA_RGB;
			iq_info->final_sie.sie_la.la_rgb2y2mod = CTL_SIE_STCS_LA_RGB;
		}
		break;

	case ISP_TRIG_IQ_IPP:
		// IQ_RGBIR_PARAM setting
		ipe_cfa->cfa_ir_sub.ir_sub_r = IQ_CLAMP(curr_irsub_r[iq_info->id] >> 4, 0, 2047);
		ipe_cfa->cfa_ir_sub.ir_sub_g = IQ_CLAMP(curr_irsub_g[iq_info->id] >> 4, 0, 2047);
		ipe_cfa->cfa_ir_sub.ir_sub_b = IQ_CLAMP(curr_irsub_b[iq_info->id] >> 4, 0, 2047);
		ipe_cfa->cfa_ir_sub.ir_sat_gain = IQ_CLAMP(curr_irsub_sat[iq_info->id] >> 4, 0, 1023);
		if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGBIR) {
			ife_wdr->input_bld.bld_sel = CTL_IFE_ISP_INPUT_BLD_GMEAN;
		} else {
			ife_wdr->input_bld.bld_sel = CTL_IFE_ISP_INPUT_BLD_3x3_Y;
		}
		break;

	default:
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "iq_operation_rgbir get unknow msg(%d) \r\n", msg);
		break;
	}
}

void iq_operation_cgain(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param)
{
	UINT32 iso_idx = 0, ob_ratio;
	UINT32 i;

	if (msg != ISP_TRIG_IQ_IPP) {
		return;
	}

	if (iq_info->final_ipp.sync_info.gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}
	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (iq_info->final_ipp.sync_info.gain < iso_map_tab[i].iso) {
			if (iq_info->final_ipp.sync_info.gain < ((iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2)) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->ob->mode == IQ_OP_TYPE_AUTO) {
		ob_ratio = (4096 - ((iq_param->ob->auto_param[iso_idx].cofs[1] + iq_param->ob->auto_param[iso_idx].cofs[2]) >> 1));
	} else {
		ob_ratio = (4096 - ((iq_param->ob->manual_param.cofs[1] + iq_param->ob->manual_param.cofs[2]) >> 1));
	}

	if ((iq_info->ipp_trig_obj.reset) || (iq_info->ipp_trig_obj.ipp_capture == TRUE)) {
		iq_info->final_ipp.sync_info.cgain[0] = IQ_CLAMP(((iq_info->final_ipp.sync_info.cgain[0] << 12) + (ob_ratio - 1)) / ob_ratio, 0, 2047);
		iq_info->final_ipp.sync_info.cgain[1] = IQ_CLAMP(((iq_info->final_ipp.sync_info.cgain[1] << 12) + (ob_ratio - 1)) / ob_ratio, 0, 2047);
		iq_info->final_ipp.sync_info.cgain[2] = IQ_CLAMP(((iq_info->final_ipp.sync_info.cgain[2] << 12) + (ob_ratio - 1)) / ob_ratio, 0, 2047);
	}
}

void iq_operation_sie_bypass(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param)
{
	switch ((UINT32)msg) {
	case ISP_TRIG_IQ_SIE:
		iq_info->final_sie.sie_ob.bypass_enable = TRUE;
		iq_info->final_sie.sie_ca.enable = FALSE;
		iq_info->final_sie.sie_la.enable = FALSE;
		iq_info->final_sie.sie_cgain.enable = FALSE;
		if ((iq_info->dpc_en == TRUE) && (iq_info->final_sie.sie_dpc != NULL)) {
			iq_info->final_sie.sie_dpc->enable = FALSE;
		}
		if ((iq_info->ecs_en == TRUE) && (iq_info->final_sie.sie_ecs != NULL)) {
			iq_info->final_sie.sie_ecs->enable = FALSE;
		}
		iq_info->final_sie.sie_companding.enable = FALSE;
		iq_info->final_sie.sie_dgain.enable = FALSE;
		iq_info->final_sie.sie_dgain.enable_stcs = FALSE;
		break;

	default:
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "iq_operation_sie_bypass get unknow msg(%d) \r\n", msg);
		break;
	}

}

#define IQ_ECS_SMOOTH_FACTOR 15
#define IQ_ECS_CT_TOLERANCE 100
void iq_operation_ecs(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag)
{
	IQ_SHADING_EXT_PARAM *ecs_ext_auto;
	IQ_SYNC_INFO iq_sync_info;
	UINT32 ct_diff = 0;
	UINT32 h_data, l_data, m_data;
	UINT32 h_rgain, h_ggain, h_bgain;
	UINT32 m_rgain, m_ggain, m_bgain;
	UINT32 l_rgain, l_ggain, l_bgain;
	UINT32 rgain, ggain, bgain;
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_O_ECS;
	UINT32 i, j;
	static UINT32 smooth_ct[IQ_ID_MAX_NUM];

	if (do_flag == FALSE) {
		return;
	}
	if ((iq_info->ecs_en != TRUE) || (iq_info->final_sie.sie_ecs == NULL)) {
		return;
	}

	ecs_ext_auto = &iq_info->iq_ref_set.ecs_ext->auto_tbl;
	if ((iq_info->final_sie.sie_ecs == NULL) || (iq_info->iq_ref_set.ecs_ext == NULL)) {
		return;
	}
	iq_sync_info = iq_info->final_sie.sync_info;

	if (iq_info->sie_trig_obj.reset == TRUE) {
		smooth_ct[iq_info->id] = iq_sync_info.ct;
	} else {
		if (smooth_ct[iq_info->id] <= iq_sync_info.ct) {
			ct_diff = iq_sync_info.ct - smooth_ct[iq_info->id];
		} else {
			ct_diff = smooth_ct[iq_info->id] - iq_sync_info.ct;
		}

		if (ct_diff > IQ_ECS_CT_TOLERANCE) {
			if (smooth_ct[iq_info->id] <= iq_sync_info.ct) {
				smooth_ct[iq_info->id] = (smooth_ct[iq_info->id] * IQ_ECS_SMOOTH_FACTOR + iq_sync_info.ct * 1 + IQ_ECS_SMOOTH_FACTOR) / (IQ_ECS_SMOOTH_FACTOR + 1); // Unconditional carry
			} else {
				smooth_ct[iq_info->id] = (smooth_ct[iq_info->id] * IQ_ECS_SMOOTH_FACTOR + iq_sync_info.ct * 1) / (IQ_ECS_SMOOTH_FACTOR + 1); // Unconditional chop
			}
		}
	}

	// IQ_SHADING_EXT_PARAM setting

	if (iq_info->iq_ref_set.ecs_mode == IQ_OP_TYPE_AUTO) {
		PRINT_IQ(dbg_mode, "IQ(%d) smooth_ct(%d) cur_ct(%d) ct_diff(%d) \r\n", iq_info->id, smooth_ct[iq_info->id], iq_sync_info.ct, ct_diff);
		for (j = 0; j < IQ_SHADING_ECS_WIN; j++) {
			for (i = 0; i < IQ_SHADING_ECS_WIN; i++) {
				h_data = ecs_ext_auto->ecs_map_tbl[IQ_ECS_TEMPERATURE_H][j * IQ_SHADING_ECS_WIN + i];
				m_data = ecs_ext_auto->ecs_map_tbl[IQ_ECS_TEMPERATURE_M][j * IQ_SHADING_ECS_WIN + i];
				l_data = ecs_ext_auto->ecs_map_tbl[IQ_ECS_TEMPERATURE_L][j * IQ_SHADING_ECS_WIN + i];
				h_rgain = (h_data >> 20) & 0x3ff;
				h_ggain = (h_data >> 10) & 0x3ff;
				h_bgain = (h_data) & 0x3ff;
				m_rgain = (m_data >> 20) & 0x3ff;
				m_ggain = (m_data >> 10) & 0x3ff;
				m_bgain = (m_data) & 0x3ff;
				l_rgain = (l_data >> 20) & 0x3ff;
				l_ggain = (l_data >> 10) & 0x3ff;
				l_bgain = (l_data) & 0x3ff;
				if ((smooth_ct[iq_info->id] < iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper) && (smooth_ct[iq_info->id] != 0)) {
					rgain = iq_intpl(smooth_ct[iq_info->id], l_rgain, m_rgain, iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower, iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper);
					ggain = iq_intpl(smooth_ct[iq_info->id], l_ggain, m_ggain, iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower, iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper);
					bgain = iq_intpl(smooth_ct[iq_info->id], l_bgain, m_bgain, iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower, iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper);
				} else if ((smooth_ct[iq_info->id] >= iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper) && (smooth_ct[iq_info->id] != 0)) {
					rgain = iq_intpl(smooth_ct[iq_info->id], m_rgain, h_rgain, iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower, iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper);
					ggain = iq_intpl(smooth_ct[iq_info->id], m_ggain, h_ggain, iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower, iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper);
					bgain = iq_intpl(smooth_ct[iq_info->id], m_bgain, h_bgain, iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower, iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper);
				} else {
					rgain = h_rgain;
					ggain = h_ggain;
					bgain = h_bgain;
				}
				iq_info->final_sie.sie_ecs->map_tbl[j * IQ_SHADING_ECS_WIN + i] = (bgain & 0x3FF) | ((ggain & 0x3FF) << 10) | ((rgain & 0x3FF) << 20);
				if (j * IQ_SHADING_ECS_WIN + i == 0) {
					PRINT_IQ(dbg_mode, "IQ(%d) smooth_ct(%d) ECS(0x%x) (%d, %d, %d) \r\n", iq_info->id, smooth_ct[iq_info->id], iq_info->final_sie.sie_ecs->map_tbl[j * IQ_SHADING_ECS_WIN + i], rgain, ggain, bgain);
				}
			}
		}
	} else {
		memcpy(iq_info->final_sie.sie_ecs->map_tbl, iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl, sizeof(UINT32) * IQ_SHADING_ECS_LEN);
	}
}

void iq_operation_color(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag)
{
	IQ_3DCC_EXT_PARAM *_3dcc_ext_auto;
	IQ_SYNC_INFO iq_sync_info;
	UINT32 color_idx_l = 0, color_idx_h = 0;
	UINT32 color_start = 0, color_end = 0;
	BOOL dbg_en = iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_CCM;
	CTL_IPE_ISP_CCM                  *ipe_ccm = &iq_info->final_ipp.ipe_ccm;
	CTL_IPE_ISP_CCTRL                *ipe_cctrl = &iq_info->final_ipp.ipe_cctrl;
	CTL_IPE_ISP_3DCC                 *ipe_3dcc = iq_info->final_ipp.ipe_3dcc;
	IQ_CCM_MANUAL_PARAM              *ccm_m;
	IQ_CCM_AUTO_PARAM                *ccm_l, *ccm_h;
	UINT32 i;

	if (do_flag == FALSE) {
		return;
	}

	iq_sync_info = iq_info->final_ipp.sync_info;

	if (iq_sync_info.ct <= iq_param->ccm->auto_param[IQ_COLOR_TEMPERATURE_4].ct) {
		color_idx_l = IQ_COLOR_TEMPERATURE_4;
		color_idx_h = IQ_COLOR_TEMPERATURE_4;
		color_start = iq_param->ccm->auto_param[IQ_COLOR_TEMPERATURE_4].ct;
		color_end = iq_param->ccm->auto_param[IQ_COLOR_TEMPERATURE_4].ct;
	}
	for (i = 1; i < IQ_COLOR_ID_MAX_NUM; i++) {
		if (iq_sync_info.ct > iq_param->ccm->auto_param[i].ct) {
			color_idx_l = i;
			color_idx_h = i-1;
			color_start = iq_param->ccm->auto_param[i].ct;
			color_end = iq_param->ccm->auto_param[i-1].ct;
			break;
		}
	}
	PRINT_IQ(dbg_en, "CT: %dK, idx_l: %d(%dK), idx_h: %d(%dK)\r\n", iq_sync_info.ct, color_idx_l, iq_param->ccm->auto_param[color_idx_l].ct, color_idx_h, iq_param->ccm->auto_param[color_idx_h].ct);

	// IQ_CCM_PARAM setting
	PRINT_IQ_VAR(dbg_en, iq_param->ccm->enable);
	if (iq_param->ccm->enable == TRUE) {
		PRINT_IQ_VAR(dbg_en, iq_param->ccm->mode);
		if (iq_param->ccm->mode == IQ_OP_TYPE_AUTO) {
			ccm_l = &iq_param->ccm->auto_param[color_idx_l];
			ccm_h = &iq_param->ccm->auto_param[color_idx_h];

			iq_intpl_tbl_cc(ccm_l->coef, ccm_h->coef, color_start, color_end, iq_sync_info.ct, ipe_ccm->coef);
			iq_intpl_tbl_uint8(ccm_l->hue_tab, ccm_h->hue_tab, CTL_IPE_ISP_CCTRL_TAB_LEN, ipe_cctrl->hue_tab, iq_sync_info.ct, color_start, color_end);
			for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
				ipe_cctrl->sat_tab[i] = (INT8)iq_intpl(iq_sync_info.ct, ccm_l->sat_tab[i], ccm_h->sat_tab[i], color_start, color_end);
			}
			for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
				ipe_cctrl->int_tab[i] = (INT8)iq_intpl(iq_sync_info.ct, ccm_l->int_tab[i], ccm_h->int_tab[i], color_start, color_end);
			}
		} else {
			ccm_m = &iq_param->ccm->manual_param;

			memcpy(ipe_ccm->coef, ccm_m->coef, sizeof(INT16) * CTL_IPE_ISP_COEF_LEN);
			memcpy(ipe_cctrl->hue_tab, ccm_m->hue_tab, sizeof(UINT8) * CTL_IPE_ISP_CCTRL_TAB_LEN);
			memcpy(ipe_cctrl->sat_tab, ccm_m->sat_tab, sizeof(INT8) * CTL_IPE_ISP_CCTRL_TAB_LEN);
			memcpy(ipe_cctrl->int_tab, ccm_m->int_tab, sizeof(INT8) * CTL_IPE_ISP_CCTRL_TAB_LEN);
		}
	} else {
		memcpy(ipe_ccm->coef, ctl_ipe_ccm_init.coef, sizeof(INT16) * CTL_IPE_ISP_COEF_LEN);
		memcpy(ipe_cctrl->hue_tab, ctl_ipe_cctrl_init.hue_tab, sizeof(UINT8) * CTL_IPE_ISP_CCTRL_TAB_LEN);
		memcpy(ipe_cctrl->sat_tab, ctl_ipe_cctrl_init.sat_tab, sizeof(INT8) * CTL_IPE_ISP_CCTRL_TAB_LEN);
		memcpy(ipe_cctrl->int_tab, ctl_ipe_cctrl_init.int_tab, sizeof(INT8) * CTL_IPE_ISP_CCTRL_TAB_LEN);
	}

	// IFE subisp
	memcpy(iq_info->final_ipp.ife_subisp.coef, ipe_ccm->coef, sizeof(INT16) * CTL_IFE_ISP_SUBISP_COEF_LEN);

	// IPE 3DCC
	if ((ipe_3dcc == NULL) || (ipe_3dcc->enable != TRUE)) {
		return;
	}

	_3dcc_ext_auto = &iq_info->iq_ref_set._3dcc_ext->auto_lut;
	if (iq_param->ccm->mode == IQ_OP_TYPE_AUTO) {
		iq_intpl_tbl_3dcc(_3dcc_ext_auto->auto_3dcc_lut[color_idx_l], _3dcc_ext_auto->auto_3dcc_lut[color_idx_h], color_start, color_end, iq_sync_info.ct, ipe_3dcc->lut.rgb_3d_lut);
	} else {
		memcpy(ipe_3dcc->lut.rgb_3d_lut, iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut, sizeof(UINT32) * IQ_3DCC_LEN);
	}
}

void iq_operation_tonecurve(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag)
{
	IQ_SYNC_INFO iq_sync_info;
	UINT32 curr_tone_level;
	UINT32 tone_idx_l = 0, tone_idx_h = 0;
	UINT32 tone_start = 0, tone_end = 0;
	UINT32 i;
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_GAMMA;
	IQ_TONE_AUTO_PARAM              *tone_l, *tone_h;
	CTL_IFE_ISP_WDR_NEQ_TABLE       *ife_wdr_tonecurve = &iq_info->final_ipp.ife_wdr.tonecurve;

	if (do_flag == FALSE) {
		return;
	}

	iq_sync_info = iq_info->final_ipp.sync_info;

	PRINT_IQ(dbg_mode, "==== iq_operation_tonecurve ==== \r\n");
	if (iq_param->tone->mode == IQ_OP_TYPE_AUTO) {
		PRINT_IQ_VAR(dbg_mode, iq_param->tone->auto_sel);
		if (iq_param->tone->auto_sel == IQ_TONE_AUTO_BY_AE_LV) {
			if (iq_sync_info.lv <= (iq_param->tone->auto_param[IQ_TONE_SET4].lv * iq_info->final_ipp.sync_info.lv_base)) {
				tone_idx_l = IQ_TONE_SET4;
				tone_idx_h = IQ_TONE_SET4;
				tone_start = iq_param->tone->auto_param[IQ_TONE_SET4].lv * 100;
				tone_end = iq_param->tone->auto_param[IQ_TONE_SET4].lv * 100;
			}
			for (i = 1; i < IQ_TONE_ID_MAX_NUM; i++) {
				if (iq_sync_info.lv > (iq_param->tone->auto_param[i].lv * iq_info->final_ipp.sync_info.lv_base)) {
					tone_idx_l = i;
					tone_idx_h = i - 1;
					tone_start = iq_param->tone->auto_param[i].lv * 100;
					tone_end = iq_param->tone->auto_param[i - 1].lv * 100;
					break;
				}
			}

			tone_l = &iq_param->tone->auto_param[tone_idx_l];
			tone_h = &iq_param->tone->auto_param[tone_idx_h];

			curr_tone_level = iq_intpl(iq_sync_info.lv / (iq_info->final_ipp.sync_info.lv_base / 100), tone_l->tone_level, tone_h->tone_level, tone_start, tone_end);
			PRINT_IQ(dbg_mode, "curr_tone_level = %d, lv = %d, idx_l = %d (lv:%d, level:%d), idx_h = %d (lv:%d, level:%d) \r\n"
				, curr_tone_level, iq_sync_info.lv
				, tone_idx_l, tone_l->lv * 100, tone_l->tone_level
				, tone_idx_h, tone_h->lv * 100, tone_h->tone_level);
		} else {
			curr_tone_level = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_TONE_LV);
			PRINT_IQ_VAR(dbg_mode, curr_tone_level);
		}

		if (curr_tone_level <= iq_param->tone->auto_set0_level) {
			memcpy(ife_wdr_tonecurve->left_table, iq_param->tone->auto_set0_lut_left, sizeof(UINT16) * CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM);
			memcpy(ife_wdr_tonecurve->right_table, iq_param->tone->auto_set0_lut_right, sizeof(UINT16) * CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM);
		} else if (curr_tone_level <= iq_param->tone->auto_set1_level) {
			iq_intpl_tbl_uint16(iq_param->tone->auto_set0_lut_left, iq_param->tone->auto_set1_lut_left, CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM, ife_wdr_tonecurve->left_table, curr_tone_level, iq_param->tone->auto_set0_level, iq_param->tone->auto_set1_level);
			iq_intpl_tbl_uint16(iq_param->tone->auto_set0_lut_right, iq_param->tone->auto_set1_lut_right, CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM, ife_wdr_tonecurve->right_table, curr_tone_level, iq_param->tone->auto_set0_level, iq_param->tone->auto_set1_level);
		} else if (curr_tone_level <= iq_param->tone->auto_set2_level) {
			iq_intpl_tbl_uint16(iq_param->tone->auto_set1_lut_left, iq_param->tone->auto_set2_lut_left, CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM, ife_wdr_tonecurve->left_table, curr_tone_level, iq_param->tone->auto_set1_level, iq_param->tone->auto_set2_level);
			iq_intpl_tbl_uint16(iq_param->tone->auto_set1_lut_right, iq_param->tone->auto_set2_lut_right, CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM, ife_wdr_tonecurve->right_table, curr_tone_level, iq_param->tone->auto_set1_level, iq_param->tone->auto_set2_level);
		} else if (curr_tone_level <= iq_param->tone->auto_set3_level) {
			iq_intpl_tbl_uint16(iq_param->tone->auto_set2_lut_left, iq_param->tone->auto_set3_lut_left, CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM, ife_wdr_tonecurve->left_table, curr_tone_level, iq_param->tone->auto_set2_level, iq_param->tone->auto_set3_level);
			iq_intpl_tbl_uint16(iq_param->tone->auto_set2_lut_right, iq_param->tone->auto_set3_lut_right, CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM, ife_wdr_tonecurve->right_table, curr_tone_level, iq_param->tone->auto_set2_level, iq_param->tone->auto_set3_level);
		} else if (curr_tone_level <= iq_param->tone->auto_set4_level) {
			iq_intpl_tbl_uint16(iq_param->tone->auto_set3_lut_left, iq_param->tone->auto_set4_lut_left, CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM, ife_wdr_tonecurve->left_table, curr_tone_level, iq_param->tone->auto_set3_level, iq_param->tone->auto_set4_level);
			iq_intpl_tbl_uint16(iq_param->tone->auto_set3_lut_right, iq_param->tone->auto_set4_lut_right, CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM, ife_wdr_tonecurve->right_table, curr_tone_level, iq_param->tone->auto_set3_level, iq_param->tone->auto_set4_level);
		} else {
			memcpy(ife_wdr_tonecurve->left_table, iq_param->tone->auto_set4_lut_left, sizeof(UINT16) * CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM);
			memcpy(ife_wdr_tonecurve->right_table, iq_param->tone->auto_set4_lut_right, sizeof(UINT16) * CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM);
		}
	} else {
		memcpy(ife_wdr_tonecurve->left_table, iq_param->tone->manual_lut_left, sizeof(UINT16) * CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM);
		memcpy(ife_wdr_tonecurve->right_table, iq_param->tone->manual_lut_right, sizeof(UINT16) * CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM);
	}

	PRINT_IQ_ARR(dbg_mode, ife_wdr_tonecurve->left_table, CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM);
	PRINT_IQ_ARR(dbg_mode, ife_wdr_tonecurve->right_table, CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM);
}

void iq_operation_gamma(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag)
{
	IQ_SYNC_INFO iq_sync_info;
	UINT32 ui_gamma_lv = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_GAMMA_LV);
	UINT32 gamma_idx_l = 0, gamma_idx_h = 0;
	UINT32 curr_gamma_level;
	UINT32 curr_gamma[IQ_GAMMA_LEN] = {0};
	UINT32 x_ofs_tone_l, x_ofs_tone_r;
	UINT32 idx_tone, th_pre_gamma, th_post_gamma;
	CTL_IPE_ISP_GAMMA                *ipe_gamma = &iq_info->final_ipp.ipe_gamma;
	IQ_GAMMA_AUTO_PARAM              *gamma_l, *gamma_h;
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_GAMMA;
	UINT32 i;

	if (do_flag == FALSE) {
		return;
	}

	iq_sync_info = iq_info->final_ipp.sync_info;

	// GAMMA setting
	PRINT_IQ(dbg_mode, "==== iq_operation_gamma ==== \r\n");
	PRINT_IQ_VAR(dbg_mode, iq_param->gamma->enable);
	if (ipe_gamma->enable == TRUE) {
		PRINT_IQ_VAR(dbg_mode, iq_param->gamma->mode);
		if (iq_param->gamma->mode == IQ_OP_TYPE_AUTO) {
			PRINT_IQ_VAR(dbg_mode, iq_param->gamma->auto_sel);
			if (iq_param->gamma->auto_sel == IQ_GAMMA_AUTO_BY_AE_LV) {
				if (iq_sync_info.lv <= (iq_param->gamma->auto_param[IQ_GAMMA_SET4].lv * iq_info->final_ipp.sync_info.lv_base)) {
					gamma_idx_l = IQ_GAMMA_SET4;
					gamma_idx_h = IQ_GAMMA_SET4;
				}
				for (i = 1; i < IQ_GAMMA_ID_MAX_NUM; i++) {
					if (iq_sync_info.lv > (iq_param->gamma->auto_param[i].lv * iq_info->final_ipp.sync_info.lv_base)) {
						gamma_idx_l = i;
						gamma_idx_h = i - 1;
						break;
					}
				}

				gamma_l = &iq_param->gamma->auto_param[gamma_idx_l];
				gamma_h = &iq_param->gamma->auto_param[gamma_idx_h];

				curr_gamma_level = iq_intpl(iq_sync_info.lv / (iq_info->final_ipp.sync_info.lv_base / 100), gamma_l->gamma_level, gamma_h->gamma_level, gamma_l->lv * 100, gamma_h->lv * 100);
				PRINT_IQ(dbg_mode, "curr_gamma_level = %d, lv = %d, idx_l = %d (lv:%d, level:%d), idx_h = %d (lv:%d, level:%d) \r\n"
					, curr_gamma_level, iq_sync_info.lv
					, gamma_idx_l, gamma_l->lv * 100, gamma_l->gamma_level
					, gamma_idx_h, gamma_h->lv * 100, gamma_h->gamma_level);
			} else {
				curr_gamma_level = ui_gamma_lv;
				PRINT_IQ_VAR(dbg_mode, ui_gamma_lv);
			}

			PRINT_IQ_VAR(dbg_mode, curr_gamma_level);
			if (curr_gamma_level <= iq_param->gamma->auto_set0_level) {
				memcpy(curr_gamma, iq_param->gamma->auto_set0_lut, sizeof(UINT32) * IQ_GAMMA_LEN);
			} else if (curr_gamma_level <= iq_param->gamma->auto_set1_level) {
				iq_intpl_tbl_gamma(iq_param->gamma->auto_set0_lut, iq_param->gamma->auto_set1_lut, iq_param->gamma->auto_set0_level, iq_param->gamma->auto_set1_level, curr_gamma_level, curr_gamma);
			} else if (curr_gamma_level <= iq_param->gamma->auto_set2_level) {
				iq_intpl_tbl_gamma(iq_param->gamma->auto_set1_lut, iq_param->gamma->auto_set2_lut, iq_param->gamma->auto_set1_level, iq_param->gamma->auto_set2_level, curr_gamma_level, curr_gamma);
			} else if (curr_gamma_level <= iq_param->gamma->auto_set3_level) {
				iq_intpl_tbl_gamma(iq_param->gamma->auto_set2_lut, iq_param->gamma->auto_set3_lut, iq_param->gamma->auto_set2_level, iq_param->gamma->auto_set3_level, curr_gamma_level, curr_gamma);
			} else if (curr_gamma_level <= iq_param->gamma->auto_set4_level) {
				iq_intpl_tbl_gamma(iq_param->gamma->auto_set3_lut, iq_param->gamma->auto_set4_lut, iq_param->gamma->auto_set3_level, iq_param->gamma->auto_set4_level, curr_gamma_level, curr_gamma);
			} else {
				memcpy(curr_gamma, iq_param->gamma->auto_set4_lut, sizeof(UINT32) * IQ_GAMMA_LEN);
			}
		} else {
			memcpy(curr_gamma, iq_param->gamma->manual_lut, sizeof(UINT32) * IQ_GAMMA_LEN);
		}
		memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_R], curr_gamma, sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
		memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_G], curr_gamma, sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
		memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_B], curr_gamma, sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
	} else {
		for (i = 0; i < IQ_GAMMA_LEN; i++) {
			curr_gamma[i] = IQ_CLAMP(i << 3, 0, 1023);
		}
	}

	// EDGE GAMMA setting
	if ((iq_info->ipp_flow_mode == IQ_FLOW_SHDR) && (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_SHDR) && (iq_param->gamma->enable == FALSE)) {
		// SHDR Mode and ToneCurve enable, set inverse tone_curve as edge_gamma
		#if 0
		iq_lib_gamma_detone(iq_info->final_ipp.ife_wdr.tonecurve.left_table, iq_info->final_ipp.ife_wdr.tonecurve.right_table, curr_gamma);
		iq_gamma_down_sample2(curr_gamma, iq_info->final_ipp.ipe_eext_tonemap.tone_map_lut);
		#else
		memcpy(iq_info->final_ipp.ipe_eext_tonemap.tone_map_lut, de_gamma, sizeof(UINT16) * CTL_IPE_ISP_TONE_MAP_LUT_LEN);
		#endif
		th_pre_gamma = iq_param->edge->blending_th << 4;
		x_ofs_tone_l = 2048 / (IQ_TONE_LEFT_NUM - 1);    // 32
		x_ofs_tone_r = 2048 / (IQ_TONE_RIGHT_NUM - 1);   // 128
		if (th_pre_gamma <= 2048) {
			idx_tone = IQ_CLAMP(th_pre_gamma / x_ofs_tone_l, 0, IQ_TONE_LEFT_NUM - 2);
			th_post_gamma = iq_intpl(th_pre_gamma, iq_info->final_ipp.ife_wdr.tonecurve.left_table[idx_tone], iq_info->final_ipp.ife_wdr.tonecurve.left_table[idx_tone + 1], x_ofs_tone_l * idx_tone, x_ofs_tone_l * (idx_tone + 1));
		} else {
			idx_tone = IQ_CLAMP((th_pre_gamma - 2048) / x_ofs_tone_r, 0, IQ_TONE_RIGHT_NUM - 2);
			th_post_gamma = iq_intpl(th_pre_gamma, iq_info->final_ipp.ife_wdr.tonecurve.right_table[idx_tone], iq_info->final_ipp.ife_wdr.tonecurve.right_table[idx_tone + 1], 2048 + x_ofs_tone_r * idx_tone, 2048 + x_ofs_tone_r * (idx_tone + 1));
		}

		iq_info->final_ipp.ipe_eext_tonemap.gamma_sel = CTL_IPE_ISP_EEXT_PRE_GAM;
		iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_th = th_post_gamma >> 4;
		iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w1 = 255 - iq_param->edge->blending_low_luma_w;
		iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w2 = 255 - iq_param->edge->blending_high_luma_w;
		if (iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w1 >= iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w2) {
			iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_slope = 8;
		} else {
			iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_slope = -8;
		}
	} else {
		// SHDR Mode and ToneCurve disable, set gamma_curve as edge_gamma
		// Linear Mode, set gamma_curve as edge_gamma

		iq_gamma_down_sample2(curr_gamma, iq_info->final_ipp.ipe_eext_tonemap.tone_map_lut);

		iq_info->final_ipp.ipe_eext_tonemap.gamma_sel = CTL_IPE_ISP_EEXT_POST_GAM;
		iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_th = iq_param->edge->blending_th;
		iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w1 = iq_param->edge->blending_low_luma_w;
		iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w2 = iq_param->edge->blending_high_luma_w;
		if (iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w1 >= iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w2) {
			iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_slope = 8;
		} else {
			iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_slope = -8;
		}
	}

	// SUB-ISP gamma
	iq_gamma_down_sample8(curr_gamma, iq_info->final_ipp.ife_subisp.subisp_gamma_lut);
}

void iq_operation_effect(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag)
{
	IQ_SYNC_INFO iq_sync_info = {0};
	UINT32 ui_nr_ratio = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_NR_LV);
	UINT32 ui_3dnr_ratio = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_3DNR_LV);
	UINT32 ui_sharp_ratio = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_SHARPNESS_LV);
	UINT32 ui_sat_ratio = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_SATURATION_LV);
	UINT32 ui_con_ratio = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_CONTRAST_LV);
	UINT32 ui_bright_ratio = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_BRIGHTNESS_LV);
	UINT32 ui_night_mode = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_NIGHT_MODE);
	UINT32 ui_image_effect = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_IMAGEEFFECT);
	UINT32 ui_ccid = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_CCID);
	UINT32 ui_hue_shift = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_HUE_SHIFT);
	UINT32 ui_ycc_format = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_YCC_FORMAT);
	UINT8 con2sat_adj;
	INT16 con2int;
	UINT32 ui_3dnr_ratio_tmp;
	CTL_PRE_ISP_FUSION            *pre_fusion =  &iq_info->final_ipp.pre_fusion;
	CTL_IFE_ISP_FILTER            *ife_filter = &iq_info->final_ipp.ife_filter;
	CTL_IFE_ISP_CGAIN             *ife_cgain =  &iq_info->final_ipp.ife_cgain;
	CTL_IPE_ISP_CCTRL             *ipe_cctrl = &iq_info->final_ipp.ipe_cctrl;
	CTL_IPE_ISP_CADJ_YCCON        *ipe_cadj_yccon = &iq_info->final_ipp.ipe_cadj_yccon;
	CTL_IPE_ISP_GAMMA             *ipe_gamma = &iq_info->final_ipp.ipe_gamma;
	CTL_IPE_ISP_YCURVE            *ipe_ycurve = &iq_info->final_ipp.ipe_ycurve;
	CTL_IPE_ISP_CFA               *ipe_cfa = &iq_info->final_ipp.ipe_cfa;
	CTL_IME_ISP_TMNR              *ime_tmnr = &iq_info->final_ipp.ime_tmnr;
	UINT32 i;

	if (do_flag == FALSE) {
		return;
	}

	switch ((UINT32)msg) {
	case ISP_TRIG_IQ_SIE:
		// IQ_UI_NIGHT_MODE
		if (ui_night_mode == IQ_UI_NIGHT_MODE_ON) {
			if (iq_info->cg_mode == IQ_CG_SIE) {
				iq_info->final_sie.sie_cgain.sel_37_fmt = FALSE;
				iq_info->final_sie.sie_cgain.r_gain = IQ_CLAMP(iq_info->final_sie.final_cgain[1], 0, 1023);
				iq_info->final_sie.sie_cgain.gr_gain = IQ_CLAMP(iq_info->final_sie.final_cgain[1], 0, 1023);
				iq_info->final_sie.sie_cgain.gb_gain = IQ_CLAMP(iq_info->final_sie.final_cgain[1], 0, 1023);
				iq_info->final_sie.sie_cgain.b_gain = IQ_CLAMP(iq_info->final_sie.final_cgain[1], 0, 1023);
				iq_info->final_sie.sie_cgain.ir_gain = IQ_CLAMP(iq_info->final_sie.final_cgain[1], 0, 1023);
			}
		}
		break;

	case ISP_TRIG_IQ_IPP:
		iq_sync_info = iq_info->final_ipp.sync_info;

		ipe_cctrl->int_ofs = iq_param->cst->y_ofs;
		ipe_cctrl->sat_ofs = ctl_ipe_cctrl_init.sat_ofs;

		// IQ_UI_NR_LV
		for (i = 0; i < CTL_IFE_ISP_RANGE_A_TH_NUM; i++) {
			ife_filter->rng_filt_r.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_r.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);

			ife_filter->rng_filt_r_1.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r_1.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_r_1.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r_1.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr_1.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr_1.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr_1.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr_1.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb_1.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb_1.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb_1.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb_1.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b_1.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b_1.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b_1.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b_1.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir_1.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir_1.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir_1.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir_1.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);

			ife_filter->rng_filt_r_2.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r_2.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_r_2.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r_2.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr_2.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr_2.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr_2.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr_2.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb_2.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb_2.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb_2.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb_2.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b_2.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b_2.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b_2.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b_2.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir_2.a_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir_2.a_th[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir_2.b_th[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir_2.b_th[i], 0, 1023, IQ_CAL_MULTIPLY);
		}
		for (i = 0; i < CTL_IFE_ISP_RANGE_A_LUT_SIZE; i++) {
			ife_filter->rng_filt_r.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_r.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);

			ife_filter->rng_filt_r_1.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r_1.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_r_1.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r_1.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr_1.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr_1.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr_1.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr_1.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb_1.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb_1.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb_1.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb_1.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b_1.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b_1.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b_1.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b_1.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir_1.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir_1.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir_1.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir_1.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);

			ife_filter->rng_filt_r_2.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r_2.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_r_2.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_r_2.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr_2.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr_2.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gr_2.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gr_2.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb_2.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb_2.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_gb_2.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_gb_2.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b_2.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b_2.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_b_2.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_b_2.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir_2.a_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir_2.a_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
			ife_filter->rng_filt_ir_2.b_lut[i] = (UINT32)iq_cal(ui_nr_ratio, ife_filter->rng_filt_ir_2.b_lut[i], 0, 1023, IQ_CAL_MULTIPLY);
		}
		ife_filter->clamp.th = (UINT32)iq_cal(200 - ui_nr_ratio, ife_filter->clamp.th, 0, 1023, IQ_CAL_MULTIPLY);

		// IQ_UI_3DNR_LV
		for (i = 0; i < CTL_IME_ISP_TMNR_NR_PRE_FILTER_STRENGTH_TAB; i++) {
			ime_tmnr->nr_param.pre_filter_str[i] = iq_cal(ui_3dnr_ratio, ime_tmnr->nr_param.pre_filter_str[i], 0, 255, IQ_CAL_MULTIPLY);
		}
		ime_tmnr->nr_param.tnr_str[0] = iq_cal(ui_3dnr_ratio, ime_tmnr->nr_param.tnr_str[0], 0, 255, IQ_CAL_MULTIPLY);
		if (ui_3dnr_ratio <= 100) {
			for (i = 0; i < CTL_IME_ISP_TMNR_NR_LUMA_LUT_TAB; i++) {
				ime_tmnr->nr_param.luma_3d_lut[i] = iq_cal(200 - ui_3dnr_ratio, ime_tmnr->nr_param.luma_3d_lut[i], 0, 16 * i, IQ_CAL_INTPL);
			}
		} else {
			for (i = 0; i < CTL_IME_ISP_TMNR_NR_LUMA_LUT_TAB; i++) {
				ui_3dnr_ratio_tmp = IQ_CLAMP(ui_3dnr_ratio * (IQ_3DNR_F4_LUT_LEN - i) / IQ_3DNR_F4_LUT_LEN, 100, 200);
				ime_tmnr->nr_param.luma_3d_lut[i] = iq_cal(200 - ui_3dnr_ratio_tmp, ime_tmnr->nr_param.luma_3d_lut[i], 0, 16 * i, IQ_CAL_INTPL);
			}
		}

		// IQ_UI_SHARPNESS_LV
		iq_info->target_edge_enh_p = (UINT16)iq_cal(ui_sharp_ratio, iq_info->target_edge_enh_p, 0, 1023, IQ_CAL_MULTIPLY);
		iq_info->target_edge_enh_n = (UINT16)iq_cal(ui_sharp_ratio, iq_info->target_edge_enh_n, 0, 1023, IQ_CAL_MULTIPLY);

		// IQ_UI_BRIGHTNESS_LV
		ipe_cctrl->int_ofs = (INT16)iq_cal(ui_bright_ratio, ipe_cctrl->int_ofs, -128, 127, IQ_CAL_INTPL);

		// IQ_UI_CONTRAST_LV
		ipe_cadj_yccon->y_con = (UINT8)iq_cal(ui_con_ratio, ipe_cadj_yccon->y_con, 0, 255, IQ_CAL_INTPL);
		if (ui_con_ratio > 100) {
			con2sat_adj = (UINT8)iq_cal(ui_con_ratio, 0, 0, 18, IQ_CAL_INTPL);
			ipe_cadj_yccon->c_con = (UINT8)IQ_CLAMP(ipe_cadj_yccon->c_con + con2sat_adj, 0, 255);
		} else {
			con2sat_adj = (UINT8)iq_cal(ui_con_ratio, 0, 18, 0, IQ_CAL_INTPL);
			ipe_cadj_yccon->c_con = (UINT8)IQ_CLAMP(ipe_cadj_yccon->c_con - con2sat_adj, 0, 255);
		}
		con2int = 128 - ipe_cadj_yccon->y_con;
		ipe_cctrl->int_ofs = (INT16)IQ_CLAMP(ipe_cctrl->int_ofs + con2int, -128, 127);

		// IQ_UI_SATURATION_LV
		ipe_cadj_yccon->c_con = (UINT8)iq_cal(ui_sat_ratio, ipe_cadj_yccon->c_con, 0, 255, IQ_CAL_INTPL);
		if (ui_sat_ratio == 0) {
			memcpy(ime_tmnr->nr_param.chroma_3d_lut, iq_tmnr_still.nr_param.chroma_3d_lut, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_CHROMA_LUT_TAB);
		}

		// IQ_UI_IMAGEEFFECT
		iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = iq_param->cst->cb_ofs;
		iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = iq_param->cst->cr_ofs;
		if (ui_image_effect == IQ_UI_IMAGEEFFECT_BW) {
			ipe_cadj_yccon->c_con = 0;
			iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = 128;
			iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = 128;
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_SEPIA) {
			ipe_cadj_yccon->c_con = 0;
			iq_info->final_ipp.ipe_cadj_cofs.enable = TRUE;
			iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = 116;
			iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = 144;
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_VIVID) {
			iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = 128;
			iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = 128;
			ipe_cctrl->sat_ofs += 50;
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_ROCK) {
			memcpy(&iq_info->final_ipp.ipe_cadj_fixth, &iq_effect_param_rock, sizeof(CTL_IPE_ISP_CADJ_FIXTH));
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_SKETCH) {
			ipe_cadj_yccon->y_con += 96;
			ipe_cadj_yccon->c_con = 0;
			iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = 128;
			iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = 128;
			ipe_cctrl->sat_ofs += 128;
			iq_info->target_edge_enh_p = 0;
			iq_info->target_edge_enh_n = (UINT16)iq_cal(200, iq_info->target_edge_enh_n, 0, 1023, IQ_CAL_MULTIPLY);
			memcpy(&iq_info->final_ipp.ipe_cadj_fixth, &iq_effect_param_sketch, sizeof(CTL_IPE_ISP_CADJ_FIXTH));
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_COLORPENCIL) {
			ipe_cadj_yccon->y_con += 96;
			iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = 128;
			iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = 128;
			iq_info->target_edge_enh_p = 0;
			iq_info->target_edge_enh_n = (UINT16)iq_cal(200, iq_info->target_edge_enh_n, 0, 1023, IQ_CAL_MULTIPLY);
			memcpy(&iq_info->final_ipp.ipe_cadj_fixth, &iq_effect_param_color_pencil, sizeof(CTL_IPE_ISP_CADJ_FIXTH));
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_COOLGREEN) {
			iq_info->final_ipp.ipe_cadj_cofs.enable = TRUE;
			iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = 128;
			iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = 64;
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_WARMYELLOW) {
			iq_info->final_ipp.ipe_cadj_cofs.enable = TRUE;
			iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = 64;
			iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = 160;
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_CCID) {
			memcpy(&ipe_cctrl->sat_tab, iq_effect_param_select_cc[ui_ccid], sizeof(INT8) * CTL_IPE_ISP_CCTRL_TAB_LEN);
		}
		if (ui_image_effect == IQ_UI_IMAGEEFFECT_NEGATIVE_DEFOG) {
			ipe_gamma->option = CTL_IPE_ISP_GAMMA_RGB_COMBINE;
			memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_R], &iq_effect_param_negative[0], sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
			memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_G], &iq_effect_param_negative[0], sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
			memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_B], &iq_effect_param_negative[0], sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
			iq_gamma_down_sample2(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_G], iq_info->final_ipp.ipe_eext_tonemap.tone_map_lut);

			ipe_ycurve->enable = iq_param->ycurve->enable;
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_THERMAL_RED) {
			ipe_gamma->option = CTL_IPE_ISP_GAMMA_RGB_SEPERATE;
			memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_R], &iq_effect_param_thermal_red[0][0], sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
			memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_G], &iq_effect_param_thermal_red[1][0], sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
			memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_B], &iq_effect_param_thermal_red[2][0], sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
			iq_gamma_down_sample2(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_R], iq_info->final_ipp.ipe_eext_tonemap.tone_map_lut);

			ipe_ycurve->enable = FALSE;
		} else if (ui_image_effect == IQ_UI_IMAGEEFFECT_THERMAL_COLOR) {
			ipe_gamma->option = CTL_IPE_ISP_GAMMA_RGB_SEPERATE;
			memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_R], &iq_effect_param_thermal_color[0][0], sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
			memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_G], &iq_effect_param_thermal_color[1][0], sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
			memcpy(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_B], &iq_effect_param_thermal_color[2][0], sizeof(UINT32) * CTL_IPE_ISP_GAMMA_LEN);
			iq_gamma_down_sample2(ipe_gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_G], iq_info->final_ipp.ipe_eext_tonemap.tone_map_lut);

			ipe_ycurve->enable = FALSE;
		} else {
			ipe_gamma->option = CTL_IPE_ISP_GAMMA_RGB_COMBINE;

			ipe_ycurve->enable = iq_param->ycurve->enable;
		}

		// IQ_UI_HUE
		if (ui_hue_shift != 0) {
			for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
				ipe_cctrl->hue_tab[i] = (UINT32)iq_cal(ui_hue_shift, ipe_cctrl->hue_tab[i], 0, 255, IQ_CAL_HUE);
			}
		}

		// IQ_UI_NIGHT_MODE
		if (ui_night_mode == IQ_UI_NIGHT_MODE_ON) {
			ipe_cadj_yccon->c_con = 0;
			memcpy(iq_info->final_ipp.ipe_ccm.coef, ctl_ipe_ccm_init.coef, sizeof(INT16) * CTL_IPE_ISP_COEF_LEN);
			iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = 128;
			iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = 128;
			if (iq_info->cg_mode == IQ_CG_IFE_F) {
				pre_fusion->fu_cgain.bit_field = CTL_IFE_ISP__8_8;
				pre_fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_R] = iq_sync_info.cgain[1];
				pre_fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_GR] = iq_sync_info.cgain[1];
				pre_fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_GB] = iq_sync_info.cgain[1];
				pre_fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_B] = iq_sync_info.cgain[1];
				pre_fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_IR] = iq_sync_info.cgain[1];
				pre_fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_R] = iq_sync_info.cgain[1];
				pre_fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_GR] = iq_sync_info.cgain[1];
				pre_fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_GB] = iq_sync_info.cgain[1];
				pre_fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_B] = iq_sync_info.cgain[1];
				pre_fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_IR] = iq_sync_info.cgain[1];
			} else if (iq_info->cg_mode == IQ_CG_IFE) {
				ife_cgain->bit_field = CTL_IFE_ISP__2_8;
				ife_cgain->cgain_r = iq_sync_info.cgain[1];
				ife_cgain->cgain_gr = iq_sync_info.cgain[1];
				ife_cgain->cgain_gb = iq_sync_info.cgain[1];
				ife_cgain->cgain_b = iq_sync_info.cgain[1];
				ife_cgain->cgain_ir = iq_sync_info.cgain[1];
			} else if (iq_info->cg_mode == IQ_CG_IPE) {
				ipe_cfa->cfa_cgain.r_gain = iq_sync_info.cgain[1];
				ipe_cfa->cfa_cgain.g_gain = iq_sync_info.cgain[1];
				ipe_cfa->cfa_cgain.b_gain = iq_sync_info.cgain[1];
				ipe_cfa->cfa_cgain.gain_range = CTL_IPE_ISP_CGAIN_2_8;
			}
			memcpy(ime_tmnr->nr_param.chroma_3d_lut, iq_tmnr_still.nr_param.chroma_3d_lut, sizeof(UINT8) * CTL_IME_ISP_TMNR_NR_CHROMA_LUT_TAB);
		}

		// IQ_UI_YCC_FORMAT
		if (ui_ycc_format == IQ_UI_YCC_OUT_FULL) {
			iq_info->final_ipp.ime_ycccvt.enable = FALSE;
		} else if (ui_ycc_format == IQ_UI_YCC_OUT_BT601) {
			iq_info->final_ipp.ime_ycccvt.enable = TRUE;
			iq_info->final_ipp.ime_ycccvt.cvt_sel = CTL_IME_ISP_YCC_CVT_BT601;
		} else if (ui_ycc_format == IQ_UI_YCC_OUT_BT709) {
			iq_info->final_ipp.ime_ycccvt.enable = TRUE;
			iq_info->final_ipp.ime_ycccvt.cvt_sel = CTL_IME_ISP_YCC_CVT_BT709;
		}
		break;

	default:
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "iq_operation_effect get unknow msg(%d) \r\n", msg);
		break;
	}
}

#define IQ_WDR_NNSC_MAX_GAIN 4
#define IQ_DEFOG_NNSC_MAX_GAIN 2
void iq_operation_nnsc(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag)
{
	UINT32 nnsc_dark_enh_ratio = iq_nnsc_get_info(iq_info->id, IQ_NNSC_ITEM_DARK_ENH_RATIO);
	UINT32 nnsc_contrast_enh_ratio = iq_nnsc_get_info(iq_info->id, IQ_NNSC_ITEM_CONTRAST_ENH_RATIO);
	UINT32 nnsc_green_enh_ratio = iq_nnsc_get_info(iq_info->id, IQ_NNSC_ITEM_GREEN_ENH_RATIO);
	UINT32 nnsc_skin_enh_ratio = iq_nnsc_get_info(iq_info->id, IQ_NNSC_ITEM_SKIN_ENH_RATIO);
	BOOL nnsc_dbg_en = ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_NNSC) ? TRUE : FALSE);
	BOOL wdr_dbg_en = nnsc_dbg_en ? TRUE : ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_A_WDR) ? TRUE : FALSE);
	BOOL defog_dbg_en = nnsc_dbg_en ? TRUE : ((iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_A_DEFOG) ? TRUE : FALSE);
	UINT32 target;
	UINT32 i;

	CTL_IPE_ISP_DEFOG             *ipe_defog = &iq_info->final_ipp.ipe_defog;
	CTL_IPE_ISP_CCTRL             *ipe_cctrl = &iq_info->final_ipp.ipe_cctrl;

	if (do_flag == FALSE) {
		return;
	}

	switch ((UINT32)msg) {
	case ISP_TRIG_IQ_SIE:
		break;

	case ISP_TRIG_IQ_IPP:
		memcpy(ipe_cctrl->edge_tab, iq_param->edge->edge_tab, sizeof(UINT8) * CTL_IPE_ISP_CCTRL_TAB_LEN);

		PRINT_IQ(nnsc_dbg_en, "original setting");
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->hue_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->sat_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->int_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->edge_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);

		PRINT_IQ(nnsc_dbg_en, "NNSC new setting");

		// IQ_NNSC_ITEM_DARK_ENH_RATIO
		if ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_WDR) && (iq_param->wdr->enable)) {
			nnsc_dark_enh_ratio = IQ_CLAMP(nnsc_dark_enh_ratio, IQ_NNSC_DARK_ENH_RATIO_OFF, IQ_NNSC_DARK_ENH_RATIO_MAX);
			iq_info->target_wdr_str = IQ_CLAMP(iq_info->target_wdr_str * (1 + (nnsc_dark_enh_ratio * IQ_WDR_NNSC_MAX_GAIN) / 100), 0, 255);
			PRINT_IQ(wdr_dbg_en, "Final NNSC WDR strength = %d (nnsc_dark_enh_ratio = %d) \r\n", iq_info->target_wdr_str, nnsc_dark_enh_ratio);
		} else {
			PRINT_IQ(wdr_dbg_en, "NNSC BackLight, Please enable WDR to get better image quality. \r\n");
		}

		// IQ_NNSC_ITEM_CONTRAST_ENH_RATIO
		if (nnsc_contrast_enh_ratio != IQ_NNSC_CONTRAST_ENH_RATIO_OFF) {
			if (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_DEFOG) {
				ipe_defog->enable = TRUE;
				nnsc_contrast_enh_ratio = IQ_CLAMP(nnsc_contrast_enh_ratio, IQ_NNSC_CONTRAST_ENH_RATIO_OFF, IQ_NNSC_CONTRAST_ENH_RATIO_MAX);
				ipe_defog->dfg_strength.fog_ratio = ipe_defog->dfg_strength.fog_ratio * (1 + (nnsc_contrast_enh_ratio * IQ_DEFOG_NNSC_MAX_GAIN) / 100);
				PRINT_IQ(defog_dbg_en, "Final NNSC DEFOG fog_ratio = %d (nnsc_contrast_enh_ratio = %d) \r\n", ipe_defog->dfg_strength.fog_ratio, nnsc_contrast_enh_ratio);
			} else {
				PRINT_IQ(defog_dbg_en, "NNSC FOGGY, Please enable DEFOG to get better image quality. \r\n");
			}
		} else {
			ipe_defog->enable = iq_param->defog->enable;
		}

		// IQ_NNSC_ITEM_GREEN_ENH_RATIO
		for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
			if (iq_nnsc_param_enh_green_cctrl_ct.hue_tab[i] < 127) {
				target = IQ_CLAMP(ipe_cctrl->hue_tab[i] + iq_nnsc_param_enh_green_cctrl_ct.hue_tab[i], 0, 255);
			} else {
				target = IQ_CLAMP(iq_nnsc_param_enh_green_cctrl_ct.hue_tab[i] - ipe_cctrl->hue_tab[i], 0, 255);
			}
			ipe_cctrl->hue_tab[i] = (UINT8)iq_intpl(nnsc_green_enh_ratio, ipe_cctrl->hue_tab[i], target, 0, 100);
		}
		for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
			ipe_cctrl->sat_tab[i] = iq_intpl(nnsc_green_enh_ratio, ipe_cctrl->sat_tab[i], IQ_CLAMP(ipe_cctrl->sat_tab[i] + iq_nnsc_param_enh_green_cctrl_ct.sat_tab[i], -128, 127), 0, 100);
		}
		for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
			ipe_cctrl->int_tab[i] = iq_intpl(nnsc_green_enh_ratio, ipe_cctrl->int_tab[i], IQ_CLAMP(ipe_cctrl->int_tab[i] + iq_nnsc_param_enh_green_cctrl_ct.int_tab[i], -128, 127), 0, 100);
		}

		PRINT_IQ(nnsc_dbg_en, "nnsc_green_enh_ratio = %d: \r\n", nnsc_green_enh_ratio);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->hue_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->sat_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->int_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->edge_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);

		// IQ_NNSC_ITEM_SKIN_ENH_RATIO
		if (nnsc_skin_enh_ratio >= IQ_NNSC_SKIN_ENH_RATIO_OFF) {
			for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
				ipe_cctrl->int_tab[i] = iq_intpl(nnsc_skin_enh_ratio, ipe_cctrl->int_tab[i], IQ_CLAMP(ipe_cctrl->int_tab[i] + iq_nnsc_param_enh_skin_white_cctrl_ct.int_tab[i], -128, 127), 50, 100);
			}
			for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
				if (iq_nnsc_param_enh_skin_cctrl.edge_tab[i] < 127) {
					target = IQ_CLAMP(ipe_cctrl->edge_tab[i] + iq_nnsc_param_enh_skin_cctrl.edge_tab[i], 0, 255);
				} else {
					target = IQ_CLAMP(iq_nnsc_param_enh_skin_cctrl.edge_tab[i] - ipe_cctrl->edge_tab[i], 0, 255);
				}
				ipe_cctrl->edge_tab[i] = (UINT8)iq_intpl(nnsc_skin_enh_ratio, ipe_cctrl->edge_tab[i], target, 50, 100);
			}
		} else {
			for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
				ipe_cctrl->int_tab[i] = iq_intpl(nnsc_skin_enh_ratio, IQ_CLAMP(ipe_cctrl->int_tab[i] + iq_nnsc_param_enh_skin_tan_cctrl_ct.int_tab[i], -128, 127), ipe_cctrl->int_tab[i], 0, 50);
			}
			for (i = 0; i < CTL_IPE_ISP_CCTRL_TAB_LEN; i++) {
				if (iq_nnsc_param_enh_skin_cctrl.edge_tab[i] < 127) {
					target = IQ_CLAMP(ipe_cctrl->edge_tab[i] + iq_nnsc_param_enh_skin_cctrl.edge_tab[i], 0, 255);
				} else {
					target = IQ_CLAMP(iq_nnsc_param_enh_skin_cctrl.edge_tab[i] - ipe_cctrl->edge_tab[i], 0, 255);
				}
				ipe_cctrl->edge_tab[i] = (UINT8)iq_intpl(nnsc_skin_enh_ratio, target, ipe_cctrl->edge_tab[i], 0, 50);
			}
		}
		PRINT_IQ(nnsc_dbg_en, "nnsc_skin_enh_ratio = %d: \r\n", nnsc_skin_enh_ratio);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->hue_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->sat_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->int_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		PRINT_IQ_ARR(nnsc_dbg_en, ipe_cctrl->edge_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);

		iq_dbg_clr_dbg_mode(iq_info->id, IQ_DBG_NNSC);
		break;

	default:
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "iq_operation_nnsc get unknow msg(%d) \r\n", msg);
		break;
	}
}

#define IQ_VIG_SMOOTH_FACTOR 15
#define IQ_WDR_SMOOTH_FACTOR 15
#define IQ_EDGE_H_SMOOTH_FACTOR 1
#define IQ_EDGE_L_SMOOTH_FACTOR 15
void iq_operation_smooth(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag)
{
	BOOL smooth_en = (iq_info->ipp_trig_obj.reset != TRUE) && (iq_info->ipp_trig_obj.ipp_capture != TRUE);
	BOOL vig_dbg_en = iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_VIG;
	BOOL smooth_dbg_en = iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_SMOOTH;
	UINT32 i;

	CTL_IFE_ISP_VIG               *ife_vig = &iq_info->final_ipp.ife_vig;
	CTL_IFE_ISP_WDR               *ife_wdr = &iq_info->final_ipp.ife_wdr;
	CTL_IPE_ISP_CADJ_EE           *ipe_cadj_ee = &iq_info->final_ipp.ipe_cadj_ee;

	switch ((UINT32)msg) {
	case ISP_TRIG_IQ_SIE:
		break;

	case ISP_TRIG_IQ_IPP:
		// VIG
		if (smooth_en) {
			PRINT_IQ(vig_dbg_en, "IQ(%d) VIG lut[16] (%d -> %d) \r\n", iq_info->id, ife_vig->ch_gr_lut[16], iq_info->target_shading_vig[16]);
			if (ife_vig->ch_gr_lut[16] <= iq_info->target_shading_vig[16]) {
				for (i = 0; i < 17; i++) {
					ife_vig->ch_gr_lut[i] = (ife_vig->ch_gr_lut[i] * IQ_VIG_SMOOTH_FACTOR + iq_info->target_shading_vig[i] * 1 + IQ_VIG_SMOOTH_FACTOR) / (IQ_VIG_SMOOTH_FACTOR + 1); // Unconditional carry
				}
			} else {
				for (i = 0; i < 17; i++) {
					ife_vig->ch_gr_lut[i] = (ife_vig->ch_gr_lut[i] * IQ_VIG_SMOOTH_FACTOR + iq_info->target_shading_vig[i] * 1) / (IQ_VIG_SMOOTH_FACTOR + 1); // Unconditional chop
				}
			}
		} else {
			memcpy(ife_vig->ch_gr_lut, iq_info->target_shading_vig, sizeof(UINT16) * IQ_SHADING_VIG_LEN);
		}
		PRINT_IQ_ARR(vig_dbg_en, ife_vig->ch_gr_lut, IQ_SHADING_VIG_LEN);
		memcpy(ife_vig->ch_r_lut, ife_vig->ch_gr_lut, sizeof(UINT16) * CTL_IFE_ISP_VIG_CH0_LUT_SIZE);
		memcpy(ife_vig->ch_gb_lut, ife_vig->ch_gr_lut, sizeof(UINT16) * CTL_IFE_ISP_VIG_CH2_LUT_SIZE);
		memcpy(ife_vig->ch_b_lut, ife_vig->ch_gr_lut, sizeof(UINT16) * CTL_IFE_ISP_VIG_CH3_LUT_SIZE);
		memcpy(ife_vig->ch_ir_lut, ife_vig->ch_gr_lut, sizeof(UINT16) * CTL_IFE_ISP_VIG_CH2_LUT_SIZE);


		// WDR
		if ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_WDR) && (iq_param->wdr->enable)) {
			if (smooth_en) {
				PRINT_IQ(smooth_dbg_en, "IQ(%d) WDR strength (%d -> %d) \r\n", iq_info->id, ife_wdr->wdr_str.strength, iq_info->target_wdr_str);
				if (ife_wdr->wdr_str.strength <= iq_info->target_wdr_str) {
					ife_wdr->wdr_str.strength = IQ_CLAMP((ife_wdr->wdr_str.strength * IQ_WDR_SMOOTH_FACTOR + iq_info->target_wdr_str * 1 + IQ_WDR_SMOOTH_FACTOR) / (IQ_WDR_SMOOTH_FACTOR + 1), 0, 255); // Unconditional carry
				} else {
					ife_wdr->wdr_str.strength = IQ_CLAMP((ife_wdr->wdr_str.strength * IQ_WDR_SMOOTH_FACTOR + iq_info->target_wdr_str * 1) / (IQ_WDR_SMOOTH_FACTOR + 1), 0, 255); // Unconditional chop
				}
			} else {
				ife_wdr->wdr_str.strength = IQ_CLAMP(iq_info->target_wdr_str, 0, 255);
			}
			PRINT_IQ(smooth_dbg_en, "IQ(%d) Final WDR strength = %d \r\n", iq_info->id, ife_wdr->wdr_str.strength);
		} else {
			ife_wdr->wdr_str.strength = 0;
		}

		// EDGE
		if (smooth_en) {
			PRINT_IQ(smooth_dbg_en, "IQ(%d) EDGE enh_p(%d -> %d) enh_n(%d -> %d) \r\n", iq_info->id, ipe_cadj_ee->edge_enh_p, iq_info->target_edge_enh_p, ipe_cadj_ee->edge_enh_n, iq_info->target_edge_enh_n);
			if (ipe_cadj_ee->edge_enh_p <= iq_info->target_edge_enh_p) {
				ipe_cadj_ee->edge_enh_p = (ipe_cadj_ee->edge_enh_p * IQ_EDGE_L_SMOOTH_FACTOR + iq_info->target_edge_enh_p * 1 + IQ_EDGE_L_SMOOTH_FACTOR) / (IQ_EDGE_L_SMOOTH_FACTOR + 1); // Unconditional carry
			} else {
				ipe_cadj_ee->edge_enh_p = (ipe_cadj_ee->edge_enh_p * IQ_EDGE_H_SMOOTH_FACTOR + iq_info->target_edge_enh_p * 1) / (IQ_EDGE_H_SMOOTH_FACTOR + 1); // Unconditional chop
			}
			if (ipe_cadj_ee->edge_enh_n <= iq_info->target_edge_enh_n) {
				ipe_cadj_ee->edge_enh_n = (ipe_cadj_ee->edge_enh_n * IQ_EDGE_L_SMOOTH_FACTOR + iq_info->target_edge_enh_n * 1 + IQ_EDGE_L_SMOOTH_FACTOR) / (IQ_EDGE_L_SMOOTH_FACTOR + 1); // Unconditional carry
			} else {
				ipe_cadj_ee->edge_enh_n = (ipe_cadj_ee->edge_enh_n * IQ_EDGE_H_SMOOTH_FACTOR + iq_info->target_edge_enh_n * 1) / (IQ_EDGE_H_SMOOTH_FACTOR + 1); // Unconditional chop
			}
		} else {
			ipe_cadj_ee->edge_enh_p = iq_info->target_edge_enh_p;
			ipe_cadj_ee->edge_enh_n = iq_info->target_edge_enh_n;
		}
		PRINT_IQ(smooth_dbg_en, "IQ(%d) Final Edge enh = (%d, %d) \r\n", iq_info->id, ipe_cadj_ee->edge_enh_p, ipe_cadj_ee->edge_enh_n);

		break;

	case ISP_TRIG_IQ_ENC:
		break;

	default:
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "iq_operation_smooth get unknow msg(%d) \r\n", msg);
		break;
	}
}

