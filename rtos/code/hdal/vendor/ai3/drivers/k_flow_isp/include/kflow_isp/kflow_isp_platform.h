/**
	@brief Source file of kflow_ai_net.

	@file kflow_isp_platform.h

	@ingroup kflow_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#ifndef _KFLOW_AI_ISP_PLATFORM_H_
#define _KFLOW_AI_ISP_PLATFORM_H_
#if defined(__FREERTOS)
#define EXPORT_SYMBOL(a)
#define MODULE_AUTHOR(a)
#define MODULE_LICENSE(a)
#define MODULE_VERSION(a)
#include <string.h>         // for memset, strncmp
#include <stdio.h>          // sscanf
#include "efuse_protected.h"
#else
//#include <linux/delay.h>
#include <linux/module.h>
#include <plat/efuse_protected.h>
#include <linux/of.h>
#endif

#endif // _KFLOW_AI_ISP_PLATFORM_H_

int kflow_ai_isp_get_config_from_dtsi(char* path, char* name, UINT32* num, UINT32 size) ;
int kflow_ai_isp_has_config_from_dtsi(char* path);
