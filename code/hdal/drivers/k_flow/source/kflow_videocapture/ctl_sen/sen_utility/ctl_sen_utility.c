/*
    sensor control - utility

    @file       ctl_sen_utility.c
    @ingroup
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#include "ctl_sen_int.h"
#include "ctl_sen_utility.h"

/*
    common
*/
UINT32 sen_uint64_dividend(UINT64 dividend, UINT32 divisor)
{
#if defined(__FREERTOS) || defined(__ECOS) || defined(__UITRON)
	return (UINT32)(dividend / divisor);
#else
	do_div(dividend, divisor);
	return (UINT32)(dividend);
#endif
}

INT32 ctl_sen_util_os_malloc(CTL_SEN_VOS_MEM_INFO *vod_mem_info, UINT32 req_size)
{
	if (vos_mem_init_cma_info(&vod_mem_info->cma_info, VOS_MEM_CMA_TYPE_CACHE, req_size) != 0) {
		ctl_sen_dbg_err("init cma failed\r\n");
		return CTL_SEN_E_NOMEM;
	}

	vod_mem_info->cma_hdl = vos_mem_alloc_from_cma(&vod_mem_info->cma_info);
	if (vod_mem_info->cma_hdl == NULL) {
		ctl_sen_dbg_err("alloc cma failed\r\n");
		return CTL_SEN_E_NOMEM;
	}

	return CTL_SEN_E_OK;
}

INT32 ctl_sen_util_os_mfree(CTL_SEN_VOS_MEM_INFO *vod_mem_info)
{
	if (vos_mem_release_from_cma(vod_mem_info->cma_hdl) != 0) {
		ctl_sen_dbg_err("release cma failed\r\n");
		return CTL_SEN_E_NOMEM;
	}
	return CTL_SEN_E_OK;
}

void *ctl_sen_util_os_malloc_wrap(UINT32 want_size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(want_size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(want_size);
#endif

	if (p_buf == NULL) {
		ctl_sen_dbg_err("aloct %d bytes fail\r\n", want_size);
	} else {
		memset(p_buf, 0, want_size);
	}

	return p_buf;
}

void ctl_sen_util_os_mfree_wrap(void *p_buf)
{
	if (p_buf == NULL) {
		return;
	}
#if defined(__LINUX)
	kfree(p_buf);
#elif defined(__FREERTOS)
	free(p_buf);
#endif
}

INT32 ctl_sen_conv_senmode(CTL_SEN_MAP_HDL *map_hdl, CTL_SEN_MODE senmode_in, CTL_SEN_MODE *senmode_sendrv)
{
	INT32 rt = CTL_SEN_E_IN_PARAM;
	UINT32 mfr_ver = 0, frm_idx;
	CTL_SEN_DRV_TAB *sendrv_tbl = map_hdl->chip_hdl->sendrv->drv_tab;

	sendrv_tbl->get_cfg(conv_senid(map_hdl), CTL_SENDRV_CFGID_GET_MFR_VER, (void *)&mfr_ver);

	if (mfr_ver == 1) {
		frm_idx = map_hdl->frm_idx; // TODO : verify
	} else {
		frm_idx = 0;
	}

	if (senmode_in == CTL_SEN_MODE_CUR) {
		if (map_hdl->chip_hdl->sen_mode != CTL_SEN_MAP_NULL) {
			*senmode_sendrv = (map_hdl->chip_hdl->sen_mode | (frm_idx << CTL_SEN_MODE_FRMIDX_OFS));
			rt = CTL_SEN_E_OK;
		} else {
			ctl_sen_dbg_wrn("sen_id=%d no cur mode\r\n", map_hdl->sen_id);
		}
	} else if (senmode_in == CTL_SEN_MODE_PWR) {
		*senmode_sendrv = senmode_in;
		rt = CTL_SEN_E_OK;
	} else {
		CTL_SENDRV_GET_ATTR_BASIC_PARAM sendrv_param;

		memset((void *)&sendrv_param, 0, sizeof(CTL_SENDRV_GET_ATTR_BASIC_PARAM));
		if (sendrv_tbl->get_cfg(conv_senid(map_hdl), CTL_SENDRV_CFGID_GET_ATTR_BASIC, &sendrv_param) == E_OK) {
			if (senmode_in < CTL_SEN_MODE_1 + sendrv_param.max_senmode) {
				*senmode_sendrv = (senmode_in | (frm_idx << CTL_SEN_MODE_FRMIDX_OFS));
				rt = CTL_SEN_E_OK;
			} else {
				ctl_sen_dbg_wrn("sen_id=%d senmode ovfl (in=%d,max=%d)\r\n", map_hdl->sen_id, senmode_in, CTL_SEN_MODE_1 + sendrv_param.max_senmode);
			}
		}
	}

	if (rt != CTL_SEN_E_OK) {
		ctl_sen_dbg_err("sen_id=%d fail\r\n", map_hdl->sen_id)
	}

	return rt;
}

UINT32 ctl_sen_conv_senid(CTL_SEN_MAP_HDL *map_hdl)
{
	if ((map_hdl == NULL) || (map_hdl->chip_hdl == NULL)) {
		ctl_sen_dbg_wrn("input null\r\n");
		return 0;
	}

	return (map_hdl->chip_hdl->is_ad == 0) ? map_hdl->chip_id : map_hdl->sen_id;
}

INT32 ctl_sen_util_setcfg_status(CTL_SEN_CFG cfg)
{
	INT32 status = CTL_SEN_STATUS_SET;
	switch (cfg) {
	case CTL_SEN_CFG_EXPT:
	case CTL_SEN_CFG_GAIN:
	case CTL_SEN_CFG_FPS:
	case CTL_SEN_CFG_FLIP:
		status = CTL_SEN_STATUS_SET_REG;
		break;
	default:
		break;
	}
//	ctl_sen_dbg_ind("status(0x%x/0x%x)\r\n", status, (CTL_SEN_CFG_SET_WITH_REG & FLGPTN_BIT(cfg) ? CTL_SEN_STATUS_SET_REG : CTL_SEN_STATUS_SET));
	return status;
}

INT32 ctl_sen_util_getcfg_status(CTL_SEN_CFG cfg)
{
	INT32 status = CTL_SEN_STATUS_GET;
	switch (cfg) {
	case CTL_SEN_CFG_EXPT:
	case CTL_SEN_CFG_GAIN:
	case CTL_SEN_CFG_FPS:
	case CTL_SEN_CFG_FLIP:
	case CTL_SEN_CFG_PLUG:
		status = CTL_SEN_STATUS_GET_REG;
		break;
	default:
		break;
	}
//	ctl_sen_dbg_ind("status(0x%x/0x%x)\r\n", status, (CTL_SEN_CFG_GET_WITH_REG & FLGPTN_BIT(cfg) ? CTL_SEN_STATUS_GET_REG : CTL_SEN_STATUS_GET));
	return status;
}

CHAR *ctl_sen_util_get_drvdev_str(CTL_SEN_DRVDEV drvdev)
{
	static CHAR str_buf[20] = {0};

	snprintf(str_buf, 20, "%s %s %s"
		, CTL_SEN_DRVDEV_MASK_CSI(drvdev) ? "CSI" : ""
		, CTL_SEN_DRVDEV_MASK_LVDS(drvdev) ? "LVDS" : ""
		, CTL_SEN_DRVDEV_MASK_TGE(drvdev) ? "TGE" : "");

	return str_buf;
}

BOOL ctl_sen_util_is_combo_sensor(CTL_SEN_MAP_HDL *map_hdl)
{
	return (map_hdl->chip_hdl->init_cfg_obj.drvdev2 != 0);
}