/**
    Public header file for DRE module.

    @file       dre_eng.h
    @ingroup    mIIPPDRE

    @brief

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/
#ifndef _DRE_ENG_H
#define _DRE_ENG_H

#ifdef __cplusplus
extern "C" {
#endif


//#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/nvt_type.h"//a header for basic variable type
#include "kdrv_type.h"
#include "dre_eng_limt.h"
#include "dre_eng_int.h"

#define DRE_SSD_DRV_NAME "Ssdrv_dre"
#define DRE_SSD_DRV_MODULE_VERSION "1.00.10"
/*****************************************************************************/




#if defined (__LINUX)

#elif defined (__FREERTOS)
//#include "kwrap/flag.h"
#endif


#define PAT_GEN 0

#define DRE_ENG_REG_NUM	196//194

//#define LAYER_MAX_NUM 10
#define DRE_IMG_IN_DMA_NUM   4
#define DRE_IMG_OUT_DMA_NUM  3

#define FUSION_WT_TABLE_NUM 17
#define FUSION_FILT_TAP		3
#define PATCH_PXLMSK_COLOR	3

#define NR_F5X5_WT			6
#define NR_RTH_TAP			8
#define COLOR3				3
#define MSNR_TABLE_NUM		128
#define YCMOD_TAP			16
//#define PYMD_MIN_WIDTH	5
//#define PYMD_MIN_HEIGHT	5
#define DRE_FUSION_IN_NUM   2




/*****************************************************************************/
typedef void (*DRE_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef enum {
	DRE_ENG_START_LOAD = 0,
	DRE_ENG_FRAME_START_LOAD,
	DRE_ENG_FRAME_END_LOAD,
	DRE_ENG_GLOBAL_LOAD,
} DRE_ENG_LOAD_TYPE;

typedef enum {
	DRE_ENG_DRAM_WT = 0,
	DRE_ENG_LUT_WT,
} DRE_ENG_WT_MAP_SRC;

typedef enum _DRE_ENG_ID {
	DRE_ID  = 0,
	DRE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(DRE_ENG_ID)
} DRE_ENG_ID;


typedef struct {
	UINT32 ofs;
	UINT32 val;
} DRE_ENG_REG;

/**
    @addtogroup mIDrvIPPDRE
*/
//@{


//------enum------

typedef enum {
	DRE_ENG_INTERRUPT_END			= 0x00000001,
	DRE_ENG_INTERRUPT_TIMEOUT		= 0x00000002,
	DRE_ENG_INTERRUPT_LL_END		= 0x00000004,
	DRE_ENG_INTERRUPT_LL_ERROR		= 0x00000008,	
	DRE_ENG_INTERRUPT_LL_JOBEND		= 0x00000010,

	DRE_ENG_INTERRUPT_ALL 			= (DRE_ENG_INTERRUPT_END |
									DRE_ENG_INTERRUPT_TIMEOUT |
									DRE_ENG_INTERRUPT_LL_END |
									DRE_ENG_INTERRUPT_LL_ERROR |
									DRE_ENG_INTERRUPT_LL_JOBEND)
} DRE_ENG_INTERRUPT;

/**
    Process type of DRE engine
*/
//@{
typedef enum {
	DRE_NR  = 0,
	DRE_FUSION = 1
} PROC_TYPE;
//@}

/**
    Input image type of DRE engine
*/
//@{
typedef enum {
	DRE_YUV422_COMP = 0,    ///<Complete process of YUV422 pack input
	DRE_YUV422_SIMP = 1,    ///<Simplified process of YUV422 pack input
	DRE_YUV420      = 2     ///<Complete process of YUV420 pack input
} DRE_IMG_TYPE;
//@}

/**
    DRE input image ID
*/
typedef enum {
	IN_0 = 0,
	IN_1 = 1
} DRE_INIMG_ID;

//------struct-------

/**
    DRE image size parameter

    Structure of input image size
*/
//@{
typedef struct _DRE_IMG_SIZE_STRUCT {
	UINT16 width;
	UINT16 height;
} DRE_IMG_SIZE;
//@}


//@}

/**
	Define DRE interrput mask.
*/
//@{
/*#define DRE_ENG_INT_FRAME_END		0x00000001  ///< Status of interrupt of frame end
#define DRE_ENG_INT_TIME_OUT		0x00000002  ///< Enable interrupt of time out
#define DRE_ENG_INT_TIME_OUT		0x00000004  ///< Enable interrupt of time out
#define DRE_ENG_INT_ALL				0x00000003  ///< Enable all interrupt*/

//@}

/*----------- engine control enumeration ------------*/
/**
    State of State Machine
*/
//@{
typedef enum {
	DRE_ENGINE_IDLE  = 0, ///< Idel state
	DRE_ENGINE_READY = 1, ///< Ready state
	DRE_ENGINE_PAUSE = 2, ///< Pause state
	DRE_ENGINE_RUN   = 3, ///< Run state

	ENUM_DUMMY4WORD(DRE_STATE_MACHINE)
} DRE_STATE_MACHINE;
//@}

/**
    Operations of engine action
*/
//@{
typedef enum {
	DRE_OP_OPEN      = 0, ///< Open engine
	DRE_OP_CLOSE     = 1, ///< Close engine
	DRE_OP_SETPARAM  = 2, ///< Set parameter
	DRE_OP_SET2READY = 3, ///< Set to ready
	DRE_OP_START     = 4, ///< Start engine
	DRE_OP_PAUSE     = 5, ///< Pause engine

	ENUM_DUMMY4WORD(DRE_ACTION_OP)
} DRE_ACTION_OP;
//@}

/**
    Engine load type
*/
//@{
typedef enum {
	DRE_START_LOAD  = 0, ///< Start load, available
	DRE_FRMEND_LOAD = 1, ///< Frame end load, not available
	DRE_IMD_LOAD    = 2, ///< Immediately load, not available

	ENUM_DUMMY4WORD(DRE_LOAD_TYPE)
} DRE_LOAD_TYPE;
//@}

/**
    Color image channel name
*/
//@{
typedef enum {
	Y_CH = 0,
	U_CH = 1,
	V_CH = 2
} YUV_CH_NAME;
//@}

/**
    engine process type
*/
//@{
typedef enum {
	DCM_PROC = 0,
	RCS_PROC = 1
} DRE_PROC_TYPE;
//@}

/**
    Engine decomposition mode
*/
//@{
typedef enum {
	DCM_MODE0 = 0, ///< Decomposition mode 0
	DCM_MODE1 = 1, ///< Decomposition mode 1
	DCM_MODE2 = 2, ///< Decomposition mode 2
	DCM_MODE3 = 3, ///< Decomposition mode 3
	DCM_MODE4 = 4, ///< Decomposition mode 4

	ENUM_DUMMY4WORD(DRE_DCM_MODE)
} DRE_DCM_MODE;
//@}

/**
    Engine reconstruction mode
*/
//@{
typedef enum {
	RCS_MODE0 = 0, ///< Reconstruction mode 0
	RCS_MODE1 = 1, ///< Reconstruction mode 1
	RCS_MODE2 = 2, ///< Reconstruction mode 2
	RCS_MODE3 = 3, ///< Reconstruction mode 3
	RCS_MODE4 = 4, ///< Reconstruction mode 4

	ENUM_DUMMY4WORD(DRE_RCS_MODE)
} DRE_RCS_MODE;
//@}

/**
    Engine input image number
*/
//@{
typedef enum {
	ONE_IMG = 0, ///< One input image of decomposition
	TWO_IMG = 1, ///< Two input image of decomposition

	ENUM_DUMMY4WORD(DRE_DCM_IN_NUM)
} DRE_DCM_IN_NUM;
//@}

/*
    dram in/out burst length
*/
//@{
typedef enum {
	DRE_BURST_64W = 0, ///< burst length 64 word
	DRE_BURST_48W = 1, ///< burst length 48 word
	DRE_BURST_32W = 2, ///< burst length 32 word
	DRE_BURST_16W = 3, ///< burst length 16 word

	ENUM_DUMMY4WORD(DRE_ENG_BURST_LEN)
} DRE_ENG_BURST_LEN;
//@}

/*----------- engine driver structure -------------*/
/*
    DRE Open Object - Open Object is used for opening DRE module.
*/
typedef struct _DRE_OPENOBJ {
	void (*FP_DREISR_CB)(UINT32 uiIntStatus); ///< isr callback function
	UINT32 DRE_CLOCKSEL;
} DRE_OPENOBJ;

typedef struct _DRE_ENG_DCM_FUNC_EN{
	BOOL img_in_num;      ///< Enable one or two input image under decomposition process
	UINT8 dcm_mode;     ///< Select mode under decomposition process
	BOOL filt_en;       ///< Enable filter under decomposition process
	BOOL nr_lut_y_en;     ///< Enable noise reduction LUT Y under decomposition process
	BOOL nr_lut_u_en;     ///< Enable noise reduction LUT U under decomposition process
	BOOL nr_lut_v_en;     ///< Enable noise reduction LUT V under decomposition process
	BOOL wt_map_sel;   ///< Enable deghost function under decomposition process
	BOOL hard_wt_en;     ///< Enable hard weighting
} DRE_ENG_DCM_FUNC_EN;

typedef struct _DRE_RCS_FUNC_EN_STRUCT {
	UINT8 rcs_mode;   ///< Select mode under reconstruction process
	BOOL ycmod_en;    ///< Enable YC modification under reconstruction process
} DRE_ENG_RCS_FUNC_EN;

typedef struct {
	BOOL proc_sel;          ///< Enable decomposition or reconstruction process
	DRE_ENG_DCM_FUNC_EN dcm;    ///< Function settings of decomposition proecss
	DRE_ENG_RCS_FUNC_EN rcs;    ///< Function settings of reconstruction proecss
} DRE_ENG_FUNC_EN;

typedef struct _DRE_ENG_LAYER_SIZE{
	UINT16 layer0_width;   ///< Width of layer 0 image
	UINT16 layer0_height;  ///< height of layer 0 image
	UINT16 layer1_width;   ///< Width of layer 1 image
	UINT16 layer1_height;  ///< height of layer 1 image
} DRE_ENG_LAYER_SIZE;

typedef struct _DRE_ENG_DRAM_IN_INFO{
	ULONG in_addr[DRE_IMG_IN_DMA_NUM];   ///< Dram address of image input
	UINT16 in_lofst[DRE_IMG_IN_DMA_NUM];  ///< Dram line offset of image input
	ULONG in_wt_addr;         ///< Dram address of weight map input
	UINT16 in_wt_lofst;        ///< Dram line offset of weight map input
	ULONG in_patch_pxlmsk_addr;  ///< Dram address of patch_pxkmsk input
	UINT16 in_patch_pxlmsk_lofst;///< Dram line offset of patch_pxkmsk map input


	
} DRE_ENG_DRAM_IN_INFO;

typedef struct _DRE_ENG_DRAM_OUT_INFO{
	ULONG out_addr[DRE_IMG_OUT_DMA_NUM];  ///< Dram address of image output
	UINT16 out_lofst[DRE_IMG_OUT_DMA_NUM]; ///< Dram line offset of image output
	ULONG out_wt_addr;         ///< Dram address of weight map input
	UINT16 out_wt_lofst;        ///< Dram line offset of weight map input
} DRE_ENG_DRAM_OUT_INFO;

typedef struct _DRE_ENG_SCALE_FACT{
	UINT32 h_sdn_factor;   ///< scaling down factor of horizontal direction
	UINT32 v_sdn_factor;   ///< scaling down factor of vertical direction
	UINT32 h_sup_factor;   ///< scaling up factor of horizontal direction
	UINT32 v_sup_factor;   ///< scaling up factor of vertical direction
} DRE_ENG_SCALE_FACT;

typedef struct _DRE_ENG_SCALE_OFST{
	UINT32 h_sdn_offset;  ///< offset of horizontal scaling down factor
	UINT32 v_sdn_offset;  ///< offset of vertical scaling down factor
} DRE_ENG_SCALE_OFST;

typedef struct _DRE_ENG_FUSION_WT_TAB{
	UINT8 wt[FUSION_WT_TABLE_NUM];  ///< Spatial weight
} DRE_ENG_FUSION_WT_TAB;

typedef struct _DRE_ENG_FUSION_FILT{
	INT16 img0_spa_coeff[FUSION_FILT_TAP];
	INT16 img1_spa_coeff[FUSION_FILT_TAP];
} DRE_ENG_FUSION_FILT;

typedef struct _DRE_ENG_PATCH_INFO{
	UINT8 color[PATCH_PXLMSK_COLOR];
} DRE_ENG_PATCH_INFO;

/**
    DRE NR joint-bilateral filter info

    Structure of one image NR spatial filter

    5x5:
    w5 w4 w3 w4 w5
    w4 w2 w1 w2 w4
    w3 w1 w0 w1 w3
    w4 w2 w1 w2 w4
    w5 w4 w3 w4 w5
*/
typedef struct _DRE_ENG_NR_FILT{
	UINT8 s_weight[NR_F5X5_WT];   ///< Spatial weight
	UINT8 rth_y_lut[NR_RTH_TAP];     ///< Table of range filter threshold, please assign a UINT32[RTH_TAP] array
	UINT8 rth_u_lut[NR_RTH_TAP];     ///< Table of range filter threshold, please assign a UINT32[RTH_TAP] array
	UINT8 rth_v_lut[NR_RTH_TAP];     ///< Table of range filter threshold, please assign a UINT32[RTH_TAP] array
} DRE_ENG_NR_FILT;

typedef struct _DRE_ENG_NR_JOINT_OUTL{
	UINT8 joint_y[COLOR3];  ///< Enable of joint channel reference
	UINT8 joint_u[COLOR3];  ///< Enable of joint channel reference
	UINT8 joint_v[COLOR3];  ///< Enable of joint channel reference
	UINT8 outl_en[COLOR3];   ///< Enable of outlier detection of each channel
} DRE_ENG_NR_JOINT_OUTL;

typedef struct _DRE_ENG_MSNR_LUT{
	UINT8 y_lut[MSNR_TABLE_NUM];
	UINT8 u_lut[MSNR_TABLE_NUM];
	UINT8 v_lut[MSNR_TABLE_NUM];
} DRE_ENG_MSNR_LUT;

typedef struct _DRE_ENG_YCMOD_TBL{
	UINT8 ylut[YCMOD_TAP];      ///< Table of YC modify LUT for Y, please assign a UINT32[YCMOD_TAP] array
	UINT8 clut[YCMOD_TAP];      ///< Table of YC modify LUT for C, please assign a UINT32[YCMOD_TAP] array
} DRE_ENG_YCMOD_TBL;

typedef struct _DRE_ENG_YCMOD_CTRL{
	UINT16 cb_ofs;
	UINT16 cr_ofs;
	UINT16 step_y;
	UINT16 step_c;
	UINT8 mod_sel;
	UINT8 mod_lut_sel;
} DRE_ENG_YCMOD_CTRL;


//extern volatile NT98690_DRE_ENG_REG_STRUCT *p_int_dre_reg;
//extern volatile NT98690_DRE_ENG_REG_STRUCT *p_dre_reg;
//extern volatile UINT8 *p_dre_reg_chg_flag;
//----------------------------------------------

typedef struct _BUF_ADDR_STRUCT {
	ULONG pa;
	ULONG va;
} BUF_ADDR;

typedef struct _DRAM_INFO_STRUCT {
	BUF_ADDR addr;
	UINT32 lofst;
} DRAM_INFO;

typedef struct _DRE_LAP_BUFF_STRUCT {
	DRAM_INFO yuv_buffer[LAYER_MAX_NUM];
} DRE_LAP_BUFF;

typedef struct _DRE_GAU_BUFF_STRUCT {
	DRAM_INFO yuv_buffer[LAYER_MAX_NUM];
	DRAM_INFO uv_buffer[2];
} DRE_GAU_BUFF;

typedef struct _DRE_IMG_DRAM_PARAM_STRUCT {
	DRAM_INFO y_buffer;
	DRAM_INFO uv_buffer;
} DRE_IMG_DRAM_PARAM;

typedef struct _DRE_WORK_INFO_STRUCT {
	ULONG work_buf_size;
	UINT8 real_layer_num;
    UINT32 sw_trig_layer_bitmap;
} DRE_WORK_INFO;

typedef struct {
    ULONG addr;
    ULONG size;
}BUF_INFO;

typedef struct{
    BUF_INFO in_buf[DRE_IMG_IN_DMA_NUM];
    BUF_INFO out_buf[DRE_IMG_OUT_DMA_NUM];
    BUF_INFO wt_in_buf;
}DRE_SW_WORK_BUF;


typedef struct _DRE_CONFIG {
	DRE_IMG_SIZE img_size;
} DRE_CONFIG;


typedef struct _DRE_DRV_CFG {
	UINT8 real_layer_num;
	DRE_IMG_SIZE layer_size[LAYER_MAX_NUM];
	BOOL bSw_trig;
} DRE_DRV_CFG;

/*
    DRE pyramid info
*/
typedef struct _DRE_ENG_PYRAMID_INFO {
	UINT8 layer_num;
	DRE_IMG_SIZE img_size;
	DRE_IMG_TYPE img_type;
	DRE_DCM_IN_NUM img_num;
	DRE_IMG_SIZE layer_size[LAYER_MAX_NUM];
	DRE_LAP_BUFF lap_buffer;
	DRE_GAU_BUFF gau_buffer[2];
	DRAM_INFO wt_map_out_buffer[LAYER_MAX_NUM];
	DRE_IMG_DRAM_PARAM img_in[2];
	DRE_IMG_DRAM_PARAM img_out;
	DRAM_INFO wt_map_in;
	DRE_WORK_INFO work_info;
} DRE_ENG_PYRAMID_INFO;

typedef struct _DRE_NR_PARAM_STRUCT {
	DRE_IMG_TYPE  type;
	DRE_IMG_SIZE img_size;
	BUF_ADDR work_buf_addr;
	UINT8 layer_num;
	DRE_IMG_DRAM_PARAM img_in;
	DRE_IMG_DRAM_PARAM img_out;    
} DRE_NR_PARAM;

typedef struct _DRE_FUSION_PARAM_STRUCT {
	DRE_IMG_TYPE type;
	DRE_IMG_SIZE img_size;
	BUF_ADDR work_buf_addr;
	UINT8 layer_num;
	DRE_IMG_DRAM_PARAM img_in[2];
	DRE_IMG_DRAM_PARAM img_out;
	DRAM_INFO wt_map_in;        
} DRE_FUSION_PARAM;


typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	//SEM_HANDLE sem;
	//ID flg_id;
	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */
	ULONG reg_io_base;

	volatile NT98690_DRE_ENG_REG_STRUCT *p_dre_reg;
	volatile UINT8 *p_dre_reg_chg_flag;

	UINT32 irq_id;
	DRE_ISR_CB isr_cb;
} DRE_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	DRE_ENG_HANDLE *p_eng;
} DRE_ENG_CTL;

typedef struct{
	DRE_IMG_SIZE img_in_size;
	ULONG img_in_addr[DRE_FUSION_IN_NUM];//virtual address
	UINT16 img_in_lofst[DRE_FUSION_IN_NUM];
    ULONG wt_in_addr;//virtual
	UINT16 wt_in_lofst;
	ULONG img_out_addr;//virtual address
	UINT16 img_out_lofst;	
	//add cfg infor at here
}DRE_ENG_SW_DCM_INFO;

extern INT32 dre_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 dre_eng_release(void);
extern DRE_ENG_HANDLE* dre_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern INT32 dre_eng_reg_isr_callback(DRE_ENG_HANDLE *p_eng, DRE_ISR_CB cb);

extern void dre_eng_set_axi_disable_hw_reg(DRE_ENG_HANDLE *p_eng, BOOL enable);
extern INT32 dre_eng_open(DRE_ENG_HANDLE *p_eng);
extern INT32 dre_eng_close(DRE_ENG_HANDLE *p_eng);
extern void dre_eng_trig_single_hw_reg(DRE_ENG_HANDLE *p_eng);
extern void dre_eng_trig_ll_hw_reg(DRE_ENG_HANDLE *p_eng, ULONG ll_addr);
extern void dre_eng_stop_single_hw_reg(DRE_ENG_HANDLE *p_eng);
extern void dre_eng_stop_ll_hw_reg(DRE_ENG_HANDLE *p_eng);

extern void dre_eng_write_hw_reg(DRE_ENG_HANDLE *p_eng, UINT32 reg_ofs, UINT32 val);
extern void dre_eng_hard_reset_hw_reg(DRE_ENG_HANDLE *p_eng);


extern void dre_eng_wait_flag_frame_end(BOOL is_clear_flag);
extern void dre_eng_wait_flag_linked_list_end(BOOL is_clear_flag);
extern void dre_eng_clear_flag_linked_list_job_end(void);
extern INT32 dre_eng_chk_limitation(ULONG reg_base_addr, ULONG reg_flag_addr);

extern void dre_eng_isr_hw_reg(DRE_ENG_HANDLE *p_eng);
extern INT32 dre_eng_init_resource(DRE_ENG_HANDLE *p_eng);

extern void dre_eng_dump(void);


extern UINT32 dre_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 dre_eng_get_reg_flag_buf_size(UINT32 eng_id);
extern void dre_eng_set_reg_buf(DRE_ENG_HANDLE *p_eng, ULONG reg_base_addr, ULONG reg_flag_addr);

extern void dre_eng_dma_channel_enable_hw_reg(DRE_ENG_HANDLE *p_eng, BOOL set_en);
extern UINT8 dre_eng_get_dma_channel_status_hw_reg(DRE_ENG_HANDLE *p_eng);

extern void dre_eng_set_time_limit(DRE_ENG_HANDLE *p_eng, UINT32 run_time);

// parameter setting API

// engine control API
extern void dre_eng_set_function_procsel(DRE_ENG_HANDLE *p_eng, BOOL proc_sel);
extern void dre_eng_set_dcm_img_in_num(DRE_ENG_HANDLE *p_eng, DRE_DCM_IN_NUM img_in_num);
extern void dre_eng_set_dcm_mode(DRE_ENG_HANDLE *p_eng, DRE_DCM_MODE mode);
extern void dre_eng_set_dcm_wt_map_sel(DRE_ENG_HANDLE *p_eng, BOOL from_wt_tbl);
extern void dre_eng_set_dcm_filt_en(DRE_ENG_HANDLE *p_eng, BOOL func_en);
extern void dre_eng_set_dcm_nr_ylut_en(DRE_ENG_HANDLE *p_eng, BOOL func_en);
extern void dre_eng_set_dcm_nr_ulut_en(DRE_ENG_HANDLE *p_eng, BOOL func_en);
extern void dre_eng_set_dcm_nr_vlut_en(DRE_ENG_HANDLE *p_eng, BOOL func_en);
extern void dre_eng_set_rcs_mode(DRE_ENG_HANDLE *p_eng, DRE_RCS_MODE mode);
extern void dre_eng_set_rcs_ycmod_en(DRE_ENG_HANDLE *p_eng, BOOL func_en);
extern void dre_eng_set_dcm_hard_wt_en(DRE_ENG_HANDLE *p_eng, BOOL func_en);


extern void dre_eng_set_img_Size(DRE_ENG_HANDLE *p_eng, DRE_ENG_LAYER_SIZE *p_img_size);
extern void dre_eng_set_dram_in(DRE_ENG_HANDLE *p_eng, DRE_ENG_DRAM_IN_INFO *p_dram_in);
extern void dre_eng_set_dram_out(DRE_ENG_HANDLE *p_eng, DRE_ENG_DRAM_OUT_INFO *p_dram_out);
extern void dre_eng_set_sc_fact(DRE_ENG_HANDLE *p_eng, DRE_ENG_SCALE_FACT *p_sc_fact);
extern void dre_eng_set_sc_ofst(DRE_ENG_HANDLE *p_eng, DRE_ENG_SCALE_OFST *p_sc_ofst);
extern void dre_eng_set_fusion_wt_tbl(DRE_ENG_HANDLE *p_eng, DRE_ENG_FUSION_WT_TAB *p_fusion_wt_tab);
extern void dre_eng_set_fusion_filt(DRE_ENG_HANDLE *p_eng, DRE_ENG_FUSION_FILT *p_fusion_filt);
extern void dre_eng_set_nr_filt(DRE_ENG_HANDLE *p_eng, DRE_ENG_NR_FILT *p_nr_filt);
extern void dre_eng_set_nr_jnt_outl(DRE_ENG_HANDLE *p_eng, DRE_ENG_NR_JOINT_OUTL *p_jnt_outl);
extern void dre_eng_set_nr_msnr_lut(DRE_ENG_HANDLE *p_eng, DRE_ENG_MSNR_LUT *p_msnr_lut);
extern void dre_eng_set_rcs_ycmod_lut(DRE_ENG_HANDLE *p_eng, DRE_ENG_YCMOD_TBL *p_ycmod_tble);
extern void dre_eng_set_rcs_ycmod_ctrl(DRE_ENG_HANDLE *p_eng, DRE_ENG_YCMOD_CTRL *p_ycmod_ctrl);

extern void dre_eng_set_int_en_reg_buf_reg(DRE_ENG_HANDLE *p_eng, UINT32 inte_en);

// burst length API
extern void dre_eng_set_burst_len(DRE_ENG_HANDLE *p_eng, DRE_ENG_BURST_LEN *p_burst_sel);
extern void dre_eng_set_dbg_level(UINT32 level);
extern BOOL dre_ssdrv_proc_ver(void);

extern INT32 dre_eng_do_sw_dcm_dre(DRE_ENG_SW_DCM_INFO sw_dcm_info);
extern INT32 dre_eng_layer_config(DRE_CONFIG *p_dre_info, DRE_DRV_CFG *p_drv_cfg);
extern void dre_eng_set_patch_pxlmsk_color(DRE_ENG_HANDLE *p_eng, DRE_ENG_PATCH_INFO *p_patch_info);
extern void dre_eng_set_patch_pxlmsk_en(DRE_ENG_HANDLE *p_eng, BOOL patch_pxlmsk_en);


#ifdef __cplusplus
}
#endif

#endif

//@}
