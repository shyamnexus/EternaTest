/**
    Public header file for dal_ive

    This file is the header file that define the API and data type for vendor_ive.

    @file       vendor_ive.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.    All rights reserved.
*/
#ifndef _VENDOR_IVE_H_
#define _VENDOR_IVE_H_

#include "hd_type.h"

#define VENDOR_IVE_GEN_FILT_NUM	     10
#define VENDOR_IVE_EDGE_COEFF_NUM	 25
#define VENDOR_IVE_EDGE_TH_LUT_TAP	 15
#define VENDOR_IVE_MORPH_NEIGH_NUM	 24
#define VENDOR_IVE_IOREMAP_IN_KERNEL 1
#define VENDOR_IVE_YUV_COEFF_NUM     9
#define VENDOR_IVE_YUV_BIAS_NUM      3

//----------------------------------------------------------------------
typedef struct _VENDOR_IVE_TRIGGER_PARAM {
    BOOL    is_nonblock;
	BOOL    wait_end;
	UINT32	time_out_ms;	///< force time out when wait_end == TRUE, set 0 for disable time out
} VENDOR_IVE_TRIGGER_PARAM;

//----------------------------------------------------------------------

/**
	IVE structure - ive kdrv open object
*/
typedef struct _VENDOR_IVE_OPENCFG {
	UINT32 ive_clock_sel;	//Engine clock selection
} VENDOR_IVE_OPENCFG;

//----------------------------------------------------------------------

typedef struct _VENDOR_IVE_IMG_INFO {
	UINT32 width;		//image width
	UINT32 height;		//image height
} VENDOR_IVE_IMG_INFO;

//----------------------------------------------------------------------

/**
    IVE VENDOR Address info
*/
typedef struct _VENDOR_IVE_IMG_DMA_INFO {
	uintptr_t addr;    //DMA address 
	UINT32    lofs;    //line offset
	UINT32    channel; //DMA channel
} VENDOR_IVE_IMG_DMA_INFO;

//----------------------------------------------------------------------
/**
    IVE VENDOR Function mode
*/
typedef enum
{
    VENDOR_IVE_CANNY_EDGE_PATH    = 0,
    VENDOR_IVE_VA_PATH            = 1,
    VENDOR_IVE_CSC_PATH           = 2,
    VENDOR_IVE_HISTOGRAM_PATH     = 3,
    VENDOR_IVE_NCC_PATH           = 4,
    VENDOR_IVE_DMA_COPY_PATH      = 5,
    VENDOR_IVE_THRES_LUT_PATH     = 6,
    VENDOR_IVE_IMG_OP_PATH        = 7,
    VENDOR_IVE_16bit_to_8bit_PATH = 8,
    VENDOR_IVE_CCL_PATH           = 9,
    VENDOR_IVE_ST_CRONER_PATH     = 10,
    VENDOR_IVE_LBP_PATH           = 11,
    VENDOR_IVE_2D_LUT_MERGE_PATH  = 12,
    ENUM_DUMMY4WORD(VENDOR_IVE_FUNCTION_PATH)
} VENDOR_IVE_FUNCTION_PATH;

//----------------------------------------------------------------------

/**
    IVE VENDOR general filter param and Enable/Disable
*/
typedef struct _VENDOR_IVE_GENERAL_FILTER_PARAM {
	BOOL enable;
	UINT32 filt_coeff[VENDOR_IVE_GEN_FILT_NUM];
	//UINT32 filt_norm;
} VENDOR_IVE_GENERAL_FILTER_PARAM;

//----------------------------------------------------------------------

typedef enum {
	VENDOR_IVE_MEDIAN = 0,    	//find median value in the filter
	VENDOR_IVE_MAX = 1,       	//find the maximum in the filter
	VENDOR_IVE_MIN = 2,       	//find the minimum in the filter
	VENDOR_IVE_MEDIAN_IVD_TH = 3, //find median value with invalid threshold in the filter //for_528_only
	//VENDOR_IVE_MEDIAN_W_INVAL = 3,
	ENUM_DUMMY4WORD(VENDOR_IVE_ORST_MODE)
} VENDOR_IVE_ORST_MODE;

/**
    IVE VENDOR median filter param and Enable/Disable
*/
typedef struct _VENDOR_IVE_ORST_FILTER_PARAM {
	BOOL enable;
	VENDOR_IVE_ORST_MODE mode;
	UINT32 medn_inval_th;		//for 528
} VENDOR_IVE_ORST_FILTER_PARAM;

//----------------------------------------------------------------------

typedef enum {
	VENDOR_IVE_BI_DIR = 0,    //use 2 edge filters of X & Y direction to calculate X & Y edge response separately
	VENDOR_IVE_NO_DIR = 1,    //use 1 edge filter without direction to calculate edge response
	ENUM_DUMMY4WORD(VENDOR_IVE_EDGE_MODE)
} VENDOR_IVE_EDGE_MODE;

/**
    IVE VENDOR edge filter param and Enable/Disable
*/
typedef struct _VENDOR_IVE_EDGE_FILTER_PARAM {
	BOOL enable;
	VENDOR_IVE_EDGE_MODE mode;
	UINT32 edge_coeff1[VENDOR_IVE_EDGE_COEFF_NUM];
	UINT32 edge_coeff2[VENDOR_IVE_EDGE_COEFF_NUM];
	UINT32 edge_shift_bit;
	UINT32 angle_slope_fact;
} VENDOR_IVE_EDGE_FILTER_PARAM;

//----------------------------------------------------------------------

/**
    IVE VENDOR non-maximal supression param and Enable/Disable
*/
typedef struct _VENDOR_IVE_NON_MAX_SUP_PARAM {
	BOOL enable;
	UINT32 mag_thres;
} VENDOR_IVE_NON_MAX_SUP_PARAM;

//----------------------------------------------------------------------

typedef enum {
	VENDOR_IVE_THRES_LUT_MODE1  = 1,
	VENDOR_IVE_THRES_LUT_MODE2  = 2, 
	VENDOR_IVE_THRES_LUT_MODE3  = 3,
	VENDOR_IVE_THRES_LUT_MODE4  = 4, 
	VENDOR_IVE_THRES_LUT_MODE5  = 5,
	VENDOR_IVE_THRES_LUT_MODE6  = 6, 
	VENDOR_IVE_THRES_LUT_MODE7  = 7,
	VENDOR_IVE_THRES_LUT_MODE8  = 8, 
	VENDOR_IVE_THRES_LUT_MODE9  = 9,
	VENDOR_IVE_THRES_LUT_MODE10 = 10, 
	VENDOR_IVE_THRES_LUT_MODE11 = 11,
	ENUM_DUMMY4WORD(VENDOR_IVE_THRES_LUT_MODE)
} VENDOR_IVE_THRES_LUT_MODE;

/**
    IVE VENDOR threshold LUT param and Enable/Disable
*/
typedef struct _VENDOR_IVE_THRES_LUT_PARAM {
	BOOL enable;
	VENDOR_IVE_THRES_LUT_MODE mode;
	UINT32 thres_lut[VENDOR_IVE_EDGE_TH_LUT_TAP];
	INT32 low_th;
	INT32 high_th;
	INT32 low_val;
	INT32 mid_val;
	INT32 high_val;
} VENDOR_IVE_THRES_LUT_PARAM;

//----------------------------------------------------------------------

typedef enum {
	VENDOR_IVE_MORPH      = 0,    // morphology
	VENDOR_IVE_HYSTERESIS = 1,    // hysteresis
	ENUM_DUMMY4WORD(VENDOR_IVE_POSTPROC_MODE)
} VENDOR_IVE_POSTPROC_MODE;

typedef enum {
	VENDOR_IVE_DILATE = 0,    //do dilation
	VENDOR_IVE_ERODE = 1,     //do erosion
	ENUM_DUMMY4WORD(VENDOR_IVE_MORPH_OP)
} VENDOR_IVE_MORPH_OP;

typedef enum {
	VENDOR_IVE_TH_LUT_IN = 0, //4 bit input from threshold LUT
	VENDOR_IVE_DRAM_4BIT = 1, //4 bit input from dram directly
	VENDOR_IVE_DRAM_1BIT = 2, //1 bit input from dram directly
	VENDOR_IVE_8BIT      = 3, //8 bit input from previous sub-module
	ENUM_DUMMY4WORD(VENDOR_IVE_MORPH_IN_SEL)
} VENDOR_IVE_MORPH_IN_SEL;

/**
    IVE VENDOR morphological filter param
*/
typedef struct _VENDOR_IVE_MORPH_FILTER_PARAM {
    VENDOR_IVE_MORPH_IN_SEL in_sel;
	VENDOR_IVE_MORPH_OP operation;
	BOOL neighbor[VENDOR_IVE_MORPH_NEIGH_NUM];
} VENDOR_IVE_MORPH_FILTER_PARAM;

/**
    IVE VENDOR post-processing param and Enable/Disable
*/
typedef struct _VENDOR_IVE_POSTPROC_PARAM {
	BOOL enable;
	VENDOR_IVE_POSTPROC_MODE mode;
	VENDOR_IVE_MORPH_FILTER_PARAM morph_parm;
} VENDOR_IVE_POSTPROC_PARAM;

//----------------------------------------------------------------------

typedef enum 
{
    VENDOR_IVE_INTEGRAL_Y   = 0,
    VENDOR_IVE_INTEGRAL_UV  = 1,
    VENDOR_IVE_INTEGRAL_RGB = 2,
	ENUM_DUMMY4WORD(VENDOR_IVE_INTEGRAL_FMT)
} VENDOR_IVE_INTEGRAL_FMT;

typedef enum 
{
    VENDOR_IVE_INTEGRAL_SUM         = 0,
    VENDOR_IVE_INTEGRAL_SQUARE_SUM  = 1,
    VENDOR_IVE_INTEGRAL_PACKED      = 2,
	ENUM_DUMMY4WORD(VENDOR_IVE_INTEGRAL_MODE)
} VENDOR_IVE_INTEGRAL_MODE;

/**
    IVE VENDOR edge filter param and Enable/Disable
*/
typedef struct _VENDOR_IVE_INTEGRAL_IMG_PARAM {
	BOOL enable;
	VENDOR_IVE_INTEGRAL_FMT integral_fmt;
	VENDOR_IVE_INTEGRAL_MODE mode;
} VENDOR_IVE_INTEGRAL_IMG_PARAM;

//----------------------------------------------------------------------

typedef enum 
{
    VENDOR_IVE_OUT_1BIT_EDGE              = 0,  // output from postproc (1 bit)
    VENDOR_IVE_OUT_4BIT_EDGE              = 1,  // output from postproc (4 bit)
    VENDOR_IVE_OUT_8BIT_MODE0             = 2,  // output from orst filter or thres LUT 
	VENDOR_IVE_OUT_8BIT_MODE1             = 3,  // output from NMS/postproc
	VENDOR_IVE_OUT_12BIT_MAG_THETA_PACK   = 4,  // output from edge filter (12 bit)
	VENDOR_IVE_OUT_16BIT_GRADIENT_XY_PACK = 5,  // output from edge filter (16 bit)
	VENDOR_IVE_OUT_INTEGRAL               = 6,  // output from integral image
	VENDOR_IVE_OUT_20BIT_MAG_THETA_PACK   = 7,  // output from edge filter (20 bit)
	VENDOR_IVE_OUT_32BIT_GRADIENT_XY_PACK = 8,  // output from edge filter (32 bit)
	ENUM_DUMMY4WORD(VENDOR_IVE_OUTSEL_FMT)
} VENDOR_IVE_OUTSEL_FMT;
/**
    IVE VENDOR output select
*/
typedef struct _VENDOR_IVE_OUTSEL_PARAM {
    VENDOR_IVE_OUTSEL_FMT out_data_sel;
}VENDOR_IVE_OUTSEL_PARAM;

//----------------------------------------------------------------------
typedef enum 
{
    VENDOR_IVE_YUV420 = 0,
    VENDOR_IVE_YVU420 = 1,
    VENDOR_IVE_YUV422_UYVY = 2,
    VENDOR_IVE_YUV422_YUYV = 3,
    VENDOR_IVE_YUV422_VYUY = 4,
    VENDOR_IVE_YUV422_YVYU = 5,
	ENUM_DUMMY4WORD(VENDOR_IVE_YUV_MODE)
} VENDOR_IVE_YUV_MODE;

typedef enum 
{
    VENDOR_IVE_YUV2RGB = 0,
    VENDOR_IVE_YUV2HSV = 1,
    VENDOR_IVE_YUV2LAB = 2,
	ENUM_DUMMY4WORD(VENDOR_IVE_CSC_MODE_SEL)
} VENDOR_IVE_CSC_MODE_SEL;

typedef enum 
{
    VENDOR_IVE_PLANAR = 0,
    VENDOR_IVE_PACKED = 1,
	ENUM_DUMMY4WORD(VENDOR_IVE_CSC_OUT_MODE)
} VENDOR_IVE_CSC_OUT_MODE;

typedef enum 
{
    VENDOR_IVE_D65 = 0,
    VENDOR_IVE_D50 = 1,
	ENUM_DUMMY4WORD(VENDOR_IVE_LAB_FMT)
} VENDOR_IVE_LAB_FMT;

/**
	IVE VENDOR CSC param
*/
typedef struct _VENDOR_IVE_CSC_PARAM {
	VENDOR_IVE_YUV_MODE yuv_mode;       	    ///< selection of yuv input format
	VENDOR_IVE_CSC_MODE_SEL mode_sel;   	    ///< selection of transfer format
	VENDOR_IVE_CSC_OUT_MODE out_mode;   	    ///< selection of output format
	UINT32 yuv_coef[VENDOR_IVE_YUV_COEFF_NUM];	///< yuv2rgb coefficient
	UINT32 yuv_bias[VENDOR_IVE_YUV_BIAS_NUM]; 	///< yuv2rgb bias
	UINT32 hue_shift;    	  			        ///< shift bit for HUE
	VENDOR_IVE_LAB_FMT lab_fmt;         	    ///< selection of LAB format
	UINT32 gamma_en;                            ///< gamma correction enable
} VENDOR_IVE_CSC_PARAM;

//----------------------------------------------------------------------
typedef enum 
{
    VENDOR_IVE_HIST_PDF         = 0,
    VENDOR_IVE_HIST_CDF         = 1,
    VENDOR_IVE_HIST_CDF_ON_DRAM = 2,
	ENUM_DUMMY4WORD(VENDOR_IVE_HIST_MODE)
} VENDOR_IVE_HIST_MODE;
/**
    IVE VENDOR histogram param and Enable/Disable
*/
typedef struct _VENDOR_IVE_HIST_PARAM {
	VENDOR_IVE_HIST_MODE mode;
	UINT32 cdf_min;
} VENDOR_IVE_HIST_PARAM;

//----------------------------------------------------------------------
/**
    IVE VENDOR NCC output param
*/
typedef struct _VENDOR_IVE_NCC_PARAM {
	UINT64 ncc_numerator;
	UINT64 ncc_quad_sum0;
	UINT64 ncc_quad_sum1;
} VENDOR_IVE_NCC_PARAM;

//----------------------------------------------------------------------

/**
    IVE VENDOR dma copy param
*/
typedef struct _VENDOR_IVE_DMA_PARAM {
	VENDOR_IVE_GENERAL_FILTER_PARAM general_filter_parm;
	UINT32 dma_inc_h;
	UINT32 dma_inc_v;
	UINT32 dma_blk_num;
} VENDOR_IVE_DMA_PARAM;

//----------------------------------------------------------------------

typedef enum 
{
    VENDOR_IVE_IMG_OP_ADD = 0,
    VENDOR_IVE_IMG_OP_SUB = 1,
	VENDOR_IVE_IMG_OP_AND = 2,
	VENDOR_IVE_IMG_OP_OR  = 3,
	VENDOR_IVE_IMG_OP_XOR = 4,
	ENUM_DUMMY4WORD(VENDOR_IVE_IMG_OP_MODE)
} VENDOR_IVE_IMG_OP_MODE;

typedef enum 
{
    VENDOR_IVE_IMG_OP_SUB_ABS    = 0,
    VENDOR_IVE_IMG_OP_SUB_NORMAL = 1,
	ENUM_DUMMY4WORD(VENDOR_IVE_IMG_OP_SUB_MODE)
} VENDOR_IVE_IMG_OP_SUB_MODE;
/**
    IVE VENDOR image operation param
*/
typedef struct _VENDOR_IVE_IMG_OP_PARAM {
	VENDOR_IVE_IMG_OP_MODE mode;
	VENDOR_IVE_IMG_OP_SUB_MODE sub_mode;
	UINT32 coef_a;
	UINT32 coef_b;
	INT32  coef_bias;
	UINT32 shift_bit;
} VENDOR_IVE_IMG_OP_PARAM;

//----------------------------------------------------------------------
typedef enum 
{
    VENDOR_IVE_16TO8BIT_MODE0 = 0,
    VENDOR_IVE_16TO8BIT_MODE1 = 1,
	VENDOR_IVE_16TO8BIT_MODE2 = 2,
	VENDOR_IVE_16TO8BIT_MODE3 = 3,
	VENDOR_IVE_16TO8BIT_MODE4 = 4,
	ENUM_DUMMY4WORD(VENDOR_IVE_16TO8BIT_MODE)
} VENDOR_IVE_16TO8BIT_MODE;

/**
    IVE VENDOR 16bit to 8bit param
*/
typedef struct _VENDOR_IVE_16TO8BIT_PARAM {
	VENDOR_IVE_16TO8BIT_MODE mode;
	UINT32 coef_a;
	UINT32 coef_b;
	INT32  coef_bias;
	UINT32 shift_bit;
} VENDOR_IVE_16TO8BIT_PARAM;

//----------------------------------------------------------------------

typedef enum 
{
    VENDOR_IVE_CCL_8_CONNECT = 0,
    VENDOR_IVE_CCL_4_CONNECT = 1,
	ENUM_DUMMY4WORD(VENDOR_IVE_CCL_MODE)
} VENDOR_IVE_CCL_MODE;

typedef enum 
{
    VENDOR_IVE_CCL_FIRST_PASS  = 0,
    VENDOR_IVE_CCL_SECOND_PASS = 1,
	ENUM_DUMMY4WORD(VENDOR_IVE_CCL_TRIG_MODE)
} VENDOR_IVE_CCL_TRIG_MODE;

/**
    IVE VENDOR CCL param
*/
typedef struct _VENDOR_IVE_CCL_PARAM {
	VENDOR_IVE_CCL_MODE mode;
	VENDOR_IVE_CCL_TRIG_MODE trig_mode;
	UINT32 region_x_num;
	UINT32 region_y_num;
	UINT32 region_width;
	UINT32 region_height;
	UINT32 foreground_val;
} VENDOR_IVE_CCL_PARAM;

//----------------------------------------------------------------------
typedef enum 
{
    VENDOR_IVE_ST_STAGE1 = 0,
    VENDOR_IVE_ST_STAGE2 = 1,
	ENUM_DUMMY4WORD(VENDOR_IVE_ST_CORNER_MODE)
} VENDOR_IVE_ST_CORNER_MODE;

typedef enum 
{
    VENDOR_IVE_ST_KER_3x3 = 0,
    VENDOR_IVE_ST_KER_5x5 = 1,
	ENUM_DUMMY4WORD(VENDOR_IVE_ST_CORNER_KERSIZE)
} VENDOR_IVE_ST_CORNER_KERSIZE;

typedef enum {
	VENDOR_IVE_LBP_CMP_NORMAL_MODE   = 0, 
	VENDOR_IVE_LBP_CMP_ABS_MODE      = 1,
	ENUM_DUMMY4WORD(VENDOR_IVE_LBP_CMP_MODE)
} VENDOR_IVE_LBP_CMP_MODE;

typedef enum {
	VENDOR_IVE_MAP_DIRECT_MODE       = 0, 
	VENDOR_IVE_MAP_INTERPOLATE_MODE  = 1,
	ENUM_DUMMY4WORD(VENDOR_IVE_MAP_CMP_MODE)
} VENDOR_IVE_MAP_MODE;

/**
    IVE VENDOR ST corner param
*/
typedef struct _VENDOR_IVE_ST_CORNER_PARAM {
	UINT32 st_quality_level;
	UINT32 st_min_dist;
	UINT32 st_max_corner_num;
	UINT32 st_filter_en;
	VENDOR_IVE_ST_CORNER_MODE st_mode;
	VENDOR_IVE_ST_CORNER_KERSIZE st_ker_size;
	UINT32 st_sort_topn;
	UINT32 st_max_eigen_val;
	UINT32 st_output_corner_num;
} VENDOR_IVE_ST_CORNER_PARAM;

//----------------------------------------------------------------------
/**
    IVE KDRV IRV parameter
*/
typedef struct _VENDOR_IVE_IRV_PARAM {
    BOOL enable;
    BOOL hist_mode_sel;
    BOOL invalid_val;
    UINT32 thr_s;
    UINT32 thr_h;
} VENDOR_IVE_IRV_PARAM;

//----------------------------------------------------------------------
/**

    IVE KDRV flow control parameter
*/
typedef struct _VENDOR_IVE_FLOWCT_PARAM {
    UINT32 dma_do_not_sync;
} VENDOR_IVE_FLOWCT_PARAM;
//----------------------------------------------------------------------

typedef enum {
    VENDOR_IVE_INPUT_INFO       = 0,    // input info
	VENDOR_IVE_INPUT_ADDR       = 1,    // input addr
	VENDOR_IVE_OUTPUT_ADDR      = 2,    // output addr
	VENDOR_IVE_GENERAL_FILTER   = 3,    // general filter
	VENDOR_IVE_ORST_FILTER      = 4,    // orst filter
	VENDOR_IVE_EDGE_FILTER      = 5,    // edge filter
	VENDOR_IVE_NON_MAX_SUP      = 6,    // non-maximum suppression
	VENDOR_IVE_THRES_LUT        = 7,    // edge filter
	VENDOR_IVE_POSTPROC         = 8,    // morphology / hysteresis
	VENDOR_IVE_INTEGRAL_IMG     = 9,    // integral image
	VENDOR_IVE_OUTSEL           = 10,   // output select
	VENDOR_IVE_IRV              = 11,   // IRV
	VENDOR_IVE_DMA_ABORT		= 12,   // dma abort
	VENDOR_IVE_FLOWCT           = 13,   // flow control
	VENDOR_IVE_FUNC_MODE		= 14,	// function mode
	VENDOR_IVE_CSC				= 15,	// CSC
	VENDOR_IVE_HIST             = 16,   // histogram
	VENDOR_IVE_NCC              = 17,   // NCC
	VENDOR_IVE_DMA_COPY         = 18,   // dma copy
	VENDOR_IVE_IMG_OP           = 19,   // image operation
	VENDOR_IVE_16TO8BIT         = 20,   // 16 bit to 8 bit 
	VENDOR_IVE_CCL              = 21,   // ccl
	VENDOR_IVE_ST_CORNER        = 22,   // st corner
	VENDOR_IVE_VERSION          = 23,   // get version info
    VENDOR_IVE_PROCESS          = 24,   // poocess
    VENDOR_IVE_QUERY            = 25,   // query
	ENUM_DUMMY4WORD(VENDOR_IVE_FUNC)
} VENDOR_IVE_FUNC;


/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern INT32 vendor_ive_init(void);
extern INT32 vendor_ive_uninit(void);
extern INT32 vendor_ive_set_param(VENDOR_IVE_FUNC param_id, void *p_param, UINT32 id);
extern INT32 vendor_ive_get_param(VENDOR_IVE_FUNC param_id, void *p_param, UINT32 id);
extern INT32 vendor_ive_trigger(VENDOR_IVE_TRIGGER_PARAM *p_param, UINT32 id);
extern INT32 vendor_ive_lock_eng(void);
extern INT32 vendor_ive_unlock_eng(void);
extern VOID  vendor_ive_get_version(void *p_param);
extern INT32 vendor_ive_set_job(VENDOR_IVE_FUNC param_id, void *p_param, BOOL instant, UINT8 priority, UINT32 *handle);
extern INT32 vendor_ive_query_job(void *query);
extern INT32 vendor_ive_query(void *p_query_param);

#endif //_VENDOR_IVE_H_
