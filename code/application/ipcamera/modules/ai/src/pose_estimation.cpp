/**
 * @file pose_estimation.cpp
 * @brief Pose Estimation — YOLOv26-Pose keypoint detection
 *
 * Ported from ai3_features/11_pose_estimation demo.
 * Output tensor: [1,56,8400] CHW — 4 bbox + 1 conf + 17×3 keypoints.
 */
#include "ipcam/pose_estimation.h"
#include "ipcam/npu_inference.h"

#include <spdlog/spdlog.h>
#include <cmath>
#include <algorithm>

#if VENDOR_AI3_ENABLED
extern "C" {
#include "vendor_ai_cpu/vendor_ai_cpu.h"
}
#endif

namespace ipcam {
namespace ai {

static const char* const kCocoKeypoints[] = {
    "nose", "left_eye", "right_eye", "left_ear", "right_ear",
    "left_shoulder", "right_shoulder", "left_elbow", "right_elbow",
    "left_wrist", "right_wrist", "left_hip", "right_hip",
    "left_knee", "right_knee", "left_ankle", "right_ankle"
};

bool PoseEstimationEngine::Init(const PoseConfig& config) {
    config_ = config;
    initialized_ = true;
    spdlog::info("PoseEstimationEngine initialized (conf={:.2f}, nms={:.2f})",
                 config_.conf_threshold, config_.nms_threshold);
    return true;
}

void PoseEstimationEngine::Shutdown() {
    initialized_ = false;
    float_buf_.clear();
}

std::vector<PoseResult> PoseEstimationEngine::Process(NpuInference& npu, int net_id) {
    std::vector<PoseResult> results;
#if VENDOR_AI3_ENABLED
    if (!initialized_ || net_id < 0) return results;

    NpuOutputInfo out;
    if (!npu.GetOutput(net_id, 0, out)) return results;

    // Dequantize
    uint32_t total = out.channel * out.width * out.height;
    float_buf_.resize(total);
    vendor_ai_cpu_util_fixed2float(
        reinterpret_cast<void*>(out.va), static_cast<HD_VIDEO_PXLFMT>(out.fmt),
        float_buf_.data(), out.scale, static_cast<INT32>(total), out.zero_point);

    // Decode
    std::vector<RawPose> poses;
    Decode(float_buf_.data(), poses);
    ApplyNms(poses);

    // Convert to output format
    for (const auto& p : poses) {
        PoseResult r;
        r.x1 = p.x1; r.y1 = p.y1;
        r.x2 = p.x2; r.y2 = p.y2;
        r.confidence = p.conf;
        r.keypoints = p.kpts;
        results.push_back(r);
        if (static_cast<int>(results.size()) >= config_.max_persons) break;
    }
#else
    (void)npu; (void)net_id;
#endif
    return results;
}

int PoseEstimationEngine::Decode(const float* data, std::vector<RawPose>& poses) {
    // CHW layout: [56, 8400] — despite NPU reporting 1×8400×56,
    // actual memory layout is channel-major (matches demo)
    int anchors = kNumAnchors;

    for (int a = 0; a < anchors; a++) {
        float conf = data[4 * anchors + a];
        if (conf < config_.conf_threshold) continue;

        RawPose p;
        float cx = data[0 * anchors + a];
        float cy = data[1 * anchors + a];
        float w  = data[2 * anchors + a];
        float h  = data[3 * anchors + a];
        p.x1 = cx - w / 2.0f;
        p.y1 = cy - h / 2.0f;
        p.x2 = cx + w / 2.0f;
        p.y2 = cy + h / 2.0f;
        p.conf = conf;

        // 17 keypoints: channels 5..55 in groups of 3 (x, y, conf)
        for (int k = 0; k < kNumKpts; k++) {
            int base = (5 + k * 3);
            p.kpts[k].x = data[base * anchors + a];
            p.kpts[k].y = data[(base + 1) * anchors + a];
            p.kpts[k].confidence = data[(base + 2) * anchors + a];
            // Keypoint conf is often quantized to 0.  Use in-bounds as fallback.
            p.kpts[k].valid = (p.kpts[k].x >= 0.0f && p.kpts[k].x <= 1.0f &&
                               p.kpts[k].y >= 0.0f && p.kpts[k].y <= 1.0f);
        }
        poses.push_back(p);
    }
    return static_cast<int>(poses.size());
}

int PoseEstimationEngine::ApplyNms(std::vector<RawPose>& poses) {
    std::sort(poses.begin(), poses.end(),
              [](const RawPose& a, const RawPose& b) { return a.conf > b.conf; });

    std::vector<bool> suppressed(poses.size(), false);
    for (size_t i = 0; i < poses.size(); i++) {
        if (suppressed[i]) continue;
        for (size_t j = i + 1; j < poses.size(); j++) {
            if (suppressed[j]) continue;
            if (IoU(poses[i], poses[j]) > config_.nms_threshold)
                suppressed[j] = true;
        }
    }
    std::vector<RawPose> kept;
    for (size_t i = 0; i < poses.size(); i++)
        if (!suppressed[i]) kept.push_back(poses[i]);
    poses = std::move(kept);
    return static_cast<int>(poses.size());
}

float PoseEstimationEngine::IoU(const RawPose& a, const RawPose& b) {
    float ix1 = std::max(a.x1, b.x1), iy1 = std::max(a.y1, b.y1);
    float ix2 = std::min(a.x2, b.x2), iy2 = std::min(a.y2, b.y2);
    float iw = std::max(0.0f, ix2 - ix1), ih = std::max(0.0f, iy2 - iy1);
    float inter = iw * ih;
    float aa = (a.x2 - a.x1) * (a.y2 - a.y1);
    float ab = (b.x2 - b.x1) * (b.y2 - b.y1);
    return inter / (aa + ab - inter + 1e-6f);
}

const char* PoseEstimationEngine::KeypointName(int idx) {
    if (idx >= 0 && idx < kNumKpts) return kCocoKeypoints[idx];
    return "unknown";
}

} // namespace ai
} // namespace ipcam
