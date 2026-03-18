#include <stdio.h>
#include <string.h>
#include "ns_func.h"
#include "apm.h"
//#include "debug.h"

/**
 * @brief Create apm ns instance and init it 
 * @return 0: suceeded, others: fail
 */
int ns_init(NS_INFO *info)
{
    NVT_APM_CONFIG *config = malloc(sizeof(NVT_APM_CONFIG));
    memset(config, 0, sizeof(NVT_APM_CONFIG));
	config->ns_enabled = info->enable;
	config->ns_level = info->ns_level;
	config->rec_sampling_rate = info->sampling_rate;
	config->rec_channel_count = info->channel_count;
	config->ref_sampling_rate = info->sampling_rate;
	config->ref_channel_count = info->channel_count;
    info->_config = config;
	nvt_apm_init(&info->_ns_lib_instance);
	nvt_apm_set(info->_ns_lib_instance, APM_PARAM_CONFIG, info->_config);
	nvt_apm_start(info->_ns_lib_instance);
	return 0;
}

/**
 * @brief Release apm instance
 * 
 * @param instance apm instance
 * @return 0: suceeded, others: fail
 */
int ns_close(NS_INFO *info)
{
    if(info->_config)
    {
        free(info->_config);
        info->_config = NULL; 
    }
	if(info->_ns_lib_instance)
	{
		nvt_apm_uninit(info->_ns_lib_instance);
		info->_ns_lib_instance = NULL;
	}
	return 0;
}

/**
 * @brief apply noise suppression to audio_frame buffer
 * 
 * @param audio_frame audio_frame buffer
 * @return 0: suceeded, others: fail
 */
int ns_run(NS_INFO *info, short *audio_frame, int sample_count_per_frame)
{
	return nvt_apm_process_ex(info->_ns_lib_instance, audio_frame, sample_count_per_frame, NULL, audio_frame);
}

/**
 * @brief detect dB level
 * 
 * @param audio_frame audio_frame buffer
 * @return >0: fail, others: dB level (e.g. -30)
 */
int ns_get_db(NS_INFO *info, short *audio_frame, int sample_count_per_frame)
{
	if (!info->_ns_lib_instance)
		return 1;
	int ref_average_db;
	nvt_apm_push_reference(info->_ns_lib_instance, audio_frame, sample_count_per_frame);
	nvt_apm_get(info->_ns_lib_instance, APM_PARAM_GET_REF_DB, &ref_average_db);
	return ref_average_db;
}