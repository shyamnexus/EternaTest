/**
	@brief Sample code of video liveview with multi videocapture.\n

	@file video_liveview_with_vcap_multi_dev.c

	@author Ben Wang

	@ingroup mhdal

	@note This file is modified from video_liveview_with_vcap_multi_dev.c

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_videocapture.h"
#include "vendor_videoenc.h"
#include "vendor_isp.h"
#include "vendor_common.h"
#include "vendor_ai.h"
#include "aiisp.h"
#include "isp.h"
#include "pd_shm.h"
// platform dependent
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>			//for pthread API
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_liveview_with_vcap_multi_dev, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)


#define AIISP_PARAM_MAX 32
#define AIISP_CONFIG_MAX 6
#define AIISP_GAIN_MAX 16

static IQT_AIISP_PARAM aiisp_pq_final_param_539A[AIISP_CONFIG_MAX] = {

	{
		.aiisp.manual_param.effect = 0,
		.aiisp.manual_param.param[0] = 127,
		.aiisp.manual_param.param[1] = 127,
	},
	{
		.aiisp.manual_param.effect = 0,
		.aiisp.manual_param.param[0] = 0,
		.aiisp.manual_param.param[1] = 0,
	},
	{
		.aiisp.manual_param.effect = 0,
		.aiisp.manual_param.param[0] = 255,
		.aiisp.manual_param.param[1] = 255,
	},

	{
		.aiisp.manual_param.effect = 1,
		.aiisp.manual_param.param[0] = 127,
		.aiisp.manual_param.param[1] = 127,
	},

	{
		.aiisp.manual_param.effect = 1,
		.aiisp.manual_param.param[0] = 0,
		.aiisp.manual_param.param[1] = 0,
	},
	{
		.aiisp.manual_param.effect = 1,
		.aiisp.manual_param.param[0] = 255,
		.aiisp.manual_param.param[1] = 255,
	},

};
static UINT32 last_config = 0;
static UINT32 conti_run = 1;

UINT32 iso_map_tab[AIISP_GAIN_MAX][2] = {
	{    100,      0},
	{    200,      1},
	{    400,      2},
	{    800,      3},
	{   1600,      4},
	{   3200,      5},
	{   6400,      6},
	{  12800,      7},
	{  25600,      8},
	{  51200,      9},
	{ 102400,     10},
	{ 204800,     11},
	{ 409600,     12},
	{ 819200,     13},
	{1638400,     14},
	{3276800,     15}
};

typedef struct _AWB_PQ_PARAM {
	UINT32 target_r_gain[AIISP_GAIN_MAX];
	UINT32 target_b_gain[AIISP_GAIN_MAX];
} AWB_PQ_PARAM;

static AWB_PQ_PARAM awb_pq_param = {
	//                                  1x,   2x,   4x,   8x,  16x,  32x,  64x,  128,  256,  512, 1024, 2048, 4096, 8192,16384,32768
	.target_r_gain                = { 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1070, 1100, 1100, 1100, 1100, 1100, 1100},
	.target_b_gain                = { 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1070, 1100, 1100, 1100, 1100, 1100, 1100},
};

HD_RESULT isp_init(void)
{
	HD_RESULT ret = HD_OK;

	// open MCU device
	if (vendor_isp_init() == HD_ERR_NG) {
		return -1;
	}

	return ret;
}


HD_RESULT isp_init_sensor(SENSOR_INFO *sensor_info, char *chip_name, UINT32 shdr, UINT32 isp_id)
{
	HD_RESULT ret = HD_OK;
	AET_CFG_INFO cfg_info = {0};
	IQT_AIISP_PARAM aiisp = {0};
	IQT_OB_MODE_MANUAL ob_mode_manual = {0};
	IQT_DG_MODE_MANUAL dg_mode_manual = {0};

	cfg_info.id = isp_id;
	if (shdr == 1) {
		if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			snprintf(cfg_info.path, 64, "/mnt/app/isp/%s_hdr_AI_539a.cfg", sensor_info->isp_cfg_name);
		} else {
			snprintf(cfg_info.path, 64, "/mnt/app/isp/%s_hdr_AI.cfg", sensor_info->isp_cfg_name);
		}
	} else {
		if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			snprintf(cfg_info.path, 64, "/mnt/app/isp/%s_AI_539a.cfg", sensor_info->isp_cfg_name);
		} else {
			snprintf(cfg_info.path, 64, "/mnt/app/isp/%s_AI.cfg", sensor_info->isp_cfg_name);
		}
	}
	printf("proc 0 load %s \n", cfg_info.path);

	ob_mode_manual.id = isp_id;
	dg_mode_manual.id = isp_id;

	if (shdr == 0) {
		ob_mode_manual.ob_mode_manual.manual_enable = 1;
		ob_mode_manual.ob_mode_manual.manual_mode = 2;
		dg_mode_manual.dg_mode_manual.manual_enable = 1;
		dg_mode_manual.dg_mode_manual.manual_mode = 2;
	} else {
		ob_mode_manual.ob_mode_manual.manual_enable = 0;
		dg_mode_manual.dg_mode_manual.manual_enable = 0;
	}
	vendor_isp_set_iq(IQT_ITEM_OB_MODE_MANUAL, &ob_mode_manual);
	vendor_isp_set_iq(IQT_ITEM_DG_MODE_MANUAL, &dg_mode_manual);

	ob_mode_manual.id = isp_id;
	dg_mode_manual.id = isp_id;
	vendor_isp_set_iq(IQT_ITEM_OB_MODE_MANUAL, &ob_mode_manual);
	vendor_isp_set_iq(IQT_ITEM_DG_MODE_MANUAL, &dg_mode_manual);

	vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
	vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
	vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);

	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		cfg_info.id = isp_id;
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
	}

	aiisp.id = isp_id;
	vendor_isp_get_iq(IQT_ITEM_AIISP_PARAM, &aiisp);
	aiisp.aiisp.path_id = 0;
	aiisp.aiisp.version = 3;
	aiisp.aiisp.param_num = 2;
	aiisp.aiisp.param_size[0] = 1;
	aiisp.aiisp.param_size[1] = 1;
	sprintf(aiisp.aiisp.param_name[0], "motion_2d_str");
	sprintf(aiisp.aiisp.param_name[1], "still_2d_str");

	vendor_isp_set_iq(IQT_ITEM_AIISP_PARAM, &aiisp);
	//printf("aiisp id = %d \n", aiisp.id);
	//printf("path_id = %d, version = %d, param_num = %d \n", aiisp.aiisp.path_id, aiisp.aiisp.version, aiisp.aiisp.param_num);
	//printf("param_size = %d, %d \n", aiisp.aiisp.param_size[0], aiisp.aiisp.param_size[1]);
	//printf("param_name = %s, %s \n", aiisp.aiisp.param_name[0], aiisp.aiisp.param_name[1]);
	//printf("param_manual.enable = %d \n", aiisp.aiisp.manual_param.enable);
	//printf("param_manual.effect = %d \n", aiisp.aiisp.manual_param.effect);
	//printf("param_manual.param = %d, %d \n", aiisp.aiisp.manual_param.param[0], aiisp.aiisp.manual_param.param[1]);

	return ret;
}

HD_RESULT isp_update_param_name(UINT32 id, MODEL_PARAM_NAME *p_param_name, UINT32 isp_id)
{
	HD_RESULT ret = HD_OK;
	IQT_AIISP_PARAM aiisp = {0};
	UINT32 i;

	aiisp.id = isp_id;
	vendor_isp_get_iq(IQT_ITEM_AIISP_PARAM, &aiisp);
	aiisp.aiisp.path_id = 0;
	aiisp.aiisp.version = 3;
	aiisp.aiisp.param_num = p_param_name->num;

	for (i = 0; i < aiisp.aiisp.param_num; i++) {
		aiisp.aiisp.param_size[i] = p_param_name->param_size[i];
		strcpy(aiisp.aiisp.param_name[i], p_param_name->param_name[i]);
		printf("aiisp param size = %d, name = %s\r\n", aiisp.aiisp.param_size[i], aiisp.aiisp.param_name[i]);
	}

	vendor_isp_set_iq(IQT_ITEM_AIISP_PARAM, &aiisp);
	//printf("aiisp id = %d \n", aiisp.id);
	//printf("path_id = %d, version = %d, param_num = %d \n", aiisp.aiisp.path_id, aiisp.aiisp.version, aiisp.aiisp.param_num);
	//printf("param_size = %d, %d \n", aiisp.aiisp.param_size[0], aiisp.aiisp.param_size[1]);
	//printf("param_name = %s, %s \n", aiisp.aiisp.param_name[0], aiisp.aiisp.param_name[1]);
	//printf("param_manual.enable = %d \n", aiisp.aiisp.manual_param.enable);
	//printf("param_manual.effect = %d \n", aiisp.aiisp.manual_param.effect);
	//printf("param_manual.param = %d, %d \n", aiisp.aiisp.manual_param.param[0], aiisp.aiisp.manual_param.param[1]);

	return ret;
}

HD_RESULT isp_set_config(BOOL manual, UINT32 config, BOOL enable, UINT32 isp_id)
{
	HD_RESULT ret = HD_OK;
	IQT_AIISP_PARAM aiisp = {0};

	aiisp.id = isp_id;
	vendor_isp_get_iq(IQT_ITEM_AIISP_PARAM, &aiisp);

	if (manual) {
		if (config < AIISP_CONFIG_MAX) {
			aiisp.aiisp.mode = IQ_OP_TYPE_MANUAL;
			aiisp.aiisp.manual_param.enable = enable;

			aiisp.aiisp.manual_param.effect = aiisp_pq_final_param_539A[config].aiisp.manual_param.effect;
			aiisp.aiisp.manual_param.param[0] = aiisp_pq_final_param_539A[config].aiisp.manual_param.param[0];
			aiisp.aiisp.manual_param.param[1] = aiisp_pq_final_param_539A[config].aiisp.manual_param.param[1];

			last_config = config;

			printf("aiisp mode MANUAL, set en=%d, effect=%d, param[0]=%d, param[1]=%d\r\n",
				aiisp.aiisp.manual_param.enable,
				aiisp.aiisp.manual_param.effect,
				aiisp.aiisp.manual_param.param[0],
				aiisp.aiisp.manual_param.param[1]);
		} else if (config == 0xFF) {
			config = last_config;

			aiisp.aiisp.mode = IQ_OP_TYPE_MANUAL;
			aiisp.aiisp.manual_param.enable = enable;

			aiisp.aiisp.manual_param.effect = aiisp_pq_final_param_539A[config].aiisp.manual_param.effect;
			aiisp.aiisp.manual_param.param[0] = aiisp_pq_final_param_539A[config].aiisp.manual_param.param[0];
			aiisp.aiisp.manual_param.param[1] = aiisp_pq_final_param_539A[config].aiisp.manual_param.param[1];

			printf("aiisp mode MANUAL, set en=%d, effect=%d, param[0]=%d, param[1]=%d\r\n",
				aiisp.aiisp.manual_param.enable,
				aiisp.aiisp.manual_param.effect,
				aiisp.aiisp.manual_param.param[0],
				aiisp.aiisp.manual_param.param[1]);
		} else {
			printf("Invalid config=%d\r\n", config);
		}

	} else {
		aiisp.aiisp.mode = IQ_OP_TYPE_AUTO;
		printf("aiisp mode AUTO\r\n");
	}

	vendor_isp_set_iq(IQT_ITEM_AIISP_PARAM, &aiisp);

	return ret;
}

INT32 isp_intpl(INT32 index, INT32 l_value, INT32 h_value, INT32 l_index, INT32 h_index)
{
	INT32 range = h_index - l_index;

	if (l_value == h_value) {
		return l_value;
	} else if (index <= l_index) {
		return l_value;
	} else if (index >= h_index) {
		return h_value;
	}
	if (h_value < l_value) {
		return l_value + ((h_value - l_value) * (index  - l_index) - (range >> 1)) / range;
	} else {
		return l_value + ((h_value - l_value) * (index  - l_index) + (range >> 1)) / range;
	}
}

void isp_intpl_awb_param(UINT32 cur_gain) {
	UINT32 iso_idx_l = 0, iso_idx_h = 0;
	UINT32 iso_start = 0, iso_end = 0;
	UINT32 i;
	AWBT_TARGET target = {0};

	// NOTE: Auto
	if (cur_gain >= iso_map_tab[AIISP_GAIN_MAX - 1][0]) {
		iso_idx_l = iso_map_tab[AIISP_GAIN_MAX - 1][1];
		iso_idx_h = iso_map_tab[AIISP_GAIN_MAX - 1][1];
		iso_start = iso_map_tab[AIISP_GAIN_MAX - 1][0];
		iso_end = iso_map_tab[AIISP_GAIN_MAX - 1][0];
	}
	for (i = 1; i < AIISP_GAIN_MAX; i++) {
		if (cur_gain < iso_map_tab[i][0]) {
			iso_idx_l = iso_map_tab[i - 1][1];
			iso_idx_h = iso_map_tab[i][1];
			iso_start = iso_map_tab[i - 1][0];
			iso_end = iso_map_tab[i][0];
			break;
		}
	}

	target.id = 0;
	vendor_isp_get_awb(AWBT_ITEM_TARGET, &target);
	target.target.rg_ratio[0] = (INT32)isp_intpl(cur_gain, awb_pq_param.target_r_gain[iso_idx_l], awb_pq_param.target_r_gain[iso_idx_h], iso_start, iso_end);
	target.target.bg_ratio[0] = (INT32)isp_intpl(cur_gain, awb_pq_param.target_b_gain[iso_idx_l], awb_pq_param.target_b_gain[iso_idx_h], iso_start, iso_end);
	target.target.rg_ratio[1] = target.target.rg_ratio[0];
	target.target.bg_ratio[1] = target.target.bg_ratio[0];
	target.target.rg_ratio[2] = target.target.rg_ratio[0];
	target.target.bg_ratio[2] = target.target.bg_ratio[0];
	target.target.rg_ratio[3] = target.target.rg_ratio[0];
	target.target.bg_ratio[3] = target.target.bg_ratio[0];
	target.target.rg_ratio[4] = target.target.rg_ratio[0];
	target.target.bg_ratio[4] = target.target.bg_ratio[0];
	target.target.rg_ratio[5] = target.target.rg_ratio[0];
	target.target.bg_ratio[5] = target.target.bg_ratio[0];
	vendor_isp_set_awb(AWBT_ITEM_TARGET, &target);
}

void *isp_smart_param_thread(void *arg)
{
	ISPT_WAIT_CFGSTR wait_cfgstr;
	ISPT_TOTAL_GAIN total_gain = {0, 0xFFFFFFFF};

	wait_cfgstr.id = 0;
	wait_cfgstr.timeout = 200;
	total_gain.id = 0;

	while (conti_run) {
		vendor_isp_get_common(ISPT_ITEM_WAIT_CFGSTR, &wait_cfgstr);
		vendor_isp_get_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);


		if (total_gain.gain == 0xFFFFFFFF) {
			continue;
		}
		isp_intpl_awb_param(total_gain.gain);
	};

	return 0;
}
