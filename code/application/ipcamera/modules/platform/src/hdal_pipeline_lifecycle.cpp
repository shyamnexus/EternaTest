/**
 * @file hdal_pipeline_lifecycle.cpp
 * @brief HDAL Pipeline - Lifecycle management and runtime control
 *
 * This file contains the main lifecycle methods (Init, Start, Stop, Shutdown),
 * path binding/unbinding, runtime stream control, RTSP server integration,
 * and statistics gathering.
 *
 * Part of the split hdal_pipeline implementation for maintainability.
 */

#include "hdal_pipeline_common.h"
#include "ipcam/hdal_pipeline.h"
#include "ipcam/osd_overlay.h"
#include "ipcam/config.h"

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ipcam {
namespace platform {

// Global mutex for pipeline operations - definition (declared extern in common.h)
std::mutex g_pipeline_mutex;

// ============================================================================
// Singleton and Constructor/Destructor
// ============================================================================

HdalPipeline& HdalPipeline::Instance() {
    static HdalPipeline instance;
    return instance;
}

HdalPipeline::HdalPipeline()
    : state_(PipelineState::kUninitialized)
    , config_loaded_(false)
    , rtsp_pid_(-1)
    , audio_stream_{} {
    
    // Initialize video stream array
    for (int i = 0; i < kMaxVideoStreams; i++) {
        video_streams_[i] = {};
    }
}

HdalPipeline::~HdalPipeline() {
    if (state_ != PipelineState::kUninitialized) {
        Shutdown();
    }
}

// ============================================================================
// Path Binding
// ============================================================================

bool HdalPipeline::BindPaths() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Binding HDAL paths...");
    
    int vcap_id = config_.sensor.vcap_id;
    
    // Bind video capture to video processing
    HD_OUT_ID vcap_out = static_cast<HD_OUT_ID>(HD_VIDEOCAP_OUT(vcap_id, 0));
    hd_videocap_bind(vcap_out, HD_VIDEOPROC_0_IN_0);
    
    // NOTE: OUT_0 is the dedicated 3DNR reference path — it is NOT bound to
    // any encoder.  User streams are shifted: stream 0→OUT_1/ENC_0, etc.
    const HD_OUT_ID proc_out_ids[] = {
        HD_VIDEOPROC_0_OUT_1, HD_VIDEOPROC_0_OUT_2, HD_VIDEOPROC_0_OUT_3
    };
    const HD_IN_ID enc_in_ids[] = {
        HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_IN_2
    };
    const int max_user_streams = std::min(kMaxVideoStreams, 3);
    
    for (int i = 0; i < max_user_streams; i++) {
        if (video_streams_[i].enabled) {
            hd_videoproc_bind(proc_out_ids[i], enc_in_ids[i]);
            spdlog::debug("Bound VideoProc OUT_{} -> VideoEnc IN_{}", i + 1, i);
        }
    }
    
    // Bind audio capture to audio encoder
    if (audio_stream_.enabled) {
        hd_audiocap_bind(HD_AUDIOCAP_0_OUT_0, HD_AUDIOENC_0_IN_0);
        spdlog::debug("Bound AudioCap -> AudioEnc");
    }
    
    spdlog::info("HDAL paths bound");
    return true;
#else
    return true;
#endif
}

void HdalPipeline::UnbindPaths() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Unbinding HDAL paths...");
    
    int vcap_id = config_.sensor.vcap_id;
    
    // Unbind video capture
    HD_OUT_ID vcap_out = static_cast<HD_OUT_ID>(HD_VIDEOCAP_OUT(vcap_id, 0));
    hd_videocap_unbind(vcap_out);
    
    // Unbind user stream video processing outputs (OUT_1..OUT_3)
    // OUT_0 (3DNR ref) is not bound to any encoder, no unbind needed
    const HD_OUT_ID proc_out_ids[] = {
        HD_VIDEOPROC_0_OUT_1, HD_VIDEOPROC_0_OUT_2, HD_VIDEOPROC_0_OUT_3
    };
    const int max_user_streams = std::min(kMaxVideoStreams, 3);
    
    for (int i = 0; i < max_user_streams; i++) {
        if (video_streams_[i].enabled) {
            hd_videoproc_unbind(proc_out_ids[i]);
        }
    }
    
    // Unbind audio
    if (audio_stream_.enabled) {
        hd_audiocap_unbind(HD_AUDIOCAP_0_OUT_0);
    }
    
    spdlog::info("HDAL paths unbound");
#endif
}

void HdalPipeline::ClosePaths() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Closing HDAL paths...");
    
    // Close video encoder and processor paths
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (video_streams_[i].enc_path) {
            hd_videoenc_close(video_streams_[i].enc_path);
            video_streams_[i].enc_path = 0;
        }
        if (video_streams_[i].proc_path) {
            hd_videoproc_close(video_streams_[i].proc_path);
            video_streams_[i].proc_path = 0;
        }
    }
    
    // Close dedicated 3DNR reference path (OUT_0)
    if (dnr_ref_proc_path_) {
        hd_videoproc_close(dnr_ref_proc_path_);
        dnr_ref_proc_path_ = 0;
        spdlog::info("3DNR reference path closed (OUT_0)");
    }
    
    // Close video capture path (shared by all streams)
    if (video_streams_[0].cap_path) {
        hd_videocap_close(video_streams_[0].cap_path);
        video_streams_[0].cap_path = 0;
    }
    
    // Close analytics VideoProc path
    if (analytics_proc_path_) {
        hd_videoproc_close(analytics_proc_path_);
        analytics_proc_path_ = 0;
        analytics_enabled_ = false;
    }
    
    // Close MD VideoProc path
    if (md_proc_path_) {
        hd_videoproc_close(md_proc_path_);
        md_proc_path_ = 0;
        md_path_enabled_ = false;
    }
    
    // Close VQA VideoProc path (tamper detection)
    if (vqa_proc_path_) {
        hd_videoproc_close(vqa_proc_path_);
        vqa_proc_path_ = 0;
        vqa_path_enabled_ = false;
    }
    
    // Close global privacy mask paths
    CloseGlobalMaskPaths();
    
    // Close audio paths
    if (audio_stream_.enc_path) {
        hd_audioenc_close(audio_stream_.enc_path);
        audio_stream_.enc_path = 0;
    }
    if (audio_stream_.cap_path) {
        hd_audiocap_close(audio_stream_.cap_path);
        audio_stream_.cap_path = 0;
    }
    if (audio_stream_.cap_ctrl) {
        hd_audiocap_close(audio_stream_.cap_ctrl);
        audio_stream_.cap_ctrl = 0;
    }
    
    spdlog::info("HDAL paths closed");
#endif
}

// ============================================================================
// Main Lifecycle Methods
// ============================================================================

bool HdalPipeline::Init() {
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);
    
    if (state_ != PipelineState::kUninitialized) {
        spdlog::warn("HDAL pipeline already initialized");
        return true;
    }
    
    spdlog::info("Initializing HDAL pipeline...");
    
    // Load configuration if not already loaded
    if (!config_loaded_ && !LoadConfig()) {
        spdlog::error("Failed to load HDAL config");
        state_ = PipelineState::kError;
        return false;
    }
    
    // Initialize in sequence - all steps must succeed
    if (!InitVendorIsp()) goto error;
    if (!LoadIspTuningConfig()) goto error;
    if (!InitHdalCommon()) goto error;
    if (!InitMemoryPools()) goto error;
    if (!InitModules()) goto error;
    if (!OpenVideoPaths()) goto error;
    if (!OpenAnalyticsPath()) goto error;  // AI analytics YUV path
    if (!OpenMdPath()) goto error;         // Motion detection YUV path (160x120)
    if (!OpenVqaPath()) goto error;        // VQA tamper detection YUV path (320x180)
    if (!OpenAudioPaths()) goto error;
    
    // Open global privacy mask paths (VIDEOPROC level)
    // These are optional - failure doesn't prevent pipeline from working
    OpenGlobalMaskPaths();
    
    if (!ConfigureCapture()) goto error;
    if (!ConfigureVideoProc()) goto error;
    if (!ConfigureAnalyticsProc()) goto error;  // Configure analytics output
    if (!ConfigureMdProc()) goto error;         // Configure MD output (160x120 for libmd)
    if (!ConfigureVqaProc()) goto error;        // Configure VQA output (320x180 for IVE tamper detection)
    if (!ConfigureEncoders()) goto error;
    if (!ConfigureAudioCapture()) goto error;  // Audio capture config before binding
    if (!ConfigureAudioEncoder()) goto error;  // Audio encoder config BEFORE binding (per sample)
    if (!BindPaths()) goto error;
    
    // Initialize OSD overlay on main stream encoder
#if HDAL_PIPELINE_ENABLED
    if (video_streams_[0].enabled && video_streams_[0].enc_path) {
        OsdOverlay::Instance().LoadConfig();
        if (!OsdOverlay::Instance().Init(reinterpret_cast<void*>(video_streams_[0].enc_path))) {
            spdlog::warn("OSD initialization failed, continuing without OSD");
        }
    }
#endif
    
    state_ = PipelineState::kInitialized;
    spdlog::info("HDAL pipeline initialized successfully");
    return true;
    
error:
    spdlog::error("HDAL pipeline initialization failed");
    Shutdown();
    state_ = PipelineState::kError;
    return false;
}

bool HdalPipeline::Start() {
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);
    
    if (state_ == PipelineState::kRunning) {
        spdlog::warn("HDAL pipeline already running");
        return true;
    }
    
    if (state_ != PipelineState::kInitialized && state_ != PipelineState::kStopped) {
        spdlog::error("HDAL pipeline not initialized, current state: {}", 
                      static_cast<int>(state_.load()));
        return false;
    }
    
    spdlog::info("Starting HDAL pipeline...");
    
#if HDAL_PIPELINE_ENABLED
    // Start video capture first
    if (video_streams_[0].cap_path) {
        hd_videocap_start(video_streams_[0].cap_path);
    }
    
    // Start video processing for all enabled streams
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (video_streams_[i].enabled && video_streams_[i].proc_path) {
            hd_videoproc_start(video_streams_[i].proc_path);
        }
    }
    
    // Start dedicated 3DNR reference path (OUT_0 at full resolution)
    if (dnr_ref_proc_path_) {
        hd_videoproc_start(dnr_ref_proc_path_);
        spdlog::info("3DNR reference path started (OUT_0)");
    }
    
    // Start analytics VideoProc path (AI YUV output)
    if (analytics_enabled_ && analytics_proc_path_) {
        hd_videoproc_start(analytics_proc_path_);
        spdlog::info("Analytics VideoProc path started");
    }
    
    // Start MD VideoProc path (160x120 YUV for libmd)
    if (md_path_enabled_ && md_proc_path_) {
        hd_videoproc_start(md_proc_path_);
        spdlog::info("Motion Detection VideoProc path started ({}x{})", md_dim_.w, md_dim_.h);
    }
    
    // Start VQA VideoProc path (320x180 YUV for IVE-accelerated tamper detection)
    if (vqa_path_enabled_ && vqa_proc_path_) {
        hd_videoproc_start(vqa_proc_path_);
        spdlog::info("VQA tamper detection VideoProc path started ({}x{})", vqa_dim_.w, vqa_dim_.h);
    }
    
    // Wait for AE/AWB to stabilize before starting encoders
    usleep(100000);  // 100ms
    
    // Start video encoders
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (video_streams_[i].enabled && video_streams_[i].enc_path) {
            HD_RESULT ret = hd_videoenc_start(video_streams_[i].enc_path);
            if (ret == HD_OK) {
                video_streams_[i].encoder_running = true;
            } else {
                spdlog::error("Failed to start encoder for stream {}: {}", i, static_cast<int>(ret));
            }
        }
    }
    
    // Start audio pipeline (encoder first, then capture - per sample)
    spdlog::info("Audio start check: enabled={} cap_path=0x{:x} enc_path=0x{:x}",
                 audio_stream_.enabled, audio_stream_.cap_path, audio_stream_.enc_path);
    if (audio_stream_.enabled) {
        // Start encoder FIRST (sample does this order)
        if (audio_stream_.enc_path) {
            HD_RESULT ret = hd_audioenc_start(audio_stream_.enc_path);
            spdlog::info("hd_audioenc_start(0x{:x}) = {}", audio_stream_.enc_path, static_cast<int>(ret));
        }
        // Then start capture
        if (audio_stream_.cap_path) {
            HD_RESULT ret = hd_audiocap_start(audio_stream_.cap_path);
            spdlog::info("hd_audiocap_start(0x{:x}) = {}", audio_stream_.cap_path, static_cast<int>(ret));
        }
    }
    
    // Start OSD overlay
    if (OsdOverlay::Instance().IsInitialized()) {
        OsdOverlay::Instance().Start();
    }
#endif
    
    state_ = PipelineState::kRunning;
    spdlog::info("HDAL pipeline started");
    
    // Start RTSP server to stream the encoded video
    // Note: If streaming.rtsp.enabled is true, the new Live555 RTSP server
    // will be started by ipcamd instead of nvtrtspd_ipc
    bool use_new_rtsp = config::Get<bool>("streaming.rtsp.enabled", false);
    if (!use_new_rtsp) {
        StartRtspServer();  // Start legacy nvtrtspd_ipc
    } else {
        spdlog::info("Skipping nvtrtspd_ipc - new RTSP server will be started by ipcamd");
    }
    
    return true;
}

bool HdalPipeline::Stop() {
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);
    
    if (state_ != PipelineState::kRunning) {
        return true;
    }
    
    spdlog::info("Stopping HDAL pipeline...");
    
#if HDAL_PIPELINE_ENABLED
    // Stop OSD first
    if (OsdOverlay::Instance().IsRunning()) {
        OsdOverlay::Instance().Stop();
    }
    
    // Stop audio
    if (audio_stream_.enabled) {
        if (audio_stream_.cap_path) {
            hd_audiocap_stop(audio_stream_.cap_path);
        }
        if (audio_stream_.enc_path) {
            hd_audioenc_stop(audio_stream_.enc_path);
        }
    }
    
    // Stop video encoders
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (video_streams_[i].enabled && video_streams_[i].enc_path) {
            hd_videoenc_stop(video_streams_[i].enc_path);
            video_streams_[i].encoder_running = false;
        }
    }
    
    // Stop video processing
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (video_streams_[i].enabled && video_streams_[i].proc_path) {
            hd_videoproc_stop(video_streams_[i].proc_path);
        }
    }
    
    // Stop dedicated 3DNR reference path (OUT_0)
    if (dnr_ref_proc_path_) {
        hd_videoproc_stop(dnr_ref_proc_path_);
        spdlog::info("3DNR reference path stopped (OUT_0)");
    }
    
    // Stop analytics VideoProc path
    if (analytics_enabled_ && analytics_proc_path_) {
        hd_videoproc_stop(analytics_proc_path_);
    }
    
    // Stop MD VideoProc path
    if (md_path_enabled_ && md_proc_path_) {
        hd_videoproc_stop(md_proc_path_);
    }
    
    // Stop VQA VideoProc path (tamper detection)
    if (vqa_path_enabled_ && vqa_proc_path_) {
        hd_videoproc_stop(vqa_proc_path_);
    }
    
    // Stop video capture last
    if (video_streams_[0].cap_path) {
        hd_videocap_stop(video_streams_[0].cap_path);
    }
#endif
    
    state_ = PipelineState::kStopped;
    spdlog::info("HDAL pipeline stopped");
    return true;
}

void HdalPipeline::Shutdown() {
    // Note: We don't lock here because Stop() has its own lock
    // and we need to handle the recursive case
    
    if (state_ == PipelineState::kUninitialized) {
        return;
    }
    
    spdlog::info("Shutting down HDAL pipeline...");
    
    // Stop RTSP server first
    StopRtspServer();
    
    // Stop pipeline if running
    if (state_ == PipelineState::kRunning) {
        Stop();
    }
    
    // Cleanup in reverse order of initialization
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);
    
    // Shutdown OSD first
    OsdOverlay::Instance().Shutdown();
    
    UnbindPaths();
    ClosePaths();
    UninitModules();
    UninitMemory();
    UninitHdalCommon();
    UninitVendorIsp();
    
    state_ = PipelineState::kUninitialized;
    spdlog::info("HDAL pipeline shut down");
}

bool HdalPipeline::ReinitWithConfig() {
    spdlog::info("Reinitializing HDAL pipeline with updated config...");

    const bool was_running = (state_ == PipelineState::kRunning);
    Shutdown();

    // Force LoadConfig() to re-read from config store so Init() picks up
    // any values that changed since the last load (e.g. resolution, crop).
    config_loaded_ = false;

    if (!Init()) {
        spdlog::error("ReinitWithConfig: Init failed");
        return false;
    }

    if (was_running) {
        if (!Start()) {
            spdlog::error("ReinitWithConfig: Start failed");
            return false;
        }
    }

    spdlog::info("ReinitWithConfig: Completed successfully");
    return true;
}

bool HdalPipeline::SetStreamBitrate(int stream_id, int bitrate_bps) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("Invalid stream_id: {}", stream_id);
        return false;
    }
    
    if (bitrate_bps <= 0 || bitrate_bps > 50000000) {
        spdlog::error("Invalid bitrate: {}bps", bitrate_bps);
        return false;
    }
    
#if HDAL_PIPELINE_ENABLED
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);
    
    if (!video_streams_[stream_id].enabled || !video_streams_[stream_id].enc_path) {
        spdlog::error("Stream {} not enabled", stream_id);
        return false;
    }
    
    HD_PATH_ID enc_path = video_streams_[stream_id].enc_path;
    
    // =========================================================================
    // CRITICAL: Check if new bitrate exceeds PATH_CONFIG limit
    // If so, we must reconfigure PATH_CONFIG which requires STOP -> SET -> START
    // Error: "path config bitrate(X) must be >= HD_H26XENC_CBR: bitrate(Y)"
    // =========================================================================
    uint32_t current_path_bitrate = video_streams_[stream_id].path_config_bitrate;
    bool need_path_reconfig = (static_cast<uint32_t>(bitrate_bps) > current_path_bitrate);
    
    if (need_path_reconfig) {
        spdlog::info("SetStreamBitrate: Bitrate {}bps exceeds PATH_CONFIG limit {}bps, reconfiguring...",
                     bitrate_bps, current_path_bitrate);
        
        // Must stop encoder to update PATH_CONFIG
        bool was_running = video_streams_[stream_id].encoder_running;
        if (was_running) {
            HD_RESULT stop_ret = hd_videoenc_stop(enc_path);
            if (stop_ret == HD_OK) {
                video_streams_[stream_id].encoder_running = false;
            } else if (stop_ret == -26) {
                spdlog::debug("SetStreamBitrate: Encoder already stopped");
                was_running = false;
            } else {
                spdlog::error("SetStreamBitrate: Failed to stop encoder for PATH_CONFIG update: {}", 
                              static_cast<int>(stop_ret));
                return false;
            }
            usleep(30000);  // 30ms for encoder to fully stop
        }
        
        // Update PATH_CONFIG with new bitrate limit (use some headroom for future changes)
        const auto& stream_cfg = config_.video_streams[stream_id];
        HD_VIDEOENC_PATH_CONFIG path_cfg;
        memset(&path_cfg, 0, sizeof(path_cfg));
        path_cfg.max_mem.codec_type = HD_CODEC_TYPE_H264;  // Always H.264 for max resources
        path_cfg.max_mem.max_dim.w = stream_cfg.width;
        path_cfg.max_mem.max_dim.h = stream_cfg.height;
        // Add 25% headroom to avoid frequent reconfiguration
        path_cfg.max_mem.bitrate = bitrate_bps + (bitrate_bps / 4);
        path_cfg.max_mem.enc_buf_ms = stream_cfg.encoder.buffer_size;
        path_cfg.max_mem.svc_layer = HD_SVC_4X;
        path_cfg.max_mem.ltr = TRUE;
        path_cfg.max_mem.rotate = FALSE;
        path_cfg.max_mem.source_output = FALSE;
        path_cfg.isp_id = config_.sensor.vcap_id;
        
        HD_RESULT ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &path_cfg);
        if (ret != HD_OK) {
            spdlog::error("SetStreamBitrate: Failed to update PATH_CONFIG: {}", static_cast<int>(ret));
            // Try to restart encoder
            if (was_running) {
                if (hd_videoenc_start(enc_path) == HD_OK) {
                    video_streams_[stream_id].encoder_running = true;
                }
            }
            return false;
        }
        video_streams_[stream_id].path_config_bitrate = path_cfg.max_mem.bitrate;
        spdlog::info("SetStreamBitrate: PATH_CONFIG updated to {}bps", path_cfg.max_mem.bitrate);
        
        // Restart encoder
        ret = hd_videoenc_start(enc_path);
        if (ret != HD_OK) {
            spdlog::error("SetStreamBitrate: Failed to restart encoder after PATH_CONFIG update: {}", 
                          static_cast<int>(ret));
            video_streams_[stream_id].encoder_running = false;
            return false;
        }
        video_streams_[stream_id].encoder_running = true;
        usleep(20000);  // 20ms for encoder to stabilize
    }
    
    HD_H26XENC_RATE_CONTROL rc;
    memset(&rc, 0, sizeof(rc));
    
    // Determine RC mode from config (case-insensitive)
    const auto& stream_cfg = config_.video_streams[stream_id];
    const auto& enc_cfg = stream_cfg.encoder;
    const auto& path_max_dim = video_streams_[stream_id].path_config_max_dim;
    bool need_path_config = (path_max_dim.w == 0 || path_max_dim.h == 0 ||
                             stream_cfg.width > static_cast<int>(path_max_dim.w) ||
                             stream_cfg.height > static_cast<int>(path_max_dim.h));
    int rc_mode_int = GetRcMode(enc_cfg.rc_mode);
    rc.rc_mode = static_cast<HD_VIDEOENC_RC_MODE>(rc_mode_int);
    
    if (rc.rc_mode == HD_RC_MODE_CBR) {
        rc.cbr.bitrate = bitrate_bps;
        rc.cbr.frame_rate_base = config_.video_streams[stream_id].fps;
        rc.cbr.frame_rate_incr = 1;
        rc.cbr.init_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
        rc.cbr.max_i_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp : 45;
        rc.cbr.min_i_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 15;
        rc.cbr.init_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
        int max_p_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp + 3 : 48;
        rc.cbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
        rc.cbr.min_p_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 15;
    } else if (rc.rc_mode == HD_RC_MODE_VBR) {
        rc.vbr.bitrate = bitrate_bps;
        rc.vbr.frame_rate_base = config_.video_streams[stream_id].fps;
        rc.vbr.frame_rate_incr = 1;
        rc.vbr.init_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
        rc.vbr.max_i_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp : 45;
        rc.vbr.min_i_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 15;
        rc.vbr.init_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
        int max_p_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp + 3 : 48;
        rc.vbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
        rc.vbr.min_p_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 15;
    } else {
        // FIXQP mode doesn't support bitrate control
        spdlog::warn("Cannot set bitrate in {} mode for stream {}", enc_cfg.rc_mode, stream_id);
        return false;
    }
    
    spdlog::info("Setting stream {} bitrate to {}bps ({:.2f}Mbps), enc_path=0x{:x}", 
                 stream_id, bitrate_bps, bitrate_bps / 1000000.0, video_streams_[stream_id].enc_path);
    
    // Use RATE_CONTROL2 for dynamic runtime change (works while encoder is running)
    HD_RESULT ret = hd_videoenc_set(video_streams_[stream_id].enc_path, 
                                     HD_VIDEOENC_PARAM_OUT_RATE_CONTROL2, &rc);
    if (ret != HD_OK) {
        spdlog::error("Failed to set bitrate for stream {}: {} (HD_RESULT)", stream_id, static_cast<int>(ret));
        return false;
    }
    
    // Update config (store in kbps for config file compatibility)
    config_.video_streams[stream_id].encoder.bitrate = bitrate_bps / 1000;  // Store as kbps
    spdlog::info("✓ Stream {} bitrate successfully updated to {}bps ({:.2f}Mbps, {} kbps)", 
                 stream_id, bitrate_bps, bitrate_bps / 1000000.0, bitrate_bps / 1000);
    return true;
#else
    return false;
#endif
}

bool HdalPipeline::SetStreamEnabled(int stream_id, bool enabled) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("Invalid stream_id: {}", stream_id);
        return false;
    }
    if (!config_loaded_ && !LoadConfig()) {
        spdlog::error("SetStreamEnabled: Config not loaded");
        return false;
    }
#if HDAL_PIPELINE_ENABLED
    std::unique_lock<std::mutex> lock(g_pipeline_mutex);

    if (enabled) {
        // Already enabled, nothing to do
        if (video_streams_[stream_id].enabled) {
            spdlog::info("SetStreamEnabled: Stream {} already enabled", stream_id);
            return true;
        }

        // Enable: mark enabled, then release the lock before reinit to avoid deadlock
        spdlog::info("SetStreamEnabled: Enabling stream {} via pipeline reinit", stream_id);
        video_streams_[stream_id].enabled = true;
        config_.video_streams[stream_id].enabled = true;

        lock.unlock();
        bool ok = ReinitWithConfig();
        if (!ok) {
            std::lock_guard<std::mutex> relock(g_pipeline_mutex);
            video_streams_[stream_id].enabled = false;
            config_.video_streams[stream_id].enabled = false;
            spdlog::error("SetStreamEnabled: Reinit failed while enabling stream {}", stream_id);
        }
        return ok;
    }

    // Disable: stop encoder/proc paths if running and mark disabled
    if (video_streams_[stream_id].encoder_running && video_streams_[stream_id].enc_path) {
        spdlog::info("SetStreamEnabled: Stopping encoder for stream {}", stream_id);
        hd_videoenc_stop(video_streams_[stream_id].enc_path);
        video_streams_[stream_id].encoder_running = false;
    }

    if (video_streams_[stream_id].proc_path) {
        spdlog::info("SetStreamEnabled: Stopping video proc for stream {}", stream_id);
        hd_videoproc_stop(video_streams_[stream_id].proc_path);
    }

    video_streams_[stream_id].enabled = false;
    config_.video_streams[stream_id].enabled = false;
    spdlog::info("SetStreamEnabled: Stream {} disabled", stream_id);
    return true;
#else
    return false;
#endif
}

bool HdalPipeline::SetStreamFps(int stream_id, int fps) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("SetStreamFps: Invalid stream_id: {}", stream_id);
        return false;
    }
    if (fps <= 0 || fps > 120) {
        spdlog::error("SetStreamFps: Invalid fps: {}", fps);
        return false;
    }

#if HDAL_PIPELINE_ENABLED
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);

    if (!video_streams_[stream_id].enabled || !video_streams_[stream_id].enc_path) {
        spdlog::error("SetStreamFps: Stream {} not enabled or encoder path missing", stream_id);
        return false;
    }

    HD_H26XENC_RATE_CONTROL rc;
    memset(&rc, 0, sizeof(rc));

    const auto& stream_cfg = config_.video_streams[stream_id];
    const auto& enc_cfg = stream_cfg.encoder;
    int rc_mode_int = GetRcMode(enc_cfg.rc_mode);
    rc.rc_mode = static_cast<HD_VIDEOENC_RC_MODE>(rc_mode_int);

    // Keep current bitrate/QP settings while updating the framerate
    const int current_bitrate = enc_cfg.bitrate > 0 ? enc_cfg.bitrate * 1000 : 0;  // kbps -> bps

    if (rc.rc_mode == HD_RC_MODE_CBR) {
        rc.cbr.bitrate = current_bitrate;
        rc.cbr.frame_rate_base = fps;
        rc.cbr.frame_rate_incr = 1;
        rc.cbr.init_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
        rc.cbr.max_i_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp : 45;
        rc.cbr.min_i_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 15;
        rc.cbr.init_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
        int max_p_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp + 3 : 48;
        rc.cbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
        rc.cbr.min_p_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 15;
    } else if (rc.rc_mode == HD_RC_MODE_VBR) {
        rc.vbr.bitrate = current_bitrate;
        rc.vbr.frame_rate_base = fps;
        rc.vbr.frame_rate_incr = 1;
        rc.vbr.init_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
        rc.vbr.max_i_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp : 45;
        rc.vbr.min_i_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 15;
        rc.vbr.init_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
        int max_p_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp + 3 : 48;
        rc.vbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
        rc.vbr.min_p_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 15;
    } else {
        spdlog::warn("SetStreamFps: Cannot change FPS in {} mode for stream {}", enc_cfg.rc_mode, stream_id);
        return false;
    }

    spdlog::info("Setting stream {} FPS to {}, enc_path=0x{:x}",
                 stream_id, fps, video_streams_[stream_id].enc_path);

    // Determine sensor (source) frame rate for FRC calculation.
    // The sensor delivers at a fixed rate; encoder input FRC drops frames.
    int sensor_fps = config_.video_streams[0].fps;  // stream 0 drives sensor FRC
    if (sensor_fps <= 0) sensor_fps = 30;
    int target_fps = (fps < sensor_fps) ? fps : sensor_fps;  // cannot exceed source rate

    // Restart encoder to ensure SPS/VUI and clients pick up new framerate
    bool was_running = video_streams_[stream_id].encoder_running;
    HD_PATH_ID enc_path = video_streams_[stream_id].enc_path;
    HD_RESULT ret = HD_OK;

    if (was_running) {
        spdlog::debug("SetStreamFps: Stopping encoder for FPS change...");
        ret = hd_videoenc_stop(enc_path);
        if (ret != HD_OK && ret != -26) {
            spdlog::error("Failed to stop encoder for stream {}: {}", stream_id, static_cast<int>(ret));
            return false;
        }
        if (ret == HD_OK) {
            video_streams_[stream_id].encoder_running = false;
        }
        usleep(30000);  // 30ms to fully stop
    }

    // Set encoder input FRC to drop frames at the encoder input stage.
    // FRC_RATIO(dst_fps, src_fps): encoder takes dst_fps out of every src_fps input frames.
    {
        HD_VIDEOENC_IN enc_in;
        memset(&enc_in, 0, sizeof(enc_in));
        enc_in.dir = HD_VIDEO_DIR_NONE;
        enc_in.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
        enc_in.dim = video_streams_[stream_id].enc_dim;
        enc_in.frc = HD_VIDEO_FRC_RATIO(target_fps, sensor_fps);

        ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_IN, &enc_in);
        if (ret != HD_OK) {
            spdlog::warn("SetStreamFps: hd_videoenc_set(IN FRC {}/{}) failed: {}",
                         target_fps, sensor_fps, static_cast<int>(ret));
            // Non-fatal: encoder may still work at old rate
        } else {
            spdlog::info("SetStreamFps: Encoder input FRC set to {}/{} for stream {}",
                         target_fps, sensor_fps, stream_id);
        }
    }

    ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL2, &rc);
    if (ret != HD_OK) {
        spdlog::error("Failed to set FPS for stream {}: {} (HD_RESULT)", stream_id, static_cast<int>(ret));
        // Try to restart encoder if we stopped it
        if (was_running) {
            hd_videoenc_start(enc_path);
            video_streams_[stream_id].encoder_running = true;
        }
        return false;
    }

    if (was_running) {
        spdlog::debug("SetStreamFps: Restarting encoder after FPS change...");
        ret = hd_videoenc_start(enc_path);
        if (ret != HD_OK) {
            spdlog::error("Failed to restart encoder after FPS change for stream {}: {}", stream_id, static_cast<int>(ret));
            video_streams_[stream_id].encoder_running = false;
            return false;
        }
        video_streams_[stream_id].encoder_running = true;

        // Request IDR to refresh SPS/VPS/PPS downstream
        HD_H26XENC_REQUEST_IFRAME req;
        memset(&req, 0, sizeof(req));
        req.enable = TRUE;
        hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, &req);
    }

    config_.video_streams[stream_id].fps = fps;
    spdlog::info("✓ Stream {} FPS successfully updated to {} (encoder restarted)", stream_id, fps);
    return true;
#else
    return false;
#endif
}

bool HdalPipeline::SetStreamResolution(int stream_id, int width, int height) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("SetStreamResolution: Invalid stream_id: {}", stream_id);
        return false;
    }

    if (width <= 0 || height <= 0) {
        spdlog::error("SetStreamResolution: Invalid resolution {}x{}", width, height);
        return false;
    }

    const int max_w = config_.sensor.crop.enabled ? config_.sensor.crop.width : config_.sensor.native_width;
    const int max_h = config_.sensor.crop.enabled ? config_.sensor.crop.height : config_.sensor.native_height;
    const int aligned_w = width - (width % 32);
    const int aligned_h = height - (height % 2);
    if (aligned_w != width) {
        spdlog::warn("SetStreamResolution: Width {} not 32-aligned, using {}", width, aligned_w);
        width = aligned_w;
    }
    if (aligned_h != height) {
        spdlog::warn("SetStreamResolution: Height {} not 2-aligned, using {}", height, aligned_h);
        height = aligned_h;
    }
    if (width > max_w) {
        spdlog::warn("SetStreamResolution: Width {} exceeds capture max {}, clamping", width, max_w);
        width = max_w;
    }
    if (height > max_h) {
        spdlog::warn("SetStreamResolution: Height {} exceeds capture max {}, clamping", height, max_h);
        height = max_h;
    }
    // All user streams use OUT_1..OUT_3 (shifted from OUT_0 which is 3DNR ref).
    // No special clamping needed — all streams can freely downscale from capture.

#if HDAL_PIPELINE_ENABLED
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);

    if (config_.video_streams[stream_id].width == width &&
        config_.video_streams[stream_id].height == height) {
        return true;
    }

    if (video_streams_[0].cap_dim.w > 0 && video_streams_[0].cap_dim.h > 0) {
        if (width > static_cast<int>(video_streams_[0].cap_dim.w) ||
            height > static_cast<int>(video_streams_[0].cap_dim.h)) {
            spdlog::error("SetStreamResolution: Requested {}x{} exceeds capture {}x{}",
                          width, height, video_streams_[0].cap_dim.w, video_streams_[0].cap_dim.h);
            return false;
        }
    }

    spdlog::info("SetStreamResolution: Changing stream {} resolution to {}x{}", stream_id, width, height);

    config_.video_streams[stream_id].width = width;
    config_.video_streams[stream_id].height = height;

    if (!video_streams_[stream_id].enabled ||
        !video_streams_[stream_id].proc_path ||
        !video_streams_[stream_id].enc_path) {
        spdlog::warn("SetStreamResolution: Stream {} not active, will apply on next start", stream_id);
        return true;
    }

    HD_PATH_ID enc_path = video_streams_[stream_id].enc_path;
    bool was_running = video_streams_[stream_id].encoder_running;
    HD_RESULT ret = HD_OK;

    if (was_running) {
        spdlog::debug("SetStreamResolution: Stopping encoder for resolution change...");
        ret = hd_videoenc_stop(enc_path);
        if (ret != HD_OK) {
            if (ret == -26) {
                spdlog::debug("SetStreamResolution: Encoder already stopped (ret=-26)");
                was_running = false;
            } else {
                spdlog::error("SetStreamResolution: Failed to stop encoder: {}", static_cast<int>(ret));
                return false;
            }
        } else {
            video_streams_[stream_id].encoder_running = false;
        }
        usleep(30000);
    }

    // -----------------------------------------------------------------------
    // VideoProc output: the Novatek encoder does NOT scale — it requires
    // the videoproc output to match the encoder input exactly.  We change
    // BOTH videoproc OUT and encoder IN to the target resolution.
    //
    // Since user streams are mapped to OUT_1..OUT_3 (not OUT_0), and the
    // dedicated 3DNR reference (OUT_0) always stays at full sensor
    // resolution, resolution changes on any user stream are safe and
    // will not degrade 3DNR quality.
    // -----------------------------------------------------------------------
    if (state_ == PipelineState::kRunning && video_streams_[stream_id].proc_path) {
        hd_videoproc_stop(video_streams_[stream_id].proc_path);
    }

    HD_VIDEOPROC_OUT proc_out;
    memset(&proc_out, 0, sizeof(proc_out));
    proc_out.func = 0;
    proc_out.dim.w = width;
    proc_out.dim.h = height;
    proc_out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
    proc_out.dir = HD_VIDEO_DIR_NONE;
    proc_out.frc = HD_VIDEO_FRC_RATIO(1, 1);

    ret = hd_videoproc_set(video_streams_[stream_id].proc_path, HD_VIDEOPROC_PARAM_OUT, &proc_out);
    if (ret != HD_OK) {
        spdlog::error("SetStreamResolution: hd_videoproc_set(OUT) failed: {}", static_cast<int>(ret));
        return false;
    }
    video_streams_[stream_id].proc_dim = proc_out.dim;

    const auto& enc_cfg = config_.video_streams[stream_id].encoder;
    const auto& path_max_dim = video_streams_[stream_id].path_config_max_dim;
    bool need_path_config = (path_max_dim.w == 0 || path_max_dim.h == 0 ||
                             width > static_cast<int>(path_max_dim.w) ||
                             height > static_cast<int>(path_max_dim.h));

    if (need_path_config) {
        HD_VIDEOENC_PATH_CONFIG path_cfg;
        memset(&path_cfg, 0, sizeof(path_cfg));
        path_cfg.max_mem.codec_type = HD_CODEC_TYPE_H264;
        path_cfg.max_mem.max_dim.w = width;
        path_cfg.max_mem.max_dim.h = height;
        path_cfg.max_mem.bitrate = enc_cfg.bitrate * 1024;
        path_cfg.max_mem.enc_buf_ms = enc_cfg.buffer_size;
        path_cfg.max_mem.svc_layer = HD_SVC_4X;
        path_cfg.max_mem.ltr = TRUE;
        path_cfg.max_mem.rotate = FALSE;
        path_cfg.max_mem.source_output = TRUE;
        path_cfg.isp_id = config_.sensor.vcap_id;

        ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &path_cfg);
        if (ret != HD_OK) {
            spdlog::error("SetStreamResolution: hd_videoenc_set(PATH_CONFIG) failed: {}", static_cast<int>(ret));
            return false;
        }
        video_streams_[stream_id].path_config_bitrate = path_cfg.max_mem.bitrate;
        video_streams_[stream_id].path_config_max_dim = path_cfg.max_mem.max_dim;
    } else {
        spdlog::debug("SetStreamResolution: Skipping PATH_CONFIG ({}x{} within max {}x{})",
                      width, height, path_max_dim.w, path_max_dim.h);
    }

    HD_VIDEOENC_IN enc_in;
    memset(&enc_in, 0, sizeof(enc_in));
    enc_in.dir = HD_VIDEO_DIR_NONE;
    enc_in.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
    enc_in.dim.w = width;
    enc_in.dim.h = height;
    enc_in.frc = HD_VIDEO_FRC_RATIO(1, 1);

    ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_IN, &enc_in);
    if (ret != HD_OK) {
        spdlog::error("SetStreamResolution: hd_videoenc_set(IN) failed: {}", static_cast<int>(ret));
        return false;
    }

    HD_VIDEOENC_OUT enc_out;
    memset(&enc_out, 0, sizeof(enc_out));
    enc_out.codec_type = static_cast<HD_VIDEO_CODEC>(GetCodecType(enc_cfg.codec));

    if (enc_cfg.codec == "h265" || enc_cfg.codec == "hevc") {
        enc_out.h26x.profile = static_cast<HD_VIDEOENC_PROFILE>(GetProfileType(enc_cfg.codec, enc_cfg.profile));
        enc_out.h26x.level_idc = static_cast<HD_VIDEOENC_LEVEL>(GetLevelType(enc_cfg.codec, enc_cfg.level));
        enc_out.h26x.gop_num = enc_cfg.gop;
        enc_out.h26x.ltr_interval = 0;
        enc_out.h26x.ltr_pre_ref = 0;
        enc_out.h26x.gray_en = 0;
        enc_out.h26x.source_output = 1;
        enc_out.h26x.svc_layer = HD_SVC_DISABLE;
        enc_out.h26x.entropy_mode = HD_H265E_CABAC_CODING;
    } else if (enc_cfg.codec == "h264" || enc_cfg.codec == "avc") {
        enc_out.h26x.profile = static_cast<HD_VIDEOENC_PROFILE>(GetProfileType(enc_cfg.codec, enc_cfg.profile));
        enc_out.h26x.level_idc = static_cast<HD_VIDEOENC_LEVEL>(GetLevelType(enc_cfg.codec, enc_cfg.level));
        enc_out.h26x.gop_num = enc_cfg.gop;
        enc_out.h26x.ltr_interval = 0;
        enc_out.h26x.ltr_pre_ref = 0;
        enc_out.h26x.gray_en = 0;
        enc_out.h26x.source_output = 1;
        enc_out.h26x.svc_layer = HD_SVC_DISABLE;
        enc_out.h26x.entropy_mode = (enc_cfg.entropy_mode == "cabac") ?
                                     HD_H264E_CABAC_CODING : HD_H264E_CAVLC_CODING;
    } else if (enc_cfg.codec == "jpeg" || enc_cfg.codec == "mjpeg") {
        enc_out.jpeg.retstart_interval = 0;
        enc_out.jpeg.image_quality = 80;
    }

    ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &enc_out);
    if (ret != HD_OK) {
        spdlog::error("SetStreamResolution: hd_videoenc_set(ENC_PARAM) failed: {}", static_cast<int>(ret));
        return false;
    }

    if (enc_cfg.codec != "jpeg" && enc_cfg.codec != "mjpeg") {
        HD_H26XENC_RATE_CONTROL rc;
        memset(&rc, 0, sizeof(rc));

        int rc_mode_int = GetRcMode(enc_cfg.rc_mode);
        rc.rc_mode = static_cast<HD_VIDEOENC_RC_MODE>(rc_mode_int);

        if (rc.rc_mode == HD_RC_MODE_CBR) {
            rc.cbr.bitrate = enc_cfg.bitrate * 1024;
            rc.cbr.frame_rate_base = config_.video_streams[stream_id].fps;
            rc.cbr.frame_rate_incr = 1;
            rc.cbr.init_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
            rc.cbr.min_i_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 10;
            rc.cbr.max_i_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp : 45;
            rc.cbr.init_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
            rc.cbr.min_p_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 10;
            int max_p_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp + 3 : 48;
            rc.cbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
            rc.cbr.static_time = 4;
            rc.cbr.ip_weight = 0;
        } else if (rc.rc_mode == HD_RC_MODE_VBR) {
            rc.vbr.bitrate = enc_cfg.bitrate * 1024;
            rc.vbr.frame_rate_base = config_.video_streams[stream_id].fps;
            rc.vbr.frame_rate_incr = 1;
            rc.vbr.init_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
            rc.vbr.min_i_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 10;
            rc.vbr.max_i_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp : 45;
            rc.vbr.init_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
            rc.vbr.min_p_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 10;
            int max_p_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp + 3 : 48;
            rc.vbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
        } else if (rc.rc_mode == HD_RC_MODE_FIX_QP) {
            rc.fixqp.fix_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
            rc.fixqp.fix_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
            spdlog::warn("SetStreamResolution: Stream {} in FIXQP mode", stream_id);
        } else {
            spdlog::warn("SetStreamResolution: Unsupported rc_mode '{}' for stream {}", enc_cfg.rc_mode, stream_id);
        }

        ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc);
        if (ret != HD_OK) {
            spdlog::error("SetStreamResolution: hd_videoenc_set(RATE_CONTROL) failed: {}", static_cast<int>(ret));
            return false;
        }
    }

    video_streams_[stream_id].enc_dim = enc_in.dim;

    // Restart videoproc (was stopped before encoder reconfigure)
    if (state_ == PipelineState::kRunning && video_streams_[stream_id].proc_path) {
        hd_videoproc_start(video_streams_[stream_id].proc_path);
    }

    if (was_running) {
        spdlog::debug("SetStreamResolution: Restarting encoder...");
        ret = hd_videoenc_start(enc_path);
        if (ret != HD_OK) {
            spdlog::error("SetStreamResolution: Failed to restart encoder: {}", static_cast<int>(ret));
            video_streams_[stream_id].encoder_running = false;
            return false;
        }
        video_streams_[stream_id].encoder_running = true;

        HD_H26XENC_REQUEST_IFRAME req;
        memset(&req, 0, sizeof(req));
        req.enable = TRUE;
        hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, &req);
    }

    spdlog::info("✓ Stream {} resolution updated to {}x{}", stream_id, width, height);
    return true;
#else
    config_.video_streams[stream_id].width = width;
    config_.video_streams[stream_id].height = height;
    return true;
#endif
}

bool HdalPipeline::SetStreamGop(int stream_id, int gop) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("Invalid stream_id: {}", stream_id);
        return false;
    }
    
    if (gop <= 0 || gop > 600) {
        spdlog::error("Invalid GOP: {}", gop);
        return false;
    }
    
#if HDAL_PIPELINE_ENABLED
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);
    
    if (!video_streams_[stream_id].enabled || !video_streams_[stream_id].enc_path) {
        return false;
    }
    
    HD_PATH_ID enc_path = video_streams_[stream_id].enc_path;
    
    spdlog::info("Setting stream {} GOP to {}, enc_path=0x{:x}", stream_id, gop, enc_path);
    
    // =========================================================================
    // IMPORTANT: Novatek SDK requires STOP -> SET -> START for GOP changes
    // The ENC_PARAM2 API does NOT work while encoder is running for this param
    // =========================================================================
    
    // Step 1: STOP the encoder (only if currently running)
    bool was_running = video_streams_[stream_id].encoder_running;
    HD_RESULT ret = HD_OK;
    
    if (was_running) {
        spdlog::debug("SetStreamGop: Stopping encoder for GOP change...");
        ret = hd_videoenc_stop(enc_path);
        if (ret != HD_OK) {
            // HD error -26 means encoder was not started - this is acceptable
            if (ret == -26) {
                spdlog::debug("SetStreamGop: Encoder was already stopped (ret=-26)");
                was_running = false;
            } else {
                spdlog::error("SetStreamGop: Failed to stop encoder: {}", static_cast<int>(ret));
                return false;
            }
        } else {
            video_streams_[stream_id].encoder_running = false;
        }
        // Small delay to ensure encoder fully stops
        usleep(30000);  // 30ms
    } else {
        spdlog::debug("SetStreamGop: Encoder not running, skipping stop");
    }
    
    // Step 2: Get current encoder parameters
    HD_VIDEOENC_OUT enc_out;
    memset(&enc_out, 0, sizeof(enc_out));
    
    ret = hd_videoenc_get(enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &enc_out);
    if (ret != HD_OK) {
        spdlog::error("Failed to get encoder params for stream {}: {}", stream_id, static_cast<int>(ret));
        // Try to restart encoder before returning (restore previous state)
        if (was_running) {
            if (hd_videoenc_start(enc_path) == HD_OK) {
                video_streams_[stream_id].encoder_running = true;
            }
        }
        return false;
    }
    
    // Step 3: Update GOP size in H26x config
    enc_out.h26x.gop_num = gop;
    
    // Apply encoder parameters (use ENC_PARAM since we're stopped)
    ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &enc_out);
    if (ret != HD_OK) {
        spdlog::error("Failed to set GOP for stream {}: {} (HD_RESULT)", stream_id, static_cast<int>(ret));
        // Try to restart encoder before returning (restore previous state)
        if (was_running) {
            if (hd_videoenc_start(enc_path) == HD_OK) {
                video_streams_[stream_id].encoder_running = true;
            }
        }
        return false;
    }
    
    // Step 4: START the encoder again
    spdlog::debug("SetStreamGop: Restarting encoder...");
    ret = hd_videoenc_start(enc_path);
    if (ret != HD_OK) {
        spdlog::error("SetStreamGop: Failed to restart encoder: {}", static_cast<int>(ret));
        video_streams_[stream_id].encoder_running = false;
        return false;
    }
    video_streams_[stream_id].encoder_running = true;
    
    // Small delay for encoder to stabilize
    usleep(20000);  // 20ms
    
    // Step 5: Request I-frame for clean stream transition
    HD_H26XENC_REQUEST_IFRAME req;
    memset(&req, 0, sizeof(req));
    req.enable = TRUE;
    
    ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, &req);
    if (ret != HD_OK) {
        spdlog::warn("SetStreamGop: Failed to request I-frame: {}", static_cast<int>(ret));
        // Continue anyway - GOP was changed and encoder is running
    }
    
    config_.video_streams[stream_id].encoder.gop = gop;
    spdlog::info("✓ Stream {} GOP successfully updated to {} (stop-set-start complete)", stream_id, gop);
    return true;
#else
    return false;
#endif
}

bool HdalPipeline::SetStreamQpRange(int stream_id, int min_qp, int max_qp) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("Invalid stream_id: {}", stream_id);
        return false;
    }
    
    if (min_qp < 0 || max_qp > 51 || min_qp >= max_qp) {
        spdlog::error("Invalid QP range: {}-{}", min_qp, max_qp);
        return false;
    }
    
#if HDAL_PIPELINE_ENABLED
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);
    
    if (!video_streams_[stream_id].enabled || !video_streams_[stream_id].enc_path) {
        return false;
    }
    
    // Update via rate control
    HD_H26XENC_RATE_CONTROL rc;
    memset(&rc, 0, sizeof(rc));
    
    const auto& enc_cfg = config_.video_streams[stream_id].encoder;
    int rc_mode_int = GetRcMode(enc_cfg.rc_mode);
    rc.rc_mode = static_cast<HD_VIDEOENC_RC_MODE>(rc_mode_int);
    
    if (rc.rc_mode == HD_RC_MODE_CBR) {
        rc.cbr.bitrate = enc_cfg.bitrate * 1024;
        rc.cbr.frame_rate_base = config_.video_streams[stream_id].fps;
        rc.cbr.frame_rate_incr = 1;
        rc.cbr.init_i_qp = (min_qp + max_qp) / 2;
        rc.cbr.max_i_qp = max_qp;
        rc.cbr.min_i_qp = min_qp;
        rc.cbr.init_p_qp = (min_qp + max_qp) / 2 + 2;
        int max_p_qp = max_qp + 3;
        rc.cbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
        rc.cbr.min_p_qp = min_qp;
    } else if (rc.rc_mode == HD_RC_MODE_VBR) {
        rc.vbr.bitrate = enc_cfg.bitrate * 1024;
        rc.vbr.frame_rate_base = config_.video_streams[stream_id].fps;
        rc.vbr.frame_rate_incr = 1;
        rc.vbr.init_i_qp = (min_qp + max_qp) / 2;
        rc.vbr.max_i_qp = max_qp;
        rc.vbr.min_i_qp = min_qp;
        rc.vbr.init_p_qp = (min_qp + max_qp) / 2 + 2;
        int max_p_qp = max_qp + 3;
        rc.vbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
        rc.vbr.min_p_qp = min_qp;
    } else {
        spdlog::warn("Cannot set QP range in {} mode for stream {}", enc_cfg.rc_mode, stream_id);
        return false;
    }
    
    // Use RATE_CONTROL2 for dynamic runtime change (works while encoder is running)
    HD_RESULT ret = hd_videoenc_set(video_streams_[stream_id].enc_path, 
                                     HD_VIDEOENC_PARAM_OUT_RATE_CONTROL2, &rc);
    if (ret != HD_OK) {
        spdlog::error("Failed to set QP range for stream {}: {}", stream_id, static_cast<int>(ret));
        return false;
    }
    
    config_.video_streams[stream_id].encoder.min_qp = min_qp;
    config_.video_streams[stream_id].encoder.max_qp = max_qp;
    spdlog::info("Stream {} QP range set to {}-{}", stream_id, min_qp, max_qp);
    return true;
#else
    return false;
#endif
}

bool HdalPipeline::SetStreamCodec(int stream_id, const std::string& codec, const std::string& profile) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("SetStreamCodec: Invalid stream_id: {}", stream_id);
        return false;
    }
    
#if HDAL_PIPELINE_ENABLED
    // Map codec string to HD_VIDEO_CODEC
    HD_VIDEO_CODEC new_codec_type;
    if (codec == "H264" || codec == "h264") {
        new_codec_type = HD_CODEC_TYPE_H264;
    } else if (codec == "H265" || codec == "h265" || codec == "HEVC" || codec == "hevc") {
        new_codec_type = HD_CODEC_TYPE_H265;
    } else if (codec == "MJPEG" || codec == "mjpeg") {
        new_codec_type = HD_CODEC_TYPE_JPEG;
    } else {
        spdlog::error("SetStreamCodec: Unsupported codec: {}", codec);
        return false;
    }
    
    // Map profile string to HD_VIDEOENC_PROFILE
    HD_VIDEOENC_PROFILE enc_profile;
    if (new_codec_type == HD_CODEC_TYPE_H264) {
        // H.264 profiles
        if (profile == "baseline" || profile == "Baseline") {
            enc_profile = HD_H264E_BASELINE_PROFILE;
        } else if (profile == "main" || profile == "Main") {
            enc_profile = HD_H264E_MAIN_PROFILE;
        } else {
            // Default to high profile for H.264
            enc_profile = HD_H264E_HIGH_PROFILE;
        }
    } else if (new_codec_type == HD_CODEC_TYPE_H265) {
        // H.265 only has main profile
        enc_profile = HD_H265E_MAIN_PROFILE;
    } else {
        // MJPEG doesn't use profiles
        enc_profile = HD_H264E_HIGH_PROFILE;  // Placeholder
    }
    
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);
    
    if (!video_streams_[stream_id].enabled || !video_streams_[stream_id].enc_path) {
        spdlog::error("SetStreamCodec: Stream {} not enabled or no encoder path", stream_id);
        return false;
    }
    
    HD_PATH_ID enc_path = video_streams_[stream_id].enc_path;
    const auto& stream_cfg = config_.video_streams[stream_id];
    const auto& enc_cfg = stream_cfg.encoder;
    
    spdlog::info("SetStreamCodec: Changing stream {} codec from {} to {}, enc_path=0x{:x}", 
                 stream_id, enc_cfg.codec, codec, enc_path);
    
    // =========================================================================
    // IMPORTANT: Novatek SDK requires STOP -> SET -> START for codec/gop changes
    // The ENC_PARAM2 API does NOT work while encoder is running for these params
    // =========================================================================
    
    // Step 1: STOP the encoder (only if currently running)
    bool was_running = video_streams_[stream_id].encoder_running;
    HD_RESULT ret = HD_OK;
    
    if (was_running) {
        spdlog::info("SetStreamCodec: Stopping encoder for codec change...");
        ret = hd_videoenc_stop(enc_path);
        if (ret != HD_OK) {
            // HD error -26 means encoder was not started - this is acceptable
            if (ret == -26) {
                spdlog::debug("SetStreamCodec: Encoder was already stopped (ret=-26)");
                was_running = false;
            } else {
                spdlog::error("SetStreamCodec: Failed to stop encoder: {}", static_cast<int>(ret));
                return false;
            }
        } else {
            video_streams_[stream_id].encoder_running = false;
        }
        // Small delay to ensure encoder fully stops
        usleep(50000);  // 50ms
    } else {
        spdlog::debug("SetStreamCodec: Encoder not running, skipping stop");
    }
    
    // Step 2: Prepare and SET new encoder parameters
    HD_VIDEOENC_OUT enc_out;
    memset(&enc_out, 0, sizeof(enc_out));
    
    // Get current parameters first
    ret = hd_videoenc_get(enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &enc_out);
    if (ret != HD_OK) {
        spdlog::error("SetStreamCodec: Failed to get current encoder params: {}", static_cast<int>(ret));
        // Try to restart encoder before returning (restore previous state)
        if (was_running) {
            if (hd_videoenc_start(enc_path) == HD_OK) {
                video_streams_[stream_id].encoder_running = true;
            }
        }
        return false;
    }
    
    // Set new codec type
    enc_out.codec_type = new_codec_type;
    
    // Configure codec-specific parameters
    if (new_codec_type == HD_CODEC_TYPE_H264 || new_codec_type == HD_CODEC_TYPE_H265) {
        // H.264/H.265 parameters
        enc_out.h26x.gop_num = enc_cfg.gop;
        enc_out.h26x.entropy_mode = (new_codec_type == HD_CODEC_TYPE_H264) ? 
                                     HD_H264E_CABAC_CODING : HD_H265E_CABAC_CODING;
        enc_out.h26x.profile = enc_profile;
        
        // CRITICAL: Set appropriate level for the new codec
        // H.264 and H.265 use different level numbering systems
        if (new_codec_type == HD_CODEC_TYPE_H264) {
            // For 2944x1664 @ 30fps, need at least Level 4.1 (supports 2048x1024@30fps max)
            // Level 5.1 supports up to 4096x2160@30fps
            enc_out.h26x.level_idc = HD_H264E_LEVEL_5_1;  // Level 5.1 for high resolution
        } else {
            // H.265 level
            enc_out.h26x.level_idc = HD_H265E_LEVEL_5;  // Level 5.0 for H.265
        }
    } else {
        // MJPEG parameters
        enc_out.jpeg.image_quality = 80;  // Default quality
        enc_out.jpeg.retstart_interval = 0;
    }
    
    // Apply encoder parameters (use ENC_PARAM since we're stopped)
    ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &enc_out);
    if (ret != HD_OK) {
        spdlog::error("SetStreamCodec: Failed to set encoder params: {} (HD_RESULT)", static_cast<int>(ret));
        // Try to restart encoder before returning (restore previous state)
        if (was_running) {
            if (hd_videoenc_start(enc_path) == HD_OK) {
                video_streams_[stream_id].encoder_running = true;
            }
        }
        return false;
    }
    spdlog::debug("SetStreamCodec: Encoder params updated successfully");
    
    // Step 3: Update rate control (still stopped)
    // Note: Rate control must be re-applied after codec change
    if (new_codec_type == HD_CODEC_TYPE_H264 || new_codec_type == HD_CODEC_TYPE_H265) {
        HD_H26XENC_RATE_CONTROL rc;
        memset(&rc, 0, sizeof(rc));
        
        int rc_mode_int = GetRcMode(enc_cfg.rc_mode);
        rc.rc_mode = static_cast<HD_VIDEOENC_RC_MODE>(rc_mode_int);
        
        if (rc.rc_mode == HD_RC_MODE_CBR) {
            rc.cbr.bitrate = enc_cfg.bitrate * 1024;
            rc.cbr.frame_rate_base = stream_cfg.fps;
            rc.cbr.frame_rate_incr = 1;
            rc.cbr.init_i_qp = 30;
            rc.cbr.max_i_qp = enc_cfg.max_qp;
            rc.cbr.min_i_qp = enc_cfg.min_qp;
            rc.cbr.init_p_qp = 32;
            int max_p_qp = enc_cfg.max_qp + 3;
            rc.cbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
            rc.cbr.min_p_qp = enc_cfg.min_qp;
            rc.cbr.ip_weight = 8;
        } else if (rc.rc_mode == HD_RC_MODE_VBR) {
            rc.vbr.bitrate = enc_cfg.bitrate * 1024;
            rc.vbr.frame_rate_base = stream_cfg.fps;
            rc.vbr.frame_rate_incr = 1;
            rc.vbr.init_i_qp = 30;
            rc.vbr.max_i_qp = enc_cfg.max_qp;
            rc.vbr.min_i_qp = enc_cfg.min_qp;
            rc.vbr.init_p_qp = 32;
            int max_p_qp = enc_cfg.max_qp + 3;
            rc.vbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
            rc.vbr.min_p_qp = enc_cfg.min_qp;
            rc.vbr.ip_weight = 8;
        }
        
        ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc);
        if (ret != HD_OK) {
            spdlog::error("SetStreamCodec: Failed to set rate control: {} (HD_RESULT)", static_cast<int>(ret));
            // Continue anyway - encoder params were set, restart encoder
        } else {
            spdlog::debug("SetStreamCodec: Rate control updated successfully");
        }
    }
    
    // Step 4: START the encoder again
    spdlog::info("SetStreamCodec: Restarting encoder...");
    ret = hd_videoenc_start(enc_path);
    if (ret != HD_OK) {
        spdlog::error("SetStreamCodec: Failed to restart encoder: {}", static_cast<int>(ret));
        video_streams_[stream_id].encoder_running = false;
        return false;
    }
    video_streams_[stream_id].encoder_running = true;
    
    // Small delay for encoder to stabilize
    usleep(20000);  // 20ms
    
    // Step 5: Request I-frame for clean stream transition
    // This ensures downstream clients get a clean key frame with the new codec
    HD_H26XENC_REQUEST_IFRAME req;
    memset(&req, 0, sizeof(req));
    req.enable = TRUE;
    
    ret = hd_videoenc_set(enc_path, HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, &req);
    if (ret != HD_OK) {
        spdlog::warn("SetStreamCodec: Failed to request I-frame after codec change: {}", static_cast<int>(ret));
        // Continue anyway - codec was changed and encoder is running
    } else {
        spdlog::debug("SetStreamCodec: I-frame requested for clean transition");
    }
    
    // Update local config
    config_.video_streams[stream_id].encoder.codec = codec;
    
    spdlog::info("✓ Stream {} codec successfully changed to {} (stop-set-start complete)", stream_id, codec);
    return true;
#else
    return false;
#endif
}

std::string HdalPipeline::GetStreamCodec(int stream_id) const {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("GetStreamCodec: Invalid stream_id: {}", stream_id);
        return "";
    }
    return config_.video_streams[stream_id].encoder.codec;
}

bool HdalPipeline::ForceIdr(int stream_id) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("Invalid stream_id: {}", stream_id);
        return false;
    }
    
#if HDAL_PIPELINE_ENABLED
    std::lock_guard<std::mutex> lock(g_pipeline_mutex);
    
    if (!video_streams_[stream_id].enabled || !video_streams_[stream_id].enc_path) {
        return false;
    }
    
    HD_H26XENC_REQUEST_IFRAME req;
    memset(&req, 0, sizeof(req));
    req.enable = TRUE;
    
    HD_RESULT ret = hd_videoenc_set(video_streams_[stream_id].enc_path, 
                                     HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, &req);
    if (ret == HD_OK) {
        spdlog::debug("Forced I-frame on stream {}", stream_id);
        return true;
    }
    return false;
#else
    return false;
#endif
}

// ============================================================================
// RTSP Server Integration
// ============================================================================

bool HdalPipeline::StartRtspServer() {
    if (rtsp_pid_ > 0) {
        spdlog::warn("RTSP server already running (pid={})", rtsp_pid_);
        return true;
    }
    
    spdlog::info("Starting RTSP server (nvtrtspd_ipc)...");
    
    rtsp_pid_ = fork();
    if (rtsp_pid_ == 0) {
        // Child process - try multiple locations for the RTSP daemon
        execl("/bin/nvtrtspd_ipc", "nvtrtspd_ipc", nullptr);
        execl("/usr/bin/nvtrtspd_ipc", "nvtrtspd_ipc", nullptr);
        execl("/mnt/app/ipcamera/nvtrtspd_ipc", "nvtrtspd_ipc", nullptr);
        execl("/mnt/app/ipcamera/bin/nvtrtspd_ipc", "nvtrtspd_ipc", nullptr);
        spdlog::error("Failed to exec nvtrtspd_ipc");
        _exit(1);
    } else if (rtsp_pid_ < 0) {
        spdlog::error("Failed to fork RTSP server: {}", strerror(errno));
        return false;
    }
    
    spdlog::info("RTSP server started (pid={})", rtsp_pid_);
    return true;
}

void HdalPipeline::StopRtspServer() {
    spdlog::info("Stopping RTSP server...");
    
    // First try to kill by tracked PID
    if (rtsp_pid_ > 0) {
        spdlog::info("Stopping RTSP server (pid={})...", rtsp_pid_);
        
        // Send SIGTERM
        kill(rtsp_pid_, SIGTERM);
        
        // Wait briefly for graceful exit
        usleep(100000);  // 100ms
        
        // Check if still running and send SIGKILL if needed
        int status;
        pid_t result = waitpid(rtsp_pid_, &status, WNOHANG);
        if (result == 0) {
            // Still running, force kill
            kill(rtsp_pid_, SIGKILL);
            waitpid(rtsp_pid_, &status, 0);
        }
        
        rtsp_pid_ = -1;
    }
    
    // Also kill any orphaned nvtrtspd_ipc processes (from previous crashes)
    int ret = system("pkill -9 nvtrtspd_ipc 2>/dev/null");
    (void)ret;  // Ignore result - process may not exist
    
    spdlog::info("RTSP server stopped");
}

// ============================================================================
// Statistics and Monitoring
// ============================================================================

HdalPipeline::PipelineStats HdalPipeline::GetStats() {
    PipelineStats stats = {};
    memset(&stats, 0, sizeof(stats));
    
    stats.running = (state_ == PipelineState::kRunning);
    
    // Note: Actual frame/byte statistics would require integration with
    // the streaming/recording modules that receive encoded frames.
    // For now, return basic state information.
    // The actual counts should be tracked by RTSP or recording modules.
    
    return stats;
}

int HdalPipeline::GetCurrentBitrate(int stream_id) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        return 0;
    }
    
    // Return configured bitrate
    // Actual measured bitrate would need periodic frame size tracking
    return config_.video_streams[stream_id].encoder.bitrate;
}

int HdalPipeline::GetCurrentFps(int stream_id) {
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        return 0;
    }
    
    // Return configured FPS
    // Actual measured FPS would need frame timing tracking
    return config_.video_streams[stream_id].fps;
}

} // namespace platform
} // namespace ipcam

// ============================================================================
// Convenience Functions (global namespace)
// ============================================================================

bool StartHdalPipeline() {
    auto& pipeline = ipcam::platform::HdalPipeline::Instance();
    
    if (!pipeline.LoadConfig()) {
        return false;
    }
    
    if (!pipeline.Init()) {
        return false;
    }
    
    if (!pipeline.Start()) {
        pipeline.Shutdown();
        return false;
    }
    
    return true;
}

void StopHdalPipeline() {
    auto& pipeline = ipcam::platform::HdalPipeline::Instance();
    pipeline.Stop();
    pipeline.Shutdown();
}
