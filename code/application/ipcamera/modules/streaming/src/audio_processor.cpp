/**
 * @file audio_processor.cpp
 * @brief Userspace Audio Processing Implementation
 * 
 * Uses Novatek SDK libraries:
 * - libaec.a: Low-level AUD_NS_* API for Noise Suppression
 * - libagc.a: audlib_agc_* API for AGC
 * - libaudfilt.a: audlib_filt_* API for HPF/Notch filters
 * 
 * Processing chain: Input → HPF → Notch → NS → AGC → Output
 */

#include "ipcam/audio_processor.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <cstdlib>

// Conditional compilation based on available libraries
#ifdef HAVE_LIBAUDFILT
extern "C" {
#include "audfilt/audlib_filt.h"
}
#endif

#ifdef HAVE_LIBAGC
extern "C" {
#include "agc/audlib_agc.h"
}
#endif

#ifdef HAVE_LIBAEC
// Low-level NS API declarations (from libaec.a / aud_ns_api.h)
// Must match vendor enum order exactly!
extern "C" {

typedef struct {
    int s32FrameSize;       // Number of samples in a frame
    int s32ChannelNum;      // Number of channels
    int s32SamplingRate;    // Sampling rate of processed signal
} ST_AUD_NS_INFO;

typedef struct {
    int u32InBufSize;         // Return Input buffer size
    int u32OutBufSize;        // Return Output buffer size
    int u32InternalBufSize;   // Return NS internal buffer size
} ST_AUD_NS_RTN;

// CRITICAL: enum order must match vendor aud_ns_api.h exactly!
typedef enum {
    EN_AUD_NS_NOISE_SUPPRESS = 0,  // Noise suppression level in dB (default=-15)
    EN_AUD_NS_DENOISE = 1,         // Wiener denoiser state (0=off, 1=on)
} EN_AUD_NS_PARAMS;

extern void AUD_NS_PreInit(ST_AUD_NS_INFO *pInfo, ST_AUD_NS_RTN *pRtn);
extern int  AUD_NS_Init(void *pInternalBuf, int u32BufSize);
// NOTE: AUD_NS_Uninit declared in vendor header but NOT exported from prebuilt libaec.so
extern void AUD_NS_Run(short *ps16InBuf, short *ps16OutBuf);
extern int  AUD_NS_SetParam(EN_AUD_NS_PARAMS enParamsCMD, void *pParamsValue);

}
#endif

namespace ipcam {
namespace streaming {

// ============================================================================
// Constructor / Destructor
// ============================================================================
AudioProcessor::AudioProcessor() {
    spdlog::debug("AudioProcessor: Created");
}

AudioProcessor::~AudioProcessor() {
    Shutdown();
    spdlog::debug("AudioProcessor: Destroyed");
}

// ============================================================================
// Initialize
// ============================================================================
bool AudioProcessor::Initialize(const AudioProcessorConfig& config) {
    if (initialized_.load()) {
        spdlog::warn("AudioProcessor: Already initialized, call Reconfigure() instead");
        return true;
    }
    
    config_ = config;
    
    spdlog::info("AudioProcessor: Initializing (rate={}Hz, NS={}, AGC={}, HPF={}, Notch={})",
                 config_.sample_rate, config_.ns_enabled, config_.agc_enabled,
                 config_.hpf_enabled, config_.notch_enabled);
    
    // Initialize filters (HPF and Notch use same library)
    if (config_.hpf_enabled || config_.notch_enabled) {
        if (!InitializeFilter()) {
            spdlog::error("AudioProcessor: Failed to initialize filters");
            Shutdown();
            return false;
        }
    }
    
    // Initialize NS (requires 8kHz!)
    if (config_.ns_enabled) {
        if (config_.sample_rate != 8000) {
            spdlog::error("AudioProcessor: NS requires 8kHz sample rate! Current: {}Hz", 
                         config_.sample_rate);
            Shutdown();
            return false;
        }
        if (!InitializeNS()) {
            spdlog::error("AudioProcessor: Failed to initialize NS");
            Shutdown();
            return false;
        }
    }
    
    // Initialize AGC
    if (config_.agc_enabled) {
        if (!InitializeAGC()) {
            spdlog::error("AudioProcessor: Failed to initialize AGC");
            Shutdown();
            return false;
        }
    }
    
    initialized_.store(true);
    spdlog::info("AudioProcessor: Initialized successfully");
    return true;
}

// ============================================================================
// Shutdown
// ============================================================================
void AudioProcessor::Shutdown() {
    if (!initialized_.load()) {
        return;
    }
    
    spdlog::info("AudioProcessor: Shutting down");
    
    ShutdownFilter();
    ShutdownNS();
    ShutdownAGC();
    
    initialized_.store(false);
}

// ============================================================================
// Reconfigure
// ============================================================================
bool AudioProcessor::Reconfigure(const AudioProcessorConfig& config) {
    spdlog::info("AudioProcessor: Reconfiguring...");
    Shutdown();
    return Initialize(config);
}

// ============================================================================
// Process - Main processing function
// ============================================================================
bool AudioProcessor::Process(int16_t* samples, size_t sample_count) {
    if (!initialized_.load() || !IsProcessingEnabled()) {
        return true;  // Nothing to do
    }
    
    if (samples == nullptr || sample_count == 0) {
        return false;
    }
    
    // Processing order: HPF → Notch → NS → AGC
    
    // 1. Apply filters (HPF and Notch)
    if (filter_initialized_ && (config_.hpf_enabled || config_.notch_enabled)) {
        ProcessFilter(samples, sample_count);
    }
    
    // 2. Apply Noise Suppression
    if (ns_initialized_ && config_.ns_enabled) {
        ProcessNS(samples, sample_count);
    }
    
    // 3. Apply AGC
    if (agc_initialized_ && config_.agc_enabled) {
        ProcessAGC(samples, sample_count);
    }
    
    frames_processed_++;
    return true;
}

// ============================================================================
// Initialize Filter (HPF and Notch)
// ============================================================================
bool AudioProcessor::InitializeFilter() {
#ifdef HAVE_LIBAUDFILT
    spdlog::info("AudioProcessor: Initializing audio filters...");
    
    AUDFILT_INIT filt_init = {};
    filt_init.filt_ch = AUDFILT_CH_MONO;
    filt_init.smooth_enable = TRUE;
    
    if (!audlib_filt_open(&filt_init)) {
        spdlog::error("AudioProcessor: audlib_filt_open() failed");
        return false;
    }
    
    // Configure HPF if enabled
    if (config_.hpf_enabled) {
        AUDFILT_EQPARAM eq_param = {};
        AUDFILT_CONFIG filt_config = {};
        
        eq_param.filt_type = AUDFILT_DESIGNTYPE_HIGHPASS;
        eq_param.sample_rate = config_.sample_rate;
        eq_param.frequency = static_cast<float>(config_.hpf_freq);
        eq_param.Q = config_.hpf_q;
        eq_param.gain_db = 0.0f;  // HPF doesn't use gain
        
        if (!audlib_filt_design_filt(&eq_param, &filt_config, AUDFILT_DESGIN_CTRL_DEFAULT)) {
            spdlog::error("AudioProcessor: Failed to design HPF");
            audlib_filt_close();
            return false;
        }
        
        audlib_filt_set_config(AUDFILT_SEL_FILT0, &filt_config);
        audlib_filt_enable_filt(AUDFILT_SEL_FILT0, TRUE);
        
        spdlog::info("AudioProcessor: HPF configured: {}Hz Q={:.1f}", 
                     config_.hpf_freq, config_.hpf_q);
    }
    
    // Configure Notch if enabled
    if (config_.notch_enabled) {
        AUDFILT_EQPARAM eq_param = {};
        AUDFILT_CONFIG filt_config = {};
        
        eq_param.filt_type = AUDFILT_DESIGNTYPE_NOTCH;
        eq_param.sample_rate = config_.sample_rate;
        eq_param.frequency = static_cast<float>(config_.notch_freq);
        eq_param.Q = config_.notch_q;
        eq_param.gain_db = static_cast<float>(config_.notch_gain);
        
        if (!audlib_filt_design_filt(&eq_param, &filt_config, AUDFILT_DESGIN_CTRL_DEFAULT)) {
            spdlog::error("AudioProcessor: Failed to design Notch filter");
            audlib_filt_close();
            return false;
        }
        
        // Use FILT1 for notch (FILT0 is HPF)
        audlib_filt_set_config(AUDFILT_SEL_FILT1, &filt_config);
        audlib_filt_enable_filt(AUDFILT_SEL_FILT1, TRUE);
        
        spdlog::info("AudioProcessor: Notch configured: {}Hz Q={:.1f} gain={}dB",
                     config_.notch_freq, config_.notch_q, config_.notch_gain);
    }
    
    if (!audlib_filt_init()) {
        spdlog::error("AudioProcessor: audlib_filt_init() failed");
        audlib_filt_close();
        return false;
    }
    
    filter_initialized_ = true;
    return true;
#else
    spdlog::warn("AudioProcessor: libaudfilt not available, filters disabled");
    return false;
#endif
}

// ============================================================================
// Initialize NS (Noise Suppression)
// ============================================================================
bool AudioProcessor::InitializeNS() {
#ifdef HAVE_LIBAEC
    spdlog::info("AudioProcessor: Initializing Noise Suppression...");
    
    ST_AUD_NS_INFO ns_info = {};
    ST_AUD_NS_RTN ns_rtn = {};
    
    ns_info.s32SamplingRate = config_.sample_rate;
    ns_info.s32ChannelNum = config_.channels;
    ns_info.s32FrameSize = (config_.sample_rate == 8000) ? 256 : 512;
    
    AUD_NS_PreInit(&ns_info, &ns_rtn);
    
    spdlog::info("AudioProcessor: NS PreInit: FrameSize={}, InternalBuf={}",
                 ns_info.s32FrameSize, ns_rtn.u32InternalBufSize);
    
    ns_frame_size_ = static_cast<uint32_t>(ns_info.s32FrameSize);
    ns_internal_buf_size_ = ns_rtn.u32InternalBufSize;
    
    // Allocate internal buffer
    ns_internal_buf_ = malloc(ns_internal_buf_size_);
    if (!ns_internal_buf_) {
        spdlog::error("AudioProcessor: Failed to allocate NS internal buffer");
        return false;
    }
    
    int ns_ret = AUD_NS_Init(ns_internal_buf_, ns_internal_buf_size_);
    if (ns_ret != 0) {
        spdlog::error("AudioProcessor: AUD_NS_Init failed: {}", ns_ret);
        free(ns_internal_buf_);
        ns_internal_buf_ = nullptr;
        return false;
    }
    
    // Set noise suppression level (vendor API: pass int* with dB value)
    int32_t ns_suppress_level = config_.ns_level;  // e.g. -20 dB
    AUD_NS_SetParam(EN_AUD_NS_NOISE_SUPPRESS, &ns_suppress_level);
    
    // Enable Wiener denoiser for better quality (like vendor sample)
    int32_t denoise_enable = 1;
    AUD_NS_SetParam(EN_AUD_NS_DENOISE, &denoise_enable);
    
    // Allocate output buffer
    ns_output_buf_.resize(ns_frame_size_);
    
    ns_initialized_ = true;
    spdlog::info("AudioProcessor: NS initialized: level={}dB, frame={}",
                 config_.ns_level, ns_frame_size_);
    return true;
#else
    spdlog::warn("AudioProcessor: libaec not available, NS disabled");
    return false;
#endif
}

// ============================================================================
// Initialize AGC
// ============================================================================
bool AudioProcessor::InitializeAGC() {
#ifdef HAVE_LIBAGC
    spdlog::info("AudioProcessor: Initializing AGC...");
    
    if (audlib_agc_open() != 0) {
        spdlog::error("AudioProcessor: audlib_agc_open() failed");
        return false;
    }
    
    // Configure AGC
    audlib_agc_set_config(AGC_CONFIG_ID_SAMPLERATE, config_.sample_rate);
    audlib_agc_set_config(AGC_CONFIG_ID_CHANNEL_NO, config_.channels);
    audlib_agc_set_config(AGC_CONFIG_ID_TARGET_LVL, AGC_DB(config_.agc_target_db));
    audlib_agc_set_config(AGC_CONFIG_ID_MAXGAIN, AGC_DB(config_.agc_max_gain));
    audlib_agc_set_config(AGC_CONFIG_ID_MINGAIN, AGC_DB(config_.agc_min_gain));
    audlib_agc_set_config(AGC_CONFIG_ID_ATTACK_TIME, AGC_TRESO_BASIS_100MS);
    audlib_agc_set_config(AGC_CONFIG_ID_DECAY_TIME, AGC_TRESO_BASIS_200MS);
    audlib_agc_set_config(AGC_CONFIG_ID_NG_THD, AGC_DB(config_.agc_noise_gate_db));
    
    if (!audlib_agc_init()) {
        spdlog::error("AudioProcessor: audlib_agc_init() failed");
        audlib_agc_close();
        return false;
    }
    
    // Allocate output buffer
    agc_output_buf_.resize(4096);
    
    agc_initialized_ = true;
    spdlog::info("AudioProcessor: AGC initialized: target={}dB, gain=[{},{}]dB, ng={}dB",
                 config_.agc_target_db, config_.agc_min_gain, config_.agc_max_gain,
                 config_.agc_noise_gate_db);
    return true;
#else
    spdlog::warn("AudioProcessor: libagc not available, AGC disabled");
    return false;
#endif
}

// ============================================================================
// Process Filter (HPF and Notch)
// ============================================================================
void AudioProcessor::ProcessFilter(int16_t* samples, size_t sample_count) {
#ifdef HAVE_LIBAUDFILT
    // CRITICAL: LP64 API takes BYTES, not samples!
    size_t byte_count = sample_count * sizeof(int16_t);
    
    if (!audlib_filt_run(reinterpret_cast<long>(samples), static_cast<int>(byte_count))) {
        errors_++;
        return;
    }
    
    filter_frames_++;
#endif
}

// ============================================================================
// Process NS (Noise Suppression)
// ============================================================================
void AudioProcessor::ProcessNS(int16_t* samples, size_t sample_count) {
#ifdef HAVE_LIBAEC
    // Process in ns_frame_size_ chunks (256 samples at 8kHz)
    size_t offset = 0;
    while (offset + ns_frame_size_ <= sample_count) {
        AUD_NS_Run(&samples[offset], ns_output_buf_.data());
        
        // Copy processed data back (in-place)
        memcpy(&samples[offset], ns_output_buf_.data(), ns_frame_size_ * sizeof(int16_t));
        
        offset += ns_frame_size_;
    }
    
    // Handle remaining samples (partial frame) - just leave unprocessed
    // This is fine for streaming since frames are typically aligned
    
    ns_frames_++;
#endif
}

// ============================================================================
// Process AGC
// ============================================================================
void AudioProcessor::ProcessAGC(int16_t* samples, size_t sample_count) {
#ifdef HAVE_LIBAGC
    if (sample_count > agc_output_buf_.size()) {
        agc_output_buf_.resize(sample_count);
    }
    
    AGC_BITSTREAM agc_io = {};
    agc_io.bitstram_buffer_in = reinterpret_cast<uint64_t>(samples);
    agc_io.bitstram_buffer_out = reinterpret_cast<uint64_t>(agc_output_buf_.data());
    agc_io.bitstram_buffer_length = sample_count;
    
    if (!audlib_agc_run(&agc_io)) {
        errors_++;
        return;
    }
    
    // Copy processed data back (in-place)
    memcpy(samples, agc_output_buf_.data(), sample_count * sizeof(int16_t));
    
    agc_frames_++;
#endif
}

// ============================================================================
// Shutdown helpers
// ============================================================================
void AudioProcessor::ShutdownFilter() {
#ifdef HAVE_LIBAUDFILT
    if (filter_initialized_) {
        audlib_filt_close();
        filter_initialized_ = false;
        spdlog::debug("AudioProcessor: Filter shutdown");
    }
#endif
}

void AudioProcessor::ShutdownNS() {
#ifdef HAVE_LIBAEC
    if (ns_initialized_) {
        // NOTE: AUD_NS_Uninit() declared in vendor header but not in prebuilt libaec.so
        // Just free the internal buffer
        if (ns_internal_buf_) {
            free(ns_internal_buf_);
            ns_internal_buf_ = nullptr;
        }
        ns_output_buf_.clear();
        ns_initialized_ = false;
        spdlog::debug("AudioProcessor: NS shutdown");
    }
#endif
}

void AudioProcessor::ShutdownAGC() {
#ifdef HAVE_LIBAGC
    if (agc_initialized_) {
        audlib_agc_close();
        agc_output_buf_.clear();
        agc_initialized_ = false;
        spdlog::debug("AudioProcessor: AGC shutdown");
    }
#endif
}

// ============================================================================
// Statistics
// ============================================================================
AudioProcessorStats AudioProcessor::GetStats() const {
    AudioProcessorStats stats;
    stats.frames_processed = frames_processed_.load();
    stats.ns_frames = ns_frames_.load();
    stats.agc_frames = agc_frames_.load();
    stats.filter_frames = filter_frames_.load();
    stats.errors = errors_.load();
    return stats;
}

} // namespace streaming
} // namespace ipcam
