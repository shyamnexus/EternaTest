/**
 * @file media_hub.cpp
 * @brief Central Media Distribution Hub - Implementation
 *
 * Implements the MediaHub singleton which manages per-channel video
 * producer threads and SPMC ring buffers. Each producer pulls encoded
 * frames from the HDAL video encoder and distributes them to all
 * registered consumers (RTSP, recording, WebRTC, HLS, etc.).
 */

#include "ipcam/media_hub.h"
#include "ipcam/hdal_pipeline.h"
#include "ipcam/config.h"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <chrono>
#include <unistd.h>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
#include "hd_videoenc.h"
#include "hd_common.h"
}
#endif

namespace ipcam {
namespace media {

// ============================================================================
// VideoRingBuffer Implementation
// ============================================================================

VideoRingBuffer::VideoRingBuffer(size_t slot_count, size_t max_frame_size)
    : slot_count_(slot_count)
    , max_frame_size_(max_frame_size)
    , slots_(slot_count) {
    
    size_t total_size = slot_count * max_frame_size;

#if HDAL_PIPELINE_ENABLED
    // Try HDAL memory first (physically contiguous, DMA-capable)
    char name[32];
    snprintf(name, sizeof(name), "v_ring_%p", this);
    
    HD_RESULT ret = hd_common_mem_alloc(name, &pool_phy_addr_, &pool_vir_addr_,
                                        static_cast<UINT32>(total_size), DDR_ID0);
    
    if (ret == HD_OK && pool_vir_addr_ != nullptr) {
        using_hdal_mem_ = true;
        spdlog::info("VideoRingBuffer: Allocated {} slots x {}KB = {:.1f} MB (HDAL memory)",
                     slot_count, max_frame_size / 1024, total_size / (1024.0 * 1024.0));
        
        uint8_t* base = static_cast<uint8_t*>(pool_vir_addr_);
        for (size_t i = 0; i < slot_count; ++i) {
            slots_[i].buffer = base + (i * max_frame_size);
        }
        return;
    } else {
        spdlog::warn("VideoRingBuffer: HDAL alloc failed (ret={}), falling back to system memory",
                     static_cast<int>(ret));
        pool_vir_addr_ = nullptr;
    }
#endif

    // System memory fallback (always available, works for unit tests too)
    pool_vir_addr_ = std::malloc(total_size);
    using_hdal_mem_ = false;
    if (pool_vir_addr_) {
        spdlog::info("VideoRingBuffer: Allocated {} slots x {}KB = {:.1f} MB (system memory)",
                     slot_count, max_frame_size / 1024, total_size / (1024.0 * 1024.0));
        
        uint8_t* base = static_cast<uint8_t*>(pool_vir_addr_);
        for (size_t i = 0; i < slot_count; ++i) {
            slots_[i].buffer = base + (i * max_frame_size);
        }
    } else {
        spdlog::critical("VideoRingBuffer: FATAL - Failed to allocate {} MB",
                         total_size / (1024.0 * 1024.0));
    }
}

VideoRingBuffer::~VideoRingBuffer() {
    if (pool_vir_addr_) {
#if HDAL_PIPELINE_ENABLED
        if (using_hdal_mem_) {
            hd_common_mem_free(pool_phy_addr_, pool_vir_addr_);
        } else {
            std::free(pool_vir_addr_);
        }
#else
        std::free(pool_vir_addr_);
#endif
        pool_vir_addr_ = nullptr;
    }
}

uint64_t VideoRingBuffer::Write(const uint8_t* frame_data, uint32_t frame_size,
                                uint64_t timestamp_us, uint64_t capture_time_ms,
                                int channel_id, bool is_keyframe, VideoCodecType codec,
                                const uint8_t* sps, uint16_t sps_sz,
                                const uint8_t* pps, uint16_t pps_sz,
                                const uint8_t* vps, uint16_t vps_sz,
                                const std::vector<VideoFrame::NalPack>& nal_packs) {
    uint64_t seq = ++frames_written_;
    size_t pos = write_pos_.fetch_add(1, std::memory_order_relaxed) % slot_count_;

    Slot& slot = slots_[pos];

    // Mark as being written (odd version = in-progress)
    slot.version.store(seq * 2 - 1, std::memory_order_release);

    // Copy frame data
    if (!slot.buffer) return 0; // Check for allocation failure

    uint32_t copy_size = std::min(frame_size, static_cast<uint32_t>(max_frame_size_));
    if (copy_size > 0 && frame_data) {
        std::memcpy(slot.buffer, frame_data, copy_size);
    }
    slot.size = copy_size;
    slot.timestamp_us = timestamp_us;
    slot.capture_time_ms = capture_time_ms;
    slot.channel_id = channel_id;
    slot.is_keyframe = is_keyframe;
    slot.codec = codec;

    // Copy NAL parameters
    if (sps && sps_sz > 0 && sps_sz <= kMaxNalParamSize) {
        std::memcpy(slot.sps, sps, sps_sz);
        slot.sps_size = sps_sz;
    } else {
        slot.sps_size = 0;
    }
    if (pps && pps_sz > 0 && pps_sz <= kMaxNalParamSize) {
        std::memcpy(slot.pps, pps, pps_sz);
        slot.pps_size = pps_sz;
    } else {
        slot.pps_size = 0;
    }
    if (vps && vps_sz > 0 && vps_sz <= kMaxNalParamSize) {
        std::memcpy(slot.vps, vps, vps_sz);
        slot.vps_size = vps_sz;
    } else {
        slot.vps_size = 0;
    }

    slot.nal_packs = nal_packs;

    // Mark as ready (even version = complete)
    slot.version.store(seq * 2, std::memory_order_release);

    return seq;
}

bool VideoRingBuffer::Read(uint64_t seq, VideoFrame& out_frame) const {
    if (seq == 0) return false;

    // Check if frame might still be in buffer
    uint64_t current = frames_written_.load(std::memory_order_acquire);
    if (seq + slot_count_ <= current) {
        // Frame has been overwritten
        return false;
    }

    size_t pos = (seq - 1) % slot_count_;
    const Slot& slot = slots_[pos];

    // Check if slot is ready and contains the expected sequence
    uint64_t ver = slot.version.load(std::memory_order_acquire);
    if (ver != seq * 2) {
        // Either being written or different frame
        return false;
    }

    // Copy frame data
    out_frame.data.resize(slot.size);
    if (slot.size > 0 && slot.buffer) {
        std::memcpy(out_frame.data.data(), slot.buffer, slot.size);
    }
    out_frame.timestamp_us = slot.timestamp_us;
    out_frame.capture_time_ms = slot.capture_time_ms;
    out_frame.channel_id = slot.channel_id;
    out_frame.is_keyframe = slot.is_keyframe;
    out_frame.codec = slot.codec;
    out_frame.sequence = seq;

    // Copy NAL parameters
    if (slot.sps_size > 0) {
        std::memcpy(out_frame.sps, slot.sps, slot.sps_size);
        out_frame.sps_size = slot.sps_size;
    } else {
        out_frame.sps_size = 0;
    }
    if (slot.pps_size > 0) {
        std::memcpy(out_frame.pps, slot.pps, slot.pps_size);
        out_frame.pps_size = slot.pps_size;
    } else {
        out_frame.pps_size = 0;
    }
    if (slot.vps_size > 0) {
        std::memcpy(out_frame.vps, slot.vps, slot.vps_size);
        out_frame.vps_size = slot.vps_size;
    } else {
        out_frame.vps_size = 0;
    }

    out_frame.nal_packs = slot.nal_packs;

    // Verify that the slot wasn't overwritten during our read
    uint64_t ver_after = slot.version.load(std::memory_order_acquire);
    if (ver_after != seq * 2) {
        return false;
    }

    return true;
}

uint64_t VideoRingBuffer::FindKeyframeBefore(uint64_t seq) const {
    uint64_t oldest = GetOldestSequence();
    if (seq < oldest) return 0;

    // Search backwards from seq
    for (uint64_t s = seq; s >= oldest && s > 0; --s) {
        size_t pos = (s - 1) % slot_count_;
        const Slot& slot = slots_[pos];

        uint64_t ver = slot.version.load(std::memory_order_acquire);
        if (ver == s * 2 && slot.is_keyframe) {
            return s;
        }
    }
    return 0;
}

uint64_t VideoRingBuffer::FindKeyframeAfter(uint64_t seq) const {
    uint64_t latest = GetLatestSequence();
    uint64_t oldest = GetOldestSequence();
    uint64_t start = std::max(seq, oldest);

    for (uint64_t s = start; s <= latest; ++s) {
        size_t pos = (s - 1) % slot_count_;
        const Slot& slot = slots_[pos];

        uint64_t ver = slot.version.load(std::memory_order_acquire);
        if (ver == s * 2 && slot.is_keyframe) {
            return s;
        }
    }
    return 0;
}

// ============================================================================
// VideoFrameConsumer Implementation
// ============================================================================

VideoFrameConsumer::VideoFrameConsumer(const VideoRingBuffer* ring_buffer,
                                       std::condition_variable* frame_cv,
                                       std::mutex* frame_mutex,
                                       uint32_t consumer_id,
                                       int channel_id,
                                       const std::string& name)
    : ring_buffer_(ring_buffer)
    , frame_cv_(frame_cv)
    , frame_mutex_(frame_mutex)
    , consumer_id_(consumer_id)
    , channel_id_(channel_id)
    , name_(name)
    , next_sequence_(0)
    , frames_consumed_(0)
    , frames_dropped_(0) {
    // Start from the next upcoming keyframe for clean playback
    ResetToKeyframe();
    spdlog::info("VideoFrameConsumer[{}]: Created '{}' for channel {}, starting at seq {}",
                 consumer_id_, name_, channel_id_, next_sequence_);
}

VideoFrameConsumer::~VideoFrameConsumer() {
    if (channel_id_ >= 0 && channel_id_ < kMaxVideoChannels) {
        auto prev = MediaHub::Instance().channels_[channel_id_].consumer_count.fetch_sub(1);
        spdlog::info("VideoFrameConsumer[{}]: Destroyed '{}' for channel {} "
                     "(total consumers: {})",
                     consumer_id_, name_, channel_id_, prev - 1);
    }
}

VideoFrameConsumer::VideoFrameConsumer(VideoFrameConsumer&& other) noexcept
    : ring_buffer_(other.ring_buffer_)
    , frame_cv_(other.frame_cv_)
    , frame_mutex_(other.frame_mutex_)
    , consumer_id_(other.consumer_id_)
    , channel_id_(other.channel_id_)
    , name_(std::move(other.name_))
    , next_sequence_(other.next_sequence_)
    , frames_consumed_(other.frames_consumed_)
    , frames_dropped_(other.frames_dropped_) {
    other.ring_buffer_ = nullptr;
    other.frame_cv_ = nullptr;
    other.frame_mutex_ = nullptr;
    other.channel_id_ = -1;  // Prevent source destructor from decrementing
}

VideoFrameConsumer& VideoFrameConsumer::operator=(VideoFrameConsumer&& other) noexcept {
    if (this != &other) {
        // Decrement count for our current channel before overwriting
        if (channel_id_ >= 0 && channel_id_ < kMaxVideoChannels) {
            MediaHub::Instance().channels_[channel_id_].consumer_count.fetch_sub(1);
        }
        ring_buffer_ = other.ring_buffer_;
        frame_cv_ = other.frame_cv_;
        frame_mutex_ = other.frame_mutex_;
        consumer_id_ = other.consumer_id_;
        channel_id_ = other.channel_id_;
        name_ = std::move(other.name_);
        next_sequence_ = other.next_sequence_;
        frames_consumed_ = other.frames_consumed_;
        frames_dropped_ = other.frames_dropped_;
        other.ring_buffer_ = nullptr;
        other.frame_cv_ = nullptr;
        other.frame_mutex_ = nullptr;
        other.channel_id_ = -1;  // Prevent source destructor from decrementing
    }
    return *this;
}

bool VideoFrameConsumer::GetNextFrame(VideoFrame& out_frame) {
    if (!ring_buffer_) return false;

    // Ensure our read position is valid
    if (!AdvanceToValidSequence()) {
        return false;
    }

    if (ring_buffer_->Read(next_sequence_, out_frame)) {
        next_sequence_++;
        frames_consumed_++;
        return true;
    }

    return false;
}

bool VideoFrameConsumer::WaitForFrame(VideoFrame& out_frame, int timeout_ms) {
    if (!ring_buffer_ || !frame_cv_ || !frame_mutex_) return false;

    // Try non-blocking first
    if (GetNextFrame(out_frame)) {
        return true;
    }

    // Wait for notification
    if (timeout_ms > 0) {
        std::unique_lock<std::mutex> lock(*frame_mutex_);
        frame_cv_->wait_for(lock, std::chrono::milliseconds(timeout_ms));
    }

    // Try again after wait
    return GetNextFrame(out_frame);
}

size_t VideoFrameConsumer::GetPendingFrameCount() const {
    if (!ring_buffer_) return 0;
    uint64_t latest = ring_buffer_->GetLatestSequence();
    if (latest >= next_sequence_) {
        return static_cast<size_t>(latest - next_sequence_ + 1);
    }
    return 0;
}

void VideoFrameConsumer::ResetToLatest() {
    if (!ring_buffer_) return;
    uint64_t latest = ring_buffer_->GetLatestSequence();
    if (latest > 0) {
        uint64_t old_seq = next_sequence_;
        next_sequence_ = latest + 1;
        if (next_sequence_ > old_seq + 1) {
            frames_dropped_ += (next_sequence_ - old_seq - 1);
        }
        spdlog::debug("VideoFrameConsumer[{}] '{}': Reset to latest seq {}",
                      consumer_id_, name_, next_sequence_);
    }
}

void VideoFrameConsumer::ResetToKeyframe() {
    if (!ring_buffer_) return;

    uint64_t latest = ring_buffer_->GetLatestSequence();
    if (latest == 0) {
        next_sequence_ = 1;
        return;
    }

    // Find the most recent keyframe
    uint64_t keyframe_seq = ring_buffer_->FindKeyframeBefore(latest);
    if (keyframe_seq > 0) {
        next_sequence_ = keyframe_seq;
        spdlog::debug("VideoFrameConsumer[{}] '{}': Reset to keyframe at seq {}",
                      consumer_id_, name_, next_sequence_);
    } else {
        // No keyframe found, start from latest
        next_sequence_ = latest + 1;
        spdlog::debug("VideoFrameConsumer[{}] '{}': No keyframe found, starting at seq {}",
                      consumer_id_, name_, next_sequence_);
    }
}

bool VideoFrameConsumer::AdvanceToValidSequence() {
    if (!ring_buffer_) return false;

    uint64_t oldest = ring_buffer_->GetOldestSequence();
    uint64_t latest = ring_buffer_->GetLatestSequence();

    if (latest == 0) {
        // No frames yet
        return false;
    }

    // If we're ahead of what's available, nothing to read yet
    if (next_sequence_ > latest) {
        return false;
    }

    // If our next read position has been overwritten, we need to catch up
    if (next_sequence_ < oldest) {
        uint64_t dropped = oldest - next_sequence_;
        frames_dropped_ += dropped;

        // Try to find the nearest keyframe after the oldest available frame
        uint64_t keyframe = ring_buffer_->FindKeyframeAfter(oldest);
        if (keyframe > 0) {
            next_sequence_ = keyframe;
            spdlog::warn("VideoFrameConsumer[{}] '{}': Dropped {} frames, "
                         "skipping to keyframe at seq {}",
                         consumer_id_, name_, dropped, next_sequence_);
        } else {
            // No keyframe found, just start from oldest
            next_sequence_ = oldest;
            spdlog::warn("VideoFrameConsumer[{}] '{}': Dropped {} frames, "
                         "no keyframe found, starting from seq {}",
                         consumer_id_, name_, dropped, next_sequence_);
        }
    }

    return true;
}

// ============================================================================
// MediaHub Implementation
// ============================================================================

MediaHub& MediaHub::Instance() {
    static MediaHub instance;
    return instance;
}

MediaHub::MediaHub() = default;

MediaHub::~MediaHub() {
    Shutdown();
}

bool MediaHub::Initialize() {
    if (initialized_.load()) {
        spdlog::warn("MediaHub: Already initialized");
        return true;
    }

    spdlog::info("MediaHub: Initializing...");

    auto& pipeline = platform::HdalPipeline::Instance();
    const auto& config = pipeline.GetConfig();

    int active_count = 0;
    for (int i = 0; i < kMaxVideoChannels; ++i) {
        bool enabled = config.video_streams[i].enabled;
        channels_[i].enabled = enabled;

        if (enabled) {
            // Calculate per-channel max frame size.
            // Stream 0 (main) always gets the large buffer since it can be
            // downscaled at runtime and still produce big I-frames.
            // Other streams: 256KB for >= 1080p, 128KB for smaller.
            uint32_t w = config.video_streams[i].width;
            uint32_t h = config.video_streams[i].height;
            size_t max_frame_size;
            if (i == 0) {
                max_frame_size = kMaxVideoFrameSize;  // 512KB — always main
            } else if (w >= 1920 || h >= 1080) {
                max_frame_size = 256 * 1024;  // 256KB for HD sub-streams
            } else {
                max_frame_size = kMaxSubStreamFrameSize;  // 128KB
            }

            channels_[i].ring_buffer = std::make_unique<VideoRingBuffer>(
                kVideoRingBufferSlots, max_frame_size);
            active_count++;
            spdlog::info("MediaHub: Channel {} enabled ({}x{} @ {}fps, max_frame={}KB)",
                         i, w, h, config.video_streams[i].fps, max_frame_size / 1024);
        }
    }

    if (active_count == 0) {
        spdlog::warn("MediaHub: No video channels enabled");
    }

    initialized_.store(true, std::memory_order_release);
    spdlog::info("MediaHub: Initialized with {} active channels", active_count);
    return true;
}

bool MediaHub::Start() {
    if (!initialized_.load()) {
        spdlog::error("MediaHub: Not initialized");
        return false;
    }

    if (running_.load()) {
        spdlog::warn("MediaHub: Already running");
        return true;
    }

    spdlog::info("MediaHub: Starting producer threads...");

    auto& pipeline = platform::HdalPipeline::Instance();
    if (!pipeline.IsRunning()) {
        spdlog::error("MediaHub: HDAL pipeline is not running");
        return false;
    }

    int started = 0;
    for (int i = 0; i < kMaxVideoChannels; ++i) {
        if (!channels_[i].enabled) continue;

        if (!SetupChannelHdal(i)) {
            spdlog::error("MediaHub: Failed to setup HDAL for channel {}", i);
            continue;
        }

        channels_[i].stop_requested.store(false);
        channels_[i].producer_thread = std::thread(&MediaHub::ProducerThread, this, i);
        started++;
        spdlog::info("MediaHub: Producer thread started for channel {}", i);
    }

    running_.store(true, std::memory_order_release);
    spdlog::info("MediaHub: Started {} producer threads", started);
    return started > 0;
}

void MediaHub::Stop() {
    // Always signal stop and join threads regardless of running_ state.
    // This prevents producer threads from outliving a pipeline Shutdown()
    // which destroys VB memory pools they reference (kernel oops).
    bool was_running = running_.exchange(false, std::memory_order_release);

    if (was_running) {
        spdlog::info("MediaHub: Stopping producer threads...");
    }

    for (int i = 0; i < kMaxVideoChannels; ++i) {
        channels_[i].stop_requested.store(true, std::memory_order_release);

        // Wake up any waiting consumers
        channels_[i].frame_cv.notify_all();

        if (channels_[i].producer_thread.joinable()) {
            channels_[i].producer_thread.join();
            if (was_running) {
                spdlog::info("MediaHub: Producer thread stopped for channel {}", i);
            }
        }

        CleanupChannelHdal(i);
    }

    if (was_running) {
        spdlog::info("MediaHub: All producer threads stopped");
    }
}

void MediaHub::Shutdown() {
    Stop();

    for (int i = 0; i < kMaxVideoChannels; ++i) {
        channels_[i].ring_buffer.reset();
        channels_[i].enabled = false;
    }

    initialized_.store(false, std::memory_order_release);
    spdlog::info("MediaHub: Shutdown complete");
}

bool MediaHub::Reinitialize() {
    spdlog::info("MediaHub: Reinitializing...");
    Stop();

    // Reset ring buffers (clear old data)
    for (int i = 0; i < kMaxVideoChannels; ++i) {
        channels_[i].ring_buffer.reset();
        channels_[i].enabled = false;
        channels_[i].paused.store(false);
        channels_[i].frames_produced.store(0);
        channels_[i].bytes_produced.store(0);
        channels_[i].keyframes_produced.store(0);
        channels_[i].pull_errors.store(0);
        channels_[i].oversize_frames.store(0);
        channels_[i].cached_sps_size = 0;
        channels_[i].cached_pps_size = 0;
        channels_[i].cached_vps_size = 0;
    }

    initialized_.store(false, std::memory_order_release);

    if (!Initialize()) {
        spdlog::error("MediaHub: Reinitialize failed");
        return false;
    }

    return Start();
}

void MediaHub::NotifyCodecChange(int channel_id, VideoCodecType new_codec) {
    if (channel_id < 0 || channel_id >= kMaxVideoChannels) {
        spdlog::error("MediaHub: Invalid channel_id {} for codec change", channel_id);
        return;
    }

    auto& ch = channels_[channel_id];
    if (!ch.enabled) {
        spdlog::warn("MediaHub: Channel {} not enabled, ignoring codec change", channel_id);
        return;
    }

    std::lock_guard<std::mutex> lock(ch.frame_mutex);

    spdlog::info("MediaHub: Channel {} codec changing from {} to {}",
                 channel_id, VideoCodecTypeToString(ch.cached_codec),
                 VideoCodecTypeToString(new_codec));

    // Clear cached parameters (will be repopulated from next keyframe)
    ch.cached_sps_size = 0;
    ch.cached_pps_size = 0;
    ch.cached_vps_size = 0;
    std::memset(ch.cached_sps, 0, sizeof(ch.cached_sps));
    std::memset(ch.cached_pps, 0, sizeof(ch.cached_pps));
    std::memset(ch.cached_vps, 0, sizeof(ch.cached_vps));

    // Update cached codec
    ch.cached_codec = new_codec;

    spdlog::info("MediaHub: Channel {} codec parameters cleared, waiting for new keyframe", channel_id);
}

void MediaHub::PauseChannel(int channel_id) {
    if (channel_id < 0 || channel_id >= kMaxVideoChannels) {
        spdlog::error("MediaHub::PauseChannel: Invalid channel_id {}", channel_id);
        return;
    }

    auto& ch = channels_[channel_id];
    if (!ch.enabled) {
        spdlog::warn("MediaHub::PauseChannel: Channel {} not enabled", channel_id);
        return;
    }

    spdlog::info("MediaHub: Pausing producer for channel {}", channel_id);
    ch.paused.store(true, std::memory_order_release);

    // Wait briefly for producer loop to notice the pause
    // (max wait = pull timeout + margin)
    std::this_thread::sleep_for(std::chrono::milliseconds(kVideoPullTimeoutMs + 50));
    spdlog::debug("MediaHub: Channel {} producer paused", channel_id);
}

void MediaHub::ResumeChannel(int channel_id, bool codec_changed) {
    if (channel_id < 0 || channel_id >= kMaxVideoChannels) {
        spdlog::error("MediaHub::ResumeChannel: Invalid channel_id {}", channel_id);
        return;
    }

    auto& ch = channels_[channel_id];
    if (!ch.enabled) {
        spdlog::warn("MediaHub::ResumeChannel: Channel {} not enabled", channel_id);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(ch.frame_mutex);

        // Clear cached SPS/PPS/VPS so they are re-extracted from first new keyframe
        ch.cached_sps_size = 0;
        ch.cached_pps_size = 0;
        ch.cached_vps_size = 0;
        std::memset(ch.cached_sps, 0, sizeof(ch.cached_sps));
        std::memset(ch.cached_pps, 0, sizeof(ch.cached_pps));
        std::memset(ch.cached_vps, 0, sizeof(ch.cached_vps));

        // Update codec type if it changed
        if (codec_changed) {
            VideoCodecType new_codec = GetChannelCodec(channel_id);
            spdlog::info("MediaHub: Channel {} codec updated to {}", channel_id,
                         VideoCodecTypeToString(new_codec));
            ch.cached_codec = new_codec;
        }
    }

    // Flush stale frames from encoder output buffer
    FlushEncoderBuffer(channel_id);

#if HDAL_PIPELINE_ENABLED
    // Request an IDR frame so consumers get clean SPS/PPS
    {
        HD_PATH_ID enc_path = static_cast<HD_PATH_ID>(ch.enc_path);
        if (enc_path != 0) {
            HD_H26XENC_REQUEST_IFRAME req;
            memset(&req, 0, sizeof(req));
            req.enable = TRUE;
            HD_RESULT ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, &req);
            if (ret != HD_OK) {
                spdlog::warn("MediaHub: Channel {} failed to request IDR after resume: {}",
                             channel_id, static_cast<int>(ret));
            }
        }
    }
#endif

    // Resume the producer
    ch.paused.store(false, std::memory_order_release);
    spdlog::info("MediaHub: Channel {} producer resumed (SPS/PPS cleared, IDR requested)", channel_id);
}

std::unique_ptr<VideoFrameConsumer> MediaHub::CreateConsumer(int channel_id,
                                                              const std::string& name) {
    if (channel_id < 0 || channel_id >= kMaxVideoChannels) {
        spdlog::error("MediaHub: Invalid channel_id {} for consumer '{}'", channel_id, name);
        return nullptr;
    }

    if (!channels_[channel_id].enabled || !channels_[channel_id].ring_buffer) {
        spdlog::error("MediaHub: Channel {} not active for consumer '{}'", channel_id, name);
        return nullptr;
    }

    uint32_t id = next_consumer_id_.fetch_add(1);
    channels_[channel_id].consumer_count.fetch_add(1);

    auto consumer = std::make_unique<VideoFrameConsumer>(
        channels_[channel_id].ring_buffer.get(),
        &channels_[channel_id].frame_cv,
        &channels_[channel_id].frame_mutex,
        id,
        channel_id,
        name);

    spdlog::info("MediaHub: Created consumer '{}' (id={}) for channel {} "
                 "(total consumers: {})",
                 name, id, channel_id,
                 channels_[channel_id].consumer_count.load());

    return consumer;
}

int MediaHub::GetActiveChannelCount() const {
    int count = 0;
    for (int i = 0; i < kMaxVideoChannels; ++i) {
        if (channels_[i].enabled) count++;
    }
    return count;
}

bool MediaHub::IsChannelActive(int channel_id) const {
    if (channel_id < 0 || channel_id >= kMaxVideoChannels) return false;
    return channels_[channel_id].enabled;
}

const VideoRingBuffer* MediaHub::GetRingBuffer(int channel_id) const {
    if (channel_id < 0 || channel_id >= kMaxVideoChannels) return nullptr;
    return channels_[channel_id].ring_buffer.get();
}

MediaHub::ChannelStats MediaHub::GetChannelStats(int channel_id) const {
    ChannelStats stats;
    if (channel_id < 0 || channel_id >= kMaxVideoChannels) return stats;

    const auto& ch = channels_[channel_id];
    stats.frames_produced = ch.frames_produced.load();
    stats.bytes_produced = ch.bytes_produced.load();
    stats.keyframes_produced = ch.keyframes_produced.load();
    stats.pull_errors = ch.pull_errors.load();
    stats.oversize_frames = ch.oversize_frames.load();
    stats.consumer_count = ch.consumer_count.load();
    if (ch.ring_buffer) {
        stats.latest_sequence = ch.ring_buffer->GetLatestSequence();
    }
    if (stats.frames_produced > 0) {
        stats.avg_frame_size = static_cast<double>(stats.bytes_produced) / stats.frames_produced;
    }
    return stats;
}

bool MediaHub::WaitForFrame(int channel_id, int timeout_ms) {
    if (channel_id < 0 || channel_id >= kMaxVideoChannels) return false;
    if (!channels_[channel_id].enabled) return false;

    std::unique_lock<std::mutex> lock(channels_[channel_id].frame_mutex);
    return channels_[channel_id].frame_cv.wait_for(
        lock, std::chrono::milliseconds(timeout_ms)) != std::cv_status::timeout;
}

// ============================================================================
// HDAL Integration (conditional compilation)
// ============================================================================

bool MediaHub::SetupChannelHdal(int channel_id) {
#if HDAL_PIPELINE_ENABLED
    auto& pipeline = platform::HdalPipeline::Instance();
    auto& ch = channels_[channel_id];

    ch.enc_path = pipeline.GetVideoEncoderPath(channel_id);
    if (ch.enc_path == 0) {
        spdlog::error("MediaHub: Invalid encoder path for channel {}", channel_id);
        return false;
    }

    // Ensure encoder is running
    if (!pipeline.IsEncoderRunning(channel_id)) {
        spdlog::warn("MediaHub: Encoder not running for channel {}, starting...", channel_id);
        if (!pipeline.EnsureEncoderRunning(channel_id)) {
            spdlog::error("MediaHub: Failed to start encoder for channel {}", channel_id);
            return false;
        }
        usleep(50000);  // 50ms stabilization
    }

    // Get buffer info
    HD_VIDEOENC_BUFINFO phy_buf = {0};
    HD_PATH_ID enc_path = static_cast<HD_PATH_ID>(ch.enc_path);

    if (hd_videoenc_get(enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf) != HD_OK) {
        spdlog::error("MediaHub: Failed to get encoder buffer info for channel {}", channel_id);
        return false;
    }

    // Memory map the encoder output buffer
    UINTPTR vir_addr = (UINTPTR)hd_common_mem_mmap(
        HD_COMMON_MEM_MEM_TYPE_CACHE,
        phy_buf.buf_info.phy_addr,
        phy_buf.buf_info.buf_size);

    if (vir_addr == 0) {
        spdlog::error("MediaHub: Failed to mmap encoder buffer for channel {}", channel_id);
        return false;
    }

    ch.vir_addr = vir_addr;
    ch.phy_addr = phy_buf.buf_info.phy_addr;
    ch.buf_size = phy_buf.buf_info.buf_size;
    ch.buffer_mapped = true;
    ch.cached_codec = GetChannelCodec(channel_id);

    spdlog::info("MediaHub: Channel {} HDAL setup complete (enc_path={}, buf_size={})",
                 channel_id, ch.enc_path, ch.buf_size);
    return true;
#else
    (void)channel_id;
    spdlog::warn("MediaHub: HDAL not enabled, channel {} skipped", channel_id);
    return false;
#endif
}

void MediaHub::CleanupChannelHdal(int channel_id) {
#if HDAL_PIPELINE_ENABLED
    auto& ch = channels_[channel_id];
    if (ch.buffer_mapped && ch.vir_addr != 0) {
        hd_common_mem_munmap((void*)ch.vir_addr, ch.buf_size);
        ch.buffer_mapped = false;
        ch.vir_addr = 0;
        spdlog::debug("MediaHub: Channel {} HDAL buffer unmapped", channel_id);
    }
    ch.enc_path = 0;
#else
    (void)channel_id;
#endif
}

void MediaHub::FlushEncoderBuffer(int channel_id) {
#if HDAL_PIPELINE_ENABLED
    auto& ch = channels_[channel_id];
    if (!ch.buffer_mapped || ch.enc_path == 0) return;

    HD_PATH_ID enc_path = static_cast<HD_PATH_ID>(ch.enc_path);
    HD_VIDEOENC_BS data_pull;
    int flushed = 0;
    constexpr int kMaxFlushFrames = 300;
    int consecutive_empty = 0;

    while (flushed < kMaxFlushFrames && consecutive_empty < 3) {
        memset(&data_pull, 0, sizeof(data_pull));
        HD_RESULT ret = hd_videoenc_pull_out_buf(enc_path, &data_pull, 0);
        if (ret != HD_OK) {
            consecutive_empty++;
            if (consecutive_empty < 3) usleep(2000);
            continue;
        }
        consecutive_empty = 0;
        hd_videoenc_release_out_buf(enc_path, &data_pull);
        flushed++;
        usleep(1000);
    }

    if (flushed > 0) {
        spdlog::info("MediaHub: Flushed {} old frames from channel {}", flushed, channel_id);
    }
#else
    (void)channel_id;
#endif
}

void MediaHub::ProducerThread(int channel_id) {
#if HDAL_PIPELINE_ENABLED
    auto& ch = channels_[channel_id];

    spdlog::info("MediaHub: Producer thread running for channel {}", channel_id);

    // Flush stale frames from encoder buffer
    FlushEncoderBuffer(channel_id);

    // Request an IDR frame to start clean
    {
        HD_PATH_ID enc_path = static_cast<HD_PATH_ID>(ch.enc_path);
        HD_H26XENC_REQUEST_IFRAME req;
        req.enable = TRUE;
        hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, &req);
    }

    HD_PATH_ID enc_path = static_cast<HD_PATH_ID>(ch.enc_path);
    HD_VIDEOENC_BS data_pull;

    int frame_count = 0;
    int consecutive_errors = 0;
    while (!ch.stop_requested.load(std::memory_order_acquire)) {
        // If paused (encoder params being changed), spin-wait
        if (ch.paused.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        memset(&data_pull, 0, sizeof(data_pull));

        HD_RESULT ret = hd_videoenc_pull_out_buf(enc_path, &data_pull, kVideoPullTimeoutMs);

        if (ret != HD_OK) {
            if (ret != HD_ERR_TIMEDOUT) {
                ch.pull_errors.fetch_add(1);
                consecutive_errors++;
                // Throttle error logs
                if (ch.pull_errors.load() % 100 == 1) {
                    spdlog::warn("MediaHub: Channel {} pull error: {} (total: {}, consec: {})", 
                                 channel_id, static_cast<int>(ret), ch.pull_errors.load(),
                                 consecutive_errors);
                }
                // Safety net: self-terminate if encoder path is gone
                // (pipeline Shutdown destroyed it). Prevents spinning on
                // freed VB pool memory which causes kernel oops.
                if (consecutive_errors > 200) {
                    spdlog::error("MediaHub: Channel {} self-terminating after {} consecutive pull errors",
                                  channel_id, consecutive_errors);
                    break;
                }
            }
            continue;
        }
        consecutive_errors = 0;  // Reset on successful pull

        if (data_pull.pack_num == 0) {
            hd_videoenc_release_out_buf(enc_path, &data_pull);
            continue;
        }

        // Log first few frames to confirm data flow
        if (frame_count < 5) {
            spdlog::info("MediaHub: Channel {} recv frame {}, size={}, packs={}, ts={}", 
                         channel_id, frame_count, 
                         data_pull.video_pack[0].size, data_pull.pack_num, 
                         data_pull.timestamp);
        }
        frame_count++;


        uint64_t hw_timestamp = data_pull.timestamp;
        uint64_t capture_time = GetCurrentTimeMs();

        // Assemble all NAL packs into a single frame buffer
        // Calculate total size first
        uint32_t total_size = 0;
        for (uint32_t j = 0; j < data_pull.pack_num; ++j) {
            total_size += data_pull.video_pack[j].size;
        }

        if (total_size > ch.ring_buffer->GetMaxFrameSize()) {
            ch.oversize_frames.fetch_add(1);
            spdlog::warn("MediaHub: Channel {} frame too large ({} bytes, max {})",
                         channel_id, total_size, ch.ring_buffer->GetMaxFrameSize());
            hd_videoenc_release_out_buf(enc_path, &data_pull);
            continue;
        }

        // Build frame data and NAL pack info
        // Use a thread-local buffer to avoid repeated allocation
        thread_local std::vector<uint8_t> frame_buffer;
        frame_buffer.clear();
        frame_buffer.reserve(total_size);

        std::vector<VideoFrame::NalPack> nal_packs;
        nal_packs.reserve(data_pull.pack_num);

        bool frame_is_keyframe = false;
        VideoCodecType frame_codec = ch.cached_codec;

        for (uint32_t j = 0; j < data_pull.pack_num; ++j) {
            uint8_t* ptr = (uint8_t*)(ch.vir_addr +
                (data_pull.video_pack[j].phy_addr - ch.phy_addr));
            uint32_t len = data_pull.video_pack[j].size;

            VideoFrame::NalPack pack;
            pack.offset = static_cast<uint32_t>(frame_buffer.size());
            pack.size = len;

            // Check if this NAL is an IDR
            bool nal_is_idr = false;
            if (frame_codec == VideoCodecType::kH265) {
                // H.265: check NALU type
                nal_is_idr = (data_pull.video_pack[j].pack_type.h265_type >= 16 &&
                              data_pull.video_pack[j].pack_type.h265_type <= 21);
            } else {
                // H.264: check for IDR type
                nal_is_idr = (data_pull.video_pack[j].pack_type.h264_type == 5);  // H264_NALU_TYPE_IDR
            }

            pack.is_idr = nal_is_idr;
            if (nal_is_idr) frame_is_keyframe = true;

            // Append NAL data
            frame_buffer.insert(frame_buffer.end(), ptr, ptr + len);
            nal_packs.push_back(pack);
        }

        // Try to force early parameter extraction if we don't have them
        if (ch.cached_sps_size == 0 || ch.cached_pps_size == 0) {
            ExtractNalParameters(frame_buffer.data(), total_size, frame_codec,
                                 ch.cached_sps, ch.cached_sps_size,
                                 ch.cached_pps, ch.cached_pps_size,
                                 ch.cached_vps, ch.cached_vps_size);
            
            if (ch.cached_sps_size > 0 && ch.cached_pps_size > 0) {
                 spdlog::info("MediaHub: Channel {} discovered SPS/PPS in frame {} (size: {}/{})", 
                              channel_id, frame_count, ch.cached_sps_size, ch.cached_pps_size);
            }
        }
        
        // Update cached status on Keyframe
        if (frame_is_keyframe) {
            bool found = ExtractNalParameters(frame_buffer.data(), total_size, frame_codec,
                                 ch.cached_sps, ch.cached_sps_size,
                                 ch.cached_pps, ch.cached_pps_size,
                                 ch.cached_vps, ch.cached_vps_size);
            if (found) {
                ch.keyframes_produced.fetch_add(1);
            }
        }

        // Write to ring buffer
        ch.ring_buffer->Write(
            frame_buffer.data(), total_size,
            hw_timestamp, capture_time,
            channel_id, frame_is_keyframe, frame_codec,
            ch.cached_sps, ch.cached_sps_size,
            ch.cached_pps, ch.cached_pps_size,
            ch.cached_vps, ch.cached_vps_size,
            nal_packs);

        // Release HDAL buffer immediately after copy
        hd_videoenc_release_out_buf(enc_path, &data_pull);

        // Update stats
        ch.frames_produced.fetch_add(1);
        ch.bytes_produced.fetch_add(total_size);

        // Notify waiting consumers
        ch.frame_cv.notify_all();
    }

    spdlog::info("MediaHub: Producer thread exiting for channel {}", channel_id);
#else
    (void)channel_id;
    spdlog::warn("MediaHub: HDAL not enabled, producer thread exiting");
#endif
}

// ============================================================================
// NAL Parsing Helpers
// ============================================================================

bool MediaHub::ExtractNalParameters(const uint8_t* data, uint32_t size,
                                     VideoCodecType codec,
                                     uint8_t* sps, uint16_t& sps_size,
                                     uint8_t* pps, uint16_t& pps_size,
                                     uint8_t* vps, uint16_t& vps_size) {
    if (!data || size < 5) return false;

    const uint8_t* ptr = data;
    const uint8_t* end = data + size;
    bool found_params = false;

    while (ptr < end - 4) {
        // Find start code
        const uint8_t* nal_start = nullptr;
        if (ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 0 && ptr[3] == 1) {
            nal_start = ptr + 4;
        } else if (ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 1) {
            nal_start = ptr + 3;
        } else {
            ptr++;
            continue;
        }

        if (nal_start >= end) break;

        // Find end of this NAL
        const uint8_t* nal_end = nal_start;
        while (nal_end < end - 3) {
            if ((nal_end[0] == 0 && nal_end[1] == 0 && nal_end[2] == 0 && nal_end[3] == 1) ||
                (nal_end[0] == 0 && nal_end[1] == 0 && nal_end[2] == 1)) {
                break;
            }
            nal_end++;
        }
        if (nal_end >= end - 3) nal_end = end;

        uint32_t nal_size = static_cast<uint32_t>(nal_end - nal_start);
        if (nal_size == 0) {
            ptr = nal_start;
            continue;
        }

        if (codec == VideoCodecType::kH264) {
            uint8_t nal_type = nal_start[0] & 0x1F;
            if (nal_type == 7 && nal_size <= kMaxNalParamSize) {  // SPS
                std::memcpy(sps, nal_start, nal_size);
                sps_size = static_cast<uint16_t>(nal_size);
                found_params = true;
            } else if (nal_type == 8 && nal_size <= kMaxNalParamSize) {  // PPS
                std::memcpy(pps, nal_start, nal_size);
                pps_size = static_cast<uint16_t>(nal_size);
                found_params = true;
            }
        } else if (codec == VideoCodecType::kH265) {
            uint8_t nal_type = (nal_start[0] >> 1) & 0x3F;
            if (nal_type == 32 && nal_size <= kMaxNalParamSize) {  // VPS
                std::memcpy(vps, nal_start, nal_size);
                vps_size = static_cast<uint16_t>(nal_size);
                found_params = true;
            } else if (nal_type == 33 && nal_size <= kMaxNalParamSize) {  // SPS
                std::memcpy(sps, nal_start, nal_size);
                sps_size = static_cast<uint16_t>(nal_size);
                found_params = true;
            } else if (nal_type == 34 && nal_size <= kMaxNalParamSize) {  // PPS
                std::memcpy(pps, nal_start, nal_size);
                pps_size = static_cast<uint16_t>(nal_size);
                found_params = true;
            }
        }

        ptr = nal_end;
    }

    return found_params;
}

bool MediaHub::IsIdrFrame(const uint8_t* data, uint32_t size, VideoCodecType codec) {
    if (size < 5) return false;

    int offset = 0;
    if (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 1) {
        offset = 4;
    } else if (data[0] == 0 && data[1] == 0 && data[2] == 1) {
        offset = 3;
    }

    if (size <= static_cast<uint32_t>(offset)) return false;

    uint8_t nal_byte = data[offset];
    if (codec == VideoCodecType::kH265) {
        uint8_t nal_type = (nal_byte >> 1) & 0x3F;
        return (nal_type >= 16 && nal_type <= 21);
    } else {
        uint8_t nal_type = nal_byte & 0x1F;
        return (nal_type == 5);
    }
}

VideoCodecType MediaHub::GetChannelCodec(int channel_id) const {
    std::string prefix = "media.video" + std::to_string(channel_id + 1);
    std::string codec_str = config::Get<std::string>(prefix + ".codec", "h264");

    if (codec_str == "h265" || codec_str == "hevc" || codec_str == "H265") {
        return VideoCodecType::kH265;
    } else if (codec_str == "mjpeg" || codec_str == "jpeg") {
        return VideoCodecType::kMJPEG;
    }
    return VideoCodecType::kH264;
}

uint64_t MediaHub::GetCurrentTimeMs() {
    auto now = std::chrono::system_clock::now();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count());
}

} // namespace media
} // namespace ipcam
