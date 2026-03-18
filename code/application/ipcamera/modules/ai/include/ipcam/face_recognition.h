/**
 * @file face_recognition.h
 * @brief Face Recognition Engine — MobileFaceNet embedding + gallery matching
 *
 * For each detected face (from SCRFD), crops to 112x112 NV12, runs MobileFaceNet
 * on the NPU to extract a 512-D L2-normalized embedding, then compares against
 * a persistent gallery using cosine similarity.
 */
#ifndef IPCAM_FACE_RECOGNITION_H_
#define IPCAM_FACE_RECOGNITION_H_

#include <string>
#include <vector>
#include <mutex>
#include <cstdint>

namespace ipcam {
namespace ai {

class NpuInference;
struct FaceDetection;

struct FaceRecognitionConfig {
    bool enabled = false;
    std::string database_path = "/mnt/app/face_db";
    float similarity_threshold = 0.70f;
    int max_gallery_size = 1000;
    float margin = 0.20f;  ///< Expand face bbox by this fraction
};

struct FaceMatch {
    int gallery_id = -1;
    std::string name;
    float similarity = 0.0f;
    bool is_known = false;
};

struct GalleryEntry {
    int id = 0;
    std::string name;
    std::vector<float> features;  ///< 512-D L2-normalised embedding
};

class FaceRecognitionEngine {
public:
    bool Init(const FaceRecognitionConfig& config);
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    bool IsEnabled() const { return initialized_ && config_.enabled; }

    /// For each face, crop→MobileFaceNet→match against gallery.
    /// Uses the shared crop buffer (must be >= 112*112*3/2 bytes).
    std::vector<FaceMatch> ProcessFaces(
        const std::vector<FaceDetection>& faces,
        const uint8_t* yuv_data, uint32_t yuv_w, uint32_t yuv_h, uint32_t yuv_stride,
        NpuInference& npu, int mobilefacenet_id,
        uintptr_t crop_pa, uintptr_t crop_va);

    // Gallery CRUD
    int EnrollFace(const std::string& name, const std::vector<float>& features);
    bool RemoveFace(int id);
    std::vector<GalleryEntry> GetGallery() const;
    bool SaveGallery() const;
    bool LoadGallery();

private:
    static constexpr uint32_t kFeatureDim = 512;
    static constexpr uint32_t kCropW = 112;
    static constexpr uint32_t kCropH = 112;

    FaceRecognitionConfig config_;
    bool initialized_ = false;
    mutable std::mutex mutex_;
    std::vector<GalleryEntry> gallery_;
    int next_id_ = 1;

    static float CosineSimilarity(const float* a, const float* b, uint32_t dim);
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_FACE_RECOGNITION_H_
