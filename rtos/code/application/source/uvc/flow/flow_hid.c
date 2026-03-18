#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/stdio.h>
#include <unistd.h>
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include <kwrap/task.h>
#include <time.h>
#include <comm/timer.h>
#include "comm/hwclock.h"
#include "UVAC.h"
#include "prjcfg.h"
#include "flow_uvc.h"
#include "flow_hid.h"
#include "flow_uvc_info.h"
#include "bin_info.h"
#include "hid_def.h"
#include "pwm.h"

#if HID_FUNC

#define _TO_DO_ 0
#define _ISP_TO_DO_ 0

#define THIS_DBGLVL       2 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER

BOOL isMTE_TESTMODE = 0;
#if UVC_LED
BOOL isMTE_LED0_ON;
BOOL isMTE_LED1_ON;
BOOL isMTE_LED2_ON;
BOOL isMTE_LED3_ON;
#endif

#if (THIS_DBGLVL>=5)
static void DumpMemory(UINT32 Addr, UINT32 Size, UINT32 Alignment)
{
	UINT32 i;
	UINT8 *pBuf = (UINT8 *)Addr;
	for (i = 0; i < Size; i++) {
		if (i % Alignment == 0) {
			DBG_DUMP("\r\n");
		}
		DBG_DUMP("0x%02X ", *(pBuf + i));
	}
	DBG_DUMP("\r\n");
}
#endif
static char dataElementValue[] = "IAmAnErrorMessage!";
void telemetry_invoke_host(void)
{
	_ALIGNED(4) UINT8 resp_buf[2];
	UINT32 resp_buf_size = sizeof(resp_buf);

	resp_buf[0] = TELEMETRY_DEVICE_REPORT_ID;
	resp_buf[1] = 1;
	resp_buf_size = UVAC_WriteHidData(resp_buf, resp_buf_size, -1);
	resp_buf[1] = 0;
	resp_buf_size = UVAC_WriteHidData(resp_buf, resp_buf_size, -1);
	//dataElementID = ElementID;
	//dataElementValue = ElementValue;
}
static BOOL telemetry_handler(UINT8 request, UINT8 *p_data, UINT32 *p_dataLen)
{
	UINT8 report_id = p_data[0];
	static UINT8 last_msg_id = 0xFF;
	static UINT8 last_element_id = 0xFF;
	UINT16 payload_len = 0;
	UINT16 metadata_len = 0;

	if (request == SET_REPORT) {
		UINT8 msg_id = p_data[1];

		//query
		payload_len = p_data[2]<<8 | p_data[3];//big endian
		DBG_DUMP("SET_REPORT:report_id=0x%X, msg_id=0x%X, len=%d\r\n", report_id, msg_id, payload_len);
		if (msg_id == TELEMETRY_MSG_ID_LOCALE) {
			UINT32 uiYear, uiMonth, uiDay;
			UINT32 uiHour, uiMinute, uiSecond;
			//payload data begin from p_data[4]
			//Example: Len-USD2018-06-18T13:08:25
			//"2018-06-18" begin form p_data[11]
			//"13:08:25" begin form p_data[22]

			if (3 == sscanf_s((char *)&p_data[11], "%d-%d-%d", &uiYear, &uiMonth, &uiDay)) {
				DBG_DUMP("Y:%d, M:%d, D:%d\r\n", uiYear, uiMonth, uiDay);
				if (3 == sscanf_s((char *)&p_data[22], "%d:%d:%d", &uiHour, &uiMinute, &uiSecond)) {
					struct tm Curr_DateTime ={0};

					DBG_DUMP("H:%d, M:%d, S:%d\r\n", uiHour, uiMinute, uiSecond);
					Curr_DateTime.tm_year = uiYear ;
					Curr_DateTime.tm_mon = uiMonth ;
					Curr_DateTime.tm_mday = uiDay ;
					Curr_DateTime.tm_hour = uiHour ;
					Curr_DateTime.tm_min = uiMinute ;
					Curr_DateTime.tm_sec = uiSecond ;
				    hwclock_set_time(TIME_ID_CURRENT, Curr_DateTime, 0);
				}
		    }
		} else if (msg_id == TELEMETRY_MSG_ID_CLIENT) {
			last_element_id = p_data[4];
		} else if (msg_id == TELEMETRY_MSG_ID_VERSION || msg_id == TELEMETRY_MSG_ID_ENUM || msg_id == TELEMETRY_MSG_ID_DEVICE) {
			//do nothing
		} else {
			DBG_ERR("unknown msg id = %d", msg_id);
			return FALSE;
		}
		last_msg_id = msg_id;
	} else {
		//response
		p_data[0] = TELEMETRY_CLIENT_REPORT_ID;
		p_data[1] = last_msg_id;

		if (last_msg_id == TELEMETRY_MSG_ID_VERSION) {
			payload_len = 1;
			//The protocol version corresponding to the current specification has a constant value 0x1.
			p_data[4] = 0x1;
		} else if (last_msg_id == TELEMETRY_MSG_ID_ENUM) {
			payload_len = 1;
			//Cici only supports Telemetry kind of even, so returns 0x20.
			p_data[4] = 0x20;
		} else if (last_msg_id == TELEMETRY_MSG_ID_LOCALE) {
			payload_len = 1;
			//locale response = { 0x9A, 0x20, 0x0, 0x1, 0x1 }
			p_data[4] = 0x1;
		} else if (last_msg_id == TELEMETRY_MSG_ID_CLIENT) {
			p_data[4] = last_element_id;
			if (last_element_id == TELEMETRY_ELEMENT_ID_CURRENT_FW) {
				extern BININFO bin_info;
				UINT32 ver = bin_info.Resv[BININFO_RESV_IDX_VER];
				char curr_fw[6];
				sprintf(&curr_fw[0],"%d", (ver >> 24));
				curr_fw[1] = '.';
				sprintf(&curr_fw[2],"%d", ((ver >> 8) & 0x00FFFF));
				curr_fw[4] = '.';
				sprintf(&curr_fw[5],"%d", (ver & 0x000000FF));
				memcpy(&p_data[5], curr_fw, strlen(curr_fw));
				payload_len = 1 + strlen(curr_fw);//element id + string length
			} else if (last_element_id == TELEMETRY_ELEMENT_ID_DEVICE_MODEL) {
				char device_model[23] = "Microsoft Modern Webcam";
				memcpy(&p_data[5], device_model, strlen(device_model));
				payload_len = 1 + strlen(device_model);//element id + string length
			} else if (last_element_id == TELEMETRY_ELEMENT_ID_DEVICE_SN) {
				memcpy(&p_data[5], &uvc_mte_info.serial_number[0], sizeof(uvc_mte_info.serial_number));
				payload_len = 1 + sizeof(uvc_mte_info.serial_number);//element id + string length
			} else if (last_element_id == TELEMETRY_ELEMENT_ID_ERROR_MSG) {
				char err_msg[] = "error msg";
				memcpy(&p_data[5], err_msg, strlen(err_msg));
				payload_len = 1 + strlen(err_msg);//element id + string length
			} else {
				DBG_ERR("unknown last_element_id = %d", last_element_id);
				return FALSE;
			}
		} else if (last_msg_id == TELEMETRY_MSG_ID_DEVICE) {
			p_data[1] = TELEMETRY_MSG_ID_DEVICE_RESPONSE;
			payload_len = 1 + strlen(dataElementValue);
			p_data[4] = 0x20;
			metadata_len = 2;
			p_data[5] = metadata_len >> 8;
			p_data[6] = metadata_len & 0xFF;
			p_data[7] = TELEMETRY_ELEMENT_ID_ERROR_MSG;
			memcpy(&p_data[8], dataElementValue, strlen(dataElementValue));
		} else {
			DBG_ERR("unknown last_msg_id = %d", last_msg_id);
			return FALSE;
		}
		//payload_len = p_data[2]<<8 | p_data[3];//big endian
		p_data[2] = payload_len >> 8;
		p_data[3] = payload_len & 0xFF;
		DBG_DUMP("GET_REPORT:report_id=0x%X, msg_id=0x%X, element_id=%x, len=%d\r\n", report_id, last_msg_id, last_element_id, payload_len);
	}
	*p_dataLen = 2 + 2 + payload_len;
	if (metadata_len) {
		*p_dataLen = *p_dataLen + 2 + metadata_len;
	}
	if (*p_dataLen > 64) {
		DBG_ERR("NOT support len(%d) > 64\r\n", *p_dataLen);
		return FALSE;
	}
	#if (THIS_DBGLVL>=5)
	DumpMemory((UINT32) p_data, *p_dataLen, 16);
	#endif
	return TRUE;
}

BOOL m_UvacHidCB(UINT8 request, UINT16 value, UINT8 *p_data, UINT32 *p_dataLen)
{
	BOOL ret = TRUE;
	UINT8 report_type, report_id;
#if _ISP_TO_DO_
	static UINT8 mte_manualexpt_idx = 6; // 30ms
	static UINT32 mte_manual_gain = 0x80; // 1x
#endif
	BOOL get_report_for_vc = FALSE;
	static UINT8 last_cmd_id = 0;

	report_type = value >> 8;
	report_id = value & 0xFF;
	DBG_DUMP(":request = 0x%X, ReportType=0x%X, ReportID=0x%X, *p_dataLen=%d\r\n", request, report_type, report_id, *p_dataLen);
#if (THIS_DBGLVL>=5)
	DBG_DUMP(":request = 0x%X, ReportType=0x%X, ReportID=0x%X, *p_dataLen=%d\r\n", request, report_type, report_id, *p_dataLen);
	DumpMemory((UINT32) p_data, *p_dataLen, 16);
#endif
	extern BOOL cfu_usb_hid_cb(UINT8 request, UINT8 report_type, UINT8 report_id, UINT8 *p_data, UINT32 *p_dataLen);
	extern int cfu_state;
	switch(request)
	{
		case SET_IDLE:
		case SET_PROTOCOL:
			ret = TRUE;
			break;
		case GET_REPORT:
			if (VC_RESPONSE_REPORT_ID == report_id) {
				get_report_for_vc = TRUE;
				//since App uses 0x27 0x01 to get response status
				//force going to the SET_REPORT flow for VC
				report_type = HID_REPORT_TYPE_FEATURE;
				p_data[1] = last_cmd_id;
			} else if (TELEMETRY_CLIENT_REPORT_ID == report_id) {
				ret = telemetry_handler(request, p_data, p_dataLen);
				break;
			} else {
				ret = cfu_usb_hid_cb(request, report_type, report_id, p_data, p_dataLen);
				break;
			}
		case SET_REPORT:
			if (HID_REPORT_TYPE_OUTPUT == report_type) {
				ret = cfu_usb_hid_cb(request, report_type, report_id, p_data, p_dataLen);
				if ((report_id == CFU_UPDATE_OFFER_REPORT_ID && ret == 1) && cfu_state == 0){
				}
			} else if (TELEMETRY_CLIENT_REPORT_ID == report_id) {
				ret = telemetry_handler(request, p_data, p_dataLen);
#if 0 //test only
			} else if (HID_REPORT_TYPE_FEATURE == report_type) {
				UINT8 cmd_id = p_data[1];
				_ALIGNED(4) UINT8 resp_buf[VC_MAX_SIZE + 1] = {0}; //including reporter ID
				UINT32 resp_buf_size;
				UINT8 status = VC_STATUS_OK;
				UINT8 payload_len = 0;
				last_cmd_id = cmd_id;
				if (report_id != VC_REQUEST_REPORT_ID) {
					DBG_WRN("Check Host Application!!!\r\n");
				}

				resp_buf[0] = VC_RESPONSE_REPORT_ID;
				resp_buf[1] = cmd_id;

				if (VC_CMD_GET_HDR == cmd_id) {
					payload_len = 1;
					resp_buf[4] = 123;//check current HDR status
					DBG_DUMP("GET HDR %d\r\n", resp_buf[4]);
				} else {
					ret = FALSE;
					status = VC_STATUS_FAILED;
				}

				resp_buf[2] = status;
				resp_buf[3] = payload_len;
				resp_buf_size = sizeof(resp_buf);
				resp_buf_size = UVAC_WriteHidData(resp_buf, resp_buf_size, -1);
				DBGD(resp_buf_size);
#endif
			} else if (HID_REPORT_TYPE_FEATURE == report_type) {
				//for camera control
				UINT8 cmd_id = p_data[1];
				_ALIGNED(4) UINT8 resp_buf[VC_MAX_SIZE + 1] = {0}; //including reporter ID
				UINT32 resp_buf_size;
				UINT8 status = VC_STATUS_OK;
				UINT8 payload_len = 0;
				last_cmd_id = cmd_id;
				if (report_id != VC_REQUEST_REPORT_ID) {
					DBG_WRN("Check Host Application!!!\r\n");
				}

				resp_buf[0] = VC_RESPONSE_REPORT_ID;
				resp_buf[1] = cmd_id;
				if (VC_CMD_GET_FW_VER == cmd_id) {
					extern BININFO bin_info;
					UINT32 ver = bin_info.Resv[BININFO_RESV_IDX_VER];
					payload_len = 4;
					resp_buf[4] = ver & 0x000000FF;
					resp_buf[5] = (ver >> 8) & 0x0000FF;
					resp_buf[6] = (ver >> 16) & 0x00FF;
					resp_buf[7] = ver >> 24;
					DBG_DUMP("GET VER %x%x%x%x\r\n", resp_buf[4], resp_buf[5], resp_buf[6], resp_buf[7]);
				} else if (VC_CMD_GET_SN == cmd_id) {
					payload_len = 14;
					memcpy(&resp_buf[4], &uvc_mte_info.serial_number[0], sizeof(uvc_mte_info.serial_number));
				} else if (VC_CMD_SET_HDR == cmd_id && isMTE_TESTMODE == 0) {
					payload_len = 0;
					DBG_DUMP("need to do SET HDR function %d\r\n", p_data[3]);
					//_TO_DO_
					if (p_data[3]) {
						//HDR ON
						uvc_custom_info.hdr_mode = p_data[3];
					} else {
						//HDR OFF
						uvc_custom_info.hdr_mode = p_data[3];
					}
					save_uvc_custom_info();
				} else if (VC_CMD_GET_HDR == cmd_id) {
					payload_len = 1;
				    //_TO_DO_
					resp_buf[4] = 0;//check current HDR status
					DBG_DUMP("need to do GET HDR function %d\r\n", p_data[4]);
				} else if (VC_CMD_SET_RETOUCH == cmd_id) {
					payload_len = 0;
					DBG_DUMP("SET RETOUCH %d\r\n", p_data[3]);
					#if _ISP_TO_DO_
					if (p_data[3]) {
						iq_set_ui_info(0, IQ_UI_IMAGEEFFECT, 31);//RETOUCH ON
						uvc_custom_info.retouch = p_data[3];
					} else {
						iq_set_ui_info(0, IQ_UI_IMAGEEFFECT, 0);//RETOUCH OFF
						uvc_custom_info.retouch = p_data[3];
					}
					#endif
					save_uvc_custom_info();
				} else if (VC_CMD_GET_RETOUCH == cmd_id) {
					payload_len = 1;
					#if _ISP_TO_DO_
					resp_buf[4] = (iq_get_ui_info(0, IQ_UI_IMAGEEFFECT))? 1 : 0;
					#endif
					DBG_DUMP("GET RETOUCH %d\r\n", p_data[4]);
				} else if (VC_CMD_SET_AWB == cmd_id) {
					payload_len = 0;
					#if _ISP_TO_DO_
					if (p_data[3]) {
						//0x01: Enhanced
						awb_set_ui_info(0, AWB_UI_PREFERENCE_MODE, AWB_PREFERENCE_ENHANCED);
						DBG_DUMP("SET AWB_PREFERENCE_ENHANCED\r\n");
					} else {
						//0x00: Standard
						awb_set_ui_info(0, AWB_UI_PREFERENCE_MODE, AWB_PREFERENCE_STANDARD);
						DBG_DUMP("SET AWB_PREFERENCE_STANDARD\r\n");
					}
					#endif
				} else if (VC_CMD_GET_AWB == cmd_id) {
					payload_len = 1;
					//HIDTODO: check current AWB status
				} else if (VC_CMD_SET_MIC_STA == cmd_id) {
					DBG_DUMP("SET UAC %d\r\n", p_data[3]);
					uvc_custom_info.mic_state = p_data[3];//UAC ON
					save_uvc_custom_info();
				} else if (VC_CMD_GET_MIC_STA == cmd_id) {
					payload_len = 1;
					resp_buf[4] = uvc_custom_info.mic_state;
					DBG_DUMP("GET UAC %d\r\n", resp_buf[4]);
				} else if (VC_CMD_GET_COLOR_NAME == cmd_id) {
					payload_len = 16;
					memcpy(&resp_buf[4], &uvc_mte_info.color_name[0], sizeof(uvc_mte_info.color_name));
				} else if (VC_CMD_GET_SENSOR_SN == cmd_id) {
					payload_len = 16;
					#if _ISP_TO_DO_
					UINT8 sensor_sn[4];
					sensor_get_info(0, SEN_GET_USER_DEFINE1, &sensor_sn);
					resp_buf[4] = sensor_sn[3];
					resp_buf[5] = sensor_sn[2];
					resp_buf[6] = sensor_sn[1];
					resp_buf[7] = sensor_sn[0];
					sensor_get_info(0, SEN_GET_USER_DEFINE1, &sensor_sn);
					resp_buf[8] = sensor_sn[3];
					resp_buf[9] = sensor_sn[2];
					resp_buf[10] = sensor_sn[1];
					resp_buf[11] = sensor_sn[0];
					sensor_get_info(0, SEN_GET_USER_DEFINE1, &sensor_sn);
					resp_buf[12] = sensor_sn[3];
					resp_buf[13] = sensor_sn[2];
					resp_buf[14] = sensor_sn[1];
					resp_buf[15] = sensor_sn[0];
					sensor_get_info(0, SEN_GET_USER_DEFINE1, &sensor_sn);
					resp_buf[16] = sensor_sn[3];
					resp_buf[17] = sensor_sn[2];
					resp_buf[18] = sensor_sn[1];
					resp_buf[19] = sensor_sn[0];
					#endif
				} else if (VC_CMD_RESET_USB_POWER == cmd_id) {
					payload_len = 0;
				} else if (VC_CMD_GET_CAPABILITY == cmd_id) {
					payload_len = 16;
					UINT8 i;
					for (i = 0;i < 16;i++){
						resp_buf[4+i] = uvc_mte_info.capability[i];
					}
				} else if (VC_CMD_GET_WRITE_PROT == cmd_id) {
					payload_len = 1;
					#if _TO_DO_
					UINT8 wp_status = 0;
					wp_status = getNORWriteProtect();
					wp_status &= 0x3C;
					resp_buf[4] = wp_status ? 1 : 0;
					DBG_DUMP("GET WRITE_PROT  %d\r\n", resp_buf[4]);
					#endif
				} else if (VC_CMD_GET_SENSOR_CHK == cmd_id) {
					payload_len = 1;
					resp_buf[4] = uvc_mte_info.check_sensor;
					DBG_DUMP("SEN CHK %d\r\n", resp_buf[4]);
				} else if (VC_CMD_GET_CCM_TYPE == cmd_id) {
					payload_len = 1;
					resp_buf[4] = uvc_mte_info.ccm_type;
					DBG_DUMP("CCM TYPE %d\r\n", resp_buf[4]);
				} else if (VC_CMD_GET_SHUTTER_DIR == cmd_id) {
					payload_len = 1;
					resp_buf[4] = uvc_mte_info.shutter_dir;
					DBG_DUMP("SHU DIR %d\r\n", resp_buf[4]);
				} else if (VC_CMD_SET_SHUTTER_CHK == cmd_id) {
					payload_len = 0;
					uvc_custom_info.check_shutter = p_data[3];
					DBG_DUMP("SET SHU CHK %d\r\n", p_data[3]);
					save_uvc_custom_info();
				} else if (VC_CMD_GET_SHUTTER_CHK == cmd_id) {
					payload_len = 1;
					resp_buf[4] = uvc_custom_info.check_shutter;
					DBG_DUMP("GET SHU CHK %d\r\n", resp_buf[4]);
				} else if (VC_CMD_SET_TEST_MODE == cmd_id) {
					isMTE_TESTMODE = (p_data[3] >> 3) & 0x1;//get bit 3
					DBG_DUMP("SET TEST MODE %d\r\n", isMTE_TESTMODE);
				} else if (VC_CMD_GET_TEST_MODE == cmd_id) {
					payload_len = 4;
					resp_buf[4] = isMTE_TESTMODE << 3;//set bit 3;
					resp_buf[5] = 0;
					resp_buf[6] = 0;
					resp_buf[7] = 0;
					DBG_DUMP("GET TEST MODE %d\r\n", isMTE_TESTMODE);

				} else if (VC_CMD_SWITCH_ISP_MODE == cmd_id && isMTE_TESTMODE == 1) {
					payload_len = 0;
					#if 0
					// seems like no need DRAM mode
					if (p_data[3]){
						isMTE_DRAMMODE = TRUE;
					} else {
						isMTE_DRAMMODE = FALSE;
					}
					#endif
				} else if (VC_CMD_GET_ISP_MODE == cmd_id && isMTE_TESTMODE == 1) {
					payload_len = 1;
					#if 0
					// seems like no need DRAM mode
					resp_buf[4] = isMTE_DRAMMODE;// current ISP mode
					DBG_DUMP("GET ISP_MODE %d\r\n", resp_buf[4]);
					#endif
				} else if (VC_CMD_GET_SHUTTER_STA == cmd_id) {
					payload_len = 1;
#if (UVC_LED == ENABLE)
					resp_buf[4] = shutterFlag;
					DBG_DUMP("PS %d\r\n", shutterFlag);
#endif
				} else if (VC_CMD_SET_LED_STA == cmd_id && isMTE_TESTMODE == 1) {
					payload_len = 0;
#if (UVC_LED == ENABLE)
					if (p_data[3] & 0x1)
						isMTE_LED0_ON = TRUE;
					else
						isMTE_LED0_ON = FALSE;

					if (p_data[3] & 0x2)
						isMTE_LED1_ON = TRUE;
					else
						isMTE_LED1_ON = FALSE;

					if (p_data[3] & 0x4)
						isMTE_LED2_ON = TRUE;
					else
						isMTE_LED2_ON = FALSE;

					if (p_data[3] & 0x8)
						isMTE_LED3_ON = TRUE;
					else
						isMTE_LED3_ON = FALSE;
#endif
				} else if (VC_CMD_SET_SN == cmd_id && isMTE_TESTMODE == 1) {
					payload_len = 0;
					UINT8 chk_ascii = 0;
					UINT8 chk_ascii_pass = TRUE;
					for (chk_ascii = 0; chk_ascii < sizeof(uvc_mte_info.serial_number); chk_ascii++)
					{
						if ((p_data[chk_ascii+3] >= 0x30 && p_data[chk_ascii+3] <= 0x39) ||
							(p_data[chk_ascii+3] >= 0x41 && p_data[chk_ascii+3] <= 0x5A) ||
							(p_data[chk_ascii+3] >= 0x61 && p_data[chk_ascii+3] <= 0x7A)){
							continue;
						} else {
							chk_ascii_pass = FALSE;
							break;
						}
					}
					if (chk_ascii_pass) {
						memcpy(&uvc_mte_info.serial_number[0], &p_data[3], sizeof(uvc_mte_info.serial_number));
						save_uvc_mte_info();
					} else {
						ret = FALSE;
						status = VC_STATUS_INVALID_PARAM;
					}
				} else if (VC_CMD_SET_COLOR_NAME == cmd_id && isMTE_TESTMODE == 1) {
					payload_len = 0;
					memcpy(&uvc_mte_info.color_name[0], &p_data[3], sizeof(uvc_mte_info.color_name));
					save_uvc_mte_info();
				} else if (VC_CMD_SET_MANUAL_GAIN == cmd_id && isMTE_TESTMODE == 1) {
				#if _ISP_TO_DO_
					payload_len = 0;
					mte_manual_gain = (UINT32)((UINT32)p_data[4]<<8|(UINT32)p_data[3]);
					MTE_Manual_Exposure(mte_manualexpt_idx, mte_manual_gain);
					DBG_DUMP("VC_CMD_SET_MANUAL_GAIN === %X, %X (AG = %X)\r\n", p_data[3], p_data[4], ((UINT16)p_data[4]<<8|(UINT16)p_data[3]));
				#endif
				} else if (VC_CMD_GET_MANUAL_GAIN == cmd_id && isMTE_TESTMODE == 1) {
				#if _ISP_TO_DO_
					payload_len = 2;
					resp_buf[4] = (UINT8)(mte_manual_gain & 0xFF);
					resp_buf[5] = (UINT8)((mte_manual_gain>>8) & 0xFF);
					DBG_DUMP("VC_CMD_GET_MANUAL_GAIN === %X, %X (AG = %X)\r\n", resp_buf[4], resp_buf[5], ((UINT16)resp_buf[4]<<8|(UINT16)resp_buf[5]));
				#endif
				} else if (VC_CMD_SET_MANUAL_EXPO == cmd_id && isMTE_TESTMODE == 1) {
				#if _ISP_TO_DO_
					payload_len = 0;
					mte_manualexpt_idx = (UINT32)p_data[3];
					MTE_Manual_Exposure(mte_manualexpt_idx, mte_manual_gain);
					DBG_DUMP("VC_CMD_SET_MANUAL_EXPT === %X\r\n", p_data[3]);
				#endif
				} else if (VC_CMD_GET_MANUAL_EXPO == cmd_id && isMTE_TESTMODE == 1) {
				#if _ISP_TO_DO_
					payload_len = 1;
					resp_buf[4] = mte_manualexpt_idx;
					DBG_DUMP("VC_CMD_GET_MANUAL_EXPO === %X\r\n", resp_buf[3]);
				#endif
				} else if (VC_CMD_SET_LED_BRIGHT == cmd_id && isMTE_TESTMODE == 1) {
					payload_len = 0;
					if (p_data[3] < 0x4) {
						switch(p_data[3]&0x3){
							case 0:
								uvc_mte_info.LED_lv0 = p_data[4];
								break;
							case 1:
								uvc_mte_info.LED_lv1 = p_data[4];
								break;
							case 2:
								uvc_mte_info.LED_lv2 = p_data[4];
								break;
							case 3:
								uvc_mte_info.LED_lv3 = p_data[4];
								break;
						}
						save_uvc_mte_info();
#if (UVC_LED == ENABLE)
						PWM_CFG led_cfg;
						led_cfg.ui_prd = 255;
						led_cfg.ui_rise = 0;
						led_cfg.ui_on_cycle = 0;
						led_cfg.ui_inv = 0;
						led_cfg.ui_fall = uvc_mte_info.LED_lv1;
						if (isMTE_LED1_ON) {
							pwm_pwm_config(PWMID_9, &led_cfg);
							pwm_pwm_reload(PWMID_9);
						}
						led_cfg.ui_fall = uvc_mte_info.LED_lv3;
						if (isMTE_LED3_ON) {
							pwm_pwm_config(PWMID_11, &led_cfg);
							pwm_pwm_reload(PWMID_11);
						}

						led_cfg.ui_fall = uvc_mte_info.LED_lv0;
						if (isMTE_LED0_ON) {
							pwm_pwm_config(PWMID_8, &led_cfg);
							pwm_pwm_reload(PWMID_8);
						}
						led_cfg.ui_fall = uvc_mte_info.LED_lv2;
						if (isMTE_LED2_ON) {
							pwm_pwm_config(PWMID_10, &led_cfg);
							pwm_pwm_reload(PWMID_10);
						}

#endif
					} else {
						ret = FALSE;
						status = VC_STATUS_INVALID_PARAM;
					}
				} else if (VC_CMD_GET_LED_BRIGHT == cmd_id && isMTE_TESTMODE == 1) {
					if (p_data[3] < 0x4) {
						payload_len = 2;
						switch(p_data[3]&0x3){
							case 0:
								resp_buf[4] = p_data[3]&0x3;
								resp_buf[5] = uvc_mte_info.LED_lv0;
								break;
							case 1:
								resp_buf[4] = p_data[3]&0x3;
								resp_buf[5] = uvc_mte_info.LED_lv1;
								break;
							case 2:
								resp_buf[4] = p_data[3]&0x3;
								resp_buf[5] = uvc_mte_info.LED_lv2;
								break;
							case 3:
								resp_buf[4] = p_data[3]&0x3;
								resp_buf[5] = uvc_mte_info.LED_lv3;
								break;
						}
					} else {
						payload_len = 0;
						ret = FALSE;
						status = VC_STATUS_INVALID_PARAM;
					}
				} else if (VC_CMD_SET_CAPABILITY == cmd_id && isMTE_TESTMODE == 1) {
					payload_len = 0;
					UINT8 i;
					for (i = 0;i < 16;i++){
						uvc_mte_info.capability[i] = p_data[3+i];
					}
					save_uvc_mte_info();
				} else if (VC_CMD_SET_SENSOR_CHK == cmd_id && isMTE_TESTMODE == 1) {
					DBG_DUMP("SET SEN CHK %d\r\n", p_data[3]);
					uvc_mte_info.check_sensor = p_data[3];
					save_uvc_mte_info();
				} else if (VC_CMD_SET_CCM_TYPE == cmd_id && isMTE_TESTMODE == 1) {
					DBG_DUMP("SET CCM TYPE %d\r\n", p_data[3]);
					uvc_mte_info.ccm_type = p_data[3];
					save_uvc_mte_info();
				} else if (VC_CMD_SET_SHUTTER_DIR == cmd_id && isMTE_TESTMODE == 1) {
					DBG_DUMP("SET SHU DIR %d\r\n", p_data[3]);
					uvc_mte_info.shutter_dir = p_data[3];
					save_uvc_mte_info();
				} else if (VC_CMD_RESET_USER_CFG == cmd_id) {
					payload_len = 0;
					if (uvc_custom_info.hdr_mode == 0){
						reset_uvc_custom_info();
						save_uvc_custom_info();
					} else {
						reset_uvc_custom_info();
						#if _ISP_TO_DO_
						uvc_pu_ctrl.Brightness[PROPERTY_CUR] = uvc_pu_ctrl.Brightness[PROPERTY_DEF];
						uvc_pu_ctrl.Contrast[PROPERTY_CUR] = uvc_pu_ctrl.Contrast[PROPERTY_DEF];
						uvc_pu_ctrl.Saturation[PROPERTY_CUR] = uvc_pu_ctrl.Saturation[PROPERTY_DEF];
						uvc_pu_ctrl.Sharpness[PROPERTY_CUR] = uvc_pu_ctrl.Sharpness[PROPERTY_DEF];
						uvc_pu_ctrl.PowerFreq[PROPERTY_CUR] = uvc_pu_ctrl.PowerFreq[PROPERTY_DEF];
						uvc_pu_ctrl.WB_Auto[PROPERTY_CUR] = uvc_pu_ctrl.WB_Auto[PROPERTY_DEF];
						uvc_pu_ctrl.WB_Temp[PROPERTY_CUR] = uvc_pu_ctrl.WB_Temp[PROPERTY_DEF];
						uvc_ct_ctrl.AE_Mode[PROPERTY_CUR] = uvc_ct_ctrl.AE_Mode[PROPERTY_DEF];
						uvc_ct_ctrl.ExpoTime[PROPERTY_CUR] = uvc_ct_ctrl.ExpoTime[PROPERTY_DEF];
						uvc_isp_set_support_func();
						iq_set_ui_info(0, IQ_UI_IMAGEEFFECT, 0);//RETOUCH OFF
						if(uvc_custom_info.afd_detect_result == 0xFF) {
							AE_AlgSetUIInfo(0, AE_SEL_AFD_UNDETERMINE_FREQ, AE_FLICKER_50HZ);
						} else {
							AE_AlgSetUIInfo(0, AE_SEL_AFD_UNDETERMINE_FREQ, uvc_custom_info.afd_detect_result);
						}
						#endif
					}
				} else {
					ret = FALSE;
					status = VC_STATUS_FAILED;
				}
				resp_buf[2] = status;
				resp_buf[3] = payload_len;
				resp_buf_size = sizeof(resp_buf);
				if (get_report_for_vc) {
					*p_dataLen = resp_buf_size;
					if (resp_buf_size > 64) {
						UINT32 *p_addr;
						p_addr = (UINT32 *)p_data;
						*p_addr = (UINT32)resp_buf;
					} else {
						memcpy(p_data, resp_buf, resp_buf_size);
					}
				} else {
					UVAC_WriteHidData(resp_buf, resp_buf_size, -1);
				}
				if (VC_CMD_RESET_USB_POWER == cmd_id){
					//_TO_DO_
					DBG_WRN("need to reset power here\r\n");
				}
			} else {
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
