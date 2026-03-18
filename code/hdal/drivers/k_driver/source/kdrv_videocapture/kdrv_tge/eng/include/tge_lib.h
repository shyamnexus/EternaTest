/*
    TGE module public header.

    @file       tge_lib.h
    @ingroup    mIDrvIPPTGE
    @note

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _TGE_LIB_H_
#define _TGE_LIB_H_

#include "kwrap/type.h"

/*************************************************************************************
 *  TGE Driver Version Definition
 *************************************************************************************/
#define TGE_ENG_DRV_VERSION         "0.01.01"

/*************************************************************************************
 *  TGE Interrupt & Event Definition
 *************************************************************************************/
#define TGE_INT_VD                  0x00000001
#define TGE_INT_VD2                 0x00000002
#define TGE_INT_VD3                 0x00000004
#define TGE_INT_VD4                 0x00000008
#define TGE_INT_VD_BP               0x00000100
#define TGE_INT_VD2_BP              0x00000200
#define TGE_INT_VD3_BP              0x00000400
#define TGE_INT_VD4_BP              0x00000800
#define TGE_INT_FLSHA_TRG           0x00010000
#define TGE_INT_MSHA_CLOSE_TRG      0x00020000
#define TGE_INT_MSHA_OPEN_TRG       0x00040000
#define TGE_INT_FLSHA_END           0x00200000
#define TGE_INT_MSHA_CLOSE_END      0x00400000
#define TGE_INT_MSHA_OPEN_END       0x00800000
#define TGE_INT_ALL                 0x00e70f0f

typedef enum  {
    TGE_INT_EVENT_VD             =  0x00000001,
    TGE_INT_EVENT_VD2            =  0x00000002,
    TGE_INT_EVENT_VD3            =  0x00000004,
    TGE_INT_EVENT_VD4            =  0x00000008,
    TGE_INT_EVENT_VD_BP          =  0x00000100,
    TGE_INT_EVENT_VD2_BP         =  0x00000200,
    TGE_INT_EVENT_VD3_BP         =  0x00000400,
    TGE_INT_EVENT_VD4_BP         =  0x00000800,
    TGE_INT_EVENT_FLSHA_TRG      =  0x00010000,
    TGE_INT_EVENT_MSHA_CLOSE_TRG =  0x00020000,
    TGE_INT_EVENT_MSHA_OPEN_TRG  =  0x00040000,
    TGE_INT_EVENT_FLSHA_END      =  0x00200000,
    TGE_INT_EVENT_MSHA_CLOSE_END =  0x00400000,
    TGE_INT_EVENT_MSHA_OPEN_END  =  0x00800000,
    TGE_INT_EVENT_ALL            =  0x00e70f0f
} TGE_INT_EVENT;

/*************************************************************************************
 *  TGE Parameter Definition
 *************************************************************************************/
typedef enum {
    MODE_MASTER = 0,
    MODE_SLAVE_TO_PAD,          ///< not supported
    MODE_SLAVE_TO_CSI,          ///< not supported
    MODE_SLAVE_TO_SLVSEC,       ///< not supported
    ENUM_DUMMY4WORD(TGE_MODE_SEL)
} TGE_MODE_SEL;

typedef enum {
    FLSH_FROM_SIE = 0,
    FLSH_FROM_SIE2,
    FLSH_FROM_SIE3,
    FLSH_FROM_SIE4,
    FLSH_FROM_SIE5,
    ENUM_DUMMY4WORD(TGE_FLSH_SIE_SRC_SEL)
} TGE_FLSH_SIE_SRC_SEL;

typedef enum {
    MSH_FROM_SIE = 0,
    MSH_FROM_SIE2,
    MSH_FROM_SIE3,
    MSH_FROM_SIE4,
    MSH_FROM_SIE5,
    ENUM_DUMMY4WORD(TGE_MSH_SIE_SRC_SEL)
} TGE_MSH_SIE_SRC_SEL;

typedef enum {
    TGE_PHASE_RISING  = 0,  ///< rising  edge latch/trigger
    TGE_PHASE_FALLING,      ///< falling edge latch/trigger
    ENUM_DUMMY4WORD(TGE_DATA_PHASE_SEL)
} TGE_DATA_PHASE_SEL;

typedef enum {
    WAVEFORM_PULSE = 0,
    WAVEFORM_LEVEL,
    ENUM_DUMMY4WORD(TGE_MSH_WAVEFORM_SEL)
} TGE_MSH_WAVEFORM_SEL;

typedef enum {
    TRG_IMD = 0,
    TRG_WAIT_VD,
    TRG_WAIT_EXT_TRG,
    TRG_CONT_WAT_TRG,
    TRG_OFF,
    ENUM_DUMMY4WORD(TGE_CTRL_PIN_TRG_SEL)
} TGE_CTRL_PIN_TRG_SEL;

typedef enum {
    TGE_CHGRST_VD_RST       = 0x00000001,
    TGE_CHGRST_VD2_RST      = 0x00000002,
    TGE_CHGRST_VD3_RST      = 0x00000004,
    TGE_CHGRST_VD4_RST      = 0x00000008,
} TGE_CHANGE_RST_SEL;

typedef enum {
    TGE_CHG_VDINFO          = 0x00000001,
    TGE_CHG_VD2INFO         = 0x00000002,
    TGE_CHG_VD3INFO         = 0x00000004,
    TGE_CHG_VD4INFO         = 0x00000008,
} TGE_CHANGE_VDHD_SEL;

typedef enum {
    TGE_CHG_VD_RST =  0,           ///< VD 1 reset                    , please refer to "TGE_VD_RST_SEL"
    TGE_CHG_VD2_RST,               ///< VD 2 reset                    , please refer to "TGE_VD_RST_SEL"
    TGE_CHG_VD3_RST,               ///< VD 3 reset                    , please refer to "TGE_VD_RST_SEL"
    TGE_CHG_VD4_RST,               ///< VD 4 reset                    , please refer to "TGE_VD_RST_SEL"

    TGE_CHG_VD_PHASE,              ///< VD 1 Phase                    , please refer to "TGE_VD_PHASE"
    TGE_CHG_VD2_PHASE,             ///< VD 2 Phase                    , please refer to "TGE_VD_PHASE"
    TGE_CHG_VD3_PHASE,             ///< VD 3 Phase                    , please refer to "TGE_VD_PHASE"
    TGE_CHG_VD4_PHASE,             ///< VD 4 Phase                    , please refer to "TGE_VD_PHASE"

    TGE_CHG_VD_INV,                ///< VD 1 Inv                      , please refer to "TGE_VD_INV"
    TGE_CHG_VD2_INV,               ///< VD 2 Inv                      , please refer to "TGE_VD_INV"
    TGE_CHG_VD3_INV,               ///< VD 3 Inv                      , please refer to "TGE_VD_INV"
    TGE_CHG_VD4_INV,               ///< VD 4 Inv                      , please refer to "TGE_VD_INV"

    TGE_CHG_VD_MODE,               ///< VD 1 Model Sel                , please refer to "TGE_MODE_SEL"
    TGE_CHG_VD2_MODE,              ///< VD 2 Model Sel                , please refer to "TGE_MODE_SEL"
    TGE_CHG_VD3_MODE,              ///< VD 3 Model Sel                , please refer to "TGE_MODE_SEL"
    TGE_CHG_VD4_MODE,              ///< VD 4 Model Sel                , please refer to "TGE_MODE_SEL"

    TGE_CHG_FLSHAVDSEL,            ///< FlashA VD/HD source           , please refer to "TGE_FLSH_SIE_SRC_SEL"
    TGE_CHG_FLSHACLKSEL,           ///< FlashA clock source           , please refer to "TGE_FLASH_CLKSRC_SEL"

    TGE_CHG_MSHAVDSEL,             ///< MshA VD/HD source             , please refer to "TGE_MSH_SIE_SRC_SEL"
    TGE_CHG_MSHACLKSEL,            ///< MshA clock source             , please refer to "TGE_MSH_CLKSRC_SEL"

    TGE_CHG_VDHD,                  ///< change VD/HD                  , please refer to "TGE_VDHD_INFO"
    TGE_CHG_VD2HD2,                ///< change VD2/HD2                , please refer to "TGE_VDHD_INFO"
    TGE_CHG_VD3HD3,                ///< change VD3/HD3                , please refer to "TGE_VDHD_INFO"
    TGE_CHG_VD4HD4,                ///< change VD4/HD4                , please refer to "TGE_VDHD_INFO"

    TGE_CHG_TIMING_VD_PAUSE,       ///< pause timing(VD/HD)           , please refer to "TGE_TIMING_PAUSE_INFO"
    TGE_CHG_TIMING_VD2_PAUSE,      ///< pause timing(VD2/HD2)         , please refer to "TGE_TIMING_PAUSE_INFO"
    TGE_CHG_TIMING_VD3_PAUSE,      ///< pause timing(VD3/HD3)         , please refer to "TGE_TIMING_PAUSE_INFO"
    TGE_CHG_TIMING_VD4_PAUSE,      ///< pause timing(VD4/HD4)         , please refer to "TGE_TIMING_PAUSE_INFO"

    TGE_CHG_VD_BP,                 ///< CHANGE VD BP                  , please refer to "TGE_BREAKPOINT_INFO"
    TGE_CHG_VD2_BP,                ///< CHANGE VD2 BP                 , please refer to "TGE_BREAKPOINT_INFO"
    TGE_CHG_VD3_BP,                ///< CHANGE VD3 BP                 , please refer to "TGE_BREAKPOINT_INFO"
    TGE_CHG_VD4_BP,                ///< CHANGE VD4 BP                 , please refer to "TGE_BREAKPOINT_INFO"

    TGE_CHG_FLSHA_CTRL_INV,        ///< FlashA inv
    TGE_CHG_FLSHA_EXT_INV,         ///< FlashA external-trig inv

    TGE_CHG_MSHA_CTRL_INV,         ///< MshA   inv
    TGE_CHG_MSHA_EXT_INV,          ///< MshA   external-trig inv
    TGE_CHG_MSHA_SWAP,             ///< MshA   two wires swap
    TGE_CHG_MSHA_CTRL_MODE,        ///< MshA   control mode           , please refer to "TGE_MSH_WAVEFORM_SEL"

    TGE_CHG_FLSHA_TRIG,            ///< trigger FlashA                , please refer to "TGE_CTRL_PIN_TRG_SEL"

    TGE_CHG_MSHA_CLOSE_TRIG,       ///< trigger MshA close pin        , please refer to "TGE_CTRL_PIN_TRG_SEL"
    TGE_CHG_MSHA_OPEN_TRIG,        ///< trigger MshA open  pin        , please refer to "TGE_CTRL_PIN_TRG_SEL"

    TGE_CHG_FLSHA_CTRL,            ///< change  FlashA setting        , please refer to "TGE_CTRL_PIN_INFO"

    TGE_CHG_MSHA_CLOSE_CTRL,       ///< change  MshA close pin setting, please refer to "TGE_CTRL_PIN_INFO"
    TGE_CHG_MSHA_OPEN_CTRL,        ///< change  MshA open  pin setting, please refer to "TGE_CTRL_PIN_INFO"

    TGE_CHG_IDX_NUM,

    ENUM_DUMMY4WORD(TGE_CHANGE_FUN_PARAM_SEL)
} TGE_CHANGE_FUN_PARAM_SEL;

typedef enum {
    FLASHCLKSRC_MCLK = 0,           ///< TGE flash clock source
    FLASHCLKSRC_MCLK2,
    FLASHCLKSRC_MCLK3,
    FLASHCLKSRC_MCLK4,
    ENUM_DUMMY4WORD(TGE_FLASH_CLKSRC_SEL)
} TGE_FLASH_CLKSRC_SEL;

typedef enum {
    MSHCLKSRC_MCLK = 0,             ///< TGE Mechanical Shutter clock source
    MSHCLKSRC_MCLK2,
    MSHCLKSRC_MCLK3,
    MSHCLKSRC_MCLK4,
    ENUM_DUMMY4WORD(TGE_MSH_CLKSRC_SEL)
} TGE_MSH_CLKSRC_SEL;

typedef enum {
    TGE_VD_DISABLE = 0,
    TGE_VD_ENABLE,
    ENUM_DUMMY4WORD(TGE_VD_RST_SEL)
} TGE_VD_RST_SEL;

typedef struct {
    TGE_MODE_SEL        ModeSel;
    TGE_MODE_SEL        Mode2Sel;
    TGE_MODE_SEL        Mode3Sel;
    TGE_MODE_SEL        Mode4Sel;
} TGE_MODE_SEL_INFO;

typedef struct {
    TGE_DATA_PHASE_SEL  VdPhase;
    TGE_DATA_PHASE_SEL  HdPhase;
} TGE_VD_PHASE;

typedef struct {
    BOOL                bVdInv;
    BOOL                bHdInv;
} TGE_VD_INV;

typedef struct {
    TGE_VD_PHASE        VdPhaseData;
    TGE_VD_PHASE        Vd2PhaseData;
    TGE_VD_PHASE        Vd3PhaseData;
    TGE_VD_PHASE        Vd4PhaseData;
} TGE_VD_PHASE_INFO;

typedef struct {
    TGE_VD_INV          VdInvData;
    TGE_VD_INV          Vd2InvData;
    TGE_VD_INV          Vd3InvData;
    TGE_VD_INV          Vd4InvData;
} TGE_VD_INV_INFO;

typedef struct {
    TGE_VD_PHASE_INFO    VdPhaseData;
    TGE_VD_INV_INFO      VdInvData;

    TGE_FLSH_SIE_SRC_SEL FlshAVdSrc;

    TGE_MSH_SIE_SRC_SEL  MshAVdSrc;

    BOOL                 bFlshAExtTrgInv;
    BOOL                 bFlshACtrlInv;

    BOOL                 bMshAExtTrgInv;
    BOOL                 bMshACtrlInv;
    BOOL                 bMshASwap;
    TGE_MSH_WAVEFORM_SEL MshAWvFrm;
} TGE_BASIC_SET_INFO;

typedef struct {
    TGE_VD_RST_SEL       VdRst;
    TGE_VD_RST_SEL       Vd2Rst;
    TGE_VD_RST_SEL       Vd3Rst;
    TGE_VD_RST_SEL       Vd4Rst;
} TGE_VD_RST_INFO;

typedef struct {
    UINT32         uiVdPeriod;
    UINT32         uiVdAssert;
    UINT32         uiVdFrontBlnk;
    UINT32         uiHdPeriod;
    UINT32         uiHdAssert;
    UINT32         uiHdCnt;
} TGE_VDHD_INFO;

typedef struct {
    TGE_VDHD_INFO  VdInfo;
    TGE_VDHD_INFO  Vd2Info;
    TGE_VDHD_INFO  Vd3Info;
    TGE_VDHD_INFO  Vd4Info;
} TGE_VDHD_SET;

typedef struct {
    UINT32      uiVdBp;         ///< VD  break point
    UINT32      uiVd2Bp;        ///< VD2 break point
    UINT32      uiVd3Bp;        ///< VD3 break point
    UINT32      uiVd4Bp;        ///< VD4 break point
} TGE_BREAKPOINT_INFO;

typedef struct {
    UINT8       uiClkCnt;

    BOOL        bVD;
    BOOL        bHD;
    BOOL        bVD2;
    BOOL        bHD2;
    BOOL        bVD3;
    BOOL        bHD3;
    BOOL        bVD4;
    BOOL        bHD4;

    BOOL        bFlshAExtTrg;
    BOOL        bFlshAOut0Ctrl;

    BOOL        bMshAExtTrg;
    BOOL        bMshAOut0Ctrl;
    BOOL        bMshAOut1Ctrl;
} TGE_ENGINE_STATUS_INFO;

typedef struct {
    UINT32      uiDelay;
    UINT32      uiAssert;
    UINT32      uiPeriod;
} TGE_CTRL_PIN_INFO;

typedef struct {
    BOOL        bVdPause;
    BOOL        bHdPause;
} TGE_TIMING_PAUSE_INFO;

typedef struct {
    UINT32              uiIntrpEn;
    TGE_BASIC_SET_INFO  BasicSetInfo;
    TGE_MODE_SEL_INFO   ModeSelInfo;            ///< not supported
    TGE_VD_RST_INFO     VdRstInfo;
    TGE_VDHD_SET        VdHdInfo;
    TGE_BREAKPOINT_INFO BreakPointInfo;
} TGE_MODE_PARAM;

typedef struct {
    TGE_FLASH_CLKSRC_SEL TgeFlashAClkSel;       ///< flash A clock source selection
    TGE_MSH_CLKSRC_SEL   TgeMshAClkSel;         ///< mechanical shutter A clock source selection

    void (*pfTgeIsrCb)(UINT32 uiIntpStatus, UINT32 uiFlshStatus);    ///< isr callback function
} TGE_OPENOBJ;

typedef struct {
    ULONG   reg_io_base;    ///< TGE I/O base address, linux from ioremap, rtos from io_base
    int     irq_id;         ///< TGE interrupt number

    void    *tge_clk;       ///< VD/HD  clock handle, used for linux, for VD/HD/FLASH/MSH clock gating
    void    *flash_a_clk;   ///< FlashA clock handle, used for linux, for FLASH clock source selection
    void    *msh_a_clk;     ///< MshA   clock handle, used for linux, for MSH   clock source selection
} TGE_RESOURCE;

typedef void (*TGE_DRV_ISR_FP)(UINT32 int_sts, UINT32 flsh_sts);

/*************************************************************************************
 *  TGE Public Function Prototype
 *************************************************************************************/
extern ER    tge_create_resource(TGE_RESOURCE *pRes);
extern void  tge_release_resource(void);
extern ER    tge_open(TGE_OPENOBJ *pObjCB);
extern ER    tge_close(void);
extern ER    tge_pause(void);
extern ER    tge_start(void);
extern ER    tge_setMode(TGE_MODE_PARAM *pTgeParam);
extern ER    tge_chgParam(void *pParam, TGE_CHANGE_FUN_PARAM_SEL FunSel);
extern ER    tge_chgRst(TGE_VD_RST_INFO *pParam, TGE_CHANGE_RST_SEL FunSel);    ///< for mutiple vd/hd reset  control
extern ER    tge_chgVdHd(TGE_VDHD_SET *pParam, TGE_CHANGE_VDHD_SEL FunSel);     ///< for mutiple vd/hd timing control
extern ER    tge_waitEvent(TGE_INT_EVENT WaitEvent, BOOL bClrFlag);             ///< default timeout 3s
extern ER    tge_waitEvent_timeout(TGE_INT_EVENT WaitEvent, BOOL bClrFlag, int timeout_ms);
extern INT32 tge_chk_limitation(UINT32 flag);
extern void  tge_getVdRstInfo(TGE_VD_RST_INFO *pParam);
extern void  tge_reg_isr_cb(TGE_DRV_ISR_FP fp);
extern void  tge_isr(void);
extern void  tge_set_dbg_level(int level);

#endif  /* _TGE_LIB_H_ */
