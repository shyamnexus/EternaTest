/**
    Public header file for IPP module.

    @file       ipp_eng.h
    @ingroup    mIIPPIFE

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _IPP_ENG_H_
#define _IPP_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kdrv_type.h"
#include "kwrap/debug.h"

#define MAX_IPP_STRIPE_SIZE 4096
#define MAX_IPP_STRIPE_NUM  8
#define IPP_IFE_IPE_RESERVED_OVLP  32 //reserved for 538 ipe ycc-lite

typedef enum {
	AI_ISP_MST_PRIOR = 0,
	AI_ISP_SST_3840_PRIOR = 1,
	CODEC_LOW_LATENCY_PRIOR = 2,
	ENUM_DUMMY4WORD(IPP_STRIPE_RULE)
} IPP_STRIPE_RULE;

typedef struct _IPP_CAL_STRP_INFO {
BOOL ime_enable;
UINT32 vdec_strp_limit;
INT32 image_width;
UINT32 stripe_num;
UINT32 ife_overlap;
UINT32 ipe_overlap;
UINT32 ime_overlap;
UINT32 ime_out_strp_divisor;
UINT32 ipe_out_strp_divisor;
UINT32 codec_strp_divisor;
BOOL ife_nn_enable;
UINT32 ife_nn_strp_max_width;
BOOL ime_nn_enable;
UINT32 ime_nn_strp_max_width;
IPP_STRIPE_RULE ipp_strp_rule;
} IPP_CAL_STRP_INFO;

typedef struct _IPP_CAL_STRP_RESULT {
UINT32 mod_stripe_num;
UINT32 ife_in_strp[8];
UINT32 ife_ovlp;
UINT32 ipe_in_strp[8];
UINT32 ipe_ovlp;
UINT32 ime_in_strp[8];
UINT32 ime_ovlp;
UINT32 tmnr_out_strp[8];
} IPP_CAL_STRP_RESULT;

extern BOOL ipp_eng_calculate_stripe_settings(IPP_CAL_STRP_INFO *strp_info, IPP_CAL_STRP_RESULT *strp_result);

#endif
