/**
 * @file analytics_router.cpp
 * @brief Main analytics router implementation
 */

#include "analytics_router.h"
#include "analytics_common.h"
#include <ipcam/analytics.h>
#include <ipcam/tamper_detection.h>
#include <ipcam/config.h>
#include <spdlog/spdlog.h>

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// ============================================================================
// GET/PUT /api/v1/analytics - Full config overview
// ============================================================================
api::Response HandleAnalytics(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            json j;
            
            // Overview of all enabled features
            j["version"] = "2.0.0";
            
            // Motion detection summary
            j["motion_detection"]["enabled"] = config::Get<bool>("motion_detection.enabled", false);
            j["motion_detection"]["sensitivity"] = config::Get<int>("motion_detection.sensitivity", 50);
            
            // Video tampering summary
            j["video_tampering"]["enabled"] = config::Get<bool>("video_tampering.enabled", false);
            j["video_tampering"]["sensitivity"] = config::Get<int>("video_tampering.sensitivity", 50);
            
            // Smart detection summary
            j["smart_detection"]["enabled"] = config::Get<bool>("smart_detection.enabled", false);
            j["smart_detection"]["pedestrian_enabled"] = config::Get<bool>("smart_detection.pedestrian.enabled", false);
            j["smart_detection"]["vehicle_enabled"] = config::Get<bool>("smart_detection.vehicle.enabled", false);
            
            // Global settings
            j["global_settings"]["inference_fps"] = config::Get<int>("global_settings.inference_fps", 10);
            j["global_settings"]["overlay_enabled"] = config::Get<bool>("global_settings.overlay_enabled", false);
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            // Update global settings if provided
            if (body.contains("global_settings")) {
                auto& gs = body["global_settings"];
                if (gs.contains("inference_fps")) {
                    config::Set<int>("global_settings.inference_fps", gs["inference_fps"].get<int>());
                }
                if (gs.contains("overlay_enabled")) {
                    config::Set<bool>("global_settings.overlay_enabled", gs["overlay_enabled"].get<bool>());
                }
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
        spdlog::error("Error in analytics handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// GET/POST /api/v1/analytics/status - Running status and aggregate stats
// ============================================================================
api::Response HandleAnalyticsStatus(const api::RequestContext& ctx) {
    api::Response resp;
    auto& engine = ai::AnalyticsEngine::Instance();

    try {
        if (ctx.method == "GET") {
            auto stats = engine.GetStats();
            
            json j;
            j["running"] = engine.IsRunning();
            j["frames_processed"] = stats.frames_processed;
            j["total_detections"] = stats.total_detections;
            j["motion_events"] = stats.motion_events;
            j["line_cross_events"] = stats.line_cross_events;
            j["loitering_events"] = stats.loitering_events;
            j["avg_process_time_ms"] = stats.avg_process_time_ms;
            j["current_fps"] = stats.current_fps;
            
            // Add Tamper Detection stats
            try {
                auto& tamper = ai::TamperDetectionEngine::Instance();
                auto tamper_stats = tamper.GetStats();
                j["tamper_detection"]["running"] = tamper.IsRunning();
                j["tamper_detection"]["frames_processed"] = tamper_stats.frames_processed;
                j["tamper_detection"]["defocus_events"] = tamper_stats.defocus_events;
                j["tamper_detection"]["masking_events"] = tamper_stats.masking_events;
                j["tamper_detection"]["scene_change_events"] = tamper_stats.scene_change_events;
                j["tamper_detection"]["too_dark_events"] = tamper_stats.too_dark_events;
                j["tamper_detection"]["too_bright_events"] = tamper_stats.too_bright_events;
            } catch (...) {
                j["tamper_detection"]["available"] = false;
            }
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            
            if (body.contains("action")) {
                std::string action = body["action"].get<std::string>();
                
                if (action == "start") {
                    if (engine.Start()) {
                        resp.status_code = 200;
                        resp.body = R"({"success": true, "message": "Analytics started"})";
                    } else {
                        resp.status_code = 500;
                        resp.body = R"({"error": "Failed to start analytics"})";
                    }
                }
                else if (action == "stop") {
                    engine.Stop();
                    resp.status_code = 200;
                    resp.body = R"({"success": true, "message": "Analytics stopped"})";
                }
                else if (action == "reset_stats") {
                    engine.ResetStats();
                    resp.status_code = 200;
                    resp.body = R"({"success": true, "message": "Stats reset"})";
                }
                else if (action == "start_tamper") {
                    try {
                        auto& tamper = ai::TamperDetectionEngine::Instance();
                        if (tamper.Start()) {
                            resp.status_code = 200;
                            resp.body = R"({"success": true, "message": "Tamper detection started"})";
                        } else {
                            resp.status_code = 500;
                            resp.body = R"({"error": "Failed to start tamper detection"})";
                        }
                    } catch (const std::exception& e) {
                        resp.status_code = 500;
                        resp.body = std::string(R"({"error": "Tamper detection error: )") + e.what() + "\"}";
                    }
                }
                else if (action == "stop_tamper") {
                    try {
                        auto& tamper = ai::TamperDetectionEngine::Instance();
                        tamper.Stop();
                        resp.status_code = 200;
                        resp.body = R"({"success": true, "message": "Tamper detection stopped"})";
                    } catch (const std::exception& e) {
                        resp.status_code = 500;
                        resp.body = std::string(R"({"error": "Tamper detection error: )") + e.what() + "\"}";
                    }
                }
                else {
                    resp.status_code = 400;
                    resp.body = R"({"error": "Invalid action"})";
                }
            }
            else {
                resp.status_code = 400;
                resp.body = R"({"error": "Missing action"})";
            }
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in analytics status handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

// ============================================================================
// GET /api/v1/analytics/capabilities - Supported features and limits
// ============================================================================
api::Response HandleAnalyticsCapabilities(const api::RequestContext& ctx) {
    api::Response resp;
    auto& analytics = ai::AnalyticsEngine::Instance();

    try {
        if (ctx.method == "GET") {
            json j;
            
            // Version
            j["api_version"] = "2.0.0";
            
            // ----------------------------------------------------------------
            // Implemented Features
            // ----------------------------------------------------------------
            j["features"]["motion_detection"] = {
                {"available", true},
                {"library", "software"},
                {"max_zones", 8}
            };
            
            j["features"]["video_tampering"] = {
                {"available", true},
                {"library", "software"},
                {"detectors", json::array({"defocus", "masking", "scene_change", "too_dark", "too_bright"})}
            };
            
            // ----------------------------------------------------------------
            // Planned Features (structure for future)
            // ----------------------------------------------------------------
            j["features"]["smart_detection"] = {
                {"available", false},
                {"planned", true},
                {"classes", json::array({"pedestrian", "vehicle", "animal"})}
            };
            
            j["features"]["face_detection"] = {
                {"available", false},
                {"planned", true}
            };
            
            j["features"]["license_plate"] = {
                {"available", false},
                {"planned", true}
            };
            
            j["features"]["line_crossing"] = {
                {"available", false},
                {"planned", true},
                {"max_lines", 4}
            };
            
            j["features"]["zone_intrusion"] = {
                {"available", false},
                {"planned", true},
                {"modes", json::array({"enter", "exit", "loiter"})}
            };
            
            j["features"]["object_detection"] = {
                {"available", false},
                {"planned", true}
            };
            
            j["features"]["cross_counting"] = {
                {"available", false},
                {"planned", true}
            };
            
            j["features"]["heatmap"] = {
                {"available", false},
                {"planned", true}
            };
            
            j["features"]["queue_length"] = {
                {"available", false},
                {"planned", true}
            };
            
            j["features"]["crowd_density"] = {
                {"available", false},
                {"planned", true}
            };
            
            j["features"]["rare_sound"] = {
                {"available", false},
                {"planned", true}
            };
            
            // ----------------------------------------------------------------
            // Common Capabilities
            // ----------------------------------------------------------------
            j["common"]["schedule"] = {
                {"available", true},
                {"max_rules", 10}
            };
            
            j["common"]["actions"] = {
                {"available", json::array({"recording", "snapshot", "https_notify", "mqtt", "email", "ftp_upload"})}
            };
            
            j["common"]["notifications"] = {
                {"mqtt", true},
                {"email", true},
                {"ftp", true},
                {"https_webhook", true}
            };
            
            // ----------------------------------------------------------------
            // Planned Management
            // ----------------------------------------------------------------
            j["management"]["list_management"] = {
                {"available", false},
                {"planned", true},
                {"types", json::array({"face_allowlist", "face_blocklist", "plate_allowlist", "plate_blocklist"})}
            };
            
            j["management"]["statistics"] = {
                {"available", false},
                {"planned", true},
                {"types", json::array({"face_count", "pedestrian_count", "vehicle_count", "line_cross_count", "heatmap_data"})}
            };
            
            // Runtime info
            j["runtime"]["smart_bbox_overlay"] = analytics.HasHdalPaths();
            j["runtime"]["input_resolutions"] = json::array({
                {{"width", 640}, {"height", 360}},
                {{"width", 320}, {"height", 180}}
            });
            j["runtime"]["process_fps_range"] = {{"min", 1}, {"max", 30}};
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in analytics capabilities handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

// ============================================================================
// Legacy Toggles (backward compatibility)
// ============================================================================
api::Response HandleAnalyticsToggles(const api::RequestContext& ctx) {
    api::Response resp;
    auto& engine = ai::AnalyticsEngine::Instance();

    try {
        if (ctx.method == "GET") {
            auto cfg = engine.GetConfig();
            json j;
            j["object_detection"] = cfg.object_detection.enabled;
            j["person_detection"] = cfg.object_detection.person.enabled;
            j["vehicle_detection"] = cfg.object_detection.vehicle.enabled;
            j["animal_detection"] = cfg.object_detection.animal.enabled;
            j["face_detection"] = cfg.face_detection.enabled;
            j["motion_detection"] = cfg.motion_detection.enabled;
            j["line_crossing"] = cfg.line_crossing.enabled;
            j["zone_intrusion"] = cfg.zone_intrusion.enabled;
            j["license_plate"] = cfg.license_plate.enabled;
            j["object_tracking"] = cfg.object_detection.tracking.enabled;
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto cfg = engine.GetConfig();
            
            if (body.contains("object_detection")) cfg.object_detection.enabled = body["object_detection"].get<bool>();
            if (body.contains("person_detection")) cfg.object_detection.person.enabled = body["person_detection"].get<bool>();
            if (body.contains("vehicle_detection")) cfg.object_detection.vehicle.enabled = body["vehicle_detection"].get<bool>();
            if (body.contains("animal_detection")) cfg.object_detection.animal.enabled = body["animal_detection"].get<bool>();
            if (body.contains("face_detection")) cfg.face_detection.enabled = body["face_detection"].get<bool>();
            if (body.contains("motion_detection")) cfg.motion_detection.enabled = body["motion_detection"].get<bool>();
            if (body.contains("line_crossing")) cfg.line_crossing.enabled = body["line_crossing"].get<bool>();
            if (body.contains("zone_intrusion")) cfg.zone_intrusion.enabled = body["zone_intrusion"].get<bool>();
            if (body.contains("license_plate")) cfg.license_plate.enabled = body["license_plate"].get<bool>();
            if (body.contains("object_tracking")) cfg.object_detection.tracking.enabled = body["object_tracking"].get<bool>();
            
            if (engine.SetConfig(cfg)) {
                json j;
                j["object_detection"] = cfg.object_detection.enabled;
                j["person_detection"] = cfg.object_detection.person.enabled;
                j["vehicle_detection"] = cfg.object_detection.vehicle.enabled;
                j["animal_detection"] = cfg.object_detection.animal.enabled;
                j["face_detection"] = cfg.face_detection.enabled;
                j["motion_detection"] = cfg.motion_detection.enabled;
                j["line_crossing"] = cfg.line_crossing.enabled;
                j["zone_intrusion"] = cfg.zone_intrusion.enabled;
                j["license_plate"] = cfg.license_plate.enabled;
                j["object_tracking"] = cfg.object_detection.tracking.enabled;
                
                resp.status_code = 200;
                resp.body = j.dump(2);
            } else {
                resp.status_code = 500;
                resp.body = R"({"error": "Failed to update toggles"})";
            }
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in analytics toggles handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
