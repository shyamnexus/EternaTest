#ifndef _FLOW_CT_PU_H
#define _FLOW_CT_PU_H

// Define Bits Containing Capabilities of the control for the Get_Info request (section 4.1.2 in the UVC spec 1.1)
#define SUPPORT_GET_REQUEST							0x01
#define SUPPORT_SET_REQUEST							0x02
#define DISABLED_DUE_TO_AUTOMATIC_MODE				0x04
#define AUTOUPDATE_CONTROL							0x08
#define ASNCHRONOUS_CONTROL							0x10
#define RESERVED_BIT5								0x20
#define RESERVED_BIT6								0x40
#define RESERVED_BIT7								0x80

//Camera Terminal Control Selectors
#define CT_CONTROL_UNDEFINED						0x00
#define CT_SCANNING_MODE_CONTROL					0x01
#define CT_AE_MODE_CONTROL							0x02
#define CT_AE_PRIORITY_CONTROL						0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL			0x04
#define CT_EXPOSURE_TIME_RELATIVE_CONTROL			0x05
#define CT_FOCUS_ABSOLUTE_CONTROL					0x06
#define CT_FOCUS_RELATIVE_CONTROL					0x07
#define CT_FOCUS_AUTO_CONTROL						0x08
#define CT_IRIS_ABSOLUTE_CONTROL					0x09
#define CT_IRIS_RELATIVE_CONTROL					0x0A
#define CT_ZOOM_ABSOLUTE_CONTROL					0x0B
#define CT_ZOOM_RELATIVE_CONTROL					0x0C
#define CT_PANTILT_ABSOLUTE_CONTROL					0x0D
#define CT_PANTILT_RELATIVE_CONTROL					0x0E
#define CT_ROLL_ABSOLUTE_CONTROL					0x0F
#define CT_ROLL_RELATIVE_CONTROL					0x10
#define CT_PRIVACY_CONTROL							0x11
#define CT_REGION_OF_INTEREST_CONTROL               0x14

//Processing Unit Control Selectors
#define PU_CONTROL_UNDEFINED						0x00
#define PU_BACKLIGHT_COMPENSATION_CONTROL			0x01
#define PU_BRIGHTNESS_CONTROL						0x02
#define PU_CONTRAST_CONTROL							0x03
#define PU_GAIN_CONTROL								0x04
#define PU_POWER_LINE_FREQUENCY_CONTROL				0x05
#define PU_HUE_CONTROL								0x06
#define PU_SATURATION_CONTROL						0x07
#define PU_SHARPNESS_CONTROL						0x08
#define PU_GAMMA_CONTROL							0x09
#define PU_WHITE_BALANCE_TEMPERATURE_CONTROL		0x0A
#define PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL	0x0B
#define PU_WHITE_BALANCE_COMPONENT_CONTROL			0x0C
#define PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL		0x0D
#define PU_DIGITAL_MULTIPLIER_CONTROL				0x0E
#define PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL			0x0F
#define PU_HUE_AUTO_CONTROL							0x10
#define PU_ANALOG_VIDEO_STANDARD_CONTROL			0x11
#define PU_ANALOG_LOCK_STATUS_CONTROL				0x12

typedef struct _PTZ_PARAM {
	UINT8 zoom_idx;
	UINT8 zoom_idx_max;
	INT32 pan_v;
	INT32 tilt_v;
	INT32 pan_idx;
	INT32 tilt_idx;
	UINT32 zoom_set_max;
} PTZ_PARAM;

extern UINT32 uvc_ct_controls;
extern UINT32 uvc_pu_controls;
extern BOOL xUvacPU_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen);
extern BOOL xUvacCT_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen);
extern PTZ_PARAM* uvac_get_ptz_param(void);

#endif
