/**
 * @file rtsp_sources_internal.h
 * @brief Internal header for RTSP server source components
 * 
 * This header contains internal class declarations shared between
 * the split RTSP server source files. Not for external use.
 */

#ifndef IPCAM_RTSP_SOURCES_INTERNAL_H
#define IPCAM_RTSP_SOURCES_INTERNAL_H

#include "ipcam/rtsp_server.h"
#include "ipcam/config.h"
#include "ipcam/hdal_pipeline.h"
#include "ipcam/media_hub.h"
#include "ipcam/g711_codec.h"
#include "ipcam/g726_codec.h"
#include "ipcam/audio_encoder.h"
#include "ipcam/onvif_metadata.h"
#include "ipcam/audio_frame_broadcaster.h"

#include <spdlog/spdlog.h>

// Live555 headers
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

// HDAL headers
extern "C" {
#include "hdal.h"
#include "hd_type.h"
#include "hd_videoenc.h"
#include "hd_audiocapture.h"
}

#include <cstring>
#include <chrono>
#include <vector>
#include <memory>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace ipcam {
namespace streaming {

// ============================================================================
// Constants
// ============================================================================
static constexpr int kFramePullTimeoutMs = 100;         // 100ms timeout
static constexpr int kMaxFrameSize = 4 * 1024 * 1024;   // 4MB max frame
static constexpr int kMaxFlushFrameCount = 200;         // Flush old frames
static constexpr int kSendBufferSize = 8 * 1024 * 1024; // 8MB send buffer

// Note: kOnvifMetadataPayloadType and kOnvifMetadataClockRate are defined in onvif_metadata.h

// ============================================================================
// HdalVideoSource - FramedSource consuming from MediaHub ring buffer
// ============================================================================

/**
 * @brief Live555 FramedSource that reads encoded video from MediaHub.
 *
 * Instead of pulling directly from the HDAL encoder (which causes contention
 * with recording and other consumers), this source reads from the MediaHub's
 * per-channel ring buffer. The MediaHub producer thread handles all HDAL
 * interaction and distributes frames to all consumers.
 */
class HdalVideoSource : public FramedSource {
public:
    static HdalVideoSource* createNew(UsageEnvironment& env,
                                      int channel_id,
                                      RtspVideoCodec codec);
    
    bool getSpsPpsVps(u_int8_t*& sps, unsigned& spsSize,
                      u_int8_t*& pps, unsigned& ppsSize,
                      u_int8_t*& vps, unsigned& vpsSize);
    
    RtspVideoCodec getCodec() const { return codec_; }
    bool checkAndHandleCodecChange();

protected:
    HdalVideoSource(UsageEnvironment& env, int channel_id, RtspVideoCodec codec);
    virtual ~HdalVideoSource();
    virtual void doGetNextFrame() override;

private:
    static void deliverFrameCallback(void* clientData);
    void deliverFrame();
    RtspVideoCodec getCurrentCodecFromConfig() const;
    
    int channel_id_;
    RtspVideoCodec codec_;
    bool got_idr_;
    
    // MediaHub consumer (replaces direct HDAL access)
    std::unique_ptr<media::VideoFrameConsumer> consumer_;
    media::VideoFrame current_frame_;     ///< Last frame read from ring buffer
    
    // Cached NAL parameters
    uint8_t sps_[256];
    unsigned sps_size_;
    uint8_t pps_[256];
    unsigned pps_size_;
    uint8_t vps_[256];
    unsigned vps_size_;
    bool have_parameters_;
    
    EventTriggerId event_trigger_id_;
    
    // Presentation timing
    struct timeval base_presentation_time_;
    uint64_t base_hw_timestamp_;
    uint64_t last_hw_timestamp_;
    bool have_base_time_;
    
    int codec_check_counter_;
    static constexpr int kCodecCheckInterval = 30;
    static constexpr int64_t kMaxPresentationDeltaUs = 10000000;  // 10 seconds max
};

// ============================================================================
// HdalVideoSubsession - ServerMediaSubsession for HDAL video
// ============================================================================

class HdalVideoSubsession : public OnDemandServerMediaSubsession {
public:
    static HdalVideoSubsession* createNew(UsageEnvironment& env,
                                          int channel_id,
                                          RtspVideoCodec codec);

protected:
    HdalVideoSubsession(UsageEnvironment& env, int channel_id, RtspVideoCodec codec);
    virtual ~HdalVideoSubsession();
    
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                unsigned& estBitrate) override;
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource* inputSource) override;

private:
    void fetchVideoParams();
    void refreshCodecIfChanged();
    void clearParams();
    RtspVideoCodec getCurrentCodecFromConfig() const;
    
    int channel_id_;
    RtspVideoCodec codec_;
    
    u_int8_t* sps_;
    unsigned sps_size_;
    u_int8_t* pps_;
    unsigned pps_size_;
    u_int8_t* vps_;
    unsigned vps_size_;
    bool have_params_;
};

// ============================================================================
// HdalAudioSource - FramedSource for HDAL audio (Consumer of AudioFrameBroadcaster)
// ============================================================================

/**
 * @brief Audio source that consumes frames from the AudioFrameBroadcaster singleton.
 * 
 * Each HdalAudioSource instance registers as a consumer with the broadcaster.
 * The broadcaster handles HDAL interaction and encoding once, broadcasting
 * encoded frames to all consumers via a lock-free ring buffer.
 */
class HdalAudioSource : public FramedSource {
public:
    static HdalAudioSource* createNew(UsageEnvironment& env,
                                      RtspAudioCodec codec,
                                      int sample_rate,
                                      int channels);
    
    RtspAudioCodec getCodec() const { return codec_; }
    int getSampleRate() const { return sample_rate_; }
    int getChannels() const { return channels_; }

protected:
    HdalAudioSource(UsageEnvironment& env, RtspAudioCodec codec, 
                    int sample_rate, int channels);
    virtual ~HdalAudioSource();
    virtual void doGetNextFrame() override;

private:
    static void deliverFrameCallback(void* clientData);
    void deliverFrame();
    
    RtspAudioCodec codec_;
    int sample_rate_;
    int channels_;
    
    // Consumer handle for the broadcaster
    std::unique_ptr<AudioFrameConsumer> consumer_;
    
    EventTriggerId event_trigger_id_;
    
    // Timing for presentation timestamps
    struct timeval base_presentation_time_;
    uint64_t base_hw_timestamp_;
    uint64_t last_hw_timestamp_;
    bool have_base_time_;
};

// ============================================================================
// HdalAudioSubsession - ServerMediaSubsession for HDAL audio
// ============================================================================

class HdalAudioSubsession : public OnDemandServerMediaSubsession {
public:
    static HdalAudioSubsession* createNew(UsageEnvironment& env,
                                          RtspAudioCodec codec,
                                          int sample_rate,
                                          int channels);

protected:
    HdalAudioSubsession(UsageEnvironment& env, RtspAudioCodec codec,
                        int sample_rate, int channels);
    virtual ~HdalAudioSubsession();
    
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                unsigned& estBitrate) override;
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource* inputSource) override;

private:
    RtspAudioCodec codec_;
    int sample_rate_;
    int channels_;
};

// ============================================================================
// OnvifMetadataSource - FramedSource for ONVIF metadata
// ============================================================================

class OnvifMetadataSource : public FramedSource {
public:
    static OnvifMetadataSource* createNew(UsageEnvironment& env);

protected:
    OnvifMetadataSource(UsageEnvironment& env);
    virtual ~OnvifMetadataSource();
    virtual void doGetNextFrame() override;

private:
    static void deliverFrame0(void* clientData);
    void deliverFrame();
    
    EventTriggerId fEventTriggerId;
};

// ============================================================================
// OnvifMetadataSubsession - ServerMediaSubsession for ONVIF metadata
// ============================================================================

class OnvifMetadataSubsession : public OnDemandServerMediaSubsession {
public:
    static OnvifMetadataSubsession* createNew(UsageEnvironment& env);

protected:
    OnvifMetadataSubsession(UsageEnvironment& env);
    virtual ~OnvifMetadataSubsession();
    
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                unsigned& estBitrate) override;
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource* inputSource) override;
    virtual char const* sdpLines(int addressFamily) override;
};

// ============================================================================
// LocalhostBypassRTSPServer - RTSP server with localhost auth bypass
// ============================================================================

class LocalhostBypassClientConnection : public RTSPServer::RTSPClientConnection {
public:
    LocalhostBypassClientConnection(RTSPServer& ourServer,
                                    int clientSocket, 
                                    struct sockaddr_storage const& clientAddr);
    
protected:
    virtual ~LocalhostBypassClientConnection() {}
    virtual Boolean authenticationOK(char const* cmdName, 
                                     char const* urlSuffix, 
                                     char const* fullRequestStr) override;

private:
    bool isLocalhost() const;
};

class LocalhostBypassRTSPServer : public RTSPServer {
public:
    static LocalhostBypassRTSPServer* createNew(UsageEnvironment& env, 
                                                 Port port,
                                                 UserAuthenticationDatabase* authDatabase,
                                                 unsigned reclamationSeconds = 65);

protected:
    LocalhostBypassRTSPServer(UsageEnvironment& env, 
                              int ourSocketIPv4, int ourSocketIPv6,
                              Port port, 
                              UserAuthenticationDatabase* authDatabase,
                              unsigned reclamationSeconds);
    
    virtual ~LocalhostBypassRTSPServer() {}
    
    virtual ClientConnection* createNewClientConnection(int clientSocket, 
                                                         struct sockaddr_storage const& clientAddr) override;
};

} // namespace streaming
} // namespace ipcam

#endif // IPCAM_RTSP_SOURCES_INTERNAL_H
