#include <unistd.h>
#include <stdlib.h>

typedef struct _AGC_INFO
{
	int enable;  //enable or disable agc feature
	int sampling_rate;
	int channel_count;
	int target_db;  //default: -6 db
    int noise_db;   //default: -50 db
} AGC_INFO;

int agc_init(AGC_INFO *info);
int agc_close(AGC_INFO *info);
int agc_run(AGC_INFO *info, short *audio_frame, int sample_count_per_frame);
int agc_update_noise_db(AGC_INFO *info, int noise_db);