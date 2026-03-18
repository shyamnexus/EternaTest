#include <ipcam/smtp_manager.h>
#include <ipcam/config.h>
#include <ipcam/storage.h>
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <ctime>
#include <sstream>
#include <vector>
#include <algorithm>

namespace ipcam {
namespace networking {

SmtpManager::SmtpManager() {
    spdlog::info("[SmtpManager] Initializing SMTP manager");
}

Result<SmtpManager::SmtpConfig> SmtpManager::GetConfig() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    SmtpConfig cfg;
    
    // Load non-sensitive from config system
    cfg.enabled = ipcam::config::Get<bool>("smtp.enabled", false);
    cfg.server = ipcam::config::Get<std::string>("smtp.server", "");
    cfg.port = ipcam::config::Get<int>("smtp.port", 587);
    cfg.use_ssl = ipcam::config::Get<int>("smtp.use_ssl", 2);
    cfg.from_email = ipcam::config::Get<std::string>("smtp.from_email", "");
    cfg.from_name = ipcam::config::Get<std::string>("smtp.from_name", "IPCamera");
    
    // Load receiver emails (up to 5)
    for (int i = 0; i < 5; i++) {
        std::string key = "smtp.receiver_email_" + std::to_string(i);
        std::string email = ipcam::config::Get<std::string>(key, "");
        if (!email.empty()) {
            cfg.receiver_emails.push_back(email);
        }
    }
    
    cfg.timeout_sec = ipcam::config::Get<int>("smtp.timeout_sec", 30);
    
    // Load sensitive from CredentialManager
    auto& cred_mgr = storage::GetCredentialManager();
    if (!cred_mgr.Initialize()) {
        spdlog::error("[SmtpManager] Failed to initialize CredentialManager");
    } else {
        auto creds = cred_mgr.GetSmtpCredentials();
        if (creds) {
            cfg.username = creds->username;
            cfg.password = creds->password;
        }
    }
    
    return Result<SmtpConfig>::Ok(cfg);
}

Result<void> SmtpManager::SetConfig(const SmtpConfig& cfg) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Store non-sensitive in config system
    ipcam::config::Set<bool>("smtp.enabled", cfg.enabled);
    ipcam::config::Set<std::string>("smtp.server", cfg.server);
    ipcam::config::Set<int>("smtp.port", cfg.port);
    ipcam::config::Set<int>("smtp.use_ssl", cfg.use_ssl);
    ipcam::config::Set<std::string>("smtp.from_email", cfg.from_email);
    ipcam::config::Set<std::string>("smtp.from_name", cfg.from_name);
    
    // Store receiver emails (up to 5)
    for (int i = 0; i < 5; i++) {
        std::string key = "smtp.receiver_email_" + std::to_string(i);
        if (i < cfg.receiver_emails.size()) {
            ipcam::config::Set<std::string>(key, cfg.receiver_emails[i]);
        } else {
            ipcam::config::Set<std::string>(key, ""); // Clear unused slots
        }
    }
    
    ipcam::config::Set<int>("smtp.timeout_sec", cfg.timeout_sec);
    
    if (!ipcam::config::Save()) {
        return Result<void>::Err("Failed to save SMTP config");
    }
    
    // Store sensitive in CredentialManager
    auto& cred_mgr = storage::GetCredentialManager();
    if (!cred_mgr.Initialize()) {
        return Result<void>::Err("Failed to initialize CredentialManager");
    }
    
    storage::CredentialManager::SmtpCredentials creds;
    creds.server = cfg.server;
    creds.port = cfg.port;
    creds.username = cfg.username;
    creds.password = cfg.password;
    creds.from_email = cfg.from_email;
    creds.from_name = cfg.from_name;
    creds.use_tls = (cfg.use_ssl == 2);
    creds.timeout_sec = cfg.timeout_sec;
    
    if (!cred_mgr.StoreSmtpCredentials(creds)) {
        return Result<void>::Err("Failed to store SMTP credentials");
    }
    
    spdlog::info("[SmtpManager] Configuration saved successfully");
    return Result<void>::Ok();
}

Result<void> SmtpManager::TestConnection() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto config_result = GetConfig();
    if (!config_result.success) {
        return Result<void>::Err("Failed to get SMTP config: " + config_result.error);
    }
    
    auto& cfg = config_result.value;
    
    if (!cfg.enabled) {
        return Result<void>::Err("SMTP is disabled");
    }
    
    if (cfg.server.empty()) {
        return Result<void>::Err("SMTP server not configured");
    }
    
    // Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        return Result<void>::Err("Failed to initialize CURL");
    }
    
    // Build SMTP URL
    std::string url;
    if (cfg.use_ssl == 1) {
        url = "smtps://" + cfg.server + ":" + std::to_string(cfg.port);
    } else {
        url = "smtp://" + cfg.server + ":" + std::to_string(cfg.port);
    }
    
    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, cfg.username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, cfg.password.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, cfg.timeout_sec);
    
    if (cfg.use_ssl == 2) {
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    }
    
    // Perform connection test
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        std::string error = "SMTP connection failed: ";
        error += curl_easy_strerror(res);
        return Result<void>::Err(error);
    }
    
    spdlog::info("[SmtpManager] Connection test successful");
    return Result<void>::Ok();
}

// Callback function for reading email payload
static size_t payload_source(char* ptr, size_t size, size_t nmemb, void* userp) {
    std::string* upload_data = static_cast<std::string*>(userp);
    size_t room = size * nmemb;
    
    if (upload_data->empty()) {
        return 0;
    }
    
    size_t len = std::min(room, upload_data->size());
    std::memcpy(ptr, upload_data->data(), len);
    upload_data->erase(0, len);
    
    return len;
}

Result<void> SmtpManager::SendEmail(const EmailMessage& email) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Validate email
    if (email.to.empty()) {
        return Result<void>::Err("Recipient email address is required");
    }
    
    if (email.subject.empty()) {
        return Result<void>::Err("Email subject is required");
    }
    
    if (email.body.empty()) {
        return Result<void>::Err("Email body is required");
    }
    
    // Get configuration
    auto config_result = GetConfig();
    if (!config_result.success) {
        return Result<void>::Err("Failed to get SMTP config: " + config_result.error);
    }
    
    auto& cfg = config_result.value;
    
    if (!cfg.enabled) {
        return Result<void>::Err("SMTP is disabled");
    }
    
    if (cfg.server.empty()) {
        return Result<void>::Err("SMTP server not configured");
    }
    
    // Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        return Result<void>::Err("Failed to initialize CURL");
    }
    
    // Build server URL
    std::string url;
    if (cfg.use_ssl == 1) {
        url = "smtps://" + cfg.server + ":" + std::to_string(cfg.port);
    } else {
        url = "smtp://" + cfg.server + ":" + std::to_string(cfg.port);
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Enable STARTTLS if configured
    if (cfg.use_ssl == 2) {
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    }
    
    // Set authentication
    if (!cfg.username.empty()) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, cfg.username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, cfg.password.c_str());
    }
    
    // Set sender
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, cfg.from_email.c_str());
    
    // Set recipients
    struct curl_slist* recipients = nullptr;
    recipients = curl_slist_append(recipients, email.to.c_str());
    
    // Parse CC addresses if provided
    if (!email.cc.empty()) {
        std::stringstream ss(email.cc);
        std::string cc_addr;
        while (std::getline(ss, cc_addr, ',')) {
            // Trim whitespace
            cc_addr.erase(0, cc_addr.find_first_not_of(" \t"));
            cc_addr.erase(cc_addr.find_last_not_of(" \t") + 1);
            if (!cc_addr.empty()) {
                recipients = curl_slist_append(recipients, cc_addr.c_str());
            }
        }
    }
    
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    
    // Build email payload
    std::time_t now = std::time(nullptr);
    char date_str[64];
    std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %z", std::localtime(&now));
    
    std::ostringstream payload;
    payload << "Date: " << date_str << "\r\n"
            << "From: " << (cfg.from_name.empty() ? "IPCamera" : cfg.from_name) 
            << " <" << cfg.from_email << ">\r\n"
            << "To: " << email.to << "\r\n";
    
    if (!email.cc.empty()) {
        payload << "Cc: " << email.cc << "\r\n";
    }
    
    payload << "Subject: " << email.subject << "\r\n"
            << "Content-Type: text/" << (email.is_html ? "html" : "plain") << "; charset=UTF-8\r\n"
            << "\r\n"
            << email.body << "\r\n";
    
    std::string payload_str = payload.str();
    
    // Set upload callbacks
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &payload_str);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, cfg.timeout_sec);
    
    // Send the email
    CURLcode res = curl_easy_perform(curl);
    
    // Cleanup
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        std::string error = "Failed to send email: ";
        error += curl_easy_strerror(res);
        return Result<void>::Err(error);
    }
    
    spdlog::info("[SmtpManager] Email sent successfully to {}", email.to);
    return Result<void>::Ok();
}

Result<void> SmtpManager::SendTestEmail(const std::string& to_email) {
    if (to_email.empty()) {
        return Result<void>::Err("Recipient email address is required");
    }
    
    EmailMessage email;
    email.to = to_email;
    email.subject = "Test Email from IP Camera";
    
    std::ostringstream body;
    body << "This is a test email from your IP Camera.\n\n"
         << "If you received this email, your SMTP configuration is working correctly.\n\n"
         << "Timestamp: " << std::time(nullptr);
    
    email.body = body.str();
    email.is_html = false;
    
    return SendEmail(email);
}

} // namespace networking
} // namespace ipcam
