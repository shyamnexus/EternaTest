/**
 * @file recording.h
 * @brief Video Recording Module for IP Camera
 * 
 * This module provides video recording functionality with:
 * - Encrypted video file storage on SD card
 * - Automatic file segmentation (configurable duration)
 * - FIFO-based storage management
 * - SD card hot-plug support
 * - Multi-channel recording support
 * 
 * @note Recordings are stored on SD card (/mnt/sd/DCIM/CHxx/)
 *       If SD card is not available, recording is disabled but
 *       the camera continues to operate normally.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <chrono>
#include <thread>
#include <optional>
#include <cstdint>
#include <array>
#include <unordered_map>

namespace ipcam {
namespace recording {

// ============================================================================
// Recording Types and Enums
// ============================================================================

/**
 * Recording type classification
 */
enum class RecordingType {
    Continuous,     ///< 24/7 continuous recording
    Motion,         ///< Motion-triggered recording
    Event,          ///< External event trigger (alarm, etc.)
    Manual,         ///< User-initiated recording
    Schedule        ///< Scheduled recording
};

/**
 * Recording status
 */
enum class RecordingStatus {
    Idle,           ///< Not recording
    Recording,      ///< Actively recording
    Paused,         ///< Recording paused
    Error           ///< Error state
};

/**
 * SD card status
 */
enum class StorageStatus {
    NotMounted,     ///< SD card not mounted
    Mounted,        ///< SD card mounted and ready
    Full,           ///< SD card is full
    ReadOnly,       ///< SD card is read-only
    Error           ///< SD card error
};

/**
 * File format for recordings
 */
enum class FileFormat {
    MP4,            ///< MP4 container (H.264/H.265 + AAC)
    TS,             ///< MPEG-TS (for streaming compatibility)
    Raw             ///< Raw H.264/H.265 elementary stream
};

// ============================================================================
// Recording Structures
// ============================================================================

/**
 * Information about a single recording file
 */
struct RecordingInfo {
    std::string id;                 ///< Unique recording ID (UUID)
    std::string filename;           ///< Full path to recording file
    std::string thumbnail;          ///< Path to thumbnail image
    int channel;                    ///< Channel ID (0-based)
    RecordingType type;             ///< Recording type
    int64_t start_time;             ///< Start timestamp (Unix epoch ms)
    int64_t end_time;               ///< End timestamp (Unix epoch ms)
    int64_t duration_ms;            ///< Duration in milliseconds
    int64_t file_size;              ///< File size in bytes
    int width;                      ///< Video width
    int height;                     ///< Video height
    int fps;                        ///< Frame rate
    std::string codec;              ///< Video codec (H264/H265)
    bool encrypted;                 ///< Whether file is encrypted
    std::string status;             ///< "complete", "recording", "error"
};

/**
 * Channel-specific recording configuration
 */
struct ChannelConfig {
    int id = 0;                         ///< Channel ID (0, 1, 2)
    bool enabled = false;               ///< Recording enabled for this channel
    std::string folder_name = "CH01";   ///< Folder name under DCIM
    FileFormat format = FileFormat::MP4;///< Recording format
    int file_duration_sec = 60;         ///< Duration per file (seconds)
    int max_files = 300;                ///< Maximum number of files
    int64_t quota_mb = 30 * 1024;       ///< Storage quota in MB (30GB default)
    std::string stream_id = "video1";   ///< Which stream to record
};

/**
 * Global recording configuration
 */
struct RecordingConfig {
    // Storage paths
    std::string sdcard_path = "/mnt/sd";
    std::string recording_base_dir = "DCIM";
    
    // Encryption
    bool encryption_enabled = true;
    std::string encryption_algorithm = "AES-256-CTR";
    
    // Storage management
    bool fifo_enabled = true;           ///< Delete oldest when full
    int64_t free_space_min_mb = 500;    ///< Minimum free space to maintain
    int cleanup_threshold_percent = 90; ///< Start cleanup at this usage %
    
    // Recording behavior
    RecordingType default_type = RecordingType::Continuous;
    bool pre_record_enabled = false;    ///< Pre-record buffer
    int pre_record_seconds = 5;         ///< Pre-record duration
    bool post_record_enabled = false;   ///< Post-record for events
    int post_record_seconds = 5;        ///< Post-record duration
    
    // Channel configurations
    std::vector<ChannelConfig> channels;
};

/**
 * Recording session state
 */
struct RecordingSession {
    int channel;                        ///< Channel ID
    RecordingStatus status;             ///< Current status
    RecordingType type;                 ///< Recording type
    std::string current_file;           ///< Current recording file path
    int64_t start_time;                 ///< Session start time (Unix ms)
    int64_t segment_start_time;         ///< Current segment start time
    int64_t bytes_written;              ///< Bytes written in current session
    int segments_completed;             ///< Number of completed segments
    std::string error_message;          ///< Error message if any
};

/**
 * Storage statistics
 */
struct StorageStats {
    StorageStatus status;
    int64_t total_bytes;
    int64_t used_bytes;
    int64_t free_bytes;
    int64_t recordings_bytes;           ///< Space used by recordings
    int total_files;
    int oldest_file_age_hours;          ///< Age of oldest recording
    std::string filesystem_type;        ///< FAT32, exFAT, ext4, etc.
};

/**
 * SD Card information
 */
struct SdCardInfo {
    bool inserted;                      ///< Physical card detected
    bool mounted;                       ///< Card is mounted
    bool writable;                      ///< Card is writable (not read-only)
    bool formatted;                     ///< Card has valid filesystem
    bool recording_ready;               ///< Card is ready for recording
    std::string device_path;            ///< e.g., /dev/mmcblk0p1
    std::string mount_point;            ///< e.g., /mnt/sd
    std::string filesystem_type;        ///< FAT32, exFAT, ext4, unknown
    std::string label;                  ///< Volume label
    int64_t total_bytes;
    int64_t free_bytes;
    int64_t used_bytes;
    std::string error_message;          ///< Error if any
    
    // Recommendations
    bool needs_format;                  ///< True if card needs formatting
    std::string recommended_filesystem; ///< "exfat" for >32GB, "fat32" for <=32GB
};

// ============================================================================
// Callbacks
// ============================================================================

/// Called when a segment is completed
using SegmentCallback = std::function<void(int channel, const RecordingInfo& info)>;

/// Called on recording status change
using StatusCallback = std::function<void(int channel, RecordingStatus status)>;

/// Called on storage events (full, low space, etc.)
using StorageCallback = std::function<void(StorageStatus status, const std::string& message)>;

// ============================================================================
// RecordingService Class
// ============================================================================

/**
 * Main recording service class
 * Manages video recording with encryption to SD card
 */
class RecordingService {
public:
    /**
     * Get singleton instance
     */
    static RecordingService& Instance();

    // Lifecycle
    /**
     * Initialize the recording service
     * @return true on success, false if SD card unavailable (graceful degradation)
     */
    bool Initialize();
    
    /**
     * Shutdown the recording service
     * Stops all active recordings and flushes buffers
     */
    void Shutdown();
    
    /**
     * Check if service is initialized
     */
    bool IsInitialized() const { return initialized_; }

    // ========================================================================
    // SD Card Management
    // ========================================================================
    
    /**
     * Get comprehensive SD card information
     * @return SdCardInfo struct with all card details
     */
    SdCardInfo GetSdCardInfo() const;
    
    /**
     * Check if SD card is inserted (physical detection)
     */
    bool IsSdCardInserted() const;
    
    /**
     * Check if SD card is mounted (from mmcblk device)
     */
    bool IsSdCardMounted() const;
    
    /**
     * Get the SD card block device path
     * @return Device path (e.g., "/dev/mmcblk0p1") or empty if not found
     */
    std::string GetSdCardDevice() const;
    
    /**
     * Get the SD card filesystem type
     * @return Filesystem type (e.g., "vfat", "exfat") or empty if not mounted
     */
    std::string GetSdCardFilesystem() const;
    
    /**
     * Check if SD card is ready for recording
     * (inserted, mounted, formatted, writable)
     */
    bool IsSdCardRecordingReady() const;
    
    /**
     * Mount SD card
     * @return true if mounted successfully
     */
    bool MountSdCard();
    
    /**
     * Unmount SD card safely (stops recordings first)
     * @return true if unmounted successfully
     */
    bool UnmountSdCard();
    
    /**
     * Format SD card with specified filesystem
     * @param filesystem "exfat" (recommended for >32GB) or "fat32" (for <=32GB)
     * @param label Volume label (default: "IPCAM")
     * @return true on success
     */
    bool FormatSdCard(const std::string& filesystem, const std::string& label = "IPCAM");
    
    /**
     * Safely eject SD card (sync, unmount)
     * @return true if ejected successfully
     */
    bool EjectSdCard();

    // Storage management (legacy - use SD card APIs above)
    /**
     * Check if SD card storage is available
     */
    bool IsStorageAvailable() const { return storage_available_; }
    
    /**
     * Try to reconnect to SD card (call after hot-plug)
     * @return true if storage is now available
     */
    bool TryReconnectStorage();
    
    /**
     * Get storage statistics
     */
    StorageStats GetStorageStats() const;
    
    /**
     * Format SD card (WARNING: erases all data)
     * @param filesystem "fat32" or "exfat"
     * @return true on success
     */
    bool FormatStorage(const std::string& filesystem);
    
    /**
     * Check if SD card needs formatting
     */
    bool NeedsFormatting() const;

    // Configuration
    /**
     * Get current configuration
     */
    RecordingConfig GetConfig() const;
    
    /**
     * Update configuration
     * @param config New configuration
     * @return true on success
     */
    bool SetConfig(const RecordingConfig& config);
    
    /**
     * Get channel configuration
     */
    ChannelConfig GetChannelConfig(int channel) const;
    
    /**
     * Update channel configuration
     */
    bool SetChannelConfig(int channel, const ChannelConfig& config);

    // Recording control
    /**
     * Start recording on channel
     * @param channel Channel ID (0, 1, 2)
     * @param type Recording type
     * @return true if started successfully
     */
    bool StartRecording(int channel, RecordingType type = RecordingType::Manual);
    
    /**
     * Stop recording on channel
     * @param channel Channel ID
     * @return true if stopped successfully
     */
    bool StopRecording(int channel);
    
    /**
     * Pause recording on channel
     */
    bool PauseRecording(int channel);
    
    /**
     * Resume recording on channel
     */
    bool ResumeRecording(int channel);
    
    /**
     * Get recording status for channel
     */
    RecordingSession GetSession(int channel) const;
    
    /**
     * Get all active recording sessions
     */
    std::vector<RecordingSession> GetActiveSessions() const;
    
    /**
     * Force cut current file and start new segment
     */
    bool CutNow(int channel);

    /**
     * @brief Notify recording that encoder settings changed for a channel
     *
     * Forces an immediate segment cut so the new segment picks up the
     * updated codec / resolution / fps / etc. from config.  Call this
     * from the VideoControl settings-change callback.
     *
     * @param channel Channel index (0-based), or -1 for all channels
     */
    void NotifyEncoderSettingsChanged(int channel = -1);

    // Recording management
    /**
     * List recordings with pagination
     * @param channel Filter by channel (-1 for all)
     * @param start_time Filter by start time (Unix ms, 0 for no filter)
     * @param end_time Filter by end time (Unix ms, 0 for no filter)
     * @param type Filter by type (nullopt for all)
     * @param offset Pagination offset
     * @param limit Maximum results (0 for all)
     * @return List of recordings
     */
    std::vector<RecordingInfo> ListRecordings(
        int channel = -1,
        int64_t start_time = 0,
        int64_t end_time = 0,
        std::optional<RecordingType> type = std::nullopt,
        int offset = 0,
        int limit = 100
    ) const;
    
    /**
     * Get recording info by ID
     */
    std::optional<RecordingInfo> GetRecording(const std::string& id) const;
    
    /**
     * Delete a recording
     * @param id Recording ID
     * @return true if deleted successfully
     */
    bool DeleteRecording(const std::string& id);
    
    /**
     * Delete multiple recordings
     * @param ids Recording IDs
     * @return Number of successfully deleted recordings
     */
    int DeleteRecordings(const std::vector<std::string>& ids);
    
    /**
     * Delete recordings older than specified time
     * @param before_time Unix timestamp in milliseconds
     * @return Number of deleted recordings
     */
    int DeleteRecordingsBefore(int64_t before_time);

    // Playback support
    /**
     * Get decryption key for a recording (for authorized playback)
     * @param id Recording ID
     * @return Decryption key or empty if not found/unauthorized
     */
    std::string GetDecryptionKey(const std::string& id) const;
    
    /**
     * Open recording file for streaming (handles decryption)
     * @param id Recording ID
     * @return File descriptor or -1 on error
     */
    int OpenForPlayback(const std::string& id);
    
    /**
     * Close playback file
     */
    void ClosePlayback(int fd);
    
    /**
     * Read decrypted data for playback
     * @param fd File descriptor from OpenForPlayback
     * @param buffer Output buffer
     * @param size Bytes to read
     * @param offset File offset
     * @return Bytes read or -1 on error
     */
    ssize_t ReadDecrypted(int fd, void* buffer, size_t size, off_t offset);

    // Callbacks
    void SetSegmentCallback(SegmentCallback callback);
    void SetStatusCallback(StatusCallback callback);
    void SetStorageCallback(StorageCallback callback);

    // Cleanup
    /**
     * Run storage cleanup (FIFO deletion)
     * @return Number of files deleted
     */
    int RunCleanup();

private:
    RecordingService() = default;
    ~RecordingService() = default;
    RecordingService(const RecordingService&) = delete;
    RecordingService& operator=(const RecordingService&) = delete;

    // Internal methods
    bool LoadConfig();
    bool SaveConfig();
    bool CheckStorageAvailable();
    bool CreateDirectoryStructure();
    std::string GenerateFilename(int channel, int64_t timestamp);
    std::string GenerateRecordingId();
    bool DeriveEncryptionKey(const std::string& recording_id, uint8_t* key, uint8_t* iv);
    void RecordingThread(int channel);
    void MonitorThread();
    bool UpdateRecordingDatabase(const RecordingInfo& info);
    bool RemoveFromDatabase(const std::string& id);

    // State
    std::atomic<bool> initialized_{false};
    std::atomic<bool> storage_available_{false};
    std::atomic<bool> shutdown_requested_{false};
    
    RecordingConfig config_;
    mutable std::mutex config_mutex_;
    
    // Per-channel recording state
    struct ChannelState {
        std::atomic<RecordingStatus> status{RecordingStatus::Idle};
        RecordingType type{RecordingType::Manual};
        std::string current_file;
        std::string current_id;
        int64_t start_time{0};
        int64_t segment_start{0};
        int64_t bytes_written{0};
        int segments_completed{0};
        std::string error;
        std::unique_ptr<std::thread> thread;
        std::atomic<bool> stop_requested{false};
        std::atomic<bool> force_segment_cut{false};  ///< Set by NotifyEncoderSettingsChanged
        int file_fd{-1};
        
        // Encryption state
        uint8_t aes_key[32];
        uint8_t aes_iv[16];
    };
    std::array<ChannelState, 3> channels_;
    mutable std::mutex channels_mutex_;
    
    // Monitor thread
    std::unique_ptr<std::thread> monitor_thread_;
    
    // Callbacks
    SegmentCallback segment_callback_;
    StatusCallback status_callback_;
    StorageCallback storage_callback_;
    std::mutex callback_mutex_;
    
    // Playback state (active file descriptors for decryption)
    struct PlaybackState {
        int fd;
        std::string recording_id;
        uint8_t aes_key[32];
        uint8_t aes_iv[16];
        off_t offset;
    };
    std::unordered_map<int, PlaybackState> playback_sessions_;
    mutable std::mutex playback_mutex_;
};

// ============================================================================
// Utility Functions
// ============================================================================

/// Convert RecordingType to string
std::string RecordingTypeToString(RecordingType type);

/// Convert string to RecordingType
RecordingType StringToRecordingType(const std::string& str);

/// Convert RecordingStatus to string
std::string RecordingStatusToString(RecordingStatus status);

/// Convert StorageStatus to string
std::string StorageStatusToString(StorageStatus status);

/// Convert FileFormat to string
std::string FileFormatToString(FileFormat format);

/// Convert string to FileFormat
FileFormat StringToFileFormat(const std::string& str);

/// Get file extension for format
std::string GetFileExtension(FileFormat format);

/// Format bytes as human-readable string (e.g., "1.5 GB")
std::string FormatBytes(int64_t bytes);

/// Format duration as human-readable string (e.g., "1h 30m 45s")
std::string FormatDuration(int64_t milliseconds);

} // namespace recording
} // namespace ipcam
