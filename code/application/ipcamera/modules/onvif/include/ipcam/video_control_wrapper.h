/**
 * @file video_control_wrapper.h
 * @brief C wrapper for C++ Video Control functions for ONVIF integration
 * 
 * Provides C-callable functions to interact with the ipcam::media::VideoControl
 * class from ONVIF C code.
 */

#ifndef ONVIF_VIDEO_CONTROL_WRAPPER_H
#define ONVIF_VIDEO_CONTROL_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// Video Codec Types
// ============================================================================
typedef enum {
    ONVIF_VIDEO_CODEC_H264 = 0,
    ONVIF_VIDEO_CODEC_H265 = 1,
    ONVIF_VIDEO_CODEC_MJPEG = 2
} OnvifVideoCodec;

typedef enum {
    ONVIF_H264_PROFILE_BASELINE = 0,
    ONVIF_H264_PROFILE_MAIN = 1,
    ONVIF_H264_PROFILE_HIGH = 2
} OnvifH264Profile;

typedef enum {
    ONVIF_RATE_CONTROL_CBR = 0,
    ONVIF_RATE_CONTROL_VBR = 1,
    ONVIF_RATE_CONTROL_CVBR = 2
} OnvifRateControlMode;

// ============================================================================
// Video Stream Configuration
// ============================================================================
typedef struct {
    bool enabled;
    int width;
    int height;
    int fps;
    OnvifVideoCodec codec;
    OnvifH264Profile profile;
    int gop;
    OnvifRateControlMode rc_mode;
    int bitrate;       // kbps
    int max_bitrate;   // kbps
    int min_bitrate;   // kbps
    int quality;       // 1-100
    int init_qp;
    int min_qp;
    int max_qp;
} OnvifVideoStreamConfig;

// ============================================================================
// Video Control Functions
// ============================================================================

/**
 * @brief Get video stream count
 * @return Number of video streams
 */
int onvif_get_stream_count(void);

/**
 * @brief Get video stream configuration
 * @param stream_idx Stream index (0-2)
 * @param config Output configuration structure
 * @return 0 on success, -1 on error
 */
int onvif_get_video_config(int stream_idx, OnvifVideoStreamConfig *config);

/**
 * @brief Set video stream configuration
 * @param stream_idx Stream index (0-2)
 * @param config Configuration to apply
 * @return 0 on success, -1 on error
 */
int onvif_set_video_config(int stream_idx, const OnvifVideoStreamConfig *config);

/**
 * @brief Set video resolution
 * @param stream_idx Stream index (0-2)
 * @param width Width in pixels
 * @param height Height in pixels
 * @return 0 on success, -1 on error
 */
int onvif_set_resolution(int stream_idx, int width, int height);

/**
 * @brief Set video frame rate
 * @param stream_idx Stream index (0-2)
 * @param fps Frame rate
 * @return 0 on success, -1 on error
 */
int onvif_set_framerate(int stream_idx, int fps);

/**
 * @brief Set video codec
 * @param stream_idx Stream index (0-2)
 * @param codec Codec type
 * @return 0 on success, -1 on error
 */
int onvif_set_video_codec(int stream_idx, OnvifVideoCodec codec);

/**
 * @brief Set video bitrate
 * @param stream_idx Stream index (0-2)
 * @param bitrate Bitrate in kbps
 * @return 0 on success, -1 on error
 */
int onvif_set_video_bitrate(int stream_idx, int bitrate);

/**
 * @brief Set video GOP (Group of Pictures)
 * @param stream_idx Stream index (0-2)
 * @param gop GOP size
 * @return 0 on success, -1 on error
 */
int onvif_set_video_gop(int stream_idx, int gop);

/**
 * @brief Set H.264 profile
 * @param stream_idx Stream index (0-2)
 * @param profile Profile type
 * @return 0 on success, -1 on error
 */
int onvif_set_h264_profile(int stream_idx, OnvifH264Profile profile);

/**
 * @brief Set video quality (for VBR mode)
 * @param stream_idx Stream index (0-2)
 * @param quality Quality level (1-100)
 * @return 0 on success, -1 on error
 */
int onvif_set_video_quality(int stream_idx, int quality);

/**
 * @brief Set QP parameters
 * @param stream_idx Stream index (0-2)
 * @param init_qp Initial QP
 * @param min_qp Minimum QP
 * @param max_qp Maximum QP
 * @return 0 on success, -1 on error
 */
int onvif_set_qp_params(int stream_idx, int init_qp, int min_qp, int max_qp);

/**
 * @brief Get supported resolution count
 * @return Number of supported resolutions
 */
int onvif_get_resolution_count(void);

/**
 * @brief Get supported resolution by index
 * @param idx Resolution index
 * @param width Output width
 * @param height Output height
 * @return 0 on success, -1 on error
 */
int onvif_get_supported_resolution(int idx, int *width, int *height);

/**
 * @brief Get RTSP stream URI
 * @param stream_idx Stream index (0-2)
 * @param uri Output buffer for URI
 * @param uri_size Size of output buffer
 * @return 0 on success, -1 on error
 */
int onvif_get_stream_uri(int stream_idx, char *uri, int uri_size);

/**
 * @brief Get snapshot URI
 * @param uri Output buffer for URI  
 * @param uri_size Size of output buffer
 * @return 0 on success, -1 on error
 */
int onvif_get_snapshot_uri(char *uri, int uri_size);

#ifdef __cplusplus
}
#endif

#endif /* ONVIF_VIDEO_CONTROL_WRAPPER_H */
