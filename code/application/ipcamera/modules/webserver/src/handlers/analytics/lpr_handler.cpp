/**
 * @file lpr_handler.cpp
 * @brief License plate recognition API handlers
 */

#include "lpr_handler.h"
#include <ipcam/analytics.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

api::Response HandleLpr(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("license_plate.enabled", false);
            j["model"] = config::Get<std::string>("license_plate.model", "lpr_nvt");
            j["regions"] = config::Get<json>("license_plate.regions", json::array({"us", "eu"}));
            j["min_confidence"] = round_json(config::Get<float>("license_plate.min_confidence", 0.7f));
            j["actions"] = BuildActionsJson("license_plate");

            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto& analytics = ai::AnalyticsEngine::Instance();

            if (body.contains("enabled"))
                config::Set<bool>("license_plate.enabled", body["enabled"].get<bool>());
            if (body.contains("min_confidence"))
                config::Set<float>("license_plate.min_confidence", body["min_confidence"].get<float>());
            if (body.contains("regions"))
                config::Set<json>("license_plate.regions", body["regions"]);

            if (body.contains("actions"))
                UpdateActionsFromJson("license_plate", body["actions"]);

            config::Save();

            auto acfg = analytics.GetConfig();
            acfg.license_plate.enabled = config::Get<bool>("license_plate.enabled", false);
            acfg.license_plate.min_confidence = config::Get<float>("license_plate.min_confidence", 0.7f);
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
        spdlog::error("Error in LPR handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
