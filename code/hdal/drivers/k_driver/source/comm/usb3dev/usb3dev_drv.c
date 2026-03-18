#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/slab.h>

//#include "usb3dev_int.h"
#include "usb3dev.h"
#include "kwrap/type.h"
#include "usb3dev_drv.h"
#include "usb3dev_ioctl.h"
#include "usb3dev_dbg.h"
#include "usb3dev_int.h"
#include <linux/dma-mapping.h>

BOOL gUSB3EverSuspend  = FALSE;

/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
int nvt_usb3dev_drv_ioctl(unsigned char uc_if, USB3DEV_MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg);
irqreturn_t nvt_usb3dev_drv_isr(int irq, void *devid);

/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
typedef irqreturn_t (*irq_handler_t)(int, void *);

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/
uintptr_t _USB3DEV_REG_BASE_ADDR[MODULE_REG_NUM];

extern USB3_DEVINFO gU3DevInfo;

static USB3DEV_CX_RET_DATA	ret_data;
static USB3DEV_ISODATA_RW 	isodata_rw_tmp;
static USB3DEV_ISODATA_RW 	isodata_rw[USB_EP_MAX];
static PU3DEV_ISOPKT		pIsoPktBuf[USB_EP_MAX];


static USB_DEVICE_DESC *pusbioc_dev_desc[3];
static UINT8 *pusbioc_cfg_desc[3];
static UINT8 *pusbioc_str_desc_ss[12],*pusbioc_str_desc_hs[12],*pusbioc_str_desc_fs[12];
static UINT8 *pusbioc_bos_desc[2];
static UINT8 *pusbioc_devquali_desc[2];
static UINT8 *pusbioc_otherspd_desc[2];
static UINT8 *usbioc_cx_data_buf = NULL;

static 	USB3DEV_DESC 	desc;


/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/
int nvt_usb3dev_drv_open(PUSB3DEV_MODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	usb3dev_api("%s\n", __func__);

	/* Add HW Moduel initial operation here when the device file opened*/

	return 0;
}


int nvt_usb3dev_drv_release(PUSB3DEV_MODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	usb3dev_api("%s\n", __func__);

	/* Add HW Moduel release operation here when device file closed */

	return 0;
}

int nvt_usb3dev_drv_init(USB3DEV_MODULE_INFO *pmodule_info, struct device *dev)
{
	int ret = 0;
	unsigned char ucloop;

	usb3dev_api("%s\n", __func__);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM; ucloop++) {
		_USB3DEV_REG_BASE_ADDR[ucloop] = (uintptr_t)pmodule_info->io_addr[ucloop];
		usb3dev_api("USB3DEV[%d]-Addr = 0x%08lX\n", ucloop, _USB3DEV_REG_BASE_ADDR[ucloop]);
	}

	if(vbus_poll)
		usb3_vbus_polling();

	ret = usb3_power_on_init();

	/* register IRQ here*/
	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		if (request_irq(pmodule_info->iinterrupt_id[ucloop], nvt_usb3dev_drv_isr, IRQF_TRIGGER_HIGH, "USB_INT", pmodule_info)) {
			nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[ucloop]);
			ret = -ENODEV;
			goto FAIL_FREE_IRQ;
		} else {
			usb3dev_api("irq request ok.\n");
		}
	}
	if (pmodule_info->vbus_gpio_pin > 0)
		usb3_detect_vbusgpio(pmodule_info->vbus_gpio_pin, pmodule_info->vbus_gpio_pol);

	if(ret)
	{
		goto FAIL_FREE_IRQ;
	}

	if(0)
	{
		dma_addr_t		dma=0;
		unsigned char   *va;


	    if (dma_set_coherent_mask(dev, DMA_BIT_MASK(32)))
	    {
	        printk("Bad mask.\n");
	    }

		va = dma_alloc_coherent(dev, U3DEV_MIN_BUFFER_SIZE, &dma, GFP_KERNEL);

		gU3DevInfo.uiBufferAddr 	= (uintptr_t)va;
		//gU3DevInfo.uiBufferAddr_pa 	= (uintptr_t)dma;
		gU3DevInfo.uiBufferSize 	= U3DEV_MIN_BUFFER_SIZE;

		usb3dev_api("driver internal buffer: va=0x%08X pa=0x%08X va2pa=0x%08X\n",(UINT32)va,(UINT32)dma,frm_va2pa((u32) va));
	}

	#if USB3_EMBED_TESTFUNC
	usb3dev_open();
	#endif

	return ret;

FAIL_FREE_IRQ:

	if(ucloop < MODULE_IRQ_NUM)
		free_irq(pmodule_info->iinterrupt_id[ucloop], pmodule_info);

	/* Add error handler here */

	return ret;

}

int nvt_usb3dev_drv_remove(USB3DEV_MODULE_INFO *pmodule_info, struct device *dev)
{
	unsigned char ucloop;

	usb3dev_api("%s\n", __func__);

	/* Add HW Moduel release operation here*/

	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		/* Free IRQ */
		free_irq(pmodule_info->iinterrupt_id[ucloop], pmodule_info);
	}

	usb3_vbus_thread_remove();

	//Check
	//dma_free_coherent(dev, (size_t)gU3DevInfo.uiBufferSize, (void *)gU3DevInfo.uiBufferAddr, (dma_addr_t)gU3DevInfo.uiBufferAddr_pa);

	//usb2dev_uninit();

	return 0;
}

#if 0
int nvt_usb2dev_drv_suspend(USB3DEV_MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	usb3dev_api("%s\n", __func__);

	/* Add suspend operation here*/

	return 0;
}

int nvt_usb2dev_drv_resume(USB3DEV_MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	usb3dev_api("%s\n", __func__);

	/* Add resume operation here*/

	return 0;
}
#endif


#if 1
static USB3DEV_EVENT usbioc_event;

static void usb3dev_ioc_vendor_req_cb(uintptr_t event)
{
	FLGPTN uiFlag = 0;
	PUSB_DEVICE_REQUEST pSETUP = (PUSB_DEVICE_REQUEST)event;

	//usb3dev_api("usb2dev_ioc_event_cb %d\r\n", event);

	// Like the sem lock
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_WAIT_EVT, TWF_ORW|TWF_CLR);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE, TWF_ORW|TWF_CLR);

	// fill event exported here
	usbioc_event.evt_type = USB3DEV_EVT_TYPE_CX_VENDOR;
	usbioc_event.event = pSETUP->b_request;
	memcpy(usbioc_event.device_request, (PUSB_DEVICE_REQUEST)pSETUP, 8);

	set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_SET);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE|FLGUSB3_IST_STOP, TWF_ORW);
}

static void usb3dev_ioc_vendor_done_cb(uintptr_t event)
{
	FLGPTN uiFlag = 0;
	PUSB_DEVICE_REQUEST pSETUP = (PUSB_DEVICE_REQUEST)event;
	//int ret = 0;

	//usb3dev_api("usb2dev_ioc_event_cb %d\r\n", event);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_WAIT_EVT, TWF_ORW|TWF_CLR);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE, TWF_ORW|TWF_CLR);

	if (UT_GET_DIR(pSETUP->bm_request_type) == UT_READ) {
		//IN
		usbioc_event.request_type = UT_READ;
	} else {
		//OUT
		//ret = copy_to_user((void __user *)ret_data.p_data, usbioc_cx_data_buf,  ret_data.ui_length);
		//if (ret != 0)
		//	DBG_ERR("copy_to_user\n");

		usbioc_event.request_type= UT_WRITE;
		usbioc_event.ui_length = ret_data.ui_length;
	}

	// fill event exported here
	usbioc_event.evt_type = USB3DEV_EVT_TYPE_CX_VENDOR_DONE;
	usbioc_event.event = pSETUP->b_request;
	memcpy(usbioc_event.device_request, (PUSB_DEVICE_REQUEST)pSETUP, 8);

	set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_SET);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE|FLGUSB3_IST_STOP, TWF_ORW);
}

static void usb3dev_ioc_class_req_cb(uintptr_t event)
{
	FLGPTN uiFlag = 0;
	PUSB_DEVICE_REQUEST pSETUP = (PUSB_DEVICE_REQUEST)event;

	//usb3dev_api("usb2dev_ioc_event_cb %d\r\n", event);

	// Like the sem lock
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_WAIT_EVT, TWF_ORW|TWF_CLR);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE, TWF_ORW|TWF_CLR);

	// fill event exported here
	usbioc_event.evt_type = USB3DEV_EVT_TYPE_CX_CLASS;
	usbioc_event.event = pSETUP->b_request;
	memcpy(usbioc_event.device_request, (PUSB_DEVICE_REQUEST)pSETUP, 8);

	set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_SET);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE|FLGUSB3_IST_STOP, TWF_ORW);
}

static void usb3dev_ioc_class_done_cb(uintptr_t event)
{
	FLGPTN uiFlag = 0;
	PUSB_DEVICE_REQUEST pSETUP = (PUSB_DEVICE_REQUEST)event;
	//int ret = 0;

	//usb3dev_api("usb2dev_ioc_event_cb %d\r\n", event);

	// Like the sem lock

	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_WAIT_EVT, TWF_ORW|TWF_CLR);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE, TWF_ORW|TWF_CLR);

	if (UT_GET_DIR(pSETUP->bm_request_type)== UT_READ) {
		//IN
		usbioc_event.request_type = UT_READ;
	} else {
		//OUT
		//ret = copy_to_user((void __user *)ret_data.p_data, usbioc_cx_data_buf,  ret_data.ui_length);
		//if (ret != 0)
		//	DBG_ERR("copy_to_user\n");

		usbioc_event.request_type= UT_WRITE;
		usbioc_event.ui_length = ret_data.ui_length;
	}

	// fill event exported here
	usbioc_event.evt_type = USB3DEV_EVT_TYPE_CX_CLASS_DONE;
	usbioc_event.event = pSETUP->b_request;
	memcpy(usbioc_event.device_request, (PUSB_DEVICE_REQUEST)pSETUP, 8);

	set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_SET);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE|FLGUSB3_IST_STOP, TWF_ORW);
}

static void usb3dev_ioc_event_cb(uintptr_t event)
{
	FLGPTN uiFlag = 0;

	//usb3dev_api("usb2dev_ioc_event_cb %d\r\n", event);

	// Like the sem lock
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_WAIT_EVT, TWF_ORW|TWF_CLR);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE, TWF_ORW|TWF_CLR);

	// fill event exported here
	usbioc_event.evt_type = USB3DEV_EVT_TYPE_USBEVENT_CB;
	usbioc_event.event = event;

	set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_SET);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE|FLGUSB3_IST_STOP, TWF_ORW);
}

static void usb3dev_ioc_charging_cb(uintptr_t event)
{
	FLGPTN uiFlag = 0;

	//usb3dev_api("usb2dev_ioc_event_cb %d\r\n", event);

	// Like the sem lock
	//vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_WAIT_EVT, TWF_ORW|TWF_CLR);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE, TWF_ORW|TWF_CLR);

	// fill event exported here
	usbioc_event.evt_type = USB3DEV_EVT_TYPE_CHARGING_CB;
	usbioc_event.event = event;

	set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_SET);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE|FLGUSB3_IST_STOP, TWF_ORW);
}

static void usb3dev_ioc_set_interface_cb(uintptr_t event)
{
	FLGPTN uiFlag = 0;
	//PUSB_DEVICE_REQUEST pSETUP = (PUSB_DEVICE_REQUEST)event;

	//usb3dev_api("usb2dev_ioc_event_cb %d\r\n", event);

	// Like the sem lock
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_WAIT_EVT, TWF_ORW|TWF_CLR);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE, TWF_ORW|TWF_CLR);

	// fill event exported here
	usbioc_event.evt_type = USB3DEV_EVT_TYPE_SET_INTF_CB;
	usbioc_event.event = event;
	//memcpy(usbioc_event.device_request, (PUSB_DEVICE_REQUEST)pSETUP, 8);

	set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_SET);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE|FLGUSB3_IST_STOP, TWF_ORW);
}

void usb3dev_ioc_std_unknown_cb(uintptr_t event)
{
	FLGPTN uiFlag = 0;
	PUSB_DEVICE_REQUEST pSETUP = (PUSB_DEVICE_REQUEST)event;

	//usb3dev_api("usb3dev_ioc_std_unknown_cb %d\r\n", event);

	// Like the sem lock
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_WAIT_EVT, TWF_ORW|TWF_CLR);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE, TWF_ORW|TWF_CLR);

	// fill event exported here
	usbioc_event.evt_type = USB3DEV_EVT_TYPE_STDUNKNOWN_CB;
	usbioc_event.event = pSETUP->b_request;
	memcpy(usbioc_event.device_request, (PUSB_DEVICE_REQUEST)pSETUP, 8);

	set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_SET);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE|FLGUSB3_IST_STOP, TWF_ORW);
}

void usb3dev_ioc_suspend_cb(uintptr_t event)
{
	FLGPTN uiFlag = 0;

	//usb3dev_api("usb3dev_ioc_suspend_cb %d\r\n", event);

	// Like the sem lock
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_WAIT_EVT, TWF_ORW|TWF_CLR);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE, TWF_ORW|TWF_CLR);

	// fill event exported here
	usbioc_event.evt_type = USB3DEV_EVT_TYPE_SUSPEND_CB;
	usbioc_event.event = 0;
	//memcpy(usbioc_event.device_request, (PUSB_DEVICE_REQUEST)pSETUP, 8);

	set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_SET);
	vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE|FLGUSB3_IST_STOP, TWF_ORW);
}
#endif

int nvt_usb3dev_drv_ioctl(unsigned char uc_if, USB3DEV_MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg)
{
	int 			ret = 0;
	UINT32			value;

	//usb3dev_api("%s IF-%d cmd:%x\n", __func__, uc_if, ui_cmd);

	switch (ui_cmd) {
	case USB3DEV_IOC_OPEN: {

		usb3dev_setCallBack(U3DEV_CALLBACK_ID_CX_VENDOR_REQUEST, usb3dev_ioc_vendor_req_cb);
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_CX_VENDOR_REQDONE, usb3dev_ioc_vendor_done_cb);
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_CX_CLASS_REQUEST,  usb3dev_ioc_class_req_cb);
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_CX_CLASS_REQDONE,  usb3dev_ioc_class_done_cb);
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_EVENT_NOTIFY, 	 usb3dev_ioc_event_cb);
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_CHARGING_EVENT, 	 usb3dev_ioc_charging_cb);
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_SET_INTERFACE, 	 usb3dev_ioc_set_interface_cb);
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_STD_UNKNOWN_REQ,   usb3dev_ioc_std_unknown_cb);
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_SUSPEND,   usb3dev_ioc_suspend_cb);
		usb3dev_setCallBack(U3DEV_CALLBACK_ID_U3_ENTRY,   usb3dev_ioc_suspend_cb);

		/* PreSet flag to make using like as sem : move to usb3dev_open*/
		//set_flg(FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE);
		if (usb3dev_open() != E_OK) {
			DBG_ERR("USB3DEV_IOC_OPEN error\r\n");
		}
	} break;
	case USB3DEV_IOC_CLOSE: {
		UINT32 i;

		usb3dev_close();

		for(i=0; i<3; i++) {
			if(pusbioc_dev_desc[i]) {
				kfree(pusbioc_dev_desc[i]);
				pusbioc_dev_desc[i] = NULL;
			}
		}

		for(i=0; i<3; i++) {
			if(pusbioc_cfg_desc[i]) {
				kfree(pusbioc_cfg_desc[i]);
				pusbioc_cfg_desc[i] = NULL;
			}
		}

		for(i=0; i<12; i++) {
			if(pusbioc_str_desc_ss[i]) {
				kfree(pusbioc_str_desc_ss[i]);
				pusbioc_str_desc_ss[i] = NULL;
			}
		}
		for(i=0; i<12; i++) {
			if(pusbioc_str_desc_hs[i]) {
				kfree(pusbioc_str_desc_hs[i]);
				pusbioc_str_desc_hs[i] = NULL;
			}
		}
		for(i=0; i<12; i++) {
			if(pusbioc_str_desc_fs[i]) {
				kfree(pusbioc_str_desc_fs[i]);
				pusbioc_str_desc_fs[i] = NULL;
			}
		}

		for(i=0; i<2; i++) {
			if(pusbioc_bos_desc[i]) {
				kfree(pusbioc_bos_desc[i]);
				pusbioc_bos_desc[i] = NULL;
			}
		}
		for(i=0; i<2; i++) {
			if(pusbioc_devquali_desc[i]) {
				kfree(pusbioc_devquali_desc[i]);
				pusbioc_devquali_desc[i] = NULL;
			}
		}
		for(i=0; i<2; i++) {
			if(pusbioc_otherspd_desc[i]) {
				kfree(pusbioc_otherspd_desc[i]);
				pusbioc_otherspd_desc[i] = NULL;
			}
		}
		if(usbioc_cx_data_buf) {
			kfree(usbioc_cx_data_buf);
			usbioc_cx_data_buf = NULL;
		}

		for(i=0; i<USB_EP_MAX; i++) {
			if(pIsoPktBuf[i]) {
				kfree(pIsoPktBuf[i]);
				pIsoPktBuf[i] = NULL;
			}
		}

	} break;

	case USB3DEV_IOC_SET_SS_DEVICE_DESC:
	case USB3DEV_IOC_SET_HS_DEVICE_DESC:
	case USB3DEV_IOC_SET_FS_DEVICE_DESC:
	{
		UINT32 id = 0;

		if(ui_cmd == USB3DEV_IOC_SET_SS_DEVICE_DESC) {
			id = 0;
		} else if(ui_cmd == USB3DEV_IOC_SET_HS_DEVICE_DESC) {
			id = 1;
		} else if(ui_cmd == USB3DEV_IOC_SET_FS_DEVICE_DESC) {
			id = 2;
		}

		ret = copy_from_user(&desc, (void __user *)ul_arg, sizeof(USB3DEV_DESC));
		if (!ret) {
			usb3dev_api("ss dev desc (%d) 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
				, desc.ui_length, desc.desc_data[0], desc.desc_data[1], desc.desc_data[2], desc.desc_data[3], desc.desc_data[4], desc.desc_data[5], desc.desc_data[6], desc.desc_data[7]);

			if(!pusbioc_dev_desc[id]) {
				pusbioc_dev_desc[id] = kmalloc(32, GFP_KERNEL);
			}

			if (desc.ui_length <=  18) {
				memcpy(pusbioc_dev_desc[id], desc.desc_data, desc.ui_length);
			}

			if(ui_cmd == USB3DEV_IOC_SET_SS_DEVICE_DESC) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_SS_DEVICE, 	(void *)pusbioc_dev_desc[id]);
			} else if(ui_cmd == USB3DEV_IOC_SET_HS_DEVICE_DESC) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_HS_DEVICE, 	(void *)pusbioc_dev_desc[id]);
			} else if(ui_cmd == USB3DEV_IOC_SET_FS_DEVICE_DESC) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_FS_DEVICE, 	(void *)pusbioc_dev_desc[id]);
			}

		}
	} break;


	case USB3DEV_IOC_SET_CFG_SS_DESC:
	case USB3DEV_IOC_SET_CFG_HS_DESC:
	case USB3DEV_IOC_SET_CFG_FS_DESC:
	{
		UINT32 id = 0;

		if(ui_cmd == USB3DEV_IOC_SET_CFG_SS_DESC) {
			id = 0;
		} else if(ui_cmd == USB3DEV_IOC_SET_CFG_HS_DESC) {
			id = 1;
		} else if(ui_cmd == USB3DEV_IOC_SET_CFG_FS_DESC) {
			id = 2;
		}

		ret = copy_from_user(&desc, (void __user *)ul_arg, sizeof(USB3DEV_DESC));
		if (!ret) {
			usb3dev_api("cfg ss desc (%d) 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
				, desc.ui_length, desc.desc_data[0], desc.desc_data[1], desc.desc_data[2], desc.desc_data[3], desc.desc_data[4], desc.desc_data[5], desc.desc_data[6], desc.desc_data[7]);

			if(!pusbioc_cfg_desc[id]) {
				pusbioc_cfg_desc[id] = kmalloc(USB_DESC_MAX_SIZE, GFP_KERNEL);
			}

			if (desc.ui_length <=  USB_DESC_MAX_SIZE)
				memcpy(pusbioc_cfg_desc[id], desc.desc_data, desc.ui_length);

			if(ui_cmd == USB3DEV_IOC_SET_CFG_SS_DESC) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_SS_CONFIG, 	(void *)pusbioc_cfg_desc[id]);
			} else if(ui_cmd == USB3DEV_IOC_SET_CFG_HS_DESC) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_HS_CONFIG, 	(void *)pusbioc_cfg_desc[id]);
			} else if(ui_cmd == USB3DEV_IOC_SET_CFG_FS_DESC) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_FS_CONFIG, 	(void *)pusbioc_cfg_desc[id]);
			}

		}
	} break;


	case USB3DEV_IOC_SET_STRING0_DESC_SS:
	case USB3DEV_IOC_SET_STRING1_DESC_SS:
	case USB3DEV_IOC_SET_STRING2_DESC_SS:
	case USB3DEV_IOC_SET_STRING3_DESC_SS:
	case USB3DEV_IOC_SET_STRING4_DESC_SS:
	case USB3DEV_IOC_SET_STRING5_DESC_SS:
	case USB3DEV_IOC_SET_STRING6_DESC_SS:
	case USB3DEV_IOC_SET_STRING7_DESC_SS:
	case USB3DEV_IOC_SET_STRING8_DESC_SS:
	case USB3DEV_IOC_SET_STRING9_DESC_SS:
	case USB3DEV_IOC_SET_STRING10_DESC_SS:
	case USB3DEV_IOC_SET_STRING11_DESC_SS:
	{
		UINT32 id = 0;

		if(ui_cmd == USB3DEV_IOC_SET_STRING0_DESC_SS) {
			id = 0;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING1_DESC_SS) {
			id = 1;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING2_DESC_SS) {
			id = 2;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING3_DESC_SS) {
			id = 3;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING4_DESC_SS) {
			id = 4;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING5_DESC_SS) {
			id = 5;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING6_DESC_SS) {
			id = 6;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING7_DESC_SS) {
			id = 7;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING8_DESC_SS) {
			id = 8;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING9_DESC_SS) {
			id = 9;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING10_DESC_SS) {
			id = 10;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING11_DESC_SS) {
			id = 11;
		}

		ret = copy_from_user(&desc, (void __user *)ul_arg, sizeof(USB3DEV_DESC));
		if (!ret) {
			usb3dev_api("str desc (%d) 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
				, desc.ui_length, desc.desc_data[0], desc.desc_data[1], desc.desc_data[2], desc.desc_data[3], desc.desc_data[4], desc.desc_data[5], desc.desc_data[6], desc.desc_data[7]);

			if(!pusbioc_str_desc_ss[id]) {
				pusbioc_str_desc_ss[id] = kmalloc(512, GFP_KERNEL);
			}

			if (desc.ui_length <=  USB_DESC_MAX_SIZE)
				memcpy(pusbioc_str_desc_ss[id], desc.desc_data, desc.ui_length);

			usb3dev_setDescriptor(U3DEV_DESC_ID_SS_STRING0+id, 	(void *)pusbioc_str_desc_ss[id]);
		}
	} break;

	case USB3DEV_IOC_SET_STRING0_DESC_HS:
	case USB3DEV_IOC_SET_STRING1_DESC_HS:
	case USB3DEV_IOC_SET_STRING2_DESC_HS:
	case USB3DEV_IOC_SET_STRING3_DESC_HS:
	case USB3DEV_IOC_SET_STRING4_DESC_HS:
	case USB3DEV_IOC_SET_STRING5_DESC_HS:
	case USB3DEV_IOC_SET_STRING6_DESC_HS:
	case USB3DEV_IOC_SET_STRING7_DESC_HS:
	case USB3DEV_IOC_SET_STRING8_DESC_HS:
	case USB3DEV_IOC_SET_STRING9_DESC_HS:
	case USB3DEV_IOC_SET_STRING10_DESC_HS:
	case USB3DEV_IOC_SET_STRING11_DESC_HS:
	{
		UINT32 id = 0;

		if(ui_cmd == USB3DEV_IOC_SET_STRING0_DESC_HS) {
			id = 0;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING1_DESC_HS) {
			id = 1;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING2_DESC_HS) {
			id = 2;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING3_DESC_HS) {
			id = 3;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING4_DESC_HS) {
			id = 4;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING5_DESC_HS) {
			id = 5;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING6_DESC_HS) {
			id = 6;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING7_DESC_HS) {
			id = 7;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING8_DESC_HS) {
			id = 8;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING9_DESC_HS) {
			id = 9;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING10_DESC_HS) {
			id = 10;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING11_DESC_HS) {
			id = 11;
		}

		ret = copy_from_user(&desc, (void __user *)ul_arg, sizeof(USB3DEV_DESC));
		if (!ret) {
			usb3dev_api("str desc (%d) 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
				, desc.ui_length, desc.desc_data[0], desc.desc_data[1], desc.desc_data[2], desc.desc_data[3], desc.desc_data[4], desc.desc_data[5], desc.desc_data[6], desc.desc_data[7]);

			if(!pusbioc_str_desc_hs[id]) {
				pusbioc_str_desc_hs[id] = kmalloc(512, GFP_KERNEL);
			}

			if (desc.ui_length <=  USB_DESC_MAX_SIZE)
				memcpy(pusbioc_str_desc_hs[id], desc.desc_data, desc.ui_length);

			usb3dev_setDescriptor(U3DEV_DESC_ID_HS_STRING0+id, 	(void *)pusbioc_str_desc_hs[id]);
		}
	} break;

	case USB3DEV_IOC_SET_STRING0_DESC_FS:
	case USB3DEV_IOC_SET_STRING1_DESC_FS:
	case USB3DEV_IOC_SET_STRING2_DESC_FS:
	case USB3DEV_IOC_SET_STRING3_DESC_FS:
	case USB3DEV_IOC_SET_STRING4_DESC_FS:
	case USB3DEV_IOC_SET_STRING5_DESC_FS:
	case USB3DEV_IOC_SET_STRING6_DESC_FS:
	case USB3DEV_IOC_SET_STRING7_DESC_FS:
	case USB3DEV_IOC_SET_STRING8_DESC_FS:
	case USB3DEV_IOC_SET_STRING9_DESC_FS:
	case USB3DEV_IOC_SET_STRING10_DESC_FS:
	case USB3DEV_IOC_SET_STRING11_DESC_FS:
	{
		UINT32 id = 0;

		if(ui_cmd == USB3DEV_IOC_SET_STRING0_DESC_FS) {
			id = 0;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING1_DESC_FS) {
			id = 1;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING2_DESC_FS) {
			id = 2;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING3_DESC_FS) {
			id = 3;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING4_DESC_FS) {
			id = 4;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING5_DESC_FS) {
			id = 5;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING6_DESC_FS) {
			id = 6;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING7_DESC_FS) {
			id = 7;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING8_DESC_FS) {
			id = 8;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING9_DESC_FS) {
			id = 9;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING10_DESC_FS) {
			id = 10;
		} else if(ui_cmd == USB3DEV_IOC_SET_STRING11_DESC_FS) {
			id = 11;
		}

		ret = copy_from_user(&desc, (void __user *)ul_arg, sizeof(USB3DEV_DESC));
		if (!ret) {
			usb3dev_api("str desc (%d) 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
				, desc.ui_length, desc.desc_data[0], desc.desc_data[1], desc.desc_data[2], desc.desc_data[3], desc.desc_data[4], desc.desc_data[5], desc.desc_data[6], desc.desc_data[7]);

			if(!pusbioc_str_desc_fs[id]) {
				pusbioc_str_desc_fs[id] = kmalloc(512, GFP_KERNEL);
			}

			if (desc.ui_length <=  USB_DESC_MAX_SIZE)
				memcpy(pusbioc_str_desc_fs[id], desc.desc_data, desc.ui_length);

			usb3dev_setDescriptor(U3DEV_DESC_ID_FS_STRING0+id, 	(void *)pusbioc_str_desc_fs[id]);
		}
	} break;


	case USB3DEV_IOC_SET_BOS_DESC_SS:
	case USB3DEV_IOC_SET_BOS_DESC_HS:
	{
		UINT32 id = 0;

		if(ui_cmd == USB3DEV_IOC_SET_BOS_DESC_SS) {
			id = 0;
		} else if(ui_cmd == USB3DEV_IOC_SET_BOS_DESC_HS) {
			id = 1;
		}

		ret = copy_from_user(&desc, (void __user *)ul_arg, sizeof(USB3DEV_DESC));
		if (!ret) {
			usb3dev_api("bos desc (%d) 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
				, desc.ui_length, desc.desc_data[0], desc.desc_data[1], desc.desc_data[2], desc.desc_data[3], desc.desc_data[4], desc.desc_data[5], desc.desc_data[6], desc.desc_data[7]);

			if(!pusbioc_bos_desc[id]) {
				pusbioc_bos_desc[id] = kmalloc(512, GFP_KERNEL);
			}

			if (desc.ui_length <=  USB_DESC_MAX_SIZE)
				memcpy(pusbioc_bos_desc[id], desc.desc_data, desc.ui_length);

			if(ui_cmd == USB3DEV_IOC_SET_BOS_DESC_SS) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_SS_BOS, 	(void *)pusbioc_bos_desc[id]);
			} else if(ui_cmd == USB3DEV_IOC_SET_BOS_DESC_HS) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_HS_BOS, 	(void *)pusbioc_bos_desc[id]);
			}
		}
	} break;

	case USB3DEV_IOC_SET_DEVQUALI_DESC_HS:
	case USB3DEV_IOC_SET_DEVQUALI_DESC_FS:
	{
		UINT32 id = 0;

		if(ui_cmd == USB3DEV_IOC_SET_DEVQUALI_DESC_HS) {
			id = 0;
		} else if(ui_cmd == USB3DEV_IOC_SET_DEVQUALI_DESC_FS) {
			id = 1;
		}

		ret = copy_from_user(&desc, (void __user *)ul_arg, sizeof(USB3DEV_DESC));
		if (!ret) {
			usb3dev_api("DEVQUALI desc (%d) 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
				, desc.ui_length, desc.desc_data[0], desc.desc_data[1], desc.desc_data[2], desc.desc_data[3], desc.desc_data[4], desc.desc_data[5], desc.desc_data[6], desc.desc_data[7]);

			if(!pusbioc_devquali_desc[id]) {
				pusbioc_devquali_desc[id] = kmalloc(512, GFP_KERNEL);
			}

			if (desc.ui_length <=  USB_DESC_MAX_SIZE)
				memcpy(pusbioc_devquali_desc[id], desc.desc_data, desc.ui_length);

			if(ui_cmd == USB3DEV_IOC_SET_DEVQUALI_DESC_HS) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_HS_DEVICEQUALIFIER, 	(void *)pusbioc_devquali_desc[id]);
			} else if(ui_cmd == USB3DEV_IOC_SET_DEVQUALI_DESC_FS) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_FS_DEVICEQUALIFIER, 	(void *)pusbioc_devquali_desc[id]);
			}
		}
	} break;

	case USB3DEV_IOC_SET_OTHERSPD_DESC_HS:
	case USB3DEV_IOC_SET_OTHERSPD_DESC_FS:
	{
		UINT32 id = 0;

		if(ui_cmd == USB3DEV_IOC_SET_OTHERSPD_DESC_HS) {
			id = 0;
		} else if(ui_cmd == USB3DEV_IOC_SET_OTHERSPD_DESC_FS) {
			id = 1;
		}

		ret = copy_from_user(&desc, (void __user *)ul_arg, sizeof(USB3DEV_DESC));
		if (!ret) {
			usb3dev_api("otherspd desc (%d) 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
				, desc.ui_length, desc.desc_data[0], desc.desc_data[1], desc.desc_data[2], desc.desc_data[3], desc.desc_data[4], desc.desc_data[5], desc.desc_data[6], desc.desc_data[7]);

			if(!pusbioc_otherspd_desc[id]) {
				pusbioc_otherspd_desc[id] = kmalloc(512, GFP_KERNEL);
			}

			if (desc.ui_length <=  USB_DESC_MAX_SIZE)
				memcpy(pusbioc_otherspd_desc[id], desc.desc_data, desc.ui_length);

			if(ui_cmd == USB3DEV_IOC_SET_OTHERSPD_DESC_HS) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_HS_OTHERSPEED, 	(void *)pusbioc_otherspd_desc[id]);
			} else if(ui_cmd == USB3DEV_IOC_SET_OTHERSPD_DESC_FS) {
				usb3dev_setDescriptor(U3DEV_DESC_ID_FS_OTHERSPEED, 	(void *)pusbioc_otherspd_desc[id]);
			}
		}
	} break;


	case USB3DEV_IOC_CX_RET_DATA: {

		ret = copy_from_user(&ret_data, (void __user *)ul_arg, sizeof(USB3DEV_CX_RET_DATA));
		if (!ret) {

			if(usbioc_cx_data_buf) {
				kfree(usbioc_cx_data_buf);
				usbioc_cx_data_buf = NULL;
			}

			if (ret_data.ui_length <= 20480) {
				usbioc_cx_data_buf = (UINT8 *)kmalloc(ret_data.ui_length, GFP_KERNEL);

				if(usbioc_cx_data_buf != NULL) {
					ret = copy_from_user(usbioc_cx_data_buf, (void __user *)ret_data.p_data, ret_data.ui_length);
					if (!ret) {
						usb3dev_handleSetupResult(ret_data.stall, (uintptr_t) usbioc_cx_data_buf, ret_data.ui_length);
					}
				} else {
					DBG_ERR("No mem\n");
				}
			} else {
					DBG_ERR("request larger than 20KB buf\n");
			}

		}

	} break;

	case USB3DEV_IOC_SET_CFG: {
		USB3DEV_SETCFG _cfg;

		ret = copy_from_user(&_cfg, (void __user *)ul_arg, sizeof(USB3DEV_SETCFG));
		if (!ret) {
				usb3dev_setConfig(_cfg.cfgid, _cfg.cfg_value);
		}

	} break;

	case USB3DEV_IOC_GET_DEVINFO: {
		UINT32 InfoID;

		ret = copy_from_user(&InfoID, (void __user *)ul_arg, sizeof(UINT32));
		if (!ret) {
			value = (UINT32) usb3dev_getDevInfo((U3DEV_DEVINFO_ID)InfoID);
			ret = copy_to_user((void __user *)ul_arg, &value, sizeof(UINT32));
			if (ret != 0)
				DBG_ERR("copy_to_user\n");
		}
	} break;

	case USB3DEV_IOC_CHK_EPBUSY: {
		USB3DEV_CHKBUSY chkbusy;

		ret = copy_from_user(&chkbusy, (void __user *)ul_arg, sizeof(USB3DEV_CHKBUSY));
		if (!ret) {
			value = (UINT32) usb3dev_chkEPBusy(chkbusy.epn, chkbusy.ep_dir);
			ret = copy_to_user((void __user *)chkbusy.busy, &value, sizeof(UINT32));
			if (ret != 0)
				DBG_ERR("copy_to_user\n");
		}
	} break;

	case USB3DEV_IOC_GET_SOF: {
		value = usb3dev_getSOF();
		ret = copy_to_user((void __user *)ul_arg, &value, sizeof(UINT32));
		if (ret != 0)
			DBG_ERR("copy_to_user\n");

	} break;

	case USB3DEV_IOC_GET_EVENT: {
		FLGPTN uiFlag;

		ret = copy_from_user(&usbioc_event, (void __user *)ul_arg, sizeof(USB3DEV_EVENT));

		if (ret != 0)
			DBG_ERR("copy_from_user, ret = %d\n", ret);

		set_flg( FLG_ID_USB3, FLGUSB3_WAIT_EVT);

		ret = vos_flag_wait_interruptible(&uiFlag, FLG_ID_USB3, FLGUSB3_USER_EVENT_SET, TWF_ORW|TWF_CLR);
		if (ret) {
			ret = -ERESTARTSYS;
			break;
		}

		if (usbioc_event.evt_type == USB3DEV_EVT_TYPE_CX_CLASS_DONE || usbioc_event.evt_type ==  USB3DEV_EVT_TYPE_CX_VENDOR_DONE){
			ret = copy_to_user((void __user *)ret_data.p_data, usbioc_cx_data_buf,  ret_data.ui_length);
			if (ret != 0)
				DBG_ERR("copy_to_user %d data len = %d\n", ret, usbioc_event.ui_length);
		}

		ret = copy_to_user((void __user *)ul_arg, &usbioc_event, sizeof(USB3DEV_EVENT));
		if (ret != 0)
			DBG_ERR("copy_to_user\n");

	} break;

	case USB3DEV_IOC_SET_EVENT_DONE: {
		set_flg( FLG_ID_USB3, FLGUSB3_USER_EVENT_DONE);
	} break;

	case USB3DEV_IOC_WRITE_EP: {
		USB3DEV_DATA_RW data_rw;
		UINT32 dma_len = 0;

		ret = copy_from_user(&data_rw, (void __user *)ul_arg, sizeof(USB3DEV_DATA_RW));
		if (!ret) {

			ret = copy_from_user(&dma_len, (void __user *)data_rw.dma_length, sizeof(UINT32));

			if (!ret) {
				if (data_rw.epn == USB_EP0) {
					DBG_ERR("usb3dev_writeEndpoint no support ep0\n");
				} else {
					if(data_rw.epn <USB_EP_MAX)
						usb3dev_writeEndpoint(data_rw.epn, data_rw.buffer, &dma_len);
					else
						DBG_ERR("usb3dev_writeEndpoint no support ep%d\n",data_rw.epn);
					//DBG_ERR("usb3dev_writeEndpoint epn  please check driver \n");
				}
				ret = copy_to_user((void __user *)data_rw.dma_length, &dma_len, sizeof(UINT32));
				if (ret != 0)
					DBG_ERR("copy_to_user\n");
			}
		}
	} break;
	case USB3DEV_IOC_READ_EP: {
		USB3DEV_DATA_RW data_rw;
		UINT32 dma_len = 0;

		ret = copy_from_user(&data_rw, (void __user *)ul_arg, sizeof(USB3DEV_DATA_RW));
		if (!ret) {

			ret = copy_from_user(&dma_len, (void __user *)data_rw.dma_length, sizeof(UINT32));

			if (!ret) {

				if (data_rw.epn == USB_EP0) {
					DBG_ERR("usb3dev_readEndpoint no support ep0\n");
				} else {
					usb3dev_readEndpoint(data_rw.epn, data_rw.buffer, &dma_len);
					//DBG_ERR("usb3dev_readEndpoint EPn   ~~ please check driver ...\n");
				}

				ret = copy_to_user((void __user *)data_rw.dma_length, &dma_len, sizeof(UINT32));
				if (ret != 0)
					DBG_ERR("copy_to_user\n");
			}
		}
	} break;

	case USB3DEV_IOC_WRITE_ISOEP: {

		ret = copy_from_user(&isodata_rw_tmp, (void __user *)ul_arg, sizeof(USB3DEV_ISODATA_RW));

		if (!ret) {

			if(isodata_rw_tmp.epn >= USB_EP_MAX) {
				DBG_ERR("isodata_rw_tmp.epn >= USB_EP_MAX\n");
				break;
			}

			if(&isodata_rw_tmp.epn != NULL)
				memcpy(&isodata_rw[isodata_rw_tmp.epn], &isodata_rw_tmp, sizeof(USB3DEV_ISODATA_RW));

			if(pIsoPktBuf[isodata_rw_tmp.epn]) {
				kfree(pIsoPktBuf[isodata_rw_tmp.epn]);
				pIsoPktBuf[isodata_rw_tmp.epn] = NULL;
			}

			if((isodata_rw[isodata_rw_tmp.epn].IsoData.uiPacketNumber >0) &&
				(isodata_rw[isodata_rw_tmp.epn].IsoData.uiPacketNumber <= 48)) {
				pIsoPktBuf[isodata_rw_tmp.epn] = kmalloc(sizeof(U3DEV_ISOPKT)*isodata_rw[isodata_rw_tmp.epn].IsoData.uiPacketNumber, GFP_KERNEL);

				ret = copy_from_user(pIsoPktBuf[isodata_rw_tmp.epn], (void __user *)isodata_rw[isodata_rw_tmp.epn].IsoData.pIsoPacket, sizeof(U3DEV_ISOPKT)*isodata_rw[isodata_rw_tmp.epn].IsoData.uiPacketNumber);

				isodata_rw[isodata_rw_tmp.epn].IsoData.pIsoPacket = pIsoPktBuf[isodata_rw_tmp.epn];

				if (!ret) {
					if (isodata_rw[isodata_rw_tmp.epn].epn == USB_EP0) {
						DBG_ERR("usb3dev_writeEndpoint no support ep0\n");
					} else {
						//usb3dev_setIsoEPWrite_pa(isodata_rw[isodata_rw_tmp.epn].epn, &isodata_rw[isodata_rw_tmp.epn].IsoData);
						DBG_ERR("usb3dev_writeEndpoint EPn  please check driver\n");
					}

					/* Non Blocking api no return size results */
					//ret = copy_to_user((void __user *)data_rw.dma_length, &dma_len, sizeof(UINT32));
					//if (ret != 0)
					//	DBG_ERR("copy_to_user\n");
				}
			}
		}
	} break;

	case USB3DEV_IOC_ABORT_EP: {
		USB3DEV_DATA_RW data_rw;

		ret = copy_from_user(&data_rw, (void __user *)ul_arg, sizeof(USB3DEV_DATA_RW));
		if (!ret) {
			if(data_rw.epn >= USB_EP_MAX) {
				DBG_ERR("epn >= USB_EP_MAX\n");
				break;
			}
			usb3dev_abortEndpoint(data_rw.epn);
		}
	} break;

	case USB3DEV_IOC_SET_EP_CFG: {
		USB3DEV_SETEPCFG _cfg;

		ret = copy_from_user(&_cfg, (void __user *)ul_arg, sizeof(USB3DEV_SETEPCFG));
		if (!ret) {
			if(_cfg.epn >= USB_EP_MAX) {
				DBG_ERR("epn >= USB_EP_MAX\n");
				break;
			}
				usb3dev_setEPConfig(_cfg.epn, _cfg.epcfgid, _cfg.cfg_value);
		}

	} break;

	case USB3DEV_IOC_SET_INT: {
		USB3DEV_SET_INT _int;

		ret = copy_from_user(&_int, (void __user *)ul_arg, sizeof(USB3DEV_SET_INT));
		if (!ret) {
			usb3dev_setInterruptEnable(_int.intrr, _int.enable);
		}

	} break;

	case USB3DEV_IOC_GET_BYTECNT: {
		ret = copy_from_user(&value, (void __user *)ul_arg, sizeof(UINT32));
		if (!ret) {
			value =  usb3dev_getEpInfo((USB_EP) value, 0, U3DEV_EPINFO_ID_BYTECOUNT);

			ret = copy_to_user((void __user *)ul_arg, &value, sizeof(UINT32));
			if (ret != 0)
				DBG_ERR("copy_to_user\n");
		}
	} break;

	case USB3DEV_IOC_STALL_EP: {
		ret = copy_from_user(&value, (void __user *)ul_arg, sizeof(UINT32));
		if (!ret) {
			usb3dev_setEPinStall((USB_EP) value);
		}

	} break;

	case USB3DEV_IOC_SET_TX0: {
		ret = copy_from_user(&value, (void __user *)ul_arg, sizeof(UINT32));
		if (!ret) {
			usb3dev_set_tx0byte((USB_EP) value);
		}

	} break;

	case USB3DEV_IOC_WRITE_EP_TMOT: {
		USB3DEV_DATA_RW data_rw;
		UINT32 dma_len = 0;

		ret = copy_from_user(&data_rw, (void __user *)ul_arg, sizeof(USB3DEV_DATA_RW));
		if (!ret) {

			ret = copy_from_user(&dma_len, (void __user *)data_rw.dma_length, sizeof(UINT32));

			if (!ret) {
				if (data_rw.epn == USB_EP0) {
					DBG_ERR("usb3dev_writeEndpoint no support ep0\n");
				} else {
					if(data_rw.epn >= USB_EP_MAX) {
						DBG_ERR("epn >= USB_EP_MAX\n");
						break;
					}
					usb3dev_writeEndpoint_timeout(data_rw.epn, data_rw.buffer, &dma_len, data_rw.timeout_ms);
					//DBG_ERR("usb3dev_writeEndpoint epn  please check driver \n");
				}
				ret = copy_to_user((void __user *)data_rw.dma_length, &dma_len, sizeof(UINT32));
				if (ret != 0)
					DBG_ERR("copy_to_user\n");
			}
		}
	} break;

	case USB3DEV_IOC_RESET_EP_CFG: {
		USB3DEV_RESETEP	_cfg;

		ret = copy_from_user(&_cfg, (void __user *)ul_arg, sizeof(USB3DEV_RESETEP));
		if (!ret) {
			if(_cfg.epn >= USB_EP_MAX) {
				DBG_ERR("epn >= USB_EP_MAX\n");
				break;
			}
			usb3_reset_ep_cfg(_cfg.epn, _cfg.EpType, _cfg.MaxPktSz, _cfg.MaxBurst, _cfg.bInterval, _cfg.isIN, _cfg.BwNum);
		}
	} break;

	case USB3DEV_IOC_CHK_CHARGER: {
		ret = copy_from_user(&value, (void __user *)ul_arg, sizeof(UINT32));
		if (!ret) {
			value = usb3dev_check_charger(value);
			ret = copy_to_user((void __user *)ul_arg, &value, sizeof(UINT32));
			if (ret != 0)
				DBG_ERR("copy_to_user\n");

		}
	} break;

	case USB3DEV_IOC_STATE_CHG: {
		value = usb3dev_StateChange();
		ret = copy_to_user((void __user *)ul_arg, &value, sizeof(UINT32));
		if (ret != 0)
			DBG_ERR("copy_to_user\n");

	} break;

	case USB3DEV_IOC_SET_WRITE_EP: {
		USB3DEV_DATA_RW data_rw;
		UINT32 dma_len = 0;

		ret = copy_from_user(&data_rw, (void __user *)ul_arg, sizeof(USB3DEV_DATA_RW));
		if (!ret) {

			ret = copy_from_user(&dma_len, (void __user *)data_rw.dma_length, sizeof(UINT32));

			if (!ret) {
				if (data_rw.epn == USB_EP0) {
					DBG_ERR("usb3dev_writeEndpoint no support ep0\n");
				} else {

					if(data_rw.epn <USB_EP_MAX)
						ret = usb3dev_setEPWrite(data_rw.epn, data_rw.buffer, &dma_len);
					else
						DBG_ERR("usb3dev_writeEndpoint no support ep%d\n",data_rw.epn);
					//DBG_ERR("usb3dev_writeEndpoint epn  please check driver \n");
				}
			}
		}
	} break;

	case USB3DEV_IOC_WAIT_WRITE_EP_DONE: {
		USB3DEV_DATA_RW data_rw;
		UINT32 dma_len = 0;

		ret = copy_from_user(&data_rw, (void __user *)ul_arg, sizeof(USB3DEV_DATA_RW));
		if (!ret) {

			ret = copy_from_user(&dma_len, (void __user *)data_rw.dma_length, sizeof(UINT32));

			if (!ret) {
				if (data_rw.epn == USB_EP0) {
					DBG_ERR("usb3dev_waitWriteDone no support ep0\n");
				} else {

					if(data_rw.epn <USB_EP_MAX) {
						usb3dev_waitWriteDone(data_rw.epn, &dma_len);

						ret = copy_to_user((void __user *)data_rw.dma_length, &dma_len, sizeof(UINT32));
						if (ret != 0)
							DBG_ERR("copy_to_user\n");
					} else {
						DBG_ERR("usb3dev_waitWriteDone no support ep%d\n",data_rw.epn);
						//DBG_ERR("usb3dev_writeEndpoint epn  please check driver \n");
					}

				}
			}
		}
	} break;

	case USB3DEV_IOC_ISO_BUFF_HW: {
		USB3DEV_ISO_BUFF_HW buf_hw;

		ret = copy_from_user(&buf_hw, (void __user *)ul_arg, sizeof(USB3DEV_ISO_BUFF_HW));
		if (!ret) {
			usb3dev_SetIsoBuff_HW(buf_hw.epn, buf_hw.enable);
		}

	} break;

	case USB3DEV_IOC_ISO_BUFF_SW: {
		USB3DEV_ISO_BUFF_SW buf_sw;

		ret = copy_from_user(&buf_sw, (void __user *)ul_arg, sizeof(USB3DEV_ISO_BUFF_SW));
		if (!ret) {
			usb3dev_SetIsoBuff_SW(buf_sw.epn, buf_sw.enable);
		}

	} break;

	default:
		break;
		/* Add other operations here */
	}

	return ret;
}


void nvt_usb3dev_phy_reset(void)
{
	struct clk *clk = NULL;

	//u3phy
	{
		clk = clk_get(NULL, "u3phy");
		if (!IS_ERR(clk)) {
			printk("%s: u3phy reset\n", __func__);
			clk_prepare_enable(clk);
			clk_disable(clk);
			clk_enable(clk);
		} else
			printk("%s: u3phy not found\n", __func__);
	}

	//u3glue
	{
		clk = clk_get(NULL, "u3glue");
		if (!IS_ERR(clk)) {
			printk("%s: u3glue reset\n", __func__);
			clk_prepare_enable(clk);
			clk_disable(clk);
			clk_enable(clk);
		} else
			printk("%s: u3glue not found\n", __func__);
	}

	//u3hi
	{
		clk = clk_get(NULL, "u3hi");
		if (!IS_ERR(clk)) {
			printk("%s: u3hi reset\n", __func__);
			clk_prepare_enable(clk);
			clk_disable(clk);
			clk_enable(clk);
		} else
			printk("%s: u3hi not found\n", __func__);
	}

}

void nvt_usb3dev_u2phy_reset(void)
{
	struct clk *clk = NULL;

	clk = clk_get(NULL, "u2phy");
	if (!IS_ERR(clk)) {
		printk("%s: u2phy reset\n", __func__);
		clk_prepare_enable(clk);
		clk_disable(clk);
		clk_enable(clk);
	} else
		pr_err("%s: clk u2phy not found\n", __func__);
}


void nvt_usb3dev_check_ssc(void)
{
	struct clk *clk = NULL;

	clk = clk_get(NULL, "pll20_ss");
	if (!IS_ERR(clk)) {
	       clk_set_phase(clk, 0x6);
	} else {
		pr_err("%s: pll20ss not found\n", __func__);
	}
}

#ifdef CONFIG_PM
void usb3dev_suspend(void)
{
	//unsigned long      flags;
	int ret = 0;
	//T_USB_DEVMAINCTRL_REG       devMainCtl;

	if (gU3DevInfo.Opened) {
		gUSB3EverSuspend = 1;
		ret = usb3dev_close();

		// disable global interrupt
		//devMainCtl.reg = USB_GETREG(USB_DEVMAINCTRL_REG_OFS);
		//devMainCtl.bit.GLINT_EN = 0;
		//USB_SETREG(USB_DEVMAINCTRL_REG_OFS, devMainCtl.reg);
		usb3_set_global_interrupt(DISABLE);

		// disable vbus
		//usb2dev_set_vbusi(0);
		//usb3_set_vbus_debounce(0x3FF);
	} else {
		//printk("[Suspend] !!! drv not open\n");
	}

}

void usb3dev_resume(void)
{
	if (gUSB3EverSuspend) {
		usb3dev_open();
		gUSB3EverSuspend = 0;
	} else {
		//printk("[Resume] !!! drv not open\n");
	}
}

int nvt_usb3dev_drv_suspend(USB3DEV_MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	usb3dev_api("%s\n", __func__);

	// stop usb controller
	usb3dev_suspend();

	//nvt_usb2dev_drv_remove(pmodule_info);
	return 0;
}

int nvt_usb3dev_drv_resume(USB3DEV_MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	usb3dev_api("%s\n", __func__);

	// restart usb controller
	usb3dev_resume();

	//nvt_usb2dev_drv_init(pmodule_info);
	return 0;
}
#endif

extern irqreturn_t usb3dev_isr(int irq, void *devid);

irqreturn_t nvt_usb3dev_drv_isr(int irq, void *devid)
{
	return usb3dev_isr(irq, devid);
}

#define DET_USB_REMOVED            0
#define DET_USB_INSERTED           1
#define DET_USB_UNKNOWN           0xFFFFFFFF

struct task_struct *p_mon;
static volatile UINT32  uiDETUSBStatus  = DET_USB_UNKNOWN;


void Detect_usb(void)
{
	static UINT32	uiDETUSBPrevSts = DET_USB_UNKNOWN;
	UINT32	uiStrgCardCurSts;

	if (usb3dev_gpio_getIntStatus() == TRUE){
		uiStrgCardCurSts = DET_USB_INSERTED;
		//printk("CurSts  Insert!\r\n");
	} else {
		uiStrgCardCurSts = DET_USB_REMOVED;
		//printk("CurSts  remove!\r\n");
	}

	// Debounce
	if ((uiStrgCardCurSts == uiDETUSBPrevSts) &&	(uiStrgCardCurSts != uiDETUSBStatus))	{
		if (uiStrgCardCurSts == DET_USB_INSERTED)
		{
			printk("USB inserted!\r\n");
			if ((usb3_get_vbus_debounce()&0x3FF) != 0x3FF) {
				//printk("vbus not gatting\r\n");
				usb3_set_vbus_valid();
			} else {
				//printk("vbus still gatting, do nothing\r\n");
			}
		} else {
			printk("USB removed!\r\n");
		}
		uiDETUSBStatus = uiStrgCardCurSts;
	}
	uiDETUSBPrevSts = uiStrgCardCurSts;
}

static int monitor_daemon_port(void *data)
{
	//struct sched_param param = { .sched_priority = 99 };

	//sched_setscheduler(current, SCHED_FIFO, &param);
	sched_set_fifo(current);
	current->flags |= PF_NOFREEZE;

	set_current_state(TASK_INTERRUPTIBLE);
	while (!kthread_should_stop()) {
		Detect_usb();
		schedule_timeout_interruptible(msecs_to_jiffies(200));
	}
	set_current_state(TASK_RUNNING);
	return 0;
}

void usb3_vbus_polling(void)
{
	if (vbus_poll) {
		p_mon = kthread_run(monitor_daemon_port, NULL, "vbus_check");
		if (IS_ERR(p_mon)) {
			printk("create monitor_daemon_port failed!\n");
		} else {
			printk("create monitor_daemon_port OK!\n");
		}
	}
}

void usb3_vbus_thread_remove(void)
{
	if (vbus_poll) {
		if(p_mon)
		{
			kthread_stop(p_mon);
		}
	}
}

