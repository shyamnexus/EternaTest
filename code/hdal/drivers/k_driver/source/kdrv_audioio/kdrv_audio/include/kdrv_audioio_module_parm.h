#ifndef __DSP_MODULE_PARM_H__
#define __DSP_MODULE_PARM_H__

int audio_i2s_num[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_i2s_num, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_i2s_num, "SSP index for each dsp interface");

int audio_enable[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_enable, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_enable, "is record enable for each ssp interface");

int audio_tdm_chan[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_tdm_chan, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_tdm_chan, "ssp channel count for each ssp interface");

int audio_sample_size[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_sample_size, uint, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_sample_size, "sample size of for each ssp interface");

int audio_sample_rate[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_sample_rate, uint, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_sample_rate, "sample rate of for each ssp interface");

int audio_bit_clock[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_bit_clock, uint, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_bit_clock, "bit clock for each ssp interface");

int audio_i2s_master[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_i2s_master, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_i2s_master, "dsp ssp_x is master/slave");

int audio_is_stereo[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_is_stereo, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_is_stereo, "is stereo on for each ssp interface");

int audio_i2s_dataorder_type[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_i2s_dataorder_type, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_i2s_dataorder_type,  "i2s TDM Data order type");

int audio_source[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_source, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_source,  "audio_source");

int audio_i2s_mclk[PLATFORM_TOATL_SSP_CNT] = {0};
module_param_array(audio_i2s_mclk, int, NULL, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(audio_i2s_mclk,  "mclk rate of for each ssp interface");


#endif /* __DSP_MODULE_PARM_H__ */
