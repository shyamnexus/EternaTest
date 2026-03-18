#include <ipcam/upnp_manager.h>
#include <spdlog/spdlog.h>
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <cstring>

namespace ipcam {
namespace networking {

UpnpManager::UpnpManager() : urls_(nullptr), igd_data_(nullptr) {
    spdlog::info("[UpnpManager] Initializing UPnP manager");
}

UpnpManager::~UpnpManager() {
    Cleanup();
}

Result<UpnpManager::UpnpConfig> UpnpManager::GetConfig() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    UpnpConfig cfg;
    cfg.enabled = config::Get<bool>("upnp.enabled", false);
    cfg.timeout_ms = config::Get<int>("upnp.timeout_ms", 2000);
    cfg.auto_map = config::Get<bool>("upnp.auto_map", false);
    
    return Result<UpnpConfig>::Ok(cfg);
}

Result<void> UpnpManager::SetConfig(const UpnpConfig& cfg) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    config::Set<bool>("upnp.enabled", cfg.enabled);
    config::Set<int>("upnp.timeout_ms", cfg.timeout_ms);
    config::Set<bool>("upnp.auto_map", cfg.auto_map);
    
    if (!config::Save()) {
        return Result<void>::Err("Failed to save UPnP config");
    }
    
    spdlog::info("[UpnpManager] Configuration saved successfully");
    return Result<void>::Ok();
}

Result<UpnpManager::UpnpDeviceInfo> UpnpManager::Discover(int timeout_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Cleanup(); // Clean up any previous session
    
    UPNPUrls* urls = new UPNPUrls();
    IGDdatas* igd_data = new IGDdatas();
    
    std::memset(urls, 0, sizeof(UPNPUrls));
    std::memset(igd_data, 0, sizeof(IGDdatas));
    
    char lan_addr[64] = {0};
    int error = 0;
    
    UPNPDev* devlist = upnpDiscover(timeout_ms, nullptr, nullptr, 0, 0, 2, &error);
    
    if (!devlist) {
        delete urls;
        delete igd_data;
        return Result<UpnpDeviceInfo>::Err("No UPnP devices found");
    }
    
    char wan_addr[64] = {0};
    int ret = UPNP_GetValidIGD(devlist, urls, igd_data, lan_addr, sizeof(lan_addr), 
                               wan_addr, sizeof(wan_addr));
    freeUPNPDevlist(devlist);
    
    if (ret == 0) {
        FreeUPNPUrls(urls);
        delete urls;
        delete igd_data;
        return Result<UpnpDeviceInfo>::Err("No valid IGD found");
    }
    
    // Get external IP
    char external_ip[40] = {0};
    int ip_ret = UPNP_GetExternalIPAddress(urls->controlURL, 
                                           igd_data->first.servicetype, 
                                           external_ip);
    
    UpnpDeviceInfo info;
    info.is_connected = true;
    info.device_name = igd_data->presentationurl[0] ? igd_data->presentationurl : "IGD Device";
    info.gateway_ip = lan_addr;
    info.external_ip = (ip_ret == 0 && external_ip[0]) ? external_ip : "";
    
    // Store for later use
    urls_ = urls;
    igd_data_ = igd_data;
    device_info_ = info;
    
    spdlog::info("[UpnpManager] Discovered UPnP device: {} ({})", 
                 info.device_name, info.external_ip);
    
    return Result<UpnpDeviceInfo>::Ok(info);
}

Result<void> UpnpManager::AddPortMapping(const UpnpPortMapping& mapping) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!urls_ || !igd_data_) {
        return Result<void>::Err("UPnP not initialized - call Discover() first");
    }
    
    UPNPUrls* urls = static_cast<UPNPUrls*>(urls_);
    IGDdatas* igd_data = static_cast<IGDdatas*>(igd_data_);
    
    int ret = UPNP_AddPortMapping(
        urls->controlURL,
        igd_data->first.servicetype,
        std::to_string(mapping.external_port).c_str(),
        std::to_string(mapping.internal_port).c_str(),
        mapping.internal_client.c_str(),
        mapping.description.c_str(),
        mapping.protocol.c_str(),
        nullptr, // remote host
        std::to_string(mapping.lease_duration).c_str()
    );
    
    if (ret != UPNPCOMMAND_SUCCESS) {
        return Result<void>::Err("Failed to add port mapping: error " + std::to_string(ret));
    }
    
    spdlog::info("[UpnpManager] Added port mapping: {}:{} -> {}:{} ({})",
                 mapping.external_port, mapping.protocol,
                 mapping.internal_port, mapping.protocol,
                 mapping.description);
    
    return Result<void>::Ok();
}

Result<void> UpnpManager::DeletePortMapping(int external_port, const std::string& protocol) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!urls_ || !igd_data_) {
        return Result<void>::Err("UPnP not initialized - call Discover() first");
    }
    
    UPNPUrls* urls = static_cast<UPNPUrls*>(urls_);
    IGDdatas* igd_data = static_cast<IGDdatas*>(igd_data_);
    
    int ret = UPNP_DeletePortMapping(
        urls->controlURL,
        igd_data->first.servicetype,
        std::to_string(external_port).c_str(),
        protocol.c_str(),
        nullptr // remote host
    );
    
    if (ret != UPNPCOMMAND_SUCCESS) {
        return Result<void>::Err("Failed to delete port mapping: error " + std::to_string(ret));
    }
    
    spdlog::info("[UpnpManager] Deleted port mapping: {}:{}", external_port, protocol);
    
    return Result<void>::Ok();
}

Result<std::vector<UpnpManager::UpnpPortMapping>> UpnpManager::GetPortMappings() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!urls_ || !igd_data_) {
        return Result<std::vector<UpnpPortMapping>>::Err("UPnP not initialized - call Discover() first");
    }
    
    UPNPUrls* urls = static_cast<UPNPUrls*>(urls_);
    IGDdatas* igd_data = static_cast<IGDdatas*>(igd_data_);
    
    std::vector<UpnpPortMapping> mappings;
    
    // Query port mappings from the IGD device
    // miniupnpc doesn't provide a direct "list all" function, so we iterate through indices
    int index = 0;
    char ext_port[16], int_client[64], int_port[16], protocol[16], desc[256];
    char enabled[16], remote_host[64], lease_duration[16];
    
    while (index < 256) { // Reasonable limit to avoid infinite loop
        int ret = UPNP_GetGenericPortMappingEntry(
            urls->controlURL,
            igd_data->first.servicetype,
            std::to_string(index).c_str(),
            ext_port,
            int_client,
            int_port,
            protocol,
            desc,
            enabled,
            remote_host,
            lease_duration
        );
        
        if (ret != UPNPCOMMAND_SUCCESS) {
            break; // No more entries
        }
        
        UpnpPortMapping mapping;
        mapping.external_port = std::atoi(ext_port);
        mapping.internal_port = std::atoi(int_port);
        mapping.internal_client = int_client;
        mapping.protocol = protocol;
        mapping.description = desc;
        mapping.lease_duration = std::atoi(lease_duration);
        
        mappings.push_back(mapping);
        index++;
    }
    
    spdlog::info("[UpnpManager] Retrieved {} port mappings", mappings.size());
    
    return Result<std::vector<UpnpPortMapping>>::Ok(mappings);
}

void UpnpManager::Cleanup() {
    if (urls_) {
        FreeUPNPUrls(static_cast<UPNPUrls*>(urls_));
        delete static_cast<UPNPUrls*>(urls_);
        urls_ = nullptr;
    }
    if (igd_data_) {
        delete static_cast<IGDdatas*>(igd_data_);
        igd_data_ = nullptr;
    }
}

} // namespace networking
} // namespace ipcam
