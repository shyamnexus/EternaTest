#ifndef _H264_INT_H_
#define _H264_INT_H_

#include <kwrap/type.h>

#include "h26x.h"
#include "h26x_def.h"
#include "h26x_common.h"
#include "h26xenc_int.h"

#include "h264_def.h"
#include "h264enc_api.h"

typedef struct _H264EncAddr_ {
	UINT32    uiFrmBufNum;

	uintptr_t uiRecRefY[FRM_IDX_MAX], uiRecRefC[FRM_IDX_MAX];
	uintptr_t uiColMvs[FRM_IDX_MAX];
	uintptr_t uiSideInfo[2][FRM_IDX_MAX];
	UINT32 uiSideInfoLineOffset;
	uintptr_t uiRRC[MAX_RRC_FRAME_LEVEL][2];
	uintptr_t uiMotionBitOut[FRM_IDX_MAX];

	// VENC //
	uintptr_t uiSeqHdr;
	uintptr_t uiPicHdr;
	uintptr_t uiBsdma;

	uintptr_t uiSizeColMVs;
} H264EncAddr;

typedef struct _H264EncSeqCfg_ {
	H264_PROFILE eProfile;
	H264_ENTROPY eEntropyMode;

	UINT8   ucLevelIdc;
	UINT16	usWidth;
	UINT16	usHeight;
	UINT16	usMbWidth;
	UINT16	usMbHeight;
	UINT32	uiTotalMBs;

	UINT32	uiDisplayWidth;
	UINT32	uiGopNum;
	UINT16	usLog2MaxFrm;
	UINT16	usLog2MaxPoc;

	INT8	cChrmQPIdx;
	INT8	cSecChrmQPIdx;
	UINT8   ucDisLFIdc;
	INT8    cDBAlpha;
	INT8    cDBBeta;

	BOOL	bTrans8x8En;
	BOOL	bDirect8x8En;		// with profile, not from init  //
	BOOL	bForwardRecChrmEn;
	BOOL	bSraqSaveDQPEn;

	// reference frame //
	UINT8	ucSVCLayer;
	UINT32	uiLTRInterval;
	BOOL	bLTRPreRef;
	UINT8	ucNumRefIdxL0;

	// rate control //
	UINT32	uiBitRate;
	UINT32	uiFrmRate;
	UINT8	ucInitIQP;
	UINT8	ucInitPQP;

	// function //
	BOOL	bFBCEn;
	BOOL	bGrayEn;
	BOOL	bGrayColorEn;
	BOOL	bFastSearchEn;
	BOOL	bHwPaddingEn;
	UINT8	ucRotate;
	BOOL    bD2dEn;
	BOOL    bColMvEn;
	BOOL    bSetCu16Qp;
	BOOL    bDebreathEn;
	BOOL    bImeAmvpMod;
	BOOL    bImeGetIntraMv;
	UINT8   ucMosaicCtrl;

	// vui header //
	BOOL	bVUIEn;
	UINT16  usSarWidth;
	UINT16  usSarHeight;
	UINT8   ucMatrixCoef;
	UINT8   ucTransferCharacteristics;
	UINT8   ucColourPrimaries;
	UINT8   ucVideoFormat;
	UINT8   ucColorRange;
	UINT8   bTimeingPresentFlag;
	H26XEncFrmCropInfo frm_crop_info;
} H264EncSeqCfg;

typedef struct _H264EncPicCfg_ {
	SLICE_TYPE ePicType;
	SLICE_TYPE eRetPicType;
	SLICE_TYPE eNxtPicType;

	UINT8	ucSliceQP;

	//UINT32	uiPicCnt;	///< every picture ++
	UINT32	uiPicCntInGop;	///< Nth picture in gop

	UINT16  usFrmNum;
	UINT16  usFrmForIdrPicId;
	INT16	sPoc;
	INT16	sRefPoc;

	UINT32	uiNalRefIdc;
	BOOL	bLastRecIsLT;

	UINT32	config_loop_filter;	// inloop filter (ilf) //
	UINT32	cabac_init_idc;

	UINT32	uiSrcTimeStamp;
	UINT32  uiFrameSkip;
	UINT32	uiPicHdrLen;
	BOOL	bFBCEn[FRM_IDX_MAX];
} H264EncPicCfg;

typedef struct _H264ENC_CTX_ {
	H264EncSeqCfg	stSeqCfg;
	H264EncPicCfg	stPicCfg;
	H264EncAddr     stAddr;
	H264EncRdo	stRdo;
	H264EncFroCfg   stFroCfg;

	UINT32 uiRecYLineOffset;
	UINT32 uiRecCLineOffset;
	UINT32 uiSideInfoLineOffset;

	H26XFRM_IDX eRecIdx;
	H26XFRM_IDX eRefIdx;
	H26XFRM_IDX eLastRecIdx;

	UINT32 uiSeqHdrLen;
	UINT32 uiSPSHdrLen;
	UINT32 uiPPSHdrLen;
	UINT32 uiSvcLable;
	BOOL   bSEIPayloadBsChksum;
	UINT32 uiBsCheckSum;
} H264ENC_CTX;

extern void h264Enc_initCfg(H264ENC_INIT *pInit, H26XENC_VAR *pVar);
extern void h264Enc_updatePicCfg(H264ENC_CTX *pVdoCtx);
extern void h264Enc_modifyRefFrm(H264ENC_CTX *pVdoCtx, UINT32 uiPicCnt, BOOL bRecFBCEn);
extern void h264Enc_updateRowRcCfg(SLICE_TYPE ePicType, H26XEncRowRc *pRowRc);

#endif

