/**
 * @file yolo_postproc.h
 * @brief YOLOv26/v8 post-processing for ipcamd analytics
 *
 * Decodes NPU output tensors from YOLO models into detection results.
 * Ported from ai3_custom_yolov26/yolov26_postproc.c for use in the
 * production ipcamd analytics engine.
 *
 * Supports: yolo26n, yolo26s, yolov8n (all use same [1,84,8400] output format)
 */
#ifndef IPCAM_YOLO_POSTPROC_H_
#define IPCAM_YOLO_POSTPROC_H_

#include "ipcam/npu_inference.h"
#include "ipcam/analytics.h"
#include <vector>
#include <string>

#if VENDOR_AI3_ENABLED
extern "C" {
#include "vendor_ai_cpu/vendor_ai_cpu.h"
}
#endif

namespace ipcam {
namespace ai {

/// YOLO model configuration
struct YoloConfig {
    float conf_threshold = 0.25f;
    float nms_threshold  = 0.45f;
    int   num_classes    = 80;
    int   num_anchors    = 8400;
    int   input_width    = 640;
    int   input_height   = 640;
    int   max_detections = 100;
};

/// Raw YOLO detection (internal, before conversion to DetectionResult)
struct YoloDetection {
    float x1, y1, x2, y2;  // normalized 0-1
    float confidence;
    int   class_id;
};

/**
 * @brief YOLO post-processor
 *
 * Usage:
 *   YoloPostProc yolo;
 *   yolo.Init(config);
 *   // after NpuInference::Infer()
 *   auto detections = yolo.Process(npu, net_id);
 */
class YoloPostProc {
public:
    bool Init(const YoloConfig& cfg = {});
    void Uninit();

    /// Process NPU outputs and return detections
    /// @param npu    NPU inference engine reference
    /// @param net_id Network ID of the YOLO model
    /// @return Vector of detection results (normalized coords)
    std::vector<DetectionResult> Process(NpuInference& npu, int net_id);

    /// Map YOLO class ID to ObjectCategory
    static ObjectCategory ClassToCategory(int class_id);

    /// Get COCO class name
    static const char* ClassName(int class_id);

    const YoloConfig& Config() const { return cfg_; }

private:
    YoloConfig cfg_;
    std::vector<float> float_buf_;
    std::vector<YoloDetection> det_buf_;
    std::vector<int> indices_;
    bool initialized_ = false;

    int DecodeOutput(const float* output, std::vector<YoloDetection>& dets);
    int ApplyNms(std::vector<YoloDetection>& dets, float nms_thresh, int max_out);

    static float CalculateIoU(const YoloDetection& a, const YoloDetection& b);
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_YOLO_POSTPROC_H_
