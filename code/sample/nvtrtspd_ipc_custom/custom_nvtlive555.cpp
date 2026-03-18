/*
 * Custom nvtlive555 implementation using local live555 library.
 * Replaces the proprietary libnvtlive555.so/a.
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

// Live555 includes
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <Base64.hh>

// Novatek includes
extern "C" {
    #include "nvtlive555.h"
}

// ---------------------------------------------------------
// Global State
// ---------------------------------------------------------
static NVTLIVE555_INIT g_init = {0};
static UsageEnvironment* g_env = NULL;
static TaskScheduler* g_scheduler = NULL;
static RTSPServer* g_rtspServer = NULL;
static EventLoopWatchVariable g_watchVariable(0);

// ---------------------------------------------------------
// Helpers
// ---------------------------------------------------------

// Helper to check for start code and skip it if present (00 00 00 01 or 00 00 01)
static unsigned char* skipStartCode(unsigned char* p, int size, int& outSize) {
    int offset = 0;
    if (size > 4 && p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1) {
        offset = 4;
    } else if (size > 3 && p[0] == 0 && p[1] == 0 && p[2] == 1) {
        offset = 3;
    }
    outSize = size - offset;
    return p + offset;
}

// ---------------------------------------------------------
// Custom FramedSource
// ---------------------------------------------------------
class NvtFramedSource : public FramedSource {
public:
    static NvtFramedSource* createNew(UsageEnvironment& env, int channel, NVTLIVE555_HDAL_CB* cb) {
        return new NvtFramedSource(env, channel, cb);
    }

protected:
    NvtFramedSource(UsageEnvironment& env, int channel, NVTLIVE555_HDAL_CB* cb)
        : FramedSource(env), m_channel(channel), m_cb(cb), m_hHandle(0) {
        if (m_cb && m_cb->open_video) {
            m_hHandle = m_cb->open_video(m_channel);
        }
    }

    virtual ~NvtFramedSource() {
        if (m_hHandle && m_cb && m_cb->close_video) {
            m_cb->close_video(m_hHandle);
        }
    }

    virtual void doGetNextFrame() {
        if (!m_hHandle || !m_cb || !m_cb->lock_video) {
            handleClosure();
            return;
        }

        NVTLIVE555_STRM_INFO strm = {0};
        // Use a short timeout to not block too long, but long enough to yield.
        // live555 is single threaded so we must not block.
        // If lock fails, schedule retry.
        int ret = m_cb->lock_video(m_hHandle, 0, &strm); 
        
        if (ret == 0 && strm.addr != 0 && strm.size > 0) {
            // Success
            if (strm.size > fMaxSize) {
                fFrameSize = fMaxSize;
                fNumTruncatedBytes = strm.size - fMaxSize;
            } else {
                fFrameSize = strm.size;
                fNumTruncatedBytes = 0;
            }
            
            memcpy(fTo, (void*)strm.addr, fFrameSize);
            
            // Set timestamp
            if (strm.timestamp > 0) {
                fPresentationTime.tv_sec = strm.timestamp / 1000000; // Assuming us
                fPresentationTime.tv_usec = strm.timestamp % 1000000;
            } else {
                 gettimeofday(&fPresentationTime, NULL);
            }
            
            // Unlock
            if (m_cb->unlock_video) {
                m_cb->unlock_video(m_hHandle);
            }

            // Signal success
            FramedSource::afterGetting(this);
        } else {
            // Retry later
            nextTask() = envir().taskScheduler().scheduleDelayedTask(10000, (TaskFunc*)getNextFrame, this);
        }
    }

private:
    static void getNextFrame(void* clientData) {
        NvtFramedSource* source = (NvtFramedSource*)clientData;
        source->doGetNextFrame();
    }

    int m_channel;
    NVTLIVE555_HDAL_CB* m_cb;
    uintptr_t m_hHandle;
};

// ---------------------------------------------------------
// Custom ServerMediaSubsession
// ---------------------------------------------------------
class NvtServerMediaSubsession : public OnDemandServerMediaSubsession {
public:
    static NvtServerMediaSubsession* createNew(UsageEnvironment& env, NVTLIVE555_URL_INFO info, NVTLIVE555_HDAL_CB* cb) {
        return new NvtServerMediaSubsession(env, info, cb);
    }

protected:
    NvtServerMediaSubsession(UsageEnvironment& env, NVTLIVE555_URL_INFO info, NVTLIVE555_HDAL_CB* cb)
        : OnDemandServerMediaSubsession(env, False), m_info(info), m_cb(cb), m_auxSDPLine(NULL) {
    }

    virtual ~NvtServerMediaSubsession() {
        if (m_auxSDPLine) delete[] m_auxSDPLine;
    }

    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate) {
        estBitrate = 4000; // Dummy bitrate
        
        NvtFramedSource* source = NvtFramedSource::createNew(envir(), m_info.channel_id, m_cb);
        if (!source) return NULL;

        // Check codec type to decide on framing
        // We need a temporary handle or just check global/cached info.
        // For simplicity, we create the source. The sink creation does the heavy lifting of type check.
        // BUT, FramedSource is generic. We probably need to wrap it in a Framer based on type.
        
        // We need to know codec type here to wrap in appropriate framer!
        // Let's get info.
        uintptr_t handle = m_cb->open_video(m_info.channel_id);
        NVTLIVE555_VIDEO_INFO vInfo = {NVTLIVE555_CODEC_UNKNOWN};
        if (handle) {
             m_cb->get_video_info(handle, 100, &vInfo);
             m_cb->close_video(handle); 
        }

        FramedSource* videoSource = source;

        if (vInfo.codec_type == NVTLIVE555_CODEC_H264) {
             return H264VideoStreamFramer::createNew(envir(), videoSource);
        } else if (vInfo.codec_type == NVTLIVE555_CODEC_H265) {
             return H265VideoStreamFramer::createNew(envir(), videoSource);
        } else if (vInfo.codec_type == NVTLIVE555_CODEC_MJPG) {
             return videoSource; // JPEG doesn't usually need a framer if it's whole frames
        }
        
        return videoSource;
    }

    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) {
        // We need codec type again.
        uintptr_t handle = m_cb->open_video(m_info.channel_id);
        NVTLIVE555_VIDEO_INFO vInfo = {NVTLIVE555_CODEC_UNKNOWN};
        if (handle) {
             m_cb->get_video_info(handle, 100, &vInfo);
             m_cb->close_video(handle); 
        }

        if (vInfo.codec_type == NVTLIVE555_CODEC_H264) {
            return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
        } else if (vInfo.codec_type == NVTLIVE555_CODEC_H265) {
            // H265VideoRTPSink
             return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
        } else if (vInfo.codec_type == NVTLIVE555_CODEC_MJPG) {
            return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
        }
        
        return NULL;
    }

    virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
        if (m_auxSDPLine) return m_auxSDPLine;

        // Fetch SPS/PPS
        uintptr_t handle = m_cb->open_video(m_info.channel_id);
        if (!handle) return NULL;
        
        NVTLIVE555_VIDEO_INFO vInfo = {NVTLIVE555_CODEC_UNKNOWN};
        m_cb->get_video_info(handle, 100, &vInfo);
        m_cb->close_video(handle);

        if (vInfo.codec_type == NVTLIVE555_CODEC_H264) {
             int spsSize = 0, ppsSize = 0;
             unsigned char* sps = skipStartCode(vInfo.sps, vInfo.sps_size, spsSize);
             unsigned char* pps = skipStartCode(vInfo.pps, vInfo.pps_size, ppsSize);
             
             unsigned profile_level_id = 0;
             if (spsSize >= 3) {
                 profile_level_id = (sps[1] << 16) | (sps[2] << 8) | sps[3];
             }

             char* spsBase64 = base64Encode((char*)sps, spsSize);
             char* ppsBase64 = base64Encode((char*)pps, ppsSize);

             std::ostringstream os;
             os << "a=fmtp:96 packetization-mode=1;profile-level-id=" << std::hex << std::setw(6) << std::setfill('0') << profile_level_id;
             os << ";sprop-parameter-sets=" << spsBase64 << "," << ppsBase64 << "\r\n";
             
             delete[] spsBase64;
             delete[] ppsBase64;
             
             m_auxSDPLine = strDup(os.str().c_str());
        } else if (vInfo.codec_type == NVTLIVE555_CODEC_H265) {
             // Basic H265 SDP generation if needed, usually H265VideoRTPSink handles it partly, but we might need vps/sps/pps
             // For now return NULL and let client figure it out or implement if needed.
             // H265 requires valid parameter sets in SDP for some players.
             // Assuming similar logic for VPS/SPS/PPS
             int vpsSize=0, spsSize=0, ppsSize=0;
             unsigned char* vps = skipStartCode(vInfo.vps, vInfo.vps_size, vpsSize);
             unsigned char* sps = skipStartCode(vInfo.sps, vInfo.sps_size, spsSize);
             unsigned char* pps = skipStartCode(vInfo.pps, vInfo.pps_size, ppsSize);
             
             // ... construction of sprop-vps etc ...
             // Simplified for now.
        }

        return m_auxSDPLine;
    }

private:
    NVTLIVE555_URL_INFO m_info;
    NVTLIVE555_HDAL_CB* m_cb;
    char* m_auxSDPLine;
};

// ---------------------------------------------------------
// Custom RTSPServer
// ---------------------------------------------------------
class NvtRTSPServer : public RTSPServer {
public:
    static NvtRTSPServer* createNew(UsageEnvironment& env, Port ourPort) {
        int ourSocket = setUpOurSocket(env, ourPort, AF_INET);
        if (ourSocket == -1) return NULL;
        return new NvtRTSPServer(env, ourSocket, ourPort, NULL, 65);
    }

protected:
    NvtRTSPServer(UsageEnvironment& env, int ourSocket, Port ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationSeconds)
        : RTSPServer(env, ourSocket, -1, ourPort, authDatabase, reclamationSeconds) {}
        
    virtual ~NvtRTSPServer() {}

    virtual void lookupServerMediaSession(char const* streamName, lookupServerMediaSessionCompletionFunc* completionFunc, void* completionClientData, Boolean isFirstLookupInSession) {
        // First check if it exists
        ServerMediaSession* sms = getServerMediaSession(streamName);
        if (sms) {
            completionFunc(completionClientData, sms);
            return;
        }

        // Not found, try to parse URL
        if (g_init.require_cb.parse_url) {
            NVTLIVE555_URL_INFO info = {0};
            if (g_init.require_cb.parse_url(streamName, &info) == 0) {
                 // Valid URL, create session
                 sms = ServerMediaSession::createNew(envir(), streamName, streamName, "Session created by custom nvtlive555 layer");
                 // Add video subsession
                 sms->addSubsession(NvtServerMediaSubsession::createNew(envir(), info, &g_init.hdal_cb));
                 
                 addServerMediaSession(sms);
                 completionFunc(completionClientData, sms);
                 return;
            }
        }
        completionFunc(completionClientData, NULL);
    }
};

// ---------------------------------------------------------
// Exported API
// ---------------------------------------------------------

int nvtlive555_open(NVTLIVE555_INIT *p_init) {
    if (!p_init) return -1;
    g_init = *p_init;
    g_watchVariable = 0;

    g_scheduler = BasicTaskScheduler::createNew();
    g_env = BasicUsageEnvironment::createNew(*g_scheduler);

    NVTLIVE555_CFG cfg = {0};
    // Default config
    cfg.port = 554;
    cfg.max_clients = 10;
    
    if (g_init.require_cb.get_cfg) {
        g_init.require_cb.get_cfg(&cfg);
    }
    
    Port rtspPort(cfg.port);
    g_rtspServer = NvtRTSPServer::createNew(*g_env, rtspPort);
    
    if (g_rtspServer == NULL) {
        *g_env << "Failed to create RTSP server: " << g_env->getResultMsg() << "\n";
        return -1;
    }

    *g_env << "NvtLive555 (Custom) started on port " << cfg.port << "\n";

    // Run the loop
    g_env->taskScheduler().doEventLoop(&g_watchVariable);

    return 0;
}

int nvtlive555_close(void) {
    g_watchVariable = 1;
    
    if (g_rtspServer) {
        Medium::close(g_rtspServer);
        g_rtspServer = NULL;
    }
    if (g_env) {
        g_env->reclaim();
        g_env = NULL;
    }
    if (g_scheduler) {
        delete g_scheduler;
        g_scheduler = NULL;
    }
    return 0;
}

int nvtlive555_dbgcmd(int argc, char* argv[]) {
    // Basic debug
    std::cout << "nvtlive555_dbgcmd called (custom implementation)" << std::endl;
    return 0;
}
