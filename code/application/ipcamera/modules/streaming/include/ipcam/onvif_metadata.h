/**
 * @file onvif_metadata.h
 * @brief ONVIF Metadata Streaming Support for RTSP
 * 
 * Provides ONVIF-compliant metadata streaming via RTP for:
 * - Video analytics (object detection, tracking)
 * - Events (motion detection, line crossing)
 * - Synchronized with video stream timestamps
 * 
 * Reference: ONVIF Streaming Specification
 * https://www.onvif.org/specs/stream/ONVIF-Streaming-Spec.pdf
 * 
 * Key features:
 * - RTP payload type 107 with vnd.onvif.metadata/90000
 * - XML-based tt:MetadataStream format
 * - Timestamp synchronization with video
 * - Thread-safe metadata injection from AI module
 * 
 * Copyright (c) 2026
 */

#ifndef IPCAM_ONVIF_METADATA_H
#define IPCAM_ONVIF_METADATA_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <cstdint>
#include <chrono>
#include <functional>
#include <array>

namespace ipcam {
namespace streaming {

// ============================================================================
// ONVIF Metadata Constants
// ============================================================================

constexpr int kOnvifMetadataPayloadType = 107;  // Dynamic RTP payload type
constexpr int kOnvifMetadataClockRate = 90000;  // 90kHz clock rate
constexpr int kMaxMetadataXmlSize = 8192;       // Max XML size per frame
constexpr int kMetadataQueueSize = 32;          // Ring buffer size

// ============================================================================
// ONVIF Bounding Box (normalized -1.0 to 1.0 per ONVIF spec)
// ============================================================================

/**
 * @brief ONVIF-compliant bounding box coordinates
 * 
 * ONVIF uses normalized coordinates from -1.0 to 1.0:
 * - Center of image is (0, 0)
 * - Top-left is (-1, 1), Bottom-right is (1, -1)
 * - left < right, bottom < top (Y-axis inverted from typical)
 */
struct OnvifBoundingBox {
    float left = 0.0f;      // X left edge (-1.0 to 1.0)
    float top = 0.0f;       // Y top edge (-1.0 to 1.0)
    float right = 0.0f;     // X right edge (-1.0 to 1.0)
    float bottom = 0.0f;    // Y bottom edge (-1.0 to 1.0)
    
    /**
     * @brief Create from pixel coordinates
     * @param px_left Left pixel X
     * @param px_top Top pixel Y  
     * @param px_right Right pixel X
     * @param px_bottom Bottom pixel Y
     * @param img_width Image width in pixels
     * @param img_height Image height in pixels
     * @return ONVIF normalized bounding box
     */
    static OnvifBoundingBox FromPixels(int px_left, int px_top, int px_right, int px_bottom,
                                        int img_width, int img_height);
    
    /**
     * @brief Create from normalized 0-1 coordinates (common AI output)
     * @param norm_x Normalized X (0-1, left to right)
     * @param norm_y Normalized Y (0-1, top to bottom)
     * @param norm_w Normalized width (0-1)
     * @param norm_h Normalized height (0-1)
     * @return ONVIF normalized bounding box
     */
    static OnvifBoundingBox FromNormalized(float norm_x, float norm_y, float norm_w, float norm_h);
};

// ============================================================================
// ONVIF Object Detection
// ============================================================================

/**
 * @brief Object class type (ONVIF VideoAnalytics)
 */
enum class OnvifObjectClass {
    kUnknown = 0,
    kHuman,         // Person
    kVehicle,       // Car, truck, etc.
    kFace,          // Face detection
    kAnimal,        // Dog, cat, etc.
    kLicensePlate,  // License plate
    kOther          // Other/unknown type
};

/**
 * @brief Single detected object for ONVIF metadata
 */
struct OnvifDetectedObject {
    uint64_t object_id = 0;             // Unique tracking ID
    OnvifObjectClass object_class = OnvifObjectClass::kUnknown;
    float confidence = 0.0f;            // Detection confidence (0.0-1.0)
    OnvifBoundingBox bbox;              // Bounding box
    
    // Optional attributes
    std::string class_name;             // Custom class name (if not enum)
    std::string license_plate_text;     // For license plate detection
};

// ============================================================================
// ONVIF Events
// ============================================================================

/**
 * @brief ONVIF event types
 */
enum class OnvifEventType {
    kMotionDetected,        // tns1:RuleEngine/CellMotionDetector/Motion
    kLineCrossing,          // tns1:RuleEngine/LineDetector/Crossed
    kFieldDetection,        // tns1:RuleEngine/FieldDetector/ObjectsInside
    kTamperDetection,       // tns1:VideoSource/ImageTooDark/AnalyticsService
    kAudioDetection,        // tns1:AudioAnalytics/Audio/DetectedSound
    kFaceRecognized,        // tns1:RuleEngine/FaceRecognition/Recognized
    kLicensePlateRecognized // tns1:RuleEngine/LicensePlateRecognition/Recognized
};

/**
 * @brief ONVIF event data
 */
struct OnvifEvent {
    OnvifEventType event_type = OnvifEventType::kMotionDetected;
    bool is_active = true;              // true = started, false = ended
    std::string source_token;           // VideoSourceToken
    std::string rule_name;              // Rule/detector name
    std::chrono::system_clock::time_point timestamp;
    
    // Event-specific data
    std::string data_value;             // Generic data field
    int region_id = 0;                  // For region-based events
};

// ============================================================================
// ONVIF Metadata Frame
// ============================================================================

/**
 * @brief Complete ONVIF metadata frame
 * 
 * Represents one frame of metadata synchronized with video.
 * Contains detected objects and/or events.
 */
struct OnvifMetadataFrame {
    std::chrono::system_clock::time_point utc_time;  // Frame timestamp
    uint64_t video_timestamp = 0;                     // Video RTP timestamp (90kHz)
    
    // Video analytics data
    std::vector<OnvifDetectedObject> objects;
    
    // Event data
    std::vector<OnvifEvent> events;
    
    /**
     * @brief Generate ONVIF-compliant XML for this frame
     * @return XML string conforming to tt:MetadataStream schema
     */
    std::string ToXml() const;
};

// ============================================================================
// ONVIF Metadata Generator
// ============================================================================

/**
 * @brief Generates ONVIF-compliant metadata XML
 * 
 * Thread-safe singleton that collects analytics data and generates
 * ONVIF XML frames synchronized with video.
 */
class OnvifMetadataGenerator {
public:
    static OnvifMetadataGenerator& Instance();
    
    // ========================================================================
    // Object Detection
    // ========================================================================
    
    /**
     * @brief Report detected objects for current frame
     * @param objects Vector of detected objects
     * @param video_timestamp Video RTP timestamp for synchronization
     */
    void ReportObjects(const std::vector<OnvifDetectedObject>& objects,
                       uint64_t video_timestamp);
    
    /**
     * @brief Report single detected object
     * @param object Detected object
     * @param video_timestamp Video RTP timestamp for synchronization
     */
    void ReportObject(const OnvifDetectedObject& object, uint64_t video_timestamp);
    
    // ========================================================================
    // Events
    // ========================================================================
    
    /**
     * @brief Report an event
     * @param event Event data
     */
    void ReportEvent(const OnvifEvent& event);
    
    /**
     * @brief Report motion detection
     * @param is_motion true if motion detected, false if motion stopped
     * @param source_token Video source token (e.g., "VideoSource_1")
     */
    void ReportMotion(bool is_motion, const std::string& source_token = "VideoSource_1");
    
    // ========================================================================
    // Frame Generation
    // ========================================================================
    
    /**
     * @brief Get next metadata frame if available
     * @param[out] frame Output frame
     * @return true if frame available, false if queue empty
     */
    bool GetNextFrame(OnvifMetadataFrame& frame);
    
    /**
     * @brief Check if metadata is available
     */
    bool HasPendingMetadata() const;
    
    /**
     * @brief Clear all pending metadata
     */
    void Clear();
    
    /**
     * @brief Set metadata enabled state
     */
    void SetEnabled(bool enabled) { enabled_ = enabled; }
    bool IsEnabled() const { return enabled_; }
    
private:
    OnvifMetadataGenerator();
    ~OnvifMetadataGenerator() = default;
    
    OnvifMetadataGenerator(const OnvifMetadataGenerator&) = delete;
    OnvifMetadataGenerator& operator=(const OnvifMetadataGenerator&) = delete;
    
    // Pending frame being built
    OnvifMetadataFrame current_frame_;
    uint64_t current_video_ts_ = 0;
    
    // Output queue
    std::queue<OnvifMetadataFrame> frame_queue_;
    mutable std::mutex queue_mutex_;
    
    bool enabled_ = true;
    
    // Flush current frame to queue when timestamp changes
    void FlushCurrentFrame();
};

// ============================================================================
// XML Generation Helpers
// ============================================================================

namespace xml {

/**
 * @brief Generate ISO8601 timestamp string
 * @param tp Time point
 * @return ISO8601 string (e.g., "2026-01-11T17:31:00Z")
 */
std::string FormatUtcTime(const std::chrono::system_clock::time_point& tp);

/**
 * @brief Generate ONVIF VideoAnalytics Frame XML
 * @param objects Detected objects
 * @param utc_time Frame timestamp
 * @return XML fragment
 */
std::string GenerateVideoAnalyticsXml(const std::vector<OnvifDetectedObject>& objects,
                                       const std::string& utc_time);

/**
 * @brief Generate ONVIF Event XML
 * @param events Events
 * @return XML fragment
 */
std::string GenerateEventXml(const std::vector<OnvifEvent>& events);

/**
 * @brief Get ONVIF topic for event type
 * @param event_type Event type
 * @return ONVIF topic string
 */
std::string GetEventTopic(OnvifEventType event_type);

/**
 * @brief Get object class string
 * @param obj_class Object class enum
 * @return ONVIF class string (e.g., "Human", "Vehicle")
 */
std::string GetObjectClassName(OnvifObjectClass obj_class);

} // namespace xml

} // namespace streaming
} // namespace ipcam

#endif // IPCAM_ONVIF_METADATA_H
