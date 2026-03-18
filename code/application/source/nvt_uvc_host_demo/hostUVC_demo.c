#include "libuvc.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <alsa/asoundlib.h>
#include <pthread.h>
//#include <nvtinfo.h>

#define USE_IPC 0  //For communicate with UVC Out
#define SAVEFILE 0 //for save frame to file

#if USE_IPC
#include <fcntl.h>		 /* For O_* constants */

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define UNIX_DOMAIN "/tmp/UNIX.domain"
#define MMAP_DATA_SIZE 0x200000
#define AUDIO_DATA_OFFSET 0x2A300


typedef struct _frame_st {
	unsigned int data_size;
	unsigned int audio_data_size;
	void *data;
} frame_st;

frame_st *frame_nvt;
void *p;


#endif

#define VERSION "1.0.2"

static int uvc_done = 0;
static int send_request = 0;

typedef struct _audio_para {
	int conn_fd;
	char hw[16];
	unsigned int sample_rate;
	int channel_num;
	int buffer_frames;
} audio_para;

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void cb(uvc_frame_t *frame, void *ptr) {
#if USE_IPC
	char buffer[4];
	int conn_fd = *((int *) ptr);
#endif

#if SAVEFILE
	FILE *fp;
	static int jpeg_count = 0;
	//static const char *H264_FILE = "iOSDevLog.h264";
	static const char *MJPEG_FILE = ".jpeg";
	//static const char *YUYV_FILE = ".YUYV";
	char filename[16];
#endif
	// static unsigned int last_time = 0;

	switch (frame->frame_format) {
		case UVC_COLOR_FORMAT_MJPEG:
#if USE_IPC
			if (send_request) {
				memcpy(&frame_nvt->data, frame->data, frame->data_bytes);
				frame_nvt->data_size = frame->data_bytes;
				memset(buffer, 0, sizeof(buffer));
				snprintf(buffer, 4, "%s\n", "JPG");
			} else {
				//Do other thing
			}
#endif
#if SAVEFILE
			sprintf(filename, "%d%s", jpeg_count, MJPEG_FILE);
			fp = fopen(filename, "w");
			fwrite(frame->data, 1, frame->data_bytes, fp);
			fclose(fp);
			jpeg_count++;
#endif
			break;
		case UVC_COLOR_FORMAT_YUYV:
#if USE_IPC
			if (send_request) {
				memcpy(&frame_nvt->data, frame->data, frame->data_bytes);
				frame_nvt->data_size = frame->data_bytes;
				memset(buffer, 0, sizeof(buffer));
				snprintf(buffer, 4, "%s\n", "YUV");
			} else {
				//Do other thing
			}
#endif
			break;
		case UVC_COLOR_FORMAT_NV12:
#if USE_IPC
			if (send_request) {
				memcpy(&frame_nvt->data, frame->data, frame->data_bytes);
				frame_nvt->data_size = frame->data_bytes;
				memset(buffer, 0, sizeof(buffer));
				snprintf(buffer, 4, "%s\n", "420");
			} else {
				//Do other thing
			}
#endif
			break;
		default:
			break;
	}
#if USE_IPC
	if (send_request) {
		write(conn_fd,buffer,sizeof(buffer));
	
	}
#endif	
	//printf("cb done = %u...%u\n",nvt_timer_tm0_get() - last_time, nvt_timer_tm0_get());
	//last_time = nvt_timer_tm0_get();
}

static void pcm_device_list(void)
{
	snd_ctl_t *handle;
	int card, err, dev, idx;
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_alloca(&pcminfo);
	snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;

	card = -1;
	if (snd_card_next(&card) < 0 || card < 0) {
		printf("no soundcards found...\n");
		return;
	}
	printf("**** List of %s Hardware Devices ****\n",
	       snd_pcm_stream_name(stream));
	while (card >= 0) {
		char name[32];
		sprintf(name, "hw:%d", card);
		if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
			printf("control open (%i): %s", card, snd_strerror(err));
			goto next_card;
		}
		if ((err = snd_ctl_card_info(handle, info)) < 0) {
			printf("control hardware info (%i): %s", card, snd_strerror(err));
			snd_ctl_close(handle);
			goto next_card;
		}
		dev = -1;
		while (1) {
			unsigned int count;
			if (snd_ctl_pcm_next_device(handle, &dev)<0)
				printf("snd_ctl_pcm_next_device");
			if (dev < 0)
				break;
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, stream);
			if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
				if (err != -ENOENT)
					printf("control digital audio info (%i): %s", card, snd_strerror(err));
				continue;
			}
			printf("card %i: %s [%s], device %i: %s [%s]\n",
				card, snd_ctl_card_info_get_id(info), snd_ctl_card_info_get_name(info),
				dev,
				snd_pcm_info_get_id(pcminfo),
				snd_pcm_info_get_name(pcminfo));
			count = snd_pcm_info_get_subdevices_count(pcminfo);
			printf("  Subdevices: %i/%i\n",
				snd_pcm_info_get_subdevices_avail(pcminfo), count);
			for (idx = 0; idx < (int)count; idx++) {
				snd_pcm_info_set_subdevice(pcminfo, idx);
				if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
					printf("control digital audio playback info (%i): %s", card, snd_strerror(err));
				} else {
					printf("  Subdevice #%i: %s\n",
						idx, snd_pcm_info_get_subdevice_name(pcminfo));
				}
			}
		}
		snd_ctl_close(handle);
	next_card:
		if (snd_card_next(&card) < 0) {
			printf("snd_card_next");
			break;
		}
	}
}

void* uac_thread(void* data) {
	int err;
	char *buffer;
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    char msg[4];
	audio_para *para = (audio_para *) data;
	unsigned int sample_rate = para->sample_rate;
	int channel_count = para->channel_num;
	int buffer_frames = para->buffer_frames;

    memset(msg, 0, sizeof(msg));
	snprintf(msg, 4, "%s\n", "PCM");

	if ((err = snd_pcm_open (&capture_handle, para->hw, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n",
			 para->hw,
			 snd_strerror (err));
		pthread_exit(NULL);
	}

	fprintf(stdout, "audio interface opened\n");

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		pthread_exit(NULL);
	}

	fprintf(stdout, "hw_params allocated\n");

	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		pthread_exit(NULL);
	}

	fprintf(stdout, "hw_params initialized\n");

	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		pthread_exit(NULL);
	}

	fprintf(stdout, "hw_params access setted\n");

	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		pthread_exit(NULL);
	}

	fprintf(stdout, "hw_params format setted\n");

	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &sample_rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		pthread_exit(NULL);
	}

	fprintf(stdout, "hw_params rate setted\n");

	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, channel_count)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		pthread_exit(NULL);
	}

	fprintf(stdout, "hw_params channels setted\n");

	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		pthread_exit(NULL);
	}

	fprintf(stdout, "hw_params setted\n");

	snd_pcm_hw_params_free (hw_params);

	fprintf(stdout, "hw_params freed\n");

	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		pthread_exit(NULL);
	}

	fprintf(stdout, "audio interface prepared\n");

	buffer = malloc(buffer_frames * snd_pcm_format_width(format) / 8 * channel_count);

	fprintf(stdout, "buffer allocated\n");

	while(1) {
		if (uvc_done == 1)
			break;

		if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
			   err, snd_strerror (err));
			break;
		} else {
#if USE_IPC
			if (send_request) {
				frame_nvt->audio_data_size = buffer_frames * snd_pcm_format_width(format) / 8 * channel_count;
				memcpy( &frame_nvt->data + AUDIO_DATA_OFFSET , buffer, frame_nvt->audio_data_size);
				write(para->conn_fd,msg,sizeof(msg));
			}
#endif
		}
	}

	free(buffer);
	fprintf(stdout, "buffer freed\n");
	snd_pcm_close (capture_handle);
	fprintf(stdout, "audio interface closed\n");
	pthread_exit(NULL);
}

#if USE_IPC
int init_share_memory(void)
{
	int fd = shm_open("frame", O_RDWR, 0666);
	ftruncate(fd, MMAP_DATA_SIZE);
	p = (void *) mmap(NULL, MMAP_DATA_SIZE,
				   PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	//based on frame data size
	frame_nvt = (frame_st *)p;
	frame_nvt->data = malloc(0x160000);
	return fd;
}

void exit_share_memory(int fd)
{
	munmap(p, MMAP_DATA_SIZE);
	close(fd);
}

#endif



int main(int argc, char **argv) {
	uvc_context_t *ctx;
	uvc_device_t *dev;
	uvc_device_handle_t *devh;
	uvc_stream_ctrl_t ctrl;
	uvc_error_t res;
#if USE_IPC
	static struct sockaddr_un srv_addr;
	int ret = 0;
	int fd;
#endif
	int connect_fd = 0;
	pthread_t uac_t;
	audio_para para;

	if (argc != 1 && argc != 4 && argc != 9 && argc != 11) {
		printf("argc should  be 1(uac devlist) be 4(uvc info) or 9(uvc) or 11(uvac)\n");
		return 0;
	}

#if USE_IPC
	//creat unix socket
	connect_fd=socket(PF_UNIX,SOCK_STREAM,0);
	if (connect_fd<0) {
		printf("cannot create communication socket\n");
		return 1;
	}
	srv_addr.sun_family=AF_UNIX;
	strcpy(srv_addr.sun_path,UNIX_DOMAIN);
	//connect server
	ret=connect(connect_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));
	if (ret==-1) {
		printf("cannot connect to the server");
		close(connect_fd);
		return 1;
	}

	fd = init_share_memory();
#endif

	if (argc == 1) {
		pcm_device_list();
#if USE_IPC
		close(connect_fd);
		exit_share_memory(fd);
		puts("exit_share_memory");
#endif
		return 0;
	}

	if (argc == 11) {
		printf("UAC initialized\n");
#if USE_IPC		
		para.conn_fd = connect_fd;
#endif		
		memset(para.hw, '\0', sizeof(para.hw));
		snprintf(para.hw, 5, "%s\n", "hw:0");
		para.sample_rate = atoi(argv[9]);
		para.channel_num = atoi(argv[10]);
        para.buffer_frames = (1000 /atoi(argv[6])) * para.sample_rate / 1000;
		pthread_create(&uac_t, NULL, uac_thread, (void *)&para);
	}

  /* Initialize a UVC service context. Libuvc will set up its own libusb
   * context. Replace NULL with a libusb_context pointer to run libuvc
   * from an existing libusb context. */
	res = uvc_init(&ctx, NULL);

	if (res < 0) {
		uvc_perror(res, "uvc_init");
		return res;
	}

	printf("UVC initialized, VER.%s\n", VERSION);

	/* Find UVC device by VID/PID, stores in dev */
	res = uvc_find_device( ctx, &dev,
		strtol(argv[1], NULL, 16), strtol(argv[2], NULL, 16), NULL );

	if (res < 0) {
		uvc_perror(res, "uvc_find_device"); /* no devices found */
	} else {
		puts("Device found");

		/* Try to open the device: requires exclusive access */
		res = uvc_open(dev, &devh);

		if (res < 0) {
			uvc_perror(res, "uvc_open"); /* unable to open device */
		} else {
			puts("Device opened");

			/* Print out a message containing all the information that libuvc
				* knows about the device */
			if (argc == 4) {
				if (atoi(argv[3]) == 1)
					uvc_print_diag(devh, stderr);

				puts("Device closed");
				uvc_close(devh);
				puts("UVC exited");
				uvc_exit(ctx);

#if USE_IPC
				close(connect_fd);
				exit_share_memory(fd);
				puts("exit_share_memory");
#endif
				return 0;
			}

			const uvc_format_desc_t *format_desc = uvc_get_format_descs(devh);
			const uvc_frame_desc_t *frame_desc = format_desc->frame_descs;
			enum uvc_frame_format frame_format;
			int width = 640;
			int height = 480;
			int fps = 30;


			switch (format_desc->bDescriptorSubtype) {
				case UVC_VS_FORMAT_MJPEG:
					frame_format = UVC_COLOR_FORMAT_MJPEG;
					break;
				default:
					frame_format = UVC_FRAME_FORMAT_YUYV;
					break;
			}

			if (frame_desc) {
				width = frame_desc->wWidth;
				height = frame_desc->wHeight;
				fps = 10000000 / frame_desc->dwDefaultFrameInterval;
			}

			if (argc >= 9) {
				width = atoi(argv[4]);
				height = atoi(argv[5]);
				fps = atoi(argv[6]);
				frame_format = atoi(argv[7]);
				send_request = atoi(argv[8]);

				if (send_request)
					printf("The camera will pass frame to server!\n");
			}

			/* negotiate the format request! */
			res = uvc_get_stream_ctrl_format_size(
				devh, &ctrl, /* result stored in ctrl */
				frame_format,
				width, height, fps );

			/* Print out the result */
			if (res >=0)
				uvc_print_stream_ctrl(&ctrl, stderr);

			if (res < 0) {
				uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
			} else {
				/* Start the video stream. The library will call user function cb:
				*	cb(frame, (void *) 12345)
				*/
				if (res >=0) {
					res = uvc_start_streaming(devh, &ctrl, cb, (void *) &connect_fd, 0);
				}

				if (res < 0) {
					uvc_perror(res, "start_streaming"); /* unable to start stream */
				} else {
					puts("Streaming...");
					sleep(10000); /* stream for 10000 seconds */

					/* End the stream. Blocks until last callback is serviced */
					uvc_stop_streaming(devh);
					puts("Done streaming.");
				}
			}
			/* Release our handle on the device */
			uvc_close(devh);
			puts("Device closed");
		}
		/* Release the device descriptor */
		uvc_unref_device(dev);
	}

  /* Close the UVC context. This closes and cleans up any existing device handles,
   * and it closes the libusb context if one was not provided. */
	uvc_exit(ctx);
	puts("UVC exited");

	uvc_done = 1;
	pthread_join(uac_t, NULL);

#if USE_IPC
	close(connect_fd);
	exit_share_memory(fd);
	puts("exit_share_memory");

#endif

	return 0;
}

