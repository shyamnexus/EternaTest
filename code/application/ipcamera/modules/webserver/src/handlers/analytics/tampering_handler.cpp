/**
 * @file tampering_handler.cpp
 * @brief Video tampering detection API handler implementations
 */

#include "tampering_handler.h"
#include "analytics_common.h"
#include <ipcam/tamper_detection.h>
#include <ipcam/analytics.h>
#include <ipcam/config.h>
#include <spdlog/spdlog.h>

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// ============================================================================
// GET/PUT /api/v1/analytics/tampering - Video tampering config
// ============================================================================
api::Response HandleVideoTampering(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("video_tampering.enabled", false);
            j["sensitivity"] = config::Get<int>("video_tampering.sensitivity", 50);
            j["defocus_detection"] = {
                {"enabled", config::Get<bool>("video_tampering.defocus_detection.enabled", true)},
                {"sensitivity", config::Get<int>("video_tampering.defocus_detection.sensitivity", 50)}
            };
            j["masking_detection"] = {
                {"enabled", config::Get<bool>("video_tampering.masking_detection.enabled", true)},
                {"sensitivity", config::Get<int>("video_tampering.masking_detection.sensitivity", 50)}
            };
            j["scene_change_detection"] = {
                {"enabled", config::Get<bool>("video_tampering.scene_change_detection.enabled", true)},
                {"sensitivity", config::Get<int>("video_tampering.scene_change_detection.sensitivity", 50)}
            };
            j["too_dark_detection"] = {
                {"enabled", config::Get<bool>("video_tampering.too_dark_detection.enabled", true)},
                {"threshold", config::Get<int>("video_tampering.too_dark_detection.threshold", 15)}
            };
            j["too_bright_detection"] = {
                {"enabled", config::Get<bool>("video_tampering.too_bright_detection.enabled", true)},
                {"threshold", config::Get<int>("video_tampering.too_bright_detection.threshold", 240)}
            };
            j["dwell_time_sec"] = config::Get<int>("video_tampering.dwell_time_sec", 3);
            j["auto_recovery"] = config::Get<bool>("video_tampering.auto_recovery", true);
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            if (body.contains("enabled")) config::Set<bool>("video_tampering.enabled", body["enabled"].get<bool>());
            if (body.contains("sensitivity")) config::Set<int>("video_tampering.sensitivity", body["sensitivity"].get<int>());
            if (body.contains("dwell_time_sec")) config::Set<int>("video_tampering.dwell_time_sec", body["dwell_time_sec"].get<int>());
            if (body.contains("auto_recovery")) config::Set<bool>("video_tampering.auto_recovery", body["auto_recovery"].get<bool>());
            
            if (body.contains("defocus_detection")) {
                auto& d = body["defocus_detection"];
                if (d.contains("enabled")) config::Set<bool>("video_tampering.defocus_detection.enabled", d["enabled"].get<bool>());
                if (d.contains("sensitivity")) config::Set<int>("video_tampering.defocus_detection.sensitivity", d["sensitivity"].get<int>());
            }
            if (body.contains("masking_detection")) {
                auto& m = body["masking_detection"];
                if (m.contains("enabled")) config::Set<bool>("video_tampering.masking_detection.enabled", m["enabled"].get<bool>());
                if (m.contains("sensitivity")) config::Set<int>("video_tampering.masking_detection.sensitivity", m["sensitivity"].get<int>());
            }
            if (body.contains("scene_change_detection")) {
                auto& s = body["scene_change_detection"];
                if (s.contains("enabled")) config::Set<bool>("video_tampering.scene_change_detection.enabled", s["enabled"].get<bool>());
                if (s.contains("sensitivity")) config::Set<int>("video_tampering.scene_change_detection.sensitivity", s["sensitivity"].get<int>());
            }
            if (body.contains("too_dark_detection")) {
                auto& td = body["too_dark_detection"];
                if (td.contains("enabled")) config::Set<bool>("video_tampering.too_dark_detection.enabled", td["enabled"].get<bool>());
                if (td.contains("threshold")) config::Set<int>("video_tampering.too_dark_detection.threshold", td["threshold"].get<int>());
            }
            if (body.contains("too_bright_detection")) {
                auto& tb = body["too_bright_detection"];
                if (tb.contains("enabled")) config::Set<bool>("video_tampering.too_bright_detection.enabled", tb["enabled"].get<bool>());
                if (tb.contains("threshold")) config::Set<int>("video_tampering.too_bright_detection.threshold", tb["threshold"].get<int>());
            }
            
            config::Save();
            
            // Apply to tamper detection engine
            auto& tamper = ai::TamperDetectionEngine::Instance();
            ai::TamperConfig tamper_cfg;
            tamper_cfg.enabled = config::Get<bool>("video_tampering.enabled", false);
            tamper_cfg.dwell_frames = config::Get<int>("video_tampering.dwell_time_sec", 3) * 12;  // ~12fps
            tamper_cfg.defocus.enabled = config::Get<bool>("video_tampering.defocus_detection.enabled", true);
            tamper_cfg.masking.enabled = config::Get<bool>("video_tampering.masking_detection.enabled", true);
            tamper_cfg.scene_change.enabled = config::Get<bool>("video_tampering.scene_change_detection.enabled", true);
            tamper_cfg.exposure.enabled = config::Get<bool>("video_tampering.too_dark_detection.enabled", true) || 
                                          config::Get<bool>("video_tampering.too_bright_detection.enabled", true);
            tamper_cfg.auto_reset.enabled = config::Get<bool>("video_tampering.auto_recovery", true);
            
            // Convert sensitivity to enum
            int sens = config::Get<int>("video_tampering.sensitivity", 50);
            if (sens < 33) tamper_cfg.sensitivity = ai::TamperSensitivity::kLow;
            else if (sens < 66) tamper_cfg.sensitivity = ai::TamperSensitivity::kMedium;
            else tamper_cfg.sensitivity = ai::TamperSensitivity::kHigh;
            
            tamper.SetConfig(tamper_cfg);
            
            // Handle runtime enable/disable of tamper engine
            if (tamper_cfg.enabled && !tamper.IsInitialized()) {
                // Engine was disabled at boot - need to init and start it now
                tamper_cfg.width = 320;
                tamper_cfg.height = 180;
                if (tamper.Init(tamper_cfg)) {
                    tamper.Start();
                    spdlog::info("Tamper Detection Engine initialized and started via API");
                } else {
                    spdlog::warn("Tamper Detection Engine runtime initialization failed");
                }
            } else if (tamper_cfg.enabled && tamper.IsInitialized() && !tamper.IsRunning()) {
                tamper.Start();
                spdlog::info("Tamper Detection Engine started via API");
            } else if (!tamper_cfg.enabled && tamper.IsRunning()) {
                tamper.Stop();
                spdlog::info("Tamper Detection Engine stopped via API");
            }
            
            // Update AnalyticsEngine config so processing loop feeds/skips frames
            auto& analytics = ai::AnalyticsEngine::Instance();
            auto acfg = analytics.GetConfig();
            acfg.tamper_detection.enabled = tamper_cfg.enabled;
            analytics.SetConfig(acfg);
            
            // Return updated config
            json j;
            j["enabled"] = config::Get<bool>("video_tampering.enabled", false);
            j["sensitivity"] = config::Get<int>("video_tampering.sensitivity", 50);
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in video tampering handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// GET/PUT /api/v1/analytics/tampering/schedule - Tampering schedule
// ============================================================================
api::Response HandleTamperingSchedule(const api::RequestContext& ctx) {
    // Use common schedule handler with feature name
    return HandleFeatureSchedule(ctx, features::TAMPERING);
}

// ============================================================================
// GET/PUT /api/v1/analytics/tampering/actions - Tampering event actions
// ============================================================================
api::Response HandleTamperingActions(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            json j;
            j["recording"] = {
                {"enabled", config::Get<bool>("video_tampering.actions.recording.enabled", true)},
                {"pre_record_sec", config::Get<int>("video_tampering.actions.recording.pre_record_sec", 5)},
                {"post_record_sec", config::Get<int>("video_tampering.actions.recording.post_record_sec", 30)}
            };
            j["snapshot"] = {
                {"enabled", config::Get<bool>("video_tampering.actions.snapshot.enabled", true)},
                {"count", config::Get<int>("video_tampering.actions.snapshot.count", 1)}
            };
            j["https_notify"] = {
                {"enabled", config::Get<bool>("video_tampering.actions.https_notify.enabled", false)},
                {"url", config::Get<std::string>("video_tampering.actions.https_notify.url", "")}
            };
            j["mqtt"] = {
                {"enabled", config::Get<bool>("video_tampering.actions.mqtt.enabled", false)},
                {"topic", config::Get<std::string>("video_tampering.actions.mqtt.topic", "camera/events/tampering")}
            };
            j["email"] = {
                {"enabled", config::Get<bool>("video_tampering.actions.email.enabled", false)},
                {"subject", config::Get<std::string>("video_tampering.actions.email.subject", "ALERT: Video Tampering Detected")},
                {"priority", config::Get<std::string>("video_tampering.actions.email.priority", "high")}
            };
            j["ftp_upload"] = {
                {"enabled", config::Get<bool>("video_tampering.actions.ftp_upload.enabled", false)},
                {"path", config::Get<std::string>("video_tampering.actions.ftp_upload.path", "/tampering")}
            };
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            std::string base = "video_tampering.actions";
            
            // Recording
            if (body.contains("recording")) {
                auto& rec = body["recording"];
                if (rec.contains("enabled")) config::Set<bool>(base + ".recording.enabled", rec["enabled"].get<bool>());
                if (rec.contains("pre_record_sec")) config::Set<int>(base + ".recording.pre_record_sec", rec["pre_record_sec"].get<int>());
                if (rec.contains("post_record_sec")) config::Set<int>(base + ".recording.post_record_sec", rec["post_record_sec"].get<int>());
            }
            
            // Snapshot
            if (body.contains("snapshot")) {
                auto& snap = body["snapshot"];
                if (snap.contains("enabled")) config::Set<bool>(base + ".snapshot.enabled", snap["enabled"].get<bool>());
                if (snap.contains("count")) config::Set<int>(base + ".snapshot.count", snap["count"].get<int>());
            }
            
            // HTTPS notification
            if (body.contains("https_notify")) {
                auto& https = body["https_notify"];
                if (https.contains("enabled")) config::Set<bool>(base + ".https_notify.enabled", https["enabled"].get<bool>());
                if (https.contains("url")) config::Set<std::string>(base + ".https_notify.url", https["url"].get<std::string>());
            }
            
            // MQTT
            if (body.contains("mqtt")) {
                auto& mqtt = body["mqtt"];
                if (mqtt.contains("enabled")) config::Set<bool>(base + ".mqtt.enabled", mqtt["enabled"].get<bool>());
                if (mqtt.contains("topic")) config::Set<std::string>(base + ".mqtt.topic", mqtt["topic"].get<std::string>());
            }
            
            // Email
            if (body.contains("email")) {
                auto& email = body["email"];
                if (email.contains("enabled")) config::Set<bool>(base + ".email.enabled", email["enabled"].get<bool>());
                if (email.contains("subject")) config::Set<std::string>(base + ".email.subject", email["subject"].get<std::string>());
                if (email.contains("priority")) config::Set<std::string>(base + ".email.priority", email["priority"].get<std::string>());
            }
            
            // FTP upload
            if (body.contains("ftp_upload")) {
                auto& ftp = body["ftp_upload"];
                if (ftp.contains("enabled")) config::Set<bool>(base + ".ftp_upload.enabled", ftp["enabled"].get<bool>());
                if (ftp.contains("path")) config::Set<std::string>(base + ".ftp_upload.path", ftp["path"].get<std::string>());
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
        spdlog::error("Error in tampering actions handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// GET /api/v1/analytics/tampering/status - Live tampering status
// ============================================================================
api::Response HandleTamperingStatus(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            auto& tamper = ai::TamperDetectionEngine::Instance();
            auto stats = tamper.GetStats();
            auto last_result = tamper.GetLastResult();
            auto current_alarms = tamper.GetCurrentAlarms();
            
            json j;
            j["running"] = tamper.IsRunning();
            j["frames_processed"] = stats.frames_processed;
            j["current_alarms"] = {
                {"defocus", (static_cast<int>(current_alarms) & static_cast<int>(ai::TamperType::kDefocus)) != 0},
                {"masking", (static_cast<int>(current_alarms) & static_cast<int>(ai::TamperType::kMasking)) != 0},
                {"scene_change", (static_cast<int>(current_alarms) & static_cast<int>(ai::TamperType::kSceneChange)) != 0},
                {"too_dark", (static_cast<int>(current_alarms) & static_cast<int>(ai::TamperType::kTooDark)) != 0},
                {"too_bright", (static_cast<int>(current_alarms) & static_cast<int>(ai::TamperType::kTooBright)) != 0}
            };
            j["statistics"] = {
                {"defocus_events", stats.defocus_events},
                {"masking_events", stats.masking_events},
                {"scene_change_events", stats.scene_change_events},
                {"too_dark_events", stats.too_dark_events},
                {"too_bright_events", stats.too_bright_events},
                {"auto_resets", stats.auto_resets}
            };
            j["last_metrics"] = {
                {"brightness", last_result.mean_brightness},
                {"blur", last_result.laplacian_variance},
                {"edge_ratio", last_result.edge_ratio},
                {"histogram_diff", last_result.histogram_diff}
            };
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in tampering status handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
