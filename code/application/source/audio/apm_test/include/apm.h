#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif

    //typedef enum _NVT_APM_TYPE
    //{
    //    APM_TYPE_AEC, 
    //    APM_TYPE_NS,
    //    APM_TYPE_AGC,
    //    APM_TYPE_HPF,
    //    APM_TYPE_COUNT
    //} NVT_APM_TYPE;

    typedef struct _NVT_APM_CONFIG
    {
        //NVT_APM_TYPE type;
        int ref_channel_count;   // for AEC
        int ref_sampling_rate;   // for AEC
        int rec_channel_count;
        int rec_sampling_rate;
        int delay_ms; // AEC: The tuned delay setting from reference data to record data, e.g. 20
        bool aec_enabled;        // AEC: auto gain control
        bool agc_enabled;        // AGC: auto gain control
        unsigned char post_gain; // ANR or AGC: 0~90: audio gain, e.g. 0
        bool hpf_enabled;
        bool ns_enabled;
        int ns_level; // 0~3: low~high, e.g. 3
        bool residual_echo_detector_enabled;
        bool vad_enabled;
        bool has_keyboard;
        int agc_max_adjust_db_per_second; // AGC: adjust db frequency, e.g. 3
        int agc_max_skip_db; // AGC: if db level is lower than this db setting, drop it.  , e.g. -50
        int agc_max_db; // AGC: control the volume not higher then this db setting.  , e.g. -5
    } NVT_APM_CONFIG;

    typedef enum _NVT_APM_PARAM_ID
    {
        APM_PARAM_MS_DELAY, /// The tuned delay setting from reference data to record data  (for AEC)
        APM_PARAM_CONFIG,
        APM_PARAM_GET_REF_VOICE_DETECTED, // bool: true: detected voice in reference audio  (for AEC)
        APM_PARAM_GET_REF_DB,             // int: average db (for AEC)
        APM_PARAM_SET_POST_GAIN,          // unsigned char: 0~90 db (for NS and AGC)
        APM_PARAM_COUNT
    } NVT_APM_PARAM_ID;

    int nvt_apm_init(void **ref_instance);
    int nvt_apm_get(void *instance, NVT_APM_PARAM_ID id, void *param);
    int nvt_apm_set(void *instance, NVT_APM_PARAM_ID id, void *param);
    int nvt_apm_start(void *instance);
    int nvt_apm_push_reference(void *instance, const short *const ref_data, unsigned long element_count);
    int nvt_apm_process(void *instance, const short *const in_data, unsigned long element_count, short *output_data);
    int nvt_apm_process_ex(void *instance, const short *const in_data, unsigned long element_count, const short *const ref_data, short *output_data);
    int nvt_apm_stop(void *instance);
    void nvt_apm_uninit(void *instance);



#ifdef __cplusplus
}
#endif