/**
 * @file analytics_common.h
 * @brief Common types, helpers, and utilities shared across all analytics modules
 * 
 * This file provides:
 * - Common JSON builders for schedule and actions (shared by all features)
 * - Schedule rule structures
 * - Event action structures
 * - Helper functions for parsing and serialization
 * 
 * PLANNED FEATURES (structure ready for modular expansion):
 * - Motion Detection (implemented)
 * - Video Tampering (implemented)
 * - Face Detection
 * - Smart Detection (Pedestrian/Vehicle/Animal)
 * - License Plate Recognition
 * - Line Crossing / Counting
 * - Zone Intrusion (Enter/Exit Region)
 * - Object Detection
 * - Cross Counting
 * - Heatmap Analysis
 * - Queue Length Detection
 * - Crowd Density
 * - Rare Sound Detection
 * 
 * PLANNED MANAGEMENT:
 * - List Management (faces, plates, etc.)
 * - Statistics/Analytics for counting features
 */

#pragma once

#include "../../api_handlers.h"
#include <nlohmann/json.hpp>
#include <ipcam/config.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

using json = nlohmann::json;

// Round float to avoid ugly IEEE 754 artifacts (e.g. 0.20000000298023224 → 0.2)
inline double round_json(float v, int decimals = 4) {
    double factor = 1.0;
    for (int i = 0; i < decimals; ++i) factor *= 10.0;
    return std::round(static_cast<double>(v) * factor) / factor;
}

// ============================================================================
// Feature Module Names (for config paths and routing)
// ============================================================================
namespace features {
    constexpr const char* MOTION      = "motion_detection";
    constexpr const char* TAMPERING   = "video_tampering";
    constexpr const char* SMART       = "smart_detection";
    constexpr const char* FACE        = "face_detection";
    constexpr const char* LPR         = "license_plate";
    constexpr const char* LINE_CROSS  = "line_crossing";
    constexpr const char* INTRUSION   = "zone_intrusion";
    constexpr const char* OBJECT      = "object_detection";
    constexpr const char* COUNTING    = "cross_counting";
    constexpr const char* HEATMAP     = "heatmap";
    constexpr const char* QUEUE       = "queue_length";
    constexpr const char* CROWD       = "crowd_density";
    constexpr const char* SOUND       = "rare_sound";
}

// ============================================================================
// Schedule Rule Structure
// ============================================================================
struct ScheduleRule {
    std::string name;
    std::vector<int> days;  // 0=Sun, 1=Mon, ... 6=Sat
    std::string start_time; // "HH:MM"
    std::string end_time;   // "HH:MM"
    bool enabled{true};
};

// ============================================================================
// Event Action Settings
// ============================================================================
struct EventActions {
    struct {
        bool enabled{false};
        int pre_record_sec{5};
        int post_record_sec{10};
    } recording;
    
    struct {
        bool enabled{false};
        int count{3};
    } snapshot;
    
    struct {
        bool enabled{false};
        std::string url;
    } https_notify;
    
    struct {
        bool enabled{false};
        std::string topic;
    } mqtt;
    
    struct {
        bool enabled{false};
        std::vector<std::string> recipients;
    } email;
    
    struct {
        bool enabled{false};
        std::string path;
    } ftp_upload;
};

// ============================================================================
// Common JSON Helpers for Schedule
// ============================================================================

/**
 * @brief Build schedule JSON from config path
 * @param feature_name Feature config key (e.g., "motion_detection")
 * @return JSON object with schedule configuration
 */
inline json BuildScheduleJson(const std::string& feature_name) {
    json j;
    std::string base = feature_name + ".schedule";
    
    j["enabled"] = config::Get<bool>(base + ".enabled", false);
    j["always_on"] = config::Get<bool>(base + ".always_on", true);
    
    // Get rules array
    j["rules"] = json::array();
    auto rules = config::Get<json>(base + ".rules", json::array());
    if (rules.is_array()) {
        j["rules"] = rules;
    }
    
    return j;
}

/**
 * @brief Update schedule config from JSON
 * @param feature_name Feature config key
 * @param body JSON with schedule updates
 */
inline void UpdateScheduleFromJson(const std::string& feature_name, const json& body) {
    std::string base = feature_name + ".schedule";
    
    if (body.contains("enabled")) {
        config::Set<bool>(base + ".enabled", body["enabled"].get<bool>());
    }
    if (body.contains("always_on")) {
        config::Set<bool>(base + ".always_on", body["always_on"].get<bool>());
    }
    if (body.contains("rules")) {
        config::Set<json>(base + ".rules", body["rules"]);
    }
}

// ============================================================================
// Common JSON Helpers for Actions/Events
// ============================================================================

/**
 * @brief Build actions JSON from config path
 * @param feature_name Feature config key (e.g., "motion_detection")
 * @return JSON object with event action configuration
 */
inline json BuildActionsJson(const std::string& feature_name) {
    json j;
    std::string base = feature_name + ".actions";
    
    // Recording
    j["recording"]["enabled"] = config::Get<bool>(base + ".recording.enabled", false);
    j["recording"]["pre_record_sec"] = config::Get<int>(base + ".recording.pre_record_sec", 5);
    j["recording"]["post_record_sec"] = config::Get<int>(base + ".recording.post_record_sec", 10);
    
    // Snapshot
    j["snapshot"]["enabled"] = config::Get<bool>(base + ".snapshot.enabled", false);
    j["snapshot"]["count"] = config::Get<int>(base + ".snapshot.count", 3);
    
    // HTTPS notification
    j["https_notify"]["enabled"] = config::Get<bool>(base + ".https_notify.enabled", false);
    j["https_notify"]["url"] = config::Get<std::string>(base + ".https_notify.url", "");
    
    // MQTT
    j["mqtt"]["enabled"] = config::Get<bool>(base + ".mqtt.enabled", false);
    j["mqtt"]["topic"] = config::Get<std::string>(base + ".mqtt.topic", "camera/events/" + feature_name);
    
    // Email
    j["email"]["enabled"] = config::Get<bool>(base + ".email.enabled", false);
    
    // FTP upload
    j["ftp_upload"]["enabled"] = config::Get<bool>(base + ".ftp_upload.enabled", false);
    j["ftp_upload"]["path"] = config::Get<std::string>(base + ".ftp_upload.path", "/" + feature_name);
    
    return j;
}

/**
 * @brief Update actions config from JSON
 * @param feature_name Feature config key
 * @param body JSON with actions updates
 */
inline void UpdateActionsFromJson(const std::string& feature_name, const json& body) {
    std::string base = feature_name + ".actions";
    
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
    }
    
    // FTP upload
    if (body.contains("ftp_upload")) {
        auto& ftp = body["ftp_upload"];
        if (ftp.contains("enabled")) config::Set<bool>(base + ".ftp_upload.enabled", ftp["enabled"].get<bool>());
        if (ftp.contains("path")) config::Set<std::string>(base + ".ftp_upload.path", ftp["path"].get<std::string>());
    }
}

// ============================================================================
// Common JSON Helpers for Zones (polygon-based regions)
// ============================================================================

/**
 * @brief Build zones JSON from config path
 * @param feature_name Feature config key
 * @return JSON array with zone configurations
 */
inline json BuildZonesJson(const std::string& feature_name) {
    std::string base = feature_name + ".zones";
    return config::Get<json>(base, json::array());
}

/**
 * @brief Update zones config from JSON array
 * @param feature_name Feature config key
 * @param zones JSON array with zone configurations
 */
inline void UpdateZonesFromJson(const std::string& feature_name, const json& zones) {
    std::string base = feature_name + ".zones";
    config::Set<json>(base, zones);
}

// ============================================================================
// Generic Feature Handler Generator (for consistent structure)
// ============================================================================

/**
 * @brief Create a generic schedule handler for any feature
 * @param feature_name Feature config key
 * @return Handler function
 */
inline api::Response HandleFeatureSchedule(const api::RequestContext& ctx, const std::string& feature_name) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            resp.status_code = 200;
            resp.body = BuildScheduleJson(feature_name).dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            UpdateScheduleFromJson(feature_name, body);
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
        spdlog::error("Error in {} schedule handler: {}", feature_name, e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

/**
 * @brief Create a generic actions handler for any feature
 * @param feature_name Feature config key
 * @return Handler function
 */
inline api::Response HandleFeatureActions(const api::RequestContext& ctx, const std::string& feature_name) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            resp.status_code = 200;
            resp.body = BuildActionsJson(feature_name).dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            UpdateActionsFromJson(feature_name, body);
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
        spdlog::error("Error in {} actions handler: {}", feature_name, e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

/**
 * @brief Create a generic zones handler for any feature
 * @param feature_name Feature config key
 * @return Handler function
 */
inline api::Response HandleFeatureZones(const api::RequestContext& ctx, const std::string& feature_name) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            resp.status_code = 200;
            resp.body = BuildZonesJson(feature_name).dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            UpdateZonesFromJson(feature_name, body);
            config::Save();
            
            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else if (ctx.method == "POST") {
            // Add a new zone
            auto body = json::parse(ctx.body);
            auto zones = BuildZonesJson(feature_name);
            
            // Generate ID if not provided
            if (!body.contains("id")) {
                int max_id = 0;
                for (const auto& z : zones) {
                    if (z.contains("id") && z["id"].get<int>() > max_id) {
                        max_id = z["id"].get<int>();
                    }
                }
                body["id"] = max_id + 1;
            }
            
            zones.push_back(body);
            UpdateZonesFromJson(feature_name, zones);
            config::Save();
            
            resp.status_code = 201;
            resp.body = body.dump(2);
        }
        else if (ctx.method == "DELETE") {
            auto body = json::parse(ctx.body);
            if (!body.contains("id")) {
                resp.status_code = 400;
                resp.body = R"({"error": "Missing zone id"})";
                return resp;
            }
            
            int zone_id = body["id"].get<int>();
            auto zones = BuildZonesJson(feature_name);
            json new_zones = json::array();
            bool found = false;
            
            for (const auto& z : zones) {
                if (z.contains("id") && z["id"].get<int>() == zone_id) {
                    found = true;
                } else {
                    new_zones.push_back(z);
                }
            }
            
            if (found) {
                UpdateZonesFromJson(feature_name, new_zones);
                config::Save();
                resp.status_code = 200;
                resp.body = R"({"success": true})";
            } else {
                resp.status_code = 404;
                resp.body = R"({"error": "Zone not found"})";
            }
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in {} zones handler: {}", feature_name, e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
