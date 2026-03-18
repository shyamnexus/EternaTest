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
#include <shm_info.h>
#include <kwrap/debug.h>
#include <vendor_videocapture.h>
#include <kwrap/util.h>
#include <plat/gpio.h>
#include <plat/pll.h>
#include <kdrv_builtin/vdoenc_builtin.h>
#include <kdrv_builtin/osg_builtin.h>
#include <kdrv_builtin/fdtfast.h>
#include "kdrv_builtin/nvtmpp_init.h"
#include <isp_builtin.h>
#include "prjcfg.h"
#include "sys_fdt.h"
#include "sys_fwload.h"
#include "sys_fastboot.h"
#include "sys_clock.h"
#include "sys_linuxboot.h"
#include "flow_boot_linux.h"
//#include "flow_preroll.h"
#include "bridge_fourcc.h"
#include "bridge_mem.h"
#include "fast_venc.h"
#include "fast_sensor.h"
#include "fast_ipp.h"
#include "fast_osg_demo.h"
#include <kdrv_gfx2d/kdrv_ise_ctl.h>

static pthread_t handle_sensor;
static pthread_t handle_panel;
static pthread_t handle_linux;
static pthread_t handle_audioout;
static pthread_t handle_loadai;
static pthread_t handle_detect;
static pthread_t handle_venc;
static pthread_t handle_preroll;

// plan the bridge memory
int bridge_mem_plan(void)
{
	//init all sensor by default off
	bridge_mem_add_tag(SENSOR_PRESET_NAME, 0xFFFFFFFF);
	bridge_mem_add_tag(SENSOR_PRESET_NAME_2, 0xFFFFFFFF);

#if defined(_SEN1_ENABLED_)
	bridge_mem_plan_sensor1();
#endif

#if defined(_SEN2_ENABLED_)
	bridge_mem_plan_sensor2();
#endif

	bridge_mem_add_tag(AMP_TAKEOVER_FROM_RTOS, 0);
	return 0;
}

#if (POWERON_MODE == POWERON_MODE_PREROLL)
int preroll_bs_decode(void)
{
	DBG_DUMP("%s\r\n", __func__);

	// wait whole rtos loaded from flash finished
	fwload_wait_done(CODE_SECTION_10);
	fastboot_wait_done(BOOT_INIT_PREROLL);
	flow_preroll_get_bs_info();
	return 0;
}
#endif

// thread to setup sensor
static void *thread_sensor(void *ptr)
{
	//quick sensor setup flow
	fastboot_wait_done(BOOT_INIT_SENSOR);
	vos_perf_list_mark("ss", __LINE__, 0);

#if defined(FASTBOOT20)
	DBG_DUMP("is_fastboot = %d\r\n", is_fastboot());
	if (is_fastboot()) {
		if (nvtmpp_parse_fastboot_mem_dt() < 0) {
	        DBG_ERR("parse fastboot_mem fail\r\n");
	        return NULL;
		}
		UINT32 turn_on_sensor = 0;
#if defined(_SEN1_ENABLED_)
		fast_open_sensor();
		turn_on_sensor |= 0x1;

#endif
#if defined(_SEN2_ENABLED_)
		fast_open_sensor2();
		turn_on_sensor |= 0x2;
#endif
#if (FASTBOOT_2A == ENABLE)
		fast_2a();
#elif (SENSOR_EMBEDDED_2A == ENABLE)
		sensor_embedded_2a();
#elif (PREROLL_2A == ENABLE)
		sensor_preroll_2a();
#elif (NORMAL_2A == ENABLE)
		sensor_normal_2a();
#endif
#if (CFG_RTOS_IPP == ENABLE)
		UINT32 en = 1;

		vendor_videocap_set_lite(turn_on_sensor, VENDOR_VIDEOCAP_PARAM_FAST_START, &en);
#else
		DBG_DUMP("no ipp, turn_on_sensor = %X\n", turn_on_sensor);
#endif
	} else {
		extern void pll_disable_clock(CG_EN Num);
		if (pll_is_clock_enabled(SIE_MCLK)) {
			DBG_DUMP("force to disable SIE_MCLK, bcz non-fastboot\n");
			pll_disable_clock(SIE_MCLK);
		}
		if (pll_is_clock_enabled(SIE_MCLK2)) {
			DBG_DUMP("force to disable SIE_MCLK2, bcz non-fastboot\n");
			pll_disable_clock(SIE_MCLK2);
		}
		if (pll_is_clock_enabled(SIE_MCLK3)) {
			DBG_DUMP("force to disable SIE_MCLK3, bcz non-fastboot\n");
			pll_disable_clock(SIE_MCLK3);
		}
		if (pll_get_pll_enable(PLL_ID_5)) {
			// force disable pll5 for linux first change mode can assign new pll5 freq
			DBG_DUMP("force to disable PLL5, bcz non-fastboot\n");
			// for driver default count is 0 issue
			// here must open first then close
			pll_set_pll_enable(PLL_ID_5, TRUE);
			pll_set_pll_enable(PLL_ID_5, FALSE);
		}
		pthread_exit((void *)0);
		return NULL;
	}
#endif // end of defined(FASTBOOT20)

	vos_perf_list_mark("ss", __LINE__, 1);

#if defined(FASTBOOT20)
#if (FASTBOOT_2A == ENABLE)
	vos_util_delay_ms(fast_2a_delay_ms()); // run 2a a while
	vos_perf_list_mark("ss", __LINE__, 2);
	isp_builtin_set_sensor_bypass(TRUE);
#if defined(_SEN1_ENABLED_)
	fast_chgmode_sensor();
#endif
#if defined(_SEN2_ENABLED_)
	fast_chgmode_sensor2();
#endif
#endif
	// fast IPP
#if (CFG_RTOS_IPP == ENABLE)
	SHMINFO *p_shm = fdt_get_shminfo();
	if (p_shm->boot.LdCtrl2 & LDCF_BOOT_CARD) {
		// skip fast_ipp, because T.bin buffer overlap between preparing linux and ipp buffer
	} else {
		fast_ipp();
#if !(CFG_LOAD_AI_DETECT)
		// if no ai detection, we can run ll early,
		// if ai detection exists, run ll will cause image buffer over written.
		fast_ipp_run_ll();
#endif
#if (POWERON_MODE == POWERON_MODE_PREROLL)
	fastboot_set_done(BOOT_INIT_PREROLL);
#endif
	}
#endif
#endif // end of defined(FASTBOOT20)

	pthread_exit((void *)0);
	return NULL;
}

// thread to setup panel
static void *thread_panel(void *ptr)
{
#if !defined(_disp_off_)
	//quick sensor setup flow
#if (_PACKAGE_DISPLAY_)
	fastboot_wait_done(BOOT_INIT_DISPLAY);
#else
	fastboot_wait_done(BOOT_INIT_OTHERS);
#endif
	vos_perf_list_mark("ps", __LINE__, 0);
	if (is_fastboot()) {
		fast_open_panel();
	}
	vos_perf_list_mark("ps", __LINE__, 1);
#endif
	pthread_exit((void *)0);
	return NULL;
}

// thread to setup aout
static void *thread_audioout(void *ptr)
{
	// wait whole rtos loaded from flash finished
	fwload_wait_done(CODE_SECTION_05);
	// wait storage_partition_init2 finished
	fastboot_wait_done(BOOT_INIT_MEIDA_ENCODER);
	vos_perf_list_mark("ao", __LINE__, 0);
	if (is_fastboot()) {
#if (CFG_LOAD_AUDIOIO)
		fast_open_audioout();
		fast_trig_audioout();
#endif
	}
	vos_perf_list_mark("ao", __LINE__, 1);

	pthread_exit((void *)0);

	return NULL;
}

// thread to setup ai
static void *thread_loadai(void *ptr)
{
#if defined(FASTBOOT20)
	// wait whole rtos loaded from flash finished
	fwload_wait_done(CODE_SECTION_10);
	// wait storage_partition_init2 finished
	fastboot_wait_done(BOOT_INIT_OTHERS);

	if (is_fastboot()) {
#if (CFG_LOAD_AI_DETECT)
#if !(_PACKAGE_VIDEO_)
		kdrv_ise_rtos_init();
#endif
		operation_ai_prepare(1);
#endif
	}
#endif // end of defined(FASTBOOT20)

	pthread_exit((void *)0);

	return NULL;
}

// thread to venc
static void *thread_venc(void *ptr)
{
#if defined(FASTBOOT20)
	// wait whole rtos loaded from flash finished
	fwload_wait_done(CODE_SECTION_05);
	// wait storage_partition_init2 finished
	fastboot_wait_done(BOOT_INIT_MEIDA_ENCODER);

	if (is_fastboot()) {
#if (CFG_RTOS_VENC)
		venc_fast_init();
#endif
		// osg located in section-10
		fwload_wait_done(CODE_SECTION_10);
		osg_module_init();
		venc_osg_demo_init();
	}
#endif // end of defined(FASTBOOT20)

	pthread_exit((void *)0);

	return NULL;
}

// thread to preroll
static void *thread_preroll(void *ptr)
{
#if (POWERON_MODE == POWERON_MODE_PREROLL)
	if (is_fastboot()) {
		fastboot_wait_done(BOOT_INIT_PREROLL);
		flow_preroll_dec_init();
		preroll_bs_decode();
	}
#endif
	pthread_exit((void *)0);

	return NULL;
}

#if defined(FASTBOOT20)
static void on_notify_linux_kdrv_builtin(void)
{
	// notify linux kdrv_builtin to next step
	bridge_mem_modify_tag(AMP_TAKEOVER_FROM_RTOS, 0);
}
#endif

// thread to setup detect
static void *thread_detect(void *ptr)
{
#if defined(FASTBOOT20)
	// wait whole rtos loaded from flash finished
	fwload_wait_done(CODE_SECTION_10);
	// wait storage_partition_init2 finished
	fastboot_wait_done(BOOT_INIT_OTHERS);

	if (is_fastboot()) {
#if (CFG_LOAD_AI_DETECT)
		operation_ai_detect(1);
#endif
	}
#endif // end of defined(FASTBOOT20)

	pthread_exit((void *)0);

	return NULL;
}

// thread to start linux
static void *thread_linux(void *ptr)
{
	LINUXBOOT_INFO info = {0};
	// wait whole rtos loaded from flash finished
	fwload_wait_done(CODE_SECTION_10);
	// wait storage_partition_init2 finished
	fastboot_wait_done(BOOT_INIT_OTHERS);

	int join_ret;
	int pthread_ret;

	// wait loadai thread
	pthread_ret = pthread_join(handle_loadai, (void *)&join_ret);
	if (0 != pthread_ret) {
		DBG_ERR("handle_loadai pthread_join failed, ret %d\r\n", pthread_ret);
		pthread_exit((void *) -1);
		return NULL;
	}
	vos_perf_list_mark("linux", __LINE__, 0);
	if (linuxboot_setup(&info) != 0) {
		pthread_exit((void *) -1);
		return NULL;
	}
	vos_perf_list_mark("linux", __LINE__, 1);
	// wait sensor thread
	pthread_ret = pthread_join(handle_sensor, (void *)&join_ret);
	if (0 != pthread_ret) {
		DBG_ERR("handle_sensor pthread_join failed, ret %d\r\n", pthread_ret);
		pthread_exit((void *) -1);
		return NULL;
	}
	// wait panel thread
	pthread_ret = pthread_join(handle_panel, (void *)&join_ret);
	if (0 != pthread_ret) {
		DBG_ERR("handle_panel pthread_join failed, ret %d\r\n", pthread_ret);
		pthread_exit((void *) -1);
		return NULL;
	}
	// wait audioout thread
	pthread_ret = pthread_join(handle_audioout, (void *)&join_ret);
	if (0 != pthread_ret) {
		DBG_ERR("handle_audioout pthread_join failed, ret %d\r\n", pthread_ret);
		pthread_exit((void *) -1);
		return NULL;
	}
	// wait detect thread
	pthread_ret = pthread_join(handle_detect, (void *)&join_ret);
	if (0 != pthread_ret) {
		DBG_ERR("handle_detect pthread_join failed, ret %d\r\n", pthread_ret);
		pthread_exit((void *) -1);
		return NULL;
	}
	// wait venc init
	pthread_ret = pthread_join(handle_venc, (void *)&join_ret);
	if (0 != pthread_ret) {
		DBG_ERR("handle_venc pthread_join failed, ret %d\r\n", pthread_ret);
		pthread_exit((void *) -1);
		return NULL;
	}
	// wait preroll init
	pthread_ret = pthread_join(handle_preroll, (void *)&join_ret);
	if (0 != pthread_ret) {
		DBG_ERR("handle_preroll pthread_join failed, ret %d\r\n", pthread_ret);
		pthread_exit((void *) -1);
		return NULL;
	}
#if (CFG_LOAD_AI_MODEL)
	load_ai_model_nowait(&info);
#endif
#if defined(FASTBOOT20)
	if (is_fastboot()) {
#if (CFG_LOAD_AI_DETECT)
		if (operation_ai_get_result() == 0) {
			//Power Off, example to use JTAG_TCK Pin to notify power controller turn off power
			//gpio_setDir(D_GPIO_4, GPIO_DIR_OUTPUT);
			//gpio_setPin(D_GPIO_4);
		}
#endif
		SHMINFO *p_shm = fdt_get_shminfo();
		if (p_shm->boot.LdCtrl2 & LDCF_BOOT_CARD) {
			fast_ipp();
		}
#if (CFG_LOAD_AI_DETECT)
		// if ai detection exists, run ll after ai detection finished..
		fast_ipp_run_ll();
#endif
	}
#endif // end of defined(FASTBOOT20)
	// plan the bridge memory must be after hdal is ready
	if (bridge_mem_plan() != 0) {
		return NULL;
	}
#if (POWERON_FAST_BOOT_MSG == DISABLE)
	fastboot_msg_en(ENABLE);
#endif

#if (POWERON_BOOT_REPORT)
	vos_util_delay_ms(200); // vos_perf_list_dump() occupy most cpu usage will effect to flow_ipp
	vos_perf_list_dump();
#endif
#if (POWERON_MODE == POWERON_MODE_PREROLL)
	if (is_fastboot()) {
		fastboot_wait_done(BOOT_FLOW_PREROLL);
	}
#endif
#if !defined(FASTBOOT_LCD_DEMO)
	clock_disable_unused();
	clock_set_apb_as_fdt();
	linuxboot_go(&info); //never returned
#endif
#if defined(FASTBOOT20)
	if (is_fastboot()) {
		// wait linux builtin
		unsigned int take_over = 0;
		while (!take_over) {
			//vos_cpu_dcache_sync((VOS_ADDR)mp_bridge, mp_bridge->bridge_size, VOS_DMA_FROM_DEVICE);
			if (bridge_mem_get_tag(AMP_TAKEOVER_FROM_RTOS, &take_over) != 0) {
				DBG_ERR("failed to get AMP_TAKEOVER_FROM_RTOS status");
				break;
			}
			vos_util_delay_us(500);
		}
	}
#if (CFG_RTOS_IPP == ENABLE) && (CFG_RTOS_VENC == ENABLE)
	if (is_fastboot()) {
		vdoenc_builtIn_disable_h26x_int();
		bridge_mem_plan_ipp();
		bridge_mem_plan_venc();
	}
#endif

#if (defined(_SEN1_ENABLED_) || defined(_SEN2_ENABLED_))
	bridge_mem_plan_sie();
#endif

	// never return
	linuxboot_release_rtos(&info, on_notify_linux_kdrv_builtin);
#endif // end of defined(FASTBOOT20)
	return NULL;
}

// main flow
int flow_boot_linux(void)
{
	int pthread_ret;
	int join_ret;
	int policy;
	struct sched_param schedparam = {0};

	// setup bridge mem description
	if (0 != bridge_mem_init()) {
		return -1;
	}
	// thread for sensor setup
	pthread_ret = pthread_create(&handle_sensor, NULL, thread_sensor, NULL);
	if (0 != pthread_ret) {
		DBG_ERR("create thread_sensor failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_sensor, &policy, &schedparam)) {
		DBG_ERR("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 20;
		pthread_setschedparam(handle_sensor, policy, &schedparam);
	}

	// thread for sensor setup
	pthread_ret = pthread_create(&handle_panel, NULL, thread_panel, NULL);
	if (0 != pthread_ret) {
		DBG_ERR("create thread_panel failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_panel, &policy, &schedparam)) {
		DBG_ERR("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 20;
		pthread_setschedparam(handle_panel, policy, &schedparam);
	}
	// thread for booting linux
	pthread_ret = pthread_create(&handle_linux, NULL, thread_linux, NULL);
	if (0 != pthread_ret) {
		DBG_ERR("create thread_cap_proc failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_linux, &policy, &schedparam)) {
		DBG_ERR("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 10;
		pthread_setschedparam(handle_linux, policy, &schedparam);
	}
	// thread for audioout setup
	pthread_ret = pthread_create(&handle_audioout, NULL, thread_audioout, NULL);
	if (0 != pthread_ret) {
		DBG_ERR("create thread_audioout failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_audioout, &policy, &schedparam)) {
		DBG_ERR("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 8;
		pthread_setschedparam(handle_audioout, policy, &schedparam);
	}

	// thread for loadai setup
	pthread_ret = pthread_create(&handle_loadai, NULL, thread_loadai, NULL);
	if (0 != pthread_ret) {
		DBG_ERR("create thread_loadai failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_loadai, &policy, &schedparam)) {
		DBG_ERR("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 8;
		pthread_setschedparam(handle_loadai, policy, &schedparam);
	}

	// thread for detect setup
	pthread_ret = pthread_create(&handle_detect, NULL, thread_detect, NULL);
	if (0 != pthread_ret) {
		DBG_ERR("create thread_detect failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_detect, &policy, &schedparam)) {
		DBG_ERR("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 8;
		pthread_setschedparam(handle_detect, policy, &schedparam);
	}

	// thread for venc
	pthread_ret = pthread_create(&handle_venc, NULL, thread_venc, NULL);
	if (0 != pthread_ret) {
		DBG_ERR("create thread_venc failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_venc, &policy, &schedparam)) {
		DBG_ERR("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 15;
		pthread_setschedparam(handle_venc, policy, &schedparam);
	}

	// thread for preroll
	pthread_ret = pthread_create(&handle_preroll, NULL, thread_preroll, NULL);
	if (0 != pthread_ret) {
		DBG_ERR("create thread_preroll failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_preroll, &policy, &schedparam)) {
		DBG_ERR("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 15;
		pthread_setschedparam(handle_preroll, policy, &schedparam);
	}

	// blocking here until linux booting
	pthread_ret = pthread_join(handle_linux, (void *)&join_ret);
	if (0 != pthread_ret) {
		DBG_ERR("handle_linux pthread_join failed, ret %d\r\n", pthread_ret);
		return -1;
	}

	return 0;
}
