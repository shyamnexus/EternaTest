/**
 * @file privacy_mosaic.cpp
 * @brief Privacy Mosaic / Face Blur Implementation
 * 
 * Implements privacy masking using Novatek vendor_gfx for hardware acceleration.
 * Falls back to CPU-based processing when GFX is unavailable.
 */

#include "ipcam/privacy_mosaic.h"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <cstring>

#if HDAL_PIPELINE_ENABLED && AI_GFX_ENABLED
extern "C" {
#include "vendor_gfx.h"
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// PrivacyMosaicEngine Implementation
// ============================================================================

PrivacyMosaicEngine& PrivacyMosaicEngine::Instance() {
    static PrivacyMosaicEngine instance;
    return instance;
}

PrivacyMosaicEngine::PrivacyMosaicEngine() = default;

PrivacyMosaicEngine::~PrivacyMosaicEngine() {
    Shutdown();
}

bool PrivacyMosaicEngine::Init(const PrivacyMosaicConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("PrivacyMosaic: Already initialized");
        return true;
    }
    
    config_ = config;
    
    // Initialize GFX
    if (!InitGfx()) {
        spdlog::warn("PrivacyMosaic: GFX not available, using CPU fallback");
    }
    
    initialized_.store(true);
    spdlog::info("PrivacyMosaic: Initialized (regions={}, auto_face_blur={})",
                 config_.regions.size(), config_.auto_face_blur);
    
    return true;
}

bool PrivacyMosaicEngine::InitGfx() {
#if HDAL_PIPELINE_ENABLED && AI_GFX_ENABLED
    // GFX is stateless in vendor_gfx - no init needed
    gfx_initialized_ = true;
    return true;
#else
    gfx_initialized_ = false;
    return false;
#endif
}

void PrivacyMosaicEngine::CleanupGfx() {
#if HDAL_PIPELINE_ENABLED && AI_GFX_ENABLED
    gfx_initialized_ = false;
#endif
}

void PrivacyMosaicEngine::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    CleanupGfx();
    
    initialized_.store(false);
    spdlog::info("PrivacyMosaic: Shutdown");
}

PrivacyMosaicConfig PrivacyMosaicEngine::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool PrivacyMosaicEngine::SetConfig(const PrivacyMosaicConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    return true;
}

bool PrivacyMosaicEngine::AddRegion(const PrivacyRegion& region) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if region with same ID already exists
    for (const auto& r : config_.regions) {
        if (r.id == region.id) {
            spdlog::warn("PrivacyMosaic: Region {} already exists", region.id);
            return false;
        }
    }
    
    config_.regions.push_back(region);
    spdlog::debug("PrivacyMosaic: Added region {} '{}'", region.id, region.name);
    return true;
}

bool PrivacyMosaicEngine::RemoveRegion(uint32_t region_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::remove_if(config_.regions.begin(), config_.regions.end(),
                              [region_id](const PrivacyRegion& r) {
                                  return r.id == region_id;
                              });
    
    if (it != config_.regions.end()) {
        config_.regions.erase(it, config_.regions.end());
        spdlog::debug("PrivacyMosaic: Removed region {}", region_id);
        return true;
    }
    
    return false;
}

bool PrivacyMosaicEngine::UpdateRegion(const PrivacyRegion& region) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& r : config_.regions) {
        if (r.id == region.id) {
            r = region;
            spdlog::debug("PrivacyMosaic: Updated region {}", region.id);
            return true;
        }
    }
    
    return false;
}

std::vector<PrivacyRegion> PrivacyMosaicEngine::GetRegions() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.regions;
}

bool PrivacyMosaicEngine::ApplyStaticRegions(UINTPTR frame_pa, int width, int height, int loff) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load() || frame_pa == 0) {
        return false;
    }
    
    bool success = true;
    
    for (const auto& region : config_.regions) {
        if (!region.enabled) {
            continue;
        }
        
        // Convert normalized coords to pixels
        int x = static_cast<int>(region.x * width);
        int y = static_cast<int>(region.y * height);
        int w = static_cast<int>(region.width * width);
        int h = static_cast<int>(region.height * height);
        
        // Clamp to frame bounds
        x = std::max(0, std::min(x, width - 1));
        y = std::max(0, std::min(y, height - 1));
        w = std::min(w, width - x);
        h = std::min(h, height - y);
        
        if (w <= 0 || h <= 0) {
            continue;
        }
        
        if (!DrawMosaic(frame_pa, width, height, loff, x, y, w, h, 
                        region.type, region.block_size)) {
            success = false;
        }
        
        stats_.regions_applied++;
    }
    
    stats_.frames_processed++;
    return success;
}

bool PrivacyMosaicEngine::ApplyDynamicMosaic(UINTPTR frame_pa, int width, int height, int loff,
                                              const std::vector<MosaicTarget>& targets) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load() || frame_pa == 0) {
        return false;
    }
    
    bool success = true;
    
    for (const auto& target : targets) {
        // Convert normalized coords to pixels
        int x = static_cast<int>(target.x * width);
        int y = static_cast<int>(target.y * height);
        int w = static_cast<int>(target.width * width);
        int h = static_cast<int>(target.height * height);
        
        // Clamp to frame bounds
        x = std::max(0, std::min(x, width - 1));
        y = std::max(0, std::min(y, height - 1));
        w = std::min(w, width - x);
        h = std::min(h, height - y);
        
        if (w <= 0 || h <= 0) {
            continue;
        }
        
        if (!DrawMosaic(frame_pa, width, height, loff, x, y, w, h,
                        target.type, target.block_size)) {
            success = false;
        }
        
        stats_.faces_blurred++;
    }
    
    return success;
}

bool PrivacyMosaicEngine::DrawMosaic(UINTPTR frame_pa, int width, int height, int loff,
                                      int x, int y, int w, int h, 
                                      MosaicType type, int block_size) {
#if HDAL_PIPELINE_ENABLED && AI_GFX_ENABLED
    if (gfx_initialized_) {
        // Use hardware GFX for mosaic
        VENDOR_GFX_GRPH_OBJ grph_obj = {};
        grph_obj.img_id = VENDOR_GFX_GRPH_IMG_ID_A;
        grph_obj.cmd = VENDOR_GFX_GRPH_CMD_VCOV;
        
        // Configure image A (destination)
        VENDOR_GFX_GRPH_OBJ_IMG* img_a = &grph_obj.grph_img[VENDOR_GFX_GRPH_IMG_ID_A];
        img_a->addr[0] = frame_pa;
        img_a->addr[1] = frame_pa + loff * height;
        img_a->width = width;
        img_a->height = height;
        img_a->lineofs = loff;
        img_a->fmt = VENDOR_GFX_FMT_Y_UV420;
        
        // Set mosaic rectangle
        VENDOR_GFX_GRPH_MOSAIC_INFO mosaic = {};
        mosaic.x = x;
        mosaic.y = y;
        mosaic.w = w;
        mosaic.h = h;
        
        // Block size: 0=8x8, 1=16x16, 2=32x32, 3=64x64
        if (block_size <= 8) mosaic.blk_size = 0;
        else if (block_size <= 16) mosaic.blk_size = 1;
        else if (block_size <= 32) mosaic.blk_size = 2;
        else mosaic.blk_size = 3;
        
        grph_obj.mosaic = &mosaic;
        grph_obj.mosaic_num = 1;
        
        HD_RESULT ret = vendor_gfx_grph_trigger_no_flush(&grph_obj);
        if (ret == HD_OK) {
            return true;
        }
        
        spdlog::debug("PrivacyMosaic: GFX failed, falling back to CPU");
    }
#endif
    
    // CPU fallback: simple block pixelation
    // Map frame to virtual address for CPU access
#if HDAL_PIPELINE_ENABLED
    void* frame_va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, frame_pa, loff * height * 3 / 2);
    if (frame_va == nullptr) {
        spdlog::error("PrivacyMosaic: Failed to map frame for CPU processing");
        return false;
    }
    
    uint8_t* y_plane = static_cast<uint8_t*>(frame_va);
    
    // Ensure block size is valid
    int bs = std::max(4, std::min(block_size, 64));
    
    // Process Y plane with block averaging
    for (int by = y; by < y + h; by += bs) {
        for (int bx = x; bx < x + w; bx += bs) {
            // Calculate block bounds
            int block_w = std::min(bs, x + w - bx);
            int block_h = std::min(bs, y + h - by);
            
            // Calculate average value in block
            int sum = 0;
            int count = 0;
            for (int py = by; py < by + block_h && py < height; py++) {
                for (int px = bx; px < bx + block_w && px < width; px++) {
                    sum += y_plane[py * loff + px];
                    count++;
                }
            }
            
            uint8_t avg = (count > 0) ? static_cast<uint8_t>(sum / count) : 128;
            
            // Fill block with average
            for (int py = by; py < by + block_h && py < height; py++) {
                for (int px = bx; px < bx + block_w && px < width; px++) {
                    y_plane[py * loff + px] = avg;
                }
            }
        }
    }
    
    // Flush cache
    hd_common_mem_flush_cache(frame_va, loff * height * 3 / 2);
    hd_common_mem_munmap(frame_va, loff * height * 3 / 2);
    
    return true;
#else
    // Non-HDAL: cannot access frame memory
    (void)frame_pa; (void)width; (void)height; (void)loff;
    (void)x; (void)y; (void)w; (void)h;
    (void)type; (void)block_size;
    return false;
#endif
}

PrivacyMosaicEngine::Stats PrivacyMosaicEngine::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void PrivacyMosaicEngine::ResetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Stats{};
}

// ============================================================================
// Helper Functions
// ============================================================================

std::string MosaicTypeToString(MosaicType type) {
    switch (type) {
        case MosaicType::kBlock: return "Block";
        case MosaicType::kBlur: return "Blur";
        case MosaicType::kSolid: return "Solid";
        default: return "Unknown";
    }
}

} // namespace ai
} // namespace ipcam
