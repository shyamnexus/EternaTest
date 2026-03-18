#if defined(__KERNEL__)
#include <linux/kernel.h>
#else
#include <stdio.h>
#endif
#include "kwrap/type.h"
#include "vpe_dbg.h"
#include "vpet_api.h"

#define MAX_CNT 50
static UINT32 vpe_dbg_msg[VPE_ID_MAX_NUM] = {0};
static UINT32 vpe_ioc_control;
static UINT32 err_msg_cnt;
static UINT32 wrn_msg_cnt;
static CHAR ioc_item_name[64];

UINT32 vpe_dbg_get_dbg_mode(VPE_ID id)
{
	if (id >= VPE_ID_MAX_NUM) {
		return vpe_dbg_msg[0];
	}

	return vpe_dbg_msg[id];
}

void vpe_dbg_set_dbg_mode(VPE_ID id, UINT32 cmd)
{
	UINT32 i;

	if (id >= VPE_ID_MAX_NUM) {
		for (i = 0; i < VPE_ID_MAX_NUM; i++) {
			vpe_dbg_msg[i] = cmd;
		}
	} else {
		vpe_dbg_msg[id] = cmd;
	}

	if (cmd == VPE_DBG_NONE) {
		vpe_dbg_clr_err_msg();
		vpe_dbg_clr_wrn_msg();
	}
}

UINT32 vpe_dbg_get_ioc_control(void)
{
	return vpe_ioc_control;
}

void vpe_dbg_set_ioc_control(UINT32 cmd)
{
	vpe_ioc_control = cmd;
}

BOOL vpe_dbg_check_err_msg(BOOL show_dbg_msg)
{
	BOOL rt = TRUE;

	if ((err_msg_cnt < MAX_CNT) && (!show_dbg_msg)) {
		rt = TRUE;
	} else {
		rt = FALSE;
	}
	err_msg_cnt++;

	return rt;
}

void vpe_dbg_clr_err_msg(void)
{
	err_msg_cnt = 0;
}

UINT32 vpe_dbg_get_err_msg(void)
{
	return err_msg_cnt;
}

BOOL vpe_dbg_check_wrn_msg(BOOL show_dbg_msg)
{
	BOOL rt = TRUE;

	if ((wrn_msg_cnt < MAX_CNT) && (!show_dbg_msg)) {
		rt = TRUE;
	} else {
		rt = FALSE;
	}
	wrn_msg_cnt++;

	return rt;
}

void vpe_dbg_clr_wrn_msg(void)
{
	wrn_msg_cnt = 0;
}

UINT32 vpe_dbg_get_wrn_msg(void)
{
	return wrn_msg_cnt;
}

CHAR *vpe_dbg_get_vpet_item(UINT32 item)
{
	switch (item) {
		case VPET_ITEM_VERSION: sprintf(ioc_item_name, "VPET_ITEM_VERSION"); break;
		case VPET_ITEM_SIZE_TAB: sprintf(ioc_item_name, "VPET_ITEM_SIZE_TAB"); break;
		case VPET_ITEM_DCE_CTL_PARAM: sprintf(ioc_item_name, "VPET_ITEM_DCE_CTL_PARAM"); break;
		case VPET_ITEM_SHARPEN_PARAM: sprintf(ioc_item_name, "VPET_ITEM_SHARPEN_PARAM"); break;
		case VPET_ITEM_2DLUT_PARAM: sprintf(ioc_item_name, "VPET_ITEM_2DLUT_PARAM"); break;
		case VPET_ITEM_DRT_PARAM: sprintf(ioc_item_name, "VPET_ITEM_DRT_PARAM"); break;
		case VPET_ITEM_DCTG_CTRL: sprintf(ioc_item_name, "VPET_ITEM_DCTG_CTRL"); break;
		case VPET_ITEM_FLIP_ROT_PARAM: sprintf(ioc_item_name, "VPET_ITEM_FLIP_ROT_PARAM"); break;
		case VPET_ITEM_2DLUT_EXPAND_PARAM: sprintf(ioc_item_name, "VPET_ITEM_2DLUT_EXPAND_PARAM"); break;
		case VPET_ITEM_DCOUT_MODE_PARAM: sprintf(ioc_item_name, "VPET_ITEM_DCOUT_MODE_PARAM"); break;
		default: sprintf(ioc_item_name, "vpet item (%d)", (int)item); break;
	}

	return ioc_item_name;
}

