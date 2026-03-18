/**
 * @file audio_control_wrapper.h
 * @brief C wrapper for C++ Audio Control functions for ONVIF integration
 * 
 * Provides C-callable functions to interact with the ipcam::media::AudioControl
 * class from ONVIF C code.
 */

#ifndef ONVIF_AUDIO_CONTROL_WRAPPER_H
#define ONVIF_AUDIO_CONTROL_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// Audio Codec Types
// ============================================================================
typedef enum {
    ONVIF_AUDIO_CODEC_PCM = 0,
    ONVIF_AUDIO_CODEC_AAC = 1,
    ONVIF_AUDIO_CODEC_G711_ULAW = 2,
    ONVIF_AUDIO_CODEC_G711_ALAW = 3,
    ONVIF_AUDIO_CODEC_G726 = 4
} OnvifAudioCodec;

// ============================================================================
// Audio Configuration
// ============================================================================
typedef struct {
    bool enabled;
    OnvifAudioCodec codec;
    int sample_rate;    // Hz: 8000, 16000, etc.
    int bitrate;        // bps
    int channels;       // 1 or 2
    int bit_width;      // 8 or 16
} OnvifAudioConfig;

// ============================================================================
// Audio Source Configuration
// ============================================================================
typedef struct {
    char token[64];
    char name[64];
    int channels;
} OnvifAudioSource;

// ============================================================================
// Audio Encoder Configuration
// ============================================================================
typedef struct {
    char token[64];
    char name[64];
    OnvifAudioCodec codec;
    int sample_rate;
    int bitrate;
    bool multicast_enabled;
    char multicast_address[16];
    int multicast_port;
    int multicast_ttl;
} OnvifAudioEncoderConfig;

// ============================================================================
// Audio Control Functions
// ============================================================================

/**
 * @brief Get audio configuration
 * @param config Output configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_audio_config(OnvifAudioConfig *config);

/**
 * @brief Set audio configuration
 * @param config Configuration to set
 * @return 0 on success, -1 on error
 */
int onvif_set_audio_config(const OnvifAudioConfig *config);

/**
 * @brief Enable/disable audio
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_audio_enabled(bool enabled);

/**
 * @brief Set audio codec
 * @param codec Codec type
 * @return 0 on success, -1 on error
 */
int onvif_set_audio_codec(OnvifAudioCodec codec);

/**
 * @brief Set audio sample rate
 * @param sample_rate Sample rate in Hz
 * @return 0 on success, -1 on error
 */
int onvif_set_audio_sample_rate(int sample_rate);

/**
 * @brief Set audio bitrate
 * @param bitrate Bitrate in bps
 * @return 0 on success, -1 on error
 */
int onvif_set_audio_bitrate(int bitrate);

/**
 * @brief Get number of audio sources
 * @return Number of audio sources
 */
int onvif_get_audio_source_count(void);

/**
 * @brief Get audio source info
 * @param idx Source index
 * @param source Output source info
 * @return 0 on success, -1 on error
 */
int onvif_get_audio_source(int idx, OnvifAudioSource *source);

/**
 * @brief Get audio encoder configuration
 * @param config Output encoder config
 * @return 0 on success, -1 on error
 */
int onvif_get_audio_encoder_config(OnvifAudioEncoderConfig *config);

/**
 * @brief Set audio encoder configuration
 * @param config Encoder config to set
 * @return 0 on success, -1 on error
 */
int onvif_set_audio_encoder_config(const OnvifAudioEncoderConfig *config);

/**
 * @brief Get supported audio codecs count
 * @return Number of supported codecs
 */
int onvif_get_supported_audio_codec_count(void);

/**
 * @brief Get supported audio codec by index
 * @param idx Codec index
 * @return Codec type, or -1 on error
 */
OnvifAudioCodec onvif_get_supported_audio_codec(int idx);

/**
 * @brief Get supported sample rates count
 * @return Number of supported sample rates
 */
int onvif_get_supported_sample_rate_count(void);

/**
 * @brief Get supported sample rate by index
 * @param idx Sample rate index
 * @return Sample rate in Hz, or -1 on error
 */
int onvif_get_supported_sample_rate(int idx);

#ifdef __cplusplus
}
#endif

#endif /* ONVIF_AUDIO_CONTROL_WRAPPER_H */
