#ifdef __KERNEL__
#include "kwrap/type.h"
#include "kwrap/debug.h"
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include "kdrv_builtin/kdrv_builtin.h"
#include "kdrv_builtin/audcap_builtin.h"

void _eac_enableclk_platform(BOOL b_en)
{
	struct clk *p_eac_clk;

	p_eac_clk = clk_get(NULL, "clk_eac");

	if (IS_ERR(p_eac_clk)) {
		nvt_dbg(ERR, "failed to get eac clk\n");
        return;
	}

	if (b_en) {
		clk_enable(p_eac_clk);
	} else {
		clk_disable(p_eac_clk);
	}
	clk_put(p_eac_clk);
}

void _eac_enableadcclk_platform(BOOL b_en)
{
	struct clk *p_eac_adc_clk;

	p_eac_adc_clk = clk_get(NULL, "clk_eacadc");
	if (IS_ERR(p_eac_adc_clk)) {
		nvt_dbg(ERR, "failed to get eac-ad clk\n");
        return;
	}

	if (b_en) {
		clk_enable(p_eac_adc_clk);
	} else {
		clk_disable(p_eac_adc_clk);
	}
	clk_put(p_eac_adc_clk);
}

void _eac_enabledacclk_platform(BOOL b_en)
{
	struct clk *p_eac_dac_clk;

	p_eac_dac_clk = clk_get(NULL, "clk_eacdac");
	if (IS_ERR(p_eac_dac_clk)) {
		nvt_dbg(ERR, "failed to get eac-da clk\n");
        return;
	}

	if (b_en) {
		clk_enable(p_eac_dac_clk);
	} else {
		clk_disable(p_eac_dac_clk);
	}
	clk_put(p_eac_dac_clk);
}

void _eac_setdacclkrate_platform(unsigned long clkrate)
{
	struct clk *eacdac_clk;

	eacdac_clk = clk_get(NULL, "clk_eacdac");
	if (IS_ERR(eacdac_clk)) {
		DBG_ERR("failed to get eacdac clk\n");
        return;
	}
	clk_set_rate(eacdac_clk, clkrate);

	// if(clkrate <= 12288000)
	// 	clk_set_rate(eacdac_clk, clkrate);
	// else{
	// 	DBG_ERR("EAC DAC CLK cannot excess 12.288MHz, force set to 12.288MHZ\n");
	// 	clk_set_rate(eacdac_clk, 12288000);
	// }

	clk_put(eacdac_clk);
}

// get clock status function

int _eac_checkclk_platform(void)
{
	struct clk *p_eac_clk;
	BOOL isEable = FALSE;

	p_eac_clk = clk_get(NULL, "clk_eac");

	if (IS_ERR(p_eac_clk)) {
		nvt_dbg(ERR, "failed to get eac clk\n");
        return -1;
	}

	isEable = __clk_is_enabled(p_eac_clk);
	clk_put(p_eac_clk);

	return isEable;
}

int _eac_checkeadcclk_platform(void)
{
	struct clk *p_eac_adc_clk;
	BOOL isEable = FALSE;

	p_eac_adc_clk = clk_get(NULL, "clk_eacadc");
	if (IS_ERR(p_eac_adc_clk)) {
		nvt_dbg(ERR, "failed to get eac-ad clk\n");
        return -1;
	}

	isEable = __clk_is_enabled(p_eac_adc_clk);
	clk_put(p_eac_adc_clk);

	return isEable;
}

int _eac_checkdacclk_platform(void)
{
	struct clk *p_eac_dac_clk;
	BOOL isEable = FALSE;

	p_eac_dac_clk = clk_get(NULL, "clk_eacdac");
	if (IS_ERR(p_eac_dac_clk)) {
		nvt_dbg(ERR, "failed to get eac-da clk\n");
        return -1;
	}

	isEable = __clk_is_enabled(p_eac_dac_clk);
	clk_put(p_eac_dac_clk);

	return isEable;
}
#endif
