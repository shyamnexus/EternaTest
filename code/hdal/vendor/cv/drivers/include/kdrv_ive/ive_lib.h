/*
    Public header file for IVE module.

    This file is the header file that define the API and data type for IVE
    module.

    @file       ive_lib.h
    @ingroup    mIDrvIPPIVE
    @note

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#ifndef _IVE_LIB_H
#define _IVE_LIB_H

/*
 *#ifdef __cplusplus
 *extern "C" {
 *#endif
 */

#include "kdrv_type.h"

//#include    "Type.h"//a header for basic variable type

/**
    @addtogroup mIDrvIPPIVE
*/
//@{

/**
    Define IVE interrput enable.
*/
//@{
#define IVE_INTE_FRM_END                    0x00000001  ///< Enable interrupt of frame end
#define IVE_INTE_LLEND                      0x00000100  ///< Status of interrupt of frame end(LL)
#define IVE_INTE_LLERROR                    0x00000200  ///< Status of interrupt of frame end(LL)
#define IVE_INTE_LLJOBEND                   0x00000400  ///< Status of interrupt of frame job_end(LL)
#define IVE_INTE_ALL                        0x00000301  ///< Enable all interrupts

//@}

/**
    Define IVE interrput status.
*/
//@{
#define IVE_INT_FRM_END                     0x00000001  ///< Status of interrupt of frame end
#define IVE_INT_LLEND                       0x00000100  ///< Status of interrupt of frame end(LL)
#define IVE_INT_LLERROR                     0x00000200  ///< Status of interrupt of frame end(LL)
#define IVE_INT_LLJOBEND                    0x00000400  ///< Status of interrupt of frame job_end(LL)
#define IVE_INT_ALL                         0x00000701  ///< Status of all interrupts
//@}

#define GEN_COEFF_NUM       10
#define EDGE_COEFF_NUM      25
#define EDGE_TH_LUT_TAP     15
#define MORPH_NEIGH_NUM     24
#define YUV_COEFF_NUM       9
#define YUV_BIAS_NUM        3

#define IVE_MAX_INPUT_NUM   2
#define IVE_MAX_OUTPUT_NUM  3

/*----------- enumeration ------------*/
/**
    Operations of engine action
*/
//@{
typedef enum {
	IVE_OP_OPEN          = 0,    ///< Open engine
	IVE_OP_CLOSE         = 1,    ///< Close engine
	IVE_OP_SETPARAM      = 2,    ///< Set parameter
	IVE_OP_START         = 3,    ///< Start engine
	IVE_OP_PAUSE         = 4,    ///< Pause engine

	ENUM_DUMMY4WORD(IVE_ACTION_OP)
} IVE_ACTION_OP;
//@}

typedef enum {
	IVE_DBG_PORT1 = 0,
	IVE_DBG_PORT2 = 1,
	IVE_DBG_PORT3 = 2,
	IVE_DBG_PORT4 = 3,

	ENUM_DUMMY4WORD(IVE_DBG_PORT_SEL)
} IVE_DBG_PORT_SEL;

/*
    Operations of engine action
*/
typedef enum {
	IVE_START_LOAD  = 0,    ///< Load parameter at engine start(enable 0->1)
	IVE_FRMEND_LOAD = 1,    ///< Load parameter at engine frame end(frame end interrupt occur)

	ENUM_DUMMY4WORD(IVE_LOAD_TYPE)
} IVE_LOAD_TYPE;

typedef enum {
	BURST_64W = 0,
	BURST_48W = 1,
	BURST_32W = 2,
	BURST_16W = 3,

	ENUM_DUMMY4WORD(BURST_LEN_SEL)
} BURST_LEN_SEL;

/**
    State of State Machine
*/
//@{
typedef enum {
	IVE_ENGINE_IDLE  = 0,        ///< Idel state
	IVE_ENGINE_READY = 1,        ///< Ready state
	IVE_ENGINE_PAUSE = 2,        ///< Pause state
	IVE_ENGINE_RUN   = 3,        ///< Run state

	ENUM_DUMMY4WORD(IVE_STATE_MACHINE)
} IVE_STATE_MACHINE;
//@}

typedef enum {
	MEDIAN = 0,
	MAX = 1,
	MIN = 2,


	ENUM_DUMMY4WORD(MEDN_MODE)
} MEDN_MODE;

typedef enum {
	BI_DIR = 0,
	NO_DIR = 1,

	ENUM_DUMMY4WORD(EDGE_MODE)
} EDGE_MODE;

typedef enum {
	EDGE_1B = 0,
	EDGE_4B = 1,
	IMG_MEDIAN_8B = 2,
	IMG_NONMAX_8B = 3,
	EDGE_MAG_THETA_12B = 4,//reserved because of alg error
	EDGE_X_Y_16B = 5,
	INTEGRAL_32B = 6,

	ENUM_DUMMY4WORD(OUT_DATA_TYPE)
} OUT_DATA_TYPE;

typedef enum {
	EDGE_20B = 0,
	EDGE_32B = 1,
	ENUM_DUMMY4WORD(OUT1_DATA_TYPE)
} OUT1_DATA_TYPE;

typedef enum {
	DILATE = 0,
	ERODE = 1,

	ENUM_DUMMY4WORD(MORPH_OP)
} MORPH_OP;



typedef enum {
	TH_LUT_IN = 0,
	DRAM_4BIT = 1,
	DRAM_1BIT = 2,
    NON_MAX_IN = 3,

	ENUM_DUMMY4WORD(MORPH_IN_SEL)
} MORPH_IN_SEL;

typedef enum 
{
    CANNY_EDGE_MODE = 0,
    CSC_MODE		= 1,
    HISTOGRAM_MODE  = 2,
    NCC_MODE		= 3,
	DMA_CP_MODE		= 4,
	LUT_MODE		= 5,
	IMG_OPERATION_MODE = 6,
	BIT_CLAMP_MODE	= 7,
	CCL_MODE		= 8,
	ST_CORNER_MODE	= 9,
	ENUM_DUMMY4WORD(FUNCTION_MODE)
} FUNCTION_MODE;

typedef enum 
{
    YUV420 = 0,
    YVU420 = 1,
    YUV422_UYVY = 2,
    YUV422_YUYV = 3,
    YUV422_VYUY = 4,
    YUV422_YVYU = 5,
	ENUM_DUMMY4WORD(YUV_MODE)
} YUV_MODE;

typedef enum 
{
    YUV2RGB = 0,
    YUV2HSV = 1,
    YUV2LAB = 2,
	ENUM_DUMMY4WORD(CSC_MODE_SEL)
} CSC_MODE_SEL;

typedef enum 
{
    PLANAR = 0,
    PACKED = 1,
	ENUM_DUMMY4WORD(CSC_OUT_MODE)
} CSC_OUT_MODE;

typedef enum 
{
    D65 = 0,
    D50 = 1,
	ENUM_DUMMY4WORD(LAB_FMT)
} LAB_FMT;

typedef enum 
{
    INTEGRAL_Y   = 0,
    INTEGRAL_UV  = 1,
    INTEGRAL_RGB = 2,
	ENUM_DUMMY4WORD(INTEGRAL_FMT)
} INTEGRAL_FMT;

typedef enum
{
	IVE_SUM = 0,			//Sum(U32)
	IVE_SQUARE = 1,			// Square(U64)
	IVE_SQUARE_SUM = 2,		//Square + Sum(U64)
	ENUM_DUMMY4WORD(INTEGRAL_MODE)
} INTEGRAL_MODE;

extern UINT32 g_uiIveFrmEndIntCnt;

/*----------- engine driver structure -------------*/
/*
    IVE Open Object - Open Object is used for opening IVE module.
*/
typedef struct _IVE_OPENOBJ {
	VOID (*FP_IVEISR_CB)(UINT32 uiIntStatus, VOID *ive_trig_hdl); ///< isr callback function
	UINT32 IVE_CLOCKSEL;
} IVE_OPENOBJ;

/**
    IVE output setting

    Structure of settings of output function
*/
//@{
typedef struct _IVE_FUNC_EN_STRUCT {
	BOOL bGenFiltEn;    ///< Enable general filter
	BOOL bMednFiltEn;   ///< Enable median filter
	BOOL bEdgeFiltEn;   ///< Enable edge filter
	BOOL bNonMaxEn;     ///< Enable non-maximal suppression
	BOOL bThresLutEn;   ///< Enable threshold LUT
	BOOL bMorphFiltEn;  ///< Enable morphological filter
	BOOL bIntegralEn;   ///< Enable integral image
	BOOL bHysteresisEn; ///< Enable hysteresis
	FUNCTION_MODE uiFunMode;   ///< FUNCTION_MODE
} IVE_FUNC_EN;
//@}

/**
    IVE DMA in info

    Structure of DMA input parameters
*/
//@{
typedef struct _IVE_DMA_IN_INFO_STRUCT {
	uintptr_t uiInSaddr0;        ///< Start address of input data
	uintptr_t uiInSaddr1;        ///< Start address of input data
	UINT32 uiInLofst0;        ///< Line offset of input data
	UINT32 uiInLofst1;        ///< Line offset of input data
} IVE_DMA_IN_INFO;
//@}

typedef struct _IVE_DMA_LL_INFO_STRUCT {
	uintptr_t uiLLAddr;        ///< Start address of input data
	uintptr_t uiLLBaseAddr;    ///< Start address of input data
} IVE_DMA_LL_INFO;

/**
    IVE DMA out info

    Structure of DMA output parameters
*/
//@{
typedef struct _IVE_DMA_OUT_INFO_STRUCT {
	uintptr_t uiOutSaddr0;       ///< Start address of output data
	uintptr_t uiOutSaddr1;       ///< Start address of output data
	uintptr_t uiOutSaddr2;       ///< Start address of output data
	UINT32 uiOutLofst0;       ///< line offset of output data
	UINT32 uiOutLofst1;       ///< line offset of output data
	UINT32 uiOutLofst2;       ///< line offset of output data
} IVE_DMA_OUT_INFO;
//@}

/**
    IVE image size info

    Structure of input image size
*/
//@{
typedef struct _IVE_IMG_SIZE_STRUCT {
	UINT32 uiWidth;              ///< input image width
	UINT32 uiHeight;             ///< input image height
} IVE_IMG_SIZE;
//@}

/**
    IVE general filter parameters

    Structure of general filter parameters
*/
//@{
typedef struct _IVE_GEN_FILT_PARAM_STRUCT {
	UINT32 puiGenCoeff[10];     ///< Pointer of general filter coefficient

} IVE_GEN_FILT_PARAM;
//@}

/**
    IVE edge filter parameters

    Structure of edge filter parameters
*/
//@{
typedef struct _IVE_EDGE_FILT_PARAM_STRUCT {
	EDGE_MODE uiEdgeMode;        ///< selection of edge type
	INT32 pEdgeCoeff1[25];      ///< Pointer of edge filter coefficient
	INT32 pEdgeCoeff2[25];      ///< Pointer of edge filter coefficient
	UINT32 uiEdgeShiftBit;    ///< shift bit to prevent overflow
	UINT32 uiAngSlpFact;      ///< angle slope factor of edge calculation
} IVE_EDGE_FILT_PARAM;
//@}

/**
	IVE lut parameters

	Structure of lut parameters
*/
//@{
typedef struct _IVE_THRES_LUT_TH_PARAM_STRUCT {
	INT16 low_th;
	INT16 high_th;
	INT8 min_val;
	INT8 mid_val;
	INT8 max_val;
} IVE_THRES_LUT_TH_PARAM;
//@}

/**
    IVE lut parameters 

    Structure of lut parameters  for mode0
*/
//@{
typedef struct _IVE_THRES_LUT_PARAM_STRUCT {
	UINT32 puiThresLut[15];            ///< pointer of threshold LUT values for mode0
} IVE_THRES_LUT_PARAM;
//@}

/**
	IVE hysteresis parameters

	Structure of hysteresis parameters
*/
//@{
typedef struct _IVE_HYSTERESIS_PARAM_STRUCT {
	INT8 min_val;
	INT8 mid_val;
	INT8 max_val;
} IVE_HYSTERESIS_PARAM;
//@}

/**
    IVE morphological filter parameters

    Structure of morphological filter parameters
*/
//@{
typedef struct _IVE_MORPH_FILT_PARAM_STRUCT {
	MORPH_OP uiMorphOp;       ///< operation of morphological filter
	BOOL pbMorphNeighEn[24];    ///< Pointer of neighbor enable array
} IVE_MORPH_FILT_PARAM;
//@}

/**
    IVE edge filter parameters

    Structure of edge filter parameters
*/
//@{
typedef struct _IVE_CSC_PARAM_STRUCT {
	YUV_MODE uiYuvMode;       ///< selection of yuv input format
	CSC_MODE_SEL uiModeSel;   ///< selection of transfer format
	CSC_OUT_MODE uiOutMode;   ///< selection of output format
	INT32 pYuvCoeff[9];         ///< Pointer of yuv2rgb coefficient
	INT32 pYuvBias[3];          ///< Pointer of yuv2rgb bias
	UINT32 uiHueShift;    	  ///< shift bit for HUE
	LAB_FMT uiLabFmt;         ///< selection of LAB format
	BOOL uiGammaEn;
} IVE_CSC_PARAM;
//@}

typedef enum {
	IVE_HISTOGRAM = 0,		 //histogram
	IVE_CDF = 1,			 //cdf_no_dump
	IVE_CDF_DUMP = 2,	     //cdf_dump
	ENUM_DUMMY4WORD(KDRV_IVE_HISTOGRAM_MODE)
} IVE_HISTOGRAM_MODE;
//@}

typedef enum {
	ADD = 0,		 //add
	SUB = 1,		 //sub
	AND = 2,		 //and
	OR = 3,		 //or
	XOR = 4,		 //xor
	ENUM_DUMMY4WORD(IVE_IMG_OPERATION_MODE)
} IVE_IMG_OPERATION_MODE;
//@}

typedef struct _IVE_ADD_PARM_STRUCT {
	UINT8 shift_bit; //[0,16]
	UINT16 coef_a;
	UINT16 coef_b;
} IVE_ADD_PARM;

//@}

typedef struct _IVE_SUB_PARM_STRUCT {
	BOOL sub_mode;
} IVE_SUB_PARM;
//@}

typedef enum {
	MODE_0 = 0,		 //MODE_0
	MODE_1 = 1,		 //MODE_1
	MODE_2 = 2,		 //MODE_2
	MODE_3 = 3,		 //MODE_3
	MODE_4 = 4,		 //MODE_4 histogram
	ENUM_DUMMY4WORD(IVE_16bitTO8bit_MODE)
} IVE_16bitTO8bit_MODE;

typedef struct _IVE_16bitTO8bit_PARM_STRUCT {
	UINT8 shift_bit; //[0,31]
	UINT32 coef_a;//U24
	UINT32 coef_b;
	INT8 bias;//-128-127
} IVE_16bitTO8bit_PARM;

//@}


typedef struct _IVE_DMA_CP_PARM_STRUCT {
	UINT32 inc_h; 
	UINT32 inc_v;
	UINT32 b_num;
	UINT32 h_cnt;
} IVE_DMA_CP_PARM;
//@}


typedef struct _IVE_ST_PARM_STRUCT {
	BOOL stage;
	BOOL hw_corner_filter;
	UINT8 quality_level;
	UINT8 min_dist;
	UINT16 max_corner_num; //1-500
	UINT32 local_ker_size;
	UINT16 sort_topn;//1-1500
	UINT16 max_eigen_value;
	UINT16 out_corner_num;
} IVE_ST_PARM;
//@}



typedef struct _IVE_CCL_PARM_STRUCT {
	BOOL stage;
	BOOL connect_mode;
	UINT8 foreground_val;
	UINT16 region_x_num; //1-5
	UINT16 region_y_num;//1-256
	UINT16 region_width;
	UINT16 region_height;
} IVE_CCL_PARM;
//@}

/**
    IVE flow control

    Structure of flow control
*/
//@{
typedef struct _IVE_FLOW_CT_PARAM_STRUCT {
	UINT32 dma_do_not_sync;            ///< do not cache flush
} IVE_FLOW_CT_PARAM;
//@}

/**
    IVE engine parameter for library

    Structure of all avaliable engine parameter
*/
//@{
typedef struct _IVE_PARAM_STRUCT {
	IVE_FUNC_EN FuncEn;             ///< Enable function of IVE
	MORPH_IN_SEL uiMorphInSel;      ///< selection of input data flow
	OUT_DATA_TYPE uiOutDataSel;     ///< selection of output data flow
	OUT1_DATA_TYPE uiOutDataSel_1;     ///< selection of output data flow
	IVE_DMA_IN_INFO DmaIn;          ///< Info of DMA frame mode input
	IVE_DMA_OUT_INFO DmaOut;        ///< Info of DMA frame mode output
	IVE_IMG_SIZE InSize;            ///< Input image size
	
	IVE_GEN_FILT_PARAM GenF;        ///< general filter parameters
	MEDN_MODE uiMednMode;           ///< median filter mode
	IVE_EDGE_FILT_PARAM EdgeF;      ///< edge filter parameters
	UINT32 uiEdgeMagTh;             ///< edge threshold for non-maximal suppression

	UINT32 uiLutMode;
	IVE_THRES_LUT_PARAM Thres;		///< threshold LUT for mode0
	IVE_THRES_LUT_TH_PARAM Thres_Th;///< threshold LUT

	IVE_MORPH_FILT_PARAM MorphF;    ///< morphological filter parameters
	INTEGRAL_FMT uiIntegralFmt;	    ///< integral parameters
	INTEGRAL_MODE uiIntegralMode;	///< integral parameters
	IVE_CSC_PARAM CscF;				///< CSC parameters

	IVE_HISTOGRAM_MODE HistogramMode; ///< histogram parameters
	UINT32 histogram_cdf_min;         ///< histogram result

	IVE_HYSTERESIS_PARAM Hysteresis;///< hysteresis parameters

	IVE_IMG_OPERATION_MODE ImgOperationMode;
	IVE_ADD_PARM AddOp;
	IVE_SUB_PARM SubMode;

	IVE_16bitTO8bit_MODE BitClampMode;
	IVE_16bitTO8bit_PARM BitClampParm;
	
	IVE_DMA_CP_PARM DmaCpParm;
	IVE_ST_PARM StParm;
	IVE_CCL_PARM CclParm;

    IVE_DMA_LL_INFO LLDmaIn;        ///< Info of DMA frame mode input
	BURST_LEN_SEL InBurstSel;       ///< Input burst length selection
	BURST_LEN_SEL OutBurstSel;      ///< Output burst length selection
	//IVE_DBG_PORT_SEL DbgPortSel;    ///< Debug port selection
	IVE_FLOW_CT_PARAM flowCT;       ///< flow control: Dma do not sync     
} IVE_PARAM;
//@}


//external function prototype

//engine control
extern ER ive_stateMachine(IVE_ACTION_OP RdeOp, BOOL bUpdate);
extern VOID ive_setSWReset(VOID);
extern ER ive_open(IVE_OPENOBJ *pIveObjCB);//Available clock rate(MHz): 240, 480, PLL6, PLL13
extern ER ive_start(VOID);
extern ER ive_setMode(IVE_PARAM *pIveParam);
extern ER ive_pause(VOID);
extern ER ive_close(VOID);
extern ER ive_waitFrameEnd(VOID);
extern ER ive_clearFrameEndFlag(VOID);
extern VOID ive_isr(VOID);
extern VOID ive_waitFrameEnd_LL(BOOL is_clr_flag);
extern VOID ive_clearFrameEndFlag_LL(VOID);
extern ER ive_ll_start(VOID);
extern ER ive_ll_setmode(IVE_DMA_LL_INFO *pIveLLParam);
extern ER ive_ll_pause(VOID);
//get settings
extern UINT32 ive_getClockRate(VOID);
extern uintptr_t ive_getDmaInAddr(VOID);
extern uintptr_t ive_getDmaOutAddr(VOID);

extern VOID ive_create_resource(VOID *parm, UINT32 clk_freq);
extern VOID ive_release_resource(VOID *parm);
extern VOID ive_set_base_addr(uintptr_t addr);
extern uintptr_t ive_get_base_addr(VOID);

extern UINT32 ive_getIntStatus(VOID);
extern ER ive_enableInt(UINT32 uiIntrEn);
extern ER ive_clearIntStatus(UINT32 uiIntrStatus);


extern VOID ive_setTriHdl(VOID *ive_trig_hdl);
extern VOID * ive_getTriHdl(VOID);
extern UINT32 ive_getTriHdl_flagId(VOID);
extern UINT32 ive_get_clock_rate(VOID);

extern UINT32 ive_get_hist_cdf_min(VOID);
extern UINT64 ive_get_ncc_numerator(VOID);
extern UINT64 ive_get_ncc_quad_sum0(VOID);
extern UINT64 ive_get_ncc_quad_sum1(VOID);
extern UINT32 ive_get_st_corner_output_num(VOID);
extern UINT32 ive_get_st_max_eigen_val(VOID);
ER kdrv_ive_chk_efuse_ive_eng(VOID);

extern INT32 ive_init(VOID);
extern INT32 ive_uninit(VOID);
/*
 *#ifdef __cplusplus
 *}
 *#endif
 */

#endif

//@}
