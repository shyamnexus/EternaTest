/**
 * @file face_handler.cpp
 * @brief Face detection and recognition API handlers
 */

#include "face_handler.h"
#include <ipcam/analytics.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

api::Response HandleFaceDetection(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"] = config::Get<bool>("face_detection.enabled", false);
            j["model"] = config::Get<std::string>("face_detection.model", "scrfd_nosig");
            j["min_face_size"] = config::Get<int>("face_detection.min_face_size", 30);
            j["confidence_threshold"] = round_json(config::Get<float>("face_detection.confidence_threshold", 0.6f));
            j["recognition"] = {
                {"enabled", config::Get<bool>("face_detection.recognition.enabled", false)},
                {"database_path", config::Get<std::string>("face_detection.recognition.database_path", "/mnt/app/face_db")},
                {"similarity_threshold", round_json(config::Get<float>("face_detection.recognition.similarity_threshold", 0.7f))},
                {"max_gallery_size", config::Get<int>("face_detection.recognition.max_gallery_size", 1000)},
                {"margin", round_json(config::Get<float>("face_detection.recognition.margin", 0.2f))}
            };
            j["attributes"] = {
                {"enabled", config::Get<bool>("face_attribute.enabled", false)},
                {"margin", round_json(config::Get<float>("face_attribute.margin", 0.2f))}
            };

            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto& analytics = ai::AnalyticsEngine::Instance();

            if (body.contains("enabled"))
                config::Set<bool>("face_detection.enabled", body["enabled"].get<bool>());
            if (body.contains("min_face_size"))
                config::Set<int>("face_detection.min_face_size", body["min_face_size"].get<int>());
            if (body.contains("confidence_threshold"))
                config::Set<float>("face_detection.confidence_threshold", body["confidence_threshold"].get<float>());

            if (body.contains("recognition")) {
                auto& r = body["recognition"];
                if (r.contains("enabled"))
                    config::Set<bool>("face_detection.recognition.enabled", r["enabled"].get<bool>());
                if (r.contains("similarity_threshold"))
                    config::Set<float>("face_detection.recognition.similarity_threshold", r["similarity_threshold"].get<float>());
                if (r.contains("max_gallery_size"))
                    config::Set<int>("face_detection.recognition.max_gallery_size", r["max_gallery_size"].get<int>());
                if (r.contains("margin"))
                    config::Set<float>("face_detection.recognition.margin", r["margin"].get<float>());
            }

            if (body.contains("attributes")) {
                auto& a = body["attributes"];
                if (a.contains("enabled"))
                    config::Set<bool>("face_attribute.enabled", a["enabled"].get<bool>());
                if (a.contains("margin"))
                    config::Set<float>("face_attribute.margin", a["margin"].get<float>());
            }

            config::Save();

            auto acfg = analytics.GetConfig();
            acfg.face_detection.enabled = config::Get<bool>("face_detection.enabled", false);
            acfg.face_attribute.enabled = config::Get<bool>("face_attribute.enabled", false);
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
        spdlog::error("Error in face detection handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

api::Response HandleFaceGallery(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method == "GET") {
            json j;
            j["gallery"] = json::array();
            j["max_size"] = config::Get<int>("face_detection.recognition.max_gallery_size", 1000);
            j["database_path"] = config::Get<std::string>("face_detection.recognition.database_path", "/mnt/app/face_db");

            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error in face gallery handler: {}", e.what());
        resp.status_code = 500;
        resp.body = std::string(R"({"error": ")") + e.what() + "\"}";
    }

    return resp;
}

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
