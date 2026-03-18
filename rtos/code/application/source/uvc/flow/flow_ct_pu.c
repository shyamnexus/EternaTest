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
#include "vendor_isp.h"
#include <pthread.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API

#if CT_PU_FUNC

#define UVCCAM_CUSTOMER_NORMAL	0
#define UVCCAM_CUSTOMER_MSFT	1
#define UVCCAM_CUSTOMER_OPTION	UVCCAM_CUSTOMER_NORMAL

#if (UVCCAM_CUSTOMER_OPTION == UVCCAM_CUSTOMER_MSFT)
// PU control
#define PU_BRIGHTNESS_EN                            ENABLE
#define PU_CONTRAST_EN                              ENABLE
#define PU_HUE_EN                                   DISABLE
#define PU_SATURATION_EN                            ENABLE
#define PU_SHARPNESS_EN                             ENABLE
#define PU_GAMMA_EN                                 DISABLE
#define PU_WB_TEMP_EN                               ENABLE
#define PU_BACKLIGHT_COMP_EN                        DISABLE
#define PU_GAIN_EN                                  DISABLE
#define PU_POWERLINE_FREQ_EN                        ENABLE
#define PU_WB_AUTO_EN                               ENABLE

// CT control
#define CT_SCANNING_MODE_EN                         DISABLE
#define CT_AE_MODE_EN                               ENABLE
#define CT_AE_PRIORITY_EN                           DISABLE
#define CT_EXPOSURE_TIME_ABSOLUTE_EN                ENABLE
#define CT_EXPOSURE_TIME_RELATIVE_EN                DISABLE
#define CT_FOCUS_RELATIVE_EN                        DISABLE
#define CT_ZOOM_ABSOLUTE_EN                         ENABLE
#define CT_ZOOM_RELATIVE_EN                         DISABLE
#define CT_PANTILT_ABSOLUTE_EN                      ENABLE
#define CT_PANTILT_RELATIVE_EN                      DISABLE
#define CT_FOCUS_ABSOLUTE_EN                        DISABLE
#define CT_PRIVACY_CONTROL_EN                       ENABLE
#define CT_AUTO_FOCUS_EN                            DISABLE
#define CT_REGION_OF_INTEREST_EN                    ENABLE
#else
// PU control
#define PU_BRIGHTNESS_EN                            DISABLE
#define PU_CONTRAST_EN                              DISABLE
#define PU_HUE_EN                                   DISABLE
#define PU_SATURATION_EN                            DISABLE
#define PU_SHARPNESS_EN                             DISABLE
#define PU_GAMMA_EN                                 DISABLE
#define PU_WB_TEMP_EN                               DISABLE
#define PU_BACKLIGHT_COMP_EN                        DISABLE
#define PU_GAIN_EN                                  DISABLE
#define PU_POWERLINE_FREQ_EN                        DISABLE
#define PU_WB_AUTO_EN                               DISABLE

// CT control
#define CT_SCANNING_MODE_EN                         DISABLE
#define CT_AE_MODE_EN                               ENABLE
#define CT_AE_PRIORITY_EN                           DISABLE
#define CT_EXPOSURE_TIME_ABSOLUTE_EN                ENABLE
#define CT_EXPOSURE_TIME_RELATIVE_EN                DISABLE
#define CT_FOCUS_RELATIVE_EN                        DISABLE
#define CT_ZOOM_ABSOLUTE_EN                         DISABLE
#define CT_ZOOM_RELATIVE_EN                         DISABLE
#define CT_PANTILT_ABSOLUTE_EN                      DISABLE
#define CT_PANTILT_RELATIVE_EN                      DISABLE
#define CT_FOCUS_ABSOLUTE_EN                        DISABLE
#define CT_PRIVACY_CONTROL_EN                       DISABLE
#define CT_AUTO_FOCUS_EN                            DISABLE
#define CT_REGION_OF_INTEREST_EN                    DISABLE
#endif

#if PU_BRIGHTNESS_EN
#define PU_BRIGHTNESS_MSK                           0x0001
#else
#define PU_BRIGHTNESS_MSK                           0x0000
#endif

#if PU_CONTRAST_EN
#define PU_CONTRAST_MSK                             0x0002
#else
#define PU_CONTRAST_MSK                             0x0000
#endif

#if PU_HUE_EN
#define PU_HUE_MSK                                  0x0004
#else
#define PU_HUE_MSK                                  0x0000
#endif

#if PU_SATURATION_EN
#define PU_SATURATION_MSK                           0x0008
#else
#define PU_SATURATION_MSK                           0x0000
#endif

#if PU_SHARPNESS_EN
#define PU_SHARPNESS_MSK                            0x0010
#else
#define PU_SHARPNESS_MSK                            0x0000
#endif

#if PU_GAMMA_EN
#define PU_GAMMA_MSK                                0x0020
#else
#define PU_GAMMA_MSK                                0x0000
#endif

#if PU_WB_TEMP_EN
#define PU_WB_TEMP_MSK                              0x0040
#else
#define PU_WB_TEMP_MSK                              0x0000
#endif

#if PU_BACKLIGHT_COMP_EN
#define PU_BACKLIGHT_COMP_MSK                       0x0100
#else
#define PU_BACKLIGHT_COMP_MSK                       0x0000
#endif

#if PU_GAIN_EN
#define PU_GAIN_MSK                                 0x0200
#else
#define PU_GAIN_MSK                                 0x0000
#endif

#if PU_POWERLINE_FREQ_EN
#define PU_POWERLINE_FREQ_MSK                       0x0400
#else
#define PU_POWERLINE_FREQ_MSK                       0x0000
#endif

#if PU_WB_AUTO_EN
#define PU_WB_AUTO_MSK                              0x1000
#else
#define PU_WB_AUTO_MSK                              0x0000
#endif

#if CT_SCANNING_MODE_EN
#define CT_SCANNING_MODE_MSK                        0x0001
#else
#define CT_SCANNING_MODE_MSK                        0x0000
#endif

#if CT_AE_MODE_EN
#define CT_AE_MODE_MSK                              0x0002
#else
#define CT_AE_MODE_MSK                              0x0000
#endif

#if CT_AE_PRIORITY_EN
#define CT_AE_PRIORITY_MSK                          0x0004
#else
#define CT_AE_PRIORITY_MSK                          0x0000
#endif

#if CT_EXPOSURE_TIME_ABSOLUTE_EN
#define CT_EXPOSURE_TIME_ABSOLUTE_MSK               0x0008
#else
#define CT_EXPOSURE_TIME_ABSOLUTE_MSK               0x0000
#endif

#if CT_EXPOSURE_TIME_RELATIVE_EN
#define CT_EXPOSURE_TIME_RELATIVE_MSK               0x0010
#else
#define CT_EXPOSURE_TIME_RELATIVE_MSK               0x0000
#endif

#if CT_FOCUS_ABSOLUTE_EN
#define CT_FOCUS_ABSOLUTE_MSK                       0x0020
#else
#define CT_FOCUS_ABSOLUTE_MSK                       0x0000
#endif

#if CT_FOCUS_RELATIVE_EN
#define CT_FOCUS_RELATIVE_MSK                       0x0040
#else
#define CT_FOCUS_RELATIVE_MSK                       0x0000
#endif

#if CT_ZOOM_ABSOLUTE_EN
#define CT_ZOOM_ABSOLUTE_MSK                        0x0200
#else
#define CT_ZOOM_ABSOLUTE_MSK                        0x0000
#endif

#if CT_ZOOM_RELATIVE_EN
#define CT_ZOOM_RELATIVE_MSK                        0x0400
#else
#define CT_ZOOM_RELATIVE_MSK                        0x0000
#endif

#if CT_PANTILT_ABSOLUTE_EN
#define CT_PANTILT_ABSOLUTE_MSK                     0x0800
#else
#define CT_PANTILT_ABSOLUTE_MSK                     0x0000
#endif

#if CT_PANTILT_RELATIVE_EN
#define CT_PANTILT_RELATIVE_MSK                     0x1000
#else
#define CT_PANTILT_RELATIVE_MSK                     0x0000
#endif

#if CT_PRIVACY_CONTROL_EN
#define CT_PRIVACY_CONTROL_MSK                      0x40000
#else
#define CT_PRIVACY_CONTROL_MSK                      0x00000
#endif

#if CT_AUTO_FOCUS_EN
#define CT_AUTO_FOCUS_MSK                           0x20000
#else
#define CT_AUTO_FOCUS_MSK                           0x00000
#endif

#if CT_REGION_OF_INTEREST_EN
#define CT_REGION_OF_INTEREST_MSK                   0x200000
#else
#define CT_REGION_OF_INTEREST_MSK                   0x00000
#endif

#define VAL_2BTO16(h, l)	((h<<8)+l)
#define VAL_4BTO32(b3, b2, b1, b0)	((b3<<24)+(b2<<16)+(b1<<8)+b0)

UINT32 uvc_ct_controls = (CT_SCANNING_MODE_MSK|CT_AE_MODE_MSK|CT_AE_PRIORITY_MSK|CT_EXPOSURE_TIME_ABSOLUTE_MSK|
                          CT_EXPOSURE_TIME_RELATIVE_MSK|CT_FOCUS_ABSOLUTE_MSK|CT_FOCUS_RELATIVE_MSK|
                          CT_ZOOM_ABSOLUTE_MSK|CT_ZOOM_RELATIVE_MSK|CT_PANTILT_ABSOLUTE_MSK|CT_PRIVACY_CONTROL_MSK|
                          CT_AUTO_FOCUS_MSK|CT_REGION_OF_INTEREST_MSK);
UINT32 uvc_pu_controls = (PU_BRIGHTNESS_MSK|PU_CONTRAST_MSK|PU_HUE_MSK|PU_SATURATION_MSK|
                          PU_SHARPNESS_MSK|PU_GAMMA_MSK|PU_WB_TEMP_MSK|PU_BACKLIGHT_COMP_MSK|
                          PU_GAIN_MSK|PU_POWERLINE_FREQ_MSK|PU_WB_AUTO_MSK);
#if CT_REGION_OF_INTEREST_EN
static UVC_ROI_INFO uvc_roi = {0};

UINT32 uvc_roi_width = 1920;
UINT32 uvc_roi_height = 1080;
#endif

#if CT_AE_MODE_EN
static BOOL xUvac_CT_AE_Mode(UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = TRUE;
	AET_MANUAL AEInfo = {0};

	switch(request)
	{
		//No need to support GET_LEN, GET_MAX, GET_MIN
		case GET_INFO:
			*pDataLen = 1;	//must be 1 according to UVC spec
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			break;
		case GET_RES:
			*pDataLen = 1;
			pData[0] = 0x06;
			break;
		case GET_DEF:
			*pDataLen = 1;
			pData[0] = 0x02;
			break;
		case GET_CUR:
			if (vendor_isp_init() == HD_ERR_NG) {
				return FALSE;
			}
			*pDataLen = 1;
			AEInfo.id = 0;
			vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
			if (AEInfo.manual.mode == MANUAL_MODE){
				pData[0] = 0x04;
			} else if (AEInfo.manual.mode == AUTO_MODE){
				pData[0] = 0x02;
			} else {
				printf("AE mode(%d) not supprt in UVC CT\n", AEInfo.manual.mode);
				return FALSE;
			}
			vendor_isp_uninit();
			break;
		case SET_CUR:
			if (vendor_isp_init() == HD_ERR_NG) {
				return FALSE;
			}
			AEInfo.id = 0;
			if (pData[0] == 0x04){
				vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
				AEInfo.manual.mode = MANUAL_MODE;
				vendor_isp_set_ae(AET_ITEM_MANUAL, &AEInfo);
			} else if (pData[0] == 0x02){
				vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
				AEInfo.manual.mode = AUTO_MODE;
				vendor_isp_set_ae(AET_ITEM_MANUAL, &AEInfo);
			} else {
				printf("CT AE mode not support (%u)\r\n", pData[0]);
			}
			vendor_isp_uninit();
			break;
		default:
			ret = FALSE;
			break;
	}
	return ret;
}
#endif
#if CT_EXPOSURE_TIME_ABSOLUTE_EN
static BOOL xUvac_CT_Exposure_Time_Absolute(UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = TRUE;
	AET_MANUAL AEInfo = {0};

	switch(request)
	{
		case GET_LEN:
			*pDataLen = 2;	//must be 2 according to UVC spec
			pData[0] = 4;
			pData[1] = 0;
			break;
		case GET_INFO:
			*pDataLen = 1;	//must be 1 according to UVC spec
			pData[0] =  SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			break;
		case GET_MIN:
			*pDataLen = 4;
			pData[0] = 625 & 0xFF;
			pData[1] = (625 >> 8) & 0xFF;
			pData[2] = (625 >> 16) & 0xFF;
			pData[3] = (625 >> 24) & 0xFF;
			break;
		case GET_MAX:
			*pDataLen = 4;
			pData[0] = 160000 & 0xFF;
			pData[1] = (160000 >> 8) & 0xFF;
			pData[2] = (160000 >> 16) & 0xFF;
			pData[3] = (160000 >> 24) & 0xFF;
			break;
		case GET_RES:
			*pDataLen = 4;
			pData[0] = 1;
			pData[1] = 0;
			pData[2] = 0;
			pData[3] = 0;
			break;
		case GET_DEF:
			*pDataLen = 4;
			pData[0] = 40000 & 0xFF;;
			pData[1] = (40000 >> 8) & 0xFF;
			pData[2] = (40000 >> 16) & 0xFF;
			pData[3] = (40000 >> 24) & 0xFF;
			break;
		case GET_CUR:
			if (vendor_isp_init() == HD_ERR_NG) {
				return FALSE;
			}
			*pDataLen = 4;
			AEInfo.id = 0;
			vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
			if (AEInfo.manual.expotime==0 || AEInfo.manual.iso_gain==0){
				pData[0] = 625 & 0xFF;
				pData[1] = (625 >> 8) & 0xFF;
				pData[2] = (625 >> 16) & 0xFF;
				pData[3] = (625 >> 24) & 0xFF;
				AEInfo.manual.expotime = 625;       //set default value
				AEInfo.manual.iso_gain = 100;		//fixed to 100
				vendor_isp_set_ae(AET_ITEM_MANUAL, &AEInfo);
			} else {
				pData[0] = AEInfo.manual.expotime & 0xFF;
				pData[1] = (AEInfo.manual.expotime >> 8) & 0xFF;
				pData[2] = (AEInfo.manual.expotime >> 16) & 0xFF;
				pData[3] = (AEInfo.manual.expotime >> 24) & 0xFF;
			}
			vendor_isp_uninit();
			break;
		case SET_CUR:
			if (vendor_isp_init() == HD_ERR_NG) {
				return FALSE;
			}
			AEInfo.id = 0;
			vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
			AEInfo.manual.expotime = pData[0] + (pData[1]<<8) + (pData[2]<<16) + (pData[3] << 24);
			AEInfo.manual.iso_gain = 100;
			vendor_isp_set_ae(AET_ITEM_MANUAL, &AEInfo);
			vendor_isp_uninit();
			break;
		default:
			ret = FALSE;
			break;
	}
	return ret;
}
#endif

#if CT_ZOOM_ABSOLUTE_EN

static UINT16 Customer_DZoom_LUT[6] = {1000, 1248, 1558, 1945, 2428, 3031};
static PTZ_PARAM ptz_param = {0};

static BOOL xUvac_CT_Zoom_Absolute(UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = TRUE;
	VIDEO_STREAM* stream = get_video_stream();
	UINT32 width, height;
	UINT32 idx1 = 0;
	UINT32 idx2 = 0;
	UINT32 uvc_idx1 = 0;
	UINT32 uvc_idx2 = 0;
	UINT32 ZoomRatio = 0;
	UINT32 coor_x_d;
	UINT32 coor_y_d;

	INT32 coor_x_c;
	INT32 coor_y_c;

	UINT32 coor_x;
	UINT32 coor_y;

	switch(request)
	{
		case GET_LEN:
			*pDataLen = 2;	//must be 2 according to UVC spec
			pData[0] = 2;
			pData[1] = 0;
			break;
		case GET_INFO:
			*pDataLen = 1;	//must be 1 according to UVC spec
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			break;
		case GET_MIN:
			*pDataLen = 2;
			pData[0] = 0;
			pData[1] = 0;
			break;
		case GET_MAX:
			*pDataLen = 2;
			pData[0] = 158;
			pData[1] = 0;
			break;
		case GET_RES:
			*pDataLen = 2;
			pData[0] = 1;
			pData[1] = 0;
			break;
		case GET_DEF:
			*pDataLen = 2;
			pData[0] = 0;		//0
			pData[1] = 0;
			break;
		case GET_CUR:
			*pDataLen = 2;
			pData[0] = ptz_param.zoom_idx;
			pData[1] = 0;
			break;
		case SET_CUR: {

				UINT32 uiWidth, uiHeight;
				UINT32 last_zoom;


				if (VAL_2BTO16((UINT32)pData[1], (UINT32)pData[0]) > 158) {
					ret = ERROR_CODE_OUT_OF_RANGE;
					break;
				}


				uiWidth  = stream[0].enc_main_dim.w;
				uiHeight = stream[0].enc_main_dim.h;

				last_zoom = ptz_param.zoom_idx;
				ptz_param.zoom_idx = pData[0];		// 0 ~ 40->depending on max dzoom factor

				if (MAX_CAP_SIZE_W != uiWidth && uiHeight != MAX_CAP_SIZE_H) {
					idx1 = (ptz_param.zoom_idx >> 5);
					idx2 = idx1+1;

					uvc_idx1 = (idx1<<5);
					uvc_idx2 = (idx2<<5);

					ZoomRatio = ((Customer_DZoom_LUT[idx2] - Customer_DZoom_LUT[idx1])*(ptz_param.zoom_idx-uvc_idx1)/(uvc_idx2 - uvc_idx1))+Customer_DZoom_LUT[idx1];

					width = ALIGN_CEIL_8(1920000 / ZoomRatio);
					height = ALIGN_CEIL_8(1080000 / ZoomRatio);

					coor_x_c = (1920/2)-(width/2);
					coor_y_c = (1080/2)-(height/2);

					coor_x_d = (1920-width)/2/10;
					coor_y_d = (1080-height)/2/10;

					coor_x = (UINT32)(coor_x_c + (ptz_param.pan_idx*coor_x_d));
					coor_y = (UINT32)(coor_y_c - (ptz_param.tilt_idx*coor_y_d));

					//printf("zoom_idx: %d pData[0]:%d idx1: %d ZoomRatio: %d, %d x %d %d %d %d %d\r\n", zoom_idx, pData[0], idx1, ZoomRatio, uiWidth, uiHeight, width, height, coor_x, coor_y);
					//printf("pan_idx: %d tilt_idx:%d\r\n", pan_idx, tilt_idx);

					if (pData[0] == 0) {

						//printf("Index = 0\r\n");

						if (last_zoom != 0) {

							if (stream[0].codec_type != HD_CODEC_TYPE_RAW) {
								HD_VIDEOCAP_CROP video_crop_param = {0};

								video_crop_param.mode = HD_CROP_OFF;
								video_crop_param.win.rect.x = 0;
								video_crop_param.win.rect.y = 0;
								video_crop_param.win.rect.w = 0;
								video_crop_param.win.rect.h= 0;
								video_crop_param.align.w = 0;
								video_crop_param.align.h = 0;
								hd_videocap_set(stream[0].cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);

								hd_videocap_start(stream[0].cap_path);
							} else {
								int en_slice = 1;

								//stop poll data
								pthread_mutex_lock(&flow_start_lock);
								encode_start = 0;
								acquire_start = 0;
								pthread_mutex_unlock(&flow_start_lock);

								if (g_capbind == 1) {
									hd_videoproc_stop(stream[0].proc_main_path);
									hd_videoproc_stop(stream[0].proc_sub_path);
									hd_videocap_stop(stream[0].cap_path);
								} else {
									hd_videocap_stop(stream[0].cap_path);
									hd_videoproc_stop(stream[0].proc_main_path);
									hd_videoproc_stop(stream[0].proc_sub_path);
								}

								if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
									hd_videoenc_stop(stream[0].enc_main_path);
								}

								hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
								hd_videoproc_close(stream[0].proc_main_path);
								hd_videoproc_close(stream[0].proc_sub_path);


								set_proc_cfg(&stream[0].proc_ctrl, &stream[0].cap_dim, en_slice);

								hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &stream[0].proc_main_path);
								hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &stream[0].proc_sub_path);
								hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);


								set_proc_param(stream[0].proc_main_path, &stream[0].cap_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
								set_proc_param(stream[0].proc_sub_path, &stream[0].enc_main_dim, VDO_PXLFMT_YUV422_YUYV, 4, 0);


								set_cap_param(stream[0].cap_path, &stream[0].cap_dim, MAX_BS_FPS, &stream[0].cap_dim);


								if (g_capbind == 0) {
									//start engine(modules)
									hd_videocap_start(stream[0].cap_path);
								}
								hd_videoproc_start(stream[0].proc_main_path);
								if (stream[0].codec_type == HD_CODEC_TYPE_RAW){
									hd_videoproc_start(stream[0].proc_sub_path);
								}
								if (g_capbind == 1) {
									//start engine(modules)
									hd_videocap_start(stream[0].cap_path);
								}

								pthread_mutex_lock(&flow_start_lock);
								encode_start = 0;
								acquire_start = 1;
								pthread_mutex_unlock(&flow_start_lock);
							}

							//printf("^Gexit zoom\r\n");
						}

						ptz_param.zoom_idx_max = pData[0];
						ptz_param.zoom_set_max = 0;

					} else if ((coor_x + width < 1920) && (coor_y + height < 1080) && (width >= uiWidth) && (height >= uiHeight)) {

						if (last_zoom == 0) {

							if (stream[0].codec_type != HD_CODEC_TYPE_RAW) {
								HD_VIDEOCAP_CROP video_crop_param = {0};

								//DBG_DUMP("Index = %d, zoom\r\n", pData[0]);

								video_crop_param.mode = HD_CROP_ON;
								video_crop_param.win.rect.x = coor_x;
								video_crop_param.win.rect.y = coor_y;
								video_crop_param.win.rect.w = width;
								video_crop_param.win.rect.h= height;
								video_crop_param.align.w = 4;
								video_crop_param.align.h = 4;
								hd_videocap_set(stream[0].cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);

								hd_videocap_start(stream[0].cap_path);
							} else {
								HD_VIDEOCAP_CROP video_crop_param = {0};
								int en_slice = 0;
								//HD_DIM cap_out_dim = {0};

								//DBG_DUMP("Index = %d, enter zoom\r\n", pData[0]);

								//stop poll data
								pthread_mutex_lock(&flow_start_lock);
								encode_start = 0;
								acquire_start = 0;
								pthread_mutex_unlock(&flow_start_lock);

								if (g_capbind == 1) {
									hd_videoproc_stop(stream[0].proc_main_path);
									hd_videoproc_stop(stream[0].proc_sub_path);
									hd_videocap_stop(stream[0].cap_path);
								} else {
									hd_videocap_stop(stream[0].cap_path);
									hd_videoproc_stop(stream[0].proc_main_path);
									hd_videoproc_stop(stream[0].proc_sub_path);
								}

								if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
									hd_videoenc_stop(stream[0].enc_main_path);
								}

								hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
								hd_videoproc_close(stream[0].proc_main_path);
								hd_videoproc_close(stream[0].proc_sub_path);

								set_proc_cfg(&stream[0].proc_ctrl, &stream[0].cap_dim, en_slice);

								hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &stream[0].proc_main_path);
								hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &stream[0].proc_sub_path);
								hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);

								//cap_out_dim.w = width;
								//cap_out_dim.h = height;

								set_proc_param(stream[0].proc_main_path, &stream[0].enc_main_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
								set_proc_param(stream[0].proc_sub_path, &stream[0].enc_main_dim, VDO_PXLFMT_YUV422_YUYV, 4, 0);

								set_cap_param(stream[0].cap_path, &stream[0].cap_dim, MAX_BS_FPS, &stream[0].enc_main_dim);

								video_crop_param.mode = HD_CROP_ON;
								video_crop_param.win.rect.x = coor_x;
								video_crop_param.win.rect.y = coor_y;
								video_crop_param.win.rect.w = width;
								video_crop_param.win.rect.h= height;
								video_crop_param.align.w = 4;
								video_crop_param.align.h = 4;
								hd_videocap_set(stream[0].cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);

								if (g_capbind == 0) {
									//start engine(modules)
									hd_videocap_start(stream[0].cap_path);
								}
								hd_videoproc_start(stream[0].proc_main_path);
								if (stream[0].codec_type == HD_CODEC_TYPE_RAW){
									hd_videoproc_start(stream[0].proc_sub_path);
								}
								if (g_capbind == 1) {
									//start engine(modules)
									hd_videocap_start(stream[0].cap_path);
								}


								pthread_mutex_lock(&flow_start_lock);
								encode_start = 0;
								acquire_start = 1;
								pthread_mutex_unlock(&flow_start_lock);
							}
						} else {
							HD_VIDEOCAP_CROP video_crop_param = {0};

							//DBG_DUMP("Index = %d, zoom\r\n", pData[0]);
							video_crop_param.mode = HD_CROP_ON;
							video_crop_param.win.rect.x = coor_x;
							video_crop_param.win.rect.y = coor_y;
							video_crop_param.win.rect.w = width;
							video_crop_param.win.rect.h= height;
							video_crop_param.align.w = 4;
							video_crop_param.align.h = 4;
							hd_videocap_set(stream[0].cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);

							hd_videocap_start(stream[0].cap_path);
						}

						//printf("Index = %d, zoom\r\n", pData[0]);

						ptz_param.zoom_idx_max = pData[0];

						if (pData[0] == 58 && uiWidth == 1280 && uiHeight == 720) {
							ptz_param.zoom_set_max = 1;
						} else {
							ptz_param.zoom_set_max = 0;
						}
					} /*else if (zoom_set_max == 0) {
						UINT32 temp_zoom_idx = 58;

						idx1 = (temp_zoom_idx >> 5);
						idx2 = idx1+1;

						uvc_idx1 = (idx1<<5);
						uvc_idx2 = (idx2<<5);

						ZoomRatio = ((Customer_DZoom_LUT[idx2] - Customer_DZoom_LUT[idx1])*(temp_zoom_idx-uvc_idx1)/(uvc_idx2 - uvc_idx1))+Customer_DZoom_LUT[idx1];

						width = ALIGN_CEIL_8(1920000 / ZoomRatio);
						height = ALIGN_CEIL_8(1080000 / ZoomRatio);

						coor_x_c = (1920/2)-(width/2);
						coor_y_c = (1080/2)-(height/2);

						coor_x_d = (1920-width)/2/10;
						coor_y_d = (1080-height)/2/10;

						coor_x = (UINT32)(coor_x_c + (pan_idx*coor_x_d));
						coor_y = (UINT32)(coor_y_c - (tilt_idx*coor_y_d));

						if (stream[0].codec_type != HD_CODEC_TYPE_RAW) {
							HD_VIDEOCAP_CROP video_crop_param = {0};

							//DBG_DUMP("Index = %d, zoom\r\n", pData[0]);

							video_crop_param.mode = HD_CROP_ON;
							video_crop_param.win.rect.x = coor_x;
							video_crop_param.win.rect.y = coor_y;
							video_crop_param.win.rect.w = width;
							video_crop_param.win.rect.h= height;
							video_crop_param.align.w = 4;
							video_crop_param.align.h = 4;
							hd_videocap_set(stream[0].cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);

							hd_videocap_start(stream[0].cap_path);
						} else {
							HD_VIDEOCAP_CROP video_crop_param = {0};
							int en_slice = 0;
							//HD_DIM cap_out_dim = {0};

							//DBG_DUMP("Index = %d, enter zoom\r\n", pData[0]);

							//stop poll data
							pthread_mutex_lock(&flow_start_lock);
							encode_start = 0;
							acquire_start = 0;
							pthread_mutex_unlock(&flow_start_lock);

							if (g_capbind == 1) {
								hd_videoproc_stop(stream[0].proc_main_path);
								hd_videoproc_stop(stream[0].proc_sub_path);
								hd_videocap_stop(stream[0].cap_path);
							} else {
								hd_videocap_stop(stream[0].cap_path);
								hd_videoproc_stop(stream[0].proc_main_path);
								hd_videoproc_stop(stream[0].proc_sub_path);
							}

							if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
								hd_videoenc_stop(stream[0].enc_main_path);
							}

							hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
							hd_videoproc_close(stream[0].proc_main_path);
							hd_videoproc_close(stream[0].proc_sub_path);

							set_proc_cfg(&stream[0].proc_ctrl, &stream[0].cap_dim, en_slice);

							hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &stream[0].proc_main_path);
							hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &stream[0].proc_sub_path);
							hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);

							//cap_out_dim.w = width;
							//cap_out_dim.h = height;

							set_proc_param(stream[0].proc_main_path, &stream[0].enc_main_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
							set_proc_param(stream[0].proc_sub_path, &stream[0].enc_main_dim, VDO_PXLFMT_YUV422_YUYV, 4, 0);

							set_cap_param(stream[0].cap_path, &stream[0].cap_dim, MAX_BS_FPS, &stream[0].enc_main_dim);

							video_crop_param.mode = HD_CROP_ON;
							video_crop_param.win.rect.x = coor_x;
							video_crop_param.win.rect.y = coor_y;
							video_crop_param.win.rect.w = width;
							video_crop_param.win.rect.h= height;
							video_crop_param.align.w = 4;
							video_crop_param.align.h = 4;
							hd_videocap_set(stream[0].cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);

							if (g_capbind == 0) {
								//start engine(modules)
								hd_videocap_start(stream[0].cap_path);
							}
							hd_videoproc_start(stream[0].proc_main_path);
							if (stream[0].codec_type == HD_CODEC_TYPE_RAW){
								hd_videoproc_start(stream[0].proc_sub_path);
							}
							if (g_capbind == 1) {
								//start engine(modules)
								hd_videocap_start(stream[0].cap_path);
							}


							pthread_mutex_lock(&flow_start_lock);
							encode_start = 0;
							acquire_start = 1;
							pthread_mutex_unlock(&flow_start_lock);
						}

						printf("Index = %d, enter max zoom\r\n", pData[0]);

						zoom_set_max = 1;
						zoom_idx_max = temp_zoom_idx;
					}*/
				}
			}
			break;
		default:
			ret = FALSE;
			break;
	}
	return ret;
}
#endif

#if CT_PANTILT_ABSOLUTE_EN
static BOOL xUvac_CT_pantilt_Absolute(UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = TRUE;
	INT32* pan = (INT32*)pData;
	INT32* tilt = (INT32*)(pData+4);
	VIDEO_STREAM* stream = get_video_stream();
	UINT32 width, height;
	UINT32 idx1 = 0;
	UINT32 idx2 = 0;
	UINT32 uvc_idx1 = 0;
	UINT32 uvc_idx2 = 0;
	UINT32 ZoomRatio = 0;
	UINT32 coor_x_d;
	UINT32 coor_y_d;

	INT32 coor_x_c;
	INT32 coor_y_c;

	UINT32 coor_x;
	UINT32 coor_y;

	switch(request)
	{
		case GET_LEN:
			//*pDataLen = 2;	//must be 2 according to UVC spec
			pData[0] = 4;
			pData[1] = 0;
			pData[2] = 0;
			pData[3] = 0;
			pData[4] = 4;
			pData[5] = 0;
			pData[6] = 0;
			pData[7] = 0;
			//DBG_DUMP("^GPT get LEN, data len=%d\r\n", *pDataLen);
			break;
		case GET_INFO:
			*pDataLen = 1;	//must be 1 according to UVC spec
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			//DBG_DUMP("^GPT get info\r\n");
			break;
		case GET_MIN:
			*pDataLen = 8;
			*pan = -36000;	//40
			*tilt = -36000;	//40

			//DBG_DUMP("^GPT get min data len=%d pan_v = %d, tilt_v = %d, pData = %02x%02x%02x%02x%02x%02x%02x%02x\r\n", *pDataLen, pan_v, tilt_v, (UINT8)(*(pData+7)), (UINT8)(*(pData+6)), (UINT8)(*(pData+5)), (UINT8)(*(pData+4)), (UINT8)(*(pData+3)), (UINT8)(*(pData+2)), (UINT8)(*(pData+1)), (UINT8)(*(pData)));
			break;
		case GET_MAX:
			*pDataLen = 8;
			*pan = 36000;	//40
			*tilt = 36000;	//40

			//DBG_DUMP("^GPT get max data len=%d pan_v = %d, tilt_v = %d, pData = %02x%02x%02x%02x%02x%02x%02x%02x\r\n", *pDataLen, pan_v, tilt_v, (UINT8)(*(pData+7)), (UINT8)(*(pData+6)), (UINT8)(*(pData+5)), (UINT8)(*(pData+4)), (UINT8)(*(pData+3)), (UINT8)(*(pData+2)), (UINT8)(*(pData+1)), (UINT8)(*(pData)));
			break;
		case GET_RES:
			//*pDataLen = 8;
			*pan = 3600;	//40
			*tilt = 3600;	//40

			//DBG_DUMP("^GPT get res data len=%d pData = %02x%02x%02x%02x%02x%02x%02x%02x\r\n", *pDataLen, (UINT8)(*(pData+7)), (UINT8)(*(pData+6)), (UINT8)(*(pData+5)), (UINT8)(*(pData+4)), (UINT8)(*(pData+3)), (UINT8)(*(pData+2)), (UINT8)(*(pData+1)), (UINT8)(*(pData)));
			break;
		case GET_DEF:
			*pDataLen = 8;
			*pan = 0;	//40
			*tilt = 0;	//40
			//DBG_DUMP("^GPT get def data len=%d\r\n", *pDataLen);
			break;
		case GET_CUR:
			*pDataLen = 8;
			*pan = ptz_param.pan_v;
			*tilt = ptz_param.tilt_v;
			break;
		case SET_CUR: {
				UINT32 uiWidth, uiHeight;

				if ((*pan > 36000) || (*pan < -36000)) {
					ret = ERROR_CODE_OUT_OF_RANGE;
					//DBG_ERR("invalid pan %d\r\n", *pan);
					break;
				} else if ((*tilt > 36000) || (*tilt < -36000)) {
					ret = ERROR_CODE_OUT_OF_RANGE;
					//DBG_ERR("invalid tilt %d\r\n", *tilt);
					break;
				}

				ptz_param.pan_v = *pan;
				ptz_param.tilt_v = *tilt;

				ptz_param.pan_idx = ptz_param.pan_v/3600;
				ptz_param.tilt_idx = ptz_param.tilt_v/3600;



				uiWidth  = stream[0].enc_main_dim.w;
				uiHeight = stream[0].enc_main_dim.h;

				if (MAX_CAP_SIZE_W != uiWidth && uiHeight != MAX_CAP_SIZE_H) {

					idx1 = (ptz_param.zoom_idx_max >> 5);
					idx2 = idx1+1;

					uvc_idx1 = (idx1<<5);
					uvc_idx2 = (idx2<<5);

					ZoomRatio = ((Customer_DZoom_LUT[idx2] - Customer_DZoom_LUT[idx1])*(ptz_param.zoom_idx_max-uvc_idx1)/(uvc_idx2 - uvc_idx1))+Customer_DZoom_LUT[idx1];

					width = ALIGN_CEIL_8(1920000 / ZoomRatio);
					height = ALIGN_CEIL_8(1080000 / ZoomRatio);

					coor_x_c = (1920/2)-(width/2);
					coor_y_c = (1080/2)-(height/2);

					coor_x_d = (1920-width)/2/10;
					coor_y_d = (1080-height)/2/10;

					coor_x = (UINT32)(coor_x_c + (ptz_param.pan_idx*coor_x_d));
					coor_y = (UINT32)(coor_y_c - (ptz_param.tilt_idx*coor_y_d));

					if (ptz_param.zoom_idx == 0) {

						/*ImageUnit_Begin(&ISF_UserProc, 0);
							ImageUnit_SetVdoPostWindow(iUVACPath + ISF_IN_BASE + codec_path,0,0,0,0);
						ImageUnit_End();

						ImageStream_UpdateAll(&ISF_Stream[iUVACPath]);*/

					} else if ((coor_x + width < 1920) && (coor_y + height < 1080) && (width >= uiWidth) && (height >= uiHeight)) {

						HD_VIDEOCAP_CROP video_crop_param = {0};

						video_crop_param.mode = HD_CROP_ON;
						video_crop_param.win.rect.x = coor_x;
						video_crop_param.win.rect.y = coor_y;
						video_crop_param.win.rect.w = width;
						video_crop_param.win.rect.h= height;
						video_crop_param.align.w = 4;
						video_crop_param.align.h = 4;
						hd_videocap_set(stream[0].cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);

						hd_videocap_start(stream[0].cap_path);
					}
				}
			}
			break;
		default:
			ret = FALSE;
			break;
	}
	return ret;
}
#endif

#if CT_REGION_OF_INTEREST_EN
static BOOL xUvac_CT_ROI(UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = TRUE;
	AE_ROI_WIN roi_win;

	switch(request)
	{
		case GET_MIN:
			DBG_IND("^G%s: GET_MIN,  pData=0x%x, len=%d\r\n", __func__, pData, *pDataLen);
			*pDataLen = 10;

			pData[0] = 0;
			pData[1] = 0;
			pData[2] = 0;
			pData[3] = 0;
			pData[4] = 0;
			pData[5] = 0;
			pData[6] = 0;
			pData[7] = 0;
			pData[8] = 0;
			pData[9] = 0;
			break;
		case GET_MAX:
			*pDataLen = 10;
			pData[0] = 0;
			pData[1] = 0;
			pData[2] = 0;
			pData[3] = 0;
			pData[4] = uvc_roi_height& 0xff;
			pData[5] = (uvc_roi_height >> 8) & 0xff;
			pData[6] = uvc_roi_width& 0xff;
			pData[7] = (uvc_roi_width >> 8) & 0xff;
			pData[8] = 0x11 & 0xff;
			pData[9] = (0x11 >> 8) & 0xff;

			DBG_IND("^G%s: GET_MAX, top=%d, left=%d, bottom=%d, right=%d, auto ctrl=0x%x\r\n", __func__, (pData[0] | (pData[1] << 8)), \
																						  (pData[2] | (pData[3] << 8)), \
																						  (pData[4] | (pData[5] << 8)), \
																						  (pData[6] | (pData[7] << 8)), \
																						  (pData[8] | (pData[9] << 8)));

			break;
		case GET_INFO:
			DBG_IND("^G%s: GET_INFO,  pData=0x%x, len=%d\r\n", __func__, pData, *pDataLen);
			*pDataLen = 1;	//must be 1 according to UVC spec
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			break;
		case GET_DEF:
			*pDataLen = 10;
			pData[0] = 0;
			pData[1] = 0;
			pData[2] = 0;
			pData[3] = 0;
			pData[4] = uvc_roi_height& 0xff;
			pData[5] = (uvc_roi_height >> 8) & 0xff;
			pData[6] = uvc_roi_width& 0xff;
			pData[7] = (uvc_roi_width >> 8) & 0xff;
			pData[8] = 0x11 & 0xff;
			pData[9] = (0x11 >> 8) & 0xff;

			DBG_IND("^G%s: GET_DEF, top=%d, left=%d, bottom=%d, right=%d, auto ctrl=0x%x\r\n", __func__, (pData[0] | (pData[1] << 8)), \
																						  (pData[2] | (pData[3] << 8)), \
																						  (pData[4] | (pData[5] << 8)), \
																						  (pData[6] | (pData[7] << 8)), \
																						  (pData[8] | (pData[7] << 9)));

			break;
		case GET_CUR:
			*pDataLen = 10;
			pData[0] = uvc_roi.top & 0xff;
			pData[1] = (uvc_roi.top >> 8) & 0xff;
			pData[2] = uvc_roi.left & 0xff;
			pData[3] = (uvc_roi.left >> 8) & 0xff;
			pData[4] = uvc_roi.bottom & 0xff;
			pData[5] = (uvc_roi.bottom >> 8) & 0xff;
			pData[6] = uvc_roi.right & 0xff;
			pData[7] = (uvc_roi.right >> 8) & 0xff;
			pData[8] = uvc_roi.auto_ctl& 0xff;
			pData[9] = (uvc_roi.auto_ctl>> 8) & 0xff;

			DBG_IND("^G%s: GET_CUR, top=%d, left=%d, bottom=%d, right=%d, auto ctrl=0x%x\r\n", __func__, uvc_roi.top, uvc_roi.left, uvc_roi.bottom, uvc_roi.right, uvc_roi.auto_ctl);

			break;
		case SET_CUR:
			uvc_roi.top = pData[0] | (pData[1] << 8);
			uvc_roi.left = pData[2] | (pData[3] << 8);
			uvc_roi.bottom = pData[4] | (pData[5] << 8);
			uvc_roi.right = pData[6] | (pData[7] << 8);
			uvc_roi.auto_ctl  = pData[8] | (pData[9] << 8);

			//if((uvc_roi.top == 0)&&(uvc_roi.left == 0)&&(uvc_roi.bottom == 0)&&(uvc_roi.right == 0)) {
			if((uvc_roi.top == 0)&&(uvc_roi.left == 0)&&(uvc_roi.bottom >= uvc_roi_height)&&(uvc_roi.right == uvc_roi_width)) {
				ae_set_ui_info(0, AE_UI_METER, AE_METER_USERDEFINE);
				//DBG_DUMP("*** No ROI ***\r\n");
				//iq_set_ui_info(0, IQ_UI_IMAGEEFFECT, IQ_IMAGEEFFECT_OFF);
			} else {
				roi_win.start_x = ((uvc_roi.left*100)/uvc_roi_width);
				roi_win.start_y = ((uvc_roi.top*100)/uvc_roi_height);
				roi_win.end_x = ((uvc_roi.right*100)/uvc_roi_width);
				roi_win.end_y = ((uvc_roi.bottom*100)/uvc_roi_height);
				roi_win.option = ROI_EROSION;
				#if 0
				roi_win.roi_w = 32;
				roi_win.other_w = 1;
				#endif
				#if 1
				roi_win.roi_w = 64;
				roi_win.other_w = 1;
				#endif
				roi_win.valid_rng.l = 0;
				roi_win.valid_rng.h = 4095;

				ae_set_ui_info(0, AE_UI_METER, AE_METER_ROI);
				AE_setROI(&roi_win);
				//DBG_WRN("^G%s: SET_CUR, top=%d, left=%d, bottom=%d, right=%d, auto ctrl=0x%x\r\n", __func__, uvc_roi.top, uvc_roi.left, uvc_roi.bottom, uvc_roi.right, uvc_roi.auto_ctl);
				//iq_set_ui_info(0, IQ_UI_IMAGEEFFECT, IQ_IMAGEEFFECT_FaceBeautyRetouch);
			}

			DBG_IND("^G%s: SET_CUR, top=%d, left=%d, bottom=%d, right=%d, auto ctrl=0x%x\r\n", __func__, uvc_roi.top, uvc_roi.left, uvc_roi.bottom, uvc_roi.right, uvc_roi.auto_ctl);
			break;
		default:
			//DBG_ERR("Request = 0x%X not supprt!\r\n", request);
			ret = FALSE;
			break;
	}
	return ret;
}
#endif

BOOL xUvacCT_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = FALSE;
	BOOL supported_cs = TRUE;

	#if 0
	DBG_DUMP("%s: CS=0x%x, request = 0x%X,  pData=0x%x, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	if (SET_CUR == request) {
		DumpMemory((UINT32) pData, *pDataLen, 16);
	}
	#endif

	switch (CS){
		case CT_ZOOM_ABSOLUTE_CONTROL:
			#if CT_ZOOM_ABSOLUTE_EN
			ret = xUvac_CT_Zoom_Absolute(request, pData, pDataLen);
			#else
			supported_cs = FALSE;
			#endif
			break;
		case CT_AE_MODE_CONTROL:
			#if CT_AE_MODE_EN
			ret = xUvac_CT_AE_Mode(request, pData, pDataLen);
			#else
			supported_cs = FALSE;
			#endif
			break;
		case CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
			#if CT_EXPOSURE_TIME_ABSOLUTE_EN
			ret = xUvac_CT_Exposure_Time_Absolute(request, pData, pDataLen);

			#else
			supported_cs = FALSE;
			#endif
			break;

		case CT_PANTILT_ABSOLUTE_CONTROL:
			#if CT_PANTILT_ABSOLUTE_EN
			ret = xUvac_CT_pantilt_Absolute(request, pData, pDataLen);
			#else
			supported_cs = FALSE;
			#endif
			break;
		case CT_REGION_OF_INTEREST_CONTROL:
			#if CT_REGION_OF_INTEREST_EN
			ret = xUvac_CT_ROI(request, pData, pDataLen);
			#else
			supported_cs = FALSE;
			#endif
			break;
		case CT_SCANNING_MODE_CONTROL:
		case CT_AE_PRIORITY_CONTROL:
		case CT_EXPOSURE_TIME_RELATIVE_CONTROL:
		case CT_FOCUS_ABSOLUTE_CONTROL:
		case CT_FOCUS_RELATIVE_CONTROL:
		case CT_FOCUS_AUTO_CONTROL:
		case CT_IRIS_ABSOLUTE_CONTROL:
		case CT_IRIS_RELATIVE_CONTROL:
		case CT_ZOOM_RELATIVE_CONTROL:
		case CT_PANTILT_RELATIVE_CONTROL:
		case CT_ROLL_ABSOLUTE_CONTROL:
		case CT_ROLL_RELATIVE_CONTROL:
			//not supported now
			supported_cs = FALSE;
			break;
		case CT_PRIVACY_CONTROL:
			#if CT_PRIVACY_CONTROL_EN
			ret = xUvac_CT_PrivacyControl(request, pData, pDataLen);
			#else
			supported_cs = FALSE;
			#endif
			break;
		default:
			supported_cs = FALSE;
			break;
	}

	if (ret == TRUE && request == SET_CUR) {
		*pDataLen = 0;
	}

	if (supported_cs == TRUE && ret == FALSE) {
		pData[0] = ERROR_CODE_INVALID_REQUEST;
		*pDataLen = 1;
	} else if (supported_cs == TRUE && ret == ERROR_CODE_WRONG_STATE) {
		pData[0] = ERROR_CODE_WRONG_STATE;
		*pDataLen = 1;
		ret = FALSE;
	} else if (supported_cs == TRUE && ret == ERROR_CODE_OUT_OF_RANGE) {
		pData[0] = ERROR_CODE_OUT_OF_RANGE;
		*pDataLen = 1;
		ret = FALSE;
	} else if (supported_cs == FALSE) {
		pData[0] = ERROR_CODE_INVALID_CONTROL;
	}

	return ret;
}

BOOL xUvacPU_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = FALSE;
	BOOL supported_cs = TRUE;

	#if 0//
	DBG_DUMP("%s: CS=0x%x, request = 0x%X,  pData=0x%x, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	if (SET_CUR == request) {
		DumpMemory((UINT32) pData, *pDataLen, 16);
	}
	#endif

	#if 0
	if (bFirstUVC && (request == SET_CUR)){
		set_flg(FLG_ID_PRJUVC, FLGPRJUVC_BOOT);
		bFirstUVC = FALSE;
		Delay_DelayMs(30);
	}
	#endif
	switch (CS){
	case PU_BACKLIGHT_COMPENSATION_CONTROL:
		#if PU_BACKLIGHT_COMP_EN
		ret = xUvac_PU_BlCompensation(request, pData, pDataLen);
		#else
		supported_cs = FALSE;
		#endif
		break;
	case PU_BRIGHTNESS_CONTROL:
		#if PU_BRIGHTNESS_EN
		ret = xUvac_PU_Brightness(request, pData, pDataLen);
		#else
		supported_cs = FALSE;
		#endif
		break;
	case PU_CONTRAST_CONTROL:
		#if PU_CONTRAST_EN
		ret = xUvac_PU_Contrast(request, pData, pDataLen);
		#else
		supported_cs = FALSE;
		#endif
		break;
	case PU_POWER_LINE_FREQUENCY_CONTROL:
		#if PU_POWERLINE_FREQ_EN
		ret = xUvac_PU_PowerFreq(request, pData, pDataLen);
		#else
		supported_cs = FALSE;
		#endif
		break;
	case PU_HUE_CONTROL:
		#if PU_HUE_EN
		ret = xUvac_PU_Hue(request, pData, pDataLen);
		#else
		supported_cs = FALSE;
		#endif
		break;
	case PU_SATURATION_CONTROL:
		#if PU_SATURATION_EN
		ret = xUvac_PU_Saturation(request, pData, pDataLen);
		#else
		supported_cs = FALSE;
		#endif
		break;
	case PU_SHARPNESS_CONTROL:
		#if PU_SHARPNESS_EN
		ret = xUvac_PU_Sharpness(request, pData, pDataLen);
		#else
		supported_cs = FALSE;
		#endif
		break;
	case PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
		#if PU_WB_TEMP_EN
		ret = xUvac_PU_WB_Temperature(request, pData, pDataLen);
		#else
		supported_cs = FALSE;
		#endif
		break;
	case PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
		#if PU_WB_AUTO_EN
		ret = xUvac_PU_WB_Auto(request, pData, pDataLen);
		#else
		supported_cs = FALSE;
		#endif
		break;
	case PU_GAIN_CONTROL:
	case PU_GAMMA_CONTROL:
	case PU_WHITE_BALANCE_COMPONENT_CONTROL:
	case PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:
	case PU_DIGITAL_MULTIPLIER_CONTROL:
	case PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL:
	case PU_HUE_AUTO_CONTROL:
	case PU_ANALOG_VIDEO_STANDARD_CONTROL:
	case PU_ANALOG_LOCK_STATUS_CONTROL:
		//not supported now
		supported_cs = FALSE;
		break;
	default:
		supported_cs = FALSE;
		break;
	}

	if (ret == TRUE && request == SET_CUR) {
		*pDataLen = 0;
	}
	if (supported_cs == TRUE && ret == FALSE) {
		pData[0] = ERROR_CODE_INVALID_REQUEST;
		*pDataLen = 1;
	} else if (supported_cs == TRUE && ret == ERROR_CODE_WRONG_STATE) {
		pData[0] = ERROR_CODE_WRONG_STATE;
		*pDataLen = 1;
		ret = FALSE;
	} else if (supported_cs == TRUE && ret == ERROR_CODE_OUT_OF_RANGE) {
		pData[0] = ERROR_CODE_OUT_OF_RANGE;
		*pDataLen = 1;
		ret = FALSE;
	} else if (supported_cs == FALSE) {
		pData[0] = ERROR_CODE_INVALID_CONTROL;
	}
	return ret;
}

PTZ_PARAM* uvac_get_ptz_param(void)
{
	#if CT_ZOOM_ABSOLUTE_EN
	return &ptz_param;
	#else
	return NULL;
	#endif
}

#endif //CT_PU_FUNC
