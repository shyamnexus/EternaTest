#if defined (__LINUX)
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include "kwrap/stdio.h"
#include <plat/nvt-gpio.h>
#include "kdrv_tge_int_drv.h"
#elif defined (__FREERTOS)
#include "stdio.h"
#include "string.h"
#include "plat/gpio.h"
#endif

#include "kdrv_type.h"
#include "kwrap/type.h"
#include "kwrap/sxcmd.h"
#include "kwrap/cmdsys.h"
#include "kwrap/file.h"
#include "kwrap/cpu.h"
#include "kwrap/util.h"
#include "kwrap/mem.h"
#include "kwrap/perf.h"
//#include "kwrap/error_no.h"
#include "comm/hwclock.h"
#include "tge_platform.h"
#include "tge_lib.h"
#include "kdrv_tge_int_api.h"
#include "kdrv_tge_int_dbg.h"
#include "kdrv_videocapture/kdrv_tge.h"
#include "kdrv_tge_int_platform.h"

#define KDRV_TGE_TEST_CMD 0
#define TGE_690_TEST 0
#define TGE_PINMUX_TEST 1

#if defined(CONFIG_NVT_FPGA_EMULATION) || defined(_NVT_FPGA_)
// Make sure do the following steps before set KDRV_TGE_TEST_SIE to 1
// 1. #if 1 to sie_setMode in sie_lib.c
// 2. add EXPORT_SYMBOL(sie_setMode); in sie_lib.c
#define KDRV_TGE_TEST_SIE 0		// modify depend on whether fpgacode include both tge and sie
#else
#define KDRV_TGE_TEST_SIE 0 //check 1->0
#endif

unsigned int kdrv_tge_debug_level = NVT_DBG_WRN;

#if KDRV_TGE_TEST_CMD
#if defined (__LINUX)
#elif defined (__FREERTOS)
#include "pll.h"
#endif
#include "comm/hwclock.h"
#include "kwrap/spinlock.h"
#include "kwrap/task.h"
#include "plat/top.h"
#include "pll_protected.h"
//#include "siep_lib.h"
//#include "sie_lib.h"

/*
For direct mode test. Make sure do the following steps
1. enable CTL_SIE_TEST_CMD in ctl_sie_api.h
2. enable CTL_SIE_TEST_DIRECT_FLOW in ctl_sie_api.h
*/
#include "sie_eng.h"
#include "sie_eng_base.h"

// TGE hardware defined IO
#define KDRV_TGE_SN_MCLK_OUT_GPIO				S_GPIO(0)	// J12.5
#define KDRV_TGE_SIE_VD_OUT_GPIO				S_GPIO(3)	// sync with sie_isr() tge verify setting

#define KDRV_TGE_SN1XVS_OUT_GPIO 				S_GPIO(4)	// J13.1
#define KDRV_TGE_SN1XHS_OUT_GPIO 				S_GPIO(5)	// J13.2

#define KDRV_TGE_SN2XVS_OUT_GPIO 				S_GPIO(7)	// J13.4
#define KDRV_TGE_SN2XHS_OUT_GPIO 				S_GPIO(8)	// J13.5

#define KDRV_TGE_SN3XVS_OUT_GPIO 				S_GPIO(9)	// J13.6
#define KDRV_TGE_SN3XHS_OUT_GPIO 				S_GPIO(10)	// J13.7

#define KDRV_TGE_SN4XVS_OUT_GPIO 				S_GPIO(17)	// J14.6
#define KDRV_TGE_SN4XHS_OUT_GPIO 				S_GPIO(18)	// J14.7

#define KDRV_TGE_FLSH_EXT_TRIG_IN_GPIO 			S_GPIO(25)	// J15.6
#define KDRV_TGE_FLSH_OUT_GPIO 					S_GPIO(26)	// J15.7
#define KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO 	S_GPIO(24)	// J15.5, connect to KDRV_TGE_FLSH_EXT_TRIG_IN_GPIO

#define KDRV_TGE_MCLK_FREQ	27000000
#define KDRV_TGE_MCLK_TIME_MS2CNT(ms) ((KDRV_TGE_MCLK_FREQ) / 1000 * (ms))

#define KDRV_TGE_ID(ch) KDRV_DEV_ID(KDRV_CHIP0, KDRV_VDOCAP_TGE_ENGINE0, (ch))
#define KDRV_TGE_CH_BIT(ch) (1 << (ch))

THREAD_HANDLE g_nvt_kdrv_tge_tsk_id;

typedef enum {
	NVT_KDRV_TGE_TSK_ITEM_NONE = 0,
	NVT_KDRV_TGE_TSK_ITEM_FLSH_IMD,
	NVT_KDRV_TGE_TSK_ITEM_FLSH_WAT_VD,
	NVT_KDRV_TGE_TSK_ITEM_FLSH_EXT_WAT_VD,

	NVT_KDRV_TGE_TSK_ITEM_MSH_IMD,
	NVT_KDRV_TGE_TSK_ITEM_MSH_WAT_VD,
	NVT_KDRV_TGE_TSK_ITEM_MSH_EXT_WAT_VD,
} NVT_KDRV_TGE_TSK_ITEM;
NVT_KDRV_TGE_TSK_ITEM g_nvt_kdrv_tge_tsk_item = NVT_KDRV_TGE_TSK_ITEM_NONE;

typedef enum {
	NVT_KDRV_TGE_OPEN_NONE = 0,
	NVT_KDRV_TGE_OPEN_VDHD = (1 << 0),
	NVT_KDRV_TGE_OPEN_FLSH = (1 << 1),
	NVT_KDRV_TGE_OPEN_MSH = (1 << 2),
	NVT_KDRV_TGE_OPEN_ITEM_MAX = 3,
} NVT_KDRV_TGE_OPEN_ITEM;
static NVT_KDRV_TGE_OPEN_ITEM g_nvt_kdrv_tge_opened = NVT_KDRV_TGE_OPEN_NONE;

#if KDRV_TGE_TEST_SIE
void nvt_kdrv_tge_sie_isr_cb(UINT32 uiIntpStatus, SIE_ENGINE_STATUS_INFO_CB *info)
{
	// sync with sie_isr() tge verify setting

	if (uiIntpStatus & SIE_INT_VD) {
		gpio_set_value(KDRV_TGE_SIE_VD_OUT_GPIO, 1);
	}

	if (uiIntpStatus & SIE_INT_BP3) {
		gpio_set_value(KDRV_TGE_SIE_VD_OUT_GPIO, 0);
	}
}
#endif

ER nvt_kdrv_tge_sie_open(void)
{
	{
		// set clock source

#if defined (__LINUX)
		struct clk *parent_clk = NULL, *src_clk = NULL;

		parent_clk = clk_get(NULL, "pll5");
		if (IS_ERR(parent_clk)) {
			KDRV_TGE_ERR("get parent clk fail\r\n");
			return E_SYS;
		}
		clk_set_rate(parent_clk, KDRV_TGE_MCLK_FREQ);

		src_clk = clk_get(NULL, "sn_mclk1");
		if (IS_ERR(src_clk)) {
			KDRV_TGE_ERR("get mclk1 src clk fail\r\n");
			return E_SYS;
		}
		clk_set_parent(src_clk, parent_clk);
		clk_set_rate(src_clk, KDRV_TGE_MCLK_FREQ);
		clk_prepare_enable(src_clk);
		clk_put(src_clk);

		src_clk = clk_get(NULL, "sn_mclk2");
		if (IS_ERR(src_clk)) {
			KDRV_TGE_ERR("get mclk2 src clk fail\r\n");
			return E_SYS;
		}
		clk_set_parent(src_clk, parent_clk);
		clk_set_rate(src_clk, KDRV_TGE_MCLK_FREQ);
		clk_prepare_enable(src_clk);
		clk_put(src_clk);

		src_clk = clk_get(NULL, "sn_mclk3");
		if (IS_ERR(src_clk)) {
			KDRV_TGE_ERR("get mclk3 src clk fail\r\n");
			return E_SYS;
		}
		clk_set_parent(src_clk, parent_clk);
		clk_set_rate(src_clk, KDRV_TGE_MCLK_FREQ);
		clk_prepare_enable(src_clk);
		clk_put(src_clk);

		src_clk = clk_get(NULL, "sn_mclk4");
		if (IS_ERR(src_clk)) {
			KDRV_TGE_ERR("get mclk4 src clk fail\r\n");
			return E_SYS;
		}
		clk_set_parent(src_clk, parent_clk);
		clk_set_rate(src_clk, KDRV_TGE_MCLK_FREQ);
		clk_prepare_enable(src_clk);
		clk_put(src_clk);

		clk_put(parent_clk);

#elif defined (__FREERTOS)
		pll_setPLLEn(PLL_ID_5, TRUE);
		pll_set_pll_freq(PLL_ID_5, KDRV_TGE_MCLK_FREQ);

		pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC, PLL_CLKSEL_SIE_MCLKSRC_PLL5);
		pll_setClockFreq(SIEMCLK_FREQ, KDRV_TGE_MCLK_FREQ);
		pll_enableClock(SIE_MCLK);

		pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC, PLL_CLKSEL_SIE_MCLK2SRC_PLL5);
		pll_setClockFreq(SIEMCLK2_FREQ, KDRV_TGE_MCLK_FREQ);
		pll_enableClock(SIE_MCLK2);

		pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC, PLL_CLKSEL_SIE_MCLK3SRC_PLL5);
		pll_setClockFreq(SIEMCLK3_FREQ, KDRV_TGE_MCLK_FREQ);
		pll_enableClock(SIE_MCLK3);

		pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC, PLL_CLKSEL_SIE_MCLK4SRC_PLL5);
		pll_setClockFreq(SIEMCLK4_FREQ, KDRV_TGE_MCLK_FREQ);
		pll_enableClock(SIE_MCLK4);
#endif

		// set test gpio
		gpio_direction_output(KDRV_TGE_SIE_VD_OUT_GPIO, 0);
	}
#if KDRV_TGE_TEST_SIE
	if (KDRV_TGE_TEST_SIE) {
		ER sieCtrlStatus;
		SIE_OPENOBJ openObjCB;

		openObjCB.pfSieIsrCb = nvt_kdrv_tge_sie_isr_cb;
		openObjCB.uiSieClockRate = 301 * 1000 * 1000;
		openObjCB.SieClkSel = SIE_CLKSRC_CURR;
		openObjCB.PxClkSel = SIE_PXCLKSRC_MCLK;

		sieCtrlStatus = sie_open(SIE_ENGINE_ID_1, &openObjCB);
		if (sieCtrlStatus != E_OK) {
			KDRV_TGE_DUMP("sie_open Fail\r\n");
			return E_SYS;
		}
	}

	if (KDRV_TGE_TEST_SIE) {
		static SIE_MODE_PARAM ModeParam = {0}; // add static to prevent stack size overflow

		ModeParam.MainInInfo.MainInSrc = MAIN_IN_SELF_PATGEN;

		ModeParam.uiFuncEn = 0;
		ModeParam.uiIntrpEn = SIE_INT_VD|SIE_INT_BP3;

		ModeParam.BreakPointInfo.uiBp3 = 100;

		ModeParam.SrcWinInfo.uiSzX = 1920;
		ModeParam.SrcWinInfo.uiSzY = 1080;

		ModeParam.ActWinInfo.uiStX = 0;
		ModeParam.ActWinInfo.uiStY = 0;
		ModeParam.ActWinInfo.uiSzX = 1920;
		ModeParam.ActWinInfo.uiSzY = 1080;
		ModeParam.ActWinInfo.CfaPat = CFA_R;

		ModeParam.CrpWinInfo.uiStX = 0;
		ModeParam.CrpWinInfo.uiStY = 0;
		ModeParam.CrpWinInfo.uiSzX = 1920;
		ModeParam.CrpWinInfo.uiSzY = 1080;
		ModeParam.CrpWinInfo.CfaPat = CFA_R;

		KDRV_TGE_DUMP("sie_setMode\r\n");
		sie_setMode(SIE_ENGINE_ID_1, &ModeParam);
	}

	if (KDRV_TGE_TEST_SIE) {
		KDRV_TGE_DUMP("sie_start\r\n");
		sie_start(SIE_ENGINE_ID_1);
		sie_waitEvent(SIE_ENGINE_ID_1, SIE_WAIT_VD, TRUE);
		KDRV_TGE_DUMP("1st VD pass \r\n");
		sie_waitEvent(SIE_ENGINE_ID_1, SIE_WAIT_VD, TRUE);
		KDRV_TGE_DUMP("2nd VD pass \r\n");
		sie_waitEvent(SIE_ENGINE_ID_1, SIE_WAIT_VD, TRUE);
		KDRV_TGE_DUMP("3rd VD pass \r\n");
	}
#endif

	return E_OK;
}

ER nvt_kdrv_tge_sie_close(void)
{
#if KDRV_TGE_TEST_SIE
	if (KDRV_TGE_TEST_SIE) {
		sie_pause(SIE_ENGINE_ID_1);
		sie_close(SIE_ENGINE_ID_1);
	}
#endif
	return E_OK;
}

ER nvt_kdrv_tge_pinmux_config(void)
{
	PIN_GROUP_CONFIG pinmux_cfg[1] = {0};

	pinmux_cfg[0].pin_function = PIN_FUNC_SENSORMISC;
	if (nvt_pinmux_capture(pinmux_cfg, 1)) {
		return E_SYS;
	}
	pinmux_cfg[0].config |=
		//PIN_SENSORMISC_CFG_SN_MCLK_1 |		// enable sn_mclk to check mclk speed match KDRV_TGE_MCLK_FREQ
		PIN_SENSORMISC_CFG_SN_XVSXHS_1 |
		PIN_SENSORMISC_CFG_SN2_XVSXHS_1 |
		PIN_SENSORMISC_CFG_SN3_XVSXHS_1 |
		//PIN_SENSORMISC_CFG_SN4_XVSXHS_1 |
		PIN_SENSORMISC_CFG_FLASH_TRIG_IN_1 |
		PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_1 |
		PIN_SENSORMISC_CFG_ME_SHUT_IN_1 |
		PIN_SENSORMISC_CFG_ME_SHUT_OUT_1;
	if (nvt_pinmux_update(pinmux_cfg, 1)) {
		return E_SYS;
	}

	return E_OK;
}

void nvt_kdrv_tge_isr_cb(KDRV_TGE_ISR_EVENT event, void *data)
{
	static UINT32 time1 = 0, event_sum = 0;
	UINT32 time2 = hwclock_get_counter();

	event_sum |= event;

	if (time2 - time1 > 1000000) {
		KDRV_TGE_DUMP("event 0x%x (%3s %3s %3s %3s %3s %3s %3s %3s %3s %3s %8s %8s %8s %8s %8s %8s)\r\n", event_sum,
			(event_sum & KDRV_TGE_INT_VD1) 		? "vd1" : "",
			(event_sum & KDRV_TGE_INT_VD2) 		? "vd2" : "",
			(event_sum & KDRV_TGE_INT_VD3) 		? "vd3" : "",
			(event_sum & KDRV_TGE_INT_VD4) 		? "vd4" : "",
			(event_sum & KDRV_TGE_INT_VD5) 		? "vd5" : "",
			(event_sum & KDRV_TGE_INT_BP1) 		? "bp1" : "",
			(event_sum & KDRV_TGE_INT_BP2) 		? "bp2" : "",
			(event_sum & KDRV_TGE_INT_BP3) 		? "bp3" : "",
			(event_sum & KDRV_TGE_INT_BP4) 		? "bp4" : "",
			(event_sum & KDRV_TGE_INT_BP5) 		? "bp5" : "",
			(event_sum & KDRV_TGE_INT_FLSH_TRG) ? "flsh_trg" : "",
			(event_sum & KDRV_TGE_INT_MSH_CLOSE_TRG) ? "msh_close_trg" : "",
			(event_sum & KDRV_TGE_INT_MSH_OPEN_TRG) ? "msh_open_trg" : "",
			(event_sum & KDRV_TGE_INT_FLSH_END) ? "flsh_end" : "",
			(event_sum & KDRV_TGE_INT_MSH_CLOSE_END) ? "msh_close_end" : "",
			(event_sum & KDRV_TGE_INT_MSH_OPEN_END) ? "msh_open_end" : "");
		time1 = time2;
		event_sum = 0;
	}
}

ER nvt_kdrv_tge_set_vdhd_param(UINT32 ch, UINT32 vp, UINT32 va, UINT32 vf,
	UINT32 hp, UINT32 ha, UINT32 hc, UINT32 vph, UINT32 hph)
{
	KDRV_TGE_VDHD_INFO vdhd_info = {0};

	vdhd_info.mode = KDRV_MODE_MASTER;
	vdhd_info.vd_period = vp;
	vdhd_info.vd_assert = va;
	vdhd_info.vd_frontblnk = vf;
	vdhd_info.hd_period = hp;
	vdhd_info.hd_assert = ha;
	vdhd_info.hd_cnt = hc;
	vdhd_info.vd_phase = vph;
	vdhd_info.hd_phase = hph;

	return kdrv_tge_set(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_VDHD, (void *)(&vdhd_info));
}

ER nvt_kdrv_tge_get_vdhd_param(UINT32 ch)
{
	KDRV_TGE_VDHD_INFO vdhd_info = {0};
	UINT32 _ch;

	if (kdrv_tge_get(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_VDHD, (void *)(&vdhd_info))) {
		return E_SYS;
	}

	for (_ch = 0; _ch < KDRV_TGE_VDHD_CH_MAX; _ch++) {
		if ((1 << _ch) & ch) {
			break;
		}
	}

	KDRV_TGE_DUMP("CH%d info: vd assert/frntbl/cnt/period = %d/%d/%d/%d\r\n", _ch, vdhd_info.vd_assert, vdhd_info.vd_frontblnk, vdhd_info.hd_cnt, vdhd_info.vd_period);
	KDRV_TGE_DUMP("CH%d info: hd assert/period = %d/%d\r\n", _ch, vdhd_info.hd_assert, vdhd_info.hd_period);

	return E_OK;
}

ER nvt_kdrv_tge_set_bp_line(UINT32 ch, UINT32 bl)
{
	KDRV_TGE_BP_INFO bp_info = {0};

	bp_info.bp_line = bl;

	return kdrv_tge_set(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_VD_BP, (void *)(&bp_info));
}

ER nvt_kdrv_tge_get_bp_line(UINT32 ch)
{
	KDRV_TGE_BP_INFO bp_info = {0};
	UINT32 _ch;

	if (kdrv_tge_get(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_VD_BP, (void *)(&bp_info))) {
		return E_SYS;
	}

	for (_ch = 0; _ch < KDRV_TGE_VDHD_CH_MAX; _ch++) {
		if ((1 << _ch) & ch) {
			break;
		}
	}

	KDRV_TGE_DUMP("CH%d bp = %d\r\n", _ch, bp_info.bp_line);

	return E_OK;
}

ER nvt_kdrv_tge_set_clk_src(UINT32 ch, KDRV_TGE_CLK_SRC clk_src)
{
	KDRV_TGE_CLK_SRC_SEL clk_src_info = {0};

	clk_src_info.clk_src_info = clk_src;

	return kdrv_tge_set(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_MCLK_SEL, (void *)(&clk_src_info));
}

ER nvt_kdrv_tge_set_flsh_param(UINT32 ch, KDRV_TGE_CLK_SRC clk_src, UINT64 delay, UINT64 assert, UINT32 period, BOOL ctrl_inv, BOOL ext_trig_inv)
{
	KDRV_TGE_FLSH_INFO flsh_info = {0};

	flsh_info.clk_src = clk_src;
	flsh_info.delay = delay;
	flsh_info.assert = assert;
	flsh_info.period = period;
	flsh_info.ctrl_pin_signal_inv = ctrl_inv;
	flsh_info.ext_trig_pin_signal_inv = ext_trig_inv;


	return kdrv_tge_set(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_FLSH_CTRL, (void *)(&flsh_info));
}

ER nvt_kdrv_tge_get_flsh_param(UINT32 ch)
{
	KDRV_TGE_FLSH_INFO flsh_info = {0};
	UINT32 _ch;

	if (kdrv_tge_get(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_FLSH_CTRL, (void *)(&flsh_info))) {
		return E_SYS;
	}

	for (_ch = 0; _ch < KDRV_TGE_FLSH_ID_MAX; _ch++) {
		if ((1 << _ch) & ch) {
			break;
		}
	}

	KDRV_TGE_DUMP("CH%d clk_src/delay/assert/period = %d/%lld/%lld/%d\r\n", _ch, flsh_info.clk_src, flsh_info.delay, flsh_info.assert, flsh_info.period);

	return E_OK;
}

ER nvt_kdrv_tge_set_flsh_vd_src(UINT32 ch, KDRV_TGE_FLSH_VD_SRC vd_src)
{
	KDRV_TGE_FLSH_VD_SRC_INFO vd_src_info = {0};

	vd_src_info.vd_src = vd_src;

	return kdrv_tge_set(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_FLSH_VD_SRC, (void *)(&vd_src_info));
}

ER nvt_kdrv_tge_get_flsh_vd_src(UINT32 ch)
{
	KDRV_TGE_FLSH_VD_SRC_INFO vd_src_info = {0};
	UINT32 _ch;

	if (kdrv_tge_get(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_FLSH_VD_SRC, (void *)(&vd_src_info))) {  //
		return E_SYS;
	}

	for (_ch = 0; _ch < KDRV_TGE_FLSH_ID_MAX; _ch++) {
		if ((1 << _ch) & ch) {
			break;
		}
	}

	KDRV_TGE_DUMP("CH%d vd_src = %d\r\n", _ch, vd_src_info.vd_src);

	return E_OK;
}

ER nvt_kdrv_tge_set_msh_param(UINT32 ch, KDRV_TGE_CLK_SRC clk_src, UINT64 delay, UINT64 assert, UINT32 period)
{
	KDRV_TGE_MSH_INFO msh_info = {0};

	msh_info.clk_src = clk_src;
	msh_info.delay = delay;
	msh_info.assert = assert;
	msh_info.period = period;

	return kdrv_tge_set(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_MSH_CTRL, (void *)(&msh_info));
}

ER nvt_kdrv_tge_get_msh_param(UINT32 ch)
{
	KDRV_TGE_MSH_INFO msh_info = {0};
	UINT32 _ch;

	if (kdrv_tge_get(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_MSH_CTRL, (void *)(&msh_info))) {
		return E_SYS;
	}

	for (_ch = 0; _ch < KDRV_TGE_MSH_ID_MAX; _ch++) {
		if ((1 << _ch) & ch) {
			break;
		}
	}

	KDRV_TGE_DUMP("CH%d clk_src/delay/assert/period = %d/%lld/%lld/%d\r\n", _ch, msh_info.clk_src, msh_info.delay, msh_info.assert, msh_info.period);

	return E_OK;
}

ER nvt_kdrv_tge_set_msh_pin_ctrl(UINT32 ch, UINT64 ctrl_pin_signal_inv, BOOL swap_ctrl_pin, BOOL ctrl_mode_pinssert, BOOL ext_trig_pin_signal_inv)
{
	KDRV_TGE_MSH_PIN_CTRL msh_ctrl_info = {0};

	msh_ctrl_info.ctrl_pin_signal_inv = ctrl_pin_signal_inv;
	msh_ctrl_info.swap_ctrl_pin = swap_ctrl_pin;
	msh_ctrl_info.ctrl_mode_pin = ctrl_mode_pinssert;
	msh_ctrl_info.ext_trig_pin_signal_inv = ext_trig_pin_signal_inv;

	return kdrv_tge_set(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_MSH_PIN_CTRL, (void *)(&msh_ctrl_info));
}

ER nvt_kdrv_tge_get_msh_pin_ctrl(UINT32 ch)
{
	KDRV_TGE_MSH_PIN_CTRL msh_ctrl_info = {0};
	UINT32 _ch;

	if (kdrv_tge_get(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_MSH_PIN_CTRL, (void *)(&msh_ctrl_info))) {  //check
		return E_SYS;
	}

	for (_ch = 0; _ch < KDRV_TGE_MSH_ID_MAX; _ch++) {
		if ((1 << _ch) & ch) {
			break;
		}
	}

	KDRV_TGE_DUMP("CH%d msh_ctrl_inv/swap/ctrl_mode/ext_trg_inv = %d/%d/%d/%d\r\n", _ch, msh_ctrl_info.ctrl_pin_signal_inv, msh_ctrl_info.swap_ctrl_pin, msh_ctrl_info.ctrl_mode_pin, msh_ctrl_info.ext_trig_pin_signal_inv);

	return E_OK;
}

ER nvt_kdrv_tge_set_msh_vd_src(UINT32 ch, KDRV_TGE_MSH_VD_SRC vd_src)
{
	KDRV_TGE_MSH_VD_SRC_INFO vd_src_info = {0};

	vd_src_info.vd_src = vd_src;

	return kdrv_tge_set(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_MSH_VD_SRC, (void *)(&vd_src_info));
}

ER nvt_kdrv_tge_get_msh_vd_src(UINT32 ch)
{
	KDRV_TGE_MSH_VD_SRC_INFO vd_src_info = {0};
	UINT32 _ch;

	if (kdrv_tge_get(KDRV_TGE_ID(ch), KDRV_TGE_PARAM_IPL_MSH_VD_SRC, (void *)(&vd_src_info))) {
		return E_SYS;
	}

	for (_ch = 0; _ch < KDRV_TGE_MSH_ID_MAX; _ch++) {
		if ((1 << _ch) & ch) {
			break;
		}
	}

	KDRV_TGE_DUMP("CH%d vd_src = %d\r\n", _ch, vd_src_info.vd_src);

	return E_OK;
}

ER nvt_kdrv_tge_trig_vd_hd(UINT32 ch, UINT32 wait_event)
{
	KDRV_TGE_TRIG_INFO trig_info = {0};

	trig_info.trig_type = KDRV_TGE_TRIG_VDHD;
	trig_info.ch_enable = TRUE;
	trig_info.wait_end_enable = TRUE;
	trig_info.wait_event = wait_event;

	return kdrv_tge_trigger(KDRV_TGE_ID(ch), NULL, NULL, (void *)&trig_info);
}

ER nvt_kdrv_tge_trig_vd_hd_stop(void)
{
	KDRV_TGE_TRIG_INFO trig_info = {0};

	trig_info.trig_type = KDRV_TGE_TRIG_VDHD_STOP;
	return kdrv_tge_trigger(KDRV_TGE_ID(0), NULL, NULL, (void *)&trig_info);
}

ER nvt_kdrv_tge_trig_flsh(UINT32 ch, KDRV_TGE_TRIG_TYPE type)
{
	KDRV_TGE_TRIG_INFO trig_info = {0};

	trig_info.trig_type = type;
	return kdrv_tge_trigger(KDRV_TGE_ID(ch), NULL, NULL, (void *)&trig_info);
}

ER nvt_kdrv_tge_trig_flsh_stop(UINT32 ch)
{
	KDRV_TGE_TRIG_INFO trig_info = {0};

	trig_info.trig_type = KDRV_TGE_TRIG_FLSH_STOP;
	return kdrv_tge_trigger(KDRV_TGE_ID(ch), NULL, NULL, (void *)&trig_info);
}

ER nvt_kdrv_tge_trig_msh_stop(UINT32 ch)
{
	KDRV_TGE_TRIG_INFO trig_info = {0};

	trig_info.trig_type = KDRV_TGE_TRIG_MSH_STOP;
	return kdrv_tge_trigger(KDRV_TGE_ID(ch), NULL, NULL, (void *)&trig_info);
}

THREAD_DECLARE(nvt_kdrv_tge_task, p1)  // check multiple trig
{
	while (!THREAD_SHOULD_STOP) {
		if (g_nvt_kdrv_tge_tsk_item == NVT_KDRV_TGE_TSK_ITEM_FLSH_IMD) {
			// pull low test gpio (use ext trigger signal)
			gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
			vos_util_delay_ms(10);

			// trigger flashlight immediately
			if (nvt_kdrv_tge_trig_flsh(KDRV_TGE_FLSH_1, KDRV_TGE_TRIG_FLSH_IMD)) {
				KDRV_TGE_ERR("trig flsh imd fail\r\n");
			}

			// pull high test gpio (use ext trigger signal)
			gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		} else if (g_nvt_kdrv_tge_tsk_item == NVT_KDRV_TGE_TSK_ITEM_FLSH_WAT_VD) {
			// pull low test gpio (use ext trigger signal)
			gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
			vos_util_delay_ms(10);

			// trigger flashlight wait vd
			if (nvt_kdrv_tge_trig_flsh(KDRV_TGE_FLSH_1, KDRV_TGE_TRIG_FLSH_WAT_VD)) {
				KDRV_TGE_ERR("trig flsh wait vd fail\r\n");
			}

			// pull high test gpio (use ext trigger signal)
			gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		} else if (g_nvt_kdrv_tge_tsk_item == NVT_KDRV_TGE_TSK_ITEM_FLSH_EXT_WAT_VD) {
			// pull low flashlight ext gpio
			gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

			// trigger flashlight wait vd ext trigger
			if (nvt_kdrv_tge_trig_flsh(KDRV_TGE_FLSH_1, KDRV_TGE_TRIG_FLSH_WAT_EXT)) {
				KDRV_TGE_ERR("trig flsh ext wait vd fail\r\n");
			}

			// wait vd load parameter
			vos_util_delay_ms(50);

			// pull high flashlight ext gpio
			gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);

			vos_util_delay_ms(200);
		}

		if (g_nvt_kdrv_tge_tsk_item == NVT_KDRV_TGE_TSK_ITEM_MSH_IMD) {
			// pull low test gpio (use ext trigger signal)
			//gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);  //check
			vos_util_delay_ms(10);

			// trigger Mechanical-Shutter immediately
			if (nvt_kdrv_tge_trig_flsh(KDRV_TGE_MSH_1_CLOSE, KDRV_TGE_TRIG_MSH_IMD)) {
				KDRV_TGE_ERR("trig msh imd fail\r\n");
			}

			// pull high test gpio (use ext trigger signal)
			//gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		} else if (g_nvt_kdrv_tge_tsk_item == NVT_KDRV_TGE_TSK_ITEM_MSH_WAT_VD) {
			// pull low test gpio (use ext trigger signal)
			//gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
			vos_util_delay_ms(10);

			// trigger Mechanical-Shutter wait vd
			if (nvt_kdrv_tge_trig_flsh(KDRV_TGE_MSH_1_CLOSE, KDRV_TGE_TRIG_MSH_WAT_VD)) {
				KDRV_TGE_ERR("trig msh wait vd fail\r\n");
			}

			// pull high test gpio (use ext trigger signal)
			//gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		} else if (g_nvt_kdrv_tge_tsk_item == NVT_KDRV_TGE_TSK_ITEM_MSH_EXT_WAT_VD) {
			// pull low flashlight ext gpio
			//gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

			// trigger Mechanical-Shutter wait vd ext trigger
			if (nvt_kdrv_tge_trig_flsh(KDRV_TGE_MSH_1_CLOSE, KDRV_TGE_TRIG_MSH_WAT_EXT)) {
				KDRV_TGE_ERR("trig msh ext wait vd fail\r\n");
			}

			// wait vd load parameter
			vos_util_delay_ms(50);

			// pull high Mechanical-Shutter ext gpio
			//gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);

			vos_util_delay_ms(200);
		}

		vos_util_delay_ms(100);
	}

	THREAD_RETURN(0);
}

ER nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_ITEM open_item)
{
	UINT32 i;

	// check if open_item is already opened
	for (i = 0; i < NVT_KDRV_TGE_OPEN_ITEM_MAX; i++) {
		if (((1 << i) & open_item) && ((1 << i) & g_nvt_kdrv_tge_opened)) {
			KDRV_TGE_ERR("re-open 0x%x\r\n", open_item);
			goto fail;
		}
	}

	// set sie to send VD at first open
	if ((g_nvt_kdrv_tge_opened == NVT_KDRV_TGE_OPEN_NONE) && nvt_kdrv_tge_sie_open()) {
		goto fail;
	}

#if (TGE_PINMUX_TEST == 1)
	// set pinmux to measure at first open
	if ((g_nvt_kdrv_tge_opened == NVT_KDRV_TGE_OPEN_NONE) && nvt_kdrv_tge_pinmux_config()) {
		goto fail;
	}
#endif

	// kdrv open
	if (kdrv_tge_open(KDRV_CHIP0, KDRV_VDOCAP_TGE_ENGINE0)) {
		goto fail;
	}

	// set isr callback at first open
	if ((g_nvt_kdrv_tge_opened == NVT_KDRV_TGE_OPEN_NONE) && kdrv_tge_set(KDRV_TGE_ID(0), KDRV_TGE_PARAM_IPL_SET_ISR_CB, nvt_kdrv_tge_isr_cb)) {
		goto fail;
	}

	// create trigger task at first open flash
	if ((open_item & NVT_KDRV_TGE_OPEN_FLSH) && (g_nvt_kdrv_tge_opened & NVT_KDRV_TGE_OPEN_FLSH) == 0) {
		g_nvt_kdrv_tge_tsk_item = NVT_KDRV_TGE_TSK_ITEM_NONE;
		THREAD_CREATE(g_nvt_kdrv_tge_tsk_id, nvt_kdrv_tge_task, NULL, "nvt_kdrv_tge_task");
		if (g_nvt_kdrv_tge_tsk_id == 0) {
			KDRV_TGE_ERR("create task fail\r\n");
			goto fail;
		}
		THREAD_SET_PRIORITY(g_nvt_kdrv_tge_tsk_id, 10);
		THREAD_RESUME(g_nvt_kdrv_tge_tsk_id);
	}

	// create trigger task at first open msh
	if ((open_item & NVT_KDRV_TGE_OPEN_MSH) && (g_nvt_kdrv_tge_opened & NVT_KDRV_TGE_OPEN_MSH) == 0) {
		g_nvt_kdrv_tge_tsk_item = NVT_KDRV_TGE_TSK_ITEM_NONE;
		THREAD_CREATE(g_nvt_kdrv_tge_tsk_id, nvt_kdrv_tge_task, NULL, "nvt_kdrv_tge_task");
		if (g_nvt_kdrv_tge_tsk_id == 0) {
			KDRV_TGE_ERR("create task fail\r\n");
			goto fail;
		}
		THREAD_SET_PRIORITY(g_nvt_kdrv_tge_tsk_id, 10);
		THREAD_RESUME(g_nvt_kdrv_tge_tsk_id);
	}

	g_nvt_kdrv_tge_opened |= open_item;

	return E_OK;

fail:
	return E_SYS;
}

ER nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_ITEM open_item)
{
	// stop task at close flash
	if (((open_item & NVT_KDRV_TGE_OPEN_FLSH) && (g_nvt_kdrv_tge_opened & NVT_KDRV_TGE_OPEN_FLSH)) || ((open_item & NVT_KDRV_TGE_OPEN_MSH) && (g_nvt_kdrv_tge_opened & NVT_KDRV_TGE_OPEN_MSH))) {
		THREAD_DESTROY(g_nvt_kdrv_tge_tsk_id);
	}

	// kdrv tge close
	if (kdrv_tge_close(KDRV_CHIP0, KDRV_VDOCAP_TGE_ENGINE0)) {
		goto fail;
	}

	// sie close at last close
	if (((g_nvt_kdrv_tge_opened & (~open_item)) == 0) && nvt_kdrv_tge_sie_close()) {
		goto fail;
	}

	g_nvt_kdrv_tge_opened &= (~open_item);

	return E_OK;

fail:
	return E_SYS;
}

BOOL nvt_kdrv_tge_cmd_test_vdhd(unsigned char argc, char **pargv)
{
	//											2700000 = 0x2932E0	5400000 = 0x5265C0	8100000 = 0x7B98A0	10800000 = 0xA4CB80		13500000 = 0xCDFE60
	UINT32 vd_period_ms[KDRV_TGE_VDHD_CH_MAX] = { 100, 				200, 				300, 				400/*, 					500*/ };
	//											891000 = 0xD9878	1782000 = 0x1B30F0	2673000 = 0x28C968	3564000 = 0x3661E0		4455000 = 0x43FA58
	UINT32 vd_assert_ms[KDRV_TGE_VDHD_CH_MAX] = { 33,  				66,  				100, 				133/*, 					166*/ };
	//											891000 = 0xD9878	1782000 = 0x1B30F0	2673000 = 0x28C968	3564000 = 0x3661E0		4455000 = 0x43FA58
	UINT32 vd_pb_line_ms[KDRV_TGE_VDHD_CH_MAX] = { 33,  			66,  				100, 				133/*, 					166*/ };
	//											54000 = 0xD2F0		54000 = 0xD2F0		54000 = 0xD2F0		54000 = 0xD2F0			54000 = 0xD2F0
	UINT32 hd_period_ms[KDRV_TGE_VDHD_CH_MAX] = { 2, 				2, 					2, 					2/*, 						2*/ };
	//											27000 = 0x6978		27000 = 0x6978		27000 = 0x6978		27000 = 0x6978			27000 = 0x6978
	UINT32 hd_assert_ms[KDRV_TGE_VDHD_CH_MAX] = { 1, 				1, 					1, 					1/*, 						1*/ };

	UINT32 i, ch = 0, wait_event = 0;
	static int already_start = 0;

	///////////////// start /////////////////
	if (already_start == 0) {
		KDRV_TGE_DUMP("VDHD test start ...\r\n");

		// start config tge
		if (nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_VDHD)) {
			goto fail;
		}

		// set all vdhd ch param
		for (i = 0; i < KDRV_TGE_VDHD_CH_MAX; i++) {
			if (nvt_kdrv_tge_set_vdhd_param((KDRV_TGE_VDHD_CH1 << i), KDRV_TGE_MCLK_TIME_MS2CNT(vd_period_ms[i]), KDRV_TGE_MCLK_TIME_MS2CNT(vd_assert_ms[i]), 0,
				KDRV_TGE_MCLK_TIME_MS2CNT(hd_period_ms[i]), KDRV_TGE_MCLK_TIME_MS2CNT(hd_assert_ms[i]), 0, KDRV_TGE_PHASE_RISING, KDRV_TGE_PHASE_RISING)) {
				goto fail;
			}

			if (nvt_kdrv_tge_set_bp_line((KDRV_TGE_VDHD_CH1 << i), KDRV_TGE_MCLK_TIME_MS2CNT(vd_pb_line_ms[i]))) {
				goto fail;
			}

			if (nvt_kdrv_tge_set_clk_src((KDRV_TGE_VDHD_CH1 << i), KDRV_TGE_CLK_MCLK1+i)) {
				goto fail;
			}
		}

		// get all vdhd ch param
		for (i = 0; i < KDRV_TGE_VDHD_CH_MAX; i++) {
			if (nvt_kdrv_tge_get_vdhd_param((KDRV_TGE_VDHD_CH1 << i))) {
				goto fail;
			}

			if (nvt_kdrv_tge_get_bp_line((KDRV_TGE_VDHD_CH1 << i))) {
				goto fail;
			}
		}

		// set all vdhd ch wait vd
		for (i = 0; i < KDRV_TGE_VDHD_CH_MAX; i++) {
			ch |= (KDRV_TGE_VDHD_CH1 << i);
			//wait_event |= ((TGE_WAIT_VD << i) | (TGE_WAIT_VD_BP1 << i));  //check
		}

		if (nvt_kdrv_tge_trig_vd_hd(ch, wait_event)) {
			goto fail;
		}

		already_start = 1;

	///////////////// stop /////////////////
	} else {
		// stop trigger vdhd
		if (nvt_kdrv_tge_trig_vd_hd_stop()) {
			goto fail;
		}

		nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_VDHD);

		already_start = 0;
		KDRV_TGE_DUMP("VDHD test done\r\n");
	}

	return TRUE;

fail:
	KDRV_TGE_DUMP("VDHD test fail\r\n");
	return FALSE;
}

BOOL nvt_kdrv_tge_cmd_test_flsh_imd(unsigned char argc, char **pargv)
{
	UINT32 flsh_period[KDRV_TGE_FLSH_ID_MAX] = { 5 };
	UINT64 flsh_assert_ms[KDRV_TGE_FLSH_ID_MAX] = { 30 };
	UINT32 i;
	BOOL ctrl_inv = 0, ext_trg_inv = 0;
	static int already_start = 0;

	///////////////// start /////////////////
	if (already_start == 0) {
		KDRV_TGE_DUMP("Flashlight IMD test start ...\r\n");

		// start config tge
		if (nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_FLSH)) {
			goto fail;
		}

		// set flashlight param
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_flsh_param((KDRV_TGE_FLSH_1 << i), KDRV_TGE_CLK_MCLK1, 0, flsh_assert_ms[i] * 1000, flsh_period[i], ctrl_inv, ext_trg_inv)) {
				goto fail;
			}
		}

		// get flashlight param
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_flsh_param((KDRV_TGE_FLSH_1 << i))) {
				goto fail;
			}
		}

		// set test gpio (use ext trigger signal)
		gpio_direction_output(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

		// mark as task trigger start
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);

		g_nvt_kdrv_tge_tsk_item = NVT_KDRV_TGE_TSK_ITEM_FLSH_IMD;

		already_start = 1;

	///////////////// stop /////////////////
	} else {
		// stop trigger
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_trig_flsh_stop(KDRV_TGE_FLSH_1 << i)) {
				goto fail;
			}
		}

		nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_FLSH);

		already_start = 0;
		KDRV_TGE_DUMP("Flashlight IMD test done\r\n");
	}

	return TRUE;

fail:
	KDRV_TGE_DUMP("Flashlight IMD test fail\r\n");
	return FALSE;
}

BOOL nvt_kdrv_tge_cmd_test_flsh_wait_vd(unsigned char argc, char **pargv)
{
	UINT32 flsh_period[KDRV_TGE_FLSH_ID_MAX] = { 5 };
	UINT64 flsh_assert_ms[KDRV_TGE_FLSH_ID_MAX] = { 30 };
	UINT32 i;
	BOOL ctrl_inv, ext_trg_inv;
	static int already_start = 0;

	if (!KDRV_TGE_TEST_SIE) {
		KDRV_TGE_ERR("not support for sie\r\n");
		return FALSE;
	}

	///////////////// start /////////////////
	if (already_start == 0) {
		KDRV_TGE_DUMP("Flashlight WAT VD test start ...\r\n");

		// start config tge
		if (nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_FLSH)) {
			goto fail;
		}

		// set flashlight param
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_flsh_param((KDRV_TGE_FLSH_1 << i), KDRV_TGE_CLK_MCLK1, 0, flsh_assert_ms[i] * 1000, flsh_period[i], ctrl_inv, ext_trg_inv)) {
				goto fail;
			}
		}

		// get flashlight param
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_flsh_param((KDRV_TGE_FLSH_1 << i))) {
				goto fail;
			}
		}

		// set test gpio (use ext trigger signal)
		gpio_direction_output(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

		// mark as task trigger start
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);

		g_nvt_kdrv_tge_tsk_item = NVT_KDRV_TGE_TSK_ITEM_FLSH_WAT_VD;

		already_start = 1;

	///////////////// stop /////////////////
	} else {
		// stop trigger
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_trig_flsh_stop(KDRV_TGE_FLSH_1 << i)) {
				goto fail;
			}
		}

		nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_FLSH);

		already_start = 0;
		KDRV_TGE_DUMP("Flashlight WAT VD test done\r\n");
	}

	return TRUE;

fail:
	KDRV_TGE_DUMP("Flashlight WAT VD test fail\r\n");
	return FALSE;
}

BOOL nvt_kdrv_tge_cmd_test_flsh_ext_wait_vd(unsigned char argc, char **pargv)
{
	UINT32 flsh_period[KDRV_TGE_FLSH_ID_MAX] = { 5 };
	UINT64 flsh_assert_ms[KDRV_TGE_FLSH_ID_MAX] = { 30 };
	UINT32 i;
	BOOL ctrl_inv, ext_trg_inv;
	static int already_start = 0;

	if (!KDRV_TGE_TEST_SIE) {
		KDRV_TGE_ERR("not support for sie\r\n");
		return FALSE;
	}

	///////////////// start /////////////////
	if (already_start == 0) {
		KDRV_TGE_DUMP("Flashlight WAT EXT+VD test start ...\r\n");

		// start config tge
		if (nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_FLSH)) {
			goto fail;
		}

		// set flashlight param
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_flsh_param((KDRV_TGE_FLSH_1 << i), KDRV_TGE_CLK_MCLK1, 0, flsh_assert_ms[i] * 1000, flsh_period[i], ctrl_inv, ext_trg_inv)) {
				goto fail;
			}
		}

		// get flashlight param
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_flsh_param((KDRV_TGE_FLSH_1 << i))) {
				goto fail;
			}
		}

		// set test gpio
		gpio_direction_output(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

		// mark as task trigger start
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);

		g_nvt_kdrv_tge_tsk_item = NVT_KDRV_TGE_TSK_ITEM_FLSH_EXT_WAT_VD;

		already_start = 1;

	///////////////// stop /////////////////
	} else {
		// stop trigger
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_trig_flsh_stop(KDRV_TGE_FLSH_1 << i)) {
				goto fail;
			}
		}

		nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_FLSH);

		already_start = 0;
		KDRV_TGE_DUMP("Flashlight WAT EXT+VD test done\r\n");
	}

	return TRUE;

fail:
	KDRV_TGE_DUMP("Flashlight WAT EXT+VD test fail\r\n");
	return FALSE;
}

BOOL nvt_kdrv_tge_cmd_test_flsh_cont_wait_vd(unsigned char argc, char **pargv)
{
	UINT32 flsh_period[KDRV_TGE_FLSH_ID_MAX] = { 5 };
	UINT64 flsh_assert_ms[KDRV_TGE_FLSH_ID_MAX] = { 30 };
	UINT32 i;
	BOOL ctrl_inv, ext_trg_inv;
	static int already_start = 0;

	if (!KDRV_TGE_TEST_SIE) {
		KDRV_TGE_ERR("not support for sie\r\n");
		return FALSE;
	}

	///////////////// start /////////////////
	if (already_start == 0) {
		KDRV_TGE_DUMP("Flashlight WAT CONT+VD test start ...\r\n");

		// start config tge
		if (nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_FLSH)) {
			goto fail;
		}

		// set flashlight param
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_flsh_param((KDRV_TGE_FLSH_1 << i), KDRV_TGE_CLK_MCLK1, 0, flsh_assert_ms[i] * 1000, flsh_period[i], ctrl_inv, ext_trg_inv)) {
				goto fail;
			}
		}

		// get flashlight param
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_flsh_param((KDRV_TGE_FLSH_1 << i))) {
				goto fail;
			}
		}

		// set test gpio (use ext trigger signal)
		gpio_direction_output(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

		// mark as task trigger start
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);

		// trigger flashlight wait vd continuous
		if (nvt_kdrv_tge_trig_flsh(KDRV_TGE_FLSH_1, KDRV_TGE_TRIG_FLSH_WAT_CONT)) {
			goto fail;
		}

		already_start = 1;

	///////////////// stop /////////////////
	} else {
		// stop trigger
		for (i = 0; i < KDRV_TGE_FLSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_trig_flsh_stop(KDRV_TGE_FLSH_1 << i)) {
				goto fail;
			}
		}
		nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_FLSH);

		already_start = 0;
		KDRV_TGE_DUMP("Flashlight WAT CONT+VD test done\r\n");
	}

	return TRUE;

fail:
	KDRV_TGE_DUMP("Flashlight WAT CONT+VD test fail\r\n");
	return FALSE;
}

BOOL nvt_kdrv_tge_cmd_test_msh_imd(unsigned char argc, char **pargv)
{
	UINT32 msh_period[KDRV_TGE_MSH_ID_MAX] = { 5, 10 };
	UINT64 msh_assert_ms[KDRV_TGE_MSH_ID_MAX] = { 30, 60 };
	UINT32 i;
	BOOL ctrl_inv = 1, swap = 1, mode = 1, ext_trig_inv = 1;
	static int already_start = 0;

	///////////////// start /////////////////
	if (already_start == 0) {
		KDRV_TGE_DUMP("Mechanical-shutter IMD test start ...\r\n");

		// start config tge
		if (nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_MSH)) {
			goto fail;
		}

		// set Mechanical-shutter param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_msh_param((KDRV_TGE_MSH_1_CLOSE << i), KDRV_TGE_CLK_MCLK1, 0, msh_assert_ms[i] * 1000, msh_period[i])) {
				goto fail;
			}
		}

		// get Mechanical-Shutter param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_msh_param((KDRV_TGE_MSH_1_CLOSE << i))) {
				goto fail;
			}
		}

		// set Mechanical-shutter pin ctrl param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_msh_pin_ctrl((KDRV_TGE_MSH_1_CLOSE << i), ctrl_inv, swap, mode, ext_trig_inv)) {
				goto fail;
			}
		}

		// get Mechanical-Shutter pin ctrl param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_msh_pin_ctrl((KDRV_TGE_MSH_1_CLOSE << i))) {
				goto fail;
			}
		}
#if TGE_690_TEST
		// set test gpio (use ext trigger signal)
		gpio_direction_output(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

		// mark as task trigger start
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
#endif
		g_nvt_kdrv_tge_tsk_item = NVT_KDRV_TGE_TSK_ITEM_MSH_IMD;

		already_start = 1;

	///////////////// stop /////////////////
	} else {
		// stop trigger
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_trig_msh_stop(KDRV_TGE_MSH_1_CLOSE << i)) {
				goto fail;
			}
		}

		nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_MSH);

		already_start = 0;
		KDRV_TGE_DUMP("Mechanical-shutter IMD test done\r\n");
	}

	return TRUE;

fail:
	KDRV_TGE_DUMP("Mechanical-shutter IMD test fail\r\n");
	return FALSE;
}

BOOL nvt_kdrv_tge_cmd_test_msh_wait_vd(unsigned char argc, char **pargv)
{
	UINT32 msh_period[KDRV_TGE_MSH_ID_MAX] = { 5, 10 };
	UINT64 msh_assert_ms[KDRV_TGE_MSH_ID_MAX] = { 30, 60 };
	UINT32 i;
	BOOL ctrl_inv = 0, swap = 0, mode = 0, ext_trig_inv = 0;
	static int already_start = 0;

	if (!KDRV_TGE_TEST_SIE) {
		KDRV_TGE_ERR("not support for sie\r\n");
		return FALSE;
	}

	///////////////// start /////////////////
	if (already_start == 0) {
		KDRV_TGE_DUMP("Mechanical-shutter WAT VD test start ...\r\n");

		// start config tge
		if (nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_MSH)) {
			goto fail;
		}

		// set Mechanical-shutter param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_msh_param((KDRV_TGE_MSH_1_CLOSE << i), KDRV_TGE_CLK_MCLK1, 0, msh_assert_ms[i] * 1000, msh_period[i])) {
				goto fail;
			}
		}

		// get Mechanical-shutter param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_msh_param((KDRV_TGE_MSH_1_CLOSE << i))) {
				goto fail;
			}
		}

		// set Mechanical-shutter pin ctrl param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_msh_pin_ctrl((KDRV_TGE_MSH_1_CLOSE << i), ctrl_inv, swap, mode, ext_trig_inv)) {
				goto fail;
			}
		}

		// get Mechanical-Shutter pin ctrl param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_msh_pin_ctrl((KDRV_TGE_MSH_1_CLOSE << i))) {
				goto fail;
			}
		}

#if TGE_690_TEST
		// set test gpio (use ext trigger signal)
		gpio_direction_output(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

		// mark as task trigger start
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
#endif
		g_nvt_kdrv_tge_tsk_item = NVT_KDRV_TGE_TSK_ITEM_MSH_WAT_VD;

		already_start = 1;

	///////////////// stop /////////////////
	} else {
		// stop trigger
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_trig_msh_stop(KDRV_TGE_MSH_1_CLOSE << i)) {
				goto fail;
			}
		}

		nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_MSH);

		already_start = 0;
		KDRV_TGE_DUMP("Mechanical-shutter WAT VD test done\r\n");
	}

	return TRUE;

fail:
	KDRV_TGE_DUMP("Mechanical-shutter WAT VD test fail\r\n");
	return FALSE;
}


BOOL nvt_kdrv_tge_cmd_test_msh_ext_wait_vd(unsigned char argc, char **pargv)
{
	UINT32 msh_period[KDRV_TGE_MSH_ID_MAX] = { 5, 10 };
	UINT64 msh_assert_ms[KDRV_TGE_MSH_ID_MAX] = { 30, 60 };
	UINT32 i;
	BOOL ctrl_inv = 0, swap = 0, mode = 0, ext_trig_inv = 0;
	static int already_start = 0;

	if (!KDRV_TGE_TEST_SIE) {
		KDRV_TGE_ERR("not support for sie\r\n");
		return FALSE;
	}

	///////////////// start /////////////////
	if (already_start == 0) {
		KDRV_TGE_DUMP("Mechanical-shutter WAT EXT+VD test start ...\r\n");

		// start config tge
		if (nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_MSH)) {
			goto fail;
		}

		// set Mechanical-shutter param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_msh_param((KDRV_TGE_MSH_1_CLOSE << i), KDRV_TGE_CLK_MCLK1, 0, msh_assert_ms[i] * 1000, msh_period[i])) {
				goto fail;
			}
		}

		// get Mechanical-shutter param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_msh_param((KDRV_TGE_MSH_1_CLOSE << i))) {
				goto fail;
			}
		}

		// set Mechanical-shutter pin ctrl param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_msh_pin_ctrl((KDRV_TGE_MSH_1_CLOSE << i), ctrl_inv, swap, mode, ext_trig_inv)) {
				goto fail;
			}
		}

		// get Mechanical-Shutter pin ctrl param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_msh_pin_ctrl((KDRV_TGE_MSH_1_CLOSE << i))) {
				goto fail;
			}
		}

#if TGE_690_TEST
		// set test gpio
		gpio_direction_output(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

		// mark as task trigger start
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
#endif
		g_nvt_kdrv_tge_tsk_item = NVT_KDRV_TGE_TSK_ITEM_MSH_EXT_WAT_VD;

		already_start = 1;

	///////////////// stop /////////////////
	} else {
		// stop trigger
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_trig_msh_stop(KDRV_TGE_MSH_1_CLOSE << i)) {
				goto fail;
			}
		}

		nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_MSH);

		already_start = 0;
		KDRV_TGE_DUMP("Mechanical-shutter WAT EXT+VD test done\r\n");
	}

	return TRUE;

fail:
	KDRV_TGE_DUMP("Mechanical-shutter WAT EXT+VD test fail\r\n");
	return FALSE;
}

BOOL nvt_kdrv_tge_cmd_test_msh_cont_wait_vd(unsigned char argc, char **pargv)
{
	UINT32 msh_period[KDRV_TGE_MSH_ID_MAX] = { 5, 10 };
	UINT64 msh_assert_ms[KDRV_TGE_MSH_ID_MAX] = { 30, 60 };
	UINT32 i;
	BOOL ctrl_inv = 0, swap = 0, mode = 0, ext_trig_inv = 0;
	static int already_start = 0;

	if (!KDRV_TGE_TEST_SIE) {
		KDRV_TGE_ERR("not support for sie\r\n");
		return FALSE;
	}

	///////////////// start /////////////////
	if (already_start == 0) {
		KDRV_TGE_DUMP("Mechanical-shutter WAT CONT+VD test start ...\r\n");

		// start config tge
		if (nvt_kdrv_tge_open(NVT_KDRV_TGE_OPEN_MSH)) {
			goto fail;
		}

		// set Mechanical-shutter param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_msh_param((KDRV_TGE_MSH_1_CLOSE << i), KDRV_TGE_CLK_MCLK1, 0, msh_assert_ms[i] * 1000, msh_period[i])) {
				goto fail;
			}
		}

		// get Mechanical-shutter param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_msh_param((KDRV_TGE_MSH_1_CLOSE << i))) {
				goto fail;
			}
		}

		// set Mechanical-shutter pin ctrl param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_set_msh_pin_ctrl((KDRV_TGE_MSH_1_CLOSE << i), ctrl_inv, swap, mode, ext_trig_inv)) {
				goto fail;
			}
		}

		// get Mechanical-Shutter pin ctrl param
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_get_msh_pin_ctrl((KDRV_TGE_MSH_1_CLOSE << i))) {
				goto fail;
			}
		}

#if TGE_690_TEST
		// set test gpio (use ext trigger signal)
		gpio_direction_output(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);

		// mark as task trigger start
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 0);
		vos_util_delay_ms(10);
		gpio_set_value(KDRV_TGE_FLSH_EXT_TRIG_CTRL_OUT_GPIO, 1);
#endif
		// trigger Mechanical-shutter wait vd continuous
		if (nvt_kdrv_tge_trig_flsh(KDRV_TGE_MSH_1_CLOSE, KDRV_TGE_TRIG_MSH_WAT_CONT)) {  //check
			goto fail;
		}

		already_start = 1;

	///////////////// stop /////////////////
	} else {
		// stop trigger
		for (i = 0; i < KDRV_TGE_MSH_ID_MAX; i++) {
			if (nvt_kdrv_tge_trig_msh_stop(KDRV_TGE_MSH_1_CLOSE << i)) {
				goto fail;
			}
		}

		nvt_kdrv_tge_close(NVT_KDRV_TGE_OPEN_MSH);

		already_start = 0;
		KDRV_TGE_DUMP("Mechanical-shutter WAT CONT+VD test done\r\n");
	}

	return TRUE;

fail:
	KDRV_TGE_DUMP("Mechanical-shutter WAT CONT+VD test fail\r\n");
	return FALSE;
}


#endif

BOOL nvt_kdrv_tge_cmd_dump_info(unsigned char argc, char **pargv)
{
	kdrv_tge_dump_info(kdrv_tge_int_printf);

	return TRUE;
}

BOOL nvt_kdrv_tge_cmd_dbg_lv(unsigned char argc, char **pargv)
{
	UINT32 arg;

	if (strcmp(pargv[0], "err") == 0 || (sscanf(pargv[0], "%u", (unsigned int *)&arg) == 1 && arg == 1)) {
		tge_set_dbg_level(1);
		kdrv_tge_debug_level = 1;
	} else if (strcmp(pargv[0], "wrn") == 0 || (sscanf(pargv[0], "%u", (unsigned int *)&arg) == 1 && arg == 2)) {
		tge_set_dbg_level(2);
		kdrv_tge_debug_level = 2;
	} else if (strcmp(pargv[0], "func") == 0 || (sscanf(pargv[0], "%u", (unsigned int *)&arg) == 1 && arg == 4)) {
		tge_set_dbg_level(4);
		kdrv_tge_debug_level = 4;
	} else if (strcmp(pargv[0], "ind") == 0 || (sscanf(pargv[0], "%u", (unsigned int *)&arg) == 1 && arg == 5)) {
		tge_set_dbg_level(5);
		kdrv_tge_debug_level = 5;
	} else {
		KDRV_TGE_ERR("dbglv err\r\n");
	}

	return TRUE;
}

static SXCMD_BEGIN(kdrv_tge_cmd_tbl, "kdrv_tge")
SXCMD_ITEM("dumpinfo", 		nvt_kdrv_tge_cmd_dump_info, 	"Show kdrv_tge information")
SXCMD_ITEM("dbglv LV(u)", 	nvt_kdrv_tge_cmd_dbg_lv, 		"Change debug level. LV: 1(err), 2(wrn), 4(func), 5(ind)")

#if (KDRV_TGE_TEST_CMD != 0)
SXCMD_ITEM("test_vdhd", 				nvt_kdrv_tge_cmd_test_vdhd, 				"test vdhd")
SXCMD_ITEM("test_flsh_imd", 			nvt_kdrv_tge_cmd_test_flsh_imd, 			"test flashlight immediately trigger")
SXCMD_ITEM("test_flsh_wat_vd", 			nvt_kdrv_tge_cmd_test_flsh_wait_vd, 		"test flashlight wait vd trigger")
SXCMD_ITEM("test_flsh_wat_ext_vd", 		nvt_kdrv_tge_cmd_test_flsh_ext_wait_vd, 	"test flashlight wait external & vd trigger")
SXCMD_ITEM("test_flsh_wat_cont_vd", 	nvt_kdrv_tge_cmd_test_flsh_cont_wait_vd, 	"test flashlight wait vd & continuous trigger")
SXCMD_ITEM("test_msh_imd", 			nvt_kdrv_tge_cmd_test_msh_imd, 			"test Mechanical-shutter immediately trigger")
SXCMD_ITEM("test_msh_wat_vd", 			nvt_kdrv_tge_cmd_test_msh_wait_vd, 		"test Mechanical-shutter wait vd trigger")
SXCMD_ITEM("test_msh_wat_ext_vd", 		nvt_kdrv_tge_cmd_test_msh_ext_wait_vd, 	"test Mechanical-shutter wait external & vd trigger")
SXCMD_ITEM("test_msh_wat_cont_vd", 	nvt_kdrv_tge_cmd_test_msh_cont_wait_vd, 	"test Mechanical-shutter wait vd & continuous trigger")
#endif
SXCMD_END();

void nvt_kdrv_tge_cmd_help(void)
{
	UINT32 i, cmd_num = SXCMD_NUM(kdrv_tge_cmd_tbl);

	for (i = 1; i <= cmd_num; i++) {
		KDRV_TGE_DUMP("CMD: %s\r\n", kdrv_tge_cmd_tbl[i].p_name);
		KDRV_TGE_DUMP("DESC: %s\r\n", kdrv_tge_cmd_tbl[i].p_desc);
	}
}

#if defined(__LINUX)
int kdrv_tge_cmd_execute(unsigned char argc, char **argv)
{
	UINT32 cmd_num = SXCMD_NUM(kdrv_tge_cmd_tbl);
	UINT32 loop;
	int    ret;

	if (argc < 1 || strcmp(argv[0], "?") == 0 || strcmp(argv[0], "help") == 0) {
		nvt_kdrv_tge_cmd_help();
		return 0;
	}

	for (loop = 1 ; loop <= cmd_num ; loop++) {
		if (strncmp(argv[0], kdrv_tge_cmd_tbl[loop].p_name, strlen(argv[0])) == 0) {
			ret = kdrv_tge_cmd_tbl[loop].p_func(argc-1, &argv[1]);
			return ret;
		}
	}

	if (loop > cmd_num) {
		KDRV_TGE_ERR("Invalid CMD !!\r\n");
		nvt_kdrv_tge_cmd_help();
		return -1;
	}

	return 0;
}
#else
MAINFUNC_ENTRY(kdrv_tge, argc, argv)
{
	UINT32 cmd_num = SXCMD_NUM(kdrv_tge_cmd_tbl);
	UINT32 loop;
	int    ret;

	if (argc < 2 || strcmp(argv[1], "?") == 0 || strcmp(argv[1], "help") == 0) {
		nvt_kdrv_tge_cmd_help();
		return 0;
	}

	for (loop = 1 ; loop <= cmd_num ; loop++) {
		if (strncmp(argv[1], kdrv_tge_cmd_tbl[loop].p_name, strlen(argv[1])) == 0) {
			ret = kdrv_tge_cmd_tbl[loop].p_func(argc-1, &argv[2]);
			return ret;
		}
	}

	if (loop > cmd_num) {
		KDRV_TGE_ERR("Invalid CMD !!\r\n");
		nvt_kdrv_tge_cmd_help();
		return -1;
	}

	return 0;
}
#endif
