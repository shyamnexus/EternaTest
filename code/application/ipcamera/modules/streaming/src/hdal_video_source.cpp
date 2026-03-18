/**
 * @file hdal_video_source.cpp
 * @brief Video Source implementation for Live555 RTSP server
 *
 * Consumes encoded video frames from the MediaHub ring buffer.
 * The MediaHub producer handles all HDAL interaction - this source
 * simply reads frames and delivers them to Live555 for RTP packetization.
 *
 * Architecture:
 *   HDAL Encoder -> MediaHub Producer -> Ring Buffer -> [This Source] -> Live555 -> RTP
 */

#include "ipcam/rtsp_sources_internal.h"

#ifndef RTSP_SERVER_ENABLED
#define RTSP_SERVER_ENABLED 1
#endif

#if RTSP_SERVER_ENABLED

namespace ipcam {
namespace streaming {

// Helper: Check if frame codec matches expected RTSP codec
static bool FrameCodecMatchesRtsp(media::VideoCodecType frame_codec, RtspVideoCodec rtsp_codec) {
    if (rtsp_codec == RtspVideoCodec::kH265)
        return frame_codec == media::VideoCodecType::kH265;
    return frame_codec == media::VideoCodecType::kH264;
}

// ============================================================================
// HdalVideoSource Implementation (MediaHub consumer)
// ============================================================================

HdalVideoSource* HdalVideoSource::createNew(UsageEnvironment& env,
                                            int channel_id,
                                            RtspVideoCodec codec) {
    return new HdalVideoSource(env, channel_id, codec);
}

HdalVideoSource::HdalVideoSource(UsageEnvironment& env,
                                 int channel_id,
                                 RtspVideoCodec codec)
    : FramedSource(env)
    , channel_id_(channel_id)
    , codec_(codec)
    , got_idr_(false)
    , sps_size_(0)
    , pps_size_(0)
    , vps_size_(0)
    , have_parameters_(false)
    , base_hw_timestamp_(0)
    , last_hw_timestamp_(0)
    , have_base_time_(false)
    , codec_check_counter_(0) {

    memset(&base_presentation_time_, 0, sizeof(base_presentation_time_));

    // Create a MediaHub consumer for this channel
    auto& hub = media::MediaHub::Instance();
    if (hub.IsRunning() && hub.IsChannelActive(channel_id)) {
        consumer_ = hub.CreateConsumer(channel_id,
            "rtsp-ch" + std::to_string(channel_id));
        if (consumer_) {
            spdlog::info("HdalVideoSource: Created MediaHub consumer for channel {} (id={})",
                         channel_id_, consumer_->GetId());
        } else {
            spdlog::error("HdalVideoSource: Failed to create MediaHub consumer for channel {}",
                          channel_id_);
        }
    } else {
        spdlog::error("HdalVideoSource: MediaHub not running or channel {} not active",
                      channel_id_);
    }

    // Create event trigger for Live555 async delivery
    event_trigger_id_ = envir().taskScheduler().createEventTrigger(deliverFrameCallback);
}

HdalVideoSource::~HdalVideoSource() {
    // Cancel pending tasks
    if (nextTask() != NULL) {
        envir().taskScheduler().unscheduleDelayedTask(nextTask());
    }

    // Delete event trigger
    if (event_trigger_id_ != 0) {
        envir().taskScheduler().deleteEventTrigger(event_trigger_id_);
        event_trigger_id_ = 0;
    }

    // Consumer is automatically destroyed via unique_ptr
    if (consumer_) {
        spdlog::info("HdalVideoSource: Destroying consumer for channel {} "
                     "(consumed={}, dropped={})",
                     channel_id_,
                     consumer_->GetConsumedFrameCount(),
                     consumer_->GetDroppedFrameCount());
    }
}

bool HdalVideoSource::getSpsPpsVps(u_int8_t*& sps, unsigned& spsSize,
                                   u_int8_t*& pps, unsigned& ppsSize,
                                   u_int8_t*& vps, unsigned& vpsSize) {
    if (!have_parameters_ && consumer_) {
        // Try to read frames from the ring buffer to get NAL parameters
        // IMPORTANT: After a codec change the ring buffer may still contain
        // frames from the old codec.  Only accept parameters from frames
        // whose codec matches the one we were created with.
        media::VideoFrame frame;
        for (int i = 0; i < 90; ++i) {  // Wait up to ~3 seconds
            if (consumer_->WaitForFrame(frame, 33)) {
                // Skip frames from the wrong codec (stale pre-change data)
                if (!FrameCodecMatchesRtsp(frame.codec, codec_)) {
                    continue;
                }

                if (frame.sps_size > 0 && frame.sps_size <= sizeof(sps_)) {
                    memcpy(sps_, frame.sps, frame.sps_size);
                    sps_size_ = frame.sps_size;
                }
                if (frame.pps_size > 0 && frame.pps_size <= sizeof(pps_)) {
                    memcpy(pps_, frame.pps, frame.pps_size);
                    pps_size_ = frame.pps_size;
                }
                if (frame.vps_size > 0 && frame.vps_size <= sizeof(vps_)) {
                    memcpy(vps_, frame.vps, frame.vps_size);
                    vps_size_ = frame.vps_size;
                }

                if (codec_ == RtspVideoCodec::kH265) {
                    if (vps_size_ > 0 && sps_size_ > 0 && pps_size_ > 0) {
                        have_parameters_ = true;
                        break;
                    }
                } else {
                    if (sps_size_ > 0 && pps_size_ > 0) {
                        have_parameters_ = true;
                        break;
                    }
                }
            }
        }

        // Reset consumer to next keyframe after parameter extraction
        if (have_parameters_) {
            consumer_->ResetToKeyframe();
        }
    }

    if (have_parameters_) {
        sps = sps_;
        spsSize = sps_size_;
        pps = pps_;
        ppsSize = pps_size_;
        vps = vps_;
        vpsSize = vps_size_;
        return true;
    }

    sps = pps = vps = nullptr;
    spsSize = ppsSize = vpsSize = 0;
    return false;
}

RtspVideoCodec HdalVideoSource::getCurrentCodecFromConfig() const {
    std::string prefix = "media.video" + std::to_string(channel_id_ + 1);
    std::string codec_str = config::Get<std::string>(prefix + ".codec", "h264");

    if (codec_str == "h265" || codec_str == "hevc") {
        return RtspVideoCodec::kH265;
    }
    return RtspVideoCodec::kH264;
}

bool HdalVideoSource::checkAndHandleCodecChange() {
    if (++codec_check_counter_ < kCodecCheckInterval) {
        return false;
    }
    codec_check_counter_ = 0;

    RtspVideoCodec config_codec = getCurrentCodecFromConfig();

    if (config_codec != codec_) {
        spdlog::info("HdalVideoSource: Codec changed for channel {} ({} -> {}), closing source",
                     channel_id_,
                     codec_ == RtspVideoCodec::kH265 ? "H.265" : "H.264",
                     config_codec == RtspVideoCodec::kH265 ? "H.265" : "H.264");

        handleClosure();
        return true;
    }

    return false;
}

void HdalVideoSource::doGetNextFrame() {
    envir().taskScheduler().triggerEvent(event_trigger_id_, this);
}

void HdalVideoSource::deliverFrameCallback(void* clientData) {
    static_cast<HdalVideoSource*>(clientData)->deliverFrame();
}

void HdalVideoSource::deliverFrame() {
    if (!isCurrentlyAwaitingData()) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            5000, (TaskFunc*)deliverFrameCallback, this);
        return;
    }

    // NOTE: Do NOT call checkAndHandleCodecChange() here.
    // It calls handleClosure() which cascades through Live555 and may delete
    // 'this' (via OnDemandServerMediaSubsession::closeStreamSource), causing
    // a use-after-free when we access this->nextTask() afterwards.
    // Codec changes are properly handled by:
    //   1. Frame-level codec mismatch check (FrameCodecMatchesRtsp) below
    //   2. ApplyToHardware -> DoRefreshStream which cleanly tears down and
    //      recreates the RTSP session with the correct codec/framer.

    // Ensure we have a valid MediaHub consumer
    if (!consumer_) {
        // Try to recreate consumer
        auto& hub = media::MediaHub::Instance();
        if (hub.IsRunning() && hub.IsChannelActive(channel_id_)) {
            consumer_ = hub.CreateConsumer(channel_id_,
                "rtsp-ch" + std::to_string(channel_id_));
        }
        if (!consumer_) {
            spdlog::error("HdalVideoSource: No MediaHub consumer for channel {}", channel_id_);
            handleClosure();
            return;
        }
    }

    // Read next frame from the ring buffer
    media::VideoFrame frame;
    if (!consumer_->WaitForFrame(frame, kFramePullTimeoutMs)) {
        // No frame available yet, retry
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            0, (TaskFunc*)deliverFrameCallback, this);
        return;
    }

    // Skip frames from wrong codec (stale pre-change data in ring buffer)
    if (!FrameCodecMatchesRtsp(frame.codec, codec_)) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            0, (TaskFunc*)deliverFrameCallback, this);
        return;
    }

    // Skip non-IDR frames until we get the first IDR
    if (!got_idr_) {
        if (frame.is_keyframe) {
            got_idr_ = true;
        } else {
            // Skip this frame, wait for IDR
            nextTask() = envir().taskScheduler().scheduleDelayedTask(
                0, (TaskFunc*)deliverFrameCallback, this);
            return;
        }
    }

    // Update NAL parameters from frame (if available)
    if (frame.sps_size > 0 && frame.sps_size <= sizeof(sps_)) {
        memcpy(sps_, frame.sps, frame.sps_size);
        sps_size_ = frame.sps_size;
        have_parameters_ = true;
    }
    if (frame.pps_size > 0 && frame.pps_size <= sizeof(pps_)) {
        memcpy(pps_, frame.pps, frame.pps_size);
        pps_size_ = frame.pps_size;
    }
    if (frame.vps_size > 0 && frame.vps_size <= sizeof(vps_)) {
        memcpy(vps_, frame.vps, frame.vps_size);
        vps_size_ = frame.vps_size;
    }

    // The frame data contains the full encoded NAL units.
    // Live555 expects individual NAL units without start codes.
    // Use the LAST NAL pack (slice data) for delivery.
    const uint8_t* src_data = frame.data.data();
    uint32_t src_size = static_cast<uint32_t>(frame.data.size());

    // If we have NAL pack info, use the last pack (the slice)
    if (!frame.nal_packs.empty()) {
        auto& last_pack = frame.nal_packs.back();
        if (last_pack.offset + last_pack.size <= frame.data.size()) {
            src_data = frame.data.data() + last_pack.offset;
            src_size = last_pack.size;
        }
    }

    // Strip start code prefix (0x00000001 or 0x000001)
    const uint8_t* nal_data = src_data;
    uint32_t nal_size = src_size;

    if (src_size >= 4 && src_data[0] == 0 && src_data[1] == 0 &&
        src_data[2] == 0 && src_data[3] == 1) {
        nal_data = src_data + 4;
        nal_size = src_size - 4;
    } else if (src_size >= 3 && src_data[0] == 0 && src_data[1] == 0 && src_data[2] == 1) {
        nal_data = src_data + 3;
        nal_size = src_size - 3;
    }

    // Copy to Live555 output buffer
    if (nal_size > fMaxSize) {
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = nal_size - fMaxSize;
        spdlog::warn("HdalVideoSource: Frame truncated ({} > {})", nal_size, fMaxSize);
    } else {
        fFrameSize = nal_size;
        fNumTruncatedBytes = 0;
    }

    memcpy(fTo, nal_data, fFrameSize);

    // Set presentation time from hardware timestamp
    uint64_t hw_timestamp = frame.timestamp_us;

    if (!have_base_time_) {
        gettimeofday(&fPresentationTime, nullptr);
        base_presentation_time_ = fPresentationTime;
        base_hw_timestamp_ = hw_timestamp;
        last_hw_timestamp_ = hw_timestamp;
        have_base_time_ = true;
        fDurationInMicroseconds = 33333;
        spdlog::debug("HdalVideoSource[{}]: Base time set, hw_ts={}", channel_id_, hw_timestamp);
    } else {
        int64_t delta_from_last = static_cast<int64_t>(hw_timestamp) - static_cast<int64_t>(last_hw_timestamp_);
        int64_t delta_from_base = static_cast<int64_t>(hw_timestamp) - static_cast<int64_t>(base_hw_timestamp_);

        bool need_reset = false;
        if (delta_from_last < 0 || delta_from_last > 1000000) {
            need_reset = true;
        } else if (delta_from_base < 0 || delta_from_base > kMaxPresentationDeltaUs) {
            need_reset = true;
        }

        if (need_reset) {
            gettimeofday(&fPresentationTime, nullptr);
            base_presentation_time_ = fPresentationTime;
            base_hw_timestamp_ = hw_timestamp;
            last_hw_timestamp_ = hw_timestamp;
            fDurationInMicroseconds = 33333;
        } else {
            uint64_t delta_us = static_cast<uint64_t>(delta_from_base);
            fPresentationTime.tv_sec = base_presentation_time_.tv_sec + static_cast<long>(delta_us / 1000000);
            fPresentationTime.tv_usec = base_presentation_time_.tv_usec + static_cast<long>(delta_us % 1000000);
            if (fPresentationTime.tv_usec >= 1000000) {
                fPresentationTime.tv_sec += fPresentationTime.tv_usec / 1000000;
                fPresentationTime.tv_usec %= 1000000;
            }

            if (delta_from_last > 0 && delta_from_last < 200000) {
                fDurationInMicroseconds = static_cast<unsigned>(delta_from_last);
            } else {
                fDurationInMicroseconds = 33333;
            }
            last_hw_timestamp_ = hw_timestamp;
        }
    }

    afterGetting(this);
}

// ============================================================================
// HdalVideoSubsession Implementation
// ============================================================================

HdalVideoSubsession* HdalVideoSubsession::createNew(UsageEnvironment& env,
                                                    int channel_id,
                                                    RtspVideoCodec codec) {
    return new HdalVideoSubsession(env, channel_id, codec);
}

HdalVideoSubsession::HdalVideoSubsession(UsageEnvironment& env,
                                         int channel_id,
                                         RtspVideoCodec codec)
    : OnDemandServerMediaSubsession(env, True /* reuse source */)
    , channel_id_(channel_id)
    , codec_(codec)
    , sps_(nullptr)
    , sps_size_(0)
    , pps_(nullptr)
    , pps_size_(0)
    , vps_(nullptr)
    , vps_size_(0)
    , have_params_(false) {

    spdlog::debug("HdalVideoSubsession: Created for channel {} codec={}",
                  channel_id, codec == RtspVideoCodec::kH265 ? "H.265" : "H.264");

    fetchVideoParams();
}

void HdalVideoSubsession::fetchVideoParams() {
    // Create a temporary source to extract SPS/PPS/VPS from the ring buffer
    HdalVideoSource* temp_source = HdalVideoSource::createNew(envir(), channel_id_, codec_);
    if (!temp_source) {
        spdlog::warn("HdalVideoSubsession: Failed to create temp source for params");
        return;
    }

    u_int8_t* sps = nullptr;
    u_int8_t* pps = nullptr;
    u_int8_t* vps = nullptr;
    unsigned sps_size = 0, pps_size = 0, vps_size = 0;

    if (temp_source->getSpsPpsVps(sps, sps_size, pps, pps_size, vps, vps_size)) {
        clearParams();

        if (sps && sps_size > 0) {
            sps_ = new u_int8_t[sps_size];
            memcpy(sps_, sps, sps_size);
            sps_size_ = sps_size;
        }
        if (pps && pps_size > 0) {
            pps_ = new u_int8_t[pps_size];
            memcpy(pps_, pps, pps_size);
            pps_size_ = pps_size;
        }
        if (vps && vps_size > 0) {
            vps_ = new u_int8_t[vps_size];
            memcpy(vps_, vps, vps_size);
            vps_size_ = vps_size;
        }
        have_params_ = true;
        spdlog::debug("HdalVideoSubsession: Fetched params for channel {} ({}): VPS={} SPS={} PPS={}",
                     channel_id_, codec_ == RtspVideoCodec::kH265 ? "H.265" : "H.264",
                     vps_size_, sps_size_, pps_size_);
    } else {
        spdlog::warn("HdalVideoSubsession: Could not fetch VPS/SPS/PPS for channel {}", channel_id_);
    }

    Medium::close(temp_source);
}

void HdalVideoSubsession::clearParams() {
    delete[] sps_;
    sps_ = nullptr;
    sps_size_ = 0;

    delete[] pps_;
    pps_ = nullptr;
    pps_size_ = 0;

    delete[] vps_;
    vps_ = nullptr;
    vps_size_ = 0;

    have_params_ = false;
}

RtspVideoCodec HdalVideoSubsession::getCurrentCodecFromConfig() const {
    std::string prefix = "media.video" + std::to_string(channel_id_ + 1);
    std::string codec_str = config::Get<std::string>(prefix + ".codec", "h264");

    if (codec_str == "h265" || codec_str == "hevc") {
        return RtspVideoCodec::kH265;
    }
    return RtspVideoCodec::kH264;
}

void HdalVideoSubsession::refreshCodecIfChanged() {
    RtspVideoCodec current_codec = getCurrentCodecFromConfig();

    if (current_codec != codec_) {
        spdlog::info("HdalVideoSubsession: Codec changed for channel {} ({} -> {})",
                     channel_id_,
                     codec_ == RtspVideoCodec::kH265 ? "H.265" : "H.264",
                     current_codec == RtspVideoCodec::kH265 ? "H.265" : "H.264");

        codec_ = current_codec;
        clearParams();
        fetchVideoParams();
    }
}

HdalVideoSubsession::~HdalVideoSubsession() {
    clearParams();
}

FramedSource* HdalVideoSubsession::createNewStreamSource(unsigned clientSessionId,
                                                         unsigned& estBitrate) {
    estBitrate = 5000;

    refreshCodecIfChanged();

    HdalVideoSource* source = HdalVideoSource::createNew(envir(), channel_id_, codec_);
    if (!source) {
        return nullptr;
    }

    if (codec_ == RtspVideoCodec::kH265) {
        return H265VideoStreamDiscreteFramer::createNew(envir(), source);
    } else {
        return H264VideoStreamDiscreteFramer::createNew(envir(), source);
    }
}

RTPSink* HdalVideoSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                               unsigned char rtpPayloadTypeIfDynamic,
                                               FramedSource* inputSource) {
    OutPacketBuffer::maxSize = kMaxFrameSize;
    increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), kSendBufferSize);

    if (!have_params_) {
        fetchVideoParams();
    }

    if (codec_ == RtspVideoCodec::kH265) {
        return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                           vps_, vps_size_,
                                           sps_, sps_size_,
                                           pps_, pps_size_);
    } else {
        return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                           sps_, sps_size_,
                                           pps_, pps_size_);
    }
}

} // namespace streaming
} // namespace ipcam

#endif // RTSP_SERVER_ENABLED
