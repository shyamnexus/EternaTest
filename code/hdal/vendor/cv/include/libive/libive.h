/***************************************************************************
* Copyright  Novatek Microelectronics Corp. 2022.  All rights reserved.    *
*--------------------------------------------------------------------------*
* Name: IVE Library Module                                                  *
* Description:                                                             *
* Author: Ming Yang                                                         *
****************************************************************************/

/**
    IVE lib

    Sample module detailed description.

    @file       libive.h
    @ingroup    mhdal
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

#ifndef _LIB_IVE_H_
#define _LIB_IVE_H_

#include "hd_type.h"
#include "vendor_ive.h"
#include "../../drivers/include/kdrv_ive/ive_ioctl.h"
#include "../../drivers/include/kdrv_ive/kdrv_ive_lmt.h"
#include <kwrap/perf.h>

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define IVE_MAX_REGION_NUM 255

/********************************************************************
TYPE DEFINITION - ENUM
********************************************************************/
typedef enum _LIB_IVE_ERROR_CODE {
	LIB_IVE_OK =                         0,
	LIB_IVE_ERR_PARAM 	= 				-1,
	///< execute error
	LIB_IVE_ERR_INIT 	=               -10, ///< init ive failure
	LIB_IVE_ERR_LOCK 	=     			-11, ///< lock ive failure
	LIB_IVE_ERR_SET  	= 		        -12, ///< ive set param failure
	LIB_IVE_ERR_GET  	=               -13, ///< ive get param failure
	LIB_IVE_ERR_TRIG 	=               -14, ///< ive trigger failure
	LIB_IVE_ERR_UNLOCK 	=               -15, ///< unlock ive failure
	LIB_IVE_ERR_UNINIT 	=               -16, ///< uninit ive failure
	LIB_IVE_ERR_UNKNOWN =               -99, ///< unknown ive failure
} LIB_IVE_ERROR_CODE;

typedef enum _LIB_IVE_DATA_TYPE {	
	LIB_IVE_DATA_TYPE_U8C1		  	=  0x0,
	LIB_IVE_DATA_TYPE_S8C1		  	=  0x1,
	LIB_IVE_DATA_TYPE_YUV420		=  0x2, 
	LIB_IVE_DATA_TYPE_YVU420		=  0x3, 
	LIB_IVE_DATA_TYPE_YUV422_UYVY	=  0x4, 
	LIB_IVE_DATA_TYPE_YUV422_YUYV	=  0x5, 
	LIB_IVE_DATA_TYPE_YUV422_VYUY	=  0x6, 
	LIB_IVE_DATA_TYPE_YUV422_YVYU	=  0x7, 
	LIB_IVE_DATA_TYPE_U16C1 	  	=  0x8,
	LIB_IVE_DATA_TYPE_S16C1 	  	=  0x9,
	LIB_IVE_DATA_TYPE_U32C1 	  	=  0xa,
	LIB_IVE_DATA_TYPE_S32C1 	  	=  0xb,
	LIB_IVE_DATA_TYPE_U64C1 	  	=  0xc,
	LIB_IVE_DATA_TYPE_S64C1 	  	=  0xd,
	LIB_IVE_DATA_TYPE_MAX,
	ENUM_DUMMY4WORD(LIB_IVE_DATA_TYPE)
} LIB_IVE_DATA_TYPE;

typedef enum _LIB_IVE_CSC_MODE {	
	LIB_IVE_CSC_MODE_YUV2RGB		=  0x0,
	LIB_IVE_CSC_MODE_YUV2HSV		=  0x1,
	LIB_IVE_CSC_MODE_YUV2LAB		=  0x2,
	ENUM_DUMMY4WORD(LIB_IVE_CSC_MODE)
} LIB_IVE_CSC_MODE;

typedef enum _LIB_IVE_CSC_GAMMA {	
	LIB_IVE_GAMMA_BYPASS  			=  0x0,
	LIB_IVE_GAMMA_ENABLE		  	=  0x1,
	ENUM_DUMMY4WORD(LIB_IVE_CSC_GAMMA)
} LIB_IVE_CSC_GAMMA;

typedef enum _LIB_IVE_CSC_OUT_FMT {	
	LIB_IVE_OUT_FMT_PLANAR  		=  0x0,
	LIB_IVE_OUT_FMT_PACKED		  	=  0x1,
	ENUM_DUMMY4WORD(LIB_IVE_CSC_OUT_FMT)
} LIB_IVE_CSC_OUT_FMT;

typedef enum _LIB_IVE_CSC_LAB_FMT {	
	LIB_IVE_LAB_FMT_D65  			=  0x0,
	LIB_IVE_LAB_FMT_D50		  		=  0x1,
	ENUM_DUMMY4WORD(LIB_IVE_CSC_LAB_FMT)
} LIB_IVE_CSC_LAB_FMT;

typedef enum _LIB_IVE_CSC_RGB_FMT {	
	LIB_IVE_RGB_FMT_BT601  			=  0x0,
	LIB_IVE_RGB_FMT_BT709	  		=  0x1,
	LIB_IVE_RGB_FMT_JPG	  			=  0x2,
	ENUM_DUMMY4WORD(LIB_IVE_CSC_RGB_FMT)
} LIB_IVE_CSC_RGB_FMT;

typedef enum _LIB_IVE_SUB_MODE {	
	LIB_IVE_SUB_MODE_ABS     		=  0x0,
	LIB_IVE_SUB_MODE_SHIFT			=  0x1,
	ENUM_DUMMY4WORD(LIB_IVE_SUB_MODE)
} LIB_IVE_SUB_MODE;

typedef enum _LIB_IVE_16TO8_MODE {	
	LIB_IVE_MODE_S16_TO_S8    		=  0x0,
	LIB_IVE_MODE_S16_TO_U8_ABS    	=  0x1,
	LIB_IVE_MODE_S16_TO_U8_BIAS		=  0x2,
	LIB_IVE_MODE_U16_TO_U8    		=  0x3,
	ENUM_DUMMY4WORD(LIB_IVE_16TO8_MODE)
} LIB_IVE_16TO8_MODE;

typedef enum _LIB_IVE_INTEGRAL_IN_FMT {	
	LIB_IVE_INTEG_IN_FMT_1CH		=  0x0,
	LIB_IVE_INTEG_IN_FMT_2CH_PACKED	=  0x1,
	LIB_IVE_INTEG_IN_FMT_3CH_PACKED	=  0x2,
	ENUM_DUMMY4WORD(LIB_IVE_INTEGRAL_IN_FMT)
} LIB_IVE_INTEGRAL_IN_FMT;

typedef enum _LIB_IVE_INTEGRAL_OUT_FMT {	
	LIB_IVE_INTEG_OUT_FMT_SUM		=  0x0,
	LIB_IVE_INTEG_OUT_FMT_SQSUM	    =  0x1,
	LIB_IVE_INTEG_OUT_FMT_COMBINE	=  0x2,
	ENUM_DUMMY4WORD(LIB_IVE_INTEGRAL_OUT_FMT)
} LIB_IVE_INTEGRAL_OUT_FMT;

typedef enum _LIB_IVE_CCL_MODE {	
	LIB_IVE_IVE_CCL_MODE_8_CONNECTIVITY =  0x0,
	LIB_IVE_IVE_CCL_MODE_4_CONNECTIVITY =  0x1,
	ENUM_DUMMY4WORD(LIB_IVE_CCL_MODE)
} LIB_IVE_CCL_MODE;

/********************************************************************
TYPE DEFINITION - Structure
********************************************************************/

/* Image in/out info */
typedef struct _IVE_IMAGE_S {
	LIB_IVE_DATA_TYPE 	enDataType;
	uintptr_t 			u64Pa;
	uintptr_t 			u64Va;
	UINT32				u32Width;
	UINT32				u32Height;
    UINT32              u32Stride;
} IVE_SRC_IMAGE_S, IVE_DST_IMAGE_S;

/* Data info */
typedef struct _IVE_MEM_INFO_S {
	uintptr_t 			u64Pa;
	uintptr_t 			u64Va;
	UINT32				u32Size;
} IVE_MEM_INFO_S;

/* General Filter control */
typedef struct _IVE_FILTER_CTRL_S {
	UINT8				u8Mask[VENDOR_IVE_GEN_FILT_NUM];
	UINT8				in_fmt; // 0:Y, 1:UV420, 2:UV422
} IVE_FILTER_CTRL_S;

/* Sobel Filter control */
typedef struct _IVE_SOBEL_CTRL_S {
	INT8				s8Mask[VENDOR_IVE_EDGE_COEFF_NUM];
	UINT8				alpha_blending_en;
	UINT8				alpha_blending_factor;
	UINT8				gradient_out_format; // 0: 8bit packed, 1: 16bit packed, 2: 8bit unpacked, 3: 16bit unpacked
} IVE_SOBEL_CTRL_S;

/* Thres control */
typedef struct _IVE_THRESH_CTRL_S {
	VENDOR_IVE_THRES_LUT_MODE   mode;
    INT32                       s32LowThresh;
    INT32                       s32HighThresh;
    INT16                       s16MinVal;
    INT16                       s16MidVal;
    INT16                       s16MaxVal;
    UINT8                       u8LutThresh[15];    // for mode 0
} IVE_THRESH_CTRL_S;

/* Morph control */
typedef struct _IVE_MORPH_CTRL_S {
	UINT8				u8Mask[VENDOR_IVE_MORPH_NEIGH_NUM];
} IVE_MORPH_CTRL_S;

/* Integral control */
typedef struct _IVE_INTEG_CTRL_S {
	LIB_IVE_INTEGRAL_IN_FMT  enInFormat;
	LIB_IVE_INTEGRAL_OUT_FMT enOutFormat;
} IVE_INTEG_CTRL_S;

/* Canny control */
typedef struct _IVE_CANNY_CTRL_S {
	INT8				s8Mask[VENDOR_IVE_EDGE_COEFF_NUM];
	UINT8				hyster_low_th;
	UINT8				hyster_high_th;
} IVE_CANNY_CTRL_S;

/* CSC control */
typedef struct _IVE_CSC_CTRL_S {
	LIB_IVE_CSC_MODE	enCSCMode;
	LIB_IVE_CSC_GAMMA	enGamma;
	LIB_IVE_CSC_OUT_FMT	enOutFmt;
	LIB_IVE_CSC_LAB_FMT	enLabFmt;
	LIB_IVE_CSC_RGB_FMT	enRgbFmt;
} IVE_CSC_CTRL_S;

/* NCC Output */
typedef struct _IVE_NCC_INFO_S {
	UINT64				u64Numerator;
	UINT64				u64QuadSum1;
	UINT64				u64QuadSum2;
} IVE_NCC_INFO_S;

/* Image operation ADD control */
typedef struct _IVE_ADD_CTRL_S {
	UINT16				u16X;
	UINT16				u16Y;
	UINT8				u8ShiftBit;
} IVE_ADD_CTRL_S;

/* Image operation SUB control */
typedef struct _IVE_SUB_CTRL_S {
	LIB_IVE_SUB_MODE	enSubMode;
} IVE_SUB_CTRL_S;

/* DMA control */
typedef struct _IVE_DMA_CTRL_S {
	UINT8				u8HorByte;
	UINT16				u16VerByte;
	UINT8				u8EleByte;
    UINT8               gen_enable;
    UINT8				gen_u8Mask[VENDOR_IVE_GEN_FILT_NUM];
    UINT8               gen_in_fmt; // 0:Y, 1:UV420, 2:UV422
    UINT8               sobel_enable;
    INT8				s8Mask[VENDOR_IVE_EDGE_COEFF_NUM];
    UINT8				dma_sobel_out_sel; // 0:24bit pack (8b+8b+8b) 1:40bit pack (16b+16b+8b)
    UINT8               input_fmt; // 0: y format, 1: 24bit (8 + 8 + 8), 2: 40bit (16 + 16 + 8)
} IVE_DMA_CTRL_S;

/* 16bit to 8bit control */
typedef struct _IVE_16BIT_TO_8BIT_CTRL_S {
	LIB_IVE_16TO8_MODE	en16To8Mode;
	UINT32				u32Coef;
	INT8				s8Bias;
	UINT8				u8ShiftBit;
} IVE_16BIT_TO_8BIT_CTRL_S;

/* Equal histo control */
typedef struct _IVE_EQUAL_HISTO_CTRL_S {
	UINT8				u8ShiftBit;
} IVE_EQUAL_HISTO_CTRL_S;

/* CCL control */
typedef struct _IVE_CCL_CTRL_S {
	UINT32 u32AreaThres;
	LIB_IVE_CCL_MODE enCCLMode;
	UINT8 u8FgVal;
	IVE_MEM_INFO_S stTempMem;
} IVE_CCL_CTRL_S;

/* region info */
typedef struct _IVE_REGION_INFO_S {
	UINT16 u16StartX;
	UINT16 u16StartY;
	UINT16 u16EndX;
	UINT16 u16EndY;
	UINT32 u32Area;
} IVE_REGION_INFO_S;

/* CCL blob info */
typedef struct _IVE_CCL_BLOB_INFO_S {
	UINT16 u16CurAreaThres;
	INT8 s8LabelStatus;
	UINT8 u8RegionNum;
	IVE_REGION_INFO_S astRegion[IVE_MAX_REGION_NUM];
} IVE_CCL_BLOB_INFO_S;

/* ST Corner control */
typedef struct _IVE_STCORNER_CTRL_S {
    UINT8 u8QualityLevel;
    UINT8 u8MinDist;
	UINT16 u16MaxCorNum;
	UINT16 u16SortTopN;
    UINT8 u8BlkDist;
    UINT8 u8Pass2_en;
    UINT8 threshold_mode; // 0: auto, 1: manual
    UINT32 threshold_value; // only vaild when thres_mode == 1
	IVE_MEM_INFO_S stTempMem ;	
} IVE_STCORNER_CTRL_S;

/* IVE query parameter */
typedef struct _IVE_QUERY_INFO_S {
    UINT32 handle;
	BOOL instant;
	BOOL job_finish;
} IVE_QUERY_INFO_S;

typedef struct _IVE_LBP_S {
	VENDOR_IVE_LBP_CMP_MODE   mode;
    INT32         	     threshold;
} IVE_LBP_S;

typedef struct _IVE_VA_S {
    UINT8                       gen_enable;
    UINT8				        gen_u8Mask[VENDOR_IVE_GEN_FILT_NUM];
    UINT8				        gen_out_sub_ratio;
    UINT8                       map_enable;
	VENDOR_IVE_MAP_MODE         map_mode;
    UINT8                       map_idx_shift;
    UINT8                       integral_enable;
    LIB_IVE_INTEGRAL_IN_FMT     enInFormat;
	LIB_IVE_INTEGRAL_OUT_FMT    enOutFormat;
} IVE_VA_S;


/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
INT32 NVT_IVE_Filter (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, IVE_FILTER_CTRL_S* pstFltCtrl, BOOL instant);
INT32 NVT_IVE_Median (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, BOOL instant);
INT32 NVT_IVE_Min (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, BOOL instant);
INT32 NVT_IVE_Max (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, BOOL instant);
INT32 NVT_IVE_Sobel(UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst1, IVE_SOBEL_CTRL_S* pstSobelCtrl, BOOL instant);
INT32 NVT_IVE_Thresh (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, IVE_THRESH_CTRL_S* pstThrCtrl, BOOL instant);
INT32 NVT_IVE_Dilate (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, IVE_MORPH_CTRL_S* pstDilateCtrl, BOOL instant);
INT32 NVT_IVE_Erode (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, IVE_MORPH_CTRL_S* pstErodeCtrl, BOOL instant);
INT32 NVT_IVE_Integ (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, IVE_INTEG_CTRL_S *pstIntegCtrl, BOOL instant);
INT32 NVT_IVE_Canny (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, IVE_CANNY_CTRL_S* pstCannyCtrl, BOOL instant);
INT32 NVT_IVE_Histo (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, BOOL instant);
INT32 NVT_IVE_NCC (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc1, IVE_SRC_IMAGE_S* pstSrc2, IVE_DST_IMAGE_S* pstDst, BOOL instant);
INT32 NVT_IVE_Add (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc1, IVE_SRC_IMAGE_S* pstSrc2, IVE_DST_IMAGE_S* pstDst, IVE_ADD_CTRL_S *pstAddCtrl, BOOL instant);
INT32 NVT_IVE_Sub (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc1, IVE_SRC_IMAGE_S* pstSrc2,  IVE_DST_IMAGE_S* pstDst, IVE_SUB_CTRL_S* pstSubCtr, BOOL instant);
INT32 NVT_IVE_And (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc1, IVE_SRC_IMAGE_S* pstSrc2, IVE_DST_IMAGE_S* pstDst, BOOL instant);
INT32 NVT_IVE_Or (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc1, IVE_SRC_IMAGE_S* pstSrc2, IVE_DST_IMAGE_S* pstDst, BOOL instant);
INT32 NVT_IVE_Xor (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc1, IVE_SRC_IMAGE_S* pstSrc2, IVE_DST_IMAGE_S* pstDst, BOOL instant);
INT32 NVT_IVE_DMA (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, IVE_DMA_CTRL_S *pstDmaCtrl, BOOL instant);
INT32 NVT_IVE_16BitTo8Bit (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, IVE_16BIT_TO_8BIT_CTRL_S* pst16BitTo8BitCtrl, BOOL instant);
INT32 NVT_IVE_EqualHisto (UINT32* p_handle1, UINT32* p_handle2, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst1, IVE_DST_IMAGE_S* pstDst2, IVE_EQUAL_HISTO_CTRL_S* pstEqualHistoCtrl, BOOL instant);
INT32 NVT_IVE_STCorner (UINT32* p_handle1, UINT32* p_handle2, IVE_SRC_IMAGE_S* pstSrc, IVE_MEM_INFO_S* pstCorner, IVE_STCORNER_CTRL_S* pstSTCornerCtrl, UINT16* pu16OutCorNum, BOOL pass2_en, BOOL instant);
INT32 NVT_IVE_STCorner_DIS_1x1 (IVE_SRC_IMAGE_S* pstSrc, IVE_MEM_INFO_S* pstCorner, IVE_STCORNER_CTRL_S* pstSTCornerCtrl, UINT16* pu16OutCorNum, UINT16 pu16OutCorNum_blk[4]);
INT32 NVT_IVE_LBP(UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst, IVE_LBP_S* pstLBPCtrl, BOOL instant);
INT32 NVT_IVE_Map (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc1, IVE_SRC_IMAGE_S* pstSrc2, IVE_DST_IMAGE_S* pstDst, BOOL instant);
INT32 NVT_IVE_Va (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc1, IVE_SRC_IMAGE_S* pstSrc2, IVE_DST_IMAGE_S* pstDst, IVE_VA_S* pstVACtrl, BOOL instant);
INT32 NVT_IVE_Planar_To_Pack (UINT32* p_handle, IVE_SRC_IMAGE_S* pstSrc1, IVE_SRC_IMAGE_S* pstSrc2, IVE_SRC_IMAGE_S* pstSrc3, IVE_DST_IMAGE_S* pstDst, BOOL instant);

UINT32 NVT_IVE_CCL_GET_TEMP_BUFFER_SIZE(UINT32 max_width, UINT32 max_height);

INT32 NVT_IVE_QUERY(IVE_QUERY_INFO_S* pstQuery);
#endif  /* _LIB_IVE_H_ */
