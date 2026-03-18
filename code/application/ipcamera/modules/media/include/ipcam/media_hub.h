/**
 * @file media_hub.h
 * @brief Central Media Distribution Hub - Shared Ring Buffer Architecture
 *
 * The MediaHub provides a centralized, zero-contention distribution layer
 * for encoded video (and future audio) frames from the HDAL pipeline.
 *
 * Architecture:
 * ┌─────────────────────────────────────────────────────────────────┐
 * │                       HDAL Pipeline                            │
 * │  Sensor → ISP → VideoProc → Encoder[0] → Encoder[1] → ...     │
 * └──────────────┬──────────────────┬──────────────────────────────┘
 *                │                  │
 *         ┌──────▼──────┐   ┌──────▼──────┐
 *         │ Producer[0] │   │ Producer[1] │  ... (one per channel)
 *         │ (HDAL pull) │   │ (HDAL pull) │
 *         └──────┬──────┘   └──────┬──────┘
 *                │                  │
 *         ┌──────▼──────┐   ┌──────▼──────┐
 *         │ RingBuffer  │   │ RingBuffer  │  ... (SPMC per channel)
 *         │  [0]        │   │  [1]        │
 *         └──┬───┬───┬──┘   └──┬───┬───┬──┘
 *            │   │   │         │   │   │
 *         ┌──▼┐┌─▼─┐┌▼──┐  ┌──▼┐┌─▼─┐┌▼──┐
 *         │RTSP│Rec │WebR│  │RTSP│Rec │HLS │  ... (consumers)
 *         └────┘└───┘└───┘  └────┘└───┘└───┘
 *
 * Design principles:
 * - Single Producer per channel: Only one thread pulls from HDAL encoder
 * - Multi Consumer: Any number of consumers read independently
 * - Lock-free reads: Sequence-number based SPMC ring buffer
 * - Copy-on-read: Frame data is copied to consumer buffers (safe, ~3MB/s)
 * - NAL parameter tracking: SPS/PPS/VPS cached for late-joining consumers
 *
 * Usage:
 * @code
 * // Producer side (done internally by MediaHub)
 * MediaHub::Instance().Initialize();
 * MediaHub::Instance().Start();
 *
 * // Consumer side (RTSP, Recording, WebRTC, HLS, etc.)
 * auto consumer = MediaHub::Instance().CreateConsumer(0, "rtsp-stream1");
 * VideoFrame frame;
 * while (running) {
 *     if (consumer->WaitForFrame(frame, 100)) {
 *         // Process frame.data, frame.size, frame.is_keyframe, etc.
 *     }
 * }
 * @endcode
 */

#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ipcam {
namespace media {

// ============================================================================
// Constants
// ============================================================================

/// Number of slots per channel ring buffer (1 second at 30fps)
static constexpr size_t kVideoRingBufferSlots = 30;

/// Maximum single frame size for main stream (512KB covers 5MP keyframes)
static constexpr size_t kMaxVideoFrameSize = 512 * 1024;

/// Maximum single frame size for sub-streams (128KB covers 720p/VGA keyframes)
static constexpr size_t kMaxSubStreamFrameSize = 128 * 1024;

/// Maximum consumers per channel
static constexpr size_t kMaxMediaHubConsumers = 16;

/// Maximum video channels supported
static constexpr int kMaxVideoChannels = 4;

/// HDAL pull timeout in milliseconds
static constexpr int kVideoPullTimeoutMs = 100;

/// NAL parameter buffer size
static constexpr size_t kMaxNalParamSize = 256;

// ============================================================================
// VideoCodecType
// ============================================================================

enum class VideoCodecType {
    kH264,
    kH265,
    kMJPEG
};

inline const char* VideoCodecTypeToString(VideoCodecType codec) {
    switch (codec) {
        case VideoCodecType::kH264:  return "H.264";
        case VideoCodecType::kH265:  return "H.265";
        case VideoCodecType::kMJPEG: return "MJPEG";
        default: return "Unknown";
    }
}

// ============================================================================
// VideoFrame - Represents a single encoded video frame
// ============================================================================

/**
 * @brief Encoded video frame with metadata
 *
 * When returned by VideoFrameConsumer::GetNextFrame() or WaitForFrame(),
 * the `data` vector contains a copy of the encoded frame data.
 * NAL parameters (SPS/PPS/VPS) are populated for keyframes.
 */
struct VideoFrame {
    std::vector<uint8_t> data;          ///< Encoded frame data (NAL units)
    uint64_t timestamp_us = 0;          ///< Hardware timestamp (microseconds)
    uint64_t capture_time_ms = 0;       ///< Wall clock time (Unix epoch ms)
    uint64_t sequence = 0;              ///< Monotonic sequence number (per channel)
    int channel_id = -1;                ///< Source channel (0-3)
    bool is_keyframe = false;           ///< True for IDR frames
    VideoCodecType codec = VideoCodecType::kH264;

    /// NAL parameters (populated for keyframes, may be cached from previous keyframe)
    uint8_t sps[kMaxNalParamSize] = {};
    uint16_t sps_size = 0;
    uint8_t pps[kMaxNalParamSize] = {};
    uint16_t pps_size = 0;
    uint8_t vps[kMaxNalParamSize] = {};  ///< H.265 only
    uint16_t vps_size = 0;

    /// Pack info (multiple NAL units per frame from HDAL)
    struct NalPack {
        uint32_t offset;    ///< Offset into data[]
        uint32_t size;      ///< Size of this NAL unit
        bool is_idr;        ///< Is this NAL an IDR slice
    };
    std::vector<NalPack> nal_packs;     ///< Individual NAL unit offsets

    void Clear() {
        data.clear();
        nal_packs.clear();
        timestamp_us = 0;
        capture_time_ms = 0;
        sequence = 0;
        channel_id = -1;
        is_keyframe = false;
        codec = VideoCodecType::kH264;
        sps_size = 0;
        pps_size = 0;
        vps_size = 0;
    }
};

// ============================================================================
// VideoRingBuffer - Single-Producer Multi-Consumer Ring Buffer
// ============================================================================

/**
 * @brief Lock-free SPMC ring buffer for encoded video frames.
 *
 * - One producer writes frames sequentially.
 * - Multiple consumers read independently using sequence numbers.
 * - Uses sequence-based versioning to detect torn reads.
 * - Each slot pre-allocates buffer memory to avoid hot-path allocations.
 */
class VideoRingBuffer {
public:
    explicit VideoRingBuffer(size_t slot_count = kVideoRingBufferSlots,
                            size_t max_frame_size = kMaxVideoFrameSize);
    ~VideoRingBuffer();

    /// Get the max frame size for this buffer
    size_t GetMaxFrameSize() const { return max_frame_size_; }

    // Non-copyable
    VideoRingBuffer(const VideoRingBuffer&) = delete;
    VideoRingBuffer& operator=(const VideoRingBuffer&) = delete;

    /**
     * @brief Write a frame to the ring buffer (producer only, single-threaded)
     *
     * @param frame_data  Encoded frame bytes
     * @param frame_size  Size of frame_data
     * @param timestamp_us Hardware timestamp in microseconds
     * @param capture_time_ms Wall clock capture time (Unix epoch ms)
     * @param channel_id  Source channel ID
     * @param is_keyframe True if IDR frame
     * @param codec       Video codec type
     * @param sps         SPS NAL data (may be null)
     * @param sps_size    SPS data size
     * @param pps         PPS NAL data (may be null)
     * @param pps_size    PPS data size
     * @param vps         VPS NAL data (may be null, H.265 only)
     * @param vps_size    VPS data size
     * @param nal_packs   Individual NAL unit boundaries within frame_data
     * @return Sequence number assigned to this frame
     */
    uint64_t Write(const uint8_t* frame_data, uint32_t frame_size,
                   uint64_t timestamp_us, uint64_t capture_time_ms,
                   int channel_id, bool is_keyframe, VideoCodecType codec,
                   const uint8_t* sps, uint16_t sps_size,
                   const uint8_t* pps, uint16_t pps_size,
                   const uint8_t* vps, uint16_t vps_size,
                   const std::vector<VideoFrame::NalPack>& nal_packs = {});

    /**
     * @brief Read a frame by sequence number (consumer, thread-safe)
     *
     * @param seq       Sequence number to read
     * @param out_frame Output frame (data is copied)
     * @return true if frame was successfully read
     */
    bool Read(uint64_t seq, VideoFrame& out_frame) const;

    /**
     * @brief Get the latest written sequence number
     */
    uint64_t GetLatestSequence() const {
        return frames_written_.load(std::memory_order_acquire);
    }

    /**
     * @brief Get the oldest sequence still available in the ring buffer
     */
    uint64_t GetOldestSequence() const {
        uint64_t latest = frames_written_.load(std::memory_order_acquire);
        if (latest <= slot_count_) return 1;
        return latest - slot_count_ + 1;
    }

    /**
     * @brief Find the nearest keyframe at or before the given sequence
     * @param seq Starting sequence to search backwards from
     * @return Sequence number of the keyframe, or 0 if not found
     */
    uint64_t FindKeyframeBefore(uint64_t seq) const;

    /**
     * @brief Find the nearest keyframe at or after the given sequence
     * @param seq Starting sequence to search forwards from
     * @return Sequence number of the keyframe, or 0 if not found
     */
    uint64_t FindKeyframeAfter(uint64_t seq) const;

    /**
     * @brief Get the slot count
     */
    size_t GetSlotCount() const { return slot_count_; }

private:
    struct Slot {
        std::atomic<uint64_t> version{0};   ///< Odd = writing, even = ready
        uint8_t* buffer = nullptr;          ///< Pointer to HDAL memory
        uint32_t size = 0;
        uint64_t timestamp_us = 0;
        uint64_t capture_time_ms = 0;
        int channel_id = -1;
        bool is_keyframe = false;
        VideoCodecType codec = VideoCodecType::kH264;

        uint8_t sps[kMaxNalParamSize] = {};
        uint16_t sps_size = 0;
        uint8_t pps[kMaxNalParamSize] = {};
        uint16_t pps_size = 0;
        uint8_t vps[kMaxNalParamSize] = {};
        uint16_t vps_size = 0;

        std::vector<VideoFrame::NalPack> nal_packs;

        Slot() = default;
    };

    const size_t slot_count_;
    const size_t max_frame_size_;
    std::vector<Slot> slots_;
    alignas(64) std::atomic<size_t> write_pos_{0};
    alignas(64) std::atomic<uint64_t> frames_written_{0};

    // Memory Management
    void* pool_vir_addr_ = nullptr;
    uintptr_t pool_phy_addr_ = 0;
    bool using_hdal_mem_ = false;  ///< true if pool_vir_addr_ was allocated via hd_common_mem_alloc
};

// ============================================================================
// VideoFrameConsumer - Consumer handle for reading from a channel's ring buffer
// ============================================================================

/**
 * @brief Consumer handle for reading video frames from a channel.
 *
 * Each consumer maintains its own read cursor (next_sequence_) and reads
 * independently. If a consumer falls too far behind, it will skip to the
 * nearest available keyframe.
 *
 * Thread-safe: Multiple consumers can read concurrently from the same
 * ring buffer without contention.
 */
class VideoFrameConsumer {
public:
    VideoFrameConsumer(const VideoRingBuffer* ring_buffer,
                       std::condition_variable* frame_cv,
                       std::mutex* frame_mutex,
                       uint32_t consumer_id,
                       int channel_id,
                       const std::string& name);
    ~VideoFrameConsumer();

    // Non-copyable
    VideoFrameConsumer(const VideoFrameConsumer&) = delete;
    VideoFrameConsumer& operator=(const VideoFrameConsumer&) = delete;

    // Movable
    VideoFrameConsumer(VideoFrameConsumer&&) noexcept;
    VideoFrameConsumer& operator=(VideoFrameConsumer&&) noexcept;

    /**
     * @brief Try to get the next frame (non-blocking)
     * @param out_frame Output frame buffer
     * @return true if a frame was obtained
     */
    bool GetNextFrame(VideoFrame& out_frame);

    /**
     * @brief Wait for the next frame (blocking with timeout)
     * @param out_frame Output frame buffer
     * @param timeout_ms Maximum wait time (0 = non-blocking)
     * @return true if a frame was obtained
     */
    bool WaitForFrame(VideoFrame& out_frame, int timeout_ms);

    /**
     * @brief Get number of frames waiting to be consumed
     */
    size_t GetPendingFrameCount() const;

    /**
     * @brief Skip to the latest frame (discard backlog)
     */
    void ResetToLatest();

    /**
     * @brief Skip to the nearest keyframe in the buffer
     *
     * Useful for late-joining consumers (RTSP clients) that need
     * to start playback from a keyframe.
     */
    void ResetToKeyframe();

    uint32_t GetId() const { return consumer_id_; }
    int GetChannelId() const { return channel_id_; }
    const std::string& GetName() const { return name_; }

    /**
     * @brief Check if this consumer has fallen behind (frames were overwritten)
     */
    bool HasDroppedFrames() const { return frames_dropped_ > 0; }
    uint64_t GetDroppedFrameCount() const { return frames_dropped_; }
    uint64_t GetConsumedFrameCount() const { return frames_consumed_; }

private:
    bool AdvanceToValidSequence();

    const VideoRingBuffer* ring_buffer_;
    std::condition_variable* frame_cv_;
    std::mutex* frame_mutex_;
    uint32_t consumer_id_;
    int channel_id_;
    std::string name_;
    uint64_t next_sequence_ = 0;
    uint64_t frames_consumed_ = 0;
    uint64_t frames_dropped_ = 0;
};

// ============================================================================
// MediaHub - Central Media Distribution Singleton
// ============================================================================

/**
 * @brief Central hub for distributing encoded media frames.
 *
 * Manages one producer thread per active video channel. Each producer
 * pulls encoded frames from the HDAL video encoder and writes them
 * into a per-channel ring buffer. Consumers register through
 * CreateConsumer() and read frames at their own pace.
 *
 * Lifecycle:
 * 1. Initialize() - Detect active channels from HdalPipeline config
 * 2. Start()      - Launch producer threads
 * 3. CreateConsumer() / destroy consumers as needed
 * 4. Stop()       - Stop producer threads
 * 5. Shutdown()   - Release all resources
 */
class MediaHub {
public:
    static MediaHub& Instance();

    // Non-copyable
    MediaHub(const MediaHub&) = delete;
    MediaHub& operator=(const MediaHub&) = delete;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /**
     * @brief Initialize the MediaHub
     *
     * Reads HdalPipeline configuration to determine active channels.
     * Allocates ring buffers for each enabled channel.
     *
     * @return true on success
     */
    bool Initialize();

    /**
     * @brief Start producer threads for all active channels
     * @return true on success
     */
    bool Start();

    /**
     * @brief Stop all producer threads (consumers can no longer get new frames)
     */
    void Stop();

    /**
     * @brief Shutdown and release all resources
     */
    void Shutdown();

    /**
     * @brief Reinitialize after HDAL pipeline restart
     *
     * Call this when the HDAL pipeline is reinitialized (e.g., resolution change).
     * Stops producers, re-reads pipeline config, and restarts.
     */
    bool Reinitialize();

    bool IsRunning() const { return running_.load(std::memory_order_acquire); }
    bool IsInitialized() const { return initialized_.load(std::memory_order_acquire); }

    // ========================================================================
    // Consumer Management
    // ========================================================================

    /**
     * @brief Create a consumer for a specific video channel
     *
     * @param channel_id Video channel (0-3)
     * @param name       Consumer name for debugging (e.g., "rtsp-stream1", "recording-ch0")
     * @return Consumer handle (unique_ptr manages lifetime), nullptr on error
     */
    std::unique_ptr<VideoFrameConsumer> CreateConsumer(int channel_id, const std::string& name);

    // ========================================================================
    // Channel Information
    // ========================================================================

    /**
     * @brief Get number of active (enabled) channels
     */
    int GetActiveChannelCount() const;

    /**
     * @brief Check if a specific channel is active
     */
    bool IsChannelActive(int channel_id) const;

    /**
     * @brief Get the ring buffer for a channel (for advanced consumers)
     * @return Ring buffer pointer, or nullptr if channel not active
     */
    const VideoRingBuffer* GetRingBuffer(int channel_id) const;

    // ========================================================================
    // Statistics
    // ========================================================================

    struct ChannelStats {
        uint64_t frames_produced = 0;
        uint64_t bytes_produced = 0;
        uint64_t keyframes_produced = 0;
        uint64_t pull_errors = 0;
        uint64_t oversize_frames = 0;   ///< Frames exceeding kMaxVideoFrameSize
        uint32_t consumer_count = 0;
        uint64_t latest_sequence = 0;
        double avg_frame_size = 0.0;
    };

    ChannelStats GetChannelStats(int channel_id) const;

    /**
     * @brief Wait for a new frame on a specific channel
     * @param channel_id Channel to wait on
     * @param timeout_ms Maximum wait time
     * @return true if signaled (new frame), false if timeout
     */
    bool WaitForFrame(int channel_id, int timeout_ms);

    /**
     * @brief Notify MediaHub that encoder codec has changed
     *
     * Call this after changing encoder codec (e.g., H.264 <-> H.265).
     * Clears cached SPS/PPS/VPS and updates internal codec tracking.
     *
     * @param channel_id Channel that changed codec (0-3)
     * @param new_codec  New codec type
     */
    void NotifyCodecChange(int channel_id, VideoCodecType new_codec);

    /**
     * @brief Pause producer for a channel prior to encoder parameter changes
     *
     * Call this BEFORE any HDAL encoder stop/start operations.
     * The producer thread will stop pulling frames until ResumeChannel() is called.
     *
     * @param channel_id Channel to pause (0-3)
     */
    void PauseChannel(int channel_id);

    /**
     * @brief Resume producer after encoder parameter changes are complete
     *
     * Clears cached SPS/PPS/VPS, flushes stale frames, requests an IDR,
     * and resumes the producer thread pulling.
     *
     * @param channel_id Channel to resume (0-3)
     * @param codec_changed If true, also update cached_codec from pipeline config
     */
    void ResumeChannel(int channel_id, bool codec_changed = false);

private:
    friend class VideoFrameConsumer;

    MediaHub();
    ~MediaHub();

    // ========================================================================
    // Per-Channel State
    // ========================================================================

    struct ChannelState {
        bool enabled = false;
        std::unique_ptr<VideoRingBuffer> ring_buffer;
        std::thread producer_thread;
        std::atomic<bool> stop_requested{false};
        std::atomic<bool> paused{false};

        // HDAL resources (set during Start())
        uint64_t enc_path = 0;
        uintptr_t vir_addr = 0;
        uint64_t phy_addr = 0;
        uint32_t buf_size = 0;
        bool buffer_mapped = false;

        // Cached NAL parameters (from most recent keyframe)
        uint8_t cached_sps[kMaxNalParamSize] = {};
        uint16_t cached_sps_size = 0;
        uint8_t cached_pps[kMaxNalParamSize] = {};
        uint16_t cached_pps_size = 0;
        uint8_t cached_vps[kMaxNalParamSize] = {};
        uint16_t cached_vps_size = 0;
        VideoCodecType cached_codec = VideoCodecType::kH264;

        // Statistics
        std::atomic<uint64_t> frames_produced{0};
        std::atomic<uint64_t> bytes_produced{0};
        std::atomic<uint64_t> keyframes_produced{0};
        std::atomic<uint64_t> pull_errors{0};
        std::atomic<uint64_t> oversize_frames{0};

        // Consumer notification
        std::mutex frame_mutex;
        std::condition_variable frame_cv;

        // Consumer tracking
        std::atomic<uint32_t> consumer_count{0};
    };

    // ========================================================================
    // Internal Methods
    // ========================================================================

    void ProducerThread(int channel_id);
    bool SetupChannelHdal(int channel_id);
    void CleanupChannelHdal(int channel_id);
    void FlushEncoderBuffer(int channel_id);

    /// Extract SPS/PPS/VPS from NAL data
    bool ExtractNalParameters(const uint8_t* data, uint32_t size,
                              VideoCodecType codec,
                              uint8_t* sps, uint16_t& sps_size,
                              uint8_t* pps, uint16_t& pps_size,
                              uint8_t* vps, uint16_t& vps_size);

    /// Detect if a NAL unit is an IDR frame
    bool IsIdrFrame(const uint8_t* data, uint32_t size, VideoCodecType codec);

    /// Get current codec type for a channel from config
    VideoCodecType GetChannelCodec(int channel_id) const;

    /// Get current wall-clock time in milliseconds
    static uint64_t GetCurrentTimeMs();

    // ========================================================================
    // Members
    // ========================================================================
    std::array<ChannelState, kMaxVideoChannels> channels_;
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::atomic<uint32_t> next_consumer_id_{0};
};

} // namespace media
} // namespace ipcam
