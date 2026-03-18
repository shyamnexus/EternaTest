/***************************************************************************
* Copyright  Novatek Microelectronics Corp. 2019.  All rights reserved.    *
*--------------------------------------------------------------------------*
* Name: MD Library Module                                                  *
* Description:                                                             *
* Author: Ming Yang                                                         *
****************************************************************************/

/**
    MD lib

    Sample module detailed description.

    @file       libmd.h
    @ingroup    mhdal
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _LIB_MD_H_
#define _LIB_MD_H_

#include "hd_type.h"
#include "vendor_md.h"

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define LIB_MD_MAX_OBJ_NUM								 32
#define LIB_MD_MAX_SUB_REGION_NUM						 4

/********************************************************************
TYPE DEFINITION - ENUM
********************************************************************/
typedef enum _LIB_MD_ERROR_CODE {
	LIB_MD_OK =                         0,
	LIB_MD_ERR_PARAM 	= 				-1,
	///< execute error
	LIB_MD_ERR_INIT 	=               -10, ///< init MD failure
	LIB_MD_ERR_LOCK 	=     			-11, ///< lock MD failure
	LIB_MD_ERR_SET  	= 		        -12, ///< MD set param failure
	LIB_MD_ERR_GET  	=               -13, ///< MD get param failure
	LIB_MD_ERR_TRIG 	=               -14, ///< MD trigger failure
	LIB_MD_ERR_UNLOCK 	=               -15, ///< unlock MD failure
	LIB_MD_ERR_UNINIT 	=               -16, ///< uninit MD failure
	LIB_MD_ERR_UNKNOWN =                -99, ///< unknown MD failure
} LIB_MD_ERROR_CODE;

typedef enum _LIB_MD_HW_STATUS {	
	LIB_MD_HW_INIT		=  0x0,
	LIB_MD_HW_RUN		=  0x1,
	LIB_MD_HW_UNINIT	=  0x2,
	LIB_MD_HW_ALL		=  0x3,
	ENUM_DUMMY4WORD(LIB_MD_HW_STATUS)
} LIB_MD_HW_STATUS;

typedef enum _LIB_MD_MORPH_EN {	
	LIB_MD_MORPH_BYPASS  	=  0x0,
	LIB_MD_MORPH_ENABLE		=  0x1,
	ENUM_DUMMY4WORD(LIB_MD_MORPH_EN)
} LIB_MD_MORPH_EN;

typedef enum _LIB_MD_BC_SENSI_LEVEL {	
	LIB_MD_BC_LOW_SENSI			=  0x0,
	LIB_MD_BC_MED_SENSI			=  0x1,
	LIB_MD_BC_HIGH_SENSI		=  0x2,
	LIB_MD_BC_SUPER_HIGH_SENSI	=  0x3,
	ENUM_DUMMY4WORD(LIB_MD_BC_SENSI_LEVEL)
} LIB_MD_BC_SENSI_LEVEL;

typedef enum _LIB_MD_BC_CONVG_SPD {	
	LIB_MD_BC_LOW_SPD		=  0x0,
	LIB_MD_BC_MED_SPD		=  0x1,
	LIB_MD_BC_HIGH_SPD		=  0x2,
	LIB_MD_BC_SUPER_HIGH_SPD=  0x3,
	ENUM_DUMMY4WORD(LIB_MD_BC_CONVG_SPD)
} LIB_MD_BC_CONVG_SPD;

typedef enum _LIB_MD_BC_IMG_FMT {	
	LIB_MD_IMG_YUV444			=  0x0,
	LIB_MD_IMG_YUV420SP			=  0x1,
	LIB_MD_IMG_Y_ONLY			=  0x2,
	LIB_MD_IMG_YUV444_LIGHT		=  0x3,
	LIB_MD_IMG_YUV420SP_LIGHT	=  0x4,
	LIB_MD_IMG_Y_ONLY_LIGHT		=  0x5,
	ENUM_DUMMY4WORD(LIB_MD_BC_IMG_FMT)
} LIB_MD_BC_IMG_FMT;

typedef enum _LIB_MD_BC_BG_NUM {	
	LIB_MD_BG_NUM_4			=  0x4,
	LIB_MD_BG_NUM_6			=  0x6,
	LIB_MD_BG_NUM_8			=  0x8,
	LIB_MD_BG_NUM_16		=  0x16,
	ENUM_DUMMY4WORD(LIB_MD_BC_BG_NUM)
} LIB_MD_BC_BG_NUM;

typedef enum _LIB_MD_BC_MODE_SEL {	
	LIB_MD_BC_INIT_MODE		=  0x0,
	LIB_MD_BC_NORM_MODE		=  0x1,
	ENUM_DUMMY4WORD(LIB_MD_BC_MODE_SEL)
} LIB_MD_BC_MODE_SEL;

typedef enum _LIB_MD_GMM_MODE_SEL {	
	LIB_MD_GMM_NORM_MODE	=  0x0,
	LIB_MD_GMM_INIT_MODE	=  0x1,
	ENUM_DUMMY4WORD(LIB_MD_GMM_MODE_SEL)
} LIB_MD_GMM_MODE_SEL;

typedef enum _LIB_MD_GMM_SENSI_LEVEL {	
	LIB_MD_GMM_SENSI_LEVEL0		=  0x0,
	LIB_MD_GMM_SENSI_LEVEL1		=  0x1,
	LIB_MD_GMM_SENSI_LEVEL2		=  0x2,
	LIB_MD_GMM_SENSI_LEVEL3		=  0x3,
	LIB_MD_GMM_SENSI_LEVEL4		=  0x4,
	LIB_MD_GMM_SENSI_LEVEL5		=  0x5,
	LIB_MD_GMM_SENSI_LEVEL6		=  0x6,
	LIB_MD_GMM_SENSI_LEVEL7		=  0x7,
	LIB_MD_GMM_SENSI_LEVEL8		=  0x8,
	LIB_MD_GMM_SENSI_LEVEL9		=  0x9,
	ENUM_DUMMY4WORD(LIB_MD_GMM_SENSI_LEVEL)
} LIB_MD_GMM_SENSI_LEVEL;

typedef enum _LIB_MD_IN_RANGE {	
	LIB_MD_IN_RANGE_0_1		=  0x0,
	LIB_MD_IN_RANGE_0_255	=  0x1,
	ENUM_DUMMY4WORD(LIB_MD_IN_RANGE)
} LIB_MD_IN_RANGE;

typedef enum _LIB_MD_CROSS_RST {	
	LIB_MD_CROSS_NONE		=  0x0,
	LIB_MD_CROSS_IN_TO_OUT	=  0x1,
	LIB_MD_CROSS_OUT_TO_IN	=  0x2,
	ENUM_DUMMY4WORD(LIB_MD_CROSS_RST)
} LIB_MD_CROSS_RST;

typedef enum _LIB_MD_CROSS_MODE {	
	LIB_MD_CROSS_LINE		=  0x0,
	LIB_MD_FORBIDDEN_ZONE	=  0x1,
	ENUM_DUMMY4WORD(LIB_MD_CROSS_MODE)
} LIB_MD_CROSS_MODE;

/********************************************************************
TYPE DEFINITION - Structure
********************************************************************/
/* Image in/out info */
typedef struct _MD_IMAGE_S {
	uintptr_t 				u64Pa;
	uintptr_t 				u64Va;
	UINT32					u32Width;
	UINT32					u32Height;
    UINT32              	u32Stride;
} MD_SRC_IMAGE_S, MD_DST_IMAGE_S;

/* Data info */
typedef struct _MD_MEM_INFO_S {
	uintptr_t 				u64Pa;
	uintptr_t 				u64Va;
	UINT32					u32Size;
} MD_MEM_INFO_S;

/* Point info */
typedef struct _MD_PT_INFO_S {
	UINT16					u16X;
	UINT16					u16Y;
} MD_PT_INFO_S;

typedef struct _MD_MORPH_CTRL_S {
	LIB_MD_MORPH_EN			enMorph;
	UINT8					u8MorphThres;
} MD_MORPH_CTRL_S;

typedef struct _MD_MDBC_CTRL_S {
	LIB_MD_BC_SENSI_LEVEL	enSensiLevel;
	LIB_MD_BC_CONVG_SPD		enConvgSpd;
	LIB_MD_BC_IMG_FMT		enInFmt;
	LIB_MD_BC_MODE_SEL		enIsInit;
	MD_MORPH_CTRL_S			stMorph[4];
	MD_MEM_INFO_S			stTempMem;
	UINT32					u32LumDiff;
	UINT32					u32FrmID;
	UINT32					u32Rnd;
	UINT32					u32TmpThres;
	LIB_MD_BC_BG_NUM		enBgNum;
} MD_MDBC_CTRL_S;

typedef struct _MD_GMM_CTRL_S {
	LIB_MD_GMM_SENSI_LEVEL	enSensiLevel;
	LIB_MD_GMM_MODE_SEL		enIsInit;
	MD_MEM_INFO_S			stTempMem;
} MD_GMM_CTRL_S;

typedef struct _MD_GALARM_CTRL_S {
	UINT8					u8AlarmThres;
} MD_GALARM_CTRL_S;

/* capture channel motion detection sub-region rect info */
typedef struct _MD_SUBPARAM_CTRL_S {
	UINT32		  			enSubRegion;	///< decide sub-region enable
	UINT32		  			u32XStart;		///< start x mb position
	UINT32		  			u32YStart;		///< start y mb position
	UINT32		  			u32XEnd;		///< end x mb position
	UINT32		  			u32YEnd;		///< end x mb position
	UINT8		  			u8SubAlarmThres;///< sub-region motion alarm sensitive threshold
} MD_SUBPARAM_CTRL_S;

typedef struct _MD_SUBALARM_CTRL_S {
	UINT8		  			u8SubNum;	
	MD_SUBPARAM_CTRL_S* 	pstSubParam;
} MD_SUBALARM_CTRL_S;

typedef struct _MD_OBJ_RST_S {
	UINT32		  			u32XStart;		///< start x mb position
	UINT32		  			u32YStart;		///< start y mb position
	UINT32		  			u32XEnd;		///< end x mb position
	UINT32		  			u32YEnd;		///< end x mb position
	UINT32					u32Label;
} MD_OBJ_RST_S;

typedef struct _MD_OBJ_INFO_S {
	UINT32		  			u32ObjNum;	 	
	MD_OBJ_RST_S	 		stObjRst[LIB_MD_MAX_OBJ_NUM];
} MD_OBJ_INFO_S;

typedef struct _MD_OBJ_CTRL_S {
	UINT32		  			u32ObjThres;
	LIB_MD_IN_RANGE			enInRange;
	MD_PT_INFO_S*			pstPtStk;
} MD_OBJ_CTRL_S;

typedef struct _LIB_MD_LIB_OBJ_INFO {
	UINT32        u32XStart;
	UINT32        u32YStart; 
	UINT32        u32XEnd;  
	UINT32        u32YEnd;
	UINT8		  u8Valid;
	UINT8		  u8Track;
	INT8		  s8Status;
} MD_CROSS_OBJ_INFO_S;

typedef struct _MD_CROSS_CTRL_S {
	LIB_MD_CROSS_MODE		enCrossMode;
	MD_CROSS_OBJ_INFO_S		stPreObj[LIB_MD_MAX_OBJ_NUM];
	UINT8					u8PreObjNum;
	UINT8					u8ObjNum;
	UINT32					u32OverlapTh;
	UINT32		  			u32X1;// Top left x
	UINT32		  			u32Y1;// Top left y
	UINT32		  			u32X2;// Top right x
	UINT32		  			u32Y2;// Top right y
	UINT32		  			u32X3;// Down right x
	UINT32		  			u32Y3;// Down right y
	UINT32		  			u32X4;// Down left x
	UINT32		  			u32Y4;// Down left y
} MD_CROSS_CTRL_S;

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
INT32 NVT_MD_MDBC (MD_SRC_IMAGE_S* pstSrc, MD_DST_IMAGE_S* pstDst, MD_MDBC_CTRL_S* pstMdbcCtrl);
INT32 NVT_MD_GMM (MD_SRC_IMAGE_S* pstSrc, MD_DST_IMAGE_S* pstDst, MD_GMM_CTRL_S* pstGmmCtrl);
INT32 NVT_MD_GlobalAlarm (MD_SRC_IMAGE_S* pstSrc, UINT8* pu8Rst, MD_GALARM_CTRL_S* pstGAlarmCtrl);
INT32 NVT_MD_SubAlarm (MD_SRC_IMAGE_S* pstSrc, UINT8* pu8Rst, MD_SUBALARM_CTRL_S* pstSubAlarmCtrl);
INT32 NVT_MD_ObjDet (MD_SRC_IMAGE_S* pstSrc, MD_OBJ_INFO_S* pstDst, MD_OBJ_CTRL_S* pstObjCtrl);
INT32 NVT_MD_CrossLine (MD_CROSS_OBJ_INFO_S* pstSrc, LIB_MD_CROSS_RST* penDst, MD_CROSS_CTRL_S* pstCrossCtrl);

#endif  /* _LIB_MD_H_ */
