/**
 * @file vqa.h
 * @brief Video Quality Analysis Module
 * 
 * Detects video quality issues using Novatek VQA library:
 * - Too bright (overexposure)
 * - Too dark (underexposure)
 * - Blur detection
 * - Camera tampering (lens covered)
 * 
 * Based on: alg_vqa_sample
 */

#ifndef IPCAM_VQA_H
#define IPCAM_VQA_H

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <memory>
#include <thread>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// VQA Types
// ============================================================================

/**
 * @brief Video quality issue types
 */
enum class VqaIssueType {
    kNone = 0,
    kTooBright = 1,
    kTooDark = 2,
    kBlurred = 3,
    kTampered = 4,
    kColorCast = 5
};

/**
 * @brief VQA detection result
 */
struct VqaResult {
    bool too_bright = false;
    bool too_dark = false;
    bool blurred = false;
    bool tampered = false;
    bool color_cast = false;
    
    // Detailed values
    float brightness_level = 0.0f;   // 0-255
    float blur_level = 0.0f;         // 0-100
    float blur_coverage = 0.0f;      // Percentage of blurred area
    
    uint64_t timestamp = 0;
};

/**
 * @brief VQA configuration
 */
struct VqaConfig {
    bool enabled = true;
    int width = 320;
    int height = 180;
    int fps = 5;  // VQA doesn't need high fps
    
    // Overexposure settings
    struct {
        bool enabled = true;
        int strength_threshold = 215;  // 0-255
        int coverage_threshold = 15;   // Percentage
        int alarm_frames = 5;          // Consecutive frames
    } too_bright;
    
    // Underexposure settings
    struct {
        bool enabled = true;
        int strength_threshold = 50;
        int coverage_threshold = 75;
        int alarm_frames = 5;
    } too_dark;
    
    // Blur settings
    struct {
        bool enabled = true;
        int strength = 30;
        int coverage_threshold = 50;
        int alarm_frames = 5;
    } blur;
    
    // General
    int global_alarm_frames = 20;
    int auto_adjust_period = 5;
};

// ============================================================================
// VQA Callback
// ============================================================================

using VqaCallback = std::function<void(const VqaResult&, VqaIssueType)>;

// ============================================================================
// VQA Engine Class
// ============================================================================

class VqaEngine {
public:
    static VqaEngine& Instance();
    
    VqaEngine(const VqaEngine&) = delete;
    VqaEngine& operator=(const VqaEngine&) = delete;
    
    // Lifecycle
    bool Init(const VqaConfig& config);
    bool Start();
    void Stop();
    void Shutdown();
    bool IsRunning() const { return running_.load(); }
    
    // Configuration
    VqaConfig GetConfig() const;
    bool SetConfig(const VqaConfig& config);
    
    // Process frame manually
    VqaResult ProcessFrame(const uint8_t* y_data, int width, int height);
    
    // Callback
    void SetCallback(VqaCallback callback);
    
    // Statistics
    struct Stats {
        uint64_t frames_processed = 0;
        uint64_t too_bright_events = 0;
        uint64_t too_dark_events = 0;
        uint64_t blur_events = 0;
        uint64_t tamper_events = 0;
    };
    Stats GetStats() const;
    void ResetStats();
    
    // Set HDAL path for frame input
    void SetVideoProcPath(uint64_t proc_path);
    
private:
    VqaEngine();
    ~VqaEngine();
    
    void ProcessingThread();
    bool InitVqaLib();
    void CleanupVqaLib();
    
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::thread processing_thread_;
    mutable std::mutex mutex_;
    
    VqaConfig config_;
    VqaCallback callback_;
    Stats stats_;
    
    // VQA library state
    bool vqa_lib_initialized_ = false;
    
#if HDAL_PIPELINE_ENABLED
    HD_PATH_ID proc_path_ = 0;
    HD_COMMON_MEM_VB_BLK vqa_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    HD_COMMON_MEM_VB_BLK y_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    UINTPTR vqa_buf_pa_ = 0;
    UINTPTR vqa_buf_va_ = 0;
    UINTPTR y_buf_pa_ = 0;
    UINTPTR y_buf_va_ = 0;
#endif
    
    // Issue tracking
    VqaIssueType current_issue_ = VqaIssueType::kNone;
    int issue_frame_count_ = 0;
};

// ============================================================================
// Helper Functions
// ============================================================================

std::string VqaIssueTypeToString(VqaIssueType type);

} // namespace ai
} // namespace ipcam

#endif // IPCAM_VQA_H
