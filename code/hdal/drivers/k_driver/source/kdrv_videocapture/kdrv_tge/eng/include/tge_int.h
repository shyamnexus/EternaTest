/*
    TGE module internal header.

    @file       tge_int.h
    @ingroup    mIIPPTGE

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _TGE_INT_H_
#define _TGE_INT_H_

#include "kwrap/type.h"
#include "tge_lib.h"

/*************************************************************************************
 *  TGE Internal Function Prototype
 *************************************************************************************/
extern void tge_setReset(BOOL bReset);
extern void tge_setLoad(void);
extern void tge_setBasicSet(TGE_BASIC_SET_INFO *pBasic);

extern void tge_setFlshAsrc(TGE_FLSH_SIE_SRC_SEL SrcSel);
extern void tge_setFlshACtrlInv(BOOL bCtrlInv);
extern void tge_setFlshAExtInv(BOOL bCtrlInv);
extern void tge_setFlshATrig(TGE_CTRL_PIN_TRG_SEL TrigSel);
extern void tge_setFlshACtrl(TGE_CTRL_PIN_INFO *pCtrlPin);

extern void tge_setVdTimingPause(TGE_TIMING_PAUSE_INFO  *pTimingPauseInfo);
extern void tge_setVd2TimingPause(TGE_TIMING_PAUSE_INFO *pTimingPauseInfo);
extern void tge_setVd3TimingPause(TGE_TIMING_PAUSE_INFO *pTimingPauseInfo);
extern void tge_setVd4TimingPause(TGE_TIMING_PAUSE_INFO *pTimingPauseInfo);

extern void   tge_enableIntEnable(BOOL bEnable, UINT32 uiIntrp);
extern void   tge_setIntEnable(UINT32 uiIntrp);
extern UINT32 tge_getIntEnable(void);
extern UINT32 tge_getIntrStatus(void);

extern void tge_clrIntrStatus(UINT32 uiIntrpStatus);
extern void tge_setAllVdRst(TGE_VD_RST_INFO *pVdRst);
extern void tge_setRst(TGE_VD_RST_INFO *pParam, TGE_CHANGE_RST_SEL FunSel);
extern void tge_setVdRst(TGE_VD_RST_SEL VdRst);
extern void tge_setVd2Rst(TGE_VD_RST_SEL VdRst);
extern void tge_setVd3Rst(TGE_VD_RST_SEL VdRst);
extern void tge_setVd4Rst(TGE_VD_RST_SEL VdRst);

extern void tge_setVdHdInfo(TGE_VD_RST_INFO *pVdRst, TGE_VDHD_SET *pVdHdInfo);
extern void tge_setVdHd(TGE_VDHD_INFO *pVdHd);
extern void tge_setVd2Hd2(TGE_VDHD_INFO *pVdHd);
extern void tge_setVd3Hd3(TGE_VDHD_INFO *pVdHd);
extern void tge_setVd4Hd4(TGE_VDHD_INFO *pVdHd);

extern void tge_setBreadPoint(TGE_VD_RST_INFO *pVdRst, TGE_BREAKPOINT_INFO *pBP);
extern void tge_setVdBreadPoint(UINT32  pBP);
extern void tge_setVd2BreadPoint(UINT32 pBP);
extern void tge_setVd3BreadPoint(UINT32 pBP);
extern void tge_setVd4BreadPoint(UINT32 pBP);

extern void tge_setVdInv(TGE_VD_INV  *pInvSel);
extern void tge_setVd2Inv(TGE_VD_INV *pInvSel);
extern void tge_setVd3Inv(TGE_VD_INV *pInvSel);
extern void tge_setVd4Inv(TGE_VD_INV *pInvSel);

extern void tge_setVdPhase(TGE_VD_PHASE  *pPhaseSel);
extern void tge_setVd2Phase(TGE_VD_PHASE *pPhaseSel);
extern void tge_setVd3Phase(TGE_VD_PHASE *pPhaseSel);
extern void tge_setVd4Phase(TGE_VD_PHASE *pPhaseSel);

extern void tge_setMshAExtInv(BOOL bCtrlInv);
extern void tge_setMshAsrc(TGE_MSH_SIE_SRC_SEL SrcSel);
extern void tge_setMshACtrlInv(BOOL bCtrlInv);
extern void tge_setMshASwap(BOOL bSwap);
extern void tge_setMshACtrlMode(TGE_MSH_WAVEFORM_SEL bCtrlMode);

extern void tge_setMshACloseTrig(TGE_CTRL_PIN_TRG_SEL TrigSel);
extern void tge_setMshACloseCtrl(TGE_CTRL_PIN_INFO *pCtrlPin);
extern void tge_setMshAOpenTrig(TGE_CTRL_PIN_TRG_SEL TrigSel);
extern void tge_setMshAOpenCtrl(TGE_CTRL_PIN_INFO *pCtrlPin);

extern void tge_getEngineStatus(TGE_ENGINE_STATUS_INFO *pEngineStatus);

#endif  /* _TGE_INT_H_ */
