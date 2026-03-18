/**
 * @file rtsp_server.h
 * @brief Live555-based RTSP Server for IP Camera
 * 
 * Pure Live555 RTSP server implementation that directly accesses HDAL
 * video encoder buffers. Provides full control over authentication,
 * URL routing, and streaming behavior.
 * 
 * Features:
 * - Multi-channel support (up to 4 video streams)
 * - Digest/Basic authentication
 * - H.264 and H.265 codec support
 * - G.711 audio support (optional)
 * - Low-latency direct buffer access
 * - Thread-safe start/stop
 * 
 * Copyright (c) 2025
 */

#ifndef IPCAM_RTSP_SERVER_H
#define IPCAM_RTSP_SERVER_H

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>

namespace ipcam {
namespace streaming {

// ============================================================================
// Forward Declarations (Live555 types - avoid including headers here)
// ============================================================================
struct RtspServerImpl;

// ============================================================================
// Constants
// ============================================================================
constexpr int kMaxRtspStreams = 3;  // 3 HDAL video encoders (main, sub, third)
constexpr int kDefaultRtspPort = 554;
constexpr int kDefaultHttpTunnelPort = 8554;

// ============================================================================
// Configuration
// ============================================================================

/**
 * @brief Video codec type for RTSP streaming
 */
enum class RtspVideoCodec {
    kH264,
    kH265,
    kMjpeg
};

/**
 * @brief Audio codec type for RTSP streaming
 */
enum class RtspAudioCodec {
    kNone,
    kPcmu,      // G.711 μ-law (RTP payload 0)
    kPcma,      // G.711 A-law (RTP payload 8)
    kG726_16,   // G.726 16 kbps (2 bits/sample)
    kG726_24,   // G.726 24 kbps (3 bits/sample)
    kG726_32,   // G.726 32 kbps (4 bits/sample) - standard
    kG726_40,   // G.726 40 kbps (5 bits/sample)
    kAac,       // AAC-LC (if available)
    kPcm        // Raw PCM (L16)
};

/**
 * @brief Authentication mode for RTSP server
 */
enum class RtspAuthMode {
    kNone,      // No authentication required
    kBasic,     // HTTP Basic authentication
    kDigest     // HTTP Digest authentication (more secure)
};

/**
 * @brief Configuration for a single RTSP stream
 */
struct RtspStreamConfig {
    int channel_id = 0;               // HDAL video encoder channel (0-3)
    std::string stream_name;          // URL path (e.g., "stream1", "live/ch0")
    RtspVideoCodec video_codec = RtspVideoCodec::kH264;
    int width = 1920;
    int height = 1080;
    int fps = 30;
    int bitrate_kbps = 2048;
    
    // Audio settings (optional)
    bool enable_audio = false;
    RtspAudioCodec audio_codec = RtspAudioCodec::kPcmu;
    int audio_sample_rate = 8000;
    int audio_channels = 1;
    int audio_bitrate = 32;  // G.726 bitrate (16/24/32/40 kbps)
    
    // ONVIF metadata settings (optional)
    bool enable_onvif_metadata = false;  // Enable ONVIF metadata track (vnd.onvif.metadata)
};

/**
 * @brief User credentials for authentication
 */
struct RtspUser {
    std::string username;
    std::string password;
};


/**
 * @brief RTSP server configuration
 */
struct RtspServerConfig {
    // Network settings
    int rtsp_port = kDefaultRtspPort;
    int http_tunnel_port = 0;         // 0 = disabled, else enable HTTP tunneling
    int max_connections = 10;
    
    // Authentication
    RtspAuthMode auth_mode = RtspAuthMode::kNone;
    std::vector<RtspUser> users;      // Authorized users (if auth enabled)
    std::string realm = "IP Camera";  // Authentication realm
    
    // Stream configuration
    std::vector<RtspStreamConfig> streams;
    
    // Performance tuning
    int send_buffer_size = 2 * 1024 * 1024;  // 2MB socket send buffer
    int max_packet_size = 1500;              // MTU size
    bool enable_rtp_over_tcp = true;         // Allow interleaved mode
};

// ============================================================================
// Callbacks
// ============================================================================

/**
 * @brief RTSP event types
 */
enum class RtspEvent {
    kClientConnect,
    kClientDisconnect,
    kStreamStart,
    kStreamStop,
    kError
};

/**
 * @brief RTSP event callback
 * @param event Event type
 * @param stream_name Stream name (e.g., "stream1")
 * @param client_ip Client IP address (for connect/disconnect)
 * @param message Optional message (for errors)
 */
using RtspEventCallback = std::function<void(RtspEvent event, 
                                              const std::string& stream_name,
                                              const std::string& client_ip,
                                              const std::string& message)>;

// ============================================================================
// RtspServer Class
// ============================================================================

/**
 * @brief Live555-based RTSP Server
 * 
 * Usage:
 * @code
 * RtspServerConfig config;
 * config.rtsp_port = 554;
 * config.auth_mode = RtspAuthMode::kDigest;
 * config.users.push_back({"admin", "password123"});
 * 
 * RtspStreamConfig stream;
 * stream.channel_id = 0;
 * stream.stream_name = "stream1";
 * stream.video_codec = RtspVideoCodec::kH264;
 * config.streams.push_back(stream);
 * 
 * RtspServer server;
 * server.Configure(config);
 * server.Start();
 * // ... server runs in background ...
 * server.Stop();
 * @endcode
 */
class RtspServer {
public:
    RtspServer();
    ~RtspServer();
    
    // Prevent copying
    RtspServer(const RtspServer&) = delete;
    RtspServer& operator=(const RtspServer&) = delete;
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * @brief Configure the RTSP server
     * @param config Server configuration
     * @return true on success
     * 
     * Must be called before Start(). Can be called again after Stop().
     */
    bool Configure(const RtspServerConfig& config);
    
    /**
     * @brief Load configuration from config system (streaming.json)
     * @return true on success
     */
    bool LoadConfig();
    
    /**
     * @brief Get current configuration
     */
    const RtspServerConfig& GetConfig() const { return config_; }
    
    // ========================================================================
    // Lifecycle
    // ========================================================================
    
    /**
     * @brief Start the RTSP server
     * @return true on success
     * 
     * Starts the event loop in a background thread.
     */
    bool Start();
    
    /**
     * @brief Stop the RTSP server
     * @return true on success
     * 
     * Stops the event loop and disconnects all clients.
     */
    bool Stop();
    
    /**
     * @brief Check if server is running
     */
    bool IsRunning() const { return running_.load(); }
    
    // ========================================================================
    // Runtime Control
    // ========================================================================
    
    /**
     * @brief Add a user for authentication
     * @param username Username
     * @param password Password
     * @return true on success
     */
    bool AddUser(const std::string& username, const std::string& password);
    
    /**
     * @brief Remove a user from authentication
     * @param username Username to remove
     * @return true if user was found and removed
     */
    bool RemoveUser(const std::string& username);
    
    /**
     * @brief Force keyframe on a stream (for seeking/reconnect)
     * @param channel_id Video encoder channel
     * @return true on success
     */
    bool ForceKeyframe(int channel_id);
    
    /**
     * @brief Refresh a stream (recreate session when codec changes)
     * @param channel_id Video encoder channel
     * @return true on success
     * 
     * Call this when the video codec changes via API. This will disconnect
     * any existing clients and recreate the session with the new codec.
     * Thread-safe: can be called from any thread.
     */
    bool RefreshStream(int channel_id);
    
    /**
     * @brief Process pending stream refreshes (called from event loop)
     * Internal method - called by event trigger callback
     */
    void ProcessPendingRefreshes();
    
    /**
     * @brief Get number of connected clients
     */
    int GetClientCount() const;
    
    /**
     * @brief Get RTSP URL for a stream
     * @param stream_name Stream name
     * @return Full RTSP URL (e.g., "rtsp://192.168.1.100:554/stream1")
     */
    std::string GetStreamUrl(const std::string& stream_name) const;
    
    // ========================================================================
    // Events
    // ========================================================================
    
    /**
     * @brief Set event callback
     * @param callback Callback function
     */
    void SetEventCallback(RtspEventCallback callback);
    
private:
    // Event loop thread function
    void EventLoopThread();
    
    // Configuration
    RtspServerConfig config_;
    mutable std::mutex config_mutex_;
    
    // State
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_requested_{false};
    std::thread event_loop_thread_;
    
    // Live555 implementation (pimpl idiom to avoid Live555 headers in public API)
    std::unique_ptr<RtspServerImpl> impl_;
    
    // Pending stream refreshes (thread-safe queue for cross-thread refresh requests)
    std::vector<int> pending_refreshes_;
    std::mutex refresh_mutex_;
    
    // Internal method to actually refresh stream (must be called from event loop thread)
    void DoRefreshStream(int channel_id);
    
    // Event callback
    RtspEventCallback event_callback_;
    std::mutex callback_mutex_;
    
    // Helper to fire events
    void FireEvent(RtspEvent event, const std::string& stream_name,
                   const std::string& client_ip, const std::string& message);
};

// ============================================================================
// Singleton Access (optional)
// ============================================================================

/**
 * @brief Get global RTSP server instance
 * 
 * For applications that want a single RTSP server instance.
 */
RtspServer& GetRtspServer();

// ============================================================================
// ONVIF Compatibility
// ============================================================================

/**
 * @brief ONVIF profile token to stream mapping
 * 
 * ONVIF uses profile tokens like "Profile_1", "Profile_2", "Profile_3".
 * These map to our stream names: "stream0", "stream1", "stream2".
 * Matches HDAL video encoders: main stream, sub stream, third stream.
 */
struct OnvifProfileMapping {
    static constexpr int kMaxProfiles = 3;  // 3 HDAL video encoders
    
    /**
     * @brief Convert ONVIF profile token to stream index
     * @param profile_token ONVIF profile token (e.g., "Profile_1")
     * @return Stream index (0-2) or -1 if invalid
     */
    static int ProfileTokenToStreamIndex(const std::string& profile_token);
    
    /**
     * @brief Convert stream index to ONVIF profile token
     * @param stream_index Stream index (0-3)
     * @return ONVIF profile token (e.g., "Profile_1")
     */
    static std::string StreamIndexToProfileToken(int stream_index);
    
    /**
     * @brief Get stream name for ONVIF profile
     * @param profile_token ONVIF profile token
     * @return Stream name (e.g., "stream0") or empty if invalid
     */
    static std::string ProfileTokenToStreamName(const std::string& profile_token);
};

/**
 * @brief Get ONVIF-compatible stream URI
 * @param profile_token ONVIF profile token (e.g., "Profile_1")
 * @param use_rtsp_over_tcp Whether to use TCP (interleaved) transport
 * @return Full RTSP URL or empty string if profile not found
 */
std::string GetOnvifStreamUri(const std::string& profile_token, bool use_rtsp_over_tcp = false);

/**
 * @brief Get stream info for ONVIF Media Service
 * @param profile_token ONVIF profile token
 * @param[out] width Video width
 * @param[out] height Video height
 * @param[out] fps Frame rate
 * @param[out] codec Codec name ("H264" or "H265")
 * @return true if profile exists
 */
bool GetOnvifStreamInfo(const std::string& profile_token,
                        int& width, int& height, int& fps, std::string& codec);

} // namespace streaming
} // namespace ipcam

// Include C interface for ONVIF module compatibility
#include "rtsp_server_onvif.h"

#endif // IPCAM_RTSP_SERVER_H
