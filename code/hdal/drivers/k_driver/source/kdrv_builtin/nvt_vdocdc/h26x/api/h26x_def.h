#ifndef _H26X_DEF_H_
#define _H26X_DEF_H_

#include "kwrap/type.h"

/*
0.0.01(2023/10/19): [NA51115-236] support embedded md
0.0.02(2023/10/25): add 265 tile mode and search range
0.0.03(2023/11/20): support slice encode
0.0.04(2023/12/04): sync up 530
0.0.05(2023/12/27): sync 530 isp-codec parameter interface
0.0.06(2024/01/15): default MAQ off (when input MD, active MAQ)
0.0.07(2024/01/17): default early skip off
0.0.08(2024/01/23): support dump reference info
0.0.09(2024/02/27): 1. add coef cost th proc, 2. support ip frame fro sync (H264/H265)
0.0.10(2024/03/12): update api of jnd
0.0.11(2024/04/01): add h264 RRC bias for AE_FIFO
0.0.12(2024/05/08): support suspend/resume
0.0.13(2024/05/31): support dygop/dyfr/scene det/smart roi
0.0.14(2024/06/04): connect dygop/dyfr (set from API) with RC and enable RCUpdate/RCPrepare in skip frame
0.0.15(2024/06/06): [NA51115-1050][IVOT_N12009_CO-1076] support privacy mask video
0.0.16(2024/07/02): fix ltr buffer saving mode
0.0.17(2024/07/31): update fro defualt parameters
0.1.00(2024/09/23): modify for 539
0.1.01(2024/10/11): [IVOT_N12191_CO-332] fix h265 bRecBufComm enable issue
0.1.02(2024/10/14): [IVOT_N12205_CO-116] support set min/max delta qp for mb min/max qp
0.1.03(2024/10/24): [IVOT_N12261_CO_66-220] update LPM setting
0.1.04(2024/10/28): [IVOT_N12191_CO-361] modify for GDR I slice
0.1.05(2024/11/01): 1.[IVOT_N12191_CO-400] add error handling. 2.[IVOT_N12191_CO-408] fixed free buffer.
0.1.06(2024/11/05): [IVOT_N12191_CO-416] add error handling when hw timeout.
0.1.07(2024/11/07): [IVOT_N12191_CO-361] add error handling for slice split.
0.1.08(2024/11/11): [IVOT_CHIP-30174] return re-trigger encode status.
0.1.09(2024/11/11): [NA51115-795] update ref_as_zero setting when encode error occurred
0.1.10(2024/11/11): [IVOT_CHIP-30581] modify clock settings
0.1.11(2024/11/12): [IVOT_CHIP-30176] Modify the settings for AXI burst read
0.1.12(2024/11/21): [IVOT_N12191_CO-446] fix multi slice + gdr issue
0.1.13(2024/11/25): [IVOT_N12191_CO-463] fix slice_i_idx bug
0.1.14(2024/11/25): [IVOT_N12191_CO-398] fix decode JPEG YUV422 Mistakenly activated SW dec
0.1.15(2024/11/29): [IVOT_N12191_CO-476] modify return picture type whle gdr enable
0.1.16(2024/12/10): [IVOT_N12191_CO-486] 1. 538 only uses the module reset. 2. Add error handling for bitstream parsing.
0.1.17(2024/12/20): [IVOT_CODEC-224] Modify the flow for controlling LPM through CODEC_MD.
0.1.17(2024/12/23): [IVOT_N12009_CO-1420] modify max width and height to 8192 for 539A
*/

#define NVT_H26XENC_VERSION     0x00011200

#ifdef CONFIG_NVT_SMALL_HDAL
	#define H26X_SAVE_KO_SIZE	(1)
#else
	#define H26X_SAVE_KO_SIZE	(0)
#endif
#define H26X_PROC_CMD       (1)
#define H26X_MIN_MB			(16)
//#define H26X_MAX_BSDMA_NUM	(H26X_MAX_HEIGHT/H26X_MIN_MB)
#ifndef VDOCDC_EMU
#define H26X_MAX_BSDMA_NUM  (15)//(0x2000)
#else
#define H26X_MAX_BSDMA_NUM  (0x2000)
#endif
#define H26X_MAX_ROI_W		(10)
#define GMV_CNT 			(16)
#define H26X_ENC_MODE		(1)	// 0 : link-list , 1 : direct mode
#define H26X_MAX_DATA_LEN	(1024)
#define H26X_MAX_DATA_SIZE	(1600)

#if !defined(VDOCDC_EMU) && !defined(VDOCDC_SIM)
#if H26X_SAVE_KO_SIZE
#define H264_P_REDUCE_16_PLANAR (0)
#else
#define H264_P_REDUCE_16_PLANAR (1)
#endif
#define H26X_USE_DIFF_MAQ       (1)
#else
#define H264_P_REDUCE_16_PLANAR (0)
#define H26X_USE_DIFF_MAQ       (0)
#endif
#define LPM_PROC_ENABLE			(1)
#define H26X_MEM_USAGE          (0)
#ifndef VDOCDC_EMU
#define H26X_SET_PROC_PARAM		(1)
#define H26X_MOTION_BITMAP_NUM  (10)
#else
#define H26X_SET_PROC_PARAM		(0)
#define H26X_MOTION_BITMAP_NUM  (3)
#endif
#define H26X_MAQ_MORPHOLOGY     (0)
#define H26X_MAQ_DIFF_SPECIFIC_FRM         (1)
#define H26X_SUPPORT_USER_DATA	(1)
#define H26X_UQPMAP_FROM_MD		(1)
#define H26X_DIS_OSG_CACHE_OPERATION	(1)
#define H26X_CHECK_MD_INVALID	(1)
#define H264_SET_COEF_COST_THD	(1)
#if !defined(VDOCDC_EMU) && !defined(VDOCDC_SIM)
#define H26X_SUPPORT_SCENE_DETECT	(1)		// scene detection
#define H26X_SUPPORT_SMART_ROI		(1)		// smart roi
#define H26X_SUPPORT_DYNAMIC_FR		(1)		// dynamic frame rate
#define H26X_SUPPORT_DYNAMIC_GOP	(1)		// dynamic gop
#define H264_AEFIFO_RRC_BIAS		(1)		// YCT, 264 RRC bias for AE_FIFO
#else
#define H26X_SUPPORT_SCENE_DETECT	(0)		// scene detection
#define H26X_SUPPORT_SMART_ROI		(0)		// smart roi
#define H26X_SUPPORT_DYNAMIC_FR		(0)		// dynamic frame rate
#define H26X_SUPPORT_DYNAMIC_GOP	(0)		// dynamic gop
#define H264_AEFIFO_RRC_BIAS		(0)		// YCT, 264 RRC bias for AE_FIFO
#endif
#define H26X_SET_MB_LIMIT_QP_IND	(1) 	// set mb qp by proc

#define H26XENC_BS_OUTBUF_MAX_SIZE 0x800000
#define H26X_MAX_W_WITHOUT_TILE_V36 (2048)
#define H26X_MAX_W_WITHOUT_TILE_V28 (2688)
#define H26X_MAX_TILE_NUM	    (2)
#define H26X_FIRST_TILE_MIN_W   (576)
#define H26X_MIDDLE_TILE_MIN_W  (576)
#define H26X_LAST_TILE_MIN_W    (448)

#define H26X_FIRST_TILE_MAX_W_SR36   (1024)
#define H26X_MIDDLE_TILE_MAX_W_SR36  (896)
#define H26X_LAST_TILE_MAX_W_SR36    (1024)
#define H26X_FIRST_TILE_MAX_W_SR28   (1408)
#define H26X_MIDDLE_TILE_MAX_W_SR28  (1280)
#define H26X_LAST_TILE_MAX_W_SR28    (1408)
#define H26X_MIN_W_TILE              (H26X_FIRST_TILE_MIN_W+H26X_LAST_TILE_MIN_W) // 1024
#define H26X_SR36_MAX_W              (H26X_FIRST_TILE_MAX_W_SR36+H26X_MIDDLE_TILE_MAX_W_SR36*3+H26X_LAST_TILE_MAX_W_SR36) // 4736
#define H26X_SR28_MAX_W              (H26X_FIRST_TILE_MAX_W_SR28+H26X_MIDDLE_TILE_MAX_W_SR28*3+H26X_LAST_TILE_MAX_W_SR28) // 6656

#define H26X_TILE_NUM_528		(4)
#define H26X_TILE_WIDTH_V52_528		(1536)

#define H26X_FIRST_TILE_MAX_W_SR52_528	(1408)
#define H26X_MIDDLE_TILE_MAX_W_SR52_528	(1280)
#define H26X_LAST_TILE_MAX_W_SR52_528	(1408)

#define H26X_SR52_MAX_W_528		(H26X_TILE_WIDTH_V28_528*H26X_TILE_NUM_528 - 256*(H26X_TILE_NUM_528-1))

#define H26X_MAX_W_WITHOUT_TILE_V36_528 (2176)
#define H26X_MAX_W_WITHOUT_TILE_V28_528 (2560)
#define H26X_MAX_W_WITHOUT_TILE_V20_528 (4096)

#define H26X_TILE_WIDTH_V52_560		(1280)

#define H26X_FIRST_TILE_MAX_W_SR52_560	(1152)
#define H26X_MIDDLE_TILE_MAX_W_SR52_560	(1024)
#define H26X_LAST_TILE_MAX_W_SR52_560	(1152)

#define H26X_MAX_W_WITHOUT_TILE_V52_560 (1408)
#define H26X_MAX_W_WITHOUT_TILE_V36_560 (2048)
#define H26X_MAX_W_WITHOUT_TILE_V28_560 (2688)
#define H26X_XVR_560E_528D_MAX_W_WITHOUT_TILE_V28_560 (2560)

#define H26X_MAX_W_WITHOUT_TILE_V52_538 (3200)
#define H26X_MAX_W_WITHOUT_TILE_V36_538 (4096)
#define H26X_MAX_W_WITHOUT_TILE_V28_538 (6144)

#define H26X_MAX_W_TILE_V52_538 (3072)
#define H26X_MAX_W_TILE_V36_538 (3968)
#define H26X_MAX_W_TILE_V28_538 (4096)


#ifndef VDOCDC_EMU
#define RRC_BY_FRAME_LEVEL		1
#else
#define RRC_BY_FRAME_LEVEL		0
#endif
#define MAX_RRC_FRAME_LEVEL		6

#define H26X_MAX_SRC_LST				(262144)
#define H26X_REDUCE_TILE_EXTRA_BUFFER  0 //0 : old method, need more buffer, 1: reduce extra buffer

#define H264E_GDRI_MIN_SLICE_HEIGHT		144
#define H265E_GDRI_MIN_SLICE_HEIGHT		192

#define SIZE_8192X(a)			((((a) + 8191)>>13)<<13)
#define SIZE_512X(a)			((((a) + 511)>>9)<<9)
#define SIZE_256X(a)			((((a) + 255)>>8)<<8)
#define SIZE_128X(a)			((((a) + 127)>>7)<<7)
#define SIZE_64X(a)				((((a) + 63)>>6)<<6)
#define SIZE_32X(a)				((((a) + 31)>>5)<<5)
#define SIZE_16X(a)				((((a) + 15)>>4)<<4)
#define SIZE_8X(a)				((((a) + 7)>>3)<<3)
#define SIZE_4X(a)				((((a) + 3)>>2)<<2)
#define SIZE_2X(a)				((((a) + 1)>>1)<<1)

#define ROUNDUP_8192X(a)		(((a) + 8191)>>13)
#define ROUNDUP_4096X(a)		(((a) + 4095)>>12)
#define ROUNDUP_2048X(a)		(((a) + 2047)>>11)
#define ROUNDUP_1024X(a)		(((a) + 1023)>>10)
#define ROUNDUP_512X(a)			(((a) + 511)>>9)
#define ROUNDUP_256X(a)			(((a) + 255)>>8)
#define ROUNDUP_128X(a)			(((a) + 127)>>7)
#define ROUNDUP_64X(a)			(((a) + 63)>>6)
#define ROUNDUP_32X(a)			(((a) + 31)>>5)
#define ROUNDUP_16X(a)			(((a) + 15)>>4)
#define ROUNDUP_8X(a)			(((a) + 7)>>3)
#define ROUNDUP_4X(a)			(((a) + 3)>>2)
#define ROUNDUP_2X(a)			(((a) + 1)>>1)

#define FLOOR_512X(a)			(((a)>>9)<<9)
#define FLOOR_256X(a)			(((a)>>8)<<8)
#define FLOOR_128X(a)			(((a)>>7)<<7)
#define FLOOR_64X(a)			(((a)>>6)<<6)
#define FLOOR_32X(a)			(((a)>>5)<<5)
#define FLOOR_16X(a)			(((a)>>4)<<4)
#define FLOOR_8X(a)				(((a)>>3)<<3)
#define FLOOR_4X(a)				(((a)>>2)<<2)
#define FLOOR_2X(a)				(((a)>>1)<<1)

#define H26X_SWAP(A,B,TT) {TT TMP; TMP=A; A=B; B=TMP;}
#define H26X_MAX(A, B) ((A > B) ? A : B)
#define H26X_MIN(A, B) ((A < B) ? A : B)
#define H26X_CLIP(LOW, HIGH, VAL)	((VAL)>(HIGH)?(HIGH):((VAL)<(LOW)?(LOW):(VAL)))

#define IS_ISLICE(X) (X == I_SLICE || X == IDR_SLICE)
#define IS_PSLICE(X) (X == P_SLICE)
#define IS_IDRSLICE(X) (X == IDR_SLICE)

#if H26X_SUPPORT_SCENE_DETECT
#define GMV_TEMPORAL_BUFFER_SIZE 4
#define GMD_TEMPORAL_BUFFER_SIZE 8
#define H26X_PAN_VAR			 4
#define H26X_PAN_STR			 10
#endif

typedef enum{
	H26XENC_FAIL = 0,
	H26XENC_SUCCESS,
}H26XEncStatus;

typedef enum{
	VCODEC_H265 = 0,
	VCODEC_H264 = 1,
	VCODEC_NON_DEFINE
}CODEC_TYPE;

typedef enum{
    P_SLICE = 0,
    B_SLICE = 1,
    I_SLICE = 2,
    IDR_SLICE = 3,
    UNSET_SLICE_TYPE = 0xFF,
    ENUM_DUMMY4WORD(SLICE_TYPE)
}SLICE_TYPE;

typedef enum {
	CU16_QP_DEFAULT = 0,
	CU16_QP_DISABLE_AQVAR = 1,
	CU16_QP_DISABLE_AQ = 2,
	CU16_QP_FIXEDQP = 3,
	ENUM_DUMMY4WORD(CU16_QP_MODE)
} CU16_QP_MODE;

typedef enum {
	QUALITY_MAIN = 0,	///< set tile config and search range as original 			(srv76)
	QUALITY_BASE,		///< set tile config and search range as saving memory mode	(srv36)
	QUALITY_HIGH,		///< set tile config and search range to high				(srv100)
} QLVL_TYPE;

typedef struct _H26XENC_VAR_{
	CODEC_TYPE eCodecType;

	UINT32 uiEncId;
	UINT32 uiChipIdx;
	UINT32 uiCtxSize;

	//UINT32 uiAPBAddr;
	uintptr_t uiReportAddr[4];

	void   *pVdoCtx;
	void   *pFuncCtx;
	void   *pComnCtx;
} H26XENC_VAR;

typedef struct _H26XDEC_VUI_{
	BOOL   bPresentFlag;
	UINT32 uiSarWidth;
	UINT32 uiSarHeight;
	UINT8  ucMatrixCoeff;
	UINT8  ucTransChar;
	UINT8  ucColorPrimaries;
	UINT8  ucVideoFmt;
	UINT8  ucColorRange;
	BOOL   bTimingPresentFlag;
} H26XDEC_VUI;

typedef struct _H26XDEC_VAR_{
	CODEC_TYPE eCodecType;

	UINT32 uiDecId;
	UINT32 uiChipIdx;

	uintptr_t uiAPBAddr;
	UINT32 uiCtxSize;
	H26XDEC_VUI stVUI;

    void   *pContext;
} H26XDEC_VAR;

#endif

