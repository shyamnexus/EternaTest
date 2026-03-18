/**
 * @file rtsp_server.cpp
 * @brief Live555 RTSP Server Implementation - Main RtspServer class
 * 
 * Direct HDAL buffer access RTSP server using pure Live555.
 * Provides low-latency streaming with full control over authentication
 * and URL routing.
 * 
 * This file contains:
 * - RtspServerImpl structure
 * - RtspServer class implementation
 * - ONVIF compatibility functions
 * - C interface for ONVIF module
 * 
 * Source/Subsession implementations are in separate files:
 * - hdal_video_source.cpp: HdalVideoSource, HdalVideoSubsession
 * - hdal_audio_source.cpp: HdalAudioSource, HdalAudioSubsession
 * - onvif_rtsp_source.cpp: OnvifMetadataSource, OnvifMetadataSubsession
 * - localhost_bypass_server.cpp: LocalhostBypassRTSPServer
 */

#include "ipcam/rtsp_server.h"
#include "ipcam/onvif_metadata.h"

#ifndef RTSP_SERVER_ENABLED
#define RTSP_SERVER_ENABLED 1
#endif

#if RTSP_SERVER_ENABLED

#include "ipcam/rtsp_sources_internal.h"

namespace ipcam {
namespace streaming {

// ============================================================================
// RtspServerImpl - Live555 implementation details
// ============================================================================

// Forward declaration for event trigger callback
class RtspServer;
static RtspServer* g_rtsp_server_instance = nullptr;

// Static callback for event trigger
static void refreshEventTriggerCallback(void* clientData) {
    (void)clientData;
    if (g_rtsp_server_instance) {
        g_rtsp_server_instance->ProcessPendingRefreshes();
    }
}

struct RtspServerImpl {
    TaskScheduler* scheduler = nullptr;
    UsageEnvironment* env = nullptr;
    RTSPServer* rtsp_server = nullptr;
    UserAuthenticationDatabase* auth_db = nullptr;
    EventLoopWatchVariable event_loop_watch = 0;
    EventTriggerId refresh_trigger = 0;
    
    ~RtspServerImpl() {
        cleanup();
    }
    
    void cleanup() {
        if (refresh_trigger && scheduler) {
            scheduler->deleteEventTrigger(refresh_trigger);
            refresh_trigger = 0;
        }
        if (rtsp_server) {
            Medium::close(rtsp_server);
            rtsp_server = nullptr;
        }
        delete auth_db;
        auth_db = nullptr;
        
        if (env) {
            env->reclaim();
            env = nullptr;
        }
        delete scheduler;
        scheduler = nullptr;
    }
};

// ============================================================================
// RtspServer Implementation
// ============================================================================

RtspServer::RtspServer()
    : impl_(std::make_unique<RtspServerImpl>()) {
}

RtspServer::~RtspServer() {
    Stop();
}

bool RtspServer::Configure(const RtspServerConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    if (running_) {
        spdlog::error("RtspServer: Cannot configure while running");
        return false;
    }
    
    config_ = config;
    spdlog::info("RtspServer: Configured with {} streams on port {}", 
                 config_.streams.size(), config_.rtsp_port);
    
    return true;
}

bool RtspServer::LoadConfig() {
    RtspServerConfig config;
    
    // Load from config system
    config.rtsp_port = config::Get<int>("streaming.rtsp.port", 554);
    config.http_tunnel_port = config::Get<int>("streaming.rtsp.http_tunnel_port", 0);
    config.max_connections = config::Get<int>("streaming.rtsp.max_connections", 10);
    
    // Authentication
    std::string auth_mode_str = config::Get<std::string>("streaming.rtsp.auth.mode", "none");
    if (auth_mode_str == "basic") {
        config.auth_mode = RtspAuthMode::kBasic;
    } else if (auth_mode_str == "digest") {
        config.auth_mode = RtspAuthMode::kDigest;
    } else {
        config.auth_mode = RtspAuthMode::kNone;
    }
    
    config.realm = config::Get<std::string>("streaming.rtsp.auth.realm", "IP Camera");
    
    // Load users from config
    std::string admin_user = config::Get<std::string>("streaming.rtsp.auth.username", "");
    std::string admin_pass = config::Get<std::string>("streaming.rtsp.auth.password", "");
    if (!admin_user.empty() && !admin_pass.empty()) {
        config.users.push_back({admin_user, admin_pass});
    }
    
    // Load stream configurations
    for (int i = 1; i <= kMaxRtspStreams; ++i) {
        std::string prefix = "media.video" + std::to_string(i);
        bool enabled = config::Get<bool>(prefix + ".enabled", i <= 2);
        
        if (enabled) {
            RtspStreamConfig stream;
            stream.channel_id = i - 1;
            stream.stream_name = "stream" + std::to_string(i - 1);
            
            std::string codec_str = config::Get<std::string>(prefix + ".codec", "h264");
            spdlog::debug("RtspServer: LoadConfig video{} codec_str='{}'", i, codec_str);
            if (codec_str == "h265" || codec_str == "hevc") {
                stream.video_codec = RtspVideoCodec::kH265;
            } else {
                stream.video_codec = RtspVideoCodec::kH264;
            }
            
            stream.width = config::Get<int>(prefix + ".width", 1920);
            stream.height = config::Get<int>(prefix + ".height", 1080);
            stream.fps = config::Get<int>(prefix + ".fps", 30);
            stream.bitrate_kbps = config::Get<int>(prefix + ".encoder.bitrate", 2048);
            
            // Audio per stream (gate by global audio enable + per-stream flag)
            bool stream_audio_enabled = config::Get<bool>(prefix + ".audio_enabled", true);
            stream.enable_audio = stream_audio_enabled && config::Get<bool>("media.audio.enabled", false);
            std::string audio_codec = config::Get<std::string>("media.audio.codec", "g711u");
            int audio_bitrate = config::Get<int>("media.audio.bitrate", 64000);
            
            // Convert codec string to enum
            if (audio_codec == "pcmu" || audio_codec == "g711u" || audio_codec == "ulaw" || 
                audio_codec == "g711_ulaw" || audio_codec == "mulaw") {
                stream.audio_codec = RtspAudioCodec::kPcmu;
            } else if (audio_codec == "pcma" || audio_codec == "g711a" || audio_codec == "alaw" ||
                     audio_codec == "g711_alaw") {
                stream.audio_codec = RtspAudioCodec::kPcma;
            } else if (audio_codec == "g726-16" || audio_codec == "g726_16") {
                stream.audio_codec = RtspAudioCodec::kG726_16;
            } else if (audio_codec == "g726-24" || audio_codec == "g726_24") {
                stream.audio_codec = RtspAudioCodec::kG726_24;
            } else if (audio_codec == "g726-32" || audio_codec == "g726_32" || audio_codec == "g726") {
                stream.audio_codec = RtspAudioCodec::kG726_32;
            } else if (audio_codec == "g726-40" || audio_codec == "g726_40") {
                stream.audio_codec = RtspAudioCodec::kG726_40;
            } else if (audio_codec == "aac" || audio_codec == "aac-lc" || audio_codec == "mpeg4-generic") {
                stream.audio_codec = RtspAudioCodec::kAac;
            } else if (audio_codec == "pcm" || audio_codec == "l16" || audio_codec == "raw") {
                stream.audio_codec = RtspAudioCodec::kPcm;
            } else {
                spdlog::warn("RtspServer: Unknown audio codec '{}', defaulting to PCMU", audio_codec);
                stream.audio_codec = RtspAudioCodec::kPcmu;
            }
            
            stream.audio_sample_rate = config::Get<int>("media.audio.sample_rate", 8000);
            stream.audio_channels = config::Get<int>("media.audio.channels", 1);
            stream.audio_bitrate = audio_bitrate / 1000;
            
            spdlog::info("RtspServer: Stream {} audio config - enabled={} codec={} rate={} channels={}",
                         i, stream.enable_audio, audio_codec, stream.audio_sample_rate, stream.audio_channels);
            
            config.streams.push_back(stream);
        }
    }
    
    return Configure(config);
}

bool RtspServer::Start() {
    if (running_) {
        spdlog::warn("RtspServer: Already running");
        return true;
    }
    
    spdlog::info("RtspServer: Starting on port {}", config_.rtsp_port);
    
    // Initialize Live555
    impl_->scheduler = BasicTaskScheduler::createNew();
    impl_->env = BasicUsageEnvironment::createNew(*impl_->scheduler);
    
    // Setup authentication if enabled
    if (config_.auth_mode != RtspAuthMode::kNone && !config_.users.empty()) {
        impl_->auth_db = new UserAuthenticationDatabase(config_.realm.c_str(), False);
        for (const auto& user : config_.users) {
            impl_->auth_db->addUserRecord(user.username.c_str(), user.password.c_str());
        }
        
        spdlog::info("RtspServer: Digest authentication enabled ({} users), localhost bypass enabled", 
                     config_.users.size());
    }
    
    // Create RTSP server with localhost auth bypass
    impl_->rtsp_server = LocalhostBypassRTSPServer::createNew(*impl_->env, config_.rtsp_port, impl_->auth_db);
    
    if (!impl_->rtsp_server) {
        spdlog::error("RtspServer: Failed to create RTSP server on port {}: {}",
                      config_.rtsp_port, impl_->env->getResultMsg());
        impl_->cleanup();
        return false;
    }
    
    // Setup HTTP tunneling if enabled
    if (config_.http_tunnel_port > 0) {
        if (impl_->rtsp_server->setUpTunnelingOverHTTP(config_.http_tunnel_port)) {
            spdlog::info("RtspServer: HTTP tunneling enabled on port {}", config_.http_tunnel_port);
        }
    }
    
    // Create sessions for each stream
    for (const auto& stream_cfg : config_.streams) {
        ServerMediaSession* session = ServerMediaSession::createNew(
            *impl_->env,
            stream_cfg.stream_name.c_str(),
            stream_cfg.stream_name.c_str(),
            "IP Camera Stream");
        
        // Add video subsession
        RtspVideoCodec codec = stream_cfg.video_codec;
        spdlog::info("RtspServer: Creating stream '{}' channel={} codec={}", 
                     stream_cfg.stream_name, stream_cfg.channel_id,
                     codec == RtspVideoCodec::kH265 ? "H.265" : "H.264");
        HdalVideoSubsession* video_subsession = 
            HdalVideoSubsession::createNew(*impl_->env, stream_cfg.channel_id, codec);
        session->addSubsession(video_subsession);
        
        // Add audio subsession if enabled
        if (stream_cfg.enable_audio && stream_cfg.audio_codec != RtspAudioCodec::kNone) {
            spdlog::info("RtspServer: Adding audio to stream '{}' codec={} rate={} ch={}",
                         stream_cfg.stream_name,
                         stream_cfg.audio_codec == RtspAudioCodec::kPcmu ? "PCMU" :
                         (stream_cfg.audio_codec == RtspAudioCodec::kPcma ? "PCMA" : "PCM"),
                         stream_cfg.audio_sample_rate,
                         stream_cfg.audio_channels);
            
            HdalAudioSubsession* audio_subsession = 
                HdalAudioSubsession::createNew(*impl_->env, 
                                               stream_cfg.audio_codec,
                                               stream_cfg.audio_sample_rate,
                                               stream_cfg.audio_channels);
            session->addSubsession(audio_subsession);
        }
        
        impl_->rtsp_server->addServerMediaSession(session);
        
        char* url = impl_->rtsp_server->rtspURL(session);
        spdlog::info("RtspServer: Stream '{}' available at {}", stream_cfg.stream_name, url);
        delete[] url;
    }
    
    // Create event trigger for stream refresh
    g_rtsp_server_instance = this;
    impl_->refresh_trigger = impl_->env->taskScheduler().createEventTrigger(refreshEventTriggerCallback);
    if (impl_->refresh_trigger == 0) {
        spdlog::warn("RtspServer: Failed to create refresh event trigger");
    }
    
    // Start event loop thread
    running_ = true;
    stop_requested_ = false;
    impl_->event_loop_watch = 0;
    
    event_loop_thread_ = std::thread(&RtspServer::EventLoopThread, this);
    
    spdlog::info("RtspServer: Started successfully");
    return true;
}

bool RtspServer::Stop() {
    if (!running_) {
        return true;
    }
    
    spdlog::info("RtspServer: Stopping...");
    
    // Signal event loop to stop
    stop_requested_ = true;
    impl_->event_loop_watch = 1;
    
    // Wait for event loop thread
    if (event_loop_thread_.joinable()) {
        event_loop_thread_.join();
    }
    
    // Cleanup
    impl_->cleanup();
    
    running_ = false;
    spdlog::info("RtspServer: Stopped");
    
    return true;
}

void RtspServer::EventLoopThread() {
    spdlog::debug("RtspServer: Event loop started");
    impl_->env->taskScheduler().doEventLoop(&impl_->event_loop_watch);
    spdlog::debug("RtspServer: Event loop ended");
}

bool RtspServer::AddUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    config_.users.push_back({username, password});
    
    if (impl_->auth_db) {
        impl_->auth_db->addUserRecord(username.c_str(), password.c_str());
    }
    
    spdlog::info("RtspServer: Added user '{}'", username);
    return true;
}

bool RtspServer::RemoveUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    auto it = std::remove_if(config_.users.begin(), config_.users.end(),
        [&](const RtspUser& u) { return u.username == username; });
    
    if (it != config_.users.end()) {
        config_.users.erase(it, config_.users.end());
        spdlog::info("RtspServer: Removed user '{}' (requires restart)", username);
        return true;
    }
    
    return false;
}

bool RtspServer::ForceKeyframe(int channel_id) {
    auto& pipeline = platform::HdalPipeline::Instance();
    return pipeline.ForceIdr(channel_id);
}

bool RtspServer::RefreshStream(int channel_id) {
    if (!running_ || !impl_->rtsp_server) {
        spdlog::warn("RtspServer::RefreshStream: Server not running");
        return false;
    }
    
    // Queue the refresh request
    {
        std::lock_guard<std::mutex> lock(refresh_mutex_);
        bool already_queued = false;
        for (int id : pending_refreshes_) {
            if (id == channel_id) {
                already_queued = true;
                break;
            }
        }
        if (!already_queued) {
            pending_refreshes_.push_back(channel_id);
            spdlog::info("RtspServer::RefreshStream: Queued refresh for channel {}", channel_id);
        }
    }
    
    // Trigger the event
    if (impl_->refresh_trigger != 0) {
        impl_->env->taskScheduler().triggerEvent(impl_->refresh_trigger, nullptr);
    }
    
    return true;
}

void RtspServer::ProcessPendingRefreshes() {
    if (!running_ || !impl_->rtsp_server) {
        spdlog::debug("RtspServer::ProcessPendingRefreshes: Server not running, skipping");
        return;
    }
    
    std::vector<int> to_refresh;
    {
        std::lock_guard<std::mutex> lock(refresh_mutex_);
        std::swap(to_refresh, pending_refreshes_);
    }
    
    for (int channel_id : to_refresh) {
        spdlog::info("RtspServer::ProcessPendingRefreshes: Processing channel {}", channel_id);
        DoRefreshStream(channel_id);
    }
}

void RtspServer::DoRefreshStream(int channel_id) {
    if (!impl_->rtsp_server) {
        spdlog::warn("RtspServer::DoRefreshStream: Server not available");
        return;
    }
    
    // Find the stream config for this channel
    RtspStreamConfig* stream_cfg = nullptr;
    std::string prefix = "media.video" + std::to_string(channel_id + 1);
    bool stream_enabled = config::Get<bool>(prefix + ".enabled", channel_id < 2);

    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        for (auto& cfg : config_.streams) {
            if (cfg.channel_id == channel_id) {
                stream_cfg = &cfg;
                break;
            }
        }

        // If stream was previously disabled at boot, it will not exist in config_.streams.
        // Create it on-demand so refresh can recreate the RTSP session when the stream is enabled later.
        if (!stream_cfg && stream_enabled) {
            RtspStreamConfig new_cfg;
            new_cfg.channel_id = channel_id;
            new_cfg.stream_name = "stream" + std::to_string(channel_id);

            std::string codec_str = config::Get<std::string>(prefix + ".codec", "h264");
            new_cfg.video_codec = (codec_str == "h265" || codec_str == "hevc") ?
                                   RtspVideoCodec::kH265 : RtspVideoCodec::kH264;

            new_cfg.width = config::Get<int>(prefix + ".width", 1920);
            new_cfg.height = config::Get<int>(prefix + ".height", 1080);
            new_cfg.fps = config::Get<int>(prefix + ".fps", 30);
            new_cfg.bitrate_kbps = config::Get<int>(prefix + ".encoder.bitrate", 2048);

            // Audio gate
            bool stream_audio_enabled = config::Get<bool>(prefix + ".audio_enabled", true);
            new_cfg.enable_audio = stream_audio_enabled && config::Get<bool>("media.audio.enabled", false);
            new_cfg.audio_codec = RtspAudioCodec::kPcmu;
            new_cfg.audio_sample_rate = config::Get<int>("media.audio.sample_rate", 8000);
            new_cfg.audio_channels = config::Get<int>("media.audio.channels", 1);
            new_cfg.audio_bitrate = config::Get<int>("media.audio.bitrate", 64000) / 1000;

            config_.streams.push_back(new_cfg);
            stream_cfg = &config_.streams.back();
            spdlog::info("RtspServer::DoRefreshStream: Added stream{} config for channel {} on refresh", 
                         channel_id + 1, channel_id);
        }
    }

    if (!stream_cfg) {
        spdlog::warn("RtspServer::DoRefreshStream: No stream for channel {} (enabled={})", 
                     channel_id, stream_enabled);
        return;
    }
    
    spdlog::info("RtspServer::DoRefreshStream: Refreshing stream '{}' for channel {}", 
                 stream_cfg->stream_name, channel_id);
    
    // Close all client connections
    spdlog::debug("RtspServer::DoRefreshStream: Closing all client sessions...");
    impl_->rtsp_server->closeAllClientSessionsForServerMediaSession(stream_cfg->stream_name.c_str());
    usleep(100000);  // 100ms
    
    // Delete the session
    spdlog::debug("RtspServer::DoRefreshStream: Deleting media session...");
    impl_->rtsp_server->deleteServerMediaSession(stream_cfg->stream_name.c_str());
    usleep(50000);  // 50ms
    
    // Update codec from config
    if (!stream_enabled) {
        spdlog::info("RtspServer::DoRefreshStream: stream{} disabled in config, removing session", channel_id + 1);
        return;  // session already deleted above, do not recreate
    }

    std::string codec_str = config::Get<std::string>(prefix + ".codec", "h264");
    if (codec_str == "h265" || codec_str == "hevc") {
        stream_cfg->video_codec = RtspVideoCodec::kH265;
    } else {
        stream_cfg->video_codec = RtspVideoCodec::kH264;
    }
    
    // Update audio config (respect per-stream enable + global audio gate)
    bool stream_audio_enabled = config::Get<bool>(prefix + ".audio_enabled", true);
    stream_cfg->enable_audio = stream_audio_enabled && config::Get<bool>("media.audio.enabled", false);
    std::string audio_codec = config::Get<std::string>("media.audio.codec", "g711u");
    stream_cfg->audio_sample_rate = config::Get<int>("media.audio.sample_rate", 8000);
    stream_cfg->audio_channels = config::Get<int>("media.audio.channels", 1);
    
    // Parse audio codec
    if (audio_codec == "pcmu" || audio_codec == "g711u" || audio_codec == "ulaw" || 
        audio_codec == "g711_ulaw" || audio_codec == "mulaw") {
        stream_cfg->audio_codec = RtspAudioCodec::kPcmu;
    } else if (audio_codec == "pcma" || audio_codec == "g711a" || audio_codec == "alaw" ||
               audio_codec == "g711_alaw") {
        stream_cfg->audio_codec = RtspAudioCodec::kPcma;
    } else if (audio_codec == "g726-16" || audio_codec == "g726_16") {
        stream_cfg->audio_codec = RtspAudioCodec::kG726_16;
    } else if (audio_codec == "g726-24" || audio_codec == "g726_24") {
        stream_cfg->audio_codec = RtspAudioCodec::kG726_24;
    } else if (audio_codec == "g726-32" || audio_codec == "g726_32" || audio_codec == "g726") {
        stream_cfg->audio_codec = RtspAudioCodec::kG726_32;
    } else if (audio_codec == "g726-40" || audio_codec == "g726_40") {
        stream_cfg->audio_codec = RtspAudioCodec::kG726_40;
    } else if (audio_codec == "aac" || audio_codec == "aac-lc") {
        stream_cfg->audio_codec = RtspAudioCodec::kAac;
    } else if (audio_codec == "pcm" || audio_codec == "l16" || audio_codec == "raw") {
        stream_cfg->audio_codec = RtspAudioCodec::kPcm;
    } else {
        stream_cfg->audio_codec = RtspAudioCodec::kPcmu;
    }
    
    // Create new session
    ServerMediaSession* session = ServerMediaSession::createNew(
        *impl_->env,
        stream_cfg->stream_name.c_str(),
        stream_cfg->stream_name.c_str(),
        "IP Camera Stream");
    
    RtspVideoCodec codec = stream_cfg->video_codec;
    spdlog::info("RtspServer: Recreating stream '{}' channel={} codec={}", 
                 stream_cfg->stream_name, channel_id,
                 codec == RtspVideoCodec::kH265 ? "H.265" : "H.264");
    
    HdalVideoSubsession* video_subsession = 
        HdalVideoSubsession::createNew(*impl_->env, channel_id, codec);
    session->addSubsession(video_subsession);
    
    // Add audio subsession if enabled
    if (stream_cfg->enable_audio && stream_cfg->audio_codec != RtspAudioCodec::kNone) {
        const char* codec_name = "unknown";
        switch (stream_cfg->audio_codec) {
            case RtspAudioCodec::kPcmu: codec_name = "PCMU"; break;
            case RtspAudioCodec::kPcma: codec_name = "PCMA"; break;
            case RtspAudioCodec::kG726_16: codec_name = "G726-16"; break;
            case RtspAudioCodec::kG726_24: codec_name = "G726-24"; break;
            case RtspAudioCodec::kG726_32: codec_name = "G726-32"; break;
            case RtspAudioCodec::kG726_40: codec_name = "G726-40"; break;
            case RtspAudioCodec::kAac: codec_name = "AAC"; break;
            case RtspAudioCodec::kPcm: codec_name = "PCM"; break;
            default: break;
        }
        spdlog::info("RtspServer: Adding audio to refreshed stream '{}' codec={} rate={} ch={}",
                     stream_cfg->stream_name, codec_name,
                     stream_cfg->audio_sample_rate, stream_cfg->audio_channels);
        
        HdalAudioSubsession* audio_subsession = 
            HdalAudioSubsession::createNew(*impl_->env, 
                                           stream_cfg->audio_codec,
                                           stream_cfg->audio_sample_rate,
                                           stream_cfg->audio_channels);
        session->addSubsession(audio_subsession);
    }
    
    impl_->rtsp_server->addServerMediaSession(session);
    
    char* url = impl_->rtsp_server->rtspURL(session);
    spdlog::info("RtspServer: Stream refreshed, available at {}", url);
    delete[] url;
}

int RtspServer::GetClientCount() const {
    return 0;
}

std::string RtspServer::GetStreamUrl(const std::string& stream_name) const {
    if (!impl_->rtsp_server) {
        return "";
    }
    
    char ipAddr[32] = {0};
    
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == 0) {
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) continue;
            if (ifa->ifa_addr->sa_family == AF_INET) {
                if (strcmp(ifa->ifa_name, "lo") == 0) continue;
                struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
                inet_ntop(AF_INET, &addr->sin_addr, ipAddr, sizeof(ipAddr));
                break;
            }
        }
        freeifaddrs(ifaddr);
    }
    
    if (ipAddr[0] == '\0') {
        strcpy(ipAddr, "127.0.0.1");
    }
    
    char url[256];
    snprintf(url, sizeof(url), "rtsp://%s:%d/%s", ipAddr, config_.rtsp_port, stream_name.c_str());
    return std::string(url);
}

void RtspServer::SetEventCallback(RtspEventCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    event_callback_ = std::move(callback);
}

void RtspServer::FireEvent(RtspEvent event, const std::string& stream_name,
                           const std::string& client_ip, const std::string& message) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (event_callback_) {
        event_callback_(event, stream_name, client_ip, message);
    }
}

// ============================================================================
// Global Instance
// ============================================================================

RtspServer& GetRtspServer() {
    static RtspServer instance;
    return instance;
}

// ============================================================================
// ONVIF Compatibility Implementation
// ============================================================================

int OnvifProfileMapping::ProfileTokenToStreamIndex(const std::string& profile_token) {
    if (profile_token == "Profile_1" || profile_token == "profile_1" || profile_token == "profile1") {
        return 0;
    } else if (profile_token == "Profile_2" || profile_token == "profile_2" || profile_token == "profile2") {
        return 1;
    } else if (profile_token == "Profile_3" || profile_token == "profile_3" || profile_token == "profile3") {
        return 2;
    }
    
    if (profile_token == "stream0") return 0;
    if (profile_token == "stream1") return 1;
    if (profile_token == "stream2") return 2;
    
    return -1;
}

std::string OnvifProfileMapping::StreamIndexToProfileToken(int stream_index) {
    if (stream_index >= 0 && stream_index < kMaxProfiles) {
        return "Profile_" + std::to_string(stream_index + 1);
    }
    return "";
}

std::string OnvifProfileMapping::ProfileTokenToStreamName(const std::string& profile_token) {
    int index = ProfileTokenToStreamIndex(profile_token);
    if (index >= 0) {
        return "stream" + std::to_string(index);
    }
    return "";
}

std::string GetOnvifStreamUri(const std::string& profile_token, bool use_rtsp_over_tcp) {
    auto& server = GetRtspServer();
    std::string stream_name = OnvifProfileMapping::ProfileTokenToStreamName(profile_token);
    
    if (stream_name.empty()) {
        spdlog::warn("ONVIF: Invalid profile token '{}'", profile_token);
        return "";
    }
    
    std::string uri = server.GetStreamUrl(stream_name);
    (void)use_rtsp_over_tcp;  // Transport negotiation happens during RTSP SETUP
    
    return uri;
}

bool GetOnvifStreamInfo(const std::string& profile_token,
                        int& width, int& height, int& fps, std::string& codec) {
    int stream_index = OnvifProfileMapping::ProfileTokenToStreamIndex(profile_token);
    if (stream_index < 0) {
        return false;
    }
    
    auto& server = GetRtspServer();
    const auto& config = server.GetConfig();
    
    for (const auto& stream : config.streams) {
        if (stream.channel_id == stream_index) {
            width = stream.width;
            height = stream.height;
            fps = stream.fps;
            codec = (stream.video_codec == RtspVideoCodec::kH265) ? "H265" : "H264";
            return true;
        }
    }
    
    return false;
}

} // namespace streaming
} // namespace ipcam

// ============================================================================
// C Interface Implementation for ONVIF Module
// ============================================================================

extern "C" {

int rtsp_get_onvif_stream_uri(const char* profile_token, const char* ip_address,
                               char* uri_buffer, int buffer_size) {
    if (!profile_token || !ip_address || !uri_buffer || buffer_size <= 0) {
        return 0;
    }
    
    std::string token(profile_token);
    int stream_index = ipcam::streaming::OnvifProfileMapping::ProfileTokenToStreamIndex(token);
    
    if (stream_index < 0) {
        return 0;
    }
    
    auto& server = ipcam::streaming::GetRtspServer();
    int port = server.GetConfig().rtsp_port;
    if (port <= 0) {
        port = 554;
    }
    
    std::string stream_name = "stream" + std::to_string(stream_index);
    std::string uri = "rtsp://" + std::string(ip_address) + ":" + std::to_string(port) + "/" + stream_name;
    
    if ((int)uri.size() >= buffer_size) {
        return 0;
    }
    
    strncpy(uri_buffer, uri.c_str(), buffer_size);
    uri_buffer[buffer_size - 1] = '\0';
    
    return 1;
}

int rtsp_get_onvif_stream_info(const char* profile_token, 
                                int* width, int* height, int* fps, int* is_h265) {
    if (!profile_token) {
        return 0;
    }
    
    std::string token(profile_token);
    std::string codec;
    int w = 0, h = 0, f = 0;
    
    if (!ipcam::streaming::GetOnvifStreamInfo(token, w, h, f, codec)) {
        return 0;
    }
    
    if (width) *width = w;
    if (height) *height = h;
    if (fps) *fps = f;
    if (is_h265) *is_h265 = (codec == "H265") ? 1 : 0;
    
    return 1;
}

int rtsp_server_is_running(void) {
    return ipcam::streaming::GetRtspServer().IsRunning() ? 1 : 0;
}

int rtsp_server_get_port(void) {
    return ipcam::streaming::GetRtspServer().GetConfig().rtsp_port;
}

int rtsp_server_get_stream_count(void) {
    return static_cast<int>(ipcam::streaming::GetRtspServer().GetConfig().streams.size());
}

}  // extern "C"

#else // !RTSP_SERVER_ENABLED

// ============================================================================
// Stub Implementation when Live555 is not available
// ============================================================================

#include <iostream>

namespace ipcam {
namespace streaming {

struct RtspServerImpl {};

RtspServer::RtspServer() : impl_(std::make_unique<RtspServerImpl>()) {}
RtspServer::~RtspServer() {}

bool RtspServer::Configure(const RtspServerConfig&) { 
    std::cerr << "RTSP server disabled - Live555 not built\n";
    return false; 
}
bool RtspServer::LoadConfig() { return false; }
bool RtspServer::Start() { return false; }
bool RtspServer::Stop() { return true; }
bool RtspServer::AddUser(const std::string&, const std::string&) { return false; }
bool RtspServer::RemoveUser(const std::string&) { return false; }
bool RtspServer::ForceKeyframe(int) { return false; }
bool RtspServer::RefreshStream(int) { return false; }
int RtspServer::GetClientCount() const { return 0; }
std::string RtspServer::GetStreamUrl(const std::string&) const { return ""; }
void RtspServer::SetEventCallback(RtspEventCallback) {}
void RtspServer::EventLoopThread() {}
void RtspServer::FireEvent(RtspEvent, const std::string&, const std::string&, const std::string&) {}

// ONVIF stubs
int OnvifProfileMapping::ProfileTokenToStreamIndex(const std::string&) { return -1; }
std::string OnvifProfileMapping::StreamIndexToProfileToken(int) { return ""; }
std::string OnvifProfileMapping::ProfileTokenToStreamName(const std::string&) { return ""; }
std::string GetOnvifStreamUri(const std::string&, bool) { return ""; }
bool GetOnvifStreamInfo(const std::string&, int&, int&, int&, std::string&) { return false; }

RtspServer& GetRtspServer() {
    static RtspServer instance;
    return instance;
}

} // namespace streaming
} // namespace ipcam

// C interface stubs
extern "C" {
int rtsp_get_onvif_stream_uri(const char*, const char*, char*, int) { return 0; }
int rtsp_get_onvif_stream_info(const char*, int*, int*, int*, int*) { return 0; }
int rtsp_server_is_running(void) { return 0; }
int rtsp_server_get_port(void) { return 554; }
int rtsp_server_get_stream_count(void) { return 0; }
}

#endif // RTSP_SERVER_ENABLED
