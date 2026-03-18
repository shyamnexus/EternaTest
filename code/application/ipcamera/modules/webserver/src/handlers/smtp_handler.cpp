#include "smtp_handler.h"
#include <ipcam/network_manager.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

api::Response HandleSmtp(const api::RequestContext& ctx) {
    auto& nm = networking::NetworkManager::Instance();
    api::Response resp;
    
    if (ctx.method == "GET") {
        // Get SMTP configuration
        auto result = nm.GetSmtpConfig();
        
        if (result.success) {
            try {
                auto& cfg = result.value;
                json j;
                j["enabled"] = cfg.enabled;
                j["server"] = cfg.server;
                j["port"] = cfg.port;
                j["use_ssl"] = cfg.use_ssl;
                j["username"] = cfg.username;
                j["password"] = cfg.password;
                j["from_email"] = cfg.from_email;
                j["from_name"] = cfg.from_name;
                j["receiver_emails"] = json::array();
                for (const auto& email : cfg.receiver_emails) {
                    j["receiver_emails"].push_back(email);
                }
                j["timeout_sec"] = cfg.timeout_sec;
                
                resp.status_code = 200;
                resp.body = j.dump(2);
            } catch (const std::exception& e) {
                spdlog::error("Failed to serialize SMTP config: {}", e.what());
                json err;
                err["error"] = "Failed to serialize SMTP configuration";
                err["details"] = e.what();
                resp.status_code = 500;
                resp.body = err.dump(2);
            }
        } else {
            json err;
            err["error"] = "Failed to get SMTP configuration";
            err["details"] = result.error;
            resp.status_code = 500;
            resp.body = err.dump(2);
        }
        
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        // Update SMTP configuration
        try {
            auto j = json::parse(ctx.body);
            
            networking::NetworkManager::SmtpConfig cfg;
            cfg.enabled = j.value("enabled", false);
            cfg.server = j.value("server", "");
            cfg.port = j.value("port", 587);
            cfg.use_ssl = j.value("use_ssl", 2);
            cfg.username = j.value("username", "");
            cfg.password = j.value("password", "");
            cfg.from_email = j.value("from_email", "");
            cfg.from_name = j.value("from_name", "IPCamera");
            
            // Parse receiver_emails array (up to 5)
            if (j.contains("receiver_emails") && j["receiver_emails"].is_array()) {
                int count = 0;
                for (const auto& email : j["receiver_emails"]) {
                    if (count >= 5) break; // Limit to 5 receivers
                    if (email.is_string() && !email.get<std::string>().empty()) {
                        cfg.receiver_emails.push_back(email.get<std::string>());
                        count++;
                    }
                }
            }
            
            cfg.timeout_sec = j.value("timeout_sec", 30);
            
            auto result = nm.SetSmtpConfig(cfg);
            
            if (result.success) {
                json response;
                response["status"] = "success";
                response["message"] = "SMTP configuration updated successfully";
                resp.status_code = 200;
                resp.body = response.dump(2);
            } else {
                json err;
                err["error"] = "Failed to set SMTP configuration";
                err["details"] = result.error;
                resp.status_code = 500;
                resp.body = err.dump(2);
            }
            
        } catch (const std::exception& e) {
            json err;
            err["error"] = "Invalid JSON format";
            err["details"] = e.what();
            resp.status_code = 400;
            resp.body = err.dump(2);
        }
    } else {
        json err;
        err["error"] = "Method not allowed";
        resp.status_code = 405;
        resp.body = err.dump(2);
    }
    
    return resp;
}

api::Response HandleSmtpTest(const api::RequestContext& ctx) {
    auto& nm = networking::NetworkManager::Instance();
    api::Response resp;
    
    if (ctx.method == "POST") {
        try {
            auto j = json::parse(ctx.body);
            
            // Check if custom config is provided for testing
            if (j.contains("config")) {
                // Test with custom configuration
                auto& cfg_json = j["config"];
                
                networking::NetworkManager::SmtpConfig cfg;
                cfg.server = cfg_json.value("server", "");
                cfg.port = cfg_json.value("port", 587);
                cfg.use_ssl = cfg_json.value("use_ssl", 2);
                cfg.username = cfg_json.value("username", "");
                cfg.password = cfg_json.value("password", "");
                cfg.from_email = cfg_json.value("from_email", "");
                cfg.from_name = cfg_json.value("from_name", "IPCamera");
                
                // Parse receiver_emails array (up to 5)
                if (cfg_json.contains("receiver_emails") && cfg_json["receiver_emails"].is_array()) {
                    int count = 0;
                    for (const auto& email : cfg_json["receiver_emails"]) {
                        if (count >= 5) break;
                        if (email.is_string() && !email.get<std::string>().empty()) {
                            cfg.receiver_emails.push_back(email.get<std::string>());
                            count++;
                        }
                    }
                }
                
                cfg.enabled = cfg_json.value("enabled", true);
                cfg.timeout_sec = cfg_json.value("timeout_sec", 30);
                
                // Temporarily set config for testing
                auto set_result = nm.SetSmtpConfig(cfg);
                if (!set_result.success) {
                    json err;
                    err["error"] = "Failed to apply test configuration";
                    err["details"] = set_result.error;
                    resp.status_code = 500;
                    resp.body = err.dump(2);
                    return resp;
                }
            }
            
            // Test connection
            auto result = nm.TestSmtpConnection();
            
            json response;
            if (result.success) {
                response["status"] = "success";
                response["message"] = "SMTP connection test successful";
                resp.status_code = 200;
            } else {
                response["status"] = "failed";
                response["error"] = result.error;
                resp.status_code = 500;
            }
            
            resp.body = response.dump(2);
            
        } catch (const std::exception& e) {
            json err;
            err["error"] = "Invalid request format";
            err["details"] = e.what();
            resp.status_code = 400;
            resp.body = err.dump(2);
        }
    } else {
        json err;
        err["error"] = "Method not allowed";
        resp.status_code = 405;
        resp.body = err.dump(2);
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
