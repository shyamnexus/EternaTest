/**
 * @file audio_processor.h
 * @brief Userspace Audio Processing (NS, AGC, HPF, Notch)
 * 
 * Integrates Novatek SDK audio processing libraries:
 * - libaec.a: Noise Suppression (NS) via low-level AUD_NS_* API
 * - libagc.a: Automatic Gain Control (AGC)
 * - libaudfilt.a: High-Pass Filter (HPF) and Notch Filter
 * 
 * Processing chain: Input → HPF → Notch → NS → AGC → Output
 * 
 * CRITICAL: NS requires 8kHz sample rate and 256-sample frames.
 */

#ifndef IPCAM_AUDIO_PROCESSOR_H
#define IPCAM_AUDIO_PROCESSOR_H

#include <cstdint>
#include <memory>
#include <vector>
#include <atomic>

namespace ipcam {
namespace streaming {

// ============================================================================
// Audio Processor Configuration
// ============================================================================
struct AudioProcessorConfig {
    int sample_rate = 8000;    // Must be 8000 for NS!
    int channels = 1;          // Mono only
    
    // High-Pass Filter (removes low-frequency rumble/wind)
    bool hpf_enabled = false;
    int hpf_freq = 100;        // Hz (20-500)
    float hpf_q = 0.5f;        // Q factor
    
    // Notch Filter (removes power line hum)
    bool notch_enabled = false;
    int notch_freq = 60;       // 50 Hz (EU) or 60 Hz (US)
    float notch_q = 5.0f;      // SDK default
    int notch_gain = -40;      // dB
    
    // Noise Suppression (requires 8kHz!)
    bool ns_enabled = false;
    int ns_level = -15;        // dB: -1 to -60 (vendor default=-15, aggressive=-50)
    
    // AGC (Automatic Gain Control)
    bool agc_enabled = false;
    int agc_target_db = -6;    // Target output level
    int agc_max_gain = 20;     // Maximum amplification (dB)
    int agc_min_gain = -10;    // Minimum attenuation (dB)
    int agc_noise_gate_db = -50;
};

// ============================================================================
// Audio Processor Statistics
// ============================================================================
struct AudioProcessorStats {
    uint64_t frames_processed = 0;
    uint64_t ns_frames = 0;
    uint64_t agc_frames = 0;
    uint64_t filter_frames = 0;
    uint64_t errors = 0;
};

// ============================================================================
// Audio Processor Class
// ============================================================================
class AudioProcessor {
public:
    AudioProcessor();
    ~AudioProcessor();
    
    // Non-copyable
    AudioProcessor(const AudioProcessor&) = delete;
    AudioProcessor& operator=(const AudioProcessor&) = delete;
    
    /**
     * @brief Initialize processor with given configuration
     * @param config Processing configuration
     * @return true on success
     */
    bool Initialize(const AudioProcessorConfig& config);
    
    /**
     * @brief Shutdown and release resources
     */
    void Shutdown();
    
    /**
     * @brief Check if processor is initialized
     */
    bool IsInitialized() const { return initialized_.load(); }
    
    /**
     * @brief Reconfigure processor (will re-initialize)
     * @param config New configuration
     * @return true on success
     */
    bool Reconfigure(const AudioProcessorConfig& config);
    
    /**
     * @brief Process audio samples in-place
     * @param samples Pointer to 16-bit PCM samples
     * @param sample_count Number of samples
     * @return true on success
     * 
     * Processing order: HPF → Notch → NS → AGC
     */
    bool Process(int16_t* samples, size_t sample_count);
    
    /**
     * @brief Get current configuration
     */
    const AudioProcessorConfig& GetConfig() const { return config_; }
    
    /**
     * @brief Check if any processing is enabled
     */
    bool IsProcessingEnabled() const {
        return config_.hpf_enabled || config_.notch_enabled ||
               config_.ns_enabled || config_.agc_enabled;
    }
    
    /**
     * @brief Get processing statistics
     */
    AudioProcessorStats GetStats() const;

private:
    // Initialization helpers
    bool InitializeFilter();
    bool InitializeNS();
    bool InitializeAGC();
    
    // Individual processors
    void ProcessFilter(int16_t* samples, size_t sample_count);
    void ProcessNS(int16_t* samples, size_t sample_count);
    void ProcessAGC(int16_t* samples, size_t sample_count);
    
    // Cleanup helpers
    void ShutdownFilter();
    void ShutdownNS();
    void ShutdownAGC();
    
    std::atomic<bool> initialized_{false};
    AudioProcessorConfig config_;
    
    // NS state (low-level AUD_NS_* API)
    void* ns_internal_buf_ = nullptr;
    uint32_t ns_internal_buf_size_ = 0;
    uint32_t ns_frame_size_ = 256;  // 256 samples at 8kHz
    std::vector<int16_t> ns_output_buf_;
    bool ns_initialized_ = false;
    
    // AGC state (audlib_agc_* API)
    void* agc_internal_buf_ = nullptr;
    uint32_t agc_internal_buf_size_ = 0;
    std::vector<int16_t> agc_output_buf_;
    bool agc_initialized_ = false;
    
    // Filter state (audlib_filt_* API for HPF and Notch)
    bool filter_initialized_ = false;
    
    // Statistics
    std::atomic<uint64_t> frames_processed_{0};
    std::atomic<uint64_t> ns_frames_{0};
    std::atomic<uint64_t> agc_frames_{0};
    std::atomic<uint64_t> filter_frames_{0};
    std::atomic<uint64_t> errors_{0};
};

} // namespace streaming
} // namespace ipcam

#endif // IPCAM_AUDIO_PROCESSOR_H
