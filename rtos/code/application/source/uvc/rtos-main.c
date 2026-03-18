// libc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// vos
#include <kwrap/cmdsys.h>
#include <kwrap/examsys.h>
#include <kwrap/perf.h>
#include <kwrap/util.h>
// kdrv
#include <comm/util/log.h>
#include <comm/timer.h>
#include <comm/hwclock.h>
#include <comm/ddr_arb.h>
#include <plat/drtc.h>
#include <plat/sdio.h>
#include <plat/grph_plat.h>
#include <plat/top.h>
#include <plat/rtc.h>
#include <plat/sdp.h>
#include <plat/spi.h>
#include <plat/usb2dev.h>
#include <plat/clock.h>
#include <kdrv_videocapture/kdrv_sie.h>
#include <kdrv_videoprocess/kdrv_ipp.h>
#include <kdrv_gfx2d/kdrv_ise.h>
// kflow
#include <kdrv_gfx2d/kdrv_ise.h>
#include <kdrv_videoprocess/kdrv_sde.h>
#include <kdrv_videoprocess/vpe/vpe_drv_ctl.h>
#include <kdrv_videoprocess/dre/kdrv_dre_ctl.h>
#include <kdrv_videocapture/kdrv_sie.h>
#include <kdrv_videocapture/kdrv_vie.h>
#include <kdrv_videocapture/kdrv_tge.h>
#include <kdrv_videoprocess/vpe/vpe_drv_ctl.h>

// kflow
#include <kflow.h>
#include <kflow_videocapture/ctl_sen.h>
#include <kflow_videocapture/ctl_sie.h>
#include <kflow_videocapture/ctl_vie.h>


#include <kflow.h>
#include <kflow_videocapture/ctl_sie.h>
#include <gximage/hd_gximage.h>
#include <videosprite/videosprite.h>
// PQ
#include "isp_rtos_inc.h"
#include "isp_api.h"
// ext_devices
#include <dispdev_panlcomn.h>
#include <sen_inc.h>
// driver
#include <serial.h>
// lib
#include <DbgUtApi.h>
// locals
#include "prjcfg.h"
#include "startup/startup.h"
#include "sys_mempool.h"
#include "sys_nvtmpp.h"
#include "sys_filesys.h"
#include "sys_card.h"
#include "sys_fwload.h"
#include "sys_fastboot.h"
#include "sys_storage_partition.h"
#include "sys_fdt.h"
#include "sys_pstore.h"
#include "flow_uvc.h"
#include "flow_updfw.h"

#define _SEN_INIT(sen, dtsi_info) sen_init_##sen(dtsi_info);
#define SEN_INIT(sen, dtsi_info) _SEN_INIT(sen, dtsi_info)

#define U3UVAC 1
#define PQ_ISP_ENABLE 0
#define PQ_VPE_ENABLE 0

static void insmod_system(void)
{
	vos_perf_list_mark("b_sys", __LINE__, 0);

	/* system */
	fwload_init();
	fwload_wait_done(CODE_SECTION_01);
	nvt_pinmux_init();      // pinmux
	arb_init();             // arb
	nvt_cmdsys_init();      // command system
	nvt_examsys_init();     // exam system
	pll_init();             // pll
	clock_platform_init();  // clock gen
	nvt_timer_drv_init();   // timer
	drtc_platform_init();   // drtc
	rtc_platform_init();    // rtc
	hwclock_init();         // hardware clock
	log_init(NULL);         // kdrv_log
	fastboot_init();        // fast boot
#if defined(_EMBMEM_EMMC_) || defined(_EMBMEM_COMBO_)
	sdio_platform_init();   // sdio for emmc storage object
#endif
	storage_partition_init1(); // storage partition for partial load
	gpio_platform_init();
	gpio_open();

#if (U3UVAC == 0)
	usb2dev_power_on_init(TRUE);//USB
#endif
	vos_perf_list_mark("b_sys", __LINE__, 1);
	fastboot_set_done(BOOT_INIT_SYSTEM);
}

static void insmod_sensor(void)
{
	SENSOR_DTSI_INFO dtsi_info;

	fwload_wait_done(CODE_SECTION_02);
	vos_perf_list_mark("b_sen", __LINE__, 0);

	dtsi_info.addr = fdt_get_sensor();
	SEN_INIT(_SEN1_, &dtsi_info); //sensor (macro expression like invoking sen_init_imx290(), sen_init_os02k10()

	vos_perf_list_mark("b_sen", __LINE__, 1);
	nvtmpp_init();
	fastboot_set_done(BOOT_INIT_SENSOR);
}

static void insmod_capture(void)
{
	fwload_wait_done(CODE_SECTION_03);
	vos_perf_list_mark("b_cap", __LINE__, 0);

#if (_PACKAGE_VIDEO_)
	// allocate fixed memory (nvtmpp is required for mempool_init)
	nvtmpp_init(); // nvtmpp_init is in part-3
	mempool_init();

	/* capture */
	kdrv_sie_init();        // sie engine
	kdrv_vie_init();        // vie engine
	kdrv_tge_init();        // tge engine
	kdrv_ife_rtos_init();        // ife engine
	kdrv_dce_rtos_init();        // dce engine
	kdrv_ipe_rtos_init();        // ipe engine
	ime_builtin_rtos_init();        // ime engine
	//kdrv_ife2_init();       // ife2 engine
	grph_platform_init();   // graphic engine
	kdrv_ise_drv_init();    // ise engine
	kflow_ctl_ipp_init();   // control ipp

	kflow_ctl_sen_init();   // control sensor
	kflow_ctl_sie_init();   // control sie
	kflow_ctl_vie_init();   // control vie

	kdrv_vpe_rtos_init();	//vpe engine
	kflow_videocap_init();  // video capture
	kflow_videoproc_init(); // video process


	#if (PQ_ISP_ENABLE)
	isp_api_reg_if(0x3F);           // isp
	ae_init_module(0x3F);           // ae
	awb_init_module(0x3F);          // awb
	iq_init_module(0x3F, 0, 0, 1);  // iq, dpc_en=0, ecs_en=0, _3dcc_en=1
	#endif
	#if (PQ_VPE_ENABLE)
	vpe_api_reg_if(0x3, 0x3, 0x4, 0x3, 0x5);  // vpe, vpe_id_list=0x3 vpe_idx_list=0x3 vpe_idx_num=0x4 vpelite_id_list=0x3 vpe_2dlut_size=0x5
	#endif

	nvt_gfx_init();         // gfx
	nvt_vds_init();         // video srpite
#else
	mempool_init2();        // mempool by malloc version
#endif

	vos_perf_list_mark("b_cap", __LINE__, 1);
	fastboot_set_done(BOOT_INIT_CAPTURE);
}

static void insmod_display(void)
{
	fwload_wait_done(CODE_SECTION_04);

	/* display */
#if (_PACKAGE_DISPLAY_)
	kflow_videoout_init();  // video output
	panel_init();           // panel
#endif

	fastboot_set_done(BOOT_INIT_DISPLAY);
}

static void insmod_storage(void)
{
	fwload_wait_done(CODE_SECTION_05);
	vos_perf_list_mark("b_strg", __LINE__, 0);

	/* storage */
#if (_PACKAGE_SDCARD_)
#if !defined(_EMBMEM_EMMC_) && !defined(_EMBMEM_COMBO_)
	sdio_platform_init();   // sdio
#endif
	card_init();            // sd card
#endif
#if (_PACKAGE_FILESYS_)
	filesys_init();         // file system
#endif

	vos_perf_list_mark("b_strg", __LINE__, 1);
	fastboot_set_done(BOOT_INIT_STORAGE);
}

static void insmod_encoder(void)
{
	fwload_wait_done(CODE_SECTION_05);

	/* media encoder */
#if (_PACKAGE_VIDEO_)
	kflow_videoenc_init();  // video encoder
#endif
#if (_PACKAGE_AUDIO_)
	kflow_audiocap_init();  // audio capture
	kflow_audioenc_init();  // audio encoder
#endif
	fastboot_set_done(BOOT_INIT_MEIDA_ENCODER);
}

static void insmod_others(void)
{
	fwload_wait_done(CODE_SECTION_10);

#if defined(_FW_TYPE_PARTIAL_) || defined(_FW_TYPE_PARTIAL_COMPRESS_)
	// !!!!! global c++'s constructors will be out of control on partial load tech. !!!!
	// !!!!! so we only allow to use c++ after section_10 loaded. !!!!
	// c++'s constructors init
	constructors_init();
#endif
	/* others */
	DbgUt_Init();           // init DbgUt to measure usage of CPU/DMA
#if (_PACKAGE_VIDEO_)
	kflow_videodec_init();  // video decoder
#endif
#if (_PACKAGE_AUDIO_)
	kflow_audiodec_init();  // audio decoder
	kflow_audioout_init();  // audio output
#endif
	sdp_platform_init();    // SDP (spi slave like)
	spi_platform_init();    // SPI (SPI master)
	storage_partition_init2(); // init others storage partition after fastboot
	System_OnStrgInit_PS();
	fastboot_set_done(BOOT_INIT_OTHERS);
}

void insmod(void)
{
#if (POWERON_FAST_BOOT)
	// all insmod depend on insmod_system, so call it first
	insmod_system();
	// create task for multi-init after startinsmod_system();
	VK_TASK_HANDLE vkt_sensor = vos_task_create(fastboot_thread, insmod_sensor, "init_sensor", 5, 4096);
	vos_task_resume(vkt_sensor);
	VK_TASK_HANDLE vkt_capture = vos_task_create(fastboot_thread, insmod_capture, "init_capture", 5, 4096);
	vos_task_resume(vkt_capture);
	VK_TASK_HANDLE vkt_display = vos_task_create(fastboot_thread, insmod_display, "init_display", 10, 4096);
	vos_task_resume(vkt_display);
	VK_TASK_HANDLE vkt_storage = vos_task_create(fastboot_thread, insmod_storage, "init_storage", 10, 4096);
	vos_task_resume(vkt_storage);
	VK_TASK_HANDLE vkt_encoder = vos_task_create(fastboot_thread, insmod_encoder, "init_encoder", 10, 4096);
	vos_task_resume(vkt_encoder);
	VK_TASK_HANDLE vkt_others  = vos_task_create(fastboot_thread, insmod_others,  "init_others",  10, 4096);
	vos_task_resume(vkt_others);
	//start partial load
	VK_TASK_HANDLE vkt_partload  = vos_task_create(fastboot_thread, fwload_partload,  "init_partload",  10, 4096);
	vos_task_resume(vkt_partload);
#else
	insmod_system();
#if defined(_FW_TYPE_PARTIAL_) || defined(_FW_TYPE_PARTIAL_COMPRESS_)
	//start partial load
	VK_TASK_HANDLE vkt_partload  = vos_task_create(fastboot_thread, fwload_partload,  "init_partload",  10, 4096);
	vos_task_resume(vkt_partload);
	fwload_wait_done(CODE_SECTION_10);
#endif
	insmod_sensor();
	insmod_capture();
	insmod_display();
	insmod_storage();
	insmod_encoder();
	insmod_others();
#endif
}

static void boot(void)
{
	vos_perf_list_reset();
	vos_perf_list_mark(__func__, __LINE__, 0);

	// show the first message
	serial_open(); // uart init first for debug message
	printf("\nHello RTOS World! (%s)\n\n", __DATE__ " - " __TIME__);

	insmod(); // insmod for modules initialzation
	flow_updfw_init_bank_info(); // init bank by thread internal
	flow_uvc(); // start uvc

	vos_perf_list_mark(__func__, __LINE__, 1);
	fastboot_set_done(BOOT_FLOW_BOOT);
}

void rtos_main(void)
{
#if (POWERON_FAST_BOOT_MSG == DISABLE)
	fastboot_msg_en(DISABLE);
#endif

#if (POWERON_FAST_BOOT)
	VK_TASK_HANDLE vkt_boot = vos_task_create(fastboot_thread, boot,  "boot",  9, 8192*2);
	vos_task_resume(vkt_boot);
	fastboot_wait_done(BOOT_FLOW_BOOT);
#else
	boot();
#endif

	// show report after filesys initialtion,
	// because filesys is the last place
	fastboot_wait_done(BOOT_INIT_STORAGE);
#if (POWERON_FAST_BOOT_MSG == DISABLE)
	fastboot_msg_en(ENABLE);
#endif

#if (POWERON_BOOT_REPORT)
	vos_perf_list_dump();
#endif
}
