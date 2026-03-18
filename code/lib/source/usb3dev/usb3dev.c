
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <kwrap/nvt_type.h>
#include <kwrap/task.h>
#include "usb3dev.h"
#include "usb3dev_ioctl.h"
#include <pthread.h>

//#include <kwrap/debug.h>
//#include <kwrap/error_no.h>
//#include <kwrap/task.h>
//#include <kwrap/flag.h>
//#include "../../hdal/include/hd_type.h"

#define MODULE_PATH  "/dev/nvt_usb3dev0"


//#define USB_MNG_MAGICNO                 0xAD5A609F
USB_DEVICE_REQUEST usb3dev_devreq;

#define DBG_ERR  printf

static int m_fd = -1;


static USB3_GENERIC_CB	usb3dev_evt_cb = NULL;
static USB3_GENERIC_CB	usb3dev_cx_vendor_cb = NULL;
static USB3_GENERIC_CB	usb3dev_cx_vendordone_cb = NULL;
static USB3_GENERIC_CB	usb3dev_cx_class_cb = NULL;
static USB3_GENERIC_CB	usb3dev_cx_classdone_cb = NULL;
static USB3_GENERIC_CB	usb3dev_set_intf_cb = NULL;
static USB3_GENERIC_CB	usb3dev_charging_cb = NULL;
static USB3_GENERIC_CB	usb3dev_std_unknown_cb = NULL;
static USB3_GENERIC_CB	usb3dev_set_config_cb = NULL;
static USB3_GENERIC_CB	usb3dev_suspend_cb = NULL;

static USB3DEV_DESC	desc_data;
static USB3DEV_EVENT usbioc_event;


#if 1
THREAD_HANDLE USB3DEV_DRV_ID = 0;
THREAD_DECLARE(usb3dev_driver_thread, arglist)
{
	int er;

	while(1) {
		if ((er = ioctl(m_fd, USB3DEV_IOC_GET_EVENT, &usbioc_event)) < 0) {
			DBG_ERR("IOC_GET_EVENT failed.\n");
		}

		//DBG_DUMP("event got = %d %d\r\n", usbioc_event.evt_type, usbioc_event.event);


		switch (usbioc_event.evt_type) {
		case USB3DEV_EVT_TYPE_USBEVENT_CB:
			if (usb3dev_evt_cb) {
				usb3dev_evt_cb(usbioc_event.event);
			}
			break;
		case USB3DEV_EVT_TYPE_CX_VENDOR:
			if (usb3dev_cx_vendor_cb) {
				memcpy(&usb3dev_devreq, usbioc_event.device_request, 8);
				usb3dev_cx_vendor_cb((uintptr_t)&usb3dev_devreq);
			}
			break;
		case USB3DEV_EVT_TYPE_CX_VENDOR_DONE:
			if (usb3dev_cx_vendordone_cb) {
				memcpy(&usb3dev_devreq, usbioc_event.device_request, 8);
				usb3dev_cx_vendordone_cb((uintptr_t)&usb3dev_devreq);
			}
			break;
		case USB3DEV_EVT_TYPE_CX_CLASS:
			if (usb3dev_cx_class_cb) {
				memcpy(&usb3dev_devreq, usbioc_event.device_request, 8);
				usb3dev_cx_class_cb((uintptr_t)&usb3dev_devreq);
			}
			break;
		case USB3DEV_EVT_TYPE_CX_CLASS_DONE:
			if (usb3dev_cx_classdone_cb) {
				memcpy(&usb3dev_devreq, usbioc_event.device_request, 8);
				usb3dev_cx_classdone_cb((uintptr_t)&usb3dev_devreq);
			}
			break;
		case USB3DEV_EVT_TYPE_SET_INTF_CB:
			if (usb3dev_set_intf_cb) {
				usb3dev_set_intf_cb(usbioc_event.event);
			}
			break;

		case USB3DEV_EVT_TYPE_SET_CONFIG_CB:
			if (usb3dev_set_config_cb) {
				usb3dev_set_config_cb(usbioc_event.event);
			}
			break;

		case USB3DEV_EVT_TYPE_CHARGING_CB:
			if (usb3dev_charging_cb) {
				usb3dev_charging_cb(usbioc_event.event);
			}
			break;

		case USB3DEV_EVT_TYPE_STDUNKNOWN_CB:
			if (usb3dev_std_unknown_cb) {
				memcpy(&usb3dev_devreq, usbioc_event.device_request, 8);
				usb3dev_std_unknown_cb((uintptr_t)&usb3dev_devreq);
			}
			break;

		case USB3DEV_EVT_TYPE_SUSPEND_CB:
			if (usb3dev_suspend_cb) {
				memcpy(&usb3dev_devreq, usbioc_event.device_request, 8);
				usb3dev_suspend_cb((uintptr_t)&usb3dev_devreq);
			}
			break;

		default: {
				break;
			}
		}

		if ((er = ioctl(m_fd, USB3DEV_IOC_SET_EVENT_DONE, NULL)) < 0) {
			DBG_ERR("IOC_SET_EVENT_DONE failed.\n");
		}
	}

	THREAD_RETURN(0);
}

#endif


ER usb3dev_open(void)
{
	int er,ret;

	if (m_fd == -1) {
		if ((m_fd = open(MODULE_PATH, O_RDWR)) < 0) {
			DBG_ERR("Error to open %s\n", MODULE_PATH);
			return -1;
		}
	}

	/*ret = pthread_create(&USB3DEV_DRV_ID, NULL, usb3dev_driver_thread, (void *)NULL);
	if (ret < 0) {
		printf("create usb3dev_driver_thread thread failed");
		return -1;
	}*/

	THREAD_CREATE(USB3DEV_DRV_ID, usb3dev_driver_thread, NULL, "usb3dev_driver_thread");
	THREAD_SET_PRIORITY(USB3DEV_DRV_ID, 4);
	THREAD_RESUME(USB3DEV_DRV_ID);

	if ((er = ioctl(m_fd, USB3DEV_IOC_OPEN, NULL)) < 0) {
		DBG_ERR("USB2DEV_IOC_OPEN failed.\n");
		return -1;
	}

	return 0;
}

ER usb3dev_close(void)
{
	int er, ret;

	if (m_fd < 0) {
		DBG_ERR("usb2dev not opened.\n");
		return -1;
	}

	if ((er = ioctl(m_fd, USB3DEV_IOC_CLOSE, NULL)) < 0) {
		DBG_ERR("USB2DEV_IOC_CLOSE failed.\n");
		return -1;
	}

	close(m_fd);
	m_fd = -1;

	THREAD_DESTROY(USB3DEV_DRV_ID);

	/*ret = pthread_cancel(USB3DEV_DRV_ID);
	if (ret) {
		DBG_ERR("Cancel usb3dev_driver_thread thread 0x%u error %d.\n", (unsigned int)USB3DEV_DRV_ID, ret);
	}*/

	return 0;
}

ER usb3dev_setCallBack(U3DEV_CALLBACK_ID U3CallBackID, USB3_GENERIC_CB pCallBack)
{
	switch (U3CallBackID) {
	case U3DEV_CALLBACK_ID_CX_VENDOR_REQUEST: {
			usb3dev_cx_vendor_cb = pCallBack;
		}
		break;
	case U3DEV_CALLBACK_ID_CX_VENDOR_REQDONE: {
			usb3dev_cx_vendordone_cb = pCallBack;
		}
		break;
	case U3DEV_CALLBACK_ID_CX_CLASS_REQUEST: {
			usb3dev_cx_class_cb = pCallBack;
		}
		break;
	case U3DEV_CALLBACK_ID_CX_CLASS_REQDONE: {
			usb3dev_cx_classdone_cb = pCallBack;
		}
		break;
	case U3DEV_CALLBACK_ID_EVENT_NOTIFY: {
			usb3dev_evt_cb = pCallBack;
		}
		break;
	case U3DEV_CALLBACK_ID_CHARGING_EVENT: {
			usb3dev_charging_cb = pCallBack;
		}
		break;
	case U3DEV_CALLBACK_ID_SET_INTERFACE: {
			usb3dev_set_intf_cb = pCallBack;
		}
		break;

	case U3DEV_CALLBACK_ID_SET_CONFIGURATION: {
			usb3dev_set_config_cb = pCallBack;
		}
		break;

	case U3DEV_CALLBACK_ID_STD_UNKNOWN_REQ: {
			usb3dev_std_unknown_cb = pCallBack;
		}
		break;

	case U3DEV_CALLBACK_ID_SUSPEND: {
			usb3dev_suspend_cb = pCallBack;
		}
		break;

	default: {
			return -1;
		}
	}

	return 0;
}

void usb3dev_handleSetupResult(BOOL bStall, uintptr_t uiRetBufAddr, UINT32 uiRetSize)
{
	int er;
	USB3DEV_CX_RET_DATA RetDat;

	RetDat.stall     = bStall;
	RetDat.ui_length = uiRetSize;
	RetDat.p_data    = (UINT8 *)((uintptr_t)uiRetBufAddr);
	usbioc_event.p_data = (UINT8 *)((uintptr_t)uiRetBufAddr);

	if ((er = ioctl(m_fd, USB3DEV_IOC_CX_RET_DATA, &RetDat)) < 0) {
		DBG_ERR("IOC_CX_RET_DATA failed.\n");
	}
}

ER usb3dev_setConfig(U3DEV_CONFIG_ID CfgID, UINT32 uiConfig)
{
	int er;
	USB3DEV_SETCFG epcfg;

	if (m_fd == -1) {
		if ((m_fd = open(MODULE_PATH, O_RDWR)) < 0) {
			DBG_ERR("Error to open %s\n", MODULE_PATH);
		}
	}

	epcfg.cfgid		= CfgID;
	epcfg.cfg_value	= uiConfig;

	if ((er = ioctl(m_fd, USB3DEV_IOC_SET_CFG, &epcfg)) < 0) {
		DBG_ERR("IOC_SET_CFG failed.\n");
	}

	return 0;
}

ER usb3dev_setEPConfig(USB_EP EPn, U3DEV_EP_CONFIG_ID CfgID, UINT32 uiConfig)
{
	int er;
	USB3DEV_SETEPCFG epcfg;

	if (m_fd == -1) {
		if ((m_fd = open(MODULE_PATH, O_RDWR)) < 0) {
			DBG_ERR("Error to open %s\n", MODULE_PATH);
		}
	}

	epcfg.epn       = EPn;
	epcfg.epcfgid	= CfgID;
	epcfg.cfg_value	= uiConfig;

	if ((er = ioctl(m_fd, USB3DEV_IOC_SET_EP_CFG, &epcfg)) < 0) {
		DBG_ERR("IOC_SET_CFG failed.\n");
	}

	return 0;
}

ER usb3dev_setDescriptor(U3DEV_DESC_ID U3DescID, void *DescBuf)
{
	int				er;

	if (m_fd == -1) {
		if ((m_fd = open(MODULE_PATH, O_RDWR)) < 0) {
			DBG_ERR("Error to open %s\n", MODULE_PATH);
		}
	}

	switch (U3DescID) {

	/* Device Descriptor */
	case U3DEV_DESC_ID_SS_DEVICE: {
		PUSB_DEVICE_DESC pDesc;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->b_length;
		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);
		if ((er = ioctl(m_fd, USB3DEV_IOC_SET_SS_DEVICE_DESC, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_SS_DEVICE failed.\n");
			return -1;
		}
	} break;
	case U3DEV_DESC_ID_HS_DEVICE: {
		PUSB_DEVICE_DESC pDesc;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->b_length;
		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if ((er = ioctl(m_fd, USB3DEV_IOC_SET_HS_DEVICE_DESC, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_HS_DEVICE failed.\n");
			return -1;
		}
	} break;
	case U3DEV_DESC_ID_FS_DEVICE: {
		PUSB_DEVICE_DESC pDesc;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->b_length;
		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if ((er = ioctl(m_fd, USB3DEV_IOC_SET_FS_DEVICE_DESC, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_FS_DEVICE failed.\n");
			return -1;
		}
	} break;

	/* Config Descriptor */
	case U3DEV_DESC_ID_SS_CONFIG: {
		PUSB_CONFIG_DESC pDesc;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->w_total_length;
		if(desc_data.ui_length > USB_DESC_MAX_SIZE) {
			DBG_ERR("desc size exceed %d\r\n", USB_DESC_MAX_SIZE);
			return -10;
		}

		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if ((er = ioctl(m_fd, USB3DEV_IOC_SET_CFG_SS_DESC, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_SS_CONFIG failed.\n");
			return -1;
		}
	} break;
	case U3DEV_DESC_ID_HS_CONFIG: {
		PUSB_CONFIG_DESC pDesc;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->w_total_length;
		if(desc_data.ui_length > USB_DESC_MAX_SIZE) {
			DBG_ERR("desc size exceed %d\r\n", USB_DESC_MAX_SIZE);
			return -10;
		}

		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if ((er = ioctl(m_fd, USB3DEV_IOC_SET_CFG_HS_DESC, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_HS_CONFIG failed.\n");
			return -1;
		}
	} break;
	case U3DEV_DESC_ID_FS_CONFIG: {
		PUSB_CONFIG_DESC pDesc;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->w_total_length;
		if(desc_data.ui_length > USB_DESC_MAX_SIZE) {
			DBG_ERR("desc size exceed %d\r\n", USB_DESC_MAX_SIZE);
			return -10;
		}

		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if ((er = ioctl(m_fd, USB3DEV_IOC_SET_CFG_FS_DESC, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_FS_CONFIG failed.\n");
			return -1;
		}
	} break;
	case U3DEV_DESC_ID_HS_OTHERSPEED: {
		PUSB_CONFIG_DESC pDesc;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->w_total_length;
		if(desc_data.ui_length > USB_DESC_MAX_SIZE) {
			DBG_ERR("desc size exceed %d\r\n", USB_DESC_MAX_SIZE);
			return -10;
		}

		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if ((er = ioctl(m_fd, USB3DEV_IOC_SET_OTHERSPD_DESC_HS, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_HS_OTHERSPEED failed.\n");
			return -1;
		}
	} break;
	case U3DEV_DESC_ID_FS_OTHERSPEED: {
		PUSB_CONFIG_DESC pDesc;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->w_total_length;
		if(desc_data.ui_length > USB_DESC_MAX_SIZE) {
			DBG_ERR("desc size exceed %d\r\n", USB_DESC_MAX_SIZE);
			return -10;
		}

		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if ((er = ioctl(m_fd, USB3DEV_IOC_SET_OTHERSPD_DESC_FS, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_FS_OTHERSPEED failed.\n");
			return -1;
		}
	} break;

	/* String Descriptor */
	case U3DEV_DESC_ID_SS_STRING0:
	case U3DEV_DESC_ID_SS_STRING1:
	case U3DEV_DESC_ID_SS_STRING2:
	case U3DEV_DESC_ID_SS_STRING3:
	case U3DEV_DESC_ID_SS_STRING4:
	case U3DEV_DESC_ID_SS_STRING5:
	case U3DEV_DESC_ID_SS_STRING6:
	case U3DEV_DESC_ID_SS_STRING7:
	case U3DEV_DESC_ID_SS_STRING8:
	case U3DEV_DESC_ID_SS_STRING9:
	case U3DEV_DESC_ID_SS_STRING10:
	case U3DEV_DESC_ID_SS_STRING11:
	{
		PUSB_STRING_DESC pDesc;
		int id = USB3DEV_IOC_SET_STRING0_DESC_SS;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->b_length;
		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if (U3DescID == U3DEV_DESC_ID_SS_STRING0)
			id = USB3DEV_IOC_SET_STRING0_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING1)
			id = USB3DEV_IOC_SET_STRING1_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING2)
			id = USB3DEV_IOC_SET_STRING2_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING3)
			id = USB3DEV_IOC_SET_STRING3_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING4)
			id = USB3DEV_IOC_SET_STRING4_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING5)
			id = USB3DEV_IOC_SET_STRING5_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING6)
			id = USB3DEV_IOC_SET_STRING6_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING7)
			id = USB3DEV_IOC_SET_STRING7_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING8)
			id = USB3DEV_IOC_SET_STRING8_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING9)
			id = USB3DEV_IOC_SET_STRING9_DESC_SS;
		else if  (U3DescID == U3DEV_DESC_ID_SS_STRING10)
			id = USB3DEV_IOC_SET_STRING10_DESC_SS;
		else
			id = USB3DEV_IOC_SET_STRING11_DESC_SS;

		if ((er = ioctl(m_fd, id, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_SS_STRING failed.\n");
			return -1;
		}
	}
	break;

	case U3DEV_DESC_ID_HS_STRING0:
	case U3DEV_DESC_ID_HS_STRING1:
	case U3DEV_DESC_ID_HS_STRING2:
	case U3DEV_DESC_ID_HS_STRING3:
	case U3DEV_DESC_ID_HS_STRING4:
	case U3DEV_DESC_ID_HS_STRING5:
	case U3DEV_DESC_ID_HS_STRING6:
	case U3DEV_DESC_ID_HS_STRING7:
	case U3DEV_DESC_ID_HS_STRING8:
	case U3DEV_DESC_ID_HS_STRING9:
	case U3DEV_DESC_ID_HS_STRING10:
	case U3DEV_DESC_ID_HS_STRING11:
	{
		PUSB_STRING_DESC pDesc;
		int id = USB3DEV_IOC_SET_STRING0_DESC_HS;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->b_length;
		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if (U3DescID == U3DEV_DESC_ID_HS_STRING0)
			id = USB3DEV_IOC_SET_STRING0_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING1)
			id = USB3DEV_IOC_SET_STRING1_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING2)
			id = USB3DEV_IOC_SET_STRING2_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING3)
			id = USB3DEV_IOC_SET_STRING3_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING4)
			id = USB3DEV_IOC_SET_STRING4_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING5)
			id = USB3DEV_IOC_SET_STRING5_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING6)
			id = USB3DEV_IOC_SET_STRING6_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING7)
			id = USB3DEV_IOC_SET_STRING7_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING8)
			id = USB3DEV_IOC_SET_STRING8_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING9)
			id = USB3DEV_IOC_SET_STRING9_DESC_HS;
		else if  (U3DescID == U3DEV_DESC_ID_HS_STRING10)
			id = USB3DEV_IOC_SET_STRING10_DESC_HS;
		else
			id = USB3DEV_IOC_SET_STRING11_DESC_HS;

		if ((er = ioctl(m_fd, id, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_HS_STRING failed.\n");
			return -1;
		}
	}
	break;

	case U3DEV_DESC_ID_FS_STRING0:
	case U3DEV_DESC_ID_FS_STRING1:
	case U3DEV_DESC_ID_FS_STRING2:
	case U3DEV_DESC_ID_FS_STRING3:
	case U3DEV_DESC_ID_FS_STRING4:
	case U3DEV_DESC_ID_FS_STRING5:
	case U3DEV_DESC_ID_FS_STRING6:
	case U3DEV_DESC_ID_FS_STRING7:
	case U3DEV_DESC_ID_FS_STRING8:
	case U3DEV_DESC_ID_FS_STRING9:
	case U3DEV_DESC_ID_FS_STRING10:
	case U3DEV_DESC_ID_FS_STRING11:
	{
		PUSB_STRING_DESC pDesc;
		int id = USB3DEV_IOC_SET_STRING0_DESC_FS;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->b_length;
		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if (U3DescID == U3DEV_DESC_ID_FS_STRING0)
			id = USB3DEV_IOC_SET_STRING0_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING1)
			id = USB3DEV_IOC_SET_STRING1_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING2)
			id = USB3DEV_IOC_SET_STRING2_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING3)
			id = USB3DEV_IOC_SET_STRING3_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING4)
			id = USB3DEV_IOC_SET_STRING4_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING5)
			id = USB3DEV_IOC_SET_STRING5_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING6)
			id = USB3DEV_IOC_SET_STRING6_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING7)
			id = USB3DEV_IOC_SET_STRING7_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING8)
			id = USB3DEV_IOC_SET_STRING8_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING9)
			id = USB3DEV_IOC_SET_STRING9_DESC_FS;
		else if  (U3DescID == U3DEV_DESC_ID_FS_STRING10)
			id = USB3DEV_IOC_SET_STRING10_DESC_FS;
		else
			id = USB3DEV_IOC_SET_STRING11_DESC_FS;

		if ((er = ioctl(m_fd, id, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_FS_STRING failed.\n");
			return -1;
		}
	}
	break;

	case U3DEV_DESC_ID_SS_BOS:
	case U3DEV_DESC_ID_HS_BOS:
	{
		PUSB_CONFIG_DESC pDesc;
		int id = USB3DEV_IOC_SET_BOS_DESC_SS;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->w_total_length;
		if(desc_data.ui_length > USB_DESC_MAX_SIZE) {
			DBG_ERR("desc size exceed %d\r\n", USB_DESC_MAX_SIZE);
			return -10;
		}

		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if (U3DescID == U3DEV_DESC_ID_SS_BOS)
			id = USB3DEV_IOC_SET_BOS_DESC_SS;
		else
			id = USB3DEV_IOC_SET_BOS_DESC_HS;

		if ((er = ioctl(m_fd, id, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_SS_BOS failed.\n");
			return -1;
		}
	}
	break;

	case U3DEV_DESC_ID_HS_DEVICEQUALIFIER:
	case U3DEV_DESC_ID_FS_DEVICEQUALIFIER:
	{
		PUSB_DEVICE_DESC pDesc;
		int id = USB3DEV_IOC_SET_DEVQUALI_DESC_HS;

		pDesc = DescBuf;
		desc_data.ui_length = pDesc->b_length;
		memcpy(desc_data.desc_data, pDesc, desc_data.ui_length);

		if (U3DescID == U3DEV_DESC_ID_HS_DEVICEQUALIFIER)
			id = USB3DEV_IOC_SET_DEVQUALI_DESC_HS;
		else
			id = USB3DEV_IOC_SET_DEVQUALI_DESC_FS;

		if ((er = ioctl(m_fd, id, &desc_data)) < 0) {
			DBG_ERR("U3DEV_DESC_ID_HS_DEVICEQUALIFIER failed.\n");
			return -1;
		}
	}
	break;


	default:
		DBG_ERR("usb3dev_setDescriptor %d failed.\n", U3DescID);
		return -2;


	}

	return 0;
}

UINT32 usb3dev_getDevInfo(U3DEV_DEVINFO_ID InfoID)
{
	int er;
	UINT32 value = 0;

	if ((er = ioctl(m_fd, USB3DEV_IOC_GET_DEVINFO, &value)) < 0) {
		DBG_ERR("USB3DEV_IOC_GET_DEVINFO failed.\n");
	}

	return value;
}

BOOL usb3dev_chkEPBusy(USB_EP EPn, U3DEV_EPDIR EpDir)
{
	USB3DEV_CHKBUSY epbusy;
	int er;
	UINT32 value = 0;

	epbusy.epn 		= EPn;
	epbusy.ep_dir	= EpDir;
	epbusy.busy		= &value;

	if ((er = ioctl(m_fd, USB3DEV_IOC_CHK_EPBUSY, &epbusy)) < 0) {
		DBG_ERR("USB3DEV_IOC_CHK_EPBUSY failed.\n");
	}

	return (BOOL)*(epbusy.busy);
}

UINT32 usb3dev_getSOF(void)
{
	int er;
	UINT32 value = 0;

	if ((er = ioctl(m_fd, USB3DEV_IOC_GET_SOF, &value)) < 0) {
		DBG_ERR("USB3DEV_IOC_GET_SOF failed.\n");
	}

	return value;
}


ER usb3dev_writeEndpoint(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen)
{
	int er;
	USB3DEV_DATA_RW data_rw;

	data_rw.epn 		= EPn;
	data_rw.buffer		= pBuffer;
	data_rw.dma_length	= pDMALen;

	if ((er = ioctl(m_fd, USB3DEV_IOC_WRITE_EP, &data_rw)) < 0) {
		DBG_ERR("IOC_WRITE_EP failed.\n");
	}

	return 0;
}

ER usb3dev_writeEndpoint_timeout(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen, UINT32 timeout_ms)
{
	int er;
	USB3DEV_DATA_RW data_rw;

	data_rw.epn 		= EPn;
	data_rw.buffer		= pBuffer;
	data_rw.dma_length	= pDMALen;
	data_rw.timeout_ms  = timeout_ms;

	if ((er = ioctl(m_fd, USB3DEV_IOC_WRITE_EP_TMOT, &data_rw)) < 0) {
		DBG_ERR("IOC_WRITE_EP failed.\n");
	}

	return 0;
}

ER usb3dev_readEndpoint(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen)
{
	int er;
	USB3DEV_DATA_RW data_rw;

	data_rw.epn 		= EPn;
	data_rw.buffer		= pBuffer;
	data_rw.dma_length	= pDMALen;

	if ((er = ioctl(m_fd, USB3DEV_IOC_READ_EP, &data_rw)) < 0) {
		DBG_ERR("IOC_WRITE_EP failed.\n");
	}

	return 0;
}

ER usb3dev_setIsoEPWrite(USB_EP EPn, PU3DEV_ISODATA pIsoData)
{
	int er;
	USB3DEV_ISODATA_RW data_rw;

	data_rw.epn 						= EPn;
	data_rw.IsoData.uiPacketNumber		= pIsoData->uiPacketNumber;
	data_rw.IsoData.pIsoPacket			= pIsoData->pIsoPacket;
	data_rw.IsoData.uiNotFinishedSize	= pIsoData->uiNotFinishedSize;

	if ((er = ioctl(m_fd, USB3DEV_IOC_WRITE_ISOEP, &data_rw)) < 0) {
		DBG_ERR("IOC_WRITE_EP failed.\n");
	}

	return 0;
}

void usb3dev_abortEndpoint(USB_EP EPn)
{
	int er;
	USB3DEV_DATA_RW data_rw;

	data_rw.epn 		= EPn;
	data_rw.buffer		= 0;
	data_rw.dma_length	= 0;

	if ((er = ioctl(m_fd, USB3DEV_IOC_ABORT_EP, &data_rw)) < 0) {
		DBG_ERR("IOC_WRITE_EP failed.\n");
	}
}

ER usb3dev_setInterruptEnable(U3DEV_INTRR Interrupt, BOOL bEnable)
{
	int er;
	USB3DEV_SET_INT interrupt;

	interrupt.intrr 		= Interrupt;
	interrupt.enable		= bEnable;

	if ((er = ioctl(m_fd, USB3DEV_IOC_SET_INT, &interrupt)) < 0) {
		DBG_ERR("IOC_WRITE_EP failed.\n");
	}

	return 0;
}

UINT32 usb3dev_getEPBytecount(USB_EP EPn)
{
	int er;
	UINT32 value;

	value = EPn;
	if ((er = ioctl(m_fd, USB3DEV_IOC_GET_BYTECNT, &value)) < 0) {
		DBG_ERR("IOC_GET_BYTECNT failed.\n");
	}

	return value;
}

void usb3dev_set_ep_stall(USB_EP EPn)
{
	int er;
	UINT32 value;

	value = EPn;
	if ((er = ioctl(m_fd, USB3DEV_IOC_STALL_EP, &value)) < 0) {
		DBG_ERR("IOC_STALL_EP failed.\n");
	}
}

void usb3dev_set_tx0byte(USB_EP EPn)
{
	int er;
	UINT32 value;

	value = EPn;
	if ((er = ioctl(m_fd, USB3DEV_IOC_SET_TX0, &value)) < 0) {
		DBG_ERR("IOC_STALL_EP failed.\n");
	}
}

void usb3_reset_ep_cfg(USB_EP EPn, UINT32 EpType, UINT32 MaxPktSz, UINT32 MaxBurst, UINT32 bInterval, BOOL isIN, UINT32 BwNum)
{
	int er;
	USB3DEV_RESETEP reset_ep;

	reset_ep.epn       = EPn;
	reset_ep.EpType    = EpType;
	reset_ep.MaxPktSz  = MaxPktSz;
	reset_ep.MaxBurst  = MaxBurst;
	reset_ep.bInterval = bInterval;
	reset_ep.isIN      = isIN;
	reset_ep.BwNum     = BwNum;

	if ((er = ioctl(m_fd, USB3DEV_IOC_RESET_EP_CFG, &reset_ep)) < 0) {
		DBG_ERR("USB3DEV_IOC_RESET_EP_CFG failed.\n");
	}
}

BOOL usb3dev_state_change(void)
{
	int er;
	UINT32 value = 0;
	BOOL ret;

	if (m_fd == -1) {
		if ((m_fd = open(MODULE_PATH, O_RDWR)) < 0) {
			DBG_ERR("Error to open %s\n",MODULE_PATH);
		}
	}

	if ((er = ioctl(m_fd, USB3DEV_IOC_STATE_CHG, &value)) < 0) {
		DBG_ERR("IOC_STATE_CHG failed.\n");
	}
	//DBG_ERR("u3 state_change value=%d\n",value);

	ret = value;
	return ret;
}

USB_CHARGER_STS usb3dev_check_charger(UINT32 uiDebounce)
{
	int er;
	UINT32 value;
	USB_CHARGER_STS ret;

	value = uiDebounce;
	if ((er = ioctl(m_fd, USB3DEV_IOC_CHK_CHARGER, &value)) < 0) {
		DBG_ERR("IOC_CHK_CHARGER failed.\n");
	}

	ret = (USB_CHARGER_STS)value;
	return ret;
}

ER usb3dev_setEPWrite(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen)
{
	int er;
	USB3DEV_DATA_RW data_rw;

	data_rw.epn 		= EPn;
	data_rw.buffer		= pBuffer;
	data_rw.dma_length	= pDMALen;

	if ((er = ioctl(m_fd, USB3DEV_IOC_SET_WRITE_EP, &data_rw)) < 0) {
		DBG_ERR("IOC_WRITE_EP failed.\n");
	}

	return 0;
}

ER usb3dev_waitWriteDone(USB_EP EPn, UINT32 *pDMALen)
{
	int er;
	USB3DEV_DATA_RW data_rw;

	data_rw.epn 		= EPn;
	data_rw.dma_length	= pDMALen;

	if ((er = ioctl(m_fd, USB3DEV_IOC_WAIT_WRITE_EP_DONE, &data_rw)) < 0) {
		DBG_ERR("IOC_WRITE_EP failed.\n");
	}

	return 0;
}

ER usb3dev_SetIsoBuff_HW(USB_EP EPn, BOOL bEnable)
{
	int er;
	USB3DEV_ISO_BUFF_HW buff_hw;

	buff_hw.epn			= EPn;
	buff_hw.enable			= bEnable;

	if ((er = ioctl(m_fd, USB3DEV_IOC_ISO_BUFF_HW, &buff_hw)) < 0) {
		DBG_ERR("IOC_ISO_BUFF_HW failed.\n");
	}

	return 0;
}

ER usb3dev_SetIsoBuff_SW(USB_EP EPn, BOOL bEnable)
{
	int er;
	USB3DEV_ISO_BUFF_SW buff_sw;

	buff_sw.epn                     = EPn;
	buff_sw.enable                  = bEnable;

	if ((er = ioctl(m_fd, USB3DEV_IOC_ISO_BUFF_SW, &buff_sw)) < 0) {
		DBG_ERR("IOC_ISO_BUFF_SW failed.\n");
	}

	return 0;
}
