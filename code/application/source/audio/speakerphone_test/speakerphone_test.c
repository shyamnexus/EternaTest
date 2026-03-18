/**
	@brief Sample code of using audio processing lib.\n

	@file speakerphone_test.c

	@author Gary Ma

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
#include "ns_func.h"
#include "wavwriter.h"
#include "wavreader.h"
//#include "debug.h"
#include <getopt.h>
#include <sys/stat.h>
#include <math.h>
#include <limits.h>
#include "aec/audlib_aec.h"
#include "agc_func.h"
#include "eq_func.h"
#if defined(__LINUX)
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <kwrap/task.h>
#define sleep(x) vos_task_delay_ms(1000 * x)
#define usleep(x) vos_task_delay_us(x)
#endif
#define CHECK_RET(func)               \
	ret = func if (ret != 0)          \
	{                                 \
		printf("error: " #func "\n"); \
		return -1;                    \
	}

#define DEBUG_MENU 1

#define DEFAULT_CAPTURE_VOLUME 60
#define DEFAULT_PLAYBACK_VOLUME 50

typedef struct _AUDIO_CAP_OUT
{
	HD_AUDIO_SR sample_rate_max;
	HD_AUDIO_SR sample_rate;

	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;
	UINT32 cap_exit;
	UINT32 flow_start;

	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;
	UINT32 out_exit;

	UINT32 cap_volume;
	UINT32 play_volume;
	int aec_mode;
} AUDIO_CAP_OUT;

enum
{
	NS_AFTER_AEC = 0,
	NS_BEFORE_AEC = 1
} NS_LOCATION;

enum
{
	AUDIO_MODE_PLAYBACK_AND_CAPTURE = 0,
	AUDIO_MODE_PLAYBACK_ONLY,
	AUDIO_MODE_CAPTURE_ONLY
} AUDIO_MODE;

#define NVTAEC_QCONSTANT16(x) ((INT32)((500 + ((x) * (((INT32)1) << (15)))) / 1000))

static AUDIO_CAP_OUT cap_out = {0};

static pthread_t cap_thread_id = 0;
static pthread_t out_thread_id = 0;
static volatile int g_keep_running = 1;

void intHandler(int sig)
{
	g_keep_running = 0;
	switch (sig)
	{
	case SIGINT:
		//LOGW("\nctrl+c detected.\r\n");
		break;
	case SIGTERM:
		//printf("\nkill signal detected.\r\n");
		break;
	default:
		break;
	}
}

#define ALIGN_BIT(x, b) ((x - 1 + (1 << b)) & (~((1 << b) - 1)))

///////////////////////////////////////////////////////////////////////////////
// default settings
///////////////////////////////////////////////////////////////////////////////
static int g_sampling_rate = 16000;
static int g_mic_count = 1;
static int g_speaker_count = 1;
static int g_sample_count_per_frame = ALIGN_BIT(16000 * 1 * 20 / 1000, 10); // 16000 sampling rate, 1 channel, 20 ms align to 1024(2^10)

static int g_ns_level = 3;				 // noise suspression level: -1 turn off, 0~3: different level
static int g_ns_location = NS_AFTER_AEC; // 0: after aec,  1: before aec

#define DEFAULT_PLAYBACK_FILE "/mnt/sd/audio_sample_radio_16000.wav"
static CHAR g_playback_file_name[128] = DEFAULT_PLAYBACK_FILE;
static CHAR g_capture_file_name[128] = {0};

static CHAR g_out_dir_path[128] = "/mnt/sd";

// The duration of real sound that can pre-init aec lib in order to avoid the echo during algorithm training period. Recorded audios can be also used as the training source. unit: seconds
static int g_preinit_duration = 1;

static int g_agc_enabled = 1;
static int g_audio_mode = AUDIO_MODE_PLAYBACK_AND_CAPTURE;
static int g_analog_gain_enabled = 1;
static int g_alc_enabled = 0;
static int g_post_gain = 0;
static char g_eq_pattern[1024] = {0};
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
		//printf("hd_common_mem_init err: %d\r\n", ret);
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
		//printf("hd_audiocap_set fail, audio_cap_volume.volume=%d", audio_cap_volume.volume);
		return ret;
	}
	//LOGI("hd_audiocap_set, audio_cap_volume.volume=%d", audio_cap_volume.volume);

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
	UINT32 alc_en = g_alc_enabled;
	ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_ALC_ENABLE, (VOID *)&alc_en);
	if (ret != HD_OK)
	{
		//printf("vendor_audiocap_set fail");
		return ret;
	}
	//LOGI("vendor_audiocap_set, alc_en=%d", alc_en);


	UINT32 gain_lvl = VENDOR_AUDIOCAP_GAIN_LEVEL32; // increase gain level count from 8 to 32
	vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL, &gain_lvl);


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
	//printf("audio cap setting: %s", (audio_driver_cfg_param.mono == HD_AUDIO_MONO_RIGHT ? "HD_AUDIO_MONO_RIGHT" : "HD_AUDIO_MONO_LEFT"));

	vendor_loopback.enabled = TRUE;
	vendor_loopback.lb_channel = HD_AUDIOCAP_LB_CH_LEFT; // depend on hardware
	//printf("loopback setting: %s.   For 52x/562/566 EVB: left. For 563/565 EVB: right. For 560 evb: left or right", (vendor_loopback.lb_channel == HD_AUDIOCAP_LB_CH_RIGHT ? "HD_AUDIOCAP_LB_CH_RIGHT" : "HD_AUDIOCAP_LB_CH_LEFT"));
	vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_LOOPBACK_CONFIG, &vendor_loopback);

	*p_audio_cap_ctrl = audio_cap_ctrl;

	return ret;
}


static HD_RESULT set_cap_param(HD_PATH_ID audio_cap_path, int sampling_rate, int sample_count_per_frame, int mic_count)
{
	HD_RESULT ret = HD_OK;
	// set hd_audiocapture input parameters
	HD_AUDIOCAP_IN audio_cap_param = {0};
	HD_AUDIOCAP_OUT audio_cap_out_param = {0};

	audio_cap_param.sample_rate = sampling_rate;
	audio_cap_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cap_param.mode = (mic_count == 1 ? HD_AUDIO_SOUND_MODE_MONO : HD_AUDIO_SOUND_MODE_STEREO);
	audio_cap_param.frame_sample = sample_count_per_frame;
	//printf("sampling_rate=%d, sample_bit=%d, mode=%d, frame_sample=%d", audio_cap_param.sample_rate, audio_cap_param.sample_bit, audio_cap_param.mode, audio_cap_param.frame_sample);
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

	if (!g_analog_gain_enabled)
	{
		// turn off analog gain
		UINT32 setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_0DB; // there is another enum，alcoff (alc off + 0 db).
		vendor_audiocap_set(audio_cap_path, VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING, (VOID *)&setting);
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
	audio_cfg_param.out_max.mode = (g_mic_count == 1 ? HD_AUDIO_SOUND_MODE_MONO : HD_AUDIO_SOUND_MODE_STEREO);
	audio_cfg_param.frame_sample_max = 1024;
	audio_cfg_param.frame_num_max = 10;
	audio_cfg_param.in_max.sample_rate = 0;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_DEV_CONFIG, &audio_cfg_param);
	if (ret != HD_OK)
	{
		return ret;
	}

	/*set audio out driver parameters*/
	audio_driver_cfg_param.mono = HD_AUDIO_MONO_LEFT; // depend on hardware
	//printf("mono setting: %s.   For 52x/562/566 EVB: left. For 563/565 EVB: right. For 560 evb: left or right", (audio_driver_cfg_param.mono == HD_AUDIO_MONO_RIGHT ? "HD_AUDIO_MONO_RIGHT" : "HD_AUDIO_MONO_LEFT"));
	audio_driver_cfg_param.output = HD_AUDIOOUT_OUTPUT_LINE;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_DRV_CONFIG, &audio_driver_cfg_param);
	if (ret != HD_OK)
	{
		//printf("set HD_AUDIOOUT_PARAM_DRV_CONFIG fail");
		return ret;
	}

	// keep the power for speaker to avoid "pop" sound
	INT32 pwr_en = TRUE;
	ret = vendor_audioout_set(audio_out_ctrl, VENDOR_AUDIOOUT_ITEM_PREPWR_ENABLE, (VOID *)&pwr_en);
	if (ret != HD_OK)
	{
		//printf("set VENDOR_AUDIOOUT_ITEM_PREPWR_ENABLE fail");
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
		//printf("set hd_audioout_set fail");
		return ret;
	}

	//LOGI("Set audio_out_vol.volume=%d", audio_out_vol.volume);

	// set hd_audioout input parameters
	audio_out_in_param.sample_rate = 0;
	ret = hd_audioout_set(audio_out_path, HD_AUDIOOUT_PARAM_IN, &audio_out_in_param);
	if (ret != HD_OK)
	{
		//printf("set HD_AUDIOOUT_PARAM_IN fail");
		return ret;
	}
    
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
		//printf("set cap-cfg fail\n");
		return HD_ERR_NG;
	}
	ret = set_cap_volume(cap_out.cap_volume);
	if (ret != HD_OK)
	{
		//printf("set set_cap_volume fail\n");
		return HD_ERR_NG;
	}
	ret = set_out_cfg(&p_cap_out->out_ctrl, p_cap_out->sample_rate_max);
	if (ret != HD_OK)
	{
		//printf("set out-cfg fail\n");
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

typedef struct _AUDIO_AEC
{
	char *aec_internal;
	char *aec_fore;
	char *aec_back;
	UINT32 sample_rate;
	UINT32 channels;
	int spk_ch;
	int mic_ch;
	int filter_size;
	int frame_size;
	int echo_suppress;
	int noise_suppress;
} AUDIO_AEC;

#if 1
static int aec_init(AUDIO_AEC *aec_t)
{
	INT32 internal_size;
	INT32 fore_size;
	INT32 back_size;

	if (audlib_aec_open() != 0)
	{
		printf("AEC open failed\r\n");
		return 0;
	}

	audlib_aec_set_config(AEC_CONFIG_ID_LEAK_ESTIMTAE_EN, 0);
	audlib_aec_set_config(AEC_CONFIG_ID_LEAK_ESTIMTAE_VAL, NVTAEC_QCONSTANT16(990));

	audlib_aec_set_config(AEC_CONFIG_ID_NOTCH_RADIUS, NVTAEC_QCONSTANT16(992));

	audlib_aec_set_config(AEC_CONFIG_ID_NOISE_CANCEL_LVL, aec_t->noise_suppress); // Defualt is -20dB. Suggest value range -3 ~ -40. Unit in dB.
	audlib_aec_set_config(AEC_CONFIG_ID_ECHO_CANCEL_LVL, aec_t->echo_suppress);	  // Defualt is -50dB. Suggest value range -30 ~ -60. Unit in dB.

	audlib_aec_set_config(AEC_CONFIG_ID_SAMPLERATE, aec_t->sample_rate);
	audlib_aec_set_config(AEC_CONFIG_ID_RECORD_CH_NO, aec_t->mic_ch);
	audlib_aec_set_config(AEC_CONFIG_ID_PLAYBACK_CH_NO, aec_t->spk_ch);
	audlib_aec_set_config(AEC_CONFIG_ID_SPK_NUMBER, 1);

	audlib_aec_set_config(AEC_CONFIG_ID_FILTER_LEN, aec_t->filter_size);
	audlib_aec_set_config(AEC_CONFIG_ID_FRAME_SIZE, aec_t->frame_size);

	audlib_aec_set_config(AEC_CONFIG_ID_PRELOAD_EN, 0);


	internal_size = ALIGN_CEIL_64(audlib_aec_get_required_buffer_size(AEC_BUFINFO_ID_INTERNAL));
	fore_size = ALIGN_CEIL_64(audlib_aec_get_required_buffer_size(AEC_BUFINFO_ID_FORESIZE));
	back_size = ALIGN_CEIL_64(audlib_aec_get_required_buffer_size(AEC_BUFINFO_ID_BACKSIZE));

	aec_t->aec_internal = calloc(internal_size, sizeof(char));
	aec_t->aec_fore = calloc(fore_size, sizeof(char));
	aec_t->aec_back = calloc(back_size, sizeof(char));

	audlib_aec_set_config(AEC_CONFIG_ID_FOREADDR, (INT64)aec_t->aec_fore);
	audlib_aec_set_config(AEC_CONFIG_ID_FORESIZE, (INT32)fore_size);

	audlib_aec_set_config(AEC_CONFIG_ID_BACKADDR, (INT64)aec_t->aec_back);
	audlib_aec_set_config(AEC_CONFIG_ID_BACKSIZE, (INT32)back_size);

	audlib_aec_set_config(AEC_CONFIG_ID_BUF_ADDR, (INT64)aec_t->aec_internal);
	audlib_aec_set_config(AEC_CONFIG_ID_BUF_SIZE, (INT32)internal_size);
    
	if (!audlib_aec_init())
	{
		printf("AEC init failed\r\n");
	}

	return 0;
}
#endif

#if 1
static int aec_close(AUDIO_AEC *aec_t)
{
	audlib_aec_close();

	if (aec_t->aec_internal)
	{
		free(aec_t->aec_internal);
		aec_t->aec_internal = 0;
	}
	if (aec_t->aec_fore)
	{
		free(aec_t->aec_fore);
		aec_t->aec_fore = 0;
	}

	if (aec_t->aec_back)
	{
		free(aec_t->aec_back);
		aec_t->aec_back = 0;
	}

	return 0;
}
#endif

/**
 * @brief Audio processing for echo cancellation
 *
 * @param aec_t audio lib instance
 * @param addr microphone input, also for output
 * @param addr_aec algorithm needs audio of speaker out for reference
 * @param size byte count
 * @return BOOL
 */
#if 1
static BOOL aec_run(AUDIO_AEC *aec_t, short *addr, short *addr_aec, UINT32 size)
{
	//AEC_BITSTREAM AecIO = {0};
    UINT64 bitstream_buffer_play_in;
    UINT64 bitstream_buffer_record_in;
    //UINT64 bitstram_buffer_out ;
    UINT32 bitstram_buffer_length;
    
	/*if (((size >> 1) % 1024) != 0) {
		usleep(10000);
		printf("The sample count should be multiple of 1024. %d\r\n", size);
		return 0;
	}*/
    //printf("sizeof(AecIO.bitstream_buffer_play_in)=%d\r\n",sizeof(AecIO.bitstream_buffer_play_in));
	bitstream_buffer_play_in = (UINT64)addr_aec;
	bitstream_buffer_record_in = (UINT64)addr;
	//bitstram_buffer_out = (UINT64)addr;
	bitstram_buffer_length = (size >> 1);
    
  
	if (aec_t->mic_ch == 2)
	{
		bitstram_buffer_length = bitstram_buffer_length >> 1;
	}
    
	if (!audlib_aec_run(bitstream_buffer_play_in,bitstream_buffer_record_in,bitstram_buffer_length))
	{
		//printf("AEC failed\r\n");
		return 0;
	}
	return size;
}
#endif

void volume_change(short *samp, int sampleCount, float percent)
{
	for (int i = 0; i < sampleCount; i++)
	{
		samp[i] = (int16_t)(samp[i] * percent);
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

void apply_fixed_gain(short *frame, int sample_count, float muliplier)
{
	for (int i = 0; i < sample_count; i++)
	{
		double new_val = ((double)frame[i] * muliplier) + 0.5;
		if (new_val < SHRT_MIN)
			new_val = SHRT_MIN;
		else if (new_val > SHRT_MAX)
			new_val = SHRT_MAX;
		frame[i] = (short)new_val;
	}
}

#if 1
static void *capture_thread(void *arg)
{
	HD_RESULT ret = HD_OK;
	UINT64 vir_addr_main;
	HD_AUDIOCAP_BUFINFO phy_buf_main;
	char file_path_cap[64];
	char file_path_ns[64];
	char file_path_aec[64];
	char file_path_agc[64];
	char file_path_ref[64];
	char file_path_post_gain[64];
	char file_path_eq[64];
	AUDIO_CAP_OUT *p_cap_only = (AUDIO_CAP_OUT *)arg;

#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

	// init aec lib
	AUDIO_AEC aec_config = {0};
	aec_config.sample_rate = g_sampling_rate;
	aec_config.spk_ch = g_speaker_count;
	aec_config.mic_ch = g_mic_count;
	aec_config.filter_size = 1024; // ALIGN_BIT(g_sampling_rate * g_mic_count / 10, 10);  // use 100 ms, unit: samples, suggestion: 100~500 ms
	aec_config.frame_size = 128;   // 128
	aec_config.echo_suppress = -50;
	aec_config.noise_suppress = -30;
	aec_init(&aec_config);
	// init ns lib
	NS_INFO cap_ns_info = {0};
	if (g_ns_level >= 0)
	{
		cap_ns_info.enable = 1;
		cap_ns_info.sampling_rate = g_sampling_rate;
		cap_ns_info.channel_count = g_speaker_count;
		cap_ns_info.ns_level = g_ns_level;
		ns_init(&cap_ns_info);
	}

	// init agc lib
	AGC_INFO agc_info = {0};
	agc_info.enable = g_agc_enabled;
	agc_info.channel_count = g_speaker_count;
	agc_info.sampling_rate = g_sampling_rate;
	agc_info.target_db = -9; // default: -9 db
	agc_info.noise_db = -50; // this value will be changed by agc_update_noise_db after detecting environment noise dB
	agc_init(&agc_info);

	float post_gain_multiplier = 1;
	if (g_post_gain)
	{
		post_gain_multiplier = pow(10.0, g_post_gain / 20.0);
	}


	EQ_INFO eq_info = {0};
	eq_info.enable = g_agc_enabled;
	eq_info.channel_count = g_speaker_count;
	eq_info.sampling_rate = g_sampling_rate;
	strcpy(eq_info.patterns, g_eq_pattern);
	eq_init(&eq_info);

	/* config output file name */
	snprintf(file_path_cap, sizeof(file_path_cap), "%s/%s", g_out_dir_path, "cap.wav");
	snprintf(file_path_ref, sizeof(file_path_ref), "%s/%s", g_out_dir_path, "ref.wav");
	snprintf(file_path_aec, sizeof(file_path_aec), "%s/%s", g_out_dir_path, "aec.wav");
	snprintf(file_path_ns, sizeof(file_path_ns), "%s/%s", g_out_dir_path, "ns.wav");
	snprintf(file_path_eq, sizeof(file_path_eq), "%s/%s", g_out_dir_path, "eq.wav");
	snprintf(file_path_post_gain, sizeof(file_path_post_gain), "%s/%s", g_out_dir_path, "post_gain.wav");
	snprintf(file_path_agc, sizeof(file_path_agc), "%s/%s", g_out_dir_path, "agc.wav");
	/* wait flow_start */
	while (p_cap_only->flow_start == 0)
	{
		if (!g_keep_running)
			break;
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
		//printf("mmap error\r\n");
		return 0;
	}

	int BYTES_PER_FRAME = g_sample_count_per_frame * sizeof(short);

	void *h_ref = wav_write_open(file_path_ref, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, g_speaker_count);
	void *h_cap = wav_write_open(file_path_cap, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, g_mic_count);
	void *h_aec = wav_write_open(file_path_aec, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, g_mic_count);
	void *h_ns = wav_write_open(file_path_ns, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, g_mic_count);
	void *h_eq = wav_write_open(file_path_eq, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, g_mic_count);
	void *h_post_gain = wav_write_open(file_path_post_gain, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, g_mic_count);
	void *h_agc = wav_write_open(file_path_agc, p_cap_only->sample_rate, HD_AUDIO_BIT_WIDTH_16, g_mic_count);
	void *h_cap_from_file = NULL;
	UINT8 *ptr = NULL;

	if ( g_capture_file_name[0] )
	{
		printf("Read from file %s", g_capture_file_name);
		h_cap_from_file = wav_read_open(g_capture_file_name);
		if (NULL == h_cap_from_file)
		{
			//printf("No such file: %s", g_capture_file_name);
			return NULL;
		}

		ptr = malloc(BYTES_PER_FRAME);
	}


	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	UINT64 start_time = hd_gettime_us();
	int preinitialized = 0;
	long long total_db = 0;
	long total_db_count = 0;

	/* pull data test */
	while (g_keep_running)
	{
		if (p_cap_only->cap_exit == 1)
		{
			break;
		}

		UINT32 byte_size = 0;

		UINT8 *ptr_ref = NULL;
		UINT32 byte_size_ref = 0;

		void *frame = NULL;


		if ( g_capture_file_name[0] )
		{
			byte_size = BYTES_PER_FRAME;
			if (BYTES_PER_FRAME != wav_read_data(h_cap_from_file, (void *)ptr, BYTES_PER_FRAME))
			{
				//LOGI("Finished. Type q to exit.");
				g_keep_running = 0;
				break;
			}
		}
		else if (g_audio_mode == AUDIO_MODE_CAPTURE_ONLY)
		{
			HD_AUDIO_FRAME data_pull;
			ret = hd_audiocap_pull_out_buf(p_cap_only->cap_path, &data_pull, 200); // >1 = timeout mode
			if (ret == HD_OK)
			{
				ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.phy_addr[0]);
				byte_size = data_pull.size;

				frame = &data_pull;
			}
		}
		else
		{
			VENDOR_AUDIOCAP_AUDIO_FRAME data_pull;
			data_pull.wait_ms = 20;
			ret = vendor_audiocap_get(p_cap_only->cap_path, VENDOR_AUDIOCAP_ITEM_AUDIO_FRAME, &data_pull);
			if (ret == HD_OK)
			{
				ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.audio_frame.phy_addr[0]);
				byte_size = data_pull.audio_frame.size;

				ptr_ref = (UINT8 *)PHY2VIRT_MAIN(data_pull.audio_aec_ref_frame.phy_addr[0]);
				byte_size_ref = data_pull.audio_aec_ref_frame.size;

				frame = &data_pull.audio_frame;
			}
		}
        
		if (ret == HD_OK)
		{
			if (!preinitialized)
			{
				UINT64 current_time = hd_gettime_us();
				if (current_time - start_time >= (UINT64)(g_preinit_duration * 1000000))
				{
					preinitialized = 1;
				}
			}

			if (h_cap)
			{
                //printf("h_cap\r\n");
				wav_write_data(h_cap, (unsigned char *)ptr, byte_size);  
                             
			}

			if (g_audio_mode != AUDIO_MODE_CAPTURE_ONLY)
			{
				if (h_ref)
				{
                   
					wav_write_data(h_ref, (unsigned char *)ptr_ref, byte_size_ref);
				}
			}

            
			if (g_ns_level >= 0 && g_ns_location == NS_BEFORE_AEC)
			{
				int ref_average_db = ns_get_db(&cap_ns_info, (short *)ptr, byte_size >> 1);
				total_db_count++;
				total_db += ref_average_db;
				if (total_db_count % 20 == 1)
				{
					double average_db = (double)total_db/total_db_count;
					//printf("Environment average dB=%f", average_db);
					agc_update_noise_db(&agc_info, average_db-7);   // -7 is an experience value

					if( total_db_count >= 3000)  //1 min
					{
						// update environment noise db periodically
						total_db_count = 1001;
						total_db = average_db * total_db_count;
					}
				}
				ns_run(&cap_ns_info, (short *)ptr, byte_size >> 1);
				if (h_ns)
				{
                    
					wav_write_data(h_ns, (unsigned char *)ptr, byte_size);
				}
			}
        
            
           
            
			if (g_audio_mode != AUDIO_MODE_CAPTURE_ONLY)
			{
				aec_run(&aec_config, (short *)ptr, (short *)ptr_ref, byte_size);
				if (h_aec && preinitialized)
				{
					wav_write_data(h_aec, (unsigned char *)ptr, byte_size);
				}
			}

            #if 1
			if (g_eq_pattern[0])
			{
                #ifndef _LP64
                eq_run(&eq_info, (short *)ptr, byte_size >> 1);
                #else
			    eq_run(&eq_info, (long )ptr, byte_size >> 1);
                #endif
                
				if (h_eq && preinitialized)
				{
					wav_write_data(h_eq, (unsigned char *)ptr, byte_size);
				}
			}
            #endif
         

            
			if (g_post_gain)
			{
				apply_fixed_gain((short *)ptr, byte_size >> 1, post_gain_multiplier);
				if (h_post_gain && preinitialized)
				{
					wav_write_data(h_post_gain, (unsigned char *)ptr, byte_size);
				}
			}

			if (g_ns_level >= 0 && g_ns_location == NS_AFTER_AEC)
			{
			    ns_run(&cap_ns_info, (short *)ptr, byte_size >> 1);

				if (h_ns && preinitialized)
				{
					wav_write_data(h_ns, (unsigned char *)ptr, byte_size);
				}
			}
            
            #if 1
			if (g_agc_enabled)
			{
				agc_run(&agc_info, (short *)ptr, byte_size >> 1);

				if (h_agc && preinitialized)
				{
					wav_write_data(h_agc, (unsigned char *)ptr, byte_size);
				}
			}
            #endif
        

			// release data
			if (frame)
			{
               
				ret = hd_audiocap_release_out_buf(p_cap_only->cap_path, frame);
				if (ret != HD_OK)
				{
					printf("release buffer failed. ret=%x\r\n", ret);
				}
			}
		}
        
	}

	/* mummap for bs buffer */
	hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);

	if (h_agc)
		wav_write_close(h_agc);
	if (h_eq)
		wav_write_close(h_eq);
	if (h_post_gain)
		wav_write_close(h_post_gain);
	if (h_ns)
		wav_write_close(h_ns);
	if (h_ref)
		wav_write_close(h_ref);
	if (h_cap)
		wav_write_close(h_cap);
	if (h_aec)
		wav_write_close(h_aec);
	if (h_cap_from_file)
	{
		wav_read_close(h_cap_from_file);
		free(ptr);
	}

	eq_close(&eq_info);
	agc_close(&agc_info);
	ns_close(&cap_ns_info);
	aec_close(&aec_config);

	//FUNC_OUT();
	return 0;
}
#endif

static void *playback_thread(void *arg)
{
	AUDIO_CAP_OUT *p_out_only = (AUDIO_CAP_OUT *)arg;
	INT ret;
	HD_AUDIO_FRAME bs_in_buf = {0};
	HD_COMMON_MEM_VB_BLK blk;
	void *pa, *va;
	UINT32 blk_size = 0x100000;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	void *bs_buf_start, *bs_buf_curr, *bs_buf_end;

	void *h_play = wav_read_open(g_playback_file_name);
	if (NULL == h_play)
	{
		//printf("No such file: %s", g_playback_file_name);
		return NULL;
	}

	// init ns lib
	NS_INFO playback_ns_info = {0};
	if (g_ns_level >= 0)
	{
		playback_ns_info.enable = 1;
		playback_ns_info.sampling_rate = g_sampling_rate;
		playback_ns_info.channel_count = g_speaker_count;
		playback_ns_info.ns_level = g_ns_level;
		ns_init(&playback_ns_info);
	}

	/* get memory */
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, blk_size, ddr_id); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK)
	{
		printf("get block fail, blk = 0x%x\n", blk);
		goto play_fclose;
	}
	pa = (void *)hd_common_mem_blk2pa(blk); // get physical addr
	if (pa == 0)
	{
		printf("blk2pa fail, blk(0x%x)\n", blk);
		goto play_fclose;
	}
	if (pa > 0)
	{
		va = (void *)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, (UINT64)pa, blk_size); // Get virtual addr
		if (va == 0)
		{
			//printf("get va fail, va(0x%x)\n", blk);
			goto play_fclose;
		}
		/* allocate bs buf */
		bs_buf_start = va;
		bs_buf_curr = bs_buf_start;
		bs_buf_end = bs_buf_start + blk_size;
		//printf("alloc bs_buf: start(0x%x) curr(0x%x) end(0x%x) size(0x%x)\n", bs_buf_start, bs_buf_curr, bs_buf_end, blk_size);
	}

	int BYTES_PER_FRAME = g_sample_count_per_frame * sizeof(short);

    
	while (p_out_only->flow_start == 0)
	{
		if (!g_keep_running)
			break;
		sleep(1);
	}
      

	while (g_keep_running)
	{
        
		if (p_out_only->out_exit == 1)
		{
			break;
		}

		/* check bs buf rollback */
		if ((bs_buf_curr + BYTES_PER_FRAME) > bs_buf_end)
		{
			bs_buf_curr = bs_buf_start;
		}
        
		/* read bs from file */
		if (BYTES_PER_FRAME != wav_read_data(h_play, (void *)bs_buf_curr, BYTES_PER_FRAME))
		{
			//printf("already playback to the end of the file, restart it");
			wav_read_close(h_play);
			h_play = wav_read_open(g_playback_file_name);
			wav_read_data(h_play, (void *)bs_buf_curr, BYTES_PER_FRAME);
		}

		if (g_ns_level >= 0)
		{
			ns_run(&playback_ns_info, (short *)bs_buf_curr, g_sample_count_per_frame);
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
			//printf("hd_audioout_push_in_buf fail, ret(%d)\n", ret);
			usleep(10000);
			goto resend;
		}
        
		bs_buf_curr += ALIGN_CEIL_4(BYTES_PER_FRAME); // shift to next
	}

	/* release memory */
	hd_common_mem_munmap((void *)va, blk_size);
	ret = hd_common_mem_release_block(blk);
	if (HD_OK != ret)
	{
		printf("release blk fail, ret(%d)\n", ret);
		goto play_fclose;
	}

play_fclose:
	if (h_play != NULL)
	{
		wav_read_close(h_play);
	}

	ns_close(&playback_ns_info);

//	FUNC_OUT();

	return 0;
}

void stop_threads(void)
{
	cap_out.cap_exit = 1;
	cap_out.out_exit = 1;
	if (g_audio_mode != AUDIO_MODE_PLAYBACK_ONLY && cap_thread_id)
	{
		pthread_join(cap_thread_id, NULL);
		cap_thread_id = 0;
	}
	if (g_audio_mode != AUDIO_MODE_CAPTURE_ONLY && out_thread_id)
	{
		pthread_join(out_thread_id, NULL);
		out_thread_id = 0;
	}
}

EXAMFUNC_ENTRY(speakerphone_test, argc, argv)
{
	HD_RESULT ret;
	INT key;
	int cmd_opt = 0;

	// default volume
	cap_out.cap_volume = DEFAULT_CAPTURE_VOLUME;
	cap_out.play_volume = DEFAULT_PLAYBACK_VOLUME;


	if (argc == 1)
	{
    /*
		printf("option:\n"
			"	f, --input_file $file_name) playback file path. default: %s \n"
			"	i, --cap_from_file $file_name) use capture file instead of microphone, but cannot change option a/M/P/L. \n"
			"	n, --ns_level $level) noise suspression level. -1: turn off, 0~3: different level. Default: %d \n"
			"	l, --ns_location $loc) noise suspression location.  0: after aec.  1: before aec. Default: 0\n"
			"	g, --enable_agc $enable) enable/disable agc (Output agc.wav file.) Default: enabled\n"
			"	a, --enable_analog_gain $enable) enable/disable analog gain. Default: disabled\n"
			"	M, --cap_volume $vol) microphone volume. Range: 0~160. Default: %d \n"
			"	P, --playback_volume $vol) line out volume.  0~100 (up to 160, but it might break the speaker.) Default: %d\n"
			"	p, --playback_only) playback only\n"
			"	c, --capture_only) capture only\n"
			"	L, --enable_alc $enable) enable/disable auto level control (ALC). Default: disabled\n"
			"	G, --post_gain $gain) apply post fixed gain. Range: -50~50 dB. Default: 0\n"
			"	e, --eq_pattern $pattern) enable EQ, and apply patterns. \n"
			"		Pattern: $(eq_type)_$(freqency)[_$(dB)]\n"
			"		eq_type: \n"
			"			LP: LOWPASS filter. Gradually reduce the frequency above the specified frequency to 0. This type ignores dB setting.\n"
			"			HP: HIGHPASS filter. Gradually reduce the frequency below the specified frequency to 0. This type ignores dB setting.\n"
			"			LS: low shelf filter. Adjust the frequency below the specified frequency. Range: -60 ~ 12 dB\n"
			"			HS: high shelf filter. Adjust frequencies above the specified frequency. Range: -60 ~ 12 dB\n"
			"			PK: PEAKING filter. Enhance the frequency on target frequency. Range: 0 ~ 12 dB\n"
			"			NC: NOTCH filter. Reduce the frequency on target frequency. Range: -60 ~ 0 dB\n"
			"		For multiple patterns, use '+' to combine them. \n"
			"		e.g. HP_200+LP_1300+PK_1000_5\n"
			"	o, --out_dir_name $path) output directory path. Default: %s\n"
			"\n"
			"example:\n"
			"   a) Test microphone: \n"
			"      1) speakerphone_test --capture_only --enable_analog_gain 1 --playback_volume 0 --cap_volume 100 --ns_level 1 --ns_location 1 --eq_pattern=\"HP_300+LP_1200+PK_1000_3\"\n"
			"      2) speakerphone_test --playback_only -f /mnt/sd/ns.wav\n"
			"      3) speakerphone_test --capture_only --cap_from_file /mnt/sd/sweeps_100-1100_tones_16000.wav --ns_level 3 --eq_pattern=\"HP_300+LP_1200+PK_1000_3\"\n"
			"   b) Test speakerphone: \n"
			"      1) speakerphone_test --enable_analog_gain 0 --playback_volume 100 --cap_volume 60 --ns_level 3 --ns_location 0\n"
			"      2) speakerphone_test --playback_only -f /mnt/sd/ns.wav\n",
			g_playback_file_name,
			g_ns_level,
			cap_out.cap_volume,
			cap_out.play_volume,
			g_out_dir_path);
            */
		return 0;
	}


	static struct option long_options[] = {
		{"input_file", required_argument, NULL, 'f'},
		{"cap_from_file", required_argument, NULL, 'i'},
		{"ns_level", required_argument, NULL, 'n'},
		{"ns_location", required_argument, NULL, 'l'},
		{"enable_agc", required_argument, NULL, 'g'},
		{"enable_analog_gain", required_argument, NULL, 'a'},
		{"cap_volume", required_argument, NULL, 'M'},
		{"playback_volume", required_argument, NULL, 'P'},
		{"playback_only", no_argument, NULL, 'p'},
		{"capture_only", no_argument, NULL, 'c'},
		{"enable_alc", required_argument, NULL, 'L'},
		{"post_gain", required_argument, NULL, 'G'},
		{"eq_pattern", required_argument, NULL, 'e'},
		{"out_dir_name", required_argument, NULL, 'o'},
		{0, 0, 0, 0}};

	while (1)
	{
	    int option_index = 0;
		cmd_opt = getopt_long(argc, argv, "f:n:g:M:P:po:l:a:cL:G:e:i:", long_options, &option_index);
		if (cmd_opt == -1)
		{
			break;
		}

		switch (cmd_opt)
		{
		case 'M': // cap volume
			cap_out.cap_volume = atoi(optarg);
			printf("cap_volume = %d", cap_out.cap_volume);
			break;
		case 'n':
		{
			int ns_level = atoi(optarg);
			if (ns_level <= -1)
				g_ns_level = -1;
			else if (ns_level > 3)
				g_ns_level = 3;
			else
				g_ns_level = ns_level;

			break;
		}
		case 'l':
		{
			int ns_location = atoi(optarg);
			if (ns_location <= 0)
				g_ns_location = NS_AFTER_AEC;
			else
				g_ns_location = NS_BEFORE_AEC;

			printf("set ns_location=%s", (g_ns_location ? "NS_BEFORE_AEC" : "NS_AFTER_AEC"));
			break;
		}
		case 'g':
		{
			int agc_enabled = atoi(optarg);
			g_agc_enabled = agc_enabled;
			printf("enable agc = %d", g_agc_enabled);
			break;
		}
		case 'a':
		{
			int analog_gain_enabled = atoi(optarg);
			g_analog_gain_enabled = analog_gain_enabled;
			printf("g_analog_gain_enabled = %d", g_analog_gain_enabled);
			break;
		}
		case 'f':
			printf("set playback file name:%s", optarg);
			strcpy(g_playback_file_name, optarg);
			break;
		case 'i':
			printf("set capture file name:%s", optarg);
			strcpy(g_capture_file_name, optarg);
			break;
		case 'P': // playback volume
			cap_out.play_volume = atoi(optarg);
			printf("play_volume = %d", cap_out.play_volume);
			break;
		case 'p':
			g_audio_mode = AUDIO_MODE_PLAYBACK_ONLY;
			printf("Playback only mode.");
			break;
		case 'c':
			g_audio_mode = AUDIO_MODE_CAPTURE_ONLY;
			g_preinit_duration = 0;
			printf("Capture only mode.");
			break;
		case 'L':
			g_alc_enabled = atoi(optarg) != 0;
			printf("set alc_enabled=%d", g_alc_enabled);
			break;
		case 'G':
			g_post_gain = atoi(optarg);
			if (g_post_gain < -50)
				g_post_gain = -50;
			else if (g_post_gain > 50)
				g_post_gain = 50;
			printf("set post_gain=%d", g_post_gain);
			break;
		case 'e':
		{
			strcpy(g_eq_pattern, optarg);
			printf("set eq_pattern = %s", g_eq_pattern);
			break;
		}
		case 'o':
			printf("Output directory name:%s", optarg);
			strcpy(g_out_dir_path, optarg);
			break;
		default:
			printf("Not supported option");
			break;
		}
	}

	if (argc > optind)
	{
		int i = 0;
		for (i = optind; i < argc; i++)
		{
			//printf("Additional argv[%d] = %s", i, argv[i]);
		}
	}
   


	printf("Playback file: %s\r\n", g_playback_file_name);
	struct stat buffer;
	int exist = stat(g_playback_file_name, &buffer);
	if (exist != 0)
	{
		printf("Cannot find file: %s", g_playback_file_name);
		return 0;
	}

	/* read test pattern */
	int play_format, play_channels, play_sample_rate, play_bits_per_sample;
	void *h_play = wav_read_open(g_playback_file_name);
	unsigned int play_data_length;
	int res = wav_get_header(h_play, &play_format, &play_channels, &play_sample_rate, &play_bits_per_sample, &play_data_length);
	if (!res)
	{
		printf("get ref header error: %d\n", res);
		return -1;
	}
	print_wav_information(h_play);
	wav_read_close(h_play);

	// set audiocap parameter
	// use the sampling rate from the header of wav file
	cap_out.sample_rate = play_sample_rate;
	cap_out.sample_rate_max = cap_out.sample_rate;

	g_sampling_rate = play_sample_rate;
	g_sample_count_per_frame = ALIGN_BIT(g_sampling_rate * g_mic_count * 20 / 1000, 10); // 20 ms sample count, align to 1024(2^10)

	printf("g_sampling_rate=%d, g_sample_count_per_frame=%d", g_sampling_rate, g_sample_count_per_frame);



	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK)
	{
		printf("init fail=%d\n", ret);
//		goto exit;
	}
    
   
	// init memory
	ret = mem_init();
	if (ret != HD_OK)
	{
		printf("mem fail=%d\n", ret);
	//	goto exit;
	}
    

    printf("init_module\r\n");
	// cap_out module init
	ret = init_module();
	if (ret != HD_OK)
	{
		printf("init fail=%d\n", ret);
//		goto exit;
	}
    
    printf("open_module\r\n");
	// open cap_out module
	ret = open_module(&cap_out);
	if (ret != HD_OK)
	{
		printf("open fail=%d\n", ret);
//		goto exit;
	}
    
	ret = set_cap_param(cap_out.cap_path, g_sampling_rate, g_sample_count_per_frame, g_mic_count);
	if (ret != HD_OK)
	{
		printf("set cap fail=%d\n", ret);
		goto exit;
	}

    printf("set_out_param\r\n");
	// set audioout parameter
	ret = set_out_param(cap_out.out_ctrl, cap_out.out_path, g_sampling_rate);
	if (ret != HD_OK)
	{
		printf("set out fail=%d\n", ret);
		goto exit;
	}
   
    #if 1
	if (g_audio_mode != AUDIO_MODE_PLAYBACK_ONLY)
	{
		// create capture thread
	    printf("pthread_create capture_thread");
		ret = pthread_create(&cap_thread_id, NULL, capture_thread, (void *)&cap_out);
		if (ret < 0)
		{
			printf("create record thread failed");
			goto exit;
		}
	}
    #endif

	// create output thread
	if (g_audio_mode != AUDIO_MODE_CAPTURE_ONLY)
	{
		printf("pthread_create playback_thread");
		ret = pthread_create(&out_thread_id, NULL, playback_thread, (void *)&cap_out);
		if (ret < 0)
		{
			printf("create playback thread failed");
			goto exit;
		}
	}

	// start cap_out module
	hd_audiocap_start(cap_out.cap_path);
	hd_audioout_start(cap_out.out_path);

	cap_out.flow_start = 1;

	// Handle ctrl+c
	struct sigaction act;
	act.sa_handler = intHandler;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);

	printf("Enter q to exit, Enter d to debug");


	while (g_keep_running)
	{
		usleep(100000);

		key = NVT_EXAMSYS_GETCHAR();

		switch (key)
		{
		case 'q':
		case 0x3:
		{
			g_keep_running = 0;
			break;
		}
		case 't':
		{
			system("top -d 1");
			break;
		}
#if (DEBUG_MENU == 1)
		case 'd':
		{
			hd_debug_run_menu(); // call debug menu
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
			break;
		}
#endif
		}
	}

exit:
	stop_threads(); // stop pull audio data

	// stop cap_out module
	hd_audiocap_stop(cap_out.cap_path);
	hd_audioout_stop(cap_out.out_path);

	// close all module
	ret = close_module(&cap_out);
	if (ret != HD_OK)
	{
		printf("close fail=%d\n", ret);
	}
	// uninit all module
	ret = exit_module();
	if (ret != HD_OK)
	{
		printf("exit fail=%d\n", ret);
	}
	// uninit memory
	ret = mem_exit();
	if (ret != HD_OK)
	{
		printf("mem fail=%d\n", ret);
	}
	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK)
	{
		printf("common-uninit fail=%d\n", ret);
	}

	system("sync");
	return 0;
}
