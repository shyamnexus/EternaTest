/**
 * @file system_logger.cpp
 * @brief SystemLogger implementation — API-controllable system data collector
 *
 * Replaces the monolithic system_logger.sh shell script.
 * All collection and destinations are controllable via config and REST API.
 */

#include <ipcam/system_logger.h>
#include <ipcam/config.h>
#include <spdlog/spdlog.h>

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdio>

#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <unistd.h>

namespace ipcam {
namespace platform {

// ============================================================================
// String conversions
// ============================================================================

const char* CollectionSourceToString(CollectionSource src) {
    switch (src) {
        case CollectionSource::Kernel:    return "kernel";
        case CollectionSource::Thermal:   return "thermal";
        case CollectionSource::SysDiag:   return "sysdiag";
        case CollectionSource::AppMirror: return "app_mirror";
    }
    return "kernel";
}

CollectionSource CollectionSourceFromString(const std::string& s) {
    if (s == "thermal")    return CollectionSource::Thermal;
    if (s == "sysdiag")    return CollectionSource::SysDiag;
    if (s == "app_mirror") return CollectionSource::AppMirror;
    return CollectionSource::Kernel;
}

// ============================================================================
// Singleton
// ============================================================================

SystemLogger& SystemLogger::Instance() {
    static SystemLogger instance;
    return instance;
}

// ============================================================================
// Timestamp helpers
// ============================================================================

static std::string Now() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf{};
    localtime_r(&t, &tm_buf);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    return buf;
}

static std::string DateStamp() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf{};
    localtime_r(&t, &tm_buf);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y%m%d", &tm_buf);
    return buf;
}

static int64_t UnixTime() {
    return static_cast<int64_t>(std::time(nullptr));
}

// ============================================================================
// Config loading / saving
// ============================================================================

void SystemLogger::LoadConfig() {
    config_.enabled = config::Get<bool>("logging.system_logger.enabled", true);

    // Sources
    auto loadSrc = [&](CollectionSource src, const std::string& key, int default_interval) {
        CollectionSourceConfig c;
        c.enabled      = config::Get<bool>("logging.system_logger.sources." + key + ".enabled", true);
        c.interval_sec = config::Get<int>("logging.system_logger.sources." + key + ".interval_sec", default_interval);
        if (c.interval_sec < 5)  c.interval_sec = 5;   // Floor: 5 seconds
        if (c.interval_sec > 3600) c.interval_sec = 3600; // Ceiling: 1 hour
        config_.sources[src] = c;
    };

    loadSrc(CollectionSource::Kernel,    "kernel",     30);
    loadSrc(CollectionSource::Thermal,   "thermal",    60);
    loadSrc(CollectionSource::SysDiag,   "sysdiag",    60);
    loadSrc(CollectionSource::AppMirror, "app_mirror", 120);

    // SD card destination
    config_.sd_card.enabled      = config::Get<bool>("logging.system_logger.destinations.sd_card.enabled", false);
    config_.sd_card.path         = config::Get<std::string>("logging.system_logger.destinations.sd_card.path", "/mnt/sd/logs");
    config_.sd_card.max_total_mb = config::Get<int>("logging.system_logger.destinations.sd_card.max_total_mb", 500);
    config_.sd_card.max_days     = config::Get<int>("logging.system_logger.destinations.sd_card.max_days", 30);
    config_.sd_card.min_free_mb  = config::Get<int>("logging.system_logger.destinations.sd_card.min_free_mb", 100);

    // Flash destination
    config_.flash.enabled      = config::Get<bool>("logging.system_logger.destinations.flash.enabled", true);
    config_.flash.path         = config::Get<std::string>("logging.system_logger.destinations.flash.path", "/mnt/app/logs");
    config_.flash.max_total_kb = config::Get<int>("logging.system_logger.destinations.flash.max_total_kb", 3072);
    config_.flash.max_file_kb  = config::Get<int>("logging.system_logger.destinations.flash.max_file_kb", 1024);
    config_.flash.min_free_kb  = config::Get<int>("logging.system_logger.destinations.flash.min_free_kb", 512);
}

void SystemLogger::SaveConfig() {
    config::Set("logging.system_logger.enabled", config_.enabled);

    auto saveSrc = [&](CollectionSource src, const std::string& key) {
        auto& c = config_.sources[src];
        config::Set("logging.system_logger.sources." + key + ".enabled", c.enabled);
        config::Set("logging.system_logger.sources." + key + ".interval_sec", c.interval_sec);
    };

    saveSrc(CollectionSource::Kernel,    "kernel");
    saveSrc(CollectionSource::Thermal,   "thermal");
    saveSrc(CollectionSource::SysDiag,   "sysdiag");
    saveSrc(CollectionSource::AppMirror, "app_mirror");

    config::Set("logging.system_logger.destinations.sd_card.enabled", config_.sd_card.enabled);
    config::Set("logging.system_logger.destinations.sd_card.max_total_mb", static_cast<int>(config_.sd_card.max_total_mb));
    config::Set("logging.system_logger.destinations.sd_card.max_days", config_.sd_card.max_days);
    config::Set("logging.system_logger.destinations.sd_card.min_free_mb", static_cast<int>(config_.sd_card.min_free_mb));
    config::Set("logging.system_logger.destinations.flash.enabled", config_.flash.enabled);
    config::Set("logging.system_logger.destinations.flash.max_total_kb", static_cast<int>(config_.flash.max_total_kb));
    config::Set("logging.system_logger.destinations.flash.max_file_kb", static_cast<int>(config_.flash.max_file_kb));
    config::Set("logging.system_logger.destinations.flash.min_free_kb", static_cast<int>(config_.flash.min_free_kb));
}

// ============================================================================
// Init / Shutdown
// ============================================================================

bool SystemLogger::Init() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) return true;

    LoadConfig();

    // Initialize source states
    for (auto& [src, cfg] : config_.sources) {
        source_state_[src] = SourceState{};
    }

    // Initialize dmesg tracking
    {
        FILE* fp = popen("dmesg 2>/dev/null | wc -l", "r");
        if (fp) {
            char buf[32];
            if (fgets(buf, sizeof(buf), fp)) {
                dmesg_last_lines_ = std::atoi(buf);
            }
            pclose(fp);
        }
    }

    // Create destination directories
    if (config_.sd_card.enabled) {
        mkdir(config_.sd_card.path.c_str(), 0755);
    }
    if (config_.flash.enabled) {
        mkdir(config_.flash.path.c_str(), 0755);
    }

    // Check initial availability
    sd_available_ = IsSDAvailable();
    flash_available_ = IsFlashAvailable();

    if (!config_.enabled) {
        spdlog::info("SystemLogger: disabled in config, loaded but not starting worker");
        initialized_ = true;
        return true;
    }

    // Start worker thread
    running_ = true;
    worker_ = std::thread(&SystemLogger::WorkerThread, this);

    initialized_ = true;
    spdlog::info("SystemLogger: started (SD={}, flash={}, sources: kernel={}s thermal={}s sysdiag={}s app_mirror={}s)",
                 sd_available_ ? "available" : "unavailable",
                 flash_available_ ? "available" : "unavailable",
                 config_.sources[CollectionSource::Kernel].interval_sec,
                 config_.sources[CollectionSource::Thermal].interval_sec,
                 config_.sources[CollectionSource::SysDiag].interval_sec,
                 config_.sources[CollectionSource::AppMirror].interval_sec);
    return true;
}

void SystemLogger::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) return;
        initialized_ = false;
    }

    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
    spdlog::info("SystemLogger: stopped");
}

void SystemLogger::ReloadConfig() {
    std::lock_guard<std::mutex> lock(mutex_);
    LoadConfig();

    // If was disabled and now enabled, start worker
    if (config_.enabled && !running_) {
        running_ = true;
        if (worker_.joinable()) worker_.join();
        worker_ = std::thread(&SystemLogger::WorkerThread, this);
        spdlog::info("SystemLogger: started via config reload");
    }
    // If was running and now disabled, stop worker
    if (!config_.enabled && running_) {
        running_ = false;
        // Worker will exit on next tick
    }
}

// ============================================================================
// Runtime Control
// ============================================================================

void SystemLogger::SetSourceEnabled(CollectionSource src, bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.sources[src].enabled = enabled;
    SaveConfig();
}

bool SystemLogger::IsSourceEnabled(CollectionSource src) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = config_.sources.find(src);
    return (it != config_.sources.end()) ? it->second.enabled : false;
}

void SystemLogger::SetSourceInterval(CollectionSource src, int seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (seconds < 5)    seconds = 5;
    if (seconds > 3600) seconds = 3600;
    config_.sources[src].interval_sec = seconds;
    SaveConfig();
}

int SystemLogger::GetSourceInterval(CollectionSource src) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = config_.sources.find(src);
    return (it != config_.sources.end()) ? it->second.interval_sec : 60;
}

void SystemLogger::SetDestinationEnabled(const std::string& dest_id, bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (dest_id == "sd_card")     config_.sd_card.enabled = enabled;
    else if (dest_id == "flash")  config_.flash.enabled = enabled;
    SaveConfig();
}

bool SystemLogger::IsDestinationEnabled(const std::string& dest_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (dest_id == "sd_card") return config_.sd_card.enabled;
    if (dest_id == "flash")   return config_.flash.enabled;
    return false;
}

void SystemLogger::SetEnabled(bool enabled) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.enabled = enabled;
        SaveConfig();
    }
    ReloadConfig();
}

bool SystemLogger::IsEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.enabled;
}

// ============================================================================
// Status
// ============================================================================

SystemLoggerStatus SystemLogger::GetStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    SystemLoggerStatus status;
    status.running = running_.load();

    for (auto& [src, cfg] : config_.sources) {
        CollectionSourceStatus ss;
        ss.id = CollectionSourceToString(src);
        ss.enabled = cfg.enabled;
        ss.interval_sec = cfg.interval_sec;
        auto sit = source_state_.find(src);
        if (sit != source_state_.end()) {
            ss.last_collection = sit->second.last_collection;
            ss.entries_collected = sit->second.entries_collected;
        }
        status.sources.push_back(ss);
    }

    // SD card
    {
        DestinationStatus ds;
        ds.id = "sd_card";
        ds.path = config_.sd_card.path;
        ds.enabled = config_.sd_card.enabled;
        ds.available = sd_available_;
        ds.read_write = true; // updated at runtime
        int64_t free_kb = GetFreeSpaceKB("/mnt/sd");
        ds.free_bytes = free_kb * 1024;
        ds.used_bytes = GetDirSizeKB(config_.sd_card.path) * 1024;
        struct statvfs st{};
        if (statvfs("/mnt/sd", &st) == 0) {
            ds.total_bytes = static_cast<int64_t>(st.f_blocks) * st.f_frsize;
        }
        status.destinations.push_back(ds);
    }

    // Flash
    {
        DestinationStatus ds;
        ds.id = "flash";
        ds.path = config_.flash.path;
        ds.enabled = config_.flash.enabled;
        ds.available = flash_available_;
        ds.read_write = true;
        int64_t free_kb = GetFreeSpaceKB("/mnt/app");
        ds.free_bytes = free_kb * 1024;
        ds.used_bytes = GetDirSizeKB(config_.flash.path) * 1024;
        struct statvfs st{};
        if (statvfs("/mnt/app", &st) == 0) {
            ds.total_bytes = static_cast<int64_t>(st.f_blocks) * st.f_frsize;
        }
        status.destinations.push_back(ds);
    }

    return status;
}

SystemLoggerConfig SystemLogger::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

// ============================================================================
// Force Collection
// ============================================================================

void SystemLogger::CollectNow(CollectionSource src) {
    switch (src) {
        case CollectionSource::Kernel:    CollectKernel();  break;
        case CollectionSource::Thermal:   CollectThermal(); break;
        case CollectionSource::SysDiag:   CollectSysDiag(); break;
        case CollectionSource::AppMirror: MirrorAppLog();   break;
    }
}

void SystemLogger::CollectAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [src, cfg] : config_.sources) {
        if (cfg.enabled) {
            // Release lock for actual collection
            mutex_.unlock();
            CollectNow(src);
            mutex_.lock();
        }
    }
}

// ============================================================================
// Worker Thread
// ============================================================================

void SystemLogger::WorkerThread() {
    constexpr int kTickSec = 5;
    constexpr int kStorageCheckSec = 300;

    int storage_counter = 0;
    std::map<CollectionSource, int> counters;

    // Initialize all counters to 0
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [src, cfg] : config_.sources) {
            counters[src] = 0;
        }
    }

    spdlog::debug("SystemLogger: worker thread started (tick={}s)", kTickSec);

    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(kTickSec));
        if (!running_) break;

        // Read config under lock
        SystemLoggerConfig cfg_snapshot;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!config_.enabled) continue;
            cfg_snapshot = config_;
        }

        // Periodic storage check
        storage_counter += kTickSec;
        if (storage_counter >= kStorageCheckSec) {
            sd_available_ = IsSDAvailable();
            flash_available_ = IsFlashAvailable();
            EnsureSDSpace();
            EnsureFlashSpace();
            storage_counter = 0;
        }

        // Check each source
        for (auto& [src, cfg] : cfg_snapshot.sources) {
            if (!cfg.enabled) continue;

            counters[src] += kTickSec;
            if (counters[src] >= cfg.interval_sec) {
                CollectNow(src);
                counters[src] = 0;
            }
        }
    }

    spdlog::debug("SystemLogger: worker thread exiting");
}

// ============================================================================
// Collectors
// ============================================================================

void SystemLogger::CollectKernel() {
    FILE* fp = popen("dmesg 2>/dev/null", "r");
    if (!fp) return;

    std::vector<std::string> lines;
    char buf[1024];
    while (fgets(buf, sizeof(buf), fp)) {
        lines.push_back(buf);
    }
    pclose(fp);

    int current_lines = static_cast<int>(lines.size());
    if (current_lines <= dmesg_last_lines_) {
        // Kernel ring buffer may have wrapped — if it shrank, collect all
        if (current_lines < dmesg_last_lines_) {
            dmesg_last_lines_ = 0;
        } else {
            return; // No new entries
        }
    }

    int new_count = current_lines - dmesg_last_lines_;
    dmesg_last_lines_ = current_lines;

    if (new_count <= 0) return;

    std::string timestamp = Now();
    std::string header = "[" + timestamp + "] === New dmesg entries (" +
                         std::to_string(new_count) + ") ===\n";

    std::string content = header;
    for (int i = current_lines - new_count; i < current_lines; ++i) {
        content += lines[i];
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (config_.sd_card.enabled && sd_available_)
            WriteSD(GetSDDateFile("dmesg"), content);
        if (config_.flash.enabled && flash_available_)
            WriteFlash(GetFlashFile("dmesg.log"), content);

        source_state_[CollectionSource::Kernel].last_collection = UnixTime();
        source_state_[CollectionSource::Kernel].entries_collected += new_count;
    }
}

void SystemLogger::CollectThermal() {
    std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
    if (!f) return;

    int raw_temp = 0;
    f >> raw_temp;
    f.close();

    std::string timestamp = Now();

    // Convert millidegrees to degrees
    float temp_c = (raw_temp > 1000) ? raw_temp / 1000.0f : static_cast<float>(raw_temp);
    char entry[128];
    std::snprintf(entry, sizeof(entry), "[%s] Temperature: %.1fC (raw: %d)\n",
                  timestamp.c_str(), temp_c, raw_temp);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (config_.sd_card.enabled && sd_available_)
            WriteSD(GetSDDateFile("thermal"), entry);
        if (config_.flash.enabled && flash_available_)
            WriteFlash(GetFlashFile("thermal.log"), entry);

        source_state_[CollectionSource::Thermal].last_collection = UnixTime();
        source_state_[CollectionSource::Thermal].entries_collected++;
    }
}

void SystemLogger::CollectSysDiag() {
    std::string ts = Now();
    std::ostringstream diag;
    diag << "[" << ts << "] === System Diagnostics ===\n";

    // Uptime
    struct sysinfo si{};
    if (sysinfo(&si) == 0) {
        int days  = si.uptime / 86400;
        int hours = (si.uptime % 86400) / 3600;
        int mins  = (si.uptime % 3600) / 60;
        diag << "Uptime: " << days << "d " << hours << "h " << mins << "m\n";
        diag << "Load: " << si.loads[0] / 65536.0f << " "
             << si.loads[1] / 65536.0f << " "
             << si.loads[2] / 65536.0f << "\n";
        diag << "Procs: " << si.procs << "\n";

        int64_t total_mb = (si.totalram * si.mem_unit) / 1048576;
        int64_t free_mb  = (si.freeram * si.mem_unit) / 1048576;
        int64_t used_mb  = total_mb - free_mb;
        int pct = (total_mb > 0) ? static_cast<int>(used_mb * 100 / total_mb) : 0;
        diag << "Memory: " << pct << "% used (" << used_mb << "MB / " << total_mb << "MB)\n";
    }

    // CPU from /proc/stat
    {
        std::ifstream f("/proc/stat");
        if (f) {
            std::string line;
            std::getline(f, line);
            if (line.substr(0, 3) == "cpu") {
                unsigned long user, nice, sys, idle, iow;
                if (sscanf(line.c_str(), "cpu %lu %lu %lu %lu %lu",
                           &user, &nice, &sys, &idle, &iow) == 5) {
                    unsigned long total = user + nice + sys + idle + iow;
                    unsigned long used = total - idle;
                    int pct = (total > 0) ? static_cast<int>(used * 100 / total) : 0;
                    diag << "CPU: " << pct << "% (user:" << user
                         << " sys:" << sys << " idle:" << idle
                         << " iowait:" << iow << ")\n";
                }
            }
        }
    }

    // Network from /proc/net/dev
    {
        std::ifstream f("/proc/net/dev");
        if (f) {
            std::string line;
            std::getline(f, line); // header 1
            std::getline(f, line); // header 2
            while (std::getline(f, line)) {
                if (line.find("eth0") != std::string::npos ||
                    line.find("wlan0") != std::string::npos ||
                    line.find("usb0") != std::string::npos) {
                    // Parse interface:rx_bytes ... tx_bytes
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string iface = line.substr(0, colon);
                        // Trim whitespace
                        iface.erase(0, iface.find_first_not_of(" \t"));
                        unsigned long rx = 0, tx = 0;
                        if (sscanf(line.c_str() + colon + 1,
                                   "%lu %*u %*u %*u %*u %*u %*u %*u %lu",
                                   &rx, &tx) == 2) {
                            diag << "  " << iface << ": RX " << (rx / 1048576)
                                 << "MB TX " << (tx / 1048576) << "MB\n";
                        }
                    }
                }
            }
        }
    }

    // Disk usage
    diag << "Storage:\n";
    for (const char* mp : {"/", "/mnt/sd", "/mnt/app", "/tmp"}) {
        struct statvfs st{};
        if (statvfs(mp, &st) == 0 && st.f_blocks > 0) {
            int64_t total = (int64_t)st.f_blocks * st.f_frsize / 1048576;
            int64_t free  = (int64_t)st.f_bfree  * st.f_frsize / 1048576;
            int64_t used  = total - free;
            int pct = (total > 0) ? static_cast<int>(used * 100 / total) : 0;
            diag << "  " << mp << ": " << pct << "% used ("
                 << used << "MB / " << total << "MB)\n";
        }
    }

    // Temperature
    {
        std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
        if (f) {
            int raw = 0;
            f >> raw;
            float t = (raw > 1000) ? raw / 1000.0f : (float)raw;
            diag << "Temperature: " << t << "C\n";
        }
    }

    diag << "=== End Diagnostics ===\n\n";

    std::string content = diag.str();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (config_.sd_card.enabled && sd_available_)
            WriteSD(GetSDDateFile("sysdiag"), content);
        if (config_.flash.enabled && flash_available_)
            WriteFlash(GetFlashFile("sysdiag.log"), content);

        source_state_[CollectionSource::SysDiag].last_collection = UnixTime();
        source_state_[CollectionSource::SysDiag].entries_collected++;
    }
}

void SystemLogger::MirrorAppLog() {
    std::string app_log = config::Get<std::string>("logging.file.path", "/var/log/ipcamd.log");
    std::ifstream src(app_log);
    if (!src) return;

    // Read entire file
    std::string content((std::istreambuf_iterator<char>(src)),
                         std::istreambuf_iterator<char>());
    src.close();

    if (content.empty()) return;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        // SD: append with timestamp header
        if (config_.sd_card.enabled && sd_available_) {
            std::string header = "[" + Now() + "] === ipcamd.log snapshot ===\n";
            WriteSD(GetSDDateFile("ipcamd"), header + content + "\n");
        }
        // Flash: overwrite (rotating) — just keep a copy
        if (config_.flash.enabled && flash_available_) {
            std::string path = GetFlashFile("ipcamd.log");
            std::ofstream f(path, std::ios::trunc);
            if (f) {
                f << content;
                RotateFlashLog(path, config_.flash.max_file_kb);
            }
        }

        source_state_[CollectionSource::AppMirror].last_collection = UnixTime();
        source_state_[CollectionSource::AppMirror].entries_collected++;
    }
}

// ============================================================================
// Destination Management — SD Card
// ============================================================================

bool SystemLogger::IsSDMountedRW() const {
    std::ifstream f("/proc/mounts");
    if (!f) return false;
    std::string line;
    while (std::getline(f, line)) {
        if (line.find(" /mnt/sd ") != std::string::npos) {
            // Options field is 4th column
            // Check if starts with "rw"
            size_t sp3 = 0;
            int spaces = 0;
            for (size_t i = 0; i < line.size() && spaces < 3; ++i) {
                if (line[i] == ' ') { ++spaces; sp3 = i + 1; }
            }
            if (spaces >= 3) {
                return (line.substr(sp3, 2) == "rw");
            }
            return false;
        }
    }
    return false;
}

bool SystemLogger::TryRemountSDRW() {
    if (IsSDMountedRW()) return true;

    spdlog::warn("SystemLogger: SD card is read-only, attempting remount rw...");
    int ret = ::system("mount -o remount,rw /mnt/sd 2>/dev/null");
    if (ret == 0 && IsSDMountedRW()) {
        spdlog::info("SystemLogger: SD card remounted read-write");
        return true;
    }
    spdlog::warn("SystemLogger: failed to remount SD card rw");
    return false;
}

bool SystemLogger::IsSDAvailable() {
    // Check if mounted at all
    {
        std::ifstream f("/proc/mounts");
        if (!f) return false;
        std::string line;
        bool found = false;
        while (std::getline(f, line)) {
            if (line.find(" /mnt/sd ") != std::string::npos) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    // Check if read-write (try remount if needed)
    if (!IsSDMountedRW()) {
        if (!TryRemountSDRW()) return false;
    }

    // Check free space
    int64_t free_mb = GetFreeSpaceKB("/mnt/sd") / 1024;
    if (free_mb < config_.sd_card.min_free_mb) {
        spdlog::warn("SystemLogger: SD card low on space ({}MB free, need {}MB)",
                     free_mb, config_.sd_card.min_free_mb);
        return false;
    }

    return true;
}

bool SystemLogger::IsFlashAvailable() {
    struct stat st{};
    if (stat("/mnt/app", &st) != 0) return false;

    int64_t free_kb = GetFreeSpaceKB("/mnt/app");
    if (free_kb < config_.flash.min_free_kb) {
        spdlog::warn("SystemLogger: flash low on space ({}KB free, need {}KB)",
                     free_kb, config_.flash.min_free_kb);
        return false;
    }
    return true;
}

void SystemLogger::EnsureSDSpace() {
    if (!sd_available_) return;

    // Clean up old logs by date
    CleanupOldSDLogs();

    // Check total size
    int64_t total_kb = GetDirSizeKB(config_.sd_card.path);
    int64_t max_kb = config_.sd_card.max_total_mb * 1024;

    if (total_kb > max_kb) {
        spdlog::info("SystemLogger: SD logs exceed {}MB, cleaning oldest...",
                     config_.sd_card.max_total_mb);

        // Collect all .log files with modification times
        std::vector<std::pair<std::string, time_t>> files;
        DIR* dir = opendir(config_.sd_card.path.c_str());
        if (!dir) return;

        struct dirent* entry;
        while ((entry = readdir(dir))) {
            std::string name = entry->d_name;
            if (name.size() > 4 && name.substr(name.size() - 4) == ".log") {
                std::string full = config_.sd_card.path + "/" + name;
                struct stat st{};
                if (stat(full.c_str(), &st) == 0) {
                    files.emplace_back(full, st.st_mtime);
                }
            }
        }
        closedir(dir);

        // Sort oldest first
        std::sort(files.begin(), files.end(),
                  [](auto& a, auto& b) { return a.second < b.second; });

        // Delete oldest until under limit
        for (auto& [path, mtime] : files) {
            if (total_kb <= max_kb) break;
            struct stat st{};
            if (stat(path.c_str(), &st) == 0) {
                total_kb -= st.st_size / 1024;
            }
            unlink(path.c_str());
            spdlog::debug("SystemLogger: deleted old SD log {}", path);
        }
    }
}

void SystemLogger::CleanupOldSDLogs() {
    int max_days = config_.sd_card.max_days;
    if (max_days <= 0) return;

    time_t cutoff = time(nullptr) - (max_days * 86400);

    DIR* dir = opendir(config_.sd_card.path.c_str());
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        std::string name = entry->d_name;
        if (name.size() > 4 && name.substr(name.size() - 4) == ".log") {
            std::string full = config_.sd_card.path + "/" + name;
            struct stat st{};
            if (stat(full.c_str(), &st) == 0 && st.st_mtime < cutoff) {
                unlink(full.c_str());
                spdlog::debug("SystemLogger: cleaned old log {} (>{} days)", name, max_days);
            }
        }
    }
    closedir(dir);
}

void SystemLogger::EnsureFlashSpace() {
    if (!flash_available_) return;

    int64_t total_kb = GetDirSizeKB(config_.flash.path);
    if (total_kb > config_.flash.max_total_kb) {
        spdlog::info("SystemLogger: flash logs exceed {}KB, truncating...",
                     config_.flash.max_total_kb);

        // Truncate each log file to keep last 25%
        DIR* dir = opendir(config_.flash.path.c_str());
        if (!dir) return;

        struct dirent* entry;
        while ((entry = readdir(dir))) {
            std::string name = entry->d_name;
            if (name.size() > 4 && name.substr(name.size() - 4) == ".log") {
                std::string full = config_.flash.path + "/" + name;
                RotateFlashLog(full, config_.flash.max_file_kb);
            }
        }
        closedir(dir);
    }
}

void SystemLogger::RotateFlashLog(const std::string& path, int64_t max_kb) {
    struct stat st{};
    if (stat(path.c_str(), &st) != 0) return;
    if (st.st_size / 1024 < max_kb) return;

    // Read file, keep last half
    std::ifstream f(path);
    if (!f) return;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line)) {
        lines.push_back(line);
    }
    f.close();

    int keep = static_cast<int>(lines.size()) / 2;
    if (keep < 100) keep = 100;
    if (keep > static_cast<int>(lines.size())) keep = lines.size();

    std::ofstream out(path, std::ios::trunc);
    if (!out) return;
    for (int i = static_cast<int>(lines.size()) - keep; i < static_cast<int>(lines.size()); ++i) {
        out << lines[i] << "\n";
    }
    spdlog::debug("SystemLogger: rotated {} (kept last {} lines)", path, keep);
}

// ============================================================================
// Safe File I/O
// ============================================================================

bool SystemLogger::WriteSD(const std::string& file, const std::string& content, bool append) {
    if (!sd_available_) return false;

    // Verify mount is still rw (zero-cost /proc/mounts check)
    if (!IsSDMountedRW()) {
        spdlog::warn("SystemLogger: SD went read-only during write");
        sd_available_ = false;
        return false;
    }

    // Quick space check
    int64_t free_kb = GetFreeSpaceKB("/mnt/sd");
    if (free_kb / 1024 < config_.sd_card.min_free_mb) {
        sd_available_ = false;
        return false;
    }

    auto mode = append ? (std::ios::app) : (std::ios::trunc);
    std::ofstream f(file, mode);
    if (!f) return false;
    f << content;
    return f.good();
}

bool SystemLogger::WriteFlash(const std::string& file, const std::string& content, bool append) {
    if (!flash_available_) return false;

    int64_t free_kb = GetFreeSpaceKB("/mnt/app");
    if (free_kb < config_.flash.min_free_kb) {
        flash_available_ = false;
        return false;
    }

    auto mode = append ? (std::ios::app) : (std::ios::trunc);
    std::ofstream f(file, mode);
    if (!f) return false;
    f << content;

    // Rotate after write
    RotateFlashLog(file, config_.flash.max_file_kb);
    return true;
}

std::string SystemLogger::GetSDDateFile(const std::string& prefix) const {
    return config_.sd_card.path + "/" + prefix + "_" + DateStamp() + ".log";
}

std::string SystemLogger::GetFlashFile(const std::string& name) const {
    return config_.flash.path + "/" + name;
}

// ============================================================================
// Free Space Helpers
// ============================================================================

int64_t SystemLogger::GetFreeSpaceKB(const std::string& mount_point) const {
    struct statvfs st{};
    if (statvfs(mount_point.c_str(), &st) != 0) return 0;
    return static_cast<int64_t>(st.f_bavail) * st.f_frsize / 1024;
}

int64_t SystemLogger::GetDirSizeKB(const std::string& dir) const {
    int64_t total = 0;
    DIR* d = opendir(dir.c_str());
    if (!d) return 0;

    struct dirent* entry;
    while ((entry = readdir(d))) {
        if (entry->d_name[0] == '.') continue;
        std::string full = dir + "/" + entry->d_name;
        struct stat st{};
        if (stat(full.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            total += st.st_size;
        }
    }
    closedir(d);
    return total / 1024;
}

}  // namespace platform
}  // namespace ipcam
