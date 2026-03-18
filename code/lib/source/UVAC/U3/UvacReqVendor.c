#include    "UvacReqVendor.h"
#include    "UvacVideoTsk.h"
#include    "UvacDbg.h"
#include    "UvacID.h"
#include    <string.h>

#define UVC_VENDREQ_BUF_SIZE                64
static UINT8 gVendorBuf[UVC_VENDREQ_BUF_SIZE];

UVAC_VENDOR_REQ_CB g_fpUvacVendorReqCB = NULL;
UVAC_VENDOR_REQ_CB g_fpUvacVendorReqIQCB = NULL;
extern UVAC_MSOS20_INFO g_u3_msos_info;
static BOOL g_HostToDevice = FALSE;
static UINT32 g_RequestLen;
static UINT32 g_req_idx;
/*---------------------------------------------------------------------------
 Function   : Read data from host by control pipe.
----------------------------------------------------------------------------*/

void UvacVendorRequestHandler(uintptr_t uiRequest)
{
	UINT32 tmpSize = 0;
	UINT8 reqIdx = 0;
	UVAC_VENDOR_PARAM Param = {0};
	PUSB_DEVICE_REQUEST    pRequest;

	DBG_IND(" req=%lu, cb=0x%x, iqcb=0x%x\r\n", uiRequest, g_fpUvacVendorReqCB, g_fpUvacVendorReqIQCB);

	pRequest = (PUSB_DEVICE_REQUEST)uiRequest;

	reqIdx = Param.uiReguest = pRequest->b_request;
	Param.uiValue = pRequest->w_value;
	Param.uiIndex = pRequest->w_index;//interface or endpoint
	Param.uiDataSize = pRequest->w_length;

	if (pRequest->bm_request_type & USB_ENDPOINT_DIRECTION_MASK) {
		Param.bHostToDevice = FALSE;
		g_HostToDevice = FALSE;

		if (g_fpUvacVendorReqIQCB && ((reqIdx & UVAC_VENDREQ_IQ_MASK) >= UVAC_VENDREQ_IQ_MASK)) {
			g_fpUvacVendorReqIQCB(&Param);
		} else if (g_fpUvacVendorReqCB && ((reqIdx & UVAC_VENDREQ_CUST_MASK) >= UVAC_VENDREQ_CUST_MASK)) {
			g_fpUvacVendorReqCB(&Param);
		} else if (reqIdx == 0x05) {
			if (g_u3_msos_info.p_msos20_desc != 0) {
				usb3dev_handleSetupResult(FALSE, (uintptr_t)g_u3_msos_info.p_msos20_desc, g_u3_msos_info.msos20_desc_size);
			} else {
				DBG_ERR("MSOS20 descripor is NULL\r\n");
				usb3dev_handleSetupResult(TRUE, 0, 0);
			}
			return;
		} else {
			DBG_ERR("Unsupport Vendor Request:0x%x, 0x%x\r\n", reqIdx, uiRequest);
			usb3dev_handleSetupResult(TRUE, 0, 0);
			return;
		}
		if (Param.uiDataSize > EP0_PACKET_SIZE) {
			Param.uiDataSize = EP0_PACKET_SIZE;
			DBG_ERR("Vendor data size should NOT exceed %d bytes!\r\n", EP0_PACKET_SIZE);
		}
		if (0 == Param.uiDataSize) {
			DBG_ERR("Data Size Zero !!\r\n");
			usb3dev_handleSetupResult(TRUE, 0, 0);
		} else {
			uintptr_t uiRetBufAddr = 0;
			UINT32 uiRetSize = 0;

			if (Param.uiDataSize > UVC_VENDREQ_BUF_SIZE) {
				tmpSize = UVC_VENDREQ_BUF_SIZE;
				DBG_ERR("Param.uiDataSize=%d > %d\r\n", Param.uiDataSize, UVC_VENDREQ_BUF_SIZE);
			} else {
				tmpSize = Param.uiDataSize;
			}
			if (Param.uiDataAddr) {
				memcpy((void *)&gVendorBuf[0], (void *)Param.uiDataAddr, tmpSize);
			}
			if (Param.uiDataSize > pRequest->w_length) {
				DBG_ERR("Param.uiDataSize=%d > Host=%d\r\n", Param.uiDataSize, pRequest->w_length);
				uiRetSize  = pRequest->w_length;
			} else {
				DBG_ERR("Param.uiDataSize=%d <= Host=%d\r\n", Param.uiDataSize, pRequest->w_length);
				uiRetSize  = Param.uiDataSize;
			}
			uiRetBufAddr = (uintptr_t)&gVendorBuf[0];
			DBG_DUMP("Data Size=%d,%d,%d\r\n", uiRetSize, Param.uiDataSize, pRequest->w_length);

			usb3dev_handleSetupResult(FALSE, uiRetBufAddr, uiRetSize);
		}

		return;
	} else {
		Param.bHostToDevice = TRUE;
		g_HostToDevice = TRUE;
		g_RequestLen = Param.uiDataSize;
		g_req_idx = reqIdx;

		usb3dev_handleSetupResult(FALSE, (uintptr_t)&gVendorBuf[0], g_RequestLen);

		return;
	}
	DBG_DUMP("HostToDevice=%d, bRequest=%d, wValue=%d, wIndex=%d, wLength=%d\r\n", Param.bHostToDevice, Param.uiReguest, Param.uiValue, Param.uiIndex, Param.uiDataSize);
#if (_UVC_DBG_LVL_ >= _UVC_DBG_CHK_)
	UINT32 tmpV = 0;
	DBG_DUMP("VenReq=0x%x/0x%x,dataSize=%d,cb=0x%x,iqcb=0x%x\r\n", reqIdx, uiRequest, Param.uiDataSize, g_fpUvacVendorReqCB, g_fpUvacVendorReqIQCB);
	while (tmpV < Param.uiDataSize) {
		DBG_DUMP("0x%x ", gVendorBuf[tmpV]);
		tmpV ++;
	}
#endif

	usb3dev_handleSetupResult(FALSE, 0, 0);

	return;
}

void UvacVendorRequestDone(uintptr_t uiEvent)
{
	UVAC_VENDOR_PARAM Param = {0};

	if (g_HostToDevice) {
		if (0 == g_RequestLen) {
			DBG_ERR("Vendor data size zero\r\n");
			//usb_setEP0Stall();
		} else {
			Param.uiDataAddr = (uintptr_t)&gVendorBuf[0];
		}
		if (g_fpUvacVendorReqIQCB && ((g_req_idx & UVAC_VENDREQ_IQ_MASK) >= UVAC_VENDREQ_IQ_MASK)) {
			g_fpUvacVendorReqIQCB(&Param);
		} else if (g_fpUvacVendorReqCB && ((g_req_idx & UVAC_VENDREQ_CUST_MASK) >= UVAC_VENDREQ_CUST_MASK)) {
			g_fpUvacVendorReqCB(&Param);
		} else {
			DBG_ERR("Unsupport Vendor Request:%d\r\n", g_req_idx);
			usb3dev_handleSetupResult(TRUE, 0, 0);

			return;
		}
	}

	usb3dev_handleSetupResult(FALSE, 0, 0);
}


