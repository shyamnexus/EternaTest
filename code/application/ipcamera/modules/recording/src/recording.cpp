/**
 * @file recording.cpp
 * @brief Video Recording Service Implementation
 * 
 * Implements encrypted video recording to SD card with:
 * - AES-256-CTR encryption for video files
 * - Automatic segment rotation
 * - FIFO storage management
 * - SD card hot-plug support
 */

#include "ipcam/recording.h"
#include "ipcam/mp4_recorder.h"
#include "ipcam/config.h"
#include "ipcam/storage.h"
#include "ipcam/aac_codec.h"
#include "ipcam/hdal_pipeline.h"
#include "ipcam/media_hub.h"
#include "ipcam/audio_frame_broadcaster.h"
#include "ipcam/rtsp_server.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

// HDAL includes (video encoder only — audio now comes from AudioFrameBroadcaster)
extern "C" {
#include "hdal.h"
#include "hd_debug.h"
}

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <random>
#include <memory>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/mount.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

// OpenSSL for encryption
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace ipcam {
namespace recording {

// ============================================================================
// Constants
// ============================================================================
static constexpr const char* RECORDING_DB_KEY = "storage.database.recordings.path";
static constexpr const char* DEFAULT_SDCARD_PATH = "/mnt/sd";
static constexpr const char* KEY_DERIVATION_SALT = "ipcam_recording_2024";
static constexpr int KEY_DERIVATION_ITERATIONS = 50000;
static constexpr size_t AES_KEY_SIZE = 32;  // 256 bits
static constexpr size_t AES_IV_SIZE = 16;   // 128 bits
static constexpr size_t ENCRYPTION_BLOCK_SIZE = 16;
static constexpr size_t WRITE_BUFFER_SIZE = 64 * 1024;  // 64KB buffer

// ============================================================================
// RawH264Writer - Simple raw H.264/H.265 file writer
// ============================================================================
class RawH264Writer {
public:
    RawH264Writer() = default;
    ~RawH264Writer() { Close(); }
    
    bool Open(const std::string& filename) {
        file_ = std::fopen(filename.c_str(), "wb");
        if (!file_) {
            spdlog::error("RawH264Writer: Failed to open file: {}", filename);
            return false;
        }
        filename_ = filename;
        bytes_written_ = 0;
        frame_count_ = 0;
        spdlog::info("RawH264Writer: Opened {}", filename);
        return true;
    }
    
    void Close() {
        if (file_) {
            std::fclose(file_);
            file_ = nullptr;
            spdlog::info("RawH264Writer: Closed {} ({} bytes, {} frames)", 
                         filename_, bytes_written_, frame_count_);
        }
    }
    
    bool IsOpen() const { return file_ != nullptr; }
    
    // Write NAL unit with Annex-B start code
    bool WriteNalUnit(const uint8_t* data, size_t size) {
        if (!file_) return false;
        
        // Data should already have start codes from HDAL encoder
        size_t written = std::fwrite(data, 1, size, file_);
        if (written != size) {
            spdlog::error("RawH264Writer: Write failed ({} of {} bytes)", written, size);
            return false;
        }
        
        bytes_written_ += size;
        frame_count_++;
        return true;
    }
    
    uint64_t GetBytesWritten() const { return bytes_written_; }
    uint64_t GetFrameCount() const { return frame_count_; }
    
private:
    FILE* file_ = nullptr;
    std::string filename_;
    uint64_t bytes_written_ = 0;
    uint64_t frame_count_ = 0;
};

// ============================================================================
// Utility Functions Implementation
// ============================================================================

std::string RecordingTypeToString(RecordingType type) {
    switch (type) {
        case RecordingType::Continuous: return "continuous";
        case RecordingType::Motion: return "motion";
        case RecordingType::Event: return "event";
        case RecordingType::Manual: return "manual";
        case RecordingType::Schedule: return "schedule";
        default: return "unknown";
    }
}

RecordingType StringToRecordingType(const std::string& str) {
    if (str == "continuous") return RecordingType::Continuous;
    if (str == "motion") return RecordingType::Motion;
    if (str == "event") return RecordingType::Event;
    if (str == "manual") return RecordingType::Manual;
    if (str == "schedule") return RecordingType::Schedule;
    return RecordingType::Manual;
}

std::string RecordingStatusToString(RecordingStatus status) {
    switch (status) {
        case RecordingStatus::Idle: return "idle";
        case RecordingStatus::Recording: return "recording";
        case RecordingStatus::Paused: return "paused";
        case RecordingStatus::Error: return "error";
        default: return "unknown";
    }
}

std::string StorageStatusToString(StorageStatus status) {
    switch (status) {
        case StorageStatus::NotMounted: return "not_mounted";
        case StorageStatus::Mounted: return "mounted";
        case StorageStatus::Full: return "full";
        case StorageStatus::ReadOnly: return "read_only";
        case StorageStatus::Error: return "error";
        default: return "unknown";
    }
}

std::string FileFormatToString(FileFormat format) {
    switch (format) {
        case FileFormat::MP4: return "mp4";
        case FileFormat::TS: return "ts";
        case FileFormat::Raw: return "h264";
        default: return "mp4";
    }
}

FileFormat StringToFileFormat(const std::string& str) {
    if (str == "mp4") return FileFormat::MP4;
    if (str == "ts") return FileFormat::TS;
    if (str == "h264" || str == "raw" || str == "h265") return FileFormat::Raw;
    return FileFormat::MP4;
}

std::string GetFileExtension(FileFormat format) {
    switch (format) {
        case FileFormat::MP4: return ".mp4";
        case FileFormat::TS: return ".ts";
        case FileFormat::Raw: return ".h264";
        default: return ".mp4";
    }
}

std::string FormatBytes(int64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(unit_index > 0 ? 2 : 0) << size << " " << units[unit_index];
    return oss.str();
}

std::string FormatDuration(int64_t milliseconds) {
    int64_t seconds = milliseconds / 1000;
    int64_t minutes = seconds / 60;
    int64_t hours = minutes / 60;
    
    std::ostringstream oss;
    if (hours > 0) {
        oss << hours << "h ";
        minutes %= 60;
    }
    if (minutes > 0 || hours > 0) {
        oss << minutes << "m ";
        seconds %= 60;
    }
    oss << seconds << "s";
    return oss.str();
}

// ============================================================================
// Helper Functions
// ============================================================================

/// Check if path is on SD card mount point
static bool IsPathOnSdCard(const std::string& path) {
    return path.find("/mnt/sd") == 0;
}

/// Get device unique ID for key derivation
static std::string GetDeviceId() {
    std::ifstream mac_file("/sys/class/net/eth0/address");
    if (mac_file.is_open()) {
        std::string mac;
        std::getline(mac_file, mac);
        if (!mac.empty()) return mac;
    }
    return "default-device-id";
}

/// Generate UUID-like recording ID
static std::string GenerateUUID() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t part1 = dis(gen);
    uint64_t part2 = dis(gen);
    
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << (part1 >> 32)
        << "-" << std::setw(4) << ((part1 >> 16) & 0xFFFF)
        << "-" << std::setw(4) << (part1 & 0xFFFF)
        << "-" << std::setw(4) << (part2 >> 48)
        << "-" << std::setw(12) << (part2 & 0xFFFFFFFFFFFF);
    return oss.str();
}

/// Get current timestamp in milliseconds
static int64_t GetCurrentTimeMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

/// Format timestamp as filename-safe string
static std::string FormatTimestamp(int64_t timestamp_ms) {
    time_t seconds = timestamp_ms / 1000;
    struct tm tm_info;
    localtime_r(&seconds, &tm_info);
    
    std::ostringstream oss;
    oss << std::put_time(&tm_info, "%Y%m%d_%H%M%S");
    return oss.str();
}

/// Check if SD card is mounted read-write by inspecting /proc/mounts options.
/// This is a zero-cost check — it reads a procfs pseudo-file and does NOT
/// touch the SD card filesystem at all. On vfat, creating/deleting test files
/// modifies the FAT allocation table, which can trigger "clusters badly computed"
/// on a dirty filesystem and cause the kernel to set it permanently read-only.
static bool IsSdCardMountedRw() {
    std::ifstream mounts("/proc/mounts");
    if (!mounts.is_open()) return false;
    
    std::string line;
    while (std::getline(mounts, line)) {
        // Find the /mnt/sd mount from an mmcblk device
        if (line.find("/mnt/sd") != std::string::npos &&
            line.find("/dev/mmcblk") != std::string::npos) {
            // Options field is the 4th space-separated field
            // e.g.: /dev/mmcblk0p1 /mnt/sd vfat rw,dirsync,... 0 0
            std::istringstream iss(line);
            std::string dev, mnt, fstype, opts;
            if (iss >> dev >> mnt >> fstype >> opts) {
                // Mount options start with "rw" or "ro"
                return opts.substr(0, 2) == "rw";
            }
        }
    }
    return false;
}

/// Attempt to remount SD card read-write after kernel set it read-only.
/// Returns true if the remount succeeded.
static bool TryRemountSdRw() {
    if (IsSdCardMountedRw()) return true;
    
    spdlog::warn("SD card is read-only, attempting remount rw...");
    
    // MS_REMOUNT to remount without changing other options
    int ret = mount(nullptr, "/mnt/sd", nullptr, MS_REMOUNT, nullptr);
    if (ret == 0 && IsSdCardMountedRw()) {
        spdlog::info("SD card successfully remounted read-write");
        return true;
    }
    
    spdlog::warn("Failed to remount SD card rw (errno={}) - filesystem may need fsck.fat", errno);
    return false;
}

// ============================================================================
// RecordingService Implementation
// ============================================================================

RecordingService& RecordingService::Instance() {
    static RecordingService instance;
    return instance;
}

bool RecordingService::Initialize() {
    if (initialized_) {
        spdlog::warn("RecordingService already initialized");
        return true;
    }
    
    spdlog::info("Initializing Recording Service...");
    
    // Load configuration
    if (!LoadConfig()) {
        spdlog::warn("Failed to load recording config, using defaults");
    }
    
    // Check storage availability
    storage_available_ = CheckStorageAvailable();
    
    if (!storage_available_) {
        spdlog::warn("SD card not available - recording disabled but camera operational");
        // Still mark as initialized for graceful degradation
        initialized_ = true;
        return true;
    }
    
    // Create directory structure
    if (!CreateDirectoryStructure()) {
        spdlog::error("Failed to create recording directory structure");
        // Continue anyway - might work later
    }
    
    // Start monitor thread
    shutdown_requested_ = false;
    monitor_thread_ = std::make_unique<std::thread>(&RecordingService::MonitorThread, this);
    
    initialized_ = true;
    spdlog::info("Recording Service initialized (storage: {})", 
                 storage_available_ ? "available" : "unavailable");
    return true;
}

void RecordingService::Shutdown() {
    if (!initialized_) return;
    
    spdlog::info("Shutting down Recording Service...");
    
    shutdown_requested_ = true;
    
    // Stop all active recordings
    for (int ch = 0; ch < 3; ch++) {
        if (channels_[ch].status == RecordingStatus::Recording) {
            StopRecording(ch);
        }
    }
    
    // Stop monitor thread
    if (monitor_thread_ && monitor_thread_->joinable()) {
        monitor_thread_->join();
    }
    monitor_thread_.reset();
    
    initialized_ = false;
    spdlog::info("Recording Service shutdown complete");
}

bool RecordingService::CheckStorageAvailable() {
    if (!IsSdCardMounted()) {
        spdlog::debug("SD card not mounted");
        return false;
    }
    
    // Check if mounted read-write by inspecting /proc/mounts.
    // IMPORTANT: Do NOT create/delete test files on the SD card!
    // On vfat, touch+rm modifies the FAT allocation table which can trigger
    // "clusters badly computed" on a dirty filesystem and cause the kernel
    // to set the entire filesystem permanently read-only.
    if (!IsSdCardMountedRw()) {
        // Try to recover from kernel-imposed read-only
        if (!TryRemountSdRw()) {
            spdlog::warn("SD card mounted but not writable");
            return false;
        }
    }
    
    // Also verify via statvfs (catches hardware write-protect tab)
    struct statvfs vfs;
    if (statvfs(config_.sdcard_path.c_str(), &vfs) == 0) {
        if (vfs.f_flag & ST_RDONLY) {
            spdlog::warn("SD card statvfs reports read-only");
            return false;
        }
    }
    
    return true;
}

bool RecordingService::TryReconnectStorage() {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    bool was_available = storage_available_;
    storage_available_ = CheckStorageAvailable();
    
    if (storage_available_ && !was_available) {
        spdlog::info("SD card storage reconnected");
        
        // Create directory structure
        CreateDirectoryStructure();
        
        // Notify via callback
        if (storage_callback_) {
            std::lock_guard<std::mutex> cb_lock(callback_mutex_);
            storage_callback_(StorageStatus::Mounted, "SD card storage available");
        }
    }
    
    return storage_available_;
}

StorageStats RecordingService::GetStorageStats() const {
    StorageStats stats{};
    
    if (!storage_available_) {
        stats.status = StorageStatus::NotMounted;
        return stats;
    }
    
    struct statvfs vfs;
    if (statvfs(config_.sdcard_path.c_str(), &vfs) != 0) {
        stats.status = StorageStatus::Error;
        return stats;
    }
    
    stats.total_bytes = static_cast<int64_t>(vfs.f_blocks) * vfs.f_frsize;
    stats.free_bytes = static_cast<int64_t>(vfs.f_bavail) * vfs.f_frsize;
    stats.used_bytes = stats.total_bytes - stats.free_bytes;
    
    // Check for read-only
    if (vfs.f_flag & ST_RDONLY) {
        stats.status = StorageStatus::ReadOnly;
    } else if (stats.free_bytes < config_.free_space_min_mb * 1024 * 1024) {
        stats.status = StorageStatus::Full;
    } else {
        stats.status = StorageStatus::Mounted;
    }
    
    // Calculate recordings size
    std::string recordings_path = config_.sdcard_path + "/" + config_.recording_base_dir;
    if (fs::exists(recordings_path)) {
        for (const auto& entry : fs::recursive_directory_iterator(recordings_path)) {
            if (entry.is_regular_file()) {
                stats.recordings_bytes += entry.file_size();
                stats.total_files++;
            }
        }
    }
    
    // Get filesystem type
    std::ifstream mounts("/proc/mounts");
    std::string line;
    while (std::getline(mounts, line)) {
        if (line.find("/mnt/sd") != std::string::npos) {
            std::istringstream iss(line);
            std::string dev, mount, type;
            iss >> dev >> mount >> type;
            stats.filesystem_type = type;
            break;
        }
    }
    
    return stats;
}

bool RecordingService::NeedsFormatting() const {
    if (!IsSdCardMounted()) {
        return false; // Can't check if not mounted
    }
    
    auto stats = GetStorageStats();
    return stats.status == StorageStatus::Error || 
           stats.filesystem_type.empty();
}

bool RecordingService::FormatStorage(const std::string& filesystem) {
    // WARNING: This erases all data!
    spdlog::warn("Formatting SD card as {} - ALL DATA WILL BE ERASED", filesystem);
    
    // Stop all recordings first
    for (int ch = 0; ch < 3; ch++) {
        StopRecording(ch);
    }
    
    // Unmount SD card
    int rc = system("umount /mnt/sd 2>/dev/null");
    (void)rc;
    
    // Format (assuming /dev/mmcblk0p1 - adjust as needed)
    std::string cmd;
    if (filesystem == "exfat") {
        cmd = "mkfs.exfat -n IPCAM /dev/mmcblk0p1";
    } else {
        cmd = "mkfs.vfat -n IPCAM /dev/mmcblk0p1";
    }
    
    rc = system(cmd.c_str());
    if (rc != 0) {
        spdlog::error("Format failed with code {}", rc);
        return false;
    }
    
    // Remount
    rc = system("mount -t auto /dev/mmcblk0p1 /mnt/sd");
    if (rc != 0) {
        spdlog::error("Failed to remount SD card");
        return false;
    }
    
    // Recreate directory structure
    storage_available_ = CheckStorageAvailable();
    if (storage_available_) {
        CreateDirectoryStructure();
    }
    
    spdlog::info("SD card formatted successfully as {}", filesystem);
    return true;
}

// ============================================================================
// SD Card Management Implementation
// ============================================================================

SdCardInfo RecordingService::GetSdCardInfo() const {
    SdCardInfo info{};
    
    // Check if card is physically inserted by looking for block device
    info.device_path = "/dev/mmcblk0p1";
    struct stat st;
    info.inserted = (stat(info.device_path.c_str(), &st) == 0);
    
    // If no partition, try the whole device
    if (!info.inserted) {
        info.device_path = "/dev/mmcblk0";
        info.inserted = (stat(info.device_path.c_str(), &st) == 0);
    }
    
    if (!info.inserted) {
        info.error_message = "No SD card detected";
        info.needs_format = false;
        info.recording_ready = false;
        return info;
    }
    
    info.mount_point = config_.sdcard_path;
    
    // Check if mounted by reading /proc/mounts
    info.mounted = false;
    std::ifstream mounts("/proc/mounts");
    std::string line;
    while (std::getline(mounts, line)) {
        if (line.find("/mnt/sd") != std::string::npos) {
            info.mounted = true;
            
            // Parse filesystem type
            std::istringstream iss(line);
            std::string dev, mount, type;
            iss >> dev >> mount >> type;
            info.filesystem_type = type;
            
            // Normalize filesystem names
            if (type == "vfat") info.filesystem_type = "FAT32";
            else if (type == "exfat") info.filesystem_type = "exFAT";
            else if (type == "ext4") info.filesystem_type = "ext4";
            
            break;
        }
    }
    
    if (!info.mounted) {
        // Card inserted but not mounted - might need formatting
        info.needs_format = true;
        info.error_message = "SD card inserted but not mounted - may need formatting";
        info.recording_ready = false;
        info.recommended_filesystem = "exfat";
        return info;
    }
    
    info.formatted = true;  // If mounted, it has a valid filesystem
    
    // Get size info
    struct statvfs vfs;
    if (statvfs(info.mount_point.c_str(), &vfs) == 0) {
        info.total_bytes = static_cast<int64_t>(vfs.f_blocks) * vfs.f_frsize;
        info.free_bytes = static_cast<int64_t>(vfs.f_bavail) * vfs.f_frsize;
        info.used_bytes = info.total_bytes - info.free_bytes;
        info.writable = !(vfs.f_flag & ST_RDONLY);
    }
    
    // Get volume label (try reading from filesystem)
    // Note: This is filesystem-specific, for now just use a placeholder
    info.label = "IPCAM";
    
    // Determine if card needs formatting
    // FAT32 has 4GB file size limit which is problematic for long recordings
    // Cards > 32GB should use exFAT
    info.needs_format = false;
    info.recommended_filesystem = "exfat";
    
    if (info.filesystem_type == "FAT32" && info.total_bytes > 32LL * 1024 * 1024 * 1024) {
        info.needs_format = true;  // Large card with FAT32 - recommend reformatting
        info.error_message = "FAT32 on large card - recommend formatting as exFAT";
    }
    
    // Check if writable using /proc/mounts rw flag (zero filesystem I/O).
    // Do NOT create/delete test files — on vfat this modifies the FAT table
    // and can trigger cluster errors on a dirty filesystem.
    info.writable = IsSdCardMountedRw();
    if (!info.writable) {
        info.error_message = "SD card is read-only (write-protected or filesystem error)";
        info.recording_ready = false;
        return info;
    }
    
    // All checks passed
    info.recording_ready = info.inserted && info.mounted && info.formatted && info.writable;
    
    return info;
}

bool RecordingService::IsSdCardInserted() const {
    // Check if MMC block device exists (nvt_mmc driver creates this when card is inserted)
    struct stat st;
    // Check for partition first (most SD cards have a partition table)
    if (stat("/dev/mmcblk0p1", &st) == 0 && S_ISBLK(st.st_mode)) {
        return true;
    }
    // Fallback to whole device (for unpartitioned cards)
    if (stat("/dev/mmcblk0", &st) == 0 && S_ISBLK(st.st_mode)) {
        return true;
    }
    return false;
}

bool RecordingService::IsSdCardMounted() const {
    // Must verify that /mnt/sd is mounted from an SD card device (mmcblk*)
    // NOT just any mount point or the rootfs directory
    std::ifstream mounts("/proc/mounts");
    if (!mounts.is_open()) return false;
    
    std::string line;
    while (std::getline(mounts, line)) {
        // Look for lines mounting to /mnt/sd from mmcblk device
        // Format: device mount_point filesystem options dump pass
        // e.g.: /dev/mmcblk0p1 /mnt/sd vfat rw,dirsync,... 0 0
        if (line.find("/mnt/sd") != std::string::npos) {
            // Verify the device is an MMC block device (not tmpfs or anything else)
            if (line.find("/dev/mmcblk") != std::string::npos) {
                return true;
            }
            // If /mnt/sd is mounted but NOT from mmcblk, it's not a valid SD mount
            spdlog::debug("Found /mnt/sd mount but not from mmcblk device: {}", line.substr(0, 50));
        }
    }
    return false;
}

std::string RecordingService::GetSdCardDevice() const {
    // Return the block device path for the SD card
    struct stat st;
    if (stat("/dev/mmcblk0p1", &st) == 0 && S_ISBLK(st.st_mode)) {
        return "/dev/mmcblk0p1";
    }
    if (stat("/dev/mmcblk0", &st) == 0 && S_ISBLK(st.st_mode)) {
        return "/dev/mmcblk0";
    }
    return "";
}

std::string RecordingService::GetSdCardFilesystem() const {
    // Parse /proc/mounts to find the filesystem type for /mnt/sd
    std::ifstream mounts("/proc/mounts");
    if (!mounts.is_open()) return "";
    
    std::string line;
    while (std::getline(mounts, line)) {
        if (line.find("/mnt/sd") != std::string::npos && 
            line.find("/dev/mmcblk") != std::string::npos) {
            // Parse: device mount_point filesystem options ...
            std::istringstream iss(line);
            std::string device, mount_point, filesystem;
            if (iss >> device >> mount_point >> filesystem) {
                return filesystem;
            }
        }
    }
    return "";
}

bool RecordingService::IsSdCardRecordingReady() const {
    auto info = GetSdCardInfo();
    return info.recording_ready;
}

bool RecordingService::MountSdCard() {
    if (IsSdCardMounted()) {
        spdlog::debug("SD card already mounted");
        return true;
    }
    
    if (!IsSdCardInserted()) {
        spdlog::error("Cannot mount - no SD card inserted");
        return false;
    }
    
    // Create mount point if it doesn't exist
    std::string mount_point = config_.sdcard_path;
    if (!fs::exists(mount_point)) {
        fs::create_directories(mount_point);
    }
    
    // Try to mount with auto-detection
    std::string device = "/dev/mmcblk0p1";
    struct stat st;
    if (stat(device.c_str(), &st) != 0) {
        device = "/dev/mmcblk0";
    }
    
    std::string cmd = "mount -t auto " + device + " " + mount_point + " 2>&1";
    int rc = system(cmd.c_str());
    
    if (rc != 0) {
        spdlog::error("Failed to mount SD card (exit code {})", rc);
        return false;
    }
    
    spdlog::info("SD card mounted at {}", mount_point);
    
    // Update storage availability
    storage_available_ = CheckStorageAvailable();
    if (storage_available_) {
        CreateDirectoryStructure();
    }
    
    return true;
}

bool RecordingService::UnmountSdCard() {
    // Stop all recordings first
    spdlog::info("Stopping recordings before unmounting...");
    for (int ch = 0; ch < 3; ch++) {
        if (channels_[ch].status == RecordingStatus::Recording) {
            StopRecording(ch);
        }
    }
    
    // Sync filesystem
    sync();
    
    // Unmount
    std::string cmd = "umount " + config_.sdcard_path + " 2>&1";
    int rc = system(cmd.c_str());
    
    if (rc != 0) {
        spdlog::error("Failed to unmount SD card (exit code {})", rc);
        return false;
    }
    
    storage_available_ = false;
    spdlog::info("SD card unmounted");
    
    return true;
}

bool RecordingService::FormatSdCard(const std::string& filesystem, const std::string& label) {
    spdlog::warn("Formatting SD card as {} with label '{}' - ALL DATA WILL BE ERASED", 
                 filesystem, label);
    
    // Stop all recordings first
    for (int ch = 0; ch < 3; ch++) {
        if (channels_[ch].status == RecordingStatus::Recording) {
            StopRecording(ch);
        }
    }
    
    // Unmount if mounted
    if (IsSdCardMounted()) {
        sync();
        std::string umount_cmd = "umount " + config_.sdcard_path + " 2>/dev/null";
        system(umount_cmd.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Determine device path
    std::string device = "/dev/mmcblk0p1";
    struct stat st;
    if (stat(device.c_str(), &st) != 0) {
        device = "/dev/mmcblk0";
        if (stat(device.c_str(), &st) != 0) {
            spdlog::error("No SD card device found");
            return false;
        }
    }
    
    // Build format command based on filesystem type
    std::string cmd;
    if (filesystem == "exfat") {
        // exFAT - good for large cards, no 4GB file limit
        cmd = "mkfs.exfat -n \"" + label + "\" " + device + " 2>&1";
    } else if (filesystem == "fat32") {
        // FAT32 - maximum compatibility but 4GB file size limit
        cmd = "mkfs.vfat -F 32 -n \"" + label + "\" " + device + " 2>&1";
    } else if (filesystem == "ext4") {
        // ext4 - Linux native, best performance
        cmd = "mkfs.ext4 -L \"" + label + "\" " + device + " 2>&1";
    } else {
        spdlog::error("Unsupported filesystem: {}", filesystem);
        return false;
    }
    
    spdlog::info("Running format command: {}", cmd);
    int rc = system(cmd.c_str());
    
    if (rc != 0) {
        spdlog::error("Format failed with exit code {}", rc);
        return false;
    }
    
    spdlog::info("Format completed successfully");
    
    // Remount
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (!MountSdCard()) {
        spdlog::warn("Format succeeded but remount failed");
        return true;  // Format succeeded even if mount didn't
    }
    
    spdlog::info("SD card formatted and remounted successfully");
    return true;
}

bool RecordingService::EjectSdCard() {
    spdlog::info("Ejecting SD card safely...");
    
    // Stop all recordings
    for (int ch = 0; ch < 3; ch++) {
        if (channels_[ch].status == RecordingStatus::Recording) {
            StopRecording(ch);
        }
    }
    
    // Sync all pending writes
    sync();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Unmount
    if (!UnmountSdCard()) {
        spdlog::error("Failed to unmount SD card for eject");
        return false;
    }
    
    spdlog::info("SD card ejected safely - you may now remove the card");
    
    // Notify via callback
    if (storage_callback_) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        storage_callback_(StorageStatus::NotMounted, "SD card ejected");
    }
    
    return true;
}

bool RecordingService::LoadConfig() {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    try {
        config_.sdcard_path = config::Get<std::string>("storage.sdcard_path", DEFAULT_SDCARD_PATH);
        config_.recording_base_dir = config::Get<std::string>("storage.recording_base_dir", "DCIM");
        config_.encryption_enabled = config::Get<bool>("storage.recording_encryption_enabled", true);
        config_.fifo_enabled = config::Get<bool>("storage.FIFO", true);
        config_.free_space_min_mb = config::Get<int>("storage.free_size_del_min", 500);
        config_.cleanup_threshold_percent = config::Get<int>("storage.file_management.cleanup_threshold_percent", 90);
        
        // Load channel configs
        config_.channels.clear();
        for (int i = 0; i < 3; i++) {
            std::string prefix = "storage.channels[" + std::to_string(i) + "]";
            ChannelConfig ch;
            ch.id = config::Get<int>(prefix + ".id", i);
            ch.enabled = config::Get<bool>(prefix + ".enabled", false);  // All channels disabled by default
            ch.folder_name = config::Get<std::string>(prefix + ".folder_name", "CH0" + std::to_string(i + 1));
            ch.format = StringToFileFormat(config::Get<std::string>(prefix + ".file_format", "mp4"));
            ch.file_duration_sec = config::Get<int>(prefix + ".file_duration", 60);
            ch.max_files = config::Get<int>(prefix + ".file_max_num", 300);
            ch.quota_mb = config::Get<int>(prefix + ".video_quota", 30) * 1024;  // GB to MB
            
            // Map channel to stream
            if (i == 0) ch.stream_id = "video1";
            else if (i == 1) ch.stream_id = "video2";
            else ch.stream_id = "video3";
            
            config_.channels.push_back(ch);
        }
        
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error loading recording config: {}", e.what());
        return false;
    }
}

bool RecordingService::SaveConfig() {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    try {
        config::Set<std::string>("storage.sdcard_path", config_.sdcard_path);
        config::Set<std::string>("storage.recording_base_dir", config_.recording_base_dir);
        config::Set<bool>("storage.recording_encryption_enabled", config_.encryption_enabled);
        config::Set<bool>("storage.FIFO", config_.fifo_enabled);
        config::Set<int>("storage.free_size_del_min", static_cast<int>(config_.free_space_min_mb));
        
        for (size_t i = 0; i < config_.channels.size(); i++) {
            std::string prefix = "storage.channels[" + std::to_string(i) + "]";
            const auto& ch = config_.channels[i];
            config::Set<int>(prefix + ".id", ch.id);
            config::Set<bool>(prefix + ".enabled", ch.enabled);
            config::Set<std::string>(prefix + ".folder_name", ch.folder_name);
            config::Set<std::string>(prefix + ".file_format", FileFormatToString(ch.format));
            config::Set<int>(prefix + ".file_duration", ch.file_duration_sec);
            config::Set<int>(prefix + ".file_max_num", ch.max_files);
            config::Set<int>(prefix + ".video_quota", static_cast<int>(ch.quota_mb / 1024));
        }
        
        // Persist to disk
        config::Save();
        
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error saving recording config: {}", e.what());
        return false;
    }
}

bool RecordingService::CreateDirectoryStructure() {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    try {
        // Create DCIM base directory
        std::string base_path = config_.sdcard_path + "/" + config_.recording_base_dir;
        fs::create_directories(base_path);
        
        // Create channel directories
        for (const auto& ch : config_.channels) {
            std::string ch_path = base_path + "/" + ch.folder_name;
            fs::create_directories(ch_path);
            spdlog::debug("Created recording directory: {}", ch_path);
        }
        
        // Create hidden database directory
        std::string db_path = config_.sdcard_path + "/.ipcamera";
        fs::create_directories(db_path);
        
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create directory structure: {}", e.what());
        return false;
    }
}

RecordingConfig RecordingService::GetConfig() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_;
}

bool RecordingService::SetConfig(const RecordingConfig& config) {
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        config_ = config;
    }
    return SaveConfig();
}

ChannelConfig RecordingService::GetChannelConfig(int channel) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (channel >= 0 && channel < static_cast<int>(config_.channels.size())) {
        return config_.channels[channel];
    }
    return ChannelConfig{};
}

bool RecordingService::SetChannelConfig(int channel, const ChannelConfig& config) {
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        if (channel < 0 || channel >= static_cast<int>(config_.channels.size())) {
            return false;
        }
        config_.channels[channel] = config;
    }
    return SaveConfig();
}

std::string RecordingService::GenerateFilename(int channel, int64_t timestamp) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    const auto& ch_config = config_.channels[channel];
    std::string base_path = config_.sdcard_path + "/" + config_.recording_base_dir + "/" + ch_config.folder_name;
    
    std::string filename = FormatTimestamp(timestamp) + GetFileExtension(ch_config.format);
    
    // If encryption is enabled, add .enc extension
    if (config_.encryption_enabled) {
        filename += ".enc";
    }
    
    return base_path + "/" + filename;
}

std::string RecordingService::GenerateRecordingId() {
    return GenerateUUID();
}

bool RecordingService::DeriveEncryptionKey(const std::string& recording_id, uint8_t* key, uint8_t* iv) {
    // Derive encryption key from device ID + recording ID
    std::string device_id = GetDeviceId();
    std::string input = device_id + "_" + recording_id + "_" + KEY_DERIVATION_SALT;
    
    // Use PBKDF2 to derive key material
    uint8_t derived[48];  // 32 bytes key + 16 bytes IV
    
    int result = PKCS5_PBKDF2_HMAC(
        input.c_str(), static_cast<int>(input.length()),
        reinterpret_cast<const unsigned char*>(KEY_DERIVATION_SALT),
        static_cast<int>(strlen(KEY_DERIVATION_SALT)),
        KEY_DERIVATION_ITERATIONS,
        EVP_sha256(),
        48, derived
    );
    
    if (result != 1) {
        spdlog::error("Key derivation failed");
        return false;
    }
    
    memcpy(key, derived, AES_KEY_SIZE);
    memcpy(iv, derived + AES_KEY_SIZE, AES_IV_SIZE);
    
    return true;
}

bool RecordingService::StartRecording(int channel, RecordingType type) {
    if (!initialized_) {
        spdlog::error("Recording service not initialized");
        return false;
    }
    
    if (!storage_available_) {
        spdlog::error("Cannot start recording - storage not available");
        return false;
    }
    
    if (channel < 0 || channel >= 3) {
        spdlog::error("Invalid channel: {}", channel);
        return false;
    }
    
    auto& ch_state = channels_[channel];
    
    if (ch_state.status == RecordingStatus::Recording) {
        spdlog::warn("Channel {} already recording", channel);
        return true;
    }
    
    // Generate filename first
    ch_state.start_time = GetCurrentTimeMs();
    ch_state.segment_start = ch_state.start_time;
    ch_state.current_file = GenerateFilename(channel, ch_state.start_time);
    
    // Use the timestamp-based filename stem as the recording ID
    // This ensures the ID used for encryption can be derived from the filename
    ch_state.current_id = FormatTimestamp(ch_state.start_time);
    
    // Derive encryption key from the filename-based ID
    if (config_.encryption_enabled) {
        if (!DeriveEncryptionKey(ch_state.current_id, ch_state.aes_key, ch_state.aes_iv)) {
            spdlog::error("Failed to derive encryption key for channel {}", channel);
            return false;
        }
    }
    
    ch_state.type = type;
    ch_state.bytes_written = 0;
    ch_state.segments_completed = 0;
    ch_state.error.clear();
    ch_state.stop_requested = false;
    
    // Start recording thread
    ch_state.thread = std::make_unique<std::thread>(&RecordingService::RecordingThread, this, channel);
    
    spdlog::info("Started recording on channel {} (type: {}, file: {})",
                 channel, RecordingTypeToString(type), ch_state.current_file);
    
    // Notify via callback
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        if (status_callback_) {
            status_callback_(channel, RecordingStatus::Recording);
        }
    }
    
    return true;
}

bool RecordingService::StopRecording(int channel) {
    if (channel < 0 || channel >= 3) {
        return false;
    }
    
    auto& ch_state = channels_[channel];
    
    if (ch_state.status != RecordingStatus::Recording) {
        spdlog::debug("Channel {} not recording", channel);
        return true;
    }
    
    // Signal thread to stop
    ch_state.stop_requested = true;
    
    // Wait for thread to finish
    if (ch_state.thread && ch_state.thread->joinable()) {
        ch_state.thread->join();
    }
    ch_state.thread.reset();
    
    ch_state.status = RecordingStatus::Idle;
    
    spdlog::info("Stopped recording on channel {}", channel);
    
    // Notify via callback
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        if (status_callback_) {
            status_callback_(channel, RecordingStatus::Idle);
        }
    }
    
    return true;
}

bool RecordingService::PauseRecording(int channel) {
    if (channel < 0 || channel >= 3) return false;
    
    auto& ch_state = channels_[channel];
    if (ch_state.status == RecordingStatus::Recording) {
        ch_state.status = RecordingStatus::Paused;
        spdlog::info("Paused recording on channel {}", channel);
        
        if (status_callback_) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            status_callback_(channel, RecordingStatus::Paused);
        }
        return true;
    }
    return false;
}

bool RecordingService::ResumeRecording(int channel) {
    if (channel < 0 || channel >= 3) return false;
    
    auto& ch_state = channels_[channel];
    if (ch_state.status == RecordingStatus::Paused) {
        ch_state.status = RecordingStatus::Recording;
        spdlog::info("Resumed recording on channel {}", channel);
        
        if (status_callback_) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            status_callback_(channel, RecordingStatus::Recording);
        }
        return true;
    }
    return false;
}

RecordingSession RecordingService::GetSession(int channel) const {
    RecordingSession session{};
    session.channel = channel;
    
    if (channel < 0 || channel >= 3) {
        session.status = RecordingStatus::Error;
        session.error_message = "Invalid channel";
        return session;
    }
    
    const auto& ch_state = channels_[channel];
    session.status = ch_state.status;
    session.type = ch_state.type;
    session.current_file = ch_state.current_file;
    session.start_time = ch_state.start_time;
    session.segment_start_time = ch_state.segment_start;
    session.bytes_written = ch_state.bytes_written;
    session.segments_completed = ch_state.segments_completed;
    session.error_message = ch_state.error;
    
    return session;
}

std::vector<RecordingSession> RecordingService::GetActiveSessions() const {
    std::vector<RecordingSession> sessions;
    
    for (int ch = 0; ch < 3; ch++) {
        if (channels_[ch].status == RecordingStatus::Recording ||
            channels_[ch].status == RecordingStatus::Paused) {
            sessions.push_back(GetSession(ch));
        }
    }
    
    return sessions;
}

bool RecordingService::CutNow(int channel) {
    if (channel < 0 || channel >= 3) {
        spdlog::error("CutNow: invalid channel {}", channel);
        return false;
    }
    auto& ch_state = channels_[channel];
    if (ch_state.status != RecordingStatus::Recording) {
        spdlog::debug("CutNow: channel {} not recording", channel);
        return false;
    }
    ch_state.force_segment_cut.store(true, std::memory_order_release);
    spdlog::info("CutNow: segment cut requested for channel {}", channel);
    return true;
}

void RecordingService::NotifyEncoderSettingsChanged(int channel) {
    if (channel < 0) {
        // All channels
        for (int ch = 0; ch < 3; ch++) {
            if (channels_[ch].status == RecordingStatus::Recording) {
                channels_[ch].force_segment_cut.store(true, std::memory_order_release);
                spdlog::info("Recording: encoder settings changed — forcing segment cut on channel {}", ch);
            }
        }
    } else if (channel < 3) {
        if (channels_[channel].status == RecordingStatus::Recording) {
            channels_[channel].force_segment_cut.store(true, std::memory_order_release);
            spdlog::info("Recording: encoder settings changed — forcing segment cut on channel {}", channel);
        }
    }
}

// Helper to parse timestamp from filename format YYYYMMDD_HHMMSS
static int64_t ParseFilenameTimestamp(const std::string& stem) {
    // Format: YYYYMMDD_HHMMSS (15 characters)
    if (stem.length() < 15 || stem[8] != '_') {
        return 0;
    }
    
    try {
        int year = std::stoi(stem.substr(0, 4));
        int month = std::stoi(stem.substr(4, 2));
        int day = std::stoi(stem.substr(6, 2));
        int hour = std::stoi(stem.substr(9, 2));
        int min = std::stoi(stem.substr(11, 2));
        int sec = std::stoi(stem.substr(13, 2));
        
        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = min;
        tm.tm_sec = sec;
        tm.tm_isdst = -1;
        
        time_t t = mktime(&tm);
        if (t == -1) {
            // Handle dates before epoch (1970) - use simple calculation
            // This is approximate but works for display purposes
            int64_t days_since_epoch = (year - 1970) * 365 + (month - 1) * 30 + day;
            int64_t secs = days_since_epoch * 86400 + hour * 3600 + min * 60 + sec;
            return secs * 1000;  // Return milliseconds
        }
        
        return static_cast<int64_t>(t) * 1000;  // Return milliseconds
    } catch (...) {
        return 0;
    }
}

std::vector<RecordingInfo> RecordingService::ListRecordings(
    int channel, int64_t start_time, int64_t end_time,
    std::optional<RecordingType> type, int offset, int limit) const {
    
    std::vector<RecordingInfo> recordings;
    
    if (!storage_available_) {
        return recordings;
    }
    
    // Always do filesystem scan for now (database is secondary index)
    spdlog::debug("Scanning filesystem for recordings");
    
    std::string base_path = config_.sdcard_path + "/" + config_.recording_base_dir;
    
    for (int ch = 0; ch < 3; ch++) {
        if (channel >= 0 && channel != ch) continue;
        
        const auto& ch_config = config_.channels[ch];
        std::string ch_path = base_path + "/" + ch_config.folder_name;
        
        if (!fs::exists(ch_path)) continue;
        
        for (const auto& entry : fs::directory_iterator(ch_path)) {
            if (!entry.is_regular_file()) continue;
            
            std::string filename = entry.path().filename().string();
            // Only include .mp4 and .mp4.enc files
            if (filename.find(".mp4") == std::string::npos) continue;
            
            RecordingInfo info;
            info.filename = entry.path().string();
            info.channel = ch;
            info.file_size = entry.file_size();
            info.encrypted = (filename.find(".enc") != std::string::npos);
            info.status = "complete";
            
            // Parse timestamp from filename
            std::string stem = entry.path().stem().string();
            // For .mp4.enc files, stem is "YYYYMMDD_HHMMSS.mp4", need to remove .mp4
            if (info.encrypted && stem.length() > 4 && stem.substr(stem.length() - 4) == ".mp4") {
                stem = stem.substr(0, stem.length() - 4);
            }
            
            info.id = stem;
            info.start_time = ParseFilenameTimestamp(stem);
            info.end_time = info.start_time + 60000;  // Assume 60 second segments
            
            // Apply time-based filter if specified
            if (start_time > 0 && info.start_time > 0 && info.start_time < start_time) {
                continue;  // Recording started before our filter start
            }
            if (end_time > 0 && info.start_time > 0 && info.start_time > end_time) {
                continue;  // Recording started after our filter end
            }
            
            recordings.push_back(info);
        }
    }
    
    // Sort by start_time descending (newest first)
    std::sort(recordings.begin(), recordings.end(),
        [](const RecordingInfo& a, const RecordingInfo& b) {
            return a.start_time > b.start_time;
        });
    
    // Apply pagination
    if (offset > 0 && offset < static_cast<int>(recordings.size())) {
        recordings.erase(recordings.begin(), recordings.begin() + offset);
    }
    if (limit > 0 && limit < static_cast<int>(recordings.size())) {
        recordings.resize(limit);
    }
    
    spdlog::debug("Found {} recordings", recordings.size());
    return recordings;
}

std::optional<RecordingInfo> RecordingService::GetRecording(const std::string& id) const {
    auto recordings = ListRecordings();
    
    for (const auto& rec : recordings) {
        if (rec.id == id) {
            return rec;
        }
    }
    
    return std::nullopt;
}

bool RecordingService::DeleteRecording(const std::string& id) {
    auto rec = GetRecording(id);
    if (!rec) {
        spdlog::warn("Recording not found: {}", id);
        return false;
    }
    
    try {
        if (fs::exists(rec->filename)) {
            fs::remove(rec->filename);
            spdlog::info("Deleted recording: {}", rec->filename);
        }
        
        // Also delete thumbnail if exists
        if (!rec->thumbnail.empty() && fs::exists(rec->thumbnail)) {
            fs::remove(rec->thumbnail);
        }
        
        // Remove from database
        RemoveFromDatabase(id);
        
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to delete recording {}: {}", id, e.what());
        return false;
    }
}

int RecordingService::DeleteRecordings(const std::vector<std::string>& ids) {
    int deleted = 0;
    for (const auto& id : ids) {
        if (DeleteRecording(id)) {
            deleted++;
        }
    }
    return deleted;
}

int RecordingService::DeleteRecordingsBefore(int64_t before_time) {
    int deleted = 0;
    auto recordings = ListRecordings();
    
    for (const auto& rec : recordings) {
        if (rec.end_time > 0 && rec.end_time < before_time) {
            if (DeleteRecording(rec.id)) {
                deleted++;
            }
        }
    }
    
    return deleted;
}

std::string RecordingService::GetDecryptionKey(const std::string& id) const {
    // Return hex-encoded decryption key for authorized playback
    uint8_t key[32], iv[16];
    if (!const_cast<RecordingService*>(this)->DeriveEncryptionKey(id, key, iv)) {
        return "";
    }
    
    std::ostringstream oss;
    for (int i = 0; i < 32; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key[i]);
    }
    return oss.str();
}

int RecordingService::OpenForPlayback(const std::string& id) {
    auto rec = GetRecording(id);
    if (!rec) {
        return -1;
    }
    
    int fd = open(rec->filename.c_str(), O_RDONLY);
    if (fd < 0) {
        spdlog::error("Failed to open recording for playback: {}", rec->filename);
        return -1;
    }
    
    if (rec->encrypted) {
        // Store decryption state
        std::lock_guard<std::mutex> lock(playback_mutex_);
        PlaybackState state;
        state.fd = fd;
        state.recording_id = id;
        state.offset = 0;
        DeriveEncryptionKey(id, state.aes_key, state.aes_iv);
        playback_sessions_[fd] = state;
    }
    
    return fd;
}

void RecordingService::ClosePlayback(int fd) {
    {
        std::lock_guard<std::mutex> lock(playback_mutex_);
        playback_sessions_.erase(fd);
    }
    close(fd);
}

ssize_t RecordingService::ReadDecrypted(int fd, void* buffer, size_t size, off_t offset) {
    std::lock_guard<std::mutex> lock(playback_mutex_);
    
    auto it = playback_sessions_.find(fd);
    if (it == playback_sessions_.end()) {
        // Not an encrypted file, just read directly
        return pread(fd, buffer, size, offset);
    }
    
    // Read encrypted data
    std::vector<uint8_t> encrypted(size);
    ssize_t bytes_read = pread(fd, encrypted.data(), size, offset);
    if (bytes_read <= 0) {
        return bytes_read;
    }
    
    // Decrypt using AES-256-CTR
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    // Adjust IV for offset (CTR mode counter)
    uint8_t adjusted_iv[AES_IV_SIZE];
    memcpy(adjusted_iv, it->second.aes_iv, AES_IV_SIZE);
    uint64_t block_num = offset / ENCRYPTION_BLOCK_SIZE;
    for (int i = AES_IV_SIZE - 1; i >= 8 && block_num > 0; i--) {
        uint64_t val = adjusted_iv[i] + (block_num & 0xFF);
        adjusted_iv[i] = val & 0xFF;
        block_num = (block_num >> 8) + (val >> 8);
    }
    
    int len = 0, total_len = 0;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), nullptr, it->second.aes_key, adjusted_iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    if (EVP_DecryptUpdate(ctx, static_cast<uint8_t*>(buffer), &len, encrypted.data(), bytes_read) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    total_len = len;
    
    if (EVP_DecryptFinal_ex(ctx, static_cast<uint8_t*>(buffer) + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    total_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    return total_len;
}

void RecordingService::SetSegmentCallback(SegmentCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    segment_callback_ = callback;
}

void RecordingService::SetStatusCallback(StatusCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    status_callback_ = callback;
}

void RecordingService::SetStorageCallback(StorageCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    storage_callback_ = callback;
}

int RecordingService::RunCleanup() {
    if (!storage_available_ || !config_.fifo_enabled) {
        return 0;
    }
    
    auto stats = GetStorageStats();
    int usage_percent = (stats.used_bytes * 100) / stats.total_bytes;
    
    if (usage_percent < config_.cleanup_threshold_percent) {
        return 0;  // No cleanup needed
    }
    
    spdlog::info("Running storage cleanup (usage: {}%)", usage_percent);
    
    // Get all recordings sorted by age (oldest first)
    auto recordings = ListRecordings();
    std::sort(recordings.begin(), recordings.end(), [](const RecordingInfo& a, const RecordingInfo& b) {
        return a.start_time < b.start_time;
    });
    
    int deleted = 0;
    int64_t freed_bytes = 0;
    int64_t target_bytes = (stats.used_bytes - (stats.total_bytes * config_.cleanup_threshold_percent / 100)) 
                          + (config_.free_space_min_mb * 1024 * 1024);
    
    for (const auto& rec : recordings) {
        if (freed_bytes >= target_bytes) break;
        
        int64_t file_size = rec.file_size;
        if (DeleteRecording(rec.id)) {
            deleted++;
            freed_bytes += file_size;
            spdlog::debug("Deleted old recording: {} ({} freed)", rec.filename, FormatBytes(file_size));
        }
    }
    
    spdlog::info("Cleanup complete: deleted {} files, freed {}", deleted, FormatBytes(freed_bytes));
    return deleted;
}

bool RecordingService::UpdateRecordingDatabase(const RecordingInfo& info) {
    auto& db = storage::GetRecordingManager();
    if (!db.IsStorageAvailable()) {
        return false;
    }
    
    // Convert to database format
    std::string date = FormatTimestamp(info.start_time).substr(0, 8);  // YYYYMMDD
    
    return db.AddRecording(
        date,
        info.filename,
        info.start_time,
        info.end_time,
        RecordingTypeToString(info.type),
        info.status
    );
}

bool RecordingService::RemoveFromDatabase(const std::string& id) {
    // TODO: Implement database removal
    // For now, just return true as filesystem delete is primary
    return true;
}

/**
 * Encrypt an MP4 file using AES-256-CTR
 * 
 * @param input_file Path to the unencrypted MP4 file (will be deleted on success)
 * @param output_file Path for the encrypted output file
 * @param key AES-256 key (32 bytes)
 * @param iv Initial vector (16 bytes)
 * @return true on success
 */
static bool EncryptFile(const std::string& input_file, 
                        const std::string& output_file,
                        const uint8_t* key, 
                        const uint8_t* iv) {
    // Open input file
    std::ifstream in(input_file, std::ios::binary);
    if (!in.is_open()) {
        spdlog::error("EncryptFile: Failed to open input file: {}", input_file);
        return false;
    }
    
    // Get file size
    in.seekg(0, std::ios::end);
    size_t file_size = in.tellg();
    in.seekg(0, std::ios::beg);
    
    // Open output file
    std::ofstream out(output_file, std::ios::binary);
    if (!out.is_open()) {
        spdlog::error("EncryptFile: Failed to open output file: {}", output_file);
        return false;
    }
    
    // Initialize encryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        spdlog::error("EncryptFile: Failed to create cipher context");
        return false;
    }
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), nullptr, key, iv) != 1) {
        spdlog::error("EncryptFile: Failed to initialize encryption");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    // Encrypt in chunks
    const size_t chunk_size = 64 * 1024;  // 64KB chunks
    std::vector<uint8_t> in_buf(chunk_size);
    std::vector<uint8_t> out_buf(chunk_size + EVP_MAX_BLOCK_LENGTH);
    
    size_t bytes_processed = 0;
    int out_len = 0;
    bool success = true;
    
    while (in.read(reinterpret_cast<char*>(in_buf.data()), chunk_size) || in.gcount() > 0) {
        size_t bytes_read = in.gcount();
        
        if (EVP_EncryptUpdate(ctx, out_buf.data(), &out_len, in_buf.data(), bytes_read) != 1) {
            spdlog::error("EncryptFile: Encryption failed at offset {}", bytes_processed);
            success = false;
            break;
        }
        
        out.write(reinterpret_cast<char*>(out_buf.data()), out_len);
        bytes_processed += bytes_read;
        
        if (!out.good()) {
            spdlog::error("EncryptFile: Write failed at offset {}", bytes_processed);
            success = false;
            break;
        }
    }
    
    // Finalize encryption
    if (success) {
        if (EVP_EncryptFinal_ex(ctx, out_buf.data(), &out_len) != 1) {
            spdlog::error("EncryptFile: Finalization failed");
            success = false;
        } else if (out_len > 0) {
            out.write(reinterpret_cast<char*>(out_buf.data()), out_len);
        }
    }
    
    EVP_CIPHER_CTX_free(ctx);
    in.close();
    out.close();
    
    if (success) {
        // Delete the unencrypted temp file
        if (std::remove(input_file.c_str()) != 0) {
            spdlog::warn("EncryptFile: Failed to delete temp file: {}", input_file);
        }
        spdlog::info("EncryptFile: Successfully encrypted {} ({} bytes)", output_file, file_size);
    } else {
        // Remove incomplete output file
        std::remove(output_file.c_str());
    }
    
    return success;
}

void RecordingService::RecordingThread(int channel) {
    spdlog::info("Recording thread started for channel {}", channel);
    
    auto& ch_state = channels_[channel];
    ch_state.status = RecordingStatus::Recording;
    
    // Encryption is handled at file level - we write MP4 first, then encrypt the complete file
    // This approach is necessary because MP4 format requires proper structure (moov atoms etc.)
    // that can't be streamed through a cipher.
    // The actual file path uses .mp4.enc extension, but we write to .mp4.tmp first
    std::string temp_file_path = ch_state.current_file;
    bool encryption_enabled = config_.encryption_enabled;
    if (encryption_enabled) {
        // Remove .enc extension for temp file (will be added back after encryption)
        if (temp_file_path.length() > 4 && 
            temp_file_path.substr(temp_file_path.length() - 4) == ".enc") {
            temp_file_path = temp_file_path.substr(0, temp_file_path.length() - 4) + ".tmp";
        }
    }
    
    // Get channel configuration
    ChannelConfig ch_config;
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        ch_config = config_.channels[channel];
    }
    
    int64_t segment_duration_ms = ch_config.file_duration_sec * 1000;
    
    // ========================================================================
    // Video: Create MediaHub consumer (replaces direct HDAL encoder access)
    // ========================================================================
    auto& media_hub = media::MediaHub::Instance();
    if (!media_hub.IsRunning()) {
        spdlog::error("MediaHub is not running for channel {}", channel);
        ch_state.status = RecordingStatus::Error;
        return;
    }
    
    std::string consumer_name = "recording-ch" + std::to_string(channel);
    auto video_consumer = media_hub.CreateConsumer(channel, consumer_name);
    if (!video_consumer) {
        spdlog::error("Failed to create MediaHub consumer for recording channel {}", channel);
        ch_state.status = RecordingStatus::Error;
        return;
    }
    
    spdlog::info("Recording channel {} using MediaHub consumer '{}'", channel, consumer_name);
    
    // Get HDAL pipeline reference (needed for audio capture only)
    auto& pipeline = platform::HdalPipeline::Instance();
    
    // Create MP4 recorder
    MP4RecorderConfig mp4_config;
    // Write to temp file if encryption is enabled, otherwise write directly
    mp4_config.filename = encryption_enabled ? temp_file_path : ch_state.current_file;
    
    // Get video parameters from media config
    std::string stream_key = "media.video" + std::to_string(channel + 1);
    mp4_config.width = config::Get<int>(stream_key + ".width", 1920);
    mp4_config.height = config::Get<int>(stream_key + ".height", 1080);
    mp4_config.fps = config::Get<int>(stream_key + ".fps", 30);
    
    // Get codec from config
    std::string codec_str = config::Get<std::string>(stream_key + ".codec", "h264");
    if (codec_str == "h265" || codec_str == "H265" || codec_str == "hevc") {
        mp4_config.video_codec = VideoCodec::H265;
    } else {
        mp4_config.video_codec = VideoCodec::H264;
    }
    
    // Audio configuration — recording ALWAYS uses AAC for universal MP4 playback.
    // The streaming codec (G.711/G.726) setting is independent and only affects RTSP.
    // Audio is gated by both the global audio enable AND the per-stream flag.
    {
        bool global_audio = config::Get<bool>("media.audio.enabled", false);
        bool stream_audio = config::Get<bool>(stream_key + ".audio_enabled", true);
        mp4_config.enable_audio = global_audio && stream_audio;
    }
    mp4_config.audio_sample_rate = config::Get<int>("media.audio.sample_rate", 8000);
    mp4_config.audio_channels = 1;  // Always mono from broadcaster
    mp4_config.audio_codec = AudioCodec::AAC;
    
    std::string audio_codec_str = "aac";  // Always AAC for recording
    
    spdlog::info("Recording audio: AAC-LC (streaming codec is independent)");
    
    // MP4 options
    mp4_config.fragmented = true;  // Power-safe fragmented MP4
    mp4_config.fragment_duration_ms = 2000;  // 2 seconds per fragment
    
    // Determine if we're using MP4 or raw H264 format
    bool use_raw_format = (ch_config.format == FileFormat::Raw);
    
    spdlog::info("Recording channel {} with {}x{} @ {}fps, codec={}, format={}, audio={}",
                 channel, mp4_config.width, mp4_config.height, mp4_config.fps,
                 (mp4_config.video_codec == VideoCodec::H265 ? "H.265" : "H.264"),
                 (use_raw_format ? "raw" : "mp4"),
                 (use_raw_format ? "disabled" : (mp4_config.enable_audio ? "enabled" : "disabled")));
    
    // Create the appropriate recorder based on format
    MP4Recorder mp4_recorder;
    RawH264Writer raw_writer;
    
    if (use_raw_format) {
        // Raw H.264/H.265 format - no audio, no container
        std::string raw_filename = encryption_enabled ? temp_file_path : ch_state.current_file;
        if (!raw_writer.Open(raw_filename)) {
            spdlog::error("Failed to open raw H264 writer for channel {}", channel);
            ch_state.status = RecordingStatus::Error;
            return;
        }
        // Disable audio for raw format
        mp4_config.enable_audio = false;
    } else {
        // MP4 format with optional audio
        if (!mp4_recorder.Open(mp4_config)) {
            spdlog::error("Failed to open MP4 recorder for channel {}", channel);
            ch_state.status = RecordingStatus::Error;
            return;
        }
    }
    
    spdlog::info("Recording loop started (channel {}, duration {}s per segment)",
                 channel, ch_config.file_duration_sec);
    
    // Audio capture setup — use AudioFrameBroadcaster's PCM ring buffer
    // instead of direct HDAL access. This gives us processed audio
    // (HPF/Notch/NS/AGC already applied) and avoids dual HDAL pulls.
    std::unique_ptr<streaming::AudioFrameConsumer> audio_pcm_consumer;
    platform::AacCodec aac_encoder;
    std::vector<uint8_t> aac_output_buffer;
    
    if (mp4_config.enable_audio) {
        auto& broadcaster = streaming::AudioFrameBroadcaster::Instance();
        
        // Eagerly initialise the broadcaster if RTSP hasn't done so yet.
        // Recording only needs the PCM ring buffer — the streaming codec
        // choice is irrelevant but we read it from config so that a later
        // RTSP connection finds the broadcaster already configured.
        if (!broadcaster.IsInitialized()) {
            // Read the streaming codec from config (same key as RTSP)
            std::string codec_str = config::Get<std::string>("media.audio.codec", "g711u");
            streaming::RtspAudioCodec stream_codec = streaming::RtspAudioCodec::kPcmu;
            if (codec_str == "pcma" || codec_str == "g711a" || codec_str == "alaw" ||
                codec_str == "g711_alaw") {
                stream_codec = streaming::RtspAudioCodec::kPcma;
            } else if (codec_str == "g726-32" || codec_str == "g726_32" || codec_str == "g726") {
                stream_codec = streaming::RtspAudioCodec::kG726_32;
            } else if (codec_str == "aac" || codec_str == "aac-lc") {
                stream_codec = streaming::RtspAudioCodec::kAac;
            }
            // Other variants left as default kPcmu — fine for PCM tap.

            int sr = config::Get<int>("media.audio.sample_rate", 8000);
            int ch = 1;  // broadcaster always resamples to mono
            spdlog::info("Recording: eagerly initialising AudioFrameBroadcaster "
                         "(codec={}, sr={}, ch={})", codec_str, sr, ch);
            if (!broadcaster.Initialize(stream_codec, sr, ch)) {
                spdlog::error("Recording: failed to initialise AudioFrameBroadcaster, "
                              "audio disabled");
                mp4_config.enable_audio = false;
            }
        }
        
        // Start the producer thread if not yet running
        if (mp4_config.enable_audio && broadcaster.IsInitialized() &&
            !broadcaster.IsRunning()) {
            if (!broadcaster.Start()) {
                spdlog::error("Recording: failed to start AudioFrameBroadcaster, "
                              "audio disabled");
                mp4_config.enable_audio = false;
            }
        }
        
        // Register as PCM consumer + create AAC encoder
        if (mp4_config.enable_audio) {
            audio_pcm_consumer = broadcaster.RegisterPcmConsumer();
            if (!audio_pcm_consumer) {
                spdlog::warn("Failed to register PCM consumer, audio disabled for recording");
                mp4_config.enable_audio = false;
            } else {
                // Initialize AAC encoder for recording
                int aac_bitrate = (mp4_config.audio_sample_rate >= 44100) ? 128000 : 64000;
                if (!aac_encoder.Init(mp4_config.audio_sample_rate, 
                                      mp4_config.audio_channels, aac_bitrate)) {
                    spdlog::error("Failed to initialize AAC encoder for recording, audio disabled");
                    audio_pcm_consumer.reset();
                    mp4_config.enable_audio = false;
                } else {
                    spdlog::info("Recording audio: AAC-LC encoder ready ({}Hz, {}ch, {} bps)",
                                 mp4_config.audio_sample_rate, mp4_config.audio_channels, aac_bitrate);
                    aac_output_buffer.reserve(platform::AacCodec::kMaxOutputSize);
                }
            }
        }
    }
    
    // Video frame from MediaHub ring buffer
    media::VideoFrame video_frame;
    static constexpr int kVideoFrameTimeoutMs = 10;
    
    while (!ch_state.stop_requested && !shutdown_requested_) {
        // Check if paused
        if (ch_state.status == RecordingStatus::Paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        // ================================================================
        // Pull encoded video data from MediaHub ring buffer
        // ================================================================
        if (video_consumer->WaitForFrame(video_frame, kVideoFrameTimeoutMs)) {
            
            bool is_keyframe = video_frame.is_keyframe;
            uint64_t timestamp_ms = video_frame.capture_time_ms;
            
            // Write to appropriate recorder based on format
            if (use_raw_format) {
                // Raw format: write entire frame data (already has Annex-B start codes)
                if (!raw_writer.WriteNalUnit(video_frame.data.data(), video_frame.data.size())) {
                    spdlog::error("Failed to write NAL unit to raw H264 file");
                }
            } else {
                // MP4 format: write each NAL pack individually
                if (!video_frame.nal_packs.empty()) {
                    for (const auto& pack : video_frame.nal_packs) {
                        if (pack.offset + pack.size <= video_frame.data.size()) {
                            if (!mp4_recorder.WriteVideoFrame(
                                    video_frame.data.data() + pack.offset,
                                    pack.size, timestamp_ms, pack.is_idr)) {
                                spdlog::error("Failed to write video frame to MP4");
                            }
                        }
                    }
                } else {
                    // Fallback: write entire frame as single NAL
                    if (!mp4_recorder.WriteVideoFrame(
                            video_frame.data.data(), video_frame.data.size(),
                            timestamp_ms, is_keyframe)) {
                        spdlog::error("Failed to write video frame to MP4");
                    }
                }
            }
            
            ch_state.bytes_written += video_frame.data.size();
        }
        
        // Pull processed PCM audio from broadcaster and encode to AAC
        if (mp4_config.enable_audio && audio_pcm_consumer) {
            streaming::AudioFrame pcm_frame;
            // Drain all available PCM frames (non-blocking)
            while (audio_pcm_consumer->GetNextFrame(pcm_frame, 0)) {
                if (pcm_frame.size == 0) continue;
                
                // pcm_frame.data contains s16le PCM at sample_rate_ mono
                const int16_t* pcm_samples = reinterpret_cast<const int16_t*>(pcm_frame.data);
                int num_samples = pcm_frame.size / sizeof(int16_t);
                uint64_t audio_timestamp_ms = GetCurrentTimeMs();
                
                // Encode PCM to AAC
                aac_output_buffer.clear();
                int aac_bytes = aac_encoder.Encode(pcm_samples, num_samples, aac_output_buffer);
                if (aac_bytes > 0) {
                    if (!mp4_recorder.WriteAudioFrame(aac_output_buffer.data(),
                                                      aac_output_buffer.size(),
                                                      audio_timestamp_ms)) {
                        spdlog::debug("Failed to write AAC audio frame to MP4");
                    }
                }
                // aac_bytes == 0 means encoder needs more input — normal for AAC
            }
        }
        
        // Check if segment duration exceeded OR an encoder settings change
        // forced an early cut (to pick up new codec / resolution / etc.)
        bool force_cut = ch_state.force_segment_cut.exchange(false, std::memory_order_acq_rel);
        int64_t now = GetCurrentTimeMs();
        if (force_cut) {
            spdlog::info("Recording channel {}: forced segment cut (encoder settings changed)", channel);
        }
        if (force_cut || (now - ch_state.segment_start >= segment_duration_ms)) {
            // Cut to new segment
            spdlog::info("Segment complete for channel {} ({} bytes)",
                        channel, ch_state.bytes_written);
            
            // Close current file (MP4 or raw)
            if (use_raw_format) {
                raw_writer.Close();
            } else {
                mp4_recorder.Close();
            }
            
            // If encryption is enabled, encrypt the temp file to the final destination
            std::string completed_temp_file = use_raw_format ? 
                (encryption_enabled ? temp_file_path : ch_state.current_file) : 
                mp4_config.filename;  // Current temp file
            if (encryption_enabled) {
                spdlog::debug("Encrypting completed segment: {}", completed_temp_file);
                if (!EncryptFile(completed_temp_file, ch_state.current_file, 
                                 ch_state.aes_key, ch_state.aes_iv)) {
                    spdlog::error("Failed to encrypt segment file for channel {}", channel);
                    // Continue anyway - the temp file still exists as backup
                }
            }
            
            // Create recording info
            RecordingInfo info;
            info.id = ch_state.current_id;
            info.filename = ch_state.current_file;
            info.channel = channel;
            info.type = ch_state.type;
            info.start_time = ch_state.segment_start;
            info.end_time = now;
            info.duration_ms = now - ch_state.segment_start;
            info.file_size = ch_state.bytes_written;
            info.encrypted = config_.encryption_enabled;
            info.status = "complete";
            
            // Update database
            UpdateRecordingDatabase(info);
            
            // Notify via callback
            {
                std::lock_guard<std::mutex> lock(callback_mutex_);
                if (segment_callback_) {
                    segment_callback_(channel, info);
                }
            }
            
            ch_state.segments_completed++;
            
            // Start new segment - generate filename first, then derive ID from it
            ch_state.segment_start = now;
            ch_state.current_file = GenerateFilename(channel, now);
            ch_state.current_id = FormatTimestamp(now);  // Use timestamp as ID for key derivation
            ch_state.bytes_written = 0;
            
            // Derive new encryption key for the new segment
            if (encryption_enabled) {
                if (!DeriveEncryptionKey(ch_state.current_id, ch_state.aes_key, ch_state.aes_iv)) {
                    spdlog::error("Failed to derive encryption key for new segment");
                }
            }
            
            // Generate temp file path for new segment
            temp_file_path = ch_state.current_file;
            if (encryption_enabled) {
                if (temp_file_path.length() > 4 && 
                    temp_file_path.substr(temp_file_path.length() - 4) == ".enc") {
                    temp_file_path = temp_file_path.substr(0, temp_file_path.length() - 4) + ".tmp";
                }
            }
            
            // Open new file (use temp path if encrypted)
            if (use_raw_format) {
                std::string raw_filename = encryption_enabled ? temp_file_path : ch_state.current_file;
                if (!raw_writer.Open(raw_filename)) {
                    spdlog::error("Failed to open new raw H264 segment file");
                    break;
                }
            } else {
                mp4_config.filename = encryption_enabled ? temp_file_path : ch_state.current_file;
                
                // Re-read ALL encoder-related config in case settings changed
                // (codec, resolution, fps, audio enabled)
                std::string codec_str = config::Get<std::string>(stream_key + ".codec", "h264");
                if (codec_str == "h265" || codec_str == "H265" || codec_str == "hevc") {
                    mp4_config.video_codec = VideoCodec::H265;
                } else {
                    mp4_config.video_codec = VideoCodec::H264;
                }
                mp4_config.width = config::Get<int>(stream_key + ".width", 1920);
                mp4_config.height = config::Get<int>(stream_key + ".height", 1080);
                mp4_config.fps = config::Get<int>(stream_key + ".fps", 30);
                
                // Re-read audio enabled (global + per-stream)
                bool new_audio_enabled = config::Get<bool>("media.audio.enabled", false)
                                      && config::Get<bool>(stream_key + ".audio_enabled", true);
                
                // Handle audio state transitions between segments
                if (new_audio_enabled && !mp4_config.enable_audio) {
                    // Audio was OFF, now ON — set up broadcaster + AAC encoder
                    spdlog::info("Recording channel {}: audio re-enabled at segment boundary", channel);
                    auto& broadcaster = streaming::AudioFrameBroadcaster::Instance();
                    bool audio_ok = true;
                    if (!broadcaster.IsInitialized()) {
                        std::string ac = config::Get<std::string>("media.audio.codec", "g711u");
                        streaming::RtspAudioCodec sc = streaming::RtspAudioCodec::kPcmu;
                        if (ac == "pcma" || ac == "g711a" || ac == "alaw" || ac == "g711_alaw")
                            sc = streaming::RtspAudioCodec::kPcma;
                        else if (ac == "g726-32" || ac == "g726_32" || ac == "g726")
                            sc = streaming::RtspAudioCodec::kG726_32;
                        else if (ac == "aac" || ac == "aac-lc")
                            sc = streaming::RtspAudioCodec::kAac;
                        int sr = config::Get<int>("media.audio.sample_rate", 8000);
                        audio_ok = broadcaster.Initialize(sc, sr, 1);
                    }
                    if (audio_ok && !broadcaster.IsRunning())
                        audio_ok = broadcaster.Start();
                    if (audio_ok) {
                        audio_pcm_consumer = broadcaster.RegisterPcmConsumer();
                        if (audio_pcm_consumer) {
                            mp4_config.audio_sample_rate = config::Get<int>("media.audio.sample_rate", 8000);
                            int aac_br = (mp4_config.audio_sample_rate >= 44100) ? 128000 : 64000;
                            if (!aac_encoder.Init(mp4_config.audio_sample_rate, 1, aac_br)) {
                                spdlog::error("Recording ch{}: failed to init AAC encoder", channel);
                                audio_pcm_consumer.reset();
                                audio_ok = false;
                            } else {
                                aac_output_buffer.reserve(platform::AacCodec::kMaxOutputSize);
                            }
                        } else {
                            audio_ok = false;
                        }
                    }
                    new_audio_enabled = audio_ok;
                } else if (!new_audio_enabled && mp4_config.enable_audio) {
                    // Audio was ON, now OFF — tear down consumer + encoder
                    spdlog::info("Recording channel {}: audio disabled at segment boundary", channel);
                    audio_pcm_consumer.reset();
                    // aac_encoder will be re-initialized if needed later
                }
                mp4_config.enable_audio = new_audio_enabled;
                
                if (!mp4_recorder.Open(mp4_config)) {
                    spdlog::error("Failed to open new MP4 segment file");
                    break;
                }
            }
            
            // Request IDR frame to get fresh SPS/PPS/VPS for the new segment
            // This ensures the new MP4 file gets the codec initialization data
            if (pipeline.ForceIdr(channel)) {
                spdlog::debug("Requested IDR frame for new segment on channel {}", channel);
            }
            
            // Run cleanup if needed
            RunCleanup();
        }
    }
    
    // Cleanup - close any remaining segment
    if (use_raw_format) {
        raw_writer.Close();
    } else {
        mp4_recorder.Close();
    }
    
    // Encrypt final segment if needed
    if (encryption_enabled && !temp_file_path.empty()) {
        // Check if temp file exists (might not if no frames were written)
        std::ifstream check_file(temp_file_path);
        if (check_file.good()) {
            check_file.close();
            spdlog::debug("Encrypting final segment: {}", temp_file_path);
            if (!EncryptFile(temp_file_path, ch_state.current_file, 
                             ch_state.aes_key, ch_state.aes_iv)) {
                spdlog::error("Failed to encrypt final segment file for channel {}", channel);
            }
        }
    }
    
    // Release MediaHub video consumer
    video_consumer.reset();
    
    // Release recording audio resources
    if (audio_pcm_consumer) {
        // Flush any remaining buffered PCM as a final AAC frame
        if (aac_encoder.IsInitialized()) {
            aac_output_buffer.clear();
            int flush_bytes = aac_encoder.Flush(aac_output_buffer);
            if (flush_bytes > 0) {
                uint64_t ts = GetCurrentTimeMs();
                mp4_recorder.WriteAudioFrame(aac_output_buffer.data(),
                                              aac_output_buffer.size(), ts);
            }
        }
        auto& broadcaster = streaming::AudioFrameBroadcaster::Instance();
        broadcaster.UnregisterPcmConsumer(audio_pcm_consumer->GetId());
        audio_pcm_consumer.reset();
    }
    aac_encoder.Shutdown();
    
    spdlog::info("Recording thread stopped for channel {}", channel);
}

void RecordingService::MonitorThread() {
    spdlog::info("Storage monitor thread started");
    
    while (!shutdown_requested_) {
        // Check storage status periodically
        bool was_available = storage_available_;
        storage_available_ = CheckStorageAvailable();
        
        if (was_available && !storage_available_) {
            spdlog::warn("SD card removed - stopping recordings");
            
            // Stop all recordings
            for (int ch = 0; ch < 3; ch++) {
                if (channels_[ch].status == RecordingStatus::Recording) {
                    StopRecording(ch);
                }
            }
            
            if (storage_callback_) {
                std::lock_guard<std::mutex> lock(callback_mutex_);
                storage_callback_(StorageStatus::NotMounted, "SD card removed");
            }
        } else if (!was_available && storage_available_) {
            spdlog::info("SD card inserted");
            CreateDirectoryStructure();
            
            if (storage_callback_) {
                std::lock_guard<std::mutex> lock(callback_mutex_);
                storage_callback_(StorageStatus::Mounted, "SD card inserted");
            }
        }
        
        // Check storage space
        if (storage_available_) {
            auto stats = GetStorageStats();
            if (stats.status == StorageStatus::Full) {
                spdlog::warn("Storage full - running cleanup");
                RunCleanup();
            }
        }
        
        // Sleep for 5 seconds
        for (int i = 0; i < 50 && !shutdown_requested_; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    spdlog::info("Storage monitor thread stopped");
}

} // namespace recording
} // namespace ipcam
