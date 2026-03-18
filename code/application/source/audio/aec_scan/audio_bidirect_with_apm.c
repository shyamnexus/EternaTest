/**
	@brief Sample code of using audio processing lib.\n

	@file audio_bidirect_with_apm.c

	@author Gary Ma

	@ingroup CSW

	@note This file is modified from audio_bidirect.c.

	Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "hdal.h"
#include "hd_debug.h"
#include <kwrap/examsys.h>
#include "vendor_audiocapture.h"
#include "vendor_audioout.h"
#include "hd_audioout.h"
#include "apm.h"
#include "wavwriter.h"
#include "wavreader.h"
#include "debug.h"
#include <getopt.h>
#include <sys/stat.h>
#include <math.h>

#if defined(__LINUX)
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <kwrap/task.h>
#define sleep(x) vos_task_delay_ms(1000 * x)
#define usleep(x) vos_task_delay_us(x)
#endif

#define DEBUG_MENU 1

#define AUDIO_BUFFER_COUNT 10 // at least 2
#define MAKE_SURE_VOICE_COUNT 3
#define MAKE_SURE_NO_FAR_VOICE_COUNT 15 // If the far voice still has some tail voice, increase this value

static const float increasing_volume_curve[] = {0, 0.1, 0.3, 0.5, 0.7, 1, 1, 1, 1, 1, 1};
static const float decreasing_volume_curve[] = {0, 0, 0, 0, 0, 0, 0.1, 0.3, 0.5, 0.7, 1};
static const int CURVE_PARAM_COUNT = sizeof(increasing_volume_curve) / sizeof(float);
static int play_curve_index = sizeof(increasing_volume_curve) / sizeof(float);
static int mic_curve_index = sizeof(increasing_volume_curve) / sizeof(float);
static int ref_db_threshold = -50; // lower then this db should not be a voice

enum AEC_MODE
{
	AEC_MODE_NONE,
	AEC_MODE_APM,
	AEC_MODE_HALF_DUPLEX,
	AEC_MODE_COUNT
};
typedef struct _AUDIO_CAP_OUT
{
	HD_AUDIO_SR sample_rate_max;
	HD_AUDIO_SR sample_rate;

	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;
	UINT32 cap_exit;
	UINT32 cap_pause;
	UINT32 flow_start;

	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;
	UINT32 out_exit;
	UINT32 out_pause;

	UINT32 cap_volume;
	UINT32 play_volume;
	int aec_mode;
} AUDIO_CAP_OUT;

static AUDIO_CAP_OUT cap_out = {0};
static void *g_apm = NULL;
static CHAR g_out_dir_path[128] = "/mnt/sd";
static CHAR g_playback_file_name[128] = {0};
static NVT_APM_CONFIG g_config = {0};
static NVT_APM_CONFIG g_playback_config = {0};
static int playback_only = 0;
static UINT32 g_playback_duration = 0;
static short playback_frames[AUDIO_BUFFER_COUNT][960] = {0}; // max: 48000 *2 / 100 = 960
static int playback_frames_enqueue_index = 0;
static int playback_frames_dequeue_index = 1;
static short microphone_in_frames[AUDIO_BUFFER_COUNT][960] = {0}; // max: 48000 *2 / 100 = 960
static int microphone_in_frames_enqueue_index = 0;
static int microphone_in_frames_dequeue_index = 1;
static pthread_t cap_thread_id = 0;
static pthread_t out_thread_id = 0;
///////////////////////////////////////////////////////////////////////////////

static int mem_init(void)
{
	HD_RESULT ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	/* dummy buffer, not for audio module */
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = 0x1000;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	/* user buffer for bs pushing in */
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_USER_POOL_BEGIN;
	mem_cfg.pool_info[1].blk_size = 0x100000;
	mem_cfg.pool_info[1].blk_cnt = 1;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;

	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret)
	{
		LOG("hd_common_mem_init err: %d\r\n", ret);
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

static HD_RESULT set_cap_volume(UINT32 volume)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_VOLUME audio_cap_volume = {0};
	audio_cap_volume.volume = volume;
	hd_audiocap_set(cap_out.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
	if (ret != HD_OK)
	{
		LOGE("hd_audiocap_set fail, audio_cap_volume.volume=%d", audio_cap_volume.volume);
		return ret;
	}
	LOGI("hd_audiocap_set, audio_cap_volume.volume=%d", audio_cap_volume.volume);

	return ret;
}

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_audio_cap_ctrl, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_DEV_CONFIG audio_cfg_param = {0};
	HD_AUDIOCAP_DRV_CONFIG audio_driver_cfg_param = {0};
	VENDOR_AUDIOCAP_LOOPBACK_CONFIG vendor_loopback = {0};
	HD_PATH_ID audio_cap_ctrl = 0;

	ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl); // open this for device control
	if (ret != HD_OK)
	{
		return ret;
	}

	// turn of ALC
	UINT32 alc_en = FALSE;
	ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_ALC_ENABLE, (VOID *)&alc_en);
	if (ret != HD_OK)
	{
		LOGE("vendor_audiocap_set fail");
		return ret;
	}
	LOGI("vendor_audiocap_set, alc_en=%d", alc_en);

#if 0
	UINT32 gain_lvl = VENDOR_AUDIOCAP_GAIN_LEVEL32; // increase gain level count from 8 to 32
	vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL, &gain_lvl);
#endif

	/*set audio capture maximum parameters*/
	audio_cfg_param.in_max.sample_rate = sample_rate;
	audio_cfg_param.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cfg_param.in_max.mode = HD_AUDIO_SOUND_MODE_STEREO;
	audio_cfg_param.in_max.frame_sample = 1024;
	audio_cfg_param.frame_num_max = 10;
	audio_cfg_param.out_max.sample_rate = 0;

	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_cfg_param);
	if (ret != HD_OK)
	{
		return ret;
	}

	/*set audio capture driver parameters*/
	audio_driver_cfg_param.mono = HD_AUDIO_MONO_RIGHT;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_driver_cfg_param);

	vendor_loopback.enabled = TRUE;
	vendor_loopback.lb_channel = HD_AUDIOCAP_LB_CH_LEFT;
	vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_LOOPBACK_CONFIG, &vendor_loopback);

	*p_audio_cap_ctrl = audio_cap_ctrl;

	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID audio_cap_path, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret = HD_OK;
	// set hd_audiocapture input parameters
	HD_AUDIOCAP_IN audio_cap_param = {0};
	HD_AUDIOCAP_OUT audio_cap_out_param = {0};

	audio_cap_param.sample_rate = sample_rate;
	audio_cap_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cap_param.mode = HD_AUDIO_SOUND_MODE_MONO;
	audio_cap_param.frame_sample = sample_rate * (audio_cap_param.mode == HD_AUDIO_SOUND_MODE_MONO ? 1 : 2) / 100; // samples per 10 ms
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_IN, &audio_cap_param);
	if (ret != HD_OK)
	{
		return ret;
	}

	// set hd_audiocapture output parameters
	audio_cap_out_param.sample_rate = 0;
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_OUT, &audio_cap_out_param);
	if (ret != HD_OK)
	{
		return ret;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_out_cfg(HD_PATH_ID *p_audio_out_ctrl, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOOUT_DEV_CONFIG audio_cfg_param = {0};
	HD_AUDIOOUT_DRV_CONFIG audio_driver_cfg_param = {0};
	HD_PATH_ID audio_out_ctrl = 0;

	ret = hd_audioout_open(0, HD_AUDIOOUT_0_CTRL, &audio_out_ctrl); // open this for device control
	if (ret != HD_OK)
	{
		return ret;
	}

	/*set audio out maximum parameters*/
	audio_cfg_param.out_max.sample_rate = sample_rate;
	audio_cfg_param.out_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cfg_param.out_max.mode = HD_AUDIO_SOUND_MODE_STEREO;
	audio_cfg_param.frame_sample_max = 1024;
	audio_cfg_param.frame_num_max = 10;
	audio_cfg_param.in_max.sample_rate = 0;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_DEV_CONFIG, &audio_cfg_param);
	if (ret != HD_OK)
	{
		return ret;
	}

	/*set audio out driver parameters*/
	audio_driver_cfg_param.mono = HD_AUDIO_MONO_LEFT;
	audio_driver_cfg_param.output = HD_AUDIOOUT_OUTPUT_LINE;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_DRV_CONFIG, &audio_driver_cfg_param);

	// keep the power for speaker to avoid "pop" sound
	INT32 pwr_en = TRUE;
	ret = vendor_audioout_set(audio_out_ctrl, VENDOR_AUDIOOUT_ITEM_PREPWR_ENABLE, (VOID *)&pwr_en);
	if (ret != HD_OK)
	{
		LOGE("set VENDOR_AUDIOOUT_ITEM_PREPWR_ENABLE fail");
		return ret;
	}

	*p_audio_out_ctrl = audio_out_ctrl;

	return ret;
}

static HD_RESULT set_out_param(HD_PATH_ID audio_out_ctrl, HD_PATH_ID audio_out_path, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret = HD_OK;
	// set hd_audioout output parameters
	HD_AUDIOOUT_OUT audio_out_out_param = {0};
	HD_AUDIOOUT_VOLUME audio_out_vol = {0};
	HD_AUDIOOUT_IN audio_out_in_param = {0};

	audio_out_out_param.sample_rate = sample_rate;
	audio_out_out_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_out_out_param.mode = HD_AUDIO_SOUND_MODE_MONO;
	ret = hd_audioout_set(audio_out_path, HD_AUDIOOUT_PARAM_OUT, &audio_out_out_param);
	if (ret != HD_OK)
	{
		return ret;
	}

	// set hd_audioout volume
	audio_out_vol.volume = cap_out.play_volume;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_VOLUME, &audio_out_vol);
	if (ret != HD_OK)
	{
		LOGE("set hd_audioout_set fail");
		return ret;
	}
	LOGI("Set audio_out_vol.volume=%d", audio_out_vol.volume);

	// set hd_audioout input parameters
	audio_out_in_param.sample_rate = 0;
	ret = hd_audioout_set(audio_out_path, HD_AUDIOOUT_PARAM_IN, &audio_out_in_param);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_audiocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_audioout_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module(AUDIO_CAP_OUT *p_cap_out)
{
	HD_RESULT ret;
	ret = set_cap_cfg(&p_cap_out->cap_ctrl, p_cap_out->sample_rate_max);
	if (ret != HD_OK)
	{
		LOG("set cap-cfg fail\n");
		return HD_ERR_NG;
	}
	ret = set_cap_volume(cap_out.cap_volume);
	if (ret != HD_OK)
	{
		LOG("set set_cap_volume fail\n");
		return HD_ERR_NG;
	}
	ret = set_out_cfg(&p_cap_out->out_ctrl, p_cap_out->sample_rate_max);
	if (ret != HD_OK)
	{
		LOG("set out-cfg fail\n");
		return HD_ERR_NG;
	}
	if ((ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_cap_out->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_audioout_open(HD_AUDIOOUT_0_IN_0, HD_AUDIOOUT_0_OUT_0, &p_cap_out->out_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module(AUDIO_CAP_OUT *p_cap_out)
{
	HD_RESULT ret;

	if ((ret = hd_audiocap_close(p_cap_out->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_audioout_close(p_cap_out->out_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_audiocap_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_audioout_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}

void volume_change(short *samp, int sampleCount, float percent)
{
	for (int i = 0; i < sampleCount; i++)
	{
		samp[i] = (int16_t)(samp[i] * percent);
	}
}

void volume_up(int *curve_index)
{
	if (*curve_index < CURVE_PARAM_COUNT - 1)
	{
		(*curve_index)++;
		LOG("volume_up, index=%d (%#x)", *curve_index, curve_index);
	}
}

void volume_down(int *curve_index)
{
	if (*curve_index > 0)
	{
		(*curve_index)--;
		LOG("volume_down, index=%d (%#x)", *curve_index, curve_index);
	}
}

// Get abstime after ms milliseconds
struct timespec get_wait_abstime(long ms)
{
	struct timespec abstime;
	clock_gettime(CLOCK_REALTIME, &abstime);
	abstime.tv_sec += ms / 1000;
	ms = ms % 1000;

	uint64_t ns = ms * 1000000 + abstime.tv_nsec;
	if (ns >= 1000000000)
	{
		// more than 1 second
		abstime.tv_sec += 1;
		ns -= 1000000000;
	}
	abstime.tv_nsec = ns;

	return abstime;
}

void speak_in_turn(bool has_far_voice, int *p_make_sure_voice_count, int *p_mic_curve_index, short *data, int size, bool is_near)
{

	bool increasing = false;
	if (has_far_voice)
	{
		if ((*p_make_sure_voice_count) <= 0)
		{
			if (is_near)
			{
				volume_down(p_mic_curve_index);
			}
			else
			{
				volume_up(p_mic_curve_index);
				increasing = true;
			}
		}
		else
		{
			if ((*p_make_sure_voice_count) > MAKE_SURE_VOICE_COUNT)
				*p_make_sure_voice_count = 3;
			// wait MAKE_SURE_VOICE_COUNT times
			(*p_make_sure_voice_count)--;
		}
	}
	else
	{
		// no far voice
		if ((*p_make_sure_voice_count) >= MAKE_SURE_NO_FAR_VOICE_COUNT)
		{
			if (is_near)
			{
				volume_up(p_mic_curve_index);
				increasing = true;
			}
			else
			{
				volume_down(p_mic_curve_index);
				increasing = true;
			}
		}
		else
		{
			// wait MAKE_SURE_NO_FAR_VOICE_COUNT times
			(*p_make_sure_voice_count)++;
		}
	}

	if (increasing)
	{
		if (increasing_volume_curve[*p_mic_curve_index] != 1)
		{
			volume_change(data, size >> 1, increasing_volume_curve[*p_mic_curve_index]);
		}
	}
	else
	{
		if (decreasing_volume_curve[*p_mic_curve_index] != 1)
		{
			volume_change(data, size >> 1, decreasing_volume_curve[*p_mic_curve_index]);
		}
	}
}

static void *capture_thread(void *arg)
{
	HD_RESULT ret = HD_OK;
	UINT32 vir_addr_main;
	HD_AUDIOCAP_BUFINFO phy_buf_main;
	char file_path_main[64];
	char file_path_aec[64];
	char file_path_ref[64];
	AUDIO_CAP_OUT *p_cap_only = (AUDIO_CAP_OUT *)arg;

	short aec_frame_out[960] = {0};
	bool has_far_voice = false;
	int make_sure_voice_count = MAKE_SURE_VOICE_COUNT;
	int ref_average_db = 0;

#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

	/* config pattern name */
	snprintf(file_path_main, sizeof(file_path_main), "%s%s", g_out_dir_path, "/cap.wav");
	snprintf(file_path_aec, sizeof(file_path_aec), "%s%s", g_out_dir_path, "/aec.wav");
	snprintf(file_path_ref, sizeof(file_path_ref), "%s%s", g_out_dir_path, "/ref.wav");

	/* wait flow_start */
	while (p_cap_only->flow_start == 0)
	{
		sleep(1);
	}

	/* query physical address of bs buffer
	  (this can ONLY query after hd_audiocap_start() is called !!) */
	hd_audiocap_get(p_cap_only->cap_ctrl, HD_AUDIOCAP_PARAM_BUFINFO, &phy_buf_main);

	/* mmap for bs buffer
	  (just mmap one time only, calculate offset to virtual address later) */
	vir_addr_main = (UINT64)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);

	if (vir_addr_main == 0)
	{
		LOGE("mmap error\r\n");
		return 0;
	}

	void *h_ref = wav_write_open(file_path_ref, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO);
	void *h_rec = wav_write_open(file_path_main, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO);
	void *h_rec_aec = NULL;
	if (cap_out.aec_mode != AEC_MODE_NONE)
	{
		h_rec_aec = wav_write_open(file_path_aec, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO);
	}

	LOG("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	/* pull data test */
	while (1)
	{
	retry1:
		if (p_cap_only->cap_exit == 1)
		{
			break;
		}

		if (p_cap_only->cap_pause == 1)
		{
			usleep(10000);
			goto retry1;
		}

		// pull data
		VENDOR_AUDIOCAP_AUDIO_FRAME data_pull;
		// pull data
		data_pull.wait_ms = 20;
		ret = vendor_audiocap_get(p_cap_only->cap_path, VENDOR_AUDIOCAP_ITEM_AUDIO_FRAME, &data_pull);
		if (ret == HD_OK)
		{
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.audio_frame.phy_addr[0]);
			UINT32 size = data_pull.audio_frame.size;

			UINT8 *ptr_ref = (UINT8 *)PHY2VIRT_MAIN(data_pull.audio_aec_ref_frame.phy_addr[0]);
			UINT32 size_ref = data_pull.audio_aec_ref_frame.size;

			if (h_rec)
			{
				wav_write_data(h_rec, (unsigned char *)ptr, size);
			}

			if (cap_out.aec_mode == AEC_MODE_APM)
			{
				nvt_apm_push_reference(g_apm, (short *)ptr_ref, size_ref);
				nvt_apm_process(g_apm, (short *)ptr, size, aec_frame_out);

				if (h_rec_aec)
				{
					wav_write_data(h_rec_aec, (unsigned char *)aec_frame_out, size);
				}
			}
			else if (cap_out.aec_mode == AEC_MODE_HALF_DUPLEX)
			{
				short *enqueue = microphone_in_frames[microphone_in_frames_enqueue_index];
				short *dequeue = microphone_in_frames[microphone_in_frames_dequeue_index];

				nvt_apm_push_reference(g_apm, (short *)ptr_ref, size_ref);
				nvt_apm_get(g_apm, APM_PARAM_GET_REF_VOICE_DETECTED, &has_far_voice);
				nvt_apm_get(g_apm, APM_PARAM_GET_REF_DB, &ref_average_db);
#if 1
				memcpy(enqueue, ptr, size);
#else
				nvt_apm_process(g_apm, (short *)ptr, size, enqueue); // Test enabling AEC with HALF_DUPLEX (speak in turn), but it uses some CPU.
#endif
				if (has_far_voice)
				{
					if (ref_average_db < ref_db_threshold)
					{
						has_far_voice = false;
					}
				}

				speak_in_turn(has_far_voice, &make_sure_voice_count, &mic_curve_index, dequeue, size, true);

				if (h_rec_aec)
				{
					wav_write_data(h_rec_aec, (unsigned char *)dequeue, size);
				}

				microphone_in_frames_enqueue_index = (microphone_in_frames_enqueue_index + 1) % AUDIO_BUFFER_COUNT;
				microphone_in_frames_dequeue_index = (microphone_in_frames_dequeue_index + 1) % AUDIO_BUFFER_COUNT;
			}

			if (h_ref)
			{
				wav_write_data(h_ref, (unsigned char *)ptr_ref, size_ref);
			}

			// release data
			ret = hd_audiocap_release_out_buf(p_cap_only->cap_path, &(data_pull.audio_frame));
			if (ret != HD_OK)
			{
				LOG("release buffer failed. ret=%x\r\n", ret);
			}
		}
	}

	/* mummap for bs buffer */
//	hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);

	if (h_ref)
		wav_write_close(h_ref);
	if (h_rec)
		wav_write_close(h_rec);
	if (h_rec_aec)
		wav_write_close(h_rec_aec);

	if (cap_out.aec_mode == AEC_MODE_APM || cap_out.aec_mode == AEC_MODE_HALF_DUPLEX)
	{
		nvt_apm_uninit(g_apm);
	}

	return 0;
}

static void *playback_thread(void *arg)
{
	INT ret;
	HD_AUDIO_FRAME bs_in_buf = {0};
	HD_COMMON_MEM_VB_BLK blk;
	void *pa, *va;
	UINT32 blk_size = 0x100000;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	void *bs_buf_start, *bs_buf_curr, *bs_buf_end;
	AUDIO_CAP_OUT *p_out_only = (AUDIO_CAP_OUT *)arg;
	bool has_far_voice = false;
	int make_sure_voice_count = MAKE_SURE_VOICE_COUNT;
	int play_format, play_channels, play_sample_rate, play_bits_per_sample;
	unsigned int play_data_length;
	int play_average_db = 0;
	void *playback_apm = NULL;

	FUNC_IN();

	void *h_play = wav_read_open(g_playback_file_name);
	if (NULL == h_play)
	{
		LOGE("No such file: %s", g_playback_file_name);
		return NULL;
	}

	int res = wav_get_header(h_play, &play_format, &play_channels, &play_sample_rate, &play_bits_per_sample, &play_data_length);
	if (!res)
	{
		LOG("get ref header error: %d\n", res);
		return NULL;
	}
	LOG("Playback file: %s\r\n", g_playback_file_name);
	print_wav_information(h_play);

	const int BYTES_PER_FRAME = play_sample_rate * play_channels * play_bits_per_sample / 8 / 100;

	LOGI("BYTES_PER_FRAME=%d", BYTES_PER_FRAME);

	/* get memory */
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, blk_size, ddr_id); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK)
	{
		LOG("get block fail, blk = 0x%x\n", blk);
		goto play_fclose;
	}
	pa = (void *)hd_common_mem_blk2pa(blk); // get physical addr
	if (pa == 0)
	{
		LOG("blk2pa fail, blk(0x%x)\n", blk);
		goto play_fclose;
	}
	if (pa > 0)
	{
		va = (void *)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, (UINT64)pa, blk_size); // Get virtual addr
		if (va == 0)
		{
			LOG("get va fail, va(0x%x)\n", blk);
			goto play_fclose;
		}
		/* allocate bs buf */
		bs_buf_start = va;
		bs_buf_curr = bs_buf_start;
		bs_buf_end = bs_buf_start + blk_size;
		LOG("alloc bs_buf: start(0x%x) curr(0x%x) end(0x%x) size(0x%x)\n", bs_buf_start, bs_buf_curr, bs_buf_end, blk_size);
	}

	nvt_apm_init(&playback_apm);
	g_playback_config.ref_channel_count = g_config.ref_channel_count;
	g_playback_config.ref_sampling_rate = g_config.ref_sampling_rate;
	g_playback_config.rec_channel_count = g_config.rec_channel_count;
	g_playback_config.rec_sampling_rate = g_config.rec_sampling_rate;
	nvt_apm_set(playback_apm, APM_PARAM_CONFIG, &g_playback_config);
	nvt_apm_start(playback_apm);

	while (p_out_only->flow_start == 0)
	{
		sleep(1);
	}

	while (1)
	{
	retry2:
		if (p_out_only->out_exit == 1)
		{
			break;
		}

		if (p_out_only->out_pause == 1)
		{
			usleep(10000);
			goto retry2;
		}

		/* check bs buf rollback */
		if ((bs_buf_curr + BYTES_PER_FRAME) > bs_buf_end)
		{
			bs_buf_curr = bs_buf_start;
		}

		/* read bs from file */
		short playback_current_frame[960] = {0};
		if (BYTES_PER_FRAME != wav_read_data(h_play, (void *)playback_current_frame, BYTES_PER_FRAME))
		{
			LOG("already playback to the end of the file, restart it");
			wav_read_close(h_play);
			h_play = wav_read_open(g_playback_file_name);
			wav_read_data(h_play, (void *)playback_current_frame, BYTES_PER_FRAME);
		}

		short *enqueue = playback_frames[playback_frames_enqueue_index];
		if (g_config.vad_enabled)
		{
			nvt_apm_process(playback_apm, (short *)playback_current_frame, BYTES_PER_FRAME, enqueue); // noise reduction

			// remove non-voice
			nvt_apm_push_reference(playback_apm, (short *)enqueue, BYTES_PER_FRAME);
			nvt_apm_get(playback_apm, APM_PARAM_GET_REF_VOICE_DETECTED, &has_far_voice);
			// LOG("has_far_voice=%d", has_far_voice);
			nvt_apm_get(playback_apm, APM_PARAM_GET_REF_DB, &play_average_db);
			// LOG("play_average_db=%d", play_average_db);

			if (has_far_voice)
			{
				if (play_average_db < ref_db_threshold)
				{
					LOG("play_average_db < ref_db_threshold");
					has_far_voice = false;
				}
			}
			short *dequeue = playback_frames[playback_frames_dequeue_index];

			speak_in_turn(has_far_voice, &make_sure_voice_count, &play_curve_index, dequeue, BYTES_PER_FRAME, false);

			memcpy(bs_buf_curr, dequeue, BYTES_PER_FRAME);
		}
		else
		{
			memcpy(bs_buf_curr, playback_current_frame, BYTES_PER_FRAME);
		}

		bs_in_buf.sign = MAKEFOURCC('A', 'F', 'R', 'M');
		bs_in_buf.phy_addr[0] = (UINT64)(pa + (bs_buf_curr - bs_buf_start)); // needs to add offset
		bs_in_buf.size = BYTES_PER_FRAME;
		bs_in_buf.ddr_id = ddr_id;
		bs_in_buf.timestamp = hd_gettime_us();
		bs_in_buf.bit_width = HD_AUDIO_BIT_WIDTH_16;
		bs_in_buf.sound_mode = HD_AUDIO_SOUND_MODE_MONO;
		bs_in_buf.sample_rate = p_out_only->sample_rate;

	resend:
		ret = hd_audioout_push_in_buf(p_out_only->out_path, &bs_in_buf, -1);
		if (ret != HD_OK)
		{
			LOG("hd_audioout_push_in_buf fail, ret(%d)\n", ret);
			usleep(10000);
			goto resend;
		}

		bs_buf_curr += ALIGN_CEIL_4(BYTES_PER_FRAME); // shift to next

		playback_frames_enqueue_index = (playback_frames_enqueue_index + 1) % AUDIO_BUFFER_COUNT;
		playback_frames_dequeue_index = (playback_frames_dequeue_index + 1) % AUDIO_BUFFER_COUNT;
	}

	/* release memory */
	hd_common_mem_munmap((void *)va, blk_size);
	ret = hd_common_mem_release_block(blk);
	if (HD_OK != ret)
	{
		LOG("release blk fail, ret(%d)\n", ret);
		goto play_fclose;
	}

play_fclose:
	if (h_play != NULL)
	{
		wav_read_close(h_play);
	}

	nvt_apm_uninit(playback_apm);

	FUNC_OUT();

	return 0;
}

void stop_threads(void)
{
	cap_out.cap_exit = 1;
	cap_out.out_exit = 1;
	if (!playback_only)
	{
		if (cap_thread_id)
		{
			pthread_join(cap_thread_id, NULL);
			cap_thread_id = 0;
		}
	}
	if (out_thread_id)
	{
		pthread_join(out_thread_id, NULL);
		out_thread_id = 0;
	}
}

EXAMFUNC_ENTRY(hd_audio_bidirect, argc, argv)
{
	HD_RESULT ret;
	INT key;
	int cmd_opt = 0;
	bool check_average_db = false;
	unsigned int skip_timespan = 800; // default 0.8 seconds

	snprintf(g_playback_file_name, sizeof(g_playback_file_name), "%s%s", g_out_dir_path, "/1.wav");

	cap_out.aec_mode = AEC_MODE_APM;
	cap_out.cap_volume = 60;
	cap_out.play_volume = 50;

	// audio processing for microphone in
	g_config.aec_enabled = false;
	g_config.ns_enabled = false;
	g_config.agc_enabled = false;
	g_config.post_gain = 0;
	g_config.hpf_enabled = false;
	g_config.vad_enabled = false;
	g_config.residual_echo_detector_enabled = false;
	g_config.has_keyboard = false;
	g_config.delay_ms = 20;

	// audio processing for playback
	g_playback_config.ns_enabled = true; // enable noise suppression
	g_playback_config.ns_level = 1;

	if (argc == 1)
	{
		LOG("option:\n"
			"	e, --aec_mode) aec_mode. 0: disable, 1: apm, 2: speak_in_turn \n"
			"	d, --delay_ms) delay_ms\n"
			"	f, --input_file) playback file path. default: /mnt/sd/1.wav \n"
			"	p, --playback_only) playback only\n"
			"	n, --ns_level) enable noise suspression, and set level 0~3\n"
			"	a, --enable_aec) enable aec\n"
			"	h, --enable_hpf) enable high pass filter\n"
			"	v, --vad_enabled) enable vad\n"
			"	r, --residual_echo_detector_enabled) enable residual_echo_detector\n"
			"	g, --enable_agc) enable agc\n"
			"	G, --agc_gain_level) set init level\n"
			"	M, --cap_volume) microphone volume.  0~160 \n"
			"	P, --playback_volume) line out volume.  0~160 \n"
			"	t, --playback_duration) playback for this timespan then stop. (unit: millisecond) 0: no limit \n"
			"	c, --check_average_db) check average db from file\n"
			"	s, --skip_timespan) skip timespan at begining. (unit: millisecond) This only works with check_average_db option.\n"
			"	b, --ref_db_threshold) if remote db is lower than this value, it should not be a voice.\n"
			"	o, --out_dir_name) output directory path.\n");
		return 0;
	}

	static struct option long_options[] = {
		{"aec_mode", required_argument, NULL, 'e'},
		{"delay_ms", required_argument, NULL, 'd'},
		{"input_file", required_argument, NULL, 'f'},
		{"playback_only", no_argument, NULL, 'p'},
		{"ns_level", required_argument, NULL, 'n'},
		{"enable_aec", required_argument, NULL, 'a'},
		{"enable_hpf", required_argument, NULL, 'h'},
		{"vad_enabled", required_argument, NULL, 'v'},
		{"residual_echo_detector_enabled", required_argument, NULL, 'r'},
		{"vad_enabled", required_argument, NULL, 'v'},
		{"agc_gain_level", required_argument, NULL, 'g'},
		{"cap_volume", required_argument, NULL, 'M'},
		{"playback_volume", required_argument, NULL, 'P'},
		{"ref_db_threshold", required_argument, NULL, 'b'},
		{"playback_duration", required_argument, NULL, 't'},
		{"check_average_db", no_argument, NULL, 'c'},
		{"skip_timespan", no_argument, NULL, 's'},
		{"out_dir_name", required_argument, NULL, 'o'},
		{0, 0, 0, 0}};

	while (1)
	{
		// LOG("proces index:%d\n", optind);
		int option_index = 0;
		cmd_opt = getopt_long(argc, argv, "h:pe:d:f:n:m:g:v:M:P:G:t:cs:b:o:", long_options, &option_index); //
		if (cmd_opt == -1)
		{
			break;
		}

		switch (cmd_opt)
		{
		case 'e':
			cap_out.aec_mode = atoi(optarg);
			if (cap_out.aec_mode > AEC_MODE_COUNT)
			{
				LOGE("Incorrect aec_mode. 0: disable, 1: apm, 2: speak in turn (input: %d)", cap_out.aec_mode);
				return -1;
			}
			g_config.aec_enabled = (cap_out.aec_mode == AEC_MODE_APM);
			LOG("enable_aec = %d", g_config.aec_enabled);
			LOG("aec_mode = %d", cap_out.aec_mode);
			break;
		case 'c':
			LOG("check average db only.");
			check_average_db = true;
			break;
		case 'v':
			g_config.vad_enabled = atoi(optarg);
			g_playback_config.vad_enabled = g_config.vad_enabled;
			break;
		case 'M': // cap volume
			cap_out.cap_volume = atoi(optarg);
			LOG("cap_volume = %d", cap_out.cap_volume);
			break;
		case 'n':
		{
			int ns_level = atoi(optarg); // 0~3
			g_config.ns_enabled = ns_level >= 0;
			if (g_config.ns_enabled)
				g_config.ns_enabled = ns_level >= 0;
			if(g_config.ns_enabled)
			{
				g_config.ns_level = ns_level;
			}
			LOG("enable_ns = %d, level=%d", g_config.ns_enabled, g_config.ns_level);
			break;
		}
		case 'g':
			g_config.agc_enabled = true;
			LOG("enable agc = %d", g_config.agc_enabled);
			break;
		case 'G':
			g_config.post_gain = (unsigned char)atoi(optarg);
			LOG("post_gain = %d", g_config.post_gain);
			break;
		case 'd':
			g_config.delay_ms = atoi(optarg);
			LOG("delay_ms = %d", g_config.delay_ms);
			break;
		case 'f':
			LOG("Playback file name:%s", optarg);
			strcpy(g_playback_file_name, optarg);
			break;
		case 'r':
			g_config.residual_echo_detector_enabled = atoi(optarg);
			LOG("residual_echo_detector_enabled = %d", g_config.residual_echo_detector_enabled);
			break;
		case 'p':
			playback_only = 1;
			LOG("playback_only = %d", playback_only);
			break;
		case 'P': // playback volume
			cap_out.play_volume = atoi(optarg);
			LOG("play_volume = %d", cap_out.play_volume);
			break;
		case 'h':
			g_config.hpf_enabled = atoi(optarg);
			LOG("hpf_enabled = %d", g_config.hpf_enabled);
			break;
		case 'b':
			ref_db_threshold = atoi(optarg);
			LOG("ref_db_threshold = %d", ref_db_threshold);
			break;
		case 't':
			g_playback_duration = atoi(optarg);
			LOG("playback_duration = %d", g_playback_duration);
			break;
		case 's':
			skip_timespan = atoi(optarg);
			LOG("skip_timespan = %d", skip_timespan);
			break;
		case 'o':
			LOG("Output directory name:%s", optarg);
			strcpy(g_out_dir_path, optarg);
			break;
		default:
			LOG("Not supported option");
			break;
		}
	}

	if (argc > optind)
	{
		int i = 0;
		for (i = optind; i < argc; i++)
		{
			/* Do we have args? */
			LOG("argv[%d] = %s", i, argv[i]);
		}
	}

	if (!playback_only)
	{
		LOGI("delay_ms: [%d]", g_config.delay_ms);
		LOGI("aec_enabled: [%d], mode=%d", g_config.aec_enabled, cap_out.aec_mode);
		LOGI("ns_enabled: [%d], level=%d", g_config.ns_enabled, g_config.ns_level);
		LOGI("agc_enabled: [%d], init gain=%d", g_config.agc_enabled, g_config.post_gain);
		LOGI("hpf_enabled: [%d]", g_config.hpf_enabled);
		LOGI("vad_enabled: [%d]", g_config.vad_enabled);
		LOGI("residual_echo_detector_enabled: [%d]", g_config.residual_echo_detector_enabled);
		LOGI("cap_volume: [%d]", cap_out.cap_volume);
		LOGI("ref_db_threshold: [%d]", ref_db_threshold);
		LOGI("play_volume: [%d]", cap_out.play_volume);
		if (g_playback_duration)
			LOGI("playback_duration: [%d]", g_playback_duration);
	}

	LOG("Playback file: %s\r\n", g_playback_file_name);
	struct stat buffer;
	int exist = stat(g_playback_file_name, &buffer);
	if (exist != 0)
	{
		// file doesn't exist
		LOGE("Cannot find file: %s", g_playback_file_name);
		return 0;
	}

	/* read test pattern */
	void *h_play = wav_read_open(g_playback_file_name);

	int play_format, play_channels, play_sample_rate, play_bits_per_sample;
	unsigned int play_data_length;
	int res = wav_get_header(h_play, &play_format, &play_channels, &play_sample_rate, &play_bits_per_sample, &play_data_length);
	if (!res)
	{
		LOGE("get ref header error: %d\n", res);
		return -1;
	}
	cap_out.sample_rate = play_sample_rate;

	print_wav_information(h_play);

	if (check_average_db)
	{
		nvt_apm_init(&g_apm);

		g_config.ref_channel_count = HD_AUDIO_SOUND_MODE_MONO;
		g_config.ref_sampling_rate = cap_out.sample_rate;
		g_config.rec_channel_count = HD_AUDIO_SOUND_MODE_MONO;
		g_config.rec_sampling_rate = cap_out.sample_rate;

		const int BYTES_PER_FRAME = play_sample_rate * play_channels * (play_bits_per_sample / 8) / 100;
		nvt_apm_set(g_apm, APM_PARAM_CONFIG, &g_config);
		nvt_apm_start(g_apm);
		int play_average_db = 0;

		long total_db = 0;
		double standard_deviation = 0;
		int peak_db = -127;
		int total_frame_count = 0;

		int skip_times = (skip_timespan + 5) / 10;
		for (int i = 0; i < skip_times; i++) // Skip data at begining, which has starting noise
		{
			if (BYTES_PER_FRAME != wav_read_data(h_play, (void *)playback_frames, BYTES_PER_FRAME))
			{
				LOGE("The file is too small to skip frames.\n");
				return -1;
			}
		}

		while (1)
		{
			if (BYTES_PER_FRAME != wav_read_data(h_play, (void *)playback_frames, BYTES_PER_FRAME))
			{
				break;
			}

			nvt_apm_push_reference(g_apm, (short *)playback_frames, BYTES_PER_FRAME);

			nvt_apm_get(g_apm, APM_PARAM_GET_REF_DB, &play_average_db);

			total_frame_count++;
			total_db += play_average_db;
			standard_deviation += play_average_db * play_average_db;
			if (play_average_db > peak_db)
				peak_db = play_average_db;
		}
		nvt_apm_uninit(g_apm);

		double average_db = (double)total_db / total_frame_count;

		double temp = standard_deviation / total_frame_count - average_db * average_db;
		if (temp <= 0)
			standard_deviation = 0;
		else
			standard_deviation = pow(temp, 0.5);

		printf("average_db=%ld\n", (long)average_db);
		printf("peak_db=%d\n", peak_db);
		printf("standard_deviation=%.03f\n", standard_deviation);
		wav_read_close(h_play);
		return 0;
	}

	wav_read_close(h_play);

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK)
	{
		LOGE("init fail=%d\n", ret);
		goto exit;
	}
	// init memory
	ret = mem_init();
	if (ret != HD_OK)
	{
		LOGE("mem fail=%d\n", ret);
		goto exit;
	}
	// cap_out module init
	ret = init_module();
	if (ret != HD_OK)
	{
		LOGE("init fail=%d\n", ret);
		goto exit;
	}
	// open cap_out module
	cap_out.sample_rate_max = cap_out.sample_rate; // assign by user
	ret = open_module(&cap_out);
	if (ret != HD_OK)
	{
		LOGE("open fail=%d\n", ret);
		goto exit;
	}
	// set audiocap parameter
	cap_out.sample_rate = cap_out.sample_rate; // assign by user
	ret = set_cap_param(cap_out.cap_path, cap_out.sample_rate);
	if (ret != HD_OK)
	{
		LOGE("set cap fail=%d\n", ret);
		goto exit;
	}

	// set audioout parameter
	ret = set_out_param(cap_out.out_ctrl, cap_out.out_path, cap_out.sample_rate);
	if (ret != HD_OK)
	{
		LOGE("set out fail=%d\n", ret);
		goto exit;
	}

	g_config.ref_channel_count = HD_AUDIO_SOUND_MODE_MONO;
	g_config.ref_sampling_rate = cap_out.sample_rate;
	g_config.rec_channel_count = HD_AUDIO_SOUND_MODE_MONO;
	g_config.rec_sampling_rate = cap_out.sample_rate;

	if (cap_out.aec_mode == AEC_MODE_APM || cap_out.aec_mode == AEC_MODE_HALF_DUPLEX)
	{
		nvt_apm_init(&g_apm);
		nvt_apm_set(g_apm, APM_PARAM_CONFIG, &g_config);
		nvt_apm_start(g_apm);
	}

	// create capture thread
	if (!playback_only)
	{
		LOGI("pthread_create capture_thread");
		ret = pthread_create(&cap_thread_id, NULL, capture_thread, (void *)&cap_out);
		if (ret < 0)
		{
			LOGE("create record thread failed");
			goto exit;
		}
		LOGI("cap_thread_id=%d", cap_thread_id);
	}

	// create output thread
	LOGI("pthread_create playback_thread");
	ret = pthread_create(&out_thread_id, NULL, playback_thread, (void *)&cap_out);
	if (ret < 0)
	{
		LOGE("create playback thread failed");
		goto exit;
	}
	LOGI("out_thread_id=%d", out_thread_id);

	// start cap_out module
	hd_audiocap_start(cap_out.cap_path);
	hd_audioout_start(cap_out.out_path);

	LOGI("flow_start");
	cap_out.flow_start = 1;

	LOGI("Enter q to exit, Enter d to debug");

	UINT32 start_time = hd_gettime_ms();

	int releasing = 0;
	while (!releasing)
	{
		usleep(100000);

		if (g_playback_duration)
		{
			if (hd_gettime_ms() - start_time >= g_playback_duration)
			{
				key = 'q';
			}
			else
			{
				continue;
			}
		}
		else
		{
			key = NVT_EXAMSYS_GETCHAR();
		}

		switch (key)
		{
		case 'q':
		case 0x3:
		{
			stop_threads();
			releasing = 1;
			break;
		}
		case 't':
		{
			system("top");
			break;
		}
#if (DEBUG_MENU == 1)
		case 'd':

		{
			hd_debug_run_menu(); // call debug menu
			LOG("\r\nEnter q to exit, Enter d to debug\r\n");
			break;
		}
#endif
		}
	}

	// stop cap_out module
	hd_audiocap_stop(cap_out.cap_path);
	hd_audioout_stop(cap_out.out_path);

exit:
	// close all module
	ret = close_module(&cap_out);
	if (ret != HD_OK)
	{
		LOG("close fail=%d\n", ret);
	}
	// uninit all module
	ret = exit_module();
	if (ret != HD_OK)
	{
		LOG("exit fail=%d\n", ret);
	}
	// uninit memory
	ret = mem_exit();
	if (ret != HD_OK)
	{
		LOG("mem fail=%d\n", ret);
	}
	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK)
	{
		LOG("common-uninit fail=%d\n", ret);
	}

	return 0;
}
