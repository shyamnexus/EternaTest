/**
 * @file network_wrapper.h
 * @brief C wrapper for C++ Network Manager functions for ONVIF integration
 * 
 * Provides C-callable functions to interact with the ipcam::networking::NetworkManager
 * class from ONVIF C code.
 */

#ifndef ONVIF_NETWORK_WRAPPER_H
#define ONVIF_NETWORK_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// IPv4 Configuration
// ============================================================================
typedef struct {
    char method[16];        // "dhcp" or "static"
    char address[16];       // e.g., "192.168.1.100"
    char netmask[16];       // e.g., "255.255.255.0"
    char gateway[16];       // e.g., "192.168.1.1"
} OnvifIpv4Config;

// ============================================================================
// DNS Configuration
// ============================================================================
typedef struct {
    char primary[16];       // Primary DNS (IPv4)
    char secondary[16];     // Secondary DNS (IPv4)
    char primary_ipv6[46];  // Primary DNS (IPv6)
    char secondary_ipv6[46]; // Secondary DNS (IPv6)
} OnvifDnsConfig;

// ============================================================================
// NTP Configuration
// ============================================================================
typedef struct {
    bool enabled;
    char server[64];
    int port;
    int refresh_interval;
} OnvifNtpConfig;

// ============================================================================
// Hostname Configuration
// ============================================================================
typedef struct {
    char method[16];    // "auto" or "manual"
    char name[64];      // Hostname
} OnvifHostnameConfig;

// ============================================================================
// Network Protocol Configuration
// ============================================================================
typedef struct {
    int http_port;
    int https_port;
    int rtsp_port;
    int onvif_port;
} OnvifNetworkProtocols;

// ============================================================================
// NIC Speed Configuration
// ============================================================================
typedef struct {
    int speed;          // Speed in Mbps
    int duplex;         // 0=half, 1=full
    int autoneg;        // 0=disabled, 1=enabled
} OnvifNicSpeed;

// ============================================================================
// Network Interface Information
// ============================================================================
typedef struct {
    char name[32];          // Interface name (e.g., "eth0")
    char mac_address[18];   // MAC address string
    bool link_up;           // Link status
    OnvifIpv4Config ipv4;
} OnvifNetworkInterface;

// ============================================================================
// Network Control Functions
// ============================================================================

/**
 * @brief Get IPv4 configuration
 * @param iface Interface name (e.g., "eth0"), NULL for default
 * @param config Output configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_ipv4_config(const char *iface, OnvifIpv4Config *config);

/**
 * @brief Set IPv4 configuration
 * @param iface Interface name
 * @param config Configuration to set
 * @return 0 on success, -1 on error
 */
int onvif_set_ipv4_config(const char *iface, const OnvifIpv4Config *config);

/**
 * @brief Get DNS configuration
 * @param config Output configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_dns_config(OnvifDnsConfig *config);

/**
 * @brief Set DNS configuration
 * @param config Configuration to set
 * @return 0 on success, -1 on error
 */
int onvif_set_dns_config(const OnvifDnsConfig *config);

/**
 * @brief Get NTP configuration
 * @param config Output configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_ntp_config(OnvifNtpConfig *config);

/**
 * @brief Set NTP configuration
 * @param config Configuration to set
 * @return 0 on success, -1 on error
 */
int onvif_set_ntp_config(const OnvifNtpConfig *config);

/**
 * @brief Sync time with NTP server
 * @return 0 on success, -1 on error
 */
int onvif_sync_ntp_time(void);

/**
 * @brief Get hostname configuration
 * @param config Output configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_hostname_config(OnvifHostnameConfig *config);

/**
 * @brief Set hostname
 * @param hostname New hostname
 * @return 0 on success, -1 on error
 */
int onvif_set_hostname(const char *hostname);

/**
 * @brief Get MAC address
 * @param iface Interface name (NULL for default)
 * @param mac_address Output buffer (at least 18 bytes)
 * @return 0 on success, -1 on error
 */
int onvif_get_mac_address(const char *iface, char *mac_address);

/**
 * @brief Get network protocol ports
 * @param protocols Output configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_network_protocols(OnvifNetworkProtocols *protocols);

/**
 * @brief Set HTTP port
 * @param port Port number
 * @return 0 on success, -1 on error
 */
int onvif_set_http_port(int port);

/**
 * @brief Set HTTPS port
 * @param port Port number
 * @return 0 on success, -1 on error
 */
int onvif_set_https_port(int port);

/**
 * @brief Set RTSP port
 * @param port Port number
 * @return 0 on success, -1 on error
 */
int onvif_set_rtsp_port(int port);

/**
 * @brief Get NIC speed/duplex settings
 * @param iface Interface name
 * @param config Output configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_nic_speed(const char *iface, OnvifNicSpeed *config);

/**
 * @brief Set NIC speed/duplex settings
 * @param iface Interface name
 * @param config Configuration to set
 * @return 0 on success, -1 on error
 */
int onvif_set_nic_speed(const char *iface, const OnvifNicSpeed *config);

/**
 * @brief Get network interface information
 * @param iface Interface name
 * @param info Output interface info
 * @return 0 on success, -1 on error
 */
int onvif_get_network_interface(const char *iface, OnvifNetworkInterface *info);

/**
 * @brief Get number of network interfaces
 * @return Number of interfaces
 */
int onvif_get_network_interface_count(void);

/**
 * @brief Check internet connectivity
 * @return 1 if connected, 0 if not
 */
int onvif_check_internet_connectivity(void);

#ifdef __cplusplus
}
#endif

#endif /* ONVIF_NETWORK_WRAPPER_H */
