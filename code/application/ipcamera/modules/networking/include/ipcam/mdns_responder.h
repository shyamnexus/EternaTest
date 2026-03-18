#pragma once
/**
 * @file mdns_responder.h
 * @brief Lightweight mDNS/DNS-SD responder for IP camera discovery
 * 
 * This module implements a minimal mDNS responder that:
 * - Responds to A record queries for <hostname>.local
 * - Announces services (_onvif._tcp, _rtsp._tcp, _http._tcp)
 * - Handles multicast DNS on 224.0.0.251:5353
 * 
 * No external dependencies (no avahi/dbus) - just raw sockets.
 */

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>
#include "ipcam/result.h"

// Forward declare sockaddr_in from system headers (avoid namespace pollution)
struct sockaddr_in;

namespace ipcam {
namespace networking {

/**
 * @brief Service record for DNS-SD announcement
 */
struct MdnsService {
    std::string name;        // e.g., "IPCamera Living Room"
    std::string type;        // e.g., "_http._tcp"
    uint16_t port;           // e.g., 80
    std::vector<std::string> txt_records; // TXT record strings like "key=value"
};

/**
 * @brief Lightweight mDNS responder for .local hostname and service discovery
 * 
 * Usage:
 *   auto& mdns = MdnsResponder::GetInstance();
 *   mdns.SetHostname("ipcamera");
 *   mdns.AddService({"My Camera", "_http._tcp", 80, {"path=/"}});
 *   mdns.AddService({"My Camera", "_rtsp._tcp", 554, {}});
 *   mdns.Start();
 */
class MdnsResponder {
public:
    /// Singleton access
    static MdnsResponder& GetInstance();
    
    /// Destructor - stops responder if running
    ~MdnsResponder();
    
    // Non-copyable
    MdnsResponder(const MdnsResponder&) = delete;
    MdnsResponder& operator=(const MdnsResponder&) = delete;
    
    /**
     * @brief Set the hostname for .local resolution
     * @param hostname The hostname (without .local suffix)
     */
    void SetHostname(const std::string& hostname);
    
    /**
     * @brief Set the network interface to use
     * @param interface Network interface name (e.g., "eth0")
     */
    void SetInterface(const std::string& interface);
    
    /**
     * @brief Add a service to announce via DNS-SD
     * @param service The service to announce
     */
    void AddService(const MdnsService& service);
    
    /**
     * @brief Remove a service from announcements
     * @param name The service name to remove
     */
    void RemoveService(const std::string& name);
    
    /**
     * @brief Start the mDNS responder
     * @return true if started successfully
     */
    bool Start();
    
    /**
     * @brief Stop the mDNS responder
     */
    void Stop();
    
    /**
     * @brief Check if the responder is running
     * @return true if running
     */
    bool IsRunning() const;
    
    /**
     * @brief Force announce all services (e.g., after IP change)
     */
    void Announce();
    
    /**
     * @brief Send goodbye packets for all services (called before shutdown)
     */
    void SendGoodbye();

private:
    MdnsResponder();
    
    // Main responder loop
    void ResponderLoop();
    
    // Socket setup
    bool CreateSocket();
    void CloseSocket();
    
    // Query handling
    void HandleQuery(const uint8_t* data, size_t length);
    
    // DNS name parsing
    std::string ParseDnsName(const uint8_t* packet, size_t packet_len, const uint8_t*& ptr);
    
    // DNS name encoding
    void EncodeDnsName(std::vector<uint8_t>& packet, const std::string& name);
    
    // Response building
    void BuildARecordResponse(std::vector<uint8_t>& packet, uint32_t ttl);
    void BuildServiceResponse(std::vector<uint8_t>& packet, const MdnsService& service, uint32_t ttl);
    
    // Send packet to multicast group
    void SendPacket(const std::vector<uint8_t>& packet);
    
    // Get local IP address for the interface
    std::string GetLocalIP() const;
    
    // Announce helpers
    void AnnounceHostname();
    void AnnounceService(const MdnsService& service);
    
    // Member variables
    int socket_fd_;
    std::atomic<bool> running_;
    std::thread responder_thread_;
    std::mutex mutex_;
    
    std::string hostname_;
    std::string interface_;
    std::vector<MdnsService> services_;
    
    // mDNS constants
    static constexpr uint16_t MDNS_PORT = 5353;
    static constexpr const char* MDNS_MULTICAST_ADDR = "224.0.0.251";
    static constexpr size_t MDNS_MAX_PACKET_SIZE = 9000;
    static constexpr uint32_t MDNS_DEFAULT_TTL = 120;
    
    // DNS record types
    static constexpr uint16_t DNS_TYPE_A = 1;
    static constexpr uint16_t DNS_TYPE_PTR = 12;
    static constexpr uint16_t DNS_TYPE_TXT = 16;
    static constexpr uint16_t DNS_TYPE_SRV = 33;
    static constexpr uint16_t DNS_TYPE_ANY = 255;
    static constexpr uint16_t DNS_CLASS_IN = 1;
};

} // namespace networking

// Bring MdnsResponder and MdnsService into ipcam namespace for convenience
using networking::MdnsResponder;
using networking::MdnsService;

} // namespace ipcam
