/**
 * @file line_crossing.h
 * @brief Line crossing detection engine
 *
 * Detects when tracked objects cross defined lines.
 * Consumes tracked object positions from ObjectTracker.
 * Uses cross-product geometry to detect line crossings
 * with direction awareness (A→B, B→A, both).
 */
#ifndef IPCAM_LINE_CROSSING_H_
#define IPCAM_LINE_CROSSING_H_

#include "ipcam/analytics.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>

namespace ipcam {
namespace ai {

/// Line crossing event
struct LineCrossEvent {
    uint32_t line_id;
    uint32_t track_id;
    ObjectCategory category;
    std::string direction;  // "a_to_b" or "b_to_a"
    int64_t timestamp;
};

/// Counting state per line
struct LineCounts {
    uint32_t line_id = 0;
    int in_count = 0;   // A→B
    int out_count = 0;  // B→A
    int64_t last_reset = 0;
};

using LineCrossEventCallback = std::function<void(const LineCrossEvent&)>;

/**
 * @brief Line crossing detection engine
 *
 * Maintains per-object position history. Each frame, checks whether
 * any tracked object's centroid has crossed any configured line since
 * the previous frame.
 *
 * Cross detection: sign change of cross product (p-a)×(b-a) between
 * two consecutive frames means the object crossed the line.
 */
class LineCrossingEngine {
public:
    void Init(const LineCrossingConfig& config);
    void Shutdown();

    /// Check tracked objects against configured lines
    /// @param objects Current tracked objects with bounding boxes
    /// @return Events generated this frame
    std::vector<LineCrossEvent> Check(const std::vector<DetectionResult>& objects);

    void SetConfig(const LineCrossingConfig& config);
    bool IsEnabled() const { return config_.enabled; }

    void SetCallback(LineCrossEventCallback cb) { callback_ = std::move(cb); }

    /// Get current counts for a line
    LineCounts GetCounts(uint32_t line_id) const;

    /// Get all line counts
    std::vector<LineCounts> GetAllCounts() const;

    /// Reset counts for a line (or all if line_id=0)
    void ResetCounts(uint32_t line_id = 0);

private:
    LineCrossingConfig config_;
    LineCrossEventCallback callback_;
    mutable std::mutex mutex_;

    // Per-object previous centroid positions
    struct ObjectHistory {
        float prev_cx = -1.0f;
        float prev_cy = -1.0f;
        bool valid = false;
    };
    std::unordered_map<uint32_t, ObjectHistory> object_history_;

    // Per-line counting
    std::unordered_map<uint32_t, LineCounts> line_counts_;

    /// Cross product sign: (p-a) × (b-a)
    static float CrossProduct(float px, float py, float ax, float ay, float bx, float by);

    /// Check if a point movement from (px1,py1) to (px2,py2) crosses line (ax,ay)-(bx,by)
    static bool SegmentsCross(float px1, float py1, float px2, float py2,
                              float ax, float ay, float bx, float by);

    /// Determine crossing direction
    static std::string GetDirection(float px1, float py1, float px2, float py2,
                                    float ax, float ay, float bx, float by);
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_LINE_CROSSING_H_
