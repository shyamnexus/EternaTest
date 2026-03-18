/**
 * @file object_tracking.h
 * @brief Multi-Object Tracking Module
 * 
 * Uses Novatek libtrke for robust object tracking:
 * - Multi-object tracking with persistent IDs
 * - Track lifecycle management
 * - Re-identification
 * - Track history for line crossing
 * 
 * Based on: alg_trke, sw_tracker
 */

#ifndef IPCAM_OBJECT_TRACKING_H
#define IPCAM_OBJECT_TRACKING_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <atomic>
#include <mutex>
#include <memory>
#include <deque>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// Tracking Types
// ============================================================================

/**
 * @brief Track state
 */
enum class TrackState {
    kTentative = 0,    // New track, not confirmed
    kConfirmed = 1,    // Confirmed track
    kLost = 2,         // Temporarily lost
    kDeleted = 3       // Marked for deletion
};

/**
 * @brief Single point in track history
 */
struct TrackPoint {
    float x = 0.0f, y = 0.0f;      // Center position (normalized)
    float width = 0.0f, height = 0.0f;
    uint64_t timestamp = 0;
};

/**
 * @brief Tracked object
 */
struct TrackedObject {
    uint32_t id = 0;                // Unique track ID
    int class_id = 0;               // Object class (COCO class id)
    float confidence = 0.0f;
    
    // Current state — stored as x1/y1/x2/y2 (normalised 0-1)
    // matching demo tracker TRK_OBJ format
    TrackState state = TrackState::kTentative;
    float x1 = 0.0f, y1 = 0.0f;
    float x2 = 0.0f, y2 = 0.0f;
    TrackPoint current;
    
    // Velocity (pixels per second)
    float vx = 0.0f, vy = 0.0f;
    
    // Track statistics
    int hit_count = 0;              // Detection hits
    int miss_count = 0;             // Consecutive misses
    int age = 0;                    // Frames since creation
    
    // History for trajectory analysis
    std::deque<TrackPoint> history;
    
    // Feature vector for re-identification (optional)
    std::vector<float> features;
};

/**
 * @brief Detection input for tracker
 */
struct Detection {
    float x1 = 0.0f, y1 = 0.0f;     // Bounding box (normalized)
    float x2 = 0.0f, y2 = 0.0f;
    int class_id = 0;
    float confidence = 0.0f;
    
    // Optional feature vector
    std::vector<float> features;
};

/**
 * @brief Tracking configuration
 */
struct TrackingConfig {
    bool enabled = true;
    
    // Association parameters
    float iou_threshold = 0.3f;          // Minimum IoU for matching
    float feature_threshold = 0.5f;      // Feature similarity threshold
    
    // Track lifecycle (matches demo: max_lost_frames=5, immediate activation)
    int min_hits = 1;                    // Hits to confirm track
    int max_age = 5;                     // Max frames without detection
    int history_size = 100;              // Track history length
    
    // Class-aware matching (like demo tracker class_aware=1)
    bool class_aware = true;
    
    // Kalman filter parameters (disabled by default to match demo)
    bool use_kalman = false;
    float process_noise = 0.01f;
    float measurement_noise = 0.1f;
    
    // Re-identification
    bool use_reid = false;
    std::string reid_model_path;
};

/**
 * @brief Tracking result
 */
struct TrackingResult {
    std::vector<TrackedObject> tracks;
    int new_tracks = 0;
    int lost_tracks = 0;
    int deleted_tracks = 0;
    uint64_t timestamp = 0;
};

// ============================================================================
// Callbacks
// ============================================================================

using TrackCallback = std::function<void(const TrackedObject& track, bool is_new)>;
using TrackLostCallback = std::function<void(uint32_t track_id)>;

// ============================================================================
// Object Tracking Engine
// ============================================================================

class ObjectTracker {
public:
    static ObjectTracker& Instance();
    
    ObjectTracker(const ObjectTracker&) = delete;
    ObjectTracker& operator=(const ObjectTracker&) = delete;
    
    // Lifecycle
    bool Init(const TrackingConfig& config);
    void Shutdown();
    bool IsInitialized() const { return initialized_.load(); }
    
    // Configuration
    TrackingConfig GetConfig() const;
    bool SetConfig(const TrackingConfig& config);
    
    /**
     * @brief Update tracker with new detections
     * @param detections New detections from AI
     * @param timestamp Frame timestamp
     * @return Updated tracking result
     */
    TrackingResult Update(const std::vector<Detection>& detections, uint64_t timestamp);
    
    /**
     * @brief Get all active tracks
     */
    std::vector<TrackedObject> GetTracks() const;
    
    /**
     * @brief Get specific track by ID
     */
    bool GetTrack(uint32_t id, TrackedObject& track) const;
    
    /**
     * @brief Clear all tracks
     */
    void Clear();
    
    // Callbacks
    void SetTrackCallback(TrackCallback callback);
    void SetTrackLostCallback(TrackLostCallback callback);
    
    // Statistics
    struct Stats {
        uint64_t frames_processed = 0;
        uint64_t total_tracks = 0;
        uint64_t active_tracks = 0;
        float avg_track_length = 0.0f;
    };
    Stats GetStats() const;
    void ResetStats();
    
private:
    ObjectTracker();
    ~ObjectTracker();
    
    // Core tracking methods
    void Predict();
    std::vector<std::pair<int, int>> Associate(const std::vector<Detection>& detections);
    void UpdateTracks(const std::vector<Detection>& detections, 
                      const std::vector<std::pair<int, int>>& matches,
                      uint64_t timestamp);
    void CreateNewTracks(const std::vector<Detection>& detections,
                         const std::vector<int>& unmatched_dets,
                         uint64_t timestamp);
    void ManageTrackLifecycle();
    
    // Utility methods
    float ComputeIoU(const TrackedObject& track, const Detection& det) const;
    float ComputeFeatureSimilarity(const TrackedObject& track, const Detection& det) const;
    
    std::atomic<bool> initialized_{false};
    mutable std::mutex mutex_;
    
    TrackingConfig config_;
    Stats stats_;
    
    // Active tracks
    std::map<uint32_t, TrackedObject> tracks_;
    uint32_t next_id_ = 1;
    
    // Callbacks
    TrackCallback track_callback_;
    TrackLostCallback track_lost_callback_;
    
    // Kalman filter state per track
    struct KalmanState {
        float x, y, w, h;      // Position and size
        float vx, vy;          // Velocity
        float P[6][6];         // Covariance matrix
    };
    std::map<uint32_t, KalmanState> kalman_states_;
};

// ============================================================================
// Helper Functions
// ============================================================================

std::string TrackStateToString(TrackState state);

} // namespace ai
} // namespace ipcam

#endif // IPCAM_OBJECT_TRACKING_H
