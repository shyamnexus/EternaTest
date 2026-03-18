/**
 * @file hdal_pipeline.h
 * @brief HDAL Pipeline Manager for Novatek IP Camera
 * 
 * This module manages the complete HDAL video/audio pipeline including:
 * - Sensor configuration and video capture
 * - Video processing (ISP, 3DNR, WDR, Defog)
 * - Multi-stream video encoding (up to 4 streams)
 * - Audio capture and encoding
 * - ISP tuning file loading
 * 
 * Based on features from:
 * - hd_video_record_with_2path.c (multi-stream encoding)
 * - pq_video_rtsp.c (ISP tuning, memory pools, video quality)
 */

#ifndef IPCAM_HDAL_PIPELINE_H
#define IPCAM_HDAL_PIPELINE_H

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>
#include "ipcam/g711_codec.h"

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
#include "hd_debug.h"
#include "vendor_isp.h"
#include "vendor_videocapture.h"
#include "vendor_videoprocess.h"
}
#endif

namespace ipcam {
namespace platform {

// ============================================================================
// Constants
// ============================================================================
constexpr int kMaxVideoStreams = 4;
constexpr int kMaxAudioStreams = 1;

// ============================================================================
// Configuration Structures (loaded from media.json)
// ============================================================================

/**
 * @brief Sensor crop configuration
 */
struct SensorCropConfig {
    bool enabled = true;
    int x = 8;
    int y = 0;
    int width = 2944;
    int height = 1664;
};

/**
 * @brief Sensor configuration from media.sensor
 */
struct SensorConfig {
    std::string driver_name = "nvt_sen_gc5603";
    std::string interface = "mipi_csi";
    int pinmux = 0;
    int data_lanes = 2;
    int vcap_id = 0;
    std::string raw_format = "raw10";
    int native_width = 2960;
    int native_height = 1664;
    SensorCropConfig crop;
};

/**
 * @brief ISP tuning configuration from media.isp_tuning
 */
struct IspTuningConfig {
    std::string config_path = "/mnt/app/isp";
    std::string config_name = "isp_gc5603_0";
    int color_temperature = 4700;
    bool hdr_enabled = false;
};

/**
 * @brief Video processing configuration from media.videoproc
 */
struct VideoProcConfig {
    std::string pipe = "rawall";
    bool ae_enabled = true;
    bool awb_enabled = true;
    bool af_enabled = true;
    bool wdr_enabled = true;
    bool defog_enabled = true;
    bool nr_3d_enabled = true;
    int nr_3d_ref_path = 4;
};

/**
 * @brief Memory pool configuration from media.memory
 */
struct MemoryConfig {
    int ddr_id = 0;
    int raw_buffer_count = 9;
    int yuv_buffer_count = 6;
    int audio_buffer_size = 16384;
};

/**
 * @brief Video stream encoder configuration
 */
struct VideoEncoderConfig {
    std::string codec = "h264";      // h264, h265, jpeg
    std::string profile = "main";    // baseline, main, high
    std::string level = "4.1";
    int bitrate = 2048;              // kbps
    int max_bitrate = 4096;
    int min_bitrate = 512;
    int gop = 30;
    int idr_interval = 1;
    std::string rc_mode = "CBR";     // CBR, VBR
    int init_qp = 26;
    int min_qp = 10;
    int max_qp = 45;
    std::string entropy_mode = "cabac";  // cabac, cavlc
    int buffer_size = 3000;          // ms
    int buffer_count = 3;
};

/**
 * @brief Video stream configuration from media.video1-4
 */
struct VideoStreamConfig {
    int id = 0;
    bool enabled = true;
    std::string stream_type = "mainStream";
    int width = 1920;
    int height = 1080;
    int fps = 30;
    VideoEncoderConfig encoder;
};

/**
 * @brief Audio stream configuration from media.audio
 */
struct AudioStreamConfig {
    bool enabled = true;
    std::string codec = "pcm";       // pcm, aac, ulaw, alaw
    int sample_rate = 48000;
    int bitrate = 64000;
    int channels = 2;
    int bit_width = 16;
    std::string mode = "stereo";
    int frame_samples = 1024;
    bool aec_enabled = false;
    bool anr_enabled = false;
    int anr_level = 3;
    bool agc_enabled = false;
    int input_gain = 80;
    int output_gain = 80;
};

/**
 * @brief Complete HDAL pipeline configuration
 */
struct HdalPipelineConfig {
    SensorConfig sensor;
    IspTuningConfig isp_tuning;
    VideoProcConfig videoproc;
    MemoryConfig memory;
    VideoStreamConfig video_streams[kMaxVideoStreams];
    AudioStreamConfig audio;
};

// ============================================================================
// Pipeline State
// ============================================================================

enum class PipelineState {
    kUninitialized,
    kInitialized,
    kRunning,
    kStopped,
    kError
};

// ============================================================================
// Stream Handle (internal use)
// ============================================================================

#if HDAL_PIPELINE_ENABLED
struct VideoStreamHandle {
    int id = -1;
    bool enabled = false;
    bool encoder_running = false;    ///< Track if encoder is started (for safe stop/start)
    uint32_t path_config_bitrate = 0; ///< Current PATH_CONFIG max bitrate (bps) for dynamic updates
    HD_DIM path_config_max_dim = {0, 0}; ///< PATH_CONFIG max dimension for safe runtime updates
    HD_PATH_ID cap_ctrl = 0;
    HD_PATH_ID cap_path = 0;
    HD_PATH_ID proc_ctrl = 0;
    HD_PATH_ID proc_path = 0;
    HD_PATH_ID enc_path = 0;
    HD_DIM cap_dim = {0, 0};
    HD_DIM proc_dim = {0, 0};
    HD_DIM enc_dim = {0, 0};
};

struct AudioStreamHandle {
    bool enabled = false;
    HD_PATH_ID cap_ctrl = 0;
    HD_PATH_ID cap_path = 0;
    HD_PATH_ID enc_path = 0;
    
    // G.711 userspace encoding (since kernel only supports PCM)
    G711Type g711_type = G711Type::kNone;  ///< G.711 codec type (kNone = use kernel encoder)
    std::string configured_codec;          ///< Original codec from config
};
#endif

// ============================================================================
// HdalPipeline Class
// ============================================================================

/**
 * @brief HDAL Pipeline Manager Singleton
 * 
 * Manages the complete video/audio pipeline lifecycle:
 * 1. LoadConfig() - Load configuration from media.json
 * 2. Init() - Initialize HDAL modules and memory
 * 3. Start() - Start video/audio capture and encoding
 * 4. Stop() - Stop all modules
 * 5. Shutdown() - Release all resources
 * 
 * After pipeline is running, use HdalWrapper for ISP tuning.
 */
class HdalPipeline {
public:
    /**
     * @brief Get singleton instance
     */
    static HdalPipeline& Instance();

    // Prevent copying
    HdalPipeline(const HdalPipeline&) = delete;
    HdalPipeline& operator=(const HdalPipeline&) = delete;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /**
     * @brief Load pipeline configuration from media.json
     * @return true on success
     */
    bool LoadConfig();

    /**
     * @brief Initialize HDAL pipeline (memory, modules, paths)
     * @return true on success
     * 
     * Sequence:
     * 1. vendor_isp_init()
     * 2. Load ISP tuning config files
     * 3. hd_common_init()
     * 4. hd_common_mem_init()
     * 5. Initialize all HDAL modules
     * 6. Open and configure all paths
     * 7. Bind paths together
     */
    bool Init();

    /**
     * @brief Start the pipeline (begin capture and encoding)
     * @return true on success
     */
    bool Start();

    /**
     * @brief Stop the pipeline (stop capture and encoding)
     * @return true on success
     */
    bool Stop();

    /**
     * @brief Shutdown and release all resources
     */
    void Shutdown();

    // ========================================================================
    // State Query
    // ========================================================================

    /**
     * @brief Get current pipeline state
     */
    PipelineState GetState() const { return state_; }

    /**
     * @brief Check if pipeline is running
     */
    bool IsRunning() const { return state_ == PipelineState::kRunning; }

    /**
     * @brief Check if pipeline is initialized
     */
    bool IsInitialized() const { 
        return state_ == PipelineState::kInitialized || 
               state_ == PipelineState::kRunning ||
               state_ == PipelineState::kStopped; 
    }

    /**
     * @brief Reinitialize the pipeline with new configuration
     * @return true on success
     */
    bool ReinitWithConfig();

    /**
     * @brief Get current configuration
     */
    const HdalPipelineConfig& GetConfig() const { return config_; }

    // ========================================================================
    // Audio G.711 Support
    // ========================================================================

    /**
     * @brief Get the G.711 encoding type for audio
     * @return G711Type (kNone if PCM passthrough, kUlaw or kAlaw if G.711)
     * 
     * When this returns non-kNone, the audio data from HDAL is PCM and needs
     * to be encoded to G.711 in userspace before sending via RTSP or recording.
     */
    G711Type GetAudioG711Type() const;

    /**
     * @brief Check if audio requires userspace G.711 encoding
     * @return true if audio is configured for G.711 and needs userspace encoding
     */
    bool RequiresG711Encoding() const;

    /**
     * @brief Get the configured audio codec string
     * @return Original codec string from config (e.g., "pcmu", "g711a", "pcm")
     */
    const std::string& GetAudioCodecString() const;

    /**
     * @brief Get the audio capture path for pulling PCM data
     * @return Audio capture path ID, 0 if audio not enabled
     * 
     * Used by RTSP and recording modules to pull audio frames from
     * the audio capture pipeline.
     */
    uint64_t GetAudioCapturePath() const;

    /**
     * @brief Get the audio capture control path for buffer queries
     * @return Audio capture control path ID, 0 if audio not enabled
     * 
     * Used for querying audio buffer info with HD_AUDIOCAP_PARAM_BUFINFO.
     */
    uint64_t GetAudioCaptureCtrlPath() const;

    /**
     * @brief Get the audio encoder path for pulling encoded audio
     * @return Audio encoder path ID, 0 if audio not enabled
     * 
     * Used by RTSP to pull PCM audio frames from the audio encoder.
     * Audio capture -> Audio encoder pipeline.
     */
    uint64_t GetAudioEncoderPath() const;

    /**
     * @brief Check if audio is enabled in the pipeline
     * @return true if audio capture is enabled and initialized
     */
    bool IsAudioEnabled() const;

    /**
     * @brief Get audio sample rate
     * @return Sample rate in Hz (e.g., 8000, 16000, 48000)
     */
    int GetAudioSampleRate() const;

    /**
     * @brief Get audio channel count
     * @return 1 for mono, 2 for stereo
     */
    int GetAudioChannels() const;

    // ========================================================================
    // Stream Control (runtime)
    // ========================================================================

    /**
     * @brief Enable/disable a video stream at runtime
     * @param stream_id Stream index (0-3)
     * @param enabled Enable or disable
     * @return true on success
     */
    bool SetStreamEnabled(int stream_id, bool enabled);

    /**
     * @brief Update encoder bitrate at runtime
     * @param stream_id Stream index (0-3)
     * @param bitrate_kbps New bitrate in kbps
     * @return true on success
     */
    bool SetStreamBitrate(int stream_id, int bitrate_kbps);

    /**
     * @brief Update encoder framerate at runtime
     * @param stream_id Stream index (0-3)
     * @param fps New framerate
     * @return true on success
     */
    bool SetStreamFps(int stream_id, int fps);

    /**
     * @brief Update encoder GOP at runtime
     * @param stream_id Stream index (0-3)
     * @param gop New GOP size
     * @return true on success
     */
    bool SetStreamGop(int stream_id, int gop);

    /**
     * @brief Update encoder QP range at runtime
     * @param stream_id Stream index (0-3)
     * @param min_qp Minimum QP
     * @param max_qp Maximum QP
     * @return true on success
     */
    bool SetStreamQpRange(int stream_id, int min_qp, int max_qp);

    /**
     * @brief Force IDR frame on a stream
     * @param stream_id Stream index (0-3)
     * @return true on success
     */
    bool ForceIdr(int stream_id);

    /**
     * @brief Change video codec at runtime without stopping the encoder
     * 
     * This uses HDAL dynamic APIs (HD_VIDEOENC_PARAM_OUT_ENC_PARAM2) to change
     * the codec type while the encoder is running. Supports switching between
     * H.264, H.265, and MJPEG codecs.
     * 
     * @param stream_id Stream index (0-3)
     * @param codec New codec type: "h264", "h265", "hevc", "jpeg", "mjpeg"
     * @param profile Optional profile (default uses codec default)
     * @return true on success
     * 
     * @note This requires the encoder path to be configured with sufficient
     *       resources for the target codec (H.264 requires more memory than H.265)
     */
    bool SetStreamCodec(int stream_id, const std::string& codec, const std::string& profile = "");

    /**
     * @brief Get current codec for a stream
     * @param stream_id Stream index (0-3)
     * @return Codec string: "h264", "h265", or "mjpeg" (empty string if invalid stream)
     */
    std::string GetStreamCodec(int stream_id) const;

    /**
     * @brief Change video resolution at runtime
     * 
     * @param stream_id Stream index (0-3)
     * @param width New width
     * @param height New height
     * @return true on success
        */
        bool SetStreamResolution(int stream_id, int width, int height);

    /**
     * @brief Flush encoder buffers (useful before/after codec change)
     * @param stream_id Stream index (0-3)
     */
    void FlushEncoderBuffers(int stream_id);

    // ========================================================================
    // Snapshot Capture
    // ========================================================================

    /**
     * @brief Capture a JPEG snapshot from the encoder source output
     * 
     * Triggers an on-demand JPEG snapshot using the encoder's source_output path.
     * The snapshot includes any OSG overlays (timestamp, logo) that are attached
     * to the encoder. Requires source_output to be enabled in ConfigureEncoders().
     * 
     * @param stream_id Stream index (0-3), typically 0 for main stream
     * @param quality JPEG quality (1-100)
     * @param buffer Output buffer that will be filled with JPEG data
     * @return true on success, false on failure
     */
    bool CaptureSnapshot(int stream_id, int quality, std::vector<uint8_t>& buffer);

    // ========================================================================
    // Statistics
    // ========================================================================

    struct PipelineStats {
        bool running;
        uint64_t video_frame_count[kMaxVideoStreams];
        uint64_t video_byte_count[kMaxVideoStreams];
        uint64_t audio_frame_count;
        uint64_t audio_byte_count;
    };

    PipelineStats GetStats();
    int GetCurrentBitrate(int stream_id);
    int GetCurrentFps(int stream_id);

    // ========================================================================
    // AI Analytics Integration
    // ========================================================================

    /**
     * @brief Check if analytics path is available
     * @return true if VideoProc analytics output path is open
     */
    bool IsAnalyticsPathAvailable() const;

    /**
     * @brief Get the VideoProc path ID for AI analytics
     * @return Path ID for analytics YUV output, 0 if not available
     * 
     * This returns the path ID for VideoProc OUT that provides
     * scaled YUV frames for AI processing. The analytics engine
     * should use this to pull frames for inference.
     */
    uint64_t GetAnalyticsPath() const;

    /**
     * @brief Get the VideoProc path for a specific stream
     * @param stream_id Stream index (0-3)
     * @return Path ID for the stream's proc path
     */
    uint64_t GetVideoProcPath(int stream_id) const;

    /**
     * @brief Get the video encoder path for a specific stream
     * @param stream_id Stream index (0-3)
     * @return Path ID for the stream's encoder path, 0 if invalid
     * 
     * This is used by the recording module to pull encoded video
     * frames from the encoder for MP4 muxing.
     */
    uint64_t GetVideoEncoderPath(int stream_id) const;

    /**
     * @brief Check if the encoder for a stream is currently running
     * @param stream_id Stream index (0-3)
     * @return true if encoder is started and running
     */
    bool IsEncoderRunning(int stream_id) const;

    /**
     * @brief Ensure encoder is running for a stream
     * 
     * If the encoder was stopped due to configuration changes or errors,
     * this method will attempt to restart it. Used by RTSP and recording
     * modules to recover from encoder failures.
     * 
     * @param stream_id Stream index (0-3)
     * @return true if encoder is now running (was running or successfully restarted)
     */
    bool EnsureEncoderRunning(int stream_id);

    /**
     * @brief Get analytics frame dimensions
     * @param width Output width
     * @param height Output height
     * @return true if analytics is configured
     */
    bool GetAnalyticsDimensions(int& width, int& height) const;

    // ========================================================================
    // Motion Detection Integration
    // ========================================================================

    /**
     * @brief Check if MD path is available
     * @return true if VideoProc MD output path is open and running
     */
    bool IsMdPathAvailable() const;

    /**
     * @brief Get the VideoProc path ID for motion detection
     * @return Path ID for MD YUV output (160x120), 0 if not available
     * 
     * This returns the path ID for VideoProc OUT that provides
     * scaled YUV frames (160x120) for libmd processing.
     */
    uint64_t GetMdPath() const;

    /**
     * @brief Get MD frame dimensions
     * @param width Output width (typically 160)
     * @param height Output height (typically 120)
     * @return true if MD path is configured
     */
    bool GetMdDimensions(int& width, int& height) const;

    // ========================================================================
    // VQA Tamper Detection Integration
    // ========================================================================

    /**
     * @brief Check if VQA tamper detection path is available
     * @return true if VideoProc VQA output path is open and running
     */
    bool IsVqaPathAvailable() const;

    /**
     * @brief Get the VideoProc path ID for VQA tamper detection
     * @return Path ID for VQA YUV output (320x180), 0 if not available
     * 
     * This returns the path ID for VideoProc OUT that provides
     * scaled YUV frames (320x180) for IVE-accelerated tamper detection.
     * Uses histogram and Sobel edge detection via IVE hardware.
     */
    uint64_t GetVqaPath() const;

    /**
     * @brief Get VQA frame dimensions
     * @param width Output width (320)
     * @param height Output height (180)
     * @return true if VQA path is configured
     */
    bool GetVqaDimensions(int& width, int& height) const;

    // ========================================================================
    // RTSP Integration
    // ========================================================================

    /**
     * @brief Start the RTSP server (nvtrtspd_ipc)
     * @return true on success
     */
    bool StartRtspServer();

    /**
     * @brief Stop the RTSP server
     */
    void StopRtspServer();

    // ========================================================================
    // Global Privacy Mask (VIDEOPROC Level)
    // ========================================================================

    /**
     * @brief Maximum number of global privacy mask regions
     */
    static constexpr int kMaxGlobalMasks = 4;

    /**
     * @brief Global privacy mask region
     */
    struct GlobalMaskRegion {
        bool enabled = false;
        int x1 = 0;
        int y1 = 0;
        int x2 = 100;
        int y2 = 100;
        uint32_t color = 0xFF000000;  // ARGB: opaque black
    };

    /**
     * @brief Set a global privacy mask region (VIDEOPROC level)
     * 
     * Global masks are applied at the video processor level, BEFORE encoding.
     * This means the mask appears on ALL video streams automatically,
     * regardless of resolution. Coordinates are in sensor resolution.
     * 
     * @param region_id Region index (0-3)
     * @param enabled Enable/disable region
     * @param x1, y1 Top-left corner in sensor coordinates
     * @param x2, y2 Bottom-right corner in sensor coordinates
     * @param color ARGB color (default: opaque black)
     * @return true on success
     */
    bool SetGlobalPrivacyMask(int region_id, bool enabled,
                               int x1, int y1, int x2, int y2,
                               uint32_t color = 0xFF000000);

    /**
     * @brief Enable/disable all global privacy masks
     * @param enabled true to enable all configured masks
     * @return true on success
     */
    bool SetGlobalPrivacyMaskEnabled(bool enabled);

    /**
     * @brief Check if global privacy mask is available
     * @return true if VIDEOPROC mask path is open
     */
    bool IsGlobalPrivacyMaskAvailable() const;

    /**
     * @brief Get global mask path ID (for advanced use)
     * @param region_id Region index (0-3)
     * @return Path ID, 0 if not available
     */
    uint64_t GetGlobalMaskPath(int region_id) const;

private:
    HdalPipeline();
    ~HdalPipeline();

    // ========================================================================
    // Internal Initialization Methods
    // ========================================================================

    bool InitVendorIsp();
    bool LoadIspTuningConfig();
    bool InitHdalCommon();
    bool InitMemoryPools();
    bool InitModules();
    bool OpenVideoPaths();
    bool OpenAudioPaths();
    bool OpenAnalyticsPath();        // AI analytics YUV output path
    bool OpenMdPath();               // Motion detection YUV output path (160x120)
    bool OpenVqaPath();              // VQA tamper detection YUV output path (320x180)
    bool ConfigureCapture();
    bool ConfigureVideoProc();
    bool ConfigureAnalyticsProc();   // Configure analytics VideoProc output
    bool ConfigureMdProc();          // Configure MD VideoProc output
    bool ConfigureVqaProc();         // Configure VQA VideoProc output (320x180)
    bool ConfigureEncoders();
    bool ConfigureAudio();           // Legacy - calls both capture and encoder
    bool ConfigureAudioCapture();    // Audio capture config (before binding)
    bool ConfigureAudioEncoder();    // Audio encoder config (AFTER binding)
    bool BindPaths();
    bool OpenGlobalMaskPaths();      // Open VIDEOPROC-level mask paths
    void CloseGlobalMaskPaths();     // Close VIDEOPROC-level mask paths

    // ========================================================================
    // Internal Cleanup Methods
    // ========================================================================

    void UnbindPaths();
    void ClosePaths();
    void UninitModules();
    void UninitMemory();
    void UninitHdalCommon();
    void UninitVendorIsp();

    // ========================================================================
    // Helper Methods
    // ========================================================================

    int CalculateRawBufferSize(int width, int height, const std::string& format);
    int CalculateYuvBufferSize(int width, int height);
    int GetCodecType(const std::string& codec);
    int GetProfileType(const std::string& codec, const std::string& profile);
    int GetLevelType(const std::string& codec, const std::string& level);
    int GetRcMode(const std::string& mode);
    int GetAudioCodecType(const std::string& codec);

    // ========================================================================
    // State
    // ========================================================================

    std::atomic<PipelineState> state_{PipelineState::kUninitialized};
    HdalPipelineConfig config_;
    bool config_loaded_ = false;

#if HDAL_PIPELINE_ENABLED
    VideoStreamHandle video_streams_[kMaxVideoStreams];
    AudioStreamHandle audio_stream_;
    
    // Dedicated 3DNR reference path (OUT_0 at full sensor resolution, no encoder)
    HD_PATH_ID dnr_ref_proc_path_ = 0;
    
    // AI Analytics path (VideoProc output for AI, no encoder)
    HD_PATH_ID analytics_proc_path_ = 0;
    HD_DIM analytics_dim_ = {640, 360};  // AI resolution
    bool analytics_enabled_ = false;
    
    // Motion Detection path (VideoProc output for libmd processing)
    HD_PATH_ID md_proc_path_ = 0;
    HD_DIM md_dim_ = {160, 120};  // MD resolution (standard for libmd)
    bool md_path_enabled_ = false;
    
    // VQA Tamper Detection path (VideoProc output for IVE-accelerated detection)
    HD_PATH_ID vqa_proc_path_ = 0;
    HD_DIM vqa_dim_ = {320, 180};  // VQA resolution (optimized for IVE histogram/Sobel)
    bool vqa_path_enabled_ = false;
    
    // Global privacy mask paths (VIDEOPROC level)
    HD_PATH_ID global_mask_paths_[kMaxGlobalMasks] = {0, 0, 0, 0};
    GlobalMaskRegion global_mask_regions_[kMaxGlobalMasks];
    bool global_mask_enabled_ = false;
    
    // Capability info
    HD_VIDEOCAP_SYSCAPS cap_syscaps_;
    HD_VIDEOPROC_SYSCAPS proc_syscaps_;
    HD_VIDEOENC_SYSCAPS enc_syscaps_;
#endif

    // RTSP server PID
    pid_t rtsp_pid_ = -1;
};

// ============================================================================
// Convenience Functions
// ============================================================================

/**
 * @brief Initialize and start the HDAL pipeline
 * @return true on success
 * 
 * Calls LoadConfig(), Init(), Start() in sequence.
 */
bool StartHdalPipeline();

/**
 * @brief Stop and shutdown the HDAL pipeline
 */
void StopHdalPipeline();

} // namespace platform
} // namespace ipcam

#endif // IPCAM_HDAL_PIPELINE_H
