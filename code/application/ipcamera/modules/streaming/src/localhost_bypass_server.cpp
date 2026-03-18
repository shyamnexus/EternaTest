/**
 * @file localhost_bypass_server.cpp
 * @brief Localhost Authentication Bypass RTSP Server implementation
 * 
 * Contains LocalhostBypassClientConnection and LocalhostBypassRTSPServer implementations.
 * Allows localhost connections (e.g., go2rtc) to bypass authentication.
 */

#include "ipcam/rtsp_sources_internal.h"
#include <netinet/tcp.h>  // TCP_NODELAY

#ifndef RTSP_SERVER_ENABLED
#define RTSP_SERVER_ENABLED 1
#endif

#if RTSP_SERVER_ENABLED

namespace ipcam {
namespace streaming {

// ============================================================================
// LocalhostBypassClientConnection Implementation
// ============================================================================

LocalhostBypassClientConnection::LocalhostBypassClientConnection(
    RTSPServer& ourServer,
    int clientSocket, 
    struct sockaddr_storage const& clientAddr)
    : RTSPServer::RTSPClientConnection(ourServer, clientSocket, clientAddr) {
}

Boolean LocalhostBypassClientConnection::authenticationOK(
    char const* cmdName, 
    char const* urlSuffix, 
    char const* fullRequestStr) {
    // Check if client is from localhost - bypass auth if so
    if (isLocalhost()) {
        return True;
    }
    
    // For external clients, use normal authentication
    return RTSPServer::RTSPClientConnection::authenticationOK(cmdName, urlSuffix, fullRequestStr);
}

bool LocalhostBypassClientConnection::isLocalhost() const {
    // fClientAddr is inherited from GenericMediaServer::ClientConnection
    if (fClientAddr.ss_family == AF_INET) {
        struct sockaddr_in const* addr4 = (struct sockaddr_in const*)&fClientAddr;
        // 127.0.0.1 in network byte order
        if (ntohl(addr4->sin_addr.s_addr) == INADDR_LOOPBACK) {
            spdlog::debug("LocalhostBypassRTSPServer: Bypassing auth for localhost IPv4");
            return true;
        }
    } else if (fClientAddr.ss_family == AF_INET6) {
        struct sockaddr_in6 const* addr6 = (struct sockaddr_in6 const*)&fClientAddr;
        
        // Check for ::1 (IPv6 loopback)
        if (IN6_IS_ADDR_LOOPBACK(&addr6->sin6_addr)) {
            spdlog::debug("LocalhostBypassRTSPServer: Bypassing auth for localhost IPv6");
            return true;
        }
        
        // Check for ::ffff:127.0.0.1 (IPv4-mapped IPv6 loopback)
        if (IN6_IS_ADDR_V4MAPPED(&addr6->sin6_addr)) {
            const uint8_t* bytes = addr6->sin6_addr.s6_addr;
            uint32_t ipv4 = (bytes[12] << 24) | (bytes[13] << 16) | (bytes[14] << 8) | bytes[15];
            if (ipv4 == INADDR_LOOPBACK) {
                spdlog::debug("LocalhostBypassRTSPServer: Bypassing auth for IPv4-mapped localhost");
                return true;
            }
        }
    }
    return false;
}

// ============================================================================
// LocalhostBypassRTSPServer Implementation
// ============================================================================

LocalhostBypassRTSPServer* LocalhostBypassRTSPServer::createNew(
    UsageEnvironment& env, 
    Port port,
    UserAuthenticationDatabase* authDatabase,
    unsigned reclamationSeconds) {
    
    int ourSocketIPv4 = setUpOurSocket(env, port, AF_INET);
    int ourSocketIPv6 = setUpOurSocket(env, port, AF_INET6);
    
    spdlog::info("RtspServer: Socket creation IPv4={} IPv6={} port={}", 
                 ourSocketIPv4, ourSocketIPv6, ntohs(port.num()));
    
    if (ourSocketIPv4 < 0 && ourSocketIPv6 < 0) {
        spdlog::error("RtspServer: Failed to create any socket for port {}", ntohs(port.num()));
        return nullptr;
    }
    
    if (ourSocketIPv4 < 0) {
        spdlog::warn("RtspServer: IPv4 socket creation failed, using IPv6 only");
    }
    
    return new LocalhostBypassRTSPServer(env, ourSocketIPv4, ourSocketIPv6, 
                                         port, authDatabase, reclamationSeconds);
}

LocalhostBypassRTSPServer::LocalhostBypassRTSPServer(
    UsageEnvironment& env, 
    int ourSocketIPv4, int ourSocketIPv6,
    Port port, 
    UserAuthenticationDatabase* authDatabase,
    unsigned reclamationSeconds)
    : RTSPServer(env, ourSocketIPv4, ourSocketIPv6, port, 
                 authDatabase, reclamationSeconds) {
}

RTSPServer::ClientConnection* LocalhostBypassRTSPServer::createNewClientConnection(
    int clientSocket, 
    struct sockaddr_storage const& clientAddr) {
    // Enable TCP_NODELAY to reduce latency for interleaved RTP/RTSP
    int flag = 1;
    if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
        spdlog::warn("RtspServer: Failed to set TCP_NODELAY on client socket");
    }
    
    return new LocalhostBypassClientConnection(*this, clientSocket, clientAddr);
}

} // namespace streaming
} // namespace ipcam

#endif // RTSP_SERVER_ENABLED
