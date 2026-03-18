#ifdef __KERNEL__
#include <linux/clk.h>
#else
#include "pll.h"
#include "pll_protected.h"
#include <kwrap/debug.h>
#endif
#include <kwrap/type.h>
#include <kwrap/util.h>
#include "include/csi_tx_platform.h"
#include <kwrap/verinfo.h>

#define CSI_TX_PLAT_CLK_NAME_STR        "pll11"
#define CSI_TX_PLAT_PLL_ID              PLL_ID_11

#if 0
#define CSI_TX2_PLAT_CLK_NAME_STR        "pll27"
#define CSI_TX2_PLAT_PLL_ID              PLL_ID_27
#endif
#if 0
void csi_tx_platform_clk_set_freq(unsigned long ui_config)
{
#ifdef __KERNEL__
	struct clk *csi_txpll_clk;

	csi_txpll_clk = clk_get(NULL, CSI_TX_PLAT_CLK_NAME_STR);
	clk_prepare(csi_txpll_clk);
	clk_set_rate(csi_txpll_clk, ui_config); //unit: Hz
	clk_enable(csi_txpll_clk);
	clk_put(csi_txpll_clk);
#else
    UINT64 clock = ui_config;
	pll_setPLLEn(CSI_TX_PLAT_PLL_ID, FALSE);
	pll_setPLL(CSI_TX_PLAT_PLL_ID, (clock * 131072 / 12 / 1000000));
	pll_setPLLEn(CSI_TX_PLAT_PLL_ID, TRUE);
#endif

}
#endif
unsigned long csi_tx_platform_clk_get_freq(void)
{
	UINT32 div;
	unsigned long csi_tx_src_clk;

#ifdef __KERNEL__
	struct clk *csi_txpll_clk;

	csi_txpll_clk = clk_get(NULL, CSI_TX_PLAT_CLK_NAME_STR);
	clk_prepare(csi_txpll_clk);
	csi_tx_src_clk = clk_get_rate(csi_txpll_clk);
	div = 1;
	return (csi_tx_src_clk / div);
#else
	csi_tx_src_clk = pll_getPLLFreq(CSI_TX_PLAT_PLL_ID);
	div = 1;
	return (csi_tx_src_clk / div);
#endif

}

unsigned long csi_tx_platform_clk_get_lp_freq(void)
{
#ifdef __KERNEL__
	struct clk *parent;
	struct clk *csi_txlp_clk;
	unsigned long parent_rate = 0;

	csi_txlp_clk = clk_get(NULL, "2f09c0000.csitx");
	parent = clk_get_parent(csi_txlp_clk);
	parent_rate = clk_get_rate(parent);
	return parent_rate;
#else
#if 0
	if (pll_getClockRate(PLL_CLKSEL_CSI_TX_LPSRC)) {
		return 80000000;
	} else 
#endif
	{
		return 60000000;
	}
#endif
}

// CSI_TX2 
#if 0
void csi_tx2_platform_clk_set_freq(unsigned long ui_config)
{
#ifdef __KERNEL__
	struct clk *csi_txpll_clk;

	csi_txpll_clk = clk_get(NULL, CSI_TX2_PLAT_CLK_NAME_STR);
	clk_prepare(csi_txpll_clk);
	clk_set_rate(csi_txpll_clk, ui_config); //unit: Hz
	clk_enable(csi_txpll_clk);
	clk_put(csi_txpll_clk);
#else
    UINT64 clock = ui_config;
	pll_setPLLEn(CSI_TX2_PLAT_PLL_ID, FALSE);
	pll_setPLL(CSI_TX2_PLAT_PLL_ID, (clock * 131072 / 12 / 1000000));
	pll_setPLLEn(CSI_TX2_PLAT_PLL_ID, TRUE);
#endif

}
#endif

#if 0 
unsigned long csi_tx2_platform_clk_get_freq(void)
{
	UINT32 div;
	unsigned long csi_tx_src_clk;

#ifdef __KERNEL__
	struct clk *csi_txpll_clk;

	csi_txpll_clk = clk_get(NULL, CSI_TX2_PLAT_CLK_NAME_STR);
	clk_prepare(csi_txpll_clk);
	csi_tx_src_clk = clk_get_rate(csi_txpll_clk);
	div = 1;
	return (csi_tx_src_clk / div);
#else
	csi_tx_src_clk = pll_getPLLFreq(CSI_TX2_PLAT_PLL_ID);
	div = 1;
	return (csi_tx_src_clk / div);
#endif

}

unsigned long csi_tx2_platform_clk_get_lp_freq(void)
{
#ifdef __KERNEL__
	struct clk *parent;
	struct clk *csi_txlp_clk;
	unsigned long parent_rate = 0;

	csi_txlp_clk = clk_get(NULL, "2f09e0000.csitx");
	parent = clk_get_parent(csi_txlp_clk);
	parent_rate = clk_get_rate(parent);
	return parent_rate;
#else
#if 0
	if (pll_getClockRate(PLL_CLKSEL_CSI_TX_LPSRC)) {
		return 80000000;
	} else 
#endif
	{
		return 60000000;
	}
#endif

}
#endif

VOS_MODULE_VERSION(csi_tx, 1, 01, 001, 06);