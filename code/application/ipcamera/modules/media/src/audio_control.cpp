#include "ipcam/audio_control.h"
#include "ipcam/hdal_pipeline.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <mutex>
#include <algorithm>

// HDAL vendor audio capture API for gain/AGC/ALC control
extern "C" {
#include "vendor_audiocapture.h"
#include "hd_audiocapture.h"
#include "hd_audioenc.h"
}

namespace ipcam {
namespace media {

namespace {
    std::mutex g_mutex;
}

// ============================================================================
// Singleton Instance
// ============================================================================
AudioControl& AudioControl::Instance() {
    static AudioControl instance;
    return instance;
}

// ============================================================================
// Constructor - Initialize capabilities and load config (no HDAL dependency)
// ============================================================================
AudioControl::AudioControl() {
    // Initialize capabilities based on NT98538 hardware specs
    // These are needed for ValidateConfig() before Init() is called
    // Keep all codecs available internally for future use
    capabilities_.supported_codecs = {
        AudioCodec::PCM, 
        AudioCodec::G711_ULAW, AudioCodec::G711_ALAW,
        AudioCodec::G726_16, AudioCodec::G726_24, AudioCodec::G726_32, AudioCodec::G726_40,
        AudioCodec::AAC,
        AudioCodec::ADPCM
    };
    capabilities_.supported_sample_rates = {8000, 16000, 32000, 44100, 48000};
    capabilities_.supported_bit_widths = {8, 16, 32};
    capabilities_.max_channels = 2;
    capabilities_.supports_aec = true;
    capabilities_.supports_anr = true;
    capabilities_.supports_agc = true;
    
    // HDAL volume range (0-160, where 100 = 0dB)
    capabilities_.min_volume = 0;
    capabilities_.max_volume = 160;
    
    // AGC settings range
    capabilities_.agc_min_target_db = -40;
    capabilities_.agc_max_target_db = -3;
    capabilities_.agc_min_noise_gate_db = -80;
    capabilities_.agc_max_noise_gate_db = -20;
    
    // Legacy
    capabilities_.min_gain = 0;
    capabilities_.max_gain = 100;
    capabilities_.min_anr_level = 0;
    capabilities_.max_anr_level = 3;
    
    // Load config from file
    LoadFromConfig();
    
    spdlog::debug("AudioControl constructed with capabilities initialized");
}

// ============================================================================
// Lifecycle
// ============================================================================
bool AudioControl::Init() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (initialized_) {
        spdlog::warn("AudioControl already initialized");
        return true;
    }

    spdlog::info("Initializing Audio Control module (HDAL ready)");

    // Capabilities and config already loaded in constructor
    // This function is called when HDAL is ready for hardware operations

    initialized_ = true;
    spdlog::info("Audio Control module initialized");
    return true;
}

void AudioControl::Shutdown() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!initialized_) {
        return;
    }

    spdlog::info("Shutting down Audio Control module");
    // TODO: Release HDAL resources
    initialized_ = false;
}

// ============================================================================
// Main Configuration
// ============================================================================
bool AudioControl::SetConfig(const AudioConfig& cfg) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (!ValidateConfig(cfg)) {
        spdlog::error("Invalid audio configuration");
        return false;
    }

    config_ = cfg;

    // Save to config file
    config::Set<bool>("media.audio.enabled", cfg.enabled);
    config::Set<std::string>("media.audio.codec", AudioCodecToString(cfg.codec));
    config::Set<int>("media.audio.sample_rate", cfg.sample_rate);
    config::Set<int>("media.audio.bitrate", cfg.bitrate);
    config::Set<int>("media.audio.channels", cfg.channels);
    config::Set<int>("media.audio.bit_width", cfg.bit_width);
    
    // New HDAL settings
    config::Set<std::string>("media.audio.boost_gain", MicBoostGainToString(cfg.boost_gain));
    config::Set<int>("media.audio.volume", cfg.volume);
    config::Set<bool>("media.audio.alc_enabled", cfg.alc_enabled);
    
    // AGC settings
    config::Set<bool>("media.audio.agc_enabled", cfg.agc_enabled);
    config::Set<int>("media.audio.agc_target_db", cfg.agc_target_db);
    config::Set<int>("media.audio.agc_noise_gate_db", cfg.agc_noise_gate_db);
    
    // ANR settings
    config::Set<bool>("media.audio.anr_enabled", cfg.anr_enabled);
    config::Set<int>("media.audio.anr_level", cfg.anr_level);
    config::Set<int>("media.audio.noise_gate_db", cfg.noise_gate_db);
    
    // AEC
    config::Set<bool>("media.audio.aec_enabled", cfg.aec_enabled);
    
    // Legacy (for backward compatibility)
    config::Set<int>("media.audio.input_gain", cfg.input_gain);
    config::Set<int>("media.audio.output_gain", cfg.output_gain);

    spdlog::info("Audio config updated: {} @ {} Hz, {} ch, boost={}, vol={}",
                 AudioCodecToString(cfg.codec), cfg.sample_rate, cfg.channels,
                 MicBoostGainToString(cfg.boost_gain), cfg.volume);

    // Apply to hardware if path is available
    if (cap_ctrl_path_ != 0) {
        ApplyToHardware(cap_ctrl_path_);
    }

    return true;
}

void AudioControl::SetReconfigureCallback(ReconfigureCallback cb) {
    std::lock_guard<std::mutex> lock(g_mutex);
    reconfigure_callback_ = cb;
}

void AudioControl::NotifyAudioProcessorChange() {
    ReconfigureCallback cb;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        cb = reconfigure_callback_;
    }
    if (cb) {
        cb();
    }
}

AudioConfig AudioControl::GetConfig() const {
    return config_;
}

// ============================================================================
// Individual Parameter Setters
// ============================================================================
bool AudioControl::Enable(bool enabled) {
    AudioConfig cfg = config_;
    cfg.enabled = enabled;
    return SetConfig(cfg);
}

bool AudioControl::SetCodec(AudioCodec codec) {
    AudioConfig cfg = config_;
    cfg.codec = codec;
    return SetConfig(cfg);
}

bool AudioControl::SetSampleRate(int sample_rate) {
    AudioConfig cfg = config_;
    cfg.sample_rate = sample_rate;
    return SetConfig(cfg);
}

bool AudioControl::SetBitrate(int bitrate) {
    AudioConfig cfg = config_;
    cfg.bitrate = bitrate;
    return SetConfig(cfg);
}

bool AudioControl::SetChannels(int channels) {
    AudioConfig cfg = config_;
    cfg.channels = channels;
    return SetConfig(cfg);
}

bool AudioControl::SetBitWidth(int bit_width) {
    AudioConfig cfg = config_;
    cfg.bit_width = bit_width;
    return SetConfig(cfg);
}

// ============================================================================
// Audio Processing Features
// ============================================================================
bool AudioControl::SetAEC(bool enabled) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    config_.aec_enabled = enabled;
    config::Set<bool>("media.audio.aec_enabled", enabled);
    config::Save();
    
    spdlog::info("AEC {}", enabled ? "enabled" : "disabled");
    return true;
}

bool AudioControl::SetANR(bool enabled, int level) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (level < capabilities_.min_anr_level || level > capabilities_.max_anr_level) {
        spdlog::error("Invalid ANR level: {} (range: {}-{})", 
                      level, capabilities_.min_anr_level, capabilities_.max_anr_level);
        return false;
    }

    config_.anr_enabled = enabled;
    config_.anr_level = level;
    config::Set<bool>("media.audio.anr_enabled", enabled);
    config::Set<int>("media.audio.anr_level", level);
    config::Save();
    
    spdlog::info("ANR {} level={}", enabled ? "enabled" : "disabled", level);
    return true;
}

// ============================================================================
// Userspace Audio Processing Setters (HPF, Notch, NS)
// ============================================================================
bool AudioControl::SetHPF(bool enabled, int freq_hz, float q) {
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        
        if (freq_hz < 20 || freq_hz > 500) {
            spdlog::error("Invalid HPF frequency: {} Hz (range: 20-500)", freq_hz);
            return false;
        }
        if (q < 0.1f || q > 10.0f) {
            spdlog::error("Invalid HPF Q factor: {} (range: 0.1-10.0)", q);
            return false;
        }
        
        config_.hpf_enabled = enabled;
        config_.hpf_freq = freq_hz;
        config_.hpf_q = q;
        
        config::Set<bool>("media.audio.processing.hpf.enabled", enabled);
        config::Set<int>("media.audio.processing.hpf.frequency", freq_hz);
        config::Set<double>("media.audio.processing.hpf.q", static_cast<double>(q));
        config::Save();
        
        spdlog::info("HPF {} freq={}Hz Q={:.1f}", enabled ? "enabled" : "disabled", freq_hz, q);
    }
    
    // Reconfigure audio processor AFTER releasing mutex
    NotifyAudioProcessorChange();
    return true;
}

bool AudioControl::SetNotchFilter(bool enabled, int freq_hz, float q, int gain_db) {
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        
        // Only allow power line frequencies
        if (freq_hz != 50 && freq_hz != 60) {
            spdlog::error("Invalid notch frequency: {} Hz (must be 50 or 60)", freq_hz);
            return false;
        }
        if (q < 1.0f || q > 30.0f) {
            spdlog::error("Invalid notch Q factor: {} (range: 1.0-30.0)", q);
            return false;
        }
        if (gain_db > 0 || gain_db < -60) {
            spdlog::error("Invalid notch gain: {} dB (range: -60 to 0)", gain_db);
            return false;
        }
        
        config_.notch_enabled = enabled;
        config_.notch_freq = freq_hz;
        config_.notch_q = q;
        config_.notch_gain = gain_db;
        
        config::Set<bool>("media.audio.processing.notch.enabled", enabled);
        config::Set<int>("media.audio.processing.notch.frequency", freq_hz);
        config::Set<double>("media.audio.processing.notch.q", static_cast<double>(q));
        config::Set<int>("media.audio.processing.notch.gain_db", gain_db);
        config::Save();
        
        spdlog::info("Notch filter {} freq={}Hz Q={:.1f} gain={}dB",
                     enabled ? "enabled" : "disabled", freq_hz, q, gain_db);
    }
    
    // Reconfigure audio processor AFTER releasing mutex
    NotifyAudioProcessorChange();
    return true;
}

bool AudioControl::SetNS(bool enabled, int level_db) {
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        
        if (level_db < -60 || level_db > -1) {
            spdlog::error("Invalid NS level: {} dB (range: -60 to -1)", level_db);
            return false;
        }
        
        // Warn if sample rate is not 8kHz (NS only works at 8kHz)
        if (enabled && config_.sample_rate != 8000) {
            spdlog::warn("NS requires 8kHz sample rate! Current: {}Hz. NS may not work correctly.",
                         config_.sample_rate);
        }
        
        config_.ns_enabled = enabled;
        config_.ns_level = level_db;
        
        config::Set<bool>("media.audio.processing.ns.enabled", enabled);
        config::Set<int>("media.audio.processing.ns.level_db", level_db);
        config::Save();
        
        spdlog::info("Noise Suppression {} level={}dB", enabled ? "enabled" : "disabled", level_db);
    }
    
    // Reconfigure audio processor AFTER releasing mutex
    NotifyAudioProcessorChange();
    return true;
}

bool AudioControl::SetAGC(bool enabled) {
    // Overload without parameters - use current config values
    return SetAGC(enabled, config_.agc_target_db, config_.agc_noise_gate_db);
}

bool AudioControl::SetAGC(bool enabled, int target_db, int noise_gate_db) {
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        
        // Validate target level
        if (target_db < capabilities_.agc_min_target_db || 
            target_db > capabilities_.agc_max_target_db) {
            spdlog::error("Invalid AGC target: {} dB (range: {} to {} dB)", 
                          target_db, capabilities_.agc_min_target_db, 
                          capabilities_.agc_max_target_db);
            return false;
        }
        
        config_.agc_enabled = enabled;
        config_.agc_target_db = target_db;
        config_.agc_noise_gate_db = noise_gate_db;
        
        config::Set<bool>("media.audio.agc_enabled", enabled);
        config::Set<int>("media.audio.agc_target_db", target_db);
        config::Set<int>("media.audio.agc_noise_gate_db", noise_gate_db);
        config::Save();
        
        // Apply to hardware if available
        if (cap_ctrl_path_ != 0) {
            HD_PATH_ID path = static_cast<HD_PATH_ID>(cap_ctrl_path_);
            VENDOR_AUDIOCAP_AGC_CONFIG agc_cfg = {};
            agc_cfg.enable = enabled ? TRUE : FALSE;
            agc_cfg.target_lvl = target_db;
            agc_cfg.ng_threshold = noise_gate_db;
            
            HD_RESULT ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_AGC_CONFIG, &agc_cfg);
            if (ret != HD_OK) {
                spdlog::warn("Failed to apply AGC config to hardware: {}", static_cast<int>(ret));
            }
        }
        
        spdlog::info("AGC {} target={}dB noise_gate={}dB", 
                     enabled ? "enabled" : "disabled", target_db, noise_gate_db);
    }
    
    // Reconfigure audio processor AFTER releasing mutex
    NotifyAudioProcessorChange();
    return true;
}

// ============================================================================
// New HDAL-compatible setters
// ============================================================================
bool AudioControl::SetBoostGain(MicBoostGain gain) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    config_.boost_gain = gain;
    config::Set<std::string>("media.audio.boost_gain", MicBoostGainToString(gain));
    config::Save();
    
    // Apply to hardware if available
    if (cap_ctrl_path_ != 0) {
        HD_PATH_ID path = static_cast<HD_PATH_ID>(cap_ctrl_path_);
        VENDOR_AUDIOCAP_DEFAULT_SETTING setting;
        
        switch (gain) {
            case MicBoostGain::DB_0:  setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_0DB; break;
            case MicBoostGain::DB_10: setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_10DB; break;
            case MicBoostGain::DB_20: setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB; break;
            case MicBoostGain::DB_30: setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_30DB; break;
            default: setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB; break;
        }
        
        HD_RESULT ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING, &setting);
        if (ret != HD_OK) {
            spdlog::warn("Failed to apply boost gain to hardware: {}", static_cast<int>(ret));
        }
    }
    
    spdlog::info("Mic boost gain set to {}", MicBoostGainToString(gain));
    return true;
}

bool AudioControl::SetVolume(int volume) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (volume < capabilities_.min_volume || volume > capabilities_.max_volume) {
        spdlog::error("Invalid volume: {} (range: {}-{})", 
                      volume, capabilities_.min_volume, capabilities_.max_volume);
        return false;
    }
    
    config_.volume = volume;
    config::Set<int>("media.audio.volume", volume);
    config::Save();
    
    // Apply to hardware using stop-set-start cycle for reliable volume changes
    if (cap_ctrl_path_ != 0) {
        auto& pipeline = platform::HdalPipeline::Instance();
        HD_PATH_ID cap_path = static_cast<HD_PATH_ID>(pipeline.GetAudioCapturePath());
        HD_PATH_ID enc_path = static_cast<HD_PATH_ID>(pipeline.GetAudioEncoderPath());
        HD_PATH_ID ctrl_path = static_cast<HD_PATH_ID>(cap_ctrl_path_);
        
        // Stop audio capture and encoder
        if (cap_path != 0) {
            hd_audiocap_stop(cap_path);
        }
        if (enc_path != 0) {
            hd_audioenc_stop(enc_path);
        }
        
        // Set volume while stopped
        HD_AUDIOCAP_VOLUME vol = {};
        vol.volume = static_cast<UINT32>(volume);
        HD_RESULT ret = hd_audiocap_set(ctrl_path, HD_AUDIOCAP_PARAM_VOLUME, &vol);
        if (ret != HD_OK) {
            spdlog::warn("Failed to set volume: {}", static_cast<int>(ret));
        }
        
        // Restart encoder first, then capture (per SDK sample order)
        if (enc_path != 0) {
            hd_audioenc_start(enc_path);
        }
        if (cap_path != 0) {
            hd_audiocap_start(cap_path);
        }
        
        spdlog::info("Volume set to {} via stop-set-start cycle", volume);
    }
    
    return true;
}

bool AudioControl::SetALC(bool enabled) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    config_.alc_enabled = enabled;
    config::Set<bool>("media.audio.alc_enabled", enabled);
    config::Save();
    
    // Apply to hardware if available
    if (cap_ctrl_path_ != 0) {
        HD_PATH_ID path = static_cast<HD_PATH_ID>(cap_ctrl_path_);
        INT32 alc_en = enabled ? TRUE : FALSE;
        
        HD_RESULT ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_ALC_ENABLE, &alc_en);
        if (ret != HD_OK) {
            spdlog::warn("Failed to apply ALC setting to hardware: {}", static_cast<int>(ret));
        }
    }
    
    spdlog::info("ALC (hardware auto-level) {}", enabled ? "enabled" : "disabled");
    return true;
}

bool AudioControl::SetNoiseGate(int threshold_db) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    config_.noise_gate_db = threshold_db;
    config::Set<int>("media.audio.noise_gate_db", threshold_db);
    config::Save();
    
    // Apply to hardware if available
    if (cap_ctrl_path_ != 0) {
        HD_PATH_ID path = static_cast<HD_PATH_ID>(cap_ctrl_path_);
        INT32 threshold = threshold_db;
        
        HD_RESULT ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD, &threshold);
        if (ret != HD_OK) {
            spdlog::warn("Failed to apply noise gate to hardware: {}", static_cast<int>(ret));
        }
    }
    
    spdlog::info("Noise gate threshold set to {} dB", threshold_db);
    return true;
}

// ============================================================================
// Gain Control
// ============================================================================
bool AudioControl::SetInputGain(int gain) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (gain < capabilities_.min_gain || gain > capabilities_.max_gain) {
        spdlog::error("Invalid input gain: {} (range: {}-{})", 
                      gain, capabilities_.min_gain, capabilities_.max_gain);
        return false;
    }

    config_.input_gain = gain;
    config::Set<int>("media.audio.input_gain", gain);
    config::Save();
    
    spdlog::debug("Input gain set to {}", gain);
    return true;
}

bool AudioControl::SetOutputGain(int gain) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (gain < capabilities_.min_gain || gain > capabilities_.max_gain) {
        spdlog::error("Invalid output gain: {} (range: {}-{})", 
                      gain, capabilities_.min_gain, capabilities_.max_gain);
        return false;
    }

    config_.output_gain = gain;
    config::Set<int>("media.audio.output_gain", gain);
    config::Save();
    
    spdlog::debug("Output gain set to {}", gain);
    return true;
}

int AudioControl::GetInputGain() const {
    return config_.input_gain;
}

int AudioControl::GetOutputGain() const {
    return config_.output_gain;
}

// ============================================================================
// Capabilities
// ============================================================================
AudioCapabilities AudioControl::GetCapabilities() const {
    return capabilities_;
}

// ============================================================================
// Status
// ============================================================================
bool AudioControl::IsEnabled() const {
    return config_.enabled;
}

// ============================================================================
// Audio Test
// ============================================================================
bool AudioControl::TestAudio() {
    spdlog::info("Testing audio...");
    // TODO: Implement audio test - play a test tone or capture/playback
    return true;
}

// ============================================================================
// Config Operations
// ============================================================================
bool AudioControl::LoadFromConfig() {
    config_.enabled = config::Get<bool>("media.audio.enabled", true);
    config_.codec = StringToAudioCodec(
        config::Get<std::string>("media.audio.codec", "g711u"));
    config_.sample_rate = config::Get<int>("media.audio.sample_rate", 8000);
    config_.bitrate = config::Get<int>("media.audio.bitrate", 64000);
    config_.channels = config::Get<int>("media.audio.channels", 1);
    config_.bit_width = config::Get<int>("media.audio.bit_width", 16);
    
    // New HDAL settings
    config_.boost_gain = StringToMicBoostGain(
        config::Get<std::string>("media.audio.boost_gain", "20db"));
    config_.volume = config::Get<int>("media.audio.volume", 100);
    config_.alc_enabled = config::Get<bool>("media.audio.alc_enabled", false);
    
    // AGC settings
    config_.agc_enabled = config::Get<bool>("media.audio.agc_enabled", false);
    config_.agc_target_db = config::Get<int>("media.audio.agc_target_db", -6);
    config_.agc_noise_gate_db = config::Get<int>("media.audio.agc_noise_gate_db", -50);
    
    // ANR settings
    config_.anr_enabled = config::Get<bool>("media.audio.anr_enabled", false);
    config_.anr_level = config::Get<int>("media.audio.anr_level", 3);
    config_.noise_gate_db = config::Get<int>("media.audio.noise_gate_db", -58);
    
    // AEC
    config_.aec_enabled = config::Get<bool>("media.audio.aec_enabled", false);
    
    // Userspace audio processing settings
    // HPF (High-Pass Filter)
    config_.hpf_enabled = config::Get<bool>("media.audio.processing.hpf.enabled", false);
    config_.hpf_freq = config::Get<int>("media.audio.processing.hpf.frequency", 100);
    config_.hpf_q = static_cast<float>(config::Get<double>("media.audio.processing.hpf.q", 0.5));
    
    // Notch Filter
    config_.notch_enabled = config::Get<bool>("media.audio.processing.notch.enabled", false);
    config_.notch_freq = config::Get<int>("media.audio.processing.notch.frequency", 60);
    config_.notch_q = static_cast<float>(config::Get<double>("media.audio.processing.notch.q", 5.0));
    config_.notch_gain = config::Get<int>("media.audio.processing.notch.gain_db", -40);
    
    // Noise Suppression (NS)
    config_.ns_enabled = config::Get<bool>("media.audio.processing.ns.enabled", false);
    config_.ns_level = config::Get<int>("media.audio.processing.ns.level_db", -15);
    
    // Legacy (for backward compatibility)
    config_.input_gain = config::Get<int>("media.audio.input_gain", 80);
    config_.output_gain = config::Get<int>("media.audio.output_gain", 80);

    spdlog::info("Audio config loaded: {} @ {}Hz, {}ch, boost={}, vol={}, NS={}, HPF={}, AGC={}",
                 AudioCodecToString(config_.codec), config_.sample_rate,
                 config_.channels, MicBoostGainToString(config_.boost_gain),
                 config_.volume, config_.ns_enabled, config_.hpf_enabled, config_.agc_enabled);
    return true;
}

bool AudioControl::SaveToConfig() {
    return config::Save();
}

bool AudioControl::ResetToDefaults() {
    std::lock_guard<std::mutex> lock(g_mutex);

    spdlog::info("Resetting audio settings to defaults");

    config_ = AudioConfig{};
    return SaveToConfig();
}

// ============================================================================
// Apply all audio settings to HDAL hardware
// Called by hdal_pipeline after audio paths are opened
// ============================================================================
bool AudioControl::ApplyToHardware(uint64_t cap_ctrl_path) {
    if (cap_ctrl_path == 0) {
        spdlog::warn("ApplyToHardware: Invalid capture control path");
        return false;
    }
    
    cap_ctrl_path_ = cap_ctrl_path;
    HD_PATH_ID path = static_cast<HD_PATH_ID>(cap_ctrl_path);
    HD_RESULT ret;
    bool success = true;
    
    spdlog::info("Applying audio settings to hardware (cap_ctrl=0x{:x})", cap_ctrl_path);
    
    // 1. Set boost gain (microphone pre-amplifier)
    {
        VENDOR_AUDIOCAP_DEFAULT_SETTING setting;
        switch (config_.boost_gain) {
            case MicBoostGain::DB_0:  setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_0DB; break;
            case MicBoostGain::DB_10: setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_10DB; break;
            case MicBoostGain::DB_20: setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB; break;
            case MicBoostGain::DB_30: setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_30DB; break;
            default: setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB; break;
        }
        
        ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING, &setting);
        if (ret != HD_OK) {
            spdlog::warn("Failed to set boost gain: {}", static_cast<int>(ret));
        } else {
            spdlog::info("  Boost gain: {}", MicBoostGainToString(config_.boost_gain));
        }
    }
    
    // 2. Set volume (0-160, 100 = 0dB)
    // HDAL may require stop-set-start cycle for runtime volume changes
    {
        auto& pipeline = platform::HdalPipeline::Instance();
        HD_PATH_ID cap_path = static_cast<HD_PATH_ID>(pipeline.GetAudioCapturePath());
        HD_PATH_ID enc_path = static_cast<HD_PATH_ID>(pipeline.GetAudioEncoderPath());
        bool pipeline_running = pipeline.IsRunning() && cap_path != 0;
        
        // If pipeline is running, do stop-set-start cycle
        if (pipeline_running) {
            spdlog::info("  Volume change: stopping audio capture...");
            hd_audiocap_stop(cap_path);
            if (enc_path != 0) {
                hd_audioenc_stop(enc_path);
            }
        }
        
        // Set volume on control path
        HD_AUDIOCAP_VOLUME vol = {};
        vol.volume = static_cast<UINT32>(config_.volume);
        ret = hd_audiocap_set(path, HD_AUDIOCAP_PARAM_VOLUME, &vol);
        if (ret != HD_OK) {
            spdlog::warn("Failed to set volume: {}", static_cast<int>(ret));
        }
        
        // Restart if we stopped
        if (pipeline_running) {
            if (enc_path != 0) {
                hd_audioenc_start(enc_path);
            }
            hd_audiocap_start(cap_path);
            spdlog::info("  Volume change: audio capture restarted");
        }
        
        spdlog::info("  Volume: {} (100=0dB)", config_.volume);
    }
    
    // 3. Configure ALC (hardware automatic level control)
    // IMPORTANT: Disable ALC when using software AGC/ANR/AEC
    {
        INT32 alc_en = config_.alc_enabled ? TRUE : FALSE;
        
        // If AGC is enabled, force ALC off
        if (config_.agc_enabled && config_.alc_enabled) {
            spdlog::warn("AGC enabled - disabling ALC to prevent interference");
            alc_en = FALSE;
            config_.alc_enabled = false;
        }
        
        ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_ALC_ENABLE, &alc_en);
        if (ret != HD_OK) {
            spdlog::warn("Failed to set ALC: {}", static_cast<int>(ret));
        } else {
            spdlog::info("  ALC: {}", alc_en ? "enabled" : "disabled");
        }
    }
    
    // 4. Configure AGC (software automatic gain control)
    if (config_.agc_enabled) {
        VENDOR_AUDIOCAP_AGC_CONFIG agc_cfg = {};
        agc_cfg.enable = TRUE;
        agc_cfg.target_lvl = config_.agc_target_db;
        agc_cfg.ng_threshold = config_.agc_noise_gate_db;
        
        ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_AGC_CONFIG, &agc_cfg);
        if (ret != HD_OK) {
            spdlog::warn("Failed to configure AGC: {}", static_cast<int>(ret));
        } else {
            spdlog::info("  AGC: enabled, target={}dB, noise_gate={}dB", 
                        config_.agc_target_db, config_.agc_noise_gate_db);
        }
    }
    
    // 5. Set noise gate threshold
    {
        INT32 threshold = config_.noise_gate_db;
        ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD, &threshold);
        if (ret != HD_OK) {
            spdlog::warn("Failed to set noise gate: {}", static_cast<int>(ret));
        } else {
            spdlog::info("  Noise gate: {}dB", config_.noise_gate_db);
        }
    }
    
    // 6. Set gain level resolution (use 32 levels for finer control)
    {
        UINT32 gain_lvl = VENDOR_AUDIOCAP_GAIN_LEVEL32;
        ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL, &gain_lvl);
        if (ret != HD_OK) {
            spdlog::warn("Failed to set gain level resolution: {}", static_cast<int>(ret));
        }
    }
    
    spdlog::info("Audio hardware settings applied");
    return success;
}

bool AudioControl::ValidateConfig(const AudioConfig& cfg) const {
    spdlog::debug("ValidateConfig: sample_rate={} bit_width={} channels={} volume={} anr_level={} agc_target_db={}",
                  cfg.sample_rate, cfg.bit_width, cfg.channels, cfg.volume, cfg.anr_level, cfg.agc_target_db);
    
    // Validate sample rate
    auto sr_it = std::find(capabilities_.supported_sample_rates.begin(),
                           capabilities_.supported_sample_rates.end(),
                           cfg.sample_rate);
    if (sr_it == capabilities_.supported_sample_rates.end()) {
        spdlog::error("Unsupported sample rate: {}", cfg.sample_rate);
        return false;
    }

    // Validate bit width
    auto bw_it = std::find(capabilities_.supported_bit_widths.begin(),
                           capabilities_.supported_bit_widths.end(),
                           cfg.bit_width);
    if (bw_it == capabilities_.supported_bit_widths.end()) {
        spdlog::error("Unsupported bit width: {}", cfg.bit_width);
        return false;
    }

    // Validate channels
    if (cfg.channels < 1 || cfg.channels > capabilities_.max_channels) {
        spdlog::error("Invalid channel count: {} (max: {})", 
                      cfg.channels, capabilities_.max_channels);
        return false;
    }

    // Validate volume (new HDAL range) - only if explicitly set (non-zero or using new API)
    if (cfg.volume < capabilities_.min_volume || cfg.volume > capabilities_.max_volume) {
        spdlog::error("Invalid volume: {} (range: {}-{})",
                      cfg.volume, capabilities_.min_volume, capabilities_.max_volume);
        return false;
    }

    // Validate ANR level
    if (cfg.anr_level < capabilities_.min_anr_level || 
        cfg.anr_level > capabilities_.max_anr_level) {
        spdlog::error("Invalid ANR level: {} (range: {}-{})", 
                      cfg.anr_level, capabilities_.min_anr_level, capabilities_.max_anr_level);
        return false;
    }
    
    // Validate AGC target
    if (cfg.agc_target_db < capabilities_.agc_min_target_db ||
        cfg.agc_target_db > capabilities_.agc_max_target_db) {
        spdlog::error("Invalid AGC target: {} dB (range: {} to {})", 
                      cfg.agc_target_db, capabilities_.agc_min_target_db, capabilities_.agc_max_target_db);
        return false;
    }

    return true;
}

// ============================================================================
// Helper Function Implementations
// ============================================================================
std::string AudioCodecToString(AudioCodec codec) {
    switch (codec) {
        case AudioCodec::PCM: return "pcm";
        case AudioCodec::AAC: return "aac";
        case AudioCodec::G711_ULAW: return "g711_ulaw";
        case AudioCodec::G711_ALAW: return "g711_alaw";
        case AudioCodec::ADPCM: return "adpcm";
        case AudioCodec::G726_16: return "g726_16";
        case AudioCodec::G726_24: return "g726_24";
        case AudioCodec::G726_32: return "g726_32";
        case AudioCodec::G726_40: return "g726_40";
        default: return "g711_ulaw";  // Default to G.711 μ-law
    }
}

AudioCodec StringToAudioCodec(const std::string& str) {
    if (str == "pcm") return AudioCodec::PCM;
    if (str == "aac") return AudioCodec::AAC;  // AAC-LC via vo-aacenc (royalty-free)
    if (str == "g711_ulaw" || str == "ulaw" || str == "g711u") return AudioCodec::G711_ULAW;
    if (str == "g711_alaw" || str == "alaw" || str == "g711a") return AudioCodec::G711_ALAW;
    if (str == "adpcm") return AudioCodec::ADPCM;
    if (str == "g726_16" || str == "g726-16") return AudioCodec::G726_16;
    if (str == "g726_24" || str == "g726-24") return AudioCodec::G726_24;
    if (str == "g726_32" || str == "g726-32" || str == "g726") return AudioCodec::G726_32;
    if (str == "g726_40" || str == "g726-40") return AudioCodec::G726_40;
    return AudioCodec::G711_ULAW;  // Default to G.711 μ-law
}

std::string AudioSoundModeToString(AudioSoundMode mode) {
    switch (mode) {
        case AudioSoundMode::Mono: return "mono";
        case AudioSoundMode::Stereo: return "stereo";
        default: return "mono";
    }
}

AudioSoundMode StringToAudioSoundMode(const std::string& str) {
    if (str == "stereo") return AudioSoundMode::Stereo;
    return AudioSoundMode::Mono;
}

std::string MicBoostGainToString(MicBoostGain gain) {
    switch (gain) {
        case MicBoostGain::DB_0:  return "0db";
        case MicBoostGain::DB_10: return "10db";
        case MicBoostGain::DB_20: return "20db";
        case MicBoostGain::DB_30: return "30db";
        default: return "20db";
    }
}

MicBoostGain StringToMicBoostGain(const std::string& str) {
    if (str == "0db" || str == "0dB" || str == "0") return MicBoostGain::DB_0;
    if (str == "10db" || str == "10dB" || str == "10") return MicBoostGain::DB_10;
    if (str == "20db" || str == "20dB" || str == "20") return MicBoostGain::DB_20;
    if (str == "30db" || str == "30dB" || str == "30") return MicBoostGain::DB_30;
    return MicBoostGain::DB_20;  // Default
}

} // namespace media
} // namespace ipcam
