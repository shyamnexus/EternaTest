#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <ipcam/result.h>
#include <ipcam/credential_manager.h>
#include <ipcam/result.h>

namespace ipcam {
namespace networking {

/**
 * @brief SMTP Manager - Handles SMTP configuration and connection testing
 * 
 * This class manages SMTP settings, integrating with CredentialManager for
 * secure credential storage and providing connection testing via CURL.
 */
class SmtpManager {
public:
    struct SmtpConfig {
        std::string server;       // SMTP server address
        int port;                 // SMTP port (25, 465, 587)
        int use_ssl;              // 0=no SSL, 1=SSL/TLS, 2=STARTTLS
        std::string username;     // Authentication username
        std::string password;     // Authentication password
        std::string from_email;   // Sender email
        std::string from_name;    // Sender name
        std::vector<std::string> receiver_emails; // Receiver emails for notifications (up to 5)
        bool enabled;             // SMTP enabled status
        int timeout_sec;          // Connection timeout
    };

    struct EmailMessage {
        std::string to;           // Recipient email (required)
        std::string cc;           // CC recipients (comma-separated, optional)
        std::string subject;      // Email subject
        std::string body;         // Email body
        bool is_html;             // true=HTML body, false=plain text
    };

    SmtpManager();
    ~SmtpManager() = default;

    // Get current SMTP configuration (with decrypted credentials)
    Result<SmtpConfig> GetConfig();
    
    // Set SMTP configuration (encrypts credentials)
    Result<void> SetConfig(const SmtpConfig& config);
    
    // Test SMTP connection with current configuration
    Result<void> TestConnection();
    
    // Send email using current configuration
    Result<void> SendEmail(const EmailMessage& email);
    
    // Send test email to verify configuration
    Result<void> SendTestEmail(const std::string& to_email);

private:
    std::mutex mutex_;
};

} // namespace networking
} // namespace ipcam
