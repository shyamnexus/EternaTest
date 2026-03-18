/**
 * @file onvif_rtsp_source.cpp
 * @brief ONVIF Metadata Source implementation for Live555 RTSP server
 * 
 * Contains OnvifMetadataSource and OnvifMetadataSubsession implementations.
 * Delivers ONVIF-compliant metadata XML via RTP.
 */

#include "ipcam/rtsp_sources_internal.h"

#ifndef RTSP_SERVER_ENABLED
#define RTSP_SERVER_ENABLED 1
#endif

#if RTSP_SERVER_ENABLED

namespace ipcam {
namespace streaming {

// ============================================================================
// OnvifMetadataSource Implementation
// ============================================================================

OnvifMetadataSource* OnvifMetadataSource::createNew(UsageEnvironment& env) {
    return new OnvifMetadataSource(env);
}

OnvifMetadataSource::OnvifMetadataSource(UsageEnvironment& env)
    : FramedSource(env)
    , fEventTriggerId(0) {
    fEventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    spdlog::debug("OnvifMetadataSource: Created");
}

OnvifMetadataSource::~OnvifMetadataSource() {
    envir().taskScheduler().deleteEventTrigger(fEventTriggerId);
    spdlog::debug("OnvifMetadataSource: Destroyed");
}

void OnvifMetadataSource::doGetNextFrame() {
    deliverFrame();
}

void OnvifMetadataSource::deliverFrame0(void* clientData) {
    ((OnvifMetadataSource*)clientData)->deliverFrame();
}

void OnvifMetadataSource::deliverFrame() {
    if (!isCurrentlyAwaitingData()) {
        return;
    }
    
    OnvifMetadataFrame frame;
    if (!OnvifMetadataGenerator::Instance().GetNextFrame(frame)) {
        // No data available - schedule retry
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            33000,  // 33ms (about one video frame period)
            (TaskFunc*)FramedSource::afterGetting, this);
        return;
    }
    
    // Generate XML
    std::string xml = frame.ToXml();
    
    // Copy to output buffer
    fFrameSize = std::min(xml.size(), (size_t)fMaxSize);
    if (fFrameSize < xml.size()) {
        fNumTruncatedBytes = xml.size() - fFrameSize;
        spdlog::warn("OnvifMetadataSource: Truncated {} bytes", fNumTruncatedBytes);
    } else {
        fNumTruncatedBytes = 0;
    }
    
    memcpy(fTo, xml.data(), fFrameSize);
    
    // Set presentation time
    gettimeofday(&fPresentationTime, NULL);
    fDurationInMicroseconds = 33333;  // ~30fps
    
    // Deliver
    afterGetting(this);
}

// ============================================================================
// OnvifMetadataSubsession Implementation
// ============================================================================

OnvifMetadataSubsession* OnvifMetadataSubsession::createNew(UsageEnvironment& env) {
    return new OnvifMetadataSubsession(env);
}

OnvifMetadataSubsession::OnvifMetadataSubsession(UsageEnvironment& env)
    : OnDemandServerMediaSubsession(env, True /* reuse source */) {
    spdlog::debug("OnvifMetadataSubsession: Created");
}

OnvifMetadataSubsession::~OnvifMetadataSubsession() {
}

FramedSource* OnvifMetadataSubsession::createNewStreamSource(unsigned clientSessionId,
                                                             unsigned& estBitrate) {
    estBitrate = 50;  // Low bitrate for metadata (~50 kbps max)
    return OnvifMetadataSource::createNew(envir());
}

RTPSink* OnvifMetadataSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                                   unsigned char rtpPayloadTypeIfDynamic,
                                                   FramedSource* inputSource) {
    // Use SimpleRTPSink for raw XML payload
    return SimpleRTPSink::createNew(
        envir(), rtpGroupsock,
        kOnvifMetadataPayloadType,      // 107
        kOnvifMetadataClockRate,        // 90000
        "application",                   // Media type
        "vnd.onvif.metadata",           // Encoding name (ONVIF spec)
        1,                               // Num channels
        True,                            // Allow multiple frames per packet
        True                             // Set marker bit on last packet
    );
}

char const* OnvifMetadataSubsession::sdpLines(int addressFamily) {
    if (fSDPLines) return fSDPLines;
    
    // Generate our own SDP (Live555 doesn't know about vnd.onvif.metadata)
    char sdpBuf[256];
    snprintf(sdpBuf, sizeof(sdpBuf),
        "m=application 0 RTP/AVP %d\r\n"
        "c=IN %s 0.0.0.0\r\n"
        "b=AS:50\r\n"
        "a=rtpmap:%d vnd.onvif.metadata/%d\r\n"
        "a=recvonly\r\n"
        "a=control:%s\r\n",
        kOnvifMetadataPayloadType,
        (addressFamily == AF_INET6) ? "IP6" : "IP4",
        kOnvifMetadataPayloadType,
        kOnvifMetadataClockRate,
        trackId());
    
    fSDPLines = strDup(sdpBuf);
    return fSDPLines;
}

} // namespace streaming
} // namespace ipcam

#endif // RTSP_SERVER_ENABLED
