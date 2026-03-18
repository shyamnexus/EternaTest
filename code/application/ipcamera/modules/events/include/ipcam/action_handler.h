/**
 * @file action_handler.h
 * @brief Base class and interface for action handlers
 */

#pragma once

#include "event_types.h"
#include "event_rule.h"
#include <string>
#include <future>
#include <memory>
#include <map>

namespace ipcam {
namespace events {

// ============================================================================
// Action Result
// ============================================================================

/**
 * @brief Result of executing an action
 */
struct ActionResult {
    bool success = false;               ///< Whether action succeeded
    std::string error_message;          ///< Error message if failed
    std::string output;                 ///< Output data (file path, response, etc.)
    int64_t execution_time_ms = 0;      ///< Time taken to execute
    std::map<std::string, std::string> metadata;  ///< Additional metadata
};

// ============================================================================
// Action Handler Base Class
// ============================================================================

/**
 * @brief Base class for all action handlers
 * 
 * Each action type (recording, email, webhook, etc.) has a handler
 * that implements this interface.
 */
class ActionHandler {
public:
    virtual ~ActionHandler() = default;
    
    /**
     * @brief Get the action type this handler supports
     */
    virtual ActionType GetType() const = 0;
    
    /**
     * @brief Get handler name for logging
     */
    virtual std::string GetName() const = 0;
    
    /**
     * @brief Execute the action synchronously
     * @param event The triggering event
     * @param action The action configuration
     * @return Action result
     */
    virtual ActionResult Execute(const Event& event, const Action& action) = 0;
    
    /**
     * @brief Execute action asynchronously
     * @param event The triggering event
     * @param action The action configuration
     * @return Future with action result
     */
    std::future<ActionResult> ExecuteAsync(const Event& event, const Action& action);
    
    /**
     * @brief Check if handler is available/configured
     * 
     * For example, email handler checks if SMTP is configured.
     */
    virtual bool IsAvailable() const { return true; }
    
    /**
     * @brief Get handler status information
     */
    virtual std::string GetStatus() const { return "OK"; }
    
    /**
     * @brief Initialize the handler (called once at startup)
     */
    virtual bool Init() { return true; }
    
    /**
     * @brief Shutdown the handler (called at exit)
     */
    virtual void Shutdown() {}
    
protected:
    /**
     * @brief Substitute placeholders in template strings
     * 
     * Supported placeholders:
     * - {device_name} - Camera device name
     * - {device_id} - Camera device ID
     * - {event_type} - Event type string
     * - {event_id} - Event UUID
     * - {timestamp} - ISO timestamp
     * - {date} - Date (YYYY-MM-DD)
     * - {time} - Time (HH:MM:SS)
     * - {year}, {month}, {day}, {hour}, {minute}, {second}
     * - {zone_id}, {zone_name} - Zone information
     * - {line_id}, {line_name} - Line information
     * - {object_class} - Detected object class
     * - {confidence} - Detection confidence
     * - {objects_json} - JSON array of detected objects
     * - {snapshot_path} - Path to snapshot (if captured)
     * - {video_path} - Path to video (if recorded)
     */
    std::string SubstitutePlaceholders(const std::string& template_str, const Event& event);
    
    /**
     * @brief Get device information for placeholders
     */
    std::string GetDeviceName() const;
    std::string GetDeviceId() const;
};

// ============================================================================
// Action Handler Factory
// ============================================================================

// Forward declaration
class EventManager;

/**
 * @brief Factory for creating action handlers
 */
class ActionHandlerFactory {
public:
    /**
     * @brief Create a handler for the specified action type
     */
    static std::shared_ptr<ActionHandler> Create(ActionType type);
    
    /**
     * @brief Create all available handlers
     */
    static std::map<ActionType, std::shared_ptr<ActionHandler>> CreateAll();
    
    /**
     * @brief Register all handlers with an EventManager instance
     */
    static void RegisterAll(EventManager& manager);
};

// ============================================================================
// Concrete Action Handler Declarations
// ============================================================================

/**
 * @brief Handler for recording actions
 */
class RecordingActionHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kStartRecording; }
    std::string GetName() const override { return "RecordingAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
    bool IsAvailable() const override;
};

/**
 * @brief Handler for snapshot capture
 */
class SnapshotActionHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kCaptureSnapshot; }
    std::string GetName() const override { return "SnapshotAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
    bool IsAvailable() const override;
};

/**
 * @brief Handler for email notifications
 */
class EmailActionHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kSendEmail; }
    std::string GetName() const override { return "EmailAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
    bool IsAvailable() const override;
    std::string GetStatus() const override;
};

/**
 * @brief Handler for webhook HTTP requests
 */
class WebhookActionHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kSendWebhook; }
    std::string GetName() const override { return "WebhookAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
};

/**
 * @brief Handler for MQTT publishing
 */
class MqttActionHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kPublishMqtt; }
    std::string GetName() const override { return "MqttAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
    bool IsAvailable() const override;
    bool Init() override;
    void Shutdown() override;
    
private:
    bool connected_ = false;
};

/**
 * @brief Handler for ONVIF event publishing
 */
class OnvifEventHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kPublishOnvifEvent; }
    std::string GetName() const override { return "OnvifEventAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
};

/**
 * @brief Handler for FTP uploads
 */
class FtpUploadHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kUploadFtp; }
    std::string GetName() const override { return "FtpUploadAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
    bool IsAvailable() const override;
};

/**
 * @brief Handler for cloud storage uploads
 */
class CloudUploadHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kUploadCloud; }
    std::string GetName() const override { return "CloudUploadAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
    bool IsAvailable() const override;
};

/**
 * @brief Handler for I/O alarm outputs
 */
class IoOutputHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kTriggerAlarmOutput; }
    std::string GetName() const override { return "IoOutputAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
    bool IsAvailable() const override;
};

/**
 * @brief Handler for white light activation
 */
class LightActionHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kActivateLight; }
    std::string GetName() const override { return "LightAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
    bool IsAvailable() const override;
};

/**
 * @brief Handler for siren/alarm audio
 */
class SirenActionHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kPlaySiren; }
    std::string GetName() const override { return "SirenAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
    bool IsAvailable() const override;
};

/**
 * @brief Handler for event logging
 */
class LogEventHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kLogEvent; }
    std::string GetName() const override { return "LogEventAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
};

/**
 * @brief Handler for thumbnail generation
 */
class ThumbnailHandler : public ActionHandler {
public:
    ActionType GetType() const override { return ActionType::kGenerateThumbnail; }
    std::string GetName() const override { return "ThumbnailAction"; }
    ActionResult Execute(const Event& event, const Action& action) override;
};

} // namespace events
} // namespace ipcam
