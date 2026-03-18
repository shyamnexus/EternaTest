/**
	@brief Sample code of audio capture.\n

	@file audio_capture_only.c

	@author HM Tseng

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include <kwrap/examsys.h>

#define EXT_CODEC 1
#if EXT_CODEC
#include "vendor_audiocapture.h"
#define AUDCAP_CKRATIO  32
#define AUDCAP_TDMCH    2
#define AUDCAP_OPMODE   0 //0 as slave, 1 as master
#endif
#define CTRL_PWR   0

/* parameter table */
#define AUDCAP_SR       HD_AUDIO_SR_48000
#define AUDCAP_BIT      HD_AUDIO_BIT_WIDTH_16
#define AUDCAP_MODE     HD_AUDIO_SOUND_MODE_STEREO
#define AUDCAP_MONO     HD_AUDIO_MONO_RIGHT

#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <kwrap/task.h>
#define sleep(x)    vos_task_delay_ms(1000*x)
#endif

#define CHKPNT  printf("\033[37mCHK: %s, %s: %d\033[0m",__FILE__,__func__,__LINE__)
#define DEBUG_MENU 1

///////////////////////////////////////////////////////////////////////////////

static int mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	/*dummy buffer, not for audio module*/
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = 0x1000;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;

	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
		return -1;
	}
	return 0;
}

static HD_RESULT mem_exit(void)
{
	HD_RESULT ret = HD_OK;
	ret = hd_common_mem_uninit();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

#if EXT_CODEC
static HD_RESULT set_cap_cfg(HD_PATH_ID *p_audio_cap_ctrl, HD_AUDIO_SR sample_rate, UINT32 i2s_num)
#else
static HD_RESULT set_cap_cfg(HD_PATH_ID *p_audio_cap_ctrl, HD_AUDIO_SR sample_rate)
#endif
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_DEV_CONFIG audio_cfg_param = {0};
	HD_AUDIOCAP_DRV_CONFIG audio_driver_cfg_param = {0};
	HD_PATH_ID audio_cap_ctrl = 0;
#if EXT_CODEC
	VENDOR_AUDIOCAP_INIT_CFG vendor_config = {0};
	{
		ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl); //open this for device control
	}
#else
	ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl); //open this for device control
#endif
	if (ret != HD_OK) {
		return ret;
	}

	/*set audio capture maximum parameters*/
	audio_cfg_param.in_max.sample_rate = sample_rate;
	audio_cfg_param.in_max.sample_bit = AUDCAP_BIT;
	audio_cfg_param.in_max.mode = AUDCAP_MODE;
	audio_cfg_param.in_max.frame_sample = 1024;
	audio_cfg_param.frame_num_max = 10;
	audio_cfg_param.out_max.sample_rate = 0;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_cfg_param);
	if (ret != HD_OK) {
		return ret;
	}

	/*set audio capture driver parameters*/
	audio_driver_cfg_param.mono = AUDCAP_MONO;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_driver_cfg_param);
	if (ret != HD_OK) {
		return ret;
	}

#if CTRL_PWR
	{
		INT32 prepwr_enable = 0;
		ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_PREPWR_ENABLE, &prepwr_enable);
		if (ret != HD_OK) {
			return ret;
		}
	}
#endif

#if EXT_CODEC
	if (i2s_num) {
		//set for external codec input
		snprintf(vendor_config.driver_name, VENDOR_AUDIOCAP_NAME_LEN-1, "nvt_aud_emu");
		vendor_config.aud_init_cfg.i2s_cfg.bit_clk_ratio = AUDCAP_CKRATIO;
		vendor_config.aud_init_cfg.i2s_cfg.bit_width     = AUDCAP_BIT;
		vendor_config.aud_init_cfg.i2s_cfg.tdm_ch        = AUDCAP_TDMCH;
		vendor_config.aud_init_cfg.i2s_cfg.op_mode       = AUDCAP_OPMODE; // acap (rx) as slave
		ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_EXT, (VOID *)&vendor_config);
	}
#endif

	*p_audio_cap_ctrl = audio_cap_ctrl;

	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID audio_cap_path, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_IN audio_cap_in_param = {0};
	HD_AUDIOCAP_OUT audio_cap_out_param = {0};

	// set hd_audiocapture input parameters
	audio_cap_in_param.sample_rate = sample_rate;
	audio_cap_in_param.sample_bit = AUDCAP_BIT;
	audio_cap_in_param.mode = AUDCAP_MODE;
	audio_cap_in_param.frame_sample = 1024;
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_IN, &audio_cap_in_param);
	if (ret != HD_OK) {
		return ret;
	}

	// set hd_audiocapture output parameters
	audio_cap_out_param.sample_rate = 0;
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_OUT, &audio_cap_out_param);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _AUDIO_CAPONLY {
	HD_AUDIO_SR sample_rate_max;
	HD_AUDIO_SR sample_rate;

	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

#if EXT_CODEC
	UINT32 cap_i2s_num;
#endif

	UINT32 cap_exit;
	UINT32 flow_start;
} AUDIO_CAPONLY;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_init()) != HD_OK)
		return ret;

	return HD_OK;
}


static HD_RESULT open_module(AUDIO_CAPONLY *p_caponly)
{
	HD_RESULT ret;
#if EXT_CODEC
	ret = set_cap_cfg(&p_caponly->cap_ctrl, p_caponly->sample_rate_max, p_caponly->cap_i2s_num);
#else
	ret = set_cap_cfg(&p_caponly->cap_ctrl, p_caponly->sample_rate_max);
#endif
	if (ret != HD_OK) {
		printf("set cap-cfg fail\n");
		return HD_ERR_NG;
	}
#if EXT_CODEC
	{
		if((ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_caponly->cap_path)) != HD_OK)
			return ret;
	}
#else
	if((ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_caponly->cap_path)) != HD_OK)
		return ret;
#endif
	return HD_OK;
}

static HD_RESULT close_module(AUDIO_CAPONLY *p_caponly)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_close(p_caponly->cap_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}

static void *capture_thread(void *arg)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIO_FRAME  data_pull;
	UINTPTR vir_addr_main;
	HD_AUDIOCAP_BUFINFO phy_buf_main;
	char file_path_main[64], file_path_len[64];
	FILE *f_out_main, *f_out_len;
	AUDIO_CAPONLY *p_cap_only = (AUDIO_CAPONLY *)arg;

	#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

	/* config pattern name */
	snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/dump_audio_bs_%d_%d_%d_pcm.dat", AUDCAP_BIT, AUDCAP_MODE, p_cap_only->sample_rate);
	snprintf(file_path_len, sizeof(file_path_len), "/mnt/sd/dump_audio_bs_%d_%d_%d_pcm.len", AUDCAP_BIT, AUDCAP_MODE, p_cap_only->sample_rate);

	/* wait flow_start */
	while (p_cap_only->flow_start == 0) {
		sleep(1);
	}

	/* query physical address of bs buffer
	  (this can ONLY query after hd_audiocap_start() is called !!) */
	hd_audiocap_get(p_cap_only->cap_ctrl, HD_AUDIOCAP_PARAM_BUFINFO, &phy_buf_main);

	/* mmap for bs buffer
	  (just mmap one time only, calculate offset to virtual address later) */
	vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);

	if (vir_addr_main == 0) {
		printf("mmap error\r\n");
		return 0;
	}

	/* open output files */
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		printf("open file (%s) fail....\r\n", file_path_main);
	} else {
		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
	}

	if ((f_out_len = fopen(file_path_len, "wb")) == NULL) {
		printf("open len file (%s) fail....\r\n", file_path_len);
	}

	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	/* pull data test */
	while (p_cap_only->cap_exit == 0) {
		// pull data
		ret = hd_audiocap_pull_out_buf(p_cap_only->cap_path, &data_pull, 200); // >1 = timeout mode

		if (ret == HD_OK) {
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.phy_addr[0]);
			UINT32 size = data_pull.size;
			UINT32 timestamp = hd_gettime_ms();
			// write bs
			if (f_out_main) fwrite(ptr, 1, size, f_out_main);
			if (f_out_main) fflush(f_out_main);

			// write bs len
			if (f_out_len) fprintf(f_out_len, "%d %d\n", size, timestamp);
			if (f_out_len) fflush(f_out_len);

			// release data
			ret = hd_audiocap_release_out_buf(p_cap_only->cap_path, &data_pull);
			if (ret != HD_OK) {
				printf("release buffer failed. ret=%x\r\n", ret);
			}
		}
	}

	/* mummap for bs buffer */
	hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);

	/* close output file */
	if (f_out_main) fclose(f_out_main);
	if (f_out_len) fclose(f_out_len);

	return 0;
}

#if defined(__LINUX)
static void *signal_thread(void *arg)
{
	UINT32 count = 0;
	AUDIO_CAPONLY *p_stream0 = (AUDIO_CAPONLY *)arg;

	while (p_stream0->cap_exit == 0) {
		usleep(50);
		setuid(0);
		count++;
		if (count % 100 == 0) {
			printf("signal count = %d\r\n", count);
		}
	}
	return 0;
}
#endif

EXAMFUNC_ENTRY(hd_audio_capture_only, argc, argv)
{
	HD_RESULT ret;
	INT key;
	AUDIO_CAPONLY caponly = {0};
	pthread_t cap_thread_id;

#if EXT_CODEC
	caponly.cap_i2s_num = 0;
	if (argc > 1) {
		caponly.cap_i2s_num = atoi(argv[1]);// 1: I2S, 0: EAC
		printf("acap use i2s%d\n", caponly.cap_i2s_num);
	}
#endif

	//init hdal
	ret = hd_common_init(0);
	if(ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}
	// init memory
	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		goto exit;
	}
	//capture module init
	ret = init_module();
	if(ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}
	//open capture module
	caponly.sample_rate_max = AUDCAP_SR; //assign by user
	ret = open_module(&caponly);
	if(ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}
	//set audiocap parameter
	caponly.sample_rate = AUDCAP_SR; //assign by user
	ret = set_cap_param(caponly.cap_path, caponly.sample_rate);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}
	//create capture thread
	ret = pthread_create(&cap_thread_id, NULL, capture_thread, (void *)&caponly);
	if (ret < 0) {
		printf("create encode thread failed");
		goto exit;
	}

	//start capture module
	hd_audiocap_start(caponly.cap_path);

	caponly.flow_start = 1;

	printf("Enter q to exit, Enter d to debug\n");
	while (1) {
		key = NVT_EXAMSYS_GETCHAR();
		if (key == 'q' || key == 0x3) {
			caponly.cap_exit = 1;
			break;
		}

		if (key == 'm') {
			system("mem r 0x2f0150000"); //eac
			system("mem r 0x2f0151000"); //dai
		}

		#if defined(__LINUX)
		if (key == 'n') {
			pthread_t  signal_thread_id;

			printf("test signal\r\n");
			ret = pthread_create(&signal_thread_id, NULL, signal_thread, (void *)&caponly);
			if (ret < 0) {
				printf("create signal thread failed");
				break;
			}
		}
		#endif

		#if (DEBUG_MENU == 1)
		if (key == 'd') {
			hd_debug_run_menu(); // call debug menu
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
		#endif
	}

	pthread_join(cap_thread_id, NULL);

	//stop capture module
	hd_audiocap_stop(caponly.cap_path);

exit:
	//close all module
	ret = close_module(&caponly);
	if(ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}
	//uninit all module
	ret = exit_module();
	if(ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}
	// uninit memory
	ret = mem_exit();
	if(ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}

	// uninit hdal
	ret = hd_common_uninit();
	if(ret != HD_OK) {
		printf("common-uninit fail=%d\n", ret);
	}

	return 0;
}
