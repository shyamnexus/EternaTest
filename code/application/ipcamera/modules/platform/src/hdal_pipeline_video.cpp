/**
 * @file hdal_pipeline_video.cpp
 * @brief HDAL Pipeline Video Capture, Processing, and Encoding
 */

#include "hdal_pipeline_common.h"
#include <algorithm>

namespace ipcam {
namespace platform {

// ============================================================================
// Video Path Opening (from hd_video_record_with_2path.c)
// ============================================================================

bool HdalPipeline::OpenVideoPaths() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Opening HDAL video paths...");
    
    HD_RESULT ret;
    int vcap_id = config_.sensor.vcap_id;
    
    // ========================================================================
    // Configure Video Capture Controller
    // ========================================================================
    
    HD_VIDEOCAP_DRV_CONFIG cap_cfg;
    memset(&cap_cfg, 0, sizeof(cap_cfg));
    strncpy(cap_cfg.sen_cfg.sen_dev.driver_name, 
            config_.sensor.driver_name.c_str(), 
            HD_VIDEOCAP_SEN_NAME_LEN - 1);
    
    cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = config_.sensor.pinmux;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
    
    // Configure data lanes (GC5603 uses 2 lanes)
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
    for (int i = 2; i < 8; i++) {
        cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = HD_VIDEOCAP_SEN_IGNORE;
    }
    
    // Open capture control path
    ret = hd_videocap_open(static_cast<HD_IN_ID>(0), static_cast<HD_OUT_ID>(HD_VIDEOCAP_CTRL(vcap_id)), &video_streams_[0].cap_ctrl);
    if (ret != HD_OK) {
        spdlog::error("hd_videocap_open(CTRL) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    ret = hd_videocap_set(video_streams_[0].cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
    if (ret != HD_OK) {
        spdlog::error("hd_videocap_set(DRV_CONFIG) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Enable AE/AWB functions
    HD_VIDEOCAP_CTRL iq_ctrl;
    memset(&iq_ctrl, 0, sizeof(iq_ctrl));
    UINT32 cap_func = 0;
    if (config_.videoproc.ae_enabled) cap_func |= HD_VIDEOCAP_FUNC_AE;
    if (config_.videoproc.awb_enabled) cap_func |= HD_VIDEOCAP_FUNC_AWB;
    iq_ctrl.func = static_cast<HD_VIDEOCAP_CTRLFUNC>(cap_func);
    
    ret = hd_videocap_set(video_streams_[0].cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctrl);
    if (ret != HD_OK) {
        spdlog::warn("hd_videocap_set(CTRL) failed: {}", static_cast<int>(ret));
    }
    
    // Set data lanes if needed
    if (config_.sensor.data_lanes == 2) {
        UINT32 data_lane = 2;
        vendor_videocap_set(video_streams_[0].cap_ctrl, VENDOR_VIDEOCAP_PARAM_DATA_LANE, &data_lane);
    }
    
    // ========================================================================
    // Configure Video Processing Controller
    // ========================================================================
    
    HD_VIDEOPROC_DEV_CONFIG proc_cfg;
    memset(&proc_cfg, 0, sizeof(proc_cfg));
    proc_cfg.pipe = HD_VIDEOPROC_PIPE_RAWALL;
    proc_cfg.isp_id = vcap_id;
    
    // Set max dimensions
    const int sensor_max_w = config_.sensor.crop.enabled ? config_.sensor.crop.width : config_.sensor.native_width;
    const int sensor_max_h = config_.sensor.crop.enabled ? config_.sensor.crop.height : config_.sensor.native_height;

    // Allocate video proc for full sensor output (e.g., GC5603 2944x1664) even if current stream is smaller.
    const int main_w = sensor_max_w;
    const int main_h = sensor_max_h;
    
    proc_cfg.in_max.dim.w = main_w;
    proc_cfg.in_max.dim.h = main_h;
    proc_cfg.in_max.pxlfmt = HD_VIDEO_PXLFMT_RAW10;
    proc_cfg.in_max.frc = HD_VIDEO_FRC_RATIO(1, 1);
    
    // Set processing functions
    UINT32 proc_func = 0;
    if (config_.videoproc.wdr_enabled) proc_func |= HD_VIDEOPROC_FUNC_WDR;
    if (config_.videoproc.defog_enabled) proc_func |= HD_VIDEOPROC_FUNC_DEFOG;
    if (config_.videoproc.nr_3d_enabled) proc_func |= HD_VIDEOPROC_FUNC_3DNR;
    if (config_.videoproc.af_enabled) proc_func |= HD_VIDEOPROC_FUNC_AF;
    proc_cfg.ctrl_max.func = static_cast<HD_VIDEOPROC_CTRLFUNC>(proc_func);
    
    // Open video processing control path
    ret = hd_videoproc_open(static_cast<HD_IN_ID>(0), static_cast<HD_OUT_ID>(HD_VIDEOPROC_0_CTRL), &video_streams_[0].proc_ctrl);
    if (ret != HD_OK) {
        spdlog::error("hd_videoproc_open(CTRL) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    ret = hd_videoproc_set(video_streams_[0].proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &proc_cfg);
    if (ret != HD_OK) {
        spdlog::error("hd_videoproc_set(DEV_CONFIG) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Set video processing control params
    HD_VIDEOPROC_CTRL proc_ctrl_param;
    memset(&proc_ctrl_param, 0, sizeof(proc_ctrl_param));
    proc_ctrl_param.func = proc_cfg.ctrl_max.func;
    proc_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;  // Use main output as 3DNR reference
    
    ret = hd_videoproc_set(video_streams_[0].proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &proc_ctrl_param);
    if (ret != HD_OK) {
        spdlog::warn("hd_videoproc_set(CTRL) failed: {}", static_cast<int>(ret));
    }
    
    // ========================================================================
    // Open Capture Path (main stream - stream 0)
    // ========================================================================
    
    ret = hd_videocap_open(static_cast<HD_IN_ID>(HD_VIDEOCAP_IN(vcap_id, 0)), 
                           static_cast<HD_OUT_ID>(HD_VIDEOCAP_OUT(vcap_id, 0)), 
                           &video_streams_[0].cap_path);
    if (ret != HD_OK) {
        spdlog::error("hd_videocap_open(path) failed: {}", static_cast<int>(ret));
        return false;
    }
    video_streams_[0].enabled = true;
    
    // ========================================================================
    // Open dedicated 3DNR reference path (OUT_0 at full sensor resolution)
    // OUT_0 must match the capture input size for 3DNR to work.  We keep
    // it permanently at full resolution and never bind it to an encoder.
    // User streams are shifted to OUT_1..OUT_3 so they can be freely resized.
    // ========================================================================
    
    ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &dnr_ref_proc_path_);
    if (ret != HD_OK) {
        spdlog::error("hd_videoproc_open(3DNR ref OUT_0) failed: {}", static_cast<int>(ret));
        return false;
    }
    spdlog::info("Opened 3DNR reference path (OUT_0) at full sensor resolution");
    
    // ========================================================================
    // Open Video Processing and Encoding Paths for all streams
    // Streams are shifted: stream 0 → OUT_1/ENC_0, stream 1 → OUT_2/ENC_1,
    //                       stream 2 → OUT_3/ENC_2.
    // OUT_0 is reserved for 3DNR reference (above).
    // ========================================================================
    
    const HD_IN_ID proc_in_ids[] = {
        HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_IN_0
    };
    const HD_OUT_ID proc_out_ids[] = {
        HD_VIDEOPROC_0_OUT_1, HD_VIDEOPROC_0_OUT_2, HD_VIDEOPROC_0_OUT_3
    };
    const HD_IN_ID enc_in_ids[] = {
        HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_IN_2
    };
    const HD_OUT_ID enc_out_ids[] = {
        HD_VIDEOENC_0_OUT_0, HD_VIDEOENC_0_OUT_1, HD_VIDEOENC_0_OUT_2
    };
    
    // Only 3 user streams possible when OUT_0 is reserved for 3DNR
    const int max_user_streams = std::min(kMaxVideoStreams, 3);
    
    for (int i = 0; i < max_user_streams; i++) {
        if (!config_.video_streams[i].enabled) {
            video_streams_[i].enabled = false;
            continue;
        }
        
        // Open video processing path (shifted: stream i uses OUT_(i+1))
        ret = hd_videoproc_open(proc_in_ids[i], proc_out_ids[i], &video_streams_[i].proc_path);
        if (ret != HD_OK) {
            spdlog::error("hd_videoproc_open(stream {} -> OUT_{}) failed: {}", i, i + 1, static_cast<int>(ret));
            return false;
        }
        
        // Open video encoding path
        ret = hd_videoenc_open(enc_in_ids[i], enc_out_ids[i], &video_streams_[i].enc_path);
        if (ret != HD_OK) {
            spdlog::error("hd_videoenc_open(stream {} -> ENC_{}) failed: {}", i, i, static_cast<int>(ret));
            return false;
        }
        
        video_streams_[i].enabled = true;
        spdlog::debug("Opened video stream {} (proc OUT_{}, enc {}): {}x{}", i, i + 1, i,
                      config_.video_streams[i].width, config_.video_streams[i].height);
    }
    
    // Disable stream 3 (index 3) since OUT_3 is used by stream 2 and there is no OUT_4
    if (kMaxVideoStreams > 3) {
        video_streams_[3].enabled = false;
        spdlog::info("Stream 3 disabled (OUT_0 reserved for 3DNR, only 3 user streams available)");
    }
    
    spdlog::info("HDAL video paths opened (OUT_0=3DNR ref, OUT_1-3=user streams)");
    return true;
#else
    return true;
#endif
}

// ============================================================================
// Configure Capture (from pq_video_rtsp.c)
// ============================================================================

bool HdalPipeline::ConfigureCapture() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Configuring video capture...");
    
    HD_RESULT ret;
    const int sensor_max_w = config_.sensor.crop.enabled ? config_.sensor.crop.width : config_.sensor.native_width;
    const int sensor_max_h = config_.sensor.crop.enabled ? config_.sensor.crop.height : config_.sensor.native_height;

    // Drive capture using full sensor output; downstream scaling handles stream resolutions.
    const int main_w = sensor_max_w;
    const int main_h = sensor_max_h;
    
    // Set capture input parameters
    HD_VIDEOCAP_IN video_in;
    memset(&video_in, 0, sizeof(video_in));
    video_in.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
    video_in.frc = HD_VIDEO_FRC_RATIO(config_.video_streams[0].fps, 1);
    video_in.dim.w = main_w;
    video_in.dim.h = main_h;
    video_in.pxlfmt = HD_VIDEO_PXLFMT_RAW10;
    video_in.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
    
    ret = hd_videocap_set(video_streams_[0].cap_path, HD_VIDEOCAP_PARAM_IN, &video_in);
    if (ret != HD_OK) {
        spdlog::error("hd_videocap_set(IN) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Set crop if enabled (GC5603: crop 2960->2944 for BNR 32-alignment)
    if (config_.sensor.crop.enabled) {
        HD_VIDEOCAP_CROP crop;
        memset(&crop, 0, sizeof(crop));
        crop.mode = HD_CROP_ON;
        crop.win.rect.x = config_.sensor.crop.x;
        crop.win.rect.y = config_.sensor.crop.y;
        crop.win.rect.w = config_.sensor.crop.width;
        crop.win.rect.h = config_.sensor.crop.height;
        crop.align.w = 4;
        crop.align.h = 4;
        
        ret = hd_videocap_set(video_streams_[0].cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &crop);
        if (ret != HD_OK) {
            spdlog::warn("hd_videocap_set(CROP) failed: {}", static_cast<int>(ret));
        }
        spdlog::debug("Capture crop: x={} y={} w={} h={}", 
                      crop.win.rect.x, crop.win.rect.y, crop.win.rect.w, crop.win.rect.h);
    } else {
        HD_VIDEOCAP_CROP crop;
        memset(&crop, 0, sizeof(crop));
        crop.mode = HD_CROP_OFF;
        hd_videocap_set(video_streams_[0].cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &crop);
    }
    
    // Set capture output parameters
    HD_VIDEOCAP_OUT video_out;
    memset(&video_out, 0, sizeof(video_out));
    video_out.pxlfmt = HD_VIDEO_PXLFMT_RAW10;
    video_out.dir = HD_VIDEO_DIR_NONE;
    
    ret = hd_videocap_set(video_streams_[0].cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out);
    if (ret != HD_OK) {
        spdlog::error("hd_videocap_set(OUT) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Get capture capabilities
    ret = hd_videocap_get(video_streams_[0].cap_ctrl, HD_VIDEOCAP_PARAM_SYSCAPS, &cap_syscaps_);
    if (ret == HD_OK) {
        spdlog::debug("Capture caps: max_dim={}x{}", 
                      cap_syscaps_.max_dim.w, cap_syscaps_.max_dim.h);
    }
    
    video_streams_[0].cap_dim.w = main_w;
    video_streams_[0].cap_dim.h = main_h;
    
    spdlog::info("Video capture configured: {}x{}@{}fps", main_w, main_h, config_.video_streams[0].fps);
    return true;
#else
    return true;
#endif
}

// ============================================================================
// Configure Video Processing
// ============================================================================

bool HdalPipeline::ConfigureVideoProc() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Configuring video processing...");
    
    HD_RESULT ret;
    
    // -----------------------------------------------------------------------
    // Configure dedicated 3DNR reference path (OUT_0) at full sensor resolution.
    // This path is never bound to an encoder — it only provides 3DNR with a
    // full-resolution YUV frame that matches the capture input size.
    // -----------------------------------------------------------------------
    if (dnr_ref_proc_path_) {
        const int sensor_w = config_.sensor.crop.enabled ? config_.sensor.crop.width : config_.sensor.native_width;
        const int sensor_h = config_.sensor.crop.enabled ? config_.sensor.crop.height : config_.sensor.native_height;
        
        HD_VIDEOPROC_OUT dnr_out;
        memset(&dnr_out, 0, sizeof(dnr_out));
        dnr_out.func = 0;
        dnr_out.dim.w = sensor_w;
        dnr_out.dim.h = sensor_h;
        dnr_out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
        dnr_out.dir = HD_VIDEO_DIR_NONE;
        dnr_out.frc = HD_VIDEO_FRC_RATIO(1, 1);
        
        ret = hd_videoproc_set(dnr_ref_proc_path_, HD_VIDEOPROC_PARAM_OUT, &dnr_out);
        if (ret != HD_OK) {
            spdlog::error("hd_videoproc_set(OUT, 3DNR ref) failed: {}", static_cast<int>(ret));
            return false;
        }
        spdlog::info("3DNR reference path configured: OUT_0 @ {}x{}", sensor_w, sensor_h);
    }
    
    // -----------------------------------------------------------------------
    // Configure user stream paths (OUT_1..OUT_3) at their requested resolution
    // -----------------------------------------------------------------------
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (!video_streams_[i].enabled) continue;
        
        HD_VIDEOPROC_OUT proc_out;
        memset(&proc_out, 0, sizeof(proc_out));
        proc_out.func = 0;
        proc_out.dim.w = config_.video_streams[i].width;
        proc_out.dim.h = config_.video_streams[i].height;
        proc_out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
        proc_out.dir = HD_VIDEO_DIR_NONE;
        proc_out.frc = HD_VIDEO_FRC_RATIO(1, 1);
        
        ret = hd_videoproc_set(video_streams_[i].proc_path, HD_VIDEOPROC_PARAM_OUT, &proc_out);
        if (ret != HD_OK) {
            spdlog::error("hd_videoproc_set(OUT, stream {}) failed: {}", i, static_cast<int>(ret));
            return false;
        }
        
        video_streams_[i].proc_dim = proc_out.dim;
        spdlog::debug("VideoProc stream {} (OUT_{}): {}x{}", i, i + 1, proc_out.dim.w, proc_out.dim.h);
    }
    
    spdlog::info("Video processing configured (3DNR ref + {} user streams)", kMaxVideoStreams);
    return true;
#else
    return true;
#endif
}

// ============================================================================
// Configure Encoders (from hd_video_record_with_2path.c)
// ============================================================================

bool HdalPipeline::ConfigureEncoders() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Configuring video encoders...");
    
    HD_RESULT ret;
    
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (!video_streams_[i].enabled) continue;
        
        const auto& stream_cfg = config_.video_streams[i];
        const auto& enc_cfg = stream_cfg.encoder;
        
        // Set encoder path config
        // IMPORTANT: Always use H.264 for max_mem allocation to support dynamic codec switching
        // H.264 requires more memory than H.265, so allocating for H.264 ensures we can switch to any codec
        HD_VIDEOENC_PATH_CONFIG path_cfg;
        memset(&path_cfg, 0, sizeof(path_cfg));
        path_cfg.max_mem.codec_type = HD_CODEC_TYPE_H264;  // Always H.264 for max resources

        // Allocate encoder buffers at least as large as the full sensor crop (e.g., GC5603 2944x1664)
        const int sensor_max_w = config_.sensor.crop.enabled ? config_.sensor.crop.width : config_.sensor.native_width;
        const int sensor_max_h = config_.sensor.crop.enabled ? config_.sensor.crop.height : config_.sensor.native_height;
        path_cfg.max_mem.max_dim.w = std::max(stream_cfg.width, sensor_max_w);
        path_cfg.max_mem.max_dim.h = std::max(stream_cfg.height, sensor_max_h);
        // Reserve encoder buffers for the largest bitrate the stream may request.
        const int max_cfg_bitrate_kbps = std::max(enc_cfg.bitrate, enc_cfg.max_bitrate);
        path_cfg.max_mem.bitrate = max_cfg_bitrate_kbps * 1024;  // kbps to bps
        path_cfg.max_mem.enc_buf_ms = enc_cfg.buffer_size;
        path_cfg.max_mem.svc_layer = HD_SVC_4X;
        path_cfg.max_mem.ltr = TRUE;
        path_cfg.max_mem.rotate = FALSE;
        path_cfg.max_mem.source_output = TRUE;  // Enable snapshot support via source output
        path_cfg.isp_id = config_.sensor.vcap_id;
        
        spdlog::info("ConfigureEncoders: Stream {} PathConfig MaxMem.source_output=TRUE", i);

        ret = hd_videoenc_set(video_streams_[i].enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &path_cfg);
        if (ret != HD_OK) {
            spdlog::error("hd_videoenc_set(PATH_CONFIG, stream {}) failed: {}", i, static_cast<int>(ret));
            return false;
        }
        
        // Track the configured PATH_CONFIG bitrate for runtime updates
        video_streams_[i].path_config_bitrate = path_cfg.max_mem.bitrate;
        video_streams_[i].path_config_max_dim = path_cfg.max_mem.max_dim;
        
        // Set encoder input parameters
        HD_VIDEOENC_IN enc_in;
        memset(&enc_in, 0, sizeof(enc_in));
        enc_in.dir = HD_VIDEO_DIR_NONE;
        enc_in.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
        enc_in.dim.w = stream_cfg.width;
        enc_in.dim.h = stream_cfg.height;
        enc_in.frc = HD_VIDEO_FRC_RATIO(1, 1);
        
        ret = hd_videoenc_set(video_streams_[i].enc_path, HD_VIDEOENC_PARAM_IN, &enc_in);
        if (ret != HD_OK) {
            spdlog::error("hd_videoenc_set(IN, stream {}) failed: {}", i, static_cast<int>(ret));
            return false;
        }
        
        // Set encoder output parameters
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
            enc_out.h26x.source_output = 1;  // Enable snapshot source output
            enc_out.h26x.svc_layer = HD_SVC_DISABLE;
            enc_out.h26x.entropy_mode = HD_H265E_CABAC_CODING;
        } else if (enc_cfg.codec == "h264" || enc_cfg.codec == "avc") {
            enc_out.h26x.profile = static_cast<HD_VIDEOENC_PROFILE>(GetProfileType(enc_cfg.codec, enc_cfg.profile));
            enc_out.h26x.level_idc = static_cast<HD_VIDEOENC_LEVEL>(GetLevelType(enc_cfg.codec, enc_cfg.level));
            enc_out.h26x.gop_num = enc_cfg.gop;
            enc_out.h26x.ltr_interval = 0;
            enc_out.h26x.ltr_pre_ref = 0;
            enc_out.h26x.gray_en = 0;
            enc_out.h26x.source_output = 1;  // Enable snapshot source output
            enc_out.h26x.svc_layer = HD_SVC_DISABLE;
            enc_out.h26x.entropy_mode = (enc_cfg.entropy_mode == "cabac") ? 
                                         HD_H264E_CABAC_CODING : HD_H264E_CAVLC_CODING;
        } else if (enc_cfg.codec == "jpeg" || enc_cfg.codec == "mjpeg") {
            enc_out.jpeg.retstart_interval = 0;
            enc_out.jpeg.image_quality = 80;
        }
        
        ret = hd_videoenc_set(video_streams_[i].enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &enc_out);
        if (ret != HD_OK) {
            spdlog::error("hd_videoenc_set(ENC_PARAM, stream {}) failed: {}", i, static_cast<int>(ret));
            return false;
        }
        
        // Set rate control
        if (enc_cfg.codec != "jpeg" && enc_cfg.codec != "mjpeg") {
            HD_H26XENC_RATE_CONTROL rc;
            memset(&rc, 0, sizeof(rc));
            
            int rc_mode_int = GetRcMode(enc_cfg.rc_mode);
            rc.rc_mode = static_cast<HD_VIDEOENC_RC_MODE>(rc_mode_int);
            
            spdlog::info("Stream {} encoder init: rc_mode='{}' -> HD_RC_MODE={}, bitrate={}kbps, fps={}", 
                         i, enc_cfg.rc_mode, rc_mode_int, enc_cfg.bitrate, stream_cfg.fps);
            
            // Fill the appropriate union member based on mode
            if (rc.rc_mode == HD_RC_MODE_CBR) {
                rc.cbr.bitrate = enc_cfg.bitrate * 1024;
                rc.cbr.frame_rate_base = stream_cfg.fps;
                rc.cbr.frame_rate_incr = 1;
                rc.cbr.init_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
                rc.cbr.min_i_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 10;
                rc.cbr.max_i_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp : 45;
                rc.cbr.init_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
                rc.cbr.min_p_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 10;
                // Ensure max_p_qp doesn't exceed 51
                int max_p_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp + 3 : 48;
                rc.cbr.max_p_qp = max_p_qp > 51 ? 51 : max_p_qp;
                rc.cbr.static_time = 4;
                rc.cbr.ip_weight = 0;
            } else if (rc.rc_mode == HD_RC_MODE_VBR) {
                rc.vbr.bitrate = enc_cfg.bitrate * 1024;
                rc.vbr.frame_rate_base = stream_cfg.fps;
                rc.vbr.frame_rate_incr = 1;
                rc.vbr.init_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
                rc.vbr.min_i_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 10;
                rc.vbr.max_i_qp = enc_cfg.max_qp > 0 ? enc_cfg.max_qp : 45;
                rc.vbr.init_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
                rc.vbr.min_p_qp = enc_cfg.min_qp > 0 ? enc_cfg.min_qp : 10;
                // Ensure max_p_qp doesn't exceed 51
                int max_p_qp_vbr = enc_cfg.max_qp > 0 ? enc_cfg.max_qp + 3 : 48;
                rc.vbr.max_p_qp = max_p_qp_vbr > 51 ? 51 : max_p_qp_vbr;
            } else if (rc.rc_mode == HD_RC_MODE_FIX_QP) {
                rc.fixqp.fix_i_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp : 26;
                rc.fixqp.fix_p_qp = enc_cfg.init_qp > 0 ? enc_cfg.init_qp + 2 : 28;
                spdlog::warn("Stream {} initialized in FIXQP mode - bitrate/fps changes will not work!", i);
            }
            
            ret = hd_videoenc_set(video_streams_[i].enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc);
            if (ret != HD_OK) {
                spdlog::error("hd_videoenc_set(RATE_CONTROL, stream {}) failed: {}", i, static_cast<int>(ret));
                return false;
            }
        }
        
        video_streams_[i].enc_dim.w = stream_cfg.width;
        video_streams_[i].enc_dim.h = stream_cfg.height;
        
        spdlog::debug("Encoder stream {}: {}x{} {} {}kbps", 
                      i, stream_cfg.width, stream_cfg.height, 
                      enc_cfg.codec, enc_cfg.bitrate);
    }
    
    spdlog::info("Video encoders configured");
    return true;
#else
    return true;
#endif
}

// ============================================================================
// AI Analytics Path (VideoProc output for AI, no encoder)
// ============================================================================

bool HdalPipeline::OpenAnalyticsPath() {
#if HDAL_PIPELINE_ENABLED
    // Check if analytics is enabled in config
    // For now, always enable if we have available outputs
    // Analytics uses a separate VideoProc output path for YUV to AI
    
    // We'll use a dedicated output that doesn't conflict with encoder streams
    // The SDK supports multiple proc outputs - we use one specifically for AI
    // This is IN_0 -> OUT (dedicated for analytics)
    
    // Check config for analytics enable - look for analytics section
    // Default: enable analytics if fewer than 4 video streams are active
    int active_streams = 0;
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (config_.video_streams[i].enabled) active_streams++;
    }
    
    // Only enable analytics path if we have room (less than 4 encoder streams)
    // Stream 3 (OUT_3) is often used for AI when not encoding
    if (active_streams >= kMaxVideoStreams) {
        spdlog::info("All 4 video streams in use, analytics path not available");
        analytics_enabled_ = false;
        return true;  // Not an error, just not available
    }
    
    // Find first unused output for analytics
    // Prefer OUT_3 if available
    int analytics_out_idx = -1;
    for (int i = kMaxVideoStreams - 1; i >= 0; i--) {
        if (!config_.video_streams[i].enabled) {
            analytics_out_idx = i;
            break;
        }
    }
    
    if (analytics_out_idx < 0) {
        spdlog::warn("No available VideoProc output for analytics");
        analytics_enabled_ = false;
        return true;
    }
    
    HD_RESULT ret;
    const HD_IN_ID proc_in_id = HD_VIDEOPROC_0_IN_0;
    HD_OUT_ID proc_out_id;
    
    switch (analytics_out_idx) {
        case 0: proc_out_id = HD_VIDEOPROC_0_OUT_0; break;
        case 1: proc_out_id = HD_VIDEOPROC_0_OUT_1; break;
        case 2: proc_out_id = HD_VIDEOPROC_0_OUT_2; break;
        case 3: proc_out_id = HD_VIDEOPROC_0_OUT_3; break;
        default: return false;
    }
    
    ret = hd_videoproc_open(proc_in_id, proc_out_id, &analytics_proc_path_);
    if (ret != HD_OK) {
        spdlog::warn("hd_videoproc_open(analytics OUT_{}) failed: {}", 
                     analytics_out_idx, static_cast<int>(ret));
        analytics_enabled_ = false;
        return true;  // Non-fatal
    }
    
    analytics_enabled_ = true;
    spdlog::info("Analytics VideoProc path opened on OUT_{}", analytics_out_idx);
    return true;
#else
    return true;
#endif
}

bool HdalPipeline::ConfigureAnalyticsProc() {
#if HDAL_PIPELINE_ENABLED
    if (!analytics_enabled_ || analytics_proc_path_ == 0) {
        return true;  // Nothing to configure
    }
    
    spdlog::info("Configuring analytics VideoProc output...");
    
    HD_RESULT ret;
    
    // Configure input from capture
    HD_VIDEOPROC_IN proc_in;
    memset(&proc_in, 0, sizeof(proc_in));
    proc_in.dim.w = video_streams_[0].cap_dim.w;
    proc_in.dim.h = video_streams_[0].cap_dim.h;
    proc_in.pxlfmt = HD_VIDEO_PXLFMT_RAW10;
    proc_in.frc = HD_VIDEO_FRC_RATIO(config_.video_streams[0].fps, 1);
    
    ret = hd_videoproc_set(analytics_proc_path_, HD_VIDEOPROC_PARAM_IN, &proc_in);
    if (ret != HD_OK) {
        spdlog::error("hd_videoproc_set(IN, analytics) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Configure output for AI - small resolution YUV420
    HD_VIDEOPROC_OUT proc_out;
    memset(&proc_out, 0, sizeof(proc_out));
    proc_out.dim.w = analytics_dim_.w;
    proc_out.dim.h = analytics_dim_.h;
    proc_out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;  // AI inference uses YUV420
    proc_out.dir = HD_VIDEO_DIR_NONE;
    proc_out.frc = HD_VIDEO_FRC_RATIO(config_.video_streams[0].fps, 1);
    proc_out.depth = 1;  // Single buffer, AI pulls latest
    
    ret = hd_videoproc_set(analytics_proc_path_, HD_VIDEOPROC_PARAM_OUT, &proc_out);
    if (ret != HD_OK) {
        spdlog::error("hd_videoproc_set(OUT, analytics) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    spdlog::info("Analytics VideoProc configured: {}x{} YUV420", 
                 analytics_dim_.w, analytics_dim_.h);
    return true;
#else
    return true;
#endif
}

bool HdalPipeline::IsAnalyticsPathAvailable() const {
#if HDAL_PIPELINE_ENABLED
    return analytics_enabled_ && analytics_proc_path_ != 0;
#else
    return false;
#endif
}

uint64_t HdalPipeline::GetAnalyticsPath() const {
#if HDAL_PIPELINE_ENABLED
    return static_cast<uint64_t>(analytics_proc_path_);
#else
    return 0;
#endif
}

uint64_t HdalPipeline::GetVideoProcPath(int stream_id) const {
#if HDAL_PIPELINE_ENABLED
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) return 0;
    return static_cast<uint64_t>(video_streams_[stream_id].proc_path);
#else
    return 0;
#endif
}

uint64_t HdalPipeline::GetVideoEncoderPath(int stream_id) const {
#if HDAL_PIPELINE_ENABLED
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) return 0;
    return static_cast<uint64_t>(video_streams_[stream_id].enc_path);
#else
    return 0;
#endif
}

bool HdalPipeline::IsEncoderRunning(int stream_id) const {
#if HDAL_PIPELINE_ENABLED
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) return false;
    return video_streams_[stream_id].encoder_running;
#else
    return false;
#endif
}

bool HdalPipeline::EnsureEncoderRunning(int stream_id) {
#if HDAL_PIPELINE_ENABLED
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) return false;
    if (!video_streams_[stream_id].enabled || !video_streams_[stream_id].enc_path) return false;
    
    // Already running
    if (video_streams_[stream_id].encoder_running) return true;
    
    // Try to start the encoder
    spdlog::info("EnsureEncoderRunning: Attempting to start encoder for stream {}", stream_id);
    HD_RESULT ret = hd_videoenc_start(video_streams_[stream_id].enc_path);
    if (ret == HD_OK) {
        video_streams_[stream_id].encoder_running = true;
        spdlog::info("EnsureEncoderRunning: Successfully started encoder for stream {}", stream_id);
        return true;
    } else {
        spdlog::error("EnsureEncoderRunning: Failed to start encoder for stream {}: {}", 
                      stream_id, static_cast<int>(ret));
        return false;
    }
#else
    return false;
#endif
}

bool HdalPipeline::GetAnalyticsDimensions(int& width, int& height) const {
#if HDAL_PIPELINE_ENABLED
    if (!analytics_enabled_) return false;
    width = analytics_dim_.w;
    height = analytics_dim_.h;
    return true;
#else
    return false;
#endif
}

// ============================================================================
// Motion Detection Path (160x120 YUV for libmd)
// ============================================================================

bool HdalPipeline::OpenMdPath() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Opening Motion Detection path...");
    
    // Find an unused VideoProc output for MD (prefer higher index to avoid conflicts)
    // We need OUT that's not used by video streams or analytics
    int md_out_idx = -1;
    
    // Check which outputs are in use
    bool out_used[4] = {false, false, false, false};
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (config_.video_streams[i].enabled) {
            out_used[i] = true;
        }
    }
    
    // If analytics is using an output, mark it
    // Analytics finds first unused from the back, so check if it took one
    if (analytics_enabled_) {
        for (int i = kMaxVideoStreams - 1; i >= 0; i--) {
            if (!out_used[i]) {
                out_used[i] = true;  // Analytics took this one
                break;
            }
        }
    }
    
    // Find first available for MD (from the back)
    for (int i = kMaxVideoStreams - 1; i >= 0; i--) {
        if (!out_used[i]) {
            md_out_idx = i;
            break;
        }
    }
    
    if (md_out_idx < 0) {
        spdlog::warn("No available VideoProc output for MD path, all 4 outputs in use");
        md_path_enabled_ = false;
        return true;  // Non-fatal
    }
    
    HD_RESULT ret;
    const HD_IN_ID proc_in_id = HD_VIDEOPROC_0_IN_0;
    HD_OUT_ID proc_out_id;
    
    switch (md_out_idx) {
        case 0: proc_out_id = HD_VIDEOPROC_0_OUT_0; break;
        case 1: proc_out_id = HD_VIDEOPROC_0_OUT_1; break;
        case 2: proc_out_id = HD_VIDEOPROC_0_OUT_2; break;
        case 3: proc_out_id = HD_VIDEOPROC_0_OUT_3; break;
        default: return false;
    }
    
    ret = hd_videoproc_open(proc_in_id, proc_out_id, &md_proc_path_);
    if (ret != HD_OK) {
        spdlog::warn("hd_videoproc_open(MD OUT_{}) failed: {}", 
                     md_out_idx, static_cast<int>(ret));
        md_path_enabled_ = false;
        return true;  // Non-fatal
    }
    
    md_path_enabled_ = true;
    spdlog::info("Motion Detection VideoProc path opened on OUT_{}", md_out_idx);
    return true;
#else
    return true;
#endif
}

bool HdalPipeline::ConfigureMdProc() {
#if HDAL_PIPELINE_ENABLED
    if (!md_path_enabled_ || md_proc_path_ == 0) {
        return true;  // Nothing to configure
    }
    
    spdlog::info("Configuring MD VideoProc output ({}x{})...", md_dim_.w, md_dim_.h);
    
    HD_RESULT ret;
    
    // Configure output for MD - small resolution YUV420 (standard 160x120 for libmd)
    HD_VIDEOPROC_OUT proc_out;
    memset(&proc_out, 0, sizeof(proc_out));
    proc_out.dim.w = md_dim_.w;
    proc_out.dim.h = md_dim_.h;
    proc_out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
    proc_out.dir = HD_VIDEO_DIR_NONE;
    proc_out.frc = HD_VIDEO_FRC_RATIO(config_.video_streams[0].fps, 1);
    proc_out.depth = 1;  // Single buffer for pull mode
    
    ret = hd_videoproc_set(md_proc_path_, HD_VIDEOPROC_PARAM_OUT, &proc_out);
    if (ret != HD_OK) {
        spdlog::error("hd_videoproc_set(OUT, MD) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    // Enable MD output function on the main video stream for hardware Motion AQ
    // This enables the encoder to use MD info for EVBR motion adaptation
    if (video_streams_[0].proc_path != 0) {
        HD_VIDEOPROC_FUNC_CONFIG func_cfg;
        memset(&func_cfg, 0, sizeof(func_cfg));
        func_cfg.out_func = HD_VIDEOPROC_OUTFUNC_MD;
        
        ret = hd_videoproc_set(video_streams_[0].proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &func_cfg);
        if (ret != HD_OK) {
            spdlog::warn("Failed to enable MD on main proc path: {}", static_cast<int>(ret));
            // Non-fatal - MD path still works for libmd processing
        } else {
            spdlog::info("Hardware MD enabled on main video path for Motion AQ");
        }
    }
    
    spdlog::info("MD VideoProc configured: {}x{} YUV420 for libmd processing", 
                 md_dim_.w, md_dim_.h);
    return true;
#else
    return true;
#endif
}

bool HdalPipeline::IsMdPathAvailable() const {
#if HDAL_PIPELINE_ENABLED
    return md_path_enabled_ && md_proc_path_ != 0;
#else
    return false;
#endif
}

uint64_t HdalPipeline::GetMdPath() const {
#if HDAL_PIPELINE_ENABLED
    return static_cast<uint64_t>(md_proc_path_);
#else
    return 0;
#endif
}

bool HdalPipeline::GetMdDimensions(int& width, int& height) const {
#if HDAL_PIPELINE_ENABLED
    if (!md_path_enabled_) return false;
    width = md_dim_.w;
    height = md_dim_.h;
    return true;
#else
    return false;
#endif
}

// ============================================================================
// VQA Tamper Detection Path (320x180 YUV for IVE-accelerated histogram/Sobel)
// ============================================================================

bool HdalPipeline::OpenVqaPath() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Opening VQA tamper detection path...");
    
    // Find an unused VideoProc output for VQA (prefer higher index to avoid conflicts)
    // We need OUT that's not used by video streams, analytics, or MD
    int vqa_out_idx = -1;
    
    // Check which outputs are in use
    bool out_used[4] = {false, false, false, false};
    for (int i = 0; i < kMaxVideoStreams; i++) {
        if (config_.video_streams[i].enabled) {
            out_used[i] = true;
        }
    }
    
    // If analytics is using an output, mark it
    if (analytics_enabled_) {
        for (int i = kMaxVideoStreams - 1; i >= 0; i--) {
            if (!out_used[i]) {
                out_used[i] = true;  // Analytics took this one
                break;
            }
        }
    }
    
    // If MD is using an output, mark it
    if (md_path_enabled_) {
        for (int i = kMaxVideoStreams - 1; i >= 0; i--) {
            if (!out_used[i]) {
                out_used[i] = true;  // MD took this one
                break;
            }
        }
    }
    
    // Find first available for VQA (from the back)
    for (int i = kMaxVideoStreams - 1; i >= 0; i--) {
        if (!out_used[i]) {
            vqa_out_idx = i;
            break;
        }
    }
    
    if (vqa_out_idx < 0) {
        spdlog::warn("No available VideoProc output for VQA path, all 4 outputs in use");
        vqa_path_enabled_ = false;
        return true;  // Non-fatal
    }
    
    HD_RESULT ret;
    const HD_IN_ID proc_in_id = HD_VIDEOPROC_0_IN_0;
    HD_OUT_ID proc_out_id;
    
    switch (vqa_out_idx) {
        case 0: proc_out_id = HD_VIDEOPROC_0_OUT_0; break;
        case 1: proc_out_id = HD_VIDEOPROC_0_OUT_1; break;
        case 2: proc_out_id = HD_VIDEOPROC_0_OUT_2; break;
        case 3: proc_out_id = HD_VIDEOPROC_0_OUT_3; break;
        default: return false;
    }
    
    ret = hd_videoproc_open(proc_in_id, proc_out_id, &vqa_proc_path_);
    if (ret != HD_OK) {
        spdlog::warn("hd_videoproc_open(VQA OUT_{}) failed: {}", 
                     vqa_out_idx, static_cast<int>(ret));
        vqa_path_enabled_ = false;
        return true;  // Non-fatal
    }
    
    vqa_path_enabled_ = true;
    spdlog::info("VQA VideoProc path opened on OUT_{}", vqa_out_idx);
    return true;
#else
    return true;
#endif
}

bool HdalPipeline::ConfigureVqaProc() {
#if HDAL_PIPELINE_ENABLED
    if (!vqa_path_enabled_ || vqa_proc_path_ == 0) {
        return true;  // Nothing to configure
    }
    
    spdlog::info("Configuring VQA VideoProc output ({}x{})...", vqa_dim_.w, vqa_dim_.h);
    
    HD_RESULT ret;
    
    // Configure output for VQA - 320x180 YUV420 (optimized for IVE histogram/Sobel)
    // This resolution is specifically chosen for efficient IVE processing:
    // - 57,600 pixels Y-plane fits well in IVE histogram engine
    // - Sobel edge detection at this resolution ~190µs
    // - Histogram calculation ~230µs
    HD_VIDEOPROC_OUT proc_out;
    memset(&proc_out, 0, sizeof(proc_out));
    proc_out.dim.w = vqa_dim_.w;
    proc_out.dim.h = vqa_dim_.h;
    proc_out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
    proc_out.dir = HD_VIDEO_DIR_NONE;
    proc_out.frc = HD_VIDEO_FRC_RATIO(config_.video_streams[0].fps, 1);
    proc_out.depth = 1;  // Single buffer for pull mode
    
    ret = hd_videoproc_set(vqa_proc_path_, HD_VIDEOPROC_PARAM_OUT, &proc_out);
    if (ret != HD_OK) {
        spdlog::error("hd_videoproc_set(OUT, VQA) failed: {}", static_cast<int>(ret));
        return false;
    }
    
    spdlog::info("VQA VideoProc configured: {}x{} YUV420 for IVE-accelerated tamper detection", 
                 vqa_dim_.w, vqa_dim_.h);
    return true;
#else
    return true;
#endif
}

bool HdalPipeline::IsVqaPathAvailable() const {
#if HDAL_PIPELINE_ENABLED
    return vqa_path_enabled_ && vqa_proc_path_ != 0;
#else
    return false;
#endif
}

uint64_t HdalPipeline::GetVqaPath() const {
#if HDAL_PIPELINE_ENABLED
    return static_cast<uint64_t>(vqa_proc_path_);
#else
    return 0;
#endif
}

bool HdalPipeline::GetVqaDimensions(int& width, int& height) const {
#if HDAL_PIPELINE_ENABLED
    if (!vqa_path_enabled_) return false;
    width = vqa_dim_.w;
    height = vqa_dim_.h;
    return true;
#else
    return false;
#endif
}

// ============================================================================
// Global Privacy Mask (VIDEOPROC Level)
// ============================================================================

bool HdalPipeline::OpenGlobalMaskPaths() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Opening global privacy mask paths (VIDEOPROC level)...");
    
    HD_RESULT ret;
    int opened = 0;
    
    // Open mask paths on VIDEOPROC_0_IN_0 (the main video processor input)
    // These masks are applied before encoding, so they affect ALL streams
    for (int i = 0; i < kMaxGlobalMasks; i++) {
        ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, static_cast<HD_OUT_ID>(HD_MASK_0 + i), &global_mask_paths_[i]);
        if (ret != HD_OK) {
            spdlog::warn("Failed to open global mask path {}: {}", i, static_cast<int>(ret));
            global_mask_paths_[i] = 0;
        } else {
            spdlog::info("Opened global mask path {}: 0x{:X}", i, global_mask_paths_[i]);
            opened++;
        }
    }
    
    if (opened > 0) {
        spdlog::info("Opened {}/{} global mask paths (VIDEOPROC level)", opened, kMaxGlobalMasks);
        return true;
    }
    
    spdlog::warn("No global mask paths available");
    return false;
#else
    return false;
#endif
}

void HdalPipeline::CloseGlobalMaskPaths() {
#if HDAL_PIPELINE_ENABLED
    for (int i = 0; i < kMaxGlobalMasks; i++) {
        if (global_mask_paths_[i]) {
            hd_videoproc_stop(global_mask_paths_[i]);
            hd_videoproc_close(global_mask_paths_[i]);
            global_mask_paths_[i] = 0;
        }
    }
    global_mask_enabled_ = false;
    spdlog::info("Closed global mask paths");
#endif
}

bool HdalPipeline::SetGlobalPrivacyMask(int region_id, bool enabled,
                                        int x1, int y1, int x2, int y2,
                                        uint32_t color) {
#if HDAL_PIPELINE_ENABLED
    if (region_id < 0 || region_id >= kMaxGlobalMasks) {
        spdlog::error("Invalid global mask region_id: {}", region_id);
        return false;
    }
    
    if (!global_mask_paths_[region_id]) {
        spdlog::error("Global mask path {} not available", region_id);
        return false;
    }
    
    // Store the configuration
    global_mask_regions_[region_id].enabled = enabled;
    global_mask_regions_[region_id].x1 = x1;
    global_mask_regions_[region_id].y1 = y1;
    global_mask_regions_[region_id].x2 = x2;
    global_mask_regions_[region_id].y2 = y2;
    global_mask_regions_[region_id].color = color;
    
    // Build the mask attribute
    HD_OSG_MASK_ATTR attr;
    memset(&attr, 0, sizeof(attr));
    
    if (enabled && global_mask_enabled_) {
        // Define quadrilateral (clockwise from top-left)
        attr.position[0].x = x1;
        attr.position[0].y = y1;
        attr.position[1].x = x2;
        attr.position[1].y = y1;
        attr.position[2].x = x2;
        attr.position[2].y = y2;
        attr.position[3].x = x1;
        attr.position[3].y = y2;
        attr.type = HD_OSG_MASK_TYPE_SOLID;
        attr.alpha = (color >> 24) & 0xFF;  // Extract alpha from ARGB
        attr.color = color;
        
        spdlog::info("Setting global mask {}: ({},{})-({},{}) color=0x{:08X}",
                     region_id, x1, y1, x2, y2, color);
    } else {
        // Disable mask (zero-size)
        attr.position[0] = {0, 0};
        attr.position[1] = {0, 0};
        attr.position[2] = {0, 0};
        attr.position[3] = {0, 0};
        attr.type = HD_OSG_MASK_TYPE_SOLID;
        attr.alpha = 0;
        attr.color = 0;
        spdlog::info("Disabling global mask {}", region_id);
    }
    
    // Apply to VIDEOPROC
    HD_RESULT ret = hd_videoproc_set(global_mask_paths_[region_id], 
                                      HD_VIDEOPROC_PARAM_IN_MASK_ATTR, &attr);
    if (ret != HD_OK) {
        spdlog::error("Failed to set global mask {}: {}", region_id, static_cast<int>(ret));
        // Continue anyway - some implementations return error but still work
    }
    
    // Start/stop the mask path based on enabled state
    if (enabled && global_mask_enabled_) {
        ret = hd_videoproc_start(global_mask_paths_[region_id]);
        if (ret != HD_OK) {
            spdlog::warn("Failed to start global mask {}: {}", region_id, static_cast<int>(ret));
        }
    } else {
        hd_videoproc_stop(global_mask_paths_[region_id]);
    }
    
    return true;
#else
    return false;
#endif
}

bool HdalPipeline::SetGlobalPrivacyMaskEnabled(bool enabled) {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("SetGlobalPrivacyMaskEnabled: {}", enabled);
    global_mask_enabled_ = enabled;
    
    // Re-apply all mask regions with new enabled state
    bool success = true;
    for (int i = 0; i < kMaxGlobalMasks; i++) {
        if (global_mask_paths_[i]) {
            const auto& r = global_mask_regions_[i];
            if (!SetGlobalPrivacyMask(i, r.enabled, r.x1, r.y1, r.x2, r.y2, r.color)) {
                success = false;
            }
        }
    }
    
    return success;
#else
    return false;
#endif
}

bool HdalPipeline::IsGlobalPrivacyMaskAvailable() const {
#if HDAL_PIPELINE_ENABLED
    for (int i = 0; i < kMaxGlobalMasks; i++) {
        if (global_mask_paths_[i]) return true;
    }
    return false;
#else
    return false;
#endif
}

uint64_t HdalPipeline::GetGlobalMaskPath(int region_id) const {
#if HDAL_PIPELINE_ENABLED
    if (region_id < 0 || region_id >= kMaxGlobalMasks) return 0;
    return global_mask_paths_[region_id];
#else
    return 0;
#endif
}

// ============================================================================
// Snapshot Capture
// ============================================================================

bool HdalPipeline::CaptureSnapshot(int stream_id, int quality, std::vector<uint8_t>& buffer) {
#if HDAL_PIPELINE_ENABLED
    if (state_ != PipelineState::kRunning) {
        spdlog::error("CaptureSnapshot: Pipeline not running");
        return false;
    }
    
    if (stream_id < 0 || stream_id >= kMaxVideoStreams) {
        spdlog::error("CaptureSnapshot: Invalid stream_id {}", stream_id);
        return false;
    }
    
    if (!video_streams_[stream_id].enabled || !video_streams_[stream_id].encoder_running) {
        spdlog::error("CaptureSnapshot: Stream {} not enabled or encoder not running", stream_id);
        return false;
    }
    
    // Clamp quality to valid range
    if (quality < 1) quality = 1;
    if (quality > 100) quality = 100;
    
    const auto& stream_cfg = config_.video_streams[stream_id];
    
    // Calculate max snapshot buffer size
    // Using 2MB fixed size to match our memory pool configuration for snapshots
    // Max JPEG size for 1080p is unlikely to exceed 2MB.
    uint32_t max_snapshot_size = 2 * 1024 * 1024;
    
    HD_COMMON_MEM_VB_BLK blk = 0;
    UINTPTR pa = 0, va = 0;
    bool success = false;
    
    // Allocate memory block from common pool
    // Use configured DDR ID
    HD_COMMON_MEM_DDR_ID ddr_id = static_cast<HD_COMMON_MEM_DDR_ID>(config_.memory.ddr_id);
    blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, max_snapshot_size, ddr_id);
    if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        spdlog::error("CaptureSnapshot: Failed to allocate memory block (size={})", max_snapshot_size);
        return false;
    }
    
    // Get physical address
    pa = hd_common_mem_blk2pa(blk);
    if (pa == 0) {
        spdlog::error("CaptureSnapshot: blk2pa failed");
        hd_common_mem_release_block(blk);
        return false;
    }
    
    // Map to virtual address
    va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, max_snapshot_size);
    if (va == 0) {
        spdlog::error("CaptureSnapshot: mmap failed (pa=0x{:x})", pa);
        hd_common_mem_release_block(blk);
        return false;
    }
    
    // Trigger snapshot
    HD_H26XENC_TRIG_SNAPSHOT snap_param = {0};
    snap_param.phy_addr      = pa;
    snap_param.size          = max_snapshot_size;
    snap_param.image_quality = quality;
    
    spdlog::info("CaptureSnapshot: Stream {} (path 0x{:x}), PA=0x{:x}, MaxSize={}, Quality={}", 
                 stream_id, static_cast<unsigned int>(video_streams_[stream_id].enc_path), 
                 static_cast<unsigned int>(pa), max_snapshot_size, quality);

    HD_RESULT ret = HD_ERR_NG;
    int retries = 3;
    
    while (retries > 0) {
        // Reset parameters for each attempt as they might be modified by the driver
        snap_param.phy_addr      = pa;
        snap_param.size          = max_snapshot_size; // Vital: Reset size!
        snap_param.image_quality = quality;

        ret = hd_videoenc_set(video_streams_[stream_id].enc_path, 
                              HD_VIDEOENC_PARAM_OUT_TRIG_SNAPSHOT, &snap_param);
        
        if (ret == HD_OK) break;
        
        spdlog::warn("CaptureSnapshot: Trigger failed (ret={}), retrying... (left={})", 
                     static_cast<int>(ret), retries-1);
        
        // If timeout, maybe wait a bit longer
        usleep(200000); // 200ms wait
        retries--;
    }

    if (ret != HD_OK) {
        spdlog::error("CaptureSnapshot: Trigger failed on stream {} after retries (ret={})", 
                      stream_id, static_cast<int>(ret));
    } else if (snap_param.size > 0 && snap_param.size <= max_snapshot_size) {
        // Copy JPEG data to output buffer
        buffer.resize(snap_param.size);
        // Invalidate cache not needed if using uncached or handling via mmap type (assumed handled by driver/OS)
        memcpy(buffer.data(), (void*)va, snap_param.size);
        success = true;
        spdlog::info("CaptureSnapshot: Success, stream={}, size={} bytes, quality={}", 
                     stream_id, snap_param.size, quality);
    } else {
        spdlog::error("CaptureSnapshot: Invalid snapshot size {} (max={})", snap_param.size, max_snapshot_size);
    }
    
    // Cleanup
    if (va) hd_common_mem_munmap((void*)va, max_snapshot_size);
    if (blk) hd_common_mem_release_block(blk);
    
    return success;
#else
    spdlog::warn("CaptureSnapshot: HDAL not enabled");
    return false;
#endif
}

} // namespace platform
} // namespace ipcam
