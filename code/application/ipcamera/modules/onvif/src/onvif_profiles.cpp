/**
 * @file onvif_profiles.cpp
 * @brief ONVIF Profile Support Implementation
 * 
 * Implements ONVIF profile support functions that aggregate functionality
 * from various ipcamera modules.
 */

#include "ipcam/onvif_profiles.h"
#include "onvif_token_defs.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <sys/statvfs.h>
#include <sys/reboot.h>
#include <ftw.h>
#include <glob.h>
#include <cstdio>

// ============================================================================
// Constants
// ============================================================================
static const char* ONVIF_VERSION = "24.06";
static const char* MANUFACTURER = "Novatek";
static const char* MODEL = "NT98538 IP Camera";
static const char* HARDWARE_ID = "Rev A";

// Storage for subscription IDs
static int g_next_subscription_id = 1;
static char g_last_subscription_id[64] = {0};

// ============================================================================
// Safe file/directory removal helpers (replaces system() calls)
// ============================================================================
static int _nftw_remove_cb(const char *path, const struct stat *sb,
                           int typeflag, struct FTW *ftwbuf) {
    (void)sb; (void)typeflag; (void)ftwbuf;
    return remove(path);
}

/** Remove files matching a glob pattern (e.g. "/etc/ipcamera/*.db"). */
static void safe_remove_glob(const char *pattern) {
    glob_t g;
    if (glob(pattern, GLOB_NOSORT, NULL, &g) == 0) {
        for (size_t i = 0; i < g.gl_pathc; i++)
            remove(g.gl_pathv[i]);
        globfree(&g);
    }
}

/** Recursively remove a directory tree (equivalent to rm -rf). */
static void safe_remove_dir(const char *path) {
    nftw(path, _nftw_remove_cb, 64, FTW_DEPTH | FTW_PHYS);
}

extern "C" {

// ============================================================================
// Profile Capabilities
// ============================================================================

int onvif_profile_s_get_capabilities(OnvifProfileCapabilities *caps) {
    if (!caps) return -1;
    
    memset(caps, 0, sizeof(OnvifProfileCapabilities));
    
    caps->profile_s_supported = true;   // Streaming - always supported
    caps->profile_g_supported = true;   // Recording - SD card support
    caps->profile_t_supported = true;   // Analytics - AI engine
    caps->profile_m_supported = true;   // Metadata - via events
    caps->profile_c_supported = false;  // Access Control - not supported
    caps->profile_a_supported = false;  // Outdoor PTZ - optional
    
    caps->ptz_supported = false;        // Digital PTZ possible, no mechanical
    caps->audio_supported = true;       // Audio input/output
    caps->relay_outputs_supported = false;  // No GPIO relay
    caps->ws_discovery_supported = true;
    caps->ws_subscription_supported = true;
    
    return 0;
}

int onvif_is_profile_s_supported(void) {
    return 1;
}

int onvif_is_profile_t_supported(void) {
    return 1;  // Analytics supported via AI engine
}

int onvif_is_profile_g_supported(void) {
    return 1;  // Recording supported via SD card/NAS
}

// ============================================================================
// Video Source Functions
// ============================================================================

int onvif_get_video_source_count(void) {
    return 1;  // Single camera module
}

int onvif_get_video_source_token(int idx, char *token, int token_size) {
    if (!token || token_size <= 0 || idx != 0) return -1;
    ONVIF_TOKEN_GEN(token, token_size, ONVIF_TOKEN_FMT_VIDEO_SOURCE, idx);
    return 0;
}

int onvif_get_video_source_by_token(const char *token, int *width, int *height, int *fps) {
    if (!token) return -1;
    
    // Parse index from token
    int idx = 0;
    if (sscanf(token, "VideoSource_%d", &idx) != 1) {
        return -1;
    }
    
    // Get main stream configuration
    OnvifVideoStreamConfig config;
    if (onvif_get_video_config(0, &config) != 0) {
        return -1;
    }
    
    if (width) *width = config.width;
    if (height) *height = config.height;
    if (fps) *fps = config.fps;
    
    return 0;
}

// ============================================================================
// Media Profile Functions
// ============================================================================

int onvif_get_media_profile_count(void) {
    return onvif_get_stream_count();
}

int onvif_get_media_profile_token(int idx, char *token, int token_size) {
    if (!token || token_size <= 0) return -1;
    if (idx < 0 || idx >= onvif_get_stream_count()) return -1;
    
    ONVIF_TOKEN_GEN(token, token_size, ONVIF_TOKEN_FMT_PROFILE, onvif_channel_to_profile_num(idx));
    return 0;
}

// ============================================================================
// Analytics Functions
// ============================================================================

uint32_t onvif_get_supported_analytics_types(void) {
    uint32_t types = 0;
    
    if (onvif_is_analytics_supported(ONVIF_ANALYTICS_MOTION)) types |= (1 << ONVIF_ANALYTICS_MOTION);
    if (onvif_is_analytics_supported(ONVIF_ANALYTICS_PERSON)) types |= (1 << ONVIF_ANALYTICS_PERSON);
    if (onvif_is_analytics_supported(ONVIF_ANALYTICS_VEHICLE)) types |= (1 << ONVIF_ANALYTICS_VEHICLE);
    if (onvif_is_analytics_supported(ONVIF_ANALYTICS_FACE)) types |= (1 << ONVIF_ANALYTICS_FACE);
    if (onvif_is_analytics_supported(ONVIF_ANALYTICS_LINE_CROSS)) types |= (1 << ONVIF_ANALYTICS_LINE_CROSS);
    if (onvif_is_analytics_supported(ONVIF_ANALYTICS_INTRUSION)) types |= (1 << ONVIF_ANALYTICS_INTRUSION);
    
    return types;
}

int onvif_get_analytics_config_count(void) {
    return onvif_get_stream_count();  // One analytics config per stream
}

int onvif_get_analytics_config_token(int idx, char *token, int token_size) {
    if (!token || token_size <= 0) return -1;
    if (idx < 0 || idx >= onvif_get_stream_count()) return -1;
    
    ONVIF_TOKEN_GEN(token, token_size, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS, onvif_channel_to_profile_num(idx));
    return 0;
}

int onvif_create_cell_motion_rule(const char *config_token, const char *rule_name, int sensitivity) {
    if (!config_token || !rule_name) return -1;
    
    OnvifMotionConfig config;
    onvif_get_motion_config(&config);
    
    config.enabled = true;
    config.sensitivity = sensitivity;
    
    return onvif_set_motion_config(&config);
}

int onvif_create_line_crossing_rule(const char *config_token, const char *rule_name,
                                     float x1, float y1, float x2, float y2, bool bidirectional) {
    if (!config_token || !rule_name) return -1;
    
    OnvifLineCrossZone zone;
    memset(&zone, 0, sizeof(zone));
    strncpy(zone.name, rule_name, sizeof(zone.name) - 1);
    zone.x1 = x1;
    zone.y1 = y1;
    zone.x2 = x2;
    zone.y2 = y2;
    zone.bidirectional = bidirectional;
    zone.enabled = true;
    
    return onvif_add_line_cross_zone(&zone);
}

// ============================================================================
// Recording/Storage Functions
// ============================================================================

int onvif_get_recording_job_count(void) {
    return 0;  // TODO: Implement recording job tracking
}

int onvif_get_storage_info(uint64_t *capacity_mb, uint64_t *used_mb) {
    if (!capacity_mb || !used_mb) return -1;
    
    struct statvfs stat;
    
    // Check SD card mount point
    if (statvfs("/mnt/sdcard", &stat) == 0) {
        *capacity_mb = (uint64_t)(stat.f_blocks * stat.f_frsize) / (1024 * 1024);
        *used_mb = (uint64_t)((stat.f_blocks - stat.f_bfree) * stat.f_frsize) / (1024 * 1024);
        return 0;
    }
    
    // Fallback to check internal storage
    if (statvfs("/mnt/app", &stat) == 0) {
        *capacity_mb = (uint64_t)(stat.f_blocks * stat.f_frsize) / (1024 * 1024);
        *used_mb = (uint64_t)((stat.f_blocks - stat.f_bfree) * stat.f_frsize) / (1024 * 1024);
        return 0;
    }
    
    *capacity_mb = 0;
    *used_mb = 0;
    return -1;
}

// ============================================================================
// Device Management Functions
// ============================================================================

int onvif_get_device_info(OnvifDeviceInfo *info) {
    if (!info) return -1;
    
    memset(info, 0, sizeof(OnvifDeviceInfo));
    
    strncpy(info->manufacturer, MANUFACTURER, sizeof(info->manufacturer) - 1);
    strncpy(info->model, MODEL, sizeof(info->model) - 1);
    strncpy(info->firmware_version, "1.0.0", sizeof(info->firmware_version) - 1);
    strncpy(info->hardware_id, HARDWARE_ID, sizeof(info->hardware_id) - 1);
    
    // Get serial number from MAC address
    char mac[18] = {0};
    onvif_get_mac_address("eth0", mac);
    snprintf(info->serial_number, sizeof(info->serial_number), "%s", mac);
    
    // Device ID
    snprintf(info->device_id, sizeof(info->device_id), "urn:uuid:%s", mac);
    
    info->max_streams = 3;
    info->max_analytics_engines = 1;
    info->max_rules = 8;
    
    return 0;
}

int onvif_set_system_datetime(int year, int month, int day,
                               int hour, int minute, int second, bool use_utc) {
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    
    time_t t;
    if (use_utc) {
        t = timegm(&tm);
    } else {
        t = mktime(&tm);
    }
    
    if (t == -1) return -1;
    
    // Set system time (requires root)
    struct timeval tv;
    tv.tv_sec = t;
    tv.tv_usec = 0;
    
    if (settimeofday(&tv, NULL) != 0) {
        spdlog::error("Failed to set system time");
        return -1;
    }
    
    return 0;
}

int onvif_get_system_datetime(int *year, int *month, int *day,
                               int *hour, int *minute, int *second, bool *is_utc) {
    time_t now = time(NULL);
    struct tm *tm_info = gmtime(&now);  // UTC
    
    if (year) *year = tm_info->tm_year + 1900;
    if (month) *month = tm_info->tm_mon + 1;
    if (day) *day = tm_info->tm_mday;
    if (hour) *hour = tm_info->tm_hour;
    if (minute) *minute = tm_info->tm_min;
    if (second) *second = tm_info->tm_sec;
    if (is_utc) *is_utc = true;
    
    return 0;
}

int onvif_get_timezone(char *tz, int tz_size) {
    if (!tz || tz_size <= 0) return -1;
    
    const char* tzenv = getenv("TZ");
    if (tzenv) {
        strncpy(tz, tzenv, tz_size - 1);
    } else {
        strncpy(tz, "UTC", tz_size - 1);
    }
    
    return 0;
}

int onvif_set_timezone(const char *tz) {
    if (!tz) return -1;
    
    setenv("TZ", tz, 1);
    tzset();
    
    return 0;
}

int onvif_system_reboot(void) {
    spdlog::info("System reboot requested via ONVIF");
    sync();
    
    // Schedule reboot after 2 seconds to allow response
    if (fork() == 0) {
        sleep(2);
        reboot(RB_AUTOBOOT);
        _exit(0);  // Only reached if reboot() fails
    }
    
    return 0;
}

int onvif_factory_reset(bool hard_reset) {
    spdlog::info("Factory reset requested via ONVIF (hard={})", hard_reset);
    
    if (hard_reset) {
        // Hard reset - remove user configurations but preserve factory defaults
        // IMPORTANT: Keep config.factory.d folder intact!
        spdlog::warn("Performing hard factory reset - preserving config.factory.d");
        
        // Remove user database and settings (but not factory defaults)
        safe_remove_glob("/etc/ipcamera/*.db");
        safe_remove_glob("/etc/ipcamera/*.json");
        safe_remove_dir("/etc/ipcamera/certs");
        
        // Remove runtime configs but keep factory
        safe_remove_dir("/mnt/app/config/config.d");
        // DO NOT delete config.factory.d - this contains factory defaults!
        
        // Remove user data
        safe_remove_dir("/mnt/app/recordings");
        safe_remove_dir("/mnt/app/snapshots");
    } else {
        // Soft reset - only reset user settings, keep user accounts and certificates
        spdlog::info("Performing soft factory reset");
        remove("/etc/ipcamera/streaming.json");
        remove("/etc/ipcamera/network.json");
        remove("/etc/ipcamera/analytics.json");
        safe_remove_dir("/mnt/app/config/config.d");
    }
    
    return onvif_system_reboot();
}

int onvif_get_scopes(char *scopes, int scopes_size) {
    if (!scopes || scopes_size <= 0) return -1;
    
    const char* default_scopes = 
        "onvif://www.onvif.org/type/Network_Video_Transmitter\n"
        "onvif://www.onvif.org/Profile/Streaming\n"
        "onvif://www.onvif.org/Profile/T\n"
        "onvif://www.onvif.org/Profile/G\n"
        "onvif://www.onvif.org/Profile/M\n"
        "onvif://www.onvif.org/hardware/Honeywell_IPCAM-5MP\n"
        "onvif://www.onvif.org/name/Honeywell_Eterna\n"
        "onvif://www.onvif.org/location/Undefined\n";
    
    strncpy(scopes, default_scopes, scopes_size - 1);
    
    // Count scopes (number of newlines)
    int count = 0;
    for (const char* p = scopes; *p; p++) {
        if (*p == '\n') count++;
    }
    
    return count;
}

int onvif_add_scope(const char *scope) {
    (void)scope;
    // TODO: Implement scope management
    return 0;
}

int onvif_remove_scope(const char *scope) {
    (void)scope;
    // TODO: Implement scope management
    return 0;
}

// ============================================================================
// PTZ Functions (Stub - for cameras without mechanical PTZ)
// ============================================================================

int onvif_is_ptz_supported(void) {
    return 0;  // PTZ not supported on this camera
}

int onvif_get_ptz_config_count(void) {
    return 0;
}

int onvif_ptz_continuous_move(const char *profile_token, float pan, float tilt, float zoom) {
    (void)profile_token; (void)pan; (void)tilt; (void)zoom;
    return -1;  // Not supported
}

int onvif_ptz_stop(const char *profile_token, bool stop_pan, bool stop_tilt) {
    (void)profile_token; (void)stop_pan; (void)stop_tilt;
    return -1;  // Not supported
}

int onvif_ptz_goto_home(const char *profile_token) {
    (void)profile_token;
    return -1;  // Not supported
}

int onvif_ptz_set_home(const char *profile_token) {
    (void)profile_token;
    return -1;  // Not supported
}

int onvif_ptz_goto_preset(const char *profile_token, const char *preset_token) {
    (void)profile_token; (void)preset_token;
    return -1;  // Not supported
}

// ============================================================================
// Event Subscription Functions (Stub)
// ============================================================================

int onvif_create_pullpoint_subscription(int initial_termination_time,
                                         char *subscription_id, int subscription_id_size) {
    if (!subscription_id || subscription_id_size <= 0) return -1;
    
    snprintf(subscription_id, subscription_id_size, "Subscription_%d", g_next_subscription_id++);
    strncpy(g_last_subscription_id, subscription_id, sizeof(g_last_subscription_id) - 1);
    
    (void)initial_termination_time;  // TODO: Implement termination
    
    return 0;
}

int onvif_pull_messages(const char *subscription_id, int timeout_ms,
                        char *messages, int messages_size) {
    if (!subscription_id || !messages || messages_size <= 0) return -1;
    
    (void)timeout_ms;
    
    // TODO: Implement actual event queue
    // For now, return empty
    messages[0] = '\0';
    
    return 0;
}

int onvif_renew_subscription(const char *subscription_id, int termination_time) {
    (void)subscription_id; (void)termination_time;
    return 0;
}

int onvif_unsubscribe(const char *subscription_id) {
    (void)subscription_id;
    return 0;
}

// ============================================================================
// Initialization
// ============================================================================

int onvif_profiles_init(void) {
    spdlog::info("ONVIF Profile Support Initialized (version {})", ONVIF_VERSION);
    return 0;
}

void onvif_profiles_cleanup(void) {
    spdlog::info("ONVIF Profile Support Cleanup");
}

const char* onvif_get_version(void) {
    return ONVIF_VERSION;
}

} // extern "C"
