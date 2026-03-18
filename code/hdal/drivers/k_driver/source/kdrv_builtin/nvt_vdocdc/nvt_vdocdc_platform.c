#include "nvt_vdocdc_platform.h"

#if defined(__LINUX)
#include <linux/vmalloc.h>

struct clk *nvt_vdocdc_clk_get(void)
{
	return clk_get(NULL, "venc_clk");
}

void nvt_vdocdc_clk_enable(struct clk* clk)
{
	clk_enable(clk);
}

void nvt_vdocdc_clk_disable(struct clk* clk)
{
	clk_disable(clk);
}

void nvt_vdocdc_clk_set_phase(struct clk* clk, int enable)
{
	clk_set_phase(clk, enable);
}

void *nvt_vdocdc_vmalloc(size_t size)
{
	return vmalloc(size);
}

void nvt_vdocdc_vfree(void *ptr)
{
	if (ptr != NULL) vfree(ptr);
}
#endif

