/**
 * @file line_crossing.cpp
 * @brief Line crossing detection engine implementation
 *
 * Uses cross-product geometry to detect when tracked object
 * centroids cross configured lines between consecutive frames.
 */
#include "ipcam/line_crossing.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <cmath>

namespace ipcam {
namespace ai {

void LineCrossingEngine::Init(const LineCrossingConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    object_history_.clear();
    line_counts_.clear();

    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    for (const auto& line : config_.lines) {
        if (line.enabled) {
            LineCounts lc;
            lc.line_id = line.id;
            lc.last_reset = now;
            line_counts_[line.id] = lc;
        }
    }

    spdlog::info("LineCrossingEngine: initialized with {} lines", config_.lines.size());
}

void LineCrossingEngine::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    object_history_.clear();
    line_counts_.clear();
}

void LineCrossingEngine::SetConfig(const LineCrossingConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
}

std::vector<LineCrossEvent> LineCrossingEngine::Check(
    const std::vector<DetectionResult>& objects) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<LineCrossEvent> events;

    if (!config_.enabled) return events;

    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    // Mark all existing objects as unseen this frame
    std::unordered_map<uint32_t, bool> seen;

    for (const auto& obj : objects) {
        uint32_t track_id = static_cast<uint32_t>(obj.id);
        // Foot-point (bottom-center) like demos 05/06 — person walks across line
        float cx = (obj.bbox.x1 + obj.bbox.x2) * 0.5f;
        float cy = obj.bbox.y2;

        // Normalize to percentage (0-100) to match line coordinate system
        float cx_pct = cx * 100.0f;
        float cy_pct = cy * 100.0f;

        seen[track_id] = true;

        auto it = object_history_.find(track_id);
        if (it == object_history_.end()) {
            // First time seeing this object — record position, no crossing check
            ObjectHistory h;
            h.prev_cx = cx_pct;
            h.prev_cy = cy_pct;
            h.valid = true;
            object_history_[track_id] = h;
            continue;
        }

        auto& hist = it->second;
        if (!hist.valid) {
            hist.prev_cx = cx_pct;
            hist.prev_cy = cy_pct;
            hist.valid = true;
            continue;
        }

        // Check against each configured line
        for (const auto& line : config_.lines) {
            if (!line.enabled) continue;

            if (SegmentsCross(hist.prev_cx, hist.prev_cy, cx_pct, cy_pct,
                              line.x1, line.y1, line.x2, line.y2)) {

                std::string dir = GetDirection(hist.prev_cx, hist.prev_cy, cx_pct, cy_pct,
                                               line.x1, line.y1, line.x2, line.y2);

                // Check direction filter
                bool emit = false;
                if (line.direction == "both") {
                    emit = true;
                } else if (line.direction == "a_to_b" && dir == "a_to_b") {
                    emit = true;
                } else if (line.direction == "b_to_a" && dir == "b_to_a") {
                    emit = true;
                }

                if (emit) {
                    LineCrossEvent evt;
                    evt.line_id = line.id;
                    evt.track_id = track_id;
                    evt.category = obj.category;
                    evt.direction = dir;
                    evt.timestamp = now;
                    events.push_back(evt);

                    // Update counts
                    auto& counts = line_counts_[line.id];
                    if (dir == "a_to_b") {
                        counts.in_count++;
                    } else {
                        counts.out_count++;
                    }

                    spdlog::info("LineCross: obj {} crossed line {} ({}), in={} out={}",
                                 track_id, line.id, dir,
                                 counts.in_count, counts.out_count);

                    if (callback_) {
                        callback_(evt);
                    }
                }
            }
        }

        // Update history
        hist.prev_cx = cx_pct;
        hist.prev_cy = cy_pct;
    }

    // Expire objects not seen this frame (after several missing frames)
    for (auto it = object_history_.begin(); it != object_history_.end(); ) {
        if (seen.find(it->first) == seen.end()) {
            it = object_history_.erase(it);
        } else {
            ++it;
        }
    }

    return events;
}

LineCounts LineCrossingEngine::GetCounts(uint32_t line_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = line_counts_.find(line_id);
    if (it != line_counts_.end()) return it->second;
    return {};
}

std::vector<LineCounts> LineCrossingEngine::GetAllCounts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<LineCounts> result;
    result.reserve(line_counts_.size());
    for (const auto& kv : line_counts_) {
        result.push_back(kv.second);
    }
    return result;
}

void LineCrossingEngine::ResetCounts(uint32_t line_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    if (line_id == 0) {
        for (auto& kv : line_counts_) {
            kv.second.in_count = 0;
            kv.second.out_count = 0;
            kv.second.last_reset = now;
        }
    } else {
        auto it = line_counts_.find(line_id);
        if (it != line_counts_.end()) {
            it->second.in_count = 0;
            it->second.out_count = 0;
            it->second.last_reset = now;
        }
    }
}

// ============================================================================
// Geometry helpers
// ============================================================================

float LineCrossingEngine::CrossProduct(float px, float py,
                                       float ax, float ay,
                                       float bx, float by) {
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

bool LineCrossingEngine::SegmentsCross(float px1, float py1, float px2, float py2,
                                       float ax, float ay, float bx, float by) {
    // Check if the movement segment (p1→p2) intersects the line segment (a→b)
    float d1 = CrossProduct(px1, py1, ax, ay, bx, by);
    float d2 = CrossProduct(px2, py2, ax, ay, bx, by);

    // Object must be on different sides of the line
    if (d1 * d2 >= 0) return false;

    // Line endpoints must be on different sides of the object path
    float d3 = CrossProduct(ax, ay, px1, py1, px2, py2);
    float d4 = CrossProduct(bx, by, px1, py1, px2, py2);

    return (d3 * d4 < 0);
}

std::string LineCrossingEngine::GetDirection(float px1, float py1,
                                              float px2, float py2,
                                              float ax, float ay,
                                              float bx, float by) {
    // Direction is determined by which side the object was on before crossing
    // A→B means the object moved from the "left" side to the "right" side
    // of the line vector (a→b)
    float cp = CrossProduct(px1, py1, ax, ay, bx, by);
    return (cp > 0) ? "a_to_b" : "b_to_a";
}

} // namespace ai
} // namespace ipcam
