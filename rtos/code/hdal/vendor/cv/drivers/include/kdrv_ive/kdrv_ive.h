/**
	Public header file for dal_ive

	This file is the header file that define the API and data type for kdrv_ive.

	@file	kdrv_ive.h
	@ingroup	mILibIPLCom
	@note	Nothing (or anything need to be mentioned).

	Copyright	Novatek Microelectronics Corp. 2018.	All rights reserved.
*/
#ifndef _KDRV_IVE_H_
#define _KDRV_IVE_H_

#include "kdrv_type.h"

#define KDRV_IVE_GEN_FILT_NUM	   10
#define KDRV_IVE_EDGE_COEFF_NUM	   25
#define KDRV_IVE_EDGE_TH_LUT_TAP   15
#define KDRV_IVE_MORPH_NEIGH_NUM   24
#define KDRV_IVE_YUV_COEFF_NUM     9
#define KDRV_IVE_YUV_BIAS_NUM      3

#define IVE_IOREMAP_IN_KERNEL 1

typedef void (* ive_hook_t)(UINT32 hook_mode, UINT32 hook_value);

//structure data type
typedef struct _KDRV_IVE_TRIGGER_PARAM {
    BOOL    is_nonblock;
	BOOL    wait_end;
	UINT32	time_out_ms;	///< force time out when wait_end == TRUE, set 0 for disable time out
} KDRV_IVE_TRIGGER_PARAM;

//----------------------------------------------------------------------

/**
	IVE structure - ive kdrv open object
*/
typedef struct _KDRV_IVE_OPENCFG {
	UINT32 ive_clock_sel;	//Engine clock selection
} KDRV_IVE_OPENCFG, *pKDRV_IVE_OPENCFG;

//----------------------------------------------------------------------

typedef struct _KDRV_IVE_IMG_INFO {
	UINT32 width;		//image width
	UINT32 height;		//image height
} KDRV_IVE_IMG_INFO;

//----------------------------------------------------------------------

/**
	IVE KDRV Address info
*/
typedef struct _KDRV_IVE_IMG_DMA_INFO {
	uintptr_t addr;    //DMA address
	UINT32    lofs;    //line offset
	UINT32    channel; //DMA channel
} KDRV_IVE_IMG_DMA_INFO;

typedef struct _KDRV_IVE_LL_DMA_INFO {
	uintptr_t addr;	//DMA address of LinkedList
} KDRV_IVE_LL_DMA_INFO;
//----------------------------------------------------------------------

/**
    IVE KDRV general filter param and Enable/Disable
*/
typedef struct _KDRV_IVE_GENERAL_FILTER_PARAM {
	BOOL enable;
	UINT32 filt_coeff[KDRV_IVE_GEN_FILT_NUM];
	//UINT32 filt_norm;
} KDRV_IVE_GENERAL_FILTER_PARAM;

//----------------------------------------------------------------------

typedef enum {
	KDRV_IVE_MEDIAN = 0,    	//find median value in the filter
	KDRV_IVE_MAX = 1,       	//find the maximum in the filter
	KDRV_IVE_MIN = 2,       	//find the minimum in the filter
	KDRV_IVE_MEDIAN_IVD_TH = 3, //find median value with invalid threshold in the filter //for_528_only
	//KDRV_IVE_MEDIAN_W_INVAL = 3,
	ENUM_DUMMY4WORD(KDRV_IVE_ORST_MODE)
} KDRV_IVE_ORST_MODE;

/**
    IVE KDRV median filter param and Enable/Disable
*/
typedef struct _KDRV_IVE_ORST_FILTER_PARAM {
	BOOL enable;
	KDRV_IVE_ORST_MODE mode;
	UINT32 medn_inval_th;		//for 528
} KDRV_IVE_ORST_FILTER_PARAM;

//----------------------------------------------------------------------

typedef enum {
	KDRV_IVE_BI_DIR = 0,    //use 2 edge filters of X & Y direction to calculate X & Y edge response separately
	KDRV_IVE_NO_DIR = 1,    //use 1 edge filter without direction to calculate edge response
	ENUM_DUMMY4WORD(KDRV_IVE_EDGE_MODE)
} KDRV_IVE_EDGE_MODE;

/**
    IVE KDRV edge filter param and Enable/Disable
*/
typedef struct _KDRV_IVE_EDGE_FILTER_PARAM {
	BOOL enable;
	KDRV_IVE_EDGE_MODE mode;
	UINT32 edge_coeff1[KDRV_IVE_EDGE_COEFF_NUM];
	UINT32 edge_coeff2[KDRV_IVE_EDGE_COEFF_NUM];
	UINT32 edge_shift_bit;
	UINT32 angle_slope_fact;
} KDRV_IVE_EDGE_FILTER_PARAM;

//----------------------------------------------------------------------

/**
    IVE KDRV non-maximal supression param and Enable/Disable
*/
typedef struct _KDRV_IVE_NON_MAX_SUP_PARAM {
	BOOL enable;
	UINT32 mag_thres;
} KDRV_IVE_NON_MAX_SUP_PARAM;

//----------------------------------------------------------------------

typedef enum {
	KDRV_IVE_THRES_LUT_MODE0  = 0,
	KDRV_IVE_THRES_LUT_MODE1  = 1,
	KDRV_IVE_THRES_LUT_MODE2  = 2,
	KDRV_IVE_THRES_LUT_MODE3  = 3,
	KDRV_IVE_THRES_LUT_MODE4  = 4,
	KDRV_IVE_THRES_LUT_MODE5  = 5,
	KDRV_IVE_THRES_LUT_MODE6  = 6,
	KDRV_IVE_THRES_LUT_MODE7  = 7,
	KDRV_IVE_THRES_LUT_MODE8  = 8,
	KDRV_IVE_THRES_LUT_MODE9  = 9,
	KDRV_IVE_THRES_LUT_MODE10 = 10,
	KDRV_IVE_THRES_LUT_MODE11 = 11,
	ENUM_DUMMY4WORD(KDRV_IVE_THRES_LUT_MODE)
} KDRV_IVE_THRES_LUT_MODE;

/**
    IVE KDRV threshold LUT param and Enable/Disable
*/
typedef struct _KDRV_IVE_THRES_LUT_PARAM {
	BOOL enable;
	KDRV_IVE_THRES_LUT_MODE mode;
	UINT32 thres_lut[KDRV_IVE_EDGE_TH_LUT_TAP];
	INT32 low_th;
	INT32 high_th;
	INT32 low_val;
	INT32 mid_val;
	INT32 high_val;
} KDRV_IVE_THRES_LUT_PARAM;

//----------------------------------------------------------------------

typedef enum {
	KDRV_IVE_MORPH      = 0,    // morphology
	KDRV_IVE_HYSTERESIS = 1,    // hysteresis
	ENUM_DUMMY4WORD(KDRV_IVE_POSTPROC_MODE)
} KDRV_IVE_POSTPROC_MODE;

typedef enum {
	KDRV_IVE_DILATE = 0,    //do dilation
	KDRV_IVE_ERODE = 1,     //do erosion
	ENUM_DUMMY4WORD(KDRV_IVE_MORPH_OP)
} KDRV_IVE_MORPH_OP;

typedef enum {
	KDRV_IVE_TH_LUT_IN = 0, //4 bit input from threshold LUT
	KDRV_IVE_DRAM_4BIT = 1, //4 bit input from dram directly
	KDRV_IVE_DRAM_1BIT = 2, //1 bit input from dram directly
	KDRV_IVE_8BIT      = 3, //8 bit input from previous sub-module
	ENUM_DUMMY4WORD(KDRV_IVE_MORPH_IN_SEL)
} KDRV_IVE_MORPH_IN_SEL;

/**
    IVE KDRV morphological filter param
*/
typedef struct _KDRV_IVE_MORPH_FILTER_PARAM {
    KDRV_IVE_MORPH_IN_SEL in_sel;
	KDRV_IVE_MORPH_OP operation;
	BOOL neighbor[KDRV_IVE_MORPH_NEIGH_NUM];
} KDRV_IVE_MORPH_FILTER_PARAM;

/**
    IVE KDRV post-processing param and Enable/Disable
*/
typedef struct _KDRV_IVE_POSTPROC_PARAM {
	BOOL enable;
	KDRV_IVE_POSTPROC_MODE mode;
	KDRV_IVE_MORPH_FILTER_PARAM morph_parm;
} KDRV_IVE_POSTPROC_PARAM;

//----------------------------------------------------------------------

typedef enum
{
    KDRV_IVE_INTEGRAL_Y   = 0,
    KDRV_IVE_INTEGRAL_UV  = 1,
    KDRV_IVE_INTEGRAL_RGB = 2,
	ENUM_DUMMY4WORD(KDRV_IVE_INTEGRAL_FMT)
} KDRV_IVE_INTEGRAL_FMT;

typedef enum
{
    KDRV_IVE_INTEGRAL_SUM         = 0,
    KDRV_IVE_INTEGRAL_SQUARE_SUM  = 1,
    KDRV_IVE_INTEGRAL_PACKED      = 2,
	ENUM_DUMMY4WORD(KDRV_IVE_INTEGRAL_MODE)
} KDRV_IVE_INTEGRAL_MODE;

/**
    IVE KDRV edge filter param and Enable/Disable
*/
typedef struct _KDRV_IVE_INTEGRAL_IMG_PARAM {
	BOOL enable;
	KDRV_IVE_INTEGRAL_FMT integral_fmt;
	KDRV_IVE_INTEGRAL_MODE mode;
} KDRV_IVE_INTEGRAL_IMG_PARAM;

//----------------------------------------------------------------------

typedef enum
{
    KDRV_IVE_OUT_1BIT_EDGE              = 0,
    KDRV_IVE_OUT_4BIT_EDGE              = 1,
    KDRV_IVE_OUT_8BIT_BEFORE_POSTPROC   = 2,
	KDRV_IVE_OUT_8BIT_AFTER_POSTPROC    = 3,
	KDRV_IVE_OUT_12BIT_MAG_THETA_PACK   = 4,
	KDRV_IVE_OUT_16BIT_GRADIENT_XY_PACK = 5,
	KDRV_IVE_OUT_INTEGRAL               = 6,
	KDRV_IVE_OUT_20BIT_MAG_THETA_PACK   = 7,
	KDRV_IVE_OUT_32BIT_GRADIENT_XY_PACK = 8,
	ENUM_DUMMY4WORD(KDRV_IVE_OUTSEL_FMT)
} KDRV_IVE_OUTSEL_FMT;
/**
    IVE KDRV output select
*/
typedef struct _KDRV_IVE_OUTSEL_PARAM {
    KDRV_IVE_OUTSEL_FMT out_data_sel;
}KDRV_IVE_OUTSEL_PARAM;


//----------------------------------------------------------------------
typedef enum
{
    KDRV_IVE_YUV420 = 0,
    KDRV_IVE_YVU420 = 1,
    KDRV_IVE_YUV422_UYVY = 2,
    KDRV_IVE_YUV422_YUYV = 3,
    KDRV_IVE_YUV422_VYUY = 4,
    KDRV_IVE_YUV422_YVYU = 5,
	ENUM_DUMMY4WORD(KDRV_IVE_YUV_MODE)
} KDRV_IVE_YUV_MODE;

typedef enum
{
    KDRV_IVE_YUV2RGB = 0,
    KDRV_IVE_YUV2HSV = 1,
    KDRV_IVE_YUV2LAB = 2,
	ENUM_DUMMY4WORD(KDRV_IVE_CSC_MODE_SEL)
} KDRV_IVE_CSC_MODE_SEL;

typedef enum
{
    KDRV_IVE_PLANAR = 0,
    KDRV_IVE_PACKED = 1,
	ENUM_DUMMY4WORD(KDRV_IVE_CSC_OUT_MODE)
} KDRV_IVE_CSC_OUT_MODE;

typedef enum
{
    KDRV_IVE_D65 = 0,
    KDRV_IVE_D50 = 1,
	ENUM_DUMMY4WORD(KDRV_IVE_LAB_FMT)
} KDRV_IVE_LAB_FMT;

/**
	IVE KDRV CSC param
*/
typedef struct _KDRV_IVE_CSC_PARAM {
	KDRV_IVE_YUV_MODE yuv_mode;       	    ///< selection of yuv input format
	KDRV_IVE_CSC_MODE_SEL mode_sel;   	    ///< selection of transfer format
	KDRV_IVE_CSC_OUT_MODE out_mode;   	    ///< selection of output format
	UINT32 yuv_coef[KDRV_IVE_YUV_COEFF_NUM];	///< yuv2rgb coefficient
	UINT32 yuv_bias[KDRV_IVE_YUV_BIAS_NUM]; 	///< yuv2rgb bias
	UINT32 hue_shift;    	  			        ///< shift bit for HUE
	KDRV_IVE_LAB_FMT lab_fmt;         	    ///< selection of LAB format
	UINT32 gamma_en;                            ///< gamma correction enable
} KDRV_IVE_CSC_PARAM;

//----------------------------------------------------------------------
typedef enum
{
    KDRV_IVE_HIST_PDF         = 0,
    KDRV_IVE_HIST_CDF         = 1,
    KDRV_IVE_HIST_CDF_ON_DRAM = 2,
	ENUM_DUMMY4WORD(KDRV_IVE_HIST_MODE)
} KDRV_IVE_HIST_MODE;
/**
    IVE KDRV histogram param and Enable/Disable
*/
typedef struct _KDRV_IVE_HIST_PARAM {
	KDRV_IVE_HIST_MODE mode;
	UINT32 cdf_min;
} KDRV_IVE_HIST_PARAM;

//----------------------------------------------------------------------
/**
    IVE KDRV NCC output param
*/
typedef struct _KDRV_IVE_NCC_PARAM {
	UINT64 ncc_numerator;
	UINT64 ncc_quad_sum0;
	UINT64 ncc_quad_sum1;
} KDRV_IVE_NCC_PARAM;

//----------------------------------------------------------------------
/**
    IVE KDRV dma copy param
*/
typedef struct _KDRV_IVE_DMA_PARAM {
	KDRV_IVE_GENERAL_FILTER_PARAM general_filter_parm;
	UINT32 dma_inc_h;
	UINT32 dma_inc_v;
	UINT32 dma_blk_num;
} KDRV_IVE_DMA_PARAM;

//----------------------------------------------------------------------

typedef enum
{
    KDRV_IVE_IMG_OP_ADD = 0,
    KDRV_IVE_IMG_OP_SUB = 1,
	KDRV_IVE_IMG_OP_AND = 2,
	KDRV_IVE_IMG_OP_OR  = 3,
	KDRV_IVE_IMG_OP_XOR = 4,
	ENUM_DUMMY4WORD(KDRV_IVE_IMG_OP_MODE)
} KDRV_IVE_IMG_OP_MODE;

typedef enum
{
    KDRV_IVE_IMG_OP_SUB_ABS    = 0,
    KDRV_IVE_IMG_OP_SUB_NORMAL = 1,
	ENUM_DUMMY4WORD(KDRV_IVE_IMG_OP_SUB_MODE)
} KDRV_IVE_IMG_OP_SUB_MODE;
/**
    IVE KDRV image operation param
*/
typedef struct _KDRV_IVE_IMG_OP_PARAM {
	KDRV_IVE_IMG_OP_MODE mode;
	KDRV_IVE_IMG_OP_SUB_MODE sub_mode;
	UINT32 coef_a;
	UINT32 coef_b;
	INT32  coef_bias;
	UINT32 shift_bit;
} KDRV_IVE_IMG_OP_PARAM;

//----------------------------------------------------------------------
typedef enum
{
    KDRV_IVE_16TO8BIT_MODE0 = 0,
    KDRV_IVE_16TO8BIT_MODE1 = 1,
	KDRV_IVE_16TO8BIT_MODE2 = 2,
	KDRV_IVE_16TO8BIT_MODE3 = 3,
	KDRV_IVE_16TO8BIT_MODE4 = 4,
	ENUM_DUMMY4WORD(KDRV_IVE_16TO8BIT_MODE)
} KDRV_IVE_16TO8BIT_MODE;

/**
    IVE KDRV 16bit to 8bit param
*/
typedef struct _KDRV_IVE_16TO8BIT_PARAM {
	KDRV_IVE_16TO8BIT_MODE mode;
	UINT32 coef_a;
	UINT32 coef_b;
	INT32  coef_bias;
	UINT32 shift_bit;
} KDRV_IVE_16TO8BIT_PARAM;

//----------------------------------------------------------------------

typedef enum
{
    KDRV_IVE_CCL_8_CONNECT = 0,
    KDRV_IVE_CCL_4_CONNECT = 1,
	ENUM_DUMMY4WORD(KDRV_IVE_CCL_MODE)
} KDRV_IVE_CCL_MODE;

typedef enum
{
    KDRV_IVE_CCL_FIRST_PASS  = 0,
    KDRV_IVE_CCL_SECOND_PASS = 1,
	ENUM_DUMMY4WORD(KDRV_IVE_CCL_TRIG_MODE)
} KDRV_IVE_CCL_TRIG_MODE;

/**
    IVE KDRV CCL param
*/
typedef struct _KDRV_IVE_CCL_PARAM {
	KDRV_IVE_CCL_MODE mode;
	KDRV_IVE_CCL_TRIG_MODE trig_mode;
	UINT32 region_x_num;
	UINT32 region_y_num;
	UINT32 region_width;
	UINT32 region_height;
	UINT32 foreground_val;
} KDRV_IVE_CCL_PARAM;

//----------------------------------------------------------------------
typedef enum
{
    KDRV_IVE_ST_STAGE1 = 0,
    KDRV_IVE_ST_STAGE2 = 1,
	ENUM_DUMMY4WORD(KDRV_IVE_ST_CORNER_MODE)
} KDRV_IVE_ST_CORNER_MODE;

typedef enum
{
    KDRV_IVE_ST_KER_3x3 = 0,
    KDRV_IVE_ST_KER_5x5 = 1,
	ENUM_DUMMY4WORD(KDRV_IVE_ST_CORNER_KERSIZE)
} KDRV_IVE_ST_CORNER_KERSIZE;

/**
    IVE KDRV ST corner param
*/
typedef struct _KDRV_IVE_ST_CORNER_PARAM {
	UINT32 st_quality_level;
	UINT32 st_min_dist;
	UINT32 st_max_corner_num;
	UINT32 st_filter_en;
	KDRV_IVE_ST_CORNER_MODE st_mode;
	KDRV_IVE_ST_CORNER_KERSIZE st_ker_size;
	UINT32 st_sort_topn;
	UINT32 st_max_eigen_val;
	UINT32 st_output_corner_num;
} KDRV_IVE_ST_CORNER_PARAM;

//----------------------------------------------------------------------
/**
    IVE KDRV IRV parameter
*/
typedef struct _KDRV_IVE_IRV_PARAM {
    BOOL enable;
    BOOL hist_mode_sel;
    BOOL invalid_val;
    UINT32 thr_s;
    UINT32 thr_h;
} KDRV_IVE_IRV_PARAM;

/**
    IVE KDRV trigger hook parameter
*/
typedef struct _KDRV_IVE_TRIGGER_HOOK_PARAM {
	UINT32 hook_mode;
	UINT32 hook_value;
	UINT32 hook_is_direct_return;
} KDRV_IVE_TRIGGER_HOOK_PARAM;

//----------------------------------------------------------------------
/**


    IVE KDRV flow control parameter
*/
typedef struct _KDRV_IVE_FLOWCT_PARAM {
    UINT32 dma_do_not_sync;
} KDRV_IVE_FLOWCT_PARAM;
//----------------------------------------------------------------------
#if 0
/**
	IVE KDRV PARAM ID, for kdrv_ive_set/kdrv_ive_get
*/
typedef enum _KDRV_IVE_PARAM_ID {
	KDRV_IVE_PARAM_OPENCFG,			// [Set]	, use KDRV_IVE_OPENCFG, set clock info
	KDRV_IVE_PARAM_IN_IMG,			// [Set/Get], use KDRV_IVE_IN_IMG_INFO structure, set input image size info, line offset
	KDRV_IVE_PARAM_IMG_DMA_IN,		// [Set/Get], use KDRV_IVE_IMG_IN_DMA_INFO structure, set the dram input address
	KDRV_IVE_PARAM_IMG_DMA_OUT,		// [Set/Get], use KDRV_IVE_IMG_OUT_DMA_INFO structure, set the dram output address
	KDRV_IVE_PARAM_ISR_CB,			// [Set/Get], use KDRV_IPP_ISRCB structure, set ive external isr cb
	KDRV_IVE_PARAM_GENERAL_FILTER,	// [Set/Get], use KDRV_IVE_GENERAL_FILTER_PARAM structure, set general filter parameters
	KDRV_IVE_PARAM_ORST_FILTER,	    // [Set/Get], use KDRV_IVE_MEDIAN_FILTER_PARAM structure, set median filter parameters
	KDRV_IVE_PARAM_EDGE_FILTER,		// [Set/Get], use KDRV_IVE_EDGE_FILTER_PARAM structure, set edge filter parameters
	KDRV_IVE_PARAM_NON_MAX_SUP,		// [Set/Get], use KDRV_IVE_NON_MAX_SUP_PARAM structure, set non-maximal supression parameters
	KDRV_IVE_PARAM_THRES_LUT,		// [Set/Get], use KDRV_IVE_THRES_LUT_PARAM structure, set edge threshold LUT parameters
	KDRV_IVE_PARAM_POSTPROC,		// [Set/Get], use KDRV_IVE_MORPH_FILTER_PARAM structure, set morphological filter parameters
	KDRV_IVE_PARAM_INTEGRAL_IMG,    // [Set/Get], use KDRV_IVE_INTEGRAL_IMG_PARAM structure, set integral image parameters
	KDRV_IVE_PARAM_OUTSEL,          // [Set/Get], use KDRV_IVE_OUTSEL_PARAM structure, set output selection parameters
	KDRV_IVE_PARAM_LL_DMA_IN,       // [Set/Get], use KDRV_IVE_LL_DMA_INFO structure, set the dram input LL address
	KDRV_IVE_PARAM_IRV,		        // [Set/Get], use KDRV_IVE_IRV_PARAM structure, set irv
	KDRV_IVE_PARAM_FLOWCT,		    // [Set/Get], use KDRV_IVE_FLOW_CT structure, set dma cache flush or not
	KDRV_IVE_PARAM_FUNCTION_MODE,	// [Set/Get], use KDRV_IVE_FUNCTION_MODE structure, set function mode
	KDRV_IVE_PARAM_CSC,				// [Set/Get], use KDRV_IVE_FUNCTION_MODE structure, set function mode
	KDRV_IVE_PARAM_HIST,
	KDRV_IVE_PARAM_NCC,
	KDRV_IVE_PARAM_DMA_COPY,
	KDRV_IVE_PARAM_IMG_OP,
	KDRV_IVE_PARAM_16TO8BIT,
	KDRV_IVE_PARAM_CCL,
	KDRV_IVE_PARAM_ST_CORNER,
	KDRV_IVE_PARAM_MAX,
	KDRV_IVE_PARAM_REV = 0x80000000,
	ENUM_DUMMY4WORD(KDRV_IVE_PARAM_ID)
} KDRV_IVE_PARAM_ID;
STATIC_ASSERT((KDRV_IVE_PARAM_MAX &KDRV_IVE_PARAM_REV) == 0);




#define KDRV_IVE_IGN_CHK KDRV_IVE_PARAM_REV	//only support set/get function

ER kdrv_ive_chk_efuse_kdrv_ive_api(VOID);

#if defined __UITRON || defined __ECOS
extern void kdrv_ive_install_id(void) _SECTION(".kercfg_text");
#elif defined(__FREERTOS)
extern void kdrv_ive_install_id(void);
#else
extern void kdrv_ive_install_id(void);
#endif

/*!
 * @fn INT32 kdrv_ive_open(UINT32 chip, UINT32 engine)
 * @brief open hardware engine
 * @param chip	the chip id of hardware
 * @param engine	the engine id of hardware
 *	- @b KDRV_GFX2D_IVE0: IVE engine 0
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_ive_open(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_ive_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip	the chip id of hardware
 * @param engine	the engine id of hardware
 *	- @b KDRV_GFX2D_IVE0: IVE engine 0
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_ive_close(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_ive_set(UINT32 id, KDRV_IVE_PARAM_ID param_id, void* p_param)
 * @brief set parameters to hardware engine
 * @param id	the id of hardware
 * @param param_id	the id of parameters
 * @param p_param	the parameters
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_ive_set(UINT32 id, KDRV_IVE_PARAM_ID param_id, void *p_param);

/*!
 * @fn INT32 kdrv_ive_get(UINT32 id, KDRV_IVE_PARAM_ID param_id, void* p_param)
 * @brief get parameters from hardware engine
 * @param id	the id of hardware
 * @param param_id	the id of parameters
 * @param p_param	the parameters
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_ive_get(UINT32 id, KDRV_IVE_PARAM_ID param_id, void *p_param);

/*!
 * @fn INT32 kdrv_ive_trigger(UINT32 id,	KDRV_IVE_TRIGGER_PARAM *p_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data)
 * @brief trigger hardware engine
 * @param id	the id of hardware
 * @param p_param	the parameter for trigger
 * @param p_cb_func	the callback function
 * @param p_user_data	the private user data
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_ive_trigger(UINT32 id, KDRV_IVE_TRIGGER_PARAM *p_ive_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);

/*!
 * @fn INT32 kdrv_ive_trigger(UINT32 id,	KDRV_IVE_TRIGGER_PARAM *p_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data)
 * @brief trigger hardware engine
 * @param id	the id of hardware
 * @param p_param	the parameter for trigger
 * @param p_cb_func	the callback function
 * @param p_user_data	the private user data
 * @return return 0 on success, -1 on error
 */
extern INT32 kdrv_ive_trigger_with_hook(UINT32 id, KDRV_IVE_TRIGGER_PARAM *p_ive_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data,
                        ive_hook_t ive_hook, KDRV_IVE_TRIGGER_HOOK_PARAM tri_hook);

#endif
#endif //_KDRV_IVE_H_
