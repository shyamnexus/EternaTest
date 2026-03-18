#if defined(__KERNEL__)
#include<linux/module.h>
#include "Audio.h"
#include "audio_int.h"

EXPORT_SYMBOL(aud_is_opened);
EXPORT_SYMBOL(aud_set_codec_samplerate);
EXPORT_SYMBOL(aud_set_codec_channel);

EXPORT_SYMBOL(aud_init);
EXPORT_SYMBOL(aud_open);
EXPORT_SYMBOL(aud_close);
EXPORT_SYMBOL(aud_get_lock_status);
EXPORT_SYMBOL(aud_is_busy);
EXPORT_SYMBOL(aud_get_transceive_object);
EXPORT_SYMBOL(aud_set_extcodec);
EXPORT_SYMBOL(aud_switch_codec);
EXPORT_SYMBOL(aud_set_device_object);
EXPORT_SYMBOL(aud_get_device_object);
EXPORT_SYMBOL(aud_set_output);
EXPORT_SYMBOL(aud_set_feature);
EXPORT_SYMBOL(aud_set_parameter);
EXPORT_SYMBOL(aud_set_total_vol_level);
EXPORT_SYMBOL(aud_get_total_vol_level);
EXPORT_SYMBOL(aud_set_volume);
EXPORT_SYMBOL(aud_get_volume);
EXPORT_SYMBOL(aud_set_gain);
EXPORT_SYMBOL(aud_set_gaindb);
EXPORT_SYMBOL(aud_set_effect);
EXPORT_SYMBOL(aud_set_record_source);
//EXPORT_SYMBOL(aud_isr_handler);
EXPORT_SYMBOL(aud_set_default_setting);
EXPORT_SYMBOL(aud_set_i2s_chbits);
EXPORT_SYMBOL(aud_set_i2s_format);
EXPORT_SYMBOL(aud_set_i2s_clkratio);
EXPORT_SYMBOL(aud_set_i2s_sentcommand);
EXPORT_SYMBOL(aud_get_i2s_readdata);

EXPORT_SYMBOL(aud_set_total_record_gain_level);
EXPORT_SYMBOL(aud_get_Total_gain_level);

EXPORT_SYMBOL(AudioRealBufferQueue_aud);
EXPORT_SYMBOL(pAudioRealBufferFront_aud);
EXPORT_SYMBOL(pAudioRealBufferRear_aud);
EXPORT_SYMBOL(pAudioRealBufferDone_aud);

EXPORT_SYMBOL(AudioPseudoBufferQueue_aud);
EXPORT_SYMBOL(pAudioPseudoBufferFront_aud);
EXPORT_SYMBOL(pAudioPseudoBufferRear_aud);
EXPORT_SYMBOL(pAudioPseudoBufferDone_aud);

EXPORT_SYMBOL(aud_clock_lost_status_rx);
EXPORT_SYMBOL(aud_clock_lost_status_tx);
EXPORT_SYMBOL(aud_clock_lost_status_tx2);
EXPORT_SYMBOL(aud_clock_lost_status_txlb);



EXPORT_SYMBOL(mylock_aud_qu);
EXPORT_SYMBOL(aud_my_lock_rx);
EXPORT_SYMBOL(aud_my_lock_tx1);
EXPORT_SYMBOL(aud_my_lock_tx2);
EXPORT_SYMBOL(aud_my_lock_txlb);
EXPORT_SYMBOL(aud_my_lock_qu);
EXPORT_SYMBOL(uibuffer_insert_number_tx1);
EXPORT_SYMBOL(uibuffer_insert_number_tx2);
EXPORT_SYMBOL(uibuffer_insert_number_rx);
EXPORT_SYMBOL(uibuffer_insert_number_txlb);



#endif
