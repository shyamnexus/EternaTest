#include "agc_func.h"
#include "agc/audlib_agc.h"
#include <stdio.h>
/**
 * @brief Create init agc lib
 * @return 0: suceeded, others: fail
 */
int agc_init(AGC_INFO *info)
{
	audlib_agc_open();

	audlib_agc_set_config(AGC_CONFIG_ID_SAMPLERATE, info->sampling_rate);
	audlib_agc_set_config(AGC_CONFIG_ID_CHANNEL_NO, info->channel_count);

	audlib_agc_set_config(AGC_CONFIG_ID_TARGET_LVL, AGC_DB(info->target_db));
	audlib_agc_set_config(AGC_CONFIG_ID_NG_THD,     AGC_DB(info->noise_db));

	//audlib_agc_set_config(AGC_CONFIG_ID_DECAY_TIME, isf_audcap_agc_cfg[path].decay_time);
	//audlib_agc_set_config(AGC_CONFIG_ID_ATTACK_TIME, isf_audcap_agc_cfg[path].attack_time);
	//audlib_agc_set_config(AGC_CONFIG_ID_MAXGAIN, 	AGC_DB(isf_audcap_agc_cfg[path].max_gain));
	//audlib_agc_set_config(AGC_CONFIG_ID_MINGAIN,    AGC_DB(isf_audcap_agc_cfg[path].min_gain));

	return audlib_agc_init() ? 0 : 1; //OK: TRUE
}

int agc_close(AGC_INFO *info)
{
	return audlib_agc_close(); //OK: E_OK = 0
}

int agc_run(AGC_INFO *info, short *audio_frame, int sample_count_per_frame)
{
    if(!info->enable)
    {
        // agc is disabled
        return 0;
    }

    AGC_BITSTREAM agc_io;
    agc_io.bitstram_buffer_in     = (UINT64)audio_frame;
    agc_io.bitstram_buffer_out    = (UINT64)audio_frame;
    agc_io.bitstram_buffer_length = sample_count_per_frame;

    //printf("audio_frame=%p\r\n",audio_frame);
    //printf("agc_io.bitstram_buffer_in=%p\r\n",agc_io.bitstram_buffer_in);
    //return 0;
    return audlib_agc_run(&agc_io) ? 0 : 1;  //OK: TRUE
}

int agc_update_noise_db(AGC_INFO *info, int noise_db)
{
	audlib_agc_set_config(AGC_CONFIG_ID_NG_THD,     AGC_DB(noise_db));
	return 0;
}