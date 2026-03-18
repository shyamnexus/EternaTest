/**
 * @file osd_overlay.h
 * @brief OSD Overlay Manager for Novatek IP Camera
 * 
 * This module provides On-Screen Display functionality including:
 * - Dynamic timestamp overlay
 * - Custom text overlay
 * - Logo/image overlay (IMPACT by Honeywell)
 * - Privacy mask support
 * - Smart font with outline for visibility
 * 
 * Based on hd_video_liveview_with_osg sample
 */

#ifndef IPCAM_OSD_OVERLAY_H
#define IPCAM_OSD_OVERLAY_H

#include <string>
#include <vector>
#include <cstdint>
#include <atomic>
#include <thread>
#include <mutex>

namespace ipcam {
namespace platform {

// ============================================================================
// OSD Configuration Structures
// ============================================================================

/**
 * @brief Timestamp OSD configuration (per-stream)
 */
struct OsdTimestampConfig {
    bool enabled = true;
    int position_x = 10;         // X position in pixels
    int position_y = 10;         // Y position in pixels
    std::string format = "%Y-%m-%d %H:%M:%S";  // strftime format
    int font_size = 48;          // Font size in pixels
    bool outline_enabled = true; // Black outline for visibility
    int outline_width = 2;       // Outline thickness in pixels
    uint32_t text_color = 0x0FFF;   // ARGB4444 color (white)
    uint32_t outline_color = 0x0000; // ARGB4444 color (black)
};

/**
 * @brief Custom text OSD configuration (per-stream)
 */
struct OsdTextConfig {
    bool enabled = false;
    int position_x = 10;
    int position_y = 60;         // Below timestamp
    std::string text = "";
    int font_size = 36;
    bool outline_enabled = true;
    int outline_width = 2;
    uint32_t text_color = 0x0FFF;
    uint32_t outline_color = 0x0000;
};

/**
 * @brief Logo OSD configuration (per-stream)
 */
struct OsdLogoConfig {
    bool enabled = true;
    int position_x = 10;
    int position_y = 100;
    bool outline_enabled = true;
    int outline_width = 2;
};

/**
 * @brief Privacy mask region
 */
struct PrivacyMaskRegion {
    bool enabled = false;
    int x1 = 0;
    int y1 = 0;
    int x2 = 100;
    int y2 = 100;
    uint32_t color = 0xFF000000;  // Opaque black (ARGB8888)
};

/**
 * @brief Per-stream privacy mask configuration
 */
struct OsdStreamPrivacyMask {
    bool enabled = false;
    static constexpr int kMaxRegions = 4;
    PrivacyMaskRegion regions[kMaxRegions];
};

/**
 * @brief Global privacy mask configuration (legacy, applies to all streams)
 */
struct OsdPrivacyMaskConfig {
    bool enabled = false;
    static constexpr int kMaxRegions = 4;
    PrivacyMaskRegion regions[kMaxRegions];
};

/**
 * @brief Auto-scaling configuration for OSD
 * 
 * When enabled, font sizes are automatically scaled based on stream resolution.
 * This ensures consistent OSD appearance across all streams without manually
 * configuring each stream's font size.
 */
struct OsdAutoScalingConfig {
    bool enabled = true;              ///< Enable auto-scaling based on resolution
    int base_font_size = 48;          ///< Base font size at reference resolution (2560x1440)
    int base_text_font_size = 36;     ///< Base text font size at reference resolution
    int min_font_size = 12;           ///< Minimum scaled font size
    int max_font_size = 72;           ///< Maximum scaled font size
    int reference_width = 2560;       ///< Reference resolution width
    int reference_height = 1440;      ///< Reference resolution height
};

/**
 * @brief Global OSD settings (shared across all streams)
 */
struct OsdGlobalConfig {
    std::string font_path = "/etc/ipcamera/assets/osd/arial.ttf";
    std::string logo_path = "/etc/ipcamera/assets/osd/logo.bin";
    OsdAutoScalingConfig auto_scaling; ///< Auto-scaling settings for all streams
};

/**
 * @brief Per-stream OSD configuration
 */
struct OsdStreamConfig {
    bool enabled = true;
    int stamp_width = 1000;      // OSD buffer width
    int stamp_height = 200;      // OSD buffer height
    int stamp_offset_x = 0;      // X offset of stamp on video frame
    int stamp_offset_y = 0;      // Y offset of stamp on video frame
    OsdTimestampConfig timestamp;
    OsdTextConfig text;
    OsdLogoConfig logo;
    OsdStreamPrivacyMask privacy_mask;  // Per-stream privacy mask (4 regions)
};

/**
 * @brief Complete OSD configuration (all streams)
 */
struct OsdConfig {
    static constexpr int kMaxStreams = 4;
    OsdGlobalConfig global;
    OsdStreamConfig streams[kMaxStreams];
    OsdPrivacyMaskConfig privacy_mask;
};

// ============================================================================
// OsdOverlay Class
// ============================================================================

/**
 * @brief OSD Overlay Manager
 * 
 * Manages on-screen display elements for video streams:
 * - Initializes FreeType for font rendering
 * - Creates and updates OSD buffers
 * - Integrates with HDAL video encoder for overlay
 * - Supports multiple video streams (up to 4)
 */
class OsdOverlay {
public:
    static constexpr int kMaxStreams = 4;

    /**
     * @brief Get singleton instance
     */
    static OsdOverlay& Instance();

    // Prevent copying
    OsdOverlay(const OsdOverlay&) = delete;
    OsdOverlay& operator=(const OsdOverlay&) = delete;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /**
     * @brief Load OSD configuration from config system
     * @return true on success
     */
    bool LoadConfig();

    /**
     * @brief Initialize OSD for a specific stream
     * @param stream_id Stream index (0-3)
     * @return true on success
     */
    bool InitStream(int stream_id);

    /**
     * @brief Initialize OSD subsystem (legacy - initializes all enabled streams)
     * @param enc_path Unused, kept for API compatibility
     * @return true on success
     */
    bool Init(void* enc_path);

    /**
     * @brief Start OSD updates (spawns update thread)
     * @return true on success
     */
    bool Start();

    /**
     * @brief Stop OSD updates
     */
    void Stop();

    /**
     * @brief Shutdown and release resources
     */
    void Shutdown();

    // ========================================================================
    // Runtime Configuration
    // ========================================================================

    /**
     * @brief Update OSD configuration at runtime
     */
    void SetConfig(const OsdConfig& config);

    /**
     * @brief Get current configuration
     */
    const OsdConfig& GetConfig() const { return config_; }

    /**
     * @brief Get stream-specific configuration
     * @param stream_id Stream index (0-3)
     */
    const OsdStreamConfig& GetStreamConfig(int stream_id) const;

    /**
     * @brief Enable/disable OSD on a specific stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetStreamEnabled(int stream_id, bool enabled);

    /**
     * @brief Enable/disable timestamp on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetTimestampEnabled(int stream_id, bool enabled);

    /**
     * @brief Set timestamp format on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetTimestampFormat(int stream_id, const std::string& format);

    /**
     * @brief Set timestamp position on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetTimestampPosition(int stream_id, int x, int y);

    /**
     * @brief Set timestamp font size on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetTimestampFontSize(int stream_id, int font_size);

    /**
     * @brief Enable/disable auto-scaling of fonts based on stream resolution
     * @param enabled true to enable auto-scaling
     * @param base_size Base font size at reference resolution (default 48)
     */
    void SetAutoScalingEnabled(bool enabled, int base_size = 48);

    /**
     * @brief Configure auto-scaling parameters
     * @param config Auto-scaling configuration
     */
    void SetAutoScalingConfig(const OsdAutoScalingConfig& config);

    /**
     * @brief Get auto-scaling configuration
     */
    const OsdAutoScalingConfig& GetAutoScalingConfig() const { return config_.global.auto_scaling; }

    /**
     * @brief Calculate scaled font size for a specific stream
     * 
     * When auto-scaling is enabled, this calculates the appropriate font size
     * based on the stream's resolution compared to the reference resolution.
     * Uses diagonal ratio for more accurate scaling.
     * 
     * @param stream_id Stream index (0-3)
     * @param base_size Base font size (use 0 for config default)
     * @return Scaled font size clamped to min/max bounds
     */
    int CalculateScaledFontSize(int stream_id, int base_size = 0) const;

    /**
     * @brief Set custom text on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetCustomText(int stream_id, const std::string& text);

    /**
     * @brief Enable/disable custom text on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetCustomTextEnabled(int stream_id, bool enabled);

    /**
     * @brief Set custom text position on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetCustomTextPosition(int stream_id, int x, int y);

    /**
     * @brief Enable/disable logo on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetLogoEnabled(int stream_id, bool enabled);

    /**
     * @brief Set logo position on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetLogoPosition(int stream_id, int x, int y);

    /**
     * @brief Configure privacy mask region for a specific stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     * @param region_id Region index (0-3)
     * @param enabled Enable/disable region
     * @param x1, y1, x2, y2 Region coordinates
     * @param color Region color (ARGB8888)
     */
    void SetPrivacyMaskRegion(int stream_id, int region_id, bool enabled, 
                               int x1, int y1, int x2, int y2, uint32_t color = 0xFF000000);

    /**
     * @brief Enable/disable privacy mask on a stream
     * @param stream_id Stream index (0-3), or -1 for all streams
     */
    void SetPrivacyMaskEnabled(int stream_id, bool enabled);

    /**
     * @brief Apply privacy mask changes to encoder
     * @param stream_id Stream index (0-3), or -1 for all streams
     * @return true on success
     */
    bool ApplyPrivacyMaskToStream(int stream_id);

    // ========================================================================
    // Global Privacy Mask (VIDEOPROC level - applies to all streams)
    // ========================================================================

    /**
     * @brief Configure global privacy mask region (VIDEOPROC level)
     * 
     * Global masks are applied at the VIDEOPROC level, meaning they apply
     * to all streams with automatic scaling. This is more efficient than
     * per-stream masks and ensures consistent masking across all resolutions.
     * 
     * @param region_id Region index (0-3)
     * @param enabled Enable/disable region
     * @param x1, y1 Top-left corner coordinates (in main stream resolution)
     * @param x2, y2 Bottom-right corner coordinates
     * @param color Region color (ARGB8888, default opaque black)
     * @return true on success
     */
    bool SetGlobalPrivacyMaskRegion(int region_id, bool enabled,
                                     int x1, int y1, int x2, int y2,
                                     uint32_t color = 0xFF000000);

    /**
     * @brief Enable/disable all global privacy masks
     * @param enabled true to enable, false to disable
     * @return true on success
     */
    bool SetGlobalPrivacyMaskEnabled(bool enabled);

    /**
     * @brief Check if global privacy mask is available
     * @return true if VIDEOPROC mask paths are open
     */
    bool IsGlobalPrivacyMaskAvailable() const;

    /**
     * @brief Get global privacy mask configuration
     * @return Reference to global mask config
     */
    const OsdPrivacyMaskConfig& GetGlobalPrivacyMaskConfig() const;

    // ========================================================================
    // Status
    // ========================================================================

    /**
     * @brief Check if OSD is initialized
     */
    bool IsInitialized() const { return initialized_; }

    /**
     * @brief Check if OSD is running
     */
    bool IsRunning() const { return running_; }

private:
    OsdOverlay();
    ~OsdOverlay();

    // ========================================================================
    // Internal Methods
    // ========================================================================

    bool InitFreetype();
    void UninitFreetype();
    bool LoadLogo();
    bool AllocateBuffers();
    void FreeBuffers();
    void UpdateThread();
    void RenderAllStreams();
    void RenderStream(int stream_id);
    void RenderTimestamp(int stream_id, uint16_t* buffer, int buf_w, int buf_h);
    void RenderCustomText(int stream_id, uint16_t* buffer, int buf_w, int buf_h);
    void RenderLogo(int stream_id, uint16_t* buffer, int buf_w, int buf_h);
    void DrawCharColored(uint16_t* buffer, int buf_w, int buf_h,
                         int pen_x, int pen_y, void* bitmap, uint16_t color);
    void DrawCharWithOutline(uint16_t* buffer, int buf_w, int buf_h,
                             int pen_x, int pen_y, void* bitmap, bool outline_enabled);
    bool ApplyOsdToEncoder();
    bool ApplyPrivacyMask();

    // ========================================================================
    // State
    // ========================================================================

    OsdConfig config_;
    OsdPrivacyMaskConfig global_privacy_mask_;  // Global VIDEOPROC-level mask config
    bool config_loaded_ = false;
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};

    // Update thread
    std::thread update_thread_;
    std::mutex config_mutex_;

    // FreeType (opaque pointers to avoid header dependency)
    void* ft_library_ = nullptr;
    void* ft_face_ = nullptr;
    bool ft_initialized_ = false;

    // Logo data (shared across all streams)
    uint16_t* logo_data_ = nullptr;
    int logo_width_ = 0;
    int logo_height_ = 0;

    // Per-stream HDAL handles and OSD buffer
    // Mask distribution: Stream0=3, Stream1=3, Stream2=2 (total 8 masks)
    static constexpr int kMaxMasksPerStream = 3;
    struct StreamState {
        bool enabled = false;
        uint16_t* osd_buffer = nullptr;      // Per-stream rendered OSD buffer
        void* stamp_path = nullptr;
        void* mask_paths[kMaxMasksPerStream] = {nullptr, nullptr, nullptr};  // Multiple masks per stream
        int num_mask_paths = 0;              // Actual number of mask paths opened
        uint64_t stamp_blk = 0;
        uintptr_t stamp_pa = 0;
        uint32_t stamp_size = 0;
        int alloc_stamp_width = 0;           // Allocated stamp buffer width
        int alloc_stamp_height = 0;          // Allocated stamp buffer height
    };
    StreamState streams_[kMaxStreams];
    
    // Internal method to reinitialize stream buffer when dimensions change
    bool ReinitStreamBuffer(int stream_id, int new_width, int new_height);
};

// ============================================================================
// Convenience Functions
// ============================================================================

/**
 * @brief Initialize OSD with default configuration
 * @param enc_path Encoder path for OSD attachment
 * @return true on success
 */
bool InitOsdOverlay(void* enc_path);

/**
 * @brief Start OSD updates
 */
bool StartOsdOverlay();

/**
 * @brief Stop OSD updates
 */
void StopOsdOverlay();

/**
 * @brief Shutdown OSD subsystem
 */
void ShutdownOsdOverlay();

} // namespace platform
} // namespace ipcam

#endif // IPCAM_OSD_OVERLAY_H
