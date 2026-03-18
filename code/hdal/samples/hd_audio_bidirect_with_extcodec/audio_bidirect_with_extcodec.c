/**
	@brief Sample code of audio capture and output with extcodec.\n

	@file audio_bidirect_with_extcodec.c

	@author HM Tseng

	@ingroup mhdal

	@note This file is modified from audio_capture_only.c and audio_output_only.c.

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
#include "vendor_audioout.h"
#define EXT_CKRATIO  32
#define EXT_TDMCH    2
#endif

#define LOOPBACK  0

#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <kwrap/task.h>
#define sleep(x)    vos_task_delay_ms(1000*x)
#define usleep(x)   vos_task_delay_us(x)
#endif

#define PRELOAD_BITSTREAM   0
#define DUMP_DAI_REG        0

#define DEBUG_MENU 1
#define BITSTREAM_SIZE      12800
#if PRELOAD_BITSTREAM
#define FRAME_SAMPLES       102400
#define USER_BUFFER_SIZE    0x200000
#else
#define FRAME_SAMPLES       1024
#define USER_BUFFER_SIZE    0x100000
#endif
#define AUD_BUFFER_CNT      5
#define ACAP_WAIT_MS        -1 //blocking
#define AOUT_WAIT_MS        -1 //blocking
#define TIME_DIFF(new_val, old_val)     ((int)(new_val) - (int)(old_val))

#define CHKPNT    printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)   printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)   printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

///////////////////////////////////////////////////////////////////////////////

static int mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	/* dummy buffer, not for audio module */
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = 0x1000;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	/* user buffer for bs pushing in */
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_USER_POOL_BEGIN;
	mem_cfg.pool_info[1].blk_size = USER_BUFFER_SIZE;
	mem_cfg.pool_info[1].blk_cnt = 1;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;

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

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_audio_cap_ctrl, HD_AUDIO_SR sample_rate, UINT32 i2s_num, HD_AUDIO_SOUND_MODE mode)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_DEV_CONFIG audio_cfg_param = {0};
	HD_AUDIOCAP_DRV_CONFIG audio_driver_cfg_param = {0};
#if LOOPBACK
	VENDOR_AUDIOCAP_LOOPBACK_CONFIG vendor_loopback = {0};
#endif
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
	audio_cfg_param.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cfg_param.in_max.mode = mode;
	audio_cfg_param.in_max.frame_sample = FRAME_SAMPLES;
	audio_cfg_param.frame_num_max = 10;
	audio_cfg_param.out_max.sample_rate = 0;

	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_cfg_param);
	if (ret != HD_OK) {
		return ret;
	}

	/*set audio capture driver parameters*/
	audio_driver_cfg_param.mono = HD_AUDIO_MONO_RIGHT;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_driver_cfg_param);

#if EXT_CODEC
	//set for external codec input
	snprintf(vendor_config.driver_name, VENDOR_AUDIOCAP_NAME_LEN-1, "nvt_aud_emu");
	vendor_config.aud_init_cfg.pin_cfg.pinmux.audio_pinmux  = 0x12; //PIN_AUDIO_CFG_I2S_2ND_PINMUX | PIN_AUDIO_CFG_MCLK

	vendor_config.aud_init_cfg.i2s_cfg.bit_clk_ratio = EXT_CKRATIO;
	vendor_config.aud_init_cfg.i2s_cfg.bit_width     = 16;
	vendor_config.aud_init_cfg.i2s_cfg.tdm_ch        = EXT_TDMCH;
	vendor_config.aud_init_cfg.i2s_cfg.op_mode       = 0; // acap (rx) as slave

	ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_EXT, (VOID *)&vendor_config);
#endif

#if LOOPBACK
	vendor_loopback.enabled = TRUE;
	if (mode == HD_AUDIO_SOUND_MODE_STEREO) {
		vendor_loopback.lb_channel = HD_AUDIOCAP_LB_CH_STEREO;
	} else {
		vendor_loopback.lb_channel = HD_AUDIOCAP_LB_CH_RIGHT;
	}
	ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_LOOPBACK_CONFIG, (VOID *)&vendor_loopback);
#endif

	*p_audio_cap_ctrl = audio_cap_ctrl;

	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID audio_cap_path, HD_AUDIO_SR sample_rate, HD_AUDIO_SOUND_MODE mode)
{
	HD_RESULT ret = HD_OK;
	//set hd_audiocapture input parameters
	HD_AUDIOCAP_IN audio_cap_param = {0};
	HD_AUDIOCAP_OUT audio_cap_out_param = {0};

	audio_cap_param.sample_rate = sample_rate;
	audio_cap_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cap_param.mode = mode;
	audio_cap_param.frame_sample = FRAME_SAMPLES;
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_IN, &audio_cap_param);
	if (ret != HD_OK) {
		return ret;
	}

	//set hd_audiocapture output parameters
	audio_cap_out_param.sample_rate = 0;
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_OUT, &audio_cap_out_param);
	if (ret != HD_OK) {
		return ret;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_out_cfg(HD_PATH_ID *p_audio_out_ctrl, HD_AUDIO_SR sample_rate, UINT32 i2s_num, HD_AUDIO_SOUND_MODE mode)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOOUT_DEV_CONFIG audio_cfg_param = {0};
	HD_AUDIOOUT_DRV_CONFIG audio_driver_cfg_param = {0};
	HD_PATH_ID audio_out_ctrl = 0;
#if EXT_CODEC
	VENDOR_AUDIOOUT_INIT_CFG vendor_config = {0};
	{
		ret = hd_audioout_open(0, HD_AUDIOOUT_0_CTRL, &audio_out_ctrl); //open this for device control
	}
#else
	ret = hd_audioout_open(0, HD_AUDIOOUT_0_CTRL, &audio_out_ctrl); //open this for device control
#endif
	if (ret != HD_OK) {
		return ret;
	}

	/*set audio out maximum parameters*/
	audio_cfg_param.out_max.sample_rate = sample_rate;
	audio_cfg_param.out_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cfg_param.out_max.mode = mode;
	audio_cfg_param.frame_sample_max = FRAME_SAMPLES;
	audio_cfg_param.frame_num_max = 10;
	audio_cfg_param.in_max.sample_rate = 0;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_DEV_CONFIG, &audio_cfg_param);
	if (ret != HD_OK) {
		return ret;
	}

	/*set audio out driver parameters*/
	audio_driver_cfg_param.mono = HD_AUDIO_MONO_RIGHT;
#if EXT_CODEC
	audio_driver_cfg_param.output = HD_AUDIOOUT_OUTPUT_I2S;
#else
	audio_driver_cfg_param.output = HD_AUDIOOUT_OUTPUT_LINE;
#endif
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_DRV_CONFIG, &audio_driver_cfg_param);
#if EXT_CODEC
	if (ret != HD_OK) {
		return ret;
	}

	{
		printf("322222222\r\n");
		snprintf(vendor_config.driver_name, VENDOR_AUDIOOUT_NAME_LEN-1, "nvt_aud_emu");
		vendor_config.aud_init_cfg.pin_cfg.pinmux.audio_pinmux  = 0x11; //PIN_AUDIO_CFG_I2S | PIN_AUDIO_CFG_MCLK
		vendor_config.aud_init_cfg.pin_cfg.pinmux.cmd_if_pinmux = 0;//0x1000; //PIN_I2C_CFG_CH4
		vendor_config.aud_init_cfg.i2s_cfg.bit_clk_ratio = EXT_CKRATIO;
		vendor_config.aud_init_cfg.i2s_cfg.bit_width     = 16;
		vendor_config.aud_init_cfg.i2s_cfg.tdm_ch        = EXT_TDMCH;
		vendor_config.aud_init_cfg.i2s_cfg.op_mode       = 1; // aout (tx) as master
		vendor_audioout_set(audio_out_ctrl, VENDOR_AUDIOOUT_ITEM_EXT, (VOID *)&vendor_config);
	}
#endif

	*p_audio_out_ctrl = audio_out_ctrl;

	return ret;
}


static HD_RESULT set_out_param(HD_PATH_ID audio_out_ctrl, HD_PATH_ID audio_out_path, HD_AUDIO_SR sample_rate, HD_AUDIO_SOUND_MODE mode)
{
	HD_RESULT ret = HD_OK;
	//set hd_audioout output parameters
	HD_AUDIOOUT_OUT audio_out_out_param = {0};
	HD_AUDIOOUT_VOLUME audio_out_vol = {0};
	HD_AUDIOOUT_IN audio_out_in_param = {0};

	audio_out_out_param.sample_rate = sample_rate;
	audio_out_out_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_out_out_param.mode = mode;
	ret = hd_audioout_set(audio_out_path, HD_AUDIOOUT_PARAM_OUT, &audio_out_out_param);
	if (ret != HD_OK) {
		return ret;
	}

	//set hd_audioout volume
	audio_out_vol.volume = 100;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_VOLUME, &audio_out_vol);
	if (ret != HD_OK) {
		return ret;
	}

	//set hd_audioout input parameters
	audio_out_in_param.sample_rate = 0;
	ret = hd_audioout_set(audio_out_path, HD_AUDIOOUT_PARAM_IN, &audio_out_in_param);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _AUDIO_CAP_OUT {
	HD_AUDIO_SR sample_rate_max;
	HD_AUDIO_SR sample_rate;
	HD_AUDIO_SOUND_MODE mode;

	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;
	UINT32 cap_exit;
	UINT32 cap_pause;
	UINT32 flow_start;
	UINT32 cap_i2s_num;
	UINT32 cap_depack;
	INT32  cap_timeout_ms;

	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;
	UINT32 out_exit;
	UINT32 out_pause;
	UINT32 out_i2s_num;
	INT32  out_timeout_ms;

	UINT32 flow_test;
	UINT32 preload_done;
} AUDIO_CAP_OUT;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_init()) != HD_OK)
		return ret;
	if((ret = hd_audioout_init()) != HD_OK)
		return ret;
	return HD_OK;
}


static HD_RESULT open_module(AUDIO_CAP_OUT *p_cap_out)
{
	HD_RESULT ret;
	ret = set_cap_cfg(&p_cap_out->cap_ctrl, p_cap_out->sample_rate_max, p_cap_out->cap_i2s_num, p_cap_out->mode);
	if (ret != HD_OK) {
		printf("set cap-cfg fail\n");
		return HD_ERR_NG;
	}
	ret = set_out_cfg(&p_cap_out->out_ctrl, p_cap_out->sample_rate_max, p_cap_out->out_i2s_num, p_cap_out->mode);
	if (ret != HD_OK) {
		printf("set out-cfg fail\n");
		return HD_ERR_NG;
	}
	{
		if((ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_cap_out->cap_path)) != HD_OK)
			return ret;
	}
	{
		if((ret = hd_audioout_open(HD_AUDIOOUT_0_IN_0, HD_AUDIOOUT_0_OUT_0, &p_cap_out->out_path)) != HD_OK)
			return ret;
	}
	return HD_OK;
}

static HD_RESULT close_module(AUDIO_CAP_OUT *p_cap_out)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_close(p_cap_out->cap_path)) != HD_OK)
		return ret;
	if((ret = hd_audioout_close(p_cap_out->out_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_uninit()) != HD_OK)
		return ret;
	if((ret = hd_audioout_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}

void test15(AUDIO_CAP_OUT *p_cap_out)
{
	system("echo r dumpinfo > /proc/audio_module/cmd");
}

static void audio_pack(UINTPTR input_addr, UINTPTR output_addr, UINT32 in_sample_cnt, UINT32 channel_num, UINT32 channel)
{
	UINT32 i;
	for (i = 0; i < in_sample_cnt; i++) {
		((INT16 *)(output_addr))[(i * channel_num) + channel] = ((INT16 *)(input_addr))[i];
	}
}

static void *capture_thread(void *arg)
{
	HD_RESULT ret = HD_OK;
#if LOOPBACK
	VENDOR_AUDIOCAP_AUDIO_FRAME  data_pull;
#else
	HD_AUDIO_FRAME  data_pull;
#endif
	UINTPTR vir_addr_main;
	HD_AUDIOCAP_BUFINFO phy_buf_main;
	char file_path_main[64], file_path_len[64];
	FILE *f_out_main, *f_out_len;
#if LOOPBACK
	char file_path_main2[64], file_path_len2[64];
	FILE *f_out_main2, *f_out_len2;
#endif
	AUDIO_CAP_OUT *p_cap_only = (AUDIO_CAP_OUT *)arg;

	#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

	/* config pattern name */
	snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/audio_bs_%d_%d_%d_pcm_cap.dat", HD_AUDIO_BIT_WIDTH_16, p_cap_only->mode, p_cap_only->sample_rate);
	snprintf(file_path_len, sizeof(file_path_len), "/mnt/sd/audio_bs_%d_%d_%d_pcm_cap.len", HD_AUDIO_BIT_WIDTH_16, p_cap_only->mode, p_cap_only->sample_rate);

#if LOOPBACK
	/* config pattern name */
	snprintf(file_path_main2, sizeof(file_path_main2), "/mnt/sd/audio_bs_%d_%d_%d_pcm_lb.dat", HD_AUDIO_BIT_WIDTH_16, p_cap_only->mode, p_cap_only->sample_rate);
	snprintf(file_path_len2, sizeof(file_path_len2), "/mnt/sd/audio_bs_%d_%d_%d_pcm_lb.len", HD_AUDIO_BIT_WIDTH_16, p_cap_only->mode, p_cap_only->sample_rate);
#endif

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

#if LOOPBACK
	/* open output files */
	if ((f_out_main2 = fopen(file_path_main2, "wb")) == NULL) {
		printf("open file (%s) fail....\r\n", file_path_main2);
	} else {
		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main2);
	}

	if ((f_out_len2 = fopen(file_path_len2, "wb")) == NULL) {
		printf("open len file (%s) fail....\r\n", file_path_len2);
	}
#endif

	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	/* pull data test */
	while (1) {
retry1:
		if (p_cap_only->cap_exit == 1) {
			printf("cap_exit\n");
			break;
		}

		if (p_cap_only->cap_pause == 1) {
			usleep(10000);
			goto retry1;
		}

		// pull data
#if LOOPBACK
		data_pull.wait_ms = p_cap_only->cap_timeout_ms; // >1 = timeout mode
		ret = vendor_audiocap_get(p_cap_only->cap_path, VENDOR_AUDIOCAP_ITEM_AUDIO_FRAME, &data_pull);
#else
		ret = hd_audiocap_pull_out_buf(p_cap_only->cap_path, &data_pull, p_cap_only->cap_timeout_ms); // >1 = timeout mode
#endif

		if (ret == HD_OK) {
#if LOOPBACK
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.audio_frame.phy_addr[0]);
			UINT32 size = data_pull.audio_frame.size;
			UINT8 *ptr2 = (UINT8 *)PHY2VIRT_MAIN(data_pull.audio_aec_ref_frame.phy_addr[0]);
			UINT32 size2 = data_pull.audio_aec_ref_frame.size;
#else
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.phy_addr[0]);
			UINT32 size = data_pull.size;
#endif
			UINT32 timestamp = hd_gettime_ms();
			//printf("buf=(0x%lx, 0x%lx)\n", data_pull.phy_addr[0], data_pull.size);

			if (!p_cap_only->flow_test) {

				// depack
				if (p_cap_only->cap_depack) {
					ULONG ptr_offset;
					UINT32 mono_size, write_size;
					UINT32 channel_idx, channel_num = EXT_TDMCH;
					UINT8 *tmp_ptr;

					mono_size = size / channel_num;
					ptr_offset = (ULONG)ptr;
					tmp_ptr = calloc(1, (mono_size * channel_num));
					for (channel_idx = 0; channel_idx < channel_num; channel_idx++) {
						audio_pack((UINTPTR)ptr_offset, (UINTPTR)tmp_ptr, (mono_size / 2), channel_num, channel_idx);
						ptr_offset = ptr_offset + (ULONG)mono_size;
					}
					write_size = mono_size * channel_num;

					// write bs
					if (f_out_main) fwrite(tmp_ptr, 1, write_size, f_out_main);
					if (f_out_main) fflush(f_out_main);

					// write bs len
					if (f_out_len) fprintf(f_out_len, "%d %d\n", size, timestamp);
					if (f_out_len) fflush(f_out_len);

					free(tmp_ptr);

#if LOOPBACK
					// write bs
					if (f_out_main2) fwrite(ptr2, 1, size2, f_out_main2);
					if (f_out_main2) fflush(f_out_main2);

					// write bs len
					if (f_out_len2) fprintf(f_out_len2, "%d %d\n", size2, timestamp);
					if (f_out_len2) fflush(f_out_len2);
#endif
				} else {
					// write bs
					if (f_out_main) fwrite(ptr, 1, size, f_out_main);
					if (f_out_main) fflush(f_out_main);

					// write bs len
					if (f_out_len) fprintf(f_out_len, "%d %d\n", size, timestamp);
					if (f_out_len) fflush(f_out_len);

#if LOOPBACK
					// write bs
					if (f_out_main2) fwrite(ptr2, 1, size2, f_out_main2);
					if (f_out_main2) fflush(f_out_main2);

					// write bs len
					if (f_out_len2) fprintf(f_out_len2, "%d %d\n", size2, timestamp);
					if (f_out_len2) fflush(f_out_len2);
#endif
				}
			}

			// release data
#if LOOPBACK
			ret = hd_audiocap_release_out_buf(p_cap_only->cap_path, &(data_pull.audio_frame));
#else
			ret = hd_audiocap_release_out_buf(p_cap_only->cap_path, &data_pull);
#endif
			if (ret != HD_OK) {
				printf("release buffer failed. ret=%x\r\n", ret);
			}
			//printf("bs_size=(%d, 0x%x)\n", size, size);
		}
	}

	/* mummap for bs buffer */
	hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);

	/* close output file */
	if (f_out_main) fclose(f_out_main);
	if (f_out_len) fclose(f_out_len);

#if LOOPBACK
	/* close output file */
	if (f_out_main2) fclose(f_out_main2);
	if (f_out_len2) fclose(f_out_len2);
#endif

	return 0;
}

static void *playback_thread(void *arg)
{
	INT ret, bs_size, result;
	CHAR filename[50];
	FILE *bs_fd, *len_fd;
	HD_AUDIO_FRAME  bs_in_buf = {0};
	HD_COMMON_MEM_VB_BLK blk;
	UINTPTR pa, va;
	UINT32 blk_size = USER_BUFFER_SIZE;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	UINTPTR bs_buf_start, bs_buf_curr, bs_buf_end;
	INT au_frame_ms, elapse_time, au_buf_time, timestamp;
	UINT start_time, data_time;
	AUDIO_CAP_OUT *p_out_only = (AUDIO_CAP_OUT*)arg;

	/* read test pattern */
#if PRELOAD_BITSTREAM
	snprintf(filename, sizeof(filename), "/mnt/sd/audio_bs_%d_%d_%d_pcm_out_pl.dat", HD_AUDIO_BIT_WIDTH_16, p_out_only->mode, p_out_only->sample_rate);
#else
	snprintf(filename, sizeof(filename), "/mnt/sd/audio_bs_%d_%d_%d_pcm_out.dat", HD_AUDIO_BIT_WIDTH_16, p_out_only->mode, p_out_only->sample_rate);
#endif
	bs_fd = fopen(filename, "rb");
	if (bs_fd == NULL) {
		printf("[ERROR] Open %s failed!!\n", filename);
		return 0;
	}
	printf("play file: [%s]\n", filename);

#if PRELOAD_BITSTREAM
	snprintf(filename, sizeof(filename), "/mnt/sd/audio_bs_%d_%d_%d_pcm_out_pl.len", HD_AUDIO_BIT_WIDTH_16, p_out_only->mode, p_out_only->sample_rate);
#else
	snprintf(filename, sizeof(filename), "/mnt/sd/audio_bs_%d_%d_%d_pcm_out.len", HD_AUDIO_BIT_WIDTH_16, p_out_only->mode, p_out_only->sample_rate);
#endif
	len_fd = fopen(filename, "rb");
	if (len_fd == NULL) {
		printf("[ERROR] Open %s failed!!\n", filename);
		goto play_fclose;
	}
	printf("len file: [%s]\n", filename);

	au_frame_ms = FRAME_SAMPLES * 1000 / p_out_only->sample_rate - 5; // the time(in ms) of each audio frame
	start_time = hd_gettime_ms();
	data_time = 0;

	/* get memory */
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, blk_size, ddr_id); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail, blk = 0x%x\n", blk);
		goto play_fclose;
	}
	pa = hd_common_mem_blk2pa(blk); // get physical addr
	if (pa == 0) {
		printf("blk2pa fail, blk(0x%x)\n", blk);
		goto play_fclose;
	}
	if (pa > 0) {
		va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size); // Get virtual addr
		if (va == 0) {
			printf("get va fail, va(0x%x)\n", blk);
			goto play_fclose;
		}
		/* allocate bs buf */
		bs_buf_start = va;
		bs_buf_curr = bs_buf_start;
		bs_buf_end = bs_buf_start + (ULONG)blk_size;
		printf("alloc bs_buf: start(0x%lx) curr(0x%lx) end(0x%lx) size(0x%lx)\n", (unsigned long)bs_buf_start, (unsigned long)bs_buf_curr, (unsigned long)bs_buf_end, (unsigned long)blk_size);
	}
#if PRELOAD_BITSTREAM
	// read bs data
	{
		/* get bs size */
		if (fscanf(len_fd, "%d %d\n", &bs_size, &timestamp) == EOF) {
			// reach EOF, read from the beginning
			fseek(bs_fd, 0, SEEK_SET);
			fseek(len_fd, 0, SEEK_SET);
			if (fscanf(len_fd, "%d %d\n", &bs_size, &timestamp) == EOF) {
				printf("[ERROR] fscanf error\n");
			}
		}
		if (bs_size == 0) {// || bs_size > BITSTREAM_SIZE) {
			printf("Invalid bs_size(%d)\n", bs_size);
		}

		/* read bs from file */
		result = fread((void *)bs_buf_curr, 1, bs_size, bs_fd);
		if (result != bs_size) {
			printf("reading error\n");
		}
		printf("bs_size=(%d, 0x%x)\n", bs_size, bs_size);

		p_out_only->preload_done = 1;
	}
#endif

	while (p_out_only->flow_start == 0) {
		sleep(1);
	}

	while (1) {
retry2:
		if (p_out_only->out_exit == 1) {
			break;
		}

		if (p_out_only->out_pause == 1) {
			usleep(10000);
			goto retry2;
		}

		elapse_time = TIME_DIFF(hd_gettime_ms(), start_time);
		au_buf_time = data_time - elapse_time;
		if (au_buf_time > AUD_BUFFER_CNT * au_frame_ms) {
			//usleep(au_frame_ms);
			//goto retry;
		}
#if PRELOAD_BITSTREAM
		// already read bs data
#else
		/* get bs size */
		if (fscanf(len_fd, "%d %d\n", &bs_size, &timestamp) == EOF) {
			// reach EOF, read from the beginning
			fseek(bs_fd, 0, SEEK_SET);
			fseek(len_fd, 0, SEEK_SET);
			if (fscanf(len_fd, "%d %d\n", &bs_size, &timestamp) == EOF) {
				printf("[ERROR] fscanf error\n");
				continue;
			}
		}
		if (bs_size == 0 || bs_size > BITSTREAM_SIZE) {
			printf("Invalid bs_size(%d)\n", bs_size);
			continue;
		}

		/* check bs buf rollback */
		if ((bs_buf_curr + (ULONG)bs_size) > bs_buf_end) {
			bs_buf_curr = bs_buf_start;
		}

		/* read bs from file */
		result = fread((void *)bs_buf_curr, 1, bs_size, bs_fd);
		if (result != bs_size) {
			printf("reading error\n");
			continue;
		}
#endif
		bs_in_buf.sign = MAKEFOURCC('A','F','R','M');
		bs_in_buf.phy_addr[0] = pa + (bs_buf_curr - bs_buf_start); // needs to add offset
		bs_in_buf.size = bs_size;
		bs_in_buf.ddr_id = ddr_id;
		bs_in_buf.timestamp = hd_gettime_us();
		bs_in_buf.bit_width = HD_AUDIO_BIT_WIDTH_16;
		bs_in_buf.sound_mode = p_out_only->mode;
		bs_in_buf.sample_rate = p_out_only->sample_rate;

		/* push in buffer */
		data_time += au_frame_ms;
resend:
		ret = hd_audioout_push_in_buf(p_out_only->out_path, &bs_in_buf, p_out_only->out_timeout_ms);
		if (ret != HD_OK) {
			printf("hd_audioout_push_in_buf fail, ret(%d)\n", ret);
			usleep(10000);
			if (p_out_only->out_timeout_ms == -1)
				goto resend;
		}
#if DUMP_DAI_REG
		{
			system("mem r 0x2f0151000"); //dai
		}
#endif
#if PRELOAD_BITSTREAM
		printf("out_exit\n");
		p_out_only->out_exit = 1;
#else
		bs_buf_curr += ALIGN_CEIL_4(bs_size); // shift to next
#endif
	}

	/* release memory */
	hd_common_mem_munmap((void*)va, blk_size);
	ret = hd_common_mem_release_block(blk);
	if (HD_OK != ret) {
		printf("release blk fail, ret(%d)\n", ret);
		goto play_fclose;
	}

play_fclose:
	if (bs_fd != NULL) { fclose(bs_fd);}
	if (len_fd != NULL) { fclose(len_fd);}

	return 0;
}


EXAMFUNC_ENTRY(hd_audio_bidirect_with_aec, argc, argv)
{
	HD_RESULT ret;
	INT key;
	AUDIO_CAP_OUT cap_out = {0};
	pthread_t cap_thread_id;
	pthread_t out_thread_id;

	// query program options
	if (argc == 3) {
		UINT32 sample_rate;
		UINT32 mode;

		sample_rate = atoi(argv[1]);
		mode = atoi(argv[2]);

		if (sample_rate == 0) {
			cap_out.sample_rate = HD_AUDIO_SR_8000;
		} else if (sample_rate == 1) {
			cap_out.sample_rate = HD_AUDIO_SR_16000;
		} else if (sample_rate == 2) {
			cap_out.sample_rate = HD_AUDIO_SR_32000;
		} else if (sample_rate == 3) {
			cap_out.sample_rate = HD_AUDIO_SR_48000;
		} else if (sample_rate == 4) {
			cap_out.sample_rate = HD_AUDIO_SR_96000;
		} else {
			cap_out.sample_rate = HD_AUDIO_SR_16000;
		}

		if (mode == 0) {
			cap_out.mode= HD_AUDIO_SOUND_MODE_MONO;
		} else if (mode == 1) {
			cap_out.mode = HD_AUDIO_SOUND_MODE_STEREO;
		} else {
			cap_out.mode = HD_AUDIO_SOUND_MODE_MONO;
		}
		printf("sample rate %d, mode %d\r\n", cap_out.sample_rate, cap_out.mode);
	} else if (argc == 4) {
		UINT32 sample_rate;
		UINT32 mode;

		sample_rate = atoi(argv[1]);
		mode = atoi(argv[2]);

		if (sample_rate == 0) {
			cap_out.sample_rate = HD_AUDIO_SR_8000;
		} else if (sample_rate == 1) {
			cap_out.sample_rate = HD_AUDIO_SR_16000;
		} else if (sample_rate == 2) {
			cap_out.sample_rate = HD_AUDIO_SR_32000;
		} else if (sample_rate == 3) {
			cap_out.sample_rate = HD_AUDIO_SR_48000;
		} else if (sample_rate == 4) {
			cap_out.sample_rate = HD_AUDIO_SR_96000;
		} else {
			cap_out.sample_rate = HD_AUDIO_SR_16000;
		}

		if (mode == 0) {
			cap_out.mode= HD_AUDIO_SOUND_MODE_MONO;
		} else if (mode == 1) {
			cap_out.mode = HD_AUDIO_SOUND_MODE_STEREO;
		} else {
			cap_out.mode = HD_AUDIO_SOUND_MODE_MONO;
		}
		cap_out.flow_test = atoi(argv[3]);
		printf("sample rate %d, mode %d, flow_test %d\r\n", cap_out.sample_rate, cap_out.mode, cap_out.flow_test);
	} else if (argc > 4) {
		UINT32 sample_rate;
		UINT32 mode;

		sample_rate = atoi(argv[1]);
		mode = atoi(argv[2]);

		if (sample_rate == 0) {
			cap_out.sample_rate = HD_AUDIO_SR_8000;
		} else if (sample_rate == 1) {
			cap_out.sample_rate = HD_AUDIO_SR_16000;
		} else if (sample_rate == 2) {
			cap_out.sample_rate = HD_AUDIO_SR_32000;
		} else if (sample_rate == 3) {
			cap_out.sample_rate = HD_AUDIO_SR_48000;
		} else if (sample_rate == 4) {
			cap_out.sample_rate = HD_AUDIO_SR_96000;
		} else {
			cap_out.sample_rate = HD_AUDIO_SR_16000;
		}

		if (mode == 0) {
			cap_out.mode= HD_AUDIO_SOUND_MODE_MONO;
		} else if (mode == 1) {
			cap_out.mode = HD_AUDIO_SOUND_MODE_STEREO;
		} else {
			cap_out.mode = HD_AUDIO_SOUND_MODE_MONO;
		}
		cap_out.flow_test = atoi(argv[3]);
		printf("sample rate %d, mode %d, flow_test %d\r\n", cap_out.sample_rate, cap_out.mode, cap_out.flow_test);

		if (argc > 4) {
			cap_out.cap_i2s_num = atoi(argv[4]);
			printf("acap use i2s%d\n",cap_out.cap_i2s_num);
		}

		if (argc > 5) {
			cap_out.out_i2s_num = atoi(argv[5]);
			printf("aout use i2s%d\n",cap_out.out_i2s_num);
		}

		if (argc > 6) {
			cap_out.cap_timeout_ms = atoi(argv[6]);
			printf("acap timeout %dms\n",cap_out.cap_timeout_ms);
		}

		if (argc > 7) {
			cap_out.out_timeout_ms = atoi(argv[7]);
			printf("aout timeout %dms\n",cap_out.out_timeout_ms);
		}

	} else {
		cap_out.sample_rate = HD_AUDIO_SR_16000;
		cap_out.mode = HD_AUDIO_SOUND_MODE_MONO;
	}

	if (cap_out.cap_timeout_ms == 0)
		cap_out.cap_timeout_ms = ACAP_WAIT_MS;
	if (cap_out.out_timeout_ms == 0)
		cap_out.out_timeout_ms = AOUT_WAIT_MS;

	//
	//	cap_out.cap_depack = 1;

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
	//cap_out module init
	ret = init_module();
	if(ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}
	//open cap_out module
	cap_out.sample_rate_max = HD_AUDIO_SR_48000; //assign by user
	ret = open_module(&cap_out);
	if(ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}
	//set audiocap parameter
	//cap_out.sample_rate = HD_AUDIO_SR_16000; //assign by user
	ret = set_cap_param(cap_out.cap_path, cap_out.sample_rate, cap_out.mode);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}
	//set audioout parameter
	ret = set_out_param(cap_out.out_ctrl, cap_out.out_path, cap_out.sample_rate, cap_out.mode);
	if (ret != HD_OK) {
		printf("set out fail=%d\n", ret);
		goto exit;
	}

	//create capture thread
	ret = pthread_create(&cap_thread_id, NULL, capture_thread, (void *)&cap_out);
	if (ret < 0) {
		printf("create record thread failed");
		goto exit;
	}

	if (!cap_out.flow_test) {
		//create output thread
		ret = pthread_create(&out_thread_id, NULL, playback_thread, (void *)&cap_out);
		if (ret < 0) {
			printf("create playback thread failed");
			goto exit;
		}
	}

	//start cap_out module
	hd_audiocap_start(cap_out.cap_path);

	if (!cap_out.flow_test) {
#if PRELOAD_BITSTREAM
		while (cap_out.preload_done == 0) {
			sleep(1);
		}
#endif
		hd_audioout_start(cap_out.out_path);
	}

	cap_out.flow_start = 1;

	printf("\r\nEnter q to exit, Enter d to debug\r\n");
	while (1) {
		key = NVT_EXAMSYS_GETCHAR();
		if (key == 'q' || key == 0x3) {
			cap_out.cap_exit = 1;
			cap_out.out_exit = 1;
			break;
		}

		if (key == 'm') {
			if (cap_out.cap_i2s_num == 0 || cap_out.out_i2s_num == 0) {
				system("mem r 0x2f0151000"); //dai
			}
		}

		if (key == 'c') {
			system("mem r 0x2f0020000"); //cg
			system("cat /sys/kernel/debug/clk/clk_summary");
		}

		#if (DEBUG_MENU == 1)
		if (key == 'd') {
			hd_debug_run_menu(); // call debug menu
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
		#endif
		if (key == 'r') {
			test15(&cap_out); // call debug menu
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
	}
	pthread_join(cap_thread_id, NULL);
	if (!cap_out.flow_test) {
		pthread_join(out_thread_id, NULL);
	}

	//stop cap_out module
	hd_audiocap_stop(cap_out.cap_path);
	if (!cap_out.flow_test) {
		hd_audioout_stop(cap_out.out_path);
	}

exit:
	//close all module
	ret = close_module(&cap_out);
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
