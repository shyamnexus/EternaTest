#ifndef _FLOW_UVC_INFO_H
#define _FLOW_UVC_INFO_H

#define UVC_CACHE_SETTING_EN		ENABLE

typedef struct _UVC_CUSTOM_INFO
{
	UINT32 info_size;
    UINT32 mic_state;
    UINT32 hdr_mode;
    UINT32 retouch;
#if (UVC_CACHE_SETTING_EN == DISABLE)
	UINT32 xu_brightness;
	UINT32 xu_contrast;
	UINT32 xu_hue;
	UINT32 xu_saturation;
	UINT32 xu_sharpness;
	UINT32 xu_power_freq;
	UINT32 xu_backlightcomp;
	UINT32 xu_wb_auto;
	UINT32 xu_wb_temp;
	UINT32 ct_expt_auto;
	UINT32 ct_expt_abs;
#endif
	UINT32 afd_detect_result;
	UINT32 usr_crc;
    UINT8 check_shutter;
    UINT8 reserved_1[31];
    UINT8 reserved_2[32];
    UINT8 reserved_3[32];
    UINT8 reserved_4[32];
} UVC_CUSTOM_INFO;

typedef struct _UVC_MTE_INFO
{
	UINT32 info_size;
    UINT8 serial_number[14];
    UINT8 color_name[16];
    UINT8 LED_lv0;
    UINT8 LED_lv1;
    UINT8 LED_lv2;
    UINT8 LED_lv3;
    UINT8 capability[16];
    UINT8 check_sensor;
    UINT8 ccm_type;
    UINT8 shutter_dir;
    UINT8 reserved_1[32];
    UINT8 reserved_2[32];
    UINT8 reserved_3[3];
} UVC_MTE_INFO;

typedef struct _UVC_ROI_INFO
{
	UINT16 top;
	UINT16 left;
	UINT16 bottom;
	UINT16 right;
	UINT16 auto_ctl;
} UVC_ROI_INFO;

extern UVC_CUSTOM_INFO uvc_custom_info;
extern UVC_MTE_INFO uvc_mte_info;
extern void check_and_restore_header(void);
extern void reset_uvc_custom_info(void);
extern void load_uvc_custom_info(void);
extern void save_uvc_custom_info(void);
extern void reset_uvc_mte_info(void);
extern void load_uvc_mte_info(void);
extern void save_uvc_mte_info(void);
extern void dump_uvc_mte_info(void);
extern void dump_uvc_custom_info(void);
extern void set_update_onehead(void);
extern void System_EnableSensor(UINT32 SensorMask);
#endif
