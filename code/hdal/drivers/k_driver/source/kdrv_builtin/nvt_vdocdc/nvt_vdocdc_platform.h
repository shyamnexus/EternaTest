#ifndef _NVT_VDOCDC_PLATFORM_H_
#define _NVT_VDOCDC_PLATFORM_H_

#if defined(__LINUX)
#include <linux/clk.h>

struct clk *nvt_vdocdc_clk_get(void);
void nvt_vdocdc_clk_enable(struct clk *clk);
void nvt_vdocdc_clk_disable(struct clk *clk);
void nvt_vdocdc_clk_set_phase(struct clk* clk, int enable);
void *nvt_vdocdc_vmalloc(size_t size);
void nvt_vdocdc_vfree(void *ptr);
#endif

#endif //_NVT_VDOCDC_PLATFORM_H_
