/**
 * @file nas_manager.cpp
 * @brief NAS Manager Implementation
 */

#include "ipcam/nas_manager.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <array>
#include <sys/statvfs.h>
#include <sys/mount.h>

namespace ipcam {
namespace storage {

using json = nlohmann::json;

// ============================================================================
// Helper Functions
// ============================================================================

static std::string ExecCommand(const std::string& cmd, int& exit_code) {
    std::array<char, 128> buffer;
    std::string result;
    
    // Open pipe
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        exit_code = -1;
        return "Failed to execute command";
    }
    
    // Read output
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    // Close and get exit code
    exit_code = pclose(pipe);
    if (WIFEXITED(exit_code)) {
        exit_code = WEXITSTATUS(exit_code);
    }
    
    return result;
}

// ============================================================================
// JSON Serialization
// ============================================================================

static json NasConfigToJson(const NasConfig& config) {
    return json{
        {"enabled", config.enabled},
        {"type", NasProtocolToString(config.type)},
        {"server", config.server},
        {"share_path", config.share_path},
        {"mount_point", config.mount_point},
        {"username", config.username},
        {"password", config.password},
        {"domain", config.domain},
        {"auto_mount", config.auto_mount}
    };
}

static NasConfig JsonToNasConfig(const json& j) {
    NasConfig config;
    config.enabled = j.value("enabled", false);
    config.type = StringToNasProtocol(j.value("type", "smb"));
    config.server = j.value("server", "");
    config.share_path = j.value("share_path", "");
    config.mount_point = j.value("mount_point", "/mnt/nas");
    config.username = j.value("username", "");
    config.password = j.value("password", "");
    config.domain = j.value("domain", "");
    config.auto_mount = j.value("auto_mount", true);
    return config;
}

// ============================================================================
// NasManager Implementation
// ============================================================================

NasManager& NasManager::Instance() {
    static NasManager instance;
    return instance;
}

bool NasManager::Init() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("[NasManager] Already initialized");
        return true;
    }
    
    // Load config from the config system
    LoadConfig();
    
    // Auto-mount if enabled
    if (config_.enabled && config_.auto_mount) {
        spdlog::info("[NasManager] Auto-mounting NAS...");
        if (Mount()) {
            spdlog::info("[NasManager] Auto-mount successful");
        } else {
            spdlog::warn("[NasManager] Auto-mount failed: {}", config_.last_error);
        }
    }
    
    initialized_.store(true);
    spdlog::info("[NasManager] Initialized");
    return true;
}

void NasManager::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    // Unmount if mounted
    if (IsMountPointMounted(config_.mount_point)) {
        Unmount();
    }
    
    SaveConfig();
    initialized_.store(false);
    spdlog::info("[NasManager] Shutdown complete");
}

NasConfig NasManager::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool NasManager::SetConfig(const NasConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Unmount if changing config while mounted
    if (IsMountPointMounted(config_.mount_point)) {
        int exit_code;
        ExecCommand("umount " + config_.mount_point, exit_code);
    }
    
    config_ = config;
    config_.status = NasStatus::Disconnected;
    
    if (SaveConfig()) {
        spdlog::info("[NasManager] Configuration updated");
        return true;
    }
    return false;
}

NasStatus NasManager::GetStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if mounted
    if (IsMountPointMounted(config_.mount_point)) {
        return NasStatus::Mounted;
    }
    return config_.status;
}

bool NasManager::IsMounted() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return IsMountPointMounted(config_.mount_point);
}

bool NasManager::TestConnection(const NasConfig* config) {
    const NasConfig& cfg = config ? *config : config_;
    
    if (cfg.server.empty()) {
        config_.last_error = "Server address is empty";
        config_.status = NasStatus::Error;
        return false;
    }
    
    int exit_code;
    std::string result;
    
    switch (cfg.type) {
        case NasProtocol::SMB:
        case NasProtocol::CIFS: {
            // Test SMB connection using smbclient
            std::string cmd = "smbclient -L //" + cfg.server + " -N 2>&1";
            if (!cfg.username.empty()) {
                cmd = "smbclient -L //" + cfg.server + " -U " + cfg.username + "%" + cfg.password + " 2>&1";
            }
            result = ExecCommand(cmd, exit_code);
            break;
        }
        case NasProtocol::NFS: {
            // Test NFS by checking if server responds
            std::string cmd = "showmount -e " + cfg.server + " 2>&1";
            result = ExecCommand(cmd, exit_code);
            break;
        }
    }
    
    if (exit_code == 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.status = NasStatus::Connected;
        config_.last_error = "";
        spdlog::info("[NasManager] Connection test successful to {}", cfg.server);
        return true;
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.status = NasStatus::Error;
        config_.last_error = result;
        spdlog::error("[NasManager] Connection test failed: {}", result);
        return false;
    }
}

bool NasManager::Mount() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!config_.enabled) {
        config_.last_error = "NAS is not enabled";
        return false;
    }
    
    if (config_.server.empty() || config_.share_path.empty()) {
        config_.last_error = "Server or share path is empty";
        return false;
    }
    
    // Ensure mount point exists
    if (!EnsureMountPoint(config_.mount_point)) {
        config_.last_error = "Failed to create mount point";
        return false;
    }
    
    // Check if already mounted
    if (IsMountPointMounted(config_.mount_point)) {
        config_.status = NasStatus::Mounted;
        spdlog::info("[NasManager] Already mounted at {}", config_.mount_point);
        return true;
    }
    
    // Build and execute mount command
    std::string cmd = BuildMountCommand(config_);
    
    int exit_code;
    std::string result = ExecCommand(cmd, exit_code);
    
    if (exit_code == 0) {
        config_.status = NasStatus::Mounted;
        config_.last_error = "";
        spdlog::info("[NasManager] Mounted {} at {}", config_.server, config_.mount_point);
        return true;
    } else {
        config_.status = NasStatus::Error;
        config_.last_error = result;
        spdlog::error("[NasManager] Mount failed: {}", result);
        return false;
    }
}

bool NasManager::Unmount() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsMountPointMounted(config_.mount_point)) {
        config_.status = NasStatus::Disconnected;
        return true;
    }
    
    int exit_code;
    std::string result = ExecCommand("umount " + config_.mount_point + " 2>&1", exit_code);
    
    if (exit_code == 0) {
        config_.status = NasStatus::Disconnected;
        spdlog::info("[NasManager] Unmounted {}", config_.mount_point);
        return true;
    } else {
        // Try force unmount
        result = ExecCommand("umount -f " + config_.mount_point + " 2>&1", exit_code);
        if (exit_code == 0) {
            config_.status = NasStatus::Disconnected;
            spdlog::info("[NasManager] Force unmounted {}", config_.mount_point);
            return true;
        }
        
        config_.last_error = result;
        spdlog::error("[NasManager] Unmount failed: {}", result);
        return false;
    }
}

bool NasManager::GetStorageInfo(int64_t& total_bytes, int64_t& free_bytes) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsMountPointMounted(config_.mount_point)) {
        return false;
    }
    
    struct statvfs st;
    if (statvfs(config_.mount_point.c_str(), &st) != 0) {
        return false;
    }
    
    total_bytes = static_cast<int64_t>(st.f_blocks) * st.f_frsize;
    free_bytes = static_cast<int64_t>(st.f_bavail) * st.f_frsize;
    return true;
}

std::string NasManager::GetLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.last_error;
}

bool NasManager::SaveConfig() {
    // Save to unified config system
    ipcam::config::Set("recording.nas.enabled", config_.enabled);
    ipcam::config::Set("recording.nas.protocol", NasProtocolToString(config_.type));
    ipcam::config::Set("recording.nas.server", config_.server);
    ipcam::config::Set("recording.nas.share", config_.share_path);
    ipcam::config::Set("recording.nas.mount_point", config_.mount_point);
    ipcam::config::Set("recording.nas.username", config_.username);
    ipcam::config::Set("recording.nas.password", config_.password);
    ipcam::config::Set("recording.nas.domain", config_.domain);
    ipcam::config::Set("recording.nas.auto_mount", config_.auto_mount);
    
    if (ipcam::config::Save()) {
        spdlog::debug("[NasManager] Saved config to config system");
        return true;
    }
    spdlog::error("[NasManager] Failed to save config to config system");
    return false;
}

bool NasManager::LoadConfig() {
    // Load from unified config system
    config_.enabled = ipcam::config::Get<bool>("recording.nas.enabled", false);
    
    std::string protocol = ipcam::config::Get<std::string>("recording.nas.protocol", "smb");
    config_.type = StringToNasProtocol(protocol);
    
    config_.server = ipcam::config::Get<std::string>("recording.nas.server", "");
    config_.share_path = ipcam::config::Get<std::string>("recording.nas.share", "");
    config_.mount_point = ipcam::config::Get<std::string>("recording.nas.mount_point", "/mnt/nas");
    config_.username = ipcam::config::Get<std::string>("recording.nas.username", "");
    config_.password = ipcam::config::Get<std::string>("recording.nas.password", "");
    config_.domain = ipcam::config::Get<std::string>("recording.nas.domain", "");
    config_.auto_mount = ipcam::config::Get<bool>("recording.nas.auto_mount", true);
    
    spdlog::info("[NasManager] Loaded config from config system");
    return true;
}

std::string NasManager::BuildMountCommand(const NasConfig& config) const {
    std::stringstream cmd;
    
    switch (config.type) {
        case NasProtocol::SMB:
        case NasProtocol::CIFS: {
            // mount -t cifs //server/share /mnt/nas -o username=user,password=pass
            cmd << "mount -t cifs //" << config.server << "/" << config.share_path;
            cmd << " " << config.mount_point;
            cmd << " -o ";
            
            if (!config.username.empty()) {
                cmd << "username=" << config.username;
                if (!config.password.empty()) {
                    cmd << ",password=" << config.password;
                }
                if (!config.domain.empty()) {
                    cmd << ",domain=" << config.domain;
                }
            } else {
                cmd << "guest";
            }
            cmd << ",vers=3.0,uid=0,gid=0,file_mode=0755,dir_mode=0755 2>&1";
            break;
        }
        case NasProtocol::NFS: {
            // mount -t nfs server:/share /mnt/nas
            cmd << "mount -t nfs " << config.server << ":" << config.share_path;
            cmd << " " << config.mount_point;
            cmd << " -o nolock,tcp 2>&1";
            break;
        }
    }
    
    return cmd.str();
}

bool NasManager::IsMountPointMounted(const std::string& mount_point) const {
    std::ifstream mounts("/proc/mounts");
    std::string line;
    
    while (std::getline(mounts, line)) {
        if (line.find(mount_point) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool NasManager::EnsureMountPoint(const std::string& mount_point) {
    try {
        if (!std::filesystem::exists(mount_point)) {
            std::filesystem::create_directories(mount_point);
        }
        return true;
    } catch (const std::exception& e) {
        spdlog::error("[NasManager] Failed to create mount point: {}", e.what());
        return false;
    }
}

} // namespace storage
} // namespace ipcam
