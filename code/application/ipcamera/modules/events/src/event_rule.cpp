/**
 * @file event_rule.cpp
 * @brief Event rule implementation
 */

#include "ipcam/event_rule.h"
#include "ipcam/event_types.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <uuid/uuid.h>

using json = nlohmann::json;

namespace ipcam {
namespace events {

// ============================================================================
// TimeRange Implementation
// ============================================================================

bool TimeRange::Contains(int hour, int minute) const {
    int current_mins = hour * 60 + minute;
    int start_mins = start_hour * 60 + start_minute;
    int end_mins = end_hour * 60 + end_minute;
    
    if (start_mins <= end_mins) {
        return current_mins >= start_mins && current_mins <= end_mins;
    } else {
        // Wraps midnight
        return current_mins >= start_mins || current_mins <= end_mins;
    }
}

std::string TimeRange::ToString() const {
    char buf[12];
    snprintf(buf, sizeof(buf), "%02d:%02d-%02d:%02d", 
             start_hour, start_minute, end_hour, end_minute);
    return buf;
}

TimeRange TimeRange::FromString(const std::string& str) {
    TimeRange range;
    if (str.size() >= 11) {
        sscanf(str.c_str(), "%d:%d-%d:%d", 
               &range.start_hour, &range.start_minute, 
               &range.end_hour, &range.end_minute);
    }
    return range;
}

// ============================================================================
// Schedule Implementation
// ============================================================================

bool Schedule::IsActiveNow() const {
    return IsActiveAt(std::chrono::system_clock::now());
}

bool Schedule::IsActiveAt(const std::chrono::system_clock::time_point& time) const {
    if (!enabled) {
        return true;  // If schedule disabled, always active
    }
    
    auto tt = std::chrono::system_clock::to_time_t(time);
    std::tm tm = *std::localtime(&tt);
    
    bool active = true;
    
    // Check day of week
    if (!days_of_week.empty()) {
        int day = tm.tm_wday;  // 0=Sunday
        bool day_matches = std::find(days_of_week.begin(), days_of_week.end(), day) 
                          != days_of_week.end();
        if (!day_matches) {
            active = false;
        }
    }
    
    // Check time ranges
    if (active && !time_ranges.empty()) {
        bool in_range = false;
        for (const auto& range : time_ranges) {
            if (range.Contains(tm.tm_hour, tm.tm_min)) {
                in_range = true;
                break;
            }
        }
        if (!in_range) {
            active = false;
        }
    }
    
    return invert ? !active : active;
}

// ============================================================================
// EventRule Implementation
// ============================================================================

bool EventRule::Matches(const Event& event) const {
    // Check if event type is in trigger list
    if (!trigger_events.empty()) {
        if (!EventTypeMatches(event.type, trigger_events)) {
            return false;
        }
    }
    
    // Check zones for motion/intrusion events
    if (!trigger_zones.empty()) {
        bool zone_match = false;
        
        if (auto* motion = std::get_if<MotionEventData>(&event.data)) {
            for (auto zone : motion->zone_ids) {
                if (ZoneMatches(zone, trigger_zones)) {
                    zone_match = true;
                    break;
                }
            }
        } else if (auto* intrusion = std::get_if<IntrusionEventData>(&event.data)) {
            zone_match = ZoneMatches(intrusion->zone_id, trigger_zones);
        }
        
        if (!zone_match) {
            return false;
        }
    }
    
    // Check lines for line crossing events
    if (!trigger_lines.empty()) {
        if (auto* line = std::get_if<LineCrossEventData>(&event.data)) {
            bool found = std::find(trigger_lines.begin(), trigger_lines.end(), 
                                  line->line_id) != trigger_lines.end();
            if (!found) {
                return false;
            }
        }
    }
    
    // Check object classes for analytics events
    if (!trigger_classes.empty()) {
        bool class_match = false;
        
        if (auto* analytics = std::get_if<AnalyticsEventData>(&event.data)) {
            for (const auto& obj : analytics->objects) {
                if (ClassMatches(obj.class_name, trigger_classes)) {
                    class_match = true;
                    break;
                }
            }
        } else if (auto* line = std::get_if<LineCrossEventData>(&event.data)) {
            class_match = ClassMatches(line->object_class, trigger_classes);
        } else if (auto* intrusion = std::get_if<IntrusionEventData>(&event.data)) {
            class_match = ClassMatches(intrusion->object_class, trigger_classes);
        }
        
        if (!class_match) {
            return false;
        }
    }
    
    // Check minimum confidence
    if (min_confidence > 0.0f) {
        if (auto* analytics = std::get_if<AnalyticsEventData>(&event.data)) {
            bool above_threshold = false;
            for (const auto& obj : analytics->objects) {
                if (obj.confidence >= min_confidence) {
                    above_threshold = true;
                    break;
                }
            }
            if (!above_threshold) {
                return false;
            }
        }
    }
    
    // Check minimum objects
    if (min_objects > 1) {
        if (auto* analytics = std::get_if<AnalyticsEventData>(&event.data)) {
            if (static_cast<int>(analytics->objects.size()) < min_objects) {
                return false;
            }
        }
    }
    
    return true;
}

bool EventRule::IsActiveNow() const {
    return schedule.IsActiveNow();
}

bool EventRule::IsCooledDown() const {
    if (cooldown_seconds <= 0) {
        return true;
    }
    
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_triggered).count();
    return elapsed >= cooldown_seconds;
}

void EventRule::MarkTriggered() const {
    last_triggered = std::chrono::system_clock::now();
    trigger_count++;
}

std::string EventRule::ToJson() const {
    json j;
    j["id"] = id;
    j["name"] = name;
    j["description"] = description;
    j["enabled"] = enabled;
    j["priority"] = priority;
    
    // Trigger events
    j["trigger_events"] = json::array();
    for (auto type : trigger_events) {
        j["trigger_events"].push_back(EventTypeToString(type));
    }
    
    j["trigger_zones"] = trigger_zones;
    j["trigger_lines"] = trigger_lines;
    j["trigger_classes"] = trigger_classes;
    j["min_confidence"] = min_confidence;
    j["min_objects"] = min_objects;
    j["cooldown_seconds"] = cooldown_seconds;
    
    // Schedule
    j["schedule"]["enabled"] = schedule.enabled;
    j["schedule"]["days_of_week"] = schedule.days_of_week;
    j["schedule"]["time_ranges"] = json::array();
    for (const auto& range : schedule.time_ranges) {
        j["schedule"]["time_ranges"].push_back(range.ToString());
    }
    j["schedule"]["invert"] = schedule.invert;
    
    // Actions (simplified for now)
    j["actions"] = json::array();
    for (const auto& action : actions) {
        json aj;
        aj["type"] = ActionTypeToString(action.type);
        aj["enabled"] = action.enabled;
        aj["priority"] = action.priority;
        aj["delay_ms"] = action.delay_ms;
        aj["name"] = action.name;
        j["actions"].push_back(aj);
    }
    
    return j.dump(2);
}

EventRule EventRule::FromJson(const std::string& json_str) {
    EventRule rule;
    
    try {
        auto j = json::parse(json_str);
        
        rule.id = j.value("id", "");
        rule.name = j.value("name", "");
        rule.description = j.value("description", "");
        rule.enabled = j.value("enabled", true);
        rule.priority = j.value("priority", 0);
        
        // Trigger events
        if (j.contains("trigger_events")) {
            for (const auto& type_str : j["trigger_events"]) {
                rule.trigger_events.push_back(StringToEventType(type_str.get<std::string>()));
            }
        }
        
        // Trigger zones
        if (j.contains("trigger_zones")) {
            rule.trigger_zones = j["trigger_zones"].get<std::vector<uint32_t>>();
        }
        
        // Trigger lines
        if (j.contains("trigger_lines")) {
            rule.trigger_lines = j["trigger_lines"].get<std::vector<uint32_t>>();
        }
        
        // Trigger classes
        if (j.contains("trigger_classes")) {
            rule.trigger_classes = j["trigger_classes"].get<std::vector<std::string>>();
        }
        
        rule.min_confidence = j.value("min_confidence", 0.0f);
        rule.min_objects = j.value("min_objects", 1);
        rule.cooldown_seconds = j.value("cooldown_seconds", 10);
        
        // Schedule
        if (j.contains("schedule")) {
            auto& sj = j["schedule"];
            rule.schedule.enabled = sj.value("enabled", false);
            if (sj.contains("days_of_week")) {
                rule.schedule.days_of_week = sj["days_of_week"].get<std::vector<int>>();
            }
            if (sj.contains("time_ranges")) {
                for (const auto& range_str : sj["time_ranges"]) {
                    rule.schedule.time_ranges.push_back(TimeRange::FromString(range_str.get<std::string>()));
                }
            }
            rule.schedule.invert = sj.value("invert", false);
        }
        
        // Actions (simplified for now)
        if (j.contains("actions")) {
            for (const auto& aj : j["actions"]) {
                Action action;
                action.type = StringToActionType(aj.value("type", "log_event"));
                action.enabled = aj.value("enabled", true);
                action.priority = aj.value("priority", 0);
                action.delay_ms = aj.value("delay_ms", 0);
                action.name = aj.value("name", "");
                rule.actions.push_back(action);
            }
        }
    } catch (const std::exception& e) {
        spdlog::error("[EventRule] Failed to parse JSON: {}", e.what());
    }
    
    return rule;
}

EventRule EventRule::Create(const std::string& name) {
    EventRule rule;
    
    // Generate UUID
    uuid_t uuid;
    uuid_generate(uuid);
    char uuid_str[37];
    uuid_unparse_lower(uuid, uuid_str);
    rule.id = uuid_str;
    
    rule.name = name;
    rule.last_triggered = std::chrono::system_clock::time_point();
    
    return rule;
}

// ============================================================================
// Helper Functions
// ============================================================================

bool EventTypeMatches(EventType event_type, const std::vector<EventType>& triggers) {
    return std::find(triggers.begin(), triggers.end(), event_type) != triggers.end();
}

bool ZoneMatches(uint32_t zone_id, const std::vector<uint32_t>& filter) {
    if (filter.empty()) {
        return true;
    }
    return std::find(filter.begin(), filter.end(), zone_id) != filter.end();
}

bool ClassMatches(const std::string& class_name, const std::vector<std::string>& filter) {
    if (filter.empty()) {
        return true;
    }
    return std::find(filter.begin(), filter.end(), class_name) != filter.end();
}

} // namespace events
} // namespace ipcam
