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
#include <kwrap/examsys.h>

#define PCM_DEVICE_REC "hw:0,0"
#define PCM_DEVICE_UAC "hw:3,0"

static snd_pcm_t *pcm_handle_uac;
static snd_pcm_t *pcm_handle_rc;
static snd_pcm_uframes_t frames;
static char *alsa_buff;
static int cap_exit = 0;

static void *capture_thread(void *arg)
{
	int pcm;

	/* pull data test */
	while (cap_exit == 0) {

		pcm = snd_pcm_readi(pcm_handle_rc, alsa_buff, frames);
        if (pcm == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(pcm_handle_rc);
        } else if (pcm < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(pcm));
        } else if (pcm != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", pcm);
        }

		pcm = snd_pcm_writei(pcm_handle_uac, alsa_buff, frames);
		if (pcm == -EPIPE) {
			printf("XRUN.\n");
			snd_pcm_prepare(pcm_handle_uac);
		} else if (pcm < 0) {
			printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
		}
	}

	return 0;
}


EXAMFUNC_ENTRY(uac_in_with_alsa, argc, argv)
{
	int pcm;
	unsigned int rate, channels, tmp;
	int ret;
	int key;
	pthread_t cap_thread_id;

	snd_pcm_hw_params_t *params;

	int buff_size;

	if (argc < 3) {
		printf("Usage: %s <sample_rate> <channels> \n",
								argv[0]);
		return -1;
	}

	rate 	 = atoi(argv[1]);
	channels = atoi(argv[2]);


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
	//printf("channels: %i ", tmp);

	//if (tmp == 1)
	//	printf("(mono)\n");
	//else if (tmp == 2)
	//	printf("(stereo)\n");

	snd_pcm_hw_params_get_rate(params, &tmp, 0);
	//printf("rate: %d bps\n", tmp);

	/* Allocate buffer to hold single period */
	snd_pcm_hw_params_get_period_size(params, &frames, 0);

	printf("frames: %d\n", frames);

	buff_size = 1024 * 2 * 2 /* 2 -> sample size */;
	alsa_buff = (char *) malloc(buff_size);

	/* Open PCM device for recording (capture). */
    pcm = snd_pcm_open(&pcm_handle_rc, PCM_DEVICE_REC, SND_PCM_STREAM_CAPTURE, 0);
    if (pcm < 0) {
            fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(pcm));
            exit(1);
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(pcm_handle_rc, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(pcm_handle_rc, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(pcm_handle_rc, params, SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(pcm_handle_rc, params, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    rate = 48000;
    snd_pcm_hw_params_set_rate_near(pcm_handle_rc, params, &rate, 0);

    /* Set period size to 32 frames. */
    frames = 16;
    snd_pcm_hw_params_set_period_size_near(pcm_handle_rc, params, &frames, 0);

    /* Write the parameters to the driver */
    pcm = snd_pcm_hw_params(pcm_handle_rc, params);
    if (pcm < 0) {
            fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(pcm));
            exit(1);
    }

	//create capture thread
	ret = pthread_create(&cap_thread_id, NULL, capture_thread, NULL);
	if (ret < 0) {
		printf("create encode thread failed");
	}


	printf("Enter q to exit\n");
	while (1) {
		key = NVT_EXAMSYS_GETCHAR();
		if (key == 'q' || key == 0x3) {
			cap_exit = 1;
			break;
		}
	}

	pthread_join(cap_thread_id, NULL);

	snd_pcm_drain(pcm_handle_uac);
	snd_pcm_close(pcm_handle_uac);

	snd_pcm_drain(pcm_handle_rc);
	snd_pcm_close(pcm_handle_rc);
	free(alsa_buff);

	return 0;
}
