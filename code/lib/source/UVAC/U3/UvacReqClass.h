/*++

Copyright (c) 2003  Novatek Microelectronics Corporation

Module Name:

    UvacReqClass.h

Abstract:

    The header file for USB Mass Storage Class Request.

Environment:

    For nt96400

Notes:

  Copyright (c) 2003 Novatek Microelectronics Corporation.  All Rights Reserved.


Revision History:

    10/02/03: Ceated by Penny Cheng
    10/29/03: Created by Alex Sun
--*/
#ifndef _UVACREQCLASS_H
#define _UVACREQCLASS_H

#include "UvacDesc.h"
#define USB_UVAC_CLS_DATABUF_LEN                0x40  //64 byte
#define USB_CLASS_REQUEST_USBSTOR_RESET         0xFF

#define USB_ClassReq_USIDC_SetCancelReq               0x64             //Host-to-Device command
#define USB_ClassReq_USIDC_GetExtendEvent             0x65             //Device-to-Host command
#define USB_ClassReq_USIDC_SetResetReq                0x66             //Host-to-Device command
#define USB_ClassReq_USIDC_GetDeviceStatusReq         0x67             //Device-to-Host command
#define USIDC_CLS_DATA_LEN    0x24  //0x0C //0x04
#define SAMPLING_FREQ_CONTROL 0x01

#define    USIDC_DEVICE_OK                0x00
#define    USIDC_DEVICE_BUSY              0x01
#define    USIDC_DEVICE_ONCANCEL          0x02

extern BOOL UVC_TrigStilImg(uintptr_t dataAddr, UINT32 dataSize);
extern void UVC_InitProbeCommitData(void);
#if (UVAC_CONTROLLER == UVAC_CONTROL_U2)
extern void UvacClassRequestHandler(void);
#elif (UVAC_CONTROLLER == UVAC_CONTROL_U3)
extern void UvacClassRequestHandler(uintptr_t uiEvent);
extern void UvacClassRequestDone(uintptr_t uiEvent);
extern void U3UvacStdRequestHandler(uintptr_t uiEvent);
#endif
extern void UVC_InitStillProbeCommitData(UVC_STILL_PROBE_COMMIT *pStilProbComm);
extern void UVC_XU_VendorCmd(UINT32 ControlCode, UINT8 CS, UINT8 *pData);
extern void UVC_WinUSB_ClassReq(UINT32 ControlCode, UINT8 CS, UINT8 *pData);

#endif  // _UVACREQCLASS_H

