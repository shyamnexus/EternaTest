#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <thread>
#include <atomic>
#include "ipcam/result.h"
#include "ipcam/smtp_manager.h"
#include "ipcam/snmp_manager.h"
#include "ipcam/upnp_manager.h"

namespace ipcam {
namespace networking {

// Use the shared Result type from utils
using ipcam::Result;

/**
 * @brief NetworkManager - Protocol-agnostic network configuration
 * 
 * This class provides a clean C++ interface for network operations.
 * It is used by multiple frontends: Web API, ONVIF, Cloud API.
 * 
 * Design principle: Business logic lives here, not in protocol handlers.
 */
class NetworkManager {
public:
    // Singleton access
    static NetworkManager& Instance();
    
    // IPv4 Configuration
    struct Ipv4Config {
        std::string method;    // "dhcp" or "static"
        std::string address;   // e.g., "192.168.1.100"
        std::string netmask;   // e.g., "255.255.255.0"
        std::string gateway;   // e.g., "192.168.1.1"
    };
    
    /**
     * Get IPv4 configuration for an interface
     * @param iface Interface name (default: "eth0")
     * @return Result with Ipv4Config on success, error message on failure
     */
    Result<Ipv4Config> GetIpv4(const std::string& iface = "eth0");
    
    /**
     * Set IPv4 configuration for an interface
     * @param iface Interface name
     * @param cfg IPv4 configuration
     * @return Result indicating success or error
     */
    Result<void> SetIpv4(const std::string& iface, const Ipv4Config& cfg);
    
    // IPv6 Configuration
    struct Ipv6Config {
        bool enabled;           // IPv6 enabled/disabled
        std::string method;     // "auto" (SLAAC/DHCPv6), "static", "dhcp"
        std::string address;    // e.g., "2001:db8::1"
        int prefix_length;      // e.g., 64
        std::string gateway;    // e.g., "fe80::1" or "2001:db8::1"
        bool privacy_extensions; // RFC 4941 privacy extensions
        bool accept_ra;         // Accept Router Advertisements
    };
    
    /**
     * Get IPv6 configuration for an interface
     * @param iface Interface name (default: "eth0")
     * @return Result with Ipv6Config on success, error message on failure
     */
    Result<Ipv6Config> GetIpv6(const std::string& iface = "eth0");
    
    /**
     * Set IPv6 configuration for an interface
     * @param iface Interface name
     * @param cfg IPv6 configuration
     * @return Result indicating success or error
     */
    Result<void> SetIpv6(const std::string& iface, const Ipv6Config& cfg);
    
    // DNS Configuration
    struct DnsConfig {
        std::string primary;     // Primary DNS server (IPv4)
        std::string secondary;   // Secondary DNS server (IPv4)
        std::string primary_ipv6;   // Primary DNS server (IPv6)
        std::string secondary_ipv6; // Secondary DNS server (IPv6)
    };
    
    /**
     * Get DNS configuration
     * @return Result with DnsConfig on success
     */
    Result<DnsConfig> GetDns();
    
    /**
     * Set DNS configuration
     * @param cfg DNS configuration
     * @return Result indicating success or error
     */
    Result<void> SetDns(const DnsConfig& cfg);
    
    // MTU Configuration
    struct MtuConfig {
        int mtu;                  // MTU value (68-9000, default 1500)
        bool path_mtu_discovery;  // Enable Path MTU Discovery
    };
    
    /**
     * Get MTU configuration for an interface
     * @param iface Interface name (default: "eth0")
     * @return Result with MtuConfig on success
     */
    Result<MtuConfig> GetMtu(const std::string& iface = "eth0");
    
    /**
     * Set MTU configuration for an interface
     * @param iface Interface name
     * @param cfg MTU configuration
     * @return Result indicating success or error
     */
    Result<void> SetMtu(const std::string& iface, const MtuConfig& cfg);
    
    // Gratuitous ARP Configuration
    struct GratuitousArpConfig {
        bool enabled;             // Enable gratuitous ARP on IP change
        int count;                // Number of ARP announcements (default: 3)
        int interval_ms;          // Interval between announcements in ms (default: 500)
    };
    
    /**
     * Get gratuitous ARP configuration
     * @return Result with GratuitousArpConfig on success
     */
    Result<GratuitousArpConfig> GetGratuitousArpConfig();
    
    /**
     * Set gratuitous ARP configuration
     * @param cfg Gratuitous ARP configuration
     * @return Result indicating success or error
     */
    Result<void> SetGratuitousArpConfig(const GratuitousArpConfig& cfg);
    
    /**
     * Send gratuitous ARP announcement for an IP address
     * Updates ARP caches on switches/routers and helps detect IP conflicts
     * @param iface Interface name
     * @param ip_address IP address to announce (if empty, uses current IP)
     * @return Result indicating success or error
     */
    Result<void> SendGratuitousArp(const std::string& iface = "eth0", const std::string& ip_address = "");
    
    // MAC Address
    /**
     * Get MAC address of an interface
     * @param iface Interface name (default: "eth0")
     * @return Result with MAC address string (e.g., "00:11:22:33:44:55")
     */
    Result<std::string> GetMac(const std::string& iface = "eth0");
    
    // NIC Speed Configuration
    struct NicSpeed {
        int speed;      // Speed in Mbps (10, 100, 1000)
        int duplex;     // 0=half duplex, 1=full duplex
        int autoneg;    // 0=disabled, 1=enabled
    };
    
    /**
     * Get NIC speed configuration
     * @param iface Interface name (default: "eth0")
     * @return Result with NicSpeed on success
     */
    Result<NicSpeed> GetNicSpeed(const std::string& iface = "eth0");
    
    /**
     * Set NIC speed configuration
     * @param iface Interface name
     * @param cfg NIC speed configuration
     * @return Result indicating success or error
     */
    Result<void> SetNicSpeed(const std::string& iface, const NicSpeed& cfg);
    
    /**
     * Get supported NIC speeds
     * @param iface Interface name (default: "eth0")
     * @return Result with comma-separated supported speeds
     */
    Result<std::string> GetSupportedNicSpeeds(const std::string& iface = "eth0");
    
    // Port Configuration (reads/writes to config module)
    /**
     * Get HTTP server port
     * @return Result with port number
     */
    Result<int> GetHttpPort();
    
    /**
     * Set HTTP server port
     * @param port Port number (1-65535)
     * @return Result indicating success or error
     */
    Result<void> SetHttpPort(int port);
    
    /**
     * Get HTTPS server port
     * @return Result with port number
     */
    Result<int> GetHttpsPort();
    
    /**
     * Set HTTPS server port
     * @param port Port number (1-65535)
     * @return Result indicating success or error
     */
    Result<void> SetHttpsPort(int port);
    
    // SSL Certificate Management
    struct SslCertificateInfo {
        std::string certificate_path;
        std::string key_path;
        std::string common_name;
        std::string issuer;
        std::string valid_from;
        std::string valid_to;
        std::vector<std::string> subject_alt_names;
        bool is_self_signed;
    };
    
    /**
     * Update SSL certificate paths
     * @param cert_path Path to SSL certificate file
     * @param key_path Path to SSL private key file
     * @return Result indicating success or error
     */
    Result<void> SetSslCertificate(const std::string& cert_path, const std::string& key_path);
    
    /**
     * Get SSL certificate information
     * @return Result with SslCertificateInfo on success
     */
    Result<SslCertificateInfo> GetSslCertificateInfo();
    
    /**
     * Generate a new self-signed SSL certificate with the current IP address
     * @param validity_days Certificate validity in days (default: 365)
     * @param key_bits RSA key size in bits (default: 2048)
     * @return Result indicating success or error
     */
    Result<void> GenerateSslCertificate(int validity_days = 365, int key_bits = 2048);
    
    /**
     * Get RTSP server port
     * @return Result with port number
     */
    Result<int> GetRtspPort();
    
    /**
     * Set RTSP server port
     * @param port Port number (1-65535)
     * @return Result indicating success or error
     */
    Result<void> SetRtspPort(int port);
    
    /**
     * Get ONVIF server port
     * @return Result with port number
     */
    Result<int> GetOnvifPort();
    
    /**
     * Set ONVIF server port
     * @param port Port number (1-65535)
     * @return Result indicating success or error
     */
    Result<void> SetOnvifPort(int port);
    
    // WiFi Management
    struct WifiNetwork {
        std::string ssid;
        int signal_strength;  // Signal strength percentage (0-100)
        std::string security; // "open", "wep", "wpa", "wpa2"
        bool connected;       // Currently connected to this network
    };
    
    /**
     * Scan for available WiFi networks
     * @return Result with vector of WifiNetwork
     */
    Result<std::vector<WifiNetwork>> ScanWifi();
    
    /**
     * Connect to a WiFi network
     * @param ssid Network SSID
     * @param psk Pre-shared key (password)
     * @return Result indicating success or error
     */
    Result<void> ConnectWifi(const std::string& ssid, const std::string& psk);
    
    /**
     * Disconnect and forget a WiFi network
     * @param ssid Network SSID
     * @return Result indicating success or error
     */
    Result<void> ForgetWifi(const std::string& ssid);
    
    /**
     * Get WiFi power state
     * @return Result with true if WiFi is on, false if off
     */
    Result<bool> GetWifiPower();
    
    /**
     * Set WiFi power state
     * @param on true to turn on, false to turn off
     * @return Result indicating success or error
     */
    Result<void> SetWifiPower(bool on);
    
    // Hostname Configuration
    struct HostnameConfig {
        std::string method;      // "auto" (MAC-based) or "manual"
        std::string prefix;      // Prefix for auto-generated hostname (e.g., "ipcam")
        std::string name;        // Manual hostname or auto-generated result
        std::string interface;   // Interface to get MAC from (default: "eth0")
    };
    
    /**
     * Get hostname configuration
     * @return Result with HostnameConfig on success
     */
    Result<HostnameConfig> GetHostnameConfig();
    
    /**
     * Set hostname configuration
     * @param cfg Hostname configuration
     * @return Result indicating success or error
     */
    Result<void> SetHostnameConfig(const HostnameConfig& cfg);
    
    /**
     * Get system hostname (resolves auto if configured)
     * @return Result with hostname string
     */
    Result<std::string> GetHostname();
    
    /**
     * Set system hostname (switches to manual mode)
     * @param hostname New hostname
     * @return Result indicating success or error
     */
    Result<void> SetHostname(const std::string& hostname);
    
    /**
     * Generate hostname from MAC address
     * @param prefix Hostname prefix (e.g., "ipcam")
     * @param iface Interface to get MAC from (default: "eth0")
     * @return Result with generated hostname (e.g., "ipcam-a1b2c3")
     */
    Result<std::string> GenerateHostnameFromMac(const std::string& prefix, const std::string& iface = "eth0");
    
    // SMTP Configuration - using type from SmtpManager
    using SmtpConfig = SmtpManager::SmtpConfig;
    using EmailMessage = SmtpManager::EmailMessage;
    
    // SNMP Configuration - using type from SnmpManager
    using SnmpConfig = SnmpManager::SnmpConfig;
    
    // UPnP Configuration - using types from UpnpManager
    using UpnpConfig = UpnpManager::UpnpConfig;
    using UpnpDeviceInfo = UpnpManager::UpnpDeviceInfo;
    using UpnpPortMapping = UpnpManager::UpnpPortMapping;
    
    /**
     * Get SMTP configuration
     * @return Result with SmtpConfig on success
     */
    Result<SmtpConfig> GetSmtpConfig();
    
    /**
     * Set SMTP configuration
     * @param cfg SMTP configuration (password will be stored securely)
     * @return Result indicating success or error
     */
    Result<void> SetSmtpConfig(const SmtpConfig& cfg);
    
    /**
     * Test SMTP connection
     * @return Result indicating success or error
     */
    Result<void> TestSmtpConnection();
    
    /**
     * Send email using current SMTP configuration
     * @param email Email message to send
     * @return Result indicating success or error
     */
    Result<void> SendEmail(const EmailMessage& email);
    
    /**
     * Send test email to verify SMTP configuration
     * @param to_email Recipient email address
     * @return Result indicating success or error
     */
    Result<void> SendTestEmail(const std::string& to_email);
    
    // SNMP Configuration
    /**
     * Get SNMP configuration
     * @return Result with SnmpConfig on success
     */
    Result<SnmpConfig> GetSnmpConfig();
    
    /**
     * Set SNMP configuration
     * @param cfg SNMP configuration
     * @return Result indicating success or error
     */
    Result<void> SetSnmpConfig(const SnmpConfig& cfg);
    
    /**
     * Start SNMP daemon
     * @return Result indicating success or error
     */
    Result<void> StartSnmp();
    
    /**
     * Stop SNMP daemon
     * @return Result indicating success or error
     */
    Result<void> StopSnmp();
    
    /**
     * Restart SNMP daemon
     * @return Result indicating success or error
     */
    Result<void> RestartSnmp();
    
    /**
     * Check if SNMP daemon is running
     * @return Result with bool status
     */
    Result<bool> IsSnmpRunning();
    
    // UPnP Configuration
    /**
     * Get UPnP configuration
     * @return Result with UpnpConfig on success
     */
    Result<UpnpConfig> GetUpnpConfig();
    
    /**
     * Set UPnP configuration
     * @param cfg UPnP configuration
     * @return Result indicating success or error
     */
    Result<void> SetUpnpConfig(const UpnpConfig& cfg);
    
    /**
     * Discover UPnP devices
     * @param timeout_ms Discovery timeout in milliseconds
     * @return Result with UpnpDeviceInfo on success
     */
    Result<UpnpDeviceInfo> DiscoverUpnp(int timeout_ms = 2000);
    
    /**
     * Add UPnP port mapping
     * @param mapping Port mapping configuration
     * @return Result indicating success or error
     */
    Result<void> AddUpnpPortMapping(const UpnpPortMapping& mapping);
    
    /**
     * Delete UPnP port mapping
     * @param external_port External port number
     * @param protocol Protocol ("TCP" or "UDP")
     * @return Result indicating success or error
     */
    Result<void> DeleteUpnpPortMapping(int external_port, const std::string& protocol);
    
    /**
     * Get all UPnP port mappings
     * @return Result with vector of port mappings on success
     */
    Result<std::vector<UpnpPortMapping>> GetUpnpPortMappings();
    
    // NTP Configuration
    struct NtpConfig {
        bool enabled;             // NTP enabled
        std::string server;       // NTP server address
        int port;                 // NTP port (default 123)
        int refresh_interval_sec; // Refresh interval
    };
    
    /**
     * Get NTP configuration
     * @return Result with NtpConfig on success
     */
    Result<NtpConfig> GetNtpConfig();
    
    /**
     * Set NTP configuration
     * @param cfg NTP configuration
     * @return Result indicating success or error
     */
    Result<void> SetNtpConfig(const NtpConfig& cfg);
    
    /**
     * Sync time with NTP server
     * Checks internet connectivity first, tries multiple servers with timeout
     * @return Result indicating success or error (gracefully handles no internet)
     */
    Result<void> SyncTimeNtp();
    
    /**
     * Check if internet connectivity is available
     * @return true if internet is reachable, false otherwise
     */
    bool CheckInternetConnectivity();
    
    // Multicast Configuration
    struct MulticastConfig {
        bool enabled;             // Multicast enabled
        std::string address;      // Multicast address (e.g., "239.255.255.255")
        int port;                 // Multicast port
        int ttl;                  // Time-to-live (hop limit)
        std::string interface;    // Network interface to use
        bool loopback;            // Enable multicast loopback
    };
    
    /**
     * Get multicast configuration
     * @return Result with MulticastConfig on success
     */
    Result<MulticastConfig> GetMulticastConfig();
    
    /**
     * Set multicast configuration
     * @param cfg Multicast configuration
     * @return Result indicating success or error
     */
    Result<void> SetMulticastConfig(const MulticastConfig& cfg);
    
    /**
     * Join a multicast group
     * @param multicast_addr Multicast address to join
     * @param iface Interface name
     * @return Result indicating success or error
     */
    Result<void> JoinMulticastGroup(const std::string& multicast_addr, const std::string& iface = "eth0");
    
    /**
     * Leave a multicast group
     * @param multicast_addr Multicast address to leave
     * @param iface Interface name
     * @return Result indicating success or error
     */
    Result<void> LeaveMulticastGroup(const std::string& multicast_addr, const std::string& iface = "eth0");
    
    // Helper functions (public for backward compatibility)
    /**
     * Validate IPv4 address format
     * @param addr IPv4 address string
     * @return true if valid IPv4 address
     */
    bool ValidateIpv4Address(const std::string& addr);
    
    /**
     * Validate IPv6 address format
     * @param addr IPv6 address string
     * @return true if valid IPv6 address
     */
    bool ValidateIpv6Address(const std::string& addr);
    
    /**
     * Validate multicast address format
     * @param addr Multicast address string (IPv4: 224.0.0.0-239.255.255.255)
     * @return true if valid multicast address
     */
    bool ValidateMulticastAddress(const std::string& addr);
    
    /**
     * Validate port number
     * @param port Port number
     * @return true if port is in range 1-65535
     */
    bool ValidatePort(int port);
    
    /**
     * Initialize network interfaces based on configuration
     * This method should be called during system startup to:
     * - Configure loopback interface
     * - Bring up network interfaces
     * - Start DHCP client or configure static IP
     * - Configure DNS servers
     * - Set default gateway
     * 
     * @return Result indicating success or error with details
     */
    Result<void> InitializeNetwork();
    
    /**
     * Generate nginx configuration file from current network settings
     * @param output_path Path to output nginx.conf (default: /usr/conf/nginx.conf)
     * @return Result indicating success or error
     */
    Result<void> GenerateNginxConfig(const std::string& output_path = "/usr/conf/nginx.conf");
    
    /**
     * Reload nginx server to apply configuration changes
     * @return Result indicating success or error
     */
    Result<void> ReloadNginx();
    
    /**
     * Initialize SSL certificates on boot - generate if needed based on DHCP IP
     * This method:
     * - Checks for existing valid certificate
     * - If missing or invalid, generates new self-signed cert using current IP
     * - Generates nginx config with proper SSL settings
     * - Reloads nginx
     * 
     * @return Result indicating success or error
     */
    Result<void> InitializeSslCertificates();
    
private:
    NetworkManager();
    ~NetworkManager();
    
    // Non-copyable
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
    
    // Private manager instances (using standalone managers)
    std::unique_ptr<SmtpManager> smtp_manager_;
    std::unique_ptr<SnmpManager> snmp_manager_;
    std::unique_ptr<UpnpManager> upnp_manager_;

    // Link monitoring
    void StartLinkMonitor();
    void StopLinkMonitor();
    void LinkMonitorThread();
    
    std::thread link_monitor_thread_;
    std::atomic<bool> link_monitor_running_{false};
    bool last_link_state_{false};
};

} // namespace networking
} // namespace ipcam
