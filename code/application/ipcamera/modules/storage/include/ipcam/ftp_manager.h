/**
 * @file ftp_manager.h
 * @brief FTP Upload Manager for IP Camera
 * 
 * Manages FTP uploads for:
 * - Recording file uploads
 * - Snapshot uploads
 * - Event-triggered uploads
 * - Scheduled uploads
 */

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <queue>
#include <thread>
#include <condition_variable>
#include <vector>

namespace ipcam {
namespace storage {

// ============================================================================
// FTP Types and Enums
// ============================================================================

/**
 * FTP upload schedule options
 */
enum class FtpUploadSchedule {
    Immediate,      ///< Upload immediately
    Hourly,         ///< Upload every hour
    Daily,          ///< Upload once a day
    OnEvent         ///< Upload on event only
};

inline std::string FtpUploadScheduleToString(FtpUploadSchedule schedule) {
    switch (schedule) {
        case FtpUploadSchedule::Immediate: return "immediate";
        case FtpUploadSchedule::Hourly: return "hourly";
        case FtpUploadSchedule::Daily: return "daily";
        case FtpUploadSchedule::OnEvent: return "on_event";
        default: return "immediate";
    }
}

inline FtpUploadSchedule StringToFtpUploadSchedule(const std::string& str) {
    if (str == "hourly") return FtpUploadSchedule::Hourly;
    if (str == "daily") return FtpUploadSchedule::Daily;
    if (str == "on_event") return FtpUploadSchedule::OnEvent;
    return FtpUploadSchedule::Immediate;
}

/**
 * FTP connection status
 */
enum class FtpStatus {
    Disconnected,   ///< Not connected
    Connected,      ///< Connection tested OK
    Uploading,      ///< Currently uploading
    Error           ///< Connection error
};

inline std::string FtpStatusToString(FtpStatus status) {
    switch (status) {
        case FtpStatus::Disconnected: return "disconnected";
        case FtpStatus::Connected: return "connected";
        case FtpStatus::Uploading: return "uploading";
        case FtpStatus::Error: return "error";
        default: return "disconnected";
    }
}

/**
 * FTP upload item status
 */
enum class UploadStatus {
    Pending,        ///< Waiting to be uploaded
    InProgress,     ///< Currently uploading
    Completed,      ///< Upload successful
    Failed          ///< Upload failed
};

// ============================================================================
// FTP Configuration Structure
// ============================================================================

/**
 * FTP server configuration
 */
struct FtpConfig {
    bool enabled = false;                   ///< Whether FTP upload is enabled
    std::string server;                     ///< FTP server hostname or IP
    int port = 21;                          ///< FTP port
    std::string username;                   ///< FTP username
    std::string password;                   ///< FTP password
    std::string remote_path = "/uploads";   ///< Remote directory path
    bool use_ssl = false;                   ///< Use FTPS
    bool passive_mode = true;               ///< Use passive mode
    FtpUploadSchedule upload_schedule = FtpUploadSchedule::Immediate;
    int upload_hour = 2;                    ///< Hour for daily uploads (0-23)
    bool upload_recordings = true;          ///< Upload recording files
    bool upload_snapshots = false;          ///< Upload snapshots
    bool delete_after_upload = false;       ///< Delete local file after upload
    int max_retries = 3;                    ///< Max upload retries
    int timeout_seconds = 30;               ///< Connection timeout
    FtpStatus status = FtpStatus::Disconnected;
    std::string last_error;                 ///< Last error message
};

/**
 * Upload queue item
 */
struct FtpUploadItem {
    std::string id;                         ///< Unique upload ID
    std::string local_path;                 ///< Local file path
    std::string remote_path;                ///< Remote file path
    UploadStatus status = UploadStatus::Pending;
    int retry_count = 0;                    ///< Number of retries
    int64_t file_size = 0;                  ///< File size in bytes
    int64_t bytes_uploaded = 0;             ///< Bytes uploaded so far
    std::string error_message;              ///< Error message if failed
    std::string created_at;                 ///< ISO timestamp
    std::string completed_at;               ///< ISO timestamp when completed
};

/**
 * Upload statistics
 */
struct FtpStats {
    int64_t total_uploads = 0;
    int64_t successful_uploads = 0;
    int64_t failed_uploads = 0;
    int64_t bytes_uploaded = 0;
    int pending_uploads = 0;
    std::string last_upload_time;
};

// ============================================================================
// FTP Manager Class
// ============================================================================

/**
 * Upload progress callback
 * @param item The upload item
 * @param bytes_uploaded Bytes uploaded
 * @param total_bytes Total file size
 */
using UploadProgressCallback = std::function<void(const FtpUploadItem& item, 
                                                   int64_t bytes_uploaded, 
                                                   int64_t total_bytes)>;

/**
 * Upload completion callback
 * @param item The completed upload item
 * @param success Whether upload succeeded
 */
using UploadCompleteCallback = std::function<void(const FtpUploadItem& item, 
                                                   bool success)>;

/**
 * Manages FTP uploads
 */
class FtpManager {
public:
    /**
     * Get singleton instance
     */
    static FtpManager& Instance();
    
    /**
     * Initialize FTP manager
     * Loads configuration from recording.ftp config section
     * @return true on success
     */
    bool Init();
    
    /**
     * Shutdown and cleanup
     */
    void Shutdown();
    
    /**
     * Check if initialized
     */
    bool IsInitialized() const { return initialized_.load(); }
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * Get current FTP configuration
     */
    FtpConfig GetConfig() const;
    
    /**
     * Update FTP configuration
     * @return true on success
     */
    bool SetConfig(const FtpConfig& config);
    
    /**
     * Get current connection status
     */
    FtpStatus GetStatus() const;
    
    /**
     * Get upload statistics
     */
    FtpStats GetStats() const;
    
    // ========================================================================
    // Connection Operations
    // ========================================================================
    
    /**
     * Test connection to FTP server
     * @param config Config to test (or current if nullptr)
     * @return true if connection successful
     */
    bool TestConnection(const FtpConfig* config = nullptr);
    
    /**
     * Get last error message
     */
    std::string GetLastError() const;
    
    // ========================================================================
    // Upload Operations
    // ========================================================================
    
    /**
     * Queue a file for upload
     * @param local_path Path to local file
     * @param remote_filename Optional remote filename (uses local name if empty)
     * @return Upload item ID
     */
    std::string QueueUpload(const std::string& local_path, 
                            const std::string& remote_filename = "");
    
    /**
     * Cancel a pending upload
     * @param upload_id Upload item ID
     * @return true if cancelled
     */
    bool CancelUpload(const std::string& upload_id);
    
    /**
     * Trigger immediate upload of all pending items
     */
    void TriggerUpload();
    
    /**
     * Get all pending uploads
     */
    std::vector<FtpUploadItem> GetPendingUploads() const;
    
    /**
     * Get recent upload history
     * @param max_items Maximum items to return
     */
    std::vector<FtpUploadItem> GetUploadHistory(int max_items = 50) const;
    
    // ========================================================================
    // Callbacks
    // ========================================================================
    
    /**
     * Set upload progress callback
     */
    void SetProgressCallback(UploadProgressCallback callback);
    
    /**
     * Set upload completion callback
     */
    void SetCompleteCallback(UploadCompleteCallback callback);
    
    // ========================================================================
    // Persistence
    // ========================================================================
    
    /**
     * Save configuration to file
     */
    bool SaveConfig();
    
    /**
     * Load configuration from file
     */
    bool LoadConfig();
    
private:
    FtpManager() = default;
    ~FtpManager() = default;
    FtpManager(const FtpManager&) = delete;
    FtpManager& operator=(const FtpManager&) = delete;
    
    /**
     * Upload worker thread function
     */
    void UploadWorker();
    
    /**
     * Perform actual FTP upload
     * @param item Upload item to process
     * @return true on success
     */
    bool PerformUpload(FtpUploadItem& item);
    
    /**
     * Build FTP URL from config
     */
    std::string BuildFtpUrl(const std::string& remote_path) const;
    
    /**
     * Generate unique upload ID
     */
    std::string GenerateUploadId();
    
    /**
     * Check if it's time for scheduled upload
     */
    bool IsScheduledUploadTime() const;
    
    std::atomic<bool> initialized_{false};
    std::atomic<bool> shutdown_requested_{false};
    mutable std::mutex mutex_;
    mutable std::mutex queue_mutex_;
    std::condition_variable upload_cv_;
    FtpConfig config_;
    FtpStats stats_;
    
    std::queue<FtpUploadItem> upload_queue_;
    std::vector<FtpUploadItem> upload_history_;
    std::unique_ptr<std::thread> worker_thread_;
    
    UploadProgressCallback progress_callback_;
    UploadCompleteCallback complete_callback_;
};

} // namespace storage
} // namespace ipcam
