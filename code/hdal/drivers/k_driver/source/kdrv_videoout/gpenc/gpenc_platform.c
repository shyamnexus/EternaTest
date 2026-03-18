#ifdef __KERNEL__
#include <linux/clk.h>
#else
#include "pll.h"
#include "pll_protected.h"
#endif
#include <kwrap/type.h>
#include <kwrap/util.h>
#include "gpenc_platform.h"

void gpenc_platform_clk_set_freq(unsigned long ui_config)
{
#if 0
#ifdef __KERNEL__
	struct clk *dsipll_clk;

	dsipll_clk = clk_get(NULL, "pll11");
	clk_prepare(dsipll_clk);
	//clk_disable(dsipll_clk);
	clk_set_rate(dsipll_clk, ui_config); //unit: Hz
	clk_enable(dsipll_clk);
	clk_put(dsipll_clk);
#else
	/*div = (UINT32) (dsi_src_clk/(ui_config/1000000));
	if(div == 0)
	{
		div++;
	}*/
	pll_setPLLEn(PLL_ID_17, FALSE);
	pll_setPLL(PLL_ID_17, (ui_config / 1000000) / 12 * 131072);
	pll_setPLLEn(PLL_ID_17, TRUE);
	//DBG_ERR("PLL11 = 0x%x\r\n", (unsigned int)(ui_config / 1000000) / 12 * 131072);
#endif
#endif
}

unsigned long gpenc_platform_clk_get_freq(void)
{
#if 0
	UINT32 div;
	unsigned long gpenc_src_clk;

#ifdef __KERNEL__
	struct clk *dsipll_clk;

	dsipll_clk = clk_get(NULL, "pll17");
	clk_prepare(dsipll_clk);
	dsi_src_clk = clk_get_rate(dsipll_clk);
	div = 1;
	return (dsi_src_clk / div);
#else
	dsi_src_clk = pll_getPLLFreq(PLL_ID_17);
	div = 1;
	return (dsi_src_clk / div);
#endif
#else
	return 0;
#endif
}

