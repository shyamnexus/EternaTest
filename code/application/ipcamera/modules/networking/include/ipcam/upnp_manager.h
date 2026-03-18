#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <ipcam/config.h>
#include <ipcam/result.h>

namespace ipcam {
namespace networking {

/**
 * @brief UPnP Manager - Handles UPnP discovery and port mapping
 * 
 * This class manages UPnP device discovery and port mapping configuration.
 * No sensitive credentials needed for UPnP.
 */
class UpnpManager {
public:
    struct UpnpConfig {
        bool enabled;              // UPnP enabled/disabled
        int timeout_ms;            // Discovery timeout in milliseconds
        bool auto_map;             // Automatically map ports
    };

    struct UpnpDeviceInfo {
        bool is_connected;         // Device found and connected
        std::string device_name;   // IGD device name
        std::string gateway_ip;    // Gateway IP address
        std::string external_ip;   // External/public IP address
    };

    struct UpnpPortMapping {
        int external_port;         // External port number
        int internal_port;         // Internal port number
        std::string protocol;      // "TCP" or "UDP"
        std::string internal_client; // Internal client IP
        std::string description;   // Mapping description
        int lease_duration;        // Lease duration in seconds (0=permanent)
    };

    UpnpManager();
    ~UpnpManager();

    // Get current UPnP configuration
    Result<UpnpConfig> GetConfig();
    
    // Set UPnP configuration
    Result<void> SetConfig(const UpnpConfig& config);
    
    // Discover UPnP devices
    Result<UpnpDeviceInfo> Discover(int timeout_ms = 2000);
    
    // Port mapping management
    Result<void> AddPortMapping(const UpnpPortMapping& mapping);
    Result<void> DeletePortMapping(int external_port, const std::string& protocol);
    Result<std::vector<UpnpPortMapping>> GetPortMappings();
    
private:
    void Cleanup();
    
    void* urls_;       // UPNPUrls* (opaque pointer to avoid including miniupnpc headers)
    void* igd_data_;   // IGDdatas* (opaque pointer)
    UpnpDeviceInfo device_info_;
    std::mutex mutex_;
};

} // namespace networking
} // namespace ipcam
