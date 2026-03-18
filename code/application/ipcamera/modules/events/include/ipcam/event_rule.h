/**
 * @file event_rule.h
 * @brief Event rule definitions for the rule engine
 * 
 * Defines rules that determine which actions to execute when events occur.
 */

#pragma once

#include "event_types.h"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <chrono>

namespace ipcam {
namespace events {

// ============================================================================
// Action Configuration Structures
// ============================================================================

/**
 * @brief Recording action configuration
 */
struct RecordingActionConfig {
    int duration_seconds = 30;          ///< Recording duration
    int pre_record_seconds = 5;         ///< Pre-event buffer
    int post_record_seconds = 10;       ///< Post-event continuation
    std::vector<int> channels = {0};    ///< Channels to record
    std::string quality = "main";       ///< "main", "sub"
};

/**
 * @brief Snapshot action configuration
 */
struct SnapshotActionConfig {
    int count = 1;                      ///< Number of snapshots
    int interval_ms = 1000;             ///< Interval between snapshots
    std::string format = "jpg";         ///< "jpg", "png"
    int quality = 85;                   ///< JPEG quality (1-100)
    bool include_overlay = false;       ///< Include OSD/detection boxes
    bool include_timestamp = true;      ///< Burn timestamp into image
    std::string save_path;              ///< Custom save path (empty = default)
};

/**
 * @brief Email action configuration
 */
struct EmailActionConfig {
    std::vector<std::string> recipients;                ///< Email addresses
    std::string subject_template = "[{device_name}] {event_type} detected";
    std::string body_template = "Event: {event_type}\nTime: {timestamp}\nZone: {zone_name}";
    bool attach_snapshot = true;
    bool attach_video = false;
    int max_attachment_size_mb = 10;
};

/**
 * @brief Webhook action configuration
 */
struct WebhookActionConfig {
    std::string url;                                    ///< Webhook URL
    std::string method = "POST";                        ///< HTTP method
    std::map<std::string, std::string> headers;         ///< Custom headers
    std::string body_template;                          ///< JSON body template
    int timeout_seconds = 10;
    int retry_count = 3;
    int retry_delay_seconds = 5;
    bool include_snapshot_base64 = false;               ///< Include snapshot in payload
    bool verify_ssl = true;
};

/**
 * @brief MQTT action configuration
 */
struct MqttActionConfig {
    std::string topic;                                  ///< MQTT topic
    int qos = 1;                                        ///< QoS level (0, 1, 2)
    bool retain = false;                                ///< Retain message
    std::string payload_template;                       ///< JSON payload template
};

/**
 * @brief FTP upload action configuration
 */
struct FtpActionConfig {
    std::string remote_path_template = "/events/{date}/{event_type}/";
    bool upload_snapshot = true;
    bool upload_video = true;
    bool delete_local_after_upload = false;
    int max_retries = 3;
};

/**
 * @brief Cloud upload action configuration
 */
struct CloudActionConfig {
    std::string provider;               ///< "aws_s3", "azure_blob", "google_cloud"
    std::string bucket;
    std::string prefix_template = "events/{year}/{month}/{day}/";
    bool upload_snapshot = true;
    bool upload_video = true;
};

/**
 * @brief I/O output action configuration
 */
struct IoActionConfig {
    int output_id = 0;                  ///< GPIO output ID
    bool state = true;                  ///< State to set
    int duration_ms = 5000;             ///< Duration (0 = toggle, >0 = pulse)
};

/**
 * @brief Light/siren action configuration
 */
struct LightSirenActionConfig {
    bool activate = true;
    int duration_ms = 5000;
    int intensity = 100;                ///< 0-100 for lights
    std::string pattern = "solid";      ///< "solid", "flash", "strobe"
};

/**
 * @brief PTZ action configuration
 */
struct PtzActionConfig {
    int preset_id = 0;                  ///< Preset position to go to
    std::string patrol_name;            ///< Patrol name to start
    bool return_after = false;          ///< Return to previous position
    int return_delay_seconds = 30;
};

/**
 * @brief ONVIF event action configuration
 */
struct OnvifActionConfig {
    std::string topic_override;         ///< Override default topic
    std::map<std::string, std::string> custom_data;
};

/**
 * @brief Log event action configuration
 */
struct LogActionConfig {
    bool save_to_database = true;
    bool save_to_file = false;
    std::string log_file_path;
    bool include_metadata = true;
};

/**
 * @brief Variant type for action configuration
 */
using ActionConfig = std::variant<
    RecordingActionConfig,
    SnapshotActionConfig,
    EmailActionConfig,
    WebhookActionConfig,
    MqttActionConfig,
    FtpActionConfig,
    CloudActionConfig,
    IoActionConfig,
    LightSirenActionConfig,
    PtzActionConfig,
    OnvifActionConfig,
    LogActionConfig
>;

// ============================================================================
// Action Definition
// ============================================================================

/**
 * @brief Single action to execute when rule triggers
 */
struct Action {
    ActionType type = ActionType::kLogEvent;
    bool enabled = true;
    ActionConfig config;
    int priority = 0;                   ///< Higher priority executes first
    int delay_ms = 0;                   ///< Delay before executing
    std::string name;                   ///< Optional display name
    
    /// Helper to get typed config
    template<typename T>
    const T* GetConfig() const {
        return std::get_if<T>(&config);
    }
};

// ============================================================================
// Schedule Definition
// ============================================================================

/**
 * @brief Time range for schedule
 */
struct TimeRange {
    int start_hour = 0;
    int start_minute = 0;
    int end_hour = 23;
    int end_minute = 59;
    
    bool Contains(int hour, int minute) const;
    std::string ToString() const;
    static TimeRange FromString(const std::string& str);
};

/**
 * @brief Schedule for when rule is active
 */
struct Schedule {
    bool enabled = false;               ///< If false, rule is always active
    std::vector<int> days_of_week;      ///< 0=Sunday, 6=Saturday (empty = all)
    std::vector<TimeRange> time_ranges; ///< Time ranges (empty = all day)
    bool invert = false;                ///< If true, active OUTSIDE schedule
    
    bool IsActiveNow() const;
    bool IsActiveAt(const std::chrono::system_clock::time_point& time) const;
};

// ============================================================================
// Event Rule
// ============================================================================

/**
 * @brief Event rule - defines what actions to take when events occur
 */
struct EventRule {
    /// Identification
    std::string id;                             ///< UUID
    std::string name;                           ///< Display name
    std::string description;                    ///< Optional description
    bool enabled = true;
    int priority = 0;                           ///< Higher priority rules evaluated first
    
    /// Trigger conditions
    std::vector<EventType> trigger_events;      ///< Events that trigger this rule
    std::vector<uint32_t> trigger_zones;        ///< Zones (empty = all)
    std::vector<uint32_t> trigger_lines;        ///< Lines (empty = all)
    std::vector<std::string> trigger_classes;   ///< Object classes (empty = all)
    float min_confidence = 0.0f;                ///< Minimum detection confidence
    int min_objects = 1;                        ///< Minimum number of objects
    
    /// Cooldown (prevent rapid re-triggering)
    int cooldown_seconds = 10;
    
    /// Schedule
    Schedule schedule;
    
    /// Actions to execute when rule triggers
    std::vector<Action> actions;
    
    /// Runtime state (not persisted)
    mutable std::chrono::system_clock::time_point last_triggered;
    mutable int trigger_count = 0;
    
    /// Check if rule matches an event
    bool Matches(const Event& event) const;
    
    /// Check if rule is currently active (schedule check)
    bool IsActiveNow() const;
    
    /// Check if cooldown has elapsed
    bool IsCooledDown() const;
    
    /// Update last triggered time
    void MarkTriggered() const;
    
    /// Serialize to JSON
    std::string ToJson() const;
    
    /// Parse from JSON
    static EventRule FromJson(const std::string& json);
    
    /// Create a new rule with generated UUID
    static EventRule Create(const std::string& name);
};

// ============================================================================
// Rule Matching Helpers
// ============================================================================

/**
 * @brief Check if event type is in the trigger list
 */
bool EventTypeMatches(EventType event_type, const std::vector<EventType>& triggers);

/**
 * @brief Check if zone matches filter
 */
bool ZoneMatches(uint32_t zone_id, const std::vector<uint32_t>& filter);

/**
 * @brief Check if object class matches filter
 */
bool ClassMatches(const std::string& class_name, const std::vector<std::string>& filter);

} // namespace events
} // namespace ipcam
