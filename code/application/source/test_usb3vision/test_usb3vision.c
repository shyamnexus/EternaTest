/**
	@brief Sample code of video record.\n

	@file VIDEO_STREAM.c

	@author Boyan Huang

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hd_debug.h"
#include "vendor_videoenc.h"
#include "vendor_audiocapture.h"
#include "vendor_common.h"
#include "vendor_videoprocess.h"
#include "vendor_videocapture.h"
#include "sys_mempool.h"
#include "usb3vision.h"
#include <kwrap/examsys.h>

// platform dependent
#if 1//defined(__LINUX)
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#define msleep(x)				usleep(1000*x)
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_record, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define POOL_SIZE_USER_DEFINIED  0x1000000

#define USB_BUF_SIZE (0x100000)
static uintptr_t usb_buf_va = 0;
static uintptr_t usb_buf_pa = 0;

#include "usb_desc.h"
#include "usb_define.h"

/**
    Supported CDC PSTN request codes.

    This definition is used for CDC_PSTN_REQUEST_CB.
*/
typedef enum _CDC_PSTN_REQUEST {
	REQ_SET_LINE_CODING         =    0x20,
	REQ_GET_LINE_CODING         =    0x21,
	REQ_SET_CONTROL_LINE_STATE  =    0x22,
	REQ_SEND_BREAK              =    0x23,
	ENUM_DUMMY4WORD(CDC_PSTN_REQUEST)
} CDC_PSTN_REQUEST;

/**
     Line coding structure.
*/
typedef _PACKED_BEGIN struct {
	UINT32   uiBaudRateBPS; ///< Data terminal rate, in bits per second.
	UINT8    uiCharFormat;  ///< Stop bits.
	UINT8    uiParityType;  ///< Parity.
	UINT8    uiDataBits;    ///< Data bits (5, 6, 7, 8 or 16).
} _PACKED_END CDCLineCoding;

/**
    Callback function prototype for PSTN subclass request.
*/
typedef BOOL (*CDC_PSTN_REQUEST_CB)(CDC_PSTN_REQUEST Code, UINT8 *pData, UINT32 *pDataLen);

#define USB_FS_CDC_CFGA_INTF0_ALT0_EP1_MAX_PACKET_SIZE        64
#define USB_FS_CDC_CFGA_INTF0_ALT0_EP2_MAX_PACKET_SIZE        64
#define USB_FS_CDC_CFGA_INTF0_ALT0_EP3_MAX_PACKET_SIZE        64

#define USB_HS_CDC_CFGA_INTF0_ALT0_EP1_MAX_PACKET_SIZE        0x0200
#define USB_HS_CDC_CFGA_INTF0_ALT0_EP2_MAX_PACKET_SIZE        0x0200
#define USB_SS_CDC_CFGA_INTF0_ALT0_EP1_MAX_PACKET_SIZE        0x0400
#define USB_SS_CDC_CFGA_INTF0_ALT0_EP2_MAX_PACKET_SIZE        0x0400
#define USB_HS_CDC_CFGA_INTF0_ALT0_EP3_MAX_PACKET_SIZE        64

#define USB_EP_CDC_INTERRUPT_FSINTERVAL      0xFF   //255ms
#define USB_EP_CDC_INTERRUPT_HSINTERVAL      0xFF   //2**(255-1)*125us=125ms


/* number of string descriptor */
#define USB_CDC_STR_NUM_STRING             4

#define USB_CDC_IF_CFGA_IF0_NUMBER_EP      1
#define USB_CDC_IF_IF0_NUMBER              0
#define USB_CDC_CFG_A_IF0_ALT_MAX          1

#define USB_CDC_IF_CFGA_IF1_NUMBER_EP      2
#define USB_CDC_IF_IF1_NUMBER              1
#define USB_CDC_CFG_A_IF1_ALT_MAX          1


/* device descriptor */
#define USB_CDC_DEV_RELEASE_NUMBER         0x0200         /* spec. release number             */
#define USB_CDC_SS_DEV_RELEASE_NUMBER      0x0310         /* spec. release number             */
#define USB_CDC_DEV_CLASS                  2              /* class code                       */
#define USB_CDC_DEV_SUBCLASS               0              /* sub class code                   */
#define USB_CDC_DEV_PROTOCOL               0              /* protocol code                    */
#define USB_CDC_DEV_MAX_PACKET_SIZE0       0x40           /* max packet size for endpoint 0   */
#define USB_CDC_VENDOR_ID                  0x0D64         /* vendor id                        */
#define USB_CDC_PRODUCT_ID                 0x8611         /* product id                       */
#define USB_CDC_DEV_RELASE_NUMBER          0x0100         /* device release number            */
#define USB_CDC_DEV_NUM_CONFIG             1              /* number of possible configuration */

/*
    configuration descriptor
*/
#define USB_CDC_CFGA_TOTAL_LENGTH          (USB_CFG_LENGTH + (USB_IF_LENGTH * 2) + (USB_BULK_EP_LENGTH * 3)+ 14)
#define USB_CDC_SS_CFGA_TOTAL_LENGTH          (USB_CFG_LENGTH + (USB_IF_LENGTH * 2) + ((USB_BULK_EP_LENGTH+USB_SSEP_COMPANISON_LENGTH) * 3)+ 14)
/* total length                     */
#define USB_CDC_CFGA_NUMBER_IF             2                   /* number of interfaces             */
#define USB_CDC_CFGA_CFG_VALUE             1                   /* configuration value              */
#define USB_CDC_CFGA_CFG_IDX               0                   /* configuration string id          */

#define USB_CDC_CFGA_CFG_ATTRIBUES_IDX     0x07                /* Attribute index in configuration */
#define USB_CDC_CFGA_CFG_ATTRIBUES_SELF    0xC0                /* characteristics: Self Powered    */
#define USB_CDC_CFGA_CFG_ATTRIBUES_BUS     0x80                /* characteristics: Bus Powered     */

#define USB_CDC_CFGA_MAX_POWER_IDX         0x08                /* maximum power index in configuration*/
#define USB_CDC_CFGA_MAX_POWER_SELF        0x32                /* 100mA maximum power. Unit in 2mA */
#define USB_CDC_CFGA_MAX_POWER_BUS         0xFA                /* 500mA maximum power. Unit in 2mA */


/*
    Interface Descriptor
*/

/* Communacation Class Interface */
/*---- alternate setting */
#define USB_CDC_IF_ALT0                    0
/* number of endpoint */
/* bInterfaceClass */
#define USB_CDC_IF_CLASS                   2
#define USB_CDC_IF_DATA_CLASS            0x0A
/* bInterfaceSubClass */
#define USB_CDC_IF_SUBCLASS_NONE           0
#define USB_CDC_IF_SUBCLASS_ACM            2
/* bInterfaceProtocol */
#define USB_CDC_IF_PROTOCOL_NONE           0
#define USB_CDC_IF_PROTOCOL_AT_CMD         1

#define USB_CDC_IF_IDX                     0



/*
    Functional Descriptor
*/
#define USB_CS_DESCRIPTOR_TYPE                  0x24  /**< Indicates that the descriptor is a class specific interface descriptor. */
//Functional header
#define USB_CDC_FUNCTIONAL_HEADER_LENGTH        5
#define USB_CS_SUBTYPE_HEADER                   0x00  /**< CDC class-specific Header functional descriptor. */
#define USB_CDC_SPEC_VER_MAJOR                  1
#define USB_CDC_SPEC_VER_MINOR                  0x10
//Functional ACM
#define USB_CDC_FUNCTIONAL_ACM_LENGTH           4
#define USB_CS_SUBTYPE_ACM                      0x02  /**< CDC class-specific Abstract Control Model functional descriptor. */
#define USB_CDC_ACM_CAPABILITIES                0x06
//Functional Union
#define USB_CDC_FUNCTIONAL_UNION_LENGTH         5
#define USB_CS_SUBTYPE_UNION                    0x06  /**< CDC class-specific Union functional descriptor. */
#define USB_CDC_MASTER_IF_NUM                   0
#define USB_CDC_SLAVE_IF_NUM                    1

#define USB_BOS_LENGTH                 5
#define USB_SS_STRG_BOS_TOTAL_LENGTH        22
#define USB_HS_STRG_BOS_TOTAL_LENGTH        12
#define USB_SS_STRG_BOS_DEVCAP_NUMBER       2
#define USB_HS_STRG_BOS_DEVCAP_NUMBER       1

/*---- string descriptor index0 is LANGID ----*/
static _ALIGNED(64) const UINT8   USBCDCStrDesc0[] = {
	4,                                  /* size of String Descriptor        */
	USB_STRING_DESCRIPTOR_TYPE,         /* String Descriptor type           */
	0x09, 0x04                          /*  Primary/Sub LANGID              */
};

/*---- USB Device Manufacturer string descriptor ----*/
static _ALIGNED(64) const UINT16 USBCDCManuStrDesc[] = {
	0x0320,                                // 20: size of size of String Descriptor = 32 bytes    // 10: size of String Descriptor = 16 bytes
	// 03: String Descriptor type
	'N', 'O', 'V', 'A', 'T', 'E', 'K', '0', '0', '0', '0', '0', '0', '0', '0',     // NOVATEK
};

/*---- USB Device Product string descriptor ----*/
static _ALIGNED(64) const UINT16 USBCDCProdStrDesc[] = {
	0x0336,                                // 36: size of String Descriptor = 54 bytes
	// 03: String Descriptor type
	'N', 'O', 'V', 'A', 'T', 'E',          // 96xxx Digital Still Camera
	'K', ' ', '9', '6', 'x', 'x',
	'x', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' '

};

/*---- USB Device SerialNumber string descriptor ----*/
static _ALIGNED(64) const UINT16 USBCDCStrDesc3[] = {
	0x0320,                                // 20: size of String Descriptor = 32 bytes
	// 03: String Descriptor type
	'9', '6', 'x', 'x', 'x',               // 96xxx-00000-001
	'0', '0', '0', '0', '0',
	'0', '0', '1', '0', '0'
};

static _ALIGNED(64) USB_DEVICE_DESC gUSBSSCDCDevDesc = {
	//---- device descriptor ----
	USB_DEV_LENGTH,                 /* descriptor size                  */
	USB_DEVICE_DESCRIPTOR_TYPE,     /* descriptor type                  */
	USB_CDC_SS_DEV_RELEASE_NUMBER,  /* spec. release number             */
	USB_CDC_DEV_CLASS,             /* class code                       */
	USB_CDC_DEV_SUBCLASS,          /* sub class code                   */
	USB_CDC_DEV_PROTOCOL,          /* protocol code                    */
	USB_CDC_DEV_MAX_PACKET_SIZE0,  /* max packet size for endpoint 0   */
	USB_CDC_VENDOR_ID,             /* vendor id                        */
	USB_CDC_PRODUCT_ID,            /* product id                       */
	USB_CDC_DEV_RELASE_NUMBER,     /* device release number            */
	1,                              /* manifacturer string id           */
	2,                              /* product string id                */
	3,                              /* serial number string id          */
	USB_CDC_DEV_NUM_CONFIG         /* number of possible configuration */
};

static _ALIGNED(64) UINT8 gUSBSSCDCConfigDesc[USB_CDC_SS_CFGA_TOTAL_LENGTH] = {
	//---- configuration(cfg-A) -------
	USB_CFG_LENGTH,                     /* descriptor size                  */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,  /* descriptor type                  */
	USB_CDC_SS_CFGA_TOTAL_LENGTH & 0xff,  /* total length                     */
	USB_CDC_SS_CFGA_TOTAL_LENGTH >> 8,    /* total length                     */
	USB_CDC_CFGA_NUMBER_IF,            /* number of interface              */
	USB_CDC_CFGA_CFG_VALUE,            /* configuration value              */
	USB_CDC_CFGA_CFG_IDX,              /* configuration string id          */
	USB_CDC_CFGA_CFG_ATTRIBUES_SELF,   /* characteristics                  */
	USB_CDC_CFGA_MAX_POWER_SELF,       /* maximum power in 2mA             */

	//----- Communacation Class interface0 ----
	USB_IF_LENGTH,                      /* descriptor size      */
	USB_INTERFACE_DESCRIPTOR_TYPE,      /* descriptor type      */
	USB_CDC_IF_IF0_NUMBER,             /* interface number     */
	USB_CDC_IF_ALT0,                   /* alternate setting    */
	USB_CDC_IF_CFGA_IF0_NUMBER_EP,     /* number of endpoint   */
	USB_CDC_IF_CLASS,                  /* interface class      */
	USB_CDC_IF_SUBCLASS_ACM,           /* interface sub-class  */
	USB_CDC_IF_PROTOCOL_AT_CMD,        /* interface protocol   */
	USB_CDC_IF_IDX,                    /* interface string id  */

	//----- Functional Header ----
	USB_CDC_FUNCTIONAL_HEADER_LENGTH,  /* descriptor size      */
	USB_CS_DESCRIPTOR_TYPE,            /* descriptor type      */
	USB_CS_SUBTYPE_HEADER,             /* sub type             */
	USB_CDC_SPEC_VER_MINOR,            /* ver of the CDC SPEC  */
	USB_CDC_SPEC_VER_MAJOR,            /* ver of the CDC SPEC  */

	//----- Functional ACM ----
	USB_CDC_FUNCTIONAL_ACM_LENGTH,     /* descriptor size      */
	USB_CS_DESCRIPTOR_TYPE,            /* descriptor type      */
	USB_CS_SUBTYPE_ACM,                /* sub type             */
	USB_CDC_ACM_CAPABILITIES,          /* capabilities         */

	//----- Functional Union ----
	USB_CDC_FUNCTIONAL_UNION_LENGTH,   /* descriptor size      */
	USB_CS_DESCRIPTOR_TYPE,            /* descriptor type      */
	USB_CS_SUBTYPE_UNION,              /* sub type             */
	USB_CDC_MASTER_IF_NUM,             /* master IF num        */
	USB_CDC_SLAVE_IF_NUM,              /* slave IF num         */

	//------Notification Endpoint
	//---- EP3 IN ----
	USB_BULK_EP_LENGTH,                                     /* descriptor size      */
	USB_ENDPOINT_DESCRIPTOR_TYPE,                           /* descriptor type      */
	USB_EP_EP3I_ADDRESS,                                    /* endpoint address     */
	USB_EP_ATR_INTERRUPT,                                   /* character address    */
	USB_HS_CDC_CFGA_INTF0_ALT0_EP3_MAX_PACKET_SIZE & 0xff,  /* max packet size      */
	USB_HS_CDC_CFGA_INTF0_ALT0_EP3_MAX_PACKET_SIZE >> 8,    /* max packet size      */
	1,                       /*USB_EP_SIDC_INTERRUPT_INTERVAL*/


	USB_SSEP_COMPANISON_LENGTH,
	USB_SS_EP_COMPANION_TYPE,
	0,
	0,
	1 & 0xff,
	1 >> 8,

	//----- Data Class interface1 ----
	USB_IF_LENGTH,                      /* descriptor size      */
	USB_INTERFACE_DESCRIPTOR_TYPE,      /* descriptor type      */
	USB_CDC_IF_IF1_NUMBER,             /* interface number     */
	USB_CDC_IF_ALT0,                   /* alternate setting    */
	USB_CDC_IF_CFGA_IF1_NUMBER_EP,     /* number of endpoint   */
	USB_CDC_IF_DATA_CLASS,             /* interface class      */
	USB_CDC_IF_SUBCLASS_NONE,           /* interface sub-class  */
	USB_CDC_IF_PROTOCOL_NONE,           /* interface protocol   */
	USB_CDC_IF_IDX,                     /* interface string id  */

	//---- EP1 IN ----
	USB_BULK_EP_LENGTH,                 /* descriptor size      */
	USB_ENDPOINT_DESCRIPTOR_TYPE,       /* descriptor type      */
	USB_EP_EP1I_ADDRESS,                /* endpoint address     */
	USB_EP_ATR_BULK,                    /* character address    */
	USB_SS_CDC_CFGA_INTF0_ALT0_EP1_MAX_PACKET_SIZE & 0xff,  /* max packet size      */
	USB_SS_CDC_CFGA_INTF0_ALT0_EP1_MAX_PACKET_SIZE >> 8,    /* max packet size      */
	USB_EP_BULK_INTERVAL,

	USB_SSEP_COMPANISON_LENGTH,
	USB_SS_EP_COMPANION_TYPE,
	0,
	0,
	USB_EP_BULK_INTERVAL & 0xff,
	USB_EP_BULK_INTERVAL >> 8,

	//---- EP2 OUT ----
	USB_BULK_EP_LENGTH,                 /* descriptor size      */
	USB_ENDPOINT_DESCRIPTOR_TYPE,       /* descriptor type      */
	USB_EP_EP2O_ADDRESS,                /* endpoint address     */
	USB_EP_ATR_BULK,                    /* character address    */
	USB_SS_CDC_CFGA_INTF0_ALT0_EP2_MAX_PACKET_SIZE & 0xff,  /* max packet size      */
	USB_SS_CDC_CFGA_INTF0_ALT0_EP2_MAX_PACKET_SIZE >> 8,    /* max packet size      */
	USB_EP_BULK_INTERVAL,

	USB_SSEP_COMPANISON_LENGTH,
	USB_SS_EP_COMPANION_TYPE,
	0,
	0,
	USB_EP_BULK_INTERVAL & 0xff,
	USB_EP_BULK_INTERVAL >> 8
};

static _ALIGNED(64) const UINT8 gCDCSSBosDesc[] = {
	USB_BOS_LENGTH,
	USB_BOS_DESCRIPTOR_TYPE,
	USB_SS_STRG_BOS_TOTAL_LENGTH & 0xFF,
	USB_SS_STRG_BOS_TOTAL_LENGTH >> 8,
	USB_SS_STRG_BOS_DEVCAP_NUMBER,

	7,
	USB_SS_DEVCAP_DESCRIPTOR_TYPE,
	0x02,
	0x02,
	0,
	0,
	0,

	10,
	USB_SS_DEVCAP_DESCRIPTOR_TYPE,
	3,
	0,
	0x0F,
	0,
	1,
	0x01,
	0xF4,
	0x01/*reserved*/
};

typedef enum _UCDC_CLASS_REQ_TYPE {
	CDC_CLASS_REQ_SET_LINE_CODING,
	ENUM_DUMMY4WORD(UCDC_CLASS_REQ_TYPE)
} UCDC_CLASS_REQ_TYPE;

static _ALIGNED(64) UINT8 gCDCClsReqDataBuf[64] = {0};
static UCDC_CLASS_REQ_TYPE  m_RequestType;
static UINT32 m_RequestLen;

static BOOL CdcPstnReqCB(UINT8 Code, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL bSupported = TRUE;
	CDCLineCoding LineCoding;

	switch (Code) {
	case REQ_GET_LINE_CODING:
		LineCoding.uiBaudRateBPS = 115200;
		LineCoding.uiCharFormat = 0;//CDC_LINEENCODING_OneStopBit;
		LineCoding.uiParityType = 0;//CDC_PARITY_None;
		LineCoding.uiDataBits = 8;
		*pDataLen = sizeof(LineCoding);

		memcpy(pData, &LineCoding, *pDataLen);
		break;
	case REQ_SET_LINE_CODING:
		if (*pDataLen == sizeof(LineCoding)) {
			memcpy(&LineCoding, pData, *pDataLen);
		} else {
			bSupported = FALSE;
		}
		break;
	case REQ_SET_CONTROL_LINE_STATE:
		//debug console test
		if (*(UINT16 *)pData == 0x3) { //console ready
		}
		break;
	default:
		bSupported = FALSE;
		break;
	}
	return bSupported;
}


static void xUCDC_SS_ClassRequestHandler(uintptr_t uiEvent)
{
	UINT8 b_request;
	PUSB_DEVICE_REQUEST    pRequest;
	UINT32   len = 0;
	BOOL bStall = FALSE;
	uintptr_t uiRetBufAddr = 0;
	UINT32 uiRetSize = 0;

	pRequest = (PUSB_DEVICE_REQUEST)uiEvent;

	b_request = pRequest->b_request;

	if (b_request == REQ_GET_LINE_CODING) {
		if (1) {
			CDCLineCoding LineCoding = {0};
			len = sizeof(LineCoding);
			if (CdcPstnReqCB(b_request, (UINT8 *)&LineCoding, &len)) {
				if (len > sizeof(LineCoding)) {
					printf("LinCoding data size should NOT exceed %d byte.\r\n", sizeof(LineCoding));
					len = sizeof(LineCoding);
				}
				//bStall = FALSE;
				uiRetBufAddr = (uintptr_t) &LineCoding;
				uiRetSize = len;
			} else {
				bStall = TRUE;
			}
		} else {
			bStall = TRUE;
		}
	} else if (b_request == REQ_SET_LINE_CODING) {
		if (1) {
			m_RequestType = CDC_CLASS_REQ_SET_LINE_CODING;
			m_RequestLen = pRequest->w_length;
			uiRetBufAddr = (uintptr_t) gCDCClsReqDataBuf;
			uiRetSize = m_RequestLen;
		} else {
			bStall = TRUE;
		}
	} else if (b_request == REQ_SET_CONTROL_LINE_STATE) {
		UINT16 ControlLineState = pRequest->w_value;
		len = sizeof(ControlLineState);
		if (1) {
			CdcPstnReqCB(b_request, (UINT8 *)&ControlLineState, &len);
		}
	} else if (b_request == REQ_SEND_BREAK) {
		UINT16 SendBreak = pRequest->w_value;
		len = sizeof(SendBreak);
		if (1) {
			CdcPstnReqCB(b_request, (UINT8 *)&SendBreak, &len);
		}
	} else {
		printf("Unsupported ClassReq=0x%x\r\n", b_request);
		//stall EP0, including unreconginze command and unsupport command.
		bStall = TRUE;
	}
	usb3dev_handleSetupResult(bStall, uiRetBufAddr, uiRetSize);
}

static void xUCDC_SS_ClassRequestDone(uintptr_t uiEvent)
{
	BOOL bStall = FALSE;
	UINT8 *pData;

	pData = &gCDCClsReqDataBuf[0];

	switch (m_RequestType) {
	case CDC_CLASS_REQ_SET_LINE_CODING:
		CdcPstnReqCB(REQ_SET_LINE_CODING, pData, &m_RequestLen);
		break;
	default:
		break;
	}

	usb3dev_handleSetupResult(bStall, 0, 0);
}


static int mem_init(void)
{
	HD_RESULT                 ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[0].blk_size = POOL_SIZE_USER_DEFINIED;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
		return ret;
	}


	if(hd_common_mem_alloc("usb_buf", &usb_buf_pa, (void **)&usb_buf_va, USB_BUF_SIZE, DDR_ID0) != HD_OK) {
		printf("hd_common_mem_alloc failed\r\n");
		usb_buf_va = 0;
		usb_buf_pa = 0;
	}

	return HD_OK;
}

static HD_RESULT mem_uninit(void)
{
	HD_RESULT ret = HD_OK;

	if (usb_buf_va) {
		ret = hd_common_mem_free(usb_buf_pa, &usb_buf_va);
		if (HD_OK == ret) {
			printf("Error handle test for free an invalid address: fail %d\r\n", (int)(ret));
		}
	}

	return hd_common_mem_uninit();
}

static void write_test(UINT8 *p_buf, UINT32 buf_size, INT32 timeout)
{
	//INT32 ret_value;

	memcpy((void *)usb_buf_va, p_buf, buf_size);
	//--- data is written by CPU, flush CPU cache to PHY memory ---
	hd_common_mem_flush_cache((void *)usb_buf_va, buf_size);
	usb3vision_write_data(USB3VISION_EP_INF_CONTROL_IN, (UINT8 *)usb_buf_pa, &buf_size, timeout);
}


static void read_test(UINT32 buf_size, INT32 timeout)
{
	//UINT8 *pBuf;
	INT32 ret_value;

	memset((void*)usb_buf_va, 0, USB_BUF_SIZE);
	hd_common_mem_flush_cache((void *)usb_buf_va, USB_BUF_SIZE);

	ret_value = usb3vision_read_data(USB3VISION_EP_INF_CONTROL_OUT, (UINT8 *)usb_buf_pa, &buf_size, timeout);

	if (ret_value == 0) {
		hd_common_mem_flush_cache((void *)usb_buf_va, buf_size);

		printf("read: %s\r\n", usb_buf_va);
	}
}

int main(int argc, char *argv[])
{
	HD_RESULT ret;
	INT key;


	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		return FALSE;
	}

	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		return FALSE;
	}

	//assign descriptor
	usb3vision_set_config(USB3VISION_CONFIG_ID_DESC, USB3VISION_DESC_STRING0, (ULONG)&USBCDCStrDesc0);
	usb3vision_set_config(USB3VISION_CONFIG_ID_DESC, USB3VISION_DESC_STRING1, (ULONG)&USBCDCManuStrDesc);
	usb3vision_set_config(USB3VISION_CONFIG_ID_DESC, USB3VISION_DESC_STRING2, (ULONG)&USBCDCProdStrDesc);
	usb3vision_set_config(USB3VISION_CONFIG_ID_DESC, USB3VISION_DESC_STRING3, (ULONG)&USBCDCStrDesc3);


	usb3vision_set_config(USB3VISION_CONFIG_ID_DESC, USB3VISION_DESC_SS_BOS, (ULONG)&gCDCSSBosDesc);
	usb3vision_set_config(USB3VISION_CONFIG_ID_DESC, USB3VISION_DESC_SS_DEVICE, (ULONG)&gUSBSSCDCDevDesc);
	usb3vision_set_config(USB3VISION_CONFIG_ID_DESC, USB3VISION_DESC_SS_CONFIG, (ULONG)&gUSBSSCDCConfigDesc);


	//assign endpoint
	usb3vision_set_config(USB3VISION_CONFIG_ID_EP, USB3VISION_EP_INF_CONTROL_IN, (ULONG)USB3VISION_EP1);
	usb3vision_set_config(USB3VISION_CONFIG_ID_EP, USB3VISION_EP_INF_CONTROL_OUT, (ULONG)USB3VISION_EP2);


	usb3vision_set_config(USB3VISION_CONFIG_ID_CLASS_REQUEST_CB, (ULONG)xUCDC_SS_ClassRequestHandler, 0);

	usb3vision_set_config(USB3VISION_CONFIG_ID_CLASS_REQUEST_DONE_CB, (ULONG)xUCDC_SS_ClassRequestDone, 0);

	usb3vision_open();

	while (1) {
		key = getchar();
		//key = 0;
		if (key == 'q' || key == 0x3) {
			break;
		}

		if (key == 'w') {
			char test[128] = {0};

			snprintf(test, 127, "test w");

			write_test((UINT8*)test, 16, 200);

			printf("write\r\n");
		}

		if (key == 'r') {
			read_test(16, 3000);
			printf("read\r\n");
		}

		usleep(10);
	}

	usb3vision_close();

	ret = mem_uninit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}

	if (hd_common_uninit() != HD_OK) {
		printf("hd_common_uninit fail\n");
	}

	return 0;
}
