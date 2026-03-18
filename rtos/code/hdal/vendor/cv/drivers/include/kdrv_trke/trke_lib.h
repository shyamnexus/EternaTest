/*
    Public header file for TRKE module.

    This file is the header file that define the API and data type for TRKE
    module.

    @file       trke_lib.h
    @ingroup    mIDrvIPPTRKE
    @note

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#ifndef _TRKE_LIB_H
#define _TRKE_LIB_H

/*
 *#ifdef __cplusplus
 *extern "C" {
 *#endif
 */

#include "kdrv_type.h"

//#include    "Type.h"//a header for basic variable type

/**
    @addtogroup mIDrvIPPTRKE
*/
//@{

/**
    Define TRKE interrput enable.
*/
//@{
#define TRKE_INTE_FRM_END                    0x00000001  ///< Enable interrupt of frame end
#define TRKE_INTE_LLEND                      0x00000100  ///< Status of interrupt of frame end(LL)
#define TRKE_INTE_LLERROR                    0x00000200  ///< Status of interrupt of frame end(LL)
#define TRKE_INTE_LLJOBEND                   0x00000400  ///< Status of interrupt of frame job_end(LL)
#define TRKE_INTE_ALL                        0x00000701  ///< Enable all interrupts

//@}

/**
    Define TRKE interrput status.
*/
//@{
#define TRKE_INT_FRM_END                     0x00000001  ///< Status of interrupt of frame end
#define TRKE_INT_LLEND                       0x00000100  ///< Status of interrupt of frame end(LL)
#define TRKE_INT_LLERROR                     0x00000200  ///< Status of interrupt of frame end(LL)
#define TRKE_INT_LLJOBEND                    0x00000400  ///< Status of interrupt of frame job_end(LL)
#define TRKE_INT_ALL                         0x00000701  ///< Status of all interrupts
//@}

#define TRKE_MAX_INPUT_NUM   4
#define TRKE_MAX_OUTPUT_NUM  2
/*----------- enumeration ------------*/
/**
    Operations of engine action
*/
//@{
typedef enum {
	TRKE_OP_OPEN          = 0,    ///< Open engine
	TRKE_OP_CLOSE         = 1,    ///< Close engine
	TRKE_OP_SETPARAM      = 2,    ///< Set parameter
	TRKE_OP_START         = 3,    ///< Start engine
	TRKE_OP_PAUSE         = 4,    ///< Pause engine

	ENUM_DUMMY4WORD(TRKE_ACTION_OP)
} TRKE_ACTION_OP;
//@}

typedef enum {
	TRKE_DBG_PORT1 = 0,
	TRKE_DBG_PORT2 = 1,
	TRKE_DBG_PORT3 = 2,
	TRKE_DBG_PORT4 = 3,

	ENUM_DUMMY4WORD(TRKE_DBG_PORT_SEL)
} TRKE_DBG_PORT_SEL;

/*
    Operations of engine action
*/
typedef enum {
	TRKE_START_LOAD  = 0,    ///< Load parameter at engine start(enable 0->1)
	TRKE_FRMEND_LOAD = 1,    ///< Load parameter at engine frame end(frame end interrupt occur)

	ENUM_DUMMY4WORD(TRKE_LOAD_TYPE)
} TRKE_LOAD_TYPE;

typedef enum {
	BURST_LEN_64W = 0,
	BURST_LEN_48W = 1,
	BURST_LEN_32W = 2,
	BURST_LEN_16W = 3,

	ENUM_DUMMY4WORD(BURST_MODE_SEL)
} BURST_MODE_SEL;

/**
    State of State Machine
*/
//@{
typedef enum {
	TRKE_ENGINE_IDLE  = 0,        ///< Idel state
	TRKE_ENGINE_READY = 1,        ///< Ready state
	TRKE_ENGINE_PAUSE = 2,        ///< Pause state
	TRKE_ENGINE_RUN   = 3,        ///< Run state

	ENUM_DUMMY4WORD(TRKE_STATE_MACHINE)
} TRKE_STATE_MACHINE;
//@}



extern UINT32 g_uiTrkeFrmEndIntCnt;

/*----------- engine driver structure -------------*/
/*
    TRKE Open Object - Open Object is used for opening TRKE module.
*/
typedef struct _TRKE_OPENOBJ {
	VOID (*FP_TRKEISR_CB)(UINT32 uiIntStatus, VOID *trke_trig_hdl); ///< isr callback function
	UINT32 TRKE_CLOCKSEL;
} TRKE_OPENOBJ;

/**
    TRKE output setting

    Structure of settings of output function
*/
//@{
typedef struct _TRKE_CONTROL_INFO_STRUCT {

	UINT32 POINT_NUM;
	UINT32 MAX_SEARCH_RANGE;
	UINT32 ITER_CNT;
	BOOL IS_INITIAL_FLOW;
	BOOL IS_SKIP_POINT;
	UINT32 EPS;
	UINT32 MIN_EIG_VAL_THRES;
	UINT32 PYR_INDEX;
	UINT32 MAX_PYR_LEVEL;

} TRKE_CONTROL_INFO;
//@}

/**
    TRKE DMA in info

    Structure of DMA input parameters
*/
//@{
typedef struct _TRKE_DMA_IN_INFO_STRUCT {
	uintptr_t uiInSaddr0;        ///< Start address of input data
	uintptr_t uiInSaddr1;        ///< Start address of input data
	uintptr_t uiInSaddr2;        ///< Start address of input data
	uintptr_t uiInSaddr3;        ///< Start address of input data
	UINT32 uiInLofst0;        ///< Line offset of input data
	UINT32 uiInLofst1;        ///< Line offset of input data
} TRKE_DMA_IN_INFO;
//@}

typedef struct _TRKE_DMA_LL_INFO_STRUCT {
	uintptr_t uiLLAddr;        ///< Start address of input data
	uintptr_t uiLLBaseAddr;    ///< Start address of input data
} TRKE_DMA_LL_INFO;

/**
    TRKE DMA out info

    Structure of DMA output parameters
*/
//@{
typedef struct _TRKE_DMA_OUT_INFO_STRUCT {
	uintptr_t uiOutSaddr0;       ///< Start address of output data
	uintptr_t uiOutSaddr1;
} TRKE_DMA_OUT_INFO;
//@}

/**
    TRKE image size info

    Structure of input image size
*/
//@{
typedef struct _TRKE_PYR_SIZE_STRUCT {
	UINT32 uiWidth;              ///< input image width
	UINT32 uiHeight;             ///< input image height
} TRKE_PYR_SIZE;
//@}


/**
    TRKE point status info

    Structure of point status
*/
//@{
typedef struct _TRKE_STATUS_STRUCT {
	UINT8 STATUS[500];        ///< STATUS
} TRKE_STATUS;
//@}



/**
    TRKE engine parameter for library

    Structure of all avaliable engine parameter
*/
//@{
typedef struct _TRKE_PARAM_STRUCT {
	TRKE_CONTROL_INFO FuncInfo;             ///< Enable function of TRKE
	TRKE_STATUS PtStatus;
	TRKE_DMA_IN_INFO DmaIn;          ///< Info of DMA frame mode input
	TRKE_DMA_OUT_INFO DmaOut;        ///< Info of DMA frame mode output
	TRKE_PYR_SIZE InSize;            ///< Input image size
    TRKE_DMA_LL_INFO LLDmaIn;        ///< Info of DMA frame mode input
	BURST_MODE_SEL InBurstSel;       ///< Input burst length selection
	BURST_MODE_SEL OutBurstSel;      ///< Output burst length selection

	//TRKE_DBG_PORT_SEL DbgPortSel;    ///< Debug port selection


} TRKE_PARAM;
//@}


//external function prototype

//engine control
extern ER trke_stateMachine(TRKE_ACTION_OP RdeOp, BOOL bUpdate);
extern VOID trke_setSWReset(VOID);
extern ER trke_open(TRKE_OPENOBJ *pTrkeObjCB);//Available clock rate(MHz): 240, 480, PLL6, PLL13
extern ER trke_start(void);
extern ER trke_setMode(TRKE_PARAM *ptrkeParam);
extern ER trke_pause(void);
extern ER trke_close(void);
extern ER trke_waitFrameEnd(void);
extern ER trke_clearFrameEndFlag(void);
extern void trke_isr(void);
extern void trke_waitFrameEnd_LL(BOOL is_clr_flag);
extern void trke_clearFrameEndFlag_LL(void);
extern ER trke_ll_start(void);
extern ER trke_ll_setmode(TRKE_DMA_LL_INFO *pTrkeLLParam);
extern ER trke_ll_pause(void);
//get settings
extern UINT32 trke_getClockRate(VOID);
//extern uintptr_t trke_getDmaInAddr(VOID);
//extern uintptr_t trke_getDmaOutAddr(VOID);

extern void trke_create_resource(void *parm, UINT32 clk_freq);
extern void trke_release_resource(void *parm);
extern void trke_set_base_addr(uintptr_t addr);
extern uintptr_t trke_get_base_addr(VOID);

extern UINT32 trke_getIntStatus(VOID);
extern ER trke_enableInt(UINT32 uiIntrEn);
extern ER trke_clearIntStatus(UINT32 uiIntrStatus);


extern VOID trke_setTriHdl(VOID *trke_trig_hdl);
extern VOID * trke_getTriHdl(VOID);
extern UINT32 trke_getTriHdl_flagId(VOID);
extern UINT32 trke_get_clock_rate(VOID);

extern VOID trke_setInBurstLengthReg(UINT32 InBurstSel);
extern VOID trke_setOutBurstLengthReg(UINT32 OutBurstSel);

UINT32 trke_getStatusReg0(VOID);
UINT32 trke_getStatusReg1(VOID);
UINT32 trke_getStatusReg2(VOID);
UINT32 trke_getStatusReg3(VOID);
UINT32 trke_getStatusReg4(VOID);
UINT32 trke_getStatusReg5(VOID);
UINT32 trke_getStatusReg6(VOID);
UINT32 trke_getStatusReg7(VOID);
UINT32 trke_getStatusReg8(VOID);
UINT32 trke_getStatusReg8(VOID);
UINT32 trke_getStatusReg9(VOID);
UINT32 trke_getStatusReg10(VOID);
UINT32 trke_getStatusReg11(VOID);
UINT32 trke_getStatusReg12(VOID);
UINT32 trke_getStatusReg13(VOID);
UINT32 trke_getStatusReg14(VOID);
UINT32 trke_getStatusReg15(VOID);

extern INT32 trke_init(VOID);
extern INT32 trke_uninit(VOID);

ER kdrv_trke_init_trke_eng(VOID);
/*
 *#ifdef __cplusplus
 *}
 *#endif
 */

#endif

//@}
