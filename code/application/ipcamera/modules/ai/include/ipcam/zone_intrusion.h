/**
 * @file zone_intrusion.h
 * @brief Zone intrusion detection engine
 *
 * Detects when tracked objects enter defined polygonal zones.
 * Supports dwell time thresholds and class-based filtering.
 */
#ifndef IPCAM_ZONE_INTRUSION_H_
#define IPCAM_ZONE_INTRUSION_H_

#include "ipcam/analytics.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>

namespace ipcam {
namespace ai {

/// Zone intrusion event
struct ZoneIntrusionEvent {
    uint32_t zone_id;
    uint32_t track_id;
    ObjectCategory category;
    int64_t enter_time;     // When object first entered zone
    int64_t trigger_time;   // When dwell threshold was exceeded
    int dwell_ms;           // Actual dwell time at trigger
};

using ZoneIntrusionCallback = std::function<void(const ZoneIntrusionEvent&)>;

/**
 * @brief Zone intrusion detection engine
 *
 * Each frame, checks whether tracked object centroids fall inside
 * configured polygonal zones. Tracks per-object dwell time per zone,
 * and triggers alarms after the configured dwell_time_ms threshold.
 *
 * Uses ray-casting algorithm for point-in-polygon test.
 */
class ZoneIntrusionEngine {
public:
    void Init(const ZoneIntrusionConfig& config);
    void Shutdown();

    /// Check tracked objects against configured zones
    /// @param objects Current tracked objects
    /// @return Events triggered this frame (dwell threshold exceeded)
    std::vector<ZoneIntrusionEvent> Check(const std::vector<DetectionResult>& objects);

    void SetConfig(const ZoneIntrusionConfig& config);
    bool IsEnabled() const { return config_.enabled; }

    void SetCallback(ZoneIntrusionCallback cb) { callback_ = std::move(cb); }

    /// Get number of objects currently inside a zone
    int GetOccupancy(uint32_t zone_id) const;

private:
    ZoneIntrusionConfig config_;
    ZoneIntrusionCallback callback_;
    mutable std::mutex mutex_;

    /// Per-object, per-zone tracking
    struct DwellState {
        int64_t enter_time = 0;
        bool inside = false;
        bool alarm_fired = false;  // Only fire once per entry
    };

    // Key: (track_id, zone_id)
    struct PairHash {
        size_t operator()(const std::pair<uint32_t, uint32_t>& p) const {
            return std::hash<uint64_t>()(
                (static_cast<uint64_t>(p.first) << 32) | p.second);
        }
    };
    std::unordered_map<std::pair<uint32_t, uint32_t>, DwellState, PairHash> dwell_states_;

    /// Ray-casting point-in-polygon test
    static bool PointInPolygon(float px, float py, const std::vector<ZonePoint>& polygon);

    /// Check if an object category matches the zone's alarm_classes filter
    static bool CategoryMatchesFilter(ObjectCategory cat,
                                      const std::vector<std::string>& alarm_classes);
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_ZONE_INTRUSION_H_
