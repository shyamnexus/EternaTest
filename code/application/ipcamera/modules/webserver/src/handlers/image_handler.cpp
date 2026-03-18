#include "image_handler.h"
#include <ipcam/isp_control.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// Helper: Build full image settings JSON
// ============================================================================
static json BuildImageSettingsJson() {
    auto& isp = platform::ISPControl::Instance();
    json j;

    // Adjustment
    auto adj = isp.GetAdjustment();
    j["adjustment"]["brightness"] = adj.brightness;
    j["adjustment"]["contrast"] = adj.contrast;
    j["adjustment"]["saturation"] = adj.saturation;
    j["adjustment"]["sharpness"] = adj.sharpness;
    j["adjustment"]["hue"] = adj.hue;
    j["adjustment"]["gamma"] = adj.gamma;

    // White Balance
    auto wb = isp.GetWhiteBalance();
    j["white_balance"]["mode"] = platform::WBModeToString(wb.mode);
    j["white_balance"]["preset"] = platform::WBPresetToString(wb.preset);
    j["white_balance"]["color_temperature"] = wb.color_temperature;
    j["white_balance"]["r_gain"] = wb.r_gain;
    j["white_balance"]["g_gain"] = wb.g_gain;
    j["white_balance"]["b_gain"] = wb.b_gain;

    // Orientation
    auto orient = isp.GetOrientation();
    j["orientation"]["mirror"] = orient.mirror;
    j["orientation"]["flip"] = orient.flip;
    j["orientation"]["rotation"] = orient.rotation;

    // Anti-flicker
    j["anti_flicker"]["mode"] = platform::FlickerModeToString(isp.GetFlickerMode());

    // Exposure
    auto exp = isp.GetExposure();
    j["exposure"]["mode"] = platform::ExposureModeToString(exp.mode);
    j["exposure"]["metering_mode"] = platform::MeteringModeToString(exp.metering);
    j["exposure"]["auto_gain_enabled"] = exp.auto_gain_enabled;
    j["exposure"]["ev_compensation"] = exp.ev_compensation;
    j["exposure"]["max_gain"] = exp.max_gain;
    j["exposure"]["min_gain"] = exp.min_gain;

    // Day/Night
    auto dn = isp.GetDayNight();
    j["day_night"]["mode"] = platform::DayNightModeToString(dn.mode);
    j["day_night"]["ir_mode"] = platform::IRModeToString(dn.ir_mode);
    j["day_night"]["sensitivity"] = dn.sensitivity;
    j["day_night"]["start_time"] = dn.start_time;
    j["day_night"]["end_time"] = dn.end_time;

    // BLC
    auto blc = isp.GetBLC();
    j["blc"]["blc_enabled"] = blc.blc_enabled;
    j["blc"]["blc_level"] = blc.blc_level;
    j["blc"]["wdr_enabled"] = blc.wdr_enabled;
    j["blc"]["wdr_mode"] = WdrModeToString(blc.wdr_mode);
    j["blc"]["wdr_level"] = blc.wdr_level;
    j["blc"]["wdr_color_protect"] = blc.wdr_color_protect;
    j["blc"]["hdr_enabled"] = blc.hdr_enabled;
    j["blc"]["hdr_mode"] = HdrModeToString(blc.hdr_mode);
    j["blc"]["hdr_level"] = blc.hdr_level;
    j["blc"]["hlc_enabled"] = blc.hlc_enabled;
    j["blc"]["hlc_level"] = blc.hlc_level;
    j["blc"]["dark_boost_level"] = blc.dark_boost_level;

    // Enhancement
    auto enh = isp.GetEnhancement();
    j["enhancement"]["scene_preset"] = ScenePresetToString(enh.scene_preset);
    j["enhancement"]["nr_2d_enabled"] = enh.nr_2d_enabled;
    j["enhancement"]["nr_2d_mode"] = NrModeToString(enh.nr_2d_mode);
    j["enhancement"]["nr_2d_level"] = enh.nr_2d_level;
    j["enhancement"]["nr_3d_enabled"] = enh.nr_3d_enabled;
    j["enhancement"]["nr_3d_mode"] = NrModeToString(enh.nr_3d_mode);
    j["enhancement"]["nr_3d_level"] = enh.nr_3d_level;
    j["enhancement"]["color_nr_enabled"] = enh.color_nr_enabled;
    j["enhancement"]["color_nr_level"] = enh.color_nr_level;
    j["enhancement"]["defog_enabled"] = enh.defog_enabled;
    j["enhancement"]["defog_mode"] = DefogModeToString(enh.defog_mode);
    j["enhancement"]["defog_level"] = enh.defog_level;
    j["enhancement"]["edge_enhance_enabled"] = enh.edge_enhance_enabled;
    j["enhancement"]["edge_enhance_level"] = enh.edge_enhance_level;
    j["enhancement"]["edge_coring"] = enh.edge_coring;

    return j;
}

// ============================================================================
// Main Image Endpoint
// ============================================================================
api::Response HandleImage(const api::RequestContext& ctx) {
    api::Response resp;
    auto& isp = platform::ISPControl::Instance();

    try {
        if (ctx.method == "GET") {
            json j = BuildImageSettingsJson();
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            bool success = true;

            // Handle adjustment updates
            if (body.contains("adjustment")) {
                auto adj = isp.GetAdjustment();
                auto& a = body["adjustment"];
                if (a.contains("brightness")) adj.brightness = a["brightness"].get<int>();
                if (a.contains("contrast")) adj.contrast = a["contrast"].get<int>();
                if (a.contains("saturation")) adj.saturation = a["saturation"].get<int>();
                if (a.contains("sharpness")) adj.sharpness = a["sharpness"].get<int>();
                if (a.contains("hue")) adj.hue = a["hue"].get<int>();
                if (a.contains("gamma")) adj.gamma = a["gamma"].get<int>();
                if (!isp.SetAdjustment(adj)) success = false;
            }

            // Handle white balance updates
            if (body.contains("white_balance")) {
                auto wb = isp.GetWhiteBalance();
                auto& w = body["white_balance"];
                if (w.contains("mode")) wb.mode = platform::StringToWBMode(w["mode"].get<std::string>());
                if (w.contains("preset")) wb.preset = platform::StringToWBPreset(w["preset"].get<std::string>());
                if (w.contains("color_temperature")) wb.color_temperature = w["color_temperature"].get<int>();
                if (w.contains("r_gain")) wb.r_gain = w["r_gain"].get<int>();
                if (w.contains("g_gain")) wb.g_gain = w["g_gain"].get<int>();
                if (w.contains("b_gain")) wb.b_gain = w["b_gain"].get<int>();
                if (!isp.SetWhiteBalance(wb)) success = false;
            }

            // Handle orientation updates
            if (body.contains("orientation")) {
                auto orient = isp.GetOrientation();
                auto& o = body["orientation"];
                if (o.contains("mirror")) orient.mirror = o["mirror"].get<bool>();
                if (o.contains("flip")) orient.flip = o["flip"].get<bool>();
                if (o.contains("rotation")) orient.rotation = o["rotation"].get<int>();
                if (!isp.SetOrientation(orient)) success = false;
            }

            // Handle anti-flicker updates
            if (body.contains("anti_flicker")) {
                auto& af = body["anti_flicker"];
                if (af.contains("mode")) {
                    auto mode = platform::StringToFlickerMode(af["mode"].get<std::string>());
                    if (!isp.SetFlickerMode(mode)) success = false;
                }
            }

            // Handle exposure updates
            if (body.contains("exposure")) {
                auto exp = isp.GetExposure();
                auto& e = body["exposure"];
                if (e.contains("mode")) exp.mode = platform::StringToExposureMode(e["mode"].get<std::string>());
                if (e.contains("metering_mode")) exp.metering = platform::StringToMeteringMode(e["metering_mode"].get<std::string>());
                if (e.contains("auto_gain_enabled")) exp.auto_gain_enabled = e["auto_gain_enabled"].get<bool>();
                if (e.contains("ev_compensation")) exp.ev_compensation = e["ev_compensation"].get<int>();
                if (e.contains("max_gain")) exp.max_gain = e["max_gain"].get<int>();
                if (e.contains("min_gain")) exp.min_gain = e["min_gain"].get<int>();
                if (!isp.SetExposure(exp)) success = false;
            }

            // Handle day/night updates
            if (body.contains("day_night")) {
                auto dn = isp.GetDayNight();
                auto& d = body["day_night"];
                if (d.contains("mode")) dn.mode = platform::StringToDayNightMode(d["mode"].get<std::string>());
                if (d.contains("ir_mode")) dn.ir_mode = platform::StringToIRMode(d["ir_mode"].get<std::string>());
                if (d.contains("sensitivity")) dn.sensitivity = d["sensitivity"].get<int>();
                if (d.contains("start_time")) dn.start_time = d["start_time"].get<std::string>();
                if (d.contains("end_time")) dn.end_time = d["end_time"].get<std::string>();
                if (!isp.SetDayNight(dn)) success = false;
            }

            // Handle BLC updates
            if (body.contains("blc")) {
                auto blc = isp.GetBLC();
                auto& b = body["blc"];
                if (b.contains("blc_enabled")) blc.blc_enabled = b["blc_enabled"].get<bool>();
                if (b.contains("blc_level")) blc.blc_level = b["blc_level"].get<int>();
                if (b.contains("wdr_enabled")) blc.wdr_enabled = b["wdr_enabled"].get<bool>();
                if (b.contains("wdr_level")) blc.wdr_level = b["wdr_level"].get<int>();
                if (b.contains("wdr_mode")) blc.wdr_mode = platform::StringToWdrMode(b["wdr_mode"].get<std::string>());
                if (b.contains("wdr_color_protect")) blc.wdr_color_protect = b["wdr_color_protect"].get<int>();
                if (b.contains("hdr_enabled")) blc.hdr_enabled = b["hdr_enabled"].get<bool>();
                if (b.contains("hdr_level")) blc.hdr_level = b["hdr_level"].get<int>();
                if (b.contains("hdr_mode")) blc.hdr_mode = platform::StringToHdrMode(b["hdr_mode"].get<std::string>());
                if (b.contains("hlc_enabled")) blc.hlc_enabled = b["hlc_enabled"].get<bool>();
                if (b.contains("hlc_level")) blc.hlc_level = b["hlc_level"].get<int>();
                if (!isp.SetBLC(blc)) success = false;
            }

            // Handle enhancement updates
            if (body.contains("enhancement")) {
                auto enh = isp.GetEnhancement();
                auto& en = body["enhancement"];
                if (en.contains("scene_preset")) enh.scene_preset = platform::StringToScenePreset(en["scene_preset"].get<std::string>());
                if (en.contains("nr_2d_enabled")) enh.nr_2d_enabled = en["nr_2d_enabled"].get<bool>();
                if (en.contains("nr_2d_level")) enh.nr_2d_level = en["nr_2d_level"].get<int>();
                if (en.contains("nr_2d_mode")) enh.nr_2d_mode = platform::StringToNrMode(en["nr_2d_mode"].get<std::string>());
                if (en.contains("nr_3d_enabled")) enh.nr_3d_enabled = en["nr_3d_enabled"].get<bool>();
                if (en.contains("nr_3d_level")) enh.nr_3d_level = en["nr_3d_level"].get<int>();
                if (en.contains("nr_3d_mode")) enh.nr_3d_mode = platform::StringToNrMode(en["nr_3d_mode"].get<std::string>());
                if (en.contains("color_nr_enabled")) enh.color_nr_enabled = en["color_nr_enabled"].get<bool>();
                if (en.contains("color_nr_level")) enh.color_nr_level = en["color_nr_level"].get<int>();
                if (en.contains("defog_enabled")) enh.defog_enabled = en["defog_enabled"].get<bool>();
                if (en.contains("defog_level")) enh.defog_level = en["defog_level"].get<int>();
                if (en.contains("defog_mode")) enh.defog_mode = platform::StringToDefogMode(en["defog_mode"].get<std::string>());
                if (en.contains("edge_enhance_enabled")) enh.edge_enhance_enabled = en["edge_enhance_enabled"].get<bool>();
                if (en.contains("edge_enhance_level")) enh.edge_enhance_level = en["edge_enhance_level"].get<int>();
                if (en.contains("edge_coring")) enh.edge_coring = en["edge_coring"].get<int>();
                if (!isp.SetEnhancement(enh)) success = false;
            }

            if (success) {
                isp.SaveToConfig();
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                result["message"] = "Image settings updated";
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
        spdlog::error("HandleImage error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Image Adjustment Endpoint
// ============================================================================
api::Response HandleImageAdjustment(const api::RequestContext& ctx) {
    api::Response resp;
    auto& isp = platform::ISPControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto adj = isp.GetAdjustment();
            json j;
            j["brightness"] = adj.brightness;
            j["contrast"] = adj.contrast;
            j["saturation"] = adj.saturation;
            j["sharpness"] = adj.sharpness;
            j["hue"] = adj.hue;
            j["gamma"] = adj.gamma;
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            auto adj = isp.GetAdjustment();

            if (body.contains("brightness")) adj.brightness = body["brightness"].get<int>();
            if (body.contains("contrast")) adj.contrast = body["contrast"].get<int>();
            if (body.contains("saturation")) adj.saturation = body["saturation"].get<int>();
            if (body.contains("sharpness")) adj.sharpness = body["sharpness"].get<int>();
            if (body.contains("hue")) adj.hue = body["hue"].get<int>();
            if (body.contains("gamma")) adj.gamma = body["gamma"].get<int>();

            if (isp.SetAdjustment(adj)) {
                isp.SaveToConfig();
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Invalid adjustment values";
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
        spdlog::error("HandleImageAdjustment error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// White Balance Endpoint
// ============================================================================
api::Response HandleImageWhiteBalance(const api::RequestContext& ctx) {
    api::Response resp;
    auto& isp = platform::ISPControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto wb = isp.GetWhiteBalance();
            json j;
            j["mode"] = platform::WBModeToString(wb.mode);
            j["preset"] = platform::WBPresetToString(wb.preset);
            j["color_temperature"] = wb.color_temperature;
            j["r_gain"] = wb.r_gain;
            j["g_gain"] = wb.g_gain;
            j["b_gain"] = wb.b_gain;
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            auto wb = isp.GetWhiteBalance();

            if (body.contains("mode")) wb.mode = platform::StringToWBMode(body["mode"].get<std::string>());
            if (body.contains("preset")) wb.preset = platform::StringToWBPreset(body["preset"].get<std::string>());
            if (body.contains("color_temperature")) wb.color_temperature = body["color_temperature"].get<int>();
            if (body.contains("r_gain")) wb.r_gain = body["r_gain"].get<int>();
            if (body.contains("g_gain")) wb.g_gain = body["g_gain"].get<int>();
            if (body.contains("b_gain")) wb.b_gain = body["b_gain"].get<int>();

            if (isp.SetWhiteBalance(wb)) {
                isp.SaveToConfig();
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Invalid white balance values";
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
        spdlog::error("HandleImageWhiteBalance error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Orientation Endpoint
// ============================================================================
api::Response HandleImageOrientation(const api::RequestContext& ctx) {
    api::Response resp;
    auto& isp = platform::ISPControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto orient = isp.GetOrientation();
            json j;
            j["mirror"] = orient.mirror;
            j["flip"] = orient.flip;
            j["rotation"] = orient.rotation;
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            auto orient = isp.GetOrientation();

            if (body.contains("mirror")) orient.mirror = body["mirror"].get<bool>();
            if (body.contains("flip")) orient.flip = body["flip"].get<bool>();
            if (body.contains("rotation")) orient.rotation = body["rotation"].get<int>();

            if (isp.SetOrientation(orient)) {
                isp.SaveToConfig();
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Invalid orientation values (rotation must be 0, 90, 180, or 270)";
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
        spdlog::error("HandleImageOrientation error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Anti-Flicker Endpoint
// ============================================================================
api::Response HandleImageAntiFlicker(const api::RequestContext& ctx) {
    api::Response resp;
    auto& isp = platform::ISPControl::Instance();

    try {
        if (ctx.method == "GET") {
            json j;
            j["mode"] = platform::FlickerModeToString(isp.GetFlickerMode());
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            
            if (body.contains("mode")) {
                auto mode = platform::StringToFlickerMode(body["mode"].get<std::string>());
                if (isp.SetFlickerMode(mode)) {
                    isp.SaveToConfig();
                    resp.status_code = 200;
                    json result;
                    result["status"] = "success";
                    resp.body = result.dump(2);
                } else {
                    resp.status_code = 400;
                    json err;
                    err["status"] = "error";
                    err["message"] = "Failed to set anti-flicker mode";
                    resp.body = err.dump(2);
                }
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Missing 'mode' field";
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
        spdlog::error("HandleImageAntiFlicker error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Exposure Endpoint
// ============================================================================
api::Response HandleImageExposure(const api::RequestContext& ctx) {
    api::Response resp;
    auto& isp = platform::ISPControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto exp = isp.GetExposure();
            json j;
            j["mode"] = platform::ExposureModeToString(exp.mode);
            j["metering_mode"] = platform::MeteringModeToString(exp.metering);
            j["auto_gain_enabled"] = exp.auto_gain_enabled;
            j["ev_compensation"] = exp.ev_compensation;
            j["max_gain"] = exp.max_gain;
            j["min_gain"] = exp.min_gain;
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            auto exp = isp.GetExposure();

            if (body.contains("mode")) exp.mode = platform::StringToExposureMode(body["mode"].get<std::string>());
            if (body.contains("metering_mode")) exp.metering = platform::StringToMeteringMode(body["metering_mode"].get<std::string>());
            if (body.contains("auto_gain_enabled")) exp.auto_gain_enabled = body["auto_gain_enabled"].get<bool>();
            if (body.contains("ev_compensation")) exp.ev_compensation = body["ev_compensation"].get<int>();
            if (body.contains("max_gain")) exp.max_gain = body["max_gain"].get<int>();
            if (body.contains("min_gain")) exp.min_gain = body["min_gain"].get<int>();

            if (isp.SetExposure(exp)) {
                isp.SaveToConfig();
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Invalid exposure values";
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
        spdlog::error("HandleImageExposure error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Day/Night Endpoint
// ============================================================================
api::Response HandleImageDayNight(const api::RequestContext& ctx) {
    api::Response resp;
    auto& isp = platform::ISPControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto dn = isp.GetDayNight();
            json j;
            j["mode"] = platform::DayNightModeToString(dn.mode);
            j["ir_mode"] = platform::IRModeToString(dn.ir_mode);
            j["sensitivity"] = dn.sensitivity;
            j["start_time"] = dn.start_time;
            j["end_time"] = dn.end_time;
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            auto dn = isp.GetDayNight();

            if (body.contains("mode")) dn.mode = platform::StringToDayNightMode(body["mode"].get<std::string>());
            if (body.contains("ir_mode")) dn.ir_mode = platform::StringToIRMode(body["ir_mode"].get<std::string>());
            if (body.contains("sensitivity")) dn.sensitivity = body["sensitivity"].get<int>();
            if (body.contains("start_time")) dn.start_time = body["start_time"].get<std::string>();
            if (body.contains("end_time")) dn.end_time = body["end_time"].get<std::string>();

            if (isp.SetDayNight(dn)) {
                isp.SaveToConfig();
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Invalid day/night values";
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
        spdlog::error("HandleImageDayNight error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// BLC Endpoint
// ============================================================================
api::Response HandleImageBLC(const api::RequestContext& ctx) {
    api::Response resp;
    auto& isp = platform::ISPControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto blc = isp.GetBLC();
            json j;
            j["blc_enabled"] = blc.blc_enabled;
            j["blc_level"] = blc.blc_level;
            j["wdr_enabled"] = blc.wdr_enabled;
            j["wdr_level"] = blc.wdr_level;
            j["wdr_mode"] = platform::WdrModeToString(blc.wdr_mode);
            j["wdr_color_protect"] = blc.wdr_color_protect;
            j["hdr_enabled"] = blc.hdr_enabled;
            j["hdr_level"] = blc.hdr_level;
            j["hdr_mode"] = platform::HdrModeToString(blc.hdr_mode);
            j["hlc_enabled"] = blc.hlc_enabled;
            j["hlc_level"] = blc.hlc_level;
            j["dark_boost_level"] = blc.dark_boost_level;
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            auto blc = isp.GetBLC();

            if (body.contains("blc_enabled")) blc.blc_enabled = body["blc_enabled"].get<bool>();
            if (body.contains("blc_level")) blc.blc_level = body["blc_level"].get<int>();
            if (body.contains("wdr_enabled")) blc.wdr_enabled = body["wdr_enabled"].get<bool>();
            if (body.contains("wdr_level")) blc.wdr_level = body["wdr_level"].get<int>();
            if (body.contains("wdr_mode")) blc.wdr_mode = platform::StringToWdrMode(body["wdr_mode"].get<std::string>());
            if (body.contains("wdr_color_protect")) blc.wdr_color_protect = body["wdr_color_protect"].get<int>();
            if (body.contains("hdr_enabled")) blc.hdr_enabled = body["hdr_enabled"].get<bool>();
            if (body.contains("hdr_level")) blc.hdr_level = body["hdr_level"].get<int>();
            if (body.contains("hdr_mode")) blc.hdr_mode = platform::StringToHdrMode(body["hdr_mode"].get<std::string>());
            if (body.contains("hlc_enabled")) blc.hlc_enabled = body["hlc_enabled"].get<bool>();
            if (body.contains("hlc_level")) blc.hlc_level = body["hlc_level"].get<int>();
            if (body.contains("dark_boost_level")) blc.dark_boost_level = body["dark_boost_level"].get<int>();

            if (isp.SetBLC(blc)) {
                isp.SaveToConfig();
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Invalid BLC values";
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
        spdlog::error("HandleImageBLC error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Enhancement Endpoint
// ============================================================================
api::Response HandleImageEnhancement(const api::RequestContext& ctx) {
    api::Response resp;
    auto& isp = platform::ISPControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto enh = isp.GetEnhancement();
            json j;
            j["scene_preset"] = platform::ScenePresetToString(enh.scene_preset);
            j["nr_2d_enabled"] = enh.nr_2d_enabled;
            j["nr_2d_level"] = enh.nr_2d_level;
            j["nr_2d_mode"] = platform::NrModeToString(enh.nr_2d_mode);
            j["nr_3d_enabled"] = enh.nr_3d_enabled;
            j["nr_3d_level"] = enh.nr_3d_level;
            j["nr_3d_mode"] = platform::NrModeToString(enh.nr_3d_mode);
            j["color_nr_enabled"] = enh.color_nr_enabled;
            j["color_nr_level"] = enh.color_nr_level;
            j["defog_enabled"] = enh.defog_enabled;
            j["defog_level"] = enh.defog_level;
            j["defog_mode"] = platform::DefogModeToString(enh.defog_mode);
            j["edge_enhance_enabled"] = enh.edge_enhance_enabled;
            j["edge_enhance_level"] = enh.edge_enhance_level;
            j["edge_coring"] = enh.edge_coring;
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT" || ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            auto enh = isp.GetEnhancement();

            if (body.contains("scene_preset")) enh.scene_preset = platform::StringToScenePreset(body["scene_preset"].get<std::string>());
            if (body.contains("nr_2d_enabled")) enh.nr_2d_enabled = body["nr_2d_enabled"].get<bool>();
            if (body.contains("nr_2d_level")) enh.nr_2d_level = body["nr_2d_level"].get<int>();
            if (body.contains("nr_2d_mode")) enh.nr_2d_mode = platform::StringToNrMode(body["nr_2d_mode"].get<std::string>());
            if (body.contains("nr_3d_enabled")) enh.nr_3d_enabled = body["nr_3d_enabled"].get<bool>();
            if (body.contains("nr_3d_level")) enh.nr_3d_level = body["nr_3d_level"].get<int>();
            if (body.contains("nr_3d_mode")) enh.nr_3d_mode = platform::StringToNrMode(body["nr_3d_mode"].get<std::string>());
            if (body.contains("color_nr_enabled")) enh.color_nr_enabled = body["color_nr_enabled"].get<bool>();
            if (body.contains("color_nr_level")) enh.color_nr_level = body["color_nr_level"].get<int>();
            if (body.contains("defog_enabled")) enh.defog_enabled = body["defog_enabled"].get<bool>();
            if (body.contains("defog_level")) enh.defog_level = body["defog_level"].get<int>();
            if (body.contains("defog_mode")) enh.defog_mode = platform::StringToDefogMode(body["defog_mode"].get<std::string>());
            if (body.contains("edge_enhance_enabled")) enh.edge_enhance_enabled = body["edge_enhance_enabled"].get<bool>();
            if (body.contains("edge_enhance_level")) enh.edge_enhance_level = body["edge_enhance_level"].get<int>();
            if (body.contains("edge_coring")) enh.edge_coring = body["edge_coring"].get<int>();

            if (isp.SetEnhancement(enh)) {
                isp.SaveToConfig();
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 400;
                json err;
                err["status"] = "error";
                err["message"] = "Invalid enhancement values";
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
        spdlog::error("HandleImageEnhancement error: {}", e.what());
        resp.status_code = 400;
        json err;
        err["status"] = "error";
        err["message"] = e.what();
        resp.body = err.dump(2);
    }

    return resp;
}

// ============================================================================
// Reset Endpoint
// ============================================================================
api::Response HandleImageReset(const api::RequestContext& ctx) {
    api::Response resp;

    if (ctx.method != "POST") {
        resp.status_code = 405;
        json err;
        err["error"] = "Method not allowed";
        resp.body = err.dump(2);
        return resp;
    }

    auto& isp = platform::ISPControl::Instance();
    
    if (isp.ResetToDefaults()) {
        resp.status_code = 200;
        json result;
        result["status"] = "success";
        result["message"] = "Image settings reset to defaults";
        resp.body = result.dump(2);
    } else {
        resp.status_code = 500;
        json err;
        err["status"] = "error";
        err["message"] = "Failed to reset image settings";
        resp.body = err.dump(2);
    }

    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
