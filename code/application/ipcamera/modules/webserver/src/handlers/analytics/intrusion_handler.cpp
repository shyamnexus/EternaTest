/**
 * @file intrusion_handler.cpp
 * @brief Zone intrusion detection API handlers
 */

#include "intrusion_handler.h"
#include <ipcam/analytics.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// ============================================================================
// Zone Intrusion Handler
// ============================================================================
api::Response HandleZoneIntrusion(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        auto& analytics = ai::AnalyticsEngine::Instance();
        
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("zone_intrusion.enabled", false);
            j["dwell_time_ms"] = config::Get<int>("zone_intrusion.dwell_time_ms", 1000);
            j["object_filter"] = {
                {"pedestrian", config::Get<bool>("zone_intrusion.object_filter.pedestrian", true)},
                {"vehicle", config::Get<bool>("zone_intrusion.object_filter.vehicle", true)},
                {"bicycle", config::Get<bool>("zone_intrusion.object_filter.bicycle", false)}
            };
            j["alarm_mode"] = config::Get<std::string>("zone_intrusion.alarm_mode", "enter"); // enter, exit, both, loiter
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            // Update in-memory config through analytics engine
            if (body.contains("enabled")) {
                bool en = body["enabled"].get<bool>();
                config::Set<bool>("zone_intrusion.enabled", en);
                // Update analytics engine's in-memory config
                auto cfg = analytics.GetConfig();
                cfg.zone_intrusion.enabled = en;
                analytics.SetConfig(cfg);
            }
            if (body.contains("dwell_time_ms")) 
                config::Set<int>("zone_intrusion.dwell_time_ms", body["dwell_time_ms"].get<int>());
            if (body.contains("alarm_mode")) 
                config::Set<std::string>("zone_intrusion.alarm_mode", body["alarm_mode"].get<std::string>());
            
            if (body.contains("object_filter")) {
                auto& f = body["object_filter"];
                if (f.contains("pedestrian")) 
                    config::Set<bool>("zone_intrusion.object_filter.pedestrian", f["pedestrian"].get<bool>());
                if (f.contains("vehicle")) 
                    config::Set<bool>("zone_intrusion.object_filter.vehicle", f["vehicle"].get<bool>());
                if (f.contains("bicycle")) 
                    config::Set<bool>("zone_intrusion.object_filter.bicycle", f["bicycle"].get<bool>());
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
        spdlog::error("Error in zone intrusion handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// Intrusion Zones Handler
// ============================================================================
api::Response HandleIntrusionZones(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        const std::string zones_key = "zone_intrusion.zones";
        
        if (ctx.method == "GET") {
            auto zones = config::GetOptional<json>(zones_key);
            if (zones.has_value()) {
                resp.body = zones.value().dump(2);
            } else {
                json j = json::array();
                resp.body = j.dump(2);
            }
            resp.status_code = 200;
        }
        else if (ctx.method == "POST") {
            // Add new zone
            auto body = json::parse(ctx.body);
            auto zones = config::GetOptional<json>(zones_key).value_or(json::array());
            
            // Generate new ID
            uint32_t new_id = 1;
            for (const auto& zone : zones) {
                if (zone.contains("id") && zone["id"].get<uint32_t>() >= new_id) {
                    new_id = zone["id"].get<uint32_t>() + 1;
                }
            }
            
            json new_zone;
            new_zone["id"] = new_id;
            new_zone["name"] = body.value("name", "Zone " + std::to_string(new_id));
            new_zone["enabled"] = body.value("enabled", true);
            new_zone["color"] = body.value("color", "#FF0000");
            new_zone["dwell_time_ms"] = body.value("dwell_time_ms", 1000);
            
            // Points for polygon zone
            if (body.contains("points")) {
                new_zone["points"] = body["points"];
            } else {
                // Default rectangle as 4 points (normalized 0-1)
                new_zone["points"] = json::array({
                    {{"x", 0.25}, {"y", 0.25}},
                    {{"x", 0.75}, {"y", 0.25}},
                    {{"x", 0.75}, {"y", 0.75}},
                    {{"x", 0.25}, {"y", 0.75}}
                });
            }
            
            zones.push_back(new_zone);
            config::Set<json>(zones_key, zones);
            config::Save();
            
            resp.status_code = 201;
            resp.body = new_zone.dump(2);
        }
        else if (ctx.method == "PUT") {
            // Update zones
            auto body = json::parse(ctx.body);
            config::Set<json>(zones_key, body);
            config::Save();
            
            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else if (ctx.method == "DELETE") {
            // Delete zone by ID
            auto id_it = ctx.query_params.find("id");
            if (id_it != ctx.query_params.end()) {
                uint32_t del_id = std::stoul(id_it->second);
                auto zones = config::GetOptional<json>(zones_key).value_or(json::array());
                
                json new_zones = json::array();
                for (const auto& zone : zones) {
                    if (!zone.contains("id") || zone["id"].get<uint32_t>() != del_id) {
                        new_zones.push_back(zone);
                    }
                }
                
                config::Set<json>(zones_key, new_zones);
                config::Save();
                
                resp.status_code = 200;
                resp.body = R"({"success": true})";
            } else {
                resp.status_code = 400;
                resp.body = R"({"error": "Missing id parameter"})";
            }
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in intrusion zones handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// Intrusion Schedule Handler
// ============================================================================
api::Response HandleIntrusionSchedule(const api::RequestContext& ctx) {
    return HandleFeatureSchedule(ctx, features::INTRUSION);
}

// ============================================================================
// Intrusion Actions Handler
// ============================================================================
api::Response HandleIntrusionActions(const api::RequestContext& ctx) {
    return HandleFeatureActions(ctx, features::INTRUSION);
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
