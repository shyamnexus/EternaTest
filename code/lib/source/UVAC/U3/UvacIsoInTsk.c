#include "UvacDbg.h"
#include "UvacIsoInTsk.h"
#include "UvacVideoTsk.h"
#include "UvacDesc.h"
#include "UVAC.h"
#include "UvacID.h"
#include "hd_type.h"
#include <stdio.h>
#include <string.h>
#include "hd_common.h"

static void uvac_wai_sem(const char * calledby) {
	//printf("wait_sem called by %s\r\n", calledby);
	wai_sem(SEMID_UVC_QUEUE);
}

static void uvac_rel_sem(const char * calledby) {
	//printf("rel_sem called by %s\r\n", calledby);
	sig_sem(SEMID_UVC_QUEUE);
}

#define loc_cpu() uvac_wai_sem(__FUNCTION__)
#define unl_cpu() uvac_rel_sem(__FUNCTION__)
#define UVC_WRITE_PERF 0

//====== global variable ======
#if (1)
const USB_EP UVAC_USB_INTR_EP[UVAC_EP_INTR_MAX] = {USB_EP5, USB_EP6, USB_EP4};
#endif
const USB_EP UVAC_USB_EP[UVAC_TXF_QUE_MAX] = {USB_EP1, USB_EP3, USB_EP2, USB_EP2, USB_EP4, USB_EP7};
const USB_EP UVAC_USB_RX_EP[UVAC_RXF_QUE_MAX] = {USB_EP7};
UINT32 gUvcMaxTxfSizeForUsb[UVAC_TXF_QUE_MAX] = {USB_MAX_TXF_SIZE_LIMIT, USB_MAX_TXF_SIZE_LIMIT, USB_MAX_TXF_SIZE_LIMIT, USB_MAX_TXF_SIZE_LIMIT, USB_MAX_TXF_SIZE_LIMIT};
UVAC_TXF_INFO gUvacTxfQue[UVAC_TXF_QUE_MAX][UVAC_TXF_MAX_QUECNT];//4*60*16=4800=4.69k
UVC_ISOIN_TXF_STATE gUvacIsoTxfState = UVC_ISOIN_TXF_STATE_STOP;
UINT32 gUvacTxfQueCurCnt[UVAC_TXF_QUE_MAX];
UINT32 gUvacTxfQueFrontIdx[UVAC_TXF_QUE_MAX];
UINT32 gUvacTxfQueRearIdx[UVAC_TXF_QUE_MAX];
UINT32 gUvcUsbDMAAbord = FALSE;
UINT32 gUacUsbDMAAbord = FALSE;
UINT32 gUvcNoVidStrm = FALSE;
extern FLGPTN gU3UvacRunningFlag[UVAC_VID_DEV_CNT_MAX];
extern UINT32 gUvacAudSampleRate[UVAC_AUD_SAMPLE_RATE_MAX_CNT];
extern UINT32 gUacChNum;
extern U3DEV_SPEED g_usb_speed;

USB_EP msdc_in_ep  = MSDC_IN_EP;
USB_EP msdc_out_ep = MSDC_OUT_EP;

_ALIGNED(4) UINT8 gUvcIsoInHdr[12] = {0x0C, 0x8F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#if (UVAC_CONTROLLER == UVAC_CONTROL_U3)
//ben to do
#define UVAC_MAX_VID_ISOIN_PKTNO    (UVAC_MAX_PAYLOAD_FRAME_SIZE/USB_MAX_TXF_SIZE_ONE_PACKET)
#define UVAC_SS_MAX_VID_ISOIN_PKTNO (UVAC_MAX_PAYLOAD_FRAME_SIZE/USB_SS_MAX_TXF_SIZE_ONE_PACKET)
//U3DEV_ISOPKT    m_UvcIsoPkt[UVAC_VID_DEV_CNT_MAX][UVAC_MAX_VID_ISOIN_PKTNO] = {0};
//U3DEV_ISOPKT    m_UvcSSIsoPkt[UVAC_VID_DEV_CNT_MAX][UVAC_SS_MAX_VID_ISOIN_PKTNO] = {0};
//U3DEV_ISODATA   m_UvcIsoData[UVAC_VID_DEV_CNT_MAX] = {0};
#define UVAC_MAX_AUD_FRAME_SIZE     0x19000//100KB
#define UVAC_MAX_AUD_ISOIN_PKTNO    (UVAC_MAX_AUD_FRAME_SIZE/UVC_ISOIN_AUD_HS_PACKET_SIZE)
//U3DEV_ISOPKT    m_UacIsoPkt[UVAC_VID_DEV_CNT_MAX][UVAC_MAX_AUD_ISOIN_PKTNO] = {0};
//U3DEV_ISODATA   m_UacIsoData[UVAC_VID_DEV_CNT_MAX] = {0};
#endif
//----------------------
extern UINT32 gUacAudStart[UVAC_AUD_DEV_CNT_MAX];
extern UINT32 gUvcVidStart[UVAC_VID_DEV_CNT_MAX];
extern UINT32 gUVCIsoinTxfUnitSize[UVAC_VID_DEV_CNT_MAX];
extern UINT8  gUvacStillImgTrigSts;
extern UINT32 gUvcHWPayload[UVAC_VID_DEV_CNT_MAX];
extern UINT32 gUvacChannel;
extern UINT32 gHsUvacChannel;
extern BOOL gUvacCdcEnabled[CDC_COM_MAX_NUM];
extern BOOL gUvacMtpEnabled;

#if (ISF_LATENCY_DEBUG)
typedef struct _UVAC_TIMESTAMP_INFO_ {
	UINT64 timestamp;
	UINT64 uvc_timestamp_start;
	UINT64 uvc_timestamp_end;
} UVAC_TIMESTAMP_INFO;
#define ISF_UVAC_MAX_ISF_DEBUG_NUM  60
static UVAC_TIMESTAMP_INFO g_u3_isf_uvac_timestamp[ISF_UVAC_MAX_ISF_DEBUG_NUM];
static UINT32 g_u3_isf_uvac_timestamp_idx = 0;
static BOOL u3_is_isf_dump = FALSE;

#endif

#if (ISF_AUDIO_LATENCY_DEBUG)
#define ISF_UVAC_AUD_MAX_ISF_DEBUG_NUM  90
typedef struct _UVAC_AUD_TIMESTAMP_INFO_ {
	UINT64 timestamp;
	UINT64 uvc_timestamp_start;
	UINT64 uvc_timestamp_write;
	UINT64 uvc_timestamp_end;
	UINT32 uvc_slice_size;
} UVAC_AUD_TIMESTAMP_INFO;
static UVAC_AUD_TIMESTAMP_INFO g_u3_isf_uvac_aud_timestamp[ISF_UVAC_AUD_MAX_ISF_DEBUG_NUM];
static UINT32 g_u3_isf_uvac_aud_timestamp_idx = 0;
static BOOL u3_is_aud_isf_dump = FALSE;
#endif

static void _rel_vdo_frm(UVAC_VID_DEV_CNT thisVidIdx)
{
	if (thisVidIdx == UVAC_VID_DEV_CNT_1) {
		vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V1);
	} else if (thisVidIdx == UVAC_VID_DEV_CNT_2) {
		vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V2);
	} else {
		vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V3);
	}
}
#if (_UVC_DBG_LVL_ > _UVC_DBG_CHK_)
void UVAC_DbgDmp_TxfInfoByQue(UINT32 queIdx)
{
	UINT32 i = 0;
	//UVAC_TXF_INFO *pTxfInfo;
	DBG_DUMP("%s ==>\r\n", __func__);
	DbgMsg_UVCIO(("Dmp Que[%d]:f=0x%x,r=0x%x,cnt=%d\r\n", queIdx, gUvacTxfQueFrontIdx[queIdx], gUvacTxfQueRearIdx[queIdx], gUvacTxfQueCurCnt[queIdx]));
	for (i = 0; i < UVAC_TXF_MAX_QUECNT; i++) {
		//pTxfInfo = &gUvacTxfQue[queIdx][i];
		//DbgMsg_UVCIO(("[%d]=0x%x,0x%x,%d,ep=%d,0x%x\r\n",i,pTxfInfo->sAddr,pTxfInfo->size,pTxfInfo->txfCnt,pTxfInfo->usbEP,pTxfInfo));
	}
	DBG_DUMP("<===========\r\n");
}
void UVAC_DbgDmp_ChkIsoInQueEmpty(UINT32 queIdx)
{
	DBG_DUMP("%s ==>\r\n", __func__);
	switch (queIdx) {
	case UVAC_TXF_QUE_V1:
		if (gUvcVidStart[0]) {
			DBG_DUMP("Txf[%d] EMP\r\n", queIdx);
		}
		break;
	case UVAC_TXF_QUE_V2:
		if (gUvcVidStart[1]) {
			DBG_DUMP("Txf[%d] EMP\r\n", queIdx);
		}
		break;
	case UVAC_TXF_QUE_A1:
		if (gUacAudStart[0]) {
			DBG_DUMP("Txf[%d] EMP\r\n", queIdx);
		}
		break;
	case UVAC_TXF_QUE_A2:
		if (gUacAudStart[1]) {
			DBG_DUMP("Txf[%d] EMP\r\n", queIdx);
		}
		break;
	case UVAC_TXF_QUE_IMG:
		DBG_DUMP("ImgTxf[%d] EMP\r\n", queIdx);
		break;
	default:
		DBG_DUMP("Unknow Que=%d\r\n", queIdx);
		break;
	}
	DBG_DUMP("<===========\r\n");
}
void UVAC_DbgDmp_TxfEPInfo(void)
{
	UINT32 i = 0;
	DBG_DUMP("%s:HWPayload=%d,%d ==>\r\n", __func__, gUvcHWPayload[UVAC_VID_DEV_CNT_1], gUvcHWPayload[UVAC_VID_DEV_CNT_2]);
	for (i = 0; i < UVAC_TXF_QUE_MAX; i++) {
		DBG_DUMP("EP[%d]=0x%X, txf-size-limit=0x%x\r\n", i, UVAC_USB_EP[i], gUvcMaxTxfSizeForUsb[i]);
	}
	DBG_DUMP("<===========\r\n");
}
void UVAC_DbgDmp_ChkTxfQueAlmostFull(UINT32 queIdx, UINT32 level, BOOL dmpFirst, BOOL dmpQue)
{
	DBG_DUMP("%s:Que[%d]=%d,max=%d,dmp=%d,%d ==>\r\n", __func__, queIdx, level, UVAC_TXF_MAX_QUECNT, dmpFirst, dmpQue);
	if (gUvacTxfQueCurCnt[queIdx] > level) {
		UINT32 itemIdx = 0;
		UVAC_TXF_INFO *pTxfInfo;
		DBG_DUMP("  Que[%d]=%d > %d \r\n", queIdx, gUvacTxfQueCurCnt[queIdx], level);
		if (dmpFirst) {
			itemIdx = gUvacTxfQueFrontIdx[queIdx];
			pTxfInfo = &gUvacTxfQue[queIdx][itemIdx];
			DBG_DUMP("EP[%d]=%d,queCnt=%d,size=0x%x,txfCnt=%d,txfLimit=0x%x\r\n", queIdx, UVAC_USB_EP[queIdx], gUvacTxfQueCurCnt[queIdx], pTxfInfo->size, pTxfInfo->txfCnt, gUvcMaxTxfSizeForUsb[queIdx]);
		}
		if (dmpQue) {
			UVAC_DbgDmp_TxfInfoByQue(queIdx);
		}
	}
	DBG_DUMP("<===========\r\n");
}
#else
void UVAC_DbgDmp_TxfInfoByQue(UINT32 queIdx) {}
void UVAC_DbgDmp_ChkIsoInQueEmpty(UINT32 queIdx) {}
void UVAC_DbgDmp_TxfEPInfo(void) {}
void UVAC_DbgDmp_ChkTxfQueAlmostFull(UINT32 queIdx, UINT32 level, BOOL dmpFirst, BOOL dmpQue) {}
#endif

static __inline UINT32 UVAC_GetTxfQueIdx(UINT8 ep)
{
	if (UVAC_USB_EP[UVAC_TXF_QUE_V1] == ep) {
		return UVAC_TXF_QUE_V1;
	} else if (UVAC_USB_EP[UVAC_TXF_QUE_V2] == ep) {
		return UVAC_TXF_QUE_V2;
	} else if (UVAC_USB_EP[UVAC_TXF_QUE_V3] == ep) {
		return UVAC_TXF_QUE_V3;
	} else if (UVAC_USB_EP[UVAC_TXF_QUE_A1] == ep) {
		return UVAC_TXF_QUE_A1;
	} else if (UVAC_USB_EP[UVAC_TXF_QUE_A2] == ep) {
		return UVAC_TXF_QUE_A2;
	} else if (UVAC_USB_EP[UVAC_TXF_QUE_IMG] == ep) {
		return UVAC_TXF_QUE_IMG;
	} else {
		DBG_ERR("^RUnknown EP=%d,uvac channel=%d, hs channel=%d\r\n", ep, gUvacChannel, gHsUvacChannel);
		return UVAC_TXF_QUE_MAX;
	}
}

static BOOL UVAC_IsoTxDbg(UINT32 queIdx, UINT32 flag)
{
	if (((1 << (queIdx + 8)) & m_uiUvacDbgIso) && (flag & m_uiUvacDbgIso)) {
		return TRUE;
	} else {
		return FALSE;
	}

}

static void UVAC_UpdateTxfQue(UVAC_TXF_INFO *pTxfInfo, UVAC_TXF_QUE queIdx)
{
	if (pTxfInfo) {
		DbgMsg_UVCIO(("+[%d]=%d,f=%d,%d\r\n", queIdx, gUvacTxfQueCurCnt[queIdx], gUvacTxfQueFrontIdx[queIdx], gUvacTxfQueRearIdx[queIdx]));
		loc_cpu();
		if (gUvacTxfQueCurCnt[queIdx] > 0) {
			gUvacTxfQueCurCnt[queIdx]--;
		}
		gUvacTxfQueFrontIdx[queIdx] = ((gUvacTxfQueFrontIdx[queIdx] + 1) % UVAC_TXF_MAX_QUECNT);
		unl_cpu();
		DbgMsg_UVCIO(("-[%d]=%d,f=%d,%d\r\n", queIdx, gUvacTxfQueCurCnt[queIdx], gUvacTxfQueFrontIdx[queIdx], gUvacTxfQueRearIdx[queIdx]));
	} else {
		DBG_ERR("^R%s pTxfInfo NULL\r\n", __func__);
	}
}
#if 0
void UVAC_IsoInTxfStateMachine(UVC_ISOIN_TXF_ACT txfAct)
{
	DbgMsg_UVCIO(("IsoSts:%d,prvSts=%d,abort=%d,NovidSrm=%d\r\n", txfAct, gUvacIsoTxfState, gUvcUsbDMAAbord, gUvcNoVidStrm));
	if (UVC_ISOIN_TXF_ACT_START == txfAct) {
		//set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_START);
		set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_TXF);
	} else if (UVC_ISOIN_TXF_ACT_TXF == txfAct) {
		set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_TXF);
	} else if (UVC_ISOIN_TXF_ACT_STOP == txfAct) {
		set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_STOP);
	}
}
#endif
void UVAC_ResetTxfPara(void)
{
	DbgMsg_UVC(("+%s:\r\n", __func__));

	memset((void *)&gUvacTxfQue[0][0], 0, (UVAC_TXF_QUE_MAX * UVAC_TXF_MAX_QUECNT * sizeof(UVAC_TXF_INFO)));
	gUvacIsoTxfState = UVC_ISOIN_TXF_STATE_STOP;
	memset((void *)&gUvacTxfQueCurCnt[0], 0, sizeof(UINT32)*UVAC_TXF_QUE_MAX);
	memset((void *)&gUvacTxfQueFrontIdx[0], 0, sizeof(UINT32)*UVAC_TXF_QUE_MAX);
	memset((void *)&gUvacTxfQueRearIdx[0], 0, sizeof(UINT32)*UVAC_TXF_QUE_MAX);
	memset((void *)&gUVCIsoinTxfUnitSize[0], 0, sizeof(UINT32)*UVAC_VID_DEV_CNT_MAX);
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
	if (gUvacChannel > UVAC_CHANNEL_1V1A && gUvacCdcEnabled[CDC_COM_1ST]) {
		gUVCIsoinTxfUnitSize = UVC_USB_FIFO_UNIT_SIZE;
	} else if (gUvacChannel > UVAC_CHANNEL_1V1A && gUvacMtpEnabled) {
		gUVCIsoinTxfUnitSize = UVC_USB_FIFO_UNIT_SIZE;
	} else {
		gUVCIsoinTxfUnitSize = UVC_ISOIN_HS_PACKET_SIZE * UVC_ISOIN_BANDWIDTH;    //must be > PAYLOAD_LEN
	}

	if (gUVCIsoinTxfUnitSize <= PAYLOAD_LEN) {
		DBG_ERR("TxfUnitSize=%d <= %d Error !!\r\n", gUVCIsoinTxfUnitSize, PAYLOAD_LEN);
	}
#endif
	gUvcMaxTxfSizeForUsb[UVAC_TXF_QUE_V1] = USB_MAX_TXF_SIZE_LIMIT;
	if (u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_1]) {
		gUVCIsoinTxfUnitSize[UVAC_VID_DEV_CNT_1] = u3_uvc_bulk_payload_size;
	}

	if (gUvacChannel > UVAC_CHANNEL_1V1A || gHsUvacChannel > UVAC_CHANNEL_1V1A) {
		gUvcMaxTxfSizeForUsb[UVAC_TXF_QUE_V2] = USB_MAX_TXF_SIZE_LIMIT;
		if (u3_uvc_bulk_mode[UVAC_VID_DEV_CNT_2]) {
			gUVCIsoinTxfUnitSize[UVAC_VID_DEV_CNT_2] = u3_uvc_bulk_payload_size;
		}
	}
	DbgMsg_UVC(("-%s:HWPayL=%d,%d,txfS=0x%x,0x%x,unitS=0x%x\r\n", __func__, gUvcHWPayload[UVAC_VID_DEV_CNT_1], gUvcHWPayload[UVAC_VID_DEV_CNT_2], gUvcMaxTxfSizeForUsb[0], gUvcMaxTxfSizeForUsb[1], gUVCIsoinTxfUnitSize));
}
UINT32 UVAC_RemoveTxfInfo(UVAC_TXF_QUE queIdx)
{
	UINT32 i = 0;
	UINT32 removeCnt = 0;

	if (queIdx >= UVAC_TXF_QUE_MAX) {
		DBG_IND("Clr all TxfInfo:%d\r\n", queIdx);
		loc_cpu();
		for (i = 0; i < UVAC_TXF_QUE_MAX; i++) {
			removeCnt += gUvacTxfQueCurCnt[i];
			gUvacTxfQueCurCnt[i] = 0;
			gUvacTxfQueFrontIdx[i] = 0;
			gUvacTxfQueRearIdx[i] = 0;
		}
		memset((void *)&gUvacTxfQue[0][0], 0, (sizeof(UVAC_TXF_INFO)*UVAC_TXF_MAX_QUECNT * UVAC_TXF_QUE_MAX));
		unl_cpu();
		vos_flag_set(FLG_ID_UVAC_FRM, (FLGUVAC_FRM_V1 | FLGUVAC_FRM_V2 | FLGUVAC_FRM_A1 | FLGUVAC_FRM_A2));
	} else {
		loc_cpu();
		removeCnt = gUvacTxfQueCurCnt[queIdx];
		gUvacTxfQueCurCnt[queIdx] = 0;
		gUvacTxfQueFrontIdx[queIdx] = 0;
		gUvacTxfQueRearIdx[queIdx] = 0;
		memset((void *)&gUvacTxfQue[queIdx][0], 0, (sizeof(UVAC_TXF_INFO)*UVAC_TXF_MAX_QUECNT));
		DbgMsg_UVC(("DelQue=%d,cnt=%d\r\n", queIdx, removeCnt));
		// UVAC_DbgDmp_TxfInfoByQue(queIdx);
		unl_cpu();
		if (queIdx == UVAC_TXF_QUE_V1) {
			vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V1);
		} else if (queIdx == UVAC_TXF_QUE_V2) {
			vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_V2);
		} else if (queIdx == UVAC_TXF_QUE_A1) {
			vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A1);
		} else if (queIdx == UVAC_TXF_QUE_A2) {
			vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A2);
		}
	}
	DbgMsg_UVCIO(("-%s:idx=%d,delC=%d\r\n", __func__, queIdx, removeCnt));
	return removeCnt;
}

void UVAC_ResetTxfInfo(UVAC_TXF_QUE queIdx)
{
	gUvacTxfQueCurCnt[queIdx] = 0;
	gUvacTxfQueFrontIdx[queIdx] = 0;
	gUvacTxfQueRearIdx[queIdx] = 0;

	return;
}
static UVAC_TXF_INFO *UVAC_GetFreeTxfInfo(UVAC_TXF_QUE queIdx)
{
	UVAC_TXF_INFO *pTxfInfo = 0;;
	UINT32 idx = 0;
	DbgMsg_UVCIO(("+%s:Que[%d],fIdx=%d,%d,cnt=%d\r\n", __func__, queIdx, gUvacTxfQueFrontIdx[queIdx], gUvacTxfQueRearIdx[queIdx], gUvacTxfQueCurCnt[queIdx]));

	if (queIdx >= UVAC_TXF_QUE_MAX) {
		DBG_ERR("Over txfInfoQueCnt=%d > %d\r\n", queIdx, UVAC_TXF_QUE_MAX);
		return pTxfInfo;
	}
	if (gUvacTxfQueCurCnt[queIdx] < UVAC_TXF_MAX_QUECNT) {
		DbgMsg_UVCIO(("+AddTxfQue[%d],curCnt=%d,f=%d,%d\r\n", queIdx, gUvacTxfQueCurCnt[queIdx], gUvacTxfQueFrontIdx[queIdx], gUvacTxfQueRearIdx[queIdx]));
		idx = gUvacTxfQueRearIdx[queIdx];
		pTxfInfo = &gUvacTxfQue[queIdx][idx];

		DbgCode_UVC(if (0 == pTxfInfo) DBG_DUMP("TxfQue[%d]:f=%d,%d,cnt=%d,pTxfInfo=0x%x\r\n", queIdx, gUvacTxfQueFrontIdx[queIdx], gUvacTxfQueRearIdx[queIdx], gUvacTxfQueCurCnt[queIdx], pTxfInfo);)

		{
			idx = (idx + 1) % UVAC_TXF_MAX_QUECNT;
		}
		gUvacTxfQueRearIdx[queIdx] = idx;
		//loc_cpu();
		gUvacTxfQueCurCnt[queIdx]++;
		//unl_cpu();

		DbgMsg_UVCIO(("-AddTxfQue[%d],curCnt=%d,f=%d,%d\r\n", queIdx, gUvacTxfQueCurCnt[queIdx], gUvacTxfQueFrontIdx[queIdx], gUvacTxfQueRearIdx[queIdx]));
		#if 0
		DbgCode_UVC(if ((idx == gUvacTxfQueFrontIdx[queIdx]) || (gUvacTxfQueCurCnt[queIdx] >= (UVAC_TXF_MAX_QUECNT - 1))) \
					DBG_DUMP("TxfQue[%d]:f=%d,r=%d,cnt=%d almost full\r\n", queIdx, gUvacTxfQueFrontIdx[queIdx], gUvacTxfQueRearIdx[queIdx], gUvacTxfQueCurCnt[queIdx]);)
		#endif
	} else {
		//DbgMsg_UVC(("^RNo txfInfo[%d]=%d\r\n", queIdx, gUvacTxfQueCurCnt[queIdx]));
	}
	return pTxfInfo;
}
ER UVAC_AddIsoInTxfInfo(UVAC_TXF_INFO *pTxfInfo, UVAC_TXF_QUE queIdx)
{
	UVAC_TXF_INFO *pDstTxfInfo;
	DbgMsg_UVCIO(("+%s:0x%x,queIdx=%d\r\n", __func__, pTxfInfo, queIdx));

	if (0 == pTxfInfo) {
		DBG_ERR("In txfInfo NULL\r\n");
		return E_PAR;
	}

	loc_cpu();
	pDstTxfInfo = UVAC_GetFreeTxfInfo(queIdx);
	if (0 == pDstTxfInfo) {
		//DBG_IND("Add:No free txfInfo:%d\r\n", queIdx);
		unl_cpu();
		return E_PAR;
	}
	memcpy((void *)pDstTxfInfo, (void *)pTxfInfo, sizeof(UVAC_TXF_INFO));
	unl_cpu();
	DbgMsg_UVCIO(("AddTxf:adr=0x%x,0x%x,ep=%d,que=%d\r\n", pTxfInfo->sAddr, pTxfInfo->size, pTxfInfo->usbEP, queIdx));

	if (UVAC_TXF_QUE_V1 == queIdx) {
		set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_VDO_TXF);
	} else if (UVAC_TXF_QUE_V2 == queIdx) {
		set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_VDO2_TXF);
	} else if (UVAC_TXF_QUE_A1 == queIdx) {
		set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_AUD_TXF);
	} else if (UVAC_TXF_QUE_A2 == queIdx) {
		set_flg(FLG_ID_UVAC, FLGUVAC_ISOIN_AUD2_TXF);
	} else if (UVAC_TXF_QUE_V3 == queIdx) {
		set_flg(FLG_ID_UVAC_UVC3, FLGUVAC_ISOIN_VDO3_TXF);
	}
	return E_OK;
}

#if (ISF_LATENCY_DEBUG)
static void isf_timestamp_dump(UVAC_TIMESTAMP_INFO *p_in_isf)
{
	UVAC_TIMESTAMP_INFO     *p_isf;
	UINT32       i;
	if (g_u3_isf_uvac_timestamp_idx < ISF_UVAC_MAX_ISF_DEBUG_NUM) {
		p_isf = &g_u3_isf_uvac_timestamp[g_u3_isf_uvac_timestamp_idx++];
		memcpy(p_isf, p_in_isf, sizeof(UVAC_TIMESTAMP_INFO));
	} else if (u3_is_isf_dump == FALSE) {
		u3_is_isf_dump = TRUE;
		for (i=0; i< ISF_UVAC_MAX_ISF_DEBUG_NUM; i++) {
			p_isf = &g_u3_isf_uvac_timestamp[i];
			DBG_DUMP("timediff %6lld %6lld %6lld\r\n", (p_isf->uvc_timestamp_start - p_isf->timestamp), (p_isf->uvc_timestamp_end - p_isf->uvc_timestamp_start), (p_isf->uvc_timestamp_end - p_isf->timestamp));
			//DBG_DUMP("timediff %6lld %6lld %6lld\r\n", p_isf->timestamp, p_isf->uvc_timestamp_start, p_isf->uvc_timestamp_end);
		}
	}
}

void UVAC_DbgDmp_TimestampNumReset(void)
{
	g_u3_isf_uvac_timestamp_idx = 0;
	u3_is_isf_dump = FALSE;
}
#endif

#if (ISF_AUDIO_LATENCY_DEBUG)
static void isf_aud_timestamp_dump(UVAC_AUD_TIMESTAMP_INFO *p_in_isf)
{
	UVAC_AUD_TIMESTAMP_INFO     *p_isf;
	UINT32 i;
	if (g_u3_isf_uvac_aud_timestamp_idx < ISF_UVAC_AUD_MAX_ISF_DEBUG_NUM) {
		p_isf = &g_u3_isf_uvac_aud_timestamp[g_u3_isf_uvac_aud_timestamp_idx++];
		memcpy(p_isf, p_in_isf, sizeof(UVAC_AUD_TIMESTAMP_INFO));
	} else if (u3_is_aud_isf_dump == FALSE) {
		u3_is_aud_isf_dump = TRUE;
		DBG_DUMP("                 start-ts end-start   end-ts \r\n");
		for (i=0; i< ISF_UVAC_AUD_MAX_ISF_DEBUG_NUM; i++) {
			p_isf = &g_u3_isf_uvac_aud_timestamp[i];
			DBG_DUMP("timediff buf[%02d] %3d %8lld  %8lld %8lld \r\n", i+1, p_isf->uvc_slice_size, (p_isf->uvc_timestamp_start - p_isf->timestamp), (p_isf->uvc_timestamp_end - p_isf->uvc_timestamp_start), (p_isf->uvc_timestamp_end - p_isf->timestamp));
		}
	}
}

void UVAC_DbgDmp_Aud_TimestampNumReset(void)
{
	g_u3_isf_uvac_aud_timestamp_idx = 0;
	u3_is_aud_isf_dump = FALSE;
}
#endif

static void video_tx(UINT32 queIdx,	UVAC_VID_DEV_CNT thisVidIdx, FLGPTN wait_flg, FLGPTN idle_flg, FLGPTN exit_flg)
{
	FLGPTN uiFlag;
	UINT32 DMALen = 0, OriDMALen = 0;
	UVAC_TXF_INFO *pTxfInfo = 0;
	UINT32 itemIdx = 0;
	ER retV = E_OK;
	VOS_TICK tt1 = 0, tt2 = 0, tt3 = 0;
	VOS_TICK this_t = 0, last_t = 0;
	UINT32 txf_total_size;
	ID flag_uvac = (thisVidIdx == UVAC_VID_DEV_CNT_3)? FLG_ID_UVAC_UVC3 : FLG_ID_UVAC;

	vos_task_enter();
	DbgMsg_UVC(("+%s:queIdx=%d,Abort=%d\r\n", __func__, queIdx, gUvcUsbDMAAbord));
	UVAC_DbgDmp_TxfEPInfo();
	clr_flg(flag_uvac, wait_flg);

	while (1) {
		DbgMsg_UVCIO(("^R+IsoVdo\r\n"));
		set_flg(flag_uvac, idle_flg);
		wai_flg(&uiFlag, flag_uvac, wait_flg | exit_flg, (TWF_CLR | TWF_ORW));
		clr_flg(flag_uvac, idle_flg);
		DbgMsg_UVCIO(("^R-IsoVdo:0x%x\r\n", uiFlag));

		if (uiFlag & exit_flg) {
			set_flg(flag_uvac, idle_flg);
			break;
		}

		/*if (gUVCIsoinTxfUnitSize[thisVidIdx] == 0) {
			if (usb3dev_getDevInfo(U3DEV_DEVINFO_ID_SPEED) == U3DEV_SPEED_SS) {
				gUVCIsoinTxfUnitSize[thisVidIdx] = gU3UvcIsoInSsPacketSize[thisVidIdx] * gU3UvcIsoInSsBandWidth[thisVidIdx];
			} else {
				gUVCIsoinTxfUnitSize[thisVidIdx] = gU3UvcIsoInHsPacketSize[thisVidIdx] * gU3UvcIsoInHsBandWidth[thisVidIdx];
			}
		}*/

		while ((gUvcUsbDMAAbord == FALSE) && (gUvcNoVidStrm == FALSE) && gUvacTxfQueCurCnt[queIdx] && gUvcVidStart[queIdx]) {
			if (gU3UvacRunningFlag[thisVidIdx] & FLGUVAC_STOP) {
				DbgMsg_UVC(("Stop ISO Video[%d] ...\r\n", thisVidIdx));
				_rel_vdo_frm(thisVidIdx);
				break;
			}
			if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_QUE_CNT)) {
				DBG_DUMP("VIsoQ[%d]=%d\r\n", queIdx, gUvacTxfQueCurCnt[queIdx]);
			}
			itemIdx = gUvacTxfQueFrontIdx[queIdx];
			pTxfInfo = &gUvacTxfQue[queIdx][itemIdx];
			DbgMsg_UVCIO(("^M+Txf:0x%x,0x%x,ep=%d\r\n", pTxfInfo->sAddr, pTxfInfo->size, pTxfInfo->usbEP));
			if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_TX)) {
				vos_perf_mark(&tt1);
			}
			if (0 == pTxfInfo->size || 0 == pTxfInfo->sAddr || pTxfInfo->usbEP != UVAC_USB_EP[queIdx]) {
				DBG_WRN("zero stream 0:0x%x,0x%x,ep=%d\r\n", pTxfInfo->sAddr, pTxfInfo->size, pTxfInfo->usbEP);
				UVAC_DbgDmp_ChkTxfQueAlmostFull(queIdx, 0, 1, 1);
				UVAC_UpdateTxfQue(pTxfInfo, queIdx);
				continue;
			}

			vos_perf_mark(&this_t);
			if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_TX))
			DBG_DUMP("diff %d us\r\n", vos_perf_duration(last_t, this_t));
			last_t = this_t;

			UVC_MakePayloadFmt(thisVidIdx, pTxfInfo);
			//_rel_vdo_frm(thisVidIdx);

			if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_TX)) {
				vos_perf_mark(&tt2);
				DBG_DUMP("PL[%d]us\r\n", vos_perf_duration(tt1, tt2));
			}
			if (0 == pTxfInfo->size) {
				DBG_DUMP("drop stream:0x%x,0x%x,ep=%d,timestamp=%llu\r\n", pTxfInfo->sAddr, pTxfInfo->size, pTxfInfo->usbEP, pTxfInfo->timestamp);
				UVAC_UpdateTxfQue(pTxfInfo, queIdx);
				_rel_vdo_frm(thisVidIdx);
				continue;
			}
			txf_total_size = pTxfInfo->size;
			if (0) {
				DbgMsg_UVCIO(("^MOri[%d]=0x%x,Txf=0x%x\r\n", queIdx, pTxfInfo->size, gUvcMaxTxfSizeForUsb[queIdx]));
				OriDMALen = DMALen = gUvcMaxTxfSizeForUsb[queIdx];
				DbgCode_UVC(if (0 != (pTxfInfo->sAddr % 4))  DBG_DUMP("Not 4-align,USB,Addr=0x%lx,Len=0x%x\r\n", pTxfInfo->sAddr, DMALen);) {
					DbgMsg_UVCIO(("^R+WA[%d]=0x%x,0x%x\r\n", pTxfInfo->usbEP, DMALen, pTxfInfo->size));
				}
				retV = usb3dev_writeEndpoint(pTxfInfo->usbEP, (UINT8 *)(pTxfInfo->sAddr), &DMALen);
				if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_TX)) {
					DBG_DUMP("V[%d][%d]->EP%d\r\n", queIdx, itemIdx, pTxfInfo->usbEP);
				}
				if (retV != E_OK) {
					DBG_ERR("Large fail=%d,EP=%d,Len=%d,addr=0x%x\r\n", retV, pTxfInfo->usbEP, DMALen, pTxfInfo->sAddr);
					DMALen = 0;
					UVAC_UpdateTxfQue(pTxfInfo, queIdx);
					_rel_vdo_frm(thisVidIdx);
					break;
				} else {
					//temporarely assume this DMA will be fully transferred
					DMALen = OriDMALen;
				}
			} else {
				if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_TX)) {
					DBG_DUMP("V[%d][%d]->EP%d %dK\r\n", queIdx, itemIdx, pTxfInfo->usbEP, pTxfInfo->size / 1024);
					vos_perf_mark(&tt1);
				}
				OriDMALen = DMALen = pTxfInfo->size;
				DbgMsg_UVCIO(("+WA[%d]=0x%x,0x%x\r\n", pTxfInfo->usbEP, DMALen, pTxfInfo->size));

				if (u3_uvc_bulk_mode[thisVidIdx] && u3_uvc_bulk_multi_payload) {
					UINT32 remain_size = DMALen;
					UINT32 curr_size = 0;

					while (remain_size > 0 && gUvcVidStart[thisVidIdx]) {

						DMALen = (remain_size > gUVCIsoinTxfUnitSize[thisVidIdx])? gUVCIsoinTxfUnitSize[thisVidIdx] : remain_size;

						retV = usb3dev_writeEndpoint(pTxfInfo->usbEP, (UINT8 *)(pTxfInfo->sAddr+curr_size), &DMALen);
						if (retV != E_OK) {
							break;
						}

						remain_size -= DMALen;
						curr_size   += DMALen;
					}
				} else {
					retV = usb3dev_writeEndpoint(pTxfInfo->usbEP, (UINT8 *)(pTxfInfo->sAddr), &DMALen);
				}

				if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_TX)) {
					UINT32 diff_us;

					vos_perf_mark(&tt2);
					while(usb3dev_chkEPBusy(pTxfInfo->usbEP, U3DEV_EPDIR_IN));
					vos_perf_mark(&tt3);
					diff_us = vos_perf_duration(tt1, tt3);
					DBG_DUMP("ts=%d, size=%d, %dus(%lluKB/sec), busy %dus\r\n", tt3, pTxfInfo->size, diff_us, ((ULONG)pTxfInfo->size)*1000/1024*1000/(ULONG)diff_us, vos_perf_duration(tt2, tt3));
				}
				//usb_setEPWrite(pTxfInfo->usbEP, (UINT8 *)(pTxfInfo->sAddr), &DMALen);
				//retV = usb_waitEPDone(pTxfInfo->usbEP, &DMALen);
				DbgMsg_UVCIO(("-WA[%d]=0x%x,0x%x\r\n",pTxfInfo->usbEP, DMALen, pTxfInfo->size));
				if (retV != E_OK) {
					DBG_ERR(":fail=%d,EP=%d,Len=%d,addr=0x%x\r\n", retV, pTxfInfo->usbEP, DMALen, pTxfInfo->sAddr);
					DMALen = 0;
					UVAC_UpdateTxfQue(pTxfInfo, queIdx);
					_rel_vdo_frm(thisVidIdx);
					break;
				} else {
					//temporarely assume this DMA will be fully transferred
					DMALen = OriDMALen;
				}
			}

			pTxfInfo->sAddr += DMALen;
			pTxfInfo->size -= DMALen;
			pTxfInfo->txfCnt ++;
			if (0 == pTxfInfo->size) {
				if (u3_uvc_bulk_mode[thisVidIdx]) {
					UINT32 packet_size;

					if (g_usb_speed == U3DEV_SPEED_SS) {
						packet_size = UVC_SS_BULKIN_MAXPKTSIZE;
					} else {
						packet_size = UVC_HS_BULKIN_MAXPKTSIZE;
					}
					if (txf_total_size % packet_size == 0) {
						usb3dev_set_tx0byte(pTxfInfo->usbEP);
					}
				}
				_rel_vdo_frm(thisVidIdx);
				UVAC_UpdateTxfQue(pTxfInfo, queIdx);
				#if (ISF_LATENCY_DEBUG)
				{
					UVAC_TIMESTAMP_INFO  time_info;

					if (pTxfInfo->frame_type == UVAC_VIDEO_FRM_NORMAL || pTxfInfo->frame_type == UVAC_VIDEO_FRM_FINAL) {
						time_info.timestamp = pTxfInfo->timestamp;
						time_info.uvc_timestamp_start = pTxfInfo->uvc_timestamp_start;
						time_info.uvc_timestamp_end = hd_gettime_us();
						isf_timestamp_dump(&time_info);
					}
					//printf("diff = %lld\r\n", cur_time - pTxfInfo->timestamp);
				}
				#endif
			} else {
				DbgMsg_UVC(("V[%d]abort DMA, size remain %d\r\n", queIdx, pTxfInfo->size));
			}
			DbgMsg_UVCIO(("^M-Txf:adr=0x%x,0x%x,ep=%d,txf=0x%x,txfcnt=%d,queC=%d,%d\r\n", pTxfInfo->sAddr, pTxfInfo->size, pTxfInfo->usbEP, DMALen, pTxfInfo->txfCnt, gUvacTxfQueCurCnt[UVAC_TXF_QUE_V1], gUvacTxfQueCurCnt[UVAC_TXF_QUE_A1]));
		}

	}
}
void* UVAC_IsoInVdo1Tsk(void* arglist)
{
	video_tx(UVAC_TXF_QUE_V1, UVAC_VID_DEV_CNT_1, FLGUVAC_ISOIN_VDO_TXF, FLGUVAC_ISOIN_VDO_READY, FLGUVAC_ISOIN_VDO_EXIT);

	return 0;
}

void* UVAC_IsoInVdo2Tsk(void* arglist)
{
	video_tx(UVAC_TXF_QUE_V2, UVAC_VID_DEV_CNT_2, FLGUVAC_ISOIN_VDO2_TXF, FLGUVAC_ISOIN_VDO2_READY, FLGUVAC_ISOIN_VDO2_EXIT);

	return 0;
}


void* UVAC_IsoInVdo3Tsk(void* arglist)
{
	video_tx(UVAC_TXF_QUE_V3, UVAC_VID_DEV_CNT_3, FLGUVAC_ISOIN_VDO3_TXF, FLGUVAC_ISOIN_VDO3_READY, FLGUVAC_ISOIN_VDO3_EXIT);

	return 0;
}

static void audio_tx(UINT32 queIdx,	UVAC_AUD_DEV_CNT thisAudIdx, FLGPTN wait_flg, FLGPTN idle_flg, FLGPTN exit_flg)
{
	FLGPTN uiFlag;
	UINT32 DMALen = 0, OriDMALen = 0;
	UVAC_TXF_INFO *pTxfInfo = 0;
	UINT32 itemIdx = 0;
	ER retV = E_OK;
	VOS_TICK tt1 = 0, tt2 = 0, tt_last = 0;
	#if 0
	UINT32 packet_size_curr = gUvacAudSampleRate[0] * gUacChNum * UAC_BIT_PER_SECOND / 8 / 1000;
	UINT32 packet_size_small = packet_size_curr;
	UINT32 packet_size_large = packet_size_curr + 4;
	#endif

	//UINT32 UacPacketSize = 0;
	//FLGPTN waiptn = 0;
	//TIMER_ID timer_id = TIMER_NUM;
	//char  *chip_name = getenv("NVT_CHIP_ID");
	//BOOL timer_mode = FALSE;

	#if (ISF_AUDIO_LATENCY_DEBUG)
	UVAC_AUD_TIMESTAMP_INFO  time_info;
	#endif

	//if (chip_name != NULL && strcmp(chip_name, "CHIP_NA51084") == 0) {
		//timer_mode = TRUE;
	//}

	#if 0
	if (timer_mode) {
		if (thisAudIdx == UVAC_AUD_DEV_CNT_1) {
			waiptn = FLGUVAC_AUD1_CHK_BUSY;
			timer_id = gUacTimerID[UVAC_AUD_DEV_CNT_1];
		} else {
			waiptn = FLGUVAC_AUD2_CHK_BUSY;
			timer_id = gUacTimerID[UVAC_AUD_DEV_CNT_2];
		}

		UacPacketSize = gUvacAudSampleRate[0] * gUacChNum * UAC_BIT_PER_SECOND / 8 / 1000; //Audio bit rate(byte per ms).
		UacPacketSize += 4;	//fix for compatibility with MacOS
		if (UacPacketSize < 36) {
			UacPacketSize = 36;
		}
		if (gUacMaxPacketSize){
			UacPacketSize = gUacMaxPacketSize;
		}
	}
	#endif

	vos_task_enter();
	DbgMsg_UVC(("+%s:QueMax=%d,Abort=%d,HWPL=%d,%d\r\n", __func__, UVAC_TXF_QUE_MAX, gUvcUsbDMAAbord, gUvcHWPayload[UVAC_VID_DEV_CNT_1], gUvcHWPayload[UVAC_VID_DEV_CNT_2]));
	clr_flg(FLG_ID_UVAC, wait_flg);
	while (1) {
		DbgMsg_UVCIO(("^R+IsoAud\r\n"));
		set_flg(FLG_ID_UVAC, idle_flg);
		wai_flg(&uiFlag, FLG_ID_UVAC, wait_flg | exit_flg, (TWF_CLR | TWF_ORW));
		clr_flg(FLG_ID_UVAC, idle_flg);
		DbgMsg_UVCIO(("^R-IsoAud:0x%x\r\n", uiFlag));

		if (uiFlag & exit_flg) {
			set_flg(FLG_ID_UVAC, idle_flg);
			break;
		}

		while ((gUacUsbDMAAbord == FALSE) && gUvacTxfQueCurCnt[queIdx] && gUacAudStart[thisAudIdx]) {
			if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_QUE_CNT)) {
				DBG_DUMP("AIsoQ[%d]=%d\r\n", queIdx, gUvacTxfQueCurCnt[queIdx]);
			}

			loc_cpu();
			itemIdx = gUvacTxfQueFrontIdx[queIdx];
			pTxfInfo = &gUvacTxfQue[queIdx][itemIdx];
			unl_cpu();
			DbgMsg_UVCIO(("^M+Txf:0x%x,0x%x,ep=%d\r\n", pTxfInfo->sAddr, pTxfInfo->size, pTxfInfo->usbEP));
			if (0 == pTxfInfo->size || 0 == pTxfInfo->sAddr || pTxfInfo->usbEP != UVAC_USB_EP[queIdx]) {

				DBG_WRN("zero stream:0x%x,0x%x,ep=%d,que=%d\r\n", pTxfInfo->sAddr, pTxfInfo->size, pTxfInfo->usbEP, gUvacTxfQueCurCnt[queIdx]);
				UVAC_UpdateTxfQue(pTxfInfo, queIdx);
				if (thisAudIdx == UVAC_AUD_DEV_CNT_1) {
					vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A1);
				} else {
					vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A2);
				}
				continue;
			}

			#if 0
			if (packet_size_curr != packet_size_large) {
				packet_size_curr = packet_size_large;
			} else {
				packet_size_curr = packet_size_small;
			}
			usb3dev_set_ep_maxpktsz(USB_EP2, packet_size_curr);
			#endif

			if (pTxfInfo->size > gUvcMaxTxfSizeForUsb[queIdx]) {
				DbgMsg_UVCIO(("^MOri[%d]=0x%x,Txf=0x%x\r\n", queIdx, pTxfInfo->size, gUvcMaxTxfSizeForUsb[queIdx]));
				OriDMALen = DMALen = gUvcMaxTxfSizeForUsb[queIdx];
				DbgCode_UVC(if (0 != (pTxfInfo->sAddr % 4))  DBG_DUMP("Not 4-align,USB,Addr=0x%lx,Len=0x%x\r\n", pTxfInfo->sAddr, DMALen);) {
					DbgMsg_UVCIO(("^R+WA[%d]=0x%x,0x%x\r\n", pTxfInfo->usbEP, DMALen, pTxfInfo->size));
				}
				retV = usb3dev_writeEndpoint(pTxfInfo->usbEP, (UINT8 *)(pTxfInfo->sAddr), &DMALen);
				if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_TX)) {
					DBG_DUMP("A[%d][%d]->EP%d\r\n", queIdx, itemIdx, pTxfInfo->usbEP);
				}
				if (retV != E_OK) {
					DBG_ERR("Large fail=%d,EP=%d,Len=%d,addr=0x%x\r\n", retV, pTxfInfo->usbEP, DMALen, pTxfInfo->sAddr);
					DMALen = 0;
					if (thisAudIdx == UVAC_AUD_DEV_CNT_1) {
						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A1);
					} else {
						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A2);
					}
					break;
				} else {
					//temporarely assume this DMA will be fully transferred
					DMALen = OriDMALen;
				}
			} else {
				#if (ISF_AUDIO_LATENCY_DEBUG)
				time_info.uvc_timestamp_start= hd_gettime_us();
				time_info.uvc_slice_size = pTxfInfo->size;
				#endif

				if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_TX)) {
					//DBG_DUMP("A[%d][%d]->EP%d %dK\r\n", queIdx, itemIdx, pTxfInfo->usbEP, pTxfInfo->size / 1024);
					vos_perf_mark(&tt1);

				}

				OriDMALen = DMALen = pTxfInfo->size;
				DbgMsg_UVCIO(("^R+WA[%d]=0x%x,0x%x\r\n", pTxfInfo->usbEP, DMALen, pTxfInfo->size));
				retV = usb3dev_writeEndpoint(pTxfInfo->usbEP, (UINT8 *)(pTxfInfo->sAddr), &DMALen);
				//retV = usb3dev_writeEndpoint_timeout(pTxfInfo->usbEP, (UINT8 *)(pTxfInfo->sAddr), &DMALen, 19);

				if (UVAC_IsoTxDbg(queIdx, UVAC_DBG_ISO_TX)) {
					UINT32 diff_us;

					vos_perf_mark(&tt2);
					diff_us = vos_perf_duration(tt1, tt2);
					DBG_DUMP("size=%d, busy %dus, que=%d, last=%d\r\n", pTxfInfo->size, diff_us, gUvacTxfQueCurCnt[queIdx], tt1-tt_last);
					tt_last = tt1;
				}
				//usb_setEPWrite(pTxfInfo->usbEP, (UINT8 *)(pTxfInfo->sAddr), &DMALen);
				//retV = usb_waitEPDone(pTxfInfo->usbEP, &DMALen);
				//DbgMsg_UVC(("^R-WA[%d]=0x%x,0x%x\r\n",pTxfInfo->usbEP, DMALen, pTxfInfo->size));
				if (retV != E_OK) {
					DBG_ERR(":fail=%d,EP=%d,Len=%d,addr=0x%x\r\n", retV, pTxfInfo->usbEP, DMALen, pTxfInfo->sAddr);
					DMALen = 0;
					if (thisAudIdx == UVAC_AUD_DEV_CNT_1) {
						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A1);
					} else {
						vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A2);
					}
					break;
				} else {
					//temporarely assume this DMA will be fully transferred
					if (DMALen != OriDMALen && gUacAudStart[thisAudIdx]) {
						printf("UAC[%d] TX write=%d, expect=%d\r\n", thisAudIdx, DMALen, OriDMALen);
					}
					DMALen = OriDMALen;
				}

				if (thisAudIdx == UVAC_AUD_DEV_CNT_1) {
					vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A1);
				} else {
					vos_flag_set(FLG_ID_UVAC_FRM, FLGUVAC_FRM_A2);
				}
			}
			pTxfInfo->sAddr += DMALen;
			pTxfInfo->size -= DMALen;
			pTxfInfo->txfCnt ++;

			if (0 == pTxfInfo->size) {
				//if (gfpUvacReleaseCB) {
				//	gfpUvacReleaseCB(thisAudIdx, pTxfInfo->oriAddr);
				//}
				#if (ISF_AUDIO_LATENCY_DEBUG)
				{
					time_info.timestamp = pTxfInfo->timestamp;
					time_info.uvc_timestamp_end = hd_gettime_us();
					isf_aud_timestamp_dump(&time_info);
					//printf("diff = %lld\r\n", cur_time - pTxfInfo->timestamp);
				}
				#endif
				UVAC_UpdateTxfQue(pTxfInfo, queIdx);
			} else {
				DbgMsg_UVC(("A[%d]abort DMA, size remain %d\r\n", queIdx, pTxfInfo->size));

			}

			DbgMsg_UVCIO(("^M-Txf:adr=0x%x,0x%x,ep=%d,txf=0x%x,txfcnt=%d,queC=%d,%d\r\n", pTxfInfo->sAddr, pTxfInfo->size, pTxfInfo->usbEP, DMALen, pTxfInfo->txfCnt, gUvacTxfQueCurCnt[UVAC_TXF_QUE_V1], gUvacTxfQueCurCnt[UVAC_TXF_QUE_A1]));
		}

	}
}
void* UVAC_IsoInAud1Tsk(void* arglist)
{
	audio_tx(UVAC_TXF_QUE_A1, UVAC_AUD_DEV_CNT_1, FLGUVAC_ISOIN_AUD_TXF, FLGUVAC_ISOIN_AUD_READY, FLGUVAC_ISOIN_AUD_EXIT);
	return 0;
}
void* UVAC_IsoInAud2Tsk(void* arglist)
{
	audio_tx(UVAC_TXF_QUE_A2, UVAC_AUD_DEV_CNT_2, FLGUVAC_ISOIN_AUD2_TXF, FLGUVAC_ISOIN_AUD2_READY, FLGUVAC_ISOIN_AUD2_EXIT);
	return 0;
}
