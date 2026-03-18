/***************************************************************************
* Copyright  Novatek Microelectronics Corp. 2024.  All rights reserved.    *
*--------------------------------------------------------------------------*
* Name: DIS Library Module                                                  *
* Description:                                                             *
* Author: Brian Kang                                                         *
****************************************************************************/

/**
    DIS lib

    Sample module detailed description.

    @file       libdis.h
    @ingroup    mhdal
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/

#ifndef _LIB_DIS_H_
#define _LIB_DIS_H_

#include "hd_type.h"
#include "vendor_trke.h"
#include "vendor_vpe.h"
#include "vpe_alg.h"
#include "libdis.h"
#include "libive.h"
#include "libtrke.h"

/********************************************************************
TYPE DEFINITION - ENUM
********************************************************************/

/* DIS error code info */
typedef enum _LIB_DIS_ERROR_CODE {
	
	LIB_DIS_OK                  =        0,
	LIB_DIS_ERR_PARAM           = 		-1,
	LIB_DIS_ERR_DMA_COPY_FRAME  =       -2,
	LIB_DIS_ERR_BUILD_PYRDOWN   =       -3,
	LIB_DIS_ERR_FILTER          =       -4,	
	LIB_DIS_ERR_STCORNER        =       -5,
	LIB_DIS_ERR_TRKE            =       -6,
	//LIB_DIS_ERR_SortErr
	LIB_DIS_ERR_MOTION_EST      =       -7,
	LIB_DIS_ERR_SMOOTH          =       -8,
	LIB_DIS_ERR_OUTCOMP   		=       -9,
	LIB_DIS_ERR_VPEOUT          =       -10,	
	
	///< execute error
	LIB_DIS_ERR_INIT 	=               -11, ///< dis init failure
	LIB_DIS_ERR_LOCK 	=     			-12, ///< dis lock failure
	LIB_DIS_ERR_SET  	= 		        -13, ///< dis set param failure
	LIB_DIS_ERR_GET  	=               -14, ///< dis get param failure
	LIB_DIS_ERR_TRIG 	=               -15, ///< dis trigger failure
	LIB_DIS_ERR_UNINIT 	=               -16, ///< dis uninit failure
	LIB_DIS_ERR_UNLOCK 	=               -17, ///< dis unlock failure
	LIB_DIS_ERR_MEM_ALLOCATE =          -18, ///< dis mem allocate failure
	LIB_DIS_ERR_UNNAMED =               -99, ///< dis unnamed failure
	//... to be listed
} LIB_DIS_ERROR_CODE;

/* DIS frmae type info */
typedef enum _DIS_FRAME_TYPE {	
	normal_frame	    =  0x0,
	start		  		=  0x1,
	ENUM_DUMMY4WORD(_DIS_FRAME_TYPE)
} DIS_FRAME_TYPE;

/* DIS lut init info */
typedef enum _DIS_LUT_INIT {	
	dis_lut_init_off	=  0x0,
	dis_lut_init_on		=  0x1,
	ENUM_DUMMY4WORD(_DIS_LUT_INIT)
} DIS_LUT_INIT;

/* DIS data type info */
typedef enum _LIB_DIS_DATA_TYPE {	
	LIB_DIS_DATA_TYPE_U8C1	 =  0x0,
	LIB_DIS_DATA_TYPE_S8C1	 =  0x1,
	ENUM_DUMMY4WORD(_LIB_DIS_DATA_TYPE)
} LIB_DIS_DATA_TYPE;

/* DIS reset control info */
typedef enum _DIS_RST_Ctrl {	
	dis_ctrl_on	    	=  0x0,
	dis_ctrl_off	    =  0x1,
	dis_ctrl_reset   	=  0x2,
	ENUM_DUMMY4WORD(_DIS_RST_Ctrl)
} DIS_RST_Ctrl;

/* DIS dbg control info */
typedef enum _DIS_PARAM_DBG {	
	dis_dbg_off	    	=  0x0,
	dis_dbg_featpts	    =  0x1,
	dis_dbg_lut   		=  0x2,
	dis_dbg_allon  		=  0x3,	
	ENUM_DUMMY4WORD(_DIS_PARAM_DBG)
} DIS_PARAM_DBG;

/* DIS random control info */
typedef enum _DIS_RAM_Ctrl {	
	dis_ram_on	    	=  0x0,
	dis_ram_off		    =  0x1,
	ENUM_DUMMY4WORD(_DIS_RAM_Ctrl)
} DIS_RAM_Ctrl;

/********************************************************************
TYPE DEFINITION - Structure
********************************************************************/

/* Image frame info */
typedef struct _DIS_IMAGE_S {
	LIB_DIS_DATA_TYPE 	enDataType;
	uintptr_t 			u64Pa;
	uintptr_t 			u64Va;
	UINT32				u32Width;
	UINT32				u32Height;
    UINT32              u32Stride;  //lineofs
} DIS_IMAGE_S;

/* DIS working buffer */
typedef struct _DIS_MEM_INFO {
	uintptr_t u64Pa;      ///< Memory buffer starting address  UINTPTR
	uintptr_t u64Va;      ///< Memory buffer starting address  UINTPTR
	UINT32 size;          ///< Memory buffer size
} DIS_MEM_INFO;

/* DIS set resolution */
typedef struct _DIS_DST_IMAGE_S {
    UINT32 width;
    UINT32 height;
} DIS_SRC_IMAGE_S, DIS_DST_IMAGE_S;

/* LUT Structure */
typedef struct _DIS_2DLUT_S {       
	uintptr_t u64Pa;
	uintptr_t u64Va;
	UINT32 lut_sz;
	UINT32 frame_id ; //out frame count, DIS provide
} DIS_2DLUT_S;

/* LUT distort table */
typedef struct _DIS_VEN_2DLUT_S {
	uintptr_t u64Va ;
	UINT32 lut_sz;
} DIS_VEN_2DLUT_S;

/* LUT distort table list */
typedef struct _DIS_2DLUT_DISTORT_TABLE {
	DIS_VEN_2DLUT_S lut2d;
	float factor;
} DIS_2DLUT_DISTORT_TABLE;

/* Pyramid in/out info */
typedef struct _DIS_SRC_PYR_S {
	TRKE_SRC_IMAGE_S pstPrevPyr[5];
    TRKE_DST_IMAGE_S pstNextPyr[5];
} DIS_SRC_PYR_S;

typedef struct _Point2i{
	int x;
	int y;
} Point2i;

typedef struct _Point2f {
	float x;
	float y;
} Point2f;

typedef struct _Scale2f {
	float scale_x;
	float scale_y;
} Scale2f;

typedef struct _ERR_IDX {
    INT16 value;
    int index;
} ERR_IDX;

typedef struct { unsigned char bits[3];} UINT24;

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

/* flow set path num */
VOID NVT_DIS_Set_PathNum(int path_num);

/* flow set lut size */
VOID NVT_DIS_Set_LutSize(int path_id,VPE_ISP_2DLUT_SZ lut_sz);

/* flow get lut size */
UINT32 NVT_DIS_Get_LutSize(int path_id,VPE_ISP_2DLUT_SZ lut_sz);

/* flow set src resolution */
VOID NVT_DIS_Set_SrcResolution(int path_id,DIS_SRC_IMAGE_S pstSrc_sz);

/* flow set dst resolution */
VOID NVT_DIS_Set_DstResolution(int path_id,DIS_DST_IMAGE_S pstDst_sz);

/* flow get dis buffer size */
UINT32 NVT_DIS_Get_BufSize(UINT32 path_id);

/* flow set working buffer */
VOID NVT_DIS_Get_MemBlock(UINT32 path_id, DIS_MEM_INFO* dis_buf);

/* flow set lut initial */
VOID NVT_DIS_SET_LUT_INIT(UINT32 path_id,DIS_2DLUT_S* lut2d, DIS_LUT_INIT lut_init);

/* flow set lut update */
VOID NVT_DIS_SET_LUT_UPDATE(DIS_2DLUT_DISTORT_TABLE* lut_table_ref, int table_count, float min_factor, float max_factor, float factor_use, DIS_VEN_2DLUT_S lut2d_use);

/* flow set crop ratio */ 
VOID NVT_DIS_CropRatio(float ratio);

/* flow dis vendor init */
LIB_DIS_ERROR_CODE lib_dis_init(void);

/* flow dis vendor uninit */
LIB_DIS_ERROR_CODE lib_dis_uninit(void);

/* flow set dis reset control */
VOID NVT_DIS_RESET(UINT32 path_id, DIS_RST_Ctrl reset_ctrl);

/* flow set dis debug info level */
VOID NVT_DIS_SET_DBG(UINT32 path_id, DIS_PARAM_DBG dbg_ctrl);

/* flow dis get process status */
UINT32 NVT_DIS_Get_Stamp(UINT32 path_id);

/* flow dis get version */
VOID NVT_DIS_Get_Version(void);

/* flow set dis random srand control */
VOID NVT_DIS_SET_RAM(UINT32 path_id, DIS_RAM_Ctrl ram_ctrl);

/* flow set lut transform */
UINT32 NVT_DIS_GET_LUT_TRANSFORM(DIS_MEM_INFO lut_in_addr,DIS_MEM_INFO lut_out_addr, DIS_SRC_IMAGE_S input_resolution, DIS_DST_IMAGE_S out_resolution , VPE_ISP_2DLUT_SZ ldc_lut_size);

/* DIS main flow function */
LIB_DIS_ERROR_CODE NVT_DIS_Process(UINT32 path_id, UINT32 frame_id , DIS_IMAGE_S* pstFrame, DIS_2DLUT_S* lut2d_out);

/* unused function */
VOID NVT_DIS_SET_Record(UINT32 path_id,UINT32 frame_id , DIS_FRAME_TYPE type);

#endif  /* _LIB_DIS_H_ */
