#include "snmp_handler.h"
#include <ipcam/network_manager.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

api::Response HandleSnmp(const api::RequestContext& ctx) {
    api::Response resp;
    auto& netmgr = networking::NetworkManager::Instance();
    
    if (ctx.method == "GET") {
        // Get SNMP configuration and status
        json result;
        
        auto config_result = netmgr.GetSnmpConfig();
        if (config_result.success) {
            const auto& cfg = config_result.value;
            result["config"] = {
                {"enabled", cfg.enabled},
                {"port", cfg.port},
                {"version", cfg.version},
                {"listen_address", cfg.listen_address},
                {"sys_location", cfg.sys_location},
                {"sys_contact", cfg.sys_contact},
                {"sys_name", cfg.sys_name}
                // Don't expose sensitive fields like community strings or passwords
            };
        } else {
            result["config"]["error"] = config_result.error;
        }
        
        auto status_result = netmgr.IsSnmpRunning();
        if (status_result.success) {
            result["status"] = {
                {"running", status_result.value},
                {"daemon", "snmpd"}
            };
        } else {
            result["status"]["error"] = status_result.error;
        }
        
        resp.status_code = 200;
        resp.body = result.dump(2);
        
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        // Update SNMP configuration
        try {
            auto j = json::parse(ctx.body);
            
            // Build SNMP config from JSON
            networking::NetworkManager::SnmpConfig cfg;
            
            if (j.contains("enabled")) cfg.enabled = j["enabled"].get<bool>();
            if (j.contains("port")) cfg.port = j["port"].get<int>();
            if (j.contains("version")) cfg.version = j["version"].get<int>();
            if (j.contains("listen_address")) cfg.listen_address = j["listen_address"].get<std::string>();
            if (j.contains("community_ro")) cfg.community_ro = j["community_ro"].get<std::string>();
            if (j.contains("community_rw")) cfg.community_rw = j["community_rw"].get<std::string>();
            if (j.contains("sys_location")) cfg.sys_location = j["sys_location"].get<std::string>();
            if (j.contains("sys_contact")) cfg.sys_contact = j["sys_contact"].get<std::string>();
            if (j.contains("sys_name")) cfg.sys_name = j["sys_name"].get<std::string>();
            
            // SNMPv3 fields
            if (j.contains("v3_user")) cfg.v3_user = j["v3_user"].get<std::string>();
            if (j.contains("v3_auth_password")) cfg.v3_auth_password = j["v3_auth_password"].get<std::string>();
            if (j.contains("v3_priv_password")) cfg.v3_priv_password = j["v3_priv_password"].get<std::string>();
            if (j.contains("v3_auth_protocol")) cfg.v3_auth_protocol = j["v3_auth_protocol"].get<std::string>();
            if (j.contains("v3_priv_protocol")) cfg.v3_priv_protocol = j["v3_priv_protocol"].get<std::string>();
            
            auto set_result = netmgr.SetSnmpConfig(cfg);
            
            if (set_result.success) {
                json result;
                result["status"] = "success";
                result["message"] = "SNMP configuration updated successfully";
                
                // Check if daemon is running
                auto status = netmgr.IsSnmpRunning();
                if (status.success) {
                    result["daemon_running"] = status.value;
                }
                
                resp.status_code = 200;
                resp.body = result.dump(2);
            } else {
                json err;
                err["error"] = "Failed to set SNMP configuration";
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

api::Response HandleSnmpControl(const api::RequestContext& ctx) {
    api::Response resp;
    auto& netmgr = networking::NetworkManager::Instance();
    
    if (ctx.method == "POST") {
        try {
            auto j = json::parse(ctx.body);
            
            if (!j.contains("action")) {
                json err;
                err["error"] = "Missing 'action' field";
                err["valid_actions"] = {"start", "stop", "restart"};
                resp.status_code = 400;
                resp.body = err.dump(2);
                return resp;
            }
            
            std::string action = j["action"].get<std::string>();
            ipcam::Result<void> result;
            
            if (action == "start") {
                result = netmgr.StartSnmp();
            } else if (action == "stop") {
                result = netmgr.StopSnmp();
            } else if (action == "restart") {
                result = netmgr.RestartSnmp();
            } else {
                json err;
                err["error"] = "Invalid action";
                err["valid_actions"] = {"start", "stop", "restart"};
                resp.status_code = 400;
                resp.body = err.dump(2);
                return resp;
            }
            
            json response;
            if (result.success) {
                response["status"] = "success";
                response["message"] = "SNMP daemon " + action + " successful";
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

api::Response HandleSnmpTest(const api::RequestContext& ctx) {
    api::Response resp;
    auto& netmgr = networking::NetworkManager::Instance();
    
    if (ctx.method == "POST") {
        // Test SNMP by checking if daemon is running
        auto result = netmgr.IsSnmpRunning();
        
        json response;
        if (result.success && result.value) {
            response["status"] = "success";
            response["message"] = "SNMP daemon is running";
            resp.status_code = 200;
        } else if (result.success && !result.value) {
            response["status"] = "failed";
            response["error"] = "SNMP daemon is not running";
            resp.status_code = 500;
        } else {
            response["status"] = "failed";
            response["error"] = result.error;
            resp.status_code = 500;
        }
        
        resp.body = response.dump(2);
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
