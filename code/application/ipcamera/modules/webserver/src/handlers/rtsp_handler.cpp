/**
 * @file rtsp_handler.cpp
 * @brief RTSP Server Configuration API Implementation
 */

#include "rtsp_handler.h"
#include "ipcam/config.h"
#include "ipcam/streaming.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// Helper to get local IP address for URL display
static std::string getLocalIpAddress() {
    // Try to get from network interface
    std::string ip = "192.168.0.2";  // Default fallback
    
    FILE* fp = popen("ip route get 1 2>/dev/null | awk '{print $7}' | head -1", "r");
    if (fp) {
        char buf[64];
        if (fgets(buf, sizeof(buf), fp)) {
            std::string result(buf);
            // Trim whitespace
            size_t end = result.find_last_not_of(" \t\n\r");
            if (end != std::string::npos) {
                ip = result.substr(0, end + 1);
            }
        }
        pclose(fp);
    }
    return ip;
}

api::Response HandleRtsp(const api::RequestContext& ctx) {
    api::Response resp;
    resp.content_type = "application/json";
    
    try {
        if (ctx.method == "GET") {
            // Return current RTSP configuration
            json j;
            
            j["enabled"] = config::Get<bool>("streaming.rtsp.enabled", true);
            j["port"] = config::Get<int>("streaming.rtsp.port", 554);
            j["http_tunnel_port"] = config::Get<int>("streaming.rtsp.http_tunnel_port", 8554);
            j["max_connections"] = config::Get<int>("streaming.rtsp.max_connections", 10);
            
            // Auth settings (password not returned)
            json auth;
            std::string auth_mode = config::Get<std::string>("streaming.rtsp.auth.mode", "none");
            auth["mode"] = auth_mode;
            auth["realm"] = config::Get<std::string>("streaming.rtsp.auth.realm", "IP Camera");
            auth["username"] = config::Get<std::string>("streaming.rtsp.auth.username", "");
            // Note: go2rtc (WebRTC) only supports Basic auth, not Digest
            auth["go2rtc_compatible"] = (auth_mode == "none" || auth_mode == "basic");
            j["auth"] = auth;
            
            // Runtime status
            json status;
            status["running"] = streaming::IsRunning();
            status["client_count"] = streaming::GetRtspServer().GetClientCount();
            
            // Stream info
            json streams = json::array();
            std::string local_ip = getLocalIpAddress();
            int port = config::Get<int>("streaming.rtsp.port", 554);
            std::string port_suffix = (port == 554) ? "" : ":" + std::to_string(port);
            
            for (int i = 0; i < 3; i++) {
                std::string prefix = "media.video" + std::to_string(i + 1);
                if (config::Get<bool>(prefix + ".enabled", i < 2)) {
                    json stream;
                    stream["name"] = "stream" + std::to_string(i);
                    stream["url"] = "rtsp://" + local_ip + port_suffix + "/stream" + std::to_string(i);
                    stream["codec"] = config::Get<std::string>(prefix + ".codec", "h264");
                    streams.push_back(stream);
                }
            }
            status["streams"] = streams;
            j["status"] = status;
            
            resp.status_code = 200;
            resp.body = j.dump(2);
            
        } else if (ctx.method == "PUT" || ctx.method == "POST") {
            // Update RTSP configuration
            json body = json::parse(ctx.body);
            bool needs_restart = false;
            
            if (body.contains("enabled")) {
                config::Set("streaming.rtsp.enabled", body["enabled"].get<bool>());
                needs_restart = true;
            }
            
            if (body.contains("port")) {
                int new_port = body["port"].get<int>();
                if (new_port < 1 || new_port > 65535) {
                    throw std::runtime_error("Invalid port number");
                }
                config::Set("streaming.rtsp.port", new_port);
                needs_restart = true;
            }
            
            if (body.contains("http_tunnel_port")) {
                int new_port = body["http_tunnel_port"].get<int>();
                if (new_port < 0 || new_port > 65535) {
                    throw std::runtime_error("Invalid HTTP tunnel port");
                }
                config::Set("streaming.rtsp.http_tunnel_port", new_port);
                needs_restart = true;
            }
            
            if (body.contains("max_connections")) {
                int max_conn = body["max_connections"].get<int>();
                if (max_conn < 1 || max_conn > 100) {
                    throw std::runtime_error("max_connections must be 1-100");
                }
                config::Set("streaming.rtsp.max_connections", max_conn);
            }
            
            // Handle auth settings inline
            if (body.contains("auth")) {
                auto auth = body["auth"];
                
                if (auth.contains("mode")) {
                    std::string mode = auth["mode"].get<std::string>();
                    if (mode != "none" && mode != "basic" && mode != "digest") {
                        throw std::runtime_error("Invalid auth mode. Use: none, basic, digest");
                    }
                    config::Set("streaming.rtsp.auth.mode", mode);
                    needs_restart = true;
                }
                
                if (auth.contains("realm")) {
                    config::Set("streaming.rtsp.auth.realm", auth["realm"].get<std::string>());
                    needs_restart = true;
                }
                
                if (auth.contains("username")) {
                    config::Set("streaming.rtsp.auth.username", auth["username"].get<std::string>());
                    needs_restart = true;
                }
                
                if (auth.contains("password")) {
                    config::Set("streaming.rtsp.auth.password", auth["password"].get<std::string>());
                    needs_restart = true;
                }
            }
            
            // Save configuration
            config::Save();
            
            // Update go2rtc config if it's enabled (auth changes)
            if (config::Get<bool>("streaming.go2rtc.enabled", false)) {
                streaming::Go2rtcManager::Instance().GenerateConfig();
            }
            
            json result;
            result["success"] = true;
            result["message"] = needs_restart ? 
                "Configuration updated. Restart RTSP server to apply changes." :
                "Configuration updated.";
            result["needs_restart"] = needs_restart;
            
            resp.status_code = 200;
            resp.body = result.dump(2);
            
        } else {
            json error;
            error["error"] = "Method not allowed";
            resp.status_code = 405;
            resp.body = error.dump();
        }
        
    } catch (const json::exception& e) {
        json error;
        error["error"] = "Invalid JSON";
        error["message"] = e.what();
        resp.status_code = 400;
        resp.body = error.dump();
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Bad request";
        error["message"] = e.what();
        resp.status_code = 400;
        resp.body = error.dump();
    }
    
    return resp;
}

api::Response HandleRtspAuth(const api::RequestContext& ctx) {
    api::Response resp;
    resp.content_type = "application/json";
    
    try {
        if (ctx.method == "GET") {
            json j;
            j["mode"] = config::Get<std::string>("streaming.rtsp.auth.mode", "none");
            j["realm"] = config::Get<std::string>("streaming.rtsp.auth.realm", "IP Camera");
            j["username"] = config::Get<std::string>("streaming.rtsp.auth.username", "");
            // Password is not returned for security
            j["password_set"] = !config::Get<std::string>("streaming.rtsp.auth.password", "").empty();
            
            resp.status_code = 200;
            resp.body = j.dump(2);
            
        } else if (ctx.method == "PUT" || ctx.method == "POST") {
            json body = json::parse(ctx.body);
            
            if (body.contains("mode")) {
                std::string mode = body["mode"].get<std::string>();
                if (mode != "none" && mode != "basic" && mode != "digest") {
                    throw std::runtime_error("Invalid auth mode. Use: none, basic, digest");
                }
                config::Set("streaming.rtsp.auth.mode", mode);
            }
            
            if (body.contains("realm")) {
                config::Set("streaming.rtsp.auth.realm", body["realm"].get<std::string>());
            }
            
            if (body.contains("username")) {
                std::string username = body["username"].get<std::string>();
                if (username.length() > 32) {
                    throw std::runtime_error("Username too long (max 32 chars)");
                }
                config::Set("streaming.rtsp.auth.username", username);
            }
            
            if (body.contains("password")) {
                std::string password = body["password"].get<std::string>();
                if (password.length() > 64) {
                    throw std::runtime_error("Password too long (max 64 chars)");
                }
                config::Set("streaming.rtsp.auth.password", password);
            }
            
            config::Save();
            
            // Restart RTSP server to apply new auth settings
            spdlog::info("RTSP auth settings changed, restarting server...");
            streaming::Stop();
            streaming::GetRtspServer().LoadConfig();
            streaming::Start();
            
            // Update go2rtc if enabled
            if (config::Get<bool>("streaming.go2rtc.enabled", false)) {
                streaming::Go2rtcManager::Instance().Restart();
            }
            
            json result;
            result["success"] = true;
            result["message"] = "Authentication settings updated and applied.";
            
            resp.status_code = 200;
            resp.body = result.dump(2);
            
        } else {
            json error;
            error["error"] = "Method not allowed";
            resp.status_code = 405;
            resp.body = error.dump();
        }
        
    } catch (const json::exception& e) {
        json error;
        error["error"] = "Invalid JSON";
        error["message"] = e.what();
        resp.status_code = 400;
        resp.body = error.dump();
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Bad request";
        error["message"] = e.what();
        resp.status_code = 400;
        resp.body = error.dump();
    }
    
    return resp;
}

api::Response HandleRtspControl(const api::RequestContext& ctx) {
    api::Response resp;
    resp.content_type = "application/json";
    
    try {
        if (ctx.method != "POST") {
            json error;
            error["error"] = "Method not allowed";
            error["message"] = "Use POST to control RTSP server";
            resp.status_code = 405;
            resp.body = error.dump();
            return resp;
        }
        
        json body = json::parse(ctx.body);
        
        if (!body.contains("action")) {
            throw std::runtime_error("Missing 'action' field");
        }
        
        std::string action = body["action"].get<std::string>();
        json result;
        
        if (action == "start") {
            if (streaming::IsRunning()) {
                result["success"] = true;
                result["message"] = "RTSP server already running";
            } else {
                streaming::GetRtspServer().LoadConfig();
                if (streaming::Start()) {
                    result["success"] = true;
                    result["message"] = "RTSP server started";
                } else {
                    result["success"] = false;
                    result["message"] = "Failed to start RTSP server";
                }
            }
            
        } else if (action == "stop") {
            streaming::Stop();
            result["success"] = true;
            result["message"] = "RTSP server stopped";
            
        } else if (action == "restart") {
            spdlog::info("RTSP control: Restarting RTSP server...");
            streaming::Stop();
            streaming::GetRtspServer().LoadConfig();
            if (streaming::Start()) {
                result["success"] = true;
                result["message"] = "RTSP server restarted";
            } else {
                result["success"] = false;
                result["message"] = "Failed to restart RTSP server";
            }
            
        } else if (action == "reload") {
            // Reload config without full restart
            streaming::GetRtspServer().LoadConfig();
            result["success"] = true;
            result["message"] = "Configuration reloaded (some changes require restart)";
            
        } else {
            throw std::runtime_error("Invalid action. Use: start, stop, restart, reload");
        }
        
        result["running"] = streaming::IsRunning();
        resp.status_code = 200;
        resp.body = result.dump(2);
        
    } catch (const json::exception& e) {
        json error;
        error["error"] = "Invalid JSON";
        error["message"] = e.what();
        resp.status_code = 400;
        resp.body = error.dump();
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Bad request";
        error["message"] = e.what();
        resp.status_code = 400;
        resp.body = error.dump();
    }
    
    return resp;
}

api::Response HandleRtspStatus(const api::RequestContext& ctx) {
    (void)ctx;  // Unused - status query doesn't need request context
    api::Response resp;
    resp.content_type = "application/json";
    
    try {
        json j;
        
        j["running"] = streaming::IsRunning();
        j["client_count"] = streaming::GetRtspServer().GetClientCount();
        
        // Port info
        j["port"] = config::Get<int>("streaming.rtsp.port", 554);
        j["http_tunnel_port"] = config::Get<int>("streaming.rtsp.http_tunnel_port", 8554);
        
        // Auth status
        std::string auth_mode = config::Get<std::string>("streaming.rtsp.auth.mode", "none");
        j["auth_enabled"] = (auth_mode != "none");
        j["auth_mode"] = auth_mode;
        
        // Stream URLs
        std::string local_ip = getLocalIpAddress();
        int port = config::Get<int>("streaming.rtsp.port", 554);
        std::string port_suffix = (port == 554) ? "" : ":" + std::to_string(port);
        
        json streams = json::array();
        for (int i = 0; i < 3; i++) {
            std::string prefix = "media.video" + std::to_string(i + 1);
            if (config::Get<bool>(prefix + ".enabled", i < 2)) {
                json stream;
                stream["name"] = "stream" + std::to_string(i);
                stream["url"] = "rtsp://" + local_ip + port_suffix + "/stream" + std::to_string(i);
                stream["codec"] = config::Get<std::string>(prefix + ".codec", "h264");
                stream["resolution"] = std::to_string(config::Get<int>(prefix + ".width", 1920)) + 
                                       "x" + std::to_string(config::Get<int>(prefix + ".height", 1080));
                stream["fps"] = config::Get<int>(prefix + ".fps", 30);
                streams.push_back(stream);
            }
        }
        j["streams"] = streams;
        
        // IPv6 support info
        j["ipv6_enabled"] = true;  // Dual-stack is now always enabled via Live555 patch
        
        resp.status_code = 200;
        resp.body = j.dump(2);
        
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Internal error";
        error["message"] = e.what();
        resp.status_code = 500;
        resp.body = error.dump();
    }
    
    return resp;
}

api::Response HandleRtspClients(const api::RequestContext& ctx) {
    (void)ctx;  // Unused - client list query doesn't need request context
    api::Response resp;
    resp.content_type = "application/json";
    
    try {
        json j;
        j["count"] = streaming::GetRtspServer().GetClientCount();
        
        // Note: Live555 doesn't provide easy access to client list
        // Future enhancement: maintain client list in RtspServer class
        j["clients"] = json::array();
        j["note"] = "Detailed client list not yet implemented";
        
        resp.status_code = 200;
        resp.body = j.dump(2);
        
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Internal error";
        error["message"] = e.what();
        resp.status_code = 500;
        resp.body = error.dump();
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
