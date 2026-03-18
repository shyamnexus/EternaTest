/**
 * @file smart_handler.cpp
 * @brief Smart detection (pedestrian/vehicle/animal) API handlers
 */

#include "smart_handler.h"
#include "analytics_common.h"
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
// Smart Detection Handler
// ============================================================================
api::Response HandleSmartDetection(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        auto& analytics = ai::AnalyticsEngine::Instance();
        
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("smart_detection.enabled", false);
            j["confidence_threshold"] = round_json(config::Get<float>("smart_detection.confidence_threshold", 0.5f));
            j["pedestrian"] = {
                {"enabled", config::Get<bool>("smart_detection.pedestrian.enabled", true)},
                {"sensitivity", config::Get<int>("smart_detection.pedestrian.sensitivity", 50)},
                {"min_confidence", round_json(config::Get<float>("smart_detection.pedestrian.min_confidence", 0.5f))}
            };
            j["vehicle"] = {
                {"enabled", config::Get<bool>("smart_detection.vehicle.enabled", true)},
                {"sensitivity", config::Get<int>("smart_detection.vehicle.sensitivity", 50)},
                {"min_confidence", round_json(config::Get<float>("smart_detection.vehicle.min_confidence", 0.5f))}
            };
            j["animal"] = {
                {"enabled", config::Get<bool>("smart_detection.animal.enabled", false)},
                {"sensitivity", config::Get<int>("smart_detection.animal.sensitivity", 50)}
            };
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            if (body.contains("enabled")) 
                config::Set<bool>("smart_detection.enabled", body["enabled"].get<bool>());
            if (body.contains("confidence_threshold")) 
                config::Set<float>("smart_detection.confidence_threshold", body["confidence_threshold"].get<float>());
            
            if (body.contains("pedestrian")) {
                auto& p = body["pedestrian"];
                if (p.contains("enabled")) {
                    bool en = p["enabled"].get<bool>();
                    config::Set<bool>("smart_detection.pedestrian.enabled", en);
                    analytics.EnablePersonDetection(en);
                }
                if (p.contains("sensitivity")) 
                    config::Set<int>("smart_detection.pedestrian.sensitivity", p["sensitivity"].get<int>());
                if (p.contains("min_confidence")) 
                    config::Set<float>("smart_detection.pedestrian.min_confidence", p["min_confidence"].get<float>());
            }
            if (body.contains("vehicle")) {
                auto& v = body["vehicle"];
                if (v.contains("enabled")) {
                    bool en = v["enabled"].get<bool>();
                    config::Set<bool>("smart_detection.vehicle.enabled", en);
                    analytics.EnableVehicleDetection(en);
                }
                if (v.contains("sensitivity")) 
                    config::Set<int>("smart_detection.vehicle.sensitivity", v["sensitivity"].get<int>());
                if (v.contains("min_confidence")) 
                    config::Set<float>("smart_detection.vehicle.min_confidence", v["min_confidence"].get<float>());
            }
            if (body.contains("animal")) {
                auto& a = body["animal"];
                if (a.contains("enabled")) 
                    config::Set<bool>("smart_detection.animal.enabled", a["enabled"].get<bool>());
                if (a.contains("sensitivity")) 
                    config::Set<int>("smart_detection.animal.sensitivity", a["sensitivity"].get<int>());
            }
            
            config::Save();
            
            // Handle runtime enable/disable of AI3 object detection
            bool want_enabled = config::Get<bool>("smart_detection.enabled", false);
            auto acfg = analytics.GetConfig();
            acfg.object_detection.enabled = want_enabled;
            analytics.SetConfig(acfg);
            
            // If enabling and AI3 was not initialized at boot, trigger init
            if (want_enabled && !analytics.IsAi3Initialized()) {
                if (analytics.InitAiModels()) {
                    spdlog::info("AI3 neural network engine initialized via API");
                } else {
                    spdlog::warn("AI3 neural network engine runtime initialization failed");
                }
            }
            
            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in smart detection handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// Smart Zones Handler
// ============================================================================
api::Response HandleSmartZones(const api::RequestContext& ctx) {
    return HandleFeatureZones(ctx, features::SMART);
}

// ============================================================================
// Smart Schedule Handler
// ============================================================================
api::Response HandleSmartSchedule(const api::RequestContext& ctx) {
    return HandleFeatureSchedule(ctx, features::SMART);
}

// ============================================================================
// Smart Actions Handler
// ============================================================================
api::Response HandleSmartActions(const api::RequestContext& ctx) {
    return HandleFeatureActions(ctx, features::SMART);
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
