/**
 * @file counting.h
 * @brief People / Vehicle counting module
 *
 * High-level counting module that aggregates detection results
 * for display on OSD and threshold-based alerts.
 * Works in two modes:
 * 1. Line-based: Consumes LineCrossEvent data for in/out counting
 * 2. Area-based: Counts objects present inside defined zones
 */
#ifndef IPCAM_COUNTING_H_
#define IPCAM_COUNTING_H_

#include "ipcam/analytics.h"
#include "ipcam/line_crossing.h"
#include "ipcam/zone_intrusion.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <string>

namespace ipcam {
namespace ai {

/// Counting statistics snapshot
struct CountingStats {
    int people_in = 0;
    int people_out = 0;
    int people_present = 0;  // Current occupancy (in - out or area-based)
    int vehicle_in = 0;
    int vehicle_out = 0;
    int vehicle_present = 0;
    int total_in = 0;
    int total_out = 0;
    int64_t period_start = 0;
    int64_t last_update = 0;
};

/// Threshold alert event
struct CountingAlert {
    std::string type;     // "occupancy_high", "occupancy_low", "count_threshold"
    int current_value;
    int threshold;
    int64_t timestamp;
};

using CountingAlertCallback = std::function<void(const CountingAlert&)>;

/// Counting configuration
struct CountingConfig {
    bool enabled = false;
    int reset_interval_sec = 86400;      // Daily reset (0 = no auto reset)
    int occupancy_high_threshold = 0;    // 0 = disabled
    int occupancy_low_threshold = 0;     // 0 = disabled
    bool count_people = true;
    bool count_vehicles = true;
    bool show_osd = true;

    /// Counting zone polygon (percentage coords 0-100, like demo 08)
    /// If empty, counts ALL objects in frame. If set, only objects
    /// whose foot-point (bottom-center) is inside the zone are counted.
    std::vector<ZonePoint> zone_points;
};

/**
 * @brief People / Vehicle counting engine
 *
 * Aggregates counting data from line crossing events and zone
 * occupancy. Provides unified statistics for OSD display and
 * threshold-based alerting.
 */
class CountingEngine {
public:
    void Init(const CountingConfig& config);
    void Shutdown();

    /// Process a batch of line cross events
    void OnLineCrossEvents(const std::vector<LineCrossEvent>& events);

    /// Update area-based counts from current detections
    void UpdateAreaCounts(const std::vector<DetectionResult>& objects);

    /// Get current statistics
    CountingStats GetStats() const;

    /// Reset all counters
    void Reset();

    /// Check if auto-reset is due, reset if needed
    void CheckAutoReset();

    void SetConfig(const CountingConfig& config);
    bool IsEnabled() const { return config_.enabled; }

    void SetAlertCallback(CountingAlertCallback cb) { alert_cb_ = std::move(cb); }

    /// Get OSD text string for overlay display
    std::string GetOsdText() const;

private:
    CountingConfig config_;
    CountingAlertCallback alert_cb_;
    mutable std::mutex mutex_;
    CountingStats stats_;

    bool occupancy_high_fired_ = false;
    bool occupancy_low_fired_ = false;

    void CheckThresholds(int64_t now);

    /// Ray-casting point-in-polygon for counting zone
    bool PointInZone(float px, float py) const;
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_COUNTING_H_
