/**
 * @file aiisp.h
 * @brief AI-ISP Image Enhancement Module
 * 
 * Uses Novatek AI neural network for image enhancement:
 * - AI-based noise reduction (2DNR, 3DNR)
 * - Low-light enhancement
 * - WDR (Wide Dynamic Range)
 * - Defog
 * 
 * Based on: hd_video_liveview_with_aiisp, demo_video_liveview_with_aiisp
 */

#ifndef IPCAM_AIISP_H
#define IPCAM_AIISP_H

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
// AIISP Types
// ============================================================================

/**
 * @brief AI-ISP model selection
 */
enum class AiispModel {
    kDefault = 0,
    kLowLight = 1,      // Optimized for dark scenes
    kHighMotion = 2,    // Optimized for moving objects
    kLowNoise = 3       // Maximum noise reduction
};

/**
 * @brief AIISP PQ parameters (from JSON config)
 */
struct AiispPqParam {
    uint32_t version = 1;
    uint32_t mode = 0;              // 0=manual, 1=auto
    
    // Manual mode parameters
    uint32_t nr_2dnr_still_str = 50;
    uint32_t coef_a = 256;
    uint32_t coef_b = 256;
    
    // Auto mode parameters
    uint32_t nr_2dnr_still_str_auto = 50;
    uint32_t coef_a_auto = 256;
    uint32_t coef_b_auto = 256;
    
    // Range limits
    uint32_t coef_a_min = 0;
    uint32_t coef_a_max = 1024;
    uint32_t coef_b_min = 0;
    uint32_t coef_b_max = 1024;
};

/**
 * @brief Noise profile for AI model
 */
struct NoiseProfile {
    uint16_t noise_base = 0;
    uint16_t noise_slope = 0;
};

/**
 * @brief SNR strength settings
 */
struct SnrStrength {
    uint16_t min_motion = 0;
    uint16_t max_motion = 255;
    uint16_t min_detail = 0;
    uint16_t max_detail = 255;
};

/**
 * @brief AIISP configuration
 */
struct AiispConfig {
    bool enabled = true;
    AiispModel model = AiispModel::kDefault;
    std::string model_path = "/mnt/app/ai_models";
    
    // Model names
    std::string model_name_normal = "nvt_model_539A_frame_2_5_1_1_0";
    std::string model_name_low = "nvt_model_539A_frame_2_5_1_1_0_low";
    
    // Processing settings
    bool adaptive_mode = true;
    int denoise_strength = 50;       // 0-100
    int sharpness = 50;              // 0-100
    
    // WDR/Defog
    bool wdr_enabled = false;
    int wdr_strength = 50;
    bool defog_enabled = false;
    int defog_strength = 50;
    
    // Advanced PQ parameters
    AiispPqParam pq_params;
    
    // Performance
    bool use_low_model_for_high_fps = true;
    int high_fps_threshold = 60;
};

// ============================================================================
// AIISP Callback
// ============================================================================

/**
 * @brief Result of AIISP processing
 */
struct AiispResult {
    bool success = false;
    float processing_time_ms = 0.0f;
    float noise_level = 0.0f;
};

using AiispCallback = std::function<void(const AiispResult&)>;

// ============================================================================
// AIISP Engine
// ============================================================================

class AiispEngine {
public:
    static AiispEngine& Instance();
    
    AiispEngine(const AiispEngine&) = delete;
    AiispEngine& operator=(const AiispEngine&) = delete;
    
    // Lifecycle
    bool Init(const AiispConfig& config);
    bool Start();
    void Stop();
    void Shutdown();
    bool IsRunning() const { return running_.load(); }
    
    // Configuration
    AiispConfig GetConfig() const;
    bool SetConfig(const AiispConfig& config);
    
    // Dynamic control
    bool SetDenoiseStrength(int strength);
    bool SetWdrEnabled(bool enabled, int strength = 50);
    bool SetDefogEnabled(bool enabled, int strength = 50);
    bool SwitchModel(AiispModel model);
    
    // Load PQ parameters from JSON
    bool LoadPqParams(const std::string& json_path);
    bool SavePqParams(const std::string& json_path);
    
    // ISP callback integration
    void SetIspCallback(bool enabled);
    
    // Callback
    void SetCallback(AiispCallback callback);
    
    // Statistics
    struct Stats {
        uint64_t frames_processed = 0;
        float avg_processing_time_ms = 0.0f;
        float current_noise_level = 0.0f;
        int current_model = 0;
        uint64_t errors = 0;
    };
    Stats GetStats() const;
    void ResetStats();
    
    // HDAL integration
    void SetVideoProcPath(uint64_t proc_ctrl_path);
    
private:
    AiispEngine();
    ~AiispEngine();
    
    void ProcessingThread();
    bool InitAiNetwork();
    void CleanupAiNetwork();
    bool LoadModel(const std::string& model_name);
    
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::thread processing_thread_;
    mutable std::mutex mutex_;
    
    AiispConfig config_;
    AiispCallback callback_;
    Stats stats_;
    
    // AI network state
    bool ai_initialized_ = false;
    uint32_t proc_id_ = 0;
    
    // Memory buffers
    struct MemParm {
        UINTPTR pa = 0;
        UINTPTR va = 0;
        uint32_t size = 0;
#if HDAL_PIPELINE_ENABLED
        HD_COMMON_MEM_VB_BLK blk = HD_COMMON_MEM_VB_INVALID_BLK;
#else
        uint32_t blk = 0;
#endif
    };
    MemParm proc_mem_;
    MemParm io_mem_;
    MemParm intl_mem_;
    
    // Input parameters
    NoiseProfile noise_profile_[3];
    SnrStrength snr_strength_[3];
    
#if HDAL_PIPELINE_ENABLED
    HD_PATH_ID proc_ctrl_path_ = 0;
#endif
};

// ============================================================================
// Helper Functions
// ============================================================================

std::string AiispModelToString(AiispModel model);
bool ParseAiispPqJson(const std::string& json_path, AiispPqParam& params);

} // namespace ai
} // namespace ipcam

#endif // IPCAM_AIISP_H
