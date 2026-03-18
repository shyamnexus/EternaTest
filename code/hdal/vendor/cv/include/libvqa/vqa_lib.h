/*
* Copyright  Novatek Microelectronics Corp. 2022.  All rights reserved.
*----------------------------------------------------------------------
* Name: VQA Library header file
* Author: IVoT CVAI/PF
*/

#ifndef _VQA_LIB_H
#define _VQA_LIB_H

#define VQA_MAX_CH_NUM	  	1
#define VQA_IMG_SIZE 		240*135

#include "hd_type.h"

#define vqa_major     01
#define vqa_minor     00
#define vqa_bugfix    230531
#define vqa_ext       0

#define _VQA_STR(a,b,c)          #a"."#b"."#c
#define VQA_STR(a,b,c)           _VQA_STR(a,b,c)
#define VQA_VERSION    VQA_STR(vqa_major, vqa_minor, vqa_bugfix)//implementation version major.minor.yymmdds

#ifndef UINTPTR
typedef uintptr_t                       UINTPTR;
#endif

typedef struct _vqa_res_t {
	UINT32 res_too_light;
	UINT32 res_too_dark;
	UINT32 res_blur;
	UINT32 res_blur_cover;
	UINT8 *res_blur_block;
}vqa_res_t;

typedef struct _enable_param_t {
	UINT32 en_auto_adj_param;
	UINT32 en_ref_md;
	UINT32 en_too_light;
	UINT32 en_too_dark;
	UINT32 en_blur;
	UINT32 en_blur_block_info;
}enable_param_t;

typedef struct _global_param_t {
	UINT32 width;
	UINT32 height;
	UINT32 mb_x_size;
	UINT32 mb_y_size;
	UINT32 auto_adj_period;
	UINT32 g_alarm_frame_num;
	UINT32 reserved[7];          ///< Reserved words
}global_param_t;

typedef struct _light_param_t {
	UINT32 too_light_strength_th;
	UINT32 too_light_cover_th;
	UINT32 too_light_alarm_times;

	UINT32 too_dark_strength_th;
	UINT32 too_dark_cover_th;
	UINT32 too_dark_alarm_times;

	UINT32 reserved[7];          ///< Reserved words
}light_param_t;

typedef struct _contrast_param_t {
	UINT32 blur_strength;
	UINT32 blur_cover_th;
	UINT32 blur_alarm_times;
	UINT32 blur_w_num;
	UINT32 blur_h_num;
	UINT32 reserved[7];          ///< Reserved words
}contrast_param_t;

typedef struct _vqa_param_t {
	enable_param_t	 enable_param;
	global_param_t	 global_param;
	light_param_t	 light_param;
	contrast_param_t contrast_param;
	UINT32 reserved[7];          ///< Reserved words
}vqa_param_t;


VOID    NVT_VQA_Init(UINT32 ch, UINTPTR pstBufCtrl);
VOID    NVT_VQA_Set_param(UINT32 ch, vqa_param_t *pstSrc);
extern UINT32   NVT_VQA_Calc_buf_size(UINT32 width, UINT32 height);

INT32	NVT_VQA_Run(UINT32 ch, UINT8 *pstSrc, vqa_res_t *pstDst);
extern VOID NVT_VQA_Set_debug(UINT32 is_debug);

#endif
