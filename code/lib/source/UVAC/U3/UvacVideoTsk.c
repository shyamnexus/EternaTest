/*
    Copyright   Novatek Microelectronics Corp. 2004.  All rights reserved.

    @file       UvacVideoTsk.c
    @ingroup    mISYSUVAC

    @brief      UVAC Video Task.
                UVAC Video Task.

*/
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <string.h>
//#include "timer.h"
#include "UvacVideoTsk.h"
#include "UvacVideoTsk_api.h"
#include "UvacReqClass.h"
#include "UVAC.h"
#include "UvacDesc.h"
#include "UvacDbg.h"
//#include "Utility.h"
#include "UvacIsoInTsk.h"
#include "UvacID.h"
//#include "hd_gfx.h"
#include "hd_common.h"
#include "uvcp.h"
#include "UvacReqVendor.h"
//#include "dma_protected.h"
//#include "cache_protected.h"
#if 0
#include "UvacSIDCCmd.h"
#include "UvacPTP_DataTransfer.h"
#include "UvacPTP_CreateObjectList.h"
#include "UvacPTPDef.h"
#include "UvacMTP_Operation.h"
#include "UvacPTP_Operation.h"
#include "UvacPTP_Operation.h"
#include "FileSysTsk.h"
#include "UvacSIDCUtility.h"
#endif

#define MAX_FRAME_SIZE_MARGIN   15

#define UVC_MAKE_PAYLOAD_PERF 0

#define UVAC_BUF_ALIGN  64

#define UVCP_FUNC 1

#if defined(__LINUX_USER__)
#define SUSPEND_RESUME_FUNC 0
#else
#define SUSPEND_RESUME_FUNC 1
#endif

BOOL gUvacMtpEnabled = FALSE;
#if 0
SIDC_INTRCMD g_UvcSidcIntrCmd;
UINT32      guiUvcSidcBufAddr, guiUvcSidcBufSize;
UINT32      g_uiUvcSidcPTPUpdateFWAddr, g_uiUvcSidcPTPUpdateFWSize;
UINT16      gUvcSidcCardLock = 0;
#endif
#if 0
extern UVAC_USIDC_GET_CARDTYPE_FUNC gUvacSIDCGetCardTypeFunc;
#endif

typedef enum {
	UVAC_BUF_TYPE_WORK = 0,
	UVAC_BUF_TYPE_VID,
	UVAC_BUF_TYPE_VID2,
	UVAC_BUF_TYPE_VID3,
} UVAC_BUF_TYPE;

typedef struct _MEMRANGE {
	uintptr_t  va;
	uintptr_t  pa;
	UINT32  size;
} MEMRANGE, *PMEMRANGE;

static BOOL uvc_direct_trigger = TRUE;
//------ Global variable declare ------//
UINT32 gUacAudStart[UVAC_AUD_DEV_CNT_MAX];
UINT32 gUvcVidStart[UVAC_VID_DEV_CNT_MAX];
FLGPTN gU3UvacRunningFlag[UVAC_VID_DEV_CNT_MAX] = {0};
//If an endpoint is set to hw-payload, then it also be auto-hdr-eof.
UINT32 gUvcHWPayload[UVAC_VID_DEV_CNT_MAX] = {FALSE};
static BOOL   gUvacOpened = FALSE;
UVAC_VEND_DEV_DESC *gpImgUnitUvacVendDevDesc = 0;
UVAC_VID_STRM_INFO gUvacVidStrmInfo[UVAC_VID_DEV_CNT_MAX];
UINT32 gUvacChannel = UVAC_CHANNEL_1V1A;//UVAC_CHANNEL_1V1A;
UINT32 gHsUvacChannel = 0;
UINT32 gUvcCapM3Enable = FALSE;
UINT32 gUvcH264TBR = UVAC_H264_TBR_DEF;
UINT32 gUvcMJPGTBR = UVAC_MJPG_TBR_DEF;
UINT32 gUvcStrmMinBitRate = 0x02000000;  //32M  bps =  4M  byte
UINT32 gUvcStrmMaxBitRate = 0x20000000;  //512 bps = 64M byte
UINT32 gUvcMJPGMaxTBR = UVAC_MJPG_TBR_MAX;
static uintptr_t guiUvacBufAddr, guiUvacBufAddr_pa;
static UINT32 guiUvacBufTotalSize;
static MEMRANGE gUvcVidBuf[UVAC_VID_DEV_CNT_MAX] = {0};
UINT8  *gpUvacHSConfigDesc;
#if (UVAC_SUPPORT_SUPER_SPEED == ENABLE)
UINT8  *gpUvacSSConfigDesc;
#endif
#if 0
TIMER_ID gUvacTimerID[UVAC_VID_DEV_CNT_MAX] = {TIMER_NUM};
#endif
static UINT32 user_data_size = 0;
UVAC_VIDEO_FORMAT gUvacCodecType[UVAC_VID_DEV_CNT_MAX];
UINT32  gUVCHeadPTS = 0x200000;//0x61D4EB38;
UINT32  gUVCHeadSCR = 0x3AB000;//0x63EC62E8;
UINT16  gSOFTokenNum = 0x0001;
UINT32  gUVCIsoinTxfUnitSize[UVAC_VID_DEV_CNT_MAX] = {0};  //must be > PAYLOAD_LEN
BOOL gUvacWinIntrfEnable = FALSE;
UINT32 gUvcTrigEnabled = 0;
UINT8 gUvacAudFmtType = UAC_FMT_TYPE_I;
UVAC_VIDEO_FORMAT_TYPE gUvcVideoFmtType[UVAC_VID_DEV_CNT_MAX] = {UVAC_VIDEO_FORMAT_H264_MJPEG, 0xFF, 0xFF};
BOOL gUvacCdcEnabled[CDC_COM_MAX_NUM] = {0};
UVAC_CDC_PSTN_REQUEST_CB gfpCdcPstnReqCB = NULL;
UINT32 gUvcXUCtrl = UVC_XU_BMCONTROLS;
static UVAC_STARTVIDEOCB  g_fpStartVideo = NULL;
//static UVAC_GETH264HEADERCB  g_fpGetH264Header = NULL;
static UVAC_STOPVIDEOCB   g_fpStopVideo = NULL;
static UVAC_STARTAUDIOCB  g_fpStartAudio = NULL;
#if 0
static UVAC_GETSTRMCB     g_fpGetStrmCB = NULL;
#endif
static UINT32 gToggleFid[UVAC_VID_DEV_CNT_MAX] = {0};
static MEM_RANGE gVidTxBuf[UVAC_VID_DEV_CNT_MAX] = {0};
//static MEM_RANGE gH264Header[UVAC_VID_DEV_CNT_MAX] = {0};
static UVAC_STRM_INFO  gStrmInfo[UVAC_VID_DEV_CNT_MAX] = {0};
static UVAC_STRM_INFO  gAudStrmInfo[UVAC_AUD_DEV_CNT_MAX] = {0};
static UINT32 gUvcMaxVideoFmtSize = UVAC_MAX_PAYLOAD_FRAME_SIZE;
UINT32 gUacChNum = UAC_NUM_PHYSICAL_CHANNEL;
UINT32 gUacRxChNum = UAC_NUM_PHYSICAL_CHANNEL;
UINT32 gUacITOutChCfg = UAC_IT_OUT_CHANNEL_CONFIG;
UINT32 gUacRxITOutChCfg = UAC_IT_OUT_CHANNEL_CONFIG;
UVAC_VID_RESO_ARY gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_MAX] = {0};
UVAC_VID_RESO_ARY gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_MAX] = {0, NULL, UVC_VSFMT_DEF_FRAMEINDEX};
UVAC_VID_RESO_ARY gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_MAX] = {0, NULL, UVC_VSFMT_DEF_FRAMEINDEX};
UVAC_VID_RESO_ARY gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_MAX] = {0};
UVAC_VID_RESO_ARY gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_MAX] = {0, NULL, UVC_VSFMT_DEF_FRAMEINDEX};

UVAC_VID_RESO_ARY gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_MAX] = {0};
UVAC_VID_RESO_ARY gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_MAX] = {0, NULL, UVC_VSFMT_DEF_FRAMEINDEX};
UVAC_VID_RESO_ARY gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_MAX] = {0, NULL, UVC_VSFMT_DEF_FRAMEINDEX};
UVAC_VID_RESO_ARY gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_MAX] = {0};
UVAC_VID_RESO_ARY gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_MAX] = {0, NULL, UVC_VSFMT_DEF_FRAMEINDEX};

UINT32 gU3UvcNv12Payload[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 gU3HsUvcNv12Payload[UVAC_VID_DEV_CNT_MAX] = {0};

UINT32 u3_uvc_mjpg_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_h264_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_yuv_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_nv12_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_h265_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};

UINT32 u3_uvc_ss_mjpg_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_ss_h264_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_ss_yuv_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_ss_nv12_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_ss_h265_max_tx_size_cnt[UVAC_VID_DEV_CNT_MAX] = {0};

UINT32 *p_u3_uvc_mjpg_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 *p_u3_uvc_h264_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 *p_u3_uvc_yuv_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 *p_u3_uvc_nv12_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 *p_u3_uvc_h265_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};

UINT32 u3_uvc_mjpg_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_h264_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_yuv_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_nv12_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_h265_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};

UINT32 *p_u3_uvc_ss_mjpg_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 *p_u3_uvc_ss_h264_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 *p_u3_uvc_ss_yuv_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 *p_u3_uvc_ss_nv12_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 *p_u3_uvc_ss_h265_max_tx_size[UVAC_VID_DEV_CNT_MAX] = {0};

UINT32 u3_uvc_ss_mjpg_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_ss_h264_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_ss_yuv_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_ss_nv12_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};
UINT32 u3_uvc_ss_h265_tx_base_size[UVAC_VID_DEV_CNT_MAX] = {0};

BOOL u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_MAX] = {FALSE, FALSE, FALSE};
BOOL u3_uvc_bulk_multi_payload = TRUE;
UINT32 u3_uvc_bulk_payload_size = UVC_BULK_PAYLOAD_SIZE;
UINT32 u3_uvc_hs_bulk_payload_size = UVC_BULK_PAYLOAD_SIZE;

USB_EP_BLKNUM gU3UvcIsoInHsBandWidth[UVAC_VID_EP_PACK_SIZE_MAX_CNT] = {BLKNUM_SINGLE, BLKNUM_SINGLE, BLKNUM_DOUBLE, BLKNUM_TRIPLE};
UINT32 gU3UvcIsoInHsPacketSize[UVAC_VID_EP_PACK_SIZE_MAX_CNT] = {UVC_USB_FIFO_UNIT_SIZE, UVC_ISOIN_HS_PACKET_SIZE, UVC_ISOIN_HS_PACKET_SIZE, UVC_ISOIN_HS_PACKET_SIZE};

UINT32 gU3UvcIsoInSsPacketSize[UVAC_VID_EP_PACK_SIZE_MAX_CNT] = {UVC_ISOIN_HS_PACKET_SIZE, UVC_ISOIN_HS_PACKET_SIZE, UVC_ISOIN_HS_PACKET_SIZE, UVC_ISOIN_HS_PACKET_SIZE};
UINT32 gU3UvcIsoInSsMaxBurst[UVAC_VID_EP_PACK_SIZE_MAX_CNT] = {0, 2, 7, 15};
UINT32 gU3UvcIsoInSsEpcAttr[UVAC_VID_EP_PACK_SIZE_MAX_CNT] = {0, 0, 1, 1};
USB_EP_BLKNUM gU3UvcIsoInSsBandWidth[UVAC_VID_EP_PACK_SIZE_MAX_CNT] =
{
	(0+1)*(0+1),
	(2+1)*(0+1),
	(7+1)*(1+1),
	(15+1)*(1+1)
};

UVAC_HID_INFO g_u3_hid_info = {0};
extern UVAC_EU_DESC u3_eu_desc[2];
UVAC_MSOS20_INFO g_u3_msos_info = {0};

UVAC_EU_DESC_ARRAY* u3_eu_desc_array = NULL;

UVAC_MSDC_INFO g_u3_msdc_info = {0};
USB3_GENERIC_CB g_u3uvac_msdc_device_evt_cb = NULL;
USB3_GENERIC_CB g_u3uvac_msdc_class_req_cb = NULL;

UINT8 u3_hid_string_idx = 0;

BOOL gU3DisableUac = FALSE;
UINT32 gU3HsDisableUac = 0xFF;
UINT16 gU3UacMaxPacketSize = 0;
UINT8 gU3UacInterval = 0;
static BOOL g_u3_uvcp_en = TRUE;

U3DEV_SPEED g_usb_speed = 0;

extern UVAC_WINUSBCLSREQCB gUvcWinUSBReqCB;
extern UINT8 gUvacIntfIdx_WinUsb;
extern UINT8 gUvacIntfIdx_VC[UVAC_VID_DEV_CNT_MAX];
extern UINT8 gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_MAX];
extern UINT8 gUvacIntfIdx_AC[UVAC_AUD_DEV_CNT_MAX];
extern UINT8 gUvacIntfIdx_AS[UVAC_AUD_DEV_CNT_MAX];
extern UINT8 gUvacIntfIdx_vidAryCurrIdx; //one video control map single video stream
extern UINT8 gUvacIntfIdx_audAryCurrIdx; //one audio control map single audio stream
extern UVAC_SETVOLCB gUacSetVolCB;
extern UVAC_SETVOLCB gUacRxSetVolCB;
extern UVAC_UNIT_CB g_fpU3UvcCT_CB;
extern UVAC_UNIT_CB g_fpU3UvcPU_CB;
extern UVAC_UNIT_CB g_fpU3UvcXU_CB;
extern INT32 u3_uac_vol_cur;
extern INT32 u3_uac_vol_max;
extern INT32 u3_uac_vol_min;
extern INT32 u3_uac_vol_res;
extern UAC_VOL_INFO u3_uac_rx_vol_info;
extern UINT32 gU3UvacUvcVer;
extern UINT32 gUvacTxfQueCurCnt[UVAC_TXF_QUE_MAX];
extern UINT32 set_probe_count_max;

extern USB_EP msdc_in_ep;
extern USB_EP msdc_out_ep;

#define FREE_STRING_INDEX_BEGIN 4
UINT8 u3_free_string_idx = FREE_STRING_INDEX_BEGIN;
UINT8 u3_uvc_string_idx[UVAC_VID_DEV_CNT_MAX] = {0};
UINT8 u3_uac_string_idx[UVAC_AUD_DEV_CNT_MAX] = {0};
UINT8 u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_MAX] = {0};
UINT8 u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_MAX] = {0};
USB_STRING_DESC *p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_MAX] = {NULL};
USB_STRING_DESC *p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_MAX] = {NULL};
USB_STRING_DESC *p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_MAX] = {NULL};
USB_STRING_DESC *p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_MAX] = {NULL};


static VOS_TICK last_set_each[UVAC_TXF_QUE_MAX] = {0};

#if SUSPEND_RESUME_FUNC
static UVAC_POWERDOWN_SUSPEND_CB g_fpSuspend_CB = NULL;
#endif

////////////////////////////////////////////////////////////////////////////////
//UAC out
static MEMRANGE gAudRxBuf[1] = {0};
BOOL gUvacUacRxEnabled = FALSE;
static UINT32 gUvacUacBlockSize = 4096;
static UINT32 gUvacUacBufSize = UVAC_AUD_RX_BUF_SIZE;
static UVAC_AUD_RAWQ uac_raw_que = {0};

void _UVAC_init_queue(void);
BOOL _UVAC_GetRaw(uintptr_t *addr, UINT32 size);
void _UVAC_unlock_pullqueue(UVAC_STRM_PATH path);
BOOL _UVAC_put_pullque(uintptr_t addr, UINT32 size);
BOOL _UVAC_release_all_pullque(UVAC_STRM_PATH path);
////////////////////////////////////////////////////////////////////////////////
//Discrete fps: fps must be in a decreasing order and 0 shall be put to the last one.
#if 0
UINT32 gUvcVidResoCnt = UVC_VSFMT_FRAMENUM1_HD;
UVAC_VID_RESO gUvcVidResoAry[UVAC_VID_RESO_MAX_CNT] = {
	{USB_UVC_HD_WIDTH, USB_UVC_HD_HEIGHT, 3, UVC_FRMRATE_20, UVC_FRMRATE_10, UVC_FRMRATE_5 },
	{USB_UVC_1024x768_WIDTH, USB_UVC_1024x768_HEIGHT, 3, UVC_FRMRATE_30, UVC_FRMRATE_15, UVC_FRMRATE_8 },
	{USB_UVC_VGA_WIDTH, USB_UVC_VGA_HEIGHT, 3, UVC_FRMRATE_30, UVC_FRMRATE_20, UVC_FRMRATE_10},
	{USB_UVC_QVGA_WIDTH, USB_UVC_QVGA_HEIGHT, 3, UVC_FRMRATE_30, UVC_FRMRATE_25, UVC_FRMRATE_12},
	{USB_UVC_1024x576_WIDTH, USB_UVC_1024x576_HEIGHT, 3, UVC_FRMRATE_60, UVC_FRMRATE_30, UVC_FRMRATE_15},
	{USB_UVC_800x600_WIDTH, USB_UVC_800x600_HEIGHT, 3, UVC_FRMRATE_60, UVC_FRMRATE_30, UVC_FRMRATE_20},
	{USB_UVC_800x480_WIDTH, USB_UVC_800x480_HEIGHT, 2, UVC_FRMRATE_60, UVC_FRMRATE_25, 0},
	{USB_UVC_FULL_HD_WIDTH, USB_UVC_FULL_HD_HEIGHT, 2, UVC_FRMRATE_60, UVC_FRMRATE_30, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0}
};
#else
UINT32 gUvcVidResoCnt = UVAC_VID_RESO_INTERNAL_TBL_CNT;
UVAC_VID_RESO gUvcVidResoAry[UVAC_VID_RESO_MAX_CNT] = {
	{1920,  1080,   1,      UVC_FRMRATE_30,      0,      0},
	{1280,   720,   1,      UVC_FRMRATE_30,      0,      0},
	{ 848,   480,   1,      UVC_FRMRATE_30,      0,      0},
	{ 640,   480,   1,      UVC_FRMRATE_30,      0,      0},
	{ 320,   240,   1,      UVC_FRMRATE_30,      0,      0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0}
};
#endif
UVAC_PARAM gUvcParamAry[UVAC_VID_DEV_CNT_MAX] = {
	{UVC_VSFMT_DEF_FRM_IDX, UVC_FRMRATE_30},
	{UVC_VSFMT_DEF_FRM_IDX, UVC_FRMRATE_30}
};
UINT32 gUvacAudSampleRate[UVAC_AUD_SAMPLE_RATE_MAX_CNT] = {
	UAC_FREQUENCY_16K,
	0,
	0,
};

UINT32 gUvacAudRxSampleRate[UVAC_AUD_RX_SAMPLE_RATE_MAX_CNT] = {
	UAC_FREQUENCY_16K,
	0,
	0,
};

UINT32 u3_ct_controls = UVC_IT_CONTROLS;
UINT32 u3_pu_controls = UVC_PU_CONTROLS;

//====== extern variable ======
extern UINT32 gUacSampleRate;
extern UINT32 gUacRxSampleRate;
extern UINT32 gUvcMaxTxfSizeForUsb[UVAC_TXF_QUE_MAX];
extern const USB_EP UVAC_USB_EP[UVAC_TXF_QUE_MAX];
extern const USB_EP UVAC_USB_RX_EP[UVAC_RXF_QUE_MAX];
extern UINT32 gUvcUsbDMAAbord;
extern UINT32 gUacUsbDMAAbord;
extern UINT32 gUvcNoVidStrm;
extern UVAC_EUVENDCMDCB gUvacEUVendCmdCB[UVAC_EU_VENDCMD_CNT];

UINT32 gUvcCapImgAddr = 0;
UINT32 gUvcCapImgSize = 0;

extern UVC_STILL_PROBE_COMMIT gUvcStillProbeCommit;
extern UINT8 gUvacStillImgTrigSts;

static void uvac_wai_sem(const char * calledby) {
	//printf("wait_sem called by %s\r\n", calledby);
	wai_sem(SEMID_UVC_QUEUE);
}

static void uvac_rel_sem(const char * calledby) {
	//printf("rel_sem called by %s\r\n", calledby);
	sig_sem(SEMID_UVC_QUEUE);
}

#if (ISF_AUDIO_LATENCY_DEBUG)
extern void UVAC_DbgDmp_Aud_TimestampNumReset(void);
#endif

#define loc_cpu() uvac_wai_sem(__FUNCTION__)
#define unl_cpu() uvac_rel_sem(__FUNCTION__)

/**
    \addtogroup mISYSUVAC
@{
*/

extern UINT32 guiVideoBufAddr, FirstSize;



static uintptr_t uvac_va_to_pa(UVAC_BUF_TYPE type, uintptr_t va)
{
	uintptr_t pa = 0;
	if (type == UVAC_BUF_TYPE_WORK) {
		if (va >= guiUvacBufAddr) {
			pa = guiUvacBufAddr_pa + (va - guiUvacBufAddr);
			if (pa <= (guiUvacBufAddr_pa + guiUvacBufTotalSize)) {
				//DBG_DUMP("work va=0x%X -> pa=0x%X", va, pa);
				return pa;
			}
		}
	} else if (type == UVAC_BUF_TYPE_VID){
		if (gUvcVidBuf[0].pa == 0) {
			return va;
		} else if (va >= gUvcVidBuf[0].va) {
			pa = gUvcVidBuf[0].pa + (va - gUvcVidBuf[0].va);
			if (pa <= (gUvcVidBuf[0].pa + gUvcVidBuf[0].size)) {
				//DBG_DUMP("vid1 va=0x%X -> pa=0x%X", va, pa);
				return pa;
			}
		}
	} else {
		if (gUvcVidBuf[1].pa == 0) {
			return va;
		} else if (va >= gUvcVidBuf[1].va) {
			pa = gUvcVidBuf[1].pa + (va - gUvcVidBuf[1].va);
			if (pa <= (gUvcVidBuf[1].pa + gUvcVidBuf[1].size)) {
				//DBG_DUMP("vid2 va=0x%X -> pa=0x%X", va, pa);
				return pa;
			}
		}
	}
	DBG_ERR("invalid value type=%d, va=0x%lX, pa=0x%lX\r\n", type, va, pa);
	return pa;
}
static uintptr_t uvac_pa_to_va(UVAC_BUF_TYPE type, uintptr_t pa)
{
	uintptr_t va = 0;
	if (type == UVAC_BUF_TYPE_WORK) {
		if (pa >= guiUvacBufAddr_pa) {
			va = guiUvacBufAddr + (pa - guiUvacBufAddr_pa);
			if (va <= (guiUvacBufAddr + guiUvacBufTotalSize)) {
				//DBG_DUMP("work pa=0x%X -> va=0x%X", pa, va);
				return va;
			}
		}
	} else if (type == UVAC_BUF_TYPE_VID){
		if (gUvcVidBuf[0].va == 0) {
			return pa;
		} else if (pa >= gUvcVidBuf[0].pa) {
			va = gUvcVidBuf[0].va + (pa - gUvcVidBuf[0].pa);
			if (va <= (gUvcVidBuf[0].va + gUvcVidBuf[0].size)) {
				//DBG_DUMP("vid1 pa=0x%X -> va=0x%X", pa, va);
				return va;
			}
		}
	} else {
		if (gUvcVidBuf[1].va == 0) {
			return pa;
		} else if (pa >= gUvcVidBuf[1].pa) {
			va = gUvcVidBuf[1].va + (pa - gUvcVidBuf[1].pa);
			if (va <= (gUvcVidBuf[1].va + gUvcVidBuf[1].size)) {
				//DBG_DUMP("vid2 pa=0x%X -> va=0x%X", pa, va);
				return va;
			}
		}
	}
	DBG_ERR("invalid value type=%d, va=0x%lX, pa=0x%lX\r\n", type, va, pa);
	return va;
}
#if 0
static ER hwmem_open(void)
{
	return E_OK;
}

static ER hwmem_close(void)
{
	return E_OK;
}

static void hwmem_memcpy(UVAC_VID_DEV_CNT  thisVidIdx, uintptr_t uiDst, uintptr_t uiSrc, UINT32 uiSize)
{
	memcpy((void *)uiDst, (void *)uiSrc, uiSize);
	hwmem_memflush((void *) uiDst, uiSize);
}
#endif

static void hwmem_memflush(void* uiDst, UINT32 uiSize)
{
	hd_common_mem_flush_cache(uiDst, uiSize);
}

static void *hwmem_mem_mmap(HD_COMMON_MEM_MEM_TYPE mem_type, UINTPTR phy_addr, UINT32 size)
{
	#if 1//defined(__LINUX_USER__)
	return hd_common_mem_mmap(mem_type, phy_addr, size);
	#else
	return (void *)phy_addr;
	#endif
}

static HD_RESULT hwmem_mem_munmap(void *virt_addr, unsigned int size)
{
	#if 1//defined(__LINUX_USER__)
	return hd_common_mem_munmap(virt_addr, size);
	#else
	return HD_OK;
	#endif
}
//====== debug ======
#if (_UVC_DBG_LVL_ == _UVC_DBG_ALL_)
static void UVAC_DbgDmp_StrDesc(UVAC_STRING_DESC *pStrDesc)
{
	UINT32 len = 0;
	UINT8 *ptr = 0;
	if (pStrDesc) {
		len = pStrDesc->bLength;
		len = (len > 2) ? (len - 2) : 0;
		DBG_DUMP("\r\nStrDesc: 0x%x, 0x%x\r\n", len, pStrDesc->bDescriptorType);
		ptr = pStrDesc->bString;
		while (len--) {
			DBG_DUMP("  %c", *ptr++);
		}
	} else {
		DBG_DUMP("\r\nStrDesc NULL\r\n");
	}
	DBG_DUMP("\r\n");
}
static void UVAC_DbgDmp_VendDevDesc(PUVAC_VEND_DEV_DESC pVendDevDesc)
{
	DBG_DUMP("%s ==>\r\n", __func__);
	if (0 == pVendDevDesc) {
		DBG_ERR("Input NULL\r\n");
		return;
	}
	DBG_DUMP("VID=0x%x, PID=0x%x\r\n", pVendDevDesc->VID, pVendDevDesc->PID);
	UVAC_DbgDmp_StrDesc(pVendDevDesc->pManuStringDesc);
	UVAC_DbgDmp_StrDesc(pVendDevDesc->pProdStringDesc);
	UVAC_DbgDmp_StrDesc(pVendDevDesc->pSerialStringDesc);
	DBG_DUMP("\r\nfpIQVendReqCB         =0x%x\r\n", pVendDevDesc->fpIQVendReqCB);
	DBG_DUMP("fpVendReqCB           =0x%x\r\n", pVendDevDesc->fpVendReqCB);
	DBG_DUMP("<===========\r\n");
}
static void UVAC_DbgDmp_EU_INFO(UVAC_EU_DESC *p_eu_desc)
{
	UINT32 i;
	DBG_DUMP("== Extension Unit Descriptor ==\r\n");
	DBG_DUMP("bLength       =%d\r\n", p_eu_desc->bLength);
	DBG_DUMP("bDescriptorType       =0x%X\r\n", p_eu_desc->bDescriptorType);
	DBG_DUMP("bDescriptorSubtype       =0x%X\r\n", p_eu_desc->bDescriptorSubtype);
	DBG_DUMP("bUnitID       =%d\r\n", p_eu_desc->bUnitID);

	DBG_DUMP("GUID=");
	for (i = 0; i < 16; i++) {
		DBG_DUMP("%X ", p_eu_desc->guidExtensionCode[i]);
	}
	DBG_DUMP("\r\n");

	DBG_DUMP("bNumControls       =%d\r\n", p_eu_desc->bNumControls);
	DBG_DUMP("bNrInPins       =%d\r\n", p_eu_desc->bNrInPins);

	DBG_DUMP("baSourceID=");
	for (i = 0; i < p_eu_desc->bNrInPins; i++) {
		DBG_DUMP("%d ", p_eu_desc->baSourceID[i]);
	}
	DBG_DUMP("\r\n");

	DBG_DUMP("bControlSize       =%d\r\n", p_eu_desc->bControlSize);

	DBG_DUMP("bmControls=");
	for (i = 0; i < p_eu_desc->bControlSize; i++) {
		DBG_DUMP("%d ", p_eu_desc->bmControls[i]);
	}
	DBG_DUMP("\r\n");
	DBG_DUMP("iExtension       =%d\r\n", p_eu_desc->iExtension);
}
static void UVAC_DbgDmp_HID_INFO(UVAC_HID_INFO *p_hid_info)
{
	DBG_DUMP("--== HID info ==--\r\n");
	DBG_DUMP("en=%d, cb=0x%lX\r\n", p_hid_info->en, (uintptr_t)p_hid_info->cb);
	DBG_DUMP("bLength =%d\r\n", p_hid_info->hid_desc.bLength);
	DBG_DUMP("bHidDescType  =0x%X\r\n", p_hid_info->hid_desc.bHidDescType);
	DBG_DUMP("bcdHID  =0x%X\r\n", p_hid_info->hid_desc.bcdHID);
	DBG_DUMP("bCountryCode  =0x%X\r\n", p_hid_info->hid_desc.bCountryCode);
	DBG_DUMP("bNumDescriptors  =%d\r\n", p_hid_info->hid_desc.bNumDescriptors);
	DBG_DUMP("bDescriptorType  =0x%X\r\n", p_hid_info->hid_desc.bDescriptorType);
	DBG_DUMP("wDescriptorLength  =%d\r\n", p_hid_info->hid_desc.wDescriptorLength);

	if (p_hid_info->hid_desc.bNumDescriptors > 1) {
		UINT8 i;
		UINT8 *p_temp = p_hid_info->hid_desc.p_desc;

		for (i = 0; i < p_hid_info->hid_desc.bNumDescriptors - 1; i++) {
			DBG_DUMP("bDescriptorType  =0x%X\r\n", *p_temp++);
			DBG_DUMP("wDescriptorLength  =%d\r\n", *p_temp + *(p_temp+1)*256);
			p_temp+=2;
		}
	}
}
#else
static __inline void UVAC_DbgDmp_HID_INFO(UVAC_HID_INFO *p_hid_info) {}
static void UVAC_DbgDmp_StrDesc(UVAC_STRING_DESC *pStrDesc){}
static void UVAC_DbgDmp_VendDevDesc(PUVAC_VEND_DEV_DESC pVendDevDesc) {}
static __inline void UVAC_DbgDmp_EU_INFO(UVAC_EU_DESC *p_eu_desc) {}
#endif

#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
static void UVAC_DbgDmp_STRM_INFO(UVAC_STRM_INFO *pStrmInfo)
{
	DBG_DUMP("%s ==>\r\n", __func__);
	DBG_DUMP("pStrmHdr      =0x%x\r\n", pStrmInfo->pStrmHdr);
	DBG_DUMP("strmHdrSize   =0x%x\r\n", pStrmInfo->strmHdrSize);
	DBG_DUMP("strmPath      =0x%x\r\n", pStrmInfo->strmPath);
	DBG_DUMP("strmWidth     =0x%x\r\n", pStrmInfo->strmWidth);
	DBG_DUMP("strmHeight    =0x%x\r\n", pStrmInfo->strmHeight);
	DBG_DUMP("strmFps       =0x%x\r\n", pStrmInfo->strmFps);
	DBG_DUMP("strmCodec     =0x%x\r\n", pStrmInfo->strmCodec);
	DBG_DUMP("<===========\r\n");
}
static void UVAC_DbgDmp_UvacInfo(UVAC_INFO *pClassInfo)
{
	DBG_DUMP("%s ==>\r\n", __func__);
	if (0 == pClassInfo) {
		DBG_ERR("Input NULL\r\n");
		return;
	}
	DBG_DUMP("UvacMemAdr=0x%x\r\n", pClassInfo->UvacMemAdr);
	DBG_DUMP("UvacMemSize=0x%x\r\n", pClassInfo->UvacMemSize);
	DBG_DUMP("hwPayload=0x%x, 0x%x\r\n", pClassInfo->hwPayload[0], pClassInfo->hwPayload[1]);
	DBG_DUMP("channel=0x%x\r\n", pClassInfo->channel);
	UVAC_DbgDmp_STRM_INFO(&(pClassInfo->strmInfo));
	DBG_DUMP("fpStartVideoCB=0x%x\r\n", pClassInfo->fpStartVideoCB);
	DBG_DUMP("fpStopVideoCB=0x%x\r\n", pClassInfo->fpStopVideoCB);

	//DBG_DUMP("fpVendReqCB           =0x%x\r\n",g_fpUvacVendorReqCB);
	//DBG_DUMP("fpIQVendReqCB         =0x%x\r\n",g_fpUvacVendorReqIQCB);
	DBG_DUMP("<===========\r\n");
}
static void UVAC_DbgDmp_MemLayout(void)
{
	UINT32 tmpIdx = 0;
	DbgMsg_UVC(("%s:sAdr=0x%x,size=0x%x,desc=0x%x\r\n", __func__, guiUvacBufAddr, guiUvacBufTotalSize, gpUvacHSConfigDesc));
#if (UVAC_SUPPORT_SUPER_SPEED == ENABLE)
	DbgMsg_UVC(("%s:gpUvacSSConfigDesc=0x%x\r\n", __func__, gpUvacSSConfigDesc));
#endif
	for (tmpIdx = 0; tmpIdx < UVAC_VID_DEV_CNT_MAX; tmpIdx++) {
		DbgMsg_UVC(("gVidTxBuf[%d]=0x%x,Size=0x%x\r\n", tmpIdx, gVidTxBuf[tmpIdx].addr, gVidTxBuf[tmpIdx].size));
	}
}
static void UVAC_DbgDmp_MSDC_INFO(UVAC_MSDC_INFO *p_msdc_info)
{
	DBG_DUMP("--== MSDC info ==--\r\n");
	DBG_DUMP("en=%d\r\n", p_msdc_info->en);
}
#else
static __inline void UVAC_DbgDmp_STRM_INFO(UVAC_STRM_INFO *pStrmInfo) {}
static __inline void UVAC_DbgDmp_UvacInfo(UVAC_INFO *pClassInfo) {}
static __inline void UVAC_DbgDmp_MemLayout(void) {}
static __inline void UVAC_DbgDmp_MSDC_INFO(UVAC_MSDC_INFO *p_msdc_info) {}
#endif

static BOOL _is_iframe(UINT8 *pVidBuf)
{
	UINT8 value;

	value = pVidBuf[UVAC_VID_I_FRM_MARK_POS + user_data_size]&0x1F;

	// (BYTE & 0x1F)  = 5 (I-frame)  , 7(SPS)  ,  8(PPS) , 1(P-frame)
	if (value == 5 || value == 7) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static BOOL _is_iframe_h265(UINT8 *pVidBuf)
{
	UINT8 value;

	value = (pVidBuf[4 + user_data_size]&0x7E) >> 1;

	//printf("==== 0x%x 0x%x 0x%x 0x%x 0x%x \r\n", (UINT8)pVidBuf[0], (UINT8)pVidBuf[1],(UINT8)pVidBuf[2],(UINT8)pVidBuf[3],(UINT8)pVidBuf[4]);

	// (BYTE & 0x1F)  = 5 (I-frame)  , 7(SPS)  ,  8(PPS) , 1(P-frame)
	if (value == 32) {
		return TRUE;
	} else {
		return FALSE;
	}
}

//====== internal function ======
static __inline void UVAC_SetupMem(void)
{
	UINT32 DescSize = UVAC_MEM_DESC_SIZE;
#if 0
	UINT32 tmpV = (guiUvacBufTotalSize - UVAC_MEM_DESC_SIZE) / 2;
	if (tmpV % 4) {
		tmpV /= 4;
		tmpV *= 4;
	}
#endif
	DbgMsg_UVC(("Mem-Addr=0x%x, Size=0x%x, Desc=0x%x +\r\n", guiUvacBufAddr, guiUvacBufTotalSize, gpUvacHSConfigDesc));
	gpUvacHSConfigDesc = (UINT8 *)ALIGN_CEIL(guiUvacBufAddr, 64);
#if (UVAC_SUPPORT_SUPER_SPEED == ENABLE)
	gpUvacSSConfigDesc = (UINT8 *)ALIGN_CEIL(((uintptr_t)gpUvacHSConfigDesc) + UVAC_MEM_DESC_SIZE, 64);
	DescSize += UVAC_MEM_DESC_SIZE;
#endif
	memset((void *) guiUvacBufAddr, 0, DescSize);

	gVidTxBuf[UVAC_VID_DEV_CNT_1].addr = ALIGN_CEIL(guiUvacBufAddr + DescSize, UVAC_BUF_ALIGN);
	gVidTxBuf[UVAC_VID_DEV_CNT_1].size = ALIGN_FLOOR(gUvcMaxVideoFmtSize, UVAC_BUF_ALIGN);

	hwmem_memflush((void *) gVidTxBuf[UVAC_VID_DEV_CNT_1].addr, gVidTxBuf[UVAC_VID_DEV_CNT_1].size);
	if (UVAC_CHANNEL_1V1A < gUvacChannel || UVAC_CHANNEL_1V1A < gHsUvacChannel) {
		gVidTxBuf[UVAC_VID_DEV_CNT_2].addr = ALIGN_CEIL(gVidTxBuf[UVAC_VID_DEV_CNT_1].addr + gVidTxBuf[UVAC_VID_DEV_CNT_1].size, UVAC_BUF_ALIGN);
		gVidTxBuf[UVAC_VID_DEV_CNT_2].size = ALIGN_FLOOR(gUvcMaxVideoFmtSize, UVAC_BUF_ALIGN);
		hwmem_memflush((void *) gVidTxBuf[UVAC_VID_DEV_CNT_2].addr, gVidTxBuf[UVAC_VID_DEV_CNT_2].size);
	}

	if (UVAC_CHANNEL_3V1A == gUvacChannel || UVAC_CHANNEL_3V1A == gHsUvacChannel) {
		gVidTxBuf[UVAC_VID_DEV_CNT_3].addr = ALIGN_CEIL(gVidTxBuf[UVAC_VID_DEV_CNT_2].addr + gVidTxBuf[UVAC_VID_DEV_CNT_2].size, UVAC_BUF_ALIGN);
		gVidTxBuf[UVAC_VID_DEV_CNT_3].size = ALIGN_FLOOR(gUvcMaxVideoFmtSize, UVAC_BUF_ALIGN);
		hwmem_memflush((void *) gVidTxBuf[UVAC_VID_DEV_CNT_3].addr, gVidTxBuf[UVAC_VID_DEV_CNT_3].size);
	}

	if (gUvacUacRxEnabled) {
		if (UVAC_CHANNEL_1V1A < gUvacChannel || UVAC_CHANNEL_1V1A < gHsUvacChannel) {
			if (UVAC_CHANNEL_3V1A == gUvacChannel || UVAC_CHANNEL_3V1A == gHsUvacChannel) {
				//3V
				gAudRxBuf[0].va = ALIGN_CEIL(gVidTxBuf[UVAC_VID_DEV_CNT_3].addr + gVidTxBuf[UVAC_VID_DEV_CNT_3].size, UVAC_BUF_ALIGN);
				gAudRxBuf[0].pa = uvac_va_to_pa(UVAC_BUF_TYPE_WORK, gAudRxBuf[0].va);
				gAudRxBuf[0].size = ALIGN_FLOOR(gUvacUacBufSize, UVAC_BUF_ALIGN);
				hd_common_mem_flush_cache((void *) gAudRxBuf[0].va, gAudRxBuf[0].size);

				memset((void *)gAudRxBuf[0].va, 0, gAudRxBuf[0].size);
				vos_cpu_dcache_sync((VOS_ADDR)(gAudRxBuf[0].va), gAudRxBuf[0].size, VOS_DMA_BIDIRECTIONAL);
			} else {
				//2V
				gAudRxBuf[0].va = ALIGN_CEIL(gVidTxBuf[UVAC_VID_DEV_CNT_2].addr + gVidTxBuf[UVAC_VID_DEV_CNT_2].size, UVAC_BUF_ALIGN);
				gAudRxBuf[0].pa = uvac_va_to_pa(UVAC_BUF_TYPE_WORK, gAudRxBuf[0].va);
				gAudRxBuf[0].size = ALIGN_FLOOR(gUvacUacBufSize, UVAC_BUF_ALIGN);
				hd_common_mem_flush_cache((void *) gAudRxBuf[0].va, gAudRxBuf[0].size);

				memset((void *)gAudRxBuf[0].va, 0, gAudRxBuf[0].size);
				vos_cpu_dcache_sync((VOS_ADDR)(gAudRxBuf[0].va), gAudRxBuf[0].size, VOS_DMA_BIDIRECTIONAL);
			}
		} else {
			//1V
			gAudRxBuf[0].va = ALIGN_CEIL(gVidTxBuf[UVAC_VID_DEV_CNT_1].addr + gVidTxBuf[UVAC_VID_DEV_CNT_1].size, UVAC_BUF_ALIGN);
			gAudRxBuf[0].pa = uvac_va_to_pa(UVAC_BUF_TYPE_WORK, gAudRxBuf[0].va);
			gAudRxBuf[0].size = ALIGN_FLOOR(gUvacUacBufSize, UVAC_BUF_ALIGN);
			hd_common_mem_flush_cache((void *) gAudRxBuf[0].va, gAudRxBuf[0].size);

			memset((void *)gAudRxBuf[0].va, 0, gAudRxBuf[0].size);
			vos_cpu_dcache_sync((VOS_ADDR)(gAudRxBuf[0].va), gAudRxBuf[0].size, VOS_DMA_BIDIRECTIONAL);
		}
	}
}

/*
    UVAC_GetImageSize
*/
static void UVAC_GetImageSize(UVAC_VIDEO_FORMAT codec_type, UINT32 frm_idx, UINT32 vidDevIdx, UINT32 *pOutWidth, UINT32 *pOutHeight)
{
	UINT32 tmpIdx = frm_idx - 1;
	UVAC_VID_RESO_ARY *p_frm_info;
	DbgMsg_UVC(("GetImageSize:idx=%d, %d\r\n", frm_idx, tmpIdx));

	if (U3DEV_SPEED_SS == g_usb_speed) {
		if (codec_type == UVAC_VIDEO_FORMAT_YUV) {
			p_frm_info = &gU3UvcYuvFrmInfo[vidDevIdx];
		} else if (codec_type == UVAC_VIDEO_FORMAT_MJPG) {
			p_frm_info = &gU3UvcMjpgFrmInfo[vidDevIdx];
		} else if (codec_type == UVAC_VIDEO_FORMAT_H264) {
			p_frm_info = &gU3UvcH264FrmInfo[vidDevIdx];
		} else if (codec_type == UVAC_VIDEO_FORMAT_NV12) {
			p_frm_info = &gU3UvcNV12FrmInfo[vidDevIdx];
		} else if (codec_type == UVAC_VIDEO_FORMAT_H265) {
			p_frm_info = &gU3UvcH265FrmInfo[vidDevIdx];
		} else {
			DBG_ERR("unknow codec type(%d)\r\n", codec_type);
			return;
		}
	} else {
		if (codec_type == UVAC_VIDEO_FORMAT_YUV) {
			if (gU3HsUvcYuvFrmInfo[vidDevIdx].aryCnt != 0) {
				p_frm_info = &gU3HsUvcYuvFrmInfo[vidDevIdx];
			} else {
				p_frm_info = &gU3UvcYuvFrmInfo[vidDevIdx];
			}
		} else if (codec_type == UVAC_VIDEO_FORMAT_MJPG) {
			if (gU3HsUvcMjpgFrmInfo[vidDevIdx].aryCnt != 0) {
				p_frm_info = &gU3HsUvcMjpgFrmInfo[vidDevIdx];
			} else {
				p_frm_info = &gU3UvcMjpgFrmInfo[vidDevIdx];
			}
		} else if (codec_type == UVAC_VIDEO_FORMAT_H264) {
			if (gU3HsUvcH264FrmInfo[vidDevIdx].aryCnt != 0) {
				p_frm_info = &gU3HsUvcH264FrmInfo[vidDevIdx];
			} else {
				p_frm_info = &gU3UvcH264FrmInfo[vidDevIdx];
			}
		} else if (codec_type == UVAC_VIDEO_FORMAT_NV12) {
			if (gU3HsUvcNV12FrmInfo[vidDevIdx].aryCnt != 0) {
				p_frm_info = &gU3HsUvcNV12FrmInfo[vidDevIdx];
			} else {
				p_frm_info = &gU3UvcNV12FrmInfo[vidDevIdx];
			}
		} else if (codec_type == UVAC_VIDEO_FORMAT_H265) {
			if (gU3HsUvcH265FrmInfo[vidDevIdx].aryCnt != 0) {
				p_frm_info = &gU3HsUvcH265FrmInfo[vidDevIdx];
			} else {
				p_frm_info = &gU3UvcH265FrmInfo[vidDevIdx];
			}
		} else {
			DBG_ERR("unknow codec type(%d)\r\n", codec_type);
			return;
		}

	}

	if (frm_idx && (p_frm_info->aryCnt>= frm_idx)) {
		*pOutWidth = p_frm_info->pVidResAry[tmpIdx].width;
		*pOutHeight = p_frm_info->pVidResAry[tmpIdx].height;
	} else {
		DBG_ERR("Frame_Index=%d, num=%d\r\n", frm_idx, p_frm_info->aryCnt);
	}
	DbgMsg_UVC(("GetImageSize[%d]=%d, %d, codec type =%d\r\n", tmpIdx, *pOutWidth, *pOutHeight, codec_type));
}

static UINT16 UVAC_SetIntfString(void)
{
	if ((p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_1] != NULL) && (p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_1]->b_descriptor_type == 0x03)) {
		u3_uvc_string_idx[UVAC_VID_DEV_CNT_1] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uvc_string_idx[UVAC_VID_DEV_CNT_1], (void *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_1]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uvc_string_idx[UVAC_VID_DEV_CNT_1], (void *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_1]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uvc_string_idx[UVAC_VID_DEV_CNT_1], (void *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_1]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_1]);
	}
	if ((p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_1] != NULL) && (p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_1]->b_descriptor_type == 0x03)) {
		u3_uac_string_idx[UVAC_AUD_DEV_CNT_1] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uac_string_idx[UVAC_AUD_DEV_CNT_1], (void *)p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_1]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uac_string_idx[UVAC_AUD_DEV_CNT_1], (void *)p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_1]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uac_string_idx[UVAC_AUD_DEV_CNT_1], (void *)p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_1]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_1]);
	}

	if ((p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_2] != NULL) && (p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_2]->b_descriptor_type == 0x03)) {
		u3_uvc_string_idx[UVAC_VID_DEV_CNT_2] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uvc_string_idx[UVAC_VID_DEV_CNT_2], (void *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_2]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uvc_string_idx[UVAC_VID_DEV_CNT_2], (void *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_2]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uvc_string_idx[UVAC_VID_DEV_CNT_2], (void *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_2]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_2]);
	}
	if ((p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_2] != NULL) && (p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_2]->b_descriptor_type == 0x03)) {
		u3_uac_string_idx[UVAC_AUD_DEV_CNT_2] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uac_string_idx[UVAC_AUD_DEV_CNT_2], (void *)p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_2]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uac_string_idx[UVAC_AUD_DEV_CNT_2], (void *)p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_2]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uac_string_idx[UVAC_AUD_DEV_CNT_2], (void *)p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_2]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_2]);
	}
	if ((p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_3] != NULL) && (p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_3]->b_descriptor_type == 0x03)) {
		u3_uvc_string_idx[UVAC_VID_DEV_CNT_3] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uvc_string_idx[UVAC_VID_DEV_CNT_3], (void *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_3]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uvc_string_idx[UVAC_VID_DEV_CNT_3], (void *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_3]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uvc_string_idx[UVAC_VID_DEV_CNT_3], (void *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_3]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_3]);
	}

	if ((g_u3_hid_info.p_vendor_string != NULL) && (g_u3_hid_info.p_vendor_string->bDescriptorType == 0x03)) {
		u3_hid_string_idx = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_hid_string_idx, (void *)g_u3_hid_info.p_vendor_string);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_hid_string_idx, (void *)g_u3_hid_info.p_vendor_string);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_hid_string_idx, (void *)g_u3_hid_info.p_vendor_string);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)g_u3_hid_info.p_vendor_string);
	}

	if ((p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_1] != NULL) && (p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_1]->b_descriptor_type == 0x03)) {
		u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_1] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_1], (void *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_1]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_1], (void *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_1]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_1], (void *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_1]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_1]);
	}
	if ((p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_1] != NULL) && (p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_1]->b_descriptor_type == 0x03)) {
		u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_1] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_1], (void *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_1]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_1], (void *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_1]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_1], (void *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_1]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_1]);
	}

	if ((p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_2] != NULL) && (p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_2]->b_descriptor_type == 0x03)) {
		u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_2] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_2], (void *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_2]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_2], (void *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_2]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_2], (void *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_2]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_2]);
	}
	if ((p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_2] != NULL) && (p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_2]->b_descriptor_type == 0x03)) {
		u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_2] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_2], (void *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_2]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_2], (void *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_2]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_2], (void *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_2]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_2]);
	}

	if ((p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_3] != NULL) && (p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_3]->b_descriptor_type == 0x03)) {
		u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_3] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_3], (void *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_3]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_3], (void *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_3]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uvc_iad_string_idx[UVAC_VID_DEV_CNT_3], (void *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_3]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_3]);
	}
	if ((p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_3] != NULL) && (p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_3]->b_descriptor_type == 0x03)) {
		u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_3] = u3_free_string_idx++;
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_3], (void *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_3]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_3], (void *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_3]);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+u3_uac_iad_string_idx[UVAC_AUD_DEV_CNT_3], (void *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_3]);
		UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_3]);
	}

	return u3_free_string_idx;
}

static __inline void UVAC_ResetParam(void)
{
	UINT32 i;
	//UINT32 uiOutWidth = 0, uiOutHeight = 0;
	DBG_IND("++\r\n");

	guiUvacBufAddr = guiUvacBufTotalSize = 0;
	gpUvacHSConfigDesc = 0;
#if (UVAC_SUPPORT_SUPER_SPEED == ENABLE)
	gpUvacSSConfigDesc = 0;
#endif
	for (i = 0; i < UVAC_VID_DEV_CNT_MAX; i++) {
		memset((void *)&gUvacVidStrmInfo[i], 0, sizeof(UVAC_VID_STRM_INFO));
		gUvacCodecType[i] = UVAC_VIDEO_FORMAT_H264;

		//UVAC_GetImageSize(gUvacCodecType[i], gUvcParamAry[i].OutSizeID, i, &uiOutWidth, &uiOutHeight);
		gUvacVidStrmInfo[i].strmInfo.strmWidth = 640;
		gUvacVidStrmInfo[i].strmInfo.strmHeight = 480;
		gUvacVidStrmInfo[i].strmInfo.strmFps = 30;
		gUvacVidStrmInfo[i].strmInfo.strmCodec = gUvacCodecType[i];
		if (UVAC_VID_DEV_CNT_1 == i) {
			gUvacVidStrmInfo[i].strmInfo.strmPath = UVAC_STRM_VID;
		} else if (UVAC_VID_DEV_CNT_2 == i) {
			gUvacVidStrmInfo[i].strmInfo.strmPath = UVAC_STRM_VID2;
		} else if (UVAC_VID_DEV_CNT_3 == i) {
			gUvacVidStrmInfo[i].strmInfo.strmPath = UVAC_STRM_VID3;
		} else {
			DBG_ERR("Unknown stream-path: %d\r\n", i);
		}

		gUvcVidStart[i] = FALSE;
		#if 0
		gUvacTimerID[i] = TIMER_NUM;
		#endif
		gU3UvacRunningFlag[i] = 0;
		gUvacIntfIdx_VC[i] = UVAC_INTF_IDX_UNKNOWN;
		gUvacIntfIdx_VS[i] = UVAC_INTF_IDX_UNKNOWN;
		gVidTxBuf[i].addr = 0;
		gVidTxBuf[i].size = 0;

	}
	for (i = 0; i < UVAC_AUD_DEV_CNT_MAX; i++) {
		gUacAudStart[i] = FALSE;
		gUvacIntfIdx_AC[i] = UVAC_INTF_IDX_UNKNOWN;
		gUvacIntfIdx_AS[i] = UVAC_INTF_IDX_UNKNOWN;
	}
	gUvcTrigEnabled = 0; //remove-able?
	gUvacIntfIdx_WinUsb = UVAC_INTF_IDX_UNKNOWN;
	gUvacIntfIdx_vidAryCurrIdx = 0;
	gUvacIntfIdx_audAryCurrIdx = 0;
	gUacSampleRate = gUvacAudSampleRate[0];
	gUacRxSampleRate = gUvacAudRxSampleRate[0];
	gUvcNoVidStrm = FALSE;
	u3_free_string_idx = FREE_STRING_INDEX_BEGIN;
	DBG_IND("--gUacSampleRate=%d\r\n", gUacSampleRate);
}
static ER UVC_TimeTrigSet(UINT32 intrVal, UINT32 idx)
{
	ER retV = E_OK;
	//UINT32 intval = intrVal;
	#if 0
	DbgMsg_UVC(("++TimeTrig:idx=%d,intrVal=%d,en=%d,T1=0x%x,T2=0x%x,maxCh=%d\n\r", idx, intrVal, gUvcTrigEnabled, gUvacTimerID[0], gUvacTimerID[1], gUvacChannel));
	#endif
	//if (gUvcTrigEnabled < UVAC_VID_DEV_CNT)
	if (gUvcTrigEnabled < UVAC_VID_DEV_CNT_MAX) {
		gUvcTrigEnabled ++;
		if (FALSE == uvc_direct_trigger) {
			#if 0
			if (0 == intval) {
				intval = 1000000 / UVC_FRMRATE_30;
				DbgMsg_UVC(("Time-intrVal zero,set to %d fps, %d\r\n", __func__, UVC_FRMRATE_30, intval));
			}
			if (UVAC_VID_DEV_CNT_1 == idx) {
				if (gUvacTimerID[UVAC_VID_DEV_CNT_1] != TIMER_NUM) {
					DBG_ERR("Timer1 already open:%d,%d\n\r", gUvacTimerID[0], TIMER_NUM);
				}
				retV = timer_open(&gUvacTimerID[UVAC_VID_DEV_CNT_1], NULL);
				if (retV != E_OK) {
					DBG_ERR("open timer fail=%d\n\r", retV);
					return retV;
				}
				retV = timer_cfg(gUvacTimerID[UVAC_VID_DEV_CNT_1], intval, TIMER_MODE_FREE_RUN | TIMER_MODE_ENABLE_INT, TIMER_STATE_PLAY);
				if (retV != E_OK) {
					DBG_ERR("set timer fail=%d,intval=%d\n\r", retV, intval);
					return retV;
				}
			} else if ((gUvacChannel > UVAC_CHANNEL_1V1A) && (UVAC_VID_DEV_CNT_2 == idx)) {
				if (gUvacTimerID[UVAC_VID_DEV_CNT_2] != TIMER_NUM) {
					DBG_ERR("Timer[%d] already open\n\r", UVAC_VID_DEV_CNT_2);
				}
				retV = timer_open(&gUvacTimerID[UVAC_VID_DEV_CNT_2], NULL);
				if (retV != E_OK) {
					DBG_ERR("open timer[%d] fail=%d\n\r", UVAC_VID_DEV_CNT_2, retV);
					return retV;
				}
				retV = timer_cfg(gUvacTimerID[UVAC_VID_DEV_CNT_2], intval, TIMER_MODE_FREE_RUN | TIMER_MODE_ENABLE_INT, TIMER_STATE_PLAY);
				if (retV != E_OK) {
					DBG_ERR("set timer[%d] fail=%d,intval=%d\n\r", UVAC_VID_DEV_CNT_2, retV, intval);
					return retV;
				}
			} else {
				DBG_ERR("Not Support MaxCh=%d or idx=%d\n\r", gUvacChannel, idx);
				return E_PAR;
			}
			#endif
		}
	} else {
		DBG_ERR("Already Trig Both=%d\r\n", gUvcTrigEnabled);
		return E_PAR;
	}

	#if 0
	DbgMsg_UVCIO(("--TimeTrig:idx=%d,intrVal=%d,en=%d,T1=0x%x,T2=0x%x,maxCh=%d\n\r", idx, intrVal, gUvcTrigEnabled, gUvacTimerID[UVAC_VID_DEV_CNT_1], gUvacTimerID[UVAC_VID_DEV_CNT_2], gUvacChannel));
	#endif

	return retV;
}

static ER UVC_TimeTrigClose(UINT32 idx)
{
	ER retV = E_OK;
	DbgMsg_UVC(("+%s:MaxCh=%d,idx=%d,cnt=%d,start=%d,%d\n\r", __func__, gUvacChannel, idx, gUvcTrigEnabled, gUacAudStart[0], gUacAudStart[1]));

	if (gUvcTrigEnabled) {
		gUvcTrigEnabled --;
		if (FALSE == uvc_direct_trigger) {
			#if 0
			if (UVAC_VID_DEV_CNT_1 == idx) {
				DbgMsg_UVC(("MaxCh=%d,idx=%d,AudStart[0]=%d\r\n", gUvacChannel, idx, gUacAudStart[UVAC_VID_DEV_CNT_1]));
				retV = timer_close(gUvacTimerID[UVAC_VID_DEV_CNT_1]);
				gUvacTimerID[UVAC_VID_DEV_CNT_1] = TIMER_NUM;
			}
			DbgMsg_UVC(("MaxCh=%d,idx=%d,AudStart[1]=%d\r\n", gUvacChannel, idx, gUacAudStart[UVAC_VID_DEV_CNT_2]));
			//if ((UVAC_VID_DEV_CNT_2 == idx) && (FALSE == gUacAudStart[UVAC_VID_DEV_CNT_2]))
			if (UVAC_VID_DEV_CNT_2 == idx) {
				retV = timer_close(gUvacTimerID[UVAC_VID_DEV_CNT_2]);
				gUvacTimerID[UVAC_VID_DEV_CNT_2] = TIMER_NUM;
			}
			#endif
		}
		#if 0
		if (FALSE == gUacAudStart[UVAC_AUD_DEV_CNT_1]) {
			UVAC_RemoveTxfInfo(UVAC_TXF_QUE_A1);
		}
		if (FALSE == gUacAudStart[UVAC_AUD_DEV_CNT_2]) {
			UVAC_RemoveTxfInfo(UVAC_TXF_QUE_A2);
		}
		if (0 == gUvcTrigEnabled) {
			UVAC_IsoInTxfStateMachine(UVC_ISOIN_TXF_ACT_STOP);
		}
		#endif
	} else {
		DBG_ERR("^RTrig Already CLose\r\n");
	}

	DbgMsg_UVCIO(("-%s:MaxCh=%d,idx=%d,cnt=%d,start=%d,%d, retV=%d\n\r", __func__, gUvacChannel, idx, gUvcTrigEnabled, gUacAudStart[UVAC_VID_DEV_CNT_1], gUacAudStart[UVAC_VID_DEV_CNT_2], retV));
	return retV;
}

/*
    Start UVAC data streaming.
*/
int UVAC_Start(UVAC_VID_DEV_CNT vidDevIdx)
{
	DbgMsg_UVC(("^R+-%s:vidDevIdx=%d\r\n", __func__, vidDevIdx));
	gUvcUsbDMAAbord = FALSE;
	if (UVAC_VID_DEV_CNT_1 == vidDevIdx) {
		clr_flg(FLG_ID_UVAC, FLGUVAC_STOP | FLGUVAC_VIDEO_STOP);
		set_flg(FLG_ID_UVAC, FLGUVAC_START);
	} else if (UVAC_VID_DEV_CNT_2 == vidDevIdx) {
		clr_flg(FLG_ID_UVAC, FLGUVAC_STOP2 | FLGUVAC_VIDEO2_STOP);
		set_flg(FLG_ID_UVAC, FLGUVAC_START2);
	}  else if (UVAC_VID_DEV_CNT_3 == vidDevIdx) {
		clr_flg(FLG_ID_UVAC_UVC3, FLGUVAC_STOP3 | FLGUVAC_VIDEO3_STOP);
		set_flg(FLG_ID_UVAC_UVC3, FLGUVAC_START3);
	} else {
		DBG_ERR("Unknown VidDevIdx=%d\r\n", vidDevIdx);
	}
	if (gUvcTrigEnabled) {
		DbgMsg_UVC(("^RVidStart[%d]:%d,V1=%d,%d,A1=%d,%d\r\n", vidDevIdx, gUvcTrigEnabled, gUvcVidStart[0], gUvcVidStart[1], gUacAudStart[0], gUacAudStart[1]));
	}
	return E_OK;
}

/*
    Stop UVAC data streaming.

    @note  Important!! Important!! Important!! Important!! Important!! Important!!
         Here we assume the stop command will send after the whole image return!!
         We don't handle stop command in FDMA running!!
         If we should support that case, we should stop/flush FDMA channel-B/USB code.

*/
static int UVAC_StopAll(void)
{
	FLGPTN uiFlag;

	uiFlag = kchk_flg(FLG_ID_UVAC, (FLGUVAC_START | FLGUVAC_START2));
	DbgMsg_UVC(("^R+%s:0x%x\r\n", __func__, uiFlag));
	if (uiFlag & (FLGUVAC_START | FLGUVAC_START2)) {
		FLGPTN uiWaitReadyFlg = 0;
		if (uiFlag & FLGUVAC_START) {
			uiWaitReadyFlg |= FLGUVAC_RDY;
		}
		if (uiFlag & FLGUVAC_START2) {
			uiWaitReadyFlg |= FLGUVAC_RDY2;
		}
		//gUvcNoVidStrm = TRUE;
		clr_flg(FLG_ID_UVAC, (FLGUVAC_START | FLGUVAC_START2));
		set_flg(FLG_ID_UVAC, (FLGUVAC_STOP | FLGUVAC_STOP2));
		if (uvc_direct_trigger) {
			clr_flg(FLG_ID_UVAC, (FLGUVAC_VIDEO_TXF | FLGUVAC_VIDEO2_TXF));
			set_flg(FLG_ID_UVAC, (FLGUVAC_VIDEO_STOP | FLGUVAC_VIDEO2_STOP));
		}
		gU3UvacRunningFlag[UVAC_VID_DEV_CNT_1] |= FLGUVAC_STOP;
		gU3UvacRunningFlag[UVAC_VID_DEV_CNT_2] |= FLGUVAC_STOP2;

		vos_flag_set(FLG_ID_UVAC_FRM, (FLGUVAC_FRM_V1 | FLGUVAC_FRM_V2 | FLGUVAC_FRM_A1 | FLGUVAC_FRM_A2));

		DbgMsg_UVC(("+UVAC STOP All \r\n"));
		wai_flg(&uiFlag, FLG_ID_UVAC, uiWaitReadyFlg, TWF_ANDW);
		DbgMsg_UVC(("-UVAC STOP All \r\n"));
		return E_OK;
	}

	uiFlag = kchk_flg(FLG_ID_UVAC_UVC3, (FLGUVAC_START3));
	DbgMsg_UVC(("^R+%s:0x%x\r\n", __func__, uiFlag));
	if (uiFlag & (FLGUVAC_START3)) {
		FLGPTN uiWaitReadyFlg = 0;
		if (uiFlag & FLGUVAC_START3) {
			uiWaitReadyFlg |= FLGUVAC_RDY3;
		}
		//gUvcNoVidStrm = TRUE;
		clr_flg(FLG_ID_UVAC_UVC3, (FLGUVAC_START3));
		set_flg(FLG_ID_UVAC_UVC3, (FLGUVAC_STOP3));
		if (uvc_direct_trigger) {
			clr_flg(FLG_ID_UVAC_UVC3, (FLGUVAC_VIDEO3_TXF));
			set_flg(FLG_ID_UVAC_UVC3, (FLGUVAC_VIDEO3_STOP));
		}
		gU3UvacRunningFlag[UVAC_VID_DEV_CNT_3] |= FLGUVAC_STOP3;

		vos_flag_set(FLG_ID_UVAC_FRM, (FLGUVAC_FRM_V3));

		DbgMsg_UVC(("+UVAC STOP All \r\n"));
		wai_flg(&uiFlag, FLG_ID_UVAC_UVC3, uiWaitReadyFlg, TWF_ANDW);
		DbgMsg_UVC(("-UVAC STOP All \r\n"));
		return E_OK;
	}

	DbgMsg_UVC(("^R-%s\r\n", __func__));
	return E_SYS;

}
/*int UVAC_Stop(UVAC_VID_DEV_CNT vidDevIdx)
{
	FLGPTN uiFlag = 0, chkFlag = 0, setFlag = 0, rdyFlag = 0;

	if (UVAC_VID_DEV_CNT_1 == vidDevIdx) {
		chkFlag = FLGUVAC_START;
		setFlag = FLGUVAC_STOP;
		if (uvc_direct_trigger) {
			setFlag |= FLGUVAC_VIDEO_STOP;
		}
		rdyFlag = FLGUVAC_RDY;
	} else if (UVAC_VID_DEV_CNT_2 == vidDevIdx) {
		chkFlag = FLGUVAC_START2;
		setFlag = FLGUVAC_STOP2;
		if (uvc_direct_trigger) {
			setFlag |= FLGUVAC_VIDEO2_STOP;
		}
		rdyFlag = FLGUVAC_RDY2;
	} else {
		DBG_ERR("Unknown VidDevIdx=%d\r\n", vidDevIdx);
		return E_SYS;
	}
	DbgMsg_UVC(("^R+%s vid[%d]:chkFlag=0x%x,setFlag=0x%x,rdyFlag=0x%x\r\n", __func__, vidDevIdx, chkFlag, setFlag, rdyFlag));
	if (kchk_flg(FLG_ID_UVAC, chkFlag) & chkFlag) {
		clr_flg(FLG_ID_UVAC, chkFlag);
		gUvacRunningFlag[vidDevIdx] |= setFlag;
		set_flg(FLG_ID_UVAC, setFlag);

		DbgMsg_UVC(("+UVAC[%d] STOP Run \r\n", vidDevIdx));
		wai_flg(&uiFlag, FLG_ID_UVAC, rdyFlag, TWF_ANDW);
		DbgMsg_UVC(("-UVAC[%d] STOP Run \r\n", vidDevIdx));

		return E_OK;
	}

	DbgMsg_UVC(("^R-%s\r\n", __func__));
	return E_SYS;
}*/

int _U3UVC_Stop(UVAC_VID_DEV_CNT vidDevIdx)
{
	FLGPTN uiFlag = 0, chkFlag = 0, setFlag = 0, rdyFlag = 0;
	ID flag_uvac = (vidDevIdx == UVAC_VID_DEV_CNT_3)? FLG_ID_UVAC_UVC3 : FLG_ID_UVAC;

	if (UVAC_VID_DEV_CNT_1 == vidDevIdx) {
		chkFlag = FLGUVAC_START;
		setFlag = FLGUVAC_STOP;
		rdyFlag = FLGUVAC_RDY;
		if (uvc_direct_trigger) {
			setFlag |= FLGUVAC_VIDEO_STOP;
		}
	} else if (UVAC_VID_DEV_CNT_2 == vidDevIdx) {
		chkFlag = FLGUVAC_START2;
		setFlag = FLGUVAC_STOP2;
		rdyFlag = FLGUVAC_RDY2;
		if (uvc_direct_trigger) {
			setFlag |= FLGUVAC_VIDEO2_STOP;
		}
	} else if (UVAC_VID_DEV_CNT_3 == vidDevIdx) {
		chkFlag = FLGUVAC_START3;
		setFlag = FLGUVAC_STOP3;
		rdyFlag = FLGUVAC_RDY3;
		if (uvc_direct_trigger) {
			setFlag |= FLGUVAC_VIDEO3_STOP;
		}
	} else {
		DBG_ERR("Unknown VidDevIdx=%d\r\n", vidDevIdx);
		return E_SYS;
	}
	DbgMsg_UVC(("^R+%s vid[%d]:chkFlag=0x%x,setFlag=0x%x,rdyFlag=0x%x\r\n", __func__, vidDevIdx, chkFlag, setFlag, rdyFlag));
	if (kchk_flg(flag_uvac, chkFlag) & chkFlag) {
		clr_flg(flag_uvac, chkFlag);
		gU3UvacRunningFlag[vidDevIdx] |= setFlag;
		set_flg(flag_uvac, setFlag);

		DbgMsg_UVC(("+UVAC[%d] STOP Run \r\n", vidDevIdx));
		wai_flg(&uiFlag, flag_uvac, rdyFlag, TWF_ANDW);
		DbgMsg_UVC(("-UVAC[%d] STOP Run \r\n", vidDevIdx));

		return E_OK;
	}

	DbgMsg_UVC(("^R-%s\r\n", __func__));
	return E_SYS;
}
static int _UAC_Stop(UVAC_AUD_DEV_CNT audDevIdx)
{
	UINT32 txfQueIdx;
	UINT32 tmpV = 0;
	FLGPTN rdyFlag = 0;


	if (UVAC_AUD_DEV_CNT_1 == audDevIdx) {
		rdyFlag = FLGUVAC_ISOIN_AUD_READY;
	} else if (UVAC_AUD_DEV_CNT_2 == audDevIdx) {
		rdyFlag = FLGUVAC_ISOIN_AUD2_READY;
	}

	if (audDevIdx == UVAC_AUD_DEV_CNT_3){
		loc_cpu();
		tmpV = gUacAudStart[UVAC_AUD_DEV_CNT_3];
		gUacAudStart[UVAC_AUD_DEV_CNT_3] = FALSE;
		unl_cpu();

		if (TRUE == tmpV) {
			usb3dev_abortEndpoint(UVAC_USB_RX_EP[0]);
		}
	} else if (audDevIdx == UVAC_AUD_DEV_CNT_1 || audDevIdx == UVAC_AUD_DEV_CNT_2){
		loc_cpu();
		tmpV = gUacAudStart[audDevIdx];
		gUacAudStart[audDevIdx] = FALSE;
		unl_cpu();
		txfQueIdx = audDevIdx + UVAC_VID_DEV_CNT_MAX;//UVAC_TXF_QUE_A1, UVAC_TXF_QUE_A2

		if (TRUE == tmpV) {
			if (g_fpStartAudio) {
				gAudStrmInfo[audDevIdx].isAudStrmOn = FALSE;
				g_fpStartAudio(audDevIdx, &gAudStrmInfo[audDevIdx]);
			}
			usb3dev_abortEndpoint(UVAC_USB_EP[txfQueIdx]);
			UVAC_RemoveTxfInfo(txfQueIdx);
			if (audDevIdx == UVAC_AUD_DEV_CNT_1) {
				vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A1);
			} else {
				vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A2);
			}
		} else {
			if (0 == kchk_flg(FLG_ID_UVAC, rdyFlag)) {
				vos_util_delay_ms(70);

				usb3dev_abortEndpoint(UVAC_USB_EP[txfQueIdx]);
			}
		}
	}


	return E_OK;

}
//INterface, AlternatSetting are highly depends on the definition of USB device driver
static BOOL UVAC_GetIntfAltsetFromCB(UINT32 uEvent, UINT8 *pIntf, UINT8 *pAltSet)
{
	if (uEvent) {
		*pIntf = uEvent >> 16;
		*pAltSet = (UINT8)uEvent;
		return TRUE;
	} else {
		return FALSE;
	}
}

static void UVAC_InterfaceCB(uintptr_t event)
{
	UINT32 i = 0;
	//ER retV = E_OK;
	//UINT32 tmpV = 0, txfQueIdx;
	UINT32 txfQueIdx;
	UINT8 intf = 0, altSet = 0;
	BOOL found = FALSE;
	UINT32 uEvent = (UINT32)event;

	DbgMsg_UVCIO(("%s:0x%x\r\n", __func__, uEvent));
	if (UVAC_GetIntfAltsetFromCB(uEvent, &intf, &altSet)) {
		DbgMsg_UVC(("^RUSBDrv-CB:0x%x,0x%x,0x%x\r\n", uEvent, intf, altSet));
	} else {
		DBG_DUMP("!!Unknown:0x%x,uvac channel=%d\r\n", uEvent, gUvacChannel);
		return;
	}
	for (i = 0; i < UVAC_VID_DEV_CNT_MAX; i++) {
		if (intf == gUvacIntfIdx_VS[i]) {
			//Clear FIFO when setting interface
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
			if (gUvacOpened) {
#if 0 //Interrupt
				usb_clrEPFIFO(USB_EP5);
				usb_clrEPFIFO(USB_EP6);
#endif
				//usb_clrEPFIFO(USB_EP1);
				if (gUvacChannel > UVAC_CHANNEL_1V1A) {
					//usb_clrEPFIFO(USB_EP2);
				}
				if (gUvcCapM3Enable) {
					//usb_clrEPFIFO(USB_EP7);
				}
			}
#endif
			/*if (gUVCIsoinTxfUnitSize[i] == 0) {
				if (usb3dev_getDevInfo(U3DEV_DEVINFO_ID_SPEED) == U3DEV_SPEED_SS) {
					gUVCIsoinTxfUnitSize[i] = gU3UvcIsoInSsPacketSize[i] * gU3UvcIsoInSsBandWidth[i];
				} else {
					gUVCIsoinTxfUnitSize[i] = gU3UvcIsoInHsPacketSize[i] * gU3UvcIsoInHsBandWidth[i];
 				}
			}*/

			if (altSet) {
				DbgMsg_UVC(("Vid[%d] Start[%d/%d]:evt=0x%x\r\n", i, intf, altSet, uEvent));

				loc_cpu();
				gUvcVidStart[i] = TRUE;
				unl_cpu();
				UVAC_Start(i);
				found = TRUE;
				//run time change packet size only valid while user set UVAC_CONFIG_MAX_PAYLOAD_TX_SIZE
				if (altSet <= UVAC_VID_EP_PACK_SIZE_MAX_CNT) {
					if (g_usb_speed == U3DEV_SPEED_SS) {
						gUVCIsoinTxfUnitSize[i] = gU3UvcIsoInSsPacketSize[altSet-1] * gU3UvcIsoInSsBandWidth[altSet-1];
						//UVAC_ResetTxfPara();
						usb3_reset_ep_cfg(UVAC_USB_EP[i], 1, gU3UvcIsoInSsPacketSize[altSet-1], gU3UvcIsoInSsMaxBurst[altSet-1], USB_EP_ISO_INTERVAL, 1, gU3UvcIsoInSsEpcAttr[altSet-1]);
					} else {
						gUVCIsoinTxfUnitSize[i] = gU3UvcIsoInHsPacketSize[altSet-1] * gU3UvcIsoInHsBandWidth[altSet-1];
						//UVAC_ResetTxfPara();
						usb3_reset_ep_cfg(UVAC_USB_EP[i], 1, gU3UvcIsoInHsPacketSize[altSet-1], 0, USB_EP_ISO_INTERVAL, 1, gU3UvcIsoInHsBandWidth[altSet-1]-1);
					}
				}
				//DbgMsg_UVC(("gUVCIsoinTxfUnitSize[%d]=0x%X\r\n", i, gUVCIsoinTxfUnitSize[i]));
				printf("gUVCIsoinTxfUnitSize[%d]=0x%X\r\n", i, gUVCIsoinTxfUnitSize[i]);
				break;
			} else {
				DbgMsg_UVC(("Vid[%d] Stop[%d/%d]:evt=0x%x,stillTrig=%d, Ori=%d\r\n", i, intf, altSet, uEvent, gUvacStillImgTrigSts, gUvcVidStart[i]));

				if (u3_uvc_bulk_mode[i] != TRUE) {
					DbgMsg_UVC(("Vid[%d] Stop[%d/%d]:evt=0x%x,stillTrig=%d, Ori=%d\r\n", i, intf, altSet, uEvent, gUvacStillImgTrigSts, gUvcVidStart[i]));

					loc_cpu();
					gUvcVidStart[i] = FALSE;
					unl_cpu();
					if (i == UVAC_VID_DEV_CNT_1) {
						txfQueIdx = UVAC_TXF_QUE_V1;
					} else { // if(i == UVAC_VID_DEV_CNT_2)
						txfQueIdx = UVAC_TXF_QUE_V2;
					}

					_U3UVC_Stop(i);
					UVAC_RemoveTxfInfo(txfQueIdx);

					usb3dev_abortEndpoint(UVAC_USB_EP[txfQueIdx]);

					if (gUvacStillImgTrigSts == UVC_STILLIMG_TRIG_CTRL_TRANSMIT_BULK) { //2
						//UVC_TrigStilImg(gUvcCapImgAddr, gUvcCapImgSize);
					}
				}

				found = TRUE;

				break;
			}
		}
	}
	if (FALSE == found) {
		for (i = 0; i < UVAC_AUD_DEV_CNT_MAX; i++) {
			if (intf == gUvacIntfIdx_AS[i] && i == UVAC_AUD_DEV_CNT_3) {
				DbgMsg_UVC(("Aud[%d] [inf: %d/alt: %d]:evt=0x%x\r\n", i, intf, altSet, uEvent));
				found = TRUE;
				//usb_clrEPFIFO(U2UVAC_USB_RX_EP[0]);

				if (altSet) {
					//usb_clrEPFIFO(U2UVAC_USB_RX_EP[0]);
					_UVAC_init_queue();
					loc_cpu();
					gUacAudStart[i] = TRUE;
					unl_cpu();
					break;
				} else {
					UINT32 tmpV = 0;

					tmpV = gUacAudStart[i];
					loc_cpu();
					gUacAudStart[i] = FALSE;
					unl_cpu();

					if (TRUE == tmpV) {
						usb3dev_abortEndpoint(UVAC_USB_RX_EP[0]);
					}

					break;
				}
				break;
			} else if (intf == gUvacIntfIdx_AS[i]) {
				//Clear FIFO when setting interface
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
				if (gUvacOpened) {
					//usb_clrEPFIFO(USB_EP3);
					if (gUvacChannel > UVAC_CHANNEL_1V1A) {
						//usb_clrEPFIFO(USB_EP4);
					}
				}
#endif
				if (altSet) {
					DbgMsg_UVC(("Aud[%d] Start[%d/%d]:evt=0x%x\r\n", i, intf, altSet, uEvent));

					#if ISF_AUDIO_LATENCY_DEBUG
					UVAC_DbgDmp_Aud_TimestampNumReset();
					#endif

					txfQueIdx = i + UVAC_VID_DEV_CNT_MAX;//UVAC_TXF_QUE_A1, UVAC_TXF_QUE_A2
					//U2UVAC_RemoveTxfInfo(txfQueIdx);

					loc_cpu();
					UVAC_ResetTxfInfo(txfQueIdx);
					gUacAudStart[i] = TRUE;
					unl_cpu();
					found = TRUE;

					#if 1
					if (g_fpStartAudio) {
						memset((void *)&gAudStrmInfo[i], 0, sizeof(UVAC_STRM_INFO));
						gAudStrmInfo[i].strmPath = i;
						gAudStrmInfo[i].isAudStrmOn = TRUE;
						g_fpStartAudio(i, &gAudStrmInfo[i]);
					}
					break;
					#endif
				} else {
					DbgMsg_UVC(("Aud[%d] Stop[%d/%d]:evt=0x%x, Ori=%d\r\n", i, intf, altSet, uEvent, gUacAudStart[i]));
					loc_cpu();
					//tmpV = gUacAudStart[i];
					gUacAudStart[i] = FALSE;
					unl_cpu();
					txfQueIdx = i + UVAC_VID_DEV_CNT_MAX;//UVAC_TXF_QUE_A1, UVAC_TXF_QUE_A2
					#if 1
					if (g_fpStartAudio) {
						gAudStrmInfo[i].isAudStrmOn = FALSE;
						g_fpStartAudio(i, &gAudStrmInfo[i]);
					}
					#endif

					usb3dev_abortEndpoint(UVAC_USB_EP[txfQueIdx]);

					//_UAC_Stop(i);
					UVAC_RemoveTxfInfo(txfQueIdx);
					if (i == UVAC_AUD_DEV_CNT_1) {
						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A1);
					} else {
						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A2);
					}
					found = TRUE;
					break;
				}
			}
		}
	}
	if (FALSE == found) {
		DBG_DUMP("!!Unknown CB evt=0x%x, Intf=%d, alt=%d,uvac channel=%d\r\n", uEvent, intf, altSet, gUvacChannel);
		return;
	}
}

/*
    UVAC_OpenNeededFIFO
*/
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
static void UVAC_OpenNeededFIFO(void)
{
	USBCOMPAT_MASKEPINT(USB_EP1);
	USBCOMPAT_MASKEPINT(USB_EP3);
	if (gUvacCdcEnabled) {
#if 0//USB_CDC_IF_COMM_NUMBER_EP
		usb_maskEPINT(CDC_COMM_IN_EP);
		usb_maskEPINT(CDC_COMM2_IN_EP);
#endif
		USBCOMPAT_MASKEPINT(CDC_DATA_IN_EP);
		USBCOMPAT_MASKEPINT(CDC_DATA2_IN_EP);
		USBCOMPAT_UNMASKEPINT(CDC_DATA_OUT_EP);
		USBCOMPAT_UNMASKEPINT(CDC_DATA2_OUT_EP);
	}
	if (gUvacMtpEnabled) {
		USBCOMPAT_MASKEPINT(SIDC_BULKIN_EP);
		USBCOMPAT_MASKEPINT(SIDC_INTRIN_EP);
		USBCOMPAT_UNMASKEPINT(SIDC_BULKOUT_EP);
	}
}
void UVC_SetTestImg(UINT32 imgAddr, UINT32 imgSize)
{
	DbgMsg_UVC(("imgAddr=0x%x, imgSize=0x%x\r\n", __func__, imgAddr, imgSize));
	gUvcCapImgAddr = imgAddr;
	gUvcCapImgSize = imgSize;
	gUvacStillImgTrigSts = UVC_STILLIMG_TRIG_CTRL_TRANSMIT_BULK;
	gUvcUsbDMAAbord = FALSE;
	UVC_TrigStilImg(gUvcCapImgAddr, gUvcCapImgSize);
}
static void UVAC_Callback(uintptr_t event)
{
	UINT32 uEvent = (UINT32)event;

	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, uEvent));
	switch (uEvent) {
	case USB_EVENT_OUT0:
		set_flg(FLG_ID_UVAC, FLGUVAC_OUT0);
		break;
	case USB_EVENT_EP1_TX: // same as USB_EVENT_BULKIN1:
		set_flg(FLG_ID_UVAC, FLGUVAC_IN1);
		break;
	case USB_EVENT_EP1TX0BYTE:
		set_flg(FLG_ID_UVAC, FLGUVAC_EP1TX0BYTE);
		break;
	case USB_EVENT_EP10_RX:
		set_flg(FLG_ID_UVAC, FLGUVAC_CDC_DATA_OUT);
		USBCOMPAT_MASKEPINT(CDC_DATA_OUT_EP);
		break;
	case USB_EVENT_EP13_RX:
		set_flg(FLG_ID_UVAC, FLGUVAC_CDC_DATA2_OUT);
		USBCOMPAT_MASKEPINT(CDC_DATA2_OUT_EP);
		break;
	case USB_EVENT_EP12_RX:
		//set_flg(FLG_ID_SIDC, FLGUVAC_SIDC_BULKOUT2);
		USBCOMPAT_MASKEPINT(SIDC_BULKOUT_EP);
		break;
	default:
		break;
	}
}
#elif (UVAC_CONTROLLER == UVAC_CONTROL_U3)
void U3UVC_SetTestImg(UINT32 imgAddr, UINT32 imgSize)
{
	DbgMsg_UVC(("imgAddr=0x%x, imgSize=0x%x\r\n", __func__, imgAddr, imgSize));
	gUvcCapImgAddr = imgAddr;
	gUvcCapImgSize = imgSize;
	gUvacStillImgTrigSts = UVC_STILLIMG_TRIG_CTRL_TRANSMIT_BULK;
	gUvcUsbDMAAbord = FALSE;
	UVC_TrigStilImg(gUvcCapImgAddr, gUvcCapImgSize);
}

extern const USB_EP UVAC_USB_INTR_EP[UVAC_EP_INTR_MAX];


static void UVAC_OpenNeededFIFO(uintptr_t event)
{
	usb3dev_setInterruptEnable(UVAC_USB_INTR_EP[UVAC_EP_INTR_V1], DISABLE);

	if (g_u3_hid_info.en) {
		usb3dev_setInterruptEnable(HID_INTRIN_EP, DISABLE);
		if (g_u3_hid_info.intr_out) {
			usb3dev_setInterruptEnable(HID_INTROUT_EP, ENABLE);
		}
	}
	if (gUvacCdcEnabled[CDC_COM_1ST]) {
#if USB_CDC_IF_COMM_NUMBER_EP
		usb3dev_setInterruptEnable(CDC_COMM_IN_EP, DISABLE);
#endif
		usb3dev_setInterruptEnable(CDC_DATA_IN_EP, DISABLE);
		usb3dev_setInterruptEnable(CDC_DATA_OUT_EP, ENABLE);
	}
	if (gUvacCdcEnabled[CDC_COM_2ND]) {
#if USB_CDC_IF_COMM_NUMBER_EP
		usb3dev_setInterruptEnable(CDC_COMM2_IN_EP, DISABLE);
#endif
		usb3dev_setInterruptEnable(CDC_DATA2_IN_EP, DISABLE);
		usb3dev_setInterruptEnable(CDC_DATA2_OUT_EP, ENABLE);
	}


	if (gUvacMtpEnabled) {
		//usb3dev_setInterruptEnable(SIDC_BULKIN_EP, DISABLE);
		//usb3dev_setInterruptEnable(SIDC_INTRIN_EP, DISABLE);
		//usb3dev_setInterruptEnable(SIDC_BULKOUT_EP, ENABLE);
	}

	if (gUvacUacRxEnabled) {
		usb3dev_setInterruptEnable(UVAC_USB_RX_EP[0], ENABLE);
	}

	if (g_u3_msdc_info.en) {
		usb3dev_setInterruptEnable(msdc_in_ep, DISABLE);
		usb3dev_setInterruptEnable(msdc_out_ep, ENABLE);
	}
}

static void UVAC_Callback(uintptr_t event)
{
	UINT32 uEvent = (UINT32)event;

	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, uEvent));
	switch (uEvent) {

	case USB3_EVENT_EP8OUT:
		if (gUvacCdcEnabled[CDC_COM_1ST]){
			usb3dev_setInterruptEnable(CDC_DATA_OUT_EP, DISABLE);
			set_flg(FLG_ID_UVAC, FLGUVAC_CDC_DATA_OUT);
			break;
		}
	case USB3_EVENT_EP7OUT:
		if (gUvacUacRxEnabled) {
			usb3dev_setInterruptEnable(UVAC_USB_RX_EP[0], DISABLE);
			set_flg(FLG_ID_UVAC_UAC_RX, FLGUVAC_AUD_DATA_OUT);
		} else if (g_u3_hid_info.en && g_u3_hid_info.intr_out) {
			usb3dev_setInterruptEnable(HID_INTROUT_EP, DISABLE);
			set_flg(FLG_ID_UVAC, FLGUVAC_HID_DATA_OUT);
			break;
		}
	case USB3_EVENT_CLRFEATURE:
        if (u3_uvc_bulk_mode[0]) {
			//gUvcVidStart[0] = FALSE;
			//_U3UVC_Stop(0);
		}
		if (u3_uvc_bulk_mode[1]) {
			// gUvcVidStart[1] = FALSE;
			//_U3UVC_Stop(1);
		}
		if (g_u3_msdc_info.en && g_u3uvac_msdc_device_evt_cb) {
			g_u3uvac_msdc_device_evt_cb(uEvent);
		}
		break;
	case USB3_EVENT_DISCONNECT:
#if 0
		_U3UVC_Stop(UVAC_VID_DEV_CNT_1);
		UVAC_RemoveTxfInfo(UVAC_TXF_QUE_V1);
		usb3dev_abortEndpoint(UVAC_USB_EP[UVAC_TXF_QUE_V1]);

		_U3UVC_Stop(UVAC_VID_DEV_CNT_2);
		UVAC_RemoveTxfInfo(UVAC_TXF_QUE_V2);
		usb3dev_abortEndpoint(UVAC_USB_EP[UVAC_TXF_QUE_V2]);

		_U3UVC_Stop(UVAC_VID_DEV_CNT_3);
		UVAC_RemoveTxfInfo(UVAC_TXF_QUE_V3);
		usb3dev_abortEndpoint(UVAC_USB_EP[UVAC_TXF_QUE_V3]);

		_UAC_Stop(UVAC_AUD_DEV_CNT_1);
		_UAC_Stop(UVAC_AUD_DEV_CNT_2);
#endif
		break;
	case USB3_EVENT_CONNECT:
		break;
	default:
		if (g_u3_msdc_info.en && g_u3uvac_msdc_device_evt_cb) {
			g_u3uvac_msdc_device_evt_cb(uEvent);
		}
		break;
	}
}

#if SUSPEND_RESUME_FUNC
static void UVAC_suspend_cb(uintptr_t event)
{


	gUvcUsbDMAAbord = TRUE;
	//gUacUsbDMAAbord = TRUE;
	if (usb3dev_chkEPBusy(UVAC_USB_EP[UVAC_TXF_QUE_A1], U3DEV_EPDIR_IN)) {
		usb3dev_abortEndpoint(UVAC_USB_EP[UVAC_TXF_QUE_A1]);
	}

	UVAC_StopAll();

	if (g_fpSuspend_CB) {
		g_fpSuspend_CB();
	}
}
#endif

#endif

void U3UVAC_SetEachStrmInfo(PUVAC_STRM_FRM pStrmFrm)
{
	UINT32 devIdx = 0;
	UINT32 tmpIdx = 0;
	UVAC_VID_STRM_INFO *pStrmQueInfo = 0;
	UVAC_STRM_PATH strmPath;
	UVAC_TXF_INFO txfInfo;
	UINT32 uvac_channel = 0;

	if (g_usb_speed == U3DEV_SPEED_HS && (gHsUvacChannel != 0)) {
		uvac_channel = gHsUvacChannel;
	} else {
		uvac_channel = gUvacChannel;
	}

	if (FALSE == gUvacOpened)
		return;

	if (0 == pStrmFrm) {
		DBG_ERR("Input pStrmFrm NULL\r\n");
		return;
	}
	strmPath = pStrmFrm->path;
	if ((m_uiUvacDbgVid & UVAC_DBG_VID_RCV_ALL)) {
		DBG_DUMP("Strm[%d]=0x%x,%d\r\n", pStrmFrm->path, pStrmFrm->addr, pStrmFrm->size);
		DBG_DUMP("UVAC-Ch=%d,v1=%d,a1=%d,v2=%d,a2=%d, abort=%d %d\r\n", uvac_channel, gUvcVidStart[0], gUacAudStart[0], gUvcVidStart[1], gUacAudStart[1], gUvcUsbDMAAbord, gUvcNoVidStrm);
	}
	if (0 == pStrmFrm->size || 0 == pStrmFrm->addr) {
		DBG_WRN("[%d]zero stream:0x%x,0x%x\r\n", strmPath, pStrmFrm->addr, pStrmFrm->size);
		return;
	}

	loc_cpu();
	if ((TRUE == gUvcVidStart[UVAC_VID_DEV_CNT_1]) && (UVAC_STRM_VID == strmPath)) {
		vos_flag_clr(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V1);
		unl_cpu();
		devIdx = UVAC_VID_DEV_CNT_1;
		if (pStrmFrm->size > gVidTxBuf[devIdx].size*98/100) { //keep for payload header
			DBG_WRN("skip! frame(%dKB)> TxBuf (%dKB)\r\n", pStrmFrm->size/1024, gVidTxBuf[devIdx].size*98/100/1024);
			vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V1);
			return;
		}
		#if defined(__LINUX_USER__)
		if (0 == gUvcVidBuf[devIdx].size && 0 == pStrmFrm->va) {
			DBG_ERR("UVAC_CONFIG_VID_BUF_INFO not configured!\r\n");
		}
		#endif
		pStrmQueInfo = &gUvacVidStrmInfo[devIdx];
		tmpIdx = pStrmQueInfo->idxProducer;
		DbgMsg_UVCIO(("VidQue[%d] new[%d]=0x%x, 0x%x\r\n", devIdx, tmpIdx, pStrmFrm->addr, pStrmFrm->size));
		if (((tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX) == pStrmQueInfo->idxConsumer) {
			DBG_ERR("VidQue[%d] is Full:%d, %d\r\n", devIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer);
		}
		if (pStrmFrm->va) {
			pStrmQueInfo->addr[tmpIdx] = pStrmFrm->va;
		} else {
			pStrmQueInfo->addr[tmpIdx] = uvac_pa_to_va(UVAC_BUF_TYPE_VID, pStrmFrm->addr);
		}

		pStrmQueInfo->size[tmpIdx] = pStrmFrm->size;
		pStrmQueInfo->idxProducer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;
		if (pStrmFrm->pStrmHdr) {
			if (gUvcVidBuf[UVAC_VID_DEV_CNT_1].pa) {
				pStrmQueInfo->strmInfo.pStrmHdr = (UINT8 *)uvac_pa_to_va(UVAC_BUF_TYPE_VID,  (uintptr_t)pStrmFrm->pStrmHdr);
			} else {
				pStrmQueInfo->strmInfo.pStrmHdr = (UINT8 *)pStrmFrm->pStrmHdr;
			}
		} else {
			pStrmQueInfo->strmInfo.pStrmHdr = 0;
		}

		pStrmQueInfo->strmInfo.strmHdrSize= pStrmFrm->strmHdrSize;
		pStrmQueInfo->timestamp = pStrmFrm->timestamp;
		pStrmQueInfo->uvc_timestamp_start = hd_gettime_us();
		pStrmQueInfo->frame_type[tmpIdx] = pStrmFrm->frame_type;

		if (uvc_direct_trigger) {
			set_flg(FLG_ID_UVAC, FLGUVAC_VIDEO_TXF);
		}
		if (m_uiUvacDbgVid & UVAC_DBG_VID_RCV_V1) {
			DBG_DUMP("V[0][%d]=0x%x,%d,type=%d\r\n", tmpIdx, pStrmFrm->addr, pStrmFrm->size, pStrmFrm->frame_type);
		}
	} else if ((TRUE == gUacAudStart[UVAC_AUD_DEV_CNT_1]) && (UVAC_STRM_AUD == strmPath)) {
		VOS_TICK cur_set_each = 0;

		vos_flag_clr(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A1);
		unl_cpu();

		vos_perf_mark(&cur_set_each);

		if (pStrmFrm->size == 0) {
			DBG_WRN("A[%d] set zero frame addr=0x%llx, size=%d\r\n", UVAC_AUD_DEV_CNT_1, pStrmFrm->addr, pStrmFrm->size);
		}

		txfInfo.sAddr = txfInfo.oriAddr = pStrmFrm->addr;
		txfInfo.size = pStrmFrm->size;
		txfInfo.usbEP = UVAC_USB_EP[UVAC_TXF_QUE_A1];
		txfInfo.streamType = UVAC_TXF_STREAM_AUD;
		txfInfo.timestamp = pStrmFrm->timestamp;
		txfInfo.uvc_timestamp_start = hd_gettime_us();
		DbgMsg_UVCIO(("^YA-EP%d:0x%x\r\n", txfInfo.usbEP, txfInfo.size));
		UVAC_AddIsoInTxfInfo(&txfInfo, UVAC_TXF_QUE_A1);
		if (m_uiUvacDbgVid & UVAC_DBG_VID_RCV_A1) {
			DBG_DUMP("A[0]0x%x, %d, t=%d, q=%d\r\n", pStrmFrm->addr, pStrmFrm->size, cur_set_each-last_set_each[UVAC_TXF_QUE_A1], gUvacTxfQueCurCnt[UVAC_TXF_QUE_A1]);
		}
		last_set_each[UVAC_TXF_QUE_A1] = cur_set_each;
	} else if ((TRUE == gUvcVidStart[UVAC_VID_DEV_CNT_2]) && (UVAC_STRM_VID2 == strmPath)) {
		vos_flag_clr(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V2);
		unl_cpu();
		devIdx = UVAC_VID_DEV_CNT_2;
		if (pStrmFrm->size > gVidTxBuf[devIdx].size*98/100) { //keep for payload header
			DBG_WRN("skip! frame(%dKB)> TxBuf (%dKB)\r\n", pStrmFrm->size/1024, gVidTxBuf[devIdx].size*98/100/1024);
			vos_flag_clr(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V2);
			return;
		}
		#if defined(__LINUX_USER__)
		if (0 == gUvcVidBuf[devIdx].size && 0 == pStrmFrm->va) {
			DBG_ERR("UVAC_CONFIG_VID_BUF_INFO not configured!\r\n");
		}
		#endif

		if (uvac_channel == UVAC_CHANNEL_1V1A) {
			DBG_ERR("1V1A only\r\n");
			vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V2);
			return;
		}

		pStrmQueInfo = &gUvacVidStrmInfo[devIdx];

		tmpIdx = pStrmQueInfo->idxProducer;
		DbgMsg_UVCIO(("VidQue[%d] new[%d]=0x%x, 0x%x\r\n", devIdx, tmpIdx, pStrmFrm->addr, pStrmFrm->size));
		if (((tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX) == pStrmQueInfo->idxConsumer) {
			DBG_ERR("VidQue[%d] is Full:%d, %d\r\n", devIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer);
		}
		if (pStrmFrm->va) {
			pStrmQueInfo->addr[tmpIdx] = pStrmFrm->va;
		} else {
			pStrmQueInfo->addr[tmpIdx] = uvac_pa_to_va(UVAC_BUF_TYPE_VID2, pStrmFrm->addr);
		}
		pStrmQueInfo->size[tmpIdx] = pStrmFrm->size;
		pStrmQueInfo->idxProducer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;
		if (pStrmFrm->pStrmHdr) {
			if (gUvcVidBuf[UVAC_VID_DEV_CNT_2].pa) {
				pStrmQueInfo->strmInfo.pStrmHdr = (UINT8 *)uvac_pa_to_va(UVAC_BUF_TYPE_VID2,  (uintptr_t)pStrmFrm->pStrmHdr);
			} else {
				pStrmQueInfo->strmInfo.pStrmHdr = (UINT8 *)pStrmFrm->pStrmHdr;
			}
		} else {
			pStrmQueInfo->strmInfo.pStrmHdr = 0;
		}
		pStrmQueInfo->strmInfo.strmHdrSize= pStrmFrm->strmHdrSize;
		pStrmQueInfo->timestamp = pStrmFrm->timestamp;
		pStrmQueInfo->uvc_timestamp_start = hd_gettime_us();
		pStrmQueInfo->frame_type[tmpIdx] = pStrmFrm->frame_type;

		if (uvc_direct_trigger) {
			set_flg(FLG_ID_UVAC, FLGUVAC_VIDEO2_TXF);
		}
		if (m_uiUvacDbgVid & UVAC_DBG_VID_RCV_V2) {
			DBG_DUMP("V[1][%d]=0x%x,%d\r\n", tmpIdx, pStrmFrm->addr, pStrmFrm->size);
		}
	} else if ((TRUE == gUacAudStart[UVAC_AUD_DEV_CNT_2]) && (UVAC_STRM_AUD2 == strmPath)) {
		VOS_TICK cur_set_each = 0;

		vos_flag_clr(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A2);
		unl_cpu();

		vos_perf_mark(&cur_set_each);

		if (uvac_channel != UVAC_CHANNEL_2V2A) {
			DBG_ERR("UVAC_STRM_AUD2 only for UVAC_CHANNEL_2V2A\r\n");
			return;
		}
		if (pStrmFrm->size == 0) {
			DBG_WRN("A[%d] set zero frame addr=0x%llx, size=%d\r\n", UVAC_AUD_DEV_CNT_2, pStrmFrm->addr, pStrmFrm->size);
		}

		txfInfo.sAddr = txfInfo.oriAddr = pStrmFrm->addr;
		txfInfo.size = pStrmFrm->size;
		txfInfo.usbEP = UVAC_USB_EP[UVAC_TXF_QUE_A2];
		txfInfo.streamType = UVAC_TXF_STREAM_AUD;
		txfInfo.timestamp = pStrmFrm->timestamp;
		txfInfo.uvc_timestamp_start = hd_gettime_us();
		DbgMsg_UVCIO(("^Y2A-EP%d:0x%x\r\n", txfInfo.usbEP, txfInfo.size));
		UVAC_AddIsoInTxfInfo(&txfInfo, UVAC_TXF_QUE_A2);
		if (m_uiUvacDbgVid & UVAC_DBG_VID_RCV_A2) {
			DBG_DUMP("A[1]0x%x, %d, t=%d, q=%d\r\n", pStrmFrm->addr, pStrmFrm->size, cur_set_each-last_set_each[UVAC_TXF_QUE_A2], gUvacTxfQueCurCnt[UVAC_TXF_QUE_A2]);
		}
		last_set_each[UVAC_TXF_QUE_A2] = cur_set_each;
	} else if ((TRUE == gUvcVidStart[UVAC_VID_DEV_CNT_3]) && (UVAC_STRM_VID3 == strmPath)) {
		vos_flag_clr(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V3);
		unl_cpu();
		devIdx = UVAC_VID_DEV_CNT_3;
		if (pStrmFrm->size > gVidTxBuf[devIdx].size*98/100) { //keep for payload header
			DBG_WRN("skip! frame(%dKB)> TxBuf (%dKB)\r\n", pStrmFrm->size/1024, gVidTxBuf[devIdx].size*98/100/1024);
			vos_flag_clr(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V3);
			return;
		}
		#if defined(__LINUX_USER__)
		if (0 == gUvcVidBuf[devIdx].size && 0 == pStrmFrm->va) {
			DBG_ERR("UVAC_CONFIG_VID_BUF_INFO not configured!\r\n");
		}
		#endif

		if (uvac_channel < UVAC_CHANNEL_3V1A) {
			DBG_ERR("1V1A only\r\n");
			vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V3);
			return;
		}

		pStrmQueInfo = &gUvacVidStrmInfo[devIdx];

		tmpIdx = pStrmQueInfo->idxProducer;
		DbgMsg_UVCIO(("VidQue[%d] new[%d]=0x%x, 0x%x\r\n", devIdx, tmpIdx, pStrmFrm->addr, pStrmFrm->size));
		if (((tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX) == pStrmQueInfo->idxConsumer) {
			DBG_ERR("VidQue[%d] is Full:%d, %d\r\n", devIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer);
		}
		if (pStrmFrm->va) {
			pStrmQueInfo->addr[tmpIdx] = pStrmFrm->va;
		} else {
			pStrmQueInfo->addr[tmpIdx] = uvac_pa_to_va(UVAC_BUF_TYPE_VID3, pStrmFrm->addr);
		}
		pStrmQueInfo->size[tmpIdx] = pStrmFrm->size;
		pStrmQueInfo->idxProducer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;
		if (pStrmFrm->pStrmHdr) {
			if (gUvcVidBuf[UVAC_VID_DEV_CNT_3].pa) {
				pStrmQueInfo->strmInfo.pStrmHdr = (UINT8 *)uvac_pa_to_va(UVAC_BUF_TYPE_VID3,  (uintptr_t)pStrmFrm->pStrmHdr);
			} else {
				pStrmQueInfo->strmInfo.pStrmHdr = (UINT8 *)pStrmFrm->pStrmHdr;
			}
		} else {
			pStrmQueInfo->strmInfo.pStrmHdr = 0;
		}
		pStrmQueInfo->strmInfo.strmHdrSize= pStrmFrm->strmHdrSize;
		pStrmQueInfo->timestamp = pStrmFrm->timestamp;
		pStrmQueInfo->uvc_timestamp_start = hd_gettime_us();
		pStrmQueInfo->frame_type[tmpIdx] = pStrmFrm->frame_type;

		if (uvc_direct_trigger) {
			set_flg(FLG_ID_UVAC_UVC3, FLGUVAC_VIDEO3_TXF);
		}
		if (m_uiUvacDbgVid & UVAC_DBG_VID_RCV_V3) {
			DBG_DUMP("V[1][%d]=0x%x,%d\r\n", tmpIdx, pStrmFrm->addr, pStrmFrm->size);
		}
	}else {
		unl_cpu();
		//DbgMsg_UVCIO(("Strm=%d,en=%d,%d,%d,%d\r\n", strmPath, gUvcVidStart[UVAC_VID_DEV_CNT_1], gUvcVidStart[UVAC_VID_DEV_CNT_2], gUacAudStart[UVAC_AUD_DEV_CNT_1], gUacAudStart[UVAC_AUD_DEV_CNT_2]));
	}
	//DbgMsg_UVCIO(("-%s:type=%d,enable=%d,%d,%d,%d\r\n", __func__, strmPath, gUvcVidStart[UVAC_VID_DEV_CNT_1], gUvcVidStart[UVAC_VID_DEV_CNT_2], gUacAudStart[UVAC_AUD_DEV_CNT_1], gUacAudStart[UVAC_AUD_DEV_CNT_2]));
}

static void UVCUtil_UpdatePts(void)
{
    gUVCHeadPTS = hd_gettime_us()*(UVC_VC_CLOCK_FREQUENCY/1000000);
}

static UINT32 UVCUtil_MakeHeadScrPts(UINT8 *pH264DecAddr, UINT32 fid)
{
	*pH264DecAddr = PAYLOAD_LEN;
	*(pH264DecAddr + 1) = PAYLOAD_SCR | PAYLOAD_PTS | PAYLOAD_EOH;

	if (fid) {
		*(pH264DecAddr + 1) |= PAYLOAD_FID;
	}

	*(pH264DecAddr + 2) = gUVCHeadPTS & 0xFF;
	*(pH264DecAddr + 3) = (gUVCHeadPTS >> 8) & 0xFF;
	*(pH264DecAddr + 4) = (gUVCHeadPTS >> 16) & 0xFF;
	*(pH264DecAddr + 5) = (gUVCHeadPTS >> 24) & 0xFF;

	*(pH264DecAddr + 6) = gUVCHeadSCR & 0xFF;
	*(pH264DecAddr + 7) = (gUVCHeadSCR >> 8) & 0xFF;
	*(pH264DecAddr + 8) = (gUVCHeadSCR >> 16) & 0xFF;
	*(pH264DecAddr + 9) = (gUVCHeadSCR >> 24) & 0xFF;

#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
	gSOFTokenNum = usb_getSOF();
#else
	gSOFTokenNum = usb3dev_getSOF();
#endif

	*(pH264DecAddr + 10) = gSOFTokenNum & 0xFF;
	*(pH264DecAddr + 11) = (gSOFTokenNum >> 8) & 0xFF;

	//gUVCHeadPTS += 0xF3BBA;
	gUVCHeadSCR += 3940;
	return PAYLOAD_LEN;
}

ER U3UVAC_ConfigVidReso(PUVAC_VID_RESO pVidReso, UINT32 cnt)
{
	ER retV = E_OK;
	DbgMsg_UVC(("+ %s:pVidReso=0x%x cnt=%d, limit=%d\r\n", __func__, pVidReso, cnt, UVAC_VID_RESO_MAX_CNT));
	if ((UVAC_VID_RESO_MAX_CNT >= cnt) && pVidReso && cnt) {
		memset((void *)gUvcVidResoAry, 0, (sizeof(UVAC_VID_RESO)* UVAC_VID_RESO_MAX_CNT));
		memcpy((void *)gUvcVidResoAry, pVidReso, (sizeof(UVAC_VID_RESO)* cnt));
		gUvcVidResoCnt = cnt;
		//just for backwark compatible
		gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt= cnt;
		gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = gUvcVidResoAry;

#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
		{
			UINT32 i = 0, j = 0;
			for (i = 0; i < gUvcVidResoCnt; i++) {
				DBG_DUMP("VidReso[%d]=%d, %d, fpsCnt=%d: \r\n", i, gUvcVidResoAry[i].width, gUvcVidResoAry[i].height, gUvcVidResoAry[i].fpsCnt);
				for (j = 0; j < UVAC_VID_RESO_FPS_MAX_CNT; j++) {
					DBG_DUMP("  fps[%d]=%d", j, gUvcVidResoAry[i].fps[j]);
				}
				DBG_DUMP("\r\n");
			}
			if (i < UVAC_VID_RESO_MAX_CNT) {
				DBG_DUMP("VidReso[%d]=%d, %d, fpsCnt=%d: \r\n", i, gUvcVidResoAry[i].width, gUvcVidResoAry[i].height, gUvcVidResoAry[i].fpsCnt);
				for (j = 0; j < UVAC_VID_RESO_FPS_MAX_CNT; j++) {
					DBG_DUMP("  fps[%d]=%d", j, gUvcVidResoAry[i].fps[j]);
				}
				DBG_DUMP("\r\n");
			}
		}
#endif

	} else {
		DBG_ERR("pVidReso=0x%lx NULL -or- cnt=%d, max = %d\r\n", (uintptr_t)pVidReso, cnt, UVAC_VID_RESO_MAX_CNT);
		retV = E_SYS;
	}
	DbgMsg_UVC(("- %s:retV=0x%x cnt=%d, limit=%d\r\n", __func__, retV, cnt, UVAC_VID_RESO_MAX_CNT));
	return retV;
}
void U3UVAC_SetConfig(UVAC_CONFIG_ID ConfigID, ULONG Value)
{
	//DBG_DUMP("ConfigID=%d, Value=0x%x, opened=%d\r\n",ConfigID, Value, gUvacOpened);
	#if 0
	if (gUvacOpened) {
		if (UVAC_CONFIG_MFK_REC2LIVEVIEW == ConfigID) {
			gUvcNoVidStrm = TRUE;
		} else if (UVAC_CONFIG_MFK_LIVEVIEW2REC == ConfigID) {
			UINT32 i;
			gUvcNoVidStrm = FALSE;
			for (i = UVAC_VID_DEV_CNT_1; i < UVAC_VID_DEV_CNT_MAX; i++) {
				if (TRUE == gUvcVidStart[i]) {
					UVAC_Start(i);
				}
			}
		} else {
			DBG_ERR("UVAC is opened, Can Not set ConfigID=%d, Value=0x%x\r\n", ConfigID, Value);
		}
		return;
	}
	#endif
	switch (ConfigID) {
	case UVAC_CONFIG_AUD_FMT_TYPE:
		if (UVAC_AUD_FMT_TYPE_PCM == Value) {
			gUvacAudFmtType = UAC_FMT_TYPE_I;
		} else {
			gUvacAudFmtType = UAC_FMT_TYPE_II;
		}
		break;
	case UVAC_CONFIG_AUD_SAMPLERATE: {
			PUVAC_AUD_SAMPLERATE_ARY pAudAry = (PUVAC_AUD_SAMPLERATE_ARY)Value;
			if (pAudAry && pAudAry->pAudSampleRateAry) {
				UINT32 maxCnt = 0;
				memset((void *)&gUvacAudSampleRate[0], 0, UVAC_AUD_SAMPLE_RATE_MAX_CNT * sizeof(UINT32));
				maxCnt = (pAudAry->aryCnt > UVAC_AUD_SAMPLE_RATE_MAX_CNT) ? UVAC_AUD_SAMPLE_RATE_MAX_CNT : pAudAry->aryCnt;
				memcpy((void *)&gUvacAudSampleRate[0], (void *)pAudAry->pAudSampleRateAry, maxCnt * sizeof(UINT32));
			} else {
				DBG_ERR("no pAudSampleRateAry\r\n");
			}
		}
		break;
	case UVAC_CONFIG_WINUSB_ENABLE:
		gUvacWinIntrfEnable = (BOOL)Value;
		break;
	case UVAC_CONFIG_WINUSB_CB:
		gUvcWinUSBReqCB = (UVAC_WINUSBCLSREQCB)Value;
		break;
	case UVAC_CONFIG_VEND_DEV_DESC:
		if (0 == Value) {
			DBG_ERR("ConfigID=%d, Value NULL\r\n", ConfigID);
			return;
		}
		gpImgUnitUvacVendDevDesc = (UVAC_VEND_DEV_DESC *)Value;
		UVAC_DbgDmp_VendDevDesc(gpImgUnitUvacVendDevDesc);
		if (gpImgUnitUvacVendDevDesc->VID != 0) {
			g_UVACDevDesc.id_vendor = gpImgUnitUvacVendDevDesc->VID;
			g_UVACSSDevDesc.id_vendor = gpImgUnitUvacVendDevDesc->VID;
		}
		if (gpImgUnitUvacVendDevDesc->PID != 0) {
			g_UVACDevDesc.id_product = gpImgUnitUvacVendDevDesc->PID;
			g_UVACSSDevDesc.id_product = gpImgUnitUvacVendDevDesc->PID;
		}
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
		if ((gpImgUnitUvacVendDevDesc->pManuStringDesc != NULL) && (gpImgUnitUvacVendDevDesc->pManuStringDesc->bDescriptorType == 0x03)) {
			USBMng.pStringDesc[1] = (USB_STRING_DESC *)(gpImgUnitUvacVendDevDesc->pManuStringDesc);
			UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)USBMng.pStringDesc[1]);
		}
		if ((gpImgUnitUvacVendDevDesc->pProdStringDesc != NULL) && (gpImgUnitUvacVendDevDesc->pProdStringDesc->bDescriptorType == 0x03)) {
			USBMng.pStringDesc[2] = (USB_STRING_DESC *)(gpImgUnitUvacVendDevDesc->pProdStringDesc);
			UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)USBMng.pStringDesc[2]);
		}
		if ((gpImgUnitUvacVendDevDesc->pSerialStringDesc != NULL) && (gpImgUnitUvacVendDevDesc->pSerialStringDesc->bDescriptorType == 0x03)) {
			USBMng.pStringDesc[3] = (USB_STRING_DESC *)(gpImgUnitUvacVendDevDesc->pSerialStringDesc);
			UVAC_DbgDmp_StrDesc((UVAC_STRING_DESC *)USBMng.pStringDesc[3]);
		}
#else
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING1, (void *)gpImgUnitUvacVendDevDesc->pManuStringDesc);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING1, (void *)gpImgUnitUvacVendDevDesc->pManuStringDesc);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING1, (void *)gpImgUnitUvacVendDevDesc->pManuStringDesc);
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING2, (void *)gpImgUnitUvacVendDevDesc->pProdStringDesc);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING2, (void *)gpImgUnitUvacVendDevDesc->pProdStringDesc);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING2, (void *)gpImgUnitUvacVendDevDesc->pProdStringDesc);
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING3, (void *)gpImgUnitUvacVendDevDesc->pSerialStringDesc);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING3, (void *)gpImgUnitUvacVendDevDesc->pSerialStringDesc);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING3, (void *)gpImgUnitUvacVendDevDesc->pSerialStringDesc);
#endif
		if (0 == gpImgUnitUvacVendDevDesc->fpIQVendReqCB) {
			DBG_IND("IQ VendReqCB NULL\r\n");
		}
		if (0 == gpImgUnitUvacVendDevDesc->fpVendReqCB) {
			DBG_IND("VendReqCB NULL\r\n");
		}
		break;
	case UVAC_CONFIG_UVAC_CAP_M3:
		gUvcCapM3Enable = (UINT32)Value;
		break;
	case UVAC_CONFIG_H264_TARGET_SIZE:
		gUvcH264TBR = (UINT32)Value;
		break;
	case UVAC_CONFIG_MJPG_TARGET_SIZE:
		gUvcMJPGTBR = (UINT32)Value;
		break;
	case UVAC_CONFIG_VENDOR_CALLBACK:
		g_fpUvacVendorReqCB = (UVAC_VENDOR_REQ_CB)Value;
		break;
	case UVAC_CONFIG_VENDOR_IQ_CALLBACK:
		g_fpUvacVendorReqIQCB = (UVAC_VENDOR_REQ_CB)Value;
		break;
	case UVAC_CONFIG_MFK_LIVEVIEW2REC:
		gUvcNoVidStrm = FALSE;          //necessary for continue loopTxf
		break;
	case UVAC_CONFIG_EU_VENDCMDCB_ID01:
	case UVAC_CONFIG_EU_VENDCMDCB_ID02:
	case UVAC_CONFIG_EU_VENDCMDCB_ID03:
	case UVAC_CONFIG_EU_VENDCMDCB_ID04:
	case UVAC_CONFIG_EU_VENDCMDCB_ID05:
	case UVAC_CONFIG_EU_VENDCMDCB_ID06:
	case UVAC_CONFIG_EU_VENDCMDCB_ID07:
	case UVAC_CONFIG_EU_VENDCMDCB_ID08:
		gUvacEUVendCmdCB[ConfigID - UVAC_CONFIG_EU_VENDCMDCB_START] = (UVAC_EUVENDCMDCB)Value;
		break;
	case UVAC_CONFIG_VIDEO_FORMAT_TYPE:
		gUvcVideoFmtType[UVAC_VID_DEV_CNT_1] = (UVAC_VIDEO_FORMAT_TYPE)Value;
		break;
	case UVAC_CONFIG_VIDEO2_FORMAT_TYPE:
		gUvcVideoFmtType[UVAC_VID_DEV_CNT_2] = (UVAC_VIDEO_FORMAT_TYPE)Value;
		break;
	case UVAC_CONFIG_VIDEO3_FORMAT_TYPE:
		gUvcVideoFmtType[UVAC_VID_DEV_CNT_3] = (UVAC_VIDEO_FORMAT_TYPE)Value;
		break;
	case UVAC_CONFIG_CDC_ENABLE:
		gUvacCdcEnabled[CDC_COM_1ST] = Value;
		break;
	case UVAC_CONFIG_CDC2_ENABLE:
		gUvacCdcEnabled[CDC_COM_2ND] = Value;
		break;
	case UVAC_CONFIG_CDC_PSTN_REQUEST_CB:
		gfpCdcPstnReqCB = (UVAC_CDC_PSTN_REQUEST_CB)Value;
		break;
#if 0//gUvacMtpEnabled
	case UVAC_CONFIG_MTP_ENABLE:
		gUvacMtpEnabled = (BOOL)Value;
		UVAC_MTPEnable((UINT8)Value);
		break;
	case UVAC_CONFIG_MTP_DEVICE_FREIEDLYNAME:
		UVAC_MTPSetDeviceFriendlyName((char *)Value);
		break;
	case UVAC_CONFIG_MTP_VENDSPECI_DEVINFO:
		UVAC_MTPSetMTPVendorSpeciDevInfo((UINT8)Value);
		break;
	case UVAC_CONFIG_MTP_DATATX_CB:
		UVAC_SidcRegDataTxfCB((UVAC_USIDCDataTxf_CB)Value);
		break;
	case UVAC_CONFIG_PTP_DEVDATETIME_CB:
		UVAC_PTPRegistryDevDateTimeCallBack((UVAC_PTPDevDateTime_CB)Value);
		break;
	case UVAC_CONFIG_PTP_BATTERY_LEVEL:
		UVAC_PTPSetBatteryLevel((UINT8)Value);
		break;
	case UVAC_CONFIG_PTP_STORAGETYPE_CB:
		UVAC_PTPRegistryStorTypeCB((UVAC_PTPStorType_CB)Value);
		break;
	case UVAC_CONFIG_PTP_STORAGEID:
		UVAC_PTPSetStorageId((UINT32)Value);
		break;
#endif
	case UVAC_CONFIG_MAX_FRAME_SIZE:
		//add margin for BRC and payload header
		gUvcMaxVideoFmtSize = ALIGN_CEIL_32((UINT32)Value + (UINT32)Value * MAX_FRAME_SIZE_MARGIN / 100);
		if (gUvcMaxVideoFmtSize > gUvcMJPGMaxTBR) {
			gUvcMJPGMaxTBR = gUvcMaxVideoFmtSize;
		}
		break;
	case UVAC_CONFIG_AUD_CHANNEL_NUM:
		if (Value == 0) {
			gUacChNum = UAC_NUM_PHYSICAL_CHANNEL;
			gUacITOutChCfg = UAC_IT_OUT_CHANNEL_CONFIG;
			DBG_WRN("Invalid channel number = %d. Set to 2\r\n", (UINT32)Value);
		} else if (Value > 2) {
			gUacChNum = Value;
		} else {
			gUacChNum = (UINT32)Value;
			if (gUacChNum == 2) {
				gUacITOutChCfg = UAC_IT_OUT_CHANNEL_CONFIG;
			} else {
				gUacITOutChCfg = 0;
			}
		}
		break;
	case UVAC_CONFIG_EU_DESC:
		memcpy(&u3_eu_desc[0], (UVAC_EU_DESC *)Value, sizeof(UVAC_EU_DESC));
		UVAC_DbgDmp_EU_INFO(&u3_eu_desc[0]);
		if (u3_eu_desc[0].bUnitID < UVC_TERMINAL_ID_EXTE_UNIT) {
			DBG_ERR("Only support EU UnitID  >= %d\r\n", UVC_TERMINAL_ID_EXTE_UNIT);
			u3_eu_desc[0].bUnitID = UVC_TERMINAL_ID_EXTE_UNIT;
		}
		break;
	case UVAC_CONFIG_EU_DESC_2ND:
		memcpy(&u3_eu_desc[1], (UVAC_EU_DESC *)Value, sizeof(UVAC_EU_DESC));
		UVAC_DbgDmp_EU_INFO(&u3_eu_desc[1]);
		if (u3_eu_desc[1].bUnitID < UVC_TERMINAL_ID_EXTE_UNIT) {
			DBG_ERR("Only support EU UnitID  >= %d\r\n", UVC_TERMINAL_ID_EXTE_UNIT);
			u3_eu_desc[1].bUnitID = UVC_TERMINAL_ID_EXTE_UNIT;
		}
		break;

	case UVAC_CONFIG_EU_DESC_ARRAY:
		u3_eu_desc_array =  (UVAC_EU_DESC_ARRAY*)Value;
		break;

	case UVAC_CONFIG_HID_INFO:
		{
			UVAC_HID_INFO *p_user = (UVAC_HID_INFO *)Value;

			UVAC_DbgDmp_HID_INFO(p_user);
			if (p_user->en) {
				memcpy((void *)&g_u3_hid_info, (const void *)p_user, sizeof(UVAC_HID_INFO));
			} else {
				memset((void *)&g_u3_hid_info, 0, sizeof(UVAC_HID_INFO));
			}
			if (g_u3_hid_info.intr_interval == 0) {
				g_u3_hid_info.intr_interval = HID_EP_INTERRUPT_INTERVAL;
			}
		}
		break;
	case UVAC_CONFIG_CT_CB:
		g_fpU3UvcCT_CB = (UVAC_UNIT_CB)Value;
		break;
	case UVAC_CONFIG_PU_CB:
		g_fpU3UvcPU_CB = (UVAC_UNIT_CB)Value;
		break;
	case UVAC_CONFIG_VID_USER_DATA_SIZE:
		user_data_size = (UINT32)Value;
		break;
	case UVAC_CONFIG_AUD_START_CB:
		g_fpStartAudio = (UVAC_STARTAUDIOCB)Value;
		break;
	case UVAC_CONFIG_YUV_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC2_YUV_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC3_YUV_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC_MJPG_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC_H264_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC2_MJPG_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC2_H264_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_NV12_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC2_NV12_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC3_NV12_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_MJPG_DEF_FRM_IDX:
		gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = Value;
		break;
	case UVAC_CONFIG_H264_DEF_FRM_IDX:
		gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = Value;
		break;
	case UVAC_CONFIG_CT_CONTROLS:
		u3_ct_controls = Value;

		DBG_DUMP("set CT ctrl = %x\r\n", u3_ct_controls);
		break;
	case UVAC_CONFIG_PU_CONTROLS:
		u3_pu_controls = Value;


		DBG_DUMP("set PU ctrl = %x\r\n", u3_pu_controls);
		break;
	#if 0
	case UVAC_CONFIG_VID_BUF_INFO:
		{
			PUVAC_VID_BUF_INFO p_vid_buf = (PUVAC_VID_BUF_INFO)Value;

			if (p_vid_buf->vid_buf_pa) {
				gUvcVidBuf[UVAC_VID_DEV_CNT_1].pa = p_vid_buf->vid_buf_pa;
				gUvcVidBuf[UVAC_VID_DEV_CNT_1].size = p_vid_buf->vid_buf_size;
				gUvcVidBuf[UVAC_VID_DEV_CNT_1].va = (UINT32)hwmem_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_vid_buf->vid_buf_pa, p_vid_buf->vid_buf_size);
			}
			if (p_vid_buf->vid2_buf_pa) {
				gUvcVidBuf[UVAC_VID_DEV_CNT_2].pa = p_vid_buf->vid2_buf_pa;
				gUvcVidBuf[UVAC_VID_DEV_CNT_2].size = p_vid_buf->vid2_buf_size;
				gUvcVidBuf[UVAC_VID_DEV_CNT_2].va = (UINT32)hwmem_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_vid_buf->vid2_buf_pa, p_vid_buf->vid2_buf_size);
			}
			//printf("VID_BUF_INFO[0] pa=0x%X,size=0x%X,va=0x%X\r\n", gUvcVidBuf[UVAC_VID_DEV_CNT_1].pa, gUvcVidBuf[UVAC_VID_DEV_CNT_1].size,gUvcVidBuf[UVAC_VID_DEV_CNT_1].va);
			//printf("VID_BUF_INFO[1] pa=0x%X,size=0x%X,va=0x%X\r\n", gUvcVidBuf[UVAC_VID_DEV_CNT_2].pa, gUvcVidBuf[UVAC_VID_DEV_CNT_2].size,gUvcVidBuf[UVAC_VID_DEV_CNT_2].va);
		}
		break;
	#endif
	case UVAC_CONFIG_UAC_VOL_INFO:
		{
			UAC_VOL_INFO *p_user = (UAC_VOL_INFO *)Value;
			u3_uac_vol_cur = p_user->vol_def;
			u3_uac_vol_max = p_user->vol_max;
			u3_uac_vol_min = p_user->vol_min;
			u3_uac_vol_res = p_user->vol_res;
		}
		break;
	case UVAC_CONFIG_AUD_CHANNEL_CONFIG:
		gUacITOutChCfg = Value;
		break;
	case UVAC_CONFIG_AUD_RX_SAMPLERATE: {
			PUVAC_AUD_SAMPLERATE_ARY pAudAry = (PUVAC_AUD_SAMPLERATE_ARY)Value;
			if (pAudAry && pAudAry->pAudSampleRateAry) {
				UINT32 maxCnt = 0;
				memset((void *)&gUvacAudRxSampleRate[0], 0, UVAC_AUD_RX_SAMPLE_RATE_MAX_CNT * sizeof(UINT32));
				maxCnt = (pAudAry->aryCnt > UVAC_AUD_RX_SAMPLE_RATE_MAX_CNT) ? UVAC_AUD_RX_SAMPLE_RATE_MAX_CNT : pAudAry->aryCnt;
				memcpy((void *)&gUvacAudRxSampleRate[0], (void *)pAudAry->pAudSampleRateAry, maxCnt * sizeof(UINT32));
			} else {
				DBG_ERR("no pAudSampleRateAry\r\n");
			}
		}
		break;
	case UVAC_CONFIG_AUD_RX_CHANNEL_NUM:
		if (Value == 0 || Value > 2) {
			gUacRxChNum = UAC_NUM_PHYSICAL_CHANNEL;
			gUacRxITOutChCfg = UAC_IT_OUT_CHANNEL_CONFIG;
			DBG_WRN("Invalid channel number = %d. Set to 2\r\n", Value);
		} else {
			gUacRxChNum = Value;
			if (gUacRxChNum == 2) {
				gUacRxITOutChCfg = UAC_IT_OUT_CHANNEL_CONFIG;
			} else {
				gUacRxITOutChCfg = 0;
			}
		}
		break;
	case UVAC_CONFIG_UAC_RX_ENABLE:
		gUvacUacRxEnabled = (Value == 0)? FALSE : TRUE;
		break;
	case UVAC_CONFIG_UAC_RX_BLK_SIZE:
		gUvacUacBlockSize = ALIGN_CEIL(Value, UVAC_BUF_ALIGN);
		break;
	case UVAC_CONFIG_UAC_RX_BUF_SIZE:
		gUvacUacBufSize = ALIGN_CEIL(Value, UVAC_BUF_ALIGN);
		break;
	case UVAC_CONFIG_UAC_RX_VOL_CB:
		{
			gUacRxSetVolCB = (UVAC_SETVOLCB)Value;
		}
		break;

	case UVAC_CONFIG_UAC_RX_VOL_INFO:
		{
			UAC_VOL_INFO *p_user = (UAC_VOL_INFO *)Value;

			u3_uac_rx_vol_info.vol_def = p_user->vol_def,
			u3_uac_rx_vol_info.vol_max = p_user->vol_max,
			u3_uac_rx_vol_info.vol_min = p_user->vol_min,
			u3_uac_rx_vol_info.vol_res = p_user->vol_res;
		}
		break;
	case UVAC_CONFIG_MSDC_INFO:
		{
			UVAC_MSDC_INFO *p_user = (UVAC_MSDC_INFO *)Value;
			UVAC_DbgDmp_MSDC_INFO(p_user);
			if (p_user->en) {
				memcpy((void *)&g_u3_msdc_info, (const void *)p_user, sizeof(UVAC_MSDC_INFO));
			} else {
				memset((void *)&g_u3_msdc_info, 0, sizeof(UVAC_MSDC_INFO));
			}
		}
		break;
	case UVAC_CONFIG_DISABLE_UAC:
		gU3DisableUac = Value;
		break;
	case UVAC_CONFIG_UAC_PACKETSIZE:
		if (Value > 512) {
			DBG_ERR("Uac wMaxPacketSize(%d) should < 512\r\n", Value);
		} else {
			gU3UacMaxPacketSize = Value;
		}
		break;
	case UVAC_CONFIG_UAC_INTERVAL:
		gU3UacInterval = Value;
		break;

	case UVAC_CONFIG_XU_CTRL:
		g_fpU3UvcXU_CB = (UVAC_UNIT_CB)Value;
		break;
#if SUSPEND_RESUME_FUNC
	case UVAC_CONFIG_POWERDOWN_SUSPEND_CB:
		g_fpSuspend_CB = (UVAC_POWERDOWN_SUSPEND_CB)Value;
		break;
#endif
	case UVAC_CONFIG_UVC_STRING:
		p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_1] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UAC_STRING:
		p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_1] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UVC2_STRING:
		p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_2] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UAC2_STRING:
		p_u3_uac_string_desc[UVAC_AUD_DEV_CNT_2] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UVC3_STRING:
		p_u3_uvc_string_desc[UVAC_VID_DEV_CNT_3] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UVC_IAD_STRING:
		p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_1] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UAC_IAD_STRING:
		p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_1] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UVC2_IAD_STRING:
		p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_2] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UAC2_IAD_STRING:
		p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_2] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UVC3_IAD_STRING:
		p_u3_uvc_iad_string_desc[UVAC_VID_DEV_CNT_3] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UAC3_IAD_STRING:
		p_u3_uac_iad_string_desc[UVAC_AUD_DEV_CNT_3] = (USB_STRING_DESC *)Value;
		break;
	case UVAC_CONFIG_UVC_VER:
		gU3UvacUvcVer = Value;
		break;
	case UVAC_CONFIG_MSOS20_INFO:
		{
			UVAC_MSOS20_INFO *p_user = (UVAC_MSOS20_INFO *)Value;

			if (p_user->en) {
				memcpy((void *)&g_u3_msos_info, (const void *)p_user, sizeof(UVAC_MSOS20_INFO));
			} else {
				memset((void *)&g_u3_msos_info, 0, sizeof(UVAC_MSOS20_INFO));
			}
		}
		break;
	case UVAC_CONFIG_MAX_PAYLOAD_TX_SIZE:
		{
			UVAC_PAYLOAD_TX_SIZE *p_user = (UVAC_PAYLOAD_TX_SIZE *)Value;

			if (p_user->cnt && p_user->p_payload_tx_size && p_user->vidDevIdx < UVAC_VID_DEV_CNT_MAX) {
				if (p_user->type == UVAC_TYPE_U2) {
					if (p_user->video_fmt == UVAC_VIDEO_FORMAT_H264) {
						p_u3_uvc_h264_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_h264_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_h264_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					} else if (p_user->video_fmt == UVAC_VIDEO_FORMAT_MJPG) {
						p_u3_uvc_mjpg_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_mjpg_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_mjpg_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					} else if (p_user->video_fmt == UVAC_VIDEO_FORMAT_YUV) {
						p_u3_uvc_yuv_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_yuv_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_yuv_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					} else if (p_user->video_fmt == UVAC_VIDEO_FORMAT_NV12) {
						p_u3_uvc_nv12_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_nv12_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_nv12_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					} else if (p_user->video_fmt == UVAC_VIDEO_FORMAT_H265) {
						p_u3_uvc_h265_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_h265_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_h265_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					}
				} else {
					if (p_user->video_fmt == UVAC_VIDEO_FORMAT_H264) {
						p_u3_uvc_ss_h264_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_ss_h264_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_ss_h264_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					} else if (p_user->video_fmt == UVAC_VIDEO_FORMAT_MJPG) {
						p_u3_uvc_ss_mjpg_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_ss_mjpg_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_ss_mjpg_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					} else if (p_user->video_fmt == UVAC_VIDEO_FORMAT_YUV) {
						p_u3_uvc_ss_yuv_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_ss_yuv_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_ss_yuv_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					} else if (p_user->video_fmt == UVAC_VIDEO_FORMAT_NV12) {
						p_u3_uvc_ss_nv12_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_ss_nv12_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_ss_nv12_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					} else if (p_user->video_fmt == UVAC_VIDEO_FORMAT_H265) {
						p_u3_uvc_ss_h265_max_tx_size[p_user->vidDevIdx] = p_user->p_payload_tx_size;
						u3_uvc_ss_h265_max_tx_size_cnt[p_user->vidDevIdx] = p_user->cnt;

						u3_uvc_ss_h265_tx_base_size[p_user->vidDevIdx] = p_user->base_payload_tx_size;
					}
				}
			} else {
				DBG_ERR("UVAC_CONFIG_MAX_PAYLOAD_TX_SIZE invalid param!\r\n");
			}
		}
		break;
	case UVAC_CONFIG_UVC_BULK_MODE:
		u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_1] = Value;
		break;
	case UVAC_CONFIG_UVC2_BULK_MODE:
		u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_2] = Value;
		break;
	case UVAC_CONFIG_UVC3_BULK_MODE:
		u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_3] = Value;
		break;

	case UVAC_CONFIG_UVC_BULK_MULTI_PAYLOAD:
		u3_uvc_bulk_multi_payload = Value;
		break;
	case UVAC_CONFIG_UVC_BULK_PAYLOAD_SIZE:
		u3_uvc_bulk_payload_size = Value;
		break;
	case UVAC_CONFIG_UVC_HS_BULK_PAYLOAD_SIZE:
		u3_uvc_hs_bulk_payload_size = Value;
		break;

	case UVAC_CONFIG_UVC_HS_MJPG_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC_HS_H264_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC2_HS_MJPG_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC2_HS_H264_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_HS_YUV_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC2_HS_YUV_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC3_HS_YUV_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_HS_NV12_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC2_HS_NV12_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC3_HS_NV12_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_PROBE_SET_CUR_MAX_COUNT:
		{
			if (Value < 4) {
				DBG_WRN("PROBE_SET_CUR_MAX_COUNT = %d is less than 4\r\n", Value);
			}
			set_probe_count_max = Value;
		}
		break;
	case UVAC_CONFIG_MSDC_IN_EP:
		{
			if (Value >= (USB3_EP_MAX-1)) {
				DBG_ERR("Invalid MSDC IN EP = %d.  Larger than EP7\r\n", Value);
			} else {
				msdc_in_ep  = Value;
				msdc_out_ep = Value+1;
			}
		}
		break;

	case UVAC_CONFIG_SS_EP_BANDWIDTH:
		{
			UINT8 i;

			UVAC_SS_BANDWIDTH_INFO *p_user = (UVAC_SS_BANDWIDTH_INFO *)Value;
			memcpy((void*)gU3UvcIsoInSsMaxBurst, (void*)p_user->max_burst, sizeof(gU3UvcIsoInSsMaxBurst));
			memcpy((void*)gU3UvcIsoInSsEpcAttr, (void*)p_user->epc_attr, sizeof(gU3UvcIsoInSsEpcAttr));

			for (i = 0; i < UVAC_VID_EP_PACK_SIZE_MAX_CNT; i++) {
				gU3UvcIsoInSsBandWidth[i] = (gU3UvcIsoInSsMaxBurst[i]+1)*(gU3UvcIsoInSsEpcAttr[i]+1);
				//DBG_DUMP("[%d] max_burst=%d, epc_attr=%d, bandwidth=%d\r\n", i, gU3UvcIsoInSsMaxBurst[i], gU3UvcIsoInSsEpcAttr[i], gU3UvcIsoInSsBandWidth[i]);
			}


		}
		break;

	case UVAC_CONFIG_UVC3_MJPG_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC3_H264_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC3_HS_MJPG_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC3_HS_H264_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC_NV12_BULK_PAYLOAD_SIZE:
		{
			gU3UvcNv12Payload[UVAC_VID_DEV_CNT_1] = ((Value/1024)+1)*1024;
		}
		break;
	case UVAC_CONFIG_UVC_NV12_HS_BULK_PAYLOAD_SIZE:
		{
			gU3HsUvcNv12Payload[UVAC_VID_DEV_CNT_1] = ((Value/1024)+1)*1024;
		}
		break;
	case UVAC_CONFIG_HS_UVAC_CHANNEL:
		{
			if (Value >= UVAC_CHANNEL_MAX) {
				DBG_ERR("Invalid UVAC channel = %d\r\n", Value);
			} else {
				gHsUvacChannel = Value;
			}
		}
		break;
	case UVAC_CONFIG_HS_DISABLE_UAC:
		gU3HsDisableUac = Value;
		break;

	case UVAC_CONFIG_UVC_UVCP_EN:
		g_u3_uvcp_en = Value;
		break;

	case UVAC_CONFIG_UVC_DISABLE_STREAM:
		if (Value < UVAC_VID_DEV_CNT_MAX) {

			loc_cpu();
			gUvcVidStart[Value] = FALSE;
			unl_cpu();
		} else {
			DBG_ERR("clear buff invalid dev cnt = %d\r\n", Value);
		}
		break;

	case UVAC_CONFIG_UVC_H265_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;
	case UVAC_CONFIG_UVC2_H265_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;



	case UVAC_CONFIG_UVC3_H265_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;

	case UVAC_CONFIG_UVC_HS_H265_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = 0;
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = NULL;
			} else {
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_1].aryCnt = p_user->aryCnt;
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_1].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_1].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;

	case UVAC_CONFIG_UVC2_HS_H265_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = 0;
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = NULL;
			} else {
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_2].aryCnt = p_user->aryCnt;
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_2].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_2].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;

	case UVAC_CONFIG_UVC3_HS_H265_FRM_INFO:
		{
			UVAC_VID_RESO_ARY *p_user = (UVAC_VID_RESO_ARY *)Value;
			if (p_user->aryCnt && NULL == p_user->pVidResAry) {
				DBG_ERR("param error, no array\r\n");
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = 0;
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = NULL;
			} else {
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_3].aryCnt = p_user->aryCnt;
				gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_3].pVidResAry = p_user->pVidResAry;
				if (p_user->bDefaultFrameIndex > p_user->aryCnt) {
					DBG_ERR("bDefaultFrameIndex should <= aryCnt\r\n", p_user->bDefaultFrameIndex,  p_user->aryCnt);
					gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else if (p_user->bDefaultFrameIndex == 0) {
					gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = UVC_VSFMT_DEF_FRAMEINDEX;
				} else {
					gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_3].bDefaultFrameIndex = p_user->bDefaultFrameIndex;
				}
			}
		}
		break;

	default:
		DBG_ERR("Not Support the config:[%d]=0x%llx\r\n", ConfigID, Value);
		break;
	}
}

ULONG U3UVAC_GetConfig(UVAC_CONFIG_ID ConfigID, UINT32 param)
{
	ULONG ret = 0;

	switch (ConfigID) {
	case UVAC_CONFIG_QUEUE_COUNT:
		if (param == UVAC_STRM_AUD) {
			ret = (ULONG)gUvacTxfQueCurCnt[UVAC_TXF_QUE_A1];
		} else if (param == UVAC_STRM_AUD2) {
			ret = (ULONG)gUvacTxfQueCurCnt[UVAC_TXF_QUE_A2];
		} else if (param == UVAC_STRM_VID) {
			ret = (ULONG)gUvacTxfQueCurCnt[UVAC_TXF_QUE_V1];
		} else if (param == UVAC_STRM_VID2) {
			ret = (ULONG)gUvacTxfQueCurCnt[UVAC_TXF_QUE_V2];
		} else {
			DBG_ERR("Not Support STRM = %d\r\n", param);
		}
		break;

	default:
		DBG_ERR("Not Support the config:[%d]\r\n", ConfigID);
		break;
	}

	return ret;
}

static int _check_payload_tx_cnt(void)
{
	int ret = E_OK;
	UINT32 i;

	for (i = 0; i < UVAC_VID_DEV_CNT_MAX; i++) {
		//SS
		if (u3_uvc_ss_mjpg_max_tx_size_cnt[i] && (u3_uvc_ss_mjpg_max_tx_size_cnt[i] != gU3UvcMjpgFrmInfo[i].aryCnt)) {
			DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_ss_mjpg_max_tx_size_cnt[i], gU3UvcMjpgFrmInfo[i].aryCnt);
			ret = E_SYS;
			break;
		}
		if (u3_uvc_ss_h264_max_tx_size_cnt[i] && (u3_uvc_ss_h264_max_tx_size_cnt[i] != gU3UvcH264FrmInfo[i].aryCnt)) {
			DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_ss_h264_max_tx_size_cnt[i], gU3UvcH264FrmInfo[i].aryCnt);
			ret = E_SYS;
			break;
		}
		if (u3_uvc_ss_yuv_max_tx_size_cnt[i] && (u3_uvc_ss_yuv_max_tx_size_cnt[i] != gU3UvcYuvFrmInfo[i].aryCnt)) {
			ret = E_SYS;
			break;
		}
		if (u3_uvc_ss_nv12_max_tx_size_cnt[i] && (u3_uvc_ss_nv12_max_tx_size_cnt[i] != gU3UvcNV12FrmInfo[i].aryCnt)) {
			ret = E_SYS;
			break;
		}

		if (u3_uvc_ss_h265_max_tx_size_cnt[i] && (u3_uvc_ss_h265_max_tx_size_cnt[i] != gU3UvcH265FrmInfo[i].aryCnt)) {
			ret = E_SYS;
			break;
		}

		//HS
		if (gU3HsUvcMjpgFrmInfo[i].aryCnt) {
			if (u3_uvc_mjpg_max_tx_size_cnt[i] && (u3_uvc_mjpg_max_tx_size_cnt[i] != gU3HsUvcMjpgFrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_mjpg_max_tx_size_cnt[i], gU3HsUvcMjpgFrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		} else {
			if (u3_uvc_mjpg_max_tx_size_cnt[i] && (u3_uvc_mjpg_max_tx_size_cnt[i] != gU3UvcMjpgFrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_mjpg_max_tx_size_cnt[i], gU3UvcMjpgFrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		}

		if (gU3HsUvcH264FrmInfo[i].aryCnt) {
			if (u3_uvc_h264_max_tx_size_cnt[i] && (u3_uvc_h264_max_tx_size_cnt[i] != gU3HsUvcH264FrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_h264_max_tx_size_cnt[i], gU3HsUvcH264FrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		} else {
			if (u3_uvc_h264_max_tx_size_cnt[i] && (u3_uvc_h264_max_tx_size_cnt[i] != gU3UvcH264FrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_h264_max_tx_size_cnt[i], gU3UvcH264FrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		}

		if (gU3HsUvcYuvFrmInfo[i].aryCnt) {
			if (u3_uvc_yuv_max_tx_size_cnt[i] && (u3_uvc_yuv_max_tx_size_cnt[i] != gU3HsUvcYuvFrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_yuv_max_tx_size_cnt[i], gU3HsUvcYuvFrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		} else {
			if (u3_uvc_yuv_max_tx_size_cnt[i] && (u3_uvc_yuv_max_tx_size_cnt[i] != gU3UvcYuvFrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_yuv_max_tx_size_cnt[i], gU3UvcYuvFrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		}

		if (gU3HsUvcNV12FrmInfo[i].aryCnt) {
			if (u3_uvc_nv12_max_tx_size_cnt[i] && (u3_uvc_nv12_max_tx_size_cnt[i] != gU3HsUvcNV12FrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_nv12_max_tx_size_cnt[i], gU3HsUvcNV12FrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		} else {
			if (u3_uvc_nv12_max_tx_size_cnt[i] && (u3_uvc_nv12_max_tx_size_cnt[i] != gU3UvcNV12FrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_nv12_max_tx_size_cnt[i], gU3UvcNV12FrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		}

		if (gU3HsUvcH265FrmInfo[i].aryCnt) {
			if (u3_uvc_h265_max_tx_size_cnt[i] && (u3_uvc_h265_max_tx_size_cnt[i] != gU3HsUvcH265FrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_h265_max_tx_size_cnt[i], gU3HsUvcH265FrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		} else {
			if (u3_uvc_h265_max_tx_size_cnt[i] && (u3_uvc_h265_max_tx_size_cnt[i] != gU3UvcH265FrmInfo[i].aryCnt)) {
				DBG_DUMP("i%d %d %d\r\n", i, u3_uvc_h265_max_tx_size_cnt[i], gU3UvcH265FrmInfo[i].aryCnt);
				ret = E_SYS;
				break;
			}
		}
	}

	return ret;
}
void UVAC_SetCodec(UVAC_VIDEO_FORMAT CodecType, UINT32 vidIdx)
{
	DbgMsg_UVC(("PrvCodec[%d]=%d,SetTo=%d\r\n", vidIdx, gUvacCodecType[vidIdx], CodecType));
	gUvacCodecType[vidIdx] = CodecType;
}
UINT32 U3UVAC_GetNeedMemSize(void)
{
	UINT32 memSize;
	UINT32 uvac_channel = (gUvacChannel > gHsUvacChannel)? gUvacChannel : gHsUvacChannel;

	memSize = ALIGN_CEIL(gUvcMaxVideoFmtSize, UVAC_BUF_ALIGN) * uvac_channel + ALIGN_CEIL(UVAC_MEM_DESC_SIZE, UVAC_BUF_ALIGN);

	#if 0//gUvacMtpEnabled
	if (gUvacMtpEnabled) {
		memSize += SIDC_WORKING_BUFFER + MIN_BUFFER_SIZE_FOR_SENDOBJ;
	}
	#endif
#if (UVAC_CONTROLLER == UVAC_CONTROL_U3)
	//memSize += U3DEV_MIN_BUFFER_SIZE;
#if (UVAC_SUPPORT_SUPER_SPEED == ENABLE)
	memSize += ALIGN_CEIL(UVAC_MEM_DESC_SIZE, UVAC_BUF_ALIGN);
#endif

	if (gUvacUacRxEnabled) {
		 memSize += ALIGN_CEIL(gUvacUacBufSize, UVAC_BUF_ALIGN);
	}

#endif
	DbgMsg_UVC(("NeedMem=0x%x, ch=%d, Max Payload Frame=0x%x, DescMem=0x%x\n\r", memSize, uvac_channel, gUvcMaxVideoFmtSize, UVAC_MEM_DESC_SIZE));
	return memSize;
}
#if 0
static UINT32 UVCIsoIn_ConcateH264Header(UVAC_VID_DEV_CNT  thisVidIdx, PUVC_PAYL_MAKE_COMP pUvcPaylMakeComp)
{
	UINT8 *pSrcAddr = 0;
	UINT8 *pDstAddr = 0;
	UINT8 *pHdrAddr = 0;
	UINT32 vidSize = 0;
	UINT32 hdrSize = 0;
	UINT32 totalSize = 0;
	UINT8 *pFirstVidData = 0;

	if (0 == pUvcPaylMakeComp) {
		DBG_ERR("Input NULL.Do nothing!!\r\n");
		return totalSize;
	} else {
		pDstAddr = pUvcPaylMakeComp->pDst;
		if (0 == pDstAddr) {
			DBG_ERR("NULL DST!!\r\n");
			return totalSize;
		}
		pSrcAddr = pUvcPaylMakeComp->pVidSrc;
		vidSize = pUvcPaylMakeComp->vidSize;
		pHdrAddr = pUvcPaylMakeComp->pHdrAddr;
		hdrSize = pUvcPaylMakeComp->hdrSize;
		DbgMsg_UVCIO(("%s:src=0x%x,dst=0x%x,len=0x%x,hdr=0x%x,0x%x\r\n", __func__, pSrcAddr, pDstAddr, vidSize, pHdrAddr, hdrSize));
	}
	hwmem_open();
	if (hdrSize && pHdrAddr) {
		//hwmem_memcpy((UINT32)pDstAddr, (UINT32)pHdrAddr, hdrSize);
		memcpy((void *)pDstAddr, (const void *)pHdrAddr, hdrSize);
		pDstAddr = (UINT8 *)((uintptr_t)pDstAddr + hdrSize);
		totalSize += hdrSize;
	}
	hwmem_memcpy(thisVidIdx, (uintptr_t)pDstAddr, (uintptr_t)pSrcAddr, vidSize);
	totalSize += vidSize;
	hwmem_close();
	pFirstVidData = pDstAddr;
	if (pUvcPaylMakeComp->pVidSrc) {
		DbgMsg_UVCIO(("UPDATE first 4 byte:0x%x,0x%x,0x%x,0x%x\r\n", *pFirstVidData, *(pFirstVidData + 1), *(pFirstVidData + 2), *(pFirstVidData + 3)));
		*pFirstVidData = 0;
		*(pFirstVidData + 1) = 0;
		*(pFirstVidData + 2) = 0;
		*(pFirstVidData + 3) = 1;
	}
	DbgMsg_UVCIO(("dst=0x%x/0x%x,src=0x%x/0x%x,tol=0x%x\r\n", pUvcPaylMakeComp->pDst, pDstAddr, pUvcPaylMakeComp->pVidSrc, pSrcAddr, totalSize));
	return totalSize;
}
#endif

#if 0
static void DumpMem(uintptr_t Addr, UINT32 Size, UINT32 Alignment)
{
	UINT32 i;
	UINT8 *pBuf = (UINT8 *)Addr;
	for (i = 0; i < Size; i++) {
		if (i > 0 && i % Alignment == 0) {
			DBG_DUMP("\r\n");
		}
		DBG_DUMP("0x%02X ", *(pBuf + i));
	}
	DBG_DUMP("\r\n");
}
#endif

//SW copy only for FPGA or small stream
static UINT32 UVCIsoIn_MakeIsoPayloadTxfDataByCPU_no_copy(UVAC_VID_DEV_CNT  thisVidIdx, PUVC_PAYL_MAKE_COMP pUvcPaylMakeComp)
{
	UINT32 toggleFid = 0;
	UINT8 *pDstAddr = 0;
	UINT32 totalSize = 0;
	UINT32 tmpSize = 0;
	UINT8 *pLastPayloadHdrAddr = 0;
#if UVC_MAKE_PAYLOAD_PERF
	UINT64 h1 = 0, h2 = 0, d1 = 0, d2 = 0, in = 0, out = 0;
	static UINT32 frm_cnt =0;

	frm_cnt++;
	in = hd_gettime_us();
#endif

	pDstAddr = pUvcPaylMakeComp->pVidSrc;
	toggleFid = pUvcPaylMakeComp->toggleFid;

	pUvcPaylMakeComp->pDst = pDstAddr;

	UVCUtil_UpdatePts();

#if UVC_MAKE_PAYLOAD_PERF
	h1 = hd_gettime_us();
#endif
	tmpSize = UVCUtil_MakeHeadScrPts(pDstAddr, toggleFid);
	pLastPayloadHdrAddr = pDstAddr;
	pDstAddr = (UINT8 *)((uintptr_t)pDstAddr + tmpSize);
	totalSize = tmpSize + pUvcPaylMakeComp->vidSize;
#if UVC_MAKE_PAYLOAD_PERF
		d1 = h2 = hd_gettime_us();
#endif


#if UVC_MAKE_PAYLOAD_PERF
		d2 = hd_gettime_us();
#endif

	if (pUvcPaylMakeComp->frame_type == UVAC_VIDEO_FRM_NORMAL || pUvcPaylMakeComp->frame_type == UVAC_VIDEO_FRM_FINAL) {
		*(pLastPayloadHdrAddr + 1) |= PAYLOAD_EOF;
	}

	hwmem_memflush((void *)pUvcPaylMakeComp->pVidSrc, totalSize);
#if UVC_MAKE_PAYLOAD_PERF
	out = hd_gettime_us();
	if (frm_cnt%150 ==0){
		DBG_DUMP("%dKB H[%llu]D[%llu]IO[%llu]us\r\n",totalSize/1024, h2-h1, d2-d1,out-in);
	}
#endif

	return totalSize;
}

static UINT32 UVCIsoIn_MakeIsoPayloadTxfDataByCPU(UVAC_VID_DEV_CNT  thisVidIdx, PUVC_PAYL_MAKE_COMP pUvcPaylMakeComp)
{
	UINT32 toggleFid = 0;
	UINT8 *pSrcAddr = 0;
	UINT8 *pDstAddr = 0;
	UINT8 *pHdrAddr = 0;
	UINT32 vidSize = 0;
	UINT32 hdrSize = 0;
	UINT32 remainSize = 0;
	UINT32 totalSize = 0;
	UINT32 tmpSize = 0;
	const UINT32 unitSize = gUVCIsoinTxfUnitSize[thisVidIdx] - PAYLOAD_LEN;
	UINT8 *pLastPayloadHdrAddr = 0;
#if UVC_MAKE_PAYLOAD_PERF
	UINT64 h1 = 0, h2 = 0, d1 = 0, d2 = 0, in = 0, out = 0;
	static UINT32 frm_cnt =0;

	frm_cnt++;
	in = hd_gettime_us();
#endif
#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
	if (gUVCIsoinTxfUnitSize[thisVidIdx] <= PAYLOAD_LEN) {
		DBG_ERR("Fail Unit Size=%d, Payload hdr size = %d\r\n", gUVCIsoinTxfUnitSize[thisVidIdx], PAYLOAD_LEN);
	}
#endif

	pDstAddr = pUvcPaylMakeComp->pDst;
	pSrcAddr = pUvcPaylMakeComp->pVidSrc;
	vidSize = pUvcPaylMakeComp->vidSize;
	toggleFid = pUvcPaylMakeComp->toggleFid;
	pHdrAddr = pUvcPaylMakeComp->pHdrAddr;
	hdrSize = pUvcPaylMakeComp->hdrSize;
	DbgMsg_UVCIO(("%s:src=0x%x,dst=0x%x,len=0x%x,fid=%d,hdr=0x%x,0x%x,unitSize=0x%x\r\n", __func__, pSrcAddr, pDstAddr, vidSize, toggleFid, pHdrAddr, hdrSize, unitSize));

	UVCUtil_UpdatePts();

#if UVC_MAKE_PAYLOAD_PERF
	h1 = hd_gettime_us();
#endif
	tmpSize = UVCUtil_MakeHeadScrPts(pDstAddr, toggleFid);
	pLastPayloadHdrAddr = pDstAddr;
	pDstAddr = (UINT8 *)((uintptr_t)pDstAddr + tmpSize);
	totalSize = tmpSize;
#if UVC_MAKE_PAYLOAD_PERF
		d1 = h2 = hd_gettime_us();
#endif
	if (hdrSize && pHdrAddr) {
#if (_UVC_DBG_LVL_ >= _UVC_DBG_CHK_)
		if (gUVCIsoinTxfUnitSize[thisVidIdx] <= (hdrSize + PAYLOAD_LEN)) {
			DBG_ERR("Fail make payload, hdrSize/%d+%d=%d > txf-unit-size=%d\r\n", hdrSize, PAYLOAD_LEN, (hdrSize + PAYLOAD_LEN), gUVCIsoinTxfUnitSize[thisVidIdx]);
		}
#endif
		//hwmem_memcpy((UINT32)pDstAddr, (UINT32)pHdrAddr, hdrSize);
		memcpy((void *)pDstAddr, (const void *)pHdrAddr, hdrSize);
		pDstAddr = (UINT8 *)((uintptr_t)pDstAddr + hdrSize);
		remainSize = unitSize - hdrSize;
		totalSize += hdrSize;
	} else {
		DbgMsg_UVCIO(("Null Head=0x%x,0x%x\r\n", pHdrAddr, hdrSize));
		remainSize = unitSize;
	}
	//pFirstVidData = pDstAddr;
	while (vidSize) {
		remainSize = (vidSize > remainSize) ? remainSize : vidSize;
		memcpy((void *)pDstAddr, (const void *)pSrcAddr, remainSize);
		vidSize -= remainSize;
		totalSize += remainSize;
		pDstAddr = (UINT8 *)((uintptr_t)pDstAddr + remainSize);
		pSrcAddr = (UINT8 *)((uintptr_t)pSrcAddr + remainSize);
		remainSize = unitSize;
		if (vidSize) {
			tmpSize = UVCUtil_MakeHeadScrPts(pDstAddr, toggleFid);
			pLastPayloadHdrAddr = pDstAddr;
			pDstAddr = (UINT8 *)((uintptr_t)pDstAddr + tmpSize);
			totalSize += tmpSize;
		}
	}
#if UVC_MAKE_PAYLOAD_PERF
		d2 = hd_gettime_us();
#endif

	if (pUvcPaylMakeComp->frame_type == UVAC_VIDEO_FRM_NORMAL || pUvcPaylMakeComp->frame_type == UVAC_VIDEO_FRM_FINAL) {
		*(pLastPayloadHdrAddr + 1) |= PAYLOAD_EOF;
	}
	/*if (pUvcPaylMakeComp->bIsH264 && pUvcPaylMakeComp->pVidSrc) {
		DbgMsg_UVCIO(("update first 4 byte:0x%x,0x%x,0x%x,0x%x\r\n", *pFirstVidData, *(pFirstVidData + 1), *(pFirstVidData + 2), *(pFirstVidData + 3)));
		*pFirstVidData = 0;
		*(pFirstVidData + 1) = 0;
		*(pFirstVidData + 2) = 0;
		*(pFirstVidData + 3) = 1;
	}*/
	hwmem_memflush((void *)pUvcPaylMakeComp->pDst, totalSize);
#if UVC_MAKE_PAYLOAD_PERF
	out = hd_gettime_us();
	if (frm_cnt%150 ==0){
		DBG_DUMP("%dKB H[%llu]D[%llu]IO[%llu]us\r\n",totalSize/1024, h2-h1, d2-d1,out-in);
	}
#endif

	return totalSize;
}

#if UVCP_FUNC
static UINT32 UVCIsoIn_MakeIsoPayloadTxfDataByUVCP(UVAC_VID_DEV_CNT  thisVidIdx, PUVC_PAYL_MAKE_COMP pUvcPaylMakeComp)
{
	UINT32 vidSize;
	UINT32 totalSize = 0;
	const UINT32 unitSize = gUVCIsoinTxfUnitSize[thisVidIdx] - PAYLOAD_LEN;
	UINT32  counter_val, pts_val, sof_val, output_size;
	UINT32 breaksz;
	VOS_TICK t1 = 0, t2 = 0, t3 = 0, t0 = 0;
	UINT32 payload_cnt;
	uintptr_t src_img_pa, dst_img_pa;
	HD_COMMON_MEM_VIRT_INFO vir_meminfo = {0};

	vos_perf_mark(&t0);

	vidSize = pUvcPaylMakeComp->vidSize;

	uvcp_set_config(UVCP_CONFIG_ID_HEADER_LENGTH, PAYLOAD_LEN);
	uvcp_set_config(UVCP_CONFIG_ID_DATALEN_PER_UF, gUVCIsoinTxfUnitSize[thisVidIdx]);
	uvcp_set_config(UVCP_CONFIG_ID_REFERENCE_CLK,	30);
	sof_val = usb3dev_getSOF();
	uvcp_set_config(UVCP_CONFIG_ID_SOF_START_VAL, sof_val);
	pts_val = counter_val = (UINT32)((UINT32)hd_gettime_us()*30);
	uvcp_set_config(UVCP_CONFIG_ID_COUNTER_START_VAL, counter_val);
	uvcp_set_config(UVCP_CONFIG_ID_PTS, pts_val);

	output_size = gVidTxBuf[thisVidIdx].size;
	vir_meminfo.va = (void *)pUvcPaylMakeComp->pVidSrc;
	if (hd_common_mem_get(HD_COMMON_MEM_PARAM_VIRT_INFO, &vir_meminfo) != HD_OK) {
		DBG_ERR("fail to convert va(0x%lX) to pa.\r\n", (unsigned long)vir_meminfo.va);
		return 0;
	}
	src_img_pa = vir_meminfo.pa;
	dst_img_pa = uvac_va_to_pa(UVAC_BUF_TYPE_WORK, (uintptr_t)pUvcPaylMakeComp->pDst);

	uvcp_config_dma(src_img_pa, vidSize, dst_img_pa, &output_size);
	breaksz = gUVCIsoinTxfUnitSize[thisVidIdx];
	//breaksz = vidSize/100;
	//breaksz = 64;
	//breaksz = vidSize*3/4;
	if (vidSize < breaksz) {
		breaksz = breaksz;
	}
	uvcp_set_config(UVCP_CONFIG_ID_BREAK_SIZE, breaksz);
	vos_perf_mark(&t1);
	if (pUvcPaylMakeComp->frame_type == UVAC_VIDEO_FRM_NORMAL || pUvcPaylMakeComp->frame_type == UVAC_VIDEO_FRM_FINAL) {
		uvcp_trigger(TRUE, FALSE);
	} else {
		uvcp_trigger(FALSE, FALSE);
	}
	// work around for uvc hangs up.
	if (vidSize > gUVCIsoinTxfUnitSize[thisVidIdx]) {
		uvcp_wait_break();
	} else {
		uvcp_wait_complete();
	}
	//DBGD(uvcp_get_config(UVCP_CONFIG_ID_GETOUT_SIZE));

	vos_perf_mark(&t2);
	//uvcp_wait_complete();
	//DBGD(uvcp_get_config(UVCP_CONFIG_ID_GETOUT_SIZE));
	vos_perf_mark(&t3);
	payload_cnt = (vidSize+(unitSize-1))/unitSize;
	totalSize = vidSize + payload_cnt*PAYLOAD_LEN;

	if (m_uiUvacDbgVid & UVAC_DBG_VID_PAYLOAD) {
		DBG_DUMP("BP=%d,F=%d,IO=%d,size=%d\r\n", vos_perf_duration(t1, t2), vos_perf_duration(t1, t3), vos_perf_duration(t0, t3), totalSize);
		//DumpMem((UINT32)pUvcPaylMakeComp->pDst, PAYLOAD_LEN, 16);
		//DumpMem((UINT32)pUvcPaylMakeComp->pDst + gUVCIsoinTxfUnitSize*(payload_cnt-1), PAYLOAD_LEN, 16);
		//DBG_DUMP("++SOF=0x%X,counter_val=0x%X\r\n", sof_val, counter_val);
		//DBG_DUMP("--SOF=0x%X,counter_val=0x%X\r\n", uvcp_get_config(UVCP_CONFIG_ID_GET_SOF_END), uvcp_get_config(UVCP_CONFIG_ID_GET_COUNTER_END));
		//DBG_DUMP("0x%X,0x%X=>%d\r\n", (UINT32)pSrcAddr, vidSize, totalSize);
	}
	//hd_common_mem_flush_cache((void *)pUvcPaylMakeComp->pDst, totalSize);
	return totalSize;
}
#endif

static UINT32 UVCIsoIn_MakeIsoPayloadTxfData(UVAC_VID_DEV_CNT  thisVidIdx, PUVC_PAYL_MAKE_COMP pUvcPaylMakeComp)
{
	UINT8 *pDstAddr = 0;
	UINT32 vidSize = 0;
	UINT32 totalSize = 0;
	const UINT32 unitSize = gUVCIsoinTxfUnitSize[thisVidIdx] - PAYLOAD_LEN;

#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
	if (gUVCIsoinTxfUnitSize[thisVidIdx] <= PAYLOAD_LEN) {
		DBG_ERR("Fail Unit Size=%d, Payload hdr size = %d\r\n", gUVCIsoinTxfUnitSize[thisVidIdx], PAYLOAD_LEN);
	}
#endif

	if (0 == pUvcPaylMakeComp) {
		DBG_ERR("Input NULL.Do nothing!!\r\n");
		return totalSize;
	} else {
		pDstAddr = pUvcPaylMakeComp->pDst;
		if (0 == pDstAddr) {
			DBG_ERR("NULL DST!!\r\n");
			return totalSize;
		}
		vidSize = pUvcPaylMakeComp->vidSize;
	}
	if (u3_uvc_bulk_mode[thisVidIdx] && pUvcPaylMakeComp->format == UVAC_VIDEO_FORMAT_NV12) {
		return UVCIsoIn_MakeIsoPayloadTxfDataByCPU_no_copy(thisVidIdx, pUvcPaylMakeComp);
	} else if (pUvcPaylMakeComp->format == UVAC_VIDEO_FORMAT_H264 || vidSize <= unitSize) {
		return UVCIsoIn_MakeIsoPayloadTxfDataByCPU(thisVidIdx, pUvcPaylMakeComp);
	} else if (pUvcPaylMakeComp->format == UVAC_VIDEO_FORMAT_MJPG && pUvcPaylMakeComp->frame_type != UVAC_VIDEO_FRM_NORMAL) {
		//slice MJPEG
		return UVCIsoIn_MakeIsoPayloadTxfDataByCPU(thisVidIdx, pUvcPaylMakeComp);
	} else {
		if (g_u3_uvcp_en) {
			if (thisVidIdx == UVAC_VID_DEV_CNT_1) {
				return UVCIsoIn_MakeIsoPayloadTxfDataByUVCP(thisVidIdx, pUvcPaylMakeComp);
			} else {
				return UVCIsoIn_MakeIsoPayloadTxfDataByCPU(thisVidIdx, pUvcPaylMakeComp);
			}
		} else {
			return UVCIsoIn_MakeIsoPayloadTxfDataByCPU(thisVidIdx, pUvcPaylMakeComp);
		}
	}

	return totalSize;
}
void UVC_MakePayloadFmt(UVAC_VID_DEV_CNT  thisVidIdx, UVAC_TXF_INFO *pTxfInfo)
{
	UVC_PAYL_MAKE_COMP  uvcPaylMakeCom;
	UINT32 DMALen = 0;
	UVAC_VID_STRM_INFO *pStrmQueInfo = 0;

	if (pTxfInfo->sAddr == 0) {
		DBG_ERR("Stream NULL\r\n");
		return;
	}

	if (pTxfInfo->size == 0) {
		DBG_ERR("Size NULL\r\n");
		return;
	}

//DBG_DUMP("[%d]%X %X\r\n",__LINE__, *(UINT8 *)(guiVideoBufAddr+FirstSize-2),*(UINT8 *)(guiVideoBufAddr+FirstSize-1));

	if (pTxfInfo->size*105/100 > gVidTxBuf[thisVidIdx].size) {
		DBG_ERR("stream (%dKB) > TxBuf (%dKB)\r\n", pTxfInfo->size*105/100, gVidTxBuf[thisVidIdx].size / 1024);
		pTxfInfo->size = 0;
		return;
	}

	uvcPaylMakeCom.pVidSrc = (UINT8 *)pTxfInfo->sAddr;
	uvcPaylMakeCom.vidSize = pTxfInfo->size;
	uvcPaylMakeCom.pDst = (UINT8 *)gVidTxBuf[thisVidIdx].addr;
	uvcPaylMakeCom.toggleFid = gToggleFid[thisVidIdx];
	uvcPaylMakeCom.frame_type = pTxfInfo->frame_type;

	if (uvcPaylMakeCom.frame_type == UVAC_VIDEO_FRM_NORMAL || uvcPaylMakeCom.frame_type == UVAC_VIDEO_FRM_FINAL) {
		gToggleFid[thisVidIdx] ^= 1;
	}
	//H264
	if (_is_iframe(uvcPaylMakeCom.pVidSrc)) {
		pStrmQueInfo = &gUvacVidStrmInfo[thisVidIdx];
		uvcPaylMakeCom.pHdrAddr = pStrmQueInfo->strmInfo.pStrmHdr;
		uvcPaylMakeCom.hdrSize = pStrmQueInfo->strmInfo.strmHdrSize;
	} else if (_is_iframe_h265(uvcPaylMakeCom.pVidSrc)) {
		pStrmQueInfo = &gUvacVidStrmInfo[thisVidIdx];
		uvcPaylMakeCom.pHdrAddr = pStrmQueInfo->strmInfo.pStrmHdr;
		uvcPaylMakeCom.hdrSize = pStrmQueInfo->strmInfo.strmHdrSize;
	} else {
		uvcPaylMakeCom.pHdrAddr = 0;
		uvcPaylMakeCom.hdrSize = 0;
	}

	uvcPaylMakeCom.format = gUvacCodecType[thisVidIdx];

	DMALen = UVCIsoIn_MakeIsoPayloadTxfData(thisVidIdx, &uvcPaylMakeCom);
//DBG_DUMP("[%d]%X %X\r\n",__LINE__, *(UINT8 *)(guiVideoBufAddr+FirstSize-2),*(UINT8 *)(guiVideoBufAddr+FirstSize-1));


	if (u3_uvc_bulk_mode[thisVidIdx] && gUvacCodecType[thisVidIdx] == UVAC_VIDEO_FORMAT_NV12) {
		HD_COMMON_MEM_VIRT_INFO vir_meminfo = {0};

		vir_meminfo.va = (void *)uvcPaylMakeCom.pVidSrc;
		if (hd_common_mem_get(HD_COMMON_MEM_PARAM_VIRT_INFO, &vir_meminfo) != HD_OK) {
			DBG_ERR("fail to convert va(0x%lX) to pa.\r\n", (unsigned long)vir_meminfo.va);
			return;
		}
		pTxfInfo->sAddr = pTxfInfo->oriAddr = vir_meminfo.pa;

	} else {
		pTxfInfo->sAddr = pTxfInfo->oriAddr = uvac_va_to_pa(UVAC_BUF_TYPE_WORK, (uintptr_t)uvcPaylMakeCom.pDst);

	}

	pTxfInfo->size = DMALen;
	if (DMALen > gVidTxBuf[thisVidIdx].size) {
		DBG_ERR("Payload Frame (%dKB) > TxBuf (%dKB)\r\n", DMALen / 1024, gVidTxBuf[thisVidIdx].size / 1024);
	}
}
/**
    UVAC Video Task.
*/
void* UVAC_VideoTsk(void* arglist)
{
	UINT32              uiOutWidth = 0, uiOutHeight = 0, uiFps = 0;
	FLGPTN              uiFlag;
	UINT32              totalFrmCnt = 0;
	UINT32              toggleFid = 0;
	UVAC_STRM_INFO      strmInfo = {0};
	//ER                  retV = E_OK;
	UVAC_TXF_INFO       txfInfo;
	UINT32              tmpIdx = 0;
	UINT8               *pVidBuf;
	UINT32              vidSize;
	UINT32              thisVidIdx = UVAC_VID_DEV_CNT_1;
	UINT32              thisTxfQueIdx = UVAC_TXF_QUE_V1;
	PUVAC_VID_STRM_INFO pStrmQueInfo = &gUvacVidStrmInfo[thisVidIdx];
	UINT32 uiQueCnt;
	UVAC_VIDEO_FRM_TYPE frame_type = 0;


	//kent_tsk();
	while (1) {
		DbgMsg_UVC(("UVAC Tsk Init..\r\n"));
		set_flg(FLG_ID_UVAC, FLGUVAC_RDY);
		wai_flg(&uiFlag, FLG_ID_UVAC, FLGUVAC_START | FLGUVAC_VIDEO_EXIT, TWF_ORW);        //Note we don't clear start flag!!!
		clr_flg(FLG_ID_UVAC, FLGUVAC_RDY);

		if (uiFlag & FLGUVAC_VIDEO_EXIT) {
			set_flg(FLG_ID_UVAC, FLGUVAC_RDY);
			break;
		}

		UVAC_GetImageSize(gUvacCodecType[thisVidIdx], gUvcParamAry[thisVidIdx].OutSizeID, thisVidIdx, &uiOutWidth, &uiOutHeight);
		uiFps = gUvcParamAry[thisVidIdx].FrameRate;

		DbgMsg_UVC(("^GVidTsk:W=%d,H=%d,Fr=%d,Fmt=%d,%d\r\n", uiOutWidth, uiOutHeight, uiFps, gUvacCodecType[thisVidIdx], gUvcNoVidStrm));
		UVAC_DbgDmp_TxfInfoByQue(thisTxfQueIdx);
		UVAC_DbgDmp_MemLayout();

		UVC_TimeTrigSet(1000000 / uiFps, thisVidIdx);

		//CB to change video resolution, codec, fps
		if (g_fpStartVideo) {
			memset((void *)&gStrmInfo[thisVidIdx], 0, sizeof(UVAC_STRM_INFO));
			gStrmInfo[thisVidIdx].strmPath = UVAC_STRM_VID;

			pStrmQueInfo->strmInfo.strmCodec = gStrmInfo[thisVidIdx].strmCodec = gUvacCodecType[thisVidIdx];
			pStrmQueInfo->strmInfo.strmWidth = gStrmInfo[thisVidIdx].strmWidth = uiOutWidth;
			pStrmQueInfo->strmInfo.strmHeight = gStrmInfo[thisVidIdx].strmHeight = uiOutHeight;
			pStrmQueInfo->strmInfo.strmFps = gStrmInfo[thisVidIdx].strmFps = uiFps;
			pStrmQueInfo->strmInfo.strmResoIdx = gStrmInfo[thisVidIdx].strmResoIdx = gUvcParamAry[thisVidIdx].OutSizeID - 1;
			pStrmQueInfo->strmInfo.isStrmOn = gStrmInfo[thisVidIdx].isStrmOn = TRUE;
			if (UVAC_VIDEO_FORMAT_H264 == gUvacCodecType[thisVidIdx]) {
				pStrmQueInfo->strmInfo.strmTBR = gStrmInfo[thisVidIdx].strmTBR = gUvcH264TBR;
			} else {
				pStrmQueInfo->strmInfo.strmTBR = gStrmInfo[thisVidIdx].strmTBR = gUvcMJPGTBR;
			}

			UVAC_RemoveTxfInfo(thisTxfQueIdx);
			if (E_OK != g_fpStartVideo(thisVidIdx, &gStrmInfo[thisVidIdx])) {
				//Get the data of H264 Header,and check resolution is workale or not
				DBG_ERR("StartVid Fail, Reso=%d/%d, Codec=%d\r\n", uiOutWidth, uiOutHeight, gUvacCodecType[thisVidIdx]);
				clr_flg(FLG_ID_UVAC, FLGUVAC_START);
				UVC_TimeTrigClose(thisVidIdx);
				continue;
			}
		} else {
			UVAC_RemoveTxfInfo(thisTxfQueIdx);
			DBG_ERR("CB NULL. Can NOT change video resolution,codec,fps\r\n");
		}
		if (gU3UvacRunningFlag[thisVidIdx] & FLGUVAC_STOP) {
			DbgMsg_UVC(("Stop C...\r\n"));
			goto L_UVAC_END;
		}

		//UVAC_RemoveTxfInfo(thisTxfQueIdx);
		totalFrmCnt = toggleFid = 0;
		gToggleFid[thisVidIdx] = 0;
		uiQueCnt = 0;
		while (gUvcVidStart[thisTxfQueIdx]) {
			if (uvc_direct_trigger) {
				wai_flg(&uiFlag, FLG_ID_UVAC, FLGUVAC_VIDEO_TXF|FLGUVAC_VIDEO_STOP, (TWF_CLR | TWF_ORW));
				if (uiFlag & FLGUVAC_VIDEO_STOP) {
					DbgMsg_UVC(("^GVdoTsk1 Stop\r\n"));
					break;
				}
			} else {
				#if 0
				timer_waitTimeup(gUvacTimerID[thisVidIdx]);
				#endif
			}
			DbgMsg_UVCIO(("11Uvac-timer:%d, %d\r\n", totalFrmCnt, toggleFid));

V1_SKIP_TIMER:
			//Send the first frame and it must be I frame
			tmpIdx = pStrmQueInfo->idxConsumer;
			DbgMsg_UVCIO(("StrmQue[%d]:cons=%d,prod=%d\r\n", thisVidIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer));
			if (tmpIdx == pStrmQueInfo->idxProducer) {
				if (m_uiUvacDbgVid & UVAC_DBG_VID_START) {
					DBG_WRN("Empty StrmQue[%d]:con=%d,prod=%d, start=%d, NoStrm=%d\r\n", thisVidIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer, gUvcVidStart[thisTxfQueIdx], gUvcNoVidStrm);
				}
				if (TRUE == gUvcVidStart[thisTxfQueIdx]) {
					continue;
				} else {
					break;
				}
			} else {
				DbgCode_UVC(if (0 == totalFrmCnt) {
				DbgMsg_UVC(("FirstFrm[%d]:cons=%d,prod=%d\r\n", thisVidIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer));
				})
			}
			if (0 == totalFrmCnt && (m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
				DBG_DUMP("idxProducer=%d, idxConsumer=%d\r\n", pStrmQueInfo->idxProducer, pStrmQueInfo->idxConsumer);
			}
			pVidBuf = (UINT8 *)(pStrmQueInfo->addr[tmpIdx]);
			vidSize = pStrmQueInfo->size[tmpIdx];
			frame_type = pStrmQueInfo->frame_type[tmpIdx];
			if (0 == totalFrmCnt) {
				if (UVAC_VIDEO_FORMAT_MJPG == gUvacCodecType[thisVidIdx] || UVAC_VIDEO_FORMAT_YUV== gUvacCodecType[thisVidIdx] || UVAC_VIDEO_FORMAT_NV12 == gUvacCodecType[thisVidIdx]) {
					//point to the latest frame index if not slice mode
					if (frame_type == UVAC_VIDEO_FRM_NORMAL) {
						pStrmQueInfo->idxConsumer = (pStrmQueInfo->idxProducer + UVAC_VID_INFO_QUE_MAX_CNT - 1) % UVAC_VID_INFO_QUE_MAX_CNT;
						tmpIdx = pStrmQueInfo->idxConsumer;
					} else if (frame_type != UVAC_VIDEO_FRM_FIRST) {
						DbgMsg_UVCIO(("Chk slice mode first Frm [%d]addr=0x%lx,0x%x,cnt=0x%x\r\n", tmpIdx, (uintptr_t)pVidBuf, vidSize, totalFrmCnt));
						if ((m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
							DBG_WRN("Wait 1st I-frm:%d, vidStrm exist=%d\r\n", tmpIdx, gUvcNoVidStrm);
						}
						pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V1);
						continue;
					}
				} else if (UVAC_VIDEO_FORMAT_H265 == gUvacCodecType[thisVidIdx]) { //UVAC_VIDEO_FORMAT_H265
					if ((0 == vidSize) || (FALSE == _is_iframe_h265(pVidBuf))) {
						DbgMsg_UVCIO(("Chk I-Frm [%d]addr=0x%lx,0x%x,cnt=0x%x\r\n", tmpIdx, (uintptr_t)pVidBuf, vidSize, totalFrmCnt));
						if ((m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
							DBG_WRN("Wait 1st I-frm:%d, vidStrm exist=%d\r\n", tmpIdx, gUvcNoVidStrm);
						}
						pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V1);
						continue;
					}
				} else { //UVAC_VIDEO_FORMAT_H264
					if ((0 == vidSize) || (FALSE == _is_iframe(pVidBuf))) {
						DbgMsg_UVCIO(("Chk I-Frm [%d]addr=0x%lx,0x%x,cnt=0x%x\r\n", tmpIdx, (uintptr_t)pVidBuf, vidSize, totalFrmCnt));
						if ((m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
							DBG_WRN("Wait 1st I-frm:%d, vidStrm exist=%d\r\n", tmpIdx, gUvcNoVidStrm);
						}
						pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V1);
						continue;
					}
				}
			}
			pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

			txfInfo.sAddr = txfInfo.oriAddr = (uintptr_t)pVidBuf;
			txfInfo.size = vidSize;
			txfInfo.usbEP = UVAC_USB_EP[thisTxfQueIdx];
			txfInfo.txfCnt = 0;
			txfInfo.streamType = UVAC_TXF_STREAM_VID;
			txfInfo.timestamp = pStrmQueInfo->timestamp;
			txfInfo.uvc_timestamp_start = pStrmQueInfo->uvc_timestamp_start;
			txfInfo.frame_type = frame_type;
			UVAC_AddIsoInTxfInfo(&txfInfo, thisTxfQueIdx);
			//UVAC_IsoInTxfStateMachine(UVC_ISOIN_TXF_ACT_TXF);
			totalFrmCnt++;
			if ((m_uiUvacDbgVid & UVAC_DBG_VID_QUEUE)) {
				//DBG_DUMP("[0]0x%X:%dKB %dms\r\n", txfInfo.sAddr, txfInfo.size, Perf_GetDuration() / 1000);
				//Perf_Mark();
			}
			if (gU3UvacRunningFlag[thisVidIdx] & FLGUVAC_STOP) {
				DbgMsg_UVC(("Stop B ...\r\n"));
				break;
			}

			#if 0
			if (usb_getControllerState() == USB_CONTROLLER_STATE_SUSPEND) {
				if ((usb_getControllerState() == USB_CONTROLLER_STATE_SUSPEND)) {
					DbgMsg_UVC(("Clr StartFlag!!\r\n"));
					gUvcUsbDMAAbord = TRUE;
					clr_flg(FLG_ID_UVAC, FLGUVAC_START);
				}
				DbgMsg_UVC(("Stop E ...\r\n"));
				break;
			}
			#endif
			uiQueCnt = pStrmQueInfo->idxProducer + UVAC_VID_INFO_QUE_MAX_CNT - pStrmQueInfo->idxConsumer;
			if (uiQueCnt >= UVAC_VID_INFO_QUE_MAX_CNT) {
				uiQueCnt -= UVAC_VID_INFO_QUE_MAX_CNT;
			}
			if (m_uiUvacDbgVid & UVAC_DBG_VID_QUE_CNT) {
				DBG_DUMP("V1 Q=%d\r\n", uiQueCnt);
			}
			#if 0
			if (uiQueCnt > 1 && ((UVAC_VIDEO_FORMAT_MJPG == gUvacCodecType[thisVidIdx]) || (UVAC_VID_I_FRM_MARK == pVidBuf[UVAC_VID_I_FRM_MARK_POS]))) {
				if (m_uiUvacDbgVid & UVAC_DBG_VID_START) {
					DBG_DUMP("V1[%d] race!\r\n", totalFrmCnt);
				}
				goto V1_SKIP_TIMER;
			}
			#else
			if (uiQueCnt) {
				goto V1_SKIP_TIMER;
			}
			#endif

		}// end of wait timer //end of while (FALSE == gUvcNoVidStrm)

L_UVAC_END:
		clr_flg(FLG_ID_UVAC, FLGUVAC_START);
		DbgMsg_UVC(("+UVAC end:Fmt[0]=%d\n\r", gUvacCodecType[thisVidIdx]));
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
		usb_abortEndpoint(UVAC_USB_EP[thisTxfQueIdx]);
#else
		usb3dev_abortEndpoint(UVAC_USB_EP[thisTxfQueIdx]);
#endif
		//DbgMsg_UVC(("--Abort EP#%d, retV=%d\n\r", UVAC_USB_EP[thisTxfQueIdx], retV));
		UVC_TimeTrigClose(thisVidIdx);
		DbgMsg_UVC(("+wait isoTsk :0x%x\n\r", uiFlag));
		//wai_flg(&uiFlag, FLG_ID_UVAC, FLGUVAC_ISOIN_READY, (TWF_CLR | TWF_ORW));

		if (0 == kchk_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_VDO_READY)) {
			vos_util_delay_ms(70);
		}
		{
			UINT32 count = 0;
			while (1) {
				if (count < 1000) {
					if (0 == kchk_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_VDO_READY)) {
						usb3dev_abortEndpoint(UVAC_USB_EP[thisTxfQueIdx]);
						DbgMsg_UVC(("\n\r#### abort EP[%d] ####\n\r\n\r", UVAC_USB_EP[thisTxfQueIdx]));
						vos_util_delay_ms(10);
					} else {
						break;
					}
				} else {
					DBG_ERR("wait ISOIN_VDO_READY failed\r\n");
					break;
				}
				count++;
			}
		}

		DbgMsg_UVC(("-wait isoTsk :0x%x\n\r", uiFlag));
		gU3UvacRunningFlag[thisVidIdx] = 0;
		clr_flg(FLG_ID_UVAC, FLGUVAC_STOP);
		//reset stream queue index
		pStrmQueInfo->idxProducer = 0;
		pStrmQueInfo->idxConsumer = 0;

		if (g_fpStopVideo) {
			g_fpStopVideo(thisVidIdx);
			pStrmQueInfo->strmInfo.isStrmOn = strmInfo.isStrmOn = FALSE;
		}
		#if ISF_LATENCY_DEBUG
		UVAC_DbgDmp_TimestampNumReset();
		#endif
		DbgMsg_UVC(("-UVAC end\n\r"));
	} // end for while(1)

	return 0;
}



/*
    Set USB UVAC frame rate.
*/
int UVAC_SetFrameRate(UINT16 uhFrameRate, UINT32 vidDevIdx)
{
	UVAC_PARAM *pUvacParam = &gUvcParamAry[vidDevIdx];
	DbgMsg_UVC(("+FRate=%d,%d, vidDevIdx=%d\r\n", uhFrameRate, pUvacParam->FrameRate, vidDevIdx));
	//pUvacParam->FrameRate = (uhFrameRate > UVC_FRMRATE_60) ? UVC_FRMRATE_60 : uhFrameRate;
	pUvacParam->FrameRate = uhFrameRate;
	DbgMsg_UVC(("-FRate=%d,%d\r\n", uhFrameRate, pUvacParam->FrameRate));
	return E_OK;
}


/*
    Set UVAC frame size.

    @param uhSize Frame size ID.
*/
int UVAC_SetImageSize(UINT16 frmIdx, UINT32 thisVidDevIdx)
{
	UINT32 uhSize = (UVC_VSFMT_PREV_FRM_IDX == frmIdx) ? gUvcParamAry[thisVidDevIdx].OutSizeID : frmIdx;
	DbgMsg_UVC(("^G+-SetImgSize=%d/%d,Prev=%d,thisVidDevIdx=%d\r\n", frmIdx, uhSize, gUvcParamAry[thisVidDevIdx].OutSizeID, thisVidDevIdx));
	gUvcParamAry[thisVidDevIdx].OutSizeID = uhSize;
	return E_OK;
}

#if 0
UINT32 UVAC_GetConfig(UVAC_CONFIG_ID ConfigID)
{
	UINT32 retV = 0;
	switch (ConfigID) {
	case UVAC_CONFIG_MTP_DEVICE_FREIEDLYNAME:
		retV = (UINT32)UVAC_MTPGetDeviceFriendlyName();
		break;
	case UVAC_CONFIG_PTP_BATTERY_LEVEL:
		retV = (UINT32)UVAC_PTPGetBatteryLevel();
		break;
	default:
		DBG_ERR("Not Support the config:[%d]\r\n", ConfigID);
		break;
	}
	return retV;
}
#endif

#if 0//gUvacMtpEnabled
static void UVAC_SidcSetCardLock(UINT16 cardlock)
{
	DBG_MSG("[IO]+-usbsidc_setCardLock:%d\r\n", cardlock);
	gUvcSidcCardLock = cardlock;
}

UINT16 UVAC_SidcGetCardLock(void)
{
	DBG_MSG("[IO]+-usbsidc_getCardLock:0x%x\r\n", gUvcSidcCardLock);
	return gUvcSidcCardLock;
}

//need a buffer to save coming file !!
static void UVAC_SidcSetSendObjBufAddrSize(UINT32 addr, UINT32 size)
{
	DBG_MSG("[IO]+Sidc_SetSendObjBufAddrSize:addr=0x%x,size=0x%x\r\n", addr, size);
	g_uiUvcSidcPTPUpdateFWAddr = addr;
	g_uiUvcSidcPTPUpdateFWSize = size;
}


UINT32 UVAC_SidcGetSidcPTPUpdateFWBuf(void)
{
	return g_uiUvcSidcPTPUpdateFWAddr;
}

UINT32 UVAC_SidcGetSidcPTPUpdateFWBufSize(void)
{
	return g_uiUvcSidcPTPUpdateFWSize;
}

void UVAC_SIDCTsk(void)
{
	FLGPTN    uiFlag = 0;

	kent_tsk();

	DBG_MSG("[IO]+-UVAC_SIDCTsk\r\n");

	UVAC_SidcInit();
	clr_flg(FLG_ID_SIDC, FLGUVAC_SIDC_MASK);
	while (UVACSIDCTSK_ID) {
		PROFILE_TASK_IDLE();
		wai_flg(&uiFlag, FLG_ID_SIDC, FLGUVAC_SIDC_BULKOUT2 | FLGUVAC_SIDC_SIDCSTOP, TWF_ORW | TWF_CLR);
		PROFILE_TASK_BUSY();
		DBG_MSG("[IO]UVAC_SIDCTsk Got flag=0x%x\r\n", uiFlag);
		if (uiFlag & FLGUVAC_SIDC_BULKOUT2) {
			UVAC_SidcProcessOutCmd();
		}
	}
}

/**
    USB SIDC Interrupt Task.

    Processing of USB SIDC Interrupt Task.
*/
void UVAC_SIDCINTRTsk(void)
{
	FLGPTN    uiFlag = 0;
	UINT32    len = 0;
	kent_tsk();
	DBG_MSG("[IO]+-UVAC_SIDCINTRTsk\r\n");

	clr_flg(FLG_ID_SIDCINTR, FLGUVAC_SIDC_INTR_START);
	while (UVACSIDCINTRTSK_ID) {
		PROFILE_TASK_IDLE();
		wai_flg(&uiFlag, FLG_ID_SIDCINTR, FLGUVAC_SIDC_INTR_START, TWF_CLR);
		PROFILE_TASK_BUSY();
		DBG_MSG("[IO]UVAC_SIDCINTRTsk Got flag=0x%x\r\n", uiFlag);
		len = g_UvcSidcIntrCmd.Len;
		UVAC_SidcSendIntrEventToHost(g_UvcSidcIntrCmd.IntEvent, &len, g_UvcSidcIntrCmd.Param1);
#if (THIS_DBGLVL >= 2)
		if (len != g_UvcSidcIntrCmd.Len) {
			DBG_ERR("UVAC_SIDCINTRTsk send data fail:%d,act=%d\r\n", len, g_UvcSidcIntrCmd.Len);
		}
#endif
		set_flg(FLG_ID_SIDCINTR, FLGUVAC_SIDC_INTR_END);
	}
}

ER UVAC_SidcAddObjectByPath(char *FilePath)
{
	char filename[KFS_LONGFILENAME_MAX_LENG + 1] = {0};
	int retV = 0;
	char parent[KFS_LONGFILENAME_MAX_LENG + 1] = {0};
	PSIDC_OBJECTLINK    pSIDCParent;
	PSIDC_OBJECTLINK    pSIDCBlock;
	UINT32 ui32TmpDirId = 0;
	FST_FILE pFileHdl = 0;
	FST_FILE_STATUS pFileSts = {0};
	CHAR *extoffset = 0;

	if (!gUvacOpened) {
		DBG_ERR("UVAC is not opened!\r\n");
		return E_SYS;
	}

	if (!gUvacMtpEnabled) {
		DBG_ERR("MTP is not enabled!\r\n");
		return E_SYS;
	}

	retV = FileSys_GetParentDir(FilePath, parent);
	//DBG_DUMP("FilePath:%s\r\n",FilePath);
	//DBG_DUMP("Parent:%s\r\n",parent);
	if (retV != FST_STA_OK) {
		DBG_ERR("GetParentDir %s failed\r\n", FilePath);
		return E_SYS;
	}

	if (strncmp(parent, "A:\\", strlen(parent)) == 0) {
		//Parent is root
		//DBG_DUMP("Parent is root. FilePath:%s\r\n",FilePath);
		pSIDCParent = NULL;
	} else {
		pSIDCParent = UVAC_usbsidc_findObjectByPath(parent);
		if (pSIDCParent == NULL) {
			//DBG_DUMP("Parent is not added\r\n");
			UVAC_SidcAddObjectByPath(parent);
			pSIDCParent = UVAC_usbsidc_findObjectByPath(parent);
		}
	}

	//File in root folder
	if (!M_IsDirectory(pFileSts.uiAttrib) && pSIDCParent == NULL) {
		//DBG_DUMP("File in root folder.\r\n");
		return E_OK;
	}

	if (strlen(FilePath + strlen(parent)) > KFS_LONGFILENAME_MAX_LENG) {
		DBG_ERR("File name is too long\r\n");
		return E_SYS;
	} else {
		strncpy(filename, FilePath + strlen(parent), strlen(FilePath + strlen(parent)));
	}
	//Get file status
	pFileHdl = FileSys_OpenFile(FilePath, FST_OPEN_READ | FST_OPEN_EXISTING);
	if (!pFileHdl) {
		DBG_ERR("File is not exist!\r\n");
		return E_SYS;
	}
	retV = FileSys_StatFile(pFileHdl, &pFileSts);
	FileSys_CloseFile(pFileHdl);

	if (retV != FSS_OK) {
		DBG_ERR("Get file status failed!\r\n");
		return E_SYS;
	}

	pSIDCBlock = UVAC_usbsidc_getBlock();
	if (pSIDCBlock == NULL) {
		DBG_ERR("No available block\r\n");
		return E_SYS;
	}

	//Attribute
	pSIDCBlock->Attrib = pFileSts.uiAttrib;
	if (UVAC_SidcGetCardLock()) {
		pSIDCBlock->Attrib |= FS_ATTRIB_READ;
	}

	//Modified date and time
	pSIDCBlock->Time = pFileSts.uiModifiedTime;
	pSIDCBlock->Date = pFileSts.uiModifiedDate;
	//File name
	snprintf(pSIDCBlock->FileName, sizeof(pSIDCBlock->FileName), "%s", filename);

	//Folder
	if (M_IsDirectory(pFileSts.uiAttrib)) {
		pSIDCBlock->ObjectFormat = FORMAT_A_ASSOCIATION;
		pSIDCBlock->FileName[strlen(pSIDCBlock->FileName) - 1] = '\0'; //Remove '\'
		wai_sem(SEMID_UVC_DIR_SIDC);
		g_UvacDirNumObject += 1;
		sig_sem(SEMID_UVC_DIR_SIDC);
		//DBG_DUMP("Add folder=%s, parent=%s\r\n", pSIDCBlock->FileName, pSIDCParent->FileName);
	}
	//File
	else if (pSIDCParent) {
		extoffset = UVAC_FindExtOffset(filename);
		if (extoffset) {
			//DBG_DUMP("EXT=%s\r\n", extoffset);
			pSIDCBlock->ObjectFormat = UVAC_PTPGetObjectFormatVendorSupport((char *) extoffset);
		} else {
			pSIDCBlock->ObjectFormat = 0;
		}
		if (pSIDCBlock->ObjectFormat == FORMAT_I_EXIF_JPEG) {
			ui32TmpDirId = 0;
			if ((pSIDCBlock->pParent &&
				 pSIDCBlock->pParent->pParent &&
				 !pSIDCBlock->pParent->pParent->pParent) &&
				!strcmp(pSIDCBlock->pParent->pParent->FileName, "DCIM")) {
				ui32TmpDirId = UVAC_PTPIsValidDCFFolder(pSIDCBlock->pParent->FileName);

				pSIDCBlock->dirId = ui32TmpDirId;
				if (ui32TmpDirId) {  // if its dad is a DCF folder.
					pSIDCBlock->fileId = UVAC_PTPIsValidDCFJPGFile(filename);
				}
			}
		}
		pSIDCBlock->ObjectSize = pFileSts.uiFileSize;
		//DBG_DUMP("Add file=%s, parent=%s\r\n", pSIDCBlock->FileName, pSIDCParent->FileName);
	}
	UVAC_SidcAddObject(pSIDCParent, pSIDCBlock);

#if 0
	//Send object added event to host
	FLGPTN uiFlag;

	g_UvcSidcIntrCmd.IntEvent = EVENT_OBJECT_ADDED;
	g_UvcSidcIntrCmd.Len = USIDC_RSP_RetParam1;
	g_UvcSidcIntrCmd.Param1 = g_UvacObjectHandle;
	DBG_DUMP("g_UvacObjectHandle=%x\r\n", g_UvacObjectHandle);
	clr_flg(FLG_ID_SIDCINTR, FLGUVAC_INTR_END);
	set_flg(FLG_ID_SIDCINTR, FLGUVAC_INTR_START);
	wai_flg(&uiFlag, FLG_ID_SIDCINTR, FLGUVAC_INTR_END, TWF_CLR | TWF_ORW);
#endif

	return E_OK;
}
#endif

static BOOL U3UVAC_is_rt_linux(void)
{
	//parse cat string
	FILE * fp;
	char buffer[40];
	BOOL ret = FALSE;

	fp = popen("cat /proc/version","r");

	if (fp == NULL) {
		DBG_ERR("proc failed\r\n");
		return 0;
	}

	if ((fgets(buffer, sizeof(buffer), fp)) != NULL) {
		if (strstr(buffer, "rt64") != NULL) {
		   ret = TRUE;
		}
	}

	pclose(fp);

	return ret;
}

/**
    Open USB PCC Task.

    @param[in] pClassInfo USB PCC task open parameters.

*/
UINT32 U3UVAC_Open(UVAC_INFO *pClassInfo)
{
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
	UINT32 i;
	USB_MNG USBMng;
	UINT32 UvcBlkSize;
#endif
	BOOL is_rt_linux = FALSE;
	UINT32 uvac_channel = 0;
	UINT32 disable_uac = 0;

	DbgMsg_UVCIO(("+%s:0x%x\r\n", __func__, pClassInfo));
	UVAC_DbgDmp_VendDevDesc(gpImgUnitUvacVendDevDesc);
	UVAC_DbgDmp_UvacInfo(pClassInfo);

	is_rt_linux = U3UVAC_is_rt_linux();

	if (gUvacOpened) {
		DBG_ERR("UVAC is already opened\r\n");
		return E_OK;
	}

	if (NULL == pClassInfo) {
		DBG_ERR("!!!!!!Input NULL\r\n");
		DBG_ERR("!!!!!!Input NULL\r\n");
		return E_PAR;
	}
	U3UVAC_InstallID();
	if (_check_payload_tx_cnt()) {
		DBG_ERR("The cnt of MAX_PAYLOAD_TX_SIZE not match resolution table!\r\n");
		return E_SYS;
	}
	//xUVAC_InstallCmd();
	UVAC_ResetParam();

#if (UVAC_CONTROLLER == UVAC_CONTROL_U3)
	//UINT32 uiU3DevBuf;
	//uiU3DevBuf = pClassInfo->UvacMemAdr;
	//guiUvacBufAddr = uiU3DevBuf + U3DEV_MIN_BUFFER_SIZE;
	//guiUvacBufTotalSize = pClassInfo->UvacMemSize - U3DEV_MIN_BUFFER_SIZE;
	guiUvacBufAddr = (uintptr_t)hwmem_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pClassInfo->UvacMemAdr, pClassInfo->UvacMemSize);
	guiUvacBufAddr_pa = pClassInfo->UvacMemAdr;
	guiUvacBufTotalSize = pClassInfo->UvacMemSize;
#else
	guiUvacBufAddr = pClassInfo->UvacMemAdr;
	guiUvacBufTotalSize = pClassInfo->UvacMemSize;
#endif
	if (!guiUvacBufAddr || (pClassInfo->UvacMemSize < UVAC_GetNeedMemSize())) {
		DBG_ERR("!!!!!!!addr=0x%x,size=0x%x\r\n", guiUvacBufAddr, guiUvacBufTotalSize);
		DBG_ERR("!!!!!!!addr=0x%x,size=0x%x\r\n", guiUvacBufAddr, guiUvacBufTotalSize);
		return E_PAR;
	}
	if (guiUvacBufAddr % 4) {
		DBG_ERR("Input MemAddr not 4-byte alignment = 0x%x\r\n", guiUvacBufAddr);
		guiUvacBufAddr += 0x03;
		guiUvacBufAddr &= 0xFFFFFFFC;
		guiUvacBufTotalSize = (pClassInfo->UvacMemAdr + pClassInfo->UvacMemSize) - guiUvacBufAddr;
	}
	gUvcHWPayload[UVAC_VID_DEV_CNT_1] = FALSE;//pClassInfo->hwPayload[UVAC_VID_DEV_CNT_1]; USB3.0 does not support hw payload
	gUvcHWPayload[UVAC_VID_DEV_CNT_2] = FALSE;//pClassInfo->hwPayload[UVAC_VID_DEV_CNT_2]; USB3.0 does not support hw payload
	gUvcHWPayload[UVAC_VID_DEV_CNT_3] = FALSE;//pClassInfo->hwPayload[UVAC_VID_DEV_CNT_3]; USB3.0 does not support hw payload
	gUvacChannel = pClassInfo->channel;
	g_fpStartVideo = pClassInfo->fpStartVideoCB;
	//g_fpGetH264Header = pClassInfo->fpGetH264HeaderCB;
	g_fpStopVideo = pClassInfo->fpStopVideoCB;
#if 0
	g_fpGetStrmCB = pClassInfo->fpGetStrmCB;
#endif
	gUacSetVolCB = pClassInfo->fpSetVolCB;

	uvac_channel = (gUvacChannel > gHsUvacChannel)? gUvacChannel : gHsUvacChannel;

	if (gU3DisableUac == 0 || gU3HsDisableUac == 0) {
		disable_uac = 0;
	} else {
		disable_uac = 1;
	}

	if (uvac_channel > UVAC_CHANNEL_1V1A) {

		if (uvac_channel == UVAC_CHANNEL_3V1A) {
			UINT32 uvc_iso_fifo = disable_uac? 9 : 6;

			if (u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_1]) {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
			} else {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_FIFO_NUM, uvc_iso_fifo);
				#if 0
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_PATCH2, 1);
				#else
				usb3dev_SetIsoBuff_HW(UVAC_USB_EP[UVAC_TXF_QUE_V1], TRUE);
				#endif
			}
			if (u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_2]) {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V2], U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
			} else {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V2], U3DEV_EP_CONFIG_ID_FIFO_NUM, uvc_iso_fifo);
				#if 0
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V2], U3DEV_EP_CONFIG_ID_PATCH2, 1);
				#else
				usb3dev_SetIsoBuff_HW(UVAC_USB_EP[UVAC_TXF_QUE_V2], TRUE);
				#endif
			}
			if (u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_3]) {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V3], U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
			} else {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V3], U3DEV_EP_CONFIG_ID_FIFO_NUM, uvc_iso_fifo);
				#if 0
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V3], U3DEV_EP_CONFIG_ID_PATCH2, 1);
				#else
				usb3dev_SetIsoBuff_HW(UVAC_USB_EP[UVAC_TXF_QUE_V3], TRUE);
				#endif
			}

			usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_A1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 6);

			if (gUvacUacRxEnabled) {
				usb3dev_setEPConfig(UVAC_USB_RX_EP[UVAC_RXF_QUE_A1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
			}

		} else {
			UINT32 uvc_iso_fifo = disable_uac? 10 : 6;

			if (u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_1]) {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
			} else {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_FIFO_NUM, uvc_iso_fifo);
				#if 0
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_PATCH2, 1);
				#else
				usb3dev_SetIsoBuff_HW(UVAC_USB_EP[UVAC_TXF_QUE_V1], TRUE);
				#endif
			}
			usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_A1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 6);
			if (u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_2]) {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V2], U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
			} else {
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V2], U3DEV_EP_CONFIG_ID_FIFO_NUM, uvc_iso_fifo);
				#if 0
				usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V2], U3DEV_EP_CONFIG_ID_PATCH2, 1);
				#else
				usb3dev_SetIsoBuff_HW(UVAC_USB_EP[UVAC_TXF_QUE_V2], TRUE);
				#endif
			}
			usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_A2], U3DEV_EP_CONFIG_ID_FIFO_NUM, 6);

			if (gUvacUacRxEnabled) {
				usb3dev_setEPConfig(UVAC_USB_RX_EP[UVAC_RXF_QUE_A1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
			}
		}

		//printf("HS v1 unit = %d, v2 unit = %d\r\n", gU3UvcIsoInHsPacketSize[UVAC_VID_DEV_CNT_1]*gU3UvcIsoInHsBandWidth[UVAC_VID_DEV_CNT_1], gU3UvcIsoInHsPacketSize[UVAC_VID_DEV_CNT_2]*gU3UvcIsoInHsBandWidth[UVAC_VID_DEV_CNT_2]);
		//printf("SS v1 unit = %d, v2 unit = %d\r\n", gU3UvcIsoInSsPacketSize[UVAC_VID_DEV_CNT_1]*gU3UvcIsoInSsBandWidth[UVAC_VID_DEV_CNT_1], gU3UvcIsoInSsPacketSize[UVAC_VID_DEV_CNT_2]*gU3UvcIsoInSsBandWidth[UVAC_VID_DEV_CNT_2]);
	} else {// (gU2UvacChannel == UVAC_CHANNEL_1V1A) {
		//gU3UvcIsoInHsPacketSize[UVAC_VID_DEV_CNT_1] = UVC_ISOIN_HS_PACKET_SIZE;
		//gU3UvcIsoInHsBandWidth[UVAC_VID_DEV_CNT_1]  = BLKNUM_TRIPLE;

		//gU3UvcIsoInSsMaxBurst[UVAC_VID_DEV_CNT_1] = 0x0F;
		//gU3UvcIsoInSsEpcAttr[UVAC_VID_DEV_CNT_1]  = 0x01;
		//gU3UvcIsoInSsPacketSize[UVAC_VID_DEV_CNT_1] = UVC_ISOIN_HS_PACKET_SIZE;
		//gU3UvcIsoInSsBandWidth[UVAC_VID_DEV_CNT_1]  = (gU3UvcIsoInSsMaxBurst[UVAC_VID_DEV_CNT_1]+1)*(gU3UvcIsoInSsEpcAttr[UVAC_VID_DEV_CNT_1]+1);
		//printf("HS v1 unit = %d\r\n", gU3UvcIsoInHsPacketSize[UVAC_VID_DEV_CNT_1]*gU3UvcIsoInHsBandWidth[UVAC_VID_DEV_CNT_1]);
		//printf("SS v1 unit = %d\r\n", gU3UvcIsoInSsPacketSize[UVAC_VID_DEV_CNT_1]*gU3UvcIsoInSsBandWidth[UVAC_VID_DEV_CNT_1]);
		if (u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_1]) {
			usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
		} else {
			usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 10);

			#if 0
			usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_PATCH2, 1);
			#else
			usb3dev_SetIsoBuff_HW(UVAC_USB_EP[UVAC_TXF_QUE_V1], TRUE);
			#endif
		}
		usb3dev_setEPConfig(UVAC_USB_EP[UVAC_TXF_QUE_A1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 10);

		usb3dev_setEPConfig(UVAC_USB_INTR_EP[UVAC_TXF_QUE_V1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 1);

		if (gUvacUacRxEnabled) {
			usb3dev_setEPConfig(UVAC_USB_RX_EP[UVAC_RXF_QUE_A1], U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
		}
	}
	if (g_u3_hid_info.en) {
		usb3dev_setEPConfig(HID_INTRIN_EP, U3DEV_EP_CONFIG_ID_FIFO_NUM, 1);
		if (g_u3_hid_info.intr_out) {
			usb3dev_setEPConfig(HID_INTROUT_EP, U3DEV_EP_CONFIG_ID_FIFO_NUM, 1);
		}
	}

	if (g_u3_msdc_info.en) {
		usb3dev_setEPConfig(msdc_in_ep, U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
		usb3dev_setEPConfig(msdc_out_ep, U3DEV_EP_CONFIG_ID_FIFO_NUM, 3);
	}

	if (gUvacCdcEnabled[CDC_COM_1ST]) {
#if USB_CDC_IF_COMM_NUMBER_EP
		usb3dev_setEPConfig(CDC_COMM_IN_EP, U3DEV_EP_CONFIG_ID_FIFO_NUM, 1);
#endif
		usb3dev_setEPConfig(CDC_DATA_IN_EP, U3DEV_EP_CONFIG_ID_FIFO_NUM, 1);
		usb3dev_setEPConfig(CDC_DATA_OUT_EP, U3DEV_EP_CONFIG_ID_FIFO_NUM, 1);
	}
	if (gUvacCdcEnabled[CDC_COM_2ND]) {
#if USB_CDC_IF_COMM_NUMBER_EP
		usb3dev_setEPConfig(CDC_COMM2_IN_EP, U3DEV_EP_CONFIG_ID_FIFO_NUM, 1);
#endif
		usb3dev_setEPConfig(CDC_DATA2_IN_EP, U3DEV_EP_CONFIG_ID_FIFO_NUM, 1);
		usb3dev_setEPConfig(CDC_DATA2_OUT_EP, U3DEV_EP_CONFIG_ID_FIFO_NUM, 1);
	}

	UVAC_SetupMem();
	UVAC_ResetTxfPara();//must be called after gUvcHWPayload and gUvacChannel
	UVAC_SetIntfString();

	DbgMsg_UVC(("UVAC:ch=%d,codec=%d,%d,adr=0x%x,size=0x%x,HWPL=%d,%d\r\n", pClassInfo->channel, gUvacCodecType[UVAC_VID_DEV_CNT_1], gUvacCodecType[UVAC_VID_DEV_CNT_2], guiUvacBufAddr, guiUvacBufTotalSize, gUvcHWPayload[UVAC_VID_DEV_CNT_1], gUvcHWPayload[UVAC_VID_DEV_CNT_2]));

	UVAC_MakeHSConfigDesc(gpUvacHSConfigDesc, UVAC_MEM_DESC_SIZE);

#if (UVAC_SUPPORT_SUPER_SPEED == ENABLE)
	UVAC_MakeSSConfigDesc(gpUvacSSConfigDesc, UVAC_MEM_DESC_SIZE);
#endif

	//UVC_InitStillProbeCommitData(&gUvcStillProbeCommit);

	//clr_flg(FLG_ID_UVAC, (FLGUVAC_START|FLGUVAC_STOP|FLGUVAC_START2|FLGUVAC_STOP2));
	//clr_flg(FLG_ID_UVAC, FLGUVAC_MASK);

	if (uvc_direct_trigger) {
		clr_flg(FLG_ID_UVAC, (FLGUVAC_VIDEO_TXF | FLGUVAC_VIDEO_STOP | FLGUVAC_VIDEO_EXIT | FLGUVAC_ISOIN_VDO_EXIT | FLGUVAC_ISOIN_AUD_EXIT |
							  FLGUVAC_VIDEO2_TXF | FLGUVAC_VIDEO2_STOP | FLGUVAC_VIDEO2_EXIT | FLGUVAC_ISOIN_VDO2_EXIT | FLGUVAC_ISOIN_AUD2_EXIT));
		clr_flg(FLG_ID_UVAC_UVC3, (FLGUVAC_VIDEO3_TXF | FLGUVAC_VIDEO3_STOP | FLGUVAC_VIDEO3_EXIT | FLGUVAC_ISOIN_VDO3_EXIT));
	}
	if (gUvacCdcEnabled[CDC_COM_1ST]) {
		clr_flg(FLG_ID_UVAC, (FLGUVAC_CDC_DATA_OUT | FLGUVAC_CDC_ABORT_READ));
	}
	if (gUvacCdcEnabled[CDC_COM_2ND]) {
		clr_flg(FLG_ID_UVAC, (FLGUVAC_CDC_DATA2_OUT | FLGUVAC_CDC_ABORT_READ2));
	}
	if (gUvacUacRxEnabled) {
		clr_flg(FLG_ID_UVAC_UAC_RX, (FLGUVAC_AUD_DATA_OUT | FLGUVAC_AUD_EXIT));
	}

	vos_flag_set(FLG_ID_UVAC_FRM, (FLGUVAC_FRM_V1 | FLGUVAC_FRM_V2 | FLGUVAC_FRM_V3 | FLGUVAC_FRM_A1 | FLGUVAC_FRM_A2));
	#if 0//gUvacMtpEnabled
	if (gUvacMtpEnabled) {
		guiUvacBufTotalSize -= SIDC_WORKING_BUFFER + MIN_BUFFER_SIZE_FOR_SENDOBJ;
		guiUvcSidcBufAddr = guiUvacBufAddr + gUvcMaxVideoFmtSize * gUvacChannel + UVAC_MEM_DESC_SIZE;
		guiUvcSidcBufSize = SIDC_WORKING_BUFFER;
		UVAC_SidcSetSendObjBufAddrSize(guiUvcSidcBufAddr + SIDC_WORKING_BUFFER, MIN_BUFFER_SIZE_FOR_SENDOBJ);
#if _UVAC_SDXC_MTP_MOUNT_NOTACCESS_
		gUvacSIDCGetCardTypeFunc = pClassInfo->pGetCardTypeFunc;
#endif
		UVAC_SidcSetCardLock(pClassInfo->uiStrgCardLock);
		DbgMsg_UVC(("MTP:adr=0x%x,size=0x%x\r\n", guiUvcSidcBufAddr, guiUvcSidcBufSize));
		clr_flg(FLG_ID_UVAC, FLGUVAC_SIDC_MASK);
	}
	#endif

	/* Set events callback functions */
	//usb3dev_setCallBack(U3DEV_CALLBACK_ID_CX_VENDOR_REQUEST, UvacVendorRequestHandler);
	usb3dev_setCallBack(U3DEV_CALLBACK_ID_CX_CLASS_REQUEST,  UvacClassRequestHandler);
	usb3dev_setCallBack(U3DEV_CALLBACK_ID_CX_CLASS_REQDONE,  UvacClassRequestDone);
	usb3dev_setCallBack(U3DEV_CALLBACK_ID_SET_INTERFACE,      UVAC_InterfaceCB);
	usb3dev_setCallBack(U3DEV_CALLBACK_ID_EVENT_NOTIFY,      UVAC_Callback);
	usb3dev_setCallBack(U3DEV_CALLBACK_ID_SET_CONFIGURATION, UVAC_OpenNeededFIFO);
	usb3dev_setCallBack(U3DEV_CALLBACK_ID_CX_VENDOR_REQUEST,  UvacVendorRequestHandler);
	usb3dev_setCallBack(U3DEV_CALLBACK_ID_CX_VENDOR_REQDONE,  UvacVendorRequestDone);

	#if SUSPEND_RESUME_FUNC
	if (g_fpSuspend_CB) {
		DBG_DUMP("Enable suspend function.\r\n");
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_SUSPEND, UVAC_suspend_cb);
		usb3dev_setConfig(U3DEV_CONFIG_ID_SUSPEND_DRAMOFF, ENABLE);
	}
	#endif

	if (g_u3_hid_info.en) {
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_STD_UNKNOWN_REQ, U3UvacStdRequestHandler);
	}

	if (g_u3_msos_info.en) {
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_BOS, (void *)&gUvacSSBosMSOSDesc);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_BOS, (void *)&gUvacHSBosMSOSDesc);
		g_UVACDevDesc.bcd_usb = 0x0210;
	} else {
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_BOS, (void *)&gUvacSSBosDesc);
	}

	/* Set SS/HS/FS Descriptors */
	usb3dev_setDescriptor(U3DEV_DESC_ID_SS_DEVICE, (void *)&g_UVACSSDevDesc);
	usb3dev_setDescriptor(U3DEV_DESC_ID_HS_DEVICE, (void *)&g_UVACDevDesc);
	//usb3dev_setDescriptor(U3DEV_DESC_ID_FS_DEVICE,           NULL);
	// USB3 Std 9-14. SuperSpeed has no Device Qualifier Descriptor
	usb3dev_setDescriptor(U3DEV_DESC_ID_HS_DEVICEQUALIFIER, (void *)&g_UVACDevQualiDesc);
	//usb3dev_setDescriptor(U3DEV_DESC_ID_FS_DEVICEQUALIFIER,  NULL);

#if (UVAC_SUPPORT_SUPER_SPEED == ENABLE)
	usb3dev_setDescriptor(U3DEV_DESC_ID_SS_CONFIG, (void *)gpUvacSSConfigDesc);
#endif
	usb3dev_setDescriptor(U3DEV_DESC_ID_HS_CONFIG, (void *)gpUvacHSConfigDesc);
	//usb3dev_setDescriptor(U3DEV_DESC_ID_FS_CONFIG,           NULL);
	usb3dev_setDescriptor(U3DEV_DESC_ID_HS_OTHERSPEED, (void *)g_U3UVACHSOtherConfigDescFBH264);
	usb3dev_setDescriptor(U3DEV_DESC_ID_FS_OTHERSPEED, (void *)g_U3UVACFSOtherConfigDescFBH264);

	usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0, (void *)&UvacStrDesc0);
	usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0, (void *)&UvacStrDesc0);
	usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0, (void *)&UvacStrDesc0);

	if (0 == gpImgUnitUvacVendDevDesc) {
		DBG_IND("Poj Not set string, use internal\r\n");
		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING1, (void *)&UvacStrDesc1);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING1, (void *)&UvacStrDesc1);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING1, (void *)&UvacStrDesc1);

		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING2, (void *)&UvacStrDesc2);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING2, (void *)&UvacStrDesc2);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING2, (void *)&UvacStrDesc2);

		usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING3, (void *)&UvacStrDesc3);
		usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING3, (void *)&UvacStrDesc3);
		usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING3, (void *)&UvacStrDesc3);
	}

#if (UVAC_SUPPORT_SUPER_SPEED == ENABLE)
	usb3dev_setConfig(U3DEV_CONFIG_ID_SPEED, U3DEV_SPEED_SS);
#else
	usb3dev_setConfig(U3DEV_CONFIG_ID_SPEED, U3DEV_SPEED_HS);
#endif

	//usb3dev_setConfig(U3DEV_CONFIG_ID_BUFFER_ADDR, uiU3DevBuf);
	//usb3dev_setConfig(U3DEV_CONFIG_ID_BUFFER_SIZE, U3DEV_MIN_BUFFER_SIZE);

	//ben to do, check if VS alt 0 and AS alt 0 are necessary.
	usb3dev_setConfig(U3DEV_CONFIG_ID_INTERFACE_ALTERNATE, USB3DEV_INTF_ALT(gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_1], 1));
	usb3dev_setConfig(U3DEV_CONFIG_ID_INTERFACE_ALTERNATE, USB3DEV_INTF_ALT(gUvacIntfIdx_AS[UVAC_AUD_DEV_CNT_1], 1));
	if (uvac_channel > UVAC_CHANNEL_1V1A) {
		usb3dev_setConfig(U3DEV_CONFIG_ID_INTERFACE_ALTERNATE, USB3DEV_INTF_ALT(gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_2], 1));
		if (uvac_channel == UVAC_CHANNEL_2V2A) {
			usb3dev_setConfig(U3DEV_CONFIG_ID_INTERFACE_ALTERNATE, USB3DEV_INTF_ALT(gUvacIntfIdx_AS[UVAC_AUD_DEV_CNT_2], 1));
		}
		if (uvac_channel >= UVAC_CHANNEL_3V1A) {
			usb3dev_setConfig(U3DEV_CONFIG_ID_INTERFACE_ALTERNATE, USB3DEV_INTF_ALT(gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_3], 1));
		}
	}

	if (E_OK == usb3dev_open()) {

		#if 0
		UVACVIDEOTSK_ID = vos_task_create(UVAC_VideoTsk, 0, "UVAC_VideoTsk", PRI_UVACVIDEO, STKSIZE_UVACVIDEO);
		if (0 == UVACVIDEOTSK_ID) {
			DBG_ERR("VideoTsk create failed\r\n");
	        return E_SYS;
		}

		UVACISOINTSK_ID = vos_task_create(UVAC_IsoInTsk, 0, "UVAC_IsoInTsk", PRI_UVACISOIN, STKSIZE_UVACISOIN);
		if (0 == UVACISOINTSK_ID) {
			DBG_ERR("VideoTsk create failed\r\n");
	        return E_SYS;
		}
		vos_task_resume(UVACISOINTSK_ID);
		#endif

		#if 1
		{
			pthread_attr_t fd_attr;
			struct sched_param fd_param;

			pthread_attr_init(&fd_attr);

			if (!is_rt_linux) {
				fd_param.sched_priority = 99-PRI_UVACVIDEO;
				pthread_attr_setschedpolicy(&fd_attr,SCHED_RR);
				pthread_attr_setschedparam(&fd_attr,&fd_param);
				pthread_attr_setinheritsched(&fd_attr,PTHREAD_EXPLICIT_SCHED);
			}
			pthread_create(&UVACVIDEOTSK_ID, &fd_attr, UVAC_VideoTsk, 0);
			pthread_setname_np(UVACVIDEOTSK_ID, "UVAC_VideoTsk");

		}

		{
			pthread_attr_t fd_attr;
			struct sched_param fd_param;

			pthread_attr_init(&fd_attr);

			if (!is_rt_linux) {
				fd_param.sched_priority = 99-PRI_UVACISOIN;
				pthread_attr_setschedpolicy(&fd_attr,SCHED_RR);
				pthread_attr_setschedparam(&fd_attr,&fd_param);
				pthread_attr_setinheritsched(&fd_attr,PTHREAD_EXPLICIT_SCHED);
			}
			pthread_create(&UVAC_TX_VDO1_ID, &fd_attr, UVAC_IsoInVdo1Tsk, 0);
			pthread_setname_np(UVAC_TX_VDO1_ID, "UVAC_IsoInVid1T");

		}

		#else

		UVACVIDEOTSK_ID = vos_task_create(UVAC_VideoTsk, 0, "UVAC_VideoTsk", PRI_UVACVIDEO, STKSIZE_UVACVIDEO);
		if (0 == UVACVIDEOTSK_ID) {
			DBG_ERR("VideoTsk create failed\r\n");
	        return E_SYS;
		}
		vos_task_resume(UVACVIDEOTSK_ID);

		UVAC_TX_VDO1_ID = vos_task_create(UVAC_IsoInVdo1Tsk, 0, "UVAC_IsoInVdo1Tsk", PRI_UVACISOIN, STKSIZE_UVACISOIN_VID);
		if (0 == UVAC_TX_VDO1_ID) {
			DBG_ERR("UVAC_IsoInVdo1Tsk create failed\r\n");
	        return E_SYS;
		}
		vos_task_resume(UVAC_TX_VDO1_ID);
		#endif
		#if 1
		pthread_attr_t fd_attr;
		struct sched_param fd_param;

		pthread_attr_init(&fd_attr);

		if (!is_rt_linux) {
			fd_param.sched_priority = 99-PRI_UVACISOIN;
			pthread_attr_setschedpolicy(&fd_attr,SCHED_RR);
			pthread_attr_setschedparam(&fd_attr,&fd_param);
			pthread_attr_setinheritsched(&fd_attr,PTHREAD_EXPLICIT_SCHED);
		}
		pthread_create(&UVAC_TX_AUD1_ID, &fd_attr, UVAC_IsoInAud1Tsk, 0);
		pthread_setname_np(UVAC_TX_AUD1_ID, "UVAC_IsoInAud1T");
		#else
		UVAC_TX_AUD1_ID = vos_task_create(UVAC_IsoInAud1Tsk, 0, "UVAC_IsoInAud1Tsk", PRI_UVACISOIN, STKSIZE_UVACISOIN_AUD);
		if (0 == UVAC_TX_AUD1_ID) {
			DBG_ERR("UVAC_IsoInAud1Tsk create failed\r\n");
	        return E_SYS;
		}
		vos_task_resume(UVAC_TX_AUD1_ID);
		#endif

		//vos_task_resume(UVACVIDEOTSK_ID);
		if (uvac_channel > UVAC_CHANNEL_1V1A) {
			#if 1
			{
				pthread_attr_t fd_attr;
				struct sched_param fd_param;

				pthread_attr_init(&fd_attr);

				if (!is_rt_linux) {
					fd_param.sched_priority = 99-PRI_UVACVIDEO;
					pthread_attr_setschedpolicy(&fd_attr,SCHED_RR);
					pthread_attr_setschedparam(&fd_attr,&fd_param);
					pthread_attr_setinheritsched(&fd_attr,PTHREAD_EXPLICIT_SCHED);
				}
				pthread_create(&UVACVIDEOTSK_ID2, &fd_attr, UVAC_VideoTsk2, 0);
				pthread_setname_np(UVACVIDEOTSK_ID2, "UVAC_VideoTsk2");

			}

			{
				pthread_attr_t fd_attr;
				struct sched_param fd_param;

				pthread_attr_init(&fd_attr);

				if (!is_rt_linux) {
					fd_param.sched_priority = 99-PRI_UVACISOIN;
					pthread_attr_setschedpolicy(&fd_attr,SCHED_RR);
					pthread_attr_setschedparam(&fd_attr,&fd_param);
					pthread_attr_setinheritsched(&fd_attr,PTHREAD_EXPLICIT_SCHED);
				}
				pthread_create(&UVAC_TX_VDO2_ID, &fd_attr, UVAC_IsoInVdo2Tsk, 0);
				pthread_setname_np(UVAC_TX_VDO2_ID, "UVAC_IsoInVid2T");

			}

			#else
			UVACVIDEOTSK_ID2 = vos_task_create(UVAC_VideoTsk2, 0, "UVAC_VideoTsk2", PRI_UVACVIDEO, STKSIZE_UVACVIDEO);
			if (0 == UVACVIDEOTSK_ID2) {
				DBG_ERR("VideoTsk create failed\r\n");
		        return E_SYS;
			}
			vos_task_resume(UVACVIDEOTSK_ID2);

			UVAC_TX_VDO2_ID = vos_task_create(UVAC_IsoInVdo2Tsk, 0, "UVAC_IsoInVdo2Tsk", PRI_UVACISOIN, STKSIZE_UVACISOIN_VID);
			if (0 == UVAC_TX_VDO2_ID) {
				DBG_ERR("UVAC_IsoInVdo2Tsk create failed\r\n");
		        return E_SYS;
			}
			vos_task_resume(UVAC_TX_VDO2_ID);
			#endif
			if (uvac_channel == UVAC_CHANNEL_2V2A) {
				#if 1
				pthread_attr_t fd_attr;
				struct sched_param fd_param;

				pthread_attr_init(&fd_attr);
				if (!is_rt_linux) {
					fd_param.sched_priority = 99-PRI_UVACISOIN;
					pthread_attr_setschedpolicy(&fd_attr,SCHED_RR);
					pthread_attr_setschedparam(&fd_attr,&fd_param);
					pthread_attr_setinheritsched(&fd_attr,PTHREAD_EXPLICIT_SCHED);
				}

				pthread_create(&UVAC_TX_AUD2_ID, &fd_attr, UVAC_IsoInAud2Tsk, 0);
				pthread_setname_np(UVAC_TX_AUD2_ID, "UVAC_IsoInAud2T");
				#else
				UVAC_TX_AUD2_ID = vos_task_create(UVAC_IsoInAud2Tsk, 0, "UVAC_IsoInAud2Tsk", PRI_UVACISOIN, STKSIZE_UVACISOIN_AUD);
				if (0 == UVAC_TX_AUD2_ID) {
					DBG_ERR("UVAC_IsoInAud2Tsk create failed\r\n");
			        return E_SYS;
				}
				vos_task_resume(UVAC_TX_AUD2_ID);
				#endif
			}

			if (uvac_channel == UVAC_CHANNEL_3V1A) {
				#if 1
				{
					pthread_attr_t fd_attr;
					struct sched_param fd_param;

					pthread_attr_init(&fd_attr);

					if (!is_rt_linux) {
						fd_param.sched_priority = 99-PRI_UVACVIDEO;
						pthread_attr_setschedpolicy(&fd_attr,SCHED_RR);
						pthread_attr_setschedparam(&fd_attr,&fd_param);
						pthread_attr_setinheritsched(&fd_attr,PTHREAD_EXPLICIT_SCHED);
					}
					pthread_create(&UVACVIDEOTSK_ID3, &fd_attr, UVAC_VideoTsk3, 0);
					pthread_setname_np(UVACVIDEOTSK_ID3, "UVAC_VideoTsk3");

				}

				{
					pthread_attr_t fd_attr;
					struct sched_param fd_param;

					pthread_attr_init(&fd_attr);

					if (!is_rt_linux) {
						fd_param.sched_priority = 99-PRI_UVACISOIN;
						pthread_attr_setschedpolicy(&fd_attr,SCHED_RR);
						pthread_attr_setschedparam(&fd_attr,&fd_param);
						pthread_attr_setinheritsched(&fd_attr,PTHREAD_EXPLICIT_SCHED);
					}
					pthread_create(&UVAC_TX_VDO3_ID, &fd_attr, UVAC_IsoInVdo3Tsk, 0);
					pthread_setname_np(UVAC_TX_VDO3_ID, "UVAC_IsoInVid3T");

				}

				#else
				UVACVIDEOTSK_ID3 = vos_task_create(UVAC_VideoTsk3, 0, "UVAC_VideoTsk3", PRI_UVACVIDEO, STKSIZE_UVACVIDEO);
				if (0 == UVACVIDEOTSK_ID3) {
					DBG_ERR("VideoTsk create failed\r\n");
			        return E_SYS;
				}
				vos_task_resume(UVACVIDEOTSK_ID3);

				UVAC_TX_VDO3_ID = vos_task_create(UVAC_IsoInVdo3Tsk, 0, "UVAC_IsoInVdo3Tsk", PRI_UVACISOIN, STKSIZE_UVACISOIN_VID);
				if (0 == UVAC_TX_VDO3_ID) {
					DBG_ERR("UVAC_IsoInVdo3Tsk create failed\r\n");
			        return E_SYS;
				}
				vos_task_resume(UVAC_TX_VDO3_ID);
				#endif
			}
		}
		#if 0//gUvacMtpEnabled
		if (gUvacMtpEnabled) {
			sta_tsk(UVACSIDCTSK_ID, 0);
			sta_tsk(UVACSIDCINTRTSK_ID, 0);
		}
		#endif

		if (gUvacUacRxEnabled) {
			#if 1
			pthread_attr_t fd_attr;
			struct sched_param fd_param;

			pthread_attr_init(&fd_attr);

			if (!is_rt_linux) {
				fd_param.sched_priority = 99-PRI_UVACISOIN;
				pthread_attr_setschedpolicy(&fd_attr,SCHED_RR);
				pthread_attr_setschedparam(&fd_attr,&fd_param);
				pthread_attr_setinheritsched(&fd_attr,PTHREAD_EXPLICIT_SCHED);
			}

			pthread_create(&UVAC_UAC_RX_ID, &fd_attr, UVAC_AudioRxTsk, 0);
			pthread_setname_np(UVAC_UAC_RX_ID, "UVAC_AudRxTsk");
			#else
			UVAC_UAC_RX_ID = vos_task_create(UVAC_AudioRxTsk, 0, "UVAC_AudRxTsk", PRI_UVACISOOUT, STKSIZE_UVACISOOUT);
			if (0 == UVAC_UAC_RX_ID) {
				DBG_ERR("AudRxTsk create failed\r\n");
		        return E_SYS;
			}
			vos_task_resume(UVAC_UAC_RX_ID);
			#endif
		}

		gUvacOpened = TRUE;

		#if UVCP_FUNC
		uvcp_open();
		#endif

		UVAC_OpenNeededFIFO(0);


		g_usb_speed = usb3dev_getDevInfo(U3DEV_DEVINFO_ID_SPEED);

		DbgMsg_UVCIO(("-%s\r\n", __func__));
		return E_OK;
	} else {
		return E_SYS;
	}
}

/**
    Close USB PCC Task.
*/

void U3UVAC_Close(void)
{
	UINT32 uvac_channel = 0;

	DbgMsg_UVC((":0x%x++\n\r", gUvacOpened));

	if (gUvacOpened) {
		gUvacOpened = FALSE;
	} else {
		DBG_ERR("UVAC not opened\r\n");
		return;                          //task aren't running. so we forget this request...
	}

	if (gUvacCdcEnabled[CDC_COM_1ST]) {
		wai_sem(SEMID_UVC_WRITE_CDC);
	}

	if (gUvacCdcEnabled[CDC_COM_2ND]) {
		wai_sem(SEMID_UVC_WRITE2_CDC);
	}

	if (g_u3_hid_info.en) {
		wai_sem(SEMID_UVC_WRITE_HID);
		wai_sem(SEMID_UVC_READ_HID);
	}

	if (gUvacUacRxEnabled) {
		FLGPTN uiFlag = 0;

		usb3dev_abortEndpoint(UVAC_USB_RX_EP[0]);

		_UVAC_unlock_pullqueue(UVAC_STRM_AUDRX);
		set_flg(FLG_ID_UVAC_UAC_RX, FLGUVAC_AUD_EXIT);
		wai_flg(&uiFlag, FLG_ID_UVAC_UAC_RX, FLGUVAC_AUD_IDLE, (TWF_CLR | TWF_ORW));
	}

	uvac_channel = (gUvacChannel > gHsUvacChannel)? gUvacChannel : gHsUvacChannel;

	_UAC_Stop(UVAC_AUD_DEV_CNT_1);

	if (uvac_channel == UVAC_CHANNEL_2V2A) {
		_UAC_Stop(UVAC_AUD_DEV_CNT_2);
	}

	for (UINT8 i = 0; i < UVAC_VID_DEV_CNT_MAX; i++) {
		gUvcVidStart[i] = FALSE;
	}

	UVAC_StopAll();

	memset((void *)gUvacEUVendCmdCB, 0, (sizeof(UVAC_EUVENDCMDCB) * UVAC_EU_VENDCMD_CNT));
	//gpImgUnitUvacVendDevDesc = 0;
	gUvacWinIntrfEnable = FALSE;
	//gUvcVideoFmtType = UVAC_VIDEO_FORMAT_H264_MJPEG;

	if (gUvacCdcEnabled[CDC_COM_1ST]) {
		set_flg(FLG_ID_UVAC, FLGUVAC_CDC_ABORT_READ);
		sig_sem(SEMID_UVC_WRITE_CDC);
	}
	if (gUvacCdcEnabled[CDC_COM_2ND]) {
		set_flg(FLG_ID_UVAC, FLGUVAC_CDC_ABORT_READ2);
		sig_sem(SEMID_UVC_WRITE2_CDC);
	}
	if (g_u3_hid_info.en) {
		sig_sem(SEMID_UVC_WRITE_HID);
		sig_sem(SEMID_UVC_READ_HID);
	}
	if (guiUvacBufAddr) {
		hwmem_mem_munmap((void *)guiUvacBufAddr, guiUvacBufTotalSize);
	}
	if (gUvcVidBuf[UVAC_VID_DEV_CNT_1].va) {
		hwmem_mem_munmap((void *)gUvcVidBuf[UVAC_VID_DEV_CNT_1].va, gUvcVidBuf[UVAC_VID_DEV_CNT_1].size);
	}
	if (gUvcVidBuf[UVAC_VID_DEV_CNT_2].va) {
		hwmem_mem_munmap((void *)gUvcVidBuf[UVAC_VID_DEV_CNT_2].va, gUvcVidBuf[UVAC_VID_DEV_CNT_2].size);
	}
	if (gUvcVidBuf[UVAC_VID_DEV_CNT_3].va) {
		hwmem_mem_munmap((void *)gUvcVidBuf[UVAC_VID_DEV_CNT_3].va, gUvcVidBuf[UVAC_VID_DEV_CNT_3].size);
	}

	set_flg(FLG_ID_UVAC, FLGUVAC_VIDEO_EXIT | \
						 FLGUVAC_ISOIN_VDO_EXIT | \
						 FLGUVAC_ISOIN_AUD_EXIT | \
						 FLGUVAC_VIDEO2_EXIT | \
						 FLGUVAC_ISOIN_VDO2_EXIT | \
						 FLGUVAC_ISOIN_AUD2_EXIT
						 );

	set_flg(FLG_ID_UVAC_UVC3, FLGUVAC_VIDEO3_EXIT | FLGUVAC_ISOIN_VDO3_EXIT);

	pthread_join(UVACVIDEOTSK_ID, NULL);
	pthread_join(UVAC_TX_VDO1_ID, NULL);
	pthread_join(UVAC_TX_AUD1_ID, NULL);

	if (uvac_channel > UVAC_CHANNEL_1V1A) {

		pthread_join(UVACVIDEOTSK_ID2, NULL);
		pthread_join(UVAC_TX_VDO2_ID, NULL);

		if (uvac_channel == UVAC_CHANNEL_2V2A) {
			pthread_join(UVAC_TX_AUD2_ID, NULL);
		}

		if (uvac_channel == UVAC_CHANNEL_3V1A) {


			pthread_join(UVACVIDEOTSK_ID3, NULL);
			pthread_join(UVAC_TX_VDO3_ID, NULL);
		}
	}

	usb3dev_close();

	#if UVCP_FUNC
	uvcp_close();
	#endif

	U3UVAC_UnInstallID();

	gUvacChannel = UVAC_CHANNEL_1V1A;
	gHsUvacChannel = 0;

	DbgMsg_UVC(("-%s:0x%x,trig=%d,ch=%d,hs ch=%d\n\r", __func__, gUvacOpened, gUvcTrigEnabled, gUvacChannel, gHsUvacChannel));
}
/**
    UVAC Video Task 2.
*/
void* UVAC_VideoTsk2(void* arglist)
{
	UINT32              uiOutWidth = 0, uiOutHeight = 0, uiFps = 0;
	FLGPTN              uiFlag;
	UINT32              totalFrmCnt = 0;
	UINT32              toggleFid = 0;
	UVAC_STRM_INFO      strmInfo = {0};
	//ER                  retV = E_OK;
	UVAC_TXF_INFO       txfInfo;
	UINT32              tmpIdx = 0;
	UINT8               *pVidBuf;
	UINT32              vidSize;
	UINT32              thisVidIdx = UVAC_VID_DEV_CNT_2;
	UINT32              thisTxfQueIdx = UVAC_TXF_QUE_V2;
	PUVAC_VID_STRM_INFO pStrmQueInfo = &gUvacVidStrmInfo[thisVidIdx];
	UINT32 uiQueCnt;
	UVAC_VIDEO_FRM_TYPE frame_type = 0;


	//kent_tsk();
	while (1) {
		DbgMsg_UVC(("UVAC Tsk2 Init..\r\n"));
		set_flg(FLG_ID_UVAC, FLGUVAC_RDY2);
		wai_flg(&uiFlag, FLG_ID_UVAC, FLGUVAC_START2 | FLGUVAC_VIDEO2_EXIT, TWF_ORW);        //Note we don't clear start flag!!!
		clr_flg(FLG_ID_UVAC, FLGUVAC_RDY2);

		if (uiFlag & FLGUVAC_VIDEO2_EXIT) {
			set_flg(FLG_ID_UVAC, FLGUVAC_RDY2);
			break;
		}

		UVAC_GetImageSize(gUvacCodecType[thisVidIdx], gUvcParamAry[thisVidIdx].OutSizeID, thisVidIdx, &uiOutWidth, &uiOutHeight);
		uiFps = gUvcParamAry[thisVidIdx].FrameRate;

		DbgMsg_UVC(("^GVidTsk2:W=%d,H=%d,Fr=%d,Fmt=%d,%d, HWP=%d\r\n", uiOutWidth, uiOutHeight, uiFps, gUvacCodecType[thisVidIdx], gUvcNoVidStrm, gUvcHWPayload[thisVidIdx]));
		UVAC_DbgDmp_TxfInfoByQue(thisTxfQueIdx);
		UVAC_DbgDmp_MemLayout();

		UVC_TimeTrigSet(1000000 / uiFps, thisVidIdx);

		//CB to change video resolution, codec, fps
		if (g_fpStartVideo) {
			memset((void *)&gStrmInfo[thisVidIdx], 0, sizeof(UVAC_STRM_INFO));
			gStrmInfo[thisVidIdx].strmPath = UVAC_STRM_VID2;

			pStrmQueInfo->strmInfo.strmCodec = gStrmInfo[thisVidIdx].strmCodec = gUvacCodecType[thisVidIdx];
			pStrmQueInfo->strmInfo.strmWidth = gStrmInfo[thisVidIdx].strmWidth = uiOutWidth;
			pStrmQueInfo->strmInfo.strmHeight = gStrmInfo[thisVidIdx].strmHeight = uiOutHeight;
			pStrmQueInfo->strmInfo.strmFps = gStrmInfo[thisVidIdx].strmFps = uiFps;
			pStrmQueInfo->strmInfo.strmResoIdx = gStrmInfo[thisVidIdx].strmResoIdx = gUvcParamAry[thisVidIdx].OutSizeID - 1;
			pStrmQueInfo->strmInfo.isStrmOn = gStrmInfo[thisVidIdx].isStrmOn = TRUE;
			if (UVAC_VIDEO_FORMAT_H264 == gUvacCodecType[thisVidIdx]) {
				pStrmQueInfo->strmInfo.strmTBR = gStrmInfo[thisVidIdx].strmTBR = gUvcH264TBR;
			} else {
				pStrmQueInfo->strmInfo.strmTBR = gStrmInfo[thisVidIdx].strmTBR = gUvcMJPGTBR;
			}

			UVAC_RemoveTxfInfo(thisTxfQueIdx);
			if (E_OK != g_fpStartVideo(thisVidIdx, &gStrmInfo[thisVidIdx])) {
				//Get the data of H264 Header,and check resolution is workale or not
				DBG_ERR("StartVid Fail, Reso=%d/%d, Codec=%d\r\n", uiOutWidth, uiOutHeight, gUvacCodecType[thisVidIdx]);
				clr_flg(FLG_ID_UVAC, FLGUVAC_START2);
				UVC_TimeTrigClose(thisVidIdx);
				continue;
			}
		} else {
			UVAC_RemoveTxfInfo(thisTxfQueIdx);
			DBG_ERR("CB NULL. Can NOT change video resolution,codec,fps\r\n");
		}

		if (gU3UvacRunningFlag[thisVidIdx] & FLGUVAC_STOP2) {
			DbgMsg_UVC(("Tsk2 Stop C...\r\n"));
			goto L_UVAC_END2;
		}

		//UVAC_RemoveTxfInfo(thisTxfQueIdx);
		totalFrmCnt = toggleFid = 0;
		gToggleFid[thisVidIdx] = 0;
		uiQueCnt = 0;
		while (FALSE == gUvcNoVidStrm) {
			if (uvc_direct_trigger) {
				wai_flg(&uiFlag, FLG_ID_UVAC, FLGUVAC_VIDEO2_TXF|FLGUVAC_VIDEO2_STOP, (TWF_CLR | TWF_ORW));
				if (uiFlag & FLGUVAC_VIDEO2_STOP) {
					DbgMsg_UVC(("^GVdoTsk2 Stop\r\n"));
					break;
				}
			} else {
				#if 0
				timer_waitTimeup(gUvacTimerID[thisVidIdx]);
				#endif
			}
			DbgMsg_UVCIO(("22Uvac-timer:%d\r\n", totalFrmCnt));
V2_SKIP_TIMER:
			//Send the first frame and it must be I frame
			tmpIdx = pStrmQueInfo->idxConsumer;
			DbgMsg_UVCIO(("22StrmQue[%d]:cons=%d,prod=%d,codec=%d\r\n", thisVidIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer, gUvacCodecType[thisVidIdx]));
			if (tmpIdx == pStrmQueInfo->idxProducer) {
				if (m_uiUvacDbgVid & UVAC_DBG_VID_START) {
					DBG_WRN("22Empty StrmQue[%d]:con=%d,prod=%d, start=%d, NoStrm=%d\r\n", thisVidIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer, gUvcVidStart[thisTxfQueIdx], gUvcNoVidStrm);
				}
				if (TRUE == gUvcVidStart[thisTxfQueIdx]) {
					continue;
				} else {
					break;
				}
			} else {
				DbgCode_UVC(if (0 == totalFrmCnt) {
				DbgMsg_UVC(("22FirstFrm[%d]:cons=%d,prod=%d\r\n", thisVidIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer));
				})
			}
			if (0 == totalFrmCnt && (m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
				DBG_DUMP("idxProducer=%d, idxConsumer=%d\r\n", pStrmQueInfo->idxProducer, pStrmQueInfo->idxConsumer);
			}
			pVidBuf = (UINT8 *)(pStrmQueInfo->addr[tmpIdx]);
			vidSize = pStrmQueInfo->size[tmpIdx];
			frame_type = pStrmQueInfo->frame_type[tmpIdx];
			if (0 == totalFrmCnt) {
				if (UVAC_VIDEO_FORMAT_MJPG == gUvacCodecType[thisVidIdx] || UVAC_VIDEO_FORMAT_YUV== gUvacCodecType[thisVidIdx] || UVAC_VIDEO_FORMAT_NV12 == gUvacCodecType[thisVidIdx]) {
					//point to the latest frame index
					pStrmQueInfo->idxConsumer = (pStrmQueInfo->idxProducer + UVAC_VID_INFO_QUE_MAX_CNT - 1) % UVAC_VID_INFO_QUE_MAX_CNT;
					tmpIdx = pStrmQueInfo->idxConsumer;
				} else if (UVAC_VIDEO_FORMAT_H265 == gUvacCodecType[thisVidIdx]) { //UVAC_VIDEO_FORMAT_H265
					if ((0 == vidSize) || (FALSE == _is_iframe_h265(pVidBuf))) {
						DbgMsg_UVCIO(("Chk I-Frm [%d]addr=0x%lx,0x%x,cnt=0x%x\r\n", tmpIdx, (uintptr_t)pVidBuf, vidSize, totalFrmCnt));
						if ((m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
							DBG_WRN("Wait 1st I-frm:%d, vidStrm exist=%d\r\n", tmpIdx, gUvcNoVidStrm);
						}
						pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V2);
						continue;
					}
				} else { //UVAC_VIDEO_FORMAT_H264
					if ((0 == vidSize) || (FALSE == _is_iframe(pVidBuf))) {
						DbgMsg_UVCIO(("Chk I-Frm [%d]addr=0x%lx,0x%x,cnt=0x%x\r\n", tmpIdx, (uintptr_t)pVidBuf, vidSize, totalFrmCnt));
						if ((m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
							DBG_WRN("Wait 1st I-frm:%d, vidStrm exist=%d\r\n", tmpIdx, gUvcNoVidStrm);
						}
						pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V2);
						continue;
					}
				}
			}
			pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

			txfInfo.sAddr = txfInfo.oriAddr = (uintptr_t)pVidBuf;
			txfInfo.size = vidSize;
			txfInfo.usbEP = UVAC_USB_EP[thisTxfQueIdx];
			txfInfo.txfCnt = 0;
			txfInfo.streamType = UVAC_TXF_STREAM_VID;
			txfInfo.timestamp = pStrmQueInfo->timestamp;
			txfInfo.uvc_timestamp_start = pStrmQueInfo->uvc_timestamp_start;
			txfInfo.frame_type = frame_type;

			UVAC_AddIsoInTxfInfo(&txfInfo, thisTxfQueIdx);
			//UVAC_IsoInTxfStateMachine(UVC_ISOIN_TXF_ACT_TXF);
			totalFrmCnt++;
			if ((m_uiUvacDbgVid & UVAC_DBG_VID_QUEUE)) {
				//DBG_DUMP("[1]0x%X:%dKB %dms\r\n", txfInfo.sAddr, txfInfo.size, Perf_GetDuration() / 1000);
			}
			if (gU3UvacRunningFlag[thisVidIdx] & FLGUVAC_STOP2) {
				DbgMsg_UVC(("Tsk2 Stop B ...\r\n"));
				break;
			}

			#if 0
			if (usb_getControllerState() == USB_CONTROLLER_STATE_SUSPEND) {
				if ((usb_getControllerState() == USB_CONTROLLER_STATE_SUSPEND)) {
					DbgMsg_UVC(("Clr StartFlag2!!\r\n"));
					gUvcUsbDMAAbord = TRUE;
					clr_flg(FLG_ID_UVAC, FLGUVAC_START2);
				}
				DbgMsg_UVC(("Tsk2 Stop E ...\r\n"));
				break;
			}
			#endif
			uiQueCnt = pStrmQueInfo->idxProducer + UVAC_VID_INFO_QUE_MAX_CNT - pStrmQueInfo->idxConsumer;
			if (uiQueCnt >= UVAC_VID_INFO_QUE_MAX_CNT) {
				uiQueCnt -= UVAC_VID_INFO_QUE_MAX_CNT;
			}
			if (m_uiUvacDbgVid & UVAC_DBG_VID_QUE_CNT) {
				DBG_DUMP("V2 Q=%d\r\n", uiQueCnt);
			}
			#if 0
			if (uiQueCnt > 1 && ((UVAC_VIDEO_FORMAT_MJPG == gUvacCodecType[thisVidIdx]) || (UVAC_VID_I_FRM_MARK == pVidBuf[UVAC_VID_I_FRM_MARK_POS]))) {
				if (m_uiUvacDbgVid & UVAC_DBG_VID_START) {
					DBG_DUMP("V2[%d] race!\r\n", totalFrmCnt);
				}
				goto V2_SKIP_TIMER;
			}
			#else
			if (uiQueCnt) {
				goto V2_SKIP_TIMER;
			}
			#endif

		}// end of wait timer2 //while (FALSE == gUvcNoVidStrm)

L_UVAC_END2:
		clr_flg(FLG_ID_UVAC, FLGUVAC_START2);
		DbgMsg_UVC(("+2UVAC end:Fmt[0]=%d\n\r", gUvacCodecType[thisVidIdx]));
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
		usb_abortEndpoint(UVAC_USB_EP[thisTxfQueIdx]);
#else
		usb3dev_abortEndpoint(UVAC_USB_EP[thisTxfQueIdx]);
#endif
		//DbgMsg_UVC(("-2Abort EP#%d, retV=%d\n\r", UVAC_USB_EP[thisTxfQueIdx], retV));
		UVC_TimeTrigClose(thisVidIdx);
		DbgMsg_UVC(("+2wait isoTsk :0x%x\n\r", uiFlag));
		//wai_flg(&uiFlag, FLG_ID_UVAC, FLGUVAC_ISOIN_READY, (TWF_CLR | TWF_ORW));

		if (0 == kchk_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_VDO2_READY)) {
			vos_util_delay_ms(70);
		}
		{
			UINT32 count = 0;
			while (1) {
				if (count < 1000) {
					if (0 == kchk_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_VDO2_READY)) {
						usb3dev_abortEndpoint(UVAC_USB_EP[thisTxfQueIdx]);
						DbgMsg_UVC(("\n\r#### abort EP[%d] ####\n\r\n\r", UVAC_USB_EP[thisTxfQueIdx]));
						vos_util_delay_ms(10);
					} else {
						break;
					}
				} else {
					DBG_ERR("wait ISOIN_VDO2_READY failed\r\n");
					break;
				}
				count++;
			}
		}

		DbgMsg_UVC(("-2wait isoTsk :0x%x\n\r", uiFlag));
		gU3UvacRunningFlag[thisVidIdx] = 0;
		clr_flg(FLG_ID_UVAC, FLGUVAC_STOP2);
		//reset stream queue index
		pStrmQueInfo->idxProducer = 0;
		pStrmQueInfo->idxConsumer = 0;
		if (g_fpStopVideo) {
			g_fpStopVideo(thisVidIdx);
			pStrmQueInfo->strmInfo.isStrmOn = strmInfo.isStrmOn = FALSE;
		}
		DbgMsg_UVC(("-UVAC2 end\n\r"));
	} // end for while(1)

	return 0;
}

/**
    UVAC Video Task 3.
*/
void* UVAC_VideoTsk3(void* arglist)
{
	UINT32              uiOutWidth = 0, uiOutHeight = 0, uiFps = 0;
	FLGPTN              uiFlag;
	UINT32              totalFrmCnt = 0;
	UINT32              toggleFid = 0;
	UVAC_STRM_INFO      strmInfo = {0};
	//ER                  retV = E_OK;
	UVAC_TXF_INFO       txfInfo;
	UINT32              tmpIdx = 0;
	UINT8               *pVidBuf;
	UINT32              vidSize;
	UINT32              thisVidIdx = UVAC_VID_DEV_CNT_3;
	UINT32              thisTxfQueIdx = UVAC_TXF_QUE_V3;
	PUVAC_VID_STRM_INFO pStrmQueInfo = &gUvacVidStrmInfo[thisVidIdx];
	UINT32 uiQueCnt;
	UVAC_VIDEO_FRM_TYPE frame_type = 0;


	//kent_tsk();
	while (1) {
		DbgMsg_UVC(("UVAC Tsk2 Init..\r\n"));
		set_flg(FLG_ID_UVAC_UVC3, FLGUVAC_RDY3);
		wai_flg(&uiFlag, FLG_ID_UVAC_UVC3, FLGUVAC_START3 | FLGUVAC_VIDEO3_EXIT, TWF_ORW);        //Note we don't clear start flag!!!
		clr_flg(FLG_ID_UVAC_UVC3, FLGUVAC_RDY3);

		if (uiFlag & FLGUVAC_VIDEO3_EXIT) {
			set_flg(FLG_ID_UVAC_UVC3, FLGUVAC_RDY3);
			break;
		}

		UVAC_GetImageSize(gUvacCodecType[thisVidIdx], gUvcParamAry[thisVidIdx].OutSizeID, thisVidIdx, &uiOutWidth, &uiOutHeight);
		uiFps = gUvcParamAry[thisVidIdx].FrameRate;

		DbgMsg_UVC(("^GVidTsk3:W=%d,H=%d,Fr=%d,Fmt=%d,%d, HWP=%d\r\n", uiOutWidth, uiOutHeight, uiFps, gUvacCodecType[thisVidIdx], gUvcNoVidStrm, gUvcHWPayload[thisVidIdx]));
		UVAC_DbgDmp_TxfInfoByQue(thisTxfQueIdx);
		UVAC_DbgDmp_MemLayout();

		UVC_TimeTrigSet(1000000 / uiFps, thisVidIdx);

		//CB to change video resolution, codec, fps
		if (g_fpStartVideo) {
			memset((void *)&gStrmInfo[thisVidIdx], 0, sizeof(UVAC_STRM_INFO));
			gStrmInfo[thisVidIdx].strmPath = UVAC_STRM_VID3;

			pStrmQueInfo->strmInfo.strmCodec = gStrmInfo[thisVidIdx].strmCodec = gUvacCodecType[thisVidIdx];
			pStrmQueInfo->strmInfo.strmWidth = gStrmInfo[thisVidIdx].strmWidth = uiOutWidth;
			pStrmQueInfo->strmInfo.strmHeight = gStrmInfo[thisVidIdx].strmHeight = uiOutHeight;
			pStrmQueInfo->strmInfo.strmFps = gStrmInfo[thisVidIdx].strmFps = uiFps;
			pStrmQueInfo->strmInfo.strmResoIdx = gStrmInfo[thisVidIdx].strmResoIdx = gUvcParamAry[thisVidIdx].OutSizeID - 1;
			pStrmQueInfo->strmInfo.isStrmOn = gStrmInfo[thisVidIdx].isStrmOn = TRUE;
			if (UVAC_VIDEO_FORMAT_H264 == gUvacCodecType[thisVidIdx]) {
				pStrmQueInfo->strmInfo.strmTBR = gStrmInfo[thisVidIdx].strmTBR = gUvcH264TBR;
			} else {
				pStrmQueInfo->strmInfo.strmTBR = gStrmInfo[thisVidIdx].strmTBR = gUvcMJPGTBR;
			}

			UVAC_RemoveTxfInfo(thisTxfQueIdx);
			if (E_OK != g_fpStartVideo(thisVidIdx, &gStrmInfo[thisVidIdx])) {
				//Get the data of H264 Header,and check resolution is workale or not
				DBG_ERR("StartVid Fail, Reso=%d/%d, Codec=%d\r\n", uiOutWidth, uiOutHeight, gUvacCodecType[thisVidIdx]);
				clr_flg(FLG_ID_UVAC_UVC3, FLGUVAC_START3);
				UVC_TimeTrigClose(thisVidIdx);
				continue;
			}
		} else {
			UVAC_RemoveTxfInfo(thisTxfQueIdx);
			DBG_ERR("CB NULL. Can NOT change video resolution,codec,fps\r\n");
		}

		if (gU3UvacRunningFlag[thisVidIdx] & FLGUVAC_STOP3) {
			DbgMsg_UVC(("Tsk3 Stop C...\r\n"));
			goto L_UVAC_END3;
		}

		//UVAC_RemoveTxfInfo(thisTxfQueIdx);
		totalFrmCnt = toggleFid = 0;
		gToggleFid[thisVidIdx] = 0;
		uiQueCnt = 0;
		while (FALSE == gUvcNoVidStrm) {
			if (uvc_direct_trigger) {
				wai_flg(&uiFlag, FLG_ID_UVAC_UVC3, FLGUVAC_VIDEO3_TXF|FLGUVAC_VIDEO3_STOP, (TWF_CLR | TWF_ORW));
				if (uiFlag & FLGUVAC_VIDEO3_STOP) {
					DbgMsg_UVC(("^GVdoTsk3 Stop\r\n"));
					break;
				}
			} else {
				#if 0
				timer_waitTimeup(gUvacTimerID[thisVidIdx]);
				#endif
			}
			DbgMsg_UVCIO(("22Uvac-timer:%d\r\n", totalFrmCnt));
V3_SKIP_TIMER:
			//Send the first frame and it must be I frame
			tmpIdx = pStrmQueInfo->idxConsumer;
			DbgMsg_UVCIO(("22StrmQue[%d]:cons=%d,prod=%d,codec=%d\r\n", thisVidIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer, gUvacCodecType[thisVidIdx]));
			if (tmpIdx == pStrmQueInfo->idxProducer) {
				if (m_uiUvacDbgVid & UVAC_DBG_VID_START) {
					DBG_WRN("22Empty StrmQue[%d]:con=%d,prod=%d, start=%d, NoStrm=%d\r\n", thisVidIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer, gUvcVidStart[thisTxfQueIdx], gUvcNoVidStrm);
				}
				if (TRUE == gUvcVidStart[thisTxfQueIdx]) {
					continue;
				} else {
					break;
				}
			} else {
				DbgCode_UVC(if (0 == totalFrmCnt) {
				DbgMsg_UVC(("22FirstFrm[%d]:cons=%d,prod=%d\r\n", thisVidIdx, pStrmQueInfo->idxConsumer, pStrmQueInfo->idxProducer));
				})
			}
			if (0 == totalFrmCnt && (m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
				DBG_DUMP("idxProducer=%d, idxConsumer=%d\r\n", pStrmQueInfo->idxProducer, pStrmQueInfo->idxConsumer);
			}
			pVidBuf = (UINT8 *)(pStrmQueInfo->addr[tmpIdx]);
			vidSize = pStrmQueInfo->size[tmpIdx];
			frame_type = pStrmQueInfo->frame_type[tmpIdx];
			if (0 == totalFrmCnt) {
				if (UVAC_VIDEO_FORMAT_MJPG == gUvacCodecType[thisVidIdx] || UVAC_VIDEO_FORMAT_YUV== gUvacCodecType[thisVidIdx] || UVAC_VIDEO_FORMAT_NV12 == gUvacCodecType[thisVidIdx]) {
					//point to the latest frame index
					pStrmQueInfo->idxConsumer = (pStrmQueInfo->idxProducer + UVAC_VID_INFO_QUE_MAX_CNT - 1) % UVAC_VID_INFO_QUE_MAX_CNT;
					tmpIdx = pStrmQueInfo->idxConsumer;
				} else if (UVAC_VIDEO_FORMAT_H265 == gUvacCodecType[thisVidIdx]) { //UVAC_VIDEO_FORMAT_H265
					if ((0 == vidSize) || (FALSE == _is_iframe_h265(pVidBuf))) {
						DbgMsg_UVCIO(("Chk I-Frm [%d]addr=0x%lx,0x%x,cnt=0x%x\r\n", tmpIdx, (uintptr_t)pVidBuf, vidSize, totalFrmCnt));
						if ((m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
							DBG_WRN("Wait 1st I-frm:%d, vidStrm exist=%d\r\n", tmpIdx, gUvcNoVidStrm);
						}
						pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V3);
						continue;
					}
				} else { //UVAC_VIDEO_FORMAT_H264
					if ((0 == vidSize) || (FALSE == _is_iframe(pVidBuf))) {
						DbgMsg_UVCIO(("Chk I-Frm [%d]addr=0x%lx,0x%x,cnt=0x%x\r\n", tmpIdx, (uintptr_t)pVidBuf, vidSize, totalFrmCnt));
						if ((m_uiUvacDbgVid & UVAC_DBG_VID_START)) {
							DBG_WRN("Wait 1st I-frm:%d, vidStrm exist=%d\r\n", tmpIdx, gUvcNoVidStrm);
						}
						pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V3);
						continue;
					}
				}
			}
			pStrmQueInfo->idxConsumer = (tmpIdx + 1) & UVAC_VID_INFO_QUE_MAX_IDX;

			txfInfo.sAddr = txfInfo.oriAddr = (uintptr_t)pVidBuf;
			txfInfo.size = vidSize;
			txfInfo.usbEP = UVAC_USB_EP[thisTxfQueIdx];
			txfInfo.txfCnt = 0;
			txfInfo.streamType = UVAC_TXF_STREAM_VID;
			txfInfo.timestamp = pStrmQueInfo->timestamp;
			txfInfo.uvc_timestamp_start = pStrmQueInfo->uvc_timestamp_start;
			txfInfo.frame_type = frame_type;

			UVAC_AddIsoInTxfInfo(&txfInfo, thisTxfQueIdx);
			//UVAC_IsoInTxfStateMachine(UVC_ISOIN_TXF_ACT_TXF);
			totalFrmCnt++;
			if ((m_uiUvacDbgVid & UVAC_DBG_VID_QUEUE)) {
				//DBG_DUMP("[1]0x%X:%dKB %dms\r\n", txfInfo.sAddr, txfInfo.size, Perf_GetDuration() / 1000);
			}
			if (gU3UvacRunningFlag[thisVidIdx] & FLGUVAC_STOP3) {
				DbgMsg_UVC(("Tsk3 Stop B ...\r\n"));
				break;
			}

			#if 0
			if (usb_getControllerState() == USB_CONTROLLER_STATE_SUSPEND) {
				if ((usb_getControllerState() == USB_CONTROLLER_STATE_SUSPEND)) {
					DbgMsg_UVC(("Clr StartFlag2!!\r\n"));
					gUvcUsbDMAAbord = TRUE;
					clr_flg(FLG_ID_UVAC, FLGUVAC_START2);
				}
				DbgMsg_UVC(("Tsk2 Stop E ...\r\n"));
				break;
			}
			#endif
			uiQueCnt = pStrmQueInfo->idxProducer + UVAC_VID_INFO_QUE_MAX_CNT - pStrmQueInfo->idxConsumer;
			if (uiQueCnt >= UVAC_VID_INFO_QUE_MAX_CNT) {
				uiQueCnt -= UVAC_VID_INFO_QUE_MAX_CNT;
			}
			if (m_uiUvacDbgVid & UVAC_DBG_VID_QUE_CNT) {
				DBG_DUMP("V3 Q=%d\r\n", uiQueCnt);
			}
			#if 0
			if (uiQueCnt > 1 && ((UVAC_VIDEO_FORMAT_MJPG == gUvacCodecType[thisVidIdx]) || (UVAC_VID_I_FRM_MARK == pVidBuf[UVAC_VID_I_FRM_MARK_POS]))) {
				if (m_uiUvacDbgVid & UVAC_DBG_VID_START) {
					DBG_DUMP("V2[%d] race!\r\n", totalFrmCnt);
				}
				goto V2_SKIP_TIMER;
			}
			#else
			if (uiQueCnt) {
				goto V3_SKIP_TIMER;
			}
			#endif

		}// end of wait timer2 //while (FALSE == gUvcNoVidStrm)

L_UVAC_END3:
		clr_flg(FLG_ID_UVAC_UVC3, FLGUVAC_START3);
		DbgMsg_UVC(("+2UVAC end:Fmt[0]=%d\n\r", gUvacCodecType[thisVidIdx]));
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
		usb_abortEndpoint(UVAC_USB_EP[thisTxfQueIdx]);
#else
		usb3dev_abortEndpoint(UVAC_USB_EP[thisTxfQueIdx]);
#endif
		//DbgMsg_UVC(("-2Abort EP#%d, retV=%d\n\r", UVAC_USB_EP[thisTxfQueIdx], retV));
		UVC_TimeTrigClose(thisVidIdx);
		DbgMsg_UVC(("+2wait isoTsk :0x%x\n\r", uiFlag));
		//wai_flg(&uiFlag, FLG_ID_UVAC, FLGUVAC_ISOIN_READY, (TWF_CLR | TWF_ORW));

		if (0 == kchk_flg(FLG_ID_UVAC_UVC3, FLGUVAC_ISOIN_VDO3_READY)) {
			vos_util_delay_ms(70);
		}
		{
			UINT32 count = 0;
			while (1) {
				if (count < 1000) {
					if (0 == kchk_flg(FLG_ID_UVAC_UVC3, FLGUVAC_ISOIN_VDO3_READY)) {
						usb3dev_abortEndpoint(UVAC_USB_EP[thisTxfQueIdx]);
						DbgMsg_UVC(("\n\r#### abort EP[%d] ####\n\r\n\r", UVAC_USB_EP[thisTxfQueIdx]));
						vos_util_delay_ms(10);
					} else {
						break;
					}
				} else {
					DBG_ERR("wait ISOIN_VDO3_READY failed\r\n");
					break;
				}
				count++;
			}
		}

		DbgMsg_UVC(("-2wait isoTsk :0x%x\n\r", uiFlag));
		gU3UvacRunningFlag[thisVidIdx] = 0;
		clr_flg(FLG_ID_UVAC_UVC3, FLGUVAC_STOP3);
		//reset stream queue index
		pStrmQueInfo->idxProducer = 0;
		pStrmQueInfo->idxConsumer = 0;
		if (g_fpStopVideo) {
			g_fpStopVideo(thisVidIdx);
			pStrmQueInfo->strmInfo.isStrmOn = strmInfo.isStrmOn = FALSE;
		}
		DbgMsg_UVC(("-UVAC3 end\n\r"));
	} // end for while(1)

	return 0;
}


ER U3UVAC_WaitStrmDone(UVAC_STRM_PATH path)
{
	FLGPTN uiFlag = 0;
	FLGPTN wait_flg;
	UINT32 timeout;
	int ret = 0;

	if (FLG_ID_UVAC_FRM == 0) {
		return E_OK;
	}

	switch (path) {
	case UVAC_STRM_VID:
		wait_flg = FLGUVAC_FRM_V1;
		if (gStrmInfo[UVAC_VID_DEV_CNT_1].strmFps != 0) {
			timeout = (10000/gStrmInfo[UVAC_VID_DEV_CNT_1].strmFps);
		} else {
			timeout = 500;
		}
		break;
	case UVAC_STRM_VID2:
		wait_flg = FLGUVAC_FRM_V2;
		if (gStrmInfo[UVAC_VID_DEV_CNT_2].strmFps != 0) {
			timeout = (10000/gStrmInfo[UVAC_VID_DEV_CNT_2].strmFps);
		} else {
			timeout = 500;
		}
		break;
	case UVAC_STRM_VID3:
		wait_flg = FLGUVAC_FRM_V3;
		if (gStrmInfo[UVAC_VID_DEV_CNT_3].strmFps != 0) {
			timeout = (10000/gStrmInfo[UVAC_VID_DEV_CNT_3].strmFps);
		} else {
			timeout = 500;
		}
		break;
	case UVAC_STRM_AUD:
		wait_flg = FLGUVAC_FRM_A1;
		wait_flg = FLGUVAC_FRM_A2;
		if (gUacSampleRate != 0) {
			timeout = (10240000/gUacSampleRate);
		} else {
			timeout = 500;
		}
		break;
	case UVAC_STRM_AUD2:
		wait_flg = FLGUVAC_FRM_A2;
		if (gUacSampleRate != 0) {
			timeout = (10240000/gUacSampleRate);
		} else {
			timeout = 500;
		}
		break;
	default:
		DBG_ERR("invalid path = %d\r\n", path);
		return E_SYS;
		break;
	}

	ret = vos_flag_wait_timeout(&uiFlag, FLG_ID_UVAC_FRM, wait_flg, TWF_ORW, vos_util_msec_to_tick(timeout));

	if (ret != 0) {
		//DBG_WRN("wait path %d flag err = %d\r\n", path, ret);
		return E_TMOUT;
	}

	return E_OK;
}

static BOOL IsCdcNotOpened(CDC_COM_ID ComID)
{
	if (FALSE == gUvacOpened || FALSE == gUvacCdcEnabled[ComID]) {
		DBG_ERR("CDC not opened!\r\n");
		return TRUE;
	} else {
		return FALSE;
	}
}

UINT32 UVAC_GetCdcDataCount(CDC_COM_ID ComID)
{
	if (IsCdcNotOpened(ComID)) {
		return 0;
	}

	if (CDC_COM_1ST == ComID) {
		return usb3dev_getEPBytecount(CDC_DATA_OUT_EP);
	} else {
		return usb3dev_getEPBytecount(CDC_DATA2_OUT_EP);
	}

	return 0;
}

ER U3UVAC_ReadCdcData(CDC_COM_ID ComID, void *p_buf, UINT32 buffer_size, INT32 timeout)
{
	#if 1
	UINT32  dataSize;
	FLGPTN      uiFlag = 0;
	UINT32 SemID;
	FLGPTN waiptn;
	USB_EP DataOutEP;
	INT32 ret_value = -1;

	if (IsCdcNotOpened(ComID)) {
		return ret_value;
	}

	if (CDC_COM_1ST == ComID) {
		SemID = SEMID_UVC_READ_CDC;
		waiptn = FLGUVAC_CDC_DATA_OUT | FLGUVAC_CDC_ABORT_READ;
		DataOutEP = CDC_DATA_OUT_EP;
	} else {
		SemID = SEMID_UVC_READ2_CDC;
		waiptn = FLGUVAC_CDC_DATA2_OUT | FLGUVAC_CDC_ABORT_READ2;
		DataOutEP = CDC_DATA2_OUT_EP;
	}

	wai_sem(SemID);

	vos_flag_wait_timeout(&uiFlag, FLG_ID_UVAC, waiptn, TWF_ORW | TWF_CLR, vos_util_msec_to_tick(timeout));
	if (uiFlag & (FLGUVAC_CDC_ABORT_READ | FLGUVAC_CDC_ABORT_READ2)) {
		sig_sem(SemID);
		return ret_value;
	}

	if (uiFlag & waiptn) {
		//usb_maskEPINT(CDC_DATA_OUT_EP); already mask in UVAC_Callback()
		dataSize = UVAC_GetCdcDataCount(ComID);
		if (dataSize > buffer_size) {
			dataSize = buffer_size;
		}
		usb3dev_readEndpoint(DataOutEP, (UINT8 *) p_buf, &dataSize);
		ret_value = (INT32)dataSize;
		usb3dev_setInterruptEnable(DataOutEP, ENABLE);
	}

	DBG_MSG("CDC read %d byte.\r\n", dataSize);
	sig_sem(SemID);
	return ret_value;
	#else
	return 0;
	#endif
}

void U3UVAC_AbortCdcRead(CDC_COM_ID ComID)
{
	if (FALSE == IsCdcNotOpened(ComID)) {
		if (CDC_COM_1ST == ComID) {
			set_flg(FLG_ID_UVAC, FLGUVAC_CDC_ABORT_READ);
		} else {
			set_flg(FLG_ID_UVAC, FLGUVAC_CDC_ABORT_READ2);
		}
	}
}
#define MAX_CDC_RETRY_CNT 1000
ER U3UVAC_WriteCdcData(CDC_COM_ID ComID, void *p_buf, UINT32 buffer_size, INT32 timeout)
{
	#if 1
	UINT32 SemID;
	USB_EP DataInEP;
	UINT32 BufSize;
	VOS_TICK begin = 0, current = 0;
	UINT32 elapsed = 0, remain;
	INT32 ret_value = -1;

	if (IsCdcNotOpened(ComID)) {
		return ret_value;
	}

	if (buffer_size > USB_MAX_DMA_LENGTH) {
		DBG_ERR("Max data length is 0x%X\r\n", USB_MAX_DMA_LENGTH);
		return ret_value;
	}

	if (CDC_COM_1ST == ComID) {
		SemID = SEMID_UVC_WRITE_CDC;
		DataInEP = CDC_DATA_IN_EP;
	} else {
		SemID = SEMID_UVC_WRITE2_CDC;
		DataInEP = CDC_DATA2_IN_EP;
	}

	//In 2V2A mode, CDC used the preivous FIFOs which share only one DMA channel.
	if (gUvacChannel > UVAC_CHANNEL_1V1A) {
		SemID = SEMID_UVC_WRITE_CDC;
	}

	wai_sem(SemID);

	vos_perf_mark(&begin);
	while(usb3dev_chkEPBusy(DataInEP, U3DEV_EPDIR_IN)) {
		vos_perf_mark(&current);
		elapsed = vos_perf_duration(begin, current)/ 1000;

		if (timeout == 0 || (elapsed > (UINT32)timeout)) {
			sig_sem(SemID);
			return ret_value;
		}
		vos_util_delay_ms(10);
	}


	if ((UINT32)timeout > elapsed) {
		remain = (UINT32)timeout - elapsed;
	} else {
		remain = 0;
	}

	if (buffer_size) {
		BufSize = buffer_size;
		usb3dev_writeEndpoint_timeout(DataInEP, p_buf, &BufSize, remain);
		ret_value = (INT32)BufSize;
	} else {
		usb3dev_set_tx0byte(DataInEP);
		ret_value = 0;
	}
	sig_sem(SemID);

	return ret_value;
	#else
	return 0;
	#endif
}

void _UVAC_init_queue(void)
{
	PUVAC_AUD_RAWQ pObj;

	pObj = &uac_raw_que;

	//memset((void*)pObj, 0, sizeof(UVAC_AUD_RAWQ));

	pObj->last_addr = gAudRxBuf[0].pa;
	pObj->lock_addr= gAudRxBuf[0].pa;
	pObj->Front = 0;
	pObj->Rear = 0;
}

BOOL _UVAC_GetRaw(uintptr_t *addr, UINT32 size)
{
	uintptr_t front, rear, next, next_end;
	PUVAC_AUD_RAWQ pObj;
	BOOL bResult = FALSE;

	pObj = &uac_raw_que;
	front = pObj->last_addr;
	rear = pObj->lock_addr;

	if (front != rear) {
		if ((front + size) > gAudRxBuf[0].pa + gAudRxBuf[0].size) {
			next = gAudRxBuf[0].pa;

		} else {
			next = front;
		}

		next_end = next + size;

		if (rear > next_end) {
			*addr = next;
			pObj->last_addr = next_end;
			bResult = TRUE;
		} else if (rear < next_end && (next_end - rear) > size) {
			*addr = next;
			pObj->last_addr = next_end;
			bResult = TRUE;
		} else {
			bResult = FALSE;
		}
	} else {
		//empty
		if ((front + size) > gAudRxBuf[0].pa + gAudRxBuf[0].size) {
			next = gAudRxBuf[0].pa;
		} else {
			next = front;
		}

		next_end = next + size;

		*addr = next;
		pObj->last_addr = next_end;
		bResult = TRUE; //no lock bs
	}

	if (pObj->last_addr == (gAudRxBuf[0].pa + gAudRxBuf[0].size)) {
		pObj->last_addr = gAudRxBuf[0].pa;
	}

	return bResult;
}

void _UVAC_unlock_pullqueue(UVAC_STRM_PATH path)
{
	if (vos_sem_wait_timeout(SEMID_UVC_UAC_QUEUE, vos_util_msec_to_tick(0))) {
		DBG_IND("[ISF_AUDCAP][%d] no data in pull queue, auto unlock pull blocking mode !!\r\n", pathID);
	}

	// [case_1]   no data in pullQ, then this 0 ->   0   -> 1 to unlock HDAL pull blocking mode (fake semaphore.... but it's OK, because _ISF_AudDec_Get_PullQueue() will return FALSE, there's no actual data in queue )
	// [case_2] have data in pullQ, then this n -> (n-1) -> n to recover the semaphore count
	vos_sem_sig(SEMID_UVC_UAC_QUEUE);
}

BOOL _UVAC_put_pullque(uintptr_t addr, UINT32 size)
{
	PUVAC_AUD_RAWQ pObj;

	pObj = &uac_raw_que;

	if ((pObj->Front != 0) && (pObj->Front - 1) == pObj->Rear) {
		//DBG_ERR("[UAC][RX] Pull Queue is Full!\r\n");
		return FALSE;
	} else if ((pObj->Front == 0) && ((pObj->Rear+1) == UVAC_AUD_RAWQ_MAX)) {
		//DBG_ERR("[UAC][RX] Pull Queue is Full!\r\n");
		return FALSE;
	} else {
		pObj->queue[pObj->Rear].addr = addr;
		pObj->queue[pObj->Rear].size = size;

		pObj->Rear = (pObj->Rear + 1) % UVAC_AUD_RAWQ_MAX;

		vos_sem_sig(SEMID_UVC_UAC_QUEUE); // PULLQ + 1
		return TRUE;
	}
}

BOOL _UVAC_release_all_pullque(UVAC_STRM_PATH path)
{
	UVAC_STRM_FRM strm_frm;

	//consume all pull queue
	while (E_OK == UVAC_PullOutStrm(&strm_frm, 0));

	return TRUE;
}

ER U3UVAC_PullOutStrm(PUVAC_STRM_FRM pStrmFrm, INT32 wait_ms)
{
	PUVAC_AUD_RAWQ pObj;

	if (wait_ms < 0) {
		// blocking (wait until data available) , if success PULLQ - 1 , else wait forever (or until signal interrupt and return FALSE)
		if (vos_sem_wait_interruptible(SEMID_UVC_UAC_QUEUE)) {
			return E_NOEXS;
		}
	} else  {
		// non-blocking (wait_ms=0) , timeout (wait_ms > 0). If success PULLQ - 1 , else just return FALSE
		if (vos_sem_wait_timeout(SEMID_UVC_UAC_QUEUE, vos_util_msec_to_tick(wait_ms))) {
			DBG_IND("[ISF_AUDCAP][%d] Pull Queue Semaphore timeout!\r\n");
			return E_TMOUT;
		}
	}

	// check state
	if (gUvacOpened == FALSE) {
		return E_OBJ;
	}

	pObj = &uac_raw_que;

	if ((pObj->Front == pObj->Rear)) {
		//DBG_ERR("[ISF_AUDCAP][%d] Pull Queue is Empty !!!\r\n", pathID);   // This should normally never happen !! If so, BUG exist. ( Because we already check & wait data available at upper code. )
		return E_NOEXS;
	} else {
		pStrmFrm->path = UVAC_STRM_AUDRX;
		pStrmFrm->addr = pObj->queue[pObj->Front].addr;
		pStrmFrm->size = pObj->queue[pObj->Front].size;

		pObj->Front= (pObj->Front + 1) % UVAC_AUD_RAWQ_MAX;

		return E_OK;
	}
}

ER U3UVAC_ReleaseOutStrm(PUVAC_STRM_FRM pStrmFrm)
{
	PUVAC_AUD_RAWQ pObj;

	pObj = &uac_raw_que;

	pObj->lock_addr = pStrmFrm->addr + pStrmFrm->size;

	if (pObj->lock_addr == (gAudRxBuf[0].pa + gAudRxBuf[0].size)) {
		pObj->lock_addr = gAudRxBuf[0].pa;
	}

	return E_OK;
}

void* UVAC_AudioRxTsk(void *arglist)
{
	FLGPTN uiFlag = 0;
	ER     ret;
	UINT32 recv_size;
	uintptr_t recv_addr;
	//UINT64 t1, t2;
	UINT64 block_time = 0;
	//BOOL add_queue;

	//coverity[no_escape]
	while (1) {

		wai_flg(&uiFlag, FLG_ID_UVAC_UAC_RX, FLGUVAC_AUD_DATA_OUT | FLGUVAC_AUD_EXIT, TWF_ORW);

		if (block_time == 0) {
			block_time = (((UINT64)gUvacUacBlockSize)*1000000) / (((UINT64)gUvacAudRxSampleRate[0]) * ((UINT64)gUacRxChNum) * UAC_BIT_PER_SECOND / 8);
			DBG_IND("block_time = %llu\r\n", block_time);
		}

		if (uiFlag & FLGUVAC_AUD_EXIT) {
			break;
		}

		if (uiFlag & FLGUVAC_AUD_DATA_OUT) {

			usb3dev_setInterruptEnable(UVAC_USB_RX_EP[0], DISABLE);

			if (!gUacAudStart[UVAC_AUD_DEV_CNT_3]) {
				usb3dev_setInterruptEnable(UVAC_USB_RX_EP[0], ENABLE);
				continue;
			}

			if ( usb3dev_chkEPBusy(UVAC_USB_RX_EP[0], U3DEV_EPDIR_OUT)) {
				DBG_ERR("EP busy\r\n");
				usb3dev_setInterruptEnable(UVAC_USB_RX_EP[0], ENABLE);
				continue;
			}

			recv_size = gUvacUacBlockSize;

			if (!_UVAC_GetRaw(&recv_addr, recv_size)) {
				DBG_ERR("No buffer\r\n");
				usb3dev_setInterruptEnable(UVAC_USB_RX_EP[0], ENABLE);
				continue;
			}

			//vos_cpu_dcache_sync((VOS_ADDR)(gAudRxBuf[0].va + read_size), recv_size, VOS_DMA_BIDIRECTIONAL);
			//t1 = hd_gettime_us();

			ret = usb3dev_readEndpoint(UVAC_USB_RX_EP[0], (UINT8 *) recv_addr, &recv_size);

			//t2 = hd_gettime_us();

			usb3dev_setInterruptEnable(UVAC_USB_RX_EP[0], ENABLE);

			if (recv_size != gUvacUacBlockSize && gUacAudStart[UVAC_AUD_DEV_CNT_3]) {
				printf("UAC[%d] RX recv=%d, expect=%d\r\n", UVAC_AUD_DEV_CNT_3, recv_size, gUvacUacBlockSize);
			}

			/*if ((t2 - t1) > block_time*2) {
				DBG_IND("%llu t1 = %llu, t2 = %llu\r\n", t2 - t1, t1, t2);
				add_queue = FALSE;
			} else {
				add_queue = TRUE;
			}*/

			if(!_UVAC_put_pullque(recv_addr, recv_size)) {
				DBG_ERR("UAC RX put queue failed\r\n");
			}

			if (ret != E_OK) {
				DBG_ERR("read error\r\n");
			}
		}

	}

	set_flg(FLG_ID_UVAC_UAC_RX, FLGUVAC_AUD_IDLE);

	return 0;
}
ER usb3_writeEndpointTimeout(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen, UINT32 remain)
{
	return usb3dev_writeEndpoint(EPn, pBuffer, pDMALen);
}

INT32 U3UVAC_WriteHidData(void *p_buf, UINT32 buffer_size, INT32 timeout)
{
	UINT32 SemID;
	USB_EP DataInEP;
	UINT32 BufSize;
	INT32 ret_value = -1;
	VOS_TICK begin = 0, current = 0;
	UINT32 elapsed = 0, remain;

	if (gUvacOpened == FALSE) {
		return ret_value;
	}

	if (FALSE == g_u3_hid_info.en) {
		return ret_value;
	}

	if (buffer_size > USB_MAX_DMA_LENGTH) {
		DBG_ERR("Max data length is 0x%X\r\n", USB_MAX_DMA_LENGTH);
		return ret_value;
	}

	SemID = SEMID_UVC_WRITE_HID;
	DataInEP = HID_INTRIN_EP;
	wai_sem(SemID);
	if (gUvacOpened == FALSE) {
		sig_sem(SemID);
		return ret_value;
	}
	vos_perf_mark(&begin);
	while(usb3dev_chkEPBusy(DataInEP, U3DEV_EPDIR_IN)) {
		vos_perf_mark(&current);
		elapsed = vos_perf_duration(begin, current)/ 1000;

		if (timeout == 0 || (elapsed > (UINT32)timeout)) {
			sig_sem(SemID);
			return ret_value;
		}
		vos_util_delay_ms(10);
	}
	if ((UINT32)timeout > elapsed) {
		remain = (UINT32)timeout - elapsed;
	} else {
		remain = 0;
	}
	if (buffer_size) {
		BufSize = buffer_size;

		usb3_writeEndpointTimeout(DataInEP, p_buf, &BufSize, remain);
		//usb3dev_writeEndpoint(DataInEP, p_buf, &BufSize);
		ret_value = (INT32)BufSize;
	}
	sig_sem(SemID);
	return ret_value;
}
INT32 U3UVAC_ReadHidData(void *p_buf, UINT32 buffer_size, INT32 timeout)
{
	UINT32  dataSize;
	UINT32 SemID;
	USB_EP DataOutEP;
	INT32 ret_value = -1;
	FLGPTN      uiFlag = 0;

	if (gUvacOpened == FALSE) {
		return ret_value;
	}

	if (FALSE == g_u3_hid_info.en) {
		return ret_value;
	}

	SemID = SEMID_UVC_READ_HID;
	DataOutEP = HID_INTROUT_EP;

	wai_sem(SemID);
	if (gUvacOpened == FALSE) {
		sig_sem(SemID);
		return ret_value;
	}

	vos_flag_wait_timeout(&uiFlag, FLG_ID_UVAC, FLGUVAC_HID_DATA_OUT, TWF_CLR | TWF_ORW, vos_util_msec_to_tick(timeout));
	if (uiFlag & FLGUVAC_HID_DATA_OUT) {
		dataSize = usb3dev_getEPBytecount(DataOutEP);
		if (dataSize > buffer_size) {
			dataSize = buffer_size;
		}
		//usb_readEndpointTimeout(DataOutEP, (UINT8 *) p_buf, &dataSize, remain);
		usb3dev_readEndpoint(DataOutEP, (UINT8 *) p_buf, &dataSize);
		ret_value = (INT32)dataSize;
		usb3dev_setInterruptEnable(DataOutEP, ENABLE);
	}
	sig_sem(SemID);

	return ret_value;
}

void _U3UVAC_stream_enable(UVAC_STRM_PATH stream, BOOL enable)
{
	uintptr_t event = 0;

	switch (stream) {
	case UVAC_STRM_VID:
		event = (gUvacIntfIdx_VS[0] << 16) | enable;
		break;
	case UVAC_STRM_AUD:
		event = (gUvacIntfIdx_AS[0] << 16) | enable;
		break;
	case UVAC_STRM_VID2:
		event = (gUvacIntfIdx_VS[1] << 16) | enable;
		break;
	case UVAC_STRM_AUD2:
		event = (gUvacIntfIdx_AS[1] << 16) | enable;
		break;
	default:
		break;
	}
	UVAC_InterfaceCB(event);
}



//@}

