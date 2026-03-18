/**
 * @file analytics_wrapper.h
 * @brief C wrapper for C++ Analytics Engine functions for ONVIF integration
 * 
 * Provides C-callable functions to interact with the ipcam::ai::AnalyticsEngine
 * class from ONVIF C code.
 */

#ifndef ONVIF_ANALYTICS_WRAPPER_H
#define ONVIF_ANALYTICS_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// Analytics Module Types
// ============================================================================
typedef enum {
    ONVIF_ANALYTICS_MOTION = 0,
    ONVIF_ANALYTICS_PERSON = 1,
    ONVIF_ANALYTICS_VEHICLE = 2,
    ONVIF_ANALYTICS_FACE = 3,
    ONVIF_ANALYTICS_LINE_CROSS = 4,
    ONVIF_ANALYTICS_INTRUSION = 5
} OnvifAnalyticsType;

// ============================================================================
// Motion Detection Configuration
// ============================================================================
typedef struct {
    bool enabled;
    int sensitivity;    // 0-100
    int threshold;      // Motion threshold
    int min_area;       // Minimum motion area
    int cooldown_ms;    // Cooldown between events
} OnvifMotionConfig;

// ============================================================================
// Motion Zone
// ============================================================================
typedef struct {
    int id;
    char name[64];
    float x;            // Normalized 0-1
    float y;
    float width;
    float height;
    int sensitivity;
    bool enabled;
} OnvifMotionZone;

// ============================================================================
// Line Crossing Configuration
// ============================================================================
typedef struct {
    int id;
    char name[64];
    float x1, y1;       // Start point (normalized)
    float x2, y2;       // End point (normalized)
    bool bidirectional;
    bool enabled;
} OnvifLineCrossZone;

// ============================================================================
// Detection Result
// ============================================================================
typedef struct {
    int id;
    OnvifAnalyticsType type;
    float x1, y1;       // Bounding box (normalized)
    float x2, y2;
    float confidence;
    uint64_t timestamp;
} OnvifDetectionResult;

// ============================================================================
// Analytics Statistics
// ============================================================================
typedef struct {
    uint64_t frames_processed;
    uint64_t total_detections;
    uint64_t motion_events;
    uint64_t line_cross_events;
    float avg_process_time_ms;
    float current_fps;
} OnvifAnalyticsStats;

// ============================================================================
// Analytics Control Functions
// ============================================================================

/**
 * @brief Check if analytics engine is running
 * @return 1 if running, 0 if not
 */
int onvif_analytics_is_running(void);

/**
 * @brief Enable/disable motion detection
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_motion_detection_enabled(bool enabled);

/**
 * @brief Get motion detection configuration
 * @param config Output configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_motion_config(OnvifMotionConfig *config);

/**
 * @brief Set motion detection configuration
 * @param config Configuration to set
 * @return 0 on success, -1 on error
 */
int onvif_set_motion_config(const OnvifMotionConfig *config);

/**
 * @brief Get motion zone count
 * @return Number of motion zones
 */
int onvif_get_motion_zone_count(void);

/**
 * @brief Get motion zone by index
 * @param idx Zone index
 * @param zone Output zone info
 * @return 0 on success, -1 on error
 */
int onvif_get_motion_zone(int idx, OnvifMotionZone *zone);

/**
 * @brief Add motion zone
 * @param zone Zone to add
 * @return Zone ID on success, -1 on error
 */
int onvif_add_motion_zone(const OnvifMotionZone *zone);

/**
 * @brief Update motion zone
 * @param zone Zone to update
 * @return 0 on success, -1 on error
 */
int onvif_update_motion_zone(const OnvifMotionZone *zone);

/**
 * @brief Remove motion zone
 * @param zone_id Zone ID to remove
 * @return 0 on success, -1 on error
 */
int onvif_remove_motion_zone(int zone_id);

/**
 * @brief Enable/disable person detection
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_person_detection_enabled(bool enabled);

/**
 * @brief Enable/disable vehicle detection
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_vehicle_detection_enabled(bool enabled);

/**
 * @brief Enable/disable face detection
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_face_detection_enabled(bool enabled);

/**
 * @brief Enable/disable line crossing detection
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_line_crossing_enabled(bool enabled);

/**
 * @brief Get line crossing zone count
 * @return Number of line crossing zones
 */
int onvif_get_line_cross_zone_count(void);

/**
 * @brief Get line crossing zone by index
 * @param idx Zone index
 * @param zone Output zone info
 * @return 0 on success, -1 on error
 */
int onvif_get_line_cross_zone(int idx, OnvifLineCrossZone *zone);

/**
 * @brief Add line crossing zone
 * @param zone Zone to add
 * @return Zone ID on success, -1 on error
 */
int onvif_add_line_cross_zone(const OnvifLineCrossZone *zone);

/**
 * @brief Remove line crossing zone
 * @param zone_id Zone ID to remove
 * @return 0 on success, -1 on error
 */
int onvif_remove_line_cross_zone(int zone_id);

/**
 * @brief Get analytics statistics
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int onvif_get_analytics_stats(OnvifAnalyticsStats *stats);

/**
 * @brief Reset analytics statistics
 * @return 0 on success, -1 on error
 */
int onvif_reset_analytics_stats(void);

/**
 * @brief Get latest detections
 * @param results Output array of results
 * @param max_results Maximum number of results to return
 * @return Number of results returned, -1 on error
 */
int onvif_get_detections(OnvifDetectionResult *results, int max_results);

/**
 * @brief Check if specific analytics type is supported
 * @param type Analytics type to check
 * @return 1 if supported, 0 if not
 */
int onvif_is_analytics_supported(OnvifAnalyticsType type);

#ifdef __cplusplus
}
#endif

#endif /* ONVIF_ANALYTICS_WRAPPER_H */
