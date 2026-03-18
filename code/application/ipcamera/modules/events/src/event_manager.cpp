/**
 * @file event_manager.cpp
 * @brief Central event manager implementation using config module APIs
 */

#include "ipcam/event_manager.h"
#include "ipcam/config.h"
#include "ipcam/video_control.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

namespace ipcam {
namespace events {

// ============================================================================
// EventManager Implementation
// ============================================================================

EventManager& EventManager::Instance() {
    static EventManager instance;
    return instance;
}

EventManager::EventManager() {
    stats_.start_time = std::chrono::system_clock::now();
}

EventManager::~EventManager() {
    Shutdown();
}

bool EventManager::Init() {
    if (initialized_.load()) {
        return true;
    }
    
    spdlog::info("[EventManager] Initializing from config module");
    
    // Load configuration from config module
    if (!LoadConfig()) {
        spdlog::warn("[EventManager] Failed to load config, using defaults");
        // Use defaults from the struct definition
    }
    
    // Register action handlers
    RegisterActionHandlers();
    
    initialized_ = true;
    return true;
}

bool EventManager::Start() {
    if (!initialized_.load()) {
        if (!Init()) {
            return false;
        }
    }
    
    if (running_.load()) {
        return true;
    }
    
    running_ = true;
    
    // Start processing thread
    processing_thread_ = std::thread([this]() {
        spdlog::info("[EventManager] Processing thread started");
        ProcessingThread();
    });
    
    spdlog::info("[EventManager] Started");
    return true;
}

void EventManager::Stop() {
    if (!running_.load()) {
        return;
    }
    
    running_ = false;
    queue_cv_.notify_all();
    
    if (processing_thread_.joinable()) {
        processing_thread_.join();
    }
    
    spdlog::info("[EventManager] Stopped");
}

void EventManager::Shutdown() {
    Stop();
    initialized_ = false;
}

bool EventManager::LoadConfig() {
    try {
        std::lock_guard<std::mutex> lock(config_mutex_);
        
        // Load global settings from config module
        config_.enabled = config::Get<bool>("events.config.enabled", true);
        config_.max_history_size = config::Get<int>("events.config.max_history_size", 1000);
        config_.default_cooldown_seconds = config::Get<int>("events.config.default_cooldown_seconds", 10);
        config_.queue_max_size = config::Get<int>("events.config.queue_max_size", 100);
        config_.processing_threads = config::Get<int>("events.config.processing_threads", 1);
        config_.snapshot_on_event = config::Get<bool>("events.config.snapshot_on_event", true);
        config_.snapshot_quality = config::Get<int>("events.config.snapshot_quality", 85);
        config_.thumbnail_on_event = config::Get<bool>("events.config.thumbnail_on_event", true);
        config_.thumbnail_size = config::Get<std::string>("events.config.thumbnail_size", "320x180");
        config_.log_all_events = config::Get<bool>("events.config.log_all_events", true);
        
        // Load rules from config module as JSON string
        std::string rules_json = config::Get<std::string>("events.rules", "[]");
        
        {
            std::lock_guard<std::mutex> rules_lock(rules_mutex_);
            rules_.clear();
            
            auto rules_arr = json::parse(rules_json);
            if (rules_arr.is_array()) {
                for (const auto& rule_j : rules_arr) {
                    EventRule rule = EventRule::FromJson(rule_j);
                    rules_.push_back(std::move(rule));
                }
                
                // Sort by priority (higher priority first)
                std::sort(rules_.begin(), rules_.end(), [](const EventRule& a, const EventRule& b) {
                    return a.priority > b.priority;
                });
            }
            
            spdlog::info("[EventManager] Loaded {} rules from config", rules_.size());
        }
        
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("[EventManager] Error loading config: {}", e.what());
        return false;
    }
}

bool EventManager::SaveConfig() {
    try {
        // Note: Caller must hold config_mutex_ if accessing config_
        // We only need to lock rules_mutex_ here for rules serialization
        
        // Save global settings to config module
        config::Set("events.config.enabled", config_.enabled);
        config::Set("events.config.max_history_size", config_.max_history_size);
        config::Set("events.config.default_cooldown_seconds", config_.default_cooldown_seconds);
        config::Set("events.config.queue_max_size", config_.queue_max_size);
        config::Set("events.config.processing_threads", config_.processing_threads);
        config::Set("events.config.snapshot_on_event", config_.snapshot_on_event);
        config::Set("events.config.snapshot_quality", config_.snapshot_quality);
        config::Set("events.config.thumbnail_on_event", config_.thumbnail_on_event);
        config::Set("events.config.thumbnail_size", config_.thumbnail_size);
        config::Set("events.config.log_all_events", config_.log_all_events);
        
        // Save rules to config module as JSON string
        {
            std::lock_guard<std::mutex> rules_lock(rules_mutex_);
            json rules_arr = json::array();
            for (const auto& rule : rules_) {
                rules_arr.push_back(rule.ToJson());
            }
            config::Set("events.rules", rules_arr.dump());
        }
        
        // Persist to disk
        config::Save();
        
        spdlog::info("[EventManager] Configuration saved");
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("[EventManager] Error saving config: {}", e.what());
        return false;
    }
}

EventManagerConfig EventManager::GetConfig() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_;
}

bool EventManager::SetConfig(const EventManagerConfig& new_config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_ = new_config;
    return SaveConfig();
}

std::string EventManager::PublishEvent(const Event& event) {
    if (!running_.load()) {
        spdlog::warn("[EventManager] Cannot publish event - manager not running");
        return "";
    }
    
    // Queue event for processing
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        
        if (static_cast<int>(event_queue_.size()) >= config_.queue_max_size) {
            spdlog::warn("[EventManager] Event queue full, dropping event");
            return "";
        }
        
        event_queue_.push(event);
    }
    
    queue_cv_.notify_one();
    
    // Update stats
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.events_published++;
        stats_.events_by_type[event.type]++;
        stats_.last_event_time = std::chrono::system_clock::now();
    }
    
    // Log event
    if (config_.log_all_events) {
        spdlog::info("[EventManager] Event published: {} ({})", 
                     EventTypeToString(event.type), event.id);
    }
    
    return event.id;
}

// ============================================================================
// Convenience Methods for Common Events
// ============================================================================

void EventManager::PublishMotionStart(const std::vector<uint32_t>& zones, float level) {
    Event event = Event::Create(EventCategory::kMotion, EventType::kMotionStart);
    event.source = "motion_detector";
    
    MotionEventData data;
    data.zone_ids = zones;
    data.motion_level = level;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishMotionEnd() {
    Event event = Event::Create(EventCategory::kMotion, EventType::kMotionEnd);
    event.source = "motion_detector";
    
    MotionEventData data;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishPersonDetected(const DetectedObject& person) {
    Event event = Event::Create(EventCategory::kAnalytics, EventType::kPersonDetected);
    event.source = "ai_analytics";
    
    AnalyticsEventData data;
    data.objects.push_back(person);
    data.total_persons = 1;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishPersonLost(uint32_t track_id) {
    Event event = Event::Create(EventCategory::kAnalytics, EventType::kPersonLost);
    event.source = "ai_analytics";
    
    AnalyticsEventData data;
    DetectedObject obj;
    obj.id = track_id;
    obj.class_name = "person";
    data.objects.push_back(obj);
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishVehicleDetected(const DetectedObject& vehicle) {
    Event event = Event::Create(EventCategory::kAnalytics, EventType::kVehicleDetected);
    event.source = "ai_analytics";
    
    AnalyticsEventData data;
    data.objects.push_back(vehicle);
    data.total_vehicles = 1;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishFaceDetected(const DetectedObject& face) {
    Event event = Event::Create(EventCategory::kAnalytics, EventType::kFaceDetected);
    event.source = "ai_analytics";
    
    AnalyticsEventData data;
    data.objects.push_back(face);
    data.total_faces = 1;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishLineCrossed(uint32_t line_id, const std::string& line_name,
                                      const std::string& direction, uint32_t object_id,
                                      const std::string& object_class) {
    EventType type = EventType::kLineCrossedAny;
    if (direction == "left_to_right") type = EventType::kLineCrossedLeftToRight;
    else if (direction == "right_to_left") type = EventType::kLineCrossedRightToLeft;
    else if (direction == "top_to_bottom") type = EventType::kLineCrossedTopToBottom;
    else if (direction == "bottom_to_top") type = EventType::kLineCrossedBottomToTop;
    
    Event event = Event::Create(EventCategory::kLineCrossing, type);
    event.source = "line_crossing_detector";
    
    LineCrossEventData data;
    data.line_id = line_id;
    data.line_name = line_name;
    data.direction = direction;
    data.object_id = object_id;
    data.object_class = object_class;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishZoneIntrusion(uint32_t zone_id, const std::string& zone_name,
                                        uint32_t object_id, const std::string& object_class,
                                        int dwell_time_ms) {
    EventType type = (dwell_time_ms > 0) ? EventType::kZoneLoitering : EventType::kZoneEntered;
    
    Event event = Event::Create(EventCategory::kIntrusion, type);
    event.source = "zone_intrusion_detector";
    
    IntrusionEventData data;
    data.zone_id = zone_id;
    data.zone_name = zone_name;
    data.object_id = object_id;
    data.object_class = object_class;
    data.dwell_time_ms = dwell_time_ms;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishSystemEvent(EventType type, const std::string& component,
                                      const std::string& message, int error_code) {
    Event event = Event::Create(EventCategory::kSystem, type);
    event.source = "system";
    
    SystemEventData data;
    data.component = component;
    data.message = message;
    data.error_code = error_code;
    // Set severity based on event type
    if (type == EventType::kSystemError) {
        data.severity = "error";
    } else if (type == EventType::kSystemStartup || type == EventType::kSystemShutdown) {
        data.severity = "info";
    } else {
        data.severity = "info";
    }
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishTamperEvent(const std::string& tamper_type, bool active,
                                      float metric_value) {
    EventType type = active ? EventType::kTamperDetected : EventType::kTamperCleared;
    Event event = Event::Create(EventCategory::kSystem, type);
    event.source = "tamper_detector";
    
    TamperEventData data;
    data.tamper_type = tamper_type;
    data.active = active;
    data.metric_value = metric_value;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishLprDetected(const std::string& plate_text, float confidence,
                                      float x1, float y1, float x2, float y2) {
    Event event = Event::Create(EventCategory::kAnalytics, EventType::kLprDetected);
    event.source = "lpr_engine";
    
    LprEventData data;
    data.plate_text = plate_text;
    data.confidence = confidence;
    data.x1 = x1;
    data.y1 = y1;
    data.x2 = x2;
    data.y2 = y2;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishAudioDetected(int class_id, const std::string& class_name,
                                        float confidence) {
    Event event = Event::Create(EventCategory::kAnalytics, EventType::kAudioDetected);
    event.source = "audio_classification";
    
    AudioEventData data;
    data.class_id = class_id;
    data.class_name = class_name;
    data.confidence = confidence;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishIOEvent(int input_id, const std::string& name, bool state) {
    Event event = Event::Create(EventCategory::kIO, state ? EventType::kAlarmInputTriggered : EventType::kAlarmInputCleared);
    event.source = "io_controller";
    
    IOEventData data;
    data.input_id = input_id;
    data.name = name;
    data.state = state;
    event.data = data;
    
    PublishEvent(event);
}

void EventManager::PublishStorageEvent(EventType type, const std::string& device,
                                       const std::string& storage_type) {
    Event event = Event::Create(EventCategory::kStorage, type);
    event.source = "storage_manager";
    
    StorageEventData data;
    data.device = device;
    data.type = storage_type;
    event.data = data;
    
    PublishEvent(event);
}

// ============================================================================
// Rule Management
// ============================================================================

std::vector<EventRule> EventManager::GetRules() const {
    std::lock_guard<std::mutex> lock(rules_mutex_);
    return rules_;
}

std::optional<EventRule> EventManager::GetRule(const std::string& rule_id) const {
    std::lock_guard<std::mutex> lock(rules_mutex_);
    
    auto it = std::find_if(rules_.begin(), rules_.end(),
        [&rule_id](const EventRule& r) { return r.id == rule_id; });
    
    if (it != rules_.end()) {
        return *it;
    }
    return std::nullopt;
}

bool EventManager::AddRule(const EventRule& rule) {
    std::lock_guard<std::mutex> lock(rules_mutex_);
    
    // Check for duplicate ID
    auto it = std::find_if(rules_.begin(), rules_.end(),
        [&rule](const EventRule& r) { return r.id == rule.id; });
    
    if (it != rules_.end()) {
        spdlog::warn("[EventManager] Rule with ID {} already exists", rule.id);
        return false;
    }
    
    rules_.push_back(rule);
    
    // Re-sort by priority
    std::sort(rules_.begin(), rules_.end(), [](const EventRule& a, const EventRule& b) {
        return a.priority > b.priority;
    });
    
    spdlog::info("[EventManager] Added rule: {} ({})", rule.name, rule.id);
    return true;
}

bool EventManager::UpdateRule(const EventRule& rule) {
    std::lock_guard<std::mutex> lock(rules_mutex_);
    
    auto it = std::find_if(rules_.begin(), rules_.end(),
        [&rule](const EventRule& r) { return r.id == rule.id; });
    
    if (it == rules_.end()) {
        spdlog::warn("[EventManager] Rule with ID {} not found", rule.id);
        return false;
    }
    
    *it = rule;
    
    // Re-sort by priority
    std::sort(rules_.begin(), rules_.end(), [](const EventRule& a, const EventRule& b) {
        return a.priority > b.priority;
    });
    
    spdlog::info("[EventManager] Updated rule: {} ({})", rule.name, rule.id);
    return true;
}

bool EventManager::DeleteRule(const std::string& rule_id) {
    std::lock_guard<std::mutex> lock(rules_mutex_);
    
    auto it = std::find_if(rules_.begin(), rules_.end(),
        [&rule_id](const EventRule& r) { return r.id == rule_id; });
    
    if (it == rules_.end()) {
        spdlog::warn("[EventManager] Rule with ID {} not found", rule_id);
        return false;
    }
    
    rules_.erase(it);
    spdlog::info("[EventManager] Deleted rule: {}", rule_id);
    return true;
}

bool EventManager::SetRuleEnabled(const std::string& rule_id, bool enabled) {
    std::lock_guard<std::mutex> lock(rules_mutex_);
    
    auto it = std::find_if(rules_.begin(), rules_.end(),
        [&rule_id](const EventRule& r) { return r.id == rule_id; });
    
    if (it == rules_.end()) {
        return false;
    }
    
    it->enabled = enabled;
    return true;
}

std::vector<Action> EventManager::TestRule(const std::string& rule_id, const Event& test_event) {
    std::vector<Action> actions;
    
    std::lock_guard<std::mutex> lock(rules_mutex_);
    
    auto it = std::find_if(rules_.begin(), rules_.end(),
        [&rule_id](const EventRule& r) { return r.id == rule_id; });
    
    if (it != rules_.end() && it->Matches(test_event)) {
        actions = it->actions;
    }
    
    return actions;
}
// ============================================================================
// Listeners
// ============================================================================

uint32_t EventManager::AddListener(EventListener listener, EventFilter filter) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    
    uint32_t id = next_listener_id_++;
    listeners_.push_back({id, std::move(listener), std::move(filter)});
    
    spdlog::debug("[EventManager] Added listener: {}", id);
    return id;
}

void EventManager::RemoveListener(uint32_t listener_id) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    
    listeners_.erase(
        std::remove_if(listeners_.begin(), listeners_.end(),
            [listener_id](const ListenerEntry& entry) { return entry.id == listener_id; }),
        listeners_.end()
    );
    
    spdlog::debug("[EventManager] Removed listener: {}", listener_id);
}

void EventManager::ClearListeners() {
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    listeners_.clear();
}

// ============================================================================
// Event History
// ============================================================================

std::vector<Event> EventManager::QueryEvents(const EventQuery& query) const {
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    std::vector<Event> results;
    int skipped = 0;
    
    for (const auto& event : event_history_) {
        // Apply filters
        if (query.category.has_value() && event.category != query.category.value()) {
            continue;
        }
        
        if (query.type.has_value() && event.type != query.type.value()) {
            continue;
        }
        
        if (query.start_time_ms.has_value() && event.timestamp_ms < query.start_time_ms.value()) {
            continue;
        }
        
        if (query.end_time_ms.has_value() && event.timestamp_ms > query.end_time_ms.value()) {
            continue;
        }
        
        // Handle offset
        if (skipped < query.offset) {
            skipped++;
            continue;
        }
        
        results.push_back(event);
        
        if (static_cast<int>(results.size()) >= query.limit) {
            break;
        }
    }
    
    return results;
}

std::optional<Event> EventManager::GetEvent(const std::string& event_id) const {
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    auto it = std::find_if(event_history_.begin(), event_history_.end(),
        [&event_id](const Event& e) { return e.id == event_id; });
    
    if (it != event_history_.end()) {
        return *it;
    }
    return std::nullopt;
}

int EventManager::GetEventCount(const EventQuery& query) const {
    auto events = QueryEvents(query);
    return static_cast<int>(events.size());
}

void EventManager::ClearHistory() {
    std::lock_guard<std::mutex> lock(history_mutex_);
    event_history_.clear();
}

std::vector<Event> EventManager::GetRecentEvents(int count) const {
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    std::vector<Event> results;
    int n = std::min(count, static_cast<int>(event_history_.size()));
    
    for (int i = 0; i < n; ++i) {
        results.push_back(event_history_[i]);
    }
    
    return results;
}

// ============================================================================
// Statistics
// ============================================================================

EventStats EventManager::GetStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void EventManager::ResetStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = EventStats{};
    stats_.start_time = std::chrono::system_clock::now();
}

// ============================================================================
// Action Handler Management
// ============================================================================

std::shared_ptr<ActionHandler> EventManager::GetHandler(ActionType type) {
    auto it = action_handlers_.find(type);
    if (it != action_handlers_.end()) {
        return it->second;
    }
    return nullptr;
}

void EventManager::RegisterActionHandler(ActionType type, std::shared_ptr<ActionHandler> handler) {
    action_handlers_[type] = std::move(handler);
    spdlog::info("[EventManager] Registered handler for action: {}", ActionTypeToString(type));
}

bool EventManager::IsActionAvailable(ActionType type) {
    auto handler = GetHandler(type);
    return handler && handler->IsAvailable();
}

void EventManager::RegisterActionHandlers() {
    ActionHandlerFactory::RegisterAll(*this);
}

// ============================================================================
// Private Methods
// ============================================================================

void EventManager::ProcessingThread() {
    while (running_.load()) {
        Event event;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] {
                return !event_queue_.empty() || !running_.load();
            });
            
            if (!running_.load() && event_queue_.empty()) {
                break;
            }
            
            if (event_queue_.empty()) {
                continue;
            }
            
            event = std::move(event_queue_.front());
            event_queue_.pop();
        }
        
        // Process the event
        ProcessEvent(event);
    }
}

void EventManager::ProcessEvent(const Event& event) {
    // Update stats
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.events_processed++;
    }
    
    // Generate snapshot if configured
    Event mutable_event = event;
    GenerateEventSnapshot(mutable_event);
    GenerateEventThumbnail(mutable_event);
    
    // Add to history
    AddToHistory(mutable_event);
    
    // Notify listeners
    NotifyListeners(mutable_event);
    
    // Match rules and execute actions
    auto matched_rules = MatchRules(mutable_event);
    
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.rules_matched += matched_rules.size();
    }
    
    for (const auto* rule : matched_rules) {
        ExecuteActions(*rule, mutable_event);
    }
}

std::vector<const EventRule*> EventManager::MatchRules(const Event& event) {
    std::vector<const EventRule*> matched;
    
    std::lock_guard<std::mutex> lock(rules_mutex_);
    
    for (const auto& rule : rules_) {
        if (rule.enabled && rule.Matches(event) && rule.IsActiveNow() && rule.IsCooledDown()) {
            matched.push_back(&rule);
        }
    }
    
    return matched;
}

void EventManager::ExecuteActions(const EventRule& rule, Event& event) {
    for (const auto& action : rule.actions) {
        if (!action.enabled) {
            continue;
        }
        
        auto handler = GetHandler(action.type);
        if (!handler) {
            spdlog::warn("[EventManager] No handler for action type: {}", 
                        ActionTypeToString(action.type));
            continue;
        }
        
        // Execute asynchronously
        std::thread([handler, action, event, rule_name = rule.name, this]() {
            try {
                auto result = handler->Execute(event, action);
                
                std::lock_guard<std::mutex> lock(stats_mutex_);
                stats_.actions_executed++;
                stats_.actions_by_type[action.type]++;
                
                if (result.success) {
                    stats_.actions_succeeded++;
                    spdlog::debug("[EventManager] Action {} completed for rule '{}'",
                                 ActionTypeToString(action.type), rule_name);
                } else {
                    stats_.actions_failed++;
                    stats_.action_failures_by_type[action.type]++;
                    spdlog::error("[EventManager] Action {} failed for rule '{}': {}",
                                 ActionTypeToString(action.type), rule_name, result.error_message);
                }
            } catch (const std::exception& e) {
                spdlog::error("[EventManager] Exception executing action {}: {}",
                             ActionTypeToString(action.type), e.what());
            }
        }).detach();
    }
}

void EventManager::NotifyListeners(const Event& event) {
    std::vector<ListenerEntry> listeners_copy;
    
    {
        std::lock_guard<std::mutex> lock(listeners_mutex_);
        listeners_copy = listeners_;
    }
    
    for (const auto& entry : listeners_copy) {
        // Check filter
        if (entry.filter && !entry.filter(event)) {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.events_filtered++;
            continue;
        }
        
        try {
            entry.listener(event);
        } catch (const std::exception& e) {
            spdlog::error("[EventManager] Listener {} threw exception: {}", entry.id, e.what());
        }
    }
}

void EventManager::AddToHistory(const Event& event) {
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    event_history_.push_front(event);
    
    // Trim history to max size
    while (static_cast<int>(event_history_.size()) > config_.max_history_size) {
        event_history_.pop_back();
    }
}

void EventManager::GenerateEventSnapshot(Event& event) {
    if (!config_.snapshot_on_event) {
        return;
    }
    
    try {
        auto& video_ctrl = media::VideoControl::Instance();
        if (!video_ctrl.IsInitialized()) {
            return;
        }
        
        auto jpeg_data = video_ctrl.CaptureSnapshot();
        if (jpeg_data.empty()) {
            return;
        }
        
        // Save snapshot to SD card under /mnt/sd/events/snapshots/
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm* tm = std::localtime(&time_t);
        
        char date_dir[32];
        snprintf(date_dir, sizeof(date_dir), "%04d%02d%02d",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
        
        std::string dir = std::string("/mnt/sd/events/snapshots/") + date_dir;
        std::filesystem::create_directories(dir);
        
        std::string path = dir + "/" + event.id + ".jpg";
        std::ofstream file(path, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(jpeg_data.data()), jpeg_data.size());
            file.close();
            event.snapshot_path = path;
        }
    } catch (const std::exception& e) {
        spdlog::debug("[EventManager] Snapshot generation failed: {}", e.what());
    }
}

void EventManager::GenerateEventThumbnail(Event& event) {
    if (!config_.thumbnail_on_event) {
        return;
    }
    
    // Thumbnail generation would be delegated to the SnapshotActionHandler
    // This is a placeholder - actual implementation depends on VideoControl integration
}

} // namespace events
} // namespace ipcam
