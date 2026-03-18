/**
 * @file analytics.cpp
 * @brief AI Analytics Engine Implementation
 * 
 * Implements video analytics using Novatek AI3 engine.
 * Pulls YUV frames from VideoProc OUT_3 (no encoder needed)
 * and processes them through AI models.
 */

#include "ipcam/analytics.h"
#include "ipcam/config.h"

// New AI module headers
#include "ipcam/motion_detection.h"
#include "ipcam/object_tracking.h"
#include "ipcam/privacy_mosaic.h"
#include "ipcam/aiisp.h"
#include "ipcam/tamper_detection.h"
#include "ipcam/hdal_pipeline.h"

// Smart analytics modules
#include "ipcam/npu_inference.h"
#include "ipcam/yolo_postproc.h"
#include "ipcam/scrfd_postproc.h"
#include "ipcam/line_crossing.h"
#include "ipcam/zone_intrusion.h"
#include "ipcam/counting.h"
#include "ipcam/face_recognition.h"
#include "ipcam/lpr.h"
#include "ipcam/heat_map.h"
#include "ipcam/pose_estimation.h"
#include "ipcam/face_attribute.h"
#include "ipcam/audio_classification.h"
#include "ipcam/audio_frame_broadcaster.h"

#include <spdlog/spdlog.h>
#include <chrono>
#include <cstring>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
#include "hd_videoprocess.h"
#include "hd_common.h"
#include "vendor_videoenc.h"
}
#endif

// Vendor AI3 neural network engine
#if VENDOR_AI3_ENABLED
extern "C" {
#include "vendor_ai.h"
}
#endif

namespace ipcam {
namespace ai {

namespace {
    std::mutex g_mutex;
    
    // Frame interval calculation
    inline int64_t GetCurrentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    
    // Calculate IoU (Intersection over Union) between two bounding boxes
    inline float CalculateBboxOverlap(const BoundingBox& a, const BoundingBox& b) {
        float x1 = std::max(a.x1, b.x1);
        float y1 = std::max(a.y1, b.y1);
        float x2 = std::min(a.x2, b.x2);
        float y2 = std::min(a.y2, b.y2);
        
        if (x2 <= x1 || y2 <= y1) return 0.0f;
        
        float intersection = (x2 - x1) * (y2 - y1);
        float area_a = (a.x2 - a.x1) * (a.y2 - a.y1);
        float area_b = (b.x2 - b.x1) * (b.y2 - b.y1);
        float union_area = area_a + area_b - intersection;
        
        return union_area > 0 ? intersection / union_area : 0.0f;
    }
    
    // Convert TrackedObject bbox to BoundingBox (for comparison)
    inline float CalculateBboxOverlap(const BoundingBox& a, const std::array<float, 4>& b) {
        BoundingBox bb;
        bb.x1 = b[0]; bb.y1 = b[1]; bb.x2 = b[2]; bb.y2 = b[3];
        return CalculateBboxOverlap(a, bb);
    }
}

// ============================================================================
// Singleton Instance
// ============================================================================

AnalyticsEngine& AnalyticsEngine::Instance() {
    static AnalyticsEngine instance;
    return instance;
}

AnalyticsEngine::AnalyticsEngine() {
    spdlog::debug("AnalyticsEngine constructor");
}

AnalyticsEngine::~AnalyticsEngine() {
    if (running_.load()) {
        Stop();
    }
    if (initialized_.load()) {
        Shutdown();
    }
}

// ============================================================================
// Configuration
// ============================================================================

bool AnalyticsEngine::LoadConfig() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    spdlog::info("Loading analytics configuration...");
    
    // Load from config system - new hierarchical structure
    config_.enabled = config::Get<bool>("analytics.enabled", true);
    
    // Engine settings
    config_.engine.type = config::Get<std::string>("analytics.engine.type", "vendor_ai3");
    config_.engine.model_path = config::Get<std::string>("analytics.engine.model_path", "/mnt/app/models");
    config_.engine.dla_enabled = config::Get<bool>("analytics.engine.dla_enabled", true);
    config_.engine.cpu_fallback = config::Get<bool>("analytics.engine.cpu_fallback", true);
    
    // Input settings
    config_.input.width = config::Get<int>("analytics.input.width", 640);
    config_.input.height = config::Get<int>("analytics.input.height", 360);
    config_.input.format = config::Get<std::string>("analytics.input.format", "yuv420");
    config_.input.fps = config::Get<int>("analytics.input.fps", 15);
    
    // Output settings
    config_.output.overlay_enabled = config::Get<bool>("analytics.output.overlay_enabled", true);
    config_.output.smart_bbox = config::Get<bool>("analytics.output.smart_bbox", true);
    config_.output.max_objects = config::Get<int>("analytics.output.max_objects", 32);
    
    // Motion detection settings
    config_.motion_detection.enabled = config::Get<bool>("motion_detection.enabled", false);
    config_.motion_detection.sensitivity = config::Get<int>("motion_detection.sensitivity", 50);
    config_.motion_detection.threshold = config::Get<int>("motion_detection.threshold", 30);
    config_.motion_detection.min_area = config::Get<int>("motion_detection.min_area", 500);
    config_.motion_detection.cooldown_ms = config::Get<int>("motion_detection.cooldown_ms", 2000);
    
    // Tamper detection settings (uses video_tampering.* keys matching analytics.json)
    config_.tamper_detection.enabled = config::Get<bool>("video_tampering.enabled", false);
    config_.tamper_detection.sensitivity = config::Get<int>("video_tampering.sensitivity", 50);
    config_.tamper_detection.defocus_enabled = config::Get<bool>("video_tampering.defocus_detection.enabled", true);
    config_.tamper_detection.masking_enabled = config::Get<bool>("video_tampering.masking_detection.enabled", true);
    config_.tamper_detection.scene_change_enabled = config::Get<bool>("video_tampering.scene_change_detection.enabled", true);
    config_.tamper_detection.exposure_enabled = config::Get<bool>("video_tampering.too_dark_detection.enabled", true);
    
    // Object detection settings (smart detection)
    config_.object_detection.enabled = config::Get<bool>("object_detection.enabled", false);
    config_.object_detection.model = config::Get<std::string>("object_detection.model", "yolov5s_nvt");
    config_.object_detection.confidence_threshold = config::Get<float>("object_detection.confidence_threshold", 0.5f);
    config_.object_detection.nms_threshold = config::Get<float>("object_detection.nms_threshold", 0.45f);
    
    // Object detection classes
    config_.object_detection.person.enabled = config::Get<bool>("object_detection.classes.person.enabled", true);
    config_.object_detection.person.min_confidence = config::Get<float>("object_detection.classes.person.min_confidence", 0.5f);
    config_.object_detection.person.color = config::Get<std::string>("object_detection.classes.person.color", "#FF0000");
    
    config_.object_detection.vehicle.enabled = config::Get<bool>("object_detection.classes.vehicle.enabled", true);
    config_.object_detection.vehicle.min_confidence = config::Get<float>("object_detection.classes.vehicle.min_confidence", 0.5f);
    config_.object_detection.vehicle.color = config::Get<std::string>("object_detection.classes.vehicle.color", "#00FF00");
    
    config_.object_detection.animal.enabled = config::Get<bool>("object_detection.classes.animal.enabled", false);
    config_.object_detection.animal.min_confidence = config::Get<float>("object_detection.classes.animal.min_confidence", 0.5f);
    config_.object_detection.animal.color = config::Get<std::string>("object_detection.classes.animal.color", "#0000FF");
    
    // Tracking settings
    config_.object_detection.tracking.enabled = config::Get<bool>("object_detection.tracking.enabled", true);
    config_.object_detection.tracking.max_age = config::Get<int>("object_detection.tracking.max_age", 30);
    config_.object_detection.tracking.min_hits = config::Get<int>("object_detection.tracking.min_hits", 3);
    config_.object_detection.tracking.iou_threshold = config::Get<float>("object_detection.tracking.iou_threshold", 0.3f);
    
    // Line crossing settings
    config_.line_crossing.enabled = config::Get<bool>("line_crossing.enabled", false);
    config_.line_crossing.counting.enabled = config::Get<bool>("line_crossing.counting.enabled", false);
    config_.line_crossing.counting.reset_interval = config::Get<int>("line_crossing.counting.reset_interval", 86400);
    
    // Zone intrusion settings
    config_.zone_intrusion.enabled = config::Get<bool>("zone_intrusion.enabled", false);
    
    // Face detection settings
    config_.face_detection.enabled = config::Get<bool>("face_detection.enabled", false);
    config_.face_detection.model = config::Get<std::string>("face_detection.model", "scrfd_nosig");
    config_.face_detection.min_face_size = config::Get<int>("face_detection.min_face_size", 30);
    config_.face_detection.confidence_threshold = config::Get<float>("face_detection.confidence_threshold", 0.6f);
    config_.face_detection.landmarks = config::Get<bool>("face_detection.landmarks", false);
    config_.face_detection.recognition.enabled = config::Get<bool>("face_detection.recognition.enabled", false);
    config_.face_detection.recognition.database_path = config::Get<std::string>("face_detection.recognition.database_path", "/mnt/app/face_db");
    config_.face_detection.recognition.similarity_threshold = config::Get<float>("face_detection.recognition.similarity_threshold", 0.7f);
    
    // License plate settings
    config_.license_plate.enabled = config::Get<bool>("license_plate.enabled", false);
    config_.license_plate.model = config::Get<std::string>("license_plate.model", "lpr_nvt");
    config_.license_plate.min_confidence = config::Get<float>("license_plate.min_confidence", 0.7f);
    
    // Heat map settings
    config_.heat_map.enabled = config::Get<bool>("heat_map.enabled", false);
    config_.heat_map.grid_w = config::Get<int>("heat_map.grid_w", 32);
    config_.heat_map.grid_h = config::Get<int>("heat_map.grid_h", 18);
    config_.heat_map.decay_rate = config::Get<float>("heat_map.decay_rate", 0.97f);
    
    // Pose estimation settings
    config_.pose_estimation.enabled = config::Get<bool>("pose_estimation.enabled", false);
    config_.pose_estimation.conf_threshold = config::Get<float>("pose_estimation.conf_threshold", 0.35f);
    config_.pose_estimation.nms_threshold = config::Get<float>("pose_estimation.nms_threshold", 0.50f);
    config_.pose_estimation.max_persons = config::Get<int>("pose_estimation.max_persons", 16);
    
    // Face attribute settings
    config_.face_attribute.enabled = config::Get<bool>("face_attribute.enabled", false);
    config_.face_attribute.margin = config::Get<float>("face_attribute.margin", 0.20f);
    
    // Audio classification settings
    config_.audio_classification.enabled = config::Get<bool>("audio_classification.enabled", false);
    config_.audio_classification.detection_threshold = config::Get<float>("audio_classification.detection_threshold", 0.3f);
    
    // Events settings
    config_.events.motion_start.enabled = config::Get<bool>("events.motion_start.enabled", true);
    config_.events.motion_start.action = config::Get<std::string>("events.motion_start.action", "notify");
    config_.events.motion_start.snapshot = config::Get<bool>("events.motion_start.snapshot", true);
    config_.events.motion_start.record_duration = config::Get<int>("events.motion_start.record_duration", 30);
    
    config_.events.motion_end.enabled = config::Get<bool>("events.motion_end.enabled", true);
    config_.events.motion_end.action = config::Get<std::string>("events.motion_end.action", "notify");
    
    config_.events.person_detected.enabled = config::Get<bool>("events.person_detected.enabled", true);
    config_.events.person_detected.action = config::Get<std::string>("events.person_detected.action", "notify");
    config_.events.person_detected.snapshot = config::Get<bool>("events.person_detected.snapshot", true);
    
    config_.events.vehicle_detected.enabled = config::Get<bool>("events.vehicle_detected.enabled", true);
    config_.events.vehicle_detected.action = config::Get<std::string>("events.vehicle_detected.action", "notify");
    config_.events.vehicle_detected.snapshot = config::Get<bool>("events.vehicle_detected.snapshot", true);
    
    config_.events.line_crossed.enabled = config::Get<bool>("events.line_crossed.enabled", false);
    config_.events.line_crossed.action = config::Get<std::string>("events.line_crossed.action", "notify");
    
    config_.events.zone_intrusion.enabled = config::Get<bool>("events.zone_intrusion.enabled", false);
    config_.events.zone_intrusion.action = config::Get<std::string>("events.zone_intrusion.action", "alarm");
    
    // Performance settings
    config_.performance.skip_frames = config::Get<int>("performance.skip_frames", 0);
    config_.performance.batch_size = config::Get<int>("performance.batch_size", 1);
    config_.performance.async_inference = config::Get<bool>("performance.async_inference", true);
    config_.performance.gpu_memory_mb = config::Get<int>("performance.gpu_memory_mb", 256);
    config_.performance.thread_count = config::Get<int>("performance.thread_count", 2);
    
    // Debug settings
    config_.debug.log_detections = config::Get<bool>("debug.log_detections", false);
    config_.debug.save_frames = config::Get<bool>("debug.save_frames", false);
    config_.debug.frame_save_path = config::Get<std::string>("debug.frame_save_path", "/tmp/ai_debug");
    config_.debug.show_inference_time = config::Get<bool>("debug.show_inference_time", false);
    
    spdlog::info("Analytics config loaded: enabled={}, object_det={}, motion_det={}, face_det={}, fps={}",
                 config_.enabled, config_.object_detection.enabled, config_.motion_detection.enabled,
                 config_.face_detection.enabled, config_.input.fps);
    
    return true;
}

AnalyticsConfig AnalyticsEngine::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool AnalyticsEngine::SetConfig(const AnalyticsConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    config_ = config;
    
    // Save to config system - new hierarchical structure
    config::Set<bool>("analytics.enabled", config_.enabled);
    
    // Engine settings
    config::Set<std::string>("analytics.engine.type", config_.engine.type);
    config::Set<std::string>("analytics.engine.model_path", config_.engine.model_path);
    config::Set<bool>("analytics.engine.dla_enabled", config_.engine.dla_enabled);
    config::Set<bool>("analytics.engine.cpu_fallback", config_.engine.cpu_fallback);
    
    // Input settings
    config::Set<int>("analytics.input.width", config_.input.width);
    config::Set<int>("analytics.input.height", config_.input.height);
    config::Set<std::string>("analytics.input.format", config_.input.format);
    config::Set<int>("analytics.input.fps", config_.input.fps);
    
    // Output settings
    config::Set<bool>("analytics.output.overlay_enabled", config_.output.overlay_enabled);
    config::Set<bool>("analytics.output.smart_bbox", config_.output.smart_bbox);
    config::Set<int>("analytics.output.max_objects", config_.output.max_objects);
    
    // Motion detection settings
    config::Set<bool>("motion_detection.enabled", config_.motion_detection.enabled);
    config::Set<int>("motion_detection.sensitivity", config_.motion_detection.sensitivity);
    config::Set<int>("motion_detection.threshold", config_.motion_detection.threshold);
    config::Set<int>("motion_detection.min_area", config_.motion_detection.min_area);
    config::Set<int>("motion_detection.cooldown_ms", config_.motion_detection.cooldown_ms);
    
    // Tamper detection settings (uses video_tampering.* keys matching analytics.json)
    config::Set<bool>("video_tampering.enabled", config_.tamper_detection.enabled);
    config::Set<int>("video_tampering.sensitivity", config_.tamper_detection.sensitivity);
    config::Set<bool>("video_tampering.defocus_detection.enabled", config_.tamper_detection.defocus_enabled);
    config::Set<bool>("video_tampering.masking_detection.enabled", config_.tamper_detection.masking_enabled);
    config::Set<bool>("video_tampering.scene_change_detection.enabled", config_.tamper_detection.scene_change_enabled);
    config::Set<bool>("video_tampering.too_dark_detection.enabled", config_.tamper_detection.exposure_enabled);
    
    // Object detection settings
    config::Set<bool>("object_detection.enabled", config_.object_detection.enabled);
    config::Set<std::string>("object_detection.model", config_.object_detection.model);
    config::Set<float>("object_detection.confidence_threshold", config_.object_detection.confidence_threshold);
    config::Set<float>("object_detection.nms_threshold", config_.object_detection.nms_threshold);
    
    // Object detection classes
    config::Set<bool>("object_detection.classes.person.enabled", config_.object_detection.person.enabled);
    config::Set<float>("object_detection.classes.person.min_confidence", config_.object_detection.person.min_confidence);
    config::Set<bool>("object_detection.classes.vehicle.enabled", config_.object_detection.vehicle.enabled);
    config::Set<float>("object_detection.classes.vehicle.min_confidence", config_.object_detection.vehicle.min_confidence);
    config::Set<bool>("object_detection.classes.animal.enabled", config_.object_detection.animal.enabled);
    
    // Tracking settings
    config::Set<bool>("object_detection.tracking.enabled", config_.object_detection.tracking.enabled);
    config::Set<int>("object_detection.tracking.max_age", config_.object_detection.tracking.max_age);
    config::Set<int>("object_detection.tracking.min_hits", config_.object_detection.tracking.min_hits);
    config::Set<float>("object_detection.tracking.iou_threshold", config_.object_detection.tracking.iou_threshold);
    
    // Line crossing settings
    config::Set<bool>("line_crossing.enabled", config_.line_crossing.enabled);
    config::Set<bool>("line_crossing.counting.enabled", config_.line_crossing.counting.enabled);
    
    // Zone intrusion settings
    config::Set<bool>("zone_intrusion.enabled", config_.zone_intrusion.enabled);
    
    // Face detection settings
    config::Set<bool>("face_detection.enabled", config_.face_detection.enabled);
    config::Set<std::string>("face_detection.model", config_.face_detection.model);
    config::Set<int>("face_detection.min_face_size", config_.face_detection.min_face_size);
    config::Set<float>("face_detection.confidence_threshold", config_.face_detection.confidence_threshold);
    config::Set<bool>("face_detection.recognition.enabled", config_.face_detection.recognition.enabled);
    
    // License plate settings
    config::Set<bool>("license_plate.enabled", config_.license_plate.enabled);
    
    // Heat map settings
    config::Set<bool>("heat_map.enabled", config_.heat_map.enabled);
    config::Set<int>("heat_map.grid_w", config_.heat_map.grid_w);
    config::Set<int>("heat_map.grid_h", config_.heat_map.grid_h);
    config::Set<float>("heat_map.decay_rate", config_.heat_map.decay_rate);
    
    // Pose estimation settings
    config::Set<bool>("pose_estimation.enabled", config_.pose_estimation.enabled);
    config::Set<float>("pose_estimation.conf_threshold", config_.pose_estimation.conf_threshold);
    config::Set<float>("pose_estimation.nms_threshold", config_.pose_estimation.nms_threshold);
    config::Set<int>("pose_estimation.max_persons", config_.pose_estimation.max_persons);
    
    // Face attribute settings
    config::Set<bool>("face_attribute.enabled", config_.face_attribute.enabled);
    config::Set<float>("face_attribute.margin", config_.face_attribute.margin);
    
    // Audio classification settings
    config::Set<bool>("audio_classification.enabled", config_.audio_classification.enabled);
    config::Set<float>("audio_classification.detection_threshold", config_.audio_classification.detection_threshold);
    
    // Events settings
    config::Set<bool>("events.motion_start.enabled", config_.events.motion_start.enabled);
    config::Set<bool>("events.motion_start.snapshot", config_.events.motion_start.snapshot);
    config::Set<int>("events.motion_start.record_duration", config_.events.motion_start.record_duration);
    config::Set<bool>("events.person_detected.enabled", config_.events.person_detected.enabled);
    config::Set<bool>("events.vehicle_detected.enabled", config_.events.vehicle_detected.enabled);
    
    // Performance settings
    config::Set<int>("performance.skip_frames", config_.performance.skip_frames);
    config::Set<int>("performance.batch_size", config_.performance.batch_size);
    config::Set<bool>("performance.async_inference", config_.performance.async_inference);
    config::Set<int>("performance.thread_count", config_.performance.thread_count);
    
    // Debug settings
    config::Set<bool>("debug.log_detections", config_.debug.log_detections);
    config::Set<bool>("debug.save_frames", config_.debug.save_frames);
    
    config::Save();
    
    spdlog::info("Analytics config updated");
    return true;
}

bool AnalyticsEngine::EnablePersonDetection(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.object_detection.person.enabled = enable;
    config::Set<bool>("object_detection.classes.person.enabled", enable);
    config::Save();
    spdlog::info("Person detection {}", enable ? "enabled" : "disabled");
    return true;
}

bool AnalyticsEngine::EnableVehicleDetection(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.object_detection.vehicle.enabled = enable;
    config::Set<bool>("object_detection.classes.vehicle.enabled", enable);
    config::Save();
    spdlog::info("Vehicle detection {}", enable ? "enabled" : "disabled");
    return true;
}

bool AnalyticsEngine::EnableFaceDetection(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.face_detection.enabled = enable;
    config::Set<bool>("face_detection.enabled", enable);
    config::Save();
    spdlog::info("Face detection {}", enable ? "enabled" : "disabled");
    return true;
}

bool AnalyticsEngine::EnableMotionDetection(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.motion_detection.enabled = enable;
    config::Set<bool>("motion_detection.enabled", enable);
    config::Save();
    spdlog::info("Motion detection {}", enable ? "enabled" : "disabled");
    return true;
}

bool AnalyticsEngine::EnableLineCrossing(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.line_crossing.enabled = enable;
    config::Set<bool>("line_crossing.enabled", enable);
    config::Save();
    spdlog::info("Line crossing detection {}", enable ? "enabled" : "disabled");
    return true;
}

// ============================================================================
// Zone Management
// ============================================================================

bool AnalyticsEngine::AddMotionZone(const MotionZone& zone) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.motion_detection.zones.push_back(zone);
    motion_zone_active_.push_back(false);
    
    // Propagate to MotionDetectionEngine
    try {
        auto& md = MotionDetectionEngine::Instance();
        if (md.IsRunning()) {
            MdZone md_zone;
            md_zone.id = zone.id;
            md_zone.name = zone.name;
            md_zone.x_start = static_cast<uint32_t>(zone.x * config_.input.width);
            md_zone.y_start = static_cast<uint32_t>(zone.y * config_.input.height);
            md_zone.x_end = static_cast<uint32_t>((zone.x + zone.width) * config_.input.width);
            md_zone.y_end = static_cast<uint32_t>((zone.y + zone.height) * config_.input.height);
            md_zone.sensitivity = zone.sensitivity;
            md_zone.enabled = zone.enabled;
            md_zone.loitering_enabled = zone.loitering_enabled;
            md_zone.loitering_threshold_ms = zone.loitering_threshold_ms;
            md.AddZone(md_zone);
        }
    } catch (const std::exception& e) {
        spdlog::warn("Failed to propagate zone to MD engine: {}", e.what());
    }
    
    spdlog::info("Added motion zone '{}' (id={})", zone.name, zone.id);
    return true;
}

bool AnalyticsEngine::RemoveMotionZone(uint32_t zone_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(config_.motion_detection.zones.begin(), config_.motion_detection.zones.end(),
                           [zone_id](const MotionZone& z) { return z.id == zone_id; });
    if (it != config_.motion_detection.zones.end()) {
        size_t idx = it - config_.motion_detection.zones.begin();
        config_.motion_detection.zones.erase(it);
        if (idx < motion_zone_active_.size()) {
            motion_zone_active_.erase(motion_zone_active_.begin() + idx);
        }
        
        // Propagate to MotionDetectionEngine
        try {
            auto& md = MotionDetectionEngine::Instance();
            if (md.IsRunning()) {
                md.RemoveZone(zone_id);
            }
        } catch (const std::exception& e) {
            spdlog::warn("Failed to propagate zone removal to MD engine: {}", e.what());
        }
        
        spdlog::info("Removed motion zone id={}", zone_id);
        return true;
    }
    return false;
}

bool AnalyticsEngine::UpdateMotionZone(const MotionZone& zone) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& z : config_.motion_detection.zones) {
        if (z.id == zone.id) {
            z = zone;
            
            // Propagate to MotionDetectionEngine (remove and re-add)
            try {
                auto& md = MotionDetectionEngine::Instance();
                if (md.IsRunning()) {
                    md.RemoveZone(zone.id);
                    MdZone md_zone;
                    md_zone.id = zone.id;
                    md_zone.name = zone.name;
                    md_zone.x_start = static_cast<uint32_t>(zone.x * config_.input.width);
                    md_zone.y_start = static_cast<uint32_t>(zone.y * config_.input.height);
                    md_zone.x_end = static_cast<uint32_t>((zone.x + zone.width) * config_.input.width);
                    md_zone.y_end = static_cast<uint32_t>((zone.y + zone.height) * config_.input.height);
                    md_zone.sensitivity = zone.sensitivity;
                    md_zone.enabled = zone.enabled;
                    md_zone.loitering_enabled = zone.loitering_enabled;
                    md_zone.loitering_threshold_ms = zone.loitering_threshold_ms;
                    md.AddZone(md_zone);
                }
            } catch (const std::exception& e) {
                spdlog::warn("Failed to propagate zone update to MD engine: {}", e.what());
            }
            
            spdlog::info("Updated motion zone '{}' (id={})", zone.name, zone.id);
            return true;
        }
    }
    return false;
}

std::vector<MotionZone> AnalyticsEngine::GetMotionZones() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.motion_detection.zones;
}

bool AnalyticsEngine::AddLineCrossZone(const LineCrossZone& zone) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Convert LineCrossZone to LineCrossLine for new config structure
    LineCrossLine line;
    line.id = zone.id;
    line.name = zone.name;
    line.x1 = zone.x1;
    line.y1 = zone.y1;
    line.x2 = zone.x2;
    line.y2 = zone.y2;
    line.direction = zone.bidirectional ? "both" : "in";
    line.enabled = zone.enabled;
    config_.line_crossing.lines.push_back(line);
    
    // Propagate to MotionDetectionEngine
    try {
        auto& md = MotionDetectionEngine::Instance();
        if (md.IsRunning()) {
            MdCrossLine md_line;
            md_line.id = zone.id;
            md_line.name = zone.name;
            md_line.x1 = static_cast<int>(zone.x1 * config_.input.width);
            md_line.y1 = static_cast<int>(zone.y1 * config_.input.height);
            md_line.x2 = static_cast<int>(zone.x2 * config_.input.width);
            md_line.y2 = static_cast<int>(zone.y2 * config_.input.height);
            md_line.direction = zone.bidirectional ? MdCrossDirection::kUnknown : MdCrossDirection::kLeftToRight;
            md_line.enabled = zone.enabled;
            md_line.counting_enabled = config_.line_crossing.counting.enabled;
            md_line.count_reset_interval_sec = config_.line_crossing.counting.reset_interval;
            md.AddCrossLine(md_line);
        }
    } catch (const std::exception& e) {
        spdlog::warn("Failed to propagate line to MD engine: {}", e.what());
    }
    
    spdlog::info("Added line crossing zone '{}' (id={})", zone.name, zone.id);
    return true;
}

bool AnalyticsEngine::RemoveLineCrossZone(uint32_t zone_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(config_.line_crossing.lines.begin(), config_.line_crossing.lines.end(),
                           [zone_id](const LineCrossLine& l) { return l.id == zone_id; });
    if (it != config_.line_crossing.lines.end()) {
        config_.line_crossing.lines.erase(it);
        
        // Propagate to MotionDetectionEngine
        try {
            auto& md = MotionDetectionEngine::Instance();
            if (md.IsRunning()) {
                md.RemoveCrossLine(zone_id);
            }
        } catch (const std::exception& e) {
            spdlog::warn("Failed to propagate line removal to MD engine: {}", e.what());
        }
        
        spdlog::info("Removed line crossing zone id={}", zone_id);
        return true;
    }
    return false;
}

bool AnalyticsEngine::UpdateLineCrossZone(const LineCrossZone& zone) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& line : config_.line_crossing.lines) {
        if (line.id == zone.id) {
            line.name = zone.name;
            line.x1 = zone.x1;
            line.y1 = zone.y1;
            line.x2 = zone.x2;
            line.y2 = zone.y2;
            line.direction = zone.bidirectional ? "both" : "in";
            line.enabled = zone.enabled;
            
            // Propagate to MotionDetectionEngine (remove and re-add)
            try {
                auto& md = MotionDetectionEngine::Instance();
                if (md.IsRunning()) {
                    md.RemoveCrossLine(zone.id);
                    MdCrossLine md_line;
                    md_line.id = zone.id;
                    md_line.name = zone.name;
                    md_line.x1 = static_cast<int>(zone.x1 * config_.input.width);
                    md_line.y1 = static_cast<int>(zone.y1 * config_.input.height);
                    md_line.x2 = static_cast<int>(zone.x2 * config_.input.width);
                    md_line.y2 = static_cast<int>(zone.y2 * config_.input.height);
                    md_line.direction = zone.bidirectional ? MdCrossDirection::kUnknown : MdCrossDirection::kLeftToRight;
                    md_line.enabled = zone.enabled;
                    md_line.counting_enabled = config_.line_crossing.counting.enabled;
                    md_line.count_reset_interval_sec = config_.line_crossing.counting.reset_interval;
                    md.AddCrossLine(md_line);
                }
            } catch (const std::exception& e) {
                spdlog::warn("Failed to propagate line update to MD engine: {}", e.what());
            }
            
            spdlog::info("Updated line crossing zone '{}' (id={})", zone.name, zone.id);
            return true;
        }
    }
    return false;
}

std::vector<LineCrossZone> AnalyticsEngine::GetLineCrossZones() const {
    std::lock_guard<std::mutex> lock(mutex_);
    // Convert LineCrossLine to LineCrossZone for backward compatibility
    std::vector<LineCrossZone> zones;
    for (const auto& line : config_.line_crossing.lines) {
        LineCrossZone zone;
        zone.id = line.id;
        zone.name = line.name;
        zone.x1 = line.x1;
        zone.y1 = line.y1;
        zone.x2 = line.x2;
        zone.y2 = line.y2;
        zone.bidirectional = (line.direction == "both");
        zone.enabled = line.enabled;
        zones.push_back(zone);
    }
    return zones;
}

// ============================================================================
// Callbacks
// ============================================================================

void AnalyticsEngine::SetDetectionCallback(DetectionCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    detection_callback_ = callback;
}

void AnalyticsEngine::SetMotionCallback(MotionCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    motion_callback_ = callback;
}

void AnalyticsEngine::SetLineCrossCallback(LineCrossCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    linecross_callback_ = callback;
}

void AnalyticsEngine::SetLoiteringCallback(LoiteringCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    loitering_callback_ = callback;
}

void AnalyticsEngine::SetTamperCallback(TamperCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    tamper_callback_ = callback;
}

void AnalyticsEngine::SetFaceRecognitionCallback(FaceRecognitionCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    face_recognition_callback_ = callback;
}

void AnalyticsEngine::SetLprCallback(LprCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    lpr_callback_ = callback;
}

void AnalyticsEngine::SetAudioEventCallback(AudioEventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    audio_event_callback_ = callback;
}

std::vector<float> AnalyticsEngine::GetHeatMapGrid() const {
    if (heat_map_engine_ && heat_map_engine_->IsInitialized())
        return heat_map_engine_->GetGrid();
    return {};
}

int AnalyticsEngine::GetHeatMapGridW() const {
    return heat_map_engine_ ? heat_map_engine_->GridW() : 0;
}

int AnalyticsEngine::GetHeatMapGridH() const {
    return heat_map_engine_ ? heat_map_engine_->GridH() : 0;
}

void AnalyticsEngine::ResetHeatMap() {
    if (heat_map_engine_) heat_map_engine_->Reset();
}

// ============================================================================
// Statistics
// ============================================================================

AnalyticsEngine::Stats AnalyticsEngine::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void AnalyticsEngine::ResetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Stats{};
}

// ============================================================================
// Lifecycle
// ============================================================================

bool AnalyticsEngine::Init() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("AnalyticsEngine already initialized");
        return true;
    }
    
    if (!config_.enabled) {
        spdlog::info("Analytics disabled in config, skipping initialization");
        return true;
    }
    
    spdlog::info("Initializing Analytics Engine...");
    
    // Initialize AI models
    if (!InitAiModels()) {
        spdlog::error("Failed to initialize AI models");
        return false;
    }
    
    // Initialize HDAL path for frame pulling
    if (!InitHdalPath()) {
        spdlog::error("Failed to initialize HDAL analytics path");
        return false;
    }
    
    // Allocate previous frame buffer for motion detection
    int frame_size = config_.input.width * config_.input.height;
    prev_frame_.resize(frame_size, 0);
    
    // Initialize motion zone states
    motion_zone_active_.resize(config_.motion_detection.zones.size(), false);
    
    initialized_.store(true);
    spdlog::info("Analytics Engine initialized");
    return true;
}

bool AnalyticsEngine::InitAiModels() {
#if VENDOR_AI3_ENABLED
    if (!config_.object_detection.enabled) {
        spdlog::info("Smart Detection (AI3) disabled by config - skipping neural network initialization");
        ai3_initialized_ = false;
        return true;
    }
    
    // Pre-flight check: verify AI model directory exists before calling vendor library.
    // vendor_ai3_dev_init() crashes with SIGSEGV if models are missing (cannot be caught by try/catch).
    {
        struct stat st;
        if (stat(config_.engine.model_path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
            spdlog::warn("AI model directory '{}' does not exist - disabling AI3 object detection",
                         config_.engine.model_path);
            spdlog::warn("To enable: mkdir -p {} and deploy AI model files", config_.engine.model_path);
            ai3_initialized_ = false;
            config_.object_detection.enabled = false;
            return true;
        }
        // Check if directory has any .bin model files
        bool has_models = false;
        DIR* dir = opendir(config_.engine.model_path.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name.size() > 4 && name.substr(name.size() - 4) == ".bin") {
                    has_models = true;
                    break;
                }
            }
            closedir(dir);
        }
        if (!has_models) {
            spdlog::warn("No AI model files (.bin) found in '{}' - disabling AI3 object detection",
                         config_.engine.model_path);
            ai3_initialized_ = false;
            config_.object_detection.enabled = false;
            return true;
        }
    }

    spdlog::info("Initializing AI3 neural network engine...");
    
    try {
        // Initialize AI3 device
        VENDOR_AI3_DEV_CFG dev_cfg;
        memset(&dev_cfg, 0, sizeof(dev_cfg));
        
        HD_RESULT ret = vendor_ai3_dev_init(&dev_cfg);
        if (ret != HD_OK) {
            spdlog::warn("vendor_ai3_dev_init failed: {} - AI inference disabled, using motion detection only", 
                         static_cast<int>(ret));
            ai3_initialized_ = false;
        } else {
            // Get AI3 version info
            VENDOR_AI3_VER ver;
            memset(&ver, 0, sizeof(ver));
            ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_VER, &ver);
            if (ret == HD_OK) {
                spdlog::info("AI3 version: lib={}, kflow={}, kdrv={}", 
                             ver.vendor_ai_impl_version, 
                             ver.kflow_ai_impl_version,
                             ver.kdrv_ai_impl_version);
            }
            ai3_initialized_ = true;
            spdlog::info("AI3 engine initialized successfully");
        }
    } catch (const std::exception& e) {
        spdlog::warn("AI3 initialization exception: {} - AI inference disabled", e.what());
        ai3_initialized_ = false;
    } catch (...) {
        spdlog::warn("AI3 initialization failed with unknown exception - AI inference disabled");
        ai3_initialized_ = false;
    }
#endif

    // Initialize Tamper Detection Engine (uses IVE hardware acceleration directly)
    // Uses 320x180 resolution for defocus, masking, scene change detection
    if (!config_.tamper_detection.enabled) {
        spdlog::info("Tamper Detection disabled by config - skipping engine initialization");
    } else try {
        auto& tamper = TamperDetectionEngine::Instance();
        TamperConfig tamper_cfg;
        tamper_cfg.width = 320;
        tamper_cfg.height = 180;
        tamper_cfg.enabled = config_.tamper_detection.enabled;
        // Convert int sensitivity (0-100) to TamperSensitivity enum
        if (config_.tamper_detection.sensitivity < 33) {
            tamper_cfg.sensitivity = TamperSensitivity::kLow;
        } else if (config_.tamper_detection.sensitivity < 66) {
            tamper_cfg.sensitivity = TamperSensitivity::kMedium;
        } else {
            tamper_cfg.sensitivity = TamperSensitivity::kHigh;
        }
        tamper_cfg.defocus.enabled = config_.tamper_detection.defocus_enabled;
        tamper_cfg.defocus.threshold = 100.0f;
        tamper_cfg.masking.enabled = config_.tamper_detection.masking_enabled;
        tamper_cfg.masking.dark_threshold = 5.0f;
        tamper_cfg.scene_change.enabled = config_.tamper_detection.scene_change_enabled;
        tamper_cfg.scene_change.histogram_threshold = 30.0f;
        tamper_cfg.exposure.enabled = config_.tamper_detection.exposure_enabled;
        tamper_cfg.exposure.too_dark_threshold = 50.0f;
        tamper_cfg.exposure.too_bright_threshold = 220.0f;
        tamper_cfg.auto_reset.enabled = true;
        
        if (tamper.Init(tamper_cfg)) {
            spdlog::info("Tamper Detection Engine initialized (320x180, IVE-accelerated)");
        } else {
            spdlog::warn("Tamper Detection Engine initialization failed");
        }
    } catch (const std::exception& e) {
        spdlog::warn("Tamper detection initialization exception: {}", e.what());
    }

    // Initialize Motion Detection Engine (requires libmd.so)
    if (!config_.motion_detection.enabled) {
        spdlog::info("Motion Detection disabled by config - skipping engine initialization");
    } else {
    try {
        auto& md = MotionDetectionEngine::Instance();
        MdConfig md_cfg;
        md_cfg.width = config_.input.width;
        md_cfg.height = config_.input.height;
        md_cfg.mode = MotionMode::kSubAlarm;  // Use sub-alarm mode for zone-based detection
        md_cfg.sensitivity = config_.motion_detection.sensitivity;
        md_cfg.global_threshold = static_cast<uint8_t>(config_.motion_detection.threshold);
        md_cfg.enabled = config_.motion_detection.enabled;
        md_cfg.cooldown_ms = config_.motion_detection.cooldown_ms;
        
        if (md.Init(md_cfg)) {
            spdlog::info("Motion Detection Engine initialized successfully");
            
            // Add motion zones
            for (const auto& zone : config_.motion_detection.zones) {
                MdZone md_zone;
                md_zone.id = zone.id;
                md_zone.name = zone.name;
                // Convert normalized coords (0-1) to pixel coords
                md_zone.x_start = static_cast<uint32_t>(zone.x * config_.input.width);
                md_zone.y_start = static_cast<uint32_t>(zone.y * config_.input.height);
                md_zone.x_end = static_cast<uint32_t>((zone.x + zone.width) * config_.input.width);
                md_zone.y_end = static_cast<uint32_t>((zone.y + zone.height) * config_.input.height);
                md_zone.sensitivity = zone.sensitivity;
                md_zone.enabled = zone.enabled;
                md_zone.loitering_enabled = zone.loitering_enabled;
                md_zone.loitering_threshold_ms = zone.loitering_threshold_ms;
                md.AddZone(md_zone);
            }
            
            // Add line crossing zones
            for (const auto& line : config_.line_crossing.lines) {
                MdCrossLine md_line;
                md_line.id = line.id;
                md_line.name = line.name;
                md_line.x1 = static_cast<int>(line.x1 * config_.input.width);
                md_line.y1 = static_cast<int>(line.y1 * config_.input.height);
                md_line.x2 = static_cast<int>(line.x2 * config_.input.width);
                md_line.y2 = static_cast<int>(line.y2 * config_.input.height);
                md_line.direction = (line.direction == "both") ? MdCrossDirection::kUnknown : MdCrossDirection::kLeftToRight;
                md_line.enabled = line.enabled;
                md_line.counting_enabled = config_.line_crossing.counting.enabled;
                md_line.count_reset_interval_sec = config_.line_crossing.counting.reset_interval;
                md.AddCrossLine(md_line);
            }
            
            // Start the engine
            if (!md.Start()) {
                spdlog::warn("Motion Detection Engine failed to start");
            } else {
                // Set up loitering callback to forward to analytics callback
                md.SetLoiteringCallback([this](const MdLoiteringEvent& event) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (loitering_callback_) {
                        loitering_callback_(event.zone_id, event.object_id, event.dwell_time_ms);
                    }
                    stats_.loitering_events++;
                });
            }
        } else {
            spdlog::warn("Motion Detection Engine initialization failed");
        }
    } catch (const std::exception& e) {
        spdlog::warn("Motion Detection initialization exception: {}", e.what());
    }
    } // end if (motion_detection.enabled)

    // Initialize Object Tracker
#if AI_TRKE_ENABLED
    try {
        auto& tracker = ObjectTracker::Instance();
        TrackingConfig trk_cfg;
        trk_cfg.max_age = config_.object_detection.tracking.max_age;
        trk_cfg.min_hits = config_.object_detection.tracking.min_hits;
        trk_cfg.iou_threshold = config_.object_detection.tracking.iou_threshold;
        trk_cfg.use_kalman = false;
        // use_reid is enabled at runtime once OSNet model is confirmed loaded
        if (tracker.Init(trk_cfg)) {
            spdlog::info("Object Tracker initialized successfully");
        } else {
            spdlog::warn("Object Tracker initialization failed");
        }
    } catch (const std::exception& e) {
        spdlog::warn("Object Tracker initialization exception: {}", e.what());
    }
#endif

    // Initialize Privacy Mosaic Engine
#if AI_GFX_ENABLED
    try {
        auto& mosaic = PrivacyMosaicEngine::Instance();
        MosaicConfig mosaic_cfg;
        mosaic_cfg.enabled = true;
        mosaic_cfg.default_type = MosaicType::kBlock;
        mosaic_cfg.block_size = 16;
        mosaic_cfg.blur_radius = 21;
        mosaic_cfg.auto_detect_faces = config_.face_detection.enabled;
        mosaic.ApplyConfig(mosaic_cfg);
        
        if (mosaic.Init()) {
            spdlog::info("Privacy Mosaic Engine initialized successfully");
        } else {
            spdlog::warn("Privacy Mosaic Engine initialization failed");
        }
    } catch (const std::exception& e) {
        spdlog::warn("Privacy Mosaic initialization exception: {}", e.what());
    }
#endif

    // Initialize AIISP Engine
#if AI_ISP_ENABLED
    try {
        auto& aiisp = AiispEngine::Instance();
        AiispConfig aiisp_cfg;
        aiisp_cfg.enabled = true;
        aiisp_cfg.denoise_strength = 50;
        aiisp_cfg.wdr_enabled = true;
        aiisp_cfg.defog_enabled = false;
        aiisp_cfg.model_path = config_.engine.model_path;
        
        if (aiisp.Init(aiisp_cfg)) {
            spdlog::info("AIISP Engine initialized successfully");
        } else {
            spdlog::warn("AIISP Engine initialization failed");
        }
    } catch (const std::exception& e) {
        spdlog::warn("AIISP initialization exception: {}", e.what());
    }
#endif

    // Initialize Tamper Detection Engine (IVE-accelerated hardware histogram/Sobel)
    // Uses dedicated 320x180 VQA path from HdalPipeline for optimal IVE performance
    if (!config_.tamper_detection.enabled) {
        spdlog::info("Tamper Detection disabled by config - skipping IVE engine initialization");
    } else try {
        auto& tamper = TamperDetectionEngine::Instance();
        TamperConfig tamper_cfg;
        
        // Load tamper detection configuration (uses hierarchical config matching TamperConfig struct)
        tamper_cfg.enabled = config::Get<bool>("video_tampering.enabled", false);
        tamper_cfg.defocus.enabled = config::Get<bool>("video_tampering.defocus_detection.enabled", true);
        tamper_cfg.defocus.threshold = config::Get<float>("video_tampering.defocus_detection.sensitivity", 500.0f);
        tamper_cfg.masking.enabled = config::Get<bool>("video_tampering.masking_detection.enabled", true);
        tamper_cfg.masking.dark_threshold = config::Get<float>("video_tampering.masking_detection.sensitivity", 30.0f);
        tamper_cfg.scene_change.enabled = config::Get<bool>("video_tampering.scene_change_detection.enabled", true);
        tamper_cfg.scene_change.histogram_threshold = config::Get<float>("video_tampering.scene_change_detection.sensitivity", 0.02f);
        tamper_cfg.exposure.enabled = config::Get<bool>("video_tampering.too_dark_detection.enabled", true);
        tamper_cfg.exposure.too_dark_threshold = config::Get<int>("video_tampering.too_dark_detection.threshold", 25);
        tamper_cfg.exposure.too_bright_threshold = config::Get<int>("video_tampering.too_bright_detection.threshold", 230);
        tamper_cfg.dwell_frames = config::Get<int>("video_tampering.dwell_time_sec", 3) * 5;
        tamper_cfg.auto_reset.enabled = config::Get<bool>("video_tampering.auto_recovery", true);
        
        if (tamper.Init(tamper_cfg)) {
            spdlog::info("Tamper Detection Engine initialized (IVE-accelerated, 320x180)");
            
            // Set callback for tamper events to forward to event system
            tamper.SetCallback([this](TamperType type, bool active, const TamperResult& result) {
                std::lock_guard<std::mutex> lock(mutex_);
                if (tamper_callback_) {
                    tamper_callback_(type, active, result);
                }
                if (active) {
                    stats_.tamper_events++;
                    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(TamperType::kSceneChange)) {
                        stats_.scene_change_events++;
                    }
                }
            });
            
            spdlog::info("Tamper detection initialized - using IVE hardware acceleration");
        } else {
            spdlog::warn("Tamper Detection Engine initialization failed");
        }
    } catch (const std::exception& e) {
        spdlog::warn("Tamper Detection initialization exception: {}", e.what());
    }

    // Initialize NPU Inference Engine and load AI models
#if VENDOR_AI3_ENABLED
    if (ai3_initialized_) {
        try {
            auto& npu = NpuInference::Instance();
            if (npu.Init()) {
                // Allocate shared NV12 crop buffer (128x256 covers all models)
                ai_crop_size_ = kOsnetInputW * kOsnetInputH * 3 / 2;  // 49152 bytes
                {
                    void* va_ptr = nullptr;
                    UINTPTR pa_tmp = 0;
                    HD_RESULT alloc_ret = hd_common_mem_alloc(
                        const_cast<CHAR*>("ai_crop"), &pa_tmp, &va_ptr,
                        ai_crop_size_, DDR_ID0);
                    if (alloc_ret == HD_OK && va_ptr) {
                        ai_crop_pa_ = pa_tmp;
                        ai_crop_va_ = reinterpret_cast<uintptr_t>(va_ptr);
                        spdlog::info("Shared AI crop buffer allocated: {}x{} NV12",
                                     kOsnetInputW, kOsnetInputH);
                    } else {
                        spdlog::warn("Failed to allocate AI crop buffer");
                    }
                }

                // Load YOLO object detection model
                if (config_.object_detection.enabled) {
                    std::string yolo_model = config_.engine.model_path + "/yolo26n.bin";
                    struct stat st;
                    if (stat(yolo_model.c_str(), &st) == 0) {
                        yolo_net_id_ = npu.LoadNetwork("yolo26n", yolo_model);
                        if (yolo_net_id_ >= 0) {
                            spdlog::info("YOLO model loaded: network_id={}", yolo_net_id_);
                            yolo_postproc_ = std::make_unique<YoloPostProc>();
                            YoloConfig yolo_cfg;
                            yolo_cfg.conf_threshold = config_.object_detection.confidence_threshold;
                            yolo_cfg.nms_threshold = config_.object_detection.nms_threshold;
                            yolo_postproc_->Init(yolo_cfg);
                        } else {
                            spdlog::warn("Failed to load YOLO model: {}", yolo_model);
                        }
                    } else {
                        spdlog::info("YOLO model not found: {} - skipping", yolo_model);
                    }
                }

                // Load OSNet Re-ID model
                if (config_.object_detection.tracking.enabled) {
                    std::string osnet_model = config_.engine.model_path + "/osnet_reid.bin";
                    struct stat st2;
                    if (stat(osnet_model.c_str(), &st2) == 0) {
                        osnet_net_id_ = npu.LoadNetwork("osnet_reid", osnet_model);
                        if (osnet_net_id_ >= 0) {
                            spdlog::info("OSNet Re-ID loaded: network_id={}", osnet_net_id_);
#if AI_TRKE_ENABLED
                            try {
                                auto& tracker = ObjectTracker::Instance();
                                auto tcfg = tracker.GetConfig();
                                tcfg.use_reid = true;
                                tracker.SetConfig(tcfg);
                                spdlog::info("Tracker Re-ID enabled");
                            } catch (...) {}
#endif
                        } else {
                            spdlog::warn("Failed to load OSNet model: {}", osnet_model);
                        }
                    } else {
                        spdlog::info("OSNet model not found: {} - Re-ID disabled", osnet_model);
                    }
                }

                // Load SCRFD face detection model
                if (config_.face_detection.enabled) {
                    std::string scrfd_model = config_.engine.model_path + "/scrfd_nosig.bin";
                    struct stat st3;
                    if (stat(scrfd_model.c_str(), &st3) == 0) {
                        scrfd_net_id_ = npu.LoadNetwork("scrfd_nosig", scrfd_model);
                        if (scrfd_net_id_ >= 0) {
                            spdlog::info("SCRFD model loaded: network_id={}", scrfd_net_id_);
                            scrfd_postproc_ = std::make_unique<ScrfdPostProc>();
                            ScrfdConfig scrfd_cfg;
                            scrfd_cfg.conf_threshold = config_.face_detection.confidence_threshold;
                            scrfd_postproc_->Init(scrfd_cfg);
                        } else {
                            spdlog::warn("Failed to load SCRFD model");
                        }
                    } else {
                        spdlog::info("SCRFD model not found: {} - face detection disabled", scrfd_model);
                    }

                    // Load MobileFaceNet for face recognition (if face detection is on)
                    if (config_.face_detection.recognition.enabled) {
                        std::string fn_model = config_.engine.model_path + "/mobilefacenet.bin";
                        struct stat st4;
                        if (stat(fn_model.c_str(), &st4) == 0) {
                            mobilefacenet_id_ = npu.LoadNetwork("mobilefacenet", fn_model);
                            if (mobilefacenet_id_ >= 0) {
                                spdlog::info("MobileFaceNet loaded: network_id={}", mobilefacenet_id_);
                                face_recognition_engine_ = std::make_unique<FaceRecognitionEngine>();
                                FaceRecognitionConfig fr_cfg;
                                fr_cfg.enabled = true;
                                fr_cfg.database_path = config_.face_detection.recognition.database_path;
                                fr_cfg.similarity_threshold = config_.face_detection.recognition.similarity_threshold;
                                face_recognition_engine_->Init(fr_cfg);
                            }
                        }
                    }

                    // Load face attribute model
                    {
                        std::string attr_model = config_.engine.model_path + "/face_attribute.bin";
                        struct stat st5;
                        if (stat(attr_model.c_str(), &st5) == 0) {
                            face_attr_net_id_ = npu.LoadNetwork("face_attribute", attr_model);
                            if (face_attr_net_id_ >= 0) {
                                spdlog::info("Face attribute model loaded: network_id={}", face_attr_net_id_);
                                face_attribute_engine_ = std::make_unique<FaceAttributeEngine>();
                                FaceAttributeConfig fa_cfg;
                                fa_cfg.enabled = true;
                                face_attribute_engine_->Init(fa_cfg);
                            }
                        }
                    }
                }

                // Load license plate detection + OCR models
                if (config_.license_plate.enabled) {
                    std::string lp_model = config_.engine.model_path + "/lp_det_yolo11n.bin";
                    struct stat st6;
                    if (stat(lp_model.c_str(), &st6) == 0) {
                        lp_det_net_id_ = npu.LoadNetwork("lp_det_yolo11n", lp_model);
                        if (lp_det_net_id_ >= 0) {
                            spdlog::info("LP detector loaded: network_id={}", lp_det_net_id_);
                        }
                    }
                    std::string ocr_model = config_.engine.model_path + "/indian_lprnet_ocr.bin";
                    struct stat st7;
                    if (stat(ocr_model.c_str(), &st7) == 0) {
                        lp_ocr_net_id_ = npu.LoadNetwork("indian_lprnet_ocr", ocr_model);
                        if (lp_ocr_net_id_ >= 0) {
                            spdlog::info("LPRNet OCR loaded: network_id={}", lp_ocr_net_id_);
                        }
                    }
                    if (lp_det_net_id_ >= 0) {
                        lpr_engine_ = std::make_unique<LprEngine>();
                        LprConfig lpr_cfg;
                        lpr_cfg.enabled = true;
                        lpr_cfg.det_conf_threshold = config_.license_plate.min_confidence;
                        lpr_engine_->Init(lpr_cfg);
                    }
                }

                // Load pose estimation model
                {
                    std::string pose_model = config_.engine.model_path + "/yolo26n_pose.bin";
                    struct stat st8;
                    if (stat(pose_model.c_str(), &st8) == 0) {
                        pose_net_id_ = npu.LoadNetwork("yolo26n_pose", pose_model);
                        if (pose_net_id_ >= 0) {
                            spdlog::info("Pose model loaded: network_id={}", pose_net_id_);
                            pose_estimation_engine_ = std::make_unique<PoseEstimationEngine>();
                            PoseConfig pose_cfg;
                            // Pose is disabled by default; enable via config/API
                            pose_cfg.enabled = false;
                            pose_estimation_engine_->Init(pose_cfg);
                        }
                    }
                }

            } else {
                spdlog::warn("NpuInference init failed");
            }
        } catch (const std::exception& e) {
            spdlog::warn("NPU initialization exception: {}", e.what());
        }
    }
#endif

    // Initialize Heat Map Engine (no NPU dependency)
    {
        heat_map_engine_ = std::make_unique<HeatMapEngine>();
        HeatMapConfig hm_cfg;
        // Heat map is off by default, enable via API
        hm_cfg.enabled = false;
        heat_map_engine_->Init(hm_cfg);
    }

    // Initialize Line Crossing Engine
    if (config_.line_crossing.enabled) {
        try {
            line_crossing_engine_ = std::make_unique<LineCrossingEngine>();
            line_crossing_engine_->Init(config_.line_crossing);
            
            // Wire line crossing events to the existing callback
            line_crossing_engine_->SetCallback([this](const LineCrossEvent& evt) {
                std::lock_guard<std::mutex> lock(mutex_);
                stats_.line_cross_events++;
                if (linecross_callback_) {
                    linecross_callback_(evt.line_id, evt.track_id, evt.category);
                }
            });
            
            spdlog::info("LineCrossingEngine initialized with {} lines", 
                         config_.line_crossing.lines.size());
        } catch (const std::exception& e) {
            spdlog::warn("LineCrossingEngine initialization exception: {}", e.what());
        }
    }

    // Initialize Zone Intrusion Engine
    if (config_.zone_intrusion.enabled) {
        try {
            zone_intrusion_engine_ = std::make_unique<ZoneIntrusionEngine>();
            zone_intrusion_engine_->Init(config_.zone_intrusion);
            spdlog::info("ZoneIntrusionEngine initialized with {} zones", 
                         config_.zone_intrusion.zones.size());
        } catch (const std::exception& e) {
            spdlog::warn("ZoneIntrusionEngine initialization exception: {}", e.what());
        }
    }

    // Initialize Counting Engine
    if (config_.line_crossing.counting.enabled) {
        try {
            CountingConfig cnt_cfg;
            cnt_cfg.enabled = true;
            cnt_cfg.reset_interval_sec = config_.line_crossing.counting.reset_interval;
            cnt_cfg.count_people = true;
            cnt_cfg.count_vehicles = true;
            cnt_cfg.show_osd = true;
            
            counting_engine_ = std::make_unique<CountingEngine>();
            counting_engine_->Init(cnt_cfg);
            spdlog::info("CountingEngine initialized (reset interval: {}s)", 
                         cnt_cfg.reset_interval_sec);
        } catch (const std::exception& e) {
            spdlog::warn("CountingEngine initialization exception: {}", e.what());
        }
    }

    // Initialize Audio Classification Engine
    if (config_.audio_classification.enabled) {
        try {
            audio_classification_engine_ = std::make_unique<AudioClassificationEngine>();
            if (audio_classification_engine_->Init(config_.audio_classification)) {
                spdlog::info("AudioClassificationEngine initialized (threshold: {:.2f})",
                             config_.audio_classification.detection_threshold);
            } else {
                spdlog::warn("AudioClassificationEngine Init() returned false");
                audio_classification_engine_.reset();
            }
        } catch (const std::exception& e) {
            spdlog::warn("AudioClassificationEngine initialization exception: {}", e.what());
            audio_classification_engine_.reset();
        }
    }

    // Always return true - modules are optional, basic analytics still works
    return true;
#if !VENDOR_AI3_ENABLED && !HDAL_PIPELINE_ENABLED
    spdlog::info("HDAL disabled, AI models not initialized");
    return true;
#endif
}

bool AnalyticsEngine::InitHdalPath() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Initializing HDAL analytics path (VideoProc OUT_{})...", config_.videoproc_out_id);
    
    // The path IDs are set via SetVideoProcPath() and SetEncoderPath()
    // which should be called after HdalPipeline::Init() completes.
    // Here we just validate that we're ready to receive them.
    
    if (proc_alg_path_ != 0) {
        spdlog::info("HDAL analytics path already configured: {}", static_cast<int>(proc_alg_path_));
    } else {
        spdlog::info("HDAL analytics path pending - call SetVideoProcPath() after HdalPipeline::Init()");
    }
    
    return true;
#else
    spdlog::info("HDAL disabled, path not initialized");
    return true;
#endif
}

void AnalyticsEngine::SetVideoProcPath(uint64_t proc_path) {
    std::lock_guard<std::mutex> lock(mutex_);
#if HDAL_PIPELINE_ENABLED
    proc_alg_path_ = static_cast<HD_PATH_ID>(proc_path);
    spdlog::info("Analytics VideoProc path set: {}", proc_path);
#else
    (void)proc_path;
    spdlog::debug("HDAL disabled, SetVideoProcPath ignored");
#endif
}

void AnalyticsEngine::SetEncoderPath(uint64_t enc_path) {
    std::lock_guard<std::mutex> lock(mutex_);
#if HDAL_PIPELINE_ENABLED
    enc_path_ = static_cast<HD_PATH_ID>(enc_path);
    spdlog::info("Analytics encoder path set: {}", enc_path);
#else
    (void)enc_path;
    spdlog::debug("HDAL disabled, SetEncoderPath ignored");
#endif
}

bool AnalyticsEngine::HasHdalPaths() const {
#if HDAL_PIPELINE_ENABLED
    return proc_alg_path_ != 0;
#else
    return false;
#endif
}

bool AnalyticsEngine::Start() {
    if (!initialized_.load()) {
        spdlog::error("Cannot start analytics - not initialized");
        return false;
    }
    
    if (!config_.enabled) {
        spdlog::info("Analytics disabled, not starting");
        return true;
    }
    
    if (running_.load()) {
        spdlog::warn("Analytics already running");
        return true;
    }
    
    spdlog::info("Starting Analytics Engine...");
    
    running_.store(true);
    processing_thread_ = std::thread(&AnalyticsEngine::ProcessingThread, this);
    
    // Start dedicated MD processing thread if MD is enabled and path available
#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
    if (config_.motion_detection.enabled) {
        auto& pipeline = platform::HdalPipeline::Instance();
        if (pipeline.IsMdPathAvailable()) {
            md_proc_path_ = static_cast<HD_PATH_ID>(pipeline.GetMdPath());
            int md_w = 0, md_h = 0;
            if (pipeline.GetMdDimensions(md_w, md_h)) {
                md_width_ = md_w;
                md_height_ = md_h;
            } else {
                md_width_ = 160;
                md_height_ = 120;
            }
            
            md_thread_running_.store(true);
            md_processing_thread_ = std::thread(&AnalyticsEngine::MdProcessingThread, this);
            spdlog::info("MD processing thread started ({}x{})", md_width_, md_height_);
        } else {
            spdlog::info("MD path not available - motion detection uses main processing path");
        }
    }
#endif
    
    // Start tamper detection engine (uses IVE hardware directly, not old VQA SDK)
#if HDAL_PIPELINE_ENABLED
    if (config_.tamper_detection.enabled) {
        try {
            auto& tamper = TamperDetectionEngine::Instance();
            auto& pipeline = platform::HdalPipeline::Instance();
            
            // Use the main VideoProc path for frame capture (320x180 scaled down from main stream)
            // The IVE-based tamper detection doesn't need the old VQA SDK path
            uint64_t videoproc_path = pipeline.GetVideoProcPath(config_.videoproc_out_id);
            tamper.SetVideoProcPath(static_cast<uint32_t>(videoproc_path));
            
            // Start tamper detection (runs its own processing thread with IVE acceleration)
            if (tamper.Start()) {
                spdlog::info("Tamper Detection Engine started (IVE-accelerated, 320x180)");
            } else {
                spdlog::warn("Tamper Detection Engine failed to start");
            }
        } catch (const std::exception& e) {
            spdlog::warn("Tamper Detection start exception: {}", e.what());
        }
    } else {
        spdlog::info("Tamper Detection disabled - engine not started");
    }
#endif
    
    // Start audio classification thread if enabled and engine is ready
    if (config_.audio_classification.enabled && audio_classification_engine_ &&
        audio_classification_engine_->IsInitialized()) {
        try {
            auto& broadcaster = streaming::AudioFrameBroadcaster::Instance();
            audio_consumer_ = broadcaster.RegisterPcmConsumer();
            if (audio_consumer_ && audio_consumer_->IsValid()) {
                audio_thread_running_.store(true);
                audio_processing_thread_ = std::thread(&AnalyticsEngine::AudioProcessingThread, this);
                spdlog::info("Audio classification thread started");
            } else {
                spdlog::warn("Failed to register PCM consumer for audio classification");
            }
        } catch (const std::exception& e) {
            spdlog::warn("Audio classification start exception: {}", e.what());
        }
    }
    
    spdlog::info("Analytics Engine started");
    return true;
}

void AnalyticsEngine::Stop() {
    if (!running_.load()) {
        return;
    }
    
    spdlog::info("Stopping Analytics Engine...");
    
    running_.store(false);
    
    // Stop audio classification thread
    if (audio_thread_running_.load()) {
        audio_thread_running_.store(false);
        if (audio_processing_thread_.joinable()) {
            audio_processing_thread_.join();
        }
        audio_consumer_.reset();
        spdlog::info("Audio classification thread stopped");
    }
    
    // Stop MD thread first
    if (md_thread_running_.load()) {
        md_thread_running_.store(false);
        if (md_processing_thread_.joinable()) {
            md_processing_thread_.join();
        }
        spdlog::info("MD processing thread stopped");
    }
    
    if (processing_thread_.joinable()) {
        processing_thread_.join();
    }
    
    spdlog::info("Analytics Engine stopped");
}

void AnalyticsEngine::Shutdown() {
    Stop();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    spdlog::info("Shutting down Analytics Engine...");
    
    // Shutdown new AI modules
    try {
        TamperDetectionEngine::Instance().Shutdown();
        spdlog::info("Tamper Detection Engine shutdown complete");
    } catch (...) {}

#if AI_MD_ENABLED
    try {
        MotionDetectionEngine::Instance().Shutdown();
        spdlog::info("Motion Detection Engine shutdown complete");
    } catch (...) {}
#endif

#if AI_TRKE_ENABLED
    try {
        ObjectTracker::Instance().Clear();
        spdlog::info("Object Tracker reset complete");
    } catch (...) {}
#endif

#if AI_GFX_ENABLED
    try {
        PrivacyMosaicEngine::Instance().Shutdown();
        spdlog::info("Privacy Mosaic Engine shutdown complete");
    } catch (...) {}
#endif

#if AI_ISP_ENABLED
    try {
        AiispEngine::Instance().Shutdown();
        spdlog::info("AIISP Engine shutdown complete");
    } catch (...) {}
#endif

    // Shutdown Tamper Detection Engine
    try {
        TamperDetectionEngine::Instance().Shutdown();
        spdlog::info("Tamper Detection Engine shutdown complete");
    } catch (...) {}

#if VENDOR_AI3_ENABLED
    // Cleanup AI3 network if open
    if (ai3_proc_id_ != 0) {
        vendor_ai3_net_stop(ai3_proc_id_);
        vendor_ai3_net_close(ai3_proc_id_);
        ai3_proc_id_ = 0;
    }
    
    // Free shared AI crop buffer
    if (ai_crop_pa_ && ai_crop_va_) {
        hd_common_mem_free(ai_crop_pa_, reinterpret_cast<void*>(ai_crop_va_));
        ai_crop_pa_ = 0;
        ai_crop_va_ = 0;
    }

    // Unload all NPU models
    auto& npu = NpuInference::Instance();
    auto unload = [&](int& net_id) {
        if (net_id >= 0) {
            try { npu.UnloadNetwork(net_id); } catch (...) {}
            net_id = -1;
        }
    };
    unload(osnet_net_id_);
    unload(yolo_net_id_);
    unload(scrfd_net_id_);
    unload(mobilefacenet_id_);
    unload(face_attr_net_id_);
    unload(lp_det_net_id_);
    unload(lp_ocr_net_id_);
    unload(pose_net_id_);

    // Reset new analytics engines
    yolo_postproc_.reset();
    scrfd_postproc_.reset();
    face_recognition_engine_.reset();
    lpr_engine_.reset();
    heat_map_engine_.reset();
    pose_estimation_engine_.reset();
    face_attribute_engine_.reset();
    audio_classification_engine_.reset();
    
    // Shutdown NPU inference
    try {
        NpuInference::Instance().Shutdown();
    } catch (...) {}
    
    // Uninit AI3 device
    if (ai3_initialized_) {
        vendor_ai3_dev_uninit();
        ai3_initialized_ = false;
    }
#endif

    // Shutdown smart analytics consumer modules
    if (line_crossing_engine_) {
        line_crossing_engine_->Shutdown();
        line_crossing_engine_.reset();
    }
    if (zone_intrusion_engine_) {
        zone_intrusion_engine_->Shutdown();
        zone_intrusion_engine_.reset();
    }
    if (counting_engine_) {
        counting_engine_->Shutdown();
        counting_engine_.reset();
    }

#if HDAL_PIPELINE_ENABLED
    proc_alg_path_ = 0;
    enc_path_ = 0;
#endif
    
    prev_frame_.clear();
    tracked_objects_.clear();
    
    initialized_.store(false);
    spdlog::info("Analytics Engine shutdown complete");
}

// ============================================================================
// Processing Thread
// ============================================================================

void AnalyticsEngine::ProcessingThread() {
    spdlog::info("Analytics processing thread started");
    
    int frame_interval_ms = 1000 / config_.input.fps;
    int64_t last_frame_time = 0;
    uint64_t frame_count = 0;
    
#if HDAL_PIPELINE_ENABLED
    HD_VIDEO_FRAME video_frame;
    memset(&video_frame, 0, sizeof(video_frame));
#endif
    
    while (running_.load()) {
        int64_t current_time = GetCurrentTimeMs();
        
        // Rate limiting
        if (current_time - last_frame_time < frame_interval_ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
#if HDAL_PIPELINE_ENABLED
        // Check if path is ready
        if (proc_alg_path_ == 0) {
            // Path not set yet - run in simulation mode
            std::this_thread::sleep_for(std::chrono::milliseconds(frame_interval_ms));
            
            DetectionFrame detection_frame;
            detection_frame.frame_number = frame_count++;
            detection_frame.timestamp = current_time;
            detection_frame.source_width = config_.input.width;
            detection_frame.source_height = config_.input.height;
            
            if (detection_callback_) {
                detection_callback_(detection_frame);
            }
            last_frame_time = current_time;
            continue;
        }
        
        // Pull YUV frame from VideoProc
        HD_RESULT ret = hd_videoproc_pull_out_buf(proc_alg_path_, &video_frame, 100);  // 100ms timeout
        
        if (ret != HD_OK) {
            if (ret != HD_ERR_TIMEDOUT) {
                spdlog::warn("Failed to pull frame from VideoProc: {}", (int)ret);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        int64_t process_start = GetCurrentTimeMs();
        
        // Process the frame
        DetectionFrame detection_frame;
        detection_frame.frame_number = frame_count++;
        detection_frame.timestamp = current_time;
        detection_frame.source_width = config_.input.width;
        detection_frame.source_height = config_.input.height;
        
        // Get YUV data pointer
        uint8_t* yuv_data = nullptr;
        UINTPTR pa = 0;
        if (video_frame.blk != HD_COMMON_MEM_VB_INVALID_BLK) {
            pa = hd_common_mem_blk2pa(video_frame.blk);
            if (pa > 0) {
                yuv_data = (uint8_t*)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, 
                    config_.input.width * config_.input.height * 3 / 2);
            }
        }
        
        if (yuv_data) {
            // Run AI detection
            ProcessDetections(detection_frame, yuv_data, pa);

            // Heat map accumulation
            if (heat_map_engine_ && heat_map_engine_->IsEnabled()) {
                try {
                    heat_map_engine_->Accumulate(detection_frame.detections);
                } catch (const std::exception& e) {
                    spdlog::debug("HeatMap exception: {}", e.what());
                }
            }
            
            // Check line crossing (legacy method)
            if (config_.line_crossing.enabled && !line_crossing_engine_) {
                CheckLineCrossing(detection_frame);
            }
            
            // --- Smart Analytics Consumer Modules ---
            // These consume tracked detection results from the frame
            
            // Line Crossing Engine (improved, replaces legacy CheckLineCrossing)
            if (line_crossing_engine_ && line_crossing_engine_->IsEnabled()) {
                try {
                    auto lc_events = line_crossing_engine_->Check(detection_frame.detections);
                    for (const auto& evt : lc_events) {
                        detection_frame.lines_crossed.push_back(evt.line_id);
                    }
                    // Forward events to counting engine
                    if (counting_engine_ && counting_engine_->IsEnabled() && !lc_events.empty()) {
                        counting_engine_->OnLineCrossEvents(lc_events);
                    }
                } catch (const std::exception& e) {
                    spdlog::debug("LineCrossing exception: {}", e.what());
                }
            }
            
            // Zone Intrusion Engine
            if (zone_intrusion_engine_ && zone_intrusion_engine_->IsEnabled()) {
                try {
                    auto zi_events = zone_intrusion_engine_->Check(detection_frame.detections);
                    // Zone intrusion events are dispatched via the engine's callback
                    (void)zi_events;
                } catch (const std::exception& e) {
                    spdlog::debug("ZoneIntrusion exception: {}", e.what());
                }
            }
            
            // Counting Engine: also update area-based counts
            if (counting_engine_ && counting_engine_->IsEnabled()) {
                try {
                    counting_engine_->UpdateAreaCounts(detection_frame.detections);
                    counting_engine_->CheckAutoReset();
                } catch (const std::exception& e) {
                    spdlog::debug("Counting exception: {}", e.what());
                }
            }
            
            // Feed frame to tamper detection (downsample from 640x360 to 320x180)
            try {
                auto& tamper = TamperDetectionEngine::Instance();
                if (tamper.IsRunning() && config_.tamper_detection.enabled) {
                    // Simple 2x2 box filter downsample for Y channel only
                    int src_w = config_.input.width;
                    int src_h = config_.input.height;
                    int dst_w = src_w / 2;  // 320
                    int dst_h = src_h / 2;  // 180
                    
                    // Allocate downsampled buffer (reuse static buffer to avoid allocs)
                    static std::vector<uint8_t> downsample_buf;
                    if (downsample_buf.size() < static_cast<size_t>(dst_w * dst_h)) {
                        downsample_buf.resize(dst_w * dst_h);
                    }
                    
                    // 2x2 box filter downsample
                    for (int y = 0; y < dst_h; y++) {
                        for (int x = 0; x < dst_w; x++) {
                            int src_x = x * 2;
                            int src_y = y * 2;
                            int sum = yuv_data[src_y * src_w + src_x] +
                                      yuv_data[src_y * src_w + src_x + 1] +
                                      yuv_data[(src_y + 1) * src_w + src_x] +
                                      yuv_data[(src_y + 1) * src_w + src_x + 1];
                            downsample_buf[y * dst_w + x] = static_cast<uint8_t>(sum / 4);
                        }
                    }
                    
                    // Process frame for tamper detection
                    tamper.ProcessFrame(downsample_buf.data(), dst_w, dst_h);
                }
            } catch (...) {
                // Tamper detection failure shouldn't affect main processing
            }
            
            // Update SMART_BBOX overlay
            if (config_.output.overlay_enabled && !detection_frame.detections.empty()) {
                UpdateSmartBbox(detection_frame);
            }
            
            // Unmap memory
            hd_common_mem_munmap(yuv_data, config_.input.width * config_.input.height * 3 / 2);
        }
        
        // Release frame buffer
        hd_videoproc_release_out_buf(proc_alg_path_, &video_frame);
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stats_.frames_processed++;
            stats_.total_detections += detection_frame.detections.size();
            
            int64_t process_time = GetCurrentTimeMs() - process_start;
            stats_.avg_process_time_ms = (stats_.avg_process_time_ms * 0.9f) + (process_time * 0.1f);
            stats_.current_fps = 1000.0f / (current_time - last_frame_time);
        }
        
        // Invoke callback
        if (detection_callback_) {
            detection_callback_(detection_frame);
        }
        
#else
        // Simulation mode when HDAL is disabled
        std::this_thread::sleep_for(std::chrono::milliseconds(frame_interval_ms));
        
        DetectionFrame detection_frame;
        detection_frame.frame_number = frame_count++;
        detection_frame.timestamp = current_time;
        
        // Invoke callback with empty frame
        if (detection_callback_) {
            detection_callback_(detection_frame);
        }
#endif
        
        last_frame_time = current_time;
    }
    
    spdlog::info("Analytics processing thread stopped");
}

// ============================================================================
// MD Processing Thread (dedicated 160x120 path for libmd)
// ============================================================================

void AnalyticsEngine::MdProcessingThread() {
    spdlog::info("MD processing thread started ({}x{})", md_width_, md_height_);
    
    constexpr int MD_FPS = 10;  // Process MD at 10fps (sufficient for motion detection)
    int frame_interval_ms = 1000 / MD_FPS;
    int64_t last_frame_time = 0;
    uint64_t md_frame_count = 0;
    
#if HDAL_PIPELINE_ENABLED && AI_MD_ENABLED
    HD_VIDEO_FRAME md_frame;
    memset(&md_frame, 0, sizeof(md_frame));
    
    auto& md_engine = MotionDetectionEngine::Instance();
    
    while (md_thread_running_.load()) {
        int64_t current_time = GetCurrentTimeMs();
        
        // Rate limiting for MD (10fps is enough)
        if (current_time - last_frame_time < frame_interval_ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
        // Pull YUV frame from dedicated MD VideoProc path
        HD_RESULT ret = hd_videoproc_pull_out_buf(md_proc_path_, &md_frame, 100);  // 100ms timeout
        
        if (ret != HD_OK) {
            if (ret != HD_ERR_TIMEDOUT) {
                spdlog::debug("MD: Failed to pull frame: {}", static_cast<int>(ret));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        // Get YUV data pointer
        uint8_t* yuv_data = nullptr;
        size_t yuv_size = md_width_ * md_height_ * 3 / 2;
        
        if (md_frame.blk != HD_COMMON_MEM_VB_INVALID_BLK) {
            UINTPTR pa = hd_common_mem_blk2pa(md_frame.blk);
            if (pa > 0) {
                yuv_data = reinterpret_cast<uint8_t*>(
                    hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, yuv_size));
            }
        }
        
        if (yuv_data && md_engine.IsRunning()) {
            // Process motion detection on 160x120 frame
            MdResult md_result = md_engine.ProcessFrame(yuv_data, md_width_, md_height_);
            
            md_frame_count++;
            
            // Process motion zone callbacks with state tracking
            for (size_t i = 0; i < md_result.zone_results.size(); ++i) {
                const auto& zone_result = md_result.zone_results[i];
                
                // Find zone index by ID
                size_t zone_idx = SIZE_MAX;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    for (size_t j = 0; j < config_.motion_detection.zones.size(); ++j) {
                        if (config_.motion_detection.zones[j].id == zone_result.zone_id) {
                            zone_idx = j;
                            break;
                        }
                    }
                }
                
                if (zone_idx < motion_zone_active_.size() && motion_callback_) {
                    bool was_active = motion_zone_active_[zone_idx];
                    bool is_active = zone_result.motion_detected;
                    
                    if (is_active && !was_active) {
                        motion_zone_active_[zone_idx] = true;
                        motion_callback_(zone_result.zone_id, true);
                        spdlog::debug("MD Thread: Motion START in zone {}", zone_result.zone_id);
                    } else if (!is_active && was_active) {
                        motion_zone_active_[zone_idx] = false;
                        motion_callback_(zone_result.zone_id, false);
                        spdlog::debug("MD Thread: Motion END in zone {}", zone_result.zone_id);
                    }
                }
            }
            
            // Process line crossing events
            for (const auto& event : md_result.line_events) {
                if (linecross_callback_) {
                    linecross_callback_(event.line_id, event.object_id, ObjectCategory::kMotion);
                }
            }
            
            // Update global motion state
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (md_result.motion_detected) {
                    stats_.motion_events++;
                }
            }
            
            // Unmap memory
            hd_common_mem_munmap(yuv_data, yuv_size);
        }
        
        // Release frame buffer
        hd_videoproc_release_out_buf(md_proc_path_, &md_frame);
        
        last_frame_time = current_time;
    }
#endif
    
    spdlog::info("MD processing thread stopped (processed {} frames)", md_frame_count);
}

// ============================================================================
// Detection Processing
// ============================================================================

void AnalyticsEngine::ProcessDetections(DetectionFrame& frame, const uint8_t* yuv_data, uintptr_t frame_pa) {
#if HDAL_PIPELINE_ENABLED
    // ========================================================================
    // Tamper Detection (uses IVE hardware - histogram/Sobel acceleration)
    // Note: Tamper detection runs on its own thread with dedicated 320x180 path
    // This ProcessDetections runs at analytics resolution (640x360)
    // ========================================================================
    // Tamper detection is handled by TamperDetectionEngine in its own thread
    // pulling from the dedicated VQA 320x180 videoproc path

    // ========================================================================
    // Motion Detection (requires libmd.so SDK)
    // Skip if dedicated MD thread is running (it handles MD at 160x120 resolution)
    // ========================================================================
    if (!md_thread_running_.load()) {
        try {
            auto& md = MotionDetectionEngine::Instance();
            if (md.IsRunning() && config_.motion_detection.enabled) {
                MdResult md_result = md.ProcessFrame(yuv_data, config_.input.width, config_.input.height);
                
                // Track motion state change - only log on transitions
                frame.motion_detected = md_result.motion_detected;
                
                if (md_result.motion_detected && !motion_active_) {
                    // Motion just started
                    std::lock_guard<std::mutex> lock(mutex_);
                    stats_.motion_events++;
                    motion_active_ = true;
                    spdlog::info("Motion started (level: {:.1f}%)", md_result.motion_level);
                } else if (!md_result.motion_detected && motion_active_) {
                    // Motion just stopped
                    motion_active_ = false;
                    spdlog::info("Motion stopped");
                }
            
            // Add detected objects as motion detections
            for (const auto& obj : md_result.detected_objects) {
                DetectionResult det;
                det.id = obj.id;
                det.category = ObjectCategory::kMotion;
                det.confidence = obj.confidence;
                det.timestamp = frame.timestamp;
                det.bbox.x1 = static_cast<float>(obj.x) / config_.input.width;
                det.bbox.y1 = static_cast<float>(obj.y) / config_.input.height;
                det.bbox.x2 = static_cast<float>(obj.x + obj.width) / config_.input.width;
                det.bbox.y2 = static_cast<float>(obj.y + obj.height) / config_.input.height;
                frame.detections.push_back(det);
            }
            
            // Invoke motion callback for zone triggers with proper start/end state tracking
            for (size_t i = 0; i < md_result.zone_results.size(); ++i) {
                const auto& zone_result = md_result.zone_results[i];
                
                // Find the index in our zones array by zone_id
                size_t zone_idx = SIZE_MAX;
                for (size_t j = 0; j < config_.motion_detection.zones.size(); ++j) {
                    if (config_.motion_detection.zones[j].id == zone_result.zone_id) {
                        zone_idx = j;
                        break;
                    }
                }
                
                if (zone_idx < motion_zone_active_.size() && motion_callback_) {
                    bool was_active = motion_zone_active_[zone_idx];
                    bool is_active = zone_result.motion_detected;
                    
                    if (is_active && !was_active) {
                        // Motion start
                        motion_zone_active_[zone_idx] = true;
                        motion_callback_(zone_result.zone_id, true);
                        spdlog::debug("Motion START in zone {}", zone_result.zone_id);
                    } else if (!is_active && was_active) {
                        // Motion end
                        motion_zone_active_[zone_idx] = false;
                        motion_callback_(zone_result.zone_id, false);
                        spdlog::debug("Motion END in zone {}", zone_result.zone_id);
                    }
                    // Continuous motion (no callback, already active)
                }
            }
            
            // Check line crossings from motion detection
            for (const auto& event : md_result.line_events) {
                frame.lines_crossed.push_back(event.line_id);
                if (linecross_callback_) {
                    linecross_callback_(event.line_id, event.object_id, ObjectCategory::kMotion);
                }
            }
        }
    } catch (const std::exception& e) {
        spdlog::debug("Motion detection exception: {}", e.what());
    }
    }  // End of !md_thread_running_ block

    // ========================================================================
    // AI Object Detection — YOLO on NPU
    // ========================================================================
#if VENDOR_AI3_ENABLED
    if (yolo_net_id_ >= 0 && config_.object_detection.enabled && yuv_data && frame_pa) {
        try {
            auto& npu = NpuInference::Instance();
            // Flush cache before NPU reads the frame
            hd_common_mem_cache_sync(const_cast<uint8_t*>(yuv_data),
                                     config_.input.width * config_.input.height * 3 / 2,
                                     HD_COMMON_MEM_DMA_TO_DEVICE);

            if (npu.Infer(yolo_net_id_, frame_pa, reinterpret_cast<uintptr_t>(yuv_data),
                          config_.input.width, config_.input.height, config_.input.width)) {
                if (yolo_postproc_) {
                    auto yolo_dets = yolo_postproc_->Process(npu, yolo_net_id_);
                    for (auto& det : yolo_dets) {
                        // Apply per-class confidence thresholds from config
                        if (det.category == ObjectCategory::kPerson &&
                            (!config_.object_detection.person.enabled ||
                             det.confidence < config_.object_detection.person.min_confidence))
                            continue;
                        if (det.category == ObjectCategory::kVehicle &&
                            (!config_.object_detection.vehicle.enabled ||
                             det.confidence < config_.object_detection.vehicle.min_confidence))
                            continue;

                        det.timestamp = frame.timestamp;
                        det.pixel_x1 = static_cast<int>(det.bbox.x1 * frame.source_width);
                        det.pixel_y1 = static_cast<int>(det.bbox.y1 * frame.source_height);
                        det.pixel_x2 = static_cast<int>(det.bbox.x2 * frame.source_width);
                        det.pixel_y2 = static_cast<int>(det.bbox.y2 * frame.source_height);
                        frame.detections.push_back(det);
                    }
                }
            }
        } catch (const std::exception& e) {
            spdlog::debug("YOLO inference exception: {}", e.what());
        }
    }
#endif

    // ========================================================================
    // SCRFD Face Detection — on NPU
    // ========================================================================
#if VENDOR_AI3_ENABLED
    std::vector<FaceDetection> face_dets;
    if (scrfd_net_id_ >= 0 && config_.face_detection.enabled && yuv_data && frame_pa) {
        try {
            auto& npu = NpuInference::Instance();
            if (npu.Infer(scrfd_net_id_, frame_pa, reinterpret_cast<uintptr_t>(yuv_data),
                          config_.input.width, config_.input.height, config_.input.width)) {
                if (scrfd_postproc_) {
                    face_dets = scrfd_postproc_->Process(npu, scrfd_net_id_);
                    for (const auto& face : face_dets) {
                        if (face.confidence < config_.face_detection.confidence_threshold)
                            continue;
                        // Convert to pixel size and filter by min_face_size
                        int face_w = static_cast<int>((face.x2 - face.x1) * frame.source_width);
                        int face_h = static_cast<int>((face.y2 - face.y1) * frame.source_height);
                        if (face_w < config_.face_detection.min_face_size ||
                            face_h < config_.face_detection.min_face_size)
                            continue;

                        DetectionResult det;
                        det.category = ObjectCategory::kFace;
                        det.confidence = face.confidence;
                        det.bbox.x1 = face.x1; det.bbox.y1 = face.y1;
                        det.bbox.x2 = face.x2; det.bbox.y2 = face.y2;
                        det.timestamp = frame.timestamp;
                        det.pixel_x1 = static_cast<int>(face.x1 * frame.source_width);
                        det.pixel_y1 = static_cast<int>(face.y1 * frame.source_height);
                        det.pixel_x2 = static_cast<int>(face.x2 * frame.source_width);
                        det.pixel_y2 = static_cast<int>(face.y2 * frame.source_height);
                        frame.detections.push_back(det);
                    }
                }
            }
        } catch (const std::exception& e) {
            spdlog::debug("SCRFD inference exception: {}", e.what());
        }
    }
#endif

    // Build face results from SCRFD detections
    std::vector<FaceMatch> face_matches;
    std::vector<FaceAttributes> face_attrs;

    // ========================================================================
    // Face Recognition — MobileFaceNet embedding matching (requires face dets)
    // ========================================================================
#if VENDOR_AI3_ENABLED
    if (face_recognition_engine_ && face_recognition_engine_->IsEnabled() &&
        mobilefacenet_id_ >= 0 && !face_dets.empty() && ai_crop_va_) {
        try {
            auto& npu = NpuInference::Instance();
            face_matches = face_recognition_engine_->ProcessFaces(
                face_dets, yuv_data, config_.input.width, config_.input.height,
                config_.input.width, npu, mobilefacenet_id_, ai_crop_pa_, ai_crop_va_);
        } catch (const std::exception& e) {
            spdlog::debug("Face recognition exception: {}", e.what());
        }
    }
#endif

    // ========================================================================
    // Face Attributes — gender/age (requires face dets)
    // ========================================================================
#if VENDOR_AI3_ENABLED
    if (face_attribute_engine_ && face_attribute_engine_->IsEnabled() &&
        face_attr_net_id_ >= 0 && !face_dets.empty() && ai_crop_va_) {
        try {
            auto& npu = NpuInference::Instance();
            face_attrs = face_attribute_engine_->Process(
                face_dets, yuv_data, config_.input.width, config_.input.height,
                config_.input.width, npu, face_attr_net_id_, ai_crop_pa_, ai_crop_va_);
        } catch (const std::exception& e) {
            spdlog::debug("Face attribute exception: {}", e.what());
        }
    }
#endif

    // Assemble FaceResult entries and populate frame.faces
    for (size_t i = 0; i < face_dets.size(); i++) {
        FaceResult fr;
        fr.bbox.x1 = face_dets[i].x1;
        fr.bbox.y1 = face_dets[i].y1;
        fr.bbox.x2 = face_dets[i].x2;
        fr.bbox.y2 = face_dets[i].y2;
        fr.confidence = face_dets[i].confidence;
        if (i < face_matches.size()) {
            fr.recognition = face_matches[i];
            fr.has_recognition = true;
        }
        if (i < face_attrs.size()) {
            fr.attributes = face_attrs[i];
            fr.has_attributes = true;
        }
        frame.faces.push_back(fr);
        // Fire callback for recognised faces
        if (fr.has_recognition && fr.recognition.is_known && face_recognition_callback_) {
            face_recognition_callback_(fr);
        }
    }

    // ========================================================================
    // Pose Estimation — YOLOv26-Pose (independent full-frame inference)
    // ========================================================================
#if VENDOR_AI3_ENABLED
    if (pose_estimation_engine_ && pose_estimation_engine_->IsEnabled() &&
        pose_net_id_ >= 0 && yuv_data && frame_pa) {
        try {
            auto& npu = NpuInference::Instance();
            if (npu.Infer(pose_net_id_, frame_pa, reinterpret_cast<uintptr_t>(yuv_data),
                          config_.input.width, config_.input.height, config_.input.width)) {
                frame.poses = pose_estimation_engine_->Process(npu, pose_net_id_);
            }
        } catch (const std::exception& e) {
            spdlog::debug("Pose estimation exception: {}", e.what());
        }
    }
#endif

    // ========================================================================
    // License Plate Recognition — YOLOv11-LP + LPRNet OCR
    // ========================================================================
#if VENDOR_AI3_ENABLED
    if (lpr_engine_ && lpr_engine_->IsEnabled() &&
        lp_det_net_id_ >= 0 && yuv_data && frame_pa) {
        try {
            auto& npu = NpuInference::Instance();
            if (npu.Infer(lp_det_net_id_, frame_pa, reinterpret_cast<uintptr_t>(yuv_data),
                          config_.input.width, config_.input.height, config_.input.width)) {
                auto plates = lpr_engine_->Process(
                    npu, lp_det_net_id_, lp_ocr_net_id_,
                    yuv_data, config_.input.width, config_.input.height, config_.input.width,
                    ai_crop_pa_, ai_crop_va_);
                for (const auto& plate : plates) {
                    if (!plate.plate_text.empty()) {
                        spdlog::info("LPR: {} (conf={:.2f})", plate.plate_text, plate.confidence);
                        frame.plates.push_back(plate);
                        if (lpr_callback_) {
                            lpr_callback_(plate);
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            spdlog::debug("LPR exception: {}", e.what());
        }
    }
#endif

    // ========================================================================
    // Object Tracking - IOU-based multi-object tracker
    // ========================================================================
#if AI_TRKE_ENABLED
    try {
        auto& tracker = ObjectTracker::Instance();
        if (tracker.IsInitialized() && config_.object_detection.tracking.enabled) {
            // Convert DetectionResult to tracker Detection format
            std::vector<Detection> detections;
            for (const auto& det : frame.detections) {
                if (det.category != ObjectCategory::kMotion) {
                    Detection d;
                    d.x1 = det.bbox.x1;
                    d.y1 = det.bbox.y1;
                    d.x2 = det.bbox.x2;
                    d.y2 = det.bbox.y2;
                    d.confidence = det.confidence;
                    d.class_id = static_cast<int>(det.category);
                    detections.push_back(d);
                }
            }
            
            // Extract Re-ID features for person detections if OSNet is loaded
            if (osnet_net_id_ >= 0 && yuv_data) {
                ExtractReidFeatures(detections, yuv_data,
                                    config_.input.width, config_.input.height,
                                    config_.input.width);  // stride = width for packed NV12
            }

            // Update tracker (if features are populated, it uses IoU+cosine blending)
            auto tracked = tracker.Update(detections);
            
            // Update detection IDs with tracking IDs
            for (auto& det : frame.detections) {
                for (const auto& trk : tracked) {
                    // Convert TrackPoint to bounding box for comparison
                    BoundingBox trk_bbox;
                    trk_bbox.x1 = trk.current.x - trk.current.width / 2.0f;
                    trk_bbox.y1 = trk.current.y - trk.current.height / 2.0f;
                    trk_bbox.x2 = trk.current.x + trk.current.width / 2.0f;
                    trk_bbox.y2 = trk.current.y + trk.current.height / 2.0f;
                    
                    float iou = (det.bbox.x2 - det.bbox.x1) > 0 ? 
                        CalculateBboxOverlap(det.bbox, trk_bbox) : 0.0f;
                    if (iou > 0.5f) {
                        det.id = trk.id;
                        break;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        spdlog::debug("Object tracking exception: {}", e.what());
    }
#endif

    // ========================================================================
    // Privacy Mosaic - Apply blur to faces if privacy mode enabled
    // ========================================================================
#if AI_GFX_ENABLED
    try {
        auto& mosaic = PrivacyMosaicEngine::Instance();
        if (mosaic.IsInitialized()) {
            std::vector<MosaicTarget> targets;
            for (const auto& det : frame.detections) {
                if (det.category == ObjectCategory::kFace) {
                    MosaicTarget target;
                    target.x = static_cast<int>(det.bbox.x1 * config_.input.width);
                    target.y = static_cast<int>(det.bbox.y1 * config_.input.height);
                    target.width = static_cast<int>((det.bbox.x2 - det.bbox.x1) * config_.input.width);
                    target.height = static_cast<int>((det.bbox.y2 - det.bbox.y1) * config_.input.height);
                    target.type = MosaicType::kBlock;
                    targets.push_back(target);
                }
            }
            
            if (!targets.empty()) {
                mosaic.ApplyMosaic(nullptr, config_.input.width, config_.input.height, targets);
            }
        }
    } catch (const std::exception& e) {
        spdlog::debug("Privacy mosaic exception: {}", e.what());
    }
#endif

#endif  // HDAL_PIPELINE_ENABLED
}

void AnalyticsEngine::CheckLineCrossing(const DetectionFrame& frame) {
    // For each tracked object, check if it crossed any line
    for (const auto& det : frame.detections) {
        float cx = det.bbox.CenterX();
        float cy = det.bbox.CenterY();
        
        // Find previous position
        for (const auto& tracked : tracked_objects_) {
            if (tracked.id == det.id) {
                float prev_cx = tracked.bbox.CenterX();
                float prev_cy = tracked.bbox.CenterY();
                
                // Check each line
                for (const auto& line : config_.line_crossing.lines) {
                    if (!line.enabled) continue;
                    
                    // Simple line crossing detection using cross product
                    float d1 = (line.x2 - line.x1) * (prev_cy - line.y1) - (line.y2 - line.y1) * (prev_cx - line.x1);
                    float d2 = (line.x2 - line.x1) * (cy - line.y1) - (line.y2 - line.y1) * (cx - line.x1);
                    
                    bool bidirectional = (line.direction == "both");
                    if ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0) || 
                        (bidirectional && d1 * d2 < 0)) {
                        // Line crossed!
                        {
                            std::lock_guard<std::mutex> lock(mutex_);
                            stats_.line_cross_events++;
                        }
                        
                        if (linecross_callback_) {
                            linecross_callback_(line.id, det.id, det.category);
                        }
                        
                        spdlog::info("Line crossing detected: line={}, object={}, category={}",
                                     line.id, det.id, ObjectCategoryToString(det.category));
                    }
                }
                break;
            }
        }
    }
    
    // Update tracked objects
    tracked_objects_.clear();
    for (const auto& det : frame.detections) {
        TrackedObject obj;
        obj.id = det.id;
        obj.category = det.category;
        obj.bbox = det.bbox;
        obj.frames_since_seen = 0;
        tracked_objects_.push_back(obj);
    }
}

void AnalyticsEngine::UpdateSmartBbox(const DetectionFrame& frame) {
#if HDAL_PIPELINE_ENABLED
    // Check if encoder path is ready
    if (enc_path_ == 0) {
        return;
    }
    
    // Pre-process detections: merge motion blocks into single box, keep others
    std::vector<DetectionResult> merged_detections;
    float motion_x1 = 1.0f, motion_y1 = 1.0f, motion_x2 = 0.0f, motion_y2 = 0.0f;
    bool has_motion = false;
    
    for (const auto& det : frame.detections) {
        if (det.category == ObjectCategory::kMotion) {
            // Merge motion blocks
            motion_x1 = std::min(motion_x1, det.bbox.x1);
            motion_y1 = std::min(motion_y1, det.bbox.y1);
            motion_x2 = std::max(motion_x2, det.bbox.x2);
            motion_y2 = std::max(motion_y2, det.bbox.y2);
            has_motion = true;
        } else {
            merged_detections.push_back(det);
        }
    }
    
    // Add merged motion box
    if (has_motion && motion_x2 > motion_x1 && motion_y2 > motion_y1) {
        DetectionResult motion_det;
        motion_det.category = ObjectCategory::kMotion;
        motion_det.bbox = {motion_x1, motion_y1, motion_x2, motion_y2};
        motion_det.confidence = 1.0f;
        merged_detections.push_back(motion_det);
    }
    
    // Send bounding boxes to encoder for overlay
    VENDOR_VIDEOENC_SMART_BBOX smart_bbox;
    memset(&smart_bbox, 0, sizeof(smart_bbox));
    
    smart_bbox.base_resolution.w = frame.source_width;
    smart_bbox.base_resolution.h = frame.source_height;
    smart_bbox.timestamp = 33333;  // Frame interval
    
    int bbox_count = std::min((int)merged_detections.size(), (int)VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM);
    smart_bbox.bbox_num = bbox_count;
    
    for (int i = 0; i < bbox_count; i++) {
        const auto& det = merged_detections[i];
        
        // Convert normalized coords to pixels
        smart_bbox.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].x = 
            (uint32_t)(det.bbox.x1 * frame.source_width);
        smart_bbox.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].y = 
            (uint32_t)(det.bbox.y1 * frame.source_height);
        smart_bbox.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].x = 
            (uint32_t)(det.bbox.x2 * frame.source_width);
        smart_bbox.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].y = 
            (uint32_t)(det.bbox.y2 * frame.source_height);
        
        // Map category to class ID using proper enum values
        if (det.category == ObjectCategory::kPerson) {
            smart_bbox.bbox[i].class_id = VENDOR_VIDEOENC_SMART_ROI_CLASS0;  // Highest priority
        } else if (det.category == ObjectCategory::kFace) {
            smart_bbox.bbox[i].class_id = VENDOR_VIDEOENC_SMART_ROI_CLASS1;
        } else if (det.category == ObjectCategory::kVehicle) {
            smart_bbox.bbox[i].class_id = VENDOR_VIDEOENC_SMART_ROI_CLASS2;
        } else {
            smart_bbox.bbox[i].class_id = VENDOR_VIDEOENC_SMART_ROI_CLASS3;
        }
    }
    
    HD_RESULT ret = vendor_videoenc_set(enc_path_, VENDOR_VIDEOENC_PARAM_SMART_BBOX, &smart_bbox);
    if (ret != HD_OK) {
        spdlog::warn("Failed to set SMART_BBOX: {}", (int)ret);
    }
#endif
}

// ============================================================================
// Manual Frame Processing
// ============================================================================

DetectionFrame AnalyticsEngine::ProcessFrame(const uint8_t* yuv_data, int width, int height) {
    DetectionFrame frame;
    frame.timestamp = GetCurrentTimeMs();
    frame.source_width = width;
    frame.source_height = height;
    
    // TODO: Process frame through AI models
    
    return frame;
}

// ============================================================================
// Audio Classification Processing Thread
// ============================================================================

void AnalyticsEngine::AudioProcessingThread() {
    spdlog::info("Audio classification thread started");
    
    // YAMNet expects ~0.96s of 16kHz mono PCM = 15360 samples = 30720 bytes
    static constexpr int kSamplesPerSegment = 15360;
    std::vector<int16_t> audio_buf;
    audio_buf.reserve(kSamplesPerSegment);
    
    while (audio_thread_running_.load()) {
        if (!audio_consumer_ || !audio_classification_engine_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        // Pull PCM frames from broadcaster and accumulate
        streaming::AudioFrame pcm_frame;
        while (audio_buf.size() < static_cast<size_t>(kSamplesPerSegment) &&
               audio_consumer_->GetNextFrame(pcm_frame, 50)) {
            // PCM frames contain 16-bit mono samples
            int num_samples = static_cast<int>(pcm_frame.size / sizeof(int16_t));
            const int16_t* samples = reinterpret_cast<const int16_t*>(pcm_frame.data);
            for (int i = 0; i < num_samples; ++i) {
                audio_buf.push_back(samples[i]);
                if (audio_buf.size() >= static_cast<size_t>(kSamplesPerSegment))
                    break;
            }
        }
        
        // Once we have a full segment, run classification
        if (audio_buf.size() >= static_cast<size_t>(kSamplesPerSegment)) {
            try {
                auto& npu = NpuInference::Instance();
                auto events = audio_classification_engine_->Process(
                    audio_buf.data(), kSamplesPerSegment,
                    npu, yamnet_net_id_,
                    0, 0);  // mel_pa/va = 0 means engine uses internal buffer
                
                if (!events.empty()) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (audio_event_callback_) {
                        for (const auto& ev : events) {
                            audio_event_callback_(ev);
                            spdlog::debug("Audio event: {} ({:.2f})", ev.class_name, ev.confidence);
                        }
                    }
                }
            } catch (const std::exception& e) {
                spdlog::warn("Audio classification exception: {}", e.what());
            }
            
            audio_buf.clear();
        }
        
        // If we didn't get enough data, wait a bit
        if (audio_buf.size() < static_cast<size_t>(kSamplesPerSegment)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    spdlog::info("Audio classification thread exiting");
}

// ============================================================================
// Helper Functions
// ============================================================================

std::string ObjectCategoryToString(ObjectCategory category) {
    switch (category) {
        case ObjectCategory::kPerson: return "person";
        case ObjectCategory::kFace: return "face";
        case ObjectCategory::kVehicle: return "vehicle";
        case ObjectCategory::kNonMotor: return "non_motor";
        case ObjectCategory::kMotion: return "motion";
        case ObjectCategory::kLineCross: return "line_cross";
        default: return "unknown";
    }
}

ObjectCategory StringToObjectCategory(const std::string& str) {
    if (str == "person") return ObjectCategory::kPerson;
    if (str == "face") return ObjectCategory::kFace;
    if (str == "vehicle") return ObjectCategory::kVehicle;
    if (str == "non_motor") return ObjectCategory::kNonMotor;
    if (str == "motion") return ObjectCategory::kMotion;
    if (str == "line_cross") return ObjectCategory::kLineCross;
    return ObjectCategory::kUnknown;
}

// ============================================================================
// Re-ID Feature Extraction (OSNet on NPU)
// ============================================================================

/// Simple NV12 nearest-neighbour crop+resize (matches demo implementation)
static void NV12CropResize(const uint8_t* src_y, const uint8_t* src_uv,
                            uint32_t src_w, uint32_t src_h, uint32_t src_stride,
                            uint32_t cx, uint32_t cy, uint32_t cw, uint32_t ch,
                            uint8_t* dst_y, uint8_t* dst_uv,
                            uint32_t dst_w, uint32_t dst_h)
{
    for (uint32_t dy = 0; dy < dst_h; dy++) {
        uint32_t sy = cy + (dy * ch) / dst_h;
        if (sy >= src_h) sy = src_h - 1;
        for (uint32_t dx = 0; dx < dst_w; dx++) {
            uint32_t sx = cx + (dx * cw) / dst_w;
            if (sx >= src_w) sx = src_w - 1;
            dst_y[dy * dst_w + dx] = src_y[sy * src_stride + sx];
        }
    }
    uint32_t dst_uvh = dst_h / 2;
    uint32_t dst_uvw = dst_w / 2;
    for (uint32_t dy = 0; dy < dst_uvh; dy++) {
        uint32_t sy = cy / 2 + (dy * (ch / 2)) / dst_uvh;
        if (sy >= src_h / 2) sy = src_h / 2 - 1;
        for (uint32_t dx = 0; dx < dst_uvw; dx++) {
            uint32_t sx = cx / 2 + (dx * (cw / 2)) / dst_uvw;
            if (sx >= src_w / 2) sx = src_w / 2 - 1;
            dst_uv[dy * dst_w + dx * 2]     = src_uv[sy * src_stride + sx * 2];
            dst_uv[dy * dst_w + dx * 2 + 1] = src_uv[sy * src_stride + sx * 2 + 1];
        }
    }
}

void AnalyticsEngine::ExtractReidFeatures(std::vector<Detection>& detections,
                                           const uint8_t* yuv_data, uint32_t yuv_w,
                                           uint32_t yuv_h, uint32_t yuv_stride) {
#if VENDOR_AI3_ENABLED
    if (osnet_net_id_ < 0 || !ai_crop_va_ || !yuv_data) return;

    auto& npu = NpuInference::Instance();
    const uint8_t* src_y  = yuv_data;
    const uint8_t* src_uv = yuv_data + yuv_stride * yuv_h;

    for (auto& det : detections) {
        // Only extract Re-ID for persons (COCO class 0 / ObjectCategory::kPerson)
        if (det.class_id != static_cast<int>(ObjectCategory::kPerson)) continue;

        // Convert normalised bbox to pixel coords (even-aligned for NV12)
        uint32_t bx = static_cast<uint32_t>(det.x1 * yuv_w) & ~1u;
        uint32_t by = static_cast<uint32_t>(det.y1 * yuv_h) & ~1u;
        uint32_t bw = static_cast<uint32_t>((det.x2 - det.x1) * yuv_w) & ~1u;
        uint32_t bh = static_cast<uint32_t>((det.y2 - det.y1) * yuv_h) & ~1u;
        if (bw < 16 || bh < 32) continue;
        if (bx + bw > yuv_w) bw = yuv_w - bx;
        if (by + bh > yuv_h) bh = yuv_h - by;

        // Crop + resize to OSNet input size (128x256)
        auto* dst_y  = reinterpret_cast<uint8_t*>(ai_crop_va_);
        auto* dst_uv = dst_y + kOsnetInputW * kOsnetInputH;
        NV12CropResize(src_y, src_uv, yuv_w, yuv_h, yuv_stride,
                       bx, by, bw, bh, dst_y, dst_uv,
                       kOsnetInputW, kOsnetInputH);

        // Flush crop buffer to DMA
        hd_common_mem_cache_sync(reinterpret_cast<void*>(ai_crop_va_),
                                 ai_crop_size_, HD_COMMON_MEM_DMA_TO_DEVICE);

        // Run OSNet inference
        if (!npu.Infer(osnet_net_id_, ai_crop_pa_, ai_crop_va_,
                       kOsnetInputW, kOsnetInputH, kOsnetInputW)) {
            continue;
        }

        // Extract 512-d feature vector from output tensor
        NpuOutputInfo out;
        if (!npu.GetOutput(osnet_net_id_, 0, out)) continue;

        // Dequantise + L2-normalise (matching osnet_postproc_process)
        det.features.resize(kOsnetFeatureDim);
        const auto* raw = reinterpret_cast<const int8_t*>(out.va);
        float norm_sq = 0.0f;
        for (uint32_t i = 0; i < kOsnetFeatureDim; i++) {
            float val = (static_cast<float>(raw[i]) - out.zero_point) * out.scale;
            det.features[i] = val;
            norm_sq += val * val;
        }
        // L2 normalise
        if (norm_sq > 1e-6f) {
            float inv_norm = 1.0f / std::sqrt(norm_sq);
            for (uint32_t i = 0; i < kOsnetFeatureDim; i++) {
                det.features[i] *= inv_norm;
            }
        }
    }
#else
    (void)detections; (void)yuv_data; (void)yuv_w; (void)yuv_h; (void)yuv_stride;
#endif
}

} // namespace ai
} // namespace ipcam
