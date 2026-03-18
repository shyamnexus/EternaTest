/**
 * @file motion_detection.h
 * @brief Hardware-accelerated Motion Detection Module
 * 
 * Uses Novatek libmd for efficient motion detection:
 * - Global alarm (full frame)
 * - Sub-region alarms (zones)
 * - Line crossing detection
 * - Object-based motion
 * - Background model (MDBC)
 * 
 * Based on: alg_md_crossline, alg_md_subalarm, alg_md_objdet, alg_md_mdbc
 */

#ifndef IPCAM_MOTION_DETECTION_H
#define IPCAM_MOTION_DETECTION_H

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <memory>
#include <cstdint>
#include <map>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
#include "hd_common.h"
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// Motion Detection Types
// ============================================================================

/**
 * @brief Motion detection mode
 */
enum class MotionMode {
    kGlobalAlarm = 0,    // Full frame motion
    kSubAlarm = 1,       // Zone-based motion
    kObjectDetect = 2,   // Motion with object tracking
    kCrossLine = 3,      // Line crossing
    kMdbc = 4            // Background model
};

/**
 * @brief Cross line direction
 */
enum class MdCrossDirection {
    kUnknown = 0,
    kLeftToRight = 1,
    kRightToLeft = 2,
    kTopToBottom = 3,
    kBottomToTop = 4
};

/**
 * @brief Motion zone definition (pixel coordinates for libmd)
 */
struct MdZone {
    uint32_t id = 0;
    std::string name;
    
    // Pixel coordinates for sub-alarm regions
    uint32_t x_start = 0;
    uint32_t y_start = 0;
    uint32_t x_end = 0;
    uint32_t y_end = 0;
    
    uint8_t threshold = 30;     // Sub-alarm threshold (0-255)
    int sensitivity = 50;       // 0-100
    bool enabled = true;
    
    // Loitering detection settings
    bool loitering_enabled = false;
    uint32_t loitering_threshold_ms = 10000;  // 10 seconds default
};

/**
 * @brief Line crossing definition with counting
 */
struct MdCrossLine {
    uint32_t id = 0;
    std::string name;
    
    // Pixel coordinates
    int x1 = 0, y1 = 0;
    int x2 = 0, y2 = 0;
    
    MdCrossDirection direction = MdCrossDirection::kUnknown;
    bool enabled = true;
    
    // Counting settings
    bool counting_enabled = false;
    uint32_t count_reset_interval_sec = 86400;  // Reset daily by default
};

/**
 * @brief Detected motion object
 */
struct MdDetectedObject {
    uint32_t id = 0;
    int x = 0, y = 0;
    int width = 0, height = 0;
    float confidence = 0.0f;
};

/**
 * @brief Zone motion result
 */
struct MdZoneResult {
    uint32_t zone_id = 0;
    bool motion_detected = false;
    float motion_level = 0.0f;
};

/**
 * @brief Cross line event
 */
struct MdCrossLineEvent {
    uint32_t line_id = 0;
    uint32_t object_id = 0;
    MdCrossDirection direction = MdCrossDirection::kUnknown;
    uint64_t timestamp_ms = 0;
};

/**
 * @brief Loitering event (object stayed in zone too long)
 */
struct MdLoiteringEvent {
    uint32_t zone_id = 0;
    uint32_t object_id = 0;
    uint64_t dwell_time_ms = 0;     ///< How long object has been in zone
    uint64_t threshold_ms = 0;       ///< Configured threshold that was exceeded
    uint64_t timestamp_ms = 0;
};

/**
 * @brief Line crossing counter state
 */
struct MdLineCrossCount {
    uint32_t line_id = 0;
    uint32_t count_in = 0;           ///< Objects crossed in "in" direction
    uint32_t count_out = 0;          ///< Objects crossed in "out" direction
    uint64_t last_reset_time = 0;    ///< Timestamp of last counter reset
};

/**
 * @brief Zone object tracking for loitering
 */
struct MdZoneObjectState {
    uint32_t object_id = 0;
    uint32_t zone_id = 0;
    uint64_t enter_time_ms = 0;      ///< When object entered the zone
    bool loitering_triggered = false; ///< Already triggered loitering event
};

/**
 * @brief Motion detection result
 */
struct MdResult {
    bool motion_detected = false;
    float motion_level = 0.0f;
    
    // Zone results
    std::vector<MdZoneResult> zone_results;
    
    // Detected objects (for object mode)
    std::vector<MdDetectedObject> detected_objects;
    
    // Line crossing events
    std::vector<MdCrossLineEvent> line_events;
    
    // Loitering events (objects in zone beyond threshold)
    std::vector<MdLoiteringEvent> loitering_events;
    
    // Line crossing counts (cumulative)
    std::vector<MdLineCrossCount> line_counts;
    
    // Statistics
    int motion_block_count = 0;
    int total_blocks = 0;
    
    uint64_t timestamp_ms = 0;
};

/**
 * @brief Motion detection configuration
 */
struct MdConfig {
    bool enabled = true;
    MotionMode mode = MotionMode::kSubAlarm;
    
    // Input resolution
    int width = 320;
    int height = 180;
    
    // Sensitivity (1=low, 2=medium, 3=high)
    int sensitivity = 2;
    
    // Global alarm threshold
    uint8_t global_threshold = 30;
    
    // Cooldown between events
    int cooldown_ms = 2000;
};

// ============================================================================
// Callbacks
// ============================================================================

using MdCallback = std::function<void(const MdResult&)>;

// ============================================================================
// Motion Detection Engine
// ============================================================================

class MotionDetectionEngine {
public:
    static MotionDetectionEngine& Instance();
    
    MotionDetectionEngine(const MotionDetectionEngine&) = delete;
    MotionDetectionEngine& operator=(const MotionDetectionEngine&) = delete;
    
    // Lifecycle
    bool Init(const MdConfig& config);
    bool Start();
    void Stop();
    void Shutdown();
    bool IsRunning() const { return running_.load(); }
    bool IsInitialized() const { return initialized_.load(); }
    
    // Configuration
    MdConfig GetConfig() const;
    bool SetConfig(const MdConfig& config);
    
    // Zone management
    void AddZone(const MdZone& zone);
    void RemoveZone(int zone_id);
    void ClearZones();
    
    // Line management
    void AddCrossLine(const MdCrossLine& line);
    void RemoveCrossLine(int line_id);
    void ClearCrossLines();
    
    // Process frame
    MdResult ProcessFrame(const uint8_t* y_data, int width, int height);
    
    // Callback
    void SetCallback(MdCallback callback);
    
    // Line crossing counts
    std::vector<MdLineCrossCount> GetLineCounts() const;
    MdLineCrossCount GetLineCount(uint32_t line_id) const;
    void ResetLineCounts();
    void ResetLineCount(uint32_t line_id);
    
    // Loitering callbacks
    using LoiteringCallback = std::function<void(const MdLoiteringEvent&)>;
    void SetLoiteringCallback(LoiteringCallback callback);
    
    // Statistics
    struct Stats {
        uint64_t frames_processed = 0;
        uint64_t motion_events = 0;
        uint64_t loitering_events = 0;
        uint64_t line_cross_events = 0;
        float avg_processing_time_ms = 0.0f;
        MdResult last_result;
    };
    Stats GetStats() const;
    void ResetStats();

private:
    MotionDetectionEngine();
    ~MotionDetectionEngine();
    
#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
    bool AllocateBuffers();
    void CleanupBuffers();
    
    // SDK integration functions (using correct libmd API signatures)
    int RunMdbc(int width, int height);
    bool RunGlobalAlarm();
    void RunSubAlarm(MdResult& result);
    void RunObjectDetection(MdResult& result);
    void RunCrossLineDetection(MdResult& result);
    
    // Loitering detection
    void UpdateLoiteringState(MdResult& result);
    void UpdateLineCounts(MdResult& result);
#endif

    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    mutable std::mutex mutex_;
    
    MdConfig config_;
    Stats stats_;
    MdCallback callback_;
    LoiteringCallback loitering_callback_;
    
    // Internal MD processing resolution (for memory efficiency)
    uint32_t md_internal_width_ = 160;
    uint32_t md_internal_height_ = 120;
    float md_scale_x_ = 1.0f;  // config_.width / md_internal_width_
    float md_scale_y_ = 1.0f;  // config_.height / md_internal_height_
    
    // Zones and lines
    std::vector<MdZone> zones_;
    std::vector<MdCrossLine> cross_lines_;
    
    // Line crossing counts (persistent between frames)
    std::map<uint32_t, MdLineCrossCount> line_counts_;
    
    // Loitering state tracking (object_id -> zone tracking state)
    std::vector<MdZoneObjectState> zone_object_states_;
    
    bool is_init_mode_ = true;

#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
    // Memory buffers (allocated via hd_common_mem_alloc)
    UINTPTR src_buf_pa_ = 0;
    UINTPTR src_buf_va_ = 0;
    uint32_t src_buf_size_ = 0;
    
    UINTPTR dst_buf_pa_ = 0;
    UINTPTR dst_buf_va_ = 0;
    uint32_t dst_buf_size_ = 0;
    
    UINTPTR temp_buf_pa_ = 0;
    UINTPTR temp_buf_va_ = 0;
    uint32_t temp_buf_size_ = 0;
    
    UINTPTR objpt_buf_pa_ = 0;
    UINTPTR objpt_buf_va_ = 0;
    uint32_t objpt_buf_size_ = 0;
    
    // Cross-line tracking state (persists between frames)
    uint8_t cross_prev_obj_num_ = 0;
#endif
};

// ============================================================================
// Helper Functions
// ============================================================================

std::string MotionModeToString(MotionMode mode);
std::string MdCrossDirectionToString(MdCrossDirection dir);

} // namespace ai
} // namespace ipcam

#endif // IPCAM_MOTION_DETECTION_H
