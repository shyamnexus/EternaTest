/**
 * @file hdal_pipeline_config.cpp
 * @brief HDAL Pipeline Configuration Loading
 */

#include "hdal_pipeline_common.h"

namespace ipcam {
namespace platform {

// ============================================================================
// Configuration Loading
// ============================================================================

bool HdalPipeline::LoadConfig() {
    // Note: Do NOT lock mutex here - this is called from Init() which already holds the lock
    // The public LoadConfig() is only called from Init() or SetConfig() which handle locking
    
    if (config_loaded_) {
        spdlog::debug("HDAL config already loaded");
        return true;
    }
    
    spdlog::info("Loading HDAL pipeline configuration from media.json");
    
    try {
        // Load sensor config
        config_.sensor.driver_name = config::Get<std::string>("media.sensor.driver_name", "nvt_sen_gc5603");
        config_.sensor.interface = config::Get<std::string>("media.sensor.interface", "mipi_csi");
        config_.sensor.pinmux = config::Get<int>("media.sensor.pinmux", 0);
        config_.sensor.data_lanes = config::Get<int>("media.sensor.data_lanes", 2);
        config_.sensor.vcap_id = config::Get<int>("media.sensor.vcap_id", 0);
        config_.sensor.raw_format = config::Get<std::string>("media.sensor.raw_format", "raw10");
        config_.sensor.native_width = config::Get<int>("media.sensor.native_width", 2960);
        config_.sensor.native_height = config::Get<int>("media.sensor.native_height", 1664);
        
        // Sensor crop
        config_.sensor.crop.enabled = config::Get<bool>("media.sensor.crop.enabled", true);
        config_.sensor.crop.x = config::Get<int>("media.sensor.crop.x", 8);
        config_.sensor.crop.y = config::Get<int>("media.sensor.crop.y", 0);
        config_.sensor.crop.width = config::Get<int>("media.sensor.crop.width", 2944);
        config_.sensor.crop.height = config::Get<int>("media.sensor.crop.height", 1664);
        
        // Load ISP tuning config
        config_.isp_tuning.config_path = config::Get<std::string>("media.isp_tuning.config_path", "/mnt/app/isp");
        config_.isp_tuning.config_name = config::Get<std::string>("media.isp_tuning.config_name", "isp_gc5603_0");
        config_.isp_tuning.color_temperature = config::Get<int>("media.isp_tuning.color_temperature", 4700);
        config_.isp_tuning.hdr_enabled = config::Get<bool>("media.isp_tuning.hdr_enabled", false);
        
        // Load videoproc config
        config_.videoproc.pipe = config::Get<std::string>("media.videoproc.pipe", "rawall");
        config_.videoproc.ae_enabled = config::Get<bool>("media.videoproc.ae_enabled", true);
        config_.videoproc.awb_enabled = config::Get<bool>("media.videoproc.awb_enabled", true);
        config_.videoproc.af_enabled = config::Get<bool>("media.videoproc.af_enabled", true);
        config_.videoproc.wdr_enabled = config::Get<bool>("media.videoproc.wdr_enabled", true);
        config_.videoproc.defog_enabled = config::Get<bool>("media.videoproc.defog_enabled", true);
        config_.videoproc.nr_3d_enabled = config::Get<bool>("media.videoproc.nr_3d_enabled", true);
        config_.videoproc.nr_3d_ref_path = config::Get<int>("media.videoproc.nr_3d_ref_path", 4);
        
        // Load memory config
        config_.memory.ddr_id = config::Get<int>("media.memory.ddr_id", 0);
        config_.memory.raw_buffer_count = config::Get<int>("media.memory.raw_buffer_count", 9);
        config_.memory.yuv_buffer_count = config::Get<int>("media.memory.yuv_buffer_count", 6);
        config_.memory.audio_buffer_size = config::Get<int>("media.memory.audio_buffer_size", 16384);
        
        // Load video stream configs
        const char* stream_keys[] = {"video1", "video2", "video3", "video4"};
        for (int i = 0; i < kMaxVideoStreams; i++) {
            std::string prefix = std::string("media.") + stream_keys[i];
            
            config_.video_streams[i].id = i;
            config_.video_streams[i].enabled = config::Get<bool>(prefix + ".enabled", i < 3);
            config_.video_streams[i].stream_type = config::Get<std::string>(prefix + ".stream_type", "mainStream");
            config_.video_streams[i].width = config::Get<int>(prefix + ".width", 1920);
            config_.video_streams[i].height = config::Get<int>(prefix + ".height", 1080);
            config_.video_streams[i].fps = config::Get<int>(prefix + ".fps", 30);
            
            // Encoder config
            config_.video_streams[i].encoder.codec = config::Get<std::string>(prefix + ".codec", "h264");
            config_.video_streams[i].encoder.profile = config::Get<std::string>(prefix + ".profile", "main");
            config_.video_streams[i].encoder.level = config::Get<std::string>(prefix + ".level", "4.1");
            config_.video_streams[i].encoder.bitrate = config::Get<int>(prefix + ".bitrate", 2048);
            config_.video_streams[i].encoder.max_bitrate = config::Get<int>(prefix + ".max_bitrate", 4096);
            config_.video_streams[i].encoder.min_bitrate = config::Get<int>(prefix + ".min_bitrate", 512);
            config_.video_streams[i].encoder.gop = config::Get<int>(prefix + ".gop", 30);
            config_.video_streams[i].encoder.idr_interval = config::Get<int>(prefix + ".idr_interval", 1);
            config_.video_streams[i].encoder.rc_mode = config::Get<std::string>(prefix + ".rc_mode", "CBR");
            config_.video_streams[i].encoder.init_qp = config::Get<int>(prefix + ".init_qp", 26);
            config_.video_streams[i].encoder.min_qp = config::Get<int>(prefix + ".min_qp", 10);
            config_.video_streams[i].encoder.max_qp = config::Get<int>(prefix + ".max_qp", 45);
            config_.video_streams[i].encoder.entropy_mode = config::Get<std::string>(prefix + ".entropy_mode", "cabac");
            config_.video_streams[i].encoder.buffer_size = config::Get<int>(prefix + ".buffer_size", 3000);
            config_.video_streams[i].encoder.buffer_count = config::Get<int>(prefix + ".buffer_count", 3);

            const int max_w = config_.sensor.crop.enabled ? config_.sensor.crop.width : config_.sensor.native_width;
            const int max_h = config_.sensor.crop.enabled ? config_.sensor.crop.height : config_.sensor.native_height;
            const int aligned_w = config_.video_streams[i].width - (config_.video_streams[i].width % 32);
            const int aligned_h = config_.video_streams[i].height - (config_.video_streams[i].height % 2);
            if (aligned_w != config_.video_streams[i].width) {
                spdlog::warn("Stream {} width {} not 32-aligned, clamping to {}", i,
                             config_.video_streams[i].width, aligned_w);
                config_.video_streams[i].width = aligned_w;
            }
            if (aligned_h != config_.video_streams[i].height) {
                spdlog::warn("Stream {} height {} not 2-aligned, clamping to {}", i,
                             config_.video_streams[i].height, aligned_h);
                config_.video_streams[i].height = aligned_h;
            }
            if (config_.video_streams[i].width > max_w) {
                spdlog::warn("Stream {} width {} exceeds capture max {}, clamping", i,
                             config_.video_streams[i].width, max_w);
                config_.video_streams[i].width = max_w;
            }
            if (config_.video_streams[i].height > max_h) {
                spdlog::warn("Stream {} height {} exceeds capture max {}, clamping", i,
                             config_.video_streams[i].height, max_h);
                config_.video_streams[i].height = max_h;
            }
            // Stream 0 is allowed to downscale from capture via videoproc.
            // Only clamp to not exceed capture dimensions (already done above).
        }
        
        // Load audio config
        config_.audio.enabled = config::Get<bool>("media.audio.enabled", true);
        config_.audio.codec = config::Get<std::string>("media.audio.codec", "pcm");
        config_.audio.sample_rate = config::Get<int>("media.audio.sample_rate", 48000);
        config_.audio.bitrate = config::Get<int>("media.audio.bitrate", 64000);
        config_.audio.channels = config::Get<int>("media.audio.channels", 2);
        config_.audio.bit_width = config::Get<int>("media.audio.bit_width", 16);
        config_.audio.mode = config::Get<std::string>("media.audio.mode", "stereo");
        config_.audio.frame_samples = config::Get<int>("media.audio.frame_samples", 1024);
        
        spdlog::info("HDAL config loaded: sensor={}, streams={}x{}@{}fps (main)",
                     config_.sensor.driver_name,
                     config_.video_streams[0].width,
                     config_.video_streams[0].height,
                     config_.video_streams[0].fps);
        
        config_loaded_ = true;
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to load HDAL config: {}", e.what());
        return false;
    }
}

// ============================================================================
// Helper Methods - Codec/Profile/Level Conversion
// ============================================================================

int HdalPipeline::GetCodecType(const std::string& codec) {
#if HDAL_PIPELINE_ENABLED
    if (codec == "h265" || codec == "hevc") return HD_CODEC_TYPE_H265;
    if (codec == "h264" || codec == "avc") return HD_CODEC_TYPE_H264;
    if (codec == "jpeg" || codec == "mjpeg") return HD_CODEC_TYPE_JPEG;
    return HD_CODEC_TYPE_H264;  // default
#else
    (void)codec;
    return 0;
#endif
}

int HdalPipeline::GetProfileType(const std::string& codec, const std::string& profile) {
#if HDAL_PIPELINE_ENABLED
    if (codec == "h265" || codec == "hevc") {
        // Only MAIN profile supported in this SDK
        (void)profile;
        return HD_H265E_MAIN_PROFILE;
    } else {
        if (profile == "baseline") return HD_H264E_BASELINE_PROFILE;
        if (profile == "main") return HD_H264E_MAIN_PROFILE;
        if (profile == "high") return HD_H264E_HIGH_PROFILE;
        return HD_H264E_MAIN_PROFILE;
    }
#else
    (void)codec; (void)profile;
    return 0;
#endif
}

int HdalPipeline::GetLevelType(const std::string& codec, const std::string& level) {
#if HDAL_PIPELINE_ENABLED
    // Parse level string like "4.1" or "5.0"
    float lvl = std::stof(level);
    
    if (codec == "h265" || codec == "hevc") {
        if (lvl >= 5.0) return HD_H265E_LEVEL_5;
        if (lvl >= 4.1) return HD_H265E_LEVEL_4_1;
        if (lvl >= 4.0) return HD_H265E_LEVEL_4;
        return HD_H265E_LEVEL_4;
    } else {
        if (lvl >= 5.1) return HD_H264E_LEVEL_5_1;
        if (lvl >= 5.0) return HD_H264E_LEVEL_5;
        if (lvl >= 4.1) return HD_H264E_LEVEL_4_1;
        if (lvl >= 4.0) return HD_H264E_LEVEL_4;
        if (lvl >= 3.1) return HD_H264E_LEVEL_3_1;
        return HD_H264E_LEVEL_4;
    }
#else
    (void)codec; (void)level;
    return 0;
#endif
}

int HdalPipeline::GetRcMode(const std::string& mode) {
#if HDAL_PIPELINE_ENABLED
    if (mode == "CBR" || mode == "cbr") return HD_RC_MODE_CBR;
    if (mode == "VBR" || mode == "vbr") return HD_RC_MODE_VBR;
    if (mode == "FIXQP" || mode == "fixqp") return HD_RC_MODE_FIX_QP;
    if (mode == "EVBR" || mode == "evbr") return HD_RC_MODE_EVBR;
    if (mode == "CVBR" || mode == "cvbr") return HD_RC_MODE_CVBR;
    return HD_RC_MODE_CBR;
#else
    (void)mode;
    return 0;
#endif
}

// GetAudioCodecType is defined in hdal_pipeline_audio.cpp

int HdalPipeline::CalculateRawBufferSize(int width, int height, const std::string& format) {
#if HDAL_PIPELINE_ENABLED
    int bpp = 10;  // RAW10 default
    if (format == "raw8") bpp = 8;
    else if (format == "raw12") bpp = 12;
    else if (format == "raw14") bpp = 14;
    
    return DBGINFO_BUFSIZE + VDO_RAW_BUFSIZE(width, height, bpp) +
           VDO_CA_BUF_SIZE(HDAL_ISP_CA_W_WINNUM, HDAL_ISP_CA_H_WINNUM) +
           VDO_LA_BUF_SIZE(HDAL_ISP_LA_W_WINNUM, HDAL_ISP_LA_H_WINNUM);
#else
    (void)width; (void)height; (void)format;
    return 0;
#endif
}

int HdalPipeline::CalculateYuvBufferSize(int width, int height) {
#if HDAL_PIPELINE_ENABLED
    return DBGINFO_BUFSIZE + VDO_YUV_BUFSIZE(width, height) +
           VDO_VA_BUF_SIZE(HDAL_ISP_VA_W_WINNUM, HDAL_ISP_VA_H_WINNUM);
#else
    (void)width; (void)height;
    return 0;
#endif
}

} // namespace platform
} // namespace ipcam
