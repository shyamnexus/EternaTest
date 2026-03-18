/**
	@brief Header file of vendor videodec module.\n
	This file contains the functions which is related to vendor videodec.

	@file vendor_videodec.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _VENDOR_VIDEODEC_H_
#define _VENDOR_VIDEODEC_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
	TYPE DEFINITION
********************************************************************/
typedef enum _VENDOR_VIDEODEC_H26X_SUB_RATIO {
	VENDOR_VDODEC_SUB_RATIO_OFF    = 0,         ///< disable extra write output
	VENDOR_VDODEC_SUB_RATIO_2x     = 2,         ///< enable, w : 1/2  h : 1/2
	VENDOR_VDODEC_SUB_RATIO_4x     = 4,         ///< enable, w : 1/4  h : 1/4
	VENDOR_VDODEC_SUB_RATIO_10_16x = 32,        ///< enable, w : 5/8  h : 5/8
	VENDOR_VDODEC_SUB_RATIO_12_16x = 64,        ///< enable, w : 3/4  h : 3/4
	ENUM_DUMMY4WORD(VENDOR_VIDEODEC_SUB_RATIO)
} VENDOR_VIDEODEC_H26X_SUB_RATIO;

//------
typedef struct _VENDOR_VIDEODEC_YUV_AUTO_DROP {
	BOOL enable;                                ///< yuv auto drop
} VENDOR_VIDEODEC_YUV_AUTO_DROP;

//------
typedef struct _VENDOR_VIDEODEC_RAWQUE_MAX_NUM {
	UINT32 rawque_max_num;                      ///< raw que max number
} VENDOR_VIDEODEC_RAWQUE_MAX_NUM;

//------
typedef struct _VENDOR_VIDEODEC_SUB_OUT_FRAME {
	VENDOR_VIDEODEC_H26X_SUB_RATIO sub_ratio;
} VENDOR_VIDEODEC_H26X_SUB_OUT_FRAME;

//------
typedef struct _VENDOR_VIDEODEC_JPEG_Y_ONLY {
	BOOL enable;
} VENDOR_VIDEODEC_JPEG_Y_ONLY;

//------
typedef struct _VENDOR_VIDEODEC_OUT {
	UINT32 dec_status;                          ///< decoder status
} VENDOR_VIDEODEC_OUT;

typedef enum _VENDOR_VIDEODEC_PARAM_ID {
	VENDOR_VIDEODEC_PARAM_IN_YUV_AUTO_DROP,     ///< CARDV only.  auto drop yuv if raw queue full
	VENDOR_VIDEODEC_PARAM_IN_RAWQUE_MAX_NUM,
	VENDOR_VIDEODEC_PARAM_OUT_STATUS,           ///< CARDV only.  return decode status is 1: start/ 0: stop
	VENDOR_VIDEODEC_PARAM_IN_H26X_SUB_OUT_FRAME,
	VENDOR_VIDEODEC_PARAM_IN_JPEG_Y_ONLY,
	ENUM_DUMMY4WORD(VENDOR_VIDEODEC_PARAM_ID)
} VENDOR_VIDEODEC_PARAM_ID;

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videodec_set(HD_PATH_ID path_id, VENDOR_VIDEODEC_PARAM_ID id, VOID *p_param);
HD_RESULT vendor_videodec_get(HD_PATH_ID path_id, VENDOR_VIDEODEC_PARAM_ID id, VOID *p_param);

#ifdef __cplusplus
}
#endif

#endif

