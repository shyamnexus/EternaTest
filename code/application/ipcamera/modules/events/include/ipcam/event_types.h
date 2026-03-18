/**
 * @file event_types.h
 * @brief Event type definitions for the Event Actions system
 * 
 * Defines all event categories, types, and data structures used
 * throughout the event system.
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <variant>
#include <optional>
#include <cstdint>

namespace ipcam {
namespace events {

// ============================================================================
// Event Categories
// ============================================================================

/**
 * @brief High-level event categories
 */
enum class EventCategory {
    kMotion,        ///< Motion detection events
    kAnalytics,     ///< AI analytics events (person, vehicle, face)
    kLineCrossing,  ///< Line crossing events
    kIntrusion,     ///< Zone intrusion events
    kSystem,        ///< System events (startup, shutdown, error)
    kIO,            ///< External I/O events (alarm input)
    kStorage,       ///< Storage events (SD card full, NAS disconnect)
    kNetwork,       ///< Network events (connection lost)
    kSchedule       ///< Scheduled events
};

// ============================================================================
// Event Types
// ============================================================================

/**
 * @brief Specific event types within categories
 */
enum class EventType {
    // Motion events
    kMotionStart,
    kMotionEnd,
    kMotionContinuous,
    
    // Analytics events
    kPersonDetected,
    kPersonLost,
    kVehicleDetected,
    kVehicleLost,
    kFaceDetected,
    kFaceLost,
    kAnimalDetected,
    kAnimalLost,
    kObjectTracked,
    kLprDetected,
    kAudioDetected,
    
    // Line crossing
    kLineCrossedLeftToRight,
    kLineCrossedRightToLeft,
    kLineCrossedTopToBottom,
    kLineCrossedBottomToTop,
    kLineCrossedAny,
    
    // Zone intrusion
    kZoneEntered,
    kZoneExited,
    kZoneLoitering,
    
    // System events
    kSystemStartup,
    kSystemShutdown,
    kSystemError,
    kTamperDetected,
    kTamperCleared,
    
    // I/O events
    kAlarmInputTriggered,
    kAlarmInputCleared,
    
    // Storage events
    kStorageMounted,
    kStorageUnmounted,
    kStorageFull,
    kStorageError,
    kRecordingStarted,
    kRecordingStopped,
    kRecordingError,
    
    // Network events
    kNetworkConnected,
    kNetworkDisconnected,
    
    // Unknown
    kUnknown
};

// ============================================================================
// Action Types
// ============================================================================

/**
 * @brief Types of actions that can be triggered by events
 */
enum class ActionType {
    // Recording actions
    kStartRecording,
    kStopRecording,
    kCaptureSnapshot,
    
    // Notification actions
    kSendEmail,
    kSendPushNotification,
    kPublishOnvifEvent,
    
    // Integration actions
    kSendWebhook,
    kPublishMqtt,
    kUploadFtp,
    kUploadCloud,
    
    // I/O actions
    kTriggerAlarmOutput,
    kClearAlarmOutput,
    kActivateLight,
    kDeactivateLight,
    kPlaySiren,
    kStopSiren,
    
    // PTZ actions
    kPtzGotoPreset,
    kPtzStartPatrol,
    
    // Analytics actions
    kLogEvent,
    kGenerateThumbnail,
    kAttachMetadata
};

// ============================================================================
// Detection Object
// ============================================================================

/**
 * @brief Detected object information (for analytics events)
 */
struct DetectedObject {
    uint32_t id = 0;                    ///< Track ID (0 if not tracked)
    std::string class_name;             ///< "person", "vehicle", "face", "animal"
    float confidence = 0.0f;            ///< Detection confidence (0.0-1.0)
    
    /// Bounding box (normalized 0.0 - 1.0)
    float x1 = 0.0f, y1 = 0.0f;
    float x2 = 0.0f, y2 = 0.0f;
    
    /// Pixel coordinates (for specific resolution)
    int pixel_x1 = 0, pixel_y1 = 0;
    int pixel_x2 = 0, pixel_y2 = 0;
    
    /// Width and height helpers
    float Width() const { return x2 - x1; }
    float Height() const { return y2 - y1; }
    float CenterX() const { return (x1 + x2) / 2.0f; }
    float CenterY() const { return (y1 + y2) / 2.0f; }
};

// ============================================================================
// Event Data Structures (payloads for different event types)
// ============================================================================

/**
 * @brief Motion event data
 */
struct MotionEventData {
    std::vector<uint32_t> zone_ids;     ///< Zones where motion detected
    float motion_level = 0.0f;          ///< Overall motion level (0.0 - 1.0)
    int motion_block_count = 0;         ///< Number of motion blocks
    int total_blocks = 0;               ///< Total blocks in frame
};

/**
 * @brief Analytics event data (person/vehicle/face detection)
 */
struct AnalyticsEventData {
    std::vector<DetectedObject> objects;  ///< Detected objects
    int total_persons = 0;
    int total_vehicles = 0;
    int total_faces = 0;
    int total_animals = 0;
};

/**
 * @brief Line crossing event data
 */
struct LineCrossEventData {
    uint32_t line_id = 0;               ///< Line ID
    std::string line_name;              ///< Line name
    uint32_t object_id = 0;             ///< Object that crossed
    std::string object_class;           ///< "person", "vehicle", etc.
    std::string direction;              ///< "left_to_right", "right_to_left", etc.
    int count_in = 0;                   ///< Total count entering
    int count_out = 0;                  ///< Total count exiting
};

/**
 * @brief Zone intrusion event data
 */
struct IntrusionEventData {
    uint32_t zone_id = 0;               ///< Zone ID
    std::string zone_name;              ///< Zone name
    uint32_t object_id = 0;             ///< Object in zone
    std::string object_class;           ///< "person", "vehicle", etc.
    int dwell_time_ms = 0;              ///< Time in zone (for loitering)
};

/**
 * @brief System event data
 */
struct SystemEventData {
    std::string component;              ///< Component name
    std::string message;                ///< Event message
    int error_code = 0;                 ///< Error code (if applicable)
    std::string severity;               ///< "info", "warning", "error", "critical"
};

/**
 * @brief I/O event data
 */
struct IOEventData {
    int input_id = 0;                   ///< Input/output ID
    std::string name;                   ///< Input/output name
    bool state = false;                 ///< Current state
    bool previous_state = false;        ///< Previous state
};

/**
 * @brief Storage event data
 */
struct StorageEventData {
    std::string device;                 ///< Device path (/mnt/sd, etc.)
    std::string type;                   ///< "sdcard", "nas", "usb"
    int64_t total_bytes = 0;
    int64_t free_bytes = 0;
    int usage_percent = 0;
    std::string error_message;
};

/**
 * @brief Network event data
 */
struct NetworkEventData {
    std::string interface;              ///< "eth0", "wlan0", etc.
    std::string ip_address;
    std::string mac_address;
    std::string connection_type;        ///< "wired", "wireless"
};

/**
 * @brief Tamper detection event data
 */
struct TamperEventData {
    std::string tamper_type;             ///< "defocus", "masking", "scene_change", "too_dark", "too_bright"
    bool active = false;                 ///< Alarm active or cleared
    float metric_value = 0.0f;           ///< Relevant detection metric
};

/**
 * @brief License plate recognition event data
 */
struct LprEventData {
    std::string plate_text;              ///< Decoded plate string
    float confidence = 0.0f;
    float x1 = 0, y1 = 0, x2 = 0, y2 = 0;  ///< Plate bounding box (normalised)
};

/**
 * @brief Audio classification event data
 */
struct AudioEventData {
    int class_id = 0;
    std::string class_name;              ///< e.g. "glass_breaking", "gunshot"
    float confidence = 0.0f;
};

/**
 * @brief Variant type for event data payload
 */
using EventData = std::variant<
    MotionEventData,
    AnalyticsEventData,
    LineCrossEventData,
    IntrusionEventData,
    SystemEventData,
    IOEventData,
    StorageEventData,
    NetworkEventData,
    TamperEventData,
    LprEventData,
    AudioEventData
>;

// ============================================================================
// Event Structure
// ============================================================================

/**
 * @brief Main event structure
 */
struct Event {
    /// Identification
    std::string id;                                 ///< UUID
    EventCategory category = EventCategory::kSystem;
    EventType type = EventType::kUnknown;
    
    /// Timing
    std::chrono::system_clock::time_point timestamp;
    uint64_t timestamp_ms = 0;                      ///< Unix timestamp in ms
    
    /// Source information
    int channel = 0;                                ///< Camera channel (0 = main)
    std::string source;                             ///< Source module name
    
    /// Data payload
    EventData data;
    
    /// Associated media (set by action handlers)
    std::optional<std::string> snapshot_path;       ///< Path to snapshot
    std::optional<std::string> video_path;          ///< Path to video clip
    std::optional<std::string> thumbnail_path;      ///< Path to thumbnail
    
    /// Constructor
    Event();
    
    /// Generate new event with UUID
    static Event Create(EventCategory cat, EventType type);
    
    /// Helper methods
    std::string GetTypeString() const;
    std::string GetCategoryString() const;
    std::string ToJson() const;
    std::string ToIsoTimestamp() const;
    
    /// Parse from JSON
    static Event FromJson(const std::string& json);
};

// ============================================================================
// Conversion Functions
// ============================================================================

std::string EventTypeToString(EventType type);
EventType StringToEventType(const std::string& str);

std::string EventCategoryToString(EventCategory cat);
EventCategory StringToEventCategory(const std::string& str);

std::string ActionTypeToString(ActionType type);
ActionType StringToActionType(const std::string& str);

} // namespace events
} // namespace ipcam
