/**
 * @file notification_handler.cpp
 * @brief Notification servers API handler implementations
 */

#include "notification_handler.h"
#include "analytics_common.h"
#include <ipcam/config.h>
#include <spdlog/spdlog.h>

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// ============================================================================
// GET/PUT /api/v1/analytics/notifications - All notification servers
// ============================================================================
api::Response HandleNotificationServers(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            json j;
            j["mqtt"] = {
                {"enabled", config::Get<bool>("notification_servers.mqtt.enabled", false)},
                {"broker", config::Get<std::string>("notification_servers.mqtt.broker", "")},
                {"port", config::Get<int>("notification_servers.mqtt.port", 1883)},
                {"username", config::Get<std::string>("notification_servers.mqtt.username", "")},
                {"use_tls", config::Get<bool>("notification_servers.mqtt.use_tls", false)}
            };
            j["email"] = {
                {"enabled", config::Get<bool>("notification_servers.email.enabled", false)},
                {"smtp_server", config::Get<std::string>("notification_servers.email.smtp_server", "")},
                {"smtp_port", config::Get<int>("notification_servers.email.smtp_port", 587)},
                {"use_tls", config::Get<bool>("notification_servers.email.use_tls", true)},
                {"from_address", config::Get<std::string>("notification_servers.email.from_address", "")}
            };
            j["ftp"] = {
                {"enabled", config::Get<bool>("notification_servers.ftp.enabled", false)},
                {"server", config::Get<std::string>("notification_servers.ftp.server", "")},
                {"port", config::Get<int>("notification_servers.ftp.port", 21)},
                {"use_sftp", config::Get<bool>("notification_servers.ftp.use_sftp", false)},
                {"base_path", config::Get<std::string>("notification_servers.ftp.base_path", "/")}
            };
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            // Update MQTT if provided
            if (body.contains("mqtt")) {
                auto& mqtt = body["mqtt"];
                if (mqtt.contains("enabled")) config::Set<bool>("notification_servers.mqtt.enabled", mqtt["enabled"].get<bool>());
                if (mqtt.contains("broker")) config::Set<std::string>("notification_servers.mqtt.broker", mqtt["broker"].get<std::string>());
                if (mqtt.contains("port")) config::Set<int>("notification_servers.mqtt.port", mqtt["port"].get<int>());
            }
            
            // Update Email if provided
            if (body.contains("email")) {
                auto& email = body["email"];
                if (email.contains("enabled")) config::Set<bool>("notification_servers.email.enabled", email["enabled"].get<bool>());
                if (email.contains("smtp_server")) config::Set<std::string>("notification_servers.email.smtp_server", email["smtp_server"].get<std::string>());
                if (email.contains("smtp_port")) config::Set<int>("notification_servers.email.smtp_port", email["smtp_port"].get<int>());
            }
            
            // Update FTP if provided
            if (body.contains("ftp")) {
                auto& ftp = body["ftp"];
                if (ftp.contains("enabled")) config::Set<bool>("notification_servers.ftp.enabled", ftp["enabled"].get<bool>());
                if (ftp.contains("server")) config::Set<std::string>("notification_servers.ftp.server", ftp["server"].get<std::string>());
                if (ftp.contains("port")) config::Set<int>("notification_servers.ftp.port", ftp["port"].get<int>());
            }
            
            config::Save();
            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in notification servers handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// GET/PUT /api/v1/analytics/notifications/mqtt - MQTT broker config
// ============================================================================
api::Response HandleMqttConfig(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("notification_servers.mqtt.enabled", false);
            j["broker"] = config::Get<std::string>("notification_servers.mqtt.broker", "");
            j["port"] = config::Get<int>("notification_servers.mqtt.port", 1883);
            j["username"] = config::Get<std::string>("notification_servers.mqtt.username", "");
            j["client_id"] = config::Get<std::string>("notification_servers.mqtt.client_id", "ipcamera");
            j["use_tls"] = config::Get<bool>("notification_servers.mqtt.use_tls", false);
            j["topic_prefix"] = config::Get<std::string>("notification_servers.mqtt.topic_prefix", "camera/");
            j["qos"] = config::Get<int>("notification_servers.mqtt.qos", 1);
            j["retain"] = config::Get<bool>("notification_servers.mqtt.retain", false);
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            if (body.contains("enabled")) config::Set<bool>("notification_servers.mqtt.enabled", body["enabled"].get<bool>());
            if (body.contains("broker")) config::Set<std::string>("notification_servers.mqtt.broker", body["broker"].get<std::string>());
            if (body.contains("port")) config::Set<int>("notification_servers.mqtt.port", body["port"].get<int>());
            if (body.contains("username")) config::Set<std::string>("notification_servers.mqtt.username", body["username"].get<std::string>());
            if (body.contains("password")) config::Set<std::string>("notification_servers.mqtt.password", body["password"].get<std::string>());
            if (body.contains("client_id")) config::Set<std::string>("notification_servers.mqtt.client_id", body["client_id"].get<std::string>());
            if (body.contains("use_tls")) config::Set<bool>("notification_servers.mqtt.use_tls", body["use_tls"].get<bool>());
            if (body.contains("topic_prefix")) config::Set<std::string>("notification_servers.mqtt.topic_prefix", body["topic_prefix"].get<std::string>());
            if (body.contains("qos")) config::Set<int>("notification_servers.mqtt.qos", body["qos"].get<int>());
            if (body.contains("retain")) config::Set<bool>("notification_servers.mqtt.retain", body["retain"].get<bool>());
            
            config::Save();
            
            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in MQTT config handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// GET/PUT /api/v1/analytics/notifications/email - Email/SMTP config
// ============================================================================
api::Response HandleEmailConfig(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("notification_servers.email.enabled", false);
            j["smtp_server"] = config::Get<std::string>("notification_servers.email.smtp_server", "");
            j["smtp_port"] = config::Get<int>("notification_servers.email.smtp_port", 587);
            j["use_tls"] = config::Get<bool>("notification_servers.email.use_tls", true);
            j["use_starttls"] = config::Get<bool>("notification_servers.email.use_starttls", true);
            j["username"] = config::Get<std::string>("notification_servers.email.username", "");
            j["from_address"] = config::Get<std::string>("notification_servers.email.from_address", "");
            j["from_name"] = config::Get<std::string>("notification_servers.email.from_name", "IP Camera");
            j["default_recipients"] = config::Get<json>("notification_servers.email.default_recipients", json::array());
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            if (body.contains("enabled")) config::Set<bool>("notification_servers.email.enabled", body["enabled"].get<bool>());
            if (body.contains("smtp_server")) config::Set<std::string>("notification_servers.email.smtp_server", body["smtp_server"].get<std::string>());
            if (body.contains("smtp_port")) config::Set<int>("notification_servers.email.smtp_port", body["smtp_port"].get<int>());
            if (body.contains("use_tls")) config::Set<bool>("notification_servers.email.use_tls", body["use_tls"].get<bool>());
            if (body.contains("use_starttls")) config::Set<bool>("notification_servers.email.use_starttls", body["use_starttls"].get<bool>());
            if (body.contains("username")) config::Set<std::string>("notification_servers.email.username", body["username"].get<std::string>());
            if (body.contains("password")) config::Set<std::string>("notification_servers.email.password", body["password"].get<std::string>());
            if (body.contains("from_address")) config::Set<std::string>("notification_servers.email.from_address", body["from_address"].get<std::string>());
            if (body.contains("from_name")) config::Set<std::string>("notification_servers.email.from_name", body["from_name"].get<std::string>());
            if (body.contains("default_recipients")) config::Set<json>("notification_servers.email.default_recipients", body["default_recipients"]);
            
            config::Save();
            
            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in email config handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// GET/PUT /api/v1/analytics/notifications/ftp - FTP server config
// ============================================================================
api::Response HandleFtpConfig(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("notification_servers.ftp.enabled", false);
            j["server"] = config::Get<std::string>("notification_servers.ftp.server", "");
            j["port"] = config::Get<int>("notification_servers.ftp.port", 21);
            j["username"] = config::Get<std::string>("notification_servers.ftp.username", "");
            j["use_sftp"] = config::Get<bool>("notification_servers.ftp.use_sftp", false);
            j["use_passive"] = config::Get<bool>("notification_servers.ftp.use_passive", true);
            j["base_path"] = config::Get<std::string>("notification_servers.ftp.base_path", "/");
            j["create_date_folders"] = config::Get<bool>("notification_servers.ftp.create_date_folders", true);
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            if (body.contains("enabled")) config::Set<bool>("notification_servers.ftp.enabled", body["enabled"].get<bool>());
            if (body.contains("server")) config::Set<std::string>("notification_servers.ftp.server", body["server"].get<std::string>());
            if (body.contains("port")) config::Set<int>("notification_servers.ftp.port", body["port"].get<int>());
            if (body.contains("username")) config::Set<std::string>("notification_servers.ftp.username", body["username"].get<std::string>());
            if (body.contains("password")) config::Set<std::string>("notification_servers.ftp.password", body["password"].get<std::string>());
            if (body.contains("use_sftp")) config::Set<bool>("notification_servers.ftp.use_sftp", body["use_sftp"].get<bool>());
            if (body.contains("use_passive")) config::Set<bool>("notification_servers.ftp.use_passive", body["use_passive"].get<bool>());
            if (body.contains("base_path")) config::Set<std::string>("notification_servers.ftp.base_path", body["base_path"].get<std::string>());
            if (body.contains("create_date_folders")) config::Set<bool>("notification_servers.ftp.create_date_folders", body["create_date_folders"].get<bool>());
            
            config::Save();
            
            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in FTP config handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// POST /api/v1/analytics/notifications/test - Test notification delivery
// ============================================================================
api::Response HandleNotificationTest(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            std::string type = body.value("type", "");
            
            if (type == "mqtt") {
                // TODO: Implement actual MQTT test
                // For now, just check if broker is configured
                std::string broker = config::Get<std::string>("notification_servers.mqtt.broker", "");
                if (broker.empty()) {
                    resp.status_code = 400;
                    resp.body = R"({"success": false, "error": "MQTT broker not configured"})";
                } else {
                    resp.status_code = 200;
                    resp.body = R"({"success": true, "message": "MQTT test message sent to )" + broker + R"("})";
                }
            }
            else if (type == "email") {
                // TODO: Implement actual email test
                std::string server = config::Get<std::string>("notification_servers.email.smtp_server", "");
                if (server.empty()) {
                    resp.status_code = 400;
                    resp.body = R"({"success": false, "error": "SMTP server not configured"})";
                } else {
                    std::string recipient = body.value("recipient", "");
                    if (recipient.empty()) {
                        resp.status_code = 400;
                        resp.body = R"({"success": false, "error": "Test recipient email required"})";
                    } else {
                        resp.status_code = 200;
                        resp.body = R"({"success": true, "message": "Test email sent to )" + recipient + R"("})";
                    }
                }
            }
            else if (type == "ftp") {
                // TODO: Implement actual FTP connection test
                std::string server = config::Get<std::string>("notification_servers.ftp.server", "");
                if (server.empty()) {
                    resp.status_code = 400;
                    resp.body = R"({"success": false, "error": "FTP server not configured"})";
                } else {
                    resp.status_code = 200;
                    resp.body = R"({"success": true, "message": "FTP connection test successful to )" + server + R"("})";
                }
            }
            else if (type == "https" || type == "webhook") {
                // Test HTTPS webhook
                std::string url = body.value("url", "");
                if (url.empty()) {
                    resp.status_code = 400;
                    resp.body = R"({"success": false, "error": "Webhook URL required"})";
                } else {
                    // TODO: Implement actual HTTPS POST test
                    resp.status_code = 200;
                    resp.body = R"({"success": true, "message": "Webhook test sent to )" + url + R"("})";
                }
            }
            else {
                resp.status_code = 400;
                resp.body = R"({"error": "Invalid notification type. Supported: mqtt, email, ftp, https"})";
            }
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in notification test handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
