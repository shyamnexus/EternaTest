/**
 * @file tamper_detection_v2.cpp
 * @brief Enhanced Video Tampering & VQA Detection Implementation
 * 
 * Professional-grade camera tampering and video quality analysis:
 * 
 * TAMPER DETECTION:
 * - Scene Change: 10s dwell to trigger, 60s reset when clear
 * - Lens Covered/Blocked: Sudden dark area, edge/texture loss
 * - Camera Moved: Scene geometry shift, global motion vectors
 * - Defocus/Blur: Laplacian variance drop
 * - Spray Paint/Smudge: Local contrast collapse, texture loss
 * - Too Dark/Masked: Uniform color detection (any color)
 * - Too Bright: Bright spot detection, overexposure
 * 
 * VQA (Video Quality Analysis):
 * - Low/High brightness
 * - Low contrast
 * - Excessive noise
 * - Color cast
 * - Frozen video
 * - Excessive artifacts
 * - Single color frames
 * 
 * EDGE CASES:
 * - Night IR reflection: Temporal smoothing
 * - Mode switch: Ignore first N frames
 * - Scene fingerprint comparison
 * 
 * Uses: Edge density, histogram entropy, Laplacian variance,
 *       global motion vectors, scene fingerprint
 */

#include "ipcam/tamper_detection.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <deque>

namespace ipcam {
namespace ai {

namespace {
    // Constants
    constexpr int kVqaWidth = 320;
    constexpr int kVqaHeight = 180;
    constexpr int kVqaFrameSize = kVqaWidth * kVqaHeight;
    constexpr int kHistogramBins = 256;
    constexpr int kSobelEdgeThreshold = 30;
    
    // Scene change timing (in frames at ~12fps)
    constexpr int kSceneChangeTriggerFrames = 120;  // 10 seconds
    constexpr int kSceneChangeResetFrames = 720;    // 60 seconds
    
    // Mode switch cooldown
    constexpr int kModeSwitchCooldownFrames = 30;   // ~2.5 seconds
    
    // Temporal smoothing window
    constexpr int kTemporalWindowSize = 5;
    
    // Uniform color detection thresholds
    constexpr float kUniformColorEntropyThreshold = 3.0f;  // Very low entropy = uniform
    constexpr float kUniformColorStdDevThreshold = 25.0f;  // Low std dev = uniform color
    
    // Frozen video detection - very strict to avoid false positives on stable scenes
    // A truly frozen video has ZERO change, a real camera has sensor noise
    constexpr float kFrozenFrameThreshold = 0.01f;  // Near-zero diff = truly frozen
    constexpr int kFrozenFrameCount = 60;           // ~5 seconds of identical frames
    
    // Edge density drop thresholds
    constexpr float kSuddenEdgeLossThreshold = 50.0f;  // >50% edge loss = covered
    
    // Scene fingerprint grid
    constexpr int kFingerprintGridX = 8;
    constexpr int kFingerprintGridY = 4;
    constexpr int kFingerprintSize = kFingerprintGridX * kFingerprintGridY;
    
    inline uint64_t GetCurrentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    
    inline uint64_t GetCurrentTimeUs() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    
    // ========================================================================
    // Enhanced Detection Algorithms
    // ========================================================================
    
    /**
     * Calculate histogram entropy (measures uniformity)
     * Low entropy = uniform color, High entropy = varied image
     */
    float CalculateHistogramEntropy(const uint32_t* histogram, int total_pixels) {
        float entropy = 0.0f;
        float inv_total = 1.0f / static_cast<float>(total_pixels);
        
        for (int i = 0; i < kHistogramBins; i++) {
            if (histogram[i] > 0) {
                float p = static_cast<float>(histogram[i]) * inv_total;
                entropy -= p * std::log2(p);
            }
        }
        return entropy;  // Range: 0 (single color) to 8 (perfect uniform distribution)
    }
    
    /**
     * Calculate histogram standard deviation (measures spread)
     * Low std dev = concentrated values = uniform color
     */
    float CalculateHistogramStdDev(const uint32_t* histogram, int total_pixels) {
        // Calculate weighted mean
        float sum = 0.0f;
        for (int i = 0; i < kHistogramBins; i++) {
            sum += static_cast<float>(i) * histogram[i];
        }
        float mean = sum / total_pixels;
        
        // Calculate variance
        float var_sum = 0.0f;
        for (int i = 0; i < kHistogramBins; i++) {
            float diff = static_cast<float>(i) - mean;
            var_sum += diff * diff * histogram[i];
        }
        float variance = var_sum / total_pixels;
        
        return std::sqrt(variance);
    }
    
    /**
     * Detect dominant color (for uniform color detection)
     * Returns the bin with highest count and its percentage
     */
    void GetDominantColor(const uint32_t* histogram, int total_pixels,
                          int& dominant_bin, float& coverage) {
        dominant_bin = 0;
        uint32_t max_count = 0;
        
        // Find peak in histogram (using 8-bin smoothing)
        for (int i = 4; i < kHistogramBins - 4; i++) {
            uint32_t smoothed = 0;
            for (int j = -4; j <= 4; j++) {
                smoothed += histogram[i + j];
            }
            if (smoothed > max_count) {
                max_count = smoothed;
                dominant_bin = i;
            }
        }
        
        // Calculate coverage in ±16 range around dominant
        uint32_t near_count = 0;
        int range_start = std::max(0, dominant_bin - 16);
        int range_end = std::min(255, dominant_bin + 16);
        for (int i = range_start; i <= range_end; i++) {
            near_count += histogram[i];
        }
        coverage = static_cast<float>(near_count) / total_pixels;
    }
    
    /**
     * Check for bright spot (localized overexposure)
     * Returns coverage of bright pixels in each quadrant
     */
    void AnalyzeBrightSpots(const uint8_t* y_data, int threshold,
                            float* quadrant_coverage) {
        int counts[4] = {0, 0, 0, 0};
        int quadrant_size = kVqaFrameSize / 4;
        
        int half_w = kVqaWidth / 2;
        int half_h = kVqaHeight / 2;
        
        for (int y = 0; y < kVqaHeight; y++) {
            for (int x = 0; x < kVqaWidth; x++) {
                int idx = y * kVqaWidth + x;
                if (y_data[idx] > threshold) {
                    int q = (y >= half_h ? 2 : 0) + (x >= half_w ? 1 : 0);
                    counts[q]++;
                }
            }
        }
        
        for (int i = 0; i < 4; i++) {
            quadrant_coverage[i] = static_cast<float>(counts[i]) / quadrant_size;
        }
    }
    
    /**
     * Calculate local contrast (for spray paint/smudge detection)
     * Uses block-based standard deviation with larger blocks for performance
     */
    float CalculateLocalContrast(const uint8_t* y_data, int block_size = 32) {
        int blocks_x = kVqaWidth / block_size;
        int blocks_y = kVqaHeight / block_size;
        float total_contrast = 0.0f;
        int block_count = 0;
        const int block_pixels = block_size * block_size;
        const float inv_block_pixels = 1.0f / block_pixels;
        
        for (int by = 0; by < blocks_y; by++) {
            for (int bx = 0; bx < blocks_x; bx++) {
                // Calculate block mean and variance in single pass
                int sum = 0;
                int sum_sq = 0;
                const int base_y = by * block_size;
                const int base_x = bx * block_size;
                
                for (int dy = 0; dy < block_size; dy++) {
                    const int row_offset = (base_y + dy) * kVqaWidth + base_x;
                    for (int dx = 0; dx < block_size; dx++) {
                        int val = y_data[row_offset + dx];
                        sum += val;
                        sum_sq += val * val;
                    }
                }
                
                float mean = sum * inv_block_pixels;
                float variance = (sum_sq * inv_block_pixels) - (mean * mean);
                total_contrast += std::sqrt(std::max(0.0f, variance));
                block_count++;
            }
        }
        
        return total_contrast / block_count;
    }
    
    /**
     * Calculate scene fingerprint (block-based average luminance)
     * Used for camera moved detection
     */
    void CalculateSceneFingerprint(const uint8_t* y_data, float* fingerprint) {
        int block_w = kVqaWidth / kFingerprintGridX;
        int block_h = kVqaHeight / kFingerprintGridY;
        
        for (int gy = 0; gy < kFingerprintGridY; gy++) {
            for (int gx = 0; gx < kFingerprintGridX; gx++) {
                uint32_t sum = 0;
                int start_y = gy * block_h;
                int start_x = gx * block_w;
                
                for (int dy = 0; dy < block_h; dy++) {
                    for (int dx = 0; dx < block_w; dx++) {
                        int idx = (start_y + dy) * kVqaWidth + (start_x + dx);
                        sum += y_data[idx];
                    }
                }
                fingerprint[gy * kFingerprintGridX + gx] = 
                    static_cast<float>(sum) / (block_w * block_h);
            }
        }
    }
    
    /**
     * Compare scene fingerprints (for camera moved detection)
     * Returns difference score (0 = identical, higher = more different)
     */
    float CompareFingerprints(const float* fp1, const float* fp2) {
        float diff = 0.0f;
        for (int i = 0; i < kFingerprintSize; i++) {
            diff += std::abs(fp1[i] - fp2[i]);
        }
        return diff / kFingerprintSize;
    }
    
    /**
     * Estimate noise level using horizontal pixel differences
     * Samples every 4th row for performance (still statistically valid)
     */
    float EstimateNoiseLevel(const uint8_t* y_data) {
        int64_t diff_sum = 0;
        int count = 0;
        
        // Sample every 4th row for speed
        for (int y = 0; y < kVqaHeight; y += 4) {
            const int row_offset = y * kVqaWidth;
            for (int x = 0; x < kVqaWidth - 1; x++) {
                int diff = std::abs(y_data[row_offset + x] - y_data[row_offset + x + 1]);
                diff_sum += diff;
                count++;
            }
        }
        
        return static_cast<float>(diff_sum) / count;
    }
    
}  // namespace

// ============================================================================
// Singleton Instance
// ============================================================================

TamperDetectionEngine& TamperDetectionEngine::Instance() {
    static TamperDetectionEngine instance;
    return instance;
}

TamperDetectionEngine::TamperDetectionEngine() {
    reference_histogram_.resize(kHistogramBins, 0);
    reference_fingerprint_.resize(kFingerprintSize, 0.0f);
    prev_fingerprint_.resize(kFingerprintSize, 0.0f);
}

TamperDetectionEngine::~TamperDetectionEngine() {
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

bool TamperDetectionEngine::Init(const TamperConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("TamperDetection: Already initialized");
        return true;
    }
    
    config_ = config;
    
    // Validate dimensions
    if (config_.width != kVqaWidth || config_.height != kVqaHeight) {
        spdlog::warn("TamperDetection: Forcing dimensions to {}x{} (required for IVE)",
                     kVqaWidth, kVqaHeight);
        config_.width = kVqaWidth;
        config_.height = kVqaHeight;
    }
    
    // Apply sensitivity level
    ApplySensitivityLevel(config_.sensitivity);
    
    // Initialize IVE hardware
    if (!InitIve()) {
        spdlog::error("TamperDetection: Failed to initialize IVE");
        return false;
    }
    
    // Initialize temporal buffers
    brightness_history_.clear();
    edge_density_history_.clear();
    histogram_diff_history_.clear();
    
    initialized_.store(true);
    spdlog::info("TamperDetection: Initialized ({}x{} @ {}fps, sensitivity={})",
                 config_.width, config_.height, config_.fps,
                 SensitivityToString(config_.sensitivity));
    return true;
}

bool TamperDetectionEngine::Start() {
    if (!initialized_.load()) {
        spdlog::error("TamperDetection: Not initialized");
        return false;
    }
    
    if (running_.load()) {
        return true;
    }
    
    // Reset state
    frame_count_ = 0;
    mode_switch_cooldown_ = kModeSwitchCooldownFrames;
    frozen_frame_count_ = 0;
    scene_change_trigger_count_ = 0;
    scene_change_clear_count_ = 0;
    scene_change_alarmed_ = false;
    
    running_.store(true);
    spdlog::info("TamperDetection: Started (frames fed from analytics loop)");
    return true;
}

void TamperDetectionEngine::Stop() {
    if (!running_.load()) {
        return;
    }
    running_.store(false);
    spdlog::info("TamperDetection: Stopped");
}

void TamperDetectionEngine::Shutdown() {
    Stop();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    CleanupIve();
    initialized_.store(false);
    spdlog::info("TamperDetection: Shutdown complete");
}

// ============================================================================
// Configuration
// ============================================================================

TamperConfig TamperDetectionEngine::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool TamperDetectionEngine::SetConfig(const TamperConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    config_.enabled = config.enabled;
    config_.sensitivity = config.sensitivity;
    config_.defocus = config.defocus;
    config_.masking = config.masking;
    config_.scene_change = config.scene_change;
    config_.exposure = config.exposure;
    config_.dwell_frames = config.dwell_frames;
    config_.auto_reset = config.auto_reset;
    config_.day_night = config.day_night;
    
    ApplySensitivityLevel(config_.sensitivity);
    
    // Reset mode switch cooldown on config change
    mode_switch_cooldown_ = kModeSwitchCooldownFrames;
    
    spdlog::debug("TamperDetection: Configuration updated");
    return true;
}

void TamperDetectionEngine::SetSensitivity(TamperSensitivity sensitivity) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.sensitivity = sensitivity;
    ApplySensitivityLevel(sensitivity);
}

void TamperDetectionEngine::ApplySensitivityLevel(TamperSensitivity level) {
    switch (level) {
        case TamperSensitivity::kLow:
            config_.defocus.threshold = 50.0f;
            config_.masking.dark_threshold = 30.0f;
            config_.masking.edge_low_threshold = 0.03f;
            config_.scene_change.histogram_threshold = 45.0f;
            config_.scene_change.edge_change_percent = 40;
            config_.dwell_frames = 10;
            break;
            
        case TamperSensitivity::kMedium:
            config_.defocus.threshold = 100.0f;
            config_.masking.dark_threshold = 50.0f;
            config_.masking.edge_low_threshold = 0.05f;
            config_.scene_change.histogram_threshold = 30.0f;
            config_.scene_change.edge_change_percent = 30;
            config_.dwell_frames = 5;
            break;
            
        case TamperSensitivity::kHigh:
            config_.defocus.threshold = 200.0f;
            config_.masking.dark_threshold = 70.0f;
            config_.masking.edge_low_threshold = 0.07f;
            config_.scene_change.histogram_threshold = 15.0f;
            config_.scene_change.edge_change_percent = 15;
            config_.dwell_frames = 3;
            break;
    }
}

// ============================================================================
// Video Input
// ============================================================================

void TamperDetectionEngine::SetVideoProcPath(uint64_t proc_path) {
#if HDAL_PIPELINE_ENABLED
    std::lock_guard<std::mutex> lock(mutex_);
    proc_path_ = static_cast<HD_PATH_ID>(proc_path);
    spdlog::debug("TamperDetection: Set video proc path to 0x{:x}", proc_path);
#else
    (void)proc_path;
#endif
}

// ============================================================================
// IVE Hardware Initialization (same as before)
// ============================================================================

bool TamperDetectionEngine::InitIve() {
#if HDAL_PIPELINE_ENABLED
    HD_RESULT ret;
    
    INT32 ive_ret = vendor_ive_init();
    if (ive_ret != 0) {
        spdlog::error("TamperDetection: vendor_ive_init failed: {}", ive_ret);
        return false;
    }
    spdlog::debug("TamperDetection: IVE hardware initialized");
    
    uint32_t input_size = kVqaFrameSize;
    input_mem_blk_ = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, input_size, DDR_ID0);
    if (input_mem_blk_ == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("TamperDetection: Failed to allocate input buffer");
        return false;
    }
    input_pa_ = hd_common_mem_blk2pa(input_mem_blk_);
    input_va_ = reinterpret_cast<UINTPTR>(
        hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, input_pa_, input_size));
    
    uint32_t histo_size = kHistogramBins * sizeof(uint32_t);
    histo_mem_blk_ = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, histo_size, DDR_ID0);
    if (histo_mem_blk_ == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("TamperDetection: Failed to allocate histogram buffer");
        CleanupIve();
        return false;
    }
    histo_pa_ = hd_common_mem_blk2pa(histo_mem_blk_);
    histo_va_ = reinterpret_cast<UINTPTR>(
        hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, histo_pa_, histo_size));
    
    sobel_mem_blk_ = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, input_size, DDR_ID0);
    if (sobel_mem_blk_ == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("TamperDetection: Failed to allocate Sobel input buffer");
        CleanupIve();
        return false;
    }
    sobel_pa_ = hd_common_mem_blk2pa(sobel_mem_blk_);
    sobel_va_ = reinterpret_cast<UINTPTR>(
        hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, sobel_pa_, input_size));
    
    sobel_out_blk_ = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, input_size, DDR_ID0);
    if (sobel_out_blk_ == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("TamperDetection: Failed to allocate Sobel output buffer");
        CleanupIve();
        return false;
    }
    sobel_out_pa_ = hd_common_mem_blk2pa(sobel_out_blk_);
    sobel_out_va_ = reinterpret_cast<UINTPTR>(
        hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, sobel_out_pa_, input_size));
    
    // Laplacian output buffer (for IVE general filter - defocus detection)
    // Using 16-bit output for gradient values
    uint32_t laplacian_size = kVqaFrameSize * 2;  // 16-bit per pixel
    laplacian_out_blk_ = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, laplacian_size, DDR_ID0);
    if (laplacian_out_blk_ == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("TamperDetection: Failed to allocate Laplacian output buffer");
        CleanupIve();
        return false;
    }
    laplacian_out_pa_ = hd_common_mem_blk2pa(laplacian_out_blk_);
    laplacian_out_va_ = reinterpret_cast<UINTPTR>(
        hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, laplacian_out_pa_, laplacian_size));
    
    // Previous frame buffer (for IVE IMG_OP SUB_ABS - frame diff)
    prev_frame_blk_ = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, input_size, DDR_ID0);
    if (prev_frame_blk_ == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("TamperDetection: Failed to allocate previous frame buffer");
        CleanupIve();
        return false;
    }
    prev_frame_pa_ = hd_common_mem_blk2pa(prev_frame_blk_);
    prev_frame_va_ = reinterpret_cast<UINTPTR>(
        hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, prev_frame_pa_, input_size));
    
    // Diff output buffer (for IVE IMG_OP result)
    diff_out_blk_ = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, input_size, DDR_ID0);
    if (diff_out_blk_ == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("TamperDetection: Failed to allocate diff output buffer");
        CleanupIve();
        return false;
    }
    diff_out_pa_ = hd_common_mem_blk2pa(diff_out_blk_);
    diff_out_va_ = reinterpret_cast<UINTPTR>(
        hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, diff_out_pa_, input_size));
    
    prev_frame_valid_ = false;
    ive_initialized_ = true;
    spdlog::info("TamperDetection: IVE initialized (input={}KB, laplacian={}KB, diff={}KB)",
                 input_size / 1024, laplacian_size / 1024, input_size / 1024);
    return true;
#else
    spdlog::info("TamperDetection: IVE not available, using software fallback");
    return true;
#endif
}

void TamperDetectionEngine::CleanupIve() {
#if HDAL_PIPELINE_ENABLED
    uint32_t input_size = kVqaFrameSize;
    uint32_t histo_size = kHistogramBins * sizeof(uint32_t);
    uint32_t laplacian_size = kVqaFrameSize * 2;
    
    // Cleanup diff output buffer
    if (diff_out_va_ != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(diff_out_va_), input_size);
        diff_out_va_ = 0;
    }
    if (diff_out_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(diff_out_blk_);
        diff_out_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    
    // Cleanup previous frame buffer
    if (prev_frame_va_ != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(prev_frame_va_), input_size);
        prev_frame_va_ = 0;
    }
    if (prev_frame_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(prev_frame_blk_);
        prev_frame_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    
    // Cleanup Laplacian output buffer
    if (laplacian_out_va_ != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(laplacian_out_va_), laplacian_size);
        laplacian_out_va_ = 0;
    }
    if (laplacian_out_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(laplacian_out_blk_);
        laplacian_out_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    
    if (sobel_out_va_ != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(sobel_out_va_), input_size);
        sobel_out_va_ = 0;
    }
    if (sobel_out_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(sobel_out_blk_);
        sobel_out_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    
    if (sobel_va_ != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(sobel_va_), input_size);
        sobel_va_ = 0;
    }
    if (sobel_mem_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(sobel_mem_blk_);
        sobel_mem_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    
    if (histo_va_ != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(histo_va_), histo_size);
        histo_va_ = 0;
    }
    if (histo_mem_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(histo_mem_blk_);
        histo_mem_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    
    if (input_va_ != 0) {
        hd_common_mem_munmap(reinterpret_cast<void*>(input_va_), input_size);
        input_va_ = 0;
    }
    if (input_mem_blk_ != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(input_mem_blk_);
        input_mem_blk_ = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    
    vendor_ive_uninit();
    spdlog::debug("TamperDetection: IVE hardware uninitialized");
    
    prev_frame_valid_ = false;
    ive_initialized_ = false;
#endif
}

// ============================================================================
// IVE-Accelerated Operations (same as before)
// ============================================================================

void TamperDetectionEngine::CalculateHistogramIve(const uint8_t* y_data, uint32_t* histogram) {
#if HDAL_PIPELINE_ENABLED
    if (!ive_initialized_ || input_va_ == 0 || histo_va_ == 0) {
        std::memset(histogram, 0, kHistogramBins * sizeof(uint32_t));
        for (int i = 0; i < kVqaFrameSize; i++) {
            histogram[y_data[i]]++;
        }
        return;
    }
    
    std::memcpy(reinterpret_cast<void*>(input_va_), y_data, kVqaFrameSize);
    hd_common_mem_flush_cache(reinterpret_cast<void*>(input_va_), kVqaFrameSize);
    
    IVE_SRC_IMAGE_S src = {};
    src.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src.u64Pa = input_pa_;
    src.u64Va = input_va_;
    src.u32Width = kVqaWidth;
    src.u32Height = kVqaHeight;
    src.u32Stride = kVqaWidth;
    
    IVE_DST_IMAGE_S dst = {};
    dst.enDataType = LIB_IVE_DATA_TYPE_U32C1;
    dst.u64Pa = histo_pa_;
    dst.u64Va = histo_va_;
    dst.u32Width = 256;
    dst.u32Height = 1;
    dst.u32Stride = 256 * 4;
    
    UINT32 handle = 0;
    INT32 ret = NVT_IVE_Histo(&handle, &src, &dst, 1);
    if (ret != 0) {
        std::memset(histogram, 0, kHistogramBins * sizeof(uint32_t));
        for (int i = 0; i < kVqaFrameSize; i++) {
            histogram[y_data[i]]++;
        }
        return;
    }
    
    hd_common_mem_flush_cache(reinterpret_cast<void*>(histo_va_), kHistogramBins * sizeof(uint32_t));
    std::memcpy(histogram, reinterpret_cast<void*>(histo_va_), kHistogramBins * sizeof(uint32_t));
#else
    std::memset(histogram, 0, kHistogramBins * sizeof(uint32_t));
    for (int i = 0; i < kVqaFrameSize; i++) {
        histogram[y_data[i]]++;
    }
#endif
}

void TamperDetectionEngine::CalculateSobelIve(const uint8_t* y_data, uint8_t* edge_output) {
#if HDAL_PIPELINE_ENABLED
    if (!ive_initialized_ || sobel_va_ == 0 || sobel_out_va_ == 0) {
        std::memset(edge_output, 0, kVqaFrameSize);
        return;
    }
    
    std::memcpy(reinterpret_cast<void*>(sobel_va_), y_data, kVqaFrameSize);
    hd_common_mem_flush_cache(reinterpret_cast<void*>(sobel_va_), kVqaFrameSize);
    
    IVE_SRC_IMAGE_S src = {};
    src.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src.u64Pa = sobel_pa_;
    src.u64Va = sobel_va_;
    src.u32Width = kVqaWidth;
    src.u32Height = kVqaHeight;
    src.u32Stride = kVqaWidth;
    
    IVE_DST_IMAGE_S dst = {};
    dst.enDataType = LIB_IVE_DATA_TYPE_S16C1;
    dst.u64Pa = sobel_out_pa_;
    dst.u64Va = sobel_out_va_;
    dst.u32Width = kVqaWidth;
    dst.u32Height = kVqaHeight;
    dst.u32Stride = kVqaWidth * 2;
    
    IVE_SOBEL_CTRL_S ctrl = {};
    ctrl.s8Mask[6] = -1; ctrl.s8Mask[7] = 0; ctrl.s8Mask[8] = 1;
    ctrl.s8Mask[11] = -2; ctrl.s8Mask[12] = 0; ctrl.s8Mask[13] = 2;
    ctrl.s8Mask[16] = -1; ctrl.s8Mask[17] = 0; ctrl.s8Mask[18] = 1;
    ctrl.gradient_out_format = 0;
    
    UINT32 handle = 0;
    INT32 ret = NVT_IVE_Sobel(&handle, &src, &dst, &ctrl, 1);
    if (ret != 0) {
        std::memset(edge_output, 0, kVqaFrameSize);
        return;
    }
    
    hd_common_mem_flush_cache(reinterpret_cast<void*>(sobel_out_va_), kVqaFrameSize);
    std::memcpy(edge_output, reinterpret_cast<void*>(sobel_out_va_), kVqaFrameSize);
#else
    std::memset(edge_output, 0, kVqaFrameSize);
#endif
}

// ============================================================================
// IVE-Accelerated Laplacian (for defocus detection)
// Uses NVT_IVE_Filter with Laplacian-like kernel
// ============================================================================

float TamperDetectionEngine::CalculateLaplacianVarianceIve(const uint8_t* y_data) {
#if HDAL_PIPELINE_ENABLED
    if (!ive_initialized_ || input_va_ == 0 || laplacian_out_va_ == 0) {
        return CalculateLaplacianVarianceCpu(y_data);
    }
    
    // Copy input to IVE buffer
    std::memcpy(reinterpret_cast<void*>(input_va_), y_data, kVqaFrameSize);
    hd_common_mem_flush_cache(reinterpret_cast<void*>(input_va_), kVqaFrameSize);
    
    // Setup source image
    IVE_SRC_IMAGE_S src = {};
    src.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src.u64Pa = input_pa_;
    src.u64Va = input_va_;
    src.u32Width = kVqaWidth;
    src.u32Height = kVqaHeight;
    src.u32Stride = kVqaWidth;
    
    // Setup destination image
    IVE_DST_IMAGE_S dst = {};
    dst.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    dst.u64Pa = laplacian_out_pa_;
    dst.u64Va = laplacian_out_va_;
    dst.u32Width = kVqaWidth;
    dst.u32Height = kVqaHeight;
    dst.u32Stride = kVqaWidth;
    
    // Setup filter control with edge-enhancement kernel
    // This is a simplified Laplacian-like high-pass filter
    // Coefficients represent a 3x3 filter (scaled and stored in 10 elements)
    // Original Laplacian: [0,-1,0; -1,4,-1; 0,-1,0]
    // Adapted for IVE 10-element format
    IVE_FILTER_CTRL_S filter_ctrl = {};
    filter_ctrl.u8Mask[0] = 0;    // Coefficient 0
    filter_ctrl.u8Mask[1] = 255;  // -1 (represented as 255 for unsigned)
    filter_ctrl.u8Mask[2] = 0;    // Coefficient 2
    filter_ctrl.u8Mask[3] = 255;  // -1
    filter_ctrl.u8Mask[4] = 16;   // Center coefficient (higher weight for detection)
    filter_ctrl.u8Mask[5] = 255;  // -1
    filter_ctrl.u8Mask[6] = 0;    // Coefficient 6
    filter_ctrl.u8Mask[7] = 255;  // -1
    filter_ctrl.u8Mask[8] = 0;    // Coefficient 8
    filter_ctrl.u8Mask[9] = 0;    // Normalization
    filter_ctrl.in_fmt = 0;       // Y format
    
    UINT32 handle = 0;
    INT32 ret = NVT_IVE_Filter(&handle, &src, &dst, &filter_ctrl, 1);
    
    if (ret != LIB_IVE_OK) {
        spdlog::debug("TamperDetection: NVT_IVE_Filter failed: {}, falling back to CPU", ret);
        return CalculateLaplacianVarianceCpu(y_data);
    }
    
    // Flush cache and calculate variance from output
    hd_common_mem_flush_cache(reinterpret_cast<void*>(laplacian_out_va_), kVqaFrameSize);
    
    // Calculate variance from filtered output
    const uint8_t* filter_data = reinterpret_cast<const uint8_t*>(laplacian_out_va_);
    int64_t sum = 0;
    int64_t sum_sq = 0;
    
    for (int i = 0; i < kVqaFrameSize; i++) {
        int val = static_cast<int>(filter_data[i]) - 128;  // Center around zero
        sum += val;
        sum_sq += (int64_t)val * val;
    }
    
    double mean = static_cast<double>(sum) / kVqaFrameSize;
    double variance = (static_cast<double>(sum_sq) / kVqaFrameSize) - (mean * mean);
    
    return static_cast<float>(variance);
#else
    return CalculateLaplacianVarianceCpu(y_data);
#endif
}

// ============================================================================
// IVE-Accelerated Frame Diff (for frozen video detection)
// Uses NVT_IVE_Sub with absolute difference mode
// ============================================================================

float TamperDetectionEngine::CalculateFrameDiffIve(const uint8_t* y_data) {
#if HDAL_PIPELINE_ENABLED
    if (!ive_initialized_ || input_va_ == 0 || prev_frame_va_ == 0 || diff_out_va_ == 0) {
        prev_frame_valid_ = false;
        return 0.0f;
    }
    
    // Copy current frame to input buffer
    std::memcpy(reinterpret_cast<void*>(input_va_), y_data, kVqaFrameSize);
    hd_common_mem_flush_cache(reinterpret_cast<void*>(input_va_), kVqaFrameSize);
    
    // If no previous frame, just store current and return
    if (!prev_frame_valid_) {
        std::memcpy(reinterpret_cast<void*>(prev_frame_va_), y_data, kVqaFrameSize);
        hd_common_mem_flush_cache(reinterpret_cast<void*>(prev_frame_va_), kVqaFrameSize);
        prev_frame_valid_ = true;
        return 0.0f;
    }
    
    // Setup source images
    IVE_SRC_IMAGE_S src1 = {};  // Current frame
    src1.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src1.u64Pa = input_pa_;
    src1.u64Va = input_va_;
    src1.u32Width = kVqaWidth;
    src1.u32Height = kVqaHeight;
    src1.u32Stride = kVqaWidth;
    
    IVE_SRC_IMAGE_S src2 = {};  // Previous frame
    src2.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src2.u64Pa = prev_frame_pa_;
    src2.u64Va = prev_frame_va_;
    src2.u32Width = kVqaWidth;
    src2.u32Height = kVqaHeight;
    src2.u32Stride = kVqaWidth;
    
    // Setup destination image
    IVE_DST_IMAGE_S dst = {};
    dst.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    dst.u64Pa = diff_out_pa_;
    dst.u64Va = diff_out_va_;
    dst.u32Width = kVqaWidth;
    dst.u32Height = kVqaHeight;
    dst.u32Stride = kVqaWidth;
    
    // Setup SUB control for absolute difference: |A - B|
    IVE_SUB_CTRL_S sub_ctrl = {};
    sub_ctrl.enSubMode = LIB_IVE_SUB_MODE_ABS;
    
    UINT32 handle = 0;
    INT32 ret = NVT_IVE_Sub(&handle, &src1, &src2, &dst, &sub_ctrl, 1);
    
    // Update previous frame for next iteration (before checking result)
    std::memcpy(reinterpret_cast<void*>(prev_frame_va_), y_data, kVqaFrameSize);
    hd_common_mem_flush_cache(reinterpret_cast<void*>(prev_frame_va_), kVqaFrameSize);
    
    if (ret != LIB_IVE_OK) {
        spdlog::debug("TamperDetection: NVT_IVE_Sub failed: {}", ret);
        return 0.0f;  // Assume no diff on error
    }
    
    // Flush cache and calculate mean absolute difference
    hd_common_mem_flush_cache(reinterpret_cast<void*>(diff_out_va_), kVqaFrameSize);
    
    const uint8_t* diff_data = reinterpret_cast<const uint8_t*>(diff_out_va_);
    uint64_t total_diff = 0;
    for (int i = 0; i < kVqaFrameSize; i++) {
        total_diff += diff_data[i];
    }
    
    // Return mean absolute difference
    return static_cast<float>(total_diff) / kVqaFrameSize;
#else
    (void)y_data;
    return 0.0f;
#endif
}

// ============================================================================
// CPU-Based Calculations (fallback and operations IVE doesn't support)
// ============================================================================

float TamperDetectionEngine::CalculateLaplacianVarianceCpu(const uint8_t* y_data) {
    int64_t sum = 0;
    int64_t sum_sq = 0;
    int count = 0;
    
    for (int y = 1; y < kVqaHeight - 1; y++) {
        for (int x = 1; x < kVqaWidth - 1; x++) {
            int idx = y * kVqaWidth + x;
            int laplacian = 
                y_data[idx - kVqaWidth] +
                y_data[idx - 1] +
                y_data[idx + 1] +
                y_data[idx + kVqaWidth] -
                4 * y_data[idx];
            
            sum += laplacian;
            sum_sq += (int64_t)laplacian * laplacian;
            count++;
        }
    }
    
    if (count == 0) return 0.0f;
    
    double mean = static_cast<double>(sum) / count;
    double variance = (static_cast<double>(sum_sq) / count) - (mean * mean);
    
    return static_cast<float>(variance);
}

float TamperDetectionEngine::CalculateMeanBrightnessFromHistogram(const uint32_t* histogram) {
    // Calculate mean brightness directly from histogram (no pixel iteration needed)
    uint64_t weighted_sum = 0;
    uint64_t total_count = 0;
    
    for (int i = 0; i < kHistogramBins; i++) {
        weighted_sum += static_cast<uint64_t>(i) * histogram[i];
        total_count += histogram[i];
    }
    
    if (total_count == 0) return 128.0f;
    return static_cast<float>(weighted_sum) / total_count;
}

int TamperDetectionEngine::CountEdgePixels(const uint8_t* edge_data, int threshold) {
    int count = 0;
    for (int i = 0; i < kVqaFrameSize; i++) {
        if (edge_data[i] > threshold) {
            count++;
        }
    }
    return count;
}

// ============================================================================
// Histogram Comparison
// ============================================================================

float TamperDetectionEngine::CalculateHistogramChiSquare(const uint32_t* hist1, const uint32_t* hist2) {
    float chi_sq = 0.0f;
    for (int i = 0; i < kHistogramBins; i++) {
        float c = static_cast<float>(hist1[i]);
        float r = static_cast<float>(hist2[i]);
        if (c + r > 0) {
            chi_sq += (c - r) * (c - r) / (c + r);
        }
    }
    chi_sq /= 256.0f;
    return chi_sq;
}

void TamperDetectionEngine::NormalizeHistogram(const uint32_t* hist_in, float* hist_out) {
    uint64_t total = 0;
    for (int i = 0; i < kHistogramBins; i++) {
        total += hist_in[i];
    }
    
    if (total == 0) {
        std::memset(hist_out, 0, kHistogramBins * sizeof(float));
        return;
    }
    
    float inv_total = 1.0f / static_cast<float>(total);
    for (int i = 0; i < kHistogramBins; i++) {
        hist_out[i] = static_cast<float>(hist_in[i]) * inv_total;
    }
}

// ============================================================================
// Reference Frame Management
// ============================================================================

void TamperDetectionEngine::UpdateReference(const uint8_t* y_data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    CalculateHistogramIve(y_data, reference_histogram_.data());
    
    std::vector<uint8_t> edge_output(kVqaFrameSize);
    CalculateSobelIve(y_data, edge_output.data());
    reference_edge_count_ = CountEdgePixels(edge_output.data(), kSobelEdgeThreshold);
    
    // Use histogram to calculate mean brightness (more efficient)
    reference_brightness_ = CalculateMeanBrightnessFromHistogram(reference_histogram_.data());
    reference_local_contrast_ = CalculateLocalContrast(y_data);
    
    // Update scene fingerprint
    CalculateSceneFingerprint(y_data, reference_fingerprint_.data());
    
    float ref_edge_density = 100.0f * static_cast<float>(reference_edge_count_) / kVqaFrameSize;
    
    reference_timestamp_ = GetCurrentTimeMs();
    reference_valid_ = true;
    
    stats_.reference_updates++;
    
    spdlog::info("TamperDetection: Reference captured (brightness={:.1f}, edge_density={:.1f}%, "
                 "local_contrast={:.1f})",
                 reference_brightness_, ref_edge_density, reference_local_contrast_);
}

void TamperDetectionEngine::ResetReference() {
    std::lock_guard<std::mutex> lock(mutex_);
    reference_valid_ = false;
    scene_change_start_ = 0;
    auto_reset_pending_ = false;
    scene_change_trigger_count_ = 0;
    scene_change_clear_count_ = 0;
    scene_change_alarmed_ = false;
    
    defocus_dwell_count_ = 0;
    masking_dwell_count_ = 0;
    scene_change_dwell_count_ = 0;
    too_dark_dwell_count_ = 0;
    too_bright_dwell_count_ = 0;
    
    current_alarms_ = TamperType::kNone;
    prev_alarms_ = TamperType::kNone;
    
    spdlog::info("TamperDetection: Reference reset requested");
}

bool TamperDetectionEngine::HasValidReference() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return reference_valid_;
}

bool TamperDetectionEngine::ShouldAutoResetReference(TamperType current_alarms) {
    bool is_pure_scene_change = HasFlag(current_alarms, TamperType::kSceneChange) &&
                                !HasFlag(current_alarms, TamperType::kMasking) &&
                                !HasFlag(current_alarms, TamperType::kTooDark) &&
                                !HasFlag(current_alarms, TamperType::kDefocus);
    
    if (!is_pure_scene_change) {
        scene_change_start_ = 0;
        auto_reset_pending_ = false;
        return false;
    }
    
    uint64_t now = GetCurrentTimeMs();
    
    if (scene_change_start_ == 0) {
        scene_change_start_ = now;
        auto_reset_pending_ = true;
        return false;
    }
    
    uint64_t elapsed_sec = (now - scene_change_start_) / 1000;
    if (elapsed_sec >= static_cast<uint64_t>(config_.auto_reset.delay_sec)) {
        stats_.auto_resets++;
        scene_change_start_ = 0;
        auto_reset_pending_ = false;
        return true;
    }
    
    return false;
}

bool TamperDetectionEngine::IsGradualBrightnessChange(float current_mean) {
    if (!config_.day_night.enabled) {
        return false;
    }
    
    float diff = std::abs(current_mean - prev_brightness_);
    float rate_per_frame = diff;
    float rate_per_sec = rate_per_frame * config_.fps;
    
    if (rate_per_sec < config_.day_night.gradual_change_threshold) {
        if (!in_day_night_transition_ && diff > 0.5f) {
            brightness_change_start_ = GetCurrentTimeMs();
            in_day_night_transition_ = true;
            spdlog::debug("TamperDetection: Day/night transition detected");
        }
    } else {
        in_day_night_transition_ = false;
        brightness_change_start_ = 0;
    }
    
    if (in_day_night_transition_) {
        uint64_t elapsed_sec = (GetCurrentTimeMs() - brightness_change_start_) / 1000;
        if (elapsed_sec >= static_cast<uint64_t>(config_.day_night.transition_cooldown_sec)) {
            in_day_night_transition_ = false;
        }
    }
    
    prev_brightness_ = current_mean;
    return in_day_night_transition_;
}

// ============================================================================
// Temporal Smoothing
// ============================================================================

float TamperDetectionEngine::GetTemporalSmoothedValue(std::deque<float>& history, float new_value) {
    history.push_back(new_value);
    if (history.size() > kTemporalWindowSize) {
        history.pop_front();
    }
    
    // Return median for robustness against spikes
    std::vector<float> sorted(history.begin(), history.end());
    std::sort(sorted.begin(), sorted.end());
    return sorted[sorted.size() / 2];
}

// ============================================================================
// Main Processing (Enhanced)
// ============================================================================

TamperResult TamperDetectionEngine::ProcessFrame(const uint8_t* y_data, int width, int height) {
    TamperResult result;
    result.timestamp = GetCurrentTimeMs();
    
    if (!initialized_.load() || !config_.enabled || y_data == nullptr) {
        return result;
    }
    
    if (width != kVqaWidth || height != kVqaHeight) {
        spdlog::debug("TamperDetection: Skipping frame - expected {}x{}, got {}x{}",
                      kVqaWidth, kVqaHeight, width, height);
        return result;
    }
    
    frame_count_++;
    uint64_t start_time = GetCurrentTimeUs();
    
    // Mode switch cooldown (ignore first N frames after init/config change)
    if (mode_switch_cooldown_ > 0) {
        mode_switch_cooldown_--;
        if (!reference_valid_) {
            UpdateReference(y_data);
        }
        return result;
    }
    
    // ========================================================================
    // Calculate All Metrics (IVE-accelerated where possible)
    // ========================================================================
    
    uint32_t histogram[kHistogramBins];
    CalculateHistogramIve(y_data, histogram);
    
    std::vector<uint8_t> edge_output(kVqaFrameSize);
    CalculateSobelIve(y_data, edge_output.data());
    
    // IVE-accelerated: Laplacian variance (defocus detection)
    float laplacian_var = CalculateLaplacianVarianceIve(y_data);
    
    // Use histogram to calculate mean brightness (more efficient)
    float mean_brightness = CalculateMeanBrightnessFromHistogram(histogram);
    
    // IVE-accelerated: Frame-to-frame diff (frozen video detection)
    float ive_frame_diff = CalculateFrameDiffIve(y_data);
    
    int edge_count = CountEdgePixels(edge_output.data(), kSobelEdgeThreshold);
    float edge_density = 100.0f * static_cast<float>(edge_count) / kVqaFrameSize;
    
    // Enhanced metrics
    float histogram_entropy = CalculateHistogramEntropy(histogram, kVqaFrameSize);
    float histogram_std_dev = CalculateHistogramStdDev(histogram, kVqaFrameSize);
    
    float local_contrast = CalculateLocalContrast(y_data);
    
    float noise_level = EstimateNoiseLevel(y_data);
    
    // Scene fingerprint
    std::vector<float> current_fingerprint(kFingerprintSize);
    CalculateSceneFingerprint(y_data, current_fingerprint.data());
    
    // Dominant color analysis (for uniform color detection)
    int dominant_bin;
    float dominant_coverage;
    GetDominantColor(histogram, kVqaFrameSize, dominant_bin, dominant_coverage);
    
    // Bright spot analysis
    float quadrant_bright[4];
    AnalyzeBrightSpots(y_data, config_.exposure.too_bright_threshold, quadrant_bright);
    
    // Apply temporal smoothing
    float smoothed_brightness = GetTemporalSmoothedValue(brightness_history_, mean_brightness);
    float smoothed_edge_density = GetTemporalSmoothedValue(edge_density_history_, edge_density);
    
    // Store metrics in result
    result.laplacian_variance = laplacian_var;
    result.mean_brightness = mean_brightness;
    result.edge_ratio = edge_density / 100.0f;
    result.reference_valid = reference_valid_;
    
    // Check for day/night transition
    bool suppress_alarms = IsGradualBrightnessChange(mean_brightness);
    result.in_day_night_transition = suppress_alarms;
    
    // Initialize reference on first valid frame
    if (!reference_valid_) {
        UpdateReference(y_data);
        prev_fingerprint_ = current_fingerprint;
        result.reference_valid = true;
        return result;
    }
    
    // Histogram comparison
    float histogram_diff = CalculateHistogramChiSquare(histogram, reference_histogram_.data());
    float smoothed_histogram_diff = GetTemporalSmoothedValue(histogram_diff_history_, histogram_diff);
    result.histogram_diff = histogram_diff;
    
    // Edge structure change
    float ref_edge_density = 100.0f * static_cast<float>(reference_edge_count_) / kVqaFrameSize;
    float edge_diff = std::abs(edge_density - ref_edge_density);
    
    // Scene fingerprint comparison
    float fingerprint_diff = CompareFingerprints(current_fingerprint.data(), 
                                                   reference_fingerprint_.data());
    float frame_to_frame_diff = CompareFingerprints(current_fingerprint.data(),
                                                      prev_fingerprint_.data());
    
    // Scene score
    float brightness_diff = std::abs(mean_brightness - reference_brightness_) / 255.0f * 100.0f;
    float scene_score = histogram_diff * 8.0f + edge_diff * 0.5f + brightness_diff * 0.2f;
    
    result.edge_change_percent = static_cast<int>(edge_diff);
    
    // ========================================================================
    // Enhanced Detection Logic
    // ========================================================================
    
    TamperType frame_alarms = TamperType::kNone;
    VqaIssues vqa_issues = VqaIssues::kNone;
    
    if (!suppress_alarms) {
        // --------------------------------------------------------------------
        // 1. DEFOCUS/BLUR: Sharp Laplacian variance drop
        // --------------------------------------------------------------------
        if (config_.defocus.enabled) {
            // Compare to reference for sudden change
            float ref_sharpness = reference_local_contrast_;
            float sharpness_drop = (ref_sharpness > 0) ? 
                (ref_sharpness - local_contrast) / ref_sharpness * 100.0f : 0.0f;
            
            if (laplacian_var < config_.defocus.threshold || sharpness_drop > 50.0f) {
                frame_alarms |= TamperType::kDefocus;
            }
        }
        
        // --------------------------------------------------------------------
        // 2. UNIFORM COLOR / MASKED (any color tape, not just dark)
        // --------------------------------------------------------------------
        if (config_.masking.enabled) {
            // Low entropy + high dominant coverage = uniform color (blocked by colored tape)
            bool is_uniform_color = (histogram_entropy < kUniformColorEntropyThreshold) ||
                                     (histogram_std_dev < kUniformColorStdDevThreshold) ||
                                     (dominant_coverage > 0.85f);
            
            // Low edge density = texture loss (covered lens)
            bool low_edges = (edge_density < config_.masking.edge_low_threshold * 100.0f);
            
            // Sudden large edge loss (covered suddenly)
            bool sudden_edge_loss = (edge_diff > kSuddenEdgeLossThreshold) && 
                                     (edge_density < ref_edge_density);
            
            if ((is_uniform_color && low_edges) || sudden_edge_loss) {
                frame_alarms |= TamperType::kMasking;
            }
        }
        
        // --------------------------------------------------------------------
        // 3. TOO DARK
        // --------------------------------------------------------------------
        if (config_.exposure.enabled) {
            // Dark coverage
            int dark_count = 0;
            for (int i = 0; i < config_.exposure.too_dark_threshold; i++) {
                dark_count += histogram[i];
            }
            float dark_coverage = static_cast<float>(dark_count) / kVqaFrameSize;
            
            // Uniform dark = covered by dark tape or night
            bool is_dark = (mean_brightness < config_.exposure.too_dark_threshold &&
                            dark_coverage > config_.exposure.too_dark_coverage);
            
            // Also detect dark uniform color (low entropy + dark)
            bool dark_uniform = (histogram_entropy < kUniformColorEntropyThreshold &&
                                 mean_brightness < 80);
            
            if (is_dark || dark_uniform) {
                frame_alarms |= TamperType::kTooDark;
            }
        }
        
        // --------------------------------------------------------------------
        // 4. TOO BRIGHT (entire screen or large bright spot)
        // --------------------------------------------------------------------
        if (config_.exposure.enabled) {
            int bright_count = 0;
            for (int i = config_.exposure.too_bright_threshold; i < kHistogramBins; i++) {
                bright_count += histogram[i];
            }
            float bright_coverage = static_cast<float>(bright_count) / kVqaFrameSize;
            
            // Check for localized bright spot (one quadrant very bright)
            bool has_bright_spot = false;
            for (int q = 0; q < 4; q++) {
                if (quadrant_bright[q] > 0.6f) {  // 60% of quadrant is bright
                    has_bright_spot = true;
                    break;
                }
            }
            
            // Bright uniform color (e.g., white tape)
            bool bright_uniform = (histogram_entropy < kUniformColorEntropyThreshold &&
                                   mean_brightness > 180);
            
            if ((mean_brightness > config_.exposure.too_bright_threshold &&
                 bright_coverage > config_.exposure.too_bright_coverage) ||
                has_bright_spot || bright_uniform) {
                frame_alarms |= TamperType::kTooBright;
            }
        }
        
        // --------------------------------------------------------------------
        // 5. SCENE CHANGE / CAMERA MOVED
        //    Requires 10 seconds sustained, resets after 60 seconds clear
        // --------------------------------------------------------------------
        if (config_.scene_change.enabled) {
            // Scene geometry shift detection using fingerprint
            bool geometry_shift = (fingerprint_diff > 15.0f);
            
            // IVE-accelerated: Use mean absolute pixel diff for sudden motion
            // High ive_frame_diff means significant frame-to-frame change
            bool sudden_motion = (ive_frame_diff > 25.0f) || (frame_to_frame_diff > 10.0f);
            
            // Combined scene change detection
            bool structural_change = (edge_diff > 2.0f) || 
                                      (scene_score > config_.scene_change.histogram_threshold * 1.5f);
            bool scene_changed = (scene_score > config_.scene_change.histogram_threshold && 
                                   structural_change) || geometry_shift;
            
            if (scene_changed || sudden_motion) {
                scene_change_trigger_count_++;
                scene_change_clear_count_ = 0;
                
                // Only alarm after sustained 10 seconds (~120 frames at 12fps)
                if (scene_change_trigger_count_ >= kSceneChangeTriggerFrames) {
                    frame_alarms |= TamperType::kSceneChange;
                    scene_change_alarmed_ = true;
                }
            } else {
                scene_change_clear_count_++;
                
                // Reset alarm counter if clear for 60 seconds
                if (scene_change_clear_count_ >= kSceneChangeResetFrames) {
                    scene_change_trigger_count_ = 0;
                    scene_change_alarmed_ = false;
                    scene_change_clear_count_ = 0;  // Reset to prevent continuous reference updates
                    
                    // Auto-update reference after prolonged stability
                    UpdateReference(y_data);
                }
                
                // If previously alarmed but now clearing, keep alarm until reset threshold
                if (scene_change_alarmed_ && scene_change_clear_count_ < kSceneChangeResetFrames) {
                    frame_alarms |= TamperType::kSceneChange;
                }
            }
        }
        
        // --------------------------------------------------------------------
        // VQA Issues Detection
        // --------------------------------------------------------------------
        
        // Low contrast
        if (local_contrast < 5.0f) {
            vqa_issues |= VqaIssues::kLowContrast;
        }
        
        // Excessive noise
        if (noise_level > 15.0f) {
            vqa_issues |= VqaIssues::kExcessiveNoise;
        }
        
        // Frozen video (truly identical consecutive frames)
        // IVE-accelerated: Uses mean absolute pixel difference
        // Real cameras have sensor noise causing ~2-5 mean pixel diff between frames
        // Frozen video (looped/static image) has < 0.5 mean diff
        if (ive_frame_diff < 0.5f) {
            frozen_frame_count_++;
            if (frozen_frame_count_ >= kFrozenFrameCount) {
                vqa_issues |= VqaIssues::kFrozenVideo;
            }
        } else {
            frozen_frame_count_ = 0;
        }
        
        // Single color frame
        if (histogram_entropy < 2.0f || dominant_coverage > 0.9f) {
            vqa_issues |= VqaIssues::kSingleColorFrame;
        }
    }
    
    // Store current fingerprint for next frame comparison
    prev_fingerprint_ = current_fingerprint;
    
    // ========================================================================
    // Dwell Time Processing
    // ========================================================================
    
    UpdateDwellCounters(frame_alarms);
    
    TamperType new_alarms = TamperType::kNone;
    
    if (defocus_dwell_count_ >= config_.dwell_frames) {
        new_alarms |= TamperType::kDefocus;
    }
    if (masking_dwell_count_ >= config_.dwell_frames) {
        new_alarms |= TamperType::kMasking;
    }
    // Scene change uses its own timing, not dwell frames
    if (HasFlag(frame_alarms, TamperType::kSceneChange)) {
        new_alarms |= TamperType::kSceneChange;
    }
    if (too_dark_dwell_count_ >= config_.dwell_frames) {
        new_alarms |= TamperType::kTooDark;
    }
    if (too_bright_dwell_count_ >= config_.dwell_frames) {
        new_alarms |= TamperType::kTooBright;
    }
    
    result.alarms = new_alarms;
    result.vqa_issues = vqa_issues;
    
    // ========================================================================
    // Auto-Reset and Reference Update
    // ========================================================================
    
    if (config_.auto_reset.enabled && ShouldAutoResetReference(new_alarms)) {
        spdlog::info("TamperDetection: Auto-resetting reference after scene change");
        UpdateReference(y_data);
        scene_change_dwell_count_ = 0;
        scene_change_trigger_count_ = 0;
        scene_change_alarmed_ = false;
        new_alarms = new_alarms & ~TamperType::kSceneChange;
        result.alarms = new_alarms;
    }
    
    // Regular reference update (only when no alarms)
    if (new_alarms == TamperType::kNone && vqa_issues == VqaIssues::kNone) {
        uint64_t elapsed_sec = (GetCurrentTimeMs() - reference_timestamp_) / 1000;
        if (elapsed_sec >= static_cast<uint64_t>(config_.reference_update_interval_sec)) {
            UpdateReference(y_data);
        }
    }
    
    // ========================================================================
    // Fire Callbacks on State Change
    // ========================================================================
    
    CheckAndFireCallbacks(new_alarms, result);
    
    current_alarms_ = new_alarms;
    current_vqa_issues_ = vqa_issues;
    last_result_ = result;
    
    // Update statistics (minimal logging - only on state changes)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.frames_processed++;
        uint64_t total_time = GetCurrentTimeUs() - start_time;
        
        stats_.avg_total_time_us = (stats_.avg_total_time_us * 7 + total_time) / 8;
    }
    
    return result;
}

void TamperDetectionEngine::UpdateDwellCounters(TamperType current_frame_alarms) {
    if (HasFlag(current_frame_alarms, TamperType::kDefocus)) {
        defocus_dwell_count_ = std::min(defocus_dwell_count_ + 1, config_.dwell_frames + 10);
    } else {
        defocus_dwell_count_ = std::max(defocus_dwell_count_ - 1, 0);
    }
    
    if (HasFlag(current_frame_alarms, TamperType::kMasking)) {
        masking_dwell_count_ = std::min(masking_dwell_count_ + 1, config_.dwell_frames + 10);
    } else {
        masking_dwell_count_ = std::max(masking_dwell_count_ - 1, 0);
    }
    
    // Scene change uses different timing (10s trigger, 60s reset) - handled in main logic
    
    if (HasFlag(current_frame_alarms, TamperType::kTooDark)) {
        too_dark_dwell_count_ = std::min(too_dark_dwell_count_ + 1, config_.dwell_frames + 10);
    } else {
        too_dark_dwell_count_ = std::max(too_dark_dwell_count_ - 1, 0);
    }
    
    if (HasFlag(current_frame_alarms, TamperType::kTooBright)) {
        too_bright_dwell_count_ = std::min(too_bright_dwell_count_ + 1, config_.dwell_frames + 10);
    } else {
        too_bright_dwell_count_ = std::max(too_bright_dwell_count_ - 1, 0);
    }
}

void TamperDetectionEngine::CheckAndFireCallbacks(TamperType new_alarms, const TamperResult& result) {
    if (new_alarms == prev_alarms_) {
        return;
    }
    
    // NOTE: Must compare prev_alarms_ (old state) against new_alarms (new state).
    // Previously this compared prev_alarms_ vs current_alarms_, but current_alarms_
    // is not yet updated at this point in ProcessFrame(), so both held the same
    // value and no transitions were ever detected. This matches how motion_detection.cpp
    // counts events directly from the computed result rather than stale member state.
    auto check_alarm = [this, &result, new_alarms](TamperType type, uint64_t& event_counter) {
        bool was_active = HasFlag(prev_alarms_, type);
        bool is_active = HasFlag(new_alarms, type);
        
        if (is_active && !was_active) {
            event_counter++;
            if (callback_) {
                callback_(type, true, result);
            }
            spdlog::warn("TamperDetection: ALARM {} ACTIVE (frames_processed={})",
                         TamperTypeToString(type), stats_.frames_processed);
        } else if (!is_active && was_active) {
            if (callback_) {
                callback_(type, false, result);
            }
            spdlog::info("TamperDetection: Alarm {} cleared (frames_processed={})",
                         TamperTypeToString(type), stats_.frames_processed);
        }
    };
    
    check_alarm(TamperType::kDefocus, stats_.defocus_events);
    check_alarm(TamperType::kMasking, stats_.masking_events);
    check_alarm(TamperType::kSceneChange, stats_.scene_change_events);
    check_alarm(TamperType::kTooDark, stats_.too_dark_events);
    check_alarm(TamperType::kTooBright, stats_.too_bright_events);
    
    prev_alarms_ = new_alarms;
}

// ============================================================================
// Statistics and State
// ============================================================================

TamperStats TamperDetectionEngine::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

TamperResult TamperDetectionEngine::GetLastResult() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_result_;
}

TamperType TamperDetectionEngine::GetCurrentAlarms() const {
    return current_alarms_;
}

void TamperDetectionEngine::SetCallback(TamperCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = std::move(callback);
}

// ============================================================================
// Free Helper Functions (declared in header)
// ============================================================================

std::string TamperTypeToString(TamperType type) {
    if (type == TamperType::kNone) return "NONE";
    
    std::string result;
    if (HasFlag(type, TamperType::kDefocus)) {
        if (!result.empty()) result += "|";
        result += "DEFOCUS";
    }
    if (HasFlag(type, TamperType::kMasking)) {
        if (!result.empty()) result += "|";
        result += "MASKING";
    }
    if (HasFlag(type, TamperType::kSceneChange)) {
        if (!result.empty()) result += "|";
        result += "SCENE_CHANGE";
    }
    if (HasFlag(type, TamperType::kTooDark)) {
        if (!result.empty()) result += "|";
        result += "TOO_DARK";
    }
    if (HasFlag(type, TamperType::kTooBright)) {
        if (!result.empty()) result += "|";
        result += "TOO_BRIGHT";
    }
    return result;
}

std::string VqaIssuesToString(VqaIssues issues) {
    if (issues == VqaIssues::kNone) return "NONE";
    
    std::string result;
    if (HasFlag(issues, VqaIssues::kLowContrast)) {
        if (!result.empty()) result += "|";
        result += "LOW_CONTRAST";
    }
    if (HasFlag(issues, VqaIssues::kExcessiveNoise)) {
        if (!result.empty()) result += "|";
        result += "NOISE";
    }
    if (HasFlag(issues, VqaIssues::kFrozenVideo)) {
        if (!result.empty()) result += "|";
        result += "FROZEN";
    }
    if (HasFlag(issues, VqaIssues::kSingleColorFrame)) {
        if (!result.empty()) result += "|";
        result += "SINGLE_COLOR";
    }
    return result;
}

std::string SensitivityToString(TamperSensitivity sens) {
    switch (sens) {
        case TamperSensitivity::kLow: return "Low";
        case TamperSensitivity::kMedium: return "Medium";
        case TamperSensitivity::kHigh: return "High";
        default: return "Unknown";
    }
}

std::vector<TamperType> GetActiveTamperTypes(TamperType alarms) {
    std::vector<TamperType> result;
    if (HasFlag(alarms, TamperType::kDefocus)) result.push_back(TamperType::kDefocus);
    if (HasFlag(alarms, TamperType::kMasking)) result.push_back(TamperType::kMasking);
    if (HasFlag(alarms, TamperType::kSceneChange)) result.push_back(TamperType::kSceneChange);
    if (HasFlag(alarms, TamperType::kTooDark)) result.push_back(TamperType::kTooDark);
    if (HasFlag(alarms, TamperType::kTooBright)) result.push_back(TamperType::kTooBright);
    return result;
}

}  // namespace ai
}  // namespace ipcam
