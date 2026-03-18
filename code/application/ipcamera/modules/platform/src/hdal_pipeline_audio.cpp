/**
 * @file hdal_pipeline_audio.cpp
 * @brief HDAL Pipeline - Audio capture and encoding paths
 *
 * This file contains all audio-related HDAL operations including
 * audio capture path setup and audio encoder configuration.
 *
 * Part of the split hdal_pipeline implementation for maintainability.
 *
 * Audio Processing:
 * - The Novatek HDAL kernel encoder only supports PCM output
 * - All codec encoding (G.711, AAC, G.726) is done in userspace
 * - Audio processing (AGC, ANR, AEC) uses vendor_audiocap API
 * - Volume/boost/noise gate settings via vendor_audiocap API
 */

#include "hdal_pipeline_common.h"
#include "ipcam/hdal_pipeline.h"
#include "ipcam/g711_codec.h"
#include "ipcam/audio_control.h"

namespace ipcam {
namespace platform {

// ============================================================================
// Audio Codec Type Helper
// ============================================================================

int HdalPipeline::GetAudioCodecType(const std::string& codec) {
    // Check if G.711 codec - if so, we'll use PCM internally and encode in userspace
    if (G711Codec::IsG711Codec(codec)) {
        // Return PCM - G.711 encoding will be done in userspace
        spdlog::info("G.711 codec '{}' requested - using PCM internally with userspace encoding", codec);
        return HD_AUDIO_CODEC_PCM;
    }
    
    if (codec == "aac") {
        // AAC also not supported by kernel encoder, fall back to PCM
        spdlog::warn("AAC codec not supported by kernel encoder - falling back to PCM");
        return HD_AUDIO_CODEC_PCM;
    } else if (codec == "pcm") {
        return HD_AUDIO_CODEC_PCM;
    } else {
        // Default to PCM
        return HD_AUDIO_CODEC_PCM;
    }
}

// ============================================================================
// Audio Path Opening
// ============================================================================

bool HdalPipeline::OpenAudioPaths() {
#if HDAL_PIPELINE_ENABLED
    if (!config_.audio.enabled) {
        spdlog::info("Audio disabled, skipping audio path setup");
        return true;
    }
    
    spdlog::info("Opening HDAL audio paths...");
    
    HD_RESULT ret;
    
    // Configure audio capture device - ALWAYS use 48kHz stereo for hardware
    // (per video_record_with_2path sample - hardware may not support lower rates)
    HD_AUDIOCAP_DEV_CONFIG audio_dev_cfg;
    memset(&audio_dev_cfg, 0, sizeof(audio_dev_cfg));
    audio_dev_cfg.in_max.sample_rate = HD_AUDIO_SR_48000;
    audio_dev_cfg.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    audio_dev_cfg.in_max.mode = HD_AUDIO_SOUND_MODE_STEREO;  // Always stereo for hardware
    audio_dev_cfg.in_max.frame_sample = 1024;  // Match sample
    audio_dev_cfg.frame_num_max = 10;
    
    // Open audio capture control
    ret = hd_audiocap_open(static_cast<HD_IN_ID>(0), static_cast<HD_OUT_ID>(HD_AUDIOCAP_0_CTRL), &audio_stream_.cap_ctrl);
    if (ret != HD_OK) {
        spdlog::error("hd_audiocap_open(CTRL) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Set device configuration
    ret = hd_audiocap_set(audio_stream_.cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_dev_cfg);
    if (ret != HD_OK) {
        spdlog::error("hd_audiocap_set(DEV_CONFIG) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Set audio driver configuration
    HD_AUDIOCAP_DRV_CONFIG audio_drv_cfg;
    memset(&audio_drv_cfg, 0, sizeof(audio_drv_cfg));
    audio_drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
    ret = hd_audiocap_set(audio_stream_.cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_drv_cfg);
    if (ret != HD_OK) {
        spdlog::warn("hd_audiocap_set(DRV_CONFIG) failed: {}", static_cast<int>(ret));
        // Non-fatal
    }
    
    // Open audio capture path
    ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &audio_stream_.cap_path);
    if (ret != HD_OK) {
        spdlog::error("hd_audiocap_open(path) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Open audio encoder path
    ret = hd_audioenc_open(HD_AUDIOENC_0_IN_0, HD_AUDIOENC_0_OUT_0, &audio_stream_.enc_path);
    if (ret != HD_OK) {
        spdlog::error("hd_audioenc_open failed: {}", static_cast<int>(ret));
        return false;
    }
    
    spdlog::info("Audio paths opened: cap_path=0x{:x} enc_path=0x{:x}", 
                 audio_stream_.cap_path, audio_stream_.enc_path);
    
    // Store the configured codec and determine if G.711 userspace encoding is needed
    audio_stream_.configured_codec = config_.audio.codec;
    audio_stream_.g711_type = G711Codec::GetTypeFromString(config_.audio.codec);
    
    if (audio_stream_.g711_type != G711Type::kNone) {
        spdlog::info("Audio G.711 userspace encoding enabled: {} -> {}", 
                     config_.audio.codec,
                     audio_stream_.g711_type == G711Type::kUlaw ? "μ-law" : "A-law");
    }
    
    audio_stream_.enabled = true;
    
    // Apply audio processing settings (volume, boost, AGC, etc.) via AudioControl
    media::AudioControl::Instance().ApplyToHardware(
        static_cast<uint64_t>(audio_stream_.cap_ctrl));
    
    spdlog::info("HDAL audio paths opened");
    return true;
#else
    return true;
#endif
}

// ============================================================================
// Audio Configuration - Split into Capture and Encoder phases
// ============================================================================

bool HdalPipeline::ConfigureAudioCapture() {
#if HDAL_PIPELINE_ENABLED
    if (!config_.audio.enabled || !audio_stream_.enabled) {
        return true;
    }
    
    spdlog::info("Configuring audio capture...");
    
    HD_RESULT ret;
    
    // ALWAYS use 48kHz stereo for hardware capture (per video_record_with_2path sample)
    // Sample rate conversion to 8kHz for G.711 will be done in userspace
    HD_AUDIOCAP_IN acap_in;
    memset(&acap_in, 0, sizeof(acap_in));
    acap_in.sample_rate = HD_AUDIO_SR_48000;  // Hardware uses 48kHz
    acap_in.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    acap_in.mode = HD_AUDIO_SOUND_MODE_STEREO;  // Hardware uses stereo
    acap_in.frame_sample = 1024;  // Match sample
    
    ret = hd_audiocap_set(audio_stream_.cap_path, HD_AUDIOCAP_PARAM_IN, &acap_in);
    if (ret != HD_OK) {
        spdlog::error("hd_audiocap_set(IN) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    spdlog::info("Audio capture configured: 48000Hz stereo (hardware) -> {}Hz {} (output)", 
                 config_.audio.sample_rate, config_.audio.codec);
    return true;
#else
    return true;
#endif
}

bool HdalPipeline::ConfigureAudioEncoder() {
#if HDAL_PIPELINE_ENABLED
    if (!config_.audio.enabled || !audio_stream_.enabled) {
        return true;
    }
    
    spdlog::info("Configuring audio encoder...");
    
    HD_RESULT ret;
    
    // ALWAYS use 48kHz stereo for encoder (matching capture hardware)
    // Sample rate conversion happens in userspace for G.711
    
    // Set audio encoder path configuration
    HD_AUDIOENC_PATH_CONFIG aenc_path_cfg;
    memset(&aenc_path_cfg, 0, sizeof(aenc_path_cfg));
    aenc_path_cfg.max_mem.codec_type = HD_AUDIO_CODEC_PCM;  // Always PCM from kernel
    aenc_path_cfg.max_mem.sample_rate = HD_AUDIO_SR_48000;  // Match hardware
    aenc_path_cfg.max_mem.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    aenc_path_cfg.max_mem.mode = HD_AUDIO_SOUND_MODE_STEREO;  // Match hardware
    
    ret = hd_audioenc_set(audio_stream_.enc_path, HD_AUDIOENC_PARAM_PATH_CONFIG, &aenc_path_cfg);
    if (ret != HD_OK) {
        spdlog::warn("hd_audioenc_set(PATH_CONFIG) failed: {} - audio encoding disabled", static_cast<int>(ret));
        // Audio encoder not available - disable audio but continue with video
        audio_stream_.enabled = false;
        return true;  // Non-fatal - video pipeline can work without audio
    }
    
    // Set audio encoder input parameters - match capture hardware (48kHz stereo)
    HD_AUDIOENC_IN aenc_in;
    memset(&aenc_in, 0, sizeof(aenc_in));
    aenc_in.sample_rate = HD_AUDIO_SR_48000;  // Match hardware
    aenc_in.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    aenc_in.mode = HD_AUDIO_SOUND_MODE_STEREO;  // Match hardware
    
    ret = hd_audioenc_set(audio_stream_.enc_path, HD_AUDIOENC_PARAM_IN, &aenc_in);
    if (ret != HD_OK) {
        spdlog::error("hd_audioenc_set(IN) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Set audio encoder output parameters
    // Always use PCM for kernel encoder - G.711 encoding and resampling happens in userspace
    HD_AUDIOENC_OUT aenc_out;
    memset(&aenc_out, 0, sizeof(aenc_out));
    aenc_out.codec_type = HD_AUDIO_CODEC_PCM;  // Always PCM from kernel
    aenc_out.aac_adts = FALSE;
    
    ret = hd_audioenc_set(audio_stream_.enc_path, HD_AUDIOENC_PARAM_OUT, &aenc_out);
    if (ret != HD_OK) {
        spdlog::error("hd_audioenc_set(OUT) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Log the actual configuration
    spdlog::info("Audio encoder configured: 48kHz stereo PCM (kernel) -> {} (userspace)", 
                 audio_stream_.g711_type != G711Type::kNone ? 
                 (audio_stream_.g711_type == G711Type::kUlaw ? "G.711 μ-law 8kHz" : "G.711 A-law 8kHz") :
                 config_.audio.codec);
    return true;
#else
    return true;
#endif
}

// Legacy wrapper for compatibility - calls both capture and encoder config
bool HdalPipeline::ConfigureAudio() {
    // Note: This should only be called if audio binding has already happened
    // For proper initialization order, use ConfigureAudioCapture() before binding
    // and ConfigureAudioEncoder() after binding
    if (!ConfigureAudioCapture()) return false;
    return ConfigureAudioEncoder();
}

// ============================================================================
// Audio G.711 Support Getters
// ============================================================================

G711Type HdalPipeline::GetAudioG711Type() const {
#if HDAL_PIPELINE_ENABLED
    return audio_stream_.g711_type;
#else
    return G711Type::kNone;
#endif
}

bool HdalPipeline::RequiresG711Encoding() const {
#if HDAL_PIPELINE_ENABLED
    return audio_stream_.enabled && audio_stream_.g711_type != G711Type::kNone;
#else
    return false;
#endif
}

const std::string& HdalPipeline::GetAudioCodecString() const {
#if HDAL_PIPELINE_ENABLED
    return audio_stream_.configured_codec;
#else
    static const std::string empty;
    return empty;
#endif
}

uint64_t HdalPipeline::GetAudioCapturePath() const {
#if HDAL_PIPELINE_ENABLED
    if (audio_stream_.enabled && audio_stream_.cap_path != 0) {
        return static_cast<uint64_t>(audio_stream_.cap_path);
    }
#endif
    return 0;
}

uint64_t HdalPipeline::GetAudioCaptureCtrlPath() const {
#if HDAL_PIPELINE_ENABLED
    if (audio_stream_.enabled && audio_stream_.cap_ctrl != 0) {
        return static_cast<uint64_t>(audio_stream_.cap_ctrl);
    }
#endif
    return 0;
}

uint64_t HdalPipeline::GetAudioEncoderPath() const {
#if HDAL_PIPELINE_ENABLED
    if (audio_stream_.enabled && audio_stream_.enc_path != 0) {
        return static_cast<uint64_t>(audio_stream_.enc_path);
    }
#endif
    return 0;
}

bool HdalPipeline::IsAudioEnabled() const {
#if HDAL_PIPELINE_ENABLED
    return audio_stream_.enabled && audio_stream_.cap_path != 0;
#else
    return false;
#endif
}

int HdalPipeline::GetAudioSampleRate() const {
#if HDAL_PIPELINE_ENABLED
    return config_.audio.sample_rate;
#else
    return 8000;
#endif
}

int HdalPipeline::GetAudioChannels() const {
#if HDAL_PIPELINE_ENABLED
    return config_.audio.channels;
#else
    return 1;
#endif
}

} // namespace platform
} // namespace ipcam
