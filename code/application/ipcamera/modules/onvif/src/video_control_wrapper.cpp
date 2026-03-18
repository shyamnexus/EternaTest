/**
 * @file video_control_wrapper.cpp
 * @brief C wrapper implementation for Video Control functions
 * 
 * Implements the C wrapper functions by calling into the C++ VideoControl class.
 */

#include "ipcam/video_control_wrapper.h"
#include "ipcam/video_control.h"
#include <spdlog/spdlog.h>
#include <cstring>

using namespace ipcam::media;

// Stream ID mapping
static const char* STREAM_IDS[] = {"video1", "video2", "video3"};
static const int MAX_STREAMS = 3;

// Helper to convert between enum types
static VideoCodec onvif_to_video_codec(OnvifVideoCodec codec) {
    switch (codec) {
        case ONVIF_VIDEO_CODEC_H264: return VideoCodec::H264;
        case ONVIF_VIDEO_CODEC_H265: return VideoCodec::H265;
        case ONVIF_VIDEO_CODEC_MJPEG: return VideoCodec::MJPEG;
        default: return VideoCodec::H264;
    }
}

static OnvifVideoCodec video_codec_to_onvif(VideoCodec codec) {
    switch (codec) {
        case VideoCodec::H264: return ONVIF_VIDEO_CODEC_H264;
        case VideoCodec::H265: return ONVIF_VIDEO_CODEC_H265;
        case VideoCodec::MJPEG: return ONVIF_VIDEO_CODEC_MJPEG;
        default: return ONVIF_VIDEO_CODEC_H264;
    }
}

static H264Profile onvif_to_h264_profile(OnvifH264Profile profile) {
    switch (profile) {
        case ONVIF_H264_PROFILE_BASELINE: return H264Profile::Baseline;
        case ONVIF_H264_PROFILE_MAIN: return H264Profile::Main;
        case ONVIF_H264_PROFILE_HIGH: return H264Profile::High;
        default: return H264Profile::Main;
    }
}

static OnvifH264Profile h264_profile_to_onvif(H264Profile profile) {
    switch (profile) {
        case H264Profile::Baseline: return ONVIF_H264_PROFILE_BASELINE;
        case H264Profile::Main: return ONVIF_H264_PROFILE_MAIN;
        case H264Profile::High: return ONVIF_H264_PROFILE_HIGH;
        default: return ONVIF_H264_PROFILE_MAIN;
    }
}

static RateControlMode onvif_to_rc_mode(OnvifRateControlMode mode) {
    switch (mode) {
        case ONVIF_RATE_CONTROL_CBR: return RateControlMode::CBR;
        case ONVIF_RATE_CONTROL_VBR: return RateControlMode::VBR;
        case ONVIF_RATE_CONTROL_CVBR: return RateControlMode::CVBR;
        default: return RateControlMode::CBR;
    }
}

static OnvifRateControlMode rc_mode_to_onvif(RateControlMode mode) {
    switch (mode) {
        case RateControlMode::CBR: return ONVIF_RATE_CONTROL_CBR;
        case RateControlMode::VBR: return ONVIF_RATE_CONTROL_VBR;
        case RateControlMode::CVBR: return ONVIF_RATE_CONTROL_CVBR;
        default: return ONVIF_RATE_CONTROL_CBR;
    }
}

static const char* get_stream_id(int stream_idx) {
    if (stream_idx < 0 || stream_idx >= MAX_STREAMS) {
        return nullptr;
    }
    return STREAM_IDS[stream_idx];
}

extern "C" {

int onvif_get_stream_count(void) {
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) {
        spdlog::warn("VideoControl not initialized");
        return MAX_STREAMS; // Return default count
    }
    return vc.GetStreamCount();
}

int onvif_get_video_config(int stream_idx, OnvifVideoStreamConfig *config) {
    if (!config) return -1;
    
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) {
        spdlog::warn("VideoControl not initialized, returning defaults");
        // Return default config
        config->enabled = true;
        config->width = 1920;
        config->height = 1080;
        config->fps = 30;
        config->codec = ONVIF_VIDEO_CODEC_H264;
        config->profile = ONVIF_H264_PROFILE_MAIN;
        config->gop = 50;
        config->rc_mode = ONVIF_RATE_CONTROL_CBR;
        config->bitrate = 4096;
        config->max_bitrate = 6144;
        config->min_bitrate = 1024;
        config->quality = 50;
        config->init_qp = 26;
        config->min_qp = 10;
        config->max_qp = 51;
        return 0;
    }
    
    auto stream_config = vc.GetStreamConfig(stream_id);
    config->enabled = stream_config.enabled;
    config->width = stream_config.width;
    config->height = stream_config.height;
    config->fps = stream_config.fps;
    config->codec = video_codec_to_onvif(stream_config.codec);
    config->profile = h264_profile_to_onvif(stream_config.profile);
    config->gop = stream_config.gop;
    config->rc_mode = rc_mode_to_onvif(stream_config.rc_mode);
    config->bitrate = stream_config.bitrate;
    config->max_bitrate = stream_config.max_bitrate;
    config->min_bitrate = stream_config.min_bitrate;
    config->quality = stream_config.quality;
    config->init_qp = stream_config.init_qp;
    config->min_qp = stream_config.min_qp;
    config->max_qp = stream_config.max_qp;
    
    return 0;
}

int onvif_set_video_config(int stream_idx, const OnvifVideoStreamConfig *config) {
    if (!config) return -1;
    
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) {
        spdlog::warn("VideoControl not initialized");
        return -1;
    }
    
    VideoStreamConfig stream_config;
    stream_config.stream_id = stream_id;
    stream_config.enabled = config->enabled;
    stream_config.width = config->width;
    stream_config.height = config->height;
    stream_config.fps = config->fps;
    stream_config.codec = onvif_to_video_codec(config->codec);
    stream_config.profile = onvif_to_h264_profile(config->profile);
    stream_config.gop = config->gop;
    stream_config.rc_mode = onvif_to_rc_mode(config->rc_mode);
    stream_config.bitrate = config->bitrate;
    stream_config.max_bitrate = config->max_bitrate;
    stream_config.min_bitrate = config->min_bitrate;
    stream_config.quality = config->quality;
    stream_config.init_qp = config->init_qp;
    stream_config.min_qp = config->min_qp;
    stream_config.max_qp = config->max_qp;
    
    if (vc.SetStreamConfig(stream_id, stream_config)) {
        return 0;
    }
    return -1;
}

int onvif_set_resolution(int stream_idx, int width, int height) {
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) return -1;
    
    if (vc.SetResolution(stream_id, width, height)) {
        return 0;
    }
    return -1;
}

int onvif_set_framerate(int stream_idx, int fps) {
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) return -1;
    
    if (vc.SetFrameRate(stream_id, fps)) {
        return 0;
    }
    return -1;
}

int onvif_set_video_codec(int stream_idx, OnvifVideoCodec codec) {
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) return -1;
    
    if (vc.SetCodec(stream_id, onvif_to_video_codec(codec))) {
        return 0;
    }
    return -1;
}

int onvif_set_video_bitrate(int stream_idx, int bitrate) {
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) return -1;
    
    if (vc.SetBitrate(stream_id, bitrate)) {
        return 0;
    }
    return -1;
}

int onvif_set_video_gop(int stream_idx, int gop) {
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) return -1;
    
    if (vc.SetGOP(stream_id, gop)) {
        return 0;
    }
    return -1;
}

int onvif_set_h264_profile(int stream_idx, OnvifH264Profile profile) {
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) return -1;
    
    if (vc.SetProfile(stream_id, onvif_to_h264_profile(profile))) {
        return 0;
    }
    return -1;
}

int onvif_set_video_quality(int stream_idx, int quality) {
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) return -1;
    
    if (vc.SetQuality(stream_id, quality)) {
        return 0;
    }
    return -1;
}

int onvif_set_qp_params(int stream_idx, int init_qp, int min_qp, int max_qp) {
    const char* stream_id = get_stream_id(stream_idx);
    if (!stream_id) return -1;
    
    auto& vc = VideoControl::Instance();
    if (!vc.IsInitialized()) return -1;
    
    if (vc.SetQP(stream_id, init_qp, min_qp, max_qp)) {
        return 0;
    }
    return -1;
}

int onvif_get_resolution_count(void) {
    // Return common IP camera resolutions
    return 8;
}

int onvif_get_supported_resolution(int idx, int *width, int *height) {
    if (!width || !height) return -1;
    
    // Common IP camera resolutions
    static const int resolutions[][2] = {
        {3840, 2160},  // 4K UHD
        {2560, 1440},  // QHD
        {1920, 1080},  // Full HD
        {1280, 720},   // HD
        {1024, 768},   // XGA
        {800, 600},    // SVGA
        {640, 480},    // VGA
        {320, 240}     // QVGA
    };
    
    if (idx < 0 || idx >= 8) return -1;
    
    *width = resolutions[idx][0];
    *height = resolutions[idx][1];
    return 0;
}

int onvif_get_stream_uri(int stream_idx, char *uri, int uri_size) {
    if (!uri || uri_size <= 0) return -1;
    if (stream_idx < 0 || stream_idx >= MAX_STREAMS) return -1;
    
    // Get IP address from environment or use localhost
    const char* ip = getenv("DEVICE_IP");
    if (!ip) ip = "192.168.1.1";
    
    // Construct RTSP URI based on stream index
    const char* stream_names[] = {"stream1", "stream2", "stream3"};
    snprintf(uri, uri_size, "rtsp://%s:554/%s", ip, stream_names[stream_idx]);
    
    return 0;
}

int onvif_get_snapshot_uri(char *uri, int uri_size) {
    if (!uri || uri_size <= 0) return -1;
    
    const char* ip = getenv("DEVICE_IP");
    if (!ip) ip = "192.168.1.1";
    
    snprintf(uri, uri_size, "http://%s/cgi-bin/snapshot.cgi", ip);
    return 0;
}

} // extern "C"
