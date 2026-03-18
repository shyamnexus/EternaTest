/**
 * @file system_logger.h
 * @brief API-controllable system data collector
 *
 * Replaces the monolithic system_logger.sh shell script with an
 * in-application component that respects configuration and can be
 * controlled at runtime through the REST API.
 *
 * Collection sources (each independently enable/disable + interval):
 *   - kernel:     New dmesg entries since last poll
 *   - thermal:    SoC temperature readings
 *   - sysdiag:    CPU, memory, load, disk, network snapshots
 *   - app_mirror: Copy of /var/log/ipcamd.log for SD card archival
 *
 * Destinations (each independently enable/disable):
 *   - sd_card:  /mnt/sd/logs/  (date-stamped, 30-day retention, space-managed)
 *   - flash:    /mnt/app/logs/ (rotating, size-capped for limited flash)
 *
 * API endpoints:
 *   GET  /api/v1/system/logger          Status + full config
 *   PUT  /api/v1/system/logger          Update config at runtime
 *   POST /api/v1/system/logger/collect  Force immediate collection
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <cstdint>
#include <functional>

namespace ipcam {
namespace platform {

// ============================================================================
// Collection Sources
// ============================================================================

/// Sources the SystemLogger can periodically collect
enum class CollectionSource {
    Kernel,     ///< dmesg — kernel ring buffer
    Thermal,    ///< SoC temperature from /sys/class/thermal
    SysDiag,    ///< CPU, memory, load, disk, network snapshot
    AppMirror   ///< Copy of ipcamd.log to persistent storage
};

const char* CollectionSourceToString(CollectionSource src);
CollectionSource CollectionSourceFromString(const std::string& s);

// ============================================================================
// Configuration Structs
// ============================================================================

/// Per-source configuration
struct CollectionSourceConfig {
    bool enabled      = true;
    int  interval_sec = 60;
};

/// Per-destination configuration
struct LogDestinationConfig {
    bool        enabled       = true;
    std::string path;
    int64_t     max_total_mb  = 500;   ///< Max total log size (SD only)
    int         max_days      = 30;    ///< Max retention days (SD only)
    int64_t     min_free_mb   = 100;   ///< Min free space to leave (SD)
    int64_t     max_file_kb   = 1024;  ///< Max per-file size (flash only)
    int64_t     max_total_kb  = 3072;  ///< Max total for all logs (flash only)
    int64_t     min_free_kb   = 512;   ///< Min free space to leave (flash)
};

/// Full SystemLogger configuration
struct SystemLoggerConfig {
    bool enabled = true;
    std::map<CollectionSource, CollectionSourceConfig> sources;
    LogDestinationConfig sd_card;
    LogDestinationConfig flash;
};

// ============================================================================
// Status Reporting
// ============================================================================

struct CollectionSourceStatus {
    std::string id;
    bool        enabled          = false;
    int         interval_sec     = 0;
    int64_t     last_collection  = 0;     ///< Unix timestamp of last collection
    int64_t     entries_collected = 0;
};

struct DestinationStatus {
    std::string id;
    std::string path;
    bool        enabled    = false;
    bool        available  = false;
    bool        read_write = false;
    int64_t     free_bytes = 0;
    int64_t     used_bytes = 0;
    int64_t     total_bytes = 0;
};

struct SystemLoggerStatus {
    bool running = false;
    std::vector<CollectionSourceStatus> sources;
    std::vector<DestinationStatus>      destinations;
};

// ============================================================================
// SystemLogger Singleton
// ============================================================================

class SystemLogger {
public:
    static SystemLogger& Instance();

    /// Initialize from config and start worker thread
    bool Init();

    /// Stop worker thread, flush pending writes
    void Shutdown();

    /// Reload configuration from config module (called after API PUT)
    void ReloadConfig();

    // ---- Runtime Control ----

    /// Enable/disable a collection source at runtime
    void SetSourceEnabled(CollectionSource src, bool enabled);
    bool IsSourceEnabled(CollectionSource src) const;

    void SetSourceInterval(CollectionSource src, int seconds);
    int  GetSourceInterval(CollectionSource src) const;

    /// Enable/disable a destination at runtime ("sd_card" or "flash")
    void SetDestinationEnabled(const std::string& dest_id, bool enabled);
    bool IsDestinationEnabled(const std::string& dest_id) const;

    /// Enable/disable the entire logger
    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    // ---- Status ----
    SystemLoggerStatus GetStatus() const;
    SystemLoggerConfig GetConfig() const;

    // ---- Force Collection ----

    /// Force immediate collection of a specific source
    void CollectNow(CollectionSource src);

    /// Force immediate collection of all enabled sources
    void CollectAll();

private:
    SystemLogger() = default;
    ~SystemLogger() = default;
    SystemLogger(const SystemLogger&) = delete;
    SystemLogger& operator=(const SystemLogger&) = delete;

    void LoadConfig();
    void SaveConfig();

    // Worker
    void WorkerThread();

    // Collectors
    void CollectKernel();
    void CollectThermal();
    void CollectSysDiag();
    void MirrorAppLog();

    // Destination management
    bool IsSDMountedRW() const;
    bool TryRemountSDRW();
    bool IsSDAvailable();
    bool IsFlashAvailable();
    void EnsureSDSpace();
    void EnsureFlashSpace();
    void CleanupOldSDLogs();
    void RotateFlashLog(const std::string& path, int64_t max_kb);

    // Safe file I/O
    bool WriteSD(const std::string& file, const std::string& content, bool append = true);
    bool WriteFlash(const std::string& file, const std::string& content, bool append = true);
    std::string GetSDDateFile(const std::string& prefix) const;
    std::string GetFlashFile(const std::string& name) const;

    // Free space helpers
    int64_t GetFreeSpaceKB(const std::string& mount_point) const;
    int64_t GetDirSizeKB(const std::string& dir) const;

    // State
    mutable std::mutex mutex_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    bool initialized_ = false;

    SystemLoggerConfig config_;

    // Per-source runtime state
    struct SourceState {
        int64_t last_collection = 0;
        int64_t entries_collected = 0;
    };
    std::map<CollectionSource, SourceState> source_state_;

    // Destination availability (updated each tick)
    bool sd_available_  = false;
    bool flash_available_ = false;

    // dmesg tracking
    int dmesg_last_lines_ = 0;
};

}  // namespace platform
}  // namespace ipcam
