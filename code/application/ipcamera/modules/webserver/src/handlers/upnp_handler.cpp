#include "upnp_handler.h"
#include <ipcam/network_manager.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <cstring>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

api::Response HandleUpnp(const api::RequestContext& ctx) {
    api::Response resp;
    auto& netmgr = networking::NetworkManager::Instance();
    
    if (ctx.method == "GET") {
        // Get UPnP configuration only (don't discover device on every GET)
        json result;
        
        auto config_result = netmgr.GetUpnpConfig();
        if (config_result.success) {
            const auto& cfg = config_result.value;
            result["config"] = {
                {"enabled", cfg.enabled},
                {"timeout_ms", cfg.timeout_ms},
                {"auto_map", cfg.auto_map}
            };
        } else {
            result["config"]["error"] = config_result.error;
        }
        
        // Don't auto-discover on GET - it's too slow
        // Use POST /api/v1/upnp/discover for explicit discovery
        result["device"] = {
            {"note", "Use POST /api/v1/upnp/discover to detect UPnP gateway"}
        };
        
        resp.status_code = 200;
        resp.body = result.dump(2);
        
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        // Update UPnP configuration
        try {
            auto j = json::parse(ctx.body);
            
            networking::NetworkManager::UpnpConfig cfg;
            if (j.contains("enabled")) cfg.enabled = j["enabled"].get<bool>();
            if (j.contains("timeout_ms")) cfg.timeout_ms = j["timeout_ms"].get<int>();
            if (j.contains("auto_map")) cfg.auto_map = j["auto_map"].get<bool>();
            
            auto set_result = netmgr.SetUpnpConfig(cfg);
            
            if (set_result.success) {
                json result;
                result["status"] = "success";
                result["message"] = "UPnP configuration updated successfully";
                resp.status_code = 200;
                resp.body = result.dump(2);
            } else {
                json err;
                err["error"] = "Failed to set UPnP configuration";
                err["details"] = set_result.error;
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

api::Response HandleUpnpDiscover(const api::RequestContext& ctx) {
    api::Response resp;
    auto& netmgr = networking::NetworkManager::Instance();
    
    if (ctx.method == "POST") {
        try {
            auto j = json::parse(ctx.body);
            int timeout_ms = j.value("timeout_ms", 2000);
            
            auto device_result = netmgr.DiscoverUpnp(timeout_ms);
            
            json result;
            if (device_result.success) {
                const auto& info = device_result.value;
                result["status"] = "success";
                result["device"] = {
                    {"gateway_ip", info.gateway_ip},
                    {"external_ip", info.external_ip},
                    {"device_name", info.device_name},
                    {"is_connected", info.is_connected}
                };
                resp.status_code = 200;
            } else {
                result["status"] = "failed";
                result["error"] = device_result.error;
                resp.status_code = 404;
            }
            
            resp.body = result.dump(2);
            
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

api::Response HandleUpnpPortMappings(const api::RequestContext& ctx) {
    api::Response resp;
    
    if (ctx.method == "GET") {
        // Get all port mappings from UPnP device
        auto& netmgr = networking::NetworkManager::Instance();
        auto mappings_result = netmgr.GetUpnpPortMappings();
        
        json result;
        if (mappings_result.success) {
            json mappings = json::array();
            for (const auto& mapping : mappings_result.value) {
                json m;
                m["external_port"] = mapping.external_port;
                m["internal_port"] = mapping.internal_port;
                m["protocol"] = mapping.protocol;
                m["internal_client"] = mapping.internal_client;
                m["description"] = mapping.description;
                m["lease_duration"] = mapping.lease_duration;
                mappings.push_back(m);
            }
            result["mappings"] = mappings;
            result["count"] = mappings_result.value.size();
            resp.status_code = 200;
        } else {
            result["mappings"] = json::array();
            result["error"] = mappings_result.error;
            resp.status_code = 500;
        }
        resp.body = result.dump(2);
        
    } else if (ctx.method == "POST") {
        // Add a new port mapping
        try {
            auto j = json::parse(ctx.body);
            auto& netmgr = networking::NetworkManager::Instance();
            
            networking::NetworkManager::UpnpPortMapping mapping;
            mapping.external_port = j.value("external_port", 0);
            mapping.internal_port = j.value("internal_port", 0);
            mapping.protocol = j.value("protocol", "TCP");
            mapping.internal_client = j.value("internal_client", "");
            mapping.description = j.value("description", "");
            mapping.lease_duration = j.value("lease_duration", 0);
            
            auto add_result = netmgr.AddUpnpPortMapping(mapping);
            
            json result;
            if (add_result.success) {
                result["status"] = "success";
                result["message"] = "Port mapping added successfully";
                resp.status_code = 200;
            } else {
                result["status"] = "failed";
                result["error"] = add_result.error;
                resp.status_code = 500;
            }
            
            resp.body = result.dump(2);
            
        } catch (const std::exception& e) {
            json err;
            err["error"] = "Invalid request format";
            err["details"] = e.what();
            resp.status_code = 400;
            resp.body = err.dump(2);
        }
        
    } else if (ctx.method == "DELETE") {
        // Delete a port mapping
        try {
            auto j = json::parse(ctx.body);
            auto& netmgr = networking::NetworkManager::Instance();
            
            if (!j.contains("external_port") || !j.contains("protocol")) {
                json err;
                err["error"] = "Missing required fields: external_port, protocol";
                resp.status_code = 400;
                resp.body = err.dump(2);
                return resp;
            }
            
            int external_port = j["external_port"];
            std::string protocol = j["protocol"];
            
            auto delete_result = netmgr.DeleteUpnpPortMapping(external_port, protocol);
            
            json result;
            if (delete_result.success) {
                result["status"] = "success";
                result["message"] = "Port mapping deleted successfully";
                resp.status_code = 200;
            } else {
                result["status"] = "failed";
                result["error"] = delete_result.error;
                resp.status_code = 500;
            }
            
            resp.body = result.dump(2);
            
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

api::Response HandleUpnpCameraPorts(const api::RequestContext& ctx) {
    api::Response resp;
    auto& netmgr = networking::NetworkManager::Instance();
    
    if (ctx.method == "POST") {
        // Add standard camera port mappings
        try {
            auto j = json::parse(ctx.body);
            
            int http_port = j.value("http_port", 0);
            int https_port = j.value("https_port", 0);
            int rtsp_port = j.value("rtsp_port", 0);
            int onvif_port = j.value("onvif_port", 0);
            
            int mappings_added = 0;
            std::vector<std::string> errors;
            
            // Add HTTP port mapping if specified
            if (http_port > 0) {
                networking::NetworkManager::UpnpPortMapping mapping;
                mapping.external_port = http_port;
                mapping.internal_port = http_port;
                mapping.protocol = "TCP";
                mapping.description = "Camera HTTP";
                mapping.lease_duration = 0;
                
                auto result = netmgr.AddUpnpPortMapping(mapping);
                if (result.success) {
                    mappings_added++;
                } else {
                    errors.push_back("HTTP: " + result.error);
                }
            }
            
            // Add HTTPS port mapping if specified
            if (https_port > 0) {
                networking::NetworkManager::UpnpPortMapping mapping;
                mapping.external_port = https_port;
                mapping.internal_port = https_port;
                mapping.protocol = "TCP";
                mapping.description = "Camera HTTPS";
                mapping.lease_duration = 0;
                
                auto result = netmgr.AddUpnpPortMapping(mapping);
                if (result.success) {
                    mappings_added++;
                } else {
                    errors.push_back("HTTPS: " + result.error);
                }
            }
            
            // Add RTSP port mapping if specified
            if (rtsp_port > 0) {
                networking::NetworkManager::UpnpPortMapping mapping;
                mapping.external_port = rtsp_port;
                mapping.internal_port = rtsp_port;
                mapping.protocol = "TCP";
                mapping.description = "Camera RTSP";
                mapping.lease_duration = 0;
                
                auto result = netmgr.AddUpnpPortMapping(mapping);
                if (result.success) {
                    mappings_added++;
                } else {
                    errors.push_back("RTSP: " + result.error);
                }
            }
            
            // Add ONVIF port mapping if specified
            if (onvif_port > 0) {
                networking::NetworkManager::UpnpPortMapping mapping;
                mapping.external_port = onvif_port;
                mapping.internal_port = onvif_port;
                mapping.protocol = "TCP";
                mapping.description = "Camera ONVIF";
                mapping.lease_duration = 0;
                
                auto result = netmgr.AddUpnpPortMapping(mapping);
                if (result.success) {
                    mappings_added++;
                } else {
                    errors.push_back("ONVIF: " + result.error);
                }
            }
            
            json result;
            result["status"] = "success";
            result["message"] = "Camera port mappings processed";
            result["mappings_added"] = mappings_added;
            
            if (!errors.empty()) {
                result["errors"] = errors;
            }
            
            resp.status_code = mappings_added > 0 ? 200 : 500;
            resp.body = result.dump(2);
            
        } catch (const std::exception& e) {
            json err;
            err["error"] = "Invalid request format";
            err["details"] = e.what();
            resp.status_code = 400;
            resp.body = err.dump(2);
        }
        
    } else if (ctx.method == "DELETE") {
        // Remove camera port mappings - would need to know which ports to delete
        // For now, return a note that individual DELETE operations should be used
        json result;
        result["status"] = "note";
        result["message"] = "Use DELETE on /api/v1/upnp/port_mappings with specific port numbers";
        result["note"] = "Bulk camera port removal not implemented - use individual port deletion";
        resp.status_code = 200;
        resp.body = result.dump(2);
        
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
