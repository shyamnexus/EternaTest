/*
    USB 3.0 device controller driver internal header file

    This file define some internal data structures for USB3 device

    @file       usb3dev_int.h
    @ingroup    mIDrvUSB3_Device
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _USB3DEV_INT_H
#define _USB3DEV_INT_H
#include <stdio.h>
#include <string.h>
#include "kwrap/error_no.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/task.h"
#include "kwrap/util.h"

#include "rcw_macro.h"
#include "kwrap/type.h"
#include "io_address.h"
#include "interrupt.h"
#include "usb3dev.h"
#include "usb3dev_reg.h"
#include "usb_define.h"
#include "dma.h"
#include "dma_protected.h"
#include "gpio.h"
//#include "sys.h"
//#include "rom_emu/usb_bulk.h"
//#include "rom_emu/usb_scsi.h"
//#include "rom_emu/usb_otg.h"

#define __MODULE__    rtos_usb3dev
#include <kwrap/debug.h>
extern unsigned int rtos_usb3dev_debug_level;
extern BOOL	usb3_suspend_to_dramoff_en;

#if defined(_NVT_FPGA_)
#define _FPGA_EMULATION_ 1
#define _EMULATION_ 1
#elif defined(_NVT_EMULATION_)
#define _FPGA_EMULATION_ 0
#define _EMULATION_ 1
#else
#define _FPGA_EMULATION_ 0
#define _EMULATION_ 0
#endif

#define USB_DELAY_US_POLL(x)	vos_util_delay_us_polling(x)
#define USB_DELAY_US(x)			vos_task_delay_ms(((x+999)/1000))
#define USB_DELAY_MS(x)			vos_task_delay_ms((x))

#define USB3DEV_DUMP_SETUP             	DISABLE
#define USB3DEV_DEBUG                	DISABLE
#if USB3DEV_DEBUG
#define usb3dbg_api(msg)               	DBG_WRN     msg
#define usb3dbg_std(msg)               	DBG_WRN     msg
#define usb3dbg_evt(msg)               	DBG_DUMP    msg
#define usb3dbg_detail(msg)             //DBG_DUMP    msg
#else
#define usb3dbg_api(msg)
#define usb3dbg_std(msg)
#define usb3dbg_evt(msg)                DBG_DUMP    msg
#define usb3dbg_detail(msg)
#endif

#define TSK_PRI_U3DEVIST        1
#define TSK_STKSIZE_U3DEVIST    4096

#define TSK_PRI_U3DEVEP0        1
#define TSK_STKSIZE_U3DEVEP0	4096

/* U3 global setting */
#define TOP_ADDR 0x2F0010000
#define CG_ADDR 0x2F0020000
#define STBC_CG_ADDR 0x2F0230000
#define U3_CLK_EN_OFS 0x70
#define U3_CLK_EN_BIT 2
#define U3_PCLK_EN_OFS 0xE0
#define U3_PCLK_EN_BIT 2

/* U3 SSC setting */
#define SSC_PER_OFS 0x4594
#define SSC_MUL_OFS 0x4590
#define SSC_EN_OFS  0x458C
#define SSC_PER_VAL 0xBE
#define SSC_MUL_VAL 0x6
#define SSC_EN_VAL  0x95


extern ID FLG_ID_USB3;

extern THREAD_HANDLE TSK_ID_U3DEVIST;
extern THREAD_HANDLE TSK_ID_U3DEVEP0;
extern THREAD_HANDLE uiU3TestTskID1;
extern THREAD_HANDLE uiU3TestTskID2;
extern THREAD_HANDLE uiU3TestTskID3;
extern THREAD_HANDLE uiU3TestTskID4;

extern void usb3dev_ist(void *pvParameters);
extern void usb3dev_ep0_task(void *pvParameters);
extern void usb3dev_bulk_testing(void *pvParameters);
extern void usb3dev_intrpt_testing(void *pvParameters);
extern void usb3dev_isoin_testing(void *pvParameters);
extern void usb3dev_isoout_testing(void *pvParameters);

#define USB3_TEST_CV                                   DISABLE         //Test U1, U2, Request = 48 Set SEL ,Request 49 Set ISO DELAY

#define USB3_SYS_SLEEP_TESTMODE         0 //0:normal, 1: PD3, 2: STBC
#define USB3_NORMAL_TEST 0
#define USB3_PD3_TEST 1
#define USB3_STBC_TEST 2

#define USB3_EMBED_TESTFUNC             DISABLE
#define USB3_EMBED_TESTFUNC_MSG         DISABLE
#define USB3_EMBED_TESTF_HEAVYLOAD      ENABLE

#define USB3_CONFIG_PHY ENABLE
#define USB3_CONFIG_TRIM ENABLE

/* This PIO is tested OK in the MSDC */
#define USB3_IN_PIO                     DISABLE
#define USB3_OUT_PIO                    DISABLE

#define IOADDR_USB3CTL_REG_BASE         IOADDR_USB3_PHY_REG_BASE
#define USB3DEV_DEBUGSEL                0x1

#if defined(_PCIE_EMU_ON_)
#define USB3_SETREG(_OFS, _VAL)         OUTW(APB_LOCAL2PCIE(IOADDR_USB3_REG_BASE)+(_OFS), (_VAL))
#define USB3_GETREG(_OFS)               INW(APB_LOCAL2PCIE(IOADDR_USB3_REG_BASE)+(_OFS))

#define USB3APB_SETREG(_OFS, _VAL)      OUTW(APB_LOCAL2PCIE(IOADDR_USB3CTL_REG_BASE)+(_OFS), (_VAL))
#define USB3APB_GETREG(_OFS)            INW(APB_LOCAL2PCIE(IOADDR_USB3CTL_REG_BASE)+(_OFS))
#else
#define USB3_SETREG(_OFS, _VAL)         OUTW(IOADDR_USB3_REG_BASE+(_OFS), (_VAL))
#define USB3_GETREG(_OFS)               INW(IOADDR_USB3_REG_BASE+(_OFS))

#define USB3APB_SETREG(_OFS, _VAL)      OUTW(IOADDR_USB3CTL_REG_BASE+(_OFS), (_VAL))
#define USB3APB_GETREG(_OFS)            INW(IOADDR_USB3CTL_REG_BASE+(_OFS))
#endif


/*
	Flag Pattern Definition
*/
#define FLGUSB3_EP01                   	FLGPTN_BIT(1) //0x00000002  // EP1
#define FLGUSB3_EP02                   	FLGPTN_BIT(2) //0x00000004  // EP2
#define FLGUSB3_EP03                   	FLGPTN_BIT(3) //0x00000008  // EP3
#define FLGUSB3_EP04                   	FLGPTN_BIT(4) //0x00000010  // EP4
#define FLGUSB3_EP05                   	FLGPTN_BIT(5) //0x00000020  // EP5
#define FLGUSB3_EP06                   	FLGPTN_BIT(6) //0x00000040  // EP6
#define FLGUSB3_EP07                   	FLGPTN_BIT(7) //0x00000080  // EP7
#define FLGUSB3_EP08                   	FLGPTN_BIT(8) //0x00000100  // EP8
#define FLGUSB3_EP09                   	FLGPTN_BIT(9) //0x00000200  // EP9
#define FLGUSB3_EP10                   	FLGPTN_BIT(10) //0x00000400  // EP10
#define FLGUSB3_EP11                   	FLGPTN_BIT(11) //0x00000800  // EP11
#define FLGUSB3_EP12                   	FLGPTN_BIT(12) //0x00001000  // EP12
#define FLGUSB3_EP13                   	FLGPTN_BIT(13) //0x00002000  // EP13
#define FLGUSB3_EP14                   	FLGPTN_BIT(14) //0x00004000  // EP14
#define FLGUSB3_EP15                   	FLGPTN_BIT(15) //0x00008000  // EP15
#define FLGUSB3_IST                     FLGPTN_BIT(16) //0x00010000
#define FLGUSB3_IST_IDLE                FLGPTN_BIT(17) //0x00020000
#define FLGUSB3_SETUP                	FLGPTN_BIT(18) //0x00040000
#define FLGUSB3_SETUP_IDLE              FLGPTN_BIT(19) //0x00080000
#define FLGUSB3_CXIN                	FLGPTN_BIT(20) //0x00100000
#define FLGUSB3_CXOUT                	FLGPTN_BIT(21) //0x00200000

#define FLGUSB3_TESTING1                FLGPTN_BIT(24) //0x01000000
#define FLGUSB3_TESTING1_IDLE           FLGPTN_BIT(25) //0x02000000
#define FLGUSB3_TESTING2                FLGPTN_BIT(26) //0x04000000
#define FLGUSB3_TESTING2_IDLE           FLGPTN_BIT(27) //0x08000000
#define FLGUSB3_TESTING3                FLGPTN_BIT(28) //0x10000000
#define FLGUSB3_TESTING3_IDLE           FLGPTN_BIT(29) //0x20000000
#define FLGUSB3_TESTING4                FLGPTN_BIT(30) //0x40000000
#define FLGUSB3_TESTING4_IDLE           FLGPTN_BIT(31) //0x80000000


#define USB3_DESC_INTF_NO               16
#define USB3CX_STDREQ_TBL_CNT           12
#define USB3_MAX_EP_NO                  15

typedef struct {
	BOOL    bSTALL;
	UINT32  uiBufAddr;
	UINT32  uiSize;
	UINT32  uiRemain;
} USB3_SETUPDATA;


typedef struct {
	BOOL            	Opened;
	UINT32          	uiBufferAddr;
	UINT32          	uiBufferSize;
	U3DEV_SPEED     	MaxSpeed;
	U3DEV_EPIN_INTSRC	EpInIntSrc;

	USB_GENERIC_CB  	gfpU3CALLBACK[U3DEV_CALLBACK_ID_NO];

} USB3_DEVINFO;


typedef enum {
	USB3_EPCTRL_FLAG_ONGOING = 0x00000001,
	USB3_EPCTRL_FLAG_AUTORST = 0x00000002,

	USB3_EPCTRL_FLAG_ALL     = 0xFFFFFFFF,
	USB3_EPCTRL_FLAG_KEEP    = USB3_EPCTRL_FLAG_AUTORST,
	ENUM_DUMMY4WORD(USB3_EPCTRL_FLAG)
} USB3_EPCTRL_FLAG;

typedef struct {
	UINT32              TxfrAddr;
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

/* Feature numbers */
#define UF_ENDPOINT_HALT                  0
#define UF_DEVICE_REMOTE_WAKEUP           1
#define UF_TEST_MODE                      2
#define UF_DEVICE_B_HNP_ENABLE            3
#define UF_DEVICE_A_HNP_SUPPORT           4
#define UF_DEVICE_A_ALT_HNP_SUPPORT       5
#define WUF_WUSB                          3
#define WUF_TX_DRPIE                      0x0
#define WUF_DEV_XMIT_PACKET               0x1
#define WUF_COUNT_PACKETS                 0x2
#define WUF_CAPTURE_PACKETS               0x3
#define UF_FUNCTION_SUSPEND               0
#define UF_U1_ENABLE                      48
#define UF_U2_ENABLE                      49
#define UF_LTM_ENABLE                     50

#define UT_GET_DIR(a)                     ((a) & 0x80)
#define UT_WRITE                          0x00
#define UT_READ                           0x80

#define UE_GET_DIR(a)                     ((a) & 0x80)
#define UE_SET_DIR(a,d)                   ((a) | (((d)&1) << 7))
#define UE_DIR_IN                         0x80
#define UE_DIR_OUT                        0x00
#define UE_ADDR                           0x0f
#define UE_GET_ADDR(a)                    ((a) & UE_ADDR)
#define UE_XFERTYPE                       0x03
#define UE_CONTROL                        0x00
#define UE_ISOCHRONOUS                    0x01
#define UE_BULK                           0x02
#define UE_INTERRUPT                      0x03
#define UE_GET_XFERTYPE(a)                ((a) & UE_XFERTYPE)
#define UE_ISO_TYPE 0x0c
#define UE_ISO_ASYNC                      0x04
#define UE_ISO_ADAPT                      0x08
#define UE_ISO_SYNC                       0x0c
#define UE_GET_ISO_TYPE(a)                ((a) & UE_ISO_TYPE)


#define UT_GET_RECIPIENT(a)               ((a) & 0x1f)
#define UT_DEVICE                         0x00
#define UT_INTERFACE                      0x01
#define UT_ENDPOINT                       0x02
#define UT_OTHER                          0x03


#define UDESC_DEVICE                      0x01
#define UDESC_CONFIG                      0x02
#define UDESC_STRING                      0x03
#define UDESC_INTERFACE                   0x04
#define UDESC_ENDPOINT                    0x05
#define UDESC_SS_USB_COMPANION            0x30
#define UDESC_DEVICE_QUALIFIER            0x06
#define UDESC_OTHER_SPEED_CONFIGURATION   0x07
#define UDESC_INTERFACE_POWER             0x08
#define UDESC_OTG                         0x09
#define UDESC_BOS                         0x0f
#define UDESC_DEVICE_CAPABILITY           0x10
#define UDESC_CS_DEVICE                   0x21    /* class specific */
#define UDESC_CS_CONFIG                   0x22
#define UDESC_CS_STRING                   0x23
#define UDESC_CS_INTERFACE                0x24
#define UDESC_CS_ENDPOINT                 0x25
#define UDESC_HUB                         0x29



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


// Test Mode Selectors
#define TM_TEST_J					0x01
#define TM_TEST_K					0x02
#define TM_TEST_SE0_NAK				0x03
#define TM_TEST_PACKET				0x04

#define TEST_J						0x01
#define TEST_K						0x02
#define TEST_SE0_NAK				0x04
#define TEST_PKY					0x08

extern ER               usb3_readPhyReg(UINT32 uiOffset, UINT32 *puiValue);
extern ER               usb3_writePhyReg(UINT32 uiOffset, UINT32 uiValue);
extern void             usb3_forceU3RxTermEnable(BOOL bEN);
extern ER               usb3_writeU2PhyReg(UINT32 uiOffset, UINT32 uiValue);
extern UINT32           usb3_readU2PhyReg(UINT32 uiOffset);
extern ER               usb_writePhyReg(UINT32 uiOffset, UINT32 uiValue);



/*
	Internal API in usb3dev.c
*/

extern void                     *gpU3Descriptors[U3DEV_DESC_ID_NO];
extern USB3_DEVINFO             gU3DevInfo;
extern volatile USB3_SETUPDATA 	gU3SetupData;

extern void                     usb3_power_on_init(void);

/*
	Internal API in usb3dev_command.c
*/
extern void 			usb3_set_bit(UINT32 uiRegOfs, UINT32 BitSet);
extern void 			usb3_clear_bit(UINT32 uiRegOfs, UINT32 BitClr);
extern void 			usb3_set_u1_timeout(UINT32 timeout_val);
extern void 			usb3_set_u2_timeout(UINT32 timeout_val);
extern void 			usb3_set_vbus_debounce(UINT32 debounce);
extern UINT32			usb3_get_vbus_debounce(void);
extern void 			usb3_set_global_interrupt(BOOL bEnable);
extern UINT32 			usb3_read_ep0_fifo(UINT32 length, UINT8 *pBuf);
extern void 			usb3_set_ep0_write_length(UINT32 length);
extern UINT32 			usb3_write_ep0_fifo(UINT32 length, UINT8 *pBuf);
extern void 			usb3_set_ep0_stall(void);
extern void 			usb3_set_ep0_done(void);
extern U3DEV_SPEED 		usb3_get_device_speed(void);
extern void 			usb3_clear_fifo(USB_EP EPn);

extern void 			usb3_resetEpInfo(void);
extern ER               usb3_setEpInfo(USB_EP EPn, USB3_EPINFO_ID CfgID, UINT32 uiCfg);
extern UINT32 			usb3_getEpInfo(USB_EP EPn, USB3_EPINFO_ID CfgID);
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
extern _ALIGNED(64) USB_DEVICE_DESC     gU3TestDevDesc;
extern _ALIGNED(64) UINT8               gU3SsTestCfgDesc[96];
extern _ALIGNED(64) UINT8               gU3SsTestBosDesc[];
extern _ALIGNED(64) UINT8               gU3StrDesc0[];
extern _ALIGNED(64) UINT8               gU3StrDesc1[];
extern _ALIGNED(64) UINT8               gU3StrDesc2[];
extern _ALIGNED(64) UINT8               gU3StrDesc3[];
extern _ALIGNED(64) UINT8               gU3StrDesc4[];

extern _ALIGNED(64) USB_DEVICE_DESC     gU3U2HsTestDevDesc;
extern _ALIGNED(64) USB_DEVICE_DESC     gU3U2FsTestDevDesc;
extern _ALIGNED(64) UINT8               gU3U2HsTestCfgDesc[60];
extern _ALIGNED(64) UINT8               gU3U2FsTestCfgDesc[46];

extern _ALIGNED(64) UINT8               puiU3ControlBuf[16384+ 4096];

extern _ALIGNED(64) UINT8               puiU3BulkBuf[262144 + 4096];

extern _ALIGNED(64) UINT8               puiU3IsoInBuf[0x201000];
extern _ALIGNED(64) UINT8               puiU3IsoOutBuf[262144];


extern void                             fotg330CxVendor(UINT32 SETUPADDR);
extern void                             fotg330UsbEventCallback(UINT32 uiEvent);
extern void                             fotg330_setConfigCallback(UINT32 uiEvent);

#endif
extern void U3TEST_InstallID(void);

#endif

