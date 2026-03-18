/**
 * @file lpr.h
 * @brief License Plate Recognition — YOLOv11-LP detection + LPRNet OCR
 *
 * Two-stage pipeline:
 * 1. Detect plates with lp_det_yolo11n (1-class YOLO, [5,8400] output)
 * 2. Crop each plate to 94x24 NV12, run indian_lprnet_ocr ([37,18] output)
 * 3. CTC greedy decode with logit averaging across frames
 */
#ifndef IPCAM_LPR_H_
#define IPCAM_LPR_H_

#include <string>
#include <vector>
#include <array>
#include <cstdint>

namespace ipcam {
namespace ai {

class NpuInference;

struct LprConfig {
    bool enabled = false;
    float det_conf_threshold = 0.25f;
    float det_nms_threshold = 0.45f;
    float ocr_confidence_margin = 2.0f;  ///< Logit margin for CTC
    float plate_margin = 0.05f;          ///< Expand plate bbox by this fraction
    int logit_avg_frames = 8;            ///< Frames to average logits over
};

struct PlateDetection {
    float x1, y1, x2, y2;   ///< Normalised bbox
    float confidence;
    std::string plate_text;  ///< Decoded plate string
};

class LprEngine {
public:
    bool Init(const LprConfig& config);
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    bool IsEnabled() const { return initialized_ && config_.enabled; }

    /// Full LPR pipeline: detect plates → crop → OCR → decode
    std::vector<PlateDetection> Process(
        NpuInference& npu, int det_net_id, int ocr_net_id,
        const uint8_t* yuv_data, uint32_t yuv_w, uint32_t yuv_h, uint32_t yuv_stride,
        uintptr_t crop_pa, uintptr_t crop_va);

private:
    static constexpr int kNumClasses  = 37;   // 0-9, A-Z, blank
    static constexpr int kSeqLen      = 18;   // CTC timesteps
    static constexpr int kBlankIdx    = 36;
    static constexpr int kMaxAnchors  = 8400;
    static constexpr int kMaxDet      = 20;
    static constexpr int kCropW       = 94;
    static constexpr int kCropH       = 24;

    LprConfig config_;
    bool initialized_ = false;

    // Logit averaging ring buffer
    std::array<float, kNumClasses * kSeqLen> logit_accum_{};
    int logit_count_ = 0;
    int frames_without_det_ = 0;

    struct DetBox { float x1, y1, x2, y2, conf; };

    std::vector<DetBox> DetectPlates(NpuInference& npu, int det_net_id);
    std::string RunOcr(NpuInference& npu, int ocr_net_id,
                       const uint8_t* yuv_data, uint32_t yuv_w, uint32_t yuv_h,
                       uint32_t yuv_stride,
                       float x1, float y1, float x2, float y2,
                       uintptr_t crop_pa, uintptr_t crop_va);
    std::string CtcGreedyDecode(const float* logits) const;

    static const char kCharset[];
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_LPR_H_
