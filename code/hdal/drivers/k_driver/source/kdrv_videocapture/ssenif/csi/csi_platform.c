#include <kwrap/type.h>
#include <kwrap/util.h>
#include "csi_platform.h"
#include "csi_reg.h"
#include "csi_int.h"

#ifdef __KERNEL__
#include "csi_drv.h"
extern PMODULE_INFO pcsi_mod_info;
//#define CSI_ENABLE_PLLCLK(x)   clk_enable(pcsi_mod_info->pclk[x])
//#define CSI_DISABLE_PLLCLK(x)  clk_disable(pcsi_mod_info->pclk[x])
static struct completion csi_completion;
static struct completion csi2_completion;
static struct completion csi3_completion;
static struct completion csi4_completion;
//static struct completion csi5_completion;

static struct clk *gCsi3_pxclk = NULL;		// CSI3 pxclk src select
static struct clk *gSie3SrcClk = NULL;		// CSI3 pxclk parner: SIE3
static struct clk *gVie1SrcClk = NULL;		// CSI3 pxclk parner: VIE1
static struct clk *gCsiSrcClk[3] = {NULL};

#else
//#define CSI_ENABLE_PLLCLK(x)    pll_enable_clock((x))
//#define CSI_DISABLE_PLLCLK(x)   pll_disable_clock((x))
#include <kwrap/debug.h>
#endif

void csi_platform_clk_enable(CSI_ID id)
{
#ifdef __KERNEL__
	DBG_IND("%s() id: %d\r\n", __func__, id);
	clk_enable(pcsi_mod_info->pclk[id]);

	if (id == CSI_ID_CSI3) {
		gCsi3_pxclk = clk_get(NULL, "csi3_pxclk");
		gSie3SrcClk = clk_get(NULL, "2f0312000.sie3");
		gVie1SrcClk = clk_get(NULL, "2f0320000.vie1" );
	}

#else

	if (id == CSI_ID_CSI) {
		pll_enable_clock(CSI_CLK);
	} else if (id == CSI_ID_CSI2) {
		pll_enable_clock(CSI2_CLK);
	} else if (id == CSI_ID_CSI3) {
		pll_enable_clock(CSI3_CLK);
	} else if (id == CSI_ID_CSI4) {
		pll_enable_clock(CSI4_CLK);
	} else {
		DBG_DUMP("CSI#%d not support\r\n", id+1);
	}
#endif
}

void csi_platform_clk_disable(CSI_ID id)
{
#ifdef __KERNEL__
	DBG_IND("%s() id: %d\r\n", __func__, id);
	clk_disable(pcsi_mod_info->pclk[id]);

	if (id == CSI_ID_CSI3) {
		clk_put(gCsi3_pxclk);
		clk_put(gSie3SrcClk);
		clk_put(gVie1SrcClk);
	}

#else

	if (id == CSI_ID_CSI) {
		pll_disable_clock(CSI_CLK);
	} else if (id == CSI_ID_CSI2) {
		pll_disable_clock(CSI2_CLK);
	} else if (id == CSI_ID_CSI3) {
		pll_disable_clock(CSI3_CLK);
	} else if (id == CSI_ID_CSI4) {
		pll_disable_clock(CSI4_CLK);
	} else {
		DBG_DUMP("CSI#%d not support\r\n", id+1);
	}
#endif
}

BOOL gcsi_hsclk_en[CSI_HSCLK_MAX] = {0};
#ifdef __KERNEL__
void csi_platform_hsclk_enable(CSI_HSCLK_SRC id)
{
	if (gcsi_hsclk_en[id]) {
		DBG_IND("%s: hsclk(%d) is opened\r\n", __func__, id);
	} else {
		DBG_IND("%s() id: %d\r\n", __func__, id);
		clk_prepare_enable(pcsi_mod_info->hsclk[id]);
		gcsi_hsclk_en[id] = 1;
	}
}
//EXPORT_SYMBOL(csi_platform_hsclk_enable);

void csi_platform_hsclk_disable(CSI_HSCLK_SRC id)
{
	if (!gcsi_hsclk_en[id]) {
		DBG_IND("%s: hsclk(%d) not opened\r\n", __func__, id);
	} else {
		DBG_IND("%s() id: %d\r\n", __func__, id);
		clk_disable_unprepare(pcsi_mod_info->hsclk[id]);
		gcsi_hsclk_en[id] = 0;
	}
}
//EXPORT_SYMBOL(csi_platform_hsclk_disable);

#else
void csi_platform_hsclk_enable(CSI_HSCLK_SRC id)
{
	if (gcsi_hsclk_en[id]) {
		DBG_IND("%s: hsclk(%d) is opened\r\n", __func__, id);
	} else if (id <= CSI4_CK3D4){
		UINT32 clkId;

		DBG_IND("%s() id: %d\r\n", __func__, id);
		switch(id) {
		case CSI_CK0D4:
			clkId = CSI_CK0D4_CLK;
			break;
		case CSI_CK1D4:
			clkId = CSI_CK1D4_CLK;
			break;
		case CSI_CK2D4:
			clkId = CSI_CK2D4_CLK;
			break;
		case CSI_CK3D4:
			clkId = CSI_CK3D4_CLK;
			break;
		case CSI2_CK1D4:
			clkId = CSI2_CK1D4_CLK;
			break;
		case CSI3_CK2D4:
			clkId = CSI3_CK2D4_CLK;
			break;
		case CSI3_CK3D4:
			clkId = CSI3_CK3D4_CLK;
			break;
		case CSI4_CK3D4:
			clkId = CSI4_CK3D4_CLK;
			break;
		default:
			DBG_WRN("%s() id: %d out of range!\r\n", __func__, id);
			return;
		}
		pll_enable_clock(clkId);
		gcsi_hsclk_en[id] = 1;
	} else {
		DBG_WRN("%s() id: %d out of range!\r\n", __func__, id);
	}
}

void csi_platform_hsclk_disable(CSI_HSCLK_SRC id)
{
	if (!gcsi_hsclk_en[id]) {
		DBG_IND("%s: hsclk(%d) not opened\r\n", __func__, id);
	} else if (id <= CSI4_CK3D4){
		UINT32 clkId;

		DBG_IND("%s() id: %d\r\n", __func__, id);
		switch(id) {
		case CSI_CK0D4:
			clkId = CSI_CK0D4_CLK;
			break;
		case CSI_CK1D4:
			clkId = CSI_CK1D4_CLK;
			break;
		case CSI_CK2D4:
			clkId = CSI_CK2D4_CLK;
			break;
		case CSI_CK3D4:
			clkId = CSI_CK3D4_CLK;
			break;
		case CSI2_CK1D4:
			clkId = CSI2_CK1D4_CLK;
			break;
		case CSI3_CK2D4:
			clkId = CSI3_CK2D4_CLK;
			break;
		case CSI3_CK3D4:
			clkId = CSI3_CK3D4_CLK;
			break;
		case CSI4_CK3D4:
			clkId = CSI4_CK3D4_CLK;
			break;
		default:
			DBG_WRN("%s() id: %d out of range!\r\n", __func__, id);
			return;
		}
		pll_disable_clock(clkId);
		gcsi_hsclk_en[id] = 0;
	} else {
		DBG_WRN("%s() id: %d out of range!\r\n", __func__, id);
	}
}
#endif


/**
	Select CSI module clock source

	@param[in] id:
	               CSI engine id

	@param[in] clkSrc:
	               0:  60MHz
	               1: 120MHz
	               2: 240MHz

*/
void csi_platform_clk_set_freq(CSI_ID id, UINT32 clkSrc)
{
#ifdef __KERNEL__
	if (id > CSI_ID_CSI4) {
		DBG_DUMP("CSI#%d not support\r\n", id+1);
		return;
	} else if (clkSrc > 2) {
		DBG_DUMP("CSI#%d clkSrc: %d out of range \r\n", id+1, clkSrc);
		return;
	}

	if (gCsiSrcClk[0] == NULL) {
		gCsiSrcClk[0] = clk_get(NULL, "fix60m");
		gCsiSrcClk[1] = clk_get(NULL, "fix120m");
		gCsiSrcClk[2] = clk_get(NULL, "fix240m");
	}
	DBG_DUMP("CSI#%d clkSrc: %d\r\n", id+1, clkSrc);

	if (IS_ERR(pcsi_mod_info->pclk[id])) {
		DBG_ERR("get pxclk source fail...\r\n");
	} else if (IS_ERR(gCsiSrcClk[clkSrc]) || gCsiSrcClk[clkSrc] == NULL) {
		DBG_ERR("get parent clk source fail...\r\n");
	} else {
		clk_set_parent(pcsi_mod_info->pclk[id], gCsiSrcClk[clkSrc]);
	}

#else
	if (id > CSI_ID_CSI4 || clkSrc > CSI_CLK_240M) {
		DBG_DUMP("CSI#%d clkSrc: %d out of range \r\n", id+1, clkSrc);
		return;
	}

	if (id == CSI_ID_CSI) {
		if (clkSrc == CSI_CLK_120M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI, PLL_CLKSEL_CSI_120);
		} else if (clkSrc == CSI_CLK_240M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI, PLL_CLKSEL_CSI_240);
		} else if (clkSrc == CSI_CLK_60M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI, PLL_CLKSEL_CSI_60);
		}
	} else if (id == CSI_ID_CSI2) {
		if (clkSrc == CSI_CLK_120M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI2, PLL_CLKSEL_CSI2_120);
		} else if (clkSrc == CSI_CLK_240M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI2, PLL_CLKSEL_CSI2_240);
		} else if (clkSrc == CSI_CLK_60M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI2, PLL_CLKSEL_CSI2_60);
		}
	} else if (id == CSI_ID_CSI3) {
		if (clkSrc == CSI_CLK_120M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI3, PLL_CLKSEL_CSI3_120);
		} else if (clkSrc == CSI_CLK_240M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI3, PLL_CLKSEL_CSI3_240);
		} else if (clkSrc == CSI_CLK_60M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI3, PLL_CLKSEL_CSI3_60);
		}
	} else if (id == CSI_ID_CSI4) {
		if (clkSrc == CSI_CLK_120M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI4, PLL_CLKSEL_CSI4_120);
		} else if (clkSrc == CSI_CLK_240M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI4, PLL_CLKSEL_CSI4_240);
		} else if (clkSrc == CSI_CLK_60M) {
			pll_set_clock_rate(PLL_CLKSEL_CSI4, PLL_CLKSEL_CSI4_60);
		}
	}
#endif
}

void csi_platform_clk_get_freq(CSI_ID id, unsigned long *p_clk)
{
#ifdef __KERNEL__
	struct clk *parent;
	struct clk *csi_clk;
	unsigned long parent_rate = 0;

	if (id == CSI_ID_CSI) {
		csi_clk = clk_get(NULL, "2f0300000.csi");
	} else if (id == CSI_ID_CSI2) {
		csi_clk = clk_get(NULL, "2f0302000.csi2");
	} else if (id == CSI_ID_CSI3) {
		csi_clk = clk_get(NULL, "2f0304000.csi3");
	} else if (id == CSI_ID_CSI4) {
		csi_clk = clk_get(NULL, "2f0306000.csi4");
	} else {
		printk("CSI#%d not support \r\n", id+1);
		return;
	}
	parent = clk_get_parent(csi_clk);
	parent_rate = clk_get_rate(parent);
	*p_clk = parent_rate;
	clk_put(csi_clk);
#else
	if (id == CSI_ID_CSI) {
		if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI) == PLL_CLKSEL_CSI_60) {
			*p_clk = 60000000;
		} else if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI) == PLL_CLKSEL_CSI_120) {
			*p_clk = 120000000;
		} else if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI) == PLL_CLKSEL_CSI_240) {
			*p_clk = 240000000;
		}
	} else if (id == CSI_ID_CSI2) {
		if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI2) == PLL_CLKSEL_CSI_60) {
			*p_clk = 60000000;
		} else if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI2) == PLL_CLKSEL_CSI_120) {
			*p_clk = 120000000;
		} else if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI2) == PLL_CLKSEL_CSI_240) {
			*p_clk = 240000000;
		}
  	} else if (id == CSI_ID_CSI3) {
		if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI3) == PLL_CLKSEL_CSI_60) {
			*p_clk = 60000000;
		} else if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI3) == PLL_CLKSEL_CSI_120) {
			*p_clk = 120000000;
		} else if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI3) == PLL_CLKSEL_CSI_240) {
			*p_clk = 240000000;
		}
	} else if (id == CSI_ID_CSI4) {
		if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI4) == PLL_CLKSEL_CSI_60) {
			*p_clk = 60000000;
		} else if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI4) == PLL_CLKSEL_CSI_120) {
			*p_clk = 120000000;
		} else if (CSI_GET_PLLCLKRATE(PLL_CLKSEL_CSI4) == PLL_CLKSEL_CSI_240) {
			*p_clk = 240000000;
		}
	} else {
		DBG_WRN("CSI#%d not support \r\n", id+1);
		return;
	}
#endif
}

/**
    Select CSI engine pxclk

    @param[in]        id:  CSI engine id, valid for CSI2/CSI4 only
    @param[in] pxclk_sel:  0: pxclk from sie;  1: pxclk from vie

*/
void csi_pxclk_sel(CSI_ID id, UINT32 pxclk_sel)
{
	if (id != CSI_ID_CSI3) {
		DBG_ERR("CSI ID%d not support \r\n", id);
		return;
	}
	DBG_DUMP("CSI eng id: %d, pxclk_sel: %d\r\n", id, pxclk_sel);

#ifdef __KERNEL__
{
	struct clk *parent_clk = NULL;
	struct clk *pxclk = NULL;

	if (id == CSI_ID_CSI3) {
		pxclk = gCsi3_pxclk;
		if (pxclk_sel) {
			parent_clk = gVie1SrcClk;
		} else {
			parent_clk = gSie3SrcClk;
		}
	}

	if (IS_ERR(pxclk)) {
		DBG_ERR("get pxclk source fail...\r\n");
	} else if (IS_ERR(parent_clk)) {
		DBG_ERR("get parent clk source fail...\r\n");
	} else {
		clk_set_parent(pxclk, parent_clk);
	}
}
#else
	//if (id == CSI_ID_CSI3) {
		if (pxclk_sel) {
			//pll_enableClock(CSI2_VIE_CLK);
			pll_set_clock_rate(PLL_CLKSEL_CSI3_PXCLK, PLL_CLKSEL_CSI3_PXCLK_VIE2);
		} else {
			//pll_disableClock(CSI2_VIE_CLK);
			pll_set_clock_rate(PLL_CLKSEL_CSI3_PXCLK, PLL_CLKSEL_CSI3_PXCLK_SIE);
		}
	//}
#endif
}

#ifdef __KERNEL__
void csi_platform_init_completion(CSI_ID id)
{
	if (id == CSI_ID_CSI)
		init_completion(&csi_completion);
	else if (id == CSI_ID_CSI2)
		init_completion(&csi2_completion);
	else if (id == CSI_ID_CSI3)
		init_completion(&csi3_completion);
	else // if (id == CSI_ID_CSI4)
		init_completion(&csi4_completion);
}

void csi_platform_reinit_completion(CSI_ID id)
{
	if (id == CSI_ID_CSI)
		reinit_completion(&csi_completion);
	else if (id == CSI_ID_CSI2)
		reinit_completion(&csi2_completion);
	else if (id == CSI_ID_CSI3)
		reinit_completion(&csi3_completion);
	else // if (id == CSI_ID_CSI4)
		reinit_completion(&csi4_completion);
}

void csi_platform_wait_completion(CSI_ID id)
{
	if (id == CSI_ID_CSI)
		wait_for_completion(&csi_completion);
	else if (id == CSI_ID_CSI2)
		wait_for_completion(&csi2_completion);
	else if (id == CSI_ID_CSI3)
		wait_for_completion(&csi3_completion);
	else // if (id == CSI_ID_CSI4)
		wait_for_completion(&csi4_completion);
}

unsigned long csi_platform_wait_completion_timeout(CSI_ID id, unsigned long timeout)
{
	if (id == CSI_ID_CSI)
		return wait_for_completion_timeout(&csi_completion, msecs_to_jiffies(timeout));
	else if (id == CSI_ID_CSI2)
		return wait_for_completion_timeout(&csi2_completion, msecs_to_jiffies(timeout));
	else if (id == CSI_ID_CSI3)
		return wait_for_completion_timeout(&csi3_completion, msecs_to_jiffies(timeout));
	else // if (id == CSI_ID_CSI4)
		return wait_for_completion_timeout(&csi4_completion, msecs_to_jiffies(timeout));
}

void csi_platform_complete(CSI_ID id)
{
	if (id == CSI_ID_CSI)
		complete(&csi_completion);
	else if (id == CSI_ID_CSI2)
		complete(&csi2_completion);
	else if (id == CSI_ID_CSI3)
		complete(&csi3_completion);
	else // if (id == CSI_ID_CSI4)
		complete(&csi4_completion);
}

void *csi_platform_ioremap(unsigned long addr, unsigned int size)
{
	return ioremap(addr, size);
}

void csi_platform_iounmap(unsigned long *addr)
{
	iounmap(addr);
}

#endif
