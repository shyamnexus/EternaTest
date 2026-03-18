/**
    Public header file for lme

    This file is the header file that define the API and data type for vendor_lme.

    @file       vendor_lme.h
    @ingroup    mIDrvIPP_LME
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.    All rights reserved.
*/
#ifndef _VENDOR_LME_H_
#define _VENDOR_LME_H_

#include "hd_type.h"

#define VENDOR_LME_IOREMAP_IN_KERNEL 1


//----------------------------------------------------------------------
typedef struct _VENDOR_LME_TRIGGER_PARAM {
	BOOL    wait_end;
	UINT32	time_out_ms;	///< force time out when wait_end == TRUE, set 0 for disable time out
} VENDOR_LME_TRIGGER_PARAM;

//----------------------------------------------------------------------

/**
	LME structure - lme open object
*/
typedef struct _VENDOR_LME_OPENCFG {
	UINT32 lme_clock_sel;	//Engine clock selection
} VENDOR_LME_OPENCFG;

//----------------------------------------------------------------------
/**
	LME input image structure
*/
typedef struct _VENDOR_LME_IN_IMG_INFO {
	UINT32 ui_width;		         ///image width
	UINT32 ui_height;		         ///image height
	UINT32 ui_inofs;	             ///DRAM line offset of input channel
	BOOL   changesize_en;            ///enable for only change size
} VENDOR_LME_IN_IMG_INFO;

//----------------------------------------------------------------------

/**
    LME VENDOR Address info
*/
typedef struct _VENDOR_LME_IMG_IN_DMA_INFO {
	uintptr_t ui_inadd0;					///< input starting DMA address 0
	uintptr_t ui_inadd1;					///< input starting DMA address 1
} VENDOR_LME_IMG_IN_DMA_INFO;

typedef struct _VENDOR_LME_IMG_OUT_DMA_INFO {
	uintptr_t ui_outadd0;                   ///< output starting DMA address 0
} VENDOR_LME_IMG_OUT_DMA_INFO;
//----------------------------------------------------------------------

/**
	LME KDRV MV output info
*/
typedef struct _VENDOR_LME_MOTION_INFOR {
	INT32   ix;          ///< x component
	INT32   iy;          ///< y component
	UINT32  ui_sad;      ///< sum of absolute difference
	UINT32  ui_cnt;      ///< number of edge pixels (src block)
	UINT32  ui_idx;      ///< block index
	BOOL    bvalid;      ///< reliability of MV
} VENDOR_LME_MOTION_INFOR;


typedef struct _VENDOR_LME_MV_IMG_OUT_DMA_INFO {
	VENDOR_LME_MOTION_INFOR* p_mvaddr;
} VENDOR_LME_MV_IMG_OUT_DMA_INFO;

//----------------------------------------------------------------------
typedef struct _VENDOR_LME_MDS_DIM {
	UINT8 ui_blknum_h;    ///< horizontal block number in use
	UINT8 ui_blknum_v;    ///< vertical block number in use
	UINT8 ui_mdsnum;      ///< total MDS number in use
} VENDOR_LME_MDS_DIM;

typedef enum {
	LME_SUBSAMPLE_SEL_1X = 0,	    /* use all pixel for motion vector */
	LME_SUBSAMPLE_SEL_2X = 1,		/* use 1/2 pixel for motion vector */
	LME_SUBSAMPLE_SEL_4X = 2,		/* use 1/4 pixel for motion vector */
	LME_SUBSAMPLE_SEL_8X = 3,		/* use 1/8 pixel for motion vector */
	ENUM_DUMMY4WORD(VENDOR_LME_SUBSEL)
} VENDOR_LME_SUBSEL;


/// jsliu@201014
typedef enum {
	VENDOR_LME_ABORT_ENABLE = 0,
	VENDOR_LME_ABORT_DISABLE = 1,
	ENUM_DUMMY4WORD(VENDOR_LME_ABORT)
} VENDOR_LME_ABORT;
//----------------------------------------------------------------------
/**
	ETH input parameter structure
*/
typedef struct _VENDOR_ETH_IN_PARAM {
	BOOL enable;		///< eth enable
	BOOL out_bit_sel;	///< 0 --> output 2 bit per pixel, 1 --> output 8 bit per pixel
	BOOL out_sel;		///< 0 --> output all pixel, 1 --> output pixel select by b_h_out_sel/b_v_out_sel
	BOOL h_out_sel; 	///< 0 --> select even pixel to output, 1 --> select odd pixel to output
	BOOL v_out_sel; 	///< 0 --> select even pixel to output, 1 --> select odd pixel to output
	
	UINT16 th_low;		///< edge threshold
	UINT16 th_mid;
	UINT16 th_high;

} VENDOR_LME_ETH_IN_PARAM;

/**
	ETH input buffer structure
*/
typedef struct _VENDOR_ETH_IN_BUFFER_INFO {
	uintptr_t ui_inadd;        ///< output buffer address
	UINT32 buf_size;	    ///< output buffer size [Set SIE]kflow will force disable eth when buffer size < eth out size, [Get] none
	UINT32 frame_cnt;
	UINT64 time_stamp;
} VENDOR_LME_ETH_IN_BUFFER_INFO;

/**
	ETH Output structure
*/
typedef struct {
	UINT32	w;		///< Rectangle width
	UINT32	h;		///< Rectangle height
} VENDOR_LME_USIZE, *PVENDOR_LME_USIZE;

typedef struct _VENDOR_LME_ETH_OUT_PARAM {
	VENDOR_LME_USIZE out_size;		///< eth output size;
	UINT32           out_lofs;      ///< eth output lineoffset;
} VENDOR_LME_ETH_OUT_PARAM;
//----------------------------------------------------------------------

typedef enum {
	VENDOR_LME_INPUT_INFO,			            
	VENDOR_LME_INPUT_ADDR,		               	
	VENDOR_LME_INT_EN,		                
	VENDOR_LME_OUTPUT_ADDR, 
	VENDOR_LME_MV_OUT,
	VENDOR_LME_SUB_SEL,
	VENDOR_LME_MDS_DIM_INFO,
	VENDOR_LME_ETH_PARAM_IN,
	VENDOR_LME_ETH_BUFFER_IN,
	VENDOR_LME_ETH_PARAM_OUT,
	VENDOR_LME_DMA_ABORT,			/// jsliu@201014
	VENDOR_LME_VERSION ,            /// get version info
	ENUM_DUMMY4WORD(VENDOR_LME_FUNC)
} VENDOR_LME_FUNC;
//----------------------------------------------------------------------

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern INT32 vendor_lme_init(void);
extern INT32 vendor_lme_uninit(void);
extern INT32 vendor_lme_set_param(VENDOR_LME_FUNC param_id, void *p_param);
extern INT32 vendor_lme_get_param(VENDOR_LME_FUNC param_id, void *p_param);
extern INT32 vendor_lme_trigger(VENDOR_LME_TRIGGER_PARAM *p_param);
extern INT32 vendor_lme_lock_eng(void);
extern INT32 vendor_lme_unlock_eng(void);
extern VOID  vendor_lme_get_version(void *p_param);

#endif //_VENDOR_LME_H_
