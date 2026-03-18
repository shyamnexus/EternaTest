/*
 * Simple sound playback using ALSA API and libasound.
 *
 * Compile:
 * $ cc -o play sound_playback.c -lasound
 *
 * Usage:
 * $ ./play <sample_rate> <channels> <seconds> < <file>
 *
 * Examples:
 * $ ./play 44100 2 5 < /dev/urandom
 * $ ./play 22050 1 8 < /path/to/file.wav
 *
 * Copyright (C) 2009 Alessandro Ghedini <alessandro@ghedini.me>
 * --------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Alessandro Ghedini wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can
 * buy me a beer in return.
 * --------------------------------------------------------------
 */

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "hdal.h"
#include "hd_debug.h"
#include <kwrap/examsys.h>

#define PCM_DEVICE_REC "hw:0,0"
#define PCM_DEVICE_UAC "hw:0,0"

#define AUDCAP_SR       HD_AUDIO_SR_48000
#define AUDCAP_BIT      HD_AUDIO_BIT_WIDTH_16
#define AUDCAP_MODE     HD_AUDIO_SOUND_MODE_STEREO
#define AUDCAP_MONO     HD_AUDIO_MONO_RIGHT
#define AUDCAP_FRAME    1024

static snd_pcm_t *pcm_handle_uac;
static snd_pcm_uframes_t frames;

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

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_audio_cap_ctrl, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_DEV_CONFIG audio_cfg_param = {0};
	HD_AUDIOCAP_DRV_CONFIG audio_driver_cfg_param = {0};
	HD_PATH_ID audio_cap_ctrl = 0;

	ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		return ret;
	}

	/*set audio capture maximum parameters*/
	audio_cfg_param.in_max.sample_rate = sample_rate;
	audio_cfg_param.in_max.sample_bit = AUDCAP_BIT;
	audio_cfg_param.in_max.mode = AUDCAP_MODE;
	audio_cfg_param.in_max.frame_sample = AUDCAP_FRAME;
	audio_cfg_param.frame_num_max = 10;
	audio_cfg_param.out_max.sample_rate = 0;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_cfg_param);
	if (ret != HD_OK) {
		return ret;
	}

	/*set audio capture driver parameters*/
	audio_driver_cfg_param.mono = AUDCAP_MONO;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_driver_cfg_param);

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
	audio_cap_in_param.frame_sample = AUDCAP_FRAME;
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

	ret = set_cap_cfg(&p_caponly->cap_ctrl, p_caponly->sample_rate_max);

	if (ret != HD_OK) {
		printf("set cap-cfg fail\n");
		return HD_ERR_NG;
	}

	if((ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_caponly->cap_path)) != HD_OK)
		return ret;

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
	int pcm;
	HD_RESULT ret = HD_OK;
	HD_AUDIO_FRAME  data_pull;
	UINTPTR vir_addr_main;
	HD_AUDIOCAP_BUFINFO phy_buf_main;
	AUDIO_CAPONLY *p_cap_only = (AUDIO_CAPONLY *)arg;
	#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))


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

	/* pull data test */
	while (p_cap_only->cap_exit == 0) {

		// pull data
		ret = hd_audiocap_pull_out_buf(p_cap_only->cap_path, &data_pull, 200); // >1 = timeout mode

		if (ret == HD_OK) {
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.phy_addr[0]);

			pcm = snd_pcm_writei(pcm_handle_uac, ptr, AUDCAP_FRAME);
			if (pcm == -EPIPE) {
				printf("XRUN.\n");
				snd_pcm_prepare(pcm_handle_uac);
			} else if (pcm < 0) {
				printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
			}

			// release data
			ret = hd_audiocap_release_out_buf(p_cap_only->cap_path, &data_pull);
			if (ret != HD_OK) {
				printf("release buffer failed. ret=%x\r\n", ret);
			}
		}
	}

	/* mummap for bs buffer */
	hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);

	return 0;
}


EXAMFUNC_ENTRY(uac_in_with_alsa, argc, argv)
{
	int pcm;
	unsigned int rate, channels, tmp;
	HD_RESULT ret;
	INT key;
	AUDIO_CAPONLY caponly = {0};
	pthread_t cap_thread_id;

	snd_pcm_hw_params_t *params;

	if (argc < 3) {
		printf("Usage: %s <sample_rate> <channels> \n",
								argv[0]);
		return -1;
	}

	rate 	 = atoi(argv[1]);
	channels = atoi(argv[2]);

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

	printf("rate = %d, channels = %d\r\n", rate, channels);

	pcm = snd_pcm_open(&pcm_handle_uac, PCM_DEVICE_UAC, SND_PCM_STREAM_PLAYBACK, 0);

	/* Open the PCM device in playback mode */
	if (pcm < 0) {
		printf("ERROR: Can't open \"%s\" PCM device. %s\n", PCM_DEVICE_UAC, snd_strerror(pcm));
		return -1;
	}
	/* Allocate parameters object and fill it with default values*/
	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcm_handle_uac, params);

	/* Set parameters */
	pcm = snd_pcm_hw_params_set_access(pcm_handle_uac, params,SND_PCM_ACCESS_RW_INTERLEAVED);
	if (pcm < 0)
		printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));

	pcm = snd_pcm_hw_params_set_format(pcm_handle_uac, params,SND_PCM_FORMAT_S16_LE);
	if (pcm < 0)
		printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));

	pcm = snd_pcm_hw_params_set_channels(pcm_handle_uac, params, channels);
	if (pcm < 0)
		printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));

	pcm = snd_pcm_hw_params_set_rate_near(pcm_handle_uac, params, &rate, 0);
	if (pcm < 0)
		printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));

	/* Write parameters */
	pcm = snd_pcm_hw_params(pcm_handle_uac, params);
	if (pcm < 0)
		printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));

	/* Resume information */
	printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle_uac));

	printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle_uac)));

	snd_pcm_hw_params_get_channels(params, &tmp);

	snd_pcm_hw_params_get_rate(params, &tmp, 0);
	//printf("rate: %d bps\n", tmp);

	/* Allocate buffer to hold single period */
	snd_pcm_hw_params_get_period_size(params, &frames, 0);

	printf("frames: %d\n", frames);

	//create capture thread
	ret = pthread_create(&cap_thread_id, NULL, capture_thread, (void*)&caponly);
	if (ret < 0) {
		printf("create encode thread failed");
	}

	//start capture module
	hd_audiocap_start(caponly.cap_path);

	caponly.flow_start = 1;

	printf("Enter q to exit\n");
	while (1) {
		key = NVT_EXAMSYS_GETCHAR();
		if (key == 'q' || key == 0x3) {
			caponly.cap_exit = 1;
			break;
		}

		if (key == 'd') {
			hd_debug_run_menu(); // call debug menu
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
	}

	pthread_join(cap_thread_id, NULL);


	//stop capture module
	hd_audiocap_stop(caponly.cap_path);

	snd_pcm_drain(pcm_handle_uac);
	snd_pcm_close(pcm_handle_uac);

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
