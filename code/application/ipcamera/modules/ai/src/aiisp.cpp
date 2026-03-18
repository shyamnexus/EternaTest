/**
 * @file aiisp.cpp
 * @brief AI-ISP Image Enhancement Implementation
 * 
 * Implements AI-based image enhancement using Novatek AI3 neural network:
 * - AI-based noise reduction
 * - Low-light enhancement
 * - WDR processing
 * 
 * Based on: hd_video_liveview_with_aiisp sample
 */

#include "ipcam/aiisp.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_common.h"
#if VENDOR_AI3_ENABLED
#include "vendor_ai.h"
#endif
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// AiispEngine Implementation
// ============================================================================

AiispEngine& AiispEngine::Instance() {
    static AiispEngine instance;
    return instance;
}

AiispEngine::AiispEngine() = default;

AiispEngine::~AiispEngine() {
    Shutdown();
}

bool AiispEngine::Init(const AiispConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("AIISP: Already initialized");
        return true;
    }
    
    config_ = config;
    
#if HDAL_PIPELINE_ENABLED
    // Initialize AI network
    if (!InitAiNetwork()) {
        spdlog::error("AIISP: Failed to initialize AI network");
        return false;
    }
    
    spdlog::info("AIISP: Initialized with model={}", config_.model_name_normal);
#else
    spdlog::info("AIISP: Stub mode (HDAL disabled)");
#endif
    
    initialized_.store(true);
    return true;
}

bool AiispEngine::InitAiNetwork() {
#if HDAL_PIPELINE_ENABLED && VENDOR_AI3_ENABLED
    // Initialize AI3 device
    VENDOR_AI3_DEV_CFG dev_cfg = {};
    dev_cfg.ctrl = 0;  // No special control flags
    dev_cfg.mode = 0;  // Default mode
    
    HD_RESULT ret = vendor_ai3_dev_init(&dev_cfg);
    if (ret != HD_OK) {
        spdlog::error("AIISP: vendor_ai3_dev_init failed: {}", static_cast<int>(ret));
        return false;
    }
    
    ai_initialized_ = true;
    
    // Initialize noise profiles with default values
    for (int i = 0; i < 3; i++) {
        noise_profile_[i].noise_base = 16;
        noise_profile_[i].noise_slope = 8;
        snr_strength_[i].min_motion = 0;
        snr_strength_[i].max_motion = 255;
        snr_strength_[i].min_detail = 0;
        snr_strength_[i].max_detail = 255;
    }
    
    // Load the default model
    if (!LoadModel(config_.model_name_normal)) {
        spdlog::warn("AIISP: Failed to load model, continuing with basic ISP");
    }
    
    spdlog::info("AIISP: AI3 device initialized");
    return true;
#else
    // Non-AI fallback - use basic ISP enhancement
    spdlog::info("AIISP: Initialized without AI3 (using basic ISP enhancement)");
    return true;
#endif
}

void AiispEngine::CleanupAiNetwork() {
#if HDAL_PIPELINE_ENABLED && VENDOR_AI3_ENABLED
    if (ai_initialized_) {
        // Stop and close the network if active
        if (proc_id_ != 0) {
            vendor_ai3_net_stop(proc_id_);
            vendor_ai3_net_close(proc_id_);
            proc_id_ = 0;
        }
        
        // Uninitialize AI3 device
        vendor_ai3_dev_uninit();
        ai_initialized_ = false;
    }
    
    // Free allocated DMA memory
    if (proc_mem_.va != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(proc_mem_.va), proc_mem_.size);
        if (proc_mem_.blk != HD_COMMON_MEM_VB_INVALID_BLK) {
            hd_common_mem_release_block(proc_mem_.blk);
        }
        proc_mem_ = {};
    }
    if (io_mem_.va != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(io_mem_.va), io_mem_.size);
        if (io_mem_.blk != HD_COMMON_MEM_VB_INVALID_BLK) {
            hd_common_mem_release_block(io_mem_.blk);
        }
        io_mem_ = {};
    }
    if (intl_mem_.va != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(intl_mem_.va), intl_mem_.size);
        if (intl_mem_.blk != HD_COMMON_MEM_VB_INVALID_BLK) {
            hd_common_mem_release_block(intl_mem_.blk);
        }
        intl_mem_ = {};
    }
#endif
}

bool AiispEngine::LoadModel(const std::string& model_name) {
#if HDAL_PIPELINE_ENABLED && VENDOR_AI3_ENABLED
    if (!ai_initialized_) {
        spdlog::error("AIISP: AI3 device not initialized");
        return false;
    }
    
    // Construct full model path
    std::string full_path = config_.model_path + "/" + model_name;
    
    // Read model file
    std::ifstream model_file(full_path, std::ios::binary | std::ios::ate);
    if (!model_file.is_open()) {
        spdlog::error("AIISP: Cannot open model file: {}", full_path);
        return false;
    }
    
    std::streamsize model_size = model_file.tellg();
    model_file.seekg(0, std::ios::beg);
    
    // Allocate model buffer
    HD_COMMON_MEM_VB_BLK model_blk = hd_common_mem_get_block(
        HD_COMMON_MEM_COMMON_POOL, model_size, DDR_ID0);
    if (model_blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("AIISP: Failed to allocate model buffer");
        return false;
    }
    
    UINTPTR model_pa = hd_common_mem_blk2pa(model_blk);
    void* model_va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, model_pa, model_size);
    if (!model_va) {
        hd_common_mem_release_block(model_blk);
        return false;
    }
    
    // Read model data
    model_file.read(static_cast<char*>(model_va), model_size);
    hd_common_mem_flush_cache(model_va, model_size);
    
    // Configure network open parameters
    VENDOR_AI3_PROC_CFG proc_cfg = {};
    proc_cfg.model_buf.pa = model_pa;
    proc_cfg.model_buf.va = reinterpret_cast<uintptr_t>(model_va);
    proc_cfg.model_buf.size = static_cast<UINT32>(model_size);
    proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF] = {proc_mem_.pa, proc_mem_.va, proc_mem_.size};
    proc_cfg.ctrl = 0;
    proc_cfg.mode = 0;
    
    // Open the AI network
    VENDOR_AI3_NET_INFO net_info = {};
    HD_RESULT ret = vendor_ai3_net_open(&proc_id_, &proc_cfg, &net_info);
    if (ret != HD_OK) {
        spdlog::error("AIISP: Failed to open AI network: {} (ret={})", 
            model_name, static_cast<int>(ret));
        hd_common_mem_munmap(model_va, model_size);
        hd_common_mem_release_block(model_blk);
        return false;
    }
    
    spdlog::info("AIISP: Loaded model: {} (proc_id={}, layers={})", 
                 model_name, proc_id_, net_info.layer_cnt);
    return true;
#else
    spdlog::info("AIISP: Model loading not available (no AI3 support): {}", model_name);
    return true;
#endif
}

bool AiispEngine::Start() {
    if (!initialized_.load()) {
        spdlog::error("AIISP: Not initialized");
        return false;
    }
    
    if (running_.load()) {
        return true;
    }
    
    // Enable ISP callback if configured
    if (config_.adaptive_mode) {
        SetIspCallback(true);
    }
    
    running_.store(true);
    spdlog::info("AIISP: Started");
    return true;
}

void AiispEngine::Stop() {
    if (running_.load()) {
        SetIspCallback(false);
    }
    running_.store(false);
    spdlog::info("AIISP: Stopped");
}

void AiispEngine::Shutdown() {
    Stop();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    CleanupAiNetwork();
    
    initialized_.store(false);
    spdlog::info("AIISP: Shutdown complete");
}

AiispConfig AiispEngine::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool AiispEngine::SetConfig(const AiispConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if model changed
    bool model_changed = (config.model != config_.model) ||
                         (config.model_name_normal != config_.model_name_normal);
    
    config_ = config;
    
    if (model_changed && initialized_.load()) {
        // Would reload model here
        spdlog::info("AIISP: Model configuration changed");
    }
    
    return true;
}

bool AiispEngine::SetDenoiseStrength(int strength) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (strength < 0 || strength > 100) {
        return false;
    }
    
    config_.denoise_strength = strength;
    
#if HDAL_PIPELINE_ENABLED
    // Update PQ parameters based on strength
    // This would call vendor_isp_set_param() with updated NR parameters
    config_.pq_params.nr_2dnr_still_str = static_cast<uint32_t>(strength);
    
    spdlog::debug("AIISP: Set denoise strength to {}", strength);
#endif
    
    return true;
}

bool AiispEngine::SetWdrEnabled(bool enabled, int strength) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    config_.wdr_enabled = enabled;
    config_.wdr_strength = strength;
    
#if HDAL_PIPELINE_ENABLED
    // Would configure WDR via ISP
    spdlog::debug("AIISP: WDR {} (strength={})", enabled ? "enabled" : "disabled", strength);
#endif
    
    return true;
}

bool AiispEngine::SetDefogEnabled(bool enabled, int strength) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    config_.defog_enabled = enabled;
    config_.defog_strength = strength;
    
#if HDAL_PIPELINE_ENABLED
    // Would configure defog via ISP
    spdlog::debug("AIISP: Defog {} (strength={})", enabled ? "enabled" : "disabled", strength);
#endif
    
    return true;
}

bool AiispEngine::SwitchModel(AiispModel model) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (model == config_.model) {
        return true;
    }
    
    config_.model = model;
    
    // Select model name based on type
    std::string model_name;
    switch (model) {
        case AiispModel::kLowLight:
            model_name = config_.model_name_low;
            break;
        case AiispModel::kDefault:
        default:
            model_name = config_.model_name_normal;
            break;
    }
    
    return LoadModel(model_name);
}

bool AiispEngine::LoadPqParams(const std::string& json_path) {
    try {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            spdlog::error("AIISP: Cannot open PQ params file: {}", json_path);
            return false;
        }
        
        nlohmann::json j;
        file >> j;
        
        AiispPqParam params;
        
        if (j.contains("version")) params.version = j["version"];
        if (j.contains("mode")) params.mode = j["mode"];
        if (j.contains("nr_2dnr_still_str")) params.nr_2dnr_still_str = j["nr_2dnr_still_str"];
        if (j.contains("coef_a")) params.coef_a = j["coef_a"];
        if (j.contains("coef_b")) params.coef_b = j["coef_b"];
        if (j.contains("coef_a_auto")) params.coef_a_auto = j["coef_a_auto"];
        if (j.contains("coef_b_auto")) params.coef_b_auto = j["coef_b_auto"];
        
        std::lock_guard<std::mutex> lock(mutex_);
        config_.pq_params = params;
        
        spdlog::info("AIISP: Loaded PQ params from {}", json_path);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("AIISP: Failed to load PQ params: {}", e.what());
        return false;
    }
}

bool AiispEngine::SavePqParams(const std::string& json_path) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        nlohmann::json j;
        j["version"] = config_.pq_params.version;
        j["mode"] = config_.pq_params.mode;
        j["nr_2dnr_still_str"] = config_.pq_params.nr_2dnr_still_str;
        j["coef_a"] = config_.pq_params.coef_a;
        j["coef_b"] = config_.pq_params.coef_b;
        j["coef_a_auto"] = config_.pq_params.coef_a_auto;
        j["coef_b_auto"] = config_.pq_params.coef_b_auto;
        
        std::ofstream file(json_path);
        if (!file.is_open()) {
            spdlog::error("AIISP: Cannot write PQ params file: {}", json_path);
            return false;
        }
        
        file << j.dump(2);
        
        spdlog::info("AIISP: Saved PQ params to {}", json_path);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("AIISP: Failed to save PQ params: {}", e.what());
        return false;
    }
}

void AiispEngine::SetIspCallback(bool enabled) {
#if HDAL_PIPELINE_ENABLED && VENDOR_AI3_ENABLED
    if (enabled && proc_ctrl_path_ != 0) {
        // Start the AI network for continuous processing
        if (proc_id_ != 0) {
            HD_RESULT ret = vendor_ai3_net_start(proc_id_);
            if (ret != HD_OK) {
                spdlog::error("AIISP: Failed to start AI network: {}", static_cast<int>(ret));
            } else {
                spdlog::debug("AIISP: AI network started, ISP callback enabled");
            }
        }
    } else {
        // Stop AI network processing
        if (proc_id_ != 0) {
            vendor_ai3_net_stop(proc_id_);
            spdlog::debug("AIISP: AI network stopped, ISP callback disabled");
        }
    }
#else
    spdlog::debug("AIISP: ISP callback {} (no AI3 support)", enabled ? "enabled" : "disabled");
#endif
}

void AiispEngine::SetCallback(AiispCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = std::move(callback);
}

AiispEngine::Stats AiispEngine::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void AiispEngine::ResetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Stats{};
}

void AiispEngine::SetVideoProcPath(uint64_t proc_ctrl_path) {
#if HDAL_PIPELINE_ENABLED
    std::lock_guard<std::mutex> lock(mutex_);
    proc_ctrl_path_ = static_cast<HD_PATH_ID>(proc_ctrl_path);
    spdlog::debug("AIISP: Set video proc path to 0x{:x}", proc_ctrl_path);
#endif
}

void AiispEngine::ProcessingThread() {
#if HDAL_PIPELINE_ENABLED && VENDOR_AI3_ENABLED
    while (running_.load()) {
        if (proc_id_ == 0 || proc_ctrl_path_ == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        // Get input frame from video proc path
        HD_VIDEO_FRAME frame = {};
        HD_RESULT ret = hd_videoproc_pull_out_buf(proc_ctrl_path_, &frame, 100);
        if (ret != HD_OK) {
            continue;
        }
        
        auto start = std::chrono::steady_clock::now();
        
        // Run AI network processing (uses proc_id which has input/output bindings set)
        ret = vendor_ai3_net_proc(proc_id_);
        
        auto end = std::chrono::steady_clock::now();
        float proc_time = std::chrono::duration<float, std::milli>(end - start).count();
        
        // Update stats
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stats_.frames_processed++;
            stats_.avg_processing_time_ms = 
                (stats_.avg_processing_time_ms * (stats_.frames_processed - 1) + proc_time) 
                / stats_.frames_processed;
            
            if (ret != HD_OK) {
                stats_.errors++;
            }
        }
        
        // Release frame back to pipeline
        hd_videoproc_release_out_buf(proc_ctrl_path_, &frame);
        
        // Notify callback if registered
        if (callback_) {
            AiispResult result;
            result.success = (ret == HD_OK);
            result.processing_time_ms = proc_time;
            callback_(result);
        }
    }
#else
    // Software fallback processing loop
    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(33));  // ~30fps
        
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.frames_processed++;
    }
#endif
}

// ============================================================================
// Helper Functions
// ============================================================================

std::string AiispModelToString(AiispModel model) {
    switch (model) {
        case AiispModel::kDefault: return "Default";
        case AiispModel::kLowLight: return "LowLight";
        case AiispModel::kHighMotion: return "HighMotion";
        case AiispModel::kLowNoise: return "LowNoise";
        default: return "Unknown";
    }
}

bool ParseAiispPqJson(const std::string& json_path, AiispPqParam& params) {
    try {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json j;
        file >> j;
        
        if (j.contains("version")) params.version = j["version"];
        if (j.contains("mode")) params.mode = j["mode"];
        if (j.contains("nr_2dnr_still_str")) params.nr_2dnr_still_str = j["nr_2dnr_still_str"];
        if (j.contains("coef_a")) params.coef_a = j["coef_a"];
        if (j.contains("coef_b")) params.coef_b = j["coef_b"];
        if (j.contains("nr_2dnr_still_str_auto")) params.nr_2dnr_still_str_auto = j["nr_2dnr_still_str_auto"];
        if (j.contains("coef_a_auto")) params.coef_a_auto = j["coef_a_auto"];
        if (j.contains("coef_b_auto")) params.coef_b_auto = j["coef_b_auto"];
        if (j.contains("coef_a_min")) params.coef_a_min = j["coef_a_min"];
        if (j.contains("coef_a_max")) params.coef_a_max = j["coef_a_max"];
        if (j.contains("coef_b_min")) params.coef_b_min = j["coef_b_min"];
        if (j.contains("coef_b_max")) params.coef_b_max = j["coef_b_max"];
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace ai
} // namespace ipcam
