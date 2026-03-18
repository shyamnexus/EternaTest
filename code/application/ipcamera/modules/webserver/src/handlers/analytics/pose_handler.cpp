/**
 * @file pose_handler.cpp
 * @brief Pose estimation API handlers
 */

#include "pose_handler.h"
#include <ipcam/analytics.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

api::Response HandlePoseEstimation(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("pose_estimation.enabled", false);
            j["model"] = config::Get<std::string>("pose_estimation.model", "yolo26n_pose");
            j["conf_threshold"] = round_json(config::Get<float>("pose_estimation.conf_threshold", 0.35f));
            j["nms_threshold"] = round_json(config::Get<float>("pose_estimation.nms_threshold", 0.50f));
            j["max_persons"] = config::Get<int>("pose_estimation.max_persons", 16);

            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto& analytics = ai::AnalyticsEngine::Instance();

            if (body.contains("enabled"))
                config::Set<bool>("pose_estimation.enabled", body["enabled"].get<bool>());
            if (body.contains("conf_threshold"))
                config::Set<float>("pose_estimation.conf_threshold", body["conf_threshold"].get<float>());
            if (body.contains("nms_threshold"))
                config::Set<float>("pose_estimation.nms_threshold", body["nms_threshold"].get<float>());
            if (body.contains("max_persons"))
                config::Set<int>("pose_estimation.max_persons", body["max_persons"].get<int>());

            config::Save();

            auto acfg = analytics.GetConfig();
            acfg.pose_estimation.enabled = config::Get<bool>("pose_estimation.enabled", false);
            acfg.pose_estimation.conf_threshold = config::Get<float>("pose_estimation.conf_threshold", 0.35f);
            acfg.pose_estimation.nms_threshold = config::Get<float>("pose_estimation.nms_threshold", 0.50f);
            acfg.pose_estimation.max_persons = config::Get<int>("pose_estimation.max_persons", 16);
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
        spdlog::error("Error in pose estimation handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
