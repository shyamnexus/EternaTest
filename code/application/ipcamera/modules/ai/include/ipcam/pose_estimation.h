/**
 * @file pose_estimation.h
 * @brief Pose Estimation — YOLOv26-Pose keypoint detection
 *
 * Runs YOLOv26n-Pose on the NPU and decodes 17 COCO keypoints per person
 * from the [1,56,8400] CHW output tensor.
 */
#ifndef IPCAM_POSE_ESTIMATION_H_
#define IPCAM_POSE_ESTIMATION_H_

#include <vector>
#include <array>
#include <cstdint>

namespace ipcam {
namespace ai {

class NpuInference;

struct PoseConfig {
    bool enabled = false;
    float conf_threshold = 0.35f;
    float nms_threshold = 0.50f;
    int max_persons = 16;
};

struct Keypoint {
    float x = 0.0f;          ///< Normalised 0–1
    float y = 0.0f;
    float confidence = 0.0f;
    bool valid = false;
};

struct PoseResult {
    float x1, y1, x2, y2;   ///< Person bbox normalised
    float confidence;
    std::array<Keypoint, 17> keypoints;
};

class PoseEstimationEngine {
public:
    bool Init(const PoseConfig& config);
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    bool IsEnabled() const { return initialized_ && config_.enabled; }

    /// Decode poses from an already-inferred yolo26n_pose network
    std::vector<PoseResult> Process(NpuInference& npu, int net_id);

    static const char* KeypointName(int idx);

private:
    static constexpr int kNumAnchors  = 8400;
    static constexpr int kNumChannels = 56;   // 4 bbox + 1 conf + 17*3 kpts
    static constexpr int kNumKpts     = 17;

    PoseConfig config_;
    bool initialized_ = false;
    std::vector<float> float_buf_;

    struct RawPose {
        float x1, y1, x2, y2, conf;
        std::array<Keypoint, 17> kpts;
    };

    int Decode(const float* data, std::vector<RawPose>& poses);
    int ApplyNms(std::vector<RawPose>& poses);
    static float IoU(const RawPose& a, const RawPose& b);
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_POSE_ESTIMATION_H_
