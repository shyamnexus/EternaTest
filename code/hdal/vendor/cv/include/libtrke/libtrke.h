/***************************************************************************
* Copyright  Novatek Microelectronics Corp. 2022.  All rights reserved.    *
*--------------------------------------------------------------------------*
* Name: TRKE Library Module                                                  *
* Description:                                                             *
* Author: Ming Yang                                                         *
****************************************************************************/

/**
    TRKE lib

    Sample module detailed description.

    @file       libtrke.h
    @ingroup    mhdal
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

#ifndef _LIB_TRKE_H_
#define _LIB_TRKE_H_

#include "hd_type.h"
#include "vendor_trke.h"
#include "../../drivers/include/kdrv_trke/trke_ioctl.h"

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define TRKE_MAX_REGION_NUM 255

/********************************************************************
TYPE DEFINITION - ENUM
********************************************************************/
typedef enum _LIB_TRKE_ERROR_CODE {
	LIB_TRKE_OK =                         0,
	LIB_TRKE_ERR_PARAM 	= 				-1,
	///< execute error
	LIB_TRKE_ERR_INIT 	=               -10, ///< init TRKE failure
	LIB_TRKE_ERR_LOCK 	=     			-11, ///< lock TRKE failure
	LIB_TRKE_ERR_SET  	= 		        -12, ///< TRKE set param failure
	LIB_TRKE_ERR_GET  	=               -13, ///< TRKE get param failure
	LIB_TRKE_ERR_TRIG 	=               -14, ///< TRKE trigger failure
	LIB_TRKE_ERR_UNLOCK 	=               -15, ///< unlock TRKE failure
	LIB_TRKE_ERR_UNINIT 	=               -16, ///< uninit TRKE failure
	LIB_TRKE_ERR_UNKNOWN =               -99, ///< unknown TRKE failure
} LIB_TRKE_ERROR_CODE;

typedef enum _LIB_TRKE_DATA_TYPE {	
	LIB_TRKE_DATA_TYPE_U8C1		  	=  0x0,
	LIB_TRKE_DATA_TYPE_S8C1		  	=  0x1,	
	LIB_TRKE_DATA_TYPE_MAX,
	ENUM_DUMMY4WORD(LIB_TRKE_DATA_TYPE)
} LIB_TRKE_DATA_TYPE;


/********************************************************************
TYPE DEFINITION - Structure
********************************************************************/

/* Image in/out info */
typedef struct _TRKE_IMAGE_S {
	LIB_TRKE_DATA_TYPE 	enDataType;
	uintptr_t 			u64Pa;
	uintptr_t 			u64Va;
	UINT32				u32Width;
	UINT32				u32Height;
    UINT32              u32Stride;
} TRKE_SRC_IMAGE_S, TRKE_DST_IMAGE_S;

/* Data info */
typedef struct _TRKE_MEM_INFO_S {
	uintptr_t 			u64Pa;
	uintptr_t 			u64Va;
	UINT32				u32Size;
} TRKE_MEM_INFO_S;


/* ST Corner control */
typedef struct _TRKE_PYR_CTRL_S {
    BOOL enUseInitFlow;
	UINT16 u16PtsNum;
	UINT8 u8MaxLevel;
	UINT8 u0q8MinEigThr;
	UINT8 u8IterCnt ;
	UINT8 u0q8Eps ;
	UINT8 patch_size ;
	
} TRKE_PYR_CTRL_S;

/* pyramid in/out info */
typedef struct _TRKE_SRC_PYR_S {
	TRKE_SRC_IMAGE_S pstPrevPyr[6] ;
    TRKE_DST_IMAGE_S pstNextPyr[6] ;
} TRKE_SRC_PYR_S, TRKE_DST_PYR_S;




/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
BOOL trke_is_539a(void);

INT32 NVT_TRKE_PYR(TRKE_SRC_PYR_S* pstPyr, TRKE_MEM_INFO_S* pstPrevPt, TRKE_MEM_INFO_S* pstNextPt, TRKE_MEM_INFO_S* pstStatus ,TRKE_MEM_INFO_S* pstErr ,TRKE_PYR_CTRL_S* pstTRKECtrl);

#endif  /* _LIB_TRKE_H_ */
