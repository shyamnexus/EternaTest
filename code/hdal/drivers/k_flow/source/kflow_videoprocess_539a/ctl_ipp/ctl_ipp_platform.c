#if defined(__LINUX)
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/io.h>
#elif defined(__FREERTOS)
#include "comm/compiler.h"
#include "comm/libfdt.h"
#include <plat/rtosfdt.h>
#include <string.h>
#include <malloc.h>
#endif
#include "ipp_debug_int.h"
#include "ctl_ipp_util.h"
#include "ctl_ipp_util_int.h"

#define CTL_IPP_UTIL_CLOCK_DFT	(280)	/* select 280 for ipp to use pll_13 as default clk src */

// sync with CHIP_ID
enum CTL_IPP_CHIP_ID {
	CTL_IPP_CHIP_NA51055 = 0x4821,
	CTL_IPP_CHIP_NA51084 = 0x5021,
	CTL_IPP_CHIP_NA51089 = 0x7021,
	CTL_IPP_CHIP_NA51090 = 0xBC21,
	CTL_IPP_CHIP_NA51102 = 0x5221,
	CTL_IPP_CHIP_NA51103 = 0x8B20,
	CTL_IPP_CHIP_NS02201 = 0xF221,
	CTL_IPP_CHIP_NS02301 = 0x7721,
	CTL_IPP_CHIP_NS02302 = 0x5A21,
	CTL_IPP_CHIP_NS02402 = 0x5B21,
};

void *ctl_ipp_util_os_malloc_wrap(UINT32 want_size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(want_size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(want_size);
#endif

	return p_buf;
}

void ctl_ipp_util_os_mfree_wrap(void *p_buf)
{
#if defined(__LINUX)
	kfree(p_buf);
#elif defined(__FREERTOS)
	free(p_buf);
#endif
}

void *ctl_ipp_util_os_ioremap_nocache_wrap(phys_addr_t offset, size_t size)
{
#if defined(__LINUX)
	return ioremap(offset, size);
#elif defined(__FREERTOS)
	return (void *)offset;
#endif
}

void ctl_ipp_util_os_iounmap_wrap(void *addr)
{
#if defined(__LINUX)
	iounmap(addr);
#endif
}

static UINT32 ctl_ipp_util_chip_id = 0;
void ctl_ipp_util_init_chip_id(void)
{
	ctl_ipp_util_chip_id = nvt_get_chip_id();
}

UINT32 ctl_ipp_util_get_chip_id(void)
{
	// check init
	if (ctl_ipp_util_chip_id == 0) {
		ctl_ipp_util_init_chip_id();
	}

	return ctl_ipp_util_chip_id;
}

INT32 ctl_ipp_util_get_max_stripe(void)
{
	UINT32 chip = ctl_ipp_util_get_chip_id();

	if (chip == CTL_IPP_CHIP_NA51055) {
		return CTL_IPP_520_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NA51084) {
		return CTL_IPP_528_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NA51089) {
		return CTL_IPP_560_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NA51102) {
		return CTL_IPP_530_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NS02201) {
		return CTL_IPP_690_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NS02302) {
		return CTL_IPP_538_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NS02301) {
		return CTL_IPP_567_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NS02402) {
		return CTL_IPP_538_MAX_STRIPE_W;
	}
	CTL_IPP_DBG_WRN("unknown chip 0x%.8x\r\n", chip);

	return CTL_IPP_538_MAX_STRIPE_W;
}

INT32 ctl_ipp_util_get_max_stripe_dual(void)
{
	UINT32 chip = ctl_ipp_util_get_chip_id();

	if (chip == CTL_IPP_CHIP_NA51055) {
		return CTL_IPP_520_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NA51084) {
		return CTL_IPP_528_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NA51089) {
		return CTL_IPP_560_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NA51102) {
		return CTL_IPP_530_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NS02201) {
		return CTL_IPP_690_MAX_STRIPE_W_DUAL;
	} else if (chip == CTL_IPP_CHIP_NS02302) {
		return CTL_IPP_538_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NS02301) {
		return CTL_IPP_567_MAX_STRIPE_W;
	} else if (chip == CTL_IPP_CHIP_NS02402) {
		return CTL_IPP_538_MAX_STRIPE_W;
	}
	CTL_IPP_DBG_WRN("unknown chip 0x%.8x\r\n", chip);

	return CTL_IPP_538_MAX_STRIPE_W;
}

USIZE ctl_ipp_util_get_max_size(void)
{
	UINT32 chip = ctl_ipp_util_get_chip_id();
	USIZE size;

	if (chip == CTL_IPP_CHIP_NA51055) {
		size.w = 8188;
		size.h = 8188;
	} else if (chip == CTL_IPP_CHIP_NA51084) {
		size.w = 8188;
		size.h = 8188;
	} else if (chip == CTL_IPP_CHIP_NA51089) {
		size.w = 8188;
		size.h = 8188;
	} else if (chip == CTL_IPP_CHIP_NA51102) {
		size.w = 8188;
		size.h = 8188;
	} else if (chip == CTL_IPP_CHIP_NS02201) {
		size.w = 16384;
		size.h = 16384;
	} else if (chip == CTL_IPP_CHIP_NS02302) {
		size.w = 8192;
		size.h = 8192;
	} else if (chip == CTL_IPP_CHIP_NS02301) {
		size.w = 8192;
		size.h = 8192;
	} else if (chip == CTL_IPP_CHIP_NS02402) {
		size.w = 8192;
		size.h = 8192;
	} else {
		CTL_IPP_DBG_WRN("unknown chip 0x%.8x\r\n", chip);
		size.w = 8192;
		size.h = 8192;
	}

	return size;
}

CTL_IPP_FUNC ctl_ipp_util_get_max_spt_func(UINT32 low_pwr_lvl)
{
	UINT32 chip = ctl_ipp_util_get_chip_id();
	CTL_IPP_FUNC max_spt_func = CTL_IPP_FUNC_MAX;

	if (chip == CTL_IPP_CHIP_NA51055) {
		return max_spt_func;
	} else if (chip == CTL_IPP_CHIP_NA51084) {
		return max_spt_func;
	} else if (chip == CTL_IPP_CHIP_NA51089) {
		return max_spt_func;
	} else if (chip == CTL_IPP_CHIP_NA51102) {
		return max_spt_func;
	} else if (chip == CTL_IPP_CHIP_NS02201) {
		return max_spt_func;
	} else if (chip == CTL_IPP_CHIP_NS02402) {
		//remove low power level 2 function
		if (low_pwr_lvl >= 2) {
			max_spt_func &= ~(CTL_IPP_FUNC_DEFOG);	//IPE
		}

		//remove low power level 3 function
		if (low_pwr_lvl >= 3) {
			max_spt_func &= ~(CTL_IPP_FUNC_PRE_VA_SUBOUT|CTL_IPP_FUNC_BNR|CTL_IPP_FUNC_BNR_STA);	//PRE
			max_spt_func &= ~(CTL_IPP_FUNC_WDR);	//IFE
		}

		//remove low power level 4 function
		if (low_pwr_lvl >= 4) {
			max_spt_func &= ~(CTL_IPP_FUNC_LCA|CTL_IPP_FUNC_3DNR|CTL_IPP_FUNC_3DNR_STA);	//IME
		}
		return max_spt_func;
	} else if (chip == CTL_IPP_CHIP_NS02301) {
		return max_spt_func;
	} else if (chip == CTL_IPP_CHIP_NS02302) {
		return max_spt_func;
	}

	CTL_IPP_DBG_WRN("unknown chip 0x%.8x\r\n", chip);
	return max_spt_func;
}

