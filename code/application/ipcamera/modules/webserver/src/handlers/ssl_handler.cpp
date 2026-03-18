#include "ssl_handler.h"
#include <ipcam/ssl_manager.h>
#include <ipcam/nginx_manager.h>
#include <ipcam/network_manager.h>
#include <ipcam/go2rtc_manager.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

api::Response HandleSsl(const api::RequestContext& ctx) {
    auto& ssl_mgr = networking::SslManager::Instance();
    api::Response resp;
    
    if (ctx.method == "GET") {
        json j;
        
        std::string cert_path = config::Get<std::string>("network.https.ssl_certificate", "/etc/nginx/ssl/Server.crt");
        std::string key_path = config::Get<std::string>("network.https.ssl_certificate_key", "/etc/nginx/ssl/Server.key");
        
        j["certificate_path"] = cert_path;
        j["key_path"] = key_path;
        
        // Get certificate information
        auto cert_info = ssl_mgr.GetCertificateInfo(cert_path);
        if (cert_info.success) {
            j["info"] = json::parse(cert_info.value);
        } else {
            j["info"] = {
                {"error", cert_info.error}
            };
        }
        
        resp.status_code = 200;
        resp.body = j.dump(2);
        
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        // Set certificate paths (for backward compatibility)
        try {
            auto req_json = json::parse(ctx.body);
            
            if (req_json.contains("certificate_path") && req_json.contains("key_path")) {
                std::string cert_path = req_json["certificate_path"];
                std::string key_path = req_json["key_path"];
                
                auto result = ssl_mgr.SetCertificate(cert_path, key_path);
                
                if (result.success) {
                    // Reload nginx to use new certificates
                    auto& nginx_mgr = networking::NginxManager::Instance();
                    nginx_mgr.GenerateConfig();
                    nginx_mgr.Reload();
                    
                    // Restart go2rtc to use new certificates (uses same cert files)
                    auto& go2rtc_mgr = streaming::Go2rtcManager::Instance();
                    auto go2rtc_result = go2rtc_mgr.Restart();
                    if (!go2rtc_result.success) {
                        spdlog::warn("go2rtc restart after cert update failed: {}", go2rtc_result.error);
                    }
                    
                    resp.status_code = 200;
                    resp.body = json({
                        {"success", true},
                        {"message", "SSL certificate updated successfully"}
                    }).dump(2);
                } else {
                    resp.status_code = 400;
                    resp.body = json({
                        {"success", false},
                        {"error", result.error}
                    }).dump(2);
                }
            } else {
                resp.status_code = 400;
                resp.body = json({
                    {"success", false},
                    {"error", "Both certificate_path and key_path are required"}
                }).dump(2);
            }
        } catch (const json::exception& e) {
            resp.status_code = 400;
            resp.body = json({
                {"success", false},
                {"error", std::string("Invalid JSON: ") + e.what()}
            }).dump(2);
        }
    } else {
        resp.status_code = 405;
        resp.body = json({{"error", "Method not allowed"}}).dump(2);
    }
    
    resp.content_type = "application/json";
    return resp;
}

api::Response HandleSslUpload(const api::RequestContext& ctx) {
    auto& ssl_mgr = networking::SslManager::Instance();
    api::Response resp;
    resp.content_type = "application/json";
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json({{"error", "Method not allowed"}}).dump(2);
        return resp;
    }
    
    try {
        auto req_json = json::parse(ctx.body);
        
        if (!req_json.contains("certificate") || !req_json.contains("key")) {
            resp.status_code = 400;
            resp.body = json({
                {"success", false},
                {"error", "Both 'certificate' and 'key' fields are required"}
            }).dump(2);
            return resp;
        }
        
        std::string cert_content = req_json["certificate"];
        std::string key_content = req_json["key"];
        std::string cert_name = req_json.value("name", "custom");
        
        // Validate and upload certificate
        auto result = ssl_mgr.UploadCertificate(cert_content, key_content, cert_name);
        
        if (result.success) {
            // Regenerate nginx config and reload
            auto& nginx_mgr = networking::NginxManager::Instance();
            auto gen_result = nginx_mgr.GenerateConfig();
            
            if (gen_result.success) {
                auto reload_result = nginx_mgr.Reload();
                
                // Also restart go2rtc to use new certificates
                auto& go2rtc_mgr = streaming::Go2rtcManager::Instance();
                auto go2rtc_result = go2rtc_mgr.Restart();
                if (!go2rtc_result.success) {
                    spdlog::warn("go2rtc restart after cert upload failed: {}", go2rtc_result.error);
                }
                
                if (reload_result.success) {
                    resp.status_code = 200;
                    resp.body = json({
                        {"success", true},
                        {"message", "SSL certificate uploaded and applied successfully"}
                    }).dump(2);
                } else {
                    resp.status_code = 500;
                    resp.body = json({
                        {"success", true},
                        {"message", "Certificate uploaded but nginx reload failed: " + reload_result.error},
                        {"warning", "Please restart nginx manually"}
                    }).dump(2);
                }
            } else {
                resp.status_code = 500;
                resp.body = json({
                    {"success", true},
                    {"message", "Certificate uploaded but config generation failed: " + gen_result.error}
                }).dump(2);
            }
        } else {
            resp.status_code = 400;
            resp.body = json({
                {"success", false},
                {"error", result.error}
            }).dump(2);
        }
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({
            {"success", false},
            {"error", std::string("Invalid JSON: ") + e.what()}
        }).dump(2);
    } catch (const std::exception& e) {
        resp.status_code = 500;
        resp.body = json({
            {"success", false},
            {"error", std::string("Server error: ") + e.what()}
        }).dump(2);
    }
    
    return resp;
}

api::Response HandleSslGenerate(const api::RequestContext& ctx) {
    auto& ssl_mgr = networking::SslManager::Instance();
    api::Response resp;
    resp.content_type = "application/json";
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json({{"error", "Method not allowed"}}).dump(2);
        return resp;
    }
    
    try {
        auto req_json = json::parse(ctx.body);
        
        int validity_days = req_json.value("validity_days", 3650);
        int key_bits = req_json.value("key_bits", 2048);
        
        // Get hostname - either from request or from NetworkManager
        std::string hostname;
        if (req_json.contains("common_name") && !req_json["common_name"].get<std::string>().empty()) {
            hostname = req_json["common_name"].get<std::string>();
        } else {
            auto hostname_result = networking::NetworkManager::Instance().GetHostname();
            hostname = hostname_result.success ? hostname_result.value : "ipcamera";
        }
        
        std::string ssl_dir = config::Get<std::string>("network.https.ssl_certs_dir", "/etc/ssl");
        std::string cert_path = ssl_dir + "/generated.crt";
        std::string key_path = ssl_dir + "/generated.key";
        
        // Generate certificate with hostname
        auto result = ssl_mgr.GenerateSelfSignedCertificate(cert_path, key_path, hostname, validity_days, key_bits);
        
        if (result.success) {
            // Set as active certificate
            auto set_result = ssl_mgr.SetCertificate(cert_path, key_path);
            
            if (set_result.success) {
                // Reload nginx
                auto& nginx_mgr = networking::NginxManager::Instance();
                nginx_mgr.GenerateConfig();
                nginx_mgr.Reload();
                
                // Restart go2rtc to use new certificates (uses same cert files as nginx)
                auto& go2rtc_mgr = streaming::Go2rtcManager::Instance();
                auto go2rtc_result = go2rtc_mgr.Restart();
                if (!go2rtc_result.success) {
                    spdlog::warn("go2rtc restart after cert generation failed: {}", go2rtc_result.error);
                }
                
                resp.status_code = 200;
                resp.body = json({
                    {"success", true},
                    {"message", "Self-signed certificate generated successfully"},
                    {"certificate_path", cert_path},
                    {"key_path", key_path},
                    {"validity_days", validity_days}
                }).dump(2);
            } else {
                resp.status_code = 500;
                resp.body = json({
                    {"success", false},
                    {"error", "Certificate generated but failed to set active: " + set_result.error}
                }).dump(2);
            }
        } else {
            resp.status_code = 500;
            resp.body = json({
                {"success", false},
                {"error", result.error}
            }).dump(2);
        }
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({
            {"success", false},
            {"error", std::string("Invalid JSON: ") + e.what()}
        }).dump(2);
    } catch (const std::exception& e) {
        resp.status_code = 500;
        resp.body = json({
            {"success", false},
            {"error", std::string("Server error: ") + e.what()}
        }).dump(2);
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
