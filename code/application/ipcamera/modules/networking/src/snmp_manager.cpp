#include <ipcam/snmp_manager.h>
#include <ipcam/storage.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>

#define SNMPD_CONF_PATH "/etc/snmp/snmpd.conf"
#define SNMPD_BINARY "/sbin/snmpd"

namespace ipcam {
namespace networking {

SnmpManager::SnmpManager() {
    spdlog::info("[SnmpManager] Initializing SNMP manager");
}

Result<SnmpManager::SnmpConfig> SnmpManager::GetConfig() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    SnmpConfig cfg;
    
    // Load non-sensitive from config system
    cfg.enabled = config::Get<bool>("snmp.enabled", false);
    cfg.port = config::Get<int>("snmp.port", 161);
    cfg.version = config::Get<int>("snmp.version", 2);
    cfg.sys_location = config::Get<std::string>("snmp.sys_location", "");
    cfg.sys_contact = config::Get<std::string>("snmp.sys_contact", "");
    cfg.sys_name = config::Get<std::string>("snmp.sys_name", "ipcamera");
    cfg.listen_address = config::Get<std::string>("snmp.listen_address", "0.0.0.0");
    
    // Load sensitive from CredentialManager
    auto& cred_mgr = storage::GetCredentialManager();
    if (!cred_mgr.Initialize()) {
        spdlog::error("[SnmpManager] Failed to initialize CredentialManager");
    } else {
        auto creds = cred_mgr.GetSnmpCredentials();
        if (creds) {
            cfg.community_ro = creds->community_ro;
            cfg.community_rw = creds->community_rw;
            cfg.v3_user = creds->v3_user;
            cfg.v3_auth_password = creds->v3_auth_password;
            cfg.v3_priv_password = creds->v3_priv_password;
            cfg.v3_auth_protocol = creds->v3_auth_protocol;
            cfg.v3_priv_protocol = creds->v3_priv_protocol;
        }
    }
    
    return Result<SnmpConfig>::Ok(cfg);
}

Result<void> SnmpManager::SetConfig(const SnmpConfig& cfg) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Store non-sensitive in config system
    config::Set<bool>("snmp.enabled", cfg.enabled);
    config::Set<int>("snmp.port", cfg.port);
    config::Set<int>("snmp.version", cfg.version);
    config::Set<std::string>("snmp.sys_location", cfg.sys_location);
    config::Set<std::string>("snmp.sys_contact", cfg.sys_contact);
    config::Set<std::string>("snmp.sys_name", cfg.sys_name);
    config::Set<std::string>("snmp.listen_address", cfg.listen_address);
    
    if (!config::Save()) {
        return Result<void>::Err("Failed to save SNMP config");
    }
    
    // Store sensitive in CredentialManager
    auto& cred_mgr = storage::GetCredentialManager();
    if (!cred_mgr.Initialize()) {
        return Result<void>::Err("Failed to initialize CredentialManager");
    }
    
    storage::CredentialManager::SnmpCredentials creds;
    creds.community_ro = cfg.community_ro;
    creds.community_rw = cfg.community_rw;
    creds.v3_user = cfg.v3_user;
    creds.v3_auth_password = cfg.v3_auth_password;
    creds.v3_priv_password = cfg.v3_priv_password;
    creds.v3_auth_protocol = cfg.v3_auth_protocol;
    creds.v3_priv_protocol = cfg.v3_priv_protocol;
    
    if (!cred_mgr.StoreSnmpCredentials(creds)) {
        return Result<void>::Err("Failed to store SNMP credentials");
    }
    
    // Generate snmpd.conf with new configuration
    auto result = GenerateSnmpdConf(cfg);
    if (!result.success) {
        return result;
    }
    
    // Restart daemon if enabled (without holding lock to avoid deadlock)
    bool should_restart = cfg.enabled;
    
    // Release lock before restart
    mutex_.unlock();
    
    if (should_restart) {
        auto restart_result = Restart();
        mutex_.lock(); // Re-acquire for RAII guard destructor
        return restart_result;
    }
    
    mutex_.lock(); // Re-acquire for RAII guard destructor
    spdlog::info("[SnmpManager] Configuration saved successfully");
    return Result<void>::Ok();
}

Result<void> SnmpManager::Start() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto running = IsRunning();
    if (running.success && running.value) {
        return Result<void>::Ok(); // Already running
    }
    
    std::string cmd = std::string(SNMPD_BINARY) + " -c " + SNMPD_CONF_PATH;
    int ret = system(cmd.c_str());
    if (ret != 0) {
        return Result<void>::Err("Failed to start SNMP daemon");
    }
    
    spdlog::info("[SnmpManager] SNMP daemon started");
    return Result<void>::Ok();
}

Result<void> SnmpManager::Stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int pid = GetDaemonPid();
    if (pid <= 0) {
        return Result<void>::Ok(); // Not running
    }
    
    kill(pid, SIGTERM);
    
    // Wait for shutdown (max 5 seconds)
    for (int i = 0; i < 50; i++) {
        if (GetDaemonPid() <= 0) {
            spdlog::info("[SnmpManager] SNMP daemon stopped");
            return Result<void>::Ok();
        }
        usleep(100000); // 100ms
    }
    
    // Force kill if needed
    kill(pid, SIGKILL);
    spdlog::warn("[SnmpManager] SNMP daemon force killed");
    
    return Result<void>::Ok();
}

Result<void> SnmpManager::Restart() {
    auto stop_result = Stop();
    if (!stop_result.success) {
        return stop_result;
    }
    
    usleep(500000); // Wait 500ms
    
    return Start();
}

Result<bool> SnmpManager::IsRunning() {
    int pid = GetDaemonPid();
    return Result<bool>::Ok(pid > 0);
}

Result<void> SnmpManager::GenerateSnmpdConf(const SnmpConfig& cfg) {
    std::ofstream f(SNMPD_CONF_PATH, std::ios::trunc);
    if (!f) {
        return Result<void>::Err("Failed to open snmpd.conf for writing");
    }
    
    // Basic settings
    f << "# Auto-generated SNMP configuration\n";
    f << "agentAddress udp:" << cfg.listen_address << ":" << cfg.port << "\n\n";
    
    // System information
    if (!cfg.sys_location.empty()) {
        f << "sysLocation " << cfg.sys_location << "\n";
    }
    if (!cfg.sys_contact.empty()) {
        f << "sysContact " << cfg.sys_contact << "\n";
    }
    if (!cfg.sys_name.empty()) {
        f << "sysName " << cfg.sys_name << "\n";
    }
    f << "\n";
    
    // Community strings (v1/v2c)
    if (cfg.version <= 2) {
        if (!cfg.community_ro.empty()) {
            f << "rocommunity " << cfg.community_ro << "\n";
        }
        if (!cfg.community_rw.empty()) {
            f << "rwcommunity " << cfg.community_rw << "\n";
        }
        f << "\n";
    }
    
    // SNMPv3 settings
    if (cfg.version >= 3 && !cfg.v3_user.empty()) {
        f << "# SNMPv3 configuration\n";
        f << "createUser " << cfg.v3_user;
        
        if (!cfg.v3_auth_protocol.empty() && !cfg.v3_auth_password.empty()) {
            f << " " << cfg.v3_auth_protocol << " " << cfg.v3_auth_password;
            
            if (!cfg.v3_priv_protocol.empty() && !cfg.v3_priv_password.empty()) {
                f << " " << cfg.v3_priv_protocol << " " << cfg.v3_priv_password;
            }
        }
        
        f << "\n";
        f << "rouser " << cfg.v3_user << "\n";
    }
    
    f.close();
    
    // Secure permissions
    chmod(SNMPD_CONF_PATH, 0600);
    
    spdlog::info("[SnmpManager] Generated snmpd.conf");
    return Result<void>::Ok();
}

int SnmpManager::GetDaemonPid() {
    FILE* fp = popen("pidof snmpd", "r");
    if (!fp) return -1;
    
    char buf[32];
    if (fgets(buf, sizeof(buf), fp) == nullptr) {
        pclose(fp);
        return -1;
    }
    
    pclose(fp);
    return atoi(buf);
}

} // namespace networking
} // namespace ipcam
