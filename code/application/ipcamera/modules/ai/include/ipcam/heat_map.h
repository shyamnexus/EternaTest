/**
 * @file heat_map.h
 * @brief Detection Heat Map — grid accumulation with exponential decay
 *
 * Maps detection centroids onto a fixed grid, accumulates with per-frame
 * exponential decay.  Exposes the grid for OSD rendering or export.
 */
#ifndef IPCAM_HEAT_MAP_H_
#define IPCAM_HEAT_MAP_H_

#include <vector>
#include <mutex>
#include <cstdint>

namespace ipcam {
namespace ai {

struct DetectionResult;

struct HeatMapConfig {
    bool enabled = false;
    int grid_w = 32;
    int grid_h = 18;
    float decay_rate = 0.97f;  ///< Per-frame multiplicative decay
};

class HeatMapEngine {
public:
    bool Init(const HeatMapConfig& config);
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    bool IsEnabled() const { return initialized_ && config_.enabled; }

    /// Accumulate detection centroids (call once per frame)
    void Accumulate(const std::vector<DetectionResult>& detections);

    /// Get current grid (values normalised to 0–1 by max_heat_)
    std::vector<float> GetGrid() const;

    /// Get raw grid dimensions
    int GridW() const { return config_.grid_w; }
    int GridH() const { return config_.grid_h; }

    void Reset();

private:
    HeatMapConfig config_;
    bool initialized_ = false;
    mutable std::mutex mutex_;
    std::vector<float> grid_;
    float max_heat_ = 1.0f;
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_HEAT_MAP_H_
