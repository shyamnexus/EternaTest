/**
 * @file system_utils.h
 * @brief Platform-level system utilities for the IP camera
 *
 * Provides low-level system operations: device info, date/time/NTP,
 * reboot/shutdown, factory reset, diagnostics, log management,
 * maintenance, watchdog, config backup/restore, and alarm thresholds.
 *
 * All functions read/write through the config module and interact with
 * the underlying BusyBox/embedded Linux system via /proc, /sys, and
 * standard POSIX APIs.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <ctime>
#include <functional>

namespace ipcam {
namespace platform {

// ============================================================================
// 1. Device Information
// ============================================================================

struct DeviceInfo {
    std::string device_name;
    std::string model;
    std::string serial_number;
    std::string firmware_version;
    std::string hardware_id;
    std::string manufacturer;
    int         telecontrol_id = 0;
    std::string encoder_version;
    std::string web_version;
    std::string plugin_version;
    int         channels_number = 1;
    int         hard_disks_number = 1;
    int         alarm_inputs_number = 0;
    int         alarm_outputs_number = 0;
    std::string firmware_version_info;
    std::string sensor_type;
};

DeviceInfo  GetDeviceInfo();
bool        SetDeviceName(const std::string& name);
bool        SetTelecontrolId(int id);

// ============================================================================
// 2. Date / Time / NTP
// ============================================================================

struct TimeInfo {
    std::string timezone;           ///< IANA timezone (e.g. "America/New_York") or "UTC"
    std::string local_time;         ///< ISO-8601 local time
    std::string utc_time;           ///< ISO-8601 UTC time
    int64_t     unix_timestamp = 0; ///< Seconds since epoch
    bool        dst_active = false; ///< Whether DST is currently in effect
    int         utc_offset_min = 0; ///< Current UTC offset in minutes (includes DST if active)
    std::string date_format;        ///< Display format: "YYYY-MM-DD", "MM/DD/YYYY", "DD/MM/YYYY"
};

struct NtpConfig {
    bool        enabled = false;
    std::vector<std::string> servers;   ///< NTP server hostnames/IPs
    int         sync_interval_sec = 3600;
    std::string last_sync_time;
    bool        last_sync_ok = false;
};

TimeInfo    GetTimeInfo();
bool        SetTimezone(const std::string& tz);
bool        SetDateTime(const std::string& iso8601);
NtpConfig   GetNtpConfig();
bool        SetNtpConfig(const NtpConfig& cfg);
bool        SyncNtpNow();
std::string GetDateFormat();
bool        SetDateFormat(const std::string& fmt);

// ============================================================================
// 3. Reboot / Shutdown
// ============================================================================

enum class RebootType {
    Warm,           ///< Normal reboot
    Cold,           ///< Power-off
    Scheduled       ///< Reboot at a specific time
};

struct RebootRequest {
    RebootType  type = RebootType::Warm;
    int         delay_seconds = 0;      ///< Delay before action (0 = immediate)
    std::string scheduled_time;         ///< ISO-8601 for scheduled reboot
};

bool        TriggerReboot(const RebootRequest& req);
bool        TriggerShutdown(int delay_seconds = 0);
bool        CancelScheduledReboot();
bool        IsRebootScheduled();
std::string GetScheduledRebootTime();

// ============================================================================
// 4. Factory Reset
// ============================================================================

enum class FactoryResetMode {
    Hard,   ///< Full wipe: all settings, users, recordings
    Soft    ///< Keep network settings and user accounts
};

bool        PerformFactoryReset(FactoryResetMode mode, int delay_seconds = 0);

// ============================================================================
// 5. Diagnostics / Health
// ============================================================================

struct CpuInfo {
    float   usage_percent = 0.0f;
    int     num_cores = 1;
    std::vector<float> per_core_percent;
};

struct MemoryInfo {
    int64_t total_bytes = 0;
    int64_t free_bytes = 0;
    int64_t available_bytes = 0;
    int64_t buffers_bytes = 0;
    int64_t cached_bytes = 0;
    float   usage_percent = 0.0f;
};

struct DiskInfo {
    std::string mount_point;
    std::string device;
    std::string filesystem;
    int64_t     total_bytes = 0;
    int64_t     free_bytes = 0;
    float       usage_percent = 0.0f;
};

struct ProcessInfo {
    int         pid = 0;
    std::string name;
    std::string state;          ///< R, S, D, Z, T ...
    float       cpu_percent = 0.0f;
    int64_t     memory_kb = 0;
};

struct DiagnosticsReport {
    CpuInfo                     cpu;
    MemoryInfo                  memory;
    std::vector<DiskInfo>       disks;
    std::vector<ProcessInfo>    processes;
    std::string                 uptime;         ///< Human-readable
    int64_t                     uptime_seconds = 0;
    float                       temperature = 0.0f;     ///< SoC temperature °C
    std::string                 kernel_version;
};

DiagnosticsReport GetDiagnostics();

// ============================================================================
// 6. Logs
// ============================================================================

/// Log source/category — maps to different files on the device
enum class LogSource {
    Application,    ///< /var/log/ipcamd.log        — spdlog app logs
    Kernel,         ///< /mnt/app/logs/dmesg.log     — kernel ring buffer
    System,         ///< /mnt/app/logs/sysdiag.log   — CPU/mem/disk/temp diagnostics
    Thermal,        ///< /mnt/app/logs/thermal.log   — SoC temperature history
    Access,         ///< /var/log/ipcamd_access.log  — HTTP request access log
    Security,       ///< /var/log/ipcamd_security.log— login/auth/lockout events
    Audit,          ///< /var/log/ipcamd_audit.log   — config change audit trail
    Nginx           ///< /var/log/nginx/error.log    — reverse proxy errors
};

/// Convert LogSource to/from string (for API)
const char*  LogSourceToString(LogSource src);
LogSource    LogSourceFromString(const std::string& s);

/// Descriptor for each log source available on the system
struct LogSourceInfo {
    std::string id;             ///< "application", "kernel", etc.
    std::string display_name;   ///< "Application Log"
    std::string description;
    std::string file_path;      ///< Primary file path
    std::string sd_path;        ///< SD card path (if any)
    int64_t     file_size = 0;  ///< Bytes
    bool        available = false;
};

/// Get list of all available log sources with their status
std::vector<LogSourceInfo> GetLogSources();

struct LogEntry {
    std::string timestamp;
    std::string level;      ///< "debug", "info", "warning", "error", "critical"
    std::string message;
    std::string source;     ///< Component/module name
};

struct LogFilter {
    std::string level;              ///< Minimum level ("debug", "info", etc.)
    std::string start_time;         ///< ISO-8601 (optional)
    std::string end_time;           ///< ISO-8601 (optional)
    std::string search;             ///< Text search in message (optional)
    LogSource   log_source = LogSource::Application;  ///< Which log source
    int         limit = 100;        ///< Max entries to return
    int         offset = 0;         ///< Pagination offset
};

struct LogConfig {
    std::string level;              ///< Global log level
    bool        console_enabled = true;
    std::string console_level;
    bool        file_enabled = true;
    std::string file_path;
    std::string file_level;
    int         max_size_mb = 5;
    int         max_files = 3;
    int         flush_interval_sec = 5;
    std::string format;
};

std::vector<LogEntry> GetLogs(const LogFilter& filter);
std::string           GetLogFilePath();     ///< Path for download
std::string           GetLogFilePath(LogSource src); ///< Path for specific source
bool                  ClearLogs();
bool                  ClearLogs(LogSource src); ///< Clear specific source
LogConfig             GetLogConfig();
bool                  SetLogConfig(const LogConfig& cfg);

/// Create a comprehensive diagnostic bundle (tar.gz) containing all logs,
/// config files, hardware info, network state. Returns path to archive.
std::string CreateDiagnosticBundle();

// ============================================================================
// 7. Maintenance
// ============================================================================

struct MaintenanceConfig {
    bool        auto_restart_enabled = false;
    std::string restart_time;           ///< "HH:MM:SS"
    std::string restart_days;           ///< "daily", "Mon,Tue,..."
};

MaintenanceConfig GetMaintenanceConfig();
bool              SetMaintenanceConfig(const MaintenanceConfig& cfg);

// ============================================================================
// 8. Watchdog
// ============================================================================

struct WatchdogConfig {
    bool    enabled = true;
    int     interval_seconds = 30;
};

WatchdogConfig  GetWatchdogConfig();
bool            SetWatchdogConfig(const WatchdogConfig& cfg);

// ============================================================================
// 10. Config Backup / Restore
// ============================================================================

/// Creates a tar.gz archive of all configs, returns path to archive
std::string CreateConfigBackup();

/// Restores configs from uploaded tar.gz archive; returns empty on success
std::string RestoreConfigBackup(const std::string& archivePath);

// ============================================================================
// 11. Alarm Thresholds
// ============================================================================

struct AlarmThresholds {
    int     disk_space_threshold = 90;      ///< Percent
    float   temperature_threshold = 70.0f;  ///< °C
    int     memory_threshold = 90;          ///< Percent
    int     alarm_outputs_number = 0;
};

AlarmThresholds GetAlarmThresholds();
bool            SetAlarmThresholds(const AlarmThresholds& thr);

} // namespace platform
} // namespace ipcam
