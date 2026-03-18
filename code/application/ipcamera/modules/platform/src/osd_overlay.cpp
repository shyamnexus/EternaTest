/**
 * @file osd_overlay.cpp
 * @brief OSD Overlay Manager Implementation
 * 
 * Implements the OSD overlay functionality using FreeType for text rendering
 * and HDAL OSG APIs for video encoder overlay.
 */

#include "ipcam/osd_overlay.h"
#include <cstring>
#include <ctime>
#include <cstdio>
#include <chrono>
#include <cmath>
#include <spdlog/spdlog.h>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
}
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

// Config system
#include "ipcam/config.h"
#include "ipcam/hdal_pipeline.h"

namespace ipcam {
namespace platform {

// ============================================================================
// Thick outline offsets (2-pixel outline)
// ============================================================================
static const int kOutlineOffsets[][2] = {
    // Inner ring (1 pixel)
    {-1, -1}, {0, -1}, {1, -1},
    {-1,  0},          {1,  0},
    {-1,  1}, {0,  1}, {1,  1},
    // Outer ring (2 pixels)
    {-2, -2}, {-1, -2}, {0, -2}, {1, -2}, {2, -2},
    {-2, -1},                             {2, -1},
    {-2,  0},                             {2,  0},
    {-2,  1},                             {2,  1},
    {-2,  2}, {-1,  2}, {0,  2}, {1,  2}, {2,  2}
};
static constexpr int kNumOutlineOffsets = sizeof(kOutlineOffsets) / sizeof(kOutlineOffsets[0]);

// ============================================================================
// Singleton Instance
// ============================================================================

OsdOverlay& OsdOverlay::Instance() {
    static OsdOverlay instance;
    return instance;
}

OsdOverlay::OsdOverlay() = default;

OsdOverlay::~OsdOverlay() {
    Shutdown();
}

// ============================================================================
// Configuration Loading
// ============================================================================

bool OsdOverlay::LoadConfig() {
    // Try to load from config system
    try {
        // Load global settings
        if (config::Has("osd.global.font_path")) {
            config_.global.font_path = config::Get<std::string>("osd.global.font_path", 
                std::string("/etc/ipcamera/assets/osd/arial.ttf"));
        }
        if (config::Has("osd.global.logo_path")) {
            config_.global.logo_path = config::Get<std::string>("osd.global.logo_path", 
                std::string("/etc/ipcamera/assets/osd/logo.bin"));
        }
        
        // Load auto-scaling settings
        auto& scaling = config_.global.auto_scaling;
        if (config::Has("osd.global.auto_scaling.enabled")) {
            scaling.enabled = config::Get<bool>("osd.global.auto_scaling.enabled", true);
        }
        if (config::Has("osd.global.auto_scaling.base_font_size")) {
            scaling.base_font_size = config::Get<int>("osd.global.auto_scaling.base_font_size", 48);
        }
        if (config::Has("osd.global.auto_scaling.base_text_font_size")) {
            scaling.base_text_font_size = config::Get<int>("osd.global.auto_scaling.base_text_font_size", 36);
        }
        if (config::Has("osd.global.auto_scaling.min_font_size")) {
            scaling.min_font_size = config::Get<int>("osd.global.auto_scaling.min_font_size", 12);
        }
        if (config::Has("osd.global.auto_scaling.max_font_size")) {
            scaling.max_font_size = config::Get<int>("osd.global.auto_scaling.max_font_size", 72);
        }
        if (config::Has("osd.global.auto_scaling.reference_width")) {
            scaling.reference_width = config::Get<int>("osd.global.auto_scaling.reference_width", 2560);
        }
        if (config::Has("osd.global.auto_scaling.reference_height")) {
            scaling.reference_height = config::Get<int>("osd.global.auto_scaling.reference_height", 1440);
        }
        
        spdlog::info("[OSD] Auto-scaling config: enabled={}, base_font={}, base_text={}, range=[{}-{}]",
                     scaling.enabled, scaling.base_font_size, scaling.base_text_font_size,
                     scaling.min_font_size, scaling.max_font_size);
        
        // Load per-stream settings (streams 0-2)
        for (int s = 0; s < 3; s++) {
            std::string base = "osd.streams.stream" + std::to_string(s);
            OsdStreamConfig& stream = config_.streams[s];
            
            // Stream enable
            if (config::Has(base + ".enabled")) {
                stream.enabled = config::Get<bool>(base + ".enabled", true);
            }
            
            // Stamp buffer size
            if (config::Has(base + ".stamp_width")) {
                stream.stamp_width = config::Get<int>(base + ".stamp_width", 1000 - s * 300);
            }
            if (config::Has(base + ".stamp_height")) {
                stream.stamp_height = config::Get<int>(base + ".stamp_height", 200 - s * 50);
            }
            
            // Stamp offset on video frame
            if (config::Has(base + ".stamp_offset_x")) {
                stream.stamp_offset_x = config::Get<int>(base + ".stamp_offset_x", 0);
            }
            if (config::Has(base + ".stamp_offset_y")) {
                stream.stamp_offset_y = config::Get<int>(base + ".stamp_offset_y", 0);
            }
            
            // Timestamp config
            std::string ts_base = base + ".timestamp";
            if (config::Has(ts_base + ".enabled")) {
                stream.timestamp.enabled = config::Get<bool>(ts_base + ".enabled", true);
            }
            if (config::Has(ts_base + ".position_x")) {
                stream.timestamp.position_x = config::Get<int>(ts_base + ".position_x", 10);
            }
            if (config::Has(ts_base + ".position_y")) {
                stream.timestamp.position_y = config::Get<int>(ts_base + ".position_y", 10);
            }
            if (config::Has(ts_base + ".format")) {
                stream.timestamp.format = config::Get<std::string>(ts_base + ".format", 
                    std::string("%Y-%m-%d %H:%M:%S"));
            }
            if (config::Has(ts_base + ".font_size")) {
                stream.timestamp.font_size = config::Get<int>(ts_base + ".font_size", 48 - s * 12);
            }
            if (config::Has(ts_base + ".outline_enabled")) {
                stream.timestamp.outline_enabled = config::Get<bool>(ts_base + ".outline_enabled", true);
            }
            if (config::Has(ts_base + ".outline_width")) {
                stream.timestamp.outline_width = config::Get<int>(ts_base + ".outline_width", 2);
            }
            
            // Custom text config
            std::string txt_base = base + ".text";
            if (config::Has(txt_base + ".enabled")) {
                stream.text.enabled = config::Get<bool>(txt_base + ".enabled", false);
            }
            if (config::Has(txt_base + ".content")) {
                stream.text.text = config::Get<std::string>(txt_base + ".content", std::string(""));
            }
            if (config::Has(txt_base + ".position_x")) {
                stream.text.position_x = config::Get<int>(txt_base + ".position_x", 10);
            }
            if (config::Has(txt_base + ".position_y")) {
                stream.text.position_y = config::Get<int>(txt_base + ".position_y", 60);
            }
            if (config::Has(txt_base + ".font_size")) {
                stream.text.font_size = config::Get<int>(txt_base + ".font_size", 36 - s * 8);
            }
            
            // Logo config
            std::string logo_base = base + ".logo";
            if (config::Has(logo_base + ".enabled")) {
                stream.logo.enabled = config::Get<bool>(logo_base + ".enabled", s < 2);  // Disable on small streams
            }
            if (config::Has(logo_base + ".position_x")) {
                stream.logo.position_x = config::Get<int>(logo_base + ".position_x", 10);
            }
            if (config::Has(logo_base + ".position_y")) {
                stream.logo.position_y = config::Get<int>(logo_base + ".position_y", 100);
            }
            if (config::Has(logo_base + ".outline_enabled")) {
                stream.logo.outline_enabled = config::Get<bool>(logo_base + ".outline_enabled", true);
            }
            if (config::Has(logo_base + ".outline_width")) {
                stream.logo.outline_width = config::Get<int>(logo_base + ".outline_width", 2);
            }
            
            // Per-stream privacy mask config
            std::string pm_base = base + ".privacy_mask";
            if (config::Has(pm_base + ".enabled")) {
                stream.privacy_mask.enabled = config::Get<bool>(pm_base + ".enabled", false);
            }
            for (int r = 0; r < OsdStreamPrivacyMask::kMaxRegions; r++) {
                std::string region_base = pm_base + ".region" + std::to_string(r);
                if (config::Has(region_base + ".enabled")) {
                    auto& region = stream.privacy_mask.regions[r];
                    region.enabled = config::Get<bool>(region_base + ".enabled", false);
                    region.x1 = config::Get<int>(region_base + ".x1", 0);
                    region.y1 = config::Get<int>(region_base + ".y1", 0);
                    region.x2 = config::Get<int>(region_base + ".x2", 100);
                    region.y2 = config::Get<int>(region_base + ".y2", 100);
                    region.color = config::Get<uint32_t>(region_base + ".color", 0xFF000000);
                }
            }
        }
        
        // Global privacy mask config (legacy - applies to all streams if per-stream not set)
        if (config::Has("osd.privacy_mask.enabled")) {
            config_.privacy_mask.enabled = config::Get<bool>("osd.privacy_mask.enabled", false);
        }
        for (int i = 0; i < OsdPrivacyMaskConfig::kMaxRegions; i++) {
            std::string prefix = "osd.privacy_mask.region" + std::to_string(i);
            if (config::Has(prefix + ".enabled")) {
                config_.privacy_mask.regions[i].enabled = config::Get<bool>(prefix + ".enabled", false);
                config_.privacy_mask.regions[i].x1 = config::Get<int>(prefix + ".x1", 0);
                config_.privacy_mask.regions[i].y1 = config::Get<int>(prefix + ".y1", 0);
                config_.privacy_mask.regions[i].x2 = config::Get<int>(prefix + ".x2", 100);
                config_.privacy_mask.regions[i].y2 = config::Get<int>(prefix + ".y2", 100);
            }
        }
        
    } catch (const std::exception& e) {
        printf("[OSD] Config load exception: %s, using defaults\n", e.what());
    }
    
    config_loaded_ = true;
    printf("[OSD] Config loaded: stream0=%d, stream1=%d, stream2=%d, privacy_mask=%d\n",
           config_.streams[0].enabled, config_.streams[1].enabled, 
           config_.streams[2].enabled, config_.privacy_mask.enabled);
    return true;
}


// ============================================================================
// FreeType Initialization
// ============================================================================

bool OsdOverlay::InitFreetype() {
#if HDAL_PIPELINE_ENABLED
    FT_Library* library = new FT_Library();
    FT_Face* face = new FT_Face();
    
    int error = FT_Init_FreeType(library);
    if (error) {
        printf("[OSD] FT_Init_FreeType failed: %d\n", error);
        delete library;
        delete face;
        return false;
    }
    
    // Use global font path
    error = FT_New_Face(*library, config_.global.font_path.c_str(), 0, face);
    if (error == FT_Err_Unknown_File_Format) {
        printf("[OSD] Font format unsupported: %s\n", config_.global.font_path.c_str());
        FT_Done_FreeType(*library);
        delete library;
        delete face;
        return false;
    } else if (error) {
        printf("[OSD] Failed to open font: %s (err: %d)\n", config_.global.font_path.c_str(), error);
        FT_Done_FreeType(*library);
        delete library;
        delete face;
        return false;
    }
    
    // Set default font size (will be changed per-stream during rendering)
    error = FT_Set_Pixel_Sizes(*face, 0, 48);
    if (error) {
        printf("[OSD] FT_Set_Pixel_Sizes failed\n");
        FT_Done_Face(*face);
        FT_Done_FreeType(*library);
        delete library;
        delete face;
        return false;
    }
    
    ft_library_ = library;
    ft_face_ = face;
    ft_initialized_ = true;
    
    printf("[OSD] FreeType initialized: %s\n", config_.global.font_path.c_str());
    return true;
#else
    printf("[OSD] FreeType disabled (HDAL not enabled)\n");
    return false;
#endif
}

void OsdOverlay::UninitFreetype() {
#if HDAL_PIPELINE_ENABLED
    if (ft_face_) {
        FT_Done_Face(*static_cast<FT_Face*>(ft_face_));
        delete static_cast<FT_Face*>(ft_face_);
        ft_face_ = nullptr;
    }
    if (ft_library_) {
        FT_Done_FreeType(*static_cast<FT_Library*>(ft_library_));
        delete static_cast<FT_Library*>(ft_library_);
        ft_library_ = nullptr;
    }
    ft_initialized_ = false;
#endif
}

// ============================================================================
// Logo Loading
// ============================================================================

bool OsdOverlay::LoadLogo() {
    if (logo_data_) {
        return true;  // Already loaded
    }
    
    FILE* f = fopen(config_.global.logo_path.c_str(), "rb");
    if (!f) {
        printf("[OSD] Failed to open logo: %s\n", config_.global.logo_path.c_str());
        return false;
    }
    
    uint32_t w, h;
    if (fread(&w, 4, 1, f) != 1 || fread(&h, 4, 1, f) != 1) {
        printf("[OSD] Failed to read logo header\n");
        fclose(f);
        return false;
    }
    
    logo_width_ = static_cast<int>(w);
    logo_height_ = static_cast<int>(h);
    logo_data_ = new uint16_t[w * h];
    
    if (fread(logo_data_, 2, w * h, f) != w * h) {
        printf("[OSD] Failed to read logo pixels\n");
        delete[] logo_data_;
        logo_data_ = nullptr;
        fclose(f);
        return false;
    }
    
    fclose(f);
    printf("[OSD] Logo loaded: %dx%d from %s\n", logo_width_, logo_height_, config_.global.logo_path.c_str());
    return true;
}

// ============================================================================
// Buffer Allocation
// ============================================================================

bool OsdOverlay::AllocateBuffers() {
    // Note: Per-stream hardware buffers are allocated in InitStream()
    // This function is kept for potential future use but currently 
    // the shared OSD buffer is allocated directly in Init()
    return true;
}

void OsdOverlay::FreeBuffers() {
    // Free per-stream OSD render buffers
    for (int i = 0; i < kMaxStreams; i++) {
        if (streams_[i].osd_buffer) {
            delete[] streams_[i].osd_buffer;
            streams_[i].osd_buffer = nullptr;
        }
    }
    // Free logo data
    if (logo_data_) {
        delete[] logo_data_;
        logo_data_ = nullptr;
    }
}

// ============================================================================
// Initialization
// ============================================================================

bool OsdOverlay::InitStream(int stream_id) {
#if HDAL_PIPELINE_ENABLED
    if (stream_id < 0 || stream_id >= kMaxStreams) {
        printf("[OSD] Invalid stream_id: %d\n", stream_id);
        return false;
    }
    
    if (streams_[stream_id].enabled) {
        return true;  // Already initialized
    }
    
    const OsdStreamConfig& stream_cfg = config_.streams[stream_id];
    
    // Map stream_id to HDAL encoder input ID
    HD_IN_ID enc_in_ids[] = {
        HD_VIDEOENC_0_IN_0,
        HD_VIDEOENC_0_IN_1,
        HD_VIDEOENC_0_IN_2,
        HD_VIDEOENC_0_IN_3
    };
    
    HD_IN_ID enc_in = enc_in_ids[stream_id];
    
    // Allocate stamp buffer for this stream using stream-specific dimensions
    HD_VIDEO_FRAME frame;
    memset(&frame, 0, sizeof(frame));
    frame.sign = MAKEFOURCC('O','S','G','P');
    frame.dim.w = static_cast<UINT32>(stream_cfg.stamp_width);
    frame.dim.h = static_cast<UINT32>(stream_cfg.stamp_height);
    frame.pxlfmt = HD_VIDEO_PXLFMT_ARGB4444;
    
    int size = hd_common_mem_calc_buf_size(&frame);
    if (size <= 0) {
        printf("[OSD] Stream %d: Failed to query OSG buffer size\n", stream_id);
        return false;
    }
    
    uint32_t stamp_size = ((size * 2 + 127) / 128) * 128;
    
    HD_COMMON_MEM_VB_BLK blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, stamp_size, DDR_ID0);
    if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("[OSD] Stream %d: Failed to allocate OSG memory\n", stream_id);
        return false;
    }
    
    streams_[stream_id].stamp_blk = blk;
    streams_[stream_id].stamp_pa = hd_common_mem_blk2pa(blk);
    streams_[stream_id].stamp_size = stamp_size;
    streams_[stream_id].alloc_stamp_width = stream_cfg.stamp_width;
    streams_[stream_id].alloc_stamp_height = stream_cfg.stamp_height;
    
    // Open stamp path (use unique output ID for each stream's stamp overlay)
    // Use HD_STAMP(n) macro and cast to HD_OUT_ID
    HD_PATH_ID stamp_path_id = 0;
    HD_OUT_ID stamp_out_id = static_cast<HD_OUT_ID>(HD_STAMP(stream_id));
    HD_RESULT ret = hd_videoenc_open(enc_in, stamp_out_id, &stamp_path_id);
    if (ret != HD_OK) {
        printf("[OSD] Stream %d: Failed to open stamp path: %d\n", stream_id, ret);
        hd_common_mem_release_block(blk);
        return false;
    }
    streams_[stream_id].stamp_path = reinterpret_cast<void*>(stamp_path_id);
    
    // Open multiple mask paths for privacy mask support
    // Distribution: Stream0=3, Stream1=3, Stream2=2, Stream3=0 (total 8 masks)
    // Mask indices: Stream0=[0,1,2], Stream1=[3,4,5], Stream2=[6,7]
    static const int mask_counts[kMaxStreams] = {3, 3, 2, 0};
    static const int mask_base[kMaxStreams] = {0, 3, 6, 8};
    
    int num_masks = mask_counts[stream_id];
    int base_idx = mask_base[stream_id];
    streams_[stream_id].num_mask_paths = 0;
    
    for (int i = 0; i < num_masks; i++) {
        HD_PATH_ID mask_path_id = 0;
        HD_OUT_ID mask_out_id = static_cast<HD_OUT_ID>(HD_MASK(base_idx + i));
        ret = hd_videoenc_open(enc_in, mask_out_id, &mask_path_id);
        if (ret == HD_OK) {
            streams_[stream_id].mask_paths[i] = reinterpret_cast<void*>(mask_path_id);
            streams_[stream_id].num_mask_paths++;
            spdlog::info("[OSD] Stream {}: Mask path {} opened (HD_MASK({}))", stream_id, i, base_idx + i);
        } else {
            spdlog::warn("[OSD] Stream {}: Failed to open mask path {} (HD_MASK({})): {}", 
                        stream_id, i, base_idx + i, static_cast<int>(ret));
        }
    }
    spdlog::info("[OSD] Stream {}: {} mask paths available for privacy masks", 
                stream_id, streams_[stream_id].num_mask_paths);
    
    streams_[stream_id].enabled = true;
    printf("[OSD] Stream %d initialized (%dx%d, stamp_pa=0x%lx)\n", 
           stream_id, stream_cfg.stamp_width, stream_cfg.stamp_height, streams_[stream_id].stamp_pa);
    return true;
#else
    (void)stream_id;
    return false;
#endif
}

bool OsdOverlay::Init(void* enc_path) {
#if HDAL_PIPELINE_ENABLED
    if (initialized_) {
        return true;
    }
    
    (void)enc_path;  // Legacy parameter, unused
    
    if (!config_loaded_) {
        LoadConfig();
    }
    
    // Check if any stream has OSD enabled
    bool any_enabled = false;
    bool any_ts_enabled = false;
    bool any_logo_enabled = false;
    for (int i = 0; i < 3; i++) {
        if (config_.streams[i].enabled) {
            any_enabled = true;
            if (config_.streams[i].timestamp.enabled || config_.streams[i].text.enabled) {
                any_ts_enabled = true;
            }
            if (config_.streams[i].logo.enabled) {
                any_logo_enabled = true;
            }
        }
    }
    
    if (!any_enabled) {
        printf("[OSD] All streams disabled in config\n");
        return true;
    }
    
    // Initialize FreeType if any stream needs text rendering
    if (any_ts_enabled) {
        if (!InitFreetype()) {
            printf("[OSD] Warning: FreeType init failed, text rendering disabled\n");
        }
    }
    
    // Load logo if any stream needs it
    if (any_logo_enabled) {
        LoadLogo();
    }
    
    // Initialize OSD on enabled streams (0, 1, 2)
    int streams_init = 0;
    for (int i = 0; i < 3; i++) {
        if (config_.streams[i].enabled && InitStream(i)) {
            streams_init++;
        }
    }
    
    if (streams_init == 0) {
        printf("[OSD] Failed to initialize any streams\n");
        return false;
    }
    
    initialized_ = true;
    printf("[OSD] Initialized successfully (%d streams)\n", streams_init);
    return true;
#else
    (void)enc_path;
    printf("[OSD] HDAL not enabled, OSD disabled\n");
    return false;
#endif
}

// ============================================================================
// Start/Stop
// ============================================================================

bool OsdOverlay::Start() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("[OSD] Start() called: initialized_={}, running_={}", initialized_.load(), running_.load());
    if (!initialized_ || running_) {
        spdlog::warn("[OSD] Start() skipped: initialized_={}, running_={}", initialized_.load(), running_.load());
        return false;
    }
    
    // Initial render
    RenderAllStreams();
    ApplyOsdToEncoder();
    
    // Apply per-stream privacy masks
    for (int i = 0; i < kMaxStreams; i++) {
        if (streams_[i].enabled && config_.streams[i].privacy_mask.enabled) {
            ApplyPrivacyMaskToStream(i);
        }
    }
    
    // Start stamp and mask for all enabled streams
    for (int i = 0; i < kMaxStreams; i++) {
        if (!streams_[i].enabled) continue;
        
        if (streams_[i].stamp_path) {
            HD_RESULT ret = hd_videoenc_start(static_cast<HD_PATH_ID>(reinterpret_cast<uintptr_t>(streams_[i].stamp_path)));
            spdlog::info("[OSD] Stream {}: Stamp path started, result={}", i, static_cast<int>(ret));
        }
        // Start all mask paths for this stream (if any masks are enabled)
        for (int m = 0; m < streams_[i].num_mask_paths; m++) {
            if (streams_[i].mask_paths[m]) {
                // Only start if this region is enabled
                if (config_.streams[i].privacy_mask.enabled && 
                    m < OsdStreamPrivacyMask::kMaxRegions && 
                    config_.streams[i].privacy_mask.regions[m].enabled) {
                    HD_RESULT ret = hd_videoenc_start(static_cast<HD_PATH_ID>(reinterpret_cast<uintptr_t>(streams_[i].mask_paths[m])));
                    spdlog::info("[OSD] Stream {}: Mask path {} started, result={}", i, m, static_cast<int>(ret));
                } else {
                    spdlog::debug("[OSD] Stream {}: Mask path {} not started (disabled)", i, m);
                }
            }
        }
    }
    
    running_ = true;
    
    // Start update thread
    update_thread_ = std::thread(&OsdOverlay::UpdateThread, this);
    
    spdlog::info("[OSD] Started on all streams");
    return true;
#else
    return false;
#endif
}

void OsdOverlay::Stop() {
    running_ = false;
    
    if (update_thread_.joinable()) {
        update_thread_.join();
    }
    
#if HDAL_PIPELINE_ENABLED
    // Stop stamp and mask for all enabled streams
    for (int i = 0; i < kMaxStreams; i++) {
        if (!streams_[i].enabled) continue;
        
        if (streams_[i].stamp_path) {
            hd_videoenc_stop(static_cast<HD_PATH_ID>(reinterpret_cast<uintptr_t>(streams_[i].stamp_path)));
        }
        // Stop all mask paths for this stream
        for (int m = 0; m < streams_[i].num_mask_paths; m++) {
            if (streams_[i].mask_paths[m]) {
                hd_videoenc_stop(static_cast<HD_PATH_ID>(reinterpret_cast<uintptr_t>(streams_[i].mask_paths[m])));
            }
        }
    }
#endif
    
    printf("[OSD] Stopped\n");
}

void OsdOverlay::Shutdown() {
    Stop();
    
#if HDAL_PIPELINE_ENABLED
    // Close and release resources for all streams
    for (int i = 0; i < kMaxStreams; i++) {
        if (!streams_[i].enabled) continue;
        
        if (streams_[i].stamp_path) {
            hd_videoenc_close(static_cast<HD_PATH_ID>(reinterpret_cast<uintptr_t>(streams_[i].stamp_path)));
            streams_[i].stamp_path = nullptr;
        }
        // Close all mask paths for this stream
        for (int m = 0; m < streams_[i].num_mask_paths; m++) {
            if (streams_[i].mask_paths[m]) {
                hd_videoenc_close(static_cast<HD_PATH_ID>(reinterpret_cast<uintptr_t>(streams_[i].mask_paths[m])));
                streams_[i].mask_paths[m] = nullptr;
            }
        }
        streams_[i].num_mask_paths = 0;
        if (streams_[i].stamp_blk) {
            hd_common_mem_release_block(static_cast<HD_COMMON_MEM_VB_BLK>(streams_[i].stamp_blk));
            streams_[i].stamp_blk = 0;
        }
        streams_[i].enabled = false;
    }
#endif
    
    FreeBuffers();
    UninitFreetype();
    initialized_ = false;
    
    printf("[OSD] Shutdown complete\n");
}

// ============================================================================
// Update Thread
// ============================================================================

void OsdOverlay::UpdateThread() {
    while (running_) {
        // Render and apply
        {
            std::lock_guard<std::mutex> lock(config_mutex_);
            RenderAllStreams();
            ApplyOsdToEncoder();
        }
        
        // Sleep for 1 second (timestamp update interval)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// ============================================================================
// Rendering
// ============================================================================

void OsdOverlay::RenderAllStreams() {
    // Render each enabled stream with its own config
    for (int s = 0; s < kMaxStreams; s++) {
        if (streams_[s].enabled) {
            RenderStream(s);
        }
    }
}

void OsdOverlay::RenderStream(int stream_id) {
    if (stream_id < 0 || stream_id >= kMaxStreams) return;
    if (!streams_[stream_id].enabled) return;
    
    const OsdStreamConfig& cfg = config_.streams[stream_id];
    StreamState& stream = streams_[stream_id];
    
    // Use allocated dimensions for buffer operations (these match the hardware buffer)
    int buf_w = stream.alloc_stamp_width;
    int buf_h = stream.alloc_stamp_height;
    
    // Skip if not yet initialized
    if (buf_w <= 0 || buf_h <= 0) return;
    
    // Allocate or reallocate buffer if needed
    if (!stream.osd_buffer) {
        stream.osd_buffer = new uint16_t[buf_w * buf_h];
    }
    
    // Clear buffer
    memset(stream.osd_buffer, 0, buf_w * buf_h * sizeof(uint16_t));
    
    // Render elements using allocated buffer dimensions
    if (cfg.timestamp.enabled && ft_initialized_) {
        RenderTimestamp(stream_id, stream.osd_buffer, buf_w, buf_h);
    }
    
    if (cfg.text.enabled && ft_initialized_ && !cfg.text.text.empty()) {
        RenderCustomText(stream_id, stream.osd_buffer, buf_w, buf_h);
    }
    
    if (cfg.logo.enabled && logo_data_) {
        RenderLogo(stream_id, stream.osd_buffer, buf_w, buf_h);
    }
}

void OsdOverlay::RenderTimestamp(int stream_id, uint16_t* buffer, int buf_w, int buf_h) {
#if HDAL_PIPELINE_ENABLED
    if (!ft_face_ || !buffer) return;
    
    const OsdStreamConfig& cfg = config_.streams[stream_id];
    FT_Face face = *static_cast<FT_Face*>(ft_face_);
    
    // Calculate font size - use auto-scaling if enabled, otherwise use per-stream config
    int font_size = config_.global.auto_scaling.enabled 
        ? CalculateScaledFontSize(stream_id, config_.global.auto_scaling.base_font_size)
        : cfg.timestamp.font_size;
    
    FT_Set_Pixel_Sizes(face, 0, font_size);
    
    // Get current time
    time_t rawtime;
    time(&rawtime);
    struct tm* timeinfo = localtime(&rawtime);
    
    char time_str[64];
    strftime(time_str, sizeof(time_str), cfg.timestamp.format.c_str(), timeinfo);
    
    // Scale positions proportionally when auto-scaling is enabled
    int pos_x = cfg.timestamp.position_x;
    int pos_y = cfg.timestamp.position_y;
    if (config_.global.auto_scaling.enabled) {
        // Scale position based on resolution ratio
        std::string prefix = "media.video" + std::to_string(stream_id + 1);
        int stream_w = config::Get<int>(prefix + ".width", 1920);
        int ref_w = config_.global.auto_scaling.reference_width;
        double scale = static_cast<double>(stream_w) / static_cast<double>(ref_w);
        pos_x = static_cast<int>(pos_x * scale);
        pos_y = static_cast<int>(pos_y * scale);
    }
    
    int pen_x = pos_x;
    int pen_y = pos_y + font_size;
    
    for (const char* p = time_str; *p; p++) {
        if (FT_Load_Char(face, *p, FT_LOAD_RENDER) != 0) continue;
        
        FT_GlyphSlot slot = face->glyph;
        
        DrawCharWithOutline(
            buffer, buf_w, buf_h,
            pen_x + slot->bitmap_left,
            pen_y - slot->bitmap_top,
            &slot->bitmap,
            cfg.timestamp.outline_enabled
        );
        
        pen_x += slot->advance.x >> 6;
    }
#endif
}

void OsdOverlay::RenderCustomText(int stream_id, uint16_t* buffer, int buf_w, int buf_h) {
#if HDAL_PIPELINE_ENABLED
    if (!ft_face_ || !buffer) return;
    
    const OsdStreamConfig& cfg = config_.streams[stream_id];
    FT_Face face = *static_cast<FT_Face*>(ft_face_);
    
    // Calculate font size - use auto-scaling if enabled, otherwise use per-stream config
    int font_size = config_.global.auto_scaling.enabled 
        ? CalculateScaledFontSize(stream_id, config_.global.auto_scaling.base_text_font_size)
        : cfg.text.font_size;
    
    FT_Set_Pixel_Sizes(face, 0, font_size);
    
    // Scale positions proportionally when auto-scaling is enabled
    int pos_x = cfg.text.position_x;
    int pos_y = cfg.text.position_y;
    if (config_.global.auto_scaling.enabled) {
        std::string prefix = "media.video" + std::to_string(stream_id + 1);
        int stream_w = config::Get<int>(prefix + ".width", 1920);
        int ref_w = config_.global.auto_scaling.reference_width;
        double scale = static_cast<double>(stream_w) / static_cast<double>(ref_w);
        pos_x = static_cast<int>(pos_x * scale);
        pos_y = static_cast<int>(pos_y * scale);
    }
    
    int pen_x = pos_x;
    int pen_y = pos_y + font_size;
    
    for (char c : cfg.text.text) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0) continue;
        
        FT_GlyphSlot slot = face->glyph;
        
        DrawCharWithOutline(
            buffer, buf_w, buf_h,
            pen_x + slot->bitmap_left,
            pen_y - slot->bitmap_top,
            &slot->bitmap,
            cfg.text.outline_enabled
        );
        
        pen_x += slot->advance.x >> 6;
    }
#endif
}

void OsdOverlay::RenderLogo(int stream_id, uint16_t* buffer, int buf_w, int buf_h) {
    if (!logo_data_ || !buffer) return;
    
    const OsdStreamConfig& cfg = config_.streams[stream_id];
    
    int start_x = cfg.logo.position_x;
    int start_y = cfg.logo.position_y;
    
    // Draw outline if enabled
    if (cfg.logo.outline_enabled) {
        for (int o = 0; o < kNumOutlineOffsets; o++) {
            int off_x = kOutlineOffsets[o][0];
            int off_y = kOutlineOffsets[o][1];
            
            for (int y = 0; y < logo_height_; y++) {
                for (int x = 0; x < logo_width_; x++) {
                    uint16_t px = logo_data_[y * logo_width_ + x];
                    if ((px & 0xF000) != 0) {  // Has alpha
                        int dst_x = start_x + x + off_x;
                        int dst_y = start_y + y + off_y;
                        
                        if (dst_x >= 0 && dst_x < buf_w &&
                            dst_y >= 0 && dst_y < buf_h) {
                            buffer[dst_y * buf_w + dst_x] = 0xF000;  // Black
                        }
                    }
                }
            }
        }
    }
    
    // Draw logo
    for (int y = 0; y < logo_height_; y++) {
        for (int x = 0; x < logo_width_; x++) {
            int dst_x = start_x + x;
            int dst_y = start_y + y;
            
            if (dst_x >= 0 && dst_x < buf_w &&
                dst_y >= 0 && dst_y < buf_h) {
                uint16_t px = logo_data_[y * logo_width_ + x];
                if ((px & 0xF000) != 0) {
                    buffer[dst_y * buf_w + dst_x] = px;
                }
            }
        }
    }
}

void OsdOverlay::DrawCharColored(uint16_t* buffer, int buf_w, int buf_h,
                                  int pen_x, int pen_y, void* bitmap_ptr, uint16_t color) {
#if HDAL_PIPELINE_ENABLED
    FT_Bitmap* bitmap = static_cast<FT_Bitmap*>(bitmap_ptr);
    
    for (int row = 0; row < static_cast<int>(bitmap->rows); row++) {
        for (int col = 0; col < static_cast<int>(bitmap->width); col++) {
            int p_x = pen_x + col;
            int p_y = pen_y + row;
            
            if (p_x < 0 || p_x >= buf_w || p_y < 0 || p_y >= buf_h) continue;
            
            uint8_t alpha = bitmap->buffer[row * bitmap->pitch + col];
            if (alpha > 32) {
                uint8_t a4 = alpha >> 4;
                if (a4 > 0) {
                    buffer[p_y * buf_w + p_x] = static_cast<uint16_t>((a4 << 12) | (color & 0x0FFF));
                }
            }
        }
    }
#endif
}

void OsdOverlay::DrawCharWithOutline(uint16_t* buffer, int buf_w, int buf_h,
                                      int pen_x, int pen_y, void* bitmap, bool outline_enabled) {
    const uint16_t COLOR_BLACK = 0x0000;
    const uint16_t COLOR_WHITE = 0x0FFF;
    
    // Draw outline
    if (outline_enabled) {
        for (int i = 0; i < kNumOutlineOffsets; i++) {
            DrawCharColored(buffer, buf_w, buf_h,
                           pen_x + kOutlineOffsets[i][0],
                           pen_y + kOutlineOffsets[i][1],
                           bitmap, COLOR_BLACK);
        }
    }
    
    // Draw text
    DrawCharColored(buffer, buf_w, buf_h, pen_x, pen_y, bitmap, COLOR_WHITE);
}

// ============================================================================
// Apply to Encoder
// ============================================================================

bool OsdOverlay::ApplyOsdToEncoder() {
#if HDAL_PIPELINE_ENABLED
    bool success = false;
    
    // Apply to all enabled streams
    for (int s = 0; s < kMaxStreams; s++) {
        if (!streams_[s].enabled || !streams_[s].stamp_path || !streams_[s].stamp_pa) continue;
        if (!streams_[s].osd_buffer) continue;
        
        const StreamState& stream = streams_[s];
        HD_PATH_ID stamp_path_id = static_cast<HD_PATH_ID>(reinterpret_cast<uintptr_t>(stream.stamp_path));
        
        // Set buffer
        HD_OSG_STAMP_BUF buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = HD_OSG_BUF_TYPE_PING_PONG;
        buf.p_addr = static_cast<UINTPTR>(stream.stamp_pa);
        buf.size = stream.stamp_size;
        
        if (hd_videoenc_set(stamp_path_id, HD_VIDEOENC_PARAM_IN_STAMP_BUF, &buf) != HD_OK) {
            printf("[OSD] Stream %d: Failed to set stamp buffer\n", s);
            continue;
        }
        
        // Set image using allocated dimensions (must match hardware buffer)
        HD_OSG_STAMP_IMG img;
        memset(&img, 0, sizeof(img));
        img.fmt = HD_VIDEO_PXLFMT_ARGB4444;
        img.dim.w = static_cast<UINT32>(stream.alloc_stamp_width);
        img.dim.h = static_cast<UINT32>(stream.alloc_stamp_height);
        img.p_addr = reinterpret_cast<uintptr_t>(stream.osd_buffer);
        
        if (hd_videoenc_set(stamp_path_id, HD_VIDEOENC_PARAM_IN_STAMP_IMG, &img) != HD_OK) {
            printf("[OSD] Stream %d: Failed to set stamp image\n", s);
            continue;
        }
        
        // Set attributes - use stamp offset from config to position stamp on video
        const OsdStreamConfig& cfg = config_.streams[s];
        HD_OSG_STAMP_ATTR attr;
        memset(&attr, 0, sizeof(attr));
        attr.position.x = cfg.stamp_offset_x;
        attr.position.y = cfg.stamp_offset_y;
        attr.alpha = 255;
        
        if (hd_videoenc_set(stamp_path_id, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &attr) != HD_OK) {
            printf("[OSD] Stream %d: Failed to set stamp attributes\n", s);
            continue;
        }
        
        success = true;
    }
    
    return success;
#else
    return false;
#endif
}

bool OsdOverlay::ApplyPrivacyMask() {
#if HDAL_PIPELINE_ENABLED
    bool success = true;
    
    // Apply to all enabled streams
    for (int s = 0; s < kMaxStreams; s++) {
        if (!ApplyPrivacyMaskToStream(s)) {
            success = false;
        }
    }
    
    return success;
#else
    return false;
#endif
}

bool OsdOverlay::ApplyPrivacyMaskToStream(int stream_id) {
#if HDAL_PIPELINE_ENABLED
    if (stream_id < 0 || stream_id >= kMaxStreams) {
        // Apply to all streams
        bool success = true;
        for (int s = 0; s < kMaxStreams; s++) {
            if (!ApplyPrivacyMaskToStream(s)) success = false;
        }
        return success;
    }
    
    if (!streams_[stream_id].enabled) {
        spdlog::warn("[OSD] Stream {}: Not enabled, cannot apply privacy mask", stream_id);
        return false;
    }
    
    int num_mask_paths = streams_[stream_id].num_mask_paths;
    if (num_mask_paths == 0) {
        spdlog::warn("[OSD] Stream {}: No mask paths available", stream_id);
        return false;
    }
    
    const auto& stream_mask = config_.streams[stream_id].privacy_mask;
    int masks_applied = 0;
    
    // Apply each enabled region to its corresponding mask path
    for (int i = 0; i < num_mask_paths && i < OsdStreamPrivacyMask::kMaxRegions; i++) {
        if (!streams_[stream_id].mask_paths[i]) continue;
        
        HD_PATH_ID mask_path_id = static_cast<HD_PATH_ID>(
            reinterpret_cast<uintptr_t>(streams_[stream_id].mask_paths[i]));
        
        HD_OSG_MASK_ATTR attr;
        memset(&attr, 0, sizeof(attr));
        
        if (stream_mask.enabled && stream_mask.regions[i].enabled) {
            const auto& region = stream_mask.regions[i];
            // Define quadrilateral (clockwise from top-left)
            attr.position[0].x = region.x1;
            attr.position[0].y = region.y1;
            attr.position[1].x = region.x2;
            attr.position[1].y = region.y1;
            attr.position[2].x = region.x2;
            attr.position[2].y = region.y2;
            attr.position[3].x = region.x1;
            attr.position[3].y = region.y2;
            attr.type = HD_OSG_MASK_TYPE_SOLID;
            attr.alpha = 255;
            attr.color = region.color;
            
            spdlog::info("[OSD] Stream {}: Applying mask region {}: ({},{})-({},{}) color=0x{:08X}",
                        stream_id, i, region.x1, region.y1, region.x2, region.y2, region.color);
        } else {
            // Disable this mask (set to zero-size)
            attr.position[0] = {0, 0};
            attr.position[1] = {0, 0};
            attr.position[2] = {0, 0};
            attr.position[3] = {0, 0};
            attr.type = HD_OSG_MASK_TYPE_SOLID;
            attr.alpha = 0;
            attr.color = 0;
            spdlog::debug("[OSD] Stream {}: Disabling mask region {}", stream_id, i);
        }
        
        HD_RESULT ret = hd_videoenc_set(mask_path_id, HD_VIDEOENC_PARAM_IN_MASK_ATTR, &attr);
        if (ret != HD_OK) {
            spdlog::error("[OSD] Stream {}: Failed to set mask {} attr: {}", stream_id, i, static_cast<int>(ret));
        } else {
            masks_applied++;
            // If running, toggle the mask path to apply changes
            if (running_) {
                if (stream_mask.enabled && stream_mask.regions[i].enabled) {
                    // Stop and restart to ensure the new attributes take effect
                    hd_videoenc_stop(mask_path_id);
                    HD_RESULT start_ret = hd_videoenc_start(mask_path_id);
                    if (start_ret != HD_OK) {
                        spdlog::error("[OSD] Stream {}: Failed to restart mask {}: {}", stream_id, i, static_cast<int>(start_ret));
                    } else {
                        spdlog::info("[OSD] Stream {}: Mask {} restarted to apply changes", stream_id, i);
                    }
                } else {
                    // Just stop the mask if disabled
                    hd_videoenc_stop(mask_path_id);
                    spdlog::info("[OSD] Stream {}: Mask {} stopped (disabled)", stream_id, i);
                }
            }
        }
    }
    
    spdlog::info("[OSD] Stream {}: Privacy mask applied ({}/{} masks, enabled={})", 
                stream_id, masks_applied, num_mask_paths, stream_mask.enabled);
    return masks_applied > 0;
#else
    (void)stream_id;
    return false;
#endif
}

// ============================================================================
// Global Privacy Mask (VIDEOPROC level)
// ============================================================================

bool OsdOverlay::SetGlobalPrivacyMaskRegion(int region_id, bool enabled,
                                             int x1, int y1, int x2, int y2,
                                             uint32_t color) {
#if HDAL_PIPELINE_ENABLED
    if (region_id < 0 || region_id >= OsdPrivacyMaskConfig::kMaxRegions) {
        spdlog::error("[OSD] Invalid global mask region_id: {}", region_id);
        return false;
    }
    
    // Update local config
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        global_privacy_mask_.regions[region_id].enabled = enabled;
        global_privacy_mask_.regions[region_id].x1 = x1;
        global_privacy_mask_.regions[region_id].y1 = y1;
        global_privacy_mask_.regions[region_id].x2 = x2;
        global_privacy_mask_.regions[region_id].y2 = y2;
        global_privacy_mask_.regions[region_id].color = color;
    }
    
    // Apply to HdalPipeline
    auto& pipeline = HdalPipeline::Instance();
    if (!pipeline.IsGlobalPrivacyMaskAvailable()) {
        spdlog::warn("[OSD] Global privacy mask not available in HdalPipeline");
        return false;
    }
    
    bool result = pipeline.SetGlobalPrivacyMask(region_id, enabled, x1, y1, x2, y2, color);
    if (result) {
        spdlog::info("[OSD] Global mask region {} set: enabled={}, ({},{})-({},{}), color=0x{:08X}",
                    region_id, enabled, x1, y1, x2, y2, color);
    } else {
        spdlog::error("[OSD] Failed to set global mask region {}", region_id);
    }
    
    return result;
#else
    (void)region_id; (void)enabled; (void)x1; (void)y1; (void)x2; (void)y2; (void)color;
    return false;
#endif
}

bool OsdOverlay::SetGlobalPrivacyMaskEnabled(bool enabled) {
#if HDAL_PIPELINE_ENABLED
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        global_privacy_mask_.enabled = enabled;
    }
    
    auto& pipeline = HdalPipeline::Instance();
    if (!pipeline.IsGlobalPrivacyMaskAvailable()) {
        spdlog::warn("[OSD] Global privacy mask not available in HdalPipeline");
        return false;
    }
    
    bool result = pipeline.SetGlobalPrivacyMaskEnabled(enabled);
    spdlog::info("[OSD] Global privacy mask {}", enabled ? "enabled" : "disabled");
    return result;
#else
    (void)enabled;
    return false;
#endif
}

bool OsdOverlay::IsGlobalPrivacyMaskAvailable() const {
#if HDAL_PIPELINE_ENABLED
    return HdalPipeline::Instance().IsGlobalPrivacyMaskAvailable();
#else
    return false;
#endif
}

const OsdPrivacyMaskConfig& OsdOverlay::GetGlobalPrivacyMaskConfig() const {
    return global_privacy_mask_;
}

// ============================================================================
// Runtime Configuration
// ============================================================================

bool OsdOverlay::ReinitStreamBuffer(int stream_id, int new_width, int new_height) {
#if HDAL_PIPELINE_ENABLED
    if (stream_id < 0 || stream_id >= kMaxStreams) {
        return false;
    }
    
    StreamState& stream = streams_[stream_id];
    
    // Skip if stream is not enabled or dimensions haven't changed
    if (!stream.enabled) {
        return false;
    }
    
    if (stream.alloc_stamp_width == new_width && stream.alloc_stamp_height == new_height) {
        return true;  // No change needed
    }
    
    printf("[OSD] Stream %d: Reinitializing buffer from %dx%d to %dx%d\n",
           stream_id, stream.alloc_stamp_width, stream.alloc_stamp_height, new_width, new_height);
    
    // Stop the stamp path temporarily
    HD_PATH_ID stamp_path_id = static_cast<HD_PATH_ID>(reinterpret_cast<uintptr_t>(stream.stamp_path));
    if (stamp_path_id) {
        hd_videoenc_stop(stamp_path_id);
    }
    
    // Release old OSD buffer
    if (stream.osd_buffer) {
        delete[] stream.osd_buffer;
        stream.osd_buffer = nullptr;
    }
    
    // Release old stamp block
    if (stream.stamp_blk) {
        hd_common_mem_release_block(static_cast<HD_COMMON_MEM_VB_BLK>(stream.stamp_blk));
        stream.stamp_blk = 0;
        stream.stamp_pa = 0;
        stream.stamp_size = 0;
    }
    
    // Allocate new stamp buffer with new dimensions
    HD_VIDEO_FRAME frame;
    memset(&frame, 0, sizeof(frame));
    frame.sign = MAKEFOURCC('O','S','G','P');
    frame.dim.w = static_cast<UINT32>(new_width);
    frame.dim.h = static_cast<UINT32>(new_height);
    frame.pxlfmt = HD_VIDEO_PXLFMT_ARGB4444;
    
    int size = hd_common_mem_calc_buf_size(&frame);
    if (size <= 0) {
        printf("[OSD] Stream %d: Failed to query OSG buffer size for new dimensions\n", stream_id);
        return false;
    }
    
    uint32_t stamp_size = ((size * 2 + 127) / 128) * 128;
    
    HD_COMMON_MEM_VB_BLK blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, stamp_size, DDR_ID0);
    if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("[OSD] Stream %d: Failed to allocate new OSG memory\n", stream_id);
        return false;
    }
    
    stream.stamp_blk = blk;
    stream.stamp_pa = hd_common_mem_blk2pa(blk);
    stream.stamp_size = stamp_size;
    stream.alloc_stamp_width = new_width;
    stream.alloc_stamp_height = new_height;
    
    // Restart the stamp path if we're running
    if (running_ && stamp_path_id) {
        hd_videoenc_start(stamp_path_id);
    }
    
    printf("[OSD] Stream %d: Buffer reinitialized successfully (%dx%d)\n",
           stream_id, new_width, new_height);
    return true;
#else
    (void)stream_id;
    (void)new_width;
    (void)new_height;
    return false;
#endif
}

void OsdOverlay::SetConfig(const OsdConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // Check if any stream's stamp dimensions changed
    // Note: Runtime resizing of stamp buffers is not supported due to OSG memory pool constraints
    // Stamp dimensions can only be set at initialization via the config file
    for (int s = 0; s < kMaxStreams; s++) {
        if (streams_[s].enabled) {
            int new_width = config.streams[s].stamp_width;
            int new_height = config.streams[s].stamp_height;
            
            if (streams_[s].alloc_stamp_width != new_width || 
                streams_[s].alloc_stamp_height != new_height) {
                printf("[OSD] Warning: Runtime stamp resize from %dx%d to %dx%d not supported. "
                       "Change stamp dimensions in config file and restart.\n",
                       streams_[s].alloc_stamp_width, streams_[s].alloc_stamp_height,
                       new_width, new_height);
            }
        }
    }
    
    // Copy config but preserve the allocated stamp dimensions
    OsdConfig new_config = config;
    for (int s = 0; s < kMaxStreams; s++) {
        if (streams_[s].enabled && streams_[s].alloc_stamp_width > 0) {
            new_config.streams[s].stamp_width = streams_[s].alloc_stamp_width;
            new_config.streams[s].stamp_height = streams_[s].alloc_stamp_height;
        }
    }
    
    config_ = new_config;
}

const OsdStreamConfig& OsdOverlay::GetStreamConfig(int stream_id) const {
    if (stream_id < 0 || stream_id >= kMaxStreams) {
        static OsdStreamConfig default_cfg;
        return default_cfg;
    }
    return config_.streams[stream_id];
}

void OsdOverlay::SetStreamEnabled(int stream_id, bool enabled) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].enabled = enabled;
            config::Set<bool>("osd.streams.stream" + std::to_string(i) + ".enabled", enabled);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].enabled = enabled;
        config::Set<bool>("osd.streams.stream" + std::to_string(stream_id) + ".enabled", enabled);
    }
    config::Save();
}

void OsdOverlay::SetTimestampEnabled(int stream_id, bool enabled) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].timestamp.enabled = enabled;
            config::Set<bool>("osd.streams.stream" + std::to_string(i) + ".timestamp.enabled", enabled);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].timestamp.enabled = enabled;
        config::Set<bool>("osd.streams.stream" + std::to_string(stream_id) + ".timestamp.enabled", enabled);
    }
    config::Save();
}

void OsdOverlay::SetTimestampFormat(int stream_id, const std::string& format) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].timestamp.format = format;
            config::Set<std::string>("osd.streams.stream" + std::to_string(i) + ".timestamp.format", format);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].timestamp.format = format;
        config::Set<std::string>("osd.streams.stream" + std::to_string(stream_id) + ".timestamp.format", format);
    }
    config::Save();
}

void OsdOverlay::SetTimestampPosition(int stream_id, int x, int y) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].timestamp.position_x = x;
            config_.streams[i].timestamp.position_y = y;
            std::string base = "osd.streams.stream" + std::to_string(i) + ".timestamp";
            config::Set<int>(base + ".position_x", x);
            config::Set<int>(base + ".position_y", y);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].timestamp.position_x = x;
        config_.streams[stream_id].timestamp.position_y = y;
        std::string base = "osd.streams.stream" + std::to_string(stream_id) + ".timestamp";
        config::Set<int>(base + ".position_x", x);
        config::Set<int>(base + ".position_y", y);
    }
    config::Save();
}

void OsdOverlay::SetTimestampFontSize(int stream_id, int font_size) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].timestamp.font_size = font_size;
            config::Set<int>("osd.streams.stream" + std::to_string(i) + ".timestamp.font_size", font_size);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].timestamp.font_size = font_size;
        config::Set<int>("osd.streams.stream" + std::to_string(stream_id) + ".timestamp.font_size", font_size);
    }
    config::Save();
}

void OsdOverlay::SetAutoScalingEnabled(bool enabled, int base_size) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_.global.auto_scaling.enabled = enabled;
    if (base_size >= config_.global.auto_scaling.min_font_size && 
        base_size <= config_.global.auto_scaling.max_font_size) {
        config_.global.auto_scaling.base_font_size = base_size;
    }
    
    config::Set<bool>("osd.global.auto_scaling.enabled", enabled);
    config::Set<int>("osd.global.auto_scaling.base_font_size", config_.global.auto_scaling.base_font_size);
    config::Save();
    
    spdlog::info("[OSD] Auto-scaling: {} (base size: {})", 
                 enabled ? "enabled" : "disabled", config_.global.auto_scaling.base_font_size);
}

void OsdOverlay::SetAutoScalingConfig(const OsdAutoScalingConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_.global.auto_scaling = config;
    
    config::Set<bool>("osd.global.auto_scaling.enabled", config.enabled);
    config::Set<int>("osd.global.auto_scaling.base_font_size", config.base_font_size);
    config::Set<int>("osd.global.auto_scaling.base_text_font_size", config.base_text_font_size);
    config::Set<int>("osd.global.auto_scaling.min_font_size", config.min_font_size);
    config::Set<int>("osd.global.auto_scaling.max_font_size", config.max_font_size);
    config::Set<int>("osd.global.auto_scaling.reference_width", config.reference_width);
    config::Set<int>("osd.global.auto_scaling.reference_height", config.reference_height);
    config::Save();
    
    spdlog::info("[OSD] Auto-scaling config updated: enabled={}, base={}, text={}, range=[{}-{}]",
                 config.enabled, config.base_font_size, config.base_text_font_size,
                 config.min_font_size, config.max_font_size);
}

int OsdOverlay::CalculateScaledFontSize(int stream_id, int base_size) const {
    const auto& scaling = config_.global.auto_scaling;
    
    // If auto-scaling disabled, return the stream's configured font size
    if (!scaling.enabled) {
        if (stream_id >= 0 && stream_id < kMaxStreams) {
            return config_.streams[stream_id].timestamp.font_size;
        }
        return base_size > 0 ? base_size : scaling.base_font_size;
    }
    
    // Get stream resolution from config
    int stream_w = 1920, stream_h = 1080;  // Defaults
    if (stream_id >= 0 && stream_id < kMaxStreams) {
        std::string prefix = "media.video" + std::to_string(stream_id + 1);
        stream_w = config::Get<int>(prefix + ".width", 1920);
        stream_h = config::Get<int>(prefix + ".height", 1080);
    }
    
    // Use base_size if provided, otherwise use config default
    int effective_base = (base_size > 0) ? base_size : scaling.base_font_size;
    
    // Calculate diagonal ratio for more accurate scaling
    // Reference diagonal: sqrt(2560^2 + 1440^2) = 2937.2
    // Stream diagonal: sqrt(w^2 + h^2)
    double ref_diagonal = std::sqrt(
        static_cast<double>(scaling.reference_width * scaling.reference_width) +
        static_cast<double>(scaling.reference_height * scaling.reference_height)
    );
    double stream_diagonal = std::sqrt(
        static_cast<double>(stream_w * stream_w) +
        static_cast<double>(stream_h * stream_h)
    );
    
    double scale_factor = stream_diagonal / ref_diagonal;
    int scaled_size = static_cast<int>(effective_base * scale_factor + 0.5);
    
    // Clamp to valid range
    if (scaled_size < scaling.min_font_size) scaled_size = scaling.min_font_size;
    if (scaled_size > scaling.max_font_size) scaled_size = scaling.max_font_size;
    
    return scaled_size;
}

void OsdOverlay::SetCustomText(int stream_id, const std::string& text) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].text.text = text;
            config::Set<std::string>("osd.streams.stream" + std::to_string(i) + ".text.content", text);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].text.text = text;
        config::Set<std::string>("osd.streams.stream" + std::to_string(stream_id) + ".text.content", text);
    }
    config::Save();
}

void OsdOverlay::SetCustomTextEnabled(int stream_id, bool enabled) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].text.enabled = enabled;
            config::Set<bool>("osd.streams.stream" + std::to_string(i) + ".text.enabled", enabled);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].text.enabled = enabled;
        config::Set<bool>("osd.streams.stream" + std::to_string(stream_id) + ".text.enabled", enabled);
    }
    config::Save();
}

void OsdOverlay::SetCustomTextPosition(int stream_id, int x, int y) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].text.position_x = x;
            config_.streams[i].text.position_y = y;
            std::string base = "osd.streams.stream" + std::to_string(i) + ".text";
            config::Set<int>(base + ".position_x", x);
            config::Set<int>(base + ".position_y", y);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].text.position_x = x;
        config_.streams[stream_id].text.position_y = y;
        std::string base = "osd.streams.stream" + std::to_string(stream_id) + ".text";
        config::Set<int>(base + ".position_x", x);
        config::Set<int>(base + ".position_y", y);
    }
    config::Save();
}

void OsdOverlay::SetLogoEnabled(int stream_id, bool enabled) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].logo.enabled = enabled;
            config::Set<bool>("osd.streams.stream" + std::to_string(i) + ".logo.enabled", enabled);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].logo.enabled = enabled;
        config::Set<bool>("osd.streams.stream" + std::to_string(stream_id) + ".logo.enabled", enabled);
    }
    config::Save();
}

void OsdOverlay::SetLogoPosition(int stream_id, int x, int y) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (stream_id == -1) {
        for (int i = 0; i < kMaxStreams; i++) {
            config_.streams[i].logo.position_x = x;
            config_.streams[i].logo.position_y = y;
            std::string base = "osd.streams.stream" + std::to_string(i) + ".logo";
            config::Set<int>(base + ".position_x", x);
            config::Set<int>(base + ".position_y", y);
        }
    } else if (stream_id >= 0 && stream_id < kMaxStreams) {
        config_.streams[stream_id].logo.position_x = x;
        config_.streams[stream_id].logo.position_y = y;
        std::string base = "osd.streams.stream" + std::to_string(stream_id) + ".logo";
        config::Set<int>(base + ".position_x", x);
        config::Set<int>(base + ".position_y", y);
    }
    config::Save();
}

void OsdOverlay::SetPrivacyMaskRegion(int stream_id, int region_id, bool enabled,
                                       int x1, int y1, int x2, int y2, uint32_t color) {
    if (stream_id < 0 || stream_id >= kMaxStreams) return;
    if (region_id < 0 || region_id >= OsdStreamPrivacyMask::kMaxRegions) return;
    
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto& region = config_.streams[stream_id].privacy_mask.regions[region_id];
    region.enabled = enabled;
    region.x1 = x1;
    region.y1 = y1;
    region.x2 = x2;
    region.y2 = y2;
    region.color = color;
    
    // Persist to config
    std::string base = "osd.streams.stream" + std::to_string(stream_id) + 
                       ".privacy_mask.region" + std::to_string(region_id);
    config::Set<bool>(base + ".enabled", enabled);
    config::Set<int>(base + ".x1", x1);
    config::Set<int>(base + ".y1", y1);
    config::Set<int>(base + ".x2", x2);
    config::Set<int>(base + ".y2", y2);
    config::Set<uint32_t>(base + ".color", color);
    config::Save();
}

void OsdOverlay::SetPrivacyMaskEnabled(int stream_id, bool enabled) {
    if (stream_id < 0 || stream_id >= kMaxStreams) return;
    
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_.streams[stream_id].privacy_mask.enabled = enabled;
    
    // Persist to config
    config::Set<bool>("osd.streams.stream" + std::to_string(stream_id) + ".privacy_mask.enabled", enabled);
    config::Save();
}

// ============================================================================
// Convenience Functions
// ============================================================================

bool InitOsdOverlay(void* enc_path) {
    return OsdOverlay::Instance().Init(enc_path);
}

bool StartOsdOverlay() {
    return OsdOverlay::Instance().Start();
}

void StopOsdOverlay() {
    OsdOverlay::Instance().Stop();
}

void ShutdownOsdOverlay() {
    OsdOverlay::Instance().Shutdown();
}

} // namespace platform
} // namespace ipcam
