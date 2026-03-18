#include "handlers/osd_handler.h"
#include <ipcam/osd_overlay.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;
using namespace ipcam::webserver::api;
using namespace ipcam::platform;

namespace ipcam {
namespace webserver {
namespace handlers {

// Forward declaration for delegation from HandleOsdStreamConfig
Response HandleOsdPrivacyMask(const RequestContext& ctx);

// Helper to serialize Stream Config
json StreamConfigToJson(const OsdStreamConfig& cfg) {
    json j;
    j["enabled"] = cfg.enabled;
    j["stamp_width"] = cfg.stamp_width;
    j["stamp_height"] = cfg.stamp_height;
    j["stamp_offset_x"] = cfg.stamp_offset_x;
    j["stamp_offset_y"] = cfg.stamp_offset_y;
    
    j["timestamp"] = {
        {"enabled", cfg.timestamp.enabled},
        {"position_x", cfg.timestamp.position_x},
        {"position_y", cfg.timestamp.position_y},
        {"format", cfg.timestamp.format},
        {"font_size", cfg.timestamp.font_size},
        {"outline_enabled", cfg.timestamp.outline_enabled},
        {"outline_width", cfg.timestamp.outline_width}
    };
    
    j["text"] = {
        {"enabled", cfg.text.enabled},
        {"content", cfg.text.text},
        {"position_x", cfg.text.position_x},
        {"position_y", cfg.text.position_y},
        {"font_size", cfg.text.font_size},
        {"outline_enabled", cfg.text.outline_enabled}
    };
    
    j["logo"] = {
        {"enabled", cfg.logo.enabled},
        {"position_x", cfg.logo.position_x},
        {"position_y", cfg.logo.position_y},
        {"outline_enabled", cfg.logo.outline_enabled},
        {"outline_width", cfg.logo.outline_width}
    };
    
    // Per-stream privacy mask
    j["privacy_mask"] = {
        {"enabled", cfg.privacy_mask.enabled}
    };
    json regions = json::array();
    for (int i = 0; i < OsdStreamPrivacyMask::kMaxRegions; i++) {
        regions.push_back({
            {"enabled", cfg.privacy_mask.regions[i].enabled},
            {"x1", cfg.privacy_mask.regions[i].x1},
            {"y1", cfg.privacy_mask.regions[i].y1},
            {"x2", cfg.privacy_mask.regions[i].x2},
            {"y2", cfg.privacy_mask.regions[i].y2},
            {"color", cfg.privacy_mask.regions[i].color}
        });
    }
    j["privacy_mask"]["regions"] = regions;
    
    return j;
}

// Helper to deserialize Stream Config
void JsonToStreamConfig(const json& j, OsdStreamConfig& cfg) {
    if (j.contains("enabled")) cfg.enabled = j["enabled"];
    if (j.contains("stamp_width")) cfg.stamp_width = j["stamp_width"];
    if (j.contains("stamp_height")) cfg.stamp_height = j["stamp_height"];
    if (j.contains("stamp_offset_x")) cfg.stamp_offset_x = j["stamp_offset_x"];
    if (j.contains("stamp_offset_y")) cfg.stamp_offset_y = j["stamp_offset_y"];
    
    if (j.contains("timestamp")) {
        const auto& t = j["timestamp"];
        if (t.contains("enabled")) cfg.timestamp.enabled = t["enabled"];
        if (t.contains("position_x")) cfg.timestamp.position_x = t["position_x"];
        if (t.contains("position_y")) cfg.timestamp.position_y = t["position_y"];
        if (t.contains("format")) cfg.timestamp.format = t["format"];
        if (t.contains("font_size")) cfg.timestamp.font_size = t["font_size"];
        if (t.contains("outline_enabled")) cfg.timestamp.outline_enabled = t["outline_enabled"];
        if (t.contains("outline_width")) cfg.timestamp.outline_width = t["outline_width"];
    }
    
    if (j.contains("text")) {
        const auto& t = j["text"];
        if (t.contains("enabled")) cfg.text.enabled = t["enabled"];
        if (t.contains("content")) cfg.text.text = t["content"];
        if (t.contains("position_x")) cfg.text.position_x = t["position_x"];
        if (t.contains("position_y")) cfg.text.position_y = t["position_y"];
        if (t.contains("font_size")) cfg.text.font_size = t["font_size"];
        if (t.contains("outline_enabled")) cfg.text.outline_enabled = t["outline_enabled"];
    }
    
    if (j.contains("logo")) {
        const auto& l = j["logo"];
        if (l.contains("enabled")) cfg.logo.enabled = l["enabled"];
        if (l.contains("position_x")) cfg.logo.position_x = l["position_x"];
        if (l.contains("position_y")) cfg.logo.position_y = l["position_y"];
        if (l.contains("outline_enabled")) cfg.logo.outline_enabled = l["outline_enabled"];
        if (l.contains("outline_width")) cfg.logo.outline_width = l["outline_width"];
    }
    
    // Per-stream privacy mask
    if (j.contains("privacy_mask")) {
        const auto& pm = j["privacy_mask"];
        if (pm.contains("enabled")) cfg.privacy_mask.enabled = pm["enabled"];
        
        if (pm.contains("regions") && pm["regions"].is_array()) {
            const auto& regions = pm["regions"];
            for (size_t i = 0; i < regions.size() && i < OsdStreamPrivacyMask::kMaxRegions; i++) {
                const auto& r = regions[i];
                auto& region = cfg.privacy_mask.regions[i];
                if (r.contains("enabled")) region.enabled = r["enabled"];
                if (r.contains("x1")) region.x1 = r["x1"];
                if (r.contains("y1")) region.y1 = r["y1"];
                if (r.contains("x2")) region.x2 = r["x2"];
                if (r.contains("y2")) region.y2 = r["y2"];
                if (r.contains("color")) region.color = r["color"];
            }
        }
    }
}

Response HandleOsdStreamConfig(const RequestContext& ctx) {
    spdlog::info("HandleOsdStreamConfig called: {}", ctx.uri);
    
    // Check if this is a privacy_mask request and delegate to that handler
    if (ctx.uri.find("/privacy_mask") != std::string::npos) {
        return HandleOsdPrivacyMask(ctx);
    }
    
    Response resp;
    
    int stream_id = -1;
    size_t pos_stream = ctx.uri.find("/stream/");
    if (pos_stream != std::string::npos) {
        size_t pos_slash = ctx.uri.find("/", pos_stream + 8);
        std::string id_str;
        if (pos_slash != std::string::npos)
             id_str = ctx.uri.substr(pos_stream + 8, pos_slash - (pos_stream + 8));
        else
             id_str = ctx.uri.substr(pos_stream + 8);
             
        try {
            stream_id = std::stoi(id_str);
        } catch (...) {
            stream_id = -1;
        }
    }
    
    if (stream_id < 0 || stream_id >= OsdOverlay::kMaxStreams) {
        resp.status_code = 400;
        json j;
        j["error"] = "Invalid stream ID or format";
        resp.body = j.dump();
        resp.content_type = "application/json";
        return resp;
    }
    
    if (ctx.method == "GET") {
        try {
            // Get current config
            const OsdConfig& global_cfg = OsdOverlay::Instance().GetConfig();
            const OsdStreamConfig& stream_cfg = global_cfg.streams[stream_id];
            
            json j = StreamConfigToJson(stream_cfg);
            resp.status_code = 200;
            resp.body = j.dump(2);
            resp.content_type = "application/json";
        } catch (const std::exception& e) {
            resp.status_code = 500;
            json j; j["error"] = e.what();
            resp.body = j.dump();
            resp.content_type = "application/json";
        }
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        try {
            json j = json::parse(ctx.body);
            
            // Get update modify set
            OsdConfig config = OsdOverlay::Instance().GetConfig();
            OsdStreamConfig old_cfg = config.streams[stream_id];
            JsonToStreamConfig(j, config.streams[stream_id]);
            OsdOverlay::Instance().SetConfig(config);
            
            // Persist font_size changes to config file
            std::string stream_base = "osd.streams.stream" + std::to_string(stream_id);
            const auto& new_cfg = config.streams[stream_id];
            
            if (new_cfg.timestamp.font_size != old_cfg.timestamp.font_size) {
                config::Set<int>(stream_base + ".timestamp.font_size", new_cfg.timestamp.font_size);
                spdlog::info("Stream {} timestamp font_size changed: {} -> {}", 
                    stream_id, old_cfg.timestamp.font_size, new_cfg.timestamp.font_size);
            }
            if (new_cfg.text.font_size != old_cfg.text.font_size) {
                config::Set<int>(stream_base + ".text.font_size", new_cfg.text.font_size);
                spdlog::info("Stream {} text font_size changed: {} -> {}", 
                    stream_id, old_cfg.text.font_size, new_cfg.text.font_size);
            }
            
            // Persist other commonly changed settings
            config::Set<bool>(stream_base + ".timestamp.enabled", new_cfg.timestamp.enabled);
            config::Set<bool>(stream_base + ".text.enabled", new_cfg.text.enabled);
            config::Set<bool>(stream_base + ".logo.enabled", new_cfg.logo.enabled);
            config::Set<int>(stream_base + ".timestamp.position_x", new_cfg.timestamp.position_x);
            config::Set<int>(stream_base + ".timestamp.position_y", new_cfg.timestamp.position_y);
            config::Set<int>(stream_base + ".text.position_x", new_cfg.text.position_x);
            config::Set<int>(stream_base + ".text.position_y", new_cfg.text.position_y);
            config::Set<std::string>(stream_base + ".timestamp.format", new_cfg.timestamp.format);
            config::Set<std::string>(stream_base + ".text.text", new_cfg.text.text);
            
            config::Save();
            
            resp.status_code = 200;
            json res; res["status"] = "ok";
            resp.body = res.dump();
            resp.content_type = "application/json";
        } catch (const std::exception& e) {
            resp.status_code = 400;
            json j; j["error"] = e.what();
            resp.body = j.dump();
            resp.content_type = "application/json";
        }
    } else {
        resp.status_code = 405; // Method not allowed
    }
    
    return resp;
}

// Helper to serialize Privacy Mask Config
json PrivacyMaskRegionToJson(const PrivacyMaskRegion& region) {
    return {
        {"enabled", region.enabled},
        {"x1", region.x1},
        {"y1", region.y1},
        {"x2", region.x2},
        {"y2", region.y2},
        {"color", region.color}
    };
}

void JsonToPrivacyMaskRegion(const json& j, PrivacyMaskRegion& region) {
    if (j.contains("enabled")) region.enabled = j["enabled"];
    if (j.contains("x1")) region.x1 = j["x1"];
    if (j.contains("y1")) region.y1 = j["y1"];
    if (j.contains("x2")) region.x2 = j["x2"];
    if (j.contains("y2")) region.y2 = j["y2"];
    if (j.contains("color")) region.color = j["color"];
}

Response HandleOsdGlobalConfig(const RequestContext& ctx) {
    Response resp;
    
    if (ctx.method == "GET") {
        try {
            const OsdConfig& config = OsdOverlay::Instance().GetConfig();
            json j;
            
            // Global settings
            j["global"] = {
                {"font_path", config.global.font_path},
                {"logo_path", config.global.logo_path}
            };
            
            // Privacy Mask
            j["privacy_mask"] = {
                {"enabled", config.privacy_mask.enabled}
            };
            
            json regions = json::array();
            for (int i = 0; i < OsdPrivacyMaskConfig::kMaxRegions; i++) {
                regions.push_back(PrivacyMaskRegionToJson(config.privacy_mask.regions[i]));
            }
            j["privacy_mask"]["regions"] = regions;
            
            resp.status_code = 200;
            resp.body = j.dump(2);
            resp.content_type = "application/json";
        } catch (const std::exception& e) {
            resp.status_code = 500;
            json j; j["error"] = e.what();
            resp.body = j.dump();
            resp.content_type = "application/json";
        }
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        try {
            json j = json::parse(ctx.body);
            OsdConfig config = OsdOverlay::Instance().GetConfig();
            
            if (j.contains("global")) {
                if (j["global"].contains("font_path")) config.global.font_path = j["global"]["font_path"];
                if (j["global"].contains("logo_path")) config.global.logo_path = j["global"]["logo_path"];
            }
            
            if (j.contains("privacy_mask")) {
                const auto& pm = j["privacy_mask"];
                if (pm.contains("enabled")) config.privacy_mask.enabled = pm["enabled"];
                
                if (pm.contains("regions") && pm["regions"].is_array()) {
                    const auto& regions = pm["regions"];
                    for (size_t i = 0; i < regions.size() && i < OsdPrivacyMaskConfig::kMaxRegions; i++) {
                        JsonToPrivacyMaskRegion(regions[i], config.privacy_mask.regions[i]);
                    }
                }
            }
            
            OsdOverlay::Instance().SetConfig(config);
            
            resp.status_code = 200;
            json res; res["status"] = "ok";
            resp.body = res.dump();
            resp.content_type = "application/json";
        } catch (const std::exception& e) {
            resp.status_code = 400;
            json j; j["error"] = e.what();
            resp.body = j.dump();
            resp.content_type = "application/json";
        }
    } else {
        resp.status_code = 405; // Method not allowed
    }
    
    return resp;
}

Response HandleOsdPrivacyMask(const RequestContext& ctx) {
    spdlog::info("HandleOsdPrivacyMask called: {} {}", ctx.method, ctx.uri);
    Response resp;
    
    // Parse stream ID from URI: /api/osd/stream/{id}/privacy_mask[/apply]
    int stream_id = -1;
    bool is_apply = ctx.uri.find("/apply") != std::string::npos;
    
    size_t pos_stream = ctx.uri.find("/stream/");
    if (pos_stream != std::string::npos) {
        size_t pos_start = pos_stream + 8;
        size_t pos_end = ctx.uri.find("/", pos_start);
        std::string id_str = (pos_end != std::string::npos) 
            ? ctx.uri.substr(pos_start, pos_end - pos_start) 
            : ctx.uri.substr(pos_start);
        try {
            stream_id = std::stoi(id_str);
        } catch (...) {
            stream_id = -1;
        }
    }
    
    if (stream_id < 0 || stream_id >= OsdOverlay::kMaxStreams) {
        resp.status_code = 400;
        json j; j["error"] = "Invalid stream ID";
        resp.body = j.dump();
        resp.content_type = "application/json";
        return resp;
    }
    
    if (ctx.method == "GET") {
        try {
            const OsdConfig& config = OsdOverlay::Instance().GetConfig();
            const auto& pm = config.streams[stream_id].privacy_mask;
            
            json j;
            j["stream_id"] = stream_id;
            j["enabled"] = pm.enabled;
            j["max_regions"] = OsdStreamPrivacyMask::kMaxRegions;
            
            json regions = json::array();
            for (int i = 0; i < OsdStreamPrivacyMask::kMaxRegions; i++) {
                regions.push_back({
                    {"region_id", i},
                    {"enabled", pm.regions[i].enabled},
                    {"x1", pm.regions[i].x1},
                    {"y1", pm.regions[i].y1},
                    {"x2", pm.regions[i].x2},
                    {"y2", pm.regions[i].y2},
                    {"color", pm.regions[i].color}
                });
            }
            j["regions"] = regions;
            
            resp.status_code = 200;
            resp.body = j.dump(2);
            resp.content_type = "application/json";
        } catch (const std::exception& e) {
            resp.status_code = 500;
            json j; j["error"] = e.what();
            resp.body = j.dump();
            resp.content_type = "application/json";
        }
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        if (is_apply) {
            // Apply privacy mask to encoder
            try {
                bool success = OsdOverlay::Instance().ApplyPrivacyMaskToStream(stream_id);
                resp.status_code = success ? 200 : 500;
                json j;
                j["status"] = success ? "applied" : "failed";
                j["stream_id"] = stream_id;
                resp.body = j.dump();
                resp.content_type = "application/json";
            } catch (const std::exception& e) {
                resp.status_code = 500;
                json j; j["error"] = e.what();
                resp.body = j.dump();
                resp.content_type = "application/json";
            }
        } else {
            // Update privacy mask config
            try {
                json j = json::parse(ctx.body);
                OsdConfig config = OsdOverlay::Instance().GetConfig();
                auto& pm = config.streams[stream_id].privacy_mask;
                
                if (j.contains("enabled")) pm.enabled = j["enabled"];
                
                if (j.contains("regions") && j["regions"].is_array()) {
                    const auto& regions = j["regions"];
                    for (size_t i = 0; i < regions.size() && i < OsdStreamPrivacyMask::kMaxRegions; i++) {
                        const auto& r = regions[i];
                        auto& region = pm.regions[i];
                        if (r.contains("enabled")) region.enabled = r["enabled"];
                        if (r.contains("x1")) region.x1 = r["x1"];
                        if (r.contains("y1")) region.y1 = r["y1"];
                        if (r.contains("x2")) region.x2 = r["x2"];
                        if (r.contains("y2")) region.y2 = r["y2"];
                        if (r.contains("color")) region.color = r["color"];
                    }
                }
                
                OsdOverlay::Instance().SetConfig(config);
                
                // Auto-apply if requested
                bool applied = false;
                if (j.contains("apply") && j["apply"].get<bool>()) {
                    applied = OsdOverlay::Instance().ApplyPrivacyMaskToStream(stream_id);
                }
                
                resp.status_code = 200;
                json res; 
                res["status"] = "ok";
                res["stream_id"] = stream_id;
                res["applied"] = applied;
                resp.body = res.dump();
                resp.content_type = "application/json";
            } catch (const std::exception& e) {
                resp.status_code = 400;
                json j; j["error"] = e.what();
                resp.body = j.dump();
                resp.content_type = "application/json";
            }
        }
    } else {
        resp.status_code = 405;
    }
    
    return resp;
}

// ============================================================================
// Global Privacy Mask Handler (VIDEOPROC level)
// ============================================================================

Response HandleOsdGlobalPrivacyMask(const RequestContext& ctx) {
    Response resp;
    
    if (ctx.method == "GET") {
        try {
            const auto& pm = OsdOverlay::Instance().GetGlobalPrivacyMaskConfig();
            bool available = OsdOverlay::Instance().IsGlobalPrivacyMaskAvailable();
            
            json j;
            j["available"] = available;
            j["enabled"] = pm.enabled;
            j["max_regions"] = OsdPrivacyMaskConfig::kMaxRegions;
            j["description"] = "VIDEOPROC-level privacy mask (applies to all streams with auto-scaling)";
            
            json regions = json::array();
            for (int i = 0; i < OsdPrivacyMaskConfig::kMaxRegions; i++) {
                regions.push_back({
                    {"region_id", i},
                    {"enabled", pm.regions[i].enabled},
                    {"x1", pm.regions[i].x1},
                    {"y1", pm.regions[i].y1},
                    {"x2", pm.regions[i].x2},
                    {"y2", pm.regions[i].y2},
                    {"color", pm.regions[i].color}
                });
            }
            j["regions"] = regions;
            
            resp.status_code = 200;
            resp.body = j.dump(2);
            resp.content_type = "application/json";
        } catch (const std::exception& e) {
            resp.status_code = 500;
            json j; j["error"] = e.what();
            resp.body = j.dump();
            resp.content_type = "application/json";
        }
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        try {
            json req = json::parse(ctx.body);
            auto& osd = OsdOverlay::Instance();
            
            if (!osd.IsGlobalPrivacyMaskAvailable()) {
                resp.status_code = 503;
                json j; j["error"] = "Global privacy mask not available (VIDEOPROC mask paths not open)";
                resp.body = j.dump();
                resp.content_type = "application/json";
                return resp;
            }
            
            bool success = true;
            int regions_updated = 0;
            
            // Update master enable if specified
            if (req.contains("enabled")) {
                bool enabled = req["enabled"].get<bool>();
                if (!osd.SetGlobalPrivacyMaskEnabled(enabled)) {
                    success = false;
                }
            }
            
            // Update individual regions if specified
            if (req.contains("regions") && req["regions"].is_array()) {
                const auto& regions = req["regions"];
                for (const auto& r : regions) {
                    int region_id = r.value("region_id", -1);
                    if (region_id < 0 || region_id >= OsdPrivacyMaskConfig::kMaxRegions) {
                        continue;
                    }
                    
                    bool region_enabled = r.value("enabled", false);
                    int x1 = r.value("x1", 0);
                    int y1 = r.value("y1", 0);
                    int x2 = r.value("x2", 100);
                    int y2 = r.value("y2", 100);
                    uint32_t color = r.value("color", 0xFF000000u);
                    
                    if (osd.SetGlobalPrivacyMaskRegion(region_id, region_enabled, x1, y1, x2, y2, color)) {
                        regions_updated++;
                    }
                }
            }
            
            resp.status_code = success ? 200 : 500;
            json res;
            res["status"] = success ? "ok" : "partial_failure";
            res["regions_updated"] = regions_updated;
            resp.body = res.dump();
            resp.content_type = "application/json";
        } catch (const std::exception& e) {
            resp.status_code = 400;
            json j; j["error"] = e.what();
            resp.body = j.dump();
            resp.content_type = "application/json";
        }
    } else {
        resp.status_code = 405;
        json j; j["error"] = "Method not allowed";
        resp.body = j.dump();
        resp.content_type = "application/json";
    }
    
    return resp;
}

// ============================================================================
// OSD Auto-Scaling Handler
// ============================================================================

Response HandleOsdAutoScaling(const RequestContext& ctx) {
    spdlog::info("HandleOsdAutoScaling called: {} {}", ctx.method, ctx.uri);
    
    Response resp;
    auto& osd = OsdOverlay::Instance();
    
    if (ctx.method == "GET") {
        try {
            const auto& scaling = osd.GetAutoScalingConfig();
            
            json j;
            j["enabled"] = scaling.enabled;
            j["base_font_size"] = scaling.base_font_size;
            j["base_text_font_size"] = scaling.base_text_font_size;
            j["min_font_size"] = scaling.min_font_size;
            j["max_font_size"] = scaling.max_font_size;
            j["reference_width"] = scaling.reference_width;
            j["reference_height"] = scaling.reference_height;
            
            // Also include calculated font sizes for each stream
            json calculated = json::array();
            for (int i = 0; i < 3; i++) {
                std::string prefix = "media.video" + std::to_string(i + 1);
                int w = config::Get<int>(prefix + ".width", 1920);
                int h = config::Get<int>(prefix + ".height", 1080);
                
                json stream_info;
                stream_info["stream_id"] = i;
                stream_info["resolution"] = std::to_string(w) + "x" + std::to_string(h);
                stream_info["timestamp_font_size"] = osd.CalculateScaledFontSize(i, scaling.base_font_size);
                stream_info["text_font_size"] = osd.CalculateScaledFontSize(i, scaling.base_text_font_size);
                calculated.push_back(stream_info);
            }
            j["calculated_sizes"] = calculated;
            
            resp.status_code = 200;
            resp.body = j.dump(2);
            resp.content_type = "application/json";
        } catch (const std::exception& e) {
            resp.status_code = 500;
            json j; j["error"] = e.what();
            resp.body = j.dump();
            resp.content_type = "application/json";
        }
    } else if (ctx.method == "POST") {
        try {
            json body = json::parse(ctx.body);
            
            OsdAutoScalingConfig scaling = osd.GetAutoScalingConfig();
            
            if (body.contains("enabled")) {
                scaling.enabled = body["enabled"];
            }
            if (body.contains("base_font_size")) {
                scaling.base_font_size = body["base_font_size"];
            }
            if (body.contains("base_text_font_size")) {
                scaling.base_text_font_size = body["base_text_font_size"];
            }
            if (body.contains("min_font_size")) {
                scaling.min_font_size = body["min_font_size"];
            }
            if (body.contains("max_font_size")) {
                scaling.max_font_size = body["max_font_size"];
            }
            if (body.contains("reference_width")) {
                scaling.reference_width = body["reference_width"];
            }
            if (body.contains("reference_height")) {
                scaling.reference_height = body["reference_height"];
            }
            
            osd.SetAutoScalingConfig(scaling);
            
            resp.status_code = 200;
            json res;
            res["status"] = "ok";
            res["message"] = "Auto-scaling configuration updated";
            res["enabled"] = scaling.enabled;
            res["base_font_size"] = scaling.base_font_size;
            resp.body = res.dump();
            resp.content_type = "application/json";
            
            spdlog::info("OSD Auto-scaling updated: enabled={}, base_font={}", 
                         scaling.enabled, scaling.base_font_size);
        } catch (const std::exception& e) {
            resp.status_code = 400;
            json j; j["error"] = e.what();
            resp.body = j.dump();
            resp.content_type = "application/json";
        }
    } else {
        resp.status_code = 405;
        json j; j["error"] = "Method not allowed";
        resp.body = j.dump();
        resp.content_type = "application/json";
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
