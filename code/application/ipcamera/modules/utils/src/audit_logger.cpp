/**
 * @file audit_logger.cpp
 * @brief Implementation of the centralized audit/access/security logger
 */

#include <ipcam/audit_logger.h>
#include <spdlog/spdlog.h>
#include <chrono>
#include <cstring>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <iomanip>
#include <sstream>

namespace ipcam {

// ============================================================================
// Singleton
// ============================================================================

AuditLogger& AuditLogger::Instance() {
    static AuditLogger instance;
    return instance;
}

AuditLogger::~AuditLogger() {
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

void AuditLogger::Init(const AuditLogConfig& config) {
    std::lock_guard<std::mutex> lk(config_mtx_);
    config_ = config;

    if (config_.access_enabled) {
        access_file_.open(config_.access_path, std::ios::app);
        if (!access_file_.is_open()) {
            spdlog::warn("[AuditLogger] Cannot open access log: {}", config_.access_path);
        }
    }

    if (config_.security_enabled) {
        security_file_.open(config_.security_path, std::ios::app);
        if (!security_file_.is_open()) {
            spdlog::warn("[AuditLogger] Cannot open security log: {}", config_.security_path);
        }
    }

    if (config_.audit_enabled) {
        audit_file_.open(config_.audit_path, std::ios::app);
        if (!audit_file_.is_open()) {
            spdlog::warn("[AuditLogger] Cannot open audit log: {}", config_.audit_path);
        }
    }

    // Initialize remote syslog socket
    if (config_.syslog_enabled && !config_.syslog_server.empty()) {
        if (config_.syslog_protocol == "udp") {
            syslog_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        } else {
            syslog_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        }

        if (syslog_fd_ >= 0 && config_.syslog_protocol == "tcp") {
            struct sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(config_.syslog_port);

            struct hostent* he = gethostbyname(config_.syslog_server.c_str());
            if (he) {
                memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
                // Non-blocking connect with timeout
                struct timeval tv{};
                tv.tv_sec = 5;
                setsockopt(syslog_fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

                if (::connect(syslog_fd_, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
                    spdlog::warn("[AuditLogger] Cannot connect to syslog {}:{}",
                                 config_.syslog_server, config_.syslog_port);
                    close(syslog_fd_);
                    syslog_fd_ = -1;
                }
            } else {
                spdlog::warn("[AuditLogger] Cannot resolve syslog server: {}",
                             config_.syslog_server);
                close(syslog_fd_);
                syslog_fd_ = -1;
            }
        }
    }

    initialized_ = true;
    spdlog::info("[AuditLogger] Initialized: access={} security={} audit={} syslog={}",
                 config_.access_enabled, config_.security_enabled,
                 config_.audit_enabled, config_.syslog_enabled);
}

void AuditLogger::Reload(const AuditLogConfig& config) {
    Shutdown();
    Init(config);
}

void AuditLogger::Shutdown() {
    std::lock_guard<std::mutex> lk(config_mtx_);
    initialized_ = false;

    {
        std::lock_guard<std::mutex> al(access_mtx_);
        if (access_file_.is_open()) access_file_.close();
    }
    {
        std::lock_guard<std::mutex> sl(security_mtx_);
        if (security_file_.is_open()) security_file_.close();
    }
    {
        std::lock_guard<std::mutex> ul(audit_mtx_);
        if (audit_file_.is_open()) audit_file_.close();
    }

    if (syslog_fd_ >= 0) {
        close(syslog_fd_);
        syslog_fd_ = -1;
    }
}

// ============================================================================
// Timestamp helper
// ============================================================================

std::string AuditLogger::Now() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) % 1000;
    auto time = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf{};
    gmtime_r(&time, &tm_buf);

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);

    char result[40];
    snprintf(result, sizeof(result), "%s.%03d", buf, (int)ms.count());
    return result;
}

const char* AuditLogger::LevelStr(AuditLevel level) {
    switch (level) {
        case AuditLevel::Info:     return "info";
        case AuditLevel::Warning:  return "warning";
        case AuditLevel::Error:    return "error";
        case AuditLevel::Critical: return "critical";
    }
    return "info";
}

const char* AuditLogger::CategoryStr(AuditCategory cat) {
    switch (cat) {
        case AuditCategory::Access:   return "access";
        case AuditCategory::Security: return "security";
        case AuditCategory::Audit:    return "audit";
    }
    return "unknown";
}

// ============================================================================
// File rotation
// ============================================================================

void AuditLogger::RotateIfNeeded(const std::string& path, int max_mb) {
    struct stat st{};
    if (stat(path.c_str(), &st) != 0) return;

    int64_t max_bytes = (int64_t)max_mb * 1024 * 1024;
    if (st.st_size < max_bytes) return;

    // Rotate: .log → .log.1, delete .log.2 (keep max 2 rotated)
    std::string rot2 = path + ".2";
    std::string rot1 = path + ".1";
    std::remove(rot2.c_str());
    std::rename(rot1.c_str(), rot2.c_str());
    std::rename(path.c_str(), rot1.c_str());
}

void AuditLogger::WriteLine(std::ofstream& file, std::mutex& mtx,
                             const std::string& path, int max_mb,
                             const std::string& line) {
    std::lock_guard<std::mutex> lk(mtx);

    // Check rotation
    RotateIfNeeded(path, max_mb);

    // Reopen if file was rotated or not open
    if (!file.is_open()) {
        file.open(path, std::ios::app);
        if (!file.is_open()) return;
    }

    file << line << "\n";
    file.flush();
}

// ============================================================================
// Access Logging
// ============================================================================

void AuditLogger::LogAccess(const AccessLogEntry& entry) {
    if (!initialized_ || !config_.access_enabled) return;

    // Format: [timestamp] [access] [info] ip=X user=X method=X uri=X status=X time_us=X ua=X req=X resp=X
    std::ostringstream ss;
    ss << "[" << (entry.timestamp.empty() ? Now() : entry.timestamp) << "] "
       << "[access] [info] "
       << "ip=" << entry.client_ip
       << " user=" << (entry.username.empty() ? "-" : entry.username)
       << " method=" << entry.method
       << " uri=" << entry.uri
       << " status=" << entry.status_code
       << " time_us=" << entry.response_time_us
       << " ua=\"" << entry.user_agent << "\""
       << " req_bytes=" << entry.request_size
       << " resp_bytes=" << entry.response_size;

    std::string line = ss.str();
    WriteLine(access_file_, access_mtx_, config_.access_path,
              config_.access_max_size_mb, line);

    {
        std::lock_guard<std::mutex> lk(stats_mtx_);
        stats_.access_entries++;
    }

    // Forward to syslog if enabled
    if (config_.syslog_enabled) {
        SendToSyslog(AuditCategory::Access, AuditLevel::Info, line);
    }
}

// ============================================================================
// Security Logging
// ============================================================================

void AuditLogger::LogSecurity(const SecurityLogEntry& entry) {
    if (!initialized_ || !config_.security_enabled) return;

    std::ostringstream ss;
    ss << "[" << (entry.timestamp.empty() ? Now() : entry.timestamp) << "] "
       << "[security] [" << LevelStr(entry.level) << "] "
       << "ip=" << entry.client_ip
       << " user=" << (entry.username.empty() ? "-" : entry.username)
       << " event=" << entry.event;
    if (!entry.details.empty()) {
        ss << " details=\"" << entry.details << "\"";
    }

    std::string line = ss.str();
    WriteLine(security_file_, security_mtx_, config_.security_path,
              config_.security_max_size_mb, line);

    {
        std::lock_guard<std::mutex> lk(stats_mtx_);
        stats_.security_entries++;
    }

    if (config_.syslog_enabled) {
        SendToSyslog(AuditCategory::Security, entry.level, line);
    }
}

void AuditLogger::LogLoginSuccess(const std::string& username, const std::string& ip,
                                   const std::string& user_agent) {
    SecurityLogEntry e;
    e.client_ip = ip;
    e.username  = username;
    e.event     = "login_success";
    e.details   = user_agent;
    e.level     = AuditLevel::Info;
    LogSecurity(e);
}

void AuditLogger::LogLoginFailure(const std::string& username, const std::string& ip,
                                   const std::string& reason) {
    SecurityLogEntry e;
    e.client_ip = ip;
    e.username  = username;
    e.event     = "login_failure";
    e.details   = reason;
    e.level     = AuditLevel::Warning;
    LogSecurity(e);
}

void AuditLogger::LogAccountLocked(const std::string& username, const std::string& ip,
                                    int failed_attempts, int lockout_minutes) {
    SecurityLogEntry e;
    e.client_ip = ip;
    e.username  = username;
    e.event     = "account_locked";
    e.details   = "attempts=" + std::to_string(failed_attempts) +
                  " lockout_min=" + std::to_string(lockout_minutes);
    e.level     = AuditLevel::Error;
    LogSecurity(e);
}

void AuditLogger::LogLogout(const std::string& username, const std::string& ip) {
    SecurityLogEntry e;
    e.client_ip = ip;
    e.username  = username;
    e.event     = "logout";
    e.level     = AuditLevel::Info;
    LogSecurity(e);
}

void AuditLogger::LogUnauthorizedAccess(const std::string& ip, const std::string& uri,
                                         const std::string& reason) {
    SecurityLogEntry e;
    e.client_ip = ip;
    e.event     = "unauthorized_access";
    e.details   = "uri=" + uri + " reason=" + reason;
    e.level     = AuditLevel::Warning;
    LogSecurity(e);
}

// ============================================================================
// Audit (Config Change) Logging
// ============================================================================

void AuditLogger::LogConfigChange(const std::string& username, const std::string& ip,
                                   const std::string& target, const std::string& old_val,
                                   const std::string& new_val) {
    if (!initialized_ || !config_.audit_enabled) return;

    std::ostringstream ss;
    ss << "[" << Now() << "] "
       << "[audit] [info] "
       << "ip=" << ip
       << " user=" << (username.empty() ? "-" : username)
       << " action=config_change"
       << " target=\"" << target << "\"";
    if (!old_val.empty()) ss << " old=\"" << old_val << "\"";
    if (!new_val.empty()) ss << " new=\"" << new_val << "\"";

    std::string line = ss.str();
    WriteLine(audit_file_, audit_mtx_, config_.audit_path,
              config_.audit_max_size_mb, line);

    {
        std::lock_guard<std::mutex> lk(stats_mtx_);
        stats_.audit_entries++;
    }

    if (config_.syslog_enabled) {
        SendToSyslog(AuditCategory::Audit, AuditLevel::Info, line);
    }
}

void AuditLogger::LogSystemAction(const std::string& username, const std::string& ip,
                                   const std::string& action, const std::string& details,
                                   AuditLevel level) {
    if (!initialized_ || !config_.audit_enabled) return;

    std::ostringstream ss;
    ss << "[" << Now() << "] "
       << "[audit] [" << LevelStr(level) << "] "
       << "ip=" << ip
       << " user=" << (username.empty() ? "-" : username)
       << " action=" << action;
    if (!details.empty()) ss << " details=\"" << details << "\"";

    std::string line = ss.str();
    WriteLine(audit_file_, audit_mtx_, config_.audit_path,
              config_.audit_max_size_mb, line);

    {
        std::lock_guard<std::mutex> lk(stats_mtx_);
        stats_.audit_entries++;
    }

    if (config_.syslog_enabled) {
        SendToSyslog(AuditCategory::Audit, level, line);
    }
}

void AuditLogger::LogUserAction(const std::string& admin_user, const std::string& ip,
                                 const std::string& action, const std::string& target_user,
                                 const std::string& details) {
    if (!initialized_ || !config_.audit_enabled) return;

    std::ostringstream ss;
    ss << "[" << Now() << "] "
       << "[audit] [info] "
       << "ip=" << ip
       << " user=" << admin_user
       << " action=" << action
       << " target_user=" << target_user;
    if (!details.empty()) ss << " details=\"" << details << "\"";

    std::string line = ss.str();
    WriteLine(audit_file_, audit_mtx_, config_.audit_path,
              config_.audit_max_size_mb, line);

    {
        std::lock_guard<std::mutex> lk(stats_mtx_);
        stats_.audit_entries++;
    }

    if (config_.syslog_enabled) {
        SendToSyslog(AuditCategory::Audit, AuditLevel::Info, line);
    }
}

// ============================================================================
// Remote Syslog
// ============================================================================

int AuditLogger::SyslogPriority(AuditCategory cat, AuditLevel level) const {
    // RFC 5424: Priority = Facility * 8 + Severity
    int facility = 16; // local0 = 16

    if (config_.syslog_facility == "local1") facility = 17;
    else if (config_.syslog_facility == "local2") facility = 18;
    else if (config_.syslog_facility == "local3") facility = 19;
    else if (config_.syslog_facility == "local4") facility = 20;
    else if (config_.syslog_facility == "local5") facility = 21;
    else if (config_.syslog_facility == "local6") facility = 22;
    else if (config_.syslog_facility == "local7") facility = 23;

    int severity;
    switch (level) {
        case AuditLevel::Critical: severity = 2; break; // critical
        case AuditLevel::Error:    severity = 3; break; // error
        case AuditLevel::Warning:  severity = 4; break; // warning
        case AuditLevel::Info:     severity = 6; break; // informational
        default:                   severity = 6; break;
    }

    return facility * 8 + severity;
}

void AuditLogger::SendToSyslog(AuditCategory category, AuditLevel level,
                                const std::string& message) {
    if (syslog_fd_ < 0) return;

    // Level filtering
    auto levelPrio = [](AuditLevel l) -> int {
        switch (l) {
            case AuditLevel::Info:     return 0;
            case AuditLevel::Warning:  return 1;
            case AuditLevel::Error:    return 2;
            case AuditLevel::Critical: return 3;
        }
        return 0;
    };
    auto minLevelPrio = [](const std::string& s) -> int {
        if (s == "warning") return 1;
        if (s == "error")   return 2;
        if (s == "critical") return 3;
        return 0;
    };

    if (levelPrio(level) < minLevelPrio(config_.syslog_min_level)) return;

    int pri = SyslogPriority(category, level);

    // Format: <PRI>VERSION TIMESTAMP HOSTNAME APP-NAME PROCID MSGID MSG
    char hostname[64] = {};
    gethostname(hostname, sizeof(hostname) - 1);

    std::ostringstream ss;
    ss << "<" << pri << ">1 " << Now() << "Z " << hostname
       << " ipcamd - - - [" << CategoryStr(category) << "] " << message;

    std::string packet = ss.str();

    if (config_.syslog_protocol == "udp") {
        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(config_.syslog_port);

        struct hostent* he = gethostbyname(config_.syslog_server.c_str());
        if (he) {
            memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
            ssize_t sent = sendto(syslog_fd_, packet.c_str(), packet.size(), 0,
                                  (struct sockaddr*)&addr, sizeof(addr));
            std::lock_guard<std::mutex> lk(stats_mtx_);
            if (sent > 0) stats_.syslog_sent++;
            else           stats_.syslog_failed++;
        }
    } else {
        ssize_t sent = send(syslog_fd_, packet.c_str(), packet.size(), MSG_NOSIGNAL);
        std::lock_guard<std::mutex> lk(stats_mtx_);
        if (sent > 0) stats_.syslog_sent++;
        else           stats_.syslog_failed++;
    }
}

// ============================================================================
// Config & Stats access
// ============================================================================

AuditLogConfig AuditLogger::GetConfig() const {
    std::lock_guard<std::mutex> lk(config_mtx_);
    return config_;
}

AuditLogger::Stats AuditLogger::GetStats() const {
    std::lock_guard<std::mutex> lk(stats_mtx_);
    return stats_;
}

}  // namespace ipcam
