/**
 * @file heatmap_handler.cpp
 * @brief Heat map analytics API handlers
 */

#include "heatmap_handler.h"
#include <ipcam/analytics.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

api::Response HandleHeatMap(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("heat_map.enabled", false);
            j["grid_w"] = config::Get<int>("heat_map.grid_w", 32);
            j["grid_h"] = config::Get<int>("heat_map.grid_h", 18);
            j["decay_rate"] = round_json(config::Get<float>("heat_map.decay_rate", 0.97f));

            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto& analytics = ai::AnalyticsEngine::Instance();

            if (body.contains("enabled"))
                config::Set<bool>("heat_map.enabled", body["enabled"].get<bool>());
            if (body.contains("grid_w"))
                config::Set<int>("heat_map.grid_w", body["grid_w"].get<int>());
            if (body.contains("grid_h"))
                config::Set<int>("heat_map.grid_h", body["grid_h"].get<int>());
            if (body.contains("decay_rate"))
                config::Set<float>("heat_map.decay_rate", body["decay_rate"].get<float>());

            config::Save();

            auto acfg = analytics.GetConfig();
            acfg.heat_map.enabled = config::Get<bool>("heat_map.enabled", false);
            acfg.heat_map.grid_w = config::Get<int>("heat_map.grid_w", 32);
            acfg.heat_map.grid_h = config::Get<int>("heat_map.grid_h", 18);
            acfg.heat_map.decay_rate = config::Get<float>("heat_map.decay_rate", 0.97f);
            analytics.SetConfig(acfg);

            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in heat map handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

api::Response HandleHeatMapGrid(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method == "GET") {
            auto& analytics = ai::AnalyticsEngine::Instance();
            auto grid = analytics.GetHeatMapGrid();
            int w = analytics.GetHeatMapGridW();
            int h = analytics.GetHeatMapGridH();

            json j;
            j["grid_w"] = w;
            j["grid_h"] = h;

            // Flatten grid to JSON array of arrays
            json rows = json::array();
            for (int y = 0; y < h && y * w < static_cast<int>(grid.size()); ++y) {
                json row = json::array();
                for (int x = 0; x < w; ++x) {
                    row.push_back(grid[y * w + x]);
                }
                rows.push_back(row);
            }
            j["data"] = rows;

            resp.status_code = 200;
            resp.body = j.dump();
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in heat map grid handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

api::Response HandleHeatMapReset(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method == "POST") {
            auto& analytics = ai::AnalyticsEngine::Instance();
            analytics.ResetHeatMap();

            resp.status_code = 200;
            resp.body = R"({"success": true})";
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in heat map reset handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
