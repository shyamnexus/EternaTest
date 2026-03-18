/**
 * @file email_action.cpp
 * @brief Email notification action handler implementation using SmtpManager
 */

#include "ipcam/action_handler.h"
#include "ipcam/smtp_manager.h"
#include <spdlog/spdlog.h>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

namespace ipcam {
namespace events {

// Static SmtpManager instance for email handler
static networking::SmtpManager& GetSmtpManager() {
    static networking::SmtpManager instance;
    return instance;
}

ActionResult EmailActionHandler::Execute(const Event& event, const Action& action) {
    ActionResult result;
    result.execution_time_ms = 0;
    
    const auto* cfg = action.GetConfig<EmailActionConfig>();
    if (!cfg) {
        result.success = false;
        result.error_message = "Invalid configuration for email action";
        return result;
    }
    
    if (cfg->recipients.empty()) {
        result.success = false;
        result.error_message = "No email recipients configured";
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Check if SMTP is configured
    auto& smtp_manager = GetSmtpManager();
    auto smtp_config_result = smtp_manager.GetConfig();
    if (!smtp_config_result || !smtp_config_result.value.enabled) {
        result.success = false;
        result.error_message = "SMTP not configured or disabled";
        return result;
    }
    
    spdlog::info("[EmailActionHandler] Sending email notification for event: {}", event.id);
    
    // Build email message
    networking::SmtpManager::EmailMessage message;
    
    // Join recipients with comma
    std::string recipients_str;
    for (size_t i = 0; i < cfg->recipients.size(); ++i) {
        if (i > 0) recipients_str += ", ";
        recipients_str += cfg->recipients[i];
    }
    message.to = recipients_str;
    
    message.subject = SubstitutePlaceholders(cfg->subject_template, event);
    
    // Build body with optional HTML formatting
    std::string body = SubstitutePlaceholders(cfg->body_template, event);
    
    // Add event details to body
    body += "\n\n--- Event Details ---\n";
    body += "Event ID: " + event.id + "\n";
    body += "Type: " + event.GetTypeString() + "\n";
    body += "Time: " + event.ToIsoTimestamp() + "\n";
    if (!event.source.empty()) {
        body += "Source: " + event.source + "\n";
    }
    
    message.body = body;
    message.is_html = false;
    
    // Note: Attachments would need SmtpManager enhancement to support
    // For now, log attachment info
    if (cfg->attach_snapshot && event.snapshot_path.has_value()) {
        std::string snapshot = event.snapshot_path.value();
        if (fs::exists(snapshot)) {
            spdlog::info("[EmailActionHandler] Snapshot available: {}", snapshot);
            // TODO: Add attachment support to SmtpManager
        }
    }
    
    // Send email
    auto send_result = smtp_manager.SendEmail(message);
    
    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    if (send_result) {
        result.success = true;
        result.output = "Email sent to " + recipients_str;
        spdlog::info("[EmailActionHandler] Email sent successfully to {}", recipients_str);
    } else {
        result.success = false;
        result.error_message = send_result.error;
        spdlog::error("[EmailActionHandler] Failed to send email: {}", result.error_message);
    }
    
    return result;
}

bool EmailActionHandler::IsAvailable() const {
    auto& smtp_manager = GetSmtpManager();
    auto config_result = smtp_manager.GetConfig();
    return config_result && config_result.value.enabled;
}

std::string EmailActionHandler::GetStatus() const {
    auto& smtp_manager = GetSmtpManager();
    auto config_result = smtp_manager.GetConfig();
    
    if (!config_result) {
        return "SMTP not configured";
    }
    
    const auto& config = config_result.value;
    if (!config.enabled) {
        return "SMTP disabled";
    }
    
    return "SMTP configured: " + config.server + ":" + std::to_string(config.port);
}

} // namespace events
} // namespace ipcam
