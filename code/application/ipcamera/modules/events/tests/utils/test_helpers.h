/**
 * @file test_helpers.h
 * @brief Test helper functions and utilities
 */

#pragma once

#include <ipcam/event_types.h>
#include <ipcam/event_rule.h>
#include <string>
#include <chrono>
#include <thread>

namespace ipcam {
namespace events {
namespace testing {

/**
 * @brief Create a test motion event
 */
inline Event CreateMotionEvent(const std::vector<uint32_t>& zone_ids = {0}, 
                               float motion_level = 0.75f) {
    Event event = Event::Create(EventCategory::kMotion, EventType::kMotionStart);
    event.source = "motion_detector";
    event.channel = 0;
    
    MotionEventData motion_data;
    motion_data.zone_ids = zone_ids;
    motion_data.motion_level = motion_level;
    motion_data.motion_block_count = 10;
    motion_data.total_blocks = 100;
    event.data = motion_data;
    
    return event;
}

/**
 * @brief Create a test person detected event
 */
inline Event CreatePersonDetectedEvent(float confidence = 0.85f) {
    Event event = Event::Create(EventCategory::kAnalytics, EventType::kPersonDetected);
    event.source = "ai_analytics";
    event.channel = 0;
    
    AnalyticsEventData analytics_data;
    DetectedObject person;
    person.id = 1;
    person.class_name = "person";
    person.confidence = confidence;
    person.x1 = 0.1f; person.y1 = 0.1f;
    person.x2 = 0.3f; person.y2 = 0.5f;
    analytics_data.objects.push_back(person);
    analytics_data.total_persons = 1;
    event.data = analytics_data;
    
    return event;
}

/**
 * @brief Create a test vehicle detected event
 */
inline Event CreateVehicleDetectedEvent(float confidence = 0.90f) {
    Event event = Event::Create(EventCategory::kAnalytics, EventType::kVehicleDetected);
    event.source = "ai_analytics";
    event.channel = 0;
    
    AnalyticsEventData analytics_data;
    DetectedObject vehicle;
    vehicle.id = 2;
    vehicle.class_name = "vehicle";
    vehicle.confidence = confidence;
    vehicle.x1 = 0.2f; vehicle.y1 = 0.4f;
    vehicle.x2 = 0.5f; vehicle.y2 = 0.6f;
    analytics_data.objects.push_back(vehicle);
    analytics_data.total_vehicles = 1;
    event.data = analytics_data;
    
    return event;
}

/**
 * @brief Create a test line crossing event
 */
inline Event CreateLineCrossingEvent(uint32_t line_id = 0, 
                                     const std::string& direction = "left_to_right") {
    Event event = Event::Create(EventCategory::kLineCrossing, EventType::kLineCrossedLeftToRight);
    event.source = "line_crossing";
    event.channel = 0;
    
    LineCrossEventData line_data;
    line_data.line_id = line_id;
    line_data.line_name = "Test Line";
    line_data.direction = direction;
    line_data.object_id = 1;
    line_data.object_class = "person";
    line_data.count_in = 1;
    line_data.count_out = 0;
    event.data = line_data;
    
    return event;
}

/**
 * @brief Create a test zone intrusion event
 */
inline Event CreateZoneIntrusionEvent(uint32_t zone_id = 0) {
    Event event = Event::Create(EventCategory::kIntrusion, EventType::kZoneEntered);
    event.source = "zone_detection";
    event.channel = 0;
    
    IntrusionEventData intrusion_data;
    intrusion_data.zone_id = zone_id;
    intrusion_data.zone_name = "Test Zone";
    intrusion_data.object_id = 1;
    intrusion_data.object_class = "person";
    intrusion_data.dwell_time_ms = 0;
    event.data = intrusion_data;
    
    return event;
}

/**
 * @brief Create a test system event
 */
inline Event CreateSystemEvent(EventType type = EventType::kSystemStartup,
                               const std::string& message = "Test system event") {
    Event event = Event::Create(EventCategory::kSystem, type);
    event.source = "system";
    event.channel = 0;
    
    SystemEventData system_data;
    system_data.message = message;
    system_data.severity = "info";
    system_data.component = "test";
    system_data.error_code = 0;
    event.data = system_data;
    
    return event;
}

/**
 * @brief Create a test event rule
 */
inline EventRule CreateTestRule(const std::string& id = "test-rule",
                                EventType trigger_type = EventType::kMotionStart) {
    EventRule rule = EventRule::Create("Test Rule");
    rule.id = id;
    rule.description = "A test rule for unit testing";
    rule.enabled = true;
    rule.priority = 10;
    
    rule.trigger_events = {trigger_type};
    rule.cooldown_seconds = 5;
    
    return rule;
}

/**
 * @brief Create a recording action
 */
inline Action CreateRecordingAction(int duration = 30) {
    Action action;
    action.type = ActionType::kStartRecording;
    action.name = "Test Recording";
    action.enabled = true;
    action.priority = 10;
    
    RecordingActionConfig config;
    config.duration_seconds = duration;
    config.pre_record_seconds = 5;
    config.post_record_seconds = 10;
    action.config = config;
    
    return action;
}

/**
 * @brief Create a snapshot action
 */
inline Action CreateSnapshotAction(int quality = 85) {
    Action action;
    action.type = ActionType::kCaptureSnapshot;
    action.name = "Test Snapshot";
    action.enabled = true;
    action.priority = 20;
    
    SnapshotActionConfig config;
    config.count = 1;
    config.quality = quality;
    config.format = "jpg";
    action.config = config;
    
    return action;
}

/**
 * @brief Create an email action
 */
inline Action CreateEmailAction() {
    Action action;
    action.type = ActionType::kSendEmail;
    action.name = "Test Email";
    action.enabled = true;
    action.priority = 5;
    
    EmailActionConfig config;
    config.recipients = {"test@example.com"};
    config.subject_template = "Test Alert: {event_type}";
    config.body_template = "Event detected at {timestamp}";
    config.attach_snapshot = true;
    action.config = config;
    
    return action;
}

/**
 * @brief Create a webhook action
 */
inline Action CreateWebhookAction(const std::string& url = "http://localhost:8080/webhook") {
    Action action;
    action.type = ActionType::kSendWebhook;
    action.name = "Test Webhook";
    action.enabled = true;
    action.priority = 5;
    
    WebhookActionConfig config;
    config.url = url;
    config.method = "POST";
    config.body_template = R"({"event": "{event_type}", "time": "{timestamp}"})";
    config.timeout_seconds = 10;
    action.config = config;
    
    return action;
}

/**
 * @brief Wait for a condition with timeout
 */
template<typename Predicate>
bool WaitFor(Predicate pred, std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < timeout) {
        if (pred()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
}

} // namespace testing
} // namespace events
} // namespace ipcam
