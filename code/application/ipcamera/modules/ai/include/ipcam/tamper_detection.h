/**
 * @file tamper_detection.h
 * @brief Video Tampering Detection Module using IVE Hardware Acceleration
 * 
 * Professional-grade camera tampering detection using Novatek IVE engine:
 * - Defocus detection (Laplacian variance)
 * - Masking/occlusion detection (low brightness + low edge content)
 * - Scene change detection (histogram comparison + edge structure)
 * - Exposure issues (too dark / too bright)
 * 
 * Features:
 * - IVE-accelerated histogram (~230µs) and Sobel edge detection (~190µs)
 * - Dwell time for alarm persistence (avoid transient triggers)
 * - Adaptive reference with day/night transition handling
 * - Auto-reset for scene changes (but NOT when lens covered)
 * 
 * Based on: vqa_tamper_stream sample with IVE acceleration
 */

#ifndef IPCAM_TAMPER_DETECTION_H
#define IPCAM_TAMPER_DETECTION_H

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <memory>
#include <thread>
#include <cstdint>
#include <deque>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
#include "hd_common.h"
#include "libive/libive.h"
#include "vendor_ive.h"
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// Tamper Detection Types
// ============================================================================

/**
 * @brief Types of tampering that can be detected
 */
enum class TamperType : uint32_t {
    kNone        = 0,
    kDefocus     = (1 << 0),  ///< Lens out of focus / blurred
    kMasking     = (1 << 1),  ///< Lens covered / spray paint
    kSceneChange = (1 << 2),  ///< Camera moved / rotated
    kTooDark     = (1 << 3),  ///< Underexposure
    kTooBright   = (1 << 4),  ///< Overexposure
};

// Allow bitwise operations on TamperType
inline TamperType operator|(TamperType a, TamperType b) {
    return static_cast<TamperType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline TamperType operator&(TamperType a, TamperType b) {
    return static_cast<TamperType>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline TamperType& operator|=(TamperType& a, TamperType b) {
    return a = a | b;
}
inline TamperType operator~(TamperType a) {
    return static_cast<TamperType>(~static_cast<uint32_t>(a));
}
inline bool HasFlag(TamperType value, TamperType flag) {
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * @brief VQA (Video Quality Analysis) issues
 */
enum class VqaIssues : uint32_t {
    kNone             = 0,
    kLowContrast      = (1 << 0),  ///< Low image contrast
    kExcessiveNoise   = (1 << 1),  ///< High noise level
    kFrozenVideo      = (1 << 2),  ///< Video appears frozen
    kSingleColorFrame = (1 << 3),  ///< Single uniform color
    kColorCast        = (1 << 4),  ///< Abnormal color tint
    kExcessiveArtifacts = (1 << 5), ///< Compression artifacts
};

// Bitwise operations for VqaIssues
inline VqaIssues operator|(VqaIssues a, VqaIssues b) {
    return static_cast<VqaIssues>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline VqaIssues operator&(VqaIssues a, VqaIssues b) {
    return static_cast<VqaIssues>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline VqaIssues& operator|=(VqaIssues& a, VqaIssues b) {
    return a = a | b;
}
inline bool HasFlag(VqaIssues value, VqaIssues flag) {
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * @brief Tamper detection sensitivity levels
 */
enum class TamperSensitivity {
    kLow = 0,     ///< Less sensitive - fewer false alarms
    kMedium = 1,  ///< Balanced sensitivity (default)
    kHigh = 2     ///< More sensitive - catches subtle changes
};

/**
 * @brief Tamper detection configuration
 */
struct TamperConfig {
    bool enabled = true;
    int width = 320;            ///< Input frame width (fixed at 320)
    int height = 180;           ///< Input frame height (fixed at 180)
    int fps = 5;                ///< Processing frame rate
    
    // Overall sensitivity (adjusts multiple thresholds)
    TamperSensitivity sensitivity = TamperSensitivity::kMedium;
    
    // Defocus detection (Laplacian variance)
    struct {
        bool enabled = true;
        float threshold = 500.0f;  ///< Laplacian variance threshold (lower = more blurry)
    } defocus;
    
    // Masking detection (lens covered)
    struct {
        bool enabled = true;
        float dark_threshold = 30.0f;      ///< Mean brightness threshold
        float edge_low_threshold = 0.001f; ///< Minimum edge ratio for non-covered
    } masking;
    
    // Scene change detection
    struct {
        bool enabled = true;
        float histogram_threshold = 0.02f; ///< Normalized histogram difference
        int edge_change_percent = 2;       ///< Percentage of edge structure change
    } scene_change;
    
    // Exposure detection
    struct {
        bool enabled = true;
        int too_dark_threshold = 25;       ///< Mean brightness for underexposure
        float too_dark_coverage = 0.80f;   ///< Coverage threshold for dark
        int too_bright_threshold = 230;    ///< Mean brightness for overexposure
        float too_bright_coverage = 0.20f; ///< Coverage threshold for bright
    } exposure;
    
    // Dwell time (frames of consistent detection before alarm)
    int dwell_frames = 5;
    
    // Reference frame management
    int reference_update_interval_sec = 10;  ///< Auto-update reference interval
    
    // Auto-reset configuration
    struct {
        bool enabled = true;
        int delay_sec = 5;                   ///< Delay before auto-reset on scene change
        bool protect_on_masking = true;      ///< Don't auto-reset if lens is covered
    } auto_reset;
    
    // Day/night transition handling
    struct {
        bool enabled = true;
        float gradual_change_threshold = 5.0f; ///< Max brightness change per second
        int transition_cooldown_sec = 3;       ///< Suppress alarms during transition
    } day_night;
};

/**
 * @brief Detection result with detailed metrics
 */
struct TamperResult {
    TamperType alarms = TamperType::kNone;  ///< Active alarms (bitfield)
    VqaIssues vqa_issues = VqaIssues::kNone; ///< VQA issues (bitfield)
    uint64_t timestamp = 0;                  ///< Detection timestamp (ms)
    
    // Detailed metrics for debugging/logging
    float laplacian_variance = 0.0f;  ///< Focus metric (higher = sharper)
    float mean_brightness = 0.0f;     ///< Average Y value (0-255)
    float histogram_diff = 0.0f;      ///< Chi-square distance from reference
    int edge_change_percent = 0;      ///< Edge structure change percentage
    float edge_ratio = 0.0f;          ///< Ratio of edge pixels
    
    // Enhanced metrics
    float histogram_entropy = 0.0f;   ///< Histogram entropy (uniformity measure)
    float local_contrast = 0.0f;      ///< Block-based local contrast
    float noise_level = 0.0f;         ///< Estimated noise level
    float fingerprint_diff = 0.0f;    ///< Scene fingerprint difference
    
    // State information
    bool reference_valid = false;     ///< Whether reference frame is set
    bool in_day_night_transition = false; ///< Suppressing due to gradual change
};

/**
 * @brief Statistics for monitoring
 */
struct TamperStats {
    uint64_t frames_processed = 0;
    uint64_t defocus_events = 0;
    uint64_t masking_events = 0;
    uint64_t scene_change_events = 0;
    uint64_t too_dark_events = 0;
    uint64_t too_bright_events = 0;
    uint64_t reference_updates = 0;
    uint64_t auto_resets = 0;
    
    // Timing statistics (microseconds)
    uint64_t avg_histogram_time_us = 0;
    uint64_t avg_sobel_time_us = 0;
    uint64_t avg_total_time_us = 0;
};

// ============================================================================
// Callback Types
// ============================================================================

/**
 * @brief Callback for tamper alarm state changes
 * @param type The tamper type that changed
 * @param active True if alarm started, false if cleared
 * @param result Full detection result with metrics
 */
using TamperCallback = std::function<void(TamperType type, bool active, const TamperResult& result)>;

// ============================================================================
// TamperDetectionEngine Class
// ============================================================================

/**
 * @brief IVE-accelerated video tampering detection engine
 * 
 * Singleton class that manages tamper detection using Novatek IVE hardware.
 * Designed to integrate with HdalPipeline and AnalyticsEngine.
 * 
 * Usage:
 *   auto& tamper = TamperDetectionEngine::Instance();
 *   tamper.Init(config);
 *   tamper.SetVideoProcPath(pipeline.GetVqaPath());
 *   tamper.Start();
 *   // ... later
 *   tamper.Stop();
 *   tamper.Shutdown();
 */
class TamperDetectionEngine {
public:
    /**
     * @brief Get singleton instance
     */
    static TamperDetectionEngine& Instance();
    
    // Prevent copying
    TamperDetectionEngine(const TamperDetectionEngine&) = delete;
    TamperDetectionEngine& operator=(const TamperDetectionEngine&) = delete;
    
    // ========================================================================
    // Lifecycle
    // ========================================================================
    
    /**
     * @brief Initialize the tamper detection engine
     * @param config Configuration parameters
     * @return true on success
     */
    bool Init(const TamperConfig& config);
    
    /**
     * @brief Start detection processing
     * @return true on success
     */
    bool Start();
    
    /**
     * @brief Stop detection processing
     */
    void Stop();
    
    /**
     * @brief Shutdown and release all resources
     */
    void Shutdown();
    
    /**
     * @brief Check if engine is running
     */
    bool IsRunning() const { return running_.load(); }
    
    /**
     * @brief Check if engine is initialized
     */
    bool IsInitialized() const { return initialized_.load(); }
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * @brief Get current configuration
     */
    TamperConfig GetConfig() const;
    
    /**
     * @brief Update configuration (can be called at runtime)
     * @param config New configuration
     * @return true on success
     */
    bool SetConfig(const TamperConfig& config);
    
    /**
     * @brief Set sensitivity level (convenience method)
     * @param sensitivity Sensitivity level
     */
    void SetSensitivity(TamperSensitivity sensitivity);
    
    // ========================================================================
    // Video Input
    // ========================================================================
    
    /**
     * @brief Set the HDAL VideoProc path for frame input
     * @param proc_path Path ID from HdalPipeline::GetVqaPath()
     */
    void SetVideoProcPath(uint64_t proc_path);
    
    /**
     * @brief Process a single frame manually (for testing or external input)
     * @param y_data Y-plane data (320x180 = 57600 bytes)
     * @param width Frame width (must be 320)
     * @param height Frame height (must be 180)
     * @return Detection result
     */
    TamperResult ProcessFrame(const uint8_t* y_data, int width, int height);
    
    // ========================================================================
    // Reference Frame Management
    // ========================================================================
    
    /**
     * @brief Manually reset the reference frame
     * 
     * Call this when you know the scene has intentionally changed
     * (e.g., after PTZ movement, camera adjustment).
     */
    void ResetReference();
    
    /**
     * @brief Check if reference frame is valid
     */
    bool HasValidReference() const;
    
    // ========================================================================
    // Callbacks
    // ========================================================================
    
    /**
     * @brief Set callback for alarm state changes
     * @param callback Function to call on alarm start/clear
     */
    void SetCallback(TamperCallback callback);
    
    // ========================================================================
    // Statistics
    // ========================================================================
    
    /**
     * @brief Get detection statistics
     */
    TamperStats GetStats() const;
    
    /**
     * @brief Reset statistics counters
     */
    void ResetStats();
    
    /**
     * @brief Get current detection result (last processed frame)
     */
    TamperResult GetLastResult() const;
    
    /**
     * @brief Get current active alarms
     */
    TamperType GetCurrentAlarms() const;
    
private:
    TamperDetectionEngine();
    ~TamperDetectionEngine();
    
    // ========================================================================
    // Internal Processing
    // ========================================================================
    
    void ProcessingThread();
    
    // IVE-accelerated operations
    bool InitIve();
    void CleanupIve();
    void CalculateHistogramIve(const uint8_t* y_data, uint32_t* histogram);
    void CalculateSobelIve(const uint8_t* y_data, uint8_t* edge_output);
    float CalculateLaplacianVarianceIve(const uint8_t* y_data);  // IVE-accelerated
    float CalculateFrameDiffIve(const uint8_t* y_data);  // IVE IMG_OP SUB_ABS
    
    // CPU operations (for metrics that IVE doesn't support directly)
    float CalculateLaplacianVarianceCpu(const uint8_t* y_data);  // Fallback
    float CalculateMeanBrightnessFromHistogram(const uint32_t* histogram);  // Use histogram
    int CountEdgePixels(const uint8_t* edge_data, int threshold);
    
    // Histogram comparison
    float CalculateHistogramChiSquare(const uint32_t* hist1, const uint32_t* hist2);
    void NormalizeHistogram(const uint32_t* hist_in, float* hist_out);
    
    // Temporal smoothing
    float GetTemporalSmoothedValue(std::deque<float>& history, float new_value);
    
    // Reference frame management
    void UpdateReference(const uint8_t* y_data);
    bool ShouldAutoResetReference(TamperType current_alarms);
    bool IsGradualBrightnessChange(float current_mean);
    
    // Alarm state management
    void UpdateDwellCounters(TamperType current_frame_alarms);
    void CheckAndFireCallbacks(TamperType new_alarms, const TamperResult& result);
    
    // Sensitivity adjustment
    void ApplySensitivityLevel(TamperSensitivity level);
    
    // ========================================================================
    // State
    // ========================================================================
    
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::thread processing_thread_;
    mutable std::mutex mutex_;
    
    TamperConfig config_;
    TamperCallback callback_;
    TamperStats stats_;
    TamperResult last_result_;
    
    // Dwell time counters
    int defocus_dwell_count_ = 0;
    int masking_dwell_count_ = 0;
    int scene_change_dwell_count_ = 0;
    int too_dark_dwell_count_ = 0;
    int too_bright_dwell_count_ = 0;
    
    // Current alarm state (for change detection)
    TamperType current_alarms_ = TamperType::kNone;
    TamperType prev_alarms_ = TamperType::kNone;
    
    // Reference frame data
    bool reference_valid_ = false;
    std::vector<uint32_t> reference_histogram_;
    int reference_edge_count_ = 0;
    float reference_brightness_ = 128.0f;
    float reference_local_contrast_ = 0.0f;
    uint64_t reference_timestamp_ = 0;
    
    // Scene fingerprint (for camera moved detection)
    std::vector<float> reference_fingerprint_;
    std::vector<float> prev_fingerprint_;
    
    // Temporal smoothing buffers
    std::deque<float> brightness_history_;
    std::deque<float> edge_density_history_;
    std::deque<float> histogram_diff_history_;
    
    // Scene change timing (10s to trigger, 60s to reset)
    int scene_change_trigger_count_ = 0;
    int scene_change_clear_count_ = 0;
    bool scene_change_alarmed_ = false;
    
    // Mode switch cooldown
    int mode_switch_cooldown_ = 0;
    
    // Frozen video detection
    int frozen_frame_count_ = 0;
    
    // Frame counter
    uint64_t frame_count_ = 0;
    
    // VQA issues tracking
    VqaIssues current_vqa_issues_ = VqaIssues::kNone;
    
    // Day/night transition tracking
    float prev_brightness_ = 128.0f;
    uint64_t brightness_change_start_ = 0;
    bool in_day_night_transition_ = false;
    
    // Auto-reset tracking
    uint64_t scene_change_start_ = 0;
    bool auto_reset_pending_ = false;
    
#if HDAL_PIPELINE_ENABLED
    HD_PATH_ID proc_path_ = 0;
    
    // IVE memory blocks
    HD_COMMON_MEM_VB_BLK input_mem_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    HD_COMMON_MEM_VB_BLK histo_mem_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    HD_COMMON_MEM_VB_BLK sobel_mem_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    HD_COMMON_MEM_VB_BLK sobel_out_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    
    // IVE Laplacian filter buffers (for defocus detection)
    HD_COMMON_MEM_VB_BLK laplacian_out_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    
    // IVE frame diff buffers (for frozen/motion detection)
    HD_COMMON_MEM_VB_BLK prev_frame_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    HD_COMMON_MEM_VB_BLK diff_out_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    
    // Physical/virtual addresses
    UINTPTR input_pa_ = 0;
    UINTPTR input_va_ = 0;
    UINTPTR histo_pa_ = 0;
    UINTPTR histo_va_ = 0;
    UINTPTR sobel_pa_ = 0;
    UINTPTR sobel_va_ = 0;
    UINTPTR sobel_out_pa_ = 0;
    UINTPTR sobel_out_va_ = 0;
    UINTPTR laplacian_out_pa_ = 0;
    UINTPTR laplacian_out_va_ = 0;
    UINTPTR prev_frame_pa_ = 0;
    UINTPTR prev_frame_va_ = 0;
    UINTPTR diff_out_pa_ = 0;
    UINTPTR diff_out_va_ = 0;
    
    bool ive_initialized_ = false;
    bool prev_frame_valid_ = false;  // First frame has no previous
#endif
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Convert TamperType to string for logging
 */
std::string TamperTypeToString(TamperType type);

/**
 * @brief Convert VqaIssues to string for logging
 */
std::string VqaIssuesToString(VqaIssues issues);

/**
 * @brief Get list of all active tamper types from bitfield
 */
std::vector<TamperType> GetActiveTamperTypes(TamperType alarms);

/**
 * @brief Convert sensitivity level to string
 */
std::string SensitivityToString(TamperSensitivity sensitivity);

} // namespace ai
} // namespace ipcam

#endif // IPCAM_TAMPER_DETECTION_H
