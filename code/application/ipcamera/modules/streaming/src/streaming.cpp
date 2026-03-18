/**
 * @file streaming.cpp
 * @brief Streaming Module Implementation
 * 
 * Provides module-level interface for streaming services.
 */

#include "ipcam/streaming.h"
#include "ipcam/rtsp_server.h"
#include "ipcam/audio_control.h"
#include "ipcam/audio_frame_broadcaster.h"
#include <iostream>

namespace ipcam {
namespace streaming {

void dummy_stream(){ std::cout << "streaming init\n"; }

#if RTSP_SERVER_ENABLED

bool Initialize() {
    // Register callback for audio processor reconfiguration
    media::AudioControl::Instance().SetReconfigureCallback([]() {
        auto& broadcaster = AudioFrameBroadcaster::Instance();
        if (broadcaster.IsInitialized()) {
            broadcaster.ReconfigureAudioProcessor();
        }
    });

    // Load RTSP server configuration from config system
    return GetRtspServer().LoadConfig();
}

bool Start() {
    return GetRtspServer().Start();
}

void Stop() {
    GetRtspServer().Stop();
}

bool IsRunning() {
    return GetRtspServer().IsRunning();
}

#else

// Stubs when RTSP server is disabled (Live555 not built)
bool Initialize() {
    std::cerr << "RTSP server disabled - Live555 not available\n";
    return false;
}

bool Start() { return false; }
void Stop() {}
bool IsRunning() { return false; }

#endif

} // namespace streaming
} // namespace ipcam
