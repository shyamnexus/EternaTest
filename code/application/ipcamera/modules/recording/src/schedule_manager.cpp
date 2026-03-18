/**
 * @file schedule_manager.cpp
 * @brief Recording Schedule Manager Implementation
 */

#include "ipcam/schedule_manager.h"
#include "ipcam/recording.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>
#include <algorithm>
#include <memory>

namespace ipcam {
namespace recording {

using json = nlohmann::json;

// ============================================================================
// JSON Serialization
// ============================================================================

static json TimeSlotToJson(const TimeSlot& slot) {
    return json{
        {"hour", slot.hour},
        {"enabled", slot.enabled},
        {"type", RecordingTypeToString(slot.type)}
    };
}

static TimeSlot JsonToTimeSlot(const json& j) {
    TimeSlot slot;
    slot.hour = j.value("hour", 0);
    slot.enabled = j.value("enabled", false);
    slot.type = StringToRecordingType(j.value("type", "continuous"));
    return slot;
}

static json DayScheduleToJson(const DaySchedule& day) {
    json slots = json::array();
    for (const auto& slot : day.slots) {
        slots.push_back(TimeSlotToJson(slot));
    }
    return json{
        {"day_of_week", day.day_of_week},
        {"slots", slots}
    };
}

static DaySchedule JsonToDaySchedule(const json& j) {
    DaySchedule day;
    day.day_of_week = j.value("day_of_week", 0);
    if (j.contains("slots") && j["slots"].is_array()) {
        int idx = 0;
        for (const auto& slot : j["slots"]) {
            if (idx < 24) {
                day.slots[idx] = JsonToTimeSlot(slot);
                idx++;
            }
        }
    }
    return day;
}

static json ScheduleProfileToJson(const ScheduleProfile& profile) {
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

static ScheduleProfile JsonToScheduleProfile(const json& j) {
    ScheduleProfile profile;
    profile.id = j.value("id", "");
    profile.name = j.value("name", "");
    profile.enabled = j.value("enabled", true);
    
    if (j.contains("channels") && j["channels"].is_array()) {
        for (const auto& ch : j["channels"]) {
            profile.channels.push_back(ch.get<int>());
        }
    }
    
    if (j.contains("weekly_schedule") && j["weekly_schedule"].is_array()) {
        int idx = 0;
        for (const auto& day : j["weekly_schedule"]) {
            if (idx < 7) {
                profile.weekly_schedule[idx] = JsonToDaySchedule(day);
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
// Helper Functions
// ============================================================================

static std::string GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

static std::string GenerateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";  // Version 4 UUID
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << (8 + dis(gen) % 4);  // Variant
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    
    return ss.str();
}

// ============================================================================
// ScheduleManager Implementation
// ============================================================================

ScheduleManager& ScheduleManager::Instance() {
    static ScheduleManager instance;
    return instance;
}

bool ScheduleManager::Init() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("[ScheduleManager] Already initialized");
        return true;
    }
    
    // Load existing schedules from config system
    if (!LoadSchedules()) {
        // Create default configuration
        config_.enabled = false;
        config_.active_profile_id = "";
        config_.profiles.clear();
        
        // Create a default 24/7 profile
        ScheduleProfile default_profile;
        default_profile.id = GenerateUUID();
        default_profile.name = "24/7 Recording";
        default_profile.enabled = false;
        default_profile.channels = {0};
        default_profile.pre_record_seconds = 5;
        default_profile.post_record_seconds = 10;
        default_profile.created_at = GetCurrentTimestamp();
        default_profile.updated_at = default_profile.created_at;
        
        // Enable all slots for 24/7 recording
        for (int d = 0; d < 7; d++) {
            default_profile.weekly_schedule[d].day_of_week = d;
            for (int h = 0; h < 24; h++) {
                default_profile.weekly_schedule[d].slots[h].hour = h;
                default_profile.weekly_schedule[d].slots[h].enabled = true;
                default_profile.weekly_schedule[d].slots[h].type = RecordingType::Continuous;
            }
        }
        
        config_.profiles.push_back(default_profile);
        SaveSchedules();
    }
    
    initialized_.store(true);
    spdlog::info("[ScheduleManager] Initialized with {} profiles", config_.profiles.size());
    
    // Start background scheduler worker if schedule is enabled
    if (config_.enabled) {
        StartSchedulerWorker();
    }
    
    return true;
}

void ScheduleManager::Shutdown() {
    // Stop the scheduler worker first (outside the lock)
    StopSchedulerWorker();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    SaveSchedules();
    initialized_.store(false);
    spdlog::info("[ScheduleManager] Shutdown complete");
}

bool ScheduleManager::LoadSchedules() {
    namespace fs = std::filesystem;
    
    // Load basic settings from config system
    config_.enabled = ipcam::config::Get<bool>("recording.schedule.enabled", false);
    config_.active_profile_id = ipcam::config::Get<std::string>("recording.schedule.active_profile_id", "default-24x7");
    config_.profiles.clear();
    
    // Get config directory from config system (typically /etc/ipcamera/configs)
    std::string config_base = "/etc/ipcamera/configs";
    
    // Profile directories
    std::string factory_profiles_dir = config_base + "/config.factory.d/recording/profiles";
    std::string user_profiles_dir = config_base + "/config.d/recording/profiles";
    
    // Helper lambda to load profiles from a directory
    auto loadProfilesFromDir = [this](const std::string& dir_path, bool is_factory) {
        namespace fs = std::filesystem;
        
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            spdlog::debug("[ScheduleManager] Profile directory not found: {}", dir_path);
            return;
        }
        
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".json") continue;
            
            try {
                std::ifstream file(entry.path());
                if (!file.is_open()) {
                    spdlog::warn("[ScheduleManager] Failed to open profile: {}", entry.path().string());
                    continue;
                }
                
                json j = json::parse(file);
                ScheduleProfile profile = JsonToScheduleProfile(j);
                
                // Use filename (without extension) as ID if not specified
                if (profile.id.empty()) {
                    profile.id = entry.path().stem().string();
                }
                
                // Check if user profile overrides factory profile
                auto existing = std::find_if(config_.profiles.begin(), config_.profiles.end(),
                    [&](const ScheduleProfile& p) { return p.id == profile.id; });
                
                if (existing != config_.profiles.end()) {
                    if (!is_factory) {
                        // User profile overrides factory
                        *existing = profile;
                        spdlog::debug("[ScheduleManager] User profile overrides factory: {}", profile.id);
                    }
                } else {
                    config_.profiles.push_back(profile);
                    spdlog::debug("[ScheduleManager] Loaded profile: {} ({})", profile.name, profile.id);
                }
            } catch (const std::exception& e) {
                spdlog::warn("[ScheduleManager] Error loading profile {}: {}", 
                            entry.path().string(), e.what());
            }
        }
    };
    
    // Load factory profiles first (lower priority)
    loadProfilesFromDir(factory_profiles_dir, true);
    
    // Load user profiles (can override factory profiles)
    loadProfilesFromDir(user_profiles_dir, false);
    
    spdlog::info("[ScheduleManager] Loaded {} profiles from file system", config_.profiles.size());
    return !config_.profiles.empty();
}

bool ScheduleManager::SaveSchedules() {
    namespace fs = std::filesystem;
    
    try {
        // Save basic settings to config system
        ipcam::config::Set("recording.schedule.enabled", config_.enabled);
        ipcam::config::Set("recording.schedule.active_profile_id", config_.active_profile_id);
        
        // Persist basic config
        if (!ipcam::config::Save()) {
            spdlog::error("[ScheduleManager] Failed to save schedule config");
            return false;
        }
        
        // Profile directory for user profiles
        std::string config_base = "/etc/ipcamera/configs";
        std::string user_profiles_dir = config_base + "/config.d/recording/profiles";
        
        // Create directory if not exists
        if (!fs::exists(user_profiles_dir)) {
            fs::create_directories(user_profiles_dir);
        }
        
        // Save each non-factory profile as individual file
        // Factory profiles (those that exist in factory dir) are not overwritten
        std::string factory_profiles_dir = config_base + "/config.factory.d/recording/profiles";
        
        for (const auto& profile : config_.profiles) {
            std::string factory_path = factory_profiles_dir + "/" + profile.id + ".json";
            std::string user_path = user_profiles_dir + "/" + profile.id + ".json";
            
            // Skip if this is an unmodified factory profile
            // (i.e., factory file exists and no user override)
            if (fs::exists(factory_path) && !fs::exists(user_path)) {
                // Check if profile was modified by comparing
                // For simplicity, we'll save if updated_at is different
                try {
                    std::ifstream factory_file(factory_path);
                    if (factory_file.is_open()) {
                        json factory_json = json::parse(factory_file);
                        std::string factory_updated = factory_json.value("updated_at", "");
                        if (factory_updated == profile.updated_at) {
                            spdlog::debug("[ScheduleManager] Skipping unmodified factory profile: {}", profile.id);
                            continue;
                        }
                    }
                } catch (...) {
                    // Error reading factory, save user copy
                }
            }
            
            // Save profile to user directory
            json profile_json = ScheduleProfileToJson(profile);
            
            std::string temp_path = user_path + ".tmp";
            std::ofstream file(temp_path);
            if (!file.is_open()) {
                spdlog::error("[ScheduleManager] Failed to create profile file: {}", temp_path);
                continue;
            }
            
            file << profile_json.dump(2);
            file.close();
            
            // Atomic rename
            if (std::rename(temp_path.c_str(), user_path.c_str()) != 0) {
                spdlog::error("[ScheduleManager] Failed to rename profile file: {}", user_path);
                std::remove(temp_path.c_str());
                continue;
            }
            
            spdlog::debug("[ScheduleManager] Saved profile: {} -> {}", profile.name, user_path);
        }
        
        spdlog::info("[ScheduleManager] Saved {} profiles to file system", config_.profiles.size());
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("[ScheduleManager] Failed to save profiles: {}", e.what());
        return false;
    }
}

ScheduleConfig ScheduleManager::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool ScheduleManager::SetEnabled(bool enabled) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.enabled = enabled;
        SaveSchedules();
        NotifyObservers();
    }
    
    // Start or stop the scheduler worker based on enabled state
    if (enabled) {
        StartSchedulerWorker();
    } else {
        StopSchedulerWorker();
    }
    
    return true;
}

bool ScheduleManager::SetActiveProfile(const std::string& profile_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Verify profile exists
    auto it = std::find_if(config_.profiles.begin(), config_.profiles.end(),
                           [&](const ScheduleProfile& p) { return p.id == profile_id; });
    if (it == config_.profiles.end()) {
        spdlog::error("[ScheduleManager] Profile not found: {}", profile_id);
        return false;
    }
    
    config_.active_profile_id = profile_id;
    SaveSchedules();
    NotifyObservers();
    return true;
}

std::string ScheduleManager::CreateProfile(const ScheduleProfile& profile) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ScheduleProfile new_profile = profile;
    new_profile.id = GenerateUUID();
    new_profile.created_at = GetCurrentTimestamp();
    new_profile.updated_at = new_profile.created_at;
    
    // Initialize slots if not set
    for (int d = 0; d < 7; d++) {
        new_profile.weekly_schedule[d].day_of_week = d;
        for (int h = 0; h < 24; h++) {
            new_profile.weekly_schedule[d].slots[h].hour = h;
        }
    }
    
    config_.profiles.push_back(new_profile);
    SaveSchedules();
    NotifyObservers();
    
    spdlog::info("[ScheduleManager] Created profile: {} ({})", new_profile.name, new_profile.id);
    return new_profile.id;
}

bool ScheduleManager::UpdateProfile(const std::string& id, const ScheduleProfile& profile) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(config_.profiles.begin(), config_.profiles.end(),
                           [&](const ScheduleProfile& p) { return p.id == id; });
    if (it == config_.profiles.end()) {
        spdlog::error("[ScheduleManager] Profile not found: {}", id);
        return false;
    }
    
    // Preserve ID and creation time
    std::string original_id = it->id;
    std::string original_created = it->created_at;
    
    *it = profile;
    it->id = original_id;
    it->created_at = original_created;
    it->updated_at = GetCurrentTimestamp();
    
    SaveSchedules();
    NotifyObservers();
    
    spdlog::info("[ScheduleManager] Updated profile: {}", id);
    return true;
}

bool ScheduleManager::DeleteProfile(const std::string& id) {
    namespace fs = std::filesystem;
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(config_.profiles.begin(), config_.profiles.end(),
                           [&](const ScheduleProfile& p) { return p.id == id; });
    if (it == config_.profiles.end()) {
        spdlog::error("[ScheduleManager] Profile not found: {}", id);
        return false;
    }
    
    std::string name = it->name;
    
    // Check if this is a factory profile
    std::string config_base = "/etc/ipcamera/configs";
    std::string factory_path = config_base + "/config.factory.d/recording/profiles/" + id + ".json";
    std::string user_path = config_base + "/config.d/recording/profiles/" + id + ".json";
    
    bool is_factory_profile = fs::exists(factory_path);
    
    // Delete user profile file if exists
    if (fs::exists(user_path)) {
        try {
            fs::remove(user_path);
            spdlog::debug("[ScheduleManager] Deleted profile file: {}", user_path);
        } catch (const std::exception& e) {
            spdlog::warn("[ScheduleManager] Failed to delete profile file: {}", e.what());
        }
    }
    
    // If it's a factory profile, we can't really delete it - just remove from memory
    // It will be reloaded on next restart. To truly "delete" a factory profile,
    // the user would need to create an override that marks it as deleted.
    if (is_factory_profile) {
        spdlog::warn("[ScheduleManager] Factory profile '{}' cannot be permanently deleted", name);
    }
    
    config_.profiles.erase(it);
    
    // Clear active profile if deleted
    if (config_.active_profile_id == id) {
        config_.active_profile_id = "";
        // Try to select another profile
        if (!config_.profiles.empty()) {
            config_.active_profile_id = config_.profiles.front().id;
        }
    }
    
    SaveSchedules();
    NotifyObservers();
    
    spdlog::info("[ScheduleManager] Deleted profile: {} ({})", name, id);
    return true;
}

std::optional<ScheduleProfile> ScheduleManager::GetProfile(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(config_.profiles.begin(), config_.profiles.end(),
                           [&](const ScheduleProfile& p) { return p.id == id; });
    if (it != config_.profiles.end()) {
        return *it;
    }
    return std::nullopt;
}

std::vector<ScheduleProfile> ScheduleManager::GetAllProfiles() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.profiles;
}

std::optional<ScheduleProfile> ScheduleManager::GetCurrentActiveProfile() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!config_.enabled || config_.active_profile_id.empty()) {
        return std::nullopt;
    }
    
    auto it = std::find_if(config_.profiles.begin(), config_.profiles.end(),
                           [&](const ScheduleProfile& p) { 
                               return p.id == config_.active_profile_id && p.enabled; 
                           });
    if (it != config_.profiles.end()) {
        return *it;
    }
    return std::nullopt;
}

bool ScheduleManager::ShouldRecordNow(int channel) const {
    auto profile = GetCurrentActiveProfile();
    if (!profile) {
        return false;
    }
    
    // Check if channel is in profile
    auto& channels = profile->channels;
    if (!channels.empty() && 
        std::find(channels.begin(), channels.end(), channel) == channels.end()) {
        return false;
    }
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    
    int day = tm->tm_wday;  // 0 = Sunday
    int hour = tm->tm_hour;
    
    // Check schedule
    if (day >= 0 && day < 7 && hour >= 0 && hour < 24) {
        return profile->weekly_schedule[day].slots[hour].enabled;
    }
    
    return false;
}

RecordingType ScheduleManager::GetCurrentRecordingType(int channel) const {
    auto profile = GetCurrentActiveProfile();
    if (!profile) {
        return RecordingType::Continuous;
    }
    
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    
    int day = tm->tm_wday;
    int hour = tm->tm_hour;
    
    if (day >= 0 && day < 7 && hour >= 0 && hour < 24) {
        return profile->weekly_schedule[day].slots[hour].type;
    }
    
    return RecordingType::Continuous;
}

std::optional<ScheduledEvent> ScheduleManager::GetNextScheduledEvent() const {
    auto profile = GetCurrentActiveProfile();
    if (!profile) {
        return std::nullopt;
    }
    
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    
    int current_day = tm->tm_wday;
    int current_hour = tm->tm_hour;
    bool currently_recording = ShouldRecordNow(0);
    
    // Search for next state change
    for (int offset = 0; offset < 7 * 24; offset++) {
        int check_hour = (current_hour + offset) % 24;
        int check_day = (current_day + (current_hour + offset) / 24) % 7;
        
        if (offset == 0) continue;  // Skip current hour
        
        bool slot_enabled = profile->weekly_schedule[check_day].slots[check_hour].enabled;
        
        if (slot_enabled != currently_recording) {
            ScheduledEvent event;
            event.type = slot_enabled ? ScheduledEventType::RecordingStart : ScheduledEventType::RecordingStop;
            
            // Calculate timestamp
            std::tm event_tm = *tm;
            event_tm.tm_wday = check_day;
            event_tm.tm_hour = check_hour;
            event_tm.tm_min = 0;
            event_tm.tm_sec = 0;
            
            // Add days if needed
            int days_ahead = (check_day - current_day + 7) % 7;
            if (days_ahead == 0 && check_hour <= current_hour) {
                days_ahead = 7;
            }
            event_tm.tm_mday += days_ahead;
            
            std::time_t event_time = std::mktime(&event_tm);
            event.timestamp = std::chrono::system_clock::from_time_t(event_time);
            event.profile_id = profile->id;
            
            return event;
        }
    }
    
    return std::nullopt;
}

void ScheduleManager::AddObserver(ScheduleObserver* observer) {
    std::lock_guard<std::mutex> lock(observer_mutex_);
    observers_.push_back(observer);
}

void ScheduleManager::RemoveObserver(ScheduleObserver* observer) {
    std::lock_guard<std::mutex> lock(observer_mutex_);
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), observer),
        observers_.end()
    );
}

void ScheduleManager::NotifyObservers() {
    std::lock_guard<std::mutex> lock(observer_mutex_);
    for (auto* observer : observers_) {
        if (observer) {
            observer->OnScheduleChanged(config_);
        }
    }
}

// ============================================================================
// Background Scheduler Worker Implementation
// ============================================================================

void ScheduleManager::StartSchedulerWorker() {
    if (worker_running_.load()) {
        spdlog::debug("[ScheduleManager] Scheduler worker already running");
        return;
    }
    
    worker_stop_requested_.store(false);
    worker_running_.store(true);
    
    worker_thread_ = std::make_unique<std::thread>(&ScheduleManager::SchedulerWorkerLoop, this);
    spdlog::info("[ScheduleManager] Background scheduler worker started");
}

void ScheduleManager::StopSchedulerWorker() {
    if (!worker_running_.load()) {
        return;
    }
    
    spdlog::info("[ScheduleManager] Stopping background scheduler worker...");
    worker_stop_requested_.store(true);
    
    // Wake up the worker thread
    {
        std::lock_guard<std::mutex> lock(worker_mutex_);
        worker_cv_.notify_all();
    }
    
    // Wait for thread to finish
    if (worker_thread_ && worker_thread_->joinable()) {
        worker_thread_->join();
    }
    worker_thread_.reset();
    
    worker_running_.store(false);
    spdlog::info("[ScheduleManager] Background scheduler worker stopped");
}

void ScheduleManager::SchedulerWorkerLoop() {
    spdlog::info("[ScheduleManager] Scheduler worker loop started");
    
    // Wait for recording service to be fully ready
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    while (!worker_stop_requested_.load()) {
        try {
            auto& recording_service = RecordingService::Instance();
            
            // Check if recording service is initialized
            if (!recording_service.IsInitialized()) {
                spdlog::debug("[ScheduleManager] Recording service not initialized, waiting...");
                std::unique_lock<std::mutex> lock(worker_mutex_);
                worker_cv_.wait_for(lock, std::chrono::seconds(5), [this] {
                    return worker_stop_requested_.load();
                });
                continue;
            }
            
            // Check if storage is available
            if (!recording_service.IsStorageAvailable()) {
                spdlog::debug("[ScheduleManager] Storage not available, skipping schedule check");
                
                // Wait for 30 seconds before checking again
                std::unique_lock<std::mutex> lock(worker_mutex_);
                worker_cv_.wait_for(lock, std::chrono::seconds(30), [this] {
                    return worker_stop_requested_.load();
                });
                continue;
            }
            
            // Check schedule enabled state
            bool schedule_enabled = false;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                schedule_enabled = config_.enabled;
            }
            
            if (!schedule_enabled) {
                spdlog::debug("[ScheduleManager] Schedule recording disabled, worker waiting");
                
                // Wait until enabled or stop requested
                std::unique_lock<std::mutex> lock(worker_mutex_);
                worker_cv_.wait_for(lock, std::chrono::seconds(10), [this] {
                    return worker_stop_requested_.load();
                });
                continue;
            }
            
            // Check each channel
            for (int channel = 0; channel < 3; channel++) {
                if (worker_stop_requested_.load()) break;
                
                bool should_record = ShouldRecordNow(channel);
                bool currently_recording = channel_recording_state_[channel];
                
                // Also check actual recording service state
                auto session = recording_service.GetSession(channel);
                bool actually_recording = (session.status == RecordingStatus::Recording);
                
                // Sync our state with actual state
                if (actually_recording != currently_recording) {
                    channel_recording_state_[channel] = actually_recording;
                    currently_recording = actually_recording;
                }
                
                if (should_record && !currently_recording) {
                    // Should start recording
                    RecordingType type = GetCurrentRecordingType(channel);
                    spdlog::info("[ScheduleManager] Schedule says to start recording on channel {} (type: {})",
                                 channel, RecordingTypeToString(type));
                    
                    if (recording_service.StartRecording(channel, type)) {
                        channel_recording_state_[channel] = true;
                        spdlog::info("[ScheduleManager] Successfully started scheduled recording on channel {}", channel);
                    } else {
                        spdlog::error("[ScheduleManager] Failed to start scheduled recording on channel {}", channel);
                    }
                } else if (!should_record && currently_recording) {
                    // Should stop recording
                    spdlog::info("[ScheduleManager] Schedule says to stop recording on channel {}", channel);
                    
                    if (recording_service.StopRecording(channel)) {
                        channel_recording_state_[channel] = false;
                        spdlog::info("[ScheduleManager] Successfully stopped scheduled recording on channel {}", channel);
                    } else {
                        spdlog::error("[ScheduleManager] Failed to stop scheduled recording on channel {}", channel);
                    }
                }
            }
            
        } catch (const std::exception& e) {
            spdlog::error("[ScheduleManager] Error in scheduler worker: {}", e.what());
        }
        
        // Check schedule every 30 seconds
        std::unique_lock<std::mutex> lock(worker_mutex_);
        worker_cv_.wait_for(lock, std::chrono::seconds(30), [this] {
            return worker_stop_requested_.load();
        });
    }
    
    spdlog::info("[ScheduleManager] Scheduler worker loop exiting");
}

} // namespace recording
} // namespace ipcam
