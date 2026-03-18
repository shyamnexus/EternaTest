/**
 * @file audio_frame_broadcaster.cpp
 * @brief Implementation of AudioFrameBroadcaster singleton
 * 
 * Singleton producer that pulls audio from HDAL, encodes once,
 * and broadcasts to all registered consumers via lock-free ring buffer.
 */

#include "ipcam/audio_frame_broadcaster.h"
#include "ipcam/hdal_pipeline.h"
#include "ipcam/audio_control.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>

namespace ipcam {
namespace streaming {

// ============================================================================
// AudioFrameConsumer Implementation
// ============================================================================

AudioFrameConsumer::AudioFrameConsumer(AudioFrameBroadcaster* broadcaster, uint32_t consumer_id,
                                       const AudioRingBuffer* ring_buffer)
    : broadcaster_(broadcaster)
    , ring_buffer_(ring_buffer)
    , consumer_id_(consumer_id)
    , next_sequence_(0)
    , active_(true)
    , is_pcm_consumer_(false) {
    // If no explicit ring buffer, use the default encoded ring buffer
    if (!ring_buffer_ && broadcaster_) {
        ring_buffer_ = &broadcaster_->GetRingBuffer();
    }
    // Start from the latest frame to avoid initial backlog
    if (ring_buffer_) {
        next_sequence_ = ring_buffer_->GetLatestSequence();
        if (next_sequence_ > 0) {
            next_sequence_++;  // Want the next new frame
        } else {
            next_sequence_ = 1;
        }
    }
    spdlog::debug("AudioFrameConsumer: Created consumer {} starting at seq {} (pcm={})", 
                  consumer_id_, next_sequence_, is_pcm_consumer_);
}

AudioFrameConsumer::~AudioFrameConsumer() {
    if (broadcaster_ && active_.load()) {
        broadcaster_->UnregisterConsumer(consumer_id_);
    }
}

AudioFrameConsumer::AudioFrameConsumer(AudioFrameConsumer&& other) noexcept
    : broadcaster_(other.broadcaster_)
    , ring_buffer_(other.ring_buffer_)
    , consumer_id_(other.consumer_id_)
    , next_sequence_(other.next_sequence_)
    , active_(other.active_.load())
    , is_pcm_consumer_(other.is_pcm_consumer_)
    , total_frames_skipped_(other.total_frames_skipped_)
    , last_skip_log_time_(other.last_skip_log_time_) {
    other.broadcaster_ = nullptr;
    other.ring_buffer_ = nullptr;
    other.active_.store(false);
}

AudioFrameConsumer& AudioFrameConsumer::operator=(AudioFrameConsumer&& other) noexcept {
    if (this != &other) {
        if (broadcaster_ && active_.load()) {
            broadcaster_->UnregisterConsumer(consumer_id_);
        }
        broadcaster_ = other.broadcaster_;
        ring_buffer_ = other.ring_buffer_;
        consumer_id_ = other.consumer_id_;
        next_sequence_ = other.next_sequence_;
        active_.store(other.active_.load());
        is_pcm_consumer_ = other.is_pcm_consumer_;
        total_frames_skipped_ = other.total_frames_skipped_;
        last_skip_log_time_ = other.last_skip_log_time_;
        other.broadcaster_ = nullptr;
        other.ring_buffer_ = nullptr;
        other.active_.store(false);
    }
    return *this;
}

bool AudioFrameConsumer::GetNextFrame(AudioFrame& out_frame, int wait_ms) {
    if (!broadcaster_ || !ring_buffer_ || !active_.load()) {
        return false;
    }
    
    const auto& ring = *ring_buffer_;
    
    // Check if our sequence is still valid (not overwritten)
    uint64_t oldest = ring.GetOldestSequence();
    if (next_sequence_ < oldest) {
        // We fell behind, skip to oldest available
        uint64_t skipped = oldest - next_sequence_;
        total_frames_skipped_ += skipped;
        auto now_tp = std::chrono::steady_clock::now();
        if (now_tp - last_skip_log_time_ >= std::chrono::seconds(1)) {
            spdlog::debug("AudioFrameConsumer {}: Fell behind, skipped {} frames "
                          "(total {})", consumer_id_, skipped, total_frames_skipped_);
            last_skip_log_time_ = now_tp;
        }
        next_sequence_ = oldest;
    }
    
    // Try to read the next frame
    if (ring.Read(next_sequence_, out_frame)) {
        next_sequence_++;
        return true;
    }
    
    // Frame not ready yet, wait if requested
    if (wait_ms > 0) {
        bool waited = is_pcm_consumer_ ? 
            broadcaster_->WaitForPcmFrame(wait_ms) :
            broadcaster_->WaitForFrame(wait_ms);
        if (waited) {
            // Try again after notification
            if (ring.Read(next_sequence_, out_frame)) {
                next_sequence_++;
                return true;
            }
        }
    }
    
    return false;
}

size_t AudioFrameConsumer::GetPendingFrameCount() const {
    if (!ring_buffer_) return 0;
    
    uint64_t latest = ring_buffer_->GetLatestSequence();
    if (next_sequence_ > latest) return 0;
    return static_cast<size_t>(latest - next_sequence_ + 1);
}

void AudioFrameConsumer::ResetToLatest() {
    if (!ring_buffer_) return;
    
    uint64_t latest = ring_buffer_->GetLatestSequence();
    next_sequence_ = (latest > 0) ? latest + 1 : 1;
    spdlog::debug("AudioFrameConsumer {}: Reset to seq {}", consumer_id_, next_sequence_);
}

// ============================================================================
// AudioFrameBroadcaster Implementation
// ============================================================================

AudioFrameBroadcaster& AudioFrameBroadcaster::Instance() {
    static AudioFrameBroadcaster instance;
    return instance;
}

AudioFrameBroadcaster::AudioFrameBroadcaster()
    : codec_(RtspAudioCodec::kPcmu)
    , sample_rate_(8000)
    , channels_(1)
    , enc_path_(0)
    , vir_addr_(0)
    , buffer_mapped_(false)
    , g711_type_(platform::G711Type::kNone)
    , initialized_(false)
    , running_(false)
    , stop_requested_(false)
    , next_consumer_id_(0)
    , frames_produced_(0)
    , frames_dropped_(0)
    , encode_errors_(0)
    , hdal_pull_errors_(0) {
    
    memset(&phy_buf_, 0, sizeof(phy_buf_));
    
    // Pre-allocate buffers
    resample_buffer_.resize(4096);
    encode_buffer_.resize(kMaxAudioFrameSize);
}

AudioFrameBroadcaster::~AudioFrameBroadcaster() {
    Stop();
    CleanupHdalBuffer();
}

bool AudioFrameBroadcaster::Initialize(RtspAudioCodec codec, int sample_rate, int channels) {
    if (initialized_.load()) {
        spdlog::warn("AudioFrameBroadcaster: Already initialized");
        return true;
    }
    
    codec_ = codec;
    sample_rate_ = sample_rate;
    channels_ = channels;
    
    // Setup encoder based on codec
    g711_type_ = platform::G711Type::kNone;
    g726_encoder_.reset();
    aac_encoder_.reset();
    
    if (codec == RtspAudioCodec::kPcmu) {
        g711_type_ = platform::G711Type::kUlaw;
    } else if (codec == RtspAudioCodec::kPcma) {
        g711_type_ = platform::G711Type::kAlaw;
    } else if (codec == RtspAudioCodec::kG726_16) {
        g726_encoder_ = std::make_unique<platform::G726Codec>(platform::G726BitRate::k16kbps);
    } else if (codec == RtspAudioCodec::kG726_24) {
        g726_encoder_ = std::make_unique<platform::G726Codec>(platform::G726BitRate::k24kbps);
    } else if (codec == RtspAudioCodec::kG726_32) {
        g726_encoder_ = std::make_unique<platform::G726Codec>(platform::G726BitRate::k32kbps);
    } else if (codec == RtspAudioCodec::kG726_40) {
        g726_encoder_ = std::make_unique<platform::G726Codec>(platform::G726BitRate::k40kbps);
    } else if (codec == RtspAudioCodec::kAac) {
        aac_encoder_ = std::make_unique<platform::AacCodec>();
        int aac_bitrate = (sample_rate >= 44100) ? 128000 : 64000;
        if (!aac_encoder_->Init(sample_rate, channels, aac_bitrate)) {
            spdlog::error("AudioFrameBroadcaster: Failed to initialize AAC encoder");
            aac_encoder_.reset();
            return false;
        }
    }
    
    // Setup HDAL buffer
    if (!SetupHdalBuffer()) {
        spdlog::error("AudioFrameBroadcaster: Failed to setup HDAL buffer");
        return false;
    }
    
    const char* codec_name = "Unknown";
    switch (codec) {
        case RtspAudioCodec::kPcmu: codec_name = "PCMU (G.711 μ-law)"; break;
        case RtspAudioCodec::kPcma: codec_name = "PCMA (G.711 A-law)"; break;
        case RtspAudioCodec::kG726_16: codec_name = "G.726-16"; break;
        case RtspAudioCodec::kG726_24: codec_name = "G.726-24"; break;
        case RtspAudioCodec::kG726_32: codec_name = "G.726-32"; break;
        case RtspAudioCodec::kG726_40: codec_name = "G.726-40"; break;
        case RtspAudioCodec::kAac: codec_name = "AAC-LC"; break;
        case RtspAudioCodec::kPcm: codec_name = "L16 (PCM)"; break;
        default: break;
    }
    
    spdlog::info("AudioFrameBroadcaster: Initialized codec={} rate={} ch={}",
                 codec_name, sample_rate, channels);
    
    // Initialize audio processor (NS, AGC, HPF, Notch) from AudioControl config
    InitializeAudioProcessor();
    
    initialized_.store(true, std::memory_order_release);
    return true;
}

bool AudioFrameBroadcaster::Reconfigure(RtspAudioCodec codec, int sample_rate, int channels) {
    spdlog::info("AudioFrameBroadcaster: Reconfiguring codec from {} to {} rate={} ch={}",
                 static_cast<int>(codec_), static_cast<int>(codec), sample_rate, channels);
    
    bool was_running = running_.load(std::memory_order_acquire);
    
    // Stop if running
    if (was_running) {
        Stop();
    }
    
    // Reset initialized flag to allow re-initialization
    initialized_.store(false, std::memory_order_release);
    
    // Cleanup old encoder resources
    g711_type_ = platform::G711Type::kNone;
    g726_encoder_.reset();
    aac_encoder_.reset();
    
    // Re-initialize with new settings
    if (!Initialize(codec, sample_rate, channels)) {
        spdlog::error("AudioFrameBroadcaster: Failed to reinitialize with new settings");
        return false;
    }
    
    // Restart if it was running before
    if (was_running) {
        if (!Start()) {
            spdlog::error("AudioFrameBroadcaster: Failed to restart after reconfigure");
            return false;
        }
    }
    
    spdlog::info("AudioFrameBroadcaster: Reconfigured successfully");
    return true;
}

void AudioFrameBroadcaster::ResetForPipelineReinit() {
    spdlog::info("AudioFrameBroadcaster: Resetting for pipeline reinit");
    Stop();
    CleanupHdalBuffer();
    initialized_.store(false, std::memory_order_release);
}

bool AudioFrameBroadcaster::SetupHdalBuffer() {
    auto& pipeline = platform::HdalPipeline::Instance();
    if (!pipeline.IsRunning() || !pipeline.IsAudioEnabled()) {
        spdlog::error("AudioFrameBroadcaster: HDAL pipeline not running or audio disabled");
        return false;
    }
    
    enc_path_ = static_cast<HD_PATH_ID>(pipeline.GetAudioEncoderPath());
    if (enc_path_ == 0) {
        spdlog::error("AudioFrameBroadcaster: Invalid audio encoder path");
        return false;
    }
    
    spdlog::info("AudioFrameBroadcaster: Using enc_path=0x{:x}", enc_path_);
    
    if (hd_audioenc_get(enc_path_, HD_AUDIOENC_PARAM_BUFINFO, &phy_buf_) != HD_OK) {
        spdlog::error("AudioFrameBroadcaster: Failed to get audio encoder buffer info");
        return false;
    }
    
    vir_addr_ = (UINTPTR)hd_common_mem_mmap(
        HD_COMMON_MEM_MEM_TYPE_CACHE,
        phy_buf_.buf_info.phy_addr,
        phy_buf_.buf_info.buf_size);
    
    if (vir_addr_ == 0) {
        spdlog::error("AudioFrameBroadcaster: Failed to mmap audio encoder buffer");
        return false;
    }
    
    buffer_mapped_ = true;
    spdlog::info("AudioFrameBroadcaster: Buffer mapped at 0x{:x}, size={}",
                 vir_addr_, phy_buf_.buf_info.buf_size);
    
    return true;
}

void AudioFrameBroadcaster::CleanupHdalBuffer() {
    if (buffer_mapped_ && vir_addr_ != 0) {
        hd_common_mem_munmap((void*)vir_addr_, phy_buf_.buf_info.buf_size);
        buffer_mapped_ = false;
        vir_addr_ = 0;
    }
}

void AudioFrameBroadcaster::InitializeAudioProcessor() {
    // Get audio processing config from AudioControl
    auto& audio_ctrl = media::AudioControl::Instance();
    auto cfg = audio_ctrl.GetConfig();
    
    // Build processor config
    AudioProcessorConfig proc_cfg;
    proc_cfg.sample_rate = sample_rate_;
    proc_cfg.channels = channels_;
    
    // HPF
    proc_cfg.hpf_enabled = cfg.hpf_enabled;
    proc_cfg.hpf_freq = cfg.hpf_freq;
    proc_cfg.hpf_q = cfg.hpf_q;
    
    // Notch
    proc_cfg.notch_enabled = cfg.notch_enabled;
    proc_cfg.notch_freq = cfg.notch_freq;
    proc_cfg.notch_q = cfg.notch_q;
    proc_cfg.notch_gain = cfg.notch_gain;
    
    // NS (only at 8kHz)
    if (cfg.ns_enabled && sample_rate_ == 8000) {
        proc_cfg.ns_enabled = true;
        proc_cfg.ns_level = cfg.ns_level;
    } else if (cfg.ns_enabled && sample_rate_ != 8000) {
        spdlog::warn("AudioFrameBroadcaster: NS disabled - requires 8kHz (current: {}Hz)", sample_rate_);
        proc_cfg.ns_enabled = false;
    }
    
    // AGC (software automatic gain control via libagc)
    proc_cfg.agc_enabled = cfg.agc_enabled;
    proc_cfg.agc_target_db = cfg.agc_target_db;
    proc_cfg.agc_noise_gate_db = cfg.agc_noise_gate_db;
    
    // Check if any processing is enabled
    if (!proc_cfg.hpf_enabled && !proc_cfg.notch_enabled && 
        !proc_cfg.ns_enabled && !proc_cfg.agc_enabled) {
        spdlog::info("AudioFrameBroadcaster: No audio processing enabled");
        audio_processor_.reset();
        return;
    }
    
    // Create and initialize processor
    audio_processor_ = std::make_unique<AudioProcessor>();
    if (!audio_processor_->Initialize(proc_cfg)) {
        spdlog::error("AudioFrameBroadcaster: Failed to initialize audio processor");
        audio_processor_.reset();
        return;
    }
    
    spdlog::info("AudioFrameBroadcaster: Audio processor initialized (HPF={}, Notch={}, NS={}, AGC={})",
                 proc_cfg.hpf_enabled, proc_cfg.notch_enabled, 
                 proc_cfg.ns_enabled, proc_cfg.agc_enabled);
}

void AudioFrameBroadcaster::ReconfigureAudioProcessor() {
    spdlog::info("AudioFrameBroadcaster: Reconfiguring audio processor...");
    
    // Shutdown existing processor
    if (audio_processor_) {
        audio_processor_->Shutdown();
        audio_processor_.reset();
    }
    
    // Re-initialize with current config
    InitializeAudioProcessor();
}

bool AudioFrameBroadcaster::Start() {
    if (!initialized_.load()) {
        spdlog::error("AudioFrameBroadcaster: Cannot start - not initialized");
        return false;
    }
    
    if (running_.load()) {
        spdlog::warn("AudioFrameBroadcaster: Already running");
        return true;
    }
    
    stop_requested_.store(false);
    producer_thread_ = std::thread(&AudioFrameBroadcaster::ProducerThread, this);
    
    // Wait for thread to start
    for (int i = 0; i < 100 && !running_.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    spdlog::info("AudioFrameBroadcaster: Started producer thread");
    return running_.load();
}

void AudioFrameBroadcaster::Stop() {
    if (!running_.load()) {
        return;
    }
    
    spdlog::info("AudioFrameBroadcaster: Stopping producer thread...");
    
    stop_requested_.store(true, std::memory_order_release);
    
    // Wake up any waiting consumers
    frame_cv_.notify_all();
    pcm_frame_cv_.notify_all();
    
    if (producer_thread_.joinable()) {
        producer_thread_.join();
    }
    
    // Flush any remaining buffers in HDAL to avoid "release all pull_out bitstream" error
    FlushHdalAudioBuffers("Stop");
    
    spdlog::info("AudioFrameBroadcaster: Producer thread stopped");
}

void AudioFrameBroadcaster::FlushHdalAudioBuffers(const char* context) {
    if (!buffer_mapped_ || enc_path_ == 0) {
        return;
    }
    
    spdlog::debug("AudioFrameBroadcaster: Flushing audio buffers ({})...", context);
    
    HD_AUDIO_BS audio_data;
    int flush_count = 0;
    constexpr int kMaxFlush = 50;            // Reduced from 200 to prevent driver stall
    constexpr int kFlushTimeoutMs = 1;       // Reduced timeout
    int consecutive_empty = 0;
    
    while (flush_count < kMaxFlush && consecutive_empty < 3) {
        memset(&audio_data, 0, sizeof(audio_data));
        HD_RESULT ret = hd_audioenc_pull_out_buf(enc_path_, &audio_data, kFlushTimeoutMs);
        
        if (ret != HD_OK || audio_data.size == 0) {
            consecutive_empty++;
            usleep(10000); // Increased wait to 10ms
            continue;
        }
        
        consecutive_empty = 0;
        hd_audioenc_release_out_buf(enc_path_, &audio_data);
        flush_count++;
        
        // Increased yield to verify stability
        usleep(5000); // 5ms yield
    }
    
    if (flush_count > 0) {
        spdlog::debug("AudioFrameBroadcaster: Flushed {} audio buffers ({})", flush_count, context);
    }
}

void AudioFrameBroadcaster::ProducerThread() {
    spdlog::info("AudioFrameBroadcaster: Producer thread started");
    
    // TEMPORARY: Disable flush on start to prevent crash due to driver overload/SIE interaction
    // FlushHdalAudioBuffers("ProducerStart");
    
    running_.store(true, std::memory_order_release);
    
    while (!stop_requested_.load(std::memory_order_acquire)) {
        auto& pipeline = platform::HdalPipeline::Instance();
        if (!pipeline.IsRunning() || !pipeline.IsAudioEnabled()) {
            if (buffer_mapped_) {
                CleanupHdalBuffer();
            }
            enc_path_ = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (!buffer_mapped_ || enc_path_ == 0) {
            if (!SetupHdalBuffer()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
        }

        // Check if we have any consumers (encoded or PCM)
        size_t consumer_count;
        size_t pcm_consumer_count;
        {
            std::lock_guard<std::mutex> lock(consumer_mutex_);
            consumer_count = active_consumers_.size();
        }
        {
            std::lock_guard<std::mutex> lock(pcm_consumer_mutex_);
            pcm_consumer_count = active_pcm_consumers_.size();
        }
        
        if (consumer_count == 0 && pcm_consumer_count == 0) {
            // No consumers - flush accumulated frames to prevent buildup
            // Flush every ~1 second by using a simple counter
            static thread_local int no_consumer_cycles = 0;
            no_consumer_cycles++;
            
            // Every ~1 second (20 cycles at 50ms each), flush accumulated audio
            if (no_consumer_cycles >= 20) {
                FlushHdalAudioBuffers("NoConsumers");
                no_consumer_cycles = 0;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        
        if (!buffer_mapped_ || enc_path_ == 0) {
            spdlog::error("AudioFrameBroadcaster: Buffer not mapped");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        // Pull audio frame from HDAL
        HD_AUDIO_BS audio_data;
        memset(&audio_data, 0, sizeof(audio_data));
        
        HD_RESULT ret = hd_audioenc_pull_out_buf(enc_path_, &audio_data, kAudioPullTimeoutMs);
        
        if (ret != HD_OK || audio_data.size == 0) {
            hdal_pull_errors_++;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        
        // Get virtual address of audio data
        #define AUDIO_PHY2VIRT(pa) (vir_addr_ + (pa - phy_buf_.buf_info.phy_addr))
        uint8_t* audio_ptr = (uint8_t*)AUDIO_PHY2VIRT(audio_data.phy_addr);
        #undef AUDIO_PHY2VIRT
        
        // Encode the frame
        AudioFrame frame;
        if (EncodeFrame(audio_ptr, audio_data.size, audio_data.timestamp, frame)) {
            // Write to ring buffer
            ring_buffer_.Write(frame);
            frames_produced_++;
            
            // Notify waiting consumers
            {
                std::lock_guard<std::mutex> lock(frame_mutex_);
            }
            frame_cv_.notify_all();
        } else {
            encode_errors_++;
        }
        
        // Release HDAL buffer
        hd_audioenc_release_out_buf(enc_path_, &audio_data);
    }
    
    running_.store(false, std::memory_order_release);
    spdlog::info("AudioFrameBroadcaster: Producer thread exiting");
}

bool AudioFrameBroadcaster::EncodeFrame(const uint8_t* pcm_data, size_t pcm_size,
                                        uint64_t timestamp, AudioFrame& out_frame) {
    // Input is 48kHz stereo 16-bit PCM
    const int16_t* pcm_stereo = reinterpret_cast<const int16_t*>(pcm_data);
    size_t stereo_samples = pcm_size / sizeof(int16_t);
    size_t frame_samples = stereo_samples / 2;  // Stereo to frames
    
    // Resample 48kHz stereo to target rate mono for G.711/G.726
    const int INPUT_RATE = 48000;
    const int DECIMATION = INPUT_RATE / sample_rate_;  // e.g., 48000/8000 = 6
    size_t mono_samples = frame_samples / DECIMATION;
    
    if (mono_samples > resample_buffer_.size()) {
        mono_samples = resample_buffer_.size();
    }
    
    // Mix stereo to mono and decimate with low-pass anti-aliasing filter.
    // Simple averaging over the decimation window acts as a box filter
    // (sinc in frequency domain) which attenuates frequencies above Nyquist.
    // This prevents aliasing artifacts that destroy G.726 speech quality.
    for (size_t i = 0; i < mono_samples; i++) {
        int32_t sum = 0;
        int count = 0;
        for (int j = 0; j < DECIMATION; j++) {
            size_t src_idx = (i * DECIMATION + j) * 2;
            if (src_idx + 1 < stereo_samples) {
                int32_t left = pcm_stereo[src_idx];
                int32_t right = pcm_stereo[src_idx + 1];
                sum += (left + right) / 2;
                count++;
            }
        }
        resample_buffer_[i] = static_cast<int16_t>(count > 0 ? sum / count : 0);
    }
    
    // Apply userspace audio processing (HPF → Notch → NS → AGC)
    if (audio_processor_ && audio_processor_->IsProcessingEnabled()) {
        audio_processor_->Process(resample_buffer_.data(), mono_samples);
    }
    
    // ================================================================
    // Write processed PCM to the PCM ring buffer (for recording)
    // This happens AFTER resampling + processing but BEFORE encoding
    // ================================================================
    {
        size_t pcm_bytes = mono_samples * sizeof(int16_t);
        if (pcm_bytes <= kMaxAudioFrameSize) {
            AudioFrame pcm_frame;
            memcpy(pcm_frame.data, resample_buffer_.data(), pcm_bytes);
            pcm_frame.size = static_cast<uint32_t>(pcm_bytes);
            pcm_frame.timestamp = timestamp;
            pcm_frame.codec = RtspAudioCodec::kPcm;
            pcm_frame.duration_us = static_cast<uint32_t>(
                (mono_samples * 1000000ULL) / sample_rate_);

            pcm_ring_buffer_.Write(pcm_frame);

            // Notify PCM consumers
            {
                std::lock_guard<std::mutex> lock(pcm_frame_mutex_);
            }
            pcm_frame_cv_.notify_all();
        }
    }
    
    // Encode based on codec
    uint8_t* output_data = nullptr;
    uint32_t output_size = 0;
    
    if (g711_type_ != platform::G711Type::kNone) {
        // G.711 encoding
        platform::G711Codec::EncodeBuffer(resample_buffer_.data(), mono_samples,
                                          encode_buffer_.data(), g711_type_);
        output_data = encode_buffer_.data();
        output_size = static_cast<uint32_t>(mono_samples);  // 1 byte per sample
    } else if (g726_encoder_) {
        // G.726 encoding
        size_t encoded = g726_encoder_->EncodeBuffer(resample_buffer_.data(), mono_samples,
                                                     encode_buffer_.data(),
                                                     encode_buffer_.size());
        output_data = encode_buffer_.data();
        output_size = static_cast<uint32_t>(encoded);
    } else if (aac_encoder_) {
        // AAC encoding (uses resampled mono PCM)
        std::vector<uint8_t> aac_output;
        int encoded = aac_encoder_->Encode(resample_buffer_.data(),
                                           static_cast<int>(mono_samples), aac_output);
        if (encoded > 0) {
            if (static_cast<size_t>(encoded) <= encode_buffer_.size()) {
                memcpy(encode_buffer_.data(), aac_output.data(), encoded);
                output_data = encode_buffer_.data();
                output_size = static_cast<uint32_t>(encoded);
            }
        }
    } else {
        // Raw PCM passthrough
        output_data = reinterpret_cast<uint8_t*>(resample_buffer_.data());
        output_size = static_cast<uint32_t>(mono_samples * sizeof(int16_t));
    }
    
    if (output_data == nullptr || output_size == 0) {
        return false;
    }
    
    // Fill output frame
    if (output_size > kMaxAudioFrameSize) {
        spdlog::warn("AudioFrameBroadcaster: Frame too large ({} > {}), truncating",
                     output_size, kMaxAudioFrameSize);
        output_size = kMaxAudioFrameSize;
    }
    
    memcpy(out_frame.data, output_data, output_size);
    out_frame.size = output_size;
    out_frame.timestamp = timestamp;
    out_frame.codec = codec_;
    
    // Calculate duration based on samples
    out_frame.duration_us = static_cast<uint32_t>((mono_samples * 1000000ULL) / sample_rate_);
    
    return true;
}

std::unique_ptr<AudioFrameConsumer> AudioFrameBroadcaster::RegisterConsumer() {
    std::lock_guard<std::mutex> lock(consumer_mutex_);
    
    if (active_consumers_.size() >= kMaxConsumers) {
        spdlog::error("AudioFrameBroadcaster: Maximum consumers ({}) reached", kMaxConsumers);
        return nullptr;
    }
    
    uint32_t id = ++next_consumer_id_;
    active_consumers_.push_back(id);
    
    spdlog::debug("AudioFrameBroadcaster: Registered consumer {} (total: {})",
                 id, active_consumers_.size());
    
    return std::make_unique<AudioFrameConsumer>(this, id);
}

void AudioFrameBroadcaster::UnregisterConsumer(uint32_t consumer_id) {
    std::lock_guard<std::mutex> lock(consumer_mutex_);
    
    auto it = std::find(active_consumers_.begin(), active_consumers_.end(), consumer_id);
    if (it != active_consumers_.end()) {
        active_consumers_.erase(it);
        spdlog::debug("AudioFrameBroadcaster: Unregistered consumer {} (remaining: {})",
                     consumer_id, active_consumers_.size());
    }
}

size_t AudioFrameBroadcaster::GetConsumerCount() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(consumer_mutex_));
    return active_consumers_.size();
}

bool AudioFrameBroadcaster::WaitForFrame(int timeout_ms) {
    std::unique_lock<std::mutex> lock(frame_mutex_);
    return frame_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                               [this]() { return stop_requested_.load(); }) == false;
}

AudioFrameBroadcaster::Stats AudioFrameBroadcaster::GetStats() const {
    Stats stats;
    stats.frames_produced = frames_produced_.load();
    stats.frames_dropped = frames_dropped_.load();
    stats.encode_errors = encode_errors_.load();
    stats.hdal_pull_errors = hdal_pull_errors_.load();
    stats.consumer_count = GetConsumerCount();
    return stats;
}

// ============================================================================
// PCM Consumer Management
// ============================================================================

std::unique_ptr<AudioFrameConsumer> AudioFrameBroadcaster::RegisterPcmConsumer() {
    std::lock_guard<std::mutex> lock(pcm_consumer_mutex_);

    if (active_pcm_consumers_.size() >= kMaxConsumers) {
        spdlog::error("AudioFrameBroadcaster: Max PCM consumers reached");
        return nullptr;
    }

    uint32_t id = ++next_consumer_id_;
    active_pcm_consumers_.push_back(id);

    // Create a consumer that reads from the PCM ring buffer
    auto consumer = std::make_unique<AudioFrameConsumer>(this, id, &pcm_ring_buffer_);
    consumer->is_pcm_consumer_ = true;

    spdlog::debug("AudioFrameBroadcaster: Registered PCM consumer {} (total: {})",
                  id, active_pcm_consumers_.size());
    return consumer;
}

void AudioFrameBroadcaster::UnregisterPcmConsumer(uint32_t consumer_id) {
    std::lock_guard<std::mutex> lock(pcm_consumer_mutex_);
    auto it = std::find(active_pcm_consumers_.begin(), active_pcm_consumers_.end(), consumer_id);
    if (it != active_pcm_consumers_.end()) {
        active_pcm_consumers_.erase(it);
        spdlog::debug("AudioFrameBroadcaster: Unregistered PCM consumer {} (remaining: {})",
                      consumer_id, active_pcm_consumers_.size());
    }
}

bool AudioFrameBroadcaster::WaitForPcmFrame(int timeout_ms) {
    std::unique_lock<std::mutex> lock(pcm_frame_mutex_);
    return pcm_frame_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                   [this]() { return stop_requested_.load(); }) == false;
}

} // namespace streaming
} // namespace ipcam
