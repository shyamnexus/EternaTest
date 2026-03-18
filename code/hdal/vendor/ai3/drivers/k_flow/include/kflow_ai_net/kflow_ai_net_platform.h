/**
	@brief Source file of kflow_ai_net.

	@file kflow_ai_net_platform.h

	@ingroup kflow_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "kwrap/platform.h"

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

#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
#include <malloc.h>
#define module_param_named(a, b, c, d)
#define MODULE_PARM_DESC(a, b)

//#define debug_msg 			vk_printk

#include <string.h>
#include <kwrap/util.h>
#define SLEEP(x)    		vos_util_delay_ms(1000*(x))
#define MSLEEP(x)    		vos_util_delay_ms(x)
#define USLEEP(x)   		vos_util_delay_us(x)
#define DELAY_M_SEC(x)		vos_util_delay_ms(x)
#define DELAY_U_SEC(x)      vos_util_delay_us(x)

#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
//#include "kflow_common/nvtmpp.h"
#include <plat/top.h>
#endif

#else

#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
//#include "kflow_common/nvtmpp.h"
//#include "rtos_ns02201/top.h"     // TODO: include RTOS .h  first
#include <plat/top.h>
#else
//#include "frammap/frammap_if.h"
#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h>
#else
#include <mach/fmem.h>
#endif
#include <asm/io.h>  /* for ioremap and iounmap */

#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/of.h>

//#include <linux/printk.h>
//#define debug_msg 			vk_printk

#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
#include <kwrap/util.h>
#define SLEEP(x)    		vos_util_delay_ms(1000*(x))
#define MSLEEP(x)    		vos_util_delay_ms(x)
#define USLEEP(x)   		vos_util_delay_us(x)
#define DELAY_M_SEC(x)		vos_util_delay_ms(x)
#define DELAY_U_SEC(x)		vos_util_delay_us(x)
#else
#include <linux/delay.h>
#define SLEEP(x)    		msleep(1000*(x))
#define MSLEEP(x)    		msleep(x)
#define USLEEP(x)   		udelay(x)
#define DELAY_M_SEC(x)      msleep(x) //require delay.h
#define DELAY_U_SEC(x)      udelay(x) //require delay.h
#endif

#endif



#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))

//#include "kflow_common/nvtmpp.h"
#include "comm/hwclock.h" //for hwclock_get_longcounter()
#define _nvt_ai_get_counter() hwclock_get_longcounter()

#else //_BSP_NA51068_

#include <plat/nvt_jiffies.h> //for get_nvt_jiffies()
#include <linux/vmalloc.h>
#include <linux/slab.h>
#define _nvt_ai_get_counter() get_nvt_jiffies_us()

#endif


extern uintptr_t nvt_ai_va2pa(uintptr_t addr);

extern uintptr_t nvt_ai_pa2va_remap(uintptr_t pa, UINT32 sz);

extern uintptr_t nvt_ai_pa2va_remap_wo_sync(uintptr_t pa, UINT32 sz);

extern VOID nvt_ai_pa2va_unmap(uintptr_t va, uintptr_t pa);

extern VOID* nvt_ai_mem_alloc(UINT32 size);

extern VOID nvt_ai_mem_free(VOID* addr);

extern int nvt_ai_user_va_cacheable(uintptr_t user_va);

extern int nvt_ai_get_config_from_dtsi(char* path, char* name, UINT32* num, UINT32 size);

extern int nvt_ai_has_config_from_dtsi(char* path) ;