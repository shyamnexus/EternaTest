
#ifndef _HID_DEF_H_
#define _HID_DEF_H_

#include <kwrap/nvt_type.h>
#include "kwrap/type.h"


/*---- HID ----*/
#define GET_REPORT                0x01
#define GET_IDLE                  0x02
#define GET_PROTOCOL              0x03
#define SET_REPORT                0x09
#define SET_IDLE                  0x0A
#define SET_PROTOCOL              0x0B

#define HID_REPORT_TYPE_INPUT      0x1
#define HID_REPORT_TYPE_OUTPUT     0x2
#define HID_REPORT_TYPE_FEATURE    0x3

#define USB_HID_DATA_SIZE_1                 1
#define USB_HID_DATA_SIZE_2                 2
#define USB_HID_DATA_SIZE_3                 3
#define USB_HID_DATA_SIZE_4                 4
#define USB_HID_DATA_SIZE_5                 5
#define USB_HID_DATA_SIZE_6                 6
#define USB_HID_DATA_SIZE_7                 7
#define USB_HID_DATA_SIZE_8                 8

/********************************************************************/

/* Report Descriptor Definitions */
#define PHYSICAL                        (0x00U)
#define UNDIFINE                        (0x00U)
#define APPLICATION                     (0x01U)
#define LOGICAL                         (0x02U)

#define Data_Arr_Abs                    (0x00U)
#define Const_Arr_Abs                   (0x01U)
#define Data_Var_Abs                    (0x02U)
#define Const_Var_Abs                   (0x03U)
#define Data_Var_Rel                    (0x06U)
#define Data_Var_Abs_Null               (0x42U)
#define BuffBytes                       (0x01U)

#define HID_REPORT_ID(a)                0x85U,(a)
#define HID_USAGE(a)                    0x09U,(a)
#define HID_USAGE_SENSOR_DATA(a,b)      (a)|(b)     //This or-s the mod with the data field
#define HID_COLLECTION(a)               0xA1U,(a)
#define HID_REPORT_SIZE(a)              0x75U,(a)
#define HID_REPORT_COUNT(a)             0x95U,(a)
#define HID_REPORT_COUNT_16(a,b)        0x96U,(a),(b)
#define HID_UNIT_EXPONENT(a)            0x55U,(a)
#define HID_UNIT(a)                     0x65U,(a)

#define HID_USAGE_8(a)                  0x09U,(a)
#define HID_USAGE_16(a,b)               0x0AU,(a),(b)

#define HID_USAGE_PAGE_8(a)             0x05U,(a)
#define HID_USAGE_PAGE_16(a,b)          0x06U,(a),(b)

#define HID_USAGE_MIN_8(a)              0x19U,(a)
#define HID_USAGE_MIN_16(a,b)           0x1AU,(a),(b)

#define HID_USAGE_MAX_8(a)              0x29U,(a)
#define HID_USAGE_MAX_16(a,b)           0x2AU,(a),(b)

#define HID_LOGICAL_MIN_8(a)            0x15U,(a)
#define HID_LOGICAL_MIN_16(a,b)         0x16U,(a),(b)
#define HID_LOGICAL_MIN_32(a,b,c,d)     0x17U,(a),(b),(c),(d)

#define HID_LOGICAL_MAX_8(a)            0x25U,(a)
#define HID_LOGICAL_MAX_16(a,b)         0x26U,(a),(b)
#define HID_LOGICAL_MAX_32(a,b,c,d)     0x27U,(a),(b),(c),(d)

#define HID_PHYSICAL_MIN_8(a)           0x35U,(a)
#define HID_PHYSICAL_MIN_16(a,b)        0x36U,(a),(b)
#define HID_PHYSICAL_MIN_32(a,b,c,d)    0x37U,(a),(b),(c),(d)

#define HID_PHYSICAL_MAX_8(a)           0x45U,(a)
#define HID_PHYSICAL_MAX_16(a,b)        0x46U,(a),(b)
#define HID_PHYSICAL_MAX_32(a,b,c,d)    0x47U,(a),(b),(c),(d)

#define HID_INPUT_8(a)                  0x81U,(a)
#define HID_INPUT_16(a,b)               0x82U,(a),(b)
#define HID_INPUT_32(a,b,c,d)           0x83U,(a),(b),(c),(d)

#define HID_OUTPUT_8(a)                 0x91U,(a)
#define HID_OUTPUT_16(a,b)              0x92U,(a),(b)
#define HID_OUTPUT_32(a,b,c,d)          0x93U,(a),(b),(c),(d)

#define HID_FEATURE_8(a)                0xB1U,(a)
#define HID_FEATURE_16(a,b)             0xB2U,(a),(b)
#define HID_FEATURE_32(a,b,c,d)         0xB3U,(a),(b),(c),(d)

#define HID_END_COLLECTION()            0xC0U

#define PUSH                            0xA4U
#define POP                             0xB4U

//HID vendor define for CFU
#define VENDOR_USAGE_PAGE_CFU_TLC           0xFF0B
#define VENDOR_USAGE_CFU                    0x0104
#define VERSION_FEATURE_USAGE               0x62
#define CONTENT_OUTPUT_USAGE                0x61
#define CONTENT_RESPONSE_INPUT_USAGE        0x66
#define OFFER_OUTPUT_USAGE                  0x8E
#define OFFER_RESPONSE_INPUT_USAGE          0x8A
#define CFU_GET_REATURE_REPORT_ID           0x2A     //set 0 for not using Report ID
#define CFU_UPDATE_OFFER_REPORT_ID          0x2D
#define CFU_UPDATE_CONTENT_REPORT_ID        0x2C


//HID vendor define for Video Control
#define VENDOR_USAGE_PAGE_VC_TLC            0xFF07
#define VENDOR_USAGE_VC                     0x0212
#define VC_REQUEST_FEATURE_USAGE            0x01
#define VC_RESPONSE_INPUT_USAGE             0x02

#define VC_REQUEST_REPORT_ID                0x24
#define VC_RESPONSE_REPORT_ID               0x27

#define VC_STATUS_OK               0x00
#define VC_STATUS_BUSY             0x01
#define VC_STATUS_INVALID_PARAM    0x02
#define VC_STATUS_FAILED           0x03

#define VC_MAX_SIZE            63   //not including report ID
//CICI spec 8.3	Command definition
#define VC_CMD_GET_FW_VER      0x01 //8.3.1	GetFirmwareVersion (0x01)
#define VC_CMD_GET_SN          0x02 //8.3.2	GetProductSN (0x02)
#define VC_CMD_SET_HDR         0x03 //8.3.3	SetHDR (0x03)
#define VC_CMD_GET_HDR         0x04 //8.3.4	GetHDR (0x04)
#define VC_CMD_SET_RETOUCH     0x05 //8.3.5	SetRetouch (0x05)
#define VC_CMD_GET_RETOUCH     0x06 //8.3.6	GetRetouch (0x06)
#define VC_CMD_SET_AWB         0x07 //8.3.7	SetAwbPreference (0x07)
#define VC_CMD_GET_AWB         0x08 //8.3.8	GetAwbPreference (0x08)
#define VC_CMD_SET_MIC_STA     0x09 //8.3.9	SetMicrophoneState (0x09)
#define VC_CMD_GET_MIC_STA     0x0A //8.3.10 GetMicrophoneState (0x0A)
#define VC_CMD_GET_COLOR_NAME  0x0B //8.3.11 GetColorName (0x0B)
#define VC_CMD_GET_SENSOR_SN   0x0C //8.3.12 GetCameraModuleSN (0x0C)
#define VC_CMD_RESET_USB_POWER 0x0D //8.3.13 ResetUsbPower (0x0D)
#define VC_CMD_GET_CAPABILITY  0x0E //8.3.14 GetDeviceCapability (0x0E)
#define VC_CMD_GET_WRITE_PROT  0x10 //8.3.15 GetFlashWriteProtectionStatus (0x10)
#define VC_CMD_GET_SENSOR_CHK  0x11 //8.3.16 GetSensorChecking (0x11)
#define VC_CMD_GET_CCM_TYPE    0x12 //8.3.17 GetCCMType (0x12)
#define VC_CMD_GET_SHUTTER_DIR 0x13 //8.3.18 GetShutterDirectionType (0x13)
#define VC_CMD_SET_SHUTTER_CHK 0x14 //8.3.19 SetShutterCheckStatus (0x14)
#define VC_CMD_GET_SHUTTER_CHK 0x15 //8.3.20 GetShutterCheckStatus (0x15)
#define VC_CMD_RESET_USER_CFG  0xAE //8.3.14 ResetUserConfiguration (0xAE)
//CICI spec 8.4	MTE commands
#define VC_CMD_SET_TEST_MODE   0x80 //8.4.1	SetTestMode (0x80)
#define VC_CMD_GET_TEST_MODE   0x81 //8.4.2	GetTestMode (0x81)
#define VC_CMD_SWITCH_ISP_MODE 0x82 //8.4.3	Switch ISP running mode (0x82)
#define VC_CMD_GET_ISP_MODE    0x83 //8.4.4	Get ISP running mode (0x83)
#define VC_CMD_GET_SHUTTER_STA 0x84 //8.4.5	GetShutterStatus (0x84)
#define VC_CMD_SET_LED_STA     0x85 //8.4.6	SetLedStatus (0x85)
#define VC_CMD_SET_SN          0x86 //8.4.7	SetProductSerialNumber (0x86)
#define VC_CMD_SET_COLOR_NAME  0x87 //8.4.8	SetColorName (0x87)
#define VC_CMD_SET_MANUAL_GAIN 0x88 //8.4.9	SetSensorManualGain (0x88)
#define VC_CMD_GET_MANUAL_GAIN 0x89 //8.4.10 GetSensorManualGain (0x89)
#define VC_CMD_SET_MANUAL_EXPO 0x8A //8.4.11 SetSensorManualExposure (0x8A)
#define VC_CMD_GET_MANUAL_EXPO 0x8B //8.4.12 GetSensorManualExposure (0x8B)
#define VC_CMD_SET_LED_BRIGHT  0x8C //8.4.13 SetLedBrightness (0x8C)
#define VC_CMD_GET_LED_BRIGHT  0x8D //8.4.14 SetLedBrightness (0x8D)
#define VC_CMD_SET_CAPABILITY  0x8E //8.4.15 SetDeviceCapability (0x8E)
#define VC_CMD_SET_SENSOR_CHK  0x8F //8.4.16 SetSensorChecking (0x8F)
#define VC_CMD_SET_CCM_TYPE    0x90 //8.4.17 SetCCMType (0x90)
#define VC_CMD_SET_SHUTTER_DIR 0x91 //8.4.16 SetShutterDirectionType (0x91)

#define VC_CMD_DUMMY           0x00 //dummy command


//HID vendor define for Telemetry
#define TELEMETRY_USAGE_PAGE_TLC            0xFF99
#define TELEMETRY_USAGE                     0x1

#define TELEMETRY_CLIENT_REPORT_ID          0x9A
#define TELEMETRY_DEVICE_REPORT_ID          0x9B

#define TELEMETRY_MSG_ID_VERSION            0x00
#define TELEMETRY_MSG_ID_ENUM               0x10
#define TELEMETRY_MSG_ID_LOCALE             0x20
#define TELEMETRY_MSG_ID_CLIENT             0x60
#define TELEMETRY_MSG_ID_DEVICE             0x40
#define TELEMETRY_MSG_ID_DEVICE_RESPONSE    0x41

#define TELEMETRY_ELEMENT_ID_CURRENT_FW     0x01
#define TELEMETRY_ELEMENT_ID_DEVICE_MODEL   0x04
#define TELEMETRY_ELEMENT_ID_DEVICE_SN      0x06
#define TELEMETRY_ELEMENT_ID_ERROR_MSG      0x10



//CFU define
#define CFU_FW_OFFER_NORMAL   0x1
#define CFU_FW_OFFER_SPECIAL  0xFF

#define OFFER_INFO_START_ENTIRE_TRANSACTION 0
#define OFFER_INFO_START_OFFER_LIST         0x1
#define OFFER_INFO_END_OFFER_LIST           0x2

#define FIRMWARE_UPDATE_FLAG_FIRST_BLOCK  0x80
#define FIRMWARE_UPDATE_FLAG_LAST_BLOCK   0x40

//CFU FW update offer RR Code
typedef enum _CFU_RR_CODE {
	FIRMWARE_OFFER_REJECT_OLD_FW = 0,
	FIRMWARE_OFFER_REJECT_INV_COMPONENT,
	FIRMWARE_UPDATE_OFFER_SWAP_PENDING,
	ENUM_DUMMY4WORD(CFU_RR_CODE)
} CFU_RR_CODE;

//CFU FW update offer Status Code
typedef enum _CFU_OFFER_STATUS {
	FIRMWARE_UPDATE_OFFER_SKIP = 0,
	FIRMWARE_UPDATE_OFFER_ACCEPT,
	FIRMWARE_UPDATE_OFFER_REJECT,
	FIRMWARE_UPDATE_OFFER_BUSY,
	FIRMWARE_UPDATE_OFFER_COMMAND_READY,
	FIRMWARE_UPDATE_CMD_NOT_SUPPORTED = 0xFF,
	ENUM_DUMMY4WORD(CFU_OFFER_STATUS)
} CFU_OFFER_STATUS;

//CFU FW update content Status Code
typedef enum _CFU_CONTENT_STATUS {
	FIRMWARE_UPDATE_SUCCESS = 0,
	FIRMWARE_UPDATE_ERROR_PREPARE,
	FIRMWARE_UPDATE_ERROR_WRITE,
	FIRMWARE_UPDATE_ERROR_COMPLETE,
	FIRMWARE_UPDATE_ERROR_VERIFY,
	FIRMWARE_UPDATE_ERROR_CRC,
	FIRMWARE_UPDATE_ERROR_SIGNATURE,
	FIRMWARE_UPDATE_ERROR_VERSION,
	FIRMWARE_UPDATE_SWAP_PENDING,
	FIRMWARE_UPDATE_ERROR_INVALID_ADDR,
	FIRMWARE_UPDATE_ERROR_NO_OFFER,
	FIRMWARE_UPDATE_ERROR_INVALID,
	ENUM_DUMMY4WORD(CFU_CONTENT_STATUS)
} CFU_CONTENT_STATUS;

typedef _PACKED_BEGIN struct _CFU_FW_HEADER {
	UINT8    component_count;    ///< For Cici, it shall always be 1 (single component)
	UINT16   rsvd;               ///< Reserved fields. Sender must set these to 0. Receiver must ignore this value.
	UINT8    protocol_version;   ///< For Cici, it should be 0x2 (v2)
} _PACKED_END CFU_FW_HEADER;

typedef struct _CFU_FW_COMPONENT {
	UINT32   fw_ver;             ///< For Cici, it shall always be 1 (single component)
	UINT8    rsvd;               ///< Cici does not use this field. Set to 0
	UINT8    component_id;       ///< A unique identifier for the component. ID=0x01 for single component case.
	UINT16   vendor_spec;        ///< Cici does not use this field. Set to 0
} CFU_FW_COMPONENT;

typedef struct _CFU_FW_VERSION {
	CFU_FW_HEADER       header;
	CFU_FW_COMPONENT    component[7];
} CFU_FW_VERSION;

typedef struct _CFU_UPDATE_OFFER {
	UINT8     seg_num;      ///< This is not used for Cici, set to 0. If component_id is 0xFF, this field represents information code.
	UINT8     force_flag;   ///< bit[6] for version and bit[7] for reset. Cici offer will always set these flags to 0
	UINT8     component_id; ///< 0x01 for normal firmware offer; 0xFF for special offer information packet
	UINT8     token;        ///< The host inserts a unique token in the offer packet to component. This token must be returned by the component in the offer response
	UINT32    fw_ver;       ///< Firmware version, [31:24] for major one and [23:0] for minor one.
	UINT32    vendor_spec;  ///< Cici does not use these four bytes, ignore them.
	UINT32    protocol_ver; ///< Cici should set to 0x2.
} CFU_UPDATE_OFFER;

typedef struct _CFU_UPDATE_SPECIAL_OFFER {
	UINT8     information_code;
	UINT8     rsvd;
	UINT8     component_id;
	UINT8     token;
} CFU_UPDATE_SPECIAL_OFFER;

typedef struct _CFU_UPDATE_OFFER_RESP {
	UINT8            rsvd1[3];     ///< reserved
	UINT8            token;        ///< Token to identify the host.
	UINT32           rsvd2;        ///< reserved
	CFU_RR_CODE      RR;           ///< Reject Reason
	CFU_OFFER_STATUS status;       ///< This value indicates the component's decision to accept, pend, skip, or reject the offer.
} CFU_UPDATE_OFFER_RESP;

typedef struct _CFU_UPDATE_CONTENT {
	UINT8     flags;          ///< FIRMWARE_UPDATE_FLAG_FIRST_BLOCK or FIRMWARE_UPDATE_FLAG_LAST_BLOCK
	UINT8     data_length;    ///< The length of applicable Data field indicating the number of bytes to be written. The maximum value is 52.
	UINT16    seq_num;        ///< This value is created by the host and is unique for each content packet issued. The component must return the sequence number in its response to this request.
	UINT32    fw_addr;        ///< Address to write the data. The address is 0-based. The firmware uses this as an offset to determine the address as needed when placing the image in memory.
	UINT8     data[52];      ///< The byte array to write. The host typically sends blocks of 4 bytes based on product architecture. Any unused bytes in the end must be 0 padded.
} CFU_UPDATE_CONTENT;

typedef struct _CFU_UPDATE_CONTENT_RESP {
	UINT16             seq_num;      ///< sequence number
	UINT16             rsvd1;        ///< reserved
	CFU_CONTENT_STATUS status;
	UINT32             rsvd2[2];     ///< reserved
} CFU_UPDATE_CONTENT_RESP;

extern int Cfu_InstallID(void);
extern UINT32 cfu_init(void);
extern CFU_CONTENT_STATUS cfu_process_update_content(CFU_UPDATE_CONTENT *content);
extern UINT32 cfu_get_firmware_version(void);
extern UINT32 cfu_process_update_offer(CFU_UPDATE_OFFER *offer, CFU_UPDATE_OFFER_RESP *resp);
extern CFU_CONTENT_STATUS verify_and_burn_firmware(void);
extern UINT32 cfu_enable(void);
extern UINT32 cfu_disable(void);
extern UINT32 _cfu_disable(void);

#endif  //_HID_DEF_H_

