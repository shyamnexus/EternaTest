/*
* Copyright  Novatek Microelectronics Corp. 2022.  All rights reserved.
*----------------------------------------------------------------------
* Name: VQA AlG Library header
* Author: IVoT CVAI/PF
*/

#ifndef _VQA_ALG_H
#define _VQA_ALG_H

#include <stdio.h>
#include <time.h>
#include <math.h>

#define VQA_MAX_CH_NUM		1

#define VQA_X_NUM	160
#define VQA_Y_NUM	94

#include "hd_type.h"

typedef struct _alg_vqa_res_t {
	UINT32 res_too_light;
	UINT32 res_too_dark;
	UINT32 res_blur;
	UINT32 res_blur_cover;
	UINT8 *res_blur_block;
} alg_vqa_res_t;

typedef struct _alg_enable_param_t {
	UINT32 en_auto_adj_param;
	UINT32 en_ref_md;
	UINT32 en_too_light;
	UINT32 en_too_dark;
	UINT32 en_blur;
	UINT32 en_blur_block_info;
	UINT32 reserved[7];          ///< Reserved words
}alg_enable_param_t;

typedef struct _alg_global_param_t {
	UINT32 width;
	UINT32 height;
	UINT32 mb_x_size;
	UINT32 mb_y_size;
	UINT32 auto_adj_period;
	UINT32 update_frame_num;
	UINT32 g_alarm_frame_num;
	UINT32 reserved[7];          ///< Reserved words
}alg_global_param_t;

typedef struct _alg_light_param_t {
	UINT32 too_light_strength_th;
	UINT32 too_light_cover_th;
	UINT32 too_light_alarm_times;

	UINT32 too_dark_strength_th;
	UINT32 too_dark_cover_th;
	UINT32 too_dark_alarm_times;

	UINT32 reserved[7];          ///< Reserved words
}alg_light_param_t;

typedef struct _alg_contrast_param_t {
	UINT32 blur_strength;
	UINT32 blur_cover_th;
	UINT32 blur_alarm_times;
	UINT32 blur_w_num;
	UINT32 blur_h_num;
	UINT32 reserved[7];          ///< Reserved words
}alg_contrast_param_t;

typedef struct _alg_vqa_param_t {
	alg_enable_param_t enable_param;
	alg_global_param_t global_param;
	alg_light_param_t light_param;
	alg_contrast_param_t contrast_param;
	UINT32 reserved[7];          ///< Reserved words
}alg_vqa_param_t;



extern void vqa_stable_counter(UINT32  ch, UINT32 *res, UINT32  type, UINT32  alarm_times_th, INT32 g_alarm_frame_num, UINT32 is_debug);
extern INT32 vqa_auto_adj_param(UINT32  ch, UINT32  width, UINT32  height, UINT8 *img_y);
extern INT32 vqa_light_abnormal_evt(UINT32  ch, UINT32  width, UINT32  height, UINT8 *img_y, UINT8 *vqa_evt, alg_vqa_res_t *vqa_res, UINT32 *res_too_light_cover, UINT32 *res_too_dark_cover, UINT32 *avg_gray_val);
extern INT32 vqa_blur_evt(UINT32 ch, UINT32  width, UINT32  height, UINT8 *img_y, UINT8 *vqa_evt, alg_vqa_res_t *vqa_res, UINT32 en_blur_block_info, UINT32 *res_blur_cover, INT32 blur_start, UINT32 is_debug);


#endif
