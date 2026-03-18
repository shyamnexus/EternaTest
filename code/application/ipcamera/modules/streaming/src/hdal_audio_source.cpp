/**
 * @file hdal_audio_source.cpp
 * @brief HDAL Audio Source implementation for Live555 RTSP server
 * 
 * Contains HdalAudioSource and HdalAudioSubsession implementations.
 * Now uses AudioFrameBroadcaster for efficient single-producer, multi-consumer
 * audio distribution. The broadcaster encodes audio once and broadcasts to all
 * registered consumers (RTSP clients).
 */

#include "ipcam/rtsp_sources_internal.h"
#include "ipcam/audio_frame_broadcaster.h"

#ifndef RTSP_SERVER_ENABLED
#define RTSP_SERVER_ENABLED 1
#endif

#if RTSP_SERVER_ENABLED

namespace ipcam {
namespace streaming {

// ============================================================================
// HdalAudioSource Implementation (Consumer of AudioFrameBroadcaster)
// ============================================================================

HdalAudioSource* HdalAudioSource::createNew(UsageEnvironment& env,
                                            RtspAudioCodec codec,
                                            int sample_rate,
                                            int channels) {
    return new HdalAudioSource(env, codec, sample_rate, channels);
}

HdalAudioSource::HdalAudioSource(UsageEnvironment& env, RtspAudioCodec codec,
                                 int sample_rate, int channels)
    : FramedSource(env)
    , codec_(codec)
    , sample_rate_(sample_rate)
    , channels_(channels)
    , base_hw_timestamp_(0)
    , last_hw_timestamp_(0)
    , have_base_time_(false) {
    
    memset(&base_presentation_time_, 0, sizeof(base_presentation_time_));
    
    // Get the AudioFrameBroadcaster singleton
    auto& broadcaster = AudioFrameBroadcaster::Instance();
    
    // Check if broadcaster needs initialization or reconfiguration
    if (!broadcaster.IsInitialized()) {
        // First time initialization
        if (!broadcaster.Initialize(codec, sample_rate, channels)) {
            spdlog::error("HdalAudioSource: Failed to initialize AudioFrameBroadcaster");
        }
    } else if (broadcaster.GetCodec() != codec) {
        // Codec changed - need to reconfigure
        spdlog::info("HdalAudioSource: Codec changed from {} to {}, reconfiguring broadcaster",
                     static_cast<int>(broadcaster.GetCodec()), static_cast<int>(codec));
        if (!broadcaster.Reconfigure(codec, sample_rate, channels)) {
            spdlog::error("HdalAudioSource: Failed to reconfigure AudioFrameBroadcaster");
        }
    }
    
    // Start broadcaster if not already running
    if (broadcaster.IsInitialized() && !broadcaster.IsRunning()) {
        if (!broadcaster.Start()) {
            spdlog::error("HdalAudioSource: Failed to start AudioFrameBroadcaster");
        }
    }
    
    // Register as a consumer
    consumer_ = broadcaster.RegisterConsumer();
    if (!consumer_) {
        spdlog::error("HdalAudioSource: Failed to register as consumer");
    } else {
        spdlog::debug("HdalAudioSource: Registered as consumer {}", consumer_->GetId());
    }
    
    // Create event trigger
    event_trigger_id_ = envir().taskScheduler().createEventTrigger(deliverFrameCallback);
    
    // Build codec name for logging
    const char* codec_name = "PCM";
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
    spdlog::debug("HdalAudioSource: Created codec={} sample_rate={} channels={} (using broadcaster)",
                 codec_name, sample_rate, channels);
}

HdalAudioSource::~HdalAudioSource() {
    // Cancel any pending scheduled tasks
    if (nextTask() != NULL) {
        envir().taskScheduler().unscheduleDelayedTask(nextTask());
    }
    
    // Delete event trigger
    if (event_trigger_id_ != 0) {
        envir().taskScheduler().deleteEventTrigger(event_trigger_id_);
        event_trigger_id_ = 0;
    }
    
    // Consumer will be automatically unregistered when unique_ptr is destroyed
    if (consumer_) {
        spdlog::debug("HdalAudioSource: Destroying consumer {}", consumer_->GetId());
    }
    consumer_.reset();
    
    usleep(5000);  // 5ms for any in-flight operations
}

void HdalAudioSource::doGetNextFrame() {
    envir().taskScheduler().triggerEvent(event_trigger_id_, this);
}

void HdalAudioSource::deliverFrameCallback(void* clientData) {
    static_cast<HdalAudioSource*>(clientData)->deliverFrame();
}

void HdalAudioSource::deliverFrame() {
    if (!isCurrentlyAwaitingData()) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            10000, (TaskFunc*)deliverFrameCallback, this);
        return;
    }
    
    // Check if we have a valid consumer
    if (!consumer_ || !consumer_->IsValid()) {
        spdlog::error("HdalAudioSource: Invalid consumer");
        handleClosure();
        return;
    }
    
    // Try to get the next frame from the broadcaster
    AudioFrame frame;
    if (!consumer_->GetNextFrame(frame, 0)) {
        // No frame available yet, retry shortly
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            5000, (TaskFunc*)deliverFrameCallback, this);
        return;
    }
    
    uint64_t hw_timestamp = frame.timestamp;
    uint32_t output_size = frame.size;
    
    // Copy to output buffer
    if (output_size > fMaxSize) {
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = output_size - fMaxSize;
        spdlog::warn("HdalAudioSource: Frame truncated ({} > {})", output_size, fMaxSize);
    } else {
        fFrameSize = output_size;
        fNumTruncatedBytes = 0;
    }
    
    memcpy(fTo, frame.data, fFrameSize);
    
    // Set presentation time
    constexpr int64_t kMaxAudioPresentationDeltaUs = 10000000;  // 10 seconds max
    
    if (!have_base_time_) {
        gettimeofday(&fPresentationTime, nullptr);
        base_presentation_time_ = fPresentationTime;
        base_hw_timestamp_ = hw_timestamp;
        last_hw_timestamp_ = hw_timestamp;
        have_base_time_ = true;
    } else {
        int64_t delta_from_last = static_cast<int64_t>(hw_timestamp) - static_cast<int64_t>(last_hw_timestamp_);
        int64_t delta_from_base = static_cast<int64_t>(hw_timestamp) - static_cast<int64_t>(base_hw_timestamp_);
        
        // Reset if:
        // 1. Discontinuity from last frame (> 500ms or negative)
        // 2. Delta from base exceeds sanity limit (prevents 149-hour timestamps)
        bool need_reset = false;
        if (delta_from_last < 0 || delta_from_last > 500000) {
            spdlog::debug("HdalAudioSource: Timestamp discontinuity from last (delta={}us)", delta_from_last);
            need_reset = true;
        } else if (delta_from_base < 0 || delta_from_base > kMaxAudioPresentationDeltaUs) {
            spdlog::debug("HdalAudioSource: Presentation delta overflow ({}us), resetting base", delta_from_base);
            need_reset = true;
        }
        
        if (need_reset) {
            gettimeofday(&fPresentationTime, nullptr);
            base_presentation_time_ = fPresentationTime;
            base_hw_timestamp_ = hw_timestamp;
            last_hw_timestamp_ = hw_timestamp;
        } else {
            uint64_t delta_us = static_cast<uint64_t>(delta_from_base);
            fPresentationTime.tv_sec = base_presentation_time_.tv_sec + static_cast<long>(delta_us / 1000000);
            fPresentationTime.tv_usec = base_presentation_time_.tv_usec + static_cast<long>(delta_us % 1000000);
            if (fPresentationTime.tv_usec >= 1000000) {
                fPresentationTime.tv_sec += fPresentationTime.tv_usec / 1000000;
                fPresentationTime.tv_usec %= 1000000;
            }
            last_hw_timestamp_ = hw_timestamp;
        }
    }
    
    // Use duration from frame
    fDurationInMicroseconds = frame.duration_us;
    
    afterGetting(this);
}

// ============================================================================
// HdalAudioSubsession Implementation
// ============================================================================

HdalAudioSubsession* HdalAudioSubsession::createNew(UsageEnvironment& env,
                                                    RtspAudioCodec codec,
                                                    int sample_rate,
                                                    int channels) {
    return new HdalAudioSubsession(env, codec, sample_rate, channels);
}

HdalAudioSubsession::HdalAudioSubsession(UsageEnvironment& env,
                                         RtspAudioCodec codec,
                                         int sample_rate,
                                         int channels)
    : OnDemandServerMediaSubsession(env, True /* reuse source */)
    , codec_(codec)
    , sample_rate_(sample_rate)
    , channels_(channels) {
    
    const char* codec_name = "PCM";
    switch (codec) {
        case RtspAudioCodec::kPcmu: codec_name = "PCMU"; break;
        case RtspAudioCodec::kPcma: codec_name = "PCMA"; break;
        case RtspAudioCodec::kG726_16: codec_name = "G726-16"; break;
        case RtspAudioCodec::kG726_24: codec_name = "G726-24"; break;
        case RtspAudioCodec::kG726_32: codec_name = "G726-32"; break;
        case RtspAudioCodec::kG726_40: codec_name = "G726-40"; break;
        case RtspAudioCodec::kAac: codec_name = "AAC-LC"; break;
        default: break;
    }
    spdlog::debug("HdalAudioSubsession: Created codec={} rate={} ch={}",
                  codec_name, sample_rate, channels);
}

HdalAudioSubsession::~HdalAudioSubsession() {
}

FramedSource* HdalAudioSubsession::createNewStreamSource(unsigned clientSessionId,
                                                         unsigned& estBitrate) {
    switch (codec_) {
        case RtspAudioCodec::kPcmu:
        case RtspAudioCodec::kPcma:
            estBitrate = 64;
            break;
        case RtspAudioCodec::kG726_16:
            estBitrate = 16;
            break;
        case RtspAudioCodec::kG726_24:
            estBitrate = 24;
            break;
        case RtspAudioCodec::kG726_32:
            estBitrate = 32;
            break;
        case RtspAudioCodec::kG726_40:
            estBitrate = 40;
            break;
        case RtspAudioCodec::kAac:
            estBitrate = (sample_rate_ >= 44100) ? 128 : 64;
            break;
        default:
            estBitrate = sample_rate_ * 16 * channels_ / 1000;
            break;
    }
    
    return HdalAudioSource::createNew(envir(), codec_, sample_rate_, channels_);
}

RTPSink* HdalAudioSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                               unsigned char rtpPayloadTypeIfDynamic,
                                               FramedSource* inputSource) {
    if (codec_ == RtspAudioCodec::kPcmu) {
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        0, 8000,
                                        "audio", "PCMU",
                                        1, True, False);
    } else if (codec_ == RtspAudioCodec::kPcma) {
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        8, 8000,
                                        "audio", "PCMA",
                                        1, True, False);
    } else if (codec_ == RtspAudioCodec::kG726_16) {
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        rtpPayloadTypeIfDynamic, 8000,
                                        "audio", "G726-16",
                                        1, True, False);
    } else if (codec_ == RtspAudioCodec::kG726_24) {
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        rtpPayloadTypeIfDynamic, 8000,
                                        "audio", "G726-24",
                                        1, True, False);
    } else if (codec_ == RtspAudioCodec::kG726_32) {
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        rtpPayloadTypeIfDynamic, 8000,
                                        "audio", "G726-32",
                                        1, True, False);
    } else if (codec_ == RtspAudioCodec::kG726_40) {
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        rtpPayloadTypeIfDynamic, 8000,
                                        "audio", "G726-40",
                                        1, True, False);
    } else if (codec_ == RtspAudioCodec::kAac) {
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        rtpPayloadTypeIfDynamic, sample_rate_,
                                        "audio", "mpeg4-generic",
                                        channels_, True, False);
    } else {
        // Raw PCM (L16)
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        rtpPayloadTypeIfDynamic, sample_rate_,
                                        "audio", "L16",
                                        channels_, True, False);
    }
}

} // namespace streaming
} // namespace ipcam

#endif // RTSP_SERVER_ENABLED
