/**
 * @file linecross_handler.cpp
 * @brief Line crossing detection API handlers
 */

#include "linecross_handler.h"
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
// Line Crossing Handler
// ============================================================================
api::Response HandleLineCrossing(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        auto& analytics = ai::AnalyticsEngine::Instance();
        
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("line_crossing.enabled", false);
            j["counting"] = {
                {"enabled", config::Get<bool>("line_crossing.counting.enabled", false)},
                {"reset_daily", config::Get<bool>("line_crossing.counting.reset_daily", true)},
                {"reset_time", config::Get<std::string>("line_crossing.counting.reset_time", "00:00")}
            };
            j["object_filter"] = {
                {"pedestrian", config::Get<bool>("line_crossing.object_filter.pedestrian", true)},
                {"vehicle", config::Get<bool>("line_crossing.object_filter.vehicle", true)},
                {"bicycle", config::Get<bool>("line_crossing.object_filter.bicycle", true)}
            };
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            if (body.contains("enabled")) {
                bool en = body["enabled"].get<bool>();
                config::Set<bool>("line_crossing.enabled", en);
                analytics.EnableLineCrossing(en);
            }
            
            if (body.contains("counting")) {
                auto& c = body["counting"];
                if (c.contains("enabled")) 
                    config::Set<bool>("line_crossing.counting.enabled", c["enabled"].get<bool>());
                if (c.contains("reset_daily")) 
                    config::Set<bool>("line_crossing.counting.reset_daily", c["reset_daily"].get<bool>());
                if (c.contains("reset_time")) 
                    config::Set<std::string>("line_crossing.counting.reset_time", c["reset_time"].get<std::string>());
            }
            
            if (body.contains("object_filter")) {
                auto& f = body["object_filter"];
                if (f.contains("pedestrian")) 
                    config::Set<bool>("line_crossing.object_filter.pedestrian", f["pedestrian"].get<bool>());
                if (f.contains("vehicle")) 
                    config::Set<bool>("line_crossing.object_filter.vehicle", f["vehicle"].get<bool>());
                if (f.contains("bicycle")) 
                    config::Set<bool>("line_crossing.object_filter.bicycle", f["bicycle"].get<bool>());
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
        spdlog::error("Error in line crossing handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// Line Cross Lines Handler (zones/lines config)
// ============================================================================
api::Response HandleLineCrossLines(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        const std::string lines_key = "line_crossing.lines";
        
        if (ctx.method == "GET") {
            auto lines = config::GetOptional<json>(lines_key);
            if (lines.has_value()) {
                resp.body = lines.value().dump(2);
            } else {
                // Return default empty array with example structure
                json j = json::array();
                resp.body = j.dump(2);
            }
            resp.status_code = 200;
        }
        else if (ctx.method == "POST") {
            // Add new line
            auto body = json::parse(ctx.body);
            auto lines = config::GetOptional<json>(lines_key).value_or(json::array());
            
            // Generate new ID
            uint32_t new_id = 1;
            for (const auto& line : lines) {
                if (line.contains("id") && line["id"].get<uint32_t>() >= new_id) {
                    new_id = line["id"].get<uint32_t>() + 1;
                }
            }
            
            json new_line;
            new_line["id"] = new_id;
            new_line["name"] = body.value("name", "Line " + std::to_string(new_id));
            new_line["enabled"] = body.value("enabled", true);
            new_line["x1"] = body.value("x1", 0.25f);
            new_line["y1"] = body.value("y1", 0.5f);
            new_line["x2"] = body.value("x2", 0.75f);
            new_line["y2"] = body.value("y2", 0.5f);
            new_line["direction"] = body.value("direction", "both"); // in, out, both
            new_line["color"] = body.value("color", "#00FF00");
            
            lines.push_back(new_line);
            config::Set<json>(lines_key, lines);
            config::Save();
            
            resp.status_code = 201;
            resp.body = new_line.dump(2);
        }
        else if (ctx.method == "PUT") {
            // Update lines
            auto body = json::parse(ctx.body);
            config::Set<json>(lines_key, body);
            config::Save();
            
            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else if (ctx.method == "DELETE") {
            // Delete line by ID from query params
            auto id_it = ctx.query_params.find("id");
            if (id_it != ctx.query_params.end()) {
                uint32_t del_id = std::stoul(id_it->second);
                auto lines = config::GetOptional<json>(lines_key).value_or(json::array());
                
                json new_lines = json::array();
                for (const auto& line : lines) {
                    if (!line.contains("id") || line["id"].get<uint32_t>() != del_id) {
                        new_lines.push_back(line);
                    }
                }
                
                config::Set<json>(lines_key, new_lines);
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
        spdlog::error("Error in line cross lines handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// Line Cross Counts Handler (statistics)
// ============================================================================
api::Response HandleLineCrossCounts(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method == "GET") {
            // Get current counts - this would come from analytics engine in real impl
            json j;
            j["total_in"] = 0;
            j["total_out"] = 0;
            j["last_reset"] = "";
            j["lines"] = json::array();
            
            // Get configured lines and add counts for each
            auto lines = config::GetOptional<json>("line_crossing.lines").value_or(json::array());
            for (const auto& line : lines) {
                json line_count;
                line_count["id"] = line.value("id", 0);
                line_count["name"] = line.value("name", "");
                line_count["in_count"] = 0;
                line_count["out_count"] = 0;
                j["lines"].push_back(line_count);
            }
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "POST") {
            // Reset counts
            auto body = json::parse(ctx.body);
            if (body.contains("action") && body["action"] == "reset") {
                // TODO: Actually reset counts in analytics engine
                spdlog::info("Line crossing counts reset requested");
                
                resp.status_code = 200;
                resp.body = R"({"success": true, "message": "Counts reset"})";
            } else {
                resp.status_code = 400;
                resp.body = R"({"error": "Invalid action"})";
            }
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in line cross counts handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }
    
    return resp;
}

// ============================================================================
// Line Cross Schedule Handler
// ============================================================================
api::Response HandleLineCrossSchedule(const api::RequestContext& ctx) {
    return HandleFeatureSchedule(ctx, features::LINE_CROSS);
}

// ============================================================================
// Line Cross Actions Handler
// ============================================================================
api::Response HandleLineCrossActions(const api::RequestContext& ctx) {
    return HandleFeatureActions(ctx, features::LINE_CROSS);
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
