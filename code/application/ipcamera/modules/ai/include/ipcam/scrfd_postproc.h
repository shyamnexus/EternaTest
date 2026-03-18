/**
 * @file scrfd_postproc.h
 * @brief SCRFD face detection post-processing for ipcamd analytics
 *
 * Decodes NPU output tensors from SCRFD-500M into face detection results.
 * Ported from ai3_custom_yolov8/scrfd_postproc.c.
 *
 * SCRFD outputs 9 tensors across 3 stride levels (8/16/32):
 *   score, bbox, keypoints for each stride.
 */
#ifndef IPCAM_SCRFD_POSTPROC_H_
#define IPCAM_SCRFD_POSTPROC_H_

#include "ipcam/npu_inference.h"
#include <vector>
#include <array>

#if VENDOR_AI3_ENABLED
extern "C" {
#include "vendor_ai_cpu/vendor_ai_cpu.h"
}
#endif

namespace ipcam {
namespace ai {

/// SCRFD configuration
struct ScrfdConfig {
    float conf_threshold = 0.25f;
    float nms_threshold  = 0.4f;
    int   input_width    = 640;
    int   input_height   = 640;
    int   max_faces      = 200;
};

/// A detected face with bounding box and 5 landmarks
struct FaceDetection {
    float x1, y1, x2, y2;      ///< Bounding box (normalized 0-1)
    float confidence;
    std::array<float, 10> landmarks;  ///< 5 keypoints: (x,y) pairs
                                      ///< [0,1]=left_eye [2,3]=right_eye
                                      ///< [4,5]=nose [6,7]=mouth_l [8,9]=mouth_r
};

/**
 * @brief SCRFD face detection post-processor
 *
 * Usage:
 *   ScrfdPostProc scrfd;
 *   scrfd.Init(config);
 *   auto faces = scrfd.Process(npu, net_id);
 */
class ScrfdPostProc {
public:
    bool Init(const ScrfdConfig& cfg = {});
    void Uninit();

    /// Process NPU outputs and return face detections
    std::vector<FaceDetection> Process(NpuInference& npu, int net_id);

    const ScrfdConfig& Config() const { return cfg_; }

private:
    static constexpr int NUM_STRIDES = 3;
    static constexpr int ANCHORS_PER_CELL = 2;
    static const int STRIDES[3];       // {8, 16, 32}
    static const int GRID_SIZES[3];    // {80, 40, 20}
    static const int ANCHOR_COUNTS[3]; // {12800, 3200, 800}

    struct StrideInfo {
        int score_idx = -1;
        int bbox_idx  = -1;
        int kps_idx   = -1;
        int num_anchors;
        int stride;
        int grid_size;
    };

    ScrfdConfig cfg_;
    bool initialized_ = false;

    int ClassifyOutputs(NpuInference& npu, int net_id, StrideInfo strides[3]);
    int DecodeStride(NpuInference& npu, int net_id, StrideInfo& si,
                     std::vector<FaceDetection>& faces);
    int ApplyNms(std::vector<FaceDetection>& faces, float nms_thresh, int max_out);
    static float CalculateIoU(const FaceDetection& a, const FaceDetection& b);
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_SCRFD_POSTPROC_H_
