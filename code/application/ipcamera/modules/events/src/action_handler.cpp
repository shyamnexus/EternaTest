/**
 * @file action_handler.cpp
 * @brief Action handler base class and utilities implementation
 */

#include "ipcam/action_handler.h"
#include "ipcam/event_manager.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <regex>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace ipcam {
namespace events {

// ============================================================================
// Placeholder Substitution Utility
// ============================================================================

std::string ActionHandler::SubstitutePlaceholders(const std::string& template_str, const Event& event) {
    std::string result = template_str;
    
    // Get current time components
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time_t);
    
    char date_buf[32], time_buf[32], datetime_buf[64];
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", tm);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm);
    strftime(datetime_buf, sizeof(datetime_buf), "%Y-%m-%d %H:%M:%S", tm);
    
    // Standard placeholders
    std::map<std::string, std::string> placeholders = {
        {"{event_id}", event.id},
        {"{event_type}", event.GetTypeString()},
        {"{event_category}", EventCategoryToString(event.category)},
        {"{channel}", std::to_string(event.channel)},
        {"{source}", event.source},
        {"{timestamp}", event.ToIsoTimestamp()},
        {"{timestamp_ms}", std::to_string(event.timestamp_ms)},
        {"{date}", date_buf},
        {"{time}", time_buf},
        {"{datetime}", datetime_buf},
        {"{device_name}", GetDeviceName()},
        {"{device_id}", GetDeviceId()},
    };
    
    // Add event-specific placeholders based on data type
    std::visit([&placeholders](auto&& data) {
        using T = std::decay_t<decltype(data)>;
        
        if constexpr (std::is_same_v<T, MotionEventData>) {
            placeholders["{motion_level}"] = std::to_string(data.motion_level);
            
            std::string zones;
            for (size_t i = 0; i < data.zone_ids.size(); ++i) {
                if (i > 0) zones += ",";
                zones += std::to_string(data.zone_ids[i]);
            }
            placeholders["{zone_ids}"] = zones;
        }
        else if constexpr (std::is_same_v<T, AnalyticsEventData>) {
            placeholders["{total_persons}"] = std::to_string(data.total_persons);
            placeholders["{total_vehicles}"] = std::to_string(data.total_vehicles);
            placeholders["{total_faces}"] = std::to_string(data.total_faces);
            placeholders["{object_count}"] = std::to_string(data.objects.size());
            
            if (!data.objects.empty()) {
                placeholders["{object_class}"] = data.objects[0].class_name;
                placeholders["{confidence}"] = std::to_string(data.objects[0].confidence);
            }
        }
        else if constexpr (std::is_same_v<T, LineCrossEventData>) {
            placeholders["{line_id}"] = std::to_string(data.line_id);
            placeholders["{line_name}"] = data.line_name;
            placeholders["{direction}"] = data.direction;
            placeholders["{object_class}"] = data.object_class;
            placeholders["{count_in}"] = std::to_string(data.count_in);
            placeholders["{count_out}"] = std::to_string(data.count_out);
        }
        else if constexpr (std::is_same_v<T, IntrusionEventData>) {
            placeholders["{zone_id}"] = std::to_string(data.zone_id);
            placeholders["{zone_name}"] = data.zone_name;
            placeholders["{object_class}"] = data.object_class;
            placeholders["{dwell_time_ms}"] = std::to_string(data.dwell_time_ms);
        }
        else if constexpr (std::is_same_v<T, SystemEventData>) {
            placeholders["{component}"] = data.component;
            placeholders["{message}"] = data.message;
            placeholders["{error_code}"] = std::to_string(data.error_code);
            placeholders["{severity}"] = data.severity;
        }
        else if constexpr (std::is_same_v<T, IOEventData>) {
            placeholders["{input_id}"] = std::to_string(data.input_id);
            placeholders["{io_name}"] = data.name;
            placeholders["{io_state}"] = data.state ? "active" : "inactive";
        }
        else if constexpr (std::is_same_v<T, StorageEventData>) {
            placeholders["{device}"] = data.device;
            placeholders["{storage_type}"] = data.type;
            placeholders["{total_bytes}"] = std::to_string(data.total_bytes);
            placeholders["{free_bytes}"] = std::to_string(data.free_bytes);
            placeholders["{usage_percent}"] = std::to_string(static_cast<int>(data.usage_percent));
        }
        else if constexpr (std::is_same_v<T, NetworkEventData>) {
            placeholders["{interface}"] = data.interface;
            placeholders["{ip_address}"] = data.ip_address;
            placeholders["{connection_type}"] = data.connection_type;
        }
    }, event.data);
    
    // Add snapshot/video paths if available
    if (event.snapshot_path.has_value()) {
        placeholders["{snapshot_path}"] = event.snapshot_path.value();
    }
    if (event.video_path.has_value()) {
        placeholders["{video_path}"] = event.video_path.value();
    }
    if (event.thumbnail_path.has_value()) {
        placeholders["{thumbnail_path}"] = event.thumbnail_path.value();
    }
    
    // Perform substitutions
    for (const auto& [key, value] : placeholders) {
        size_t pos = 0;
        while ((pos = result.find(key, pos)) != std::string::npos) {
            result.replace(pos, key.length(), value);
            pos += value.length();
        }
    }
    
    return result;
}

std::string ActionHandler::GetDeviceName() const {
    // Get device name from config module
    return config::Get<std::string>("system.device_name", "IPCamera");
}

std::string ActionHandler::GetDeviceId() const {
    // Get device ID from config module
    return config::Get<std::string>("system.device_id", "unknown");
}

// ============================================================================
// ActionHandlerFactory Implementation
// ============================================================================

std::shared_ptr<ActionHandler> ActionHandlerFactory::Create(ActionType type) {
    switch (type) {
        case ActionType::kStartRecording:
        case ActionType::kStopRecording:
            return std::make_shared<RecordingActionHandler>();
            
        case ActionType::kCaptureSnapshot:
        case ActionType::kGenerateThumbnail:
            return std::make_shared<SnapshotActionHandler>();
            
        case ActionType::kSendEmail:
            return std::make_shared<EmailActionHandler>();
            
        case ActionType::kSendWebhook:
            return std::make_shared<WebhookActionHandler>();
            
        case ActionType::kPublishMqtt:
            return std::make_shared<MqttActionHandler>();
            
        case ActionType::kTriggerAlarmOutput:
        case ActionType::kClearAlarmOutput:
            return std::make_shared<IoOutputHandler>();
            
        case ActionType::kActivateLight:
        case ActionType::kDeactivateLight:
            return std::make_shared<LightActionHandler>();
            
        case ActionType::kPlaySiren:
        case ActionType::kStopSiren:
            return std::make_shared<SirenActionHandler>();
            
        case ActionType::kUploadFtp:
            return std::make_shared<FtpUploadHandler>();
            
#ifdef HAVE_ONVIF
        case ActionType::kPublishOnvifEvent:
            return std::make_shared<OnvifEventHandler>();
#endif
            
        default:
            spdlog::warn("[ActionHandlerFactory] No handler for action type: {}", 
                        ActionTypeToString(type));
            return nullptr;
    }
}

void ActionHandlerFactory::RegisterAll(EventManager& manager) {
    spdlog::info("[ActionHandlerFactory] Registering all action handlers");
    
    // Recording actions
    manager.RegisterActionHandler(ActionType::kStartRecording, 
                                  std::make_shared<RecordingActionHandler>());
    
    // Snapshot actions
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot,
                                  std::make_shared<SnapshotActionHandler>());
    manager.RegisterActionHandler(ActionType::kGenerateThumbnail,
                                  std::make_shared<SnapshotActionHandler>());
    
    // Notification actions
    manager.RegisterActionHandler(ActionType::kSendEmail,
                                  std::make_shared<EmailActionHandler>());
    manager.RegisterActionHandler(ActionType::kSendWebhook,
                                  std::make_shared<WebhookActionHandler>());
    manager.RegisterActionHandler(ActionType::kPublishMqtt,
                                  std::make_shared<MqttActionHandler>());
    
    // I/O actions
    auto io_handler = std::make_shared<IoOutputHandler>();
    manager.RegisterActionHandler(ActionType::kTriggerAlarmOutput, io_handler);
    manager.RegisterActionHandler(ActionType::kClearAlarmOutput, io_handler);
    
    auto light_handler = std::make_shared<LightActionHandler>();
    manager.RegisterActionHandler(ActionType::kActivateLight, light_handler);
    manager.RegisterActionHandler(ActionType::kDeactivateLight, light_handler);
    
    auto siren_handler = std::make_shared<SirenActionHandler>();
    manager.RegisterActionHandler(ActionType::kPlaySiren, siren_handler);
    manager.RegisterActionHandler(ActionType::kStopSiren, siren_handler);
    
    // Upload actions
    manager.RegisterActionHandler(ActionType::kUploadFtp,
                                  std::make_shared<FtpUploadHandler>());
    
#ifdef HAVE_ONVIF
    manager.RegisterActionHandler(ActionType::kPublishOnvifEvent,
                                  std::make_shared<OnvifEventHandler>());
#endif
    
    spdlog::info("[ActionHandlerFactory] Registered {} action handlers", 
                 12);  // Approximate count
}

} // namespace events
} // namespace ipcam
