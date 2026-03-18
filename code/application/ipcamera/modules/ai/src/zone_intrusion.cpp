/**
 * @file zone_intrusion.cpp
 * @brief Zone intrusion detection engine implementation
 *
 * Uses ray-casting for point-in-polygon test and dwell-time
 * tracking per object per zone to generate intrusion alarms.
 */
#include "ipcam/zone_intrusion.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>

namespace ipcam {
namespace ai {

void ZoneIntrusionEngine::Init(const ZoneIntrusionConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    dwell_states_.clear();
    spdlog::info("ZoneIntrusionEngine: initialized with {} zones", config_.zones.size());
}

void ZoneIntrusionEngine::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    dwell_states_.clear();
}

void ZoneIntrusionEngine::SetConfig(const ZoneIntrusionConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    dwell_states_.clear();
}

std::vector<ZoneIntrusionEvent> ZoneIntrusionEngine::Check(
    const std::vector<DetectionResult>& objects) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ZoneIntrusionEvent> events;

    if (!config_.enabled) return events;

    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    // Track which (track_id, zone_id) pairs are active this frame
    std::unordered_map<std::pair<uint32_t, uint32_t>, bool, PairHash> active;

    for (const auto& obj : objects) {
        uint32_t track_id = static_cast<uint32_t>(obj.id);

        // Foot-point (bottom-center) in percentage coordinates (0-100)
        // Matches demo 06 approach: person stands ON ground, so test foot position
        float cx = ((obj.bbox.x1 + obj.bbox.x2) * 0.5f) * 100.0f;
        float cy = obj.bbox.y2 * 100.0f;

        for (const auto& zone : config_.zones) {
            if (!zone.enabled) continue;
            if (zone.points.size() < 3) continue;

            // Check class filter
            if (!zone.alarm_classes.empty() &&
                !CategoryMatchesFilter(obj.category, zone.alarm_classes)) {
                continue;
            }

            auto key = std::make_pair(track_id, zone.id);
            bool inside = PointInPolygon(cx, cy, zone.points);
            active[key] = true;

            auto& state = dwell_states_[key];

            if (inside) {
                if (!state.inside) {
                    // Just entered
                    state.inside = true;
                    state.enter_time = now;
                    state.alarm_fired = false;
                }

                // Check dwell time threshold
                int dwell = static_cast<int>(now - state.enter_time);
                if (dwell >= zone.dwell_time_ms && !state.alarm_fired) {
                    state.alarm_fired = true;

                    ZoneIntrusionEvent evt;
                    evt.zone_id = zone.id;
                    evt.track_id = track_id;
                    evt.category = obj.category;
                    evt.enter_time = state.enter_time;
                    evt.trigger_time = now;
                    evt.dwell_ms = dwell;
                    events.push_back(evt);

                    spdlog::info("ZoneIntrusion: obj {} in zone {} for {}ms (threshold {}ms)",
                                 track_id, zone.id, dwell, zone.dwell_time_ms);

                    if (callback_) {
                        callback_(evt);
                    }
                }
            } else {
                // Object left the zone
                state.inside = false;
                state.alarm_fired = false;
            }
        }
    }

    // Expire stale entries for objects no longer tracked
    for (auto it = dwell_states_.begin(); it != dwell_states_.end(); ) {
        if (active.find(it->first) == active.end()) {
            it = dwell_states_.erase(it);
        } else {
            ++it;
        }
    }

    return events;
}

int ZoneIntrusionEngine::GetOccupancy(uint32_t zone_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    int count = 0;
    for (const auto& kv : dwell_states_) {
        if (kv.first.second == zone_id && kv.second.inside) {
            count++;
        }
    }
    return count;
}

// ============================================================================
// Geometry helpers
// ============================================================================

bool ZoneIntrusionEngine::PointInPolygon(float px, float py,
                                          const std::vector<ZonePoint>& polygon) {
    // Ray-casting algorithm: cast horizontal ray from point to +inf X,
    // count intersections with polygon edges.
    int n = static_cast<int>(polygon.size());
    bool inside = false;

    for (int i = 0, j = n - 1; i < n; j = i++) {
        float xi = static_cast<float>(polygon[i].x);
        float yi = static_cast<float>(polygon[i].y);
        float xj = static_cast<float>(polygon[j].x);
        float yj = static_cast<float>(polygon[j].y);

        bool intersect = ((yi > py) != (yj > py)) &&
                         (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
        if (intersect) inside = !inside;
    }

    return inside;
}

bool ZoneIntrusionEngine::CategoryMatchesFilter(
    ObjectCategory cat, const std::vector<std::string>& alarm_classes) {

    static const std::unordered_map<std::string, ObjectCategory> name_map = {
        {"person",    ObjectCategory::kPerson},
        {"vehicle",   ObjectCategory::kVehicle},
        {"non_motor", ObjectCategory::kNonMotor},
        {"face",      ObjectCategory::kFace},
        {"motion",    ObjectCategory::kMotion},
    };

    for (const auto& cls : alarm_classes) {
        auto it = name_map.find(cls);
        if (it != name_map.end() && it->second == cat) {
            return true;
        }
    }
    return false;
}

} // namespace ai
} // namespace ipcam
