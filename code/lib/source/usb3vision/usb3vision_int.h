/**
    USB3 vision, Internal function declare

    @file       usb3vision_int.h

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _USB3VISIONINT_H
#define _USB3VISIONINT_H

#include "kwrap/type.h"
#include "usb3vision.h"
#include "kwrap/error_no.h"
#include "kwrap/task.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/sxcmd.h"
#include "kwrap/perf.h"
#include "kwrap/util.h"
#include "string.h"
/**
*   Internal configuration
*/

#define THIS_DBGLVL         2 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          USB3VISION
#define __DBGLVL__          THIS_DBGLVL
#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
#include <kwrap/debug.h>
/**
    Flag Pattern
*/
//@{
//Task Operation
#define FLG_CLOSE             FLGPTN_BIT(1)   //
#define FLG_ABORTREAD         FLGPTN_BIT(2)   //
#define FLG_BULK_CONTROL_OUT  FLGPTN_BIT(3)   //

#define FLG_ALL (FLG_CLOSE | FLG_ABORTREAD | FLG_BULK_CONTROL_OUT)

#define USB_MAX_DMA_LENGTH      0x7fffff

typedef struct _USB3VISION_CONTEXT {
	BOOL is_opened;
} USB3VISION_CONTEXT;

extern USB_EP usb3_vision_ep[USB3VISION_EP_TYPE_MAX];
extern UINT32 control_out_event;
extern USB3VISION_CLASS_REQUEST_CB usb3vision_class_req_cb;
extern USB3VISION_CLASS_REQUEST_DONE_CB usb3vision_class_req_done_cb;

ER _usb3vision_open(void);
ER _usb3vision_close(void);
USB3VISION_CONTEXT* _usb3vision_get_context(void);
void _usb3vision_open_needed_fifo(uintptr_t event);
void _usb3vision_event_callback(uintptr_t event);
void _usb3vision_class_request_handler(uintptr_t event);
void _usb3vision_class_request_done(uintptr_t event);

#endif
