/**
 * @file yolo_postproc.cpp
 * @brief YOLOv26/v8 post-processing implementation
 *
 * Ported from ai3_custom_yolov26/yolov26_postproc.c.
 * Decodes [1, 84, 8400] CHW tensor into detection results with NMS.
 */
#include "ipcam/yolo_postproc.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace ipcam {
namespace ai {

// COCO 80-class names
static const char* const COCO_NAMES[80] = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck",
    "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
    "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra",
    "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",
    "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup",
    "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange",
    "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse",
    "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
    "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

bool YoloPostProc::Init(const YoloConfig& cfg) {
    cfg_ = cfg;

    int total = cfg_.num_anchors * (4 + cfg_.num_classes);
    float_buf_.resize(total);
    det_buf_.resize(cfg_.max_detections * 4);
    indices_.resize(cfg_.max_detections * 4);

    initialized_ = true;
    spdlog::debug("YoloPostProc: initialized (classes={}, anchors={}, conf={:.2f})",
                  cfg_.num_classes, cfg_.num_anchors, cfg_.conf_threshold);
    return true;
}

void YoloPostProc::Uninit() {
    float_buf_.clear();
    det_buf_.clear();
    indices_.clear();
    initialized_ = false;
}

std::vector<DetectionResult> YoloPostProc::Process(NpuInference& npu, int net_id) {
    std::vector<DetectionResult> results;
    if (!initialized_) return results;

#if VENDOR_AI3_ENABLED
    // Get output tensor (YOLO has single output)
    NpuOutputInfo out;
    if (!npu.GetOutput(net_id, 0, out)) {
        spdlog::warn("YoloPostProc: failed to get output tensor");
        return results;
    }

    int total_elements = cfg_.num_anchors * (4 + cfg_.num_classes);

    // Convert fixed-point to float
    HD_RESULT ret = vendor_ai_cpu_util_fixed2float(
        reinterpret_cast<void*>(out.va),
        static_cast<HD_VIDEO_PXLFMT>(out.fmt),
        float_buf_.data(),
        out.scale,
        total_elements,
        out.zero_point
    );

    if (ret != HD_OK) {
        spdlog::warn("YoloPostProc: fixed2float failed: {}", static_cast<int>(ret));
        return results;
    }

    // Decode
    det_buf_.clear();
    int pre_nms = DecodeOutput(float_buf_.data(), det_buf_);

    // NMS
    int post_nms = ApplyNms(det_buf_, cfg_.nms_threshold, cfg_.max_detections);

    // Convert to DetectionResult
    results.reserve(post_nms);
    for (int i = 0; i < post_nms; i++) {
        const auto& d = det_buf_[i];
        DetectionResult r;
        r.id = i;
        r.category = ClassToCategory(d.class_id);
        r.bbox.x1 = d.x1;
        r.bbox.y1 = d.y1;
        r.bbox.x2 = d.x2;
        r.bbox.y2 = d.y2;
        r.confidence = d.confidence;
        results.push_back(r);
    }
#else
    (void)npu; (void)net_id;
#endif

    return results;
}

int YoloPostProc::DecodeOutput(const float* output, std::vector<YoloDetection>& dets) {
    const float input_w = static_cast<float>(cfg_.input_width);
    const float input_h = static_cast<float>(cfg_.input_height);
    const int num_anchors = cfg_.num_anchors;
    const int num_classes = cfg_.num_classes;

    // CHW layout: 84 channels x 8400 anchors
    const float* cx_data  = output + 0 * num_anchors;
    const float* cy_data  = output + 1 * num_anchors;
    const float* w_data   = output + 2 * num_anchors;
    const float* h_data   = output + 3 * num_anchors;
    const float* cls_data = output + 4 * num_anchors;

    int count = 0;
    int max_pre_nms = cfg_.max_detections * 4;

    for (int i = 0; i < num_anchors && count < max_pre_nms; i++) {
        // Find best class
        int best_class = 0;
        float best_score = 0.0f;

        for (int c = 0; c < num_classes; c++) {
            float score = cls_data[c * num_anchors + i];
            if (score > best_score) {
                best_score = score;
                best_class = c;
            }
        }

        if (best_score < cfg_.conf_threshold) continue;

        float cx = cx_data[i];
        float cy = cy_data[i];
        float w  = w_data[i];
        float h  = h_data[i];

        YoloDetection d;
        d.x1 = std::max(0.0f, std::min(1.0f, (cx - w / 2.0f) / input_w));
        d.y1 = std::max(0.0f, std::min(1.0f, (cy - h / 2.0f) / input_h));
        d.x2 = std::max(0.0f, std::min(1.0f, (cx + w / 2.0f) / input_w));
        d.y2 = std::max(0.0f, std::min(1.0f, (cy + h / 2.0f) / input_h));
        d.confidence = best_score;
        d.class_id = best_class;

        dets.push_back(d);
        count++;
    }

    return count;
}

float YoloPostProc::CalculateIoU(const YoloDetection& a, const YoloDetection& b) {
    float x1 = std::max(a.x1, b.x1);
    float y1 = std::max(a.y1, b.y1);
    float x2 = std::min(a.x2, b.x2);
    float y2 = std::min(a.y2, b.y2);

    if (x2 <= x1 || y2 <= y1) return 0.0f;

    float inter = (x2 - x1) * (y2 - y1);
    float area_a = (a.x2 - a.x1) * (a.y2 - a.y1);
    float area_b = (b.x2 - b.x1) * (b.y2 - b.y1);
    float u = area_a + area_b - inter;

    return u > 0.0f ? inter / u : 0.0f;
}

int YoloPostProc::ApplyNms(std::vector<YoloDetection>& dets, float nms_thresh, int max_out) {
    if (dets.empty()) return 0;

    // Sort by confidence descending
    std::sort(dets.begin(), dets.end(),
              [](const YoloDetection& a, const YoloDetection& b) {
                  return a.confidence > b.confidence;
              });

    int n = static_cast<int>(dets.size());
    std::vector<bool> suppressed(n, false);
    std::vector<YoloDetection> kept;
    kept.reserve(max_out);

    for (int i = 0; i < n && static_cast<int>(kept.size()) < max_out; i++) {
        if (suppressed[i]) continue;
        kept.push_back(dets[i]);

        for (int j = i + 1; j < n; j++) {
            if (suppressed[j]) continue;
            if (dets[j].class_id != dets[i].class_id) continue;
            if (CalculateIoU(dets[i], dets[j]) > nms_thresh) {
                suppressed[j] = true;
            }
        }
    }

    dets = std::move(kept);
    return static_cast<int>(dets.size());
}

ObjectCategory YoloPostProc::ClassToCategory(int class_id) {
    switch (class_id) {
        case 0:  return ObjectCategory::kPerson;
        case 1:  // bicycle
        case 2:  // car
        case 3:  // motorcycle
        case 5:  // bus
        case 7:  // truck
            return ObjectCategory::kVehicle;
        case 14: // bird
        case 15: // cat
        case 16: // dog
        case 17: // horse
        case 18: // sheep
        case 19: // cow
        case 20: // elephant
        case 21: // bear
        case 22: // zebra
        case 23: // giraffe
            return ObjectCategory::kUnknown; // animal classes
        default:
            return ObjectCategory::kUnknown;
    }
}

const char* YoloPostProc::ClassName(int class_id) {
    if (class_id >= 0 && class_id < 80) return COCO_NAMES[class_id];
    return "unknown";
}

} // namespace ai
} // namespace ipcam
