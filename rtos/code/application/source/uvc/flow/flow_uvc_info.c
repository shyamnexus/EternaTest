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
#include "hid_def.h"
#include "PStore.h"
#include "sys_mempool.h"

#define PS_UVC_USR_PARAM         "UVC_USR_P"
#define PS_UVC_MTE_PARAM         "UVC_MTE_P"
#define PS_UVC_VEN_PARAM         "UVC_VEN_P"
#define _TO_DO_         0

void check_and_restore_header(void)
{
	UINT32 ErrorSection = 0;
	UINT8 *psBuf;
	psBuf = (UINT8 *)mempool_pstore;

	PStore_CheckTotalSection(&ErrorSection);
	if (ErrorSection == PS_HEAD1)
		PStore_RestoreHeader((UINT8 *)psBuf, PS_HEAD1);
	else if (ErrorSection == PS_HEAD2)
		PStore_RestoreHeader((UINT8 *)psBuf, PS_HEAD2);
	else
		DBG_DUMP("PStore header OK %d\r\n", psBuf);
}

void set_update_onehead(void)
{
	PStore_EnableUpdateHeader(PS_UPDATE_ONEHEAD);
}

UINT32 gen_custom_crc(void)
{
	UINT32 usr_crc;
	usr_crc = uvc_custom_info.mic_state | uvc_custom_info.hdr_mode << 1 | uvc_custom_info.retouch << 2 | uvc_custom_info.afd_detect_result << 3;
	return usr_crc;
}

UINT32 comp_custom_crc(void)
{
	UINT32 usr_crc_gen;
	usr_crc_gen = gen_custom_crc();
	if (usr_crc_gen == uvc_custom_info.usr_crc)
		return 1;
	else
		return 0;
}

UVC_CUSTOM_INFO uvc_custom_info = {0};
UVC_MTE_INFO uvc_mte_info = {0};
void dump_uvc_custom_info(void)
{
	DBG_DUMP("###### uvc_custom_info ########\r\n");
	DBG_DUMP("info_size = %d\r\n", uvc_custom_info.info_size);
	DBG_DUMP("mic_state = %d\r\n", uvc_custom_info.mic_state);
	DBG_DUMP("hdr_mode  = %d\r\n", uvc_custom_info.hdr_mode);
	DBG_DUMP("retouch   = %d\r\n", uvc_custom_info.retouch);
#if (UVC_CACHE_SETTING_EN == DISABLE)
	DBG_DUMP("XU_Brightness    = %d\r\n", uvc_custom_info.xu_brightness);
	DBG_DUMP("XU_Contrast      = %d\r\n", uvc_custom_info.xu_contrast);
	DBG_DUMP("XU_Hue           = %d\r\n", uvc_custom_info.xu_hue);
	DBG_DUMP("XU_Saturation    = %d\r\n", uvc_custom_info.xu_saturation);
	DBG_DUMP("XU_Sharpness     = %d\r\n", uvc_custom_info.xu_sharpness);
	DBG_DUMP("XU_PowerFreq     = %d\r\n", uvc_custom_info.xu_power_freq);
	DBG_DUMP("XU_BacklightComp = %d\r\n", uvc_custom_info.xu_backlightcomp);
	DBG_DUMP("XU_WB_Auto       = %d\r\n", uvc_custom_info.xu_wb_auto);
	DBG_DUMP("XU_WB_Temp       = %d\r\n", uvc_custom_info.xu_wb_temp);
	DBG_DUMP("CT_ExpT_Auto     = %d\r\n", uvc_custom_info.ct_expt_auto);
	DBG_DUMP("CT_ExpT_Abs      = %d\r\n", uvc_custom_info.ct_expt_abs);
#endif
	DBG_DUMP("AFD_DetectResult = %d\r\n", uvc_custom_info.afd_detect_result);
	DBG_DUMP("check_shutter = %d\r\n", uvc_custom_info.check_shutter);
	DBG_DUMP("custom crc = %d\r\n", uvc_custom_info.usr_crc);
}
void reset_uvc_custom_info_all(void)//reset all setting for error recover
{
	uvc_custom_info.info_size = sizeof(UVC_CUSTOM_INFO);
	uvc_custom_info.mic_state       = 0;
	uvc_custom_info.hdr_mode        = 1;
	uvc_custom_info.retouch	        = 0;
	uvc_custom_info.check_shutter	= 0;
#if (UVC_CACHE_SETTING_EN == DISABLE)
	uvc_custom_info.xu_brightness = uvc_pu_ctrl.Brightness[PROPERTY_DEF];
	uvc_custom_info.xu_contrast = uvc_pu_ctrl.Contrast[PROPERTY_DEF];
	uvc_custom_info.xu_hue = uvc_pu_ctrl.Hue[PROPERTY_DEF];
	uvc_custom_info.xu_saturation = uvc_pu_ctrl.Saturation[PROPERTY_DEF];
	uvc_custom_info.xu_sharpness = uvc_pu_ctrl.Sharpness[PROPERTY_DEF];
	uvc_custom_info.xu_power_freq = uvc_pu_ctrl.PowerFreq[PROPERTY_DEF];
	uvc_custom_info.xu_backlightcomp = uvc_pu_ctrl.Backlight[PROPERTY_DEF];
	uvc_custom_info.xu_wb_auto = uvc_pu_ctrl.WB_Auto[PROPERTY_DEF];
	uvc_custom_info.xu_wb_temp = uvc_pu_ctrl.WB_Temp[PROPERTY_DEF];
	uvc_custom_info.ct_expt_auto = uvc_ct_ctrl.AE_Mode[PROPERTY_DEF];
	uvc_custom_info.ct_expt_abs = uvc_ct_ctrl.ExpoTime[PROPERTY_DEF];
#endif
	uvc_custom_info.afd_detect_result = 0xFF;
	uvc_custom_info.usr_crc = gen_custom_crc();
}

void reset_uvc_custom_info(void)//this is for reset to default command
{
	uvc_custom_info.info_size = sizeof(UVC_CUSTOM_INFO);
	uvc_custom_info.hdr_mode        = 1;
	uvc_custom_info.retouch	        = 0;
	uvc_custom_info.check_shutter	= 0;
#if (UVC_CACHE_SETTING_EN == DISABLE)
	uvc_custom_info.xu_brightness = uvc_pu_ctrl.Brightness[PROPERTY_DEF];
	uvc_custom_info.xu_contrast = uvc_pu_ctrl.Contrast[PROPERTY_DEF];
	uvc_custom_info.xu_hue = uvc_pu_ctrl.Hue[PROPERTY_DEF];
	uvc_custom_info.xu_saturation = uvc_pu_ctrl.Saturation[PROPERTY_DEF];
	uvc_custom_info.xu_sharpness = uvc_pu_ctrl.Sharpness[PROPERTY_DEF];
	uvc_custom_info.xu_power_freq = uvc_pu_ctrl.PowerFreq[PROPERTY_DEF];
	uvc_custom_info.xu_backlightcomp = uvc_pu_ctrl.Backlight[PROPERTY_DEF];
	uvc_custom_info.xu_wb_auto = uvc_pu_ctrl.WB_Auto[PROPERTY_DEF];
	uvc_custom_info.xu_wb_temp = uvc_pu_ctrl.WB_Temp[PROPERTY_DEF];
	uvc_custom_info.ct_expt_auto = uvc_ct_ctrl.AE_Mode[PROPERTY_DEF];
	uvc_custom_info.ct_expt_abs = uvc_ct_ctrl.ExpoTime[PROPERTY_DEF];
#endif

	uvc_custom_info.afd_detect_result = 0xFF;
	uvc_custom_info.usr_crc = gen_custom_crc();
}

void load_uvc_custom_info(void)
{
	PPSTORE_SECTION_HANDLE  pSection;
	UINT32  result;
//	UIAPPIPCAM_IPL_FUNC *pIPLFunc = UIAppIPCam_get_IPLFuncInfo();

	// Read uvc custom info from PStore
	if ((pSection = PStore_OpenSection(PS_UVC_USR_PARAM, PS_RDWR)) != E_PS_SECHDLER) {
		UINT32 uiPsFreeSpace = PStore_GetInfo(PS_INFO_FREE_SPACE);
		result = PStore_ReadSection((UINT8 *)&uvc_custom_info, 0, sizeof(UVC_CUSTOM_INFO), pSection);
		if ((result != E_PS_OK || uvc_custom_info.info_size != sizeof(uvc_custom_info)) && uiPsFreeSpace) {
			DBG_DUMP("PStore reset info.\r\n");
			memset(&uvc_custom_info, 0, sizeof(uvc_custom_info));
			uvc_custom_info.info_size = sizeof(UVC_CUSTOM_INFO);
			reset_uvc_custom_info_all();
			PStore_WriteSection((UINT8 *)&uvc_custom_info, 0, sizeof(UVC_CUSTOM_INFO), pSection);
			PStore_CloseSection(pSection);
		} else if ((result != E_PS_OK) && (uvc_custom_info.info_size == sizeof(uvc_custom_info)) &&
				   (uiPsFreeSpace == 0)) {
			//if current size is the same with previous, use PS_UPDATE instead
			//of PS_RDWR to prevent that PStore is no free space
			DBG_DUMP("PStore reset info for uiPsFreeSpace=0.\r\n");
			PStore_CloseSection(pSection);
			memset(&uvc_custom_info, 0, sizeof(uvc_custom_info));
			uvc_custom_info.info_size = sizeof(UVC_CUSTOM_INFO);
			reset_uvc_custom_info_all();
			if ((pSection = PStore_OpenSection(PS_UVC_USR_PARAM, PS_UPDATE)) != E_PS_SECHDLER) {
				PStore_WriteSection((UINT8 *)&uvc_custom_info, 0, sizeof(UVC_CUSTOM_INFO), pSection);
				PStore_CloseSection(pSection);
			} else {
				DBG_ERR("Update uvc param fail in load_uvc_custom_info\r\n");
			}
		} else if (comp_custom_crc() == 0){
			DBG_ERR("custom CRC fail, reset info\r\n");
			reset_uvc_custom_info_all();
			if (comp_custom_crc())
			{
				PStore_WriteSection((UINT8 *)&uvc_custom_info, 0, sizeof(UVC_CUSTOM_INFO), pSection);
				PStore_CloseSection(pSection);
			} else {
				DBG_ERR("can't get correct CRC\r\n");
			}
		} else {
			PStore_CloseSection(pSection);
		}

		//some settings need to apply after booting
#if _TO_DO_
//#if IPCAM_UVAC_DRAMMODE_FUNC
//		if (isMTE_DRAMMODE)
//			pIPLFunc[0].sHDR.eHDREnable = 0;
//		else
//#endif
//			pIPLFunc[0].sHDR.eHDREnable = uvc_custom_info.hdr_mode;//HDR
#endif

		//retouch
#if (UVC_CACHE_SETTING_EN == DISABLE)
		uvc_pu_ctrl.Brightness[PROPERTY_CUR] = uvc_custom_info.xu_brightness;
		uvc_pu_ctrl.Contrast[PROPERTY_CUR] = uvc_custom_info.xu_contrast;
		uvc_pu_ctrl.Hue[PROPERTY_CUR] = uvc_custom_info.xu_hue;
		uvc_pu_ctrl.Saturation[PROPERTY_CUR] = uvc_custom_info.xu_saturation;
		uvc_pu_ctrl.Sharpness[PROPERTY_CUR] = uvc_custom_info.xu_sharpness;
		uvc_pu_ctrl.Backlight[PROPERTY_CUR] = uvc_custom_info.xu_backlightcomp;
		uvc_pu_ctrl.WB_Auto[PROPERTY_CUR] = uvc_custom_info.xu_wb_auto;
		uvc_pu_ctrl.WB_Temp[PROPERTY_CUR] = uvc_custom_info.xu_wb_temp;
		uvc_ct_ctrl.AE_Mode[PROPERTY_CUR] = uvc_custom_info.ct_expt_auto;
		uvc_ct_ctrl.ExpoTime[PROPERTY_CUR] = uvc_custom_info.ct_expt_abs;

		//DBG_WRN("afd_detect_result = %d\r\n", uvc_custom_info.afd_detect_result);

		if(uvc_custom_info.xu_power_freq==3) { // Auto Flicker Case
			if(uvc_custom_info.afd_detect_result == 0xFF) {
				AE_AlgSetUIInfo(0, AE_SEL_AFD_UNDETERMINE_FREQ, AE_FLICKER_50HZ);
			} else {
				AE_AlgSetUIInfo(0, AE_SEL_AFD_UNDETERMINE_FREQ, uvc_custom_info.afd_detect_result);
			}
		}

		//uvc_pu_ctrl.PowerFreq[PROPERTY_DEF] = uvc_custom_info.xu_power_freq;
		uvc_pu_ctrl.PowerFreq[PROPERTY_CUR] = uvc_custom_info.xu_power_freq;
#endif
#if _TO_DO_
//		if(uvc_custom_info.retouch == 1) {
//			iq_set_ui_info(0, IQ_UI_IMAGEEFFECT, 31);//RETOUCH ON
//		} else {
//			iq_set_ui_info(0, IQ_UI_IMAGEEFFECT, 0);//RETOUCH OFF
//		}
#endif
		//DBG_WRN("afd_undetermine_freq = %d\r\n", uvc_custom_info.xu_power_freq);
		//DBG_WRN("xu_power_freq = %d\r\n", uvc_custom_info.xu_power_freq);
		//DBG_WRN("uvc_powerfreq_def = %d\r\n", uvc_pu_ctrl.PowerFreq[PROPERTY_DEF]);

	} else {
		DBG_ERR("open USR fail \r\n");
		reset_uvc_custom_info_all();
		if ((pSection = PStore_OpenSection(PS_UVC_USR_PARAM, PS_RDWR | PS_CREATE)) != E_PS_SECHDLER) {
			PStore_WriteSection((UINT8 *)&uvc_custom_info, 0, sizeof(UVC_CUSTOM_INFO), pSection);
			PStore_CloseSection(pSection);
		} else {
			DBG_ERR("Write param fail\r\n");
		}
	}
}
void save_uvc_custom_info()
{
	PPSTORE_SECTION_HANDLE  pSection;
	UINT32 error = 0;
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_
	DisableNORWriteProtect();
#endif
	uvc_custom_info.info_size = sizeof(UVC_CUSTOM_INFO);
	uvc_custom_info.usr_crc = gen_custom_crc();
	UINT32 uiPsFreeSpace = PStore_GetInfo(PS_INFO_FREE_SPACE);
	if (uiPsFreeSpace == 0) {
		if ((pSection = PStore_OpenSection(PS_UVC_USR_PARAM, PS_UPDATE)) != E_PS_SECHDLER) {
			PStore_WriteSection((UINT8 *)&uvc_custom_info, 0, sizeof(UVC_CUSTOM_INFO), pSection);
			PStore_CloseSection(pSection);
		} else {
			DBG_ERR("Update param fail\r\n");
		}
	} else {
		if ((pSection = PStore_OpenSection(PS_UVC_USR_PARAM, PS_RDWR | PS_CREATE)) != E_PS_SECHDLER) {
			PStore_WriteSection((UINT8 *)&uvc_custom_info, 0, sizeof(UVC_CUSTOM_INFO), pSection);
			PStore_CloseSection(pSection);
		} else {
			DBG_ERR("Write param fail\r\n");
		}
	}

	if (PStore_CheckSection(PS_UVC_USR_PARAM, &error) != E_PS_OK) {
		DBG_ERR("PStore_CheckSection\r\n");
	}

	if (error != E_PS_OK) {
		DBG_ERR("PStore error msg :%x\r\n", error);
	}
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_
	EnableNORWriteProtect();
#endif
}

void dump_uvc_mte_info(void)
{
	UINT32  i;
	DBG_DUMP("###### uvc_mte_info ########\r\n");
	DBG_DUMP("info_size = %d\r\n", uvc_mte_info.info_size);
	DBG_DUMP("serial_number = ");
	for (i = 0; i<14; i++)
		DBG_DUMP("%x ", uvc_mte_info.serial_number[i]);
	DBG_DUMP("\r\n");
	DBG_DUMP("color_name = ");
	for (i = 0; i<16; i++)
		DBG_DUMP("%x ", uvc_mte_info.color_name[i]);
	DBG_DUMP("\r\n");
	DBG_DUMP("LED_lv0 = %d\r\n", uvc_mte_info.LED_lv0);
	DBG_DUMP("LED_lv1 = %d\r\n", uvc_mte_info.LED_lv1);
	DBG_DUMP("LED_lv2 = %d\r\n", uvc_mte_info.LED_lv2);
	DBG_DUMP("LED_lv3 = %d\r\n", uvc_mte_info.LED_lv3);
	DBG_DUMP("Capability = ");
	for (i = 0; i<16; i++)
		DBG_DUMP("%x ", uvc_mte_info.capability[i]);
	DBG_DUMP("\r\n");
	DBG_DUMP("check_sensor = %d\r\n", uvc_mte_info.check_sensor);
	DBG_DUMP("ccm_type = %d\r\n", uvc_mte_info.ccm_type);
	DBG_DUMP("shutter_dir = %d\r\n", uvc_mte_info.shutter_dir);
}

void reset_uvc_mte_info(void)
{
	uvc_mte_info.info_size = sizeof(UVC_MTE_INFO);
	memset(&uvc_mte_info.serial_number, 0x30, sizeof(uvc_mte_info.serial_number));
	memset(&uvc_mte_info.color_name, 0, sizeof(uvc_mte_info.color_name));
	uvc_mte_info.LED_lv0        = 0x14;//GPIO8
	uvc_mte_info.LED_lv1        = 0xE0;//GPIO9
	uvc_mte_info.LED_lv2        = 0x48;//GPIO10
	uvc_mte_info.LED_lv3        = 0x48;//GPIO11
	memset(&uvc_mte_info.capability, 0, sizeof(uvc_mte_info.capability));
}

void load_uvc_mte_info(void)
{
	PPSTORE_SECTION_HANDLE  pSection;
	UINT32  result;
	// Read uvc custom info from PStore
	if ((pSection = PStore_OpenSection(PS_UVC_MTE_PARAM, PS_RDWR)) != E_PS_SECHDLER) {
		UINT32 uiPsFreeSpace = PStore_GetInfo(PS_INFO_FREE_SPACE);
		result = PStore_ReadSection((UINT8 *)&uvc_mte_info, 0, sizeof(UVC_MTE_INFO), pSection);
		if ((result != E_PS_OK || uvc_mte_info.info_size != sizeof(uvc_mte_info)) && uiPsFreeSpace) {
			DBG_DUMP("PStore reset info.\r\n");
			memset(&uvc_mte_info, 0, sizeof(uvc_mte_info));
			uvc_mte_info.info_size = sizeof(UVC_MTE_INFO);
			reset_uvc_mte_info();
			PStore_WriteSection((UINT8 *)&uvc_mte_info, 0, sizeof(UVC_MTE_INFO), pSection);
			PStore_CloseSection(pSection);
		} else if ((result != E_PS_OK) && (uvc_mte_info.info_size == sizeof(uvc_mte_info)) &&
				   (uiPsFreeSpace == 0)) {
			//if current size is the same with previous, use PS_UPDATE instead
			//of PS_RDWR to prevent that PStore is no free space
			DBG_DUMP("PStore reset info for uiPsFreeSpace=0.\r\n");
			PStore_CloseSection(pSection);
			memset(&uvc_mte_info, 0, sizeof(uvc_mte_info));
			uvc_mte_info.info_size = sizeof(UVC_MTE_INFO);
			reset_uvc_mte_info();
			if ((pSection = PStore_OpenSection(PS_UVC_MTE_PARAM, PS_UPDATE)) != E_PS_SECHDLER) {
				PStore_WriteSection((UINT8 *)&uvc_mte_info, 0, sizeof(UVC_MTE_INFO), pSection);
				PStore_CloseSection(pSection);
			} else {
				DBG_ERR("Update uvc param fail in load_uvc_custom_info\r\n");
			}
		} else {
#if _TO_DO_//(UVCCAM_CUSTOMER_OPTION == UVCCAM_CUSTOMER_MSFT)
			if(uvc_mte_info.ccm_type == 0x00) {
				iq_set_ui_info(0, IQ_UI_CUSTOMER, IQ_CUSTOMER_OPTION_1);
				DBG_DUMP("***** CCM Type = IRCF.(%d) *****\r\n", uvc_mte_info.ccm_type);
			} else {
				iq_set_ui_info(0, IQ_UI_CUSTOMER, IQ_CUSTOMER_OPTION_2);
				DBG_DUMP("***** CCM Type = BGCF.(%d) *****\r\n", uvc_mte_info.ccm_type);
			}
#endif
			PStore_CloseSection(pSection);
		}

	} else {
		DBG_ERR("open MTE fail \r\n");
		reset_uvc_mte_info();
		if ((pSection = PStore_OpenSection(PS_UVC_MTE_PARAM, PS_RDWR | PS_CREATE)) != E_PS_SECHDLER) {
			PStore_WriteSection((UINT8 *)&uvc_mte_info, 0, sizeof(UVC_MTE_INFO), pSection);
			PStore_CloseSection(pSection);
		} else {
			DBG_ERR("Write param fail\r\n");
		}
	}
}
void save_uvc_mte_info()
{
	PPSTORE_SECTION_HANDLE  pSection;
	UINT32 error = 0;
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_
	DisableNORWriteProtect();
#endif
	uvc_mte_info.info_size = sizeof(UVC_MTE_INFO);
	UINT32 uiPsFreeSpace = PStore_GetInfo(PS_INFO_FREE_SPACE);
	if (uiPsFreeSpace == 0) {
		if ((pSection = PStore_OpenSection(PS_UVC_MTE_PARAM, PS_UPDATE)) != E_PS_SECHDLER) {
			PStore_WriteSection((UINT8 *)&uvc_mte_info, 0, sizeof(UVC_MTE_INFO), pSection);
			PStore_CloseSection(pSection);
		} else {
			DBG_ERR("Update param fail\r\n");
		}
	} else {
		if ((pSection = PStore_OpenSection(PS_UVC_MTE_PARAM, PS_RDWR | PS_CREATE)) != E_PS_SECHDLER) {
			PStore_WriteSection((UINT8 *)&uvc_mte_info, 0, sizeof(UVC_MTE_INFO), pSection);
			PStore_CloseSection(pSection);
		} else {
			DBG_ERR("Write param fail\r\n");
		}
	}

	if (PStore_CheckSection(PS_UVC_MTE_PARAM, &error) != E_PS_OK) {
		DBG_ERR("PStore_CheckSection\r\n");
	}

	if (error != E_PS_OK) {
		DBG_ERR("PStore error msg :%x\r\n", error);
	}
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_
	EnableNORWriteProtect();
#endif
}

