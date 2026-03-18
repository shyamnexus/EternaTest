/**
 * @file audio_control_wrapper.cpp
 * @brief C wrapper implementation for Audio Control functions
 * 
 * Implements the C wrapper functions by calling into the C++ AudioControl class.
 */

#include "ipcam/audio_control_wrapper.h"
#include "ipcam/audio_control.h"
#include <spdlog/spdlog.h>
#include <cstring>

using namespace ipcam::media;

// Helper to convert between enum types
static AudioCodec onvif_to_audio_codec(OnvifAudioCodec codec) {
    switch (codec) {
        case ONVIF_AUDIO_CODEC_PCM: return AudioCodec::PCM;
        case ONVIF_AUDIO_CODEC_AAC: return AudioCodec::AAC;
        case ONVIF_AUDIO_CODEC_G711_ULAW: return AudioCodec::G711_ULAW;
        case ONVIF_AUDIO_CODEC_G711_ALAW: return AudioCodec::G711_ALAW;
        case ONVIF_AUDIO_CODEC_G726: return AudioCodec::G726_32;
        default: return AudioCodec::G711_ULAW;
    }
}

static OnvifAudioCodec audio_codec_to_onvif(AudioCodec codec) {
    switch (codec) {
        case AudioCodec::PCM: return ONVIF_AUDIO_CODEC_PCM;
        case AudioCodec::AAC: return ONVIF_AUDIO_CODEC_AAC;
        case AudioCodec::G711_ULAW: return ONVIF_AUDIO_CODEC_G711_ULAW;
        case AudioCodec::G711_ALAW: return ONVIF_AUDIO_CODEC_G711_ALAW;
        case AudioCodec::G726_16:
        case AudioCodec::G726_24:
        case AudioCodec::G726_32:
        case AudioCodec::G726_40: return ONVIF_AUDIO_CODEC_G726;
        default: return ONVIF_AUDIO_CODEC_G711_ULAW;
    }
}

extern "C" {

int onvif_get_audio_config(OnvifAudioConfig *config) {
    if (!config) return -1;
    
    auto& ac = AudioControl::Instance();
    if (!ac.IsInitialized()) {
        spdlog::warn("AudioControl not initialized, returning defaults");
        config->enabled = true;
        config->codec = ONVIF_AUDIO_CODEC_G711_ULAW;
        config->sample_rate = 8000;
        config->bitrate = 64000;
        config->channels = 1;
        config->bit_width = 16;
        return 0;
    }
    
    auto audio_config = ac.GetConfig();
    config->enabled = audio_config.enabled;
    config->codec = audio_codec_to_onvif(audio_config.codec);
    config->sample_rate = audio_config.sample_rate;
    config->bitrate = audio_config.bitrate;
    config->channels = audio_config.channels;
    config->bit_width = audio_config.bit_width;
    
    return 0;
}

int onvif_set_audio_config(const OnvifAudioConfig *config) {
    if (!config) return -1;
    
    auto& ac = AudioControl::Instance();
    if (!ac.IsInitialized()) {
        spdlog::warn("AudioControl not initialized");
        return -1;
    }
    
    AudioConfig audio_config;
    audio_config.enabled = config->enabled;
    audio_config.codec = onvif_to_audio_codec(config->codec);
    audio_config.sample_rate = config->sample_rate;
    audio_config.bitrate = config->bitrate;
    audio_config.channels = config->channels;
    audio_config.bit_width = config->bit_width;
    
    if (ac.SetConfig(audio_config)) {
        return 0;
    }
    return -1;
}

int onvif_set_audio_enabled(bool enabled) {
    auto& ac = AudioControl::Instance();
    if (!ac.IsInitialized()) return -1;
    
    if (ac.Enable(enabled)) {
        return 0;
    }
    return -1;
}

int onvif_set_audio_codec(OnvifAudioCodec codec) {
    auto& ac = AudioControl::Instance();
    if (!ac.IsInitialized()) return -1;
    
    if (ac.SetCodec(onvif_to_audio_codec(codec))) {
        return 0;
    }
    return -1;
}

int onvif_set_audio_sample_rate(int sample_rate) {
    auto& ac = AudioControl::Instance();
    if (!ac.IsInitialized()) return -1;
    
    if (ac.SetSampleRate(sample_rate)) {
        return 0;
    }
    return -1;
}

int onvif_set_audio_bitrate(int bitrate) {
    auto& ac = AudioControl::Instance();
    if (!ac.IsInitialized()) return -1;
    
    if (ac.SetBitrate(bitrate)) {
        return 0;
    }
    return -1;
}

int onvif_get_audio_source_count(void) {
    // Most IP cameras have 1 audio source (built-in microphone)
    return 1;
}

int onvif_get_audio_source(int idx, OnvifAudioSource *source) {
    if (!source || idx != 0) return -1;
    
    strncpy(source->token, "AudioSource_1", sizeof(source->token));
    strncpy(source->name, "Built-in Microphone", sizeof(source->name));
    source->channels = 1;
    
    return 0;
}

int onvif_get_audio_encoder_config(OnvifAudioEncoderConfig *config) {
    if (!config) return -1;
    
    OnvifAudioConfig audio_cfg;
    if (onvif_get_audio_config(&audio_cfg) != 0) {
        return -1;
    }
    
    strncpy(config->token, "AudioEncoder_1", sizeof(config->token));
    strncpy(config->name, "Audio Encoder", sizeof(config->name));
    config->codec = audio_cfg.codec;
    config->sample_rate = audio_cfg.sample_rate;
    config->bitrate = audio_cfg.bitrate;
    config->multicast_enabled = false;
    strncpy(config->multicast_address, "0.0.0.0", sizeof(config->multicast_address));
    config->multicast_port = 0;
    config->multicast_ttl = 1;
    
    return 0;
}

int onvif_set_audio_encoder_config(const OnvifAudioEncoderConfig *config) {
    if (!config) return -1;
    
    auto& ac = AudioControl::Instance();
    if (!ac.IsInitialized()) return -1;
    
    // Set codec
    if (!ac.SetCodec(onvif_to_audio_codec(config->codec))) {
        return -1;
    }
    
    // Set sample rate
    if (!ac.SetSampleRate(config->sample_rate)) {
        return -1;
    }
    
    // Set bitrate
    if (!ac.SetBitrate(config->bitrate)) {
        return -1;
    }
    
    return 0;
}

int onvif_get_supported_audio_codec_count(void) {
    return 4;  // G.711 μ-law, G.711 A-law, AAC, G.726
}

OnvifAudioCodec onvif_get_supported_audio_codec(int idx) {
    static const OnvifAudioCodec codecs[] = {
        ONVIF_AUDIO_CODEC_G711_ULAW,
        ONVIF_AUDIO_CODEC_G711_ALAW,
        ONVIF_AUDIO_CODEC_AAC,
        ONVIF_AUDIO_CODEC_G726
    };
    
    if (idx < 0 || idx >= 4) return ONVIF_AUDIO_CODEC_G711_ULAW;
    return codecs[idx];
}

int onvif_get_supported_sample_rate_count(void) {
    return 5;
}

int onvif_get_supported_sample_rate(int idx) {
    static const int rates[] = {8000, 16000, 32000, 44100, 48000};
    
    if (idx < 0 || idx >= 5) return -1;
    return rates[idx];
}

} // extern "C"
