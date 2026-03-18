#include <unistd.h>
#include <stdlib.h>

typedef struct _NS_INFO
{
	int enable;  //enable or disable NS feature
	int sampling_rate;
	int channel_count;
	int ns_level;

	//NS lib internal use
	void *_ns_lib_instance;
	void *_config;
} NS_INFO;

int ns_init(NS_INFO *info);
int ns_close(NS_INFO *info);
int ns_run(NS_INFO *info, short *audio_frame, int sample_count_per_frame);
int ns_get_db(NS_INFO *info, short *audio_frame, int sample_count_per_frame);