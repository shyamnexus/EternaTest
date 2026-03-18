/*
    USB 3.0 device controller ISO TRB management driver

    USB 3.0 device controller ISO TRB management driver

    @file       usb3dev_isotrb.h
    @ingroup    mIDrvUSB3_Device
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/
#ifndef _USB3DEV_ISOTRB_H
#define _USB3DEV_ISOTRB_H

#include "usb3dev_int.h"


typedef enum {
	USB3_ISOTRB_IDX_0,
	USB3_ISOTRB_IDX_1,
	USB3_ISOTRB_IDX_2,
	USB3_ISOTRB_IDX_3,

	USB3_ISOTRB_IDX_MAX,
	USB3_ISOTRB_IDX_NULL = USB3_ISOTRB_IDX_MAX,
} USB3_ISOTRB_IDX;


//
typedef enum {
	USB3_EPCTRL_FLAG_ONGOING = 0x00000001,
	USB3_EPCTRL_FLAG_STALL   = 0x00000002,


	USB3_EPCTRL_FLAG_ISOTRIG = 0x01000000,
	USB3_EPCTRL_FLAG_ISOEVT  = 0x02000000,

	USB3_EPCTRL_FLAG_ALL     = 0xFFFFFFFF,
	ENUM_DUMMY4WORD(USB3_EPCTRL_FLAG)
} USB3_EPCTRL_FLAG;


/*
    After adding item to USB3_PEPINFO.
    Please remember to add defualt value to usb3_initPEpInfo();
*/
typedef struct {
	USB_EP_ATR          EpAttr;         // EP Attribute
	uintptr_t              TxfrAddr;       // For Bulk/Interrupt EP Usage
	UINT32              TxfrSize;       // For Bulk/Interrupt EP Usage
	UINT32              MaxPacketSize;  // Include HS HBW
	UINT32              LatestEvent;
	USB3_EPCTRL_FLAG    uiControlFlag;
	UINT8               uiTRI;          // transfer resource index
	UINT8               Interval;

	USB3_ISOTRB_IDX     IsoIdx;
	PUSB3_TRB           pCurrIsoTRB;

	PU3DEV_ISODATA      pIsoData;       // command
	UINT32              uiDonePktIdx;
	UINT32              uiRemainingPackets;

} USB3_PEPINFO;


typedef enum {
	USB3_PEPINFO_ID_INT_EVENT,
	USB3_PEPINFO_ID_TRI,
	USB3_PEPINFO_ID_ISOIDX,
	USB3_PEPINFO_ID_INTERVAL,
	USB3_PEPINFO_ID_TRANSFERADDR,
	USB3_PEPINFO_ID_TRANSFERSIZE,

	// Set Only
	USB3_PEPINFO_ATTRIBUTE,
	USB3_PEPINFO_MAXPACKET,
	USB3_PEPINFO_ID_SET_CTRLFLAG,
	USB3_PEPINFO_ID_CLR_CTRLFLAG,

	// Get Only
	USB3_PEPINFO_ID_MAXPKTSIZE,
	USB3_PEPINFO_ID_HBW,
	USB3_PEPINFO_ID_CBITYPE,        // Control/Bulk/Interrupt/Iso Type. Pls use USB_EP_ATR
	USB3_PEPINFO_ID_GET_CTRLFLAG,
	USB3_PEPINFO_ID_CHK_ENABLE,

	ENUM_DUMMY4WORD(USB3_PEPINFO_ID)
} USB3_PEPINFO_ID;


#if _FPGA_EMULATION_
// This is for driver slicing test
#define USB3_ISOTRB_NO                  (64+1)
#define USB3_ISO_SLICE_MAX              32          // 2ms
#else
#define USB3_ISOTRB_NO                  (128+1)     // include 1 link TRB. Must be "power of 2 +1"
#define USB3_ISO_SLICE_MAX              64          // 64/8 = 8 ms
#endif


#define USB3_ISOTRB_EPBUF_SIZE          0x840
#define USB3_ISOTRB_TOT_BUFSIZE         ((USB3_ISOTRB_EPBUF_SIZE * USB3_ISOTRB_IDX_MAX)+64) // 64B is for buffer addr align
#define USB3_ISOTRB_BUFALIGN            0x3F





extern USB3_PEPINFO     gU3PepInfo[USB3_PEP_NO];


extern  void            usb3_iso_array_init(uintptr_t IsoTrbBufAddr, UINT32 SizeInBytes);
extern  PUSB3_TRB       usb3_assignNextIsoTRB(USB3_PEP PhyEP);
extern  PUSB3_TRB       usb3_getNextIsoTRB(USB3_PEP PhyEP, PUSB3_TRB pCurr);
extern  USB3_ISOTRB_IDX usb3_assignIsoTrbIdx(USB3_PEP PhyEP);
extern  void            usb3_clearIsoTrbIdx(USB3_PEP PhyEP);

extern void             usb3_initPEpInfo(void);
extern ER               usb3_setPEpInfo(USB3_PEP PhyEP, USB3_PEPINFO_ID CfgID, UINT32 uiCfg);
extern UINT32           usb3_getPEpInfo(USB3_PEP PhyEP, USB3_PEPINFO_ID CfgID);
extern BOOL             usb3_check_stalled(USB3_PEP PhyEP);
#endif
