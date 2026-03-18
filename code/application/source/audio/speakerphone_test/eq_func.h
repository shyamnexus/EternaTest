#include <unistd.h>
#include <stdlib.h>
#include "audfilt/audlib_filt.h"

typedef struct _EQ_INFO
{
	int enable;  //enable or disable agc feature
	int sampling_rate;
	int channel_count;
	char patterns[1024];

	//internal use
	AUDFILT_INIT AudFilterInit;
} EQ_INFO;

int eq_init(EQ_INFO *info);
int eq_close(EQ_INFO *info);
#ifndef _LP64
int eq_run(EQ_INFO *info, short *audio_frame, int sample_count_per_frame);
#else
int eq_run(EQ_INFO *info, long audio_frame, int sample_count_per_frame);
#endif