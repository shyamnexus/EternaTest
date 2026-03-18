/**
 * @file privacy_mosaic.h
 * @brief Privacy Mosaic / Face Blur Module
 * 
 * Uses Novatek vendor_gfx for hardware-accelerated:
 * - Face blurring for privacy
 * - Region masking
 * - Dynamic mosaic on detected areas
 * 
 * Based on: hd_gfx_mosaic
 */

#ifndef IPCAM_PRIVACY_MOSAIC_H
#define IPCAM_PRIVACY_MOSAIC_H

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <memory>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// Privacy Mosaic Types
// ============================================================================

/**
 * @brief Mosaic type
 */
enum class MosaicType {
    kBlock = 0,      // Block pixelation
    kBlur = 1,       // Gaussian blur
    kSolid = 2       // Solid color fill
};

/**
 * @brief Privacy region (static mask)
 */
struct PrivacyRegion {
    uint32_t id = 0;
    std::string name;
    float x = 0.0f, y = 0.0f;      // Normalized 0-1
    float width = 0.1f, height = 0.1f;
    MosaicType type = MosaicType::kBlock;
    int block_size = 16;            // Mosaic block size
    uint32_t color = 0xFF000000;    // For solid fill
    bool enabled = true;
};

/**
 * @brief Dynamic mosaic target (linked to detection)
 */
struct MosaicTarget {
    float x = 0.0f, y = 0.0f;
    float width = 0.0f, height = 0.0f;
    MosaicType type = MosaicType::kBlock;
    int block_size = 8;
};

/**
 * @brief Privacy mosaic configuration
 */
struct PrivacyMosaicConfig {
    bool enabled = true;
    
    // Static privacy regions
    std::vector<PrivacyRegion> regions;
    
    // Dynamic face blur
    bool auto_face_blur = false;
    MosaicType face_blur_type = MosaicType::kBlock;
    int face_blur_block_size = 12;
    
    // Performance
    int process_fps = 30;
};

// ============================================================================
// Privacy Mosaic Engine
// ============================================================================

class PrivacyMosaicEngine {
public:
    static PrivacyMosaicEngine& Instance();
    
    PrivacyMosaicEngine(const PrivacyMosaicEngine&) = delete;
    PrivacyMosaicEngine& operator=(const PrivacyMosaicEngine&) = delete;
    
    // Lifecycle
    bool Init(const PrivacyMosaicConfig& config);
    void Shutdown();
    bool IsInitialized() const { return initialized_.load(); }
    
    // Configuration
    PrivacyMosaicConfig GetConfig() const;
    bool SetConfig(const PrivacyMosaicConfig& config);
    
    // Region management
    bool AddRegion(const PrivacyRegion& region);
    bool RemoveRegion(uint32_t region_id);
    bool UpdateRegion(const PrivacyRegion& region);
    std::vector<PrivacyRegion> GetRegions() const;
    
    /**
     * @brief Apply static privacy regions to frame
     * @param frame_pa Physical address of YUV frame
     * @param width Frame width
     * @param height Frame height
     * @param loff Line offset
     */
    bool ApplyStaticRegions(UINTPTR frame_pa, int width, int height, int loff);
    
    /**
     * @brief Apply dynamic mosaic to detected objects
     * @param frame_pa Physical address of YUV frame
     * @param width Frame width
     * @param height Frame height
     * @param loff Line offset
     * @param targets Regions to blur
     */
    bool ApplyDynamicMosaic(UINTPTR frame_pa, int width, int height, int loff,
                            const std::vector<MosaicTarget>& targets);
    
    // Statistics
    struct Stats {
        uint64_t frames_processed = 0;
        uint64_t regions_applied = 0;
        uint64_t faces_blurred = 0;
    };
    Stats GetStats() const;
    void ResetStats();
    
private:
    PrivacyMosaicEngine();
    ~PrivacyMosaicEngine();
    
    bool InitGfx();
    void CleanupGfx();
    
    bool DrawMosaic(UINTPTR frame_pa, int width, int height, int loff,
                    int x, int y, int w, int h, MosaicType type, int block_size);
    
    std::atomic<bool> initialized_{false};
    mutable std::mutex mutex_;
    
    PrivacyMosaicConfig config_;
    Stats stats_;
    
    // GFX state
    bool gfx_initialized_ = false;
    
#if HDAL_PIPELINE_ENABLED
    // GFX handle (if needed for advanced operations)
#endif
};

// ============================================================================
// Helper Functions
// ============================================================================

std::string MosaicTypeToString(MosaicType type);

} // namespace ai
} // namespace ipcam

#endif // IPCAM_PRIVACY_MOSAIC_H
