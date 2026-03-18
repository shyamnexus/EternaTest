/**
 * @file event_types.cpp
 * @brief Event type implementations
 */

#include "ipcam/event_types.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>

using json = nlohmann::json;

namespace ipcam {
namespace events {

// ============================================================================
// Event Implementation
// ============================================================================

Event::Event() {
    timestamp = std::chrono::system_clock::now();
    timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()).count();
}

Event Event::Create(EventCategory cat, EventType type) {
    Event event;
    event.category = cat;
    event.type = type;
    
    // Generate UUID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    const char* hex = "0123456789abcdef";
    std::string uuid(36, '-');
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) continue;
        uuid[i] = hex[dis(gen)];
    }
    uuid[14] = '4'; // UUID version 4
    event.id = uuid;
    
    return event;
}

std::string Event::GetTypeString() const {
    return EventTypeToString(type);
}

std::string Event::GetCategoryString() const {
    return EventCategoryToString(category);
}

std::string Event::ToIsoTimestamp() const {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string Event::ToJson() const {
    json j;
    j["id"] = id;
    j["category"] = GetCategoryString();
    j["type"] = GetTypeString();
    j["timestamp"] = ToIsoTimestamp();
    j["timestamp_ms"] = timestamp_ms;
    j["channel"] = channel;
    j["source"] = source;
    
    // Add snapshot/video paths if present
    if (snapshot_path.has_value()) {
        j["snapshot_path"] = snapshot_path.value();
    }
    if (video_path.has_value()) {
        j["video_path"] = video_path.value();
    }
    if (thumbnail_path.has_value()) {
        j["thumbnail_path"] = thumbnail_path.value();
    }
    
    // Serialize data based on type
    std::visit([&j](auto&& data) {
        using T = std::decay_t<decltype(data)>;
        
        if constexpr (std::is_same_v<T, MotionEventData>) {
            j["data"]["zone_ids"] = data.zone_ids;
            j["data"]["motion_level"] = data.motion_level;
            j["data"]["motion_block_count"] = data.motion_block_count;
            j["data"]["total_blocks"] = data.total_blocks;
        }
        else if constexpr (std::is_same_v<T, AnalyticsEventData>) {
            j["data"]["total_persons"] = data.total_persons;
            j["data"]["total_vehicles"] = data.total_vehicles;
            j["data"]["total_faces"] = data.total_faces;
            j["data"]["objects"] = json::array();
            for (const auto& obj : data.objects) {
                json obj_j;
                obj_j["id"] = obj.id;
                obj_j["class"] = obj.class_name;
                obj_j["confidence"] = obj.confidence;
                obj_j["bbox"] = {obj.x1, obj.y1, obj.x2, obj.y2};
                j["data"]["objects"].push_back(obj_j);
            }
        }
        else if constexpr (std::is_same_v<T, LineCrossEventData>) {
            j["data"]["line_id"] = data.line_id;
            j["data"]["line_name"] = data.line_name;
            j["data"]["object_id"] = data.object_id;
            j["data"]["object_class"] = data.object_class;
            j["data"]["direction"] = data.direction;
            j["data"]["count_in"] = data.count_in;
            j["data"]["count_out"] = data.count_out;
        }
        else if constexpr (std::is_same_v<T, IntrusionEventData>) {
            j["data"]["zone_id"] = data.zone_id;
            j["data"]["zone_name"] = data.zone_name;
            j["data"]["object_id"] = data.object_id;
            j["data"]["object_class"] = data.object_class;
            j["data"]["dwell_time_ms"] = data.dwell_time_ms;
        }
        else if constexpr (std::is_same_v<T, SystemEventData>) {
            j["data"]["component"] = data.component;
            j["data"]["message"] = data.message;
            j["data"]["error_code"] = data.error_code;
            j["data"]["severity"] = data.severity;
        }
        else if constexpr (std::is_same_v<T, IOEventData>) {
            j["data"]["input_id"] = data.input_id;
            j["data"]["name"] = data.name;
            j["data"]["state"] = data.state;
        }
        else if constexpr (std::is_same_v<T, StorageEventData>) {
            j["data"]["device"] = data.device;
            j["data"]["type"] = data.type;
            j["data"]["total_bytes"] = data.total_bytes;
            j["data"]["free_bytes"] = data.free_bytes;
            j["data"]["usage_percent"] = data.usage_percent;
        }
        else if constexpr (std::is_same_v<T, NetworkEventData>) {
            j["data"]["interface"] = data.interface;
            j["data"]["ip_address"] = data.ip_address;
            j["data"]["connection_type"] = data.connection_type;
        }
        else if constexpr (std::is_same_v<T, TamperEventData>) {
            j["data"]["tamper_type"] = data.tamper_type;
            j["data"]["active"] = data.active;
            j["data"]["metric_value"] = data.metric_value;
        }
        else if constexpr (std::is_same_v<T, LprEventData>) {
            j["data"]["plate_text"] = data.plate_text;
            j["data"]["confidence"] = data.confidence;
            j["data"]["bbox"] = {data.x1, data.y1, data.x2, data.y2};
        }
        else if constexpr (std::is_same_v<T, AudioEventData>) {
            j["data"]["class_id"] = data.class_id;
            j["data"]["class_name"] = data.class_name;
            j["data"]["confidence"] = data.confidence;
        }
    }, data);
    
    return j.dump();
}

Event Event::FromJson(const std::string& json_str) {
    Event event;
    try {
        auto j = json::parse(json_str);
        
        event.id = j.value("id", "");
        event.category = StringToEventCategory(j.value("category", "system"));
        event.type = StringToEventType(j.value("type", "unknown"));
        event.timestamp_ms = j.value("timestamp_ms", 0ULL);
        event.channel = j.value("channel", 0);
        event.source = j.value("source", "");
        
        if (j.contains("snapshot_path")) {
            event.snapshot_path = j["snapshot_path"].get<std::string>();
        }
        if (j.contains("video_path")) {
            event.video_path = j["video_path"].get<std::string>();
        }
        if (j.contains("thumbnail_path")) {
            event.thumbnail_path = j["thumbnail_path"].get<std::string>();
        }
        
        // Parse data based on category
        // (simplified - full implementation would parse all data types)
        
    } catch (const std::exception& e) {
        // Return default event on parse error
    }
    
    return event;
}

// ============================================================================
// Conversion Functions
// ============================================================================

std::string EventTypeToString(EventType type) {
    switch (type) {
        // Motion
        case EventType::kMotionStart: return "motion_start";
        case EventType::kMotionEnd: return "motion_end";
        case EventType::kMotionContinuous: return "motion_continuous";
        
        // Analytics
        case EventType::kPersonDetected: return "person_detected";
        case EventType::kPersonLost: return "person_lost";
        case EventType::kVehicleDetected: return "vehicle_detected";
        case EventType::kVehicleLost: return "vehicle_lost";
        case EventType::kFaceDetected: return "face_detected";
        case EventType::kFaceLost: return "face_lost";
        case EventType::kAnimalDetected: return "animal_detected";
        case EventType::kAnimalLost: return "animal_lost";
        case EventType::kObjectTracked: return "object_tracked";
        case EventType::kLprDetected: return "lpr_detected";
        case EventType::kAudioDetected: return "audio_detected";
        
        // Line crossing
        case EventType::kLineCrossedLeftToRight: return "line_crossed_left_to_right";
        case EventType::kLineCrossedRightToLeft: return "line_crossed_right_to_left";
        case EventType::kLineCrossedTopToBottom: return "line_crossed_top_to_bottom";
        case EventType::kLineCrossedBottomToTop: return "line_crossed_bottom_to_top";
        case EventType::kLineCrossedAny: return "line_crossed_any";
        
        // Zone intrusion
        case EventType::kZoneEntered: return "zone_entered";
        case EventType::kZoneExited: return "zone_exited";
        case EventType::kZoneLoitering: return "zone_loitering";
        
        // System
        case EventType::kSystemStartup: return "system_startup";
        case EventType::kSystemShutdown: return "system_shutdown";
        case EventType::kSystemError: return "system_error";
        case EventType::kTamperDetected: return "tamper_detected";
        case EventType::kTamperCleared: return "tamper_cleared";
        
        // I/O
        case EventType::kAlarmInputTriggered: return "alarm_input_triggered";
        case EventType::kAlarmInputCleared: return "alarm_input_cleared";
        
        // Storage
        case EventType::kStorageMounted: return "storage_mounted";
        case EventType::kStorageUnmounted: return "storage_unmounted";
        case EventType::kStorageFull: return "storage_full";
        case EventType::kStorageError: return "storage_error";
        case EventType::kRecordingStarted: return "recording_started";
        case EventType::kRecordingStopped: return "recording_stopped";
        case EventType::kRecordingError: return "recording_error";
        
        // Network
        case EventType::kNetworkConnected: return "network_connected";
        case EventType::kNetworkDisconnected: return "network_disconnected";
        
        default: return "unknown";
    }
}

EventType StringToEventType(const std::string& str) {
    static const std::map<std::string, EventType> map = {
        // Motion
        {"motion_start", EventType::kMotionStart},
        {"motion_end", EventType::kMotionEnd},
        {"motion_continuous", EventType::kMotionContinuous},
        
        // Analytics
        {"person_detected", EventType::kPersonDetected},
        {"person_lost", EventType::kPersonLost},
        {"vehicle_detected", EventType::kVehicleDetected},
        {"vehicle_lost", EventType::kVehicleLost},
        {"face_detected", EventType::kFaceDetected},
        {"face_lost", EventType::kFaceLost},
        {"animal_detected", EventType::kAnimalDetected},
        {"animal_lost", EventType::kAnimalLost},
        {"object_tracked", EventType::kObjectTracked},
        {"lpr_detected", EventType::kLprDetected},
        {"audio_detected", EventType::kAudioDetected},
        
        // Line crossing
        {"line_crossed_left_to_right", EventType::kLineCrossedLeftToRight},
        {"line_crossed_right_to_left", EventType::kLineCrossedRightToLeft},
        {"line_crossed_top_to_bottom", EventType::kLineCrossedTopToBottom},
        {"line_crossed_bottom_to_top", EventType::kLineCrossedBottomToTop},
        {"line_crossed_any", EventType::kLineCrossedAny},
        
        // Zone intrusion
        {"zone_entered", EventType::kZoneEntered},
        {"zone_exited", EventType::kZoneExited},
        {"zone_loitering", EventType::kZoneLoitering},
        
        // System
        {"system_startup", EventType::kSystemStartup},
        {"system_shutdown", EventType::kSystemShutdown},
        {"system_error", EventType::kSystemError},
        {"tamper_detected", EventType::kTamperDetected},
        {"tamper_cleared", EventType::kTamperCleared},
        
        // I/O
        {"alarm_input_triggered", EventType::kAlarmInputTriggered},
        {"alarm_input_cleared", EventType::kAlarmInputCleared},
        
        // Storage
        {"storage_mounted", EventType::kStorageMounted},
        {"storage_unmounted", EventType::kStorageUnmounted},
        {"storage_full", EventType::kStorageFull},
        {"storage_error", EventType::kStorageError},
        {"recording_started", EventType::kRecordingStarted},
        {"recording_stopped", EventType::kRecordingStopped},
        {"recording_error", EventType::kRecordingError},
        
        // Network
        {"network_connected", EventType::kNetworkConnected},
        {"network_disconnected", EventType::kNetworkDisconnected}
    };
    
    auto it = map.find(str);
    return (it != map.end()) ? it->second : EventType::kUnknown;
}

std::string EventCategoryToString(EventCategory cat) {
    switch (cat) {
        case EventCategory::kMotion: return "motion";
        case EventCategory::kAnalytics: return "analytics";
        case EventCategory::kLineCrossing: return "line_crossing";
        case EventCategory::kIntrusion: return "intrusion";
        case EventCategory::kSystem: return "system";
        case EventCategory::kIO: return "io";
        case EventCategory::kStorage: return "storage";
        case EventCategory::kNetwork: return "network";
        case EventCategory::kSchedule: return "schedule";
        default: return "unknown";
    }
}

EventCategory StringToEventCategory(const std::string& str) {
    static const std::map<std::string, EventCategory> map = {
        {"motion", EventCategory::kMotion},
        {"analytics", EventCategory::kAnalytics},
        {"line_crossing", EventCategory::kLineCrossing},
        {"intrusion", EventCategory::kIntrusion},
        {"system", EventCategory::kSystem},
        {"io", EventCategory::kIO},
        {"storage", EventCategory::kStorage},
        {"network", EventCategory::kNetwork},
        {"schedule", EventCategory::kSchedule}
    };
    
    auto it = map.find(str);
    return (it != map.end()) ? it->second : EventCategory::kSystem;
}

std::string ActionTypeToString(ActionType type) {
    switch (type) {
        case ActionType::kStartRecording: return "start_recording";
        case ActionType::kStopRecording: return "stop_recording";
        case ActionType::kCaptureSnapshot: return "capture_snapshot";
        case ActionType::kSendEmail: return "send_email";
        case ActionType::kSendPushNotification: return "send_push";
        case ActionType::kPublishOnvifEvent: return "publish_onvif_event";
        case ActionType::kSendWebhook: return "send_webhook";
        case ActionType::kPublishMqtt: return "publish_mqtt";
        case ActionType::kUploadFtp: return "upload_ftp";
        case ActionType::kUploadCloud: return "upload_cloud";
        case ActionType::kTriggerAlarmOutput: return "trigger_alarm_output";
        case ActionType::kClearAlarmOutput: return "clear_alarm_output";
        case ActionType::kActivateLight: return "activate_light";
        case ActionType::kDeactivateLight: return "deactivate_light";
        case ActionType::kPlaySiren: return "play_siren";
        case ActionType::kStopSiren: return "stop_siren";
        case ActionType::kPtzGotoPreset: return "ptz_goto_preset";
        case ActionType::kPtzStartPatrol: return "ptz_start_patrol";
        case ActionType::kLogEvent: return "log_event";
        case ActionType::kGenerateThumbnail: return "generate_thumbnail";
        case ActionType::kAttachMetadata: return "attach_metadata";
        default: return "unknown";
    }
}

ActionType StringToActionType(const std::string& str) {
    static const std::map<std::string, ActionType> map = {
        {"start_recording", ActionType::kStartRecording},
        {"stop_recording", ActionType::kStopRecording},
        {"capture_snapshot", ActionType::kCaptureSnapshot},
        {"send_email", ActionType::kSendEmail},
        {"send_push", ActionType::kSendPushNotification},
        {"publish_onvif_event", ActionType::kPublishOnvifEvent},
        {"send_webhook", ActionType::kSendWebhook},
        {"publish_mqtt", ActionType::kPublishMqtt},
        {"upload_ftp", ActionType::kUploadFtp},
        {"upload_cloud", ActionType::kUploadCloud},
        {"trigger_alarm_output", ActionType::kTriggerAlarmOutput},
        {"clear_alarm_output", ActionType::kClearAlarmOutput},
        {"activate_light", ActionType::kActivateLight},
        {"deactivate_light", ActionType::kDeactivateLight},
        {"play_siren", ActionType::kPlaySiren},
        {"stop_siren", ActionType::kStopSiren},
        {"ptz_goto_preset", ActionType::kPtzGotoPreset},
        {"ptz_start_patrol", ActionType::kPtzStartPatrol},
        {"log_event", ActionType::kLogEvent},
        {"generate_thumbnail", ActionType::kGenerateThumbnail},
        {"attach_metadata", ActionType::kAttachMetadata}
    };
    
    auto it = map.find(str);
    return (it != map.end()) ? it->second : ActionType::kLogEvent;
}

} // namespace events
} // namespace ipcam
