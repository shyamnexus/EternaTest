
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
#include <stdint.h>
#include "aec/audlib_aec.h"
#include "kwrap/type.h"
#include "kwrap/util.h"
#include "wavwriter.h"
#include "wavreader.h"

#ifdef _ARMV7_
#include <sys/time.h>
#include <unistd.h>
#endif

//#define WRITE_2CHAN_PCM

static int SAMPLING_RATE = 8000;
static int NN = 2048;
static int TAIL = 2048;

#define NUM_MIC 1
#define NUM_SPEAKER 1

static int CH = 1;

/*----------------------------------*/
/* debug  */
/*----------------------------------*/

/*----------------------------------*/
/* static  */
/*----------------------------------*/
static int _u32FrameCount;
// static void _dynamicParamsTest(void);

// int main_AEC(void);
// int main_AGC(void);
// int main_NS(int);

/*----------------------------------*/
/* main  */
/*----------------------------------*/
int main_bak(int argc, char **argv)
{
    int key;

    // query user key
    printf("Enter e for echo test\n      d for denoise\n      n for noise subpress\n      a for AGC test and q to exit\n");
    while (1)
    {
        key = getchar();
        if (key == 'q' || key == 0x3)
        {
            break;
        }
        if (key == 'e')
        {
            // main_AEC();
            break;
        }
#if 0
        if (key == 'd')
        {
            main_NS(1);
            break;
        }
        if (key == 'n')
        {
            main_NS(0);
            break;
        }
        if (key == 'a')
        {
            main_AGC();
            break;
        }
#endif
    }

    return 0;
}

typedef struct _AUDIO_AEC {

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

#define NVTAEC_QCONSTANT16(x) ((INT32)((500+((x)*(((INT32)1)<<(15))))/1000))

static int aec_init(AUDIO_AEC *aec_t)
{
	INT32 internal_size;
	INT32 fore_size;
	INT32 back_size;

	if (audlib_aec_open() != 0) {
		printf("AEC open failed\r\n");
		return 0;
	}
    
	audlib_aec_set_config(AEC_CONFIG_ID_LEAK_ESTIMTAE_EN, 0);
	audlib_aec_set_config(AEC_CONFIG_ID_LEAK_ESTIMTAE_VAL, NVTAEC_QCONSTANT16(990));

	audlib_aec_set_config(AEC_CONFIG_ID_NOTCH_RADIUS, NVTAEC_QCONSTANT16(992));

	audlib_aec_set_config(AEC_CONFIG_ID_NOISE_CANCEL_LVL, aec_t->noise_suppress);  //Defualt is -20dB. Suggest value range -3 ~ -40. Unit in dB.
	audlib_aec_set_config(AEC_CONFIG_ID_ECHO_CANCEL_LVL, aec_t->echo_suppress);   //Defualt is -50dB. Suggest value range -30 ~ -60. Unit in dB.

	audlib_aec_set_config(AEC_CONFIG_ID_SAMPLERATE, aec_t->sample_rate);
	audlib_aec_set_config(AEC_CONFIG_ID_RECORD_CH_NO, aec_t->mic_ch);
	audlib_aec_set_config(AEC_CONFIG_ID_PLAYBACK_CH_NO, aec_t->spk_ch);
	audlib_aec_set_config(AEC_CONFIG_ID_SPK_NUMBER, 1);

	audlib_aec_set_config(AEC_CONFIG_ID_FILTER_LEN, aec_t->filter_size);
	audlib_aec_set_config(AEC_CONFIG_ID_FRAME_SIZE, aec_t->frame_size);

	audlib_aec_set_config(AEC_CONFIG_ID_PRELOAD_EN, 0);


	internal_size = ALIGN_CEIL_64(audlib_aec_get_required_buffer_size(AEC_BUFINFO_ID_INTERNAL));
	fore_size     = ALIGN_CEIL_64(audlib_aec_get_required_buffer_size(AEC_BUFINFO_ID_FORESIZE));
	back_size     = ALIGN_CEIL_64(audlib_aec_get_required_buffer_size(AEC_BUFINFO_ID_BACKSIZE));

    
	aec_t->aec_internal = calloc(internal_size, sizeof(char));
	aec_t->aec_fore     = calloc(fore_size, sizeof(char));
	aec_t->aec_back     = calloc(back_size, sizeof(char));
    
  

	audlib_aec_set_config(AEC_CONFIG_ID_FOREADDR, (INT64)aec_t->aec_fore);
	audlib_aec_set_config(AEC_CONFIG_ID_FORESIZE, (INT32)fore_size);

  
	audlib_aec_set_config(AEC_CONFIG_ID_BACKADDR, (INT64)aec_t->aec_back);
	audlib_aec_set_config(AEC_CONFIG_ID_BACKSIZE, (INT32)back_size);


	audlib_aec_set_config(AEC_CONFIG_ID_BUF_ADDR, (INT64)aec_t->aec_internal);
	audlib_aec_set_config(AEC_CONFIG_ID_BUF_SIZE, (INT32)internal_size);


	if (!audlib_aec_init()) {
		printf("AEC init failed\r\n");
	}

	return 0;
}


static int aec_close(AUDIO_AEC *aec_t)
{
	audlib_aec_close();

	if (aec_t->aec_internal) {
		free(aec_t->aec_internal);
		aec_t->aec_internal = 0;
	}
	if (aec_t->aec_fore) {
		free(aec_t->aec_fore);
		aec_t->aec_fore = 0;
	}

	if (aec_t->aec_back) {
		free(aec_t->aec_back);
		aec_t->aec_back = 0;
	}

	return 0;
}


static BOOL aec_run(AUDIO_AEC *aec_t, UINT64 addr, UINT64 addr_aec, UINT32 size)
{
	AEC_BITSTREAM AecIO = {0};

	if (((size >> 1) % 1024) != 0) {
		usleep(10000);
		return 0;
	}

	AecIO.bitstream_buffer_play_in = (UINT64)addr_aec;
	AecIO.bitstream_buffer_record_in = (UINT64)addr;
	AecIO.bitstram_buffer_out = (UINT64)addr;
	AecIO.bitstram_buffer_length = (size >> 1);
    
	if (aec_t->mic_ch == 2) {
		AecIO.bitstram_buffer_length = AecIO.bitstram_buffer_length >> 1;
	}

    //printf("AecIO addr=%x\r\n",&AecIO);
    
    audlib_aec_run(AecIO.bitstream_buffer_play_in,AecIO.bitstream_buffer_record_in,AecIO.bitstram_buffer_length);


	return size;
}


// int main_AEC(void)
int main(int argc, char **argv)
{
	AUDIO_AEC aec_config = {0};


    if (argc < 4)
    {
        printf("Please input 3 file paths. ref cap out\n");
        return 1;
    }



#if 0
    char mic_pcm     = "aec_mic.pcm";
    char speaker_pcm[32] = "aec_speaker.pcm";
    char out_pcm[64]     = "aec_output.pcm";
#else
    char *speaker_pcm = argv[1];
    char *mic_pcm = argv[2];
    char *out_pcm = argv[3];
#endif
    // char FilterW_bin[64] = "preload.bin";
    printf("   mic pcm : %s\n", mic_pcm);
    printf("speaker pcm: %s\n", speaker_pcm);
    printf(" output pcm: %s\n\n", out_pcm);


    void *echo_fd = wav_read_open(speaker_pcm);
    if(!echo_fd)
    {
        printf("Open file error: %s\n", speaker_pcm);
        return -1;
    }
    int play_format, play_channels, play_sample_rate, play_bits_per_sample;
    unsigned int play_data_length;
    int res = wav_get_header(echo_fd, &play_format, &play_channels, &play_sample_rate, &play_bits_per_sample, &play_data_length);
    if (!res)
    {
        printf("get ref header error: %d\n", res);
        printf("play_sample_rate=%d\r\n",play_sample_rate);
        printf("play_format=%d\r\n",play_format);
        return -1;
    }
    SAMPLING_RATE = play_sample_rate;
    CH = play_channels;
    printf("sample rate: %d, num_mic: %d, num_speaker: %d, channel: %d\n", SAMPLING_RATE, NUM_MIC, NUM_SPEAKER, CH);

    if (SAMPLING_RATE == 48000)
    {
        NN = 1024;
        TAIL = 1024;
    }
    else if (SAMPLING_RATE == 32000)
    {
        NN = 512;
        TAIL = 1024;
    }



    //FILE *echo_fd, *ref_fd; //, *e_fd;
    // FILE *FilterW_fd, *FilterR_fd;
  
    short *echo_buf = malloc(NUM_SPEAKER * NN * 2);
    short *ref_buf = malloc(NUM_MIC * NN * 2);
    short *e_buf = ref_buf;
    int bufferByteSize = 1024*2;
  

    _u32FrameCount = 0;

    // Open file
    void *ref_fd = wav_read_open(mic_pcm);


    void *h_rec_aec = wav_write_open(out_pcm, SAMPLING_RATE, play_bits_per_sample, play_channels);

    aec_config.sample_rate = SAMPLING_RATE;
	aec_config.spk_ch = 1;
	aec_config.mic_ch = 1;
	aec_config.filter_size = 1024;
	aec_config.frame_size = 128;
	aec_config.echo_suppress = -50;
	aec_config.noise_suppress = -30;

	aec_init(&aec_config);
   
    while (1)
    {
        _u32FrameCount++;

        if (bufferByteSize != wav_read_data(ref_fd, (void *)ref_buf, bufferByteSize))
            break;
        if (bufferByteSize != wav_read_data(echo_fd, (void *)echo_buf, bufferByteSize))
            break;

      
        /* Run */
		aec_run(&aec_config, (UINT64)ref_buf, (UINT64)echo_buf, (UINT32)bufferByteSize);

        // fwrite(e_buf, sizeof(short), micShortSize, e_fd);
        wav_write_data(h_rec_aec, (unsigned char *)e_buf, bufferByteSize);

        printf("[frames]%d\r", _u32FrameCount);

    }

    // fwrite(&stAecPreload.u32ForegroundSize, sizeof(u32), 1, FilterW_fd);
    // fwrite(stAecPreload.ps16Foreground, 1, stAecPreload.u32ForegroundSize, FilterW_fd);
    // fwrite(&stAecPreload.u32BackgroundSize, sizeof(u32), 1, FilterW_fd);
    // fwrite(stAecPreload.ps32Background, 1, stAecPreload.u32BackgroundSize, FilterW_fd);

    // fclose(e_fd);
    wav_write_close(h_rec_aec);
    wav_read_close(echo_fd);
    wav_read_close(ref_fd);
    // fclose(FilterW_fd);
	aec_close(&aec_config);


    free(echo_buf);
    free(ref_buf);
    return 0;
}
