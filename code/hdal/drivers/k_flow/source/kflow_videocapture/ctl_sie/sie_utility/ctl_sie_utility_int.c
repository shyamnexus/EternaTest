/**
    ctrl SIE utility

    @file       ctl_sie_utility_int.c
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#include "ctl_sie_utility_int.h"

#define CTL_SIE_TAG(ch0, ch1, ch2, ch3) ((UINT32)(UINT8)(ch0) |((UINT32)(UINT8)(ch1) << 8) |((UINT32)(UINT8)(ch2) << 16) |((UINT32)(UINT8)(ch3) << 24))
#define CTL_SIE_HDL_TAG CTL_SIE_TAG('C', 'S', 'I', 'E')

/* Refers to kdrv_sie.c `kdrv_sie_limit_98xxx` */
CTL_SIE_LIMIT ctl_sie_limit[CTL_SIE_MAX_SUPPORT_ID] = {0};

BOOL ctl_sie_module_chk_id_valid(CTL_SIE_ID id)
{
	if (id >= CTL_SIE_MAX_SUPPORT_ID) {
		ctl_sie_dbg_err("illegal id %d > maximum id %d\r\n", (int)(id), (int)(CTL_SIE_MAX_SUPPORT_ID - 1));
		return FALSE;
	}
	return TRUE;
}

static INT32 ctl_sie_chk_hdl(ULONG hdl)
{
	CTL_SIE_HDL *ptr = (CTL_SIE_HDL *)hdl;

	if (hdl == 0) {
		ctl_sie_dbg_err("input handle zero\r\n");
		return CTL_SIE_E_HDL;
	}

	if (ptr->tag != CTL_SIE_HDL_TAG) {
		ctl_sie_dbg_err("check handle fail\r\n");
		return CTL_SIE_E_HDL;
	}
	return CTL_SIE_E_OK;
}

UINT32 ctl_sie_get_hdl_tag(void)
{
	return CTL_SIE_HDL_TAG;
}

CTL_SIE_ID ctl_sie_hdl_conv2_id(ULONG hdl)
{
	CTL_SIE_HDL *ctl_sie_hdl;

	if (ctl_sie_chk_hdl(hdl) != CTL_SIE_E_OK) {
		return CTL_SIE_MAX_SUPPORT_ID;
	}

	ctl_sie_hdl = (CTL_SIE_HDL *)hdl;

	return ctl_sie_hdl->id;
}

BOOL ctl_sie_module_chk_is_raw(CTL_SEN_MODE_TYPE mode_type)
{
	switch ((UINT64)mode_type) {
	case CTL_SEN_MODE_LINEAR:
	case CTL_SEN_MODE_BUILTIN_HDR:
	case CTL_SEN_MODE_STAGGER_HDR:
	case CTL_SEN_MODE_PDAF:
		return TRUE;

	case CTL_SEN_MODE_CCIR:
	case CTL_SEN_MODE_CCIR_INTERLACE:
		return FALSE;

	default:
		return TRUE;
	}
}

UINT32 ctl_sie_uint64_dividend(UINT64 dividend, UINT32 divisor)
{
	if (divisor == 0) {
		return 0;
	}

#if defined (__FREERTOS)
	return (UINT32)(dividend / divisor);
#else
	do_div(dividend, divisor);
	return (UINT32)(dividend);
#endif
}

UINT32 ctl_sie_max(UINT32 x, UINT32 y)
{
	if (x >= y) {
		return x;
	} else {
		return y;
	}
}

UINT32 ctl_sie_min(UINT32 x, UINT32 y)
{
	if (x <= y) {
		return x;
	} else {
		return y;
	}
}

UINT32 ctl_sie_lcm(UINT32 a, UINT32 b)
{
	UINT64 input_a, input_b;
	UINT64 tmp;

	if (a == 0) {
		a++;
	}
	if (b == 0) {
		b++;
	}

	input_a = a;
	input_b = b;

	while (b != 0) {
		tmp = a % b;
		a = b;
		b = tmp;
	}

	return CTL_SIE_DIV_U64(input_a * input_b, a);
}

BOOL ctl_sie_chk_align(UINT32 src, UINT32 align)
{
	if ((src == 0) || (align == 0)) {
		return TRUE;
	}
	if ((src % align) == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

UINT64 ctl_sie_util_get_syst_timestamp(void)
{
	return hwclock_get_longcounter();
}

UINT32 ctl_sie_util_get_timestamp(void)
{
	return hwclock_get_counter();
}

INT32 ctl_sie_util_os_malloc(CTL_SIE_VOS_MEM_INFO *vod_mem_info, UINT32 req_size)
{
	if (vos_mem_init_cma_info(&vod_mem_info->cma_info, VOS_MEM_CMA_TYPE_CACHE, req_size) != 0) {
		DBG_ERR("init cma failed\r\n");
		return CTL_SIE_E_NOMEM;
	}

	vod_mem_info->cma_hdl = vos_mem_alloc_from_cma(&vod_mem_info->cma_info);
	if (vod_mem_info->cma_hdl == NULL) {
		DBG_ERR("alloc cma failed\r\n");
		return CTL_SIE_E_NOMEM;
	}

	return CTL_SIE_E_OK;
}

INT32 ctl_sie_util_os_mfree(CTL_SIE_VOS_MEM_INFO *vod_mem_info)
{
	if (vos_mem_release_from_cma(vod_mem_info->cma_hdl) != 0) {
		DBG_ERR("release cma failed\r\n");
		return CTL_SIE_E_NOMEM;
	}
	return CTL_SIE_E_OK;
}

void *ctl_sie_util_os_malloc_wrap(UINT32 want_size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(want_size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(want_size);
#endif

	return p_buf;
}

void ctl_sie_util_os_mfree_wrap(void *p_buf)
{
#if defined(__LINUX)
	kfree(p_buf);
#elif defined(__FREERTOS)
	free(p_buf);
#endif
}


/*
NT96530:
    SIE1: support all output dest
    SIE2/SIE3: support DRAM and DIRECT(only when shdr sensor)
    SIE4/SIE5/SIE6: only support DRAM



           | SIE1 | SIE2 | SIE3 | SIE4 | SIE5 |
    -------|-----------------------------------
    dram   |  O   |  O   |  O   |  O   |  O   |
    direct |  O   | flow | flow |  -   |  -   |
    -------|-----------------------------------
    CSI1   |  O   |  O   |  O   |  O   |  -   |
    CSI2   |  -   |  -   |  O   |  O   |  O   |
    CSI3   |  -   |  -   |  O   |  O   |  O   |
    CSI4   |  -   |  -   |  O   |  O   |  O   |
    CSI5   |  -   |  -   |  -   |  -   |  O   |

*/

BOOL ctl_sie_chk_hw_direct_id(CTL_SIE_ID id)
{
	if (ctl_sie_limit[id].support_func & KDRV_SIE_FUNC_SPT_DIRECT) {
		return TRUE;
	}
	return FALSE;
}

BOOL ctl_sie_chk_hw_ringbuf_id(CTL_SIE_ID id)
{
	if ((id == CTL_SIE_ID_2) || (id == CTL_SIE_ID_3)) {
		return TRUE;
	}
	return FALSE;
}

#if defined(__KERNEL__)
/**
 * @brief  Get clk. Always clk_put after using it.
 * 
 * Reference on `/base/linux-BSP/linux-kernel/drivers/clk/clk.c`
 * 
 * @param clk_name
 * @return struct clk*
 */
struct clk *ctl_sie_clk_get(const char *clk_name)
{
	return clk_get(NULL, clk_name);
}

/**
 * @brief Always put clk after using it.
 * 
 * Reference on `/base/linux-BSP/linux-kernel/drivers/clk/clk.c`
 * 
 * @param clk 
 */
void ctl_sie_clk_put(struct clk *clk)
{
	return clk_put(clk);
}

/**
 * @brief Get parent clk node. Always clk_put after using it.
 * 
 * Reference on `/base/linux-BSP/linux-kernel/drivers/clk/clk.c`
 * 
 * @param clk 
 * @return struct clk* 
 */
struct clk *ctl_sie_clk_get_parent(struct clk *clk)
{
	return clk_get_parent(clk);
}

/**
 * @brief Set parent clk node. Always clk_put after using it.
 * 
 * Reference on `/base/linux-BSP/linux-kernel/drivers/clk/clk.c`
 * 
 * @param clk 
 * @param parent 
 * @return int 
 */
int ctl_sie_clk_set_parent(struct clk *clk, struct clk *parent)
{
	return clk_set_parent(clk, parent);
}

/**
 * @brief Get clk_rate. Get clk first, and put after using it.
 * 
 * Reference on `/base/linux-BSP/linux-kernel/drivers/clk/clk.c`
 * 
 * @param clk 
 * @return unsigned long 
 */
unsigned long ctl_sie_clk_get_rate(struct clk *clk)
{
	return clk_get_rate(clk);
}

/**
 * @brief IVOT_N12191_CO-352
 *   - In `build/nvt-tools/check_clock.sh`, it will check the clk_set_rate
 *       usage. It can only be used while fetching dts information.
 *   - E.G. of_property_read_u32(node, "current_rate", &current_rate)" 
 *   - In this case, kflow/sie should not support clk_set_rate api to avoid
 *       violate the philosophy.
 */

/**
 * @brief Set clk autogate. Always clk_put after using it.
 * 
 * Reference on `/base/linux-BSP/linux-kernel/drivers/clk/clk.c`
 * 
 * @param clk 
 * @param enable 
 * @return int 
 */
int ctl_sie_clk_set_gate(struct clk *clk, BOOL enable)
{
	return clk_set_phase(clk, (int) enable);
}
#endif
