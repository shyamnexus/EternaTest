/**
    USB3 Vision, APIs declare.

    @file       usb3vision.h
    @note       --

    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

#ifndef _USB3VISIONAPI_H
#define _USB3VISIONAPI_H

#include "usb3dev.h"


/**
    Callback function prototype for class request.
*/
typedef void (*USB3VISION_CLASS_REQUEST_CB)(PUSB_DEVICE_REQUEST request);
typedef BOOL (*USB3VISION_CLASS_REQUEST_DONE_CB)(PUSB_DEVICE_REQUEST request);

/**
    USB3 Vision configuration identifier.
*/
typedef enum _USB3VISION_CONFIG_ID {
	USB3VISION_CONFIG_ID_DESC,
	USB3VISION_CONFIG_ID_EP,
	USB3VISION_CONFIG_ID_CLASS_REQUEST_CB,
	USB3VISION_CONFIG_ID_CLASS_REQUEST_DONE_CB,
	ENUM_DUMMY4WORD(USB3VISION_CONFIG_ID)
} USB3VISION_CONFIG_ID;


typedef enum _USB3VISION_DESC_TYPE {
	USB3VISION_DESC_SS_DEVICE = 0,
	USB3VISION_DESC_SS_CONFIG,
	USB3VISION_DESC_SS_BOS,
	USB3VISION_DESC_HS_DEVICE,
	USB3VISION_DESC_HS_CONFIG,
	USB3VISION_DESC_HS_DEVICEQUALIFIER,
	USB3VISION_DESC_HS_OTHERSPEED,
	USB3VISION_DESC_FS_DEVICE,
	USB3VISION_DESC_FS_CONFIG,
	USB3VISION_DESC_FS_DEVICEQUALIFIER,
	USB3VISION_DESC_FS_OTHERSPEED,
	USB3VISION_DESC_STRING0,
	USB3VISION_DESC_STRING1,
	USB3VISION_DESC_STRING2,
	USB3VISION_DESC_STRING3,
	USB3VISION_DESC_STRING4,
	USB3VISION_DESC_STRING5,
	USB3VISION_DESC_STRING6,
	USB3VISION_DESC_STRING7,
	USB3VISION_DESC_STRING8,
	USB3VISION_DESC_STRING9,
	USB3VISION_DESC_STRING10,
	USB3VISION_DESC_STRING11,
	USB3VISION_DESC_TYPE_MAX,
	ENUM_DUMMY4WORD(USB3VISION_DESC_TYPE)
} USB3VISION_DESC_TYPE;

typedef enum _USB3VISION_EP_TYPE {
	USB3VISION_EP_INF_CONTROL_IN = 0,
	USB3VISION_EP_INF_CONTROL_OUT,
	USB3VISION_EP_INF_EVENT_IN,
	USB3VISION_EP_INF_STREAM_IN,
	USB3VISION_EP_TYPE_MAX,
	ENUM_DUMMY4WORD(USB3VISION_EP_TYPE)
} USB3VISION_EP_TYPE;

typedef enum _USB3VISION_EP {
	USB3VISION_EP1 = 1,
	USB3VISION_EP2,
	USB3VISION_EP3,
	USB3VISION_EP4,
	USB3VISION_EP5,
	USB3VISION_EP6,
	USB3VISION_EP7,
	USB3VISION_EP8,
	ENUM_DUMMY4WORD(USB3VISION_EP)
} USB3VISION_EP;

/**
    Set USB3 Vision configuration

    Assign new configuration of the specified config_id.

    @param[in] config_id        Configuration identifier
    @param[in] param1           Configuration context for config_id
    @param[in] param2           Configuration context for config_id

    @return
        - @b E_OK: Start to transfer.
        - @b E_SYS: Failed.
*/
extern ER usb3vision_set_config(USB3VISION_CONFIG_ID config_id, ULONG param1, ULONG param2);

/**
    Get USB3 Vision configuration

    Get configuration value of the specified config_id.

    @param[in] config_id        Configuration identifier
    @param[in] param            Configuration context for config_id

    @return
        - @b ULONG configuration value
*/
extern ULONG usb3vision_get_config(USB3VISION_CONFIG_ID config_id, ULONG param);

/**
    Open USB3 Vision module.

*/
extern ER usb3vision_open(void);

/**
    Close USB Vision module.
*/
extern ER usb3vision_close(void);

/**
    Read data from USB host.

    @param[in] ep  Endpoint type
    @param[in] buf  Buffer pointer
    @param[in] buf_size  Input length to write.
    @param[in] timeout  wait ms, -1 for blocking, 0 for non-blocking

    @return
        - @b E_OK:  Success.
        - @b E_SYS: Failed
        - @b E_RSATR: UVAC_AbortCdcRead() is invoked by another task.
*/
extern ER usb3vision_read_data(USB3VISION_EP_TYPE ep, UINT8 *buf_pa, UINT32 *buf_size, UINT32 timeout);

/**
    Abort the function of usb3vision_read_data().
*/
extern ER usb3vision_abort_read(USB3VISION_EP_TYPE ep);

/**
    Write data to USB host.

    This function is a blocking API and will return after data is sent to USB host or timeout.

	@param[in] ep  Endpoint type
    @param[in] buf  Buffer pointer
    @param[in] buf_size  Input length to write.
    @param[in] timeout  wait ms, -1 for blocking, 0 for non-blocking

    @return
        - @b INT32: positive value means acturally written size, negative value means USB engine busy or error code.
*/
extern INT32 usb3vision_write_data(USB3VISION_EP_TYPE ep, UINT8 *buf_pa, UINT32 *buf_size, UINT32 timeout);
#endif

