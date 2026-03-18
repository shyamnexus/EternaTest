#pragma once

#include <string>
#include <mutex>
#include <ipcam/config.h>
#include <ipcam/credential_manager.h>
#include <ipcam/result.h>

namespace ipcam {
namespace networking {

/**
 * @brief SNMP Manager - Handles SNMP configuration and daemon management
 * 
 * This class manages SNMP settings, integrating with CredentialManager for
 * secure credential storage (community strings, SNMPv3 passwords).
 */
class SnmpManager {
public:
    struct SnmpConfig {
        bool enabled;                  // SNMP enabled/disabled
        int port;                      // SNMP port (default 161)
        int version;                   // SNMP version (1, 2, or 3)
        std::string community_ro;      // Read-only community string
        std::string community_rw;      // Read-write community string
        std::string sys_location;      // System location
        std::string sys_contact;       // System contact
        std::string sys_name;          // System name
        std::string listen_address;    // Listen address
        // SNMPv3 settings
        std::string v3_user;           // SNMPv3 username
        std::string v3_auth_password;  // SNMPv3 auth password
        std::string v3_priv_password;  // SNMPv3 priv password
        std::string v3_auth_protocol;  // Auth protocol (MD5/SHA)
        std::string v3_priv_protocol;  // Priv protocol (DES/AES)
    };

    SnmpManager();
    ~SnmpManager() = default;

    // Get current SNMP configuration (with decrypted credentials)
    Result<SnmpConfig> GetConfig();
    
    // Set SNMP configuration (encrypts credentials)
    Result<void> SetConfig(const SnmpConfig& config);
    
    // SNMP daemon management
    Result<void> Start();
    Result<void> Stop();
    Result<void> Restart();
    
    // Get daemon status
    Result<bool> IsRunning();

private:
    Result<void> GenerateSnmpdConf(const SnmpConfig& cfg);
    int GetDaemonPid();
    
    std::mutex mutex_;
};

} // namespace networking
} // namespace ipcam
