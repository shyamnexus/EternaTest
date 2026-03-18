#include "apm.h"

#include <stdio.h>
#include "wavreader.h"
#include "wavwriter.h"
#include <stdlib.h>
#include <unistd.h>

#define CHECK_RET(func)               \
	ret = func if (ret != 0)          \
	{                                 \
		printf("error: " #func "\n"); \
		return -1;                    \
	}

typedef enum
{
	MODE_AEC = 0, // Acoustic Echo Cancellation
	MODE_AGC,	  // Auto gain control
	MODE_NS,	  // noise suppression
	MODE_HPF,	  // high pass filter
} APM_MODE;

void print_help(void)
{
	printf("Usage: apm_test $MODE output.wav input.wav [reference.wav or level]\n");
	printf("\tMODE options)\n\t%d. AEC (Acoustic Echo Cancellation)\n\t%d. AGC (Auto gain control)\n\t%d. NS (noise suppression)\n\t%d. HPF (high pass filter)\n", MODE_AEC, MODE_AGC, MODE_NS, MODE_HPF);
	printf("\tAEC: need reference.wav. e.g. apm_test 0 output.wav input.wav reference.wav\n");
	printf("\tAGC: e.g. apm_test 1 output.wav input.wav [$agc_max_adjust_db_per_second] [$agc_max_skip_db] [$agc_target_db]\n");
	printf("\tNS: need $level (0~3) and $post_gain (0~90). e.g. apm_test 2 output.wav input.wav 3 10\n");
}

int main(int argc, char *argv[])
{
	NVT_APM_CONFIG config = {0};
	int ns_level = 1;
	if (argc <= 1)
	{
		print_help();
		return -1;
	}
	int mode = atoi(argv[1]);
	printf("MODE: %d\n", mode);
	switch (mode)
	{
	case MODE_AGC:
		{
			config.agc_max_adjust_db_per_second = 3; // 3
			config.agc_max_skip_db = -50; // if db level is lower than this db setting, drop it.  default: -50
			config.agc_max_db = -10; // AGC will control the volume not higher then this db setting.  default: -5
			if (argc >= 5)
			{
				config.agc_max_adjust_db_per_second = atoi(argv[4]); // 3
				printf("agc_max_adjust_db_per_second = %d\n", config.agc_max_adjust_db_per_second);
			}
			if (argc >= 6)
			{
				config.agc_max_skip_db = atoi(argv[5]); // 3
				printf("agc_max_skip_db = %d\n", config.agc_max_skip_db);
			}
			if (argc >= 7)
			{
				config.agc_max_db = atoi(argv[6]); // 3
				printf("agc_target_db = %d\n", config.agc_max_db);
			}
		}
		break;
	case MODE_NS:
		if (argc < 4)
		{
			print_help();
			return -1;
		}

		if (argc >= 5)
		{
			ns_level = atoi(argv[4]); // 0~3
			if (ns_level < 0 || ns_level > 3)
			{
				printf("ns level only support 0~3\n");
				return -1;
			}
		}
		if (argc >= 6)
		{
			config.post_gain = atoi(argv[5]);
			printf("post gain = %d\n", config.post_gain);
		}
		
		printf("out: %s, rec: %s, level: %d, post_gain=%d\n", argv[2], argv[3], ns_level, config.post_gain);
		break;
	case MODE_AEC:
		if (argc < 5)
		{
			print_help();
			return -1;
		}
		printf("out: %s, rec: %s, ref: %s\n", argv[2], argv[3], argv[4]);
		break;
	default:
		if (argc < 4)
		{
			print_help();
			return -1;
		}
		printf("out: %s, rec: %s\n", argv[2], argv[3]);
		break;
	}

	void *h_ref = NULL;
	void *h_rec = wav_read_open(argv[3]);

	
	int rec_format, rec_channels, rec_sample_rate, rec_bits_per_sample;
	unsigned int ref_data_length, rec_data_length;

	int res;

	res = wav_get_header(h_rec, &rec_format, &rec_channels, &rec_sample_rate, &rec_bits_per_sample, &rec_data_length);
	if (!res)
	{
		printf("get rec header error: %d\n", res);
		return -1;
	}
	print_wav_information(h_rec);

	if (MODE_AEC == mode)
	{
		int ref_format, ref_channels, ref_sample_rate, ref_bits_per_sample;
		h_ref = wav_read_open(argv[4]);
		res = wav_get_header(h_ref, &ref_format, &ref_channels, &ref_sample_rate, &ref_bits_per_sample, &ref_data_length);
		if (!res)
		{
			printf("get ref header error: %d\n", res);
			return -1;
		}
		print_wav_information(h_ref);

		if (ref_format != rec_format ||
			ref_channels != rec_channels ||
			ref_sample_rate != rec_sample_rate ||
			ref_bits_per_sample != rec_bits_per_sample)
		{
			printf("ref file format != rec file format\n");
			return -1;
		}
		
	}

	void *h_out = wav_write_open(argv[2], rec_sample_rate, rec_bits_per_sample, rec_channels);

	int samples_per_frame = rec_channels * rec_sample_rate / 50;  // an audio frame sample count (short int)
	int byte_per_frame = samples_per_frame * rec_bits_per_sample / 8;

	int delay_ms = 20;

	printf("samples_per_frame = %d, byte_per_frame=%d, delay_ms=%d\n", samples_per_frame, byte_per_frame, delay_ms);

	short *render_frame = (short *)malloc(byte_per_frame); // >samples_per_frame
	short *render_frame_out = (short *)malloc(byte_per_frame);
	short *capture_frame = (short *)malloc(byte_per_frame);
	short *capture_frame_out = (short *)malloc(byte_per_frame);
	void *instance = NULL;

	int ret = 0;
	CHECK_RET(nvt_apm_init(&instance);)

	
	config.aec_enabled = false;
	config.agc_enabled = false;
	config.hpf_enabled = false;
	config.vad_enabled = false;
	config.ns_enabled = false;
	config.ns_level = 1; // 0~3
	config.residual_echo_detector_enabled = false;
	config.delay_ms = delay_ms;
	config.ref_channel_count = rec_channels;
	config.ref_sampling_rate = rec_sample_rate;
	config.rec_channel_count = rec_channels;
	config.rec_sampling_rate = rec_sample_rate;
	config.has_keyboard = false;
	switch (mode)
	{
	case MODE_AEC:
	{
		config.aec_enabled = true;
		break;
	}
	case MODE_AGC:
	{
		config.agc_enabled = true;
		break;
	}
	case MODE_NS:
	{
		config.ns_enabled = true;
		config.ns_level = ns_level; 
		break;
	}
	case MODE_HPF:
	{
		config.hpf_enabled = true;
		break;
	}
	default:
		break;
	}

	CHECK_RET(nvt_apm_set(instance, APM_PARAM_CONFIG, &config);)
	CHECK_RET(nvt_apm_start(instance);)

	while (1)
	{
		static int index = 0;
		printf("%d.", ++index);
		if (byte_per_frame == wav_read_data(h_rec, (unsigned char *)capture_frame, byte_per_frame))
		{
			switch (mode)
			{
			case MODE_AEC:
			{
				wav_read_data(h_ref, (unsigned char *)render_frame, byte_per_frame);
				CHECK_RET(nvt_apm_process_ex(instance, capture_frame, samples_per_frame, render_frame, capture_frame_out);)
				break;
			}
			default:
				CHECK_RET(nvt_apm_process_ex(instance, capture_frame, samples_per_frame, NULL, capture_frame_out);)
				break;
			}

			wav_write_data(h_out, (unsigned char *)capture_frame_out, byte_per_frame);
		}
		else
		{
			printf("EOF\n");
			break;
		}
	}
	CHECK_RET(nvt_apm_stop(instance);)

	nvt_apm_uninit(instance);

	free(capture_frame);
	free(capture_frame_out);
	free(render_frame);
	free(render_frame_out);
	if (NULL != h_ref)
	{
		wav_read_close(h_ref);
	}
	wav_read_close(h_rec);
	wav_write_close(h_out);
	return 0;
}
