/**
 * @file schedule_handler.cpp
 * @brief Recording Schedule API handler implementations
 */

#include "schedule_handler.h"
#include <ipcam/schedule_manager.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <algorithm>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// JSON Conversion Helpers
// ============================================================================

static json TimeSlotToJson(const recording::TimeSlot& slot) {
    return json{
        {"hour", slot.hour},
        {"enabled", slot.enabled},
        {"type", recording::RecordingTypeToString(slot.type)}
    };
}

static recording::TimeSlot JsonToTimeSlot(const json& j) {
    recording::TimeSlot slot;
    slot.hour = j.value("hour", 0);
    slot.enabled = j.value("enabled", false);
    slot.type = recording::StringToRecordingType(j.value("type", "continuous"));
    return slot;
}

static json DayScheduleToJson(const recording::DaySchedule& day) {
    json slots = json::array();
    for (const auto& slot : day.slots) {
        slots.push_back(TimeSlotToJson(slot));
    }
    return json{
        {"day_of_week", day.day_of_week},
        {"slots", slots}
    };
}

static recording::DaySchedule JsonToDaySchedule(const json& j) {
    recording::DaySchedule day;
    day.day_of_week = j.value("day_of_week", 0);
    if (j.contains("slots") && j["slots"].is_array()) {
        int idx = 0;
        for (const auto& slot_json : j["slots"]) {
            if (idx < 24) {
                day.slots[idx] = JsonToTimeSlot(slot_json);
                idx++;
            }
        }
    }
    return day;
}

static json ProfileToJson(const recording::ScheduleProfile& profile) {
    json weekly = json::array();
    for (const auto& day : profile.weekly_schedule) {
        weekly.push_back(DayScheduleToJson(day));
    }
    
    return json{
        {"id", profile.id},
        {"name", profile.name},
        {"enabled", profile.enabled},
        {"channels", profile.channels},
        {"weekly_schedule", weekly},
        {"pre_record_seconds", profile.pre_record_seconds},
        {"post_record_seconds", profile.post_record_seconds},
        {"created_at", profile.created_at},
        {"updated_at", profile.updated_at}
    };
}

static recording::ScheduleProfile JsonToProfile(const json& j) {
    recording::ScheduleProfile profile;
    profile.id = j.value("id", "");
    profile.name = j.value("name", "New Profile");
    profile.enabled = j.value("enabled", true);
    
    if (j.contains("channels") && j["channels"].is_array()) {
        for (const auto& ch : j["channels"]) {
            profile.channels.push_back(ch.get<int>());
        }
    } else {
        profile.channels = {0};  // Default to channel 0
    }
    
    if (j.contains("weekly_schedule") && j["weekly_schedule"].is_array()) {
        int idx = 0;
        for (const auto& day_json : j["weekly_schedule"]) {
            if (idx < 7) {
                profile.weekly_schedule[idx] = JsonToDaySchedule(day_json);
                idx++;
            }
        }
    }
    
    profile.pre_record_seconds = j.value("pre_record_seconds", 5);
    profile.post_record_seconds = j.value("post_record_seconds", 10);
    profile.created_at = j.value("created_at", "");
    profile.updated_at = j.value("updated_at", "");
    
    return profile;
}

// ============================================================================
// Handler Implementations
// ============================================================================

api::Response HandleSchedules(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = recording::ScheduleManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "Schedule manager not initialized"}}.dump();
        return resp;
    }
    
    try {
        if (ctx.method == "GET") {
            // List all schedule profiles
            auto profiles = mgr.GetAllProfiles();
            auto config = mgr.GetConfig();
            
            json profiles_json = json::array();
            for (const auto& p : profiles) {
                profiles_json.push_back(ProfileToJson(p));
            }
            
            json result = {
                {"enabled", config.enabled},
                {"active_profile_id", config.active_profile_id},
                {"profiles", profiles_json}
            };
            
            resp.status_code = 200;
            resp.body = result.dump(2);
        }
        else if (ctx.method == "POST") {
            // Create new profile
            auto j = json::parse(ctx.body);
            auto profile = JsonToProfile(j);
            
            std::string id = mgr.CreateProfile(profile);
            if (id.empty()) {
                resp.status_code = 500;
                resp.body = json{{"error", "Failed to create profile"}}.dump();
                return resp;
            }
            
            auto created = mgr.GetProfile(id);
            if (!created) {
                resp.status_code = 500;
                resp.body = json{{"error", "Profile created but not found"}}.dump();
                return resp;
            }
            
            spdlog::info("[ScheduleHandler] Created profile: {} ({})", created->name, id);
            
            resp.status_code = 201;
            resp.body = ProfileToJson(*created).dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = json{{"error", "Method not allowed"}}.dump();
        }
    } catch (const json::exception& e) {
        spdlog::error("[ScheduleHandler] JSON error: {}", e.what());
        resp.status_code = 400;
        resp.body = json{{"error", std::string("Invalid JSON: ") + e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("[ScheduleHandler] Error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleScheduleById(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = recording::ScheduleManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "Schedule manager not initialized"}}.dump();
        return resp;
    }
    
    // Extract profile ID from URI: /api/v1/recording/schedules/{id}
    std::string id;
    size_t last_slash = ctx.uri.rfind('/');
    if (last_slash != std::string::npos && last_slash < ctx.uri.length() - 1) {
        id = ctx.uri.substr(last_slash + 1);
    }
    
    if (id.empty() || id == "schedules") {
        resp.status_code = 400;
        resp.body = json{{"error", "Profile ID required"}}.dump();
        return resp;
    }
    
    try {
        if (ctx.method == "GET") {
            auto profile = mgr.GetProfile(id);
            if (!profile) {
                resp.status_code = 404;
                resp.body = json{{"error", "Profile not found"}}.dump();
                return resp;
            }
            resp.status_code = 200;
            resp.body = ProfileToJson(*profile).dump(2);
        }
        else if (ctx.method == "PUT") {
            auto j = json::parse(ctx.body);
            auto profile = JsonToProfile(j);
            
            if (!mgr.UpdateProfile(id, profile)) {
                resp.status_code = 404;
                resp.body = json{{"error", "Profile not found"}}.dump();
                return resp;
            }
            
            auto updated = mgr.GetProfile(id);
            if (!updated) {
                resp.status_code = 500;
                resp.body = json{{"error", "Profile updated but not found"}}.dump();
                return resp;
            }
            
            spdlog::info("[ScheduleHandler] Updated profile: {}", id);
            resp.status_code = 200;
            resp.body = ProfileToJson(*updated).dump(2);
        }
        else if (ctx.method == "DELETE") {
            if (!mgr.DeleteProfile(id)) {
                resp.status_code = 404;
                resp.body = json{{"error", "Profile not found"}}.dump();
                return resp;
            }
            
            spdlog::info("[ScheduleHandler] Deleted profile: {}", id);
            resp.status_code = 200;
            resp.body = json{{"message", "Profile deleted"}}.dump();
        }
        else {
            resp.status_code = 405;
            resp.body = json{{"error", "Method not allowed"}}.dump();
        }
    } catch (const json::exception& e) {
        spdlog::error("[ScheduleHandler] JSON error: {}", e.what());
        resp.status_code = 400;
        resp.body = json{{"error", std::string("Invalid JSON: ") + e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("[ScheduleHandler] Error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleScheduleConfig(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = recording::ScheduleManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "Schedule manager not initialized"}}.dump();
        return resp;
    }
    
    try {
        if (ctx.method == "GET") {
            auto config = mgr.GetConfig();
            
            json result = {
                {"enabled", config.enabled},
                {"active_profile_id", config.active_profile_id}
            };
            
            // Add current status info
            auto active = mgr.GetCurrentActiveProfile();
            if (active) {
                result["active_profile_name"] = active->name;
                result["should_record_now"] = mgr.ShouldRecordNow(0);
                result["current_recording_type"] = recording::RecordingTypeToString(
                    mgr.GetCurrentRecordingType(0));
                
                auto next_event = mgr.GetNextScheduledEvent();
                if (next_event) {
                    auto time = std::chrono::system_clock::to_time_t(next_event->timestamp);
                    std::tm tm = *std::localtime(&time);
                    char buf[64];
                    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
                    
                    result["next_event"] = json{
                        {"type", next_event->type == recording::ScheduledEventType::RecordingStart ? 
                                 "start" : "stop"},
                        {"timestamp", buf}
                    };
                }
            }
            
            resp.status_code = 200;
            resp.body = result.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto j = json::parse(ctx.body);
            
            if (j.contains("enabled")) {
                mgr.SetEnabled(j["enabled"].get<bool>());
            }
            
            if (j.contains("active_profile_id")) {
                std::string profile_id = j["active_profile_id"].get<std::string>();
                if (!profile_id.empty()) {
                    if (!mgr.SetActiveProfile(profile_id)) {
                        resp.status_code = 400;
                        resp.body = json{{"error", "Profile not found"}}.dump();
                        return resp;
                    }
                }
            }
            
            auto config = mgr.GetConfig();
            json result = {
                {"enabled", config.enabled},
                {"active_profile_id", config.active_profile_id},
                {"message", "Configuration updated"}
            };
            
            spdlog::info("[ScheduleHandler] Updated schedule config: enabled={}, active_profile={}",
                        config.enabled, config.active_profile_id);
            
            resp.status_code = 200;
            resp.body = result.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = json{{"error", "Method not allowed"}}.dump();
        }
    } catch (const json::exception& e) {
        spdlog::error("[ScheduleHandler] JSON error: {}", e.what());
        resp.status_code = 400;
        resp.body = json{{"error", std::string("Invalid JSON: ") + e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("[ScheduleHandler] Error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
