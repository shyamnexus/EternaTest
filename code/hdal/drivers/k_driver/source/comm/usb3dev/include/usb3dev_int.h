/*
    USB 3.0 device controller driver internal header file

    This file define some internal data structures for USB3 device

    @file       usb3dev_int.h
    @ingroup    mIDrvUSB3_Device
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/
#ifndef _USB3DEV_INT_H
#define _USB3DEV_INT_H

//#include "mach/rcw_macro.h"
#include <linux/soc/nvt/rcw_macro.h>
#include "kwrap/type.h"
//#include "DrvCommon.h"
//#include "IOAddress.h"
//#include "Utility.h"
//#include "i2c.h"

#include "kwrap/error_no.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/task.h"
#include "kwrap/util.h"
#include "kwrap/platform.h"

#include "usb3dev_dbg.h"
#include "usb3dev_drv.h"
#include "usb3dev.h"
#include "usb3dev_reg.h"
#include "usb3dev_trbevt.h"
#include "usb_define.h"

#include <linux/delay.h>
#include <linux/clk.h>
//#include "frammap/frammap_if.h"
#include <linux/kernel.h>
#include <linux/slab.h>

#define _FPGA_EMULATION_ 		DISABLE
#define _EMULATION_ 			DISABLE
#define _USB3_PORTING_TODO		DISABLE
#define _USB3_680_VERSION_A_	DISABLE
#define _USB3_KERN_SELFTEST 0

#if _USB3_KERN_SELFTEST
#define dma_flushWriteCache(addr, size) fmem_dcache_sync((void *)addr, size, DMA_TO_DEVICE)
#define dma_flushReadCache(addr, size) fmem_dcache_sync((void *)addr, size, DMA_FROM_DEVICE)
#define dma_getPhyAddr(parm) fmem_lookup_pa(parm)
#else
#define dma_flushWriteCache(addr, size) //fmem_dcache_sync((void *)addr, size, DMA_TO_DEVICE)
#define dma_flushReadCache(addr, size) //fmem_dcache_sync((void *)addr, size, DMA_FROM_DEVICE)
#define dma_getPhyAddr(parm) parm//fmem_lookup_pa(parm)
#endif
#define dma_getNonCacheAddr(addr) addr

#define Delay_DelayMs(ms)		 mdelay(ms)
#define Delay_DelayUs(us)		 udelay(us)
#define Delay_DelayUsPolling(us) udelay(us)

#define USB_DELAY_US_POLL(x)	vos_util_delay_us_polling(x)
#define USB_DELAY_US(x)			vos_task_delay_ms(((x+999)/1000))
#define USB_DELAY_MS(x)			vos_task_delay_ms((x))

extern ID FLG_ID_USB3;
extern BOOL	usb3_suspend_to_dramoff_en;

/*
        USB3 Device Driver Design Notifications
===========================================================
1.  When connect to Faraday PC Test Tool or linux nvtusb(2.0) test program.
    Please turn ENABLE USB3_EMBED_TESTFUNC. And USB3_EMBED_TESTFUNC_MSG is Optional.
    This version test program ONLY "control/bulk/isochronous" are OK for Faraday PC tool.

...continued...

*/
#define USB3_EMBED_TESTFUNC             DISABLE
#define USB3_EMBED_TESTFUNC_MSG         DISABLE

#define USB3_TXFIFO_REMAP				ENABLE

#define USB3_CONFIG_PHY				ENABLE
#define USB3_CONFIG_TRIM 				DISABLE

/* This PIO is tested OK in the MSDC */
#define USB3_IN_PIO                     DISABLE
#define USB3_OUT_PIO                    DISABLE

/* This hanshake option must perform complete testings before ENABLE */
#define USB3_HANDSHAKE_TIMERDELAY       DISABLE

// NOTE: When testing palette address, the PC Bulk size must be use fixed size less than 512
#define USB3_IDEPALETTE_TEST            DISABLE
#define USB3_TESTPALETTE_ADDR           0xF0900200 // 0xF0800200 or 0xF0900200

#define USB3_EMBED_TEST_SSC             DISABLE
#define USB3_EMBED_TEST_ELASTICBUFMODE  ENABLE

#define USB3_BARRIER                    DISABLE
#define USB3_PHYRESET_PATCH             DISABLE

#define USB3_POWERDOWN_TEST				DISABLE

#define USB3DEV_DEBUG                   DISABLE
#if USB3DEV_DEBUG
#define usb3dbg_api(msg)               	DBG_WRN     msg
#define usb3_debug(msg)                 DBG_WRN     msg
#define usb3dbg_intf(msg)               DBG_DUMP    msg
#define usb3dbg_flow(msg)               DBG_WRN     msg
#define usb3dbg_devevt(msg)             DBG_DUMP    msg
#define usb3dbg_detail(msg)             DBG_DUMP    msg
#define usb3dbg_std(msg)               	DBG_WRN     msg
#define usb3dbg_evt(msg)               	DBG_DUMP    msg
#define USB3_TEST_SS					ENABLE
#else
#define usb3dbg_api(msg)
#define usb3_debug(msg)                 //DBG_WRN   msg
#define usb3dbg_devevt(msg)             //DBG_DUMP  msg // Report State change such as suspend/resume
#define usb3dbg_flow(msg)               //DBG_WRN   msg
#define usb3dbg_intf(msg)               //DBG_DUMP  msg
#define usb3dbg_std(msg)
#define usb3dbg_evt(msg)               	DBG_DUMP    msg
#define usb3dbg_detail(msg)
#define USB3_TEST_SS					DISABLE
#endif

#define USB3DEV_DEBUGSEL                0x1

#define TSK_PRI_U3DEVIST        4
#define TSK_STKSIZE_U3DEVIST    4096

#define TSK_PRI_U3DEVEP0        4
#define TSK_STKSIZE_U3DEVEP0	4096

extern ID FLG_ID_USB3;

extern uintptr_t _USB3DEV_REG_BASE_ADDR[MODULE_REG_NUM];
//#define USB3_SETREG(_OFS, _VAL)         OUTW(IOADDR_USB3_REG_BASE+(_OFS), (_VAL))
//#define USB3_GETREG(_OFS)               INW(IOADDR_USB3_REG_BASE+(_OFS))
#define USB3_SETREG(_OFS, _VAL)         iowrite32((_VAL), (void *)(_USB3DEV_REG_BASE_ADDR[0] + (_OFS)))
#define USB3_GETREG(_OFS)               ioread32((void *)(_USB3DEV_REG_BASE_ADDR[0] + (_OFS)))

#define USB3APB_SETREG(_OFS, _VAL)      iowrite32((_VAL), (void *)(_USB3DEV_REG_BASE_ADDR[1] + (_OFS)))
#define USB3APB_GETREG(_OFS)            ioread32((void *)(_USB3DEV_REG_BASE_ADDR[1] + (_OFS)))

#define USB3CG_SETREG(_OFS, _VAL)      iowrite32((_VAL), (void *)(_USB3DEV_REG_BASE_ADDR[2] + (_OFS)))
#define USB3CG_GETREG(_OFS)            ioread32((void *)(_USB3DEV_REG_BASE_ADDR[2] + (_OFS)))


#define USB3_DEVLPM                     ENABLE
#define USB3_ENABLE_U1                  DISABLE
#define USB3_ENABLE_U2                  DISABLE


#define USB3_EVENT_BUF_SIZE             256 // 256x4=1KB
#define USB3_SETUP_BUF_SIZE             512
#define USB3_STATUS_BUF_SIZE            512


#define USB3_U1SPEL						0
#define USB3_U2SEL						1
#define USB3_U2PEL						2

#define USB3_PEP_IS_IN(PEP)             ((PEP) & 0x1)

#define USB3_EP0_DEFAULT_MAXPKTSIZE     512

#define USB3_GSBUSCFG0_DEFAULT          0x00000001
#define USB3_GSBUSCFG1_DEFAULT          0x00000300 // 0x00000300 ???? 0xC104. Outstanding req number

#define USB3_GUSB2PHYCFG_DEFAULT        0x40102408
#if USB3_EMBED_TEST_ELASTICBUFMODE
#define USB3_GUSB3PIPECTL_DEFAULT       0x010C0003 // 10e0002 ?? //0x010C0002
#else
#define USB3_GUSB3PIPECTL_DEFAULT       0x010C0002
#endif

#define USB3_DESC_INTF_NO               16


#define USB3_GCTL_PRT_CAP_HOST          1
#define USB3_GCTL_PRT_CAP_DEVICE        2
#define USB3_GCTL_PRT_CAP_OTG           3


#define FLGUSB3_PEP00                   0x00000001  // EP0-OUT
#define FLGUSB3_PEP01                   0x00000002  // EP0-IN
#define FLGUSB3_PEP02                   0x00000004  // EP1-OUT
#define FLGUSB3_PEP03                   0x00000008  // EP1-IN
#define FLGUSB3_PEP04                   0x00000010  // EP2-OUT
#define FLGUSB3_PEP05                   0x00000020  // EP2-IN
#define FLGUSB3_PEP06                   0x00000040  // EP3-OUT
#define FLGUSB3_PEP07                   0x00000080  // EP2-IN
#define FLGUSB3_PEP08                   0x00000100  // EP4-OUT
#define FLGUSB3_PEP09                   0x00000200  // EP4-IN
#define FLGUSB3_PEP10                   0x00000400  // EP5-OUT
#define FLGUSB3_PEP11                   0x00000800  // EP5-IN
#define FLGUSB3_PEP12                   0x00001000  // EP6-OUT
#define FLGUSB3_PEP13                   0x00002000  // EP6-IN
#define FLGUSB3_PEP14                   0x00004000  // EP7-OUT
#define FLGUSB3_PEP15                   0x00008000  // EP7-IN
#define FLGUSB3_PEP16                   0x00010000  // EP8-OUT
#define FLGUSB3_PEP17                   0x00020000  // EP8-IN

#define FLGUSB3_SETUP				0x00040000
#define FLGUSB3_SETUP_IDLE			0x00080000
#define FLGUSB3_CXIN                		0x01000000
#define FLGUSB3_CXOUT                	0x02000000

#define FLGUSB3_IST                     0x00100000
#define FLGUSB3_IST_IDLE                0x00200000
#define FLGUSB3_SUSPEND_UNLOCK          0x00400000
#define FLGUSB3_HANDSHAKEIDLE           0x00800000

#define FLGUSB3_TESTING                 0x01000000
#define FLGUSB3_TESTING_IDLE            0x02000000
#define FLGUSB3_TESTING2                0x04000000
#define FLGUSB3_TESTING2_IDLE           0x08000000

#define FLGUSB3_USER_EVENT_SET          0x10000000
#define FLGUSB3_USER_EVENT_DONE         0x20000000
#define FLGUSB3_IST_STOP         		0x40000000
#define FLGUSB3_SETUP_STOP				0x80000000
#define FLGUSB3_WAIT_EVT               	0x04000000

/* U3 SSC setting */
#define SSC_PER_OFS 0x4594
#define SSC_MUL_OFS 0x4590
#define SSC_EN_OFS  0x458C
#define SSC_PER_VAL 0xBE
#define SSC_MUL_VAL 0x6
#define SSC_EN_VAL  0x95

typedef struct {
	BOOL    bSTALL;
	uintptr_t  uiBufAddr;
	UINT32  uiSize;
	UINT32  uiRemain;
} USB3_SETUPDATA;

extern volatile USB3_SETUPDATA 	gU3SetupData;

/*
    USB3 Physical Endpoint Definition
*/
typedef enum {
	USB3_PEP_0, //0xC800   EP0 OUT
	USB3_PEP_1, //0xC810   EP0 IN
	USB3_PEP_2, //0xC820   EP1 OUT
	USB3_PEP_3, //0xC830   EP1 IN
	USB3_PEP_4, //0xC840   EP2 OUT
	USB3_PEP_5, //0xC850   EP2 IN
	USB3_PEP_6, //0xC860   EP3 OUT
	USB3_PEP_7, //0xC870   EP3 IN
	USB3_PEP_8, //0xC880   EP4 OUT
	USB3_PEP_9, //0xC890   EP4 IN
	USB3_PEP_10,//0xC8A0   EP5 OUT
	USB3_PEP_11,//0xC8B0   EP5 IN
	USB3_PEP_12,//0xC8C0   EP6 OUT
	USB3_PEP_13,//0xC8D0   EP6 IN
	USB3_PEP_14,//0xC8E0   EP7 OUT
	USB3_PEP_15,//0xC8F0   EP7 IN
	USB3_PEP_16,//0xC900   EP8 OUT
	USB3_PEP_17,//0xC910   EP8 IN

	USB3_PEP_NO,
	ENUM_DUMMY4WORD(USB3_PEP)
} USB3_PEP;


#define USB3_CXCB_CLASSDONE         0x00000001
#define USB3_CXCB_VENDORDONE        0x00000002


typedef struct {
	BOOL            Opened;
	uintptr_t          uiBufferAddr;
	UINT32          uiBufferSize;
	U3DEV_SPEED     MaxSpeed;
	U3DEV_EPIN_INTSRC	EpInIntSrc;

	USB_GENERIC_CB  gfpU3CALLBACK[U3DEV_CALLBACK_ID_NO];

} USB3_DEVINFO;

typedef enum {
	USB3_EPCTRL_FLAG_ONGOING = 0x00000001,
	USB3_EPCTRL_FLAG_AUTORST = 0x00000002,

	USB3_EPCTRL_FLAG_ALL     = 0xFFFFFFFF,
	USB3_EPCTRL_FLAG_KEEP    = USB3_EPCTRL_FLAG_AUTORST,
	ENUM_DUMMY4WORD(USB3_EPCTRL_FLAG)
} USB3_EPCTRL_FLAG;

typedef struct {
	uintptr_t              TxfrAddr;
	UINT32              TxfrSize;
	USB3_EPCTRL_FLAG    CtrlFlag;

} USB3_EPINFO;



typedef enum {
	USB3_EPINFO_ID_TRANSFERADDR,
	USB3_EPINFO_ID_TRANSFERSIZE,

	// Set Only
	USB3_EPINFO_ID_SET_FLAG,
	USB3_EPINFO_ID_CLR_FLAG,

	// Get Only
	USB3_EPINFO_ID_GET_FLAG,

	ENUM_DUMMY4WORD(USB3_EPINFO_ID)
} USB3_EPINFO_ID;


#define OTG_CSR_CID                       (1 << 0)
#define OTG_CSR_CROLE                     (1 << 1)

#define OTG_GIER_OTG_INT_EN               (1 << 0)
#define OTG_GIER_HOST_INT_EN              (1 << 1)
#define OTG_GIER_DEV_INT_EN               (1 << 2)

/**
 * This enum represents the bit fields in the Device Configuration
 * Register (DCFG).
 */
typedef enum {
	/** Device Speed            <i>Access: R_W</i> */
	DWC_DCFG_DEVSPD_BITS        = 0x000007,
	DWC_DCFG_DEVSPD_SHIFT       = 0,

	/** Device Speed values */
	DWC_SPEED_HS_PHY_30MHZ_OR_60MHZ     = 0,    /** @< */
	DWC_SPEED_FS_PHY_30MHZ_OR_60MHZ     = 1,    /** @< */
	DWC_SPEED_LS_PHY_6MHZ               = 2,    /** @< */
	DWC_SPEED_FS_PHY_48MHZ              = 3,    /** @< */
	DWC_SPEED_SS_PHY_125MHZ_OR_250MHZ   = 4,    /** @< */
	DWC_SPEED_SS_PHY_SSP                = 5,

	/** Device Address          <i>Access: R_W</i> */
	DWC_DCFG_DEVADDR_BITS       = 0x0003f8,
	DWC_DCFG_DEVADDR_SHIFT      = 3,

	/** Periodic Frame Interval     <i>Access: R_W</i> */
	DWC_DCFG_PER_FR_INTVL_BITS  = 0x000c00,
	DWC_DCFG_PER_FR_INTVL_SHIFT = 10,

	/** Periodic Frame Interval values */
	DWC_DCFG_PER_FR_INTVL_80            = 0,    /** @< */
	DWC_DCFG_PER_FR_INTVL_85            = 1,    /** @< */
	DWC_DCFG_PER_FR_INTVL_90            = 2,    /** @< */
	DWC_DCFG_PER_FR_INTVL_95            = 3,

	/** Device Interrupt Number     <i>Access: R_W</i> */
	DWC_DCFG_DEV_INTR_NUM_BITS  = 0x01f000,
	DWC_DCFG_DEV_INTR_NUM_SHIFT = 12,

	/** Number of Receive Buffers       <i>Access: R_W</i> */
	DWC_DCFG_NUM_RCV_BUF_BITS   = 0x3e0000,
	DWC_DCFG_NUM_RCV_BUF_SHIFT  = 17,

	/** LPM Capable             <i>Access: R_W</i> */
	DWC_DCFG_LPM_CAP_BIT        = 0x400000,
	DWC_DCFG_LPM_CAP_SHIFT      = 22,

	ENUM_DUMMY4WORD(DWC_DCFG)
} DWC_DCFG;







#define UT_GET_DIR(a) ((a) & 0x80)
#define UT_WRITE        0x00
#define UT_READ         0x80

#define UT_GET_TYPE(a) ((a) & 0x60)
#define UT_STANDARD     0x00
#define UT_CLASS        0x20
#define UT_VENDOR       0x40

#define UT_GET_RECIPIENT(a) ((a) & 0x1f)
#define UT_DEVICE       0x00
#define UT_INTERFACE    0x01
#define UT_ENDPOINT     0x02
#define UT_OTHER        0x03

#define UT_READ_DEVICE      (UT_READ  | UT_STANDARD | UT_DEVICE)
#define UT_READ_INTERFACE   (UT_READ  | UT_STANDARD | UT_INTERFACE)
#define UT_READ_ENDPOINT    (UT_READ  | UT_STANDARD | UT_ENDPOINT)
#define UT_WRITE_DEVICE     (UT_WRITE | UT_STANDARD | UT_DEVICE)
#define UT_WRITE_INTERFACE  (UT_WRITE | UT_STANDARD | UT_INTERFACE)
#define UT_WRITE_ENDPOINT   (UT_WRITE | UT_STANDARD | UT_ENDPOINT)
#define UT_READ_CLASS_DEVICE    (UT_READ  | UT_CLASS | UT_DEVICE)
#define UT_READ_CLASS_INTERFACE (UT_READ  | UT_CLASS | UT_INTERFACE)
#define UT_READ_CLASS_OTHER (UT_READ  | UT_CLASS | UT_OTHER)
#define UT_READ_CLASS_ENDPOINT  (UT_READ  | UT_CLASS | UT_ENDPOINT)
#define UT_WRITE_CLASS_DEVICE   (UT_WRITE | UT_CLASS | UT_DEVICE)
#define UT_WRITE_CLASS_INTERFACE (UT_WRITE | UT_CLASS | UT_INTERFACE)
#define UT_WRITE_CLASS_OTHER    (UT_WRITE | UT_CLASS | UT_OTHER)
#define UT_WRITE_CLASS_ENDPOINT (UT_WRITE | UT_CLASS | UT_ENDPOINT)
#define UT_READ_VENDOR_DEVICE   (UT_READ  | UT_VENDOR | UT_DEVICE)
#define UT_READ_VENDOR_INTERFACE (UT_READ  | UT_VENDOR | UT_INTERFACE)
#define UT_READ_VENDOR_OTHER    (UT_READ  | UT_VENDOR | UT_OTHER)
#define UT_READ_VENDOR_ENDPOINT (UT_READ  | UT_VENDOR | UT_ENDPOINT)
#define UT_WRITE_VENDOR_DEVICE  (UT_WRITE | UT_VENDOR | UT_DEVICE)
#define UT_WRITE_VENDOR_INTERFACE (UT_WRITE | UT_VENDOR | UT_INTERFACE)
#define UT_WRITE_VENDOR_OTHER   (UT_WRITE | UT_VENDOR | UT_OTHER)
#define UT_WRITE_VENDOR_ENDPOINT (UT_WRITE | UT_VENDOR | UT_ENDPOINT)




/* Requests */
#define UR_GET_STATUS       0x00
#define  USTAT_STANDARD_STATUS  0x00
#define  WUSTAT_WUSB_FEATURE    0x01
#define  WUSTAT_CHANNEL_INFO    0x02
#define  WUSTAT_RECEIVED_DATA   0x03
#define  WUSTAT_MAS_AVAILABILITY 0x04
#define  WUSTAT_CURRENT_TRANSMIT_POWER 0x05
#define UR_CLEAR_FEATURE    0x01
#define UR_SET_FEATURE      0x03
#define UR_SET_AND_TEST_FEATURE 0x0c
#define UR_SET_ADDRESS      0x05
#define UR_GET_DESCRIPTOR   0x06
#define  UDESC_DEVICE       0x01
#define  UDESC_CONFIG       0x02
#define  UDESC_STRING       0x03
#define  UDESC_INTERFACE    0x04
#define  UDESC_ENDPOINT     0x05
#define  UDESC_SS_USB_COMPANION 0x30
#define  UDESC_DEVICE_QUALIFIER 0x06
#define  UDESC_OTHER_SPEED_CONFIGURATION 0x07
#define  UDESC_INTERFACE_POWER  0x08
#define  UDESC_OTG      0x09
#define  WUDESC_SECURITY    0x0c
#define  WUDESC_KEY     0x0d
#define   WUD_GET_KEY_INDEX(_wValue_) ((_wValue_) & 0xf)
#define   WUD_GET_KEY_TYPE(_wValue_) (((_wValue_) & 0x30) >> 4)
#define    WUD_KEY_TYPE_ASSOC   0x01
#define    WUD_KEY_TYPE_GTK 0x02
#define   WUD_GET_KEY_ORIGIN(_wValue_) (((_wValue_) & 0x40) >> 6)
#define    WUD_KEY_ORIGIN_HOST  0x00
#define    WUD_KEY_ORIGIN_DEVICE 0x01
#define  WUDESC_ENCRYPTION_TYPE 0x0e
#define  WUDESC_BOS     0x0f
#define  WUDESC_DEVICE_CAPABILITY 0x10
#define  WUDESC_WIRELESS_ENDPOINT_COMPANION 0x11
#define  UDESC_BOS      0x0f
#define  UDESC_DEVICE_CAPABILITY 0x10
#define  UDESC_CS_DEVICE    0x21    /* class specific */
#define  UDESC_CS_CONFIG    0x22
#define  UDESC_CS_STRING    0x23
#define  UDESC_CS_INTERFACE 0x24
#define  UDESC_CS_ENDPOINT  0x25
#define  UDESC_HUB      0x29
#define UR_SET_DESCRIPTOR   0x07
#define UR_GET_CONFIG       0x08
#define UR_SET_CONFIG       0x09
#define UR_GET_INTERFACE    0x0a
#define UR_SET_INTERFACE    0x0b
#define UR_SYNCH_FRAME      0x0c
#define UR_SET_SEL      0x30
#define UR_SET_ISOC_DELAY   0x31
#define WUR_SET_ENCRYPTION  0x0d
#define WUR_GET_ENCRYPTION  0x0e
#define WUR_SET_HANDSHAKE   0x0f
#define WUR_GET_HANDSHAKE   0x10
#define WUR_SET_CONNECTION  0x11
#define WUR_SET_SECURITY_DATA   0x12
#define WUR_GET_SECURITY_DATA   0x13
#define WUR_SET_WUSB_DATA   0x14
#define  WUDATA_DRPIE_INFO  0x01
#define  WUDATA_TRANSMIT_DATA   0x02
#define  WUDATA_TRANSMIT_PARAMS 0x03
#define  WUDATA_RECEIVE_PARAMS  0x04
#define  WUDATA_TRANSMIT_POWER  0x05
#define WUR_LOOPBACK_DATA_WRITE 0x15
#define WUR_LOOPBACK_DATA_READ  0x16
#define WUR_SET_INTERFACE_DS    0x17

/* Feature numbers */
#define UF_ENDPOINT_HALT            0
#define UF_DEVICE_REMOTE_WAKEUP     1
#define UF_TEST_MODE                2
#define UF_DEVICE_B_HNP_ENABLE      3
#define UF_DEVICE_A_HNP_SUPPORT     4
#define UF_DEVICE_A_ALT_HNP_SUPPORT 5
#define WUF_WUSB        3
#define  WUF_TX_DRPIE       0x0
#define  WUF_DEV_XMIT_PACKET    0x1
#define  WUF_COUNT_PACKETS  0x2
#define  WUF_CAPTURE_PACKETS    0x3
#define UF_FUNCTION_SUSPEND 0
#define UF_U1_ENABLE        48
#define UF_U2_ENABLE        49
#define UF_LTM_ENABLE       50


#define UE_GET_DIR(a)   ((a) & 0x80)
#define UE_SET_DIR(a,d) ((a) | (((d)&1) << 7))
#define UE_DIR_IN   0x80
#define UE_DIR_OUT  0x00
#define UE_ADDR     0x0f
#define UE_GET_ADDR(a)  ((a) & UE_ADDR)
#define UE_XFERTYPE 0x03
#define  UE_CONTROL 0x00
#define  UE_ISOCHRONOUS 0x01
#define  UE_BULK    0x02
#define  UE_INTERRUPT   0x03
#define UE_GET_XFERTYPE(a)  ((a) & UE_XFERTYPE)
#define UE_ISO_TYPE 0x0c
#define  UE_ISO_ASYNC   0x04
#define  UE_ISO_ADAPT   0x08
#define  UE_ISO_SYNC    0x0c
#define UE_GET_ISO_TYPE(a)  ((a) & UE_ISO_TYPE)


#define SS_CTL_MAX_PACKET_SIZE            0x200


/*
 * *Control Transfer Configuration and Status Register
 * (CX_Config_Status, offset = 00CH)
 * */
#define FUSB300_CSR_LEN(x)                (((x) & 0xFFFF) << 8)
#define FUSB300_CSR_LEN_MSK               (0xFFFF << 8)
#define FUSB300_CSR_EMP                   (1 << 4)
#define FUSB300_CSR_FUL                   (1 << 3)
#define FUSB300_CSR_CLR                   (1 << 2)
#define FUSB300_CSR_STL                   (1 << 1)
#define FUSB300_CSR_DONE                  (1 << 0)

/*
 * *HS Control Register (offset = 304H)
 * */
#define FUSB300_HSCR_HS_LPM_PERMIT        (1 << 8)
#define FUSB300_HSCR_HS_LPM_RMWKUP        (1 << 7)
#define FUSB300_HSCR_CAP_LPM_RMWKUP       (1 << 6)
#define FUSB300_HSCR_HS_GOSUSP            (1 << 5)
#define FUSB300_HSCR_HS_GORMWKU           (1 << 4)
#define FUSB300_HSCR_CAP_RMWKUP           (1 << 3)
#define FUSB300_HSCR_IDLECNT_0MS          0
#define FUSB300_HSCR_IDLECNT_1MS          1
#define FUSB300_HSCR_IDLECNT_2MS          2
#define FUSB300_HSCR_IDLECNT_3MS          3
#define FUSB300_HSCR_IDLECNT_4MS          4
#define FUSB300_HSCR_IDLECNT_5MS          5
#define FUSB300_HSCR_IDLECNT_6MS          6
#define FUSB300_HSCR_IDLECNT_7MS          7

/*
 * * SS Controller Register 1 (offset = 30CH)
 * */
#define FUSB300_SSCR1_GO_U3_DONE          (1 << 8)
#define FUSB300_SSCR1_TXDEEMPH_LEVEL      (1 << 7)
#define FUSB300_SSCR1_DIS_SCRMB           (1 << 6)
#define FUSB300_SSCR1_FORCE_RECOVERY      (1 << 5)
#define FUSB300_SSCR1_U3_WAKEUP_EN        (1 << 4)
#define FUSB300_SSCR1_U2_EXIT_EN          (1 << 3)
#define FUSB300_SSCR1_U1_EXIT_EN          (1 << 2)
#define FUSB300_SSCR1_U2_ENTRY_EN         (1 << 1)
#define FUSB300_SSCR1_U1_ENTRY_EN         (1 << 0)


#define USB3_DESC_INTF_NO               16
#define USB3CX_STDREQ_TBL_CNT           12
#define USB3_MAX_EP_NO                  15


extern THREAD_HANDLE TSK_ID_U3DEVIST;


extern void                     *gpU3Descriptors[U3DEV_DESC_ID_NO];
extern USB3_DEVINFO     gU3DevInfo;
extern BOOL				bU3DynamicPM;
extern irqreturn_t     	usb3dev_isr(int irq, void *devid);
//extern int            	usb3dev_ist(void *pvParameters);
extern THREAD_RETTYPE 			usb3dev_ist(void *pvParameters);

extern int            		usb3_power_on_init(void);
extern BOOL 			usb3_debugDump(CHAR *strCmd);
extern void            	usb3_handle_ep0_xfer(USB3_PEP PhyEP, UINT32 Event);

extern UINT8			*gpCxBuffer;
extern UINT32			gCxBufferSize;


extern ER               usb3_readPhyReg(UINT32 uiOffset, UINT32 *puiValue);
extern ER               usb3_writePhyReg(UINT32 uiOffset, UINT32 uiValue);
extern void             usb3_forceU3RxTermEnable(BOOL bEN);
extern ER               usb3_writeU2PhyReg(UINT32 uiOffset, UINT32 uiValue);
extern UINT32           usb3_readU2PhyReg(UINT32 uiOffset);

/*
	Internal API in usb3dev_command.c
*/
extern void 			usb3_set_bit(UINT32 uiRegOfs, UINT32 BitSet);
extern void 			usb3_clear_bit(UINT32 uiRegOfs, UINT32 BitClr);
extern void 			usb3_set_u1_timeout(UINT32 timeout_val);
extern void 			usb3_set_u2_timeout(UINT32 timeout_val);
extern void 			usb3_set_vbus_debounce(UINT32 debounce);
extern void 			usb3_set_global_interrupt(BOOL bEnable);
extern UINT32 			usb3_read_ep0_fifo(UINT32 length, UINT8 *pBuf);
extern void 			usb3_set_ep0_write_length(UINT32 length);
extern UINT32 			usb3_write_ep0_fifo(UINT32 length, UINT8 *pBuf);
extern void 			usb3_set_ep0_stall(void);
extern void 			usb3_set_ep0_done(void);
extern U3DEV_SPEED 		usb3_get_device_speed(void);
extern void 			usb3_clear_fifo(USB_EP EPn);

extern void 			usb3_resetEpInfo(void);
extern ER                       usb3_setEpInfo(USB_EP EPn, USB3_EPINFO_ID CfgID, UINT32 uiCfg);
extern UINT32 			usb3_getEpInfo(USB_EP EPn, USB3_EPINFO_ID CfgID);
extern UINT32                   usb3_get_vbus_debounce(void);

extern void 			usb3_set_controller_state(USB3_CONTROLLER_STATE USB3CtrlState);
extern USB3_CONTROLLER_STATE usb3_get_controller_state(void);

extern void usb3_setLDOTX_off(void);
extern void usb3_setLDOTX_on(void);
extern void usb3_setU2P1_PD(void);
extern void usb2_setU3LDOTX_off(void);
extern void usb2_setU3LDOTX_on(void);

/*
	Internal API in usb3dev_stdrequest.c
*/
extern void usb3_stdreq_reserved(PUSB_DEVICE_REQUEST pDevReq);
extern void usb3_stdreq_set_sel(PUSB_DEVICE_REQUEST pDevReq);
extern void usb3_stdreq_set_isoc_delay(PUSB_DEVICE_REQUEST pDevReq);

extern void (*usb3_standard_device_request[])(PUSB_DEVICE_REQUEST);

#if USB3_EMBED_TESTFUNC
extern _ALIGNED(64) USB_DEVICE_DESC     *gU3TestDevDesc;
extern _ALIGNED(64) UINT8               *gU3SsTestCfgDesc;
extern _ALIGNED(64) UINT8               *gU3SsTestBosDesc;
extern _ALIGNED(64) UINT8               *gU3StrDesc0;
extern _ALIGNED(64) UINT8               *gU3StrDesc1;
extern _ALIGNED(64) UINT8               *gU3StrDesc2;
extern _ALIGNED(64) UINT8               *gU3StrDesc3;
extern _ALIGNED(64) UINT8               *gU3StrDesc4;

extern _ALIGNED(64) USB_DEVICE_DESC     *gU2HsTestDevDesc;
extern _ALIGNED(64) USB_DEVICE_DESC     *gU2FsTestDevDesc;
extern _ALIGNED(64) UINT8               *gU2HsTestCfgDesc;
extern _ALIGNED(64) UINT8               *gU2FsTestCfgDesc;

extern _ALIGNED(64) UINT8               *puiU3ControlBuf;

extern _ALIGNED(64) UINT8               *puiU3BulkBuf;
extern _ALIGNED(64) UINT8               *puiU3IsoInBuf;
extern _ALIGNED(64) UINT8               *puiU3IsoOutBuf;


extern void                             fotg200CxVendor(uintptr_t SETUPADDR);
extern void                             fotg200UsbEventCallback(UINT32 uiEvent);

extern THREAD_HANDLE                           uiU3TestTskID;
extern THREAD_HANDLE                           uiU3TestTskID2;
#endif
extern void                             U3TEST_InstallID(void);

extern ER                               usb_writePhyReg(UINT32 uiOffset, UINT32 uiValue);
extern void                             usb3_setPowerSaving(BOOL    bEnable);

extern THREAD_RETTYPE usb3dev_ep0_task(void *pvParameters);
extern void usb3dev_bulk_testing(void *pvParameters);
extern void usb3dev_intrpt_testing(void *pvParameters);
extern void usb3dev_isoin_testing(void *pvParameters);
extern void usb3dev_isoout_testing(void *pvParameters);

extern int usb3dev_testing(void *pvParameters);
extern int usb3dev_testing2(void *pvParameters);
#endif

