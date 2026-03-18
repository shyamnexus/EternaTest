/**
 * @file audio_handler.cpp
 * @brief Audio classification API handlers
 */

#include "audio_handler.h"
#include <ipcam/analytics.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

api::Response HandleAudioClassification(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("audio_classification.enabled", false);
            j["model"] = config::Get<std::string>("audio_classification.model", "yamnet");
            j["detection_threshold"] = round_json(config::Get<float>("audio_classification.detection_threshold", 0.3f));

            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto& analytics = ai::AnalyticsEngine::Instance();

            if (body.contains("enabled"))
                config::Set<bool>("audio_classification.enabled", body["enabled"].get<bool>());
            if (body.contains("detection_threshold"))
                config::Set<float>("audio_classification.detection_threshold", body["detection_threshold"].get<float>());

            config::Save();

            auto acfg = analytics.GetConfig();
            acfg.audio_classification.enabled = config::Get<bool>("audio_classification.enabled", false);
            acfg.audio_classification.detection_threshold = config::Get<float>("audio_classification.detection_threshold", 0.3f);
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
        spdlog::error("Error in audio classification handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
