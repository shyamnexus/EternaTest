/**
 * @file face_attribute.h
 * @brief Face Attribute Engine — gender/age prediction
 *
 * For each detected face, crops to 96x96 NV12, runs the face_attribute model on
 * the NPU, and extracts gender (sigmoid) and age (scaled) from the 3-D output.
 */
#ifndef IPCAM_FACE_ATTRIBUTE_H_
#define IPCAM_FACE_ATTRIBUTE_H_

#include <vector>
#include <cstdint>

namespace ipcam {
namespace ai {

class NpuInference;
struct FaceDetection;

struct FaceAttributeConfig {
    bool enabled = false;
    float margin = 0.20f;   ///< Expand face bbox by this fraction
};

struct FaceAttributes {
    bool is_male = false;
    int age = 0;
    float gender_confidence = 0.0f;
};

class FaceAttributeEngine {
public:
    bool Init(const FaceAttributeConfig& config);
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    bool IsEnabled() const { return initialized_ && config_.enabled; }

    /// For each face, crop→face_attribute→decode gender/age
    std::vector<FaceAttributes> Process(
        const std::vector<FaceDetection>& faces,
        const uint8_t* yuv_data, uint32_t yuv_w, uint32_t yuv_h, uint32_t yuv_stride,
        NpuInference& npu, int attr_net_id,
        uintptr_t crop_pa, uintptr_t crop_va);

private:
    static constexpr uint32_t kCropW = 96;
    static constexpr uint32_t kCropH = 96;
    static constexpr int kOutputDim = 3;

    FaceAttributeConfig config_;
    bool initialized_ = false;
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_FACE_ATTRIBUTE_H_
