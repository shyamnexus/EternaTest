#include "ipcam/ntp_manager.h"
#include <ipcam/config.h>
#include <spdlog/spdlog.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include <chrono>
#include <sstream>
#include <fstream>
#include <cctype>

namespace ipcam {
namespace networking {

// Static fallback IPs when DNS is unavailable
// These are well-known, reliable NTP server IPs
const std::vector<std::string> NtpManager::FALLBACK_IPS = {
    // Google Public NTP (time.google.com)
    "216.239.35.0",
    "216.239.35.4",
    "216.239.35.8",
    "216.239.35.12",
    // Cloudflare NTP (time.cloudflare.com)
    "162.159.200.1",
    "162.159.200.123",
    // NIST NTP (time.nist.gov)
    "129.6.15.28",
    "129.6.15.29",
    // Apple NTP (time.apple.com) 
    "17.253.34.123"
};

NtpManager& NtpManager::Instance() {
    static NtpManager instance;
    return instance;
}

NtpManager::NtpManager() {
    last_result_.status = SyncStatus::Error;
    last_result_.message = "Not yet synced";
}

NtpManager::~NtpManager() {
    Shutdown();
}

Result<void> NtpManager::Initialize() {
    spdlog::info("[NtpManager] Initializing...");
    
    LoadConfig();
    
    if (!config_.enabled) {
        spdlog::info("[NtpManager] NTP is disabled in configuration");
        return Result<void>::Ok();
    }
    
    spdlog::info("[NtpManager] Primary server: {}", config_.primary_server);
    spdlog::info("[NtpManager] Retry count: {}, Timeout: {}s", 
                 config_.retry_count, config_.sync_timeout_sec);
    spdlog::info("[NtpManager] Refresh interval: {} seconds", config_.refresh_interval_sec);
    
    return Result<void>::Ok();
}

void NtpManager::Shutdown() {
    spdlog::info("[NtpManager] Shutting down...");
    StopPeriodicSync();
}

void NtpManager::LoadConfig() {
    // Load from system.time.ntp section in config
    config_.enabled = config::Get<bool>("system.time.ntp.enabled", true);
    config_.primary_server = config::Get<std::string>("system.time.ntp.primary_server", "pool.ntp.org");
    config_.sync_timeout_sec = config::Get<int>("system.time.ntp.sync_timeout_sec", 10);
    config_.retry_count = config::Get<int>("system.time.ntp.retry_count", 3);
    config_.port = config::Get<int>("system.time.ntp.port", 123);
    config_.refresh_interval_sec = config::Get<int>("system.time.ntp.refresh_interval_sec", 3600);
    config_.use_fallback_ips = config::Get<bool>("system.time.ntp.use_fallback_ips", true);
    config_.timezone = config::Get<std::string>("system.time.timezone", "UTC");
    
    // Load source setting (auto/dhcp/manual)
    std::string source_str = config::Get<std::string>("system.time.ntp.source", "auto");
    if (source_str == "dhcp") {
        config_.source = Source::Dhcp;
    } else if (source_str == "manual") {
        config_.source = Source::Manual;
    } else {
        config_.source = Source::Auto;
    }
    
    // Load fallback servers from config array (JSON array stored as comma-separated)
    auto fallback_str = config::Get<std::string>("system.time.ntp.fallback_servers", 
        "0.pool.ntp.org,1.pool.ntp.org,2.pool.ntp.org,3.pool.ntp.org,"
        "time.google.com,time.cloudflare.com,time.windows.com,"
        "time.nist.gov,time.apple.com,ntp.ubuntu.com");
    
    config_.fallback_servers.clear();
    std::istringstream ss(fallback_str);
    std::string server;
    while (std::getline(ss, server, ',')) {
        // Trim whitespace
        size_t start = server.find_first_not_of(" \t");
        size_t end = server.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            config_.fallback_servers.push_back(server.substr(start, end - start + 1));
        }
    }
    
    // Load fallback IPs from config
    auto fallback_ips_str = config::Get<std::string>("system.time.ntp.fallback_ips",
        "216.239.35.0,216.239.35.4,162.159.200.1,129.6.15.28");
    
    config_.fallback_ips.clear();
    std::istringstream ips_ss(fallback_ips_str);
    std::string ip;
    while (std::getline(ips_ss, ip, ',')) {
        size_t start = ip.find_first_not_of(" \t");
        size_t end = ip.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            config_.fallback_ips.push_back(ip.substr(start, end - start + 1));
        }
    }
    
    // Load DST configuration
    dst_config_.enabled = config::Get<bool>("system.time.dst.enabled", false);
    dst_config_.mode = config::Get<std::string>("system.time.dst.mode", "auto");
    dst_config_.offset_minutes = config::Get<int>("system.time.dst.offset_minutes", 60);
    dst_config_.start_month = config::Get<int>("system.time.dst.start_month", 3);
    dst_config_.start_week = config::Get<int>("system.time.dst.start_week", 2);
    dst_config_.start_day_of_week = config::Get<int>("system.time.dst.start_day_of_week", 0);
    dst_config_.start_hour = config::Get<int>("system.time.dst.start_hour", 2);
    dst_config_.end_month = config::Get<int>("system.time.dst.end_month", 11);
    dst_config_.end_week = config::Get<int>("system.time.dst.end_week", 1);
    dst_config_.end_day_of_week = config::Get<int>("system.time.dst.end_day_of_week", 0);
    dst_config_.end_hour = config::Get<int>("system.time.dst.end_hour", 2);

    spdlog::debug("[NtpManager] Config loaded: source={}, primary={}, port={}, {} fallback servers, {} fallback IPs, DST={}",
                  source_str, config_.primary_server, config_.port,
                  config_.fallback_servers.size(), config_.fallback_ips.size(),
                  dst_config_.enabled ? "on" : "off");
}

bool NtpManager::SaveConfig() {
    config::Set<bool>("system.time.ntp.enabled", config_.enabled);
    config::Set<std::string>("system.time.ntp.primary_server", config_.primary_server);
    config::Set<int>("system.time.ntp.port", config_.port);
    config::Set<int>("system.time.ntp.sync_timeout_sec", config_.sync_timeout_sec);
    config::Set<int>("system.time.ntp.retry_count", config_.retry_count);
    config::Set<int>("system.time.ntp.refresh_interval_sec", config_.refresh_interval_sec);
    config::Set<bool>("system.time.ntp.use_fallback_ips", config_.use_fallback_ips);
    config::Set<std::string>("system.time.timezone", config_.timezone);
    
    // Save source
    std::string source_str;
    switch (config_.source) {
        case Source::Dhcp: source_str = "dhcp"; break;
        case Source::Manual: source_str = "manual"; break;
        default: source_str = "auto"; break;
    }
    config::Set<std::string>("system.time.ntp.source", source_str);
    
    // Save fallback servers as comma-separated
    std::ostringstream oss;
    for (size_t i = 0; i < config_.fallback_servers.size(); ++i) {
        if (i > 0) oss << ",";
        oss << config_.fallback_servers[i];
    }
    config::Set<std::string>("system.time.ntp.fallback_servers", oss.str());
    
    // Save fallback IPs
    std::ostringstream ips_oss;
    for (size_t i = 0; i < config_.fallback_ips.size(); ++i) {
        if (i > 0) ips_oss << ",";
        ips_oss << config_.fallback_ips[i];
    }
    config::Set<std::string>("system.time.ntp.fallback_ips", ips_oss.str());

    // Save DST config
    config::Set<bool>("system.time.dst.enabled", dst_config_.enabled);
    config::Set<std::string>("system.time.dst.mode", dst_config_.mode);
    config::Set<int>("system.time.dst.offset_minutes", dst_config_.offset_minutes);
    config::Set<int>("system.time.dst.start_month", dst_config_.start_month);
    config::Set<int>("system.time.dst.start_week", dst_config_.start_week);
    config::Set<int>("system.time.dst.start_day_of_week", dst_config_.start_day_of_week);
    config::Set<int>("system.time.dst.start_hour", dst_config_.start_hour);
    config::Set<int>("system.time.dst.end_month", dst_config_.end_month);
    config::Set<int>("system.time.dst.end_week", dst_config_.end_week);
    config::Set<int>("system.time.dst.end_day_of_week", dst_config_.end_day_of_week);
    config::Set<int>("system.time.dst.end_hour", dst_config_.end_hour);
    
    return config::Save();
}

std::vector<std::string> NtpManager::GetDhcpNtpServers() {
    std::vector<std::string> servers;
    
    // Common DHCP lease file locations
    const char* lease_files[] = {
        "/var/lib/dhcp/dhclient.leases",
        "/var/lib/dhclient/dhclient.leases",
        "/var/run/dhclient.leases",
        "/tmp/udhcpc.ntp",              // udhcpc can write NTP servers here
        "/var/run/udhcpc.ntp",
        "/etc/ntp.dhcp"                 // Some systems write DHCP NTP here
    };
    
    for (const auto& path : lease_files) {
        std::ifstream file(path);
        if (!file.is_open()) continue;
        
        std::string line;
        while (std::getline(file, line)) {
            // Look for NTP server option in various formats
            // dhclient format: option ntp-servers 1.2.3.4, 5.6.7.8;
            // udhcpc format: just IP addresses, one per line
            
            size_t pos = line.find("ntp-servers");
            if (pos != std::string::npos) {
                // dhclient format
                size_t start = line.find_first_of("0123456789", pos);
                if (start != std::string::npos) {
                    size_t end = line.find(';', start);
                    if (end == std::string::npos) end = line.length();
                    
                    std::string ntp_list = line.substr(start, end - start);
                    std::istringstream iss(ntp_list);
                    std::string server;
                    while (std::getline(iss, server, ',')) {
                        // Trim whitespace
                        size_t s = server.find_first_not_of(" \t");
                        size_t e = server.find_last_not_of(" \t");
                        if (s != std::string::npos && e != std::string::npos) {
                            servers.push_back(server.substr(s, e - s + 1));
                        }
                    }
                }
            } else if (path == std::string("/tmp/udhcpc.ntp") || 
                       path == std::string("/var/run/udhcpc.ntp")) {
                // udhcpc simple format: one IP per line
                size_t start = line.find_first_not_of(" \t");
                size_t end = line.find_last_not_of(" \t\n\r");
                if (start != std::string::npos && end != std::string::npos) {
                    std::string ip = line.substr(start, end - start + 1);
                    // Basic validation: starts with digit
                    if (!ip.empty() && isdigit(ip[0])) {
                        servers.push_back(ip);
                    }
                }
            }
        }
        
        if (!servers.empty()) {
            spdlog::info("[NtpManager] Found {} NTP servers from DHCP ({})", 
                        servers.size(), path);
            break;  // Found servers, stop searching
        }
    }
    
    if (servers.empty()) {
        spdlog::debug("[NtpManager] No NTP servers found in DHCP lease files");
    }
    
    return servers;
}
NtpManager::Config NtpManager::GetConfig() const {
    return config_;
}

Result<void> NtpManager::SetConfig(const Config& cfg) {
    config_ = cfg;
    
    if (!SaveConfig()) {
        return Result<void>::Err("Failed to save NTP configuration");
    }
    
    return Result<void>::Ok();
}

NtpManager::SyncResult NtpManager::GetLastSyncResult() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    return last_result_;
}

NtpManager::DstConfig NtpManager::GetDstConfig() const {
    return dst_config_;
}

Result<void> NtpManager::SetDstConfig(const DstConfig& cfg) {
    dst_config_ = cfg;
    if (!SaveConfig()) {
        return Result<void>::Err("Failed to save DST configuration");
    }
    spdlog::info("[NtpManager] DST config updated: enabled={}, mode={}, offset={}min",
                 cfg.enabled, cfg.mode, cfg.offset_minutes);
    return Result<void>::Ok();
}

void NtpManager::SetSyncCallback(SyncCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    sync_callback_ = callback;
}

bool NtpManager::CheckInternetConnectivity() {
    // Method 1: Try DNS resolution (works even if ICMP is blocked)
    // This is more reliable than ping as many networks block ICMP
    struct addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    // Try multiple hostnames in case one DNS is cached/blocked
    const char* test_hosts[] = {"pool.ntp.org", "time.google.com", "google.com"};
    for (const auto& host : test_hosts) {
        int ret = getaddrinfo(host, nullptr, &hints, &res);
        if (ret == 0 && res != nullptr) {
            freeaddrinfo(res);
            spdlog::debug("[NtpManager] Internet check passed via DNS resolve of {}", host);
            return true;
        }
    }
    
    // Method 2: DNS failed, try UDP socket connect to known NTP server IPs
    // UDP connect() doesn't actually send packets, just checks route exists
    const char* test_ips[] = {"216.239.35.0", "162.159.200.1", "129.6.15.28"};
    for (const auto& ip : test_ips) {
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock < 0) continue;
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(config_.port);  // Use configured NTP port
        inet_pton(AF_INET, ip, &addr.sin_addr);
        
        int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        close(sock);
        
        if (result == 0) {
            spdlog::debug("[NtpManager] Internet check passed via UDP connect to {}", ip);
            return true;
        }
    }
    
    spdlog::debug("[NtpManager] All internet connectivity checks failed");
    return false;
}

int NtpManager::CreateNtpSocket() {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        spdlog::error("[NtpManager] Failed to create socket: {}", strerror(errno));
        return -1;
    }
    
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(0);
    
    if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        spdlog::error("[NtpManager] Failed to bind socket: {}", strerror(errno));
        close(sock);
        return -1;
    }
    
    struct timeval tv;
    tv.tv_sec = config_.sync_timeout_sec;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    return sock;
}

void NtpManager::BuildNtpRequest(NtpPacket& packet) {
    memset(&packet, 0, sizeof(packet));
    
    // LI = 0, VN = 3, Mode = 3 (client)
    packet.li_vn_mode = (0 << 6) | (3 << 3) | 3;
    packet.stratum = 0;
    packet.poll = 4;
    packet.precision = -6;
    packet.root_delay = htonl(1 << 16);
    packet.root_dispersion = htonl(1 << 16);
    
    struct timeval now;
    gettimeofday(&now, nullptr);
    packet.trans_time.seconds = htonl(now.tv_sec + JAN_1970);
    packet.trans_time.fraction = htonl((uint32_t)(((uint64_t)now.tv_usec * 4294967296ULL) / 1000000ULL));
}

bool NtpManager::ParseNtpResponse(const NtpPacket& packet, int64_t& offset_ms) {
    struct timeval now;
    gettimeofday(&now, nullptr);
    
    uint32_t server_sec = ntohl(packet.trans_time.seconds) - JAN_1970;
    uint32_t server_frac = ntohl(packet.trans_time.fraction);
    uint64_t server_usec = ((uint64_t)server_frac * 1000000ULL) >> 32;
    
    int64_t local_ms = (int64_t)now.tv_sec * 1000 + now.tv_usec / 1000;
    int64_t server_ms = (int64_t)server_sec * 1000 + server_usec / 1000;
    
    offset_ms = server_ms - local_ms;
    
    // Sanity check — embedded devices (especially new ones) can boot with RTC
    // at Unix epoch (1970) or any arbitrary past date if RTC battery is dead or
    // was never set.  A 56+ year offset is perfectly normal in that case.
    // We only reject negative offsets that would set time before Unix epoch, or
    // offsets that set time unreasonably far in the future (>10 years from now).
    constexpr int64_t EPOCH_MS = 0;
    int64_t result_ms = local_ms + offset_ms;
    if (result_ms < EPOCH_MS) {
        spdlog::warn("[NtpManager] NTP would set time before Unix epoch (result={}ms), ignoring", result_ms);
        return false;
    }
    // Reject if resulting time is >10 years in the future from a reasonable "now"
    // (use server time itself: if server says it's year 2036+, that's suspicious)
    constexpr int64_t TEN_YEARS_MS = 10LL * 365 * 24 * 60 * 60 * 1000;
    constexpr int64_t JAN_2025_MS = 1735689600LL * 1000;  // 2025-01-01 epoch ms
    if (server_ms > JAN_2025_MS + TEN_YEARS_MS) {
        spdlog::warn("[NtpManager] NTP server time is unreasonably far in the future ({}ms), ignoring", server_ms);
        return false;
    }

    if (std::abs(offset_ms) > 365LL * 24 * 60 * 60 * 1000) {
        spdlog::warn("[NtpManager] Large time offset: {}ms ({:.1f} years) — RTC was likely wrong, applying correction",
                     offset_ms, offset_ms / (365.25 * 24 * 60 * 60 * 1000));
    }
    
    return true;
}

bool NtpManager::ApplyTimeOffset(int64_t offset_ms) {
    if (std::abs(offset_ms) < 100) {  // Less than 100ms, consider synced
        spdlog::debug("[NtpManager] Time already synchronized (offset={}ms)", offset_ms);
        return true;
    }
    
    struct timeval now, newtime;
    gettimeofday(&now, nullptr);
    
    int64_t new_ms = (int64_t)now.tv_sec * 1000 + now.tv_usec / 1000 + offset_ms;
    newtime.tv_sec = new_ms / 1000;
    newtime.tv_usec = (new_ms % 1000) * 1000;
    
    if (settimeofday(&newtime, nullptr) < 0) {
        spdlog::error("[NtpManager] Failed to set system time: {}", strerror(errno));
        return false;
    }
    
    time_t t = newtime.tv_sec;
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_info);
    spdlog::info("[NtpManager] System time set to: {} (offset: {}ms)", time_buf, offset_ms);
    
    // Sync to hardware RTC so time persists across reboots
    if (::system("hwclock -w 2>/dev/null") != 0) {
        spdlog::warn("[NtpManager] hwclock -w failed; RTC may not persist new time");
    } else {
        spdlog::info("[NtpManager] Hardware clock (RTC) updated");
    }
    
    return true;
}

bool NtpManager::TrySyncWithServer(const std::string& server, int64_t& offset_ms) {
    spdlog::debug("[NtpManager] Attempting sync with: {}", server);
    
    struct addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    int ret = getaddrinfo(server.c_str(), nullptr, &hints, &res);
    if (ret != 0) {
        spdlog::debug("[NtpManager] Failed to resolve {}: {}", server, gai_strerror(ret));
        return false;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config_.port);
    memcpy(&server_addr.sin_addr, 
           &((struct sockaddr_in*)res->ai_addr)->sin_addr, 
           sizeof(server_addr.sin_addr));
    freeaddrinfo(res);
    
    int sock = CreateNtpSocket();
    if (sock < 0) return false;
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        spdlog::debug("[NtpManager] Failed to connect to {}: {}", server, strerror(errno));
        close(sock);
        return false;
    }
    
    for (int attempt = 0; attempt < config_.retry_count; ++attempt) {
        spdlog::debug("[NtpManager] Attempt {} of {} for {}", 
                      attempt + 1, config_.retry_count, server);
        
        NtpPacket request;
        BuildNtpRequest(request);
        
        if (send(sock, &request, sizeof(request), 0) < 0) {
            spdlog::debug("[NtpManager] Send failed: {}", strerror(errno));
            continue;
        }
        
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        
        struct timeval timeout;
        timeout.tv_sec = config_.sync_timeout_sec;
        timeout.tv_usec = 0;
        
        ret = select(sock + 1, &readfds, nullptr, nullptr, &timeout);
        if (ret <= 0) {
            if (ret == 0) {
                spdlog::debug("[NtpManager] Timeout waiting for response from {}", server);
            }
            continue;
        }
        
        NtpPacket response;
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        
        ssize_t recv_len = recvfrom(sock, &response, sizeof(response), 0,
                                    (struct sockaddr*)&from_addr, &from_len);
        if (recv_len < (ssize_t)sizeof(response)) {
            spdlog::debug("[NtpManager] Incomplete response from {}", server);
            continue;
        }
        
        if (ParseNtpResponse(response, offset_ms)) {
            close(sock);
            return true;
        }
    }
    
    close(sock);
    return false;
}

NtpManager::SyncResult NtpManager::SyncNow() {
    SyncResult result;
    
    if (!config_.enabled) {
        result.status = SyncStatus::Disabled;
        result.message = "NTP is disabled in configuration";
        spdlog::info("[NtpManager] {}", result.message);
        std::lock_guard<std::mutex> lock(mutex_);
        last_result_ = result;
        return result;
    }
    
    bool expected = false;
    if (!syncing_.compare_exchange_strong(expected, true)) {
        result.status = SyncStatus::InProgress;
        result.message = "Sync already in progress";
        return result;
    }
    
    spdlog::info("[NtpManager] Starting time synchronization (source: {})...",
                 config_.source == Source::Dhcp ? "dhcp" : 
                 config_.source == Source::Manual ? "manual" : "auto");
    
    if (!CheckInternetConnectivity()) {
        syncing_ = false;
        result.status = SyncStatus::NoInternet;
        result.message = "No internet connectivity - using system/RTC time";
        spdlog::warn("[NtpManager] {}", result.message);
        std::lock_guard<std::mutex> lock(mutex_);
        last_result_ = result;
        if (sync_callback_) sync_callback_(result);
        return result;
    }
    
    // Build server list based on source setting
    std::vector<std::string> servers;
    
    // Step 1: Add DHCP servers first if source is auto or dhcp
    if (config_.source == Source::Auto || config_.source == Source::Dhcp) {
        auto dhcp_servers = GetDhcpNtpServers();
        for (const auto& s : dhcp_servers) {
            servers.push_back(s);
            spdlog::debug("[NtpManager] Added DHCP NTP server: {}", s);
        }
    }
    
    // Step 2: Add manual servers if source is auto or manual
    if (config_.source == Source::Auto || config_.source == Source::Manual) {
        // Primary server
        if (!config_.primary_server.empty()) {
            bool found = false;
            for (const auto& existing : servers) {
                if (existing == config_.primary_server) { found = true; break; }
            }
            if (!found) servers.push_back(config_.primary_server);
        }
        
        // Fallback servers from config
        for (const auto& s : config_.fallback_servers) {
            bool found = false;
            for (const auto& existing : servers) {
                if (existing == s) { found = true; break; }
            }
            if (!found) servers.push_back(s);
        }
    }
    
    // Step 3: Add fallback IPs as last resort (works without DNS)
    if (config_.use_fallback_ips) {
        // Use configured fallback IPs, or static defaults if not configured
        const auto& ips = config_.fallback_ips.empty() ? FALLBACK_IPS : config_.fallback_ips;
        for (const auto& ip : ips) {
            servers.push_back(ip);
        }
    }
    
    spdlog::debug("[NtpManager] Will try {} NTP servers", servers.size());
    
    int64_t offset_ms = 0;
    for (const auto& server : servers) {
        if (TrySyncWithServer(server, offset_ms)) {
            if (ApplyTimeOffset(offset_ms)) {
                syncing_ = false;
                result.status = SyncStatus::Success;
                result.server_used = server;
                result.offset_ms = offset_ms;
                result.message = "Time synchronized with " + server;
                spdlog::info("[NtpManager] {}", result.message);
                std::lock_guard<std::mutex> lock(mutex_);
                last_result_ = result;
                if (sync_callback_) sync_callback_(result);
                return result;
            }
        }
    }
    
    syncing_ = false;
    result.status = SyncStatus::AllServersFailed;
    result.message = "All NTP servers unreachable - using system/RTC time";
    spdlog::warn("[NtpManager] {}", result.message);
    std::lock_guard<std::mutex> lock(mutex_);
    last_result_ = result;
    if (sync_callback_) sync_callback_(result);
    return result;
}

void NtpManager::TriggerSync(SyncCallback callback) {
    if (callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        sync_callback_ = callback;
    }
    sync_requested_ = true;
    cv_.notify_one();
}

Result<void> NtpManager::StartPeriodicSync() {
    if (!config_.enabled) {
        return Result<void>::Ok();
    }
    
    if (running_.load()) {
        return Result<void>::Ok();
    }
    
    spdlog::info("[NtpManager] Starting periodic sync (interval: {}s)", 
                 config_.refresh_interval_sec);
    
    running_ = true;
    sync_thread_ = std::thread(&NtpManager::SyncThread, this);
    
    return Result<void>::Ok();
}

void NtpManager::StopPeriodicSync() {
    if (!running_.load()) {
        return;
    }
    
    spdlog::info("[NtpManager] Stopping periodic sync...");
    running_ = false;
    cv_.notify_all();
    
    if (sync_thread_.joinable()) {
        sync_thread_.join();
    }
}

void NtpManager::SyncThread() {
    SyncNow();
    
    while (running_.load()) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, std::chrono::seconds(config_.refresh_interval_sec), [this]() {
            return !running_.load() || sync_requested_.load();
        });
        
        if (!running_.load()) break;
        
        sync_requested_ = false;
        lock.unlock();
        
        spdlog::debug("[NtpManager] Periodic sync triggered");
        SyncNow();
    }
}

} // namespace networking
} // namespace ipcam
