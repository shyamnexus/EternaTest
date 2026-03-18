/**
	@brief Source file of vendor net flow sample.

	@file kflow_ai_net_platform.c

	@ingroup kflow ai net mem map

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kwrap/cpu.h"

#include "kflow_isp/kflow_isp_platform.h"

#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h> //for fmem_lookup_pa, PAGE_ALIGN
#else
#include <mach/fmem.h> //for fmem_lookup_pa, PAGE_ALIGN
#endif
#include <asm/io.h>  /* for ioremap and iounmap, pfn_valid, __phys_to_pfn */

#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_))
//#include "kflow_common/nvtmpp.h"
#endif


int kflow_ai_isp_has_config_from_dtsi(char* path)
{
#if defined(__FREERTOS)
	return 0 ;
#else	

	struct device_node *dt_node;

	dt_node = of_find_node_by_path(path);
	if (dt_node) {
		return 0 ; 
	}	
	return EFAULT; 
#endif
}

int kflow_ai_isp_get_config_from_dtsi(char* path, char* name, UINT32* num, UINT32 size)
{
#if defined(__FREERTOS)
	return 0 ;
#else	

	struct device_node *dt_node;

	dt_node = of_find_node_by_path(path);
	if (dt_node) {
		if (of_property_read_u32_array(dt_node, name, (UINT32 *)num, size)) {
			return EFAULT; 
		}
		return 0 ; 
	}	
	return EFAULT; 
#endif
}
