#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/div64.h>
#include <linux/math64.h>
#include <linux/of.h>
#endif

#include <libfdt.h>
#include <fdtfast.h>
#include "kwrap/debug.h"
#include <kwrap/perf.h>

#include "ae_builtin.h"
#include "ae_common.h"

#if (NVT_FAST_ISP_FLOW)
//=============================================================================
// define
//=============================================================================

#define AE_BUILTIN_DBG_MSG    0
#define AE_BUILTIN_DBG_MSG2   0


#define EXPT_MIN_OS04C10    50
#define GAIN_MIN_OS04C10    100
#define EXPT_MAX_OS04C10    33333
#define GAIN_MAX_OS04C10    204800
#define DGAIN_TH_OS04C10    99200

#define FLICKER_BASE    10000
#define LA_X            32
#define LA_Y            32
#define LA_NUM          (LA_X * LA_Y)
#define AE_WIN_X        8
#define AE_WIN_Y        8
#define AE_WIN_NUM      (AE_WIN_X * AE_WIN_Y)

#define TARGET_Y        50
#define TARGET_IR_Y     48

// NOTE: swtich 12/24 to test covergence speed
#define P_FACTOR        24
#define AE_SPEED        128
// NOTE: swtich 10/20 to test covergence speed
#define AE_STABLE_RNG   20
#define AE_ISO_THRES    1600

#define SHDR_EV_RATIO   16

#define AE_FNO                      40

#define SUB_NODE_LENGTH 64
#define AE_MEASURE_ENABLE 0
//=============================================================================
// global
//=============================================================================
#if (AE_MEASURE_ENABLE)
extern UINT32 trig_vd_num;
UINT32 ae_measure[5][100] = {0};
unsigned int __cnt = 0;
unsigned int __sum = 0;
UINT32 begin, diff;
#endif

typedef enum _AE_SHOOT_DIR {
	SHOOT_UP = 0,   // AE shoot up (increase gain)
	SHOOT_DOWN,     // AE shoot down (decrease gain)
} AE_SHOOT_DIR;

static AE_BUILTIN_INFO ae_builtin_info[ISP_BUILTIN_ID_MAX_NUM];
static AE_BUILTIN_SHDR ae_builtin_shdr[ISP_BUILTIN_ID_MAX_NUM];


static AE_BUILTIN_INFO ae_builtin_info_os04c10 = {
	EXPT_MIN_OS04C10, EXPT_MAX_OS04C10,
	GAIN_MIN_OS04C10, GAIN_MAX_OS04C10,
	DGAIN_TH_OS04C10,
	(UINT64)((UINT64)EXPT_MIN_OS04C10*(UINT64)GAIN_MIN_OS04C10), (UINT64)((UINT64)EXPT_MAX_OS04C10*(UINT64)GAIN_MAX_OS04C10)
};

static UINT32 ae_sen_expt[ISP_BUILTIN_ID_MAX_NUM] = {10000, 10000, 10000, 10000, 10000};
static UINT32 ae_sen_gain[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 ae_isp_dgain[ISP_BUILTIN_ID_MAX_NUM] = {128, 128, 128, 128, 128};

static UINT32 ae_meter_matrix[ISP_BUILTIN_ID_MAX_NUM][AE_WIN_NUM] = {
{
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1
},
{
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1
},
{
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1
},
{
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1
},
{
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1
}
};

AE_BUILTIN_FRONT_DTSI ae_front_dtsi = {
	DISABLE,
	50,
	33333
};

static UINT32 log_tab[11] = {0, 107 * AEALG_EV_ACCURACY_BASE, 115 * AEALG_EV_ACCURACY_BASE, 123 * AEALG_EV_ACCURACY_BASE, 132 * AEALG_EV_ACCURACY_BASE, 141 * AEALG_EV_ACCURACY_BASE, 152 * AEALG_EV_ACCURACY_BASE, 162 * AEALG_EV_ACCURACY_BASE, 174 * AEALG_EV_ACCURACY_BASE, 187 * AEALG_EV_ACCURACY_BASE, 200 * AEALG_EV_ACCURACY_BASE};

//=============================================================================
// function declaration
//=============================================================================

//=============================================================================
// external functions
//=============================================================================
static UINT64 ae_builtin_uint64_div(UINT64 numerator, UINT64 denominator)
{
	UINT64 result;

	if (denominator == 0) {
		return 0;
	}

	#ifdef __KERNEL__
	result = div64_u64(numerator, denominator);
	#else
	result = (numerator/denominator);
	#endif

	return result;
}

static UINT32 ae_builtin_op_log2(UINT64 Devider, UINT64 Devident)
{
	UINT64 Ratio;
	UINT64 idx;
	UINT64 integer, fraction;
	UINT64 idx0, idx1;
	UINT64 val0, val1;
	UINT64 ratio_thr;
	UINT64 tmp, tmp0, tmp1;

	if (Devident == 0) {
		return 1;
	}

	integer = 0;

	Ratio = (UINT64)ae_builtin_uint64_div((UINT64)((UINT64)Devider * (UINT64)100), (UINT64)Devident);

	ratio_thr = (UINT64)((UINT64)200 * (UINT64)AEALG_EV_ACCURACY_BASE);

	while (Ratio >= ratio_thr) {
		Ratio = (Ratio>>1);
		integer += AEALG_LV_ACCURACY_BASE;
	}

	for (idx = 1; idx < 11; idx++) {
		if (Ratio < (UINT64)log_tab[idx]) {
			break;
		}
	}

	if (idx > 10) {
		idx = 10;
	}

	idx0 = idx-1;
	idx1 = idx;
	val0 = idx0*AEALG_LV_ACCURACY_BASE;
	val1 = idx1*AEALG_LV_ACCURACY_BASE;

	tmp0 = (UINT64)((val1 - val0) * (UINT64)(Ratio - (UINT64)log_tab[idx0]));
	tmp1 = (UINT64)((UINT64)log_tab[idx1] - (UINT64)log_tab[idx0]);
	tmp = val0 + ae_builtin_uint64_div(tmp0, tmp1);
	fraction = ae_builtin_uint64_div(tmp, 10);

	return (UINT32)(integer + fraction);
}

static UINT64 ae_builtin_cal_ev(UINT64 exptime, UINT64 isogain, UINT32 iso_coef)
{
	UINT64 tmp;

	tmp = 1000000;
	tmp = (tmp * 100);
	tmp = (tmp * AEALG_EV_ACCURACY_BASE);
	tmp = (tmp * iso_coef);

	if (exptime == 0) {
		exptime = 1;
	}

	tmp = (UINT64)ae_builtin_uint64_div((UINT64)tmp, (UINT64)exptime);

	if (isogain == 0) {
		isogain = 1;
	}

	tmp = (UINT64)ae_builtin_uint64_div((UINT64)tmp, (UINT64)isogain);

	tmp = (tmp < 1) ? 1 : tmp;

	return (UINT64)tmp;
}

// TODO: ISP builtin
#if defined(__KERNEL__)
static void ae_builtin_front_dtsi_sub_load(CHAR *node_path, UINT8 *param_ptr, CHAR *node_name, INT32 node_size)
{
	UINT8 *pfdt_addr = (unsigned char *)fdtfast_get_base();
	CHAR sub_node_name[SUB_NODE_LENGTH];
	const void *pfdt_node;
	UINT32 size = 0;
	INT32 node_ofst = 0, data_size;

	sprintf(sub_node_name, "%s/%s", node_path, node_name);
	node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)&sub_node_name);
	if (node_ofst >= 0) {
		pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "size", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			size = *(UINT32 *)pfdt_node;
			if (size != node_size) {
				DBG_DUMP("%s/size mismatch (dtsi:%d, sdk:%d) \r\n", sub_node_name, size, node_size);
				return;
			}
		} else {
			DBG_DUMP("cannot find %s/size \r\n", sub_node_name);
			return;
		}

		pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "data", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			memcpy(param_ptr, pfdt_node, node_size);
		} else {
			DBG_DUMP("cannot find %s/data \r\n", sub_node_name);
		}
	} else {
		DBG_DUMP("cannot find %s \r\n", sub_node_name);
	}
}

void ae_builtin_front_dtsi_load(void)
{
	CHAR node_path[SUB_NODE_LENGTH];

	sprintf(node_path, "/fastboot/front_ae_0");


	ae_builtin_front_dtsi_sub_load(node_path, (UINT8 *)&ae_front_dtsi.enable, "enable", sizeof(ae_front_dtsi.enable));

	ae_builtin_front_dtsi_sub_load(node_path, (UINT8 *)&ae_front_dtsi.sen_expt_min, "sen_expt_min", sizeof(ae_front_dtsi.sen_expt_min));

	ae_builtin_front_dtsi_sub_load(node_path, (UINT8 *)&ae_front_dtsi.sen_expt_max, "sen_expt_max", sizeof(ae_front_dtsi.sen_expt_max));

}
#endif

INT32 ae_builtin_init(UINT32 id)
{
	ISP_BUILTIN_SENSOR_CTRL *p_sensor_ctrl;
	// TODO: ISP builtin, must check here
	#if defined(__KERNEL__)
	UINT32 sen_expt, sen_gain, isp_dgain;
	#else
	UINT32 sen_gain, isp_dgain;
	#endif
	UINT32 isp_total_gain;
	UINT32 sen_fps = 3000;

	// TODO: ISP builtin
	#if defined(__KERNEL__)
	ae_builtin_front_dtsi_load();
	#endif

	switch (isp_builtin_get_sensor_name(id)) {
		default:
		case 8:
			ae_builtin_info[id] = ae_builtin_info_os04c10;
		break;
	}

	sen_fps = isp_builtin_get_chgmode_fps(id);

	if(sen_fps < 1) {
		sen_fps = 1;
	}

	if(ae_front_dtsi.enable == ENABLE) {
		ae_builtin_info[id].expt_min = ae_front_dtsi.sen_expt_min;
		ae_builtin_info[id].expt_max = ae_front_dtsi.sen_expt_max;
	} else {
		ae_builtin_info[id].expt_max = isp_builtin_get_sensor_expt_max(id);
	}

	#if 0
	DBG_DUMP("sen_expt    = %d, %d\n", ae_builtin_info.expt_min, ae_builtin_info.expt_max);
	DBG_DUMP("sen_gain    = %d, %d\n", ae_builtin_info.gain_min, ae_builtin_info.gain_max);
	DBG_DUMP("isp_gain_th = %d\n", ae_builtin_info.isp_gain_th);
	DBG_DUMP("total_gain  = %lld, %lld\n", ae_builtin_info.total_gain_min, ae_builtin_info.total_gain_max);
	#endif
	// shdr parameter
	ae_builtin_shdr[id].enable = isp_builtin_get_shdr_enable(id);
	ae_builtin_shdr[id].frm_num = (ae_builtin_shdr[id].enable == ENABLE) ? 2 : 1;
	ae_builtin_shdr[id].mask = 0x00;
	ae_builtin_shdr[id].le_id = 1;
	ae_builtin_shdr[id].se_id = 0;
	ae_builtin_shdr[id].ev_ratio = SHDR_EV_RATIO;
	ae_builtin_shdr[id].expt_max = ((ae_builtin_info[id].expt_max*ae_builtin_shdr[id].ev_ratio)/(ae_builtin_shdr[id].ev_ratio+1));

	// TODO: ISP builtin, must check here
	#if defined(__KERNEL__)
	p_sensor_ctrl = isp_builtin_get_sensor_expt(id);
	sen_expt = p_sensor_ctrl->exp_time[0];
	#endif
	p_sensor_ctrl = isp_builtin_get_sensor_gain(id);
	sen_gain = p_sensor_ctrl->gain_ratio[0]/10;
	isp_dgain = isp_builtin_get_dgain(id);
	isp_total_gain = ((sen_gain * isp_dgain)>>7);
	isp_builtin_set_total_gain(id, isp_total_gain);

	#if (AE_MEASURE_ENABLE)
	UINT32 i;
	for (i = 0; i < 100; i++) {
		ae_measure[0][i] = 0;
		ae_measure[1][i] = 0;
		ae_measure[2][i] = 0;
		ae_measure[3][i] = 0;
	}
	#endif

	return 0;
}

UINT32 ae_builtin_get_lum(UINT32 id)
{
	ISP_BUILTIN_LA_RSLT *la_rslt;
	UINT32 ix, iy, iyw, win_ofst;
	UINT32 la_raw, raw_total, w_total;
	UINT32 ratio_x, ratio_y;
	UINT32 lum = 0;

	la_rslt = isp_builtin_get_la(id);

	if(la_rslt == NULL) {
		DBG_DUMP("isp_builtin_get_la is NULL\r\n");
		return 0;
	}

	raw_total = 0;
	w_total = 0;
	ratio_x = LA_X / AE_WIN_X;
	ratio_y = LA_Y / AE_WIN_Y;

	for (iy = 0; iy < LA_Y; iy++) {
		iyw = iy * LA_X;
		for (ix = 0; ix < LA_X; ix++) {
			la_raw = (UINT32)la_rslt->lum_1[iyw + ix];
			la_raw = AE_CLAMP(la_raw, 0, 4095);
			win_ofst = (((iy / ratio_y)*AE_WIN_X) + (ix / ratio_x));
			raw_total += (ae_meter_matrix[id][win_ofst] * la_raw);
			w_total += ae_meter_matrix[id][win_ofst];
		}
	}

	lum = ((raw_total / w_total)>>4);
	return lum;
}

INT32 ae_builtin_trig(UINT32 id, ISP_BUILTIN_AE_TRIG_MSG msg)
{
	INT64 total_gain, target_total_gain;
	INT64 curr_total_gain_delta, delta;
	UINT32 sen_expt, sen_gain;
	UINT32 curlum, speed, rem_curlum;
	static UINT32 shoot_dir[ISP_BUILTIN_ID_MAX_NUM] = {SHOOT_DOWN, SHOOT_DOWN, SHOOT_DOWN, SHOOT_DOWN, SHOOT_DOWN};
	INT64 p_part;
	static INT64 prev_target_total_gain[ISP_BUILTIN_ID_MAX_NUM] = {0};
	UINT32 expt, gain, expt_idx;
	UINT32 converge_th_lo, converge_th_hi;
	UINT32 converge_rng_lo, converge_rng_hi;
	static UINT32 first_in[ISP_BUILTIN_ID_MAX_NUM] = {TRUE, TRUE, TRUE, TRUE, TRUE};
	UINT32 lv, isp_total_gain;
	UINT64 ev;
	static UINT32 ae_cnt[ISP_BUILTIN_ID_MAX_NUM] = {0};
	ISP_BUILTIN_SENSOR_CTRL *p_sensor_ctrl;
	ISP_BUILTIN_SENSOR_CTRL sensor_ctrl;
	UINT32 scene_change_w, pfactor;
	UINT32 sen_fps = 3000;
	UINT32 lum_ratio = 100;
	static UINT32 first_stable_flag[ISP_BUILTIN_ID_MAX_NUM] = {FALSE, FALSE, FALSE, FALSE, FALSE};
	UINT32 y_tar;
	static BOOL is_print_stable[ISP_BUILTIN_ID_MAX_NUM];
	static UINT32 first_lum[ISP_BUILTIN_ID_MAX_NUM]= {0};

	#if (AE_MEASURE_ENABLE)
	begin = *(volatile unsigned int *)(0xF0040108);
	#endif

	if (msg == ISP_BUILTIN_AE_TRIG_RESET) {
		return 0;
	}

	ae_cnt[id]++;

	#if defined(__KERNEL__)
	if((ae_cnt[id] >= 2)&&(ae_cnt[id] <= 3)) {
		return 0;
	}
	#endif

	sen_fps = isp_builtin_get_chgmode_fps(id);

	if(sen_fps < 1) {
		sen_fps = 1;
	}

	if(ae_front_dtsi.enable == ENABLE) {
		ae_builtin_info[id].expt_min = ae_front_dtsi.sen_expt_min;
		ae_builtin_info[id].expt_max = ae_front_dtsi.sen_expt_max;
	} else {
		ae_builtin_info[id].expt_max = isp_builtin_get_sensor_expt_max(id);
	}

	if(ae_builtin_shdr[id].enable == ENABLE) {
		curlum = ae_builtin_get_lum(ae_builtin_shdr[id].le_id);
	} else {
		curlum = ae_builtin_get_lum(id);
	}

	curlum = AE_CLAMP(curlum, 1, 255);
	rem_curlum = curlum;

	if(ae_cnt[id] == 1){
		first_lum[id] = rem_curlum;
	}

	if(rem_curlum < 3){
		curlum = 4;
	}else if(rem_curlum < 4){
		curlum = 5;
	}
	else if((rem_curlum >=4) &&(rem_curlum <=7)){
		curlum = 7;
	}

	#if 1
	converge_rng_lo = AE_STABLE_RNG;
	converge_rng_hi = AE_STABLE_RNG;
	#else
	if(shoot_dir[id] == SHOOT_DOWN) {
		converge_rng_lo = (((AE_STABLE_RNG<<1)*1)>>2);
		converge_rng_hi = (((AE_STABLE_RNG<<1)*3)>>2);
	} else {
		converge_rng_lo = (((AE_STABLE_RNG<<1)*3)>>2);
		converge_rng_hi = (((AE_STABLE_RNG<<1)*1)>>2);
	}
	#endif

	if(isp_builtin_get_night_mode(id)==TRUE) {
		y_tar = TARGET_IR_Y;
	} else {
		y_tar = TARGET_Y;
		
		if(first_lum[id] < y_tar){
			y_tar = 40;
		}
		
	}

	converge_th_lo = (y_tar*(100-converge_rng_lo))/100;
	converge_th_hi = (y_tar*(100+converge_rng_hi))/100;

	if((curlum >= converge_th_lo) && (curlum <= converge_th_hi)) {
		first_stable_flag[id] = TRUE;

		if (first_stable_flag[id] && !is_print_stable[id]) {
			is_print_stable[id] = TRUE;
			#if defined(__FREERTOS)
			vos_perf_list_mark("ss", __LINE__, 2);
			#endif
			DBG_DUMP("===== AE_STABLE (%d) (%d) (%d) =====\r\n", id, ae_cnt[id] - 1, curlum);
			#if (AE_MEASURE_ENABLE)
			UINT32 i;
			for (i = 1; i < 20; i++) {
				DBG_DUMP("vd: %2d, cnt: %2d, time: %4d, expt %3d, iso %4d, lum %d \r\n", i , ae_measure[0][i], ae_measure[1][i], ae_measure[2][i] / 1000, ae_measure[3][i] / 10, ae_measure[4][i]);
			}
			#endif
		}

		scene_change_w = 100;//AE_CLAMP((ae_stable_cnt * 10), 0, 100);
		isp_builtin_set_scene_chg_w(id, scene_change_w);
		return 0;
	}

	if(first_in[id]) {
		first_in[id] = FALSE;
		p_sensor_ctrl = isp_builtin_get_sensor_expt(id);
		ae_sen_expt[id] = p_sensor_ctrl->exp_time[0];
		p_sensor_ctrl = isp_builtin_get_sensor_gain(id);
		ae_sen_gain[id] = p_sensor_ctrl->gain_ratio[0]/10;
	}

	ae_isp_dgain[id] = isp_builtin_get_dgain(id);

	sen_expt = ae_sen_expt[id];
	sen_gain = ae_sen_gain[id];

	if(curlum*25/10 <= y_tar){
		y_tar = 35;
	}

	total_gain = (INT64)(((UINT64)sen_expt * (UINT64)sen_gain*(UINT64)ae_isp_dgain[id])>>7);
	target_total_gain = (INT64)ae_builtin_uint64_div((UINT64)((UINT64)total_gain*(UINT64)y_tar), (UINT64)curlum);
	target_total_gain = AE_CLAMP(target_total_gain, ae_builtin_info[id].total_gain_min, ae_builtin_info[id].total_gain_max);

	if (prev_target_total_gain[id] == 0) {
		prev_target_total_gain[id] = total_gain;
	}

	shoot_dir[id] = (curlum > y_tar) ? SHOOT_DOWN : SHOOT_UP;

	if (shoot_dir[id] == SHOOT_UP) {
		pfactor = 28;
	}else{
		pfactor = P_FACTOR;
	}

	curr_total_gain_delta = (INT64)((INT64)target_total_gain - (INT64)total_gain);

	p_part = (curr_total_gain_delta * pfactor) >> 5;

	delta = p_part;

	if (shoot_dir[id] == SHOOT_DOWN) {
		delta = AE_MIN(0, delta);
		speed = AE_SPEED;
	} else {
		delta = AE_MAX(0, delta);
		speed = (AE_SPEED >> 1);
	}

	if (shoot_dir[id] == SHOOT_UP) {
		if(curlum > 15){
			 if (ae_cnt[id] > 2){
				speed = AE_SPEED >> 4;
			}else if (ae_cnt[id] > 1){
				speed = AE_SPEED >> 1;
			}
		}else{
			if(ae_cnt[id] > 3){
				speed = AE_SPEED >> 5;
			}else if (ae_cnt[id] > 2){
				speed = AE_SPEED >> 4;
			}else if (ae_cnt[id] > 1){
				speed = AE_SPEED >> 1;
			}
		}
	}

	delta = (delta * speed) >> 7;

	target_total_gain = (INT64)((INT64)prev_target_total_gain[id] + (INT64)delta);
	target_total_gain = AE_CLAMP(target_total_gain, ae_builtin_info[id].total_gain_min, ae_builtin_info[id].total_gain_max);

	if(target_total_gain < (FLICKER_BASE*ae_builtin_info[id].gain_min)) {
		gain = ae_builtin_info[id].gain_min;
		expt = (INT64)ae_builtin_uint64_div((UINT64)target_total_gain, (UINT64)gain);
	} else {

		expt = sen_expt;
		gain = (UINT32)ae_builtin_uint64_div((UINT64)target_total_gain, (UINT64)sen_expt);

		if(gain <= AE_ISO_THRES) {

			if(gain < ae_builtin_info[id].gain_min) {
				for(expt_idx=sen_expt; expt_idx>=FLICKER_BASE; expt_idx-=FLICKER_BASE) {
					expt = expt_idx;
					gain = (UINT32)ae_builtin_uint64_div((UINT64)target_total_gain, (UINT64)expt);
					if(gain >= ae_builtin_info[id].gain_min) {
						break;
					}
				}
			}

			if(expt < FLICKER_BASE) {
				expt = FLICKER_BASE;
				gain = (UINT32)ae_builtin_uint64_div((UINT64)target_total_gain, (UINT64)sen_expt);
			}
		} else {
			for(expt_idx=FLICKER_BASE; expt_idx<=ae_builtin_info[id].expt_max; expt_idx+=FLICKER_BASE) {
				expt = expt_idx;
				gain = (UINT32)ae_builtin_uint64_div((UINT64)target_total_gain, (UINT64)expt);
				if(gain <= AE_ISO_THRES) {
					break;
				}
			}
		}
	}

	prev_target_total_gain[id] = target_total_gain;

	ae_sen_expt[id] = expt;
	ae_sen_gain[id] = gain;

	if(gain > ae_builtin_info[id].isp_gain_th) {
		ae_isp_dgain[id] = (UINT32)((gain<<7)/ae_builtin_info[id].isp_gain_th);
		ae_sen_gain[id] = ae_builtin_info[id].isp_gain_th;
	} else {
		ae_isp_dgain[id] = 128;
		ae_sen_gain[id] = gain;
	}

	ae_isp_dgain[id] = AE_CLAMP(ae_isp_dgain[id], 128, 1023);

	isp_total_gain = (UINT32)(((UINT64)ae_sen_gain[id] * (UINT64)ae_isp_dgain[id]) >> 7);

	ev = ae_builtin_cal_ev((UINT64)expt, (UINT64)isp_total_gain, (UINT32)AE_FNO);
	lv = ae_builtin_op_log2(ev, 10);

	sensor_ctrl.exp_time[0] = ae_sen_expt[id];
	sensor_ctrl.gain_ratio[0] = ae_sen_gain[id]*10;

	if(ae_builtin_shdr[id].enable == ENABLE) {
		sensor_ctrl.exp_time[0] = AE_CLAMP(sensor_ctrl.exp_time[0], (ae_builtin_info[id].expt_min*ae_builtin_shdr[id].ev_ratio), ae_builtin_shdr[id].expt_max);
		sensor_ctrl.exp_time[1] = (sensor_ctrl.exp_time[0]/ae_builtin_shdr[id].ev_ratio);
		sensor_ctrl.exp_time[1] = AE_CLAMP(sensor_ctrl.exp_time[1], ae_builtin_info[id].expt_min, (ae_builtin_shdr[id].expt_max/ae_builtin_shdr[id].ev_ratio));
		sensor_ctrl.gain_ratio[1] = sensor_ctrl.gain_ratio[0];
	}

	#if AE_BUILTIN_DBG_MSG2
	if(id==0){
		DBG_DUMP("cnt=%d, simple %d exp=%d, gain=%d, isp_dgain=%d \r\n", ae_cnt[id], id, ae_sen_expt[id], ae_sen_gain[id], ae_isp_dgain[id]);

	}else{
		DBG_DUMP("cnt=%d, simple %d exp=%d, gain=%d, isp_dgain=%d \r\n", ae_cnt[id], id, ae_sen_expt[id], ae_sen_gain[id], ae_isp_dgain[id]);
	}
	#endif

	isp_builtin_set_lv(id, lv);
	isp_builtin_set_sensor_expt(id, &sensor_ctrl);
	isp_builtin_set_sensor_gain(id, &sensor_ctrl);
	isp_builtin_set_total_gain(id, isp_total_gain);
	isp_builtin_set_dgain(id, ae_isp_dgain[id]);

	if(first_stable_flag[id] == TRUE) {
		scene_change_w = 100;
	} else {
		if(isp_total_gain >= ae_builtin_info[id].total_gain_max) {
			scene_change_w = 100;
		} else {
			lum_ratio = ((curlum*100)/y_tar);

			if(lum_ratio < 70) {
				scene_change_w = 0;
			} else {
				scene_change_w = lum_ratio;
			}
		}
	}

	scene_change_w = AE_CLAMP(scene_change_w, 0, 100);
	isp_builtin_set_scene_chg_w(id, scene_change_w);
	//DBG_DUMP("ae_cnt[id] =%d, rem_curlum=%d, lum = %d, lv = %d, y_tar = %d, sen_expt = %d, sen_gain = %d, dgain = %d\r\n", ae_cnt[id], rem_curlum,  curlum, lv, y_tar, sensor_ctrl.exp_time[0], sensor_ctrl.gain_ratio[0], ae_isp_dgain[id]);

	#if (AE_MEASURE_ENABLE)
	ae_measure[0][trig_vd_num] = ae_cnt[id];
	diff = (*(volatile unsigned int *)(0xF0040108) - begin);
	ae_measure[1][trig_vd_num] = diff;
	ae_measure[2][trig_vd_num] = ae_sen_expt[id];
	ae_measure[3][trig_vd_num] = ae_sen_gain[id];
	ae_measure[4][trig_vd_num] = ae_builtin_get_lum(id);
	#endif

	#if AE_BUILTIN_DBG_MSG
	//DBG_DUMP("curlum = %d, expt = %d, gain = %d\r\n", ae_builtin_get_lum(id), expt, gain);
	//DBG_DUMP("ev = %lld, lv = %d\r\n", ev, lv);
	DBG_DUMP("lum = %d, lv = %d, y_tar = %d, sen_expt = %d, sen_gain = %d, dgain = %d\r\n", curlum, lv, y_tar, sensor_ctrl.exp_time[0], sensor_ctrl.gain_ratio[0], ae_isp_dgain[id]);
	#endif

	return 0;
}

#endif

