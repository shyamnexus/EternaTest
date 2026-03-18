/*
    USB 3.0 device controller driver Basic IO structure header file

    This file defines some Basic IO data structures for USB3 device

    @file       usb3dev_io.h
    @ingroup    mIDrvUSB3_Device
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/
#ifndef _USB3DEV_IO_H
#define _USB3DEV_IO_H

#include "usb3dev_int.h"




#define USB_SPEED_UNKNOWN       0
#define USB_SPEED_LOW           1
#define USB_SPEED_FULL          2
#define USB_SPEED_HIGH          3
#define USB_SPEED_VARIABLE      4
#define USB_SPEED_SUPER         5
#define USB_SPEED_SUPER_PLUS    6



#define USB3_LTSSM_U0           0x0
#define USB3_LTSSM_U1           0x1
#define USB3_LTSSM_U2           0x2
#define USB3_LTSSM_U3           0x3
#define USB3_LTSSM_SS_DIS       0x4
#define USB3_LTSSM_RXDET        0x5
#define USB3_LTSSM_SS_INACT     0x6
#define USB3_LTSSM_POLL         0x7
#define USB3_LTSSM_RECOV        0x8
#define USB3_LTSSM_HRESET       0x9
#define USB3_LTSSM_CTS          0xA
#define USB3_LTSSM_LPBK         0xB
#define USB3_LTSSM_RESUME_RST   0xF







void    usb3_enable_global_interrupt(void);
void    usb3_disable_global_interrupt(void);

INT32   usb3_get_device_speed(void);
void    usb3_set_address(UINT32 addr);
INT32   usb3_get_link_state(void);

BOOL    usb3_u1_enabled(void);
BOOL    usb3_u2_enabled(void);
void    usb3_accept_u1(BOOL bAccept);
void    usb3_accept_u2(BOOL bAccept);
void    usb3_disable_u1(void);
void    usb3_disable_u2(void);
void    usb3_enable_u1(void);
void    usb3_enable_u2(void);
void    usb3_enable_ep(USB3_PEP PhyEP);
void    usb3_disable_ep(USB3_PEP PhyEP);

void    usb3_lock_utmi_suspend(void);
void    usb3_unlock_utmi_suspend(void);

void    usb3_suspendPipePhy(BOOL bEn);
void    usb3_suspendUtmiPhy(BOOL bEn);

void    usb3_dumpRegister(UINT32 Ofs);

#endif
