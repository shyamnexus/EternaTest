/*
    USB 3.0 device controller standard request or basic API internal header file

    Primary interface for USB standard request or basic API internal header file

    @file       usb3dev_core.h
    @ingroup    mIDrvUSB3_Device
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/
#ifndef _USB3DEV_CORE_H
#define _USB3DEV_CORE_H

#include "usb3dev_int.h"


/*

*/
typedef enum {
	USB3_EP0STATE_OFF,
	USB3_EP0STATE_WAIT_SETUP,
	USB3_EP0STATE_WAITDATA_OUT_NRDY,
	USB3_EP0STATE_WAITDATA_IN_NRDY,
	USB3_EP0STATE_WAITSTS_OUT_NRDY,     // Status phase without data stage present
	USB3_EP0STATE_WAITSTS_OUTDATA_NRDY, // Status phase with data stage present
	USB3_EP0STATE_WAITSTS_IN_NRDY,      // Status phase without data stage present
	USB3_EP0STATE_WAITSTS_INDATA_NRDY,  // Status phase with data stage present
	USB3_EP0STATE_WAIT_RESUME,

	ENUM_DUMMY4WORD(USB3_EP0STATE)
} USB3_EP0STATE;


extern void     usb3_set_ep0_state(USB3_EP0STATE State);
extern void     usb3_init_ep_trb(uintptr_t uiAddr, UINT32 uiSize);

extern void     usb3_stop_all_xfers(BOOL bIncludeEP0);
extern void     usb3_ep0_out_start(void);

extern  BOOL    usb3_handle_dev_intr(void);
extern  void    usb3_handle_ep_intr(void);

extern ER       usb3_readEndpoint(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen);
extern ER       usb3_readEndpoint_pa(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen);
extern ER       usb3_writeEndpoint(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen);
extern ER       usb3_writeEndpoint_pa(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen);
extern ER       usb3_setEPRead(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen);
extern ER       usb3_setEPWrite(USB_EP EPn, UINT8 *pBuffer, UINT32 *pDMALen);
extern ER       usb3_readIsoEndpoint(USB_EP EPn, PU3DEV_ISODATA pIsoData);
extern ER       usb3_writeIsoEndpoint(USB_EP EPn, PU3DEV_ISODATA pIsoData);
extern ER       usb3_setIsoEPRead(USB_EP EPn, PU3DEV_ISODATA pIsoData);
extern ER       usb3_setIsoEPWrite(USB_EP EPn, PU3DEV_ISODATA pIsoData);
extern ER       usb3_setIsoEPWrite_pa(USB_EP EPn, PU3DEV_ISODATA pIsoData);
extern ER       usb3_waitReadDone(USB_EP EPn, UINT32 *pDMALen);
extern ER       usb3_waitWriteDone(USB_EP EPn, UINT32 *pDMALen);

extern void     usb3_handleSetupResult(BOOL bStall, uintptr_t uiRetBufAddr, UINT32 uiRetSize);
extern ER       usb3_setDescriptor(U3DEV_DESC_ID U3DescID, void *DescBuf);
extern ER       usb3_getDescriptor(U3DEV_DESC_ID U3DescID, void **DescBuf);
extern void     usb3_setEPinStall(USB_EP EPn);
extern void     usb3_setEPoutStall(USB_EP EPn);

#endif
