/*
    Internal header file for SIE module.

    @file       sie_int.h
    @ingroup    mIIPPSIE

    Copyright   Novatek Microelectronics Corp. 2010.  All rights reserved.
*/

#ifndef _SIE_INT_H
#define _SIE_INT_H
#include "kwrap/type.h"
#include "vie_lib.h"

extern void vie_setReset(UINT32 id, BOOL bReset);
extern BOOL vie_getReset(UINT32 id);
extern void vie_setLoad(UINT32 id, UINT32 ch);
extern BOOL vie_getLoad(UINT32 id, UINT32 ch);
extern void vie_setChannelEnable(UINT32 id, UINT32 ch);
extern void vie_setChannelDisable(UINT32 id, UINT32 ch);
extern void vie_set_ChannelRst(UINT32 id, UINT32 ch);
extern void vie_release_ChannelRst(UINT32 id, UINT32 ch);
extern void vie_initParam(void);

extern void vie_setPadAsrc(UINT32 id, VIE_PADA_SRC src);
extern void vie_enableAxi(UINT32 id);
extern void vie_disableAxi(UINT32 id);

extern void vie_getSysDbgInfo(UINT32 id, UINT32 ch, VIE_SYS_DEBUG_INFO*pDbugInfo);
extern void vie_setChecksumEn(UINT32 id,UINT32 uiEn);
extern void vie_setdelaychain(UINT32 id, VIE_DELAYCHAIN_INFO delay_info);
extern void vie_setMainInput(UINT32 id, UINT32 ch, VIE_MAIN_INPUT_INFO *pMainInInfo);
extern void vie_getMainInput(UINT32 id, UINT32 ch, VIE_MAIN_INPUT_INFO *pMainInInfo);


extern ER vie_verifyFunction(UINT32 uiFunction);
extern void vie_setFunction(UINT32 id, UINT32 uiFunction);
extern void vie_enableFunction(UINT32 id, BOOL bEnable, UINT32 uiFunction);
extern UINT32 vie_getFunction(UINT32 id);
extern void vie_setDbgEn(UINT32 id, UINT32 uiFunction);

extern void vie_enableIntEnable(UINT32 id, BOOL bEnable, UINT32 uiIntrp);
extern void vie_setIntEnable(UINT32 id, UINT32 uiIntrp);
extern UINT32 vie_getIntEnable(UINT32 id);

extern void vie_clrEngineStatus(UINT32 id, UINT32 ch, VIE_ENGINE_STATUS_INFO *pEngineStatus);
extern void vie_clr656hitStatus(UINT32 id, UINT32 ch, VIE_ENGINE_STATUS_INFO *pEngineStatus);
extern void vie_clrlnfovflStatus(UINT32 id, UINT32 ch, VIE_ENGINE_STATUS_INFO *pEngineStatus);
extern void vie_getEngineStatus(UINT32 id, VIE_ENGINE_STATUS_INFO *pEngineStatus);
extern void vie_clrIntrStatus(UINT32 id, UINT32 uiIntrpStatus);
extern UINT32 vie_getIntrStatus(UINT32 id);


extern void vie_setSourceWindow(UINT32 id, UINT32 ch, VIE_SRC_WIN_INFO *pSrcWin);
extern void vie_getSourceWindow(UINT32 id, UINT32 ch, VIE_SRC_WIN_INFO *pSrcWin);
extern void vie_setCropWindow(UINT32 id, UINT32 ch, VIE_CRP_WIN_INFO *pCrpWin);
extern void vie_getCropWindow(UINT32 id, UINT32 ch, VIE_CRP_WIN_INFO *pCrpWin);

extern void vie_setBreakPoint(UINT32 id, UINT32 ch, VIE_BREAKPOINT_INFO *pBP);
extern void vie_setBreakPoint(UINT32 id, UINT32 ch, VIE_BREAKPOINT_INFO *pBP);

extern void vie_setDramOut0_Flip(UINT32 id, UINT32 ch, VIE_DRAM_OUT_INFO *pDOut0);
extern void vie_setDramOut0_Flip_dram2(UINT32 id, UINT32 ch, VIE_DRAM_OUT_INFO *pDOut0, UINT32 pa_low, UINT32 pa_high);

extern void vie_setDramOut1_Flip(UINT32 id, UINT32 ch, VIE_DRAM_OUT_INFO *pDOut1);
extern void vie_setDramOut1_Flip_dram2(UINT32 id, UINT32 ch, VIE_DRAM_OUT_INFO *pDOut1, UINT32 pa_low, UINT32 pa_high);

extern void vie_setDramDbgOut(UINT32 id, UINT32 ch, VIE_DRAM_OUT_INFO *pDbgOut);
extern void vie_setDramDbgOut_dram2(UINT32 id, UINT32 ch, VIE_DRAM_OUT_INFO *pDbgOut, UINT32 pa_low, UINT32 pa_high);

extern void vie_getDramOut0_Flip(UINT32 id, UINT32 ch, VIE_DRAM_OUT_INFO *pDOut0);
extern void vie_getDramOut1_Flip(UINT32 id, UINT32 ch, VIE_DRAM_OUT_INFO *pDOut1);


//extern UINT32 vie_getOutAdd_Flip(UINT32 id, UINT32 ch, UINT32 uiOutChIdx);
extern void vie_ChgOut0Addr_Flip(UINT32 id, UINT32 ch, VIE_BUF_ADDR_INFO *uiAddr);
extern void vie_ChgOut1Addr_Flip(UINT32 id, UINT32 ch, VIE_BUF_ADDR_INFO *uiAddr);
extern void vie_ChgDbgOutAddr(UINT32 id, UINT32 ch, VIE_BUF_ADDR_INFO *uiAddr);
extern void vie_ChgOut0Lofs(UINT32 id, UINT32 ch, UINT32 uiOutLofs);
extern void vie_ChgOut1Lofs(UINT32 id, UINT32 ch, UINT32 uiOutLofs);


extern void vie_setBurstLength(UINT32 id, UINT32 ch, VIE_BURST_LENGTH *pBurstLen);
extern void vie_getBurstLength(UINT32 id, UINT32 ch, VIE_BURST_LENGTH *pBurstLen);
extern void vie_setFlip(UINT32 id, UINT32 ch, VIE_FLIP_INFO *pFlipParm);
extern void vie_getFlip(UINT32 id, UINT32 ch, VIE_FLIP_INFO *pFlipParm);

extern void vie_setDramOutMode(UINT32 id, UINT32 ch, VIE_DRAM_OUT_CTRL OutMode);
extern void vie_setDramSingleOut(UINT32 id, UINT32 ch, VIE_DRAM_SINGLE_OUT *pDOut);
extern void vie_getDramSingleOut(UINT32 id, UINT32 ch, VIE_DRAM_SINGLE_OUT *pDOut);

extern void vie_setCCIR656Header(UINT32 id, UINT32 ch, UINT16 value);
extern void vie_getCCIR656HeaderStatus(UINT32 id, UINT32 ch, UINT32 *value);
extern void vie_setDVI(UINT32 id, UINT32 ch, VIE_DVI_INFO *pDviParam);
extern void vie_getDVI(UINT32 id, UINT32 ch, VIE_DVI_INFO *pDviParam);

extern void vie_setPatGen(UINT32 id, VIE_PATGEN_INFO *pPatGen);
extern void vie_getPatGen(UINT32 id, VIE_PATGEN_INFO *pPatGen);

extern void vie_setMask(UINT32 id, UINT32 ch, VIE_MASK_INFO *pMask);
extern void vie_getMask(UINT32 id, UINT32 ch, VIE_MASK_INFO *pMask);
//YCC APIs
extern void vie_setYccShiftEn(UINT32 id, BOOL *pShiftEn);
extern void vie_setYccParam(UINT32 id, VIE_YCC_PARAM_INFO *pYccParam);
extern VOID vie_setYccEn(UINT32 id, BOOL bYccEn);
extern VOID vie_setYccIniParm(VIE_YCC_PARAM_INFO *pYccParam);


#endif// _VIE_INT_H
