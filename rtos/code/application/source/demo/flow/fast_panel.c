#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/type.h>
#include <kwrap/perf.h>
#include <kwrap/cpu.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include "prjcfg.h"
#include "sys_fwload.h"
#include "sys_fastboot.h"
#include "sys_linuxboot.h"
#include "flow_boot_linux.h"
#include <kwrap/debug.h>
#include "vendor_videocapture.h"
#include "isp_api.h"
#include "sen_inc.h"
#include "kwrap/util.h"
#include <dispdevctrl.h>
#include <plat/nand.h>
#include "sys_storage_partition.h"
#include <MemCheck.h>
#include "kdrv_audioio/kdrv_audioio.h"
#include "isp_builtin.h" // NOTE: Include ISP builtin


#if !defined(_disp_off_)
#include "dispdev_ioctrl.h"
ER fast_dispdev1_ioctrl(DISPDEV_IOCTRL_OP disp_dev_ctrl, PDISPDEV_IOCTRL_PARAM p_disp_dev_param)
{
	memset(p_disp_dev_param, 0, sizeof(DISPDEV_IOCTRL_PARAM));
	switch (disp_dev_ctrl) {
	case DISPDEV_IOCTRL_GET_REG_IF:
		p_disp_dev_param->SEL.GET_REG_IF.lcd_ctrl     = DISPDEV_LCDCTRL_SIF;
		p_disp_dev_param->SEL.GET_REG_IF.ui_sif_ch    = SIF_CH2;
		p_disp_dev_param->SEL.GET_REG_IF.ui_gpio_sen   = 0;
		p_disp_dev_param->SEL.GET_REG_IF.ui_gpio_clk   = 0;
		p_disp_dev_param->SEL.GET_REG_IF.ui_gpio_data  = 0;
		break;

	case DISPDEV_IOCTRL_GET_PANEL_ADJUST:
		p_disp_dev_param->SEL.GET_PANEL_ADJUST.pfp_adjust = NULL;
		break;
	default:
		DBG_IND("unsupported: %d\n", disp_dev_ctrl);
		break;
	}
	return 0;
}

int fast_open_panel(void)
{
#if !(_PACKAGE_DISPLAY_)
	DISPDEV_OBJ *p_dev = dispdev_get_lcd1_dev_obj();
	p_dev->set_dev_io_ctrl((FP)fast_dispdev1_ioctrl);
	p_dev->open();
	return 0;
#else
	int er;
	DISPDEV_PARAM disp_param;
	DISPCTRL_PARAM ctrl_param = {0};
	PDISP_OBJ pdisp_obj = disp_get_display_object(DISP_1);

	if ((er = pdisp_obj->open()) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
	}

	// ide display black screen first
	ctrl_param.SEL.SET_ALL_LYR_EN.disp_lyr  = DISPLAYER_VDO1;
	if ((er = pdisp_obj->disp_ctrl(DISPCTRL_SET_ALL_LYR_EN, &ctrl_param)) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
		return er;
	}
	pdisp_obj->load(FALSE);

	memset(&disp_param, 0, sizeof(DISPDEV_PARAM));
	disp_param.SEL.HOOK_DEVICE_OBJECT.dev_id  = DISPDEV_ID_PANEL;
	disp_param.SEL.HOOK_DEVICE_OBJECT.p_disp_dev_obj = dispdev_get_lcd1_dev_obj();
	if ((er = pdisp_obj->dev_ctrl(DISPDEV_HOOK_DEVICE_OBJECT, &disp_param)) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
	}

	memset(&disp_param, 0, sizeof(DISPDEV_PARAM));
	disp_param.SEL.OPEN_DEVICE.dev_id = DISPDEV_ID_PANEL;
	if ((er = pdisp_obj->dev_ctrl(DISPDEV_OPEN_DEVICE, &disp_param)) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
	}

	return er;
#endif
}
#endif
