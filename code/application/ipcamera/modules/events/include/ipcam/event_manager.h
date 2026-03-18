/**
 * @file event_manager.h
 * @brief Central event dispatcher and rule engine
 * 
 * The EventManager is the core of the event system:
 * - Receives events from all sources (motion, analytics, I/O, etc.)
 * - Matches events against configured rules
 * - Executes actions for matching rules
 * - Maintains event history
 * - Provides real-time event streaming to listeners
 */

#pragma once

#include "event_types.h"
#include "event_rule.h"
#include "action_handler.h"
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <condition_variable>
#include <optional>
#include <deque>

namespace ipcam {
namespace events {

// ============================================================================
// Event Listener Types
// ============================================================================

/**
 * @brief Callback function for event listeners
 */
using EventListener = std::function<void(const Event&)>;

/**
 * @brief Filter function for event listeners (return true to receive event)
 */
using EventFilter = std::function<bool(const Event&)>;

// ============================================================================
// Event Query
// ============================================================================

/**
 * @brief Query parameters for event history
 */
struct EventQuery {
    std::optional<EventCategory> category;
    std::optional<EventType> type;
    std::optional<uint64_t> start_time_ms;
    std::optional<uint64_t> end_time_ms;
    std::vector<uint32_t> zone_ids;
    std::vector<std::string> object_classes;
    int limit = 100;
    int offset = 0;
    bool include_snapshots = false;
};

// ============================================================================
// Event Manager Configuration
// ============================================================================

/**
 * @brief Event manager configuration
 */
struct EventManagerConfig {
    bool enabled = true;
    int max_history_size = 1000;
    int default_cooldown_seconds = 10;
    int queue_max_size = 100;
    int processing_threads = 1;
    
    // Global action settings
    bool snapshot_on_event = true;
    int snapshot_quality = 85;
    bool thumbnail_on_event = true;
    std::string thumbnail_size = "320x180";
    bool log_all_events = true;
};

// ============================================================================
// Event Manager Statistics
// ============================================================================

/**
 * @brief Event manager statistics
 */
struct EventStats {
    uint64_t events_published = 0;
    uint64_t events_processed = 0;
    uint64_t events_filtered = 0;
    uint64_t rules_matched = 0;
    uint64_t actions_executed = 0;
    uint64_t actions_succeeded = 0;
    uint64_t actions_failed = 0;
    
    std::map<EventType, uint64_t> events_by_type;
    std::map<ActionType, uint64_t> actions_by_type;
    std::map<ActionType, uint64_t> action_failures_by_type;
    
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point last_event_time;
};

// ============================================================================
// Event Manager (Singleton)
// ============================================================================

/**
 * @brief Central event manager singleton
 * 
 * Usage:
 * @code
 * // Initialize
 * auto& mgr = EventManager::Instance();
 * mgr.Init();
 * mgr.Start();
 * 
 * // Publish events from detection callbacks
 * mgr.PublishMotionStart({1, 2}, 0.8f);
 * mgr.PublishPersonDetected(person_obj);
 * 
 * // Query history
 * EventQuery query;
 * query.type = EventType::kPersonDetected;
 * query.limit = 50;
 * auto events = mgr.QueryEvents(query);
 * 
 * // Add real-time listener
 * mgr.AddListener([](const Event& e) {
 *     std::cout << "Event: " << e.GetTypeString() << std::endl;
 * });
 * @endcode
 */
class EventManager {
public:
    /**
     * @brief Get singleton instance
     */
    static EventManager& Instance();
    
    // Prevent copying
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;
    
    // ========================================================================
    // Lifecycle
    // ========================================================================
    
    /**
     * @brief Initialize the event manager
     * @return true if successful
     */
    bool Init();
    
    /**
     * @brief Start event processing
     * @return true if started successfully
     */
    bool Start();
    
    /**
     * @brief Stop event processing
     */
    void Stop();
    
    /**
     * @brief Shutdown and release resources
     */
    void Shutdown();
    
    /**
     * @brief Check if running
     */
    bool IsRunning() const { return running_.load(); }
    
    /**
     * @brief Check if initialized
     */
    bool IsInitialized() const { return initialized_.load(); }
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * @brief Load configuration from config system
     */
    bool LoadConfig();
    
    /**
     * @brief Save configuration to config system
     */
    bool SaveConfig();
    
    /**
     * @brief Get current configuration
     */
    EventManagerConfig GetConfig() const;
    
    /**
     * @brief Set configuration
     */
    bool SetConfig(const EventManagerConfig& config);
    
    // ========================================================================
    // Event Publishing (Thread-Safe)
    // ========================================================================
    
    /**
     * @brief Publish a generic event
     * @param event The event to publish
     * @return Event ID
     */
    std::string PublishEvent(const Event& event);
    
    /**
     * @brief Publish motion start event
     * @param zones Zone IDs where motion detected
     * @param level Overall motion level (0.0-1.0)
     */
    void PublishMotionStart(const std::vector<uint32_t>& zones, float level);
    
    /**
     * @brief Publish motion end event
     */
    void PublishMotionEnd();
    
    /**
     * @brief Publish person detected event
     * @param person Detected person object
     */
    void PublishPersonDetected(const DetectedObject& person);
    
    /**
     * @brief Publish person lost event
     * @param track_id Lost track ID
     */
    void PublishPersonLost(uint32_t track_id);
    
    /**
     * @brief Publish vehicle detected event
     * @param vehicle Detected vehicle object
     */
    void PublishVehicleDetected(const DetectedObject& vehicle);
    
    /**
     * @brief Publish face detected event
     * @param face Detected face object
     */
    void PublishFaceDetected(const DetectedObject& face);
    
    /**
     * @brief Publish line crossed event
     * @param line_id Line ID
     * @param line_name Line name
     * @param direction Crossing direction
     * @param object_id Crossing object track ID
     * @param object_class Object class ("person", "vehicle")
     */
    void PublishLineCrossed(uint32_t line_id, const std::string& line_name,
                           const std::string& direction, uint32_t object_id,
                           const std::string& object_class);
    
    /**
     * @brief Publish zone intrusion event
     * @param zone_id Zone ID
     * @param zone_name Zone name
     * @param object_id Intruding object track ID
     * @param object_class Object class
     * @param dwell_time_ms Time in zone (for loitering)
     */
    void PublishZoneIntrusion(uint32_t zone_id, const std::string& zone_name,
                             uint32_t object_id, const std::string& object_class,
                             int dwell_time_ms = 0);
    
    /**
     * @brief Publish system event
     * @param type Event type
     * @param component Component name
     * @param message Event message
     * @param error_code Error code (0 = no error)
     */
    void PublishSystemEvent(EventType type, const std::string& component,
                           const std::string& message, int error_code = 0);
    
    /**
     * @brief Publish tamper detection event
     * @param tamper_type Type string ("defocus", "masking", etc.)
     * @param active True if alarm started, false if cleared
     * @param metric_value Relevant detection metric
     */
    void PublishTamperEvent(const std::string& tamper_type, bool active,
                           float metric_value = 0.0f);
    
    /**
     * @brief Publish license plate recognition event
     * @param plate_text Decoded plate string
     * @param confidence Detection confidence
     * @param x1 Plate bbox left (normalised)
     * @param y1 Plate bbox top (normalised)
     * @param x2 Plate bbox right (normalised)
     * @param y2 Plate bbox bottom (normalised)
     */
    void PublishLprDetected(const std::string& plate_text, float confidence,
                            float x1, float y1, float x2, float y2);
    
    /**
     * @brief Publish audio classification event
     * @param class_id YAMNet class ID
     * @param class_name Human-readable class name
     * @param confidence Detection confidence
     */
    void PublishAudioDetected(int class_id, const std::string& class_name,
                              float confidence);
    
    /**
     * @brief Publish I/O event
     * @param input_id Input ID
     * @param name Input name
     * @param state Current state
     */
    void PublishIOEvent(int input_id, const std::string& name, bool state);
    
    /**
     * @brief Publish storage event
     * @param type Event type
     * @param device Device path
     * @param storage_type Storage type
     */
    void PublishStorageEvent(EventType type, const std::string& device,
                            const std::string& storage_type);
    
    // ========================================================================
    // Event Rules Management
    // ========================================================================
    
    /**
     * @brief Get all event rules
     */
    std::vector<EventRule> GetRules() const;
    
    /**
     * @brief Get rule by ID
     */
    std::optional<EventRule> GetRule(const std::string& rule_id) const;
    
    /**
     * @brief Add a new rule
     * @return true if added successfully
     */
    bool AddRule(const EventRule& rule);
    
    /**
     * @brief Update an existing rule
     * @return true if updated successfully
     */
    bool UpdateRule(const EventRule& rule);
    
    /**
     * @brief Delete a rule
     * @return true if deleted successfully
     */
    bool DeleteRule(const std::string& rule_id);
    
    /**
     * @brief Enable/disable a rule
     */
    bool SetRuleEnabled(const std::string& rule_id, bool enabled);
    
    /**
     * @brief Test a rule with a simulated event
     * @return List of actions that would execute
     */
    std::vector<Action> TestRule(const std::string& rule_id, const Event& test_event);
    
    // ========================================================================
    // Event Listeners
    // ========================================================================
    
    /**
     * @brief Add event listener
     * @param listener Callback function
     * @param filter Optional filter (nullptr = receive all)
     * @return Listener ID for removal
     */
    uint32_t AddListener(EventListener listener, EventFilter filter = nullptr);
    
    /**
     * @brief Remove event listener
     */
    void RemoveListener(uint32_t listener_id);
    
    /**
     * @brief Remove all listeners
     */
    void ClearListeners();
    
    // ========================================================================
    // Event History
    // ========================================================================
    
    /**
     * @brief Query event history
     */
    std::vector<Event> QueryEvents(const EventQuery& query) const;
    
    /**
     * @brief Get event by ID
     */
    std::optional<Event> GetEvent(const std::string& event_id) const;
    
    /**
     * @brief Get event count matching query
     */
    int GetEventCount(const EventQuery& query) const;
    
    /**
     * @brief Clear event history
     */
    void ClearHistory();
    
    /**
     * @brief Get recent events (last N)
     */
    std::vector<Event> GetRecentEvents(int count = 10) const;
    
    // ========================================================================
    // Statistics
    // ========================================================================
    
    /**
     * @brief Get event statistics
     */
    EventStats GetStats() const;
    
    /**
     * @brief Reset statistics
     */
    void ResetStats();
    
    // ========================================================================
    // Action Handlers
    // ========================================================================
    
    /**
     * @brief Get action handler for type
     */
    std::shared_ptr<ActionHandler> GetHandler(ActionType type);
    
    /**
     * @brief Register an action handler for a specific type
     */
    void RegisterActionHandler(ActionType type, std::shared_ptr<ActionHandler> handler);
    
    /**
     * @brief Check if action type is available
     */
    bool IsActionAvailable(ActionType type);
    
private:
    EventManager();
    ~EventManager();
    
    // Processing thread function
    void ProcessingThread();
    
    // Process a single event
    void ProcessEvent(const Event& event);
    
    // Match event against rules
    std::vector<const EventRule*> MatchRules(const Event& event);
    
    // Execute actions for a rule
    void ExecuteActions(const EventRule& rule, Event& event);
    
    // Notify listeners
    void NotifyListeners(const Event& event);
    
    // Add event to history
    void AddToHistory(const Event& event);
    
    // Register all action handlers
    void RegisterActionHandlers();
    
    // Generate snapshot for event if configured
    void GenerateEventSnapshot(Event& event);
    
    // Generate thumbnail for event if configured
    void GenerateEventThumbnail(Event& event);
    
    // State
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    
    // Configuration
    EventManagerConfig config_;
    mutable std::mutex config_mutex_;
    
    // Event queue
    std::queue<Event> event_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    // Processing thread
    std::thread processing_thread_;
    
    // Rules
    std::vector<EventRule> rules_;
    mutable std::mutex rules_mutex_;
    
    // Listeners
    struct ListenerEntry {
        uint32_t id;
        EventListener listener;
        EventFilter filter;
    };
    std::vector<ListenerEntry> listeners_;
    std::mutex listeners_mutex_;
    std::atomic<uint32_t> next_listener_id_{1};
    
    // Action handlers
    std::map<ActionType, std::shared_ptr<ActionHandler>> action_handlers_;
    
    // Event history (ring buffer)
    std::deque<Event> event_history_;
    mutable std::mutex history_mutex_;
    
    // Statistics
    EventStats stats_;
    mutable std::mutex stats_mutex_;
};

} // namespace events
} // namespace ipcam
