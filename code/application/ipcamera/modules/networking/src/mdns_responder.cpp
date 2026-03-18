/**
 * @file mdns_responder.cpp
 * @brief Lightweight mDNS responder for .local hostname resolution and DNS-SD service discovery
 * 
 * Implements RFC 6762 (mDNS) and RFC 6763 (DNS-SD) for zero-configuration networking.
 * Replaces the heavier avahi daemon while providing essential functionality:
 * - Responds to A record queries for hostname.local
 * - Announces services (_http._tcp, _rtsp._tcp, _onvif._tcp)
 * - Sends goodbye packets on shutdown
 */

#include "ipcam/mdns_responder.h"
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <syslog.h>
#include <algorithm>

namespace ipcam {
namespace networking {

// DNS header structure
struct DnsHeader {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;  // Question count
    uint16_t ancount;  // Answer count
    uint16_t nscount;  // Authority count
    uint16_t arcount;  // Additional count
} __attribute__((packed));

// mDNS flag constants
static constexpr uint16_t DNS_FLAG_QR      = 0x8000;  // Query/Response
static constexpr uint16_t DNS_FLAG_AA      = 0x0400;  // Authoritative Answer
static constexpr uint16_t DNS_FLAG_OPCODE  = 0x7800;  // Opcode mask

MdnsResponder& MdnsResponder::GetInstance() {
    static MdnsResponder instance;
    return instance;
}

MdnsResponder::MdnsResponder() 
    : socket_fd_(-1)
    , running_(false)
    , hostname_("ipcamera")
    , interface_("eth0") {
}

MdnsResponder::~MdnsResponder() {
    Stop();
}

void MdnsResponder::SetHostname(const std::string& hostname) {
    std::lock_guard<std::mutex> lock(mutex_);
    hostname_ = hostname;
    // Remove .local suffix if present
    size_t pos = hostname_.find(".local");
    if (pos != std::string::npos) {
        hostname_ = hostname_.substr(0, pos);
    }
}

void MdnsResponder::SetInterface(const std::string& interface) {
    std::lock_guard<std::mutex> lock(mutex_);
    interface_ = interface;
}

void MdnsResponder::AddService(const MdnsService& service) {
    std::lock_guard<std::mutex> lock(mutex_);
    services_.push_back(service);
}

void MdnsResponder::RemoveService(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    services_.erase(
        std::remove_if(services_.begin(), services_.end(),
            [&name](const MdnsService& s) { return s.name == name; }),
        services_.end()
    );
}

bool MdnsResponder::Start() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (running_) {
        return true;
    }
    
    if (!CreateSocket()) {
        syslog(LOG_ERR, "[MDNS] Failed to create multicast socket");
        return false;
    }
    
    running_ = true;
    responder_thread_ = std::thread(&MdnsResponder::ResponderLoop, this);
    
    syslog(LOG_INFO, "[MDNS] Started mDNS responder for %s.local", hostname_.c_str());
    return true;
}

void MdnsResponder::Stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            return;
        }
        
        // Send goodbye packets before stopping
        SendGoodbye();
        
        running_ = false;
    }
    
    // Close socket to unblock recvfrom
    if (socket_fd_ >= 0) {
        shutdown(socket_fd_, SHUT_RDWR);
        close(socket_fd_);
        socket_fd_ = -1;
    }
    
    if (responder_thread_.joinable()) {
        responder_thread_.join();
    }
    
    syslog(LOG_INFO, "[MDNS] Stopped mDNS responder");
}

bool MdnsResponder::IsRunning() const {
    return running_;
}

void MdnsResponder::Announce() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_ || socket_fd_ < 0) {
        return;
    }
    
    // Announce hostname
    AnnounceHostname();
    
    // Announce all services
    for (const auto& service : services_) {
        AnnounceService(service);
    }
}

void MdnsResponder::SendGoodbye() {
    if (socket_fd_ < 0) {
        return;
    }
    
    // Send goodbye packet (TTL=0) for hostname
    std::vector<uint8_t> packet;
    BuildARecordResponse(packet, 0);  // TTL=0 means goodbye
    SendPacket(packet);
    
    syslog(LOG_INFO, "[MDNS] Sent goodbye packet for %s.local", hostname_.c_str());
}

bool MdnsResponder::CreateSocket() {
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        syslog(LOG_ERR, "[MDNS] Failed to create socket: %s", strerror(errno));
        return false;
    }
    
    // Allow address reuse
    int reuse = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        syslog(LOG_WARNING, "[MDNS] Failed to set SO_REUSEADDR: %s", strerror(errno));
    }
    
#ifdef SO_REUSEPORT
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        syslog(LOG_WARNING, "[MDNS] Failed to set SO_REUSEPORT: %s", strerror(errno));
    }
#endif
    
    // Bind to mDNS port
    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(MDNS_PORT);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(socket_fd_, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) < 0) {
        syslog(LOG_ERR, "[MDNS] Failed to bind to port %d: %s", MDNS_PORT, strerror(errno));
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }
    
    // Join multicast group
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(MDNS_MULTICAST_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    
    if (setsockopt(socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        syslog(LOG_ERR, "[MDNS] Failed to join multicast group: %s", strerror(errno));
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }
    
    // Set multicast TTL
    unsigned char ttl = 255;
    if (setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        syslog(LOG_WARNING, "[MDNS] Failed to set multicast TTL: %s", strerror(errno));
    }
    
    // Disable multicast loopback
    unsigned char loop = 0;
    if (setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
        syslog(LOG_WARNING, "[MDNS] Failed to disable multicast loopback: %s", strerror(errno));
    }
    
    return true;
}

void MdnsResponder::CloseSocket() {
    if (socket_fd_ >= 0) {
        // Leave multicast group
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(MDNS_MULTICAST_ADDR);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        setsockopt(socket_fd_, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
        
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

void MdnsResponder::ResponderLoop() {
    std::vector<uint8_t> buffer(MDNS_MAX_PACKET_SIZE);
    struct sockaddr_in sender_addr;
    socklen_t sender_len;
    
    while (running_) {
        sender_len = sizeof(sender_addr);
        
        ssize_t received = recvfrom(socket_fd_, buffer.data(), buffer.size(), 0,
                                     (struct sockaddr*)&sender_addr, &sender_len);
        
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (!running_) {
                break;
            }
            syslog(LOG_WARNING, "[MDNS] recvfrom error: %s", strerror(errno));
            continue;
        }
        
        if (received < static_cast<ssize_t>(sizeof(DnsHeader))) {
            continue;  // Too small to be valid DNS
        }
        
        HandleQuery(buffer.data(), received);
    }
}

void MdnsResponder::HandleQuery(const uint8_t* data, size_t length) {
    const DnsHeader* header = reinterpret_cast<const DnsHeader*>(data);
    
    // Check if this is a query (QR=0) with standard opcode
    uint16_t flags = ntohs(header->flags);
    if (flags & DNS_FLAG_QR) {
        return;  // This is a response, not a query
    }
    if (flags & DNS_FLAG_OPCODE) {
        return;  // Non-standard opcode
    }
    
    uint16_t qdcount = ntohs(header->qdcount);
    if (qdcount == 0) {
        return;
    }
    
    // Parse questions
    const uint8_t* ptr = data + sizeof(DnsHeader);
    const uint8_t* end = data + length;
    
    for (uint16_t i = 0; i < qdcount && ptr < end; i++) {
        std::string qname = ParseDnsName(data, length, ptr);
        if (ptr + 4 > end) {
            break;
        }
        
        uint16_t qtype = (ptr[0] << 8) | ptr[1];
        uint16_t qclass = (ptr[2] << 8) | ptr[3];
        ptr += 4;
        
        // Mask unicast-response bit
        qclass &= 0x7FFF;
        
        if (qclass != DNS_CLASS_IN && qclass != 255) {  // IN or ANY
            continue;
        }
        
        // Check if query matches our hostname
        std::string our_name = hostname_ + ".local";
        if (qname == our_name || qname == hostname_ + ".local.") {
            if (qtype == DNS_TYPE_A || qtype == DNS_TYPE_ANY) {
                std::vector<uint8_t> response;
                BuildARecordResponse(response, MDNS_DEFAULT_TTL);
                SendPacket(response);
            }
        }
        
        // Check for PTR queries for our services
        if (qtype == DNS_TYPE_PTR || qtype == DNS_TYPE_ANY) {
            for (const auto& service : services_) {
                std::string service_type = service.type + ".local";
                if (qname == service_type || qname == service_type + ".") {
                    std::vector<uint8_t> response;
                    BuildServiceResponse(response, service, MDNS_DEFAULT_TTL);
                    SendPacket(response);
                }
            }
        }
    }
}

std::string MdnsResponder::ParseDnsName(const uint8_t* packet, size_t packet_len, const uint8_t*& ptr) {
    std::string name;
    const uint8_t* start = packet;
    const uint8_t* end = packet + packet_len;
    int jumps = 0;
    const int max_jumps = 10;
    bool jumped = false;
    const uint8_t* next_ptr = nullptr;
    
    while (ptr < end && jumps < max_jumps) {
        uint8_t len = *ptr;
        
        if (len == 0) {
            ptr++;
            break;
        }
        
        if ((len & 0xC0) == 0xC0) {
            // Pointer
            if (ptr + 1 >= end) break;
            if (!jumped) {
                next_ptr = ptr + 2;
            }
            uint16_t offset = ((len & 0x3F) << 8) | ptr[1];
            if (offset >= packet_len) break;
            ptr = start + offset;
            jumped = true;
            jumps++;
            continue;
        }
        
        ptr++;
        if (ptr + len > end) break;
        
        if (!name.empty()) {
            name += ".";
        }
        name.append(reinterpret_cast<const char*>(ptr), len);
        ptr += len;
    }
    
    if (jumped && next_ptr) {
        ptr = next_ptr;
    }
    
    return name;
}

void MdnsResponder::EncodeDnsName(std::vector<uint8_t>& packet, const std::string& name) {
    size_t pos = 0;
    while (pos < name.length()) {
        size_t dot = name.find('.', pos);
        if (dot == std::string::npos) {
            dot = name.length();
        }
        
        size_t label_len = dot - pos;
        if (label_len > 63) {
            label_len = 63;  // Max label length
        }
        
        packet.push_back(static_cast<uint8_t>(label_len));
        for (size_t i = 0; i < label_len; i++) {
            packet.push_back(static_cast<uint8_t>(name[pos + i]));
        }
        
        pos = dot + 1;
    }
    packet.push_back(0);  // Null terminator
}

void MdnsResponder::BuildARecordResponse(std::vector<uint8_t>& packet, uint32_t ttl) {
    std::string local_ip = GetLocalIP();
    if (local_ip.empty()) {
        return;
    }
    
    // Parse IP address
    struct in_addr addr;
    if (inet_aton(local_ip.c_str(), &addr) == 0) {
        return;
    }
    
    // DNS header
    DnsHeader header;
    memset(&header, 0, sizeof(header));
    header.id = 0;
    header.flags = htons(DNS_FLAG_QR | DNS_FLAG_AA);  // Response, Authoritative
    header.ancount = htons(1);
    
    packet.resize(sizeof(header));
    memcpy(packet.data(), &header, sizeof(header));
    
    // Answer section
    std::string fqdn = hostname_ + ".local";
    EncodeDnsName(packet, fqdn);
    
    // Type A
    packet.push_back(0);
    packet.push_back(DNS_TYPE_A);
    
    // Class IN with cache-flush bit
    packet.push_back(0x80);  // Cache-flush
    packet.push_back(DNS_CLASS_IN);
    
    // TTL
    packet.push_back((ttl >> 24) & 0xFF);
    packet.push_back((ttl >> 16) & 0xFF);
    packet.push_back((ttl >> 8) & 0xFF);
    packet.push_back(ttl & 0xFF);
    
    // RDLENGTH (4 bytes for IPv4)
    packet.push_back(0);
    packet.push_back(4);
    
    // RDATA (IP address)
    uint32_t ip_bytes = ntohl(addr.s_addr);
    packet.push_back((ip_bytes >> 24) & 0xFF);
    packet.push_back((ip_bytes >> 16) & 0xFF);
    packet.push_back((ip_bytes >> 8) & 0xFF);
    packet.push_back(ip_bytes & 0xFF);
}

void MdnsResponder::BuildServiceResponse(std::vector<uint8_t>& packet, const MdnsService& service, uint32_t ttl) {
    std::string local_ip = GetLocalIP();
    if (local_ip.empty()) {
        return;
    }
    
    // DNS header
    DnsHeader header;
    memset(&header, 0, sizeof(header));
    header.id = 0;
    header.flags = htons(DNS_FLAG_QR | DNS_FLAG_AA);
    header.ancount = htons(4);  // PTR + SRV + TXT + A records
    
    packet.resize(sizeof(header));
    memcpy(packet.data(), &header, sizeof(header));
    
    std::string service_type = service.type + ".local";
    std::string instance_name = service.name + "." + service_type;
    std::string hostname_local = hostname_ + ".local";
    
    // PTR record: _service._tcp.local -> Instance._service._tcp.local
    EncodeDnsName(packet, service_type);
    packet.push_back(0);
    packet.push_back(DNS_TYPE_PTR);
    packet.push_back(0);
    packet.push_back(DNS_CLASS_IN);
    
    // TTL
    packet.push_back((ttl >> 24) & 0xFF);
    packet.push_back((ttl >> 16) & 0xFF);
    packet.push_back((ttl >> 8) & 0xFF);
    packet.push_back(ttl & 0xFF);
    
    // PTR RDATA - need to calculate length
    std::vector<uint8_t> ptr_rdata;
    EncodeDnsName(ptr_rdata, instance_name);
    
    packet.push_back((ptr_rdata.size() >> 8) & 0xFF);
    packet.push_back(ptr_rdata.size() & 0xFF);
    packet.insert(packet.end(), ptr_rdata.begin(), ptr_rdata.end());
    
    // SRV record: Instance._service._tcp.local -> priority weight port hostname.local
    EncodeDnsName(packet, instance_name);
    packet.push_back(0);
    packet.push_back(DNS_TYPE_SRV);
    packet.push_back(0x80);  // Cache-flush
    packet.push_back(DNS_CLASS_IN);
    
    // TTL
    packet.push_back((ttl >> 24) & 0xFF);
    packet.push_back((ttl >> 16) & 0xFF);
    packet.push_back((ttl >> 8) & 0xFF);
    packet.push_back(ttl & 0xFF);
    
    // SRV RDATA
    std::vector<uint8_t> srv_rdata;
    srv_rdata.push_back(0);  // Priority high
    srv_rdata.push_back(0);  // Priority low
    srv_rdata.push_back(0);  // Weight high
    srv_rdata.push_back(0);  // Weight low
    srv_rdata.push_back((service.port >> 8) & 0xFF);
    srv_rdata.push_back(service.port & 0xFF);
    EncodeDnsName(srv_rdata, hostname_local);
    
    packet.push_back((srv_rdata.size() >> 8) & 0xFF);
    packet.push_back(srv_rdata.size() & 0xFF);
    packet.insert(packet.end(), srv_rdata.begin(), srv_rdata.end());
    
    // TXT record
    EncodeDnsName(packet, instance_name);
    packet.push_back(0);
    packet.push_back(DNS_TYPE_TXT);
    packet.push_back(0x80);  // Cache-flush
    packet.push_back(DNS_CLASS_IN);
    
    // TTL
    packet.push_back((ttl >> 24) & 0xFF);
    packet.push_back((ttl >> 16) & 0xFF);
    packet.push_back((ttl >> 8) & 0xFF);
    packet.push_back(ttl & 0xFF);
    
    // TXT RDATA
    std::vector<uint8_t> txt_rdata;
    if (service.txt_records.empty()) {
        // Empty TXT record (single null byte)
        txt_rdata.push_back(0);
    } else {
        for (const auto& txt : service.txt_records) {
            if (txt.length() > 255) {
                txt_rdata.push_back(255);
                txt_rdata.insert(txt_rdata.end(), txt.begin(), txt.begin() + 255);
            } else {
                txt_rdata.push_back(static_cast<uint8_t>(txt.length()));
                txt_rdata.insert(txt_rdata.end(), txt.begin(), txt.end());
            }
        }
    }
    
    packet.push_back((txt_rdata.size() >> 8) & 0xFF);
    packet.push_back(txt_rdata.size() & 0xFF);
    packet.insert(packet.end(), txt_rdata.begin(), txt_rdata.end());
    
    // A record for target hostname
    struct in_addr addr;
    if (inet_aton(local_ip.c_str(), &addr) != 0) {
        EncodeDnsName(packet, hostname_local);
        packet.push_back(0);
        packet.push_back(DNS_TYPE_A);
        packet.push_back(0x80);  // Cache-flush
        packet.push_back(DNS_CLASS_IN);
        
        // TTL
        packet.push_back((ttl >> 24) & 0xFF);
        packet.push_back((ttl >> 16) & 0xFF);
        packet.push_back((ttl >> 8) & 0xFF);
        packet.push_back(ttl & 0xFF);
        
        // RDLENGTH
        packet.push_back(0);
        packet.push_back(4);
        
        // RDATA
        uint32_t ip_bytes = ntohl(addr.s_addr);
        packet.push_back((ip_bytes >> 24) & 0xFF);
        packet.push_back((ip_bytes >> 16) & 0xFF);
        packet.push_back((ip_bytes >> 8) & 0xFF);
        packet.push_back(ip_bytes & 0xFF);
    }
}

void MdnsResponder::SendPacket(const std::vector<uint8_t>& packet) {
    if (socket_fd_ < 0 || packet.empty()) {
        return;
    }
    
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(MDNS_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(MDNS_MULTICAST_ADDR);
    
    ssize_t sent = sendto(socket_fd_, packet.data(), packet.size(), 0,
                          (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    if (sent < 0) {
        syslog(LOG_WARNING, "[MDNS] Failed to send packet: %s", strerror(errno));
    }
}

std::string MdnsResponder::GetLocalIP() const {
    struct ifaddrs* ifaddr = nullptr;
    std::string result;
    
    if (getifaddrs(&ifaddr) == -1) {
        syslog(LOG_WARNING, "[MDNS] getifaddrs failed: %s", strerror(errno));
        return result;
    }
    
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }
        
        if (ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        
        // Match interface name if specified
        if (!interface_.empty() && interface_ != ifa->ifa_name) {
            continue;
        }
        
        // Skip loopback
        if (strcmp(ifa->ifa_name, "lo") == 0) {
            continue;
        }
        
        struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
        char ip_str[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &addr->sin_addr, ip_str, sizeof(ip_str)) != nullptr) {
            result = ip_str;
            break;
        }
    }
    
    freeifaddrs(ifaddr);
    return result;
}

void MdnsResponder::AnnounceHostname() {
    std::vector<uint8_t> packet;
    BuildARecordResponse(packet, MDNS_DEFAULT_TTL);
    
    // Send twice with short delay (mDNS recommendation)
    SendPacket(packet);
    usleep(250000);  // 250ms
    SendPacket(packet);
}

void MdnsResponder::AnnounceService(const MdnsService& service) {
    std::vector<uint8_t> packet;
    BuildServiceResponse(packet, service, MDNS_DEFAULT_TTL);
    
    // Send twice with short delay
    SendPacket(packet);
    usleep(250000);
    SendPacket(packet);
}

}  // namespace networking
}  // namespace ipcam
