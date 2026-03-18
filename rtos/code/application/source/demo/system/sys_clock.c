#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include <plat/clock.h>
#include <plat/pll.h>
#include <kwrap/debug.h>
#include "sys_clock.h"
#include "prjcfg.h"

#define IDE1_CLK 16

//@{
#define PLL_CLKSEL_APB            8
#define PLL_CLKSEL_APB_48         (0x00 << PLL_CLKSEL_APB)      //< Select APB  48MHz
#define PLL_CLKSEL_APB_60         (0x01 << PLL_CLKSEL_APB)      //< Select APB  60MHz
#define PLL_CLKSEL_APB_80         (0x02 << PLL_CLKSEL_APB)      //< Select APB  80MHz
#define PLL_CLKSEL_APB_120        (0x03 << PLL_CLKSEL_APB)      //< Select APB 120MHz
//@}


/**
 * @brief close unused clocks such as IDE
 *
 */
extern int clock_disable_unused(void)
{
#if defined(_disp_off_)
	extern void pll_disable_clock(int num);
	pll_disable_clock(IDE1_CLK); //disable ide
#if defined(_NVT_ETHERNET_NONE_)
	// if both eth and ide off, we disable pll6
	pll_set_pll_enable(PLL_ID_6, TRUE);
	pll_set_pll_enable(PLL_ID_6, FALSE);
#endif
#endif
	return 0;
}

/**
 * @brief set apb clock if fdt indicated
 *
 */
extern int clock_set_apb_as_fdt(void)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	// read from fdt
	int len, nodeoffset;
	const void *nodep;
	const char path[] = "/cpus/cpu@0";
	nodeoffset = fdt_path_offset(p_fdt, path);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", path, nodeoffset);
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "apb-frequenccy", &len);
	if (len == 0 || nodep == NULL) {
		return 0; //skip
	} else {
		unsigned int *p_data = (unsigned int *)nodep;
		unsigned int apb_clk = be32_to_cpu(p_data[0]) / 1000000;
		unsigned int pll_apb_sel = 0;
		switch (apb_clk) {
		case 48:
			pll_apb_sel = PLL_CLKSEL_APB_48;
			break;
		case 60:
			pll_apb_sel = PLL_CLKSEL_APB_60;
			break;
		case 120:
			pll_apb_sel = PLL_CLKSEL_APB_60;
			break;
		default:
			DBG_DUMP("unknown apb clock sel %d .\n", apb_clk);
			return -1;
		}
		if (clk_change_apb(pll_apb_sel) == TRUE) {
			DBG_DUMP("set apb to %d success.\n", apb_clk);
		} else {
			DBG_DUMP("failed to set apb to %d success.\n", apb_clk);
		}

	}
	return 0;
}