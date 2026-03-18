/**
 * @file onvif_metadata.cpp
 * @brief ONVIF Metadata Streaming Implementation
 * 
 * Implements ONVIF-compliant metadata XML generation and
 * thread-safe metadata frame management.
 * 
 * Copyright (c) 2026
 */

#include "ipcam/onvif_metadata.h"
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace ipcam {
namespace streaming {

// ============================================================================
// OnvifBoundingBox Implementation
// ============================================================================

OnvifBoundingBox OnvifBoundingBox::FromPixels(int px_left, int px_top, int px_right, int px_bottom,
                                               int img_width, int img_height) {
    OnvifBoundingBox box;
    
    // Convert from pixel (0 to width/height) to ONVIF (-1 to 1)
    // X: 0 -> -1, width -> 1
    // Y: 0 -> 1, height -> -1 (Y is inverted in ONVIF)
    
    float w = static_cast<float>(img_width);
    float h = static_cast<float>(img_height);
    
    box.left = (2.0f * px_left / w) - 1.0f;
    box.right = (2.0f * px_right / w) - 1.0f;
    box.top = 1.0f - (2.0f * px_top / h);
    box.bottom = 1.0f - (2.0f * px_bottom / h);
    
    return box;
}

OnvifBoundingBox OnvifBoundingBox::FromNormalized(float norm_x, float norm_y, float norm_w, float norm_h) {
    OnvifBoundingBox box;
    
    // Convert from normalized (0-1) center-based to ONVIF (-1 to 1)
    // Assume norm_x, norm_y are center coordinates
    float half_w = norm_w / 2.0f;
    float half_h = norm_h / 2.0f;
    
    box.left = (2.0f * (norm_x - half_w)) - 1.0f;
    box.right = (2.0f * (norm_x + half_w)) - 1.0f;
    box.top = 1.0f - (2.0f * (norm_y - half_h));    // Y inverted
    box.bottom = 1.0f - (2.0f * (norm_y + half_h));  // Y inverted
    
    return box;
}

// ============================================================================
// XML Namespace Helpers
// ============================================================================

namespace xml {

std::string FormatUtcTime(const std::chrono::system_clock::time_point& tp) {
    auto time_t_val = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()) % 1000;
    
    std::tm tm_val;
    gmtime_r(&time_t_val, &tm_val);
    
    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    
    return oss.str();
}

std::string GetEventTopic(OnvifEventType event_type) {
    switch (event_type) {
        case OnvifEventType::kMotionDetected:
            return "tns1:RuleEngine/CellMotionDetector/Motion";
        case OnvifEventType::kLineCrossing:
            return "tns1:RuleEngine/LineDetector/Crossed";
        case OnvifEventType::kFieldDetection:
            return "tns1:RuleEngine/FieldDetector/ObjectsInside";
        case OnvifEventType::kTamperDetection:
            return "tns1:VideoSource/ImageTooDark/AnalyticsService";
        case OnvifEventType::kAudioDetection:
            return "tns1:AudioAnalytics/Audio/DetectedSound";
        case OnvifEventType::kFaceRecognized:
            return "tns1:RuleEngine/FaceRecognition/Recognized";
        case OnvifEventType::kLicensePlateRecognized:
            return "tns1:RuleEngine/LicensePlateRecognition/Recognized";
        default:
            return "tns1:RuleEngine/CellMotionDetector/Motion";
    }
}

std::string GetObjectClassName(OnvifObjectClass obj_class) {
    switch (obj_class) {
        case OnvifObjectClass::kHuman:
            return "Human";
        case OnvifObjectClass::kVehicle:
            return "Vehicle";
        case OnvifObjectClass::kFace:
            return "Face";
        case OnvifObjectClass::kAnimal:
            return "Animal";
        case OnvifObjectClass::kLicensePlate:
            return "LicensePlate";
        default:
            return "Object";
    }
}

std::string GenerateVideoAnalyticsXml(const std::vector<OnvifDetectedObject>& objects,
                                       const std::string& utc_time) {
    if (objects.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << "  <tt:VideoAnalytics>\n";
    oss << "    <tt:Frame UtcTime=\"" << utc_time << "\">\n";
    
    for (const auto& obj : objects) {
        oss << "      <tt:Object ObjectId=\"" << obj.object_id << "\">\n";
        oss << "        <tt:Appearance>\n";
        oss << "          <tt:Shape>\n";
        oss << "            <tt:BoundingBox "
            << "left=\"" << std::fixed << std::setprecision(4) << obj.bbox.left << "\" "
            << "top=\"" << obj.bbox.top << "\" "
            << "right=\"" << obj.bbox.right << "\" "
            << "bottom=\"" << obj.bbox.bottom << "\"/>\n";
        oss << "          </tt:Shape>\n";
        
        // Class with likelihood/confidence
        std::string class_name = obj.class_name.empty() 
            ? GetObjectClassName(obj.object_class) 
            : obj.class_name;
        
        oss << "          <tt:Class>\n";
        oss << "            <tt:Type Likelihood=\"" 
            << std::fixed << std::setprecision(2) << obj.confidence << "\">"
            << class_name << "</tt:Type>\n";
        oss << "          </tt:Class>\n";
        oss << "        </tt:Appearance>\n";
        oss << "      </tt:Object>\n";
    }
    
    oss << "    </tt:Frame>\n";
    oss << "  </tt:VideoAnalytics>\n";
    
    return oss.str();
}

std::string GenerateEventXml(const std::vector<OnvifEvent>& events) {
    if (events.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << "  <tt:Event>\n";
    
    for (const auto& evt : events) {
        std::string utc_time = FormatUtcTime(evt.timestamp);
        std::string topic = GetEventTopic(evt.event_type);
        
        oss << "    <wsnt:NotificationMessage>\n";
        oss << "      <wsnt:Topic Dialect=\"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet\">\n";
        oss << "        " << topic << "\n";
        oss << "      </wsnt:Topic>\n";
        oss << "      <wsnt:Message>\n";
        oss << "        <tt:Message UtcTime=\"" << utc_time << "\">\n";
        oss << "          <tt:Source>\n";
        oss << "            <tt:SimpleItem Name=\"VideoSourceToken\" Value=\"" 
            << evt.source_token << "\"/>\n";
        
        if (!evt.rule_name.empty()) {
            oss << "            <tt:SimpleItem Name=\"Rule\" Value=\"" 
                << evt.rule_name << "\"/>\n";
        }
        
        oss << "          </tt:Source>\n";
        oss << "          <tt:Data>\n";
        
        // Event-specific data
        switch (evt.event_type) {
            case OnvifEventType::kMotionDetected:
                oss << "            <tt:SimpleItem Name=\"IsMotion\" Value=\""
                    << (evt.is_active ? "true" : "false") << "\"/>\n";
                break;
            case OnvifEventType::kLineCrossing:
                oss << "            <tt:SimpleItem Name=\"ObjectId\" Value=\"" 
                    << evt.data_value << "\"/>\n";
                break;
            case OnvifEventType::kFieldDetection:
                oss << "            <tt:SimpleItem Name=\"IsInside\" Value=\""
                    << (evt.is_active ? "true" : "false") << "\"/>\n";
                break;
            default:
                if (!evt.data_value.empty()) {
                    oss << "            <tt:SimpleItem Name=\"Value\" Value=\"" 
                        << evt.data_value << "\"/>\n";
                }
                break;
        }
        
        oss << "          </tt:Data>\n";
        oss << "        </tt:Message>\n";
        oss << "      </wsnt:Message>\n";
        oss << "    </wsnt:NotificationMessage>\n";
    }
    
    oss << "  </tt:Event>\n";
    
    return oss.str();
}

} // namespace xml

// ============================================================================
// OnvifMetadataFrame Implementation
// ============================================================================

std::string OnvifMetadataFrame::ToXml() const {
    std::ostringstream oss;
    
    std::string utc_time_str = xml::FormatUtcTime(utc_time);
    
    // XML declaration and root element with namespaces
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    oss << "<tt:MetadataStream xmlns:tt=\"http://www.onvif.org/ver10/schema\"\n";
    oss << "                   xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\">\n";
    
    // Video analytics (detected objects)
    if (!objects.empty()) {
        oss << xml::GenerateVideoAnalyticsXml(objects, utc_time_str);
    }
    
    // Events
    if (!events.empty()) {
        oss << xml::GenerateEventXml(events);
    }
    
    oss << "</tt:MetadataStream>\n";
    
    return oss.str();
}

// ============================================================================
// OnvifMetadataGenerator Implementation
// ============================================================================

OnvifMetadataGenerator& OnvifMetadataGenerator::Instance() {
    static OnvifMetadataGenerator instance;
    return instance;
}

OnvifMetadataGenerator::OnvifMetadataGenerator() {
    spdlog::debug("OnvifMetadataGenerator: Initialized");
}

void OnvifMetadataGenerator::FlushCurrentFrame() {
    if (current_frame_.objects.empty() && current_frame_.events.empty()) {
        return;  // Nothing to flush
    }
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    // Limit queue size
    while (frame_queue_.size() >= kMetadataQueueSize) {
        frame_queue_.pop();
    }
    
    frame_queue_.push(std::move(current_frame_));
    
    // Reset current frame
    current_frame_ = OnvifMetadataFrame();
    current_video_ts_ = 0;
}

void OnvifMetadataGenerator::ReportObjects(const std::vector<OnvifDetectedObject>& objects,
                                            uint64_t video_timestamp) {
    if (!enabled_ || objects.empty()) {
        return;
    }
    
    // If timestamp changed, flush previous frame
    if (current_video_ts_ != 0 && current_video_ts_ != video_timestamp) {
        FlushCurrentFrame();
    }
    
    // Set timestamp and time
    current_video_ts_ = video_timestamp;
    current_frame_.video_timestamp = video_timestamp;
    current_frame_.utc_time = std::chrono::system_clock::now();
    
    // Add objects
    for (const auto& obj : objects) {
        current_frame_.objects.push_back(obj);
    }
}

void OnvifMetadataGenerator::ReportObject(const OnvifDetectedObject& object,
                                           uint64_t video_timestamp) {
    std::vector<OnvifDetectedObject> objects = {object};
    ReportObjects(objects, video_timestamp);
}

void OnvifMetadataGenerator::ReportEvent(const OnvifEvent& event) {
    if (!enabled_) {
        return;
    }
    
    // Events go into current frame or create new one
    if (current_frame_.objects.empty() && current_frame_.events.empty()) {
        current_frame_.utc_time = std::chrono::system_clock::now();
    }
    
    current_frame_.events.push_back(event);
    
    // Flush immediately for events (they're time-critical)
    FlushCurrentFrame();
}

void OnvifMetadataGenerator::ReportMotion(bool is_motion, const std::string& source_token) {
    OnvifEvent evt;
    evt.event_type = OnvifEventType::kMotionDetected;
    evt.is_active = is_motion;
    evt.source_token = source_token;
    evt.timestamp = std::chrono::system_clock::now();
    
    ReportEvent(evt);
}

bool OnvifMetadataGenerator::GetNextFrame(OnvifMetadataFrame& frame) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    if (frame_queue_.empty()) {
        return false;
    }
    
    frame = std::move(frame_queue_.front());
    frame_queue_.pop();
    
    return true;
}

bool OnvifMetadataGenerator::HasPendingMetadata() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return !frame_queue_.empty();
}

void OnvifMetadataGenerator::Clear() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!frame_queue_.empty()) {
        frame_queue_.pop();
    }
    current_frame_ = OnvifMetadataFrame();
    current_video_ts_ = 0;
}

} // namespace streaming
} // namespace ipcam
