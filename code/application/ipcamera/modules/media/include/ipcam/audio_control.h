#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ipcam {
namespace media {

// ============================================================================
// Audio Codec Types
// ============================================================================
enum class AudioCodec {
    PCM,
    AAC,          // AAC-LC via vo-aacenc (patents expired 2017-2018, royalty-free)
    G711_ULAW,    // G.711 μ-law (default for telephony, royalty-free)
    G711_ALAW,    // G.711 A-law (royalty-free)
    ADPCM,
    // G.726 ADPCM variants (royalty-free, better compression than G.711)
    G726_16,      // 16 kbps (4:1 compression)
    G726_24,      // 24 kbps (2.67:1 compression)
    G726_32,      // 32 kbps (2:1 compression) - most common
    G726_40       // 40 kbps (1.6:1 compression)
};

enum class AudioSoundMode {
    Mono,
    Stereo
};

// ============================================================================
// Microphone Boost Gain Settings (VENDOR_AUDIOCAP_DEFAULT_SETTING)
// Pre-amplifier gain before ADC - set according to microphone sensitivity
// ============================================================================
enum class MicBoostGain {
    DB_0  = 0,    // 0 dB  - for high-sensitivity mics, prevents clipping
    DB_10 = 1,    // 10 dB - for normal microphones (speakerphone use)
    DB_20 = 2,    // 20 dB - default, good for most IP camera mics
    DB_30 = 3,    // 30 dB - for far-field/low-sensitivity mics
};

// ============================================================================
// Audio Configuration
// ============================================================================
struct AudioConfig {
    bool enabled = true;
    AudioCodec codec = AudioCodec::G711_ULAW;  // Default to G.711 μ-law
    int sample_rate = 8000;     // Hz: 8000, 16000, 32000, 44100, 48000
    int bitrate = 64000;        // bps (for AAC, G.726)
    int channels = 1;           // 1 (mono) or 2 (stereo)
    int bit_width = 16;         // 8, 16, or 32 bits
    
    // ========== Gain Settings (Novatek HDAL) ==========
    // Microphone boost gain (pre-amp before ADC)
    MicBoostGain boost_gain = MicBoostGain::DB_20;  // Default 20dB
    
    // Volume (0-160, where 100 = 0dB reference)
    // Maps to HD_AUDIOCAP_PARAM_VOLUME
    int volume = 100;           // 0-160 (100 = 0dB)
    
    // ========== Hardware ALC (Automatic Level Control) ==========
    // ALC is hardware-based gain control in the audio codec
    // Should be DISABLED when using software AGC/ANR/AEC
    bool alc_enabled = false;
    
    // ========== Software AGC (Automatic Gain Control) ==========
    // Normalizes volume levels - use instead of ALC for processing
    bool agc_enabled = false;
    int agc_target_db = -6;     // Target output level: -3 to -40 dB
    int agc_noise_gate_db = -50; // Noise gate threshold in dB
    
    // ========== Noise Suppression ==========
    bool anr_enabled = false;   // Audio Noise Reduction
    int anr_level = 3;          // 0-3 (0=weakest, 3=strongest)
    int noise_gate_db = -58;    // Noise gate threshold (dB)
    
    // ========== Echo Cancellation (for intercom/speakerphone) ==========
    bool aec_enabled = false;   // Acoustic Echo Cancellation
    
    // ========== Userspace Audio Processing (via libaec, libagc, libaudfilt) ==========
    // High-Pass Filter (removes low-frequency rumble/wind noise)
    bool hpf_enabled = false;
    int hpf_freq = 100;         // Cutoff frequency in Hz (20-500)
    float hpf_q = 0.5f;         // Q factor (0.5 = gentle rolloff)
    
    // Notch Filter (removes power line hum)
    bool notch_enabled = false;
    int notch_freq = 60;        // 50 Hz (EU/Asia) or 60 Hz (Americas)
    float notch_q = 5.0f;       // SDK default Q
    int notch_gain = -40;       // SDK default attenuation in dB
    
    // Noise Suppression (NS) - CRITICAL: Requires 8kHz sample rate!
    // This replaces the legacy anr_enabled/anr_level with proper NS API
    bool ns_enabled = false;
    int ns_level = -15;         // Noise suppression level: -1 to -60 dB (vendor default=-15)
    
    // Legacy fields (for backward compatibility)
    int input_gain = 80;        // Maps to volume (0-100 -> 0-160)
    int output_gain = 80;       // Speaker output gain
};

// ============================================================================
// Audio Capabilities
// ============================================================================
struct AudioCapabilities {
    std::vector<AudioCodec> supported_codecs;
    std::vector<int> supported_sample_rates;
    std::vector<int> supported_bit_widths;
    int max_channels = 2;
    bool supports_aec = true;
    bool supports_anr = true;
    bool supports_agc = true;
    
    // HDAL volume range
    int min_volume = 0;
    int max_volume = 160;
    
    // AGC settings
    int agc_min_target_db = -40;
    int agc_max_target_db = -3;
    int agc_min_noise_gate_db = -80;
    int agc_max_noise_gate_db = -20;
    
    // Legacy (deprecated)
    int min_gain = 0;
    int max_gain = 100;
    int min_anr_level = 0;
    int max_anr_level = 3;
};

// ============================================================================
// Audio Control Interface
// ============================================================================
class AudioControl {
public:
    static AudioControl& Instance();

    // Lifecycle
    bool Init();
    void Shutdown();
    bool IsInitialized() const { return initialized_; }

    // Main configuration
    bool SetConfig(const AudioConfig& config);
    AudioConfig GetConfig() const;

    // Individual parameter setters
    bool Enable(bool enabled);
    bool SetCodec(AudioCodec codec);
    bool SetSampleRate(int sample_rate);
    bool SetBitrate(int bitrate);
    bool SetChannels(int channels);
    bool SetBitWidth(int bit_width);

    // ========== New HDAL-compatible setters ==========
    // Boost gain (microphone pre-amplifier)
    bool SetBoostGain(MicBoostGain gain);
    MicBoostGain GetBoostGain() const { return config_.boost_gain; }
    
    // Volume (0-160, HDAL compatible)
    bool SetVolume(int volume);
    int GetVolume() const { return config_.volume; }
    
    // ALC (hardware automatic level control)
    bool SetALC(bool enabled);
    bool IsALCEnabled() const { return config_.alc_enabled; }
    
    // AGC with parameters
    bool SetAGC(bool enabled, int target_db = -6, int noise_gate_db = -50);
    
    // Noise gate
    bool SetNoiseGate(int threshold_db);
    int GetNoiseGateDB() const { return config_.noise_gate_db; }
    
    // Audio processing features (legacy API)
    bool SetAEC(bool enabled);
    bool SetANR(bool enabled, int level);
    bool SetAGC(bool enabled);
    
    // ========== Userspace Audio Processing Setters ==========
    // High-Pass Filter
    bool SetHPF(bool enabled, int freq_hz = 100, float q = 0.5f);
    bool IsHPFEnabled() const { return config_.hpf_enabled; }
    int GetHPFFrequency() const { return config_.hpf_freq; }
    
    // Notch Filter
    bool SetNotchFilter(bool enabled, int freq_hz = 60, float q = 5.0f, int gain_db = -40);
    bool IsNotchEnabled() const { return config_.notch_enabled; }
    int GetNotchFrequency() const { return config_.notch_freq; }
    
    // Noise Suppression (NS) - requires 8kHz sample rate
    bool SetNS(bool enabled, int level_db = -20);
    bool IsNSEnabled() const { return config_.ns_enabled; }
    int GetNSLevel() const { return config_.ns_level; }

    // Gain control (legacy - maps to volume)
    bool SetInputGain(int gain);
    bool SetOutputGain(int gain);
    int GetInputGain() const;
    int GetOutputGain() const;

    // Capabilities
    AudioCapabilities GetCapabilities() const;

    // Status
    bool IsEnabled() const;

    // Audio test
    bool TestAudio();

    // Config persistence
    bool SaveToConfig();
    bool ResetToDefaults();
    
    // Apply settings to HDAL (called by pipeline)
    bool ApplyToHardware(uint64_t cap_ctrl_path);

    // Callback for when audio processing settings change (to notify streaming module)
    using ReconfigureCallback = std::function<void()>;
    void SetReconfigureCallback(ReconfigureCallback cb);
    void NotifyAudioProcessorChange();

private:
    AudioControl();  // Constructor initializes capabilities and loads config
    ~AudioControl() = default;
    AudioControl(const AudioControl&) = delete;
    AudioControl& operator=(const AudioControl&) = delete;

    bool LoadFromConfig();
    bool ValidateConfig(const AudioConfig& config) const;

    bool initialized_ = false;
    AudioConfig config_;
    AudioCapabilities capabilities_;
    uint64_t cap_ctrl_path_ = 0;  // HDAL audio capture control path
    
    ReconfigureCallback reconfigure_callback_;
};

// ============================================================================
// Helper Functions
// ============================================================================
std::string AudioCodecToString(AudioCodec codec);
AudioCodec StringToAudioCodec(const std::string& str);

std::string AudioSoundModeToString(AudioSoundMode mode);
AudioSoundMode StringToAudioSoundMode(const std::string& str);

std::string MicBoostGainToString(MicBoostGain gain);
MicBoostGain StringToMicBoostGain(const std::string& str);

} // namespace media
} // namespace ipcam
