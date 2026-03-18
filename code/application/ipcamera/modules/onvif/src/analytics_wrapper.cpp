/**
 * @file analytics_wrapper.cpp
 * @brief C wrapper implementation for Analytics Engine functions
 * 
 * Implements the C wrapper functions by calling into the C++ AnalyticsEngine class.
 */

#include "ipcam/analytics_wrapper.h"
#include "ipcam/analytics.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <vector>

using namespace ipcam::ai;

// Store latest detections for retrieval
static std::vector<DetectionResult> g_latest_detections;
static std::mutex g_detections_mutex;

// Detection callback to store results
static void detection_callback(const DetectionFrame& frame) {
    std::lock_guard<std::mutex> lock(g_detections_mutex);
    g_latest_detections.clear();
    for (const auto& det : frame.detections) {
        g_latest_detections.push_back(det);
    }
}

// Helper to convert between enum types
static ObjectCategory onvif_to_object_category(OnvifAnalyticsType type) {
    switch (type) {
        case ONVIF_ANALYTICS_MOTION: return ObjectCategory::kMotion;
        case ONVIF_ANALYTICS_PERSON: return ObjectCategory::kPerson;
        case ONVIF_ANALYTICS_VEHICLE: return ObjectCategory::kVehicle;
        case ONVIF_ANALYTICS_FACE: return ObjectCategory::kFace;
        case ONVIF_ANALYTICS_LINE_CROSS: return ObjectCategory::kLineCross;
        default: return ObjectCategory::kUnknown;
    }
}

static OnvifAnalyticsType object_category_to_onvif(ObjectCategory category) {
    switch (category) {
        case ObjectCategory::kMotion: return ONVIF_ANALYTICS_MOTION;
        case ObjectCategory::kPerson: return ONVIF_ANALYTICS_PERSON;
        case ObjectCategory::kVehicle: return ONVIF_ANALYTICS_VEHICLE;
        case ObjectCategory::kFace: return ONVIF_ANALYTICS_FACE;
        case ObjectCategory::kLineCross: return ONVIF_ANALYTICS_LINE_CROSS;
        default: return ONVIF_ANALYTICS_MOTION;
    }
}

extern "C" {

int onvif_analytics_is_running(void) {
    auto& ae = AnalyticsEngine::Instance();
    return ae.IsRunning() ? 1 : 0;
}

int onvif_set_motion_detection_enabled(bool enabled) {
    auto& ae = AnalyticsEngine::Instance();
    if (ae.EnableMotionDetection(enabled)) {
        return 0;
    }
    return -1;
}

int onvif_get_motion_config(OnvifMotionConfig *config) {
    if (!config) return -1;
    
    auto& ae = AnalyticsEngine::Instance();
    auto analytics_config = ae.GetConfig();
    
    config->enabled = analytics_config.motion_detection.enabled;
    config->sensitivity = analytics_config.motion_detection.sensitivity;
    config->threshold = analytics_config.motion_detection.threshold;
    config->min_area = analytics_config.motion_detection.min_area;
    config->cooldown_ms = analytics_config.motion_detection.cooldown_ms;
    
    return 0;
}

int onvif_set_motion_config(const OnvifMotionConfig *config) {
    if (!config) return -1;
    
    auto& ae = AnalyticsEngine::Instance();
    auto analytics_config = ae.GetConfig();
    
    analytics_config.motion_detection.enabled = config->enabled;
    analytics_config.motion_detection.sensitivity = config->sensitivity;
    analytics_config.motion_detection.threshold = config->threshold;
    analytics_config.motion_detection.min_area = config->min_area;
    analytics_config.motion_detection.cooldown_ms = config->cooldown_ms;
    
    if (ae.SetConfig(analytics_config)) {
        return 0;
    }
    return -1;
}

int onvif_get_motion_zone_count(void) {
    auto& ae = AnalyticsEngine::Instance();
    auto zones = ae.GetMotionZones();
    return static_cast<int>(zones.size());
}

int onvif_get_motion_zone(int idx, OnvifMotionZone *zone) {
    if (!zone) return -1;
    
    auto& ae = AnalyticsEngine::Instance();
    auto zones = ae.GetMotionZones();
    
    if (idx < 0 || idx >= static_cast<int>(zones.size())) {
        return -1;
    }
    
    const auto& z = zones[idx];
    zone->id = z.id;
    strncpy(zone->name, z.name.c_str(), sizeof(zone->name) - 1);
    zone->x = z.x;
    zone->y = z.y;
    zone->width = z.width;
    zone->height = z.height;
    zone->sensitivity = z.sensitivity;
    zone->enabled = z.enabled;
    
    return 0;
}

int onvif_add_motion_zone(const OnvifMotionZone *zone) {
    if (!zone) return -1;
    
    auto& ae = AnalyticsEngine::Instance();
    
    MotionZone mz;
    mz.id = zone->id;
    mz.name = zone->name;
    mz.x = zone->x;
    mz.y = zone->y;
    mz.width = zone->width;
    mz.height = zone->height;
    mz.sensitivity = zone->sensitivity;
    mz.enabled = zone->enabled;
    
    if (ae.AddMotionZone(mz)) {
        return mz.id;
    }
    return -1;
}

int onvif_update_motion_zone(const OnvifMotionZone *zone) {
    if (!zone) return -1;
    
    auto& ae = AnalyticsEngine::Instance();
    
    MotionZone mz;
    mz.id = zone->id;
    mz.name = zone->name;
    mz.x = zone->x;
    mz.y = zone->y;
    mz.width = zone->width;
    mz.height = zone->height;
    mz.sensitivity = zone->sensitivity;
    mz.enabled = zone->enabled;
    
    if (ae.UpdateMotionZone(mz)) {
        return 0;
    }
    return -1;
}

int onvif_remove_motion_zone(int zone_id) {
    auto& ae = AnalyticsEngine::Instance();
    if (ae.RemoveMotionZone(zone_id)) {
        return 0;
    }
    return -1;
}

int onvif_set_person_detection_enabled(bool enabled) {
    auto& ae = AnalyticsEngine::Instance();
    if (ae.EnablePersonDetection(enabled)) {
        return 0;
    }
    return -1;
}

int onvif_set_vehicle_detection_enabled(bool enabled) {
    auto& ae = AnalyticsEngine::Instance();
    if (ae.EnableVehicleDetection(enabled)) {
        return 0;
    }
    return -1;
}

int onvif_set_face_detection_enabled(bool enabled) {
    auto& ae = AnalyticsEngine::Instance();
    if (ae.EnableFaceDetection(enabled)) {
        return 0;
    }
    return -1;
}

int onvif_set_line_crossing_enabled(bool enabled) {
    auto& ae = AnalyticsEngine::Instance();
    if (ae.EnableLineCrossing(enabled)) {
        return 0;
    }
    return -1;
}

int onvif_get_line_cross_zone_count(void) {
    auto& ae = AnalyticsEngine::Instance();
    auto zones = ae.GetLineCrossZones();
    return static_cast<int>(zones.size());
}

int onvif_get_line_cross_zone(int idx, OnvifLineCrossZone *zone) {
    if (!zone) return -1;
    
    auto& ae = AnalyticsEngine::Instance();
    auto zones = ae.GetLineCrossZones();
    
    if (idx < 0 || idx >= static_cast<int>(zones.size())) {
        return -1;
    }
    
    const auto& z = zones[idx];
    zone->id = z.id;
    strncpy(zone->name, z.name.c_str(), sizeof(zone->name) - 1);
    zone->x1 = z.x1;
    zone->y1 = z.y1;
    zone->x2 = z.x2;
    zone->y2 = z.y2;
    zone->bidirectional = z.bidirectional;
    zone->enabled = z.enabled;
    
    return 0;
}

int onvif_add_line_cross_zone(const OnvifLineCrossZone *zone) {
    if (!zone) return -1;
    
    auto& ae = AnalyticsEngine::Instance();
    
    LineCrossZone lz;
    lz.id = zone->id;
    lz.name = zone->name;
    lz.x1 = zone->x1;
    lz.y1 = zone->y1;
    lz.x2 = zone->x2;
    lz.y2 = zone->y2;
    lz.bidirectional = zone->bidirectional;
    lz.enabled = zone->enabled;
    
    if (ae.AddLineCrossZone(lz)) {
        return lz.id;
    }
    return -1;
}

int onvif_remove_line_cross_zone(int zone_id) {
    auto& ae = AnalyticsEngine::Instance();
    if (ae.RemoveLineCrossZone(zone_id)) {
        return 0;
    }
    return -1;
}

int onvif_get_analytics_stats(OnvifAnalyticsStats *stats) {
    if (!stats) return -1;
    
    auto& ae = AnalyticsEngine::Instance();
    auto s = ae.GetStats();
    
    stats->frames_processed = s.frames_processed;
    stats->total_detections = s.total_detections;
    stats->motion_events = s.motion_events;
    stats->line_cross_events = s.line_cross_events;
    stats->avg_process_time_ms = s.avg_process_time_ms;
    stats->current_fps = s.current_fps;
    
    return 0;
}

int onvif_reset_analytics_stats(void) {
    auto& ae = AnalyticsEngine::Instance();
    ae.ResetStats();
    return 0;
}

int onvif_get_detections(OnvifDetectionResult *results, int max_results) {
    if (!results || max_results <= 0) return -1;
    
    std::lock_guard<std::mutex> lock(g_detections_mutex);
    
    int count = 0;
    for (const auto& det : g_latest_detections) {
        if (count >= max_results) break;
        
        results[count].id = det.id;
        results[count].type = object_category_to_onvif(det.category);
        results[count].x1 = det.bbox.x1;
        results[count].y1 = det.bbox.y1;
        results[count].x2 = det.bbox.x2;
        results[count].y2 = det.bbox.y2;
        results[count].confidence = det.confidence;
        results[count].timestamp = det.timestamp;
        
        count++;
    }
    
    return count;
}

int onvif_is_analytics_supported(OnvifAnalyticsType type) {
    // All listed types are supported
    switch (type) {
        case ONVIF_ANALYTICS_MOTION:
        case ONVIF_ANALYTICS_PERSON:
        case ONVIF_ANALYTICS_VEHICLE:
        case ONVIF_ANALYTICS_FACE:
        case ONVIF_ANALYTICS_LINE_CROSS:
        case ONVIF_ANALYTICS_INTRUSION:
            return 1;
        default:
            return 0;
    }
}

} // extern "C"
