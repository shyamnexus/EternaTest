#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define USE_IPC 0


#if USE_IPC
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define AUDIO_DATA_OFFSET 0x2A300
#define UNIX_DOMAIN "/tmp/UNIX.domain"
#define MMAP_DATA_SIZE 0x200000
#define VERSION "1.0.1"



typedef struct _frame_st
{
    unsigned int data_size;
    unsigned int audio_data_size;
	void *data;
} frame_st;

frame_st *frame_nvt;
void *p;
//static int send_request = 0;
#endif

#include <alsa/asoundlib.h>

void dump_device_info (int card, int dev)
{
    int err;
    int rc;
    snd_pcm_t *handle;
   
    snd_pcm_hw_params_t *params;
     unsigned int min, max;
    
     
    char name[32];

    printf("dump_device_info\r\n");
	//sprintf(name, "hw:%d", card);
    sprintf(name, "hw:%d,%d",card,dev);
    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handle, name,
        SND_PCM_STREAM_CAPTURE, 0);
        
    if (rc < 0) {
        fprintf(stderr,
                "unable to open pcm device: %s\n",
                snd_strerror(rc));
        exit(1);
    }
        
    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);
   
    err = snd_pcm_hw_params_get_channels_max(params, &max);    
    if(err<0)
        exit(1);
    printf("Max channels: %u.\r\n", max);
  
    err = snd_pcm_hw_params_get_rate_min(params, &min, 0);
    if(err<0)
        exit(1);   
    printf("Min sample rate: %u. \r\n", min);
  
    err = snd_pcm_hw_params_get_rate_max(params, &max, 0);
    if(err<0)
        exit(1);
    printf("Max sample rate: %u. \r\n", max);
        
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
}


static void device_list(void)
{
	snd_ctl_t *handle;
	int card, err, dev, idx;
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	char name[32];
    snd_ctl_card_info_alloca(&info);
    snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
    
    snd_pcm_info_alloca(&pcminfo);
    
	card = -1;
	
    if (snd_card_next(&card) < 0 || card < 0) {
		printf("no soundcards found...\r\n");
		return;
	}
	printf("**** List of %s Hardware Devices ****\n",
	       snd_pcm_stream_name(stream));
	while (card >= 0) {
	               
        sprintf(name, "hw:%d", card);
        printf("open %s\r\n",name);
		if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
		
			goto next_card;
		}
      
		if ((err = snd_ctl_card_info(handle, info)) < 0) {
		
			snd_ctl_close(handle);
			goto next_card;
		}
		dev = -1;
		while (1) {
			unsigned int count;
            if (snd_ctl_pcm_next_device(handle, &dev)<0)
				printf("snd_ctl_pcm_next_device\r\n");	    
			if (dev < 0)
				break;
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, stream);
			if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
				
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
			        printf("error\r\n");
				} else {
					printf("  Subdevice #%i: %s\n",
						idx, snd_pcm_info_get_subdevice_name(pcminfo));
                    
                    dump_device_info(card,idx);
				}
			}
		}
        
		snd_ctl_close(handle);
	next_card:
		if (snd_card_next(&card) < 0) {
				break;
		}
	}
}


#if USE_IPC
int init_share_memory(void)
{
	int fd = shm_open("frame", O_RDWR, 0666);
	ftruncate(fd, MMAP_DATA_SIZE);
	p = (void *) mmap(NULL, MMAP_DATA_SIZE,
				   PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	frame_nvt = (frame_st *)p;
	//based on frame data size
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

    FILE *fp1;
    size_t numwritten;
    long loops;
    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;

#if USE_IPC	
	static struct sockaddr_un srv_addr;
	int fd;
	int connect_fd;
	char buffer_socket[4];
	int ret = 0;
#endif	
    int err;

    unsigned int min, max;
    
    if( argc==2 && strstr(argv[1], "-l") )
    {
        device_list();
        return 0;
    }
    
    fp1 = fopen("test_host.raw","wb");
    if(fp1!=0)
    {
        printf("open test.raw successfully!!");
    }
    else
    {
        printf("fail to open test.raw\r\n");
        return 0;
    }

#if USE_IPC
	//creat unix socket
	connect_fd=socket(PF_UNIX,SOCK_STREAM,0);
	if(connect_fd<0)
	{
		printf("cannot create communication socket\n");
		return 1;
	}
	srv_addr.sun_family=AF_UNIX;
	strcpy(srv_addr.sun_path,UNIX_DOMAIN);
	//connect server
	ret=connect(connect_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));
	if(ret==-1)
	{
		printf("cannot connect to the server");
		close(connect_fd);
		return 1;
	}

	fd = init_share_memory();
#endif	

    /* Open PCM device for recording (capture). */
    err = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        printf("unable to open pcm device\r\n");
        exit(1);
    }
    else
        printf("open pcm default successully\r\n");
    

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);
   
    

    /* Fill it in with default values. */
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_any successully\r\n");
  
    // Get min and max number of channels
    err = snd_pcm_hw_params_get_channels_min(params, &min);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_get_channels_min successully\r\n");
    printf("Min channels: %u.\r\n", min);
    
    err = snd_pcm_hw_params_get_channels_max(params, &max);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_get_channels_max successully\r\n");    
    printf("Max channels: %u.\r\n", max);
  
    // Get min and max sample rate
    err = snd_pcm_hw_params_get_rate_min(params, &min, 0);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_get_rate_min successully\r\n");
    printf("Min sample rate: %u. \r\n", min);
  
    err = snd_pcm_hw_params_get_rate_max(params, &max, 0);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_get_rate_max successully\r\n");        
    printf("Max sample rate: %u. \r\n", max);

    /* Set the desired hardware parameters. */


    /* Interleaved mode */
    err = snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_set_access successully\r\n");
        
    /* Signed 16-bit little-endian format */
    err = snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_set_format successully\r\n");
        
    /* Two channels (stereo) */
    err = snd_pcm_hw_params_set_channels(handle, params, 2);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_set_channels successully\r\n");

    /* 44100 bits/second sampling rate (CD quality) */
    val = 44100;
    err = snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_set_rate_near successully\r\n");
                                  

    /* Set period size to 32 frames. */
    frames = 1024;
    err = snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_set_period_size_near successully\r\n");

    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf("unable to set hw parameters\n");
        exit(1);
    }
    else
        printf("snd_pcm_hw_params successully\r\n");

    /* Use a buffer large enough to hold one period */
    err = snd_pcm_hw_params_get_period_size(params,
                                      &frames, &dir);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_get_period_size successully\r\n");
    

    
    size = frames * 4; /* 2 bytes/sample, 2 channels */
    buffer = (char *) malloc(size);
  
    printf("get period time\r\n");
    /* We want to loop for 5 seconds */
    err = snd_pcm_hw_params_get_period_time(params,
                                         &val, &dir);
    if (err < 0)
        exit (1);
    else
        printf("snd_pcm_hw_params_get_period_time successully\r\n");
    
    loops = 5000000 / val; // duration 5 sec

    while (loops > 0)
    {
        loops--;
        rc = snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            fprintf(stderr,
                "error from read: %s\n",
                snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
        
        numwritten = fwrite(buffer, sizeof(char),size, fp1);
        if(numwritten > 0)
        {
            printf("write %d bytes to file\r\n",numwritten);
        }
#if USE_IPC     
        // !!caution: memory copy to specific address by adding AUDIO_DATA_OFFSET for Audio Stream 
        memcpy(&frame_nvt->data+AUDIO_DATA_OFFSET, buffer, size);
        frame_nvt->audio_data_size = size;
 
        memset(buffer_socket, 0, sizeof(buffer_socket));
        snprintf(buffer_socket, 4, "%s\n", "PCM");	
        write(connect_fd,buffer_socket,sizeof(buffer_socket));
#endif        
    }    
  
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
    fclose(fp1);



#if USE_IPC
	close(connect_fd);
	exit_share_memory(fd);
	puts("exit_share_memory");

#endif
	return 0;
}

