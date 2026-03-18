/**
 * @file audit_logger.h
 * @brief Centralized audit/access/security logging for IP camera
 *
 * Provides structured logging to separate files for:
 *   - Access log:   Every HTTP API request (who, what, when, from where)
 *   - Security log: Authentication events (login, logout, failures, lockouts)
 *   - Audit log:    Configuration changes (what changed, by whom, old/new values)
 *
 * All logs are written in a parseable format:
 *   [YYYY-MM-DD HH:MM:SS.mmm] [CATEGORY] [LEVEL] JSON_PAYLOAD
 *
 * Thread-safe singleton — safe to call from any CivetWeb worker thread.
 */

#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <cstdint>

namespace ipcam {

/// Log categories for the audit system
enum class AuditCategory {
    Access,     ///< HTTP request log (like nginx access_log but at app layer)
    Security,   ///< Auth events: login, logout, failure, lockout, brute-force
    Audit       ///< Config changes: setting changed, by whom, old→new
};

/// Severity levels for audit entries
enum class AuditLevel {
    Info,       ///< Normal operation (login success, config read)
    Warning,    ///< Suspicious activity (failed login, unusual access)
    Error,      ///< Security event (lockout, brute force detected)
    Critical    ///< Critical security event (factory reset, firmware change)
};

/// Single access log entry (one per HTTP request)
struct AccessLogEntry {
    std::string timestamp;
    std::string client_ip;
    std::string username;       ///< Empty if unauthenticated
    std::string method;         ///< GET, POST, PUT, DELETE
    std::string uri;
    int         status_code = 0;
    int64_t     response_time_us = 0;  ///< Microseconds
    std::string user_agent;
    int64_t     request_size = 0;      ///< Bytes
    int64_t     response_size = 0;     ///< Bytes
};

/// Single security log entry
struct SecurityLogEntry {
    std::string timestamp;
    std::string client_ip;
    std::string username;
    std::string event;          ///< "login_success", "login_failure", "logout",
                                ///< "account_locked", "brute_force_detected",
                                ///< "password_changed", "session_expired",
                                ///< "unauthorized_access", "token_invalid"
    std::string details;        ///< Additional info (failure reason, etc.)
    AuditLevel  level = AuditLevel::Info;
};

/// Single audit (config change) log entry
struct AuditLogEntry {
    std::string timestamp;
    std::string client_ip;
    std::string username;
    std::string action;         ///< "config_change", "firmware_upgrade",
                                ///< "factory_reset", "reboot", "user_created",
                                ///< "user_deleted", "password_reset"
    std::string target;         ///< What was changed (config key, user, etc.)
    std::string old_value;      ///< Previous value (empty if N/A)
    std::string new_value;      ///< New value (empty if N/A)
    std::string details;        ///< Extra context
    AuditLevel  level = AuditLevel::Info;
};

/// Configuration for the audit logger
struct AuditLogConfig {
    bool        access_enabled = true;
    std::string access_path = "/var/log/ipcamd_access.log";
    int         access_max_size_mb = 5;

    bool        security_enabled = true;
    std::string security_path = "/var/log/ipcamd_security.log";
    int         security_max_size_mb = 2;

    bool        audit_enabled = true;
    std::string audit_path = "/var/log/ipcamd_audit.log";
    int         audit_max_size_mb = 2;

    // Remote syslog forwarding
    bool        syslog_enabled = false;
    std::string syslog_server;          ///< Hostname or IP
    int         syslog_port = 514;
    std::string syslog_protocol = "udp"; ///< "udp" or "tcp"
    std::string syslog_facility = "local0";
    std::string syslog_min_level = "info";  ///< Minimum level to forward
};

/**
 * @class AuditLogger
 * @brief Thread-safe singleton for structured audit logging
 */
class AuditLogger {
public:
    /// Get the singleton instance
    static AuditLogger& Instance();

    /// Initialize with configuration (call once at startup)
    void Init(const AuditLogConfig& config);

    /// Reload configuration (e.g., after API config change)
    void Reload(const AuditLogConfig& config);

    /// Shutdown and flush all files
    void Shutdown();

    // ----- Access Logging -----

    /// Log an HTTP request/response
    void LogAccess(const AccessLogEntry& entry);

    // ----- Security Logging -----

    /// Log a security event
    void LogSecurity(const SecurityLogEntry& entry);

    /// Convenience: log a successful login
    void LogLoginSuccess(const std::string& username, const std::string& ip,
                         const std::string& user_agent = "");

    /// Convenience: log a failed login
    void LogLoginFailure(const std::string& username, const std::string& ip,
                         const std::string& reason);

    /// Convenience: log account lockout
    void LogAccountLocked(const std::string& username, const std::string& ip,
                          int failed_attempts, int lockout_minutes);

    /// Convenience: log logout
    void LogLogout(const std::string& username, const std::string& ip);

    /// Convenience: log unauthorized access attempt
    void LogUnauthorizedAccess(const std::string& ip, const std::string& uri,
                               const std::string& reason);

    // ----- Audit (Config Change) Logging -----

    /// Log a configuration change
    void LogConfigChange(const std::string& username, const std::string& ip,
                         const std::string& target, const std::string& old_val,
                         const std::string& new_val);

    /// Log a system action (reboot, factory reset, firmware upgrade, etc.)
    void LogSystemAction(const std::string& username, const std::string& ip,
                         const std::string& action, const std::string& details = "",
                         AuditLevel level = AuditLevel::Info);

    /// Log a user management action (create, delete, password change)
    void LogUserAction(const std::string& admin_user, const std::string& ip,
                       const std::string& action, const std::string& target_user,
                       const std::string& details = "");

    // ----- Remote Syslog -----

    /// Send a message to the configured remote syslog server
    void SendToSyslog(AuditCategory category, AuditLevel level,
                      const std::string& message);

    // ----- Config Access -----

    /// Get current configuration
    AuditLogConfig GetConfig() const;

    /// Get statistics
    struct Stats {
        uint64_t access_entries = 0;
        uint64_t security_entries = 0;
        uint64_t audit_entries = 0;
        uint64_t syslog_sent = 0;
        uint64_t syslog_failed = 0;
    };
    Stats GetStats() const;

private:
    AuditLogger() = default;
    ~AuditLogger();
    AuditLogger(const AuditLogger&) = delete;
    AuditLogger& operator=(const AuditLogger&) = delete;

    /// Get current timestamp as string
    static std::string Now();

    /// Write a line to a log file with rotation check
    void WriteLine(std::ofstream& file, std::mutex& mtx,
                   const std::string& path, int max_mb,
                   const std::string& line);

    /// Rotate a log file if it exceeds max size
    void RotateIfNeeded(const std::string& path, int max_mb);

    /// Format syslog priority
    int SyslogPriority(AuditCategory cat, AuditLevel level) const;

    /// Convert level to string
    static const char* LevelStr(AuditLevel level);

    /// Convert category to string
    static const char* CategoryStr(AuditCategory cat);

    AuditLogConfig config_;
    bool           initialized_ = false;

    std::ofstream  access_file_;
    std::ofstream  security_file_;
    std::ofstream  audit_file_;

    mutable std::mutex access_mtx_;
    mutable std::mutex security_mtx_;
    mutable std::mutex audit_mtx_;
    mutable std::mutex config_mtx_;

    Stats stats_;
    mutable std::mutex stats_mtx_;

    // Remote syslog socket
    int syslog_fd_ = -1;
};

}  // namespace ipcam
