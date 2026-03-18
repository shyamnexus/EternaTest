#include "ipcam/network_manager.h"
#include "ipcam/ssl_manager.h"
#include "ipcam/nginx_manager.h"
#include "ipcam/upnp_manager.h"
#include "ipcam/ntp_manager.h"
#include "ipcam/credential_manager.h"
#include "ipcam/mdns_responder.h"
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ipcam/config.h>
#include <spdlog/spdlog.h>
#include <curl/curl.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <mutex>
#include <signal.h>

// Macros for logging compatibility
#define LOG_WARNING(...) spdlog::warn(__VA_ARGS__)
#define LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define LOG_ERROR(...) spdlog::error(__VA_ARGS__)

namespace ipcam {
namespace networking {

// Helper functions
namespace {

bool hexToIp(const std::string& hex, std::string& out) {
    if(hex.empty()) return false;
    
    unsigned long gw = std::strtoul(hex.c_str(), nullptr, 16);
    struct in_addr a;
    a.s_addr = gw;
    
    // Gateway in /proc/net/route is little-endian hex; convert
    unsigned long gw_be = ntohl(a.s_addr);
    a.s_addr = htonl(gw_be);
    
    char buffer[INET_ADDRSTRLEN] = {0};
    const char* s = inet_ntop(AF_INET, &a, buffer, sizeof(buffer));
    if (s != nullptr) {
        out = s;
        return true;
    }
    return false;
}

int netmaskToPrefix(const std::string& netmask) {
    if(netmask.empty()) return -1;
    
    struct in_addr nm;
    if (inet_pton(AF_INET, netmask.c_str(), &nm) != 1) return -1;
    
    uint32_t m = ntohl(nm.s_addr);
    int bits = 0;
    while (m & 0x80000000) {
        bits++;
        m <<= 1;
    }
    return bits;
}

// Helper to regenerate nginx config and reload nginx
bool RegenerateNginxConfig() {
    // Use the NginxManager instead of shell script
    auto& nginx_mgr = NginxManager::Instance();
    
    auto gen_result = nginx_mgr.GenerateConfig();
    if (!gen_result) {
        spdlog::error("Failed to generate nginx config: {}", gen_result.error);
        return false;
    }
    
    auto reload_result = nginx_mgr.Reload();
    if (!reload_result) {
        spdlog::error("Failed to reload nginx: {}", reload_result.error);
        return false;
    }
    
    return true;
}

} // anonymous namespace

// Singleton implementation
NetworkManager& NetworkManager::Instance() {
    static NetworkManager instance;
    return instance;
}

// Constructor - initialize manager instances
NetworkManager::NetworkManager() {
    spdlog::info("[NetworkManager] Initializing with internal managers");
    smtp_manager_ = std::make_unique<SmtpManager>();
    snmp_manager_ = std::make_unique<SnmpManager>();
    upnp_manager_ = std::make_unique<UpnpManager>();
}

// Destructor
NetworkManager::~NetworkManager() {
    spdlog::info("[NetworkManager] Shutting down");
}

// ============================================================================
// SMTP Facade Methods (delegate to SmtpManager)
// ============================================================================

Result<NetworkManager::SmtpConfig> NetworkManager::GetSmtpConfig() {
    if (!smtp_manager_) {
        return Result<SmtpConfig>::Err("SMTP manager not initialized");
    }
    return smtp_manager_->GetConfig();
}

Result<void> NetworkManager::SetSmtpConfig(const SmtpConfig& cfg) {
    if (!smtp_manager_) {
        return Result<void>::Err("SMTP manager not initialized");
    }
    return smtp_manager_->SetConfig(cfg);
}

Result<void> NetworkManager::TestSmtpConnection() {
    if (!smtp_manager_) {
        return Result<void>::Err("SMTP manager not initialized");
    }
    return smtp_manager_->TestConnection();
}

Result<void> NetworkManager::SendEmail(const EmailMessage& email) {
    if (!smtp_manager_) {
        return Result<void>::Err("SMTP manager not initialized");
    }
    return smtp_manager_->SendEmail(email);
}

Result<void> NetworkManager::SendTestEmail(const std::string& to_email) {
    if (!smtp_manager_) {
        return Result<void>::Err("SMTP manager not initialized");
    }
    return smtp_manager_->SendTestEmail(to_email);
}

// ============================================================================
// SNMP Facade Methods (delegate to SnmpManager - stubs for now)
// ============================================================================

Result<NetworkManager::SnmpConfig> NetworkManager::GetSnmpConfig() {
    if (!snmp_manager_) {
        return Result<SnmpConfig>::Err("SNMP manager not initialized");
    }
    return snmp_manager_->GetConfig();
}

Result<void> NetworkManager::SetSnmpConfig(const SnmpConfig& cfg) {
    if (!snmp_manager_) {
        return Result<void>::Err("SNMP manager not initialized");
    }
    return snmp_manager_->SetConfig(cfg);
}

Result<void> NetworkManager::StartSnmp() {
    if (!snmp_manager_) {
        return Result<void>::Err("SNMP manager not initialized");
    }
    return snmp_manager_->Start();
}

Result<void> NetworkManager::StopSnmp() {
    if (!snmp_manager_) {
        return Result<void>::Err("SNMP manager not initialized");
    }
    return snmp_manager_->Stop();
}

Result<void> NetworkManager::RestartSnmp() {
    if (!snmp_manager_) {
        return Result<void>::Err("SNMP manager not initialized");
    }
    return snmp_manager_->Restart();
}

Result<bool> NetworkManager::IsSnmpRunning() {
    if (!snmp_manager_) {
        return Result<bool>::Err("SNMP manager not initialized");
    }
    return snmp_manager_->IsRunning();
}

// ============================================================================
// UPnP Facade Methods (delegate to UpnpManager)
// ============================================================================

Result<NetworkManager::UpnpConfig> NetworkManager::GetUpnpConfig() {
    if (!upnp_manager_) {
        return Result<UpnpConfig>::Err("UPnP manager not initialized");
    }
    return upnp_manager_->GetConfig();
}

Result<void> NetworkManager::SetUpnpConfig(const UpnpConfig& cfg) {
    if (!upnp_manager_) {
        return Result<void>::Err("UPnP manager not initialized");
    }
    return upnp_manager_->SetConfig(cfg);
}

Result<NetworkManager::UpnpDeviceInfo> NetworkManager::DiscoverUpnp(int timeout_ms) {
    if (!upnp_manager_) {
        return Result<UpnpDeviceInfo>::Err("UPnP manager not initialized");
    }
    return upnp_manager_->Discover(timeout_ms);
}

Result<void> NetworkManager::AddUpnpPortMapping(const UpnpPortMapping& mapping) {
    if (!upnp_manager_) {
        return Result<void>::Err("UPnP manager not initialized");
    }
    return upnp_manager_->AddPortMapping(mapping);
}

Result<void> NetworkManager::DeleteUpnpPortMapping(int external_port, const std::string& protocol) {
    if (!upnp_manager_) {
        return Result<void>::Err("UPnP manager not initialized");
    }
    return upnp_manager_->DeletePortMapping(external_port, protocol);
}

Result<std::vector<NetworkManager::UpnpPortMapping>> NetworkManager::GetUpnpPortMappings() {
    if (!upnp_manager_) {
        return Result<std::vector<NetworkManager::UpnpPortMapping>>::Err("UPnP manager not initialized");
    }
    return upnp_manager_->GetPortMappings();
}

// Validation helpers
bool NetworkManager::ValidateIpv4Address(const std::string& addr) {
    if (addr.empty()) return false;
    struct in_addr a;
    return inet_pton(AF_INET, addr.c_str(), &a) == 1;
}

bool NetworkManager::ValidateIpv6Address(const std::string& addr) {
    if (addr.empty()) return false;
    struct in6_addr a;
    return inet_pton(AF_INET6, addr.c_str(), &a) == 1;
}

bool NetworkManager::ValidateMulticastAddress(const std::string& addr) {
    if (addr.empty()) return false;
    
    struct in_addr a;
    if (inet_pton(AF_INET, addr.c_str(), &a) != 1) {
        return false;
    }
    
    // IPv4 multicast range: 224.0.0.0 to 239.255.255.255
    uint32_t ip = ntohl(a.s_addr);
    return (ip >= 0xE0000000) && (ip <= 0xEFFFFFFF);
}

bool NetworkManager::ValidatePort(int port) {
    return port >= 1 && port <= 65535;
}

// IPv4 Configuration
Result<NetworkManager::Ipv4Config> NetworkManager::GetIpv4(const std::string& iface) {
    if (iface.empty()) {
        return Result<Ipv4Config>::Err("Interface name cannot be empty");
    }
    
    Ipv4Config cfg;
    // Read the method from configuration instead of assuming DHCP
    cfg.method = config::Get<std::string>("network.ipv4.method", "dhcp");
    
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        return Result<Ipv4Config>::Err("Failed to get interface addresses");
    }

    bool found = false;
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (std::strcmp(ifa->ifa_name, iface.c_str()) != 0) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            char addrbuf[INET_ADDRSTRLEN] = {0};
            char maskbuf[INET_ADDRSTRLEN] = {0};
            struct sockaddr_in *sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
            struct sockaddr_in *nm = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_netmask);
            
            inet_ntop(AF_INET, &sa->sin_addr, addrbuf, sizeof(addrbuf));
            inet_ntop(AF_INET, &nm->sin_addr, maskbuf, sizeof(maskbuf));
            
            cfg.address = addrbuf;
            cfg.netmask = maskbuf;
            found = true;
            break;
        }
    }
    freeifaddrs(ifaddr);

    if (!found) {
        return Result<Ipv4Config>::Err("Interface not found or has no IPv4 address");
    }

    // Try to find default gateway for this interface
    std::ifstream f("/proc/net/route");
    std::string line;
    
    // Skip header
    std::getline(f, line);
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::string ifname, dest, gw, flags, rest;
        
        if (!(iss >> ifname >> dest >> gw)) continue;
        
        if (dest == "00000000" && ifname == iface) {
            std::string gwIp;
            if (hexToIp(gw, gwIp)) {
                cfg.gateway = gwIp;
                break;
            }
        }
    }

    return Result<Ipv4Config>::Ok(cfg);
}

Result<void> NetworkManager::SetIpv4(const std::string& iface, const Ipv4Config& cfg) {
    if (iface.empty()) {
        return Result<void>::Err("Interface name cannot be empty");
    }

    if (cfg.method == "dhcp") {
        // First, capture the current IP configuration before any changes
        // This will be used as fallback if DHCP fails
        auto current_config = GetIpv4(iface);
        std::string current_address = "";
        std::string current_netmask = "";
        std::string current_gateway = "";
        
        if (current_config.success) {
            current_address = current_config.value.address;
            current_netmask = current_config.value.netmask;
            current_gateway = current_config.value.gateway;
            spdlog::info("Captured current IP config before DHCP switch: {}/{} gw {}", 
                        current_address, current_netmask, current_gateway);
        }
        
        // Kill any existing DHCP client first
        std::string kill_cmd = "killall -q udhcpc 2>/dev/null || true";
        system(kill_cmd.c_str());
        
        // DON'T flush the IP - keep current IP while DHCP tries to get a new one
        // This maintains connectivity even if DHCP server is not available
        
        // Get DHCP configuration 
        std::string pid_file = config::Get<std::string>("network.ipv4.dhcp_options.pid_file", "/var/run/udhcpc_" + iface + ".pid");
        int timeout = config::Get<int>("network.ipv4.dhcp_options.timeout", 5);
        int retry_count = config::Get<int>("network.ipv4.dhcp_options.retry_count", 3);
        
        // Build udhcpc command
        // -b: Go to background immediately (don't block waiting for lease)
        // -S: Log to syslog
        // Note: udhcpc will update the IP only when it successfully gets a lease
        std::ostringstream dhcp_cmd;
        dhcp_cmd << "udhcpc -i " << iface 
                << " -p " << pid_file
                << " -T " << timeout
                << " -t " << retry_count
                << " -b";   // Background immediately
        
        spdlog::info("Starting DHCP client: {}", dhcp_cmd.str());
        
        int rc = system(dhcp_cmd.str().c_str());
        if (rc != 0) {
            spdlog::warn("DHCP client failed to start, keeping current IP: {}", current_address);
        }
        
        // Persist DHCP configuration
        // Store current IP as fallback so it persists across reboots if DHCP fails
        config::Set<std::string>("network.ipv4.method", "dhcp");
        config::Set<std::string>("network.ipv4.interface", iface);
        
        // Store current IP as fallback address (used if DHCP fails on next boot)
        if (!current_address.empty() && current_address != "0.0.0.0") {
            config::Set<bool>("network.ipv4.fallback.enabled", true);
            config::Set<std::string>("network.ipv4.fallback.address", current_address);
            config::Set<std::string>("network.ipv4.fallback.netmask", current_netmask);
            config::Set<std::string>("network.ipv4.fallback.gateway", current_gateway);
        }
        
        // Clear the static IP settings (method is now DHCP)
        config::Set<std::string>("network.ipv4.address", "");
        config::Set<std::string>("network.ipv4.netmask", "");
        config::Set<std::string>("network.ipv4.gateway", "");
        config::Save();
        
        spdlog::info("IPv4 configuration set to DHCP on interface {} (fallback: {})", iface, current_address);
        return Result<void>::Ok();
    }

    // Static/manual configuration
    if (cfg.address.empty() || cfg.netmask.empty()) {
        return Result<void>::Err("Address and netmask are required for static IP");
    }
    
    if (!ValidateIpv4Address(cfg.address)) {
        return Result<void>::Err("Invalid IPv4 address");
    }
    
    if (!ValidateIpv4Address(cfg.netmask)) {
        return Result<void>::Err("Invalid netmask");
    }
    
    int prefix = netmaskToPrefix(cfg.netmask);
    if (prefix <= 0) {
        return Result<void>::Err("Invalid netmask format");
    }
    
    // Kill any running DHCP client before setting static IP
    std::string kill_cmd = "killall -q udhcpc 2>/dev/null || true";
    system(kill_cmd.c_str());
    
    // Construct command to configure interface
    std::ostringstream cmdStream;
    cmdStream << "ip addr flush dev " << iface
             << " && ip addr add " << cfg.address
             << "/" << prefix
             << " dev " << iface
             << " && ip link set " << iface << " up";
    
    int rc = system(cmdStream.str().c_str());
    if (rc != 0) {
        return Result<void>::Err("Failed to configure interface");
    }
    
    // Set gateway if provided
    if (!cfg.gateway.empty()) {
        if (!ValidateIpv4Address(cfg.gateway)) {
            return Result<void>::Err("Invalid gateway address");
        }
        
        std::ostringstream gwCmdStream;
        gwCmdStream << "ip route replace default via " 
                   << cfg.gateway << " dev " << iface;
        system(gwCmdStream.str().c_str());
    }
    
    // Apply MTU if configured
    int mtu = config::Get<int>("network.ipv4.mtu", 1500);
    if (mtu != 1500) {
        MtuConfig mtu_cfg;
        mtu_cfg.mtu = mtu;
        mtu_cfg.path_mtu_discovery = config::Get<bool>("network.ipv4.path_mtu_discovery", true);
        SetMtu(iface, mtu_cfg);
    }
    
    // Send gratuitous ARP to announce new IP to network
    SendGratuitousArp(iface, cfg.address);
    
    // Persist static IP configuration
    config::Set<std::string>("network.ipv4.method", "static");
    config::Set<std::string>("network.ipv4.interface", iface);
    config::Set<std::string>("network.ipv4.address", cfg.address);
    config::Set<std::string>("network.ipv4.netmask", cfg.netmask);
    config::Set<std::string>("network.ipv4.gateway", cfg.gateway);
    config::Save();
    
    spdlog::info("Static IPv4 configuration saved: {}/{} gateway {} on {}", 
                 cfg.address, cfg.netmask, cfg.gateway, iface);
    
    return Result<void>::Ok();
}

// IPv6 Configuration
Result<NetworkManager::Ipv6Config> NetworkManager::GetIpv6(const std::string& iface) {
    if (iface.empty()) {
        return Result<Ipv6Config>::Err("Interface name cannot be empty");
    }
    
    Ipv6Config cfg;
    // Read configuration values from config instead of hardcoding
    cfg.enabled = config::Get<bool>("network.ipv6.enabled", true);
    cfg.method = config::Get<std::string>("network.ipv6.method", "auto");
    cfg.prefix_length = config::Get<int>("network.ipv6.prefix_length", 64);
    cfg.privacy_extensions = config::Get<bool>("network.ipv6.privacy_extensions", true);
    cfg.accept_ra = config::Get<bool>("network.ipv6.accept_ra", true);
    
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        return Result<Ipv6Config>::Err("Failed to get interface addresses");
    }

    bool found = false;
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (std::strcmp(ifa->ifa_name, iface.c_str()) != 0) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET6) {
            char addrbuf[INET6_ADDRSTRLEN] = {0};
            struct sockaddr_in6 *sa = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
            
            // Skip link-local addresses for configuration display
            if (IN6_IS_ADDR_LINKLOCAL(&sa->sin6_addr)) {
                continue;
            }
            
            inet_ntop(AF_INET6, &sa->sin6_addr, addrbuf, sizeof(addrbuf));
            cfg.address = addrbuf;
            found = true;
            break;
        }
    }
    freeifaddrs(ifaddr);

    if (!found) {
        // IPv6 might not be configured yet, return default config
        cfg.enabled = false;
        cfg.address = "::";
    }

    // Try to find default IPv6 gateway
    std::ifstream f("/proc/net/ipv6_route");
    std::string line;
    
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::string dest, prefix, src, src_prefix, next_hop, metric, refcnt, use, flags, ifname;
        
        if (!(iss >> dest >> prefix >> src >> src_prefix >> next_hop >> metric >> refcnt >> use >> flags >> ifname)) 
            continue;
        
        // Default route has destination 00000000000000000000000000000000
        if (dest == "00000000000000000000000000000000" && ifname == iface) {
            // Convert hex string to IPv6 address
            if (next_hop.length() == 32) {
                struct in6_addr gw_addr;
                for (int i = 0; i < 16; i++) {
                    std::string byte_str = next_hop.substr(i * 2, 2);
                    gw_addr.s6_addr[i] = static_cast<unsigned char>(std::strtoul(byte_str.c_str(), nullptr, 16));
                }
                
                char gw_buf[INET6_ADDRSTRLEN] = {0};
                inet_ntop(AF_INET6, &gw_addr, gw_buf, sizeof(gw_buf));
                cfg.gateway = gw_buf;
                break;
            }
        }
    }

    return Result<Ipv6Config>::Ok(cfg);
}

Result<void> NetworkManager::SetIpv6(const std::string& iface, const Ipv6Config& cfg) {
    if (iface.empty()) {
        return Result<void>::Err("Interface name cannot be empty");
    }

    // Persist enabled state first
    config::Set<bool>("network.ipv6.enabled", cfg.enabled);

    // Disable IPv6 if requested
    if (!cfg.enabled) {
        std::string cmd = "sysctl -w net.ipv6.conf." + iface + ".disable_ipv6=1";
        int rc = system(cmd.c_str());
        config::Save();
        if (rc != 0) {
            return Result<void>::Err("Failed to disable IPv6");
        }
        spdlog::info("IPv6 disabled on interface {}", iface);
        return Result<void>::Ok();
    }
    
    // Enable IPv6
    std::string enable_cmd = "sysctl -w net.ipv6.conf." + iface + ".disable_ipv6=0";
    system(enable_cmd.c_str());

    if (cfg.method == "auto" || cfg.method == "slaac") {
        // SLAAC mode - get address from router advertisements
        std::string accept_ra_cmd = "sysctl -w net.ipv6.conf." + iface + ".accept_ra=2";
        system(accept_ra_cmd.c_str());
        
        // Enable autoconf
        std::string autoconf_cmd = "sysctl -w net.ipv6.conf." + iface + ".autoconf=1";
        system(autoconf_cmd.c_str());
        
        // Enable privacy extensions if requested
        if (cfg.privacy_extensions) {
            std::string privacy_cmd = "sysctl -w net.ipv6.conf." + iface + ".use_tempaddr=2";
            system(privacy_cmd.c_str());
        } else {
            std::string privacy_cmd = "sysctl -w net.ipv6.conf." + iface + ".use_tempaddr=0";
            system(privacy_cmd.c_str());
        }
        
        // Persist configuration
        config::Set<std::string>("network.ipv6.method", cfg.method);
        config::Set<std::string>("network.ipv6.interface", iface);
        config::Set<bool>("network.ipv6.accept_ra", cfg.accept_ra);
        config::Set<bool>("network.ipv6.privacy_extensions", cfg.privacy_extensions);
        config::Set<int>("network.ipv6.prefix_length", cfg.prefix_length);
        // Clear static settings when using auto
        config::Set<std::string>("network.ipv6.address", "");
        config::Set<std::string>("network.ipv6.gateway", "");
        config::Save();
        
        spdlog::info("IPv6 set to auto/SLAAC mode on interface {}", iface);
        return Result<void>::Ok();
    }
    
    if (cfg.method == "dhcp" || cfg.method == "dhcpv6") {
        // DHCPv6 mode
        // Accept router advertisements for M/O flags
        std::string accept_ra_cmd = "sysctl -w net.ipv6.conf." + iface + ".accept_ra=" + 
                                    (cfg.accept_ra ? "2" : "0");
        system(accept_ra_cmd.c_str());
        
        // Enable privacy extensions if requested
        if (cfg.privacy_extensions) {
            std::string privacy_cmd = "sysctl -w net.ipv6.conf." + iface + ".use_tempaddr=2";
            system(privacy_cmd.c_str());
        }
        
        // Persist configuration
        config::Set<std::string>("network.ipv6.method", "dhcp");
        config::Set<std::string>("network.ipv6.interface", iface);
        config::Set<bool>("network.ipv6.accept_ra", cfg.accept_ra);
        config::Set<bool>("network.ipv6.privacy_extensions", cfg.privacy_extensions);
        config::Set<int>("network.ipv6.prefix_length", cfg.prefix_length);
        config::Save();
        
        spdlog::info("IPv6 set to DHCPv6 mode on interface {}", iface);
        return Result<void>::Ok();
    }

    // Static configuration
    if (cfg.address.empty()) {
        return Result<void>::Err("Address is required for static IPv6");
    }
    
    if (!ValidateIpv6Address(cfg.address)) {
        return Result<void>::Err("Invalid IPv6 address");
    }
    
    if (cfg.prefix_length <= 0 || cfg.prefix_length > 128) {
        return Result<void>::Err("Invalid prefix length (must be 1-128)");
    }
    
    // Disable autoconf for static configuration
    std::string autoconf_cmd = "sysctl -w net.ipv6.conf." + iface + ".autoconf=0";
    system(autoconf_cmd.c_str());
    
    // Construct command to configure interface
    // Don't flush first - add the new address (allows graceful transition)
    std::ostringstream cmdStream;
    cmdStream << "ip -6 addr add " << cfg.address
             << "/" << cfg.prefix_length
             << " dev " << iface << " 2>/dev/null || true";
    
    int rc = system(cmdStream.str().c_str());
    if (rc != 0) {
        spdlog::warn("Failed to add IPv6 address (may already exist)");
    }
    
    // Set gateway if provided
    if (!cfg.gateway.empty()) {
        if (!ValidateIpv6Address(cfg.gateway)) {
            return Result<void>::Err("Invalid IPv6 gateway address");
        }
        
        std::ostringstream gwCmdStream;
        gwCmdStream << "ip -6 route replace default via " 
                   << cfg.gateway << " dev " << iface;
        system(gwCmdStream.str().c_str());
    }
    
    // Persist static configuration
    config::Set<std::string>("network.ipv6.method", "static");
    config::Set<std::string>("network.ipv6.interface", iface);
    config::Set<std::string>("network.ipv6.address", cfg.address);
    config::Set<int>("network.ipv6.prefix_length", cfg.prefix_length);
    config::Set<std::string>("network.ipv6.gateway", cfg.gateway);
    config::Set<bool>("network.ipv6.accept_ra", cfg.accept_ra);
    config::Set<bool>("network.ipv6.privacy_extensions", cfg.privacy_extensions);
    config::Save();
    
    spdlog::info("Static IPv6 configuration saved: {}/{} gateway {} on {}", 
                 cfg.address, cfg.prefix_length, cfg.gateway, iface);
    
    return Result<void>::Ok();
}

// DNS Configuration
Result<NetworkManager::DnsConfig> NetworkManager::GetDns() {
    std::ifstream f("/etc/resolv.conf");
    if (!f) {
        return Result<DnsConfig>::Err("Cannot open /etc/resolv.conf");
    }
    
    DnsConfig cfg;
    std::string line;
    int ipv4_count = 0;
    int ipv6_count = 0;
    
    while (std::getline(f, line)) {
        if (line.rfind("nameserver", 0) == 0) {
            std::istringstream iss(line);
            std::string token, addr;
            iss >> token >> addr;
            
            // Check if it's IPv6 or IPv4
            if (addr.find(':') != std::string::npos) {
                // IPv6 address
                if (ipv6_count == 0) {
                    cfg.primary_ipv6 = addr;
                    ipv6_count++;
                } else if (ipv6_count == 1) {
                    cfg.secondary_ipv6 = addr;
                    ipv6_count++;
                }
            } else {
                // IPv4 address
                if (ipv4_count == 0) {
                    cfg.primary = addr;
                    ipv4_count++;
                } else if (ipv4_count == 1) {
                    cfg.secondary = addr;
                    ipv4_count++;
                }
            }
        }
    }
    
    if (ipv4_count == 0 && ipv6_count == 0) {
        return Result<DnsConfig>::Err("No DNS servers configured");
    }
    
    return Result<DnsConfig>::Ok(cfg);
}

Result<void> NetworkManager::SetDns(const DnsConfig& cfg) {
    // Validate that at least one DNS server is provided
    if (cfg.primary.empty() && cfg.primary_ipv6.empty()) {
        return Result<void>::Err("At least one DNS server is required");
    }
    
    // Validate IPv4 addresses
    if (!cfg.primary.empty() && !ValidateIpv4Address(cfg.primary)) {
        return Result<void>::Err("Invalid primary DNS address (IPv4)");
    }
    
    if (!cfg.secondary.empty() && !ValidateIpv4Address(cfg.secondary)) {
        return Result<void>::Err("Invalid secondary DNS address (IPv4)");
    }
    
    // Validate IPv6 addresses
    if (!cfg.primary_ipv6.empty() && !ValidateIpv6Address(cfg.primary_ipv6)) {
        return Result<void>::Err("Invalid primary DNS address (IPv6)");
    }
    
    if (!cfg.secondary_ipv6.empty() && !ValidateIpv6Address(cfg.secondary_ipv6)) {
        return Result<void>::Err("Invalid secondary DNS address (IPv6)");
    }
    
    std::ofstream f("/etc/resolv.conf", std::ios::trunc);
    if (!f) {
        return Result<void>::Err("Cannot write to /etc/resolv.conf");
    }
    
    // Write IPv4 DNS servers
    if (!cfg.primary.empty()) {
        f << "nameserver " << cfg.primary << "\n";
    }
    if (!cfg.secondary.empty()) {
        f << "nameserver " << cfg.secondary << "\n";
    }
    
    // Write IPv6 DNS servers
    if (!cfg.primary_ipv6.empty()) {
        f << "nameserver " << cfg.primary_ipv6 << "\n";
    }
    if (!cfg.secondary_ipv6.empty()) {
        f << "nameserver " << cfg.secondary_ipv6 << "\n";
    }
    
    // Persist DNS settings to config for restore after reboot
    config::Set<std::string>("network.dns.method", "manual");
    config::Set<std::string>("network.dns.dns1", cfg.primary);
    config::Set<std::string>("network.dns.dns2", cfg.secondary);
    config::Set<std::string>("network.dns.dns1_ipv6", cfg.primary_ipv6);
    config::Set<std::string>("network.dns.dns2_ipv6", cfg.secondary_ipv6);
    config::Save();
    
    spdlog::info("DNS configuration saved: primary={}, secondary={}", cfg.primary, cfg.secondary);
    
    return Result<void>::Ok();
}

// MTU Configuration
Result<NetworkManager::MtuConfig> NetworkManager::GetMtu(const std::string& iface) {
    if (iface.empty()) {
        return Result<MtuConfig>::Err("Interface name cannot be empty");
    }
    
    MtuConfig cfg;
    cfg.mtu = config::Get<int>("network.ipv4.mtu", 1500);
    cfg.path_mtu_discovery = config::Get<bool>("network.ipv4.path_mtu_discovery", true);
    
    // Read actual MTU from interface
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return Result<MtuConfig>::Err("Failed to create socket for MTU query");
    }
    
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(fd, SIOCGIFMTU, &ifr) == 0) {
        cfg.mtu = ifr.ifr_mtu;
    }
    close(fd);
    
    return Result<MtuConfig>::Ok(cfg);
}

Result<void> NetworkManager::SetMtu(const std::string& iface, const MtuConfig& cfg) {
    if (iface.empty()) {
        return Result<void>::Err("Interface name cannot be empty");
    }
    
    // Validate MTU range (68 is minimum for IPv4, 9000 is jumbo frame)
    if (cfg.mtu < 68 || cfg.mtu > 9000) {
        return Result<void>::Err("MTU must be between 68 and 9000");
    }
    
    // Set MTU using ioctl
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return Result<void>::Err("Failed to create socket for MTU configuration");
    }
    
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);
    ifr.ifr_mtu = cfg.mtu;
    
    if (ioctl(fd, SIOCSIFMTU, &ifr) != 0) {
        close(fd);
        return Result<void>::Err("Failed to set MTU: " + std::string(strerror(errno)));
    }
    close(fd);
    
    // Configure Path MTU Discovery via sysctl
    std::string pmtud_value = cfg.path_mtu_discovery ? "1" : "0";
    std::ofstream pmtud_file("/proc/sys/net/ipv4/ip_no_pmtu_disc");
    if (pmtud_file) {
        // Note: ip_no_pmtu_disc=0 means PMTUD is enabled
        pmtud_file << (cfg.path_mtu_discovery ? "0" : "1");
        pmtud_file.close();
    }
    
    // Persist MTU settings
    config::Set<int>("network.ipv4.mtu", cfg.mtu);
    config::Set<bool>("network.ipv4.path_mtu_discovery", cfg.path_mtu_discovery);
    config::Save();
    
    spdlog::info("MTU set to {} on interface {} (PMTUD: {})", cfg.mtu, iface, 
                 cfg.path_mtu_discovery ? "enabled" : "disabled");
    
    return Result<void>::Ok();
}

// Gratuitous ARP Configuration
Result<NetworkManager::GratuitousArpConfig> NetworkManager::GetGratuitousArpConfig() {
    GratuitousArpConfig cfg;
    cfg.enabled = config::Get<bool>("network.ipv4.gratuitous_arp.enabled", true);
    cfg.count = config::Get<int>("network.ipv4.gratuitous_arp.count", 3);
    cfg.interval_ms = config::Get<int>("network.ipv4.gratuitous_arp.interval_ms", 500);
    return Result<GratuitousArpConfig>::Ok(cfg);
}

Result<void> NetworkManager::SetGratuitousArpConfig(const GratuitousArpConfig& cfg) {
    // Validate configuration
    if (cfg.count < 1 || cfg.count > 10) {
        return Result<void>::Err("ARP announcement count must be between 1 and 10");
    }
    if (cfg.interval_ms < 100 || cfg.interval_ms > 5000) {
        return Result<void>::Err("ARP interval must be between 100 and 5000 ms");
    }
    
    // Persist configuration
    config::Set<bool>("network.ipv4.gratuitous_arp.enabled", cfg.enabled);
    config::Set<int>("network.ipv4.gratuitous_arp.count", cfg.count);
    config::Set<int>("network.ipv4.gratuitous_arp.interval_ms", cfg.interval_ms);
    config::Save();
    
    spdlog::info("Gratuitous ARP config: enabled={}, count={}, interval_ms={}", 
                 cfg.enabled, cfg.count, cfg.interval_ms);
    
    return Result<void>::Ok();
}

Result<void> NetworkManager::SendGratuitousArp(const std::string& iface, const std::string& ip_address) {
    std::string iface_name = iface.empty() ? "eth0" : iface;
    std::string target_ip = ip_address;
    
    // If no IP provided, get current IP from interface
    if (target_ip.empty()) {
        auto ipv4_result = GetIpv4(iface_name);
        if (!ipv4_result.success || ipv4_result.value.address.empty()) {
            return Result<void>::Err("No IP address to announce");
        }
        target_ip = ipv4_result.value.address;
    }
    
    // Check if gratuitous ARP is enabled
    auto arp_config = GetGratuitousArpConfig();
    if (!arp_config.success) {
        return Result<void>::Err("Failed to get gratuitous ARP config");
    }
    
    if (!arp_config.value.enabled) {
        spdlog::debug("Gratuitous ARP is disabled, skipping announcement");
        return Result<void>::Ok();
    }
    
    int count = arp_config.value.count;
    int interval_ms = arp_config.value.interval_ms;
    
    // Build arping command for gratuitous ARP
    // -A: ARP reply mode (gratuitous ARP)
    // -U: Unsolicited ARP mode (alternative, also works)
    // -I: Interface
    // -c: Count
    // -w: Timeout (use interval * count as total timeout)
    std::ostringstream cmd;
    cmd << "arping -A -I " << iface_name 
        << " -c " << count 
        << " -w " << ((count * interval_ms) / 1000 + 1)
        << " " << target_ip 
        << " >/dev/null 2>&1 &";  // Run in background
    
    spdlog::info("Sending gratuitous ARP: {} on {} ({} times)", target_ip, iface_name, count);
    
    int rc = system(cmd.str().c_str());
    if (rc != 0) {
        // arping might not be installed, try using ip neigh
        spdlog::warn("arping command failed, attempting fallback with raw socket");
        // Fallback: just log warning, device still works
        return Result<void>::Ok();  // Don't fail the operation
    }
    
    return Result<void>::Ok();
}

// MAC Address
Result<std::string> NetworkManager::GetMac(const std::string& iface) {
    if (iface.empty()) {
        return Result<std::string>::Err("Interface name cannot be empty");
    }
    
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return Result<std::string>::Err("Failed to create socket");
    }
    
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ-1);
    
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        close(fd);
        return Result<std::string>::Err("Failed to get MAC address");
    }
    
    close(fd);
    
    unsigned char *hw = reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);
    char macBuffer[18];
    std::snprintf(macBuffer, sizeof(macBuffer), "%02x:%02x:%02x:%02x:%02x:%02x",
               hw[0], hw[1], hw[2], hw[3], hw[4], hw[5]);
    
    return Result<std::string>::Ok(std::string(macBuffer));
}

// NIC Speed (stub implementations for now - platform specific)
Result<NetworkManager::NicSpeed> NetworkManager::GetNicSpeed(const std::string& iface) {
    (void)iface;
    // TODO: Implement using ethtool or similar
    return Result<NicSpeed>::Err("NIC speed detection not implemented");
}

Result<void> NetworkManager::SetNicSpeed(const std::string& iface, const NicSpeed& cfg) {
    (void)iface;
    (void)cfg;
    // TODO: Implement using ethtool or similar
    return Result<void>::Err("NIC speed configuration not implemented");
}

Result<std::string> NetworkManager::GetSupportedNicSpeeds(const std::string& iface) {
    (void)iface;
    // TODO: Implement using ethtool or similar
    return Result<std::string>::Err("NIC speed detection not implemented");
}

// Port Configuration (uses config module)
Result<int> NetworkManager::GetHttpPort() {
    int port = config::Get<int>("network.http.port", 80);
    return Result<int>::Ok(port);
}

Result<void> NetworkManager::SetHttpPort(int port) {
    if (!ValidatePort(port)) {
        return Result<void>::Err("Invalid port number (must be 1-65535)");
    }
    
    config::Set<int>("network.http.port", port);
    if (!config::Save()) {
        return Result<void>::Err("Failed to save configuration");
    }
    
    // Regenerate nginx config and reload
    if (!RegenerateNginxConfig()) {
        return Result<void>::Err("Webserver configuration changed - restart required");
    }
    
    return Result<void>::Ok();
}

Result<int> NetworkManager::GetHttpsPort() {
    int port = config::Get<int>("network.https.port", 443);
    return Result<int>::Ok(port);
}

Result<void> NetworkManager::SetHttpsPort(int port) {
    if (!ValidatePort(port)) {
        return Result<void>::Err("Invalid port number (must be 1-65535)");
    }
    
    config::Set<int>("network.https.port", port);
    if (!config::Save()) {
        return Result<void>::Err("Failed to save configuration");
    }
    
    // Regenerate nginx config and reload
    if (!RegenerateNginxConfig()) {
        return Result<void>::Err("Webserver configuration changed - restart required");
    }
    
    return Result<void>::Ok();
}

Result<void> NetworkManager::SetSslCertificate(const std::string& cert_path, const std::string& key_path) {
    if (cert_path.empty() || key_path.empty()) {
        return Result<void>::Err("Certificate and key paths cannot be empty");
    }
    
    // Verify files exist
    std::ifstream cert_file(cert_path);
    std::ifstream key_file(key_path);
    
    if (!cert_file.good()) {
        return Result<void>::Err("SSL certificate file not found: " + cert_path);
    }
    if (!key_file.good()) {
        return Result<void>::Err("SSL key file not found: " + key_path);
    }
    
    // Update configuration
    config::Set<std::string>("network.https.ssl_certificate", cert_path);
    config::Set<std::string>("network.https.ssl_certificate_key", key_path);
    
    if (!config::Save()) {
        return Result<void>::Err("Failed to save configuration");
    }
    
    // Regenerate nginx config and reload
    if (!RegenerateNginxConfig()) {
        return Result<void>::Err("Webserver configuration changed - restart required");
    }
    
    return Result<void>::Ok();
}

Result<NetworkManager::SslCertificateInfo> NetworkManager::GetSslCertificateInfo() {
    SslCertificateInfo info;
    
    // Get certificate paths from config
    info.certificate_path = config::Get<std::string>("network.https.ssl_certificate", "/etc/nginx/ssl/Server.crt");
    info.key_path = config::Get<std::string>("network.https.ssl_certificate_key", "/etc/nginx/ssl/Server.key");
    
    // Open certificate file
    FILE* cert_file = fopen(info.certificate_path.c_str(), "r");
    if (!cert_file) {
        return Result<SslCertificateInfo>::Err("Certificate file not found: " + info.certificate_path);
    }
    
    // Read certificate using OpenSSL library
    X509* cert = PEM_read_X509(cert_file, nullptr, nullptr, nullptr);
    fclose(cert_file);
    
    if (!cert) {
        return Result<SslCertificateInfo>::Err("Failed to parse certificate");
    }
    
    // Extract Common Name from subject
    X509_NAME* subject = X509_get_subject_name(cert);
    if (subject) {
        char cn_buf[256] = {0};
        int len = X509_NAME_get_text_by_NID(subject, NID_commonName, cn_buf, sizeof(cn_buf));
        if (len > 0) {
            info.common_name = std::string(cn_buf, len);
        }
    }
    
    // Extract Issuer CN
    X509_NAME* issuer = X509_get_issuer_name(cert);
    if (issuer) {
        char issuer_buf[256] = {0};
        int len = X509_NAME_get_text_by_NID(issuer, NID_commonName, issuer_buf, sizeof(issuer_buf));
        if (len > 0) {
            info.issuer = std::string(issuer_buf, len);
        }
    }
    
    // Extract validity dates
    ASN1_TIME* not_before = X509_get_notBefore(cert);
    ASN1_TIME* not_after = X509_get_notAfter(cert);
    
    if (not_before) {
        BIO* bio = BIO_new(BIO_s_mem());
        ASN1_TIME_print(bio, not_before);
        char time_buf[256] = {0};
        int len = BIO_read(bio, time_buf, sizeof(time_buf) - 1);
        if (len > 0) {
            info.valid_from = std::string(time_buf, len);
        }
        BIO_free(bio);
    }
    
    if (not_after) {
        BIO* bio = BIO_new(BIO_s_mem());
        ASN1_TIME_print(bio, not_after);
        char time_buf[256] = {0};
        int len = BIO_read(bio, time_buf, sizeof(time_buf) - 1);
        if (len > 0) {
            info.valid_to = std::string(time_buf, len);
        }
        BIO_free(bio);
    }
    
    // Extract Subject Alternative Names (SAN)
    STACK_OF(GENERAL_NAME)* san_names = static_cast<STACK_OF(GENERAL_NAME)*>(
        X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr));
    
    if (san_names) {
        int san_count = sk_GENERAL_NAME_num(san_names);
        for (int i = 0; i < san_count; i++) {
            GENERAL_NAME* san = sk_GENERAL_NAME_value(san_names, i);
            if (san->type == GEN_DNS) {
                ASN1_STRING* dns_name = san->d.dNSName;
                if (dns_name && dns_name->data && dns_name->length > 0) {
                    info.subject_alt_names.push_back(
                        std::string(reinterpret_cast<const char*>(dns_name->data), dns_name->length)
                    );
                }
            }
        }
        GENERAL_NAMES_free(san_names);
    }
    
    // Check if self-signed (subject == issuer)
    info.is_self_signed = (info.common_name == info.issuer);
    
    X509_free(cert);
    
    return Result<SslCertificateInfo>::Ok(std::move(info));
}

Result<void> NetworkManager::GenerateSslCertificate(int validity_days, int key_bits) {
    // Get hostname for certificate CN (industry standard - not IP)
    auto hostname_result = GetHostname();
    std::string cn_name;
    if (hostname_result) {
        cn_name = hostname_result.value;
    } else {
        // Fallback to generic name if hostname unavailable
        cn_name = "IPCamera";
        spdlog::warn("Could not get hostname, using generic CN: {}", cn_name);
    }
    
    // Get current IP for SAN (optional, for direct IP access)
    std::string ip_address;
    auto ipv4_result = GetIpv4("eth0");
    if (ipv4_result && !ipv4_result.value.address.empty() && ipv4_result.value.address != "0.0.0.0") {
        ip_address = ipv4_result.value.address;
    }
    
    // Get certificate paths from config
    std::string cert_path = config::Get<std::string>("network.https.ssl_certificate", "/etc/nginx/ssl/Server.crt");
    std::string key_path = config::Get<std::string>("network.https.ssl_certificate_key", "/etc/nginx/ssl/Server.key");
    std::string ssl_dir = config::Get<std::string>("network.https.ssl_certs_dir", "/etc/nginx/ssl");
    
    // Ensure SSL directory exists
    std::string mkdir_cmd = "mkdir -p " + ssl_dir;
    system(mkdir_cmd.c_str());
    
    // Use temporary files for new certificates to avoid breaking existing setup
    std::string temp_cert = cert_path + ".tmp";
    std::string temp_key = key_path + ".tmp";
    std::string backup_cert = cert_path + ".bak";
    std::string backup_key = key_path + ".bak";
    
    // Generate RSA key pair using modern OpenSSL 3.0 EVP API
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        return Result<void>::Err("Failed to create EVP_PKEY_CTX");
    }
    
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return Result<void>::Err("Failed to initialize key generation");
    }
    
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, key_bits) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return Result<void>::Err("Failed to set RSA key bits");
    }
    
    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return Result<void>::Err("Failed to generate RSA key");
    }
    EVP_PKEY_CTX_free(ctx);
    
    // Create X509 certificate
    X509* x509 = X509_new();
    if (!x509) {
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to create X509 structure");
    }
    
    // Set certificate version (X509 v3)
    X509_set_version(x509, 2);
    
    // Set serial number
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    
    // Set validity period
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), validity_days * 24 * 3600);
    
    // Set public key
    X509_set_pubkey(x509, pkey);
    
    // Set subject name (CN = hostname, industry standard)
    X509_NAME* name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, 
                               (unsigned char*)cn_name.c_str(), -1, -1, 0);
    
    // Set issuer = subject (self-signed)
    X509_set_issuer_name(x509, name);
    
    // Add X509v3 extensions for proper browser compatibility
    X509_EXTENSION* ext = nullptr;
    X509V3_CTX x509v3_ctx;
    X509V3_set_ctx_nodb(&x509v3_ctx);
    X509V3_set_ctx(&x509v3_ctx, x509, x509, nullptr, nullptr, 0);
    
    // Add Basic Constraints (CA:FALSE for end-entity certificate)
    ext = X509V3_EXT_conf_nid(nullptr, &x509v3_ctx, NID_basic_constraints, 
                              const_cast<char*>("CA:FALSE"));
    if (ext) {
        X509_add_ext(x509, ext, -1);
        X509_EXTENSION_free(ext);
        ext = nullptr;
    } else {
        spdlog::warn("Failed to add Basic Constraints extension");
    }
    
    // Add Key Usage extension
    ext = X509V3_EXT_conf_nid(nullptr, &x509v3_ctx, NID_key_usage, 
                              const_cast<char*>("digitalSignature, keyEncipherment"));
    if (ext) {
        X509_add_ext(x509, ext, -1);
        X509_EXTENSION_free(ext);
        ext = nullptr;
    } else {
        spdlog::warn("Failed to add Key Usage extension");
    }
    
    // Add Subject Alternative Name extension
    // Include hostname.local for mDNS, localhost, and current IP if available
    // CRITICAL: This is required for browsers to accept the certificate
    std::string san_value = "DNS:" + cn_name + ",DNS:" + cn_name + ".local,DNS:localhost";
    if (!ip_address.empty()) {
        san_value += ",IP:" + ip_address;
    }
    spdlog::info("Generating SSL certificate with SAN: {}", san_value);
    
    ext = X509V3_EXT_conf_nid(nullptr, &x509v3_ctx, NID_subject_alt_name, 
                              const_cast<char*>(san_value.c_str()));
    if (ext) {
        if (X509_add_ext(x509, ext, -1) != 1) {
            spdlog::error("Failed to add SAN extension to certificate");
        } else {
            spdlog::debug("Successfully added SAN extension to certificate");
        }
        X509_EXTENSION_free(ext);
        ext = nullptr;
    } else {
        spdlog::error("Failed to create SAN extension - this will cause browser certificate warnings!");
        // Log OpenSSL error
        unsigned long err = ERR_get_error();
        if (err != 0) {
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            spdlog::error("OpenSSL error: {}", err_buf);
        }
    }
    
    // Sign certificate with private key
    if (X509_sign(x509, pkey, EVP_sha256()) == 0) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to sign certificate");
    }
    
    // Write private key to temporary file first (safe approach)
    FILE* key_file = fopen(temp_key.c_str(), "wb");
    if (!key_file) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to open temporary key file for writing: " + temp_key);
    }
    
    if (PEM_write_PrivateKey(key_file, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1) {
        fclose(key_file);
        unlink(temp_key.c_str());  // Clean up failed temporary file
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to write private key");
    }
    fclose(key_file);
    
    // Write certificate to temporary file first (safe approach)
    FILE* cert_file = fopen(temp_cert.c_str(), "wb");
    if (!cert_file) {
        unlink(temp_key.c_str());  // Clean up temporary key file
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to open temporary certificate file for writing: " + temp_cert);
    }
    
    if (PEM_write_X509(cert_file, x509) != 1) {
        fclose(cert_file);
        unlink(temp_cert.c_str());  // Clean up failed temporary file
        unlink(temp_key.c_str());   // Clean up temporary key file
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to write certificate");
    }
    fclose(cert_file);
    
    // Now that new certificates are successfully written, safely replace old ones
    // 1. Backup existing certificates if they exist (only if not already backed up)
    std::ifstream cert_check(cert_path);
    if (cert_check.good()) {
        cert_check.close();
        // Move (not copy) existing certificates to backup
        rename(cert_path.c_str(), backup_cert.c_str());
        rename(key_path.c_str(), backup_key.c_str());
    }
    
    // 2. Move new certificates into place (atomic operation on most filesystems)
    if (rename(temp_cert.c_str(), cert_path.c_str()) != 0) {
        // Restore backup if rename fails
        rename(backup_cert.c_str(), cert_path.c_str());
        rename(backup_key.c_str(), key_path.c_str());
        unlink(temp_cert.c_str());
        unlink(temp_key.c_str());
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to install new certificate");
    }
    
    if (rename(temp_key.c_str(), key_path.c_str()) != 0) {
        // Restore backup if rename fails
        rename(backup_cert.c_str(), cert_path.c_str());
        rename(backup_key.c_str(), key_path.c_str());
        unlink(temp_key.c_str());
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to install new key");
    }
    
    // Also copy to nginx SSL directory for both nginx and go2rtc
    std::string nginx_ssl_dir = "/etc/nginx/ssl";
    system(("mkdir -p " + nginx_ssl_dir).c_str());
    system(("cp -f " + cert_path + " " + nginx_ssl_dir + "/Server.crt").c_str());
    system(("cp -f " + key_path + " " + nginx_ssl_dir + "/Server.key").c_str());
    system(("chmod 600 " + nginx_ssl_dir + "/Server.key").c_str());
    
    // Clean up
    X509_free(x509);
    EVP_PKEY_free(pkey);
    
    spdlog::info("Generated new SSL certificate for CN: {} (SANs: {})", cn_name, san_value);
    
    // Reload nginx to use new certificate
    if (!RegenerateNginxConfig()) {
        spdlog::warn("Failed to reload nginx after certificate generation");
    }
    
    // Restart go2rtc to pick up new certificate (it caches SSL context)
    spdlog::info("Restarting go2rtc to use new certificate...");
    int ret = system("pkill -HUP go2rtc_linux_arm64 2>/dev/null || (pkill go2rtc_linux_arm64 && sleep 1 && /etc/init.d/S51go2rtc start) 2>/dev/null");
    if (ret != 0) {
        spdlog::warn("Could not restart go2rtc (may need manual restart)");
    }
    
    return Result<void>::Ok();
}

Result<int> NetworkManager::GetRtspPort() {
    int port = config::Get<int>("network.rtsp.port", 554);
    return Result<int>::Ok(port);
}

Result<void> NetworkManager::SetRtspPort(int port) {
    if (!ValidatePort(port)) {
        return Result<void>::Err("Invalid port number (must be 1-65535)");
    }
    
    config::Set<int>("network.rtsp.port", port);
    if (!config::Save()) {
        return Result<void>::Err("Failed to save configuration");
    }
    
    return Result<void>::Ok();
}

Result<int> NetworkManager::GetOnvifPort() {
    int port = config::Get<int>("network.onvif.port", 8080);
    return Result<int>::Ok(port);
}

Result<void> NetworkManager::SetOnvifPort(int port) {
    if (!ValidatePort(port)) {
        return Result<void>::Err("Invalid port number (must be 1-65535)");
    }
    
    config::Set<int>("network.onvif.port", port);
    if (!config::Save()) {
        return Result<void>::Err("Failed to save configuration");
    }
    
    return Result<void>::Ok();
}

// WiFi Management (stub implementations for now - platform specific)
Result<std::vector<NetworkManager::WifiNetwork>> NetworkManager::ScanWifi() {
    // TODO: Implement using wpa_cli or similar
    return Result<std::vector<WifiNetwork>>::Err("WiFi scanning not implemented");
}

Result<void> NetworkManager::ConnectWifi(const std::string& ssid, const std::string& psk) {
    (void)ssid;
    (void)psk;
    // TODO: Implement using wpa_cli or similar
    return Result<void>::Err("WiFi connection not implemented");
}

Result<void> NetworkManager::ForgetWifi(const std::string& ssid) {
    (void)ssid;
    // TODO: Implement using wpa_cli or similar
    return Result<void>::Err("WiFi forget not implemented");
}

Result<bool> NetworkManager::GetWifiPower() {
    // TODO: Implement
    return Result<bool>::Err("WiFi power status not implemented");
}

Result<void> NetworkManager::SetWifiPower(bool on) {
    (void)on;
    // TODO: Implement
    return Result<void>::Err("WiFi power control not implemented");
}

// ============================================================================
// Hostname Configuration
// ============================================================================

Result<std::string> NetworkManager::GenerateHostnameFromMac(const std::string& prefix, const std::string& iface) {
    auto mac_result = GetMac(iface);
    if (!mac_result.success) {
        return Result<std::string>::Err("Failed to get MAC address: " + mac_result.error);
    }
    
    // Get last 6 characters of MAC (without colons)
    std::string mac = mac_result.value;
    std::string mac_suffix;
    for (char c : mac) {
        if (c != ':') {
            mac_suffix += c;
        }
    }
    
    // Use last 6 hex digits
    if (mac_suffix.length() >= 6) {
        mac_suffix = mac_suffix.substr(mac_suffix.length() - 6);
    }
    
    // Convert to lowercase for consistency
    for (char& c : mac_suffix) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    
    std::string hostname = prefix + "-" + mac_suffix;
    spdlog::info("Generated hostname from MAC: {}", hostname);
    return Result<std::string>::Ok(hostname);
}

Result<NetworkManager::HostnameConfig> NetworkManager::GetHostnameConfig() {
    HostnameConfig cfg;
    cfg.method = config::Get<std::string>("network.hostname.method", "auto");
    cfg.prefix = config::Get<std::string>("network.hostname.prefix", "ipcam");
    cfg.name = config::Get<std::string>("network.hostname.name", "ipcamera");
    cfg.interface = config::Get<std::string>("network.hostname.interface", "eth0");
    return Result<HostnameConfig>::Ok(cfg);
}

Result<void> NetworkManager::SetHostnameConfig(const HostnameConfig& cfg) {
    config::Set<std::string>("network.hostname.method", cfg.method);
    config::Set<std::string>("network.hostname.prefix", cfg.prefix);
    config::Set<std::string>("network.hostname.name", cfg.name);
    config::Set<std::string>("network.hostname.interface", cfg.interface);
    
    if (!config::Save()) {
        return Result<void>::Err("Failed to save hostname configuration");
    }
    
    // Apply the hostname
    auto hostname_result = GetHostname();
    if (hostname_result.success) {
        std::string cmd = "hostname " + hostname_result.value;
        system(cmd.c_str());
        
        // Update /etc/hostname
        std::ofstream hostname_file("/etc/hostname");
        if (hostname_file.is_open()) {
            hostname_file << hostname_result.value << "\n";
            hostname_file.close();
        }
        
        spdlog::info("Hostname set to: {}", hostname_result.value);
    }
    
    return Result<void>::Ok();
}

Result<std::string> NetworkManager::GetHostname() {
    std::string method = config::Get<std::string>("network.hostname.method", "auto");
    
    if (method == "auto") {
        // Generate from MAC address
        std::string prefix = config::Get<std::string>("network.hostname.prefix", "ipcam");
        std::string iface = config::Get<std::string>("network.hostname.interface", "eth0");
        
        auto gen_result = GenerateHostnameFromMac(prefix, iface);
        if (gen_result.success) {
            return gen_result;
        }
        // Fall back to manual name if MAC generation fails
        spdlog::warn("Failed to generate hostname from MAC, using manual: {}", gen_result.error);
    }
    
    // Manual mode or fallback
    std::string hostname = config::Get<std::string>("network.hostname.name", "ipcamera");
    return Result<std::string>::Ok(hostname);
}

Result<void> NetworkManager::SetHostname(const std::string& hostname) {
    if (hostname.empty()) {
        return Result<void>::Err("Hostname cannot be empty");
    }
    
    // Switch to manual mode when setting hostname directly
    config::Set<std::string>("network.hostname.method", "manual");
    config::Set<std::string>("network.hostname.name", hostname);
    
    if (!config::Save()) {
        return Result<void>::Err("Failed to save configuration");
    }
    
    // Set system hostname
    std::string cmd = "hostname " + hostname;
    system(cmd.c_str());
    
    // Update /etc/hostname
    std::ofstream hostname_file("/etc/hostname");
    if (hostname_file.is_open()) {
        hostname_file << hostname << "\n";
        hostname_file.close();
    }
    
    spdlog::info("Hostname manually set to: {}", hostname);
    return Result<void>::Ok();
}

// NTP Configuration
Result<NetworkManager::NtpConfig> NetworkManager::GetNtpConfig() {
    NtpConfig cfg;
    cfg.enabled = config::Get<bool>("system.time.ntp.enabled", true);
    cfg.server = config::Get<std::string>("system.time.ntp.server", "pool.ntp.org");
    cfg.port = config::Get<int>("system.time.ntp.port", 123);
    cfg.refresh_interval_sec = config::Get<int>("system.time.ntp.refresh_interval_sec", 3600);
    
    return Result<NtpConfig>::Ok(cfg);
}

Result<void> NetworkManager::SetNtpConfig(const NtpConfig& cfg) {
    config::Set<bool>("system.time.ntp.enabled", cfg.enabled);
    config::Set<std::string>("system.time.ntp.server", cfg.server);
    config::Set<int>("system.time.ntp.port", cfg.port);
    config::Set<int>("system.time.ntp.refresh_interval_sec", cfg.refresh_interval_sec);
    
    if (!config::Save()) {
        return Result<void>::Err("Failed to save NTP configuration");
    }
    
    return Result<void>::Ok();
}

Result<void> NetworkManager::SyncTimeNtp() {
    // Delegate to NtpManager
    auto result = NtpManager::Instance().SyncNow();
    
    if (result.status == NtpManager::SyncStatus::Success) {
        return Result<void>::Ok();
    } else if (result.status == NtpManager::SyncStatus::Disabled ||
               result.status == NtpManager::SyncStatus::NoInternet ||
               result.status == NtpManager::SyncStatus::AllServersFailed) {
        // These are not errors - camera works fine with RTC
        return Result<void>::Ok();
    }
    
    return Result<void>::Err(result.message);
}

bool NetworkManager::CheckInternetConnectivity() {
    // Delegate to NtpManager
    return NtpManager::Instance().CheckInternetConnectivity();
}

// Multicast Configuration
Result<NetworkManager::MulticastConfig> NetworkManager::GetMulticastConfig() {
    MulticastConfig cfg;
    cfg.enabled = config::Get<bool>("network.multicast.enabled", true);
    cfg.address = config::Get<std::string>("network.multicast.address", "239.255.255.255");
    cfg.port = config::Get<int>("network.multicast.port", 5353);
    cfg.ttl = config::Get<int>("network.multicast.ttl", 1);
    cfg.interface = config::Get<std::string>("network.multicast.interface", "eth0");
    cfg.loopback = config::Get<bool>("network.multicast.loopback", false);
    
    return Result<MulticastConfig>::Ok(cfg);
}

Result<void> NetworkManager::SetMulticastConfig(const MulticastConfig& cfg) {
    // Validate multicast address
    if (!cfg.address.empty() && !ValidateMulticastAddress(cfg.address)) {
        return Result<void>::Err("Invalid multicast address (must be 224.0.0.0-239.255.255.255)");
    }
    
    // Validate port
    if (cfg.port > 0 && !ValidatePort(cfg.port)) {
        return Result<void>::Err("Invalid port number (must be 1-65535)");
    }
    
    // Validate TTL
    if (cfg.ttl < 0 || cfg.ttl > 255) {
        return Result<void>::Err("Invalid TTL (must be 0-255)");
    }
    
    config::Set<bool>("network.multicast.enabled", cfg.enabled);
    config::Set<std::string>("network.multicast.address", cfg.address);
    config::Set<int>("network.multicast.port", cfg.port);
    config::Set<int>("network.multicast.ttl", cfg.ttl);
    config::Set<std::string>("network.multicast.interface", cfg.interface);
    config::Set<bool>("network.multicast.loopback", cfg.loopback);
    
    if (!config::Save()) {
        return Result<void>::Err("Failed to save multicast configuration");
    }
    
    return Result<void>::Ok();
}

Result<void> NetworkManager::JoinMulticastGroup(const std::string& multicast_addr, const std::string& iface) {
    if (multicast_addr.empty()) {
        return Result<void>::Err("Multicast address cannot be empty");
    }
    
    if (!ValidateMulticastAddress(multicast_addr)) {
        return Result<void>::Err("Invalid multicast address");
    }
    
    if (iface.empty()) {
        return Result<void>::Err("Interface name cannot be empty");
    }
    
    // Use ip command to add multicast route
    std::ostringstream cmd;
    cmd << "ip route add " << multicast_addr << " dev " << iface;
    
    int rc = system(cmd.str().c_str());
    if (rc != 0) {
        // Route might already exist, which is okay
        spdlog::warn("Failed to add multicast route (may already exist): {}", multicast_addr);
    }
    
    return Result<void>::Ok();
}

Result<void> NetworkManager::LeaveMulticastGroup(const std::string& multicast_addr, const std::string& iface) {
    if (multicast_addr.empty()) {
        return Result<void>::Err("Multicast address cannot be empty");
    }
    
    if (!ValidateMulticastAddress(multicast_addr)) {
        return Result<void>::Err("Invalid multicast address");
    }
    
    if (iface.empty()) {
        return Result<void>::Err("Interface name cannot be empty");
    }
    
    // Use ip command to remove multicast route
    std::ostringstream cmd;
    cmd << "ip route del " << multicast_addr << " dev " << iface;
    
    int rc = system(cmd.str().c_str());
    if (rc != 0) {
        return Result<void>::Err("Failed to remove multicast route");
    }
    
    return Result<void>::Ok();
}

// Network Initialization
Result<void> NetworkManager::InitializeNetwork() {
    int rc;
    
    // Step 0: Configure hostname (before network, for DHCP hostname option)
    auto hostname_result = GetHostname();
    if (hostname_result.success) {
        std::string cmd_hostname = "hostname " + hostname_result.value;
        rc = system(cmd_hostname.c_str());
        if (rc == 0) {
            spdlog::info("System hostname set to: {}", hostname_result.value);
            
            // Also update /etc/hostname
            std::ofstream hostname_file("/etc/hostname");
            if (hostname_file.is_open()) {
                hostname_file << hostname_result.value << "\n";
                hostname_file.close();
            }
        } else {
            spdlog::warn("Failed to set hostname");
        }
    }
    
    // Step 1: Initialize loopback interface
    bool loopback_enabled = config::Get<bool>("network.interfaces.loopback.enabled", true);
    if (loopback_enabled) {
        std::string lo_address = config::Get<std::string>("network.interfaces.loopback.address", "127.0.0.1");
        std::string cmd_lo = "ifconfig lo " + lo_address;
        rc = system(cmd_lo.c_str());
        if (rc != 0) {
            return Result<void>::Err("Failed to configure loopback interface");
        }
    }
    
    // Step 2: Get primary interface configuration
    std::string primary_iface = config::Get<std::string>("network.ipv4.interface", "eth0");
    bool bring_up = config::Get<bool>("network.interfaces." + primary_iface + ".bring_up_at_boot", true);
    
    if (bring_up) {
        // Bring up interface
        std::string cmd_up = "ifconfig " + primary_iface + " up";
        rc = system(cmd_up.c_str());
        if (rc != 0) {
            return Result<void>::Err("Failed to bring up interface " + primary_iface);
        }
    }
    
    // Step 3: Configure IP based on method (DHCP or static)
    std::string method = config::Get<std::string>("network.ipv4.method", "dhcp");
    
    if (method == "dhcp") {
        // Get DHCP configuration from ipv4.dhcp_options
        std::string pid_file = config::Get<std::string>("network.ipv4.dhcp_options.pid_file", "/var/run/udhcpc_eth0.pid");
        int timeout = config::Get<int>("network.ipv4.dhcp_options.timeout", 2);
        int retry_count = config::Get<int>("network.ipv4.dhcp_options.retry_count", 1);
        bool send_hostname = config::Get<bool>("network.ipv4.dhcp_options.send_hostname", true);
        
        // Kill any existing udhcpc instance for this interface to prevent duplicates
        std::ostringstream kill_cmd;
        kill_cmd << "killall -q udhcpc 2>/dev/null || true";
        system(kill_cmd.str().c_str());
        
        // Build udhcpc command - start directly in background mode for instant startup
        std::ostringstream dhcp_cmd;
        dhcp_cmd << "udhcpc -i " << primary_iface 
                << " -p " << pid_file
                << " -T " << timeout
                << " -t " << retry_count
                << " -b";  // Background mode immediately, fallback if no lease
        
        if (send_hostname) {
            // Use the auto-generated or manual hostname (already computed in Step 0)
            auto hostname_for_dhcp = GetHostname();
            std::string hostname = hostname_for_dhcp.success ? hostname_for_dhcp.value : "ipcamera";
            dhcp_cmd << " -x hostname:" << hostname;
        }
        
        // Check if fallback is configured - if so, add script option
        bool fallback_enabled = config::Get<bool>("network.ipv4.fallback.enabled", false);
        if (fallback_enabled) {
            std::string fallback_addr = config::Get<std::string>("network.ipv4.fallback.address", "169.254.1.100");
            std::string fallback_mask = config::Get<std::string>("network.ipv4.fallback.netmask", "255.255.0.0");
            
            // Store fallback config for udhcpc script to use if needed
            config::Set<std::string>("network.ipv4.dhcp_options.fallback_addr", fallback_addr);
            config::Set<std::string>("network.ipv4.dhcp_options.fallback_mask", fallback_mask);
        }
        
        // Start udhcpc in background - it will handle retries and renewals
        rc = system(dhcp_cmd.str().c_str());
        
        if (rc != 0) {
            LOG_ERROR("Failed to start udhcpc");
            
            // Apply fallback immediately if enabled
            if (fallback_enabled) {
                std::string fallback_addr = config::Get<std::string>("network.ipv4.fallback.address", "169.254.1.100");
                std::string fallback_mask = config::Get<std::string>("network.ipv4.fallback.netmask", "255.255.0.0");
                
                LOG_WARNING("Applying fallback IP: {}", fallback_addr);
                
                std::ostringstream fallback_cmd;
                fallback_cmd << "ifconfig " << primary_iface << " " << fallback_addr 
                            << " netmask " << fallback_mask;
                rc = system(fallback_cmd.str().c_str());
                
                if (rc != 0) {
                    return Result<void>::Err("Failed to start DHCP client and fallback IP configuration failed");
                }
                
                LOG_INFO("Fallback IP configured: {}/{}", fallback_addr, fallback_mask);
            } else {
                return Result<void>::Err("Failed to start DHCP client and no fallback configured");
            }
        } else {
            LOG_INFO("DHCP client started in background mode");
        }
    } else if (method == "static" || method == "manual") {
        // Static IP configuration
        std::string address = config::Get<std::string>("network.ipv4.address", "");
        std::string netmask = config::Get<std::string>("network.ipv4.netmask", "255.255.255.0");
        
        if (address.empty()) {
            return Result<void>::Err("Static IP address not configured");
        }
        
        if (!ValidateIpv4Address(address)) {
            return Result<void>::Err("Invalid static IP address");
        }
        
        // Configure interface with static IP
        std::ostringstream ip_cmd;
        ip_cmd << "ifconfig " << primary_iface << " " << address 
               << " netmask " << netmask;
        rc = system(ip_cmd.str().c_str());
        if (rc != 0) {
            return Result<void>::Err("Failed to configure static IP address");
        }
    }
    
    // Step 4: Configure default gateway
    std::string gateway = config::Get<std::string>("network.ipv4.gateway", "");
    if (!gateway.empty()) {
        if (!ValidateIpv4Address(gateway)) {
            return Result<void>::Err("Invalid gateway address");
        }
        
        std::ostringstream gw_cmd;
        gw_cmd << "route add default gw " << gateway << " " << primary_iface;
        rc = system(gw_cmd.str().c_str());
        // Don't fail if gateway already exists
        // if (rc != 0) {
        //     return Result<void>::Err("Failed to set default gateway");
        // }
    }
    
    // Step 5: Configure DNS servers
    std::string dns_method = config::Get<std::string>("network.dns.method", "manual");
    if (dns_method == "manual") {
        std::string dns1 = config::Get<std::string>("network.dns.dns1", "");
        std::string dns2 = config::Get<std::string>("network.dns.dns2", "");
        
        if (!dns1.empty()) {
            DnsConfig dns_cfg;
            dns_cfg.primary = dns1;
            dns_cfg.secondary = dns2;
            
            auto dns_result = SetDns(dns_cfg);
            if (!dns_result.success) {
                // Don't fail initialization, just log error
                // return dns_result;
            }
        }
    }
    
    // Step 5.5: Apply MTU configuration if different from default
    int configured_mtu = config::Get<int>("network.ipv4.mtu", 1500);
    if (configured_mtu != 1500) {
        MtuConfig mtu_cfg;
        mtu_cfg.mtu = configured_mtu;
        mtu_cfg.path_mtu_discovery = config::Get<bool>("network.ipv4.path_mtu_discovery", true);
        auto mtu_result = SetMtu(primary_iface, mtu_cfg);
        if (!mtu_result.success) {
            spdlog::warn("Failed to set MTU to {}: {}", configured_mtu, mtu_result.error);
        }
    }
    
    // Step 5.6: Send gratuitous ARP to announce IP (for static IP, DHCP script handles its own)
    if (method == "static" || method == "manual") {
        SendGratuitousArp(primary_iface);
    }
    
    // Step 6: Initialize SSL certificates and nginx configuration
    // This will:
    // - Check for existing certificates
    // - Generate new ones if needed (based on current IP)
    // - Generate nginx configuration
    // Initialize SSL certificates and nginx configuration
    // These are now decoupled - SSL manager handles certificates, nginx manager handles web server
    spdlog::info("Initializing SSL certificates...");
    auto& ssl_mgr = SslManager::Instance();
    auto ssl_result = ssl_mgr.InitializeCertificates();
    if (!ssl_result.success) {
        spdlog::warn("Failed to initialize SSL certificates: {}", ssl_result.error);
        // Don't fail network initialization due to SSL issues
    }
    
    // Generate nginx config (always, independent of SSL)
    spdlog::info("Generating nginx configuration...");
    auto& nginx_mgr = NginxManager::Instance();
    auto nginx_result = nginx_mgr.GenerateConfig();
    if (!nginx_result.success) {
        spdlog::warn("Failed to generate nginx config: {}", nginx_result.error);
    } else {
        // Check if nginx should be enabled
        bool http_enabled = config::Get<bool>("network.http.enabled", true);
        bool https_enabled = config::Get<bool>("network.https.enabled", true);
        
        if (http_enabled || https_enabled) {
            // If nginx is running, reload it. Otherwise, start it.
            if (nginx_mgr.IsRunning()) {
                auto reload_result = nginx_mgr.Reload();
                if (!reload_result.success) {
                    spdlog::warn("Failed to reload nginx: {}", reload_result.error);
                }
            } else {
                spdlog::info("Starting nginx web server...");
                auto start_result = nginx_mgr.Start();
                if (!start_result.success) {
                    spdlog::warn("Failed to start nginx: {}", start_result.error);
                } else {
                    spdlog::info("nginx started successfully");
                }
            }
        } else {
            spdlog::info("HTTP/HTTPS disabled, nginx will not be started");
        }
    }
    
    // Step 7: Initialize and start NTP time synchronization
    spdlog::info("Initializing NTP manager...");
    auto& ntp_mgr = NtpManager::Instance();
    auto ntp_init_result = ntp_mgr.Initialize();
    if (!ntp_init_result.success) {
        spdlog::warn("Failed to initialize NTP manager: {}", ntp_init_result.error);
    } else {
        // Start periodic sync (does initial sync + background refresh)
        ntp_mgr.StartPeriodicSync();
    }
    
    // Step 8: Initialize mDNS responder for .local hostname resolution
    bool mdns_enabled = config::Get<bool>("network.mdns.enabled", true);
    if (mdns_enabled) {
        spdlog::info("Initializing mDNS responder...");
        auto& mdns = MdnsResponder::GetInstance();
        
        // Set hostname from config
        if (hostname_result.success) {
            mdns.SetHostname(hostname_result.value);
        }
        
        // Set interface
        mdns.SetInterface(primary_iface);
        
        // Add services based on config
        std::string service_name = hostname_result.success ? hostname_result.value : "ipcamera";
        
        // HTTP web interface
        bool http_service_enabled = config::Get<bool>("network.mdns.services.http", true);
        bool http_enabled = config::Get<bool>("network.http.enabled", true);
        if (http_service_enabled && http_enabled) {
            int http_port = config::Get<int>("network.http.port", 80);
            MdnsService http_service;
            http_service.name = service_name;
            http_service.type = "_http._tcp";
            http_service.port = static_cast<uint16_t>(http_port);
            http_service.txt_records.push_back("path=/");
            mdns.AddService(http_service);
            spdlog::info("mDNS: Added HTTP service on port {}", http_port);
        }
        
        // HTTPS web interface
        bool https_service_enabled = config::Get<bool>("network.mdns.services.https", true);
        bool https_enabled = config::Get<bool>("network.https.enabled", true);
        if (https_service_enabled && https_enabled) {
            int https_port = config::Get<int>("network.https.port", 443);
            MdnsService https_service;
            https_service.name = service_name;
            https_service.type = "_https._tcp";
            https_service.port = static_cast<uint16_t>(https_port);
            https_service.txt_records.push_back("path=/");
            mdns.AddService(https_service);
            spdlog::info("mDNS: Added HTTPS service on port {}", https_port);
        }
        
        // RTSP streaming
        bool rtsp_service_enabled = config::Get<bool>("network.mdns.services.rtsp", true);
        bool rtsp_enabled = config::Get<bool>("network.rtsp.enabled", true);
        if (rtsp_service_enabled && rtsp_enabled) {
            int rtsp_port = config::Get<int>("network.rtsp.port", 554);
            MdnsService rtsp_service;
            rtsp_service.name = service_name;
            rtsp_service.type = "_rtsp._tcp";
            rtsp_service.port = static_cast<uint16_t>(rtsp_port);
            mdns.AddService(rtsp_service);
            spdlog::info("mDNS: Added RTSP service on port {}", rtsp_port);
        }
        
        // ONVIF service discovery
        bool onvif_service_enabled = config::Get<bool>("network.mdns.services.onvif", true);
        bool onvif_enabled = config::Get<bool>("network.onvif.enabled", true);
        if (onvif_service_enabled && onvif_enabled) {
            int onvif_port = config::Get<int>("network.onvif.port", 80);
            MdnsService onvif_service;
            onvif_service.name = service_name;
            onvif_service.type = "_onvif._tcp";
            onvif_service.port = static_cast<uint16_t>(onvif_port);
            onvif_service.txt_records.push_back("txtvers=1");
            onvif_service.txt_records.push_back("Scopes=onvif://www.onvif.org/type/Network_Video_Transmitter");
            mdns.AddService(onvif_service);
            spdlog::info("mDNS: Added ONVIF service on port {}", onvif_port);
        }
        
        // Start the mDNS responder
        if (mdns.Start()) {
            // Initial announcement
            mdns.Announce();
            spdlog::info("mDNS responder started for {}.local", service_name);
        } else {
            spdlog::warn("Failed to start mDNS responder");
        }
    } else {
        spdlog::info("mDNS responder disabled by config");
    }
    
    spdlog::info("Network initialized successfully");
    return Result<void>::Ok();
}

// Generate nginx configuration file
Result<void> NetworkManager::GenerateNginxConfig(const std::string& output_path) {
    return NginxManager::Instance().GenerateConfig(output_path);
}

// Reload nginx server
Result<void> NetworkManager::ReloadNginx() {
    return NginxManager::Instance().Reload();
}


// Initialize SSL certificates on boot
Result<void> NetworkManager::InitializeSslCertificates() {
    spdlog::info("Initializing SSL certificates...");
    
    // Get SSL certificate paths from config
    std::string cert_path = config::Get<std::string>("network.https.ssl_certificate", "/etc/nginx/ssl/Server.crt");
    std::string key_path = config::Get<std::string>("network.https.ssl_certificate_key", "/etc/nginx/ssl/Server.key");
    std::string ssl_dir = config::Get<std::string>("network.https.ssl_certs_dir", "/etc/nginx/ssl");
    
    // Ensure SSL directory exists
    std::string mkdir_cmd = "mkdir -p " + ssl_dir;
    system(mkdir_cmd.c_str());
    
    bool need_new_cert = false;
    std::string reason;
    
    // Check if certificate files exist and are readable
    // Regenerate if missing, corrupted, or CN doesn't match current hostname
    std::ifstream cert_file(cert_path);
    std::ifstream key_file(key_path);
    
    if (!cert_file.good() || !key_file.good()) {
        need_new_cert = true;
        reason = "Certificate or key file missing (first boot)";
    } else {
        // Verify certificate is valid/readable
        auto cert_info_result = GetSslCertificateInfo();
        if (!cert_info_result) {
            need_new_cert = true;
            reason = "Existing certificate is invalid or corrupted";
        } else {
            // Check if CN matches current hostname
            auto hostname_result = GetHostname();
            std::string current_hostname = hostname_result.success ? hostname_result.value : "ipcamera";
            std::string cert_cn = cert_info_result.value.common_name;
            
            if (cert_cn != current_hostname) {
                need_new_cert = true;
                reason = "Certificate CN (" + cert_cn + ") doesn't match hostname (" + current_hostname + ")";
                spdlog::info("SSL certificate CN mismatch detected, will regenerate");
            } else {
                // Certificate exists, valid, and CN matches - use it
                spdlog::info("SSL certificates already exist with correct CN: {}", cert_cn);
            }
        }
    }
    
    // Generate new certificate only if needed (first boot or corrupted)
    if (need_new_cert) {
        spdlog::info("Generating new SSL certificate: {}", reason);
        
        auto gen_result = GenerateSslCertificate(3650, 2048);  // 10 year validity
        if (!gen_result) {
            spdlog::error("Failed to generate SSL certificate: {}", gen_result.error);
            // Continue anyway - we'll use whatever certificate exists
        } else {
            spdlog::info("SSL certificate generated successfully");
        }
    }
    
    // Generate nginx configuration
    auto nginx_result = GenerateNginxConfig();
    if (!nginx_result) {
        return Result<void>::Err("Failed to generate nginx config: " + nginx_result.error);
    }
    
    // Reload nginx to apply changes
    auto reload_result = ReloadNginx();
    if (!reload_result) {
        return Result<void>::Err("Failed to reload nginx: " + reload_result.error);
    }
    
    spdlog::info("SSL certificates initialized successfully");
    return Result<void>::Ok();
}

} // namespace networking
} // namespace ipcam
