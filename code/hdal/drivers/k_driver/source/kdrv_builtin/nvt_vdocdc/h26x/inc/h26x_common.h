#ifndef _H26X_COMMON_H_
#define _H26X_COMMON_H_

#if defined(__LINUX)
#include <linux/mm.h>
#elif defined(__FREERTOS)
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#endif

#include "kwrap/type.h"

#include "h26x_def.h"
#include "h26xenc_rc.h"
#include "kdrv_videoenc/kdrv_videoenc_lmt.h"

#define Clip3_JM(min,max,val)	(((val)<(min))?(min):(((val)>(max))?(max):(val)))

typedef enum {
	FRM_IDX_ST_0 = 0,
	FRM_IDX_LT_0,
	FRM_IDX_ST_1,
	FRM_IDX_NON_REF,
	FRM_IDX_MAX,
	ENUM_DUMMY4WORD(H26XFRM_IDX)
} H26XFRM_IDX;

typedef enum{
	H26X_LL_START = 0,
	H26X_LL_WR,
	H26X_LL_RD,
	H26X_LL_FINISH = 15
}H26X_LL_CMD;

typedef enum {
	H26X_XVR_DISABLE = 0,
	H26X_XVR_56XE_528D = 1,
	H26X_XVR_RESERVED1,
	H26X_XVR_RESERVED2,
	H26X_XVR_RESERVED3
} H26X_XVR_APP;

typedef enum {
	H26X_CODEC_MD = 0,
	H26X_EXTERNAL_MD = 1,
} H26X_MD_TYPE;

#if H26X_SUPPORT_SCENE_DETECT
typedef enum{
	H26X_SD_FIXED_CAM = 0,
	H26X_SD_CAR_CAM,
	H26X_SD_SPORT_CAM,
	H26X_SD_CAM_MAX
}H26X_SD_CAM_TYPE;

typedef enum{
	H26X_SD_DEFAULT_SCENE        = 0x00000000,
	H26X_SD_STATIC_SCENE         = 0x00000001,
	H26X_SD_MOTION_SCENE         = 0x00001000,
	H26X_SD_ZOOM_IN_SCENE        = 0x00010000 | H26X_SD_MOTION_SCENE,
	H26X_SD_ZOOM_OUT_SCENE       = 0x00100000 | H26X_SD_MOTION_SCENE,
	H26X_SD_PAN_SCENE            = 0x01000000 | H26X_SD_MOTION_SCENE,
	H26X_SD_UNKNOWN_MOTION_SCENE = 0x10000000 | H26X_SD_MOTION_SCENE,
}H26X_SD_SCENE_TYPE;

typedef enum{
	H26X_SD_OPT_LAMBDA 		= 0x00000020,
	H26X_SD_OPT_MAQ 		= 0x00000800,
	H26X_SD_OPT_MAQ_DIFF 	= 0x01000000,
	H26X_SD_OPT_BGRDO 		= 0x08000000,
	H26X_SD_OPT_ALL			= 0x0FFFFFFF,
}H26X_SD_OPTIMIZED_FLAG;

#define IS_MOTION_SCENE(a)			(a & H26X_SD_MOTION_SCENE)
#define IS_STATIC_SCENE(a)			(a & H26X_SD_STATIC_SCENE)
#define IS_ZOOM_SCENE(a)			((a == H26X_SD_ZOOM_IN_SCENE) | (a == H26X_SD_ZOOM_OUT_SCENE))
#define IS_PAN_SCENE(a)				(a == H26X_SD_PAN_SCENE)
#define IS_UNKNOWN_MOTION_SCENE(a)	(a == H26X_SD_UNKNOWN_MOTION_SCENE)
#endif

#if H26X_SUPPORT_DYNAMIC_FR
static const UINT16 allSkipMDTHR_dyfr_tbl[16]        = {10, 9, 8, 7, 6, 5, 4, 3, 3, 3, 2, 2, 2, 2, 1, 1};
static const UINT16 fullFPSMDTHR_dyfr_tbl[16]        = {1000, 900, 800, 750, 650, 550, 450, 350, 250, 200, 150, 100, 75, 50, 25, 2};
static const UINT16 motion2StaticMDTHR_dyfr_tbl[16]  = {500, 400, 300, 250, 200, 170, 150, 100, 75, 50, 35, 25, 20, 10, 5, 1};
static const UINT16 static2MotionMDTHR_dyfr_tbl[16]  = {500, 400, 300, 250, 200, 170, 150, 75, 50, 40, 25, 20, 15, 10, 5, 1};
static const UINT16 motion2StaticCntTHR_dyfr_tbl[16] = {5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 15, 20, 25, 30, 30, 30};
static const UINT16 static2MotionCntTHR_dyfr_tbl[16] = {5, 4, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0};
#endif

#if H26X_SUPPORT_DYNAMIC_GOP
static const UINT16 motion2StaticMDTHR_dygop_tbl[16]  = {500, 400, 300, 250, 200, 170, 150, 120, 90, 75, 60, 45, 35, 25, 15, 5};
static const UINT16 static2MotionMDTHR_dygop_tbl[16]  = {500, 400, 300, 250, 200, 170, 150, 120, 90, 75, 60, 45, 35, 25, 15, 5};
static const UINT16 QP_FACTOR[16] = {100, 100, 100, 90, 80, 70, 60, 50, 40, 30, 20, 10, 0, 0, 0, 0};
static const UINT16 MOTION_SENSITIVE_FACTOR[16] = {8000, 7500, 7000, 6500, 6000, 5500, 5000, 4500, 4000, 3500, 3000, 2500, 2000, 1500, 1000, 500}; //can't be set zero
#endif

typedef struct _H26XEncAddr_{
	uintptr_t uiAPBAddr;
	uintptr_t uiNaluLen;
	uintptr_t uiBsOutAddr;
	uintptr_t uiBsOutAddr2;
	uintptr_t uiLLCAddr;
#ifdef VDOCDC_EMU
	UINT32 uiSideInfoSize;
	UINT32 uiSizeNaluLen;
#endif
	UINT32 uiLtrSavingBufBsSize; ///< 0: LTR need to mallocate REC buffer, > 0: LTR Saving Rec Mode, unit is byte
	uintptr_t uiDummyWTAddr;
	uintptr_t uiKPRefAPBAddr;
	uintptr_t uiKPRefBsAddr;
	UINT32 uiKPRefBsSize;

	UINT32 uiBsBufSize;
	UINT32 uiBsBufSize2;

	UINT32 bs_buf_size;
	UINT32 bs_buf_size2;
} H26XEncAddr;

typedef struct _H26XEncRCCfg_{
	UINT8 ucMaxIQp;			///< Rate Control's Max I Qp
    UINT8 ucMinIQp;			///< Rate Control's Min I Qp
    UINT8 ucMaxPQp;			///< Rate Control's Max P Qp
    UINT8 ucMinPQp;			///< Rate Control's Min P Qp
}H26XEncRcCfg;

typedef struct _H26XEncIspRatioCfg_{
	UINT8 ucRatioBse;
	UINT8 ucEdgeRatio;
	UINT8 ucDn2DRatio;
	UINT8 ucDn3DRatio;
}H26XEncIspRatioCfg;

typedef struct _H26XEncIspCbCfg_{
    void (*vdoenc_isp_cb)(UINT32 path_id, ULONG config);
    UINT32   id;
}H26XEncIspCbCfg;

typedef struct _H26XEncIspParamCfg_{
	UINT32 ISP_ISO;
	UINT32 ISP_EDGE_SUM;
	UINT32 ISP_MOTION_SUM;
	UINT32 ISP_RESERVED[5];
}H26XEncIspParamCfg;

typedef struct _H26XEncSignedVdoCfg_{
    BOOL signed_vdo_en;
    UINT32 machine_id[4];
} H26XEncSignedVdoCfg;

typedef struct _H26XEncSignedVdo_{
    H26XEncSignedVdoCfg stSVCfg;
    UINT32 sha_ini_value[8];
} H26XEncSignedVdo;

typedef struct _H26XCOMN_CTX_{
	H26XEncAddr		stVaAddr;
	H26XEncRC		stRc;
	H26XEncIspRatioCfg stIspRatio;
	H26XEncIspCbCfg stIspCbCfg;
	H26XEncIspParamCfg stIspParamCfg;
	H26XEncRCParam  stRcParam;//
	H26XEncSignedVdo  stSignedVdo;
	UINT32 uiWidth;
	UINT32 uiHeight;
	UINT32 uiPicCnt;
	UINT32 uiEncRatio;
	UINT32 uiLTRInterval;
	UINT8  ucSVCLayer;
	UINT8  ucRcMode;
	UINT32 uiLastHwInt;

	uintptr_t uipRcLogAddr;
	UINT32 uiRcLogLen;
	UINT32 uiPicHdrBufSize;

	BOOL bSliceLowLatencyEn;
	UINT32 uiSliceCurIdx;
	UINT32 uiLastSliceNumForSlioceLowLatency;

	UINT32 uiMotionSum;
	UINT32 uiFrmRate;
	UINT32 uiInitGOP;
	UINT32 uiNumOfMB;
	UINT8  ucLastIQP;
	UINT8  ucLastPQP;
	UINT8  bEVBRStillFlag;
	UINT8  ucLastMDMode;
	
	BOOL bGDPRSvnEn;
}H26XCOMN_CTX;

typedef struct _H26XDecAddr {
	UINT32 uiFrmBufNum;
	uintptr_t uiRefAndRecYAddr[FRM_IDX_MAX], uiRefAndRecUVAddr[FRM_IDX_MAX];
	INT32  iRefAndRecPOC[FRM_IDX_MAX];
	INT32  iColRefAndRecPOC[FRM_IDX_MAX];
	UINT32 uiRefAndRecIsIntra[FRM_IDX_MAX];
	UINT32 uiRefAndRecIsLT[FRM_IDX_MAX];
	UINT32 uiRef0Idx;
	UINT32 uiRecIdx;
	uintptr_t uiURIAddr;
	uintptr_t uiRecURAddr;
	uintptr_t uiIlfUpBotAddr;
	uintptr_t uiIlfSideInfoAddr[FRM_IDX_MAX];
	UINT32 uiIlfRIdx;
	UINT32 uiIlfWIdx;
	uintptr_t uiCMDBufAddr;
	uintptr_t uiHwBsAddr;

	UINT32 uiHwBSCmdNum;
	UINT32 uiHwBsCmdSize[H26X_MAX_BSDMA_NUM];

	uintptr_t uiColMvsAddr[FRM_IDX_MAX];
	UINT32 uiColMvRIdx;
	UINT32 uiColMvWIdx;

	uintptr_t uiResYAddr;
	uintptr_t uiResYAddr2;
	uintptr_t uiDummyWTAddr;
} H26XDecAddr;

typedef struct _H26XEncFrmCropInfo {
	BOOL frm_crop_enable;
	UINT16 display_w;
	UINT16 display_h;
} H26XEncFrmCropInfo;

#if H26X_MEM_USAGE

typedef enum{
	H26X_MEM_VDO_CTX = 0,
	H26X_MEM_FUNC_CTX,
	H26X_MEM_COMN_CTX,

	H26X_MEM_FRM_ST0,
	H26X_MEM_COLMV,
	H26X_MEM_EXTRA_ST0,
	H26X_MEM_SI_ST0,

	H26X_MEM_FRM_ST1,
	H26X_MEM_COLMV_ST1,
	H26X_MEM_EXTRA_ST1,
	H26X_MEM_SI_ST1,

	H26X_MEM_FRM_LT,
	H26X_MEM_COLMV_LT,
	H26X_MEM_EXTRA_LT,
	H26X_MEM_SI_LT,

	H26X_MEM_APB,
	H26X_MEM_LL,
	H26X_MEM_BSDMA,
	H26X_MEM_NALU_LEN,

	H26X_MEM_RC_REF,
	H26X_MEM_SEQ_HDR,
	H26X_MEM_PIC_HDR,

	H26X_MEM_USR_QP,
	H26X_MEM_MD,
	H26X_MEM_HIST,
	H26X_MEM_GCAC,

	H26X_MEM_USAGE_CNT
}H26X_MEM_BUF_CMD;

typedef struct _H26XBufCmd {
	uintptr_t st_adr;
	UINT32 size;
} H26XBufCmd;
typedef struct _H26XMemUsage {
	UINT32 cxt_size;
	H26XBufCmd buf_cmd[H26X_MEM_USAGE_CNT];
} H26XMemUsage;
#endif

extern int g_rc_dump_log;

UINT32 log2bin(UINT32 uiVal);

void save_to_reg(UINT32 *reg, INT32 val, UINT32 b_addr, UINT32 bits);
UINT32 get_from_reg(UINT32 reg, UINT32 b_addr, UINT32 bits);
UINT32 set_ll_cmd(H26X_LL_CMD cmd, int job_id, int offset, int burst);
void SetMemoryAddr(uintptr_t *puiVirAddr, uintptr_t *puiVirMemAddr, UINT32 uiSize);
UINT32 rbspToebsp(UINT8 *pucAddr, UINT8 *pucBuf, UINT32 uiSize);
UINT32 ebspTorbsp(UINT8 *pucAddr, UINT8 *pucBuf, UINT32 uiTotalBytes);
INT32 h26x_setLLCmd(UINT32 id, uintptr_t uiVaApbAddr, uintptr_t uiVaCurLLAddr, uintptr_t uiPaNxtLLAddr, UINT32 uiLLBufSize);
UINT8 bit_reverse(UINT8 pix);

#endif // _H26X_COMMON_H_
