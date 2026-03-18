/**
 * @file heat_map.cpp
 * @brief Detection Heat Map — grid accumulation with exponential decay
 *
 * Ported from ai3_features/12_heat_map demo.
 */
#include "ipcam/heat_map.h"
#include "ipcam/analytics.h"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <cstring>

namespace ipcam {
namespace ai {

bool HeatMapEngine::Init(const HeatMapConfig& config) {
    config_ = config;
    grid_.assign(config_.grid_w * config_.grid_h, 0.0f);
    max_heat_ = 1.0f;
    initialized_ = true;
    spdlog::info("HeatMapEngine initialized ({}x{}, decay={:.3f})",
                 config_.grid_w, config_.grid_h, config_.decay_rate);
    return true;
}

void HeatMapEngine::Shutdown() {
    initialized_ = false;
    grid_.clear();
}

void HeatMapEngine::Accumulate(const std::vector<DetectionResult>& detections) {
    if (!initialized_) return;
    std::lock_guard<std::mutex> lock(mutex_);

    int gw = config_.grid_w;
    int gh = config_.grid_h;

    // Exponential decay
    for (auto& v : grid_) v *= config_.decay_rate;

    // Accumulate centroids
    for (const auto& det : detections) {
        if (det.category == ObjectCategory::kMotion) continue;
        float cx = det.bbox.CenterX();
        float cy = det.bbox.CenterY();
        int gx = static_cast<int>(cx * gw);
        int gy = static_cast<int>(cy * gh);
        if (gx < 0) gx = 0; if (gx >= gw) gx = gw - 1;
        if (gy < 0) gy = 0; if (gy >= gh) gy = gh - 1;
        grid_[gy * gw + gx] += 1.0f;
    }

    // Also accumulate bounding box area (each covered cell gets a partial increment)
    // This gives richer heat than centroid-only
    for (const auto& det : detections) {
        if (det.category == ObjectCategory::kMotion) continue;
        int x1 = static_cast<int>(det.bbox.x1 * gw);
        int y1 = static_cast<int>(det.bbox.y1 * gh);
        int x2 = static_cast<int>(det.bbox.x2 * gw);
        int y2 = static_cast<int>(det.bbox.y2 * gh);
        x1 = std::max(0, std::min(x1, gw - 1));
        y1 = std::max(0, std::min(y1, gh - 1));
        x2 = std::max(0, std::min(x2, gw - 1));
        y2 = std::max(0, std::min(y2, gh - 1));
        float area_incr = 0.2f;  // Lighter weight for area coverage
        for (int gy = y1; gy <= y2; gy++)
            for (int gx = x1; gx <= x2; gx++)
                grid_[gy * gw + gx] += area_incr;
    }

    // Track max for normalisation
    float mx = 1.0f;
    for (const auto& v : grid_) mx = std::max(mx, v);
    max_heat_ = mx;
}

std::vector<float> HeatMapEngine::GetGrid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<float> normalised(grid_.size());
    float inv = 1.0f / max_heat_;
    for (size_t i = 0; i < grid_.size(); i++)
        normalised[i] = grid_[i] * inv;
    return normalised;
}

void HeatMapEngine::Reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::fill(grid_.begin(), grid_.end(), 0.0f);
    max_heat_ = 1.0f;
}

} // namespace ai
} // namespace ipcam
