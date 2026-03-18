#include <string.h>
#include "UvacVideoTsk.h"
#include "UvacDesc.h"
#include "UVAC.h"
#include "UvacDbg.h"
#include "UvacDesc.h"
#include "UvacReqClass.h"
#include "UvacIsoInTsk.h"
#include "hd_util.h"

extern UINT32 gUvcUsbDMAAbord;
extern const USB_EP UVAC_USB_EP[UVAC_TXF_QUE_MAX];
extern UINT32 gUvcCapImgAddr;
extern UINT32 gUvcCapImgSize;
extern UINT8 gUvacIntfIdx_WinUsb;
extern BOOL gUvacWinIntrfEnable;
extern UINT8 gUvacIntfIdx_VC[UVAC_VID_DEV_CNT_MAX];
extern UINT8 gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_MAX];
extern UINT8 gUvacIntfIdx_AC[UVAC_AUD_DEV_CNT_MAX];
extern UINT8 gUvacIntfIdx_AS[UVAC_AUD_DEV_CNT_MAX];
extern UINT32 gUvacAudSampleRate[UVAC_AUD_SAMPLE_RATE_MAX_CNT];
extern UVAC_VID_RESO gUvcVidResoAry[UVAC_VID_RESO_MAX_CNT];
extern UINT8 gUvacIntfIdx_CDC_COMM[];
extern UINT8 gUvacIntfIdx_CDC_DATA[];
extern UVAC_CDC_PSTN_REQUEST_CB gfpCdcPstnReqCB;
extern BOOL gUvacCdcEnabled[CDC_COM_MAX_NUM];
extern UINT8 gUvacIntfIdx_SIDC;
extern BOOL gUvacMtpEnabled;

extern UVAC_VID_RESO_ARY gU3UvcYuvFrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UVAC_VID_RESO_ARY gU3UvcMjpgFrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UVAC_VID_RESO_ARY gU3UvcH264FrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UVAC_VID_RESO_ARY gU3UvcNV12FrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UVAC_VID_RESO_ARY gU3UvcH265FrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UVAC_VID_RESO_ARY gU3HsUvcYuvFrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UVAC_VID_RESO_ARY gU3HsUvcMjpgFrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UVAC_VID_RESO_ARY gU3HsUvcH264FrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UVAC_VID_RESO_ARY gU3HsUvcNV12FrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UVAC_VID_RESO_ARY gU3HsUvcH265FrmInfo[UVAC_VID_DEV_CNT_MAX];
extern UINT32 gUvcVidResoCnt;
extern UVAC_VID_RESO gUvcVidResoAry[UVAC_VID_RESO_MAX_CNT];
extern UINT8 u3_h264_fmt_index[UVAC_VID_DEV_CNT_MAX];
extern UINT8 u3_mjpeg_fmt_index[UVAC_VID_DEV_CNT_MAX];
extern UINT8 u3_yuv_fmt_index;
extern UINT8 u3_nv12_fmt_index;
extern UINT8 u3_h265_fmt_index[UVAC_VID_DEV_CNT_MAX];
extern UINT32 gUvcMJPGMaxTBR;
extern UINT32 gUvacChannel;
extern UINT32 gHsUvacChannel;
extern UINT32 gUvcVidStart[UVAC_VID_DEV_CNT_MAX];

extern const USB_EP UVAC_USB_RX_EP[UVAC_RXF_QUE_MAX];
extern BOOL gUvacUacRxEnabled;
extern UINT32 gUvacAudRxSampleRate[UVAC_AUD_RX_SAMPLE_RATE_MAX_CNT];

extern UVAC_HID_INFO g_u3_hid_info;
extern UINT8 gU3UvacIntfIdx_HID;

extern UINT8 u3_eu_unit_id[2];
extern UVAC_EU_DESC u3_eu_desc[2];
extern UVAC_EU_DESC_ARRAY* u3_eu_desc_array;
extern UVAC_MSOS20_INFO g_u3_msos_info;
extern U3DEV_SPEED g_usb_speed;
extern UINT32  gUVCIsoinTxfUnitSize[UVAC_VID_DEV_CNT_MAX];

extern USB_EP_BLKNUM gU3UvcIsoInHsBandWidth[UVAC_VID_EP_PACK_SIZE_MAX_CNT];
extern UINT32 gU3UvcIsoInHsPacketSize[UVAC_VID_EP_PACK_SIZE_MAX_CNT];

extern UINT32 gU3UvcIsoInSsPacketSize[UVAC_VID_EP_PACK_SIZE_MAX_CNT];
extern UINT32 gU3UvcIsoInSsMaxBurst[UVAC_VID_EP_PACK_SIZE_MAX_CNT];
extern UINT32 gU3UvcIsoInSsEpcAttr[UVAC_VID_EP_PACK_SIZE_MAX_CNT];
extern USB_EP_BLKNUM gU3UvcIsoInSsBandWidth[UVAC_VID_EP_PACK_SIZE_MAX_CNT];

void UVC_DmpStillProbeCommitData(PUVC_STILL_PROBE_COMMIT pStilProbComm);
static void UVC_VS_Still_ProbeCtrl(UINT8 req, UINT8 *pData, UINT8 intrf);
static void UVC_VS_Still_CommitCtrl(UINT8 req, UINT8 *pData, UINT8 intrf);
static void UVC_VS_Still_ImgTrigCtrl(UINT8 req, UINT8 *pData, UINT8 intrf);
//static void UVC_CapStilImgUpload(UINT8 *pData);


UVC_STILL_PROBE_COMMIT gUvcStillProbeCommit = {0};
UINT8 gUvacStillImgTrigSts = UVC_STILLIMG_TRIG_CTRL_NORMAL;
UINT16 gUACRes = 0x0002;
UVC_PROBE_COMMIT g_ProbeCommit[UVAC_VID_DEV_CNT_MAX] = {0};
static _ALIGNED(64) UINT8 gUvacClsReqDataBuf[USB_UVAC_CLS_DATABUF_LEN];
UINT32 gUacSampleRate = UAC_FREQUENCY_16K;
UINT32 gUacRxSampleRate = UAC_FREQUENCY_16K;
static UINT32 gUvacVendDataLen = USB_UVAC_CLS_DATABUF_LEN;
UVAC_EUVENDCMDCB gUvacEUVendCmdCB[UVAC_EU_VENDCMD_CNT] = {0};
UVAC_WINUSBCLSREQCB gUvcWinUSBReqCB = 0;
UINT32 gUvcWinUsbDataLen = 0;
static BOOL gUacMute = FALSE;
static BOOL gUacRxMute = FALSE;
INT32 u3_uac_vol_cur = UAC_VOL_MAX;
INT32 u3_uac_vol_max = UAC_VOL_MAX;
INT32 u3_uac_vol_min = UAC_VOL_MIN;
INT32 u3_uac_vol_res = UAC_VOL_RES;
UAC_VOL_INFO u3_uac_rx_vol_info = {
	.vol_def = UAC_VOL_MAX,
	.vol_max = UAC_VOL_MAX,
	.vol_min = UAC_VOL_MIN,
	.vol_res = UAC_VOL_RES,
};
UVAC_SETVOLCB gUacSetVolCB = 0;
UVAC_SETVOLCB gUacRxSetVolCB = 0;

UVAC_UNIT_CB g_fpU3UvcCT_CB = NULL;
UVAC_UNIT_CB g_fpU3UvcPU_CB = NULL;
UVAC_UNIT_CB g_fpU3UvcXU_CB = NULL;

typedef enum _UVAC_CLASS_REQ_TYPE {
	UVAC_CLASS_REQ_NONE,
	UVAC_CLASS_REQ_VS_PROBE,
	UVAC_CLASS_REQ_VS_COMMIT,
//    UVAC_CLASS_REQ_AC_PROBE,
	UVAC_CLASS_REQ_AC_COMMIT_SET_CUR,
	UVAC_CLASS_REQ_AC_COMMIT_SET_RES,
	UVAC_CLASS_REQ_CDC,
	UVAC_CLASS_REQ_SIDC,
	UVAC_CLASS_REQ_XU,
	UVAC_CLASS_REQ_AC_MUTE,
	UVAC_CLASS_REQ_AC_VOL,
	UVAC_CLASS_REQ_AC_FREQ_SET_CUR,
	UVAC_CLASS_REQ_CT_SET_CUR,
	UVAC_CLASS_REQ_PU_SET_CUR,
	UVAC_CLASS_REQ_ACO_COMMIT_SET_CUR,
	UVAC_CLASS_REQ_ACO_COMMIT_SET_RES,
	UVAC_CLASS_REQ_ACO_MUTE,
	UVAC_CLASS_REQ_ACO_VOL,
	UVAC_CLASS_REQ_ACO_FREQ_SET_CUR,
	UVAC_CLASS_REQ_HID_SET,
	UVAC_CLASS_REQ_EU_SET_CUR,
	UVAC_CLASS_REQ_EU2_SET_CUR,
	UVAC_CLASS_REQ_EU_ARRAY_SET_CUR,
	ENUM_DUMMY4WORD(UVAC_CLASS_REQ_TYPE)
} UVAC_CLASS_REQ_TYPE;

static UVAC_CLASS_REQ_TYPE  m_RequestType;
static UINT32 m_RequestLen;
static UVC_PROBE_COMMIT *m_pProbeCommit;
static CDC_COM_ID m_ComID;
static UINT32 m_uiXUControlCode;
static UINT32 m_VidDevId;

static UINT32  eu_id = 0;

extern USB3_GENERIC_CB g_u3uvac_msdc_class_req_cb;
extern UVAC_MSDC_INFO g_u3_msdc_info;
extern UINT32 gU3UvacUvcVer;

static BOOL init_probe = FALSE;

static UINT32 set_probe_count = 0;
UINT32 set_probe_count_max = 10;

static void UVC_Unit_Cmd(UVAC_UNIT_CB fp_UnitCB, UINT32 ControlCode, UINT8 request, UINT8 *p_data, UINT16 length)
{
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	DbgMsg_UVCIO(("ctrl=%d, request=0x%x, pData=0x%x\r\n", ControlCode, request, p_data));
	//DBG_DUMP("VendCmd: ctrl=%d, cs=0x%x, pData=0x%x\r\n", ControlCode, CS, pData);

	//ControlCode: 1, 2, ..., 8
	if (fp_UnitCB) {
		if (SET_CUR == request) {
			if (length > USB_UVAC_CLS_DATABUF_LEN) {
				gUvacVendDataLen = USB_UVAC_CLS_DATABUF_LEN;
			} else {
				gUvacVendDataLen = length;
			}
			memset(p_data, 0, USB_UVAC_CLS_DATABUF_LEN);

			if (fp_UnitCB == g_fpU3UvcCT_CB) {
				m_RequestType = UVAC_CLASS_REQ_CT_SET_CUR;
			} else if (fp_UnitCB == g_fpU3UvcPU_CB){
				m_RequestType = UVAC_CLASS_REQ_PU_SET_CUR;
			} else if (fp_UnitCB == g_fpU3UvcXU_CB || fp_UnitCB == u3_eu_desc[0].eu_cb){
				m_RequestType = UVAC_CLASS_REQ_EU_SET_CUR;
			} else if (fp_UnitCB == u3_eu_desc[1].eu_cb){
				m_RequestType = UVAC_CLASS_REQ_EU2_SET_CUR;
			} else {
				UINT32 i;
				BOOL found = FALSE;

				if (u3_eu_desc_array) {
					for (i = 0; i < u3_eu_desc_array->eu_num; i++) {
						if (fp_UnitCB == u3_eu_desc_array->eu_desc[i].eu_cb) {
							m_RequestType = UVAC_CLASS_REQ_EU_ARRAY_SET_CUR;
							eu_id = i;
							found = TRUE;
						}
					}
				}

				if (found == FALSE) {
					bStall = TRUE;
				}

			}
			m_RequestLen = gUvacVendDataLen;
			uiRetBufAddr = (uintptr_t) p_data;
			uiRetSize = m_RequestLen;

			usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			return;
		}
		if (fp_UnitCB(ControlCode, request, p_data, &gUvacVendDataLen)) {
			DbgMsg_UVCIO(("VendCmd Ok, retLen=%d,ctrl=%d, request=0x%x, p_data=0x%x\r\n", gUvacVendDataLen, ControlCode, request, p_data));
			DbgMsg_UVCIO(("  ==> *p_data=0x%x\r\n", *p_data));
			//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, p_data=0x%x", gUvacVendDataLen, ControlCode, CS, p_data);
			//DBG_DUMP("==> *p_data=0x%x\r\n", *p_data);
			if (gUvacVendDataLen) {
				if (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) {
					length = gUvacVendDataLen;
					//DBG_ERR("pDataLen(%d) should <= 64\r\n");
				} else {
					length = gUvacVendDataLen;
				}

				uiRetBufAddr = (uintptr_t) p_data;
				uiRetSize = length;

				usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			}
		} else {
			bStall = TRUE;
			uiRetBufAddr = (uintptr_t) p_data;
			uiRetSize = length;
			//DBG_IND("VendCmd Fail, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, p_data);
			usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
		}
	} else {
		bStall = TRUE;
		uiRetBufAddr = (uintptr_t) p_data;
		uiRetSize = length;
		usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
	}
}
static void UVC_HID_ClassReq(UINT32 ControlCode, UINT8 request, UINT8 *p_data, UINT16 length)
{
	UINT16 wValue;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	DbgMsg_UVCIO(("request=0x%x, pData=0x%x\r\n", request, p_data));

	wValue = (UINT16)ControlCode;

	//gUvacVendDataLen = 0;

	if (g_u3_hid_info.cb) {
		if (HID_SET_REPORT == request || HID_SET_IDLE == request || HID_SET_PROTOCOL == request) {
			if (length > USB_UVAC_CLS_DATABUF_LEN) {
				//gUvacVendDataLen = USB_UVAC_CLS_DATABUF_LEN;
				DBG_ERR("pDataLen(%d) should <= 64\r\n");
				usb3dev_handleSetupResult(TRUE, 0, 0);
			} else {
				memset(p_data, 0, USB_UVAC_CLS_DATABUF_LEN);
				m_RequestType = UVAC_CLASS_REQ_HID_SET;
				m_RequestLen = gUvacVendDataLen = length;
				uiRetBufAddr = (uintptr_t) p_data;
				uiRetSize = m_RequestLen;
				usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			}
			return;

		} else {
			gUvacVendDataLen = CONTROL_DATA.device_request.w_length;
			if (g_u3_hid_info.cb(request, wValue, p_data, &gUvacVendDataLen)) {
				//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, p_data=0x%x", gUvacVendDataLen, ControlCode, CS, p_data);
				//DBG_DUMP("==> *p_data=0x%x\r\n", *p_data);
				if (gUvacVendDataLen) {
					if (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) {
						UINT8 *p_addr = (UINT8 *)*(uintptr_t *)p_data;

						uiRetBufAddr = (uintptr_t) p_addr;
					} else {
						uiRetBufAddr = (uintptr_t) p_data;
					}
					uiRetSize = gUvacVendDataLen;

					usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
				}
			} else {
				usb3dev_handleSetupResult(TRUE, 0, 0);
			}
		}
	} else {
		usb3dev_handleSetupResult(TRUE, 0, 0);
	}


}
void UVC_WinUSB_ClassReq(UINT32 ControlCode, UINT8 CS, UINT8 *pData)
{
	//UINT8 vendCBRet;
	DbgMsg_UVC(("+WinUSB_ClassReq: ctrl=%d, cs=0x%x, pData=0x%x, cb=0x%x\r\n", ControlCode, CS, pData, gUvcWinUSBReqCB));
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
	if (gUvcWinUSBReqCB) {
		vendCBRet = gUvcWinUSBReqCB(ControlCode, CS, pData, &gUvcWinUsbDataLen);
		if (TRUE == vendCBRet) {
			DbgMsg_UVC(("WinUSB_ClassReq Ok, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvcWinUsbDataLen, ControlCode, CS, pData));
			DbgMsg_UVC(("  ==> *pData=0x%x\r\n", (UINT32)*pData));
			gUvcWinUsbDataLen = (gUvcWinUsbDataLen > USB_UVAC_CLS_DATABUF_LEN) ? USB_UVAC_CLS_DATABUF_LEN : gUvcWinUsbDataLen;
			ControlData.w_length  = gUvcWinUsbDataLen;
			ControlData.pData    = pData;
			usb_RetSetupData();
		} else {
			DBG_ERR("WinUSB_ClassReq Fail, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvcWinUsbDataLen, ControlCode, CS, (UINT32)pData);
			usb_setEP0Stall();
			usb_setEP0Done();
		}
	}
#endif
	DbgMsg_UVC(("-WinUSB_ClassReq: ctrl=%d, cs=0x%x, pData=0x%x\r\n", ControlCode, CS, pData));
}
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
/**
    UVC Select Unit control

    @param UINT8 CS       Control attribute
    @param UINT8 *pData   Data pointer
    @return void
*/
static void UVC_SU_SelectCtrl(UINT8 CS, UINT8 *pData)
{
	UINT8 *pDataAddr;
	UINT8 datasize;

	pDataAddr = pData;
	DbgMsg_UVCIO(("%s:%d,len=%d\r\n", __func__, CS, ControlData.w_length));
	switch (CS) {
	case GET_INFO:
		*pData = 0x1;
		datasize = 1;
		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;

		usb_RetSetupData();
		break;
	case SET_CUR:
	case GET_CUR:
	case GET_MIN:
	case GET_MAX:
	case GET_RES:
	case GET_LEN:
	case GET_DEF:
		//stall EP0
		usb_setEP0Stall();
		usb_setEP0Done();
		DBG_ERR("UVC:[SU Select control] Not imp\r\n");
		break;
	}
}

/**
    UVC VideoControl interface -- Request error code control

    @param UINT8 CS       Control attribute
    @param UINT8 *pData   Data pointer
    @return void
*/
static void UVC_VC_ErrorCtrl(UINT8 CS, UINT8 *pData)
{
	UINT8 *pDataAddr;
	UINT8 datasize;

	pDataAddr = pData;
	DbgMsg_UVCIO(("%s:%d\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR:
		DbgMsg_UVCIO(("UVC:[VC error control] --GET_CUR\r\n"));
		*pData = 0x06;
		datasize = 1;
		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;

		usb_RetSetupData();
		break;
	case SET_CUR:
	case GET_INFO:
	case GET_MIN:
	case GET_MAX:
	case GET_RES:
	case GET_LEN:
	case GET_DEF:
		//stall EP0
		usb_setEP0Stall();
		usb_setEP0Done();
		DbgMsg_UVCIO(("UVC:[VC error control] Not imp\r\n"));
		break;
	}

}
#elif (UVAC_CONTROLLER == UVAC_CONTROL_U3)
/**
    UVC Select Unit control

    @param UINT8 CS       Control attribute
    @param UINT8 *pData   Data pointer
    @return void
*/
static void UVC_SU_SelectCtrl(UINT8 CS, UINT8 *pData)
{
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	//DbgMsg_UVCIO(("%s:%d,len=%d\r\n", __func__, CS, ControlData.w_length));
	switch (CS) {
	case GET_INFO:
		*pData = 0x1;
		uiRetBufAddr = (uintptr_t)pData;
		uiRetSize = 1;
		break;
	case SET_CUR:
	case GET_CUR:
	case GET_MIN:
	case GET_MAX:
	case GET_RES:
	case GET_LEN:
	case GET_DEF:
		//stall EP0
		bStall = TRUE;
		DBG_ERR("UVC:[SU Select control] Not imp\r\n");
		break;
	}
	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
}

/**
    UVC VideoControl interface -- Request error code control

    @param UINT8 CS       Control attribute
    @param UINT8 *pData   Data pointer
    @return void
*/
static void UVC_VC_ErrorCtrl(UINT8 CS, UINT8 *pData)
{
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;
	DbgMsg_UVCIO(("%s:%d\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR:
		DbgMsg_UVCIO(("UVC:[VC error control] --GET_CUR\r\n"));
		*pData = 0x06;
		uiRetBufAddr = (uintptr_t)pData;
		uiRetSize = 1;
		break;
	case SET_CUR:
	case GET_INFO:
	case GET_MIN:
	case GET_MAX:
	case GET_RES:
	case GET_LEN:
	case GET_DEF:
		//stall EP0
		bStall = TRUE;
		DbgMsg_UVCIO(("UVC:[VC error control] Not imp\r\n"));
		break;
	}
	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
}
#endif
static void UVC_DmpProbeData(UINT32 srcId, UVC_PROBE_COMMIT *pProbeCommit)
{
#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
	DBG_DUMP("+%s:%d, pProbeCommit=0x%x\r\n", __func__, srcId, pProbeCommit);
	DBG_DUMP("bmHint=0x%x\r\n",                            pProbeCommit->bmHint);
	DBG_DUMP("bFormatIndex=0x%x\r\n",                      pProbeCommit->bFormatIndex);
	DBG_DUMP("bFrameIndex=0x%x\r\n",                       pProbeCommit->bFrameIndex);
	DBG_DUMP("dwFrameInterval=0x%x\r\n",                   pProbeCommit->dwFrameInterval);
	DBG_DUMP("wKeyFrameRate=0x%x\r\n",                     pProbeCommit->wKeyFrameRate);
	DBG_DUMP("wPFrameRate=0x%x\r\n",                       pProbeCommit->wPFrameRate);
	DBG_DUMP("wCompQuality=0x%x\r\n",                      pProbeCommit->wCompQuality);
	DBG_DUMP("wCompWindowSize=0x%x\r\n",                   pProbeCommit->wCompWindowSize);
	DBG_DUMP("wDelay=0x%x\r\n",                            pProbeCommit->wDelay);
	DBG_DUMP("dwMaxVideoFrameSize=0x%x\r\n",               pProbeCommit->dwMaxVideoFrameSize);
	DBG_DUMP("dwMaxPayloadTransferSize=0x%x\r\n",          pProbeCommit->dwMaxPayloadTransferSize);
	if (gU3UvacUvcVer >= UVAC_UVC_VER_110) {
		DBG_DUMP("dwClockFrequency=0x%x\r\n",              pProbeCommit->dwClockFrequency);
		DBG_DUMP("bmFramingInfo=0x%x\r\n",                 pProbeCommit->bmFramingInfo);
		DBG_DUMP("bPreferredVersion=0x%x\r\n",             pProbeCommit->bPreferredVersion);
		DBG_DUMP("bMinVersion=0x%x\r\n",                   pProbeCommit->bMinVersion);
		DBG_DUMP("bMaxVersion=0x%x\r\n",                   pProbeCommit->bMaxVersion);
	}
	DBG_DUMP("-%s\r\n", __func__);
#endif
}
static void UVC_GetCurProbeCommitData(UINT8 *pData, UVC_PROBE_COMMIT *pProbeCommit)
{
	pData += 2;
	*pData++ = pProbeCommit->bFormatIndex;
	*pData++ = pProbeCommit->bFrameIndex;

	*pData++ = pProbeCommit->dwFrameInterval & 0xFF;
	*pData++ = (pProbeCommit->dwFrameInterval >> 8) & 0xFF;
	*pData++ = (pProbeCommit->dwFrameInterval >> 16) & 0xFF;
	*pData++ = (pProbeCommit->dwFrameInterval >> 24) & 0xFF;

	pData += 4;

	*pData++ = pProbeCommit->wCompQuality& 0xFF;
	*pData++ = (pProbeCommit->wCompQuality >> 8) & 0xFF;

	pData += 4;

	*pData++ = pProbeCommit->dwMaxVideoFrameSize & 0xFF;
	*pData++ = (pProbeCommit->dwMaxVideoFrameSize >> 8) & 0xFF;
	*pData++ = (pProbeCommit->dwMaxVideoFrameSize >> 16) & 0xFF;
	*pData++ = (pProbeCommit->dwMaxVideoFrameSize >> 24) & 0xFF;

	*pData++ = (pProbeCommit->dwMaxPayloadTransferSize) & 0xFF;
	*pData++ = (pProbeCommit->dwMaxPayloadTransferSize >> 8) & 0xFF;
	*pData++ = (pProbeCommit->dwMaxPayloadTransferSize >> 16) & 0xFF;
	*pData++ = (pProbeCommit->dwMaxPayloadTransferSize >> 24) & 0xFF;

	if (gU3UvacUvcVer >= UVAC_UVC_VER_110) {
		*pData++= (pProbeCommit->dwClockFrequency) & 0xFF;
		*pData++= (pProbeCommit->dwClockFrequency>>8) & 0xFF;
		*pData++= (pProbeCommit->dwClockFrequency>>16) & 0xFF;
		*pData++= (pProbeCommit->dwClockFrequency>>24) & 0xFF;
		*pData++= pProbeCommit->bmFramingInfo;
		*pData++= pProbeCommit->bPreferredVersion;
		*pData++= pProbeCommit->bMinVersion;
		*pData++= pProbeCommit->bMaxVersion;
	}

	if (gU3UvacUvcVer == UVAC_UVC_VER_150) {
		*pData++= pProbeCommit->bUsage;
		*pData++= pProbeCommit->bBitDepthLuma;
		*pData++= pProbeCommit->bmSettings;
		*pData++= pProbeCommit->bMaxNumberOfRefFramesPlus1;

		*pData++= (pProbeCommit->bmRateControlModes) & 0xFF;
		*pData++= (pProbeCommit->bmRateControlModes>>8) & 0xFF;

		*pData++= (pProbeCommit->bmLayoutPerStream[0]) & 0xFF;
		*pData++= (pProbeCommit->bmLayoutPerStream[0]>>8) & 0xFF;
		*pData++= (pProbeCommit->bmLayoutPerStream[1]) & 0xFF;
		*pData++= (pProbeCommit->bmLayoutPerStream[1]>>8) & 0xFF;
		*pData++= (pProbeCommit->bmLayoutPerStream[2]) & 0xFF;
		*pData++= (pProbeCommit->bmLayoutPerStream[2]>>8) & 0xFF;
		*pData++= (pProbeCommit->bmLayoutPerStream[3]) & 0xFF;
		*pData++= (pProbeCommit->bmLayoutPerStream[3]>>8) & 0xFF;
	}
}
void UVC_InitProbeCommitData(void)
{
	UINT32 i = 0, dev_cnt;
	UVC_PROBE_COMMIT *pProbeCommit = 0;
	UINT32 uvac_channel = (gUvacChannel > gHsUvacChannel)? gUvacChannel : gHsUvacChannel;

	memset((void *)g_ProbeCommit, 0, (sizeof(UVC_PROBE_COMMIT)* UVAC_VID_DEV_CNT_MAX));

	if (uvac_channel > UVAC_CHANNEL_2V1A) {
		dev_cnt = 3;
	} else if (uvac_channel == UVAC_CHANNEL_1V1A){
		dev_cnt = 1;
	} else {
		dev_cnt = 2;
	}

	g_usb_speed = usb3dev_getDevInfo(U3DEV_DEVINFO_ID_SPEED);

	for (i = 0; i < dev_cnt; i++) {
		pProbeCommit = &g_ProbeCommit[i];
		pProbeCommit->bmHint = 0;
		pProbeCommit->bFormatIndex = UVC_VSFMT_FIRSTINDEX;
		pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;
		pProbeCommit->dwFrameInterval = UVC_VSFRM_FRAMEINTERVAL_MIN;
		pProbeCommit->wKeyFrameRate = UVC_FRMRATE_30;//0
		pProbeCommit->wPFrameRate = 0;
		pProbeCommit->wCompQuality = 0;//0x3D;
		pProbeCommit->wCompWindowSize = 0;
		pProbeCommit->wDelay = 666;
		if (u3_uvc_bulk_mode[i]) {
			if (g_usb_speed == U3DEV_SPEED_SS) {
				pProbeCommit->dwMaxPayloadTransferSize = u3_uvc_bulk_payload_size;
			} else {
				pProbeCommit->dwMaxPayloadTransferSize = u3_uvc_hs_bulk_payload_size;
			}
		}else if (g_usb_speed == U3DEV_SPEED_SS) {
		//if (0) {
			pProbeCommit->dwMaxPayloadTransferSize = gU3UvcIsoInSsPacketSize[UVAC_VID_EP_PACK_SIZE_MAX_CNT-1] * gU3UvcIsoInSsBandWidth[UVAC_VID_EP_PACK_SIZE_MAX_CNT-1];
		} else {
			pProbeCommit->dwMaxPayloadTransferSize = gU3UvcIsoInHsPacketSize[UVAC_VID_EP_PACK_SIZE_MAX_CNT-1] * gU3UvcIsoInHsBandWidth[UVAC_VID_EP_PACK_SIZE_MAX_CNT-1];
		}

		if (pProbeCommit->bFormatIndex == u3_h264_fmt_index[i]) {
			pProbeCommit->dwMaxVideoFrameSize = gUvcMJPGMaxTBR; //no use for h264
		} else if (pProbeCommit->bFormatIndex == u3_mjpeg_fmt_index[i]) {
			pProbeCommit->dwMaxVideoFrameSize = gUvcVidResoAry[UVC_VSFMT_DEF_FRM_IDX-1].width*gUvcVidResoAry[UVC_VSFMT_DEF_FRM_IDX-1].height*2;
		} else if (pProbeCommit->bFormatIndex == u3_yuv_fmt_index) {
			pProbeCommit->dwMaxVideoFrameSize = gU3UvcYuvFrmInfo[i].pVidResAry[UVC_VSFMT_DEF_FRM_IDX-1].width*gU3UvcYuvFrmInfo[i].pVidResAry[UVC_VSFMT_DEF_FRM_IDX-1].height*2;
		} else if (pProbeCommit->bFormatIndex == u3_nv12_fmt_index) {
			pProbeCommit->dwMaxVideoFrameSize = gU3UvcNV12FrmInfo[i].pVidResAry[UVC_VSFMT_DEF_FRM_IDX-1].width*gU3UvcNV12FrmInfo[i].pVidResAry[UVC_VSFMT_DEF_FRM_IDX-1].height*2;
		} else if (pProbeCommit->bFormatIndex == u3_h265_fmt_index[i]) {
			pProbeCommit->dwMaxVideoFrameSize = gUvcMJPGMaxTBR; //no use for h265
		} else {
			DBG_ERR("[%d]unknow format index=%d, h264=%d, mjpeg=%d, yuv=%d, nv12=%d, h265=%d\r\n", i, pProbeCommit->bFormatIndex, u3_h264_fmt_index[i], u3_mjpeg_fmt_index[i], u3_yuv_fmt_index, u3_nv12_fmt_index, u3_h265_fmt_index[i]);
		}

		if (gU3UvacUvcVer >= UVAC_UVC_VER_110) {
			pProbeCommit->dwClockFrequency = UVC_VC_CLOCK_FREQUENCY;
			pProbeCommit->bmFramingInfo = 0;
			pProbeCommit->bPreferredVersion = 1;
			pProbeCommit->bMinVersion = 1;

			if (u3_nv12_fmt_index != 0) {
				pProbeCommit->bMaxVersion = u3_nv12_fmt_index;
			} else if (u3_yuv_fmt_index != 0) {
				pProbeCommit->bMaxVersion = u3_yuv_fmt_index;
			} else if (u3_mjpeg_fmt_index[i] != 0) {
				pProbeCommit->bMaxVersion = u3_mjpeg_fmt_index[i];
			} else if (u3_h265_fmt_index[i] != 0) {
				pProbeCommit->bMaxVersion = u3_h265_fmt_index[i];
			} else {
				pProbeCommit->bMaxVersion = u3_h264_fmt_index[i];
			}
		}

		if (gU3UvacUvcVer == UVAC_UVC_VER_150) {
			pProbeCommit->bUsage = 0;
			pProbeCommit->bBitDepthLuma = 0;
			pProbeCommit->bmSettings = 0;
			pProbeCommit->bMaxNumberOfRefFramesPlus1 = 0;
			pProbeCommit->bmRateControlModes = 0;
			pProbeCommit->bmLayoutPerStream[0] = 0;
			pProbeCommit->bmLayoutPerStream[1] = 0;
			pProbeCommit->bmLayoutPerStream[2] = 0;
			pProbeCommit->bmLayoutPerStream[3] = 0;
		}
	}
}
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
static void UVC_VS_ProbeCtrl(UINT8 CS, UINT8 *pData, UVC_PROBE_COMMIT *pProbeCommit)
{
	UINT8 *pDataAddr;
	UINT8 datasize;
	UINT32 ProbeData;

	pDataAddr = pData;
	DbgMsg_UVC(("^B+%s:0x%x, pProbeCommit=0x%x\r\n", __func__, CS, pProbeCommit));

	switch (CS) {
	case GET_CUR:
		DbgMsg_UVCIO(("^BUVC:[VS Probe control --GET_CUR]:%d\r\n", ControlData.w_length));
		UVC_DmpProbeData(1, pProbeCommit);
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));
		UVC_GetCurProbeCommitData(pData, pProbeCommit);
		datasize = USB_UVC_PROBE_LENGTH;
		if (datasize > ControlData.w_length) {
			DBG_WRN("dataSize=%d,%d\r\n", datasize, ControlData.w_length);
			datasize = ControlData.w_length;
		}
		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;

	case SET_CUR:
		DbgMsg_UVCIO(("^B+UVC:[VS Probe control --SET_CUR]\r\n"));
		memset(pProbeCommit, 0, sizeof(UVC_PROBE_COMMIT));

		ProbeData = USB_UVC_PROBE_LENGTH;
		memset(pData, 0, USB_UVC_PROBE_LENGTH);
		usb_readEndpoint(USB_EP0, pData, &ProbeData);
		usb_setEP0Done();

		pProbeCommit->bFormatIndex = *(pData + 2);
		pProbeCommit->bFrameIndex = *(pData + 3);

		pProbeCommit->dwFrameInterval = *(pData + 4);
		pProbeCommit->dwFrameInterval |= (*(pData + 5) << 8);
		pProbeCommit->dwFrameInterval |= (*(pData + 6) << 16);
		pProbeCommit->dwFrameInterval |= (*(pData + 7) << 24);

		pProbeCommit->dwMaxVideoFrameSize = UVC_VSFRM_MAX_FBSIZE;
		//if (0) {
		if (g_usb_speed == U3DEV_SPEED_SS) {
			pProbeCommit->dwMaxPayloadTransferSize = UVC_ISOIN_HS_PACKET_SIZE * UVC_ISOIN_SS_BANDWIDTH;
		} else {
			pProbeCommit->dwMaxPayloadTransferSize = UVC_ISOIN_HS_PACKET_SIZE * UVC_ISOIN_BANDWIDTH;
 		}

		UVC_DmpProbeData(2, pProbeCommit);

		DbgMsg_UVCIO(("UVC: Format index =%d Frame index =%d  Interval=0x%x \r\n",
					  pProbeCommit->bFormatIndex, pProbeCommit->bFrameIndex, pProbeCommit->dwFrameInterval));
		break;

	case GET_INFO:
		DbgMsg_UVCIO(("^BUVC:[VS Probe control --GET_INFO]\r\n"));
		*pData++ = 0x03;
		ControlData.w_length  = 1;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;

	case GET_MIN:
		DbgMsg_UVCIO(("^BUVC:[VS Probe control --GET_MIN]\r\n"));
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));

		pData += 2;
		*pData++ = 1;
		*pData++ = 1;

		*pData++ = UVC_VSFRM_FRAMEINTERVAL_MIN & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_MIN >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_MIN >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_MIN >> 24) & 0xFF;

		pData += 10;
		*pData++ = UVC_VSFRM_MAX_FBSIZE & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 24) & 0xFF;

		*pData++ = (UVC_VSFRM_MAX_FBSIZE + 2) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 24) & 0xFF;

		datasize = USB_UVC_PROBE_LENGTH;

		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		DbgMsg_UVCIO(("^B-UVC:[VS Probe control --GET_MIN]:%d\r\n", ControlData.w_length));
		usb_RetSetupData();
		DbgMsg_UVCIO(("^B---UVC:[VS Probe control --GET_MIN]:%d\r\n", ControlData.w_length));
		break;

	case GET_MAX:
		DbgMsg_UVCIO(("^B+UVC:[VS Probe control --GET_MAX]\r\n"));
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));
		UVC_GetCurProbeCommitData(pData, pProbeCommit);
		datasize = USB_UVC_PROBE_LENGTH;

		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		DbgMsg_UVCIO(("^B-UVC:[VS Probe control --GET_MAX]:%d\r\n", ControlData.w_length));
		usb_RetSetupData();
		DbgMsg_UVCIO(("^B---UVC:[VS Probe control --GET_MAX]\r\n"));
		break;

	case GET_RES:
		usb_setEP0Stall();
		usb_setEP0Done();
		DBG_ERR("^RUVC:[VS Probe control --GET_RES] Not imp\r\n");
		break;

	case GET_LEN:
		DbgMsg_UVCIO(("^BUVC:[VS Probe control --GET_LEN]\r\n"));
		*pData++ = USB_UVC_PROBE_LENGTH;
		ControlData.w_length  = 1;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;

	case GET_DEF:
		DbgMsg_UVCIO(("^BUVC:[VS Probe control --GET_DEF]\r\n"));
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));
		pData += 2;
		*pData++ = 1;
		*pData++ = 1;

		*pData++ = UVC_VSFRM_FRAMEINTERVAL_DEF & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_DEF >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_DEF >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_DEF >> 24) & 0xFF;

		pData += 10;
		*pData++ = UVC_VSFRM_MAX_FBSIZE & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 24) & 0xFF;

		*pData++ = (UVC_VSFRM_MAX_FBSIZE + 2) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 24) & 0xFF;

		datasize = USB_UVC_PROBE_LENGTH;

		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;

	default:
		DBG_ERR("^RUVC:[VS Probe control -- %d] not support\r\n", CS);
		break;
	}
	DbgMsg_UVCIO(("^B-%s:0x%x\r\n", __func__, CS));
}


/**
    UVC VideoStreaming Control -- Commit

    @param UINT8 CS       Control attribute
    @param UINT8 *pData   Data pointer
    @return void
*/

static void UVC_VS_CommitCtrl(UINT8 CS, UINT8 *pData, UVC_PROBE_COMMIT *pProbeCommit, UINT32 vidDevIdx)
{
	UINT8 *pDataAddr;
	UINT8 datasize;
	UINT32 len;
	UVAC_VIDEO_FORMAT codecType = UVAC_VIDEO_FORMAT_H264;

	pDataAddr = pData;
	DbgMsg_UVC(("^B%s:0x%x, vidDevIdx=%d\r\n", __func__, CS, vidDevIdx));
	switch (CS) {
	case SET_CUR:
		DbgMsg_UVCIO(("^B-UVC:[VS Commit control --SET_CUR],Prv frmIdx=%d\r\n", pProbeCommit->bFrameIndex));
		len = USB_UVC_PROBE_LENGTH;
		if (E_OK != usb_readEndpoint(USB_EP0, pData, &len)) {
			DBG_ERR("Commit SetCur readEndpoint fail\r\n");
		}
		pProbeCommit->bFormatIndex = *(pData + 2);
		pProbeCommit->bFrameIndex = *(pData + 3);
		pProbeCommit->dwFrameInterval = *(pData + 4);
		pProbeCommit->dwFrameInterval |= *(pData + 5) << 8;
		pProbeCommit->dwFrameInterval |= *(pData + 6) << 16;
		pProbeCommit->dwFrameInterval |= *(pData + 7) << 24;

		pProbeCommit->wKeyFrameRate = *(pData + 8);
		pProbeCommit->wKeyFrameRate |= (*(pData + 9) << 8);
		pProbeCommit->wPFrameRate = *(pData + 10);
		pProbeCommit->wPFrameRate |= (*(pData + 11) << 8);
		pProbeCommit->wCompQuality = *(pData + 12);
		pProbeCommit->wCompQuality |= (*(pData + 13) << 8);
		pProbeCommit->wCompWindowSize = *(pData + 14);
		pProbeCommit->wCompWindowSize |= (*(pData + 15) << 8);
		pProbeCommit->wDelay = *(pData + 16);
		pProbeCommit->wDelay |= (*(pData + 17) << 8);
		pProbeCommit->dwMaxVideoFrameSize = *(pData + 18);
		pProbeCommit->dwMaxVideoFrameSize |= (*(pData + 19) << 8);
		pProbeCommit->dwMaxVideoFrameSize |= (*(pData + 20) << 16);
		pProbeCommit->dwMaxVideoFrameSize |= (*(pData + 21) << 24);
		pProbeCommit->dwMaxPayloadTransferSize = *(pData + 22);
		pProbeCommit->dwMaxPayloadTransferSize |= (*(pData + 23) << 8);
		pProbeCommit->dwMaxPayloadTransferSize |= (*(pData + 24) << 16);
		pProbeCommit->dwMaxPayloadTransferSize |= (*(pData + 25) << 24);

		DbgMsg_UVCIO(("^B--UVC:[VS Commit control --SET_CUR],frmIdx=%d,payload=%d\r\n", pProbeCommit->bFrameIndex, pProbeCommit->dwMaxPayloadTransferSize));


		if (0 == pProbeCommit->bFrameIndex) {
			DBG_ERR("Wrong FrameIdx=%d,pProbeCommit=0x%lx => Set2Previous, DEF=%d\r\n", pProbeCommit->bFrameIndex, pProbeCommit, UVC_VSFMT_DEF_FRM_IDX);
			UVAC_SetImageSize(UVC_VSFMT_PREV_FRM_IDX, vidDevIdx);
		} else if (UVAC_VID_RESO_MAX_CNT >= pProbeCommit->bFrameIndex) {
			DbgMsg_UVCIO(("SET_CUR,frmIdx=%d,vidDevIdx=%d,wid=%d,height=%d\r\n", pProbeCommit->bFrameIndex, vidDevIdx, gUvcVidResoAry[pProbeCommit->bFrameIndex - 1].width, gUvcVidResoAry[pProbeCommit->bFrameIndex - 1].height));
			UVAC_SetImageSize(pProbeCommit->bFrameIndex, vidDevIdx);
		} else {
			DBG_ERR("Wrong FrameIdx=%d,vidDevIdx=%d, Stall\r\n", pProbeCommit->bFrameIndex, vidDevIdx);
			usb_setEP0Stall();
			break;
		}
		usb_setEP0Done();

		UVC_DmpProbeData(3, pProbeCommit);
		if (pProbeCommit->dwFrameInterval) {
			UVAC_SetFrameRate(10000000 / pProbeCommit->dwFrameInterval, vidDevIdx);
		} else {
			DBG_ERR("Host Set FrameInterval=0 Fail. Set to %d fps, intrf=%d\r\n", UVC_FRMRATE_30, vidDevIdx);
			UVAC_SetFrameRate(UVC_FRMRATE_30, vidDevIdx);
		}

		if (UVAC_VID_DEV_CNT_MAX > vidDevIdx) {
			if (pProbeCommit->bFormatIndex <= UVC_VSFMT_FIRSTINDEX) {
				codecType = UVAC_VIDEO_FORMAT_H264;
			} else {
				codecType = UVAC_VIDEO_FORMAT_MJPG;
			}
			UVAC_SetCodec(codecType, vidDevIdx);
		}
		break;

	case GET_CUR:
		DbgMsg_UVCIO(("^BUVC:[VS Commit control --GET_CUR]\r\n"));
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));
		UVC_GetCurProbeCommitData(pData, pProbeCommit);
		datasize = USB_UVC_PROBE_LENGTH;
		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;

	case GET_INFO:
		DbgMsg_UVCIO(("^BUVC:[VS Commit control --GET_INFO]\r\n"));
		*pData++ = 0x03;
		ControlData.w_length  = 1;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;

	case GET_LEN:
		DbgMsg_UVCIO(("^BUVC:[VS Commit control --GET_LEN]\r\n"));
		*pData++ = USB_UVC_PROBE_LENGTH;
		ControlData.w_length  = 1;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;

	case GET_MIN:
	case GET_MAX:
	case GET_RES:
	case GET_DEF:
	default:
		usb_setEP0Stall();
		usb_setEP0Done();
		DBG_ERR("^RUVC:[VS Commit control -- %d] Not imp\r\n", CS);
		break;
	}

}

static void UVC_AC_ProbeCommit_GetCur(UINT8 *pData)
{
	UINT8 *pDataAddr;
	UINT32 datasize;
	UINT32 len = sizeof(gUacSampleRate);
	UINT32 i;

	pDataAddr = pData;
	datasize = (UINT32)ControlData.w_length;
	datasize = (datasize < len) ? datasize : len;
	DbgMsg_UVC(("^GUAC--GET_CUR,gUacSampleRate=0x%x,len=%d,usblen=%d\r\n", gUacSampleRate, len, datasize));
	memset(pData, 0, datasize);
	for (i = 0; i < datasize; i++) {
		*pData++ = (gUacSampleRate >> (8 * i)) & 0xFF;
	}
	ControlData.w_length  = datasize;
	ControlData.pData    = pDataAddr;
	usb_RetSetupData();
}
static void UVC_AC_ProbeCommit_SetCur(UINT8 *pData)
{
	UINT32 len = sizeof(gUacSampleRate);
	UINT32 datasize = (UINT32)ControlData.w_length;
	UINT32 i = 0;
	UINT32 tmpSampleRate = 0;

	len = (len < datasize) ? len : datasize;
	memset(pData, 0, datasize);
	if (E_OK != usb_readEndpoint(USB_EP0, pData, &datasize)) {
		DBG_ERR("SetCur readEndpoint fail\r\n");
		tmpSampleRate = UAC_FREQUENCY_DEF;
	} else {
		tmpSampleRate = 0;
		for (i = 0; i < len; i++) {
			tmpSampleRate += (*(pData + i) << (8 * i));
		}
	}
	DbgMsg_UVCIO(("^GUAC--SET_CUR,gUacSampleRate=0x%x,len=%d,usblen=%d\r\n", tmpSampleRate, len, datasize));
	if ((tmpSampleRate > UAC_FREQUENCY_48K) || (tmpSampleRate < UAC_FREQUENCY_08K)) {
		DBG_ERR("Aud-Sample-Rate: 0x%x, prev=0x%x\r\n", tmpSampleRate, gUacSampleRate);
		usb_setEP0Stall();
		usb_setEP0Done();
	} else {
		gUacSampleRate = tmpSampleRate;
		usb_setEP0Done();
	}
#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
	{
		i = 0;
		while (i < datasize) {
			DBG_DUMP("0x%x ", *pData++);
			i++;
			if ((i % 16) == 0) {
				DBG_DUMP("\r\n");
			}
		}
		DBG_DUMP("\r\n");
	}
#endif
}
static void UVC_AC_ProbeCtrl(UINT8 CS, UINT8 *pData)
{
	UINT8 *pDataAddr;
	UINT32 datasize;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR://0x81
		DbgMsg_UVCIO(("UVC:[AC Probe control --GET_CUR]\r\n"));
		UVC_AC_ProbeCommit_GetCur(pData);
		break;
	case SET_CUR:
		DbgMsg_UVCIO(("UVC:[AC Probe control --SET_CUR]\r\n"));
		UVC_AC_ProbeCommit_SetCur(pData);
		//UVC_AudSetSampleRate(gUacSampleRate);

		DbgMsg_UVCIO(("UAC Probe SetCurr:sampleRateFormat=%d\r\n", gUacSampleRate));
		break;
	case GET_MIN:
		DbgMsg_UVC(("UVC:[AC Probe control --GET_MIN]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MIN & 0xFF;
		*pData++ = (UAC_FREQUENCY_MIN >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;

		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_MAX:
		DbgMsg_UVC(("UVC:[AC Probe control --GET_MAX]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MAX & 0xFF;
		*pData++ = (UAC_FREQUENCY_MAX >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;

		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;

	case GET_RES:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_RES]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = 0x02;
		*pData++ = 0x00;
		datasize = USB_UAC_PROBE_LENGTH;

		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	default:
		DBG_ERR("^RUVC:[AC Probe control -- %x] not support\r\n", CS);
		usb_setEP0Stall();
		usb_setEP0Done();
		break;
	}

}
static void UVC_AC_CommitCtrl(UINT8 CS, UINT8 *pData)
{
	UINT8 *pDataAddr;
	UINT32 datasize;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR://0x81
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_CUR]\r\n"));
		UVC_AC_ProbeCommit_GetCur(pData);
		break;

	case SET_CUR:
		DbgMsg_UVCIO(("^G+UVC:[AC Commit control --SET_CUR]\r\n"));
		UVC_AC_ProbeCommit_SetCur(pData);
		//UVC_AudSetSampleRate(gUacSampleRate);
		break;

	case GET_MIN:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_MIN]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MIN & 0xFF;
		*pData++ = (UAC_FREQUENCY_MIN >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;

		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_MAX:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_MAX]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MAX & 0xFF;
		*pData++ = (UAC_FREQUENCY_MAX >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;

		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_RES:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_RES]\r\n"));

		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = (gUACRes & 0xFF);
		*pData++ = (gUACRes >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;

		if (datasize > ControlData.w_length) {
			datasize = ControlData.w_length;
		}

		ControlData.w_length  = datasize;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case 4://SET_RES
		datasize = ControlData.w_length;
		DbgMsg_UVCIO(("^G+UAC:req4,s=%d\r\n", datasize));
		if (E_OK != usb_readEndpoint(USB_EP0, pData, &datasize)) {
			DBG_ERR("UAC req-4 readEndpoint fail\r\n");
		} else {
			usb_setEP0Done();
		}
		gUACRes = *pData ;
		gUACRes += *(pData + 1) << 8;
		DbgMsg_UVCIO(("^G-UAC:req4,s=%d,0x%x,0x%x\r\n", datasize, *pData, *(pData + 1)));
		break;
	default:
		DBG_ERR("^RUAC-Commit %d not support\r\n", CS);

		usb_setEP0Stall();
		usb_setEP0Done();
		break;
	}

}
#elif (UVAC_CONTROLLER == UVAC_CONTROL_U3)
static void UVC_VS_ProbeCtrl(UINT8 CS, UINT8 *pData, UVC_PROBE_COMMIT *pProbeCommit)
{
	UINT8 *pDataAddr;
	UINT32 ProbeData;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;
	UINT32 probe_len = (gU3UvacUvcVer == UVAC_UVC_VER_100)? USB_UVC_PROBE_LENGTH : USB_UVC_110_PROBE_LENGTH;

	pDataAddr = pData;
	DbgMsg_UVC(("^B+%s:0x%x, pProbeCommit=0x%x\r\n", __func__, CS, pProbeCommit));

	switch (CS) {
	case GET_CUR:
		UVC_DmpProbeData(1, pProbeCommit);
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));
		UVC_GetCurProbeCommitData(pData, pProbeCommit);

		uiRetBufAddr = (uintptr_t)pDataAddr;
		uiRetSize = probe_len;
		break;

	case SET_CUR:
		DbgMsg_UVCIO(("^B+UVC:[VS Probe control --SET_CUR]\r\n"));

		memset(pProbeCommit, 0, sizeof(UVC_PROBE_COMMIT));

		ProbeData = probe_len;
		memset(pData, 0, probe_len);
#if 0
		USBCOMPAT_READENDPOINT(USB_EP0, pData, &ProbeData);

		pProbeCommit->bFormatIndex = *(pData + 2);
		pProbeCommit->bFrameIndex = *(pData + 3);

		pProbeCommit->dwFrameInterval = *(pData + 4);
		pProbeCommit->dwFrameInterval |= (*(pData + 5) << 8);
		pProbeCommit->dwFrameInterval |= (*(pData + 6) << 16);
		pProbeCommit->dwFrameInterval |= (*(pData + 7) << 24);

		pProbeCommit->dwMaxVideoFrameSize = UVC_VSFRM_MAX_FBSIZE;
		pProbeCommit->dwMaxPayloadTransferSize = UVC_ISOIN_HS_PACKET_SIZE * UVC_ISOIN_BANDWIDTH;

		UVC_DmpProbeData(2, pProbeCommit);

		DbgMsg_UVCIO(("UVC: Format index =%d Frame index =%d  Interval=0x%x \r\n",
					  pProbeCommit->bFormatIndex, pProbeCommit->bFrameIndex, pProbeCommit->dwFrameInterval));
#else
		m_pProbeCommit = pProbeCommit;
		m_RequestType = UVAC_CLASS_REQ_VS_PROBE;
		m_RequestLen = ProbeData;
		uiRetBufAddr = (uintptr_t) pData;
		uiRetSize = m_RequestLen;
#endif
		break;

	case GET_INFO:
		DbgMsg_UVCIO(("^BUVC:[VS Probe control --GET_INFO]\r\n"));
		*pData++ = 0x03;
		uiRetBufAddr = (uintptr_t) pDataAddr;
		uiRetSize = 1;
		break;

	case GET_MIN:
		DbgMsg_UVCIO(("^BUVC:[VS Probe control --GET_MIN]\r\n"));
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));

		pData += 2;
		*pData++ = 1;
		*pData++ = 1;

		*pData++ = UVC_VSFRM_FRAMEINTERVAL_MIN & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_MIN >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_MIN >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_MIN >> 24) & 0xFF;

		pData += 10;
		*pData++ = pProbeCommit->dwMaxVideoFrameSize & 0xFF;
		*pData++ = (pProbeCommit->dwMaxVideoFrameSize >> 8) & 0xFF;
		*pData++ = (pProbeCommit->dwMaxVideoFrameSize >> 16) & 0xFF;
		*pData++ = (pProbeCommit->dwMaxVideoFrameSize >> 24) & 0xFF;

		*pData++ = (pProbeCommit->dwMaxPayloadTransferSize) & 0xFF;
		*pData++ = (pProbeCommit->dwMaxPayloadTransferSize >> 8) & 0xFF;
		*pData++ = (pProbeCommit->dwMaxPayloadTransferSize >> 16) & 0xFF;
		*pData++ = (pProbeCommit->dwMaxPayloadTransferSize >> 24) & 0xFF;

		if (gU3UvacUvcVer >= UVAC_UVC_VER_110) {
			*pData++= (pProbeCommit->dwClockFrequency) & 0xFF;
			*pData++= (pProbeCommit->dwClockFrequency>>8) & 0xFF;
			*pData++= (pProbeCommit->dwClockFrequency>>16) & 0xFF;
			*pData++= (pProbeCommit->dwClockFrequency>>24) & 0xFF;
			*pData++= pProbeCommit->bmFramingInfo;
			*pData++= pProbeCommit->bPreferredVersion;
			*pData++= pProbeCommit->bMinVersion;
			*pData++= pProbeCommit->bMaxVersion;
		}

		if (gU3UvacUvcVer == UVAC_UVC_VER_150) {
			pProbeCommit->bUsage = 0;
			pProbeCommit->bBitDepthLuma = 0;
			pProbeCommit->bmSettings = 0;
			pProbeCommit->bMaxNumberOfRefFramesPlus1 = 0;
			pProbeCommit->bmRateControlModes = 0;
			pProbeCommit->bmLayoutPerStream[0] = 0;
			pProbeCommit->bmLayoutPerStream[1] = 0;
			pProbeCommit->bmLayoutPerStream[2] = 0;
			pProbeCommit->bmLayoutPerStream[3] = 0;
		}

		uiRetBufAddr = (uintptr_t) pDataAddr;
		uiRetSize = probe_len;
		break;

	case GET_MAX:
		DbgMsg_UVCIO(("^B+UVC:[VS Probe control --GET_MAX]\r\n"));
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));
		UVC_GetCurProbeCommitData(pData, pProbeCommit);

		uiRetBufAddr = (uintptr_t) pDataAddr;
		uiRetSize = probe_len;

		break;

	case GET_RES:
		bStall = TRUE;
		DBG_ERR("^RUVC:[VS Probe control --GET_RES] Not imp\r\n");
		break;

	case GET_LEN:
		DbgMsg_UVCIO(("^BUVC:[VS Probe control --GET_LEN]\r\n"));
		*pData++ = USB_UVC_PROBE_LENGTH;

		uiRetBufAddr = (uintptr_t) pDataAddr;
		uiRetSize = 1;
		break;

	case GET_DEF:
		DbgMsg_UVCIO(("^BUVC:[VS Probe control --GET_DEF]\r\n"));
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));
		pData += 2;
		*pData++ = 1;
		*pData++ = 1;

		*pData++ = UVC_VSFRM_FRAMEINTERVAL_DEF & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_DEF >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_DEF >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_FRAMEINTERVAL_DEF >> 24) & 0xFF;

		pData += 10;
		*pData++ = UVC_VSFRM_MAX_FBSIZE & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 24) & 0xFF;

		*pData++ = (UVC_VSFRM_MAX_FBSIZE + 2) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 8) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 16) & 0xFF;
		*pData++ = (UVC_VSFRM_MAX_FBSIZE >> 24) & 0xFF;

		uiRetBufAddr = (uintptr_t) pDataAddr;
		uiRetSize = probe_len;
		break;

	default:
		DBG_ERR("^RUVC:[VS Probe control -- %d] not support\r\n", CS);
		break;
	}
	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
	DbgMsg_UVCIO(("^B-%s:0x%x\r\n", __func__, CS));
}


/**
    UVC VideoStreaming Control -- Commit

    @param UINT8 CS       Control attribute
    @param UINT8 *pData   Data pointer
    @return void
*/

static void UVC_VS_CommitCtrl(UINT8 CS, UINT8 *pData, UVC_PROBE_COMMIT *pProbeCommit, UINT32 vidDevIdx)
{
	UINT8 *pDataAddr;
	//UINT32 len;
	BOOL bStall = FALSE;
	UINT32 uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;
	UINT32 probe_len = (gU3UvacUvcVer == UVAC_UVC_VER_100)? USB_UVC_PROBE_LENGTH : USB_UVC_110_PROBE_LENGTH;

	//UVAC_VIDEO_FORMAT codecType = UVAC_VIDEO_FORMAT_H264;

	pDataAddr = pData;
	DbgMsg_UVC(("^B%s:0x%x, vidDevIdx=%d\r\n", __func__, CS, vidDevIdx));
	switch (CS) {
	case SET_CUR:
		DbgMsg_UVCIO(("^B-UVC:[VS Commit control --SET_CUR],Prv frmIdx=%d\r\n", pProbeCommit->bFrameIndex));
#if 0
		len = USB_UVC_PROBE_LENGTH;
		if (E_OK != USBCOMPAT_READENDPOINT(USB_EP0, pData, &len)) {
			DBG_ERR("Commit SetCur readEndpoint fail\r\n");
		}
		pProbeCommit->bFormatIndex = *(pData + 2);
		pProbeCommit->bFrameIndex = *(pData + 3);
		pProbeCommit->dwFrameInterval = *(pData + 4);
		pProbeCommit->dwFrameInterval |= *(pData + 5) << 8;
		pProbeCommit->dwFrameInterval |= *(pData + 6) << 16;
		pProbeCommit->dwFrameInterval |= *(pData + 7) << 24;

		pProbeCommit->wKeyFrameRate = *(pData + 8);
		pProbeCommit->wKeyFrameRate |= (*(pData + 9) << 8);
		pProbeCommit->wPFrameRate = *(pData + 10);
		pProbeCommit->wPFrameRate |= (*(pData + 11) << 8);
		pProbeCommit->wCompQuality = *(pData + 12);
		pProbeCommit->wCompQuality |= (*(pData + 13) << 8);
		pProbeCommit->wCompWindowSize = *(pData + 14);
		pProbeCommit->wCompWindowSize |= (*(pData + 15) << 8);
		pProbeCommit->wDelay = *(pData + 16);
		pProbeCommit->wDelay |= (*(pData + 17) << 8);
		pProbeCommit->dwMaxVideoFrameSize = *(pData + 18);
		pProbeCommit->dwMaxVideoFrameSize |= (*(pData + 19) << 8);
		pProbeCommit->dwMaxVideoFrameSize |= (*(pData + 20) << 16);
		pProbeCommit->dwMaxVideoFrameSize |= (*(pData + 21) << 24);
		pProbeCommit->dwMaxPayloadTransferSize = *(pData + 22);
		pProbeCommit->dwMaxPayloadTransferSize |= (*(pData + 23) << 8);
		pProbeCommit->dwMaxPayloadTransferSize |= (*(pData + 24) << 16);
		pProbeCommit->dwMaxPayloadTransferSize |= (*(pData + 25) << 24);

		DbgMsg_UVCIO(("^B--UVC:[VS Commit control --SET_CUR],frmIdx=%d,payload=%d\r\n", pProbeCommit->bFrameIndex, pProbeCommit->dwMaxPayloadTransferSize));


		if (0 == pProbeCommit->bFrameIndex) {
			DBG_ERR("Wrong FrameIdx=%d,pProbeCommit=0x%x => Set2Previous, DEF=%d\r\n", pProbeCommit->bFrameIndex, pProbeCommit, UVC_VSFMT_DEF_FRM_IDX);
			UVAC_SetImageSize(UVC_VSFMT_PREV_FRM_IDX, vidDevIdx);
		} else if (UVAC_VID_RESO_MAX_CNT >= pProbeCommit->bFrameIndex) {
			DbgMsg_UVCIO(("SET_CUR,frmIdx=%d,vidDevIdx=%d,wid=%d,height=%d\r\n", pProbeCommit->bFrameIndex, vidDevIdx, gUvcVidResoAry[pProbeCommit->bFrameIndex - 1].width, gUvcVidResoAry[pProbeCommit->bFrameIndex - 1].height));
			UVAC_SetImageSize(pProbeCommit->bFrameIndex, vidDevIdx);
		} else {
			DBG_ERR("Wrong FrameIdx=%d,vidDevIdx=%d, Stall\r\n", pProbeCommit->bFrameIndex, vidDevIdx);
			bStall = TRUE;
			break;
		}

		UVC_DmpProbeData(3, pProbeCommit);
		if (pProbeCommit->dwFrameInterval) {
			UVAC_SetFrameRate(10000000 / pProbeCommit->dwFrameInterval, vidDevIdx);
		} else {
			DBG_ERR("Host Set FrameInterval=0 Fail. Set to %d fps, intrf=%d\r\n", UVC_FRMRATE_30, vidDevIdx);
			UVAC_SetFrameRate(UVC_FRMRATE_30, vidDevIdx);
		}

		if (UVAC_VID_DEV_CNT_MAX > vidDevIdx) {
			if (pProbeCommit->bFormatIndex <= UVC_VSFMT_FIRSTINDEX) {
				codecType = UVAC_VIDEO_FORMAT_H264;
			} else {
				codecType = UVAC_VIDEO_FORMAT_MJPG;
			}
			UVAC_SetCodec(codecType, vidDevIdx);
		}
#else
		m_pProbeCommit = pProbeCommit;
		m_RequestType = UVAC_CLASS_REQ_VS_COMMIT;
		m_RequestLen = probe_len;
		uiRetBufAddr = (uintptr_t) pData;
		uiRetSize = m_RequestLen;
#endif
		break;

	case GET_CUR:
		DbgMsg_UVCIO(("^BUVC:[VS Commit control --GET_CUR]\r\n"));
		memset(pData, 0, sizeof(UVC_PROBE_COMMIT));
		UVC_GetCurProbeCommitData(pData, pProbeCommit);
		uiRetBufAddr = (uintptr_t) pDataAddr;
		uiRetSize = probe_len;
		break;

	case GET_INFO:
		DbgMsg_UVCIO(("^BUVC:[VS Commit control --GET_INFO]\r\n"));
		*pData++ = 0x03;
		uiRetBufAddr = (uintptr_t) pDataAddr;
		uiRetSize = 1;
		break;

	case GET_LEN:
		DbgMsg_UVCIO(("^BUVC:[VS Commit control --GET_LEN]\r\n"));
		*pData++ = probe_len;
		uiRetBufAddr = (uintptr_t) pDataAddr;
		uiRetSize = 1;
		break;

	case GET_MIN:
	case GET_MAX:
	case GET_RES:
	case GET_DEF:
	default:
		bStall = TRUE;
		DBG_ERR("^RUVC:[VS Commit control -- %d] Not imp\r\n", CS);
		break;
	}
	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);

}

static void UVC_AC_ProbeCommit_GetCur(UINT8 *pData, UINT16 datalen)
{
	UINT8 *pDataAddr;
	UINT32 datasize;
	UINT32 len = sizeof(gUacSampleRate);
	UINT32 i;

	pDataAddr = pData;
	datasize = (UINT32)datalen;
	datasize = (datasize < len) ? datasize : len;
	DbgMsg_UVC(("^GUAC--GET_CUR,gUacSampleRate=0x%x,len=%d,usblen=%d\r\n", gUacSampleRate, len, datasize));
	memset(pData, 0, datasize);
	for (i = 0; i < datasize; i++) {
		*pData++ = (gUacSampleRate >> (8 * i)) & 0xFF;
	}

	usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, datasize);
}
static void UVC_AC_ProbeCommit_SetCur(UINT8 *pData, UINT16 datalen)
{
	//UINT32 len = sizeof(gUacSampleRate);
	UINT32 datasize = (UINT32)datalen;
	//UINT32 i = 0;


	//len = (len < datasize) ? len : datasize;
	memset(pData, 0, datasize);
#if 0
	if (E_OK != USBCOMPAT_READENDPOINT(USB_EP0, pData, &datasize)) {
		DBG_ERR("SetCur readEndpoint fail\r\n");
		tmpSampleRate = UAC_FREQUENCY_DEF;
	} else {
		tmpSampleRate = 0;
		for (i = 0; i < len; i++) {
			tmpSampleRate += (*(pData + i) << (8 * i));
		}
	}
	DbgMsg_UVCIO(("^GUAC--SET_CUR,gUacSampleRate=0x%x,len=%d,usblen=%d\r\n", tmpSampleRate, len, datasize));
	if ((tmpSampleRate > UAC_FREQUENCY_48K) || (tmpSampleRate < UAC_FREQUENCY_08K)) {
		DBG_ERR("Aud-Sample-Rate: 0x%x, prev=0x%x\r\n", tmpSampleRate, gUacSampleRate);
		usb3dev_handleSetupResult(TRUE, 0, 0);
	} else {
		gUacSampleRate = tmpSampleRate;
		usb3dev_handleSetupResult(FALSE, 0, 0);
	}
#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
	{
		i = 0;
		while (i < datasize) {
			DBG_DUMP("0x%x ", *pData++);
			i++;
			if ((i % 16) == 0) {
				DBG_DUMP("\r\n");
			}
		}
		DBG_DUMP("\r\n");
	}
#endif
#else
	m_RequestType = UVAC_CLASS_REQ_AC_COMMIT_SET_CUR;
	m_RequestLen = datasize;
	usb3dev_handleSetupResult(FALSE, (uintptr_t) pData, m_RequestLen);
#endif
}
static void UVC_AC_ProbeCtrl(UINT8 CS, UINT8 *pData, UINT16 datalen)
{
	UINT8 *pDataAddr;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR://0x81
		DbgMsg_UVCIO(("UVC:[AC Probe control --GET_CUR]\r\n"));
		UVC_AC_ProbeCommit_GetCur(pData, datalen);
		break;
	case SET_CUR:
		DbgMsg_UVCIO(("UVC:[AC Probe control --SET_CUR]\r\n"));
		UVC_AC_ProbeCommit_SetCur(pData, datalen);
		//UVC_AudSetSampleRate(gUacSampleRate);

		DbgMsg_UVCIO(("UAC Probe SetCurr:sampleRateFormat=%d\r\n", gUacSampleRate));
		break;
	case GET_MIN:
		DbgMsg_UVC(("UVC:[AC Probe control --GET_MIN]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MIN & 0xFF;
		*pData++ = (UAC_FREQUENCY_MIN >> 8) & 0xFF;

		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, USB_UAC_PROBE_LENGTH);
		break;
	case GET_MAX:
		DbgMsg_UVC(("UVC:[AC Probe control --GET_MAX]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MAX & 0xFF;
		*pData++ = (UAC_FREQUENCY_MAX >> 8) & 0xFF;

		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, USB_UAC_PROBE_LENGTH);
		break;

	case GET_RES:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_RES]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = 0x02;
		*pData++ = 0x00;

		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, USB_UAC_PROBE_LENGTH);
		break;
	default:
		DBG_ERR("^RUVC:[AC Probe control -- %x] not support\r\n", CS);
		usb3dev_handleSetupResult(TRUE, 0, 0);
		break;
	}

}
static void UVC_AC_CommitCtrl(UINT8 CS, UINT8 *pData, UINT16 datalen)
{
	UINT8 *pDataAddr;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR://0x81
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_CUR]\r\n"));
		UVC_AC_ProbeCommit_GetCur(pData, datalen);
		break;

	case SET_CUR:
		DbgMsg_UVCIO(("^G+UVC:[AC Commit control --SET_CUR]\r\n"));
		UVC_AC_ProbeCommit_SetCur(pData, datalen);
		//UVC_AudSetSampleRate(gUacSampleRate);
		break;

	case GET_MIN:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_MIN]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MIN & 0xFF;
		*pData++ = (UAC_FREQUENCY_MIN >> 8) & 0xFF;

		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, USB_UAC_PROBE_LENGTH);
		break;
	case GET_MAX:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_MAX]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MAX & 0xFF;
		*pData++ = (UAC_FREQUENCY_MAX >> 8) & 0xFF;

		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, USB_UAC_PROBE_LENGTH);
		break;
	case GET_RES:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_RES]\r\n"));

		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = (gUACRes & 0xFF);
		*pData++ = (gUACRes >> 8) & 0xFF;

		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, USB_UAC_PROBE_LENGTH);
		break;
	case 4://SET_RES
		//DbgMsg_UVCIO(("^G+UAC:req4,s=%d\r\n", ControlData.w_length));
#if 0
		if (E_OK != USBCOMPAT_READENDPOINT(USB_EP0, pData, &datasize)) {
			DBG_ERR("UAC req-4 readEndpoint fail\r\n");
		} else {
			usb3dev_handleSetupResult(FALSE, 0, 0);
		}
		gUACRes = *pData ;
		gUACRes += *(pData + 1) << 8;
#else
		m_RequestType = UVAC_CLASS_REQ_AC_COMMIT_SET_RES;
		m_RequestLen = datalen;
		usb3dev_handleSetupResult(FALSE, (uintptr_t) pData, m_RequestLen);
#endif
		//DbgMsg_UVCIO(("^G-UAC:req4,s=%d,0x%x,0x%x\r\n", ControlData.w_length, *pData, *(pData + 1)));
		break;
	default:
		DBG_ERR("^RUAC-Commit %d not support\r\n", CS);
		usb3dev_handleSetupResult(TRUE, 0, 0);
		break;
	}

}

static void UVC_AC_EP_Freq_Ctrl(UINT8 CS, UINT8 *pData)
{
	UINT32 uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;
	BOOL bStall = FALSE;

	DBG_IND("^G%s:0x%x\r\n", __func__, CS);

	switch (CS) {
	case SET_CUR: {
			#if 0
			//UINT32 freq = 0;

			DBG_IND("^BUAC:[AC Freq control --SET_CUR]\r\n");

			datasize = 3;
			memset(pData, 0, datasize);
			if (E_OK != _readEP0(pData, &datasize)) {
				DBG_ERR("SetCur readEndpoint fail\r\n");
			} else {
				//freq = *pData + (*(pData + 1) << 8) + (*(pData + 2) << 8);
			}

			//DBG_IND("freq = %d\r\n", freq);
			usb_setEP0Done();
			#else

			m_RequestType = UVAC_CLASS_REQ_AC_FREQ_SET_CUR;
			m_RequestLen = 3;
			uiRetBufAddr = (uintptr_t) pData;
			uiRetSize = m_RequestLen;
			#endif
			break;
		}
	case GET_CUR:
	case GET_MIN:
	case GET_MAX:
	case GET_RES:

	default:
		DBG_ERR("^RUAC:[AC Freq control -- %x] not support\r\n", CS);
		bStall = TRUE;
		break;
	}

	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
}

static void UVC_ACO_ProbeCtrl(UINT8 CS, UINT8 *pData, UINT16 datalen)
{
	UINT8 *pDataAddr;
	UINT32 datasize;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR://0x81
		DbgMsg_UVCIO(("UVC:[AC Probe control --GET_CUR]\r\n"));
		UVC_AC_ProbeCommit_GetCur(pData, datalen);
		break;
	case SET_CUR:
		DbgMsg_UVCIO(("UVC:[AC Probe control --SET_CUR]\r\n"));
		UVC_AC_ProbeCommit_SetCur(pData, datalen);
		//UVC_AudSetSampleRate(gU2UacSampleRate);

		DbgMsg_UVCIO(("UAC Probe SetCurr:sampleRateFormat=%d\r\n", gUacRxSampleRate));
		break;
	case GET_MIN:
		DbgMsg_UVC(("UVC:[AC Probe control --GET_MIN]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MIN & 0xFF;
		*pData++ = (UAC_FREQUENCY_MIN >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;
		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, datasize);
		break;
	case GET_MAX:
		DbgMsg_UVC(("UVC:[AC Probe control --GET_MAX]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MAX & 0xFF;
		*pData++ = (UAC_FREQUENCY_MAX >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;
		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, datasize);
		break;

	case GET_RES:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_RES]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = 0x02;
		*pData++ = 0x00;
		datasize = USB_UAC_PROBE_LENGTH;
		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, datasize);
		break;
	default:
		DBG_ERR("^RUVC:[AC Probe control -- %x] not support\r\n", CS);
		usb3dev_handleSetupResult(TRUE, 0, 0);
		break;
	}
}
static void UVC_ACO_CommitCtrl(UINT8 CS, UINT8 *pData, UINT16 datalen)
{
	UINT8 *pDataAddr;
	UINT32 datasize;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR://0x81
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_CUR]\r\n"));
		UVC_AC_ProbeCommit_GetCur(pData, datalen);
		break;

	case SET_CUR:
		DbgMsg_UVCIO(("^G+UVC:[AC Commit control --SET_CUR]\r\n"));
		UVC_AC_ProbeCommit_SetCur(pData, datalen);
		//UVC_AudSetSampleRate(gU2UacSampleRate);
		break;

	case GET_MIN:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_MIN]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MIN & 0xFF;
		*pData++ = (UAC_FREQUENCY_MIN >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;
		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, datasize);
		break;
	case GET_MAX:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_MAX]\r\n"));
		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = UAC_FREQUENCY_MAX & 0xFF;
		*pData++ = (UAC_FREQUENCY_MAX >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;
		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, datasize);
		break;
	case GET_RES:
		DbgMsg_UVCIO(("UVC:[AC Commit control --GET_RES]\r\n"));

		memset(pData, 0, USB_UAC_PROBE_LENGTH);
		*pData++ = (gUACRes & 0xFF);
		*pData++ = (gUACRes >> 8) & 0xFF;
		datasize = USB_UAC_PROBE_LENGTH;
		usb3dev_handleSetupResult(FALSE, (uintptr_t) pDataAddr, datasize);
		break;
	case 4://SET_RES
		m_RequestType = UVAC_CLASS_REQ_AC_COMMIT_SET_RES;
		m_RequestLen = datalen;
		usb3dev_handleSetupResult(FALSE, (uintptr_t) pData, m_RequestLen);
		break;
	default:
		DBG_ERR("^RUAC-Commit %d not support\r\n", CS);

		usb3dev_handleSetupResult(TRUE, 0, 0);
		break;
	}

}

static void UVC_ACO_MuteCtrl(UINT8 CS, UINT8 *pData, UINT32 datalen)
{
	UINT8 *pDataAddr;
	UINT32 datasize;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR: { //0x81
			DbgMsg_UVCIO(("^BUAC:[AS Mute control --GET_CUR]\r\n"));

			datasize = datalen;
			memset(pData, 0, datasize);
			*pData = gUacRxMute;

			uiRetSize    = datasize;
			uiRetBufAddr = (uintptr_t)pDataAddr;
			usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			break;
		}
	case SET_CUR: {
			DbgMsg_UVCIO(("^BUAC:[AS Mute control --SET_CUR]\r\n"));

			m_RequestType = UVAC_CLASS_REQ_ACO_MUTE;
			m_RequestLen = datalen;
			usb3dev_handleSetupResult(FALSE, (uintptr_t) pData, m_RequestLen);
			break;
		}
	case GET_MIN:
	case GET_MAX:
	case GET_RES:
	default:
		DBG_ERR("^RUAC:[AC MuteCtrl -- %x] not support\r\n", CS);
		usb3dev_handleSetupResult(TRUE, 0, 0);
		break;
	}
}

static void UVC_ACO_VolCtrl(UINT8 CS, UINT8 *pData, UINT32 datalen)
{
	UINT8 *pDataAddr;
	UINT32 datasize;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR: { //0x81
			DbgMsg_UVCIO(("^BUAC:[AS Vol control --GET_CUR]\r\n"));

			datasize = 2;
			memset(pData, 0, datasize);

			*pData++ = u3_uac_rx_vol_info.vol_def & 0xFF;
			*pData++ = (u3_uac_rx_vol_info.vol_def >> 8) & 0xFF;

			uiRetSize    = datasize;
			uiRetBufAddr = (uintptr_t)pDataAddr;
			usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);

			break;
		}
	case SET_CUR: {
			DbgMsg_UVCIO(("^BUAC:[AS Vol control --SET_CUR]\r\n"));
			m_RequestType = UVAC_CLASS_REQ_ACO_VOL;
			m_RequestLen = datalen;
			usb3dev_handleSetupResult(FALSE, (uintptr_t) pData, m_RequestLen);

			break;
		}
	case GET_MIN:
	case GET_MAX:
	case GET_RES: {
			UINT32 tmpsetting;

			if (CS == GET_RES) {
				tmpsetting = u3_uac_rx_vol_info.vol_res;
			} else if (CS == GET_MAX) {
				tmpsetting = u3_uac_rx_vol_info.vol_max;
			} else {
				tmpsetting = u3_uac_rx_vol_info.vol_min;
			}

			datasize = datalen;
			memset(pData, 0, datasize);

			*pData++ = tmpsetting & 0xFF;
			*pData++ = (tmpsetting >> 8) & 0xFF;

			uiRetSize    = datasize;
			uiRetBufAddr = (uintptr_t)pDataAddr;
			usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			break;
		}
	default:
		DBG_ERR("^RUAC:[AC VolCtrl -- %x] not support\r\n", CS);
		usb3dev_handleSetupResult(TRUE, 0, 0);
		break;
	}
}

static void UVC_ACO_EP_Freq_Ctrl(UINT8 CS, UINT8 *pData)
{
	UINT8 *pDataAddr;
	UINT32 datasize;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	pDataAddr = pData;
	DBG_IND("^G%s:0x%x\r\n", __func__, CS);

	switch (CS) {
	case SET_CUR: {
			#if 0
			//UINT32 freq = 0;

			DBG_IND("^BUAC:[AC Freq control --SET_CUR]\r\n");

			datasize = 3;
			memset(pData, 0, datasize);
			if (E_OK != _readEP0(pData, &datasize)) {
				DBG_ERR("SetCur readEndpoint fail\r\n");
			} else {
				//freq = *pData + (*(pData + 1) << 8) + (*(pData + 2) << 8);
			}

			//DBG_IND("freq = %d\r\n", freq);
			usb_setEP0Done();
			#else

			m_RequestType = UVAC_CLASS_REQ_ACO_FREQ_SET_CUR;
			m_RequestLen = 3;
			uiRetBufAddr = (uintptr_t) pData;
			uiRetSize = m_RequestLen;
			#endif
			break;
		}
	case GET_CUR:
			DBG_IND("^BUAC:[ACO_EP Freq control --GET_CUR]\r\n");

			datasize = 3;
			memset(pData, 0, datasize);

			*pData++ = gUvacAudRxSampleRate[0] & 0xFF;
			*pData++ = (gUvacAudRxSampleRate[0] >> 8) & 0xFF;
			*pData++ = (gUvacAudRxSampleRate[0] >> 16) & 0xFF;

			uiRetSize    = datasize;
			uiRetBufAddr = (uintptr_t)pDataAddr;
			usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			break;
	case GET_MIN:
	case GET_MAX:
	case GET_RES:

	default:
		DBG_ERR("^RUAC:[AC Freq control -- %x] not support\r\n", CS);
		bStall = TRUE;
		break;
	}

	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
}


#endif

#if 1//(ENABLE == UVC_METHOD3)
void UVC_DmpStillProbeCommitData(PUVC_STILL_PROBE_COMMIT pStilProbComm)
{
#if (_UVC_DBG_LVL_ >= _UVC_DBG_CHK_)
	DBG_DUMP("+%s\r\n", __func__);
	DBG_DUMP("bFormatIndex=0x%x\r\n",                      pStilProbComm->bFormatIndex);
	DBG_DUMP("bFrameIndex=0x%x\r\n",                       pStilProbComm->bFrameIndex);
	DBG_DUMP("bCompressionIndex=0x%x\r\n",                 pStilProbComm->bCompressionIndex);
	DBG_DUMP("dwMaxVideoFrameSize=0x%x\r\n",               pStilProbComm->dwMaxVideoFrameSize);
	DBG_DUMP("dwMaxPayloadTransferSize=0x%x\r\n",          pStilProbComm->dwMaxPayloadTransferSize);
	DBG_DUMP("-%s\r\n", __func__);
#endif
}
void UVC_InitStillProbeCommitData(PUVC_STILL_PROBE_COMMIT pStilProbComm)
{
	pStilProbComm->bFormatIndex = UVC_VSFMT_FIRSTINDEX; //video format index;ex:1,2,...
	pStilProbComm->bFrameIndex = UVC_VS_STILL_DEFAULT_FRM_IDX;
	pStilProbComm->bCompressionIndex = 0; //no compression support now
	pStilProbComm->dwMaxVideoFrameSize = UVC_VS_STILL_FRM_MAX_SIZE;
	//if (0) {
	if (g_usb_speed == U3DEV_SPEED_SS) {
		pStilProbComm->dwMaxPayloadTransferSize = USB_SS_MAX_TXF_SIZE_ONE_PACKET;
	} else {
		pStilProbComm->dwMaxPayloadTransferSize = USB_MAX_TXF_SIZE_ONE_PACKET;
	}
}
#if 0
static void UVC_GetCurStillProbeCommitData(UINT8 *pData)
{
	*pData++ = gUvcStillProbeCommit.bFormatIndex;
	*pData++ = gUvcStillProbeCommit.bFrameIndex;
	*pData++ = gUvcStillProbeCommit.bCompressionIndex;

	*pData++ = gUvcStillProbeCommit.dwMaxVideoFrameSize & 0xFF;
	*pData++ = (gUvcStillProbeCommit.dwMaxVideoFrameSize >> 8) & 0xFF;
	*pData++ = (gUvcStillProbeCommit.dwMaxVideoFrameSize >> 16) & 0xFF;
	*pData++ = (gUvcStillProbeCommit.dwMaxVideoFrameSize >> 24) & 0xFF;

#if 1
	if (usb3dev_getDevInfo(U3DEV_DEVINFO_ID_SPEED) == U3DEV_SPEED_SS) {
		*pData++ = (USB_SS_MAX_TXF_SIZE_ONE_PACKET) & 0xFF;
		*pData++ = (USB_SS_MAX_TXF_SIZE_ONE_PACKET >> 8) & 0xFF;
		*pData++ = (USB_SS_MAX_TXF_SIZE_ONE_PACKET >> 16) & 0xFF;
		*pData++ = (USB_SS_MAX_TXF_SIZE_ONE_PACKET >> 24) & 0xFF;

	} else {
		*pData++ = (USB_MAX_TXF_SIZE_ONE_PACKET) & 0xFF;
		*pData++ = (USB_MAX_TXF_SIZE_ONE_PACKET >> 8) & 0xFF;
		*pData++ = (USB_MAX_TXF_SIZE_ONE_PACKET >> 16) & 0xFF;
		*pData++ = (USB_MAX_TXF_SIZE_ONE_PACKET >> 24) & 0xFF;
	}
#else
	*pData++ = (gUvcStillProbeCommit.dwMaxPayloadTransferSize) & 0xFF;
	*pData++ = (gUvcStillProbeCommit.dwMaxPayloadTransferSize >> 8) & 0xFF;
	*pData++ = (gUvcStillProbeCommit.dwMaxPayloadTransferSize >> 16) & 0xFF;
	*pData++ = (gUvcStillProbeCommit.dwMaxPayloadTransferSize >> 24) & 0xFF;
#endif
}
#endif
BOOL UVC_TrigStilImg(uintptr_t dataAddr, UINT32 dataSize)
{
	UVAC_TXF_INFO   txfInfo;
	memset((void *) &txfInfo, 0, sizeof(UVAC_TXF_INFO));
	DbgMsg_UVC(("%s:addr=0x%x,Size=0x%x,sts=%d,dmaAbort=%d\r\n", __func__, dataAddr, dataSize, gUvacStillImgTrigSts, gUvcUsbDMAAbord));
	if (FALSE == gUvcUsbDMAAbord) {
		txfInfo.sAddr = txfInfo.oriAddr = dataAddr;
		txfInfo.size = dataSize;
		txfInfo.usbEP = (UINT8)UVAC_USB_EP[UVAC_TXF_QUE_IMG];
		txfInfo.txfCnt = 0;
		txfInfo.streamType = UVAC_TXF_STREAM_STILLIMG;
		UVAC_AddIsoInTxfInfo(&txfInfo, UVAC_TXF_QUE_IMG);
		//set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_TXF);
		return TRUE;
	} else {
		DBG_ERR("DMA-Abort,Can not trigger still image\r\n");
#if 0
		DbgMsg_UVC(("^MDMA-Abort but try to bulk-in image\r\n"));
		txfInfo.sAddr = txfInfo.oriAddr = dataAddr;
		txfInfo.size = dataSize;
		//txfInfo.usbEP = (UINT8)UVAC_USB_EP[UVAC_TXF_QUE_IMG];
		txfInfo.usbEP = (UINT8)UVAC_USB_EP[UVAC_TXF_QUE_V1];
		txfInfo.txfCnt = 0;
		txfInfo.streamType = UVAC_TXF_STREAM_STILLIMG;
		//UVAC_AddIsoInTxfInfo(&txfInfo, UVAC_TXF_QUE_IMG);
		UVAC_AddIsoInTxfInfo(&txfInfo, UVAC_TXF_QUE_V1);
		gUvcUsbDMAAbord = FALSE;
		set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_TXF);
#endif
		return FALSE;
	}
}
#if 0
static void UVC_CapStilImgUploadTest(UINT8 *pData)
{
	DbgMsg_UVC(("^G%s TrigSts=%d\r\n", __func__, gUvacStillImgTrigSts));
#if 0
	gUvacStillImgTrigSts = UVC_STILLIMG_TRIG_CTRL_TRANSMIT_BULK;//2
	//UVC_TrigStilImg(gUvcCapImgAddr, gUvcCapImgSize);
#endif
}
static void UVC_CapStilImgUpload(UINT8 *pData)
{
	FLGPTN uiFlag;
	DbgMsg_UVC(("^G%s TrigSts=%d\r\n", __func__, gUvacStillImgTrigSts));
#if 0
	uiFlag = kchk_flg(FLG_ID_UVAC, FLGUVAC_START);
#else
	uiFlag = FLGUVAC_START;
#endif
	if (uiFlag & FLGUVAC_START) {
		//1. Start IPL flow to capture a full-size image => another task is needed
		//2. divide the image into several
		//3. set a flag to txf
		gUvacStillImgTrigSts = *pData; //UVC_STILLIMG_TRIG_CTRL_TRANSMIT_BULK;//2
		//gUvacStillImgTrigSts = UVC_STILLIMG_TRIG_CTRL_TRANSMIT_BULK;//2
#if 1//(ENABLE == UVC_METHOD3_CMDTEST)
		UVC_TrigStilImg(gUvcCapImgAddr, gUvcCapImgSize);
#endif
	}
}
#endif
static void UVC_VS_Still_ProbeCtrl(UINT8 req, UINT8 *pData, UINT8 intrf)
{
#if 0
	UINT8 *pDataAddr = pData;
	UINT32 len = 0;

	DbgMsg_UVCIO(("^B+%s:req=%d,pDataAddr=0x%x, size=0x%x, intrf=%d, ctrl-len=%d\r\n", __func__, req, pData, len, intrf, ControlData.w_length));
	switch (req) {
	case SET_CUR:
		DbgMsg_UVCIO(("^B+[Still_ProbeCtrl--SET_CUR],len=%d,gUvacStillImgTrigSts%d\r\n", len, gUvacStillImgTrigSts));
		len = UVC_STILL_PROBE_COMMIT_LEN;
		if (E_OK != usb_readEndpoint(USB_EP0, pData, &len)) {
			DBG_ERR("SetCur readEndpoint fail\r\n");
		}
		usb_setEP0Done();
		gUvcStillProbeCommit.bFormatIndex = *pData;
		gUvcStillProbeCommit.bFrameIndex = *(pData + 1);
		gUvcStillProbeCommit.bCompressionIndex = *(pData + 2);
		gUvcStillProbeCommit.dwMaxVideoFrameSize = *(pData + 3);
		gUvcStillProbeCommit.dwMaxVideoFrameSize |= (*(pData + 4) << 8);
		gUvcStillProbeCommit.dwMaxVideoFrameSize |= (*(pData + 5) << 16);
		gUvcStillProbeCommit.dwMaxVideoFrameSize |= (*(pData + 6) << 24);
		gUvcStillProbeCommit.dwMaxPayloadTransferSize = *(pData + 7);
		gUvcStillProbeCommit.dwMaxPayloadTransferSize |= (*(pData + 8) << 8);
		gUvcStillProbeCommit.dwMaxPayloadTransferSize |= (*(pData + 9) << 16);
		gUvcStillProbeCommit.dwMaxPayloadTransferSize |= (*(pData + 10) << 24);
		DbgMsg_UVCIO(("^B-[Still_ProbeCtrl--SET_CUR],fmtIdx=%d,frmIdx=%d,frmSize=0x%x,txfSize=0x%x\r\n", gUvcStillProbeCommit.bFormatIndex, \
					  gUvcStillProbeCommit.bFrameIndex, gUvcStillProbeCommit.dwMaxVideoFrameSize, gUvcStillProbeCommit.dwMaxPayloadTransferSize));
		break;
	case GET_CUR:
		DbgMsg_UVCIO(("^B[Still_ProbeCtrl--GET_CUR]\r\n"));
		UVC_DmpStillProbeCommitData(&gUvcStillProbeCommit);
		memset(pData, 0, sizeof(UVC_STILL_PROBE_COMMIT));
		UVC_GetCurStillProbeCommitData(pData);
		len = UVC_STILL_PROBE_COMMIT_LEN;
		if (len > ControlData.w_length) {
			DBG_ERR("dataSize=%d,%d\r\n", len, ControlData.w_length);
			len = ControlData.w_length;
		}
		ControlData.w_length  = len;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_MIN://0x82, 130
	case GET_MAX://0x83, 131
	case GET_DEF://0x87, 135
		if (GET_MIN == req) {
			len = UVC_VS_STILL_FRM_MIN_SIZE;
		} else if (GET_DEF == req) {
			len = UVC_VS_STILL_FRM_DEF_SIZE;
		} else {
			len = UVC_VS_STILL_FRM_MAX_SIZE;
		}
		DbgMsg_UVCIO(("^B[Still_ProbeCtrl--GET_MIN/MAX]:req=%d,len=0x%x\r\n", req, len));
		UVC_InitStillProbeCommitData((PUVC_STILL_PROBE_COMMIT)pData);
		pData += 3;
		*pData++ = len & 0xFF;
		*pData++ = (len >> 8) & 0xFF;
		*pData++ = (len >> 16) & 0xFF;
		*pData++ = (len  >> 24) & 0xFF;


#if 1
		if (usb3dev_getDevInfo(U3DEV_DEVINFO_ID_SPEED) == U3DEV_SPEED_SS) {
			*pData++ = (USB_SS_MAX_TXF_SIZE_ONE_PACKET) & 0xFF;
			*pData++ = (USB_SS_MAX_TXF_SIZE_ONE_PACKET >> 8) & 0xFF;
			*pData++ = (USB_SS_MAX_TXF_SIZE_ONE_PACKET >> 16) & 0xFF;
			*pData++ = (USB_SS_MAX_TXF_SIZE_ONE_PACKET >> 24) & 0xFF;
		} else {
			*pData++ = (USB_MAX_TXF_SIZE_ONE_PACKET) & 0xFF;
			*pData++ = (USB_MAX_TXF_SIZE_ONE_PACKET >> 8) & 0xFF;
			*pData++ = (USB_MAX_TXF_SIZE_ONE_PACKET >> 16) & 0xFF;
			*pData++ = (USB_MAX_TXF_SIZE_ONE_PACKET >> 24) & 0xFF;
		}
#else
		len += 2;
		*pData++ = len & 0xFF;
		*pData++ = (len >> 8) & 0xFF;
		*pData++ = (len >> 16) & 0xFF;
		*pData++ = (len  >> 24) & 0xFF;
#endif

		if (UVC_STILL_PROBE_COMMIT_LEN > ControlData.w_length) {
			DbgMsg_UVCIO(("^RHost len=0x%x < spec-len=0x%x\r\n", ControlData.w_length, UVC_STILL_PROBE_COMMIT_LEN));
		} else {
			ControlData.w_length = UVC_STILL_PROBE_COMMIT_LEN;
		}
		ControlData.pData = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_INFO:
		DbgMsg_UVCIO(("^B[Still_ProbeCtrl--GET_INFO]\r\n"));
		*pData = 0x03;
		ControlData.w_length  = 1;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_LEN:
		DbgMsg_UVCIO(("^B[Still_ProbeCtrl--GET_LEN]\r\n"));
		*pData = UVC_STILL_PROBE_COMMIT_LEN;
		ControlData.w_length  = 1;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_RES:
	default:
		usb_setEP0Stall();
		usb_setEP0Done();
		DBG_ERR("Still_ProbeCtrl-Not implement:%d\r\n", req);
		break;
	}
	DbgMsg_UVCIO(("^B-%s:req=%d,pDataAddr=0x%x, size=0x%x, intrf=%d\r\n", __func__, req, (UINT32)pDataAddr, len, intrf));
#endif
}
void UVC_VS_Still_CommitCtrl(UINT8 req, UINT8 *pData, UINT8 intrf)
{
#if 0
	UINT8 *pDataAddr = pData;
	UINT32 len = 0;

	DbgMsg_UVCIO(("^B+%s:req=%d,pDataAddr=0x%x, size=0x%x, intrf=%d, ctrl-len=%d\r\n", __func__, req, pData, len, intrf, ControlData.w_length));
	switch (req) {
	case SET_CUR:
		DbgMsg_UVCIO(("^B+[Still_CommitCtrl--SET_CUR],len=%d,gUvacStillImgTrigSts%d\r\n", len, gUvacStillImgTrigSts));
		len = UVC_STILL_PROBE_COMMIT_LEN;
		if (E_OK != usb_readEndpoint(USB_EP0, pData, &len)) {
			DBG_ERR("SetCur readEndpoint fail\r\n");
		}
		if (*pData != (gUvcStillProbeCommit.bFormatIndex)) {
			DBG_ERR("Not Match Format,in=%d,ori=%d\r\n", *pData, gUvcStillProbeCommit.bFormatIndex);
			usb_setEP0Stall();
		} else {
			gUvcStillProbeCommit.bFormatIndex = *pData;
			gUvcStillProbeCommit.bFrameIndex = *(pData + 1);
			gUvcStillProbeCommit.bCompressionIndex = *(pData + 2);
			gUvcStillProbeCommit.dwMaxVideoFrameSize = *(pData + 3);
			gUvcStillProbeCommit.dwMaxVideoFrameSize |= (*(pData + 4) << 8);
			gUvcStillProbeCommit.dwMaxVideoFrameSize |= (*(pData + 5) << 16);
			gUvcStillProbeCommit.dwMaxVideoFrameSize |= (*(pData + 6) << 24);
			gUvcStillProbeCommit.dwMaxPayloadTransferSize = *(pData + 7);
			gUvcStillProbeCommit.dwMaxPayloadTransferSize |= (*(pData + 8) << 8);
			gUvcStillProbeCommit.dwMaxPayloadTransferSize |= (*(pData + 9) << 16);
			gUvcStillProbeCommit.dwMaxPayloadTransferSize |= (*(pData + 10) << 24);
		}
		usb_setEP0Done();
		DbgMsg_UVCIO(("^B-[Still_CommitCtrl--SET_CUR],fmtIdx=%d,frmIdx=%d,frmSize=0x%x,txfSize=0x%x\r\n", gUvcStillProbeCommit.bFormatIndex, \
					  gUvcStillProbeCommit.bFrameIndex, gUvcStillProbeCommit.dwMaxVideoFrameSize, gUvcStillProbeCommit.dwMaxPayloadTransferSize));

		//call back to trigger take-a-photo event
		UVC_CapStilImgUploadTest(pData);
		break;
	case GET_CUR:
		DbgMsg_UVCIO(("^B[Still_CommitCtrl--GET_CUR]\r\n"));
		UVC_DmpStillProbeCommitData(&gUvcStillProbeCommit);
		memset(pData, 0, sizeof(UVC_STILL_PROBE_COMMIT));
		UVC_GetCurStillProbeCommitData(pData);
		len = UVC_STILL_PROBE_COMMIT_LEN;
		if (len > ControlData.w_length) {
			DBG_ERR("dataSize=%d,%d\r\n", len, ControlData.w_length);
			len = ControlData.w_length;
		}
		ControlData.w_length  = len;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_INFO:
		DbgMsg_UVCIO(("^B[Still_CommitCtrl--GET_INFO]\r\n"));
		*pData = 0x03;
		ControlData.w_length  = 1;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_LEN:
		DbgMsg_UVCIO(("^B[Still_CommitCtrl--GET_LEN]\r\n"));
		*pData = UVC_STILL_PROBE_COMMIT_LEN;
		ControlData.w_length  = 1;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_MIN://0x82, 130
	case GET_MAX://0x83, 131
	case GET_DEF://0x87, 135
	case GET_RES:
	default:
		usb_setEP0Stall();
		usb_setEP0Done();
		DBG_ERR("Still_CommitCtrl-Not implement:%d\r\n", req);
		break;
	}
	DbgMsg_UVCIO(("^B-%s:req=%d,pDataAddr=0x%x, size=0x%x, intrf=%d\r\n", __func__, req, (UINT32)pDataAddr, len, intrf));
#endif
}
void UVC_VS_Still_ImgTrigCtrl(UINT8 req, UINT8 *pData, UINT8 intrf)
{
#if 0
	UINT8 *pDataAddr = pData;
	UINT32 len = 0;

	DbgMsg_UVCIO(("^B+%s:req=%d,pDataAddr=0x%x, size=0x%x, intrf=%d\r\n", __func__, req, pData, len, intrf));
	switch (req) {
	case SET_CUR:
		len = UVC_STILLIMG_TRIG_REQ_LEN;
		DbgMsg_UVCIO(("^B+[Still_ImgTrigCtrl--SET_CUR],len=%d,gUvacStillImgTrigSts%d\r\n", len, gUvacStillImgTrigSts));
		if (E_OK != usb_readEndpoint(USB_EP0, pData, &len)) {
			DBG_ERR("SetCur readEndpoint fail\r\n");
		}
		usb_setEP0Done();
		DbgMsg_UVCIO(("^B-[Still_ImgTrigCtrl--SET_CUR],len=%d,bTrigger=%d,preTrig=%d\r\n", len, *pData, gUvacStillImgTrigSts));

		UVC_CapStilImgUpload(pData);
		break;

	case GET_CUR:
		DbgMsg_UVCIO(("^B[Still_ImgTrigCtrl--GET_CUR]\r\n"));
		*pData = gUvacStillImgTrigSts;
		ControlData.w_length  = UVC_STILLIMG_TRIG_REQ_LEN;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;
	case GET_INFO:
		DbgMsg_UVCIO(("^B[Still_ImgTrigCtrl--GET_INFO]\r\n"));
		*pData = 0x03;
		ControlData.w_length  = 1;
		ControlData.pData    = pDataAddr;
		usb_RetSetupData();
		break;

	default:
		usb_setEP0Stall();
		usb_setEP0Done();
		DBG_ERR("Not in UVC-Spec:%d\r\n", req);
		break;
	}
#endif
	//DbgMsg_UVCIO(("^B-%s:req=%d,pDataAddr=0x%x, size=0x%x, intrf=%d\r\n", __func__, req, (UINT32)pDataAddr, len, intrf));
}
#endif
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
static void UVC_CDC_ClassReq(UINT8 nInterface, UINT8 Request)
{
	ER       retV;
	UINT32   len = 0;
	CDC_COM_ID ComID;
	if (gUvacIntfIdx_CDC_COMM[CDC_COM_1ST] == nInterface || gUvacIntfIdx_CDC_DATA[CDC_COM_1ST] == nInterface) {
		ComID = CDC_COM_1ST;
	} else {
		ComID = CDC_COM_2ND;
	}
	if (Request == REQ_GET_LINE_CODING) {
		if (gfpCdcPstnReqCB) {
			CDCLineCoding LineCoding = {0};
			len = sizeof(LineCoding);
			if (gfpCdcPstnReqCB(ComID, Request, (UINT8 *)&LineCoding, &len)) {
				if (len > sizeof(LineCoding)) {
					DBG_ERR("LinCoding data size should NOT exceed %d byte.\r\n", sizeof(LineCoding));
					len = sizeof(LineCoding);
				}
				retV = usb_writeEndpoint(USB_EP0, (UINT8 *)&LineCoding, &len);
				if (retV != E_OK) {
					DBG_ERR("ClsReq GetDevSts,WriteEP=%d Fail=%d\r\n", USB_EP0, retV);
				}
			} else {
				usb_setEP0Stall();
			}
		} else {
			usb_setEP0Stall();
		}
	} else if (Request == REQ_SET_LINE_CODING) {
		if (gfpCdcPstnReqCB) {
			CDCLineCoding LineCoding;
			len = ControlData.DeviceRequest.w_length;
			retV = usb_readEndpoint(USB_EP0, (UINT8 *)&LineCoding, &len);
			if (retV != E_OK) {
				DBG_ERR("SetCancelReq,ReadEP=%d Fail=%d\r\n", USB_EP0, retV);
			} else {
				gfpCdcPstnReqCB(ComID, Request, (UINT8 *)&LineCoding, &len);
			}
		} else {
			usb_setEP0Stall();
		}
	} else if (Request == REQ_SET_CONTROL_LINE_STATE) {
		UINT16 ControlLineState = ControlData.DeviceRequest.w_value;
		len = sizeof(ControlLineState);
		if (gfpCdcPstnReqCB) {
			gfpCdcPstnReqCB(ComID, Request, (UINT8 *)&ControlLineState, &len);
		}
	} else if (Request == REQ_SEND_BREAK) {
		UINT16 SendBreak = ControlData.DeviceRequest.w_value;
		len = sizeof(SendBreak);
		if (gfpCdcPstnReqCB) {
			gfpCdcPstnReqCB(ComID, Request, (UINT8 *)&SendBreak, &len);
		}
	} else {
		DBG_ERR("Unsupported ClassReq=0x%x\r\n", ControlData.DeviceRequest.b_request);
		//stall EP0, including unreconginze command and unsupport command.
		usb_setEP0Stall();
	}
	usb_setEP0Done();
	DBG_MSG("[CHK]--classRequest\r\n");
}

static void UVC_SIDC_ClassReq(UINT8 b_request)
{
	UINT8    bClassData[USIDC_CLS_DATA_LEN] = {0};
	UINT16   i = 0;
	UINT32   len = 0;
	ER       retV = E_OK;

	if (b_request == USB_ClassReq_USIDC_GetDeviceStatusReq) {
		DBG_DUMP("GetDevStsReq:DevSts=%d\r\n", g_UvacDeviceState);

		i = USIDC_CLS_DATA_LEN;
		bClassData[0] = (UINT8) i;
		bClassData[3] = 0x20;               /*hard code response code*/

		if (g_UvacDeviceState == USIDC_DEVICE_OK) {
			bClassData[2] = 0x01;   //Device Ok
		} else {
			bClassData[2] = 0x19;   //Device Busy
		}
		g_UvacDeviceState = USIDC_DEVICE_OK;
		if (i > ControlData.DeviceRequest.w_length) {
			i = ControlData.DeviceRequest.w_length;
		}
		len = (i  < EP0_PACKET_SIZE) ? i : EP0_PACKET_SIZE;
		//driver will handle the cache issue
		//cpu_cleanInvalidateDCacheBlock((UINT32)bClassData, (((UINT32)bClassData) + len - 1));

		retV = usb_writeEndpoint(USB_EP0, (UINT8 *)bClassData, &len);
#if (THIS_DBGLVL >= 2)
		if (len != i) {
			DBG_ERR("ClsReq GetDevSts Len fail:%d,act=%d\r\n", i, len);
		}
#endif
		if (retV != E_OK) {
			DBG_ERR("ClsReq GetDevSts,WriteEP=%d Fail=%d\r\n", USB_EP0, retV);
		}
		usb_setEP0Done();

	} else if (b_request == USB_ClassReq_USIDC_SetResetReq) {
		DBG_ERR("#SetResetReq\r\n");
		usb_setEP0Done();
		g_UvacSessionID = 0;

	} else if (b_request == USB_ClassReq_USIDC_SetCancelReq) {
		DBG_ERR("#SetCancelReq\r\n");

		if ((ControlData.DeviceRequest.w_index == 0)  && \
			(ControlData.DeviceRequest.w_length == 0x06) && \
			(ControlData.DeviceRequest.w_value == 0)) {
			gUvacStop = 1;
		}
		g_UvacDeviceState = USIDC_DEVICE_BUSY;
		len = ControlData.DeviceRequest.w_length;
		if (len > USIDC_CLS_DATA_LEN) {
			len = USIDC_CLS_DATA_LEN;
		}
#if (THIS_DBGLVL >= 2)
		i = len;
#endif
		retV = usb_readEndpoint(USB_EP0, (UINT8 *)bClassData, &len);
#if (THIS_DBGLVL >= 2)
		if (len != i) {
			DBG_ERR("SetCancelReq Len fail:%d,act=%d\r\n", i, len);
		}
#endif
		if (retV != E_OK) {
			DBG_ERR("SetCancelReq,ReadEP=%d Fail=%d\r\n", USB_EP0, retV);
		}
		usb_setEP0Done();
	} else {
		DBG_ERR("Special ClassReq=0x%x\r\n", b_request);
		//stall EP0, including unreconginze command and unsupport command.
		usb_setEP0Stall();
		usb_setEP0Done();
	}

	DBG_MSG("[CHK]--classRequest\r\n");
}
/*---------------------------------------------------------------------------
 TPBulk_ClassRequestHandler

 Input      : none
 Output     : none
 Return Val : none
 Function   : processing of USB setup transaction TYPE = CLASS
----------------------------------------------------------------------------*/
void UvacClassRequestHandler(void)
{
	UINT8 ControlCode;
	UINT8 nInterface;
	UINT8 Unit_ID;
	UINT8 b_request;
	UINT16 datalen;
	UINT8 *pData;
	UINT32 thisDevIdx = UVAC_VID_DEV_CNT_1;
	UVC_PROBE_COMMIT *pProbeCommit = 0;


	ControlCode = ControlData.DeviceRequest.w_value >> 8;
	nInterface = ControlData.DeviceRequest.w_index & 0xFF;
	Unit_ID = ControlData.DeviceRequest.w_index >> 8;
	datalen = ControlData.DeviceRequest.w_length;
	b_request = ControlData.DeviceRequest.b_request;

	pData = &gUvacClsReqDataBuf[0];
	DbgMsg_UVCIO(("^G%s:req=0x%x,ctrl=%d,intr=%d,uid=%d,len=%d\r\n", __func__, b_request, ControlCode, nInterface, Unit_ID, datalen));
	if (ControlData.DeviceRequest.b_request == USB_CLASS_REQUEST_USBSTOR_RESET) {
		if ((ControlData.DeviceRequest.w_index == 0)  &&
			(ControlData.DeviceRequest.w_length == 0) &&
			(ControlData.DeviceRequest.w_value == 0)  &&
			!(ControlData.DeviceRequest.bm_request_type & USB_ENDPOINT_DIRECTION_MASK)) {
			usb_setEP0Done();
		} else {
			usb_setEP0Stall();
		}
	}
	if (gUvacWinIntrfEnable && (gUvacIntfIdx_WinUsb == nInterface)) {
		DbgMsg_UVC(("^RWinUSB:req=%d,ctrl=%d,intr=%d,uid=%d,len=%d\r\n", b_request, ControlCode, nInterface, Unit_ID, datalen));
		//callback
		UVC_WinUSB_ClassReq(ControlCode, b_request, pData);
	} else if (gUvacCdcEnabled && ((gUvacIntfIdx_CDC_COMM[CDC_COM_1ST] == nInterface) || (gUvacIntfIdx_CDC_DATA[CDC_COM_1ST] == nInterface) ||
								   (gUvacIntfIdx_CDC_COMM[CDC_COM_2ND] == nInterface) || (gUvacIntfIdx_CDC_COMM[CDC_COM_2ND] == nInterface))) {
		UVC_CDC_ClassReq(nInterface, b_request);
	} else if (gUvacMtpEnabled && gUvacIntfIdx_SIDC == nInterface) {
		UVC_SIDC_ClassReq(b_request);
	//else if ((nInterface == gUvacIntfIdx_VC[0]) || (nInterface == gUvacIntfIdx_VC[1]))
	else if (((nInterface & 0x7F) == gUvacIntfIdx_VC[UVAC_VID_DEV_CNT_1]) || ((nInterface & 0x7F) == gUvacIntfIdx_VC[UVAC_VID_DEV_CNT_2])) {
		UVC_TERMINAL_ID UnitID = Unit_ID;
		if (UnitID == UVC_TERMINAL_ID_NULL) {
			if (ControlCode == VC_VIDEO_POWER_MODE_CONTROL) {
				DBG_ERR("UVC:[VC_VIDEO_POWER_MODE_CONTROL] Not imp.\r\n");

			} else if (ControlCode == VC_REQUEST_ERROR_CODE_CONTROL) {
				UVC_VC_ErrorCtrl(b_request, pData);
			}
		}
		// Camera Terminal Requests
		else if (UnitID == UVC_TERMINAL_ID_ITT) {
			DBG_ERR("UVC: Unsupported Camera Terminal CONTROLs \r\n");
			usb_setEP0Stall();
			usb_setEP0Done();
		}
		// Selector Unit Requests
		else if (UnitID == UVC_TERMINAL_ID_SEL_UNIT) {
			if (ControlCode == SU_INPUT_SELECT_CONTROL) {
				DbgMsg_UVCIO(("UVC:[SU_INPUT_SELECT_CONTROL] \r\n"));
				UVC_SU_SelectCtrl(b_request, pData);
			}
		} else if (UnitID == UVC_TERMINAL_ID_EXTE_UNIT) {
			DbgMsg_UVCIO(("^BUVC-EU:ctl=%d, req=%d, pData=0x%x\r\n", ControlCode, b_request, pData));
			UVC_XU_VendorCmd(ControlCode, b_request, pData);
		} else {
			DBG_ERR("UVC: Unsupported CONTROLs, Uid=%d \r\n", UnitID);
			usb_setEP0Stall();
			usb_setEP0Done();
		}
	}
	//Video Streaming I/F requests
	else if ((gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_1] == (nInterface & 0x7F)) || (gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_2] == (nInterface & 0x7F))) {
		UVC_TERMINAL_ID UnitID = Unit_ID;
		if (gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_1] == (nInterface & 0x7F)) {
			thisDevIdx = UVAC_VID_DEV_CNT_1;
		} else {
			thisDevIdx = UVAC_VID_DEV_CNT_2;
		}
		pProbeCommit = &g_ProbeCommit[thisDevIdx];
		DbgMsg_UVC(("^BUVC:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x,thisDevIdx=%d,pProbeCommit=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode, thisDevIdx, pProbeCommit));

		// Interface Requests
		if (UnitID == UVC_TERMINAL_ID_NULL) {
			if (ControlCode == VS_PROBE_CONTROL) {
				UVC_VS_ProbeCtrl(b_request, pData, pProbeCommit);
			} else if (ControlCode == VS_COMMIT_CONTROL) {
				UVC_VS_CommitCtrl(b_request, pData, pProbeCommit, thisDevIdx);
			} else if (ControlCode == VS_STILL_PROBE_CONTROL) {
				DbgMsg_UVCIO(("^BVS_STILL_PROBE_CONTROL:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode));
				UVC_VS_Still_ProbeCtrl(b_request, pData, nInterface);
			} else if (ControlCode == VS_STILL_COMMIT_CONTROL) {
				DbgMsg_UVCIO(("^BVS_STILL_COMMIT_CONTROL:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode));
				UVC_VS_Still_CommitCtrl(b_request, pData, nInterface);
			} else if (ControlCode == VS_STILL_IMAGE_TRIGGER_CONTROL) {
				DbgMsg_UVCIO(("^BVS_STILL_IMAGE_TRIGGER_CONTROL:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode));
				UVC_VS_Still_ImgTrigCtrl(b_request, pData, nInterface);
			} else {
				DBG_ERR("^RUVC: Unsupported CONTROLs \r\n");
				usb_setEP0Stall();
				usb_setEP0Done();

			}
		}
	}
	//Audio Control I/F requests
	else if ((gUvacIntfIdx_AC[0] == (nInterface & 0x7F)) || (gUvacIntfIdx_AC[1] == (nInterface & 0x7F)) || (gUvacIntfIdx_AS[0] == (nInterface & 0x7F)) || (gUvacIntfIdx_AS[1] == (nInterface & 0x7F))) {
		UAC_TERMINAL_ID UnitID = ControlData.DeviceRequest.w_index >> 8;;
		DbgMsg_UVCIO(("^GUAC:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode));
		if ((UnitID == UAC_TERMINAL_ID_FU) || (UnitID == UAC_TERMINAL_ID_NULL)) {
			if (ControlCode == VS_PROBE_CONTROL) {
				UVC_AC_ProbeCtrl(b_request, pData);
			} else if (ControlCode == VS_COMMIT_CONTROL) {
				UVC_AC_CommitCtrl(b_request, pData);
			} else {
				DBG_ERR("^RUAC: Unsupported CONTROLs \r\n");
				usb_setEP0Stall();
				usb_setEP0Done();
			}
		}
	} else {
		DBG_ERR("UVC:Cannot handle this interface] %x\r\n", nInterface);
		DBG_ERR("req=0x%x,ctrl=%d,intr=%d,uid=%d,len=%d\r\n", b_request, ControlCode, nInterface, Unit_ID, datalen);

		usb_setEP0Stall();
		usb_setEP0Done();
	}
}
void UVC_XU_VendorCmd(UINT32 ControlCode, UINT8 CS, UINT8 *pData)
{
	UINT8 vendCBRet;
	DbgMsg_UVCIO(("VendCmd: ctrl=%d, cs=0x%x, pData=0x%x\r\n", ControlCode, CS, pData));
	//DBG_DUMP("VendCmd: ctrl=%d, cs=0x%x, pData=0x%x\r\n", ControlCode, CS, pData);

	//ControlCode: 1, 2, ..., 8
	if ((UVAC_EU_VENDCMD_CNT > ControlCode) && (gUvacEUVendCmdCB[ControlCode])) {
		if (SET_CUR == CS) {
			DbgMsg_UVCIO(("^BUVC:[EU --SET_CUR]\r\n"));
			gUvacVendDataLen = USB_UVAC_CLS_DATABUF_LEN;
			memset(pData, 0, USB_UVAC_CLS_DATABUF_LEN);
			usb_readEndpoint(USB_EP0, pData, &gUvacVendDataLen);
			usb_setEP0Done();
		}
		vendCBRet = gUvacEUVendCmdCB[ControlCode](ControlCode, CS, pData, &gUvacVendDataLen);
	} else {
		DBG_IND("gUvacEUVendCmdCB[%d] NULL, CS=%d, pData=0x%x\r\n", ControlCode, CS, pData);
		vendCBRet = FALSE;
	}
	if (TRUE == vendCBRet) {
		DbgMsg_UVCIO(("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, pData));
		DbgMsg_UVCIO(("  ==> *pData=0x%x\r\n", *pData));
		//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x", gUvacVendDataLen, ControlCode, CS, pData);
		//DBG_DUMP("==> *pData=0x%x\r\n", *pData);
		gUvacVendDataLen = (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) ? USB_UVAC_CLS_DATABUF_LEN : gUvacVendDataLen;
		if (SET_CUR != CS) {
			ControlData.w_length  = gUvacVendDataLen;
			ControlData.pData    = pData;
			usb_RetSetupData();
		}
	} else {
		DBG_IND("VendCmd Fail, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, pData);
		usb_setEP0Stall();
		usb_setEP0Done();
	}
}
#elif (UVAC_CONTROLLER == UVAC_CONTROL_U3)
static void UVC_CDC_ClassReq(CDC_COM_ID ComID, PUSB_DEVICE_REQUEST pRequest)
{
	//ER       retV;
	UINT32   len = 0;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	UINT8 Request;

	Request = pRequest->b_request;

	if (Request == REQ_GET_LINE_CODING) {
		if (gfpCdcPstnReqCB) {
			CDCLineCoding LineCoding = {0};
			len = sizeof(LineCoding);
			if (gfpCdcPstnReqCB(ComID, Request, (UINT8 *)&LineCoding, &len)) {
				if (len > sizeof(LineCoding)) {
					DBG_ERR("LinCoding data size should NOT exceed %d byte.\r\n", sizeof(LineCoding));
					len = sizeof(LineCoding);
				}
				//bStall = FALSE;
				uiRetBufAddr = (uintptr_t) &LineCoding;
				uiRetSize = len;
			} else {
				bStall = TRUE;
			}
		} else {
			bStall = TRUE;
		}
	} else if (Request == REQ_SET_LINE_CODING) {
		if (gfpCdcPstnReqCB) {
#if 0
			CDCLineCoding LineCoding;
			len = pRequest->w_length;
			retV = USBCOMPAT_READENDPOINT(USB_EP0, (UINT8 *)&LineCoding, &len);
			if (retV != E_OK) {
				DBG_ERR("SetCancelReq,ReadEP=%d Fail=%d\r\n", USB_EP0, retV);
			} else {
				gfpCdcPstnReqCB(ComID, Request, (UINT8 *)&LineCoding, &len);
			}
#else
			m_ComID = ComID;
			m_RequestType = UVAC_CLASS_REQ_CDC;
			m_RequestLen = pRequest->w_length;
			uiRetBufAddr = (uintptr_t) gUvacClsReqDataBuf;
			uiRetSize = m_RequestLen;
#endif

		} else {
			bStall = TRUE;
		}
	} else if (Request == REQ_SET_CONTROL_LINE_STATE) {
		UINT16 ControlLineState = pRequest->w_value;
		len = sizeof(ControlLineState);
		if (gfpCdcPstnReqCB) {
			gfpCdcPstnReqCB(ComID, Request, (UINT8 *)&ControlLineState, &len);
		}
	} else if (Request == REQ_SEND_BREAK) {
		UINT16 SendBreak = pRequest->w_value;
		len = sizeof(SendBreak);
		if (gfpCdcPstnReqCB) {
			gfpCdcPstnReqCB(ComID, Request, (UINT8 *)&SendBreak, &len);
		}
	} else {
		DBG_ERR("Unsupported ClassReq=0x%x\r\n", Request);
		//stall EP0, including unreconginze command and unsupport command.
		bStall = TRUE;
	}
	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
	DBG_MSG("[CHK]--classRequest\r\n");
}

static void UVC_SIDC_ClassReq(PUSB_DEVICE_REQUEST pRequest)
{
#if 0//gUvacMtpEnabled
	UINT8    bClassData[USIDC_CLS_DATA_LEN] = {0};
	UINT16   i = 0;
	UINT32   len = 0;
	ER       retV = E_OK;
	BOOL bStall = FALSE;
	UINT32 uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;
	if (pRequest->b_request == USB_ClassReq_USIDC_GetDeviceStatusReq) {
		DBG_DUMP("GetDevStsReq:DevSts=%d\r\n", g_UvacDeviceState);

		i = USIDC_CLS_DATA_LEN;
		bClassData[0] = (UINT8) i;
		bClassData[3] = 0x20;               /*hard code response code*/

		if (g_UvacDeviceState == USIDC_DEVICE_OK) {
			bClassData[2] = 0x01;   //Device Ok
		} else {
			bClassData[2] = 0x19;   //Device Busy
		}
		g_UvacDeviceState = USIDC_DEVICE_OK;
		if (i > pRequest->w_length) {
			i = pRequest->w_length;
		}
		len = (i  < EP0_PACKET_SIZE) ? i : EP0_PACKET_SIZE;
		//driver will handle the cache issue
		//cpu_cleanInvalidateDCacheBlock((UINT32)bClassData, (((UINT32)bClassData) + len - 1));
		uiRetBufAddr = (UINT32)bClassData;
		uiRetSize = len;
#if (THIS_DBGLVL >= 2)
		if (len != i) {
			DBG_ERR("ClsReq GetDevSts Len fail:%d,act=%d\r\n", i, len);
		}
#endif
		if (retV != E_OK) {
			DBG_ERR("ClsReq GetDevSts,WriteEP=%d Fail=%d\r\n", USB_EP0, retV);
		}
	} else if (pRequest->b_request == USB_ClassReq_USIDC_SetResetReq) {
		DBG_ERR("#SetResetReq\r\n");
		g_UvacSessionID = 0;

	} else if (pRequest->b_request == USB_ClassReq_USIDC_SetCancelReq) {
		DBG_ERR("#SetCancelReq\r\n");

		if ((pRequest->w_index == 0)  && \
			(pRequest->w_length == 0x06) && \
			(pRequest->w_value == 0)) {
			gUvacStop = 1;
		}
		g_UvacDeviceState = USIDC_DEVICE_BUSY;
#if 0
		len = pRequest->w_length;
		if (len > USIDC_CLS_DATA_LEN) {
			len = USIDC_CLS_DATA_LEN;
		}
#if (THIS_DBGLVL >= 2)
		i = len;
#endif
		retV = USBCOMPAT_READENDPOINT(USB_EP0, (UINT8 *)bClassData, &len);
#if (THIS_DBGLVL >= 2)
		if (len != i) {
			DBG_ERR("SetCancelReq Len fail:%d,act=%d\r\n", i, len);
		}
#endif
		if (retV != E_OK) {
			DBG_ERR("SetCancelReq,ReadEP=%d Fail=%d\r\n", USB_EP0, retV);
		}
#else
		m_RequestType = UVAC_CLASS_REQ_SIDC;
		m_RequestLen = pRequest->w_length;
		uiRetBufAddr = (UINT32) bClassData;
		uiRetSize = m_RequestLen;
#endif


	} else {
		DBG_ERR("Special ClassReq=0x%x\r\n", pRequest->b_request);
		//stall EP0, including unreconginze command and unsupport command.
		bStall = TRUE;
	}
	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
	DBG_MSG("[CHK]--classRequest\r\n");
#else
	usb3dev_handleSetupResult(TRUE, 0, 0);
#endif
}
static void UVC_AC_MuteCtrl(UINT8 CS, UINT8 *pData, UINT32 datalen)
{
	UINT8 *pDataAddr;
	UINT32 datasize;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR: { //0x81
			DbgMsg_UVCIO(("^BUAC:[AS Mute control --GET_CUR]\r\n"));

			datasize = datalen;
			memset(pData, 0, datasize);
			*pData = gUacMute;

			uiRetSize    = datasize;
			uiRetBufAddr = (uintptr_t)pDataAddr;
			usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			break;
		}
	case SET_CUR: {
			DbgMsg_UVCIO(("^BUAC:[AS Mute control --SET_CUR]\r\n"));

			m_RequestType = UVAC_CLASS_REQ_AC_MUTE;
			m_RequestLen = datalen;
			usb3dev_handleSetupResult(FALSE, (uintptr_t) pData, m_RequestLen);
			break;
		}
	case GET_MIN:
	case GET_MAX:
	case GET_RES:
	default:
		DBG_ERR("^RUAC:[AC MuteCtrl -- %x] not support\r\n", CS);
		usb3dev_handleSetupResult(TRUE, 0, 0);
		break;
	}
}

static void UVC_AC_VolCtrl(UINT8 CS, UINT8 *pData, UINT32 datalen)
{
	UINT8 *pDataAddr;
	UINT32 datasize;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	pDataAddr = pData;
	DbgMsg_UVCIO(("^G%s:0x%x\r\n", __func__, CS));

	switch (CS) {
	case GET_CUR: { //0x81
			DbgMsg_UVCIO(("^BUAC:[AS Vol control --GET_CUR]\r\n"));

			datasize = 2;
			memset(pData, 0, datasize);

			*pData++ = u3_uac_vol_cur & 0xFF;
			*pData++ = (u3_uac_vol_cur >> 8) & 0xFF;

			uiRetSize    = datasize;
			uiRetBufAddr = (uintptr_t)pDataAddr;
			usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);

			break;
		}
	case SET_CUR: {
			DbgMsg_UVCIO(("^BUAC:[AS Vol control --SET_CUR]\r\n"));
			m_RequestType = UVAC_CLASS_REQ_AC_VOL;
			m_RequestLen = datalen;
			usb3dev_handleSetupResult(FALSE, (uintptr_t) pData, m_RequestLen);

			break;
		}
	case GET_MIN:
	case GET_MAX:
	case GET_RES: {
			UINT32 tmpsetting;

			if (CS == GET_RES) {
				tmpsetting = u3_uac_vol_res;
			} else if (CS == GET_MAX) {
				tmpsetting = u3_uac_vol_max;
			} else {
				tmpsetting = u3_uac_vol_min;
			}

			datasize = datalen;
			memset(pData, 0, datasize);

			*pData++ = tmpsetting & 0xFF;
			*pData++ = (tmpsetting >> 8) & 0xFF;

			uiRetSize    = datasize;
			uiRetBufAddr = (uintptr_t)pDataAddr;
			usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			break;
		}
	default:
		DBG_ERR("^RUAC:[AC VolCtrl -- %x] not support\r\n", CS);
		usb3dev_handleSetupResult(TRUE, 0, 0);
		break;
	}
}
void UvacClassRequestHandler(uintptr_t uiEvent)
{
	UINT8 ControlCode;
	UINT8 nInterface;
	UINT8 Unit_ID;
	UINT8 b_request;
	UINT16 datalen;
	UINT8 *pData;
	UINT32 thisDevIdx = UVAC_VID_DEV_CNT_1;
	UVC_PROBE_COMMIT *pProbeCommit = 0;
	UINT32 recipient;

	PUSB_DEVICE_REQUEST    pRequest;

	pRequest = (PUSB_DEVICE_REQUEST)uiEvent;

	ControlCode = pRequest->w_value >> 8;
	nInterface = pRequest->w_index & 0xFF;
	Unit_ID = pRequest->w_index >> 8;
	datalen = pRequest->w_length;
	b_request = pRequest->b_request;
	recipient = pRequest->bm_request_type & 0x1F;

	if (!init_probe) {
		UVC_InitProbeCommitData();
		init_probe = TRUE;
	}

	g_usb_speed = usb3dev_getDevInfo(U3DEV_DEVINFO_ID_SPEED);

	pData = &gUvacClsReqDataBuf[0];
	DbgMsg_UVCIO(("^G%s:req=0x%x,ctrl=%d,intr=%d,uid=%d,len=%d\r\n", __func__, b_request, ControlCode, nInterface, Unit_ID, datalen));

	m_RequestType = UVAC_CLASS_REQ_NONE;
	if (pRequest->b_request == USB_CLASS_REQUEST_USBSTOR_RESET) {
		if ((pRequest->w_index == 0)  &&
			(pRequest->w_length == 0) &&
			(pRequest->w_value == 0)  &&
			!(pRequest->bm_request_type & USB_ENDPOINT_DIRECTION_MASK)) {
			usb3dev_handleSetupResult(FALSE, 0, 0);
		} else {
			//stall EP0
			usb3dev_handleSetupResult(TRUE, 0, 0);
		}
	}
	if (pRequest->bm_request_type & 0x2 && nInterface == UVAC_USB_RX_EP[0]) {
		//Recipient is endpoint
		DbgMsg_UVCIO(("^GEP:EP=0x%x,Req=0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, Unit_ID, ControlCode));
		if (nInterface == UVAC_USB_RX_EP[0]) {

			if (ControlCode == SAMPLING_FREQ_CONTROL) {
				UVC_ACO_EP_Freq_Ctrl(b_request, pData);
			} else {
				DBG_ERR("EP ControlCode = %d\r\n", ControlCode);
				usb3dev_handleSetupResult(TRUE, 0, 0);
			}
		} else {
			DBG_ERR("EP = %d\r\n", Unit_ID);
			usb3dev_handleSetupResult(TRUE, 0, 0);
		}
	} else if (gUvacCdcEnabled[CDC_COM_1ST] && ((gUvacIntfIdx_CDC_COMM[CDC_COM_1ST] == nInterface) || (gUvacIntfIdx_CDC_DATA[CDC_COM_1ST] == nInterface))) {
		UVC_CDC_ClassReq(CDC_COM_1ST, pRequest);
	} else if (gUvacCdcEnabled[CDC_COM_2ND] && ((gUvacIntfIdx_CDC_COMM[CDC_COM_2ND] == nInterface) || (gUvacIntfIdx_CDC_DATA[CDC_COM_2ND] == nInterface))) {
		UVC_CDC_ClassReq(CDC_COM_2ND, pRequest);
	} else if (gUvacMtpEnabled && gUvacIntfIdx_SIDC == nInterface) {
		UVC_SIDC_ClassReq(pRequest);
	} else if (g_u3_hid_info.en && gU3UvacIntfIdx_HID == nInterface) {
		UVC_HID_ClassReq(pRequest->w_value, b_request, pData, datalen);
	} else if (nInterface == (UVAC_USB_EP[UVAC_TXF_QUE_A1]| USB_EP_IN_ADDRESS) || nInterface == (UVAC_USB_EP[UVAC_TXF_QUE_A2]| USB_EP_IN_ADDRESS)) {
		//Recipient is endpoint
		DbgMsg_UVCIO(("^GEP:EP=0x%x,Req=0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, Unit_ID, ControlCode));

		if (ControlCode == SAMPLING_FREQ_CONTROL) {
			UVC_AC_EP_Freq_Ctrl(b_request, pData);
		} else {
			DBG_ERR("EP ControlCode = %d\r\n", ControlCode);
			usb3dev_handleSetupResult(TRUE, 0, 0);
		}
	}
	else if (recipient == 0 || ((nInterface & 0x7F) == gUvacIntfIdx_VC[UVAC_VID_DEV_CNT_1]) || ((nInterface & 0x7F) == gUvacIntfIdx_VC[UVAC_VID_DEV_CNT_2]) || ((nInterface & 0x7F) == gUvacIntfIdx_VC[UVAC_VID_DEV_CNT_3])) {
		UVC_TERMINAL_ID UnitID = Unit_ID;
		if (UnitID == UVC_TERMINAL_ID_NULL) {
			if (ControlCode == VC_VIDEO_POWER_MODE_CONTROL) {
				DBG_ERR("UVC:[VC_VIDEO_POWER_MODE_CONTROL] Not imp.\r\n");

			} else if (ControlCode == VC_REQUEST_ERROR_CODE_CONTROL) {
				UVC_VC_ErrorCtrl(b_request, pData);
			}
		}
		// Camera Terminal Requests
		else if (UnitID == UVC_TERMINAL_ID_ITT) {
			UVC_Unit_Cmd(g_fpU3UvcCT_CB, ControlCode, b_request, pData, datalen);
		}
		// PU
		else if (UnitID == UVC_TERMINAL_ID_PROC_UNIT) {
			UVC_Unit_Cmd(g_fpU3UvcPU_CB, ControlCode, b_request, pData, datalen);
		}
		// Selector Unit Requests
		else if (UnitID == UVC_TERMINAL_ID_SEL_UNIT) {
			if (ControlCode == SU_INPUT_SELECT_CONTROL) {
				DbgMsg_UVCIO(("UVC:[SU_INPUT_SELECT_CONTROL] \r\n"));
				UVC_SU_SelectCtrl(b_request, pData);
			}
		} else if (UnitID == u3_eu_unit_id[0]) {
			DbgMsg_UVCIO(("^BUVC-EU:ctl=%d, req=%d, pData=0x%x\r\n", ControlCode, b_request, pData));
			if (g_fpU3UvcXU_CB) {
				UVC_Unit_Cmd(g_fpU3UvcXU_CB, ControlCode, b_request, pData, datalen);
			} else if (u3_eu_desc[0].eu_cb) {
				UVC_Unit_Cmd(u3_eu_desc[0].eu_cb, ControlCode, b_request, pData, datalen);
			} else {
				UVC_XU_VendorCmd(ControlCode, b_request, pData);
			}
		} else if (UnitID == u3_eu_unit_id[1]) {
			DbgMsg_UVCIO(("^BUVC-EU:ctl=%d, req=%d, pData=0x%x\r\n", ControlCode, b_request, pData));
			if (u3_eu_desc[1].eu_cb) {
				UVC_Unit_Cmd(u3_eu_desc[1].eu_cb, ControlCode, b_request, pData, datalen);
			}else {
				UVC_XU_VendorCmd(ControlCode, b_request, pData);
			}
		} else {

			UINT32 i;
			BOOL found_eu = FALSE;

			if (u3_eu_desc_array) {
				for (i = 0; i < u3_eu_desc_array->eu_num; i++) {
					if (UnitID == u3_eu_desc_array->eu_desc[i].bUnitID) {
						UVC_Unit_Cmd(u3_eu_desc_array->eu_desc[i].eu_cb, ControlCode, b_request, pData, datalen);
						found_eu = TRUE;
					}

				}
			}

			if (!found_eu) {
				DBG_ERR("UVC: Unsupported CONTROLs, Uid=%d \r\n", UnitID);
				//stall EP0
				usb3dev_handleSetupResult(TRUE, 0, 0);
			}
		}
	}
	//Video Streaming I/F requests
	else if ((gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_1] == (nInterface & 0x7F)) || (gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_2] == (nInterface & 0x7F)) || (gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_3] == (nInterface & 0x7F))) {
		UVC_TERMINAL_ID UnitID = Unit_ID;
		if (gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_1] == (nInterface & 0x7F)) {
			thisDevIdx = UVAC_VID_DEV_CNT_1;
			m_VidDevId = UVAC_VID_DEV_CNT_1;
		} else if (gUvacIntfIdx_VS[UVAC_VID_DEV_CNT_2] == (nInterface & 0x7F)) {
			thisDevIdx = UVAC_VID_DEV_CNT_2;
			m_VidDevId = UVAC_VID_DEV_CNT_2;
		} else {
			thisDevIdx = UVAC_VID_DEV_CNT_3;
			m_VidDevId = UVAC_VID_DEV_CNT_3;
		}
		pProbeCommit = &g_ProbeCommit[thisDevIdx];
		DbgMsg_UVC(("^BUVC:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x,thisDevIdx=%d,pProbeCommit=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode, thisDevIdx, pProbeCommit));

		// Interface Requests
		if (UnitID == UVC_TERMINAL_ID_NULL) {
			if (ControlCode == VS_PROBE_CONTROL) {
				UVC_VS_ProbeCtrl(b_request, pData, pProbeCommit);
			} else if (ControlCode == VS_COMMIT_CONTROL) {
				UVC_VS_CommitCtrl(b_request, pData, pProbeCommit, thisDevIdx);
			} else if (ControlCode == VS_STILL_PROBE_CONTROL) {
				DbgMsg_UVCIO(("^BVS_STILL_PROBE_CONTROL:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode));
				UVC_VS_Still_ProbeCtrl(b_request, pData, nInterface);
			} else if (ControlCode == VS_STILL_COMMIT_CONTROL) {
				DbgMsg_UVCIO(("^BVS_STILL_COMMIT_CONTROL:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode));
				UVC_VS_Still_CommitCtrl(b_request, pData, nInterface);
			} else if (ControlCode == VS_STILL_IMAGE_TRIGGER_CONTROL) {
				DbgMsg_UVCIO(("^BVS_STILL_IMAGE_TRIGGER_CONTROL:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode));
				UVC_VS_Still_ImgTrigCtrl(b_request, pData, nInterface);
			} else {
				DBG_ERR("^RUVC: Unsupported CONTROLs \r\n");
				//stall EP0
				usb3dev_handleSetupResult(TRUE, 0, 0);

			}
		}
	}
	//Audio Control I/F requests
	else if ((gUvacIntfIdx_AC[0] == (nInterface & 0x7F)) || (gUvacIntfIdx_AC[1] == (nInterface & 0x7F)) || (gUvacIntfIdx_AS[0] == (nInterface & 0x7F)) || (gUvacIntfIdx_AS[1] == (nInterface & 0x7F))) {
		UAC_TERMINAL_ID UnitID = pRequest->w_index >> 8;;
		DbgMsg_UVCIO(("^GUAC:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode));

		if (UnitID == UAC_TERMINAL_ID_NULL) { //endpoint control
			if (ControlCode == VS_PROBE_CONTROL) {
				UVC_AC_ProbeCtrl(b_request, pData, datalen);
			} else if (ControlCode == VS_COMMIT_CONTROL) {
				UVC_AC_CommitCtrl(b_request, pData, datalen);
			} else {
				DBG_ERR("^RUAC: Unsupported CONTROLs \r\n");
				//stall EP0
				usb3dev_handleSetupResult(TRUE, 0, 0);
			}
		} else if (UnitID == UAC_TERMINAL_ID_FU) {
			if (ControlCode == UAC_FU_CONTROL_MUTE) {
				UVC_AC_MuteCtrl(b_request, pData, datalen);
			} else if (ControlCode == UAC_FU_CONTROL_VOLUME) {
				UVC_AC_VolCtrl(b_request, pData, datalen);
			} else {
				DBG_ERR("^RUAC: Unsupported CONTROLs \r\n");
				//stall EP0
				usb3dev_handleSetupResult(TRUE, 0, 0);
			}
		} else if (UnitID == UAC_TERMINAL_ID_FU2) {
			if (ControlCode == UAC_FU_CONTROL_MUTE) {
				UVC_ACO_MuteCtrl(b_request, pData, datalen);
			} else if (ControlCode == UAC_FU_CONTROL_VOLUME) {
				UVC_ACO_VolCtrl(b_request, pData, datalen);
			} else {
				DBG_ERR("^RUAC: Unsupported CONTROLs \r\n");
				usb3dev_handleSetupResult(TRUE, 0, 0);
			}
		}
	} //Audio Control I/F requests
	else if ((gUvacIntfIdx_AC[2] == (nInterface & 0x7F)) || (gUvacIntfIdx_AS[2] == (nInterface & 0x7F))) {
		UAC_TERMINAL_ID UnitID = CONTROL_DATA.device_request.w_index >> 8;;
		DBG_IND("^GUAC:intr=0x%x,0x%x,0x%x,unit=0x%x,ctrl=0x%x\r\n", nInterface, b_request, pData, UnitID, ControlCode);


		if (UnitID == UAC_TERMINAL_ID_NULL) { //endpoint control
			if (ControlCode == VS_PROBE_CONTROL) {
				UVC_ACO_ProbeCtrl(b_request, pData, datalen);
			} else if (ControlCode == VS_COMMIT_CONTROL) {
				UVC_ACO_CommitCtrl(b_request, pData, datalen);
			} else {
				DBG_ERR("^RUAC: Unsupported CONTROLs \r\n");
				usb3dev_handleSetupResult(TRUE, 0, 0);
			}
		} else if (UnitID == UAC_TERMINAL_ID_FU2) {
			if (ControlCode == UAC_FU_CONTROL_MUTE) {
				UVC_ACO_MuteCtrl(b_request, pData, datalen);
			} else if (ControlCode == UAC_FU_CONTROL_VOLUME) {
				UVC_ACO_VolCtrl(b_request, pData, datalen);
			} else {
				DBG_ERR("^RUAC: Unsupported CONTROLs \r\n");
				usb3dev_handleSetupResult(TRUE, 0, 0);
			}
		}
	} else {
		if (g_u3_msdc_info.en && g_u3uvac_msdc_class_req_cb) {
			g_u3uvac_msdc_class_req_cb(uiEvent);
		} else {
			DBG_ERR("UVC:Cannot handle this interface] %x\r\n", nInterface);
			DBG_ERR("req=0x%x,ctrl=%d,intr=%d,uid=%d,len=%d\r\n", b_request, ControlCode, nInterface, Unit_ID, datalen);

			//stall EP0
			usb3dev_handleSetupResult(TRUE, 0, 0);
		}
	}
}

extern int _U3UVC_Stop(UVAC_VID_DEV_CNT vidDevIdx);
extern UINT32 gUvacTxfQueCurCnt[UVAC_TXF_QUE_MAX];
void UvacClassRequestDone(uintptr_t uiEvent)
{
	UINT32 vidDevIdx;
	UINT32 tmpSampleRate = 0;
	UINT32 len = sizeof(gUacSampleRate);
	UINT32 i = 0;
	BOOL bStall = FALSE;
	//uintptr_t uiRetBufAddr = 0;
	//UINT32 uiRetSize = 0;
	UINT8 *pData;
	UINT32 idx;
	UVAC_VIDEO_FORMAT codecType = UVAC_VIDEO_FORMAT_H264;

	UINT8 ControlCode;
	UINT8 b_request;
	PUSB_DEVICE_REQUEST    pRequest;

	UINT32 *mjpg_max_tx_size = 0;
	UINT32 *h264_max_tx_size = 0;
	UINT32 *yuv_max_tx_size = 0;
	UINT32 *nv12_max_tx_size = 0;
	UINT32 *h265_max_tx_size = 0;

	UINT32 mjpg_tx_base_size = 0;
	UINT32 h264_tx_base_size = 0;
	UINT32 yuv_tx_base_size = 0;
	UINT32 nv12_tx_base_size = 0;
	UINT32 h265_tx_base_size = 0;

	UINT32 mjpg_tx_base_size_cnt = 0;
	UINT32 h264_tx_base_size_cnt = 0;
	UINT32 yuv_tx_base_size_cnt = 0;
	UINT32 nv12_tx_base_size_cnt = 0;
	UINT32 h265_tx_base_size_cnt = 0;

	UVAC_VID_RESO_ARY *yuv_info;
	UVAC_VID_RESO_ARY *mjpg_info;
	UVAC_VID_RESO_ARY *h264_info;
	UVAC_VID_RESO_ARY *nv12_info;
	UVAC_VID_RESO_ARY *h265_info;

	pRequest = (PUSB_DEVICE_REQUEST)uiEvent;

	ControlCode = pRequest->w_value >> 8;
	b_request = pRequest->b_request;

	len = (len < m_RequestLen) ? len : m_RequestLen;

	DBG_IND("m_RequestType=%d\r\n", m_RequestType);

	pData = &gUvacClsReqDataBuf[0];

	switch (m_RequestType) {
	case UVAC_CLASS_REQ_VS_PROBE:
		if (m_pProbeCommit == &g_ProbeCommit[UVAC_VID_DEV_CNT_1]) {
			vidDevIdx = UVAC_VID_DEV_CNT_1;
		} else if (m_pProbeCommit == &g_ProbeCommit[UVAC_VID_DEV_CNT_2]) {
			vidDevIdx = UVAC_VID_DEV_CNT_2;
		} else if (m_pProbeCommit == &g_ProbeCommit[UVAC_VID_DEV_CNT_3]) {
			vidDevIdx = UVAC_VID_DEV_CNT_3;
		} else {
			DBG_ERR("m_pProbeCommit=0x%lX,&g_ProbeCommit[0]=0x%lX,&g_ProbeCommit[1]=0x%lX,&g_ProbeCommit[2]=0x%lX\r\n", (uintptr_t)m_pProbeCommit, (uintptr_t)&g_ProbeCommit[UVAC_VID_DEV_CNT_1], (uintptr_t)&g_ProbeCommit[UVAC_VID_DEV_CNT_2], (uintptr_t)&g_ProbeCommit[UVAC_VID_DEV_CNT_3]);
			break;
		}
		m_pProbeCommit->bFormatIndex = *(pData + 2);
		m_pProbeCommit->bFrameIndex = *(pData + 3);
		if (0 == m_pProbeCommit->bFrameIndex) {
			DBG_ERR("frame index zero!?\r\n");
			m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
		}
		m_pProbeCommit->dwFrameInterval = *(pData + 4);
		m_pProbeCommit->dwFrameInterval |= (*(pData + 5) << 8);
		m_pProbeCommit->dwFrameInterval |= (*(pData + 6) << 16);
		m_pProbeCommit->dwFrameInterval |= (*(pData + 7) << 24);

		m_pProbeCommit->wCompQuality = *(pData + 12);
		m_pProbeCommit->wCompQuality |= (*(pData + 13) << 8);

		set_probe_count++;

		if (g_usb_speed == U3DEV_SPEED_SS) {
			m_pProbeCommit->dwMaxPayloadTransferSize = gU3UvcIsoInSsPacketSize[UVAC_VID_EP_PACK_SIZE_MAX_CNT-1] * gU3UvcIsoInSsBandWidth[UVAC_VID_EP_PACK_SIZE_MAX_CNT-1];

			h264_max_tx_size = p_u3_uvc_ss_h264_max_tx_size[vidDevIdx];
			mjpg_max_tx_size = p_u3_uvc_ss_mjpg_max_tx_size[vidDevIdx];
			yuv_max_tx_size  = p_u3_uvc_ss_yuv_max_tx_size[vidDevIdx];
			nv12_max_tx_size  = p_u3_uvc_ss_nv12_max_tx_size[vidDevIdx];
			h265_max_tx_size = p_u3_uvc_ss_h265_max_tx_size[vidDevIdx];

			mjpg_tx_base_size = u3_uvc_ss_mjpg_tx_base_size[vidDevIdx];
			h264_tx_base_size = u3_uvc_ss_h264_tx_base_size[vidDevIdx];
			yuv_tx_base_size = u3_uvc_ss_yuv_tx_base_size[vidDevIdx];
			nv12_tx_base_size = u3_uvc_ss_nv12_tx_base_size[vidDevIdx];
			h265_tx_base_size = u3_uvc_ss_h265_tx_base_size[vidDevIdx];

			mjpg_tx_base_size_cnt = u3_uvc_ss_mjpg_max_tx_size_cnt[vidDevIdx];
			h264_tx_base_size_cnt = u3_uvc_ss_h264_max_tx_size_cnt[vidDevIdx];
			yuv_tx_base_size_cnt = u3_uvc_ss_yuv_max_tx_size_cnt[vidDevIdx];
			nv12_tx_base_size_cnt = u3_uvc_ss_nv12_max_tx_size_cnt[vidDevIdx];
			h265_tx_base_size_cnt = u3_uvc_ss_h265_max_tx_size_cnt[vidDevIdx];

			yuv_info  = &gU3UvcYuvFrmInfo[0];
			mjpg_info = &gU3UvcMjpgFrmInfo[0];
			h264_info = &gU3UvcH264FrmInfo[0];
			nv12_info = &gU3UvcNV12FrmInfo[0];
			h265_info = &gU3UvcH265FrmInfo[0];

		} else {
			m_pProbeCommit->dwMaxPayloadTransferSize = gU3UvcIsoInHsPacketSize[UVAC_VID_EP_PACK_SIZE_MAX_CNT-1] * gU3UvcIsoInHsBandWidth[UVAC_VID_EP_PACK_SIZE_MAX_CNT-1];

			h264_max_tx_size = p_u3_uvc_h264_max_tx_size[vidDevIdx];
			mjpg_max_tx_size = p_u3_uvc_mjpg_max_tx_size[vidDevIdx];
			yuv_max_tx_size  = p_u3_uvc_yuv_max_tx_size[vidDevIdx];
			nv12_max_tx_size  = p_u3_uvc_nv12_max_tx_size[vidDevIdx];
			h265_max_tx_size = p_u3_uvc_h265_max_tx_size[vidDevIdx];

			mjpg_tx_base_size = u3_uvc_mjpg_tx_base_size[vidDevIdx];
			h264_tx_base_size = u3_uvc_h264_tx_base_size[vidDevIdx];
			yuv_tx_base_size = u3_uvc_yuv_tx_base_size[vidDevIdx];
			nv12_tx_base_size = u3_uvc_nv12_tx_base_size[vidDevIdx];
			h265_tx_base_size = u3_uvc_h265_tx_base_size[vidDevIdx];

			mjpg_tx_base_size_cnt = u3_uvc_mjpg_max_tx_size_cnt[vidDevIdx];
			h264_tx_base_size_cnt = u3_uvc_h264_max_tx_size_cnt[vidDevIdx];
			yuv_tx_base_size_cnt = u3_uvc_yuv_max_tx_size_cnt[vidDevIdx];
			nv12_tx_base_size_cnt = u3_uvc_nv12_max_tx_size_cnt[vidDevIdx];
			h265_tx_base_size_cnt = u3_uvc_h265_max_tx_size_cnt[vidDevIdx];

			if (gU3HsUvcYuvFrmInfo[0].aryCnt != 0) {
				yuv_info  = &gU3HsUvcYuvFrmInfo[0];
			} else {
				yuv_info  = &gU3UvcYuvFrmInfo[0];
			}

			if (gU3HsUvcMjpgFrmInfo[0].aryCnt != 0) {
				mjpg_info = &gU3HsUvcMjpgFrmInfo[0];
			} else {
				mjpg_info = &gU3UvcMjpgFrmInfo[0];
			}

			if (gU3HsUvcH264FrmInfo[0].aryCnt != 0) {
				h264_info = &gU3HsUvcH264FrmInfo[0];
			} else {
				h264_info = &gU3UvcH264FrmInfo[0];
			}

			if (gU3HsUvcNV12FrmInfo[0].aryCnt != 0) {
				nv12_info = &gU3HsUvcNV12FrmInfo[0];
			} else {
				nv12_info = &gU3UvcNV12FrmInfo[0];
			}

			if (gU3HsUvcH265FrmInfo[0].aryCnt != 0) {
				h265_info = &gU3HsUvcH265FrmInfo[0];
			} else {
				h265_info = &gU3UvcH265FrmInfo[0];
			}
		}


		if (set_probe_count < set_probe_count_max) {

		if (m_pProbeCommit->bFormatIndex == u3_h264_fmt_index[vidDevIdx]) {
			if (m_pProbeCommit->bFrameIndex > h264_info[vidDevIdx].aryCnt) {
				DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, h264_info[vidDevIdx].aryCnt);
				m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
			}
			m_pProbeCommit->dwMaxVideoFrameSize = gUvcMJPGMaxTBR; //no use for h264
			idx = m_pProbeCommit->bFrameIndex - 1;
			if (h264_max_tx_size && h264_tx_base_size_cnt) {
				m_pProbeCommit->dwMaxPayloadTransferSize = h264_max_tx_size[idx];
			}
		} else if (m_pProbeCommit->bFormatIndex == u3_mjpeg_fmt_index[vidDevIdx]) {
			if (m_pProbeCommit->bFrameIndex > mjpg_info[vidDevIdx].aryCnt) {
				DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, mjpg_info[vidDevIdx].aryCnt);
				m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
			}
			idx = m_pProbeCommit->bFrameIndex - 1;
			m_pProbeCommit->dwMaxVideoFrameSize = mjpg_info[vidDevIdx].pVidResAry[idx].width*mjpg_info[vidDevIdx].pVidResAry[idx].height*2;
			if (mjpg_max_tx_size && mjpg_tx_base_size_cnt) {
				m_pProbeCommit->dwMaxPayloadTransferSize = mjpg_max_tx_size[idx];
			}
		} else if (m_pProbeCommit->bFormatIndex == u3_yuv_fmt_index) {
			if (m_pProbeCommit->bFrameIndex > yuv_info[vidDevIdx].aryCnt) {
				DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, yuv_info[vidDevIdx].aryCnt);
				m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
			}
			idx = m_pProbeCommit->bFrameIndex - 1;
			m_pProbeCommit->dwMaxVideoFrameSize = yuv_info[vidDevIdx].pVidResAry[idx].width*yuv_info[vidDevIdx].pVidResAry[idx].height*2;
			if (yuv_max_tx_size && yuv_tx_base_size_cnt) {
				m_pProbeCommit->dwMaxPayloadTransferSize = yuv_max_tx_size[idx];
			}
		} else if (m_pProbeCommit->bFormatIndex == u3_nv12_fmt_index) {
			if (m_pProbeCommit->bFrameIndex > nv12_info[vidDevIdx].aryCnt) {
				DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, nv12_info[vidDevIdx].aryCnt);
				m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
			}
			idx = m_pProbeCommit->bFrameIndex - 1;
			m_pProbeCommit->dwMaxVideoFrameSize = nv12_info[vidDevIdx].pVidResAry[idx].width*nv12_info[vidDevIdx].pVidResAry[idx].height*3/2;
			if (nv12_max_tx_size && nv12_tx_base_size_cnt) {
				m_pProbeCommit->dwMaxPayloadTransferSize = nv12_max_tx_size[idx];
			}
		} else if (m_pProbeCommit->bFormatIndex == u3_h265_fmt_index[vidDevIdx]) {
			if (m_pProbeCommit->bFrameIndex > h265_info[vidDevIdx].aryCnt) {
				DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, h265_info[vidDevIdx].aryCnt);
				m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
			}
			m_pProbeCommit->dwMaxVideoFrameSize = gUvcMJPGMaxTBR; //no use for h265
			idx = m_pProbeCommit->bFrameIndex - 1;
			if (h265_max_tx_size && h265_tx_base_size_cnt) {
				m_pProbeCommit->dwMaxPayloadTransferSize = h265_max_tx_size[idx];
			}
		} else {
			DBG_ERR("unknow format index=%d, h264=%d, mjpeg=%d, yuv=%d, nv12=%d\r\n", m_pProbeCommit->bFormatIndex, u3_h264_fmt_index[vidDevIdx], u3_mjpeg_fmt_index[vidDevIdx], u3_yuv_fmt_index, u3_nv12_fmt_index);
		}

		} else {
			if (m_pProbeCommit->bFormatIndex == u3_h264_fmt_index[vidDevIdx]) {
				if (m_pProbeCommit->bFrameIndex > h264_info[vidDevIdx].aryCnt) {
					DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, h264_info[vidDevIdx].aryCnt);
					m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
				}

				m_pProbeCommit->dwMaxVideoFrameSize = gUvcMJPGMaxTBR; //no use for h264
				idx = m_pProbeCommit->bFrameIndex - 1;
				if (h264_tx_base_size) {
					m_pProbeCommit->dwMaxPayloadTransferSize = h264_tx_base_size;
					DBG_DUMP("UVC[%d] probe dwMaxPayloadTransferSize = h264 base size = %d\r\n", vidDevIdx, h264_tx_base_size);
				}
			} else if (m_pProbeCommit->bFormatIndex == u3_mjpeg_fmt_index[vidDevIdx]) {
				if (m_pProbeCommit->bFrameIndex > mjpg_info[vidDevIdx].aryCnt) {
					DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, mjpg_info[vidDevIdx].aryCnt);
					m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
				}
				idx = m_pProbeCommit->bFrameIndex - 1;
				m_pProbeCommit->dwMaxVideoFrameSize = mjpg_info[vidDevIdx].pVidResAry[idx].width*mjpg_info[vidDevIdx].pVidResAry[idx].height*2;

				if (mjpg_tx_base_size) {
					m_pProbeCommit->dwMaxPayloadTransferSize = mjpg_tx_base_size;
					DBG_DUMP("UVC[%d] probe dwMaxPayloadTransferSize = mjpg base size = %d\r\n", vidDevIdx, mjpg_tx_base_size);
				}
			} else if (m_pProbeCommit->bFormatIndex == u3_yuv_fmt_index) {
				if (m_pProbeCommit->bFrameIndex > yuv_info[vidDevIdx].aryCnt) {
					DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, yuv_info[vidDevIdx].aryCnt);
					m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
					}
				idx = m_pProbeCommit->bFrameIndex - 1;
				m_pProbeCommit->dwMaxVideoFrameSize = yuv_info[vidDevIdx].pVidResAry[idx].width*yuv_info[vidDevIdx].pVidResAry[idx].height*2;
				if (yuv_tx_base_size) {
					m_pProbeCommit->dwMaxPayloadTransferSize = yuv_tx_base_size;
					DBG_DUMP("UVC[%d] probe dwMaxPayloadTransferSize = yuv base size = %d\r\n", vidDevIdx, yuv_tx_base_size);
				}
			} else if (m_pProbeCommit->bFormatIndex == u3_nv12_fmt_index) {
				if (m_pProbeCommit->bFrameIndex > nv12_info[vidDevIdx].aryCnt) {
					DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, nv12_info[vidDevIdx].aryCnt);
					m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
				}
				idx = m_pProbeCommit->bFrameIndex - 1;
				m_pProbeCommit->dwMaxVideoFrameSize = nv12_info[vidDevIdx].pVidResAry[idx].width*nv12_info[vidDevIdx].pVidResAry[idx].height*3/2;
				if (nv12_tx_base_size) {
					m_pProbeCommit->dwMaxPayloadTransferSize = nv12_tx_base_size;
					DBG_DUMP("UVC[%d] probe dwMaxPayloadTransferSize = nv12 base size = %d\r\n", vidDevIdx, nv12_tx_base_size);
				}
			} else if (m_pProbeCommit->bFormatIndex == u3_h265_fmt_index[vidDevIdx]) {
				if (m_pProbeCommit->bFrameIndex > h265_info[vidDevIdx].aryCnt) {
					DBG_ERR("frame index(%d) > resolution cnt(%d)\r\n", m_pProbeCommit->bFrameIndex, h265_info[vidDevIdx].aryCnt);
					m_pProbeCommit->bFrameIndex = UVC_VSFMT_DEF_FRM_IDX;//set to default
				}

				m_pProbeCommit->dwMaxVideoFrameSize = gUvcMJPGMaxTBR; //no use for h265
				idx = m_pProbeCommit->bFrameIndex - 1;
				if (h265_tx_base_size) {
					m_pProbeCommit->dwMaxPayloadTransferSize = h265_tx_base_size;
					DBG_DUMP("UVC[%d] probe dwMaxPayloadTransferSize = h265 base size = %d\r\n", vidDevIdx, h265_tx_base_size);
				}
			} else {
				DBG_ERR("unknow format index=%d, h264=%d, mjpeg=%d, yuv=%d, nv12=%d, h265=%d\r\n", m_pProbeCommit->bFormatIndex, u3_h264_fmt_index[vidDevIdx], u3_mjpeg_fmt_index[vidDevIdx], u3_yuv_fmt_index, u3_nv12_fmt_index, u3_h265_fmt_index[vidDevIdx]);
			}
		}
		if (u3_uvc_bulk_mode[vidDevIdx]) {
			if (m_pProbeCommit->bFormatIndex == u3_nv12_fmt_index) {
				if (g_usb_speed == U3DEV_SPEED_SS) {
					if (gU3UvcNv12Payload[vidDevIdx]) {
						m_pProbeCommit->dwMaxPayloadTransferSize = gU3UvcNv12Payload[vidDevIdx];
					} else {
						m_pProbeCommit->dwMaxPayloadTransferSize = u3_uvc_bulk_payload_size;
					}
				} else {
					if (gU3HsUvcNv12Payload[vidDevIdx]) {
						m_pProbeCommit->dwMaxPayloadTransferSize = gU3HsUvcNv12Payload[vidDevIdx];
					} else {
						m_pProbeCommit->dwMaxPayloadTransferSize = u3_uvc_hs_bulk_payload_size;
					}
				}
			} else {
				if (g_usb_speed == U3DEV_SPEED_SS) {
					m_pProbeCommit->dwMaxPayloadTransferSize = u3_uvc_bulk_payload_size;
				} else {
					m_pProbeCommit->dwMaxPayloadTransferSize = u3_uvc_hs_bulk_payload_size;
				}
			}
		}
		if (gU3UvacUvcVer >= UVAC_UVC_VER_110) {
			m_pProbeCommit->dwClockFrequency = UVC_VC_CLOCK_FREQUENCY;
			m_pProbeCommit->bmFramingInfo = 0;
			m_pProbeCommit->bPreferredVersion = 1;
			m_pProbeCommit->bMinVersion = 1;
			m_pProbeCommit->bMaxVersion = 1;
		}

		if (gU3UvacUvcVer == UVAC_UVC_VER_150) {
			m_pProbeCommit->bUsage = 0;
			m_pProbeCommit->bBitDepthLuma = 0;
			m_pProbeCommit->bmSettings = 0;
			m_pProbeCommit->bMaxNumberOfRefFramesPlus1 = 0;
			m_pProbeCommit->bmRateControlModes = 0;
			m_pProbeCommit->bmLayoutPerStream[0] = 0;
			m_pProbeCommit->bmLayoutPerStream[1] = 0;
			m_pProbeCommit->bmLayoutPerStream[2] = 0;
			m_pProbeCommit->bmLayoutPerStream[3] = 0;
		}

		UVC_DmpProbeData(2, m_pProbeCommit);

		DbgMsg_UVCIO(("UVC: Format index =%d Frame index =%d  Interval=0x%x \r\n",
					  m_pProbeCommit->bFormatIndex, m_pProbeCommit->bFrameIndex, m_pProbeCommit->dwFrameInterval));
		break;
	case UVAC_CLASS_REQ_VS_COMMIT:

		set_probe_count = 0;

		if (m_pProbeCommit == &g_ProbeCommit[UVAC_VID_DEV_CNT_1]) {
			vidDevIdx = UVAC_VID_DEV_CNT_1;
		} else if (m_pProbeCommit == &g_ProbeCommit[UVAC_VID_DEV_CNT_2]) {
			vidDevIdx = UVAC_VID_DEV_CNT_2;
		} else if (m_pProbeCommit == &g_ProbeCommit[UVAC_VID_DEV_CNT_3]) {
			vidDevIdx = UVAC_VID_DEV_CNT_3;
		} else {
			DBG_ERR("m_pProbeCommit=0x%lX,&g_ProbeCommit[0]=0x%lX,&g_ProbeCommit[1]=0x%lX,&g_ProbeCommit[2]=0x%lX\r\n", (uintptr_t)m_pProbeCommit, (uintptr_t)&g_ProbeCommit[UVAC_VID_DEV_CNT_1], (uintptr_t)&g_ProbeCommit[UVAC_VID_DEV_CNT_2], (uintptr_t)&g_ProbeCommit[UVAC_VID_DEV_CNT_3]);
			break;
		}
		m_pProbeCommit->bFormatIndex = *(pData + 2);
		m_pProbeCommit->bFrameIndex = *(pData + 3);
		m_pProbeCommit->dwFrameInterval = *(pData + 4);
		m_pProbeCommit->dwFrameInterval |= *(pData + 5) << 8;
		m_pProbeCommit->dwFrameInterval |= *(pData + 6) << 16;
		m_pProbeCommit->dwFrameInterval |= *(pData + 7) << 24;

		m_pProbeCommit->wKeyFrameRate = *(pData + 8);
		m_pProbeCommit->wKeyFrameRate |= (*(pData + 9) << 8);
		m_pProbeCommit->wPFrameRate = *(pData + 10);
		m_pProbeCommit->wPFrameRate |= (*(pData + 11) << 8);
		m_pProbeCommit->wCompQuality = *(pData + 12);
		m_pProbeCommit->wCompQuality |= (*(pData + 13) << 8);
		m_pProbeCommit->wCompWindowSize = *(pData + 14);
		m_pProbeCommit->wCompWindowSize |= (*(pData + 15) << 8);
		m_pProbeCommit->wDelay = *(pData + 16);
		m_pProbeCommit->wDelay |= (*(pData + 17) << 8);
		m_pProbeCommit->dwMaxVideoFrameSize = *(pData + 18);
		m_pProbeCommit->dwMaxVideoFrameSize |= (*(pData + 19) << 8);
		m_pProbeCommit->dwMaxVideoFrameSize |= (*(pData + 20) << 16);
		m_pProbeCommit->dwMaxVideoFrameSize |= (*(pData + 21) << 24);
		m_pProbeCommit->dwMaxPayloadTransferSize = *(pData + 22);
		m_pProbeCommit->dwMaxPayloadTransferSize |= (*(pData + 23) << 8);
		m_pProbeCommit->dwMaxPayloadTransferSize |= (*(pData + 24) << 16);
		m_pProbeCommit->dwMaxPayloadTransferSize |= (*(pData + 25) << 24);

		if (gU3UvacUvcVer >= UVAC_UVC_VER_110) {
			m_pProbeCommit->dwClockFrequency = *(pData + 26);
			m_pProbeCommit->dwClockFrequency |= (*(pData + 27) << 8);
			m_pProbeCommit->dwClockFrequency |= (*(pData + 28) << 16);
			m_pProbeCommit->dwClockFrequency |= (*(pData + 29) << 24);
			m_pProbeCommit->bmFramingInfo = *(pData + 30);
			m_pProbeCommit->bPreferredVersion = *(pData + 31);
			m_pProbeCommit->bMinVersion = *(pData + 32);
			m_pProbeCommit->bMaxVersion = *(pData + 33);
		}

		if (gU3UvacUvcVer == UVAC_UVC_VER_150) {
			*pData++= m_pProbeCommit->bUsage;
			*pData++= m_pProbeCommit->bBitDepthLuma;
			*pData++= m_pProbeCommit->bmSettings;
			*pData++= m_pProbeCommit->bMaxNumberOfRefFramesPlus1;

			*pData++= (m_pProbeCommit->bmRateControlModes) & 0xFF;
			*pData++= (m_pProbeCommit->bmRateControlModes>>8) & 0xFF;

			*pData++= (m_pProbeCommit->bmLayoutPerStream[0]) & 0xFF;
			*pData++= (m_pProbeCommit->bmLayoutPerStream[0]>>8) & 0xFF;
			*pData++= (m_pProbeCommit->bmLayoutPerStream[1]) & 0xFF;
			*pData++= (m_pProbeCommit->bmLayoutPerStream[1]>>8) & 0xFF;
			*pData++= (m_pProbeCommit->bmLayoutPerStream[2]) & 0xFF;
			*pData++= (m_pProbeCommit->bmLayoutPerStream[2]>>8) & 0xFF;
			*pData++= (m_pProbeCommit->bmLayoutPerStream[3]) & 0xFF;
			*pData++= (m_pProbeCommit->bmLayoutPerStream[3]>>8) & 0xFF;
		}

		DbgMsg_UVCIO(("^B--UVC:[VS Commit control --SET_CUR],formatIdx=%d,frmIdx=%d,payload=%d\r\n", m_pProbeCommit->bFormatIndex, m_pProbeCommit->bFrameIndex, m_pProbeCommit->dwMaxPayloadTransferSize));


		if (0 == m_pProbeCommit->bFrameIndex) {
			DBG_ERR("Wrong FrameIdx=%d,m_pProbeCommit=0x%lx => Set2Previous, DEF=%d\r\n", m_pProbeCommit->bFrameIndex, (uintptr_t)m_pProbeCommit, UVC_VSFMT_DEF_FRM_IDX);
			UVAC_SetImageSize(UVC_VSFMT_PREV_FRM_IDX, vidDevIdx);
		} else if (UVAC_VID_RESO_MAX_CNT >= m_pProbeCommit->bFrameIndex) {
			DbgMsg_UVCIO(("SET_CUR,frmIdx=%d,vidDevIdx=%d,wid=%d,height=%d\r\n", m_pProbeCommit->bFrameIndex, vidDevIdx, gUvcVidResoAry[m_pProbeCommit->bFrameIndex - 1].width, gUvcVidResoAry[m_pProbeCommit->bFrameIndex - 1].height));
			UVAC_SetImageSize(m_pProbeCommit->bFrameIndex, vidDevIdx);
		} else {
			DBG_ERR("Wrong FrameIdx=%d,vidDevIdx=%d, Stall\r\n", m_pProbeCommit->bFrameIndex, vidDevIdx);
			bStall = TRUE;
			break;
		}

		UVC_DmpProbeData(3, m_pProbeCommit);

		if (m_pProbeCommit->dwFrameInterval) {
			UVAC_SetFrameRate(10000000 / m_pProbeCommit->dwFrameInterval, vidDevIdx);
		} else {
			DBG_ERR("Host Set FrameInterval=0 Fail. Set to %d fps, intrf=%d\r\n", UVC_FRMRATE_30, vidDevIdx);
			UVAC_SetFrameRate(UVC_FRMRATE_30, vidDevIdx);
		}

		if (UVAC_VID_DEV_CNT_MAX > vidDevIdx) {
			if (m_pProbeCommit->bFormatIndex == u3_h264_fmt_index[vidDevIdx]) {
				codecType = UVAC_VIDEO_FORMAT_H264;
			} else if (m_pProbeCommit->bFormatIndex == u3_mjpeg_fmt_index[vidDevIdx]) {
				codecType = UVAC_VIDEO_FORMAT_MJPG;
			} else if (m_pProbeCommit->bFormatIndex == u3_yuv_fmt_index) {
				codecType = UVAC_VIDEO_FORMAT_YUV;
			} else if (m_pProbeCommit->bFormatIndex == u3_nv12_fmt_index) {
				codecType = UVAC_VIDEO_FORMAT_NV12;
			} else if (m_pProbeCommit->bFormatIndex == u3_h265_fmt_index[vidDevIdx]) {
				codecType = UVAC_VIDEO_FORMAT_H265;
			} else {
				DBG_ERR("unknow format index=%d, h264=%d, mjpeg=%d, yuv=%d, nv12=%d, h265=%d\r\n", m_pProbeCommit->bFormatIndex, u3_h264_fmt_index[vidDevIdx], u3_mjpeg_fmt_index[vidDevIdx], u3_yuv_fmt_index, u3_nv12_fmt_index, u3_h265_fmt_index[vidDevIdx]);
			}
		}
		UVAC_SetCodec(codecType, vidDevIdx);
		if (u3_uvc_bulk_mode[vidDevIdx]) {
			UINT32 txfQueIdx;

			gUvcVidStart[vidDevIdx] = FALSE;
			if (vidDevIdx == UVAC_VID_DEV_CNT_1) {
				txfQueIdx = UVAC_TXF_QUE_V1;
			} else if(vidDevIdx == UVAC_VID_DEV_CNT_2) {
				txfQueIdx = UVAC_TXF_QUE_V2;
			} else {
				txfQueIdx = UVAC_TXF_QUE_V3;
			}

			//UVAC_RemoveTxfInfo(txfQueIdx);

			//usb3dev_abortEndpoint(UVAC_USB_EP[txfQueIdx]);

			_U3UVC_Stop(vidDevIdx);

			UVAC_RemoveTxfInfo(txfQueIdx);

			usb3dev_abortEndpoint(UVAC_USB_EP[txfQueIdx]);

			gUVCIsoinTxfUnitSize[vidDevIdx] = m_pProbeCommit->dwMaxPayloadTransferSize;

			gUvcVidStart[vidDevIdx] = TRUE;
			UVAC_Start(vidDevIdx);
		}
		break;
	case UVAC_CLASS_REQ_AC_COMMIT_SET_CUR:
		for (i = 0; i < len; i++) {
			tmpSampleRate += (*(pData + i) << (8 * i));
		}

		DbgMsg_UVCIO(("^GUAC--SET_CUR,gUacSampleRate=0x%x,len=%d,usblen=%d\r\n", tmpSampleRate, len, m_RequestLen));
		if ((tmpSampleRate > UAC_FREQUENCY_48K) || (tmpSampleRate < UAC_FREQUENCY_08K)) {
			DBG_ERR("Aud-Sample-Rate: 0x%x, prev=0x%x\r\n", tmpSampleRate, gUacSampleRate);
			bStall = TRUE;
		} else {
			gUacSampleRate = tmpSampleRate;
		}
#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
		{
			i = 0;
			while (i < m_RequestLen) {
				DBG_DUMP("0x%x ", *pData++);
				i++;
				if ((i % 16) == 0) {
					DBG_DUMP("\r\n");
				}
			}
			DBG_DUMP("\r\n");
		}
#endif
		break;
	case UVAC_CLASS_REQ_AC_COMMIT_SET_RES:
		gUACRes = *pData ;
		gUACRes += *(pData + 1) << 8;
		break;
	case UVAC_CLASS_REQ_CDC:
		gfpCdcPstnReqCB(m_ComID, REQ_SET_LINE_CODING, pData, &m_RequestLen);
		break;
	case UVAC_CLASS_REQ_XU:
		if (FALSE == gUvacEUVendCmdCB[m_uiXUControlCode](m_uiXUControlCode, SET_CUR, pData, &gUvacVendDataLen)) {
			bStall = TRUE;
		}
		break;
	case UVAC_CLASS_REQ_AC_MUTE: {
			BOOL tmpMute;

			tmpMute = *pData;

			if (tmpMute > 1) {
				DBG_ERR("Aud-Mute: 0x%x, prev=0x%x\r\n", tmpMute, gUacMute);
				bStall = TRUE;
			} else {
				gUacMute = tmpMute;

				if (gUacMute) {
					if (gUacSetVolCB)
						gUacSetVolCB(1);
					else
						DbgMsg_UVCIO(("gUacSetVolCB is NULL\r\n"));
				} else {
					if (gUacSetVolCB)
						gUacSetVolCB(u3_uac_vol_cur);
					else
						DbgMsg_UVCIO(("gUacSetVolCB is NULL\r\n"));
				}

				DbgMsg_UVCIO(("UAC AS Mute control:Mute=%d\r\n", gUacMute));
			}
		}
		break;
	case UVAC_CLASS_REQ_AC_VOL: {
			DbgMsg_UVCIO(("^BUAC:[AS Vol control --SET_CUR]\r\n"));
			BOOL tmpVol;

			tmpVol = *pData + (*(pData + 1) << 8);

			u3_uac_vol_cur = tmpVol;

			if (gUacMute == 0) {
				if (gUacSetVolCB)
					gUacSetVolCB(u3_uac_vol_cur);
				else
					DbgMsg_UVCIO(("gUacSetVolCB is NULL\r\n"));
			}
			DbgMsg_UVCIO(("UAC AS Vol control:Volume=%d\r\n", u3_uac_vol_cur));
		}
		break;
	case UVAC_CLASS_REQ_AC_FREQ_SET_CUR:
		{
			DbgMsg_UVCIO(("^BUAC:[AC Freq --SET_CUR]\r\n"));

			DbgMsg_UVCIO(("UAC AC Freq control\r\n"));
		}
		break;
	case UVAC_CLASS_REQ_CT_SET_CUR:
		if (g_fpU3UvcCT_CB(ControlCode, b_request, pData, &gUvacVendDataLen)) {
			DbgMsg_UVCIO(("VendCmd Ok, retLen=%d,ctrl=%d, request=0x%x, p_data=0x%x\r\n", gUvacVendDataLen, ControlCode, b_request, pData));
			DbgMsg_UVCIO(("  ==> *p_data=0x%x\r\n", *pData));
			//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, p_data=0x%x", gUvacVendDataLen, ControlCode, CS, p_data);
			//DBG_DUMP("==> *p_data=0x%x\r\n", *p_data);
			if (gUvacVendDataLen) {
				if (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) {
					len = USB_UVAC_CLS_DATABUF_LEN;
					DBG_ERR("pDataLen(%d) should <= 64\r\n");
				} else {
					len = gUvacVendDataLen;
				}
				//uiRetBufAddr = (uintptr_t) pData;
				//uiRetSize = 0;
			}
		} else {
			bStall = TRUE;
			//uiRetBufAddr = (uintptr_t) pData;
			//uiRetSize = 0;
			//DBG_IND("VendCmd Fail, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, pData);
			//usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
		}

		break;
	case UVAC_CLASS_REQ_PU_SET_CUR:
		if (g_fpU3UvcPU_CB(ControlCode, b_request, pData, &gUvacVendDataLen)) {
			DbgMsg_UVCIO(("VendCmd Ok, retLen=%d,ctrl=%d, request=0x%x, p_data=0x%x\r\n", gUvacVendDataLen, ControlCode, b_request, pData));
			DbgMsg_UVCIO(("  ==> *p_data=0x%x\r\n", *pData));
			//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, p_data=0x%x", gUvacVendDataLen, ControlCode, CS, p_data);
			//DBG_DUMP("==> *p_data=0x%x\r\n", *p_data);
			if (gUvacVendDataLen) {
				if (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) {
					len = USB_UVAC_CLS_DATABUF_LEN;
					DBG_ERR("pDataLen(%d) should <= 64\r\n");
				} else {
					len = gUvacVendDataLen;
				}
				//uiRetBufAddr = (uintptr_t) pData;
				//uiRetSize = 0;
			}
		} else {
			bStall = TRUE;
			//uiRetBufAddr = (uintptr_t) pData;
			//uiRetSize = 0;
			//DBG_IND("VendCmd Fail, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, pData);
			//usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
		}

		break;
	case UVAC_CLASS_REQ_ACO_COMMIT_SET_CUR:
		for (i = 0; i < len; i++) {
			tmpSampleRate += (*(pData + i) << (8 * i));
		}

		DbgMsg_UVCIO(("^GUAC--SET_CUR,gUacSampleRate=0x%x,len=%d,usblen=%d\r\n", tmpSampleRate, len, m_RequestLen));
		if ((tmpSampleRate > UAC_FREQUENCY_48K) || (tmpSampleRate < UAC_FREQUENCY_08K)) {
			DBG_ERR("Aud-Sample-Rate: 0x%x, prev=0x%x\r\n", tmpSampleRate, gUacSampleRate);
			bStall = TRUE;
		} else {
			gUacSampleRate = tmpSampleRate;
		}
#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
		{
			i = 0;
			while (i < m_RequestLen) {
				DBG_DUMP("0x%x ", *pData++);
				i++;
				if ((i % 16) == 0) {
					DBG_DUMP("\r\n");
				}
			}
			DBG_DUMP("\r\n");
		}
#endif
		break;
	case UVAC_CLASS_REQ_ACO_COMMIT_SET_RES:
		gUACRes = *pData ;
		gUACRes += *(pData + 1) << 8;
		break;
	case UVAC_CLASS_REQ_ACO_MUTE: {
			BOOL tmpMute;

			tmpMute = *pData;

			if (tmpMute > 1) {
				DBG_ERR("Aud-Mute: 0x%x, prev=0x%x\r\n", tmpMute, gUacRxMute);
				bStall = TRUE;
			} else {
				gUacRxMute = tmpMute;

				if (gUacRxMute) {
					if (gUacRxSetVolCB)
						gUacRxSetVolCB(1);
					else
						DbgMsg_UVCIO(("gUacSetVolCB is NULL\r\n"));
				} else {
					if (gUacRxSetVolCB)
						gUacRxSetVolCB(u3_uac_rx_vol_info.vol_def);
					else
						DbgMsg_UVCIO(("gUacSetVolCB is NULL\r\n"));
				}

				DbgMsg_UVCIO(("UAC AS Mute control:Mute=%d\r\n", gUacRxMute));
			}
		}
		break;
	case UVAC_CLASS_REQ_ACO_VOL: {
			DbgMsg_UVCIO(("^BUAC:[AS Vol control --SET_CUR]\r\n"));
			BOOL tmpVol;

			tmpVol = *pData + (*(pData + 1) << 8);

			u3_uac_rx_vol_info.vol_def = tmpVol;

			if (gUacRxMute == 0) {
				if (gUacRxSetVolCB)
					gUacRxSetVolCB(u3_uac_rx_vol_info.vol_def);
				else
					DbgMsg_UVCIO(("gUacSetVolCB is NULL\r\n"));
			}
			DbgMsg_UVCIO(("UAC AS Vol control:Volume=%d\r\n", u3_uac_rx_vol_info.vol_def));
		}
		break;
	case UVAC_CLASS_REQ_ACO_FREQ_SET_CUR:
		{
			DbgMsg_UVCIO(("^BUAC:[AC Freq --SET_CUR]\r\n"));

			gUvacAudRxSampleRate[0] = *(pData);
			gUvacAudRxSampleRate[0] |= (*(pData + 1) << 8);
			gUvacAudRxSampleRate[0] |= (*(pData + 2) << 16);

			DbgMsg_UVCIO(("UAC AC Freq control\r\n"));
		}
		break;
	case UVAC_CLASS_REQ_HID_SET:
		if (g_u3_hid_info.cb(b_request, pRequest->w_value, pData, &gUvacVendDataLen)) {
			//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, p_data=0x%x", gUvacVendDataLen, ControlCode, CS, p_data);
			//DBG_DUMP("==> *p_data=0x%x\r\n", *p_data);
			if (gUvacVendDataLen) {
				if (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) {
					//len = USB_UVAC_CLS_DATABUF_LEN;
					DBG_ERR("pDataLen(%d) should <= 64\r\n");
				} else {
					//len = gUvacVendDataLen;
				}
			}
		} else {
			bStall = TRUE;
		}
		break;
	case UVAC_CLASS_REQ_EU_SET_CUR:
		{
			BOOL ret = FALSE;

			if (g_fpU3UvcXU_CB) {
				ret = g_fpU3UvcXU_CB(ControlCode, b_request, pData, &gUvacVendDataLen);
			} else if (u3_eu_desc[0].eu_cb) {
				ret = u3_eu_desc[0].eu_cb(ControlCode, b_request, pData, &gUvacVendDataLen);
			}

			if (ret) {
				DbgMsg_UVCIO(("VendCmd Ok, retLen=%d,ctrl=%d, request=0x%x, p_data=0x%x\r\n", gUvacVendDataLen, ControlCode, b_request, pData));
				DbgMsg_UVCIO(("  ==> *p_data=0x%x\r\n", *pData));
				//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, p_data=0x%x", gUvacVendDataLen, ControlCode, CS, p_data);
				//DBG_DUMP("==> *p_data=0x%x\r\n", *p_data);
				if (gUvacVendDataLen) {
					if (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) {
						len = USB_UVAC_CLS_DATABUF_LEN;
						DBG_ERR("pDataLen(%d) should <= 64\r\n");
					} else {
						len = gUvacVendDataLen;
					}
					//uiRetBufAddr = (uintptr_t) pData;
					//uiRetSize = 0;
				}
			} else {
				bStall = TRUE;
				//uiRetBufAddr = (uintptr_t) pData;
				//uiRetSize = 0;
				//DBG_IND("VendCmd Fail, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, pData);
				//usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			}
		}
		break;
	case UVAC_CLASS_REQ_EU2_SET_CUR:
		{
			BOOL ret = FALSE;

			if (u3_eu_desc[1].eu_cb) {
				ret = u3_eu_desc[1].eu_cb(ControlCode, b_request, pData, &gUvacVendDataLen);
			}

			if (ret) {
				DbgMsg_UVCIO(("VendCmd Ok, retLen=%d,ctrl=%d, request=0x%x, p_data=0x%x\r\n", gUvacVendDataLen, ControlCode, b_request, pData));
				DbgMsg_UVCIO(("  ==> *p_data=0x%x\r\n", *pData));
				//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, p_data=0x%x", gUvacVendDataLen, ControlCode, CS, p_data);
				//DBG_DUMP("==> *p_data=0x%x\r\n", *p_data);
				if (gUvacVendDataLen) {
					if (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) {
						len = USB_UVAC_CLS_DATABUF_LEN;
						DBG_ERR("pDataLen(%d) should <= 64\r\n");
					} else {
						len = gUvacVendDataLen;
					}
					//uiRetBufAddr = (uintptr_t) pData;
					//uiRetSize = 0;
				}
			} else {
				bStall = TRUE;
				//uiRetBufAddr = (uintptr_t) pData;
				//uiRetSize = 0;
				//DBG_IND("VendCmd Fail, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, pData);
				//usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			}
		}
		break;

	case UVAC_CLASS_REQ_EU_ARRAY_SET_CUR:
		{
			BOOL ret = FALSE;

			if (u3_eu_desc_array->eu_desc[eu_id].eu_cb) {
				ret = u3_eu_desc_array->eu_desc[eu_id].eu_cb(ControlCode, b_request, pData, &gUvacVendDataLen);
			}

			if (ret) {
				DbgMsg_UVCIO(("VendCmd Ok, retLen=%d,ctrl=%d, request=0x%x, p_data=0x%x\r\n", gUvacVendDataLen, ControlCode, b_request, pData));
				DbgMsg_UVCIO(("  ==> *p_data=0x%x\r\n", *pData));
				//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, p_data=0x%x", gUvacVendDataLen, ControlCode, CS, p_data);
				//DBG_DUMP("==> *p_data=0x%x\r\n", *p_data);
				if (gUvacVendDataLen) {
					if (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) {
						len = USB_UVAC_CLS_DATABUF_LEN;
						DBG_ERR("pDataLen(%d) should <= 64\r\n");
					} else {
						len = gUvacVendDataLen;
					}
					//uiRetBufAddr = (uintptr_t) pData;
					//uiRetSize = 0;
				}
			} else {
				bStall = TRUE;
				//uiRetBufAddr = (uintptr_t) pData;
				//uiRetSize = 0;
				//DBG_IND("VendCmd Fail, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, pData);
				//usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
			}
		}
		break;
	default:
		break;
	}

	usb3dev_handleSetupResult(bStall, 0, 0);
}
void UVC_XU_VendorCmd(UINT32 ControlCode, UINT8 CS, UINT8 *pData)
{
	UINT8 vendCBRet;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;
	DbgMsg_UVCIO(("VendCmd: ctrl=%d, cs=0x%x, pData=0x%x\r\n", ControlCode, CS, pData));
	//DBG_DUMP("VendCmd: ctrl=%d, cs=0x%x, pData=0x%x\r\n", ControlCode, CS, pData);

	//ControlCode: 1, 2, ..., 8
	if ((UVAC_EU_VENDCMD_CNT > ControlCode) && (gUvacEUVendCmdCB[ControlCode])) {
		if (SET_CUR == CS) {
			DbgMsg_UVCIO(("^BUVC:[EU --SET_CUR]\r\n"));
			gUvacVendDataLen = USB_UVAC_CLS_DATABUF_LEN;
			memset(pData, 0, USB_UVAC_CLS_DATABUF_LEN);
			m_uiXUControlCode = ControlCode;
			m_RequestType = UVAC_CLASS_REQ_XU;
			m_RequestLen = gUvacVendDataLen;
			uiRetBufAddr = (uintptr_t) pData;
			uiRetSize = m_RequestLen;
		} else {
			vendCBRet = gUvacEUVendCmdCB[ControlCode](ControlCode, CS, pData, &gUvacVendDataLen);
			if (TRUE == vendCBRet) {
				DbgMsg_UVCIO(("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, pData));
				DbgMsg_UVCIO(("  ==> *pData=0x%x\r\n", *pData));
				//DBG_DUMP("VendCmd Ok, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x", gUvacVendDataLen, ControlCode, CS, pData);
				//DBG_DUMP("==> *pData=0x%x\r\n", *pData);
				gUvacVendDataLen = (gUvacVendDataLen > USB_UVAC_CLS_DATABUF_LEN) ? USB_UVAC_CLS_DATABUF_LEN : gUvacVendDataLen;
				if (SET_CUR != CS) {
					uiRetBufAddr = (uintptr_t)pData;
					uiRetSize = gUvacVendDataLen;
				}
			} else {
				DBG_IND("VendCmd Fail, retLen=%d,ctrl=%d, cs=0x%x, pData=0x%x\r\n", gUvacVendDataLen, ControlCode, CS, pData);
				bStall = TRUE;
			}
		}
	} else {
		DBG_IND("gUvacEUVendCmdCB[%d] NULL, CS=%d, pData=0x%x\r\n", ControlCode, CS, pData);
		bStall = TRUE;
	}

	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
}
void U3UvacStdRequestHandler(uintptr_t uiEvent)
{
	UINT8 nInterface;
	UINT8 desc_type;
	UINT16 desc_len = 0;
	uintptr_t uiRetBufAddr = 0;
	PUSB_DEVICE_REQUEST    pRequest;

	pRequest = (PUSB_DEVICE_REQUEST)uiEvent;
	desc_type = pRequest->w_value >> 8;
	nInterface = pRequest->w_index & 0xFF;
	DbgMsg_UVC(("++%s:req=0x%x, desc_type=0x%X, intr=%d, len=%d\r\n", __func__,
													pRequest->b_request,
													desc_type,
													nInterface,
													pRequest->w_length));
	if (g_u3_hid_info.en) {
		//get reporter descriptor
		if (desc_type == g_u3_hid_info.hid_desc.bDescriptorType && nInterface == gU3UvacIntfIdx_HID) {
			desc_len = g_u3_hid_info.hid_desc.wDescriptorLength;
			if (g_u3_hid_info.p_report_desc) {
				if (desc_len) {
					uiRetBufAddr = (uintptr_t)g_u3_hid_info.p_report_desc;
					usb3dev_handleSetupResult(FALSE, uiRetBufAddr, desc_len);
					return;
				}
			}
		} else if (desc_type == g_u3_hid_info.hid_desc.bHidDescType && nInterface == gU3UvacIntfIdx_HID) {
		//get hid descriptor
			desc_len = g_u3_hid_info.hid_desc.bLength;
			if (desc_len) {
				uiRetBufAddr = (uintptr_t)&g_u3_hid_info.hid_desc;
				usb3dev_handleSetupResult(FALSE, uiRetBufAddr, desc_len);
				return;
			}
		}
	}

	usb3dev_handleSetupResult(TRUE, 0, 0);
	DbgMsg_UVC(("--%s:req=0x%x, desc_type=0x%X, intr=%d, len=%d\r\n", __func__,
													pRequest->b_request,
													desc_type,
													nInterface,
													pRequest->w_length));
}
#endif


