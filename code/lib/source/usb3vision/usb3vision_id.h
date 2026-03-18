
#ifndef _USB3VISIONID_H
#define _USB3VISIONID_H

#include "usb3vision_int.h"

extern ID usb3_vision_flg;
extern ID usb3_vision_sem[USB3VISION_EP_TYPE_MAX];

extern void usb3vision_install_id(void);
extern void usb3vision_uninstall_id(void);

#endif //_USB3VISIONID_H