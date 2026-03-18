/**
	@brief Source file of kflow_ai_net.

	@file kflow_pou_platform.h

	@ingroup kflow_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#ifndef _KFLOW_AI_POU_PLATFORM_H_
#define _KFLOW_AI_POU_PLATFORM_H_

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
#endif

#endif // _KFLOW_AI_POU_PLATFORM_H_
