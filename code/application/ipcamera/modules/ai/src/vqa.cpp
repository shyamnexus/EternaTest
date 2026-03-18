/**
 * @file vqa.cpp
 * @brief Video Quality Analysis Implementation
 * 
 * Implements video quality detection using Novatek VQA library.
 * Detects: brightness issues, blur, tampering
 */

#include "ipcam/vqa.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <cmath>
#include <thread>
#include <cstring>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_common.h"
#include "libvqa/vqa_lib.h"
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// VqaEngine Implementation
// ============================================================================

VqaEngine& VqaEngine::Instance() {
    static VqaEngine instance;
    return instance;
}

VqaEngine::VqaEngine() = default;

VqaEngine::~VqaEngine() {
    Shutdown();
}

bool VqaEngine::Init(const VqaConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("VQA: Already initialized");
        return true;
    }
    
    config_ = config;
    
    // Initialize VQA library
    if (!InitVqaLib()) {
        spdlog::error("VQA: Failed to initialize VQA library");
        return false;
    }
    
    initialized_.store(true);
    spdlog::info("VQA: Initialized ({}x{} @ {}fps)", 
                 config_.width, config_.height, config_.fps);
    return true;
}

bool VqaEngine::InitVqaLib() {
#if HDAL_PIPELINE_ENABLED && AI_VQA_ENABLED
    // Calculate VQA buffer size
    uint32_t buf_size = NVT_VQA_Calc_buf_size(config_.width, config_.height);
    if (buf_size == 0) {
        spdlog::error("VQA: Failed to calculate buffer size");
        return false;
    }
    
    // Allocate VQA working buffer from common memory pool
    vqa_blk_ = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, buf_size, DDR_ID0);
    if (vqa_blk_ == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("VQA: Failed to allocate VQA buffer block");
        return false;
    }
    
    // Get physical address
    vqa_buf_pa_ = hd_common_mem_blk2pa(vqa_blk_);
    if (vqa_buf_pa_ == 0) {
        spdlog::error("VQA: Failed to get physical address");
        hd_common_mem_release_block(vqa_blk_);
        vqa_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
        return false;
    }
    
    // Map to virtual address
    vqa_buf_va_ = reinterpret_cast<UINTPTR>(
        hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, vqa_buf_pa_, buf_size)
    );
    if (vqa_buf_va_ == 0) {
        spdlog::error("VQA: Failed to map buffer");
        hd_common_mem_release_block(vqa_blk_);
        vqa_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
        return false;
    }
    
    // Allocate Y buffer for input frames
    uint32_t y_buf_size = config_.width * config_.height;
    y_blk_ = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, y_buf_size, DDR_ID0);
    if (y_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        y_buf_pa_ = hd_common_mem_blk2pa(y_blk_);
        y_buf_va_ = reinterpret_cast<UINTPTR>(
            hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, y_buf_pa_, y_buf_size)
        );
    }
    
    // Initialize VQA library (returns void) - same as SDK sample
    NVT_VQA_Init(0, vqa_buf_va_);
    
    // Set VQA parameters - using vqa_param_t structure from vqa_lib.h
    vqa_param_t vqa_param = {};
    
    // Enable parameters
    vqa_param.enable_param.en_too_light = config_.too_bright.enabled ? 1 : 0;
    vqa_param.enable_param.en_too_dark = config_.too_dark.enabled ? 1 : 0;
    vqa_param.enable_param.en_blur = config_.blur.enabled ? 1 : 0;
    vqa_param.enable_param.en_blur_block_info = 1;
    vqa_param.enable_param.en_auto_adj_param = 0;
    vqa_param.enable_param.en_ref_md = 0;
    
    // Global parameters
    vqa_param.global_param.width = config_.width;
    vqa_param.global_param.height = config_.height;
    vqa_param.global_param.mb_x_size = 32;
    vqa_param.global_param.mb_y_size = 40;
    vqa_param.global_param.auto_adj_period = config_.auto_adjust_period;
    vqa_param.global_param.g_alarm_frame_num = config_.global_alarm_frames;
    
    // Overexposure (too light) parameters
    vqa_param.light_param.too_light_strength_th = config_.too_bright.strength_threshold;
    vqa_param.light_param.too_light_cover_th = config_.too_bright.coverage_threshold;
    vqa_param.light_param.too_light_alarm_times = config_.too_bright.alarm_frames;
    
    // Underexposure (too dark) parameters
    vqa_param.light_param.too_dark_strength_th = config_.too_dark.strength_threshold;
    vqa_param.light_param.too_dark_cover_th = config_.too_dark.coverage_threshold;
    vqa_param.light_param.too_dark_alarm_times = config_.too_dark.alarm_frames;
    
    // Blur parameters
    vqa_param.contrast_param.blur_strength = config_.blur.strength;
    vqa_param.contrast_param.blur_cover_th = config_.blur.coverage_threshold;
    vqa_param.contrast_param.blur_alarm_times = config_.blur.alarm_frames;
    vqa_param.contrast_param.blur_w_num = 160;  // From SDK sample
    vqa_param.contrast_param.blur_h_num = 94;   // From SDK sample
    
    NVT_VQA_Set_param(0, &vqa_param);
    
    vqa_lib_initialized_ = true;
    spdlog::info("VQA: Library initialized with buffer size {}", buf_size);
    return true;
#else
    // Software fallback - no VQA library
    spdlog::info("VQA: Initialized in software fallback mode");
    vqa_lib_initialized_ = false;
    return true;
#endif
}

void VqaEngine::CleanupVqaLib() {
#if HDAL_PIPELINE_ENABLED
    // Free Y buffer
    if (y_buf_va_ != 0) {
        uint32_t y_buf_size = config_.width * config_.height;
        hd_common_mem_munmap(reinterpret_cast<void*>(y_buf_va_), y_buf_size);
        y_buf_va_ = 0;
    }
    if (y_buf_pa_ != 0) {
        y_buf_pa_ = 0;
    }
    if (y_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(y_blk_);
        y_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    
    // Free VQA buffer - use hd_common_mem_blk_release which handles size internally
    if (vqa_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        if (vqa_buf_va_ != 0) {
            // Calculate approximate size: 3x frame size for working buffer
            uint32_t approx_size = config_.width * config_.height * 3;
            hd_common_mem_munmap(reinterpret_cast<void*>(vqa_buf_va_), approx_size);
            vqa_buf_va_ = 0;
        }
        vqa_buf_pa_ = 0;
        hd_common_mem_release_block(vqa_blk_);
        vqa_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    
    vqa_lib_initialized_ = false;
#endif
}

bool VqaEngine::Start() {
    if (!initialized_.load()) {
        spdlog::error("VQA: Not initialized");
        return false;
    }
    
    if (running_.load()) {
        return true;
    }
    
    running_.store(true);
    
    // Start processing thread
    processing_thread_ = std::thread(&VqaEngine::ProcessingThread, this);
    
    spdlog::info("VQA: Started");
    return true;
}

void VqaEngine::Stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    
    // Wait for processing thread
    if (processing_thread_.joinable()) {
        processing_thread_.join();
    }
    
    spdlog::info("VQA: Stopped");
}

void VqaEngine::Shutdown() {
    Stop();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    CleanupVqaLib();
    
    initialized_.store(false);
    spdlog::info("VQA: Shutdown complete");
}

VqaConfig VqaEngine::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool VqaEngine::SetConfig(const VqaConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    config_ = config;
    
#if HDAL_PIPELINE_ENABLED && AI_VQA_ENABLED
    if (vqa_lib_initialized_) {
        // Update VQA parameters using correct vqa_param_t structure
        vqa_param_t vqa_param = {};
        
        vqa_param.enable_param.en_too_light = config_.too_bright.enabled ? 1 : 0;
        vqa_param.enable_param.en_too_dark = config_.too_dark.enabled ? 1 : 0;
        vqa_param.enable_param.en_blur = config_.blur.enabled ? 1 : 0;
        vqa_param.enable_param.en_blur_block_info = 1;
        vqa_param.enable_param.en_auto_adj_param = 0;
        vqa_param.enable_param.en_ref_md = 0;
        
        vqa_param.global_param.width = config_.width;
        vqa_param.global_param.height = config_.height;
        vqa_param.global_param.mb_x_size = 32;
        vqa_param.global_param.mb_y_size = 40;
        vqa_param.global_param.auto_adj_period = config_.auto_adjust_period;
        vqa_param.global_param.g_alarm_frame_num = config_.global_alarm_frames;
        
        vqa_param.light_param.too_light_strength_th = config_.too_bright.strength_threshold;
        vqa_param.light_param.too_light_cover_th = config_.too_bright.coverage_threshold;
        vqa_param.light_param.too_light_alarm_times = config_.too_bright.alarm_frames;
        
        vqa_param.light_param.too_dark_strength_th = config_.too_dark.strength_threshold;
        vqa_param.light_param.too_dark_cover_th = config_.too_dark.coverage_threshold;
        vqa_param.light_param.too_dark_alarm_times = config_.too_dark.alarm_frames;
        
        vqa_param.contrast_param.blur_strength = config_.blur.strength;
        vqa_param.contrast_param.blur_cover_th = config_.blur.coverage_threshold;
        vqa_param.contrast_param.blur_alarm_times = config_.blur.alarm_frames;
        vqa_param.contrast_param.blur_w_num = 160;
        vqa_param.contrast_param.blur_h_num = 94;
        
        NVT_VQA_Set_param(0, &vqa_param);
    }
#endif
    
    return true;
}

VqaResult VqaEngine::ProcessFrame(const uint8_t* y_data, int width, int height) {
    VqaResult result;
    
    if (!initialized_.load() || y_data == nullptr) {
        return result;
    }
    
    // VQA requires exact 320x180 as per SDK specification
    // If caller provides different dimensions, skip processing to avoid buffer overflow
    if (width != config_.width || height != config_.height) {
        spdlog::debug("VQA: Skipping frame - expected {}x{}, got {}x{}", 
                      config_.width, config_.height, width, height);
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
#if HDAL_PIPELINE_ENABLED && AI_VQA_ENABLED
    if (vqa_lib_initialized_) {
        // Copy Y data to DMA buffer (same as SDK sample)
        // VQA buffer size is config_.width * config_.height (320*180 = 57600 bytes)
        if (y_buf_va_ != 0) {
            size_t buf_size = static_cast<size_t>(config_.width * config_.height);
            memcpy(reinterpret_cast<void*>(y_buf_va_), y_data, buf_size);
            hd_common_mem_flush_cache(reinterpret_cast<void*>(y_buf_va_), buf_size);
        }
        
        // Prepare result structure - using vqa_res_t from vqa_lib.h
        vqa_res_t vqa_rst = {};
        // Note: res_blur_block needs separate buffer if blur block info is enabled
        
        // Run VQA analysis - NVT_VQA_Run(channel, y_buffer, result) as per SDK sample
        INT32 ret = NVT_VQA_Run(0, reinterpret_cast<UINT8*>(y_buf_va_), &vqa_rst);
        if (ret == 0) {
            // Parse results from vqa_res_t structure
            result.too_bright = (vqa_rst.res_too_light > 0);
            result.too_dark = (vqa_rst.res_too_dark > 0);
            result.blurred = (vqa_rst.res_blur > 0);
            
            result.blur_coverage = static_cast<float>(vqa_rst.res_blur_cover);
            result.blur_level = result.blur_coverage;  // Use coverage as blur level
            
            // Brightness level from the detection flags
            if (result.too_bright) {
                result.brightness_level = 255.0f;  // Overexposed
            } else if (result.too_dark) {
                result.brightness_level = 0.0f;    // Underexposed
            } else {
                result.brightness_level = 128.0f;  // Normal
            }
        }
    } else
#endif
    {
        // SDK not available - VQA disabled
        spdlog::debug("VQA: SDK library not available, video quality analysis disabled");
        (void)y_data;
        (void)width;
        (void)height;
    }
    
    // Set timestamp
    result.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    // Update statistics
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.frames_processed++;
        if (result.too_bright) stats_.too_bright_events++;
        if (result.too_dark) stats_.too_dark_events++;
        if (result.blurred) stats_.blur_events++;
        if (result.tampered) stats_.tamper_events++;
    }
    
    // Determine issue type for callback
    VqaIssueType issue = VqaIssueType::kNone;
    if (result.too_bright) issue = VqaIssueType::kTooBright;
    else if (result.too_dark) issue = VqaIssueType::kTooDark;
    else if (result.blurred) issue = VqaIssueType::kBlurred;
    else if (result.tampered) issue = VqaIssueType::kTampered;
    
    // Track consecutive issues for alarm
    if (issue != VqaIssueType::kNone) {
        if (issue == current_issue_) {
            issue_frame_count_++;
        } else {
            current_issue_ = issue;
            issue_frame_count_ = 1;
        }
        
        // Trigger callback on alarm threshold
        if (callback_ && issue_frame_count_ >= config_.global_alarm_frames) {
            callback_(result, issue);
        }
    } else {
        current_issue_ = VqaIssueType::kNone;
        issue_frame_count_ = 0;
    }
    
    return result;
}

void VqaEngine::SetCallback(VqaCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = std::move(callback);
}

VqaEngine::Stats VqaEngine::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void VqaEngine::ResetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Stats{};
}

void VqaEngine::SetVideoProcPath(uint64_t proc_path) {
#if HDAL_PIPELINE_ENABLED
    std::lock_guard<std::mutex> lock(mutex_);
    proc_path_ = static_cast<HD_PATH_ID>(proc_path);
    spdlog::debug("VQA: Set video proc path to 0x{:x}", proc_path);
#endif
}

void VqaEngine::ProcessingThread() {
    spdlog::debug("VQA: Processing thread started");
    
    int frame_interval_ms = 1000 / config_.fps;
    
    while (running_.load()) {
#if HDAL_PIPELINE_ENABLED
        if (proc_path_ != 0) {
            // Pull frame from video proc path
            HD_VIDEO_FRAME frame = {};
            HD_RESULT ret = hd_videoproc_pull_out_buf(proc_path_, &frame, frame_interval_ms);
            
            if (ret == HD_OK) {
                // Get virtual address for Y data
                uint8_t* y_data = reinterpret_cast<uint8_t*>(frame.sign);
                
                if (y_data != nullptr) {
                    ProcessFrame(y_data, frame.dim.w, frame.dim.h);
                }
                
                // Release frame
                hd_videoproc_release_out_buf(proc_path_, &frame);
            }
        } else
#endif
        {
            // No source connected, just wait
            std::this_thread::sleep_for(std::chrono::milliseconds(frame_interval_ms));
        }
    }
    
    spdlog::debug("VQA: Processing thread stopped");
}

// ============================================================================
// Helper Functions
// ============================================================================

std::string VqaIssueTypeToString(VqaIssueType type) {
    switch (type) {
        case VqaIssueType::kNone: return "None";
        case VqaIssueType::kTooBright: return "TooBright";
        case VqaIssueType::kTooDark: return "TooDark";
        case VqaIssueType::kBlurred: return "Blurred";
        case VqaIssueType::kTampered: return "Tampered";
        case VqaIssueType::kColorCast: return "ColorCast";
        default: return "Unknown";
    }
}

} // namespace ai
} // namespace ipcam
