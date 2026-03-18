#ifndef _LVDS_PLATFORM_H_
#define _LVDS_PLATFORM_H_
#include "../../lvds.h"

extern void lvds_platform_clk_enable(LVDS_ID id) ;
extern void lvds_platform_clk_disable(LVDS_ID id);

#ifdef __KERNEL__
extern void lvds_platform_init_completion(LVDS_ID id);
extern void lvds_platform_reinit_completion(LVDS_ID id);
extern void lvds_platform_wait_completion(LVDS_ID id);
extern unsigned long lvds_platform_wait_completion_timeout(LVDS_ID id, unsigned long timeout);
extern void lvds_platform_complete(LVDS_ID id);

extern void *csi_platform_ioremap(unsigned long addr, unsigned int size);
extern void csi_platform_iounmap(unsigned long *addr);
#endif

/*
extern void csi_platform_clk_get_freq(CSI_ID id, unsigned long *p_clk);
extern void csi_pxclk_sel(CSI_ID id, UINT32 pxclk_sel);
#ifdef __KERNEL__

extern void csi_platform_hsclk_enable(CSI_HSCLK_SRC id);
extern void csi_platform_hsclk_disable(CSI_HSCLK_SRC id);
#endif
*/
#endif
