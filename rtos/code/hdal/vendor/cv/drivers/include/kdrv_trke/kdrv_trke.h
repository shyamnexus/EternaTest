/**
	Public header file for dal_trke

	This file is the header file that define the API and data type for kdrv_trke.

	@file	kdrv_trke.h
	@ingroup	mILibIPLCom
	@note	Nothing (or anything need to be mentioned).

	Copyright	Novatek Microelectronics Corp. 2018.	All rights reserved.
*/
#ifndef _KDRV_TRKE_H_
#define _KDRV_TRKE_H_

#include "kdrv_type.h"


#define TRKE_IOREMAP_IN_KERNEL 1
#define TRKE_STATUS_NUM     500


typedef void (* trke_hook_t)(UINT32 hook_mode, UINT32 hook_value);

//structure data type
typedef struct _KDRV_TRKE_TRIGGER_PARAM {
    BOOL    is_nonblock;
	BOOL    wait_end;
	UINT32	time_out_ms;	///< force time out when wait_end == TRUE, set 0 for disable time out
} KDRV_TRKE_TRIGGER_PARAM;

//----------------------------------------------------------------------

/**
	TRKE structure - trke kdrv open object
*/
typedef struct _KDRV_TRKE_OPENCFG {
	UINT32 trke_clock_sel;	//Engine clock selection
} KDRV_TRKE_OPENCFG, *pKDRV_TRKE_OPENCFG;

//----------------------------------------------------------------------

typedef struct _KDRV_TRKE_IN_IMG_INFO {
	UINT32 width;		//image width
	UINT32 height;		//image height
} KDRV_TRKE_IN_IMG_INFO;

//----------------------------------------------------------------------

/**
	TRKE KDRV Input Address info
*/
typedef struct _KDRV_TRKE_IMG_IN_DMA_INFO {
	uintptr_t addr;    //DMA address
	UINT32    lofs;    //line offset
	UINT32    channel; //DMA channel
} KDRV_TRKE_IMG_IN_DMA_INFO;

/**
	TRKE KDRV Output Address info
*/
typedef struct _KDRV_TRKE_IMG_OUT_DMA_INFO {
	uintptr_t addr;    //DMA address
	UINT32    lofs;    //line offset
	UINT32    channel; //DMA channel
} KDRV_TRKE_IMG_OUT_DMA_INFO;

typedef struct _KDRV_TRKE_LL_DMA_INFO {
	uintptr_t addr;	//DMA address of LinkedList
} KDRV_TRKE_LL_DMA_INFO;
//----------------------------------------------------------------------


/**
	TRKE KDRV
*/
typedef struct _KDRV_TRKE_FUNCTION_PARAM {
	UINT32 point_num;
	UINT32 max_search_range;
	UINT32 max_iter_cnt;   	  //ITER_CNT
	UINT32 eps;
	UINT32 min_eig_val_thres ;//MIN_EIG_VAL_THRES;
	UINT32 pyr_index ;        //PYR_INDEX;
	UINT32 max_pyr_level;     //MAX_PYR_LEVEL;
	BOOL  is_initial_flow ;   //IS_INITIAL_FLOW;
	BOOL  is_skip_point ;     //IS_SKIP_POINT;
} KDRV_TRKE_FUNCTION_PARAM;

/**
	TRKE KDRV
*/
typedef struct _KDRV_TRKE_STATUS_PARAM {
	UINT8 PtStatus[TRKE_STATUS_NUM];
} KDRV_TRKE_STATUS_PARAM;


//----------------------------------------------------------------------


#if 0
/**
    TRKE KDRV trigger hook parameter
*/
typedef struct _KDRV_TRKE_TRIGGER_HOOK_PARAM {
	UINT32 hook_mode;
	UINT32 hook_value;
	UINT32 hook_is_direct_return;
} KDRV_TRKE_TRIGGER_HOOK_PARAM;



/**
	TRKE KDRV PARAM ID, for kdrv_trke_set/kdrv_trke_get
*/
typedef enum _KDRV_TRKE_PARAM_ID {
	KDRV_TRKE_PARAM_IPL_OPENCFG,			// [Set]	, use KDRV_TRKE_OPENCFG, set clock info
	KDRV_TRKE_PARAM_IPL_IN_IMG,			// [Set/Get], use KDRV_TRKE_IN_IMG_INFO structure, set input image size info, line offset
	KDRV_TRKE_PARAM_IPL_IMG_DMA_IN,		// [Set/Get], use KDRV_TRKE_IMG_IN_DMA_INFO structure, set the dram input address
	KDRV_TRKE_PARAM_IPL_IMG_DMA_OUT,		// [Set/Get], use KDRV_TRKE_IMG_OUT_DMA_INFO structure, set the dram output address
	KDRV_TRKE_PARAM_IPL_ISR_CB,			// [Set/Get], use KDRV_IPP_ISRCB structure, set trke external isr cb
	KDRV_TRKE_PARAM_IQ_STATUS,	// [Set/Get], use KDRV_TRKE_GENERAL_FILTER_PARAM structure, set general filter parameters
	KDRV_TRKE_PARAM_IPL_LL_DMA_IN,       // [Set/Get], use KDRV_TRKE_LL_DMA_INFO structure, set the dram input LL address
	KDRV_TRKE_PARAM_FUNCTION_MODE,		// [Set/Get], use KDRV_TRKE_FUNCTION_MODE structure, set function mode

	KDRV_TRKE_PARAM_MAX,
	KDRV_TRKE_PARAM_REV = 0x80000000,
	ENUM_DUMMY4WORD(KDRV_TRKE_PARAM_ID)
} KDRV_TRKE_PARAM_ID;
STATIC_ASSERT((KDRV_TRKE_PARAM_MAX &KDRV_TRKE_PARAM_REV) == 0);




#define KDRV_TRKE_IGN_CHK KDRV_TRKE_PARAM_REV	//only support set/get function

ER kdrv_trke_init_kdrv_trke_api(VOID);

#if defined __UITRON || defined __ECOS
extern void kdrv_trke_install_id(void) _SECTION(".kercfg_text");
#elif defined(__FREERTOS)
extern void kdrv_trke_install_id(void);
#else
extern void kdrv_trke_install_id(void);
#endif

/*!
 * @fn INT32 kdrv_trke_open(UINT32 chip, UINT32 engine)
 * @brief open hardware engine
 * @param chip	the chip id of hardware
 * @param engine	the engine id of hardware
 *	- @b KDRV_GFX2D_TRKE0: TRKE engine 0
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_trke_open(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_trke_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip	the chip id of hardware
 * @param engine	the engine id of hardware
 *	- @b KDRV_GFX2D_TRKE0: TRKE engine 0
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_trke_close(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_trke_set(UINT32 id, KDRV_TRKE_PARAM_ID param_id, void* p_param)
 * @brief set parameters to hardware engine
 * @param id	the id of hardware
 * @param param_id	the id of parameters
 * @param p_param	the parameters
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_trke_set(UINT32 id, KDRV_TRKE_PARAM_ID param_id, void *p_param);

/*!
 * @fn INT32 kdrv_trke_get(UINT32 id, KDRV_TRKE_PARAM_ID param_id, void* p_param)
 * @brief get parameters from hardware engine
 * @param id	the id of hardware
 * @param param_id	the id of parameters
 * @param p_param	the parameters
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_trke_get(UINT32 id, KDRV_TRKE_PARAM_ID param_id, void *p_param);

/*!
 * @fn INT32 kdrv_trke_trigger(UINT32 id,	KDRV_TRKE_TRIGGER_PARAM *p_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data)
 * @brief trigger hardware engine
 * @param id	the id of hardware
 * @param p_param	the parameter for trigger
 * @param p_cb_func	the callback function
 * @param p_user_data	the private user data
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_trke_trigger(UINT32 id, KDRV_TRKE_TRIGGER_PARAM *p_trke_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);

/*!
 * @fn INT32 kdrv_trke_trigger(UINT32 id,	KDRV_TRKE_TRIGGER_PARAM *p_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data)
 * @brief trigger hardware engine
 * @param id	the id of hardware
 * @param p_param	the parameter for trigger
 * @param p_cb_func	the callback function
 * @param p_user_data	the private user data
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_trke_trigger_with_hook(UINT32 id, KDRV_TRKE_TRIGGER_PARAM *p_trke_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data,
                        trke_hook_t trke_hook, KDRV_TRKE_TRIGGER_HOOK_PARAM tri_hook);

#endif
#endif //_KDRV_TRKE_H_
