////////////////////////////////////////////////////////////////////////////////
//#include "SysCommon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hid_def.h"
#include "UVAC.h"
//#include "CFU_flow.h"
#include "prjcfg.h"
#include <kwrap/task.h>
#include <kwrap/util.h>
#include "kwrap/semaphore.h"
#include <kwrap/debug.h>
////////////////////////////////////////////////////////////////////////////////

//#define THIS_DBGLVL        2 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
///////////////////////////////////////////////////////////////////////////////
//#define __MODULE__          CFU
//#define __DBGLVL__          ((THIS_DBGLVL>=PRJ_DBG_LVL)?THIS_DBGLVL:PRJ_DBG_LVL)
//#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
//#define __DBGFLT__          "[cap]"
//#define __DBGFLT__          "[cb]"
//#define __DBGFLT__          "[set]"
//#define __DBGFLT__          "[zoom]"
//#define __DBGFLT__          "[af]"
//#include "DebugModule.h"
#define CFU_STATE_IDLE        0 //cfu is enabled. temp buffer is released from ipl
#define CFU_STATE_IN_PROCESS  1 //signature verify and firmware burning is in process
#define CFU_STATE_DISABLE     2 //cfu is disabled. temp buffer is returned back to system
int cfu_state = CFU_STATE_DISABLE;
static UINT8 update_content_resp[sizeof(CFU_UPDATE_CONTENT_RESP)+1] = {0};

static THREAD_HANDLE CFU_TSK_ID = 0;
SEM_HANDLE    CFU_SEM_ID = 0;

THREAD_DECLARE(Cfu_Tsk, arglist)
{
	CFU_CONTENT_STATUS status;
	UINT32 resp_buf_size = sizeof(update_content_resp);
	int state;
	CFU_UPDATE_CONTENT_RESP cont_resp;
	
	THREAD_ENTRY();

	while(1){
		vos_util_delay_ms(3000);

		SEM_WAIT(CFU_SEM_ID);
		state = cfu_state;
		SEM_SIGNAL(CFU_SEM_ID);

		if(state != CFU_STATE_IN_PROCESS)
			continue;

		DBG_DUMP("verifying and burning firmware...\r\n");
		status = verify_and_burn_firmware();
		DBG_DUMP("verify_and_burn_firmware()=%x\r\n", status);
		memcpy(&cont_resp, &(update_content_resp[1]), sizeof(cont_resp));
		cont_resp.status = status;//not 4 byte aligned, cause exception
		memcpy(&(update_content_resp[1]), &cont_resp, sizeof(cont_resp));
		resp_buf_size = sizeof(update_content_resp);
		resp_buf_size = UVAC_WriteHidData(update_content_resp, resp_buf_size, -1);

		SEM_WAIT(CFU_SEM_ID);
		cfu_state = CFU_STATE_IDLE;
		SEM_SIGNAL(CFU_SEM_ID);

		state = CFU_STATE_IDLE;
	}
	
	THREAD_RETURN(0);
}

int Cfu_InstallID(void){
	THREAD_CREATE(CFU_TSK_ID, Cfu_Tsk, NULL, "Cfu_Tsk");
	if(CFU_TSK_ID == 0){
		DBG_ERR("fail to create cfu task\n");
		return -1;
	}
	SEM_CREATE(CFU_SEM_ID, 1);
	return 0;
}

BOOL cfu_usb_hid_cb(UINT8 request, UINT8 report_type, UINT8 report_id, UINT8 *p_data, UINT32 *p_dataLen)
{
	BOOL ret = TRUE;
	static int init = 0;

	if(!init){
		THREAD_RESUME(CFU_TSK_ID);
		init = 1;
	}

	switch(request)
	{
		case GET_REPORT:
			if (HID_REPORT_TYPE_FEATURE == report_type) {//"query firmware version" command
				CFU_FW_VERSION fw_ver = {0};
				UINT32 report_len;

				//memset(&fw_ver, 0, sizeof(fw_ver));
				fw_ver.header.component_count = 1;
				fw_ver.header.protocol_version = 0x4;
				fw_ver.component[0].fw_ver = cfu_get_firmware_version();
				fw_ver.component[0].component_id = 1;
				#if (CFU_GET_REATURE_REPORT_ID != 0)
				report_len = sizeof(CFU_FW_VERSION) + 1;
				*p_data = CFU_GET_REATURE_REPORT_ID;
				memcpy(&p_data[1], &fw_ver, *p_dataLen);
				#else
				report_len = sizeof(CFU_FW_VERSION);
				memcpy(p_data, &fw_ver, *p_dataLen);
				#endif
				if (report_len != *p_dataLen) {
					DBG_WRN("Size of CFU_FW_VERSION(%d) != %d! \r\n", report_len, *p_dataLen);
				}
			} else {
				DBG_ERR("GET_REPORT/report_type(%d) is not supported\n", report_type);
				ret = FALSE;
			}
			break;
		case SET_REPORT:
			if (HID_REPORT_TYPE_OUTPUT == report_type) {
				if (report_id == CFU_UPDATE_OFFER_REPORT_ID) {//"update offer" command
					CFU_UPDATE_OFFER offer = {0};
					CFU_UPDATE_OFFER_RESP offer_resp = {0};
					UINT8 offer_resp_buf[sizeof(CFU_UPDATE_OFFER_RESP)+1] = {0};
					UINT32 resp_buf_size = sizeof(offer_resp_buf);

					memcpy(&offer, &p_data[1], sizeof(CFU_UPDATE_OFFER));
					//DBG_DUMP("ComponentID=0x%X, Token=0x%X, FW Ver=0x%X, Protocol=0x%X\r\n", offer.component_id, offer.token, offer.fw_ver, offer.protocol_ver);

					SEM_WAIT(CFU_SEM_ID);

					if(cfu_state == CFU_STATE_DISABLE) {//system is busy in streaming, temp buffer is not available
						offer_resp.status = FIRMWARE_UPDATE_OFFER_BUSY;
						offer_resp.RR = ((offer.token << 24) | 0xFF);
					} else if(cfu_state == CFU_STATE_IDLE)//only when cfu is idle should new update offer be accepted
						offer_resp.status = cfu_process_update_offer(&offer, &offer_resp);
					else//may be verifying or burning firmware, reject new update offer
						offer_resp.status = FIRMWARE_UPDATE_OFFER_BUSY;

					SEM_SIGNAL(CFU_SEM_ID);

					//send response to Host
					offer_resp.token = offer.token;
					offer_resp_buf[0] = report_id;
					memcpy(&offer_resp_buf[1], &offer_resp, sizeof(CFU_UPDATE_OFFER_RESP));
					resp_buf_size = UVAC_WriteHidData(offer_resp_buf, (UINT32)resp_buf_size, -1);

				} else if (report_id == CFU_UPDATE_CONTENT_REPORT_ID) {//"update content" command
					CFU_UPDATE_CONTENT content = {0};
					CFU_UPDATE_CONTENT_RESP cont_resp = {0};
					UINT8 content_resp[sizeof(CFU_UPDATE_CONTENT_RESP)+1] = {0};
					UINT32 resp_buf_size = sizeof(content_resp);

					memcpy(&content, &p_data[1], sizeof(CFU_UPDATE_CONTENT));
					//DBG_DUMP("flags=0x%X, data_length=%d, seq=%d, addr=0x%X\r\n", content.flags, content.data_length, content.seq_num, content.fw_addr);

					SEM_WAIT(CFU_SEM_ID);

					//send response to Host
					cont_resp.seq_num = content.seq_num;
					if(cfu_state == CFU_STATE_IDLE)
						cont_resp.status = cfu_process_update_content(&content);
					else if(cfu_state == CFU_STATE_DISABLE)
						cont_resp.status = FIRMWARE_UPDATE_ERROR_PREPARE;
					else//cfu_state == CFU_STATE_IN_PROCESS
						cont_resp.status = FIRMWARE_UPDATE_SWAP_PENDING;
					content_resp[0] = report_id;
					memcpy(&content_resp[1], &cont_resp, sizeof(CFU_UPDATE_OFFER_RESP));

					if(cont_resp.status == FIRMWARE_UPDATE_SUCCESS && content.flags == FIRMWARE_UPDATE_FLAG_LAST_BLOCK){
						update_content_resp[0] = report_id;
						memcpy(&update_content_resp[1], &cont_resp, sizeof(CFU_UPDATE_OFFER_RESP));
						cfu_state = CFU_STATE_IN_PROCESS;
						SEM_SIGNAL(CFU_SEM_ID);
					}else{
						SEM_SIGNAL(CFU_SEM_ID);
						UVAC_WriteHidData(content_resp, resp_buf_size, -1);
					}

				} else {
					DBG_ERR("SET_REPORT/HID_REPORT_TYPE_OUTPUT/report_id(%d) is not supported\n", report_id);
					ret = FALSE;
				}
			} else {
				DBG_ERR("SET_REPORT/report_type(%d) is not supported\n", report_type);
				ret = FALSE;
			}
			break;
		default:
			DBG_ERR("request(%d) is not supported\n", request);
			ret = FALSE;
			break;
	}
	return ret;
}

UINT32 cfu_enable(void)
{
	SEM_WAIT(CFU_SEM_ID);

	if(cfu_state == CFU_STATE_DISABLE)
		cfu_state = CFU_STATE_IDLE;

	SEM_SIGNAL(CFU_SEM_ID);

	return 0;
}

UINT32 cfu_disable(void)
{
	while(1){

		SEM_WAIT(CFU_SEM_ID);

		if(cfu_state == CFU_STATE_IN_PROCESS){
			SEM_SIGNAL(CFU_SEM_ID);
			DBG_DUMP("cfu is still in process\r\n");
			vos_util_delay_ms(1000);
			continue;
		}

		cfu_state = CFU_STATE_DISABLE;

		if(_cfu_disable())
			DBG_ERR("_cfu_disable() fail\r\n");

		SEM_SIGNAL(CFU_SEM_ID);
		break;
	}

	return 0;
}
