#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <cstdint>
#include <ipcam/result.h>

namespace ipcam {
namespace networking {

/**
 * @brief NTP time synchronization manager
 * 
 * Handles all NTP-related functionality:
 * - Pure C++ NTP implementation (no external ntpd dependency)
 * - Time synchronization with configurable servers
 * - Automatic fallback to backup servers and direct IPs
 * - Internet connectivity checking via DNS/socket
 * - Periodic re-sync in background thread
 * 
 * Default NTP servers (in order of priority):
 * - pool.ntp.org (0-3.pool.ntp.org)
 * - time.google.com
 * - time.cloudflare.com
 * - time.windows.com
 * - time.nist.gov
 * - time.apple.com
 * - ntp.ubuntu.com
 * 
 * Fallback IPs when DNS unavailable:
 * - Google: 216.239.35.0/4/8/12
 * - Cloudflare: 162.159.200.1/123
 * - NIST: 129.6.15.28/29
 * - Apple: 17.253.34.123
 */
class NtpManager {
public:
    /// NTP server source
    enum class Source {
        Auto,       ///< Try DHCP first, then manual servers
        Dhcp,       ///< DHCP only (from router/network)
        Manual      ///< Manual servers only (from config)
    };
    
    /// NTP configuration (loaded from network.json)
    struct Config {
        bool enabled = true;                          ///< NTP enabled
        Source source = Source::Auto;                 ///< Server source (auto/dhcp/manual)
        std::string primary_server = "pool.ntp.org";  ///< Primary NTP server
        int port = 123;                               ///< NTP server port (default 123)
        std::vector<std::string> fallback_servers;    ///< Fallback servers from config
        std::vector<std::string> fallback_ips;        ///< Direct IP fallbacks when DNS fails
        bool use_fallback_ips = true;                 ///< Use IP fallbacks if DNS fails
        int sync_timeout_sec = 10;                    ///< Timeout for each sync attempt
        int retry_count = 3;                          ///< Retries per server
        int refresh_interval_sec = 3600;              ///< Periodic sync interval (1 hour)
        std::string timezone = "UTC";                 ///< Timezone setting
    };

    /// DST (Daylight Saving Time) configuration
    struct DstConfig {
        bool enabled = false;                         ///< DST enabled
        std::string mode = "auto";                    ///< "auto" (from IANA tz) or "custom"
        int offset_minutes = 60;                      ///< DST offset in minutes (typically 60)
        int start_month = 3;                          ///< 1-12 (March)
        int start_week = 2;                           ///< 1-5 (2nd week), 5 = last
        int start_day_of_week = 0;                    ///< 0=Sun, 1=Mon, ... 6=Sat
        int start_hour = 2;                           ///< Hour (0-23)
        int end_month = 11;                           ///< 1-12 (November)
        int end_week = 1;                             ///< 1-5 (1st week), 5 = last
        int end_day_of_week = 0;                      ///< 0=Sun
        int end_hour = 2;                             ///< Hour (0-23)
    };

    /**
     * @brief Get current DST configuration
     */
    DstConfig GetDstConfig() const;

    /**
     * @brief Set DST configuration and persist
     */
    Result<void> SetDstConfig(const DstConfig& cfg);
    
    /// Sync result status
    enum class SyncStatus {
        Success,            ///< Time synced successfully
        NoInternet,         ///< No internet connectivity
        AllServersFailed,   ///< All servers unreachable
        Disabled,           ///< NTP is disabled
        InProgress,         ///< Sync already in progress
        Timeout,            ///< Sync timed out
        Error               ///< Other error
    };
    
    /// Sync result details
    struct SyncResult {
        SyncStatus status = SyncStatus::Error;
        std::string server_used;    ///< Server that succeeded (if any)
        std::string message;        ///< Human-readable message
        int64_t offset_ms = 0;      ///< Time offset applied in milliseconds
    };
    
    /// Callback for sync completion
    using SyncCallback = std::function<void(const SyncResult&)>;
    
    /// Get singleton instance
    static NtpManager& Instance();
    
    // Disable copy/move
    NtpManager(const NtpManager&) = delete;
    NtpManager& operator=(const NtpManager&) = delete;
    
    /**
     * @brief Initialize NTP manager and load config
     * @return Result indicating success or error
     */
    Result<void> Initialize();
    
    /**
     * @brief Shutdown NTP manager and stop background sync
     */
    void Shutdown();
    
    /**
     * @brief Get current NTP configuration
     * @return Current config
     */
    Config GetConfig() const;
    
    /**
     * @brief Set NTP configuration
     * @param cfg New configuration
     * @return Result indicating success or error
     */
    Result<void> SetConfig(const Config& cfg);
    
    /**
     * @brief Sync time now (one-shot, blocking)
     * @return SyncResult with status and details
     */
    SyncResult SyncNow();
    
    /**
     * @brief Trigger async sync with callback
     * @param callback Optional callback when sync completes
     */
    void TriggerSync(SyncCallback callback = nullptr);
    
    /**
     * @brief Start background periodic sync thread
     * @return Result indicating success or error
     */
    Result<void> StartPeriodicSync();
    
    /**
     * @brief Stop background periodic sync thread
     */
    void StopPeriodicSync();
    
    /**
     * @brief Check if internet is reachable
     * Uses DNS resolution and UDP socket connect (not ICMP ping)
     * @return true if internet connectivity available
     */
    bool CheckInternetConnectivity();
    
    /**
     * @brief Check if NTP sync is currently in progress
     * @return true if sync is running
     */
    bool IsSyncing() const { return syncing_.load(); }
    
    /**
     * @brief Get last sync result
     * @return Last sync result (thread-safe copy)
     */
    SyncResult GetLastSyncResult() const;
    
    /**
     * @brief Set callback for sync events
     * @param callback Function to call on sync completion
     */
    void SetSyncCallback(SyncCallback callback);
    
    /**
     * @brief Get NTP servers from DHCP lease
     * Reads DHCP lease file for NTP server option (option 42)
     * @return Vector of NTP server addresses from DHCP
     */
    std::vector<std::string> GetDhcpNtpServers();

private:
    NtpManager();
    ~NtpManager();
    
    /// Load config from config system
    void LoadConfig();
    
    /// Save config to config system
    bool SaveConfig();
    
    /// NTP packet structure (RFC 5905)
    struct NtpTimestamp {
        uint32_t seconds;
        uint32_t fraction;
    };
    
    struct NtpPacket {
        uint8_t li_vn_mode;      // Leap indicator, version, mode
        uint8_t stratum;         // Stratum level
        uint8_t poll;            // Poll interval
        int8_t precision;        // Precision
        uint32_t root_delay;     // Root delay
        uint32_t root_dispersion;// Root dispersion
        uint32_t ref_id;         // Reference ID
        NtpTimestamp ref_time;   // Reference timestamp
        NtpTimestamp orig_time;  // Origin timestamp
        NtpTimestamp recv_time;  // Receive timestamp
        NtpTimestamp trans_time; // Transmit timestamp
    };
    
    /// Create NTP socket with timeout
    int CreateNtpSocket();
    
    /// Build NTP request packet
    void BuildNtpRequest(NtpPacket& packet);
    
    /// Parse NTP response and calculate offset
    bool ParseNtpResponse(const NtpPacket& packet, int64_t& offset_ms);
    
    /// Apply time offset to system clock
    bool ApplyTimeOffset(int64_t offset_ms);
    
    /// Try to sync with a specific server
    bool TrySyncWithServer(const std::string& server, int64_t& offset_ms);
    
    /// Background sync thread function
    void SyncThread();
    
    // Configuration and state
    Config config_;
    DstConfig dst_config_;
    SyncResult last_result_;
    SyncCallback sync_callback_;
    
    // Thread synchronization
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> syncing_{false};
    std::atomic<bool> running_{false};
    std::atomic<bool> sync_requested_{false};
    std::thread sync_thread_;
    
    /// Default fallback NTP server IPs when DNS is unavailable
    /// Includes: Google, Cloudflare, NIST, Apple NTP servers
    static const std::vector<std::string> FALLBACK_IPS;
    
    // NTP constants
    static constexpr int NTP_PORT = 123;
    static constexpr uint32_t JAN_1970 = 2208988800UL;  // Seconds from 1900 to 1970
};

} // namespace networking
} // namespace ipcam
