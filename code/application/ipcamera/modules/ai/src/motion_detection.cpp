/**
 * @file motion_detection.cpp
 * @brief Motion Detection Implementation using Novatek libmd
 * 
 * Real implementation using NVT_MD_MDBC, NVT_MD_GlobalAlarm, 
 * NVT_MD_SubAlarm, NVT_MD_ObjDet, and NVT_MD_CrossLine APIs.
 */

#include "ipcam/motion_detection.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <cstring>
#include <cmath>
#include <algorithm>

#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_common.h"
#include "vendor_md.h"
#include "libmd/libmd.h"
}
#endif

namespace {

/**
 * @brief Fast box-filter downscaling of Y plane image
 * 
 * Scales src (src_w x src_h) to dst (dst_w x dst_h) using simple box filtering.
 * This is efficient for MD since we don't need interpolation quality.
 * 
 * @param src Source Y plane data
 * @param src_w Source width
 * @param src_h Source height
 * @param dst Destination buffer
 * @param dst_w Destination width
 * @param dst_h Destination height
 */
void ScaleYPlane(const uint8_t* src, int src_w, int src_h,
                 uint8_t* dst, int dst_w, int dst_h) {
    // Calculate scaling ratios (use 16.16 fixed point for speed)
    const uint32_t x_ratio = ((src_w << 16) / dst_w);
    const uint32_t y_ratio = ((src_h << 16) / dst_h);
    
    for (int y = 0; y < dst_h; y++) {
        const int src_y = (y * y_ratio) >> 16;
        const uint8_t* src_row = src + src_y * src_w;
        uint8_t* dst_row = dst + y * dst_w;
        
        for (int x = 0; x < dst_w; x++) {
            const int src_x = (x * x_ratio) >> 16;
            dst_row[x] = src_row[src_x];
        }
    }
}

}  // namespace

namespace ipcam {
namespace ai {

// ============================================================================
// MotionDetectionEngine Implementation
// ============================================================================

MotionDetectionEngine& MotionDetectionEngine::Instance() {
    static MotionDetectionEngine instance;
    return instance;
}

MotionDetectionEngine::MotionDetectionEngine() = default;

MotionDetectionEngine::~MotionDetectionEngine() {
    Shutdown();
}

// Fixed internal MD resolution for efficient processing
// libmd doesn't need high resolution - 160x120 is standard for motion detection
static constexpr uint32_t MD_INTERNAL_WIDTH = 160;
static constexpr uint32_t MD_INTERNAL_HEIGHT = 120;  // ~16:10 close to 16:9

bool MotionDetectionEngine::Init(const MdConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("MD: Already initialized");
        return true;
    }
    
    config_ = config;
    
    // Override internal processing resolution for memory efficiency
    // Store original for coordinate scaling
    md_scale_x_ = static_cast<float>(config_.width) / MD_INTERNAL_WIDTH;
    md_scale_y_ = static_cast<float>(config_.height) / MD_INTERNAL_HEIGHT;
    md_internal_width_ = MD_INTERNAL_WIDTH;
    md_internal_height_ = MD_INTERNAL_HEIGHT;
    
#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
    // Initialize vendor MD wrapper
    INT32 init_ret = vendor_md_init();
    if (init_ret != 0) {
        spdlog::error("MD: vendor_md_init failed: {}", init_ret);
        return false;
    }
    
    // Allocate DMA buffers for MD processing (at internal resolution)
    if (!AllocateBuffers()) {
        spdlog::error("MD: Failed to allocate DMA buffers");
        vendor_md_uninit();
        return false;
    }
    
    spdlog::info("MD: Initialized {}x{} (internal: {}x{}), mode={}", 
                 config_.width, config_.height,
                 md_internal_width_, md_internal_height_,
                 MotionModeToString(config_.mode));
#else
    spdlog::info("MD: Initialized (SDK libraries not available, using software fallback)");
#endif
    
    is_init_mode_ = true;
    initialized_.store(true);
    return true;
}

#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
bool MotionDetectionEngine::AllocateBuffers() {
    // Use internal MD resolution for memory efficiency (160x120 is typical for libmd)
    uint32_t buf_size = md_internal_width_ * md_internal_height_;
    HD_RESULT ret;
    void* va_ptr = nullptr;
    
    spdlog::debug("MD: Allocating buffers at internal resolution {}x{} (buf_size={})",
                  md_internal_width_, md_internal_height_, buf_size);
    
    // Use hd_common_mem_alloc which allocates from general memory region
    // This is more flexible than hd_common_mem_get_block which requires pools to be pre-configured
    
    // Source image buffer (input Y plane)
    va_ptr = nullptr;
    ret = hd_common_mem_alloc(const_cast<char*>("md_src"), &src_buf_pa_, &va_ptr, buf_size, DDR_ID0);
    if (ret != HD_OK) {
        spdlog::error("MD: Failed to allocate source buffer ({} bytes), ret={}", buf_size, static_cast<int>(ret));
        return false;
    }
    src_buf_va_ = reinterpret_cast<UINTPTR>(va_ptr);
    src_buf_size_ = buf_size;
    
    // Destination buffer (MDBC output)
    va_ptr = nullptr;
    ret = hd_common_mem_alloc(const_cast<char*>("md_dst"), &dst_buf_pa_, &va_ptr, buf_size, DDR_ID0);
    if (ret != HD_OK) {
        spdlog::error("MD: Failed to allocate destination buffer ({} bytes)", buf_size);
        CleanupBuffers();
        return false;
    }
    dst_buf_va_ = reinterpret_cast<UINTPTR>(va_ptr);
    dst_buf_size_ = buf_size;
    
    // Temporary work buffer for MDBC (requires ~48x frame size per SDK samples)
    uint32_t temp_size = buf_size * 48;
    va_ptr = nullptr;
    ret = hd_common_mem_alloc(const_cast<char*>("md_temp"), &temp_buf_pa_, &va_ptr, temp_size, DDR_ID0);
    if (ret != HD_OK) {
        spdlog::error("MD: Failed to allocate temp buffer ({} bytes)", temp_size);
        CleanupBuffers();
        return false;
    }
    temp_buf_va_ = reinterpret_cast<UINTPTR>(va_ptr);
    temp_buf_size_ = temp_size;
    
    // Object detection point stack buffer (required by NVT_MD_ObjDet)
    // Size: width * height * sizeof(MD_PT_INFO_S) - allocate for alignment
    uint32_t objpt_size = buf_size * sizeof(uint32_t) * 2;  // MD_PT_INFO_S is {u32X, u32Y}
    va_ptr = nullptr;
    ret = hd_common_mem_alloc(const_cast<char*>("md_objpt"), &objpt_buf_pa_, &va_ptr, objpt_size, DDR_ID0);
    if (ret != HD_OK) {
        spdlog::error("MD: Failed to allocate objpt buffer ({} bytes)", objpt_size);
        CleanupBuffers();
        return false;
    }
    objpt_buf_va_ = reinterpret_cast<UINTPTR>(va_ptr);
    objpt_buf_size_ = objpt_size;
    
    // Initialize cross-line tracking state
    cross_prev_obj_num_ = 0;
    
    spdlog::info("MD: Allocated buffers - src:{}, dst:{}, temp:{}, objpt:{} bytes",
                 src_buf_size_, dst_buf_size_, temp_buf_size_, objpt_buf_size_);
    
    return true;
}

void MotionDetectionEngine::CleanupBuffers() {
    // For hd_common_mem_alloc, we use hd_common_mem_free(pa, va)
    if (objpt_buf_va_ != 0 && objpt_buf_pa_ != 0) {
        hd_common_mem_free(objpt_buf_pa_, reinterpret_cast<void*>(objpt_buf_va_));
        objpt_buf_va_ = 0;
        objpt_buf_pa_ = 0;
    }
    
    if (temp_buf_va_ != 0 && temp_buf_pa_ != 0) {
        hd_common_mem_free(temp_buf_pa_, reinterpret_cast<void*>(temp_buf_va_));
        temp_buf_va_ = 0;
        temp_buf_pa_ = 0;
    }
    
    if (dst_buf_va_ != 0 && dst_buf_pa_ != 0) {
        hd_common_mem_free(dst_buf_pa_, reinterpret_cast<void*>(dst_buf_va_));
        dst_buf_va_ = 0;
        dst_buf_pa_ = 0;
    }
    
    if (src_buf_va_ != 0 && src_buf_pa_ != 0) {
        hd_common_mem_free(src_buf_pa_, reinterpret_cast<void*>(src_buf_va_));
        src_buf_va_ = 0;
        src_buf_pa_ = 0;
    }
}
#endif

bool MotionDetectionEngine::Start() {
    if (!initialized_.load()) {
        spdlog::error("MD: Not initialized");
        return false;
    }
    
    if (running_.load()) {
        return true;
    }
    
    is_init_mode_ = true;  // Reset background model on start
    running_.store(true);
    spdlog::info("MD: Started");
    return true;
}

void MotionDetectionEngine::Stop() {
    running_.store(false);
    spdlog::info("MD: Stopped");
}

void MotionDetectionEngine::Shutdown() {
    Stop();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
    CleanupBuffers();
    vendor_md_uninit();
#endif
    
    zones_.clear();
    cross_lines_.clear();
    
    initialized_.store(false);
    spdlog::info("MD: Shutdown complete");
}

MdConfig MotionDetectionEngine::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool MotionDetectionEngine::SetConfig(const MdConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    bool needs_reinit = (config.width != config_.width || 
                         config.height != config_.height);
    
    config_ = config;
    
    if (needs_reinit && initialized_.load()) {
        spdlog::warn("MD: Resolution change requires restart");
    }
    
    return true;
}

void MotionDetectionEngine::AddZone(const MdZone& zone) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Update existing or add new
    for (auto& existing : zones_) {
        if (existing.id == zone.id) {
            existing = zone;
            spdlog::info("MD: Updated zone {} ({},{}) - ({},{})",
                         zone.id, zone.x_start, zone.y_start, zone.x_end, zone.y_end);
            return;
        }
    }
    
    zones_.push_back(zone);
    spdlog::info("MD: Added zone {} ({},{}) - ({},{})",
                 zone.id, zone.x_start, zone.y_start, zone.x_end, zone.y_end);
}

void MotionDetectionEngine::RemoveZone(int zone_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::remove_if(zones_.begin(), zones_.end(),
        [zone_id](const MdZone& z) { return static_cast<int>(z.id) == zone_id; });
    
    if (it != zones_.end()) {
        zones_.erase(it, zones_.end());
        spdlog::info("MD: Removed zone {}", zone_id);
    }
}

void MotionDetectionEngine::ClearZones() {
    std::lock_guard<std::mutex> lock(mutex_);
    zones_.clear();
    spdlog::info("MD: Cleared all zones");
}

void MotionDetectionEngine::AddCrossLine(const MdCrossLine& line) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& existing : cross_lines_) {
        if (existing.id == line.id) {
            existing = line;
            spdlog::info("MD: Updated cross line {} ({},{}) - ({},{})",
                         line.id, line.x1, line.y1, line.x2, line.y2);
            return;
        }
    }
    
    cross_lines_.push_back(line);
    spdlog::info("MD: Added cross line {} ({},{}) - ({},{})",
                 line.id, line.x1, line.y1, line.x2, line.y2);
}

void MotionDetectionEngine::RemoveCrossLine(int line_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::remove_if(cross_lines_.begin(), cross_lines_.end(),
        [line_id](const MdCrossLine& l) { return static_cast<int>(l.id) == line_id; });
    
    if (it != cross_lines_.end()) {
        cross_lines_.erase(it, cross_lines_.end());
        spdlog::info("MD: Removed cross line {}", line_id);
    }
}

void MotionDetectionEngine::ClearCrossLines() {
    std::lock_guard<std::mutex> lock(mutex_);
    cross_lines_.clear();
    spdlog::info("MD: Cleared all cross lines");
}

void MotionDetectionEngine::SetCallback(MdCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = std::move(callback);
}

void MotionDetectionEngine::SetLoiteringCallback(LoiteringCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    loitering_callback_ = std::move(callback);
}

std::vector<MdLineCrossCount> MotionDetectionEngine::GetLineCounts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MdLineCrossCount> counts;
    for (const auto& [line_id, count] : line_counts_) {
        counts.push_back(count);
    }
    return counts;
}

MdLineCrossCount MotionDetectionEngine::GetLineCount(uint32_t line_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = line_counts_.find(line_id);
    if (it != line_counts_.end()) {
        return it->second;
    }
    return MdLineCrossCount{line_id, 0, 0, 0};
}

void MotionDetectionEngine::ResetLineCounts() {
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    for (auto& [line_id, count] : line_counts_) {
        count.count_in = 0;
        count.count_out = 0;
        count.last_reset_time = now;
    }
    spdlog::info("MD: Reset all line crossing counts");
}

void MotionDetectionEngine::ResetLineCount(uint32_t line_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = line_counts_.find(line_id);
    if (it != line_counts_.end()) {
        it->second.count_in = 0;
        it->second.count_out = 0;
        it->second.last_reset_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        spdlog::info("MD: Reset line {} crossing count", line_id);
    }
}

MdResult MotionDetectionEngine::ProcessFrame(const uint8_t* y_data, int width, int height) {
    MdResult result;
    result.motion_detected = false;
    
    if (!running_.load()) {
        return result;
    }
    
    if (!y_data || width <= 0 || height <= 0) {
        return result;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Scale input to internal MD resolution if needed
    // libmd works at 160x120 for efficiency, input is typically 640x360 or larger
    int proc_width = static_cast<int>(md_internal_width_);
    int proc_height = static_cast<int>(md_internal_height_);
    
    if (width != proc_width || height != proc_height) {
        // Scale down to internal resolution before processing
        ScaleYPlane(y_data, width, height,
                    reinterpret_cast<uint8_t*>(src_buf_va_), proc_width, proc_height);
    } else {
        // Same size - direct copy
        std::memcpy(reinterpret_cast<void*>(src_buf_va_), y_data, proc_width * proc_height);
    }
    hd_common_mem_flush_cache(reinterpret_cast<void*>(src_buf_va_), proc_width * proc_height);
    
    // Run MDBC (Motion Detection with Background Compensation) at internal resolution
    int md_ret = RunMdbc(proc_width, proc_height);
    if (md_ret != LIB_MD_OK) {
        spdlog::error("MD: MDBC failed with error {}", md_ret);
        return result;
    }
    
    // Invalidate dst buffer cache to get the updated MDBC output
    hd_common_mem_flush_cache(reinterpret_cast<void*>(dst_buf_va_), proc_width * proc_height);
    
    // First frame is initialization only
    if (is_init_mode_) {
        is_init_mode_ = false;
        spdlog::debug("MD: Background model initialized");
        return result;
    }
    
    result.timestamp_ms = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    
    // Process based on configured mode
    switch (config_.mode) {
        case MotionMode::kGlobalAlarm:
            result.motion_detected = RunGlobalAlarm();
            result.motion_level = result.motion_detected ? 100.0f : 0.0f;
            break;
            
        case MotionMode::kSubAlarm:
            RunSubAlarm(result);
            break;
            
        case MotionMode::kObjectDetect:
            RunObjectDetection(result);
            break;
            
        case MotionMode::kCrossLine:
            RunCrossLineDetection(result);
            break;
            
        case MotionMode::kMdbc:
        default:
            // Just MDBC - count motion pixels
            {
                uint8_t* dst = reinterpret_cast<uint8_t*>(dst_buf_va_);
                int motion_count = 0;
                int total_pixels = proc_width * proc_height;
                for (int i = 0; i < total_pixels; i++) {
                    if (dst[i] > 0) motion_count++;
                }
                result.motion_block_count = motion_count;
                result.total_blocks = total_pixels;
                result.motion_level = (motion_count * 100.0f) / total_pixels;
                result.motion_detected = (result.motion_level > 1.0f);
            }
            break;
    }
    
    // Update loitering state for zones with detected objects
    UpdateLoiteringState(result);
    
    // Update line crossing counts
    UpdateLineCounts(result);
    
    // Copy line counts to result
    for (const auto& [line_id, count] : line_counts_) {
        result.line_counts.push_back(count);
    }
#else
    // SDK not available - motion detection disabled
    spdlog::debug("MD: SDK library not available, motion detection disabled");
    (void)y_data;
    (void)width;
    (void)height;
#endif
    
    // Update statistics
    stats_.frames_processed++;
    if (result.motion_detected) {
        stats_.motion_events++;
    }
    stats_.loitering_events += result.loitering_events.size();
    stats_.line_cross_events += result.line_events.size();
    
    auto end = std::chrono::high_resolution_clock::now();
    float processing_time = std::chrono::duration<float, std::milli>(end - start).count();
    stats_.avg_processing_time_ms = (stats_.avg_processing_time_ms * 0.9f) + (processing_time * 0.1f);
    stats_.last_result = result;
    
    // Invoke callbacks
    if (callback_) {
        callback_(result);
    }
    
    // Invoke loitering callback for each loitering event
    if (loitering_callback_) {
        for (const auto& evt : result.loitering_events) {
            loitering_callback_(evt);
        }
    }
    
    return result;
}

#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
int MotionDetectionEngine::RunMdbc(int width, int height) {
    MD_SRC_IMAGE_S src_img = {};
    src_img.u32Width = static_cast<uint32_t>(width);
    src_img.u32Height = static_cast<uint32_t>(height);
    src_img.u32Stride = static_cast<uint32_t>(width);
    src_img.u64Pa = src_buf_pa_;
    src_img.u64Va = src_buf_va_;
    
    MD_SRC_IMAGE_S dst_img = {};
    dst_img.u32Width = static_cast<uint32_t>(width);
    dst_img.u32Height = static_cast<uint32_t>(height);
    dst_img.u32Stride = static_cast<uint32_t>(width);
    dst_img.u64Pa = dst_buf_pa_;
    dst_img.u64Va = dst_buf_va_;
    
    MD_MDBC_CTRL_S mdbc_ctrl = {};
    mdbc_ctrl.enInFmt = LIB_MD_IMG_Y_ONLY;
    mdbc_ctrl.enIsInit = is_init_mode_ ? LIB_MD_BC_INIT_MODE : LIB_MD_BC_NORM_MODE;
    mdbc_ctrl.enBgNum = LIB_MD_BG_NUM_8;  // Number of background frames for averaging
    
    // Set sensitivity level based on config
    switch (config_.sensitivity) {
        case 1:
            mdbc_ctrl.enSensiLevel = LIB_MD_BC_LOW_SENSI;
            break;
        case 3:
            mdbc_ctrl.enSensiLevel = LIB_MD_BC_HIGH_SENSI;
            break;
        case 2:
        default:
            mdbc_ctrl.enSensiLevel = LIB_MD_BC_MED_SENSI;
            break;
    }
    
    mdbc_ctrl.enConvgSpd = LIB_MD_BC_MED_SPD;  // Medium convergence speed (was HIGH)
    
    // Configure morphology operations for noise reduction
    mdbc_ctrl.stMorph[0].enMorph = LIB_MD_MORPH_ENABLE;
    mdbc_ctrl.stMorph[0].u8MorphThres = 4;
    mdbc_ctrl.stMorph[1].enMorph = LIB_MD_MORPH_ENABLE;
    mdbc_ctrl.stMorph[1].u8MorphThres = 0;
    mdbc_ctrl.stMorph[2].enMorph = LIB_MD_MORPH_ENABLE;
    mdbc_ctrl.stMorph[2].u8MorphThres = 8;
    mdbc_ctrl.stMorph[3].enMorph = LIB_MD_MORPH_BYPASS;
    mdbc_ctrl.stMorph[3].u8MorphThres = 0;
    
    mdbc_ctrl.u32TmpThres = 0;
    mdbc_ctrl.stTempMem.u64Va = temp_buf_va_;
    mdbc_ctrl.stTempMem.u64Pa = temp_buf_pa_;
    mdbc_ctrl.stTempMem.u32Size = temp_buf_size_;
    
    return static_cast<int>(NVT_MD_MDBC(&src_img, &dst_img, &mdbc_ctrl));
}

bool MotionDetectionEngine::RunGlobalAlarm() {
    MD_SRC_IMAGE_S dst_img = {};
    dst_img.u32Width = md_internal_width_;
    dst_img.u32Height = md_internal_height_;
    dst_img.u32Stride = md_internal_width_;
    dst_img.u64Pa = dst_buf_pa_;
    dst_img.u64Va = dst_buf_va_;
    
    MD_GALARM_CTRL_S ctrl = {};
    ctrl.u8AlarmThres = config_.global_threshold;
    
    uint8_t alarm_result = 0;
    
    INT32 ret = NVT_MD_GlobalAlarm(&dst_img, &alarm_result, &ctrl);
    
    // Debug: log threshold and result periodically
    static int call_count = 0;
    if (++call_count % 100 == 0) {
        spdlog::debug("MD GlobalAlarm: threshold={}, result={}", config_.global_threshold, alarm_result);
    }
    if (ret != static_cast<INT32>(LIB_MD_OK)) {
        spdlog::error("MD: NVT_MD_GlobalAlarm failed: {}", static_cast<int>(ret));
        return false;
    }
    
    return (alarm_result == 1);
}

void MotionDetectionEngine::RunSubAlarm(MdResult& result) {
    if (zones_.empty()) {
        // Fall back to global alarm if no zones defined
        result.motion_detected = RunGlobalAlarm();
        result.motion_level = result.motion_detected ? 100.0f : 0.0f;
        return;
    }
    
    MD_SRC_IMAGE_S dst_img = {};
    dst_img.u32Width = md_internal_width_;
    dst_img.u32Height = md_internal_height_;
    dst_img.u32Stride = md_internal_width_;
    dst_img.u64Pa = dst_buf_pa_;
    dst_img.u64Va = dst_buf_va_;
    
    // Prepare sub-region parameters - scale coordinates to internal resolution
    std::vector<MD_SUBPARAM_CTRL_S> sub_params(zones_.size());
    std::vector<uint8_t> sub_results(zones_.size(), 0);
    
    for (size_t i = 0; i < zones_.size(); i++) {
        const auto& zone = zones_[i];
        sub_params[i].enSubRegion = zone.enabled ? 1 : 0;
        // Scale zone coordinates from config space to internal MD space
        sub_params[i].u32XStart = static_cast<uint32_t>(zone.x_start / md_scale_x_);
        sub_params[i].u32YStart = static_cast<uint32_t>(zone.y_start / md_scale_y_);
        sub_params[i].u32XEnd = static_cast<uint32_t>(zone.x_end / md_scale_x_);
        sub_params[i].u32YEnd = static_cast<uint32_t>(zone.y_end / md_scale_y_);
        sub_params[i].u8SubAlarmThres = zone.threshold;
    }
    
    MD_SUBALARM_CTRL_S ctrl = {};
    ctrl.pstSubParam = sub_params.data();
    ctrl.u8SubNum = static_cast<uint8_t>(zones_.size());
    
    INT32 ret = NVT_MD_SubAlarm(&dst_img, sub_results.data(), &ctrl);
    if (ret != static_cast<INT32>(LIB_MD_OK)) {
        spdlog::error("MD: NVT_MD_SubAlarm failed: {}", ret);
        return;
    }
    
    // Process results for each zone
    for (size_t i = 0; i < zones_.size(); i++) {
        if (sub_results[i] == 1) {
            result.motion_detected = true;
            
            MdZoneResult zone_result;
            zone_result.zone_id = zones_[i].id;
            zone_result.motion_detected = true;
            zone_result.motion_level = 100.0f;
            result.zone_results.push_back(zone_result);
        }
    }
    
    if (result.motion_detected) {
        result.motion_level = (result.zone_results.size() * 100.0f) / zones_.size();
    }
}

void MotionDetectionEngine::RunObjectDetection(MdResult& result) {
    // Prepare destination image (MDBC output / foreground mask) at internal resolution
    MD_SRC_IMAGE_S dst_img = {};
    dst_img.u32Width = md_internal_width_;
    dst_img.u32Height = md_internal_height_;
    dst_img.u32Stride = md_internal_width_;
    dst_img.u64Pa = dst_buf_pa_;
    dst_img.u64Va = dst_buf_va_;
    
    // Object detection output structure (SDK uses MD_OBJ_INFO_S, NOT separate array)
    MD_OBJ_INFO_S obj_info = {};
    
    // Object detection control
    MD_OBJ_CTRL_S obj_ctrl = {};
    obj_ctrl.u32ObjThres = 50;  // Minimum object area threshold
    obj_ctrl.enInRange = LIB_MD_IN_RANGE_0_1;  // Input is 0/1 binary mask
    obj_ctrl.pstPtStk = reinterpret_cast<MD_PT_INFO_S*>(objpt_buf_va_);  // Required point stack
    
    // Clear point stack before use
    std::memset(reinterpret_cast<void*>(objpt_buf_va_), 0, objpt_buf_size_);
    
    // Call correct API: NVT_MD_ObjDet(src, &obj_info, &ctrl)
    INT32 ret = NVT_MD_ObjDet(&dst_img, &obj_info, &obj_ctrl);
    if (ret != static_cast<INT32>(LIB_MD_OK)) {
        spdlog::error("MD: NVT_MD_ObjDet failed: {}", ret);
        // Fall back to global alarm
        result.motion_detected = RunGlobalAlarm();
        return;
    }
    
    // Convert detected objects - NOTE: libmd uses 1-BASED indexing!
    // Scale coordinates from internal MD space back to config space
    for (uint32_t i = 1; i <= obj_info.u32ObjNum && i < LIB_MD_MAX_OBJ_NUM; i++) {
        MdDetectedObject obj;
        obj.id = i;
        obj.x = static_cast<int>(obj_info.stObjRst[i].u32XStart * md_scale_x_);
        obj.y = static_cast<int>(obj_info.stObjRst[i].u32YStart * md_scale_y_);
        obj.width = static_cast<int>((obj_info.stObjRst[i].u32XEnd - obj_info.stObjRst[i].u32XStart) * md_scale_x_);
        obj.height = static_cast<int>((obj_info.stObjRst[i].u32YEnd - obj_info.stObjRst[i].u32YStart) * md_scale_y_);
        obj.confidence = 1.0f;
        result.detected_objects.push_back(obj);
    }
    
    result.motion_detected = (obj_info.u32ObjNum > 0);
    result.motion_level = std::min(100.0f, static_cast<float>(obj_info.u32ObjNum) * 10.0f);
}

void MotionDetectionEngine::RunCrossLineDetection(MdResult& result) {
    if (cross_lines_.empty()) {
        result.motion_detected = RunGlobalAlarm();
        return;
    }
    
    // First detect objects using the corrected API at internal resolution
    MD_SRC_IMAGE_S dst_img = {};
    dst_img.u32Width = md_internal_width_;
    dst_img.u32Height = md_internal_height_;
    dst_img.u32Stride = md_internal_width_;
    dst_img.u64Pa = dst_buf_pa_;
    dst_img.u64Va = dst_buf_va_;
    
    // Object detection
    MD_OBJ_INFO_S obj_info = {};
    MD_OBJ_CTRL_S obj_ctrl = {};
    obj_ctrl.u32ObjThres = 50;
    obj_ctrl.enInRange = LIB_MD_IN_RANGE_0_1;
    obj_ctrl.pstPtStk = reinterpret_cast<MD_PT_INFO_S*>(objpt_buf_va_);
    std::memset(reinterpret_cast<void*>(objpt_buf_va_), 0, objpt_buf_size_);
    
    INT32 ret = NVT_MD_ObjDet(&dst_img, &obj_info, &obj_ctrl);
    if (ret != static_cast<INT32>(LIB_MD_OK)) {
        spdlog::error("MD: NVT_MD_ObjDet failed in CrossLine: {}", ret);
        cross_prev_obj_num_ = 0;  // Reset tracking
        return;
    }
    
    if (obj_info.u32ObjNum == 0) {
        cross_prev_obj_num_ = 0;  // Reset tracking when no objects
        return;
    }
    
    // Convert to MdDetectedObject for result (1-based indexing)
    // Scale coordinates from internal MD space back to config space
    for (uint32_t i = 1; i <= obj_info.u32ObjNum && i < LIB_MD_MAX_OBJ_NUM; i++) {
        MdDetectedObject obj;
        obj.id = i;
        obj.x = static_cast<int>(obj_info.stObjRst[i].u32XStart * md_scale_x_);
        obj.y = static_cast<int>(obj_info.stObjRst[i].u32YStart * md_scale_y_);
        obj.width = static_cast<int>((obj_info.stObjRst[i].u32XEnd - obj_info.stObjRst[i].u32XStart) * md_scale_x_);
        obj.height = static_cast<int>((obj_info.stObjRst[i].u32YEnd - obj_info.stObjRst[i].u32YStart) * md_scale_y_);
        obj.confidence = 1.0f;
        result.detected_objects.push_back(obj);
    }
    result.motion_detected = true;
    
    // Process each configured cross-line
    for (const auto& line : cross_lines_) {
        if (!line.enabled) continue;
        
        // Prepare cross-line objects array (1-based indexing)
        MD_CROSS_OBJ_INFO_S cross_objs[LIB_MD_MAX_OBJ_NUM] = {};
        LIB_MD_CROSS_RST cross_results[LIB_MD_MAX_OBJ_NUM] = {};
        
        // Transfer objects to cross-line structure
        for (uint32_t i = 1; i <= obj_info.u32ObjNum && i < LIB_MD_MAX_OBJ_NUM; i++) {
            cross_objs[i].u32XStart = obj_info.stObjRst[i].u32XStart;
            cross_objs[i].u32YStart = obj_info.stObjRst[i].u32YStart;
            cross_objs[i].u32XEnd = obj_info.stObjRst[i].u32XEnd;
            cross_objs[i].u32YEnd = obj_info.stObjRst[i].u32YEnd;
            cross_objs[i].u8Valid = 1;
            cross_objs[i].u8Track = 0;
        }
        
        // Configure cross-line control (scale line coordinates from config to internal MD space)
        // Line coordinates are typically in config space (e.g., 640x360)
        
        MD_CROSS_CTRL_S cross_ctrl = {};
        cross_ctrl.enCrossMode = LIB_MD_CROSS_LINE;
        cross_ctrl.u8ObjNum = static_cast<uint8_t>(obj_info.u32ObjNum);
        cross_ctrl.u32OverlapTh = 10;
        
        // Scale line coordinates from config space to internal MD space
        int lx1 = static_cast<int>(line.x1 / md_scale_x_);
        int ly1 = static_cast<int>(line.y1 / md_scale_y_);
        int lx2 = static_cast<int>(line.x2 / md_scale_x_);
        int ly2 = static_cast<int>(line.y2 / md_scale_y_);
        
        // Create corridor around line (5 pixels wide) in internal MD space
        int md_width = static_cast<int>(md_internal_width_);
        int md_height = static_cast<int>(md_internal_height_);
        cross_ctrl.u32X1 = std::max(0, std::min(lx1, lx2) - 2);
        cross_ctrl.u32Y1 = std::max(0, std::min(ly1, ly2) - 2);
        cross_ctrl.u32X2 = std::min(md_width - 1, std::max(lx1, lx2) + 2);
        cross_ctrl.u32Y2 = std::max(0, std::min(ly1, ly2) - 2);
        cross_ctrl.u32X3 = std::min(md_width - 1, std::max(lx1, lx2) + 2);
        cross_ctrl.u32Y3 = std::min(md_height - 1, std::max(ly1, ly2) + 2);
        cross_ctrl.u32X4 = std::max(0, std::min(lx1, lx2) - 2);
        cross_ctrl.u32Y4 = std::min(md_height - 1, std::max(ly1, ly2) + 2);
        
        // Previous frame tracking (required for direction detection)
        cross_ctrl.u8PreObjNum = cross_prev_obj_num_;
        // Note: stPreObj should be persisted, but for simplicity we track count only
        
        // Call NVT_MD_CrossLine
        ret = NVT_MD_CrossLine(cross_objs, cross_results, &cross_ctrl);
        if (ret != static_cast<INT32>(LIB_MD_OK)) {
            spdlog::warn("MD: NVT_MD_CrossLine failed: {}", ret);
            continue;
        }
        
        // Check results (1-based indexing)
        for (uint32_t i = 1; i <= cross_ctrl.u8ObjNum && i < LIB_MD_MAX_OBJ_NUM; i++) {
            if (cross_results[i] == LIB_MD_CROSS_IN_TO_OUT || 
                cross_results[i] == LIB_MD_CROSS_OUT_TO_IN) {
                
                MdCrossLineEvent event;
                event.line_id = line.id;
                event.object_id = i;
                event.direction = (cross_results[i] == LIB_MD_CROSS_IN_TO_OUT) 
                    ? MdCrossDirection::kLeftToRight : MdCrossDirection::kRightToLeft;
                event.timestamp_ms = result.timestamp_ms;
                result.line_events.push_back(event);
                
                spdlog::debug("MD: Cross-line event - line:{} obj:{} dir:{}", 
                              line.id, i, 
                              cross_results[i] == LIB_MD_CROSS_IN_TO_OUT ? "IN->OUT" : "OUT->IN");
            }
        }
    }
    
    // Save current object count for next frame tracking
    cross_prev_obj_num_ = static_cast<uint8_t>(obj_info.u32ObjNum);
}

void MotionDetectionEngine::UpdateLoiteringState(MdResult& result) {
    // Get current time
    uint64_t now_ms = result.timestamp_ms;
    if (now_ms == 0) {
        now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    // For each detected object, check if it's in a loitering-enabled zone
    for (const auto& obj : result.detected_objects) {
        // Calculate object center
        int obj_cx = obj.x + obj.width / 2;
        int obj_cy = obj.y + obj.height / 2;
        
        for (const auto& zone : zones_) {
            if (!zone.enabled || !zone.loitering_enabled) continue;
            
            // Check if object center is inside zone
            bool in_zone = (obj_cx >= static_cast<int>(zone.x_start) && 
                           obj_cx <= static_cast<int>(zone.x_end) &&
                           obj_cy >= static_cast<int>(zone.y_start) && 
                           obj_cy <= static_cast<int>(zone.y_end));
            
            // Find existing state for this object+zone
            auto it = std::find_if(zone_object_states_.begin(), zone_object_states_.end(),
                [&](const MdZoneObjectState& s) { 
                    return s.object_id == obj.id && s.zone_id == zone.id; 
                });
            
            if (in_zone) {
                if (it == zone_object_states_.end()) {
                    // New object in zone - start tracking
                    MdZoneObjectState state;
                    state.object_id = obj.id;
                    state.zone_id = zone.id;
                    state.enter_time_ms = now_ms;
                    state.loitering_triggered = false;
                    zone_object_states_.push_back(state);
                    spdlog::debug("MD: Object {} entered loitering zone {}", obj.id, zone.id);
                } else {
                    // Object still in zone - check if loitering threshold exceeded
                    uint64_t dwell_time = now_ms - it->enter_time_ms;
                    
                    if (!it->loitering_triggered && dwell_time >= zone.loitering_threshold_ms) {
                        // Trigger loitering event
                        MdLoiteringEvent evt;
                        evt.zone_id = zone.id;
                        evt.object_id = obj.id;
                        evt.dwell_time_ms = dwell_time;
                        evt.threshold_ms = zone.loitering_threshold_ms;
                        evt.timestamp_ms = now_ms;
                        result.loitering_events.push_back(evt);
                        
                        it->loitering_triggered = true;
                        spdlog::info("MD: Loitering detected - obj:{} zone:{} dwell:{}ms", 
                                    obj.id, zone.id, dwell_time);
                    }
                }
            } else if (it != zone_object_states_.end()) {
                // Object left zone - remove tracking
                spdlog::debug("MD: Object {} left loitering zone {}", obj.id, zone.id);
                zone_object_states_.erase(it);
            }
        }
    }
    
    // Clean up stale object states (objects no longer detected)
    zone_object_states_.erase(
        std::remove_if(zone_object_states_.begin(), zone_object_states_.end(),
            [&result](const MdZoneObjectState& s) {
                return std::find_if(result.detected_objects.begin(), result.detected_objects.end(),
                    [&s](const MdDetectedObject& obj) { return obj.id == s.object_id; }) 
                    == result.detected_objects.end();
            }),
        zone_object_states_.end()
    );
}

void MotionDetectionEngine::UpdateLineCounts(MdResult& result) {
    uint64_t now_ms = result.timestamp_ms;
    if (now_ms == 0) {
        now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    // Initialize line counts for new lines
    for (const auto& line : cross_lines_) {
        if (line.counting_enabled && line_counts_.find(line.id) == line_counts_.end()) {
            MdLineCrossCount count;
            count.line_id = line.id;
            count.count_in = 0;
            count.count_out = 0;
            count.last_reset_time = now_ms;
            line_counts_[line.id] = count;
        }
    }
    
    // Update counts from line events
    for (const auto& event : result.line_events) {
        auto it = line_counts_.find(event.line_id);
        if (it != line_counts_.end()) {
            // Check for auto-reset interval
            auto line_it = std::find_if(cross_lines_.begin(), cross_lines_.end(),
                [&event](const MdCrossLine& l) { return l.id == event.line_id; });
            
            if (line_it != cross_lines_.end() && line_it->counting_enabled) {
                uint64_t reset_interval_ms = line_it->count_reset_interval_sec * 1000ULL;
                if (now_ms - it->second.last_reset_time > reset_interval_ms) {
                    it->second.count_in = 0;
                    it->second.count_out = 0;
                    it->second.last_reset_time = now_ms;
                    spdlog::info("MD: Auto-reset line {} count (interval: {}s)", 
                                event.line_id, line_it->count_reset_interval_sec);
                }
            }
            
            // Count based on direction
            if (event.direction == MdCrossDirection::kLeftToRight ||
                event.direction == MdCrossDirection::kTopToBottom) {
                it->second.count_in++;
            } else if (event.direction == MdCrossDirection::kRightToLeft ||
                       event.direction == MdCrossDirection::kBottomToTop) {
                it->second.count_out++;
            } else {
                // Unknown direction - count as "in"
                it->second.count_in++;
            }
            
            spdlog::debug("MD: Line {} count updated - in:{} out:{}", 
                         event.line_id, it->second.count_in, it->second.count_out);
        }
    }
}
#endif

MotionDetectionEngine::Stats MotionDetectionEngine::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void MotionDetectionEngine::ResetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Stats{};
}

// ============================================================================
// Helper Functions
// ============================================================================

std::string MotionModeToString(MotionMode mode) {
    switch (mode) {
        case MotionMode::kGlobalAlarm: return "GlobalAlarm";
        case MotionMode::kSubAlarm: return "SubAlarm";
        case MotionMode::kObjectDetect: return "ObjectDetect";
        case MotionMode::kCrossLine: return "CrossLine";
        case MotionMode::kMdbc: return "MDBC";
        default: return "Unknown";
    }
}

std::string MdCrossDirectionToString(MdCrossDirection dir) {
    switch (dir) {
        case MdCrossDirection::kLeftToRight: return "LeftToRight";
        case MdCrossDirection::kRightToLeft: return "RightToLeft";
        case MdCrossDirection::kTopToBottom: return "TopToBottom";
        case MdCrossDirection::kBottomToTop: return "BottomToTop";
        case MdCrossDirection::kUnknown:
        default: return "Unknown";
    }
}

} // namespace ai
} // namespace ipcam
