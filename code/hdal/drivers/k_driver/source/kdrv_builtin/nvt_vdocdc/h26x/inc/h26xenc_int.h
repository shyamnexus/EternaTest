#ifndef _H26XENC_INT_H_
#define _H26XENC_INT_H_

#include "kwrap/type.h"

#include "h26x_def.h"

#include "h26x.h"

#include "h26xenc_api.h"

typedef struct _H26XUsrQp_{
	H26XEncUsrQpCfg stCfg;
}H26XUsrQp;

typedef struct _H26XEncSliceSplit_{
	H26XEncSliceSplitCfg stCfg;
	H26XEncSliceSplitCfg stExe;

	UINT32 uiNaluNum;
}H26XEncSliceSplit;

typedef struct _H26XEncMaskLine_{
	INT32 iCoeffa;
	INT32 iCoeffb;
	INT32 iCoeffc;

	UINT16 usX0;
	UINT16 usY0;
	UINT16 usX1;
	UINT16 usY1;

	UINT8  ucTh;
	UINT32 uiLineChkAdd;
}H26XEncMaskLine;

typedef struct _H26XEncMaskConcave_{
	UINT8  ucConcave;
	UINT8  ucMinx;
	UINT8  ucMiny;
	UINT8  ucMaxx;
	UINT8  ucMaxy;
}H26XEncMaskConcave;

typedef struct _H26XEncMask_{
	BOOL bEnable;			// for total mask enable/disable //

	H26XEncMaskInitCfg stInitCfg;
	H26XEncMaskWinCfg  stWinCfg[16];
	H26XEncMaskLine    stLine[16][4];
	H26XEncMaskConcave stConcave[16];
}H26XEncMask;

typedef struct _H26XEncGdr_{
	H26XEncGdrCfg stCfg;

	UINT16 usStart;
	UINT32 uiStartPicCnt;
	BOOL   bGdrQpEn;
    UINT8  ucGdrQp;

	BOOL   bClearISliceIdx;
	BOOL   bClearSliceSplit;
}H26XEncGdr;

typedef struct _H26XEncRoi_{
	UINT8 ucNumber;

	H26XEncRoiCfg stCfg[H26X_MAX_ROI_W];
}H26XEncRoi;

typedef struct _H26XEncRowRc_{
	H26XEncRowRcCfg stCfg;

	UINT8  ucMinCostTh;	// need init while enable rrc //
	UINT8  ucZeroBitMode;
	UINT16 usIdxStart;

	UINT8  ucInitQp;
	UINT32 uiPlannedStop;
	UINT32 uiPlannedTop;
	UINT32 uiPlannedBot;
	UINT16 usRefFrmCoeff[H26X_MAX_TILE_NUM][MAX_RRC_FRAME_LEVEL]; // 0 : I. 1 : P
	UINT32 uiFrmCostLsb[MAX_RRC_FRAME_LEVEL]; // 0 : I. 1 : P
	UINT32 uiFrmCostMsb[MAX_RRC_FRAME_LEVEL]; // 0 : I. 1 : P
	UINT32 uiFrmCmpxLsb[MAX_RRC_FRAME_LEVEL]; // 0 : I. 1 : P
	UINT32 uiFrmCmpxMsb[MAX_RRC_FRAME_LEVEL]; // 0 : I. 1 : P
	INT32  iPrevPicBitsBias[MAX_RRC_FRAME_LEVEL];// 0 : I. 1 : P
	UINT32 uiPredBits;

	BOOL  bRRCMode; //0:original, 1: new
	UINT8 ucTileQPStep[2];
	BOOL  bTileQPRst;
	UINT8 ucZeroBitMod;

	INT32 iTH_0;
	INT32 iTH_1;
	INT32 iTH_2;
	INT32 iTH_M;
	INT32 iTH_3;
	INT32 iTH_4;
	INT32 iBeta;
	INT32 m_ModU_Frm[4];  // 0 : I. 1 : P
	INT32 m_ModD_Frm[4];  // 0 : I. 1 : P
	INT64 i_all_ref_pred_tmpl;
	INT32 iLastFrmAvgQPDiff;
	INT32 iTargetBitsScale;
	UINT8 ucNDQPStep;
	UINT8 ucNDQPRange;
	UINT8 ucRRCMode; //0: rrc 1.0, 1: rrc 2.0

	//#if RRC_BY_FRAME_LEVEL
	UINT8 ucFrameLevel;
	//#endif
}H26XEncRowRc;

typedef struct _H26XEncAq_{
	H26XEncAqCfg stCfg;
}H26XEncAq;

typedef struct _H26XEncLpm_{
	H26XEncLpmCfg stCfg;
    H26XEncLpmSetCfg stSetCfg;
}H26XEncLpm;

typedef struct _H26XEncUserData_{
	H26XEncUserDataCfg stCfg;
}H26XEncUserData;

typedef struct _H26XEncRnd_{
	H26XEncRndCfg stCfg;
}h26XEncRnd;

typedef struct _H26XEncScd_{
	H26XEncScdCfg stCfg;
}H26XEncScd;

typedef struct _H26XEncVar_{
	H26XEncVarCfg stCfg;
}H26XEncVar;

typedef struct _H26XEncOsg_{
	H26XEncOsgGlobalCfg	stGlobal;
	H26XEncOsgRgbCfg stRgb;
	H26XEncOsgWinCfg stWin[32];
	H26XEncOsgPalCfg stPal[16];
}H26XEncOsg;

typedef struct _H26XEncMotAq_{
	H26XEncMotAqCfg		stCfg;
	H26XEncMotAddrCfg	stAddrCfg;
	H26XEncMdInfoCfg    stMdInfoCfg;
	H26XEncMAQDiffInfo  stMAQDiffInfo;
	H26XEncMAQDiffInfoCfg stMAQDiffInfoCfg;
	H26XEncMAQBGBiasCfg stBGSBias;
}H26XEncMotAq;

typedef struct _H26XEncJnd_{
	H26XEncJndCfg stCfg;
}H26XEncJnd;

typedef struct _H26XEncSdec_{
	H26XEncSdeCfg stCfg;
}H26XEncSdec;

typedef struct _H26XEncRmd_{
	H26XEncRmdCfg stCfg;
}H26XEncRmd;

typedef struct _H26XEncLambda_{
	H26XEncLambdaCfg stCfg;
}H26XEncLambda;

typedef struct _H26XEncEarlySkip_{
	H26XEncEarlySkipCfg stCfg;
} H26XEncEarlySkip;

typedef struct _H26XEncSpn_{
	H26XEncSpnCfg stCfg;
} H26XEncSpn;

typedef struct _H26XEncSha256_{
	unsigned int state[8];
} H26XEncSha256;

typedef struct _H26XEncResSup_{
    H26XEncResSupCfg stCfg;
} H26XEncResSup;

typedef struct _H26XEncMotFilter_{
	H26XEncMotFilterCfg stCfg;
} H26XEncMotFilter;

typedef struct _H26XEncMotDetect_ {
	H26XEncMotDetectCfg		stCfg;
	H26XEncMotDetAddrCfg	stAddrCfg;

	UINT32 uiMDState[8];
} H26XEncMotDetect;

typedef struct _H26XEncPartFrmSkip_{
	H26XEncPartFrmSkipCfg stCfg;
} H26XEncPartFrmSkip;

typedef struct _H26XEncWaterMark_{
	H26XEncWaterMarkCfg stCfg;
} H26XEncWaterMark;

typedef struct _H26xEncGMV_{
	UINT8 gmv_ctrl;
	UINT8 gmv_en[16];
	UINT8 gmv_x[16];
	UINT8 gmv_y[16];
} H26XEncGMV;

typedef struct _H26XEncTurbo_ {
	H26XEncTurboCfg stCfg;

	UINT8 ucTxTh;
} H26XEncTurbo;

#if H26X_SUPPORT_SCENE_DETECT
typedef struct _H26XEncSceneDetect_{
	H26XEncSceneDetCfg stCfg;
	H26XEncSceneDetParam stParam;
	H26XEncFgAddrCfg   stAddrCfg;
	H26XEncSDParamBak  stParamBak;
}H26XEncSceneDetect;
#endif

#if H26X_SUPPORT_SMART_ROI
typedef struct _H26XEncSmartRoi_{
	H26XEncSmartRoiCfg stCfg;
} H26XEncSmartRoi;

typedef struct _H26XEncSmartBBox_{
	H26XEncSmartBBoxCfg stCfg;
} H26XEncSmartBBox;
#endif

#if H26X_SUPPORT_DYNAMIC_FR
typedef struct _H26XEncDynFR_{
	H26XEncDynFRCfg stCfg;
	H26XEncDynFRParam stParam;
} H26XEncDynFR;
#endif

#if H26X_SUPPORT_DYNAMIC_GOP
typedef struct _H26XEncDynGop_{
	H26XEncDynGopCfg stCfg;
	H26XEncDynGopParam stParam;
} H26XEncDynGop;
#endif

typedef struct _H26XFUNC_CTX_{
	H26XUsrQp			stUsrQp;
	H26XEncSliceSplit	stSliceSplit;
	H26XEncMask			stMask;
	H26XEncGdr			stGdr;
	H26XEncRoi			stRoi;
	H26XEncRowRc		stRowRc;
	H26XEncAq			stAq;
	H26XEncLpm			stLpm;
	h26XEncRnd			stRnd;
	H26XEncScd			stScd;
	H26XEncOsg			stOsg;
	H26XEncMotAq		stMAq;
	H26XEncJnd			stJnd;
	H26XEncQpRelatedCfg	stQR;
	H26XEncVarCfg		stVar;
	H26XEncSdec			stSdec;
	H26XEncUserData     stUserData;

    H26XEncRmd          stRmd;
    H26XEncLambda       stLambda;
	H26XEncEarlySkip	stEarlySkip;
	H26XEncSpn			stSpn;

	// nt98538 //
	H26XEncResSup		stResSup;
	H26XEncMotFilter    stMotFilter;
	H26XEncMotDetect    stMotDetect;
	H26XEncPartFrmSkip  stPartFrmSkip;
	H26XEncGMV			stGMV;
	H26XEncWaterMark    stWTMK;

	// nt98539A //
	H26XEncTurbo        stTurbo;

	BOOL bPSNREn;
	BOOL inter_only;
#if H26X_SUPPORT_SCENE_DETECT
	H26XEncSceneDetect	stSceneDet;
#endif
#if H26X_SUPPORT_SMART_ROI
	H26XEncSmartRoi     stSmartRoi;
	H26XEncSmartBBox    stSmartBBox;
#endif
#if H26X_SUPPORT_DYNAMIC_FR
	H26XEncDynFR        stDynFR;
#endif
#if H26X_SUPPORT_DYNAMIC_GOP
	H26XEncDynGop       stDynGop;
#endif
} H26XFUNC_CTX;

extern void h26XEnc_setGdrQp(H26XENC_VAR *pVar, BOOL bGdrQpEn, UINT8 ucGdrQp);
#if H264_AEFIFO_RRC_BIAS
extern void h26XEnc_getRowRcState(SLICE_TYPE ePicType, H26XRegSet *pRegSet, H26XEncRowRc *pRowRc, UINT32 uiSVCId, int uiBSLen);
#else
extern void h26XEnc_getRowRcState(SLICE_TYPE ePicType, H26XRegSet *pRegSet, H26XEncRowRc *pRowRc, UINT32 uiSVCId);
#endif
extern void h26XEnc_setInitSde(H26XRegSet *pRegSet);
extern UINT8 h26XEnc_getRcQuant(H26XENC_VAR *pVar, SLICE_TYPE ePicType, BOOL bIsLTRFrm, UINT32 uiBsOutBufSize, BOOL bSkipFrmEn, UINT8 ucLastQP);
extern void h26XEnc_UpdateRc(H26XENC_VAR *pVar, SLICE_TYPE ePicType, H26XEncResultCfg *pResult);
extern void h26XEnc_setGMV(H26XENC_VAR *pVar, H26XEncGMV *gmv);
extern void h26XEnc_updateEmbeddedMd(H26XENC_VAR *pVar, UINT8 uiLastQP, UINT8 uiCurQP);

#ifdef VDOCDC_EMU
extern void h26XEnc_getRowRcState_EMU_265(SLICE_TYPE ePicType, H26XRegSet * pRegSet, H26XEncRowRc * pRowRc, UINT32 uiSVCId);
#endif
#endif	//_H26XENC_INT_H_

