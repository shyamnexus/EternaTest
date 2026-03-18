#ifdef __KERNEL__
#include <linux/clk.h>
#include <kwrap/debug.h>
#else
#include "pll.h"
#include "pll_protected.h"
#endif
#include <kwrap/type.h>
#include <kwrap/util.h>


#include "mi.h"
#include "mi_int.h"
#include "mi_platform.h"
#include <kwrap/verinfo.h>



#ifdef __KERNEL__

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================

static struct clk *mi_clk;

#endif
void mi_platform_set_clksrc(UINT32 src)
{
#ifdef __KERNEL__
	struct clk *source_clk;

	if (src == 0) {
		source_clk = clk_get(NULL, "fix192m");
		if (IS_ERR(source_clk)) {
			DBG_IND("mi get clk source err\n");
			return ;
		}
		clk_set_parent(mi_clk, source_clk);
	} else if (src == 1) {
		source_clk = clk_get(NULL, "fix240m");
		if (IS_ERR(source_clk)) {
			DBG_IND("mi get clk source err\n");
			return ;
		}
		clk_set_parent(mi_clk, source_clk);
	} else {
		DBG_IND("mi clk source %d not support\n", src);
		return ;
	}
#else

#endif

}

void mi_platform_set_clkfreq(UINT32 freq)
{
#ifdef __KERNEL__
	struct clk *mi_clk;

	mi_clk = clk_get(NULL, "2f01a0000.mi");
	if (IS_ERR(mi_clk)) {
		DBG_ERR("mi get clk err\n");
		return;
	}
	clk_set_rate(mi_clk, freq);
#else

#endif

}

void mi_platform_clk_enable(BOOL en)
{
	if(en){
#if defined __UITRON || defined __ECOS || defined __FREERTOS
		pll_enable_clock(MI_CLK);
#else
		clk_prepare(mi_clk);

		clk_enable(mi_clk);
#endif
	}
	else{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
		pll_disable_clock(MI_CLK);
#else
		clk_disable(mi_clk);

		clk_unprepare(mi_clk);
#endif
	}
}

#ifdef __KERNEL__
EXPORT_SYMBOL(mi_platform_set_clksrc);
EXPORT_SYMBOL(mi_platform_set_clkfreq);
EXPORT_SYMBOL(mi_platform_clk_enable);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("mi driver");
MODULE_LICENSE("GPL");
#endif
VOS_MODULE_VERSION(mi, 1, 01, 001, 00);

