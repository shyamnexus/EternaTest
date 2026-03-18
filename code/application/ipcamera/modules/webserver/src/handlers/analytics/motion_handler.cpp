/**
 * @file motion_handler.cpp
 * @brief Motion detection API handler implementations
 */

#include "motion_handler.h"
#include "analytics_common.h"
#include <ipcam/analytics.h>
#include <ipcam/motion_detection.h>
#include <ipcam/config.h>
#include <spdlog/spdlog.h>

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// ============================================================================
// GET/PUT /api/v1/analytics/motion - Motion detection config
// ============================================================================
api::Response HandleMotionDetection(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        auto& analytics = ai::AnalyticsEngine::Instance();
        
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("motion_detection.enabled", false);
            j["sensitivity"] = config::Get<int>("motion_detection.sensitivity", 50);
            j["threshold"] = config::Get<int>("motion_detection.threshold", 30);
            j["min_area"] = config::Get<int>("motion_detection.min_area", 500);
            j["cooldown_ms"] = config::Get<int>("motion_detection.cooldown_ms", 2000);
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            // Update enable state through analytics engine (updates in-memory + persists)
            if (body.contains("enabled")) {
                analytics.EnableMotionDetection(body["enabled"].get<bool>());
            }
            // Update other settings via config (analytics engine reads these on next frame)
            if (body.contains("sensitivity")) {
                config::Set<int>("motion_detection.sensitivity", body["sensitivity"].get<int>());
            }
            if (body.contains("threshold")) {
                config::Set<int>("motion_detection.threshold", body["threshold"].get<int>());
            }
            if (body.contains("min_area")) {
                config::Set<int>("motion_detection.min_area", body["min_area"].get<int>());
            }
            if (body.contains("cooldown_ms")) {
                config::Set<int>("motion_detection.cooldown_ms", body["cooldown_ms"].get<int>());
            }
            
            config::Save();
            
            // Handle runtime enable/disable of motion detection engine
            bool want_enabled = config::Get<bool>("motion_detection.enabled", false);
            auto& md = ai::MotionDetectionEngine::Instance();
            
            if (want_enabled && !md.IsInitialized()) {
                // Engine was disabled at boot - need to init and start it now
                ai::MdConfig md_cfg;
                md_cfg.width = 640;
                md_cfg.height = 360;
                md_cfg.mode = ai::MotionMode::kSubAlarm;
                md_cfg.sensitivity = config::Get<int>("motion_detection.sensitivity", 50);
                md_cfg.global_threshold = static_cast<uint8_t>(config::Get<int>("motion_detection.threshold", 30));
                md_cfg.enabled = true;
                md_cfg.cooldown_ms = config::Get<int>("motion_detection.cooldown_ms", 2000);
                if (md.Init(md_cfg)) {
                    md.Start();
                    spdlog::info("Motion Detection Engine initialized and started via API");
                } else {
                    spdlog::warn("Motion Detection Engine runtime initialization failed");
                }
            } else if (want_enabled && md.IsInitialized() && !md.IsRunning()) {
                md.Start();
                spdlog::info("Motion Detection Engine started via API");
            } else if (!want_enabled && md.IsRunning()) {
                md.Stop();
                spdlog::info("Motion Detection Engine stopped via API");
            }
            
            // Return updated config
            json j;
            j["enabled"] = config::Get<bool>("motion_detection.enabled", false);
            j["sensitivity"] = config::Get<int>("motion_detection.sensitivity", 50);
            j["threshold"] = config::Get<int>("motion_detection.threshold", 30);
            j["min_area"] = config::Get<int>("motion_detection.min_area", 500);
            j["cooldown_ms"] = config::Get<int>("motion_detection.cooldown_ms", 2000);
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in motion detection handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// GET/POST/PUT/DELETE /api/v1/analytics/motion/zones - Motion zones
// ============================================================================
api::Response HandleMotionZones(const api::RequestContext& ctx) {
    // Use common zone handler with feature name
    return HandleFeatureZones(ctx, features::MOTION);
}

// ============================================================================
// GET/PUT /api/v1/analytics/motion/schedule - Motion schedule
// ============================================================================
api::Response HandleMotionSchedule(const api::RequestContext& ctx) {
    // Use common schedule handler with feature name
    return HandleFeatureSchedule(ctx, features::MOTION);
}

// ============================================================================
// GET/PUT /api/v1/analytics/motion/actions - Motion event actions
// ============================================================================
api::Response HandleMotionActions(const api::RequestContext& ctx) {
    // Use common actions handler with feature name
    return HandleFeatureActions(ctx, features::MOTION);
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
