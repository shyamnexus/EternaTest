/**
 * @file object_tracking.cpp
 * @brief Multi-Object Tracking Implementation
 * 
 * Implements multi-object tracking using:
 * - IOU (Intersection over Union) matching
 * - Simple Kalman filtering for motion prediction
 * - Track lifecycle management (creation, update, deletion)
 * 
 * Based on: alg_trke sample (Lucas-Kanade optical flow)
 */

#include "ipcam/object_tracking.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>
#include <cmath>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_common.h"
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// ObjectTracker Implementation
// ============================================================================

ObjectTracker& ObjectTracker::Instance() {
    static ObjectTracker instance;
    return instance;
}

ObjectTracker::ObjectTracker() = default;

ObjectTracker::~ObjectTracker() {
    Shutdown();
}

bool ObjectTracker::Init(const TrackingConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("ObjectTracker: Already initialized");
        return true;
    }
    
    config_ = config;
    tracks_.clear();
    kalman_states_.clear();
    next_id_ = 1;
    stats_ = Stats{};
    
    initialized_.store(true);
    spdlog::info("ObjectTracker: Initialized (iou_thresh={}, min_hits={}, max_age={})",
                 config_.iou_threshold, config_.min_hits, config_.max_age);
    
    return true;
}

void ObjectTracker::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    tracks_.clear();
    kalman_states_.clear();
    initialized_.store(false);
    
    spdlog::info("ObjectTracker: Shutdown");
}

TrackingConfig ObjectTracker::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool ObjectTracker::SetConfig(const TrackingConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    return true;
}

TrackingResult ObjectTracker::Update(const std::vector<Detection>& detections, 
                                      uint64_t timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    TrackingResult result;
    result.timestamp = timestamp;
    
    if (!initialized_.load()) {
        return result;
    }
    
    // Step 1: Predict positions using Kalman filter
    Predict();
    
    // Step 1.5: Increment miss_count for all tracks (like demo: lost_cnt++ for all)
    for (auto& [id, track] : tracks_) {
        track.miss_count++;
    }
    
    // Step 2: Associate detections with existing tracks
    auto matches = Associate(detections);
    
    // Build list of unmatched detections
    std::vector<bool> det_matched(detections.size(), false);
    for (const auto& match : matches) {
        det_matched[match.second] = true;
    }
    std::vector<int> unmatched_dets;
    for (size_t i = 0; i < detections.size(); i++) {
        if (!det_matched[i]) {
            unmatched_dets.push_back(static_cast<int>(i));
        }
    }
    
    // Step 3: Update matched tracks (resets miss_count to 0)
    UpdateTracks(detections, matches, timestamp);
    
    // Step 4: Create new tracks for unmatched detections
    int prev_track_count = static_cast<int>(tracks_.size());
    CreateNewTracks(detections, unmatched_dets, timestamp);
    result.new_tracks = static_cast<int>(tracks_.size()) - prev_track_count;
    
    // Step 5: Manage track lifecycle (delete old tracks)
    ManageTrackLifecycle();
    
    // Build result
    for (const auto& [id, track] : tracks_) {
        if (track.state == TrackState::kConfirmed || 
            track.state == TrackState::kTentative) {
            result.tracks.push_back(track);
        }
    }
    
    stats_.frames_processed++;
    stats_.active_tracks = tracks_.size();
    
    return result;
}

std::vector<TrackedObject> ObjectTracker::GetTracks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<TrackedObject> result;
    for (const auto& [id, track] : tracks_) {
        result.push_back(track);
    }
    return result;
}

bool ObjectTracker::GetTrack(uint32_t id, TrackedObject& track) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = tracks_.find(id);
    if (it != tracks_.end()) {
        track = it->second;
        return true;
    }
    return false;
}

void ObjectTracker::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    tracks_.clear();
    kalman_states_.clear();
    next_id_ = 1;
}

void ObjectTracker::SetTrackCallback(TrackCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    track_callback_ = std::move(callback);
}

void ObjectTracker::SetTrackLostCallback(TrackLostCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    track_lost_callback_ = std::move(callback);
}

ObjectTracker::Stats ObjectTracker::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void ObjectTracker::ResetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Stats{};
}

// ============================================================================
// Private Methods
// ============================================================================

void ObjectTracker::Predict() {
    if (!config_.use_kalman) {
        return;
    }
    
    for (auto& [id, track] : tracks_) {
        auto it = kalman_states_.find(id);
        if (it == kalman_states_.end()) {
            continue;
        }
        
        KalmanState& ks = it->second;
        
        // Predict state: x = F * x
        ks.x += ks.vx;
        ks.y += ks.vy;
        
        // Update covariance: P = F * P * F' + Q
        float q = config_.process_noise;
        for (int i = 0; i < 6; i++) {
            ks.P[i][i] += q;
        }
        
        // Update track current position from prediction
        track.current.x = ks.x;
        track.current.y = ks.y;
    }
}

std::vector<std::pair<int, int>> ObjectTracker::Associate(
    const std::vector<Detection>& detections) {
    
    std::vector<std::pair<int, int>> matches;
    
    if (tracks_.empty() || detections.empty()) {
        return matches;
    }
    
    // Build track index list
    std::vector<uint32_t> track_ids;
    for (const auto& [id, track] : tracks_) {
        track_ids.push_back(id);
    }
    
    // Compute cost matrix (IoU)
    std::vector<std::vector<float>> cost_matrix(track_ids.size(),
                                                  std::vector<float>(detections.size(), 0.0f));
    
    for (size_t t = 0; t < track_ids.size(); t++) {
        const auto& track = tracks_.at(track_ids[t]);
        for (size_t d = 0; d < detections.size(); d++) {
            float iou = ComputeIoU(track, detections[d]);
            
            // Add feature similarity if available
            if (config_.use_reid && !track.features.empty() && 
                !detections[d].features.empty()) {
                float feat_sim = ComputeFeatureSimilarity(track, detections[d]);
                cost_matrix[t][d] = 0.7f * iou + 0.3f * feat_sim;
            } else {
                cost_matrix[t][d] = iou;
            }
        }
    }
    
    // Greedy matching (for proper Hungarian, would need full algorithm)
    std::vector<bool> det_matched(detections.size(), false);
    std::vector<bool> track_matched(track_ids.size(), false);
    
    // Sort all possible matches by cost (descending)
    std::vector<std::tuple<float, int, int>> all_costs;
    for (size_t t = 0; t < track_ids.size(); t++) {
        for (size_t d = 0; d < detections.size(); d++) {
            if (cost_matrix[t][d] >= config_.iou_threshold) {
                all_costs.push_back({cost_matrix[t][d], static_cast<int>(t), static_cast<int>(d)});
            }
        }
    }
    
    std::sort(all_costs.begin(), all_costs.end(),
              [](const auto& a, const auto& b) {
                  return std::get<0>(a) > std::get<0>(b);
              });
    
    // Greedily match highest cost pairs
    for (const auto& [cost, t, d] : all_costs) {
        if (!track_matched[t] && !det_matched[d]) {
            matches.push_back({static_cast<int>(track_ids[t]), d});
            track_matched[t] = true;
            det_matched[d] = true;
        }
    }
    
    return matches;
}

void ObjectTracker::UpdateTracks(const std::vector<Detection>& detections,
                                  const std::vector<std::pair<int, int>>& matches,
                                  uint64_t timestamp) {
    for (const auto& [track_id, det_idx] : matches) {
        auto it = tracks_.find(track_id);
        if (it == tracks_.end()) continue;
        
        TrackedObject& track = it->second;
        const Detection& det = detections[det_idx];
        
        // Store x1/y1/x2/y2 directly (matching demo TRK_OBJ format)
        float cx = (det.x1 + det.x2) / 2.0f;
        float cy = (det.y1 + det.y2) / 2.0f;
        float w = det.x2 - det.x1;
        float h = det.y2 - det.y1;
        
        // Calculate velocity from center movement
        track.vx = cx - track.current.x;
        track.vy = cy - track.current.y;
        
        // Update bbox coords (x1/y1/x2/y2)
        track.x1 = det.x1;
        track.y1 = det.y1;
        track.x2 = det.x2;
        track.y2 = det.y2;
        
        // Update current center+size (kept for history/trajectory)
        track.current.x = cx;
        track.current.y = cy;
        track.current.width = w;
        track.current.height = h;
        track.current.timestamp = timestamp;
        
        track.confidence = det.confidence;
        track.class_id = det.class_id;
        
        // Update counters
        track.hit_count++;
        track.miss_count = 0;
        track.age++;
        
        // Promote to confirmed if enough hits
        if (track.hit_count >= config_.min_hits && 
            track.state == TrackState::kTentative) {
            track.state = TrackState::kConfirmed;
            
            // Notify callback
            if (track_callback_) {
                track_callback_(track, true);
            }
        }
        
        // Update Kalman state
        if (config_.use_kalman) {
            auto kit = kalman_states_.find(track_id);
            if (kit != kalman_states_.end()) {
                KalmanState& ks = kit->second;
                
                // Kalman update with measurement
                float r = config_.measurement_noise;
                float k = ks.P[0][0] / (ks.P[0][0] + r);
                
                ks.x += k * (cx - ks.x);
                ks.y += k * (cy - ks.y);
                ks.w = w;
                ks.h = h;
                ks.vx = 0.8f * ks.vx + 0.2f * track.vx;
                ks.vy = 0.8f * ks.vy + 0.2f * track.vy;
                
                ks.P[0][0] *= (1 - k);
                ks.P[1][1] *= (1 - k);
            }
        }
        
        // Update history
        if (track.history.size() >= static_cast<size_t>(config_.history_size)) {
            track.history.pop_front();
        }
        track.history.push_back(track.current);
        
        // Update features
        if (!det.features.empty()) {
            track.features = det.features;
        }
    }
}

void ObjectTracker::CreateNewTracks(const std::vector<Detection>& detections,
                                     const std::vector<int>& unmatched_dets,
                                     uint64_t timestamp) {
    for (int idx : unmatched_dets) {
        const Detection& det = detections[idx];
        
        TrackedObject track;
        track.id = next_id_++;
        track.class_id = det.class_id;
        track.confidence = det.confidence;
        track.state = TrackState::kTentative;
        
        // Store x1/y1/x2/y2 directly (matching demo TRK_OBJ format)
        track.x1 = det.x1;
        track.y1 = det.y1;
        track.x2 = det.x2;
        track.y2 = det.y2;
        
        float cx = (det.x1 + det.x2) / 2.0f;
        float cy = (det.y1 + det.y2) / 2.0f;
        
        track.current.x = cx;
        track.current.y = cy;
        track.current.width = det.x2 - det.x1;
        track.current.height = det.y2 - det.y1;
        track.current.timestamp = timestamp;
        
        track.vx = 0;
        track.vy = 0;
        track.hit_count = 1;
        track.miss_count = 0;
        track.age = 1;
        
        track.history.push_back(track.current);
        track.features = det.features;
        
        tracks_[track.id] = track;
        stats_.total_tracks++;
        
        // Initialize Kalman state
        if (config_.use_kalman) {
            KalmanState ks = {};
            ks.x = cx;
            ks.y = cy;
            ks.w = track.current.width;
            ks.h = track.current.height;
            ks.vx = 0;
            ks.vy = 0;
            
            // Initialize covariance
            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < 6; j++) {
                    ks.P[i][j] = (i == j) ? 100.0f : 0.0f;
                }
            }
            
            kalman_states_[track.id] = ks;
        }
    }
}

void ObjectTracker::ManageTrackLifecycle() {
    std::vector<uint32_t> to_delete;
    
    for (auto& [id, track] : tracks_) {
        // Mark as lost if miss_count > 0 (set during UpdateTracks pass)
        if (track.miss_count > 0 && 
            (track.state == TrackState::kConfirmed || track.state == TrackState::kTentative)) {
            track.state = TrackState::kLost;
        }
        
        // Delete if exceeds max_age (demo: max_lost_frames = 5)
        if (track.miss_count > config_.max_age) {
            track.state = TrackState::kDeleted;
            to_delete.push_back(id);
            
            if (track_lost_callback_) {
                track_lost_callback_(id);
            }
        }
    }
    
    // Remove deleted tracks
    for (uint32_t id : to_delete) {
        tracks_.erase(id);
        kalman_states_.erase(id);
    }
}

float ObjectTracker::ComputeIoU(const TrackedObject& track, const Detection& det) const {
    // Class-aware: if classes differ, IoU = 0 (matching demo class_aware=1)
    if (config_.class_aware && track.class_id != det.class_id) {
        return 0.0f;
    }
    
    // Track box — use stored x1/y1/x2/y2 directly (like demo TRK_OBJ)
    float t_x1 = track.x1;
    float t_y1 = track.y1;
    float t_x2 = track.x2;
    float t_y2 = track.y2;
    
    // Detection box
    float d_x1 = det.x1;
    float d_y1 = det.y1;
    float d_x2 = det.x2;
    float d_y2 = det.y2;
    
    // Intersection
    float inter_x1 = std::max(t_x1, d_x1);
    float inter_y1 = std::max(t_y1, d_y1);
    float inter_x2 = std::min(t_x2, d_x2);
    float inter_y2 = std::min(t_y2, d_y2);
    
    float inter_w = std::max(0.0f, inter_x2 - inter_x1);
    float inter_h = std::max(0.0f, inter_y2 - inter_y1);
    float inter_area = inter_w * inter_h;
    
    // Union
    float t_area = (t_x2 - t_x1) * (t_y2 - t_y1);
    float d_area = (d_x2 - d_x1) * (d_y2 - d_y1);
    float union_area = t_area + d_area - inter_area;
    
    if (union_area < 1e-6f) {
        return 0.0f;
    }
    
    return inter_area / union_area;
}

float ObjectTracker::ComputeFeatureSimilarity(const TrackedObject& track, 
                                               const Detection& det) const {
    if (track.features.empty() || det.features.empty()) {
        return 0.0f;
    }
    
    if (track.features.size() != det.features.size()) {
        return 0.0f;
    }
    
    // Cosine similarity
    float dot = 0.0f;
    float norm_t = 0.0f;
    float norm_d = 0.0f;
    
    for (size_t i = 0; i < track.features.size(); i++) {
        dot += track.features[i] * det.features[i];
        norm_t += track.features[i] * track.features[i];
        norm_d += det.features[i] * det.features[i];
    }
    
    if (norm_t < 1e-6f || norm_d < 1e-6f) {
        return 0.0f;
    }
    
    return dot / (std::sqrt(norm_t) * std::sqrt(norm_d));
}

// ============================================================================
// Helper Functions
// ============================================================================

std::string TrackStateToString(TrackState state) {
    switch (state) {
        case TrackState::kTentative: return "Tentative";
        case TrackState::kConfirmed: return "Confirmed";
        case TrackState::kLost: return "Lost";
        case TrackState::kDeleted: return "Deleted";
        default: return "Unknown";
    }
}

} // namespace ai
} // namespace ipcam
