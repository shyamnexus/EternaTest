/**
 * @file system_utils.cpp
 * @brief Platform-level system utilities implementation
 *
 * Implements device info, date/time/NTP, reboot/shutdown, factory reset,
 * diagnostics, log management, maintenance, watchdog, config backup/restore,
 * and alarm thresholds for BusyBox / embedded Linux systems.
 */

#include "ipcam/system_utils.h"
#include <ipcam/config.h>
#include <ipcam/paths.h>
#include <ipcam/ntp_manager.h>
#include <spdlog/spdlog.h>

#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <thread>
#include <algorithm>
#include <array>
#include <filesystem>
#include <regex>
#include <mutex>
#include <atomic>

#include <unistd.h>
#include <sys/reboot.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <signal.h>

namespace fs = std::filesystem;

namespace ipcam {
namespace platform {

// ============================================================================
// Internal helpers
// ============================================================================

namespace {

/// Run a shell command and capture stdout (max 4 KB)
std::string ExecCommand(const std::string& cmd) {
    std::array<char, 4096> buf;
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return {};
    while (fgets(buf.data(), buf.size(), pipe)) {
        result += buf.data();
    }
    pclose(pipe);
    // Trim trailing newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        result.pop_back();
    return result;
}

/// Parse /proc/meminfo key in kB
int64_t ReadMemInfoKb(const std::string& key) {
    std::ifstream f("/proc/meminfo");
    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind(key, 0) == 0) {
            // e.g. "MemTotal:        1020456 kB"
            std::istringstream ss(line.substr(key.size() + 1));
            int64_t val = 0;
            ss >> val;
            return val;
        }
    }
    return 0;
}

/// Read SoC temperature from thermal zone
float ReadSocTemperature() {
    // Primary: Novatek nvt_thermal returns degrees Celsius directly (not millidegrees)
    {
        std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
        if (f.is_open()) {
            int raw = 0;
            f >> raw;
            // Novatek nvt_thermal: values < 200 are degrees C directly (e.g. 61)
            // Standard Linux thermal: values >= 1000 are millidegrees (e.g. 61000)
            if (raw >= 1000) {
                return raw / 1000.0f;
            }
            return static_cast<float>(raw);
        }
    }

    // Fallback paths
    const char* fallbacks[] = {
        "/sys/devices/virtual/thermal/thermal_zone0/temp",
        "/sys/class/hwmon/hwmon0/temp1_input",
    };
    for (auto p : fallbacks) {
        std::ifstream f(p);
        if (f.is_open()) {
            int millideg = 0;
            f >> millideg;
            return millideg / 1000.0f;
        }
    }
    return 0.0f;
}

/// ISO-8601 from time_t
std::string TimeToISO(std::time_t t, bool utc = false) {
    struct tm tmBuf {};
    if (utc) gmtime_r(&t, &tmBuf);
    else     localtime_r(&t, &tmBuf);

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tmBuf);
    if (utc) {
        strcat(buf, "Z");
    } else {
        // append offset  e.g. +05:30
        char off[8];
        strftime(off, sizeof(off), "%z", &tmBuf);
        // insert colon: +0530 -> +05:30
        std::string o(off);
        if (o.size() >= 5) o.insert(3, ":");
        strcat(buf, o.c_str());
    }
    return buf;
}

/// Uptime in seconds from /proc/uptime
int64_t ReadUptimeSeconds() {
    std::ifstream f("/proc/uptime");
    double up = 0;
    f >> up;
    return static_cast<int64_t>(up);
}

/// Human-readable uptime
std::string FormatUptime(int64_t seconds) {
    int days  = seconds / 86400; seconds %= 86400;
    int hours = seconds / 3600;  seconds %= 3600;
    int mins  = seconds / 60;    seconds %= 60;
    char buf[64];
    snprintf(buf, sizeof(buf), "%dd %02dh %02dm %02ds", days, hours, mins, (int)seconds);
    return buf;
}

// Scheduled-reboot state (process-lifetime)
static std::mutex s_rebootMutex;
static std::atomic<bool> s_rebootScheduled{false};
static std::string s_rebootTime;
static std::thread s_rebootThread;

/// Validate a hostname/IP for safe use in shell commands.
/// Allows only [a-zA-Z0-9._:-] to prevent command injection.
bool IsValidHostname(const std::string& host) {
    if (host.empty() || host.size() > 253) return false;
    for (char c : host) {
        if (!std::isalnum(static_cast<unsigned char>(c)) &&
            c != '.' && c != '-' && c != '_' && c != ':') {
            return false;
        }
    }
    return true;
}

/// Validate a filesystem path for safe use in shell commands.
/// Allows only [a-zA-Z0-9._/-] (no ; & | ` $ etc.)
bool IsValidPath(const std::string& path) {
    if (path.empty()) return false;
    for (char c : path) {
        if (!std::isalnum(static_cast<unsigned char>(c)) &&
            c != '/' && c != '.' && c != '-' && c != '_') {
            return false;
        }
    }
    return true;
}

} // anonymous namespace

// ============================================================================
// 1. Device Information
// ============================================================================

DeviceInfo GetDeviceInfo() {
    DeviceInfo d;
    d.device_name        = config::Get<std::string>("device.info.device_name", "IP Camera");
    d.model              = config::Get<std::string>("device.info.model", "Unknown");
    d.serial_number      = config::Get<std::string>("device.info.serial_number", "N/A");
    d.firmware_version   = config::Get<std::string>("device.info.firmware_version", "0.0.0");
    d.hardware_id        = config::Get<std::string>("device.info.hardware_id", "");
    d.manufacturer       = config::Get<std::string>("device.info.manufacturer", "");
    d.telecontrol_id     = config::Get<int>("device.info.telecontrol_id", 88);
    d.encoder_version    = config::Get<std::string>("device.info.encoder_version", "");
    d.web_version        = config::Get<std::string>("device.info.web_version", "");
    d.plugin_version     = config::Get<std::string>("device.info.plugin_version", "");
    d.channels_number    = config::Get<int>("device.info.channels_number", 1);
    d.hard_disks_number  = config::Get<int>("device.info.hard_disks_number", 1);
    d.alarm_inputs_number  = config::Get<int>("device.info.alarm_inputs_number", 0);
    d.alarm_outputs_number = config::Get<int>("device.info.alarm_outputs_number", 0);
    d.firmware_version_info = config::Get<std::string>("device.info.firmware_version_info", "");
    d.sensor_type        = config::Get<std::string>("device.sensor.type", "");
    return d;
}

bool SetDeviceName(const std::string& name) {
    if (name.empty() || name.size() > 64) return false;
    bool ok = config::Set<std::string>("device.info.device_name", name);
    if (ok) config::Save();
    return ok;
}

bool SetTelecontrolId(int id) {
    if (id < 0 || id > 255) return false;
    bool ok = config::Set<int>("device.info.telecontrol_id", id);
    if (ok) config::Save();
    return ok;
}

// ============================================================================
// 2. Date / Time / NTP
// ============================================================================

TimeInfo GetTimeInfo() {
    TimeInfo ti;
    ti.timezone = config::Get<std::string>("system.time.timezone", "UTC");
    auto now = std::time(nullptr);
    ti.unix_timestamp = static_cast<int64_t>(now);
    ti.local_time = TimeToISO(now, false);
    ti.utc_time   = TimeToISO(now, true);
    ti.date_format = config::Get<std::string>("system.time.date_format", "YYYY-MM-DD");

    // DST and UTC offset from localtime (automatic for IANA timezones)
    struct tm local_tm;
    localtime_r(&now, &local_tm);

    auto& ntp = networking::NtpManager::Instance();
    auto dst_cfg = ntp.GetDstConfig();

    if (dst_cfg.enabled && dst_cfg.mode == "auto") {
        // Auto mode: Linux localtime() already applies DST from IANA tzdata
        ti.dst_active = (local_tm.tm_isdst > 0);
    } else if (dst_cfg.enabled && dst_cfg.mode == "custom") {
        // Custom mode: check if current date falls within user-defined DST window
        // Use week-of-month rules (same as Hikvision/Dahua)
        ti.dst_active = false;  // Default
        int mon = local_tm.tm_mon + 1;  // 1-12
        if (dst_cfg.start_month < dst_cfg.end_month) {
            // Northern hemisphere: DST between start and end months
            if (mon > dst_cfg.start_month && mon < dst_cfg.end_month) {
                ti.dst_active = true;
            } else if (mon == dst_cfg.start_month) {
                // Check if past the start transition
                int dom = local_tm.tm_mday;
                int hour = local_tm.tm_hour;
                // Approximate: start_week'th start_day_of_week of month
                // Week 1 = 1-7, Week 2 = 8-14, etc.
                int earliest_day = (dst_cfg.start_week - 1) * 7 + 1;
                int latest_day = dst_cfg.start_week * 7;
                if (dom > latest_day || (dom >= earliest_day && hour >= dst_cfg.start_hour)) {
                    ti.dst_active = true;
                }
            } else if (mon == dst_cfg.end_month) {
                int dom = local_tm.tm_mday;
                int hour = local_tm.tm_hour;
                int earliest_day = (dst_cfg.end_week - 1) * 7 + 1;
                int latest_day = dst_cfg.end_week * 7;
                if (dom < earliest_day || (dom <= latest_day && hour < dst_cfg.end_hour)) {
                    ti.dst_active = true;
                }
            }
        } else {
            // Southern hemisphere: DST wraps around new year
            if (mon > dst_cfg.start_month || mon < dst_cfg.end_month) {
                ti.dst_active = true;
            }
        }
    } else {
        ti.dst_active = false;
    }

    // Compute UTC offset in minutes
    struct tm utc_tm;
    gmtime_r(&now, &utc_tm);
    time_t local_epoch = mktime(&local_tm);
    time_t utc_epoch = mktime(&utc_tm);
    ti.utc_offset_min = static_cast<int>(difftime(local_epoch, utc_epoch) / 60.0);

    // If custom DST is active, add the custom offset to local time display
    if (dst_cfg.enabled && dst_cfg.mode == "custom" && ti.dst_active) {
        ti.utc_offset_min += dst_cfg.offset_minutes;
    }

    return ti;
}

bool SetTimezone(const std::string& tz) {
    if (tz.empty()) return false;

    // Validate that the timezone file exists in the zoneinfo database
    std::string zonefile = "/usr/share/zoneinfo/" + tz;
    if (tz != "UTC" && !fs::exists(zonefile)) {
        spdlog::warn("Timezone file not found: {} — will fall back to TZ env approach", zonefile);
    }

    // Set TZ environment variable
    setenv("TZ", tz.c_str(), 1);
    tzset();

    // Persist symlink /etc/localtime -> zoneinfo
    if (fs::exists(zonefile)) {
        unlink("/etc/localtime");
        if (symlink(zonefile.c_str(), "/etc/localtime") != 0) {
            spdlog::warn("Failed to symlink /etc/localtime: {}", strerror(errno));
        }
    }

    // Write to /etc/timezone for BusyBox
    {
        std::ofstream f("/etc/timezone");
        if (f) f << tz << "\n";
    }

    config::Set<std::string>("system.time.timezone", tz);
    config::Save();
    spdlog::info("Timezone set to {}", tz);
    return true;
}

bool SetDateTime(const std::string& iso8601) {
    // Parse ISO-8601 e.g. "2026-02-20T15:30:00"
    struct tm tmBuf {};
    if (strptime(iso8601.c_str(), "%Y-%m-%dT%H:%M:%S", &tmBuf) == nullptr) {
        spdlog::error("SetDateTime: invalid ISO-8601 format: {}", iso8601);
        return false;
    }
    time_t newTime = mktime(&tmBuf);
    if (newTime == (time_t)-1) return false;

    struct timespec ts;
    ts.tv_sec = newTime;
    ts.tv_nsec = 0;
    if (clock_settime(CLOCK_REALTIME, &ts) != 0) {
        spdlog::error("SetDateTime: clock_settime failed: {}", strerror(errno));
        return false;
    }

    // Sync to hardware clock (BusyBox hwclock)
    ::system("hwclock -w 2>/dev/null");

    spdlog::info("System time set to {}", iso8601);
    return true;
}

NtpConfig GetNtpConfig() {
    NtpConfig c;
    c.enabled           = config::Get<bool>("system.time.ntp.enabled", false);
    c.sync_interval_sec = config::Get<int>("system.time.ntp.refresh_interval_sec", 3600);
    c.last_sync_time    = config::Get<std::string>("system.time.ntp.last_sync_time", "");
    c.last_sync_ok      = config::Get<bool>("system.time.ntp.last_sync_ok", false);

    // Primary server
    std::string primary = config::Get<std::string>("system.time.ntp.primary_server", "pool.ntp.org");
    if (!primary.empty()) c.servers.push_back(primary);

    // Fallback servers (comma-separated in config)
    std::string raw = config::Get<std::string>("system.time.ntp.fallback_servers", "");
    if (!raw.empty()) {
        std::istringstream ss(raw);
        std::string tok;
        while (std::getline(ss, tok, ',')) {
            tok.erase(0, tok.find_first_not_of(" "));
            tok.erase(tok.find_last_not_of(" ") + 1);
            if (!tok.empty()) c.servers.push_back(tok);
        }
    }
    if (c.servers.empty()) c.servers.push_back("pool.ntp.org");
    return c;
}

bool SetNtpConfig(const NtpConfig& cfg) {
    config::Set<bool>("system.time.ntp.enabled", cfg.enabled);
    config::Set<int>("system.time.ntp.refresh_interval_sec", cfg.sync_interval_sec);

    // First server is primary, rest are fallback
    if (!cfg.servers.empty()) {
        config::Set<std::string>("system.time.ntp.primary_server", cfg.servers[0]);
        std::string fallback;
        for (size_t i = 1; i < cfg.servers.size(); ++i) {
            if (i > 1) fallback += ",";
            fallback += cfg.servers[i];
        }
        config::Set<std::string>("system.time.ntp.fallback_servers", fallback);
    }
    config::Save();
    spdlog::info("NTP config updated (network.ntp keys)");
    return true;
}

bool SyncNtpNow() {
    auto cfg = GetNtpConfig();
    if (cfg.servers.empty()) return false;

    // Validate hostname to prevent command injection
    const std::string& server = cfg.servers[0];
    if (!IsValidHostname(server)) {
        spdlog::error("NTP sync rejected: invalid server hostname '{}'", server);
        return false;
    }

    // Use ntpd one-shot sync
    std::string cmd = "ntpd -n -q -p " + server + " 2>&1";
    std::string out = ExecCommand(cmd);
    bool ok = (out.find("setting clock") != std::string::npos || out.find("offset") != std::string::npos);

    // Fallback: try BusyBox rdate
    if (!ok) {
        cmd = "rdate -s " + server + " 2>/dev/null";
        ok = (::system(cmd.c_str()) == 0);
    }

    auto now = TimeToISO(std::time(nullptr), true);
    config::Set<std::string>("system.time.ntp.last_sync_time", now);
    config::Set<bool>("system.time.ntp.last_sync_ok", ok);
    config::Save();

    spdlog::info("NTP manual sync {}: {}", ok ? "succeeded" : "failed", server);
    return ok;
}

std::string GetDateFormat() {
    return config::Get<std::string>("system.time.date_format", "YYYY-MM-DD");
}

bool SetDateFormat(const std::string& fmt) {
    // Validate format string
    if (fmt != "YYYY-MM-DD" && fmt != "MM/DD/YYYY" && fmt != "DD/MM/YYYY") {
        spdlog::warn("Invalid date format: {} (expected YYYY-MM-DD, MM/DD/YYYY, or DD/MM/YYYY)", fmt);
        return false;
    }
    config::Set<std::string>("system.time.date_format", fmt);
    config::Save();
    spdlog::info("Date format set to {}", fmt);
    return true;
}

// ============================================================================
// 3. Reboot / Shutdown
// ============================================================================

bool TriggerReboot(const RebootRequest& req) {
    spdlog::info("Reboot requested: type={}, delay={}s",
                 static_cast<int>(req.type), req.delay_seconds);

    if (req.type == RebootType::Scheduled) {
        // Schedule for later
        std::lock_guard<std::mutex> lk(s_rebootMutex);
        s_rebootScheduled = true;
        s_rebootTime = req.scheduled_time.empty()
            ? TimeToISO(std::time(nullptr) + req.delay_seconds, true)
            : req.scheduled_time;

        // Spawn a thread to wait and reboot
        if (s_rebootThread.joinable()) s_rebootThread.detach();
        int wait = req.delay_seconds > 0 ? req.delay_seconds : 60;
        s_rebootThread = std::thread([wait]() {
            std::this_thread::sleep_for(std::chrono::seconds(wait));
            if (s_rebootScheduled) {
                spdlog::info("Executing scheduled reboot now");
                sync();
                ::reboot(RB_AUTOBOOT);
            }
        });
        s_rebootThread.detach();
        return true;
    }

    // Immediate or delayed warm/cold reboot
    auto doReboot = [&]() {
        sync();
        if (req.type == RebootType::Cold) {
            ::reboot(RB_POWER_OFF);
        } else {
            ::reboot(RB_AUTOBOOT);
        }
    };

    if (req.delay_seconds > 0) {
        std::thread([delay = req.delay_seconds, doReboot]() {
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            doReboot();
        }).detach();
    } else {
        // Small delay to allow HTTP response to be sent
        std::thread([doReboot]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            doReboot();
        }).detach();
    }
    return true;
}

bool TriggerShutdown(int delay_seconds) {
    spdlog::info("Shutdown requested, delay={}s", delay_seconds);
    std::thread([delay_seconds]() {
        if (delay_seconds > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(delay_seconds));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        sync();
        ::reboot(RB_POWER_OFF);
    }).detach();
    return true;
}

bool CancelScheduledReboot() {
    std::lock_guard<std::mutex> lk(s_rebootMutex);
    if (!s_rebootScheduled) return false;
    s_rebootScheduled = false;
    s_rebootTime.clear();
    spdlog::info("Scheduled reboot cancelled");
    return true;
}

bool IsRebootScheduled() {
    return s_rebootScheduled.load();
}

std::string GetScheduledRebootTime() {
    std::lock_guard<std::mutex> lk(s_rebootMutex);
    return s_rebootTime;
}

// ============================================================================
// 4. Factory Reset
// ============================================================================

bool PerformFactoryReset(FactoryResetMode mode, int delay_seconds) {
    spdlog::warn("Factory reset requested: mode={}, delay={}s",
                 mode == FactoryResetMode::Hard ? "hard" : "soft", delay_seconds);

    std::thread([mode, delay_seconds]() {
        if (delay_seconds > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(delay_seconds));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        std::string configDir = paths::kConfigDirActive;
        std::string factoryDir = paths::kConfigDirFactory;

        if (mode == FactoryResetMode::Hard) {
            // Full wipe: remove active configs, copy factory defaults
            spdlog::info("Factory reset HARD: wiping all config");
            std::string cmd = "rm -rf " + configDir + "/*";
            ::system(cmd.c_str());
            cmd = "cp -a " + factoryDir + "/* " + configDir + "/";
            ::system(cmd.c_str());

            // Remove user database
            if (fs::exists(paths::kUsersDatabase)) {
                fs::remove(paths::kUsersDatabase);
            }
            // Remove recordings
            std::string recCmd = "rm -rf /mnt/sd/recordings/* 2>/dev/null";
            ::system(recCmd.c_str());
        } else {
            // Soft reset: keep network.json and auth.json, replace everything else
            spdlog::info("Factory reset SOFT: keeping network & auth settings");
            // Backup network and auth
            ::system("cp /etc/ipcamera/configs/config.d/network.json /tmp/net_bak.json 2>/dev/null");
            ::system("cp /etc/ipcamera/configs/config.d/auth.json /tmp/auth_bak.json 2>/dev/null");

            std::string cmd = "rm -rf " + configDir + "/*";
            ::system(cmd.c_str());
            cmd = "cp -a " + factoryDir + "/* " + configDir + "/";
            ::system(cmd.c_str());

            // Restore network and auth
            ::system("cp /tmp/net_bak.json /etc/ipcamera/configs/config.d/network.json 2>/dev/null");
            ::system("cp /tmp/auth_bak.json /etc/ipcamera/configs/config.d/auth.json 2>/dev/null");
            ::system("rm -f /tmp/net_bak.json /tmp/auth_bak.json");
        }

        spdlog::info("Factory reset complete, rebooting");
        sync();
        ::reboot(RB_AUTOBOOT);
    }).detach();

    return true;
}

// ============================================================================
// 5. Diagnostics / Health
// ============================================================================

DiagnosticsReport GetDiagnostics() {
    DiagnosticsReport rpt;

    // --- CPU ---
    // Read /proc/stat twice with a short interval for utilisation
    auto readCpuJiffies = []() -> std::vector<std::vector<long>> {
        std::vector<std::vector<long>> out;
        std::ifstream f("/proc/stat");
        std::string line;
        while (std::getline(f, line)) {
            if (line.rfind("cpu", 0) != 0) break;
            std::istringstream ss(line);
            std::string label;
            ss >> label;
            std::vector<long> vals;
            long v;
            while (ss >> v) vals.push_back(v);
            out.push_back(vals);
        }
        return out;
    };

    auto j1 = readCpuJiffies();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto j2 = readCpuJiffies();

    auto cpuPercent = [](const std::vector<long>& a, const std::vector<long>& b) -> float {
        if (a.size() < 4 || b.size() < 4) return 0;
        long idle1 = a[3], idle2 = b[3];
        long total1 = 0, total2 = 0;
        for (auto v : a) total1 += v;
        for (auto v : b) total2 += v;
        long dTotal = total2 - total1;
        long dIdle  = idle2 - idle1;
        if (dTotal == 0) return 0;
        return 100.0f * (1.0f - (float)dIdle / (float)dTotal);
    };

    if (!j1.empty() && !j2.empty()) {
        rpt.cpu.usage_percent = cpuPercent(j1[0], j2[0]);
        rpt.cpu.num_cores = std::max(1, (int)j1.size() - 1);
        for (size_t i = 1; i < j1.size() && i < j2.size(); ++i) {
            rpt.cpu.per_core_percent.push_back(cpuPercent(j1[i], j2[i]));
        }
    }

    // --- Memory ---
    rpt.memory.total_bytes     = ReadMemInfoKb("MemTotal") * 1024;
    rpt.memory.free_bytes      = ReadMemInfoKb("MemFree") * 1024;
    rpt.memory.available_bytes = ReadMemInfoKb("MemAvailable") * 1024;
    rpt.memory.buffers_bytes   = ReadMemInfoKb("Buffers") * 1024;
    rpt.memory.cached_bytes    = ReadMemInfoKb("Cached") * 1024;
    if (rpt.memory.total_bytes > 0) {
        int64_t used = rpt.memory.total_bytes - rpt.memory.available_bytes;
        rpt.memory.usage_percent = 100.0f * (float)used / (float)rpt.memory.total_bytes;
    }

    // --- Disks ---
    auto addDisk = [&](const char* mount) {
        struct statvfs st;
        if (statvfs(mount, &st) == 0) {
            DiskInfo di;
            di.mount_point = mount;
            di.total_bytes = (int64_t)st.f_frsize * st.f_blocks;
            di.free_bytes  = (int64_t)st.f_frsize * st.f_bfree;
            if (di.total_bytes > 0) {
                di.usage_percent = 100.0f * (1.0f - (float)di.free_bytes / (float)di.total_bytes);
            }
            rpt.disks.push_back(di);
        }
    };
    addDisk("/");
    addDisk("/tmp");
    addDisk("/mnt/sd");

    // --- Processes --- (top 20 by memory)
    {
        DIR* dir = opendir("/proc");
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir))) {
                if (entry->d_type != DT_DIR) continue;
                char* end;
                long pid = strtol(entry->d_name, &end, 10);
                if (*end != '\0' || pid <= 0) continue;

                ProcessInfo pi;
                pi.pid = (int)pid;

                // Read comm
                std::string commPath = std::string("/proc/") + entry->d_name + "/comm";
                std::ifstream fc(commPath);
                if (fc) std::getline(fc, pi.name);

                // Read status for VmRSS and state
                std::string statusPath = std::string("/proc/") + entry->d_name + "/status";
                std::ifstream fs(statusPath);
                std::string line;
                while (std::getline(fs, line)) {
                    if (line.rfind("State:", 0) == 0) {
                        pi.state = line.substr(7, 1);
                    } else if (line.rfind("VmRSS:", 0) == 0) {
                        std::istringstream ss(line.substr(6));
                        ss >> pi.memory_kb;
                    }
                }

                if (!pi.name.empty()) rpt.processes.push_back(pi);
            }
            closedir(dir);
        }
        // Sort by memory descending, keep top 20
        std::sort(rpt.processes.begin(), rpt.processes.end(),
                  [](const ProcessInfo& a, const ProcessInfo& b) { return a.memory_kb > b.memory_kb; });
        if (rpt.processes.size() > 20) rpt.processes.resize(20);
    }

    // --- Uptime ---
    rpt.uptime_seconds = ReadUptimeSeconds();
    rpt.uptime = FormatUptime(rpt.uptime_seconds);

    // --- Temperature ---
    rpt.temperature = ReadSocTemperature();

    // --- Kernel ---
    struct utsname un;
    if (uname(&un) == 0) {
        rpt.kernel_version = std::string(un.sysname) + " " + un.release;
    }

    return rpt;
}

// ============================================================================
// 6. Logs
// ============================================================================

// ============================================================================
// 6. Logs  — Multi-source log management
// ============================================================================

const char* LogSourceToString(LogSource src) {
    switch (src) {
        case LogSource::Application: return "application";
        case LogSource::Kernel:      return "kernel";
        case LogSource::System:      return "system";
        case LogSource::Thermal:     return "thermal";
        case LogSource::Access:      return "access";
        case LogSource::Security:    return "security";
        case LogSource::Audit:       return "audit";
        case LogSource::Nginx:       return "nginx";
    }
    return "application";
}

LogSource LogSourceFromString(const std::string& s) {
    if (s == "kernel")      return LogSource::Kernel;
    if (s == "system")      return LogSource::System;
    if (s == "thermal")     return LogSource::Thermal;
    if (s == "access")      return LogSource::Access;
    if (s == "security")    return LogSource::Security;
    if (s == "audit")       return LogSource::Audit;
    if (s == "nginx")       return LogSource::Nginx;
    return LogSource::Application;
}

static std::string LogSourcePath(LogSource src) {
    switch (src) {
        case LogSource::Application: return config::Get<std::string>("logging.file.path", paths::kLogFile);
        case LogSource::Kernel:      return "/mnt/app/logs/dmesg.log";
        case LogSource::System:      return "/mnt/app/logs/sysdiag.log";
        case LogSource::Thermal:     return "/mnt/app/logs/thermal.log";
        case LogSource::Access:      return config::Get<std::string>("logging.access.path", "/var/log/ipcamd_access.log");
        case LogSource::Security:    return config::Get<std::string>("logging.security.path", "/var/log/ipcamd_security.log");
        case LogSource::Audit:       return config::Get<std::string>("logging.audit.path", "/var/log/ipcamd_audit.log");
        case LogSource::Nginx:       return "/var/log/nginx/error.log";
    }
    return paths::kLogFile;
}

static std::string LogSourceSdPath(LogSource src) {
    switch (src) {
        case LogSource::Kernel:  return "/mnt/sd/logs/";  // dmesg_YYYYMMDD.log
        case LogSource::System:  return "/mnt/sd/logs/";   // sysdiag_YYYYMMDD.log
        case LogSource::Thermal: return "/mnt/sd/logs/";   // thermal_YYYYMMDD.log
        case LogSource::Application: return "/mnt/sd/logs/"; // ipcamd_YYYYMMDD.log
        default: return "";
    }
}

std::vector<LogSourceInfo> GetLogSources() {
    std::vector<LogSourceInfo> sources;

    auto addSource = [&](LogSource src, const std::string& name,
                         const std::string& desc) {
        LogSourceInfo info;
        info.id = LogSourceToString(src);
        info.display_name = name;
        info.description = desc;
        info.file_path = LogSourcePath(src);
        info.sd_path = LogSourceSdPath(src);

        struct stat st{};
        if (stat(info.file_path.c_str(), &st) == 0) {
            info.file_size = st.st_size;
            info.available = true;
        } else {
            info.available = false;
        }
        sources.push_back(std::move(info));
    };

    addSource(LogSource::Application, "Application",
              "ipcamd application log — config, pipeline, NTP, API handlers");
    addSource(LogSource::Kernel, "Kernel",
              "Linux kernel ring buffer — hardware errors, driver events, OOM");
    addSource(LogSource::System, "System Diagnostics",
              "Periodic CPU, memory, load, disk, network, temperature snapshots");
    addSource(LogSource::Thermal, "Thermal",
              "SoC temperature history");
    addSource(LogSource::Access, "Access",
              "HTTP API request log — who accessed what endpoint, when, from where");
    addSource(LogSource::Security, "Security",
              "Authentication events — login, logout, failures, lockouts, brute-force");
    addSource(LogSource::Audit, "Audit",
              "Configuration change audit trail — what changed, by whom, old/new values");
    addSource(LogSource::Nginx, "Nginx",
              "Reverse proxy errors — upstream failures, SSL errors, connection issues");

    return sources;
}

// Level priority helper (reusable across sources)
static int LevelPriority(const std::string& l) {
    if (l == "trace")    return 0;
    if (l == "debug")    return 1;
    if (l == "info")     return 2;
    if (l == "warning" || l == "warn") return 3;
    if (l == "error" || l == "err")    return 4;
    if (l == "critical") return 5;
    return 0;
}

// Parse spdlog-format lines: [2026-02-20 15:30:00.123] [component] [level] message
static bool ParseSpdlogLine(const std::string& line, LogEntry& e) {
    // Fast prefix check
    if (line.size() < 30 || line[0] != '[') return false;

    static const std::regex re(R"(\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d+)\] \[([^\]]*)\] \[([^\]]*)\] (.*))");
    std::smatch m;
    if (!std::regex_match(line, m, re)) return false;

    e.timestamp = m[1].str();
    e.source    = m[2].str();
    e.level     = m[3].str();
    e.message   = m[4].str();
    return true;
}

// Parse audit/access/security log lines:
// [2026-02-20 15:30:00.123] [category] [level] key=val key=val ...
static bool ParseAuditLine(const std::string& line, LogEntry& e) {
    if (line.size() < 30 || line[0] != '[') return false;

    static const std::regex re(R"(\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d+)\] \[([^\]]*)\] \[([^\]]*)\] (.*))");
    std::smatch m;
    if (!std::regex_match(line, m, re)) return false;

    e.timestamp = m[1].str();
    e.source    = m[2].str();
    e.level     = m[3].str();
    e.message   = m[4].str();
    return true;
}

// Parse simple timestamp-prefixed lines: [2026-02-20 15:30:00] message
static bool ParseTimestampedLine(const std::string& line, LogEntry& e,
                                  const std::string& defaultSource) {
    if (line.size() < 22 || line[0] != '[') {
        // Lines without timestamps (e.g., dmesg continuation)
        e.timestamp = "";
        e.source = defaultSource;
        e.level = "info";
        e.message = line;
        return !line.empty();
    }

    // Try [YYYY-MM-DD HH:MM:SS] format
    size_t close = line.find(']');
    if (close != std::string::npos && close >= 19) {
        e.timestamp = line.substr(1, close - 1);
        e.source = defaultSource;
        e.level = "info";
        e.message = (close + 2 < line.size()) ? line.substr(close + 2) : "";
        return true;
    }

    e.timestamp = "";
    e.source = defaultSource;
    e.level = "info";
    e.message = line;
    return true;
}

// Parse nginx error log: YYYY/MM/DD HH:MM:SS [level] PID#TID: ...
static bool ParseNginxLine(const std::string& line, LogEntry& e) {
    // Format: 2020/01/01 00:03:52 [crit] 8535#0: ...
    if (line.size() < 25) return false;

    // Extract date
    if (line[4] == '/' && line[7] == '/') {
        e.timestamp = line.substr(0, 19);
        // Replace / with -
        std::replace(e.timestamp.begin(), e.timestamp.end(), '/', '-');
    } else {
        e.timestamp = "";
    }

    // Find [level]
    size_t lb = line.find('[', 19);
    size_t rb = (lb != std::string::npos) ? line.find(']', lb) : std::string::npos;
    if (lb != std::string::npos && rb != std::string::npos) {
        std::string lvl = line.substr(lb + 1, rb - lb - 1);
        if (lvl == "emerg" || lvl == "alert" || lvl == "crit") e.level = "critical";
        else if (lvl == "error") e.level = "error";
        else if (lvl == "warn") e.level = "warning";
        else e.level = "info";
    } else {
        e.level = "info";
    }

    e.source = "nginx";
    // Message is everything after the PID#TID:
    size_t colon = (rb != std::string::npos) ? line.find(':', rb) : std::string::npos;
    if (colon != std::string::npos && colon + 2 < line.size()) {
        e.message = line.substr(colon + 2);
    } else {
        e.message = line;
    }
    return true;
}

std::vector<LogEntry> GetLogs(const LogFilter& filter) {
    std::vector<LogEntry> entries;

    std::string path = LogSourcePath(filter.log_source);
    std::ifstream f(path);
    if (!f.is_open()) return entries;

    int minLevel = filter.level.empty() ? 0 : LevelPriority(filter.level);

    std::string line;
    int skipped = 0;
    while (std::getline(f, line) && (int)entries.size() < filter.limit) {
        if (line.empty()) continue;

        LogEntry e;
        bool parsed = false;

        switch (filter.log_source) {
            case LogSource::Application:
                parsed = ParseSpdlogLine(line, e);
                break;
            case LogSource::Access:
            case LogSource::Security:
            case LogSource::Audit:
                parsed = ParseAuditLine(line, e);
                break;
            case LogSource::Nginx:
                parsed = ParseNginxLine(line, e);
                break;
            case LogSource::Kernel:
                parsed = ParseTimestampedLine(line, e, "kernel");
                break;
            case LogSource::System:
                parsed = ParseTimestampedLine(line, e, "sysdiag");
                break;
            case LogSource::Thermal:
                parsed = ParseTimestampedLine(line, e, "thermal");
                break;
        }

        if (!parsed) continue;

        // Level filter
        if (LevelPriority(e.level) < minLevel) continue;

        // Time range filter
        if (!filter.start_time.empty() && e.timestamp < filter.start_time) continue;
        if (!filter.end_time.empty()   && e.timestamp > filter.end_time)   continue;

        // Text search
        if (!filter.search.empty() &&
            e.message.find(filter.search) == std::string::npos &&
            e.source.find(filter.search) == std::string::npos) continue;

        // Offset
        if (skipped < filter.offset) { ++skipped; continue; }

        entries.push_back(std::move(e));
    }

    return entries;
}

std::string GetLogFilePath() {
    return config::Get<std::string>("logging.file.path", paths::kLogFile);
}

std::string GetLogFilePath(LogSource src) {
    return LogSourcePath(src);
}

bool ClearLogs() {
    std::string path = GetLogFilePath();
    std::ofstream f(path, std::ios::trunc);
    if (!f) {
        spdlog::error("ClearLogs: failed to truncate {}", path);
        return false;
    }
    spdlog::info("Logs cleared");
    return true;
}

bool ClearLogs(LogSource src) {
    std::string path = LogSourcePath(src);
    std::ofstream f(path, std::ios::trunc);
    if (!f) {
        spdlog::error("ClearLogs({}): failed to truncate {}", LogSourceToString(src), path);
        return false;
    }
    spdlog::info("Logs cleared: source={}", LogSourceToString(src));
    return true;
}

// ---- Diagnostic Bundle ----

std::string CreateDiagnosticBundle() {
    std::string bundle_path = "/tmp/diagnostic_bundle.tar.gz";

    // Create a staging directory
    std::string staging = "/tmp/diag_bundle";
    ::system(("rm -rf " + staging).c_str());
    ::system(("mkdir -p " + staging + "/logs " + staging + "/config " +
              staging + "/hardware " + staging + "/network").c_str());

    // 1. Copy all log files
    ::system(("cp /var/log/ipcamd.log " + staging + "/logs/ 2>/dev/null").c_str());
    ::system(("cp /var/log/ipcamd_access.log " + staging + "/logs/ 2>/dev/null").c_str());
    ::system(("cp /var/log/ipcamd_security.log " + staging + "/logs/ 2>/dev/null").c_str());
    ::system(("cp /var/log/ipcamd_audit.log " + staging + "/logs/ 2>/dev/null").c_str());
    ::system(("cp /var/log/nginx/error.log " + staging + "/logs/nginx_error.log 2>/dev/null").c_str());
    ::system(("cp /mnt/app/logs/*.log " + staging + "/logs/ 2>/dev/null").c_str());

    // 2. Capture live dmesg
    ::system(("dmesg > " + staging + "/logs/dmesg_live.log 2>/dev/null").c_str());

    // 3. Copy config files (sanitize passwords)
    ::system(("cp -r /etc/ipcamera/configs/config.factory.d " + staging + "/config/ 2>/dev/null").c_str());
    // Remove sensitive auth data from bundle
    ::system(("rm -f " + staging + "/config/config.factory.d/auth.json 2>/dev/null").c_str());

    // 4. Hardware info
    ::system(("cat /proc/cpuinfo > " + staging + "/hardware/cpuinfo.txt 2>/dev/null").c_str());
    ::system(("cat /proc/meminfo > " + staging + "/hardware/meminfo.txt 2>/dev/null").c_str());
    ::system(("cat /proc/version > " + staging + "/hardware/kernel.txt 2>/dev/null").c_str());
    ::system(("cat /sys/class/thermal/thermal_zone0/temp > " + staging + "/hardware/temperature.txt 2>/dev/null").c_str());
    ::system(("df -h > " + staging + "/hardware/disk.txt 2>/dev/null").c_str());
    ::system(("cat /proc/uptime > " + staging + "/hardware/uptime.txt 2>/dev/null").c_str());
    ::system(("free > " + staging + "/hardware/memory.txt 2>/dev/null").c_str());
    ::system(("ps -ef > " + staging + "/hardware/processes.txt 2>/dev/null || ps > " + staging + "/hardware/processes.txt 2>/dev/null").c_str());
    ::system(("cat /proc/loadavg > " + staging + "/hardware/loadavg.txt 2>/dev/null").c_str());

    // 5. Network state
    ::system(("ip addr > " + staging + "/network/interfaces.txt 2>/dev/null || ifconfig > " + staging + "/network/interfaces.txt 2>/dev/null").c_str());
    ::system(("ip route > " + staging + "/network/routes.txt 2>/dev/null || route > " + staging + "/network/routes.txt 2>/dev/null").c_str());
    ::system(("cat /etc/resolv.conf > " + staging + "/network/dns.txt 2>/dev/null").c_str());
    ::system(("netstat -tlnp > " + staging + "/network/ports.txt 2>/dev/null || ss -tlnp > " + staging + "/network/ports.txt 2>/dev/null").c_str());
    ::system(("cat /proc/net/dev > " + staging + "/network/netdev.txt 2>/dev/null").c_str());

    // 6. Generate manifest
    {
        std::ofstream manifest(staging + "/manifest.txt");
        if (manifest.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto t = std::chrono::system_clock::to_time_t(now);
            struct tm tm_buf{};
            gmtime_r(&t, &tm_buf);
            char ts[32];
            std::strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &tm_buf);

            char hostname[64] = {};
            gethostname(hostname, sizeof(hostname) - 1);

            manifest << "Diagnostic Bundle\n";
            manifest << "Generated: " << ts << "\n";
            manifest << "Hostname: " << hostname << "\n";

            struct utsname uts{};
            if (uname(&uts) == 0) {
                manifest << "Kernel: " << uts.release << "\n";
                manifest << "Machine: " << uts.machine << "\n";
            }

            struct sysinfo si{};
            if (sysinfo(&si) == 0) {
                manifest << "Uptime: " << si.uptime << " seconds\n";
                manifest << "Total RAM: " << (si.totalram * si.mem_unit / 1048576) << " MB\n";
                manifest << "Free RAM: " << (si.freeram * si.mem_unit / 1048576) << " MB\n";
            }
            manifest.close();
        }
    }

    // 7. Create tar.gz
    std::string cmd = "cd /tmp && tar czf diagnostic_bundle.tar.gz -C " + staging + " . 2>/dev/null";
    int ret = ::system(cmd.c_str());
    ::system(("rm -rf " + staging).c_str());

    if (ret != 0) {
        spdlog::error("Failed to create diagnostic bundle archive");
        return "";
    }

    spdlog::info("Diagnostic bundle created: {}", bundle_path);
    return bundle_path;
}

LogConfig GetLogConfig() {
    LogConfig c;
    c.level              = config::Get<std::string>("logging.level", "info");
    c.console_enabled    = config::Get<bool>("logging.console.enabled", true);
    c.console_level      = config::Get<std::string>("logging.console.level", "info");
    c.file_enabled       = config::Get<bool>("logging.file.enabled", true);
    c.file_path          = config::Get<std::string>("logging.file.path", paths::kLogFile);
    c.file_level         = config::Get<std::string>("logging.file.level", "debug");
    c.max_size_mb        = config::Get<int>("logging.file.max_size_mb", 5);
    c.max_files          = config::Get<int>("logging.file.max_files", 3);
    c.flush_interval_sec = config::Get<int>("logging.file.flush_interval_sec", 5);
    c.format             = config::Get<std::string>("logging.format", "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
    return c;
}

bool SetLogConfig(const LogConfig& cfg) {
    config::Set<std::string>("logging.level", cfg.level);
    config::Set<bool>("logging.console.enabled", cfg.console_enabled);
    config::Set<std::string>("logging.console.level", cfg.console_level);
    config::Set<bool>("logging.file.enabled", cfg.file_enabled);
    config::Set<std::string>("logging.file.path", cfg.file_path);
    config::Set<std::string>("logging.file.level", cfg.file_level);
    config::Set<int>("logging.file.max_size_mb", cfg.max_size_mb);
    config::Set<int>("logging.file.max_files", cfg.max_files);
    config::Set<int>("logging.file.flush_interval_sec", cfg.flush_interval_sec);
    config::Set<std::string>("logging.format", cfg.format);
    config::Save();

    // Apply runtime log level change via spdlog
    auto slevel = spdlog::level::from_str(cfg.level);
    spdlog::set_level(slevel);

    spdlog::info("Log config updated: level={}", cfg.level);
    return true;
}

// ============================================================================
// 7. Maintenance
// ============================================================================

MaintenanceConfig GetMaintenanceConfig() {
    MaintenanceConfig c;
    c.auto_restart_enabled = config::Get<bool>("system.maintenance.auto_restart_enabled", false);
    c.restart_time         = config::Get<std::string>("system.maintenance.restart_time", "03:00:00");
    c.restart_days         = config::Get<std::string>("system.maintenance.restart_days", "daily");
    return c;
}

bool SetMaintenanceConfig(const MaintenanceConfig& cfg) {
    config::Set<bool>("system.maintenance.auto_restart_enabled", cfg.auto_restart_enabled);
    config::Set<std::string>("system.maintenance.restart_time", cfg.restart_time);
    config::Set<std::string>("system.maintenance.restart_days", cfg.restart_days);
    config::Save();
    spdlog::info("Maintenance config updated: enabled={}, time={}, days={}",
                 cfg.auto_restart_enabled, cfg.restart_time, cfg.restart_days);
    return true;
}

// ============================================================================
// 8. Watchdog
// ============================================================================

WatchdogConfig GetWatchdogConfig() {
    WatchdogConfig c;
    c.enabled          = config::Get<bool>("system.watchdog.enabled", true);
    c.interval_seconds = config::Get<int>("system.watchdog.interval_seconds", 30);
    return c;
}

bool SetWatchdogConfig(const WatchdogConfig& cfg) {
    config::Set<bool>("system.watchdog.enabled", cfg.enabled);
    config::Set<int>("system.watchdog.interval_seconds", cfg.interval_seconds);
    config::Save();

    // Enable/disable hardware watchdog
    if (cfg.enabled) {
        // Open /dev/watchdog to enable (BusyBox watchdog daemon)
        std::string cmd = "watchdog -t " + std::to_string(cfg.interval_seconds) + " /dev/watchdog 2>/dev/null &";
        ::system("killall watchdog 2>/dev/null");
        ::system(cmd.c_str());
        spdlog::info("Watchdog enabled, interval={}s", cfg.interval_seconds);
    } else {
        // Write 'V' (magic close) to disable watchdog cleanly
        ::system("killall watchdog 2>/dev/null");
        int fd = open("/dev/watchdog", O_WRONLY);
        if (fd >= 0) {
            write(fd, "V", 1);
            close(fd);
        }
        spdlog::info("Watchdog disabled");
    }
    return true;
}

// ============================================================================
// 10. Config Backup / Restore
// ============================================================================

std::string CreateConfigBackup() {
    std::string outPath = "/tmp/config_backup.tar.gz";
    std::string configDir = paths::kConfigDirActive;

    std::string cmd = "tar czf " + outPath + " -C " + configDir + " . 2>&1";
    std::string out = ExecCommand(cmd);
    if (!fs::exists(outPath)) {
        spdlog::error("CreateConfigBackup: tar failed: {}", out);
        return {};
    }
    spdlog::info("Config backup created: {}", outPath);
    return outPath;
}

std::string RestoreConfigBackup(const std::string& archivePath) {
    if (!fs::exists(archivePath)) {
        return "Archive file not found: " + archivePath;
    }

    // Validate path to prevent command injection
    if (!IsValidPath(archivePath)) {
        spdlog::error("RestoreConfigBackup: invalid archive path rejected");
        return "Invalid archive path";
    }

    std::string configDir = paths::kConfigDirActive;

    // Verify it's a valid tar archive
    std::string testCmd = "tar tzf " + archivePath + " >/dev/null 2>&1";
    if (::system(testCmd.c_str()) != 0) {
        return "Invalid archive format";
    }

    // Backup current config before overwriting
    std::string backupCmd = "cp -a " + configDir + " " + configDir + ".bak 2>/dev/null";
    ::system(backupCmd.c_str());

    // Extract into config directory
    std::string extractCmd = "tar xzf " + archivePath + " -C " + configDir + " 2>&1";
    std::string out = ExecCommand(extractCmd);

    // Remove uploaded file
    fs::remove(archivePath);

    // Reload config
    spdlog::info("Config restored from backup, reloading...");
    std::string err;
    config::Init(paths::kConfigDir, err);

    return {};  // empty = success
}

// ============================================================================
// 11. Alarm Thresholds
// ============================================================================

AlarmThresholds GetAlarmThresholds() {
    AlarmThresholds t;
    t.disk_space_threshold   = config::Get<int>("system.alarms.disk_space_threshold", 90);
    t.temperature_threshold  = config::Get<double>("system.alarms.temperature_threshold", 70.0);
    t.memory_threshold       = config::Get<int>("system.alarms.memory_threshold", 90);
    t.alarm_outputs_number   = config::Get<int>("system.alarms.alarm_outputs_number", 0);
    return t;
}

bool SetAlarmThresholds(const AlarmThresholds& t) {
    if (t.disk_space_threshold < 0 || t.disk_space_threshold > 100) return false;
    if (t.memory_threshold < 0 || t.memory_threshold > 100) return false;
    if (t.temperature_threshold < 0 || t.temperature_threshold > 150) return false;

    config::Set<int>("system.alarms.disk_space_threshold", t.disk_space_threshold);
    config::Set<double>("system.alarms.temperature_threshold", (double)t.temperature_threshold);
    config::Set<int>("system.alarms.memory_threshold", t.memory_threshold);
    config::Set<int>("system.alarms.alarm_outputs_number", t.alarm_outputs_number);
    config::Save();
    spdlog::info("Alarm thresholds updated: disk={}%, temp={}°C, mem={}%",
                 t.disk_space_threshold, t.temperature_threshold, t.memory_threshold);
    return true;
}

} // namespace platform
} // namespace ipcam
