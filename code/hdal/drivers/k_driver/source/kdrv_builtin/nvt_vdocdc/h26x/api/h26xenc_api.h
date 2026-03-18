#ifndef _H26XENC_API_H_
#define _H26XENC_API_H_

#include "kwrap/type.h"
#include "h26x_def.h"
#include "h26xenc_rc.h"
#include "h26x_common.h"

#include "kdrv_videoenc/kdrv_videoenc_lmt.h"

#define H26X_AQ_TH_TBL_SIZE	(30)

typedef struct _H26XEncFroCfg_ {
	BOOL bEnable;
} H26XEncFroCfg;

typedef struct _H26XEncUsrQpCfg_ {
	BOOL bEnable;

	uintptr_t uiQpMapAddr;
	UINT32 uiQpMapSize;
	UINT32 uiQpMapLineOffset;
	UINT32 uiQpMapRotate;
	BOOL bPiority;
	UINT8  ucMosaicCtrl;
} H26XEncUsrQpCfg;

typedef struct _H26XEncSliceSplitCfg_ {
	BOOL   bEnable;        // enable multiple slice , range : 0 ~ 1 //
	UINT8  uiSliceMode;
	UINT16  uiSliceNum;
	UINT32  uiSliceRowNum[4]; // number of macroblock rows occupied by a slice , range : 1 ~ SIZE16X(picture_height)
	INT32  iISliceIdx;    // iISliceIdx ==-1 :disable!@i-slice
} H26XEncSliceSplitCfg;

typedef struct _H26XEncVarCfg_ {
	UINT16 usThreshold;
	UINT8  ucAVGMin;
	UINT8  ucAVGMax;
	UINT8  ucDelta;
	UINT8  ucIRangeDelta;
	UINT8  ucPRangeDelta;
	UINT8  ucMerage;
} H26XEncVarCfg;

typedef struct _H26XEncGdrCfg_ {
	BOOL bEnable;

	UINT16 usPeriod;
	UINT16 usNumber;

    BOOL bGDRIFrm;
	BOOL bForceGDRQpEn;
	UINT8 ucGDRQp;
} H26XEncGdrCfg;

typedef struct _H26XEncRoiCfg_ {
	BOOL bEnable;

	UINT16 usCoord_X;
	UINT16 usCoord_Y;
	UINT16 usWidth;
	UINT16 usHeight;
	INT8   cQP;
	UINT8  ucMode;
} H26XEncRoiCfg;

typedef struct _H26XEncRowRCCfg_ {
	BOOL   bEnable;
	UINT8  ucPredWt[2]; // 0 : I. 1 : P
	UINT8  ucScale;
	UINT8  ucQPRange[2];
	UINT8  ucQPStep[2];
    UINT8  ucTileQPStep[2];
    BOOL   bTileQPRst;
	UINT8  ucMinQP[2]; // 0 : I. 1 : P
	UINT8  ucMaxQP[2]; // 0 : I. 1 : P
	UINT32 uiInitFrmCoeff;
    UINT8  ucNDQPRange;
    UINT8  ucNDQPStep;
} H26XEncRowRcCfg;

typedef struct _H26XEncAqCfg_ {
	BOOL bEnable;

	UINT8  ucIC2;
	UINT8  ucIStr;
	UINT8  ucPStr;
	INT8   cMaxDQp;
	INT8   cMinDQp;
	UINT8  ucAslog2;
	UINT8  ucDepth;
	UINT8  ucPlaneX;	//HEVC only
	UINT8  ucPlaneY;	//HEVC only
	INT16  sTh[H26X_AQ_TH_TBL_SIZE];

	UINT8  ucAqMode;
	UINT8  ucIStr1;     //HEVC only, cu32
	UINT8  ucPStr1;     //HEVC only, cu32
	UINT8  ucIStr2;     //HEVC only, cu16
	UINT8  ucPStr2;     //HEVC only, cu16
} H26XEncAqCfg;

typedef struct _H26XEncLpmCfg_ {
	BOOL bEnable;

	UINT8 ucRmdSadEn;
	UINT8 ucIMEStopEn;
	UINT8 ucIMEStopTh;
	UINT8 ucRdoStopEn;
	UINT8 ucRdoStopTh;
	UINT8 ucChrmDmEn;
	UINT8 ucQPMapDefulat;
	UINT8 ucI16On;
	UINT8 ucIraEn;
	UINT8 ucIraTh;
	UINT8 ucILFLpm;

	// 539A //
	UINT8  ucCodecMDLpmIraStopEn;
} H26XEncLpmCfg;

typedef struct _H26XEncLpmSetCfg_ {
	UINT8 ucIraEn;
	UINT8 ucCodecMDLpmIraStopEn;
} H26XEncLpmSetCfg;

typedef struct _H26XEncRndCfg_ {
	BOOL bEnable;

	unsigned int  uiSeed;
	unsigned char ucRange;
} H26XEncRndCfg;

typedef struct _H26XEncScdCfg_ {
	BOOL   bStop;

	UINT16 usTh;
	UINT8  ucSc;
	UINT8  ucOverrideRowRC;
} H26XEncScdCfg;

typedef struct _H26XEncOsgRgbCfg_ {
	UINT8  ucRgb2Yuv[3][3];
} H26XEncOsgRgbCfg;

typedef struct _H26XEncOsgPalCfg_ {
	UINT8 ucAlpha;
	UINT8 ucRed;
	UINT8 ucGreen;
	UINT8 ucBlue;
} H26XEncOsgPalCfg;

typedef struct _H26XEncOsgGrapCfg_ {
	UINT8  ucType;
	UINT16 usWidth;
	UINT16 usHeight;
	UINT16 usLofs;
	uintptr_t uiAddr;
	UINT8  ucDropUV;
} H26XEncOsgGrapCfg;

typedef struct _H26XEncOsgDispCfg_ {
	UINT8  ucMode;
	UINT16 usXStr;
	UINT16 usYStr;
	UINT8  ucBgAlpha;
	UINT8  ucFgAlpha;
	UINT8  ucMaskType;
	UINT8  ucMaskBdSize;
	UINT8  ucMaskY[2];
	UINT8  ucMaskCb;
	UINT8  ucMaskCr;
} H26XEncOsgDispCfg;

typedef struct _H26XEncOsgQpmapCfg_ {
	UINT8 ucLpmMode;
	UINT8 ucTnrMode;
	UINT8 ucFroMode;
	UINT8 ucQpMode;
	INT8 cQpVal;
	UINT8 ucBgrMode;
	UINT8 ucMaqMode;
	UINT8 ucSkipLabel;
} H26XEncOsgQpmapCfg;

typedef struct _H26XEncOsgColorkeyCfg_ {
	BOOL bEnable;
	BOOL bAlphaEn;
	UINT8 ucAlpha;
	UINT8 ucRed;
	UINT8 ucGreen;
	UINT8 ucBlue;
} H26XEncOsgColorkeyCfg;

typedef struct _H26XEncOsgMosaicCfg_ {
	UINT8 ucMosaicBlkSz;
} H26XEncOsgMosaicCfg;

typedef struct _H26XEncOsgWinCfg_ {
	BOOL bEnable;

	H26XEncOsgGrapCfg stGrap;
	H26XEncOsgDispCfg stDisp;
	H26XEncOsgQpmapCfg stQpmap;
	H26XEncOsgColorkeyCfg stKey;
    H26XEncOsgMosaicCfg stMosaic;
} H26XEncOsgWinCfg;

typedef struct _H26XEncOsgGlobalCfg_ {
    BOOL bChomaAlphaEn;
    BOOL bOverlapType;
    UINT8 ucMosaicBitMapBlkSz;
} H26XEncOsgGlobalCfg;

typedef struct _H26XEncMaskInitCfg_ {
	BOOL  bEnable;
	UINT8 ucMosaicBlkW;	// keep latest set mosaic blk w //
	UINT8 ucMosaicBlkH;	// keep latest set mosaci blk h //

	UINT8  ucPalY[16];
	UINT8  ucPalCb[16];
	UINT8  ucPalCr[16];
} H26XEncMaskInitCfg;

typedef struct _H26XEncMaskPosCfg_ {
	UINT16 usPosX;
	UINT16 usPosY;
} H26XEncMaskPosCfg;

typedef struct _H26XEncMaskWinCfg_ {
	BOOL bEnable;

	UINT8  ucDid;
	UINT8  ucPalSel;
	UINT8  ucLineHitOpt;
	UINT16 usAlpha;
	UINT8  ucAlphaType;
	UINT8  ucComp[4];
	UINT8  ucHitWidth[4]; // when line hit opt is 2 or 3, hit width is 2; when line hit opt is 0, 1 or 4, hit width is 0; range: 0~15

	H26XEncMaskPosCfg stPos[4];
	#ifndef VDOCDC_EMU
	UINT8  line_chk_th;
	#endif
} H26XEncMaskWinCfg;

typedef struct _H26XEncMotAqCfg_ {
	UINT8 ucMode[3];
	UINT8 uc8x8to16x16Th;
	UINT8 ucDqpRoiTh;
	UINT8 ucDqpMotTh;
	INT8  cDqp[6];
	UINT8 ucDqpnum;
} H26XEncMotAqCfg;

typedef struct _H26XEncMotAddrCfg_ {
	uintptr_t uiMotAddr[3];
	UINT32 uiMotLineOffset;
	UINT8  ucMotBufNum;
    UINT32 uiMotSize;
} H26XEncMotAddrCfg;

typedef struct _H26XEncMdInfoCfg_ {
    UINT32 uiMdWidth;
    UINT32 uiMdHeight;
    UINT32 uiMdLofs;
    uintptr_t uiMdBufAdr;
    UINT32 uiRot;
    UINT32 uiRoiXY;
    UINT32 uiRoiWH;
} H26XEncMdInfoCfg;

typedef struct _H26XEncJndCfg_ {
	UINT8 ucEnable;

	UINT8 ucStr;
	UINT8 ucLevel;
	UINT8 ucTh;
	UINT8 ucCStr;

	UINT8 ucR5Flag;
	UINT8 ucBilaFlag;
	UINT8 ucLsigmaTh;
	UINT8 ucLsigma;
	UINT8 ucLaNew;

	UINT8 ucYTH1;
	UINT8 ucYTH2;
	UINT8 ucCmTH1;
	UINT8 ucCmTH2;

	UINT8 ucUTH1;
	UINT8 ucUTH2;
	UINT8 ucVTH1;
	UINT8 ucVTH2;
} H26XEncJndCfg;

typedef struct _H26XEncUserDataCfg_{
	BOOL bEnable;
	uintptr_t uiDataAddr;
	UINT32 uiDataLength;
}H26XEncUserDataCfg;

#ifndef VDOCDC_EMU
typedef struct _H26XEncSdeCfg_{
	BOOL bEnable;
	UINT32 uiWidth;
	UINT32 uiHeight;
	UINT32 uiYLofst;
	UINT32 uiCLofst;
}H26XEncSdeCfg;
#else
typedef struct _H26XEncSdeCfg_{
	UINT32 uicfg[49];
}H26XEncSdeCfg;
#endif
typedef struct _H26XEncQpRelatedCfg_{
	UINT8 ucSaveDeltaQp;  // 0~15
}H26XEncQpRelatedCfg;

typedef struct _H26XEncNaluLenResult_{
	UINT32 uiSliceNum;
	uintptr_t uiVaAddr;
}H26XEncNaluLenResult;


#if H26X_SUPPORT_SCENE_DETECT
typedef struct _H26XEncSDParamBak_{
	BOOL   bBGRDO_en;			//0: disable, 1: enable
	INT8   cDqp[6];				//0: maq, 1: maq diff
}H26XEncSDParamBak;

typedef struct _H26XEncFgAddrCfg_ {
	uintptr_t uiFgAddr;
	UINT8  ucFgBufNum;
	UINT16 uiFgSize;
}H26XEncFgAddrCfg;

typedef struct _H26XEncSceneDetParam_ {
	H26X_SD_SCENE_TYPE uiSceneType;
	H26X_SD_OPTIMIZED_FLAG reset_flag;
	UINT32	uiFgSum;
	UINT32	uiAccFgSum;
	INT16	gmv_x_sumy[GMV_TEMPORAL_BUFFER_SIZE][GMV_CNT/4];
	INT16	gmv_y_sumx[GMV_TEMPORAL_BUFFER_SIZE][GMV_CNT/4];
}H26XEncSceneDetParam;

typedef struct _H26XEncSceneDetCfg_{
	BOOL   					bEnable;
	H26X_SD_CAM_TYPE 		ucCamType;			//0: FIXED CAM, 1: CAR CAM, 2: SPORT CAM
	//proc
	H26X_SD_OPTIMIZED_FLAG	optimized_flag;
	UINT8  					ucFGProtectThr;	//set foreground protect when foreground ratio is larger than thr
	UINT8  					ucStaticBlkThr;	//sperate a frame into 4x4 block, if block MD < ucStaticBlkThr then the blk is static blk(range: 0~100)
	UINT8  					ucMotionBlkThr;	//sperate a frame into 4x4 block, if block MD > ucMotionBlkThr then the blk is motion blk(range: 0~100)
	UINT8  					ucStaticFrmThr;	//sperate a frame into 4x4 block, if static blocks > ucStaticFrmThr then the frm is static frm(range: 0~16)
	UINT8  					ucMotionFrmThr;	//sperate a frame into 4x4 block, if static blocks > ucMotionFrmThr then the frm is motion frm(range: 0~16)
}H26XEncSceneDetCfg;
#endif

#if H26X_SUPPORT_SMART_ROI
#define H26X_SMART_ROI_MAX_NUM 	    32
#define H26X_SMART_ROI_CLASS_NUM	5
typedef struct {
	UINT32 tl_x;
	UINT32 tl_y;
	UINT32 rb_x;
	UINT32 rb_y;
	UINT32 class_id;                    ///< bounding box class
} H26XEncBBox;

typedef struct {
	UINT32		base_width;
	UINT32		base_height;                       ///< base resolution
	UINT8		bbox_num;                          ///< number of bounding boxes, range: KDRV_VDOENC_SMART_ROI_MAX_NUM
	H26XEncBBox bbox[H26X_SMART_ROI_MAX_NUM];      ///< bounding boxes
	UINT64 		timestamp;                         ///< timestamp of bounding boxes
} H26XEncSmartBBoxCfg;

typedef struct {
	BOOL	enable;                                ///< enable AI smart encode, range: 0~1
	UINT8	fg_str[H26X_SMART_ROI_CLASS_NUM];      ///< foreground strength of each classes, range: 0~255
	UINT8	mode;                                  ///< 0: adjust qp only, 1: adjust qp + BGRDO
} H26XEncSmartRoiCfg;
#endif

#if H26X_SUPPORT_DYNAMIC_FR
#define DYNAMIC_FR_MAX_PERIOD 5
typedef enum {
	H26X_DYN_FR_ALL_SKIP,
	H26X_DYN_FR_STATIC,
	H26X_DYN_FR_MOTION,
	H26X_DYN_FR_FULL_FPS,
} H26XEncDynFRState;

typedef struct {
	BOOL	enable;						///< enable dynamic frame rate, range: 0~1,     default: 0
	UINT32 	min_fps;					///< the minimal frame rate,    range: 1~fps-1, default: fps / 3
	UINT32	motion_sensitivity;			///< the bias for fps refresh,  range: 0~15,    default: 14
	//proc
	BOOL	bAllSkip;
	UINT32	mid_fps;
	UINT32	allSkipMD_thr;
	UINT32	fullFPSMD_thr;
	UINT32	motion2StaticMD_thr;
	UINT32	static2MotionMD_thr;
	UINT32	static2AllSkipCnt_thr;
	UINT32	motion2StaticCnt_thr;
	UINT32	static2MotionCnt_thr;
	UINT32	delay_gopCnt;
} H26XEncDynFRCfg;

typedef struct {
	//Dyn FR state ctrl
	H26XEncDynFRState state;
	UINT32	static2AllSkipCnt;
	UINT32	static2MotionCnt;
	UINT32	motion2StaticCnt;
	UINT32	skipFrmNum;
	UINT32	pFrmNum;
	UINT32	frmCnt;
	UINT32	period;
	UINT8	ucRecordSkip[DYNAMIC_FR_MAX_PERIOD];
	//RC ctrl
	BOOL	bTrigger;
	UINT32	gopCnt;
} H26XEncDynFRParam; //internal data, should never be modify by anyone at anytime
#endif

#if H26X_SUPPORT_DYNAMIC_GOP
#define MD_MAX_RECORD_SIZE 60
typedef enum {
	H26X_DYN_GOP_STATIC,
	H26X_DYN_GOP_MOTION,
} H26XEncDynGopState;

typedef struct {
	BOOL	enable;					///< enable dynamic frame rate,      range: 0~1,        default: 0
	UINT32	max_gop;				///< the maximal gop of dynamic gop, range: gop ~ 4096, default: gop * 5 ~ gop * 10
	UINT32	motion_sensitivity;		///< the bias for fps refresh,       range: 0~15,       default: 8
	//proc
	BOOL	sceneChangeInsertI;
	UINT32	delay_gopCnt;
	UINT32	abruptSC_thr;
	UINT32	static2MotionMD_thr;
	UINT32	motion2StaticMD_thr;
	UINT32	motion2StaticCnt_thr;
	UINT32	delayFlashIPicCnt_thr;
} H26XEncDynGopCfg;

typedef struct _H26XEncDynGopParam_{
	//state ctrl
	H26XEncDynGopState state;
	H26XEncDynGopState lastState;
	UINT32	delayPicCnt;
	UINT32	motion2StaticCnt;
	UINT32	min_gop;
	//statistic val
	//UINT32	LastIEdgeSum;
	//UINT32	LastEdgeSum;
	//UINT32	edgeSumCnt;
	//INT32	LastIPSNR;
	//INT32	LastPPSNR;
	//RC ctrl
	BOOL	bTrigger;
	BOOL	bInsertIGOPMultiple;
	UINT32	gopCnt;
}H26XEncDynGopParam; //internal data, should not be modify by anyone at anytime
#endif

typedef struct _H26XEncRmdCfg_{
	UINT8 ucIraModeBit0[2];	//264: ucRmdVh4Y, 265: RMD PL Y, i/p
	UINT8 ucIraModeBit1[2];	//264: ucRmdVh8Y, 265: RMD DC Y, i/p
	UINT8 ucIraModeBit2[2];	//264: ucRmdVh16Y, 265: RMD HR Y, i/p
	UINT8 ucIraModeBit3[2];	//264: ucRmdPl16Y, 265: RMD VT Y, i/p
	UINT8 ucIraModeBit4[2];	//264: ucRmdOt4Y, 265: RMD OT Y, i/p
	UINT8 ucIraModeBit5[2];	//264: ucRmdOt8Y, i/p
	UINT8 ucIraModeBitShift[2];	//IRA_MODE_BITS_SHIFT, only shift for IRA_MODE_BITS_0 ~ IRA_MODE_BITS_4, i/p
}H26XEncRmdCfg;

typedef struct _H26XEncLambdaCfg_{
    BOOL  bEnable;

	// 538 //
    INT16 usLMTbl[2][52];
    INT16 usSLMTabl[2][52];

	// 539A //
	INT8 cDeltaSTA[2];
	INT8 cDeltaMOT[2];
	INT8 cDeltaGRD[2];
	UINT8 ucGDRTh[2];
}H26XEncLambdaCfg;

typedef struct _H26XEncEarlySkipCfg_{
	UINT8  ucMDTh;
	UINT8  ucMVTh;
	UINT32 uiSkipCostTh[2];

	// 539A //
	BOOL   bCodecMDSkipEn;
} H26XEncEarlySkipCfg;

typedef struct _H26XEncSpnCfg_{
    BOOL   bEnable;
    UINT8  ucConEng;
    UINT16  usSlopConEng;
    UINT8  ucBHC;
    UINT8  ucDHC;
    UINT8  ucEWT;
    UINT8  ucEWG;
    //UINT8  ucESS[2];
    UINT8  ucESS;
    //UINT8  ucFSS;
    UINT8  ucCT;
    UINT8  ucNL;
    UINT8  ucBIG;
    UINT16 usFlatTh;
    UINT16 usEdgeTh;
    UINT8  ucLowWt; // n/a
    UINT8  ucHightWt; // n/a
    UINT16 usHLDFlatTh; // n/a
    UINT16 usHLDEdgeTh; // n/a
    UINT8  ucHLDLowWt; // n/a
    UINT8  ucHLDHightWt; // n/a
    UINT8  ucEdgeStr;
    UINT8  ucTransitionStr;
    UINT8  ucMotionStr;
    UINT8  ucStaticStr;
    UINT8  ucHLDSlopConEng; // n/a
    UINT8  ucFlatStr;
    UINT16 usHLDLumTh; // n/a
    UINT8  ucNC[17];
    UINT16 usEWG[9];
    BOOL   bShowSharpInfo;
} H26XEncSpnCfg;

typedef struct _H26XEncResultCfg_{
    UINT32 uiBSLen;
    UINT32 uiBSChkSum;
    UINT32 uiRDOCost[2];
	//UINT32 uiQPSum;
	UINT32 uiAvgQP;
	UINT32 uiYPSNR[2];
    UINT32 uiUPSNR[2];
    UINT32 uiVPSNR[2];
	UINT32 uiMOTION_YPSNR[2];
    UINT32 uiMOTION_UPSNR[2];
    UINT32 uiMOTION_VPSNR[2];

    UINT32 uiRecYHitCnt;
    UINT32 uiRecCHitCnt;
    //UINT32 uiInterCnt;
    //UINT32 uiSkipCnt;
    UINT32 uiMotionRatio;

    INT32 iVPSHdrLen;
    INT32 iSPSHdrLen;
    INT32 iPPSHdrLen;

    UINT32 uiInterCnt;
    UINT32 uiSkipCnt;
    UINT32 uiMergeCnt;
    UINT32 uiIntra4Cnt;
    UINT32 uiIntra8Cnt;
    UINT32 uiIntra16Cnt;
    UINT32 uiIntra32Cnt;
    UINT32 uiCU64Cnt;
    UINT32 uiCU32Cnt;
    UINT32 uiCU16Cnt;

	UINT32 uiSvcLable;
	BOOL   bRefLT;
    BOOL   bEVBRStillFlag;
	UINT8  ucNxtPicType;
	UINT8  ucQP;
	UINT8  ucPicType;
	UINT32 uiHwEncTime;

	UINT32 uiSHAState[8];
}H26XEncResultCfg;

typedef struct _H26XEncMemInfo_{
	UINT32 uiWidth;
	UINT32 uiHeight;
	UINT32 ucSVCLayer;
	UINT32 uiLTRInterval;
	BOOL   bTileEn;
	BOOL   bD2dEn;
	BOOL   bGdcEn;
	BOOL   bD2d1TileEn;
	BOOL   bColMvEn;
	UINT8  ucQualityLevel;
	BOOL   bCommReconFrmBuf;
	UINT32  uiLtrSavingBufBsSize;  //0: LTR need to mallocate REC buffer, > 0: LTR Saving Rec Mode, unit is byte
	H26X_XVR_APP xvr_app; // for 265 tile mode boundary
	UINT8 bFBCEn;
	UINT32 uiRecLineOffset;
#ifdef VDOCDC_EMU
	UINT8  ucTileNum;
	UINT32 uiTileWidth[H265E_TILE_MAX];
	UINT16 usSliceNum;
#endif
} H26XEncMeminfo;

typedef struct _H26XEncMAQDiffInfo_ {
#if H26X_USE_DIFF_MAQ
    uintptr_t uiHistMotAddr[H26X_MOTION_BITMAP_NUM+1];
    UINT32 uiMotFrmCnt;
    UINT8  ucCurMotIdx;
#endif
} H26XEncMAQDiffInfo;

typedef struct _H26XEncMAQDiffInfoCfg_{
	BOOL enable;
	INT8 str;
	UINT8 start_idx;
	UINT8 end_idx;
} H26XEncMAQDiffInfoCfg;

typedef struct _H26XEncMAQBGBiasCfg_{
	BOOL enable;
	UINT8 mode;
} H26XEncMAQBGBiasCfg;

typedef struct _H26xEncResSupCfg_{
	BOOL en;
	BOOL en_c;
	UINT8 opt;
	UINT8 noise_th;
	UINT8 mean_bnd1;
	UINT8 mean_bnd2;

	INT8 peak_th;
	UINT8 peak_rng;
	UINT8 edge_level;
	UINT8 la_th;
	INT8 ib_th1;
	UINT8 ib_th2;

	UINT8 qp_bnd1;
	UINT8 qp_bnd2;
	UINT16 str;
	UINT16 str_c;
} H26XEncResSupCfg;

typedef struct _H26XEncMotFilterCfg_{
	BOOL  bEnable;
	UINT8 ucTHR;
	INT8  cFilterDQP;
	INT8  cLabelDQP;
} H26XEncMotFilterCfg;

typedef struct _H26XEncMotDetAddrCfg_ {
	UINT32		uiMotDetLineOffset;
    UINT32		uiMotDetSize;
} H26XEncMotDetAddrCfg;

typedef struct _H26XEncMotDetectCfg_{
	BOOL  bEnable;
	UINT8 ucMode;
	INT8  cDQP[2];
	UINT8 ucGain;
	UINT8 ucFrmOffset;
	UINT8 ucQOffset;
	UINT8 ucQStep;

	INT8   cUsrOffset;
	UINT16 usBoundaryThr;
} H26XEncMotDetectCfg;

typedef struct _H26XEncPartFrmSkipCfg_{
	BOOL  bEnable;
	UINT8 ucLeft;
	UINT8 ucTop;
	UINT8 ucRight;
	UINT8 ucBot;
} H26XEncPartFrmSkipCfg;

typedef struct _H26XEncWaterMarkGlobalCfg_{
	BOOL   bEnable;
	UINT8  bUV;
	UINT8  ucSTR;
	UINT32 uiLT;
	UINT8  ucSize;
} H26XEncWaterMarkGlobalCfg;

typedef struct _H26XEncWaterMarkInfoCfg_{
	uintptr_t uiAddr;
} H26XEncWaterMarkInfoCfg;

typedef struct _H26XEncWaterMarkCfg_{
    H26XEncWaterMarkGlobalCfg  stWTMKGlobal[8];
    H26XEncWaterMarkInfoCfg    stWTMKInfo[8];
} H26XEncWaterMarkCfg;

typedef struct _H26XEncInfoCfg_{
	UINT32 uiEncId;
	CODEC_TYPE eCodecType;
	UINT32 uiWidth;
	UINT32 uiHeight;
	UINT32 uiGop;
	UINT32 uiRcMode;
	UINT32 uiRefNum;
	uintptr_t uiRefAddr[3];
	UINT32 uiRefSize;
}H26XEncInfoCfg;

typedef struct _H26XEncTurboCfg_ {
    BOOL	bEnable;
	UINT16	usSkipTh;
	UINT8	ucInitTxTh;
    UINT16  usMvDiffTh;
	BOOL	bInterType;			// h265 only
	UINT8	ucImeScaleCBias;	// h265 only
	UINT8	ucImeScalePBias;	// h265 only
    UINT16  usCostTh[3];
	UINT8	ucMode;				// h265 only
	UINT8   ucMergeBias;		// h265 only
} H26XEncTurboCfg;

extern INT32 h26XEnc_setUsrQpCfg(H26XENC_VAR *pVar, H26XEncUsrQpCfg *pUsrQp);
extern INT32 h26XEnc_setPSNRCfg(H26XENC_VAR *pVar, BOOL bEnable);
extern INT32 h26XEnc_setSliceSplitCfg(H26XENC_VAR *pVar, H26XEncSliceSplitCfg *pSliceSplit);
extern INT32 h26XEnc_setVarCfg(H26XENC_VAR *pVar, H26XEncVarCfg *pVarCfg);
extern INT32 h26XEnc_setGdrCfg(H26XENC_VAR *pVar, H26XEncGdrCfg *pGdrCfg);
extern INT32 h26XEnc_setRoiCfg(H26XENC_VAR *pVar, UINT8 ucIdx, H26XEncRoiCfg *pRoiCfg);
extern INT32 h26XEnc_setRowRcCfg(H26XENC_VAR *pVar, H26XEncRowRcCfg *pRowRcCfg);
extern INT32 h26XEnc_setAqCfg(H26XENC_VAR *pVar, H26XEncAqCfg *pAqCfg);
extern INT32 h26XEnc_setLpmCfg(H26XENC_VAR *pVar, H26XEncLpmCfg *pLpmCfg);
extern INT32 h26XEnc_setUserDataCfg(H26XENC_VAR *pVar, H26XEncUserDataCfg *pUserDataCfg);
extern INT32 h26XEnc_setRndCfg(H26XENC_VAR *pVar, H26XEncRndCfg *pRndCfg);
extern INT32 h26XEnc_setScdCfg(H26XENC_VAR *pVar, H26XEncScdCfg *pScdCfg);
extern INT32 h26xEnc_setMaskInitCfg(H26XENC_VAR *pVar, H26XEncMaskInitCfg *pMaskInitCfg);
extern INT32 h26xEnc_setMaskWinCfg(H26XENC_VAR *pVar, UINT8 ucIdx, H26XEncMaskWinCfg *pMaskWinCfg);
extern INT32 h26XEnc_setOsgRgbCfg(H26XENC_VAR *pVar, H26XEncOsgRgbCfg *pOsgRgbCfg);
extern INT32 h26XEnc_setOsgPalCfg(H26XENC_VAR *pVar, UINT8 ucIdx, H26XEncOsgPalCfg *pOsgPalCfg);
extern INT32 h26XEnc_setOsgWinCfg(H26XENC_VAR *pVar, UINT8 ucIdx, H26XEncOsgWinCfg *pOsgWinCfg);
extern INT32 h26XEnc_setMotAqCfg(H26XENC_VAR *pVar, H26XEncMotAqCfg *pMAqCfg);
//extern INT32 h26XEnc_setMotAddrCfg(H26XENC_VAR *pVar, H26XEncMotAddrCfg *pMotAddrCfg);
extern INT32 h26XEnc_setMdInfoCfg(H26XENC_VAR *pVar, H26XEncMdInfoCfg *pMdInfoCfg);
#if H26X_UQPMAP_FROM_MD
extern INT32 h26XEnc_setUsrQPMap(H26XENC_VAR *pVar, H26XEncMdInfoCfg *pMdInfoCfg);
extern INT32 h26XEnc_setBGRdoCfg(H26XENC_VAR *pVar, BOOL enable, UINT8 mode);
#endif
extern INT32 h26XEnc_setJndCfg(H26XENC_VAR *pVar, H26XEncJndCfg *pJndCfg);
extern INT32 h26XEnc_setQpRelatedCfg(H26XENC_VAR *pVar, H26XEncQpRelatedCfg *pQRCfg);
extern INT32 h26XEnc_setRcInit(H26XENC_VAR *pVar, H26XEncRCParam *pRcParam);
extern void h26XEnc_SetIspRatio(H26XENC_VAR *pVar, H26XEncIspRatioCfg *pIspRatio);
// nt98520 //
extern INT32 h26XEnc_setRmdCfg(H26XENC_VAR *pVar, H26XEncRmdCfg *pIraCfg, SLICE_TYPE pic_type);
extern INT32 h26XEnc_setLambdaCfg(H26XENC_VAR *pVar, H26XEncLambdaCfg *pLambdaCfg, SLICE_TYPE pic_type);
extern INT32 h26XEnc_setIspCbCfg(H26XENC_VAR *pVar, H26XEncIspCbCfg *pIspCbCfg);
extern uintptr_t h26xEnc_getVaAPBAddr(H26XENC_VAR *pVar);
extern uintptr_t h26xEnc_getVaLLCAddr(H26XENC_VAR *pVar);
extern INT32 h26xEnc_getNaluLenResult(H26XENC_VAR *pVar, H26XEncNaluLenResult *pResult);
extern void h26xEnc_getGdrCfg(H26XENC_VAR *pVar, H26XEncGdrCfg *pGdrCfg);
extern void h26xEnc_getAqCfg(H26XENC_VAR *pVar, H26XEncAqCfg *pAqCfg);
extern UINT32 h26xEnc_getBsLen(H26XENC_VAR *pVar);
extern void h26xEnc_getIspRatioCfg(H26XENC_VAR *pVar, H26XEncIspRatioCfg *pIspRatioCfg);
extern void h26xEnc_getRowRcCfg(H26XENC_VAR *pVar, H26XEncRowRcCfg *pRrcCfg);

#if H26X_SET_PROC_PARAM
extern int h26xEnc_getRCDumpLog(void);
extern int h26xEnc_setRCDumpLog(int value);
#endif
extern int h26xEnc_getNDQPStep(void);
extern int h26xEnc_setNDQPStep(int value);
extern int h26xEnc_getNDQPRange(void);
extern int h26xEnc_setNDQPRange(int value);
extern int h26xEnc_getRRCSyncQPCond(void);
extern int h26xEnc_setRRCSyncQPCond(int value);
#if H26X_DIS_OSG_CACHE_OPERATION
extern int h26xEnc_getOsgCacheFlush(void);
extern int h26xEnc_setOsgCacheFlush(int value);
#endif
extern int h26xEnc_getESMVTh(void);
extern int h26xEnc_setESMVTh(int value);
#if H26X_SET_MB_LIMIT_QP_IND
extern int h26xEnc_getIMinDeltaMBQP(void);
extern int h26xEnc_setIMinDeltaMBQP(int value);
extern int h26xEnc_getIMaxDeltaMBQP(void);
extern int h26xEnc_setIMaxDeltaMBQP(int value);
extern int h26xEnc_getPMinDeltaMBQP(void);
extern int h26xEnc_setPMinDeltaMBQP(int value);
extern int h26xEnc_getPMaxDeltaMBQP(void);
extern int h26xEnc_setPMaxDeltaMBQP(int value);
#endif

extern int h26xEnc_getVersion(void);
#if H26X_MEM_USAGE
extern UINT32 h26xEnc_getMemUsage(UINT32 type, UINT32 id);
#endif
extern UINT32 h26xEnc_queryRecFrmNum(const H26XEncMeminfo *pInfo);
extern UINT32 h26xEnc_queryMinBSBufSize(UINT32 uiWidth, UINT32 uiHeight, UINT32 uiMaxIQp);

extern INT32 h26XEnc_setEarlySkipCfg(H26XENC_VAR *pVar, H26XEncEarlySkipCfg *pEarlySkipCfg);
extern INT32 h26XEnc_setSpnCfg(H26XENC_VAR *pVar, H26XEncSpnCfg *pSpnCfg);

#if LPM_PROC_ENABLE
void h26XEnc_getLpmCfg(H26XENC_VAR *pVar, H26XEncLpmCfg *pLpmCfg);
extern BOOL h26xEnc_getInterOnlyCfg(H26XENC_VAR *pVar);
#endif

#if H26X_USE_DIFF_MAQ
extern int h26xEnc_checkDiffMAQEnable(H26XENC_VAR *pVar);
extern int h26XEnc_setMaqDiffBitmap(H26XENC_VAR *pVar, H26XEncMdInfoCfg *pMdInfoCfg);
extern void h26XEnc_setMAQDiffInfoCfg(H26XENC_VAR *pVar, H26XEncMAQDiffInfoCfg *pMAQDiffInfoCfg);
extern void h26XEnc_getMAQDiffInfoCfg(H26XENC_VAR *pVar, H26XEncMAQDiffInfoCfg *pMAQDiffInfoCfg);
#endif
extern int h26xEnc_getMDMode(void);
extern int h26xEnc_setMDMode(int value);

extern void h26xEnc_setInterOnlyCfg(H26XENC_VAR *pVar, BOOL enable);
extern INT32 h26xEnc_setResSupCfg(H26XENC_VAR *pVar, H26XEncResSupCfg *pResSupCfg);
extern void h26xEnc_setMotFilterCfg(H26XENC_VAR *pVar, H26XEncMotFilterCfg *pMotFilterCfg);
extern void h26xEnc_setMotDetectCfg(H26XENC_VAR *pVar, H26XEncMotDetectCfg *pMotDetectCfg);
extern void h26xEnc_setPartFrmSkipCfg(H26XENC_VAR *pVar, H26XEncPartFrmSkipCfg *pPartFrmSkipCfg);
extern void h26xEnc_setWaterMarkCfg(H26XENC_VAR *pVar, H26XEncWaterMarkCfg *pWTMKCfg);

extern INT32 h26XEnc_setSdecCfg(H26XENC_VAR *pVar, H26XEncSdeCfg *pSdecCfg);
extern void h26XEnc_setSHA256(H26XENC_VAR *pVar, unsigned int state[8], BOOL rst);
//void h26xEnc_setResSupCfg(H26XENC_VAR *pVar, H26XEncResSupCfg *pResSupCfg);
extern INT32 h26XEnc_setMotAddrCfg(H26XENC_VAR *pVar, H26XEncMotAddrCfg *pMotAddrCfg);
extern uintptr_t h26xEnc_getMaqMotAddr(H26XENC_VAR *pVar, UINT32 i);
extern UINT32 h26xEnc_getMaqMotSize(H26XENC_VAR *pVar);
extern INT32 h26XEnc_setOsgGlobalCfg(H26XENC_VAR *pVar, H26XEncOsgGlobalCfg *pOsgGlobal);
extern UINT32 h26xEnc_getMinBSRatio(void *param);
extern UINT32 h26xEnc_getEncInfo(H26XENC_VAR *pVar, H26XEncInfoCfg *pEncInfo);
extern UINT8 h26xEnc_getRCMode(H26XENC_VAR *pVar);

extern UINT32 h26xEnc_getLastSliceForSliceLowLatency(H26XENC_VAR *pVar);
extern void h26xEnc_getJndCfg(H26XENC_VAR *pVar, H26XEncJndCfg *pJndCfg);
extern void h26xEnc_getMotDetectCfg(H26XENC_VAR *pVar, H26XEncMotDetectCfg *pMotDetectCfg);
extern void h26xEnc_getMotFilterCfg(H26XENC_VAR *pVar, H26XEncMotFilterCfg *pMotFilterCfg);
extern void h26xEnc_getResSupCfg(H26XENC_VAR *pVar, H26XEncResSupCfg *pResSupCfg);

extern void h26XEnc_setIspParamCfg(H26XENC_VAR *pVar, H26XEncIspParamCfg *pIspParamCfg);
#if H26X_SUPPORT_SCENE_DETECT
extern INT32 h26XEnc_SceneTrigger(H26XENC_VAR *pVar, H26XEncMdInfoCfg *pMdInfoCfg);
extern void h26XEnc_setSceneDetCfg(H26XENC_VAR *pVar, H26XEncSceneDetCfg* pSceneDetCfg);
extern void h26XEnc_getSceneDetCfg(H26XENC_VAR *pVar, H26XEncSceneDetCfg* pSceneDetCfg);
#endif
#if H26X_SUPPORT_SMART_ROI
extern INT32 h26xEnc_setSmartRoiCfg(H26XENC_VAR *pVar, H26XEncSmartRoiCfg *pSmartRoiCfg);
extern INT32 h26xEnc_setSmartBBoxCfg(H26XENC_VAR *pVar, H26XEncSmartBBoxCfg *pSmartBboxCfg);
extern INT32 h26XEnc_setSmartRoi2UserQP(H26XENC_VAR *pVar, BOOL smt_roi_update);
extern INT32 h26XEnc_smartROI2SEI(H26XENC_VAR *pVar, CODEC_TYPE eCodecType, UINT8* out_stream);
#endif
#if H26X_SUPPORT_DYNAMIC_FR
extern INT32 h26xEnc_setDynFRCfg(H26XENC_VAR *pVar, H26XEncDynFRCfg *pDynFRCfg);
extern INT32 h26xEnc_setDynFRProcCfg(H26XENC_VAR *pVar, H26XEncDynFRCfg *pDynFRCfg);
extern INT32 h26xEnc_setDynFRParam(H26XENC_VAR *pVar, H26XEncDynFRParam *pDynFRParam);
extern INT32 h26xEnc_SkipPframe(H26XENC_VAR *pVar);
#endif
#if H26X_SUPPORT_DYNAMIC_GOP
extern INT32 h26xEnc_setDynGopCfg(H26XENC_VAR *pVar, H26XEncDynGopCfg *pDynGopCfg);
extern INT32 h26xEnc_setDynGopProcCfg(H26XENC_VAR *pVar, H26XEncDynGopCfg *pDynGopCfg);
extern INT32 h26xEnc_setDynGopParam(H26XENC_VAR *pVar, H26XEncDynGopParam *pDynGopParam);
extern INT32 h26xEnc_InsertIframe(H26XENC_VAR *pVar);
#endif

#if H264_AEFIFO_RRC_BIAS
extern int h26xEnc_set264TBias(int value);
extern int h26xEnc_get264TBias(void);
#endif

extern void h26xEnc_setSEISignedVdo(H26XENC_VAR *pVar, H26XEncSignedVdoCfg *pSignedVdoCfg);
extern void h26xEnc_getSEISignedVdo(H26XENC_VAR *pVar, H26XEncSignedVdoCfg *pSVCfg);
extern void h26xEnc_updateSEISignedVdo(H26XENC_VAR *pVar, UINT32 gop_idx, UINT64 src_timestamp);

extern INT32 h26xEnc_setTurboCfg(H26XENC_VAR *pVar, H26XEncTurboCfg *pTurboCfg);
extern void h26XEnc_switchLpmSetByMdMode(H26XENC_VAR *pVar);
#endif // _H26XENC_API_H_
