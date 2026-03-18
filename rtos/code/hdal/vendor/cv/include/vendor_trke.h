/**
    Public header file for dal_ive

    This file is the header file that define the API and data type for vendor_ive.

    @file       vendor_trke.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.    All rights reserved.
*/
#ifndef _VENDOR_TRKE_H_
#define _VENDOR_TRKE_H_

#include "hd_type.h"

#define VENDOR_TRKE_STATUS_NUM     500
#define VENDOR_TRKE_IOREMAP_IN_KERNEL 1

//----------------------------------------------------------------------
typedef struct _VENDOR_TRKE_TRIGGER_PARAM {
    BOOL    is_nonblock;
	BOOL    wait_end;
	UINT32	time_out_ms;	///< force time out when wait_end == TRUE, set 0 for disable time out
} VENDOR_TRKE_TRIGGER_PARAM;

//----------------------------------------------------------------------

/**
	TRKE structure - trke kdrv open object
*/
typedef struct _VENDOR_TRKE_OPENCFG {
	UINT32 trke_clock_sel;	//Engine clock selection
} VENDOR_TRKE_OPENCFG;

//----------------------------------------------------------------------

typedef struct _VENDOR_TRKE_IMG_INFO {
	UINT32 width;		//image width
	UINT32 height;		//image height
} VENDOR_TRKE_IMG_INFO;

//----------------------------------------------------------------------

/**
    TRKE VENDOR Address info
*/

typedef struct _VENDOR_TRKE_IMG_DMA_INFO {
	uintptr_t addr;    //DMA address 
	UINT32    lofs;    //line offset
	UINT32    channel; //DMA channel
} VENDOR_TRKE_IMG_DMA_INFO;


//----------------------------------------------------------------------
/**
    TRKE VENDOR Function mode
*/
//----------------------------------------------------------------------


/**
	TRKE KDRV Output Address info
*/
typedef struct _VENDOR_TRKE_IMG_OUT_DMA_INFO {
	UINT32 out_addr0;      //DMA address of status channel
	UINT32 out_addr1;      //DMA address of err channel	
} VENDOR_TRKE_IMG_OUT_DMA_INFO;


/**
    TRKE VENDOR LL address
*/

typedef struct _VENDOR_TRKE_LL_DMA_INFO {
	UINT32 addr;	//DMA address of LinkedList
} VENDOR_TRKE_LL_DMA_INFO;


/**
	TRKE VENDOR parameter 
*/
typedef struct _VENDOR_TRKE_FUNCTION_PARAM {
	UINT32 point_num;	
	UINT32 max_iter_cnt;   	  //ITER_CNT
	UINT32 eps; 
	UINT32 min_eig_val_thres ;//MIN_EIG_VAL_THRES;	
	UINT32 pyr_index ;        //PYR_INDEX;
	UINT32 max_pyr_level;     //MAX_PYR_LEVEL;
	BOOL  is_initial_flow ;   //IS_INITIAL_FLOW;
	BOOL  is_skip_point ;     //IS_SKIP_POINT;
	BOOL  max_sr_disable ; //max search range disable
	UINT32 pyr_search_range;     //MAX_PYR_LEVEL;
	UINT32 layer_search_range;     //MAX_PYR_LEVEL;
	UINT32 patch_size; 
	UINT32 max_search_range;
} VENDOR_TRKE_FUNCTION_PARAM;

/**
	TRKE VENDOR STATUS 
*/
typedef struct _VENDOR_TRKE_STATUS_PARAM {
	UINT8 PtStatus[VENDOR_TRKE_STATUS_NUM];	
} VENDOR_TRKE_STATUS_PARAM;



//----------------------------------------------------------------------

typedef enum {
    VENDOR_TRKE_INPUT_INFO       = 0,    // input info
	VENDOR_TRKE_INPUT_ADDR       = 1,    // input addr
	VENDOR_TRKE_OUTPUT_ADDR      = 2,    // output addr
	VENDOR_TRKE_FUNC_PARAM       = 3,    // VENDOR_TRKE_FUNCTION_PARAM
	VENDOR_TRKE_STATUS           = 4,    // VENDOR_TRKE_STATUS_PARAM
	VENDOR_TRKE_VERSION          = 5,    // get version info
    VENDOR_TRKE_PROCESS          = 6,   // poocess
    VENDOR_TRKE_QUERY            = 7,   // query
	ENUM_DUMMY4WORD(VENDOR_TRKE_FUNC)
} VENDOR_TRKE_FUNC;


//----------------------------------------------------------------------


/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern INT32 vendor_trke_init(void);
extern INT32 vendor_trke_uninit(void);
extern INT32 vendor_trke_set_param(VENDOR_TRKE_FUNC param_id, void *p_param, UINT32 id);
extern INT32 vendor_trke_get_param(VENDOR_TRKE_FUNC param_id, void *p_param, UINT32 id);
extern INT32 vendor_trke_trigger(VENDOR_TRKE_TRIGGER_PARAM *p_param, UINT32 id);
extern INT32 vendor_trke_lock_eng(void);
extern INT32 vendor_trke_unlock_eng(void);
extern VOID  vendor_trke_get_version(void *p_param);
extern INT32 vendor_trke_set_job(VENDOR_TRKE_FUNC param_id, void *p_param, BOOL instant, UINT8 priority, UINT32 *handle);
extern INT32 vendor_trke_query_job(VENDOR_TRKE_FUNC param_id, void *param);

#endif //_VENDOR_TRKE_H_
