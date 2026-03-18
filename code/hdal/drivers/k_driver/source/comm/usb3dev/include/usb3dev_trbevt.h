/*
    USB 3.0 device controller driver Transfer-Request-Block/Event data structure header file

    This file defines some Transfer-Request-Block/Event data structures for USB3 device

    @file       usb3dev_trbevt.h
    @ingroup    mIDrvUSB3_Device
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/
#ifndef _USB3DEV_TRBEVT_H
#define _USB3DEV_TRBEVT_H

#include "usb3dev_int.h"


#define USB3_EP0TRB_NO      3
#define USB3_TRB_SIZE       16

#define USB3_EP0TRB_SETUP   0
#define USB3_EP0TRB_IN      1
#define USB3_EP0TRB_OUT     2


/*
    This enum represents the bit fields in the DMA Descriptor
    Status quadlet.
*/
typedef enum {
	/** Transfer Count */
	DWC_DSCSTS_XFRCNT_BITS  = 0x00ffffff,
	DWC_DSCSTS_XFRCNT_SHIFT = 0,

	/** Packet Count Minus 1 (for HS IN transfers) */
	DWC_DSCSTS_PCM1_BITS    = 0x03000000,
	DWC_DSCSTS_PCM1_SHIFT   = 24,

	/** Transfer Request Block Response */
	DWC_DSCSTS_TRBRSP_BITS  = 0xf0000000,
	DWC_DSCSTS_TRBRSP_SHIFT = 28,

	/** Response values */
	DWC_TRBRSP_MISSED_ISOC_IN   = 1,    /** @< */
	DWC_TRBRSP_SETUP_PEND       = 2,    /** @< */
	DWC_TRBRSP_XFER_IN_PROG     = 4,

	ENUM_DUMMY4WORD(DWC_DSCSTS)
} DWC_DSCSTS;




/*
    This enum represents the bit fields in the DMA Descriptor
    Control quadlet.
*/
typedef enum {
	/** Hardware-Owned bit */
	DWC_DSCCTL_HWO_BIT              = 0x00000001,
	DWC_DSCCTL_HWO_SHIFT            = 0,

	/** Last Descriptor bit */
	DWC_DSCCTL_LST_BIT              = 0x00000002,
	DWC_DSCCTL_LST_SHIFT            = 1,

	/** Chain Buffer bit */
	DWC_DSCCTL_CHN_BIT              = 0x00000004,
	DWC_DSCCTL_CHN_SHIFT            = 2,

	/** Continue on Short Packet bit */
	DWC_DSCCTL_CSP_BIT              = 0x00000008,
	DWC_DSCCTL_CSP_SHIFT            = 3,

	/** Transfer Request Block Control field */
	DWC_DSCCTL_TRBCTL_BITS          = 0x000003f0,
	DWC_DSCCTL_TRBCTL_SHIFT         = 4,

	/** Transfer Request Block Control types */
	DWC_DSCCTL_TRBCTL_NORMAL            = 1,    /** @< */
	DWC_DSCCTL_TRBCTL_SETUP             = 2,    /** @< */
	DWC_DSCCTL_TRBCTL_STATUS_2          = 3,    /** @< */
	DWC_DSCCTL_TRBCTL_STATUS_3          = 4,    /** @< */
	DWC_DSCCTL_TRBCTL_CTLDATA_1ST       = 5,    /** @< */
	DWC_DSCCTL_TRBCTL_ISOC_1ST          = 6,    /** @< */
	DWC_DSCCTL_TRBCTL_ISOC              = 7,    /** @< */
	DWC_DSCCTL_TRBCTL_LINK              = 8,
	DWC_DSCCTL_TRBCTL_NORMAL_ZLP        = 9,

	/** Interrupt on Short Packet bit */
	DWC_DSCCTL_ISP_BIT              = 0x00000400,
	DWC_DSCCTL_ISP_SHIFT            = 10,

#define DWC_DSCCTL_IMI_BIT          DWC_DSCCTL_ISP_BIT
#define DWC_DSCCTL_IMI_SHIFT        DWC_DSCCTL_ISP_SHIFT

	/** Interrupt on Completion bit */
	DWC_DSCCTL_IOC_BIT              = 0x00000800,
	DWC_DSCCTL_IOC_SHIFT            = 11,

	/** Stream ID / SOF Number */
	DWC_DSCCTL_STRMID_SOFN_BITS     = 0x3fffc000,
	DWC_DSCCTL_STRMID_SOFN_SHIFT    = 14,

	ENUM_DUMMY4WORD(DWC_DSCCTL)
} DWC_DSCCTL;




/*
    TRB (DMA Descriptor) structure

    DMA Descriptor structure contains 4 quadlets:
    Buffer Pointer Low address, Buffer Pointer High address, Status, and Control.
*/
typedef struct {
	UINT32 bptl;    /* Buffer Pointer - Low address quadlet */
	UINT32 bpth;    /* Buffer Pointer - Low address quadlet */
	UINT32 status;  /* Status quadlet. Fields defined in enum @ref desc_sts_data. */
	UINT32 control; /* Control quadlet. Fields defined in enum @ref desc_ctl_data. */

} USB3_TRB, *PUSB3_TRB;




//
//  Event Definition Group
//
/*
    This enum represents the bit fields of a generic Event Buffer entry.
*/
typedef enum {
	/** Non-Endpoint Specific Event flag */
	DWC_EVENT_NON_EP_BIT            = 0x01,
	DWC_EVENT_NON_EP_SHIFT          = 0,

	/** Non-Endpoint Specific Event Type */
	DWC_EVENT_INTTYPE_BITS          = 0xfe,
	DWC_EVENT_INTTYPE_SHIFT         = 1,

	/** Non-Endpoint Specific Event Type values */
	DWC_EVENT_DEV_INT               = 0,    /** @< */
	DWC_EVENT_OTG_INT               = 1,    /** @< */
	DWC_EVENT_CARKIT_INT            = 3,    /** @< */
	DWC_EVENT_I2C_INT               = 4,

	ENUM_DUMMY4WORD(DWC_EVENT)
} DWC_EVENT;


/**
 * This enum represents the non-generic bit fields of an Event Buffer entry
 * for Device Specific events (DEVT).
 */
typedef enum {
	/** Device Specific Event Type */
	DWC_DEVT_BITS                   = 0x00000f00,
	DWC_DEVT_SHIFT                  = 8,

	/** Device Specific Event Type values */
	DWC_DEVT_DISCONN            = 0,    /** @< */
	DWC_DEVT_USBRESET           = 1,    /** @< */
	DWC_DEVT_CONNDONE           = 2,    /** @< */
	DWC_DEVT_ULST_CHNG          = 3,    /** @< */
	DWC_DEVT_WKUP               = 4,    /** @< */
	DWC_DEVT_HIBER_REQ          = 5,    /** @< */
	DWC_DEVT_U3_L2L1_SUSP       = 6,    /** @< */
	DWC_DEVT_SOF                = 7,    /** @< */
	DWC_DEVT_ERRATICERR         = 9,    /** @< */
	DWC_DEVT_L1_SUS             = 8,    /** @< */
	DWC_DEVT_CMD_CMPL           = 10,   /** @< */
	DWC_DEVT_OVERFLOW           = 11,   /** @< */
	DWC_DEVT_VNDR_DEV_TST_RCVD  = 12,   /** @< */
	DWC_DEVT_INACT_TIMEOUT_RCVD = 13,   /** @< */
	DWC_DEVT_L1_WK              = 14,

	/** Event Information */
	DWC_DEVT_EVT_INFO_BITS          = 0xffff0000,
	DWC_DEVT_EVT_INFO_SHIFT         = 16,

	/** USB/Link State */
	DWC_DEVT_ULST_STATE_BITS        = 0x000f0000,
	DWC_DEVT_ULST_STATE_SHIFT       = 16,

	/** USB/Link State values in SS */
	DWC_LINK_STATE_U0           = 0,    /** @< */
	DWC_LINK_STATE_U1           = 1,    /** @< */
	DWC_LINK_STATE_U2           = 2,    /** @< */
	DWC_LINK_STATE_U3           = 3,    /** @< */
	DWC_LINK_STATE_SS_DIS       = 4,    /** @< */
	DWC_LINK_STATE_RX_DET       = 5,    /** @< */
	DWC_LINK_STATE_SS_INACT     = 6,    /** @< */
	DWC_LINK_STATE_POLL         = 7,    /** @< */
	DWC_LINK_STATE_RECOV        = 8,    /** @< */
	DWC_LINK_STATE_HRESET       = 9,    /** @< */
	DWC_LINK_STATE_CMPLY        = 10,   /** @< */
	DWC_LINK_STATE_LPBK         = 11,   /** @< */
	DWC_LINK_STATE_RESET        = 14,   /** @< */
	DWC_LINK_STATE_RESUME       = 15,

	/** USB/Link State values in HS/FS/LS */
	DWC_LINK_STATE_ON           = 0,    /** @< */
	DWC_LINK_STATE_SLEEP        = 2,    /** @< */
	DWC_LINK_STATE_SUSPEND      = 3,    /** @< */
	DWC_LINK_STATE_EARLY_SUSPEND    = 5,

	DWC_DEVT_ULST_SS_BIT            = 0x00100000,
	DWC_DEVT_ULST_SS_SHIFT          = 20,

#define DWC_DEVT_HIBER_STATE_BITS   DWC_DEVT_ULST_STATE_BITS
#define DWC_DEVT_HIBER_STATE_SHIFT  DWC_DEVT_ULST_STATE_SHIFT

#define DWC_DEVT_HIBER_SS_BIT       DWC_DEVT_ULST_SS_BIT
#define DWC_DEVT_HIBER_SS_SHIFT     DWC_DEVT_ULST_SS_SHIFT

	DWC_DEVT_HIBER_HIRD_BITS        = 0x0f000000,
	DWC_DEVT_HIBER_HIRD_SHIFT       = 24,

	ENUM_DUMMY4WORD(DWC_DEVT)
} DWC_DEVT;


/**
 * This enum represents the bit fields of an Event Buffer entry for
 * Endpoint Specific events (DEPEVT).
 */
typedef enum {
	/** Endpoint Number */
	DWC_DEPEVT_EPNUM_BITS               = 0x0000003e,
	DWC_DEPEVT_EPNUM_SHIFT              = 1,

	/** Endpoint Event Type */
	DWC_DEPEVT_INTTYPE_BITS             = 0x000003c0,
	DWC_DEPEVT_INTTYPE_SHIFT            = 6,

	/** Endpoint Event Type values */
	DWC_DEPEVT_XFER_CMPL        = 1,    /** @< */
	DWC_DEPEVT_XFER_IN_PROG     = 2,    /** @< */
	DWC_DEPEVT_XFER_NRDY        = 3,    /** @< */
	DWC_DEPEVT_FIFOXRUN         = 4,    /** @< */
	DWC_DEPEVT_STRM_EVT         = 6,    /** @< */
	DWC_DEPEVT_EPCMD_CMPL       = 7,

	/** Event Status for Start Xfer Command */
	DWC_DEPEVT_NO_MORE_RSCS_BIT         = 0x00001000,
	DWC_DEPEVT_NO_MORE_RSCS_SHIFT       = 12,
	DWC_DEPEVT_ISOC_TIME_PASSED_BIT     = 0x00002000,
	DWC_DEPEVT_ISOC_TIME_PASSED_SHIFT   = 13,

	/** Event Status for Stream Event */
	DWC_DEPEVT_STRM_EVT_BITS            = 0x0000f000,
	DWC_DEPEVT_STRM_EVT_SHIFT           = 12,

	/** Stream Event Status values */
	DWC_DEPEVT_STRM_FOUND       = 1,    /** @< */
	DWC_DEPEVT_STRM_NOT_FOUND   = 2,

	/** Event Status for Xfer Complete or Xfer In Progress Event */
	DWC_DEPEVT_BUS_ERR_BIT              = 0x00001000,
	DWC_DEPEVT_BUS_ERR_SHIFT            = 12,
	DWC_DEPEVT_SHORT_PKT_BIT            = 0x00002000,
	DWC_DEPEVT_SHORT_PKT_SHIFT          = 13,
	DWC_DEPEVT_IOC_BIT                  = 0x00004000,
	DWC_DEPEVT_IOC_SHIFT                = 14,
	DWC_DEPEVT_LST_BIT                  = 0x00008000,
	DWC_DEPEVT_LST_SHIFT                = 15,
#define DWC_DEPEVT_MISSED_ISOC_BIT      DWC_DEPEVT_LST_BIT
#define DWC_DEPEVT_MISSED_ISOC_SHIFT    DWC_DEPEVT_LST_SHIFT

	/** Event Status for Xfer Not Ready Event */
	DWC_DEPEVT_CTRL_BITS                = 0x00003000,
	DWC_DEPEVT_CTRL_SHIFT               = 12,
	DWC_DEPEVT_XFER_ACTIVE_BIT          = 0x00008000,
	DWC_DEPEVT_XFER_ACTIVE_SHIFT        = 15,

	/** Xfer Not Ready Event Status values */
	DWC_DEPEVT_CTRL_SETUP       = 0,    /** @< */
	DWC_DEPEVT_CTRL_DATA        = 1,    /** @< */
	DWC_DEPEVT_CTRL_STATUS      = 2,

	/** Stream ID */
	DWC_DEPEVT_STRM_ID_BITS             = 0xffff0000,
	DWC_DEPEVT_STRM_ID_SHIFT            = 16,

	/** Isoc uFrame Number (for Xfer Not Ready on Isoc EP) */
	DWC_DEPEVT_ISOC_UFRAME_NUM_BITS     = 0xffff0000,
	DWC_DEPEVT_ISOC_UFRAME_NUM_SHIFT    = 16,

	/** Xfer Resource Index (for Start Xfer Command) */
	DWC_DEPEVT_XFER_RSC_IDX_BITS        = 0x007f0000,
	DWC_DEPEVT_XFER_RSC_IDX_SHIFT       = 16,

	/** Current Data Sequence Number (for Get Endpoint State Command) */
	DWC_DEPEVT_CUR_DAT_SEQ_NUM_BITS     = 0x001f0000,
	DWC_DEPEVT_CUR_DAT_SEQ_NUM_SHIFT    = 16,

	/** Flow Control State (for Get Endpoint State Command) */
	DWC_DEPEVT_FLOW_CTRL_BIT            = 0x00200000,
	DWC_DEPEVT_FLOW_CTRL_SHIFT          = 21,

	ENUM_DUMMY4WORD(DWC_DEPEVT)
} DWC_DEPEVT;










extern uintptr_t *gpUsbEventBuf, *gpUsbCurEvent;






void usb3_fill_desc(PUSB3_TRB pTrb, uintptr_t dma_addr,
					UINT32 dma_len, UINT32 stream, UINT32 type,
					UINT32 ctrlbits, UINT32 HBW, UINT32 own);
void usb3_fill_desc_pa(PUSB3_TRB pTrb, uintptr_t dma_addr,
					UINT32 dma_len, UINT32 stream, UINT32 type,
					UINT32 ctrlbits, UINT32 HBW, UINT32 own);


UINT32  usb3_getEvent(void);
void    usb3_advanceEvent(void);
UINT32  usb3_getEventCount(void);

void    usb3_updateEventCount(UINT32 EventCount);

void    usb3_handle_disconnect_intr(void);
void    usb3_handle_connect_done_intr(void);
void    usb3_handle_suspend_intr(void);
#endif
