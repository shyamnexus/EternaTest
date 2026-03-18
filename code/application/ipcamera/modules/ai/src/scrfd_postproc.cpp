/**
 * @file scrfd_postproc.cpp
 * @brief SCRFD face detection post-processing implementation
 *
 * Ported from ai3_custom_yolov8/scrfd_postproc.c.
 * Decodes 9 output tensors across 3 stride levels into face detections.
 */
#include "ipcam/scrfd_postproc.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cstring>
#include <cmath>

namespace ipcam {
namespace ai {

const int ScrfdPostProc::STRIDES[3]       = {8, 16, 32};
const int ScrfdPostProc::GRID_SIZES[3]    = {80, 40, 20};
const int ScrfdPostProc::ANCHOR_COUNTS[3] = {12800, 3200, 800};

bool ScrfdPostProc::Init(const ScrfdConfig& cfg) {
    cfg_ = cfg;
    initialized_ = true;
    spdlog::debug("ScrfdPostProc: initialized (conf={:.2f}, max_faces={})",
                  cfg_.conf_threshold, cfg_.max_faces);
    return true;
}

void ScrfdPostProc::Uninit() {
    initialized_ = false;
}

std::vector<FaceDetection> ScrfdPostProc::Process(NpuInference& npu, int net_id) {
    std::vector<FaceDetection> results;
    if (!initialized_) return results;

#if VENDOR_AI3_ENABLED
    StrideInfo strides[3];
    if (ClassifyOutputs(npu, net_id, strides) != 0) {
        spdlog::warn("ScrfdPostProc: failed to classify outputs");
        return results;
    }

    // Decode faces from each stride
    std::vector<FaceDetection> all_faces;
    for (int s = 0; s < NUM_STRIDES; s++) {
        int count = DecodeStride(npu, net_id, strides[s], all_faces);
        spdlog::debug("ScrfdPostProc: stride {} -> {} faces", STRIDES[s], count);
    }

    // NMS
    ApplyNms(all_faces, cfg_.nms_threshold, cfg_.max_faces);
    results = std::move(all_faces);
#else
    (void)npu; (void)net_id;
#endif

    return results;
}

int ScrfdPostProc::ClassifyOutputs(NpuInference& npu, int net_id, StrideInfo strides[3]) {
    uint32_t num_outputs = npu.GetOutputCount(net_id);

    for (int s = 0; s < 3; s++) {
        strides[s].score_idx = -1;
        strides[s].bbox_idx  = -1;
        strides[s].kps_idx   = -1;
        strides[s].num_anchors = ANCHOR_COUNTS[s];
        strides[s].stride      = STRIDES[s];
        strides[s].grid_size   = GRID_SIZES[s];
    }

    for (uint32_t i = 0; i < num_outputs; i++) {
        NpuOutputInfo out;
        if (!npu.GetOutput(net_id, i, out)) continue;

        int num_anchors = out.channel;
        int feat_count  = out.height;

        int stride_idx = -1;
        for (int s = 0; s < 3; s++) {
            if (num_anchors == ANCHOR_COUNTS[s]) {
                stride_idx = s;
                break;
            }
        }
        if (stride_idx < 0) continue;

        if (feat_count == 1)       strides[stride_idx].score_idx = i;
        else if (feat_count == 4)  strides[stride_idx].bbox_idx  = i;
        else if (feat_count == 10) strides[stride_idx].kps_idx   = i;
    }

    // Verify required outputs
    for (int s = 0; s < 3; s++) {
        if (strides[s].score_idx < 0 || strides[s].bbox_idx < 0) {
            spdlog::error("ScrfdPostProc: missing score/bbox for stride {}", STRIDES[s]);
            return -1;
        }
    }
    return 0;
}

int ScrfdPostProc::DecodeStride(NpuInference& npu, int net_id, StrideInfo& si,
                                std::vector<FaceDetection>& faces) {
#if VENDOR_AI3_ENABLED
    NpuOutputInfo score_out, bbox_out, kps_out;
    bool has_kps = false;

    if (!npu.GetOutput(net_id, si.score_idx, score_out)) return 0;
    if (!npu.GetOutput(net_id, si.bbox_idx, bbox_out)) return 0;
    if (si.kps_idx >= 0 && npu.GetOutput(net_id, si.kps_idx, kps_out)) {
        has_kps = true;
    }

    int num_anchors = si.num_anchors;
    float input_w = static_cast<float>(cfg_.input_width);
    float input_h = static_cast<float>(cfg_.input_height);

    // Convert to float
    std::vector<float> score_float(num_anchors);
    std::vector<float> bbox_float(num_anchors * 4);
    std::vector<float> kps_float;

    vendor_ai_cpu_util_fixed2float(reinterpret_cast<void*>(score_out.va),
                                   static_cast<HD_VIDEO_PXLFMT>(score_out.fmt), score_float.data(),
                                   score_out.scale, num_anchors, score_out.zero_point);

    vendor_ai_cpu_util_fixed2float(reinterpret_cast<void*>(bbox_out.va),
                                   static_cast<HD_VIDEO_PXLFMT>(bbox_out.fmt), bbox_float.data(),
                                   bbox_out.scale, num_anchors * 4, bbox_out.zero_point);

    if (has_kps) {
        kps_float.resize(num_anchors * 10);
        vendor_ai_cpu_util_fixed2float(reinterpret_cast<void*>(kps_out.va),
                                       static_cast<HD_VIDEO_PXLFMT>(kps_out.fmt), kps_float.data(),
                                       kps_out.scale, num_anchors * 10, kps_out.zero_point);
    }

    int face_count = 0;

    for (int anchor_idx = 0; anchor_idx < num_anchors; anchor_idx++) {
        float score = score_float[anchor_idx];
        if (score < cfg_.conf_threshold) continue;

        int cell_idx = anchor_idx / ANCHORS_PER_CELL;
        int gx = cell_idx % si.grid_size;
        int gy = cell_idx / si.grid_size;

        float cx = (gx + 0.5f) * si.stride;
        float cy = (gy + 0.5f) * si.stride;

        // Distance-based bbox decode
        float dx = bbox_float[anchor_idx * 4 + 0] * si.stride;
        float dy = bbox_float[anchor_idx * 4 + 1] * si.stride;
        float dw = bbox_float[anchor_idx * 4 + 2] * si.stride;
        float dh = bbox_float[anchor_idx * 4 + 3] * si.stride;

        FaceDetection f;
        f.x1 = std::max(0.0f, std::min(1.0f, (cx - dx) / input_w));
        f.y1 = std::max(0.0f, std::min(1.0f, (cy - dy) / input_h));
        f.x2 = std::max(0.0f, std::min(1.0f, (cx + dw) / input_w));
        f.y2 = std::max(0.0f, std::min(1.0f, (cy + dh) / input_h));
        f.confidence = score;

        // Decode keypoints
        if (has_kps) {
            for (int k = 0; k < 5; k++) {
                float kp_x = kps_float[anchor_idx * 10 + k * 2 + 0] * si.stride;
                float kp_y = kps_float[anchor_idx * 10 + k * 2 + 1] * si.stride;
                f.landmarks[k * 2 + 0] = std::max(0.0f, std::min(1.0f, (cx + kp_x) / input_w));
                f.landmarks[k * 2 + 1] = std::max(0.0f, std::min(1.0f, (cy + kp_y) / input_h));
            }
        } else {
            f.landmarks.fill(0.0f);
        }

        faces.push_back(f);
        face_count++;
    }

    return face_count;
#else
    (void)npu; (void)net_id; (void)si; (void)faces;
    return 0;
#endif
}

float ScrfdPostProc::CalculateIoU(const FaceDetection& a, const FaceDetection& b) {
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

int ScrfdPostProc::ApplyNms(std::vector<FaceDetection>& faces, float nms_thresh, int max_out) {
    if (faces.empty()) return 0;

    std::sort(faces.begin(), faces.end(),
              [](const FaceDetection& a, const FaceDetection& b) {
                  return a.confidence > b.confidence;
              });

    int n = static_cast<int>(faces.size());
    std::vector<bool> suppressed(n, false);
    std::vector<FaceDetection> kept;
    kept.reserve(max_out);

    for (int i = 0; i < n && static_cast<int>(kept.size()) < max_out; i++) {
        if (suppressed[i]) continue;
        kept.push_back(faces[i]);

        for (int j = i + 1; j < n; j++) {
            if (!suppressed[j] && CalculateIoU(faces[i], faces[j]) > nms_thresh) {
                suppressed[j] = true;
            }
        }
    }

    faces = std::move(kept);
    return static_cast<int>(faces.size());
}

} // namespace ai
} // namespace ipcam
