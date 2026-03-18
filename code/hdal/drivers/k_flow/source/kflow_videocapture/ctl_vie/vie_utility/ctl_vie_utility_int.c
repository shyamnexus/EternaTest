/**
    ctrl VIE utility

    @file       ctl_vie_utility_int.c
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#include "ctl_vie_utility_int.h"

#define CTL_VIE_TAG(ch0, ch1, ch2, ch3) ((UINT32)(UINT8)(ch0) |((UINT32)(UINT8)(ch1) << 8) |((UINT32)(UINT8)(ch2) << 16) |((UINT32)(UINT8)(ch3) << 24))
#define CTL_VIE_HDL_TAG CTL_VIE_TAG('C', 'S', 'I', 'E')

CTL_VIE_LIMIT ctl_vie_limit[CTL_VIE_MAX_SUPPORT_ID] = {0};

BOOL ctl_vie_module_chk_id_valid(CTL_VIE_ID id)
{
	if (id >= CTL_VIE_MAX_SUPPORT_ID) {
		ctl_vie_dbg_err("illegal id %d > maximum id %d\r\n", (int)(id), (int)(CTL_VIE_MAX_SUPPORT_ID - 1));
		return FALSE;
	}
	return TRUE;
}

static INT32 ctl_vie_chk_hdl(ULONG hdl)
{
	CTL_VIE_HDL *ptr = (CTL_VIE_HDL *)hdl;

	if (hdl == 0) {
		ctl_vie_dbg_err("input handle zero\r\n");
		return CTL_VIE_E_HDL;
	}

	if (ptr->tag != CTL_VIE_HDL_TAG) {
		ctl_vie_dbg_err("check handle fail\r\n");
		return CTL_VIE_E_HDL;
	}
	return CTL_VIE_E_OK;
}

UINT32 ctl_vie_get_hdl_tag(void)
{
	return CTL_VIE_HDL_TAG;
}

CTL_VIE_ID ctl_vie_hdl_conv2_id(ULONG hdl)
{
	CTL_VIE_HDL *ctl_vie_hdl;

	if (ctl_vie_chk_hdl(hdl) != CTL_VIE_E_OK) {
		return CTL_VIE_MAX_SUPPORT_ID;
	}

	ctl_vie_hdl = (CTL_VIE_HDL *)hdl;

	return ctl_vie_hdl->id;
}

KDRV_DEV_ENGINE __conv2_vie_kdrv_eng(CTL_VIE_ID id)
{
	KDRV_DEV_ENGINE kdrv_eng_map[] = {
		KDRV_VDOCAP_VIE_ENGINE0,
		KDRV_VDOCAP_VIE_ENGINE1,
	};

	if (!ctl_vie_module_chk_id_valid(id)) {
		return 0;
	}

	return kdrv_eng_map[id / KDRV_VIE_MAX_VDO_CH];
}

KDRV_VIE_VDO_CH __conv2_vie_kdrv_ch(CTL_VIE_ID id)
{
	KDRV_VIE_VDO_CH ch = id;

	if (!ctl_vie_module_chk_id_valid(id)) {
		return 0;
	}

	while (ch >= KDRV_VIE_MAX_VDO_CH) {
		ch -= KDRV_VIE_MAX_VDO_CH;
	};
	return ch;
}

KDRV_VIE_PROC_ID __conv2_vie_kdrv_id(CTL_VIE_ID id)
{
	UINT32 kdrv_idx = id;

	kdrv_idx = kdrv_idx / KDRV_VIE_MAX_VDO_CH;
	if (kdrv_idx < KDRV_VIE_ID_MAX_NUM - KDRV_VIE_ID_1) {
		return KDRV_VIE_ID_1 + kdrv_idx;
	}

	ctl_vie_dbg_err("ctl id %d overflow\r\n", id);
	return KDRV_VIE_ID_MAX_NUM;
}

UINT32 ctl_vie_uint64_dividend(UINT64 dividend, UINT32 divisor)
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

UINT32 ctl_vie_max(UINT32 x, UINT32 y)
{
	if (x >= y) {
		return x;
	} else {
		return y;
	}
}

UINT32 ctl_vie_min(UINT32 x, UINT32 y)
{
	if (x <= y) {
		return x;
	} else {
		return y;
	}
}

UINT32 ctl_vie_lcm(UINT32 a, UINT32 b)
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

	return CTL_VIE_DIV_U64(input_a * input_b, a);
}

BOOL ctl_vie_chk_align(UINT32 src, UINT32 align)
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

UINT64 ctl_vie_util_get_syst_timestamp(void)
{
	return hwclock_get_longcounter();
}

UINT32 ctl_vie_util_get_timestamp(void)
{
	return hwclock_get_counter();
}

INT32 ctl_vie_util_os_malloc(CTL_VIE_VOS_MEM_INFO *vod_mem_info, UINT32 req_size)
{
	if (vos_mem_init_cma_info(&vod_mem_info->cma_info, VOS_MEM_CMA_TYPE_CACHE, req_size) != 0) {
		DBG_ERR("init cma failed\r\n");
		return CTL_VIE_E_NOMEM;
	}

	vod_mem_info->cma_hdl = vos_mem_alloc_from_cma(&vod_mem_info->cma_info);
	if (vod_mem_info->cma_hdl == NULL) {
		DBG_ERR("alloc cma failed\r\n");
		return CTL_VIE_E_NOMEM;
	}

	return CTL_VIE_E_OK;
}

INT32 ctl_vie_util_os_mfree(CTL_VIE_VOS_MEM_INFO *vod_mem_info)
{
	if (vos_mem_release_from_cma(vod_mem_info->cma_hdl) != 0) {
		DBG_ERR("release cma failed\r\n");
		return CTL_VIE_E_NOMEM;
	}
	return CTL_VIE_E_OK;
}

void *ctl_vie_util_os_malloc_wrap(UINT32 want_size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(want_size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(want_size);
#endif

	return p_buf;
}

void ctl_vie_util_os_mfree_wrap(void *p_buf)
{
#if defined(__LINUX)
	kfree(p_buf);
#elif defined(__FREERTOS)
	free(p_buf);
#endif
}

void ctl_vie_chk_hw_crop_w(CTL_VIE_ID id, UINT32 crop_w, BOOL *hw_spt, BOOL *four_k_mode)
{
	KDRV_VIE_VDO_CH kdrv_ch = __conv2_vie_kdrv_ch(id);

	switch (kdrv_ch) {
		case KDRV_VIE_VDO_CH0:
		case KDRV_VIE_VDO_CH1:
			if (crop_w > ctl_vie_limit[id].crp_win_max_four_k.w) {
				*hw_spt = FALSE;
				*four_k_mode = FALSE;
				ctl_vie_dbg_err("id %d kdrv_ch %d crop_w %d > hw spt %d\r\n", id, kdrv_ch, crop_w, ctl_vie_limit[id].crp_win_max_four_k.w);
			} else if (crop_w > ctl_vie_limit[id].crp_win_max.w) {
				*hw_spt = TRUE;
				*four_k_mode = TRUE;
			} else {
				*hw_spt = TRUE;
				*four_k_mode = FALSE;
			}
			break;
		case KDRV_VIE_VDO_CH2:
		case KDRV_VIE_VDO_CH3:
			if (crop_w > ctl_vie_limit[id].crp_win_max.w) {
				*hw_spt = FALSE;
				*four_k_mode = FALSE;
				ctl_vie_dbg_err("id %d kdrv_ch %d crop_w %d > hw spt %d\r\n", id, kdrv_ch, crop_w, ctl_vie_limit[id].crp_win_max.w);
			} else {
				*hw_spt = TRUE;
				*four_k_mode = FALSE;
			}
			break;
		default:
			ctl_vie_dbg_err("id %d kdrv_ch %d ovfl\r\n", id, kdrv_ch);
			*hw_spt = FALSE;
			*four_k_mode = FALSE;
			break;
	}
}

