/**
 * @file schedule_manager.h
 * @brief Recording Schedule Manager for IP Camera
 * 
 * Manages recording schedules with:
 * - Multiple schedule profiles
 * - Weekly time slot configuration (24 hours x 7 days)
 * - Normal (continuous) and event-triggered recording modes
 * - Pre/post recording buffers
 */

#pragma once

#include "recording.h"  // For RecordingType
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <optional>
#include <array>
#include <thread>
#include <condition_variable>
#include <functional>

namespace ipcam {
namespace recording {

// ============================================================================
// Schedule Structures
// ============================================================================

/**
 * Single hour time slot in a schedule
 */
struct TimeSlot {
    int hour = 0;                                   ///< Hour of day (0-23)
    bool enabled = false;                           ///< Whether recording is enabled
    RecordingType type = RecordingType::Continuous; ///< Recording type
};

/**
 * Schedule for a single day of the week
 */
struct DaySchedule {
    int day_of_week = 0;                     ///< Day index (0=Sunday, 6=Saturday)
    std::array<TimeSlot, 24> slots;          ///< 24 hourly slots
    
    DaySchedule() {
        for (int i = 0; i < 24; i++) {
            slots[i].hour = i;
            slots[i].enabled = false;
            slots[i].type = RecordingType::Continuous;
        }
    }
};

/**
 * Complete schedule profile
 */
struct ScheduleProfile {
    std::string id;                              ///< Unique profile ID (UUID)
    std::string name;                            ///< Profile name
    bool enabled = true;                         ///< Whether profile is active
    std::vector<int> channels;                   ///< Channel IDs this applies to
    std::array<DaySchedule, 7> weekly_schedule;  ///< Weekly schedule (Sun-Sat)
    int pre_record_seconds = 5;                  ///< Pre-record buffer
    int post_record_seconds = 10;                ///< Post-record buffer
    std::string created_at;                      ///< Creation timestamp
    std::string updated_at;                      ///< Last modification timestamp
    
    ScheduleProfile() {
        for (int d = 0; d < 7; d++) {
            weekly_schedule[d].day_of_week = d;
        }
    }
};

/**
 * Schedule configuration
 */
struct ScheduleConfig {
    bool enabled = false;                   ///< Master enable for schedule recording
    std::string active_profile_id;          ///< Currently active profile ID
    std::vector<ScheduleProfile> profiles;  ///< All schedule profiles
};

/**
 * Scheduled event type
 */
enum class ScheduledEventType {
    RecordingStart,
    RecordingStop
};

/**
 * Scheduled event info
 */
struct ScheduledEvent {
    ScheduledEventType type;
    std::chrono::system_clock::time_point timestamp;
    std::string profile_id;
};

/**
 * Observer interface for schedule changes
 */
class ScheduleObserver {
public:
    virtual ~ScheduleObserver() = default;
    virtual void OnScheduleChanged(const ScheduleConfig& config) = 0;
};

// ============================================================================
// Schedule Manager Class
// ============================================================================

/**
 * Manages recording schedules
 */
class ScheduleManager {
public:
    /**
     * Get singleton instance
     */
    static ScheduleManager& Instance();
    
    /**
     * Initialize the schedule manager
     * Loads configuration from recording.schedule config section
     * @return true on success
     */
    bool Init();
    
    /**
     * Shutdown and save state
     */
    void Shutdown();
    
    /**
     * Check if initialized
     */
    bool IsInitialized() const { return initialized_.load(); }
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * Get current schedule configuration
     */
    ScheduleConfig GetConfig() const;
    
    /**
     * Enable/disable schedule recording
     */
    bool SetEnabled(bool enabled);
    
    /**
     * Set active profile
     */
    bool SetActiveProfile(const std::string& profile_id);
    
    // ========================================================================
    // Profile Management
    // ========================================================================
    
    /**
     * Get all schedule profiles
     */
    std::vector<ScheduleProfile> GetAllProfiles() const;
    
    /**
     * Get profile by ID
     */
    std::optional<ScheduleProfile> GetProfile(const std::string& id) const;
    
    /**
     * Create a new profile
     * @return The created profile ID
     */
    std::string CreateProfile(const ScheduleProfile& profile);
    
    /**
     * Update an existing profile
     * @return true on success
     */
    bool UpdateProfile(const std::string& id, const ScheduleProfile& profile);
    
    /**
     * Delete a profile
     * @return true on success
     */
    bool DeleteProfile(const std::string& id);
    
    // ========================================================================
    // Schedule Checking
    // ========================================================================
    
    /**
     * Get currently active profile (if any)
     */
    std::optional<ScheduleProfile> GetCurrentActiveProfile() const;
    
    /**
     * Check if recording should be active for a channel at current time
     * @param channel Channel ID
     * @return true if should record now
     */
    bool ShouldRecordNow(int channel = 0) const;
    
    /**
     * Get current recording type based on schedule
     * @param channel Channel ID
     * @return Recording type from current schedule slot
     */
    RecordingType GetCurrentRecordingType(int channel = 0) const;
    
    /**
     * Get next scheduled event
     */
    std::optional<ScheduledEvent> GetNextScheduledEvent() const;
    
    // ========================================================================
    // Observer Pattern
    // ========================================================================
    
    /**
     * Add schedule change observer
     */
    void AddObserver(ScheduleObserver* observer);
    
    /**
     * Remove schedule change observer
     */
    void RemoveObserver(ScheduleObserver* observer);

    // ========================================================================
    // Background Scheduler Worker
    // ========================================================================
    
    /**
     * Start the background scheduler worker thread
     * This thread monitors the schedule and auto-starts/stops recording
     */
    void StartSchedulerWorker();
    
    /**
     * Stop the background scheduler worker thread
     */
    void StopSchedulerWorker();
    
    /**
     * Check if scheduler worker is running
     */
    bool IsSchedulerWorkerRunning() const { return worker_running_.load(); }
    
    // ========================================================================
    // Persistence
    // ========================================================================
    
    /**
     * Save schedules to config file
     */
    bool SaveSchedules();
    
    /**
     * Load schedules from config file
     */
    bool LoadSchedules();
    
private:
    ScheduleManager() = default;
    ~ScheduleManager() = default;
    ScheduleManager(const ScheduleManager&) = delete;
    ScheduleManager& operator=(const ScheduleManager&) = delete;
    
    /**
     * Notify all observers of schedule change
     */
    void NotifyObservers();
    
    /**
     * Background worker thread function
     */
    void SchedulerWorkerLoop();
    
    std::atomic<bool> initialized_{false};
    mutable std::mutex mutex_;
    mutable std::mutex observer_mutex_;
    ScheduleConfig config_;
    std::vector<ScheduleObserver*> observers_;
    
    // Background scheduler worker
    std::unique_ptr<std::thread> worker_thread_;
    std::atomic<bool> worker_running_{false};
    std::atomic<bool> worker_stop_requested_{false};
    std::condition_variable worker_cv_;
    std::mutex worker_mutex_;
    std::array<bool, 3> channel_recording_state_{false, false, false};  // Track current recording state per channel
};

} // namespace recording
} // namespace ipcam
