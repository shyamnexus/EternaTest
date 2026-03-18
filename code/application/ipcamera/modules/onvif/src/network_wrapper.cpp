/**
 * @file network_wrapper.cpp
 * @brief C wrapper implementation for Network Manager functions
 * 
 * Implements the C wrapper functions by calling into the C++ NetworkManager class.
 * Note: Uses ipcam::Result<T> which has .success, .value, and .error members.
 */

#include "ipcam/network_wrapper.h"
#include "ipcam/network_manager.h"
#include <spdlog/spdlog.h>
#include <cstring>

using namespace ipcam::networking;

extern "C" {

int onvif_get_ipv4_config(const char *iface, OnvifIpv4Config *config) {
    if (!config) return -1;
    
    std::string interface = iface ? iface : "eth0";
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.GetIpv4(interface);
    
    if (!result.success) {
        spdlog::warn("Failed to get IPv4 config: {}", result.error);
        // Return defaults
        strncpy(config->method, "dhcp", sizeof(config->method));
        strncpy(config->address, "0.0.0.0", sizeof(config->address));
        strncpy(config->netmask, "255.255.255.0", sizeof(config->netmask));
        strncpy(config->gateway, "0.0.0.0", sizeof(config->gateway));
        return 0;
    }
    
    auto& ipv4 = result.value;
    strncpy(config->method, ipv4.method.c_str(), sizeof(config->method) - 1);
    strncpy(config->address, ipv4.address.c_str(), sizeof(config->address) - 1);
    strncpy(config->netmask, ipv4.netmask.c_str(), sizeof(config->netmask) - 1);
    strncpy(config->gateway, ipv4.gateway.c_str(), sizeof(config->gateway) - 1);
    
    return 0;
}

int onvif_set_ipv4_config(const char *iface, const OnvifIpv4Config *config) {
    if (!config) return -1;
    
    std::string interface = iface ? iface : "eth0";
    
    NetworkManager::Ipv4Config ipv4;
    ipv4.method = config->method;
    ipv4.address = config->address;
    ipv4.netmask = config->netmask;
    ipv4.gateway = config->gateway;
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.SetIpv4(interface, ipv4);
    
    if (!result.success) {
        spdlog::error("Failed to set IPv4 config: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_get_dns_config(OnvifDnsConfig *config) {
    if (!config) return -1;
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.GetDns();
    
    if (!result.success) {
        spdlog::warn("Failed to get DNS config: {}", result.error);
        memset(config, 0, sizeof(OnvifDnsConfig));
        return 0;
    }
    
    auto& dns = result.value;
    strncpy(config->primary, dns.primary.c_str(), sizeof(config->primary) - 1);
    strncpy(config->secondary, dns.secondary.c_str(), sizeof(config->secondary) - 1);
    strncpy(config->primary_ipv6, dns.primary_ipv6.c_str(), sizeof(config->primary_ipv6) - 1);
    strncpy(config->secondary_ipv6, dns.secondary_ipv6.c_str(), sizeof(config->secondary_ipv6) - 1);
    
    return 0;
}

int onvif_set_dns_config(const OnvifDnsConfig *config) {
    if (!config) return -1;
    
    NetworkManager::DnsConfig dns;
    dns.primary = config->primary;
    dns.secondary = config->secondary;
    dns.primary_ipv6 = config->primary_ipv6;
    dns.secondary_ipv6 = config->secondary_ipv6;
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.SetDns(dns);
    
    if (!result.success) {
        spdlog::error("Failed to set DNS config: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_get_ntp_config(OnvifNtpConfig *config) {
    if (!config) return -1;
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.GetNtpConfig();
    
    if (!result.success) {
        spdlog::warn("Failed to get NTP config: {}", result.error);
        config->enabled = false;
        strncpy(config->server, "pool.ntp.org", sizeof(config->server));
        config->port = 123;
        config->refresh_interval = 3600;
        return 0;
    }
    
    auto& ntp = result.value;
    config->enabled = ntp.enabled;
    strncpy(config->server, ntp.server.c_str(), sizeof(config->server) - 1);
    config->port = ntp.port;
    config->refresh_interval = ntp.refresh_interval_sec;
    
    return 0;
}

int onvif_set_ntp_config(const OnvifNtpConfig *config) {
    if (!config) return -1;
    
    NetworkManager::NtpConfig ntp;
    ntp.enabled = config->enabled;
    ntp.server = config->server;
    ntp.port = config->port;
    ntp.refresh_interval_sec = config->refresh_interval;
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.SetNtpConfig(ntp);
    
    if (!result.success) {
        spdlog::error("Failed to set NTP config: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_sync_ntp_time(void) {
    auto& nm = NetworkManager::Instance();
    auto result = nm.SyncTimeNtp();
    
    if (!result.success) {
        spdlog::error("Failed to sync NTP time: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_get_hostname_config(OnvifHostnameConfig *config) {
    if (!config) return -1;
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.GetHostnameConfig();
    
    if (!result.success) {
        spdlog::warn("Failed to get hostname config: {}", result.error);
        strncpy(config->method, "manual", sizeof(config->method));
        strncpy(config->name, "ipcamera", sizeof(config->name));
        return 0;
    }
    
    auto& hostname = result.value;
    strncpy(config->method, hostname.method.c_str(), sizeof(config->method) - 1);
    strncpy(config->name, hostname.name.c_str(), sizeof(config->name) - 1);
    
    return 0;
}

int onvif_set_hostname(const char *hostname) {
    if (!hostname) return -1;
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.SetHostname(hostname);
    
    if (!result.success) {
        spdlog::error("Failed to set hostname: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_get_mac_address(const char *iface, char *mac_address) {
    if (!mac_address) return -1;
    
    std::string interface = iface ? iface : "eth0";
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.GetMac(interface);
    
    if (!result.success) {
        spdlog::warn("Failed to get MAC address: {}", result.error);
        strncpy(mac_address, "00:00:00:00:00:00", 18);
        return 0;
    }
    
    strncpy(mac_address, result.value.c_str(), 17);
    mac_address[17] = '\0';
    
    return 0;
}

int onvif_get_network_protocols(OnvifNetworkProtocols *protocols) {
    if (!protocols) return -1;
    
    auto& nm = NetworkManager::Instance();
    
    auto http_result = nm.GetHttpPort();
    protocols->http_port = http_result.success ? http_result.value : 80;
    
    auto https_result = nm.GetHttpsPort();
    protocols->https_port = https_result.success ? https_result.value : 443;
    
    auto rtsp_result = nm.GetRtspPort();
    protocols->rtsp_port = rtsp_result.success ? rtsp_result.value : 554;
    
    auto onvif_result = nm.GetOnvifPort();
    protocols->onvif_port = onvif_result.success ? onvif_result.value : 80;
    
    return 0;
}

int onvif_set_http_port(int port) {
    auto& nm = NetworkManager::Instance();
    auto result = nm.SetHttpPort(port);
    
    if (!result.success) {
        spdlog::error("Failed to set HTTP port: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_set_https_port(int port) {
    auto& nm = NetworkManager::Instance();
    auto result = nm.SetHttpsPort(port);
    
    if (!result.success) {
        spdlog::error("Failed to set HTTPS port: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_set_rtsp_port(int port) {
    auto& nm = NetworkManager::Instance();
    auto result = nm.SetRtspPort(port);
    
    if (!result.success) {
        spdlog::error("Failed to set RTSP port: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_get_nic_speed(const char *iface, OnvifNicSpeed *config) {
    if (!config) return -1;
    
    std::string interface = iface ? iface : "eth0";
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.GetNicSpeed(interface);
    
    if (!result.success) {
        spdlog::warn("Failed to get NIC speed: {}", result.error);
        config->speed = 1000;  // Default to 1Gbps
        config->duplex = 1;    // Full duplex
        config->autoneg = 1;   // Autoneg enabled
        return 0;
    }
    
    auto& speed = result.value;
    config->speed = speed.speed;
    config->duplex = speed.duplex;
    config->autoneg = speed.autoneg;
    
    return 0;
}

int onvif_set_nic_speed(const char *iface, const OnvifNicSpeed *config) {
    if (!config) return -1;
    
    std::string interface = iface ? iface : "eth0";
    
    NetworkManager::NicSpeed speed;
    speed.speed = config->speed;
    speed.duplex = config->duplex;
    speed.autoneg = config->autoneg;
    
    auto& nm = NetworkManager::Instance();
    auto result = nm.SetNicSpeed(interface, speed);
    
    if (!result.success) {
        spdlog::error("Failed to set NIC speed: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_get_network_interface(const char *iface, OnvifNetworkInterface *info) {
    if (!info) return -1;
    
    std::string interface = iface ? iface : "eth0";
    memset(info, 0, sizeof(OnvifNetworkInterface));
    
    strncpy(info->name, interface.c_str(), sizeof(info->name) - 1);
    
    // Get MAC address
    onvif_get_mac_address(iface, info->mac_address);
    
    // Get IPv4 config
    OnvifIpv4Config ipv4;
    if (onvif_get_ipv4_config(iface, &ipv4) == 0) {
        info->ipv4 = ipv4;
    }
    
    // Assume link is up if we have an IP
    info->link_up = (strlen(info->ipv4.address) > 0 && 
                     strcmp(info->ipv4.address, "0.0.0.0") != 0);
    
    return 0;
}

int onvif_get_network_interface_count(void) {
    // Typically IP cameras have 1 Ethernet interface
    return 1;
}

int onvif_check_internet_connectivity(void) {
    auto& nm = NetworkManager::Instance();
    return nm.CheckInternetConnectivity() ? 1 : 0;
}

} // extern "C"
