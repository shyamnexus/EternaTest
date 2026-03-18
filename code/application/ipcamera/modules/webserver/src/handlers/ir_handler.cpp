#include "ir_handler.h"
#include <ipcam/ir_control.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// Helper: Build full IR settings JSON (user-facing settings only)
// Low-level hardware settings (PWM channel, GPIO pins, etc.) are kept in
// the config file but not exposed via API for a cleaner user experience.
// ============================================================================
static json BuildIRSettingsJson() {
    auto& ir = platform::IRControl::Instance();
    json j;

    // IR LED settings (user-facing only)
    auto led = ir.GetIRLedSettings();
    j["led"]["enabled"] = led.enabled;
    j["led"]["brightness"] = led.brightness;

    // IR Cut settings (user-facing only)
    auto cut = ir.GetIRCutSettings();
    j["cut"]["enabled"] = cut.enabled;
    j["cut"]["state"] = platform::IRCutStateToString(ir.GetIRCutState());

    // Auto day/night settings (user-facing only)
    auto dn = ir.GetDayNightAutoSettings();
    j["auto"]["mode"] = platform::DayNightAutoModeToString(dn.mode);
    j["auto"]["day_to_night_threshold"] = dn.day_to_night_threshold;
    j["auto"]["night_to_day_threshold"] = dn.night_to_day_threshold;
    j["auto"]["switch_delay_sec"] = dn.switch_delay_sec;
    j["auto"]["schedule_night_start"] = dn.schedule_night_start;
    j["auto"]["schedule_day_start"] = dn.schedule_day_start;
    j["auto"]["night_brightness"] = dn.night_brightness;
    j["auto"]["sync_sensor_bw_mode"] = dn.sync_sensor_bw_mode;

    // Current status
    auto status = ir.GetStatus();
    j["status"]["ir_led_on"] = status.ir_led_on;
    j["status"]["ir_led_brightness"] = status.ir_led_brightness;
    j["status"]["ir_cut_state"] = platform::IRCutStateToString(status.ir_cut_state);
    j["status"]["is_night_mode"] = status.is_night_mode;
    j["status"]["current_luma"] = status.current_luma;
    j["status"]["current_ev"] = status.current_ev;
    j["status"]["last_switch_time"] = status.last_switch_time;
    j["status"]["last_switch_reason"] = status.last_switch_reason;
    j["status"]["sw_cds_active"] = status.sw_cds_active;

    return j;
}

// ============================================================================
// Main IR Endpoint - GET/PUT full configuration
// ============================================================================
api::Response HandleIR(const api::RequestContext& ctx) {
    api::Response resp;
    auto& ir = platform::IRControl::Instance();

    try {
        if (ctx.method == "GET") {
            json j = BuildIRSettingsJson();
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            bool success = true;
            std::string message;

            // Update LED settings (user-facing only)
            if (body.contains("led")) {
                auto& led_json = body["led"];
                platform::IRLedSettings led = ir.GetIRLedSettings();
                
                if (led_json.contains("enabled")) led.enabled = led_json["enabled"];
                if (led_json.contains("brightness")) led.brightness = led_json["brightness"];
                
                if (!ir.SetIRLedSettings(led)) {
                    success = false;
                    message = "Failed to update LED settings";
                }
            }

            // Update IR Cut settings (user-facing only)
            if (body.contains("cut") && success) {
                auto& cut_json = body["cut"];
                platform::IRCutSettings cut = ir.GetIRCutSettings();
                
                if (cut_json.contains("enabled")) cut.enabled = cut_json["enabled"];
                
                if (!ir.SetIRCutSettings(cut)) {
                    success = false;
                    message = "Failed to update IR cut settings";
                }
            }

            // Update Auto day/night settings (user-facing only)
            if (body.contains("auto") && success) {
                auto& auto_json = body["auto"];
                platform::DayNightAutoSettings dn = ir.GetDayNightAutoSettings();
                
                if (auto_json.contains("mode")) {
                    dn.mode = platform::StringToDayNightAutoMode(auto_json["mode"]);
                }
                if (auto_json.contains("day_to_night_threshold")) {
                    dn.day_to_night_threshold = auto_json["day_to_night_threshold"];
                }
                if (auto_json.contains("night_to_day_threshold")) {
                    dn.night_to_day_threshold = auto_json["night_to_day_threshold"];
                }
                if (auto_json.contains("switch_delay_sec")) {
                    dn.switch_delay_sec = auto_json["switch_delay_sec"];
                }
                if (auto_json.contains("schedule_night_start")) {
                    dn.schedule_night_start = auto_json["schedule_night_start"];
                }
                if (auto_json.contains("schedule_day_start")) {
                    dn.schedule_day_start = auto_json["schedule_day_start"];
                }
                if (auto_json.contains("night_brightness")) {
                    dn.night_brightness = auto_json["night_brightness"];
                }
                if (auto_json.contains("sync_sensor_bw_mode")) {
                    dn.sync_sensor_bw_mode = auto_json["sync_sensor_bw_mode"];
                }
                
                if (!ir.SetDayNightAutoSettings(dn)) {
                    success = false;
                    message = "Failed to update auto day/night settings";
                }
            }

            json response;
            response["success"] = success;
            response["message"] = success ? "IR settings updated" : message;
            if (success) {
                response["config"] = BuildIRSettingsJson();
            }
            
            resp.status_code = success ? 200 : 400;
            resp.body = response.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleIR error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }

    resp.headers["Content-Type"] = "application/json";
    return resp;
}

// ============================================================================
// IR LED Endpoint (user-facing settings only)
// ============================================================================
api::Response HandleIRLed(const api::RequestContext& ctx) {
    api::Response resp;
    auto& ir = platform::IRControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto led = ir.GetIRLedSettings();
            json j;
            j["enabled"] = led.enabled;
            j["brightness"] = led.brightness;
            j["is_on"] = ir.GetStatus().ir_led_on;
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            platform::IRLedSettings led = ir.GetIRLedSettings();
            
            if (body.contains("enabled")) led.enabled = body["enabled"];
            if (body.contains("brightness")) led.brightness = body["brightness"];
            
            bool success = ir.SetIRLedSettings(led);
            
            json response;
            response["success"] = success;
            response["message"] = success ? "IR LED settings updated" : "Failed to update settings";
            
            resp.status_code = success ? 200 : 400;
            resp.body = response.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleIRLed error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }

    resp.headers["Content-Type"] = "application/json";
    return resp;
}

// ============================================================================
// IR LED Brightness Endpoint
// ============================================================================
api::Response HandleIRLedBrightness(const api::RequestContext& ctx) {
    api::Response resp;
    auto& ir = platform::IRControl::Instance();

    try {
        if (ctx.method == "GET") {
            json j;
            j["brightness"] = ir.GetIRLedBrightness();
            j["is_on"] = ir.GetStatus().ir_led_on;
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            if (!body.contains("brightness")) {
                resp.status_code = 400;
                resp.body = R"({"error": "Missing 'brightness' field"})";
                resp.headers["Content-Type"] = "application/json";
                return resp;
            }
            
            int brightness = body["brightness"];
            bool success = ir.SetIRLedBrightness(brightness);
            
            json response;
            response["success"] = success;
            response["brightness"] = ir.GetIRLedBrightness();
            response["message"] = success ? "Brightness updated" : "Failed to set brightness";
            
            resp.status_code = success ? 200 : 400;
            resp.body = response.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleIRLedBrightness error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }

    resp.headers["Content-Type"] = "application/json";
    return resp;
}

// ============================================================================
// IR Cut Filter Endpoint (user-facing settings only)
// ============================================================================
api::Response HandleIRCut(const api::RequestContext& ctx) {
    api::Response resp;
    auto& ir = platform::IRControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto cut = ir.GetIRCutSettings();
            json j;
            j["enabled"] = cut.enabled;
            j["state"] = platform::IRCutStateToString(ir.GetIRCutState());
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            // If just setting state
            if (body.contains("state")) {
                std::string state_str = body["state"];
                platform::IRCutState state = platform::StringToIRCutState(state_str);
                bool success = ir.SetIRCutState(state);
                
                json response;
                response["success"] = success;
                response["state"] = platform::IRCutStateToString(ir.GetIRCutState());
                response["message"] = success ? "IR cut state updated" : "Failed to set state";
                
                resp.status_code = success ? 200 : 400;
                resp.body = response.dump(2);
            }
            else {
                // Update enabled setting
                platform::IRCutSettings cut = ir.GetIRCutSettings();
                
                if (body.contains("enabled")) cut.enabled = body["enabled"];
                
                bool success = ir.SetIRCutSettings(cut);
                
                json response;
                response["success"] = success;
                response["message"] = success ? "IR cut settings updated" : "Failed to update settings";
                
                resp.status_code = success ? 200 : 400;
                resp.body = response.dump(2);
            }
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleIRCut error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }

    resp.headers["Content-Type"] = "application/json";
    return resp;
}

// ============================================================================
// Day/Night Settings Endpoint (user-facing settings only)
// ============================================================================
api::Response HandleDayNight(const api::RequestContext& ctx) {
    api::Response resp;
    auto& ir = platform::IRControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto dn = ir.GetDayNightAutoSettings();
            json j;
            j["mode"] = platform::DayNightAutoModeToString(dn.mode);
            j["day_to_night_threshold"] = dn.day_to_night_threshold;
            j["night_to_day_threshold"] = dn.night_to_day_threshold;
            j["switch_delay_sec"] = dn.switch_delay_sec;
            j["schedule_night_start"] = dn.schedule_night_start;
            j["schedule_day_start"] = dn.schedule_day_start;
            j["night_brightness"] = dn.night_brightness;
            j["sync_sensor_bw_mode"] = dn.sync_sensor_bw_mode;
            j["is_night_mode"] = ir.IsNightMode();
            j["current_luma"] = ir.GetCurrentLuma();
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            platform::DayNightAutoSettings dn = ir.GetDayNightAutoSettings();
            
            if (body.contains("mode")) {
                dn.mode = platform::StringToDayNightAutoMode(body["mode"]);
            }
            if (body.contains("day_to_night_threshold")) {
                dn.day_to_night_threshold = body["day_to_night_threshold"];
            }
            if (body.contains("night_to_day_threshold")) {
                dn.night_to_day_threshold = body["night_to_day_threshold"];
            }
            if (body.contains("switch_delay_sec")) {
                dn.switch_delay_sec = body["switch_delay_sec"];
            }
            if (body.contains("schedule_night_start")) {
                dn.schedule_night_start = body["schedule_night_start"];
            }
            if (body.contains("schedule_day_start")) {
                dn.schedule_day_start = body["schedule_day_start"];
            }
            if (body.contains("night_brightness")) {
                dn.night_brightness = body["night_brightness"];
            }
            if (body.contains("sync_sensor_bw_mode")) {
                dn.sync_sensor_bw_mode = body["sync_sensor_bw_mode"];
            }
            
            bool success = ir.SetDayNightAutoSettings(dn);
            
            json response;
            response["success"] = success;
            response["message"] = success ? "Day/Night settings updated" : "Failed to update settings";
            
            resp.status_code = success ? 200 : 400;
            resp.body = response.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleDayNight error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }

    resp.headers["Content-Type"] = "application/json";
    return resp;
}

// ============================================================================
// Day/Night Mode Quick Switch Endpoint
// ============================================================================
api::Response HandleDayNightMode(const api::RequestContext& ctx) {
    api::Response resp;
    auto& ir = platform::IRControl::Instance();

    try {
        if (ctx.method == "GET") {
            json j;
            j["is_night_mode"] = ir.IsNightMode();
            j["mode"] = ir.IsNightMode() ? "night" : "day";
            j["current_luma"] = ir.GetCurrentLuma();
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            
            if (!body.contains("mode")) {
                resp.status_code = 400;
                resp.body = R"json({"error": "Missing 'mode' field - use 'day' or 'night'"})json";
                resp.headers["Content-Type"] = "application/json";
                return resp;
            }
            
            std::string mode = body["mode"];
            bool success = false;
            
            if (mode == "night") {
                success = ir.SwitchToNightMode();
            } else if (mode == "day") {
                success = ir.SwitchToDayMode();
            } else {
                resp.status_code = 400;
                resp.body = R"json({"error": "Invalid mode - use 'day' or 'night'"})json";
                resp.headers["Content-Type"] = "application/json";
                return resp;
            }
            
            json response;
            response["success"] = success;
            response["is_night_mode"] = ir.IsNightMode();
            response["message"] = success ? "Mode switched" : "Failed to switch mode";
            
            resp.status_code = success ? 200 : 400;
            resp.body = response.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleDayNightMode error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }

    resp.headers["Content-Type"] = "application/json";
    return resp;
}

// ============================================================================
// IR Status Endpoint
// ============================================================================
api::Response HandleIRStatus(const api::RequestContext& ctx) {
    api::Response resp;
    auto& ir = platform::IRControl::Instance();

    try {
        if (ctx.method == "GET") {
            auto status = ir.GetStatus();
            json j;
            j["ir_led_on"] = status.ir_led_on;
            j["ir_led_brightness"] = status.ir_led_brightness;
            j["ir_cut_state"] = platform::IRCutStateToString(status.ir_cut_state);
            j["is_night_mode"] = status.is_night_mode;
            j["current_luma"] = status.current_luma;
            j["current_ev"] = status.current_ev;
            j["last_switch_time"] = status.last_switch_time;
            j["last_switch_reason"] = status.last_switch_reason;
            j["auto_mode"] = platform::DayNightAutoModeToString(ir.GetDayNightAutoMode());
            j["sw_cds_initialized"] = status.sw_cds_initialized;
            j["sw_cds_active"] = status.sw_cds_active;
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleIRStatus error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }

    resp.headers["Content-Type"] = "application/json";
    return resp;
}

// ============================================================================
// IR Action Endpoint (POST actions: switch_day, switch_night, reset)
// ============================================================================
api::Response HandleIRAction(const api::RequestContext& ctx) {
    api::Response resp;
    auto& ir = platform::IRControl::Instance();

    try {
        if (ctx.method == "POST") {
            auto body = json::parse(ctx.body);
            
            if (!body.contains("action")) {
                resp.status_code = 400;
                resp.body = R"({"error": "Missing 'action' field"})";
                resp.headers["Content-Type"] = "application/json";
                return resp;
            }
            
            std::string action = body["action"];
            bool success = false;
            std::string message;
            
            if (action == "switch_day") {
                success = ir.SwitchToDayMode();
                message = success ? "Switched to day mode" : "Failed to switch to day mode";
            }
            else if (action == "switch_night") {
                success = ir.SwitchToNightMode();
                message = success ? "Switched to night mode" : "Failed to switch to night mode";
            }
            else if (action == "reset") {
                success = ir.ResetToDefaults();
                message = success ? "IR settings reset to defaults" : "Failed to reset settings";
            }
            else if (action == "test_led") {
                // Test IR LED at specified brightness
                int brightness = body.value("brightness", 50);
                success = ir.SetIRLedBrightness(brightness);
                message = success ? "IR LED test started" : "Failed to test IR LED";
            }
            else {
                resp.status_code = 400;
                resp.body = R"({"error": "Invalid action. Use: switch_day, switch_night, reset, test_led"})";
                resp.headers["Content-Type"] = "application/json";
                return resp;
            }
            
            json response;
            response["success"] = success;
            response["message"] = message;
            response["status"] = {
                {"is_night_mode", ir.IsNightMode()},
                {"ir_led_on", ir.GetStatus().ir_led_on}
            };
            
            resp.status_code = success ? 200 : 400;
            resp.body = response.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = R"({"error": "Method not allowed"})";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("HandleIRAction error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }

    resp.headers["Content-Type"] = "application/json";
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
