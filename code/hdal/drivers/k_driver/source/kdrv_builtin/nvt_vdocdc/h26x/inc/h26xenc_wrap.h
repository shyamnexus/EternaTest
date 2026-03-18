#ifndef _H26XENC_WRAP_H_
#define _H26XENC_WRAP_H_

#include "kwrap/type.h"

#include "h26x.h"

#include "h26xenc_int.h"

extern void h26xEnc_wrapUsrQpCfg(H26XRegSet *pRegSet, H26XUsrQp *pUsrQp, CODEC_TYPE eCodecType);
extern void h26xEnc_wrapPSNRCfg(H26XRegSet *pRegSet, BOOL bEnable);
extern void h26xEnc_wrapSliceSplitCfg(H26XRegSet *pRegSet, H26XEncSliceSplit *pSliceSplit);
extern void h26xEnc_wrapVarCfg(H26XRegSet *pRegSet, H26XEncVarCfg *pVarCfg);
extern void h26xEnc_wrapMaskInitCfg(H26XRegSet *pRegSet, H26XEncMask *pMask);
extern void h26xEnc_wrapMaskWinCfg(H26XRegSet *pRegSet, H26XEncMask *pMask, UINT8 ucIdx);
extern void h26xEnc_wrapGdrCfg(H26XRegSet *pRegSet, H26XEncGdr *pGdr, UINT16 usRowHeight, UINT32 uiPicCnt, BOOL bIsIfrm);
extern void h26xEnc_wrapRoiCfg(H26XRegSet *pRegSet, H26XEncRoi *pRoi, UINT8 ucIdx);
extern void h26xEnc_wrapRowRcCfg(H26XRegSet *pRegSet, H26XEncRowRc *pRowRc);
extern void h26xEnc_wrapRowRcPicUpdate(H26XRegSet *pRegSet, H26XEncRowRc *pRowRc, UINT32 uiPicCnt, BOOL bIsPFrm, UINT8  ucInitQp, UINT32 uiSVCId, H26XCOMN_CTX *pComnCtx);
extern void h26xEnc_wrapAqCfg(H26XRegSet *pRegSet, H26XEncAq *pAq, CODEC_TYPE codec_type);
extern void h26xEnc_wrapLpmCfg(H26XRegSet *pRegSet, H26XEncLpm *pLpm);
extern void h26xEnc_wrapRndCfg(H26XRegSet *pRegSet, h26XEncRnd *pRnd);
extern void h26xEnc_wrapScdCfg(H26XRegSet *pRegSet, H26XEncScd *pScd);
extern void h26xEnc_wrapOsgRgbCfg(H26XRegSet *pRegSet, H26XEncOsgRgbCfg *pOsgRgb);
extern void h26xEnc_wrapOsgPalCfg(H26XRegSet *pRegSet, UINT8 ucIdx, H26XEncOsgPalCfg *pOsgPal);
extern void h26xEnc_wrapOsgWinCfg(H26XRegSet *pRegSet, UINT8 ucIdx, H26XEncOsgWinCfg *pOsgWin, BOOL bOverlapType);
extern void h26xEnc_wrapOsgGlobal(H26XRegSet *pRegSet, H26XEncOsgGlobalCfg *pOsgGlobal);
extern void h26xEnc_wrapMotAqCfg(H26XRegSet *pRegSet, H26XEncMotAq *pMAq);
extern INT32 h26xEnc_wrapMotAddrCfg(H26XRegSet *pRegSet, H26XEncMotAddrCfg *pMotAddr, UINT32 md_lineoft);
extern void h26xEnc_wrapJndCfg(H26XRegSet *pRegSet, H26XEncJnd *pJnd);
extern void h26xEnc_wrapQpRelatedCfg(H26XRegSet *pRegSet, H26XEncQpRelatedCfg *pQpCfg);
extern void h26xEnc_wrapSdeCfg(H26XRegSet *pRegSet, H26XEncSdec *pSde);
// nt98528 //
extern void h26xEnc_wrapRmdCfg(H26XRegSet *pRegSet, H26XEncRmd *pIraMod, SLICE_TYPE pic_type);
extern void h26xEnc_wrapLambdaCfg(H26XRegSet *pRegSet, H26XEncLambda *pLambda, SLICE_TYPE pic_type);
extern void h26xEnc_wrapEarlySkipCfg(H26XRegSet *pRegSet, H26XEncEarlySkip *pEarlySkip);
extern void h26xEnc_wrapSpnCfg(H26XRegSet *pRegSet, H26XEncSpn *pSpn);
extern void h26xEnc_wrapMAQDiffCfg(H26XRegSet *pRegSet, H26XEncMAQDiffInfoCfg *pMAQDiffCfg);
extern void h26xEnc_wrapSha256Cfg(H26XRegSet *pRegSet, H26XEncSha256 *pSha);
extern void h26xEnc_wrapInterOnlyCfg(H26XRegSet *pRegSet, BOOL enable);
extern void h26xEnc_wrapResSupCfg(H26XRegSet *pRegSet, H26XEncResSup *pResSup);
extern void h26xEnc_wrapGMV(H26XRegSet *pRegSet, H26XEncGMV *pGMV);
extern void h26xEnc_wrapMotFilterCfg(H26XRegSet *pRegSet, H26XEncMotFilterCfg *pMotFilterCfg, BOOL bfirstFrm);
extern void h26xEnc_wrapMotDetectCfg(H26XRegSet *pRegSet, H26XEncMotDetectCfg *pMotDetectCfg, BOOL bfirstFrm);
extern void h26xEnc_wrapPartFrmSkipCfg(H26XRegSet *pRegSet, H26XEncPartFrmSkip *pPartFrmSkip);
extern void h26xEnc_wrapWaterMarkCfg(H26XRegSet *pRegSet, H26XEncWaterMark *pWaterMark);

extern void h26xEnc_wrapTurboCfg(H26XRegSet *pRegSet, H26XEncTurbo *pTurbo);
#endif	//_H26XENC_WRAP_H_

