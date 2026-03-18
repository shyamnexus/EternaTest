/**
 * @file onvif_profiles.h
 * @brief ONVIF Profile Support for IP Camera
 * 
 * This header provides unified access to ONVIF profile functionalities:
 * - Profile S (Streaming)
 * - Profile G (Recording)
 * - Profile T (Video Analytics)
 * - Profile M (Metadata)
 * 
 * All functions in this header can be called from ONVIF C code to access
 * the C++ ipcamera modules.
 */

#ifndef ONVIF_PROFILES_H
#define ONVIF_PROFILES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// Include all wrapper headers
#include "ipcam/video_control_wrapper.h"
#include "ipcam/audio_control_wrapper.h"
#include "ipcam/network_wrapper.h"
#include "ipcam/analytics_wrapper.h"
#include "ipcam/osd_wrapper.h"
#include "ipcam/user_wrapper.h"
#include "ipcam/image_adjustment_wrapper.h"

// ============================================================================
// Profile Capabilities
// ============================================================================

typedef struct {
    bool profile_s_supported;   // Streaming Profile
    bool profile_g_supported;   // Recording Profile
    bool profile_t_supported;   // Analytics Profile
    bool profile_m_supported;   // Metadata Profile
    bool profile_c_supported;   // Access Control Profile
    bool profile_a_supported;   // Outdoor Profile (PTZ)
    bool ptz_supported;
    bool audio_supported;
    bool relay_outputs_supported;
    bool ws_discovery_supported;
    bool ws_subscription_supported;
} OnvifProfileCapabilities;

// ============================================================================
// Device Information
// ============================================================================

typedef struct {
    char manufacturer[64];
    char model[64];
    char firmware_version[32];
    char serial_number[64];
    char hardware_id[32];
    char device_id[64];
    int max_streams;
    int max_analytics_engines;
    int max_rules;
} OnvifDeviceInfo;

// ============================================================================
// Profile S (Streaming) Functions
// ============================================================================

/**
 * @brief Get Profile S capabilities
 * @param caps Output capabilities
 * @return 0 on success
 */
int onvif_profile_s_get_capabilities(OnvifProfileCapabilities *caps);

/**
 * @brief Check if Profile S is supported
 * @return 1 if supported
 */
int onvif_is_profile_s_supported(void);

/**
 * @brief Get number of video sources
 * @return Number of video sources (typically 1)
 */
int onvif_get_video_source_count(void);

/**
 * @brief Get video source token
 * @param idx Video source index
 * @param token Output buffer for token
 * @param token_size Buffer size
 * @return 0 on success
 */
int onvif_get_video_source_token(int idx, char *token, int token_size);

/**
 * @brief Get video source configuration
 * @param token Video source token
 * @param width Output width
 * @param height Output height
 * @param fps Output frame rate
 * @return 0 on success
 */
int onvif_get_video_source_by_token(const char *token, int *width, int *height, int *fps);

/**
 * @brief Get media profile count
 * @return Number of media profiles (typically 3)
 */
int onvif_get_media_profile_count(void);

/**
 * @brief Get media profile token
 * @param idx Profile index
 * @param token Output buffer for token
 * @param token_size Buffer size
 * @return 0 on success
 */
int onvif_get_media_profile_token(int idx, char *token, int token_size);

// ============================================================================
// Profile T (Analytics) Functions
// ============================================================================

/**
 * @brief Check if Profile T is supported
 * @return 1 if supported
 */
int onvif_is_profile_t_supported(void);

/**
 * @brief Get supported analytics module types
 * @return Bitmask of supported types
 */
uint32_t onvif_get_supported_analytics_types(void);

/**
 * @brief Get analytics configuration count
 * @return Number of analytics configurations
 */
int onvif_get_analytics_config_count(void);

/**
 * @brief Get analytics configuration token
 * @param idx Configuration index
 * @param token Output buffer for token
 * @param token_size Buffer size
 * @return 0 on success
 */
int onvif_get_analytics_config_token(int idx, char *token, int token_size);

/**
 * @brief Create cell motion detector rule
 * @param config_token Analytics configuration token
 * @param rule_name Rule name
 * @param sensitivity Sensitivity (0-100)
 * @return 0 on success
 */
int onvif_create_cell_motion_rule(const char *config_token, const char *rule_name, int sensitivity);

/**
 * @brief Create line crossing detector rule
 * @param config_token Analytics configuration token
 * @param rule_name Rule name
 * @param x1, y1, x2, y2 Line coordinates (normalized 0-1)
 * @param bidirectional True for both directions
 * @return 0 on success
 */
int onvif_create_line_crossing_rule(const char *config_token, const char *rule_name,
                                     float x1, float y1, float x2, float y2, bool bidirectional);

// ============================================================================
// Profile G (Recording) Functions
// ============================================================================

/**
 * @brief Check if Profile G is supported
 * @return 1 if supported
 */
int onvif_is_profile_g_supported(void);

/**
 * @brief Get recording job count
 * @return Number of recording jobs
 */
int onvif_get_recording_job_count(void);

/**
 * @brief Get storage info
 * @param capacity_mb Output capacity in MB
 * @param used_mb Output used space in MB
 * @return 0 on success
 */
int onvif_get_storage_info(uint64_t *capacity_mb, uint64_t *used_mb);

// ============================================================================
// Device Management Functions
// ============================================================================

/**
 * @brief Get device information
 * @param info Output device info
 * @return 0 on success
 */
int onvif_get_device_info(OnvifDeviceInfo *info);

/**
 * @brief Set system date and time
 * @param year, month, day Date components
 * @param hour, minute, second Time components
 * @param use_utc If true, time is UTC
 * @return 0 on success
 */
int onvif_set_system_datetime(int year, int month, int day, 
                               int hour, int minute, int second, bool use_utc);

/**
 * @brief Get system date and time
 * @param year, month, day Output date components
 * @param hour, minute, second Output time components
 * @param is_utc Output UTC flag
 * @return 0 on success
 */
int onvif_get_system_datetime(int *year, int *month, int *day,
                               int *hour, int *minute, int *second, bool *is_utc);

/**
 * @brief Get timezone
 * @param tz Output timezone string (e.g., "UTC+05:30")
 * @param tz_size Buffer size
 * @return 0 on success
 */
int onvif_get_timezone(char *tz, int tz_size);

/**
 * @brief Set timezone
 * @param tz Timezone string
 * @return 0 on success
 */
int onvif_set_timezone(const char *tz);

/**
 * @brief System reboot
 * @return 0 on success (will reboot)
 */
int onvif_system_reboot(void);

/**
 * @brief Factory reset
 * @param hard_reset If true, perform hard reset
 * @return 0 on success
 */
int onvif_factory_reset(bool hard_reset);

/**
 * @brief Get scope URIs
 * @param scopes Output buffer for scope URIs (newline separated)
 * @param scopes_size Buffer size
 * @return Number of scopes
 */
int onvif_get_scopes(char *scopes, int scopes_size);

/**
 * @brief Add scope URI
 * @param scope Scope URI to add
 * @return 0 on success
 */
int onvif_add_scope(const char *scope);

/**
 * @brief Remove scope URI
 * @param scope Scope URI to remove
 * @return 0 on success
 */
int onvif_remove_scope(const char *scope);

// ============================================================================
// PTZ Functions (if supported)
// ============================================================================

/**
 * @brief Check if PTZ is supported
 * @return 1 if supported
 */
int onvif_is_ptz_supported(void);

/**
 * @brief Get PTZ configuration count
 * @return Number of PTZ configurations
 */
int onvif_get_ptz_config_count(void);

/**
 * @brief PTZ continuous move
 * @param profile_token Media profile token
 * @param pan Pan speed (-1 to 1)
 * @param tilt Tilt speed (-1 to 1)
 * @param zoom Zoom speed (-1 to 1)
 * @return 0 on success
 */
int onvif_ptz_continuous_move(const char *profile_token, float pan, float tilt, float zoom);

/**
 * @brief PTZ stop
 * @param profile_token Media profile token
 * @param stop_pan Stop pan movement
 * @param stop_tilt Stop tilt movement
 * @return 0 on success
 */
int onvif_ptz_stop(const char *profile_token, bool stop_pan, bool stop_tilt);

/**
 * @brief PTZ go to home position
 * @param profile_token Media profile token
 * @return 0 on success
 */
int onvif_ptz_goto_home(const char *profile_token);

/**
 * @brief PTZ set home position
 * @param profile_token Media profile token
 * @return 0 on success
 */
int onvif_ptz_set_home(const char *profile_token);

/**
 * @brief PTZ go to preset
 * @param profile_token Media profile token
 * @param preset_token Preset token
 * @return 0 on success
 */
int onvif_ptz_goto_preset(const char *profile_token, const char *preset_token);

// ============================================================================
// Event Subscription Functions
// ============================================================================

/**
 * @brief Create pull point subscription
 * @param initial_termination_time Termination time in seconds
 * @param subscription_id Output subscription ID
 * @param subscription_id_size Buffer size
 * @return 0 on success
 */
int onvif_create_pullpoint_subscription(int initial_termination_time,
                                         char *subscription_id, int subscription_id_size);

/**
 * @brief Pull messages from subscription
 * @param subscription_id Subscription ID
 * @param timeout_ms Timeout in milliseconds
 * @param messages Output buffer for messages (JSON format)
 * @param messages_size Buffer size
 * @return Number of messages, -1 on error
 */
int onvif_pull_messages(const char *subscription_id, int timeout_ms,
                        char *messages, int messages_size);

/**
 * @brief Renew subscription
 * @param subscription_id Subscription ID
 * @param termination_time New termination time in seconds
 * @return 0 on success
 */
int onvif_renew_subscription(const char *subscription_id, int termination_time);

/**
 * @brief Unsubscribe from subscription
 * @param subscription_id Subscription ID
 * @return 0 on success
 */
int onvif_unsubscribe(const char *subscription_id);

// ============================================================================
// Initialization and Cleanup
// ============================================================================

/**
 * @brief Initialize ONVIF profile support
 * @return 0 on success
 */
int onvif_profiles_init(void);

/**
 * @brief Cleanup ONVIF profile support
 */
void onvif_profiles_cleanup(void);

/**
 * @brief Get ONVIF version string
 * @return Version string (e.g., "24.06")
 */
const char* onvif_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* ONVIF_PROFILES_H */
