/**
 * @file audio_frame_broadcaster.h
 * @brief Singleton Audio Frame Broadcaster with Producer-Consumer Pattern
 * 
 * Implements a singleton producer that:
 * - Pulls audio from HDAL encoder once
 * - Encodes to G.711/G.726/AAC once
 * - Broadcasts encoded frames to all registered consumers via lock-free ring buffer
 * 
 * Each consumer (RTSP client) receives a copy of every encoded frame,
 * avoiding redundant HDAL pulls and encoding.
 */

#ifndef IPCAM_AUDIO_FRAME_BROADCASTER_H
#define IPCAM_AUDIO_FRAME_BROADCASTER_H

#include "ipcam/rtsp_server.h"
#include "ipcam/g711_codec.h"
#include "ipcam/g726_codec.h"
#include "ipcam/aac_codec.h"
#include "ipcam/audio_processor.h"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

// HDAL headers
extern "C" {
#include "hdal.h"
#include "hd_type.h"
#include "hd_audiocapture.h"
}

namespace ipcam {
namespace streaming {

// ============================================================================
// Constants
// ============================================================================
static constexpr size_t kAudioRingBufferSlots = 256;      // Number of frames in ring buffer
static constexpr size_t kMaxAudioFrameSize = 8192;        // Max encoded frame size
static constexpr size_t kMaxConsumers = 32;               // Max concurrent consumers
static constexpr int kAudioPullTimeoutMs = 50;            // HDAL pull timeout

// ============================================================================
// AudioFrame - Single encoded audio frame
// ============================================================================

struct AudioFrame {
    uint8_t data[kMaxAudioFrameSize];    // Encoded audio data
    uint32_t size;                        // Actual data size
    uint64_t timestamp;                   // Hardware timestamp (microseconds)
    uint64_t sequence;                    // Monotonic sequence number
    uint32_t duration_us;                 // Frame duration in microseconds
    RtspAudioCodec codec;                 // Codec used for encoding
    
    AudioFrame() : size(0), timestamp(0), sequence(0), duration_us(0), 
                   codec(RtspAudioCodec::kNone) {
        memset(data, 0, sizeof(data));
    }
};

// ============================================================================
// AudioRingBuffer - Lock-free SPMC (Single Producer, Multi Consumer) ring buffer
// ============================================================================

class AudioRingBuffer {
public:
    AudioRingBuffer() : write_pos_(0), frames_written_(0) {
        for (auto& slot : slots_) {
            slot.sequence.store(0, std::memory_order_relaxed);
        }
    }
    
    /**
     * @brief Write a new frame to the ring buffer (producer only)
     * @param frame The frame to write
     * @return The sequence number assigned to this frame
     */
    uint64_t Write(const AudioFrame& frame) {
        uint64_t seq = ++frames_written_;
        size_t pos = write_pos_.fetch_add(1, std::memory_order_relaxed) % kAudioRingBufferSlots;
        
        Slot& slot = slots_[pos];
        
        // Mark as being written (odd sequence = in-progress)
        slot.sequence.store(seq * 2 - 1, std::memory_order_release);
        
        // Copy frame data
        slot.frame = frame;
        slot.frame.sequence = seq;
        
        // Mark as complete (even sequence = ready)
        slot.sequence.store(seq * 2, std::memory_order_release);
        
        return seq;
    }
    
    /**
     * @brief Read a frame by sequence number (consumer)
     * @param seq The sequence number to read
     * @param out_frame Output frame buffer
     * @return true if frame was successfully read, false if not available or overwritten
     */
    bool Read(uint64_t seq, AudioFrame& out_frame) const {
        if (seq == 0) return false;
        
        // Check if frame might still be in buffer
        uint64_t current = frames_written_.load(std::memory_order_acquire);
        if (seq + kAudioRingBufferSlots <= current) {
            // Frame has been overwritten
            return false;
        }
        
        // Find slot
        size_t pos = (seq - 1) % kAudioRingBufferSlots;
        const Slot& slot = slots_[pos];
        
        // Read sequence to check if ready
        uint64_t slot_seq = slot.sequence.load(std::memory_order_acquire);
        if (slot_seq != seq * 2) {
            // Either being written or different frame
            return false;
        }
        
        // Copy frame data
        out_frame = slot.frame;
        
        // Verify sequence hasn't changed during read
        if (slot.sequence.load(std::memory_order_acquire) != seq * 2) {
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief Get the latest available sequence number
     */
    uint64_t GetLatestSequence() const {
        return frames_written_.load(std::memory_order_acquire);
    }
    
    /**
     * @brief Get a safe starting sequence (oldest still in buffer)
     */
    uint64_t GetOldestSequence() const {
        uint64_t latest = frames_written_.load(std::memory_order_acquire);
        if (latest <= kAudioRingBufferSlots) {
            return 1;
        }
        return latest - kAudioRingBufferSlots + 1;
    }

private:
    struct Slot {
        std::atomic<uint64_t> sequence;  // Even = ready, odd = being written
        AudioFrame frame;
        
        // Padding to avoid false sharing (cache line is typically 64 bytes)
        char padding[64 - sizeof(std::atomic<uint64_t>)];
    };
    
    alignas(64) Slot slots_[kAudioRingBufferSlots];
    alignas(64) std::atomic<size_t> write_pos_;
    alignas(64) std::atomic<uint64_t> frames_written_;
};

// ============================================================================
// AudioFrameConsumer - Handle for consumers to read frames
// ============================================================================

class AudioFrameBroadcaster;  // Forward declaration

class AudioFrameConsumer {
public:
    using FrameCallback = std::function<void(const AudioFrame&)>;
    
    /**
     * @brief Construct a consumer that reads from a specific ring buffer
     * @param broadcaster Parent broadcaster (for wait notifications)
     * @param consumer_id Unique consumer ID
     * @param ring_buffer Ring buffer to read from (encoded or PCM)
     */
    AudioFrameConsumer(AudioFrameBroadcaster* broadcaster, uint32_t consumer_id,
                       const AudioRingBuffer* ring_buffer = nullptr);
    ~AudioFrameConsumer();
    
    // Non-copyable, movable
    AudioFrameConsumer(const AudioFrameConsumer&) = delete;
    AudioFrameConsumer& operator=(const AudioFrameConsumer&) = delete;
    AudioFrameConsumer(AudioFrameConsumer&& other) noexcept;
    AudioFrameConsumer& operator=(AudioFrameConsumer&& other) noexcept;
    
    /**
     * @brief Try to get the next frame
     * @param out_frame Output frame buffer
     * @param wait_ms Maximum time to wait (0 = non-blocking)
     * @return true if frame was obtained
     */
    bool GetNextFrame(AudioFrame& out_frame, int wait_ms = 0);
    
    /**
     * @brief Get number of frames waiting to be consumed
     */
    size_t GetPendingFrameCount() const;
    
    /**
     * @brief Reset to latest frame (skip any backlog)
     */
    void ResetToLatest();
    
    /**
     * @brief Get consumer ID
     */
    uint32_t GetId() const { return consumer_id_; }
    
    /**
     * @brief Check if consumer is valid
     */
    bool IsValid() const { return broadcaster_ != nullptr; }
    
    /**
     * @brief Returns true if this consumer reads from the PCM ring buffer
     */
    bool IsPcmConsumer() const { return is_pcm_consumer_; }

private:
    friend class AudioFrameBroadcaster;
    AudioFrameBroadcaster* broadcaster_;
    const AudioRingBuffer* ring_buffer_;  // Ring buffer to read from
    uint32_t consumer_id_;
    uint64_t next_sequence_;              // Next sequence number to read
    std::atomic<bool> active_;
    bool is_pcm_consumer_ = false;        // True if reading from PCM ring buffer

    // Rate-limited skip logging
    uint64_t total_frames_skipped_ = 0;
    std::chrono::steady_clock::time_point last_skip_log_time_{};
};

// ============================================================================
// AudioFrameBroadcaster - Singleton producer
// ============================================================================

class AudioFrameBroadcaster {
public:
    /**
     * @brief Get the singleton instance
     */
    static AudioFrameBroadcaster& Instance();
    
    // Delete copy/move operations
    AudioFrameBroadcaster(const AudioFrameBroadcaster&) = delete;
    AudioFrameBroadcaster& operator=(const AudioFrameBroadcaster&) = delete;
    
    /**
     * @brief Initialize the broadcaster with audio configuration
     * @param codec Audio codec to use
     * @param sample_rate Target sample rate (e.g., 8000 for G.711)
     * @param channels Number of channels (typically 1)
     * @return true on success
     */
    bool Initialize(RtspAudioCodec codec, int sample_rate, int channels);
    
    /**
     * @brief Reconfigure the broadcaster with new audio settings
     * @param codec New audio codec to use
     * @param sample_rate New sample rate (e.g., 8000 for G.711)
     * @param channels New number of channels
     * @return true on success
     * 
     * This will stop the broadcaster, reinitialize with new settings, and restart.
     * Called when audio codec is changed via API.
     */
    bool Reconfigure(RtspAudioCodec codec, int sample_rate, int channels);

    /**
     * @brief Reset broadcaster state for HDAL pipeline reinit
     */
    void ResetForPipelineReinit();
    
    /**
     * @brief Start the producer thread
     * @return true on success
     */
    bool Start();
    
    /**
     * @brief Stop the producer thread
     */
    void Stop();
    
    /**
     * @brief Check if broadcaster is running
     */
    bool IsRunning() const { return running_.load(std::memory_order_acquire); }
    
    /**
     * @brief Check if broadcaster is initialized
     */
    bool IsInitialized() const { return initialized_.load(std::memory_order_acquire); }
    
    /**
     * @brief Register a new consumer
     * @return Consumer handle (unique_ptr manages lifetime)
     */
    std::unique_ptr<AudioFrameConsumer> RegisterConsumer();
    
    /**
     * @brief Unregister a consumer by ID
     */
    void UnregisterConsumer(uint32_t consumer_id);
    
    /**
     * @brief Get the ring buffer for direct access
     */
    const AudioRingBuffer& GetRingBuffer() const { return ring_buffer_; }
    
    /**
     * @brief Get current codec
     */
    RtspAudioCodec GetCodec() const { return codec_; }
    
    /**
     * @brief Get sample rate
     */
    int GetSampleRate() const { return sample_rate_; }
    
    /**
     * @brief Get number of channels
     */
    int GetChannels() const { return channels_; }
    
    /**
     * @brief Reconfigure audio processor with current AudioControl settings
     * 
     * Call this after changing audio processing settings via API
     * (AGC, NS, HPF, Notch, etc.) to apply changes immediately
     * without restarting ipcamd.
     */
    void ReconfigureAudioProcessor();
    
    /**
     * @brief Get number of active consumers
     */
    size_t GetConsumerCount() const;
    
    /**
     * @brief Wait for a new frame (used by consumers)
     * @param timeout_ms Maximum wait time
     * @return true if signaled, false if timeout
     */
    bool WaitForFrame(int timeout_ms);
    
    /**
     * @brief Get statistics
     */
    struct Stats {
        uint64_t frames_produced;
        uint64_t frames_dropped;        // Frames dropped due to no consumers
        uint64_t encode_errors;
        uint64_t hdal_pull_errors;
        uint64_t consumer_count;
    };
    Stats GetStats() const;

    // ==================================================================
    // PCM Ring Buffer — processed (resampled + filtered) mono PCM
    // Used by the recording module to get clean audio for AAC encoding
    // ==================================================================

    /**
     * @brief Get the PCM ring buffer for direct read access
     *
     * Contains processed 8 kHz / mono / s16le PCM frames written by
     * the producer thread after resampling + audio processing but
     * *before* codec encoding.  Recording registers a consumer on
     * this buffer to get clean PCM for its own AAC encoder.
     */
    const AudioRingBuffer& GetPcmRingBuffer() const { return pcm_ring_buffer_; }

    /**
     * @brief Register a new PCM consumer (for recording)
     * @return Consumer handle — audio frames will have codec == kPcm
     */
    std::unique_ptr<AudioFrameConsumer> RegisterPcmConsumer();

    /**
     * @brief Unregister a PCM consumer
     */
    void UnregisterPcmConsumer(uint32_t consumer_id);

    /**
     * @brief Wait for a new PCM frame
     */
    bool WaitForPcmFrame(int timeout_ms);

private:
    AudioFrameBroadcaster();
    ~AudioFrameBroadcaster();
    
    void ProducerThread();
    bool SetupHdalBuffer();
    void CleanupHdalBuffer();
    bool EncodeFrame(const uint8_t* pcm_data, size_t pcm_size, 
                     uint64_t timestamp, AudioFrame& out_frame);
    void InitializeAudioProcessor();
    void FlushHdalAudioBuffers(const char* context);
    
    // Configuration
    RtspAudioCodec codec_;
    int sample_rate_;
    int channels_;
    
    // HDAL resources
    HD_PATH_ID enc_path_;
    UINTPTR vir_addr_;
    HD_AUDIOENC_BUFINFO phy_buf_;
    bool buffer_mapped_;
    
    // Encoders
    platform::G711Type g711_type_;
    std::unique_ptr<platform::G726Codec> g726_encoder_;
    std::unique_ptr<platform::AacCodec> aac_encoder_;
    
    // Working buffers
    std::vector<int16_t> resample_buffer_;
    std::vector<uint8_t> encode_buffer_;
    
    // Audio processor (NS, AGC, HPF, Notch)
    std::unique_ptr<AudioProcessor> audio_processor_;
    
    // Ring buffer (encoded audio for RTSP consumers)
    AudioRingBuffer ring_buffer_;

    // PCM ring buffer (processed mono PCM for recording consumers)
    AudioRingBuffer pcm_ring_buffer_;

    // PCM consumer management
    std::mutex pcm_consumer_mutex_;
    std::vector<uint32_t> active_pcm_consumers_;
    std::mutex pcm_frame_mutex_;
    std::condition_variable pcm_frame_cv_;

    // Thread control
    std::atomic<bool> initialized_;
    std::atomic<bool> running_;
    std::atomic<bool> stop_requested_;
    std::thread producer_thread_;
    
    // Consumer notification
    std::mutex frame_mutex_;
    std::condition_variable frame_cv_;
    
    // Consumer management
    std::mutex consumer_mutex_;
    std::atomic<uint32_t> next_consumer_id_;
    std::vector<uint32_t> active_consumers_;
    
    // Statistics
    std::atomic<uint64_t> frames_produced_;
    std::atomic<uint64_t> frames_dropped_;
    std::atomic<uint64_t> encode_errors_;
    std::atomic<uint64_t> hdal_pull_errors_;
};

} // namespace streaming
} // namespace ipcam

#endif // IPCAM_AUDIO_FRAME_BROADCASTER_H
