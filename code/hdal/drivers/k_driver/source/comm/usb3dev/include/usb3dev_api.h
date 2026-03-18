#ifndef __USB3DEV_API_H_
#define __USB3DEV_API_H_
#include "usb3dev_drv.h"

int nvt_usb3dev_api_write_reg(PUSB3DEV_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_usb3dev_api_write_pattern(PUSB3DEV_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_usb3dev_api_read_reg(PUSB3DEV_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);

#endif
