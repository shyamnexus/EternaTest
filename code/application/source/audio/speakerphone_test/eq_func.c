#include "eq_func.h"
#include <string.h>
#include <stdio.h>

#define MAX_PATTERN_COUNT 12
#define MAX_PARAMETER_COUNT 3

int eq_init(EQ_INFO *info)
{
	// init EQ lib
	info->AudFilterInit.filt_ch = info->channel_count; //channel count
	info->AudFilterInit.smooth_enable = 1;
	audlib_filt_open(&info->AudFilterInit);
	audlib_filt_init();

    AUDFILT_EQPARAM EqParam;
    EqParam.sample_rate = info->sampling_rate; //audio sample rate
    EqParam.Q = 1.0f;  //typical curve
    
    if (info->patterns[0])
    {
        //fint pattern count
        int pattern_count = 1;
        char* ptr = info->patterns + strlen(info->patterns) - 1;
        while (ptr >= info->patterns)
        {
            if (*(ptr--) == '+')
                pattern_count++;
        }
        
        audlib_filt_enable_eq(ENABLE, pattern_count);
        printf("enable EQ pattern count %d\r\n", pattern_count);


        //char *patterns[MAX_PATTERN_COUNT];
        int pattern_index = 0;
        
        char *rest_patterns = info->patterns;
        char *pattern = strtok_r(rest_patterns, "+", &rest_patterns);

        while (pattern != NULL && pattern_index < MAX_PATTERN_COUNT)
        {
            //char *parameters[MAX_PARAMETER_COUNT];
            int parameter_index = 0;

            //patterns[pattern_index] = pattern;

            char *rest_parameters = pattern;
            char *parameter = strtok_r(rest_parameters, "_", &rest_parameters);

            while (parameter != NULL && parameter_index < MAX_PARAMETER_COUNT)
            {
                //parameters[parameter_index] = parameter;
                switch (parameter_index)
                {
                    case 0:  //type
                        if(strcmp(parameter, "LP") == 0)
                        {
                            EqParam.filt_type = AUDFILT_DESIGNTYPE_LOWPASS;
                        }
                        else if(strcmp(parameter, "HP") == 0)
                        {
                            EqParam.filt_type = AUDFILT_DESIGNTYPE_HIGHPASS;
                        }
                        else if(strcmp(parameter, "LS") == 0)
                        {
                            EqParam.filt_type = AUDFILT_DESIGNTYPE_LOWSHELF;
                        }
                        else if(strcmp(parameter, "HS") == 0)
                        {
                            EqParam.filt_type = AUDFILT_DESIGNTYPE_HIGHSHELF;
                        }
                        else if(strcmp(parameter, "PK") == 0)
                        {
                            EqParam.filt_type = AUDFILT_DESIGNTYPE_PEAKING;
                        }
                        else if(strcmp(parameter, "NC") == 0)
                        {
                            EqParam.filt_type = AUDFILT_DESIGNTYPE_NOTCH;
                        }
                    case 1: //frequency
                        EqParam.frequency = atof(parameter);
                        printf("EqParam.frequency = %f, from parameter=%s\r\n", EqParam.frequency, parameter);
                    case 2:  // dB
                    default:
                        EqParam.gain_db = atof(parameter);
                }

                parameter_index++;
                parameter = strtok_r(NULL, "_", &rest_parameters);
            }

            audlib_filt_design_eq(pattern_index, &EqParam); //filter index 
            printf("design EQ filter %d, %f, %f\r\n", EqParam.filt_type, EqParam.frequency, EqParam.gain_db);
            pattern_index++;
            pattern = strtok_r(NULL, "+", &rest_patterns);
        }

    }
    return 0;
}


int eq_close(EQ_INFO *info)
{
    audlib_filt_close();
    return 0;
}

#ifndef _LP64
int eq_run(EQ_INFO *info, short *audio_frame, int sample_count_per_frame)
#else
int eq_run(EQ_INFO *info, long audio_frame, int sample_count_per_frame)
#endif
{
#ifndef _LP64
    AUDFILT_BITSTREAM AudBitStream;
    AudBitStream.bitstram_buffer_in = (UINT64)audio_frame; //input buffer
    AudBitStream.bitstram_buffer_out = (UINT64)audio_frame; //output buffer
    AudBitStream.bitstram_buffer_length = sample_count_per_frame << 1;
    // apply audio filter
    audlib_filt_run(&AudBitStream);
#else
    // fix bug #IVOT_N12191_CO-181
     audlib_filt_run((UINT64)audio_frame,sample_count_per_frame << 1);
#endif
    return 0;
}