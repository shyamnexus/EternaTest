/**
 * @file analytics.h
 * @brief AI Analytics Engine for Video Analysis
 * 
 * This module provides real-time video analytics using Novatek AI3 engine:
 * - Person/Vehicle Detection (PVDCNN)
 * - Face Detection (FDCNN)
 * - Motion Detection (MD)
 * - Line Crossing Detection
 * - Object Tracking
 * 
 * The analytics engine pulls YUV frames from VideoProc OUT_3 (640x360)
 * and processes them through AI models without requiring a video encoder.
 * Detection results are overlaid on the main stream via SMART_BBOX.
 */

#ifndef IPCAM_ANALYTICS_H
#define IPCAM_ANALYTICS_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>

#include "ipcam/face_recognition.h"  // for FaceRecognitionConfig, FaceMatch
#include "ipcam/lpr.h"               // for LprConfig, PlateDetection
#include "ipcam/heat_map.h"           // for HeatMapConfig
#include "ipcam/pose_estimation.h"    // for PoseConfig, PoseResult
#include "ipcam/face_attribute.h"     // for FaceAttributeConfig, FaceAttributes
#include "ipcam/audio_classification.h" // for AudioClassificationConfig, AudioEvent

// Forward declarations for new AI modules
namespace ipcam {
namespace streaming {
    class AudioFrameConsumer;
}
namespace ai {
    class MotionDetectionEngine;
    class ObjectTracker;
    class PrivacyMosaicEngine;
    class AiispEngine;
    class TamperDetectionEngine;
    class LineCrossingEngine;
    class ZoneIntrusionEngine;
    class CountingEngine;
    class YoloPostProc;
    class ScrfdPostProc;
    class FaceRecognitionEngine;
    class LprEngine;
    class HeatMapEngine;
    class PoseEstimationEngine;
    class FaceAttributeEngine;
    class AudioClassificationEngine;
    
    // Forward declare tamper types
    enum class TamperType : uint32_t;
    struct TamperResult;
    struct Detection;  // from object_tracking.h
    struct FaceDetection;  // from scrfd_postproc.h
}
}

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_type.h"
}
#endif

namespace ipcam {
namespace ai {

// ============================================================================
// Detection Types
// ============================================================================

/**
 * @brief Object categories detected by AI
 */
enum class ObjectCategory {
    kUnknown = 0,
    kPerson = 1,
    kFace = 2,
    kVehicle = 4,
    kNonMotor = 5,  // Bicycle, motorcycle
    kMotion = 10,   // Generic motion
    kLineCross = 11
};

/**
 * @brief Bounding box for detected object
 */
struct BoundingBox {
    float x1 = 0.0f;  ///< Top-left X (0.0 - 1.0 normalized)
    float y1 = 0.0f;  ///< Top-left Y (0.0 - 1.0 normalized)
    float x2 = 0.0f;  ///< Bottom-right X (0.0 - 1.0 normalized)
    float y2 = 0.0f;  ///< Bottom-right Y (0.0 - 1.0 normalized)
    
    float Width() const { return x2 - x1; }
    float Height() const { return y2 - y1; }
    float CenterX() const { return (x1 + x2) / 2.0f; }
    float CenterY() const { return (y1 + y2) / 2.0f; }
};

/**
 * @brief Detection result for a single object
 */
struct DetectionResult {
    uint32_t id = 0;                    ///< Tracking ID (0 if not tracked)
    ObjectCategory category = ObjectCategory::kUnknown;
    BoundingBox bbox;                   ///< Bounding box (normalized 0.0-1.0)
    float confidence = 0.0f;            ///< Detection confidence (0.0-1.0)
    uint64_t timestamp = 0;             ///< Frame timestamp
    
    // Pixel coordinates (for specific resolution)
    int pixel_x1 = 0;
    int pixel_y1 = 0;
    int pixel_x2 = 0;
    int pixel_y2 = 0;
};

/**
 * @brief Line crossing zone definition
 */
struct LineCrossZone {
    uint32_t id = 0;
    std::string name;
    float x1 = 0.0f, y1 = 0.0f;  ///< Line start point (normalized)
    float x2 = 0.0f, y2 = 0.0f;  ///< Line end point (normalized)
    bool bidirectional = false;   ///< Trigger on both directions
    bool enabled = true;
};

/**
 * @brief Motion detection zone
 */
struct MotionZone {
    uint32_t id = 0;
    std::string name;
    float x = 0.0f, y = 0.0f;     ///< Top-left (normalized)
    float width = 1.0f, height = 1.0f;  ///< Size (normalized)
    int sensitivity = 50;          ///< 0-100
    bool enabled = true;
    
    // Loitering detection
    bool loitering_enabled = false;
    int loitering_threshold_ms = 10000;  ///< Dwell time to trigger loitering (default 10s)
};

/**
 * @brief Face detection with recognition/attribute results
 */
struct FaceResult {
    BoundingBox bbox;               ///< Face bounding box (normalised)
    float confidence = 0.0f;
    FaceMatch recognition;           ///< Gallery match result
    FaceAttributes attributes;       ///< Gender/age
    bool has_recognition = false;
    bool has_attributes = false;
};

/**
 * @brief Frame of detection results
 */
struct DetectionFrame {
    uint64_t frame_number = 0;
    uint64_t timestamp = 0;
    int source_width = 0;
    int source_height = 0;
    std::vector<DetectionResult> detections;
    bool motion_detected = false;
    std::vector<uint32_t> lines_crossed;  ///< IDs of crossed lines

    // Extended results from new AI modules
    std::vector<FaceResult> faces;           ///< Face detection + recognition + attributes
    std::vector<PlateDetection> plates;      ///< License plate detections with OCR text
    std::vector<PoseResult> poses;           ///< Pose estimation results
    std::vector<AudioEvent> audio_events;    ///< Audio classification results
};

// ============================================================================
// Analytics Configuration
// ============================================================================

/**
 * @brief Engine configuration (AI backend)
 */
struct EngineConfig {
    std::string type = "vendor_ai3";
    std::string model_path = "/mnt/app/models";
    bool dla_enabled = true;
    bool cpu_fallback = true;
};

/**
 * @brief Input configuration
 */
struct InputConfig {
    int width = 640;
    int height = 360;
    std::string format = "yuv420";
    int fps = 15;
};

/**
 * @brief Output/overlay configuration
 */
struct OutputConfig {
    bool overlay_enabled = true;
    bool smart_bbox = true;
    int max_objects = 32;
};

/**
 * @brief Object class detection settings
 */
struct ObjectClassConfig {
    bool enabled = true;
    float min_confidence = 0.5f;
    std::string color = "#FF0000";
};

/**
 * @brief Object tracking configuration (simplified for ObjectDetectionConfig)
 * Note: Full TrackingConfig is defined in object_tracking.h
 */
struct SimpleTrackingConfig {
    bool enabled = true;
    int max_age = 5;
    int min_hits = 1;
    float iou_threshold = 0.3f;
};

/**
 * @brief Object detection configuration
 */
struct ObjectDetectionConfig {
    bool enabled = false;  ///< Default OFF - requires AI models deployed to /mnt/app/ai_models
    std::string model = "yolo26n";
    float confidence_threshold = 0.35f;
    float nms_threshold = 0.45f;
    ObjectClassConfig person{true, 0.35f, "#FF0000"};
    ObjectClassConfig vehicle{true, 0.35f, "#00FF00"};
    ObjectClassConfig animal{false, 0.35f, "#0000FF"};
    SimpleTrackingConfig tracking;
};

/**
 * @brief Motion detection configuration
 */
struct MotionDetectionConfig {
    bool enabled = false;
    int sensitivity = 50;
    int threshold = 30;
    int min_area = 500;
    int cooldown_ms = 2000;
    std::vector<MotionZone> zones;
};

/**
 * @brief Tamper detection configuration (for AnalyticsConfig)
 * Note: Full TamperConfig is defined in tamper_detection.h
 */
struct TamperDetectionConfig {
    bool enabled = false;
    int sensitivity = 50;  ///< Overall sensitivity (0-100)
    bool defocus_enabled = true;
    bool masking_enabled = true;
    bool scene_change_enabled = true;
    bool exposure_enabled = true;
};

/**
 * @brief Line crossing line definition
 */
struct LineCrossLine {
    uint32_t id = 0;
    std::string name;
    bool enabled = false;
    float x1 = 0.0f, y1 = 50.0f;
    float x2 = 100.0f, y2 = 50.0f;
    std::string direction = "both";
    std::string color = "#FFFF00";
};

/**
 * @brief Line crossing counting configuration
 */
struct LineCrossCountingConfig {
    bool enabled = false;
    int reset_interval = 86400;
};

/**
 * @brief Line crossing configuration
 */
struct LineCrossingConfig {
    bool enabled = false;
    std::vector<LineCrossLine> lines;
    LineCrossCountingConfig counting;
};

/**
 * @brief Zone intrusion point
 */
struct ZonePoint {
    int x = 0;
    int y = 0;
};

/**
 * @brief Zone intrusion zone definition
 */
struct IntrusionZone {
    uint32_t id = 0;
    std::string name;
    bool enabled = false;
    std::vector<ZonePoint> points;
    int dwell_time_ms = 3000;
    std::vector<std::string> alarm_classes;
};

/**
 * @brief Zone intrusion configuration
 */
struct ZoneIntrusionConfig {
    bool enabled = false;
    std::vector<IntrusionZone> zones;
};

/**
 * @brief Face detection configuration
 */
struct FaceDetectionConfig {
    bool enabled = false;
    std::string model = "scrfd_nosig";
    int min_face_size = 30;
    float confidence_threshold = 0.6f;
    bool landmarks = false;
    FaceRecognitionConfig recognition;
};

/**
 * @brief License plate recognition configuration
 */
struct LicensePlateConfig {
    bool enabled = false;
    std::string model = "lpr_nvt";
    std::vector<std::string> regions{"us", "eu"};
    float min_confidence = 0.7f;
};

/**
 * @brief Event action configuration
 */
struct EventConfig {
    bool enabled = true;
    std::string action = "notify";
    bool snapshot = false;
    int record_duration = 0;
};

/**
 * @brief Events configuration
 */
struct EventsConfig {
    EventConfig motion_start{true, "notify", true, 30};
    EventConfig motion_end{true, "notify", false, 0};
    EventConfig person_detected{true, "notify", true, 0};
    EventConfig vehicle_detected{true, "notify", true, 0};
    EventConfig line_crossed{false, "notify", false, 0};
    EventConfig zone_intrusion{false, "alarm", false, 0};
};

/**
 * @brief Performance tuning configuration
 */
struct PerformanceConfig {
    int skip_frames = 0;
    int batch_size = 1;
    bool async_inference = true;
    int gpu_memory_mb = 256;
    int thread_count = 2;
};

/**
 * @brief Debug configuration
 */
struct DebugConfig {
    bool log_detections = false;
    bool save_frames = false;
    std::string frame_save_path = "/tmp/ai_debug";
    bool show_inference_time = false;
};

/**
 * @brief Analytics engine configuration
 */
struct AnalyticsConfig {
    bool enabled = false;  ///< Default OFF - enable via analytics.json config
    
    // Core engine settings
    EngineConfig engine;
    InputConfig input;
    OutputConfig output;
    
    // Detection modules
    MotionDetectionConfig motion_detection;
    ObjectDetectionConfig object_detection;
    LineCrossingConfig line_crossing;
    ZoneIntrusionConfig zone_intrusion;
    FaceDetectionConfig face_detection;
    LicensePlateConfig license_plate;
    TamperDetectionConfig tamper_detection;
    
    // New AI analytics modules
    HeatMapConfig heat_map;
    PoseConfig pose_estimation;
    FaceAttributeConfig face_attribute;
    AudioClassificationConfig audio_classification;
    
    // Events and triggers
    EventsConfig events;
    
    // Performance and debug
    PerformanceConfig performance;
    DebugConfig debug;
    
    // Legacy compatibility (kept for HDAL integration)
    int videoproc_out_id = 3;          ///< VideoProc output to use
    int bbox_encoder_id = 0;           ///< Encoder for SMART_BBOX overlay
};

// ============================================================================
// Event Callbacks
// ============================================================================

/**
 * @brief Callback for detection events
 */
using DetectionCallback = std::function<void(const DetectionFrame&)>;

/**
 * @brief Callback for motion detection
 */
using MotionCallback = std::function<void(uint32_t zone_id, bool motion_start)>;

/**
 * @brief Callback for line crossing
 */
using LineCrossCallback = std::function<void(uint32_t line_id, uint32_t object_id, ObjectCategory category)>;

/**
 * @brief Callback for loitering detection
 * @param zone_id Zone where loitering was detected
 * @param object_id ID of the loitering object
 * @param dwell_time_ms Time the object has been in the zone
 */
using LoiteringCallback = std::function<void(uint32_t zone_id, uint32_t object_id, uint64_t dwell_time_ms)>;

/**
 * @brief Callback for tamper detection (defocus, masking, scene change, brightness)
 * @param type Bitmask of detected tamper types (TamperType enum)
 * @param active True if alarm started, false if cleared
 * @param result Detailed tamper detection results with metrics
 */
using TamperCallback = std::function<void(TamperType type, bool active, const TamperResult& result)>;

/**
 * @brief Callback for face recognition events
 */
using FaceRecognitionCallback = std::function<void(const FaceResult& face)>;

/**
 * @brief Callback for license plate detection
 */
using LprCallback = std::function<void(const PlateDetection& plate)>;

/**
 * @brief Callback for audio classification events
 */
using AudioEventCallback = std::function<void(const AudioEvent& event)>;

// ============================================================================
// Analytics Engine Class
// ============================================================================

/**
 * @brief AI Analytics Engine
 * 
 * Usage:
 * @code
 * auto& engine = AnalyticsEngine::Instance();
 * engine.LoadConfig();
 * engine.SetDetectionCallback([](const DetectionFrame& frame) {
 *     for (const auto& det : frame.detections) {
 *         // Handle detection
 *     }
 * });
 * engine.Init();
 * engine.Start();
 * @endcode
 */
class AnalyticsEngine {
public:
    static AnalyticsEngine& Instance();
    
    // Prevent copying
    AnalyticsEngine(const AnalyticsEngine&) = delete;
    AnalyticsEngine& operator=(const AnalyticsEngine&) = delete;
    
    // ========================================================================
    // Lifecycle
    // ========================================================================
    
    /**
     * @brief Load configuration from config system
     * @return true if config loaded successfully
     */
    bool LoadConfig();
    
    /**
     * @brief Initialize AI models and HDAL paths
     * @return true if initialized successfully
     */
    bool Init();
    
    /**
     * @brief Start analytics processing thread
     * @return true if started successfully
     */
    bool Start();
    
    /**
     * @brief Stop analytics processing
     */
    void Stop();
    
    /**
     * @brief Shutdown and release resources
     */
    void Shutdown();
    
    /**
     * @brief Check if engine is running
     */
    bool IsRunning() const { return running_.load(); }

    // ========================================================================
    // HDAL Integration
    // ========================================================================

    /**
     * @brief Set VideoProc path for analytics input
     * @param proc_path Path ID from HdalPipeline::GetAnalyticsPath()
     * 
     * Call this after HdalPipeline::Init() to connect the analytics
     * engine to the HDAL video processing output.
     */
    void SetVideoProcPath(uint64_t proc_path);

    /**
     * @brief Set encoder path for SMART_BBOX overlay
     * @param enc_path Path ID of main encoder (stream 0)
     * 
     * Call this to enable detection result overlay on encoded video.
     */
    void SetEncoderPath(uint64_t enc_path);
    
    /**
     * @brief Check if HDAL paths are configured
     */
    bool HasHdalPaths() const;

    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * @brief Get current configuration
     */
    AnalyticsConfig GetConfig() const;
    
    /**
     * @brief Update configuration
     * @param config New configuration
     * @return true if applied successfully
     */
    bool SetConfig(const AnalyticsConfig& config);
    
    /**
     * @brief Enable/disable specific detection
     */
    bool EnablePersonDetection(bool enable);
    bool EnableVehicleDetection(bool enable);
    bool EnableFaceDetection(bool enable);
    bool EnableMotionDetection(bool enable);
    bool EnableLineCrossing(bool enable);
    
    /**
     * @brief Check if AI3 neural network engine is initialized
     */
    bool IsAi3Initialized() const { return ai3_initialized_; }
    
    /**
     * @brief Initialize AI3 models at runtime (called when smart detection is enabled via API)
     */
    bool InitAiModels();
    
    // ========================================================================
    // Zone Management
    // ========================================================================
    
    /**
     * @brief Add motion detection zone
     */
    bool AddMotionZone(const MotionZone& zone);
    bool RemoveMotionZone(uint32_t zone_id);
    bool UpdateMotionZone(const MotionZone& zone);
    std::vector<MotionZone> GetMotionZones() const;
    
    /**
     * @brief Add line crossing zone
     */
    bool AddLineCrossZone(const LineCrossZone& zone);
    bool RemoveLineCrossZone(uint32_t zone_id);
    bool UpdateLineCrossZone(const LineCrossZone& zone);
    std::vector<LineCrossZone> GetLineCrossZones() const;
    
    // ========================================================================
    // Callbacks
    // ========================================================================
    
    /**
     * @brief Set detection callback (called for each processed frame)
     */
    void SetDetectionCallback(DetectionCallback callback);
    
    /**
     * @brief Set motion detection callback
     */
    void SetMotionCallback(MotionCallback callback);
    
    /**
     * @brief Set line crossing callback
     */
    void SetLineCrossCallback(LineCrossCallback callback);
    
    /**
     * @brief Set loitering detection callback
     */
    void SetLoiteringCallback(LoiteringCallback callback);
    
    /**
     * @brief Set tamper detection callback (defocus, masking, scene change, brightness)
     */
    void SetTamperCallback(TamperCallback callback);
    void SetFaceRecognitionCallback(FaceRecognitionCallback callback);
    void SetLprCallback(LprCallback callback);
    void SetAudioEventCallback(AudioEventCallback callback);
    
    /// Get current heat map grid (normalised 0-1, grid_w * grid_h floats)
    std::vector<float> GetHeatMapGrid() const;
    int GetHeatMapGridW() const;
    int GetHeatMapGridH() const;
    void ResetHeatMap();
    
    // ========================================================================
    // Statistics
    // ========================================================================
    
    struct Stats {
        uint64_t frames_processed = 0;
        uint64_t total_detections = 0;
        uint64_t motion_events = 0;
        uint64_t line_cross_events = 0;
        uint64_t loitering_events = 0;
        uint64_t tamper_events = 0;           ///< Tamper alarm events
        uint64_t scene_change_events = 0;     ///< Scene change events
        float avg_process_time_ms = 0.0f;
        float current_fps = 0.0f;
    };
    
    Stats GetStats() const;
    void ResetStats();
    
    // ========================================================================
    // Manual Frame Processing (for testing)
    // ========================================================================
    
    /**
     * @brief Process a single frame manually (for testing)
     * @param yuv_data YUV420 frame data
     * @param width Frame width
     * @param height Frame height
     * @return Detection results
     */
    DetectionFrame ProcessFrame(const uint8_t* yuv_data, int width, int height);
    
private:
    AnalyticsEngine();
    ~AnalyticsEngine();
    
    // Processing thread
    void ProcessingThread();
    
    // Internal methods
    bool InitHdalPath();
    void ProcessDetections(DetectionFrame& frame, const uint8_t* yuv_data, uintptr_t frame_pa);
    void CheckLineCrossing(const DetectionFrame& frame);
    void UpdateSmartBbox(const DetectionFrame& frame);
    
    // State
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::thread processing_thread_;
    mutable std::mutex mutex_;
    
    // Configuration
    AnalyticsConfig config_;
    
    // Callbacks
    DetectionCallback detection_callback_;
    MotionCallback motion_callback_;
    LineCrossCallback linecross_callback_;
    LoiteringCallback loitering_callback_;
    TamperCallback tamper_callback_;
    FaceRecognitionCallback face_recognition_callback_;
    LprCallback lpr_callback_;
    AudioEventCallback audio_event_callback_;
    
    // Statistics
    Stats stats_;
    
#if HDAL_PIPELINE_ENABLED
    // HDAL handles
    HD_PATH_ID proc_alg_path_ = 0;
    HD_PATH_ID enc_path_ = 0;  // For SMART_BBOX overlay
#endif
    
    // AI3 engine state
    bool ai3_initialized_ = false;
    uint32_t ai3_proc_id_ = 0;  // AI3 network process ID
    
    // Previous frame for motion detection
    std::vector<uint8_t> prev_frame_;
    std::vector<bool> motion_zone_active_;
    bool motion_active_ = false;  // Global motion state for transition logging
    
    // Object tracking state
    struct TrackedObject {
        uint32_t id;
        ObjectCategory category;
        BoundingBox bbox;
        int frames_since_seen;
    };
    std::vector<TrackedObject> tracked_objects_;
    uint32_t next_track_id_ = 1;
    
    // New AI module engines (singleton references)
    // These are initialized in InitAiModels() and used in ProcessingThread()
    // TamperDetectionEngine - IVE-accelerated tamper detection (defocus, masking, scene change)
    // MotionDetectionEngine - Hardware-accelerated motion detection with zones
    // ObjectTracker - Multi-object IOU-based tracking with Kalman filter
    // PrivacyMosaicEngine - Hardware-accelerated face/region blurring
    // AiispEngine - AI neural network ISP enhancement
    
    // Smart analytics consumer modules
    std::unique_ptr<class LineCrossingEngine> line_crossing_engine_;
    std::unique_ptr<class ZoneIntrusionEngine> zone_intrusion_engine_;
    std::unique_ptr<class CountingEngine> counting_engine_;
    std::unique_ptr<class FaceRecognitionEngine> face_recognition_engine_;
    std::unique_ptr<class LprEngine> lpr_engine_;
    std::unique_ptr<class HeatMapEngine> heat_map_engine_;
    std::unique_ptr<class PoseEstimationEngine> pose_estimation_engine_;
    std::unique_ptr<class FaceAttributeEngine> face_attribute_engine_;
    std::unique_ptr<class AudioClassificationEngine> audio_classification_engine_;

    // NPU inference + postproc
    std::unique_ptr<class YoloPostProc> yolo_postproc_;
    std::unique_ptr<class ScrfdPostProc> scrfd_postproc_;
    int yolo_net_id_ = -1;      // YOLO object detection
    int scrfd_net_id_ = -1;     // SCRFD face detection
    int osnet_net_id_ = -1;     // OSNet Re-ID
    int mobilefacenet_id_ = -1; // MobileFaceNet face embeddings
    int face_attr_net_id_ = -1; // Face attribute (gender/age)
    int lp_det_net_id_ = -1;    // License plate detector (YOLO11n)
    int lp_ocr_net_id_ = -1;    // LPRNet OCR
    int pose_net_id_ = -1;      // YOLOv26-Pose
    int yamnet_net_id_ = -1;    // YAMNet audio classification

    // Shared NV12 crop buffer (sequential use: Re-ID, face recog, face attr, LPR)
    uintptr_t ai_crop_pa_ = 0;
    uintptr_t ai_crop_va_ = 0;
    uint32_t  ai_crop_size_ = 0;
    static constexpr uint32_t kOsnetInputW = 128;
    static constexpr uint32_t kOsnetInputH = 256;
    static constexpr uint32_t kOsnetFeatureDim = 512;

    /// Extract OSNet Re-ID features for person detections (crops bbox -> NPU infer -> 512-d embedding)
    void ExtractReidFeatures(std::vector<Detection>& detections,
                             const uint8_t* yuv_data, uint32_t yuv_w,
                             uint32_t yuv_h, uint32_t yuv_stride);
    
    // Dedicated MD processing thread (uses separate 160x120 path for libmd)
    void MdProcessingThread();
    std::thread md_processing_thread_;
    std::atomic<bool> md_thread_running_{false};
    
    // Audio classification processing thread
    void AudioProcessingThread();
    std::thread audio_processing_thread_;
    std::atomic<bool> audio_thread_running_{false};
    std::unique_ptr<streaming::AudioFrameConsumer> audio_consumer_;
    
#if HDAL_PIPELINE_ENABLED
    HD_PATH_ID md_proc_path_ = 0;  // Dedicated MD VideoProc path (160x120)
    int md_width_ = 160;
    int md_height_ = 120;
#endif
};

// ============================================================================
// Helper Functions
// ============================================================================

std::string ObjectCategoryToString(ObjectCategory category);
ObjectCategory StringToObjectCategory(const std::string& str);

} // namespace ai
} // namespace ipcam

#endif // IPCAM_ANALYTICS_H
