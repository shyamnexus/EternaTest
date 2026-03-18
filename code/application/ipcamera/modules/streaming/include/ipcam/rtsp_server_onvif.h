/**
 * @file rtsp_server_onvif.h
 * @brief C Interface for RTSP Server ONVIF Integration
 * 
 * This header provides C-compatible functions for the ONVIF module
 * to query RTSP server stream information. The ONVIF module is written
 * in C, so it cannot include the main C++ header directly.
 * 
 * Copyright (c) 2025
 */

#ifndef IPCAM_RTSP_SERVER_ONVIF_H
#define IPCAM_RTSP_SERVER_ONVIF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get RTSP stream URI for ONVIF profile
 * @param profile_token ONVIF profile token (e.g., "Profile_1", "Profile_2", "Profile_3")
 * @param ip_address IP address to use in the URI
 * @param uri_buffer Buffer to store the URI
 * @param buffer_size Size of the buffer
 * @return 1 on success, 0 on failure
 * 
 * This function is called by the ONVIF module to get stream URIs.
 * The URI format is: rtsp://<ip>:<port>/stream<n>
 * 
 * Profile token mapping (3 profiles to match HDAL encoders):
 * - "Profile_1" -> stream0 (main stream, typically 1080p H.264/H.265)
 * - "Profile_2" -> stream1 (sub stream, typically 720p/480p)
 * - "Profile_3" -> stream2 (third stream, typically low-res for mobile)
 */
int rtsp_get_onvif_stream_uri(const char* profile_token, const char* ip_address,
                               char* uri_buffer, int buffer_size);

/**
 * @brief Get stream video info for ONVIF profile
 * @param profile_token ONVIF profile token
 * @param[out] width Video width
 * @param[out] height Video height
 * @param[out] fps Frame rate
 * @param[out] is_h265 1 if H.265, 0 if H.264
 * @return 1 on success, 0 if profile not found
 */
int rtsp_get_onvif_stream_info(const char* profile_token, 
                                int* width, int* height, int* fps, int* is_h265);

/**
 * @brief Check if RTSP server is running
 * @return 1 if running, 0 if not
 */
int rtsp_server_is_running(void);

/**
 * @brief Get RTSP server port
 * @return RTSP port number (typically 554)
 */
int rtsp_server_get_port(void);

/**
 * @brief Get number of active RTSP streams
 * @return Number of configured streams
 */
int rtsp_server_get_stream_count(void);

#ifdef __cplusplus
}
#endif

#endif // IPCAM_RTSP_SERVER_ONVIF_H
