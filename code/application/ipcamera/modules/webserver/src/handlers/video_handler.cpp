#include "video_handler.h"
#include <ipcam/video_control.h>
#include <ipcam/rtsp_server.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// Helper: Build video stream JSON
// ============================================================================
static json BuildVideoStreamJson(const media::VideoStreamConfig& cfg) {
    json j;
    j["stream_id"] = cfg.stream_id;
    j["enabled"] = cfg.enabled;
    j["audio_enabled"] = cfg.audio_enabled;
    j["stream_type"] = cfg.stream_type;
    j["width"] = cfg.width;
    j["height"] = cfg.height;
    j["fps"] = cfg.fps;
    j["codec"] = media::VideoCodecToString(cfg.codec);
    j["profile"] = media::H264ProfileToString(cfg.profile);
    j["level"] = cfg.level;
    j["gop"] = cfg.gop;
    j["idr_interval"] = cfg.idr_interval;
    j["iframe_interval"] = cfg.iframe_interval;
    j["rc_mode"] = media::RateControlModeToString(cfg.rc_mode);
    j["bitrate"] = cfg.bitrate;
    j["max_bitrate"] = cfg.max_bitrate;
    j["min_bitrate"] = cfg.min_bitrate;
    j["quality"] = cfg.quality;
    j["init_qp"] = cfg.init_qp;
    j["min_qp"] = cfg.min_qp;
    j["max_qp"] = cfg.max_qp;
    j["entropy_mode"] = media::EntropyModeToString(cfg.entropy_mode);
    return j;
}

// Map stream_id (video1/video2/video3) to RTSP channel index
static int StreamIdToChannel(const std::string& stream_id) {
    if (stream_id == "video1") return 0;
    if (stream_id == "video2") return 1;
    if (stream_id == "video3") return 2;
    return -1;
}

// Trigger RTSP stream refresh for given channel ids if server is running
static void RefreshRtspStreams(const std::vector<int>& channels) {
    auto& rtsp = streaming::GetRtspServer();
    if (!rtsp.IsRunning()) return;
    for (int ch : channels) {
        if (ch >= 0) rtsp.RefreshStream(ch);
    }
}

// ============================================================================
// Helper: Parse video stream from JSON
// ============================================================================
static media::VideoStreamConfig ParseVideoStreamJson(const json& body, const media::VideoStreamConfig& current) {
    media::VideoStreamConfig cfg = current;
    
    if (body.contains("enabled")) cfg.enabled = body["enabled"].get<bool>();
    if (body.contains("audio_enabled")) cfg.audio_enabled = body["audio_enabled"].get<bool>();
    if (body.contains("stream_type")) cfg.stream_type = body["stream_type"].get<std::string>();
    if (body.contains("width")) cfg.width = body["width"].get<int>();
    if (body.contains("height")) cfg.height = body["height"].get<int>();
    if (body.contains("fps")) cfg.fps = body["fps"].get<int>();
    if (body.contains("codec")) cfg.codec = media::StringToVideoCodec(body["codec"].get<std::string>());
    if (body.contains("profile")) cfg.profile = media::StringToH264Profile(body["profile"].get<std::string>());
    if (body.contains("level")) cfg.level = body["level"].get<std::string>();
    if (body.contains("gop")) cfg.gop = body["gop"].get<int>();
    if (body.contains("idr_interval")) cfg.idr_interval = body["idr_interval"].get<int>();
    if (body.contains("iframe_interval")) cfg.iframe_interval = body["iframe_interval"].get<int>();
    if (body.contains("rc_mode")) cfg.rc_mode = media::StringToRateControlMode(body["rc_mode"].get<std::string>());
    if (body.contains("bitrate")) cfg.bitrate = body["bitrate"].get<int>();
    if (body.contains("max_bitrate")) cfg.max_bitrate = body["max_bitrate"].get<int>();
    if (body.contains("min_bitrate")) cfg.min_bitrate = body["min_bitrate"].get<int>();
    if (body.contains("quality")) cfg.quality = body["quality"].get<int>();
    if (body.contains("init_qp")) cfg.init_qp = body["init_qp"].get<int>();
    if (body.contains("min_qp")) cfg.min_qp = body["min_qp"].get<int>();
    if (body.contains("max_qp")) cfg.max_qp = body["max_qp"].get<int>();
    if (body.contains("entropy_mode")) cfg.entropy_mode = media::StringToEntropyMode(body["entropy_mode"].get<std::string>());
    
    return cfg;
}

// ============================================================================
// Helper: Extract stream ID from URI
// ============================================================================
static std::string ExtractStreamId(const std::string& uri) {
    // Expected URIs: /api/v1/video/video1, /api/v1/video/video2, /api/v1/video/video3
    size_t lastSlash = uri.rfind('/');
    if (lastSlash != std::string::npos) {
        return uri.substr(lastSlash + 1);
    }
    return "";
}

// ============================================================================
// Main Video Endpoint
// ============================================================================
api::Response HandleVideo(const api::RequestContext& ctx) {
    api::Response resp;
    auto& video = media::VideoControl::Instance();

    try {
        if (ctx.method == "GET") {
            json j;
            j["streams"] = json::array();
            
            for (const auto& stream_id : video.GetStreamIds()) {
                auto cfg = video.GetStreamConfig(stream_id);
                j["streams"].push_back(BuildVideoStreamJson(cfg));
            }

            // Add snapshot config
            auto snap = video.GetSnapshotConfig();
            j["snapshot"]["enabled"] = snap.enabled;
            j["snapshot"]["width"] = snap.width;
            j["snapshot"]["height"] = snap.height;
            j["snapshot"]["quality"] = snap.quality;

            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            bool success = true;
            std::vector<int> refresh_channels;

            // Handle stream updates
            if (body.contains("streams") && body["streams"].is_array()) {
                for (const auto& stream : body["streams"]) {
                    if (!stream.contains("stream_id")) continue;
                    
                    std::string stream_id = stream["stream_id"].get<std::string>();
                    auto current = video.GetStreamConfig(stream_id);
                    auto updated = ParseVideoStreamJson(stream, current);
                    
                    if (!video.SetStreamConfig(stream_id, updated)) {
                        success = false;
                    } else {
                        refresh_channels.push_back(StreamIdToChannel(stream_id));
                    }
                }
            }

            // Handle individual stream fields (for simpler updates)
            for (const auto& stream_id : video.GetStreamIds()) {
                if (body.contains(stream_id)) {
                    auto current = video.GetStreamConfig(stream_id);
                    auto updated = ParseVideoStreamJson(body[stream_id], current);
                    
                    if (!video.SetStreamConfig(stream_id, updated)) {
                        success = false;
                    } else {
                        refresh_channels.push_back(StreamIdToChannel(stream_id));
                    }
                }
            }

            // Handle snapshot config
            if (body.contains("snapshot")) {
                auto snap = video.GetSnapshotConfig();
                auto& s = body["snapshot"];
                if (s.contains("enabled")) snap.enabled = s["enabled"].get<bool>();
                if (s.contains("width")) snap.width = s["width"].get<int>();
                if (s.contains("height")) snap.height = s["height"].get<int>();
                if (s.contains("quality")) snap.quality = s["quality"].get<int>();
                if (!video.SetSnapshotConfig(snap)) success = false;
            }

            if (success) {
                video.SaveToConfig();
                RefreshRtspStreams(refresh_channels);
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                result["message"] = "Video settings updated";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Failed to apply some settings";
                resp.body = err.dump(2);
            }
        }
        else {
            resp.status_code = 405;
            json err;
            err["error"] = "Method not allowed";
            resp.body = err.dump(2);
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleVideo error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Individual Stream Endpoint
// ============================================================================
api::Response HandleVideoStream(const api::RequestContext& ctx) {
    api::Response resp;
    auto& video = media::VideoControl::Instance();

    try {
        std::string stream_id = ExtractStreamId(ctx.uri);
        
        // Validate stream ID
        auto valid_ids = video.GetStreamIds();
        bool valid = false;
        for (const auto& id : valid_ids) {
            if (id == stream_id) {
                valid = true;
                break;
            }
        }
        
        if (!valid) {
            resp.status_code = 404;
            json err;
            err["error"] = "Stream not found";
            err["message"] = "Invalid stream ID: " + stream_id;
            resp.body = err.dump(2);
            return resp;
        }

        if (ctx.method == "GET") {
            auto cfg = video.GetStreamConfig(stream_id);
            json j = BuildVideoStreamJson(cfg);
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            auto current = video.GetStreamConfig(stream_id);
            auto updated = ParseVideoStreamJson(body, current);
            
            if (video.SetStreamConfig(stream_id, updated)) {
                video.SaveToConfig();
                RefreshRtspStreams({StreamIdToChannel(stream_id)});
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                result["message"] = "Stream " + stream_id + " updated";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Invalid stream configuration";
                resp.body = err.dump(2);
            }
        }
        else if (ctx.method == "DELETE") {
            // Disable the stream
            if (video.EnableStream(stream_id, false)) {
                video.SaveToConfig();
                RefreshRtspStreams({StreamIdToChannel(stream_id)});
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                result["message"] = "Stream " + stream_id + " disabled";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 500;
                json err;
                err["status"] = "error";
                err["message"] = "Failed to disable stream";
                resp.body = err.dump(2);
            }
        }
        else {
            resp.status_code = 405;
            json err;
            err["error"] = "Method not allowed";
            resp.body = err.dump(2);
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleVideoStream error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Video Capabilities Endpoint
// ============================================================================
api::Response HandleVideoCapabilities(const api::RequestContext& ctx) {
    api::Response resp;
    auto& video = media::VideoControl::Instance();

    if (ctx.method != "GET") {
        resp.status_code = 405;
        json err;
        err["error"] = "Method not allowed";
        resp.body = err.dump(2);
        return resp;
    }

    try {
        auto caps = video.GetCapabilities();
        json j;

        // Codecs
        j["codecs"] = json::array();
        for (const auto& codec : caps.supported_codecs) {
            j["codecs"].push_back(media::VideoCodecToString(codec));
        }

        // Resolutions
        j["resolutions"] = json::array();
        for (const auto& [w, h] : caps.supported_resolutions) {
            json res;
            res["width"] = w;
            res["height"] = h;
            res["label"] = std::to_string(w) + "x" + std::to_string(h);
            j["resolutions"].push_back(res);
        }

        // Profiles
        j["h264_profiles"] = json::array();
        for (const auto& profile : caps.supported_h264_profiles) {
            j["h264_profiles"].push_back(media::H264ProfileToString(profile));
        }

        j["h265_profiles"] = json::array();
        for (const auto& profile : caps.supported_h265_profiles) {
            j["h265_profiles"].push_back(media::H265ProfileToString(profile));
        }

        // Levels
        j["levels"] = caps.supported_levels;

        // Rate control modes
        j["rc_modes"] = json::array();
        for (const auto& mode : caps.supported_rc_modes) {
            j["rc_modes"].push_back(media::RateControlModeToString(mode));
        }

        // Limits
        j["limits"]["fps"]["min"] = caps.min_fps;
        j["limits"]["fps"]["max"] = caps.max_fps;
        j["limits"]["bitrate"]["min"] = caps.min_bitrate;
        j["limits"]["bitrate"]["max"] = caps.max_bitrate;
        j["limits"]["gop"]["min"] = caps.min_gop;
        j["limits"]["gop"]["max"] = caps.max_gop;
        j["limits"]["qp"]["min"] = 0;
        j["limits"]["qp"]["max"] = 51;

        resp.status_code = 200;
        resp.body = j.dump(2);
    }
    catch (const std::exception& e) {
        spdlog::error("HandleVideoCapabilities error: {}", e.what());
        resp.status_code = 500;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Snapshot Endpoint
// ============================================================================
api::Response HandleSnapshot(const api::RequestContext& ctx) {
    api::Response resp;

    if (ctx.method != "GET") {
        resp.status_code = 405;
        json err;
        err["error"] = "Method not allowed";
        resp.body = err.dump(2);
        return resp;
    }

    try {
        auto& video = media::VideoControl::Instance();
        auto data = video.CaptureSnapshot();

        if (!data.empty()) {
            resp.status_code = 200;
            resp.content_type = "image/jpeg";
            resp.body = std::string(data.begin(), data.end());
        } else {
            // Return placeholder message until HDAL is integrated
            resp.status_code = 503;
            json err;
            err["status"] = "error";
            err["message"] = "Snapshot capture not implemented - HDAL integration pending";
            resp.body = err.dump(2);
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleSnapshot error: {}", e.what());
        resp.status_code = 500;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Snapshot Config Endpoint
// ============================================================================
api::Response HandleSnapshotConfig(const api::RequestContext& ctx) {
    api::Response resp;
    auto& video = media::VideoControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto snap = video.GetSnapshotConfig();
            json j;
            j["enabled"] = snap.enabled;
            j["width"] = snap.width;
            j["height"] = snap.height;
            j["quality"] = snap.quality;
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            auto snap = video.GetSnapshotConfig();

            if (body.contains("enabled")) snap.enabled = body["enabled"].get<bool>();
            if (body.contains("width")) snap.width = body["width"].get<int>();
            if (body.contains("height")) snap.height = body["height"].get<int>();
            if (body.contains("quality")) snap.quality = body["quality"].get<int>();

            if (video.SetSnapshotConfig(snap)) {
                video.SaveToConfig();
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Invalid snapshot configuration";
                resp.body = err.dump(2);
            }
        }
        else {
            resp.status_code = 405;
            json err;
            err["error"] = "Method not allowed";
            resp.body = err.dump(2);
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleSnapshotConfig error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Video Reset Endpoint
// ============================================================================
api::Response HandleVideoReset(const api::RequestContext& ctx) {
    api::Response resp;

    if (ctx.method != "POST") {
        resp.status_code = 405;
        json err;
        err["error"] = "Method not allowed";
        resp.body = err.dump(2);
        return resp;
    }

    auto& video = media::VideoControl::Instance();
    
    if (video.ResetToDefaults()) {
        resp.status_code = 200;
        json result;
        result["status"] = "success";
        result["message"] = "Video settings reset to defaults";
        resp.body = result.dump(2);
    } else {
        resp.status_code = 500;
        json err;
        err["status"] = "error";
        err["message"] = "Failed to reset video settings";
        resp.body = err.dump(2);
    }

    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
