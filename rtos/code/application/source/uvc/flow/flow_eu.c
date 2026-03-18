#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/stdio.h>
#include <unistd.h>
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include <kwrap/task.h>
#include "kwrap/semaphore.h"
#include <hdal.h>
#include <time.h>
#include <comm/timer.h>
#include "comm/hwclock.h"
#include "UVAC.h"
#include "prjcfg.h"
#include "flow_uvc.h"
#include "flow_ct_pu.h"
#include "flow_eu.h"
#include "vendor_isp.h"
#include <pthread.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API

extern VIDEO_STREAM stream[1];
static UINT32 raw_seq = NUM_PACK;
UINT32 raw_offser = 0;
static HD_VIDEO_FRAME raw_data = {0};
static BOOL xUvac_EU_GetRawCB(UINT8 request, UINT8 *pData, UINT32 *pDataLen){
	static UINT8 buf[SIZE_PACK];	//maximum 20KB per packet
	HD_RESULT ret = HD_OK;

	switch (request){
		case GET_LEN:
			pData[0] = 0x00;
			pData[1] = 0x50;	//20 KB
			*pDataLen = 2;		//must be 2 according to UVC spec
			return TRUE;
		case GET_INFO:
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			*pDataLen = 1;		//must be 1 according to UVC spec
			return TRUE;
		case GET_MIN:
			memset(buf, 0x00, sizeof(buf));
			*(UINT32 *)pData = (UINT32)buf;
			*pDataLen = sizeof(buf);
			return TRUE;
		case GET_MAX:
			memset(buf, 0xFF, sizeof(buf));
			*(UINT32 *)pData = (UINT32)buf;
			*pDataLen = sizeof(buf);
			return TRUE;
		case GET_RES:
			memset(buf, 0x01, sizeof(buf));
			*(UINT32 *)pData = (UINT32)buf;
			*pDataLen = sizeof(buf);
			return TRUE;
		case GET_DEF:
			memset(buf, 0x00, sizeof(buf));
			*(UINT32 *)pData = (UINT32)buf;
			*pDataLen = sizeof(buf);
			return TRUE;
		case GET_CUR:
			//read raw date and store them into buf
			if (raw_seq > 0){
				raw_seq--;
			} else {
				//reset raw_seq
				raw_seq = NUM_PACK;
			}
			*(UINT32 *)pData = raw_data.phy_addr[0] + (raw_offser * SIZE_PACK);
			*pDataLen = SIZE_PACK;
			raw_offser++;

			if (raw_offser == NUM_PACK){
				raw_offser = 0;	
				ret = hd_videocap_release_out_buf(stream[0].cap_path, &raw_data);
				if (ret != HD_OK) {
					printf("cap_release error=%d !!\r\n\r\n", ret);
				}
				
				hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);
				hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
				hd_videoproc_start(stream[0].proc_main_path);
				hd_videoenc_start(stream[0].enc_main_path);
				//printf("HDAL bind/start! \r\n\r\n");
			}
			return TRUE;
		case SET_CUR:
			//UVAC lib does not support XU set_cur with data larger than 64 bytes
			DBG_ERR("not support Data = 0x%02X for SET_CUR\r\n", *pData);
			return FALSE;
	}
	DBG_ERR("get raw not support!\r\n");
	return FALSE;
}

static BOOL xUvac_EU_SetRawSeqCB(UINT8 request, UINT8 *pData, UINT32 *pDataLen){

	DBG_DUMP("xUvac_EU_SetRawSeqCB request=0x%02X, pData=0x%8x, pDataLen=0x%8x\r\n", request, pData, pDataLen);

	HD_RESULT ret = HD_OK;

	switch (request){
		case GET_LEN:
			pData[0] = 0x02;
			pData[1] = 0x00;
			*pDataLen = 2;		//must be 2 according to UVC spec
			return TRUE;
		case GET_INFO:
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			*pDataLen = 1;		//must be 1 according to UVC spec
			return TRUE;
		case GET_MIN:
			pData[0] = 0x00;
			pData[1] = 0x00;
			*pDataLen = 2;
			return TRUE;
		case GET_MAX:
			pData[0] = 0xFF;
			pData[1] = 0xFF;
			*pDataLen = 2;
			return TRUE;
		case GET_RES:
			pData[0] = 0x01;
			pData[1] = 0x00;
			*pDataLen = 2;
			return TRUE;
		case GET_DEF:
			pData[0] = 0x00;
			pData[1] = 0x00;
			*pDataLen = 2;
			return TRUE;
		case GET_CUR:
			pData[0] = raw_seq & 0xFF;
			pData[1] = (raw_seq>>8) & 0xFF;
			*pDataLen = 2;
			return TRUE;
		case SET_CUR:
			hd_videoproc_stop(stream[0].proc_main_path);
			hd_videoenc_stop(stream[0].enc_main_path);
			hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
			hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);

			raw_seq = pData[0] + (pData[1]<<8);
			//printf("HDAL stop/unbind%d\r\n\r\n");
			ret = hd_videocap_pull_out_buf(stream[0].cap_path, &raw_data, -1); // -1 = blocking mode
			if (ret != HD_OK) {
				printf("cap_pull error=%d !!\r\n\r\n", ret);
				return FALSE;
			}
			printf("hd_videocap_pull_out_buf done\r\n\r\n");
			return TRUE;
	}
	DBG_ERR("set raw_seq not support!\r\n");
	return FALSE;
}


#if UVC_DPC_FUNC
static BOOL xUvac_EU_dpc(UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = TRUE;

	switch(request)
	{
		case GET_INFO:
			*pDataLen = 1;	//must be 1 according to UVC spec
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			break;
		case GET_LEN:
			*pDataLen = 2;	//must be 2 according to UVC spec
			pData[0] = 1;
			pData[1] = 0;
			break;
		case GET_MIN:
			*pDataLen = 1;
			pData[0] = 0;
			break;
		case GET_MAX:
			*pDataLen = 1;
			pData[0] = 1;
			break;
		case GET_RES:
			*pDataLen = 1;
			pData[0] = 1;
			break;
		case GET_DEF:
			*pDataLen = 1;
			pData[0] = 0;
			break;
		case GET_CUR:
			*pDataLen = 1;
			pData[0] = dpc_get_status();
			break;
		case SET_CUR:
			if (pData[0] == 1){	//do bright pixel calibration
				if (dpc_bright_start() < 0){
					ret = FALSE;
				} else {
					ret = TRUE;
				}
			} else if (pData[0] == 2){
				if (dpc_dark_start() < 0){
					ret = FALSE;
				} else {
					ret = TRUE;
				}
			} else {
				printf("dpc command(%u) not supprted.\n", pData[0]);
				ret = FALSE;
			}
			break;
		default:
			ret = FALSE;
			break;
	}
	return ret;
}
#endif

BOOL xUvacEU_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen){
	BOOL ret = FALSE;

	if (request == SET_CUR){
	#if 0
		printf("%s: SET_CUR, CS=0x%02X, request = 0x%X,  pData=0x%X, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	#endif
	}
	switch (CS){
		case EU_CONTROL_ID_01:
		case EU_CONTROL_ID_02:
		case EU_CONTROL_ID_03:
		case EU_CONTROL_ID_04:
		case EU_CONTROL_ID_05:
			ret = xUvac_EU_GetRawCB(request, pData, pDataLen);
			break;
		case EU_CONTROL_ID_06:
			ret = xUvac_EU_SetRawSeqCB(request, pData, pDataLen);
			break;
		case EU_CONTROL_ID_07:
		case EU_CONTROL_ID_08:
		case EU_CONTROL_ID_09:
		case EU_CONTROL_ID_10:
		case EU_CONTROL_ID_11:
		case EU_CONTROL_ID_12:
		case EU_CONTROL_ID_13:
		case EU_CONTROL_ID_14:
		case EU_CONTROL_ID_15:
		case EU_CONTROL_ID_16:
#if UVC_DPC_FUNC
			ret = xUvac_EU_dpc(request, pData, pDataLen);
#endif
			//not supported now
			break;
		default:
			break;
	}

	if (request != SET_CUR) {
		printf("%s: CS=0x%02X, request = 0x%X,  pData=0x%X, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	}

	if (ret && request == SET_CUR){
		*pDataLen = 0;
	}

	return ret;
}

#if 0 //sample code for customized 2nd EU callback
BOOL xUvacEU2_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen){
	BOOL ret = FALSE;

	if (request == SET_CUR){
		printf("%s: SET_CUR, CS=0x%02X, request = 0x%X,  pData=0x%X, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	}
	switch (CS){
		case EU_CONTROL_ID_01:
		case EU_CONTROL_ID_02:
		case EU_CONTROL_ID_03:
		case EU_CONTROL_ID_04:
		case EU_CONTROL_ID_05:
		case EU_CONTROL_ID_06:
		case EU_CONTROL_ID_07:
		case EU_CONTROL_ID_08:
		case EU_CONTROL_ID_09:
		case EU_CONTROL_ID_10:
		case EU_CONTROL_ID_11:
		case EU_CONTROL_ID_12:
		case EU_CONTROL_ID_13:
		case EU_CONTROL_ID_14:
		case EU_CONTROL_ID_15:
		case EU_CONTROL_ID_16:
			//not supported now
			break;
		default:
			break;
	}

	if (request != SET_CUR) {
		printf("%s: CS=0x%02X, request = 0x%X,  pData=0x%X, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	}

	if (ret && request == SET_CUR){
		*pDataLen = 0;
	}

	return ret;
}
#endif

