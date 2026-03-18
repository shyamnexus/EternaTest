/**
 * @file face_recognition.cpp
 * @brief Face Recognition — MobileFaceNet embedding extraction + gallery matching
 *
 * Ported from ai3_features/09_face_recognition demo.
 */
#include "ipcam/face_recognition.h"
#include "ipcam/npu_inference.h"
#include "ipcam/scrfd_postproc.h"

#include <spdlog/spdlog.h>
#include <cmath>
#include <cstring>
#include <fstream>
#include <algorithm>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hd_common.h"
}
#endif

#if VENDOR_AI3_ENABLED
extern "C" {
#include "vendor_ai_cpu/vendor_ai_cpu.h"
}
#endif

namespace ipcam {
namespace ai {

// NV12 crop+resize (same helper used across all modules)
static void NV12CropResize(const uint8_t* src_y, const uint8_t* src_uv,
                            uint32_t src_w, uint32_t src_h, uint32_t src_stride,
                            uint32_t cx, uint32_t cy, uint32_t cw, uint32_t ch,
                            uint8_t* dst_y, uint8_t* dst_uv,
                            uint32_t dst_w, uint32_t dst_h)
{
    for (uint32_t dy = 0; dy < dst_h; dy++) {
        uint32_t sy = cy + (dy * ch) / dst_h;
        if (sy >= src_h) sy = src_h - 1;
        for (uint32_t dx = 0; dx < dst_w; dx++) {
            uint32_t sx = cx + (dx * cw) / dst_w;
            if (sx >= src_w) sx = src_w - 1;
            dst_y[dy * dst_w + dx] = src_y[sy * src_stride + sx];
        }
    }
    uint32_t dst_uvh = dst_h / 2;
    for (uint32_t dy = 0; dy < dst_uvh; dy++) {
        uint32_t sy = cy / 2 + (dy * (ch / 2)) / dst_uvh;
        if (sy >= src_h / 2) sy = src_h / 2 - 1;
        for (uint32_t dx = 0; dx < dst_w / 2; dx++) {
            uint32_t sx = cx / 2 + (dx * (cw / 2)) / (dst_w / 2);
            if (sx >= src_w / 2) sx = src_w / 2 - 1;
            dst_uv[dy * dst_w + dx * 2]     = src_uv[sy * src_stride + sx * 2];
            dst_uv[dy * dst_w + dx * 2 + 1] = src_uv[sy * src_stride + sx * 2 + 1];
        }
    }
}

bool FaceRecognitionEngine::Init(const FaceRecognitionConfig& config) {
    config_ = config;
    LoadGallery();
    initialized_ = true;
    spdlog::info("FaceRecognitionEngine initialized (gallery={} faces, threshold={:.2f})",
                 gallery_.size(), config_.similarity_threshold);
    return true;
}

void FaceRecognitionEngine::Shutdown() {
    if (initialized_) {
        SaveGallery();
        initialized_ = false;
    }
}

std::vector<FaceMatch> FaceRecognitionEngine::ProcessFaces(
    const std::vector<FaceDetection>& faces,
    const uint8_t* yuv_data, uint32_t yuv_w, uint32_t yuv_h, uint32_t yuv_stride,
    NpuInference& npu, int mobilefacenet_id,
    uintptr_t crop_pa, uintptr_t crop_va)
{
    std::vector<FaceMatch> results;
#if VENDOR_AI3_ENABLED
    if (!initialized_ || mobilefacenet_id < 0 || !yuv_data || !crop_va) return results;

    const uint8_t* src_y  = yuv_data;
    const uint8_t* src_uv = yuv_data + yuv_stride * yuv_h;

    for (const auto& face : faces) {
        // Expand bbox by margin
        float fw = face.x2 - face.x1;
        float fh = face.y2 - face.y1;
        float mx = fw * config_.margin;
        float my = fh * config_.margin;
        float fx1 = std::max(0.0f, face.x1 - mx);
        float fy1 = std::max(0.0f, face.y1 - my);
        float fx2 = std::min(1.0f, face.x2 + mx);
        float fy2 = std::min(1.0f, face.y2 + my);

        // Convert to pixel coords (even-aligned for NV12)
        uint32_t bx = static_cast<uint32_t>(fx1 * yuv_w) & ~1u;
        uint32_t by = static_cast<uint32_t>(fy1 * yuv_h) & ~1u;
        uint32_t bw = (static_cast<uint32_t>((fx2 - fx1) * yuv_w) + 1) & ~1u;
        uint32_t bh = (static_cast<uint32_t>((fy2 - fy1) * yuv_h) + 1) & ~1u;
        if (bw < 16 || bh < 16) continue;
        if (bx + bw > yuv_w) bw = yuv_w - bx;
        if (by + bh > yuv_h) bh = yuv_h - by;

        // Crop + resize to 112x112
        auto* dst_y  = reinterpret_cast<uint8_t*>(crop_va);
        auto* dst_uv = dst_y + kCropW * kCropH;
        NV12CropResize(src_y, src_uv, yuv_w, yuv_h, yuv_stride,
                       bx, by, bw, bh, dst_y, dst_uv, kCropW, kCropH);

        hd_common_mem_cache_sync(reinterpret_cast<void*>(crop_va),
                                 kCropW * kCropH * 3 / 2, HD_COMMON_MEM_DMA_TO_DEVICE);

        if (!npu.Infer(mobilefacenet_id, crop_pa, crop_va, kCropW, kCropH, kCropW))
            continue;

        // Extract 512-D embedding
        NpuOutputInfo out;
        if (!npu.GetOutput(mobilefacenet_id, 0, out)) continue;

        float embedding[kFeatureDim];
        const auto* raw = reinterpret_cast<const int8_t*>(out.va);
        float norm_sq = 0.0f;
        for (uint32_t i = 0; i < kFeatureDim; i++) {
            float val = (static_cast<float>(raw[i]) - out.zero_point) * out.scale;
            embedding[i] = val;
            norm_sq += val * val;
        }
        if (norm_sq > 1e-6f) {
            float inv = 1.0f / std::sqrt(norm_sq);
            for (uint32_t i = 0; i < kFeatureDim; i++) embedding[i] *= inv;
        }

        // Match against gallery
        FaceMatch match;
        float best_sim = -1.0f;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& entry : gallery_) {
                float sim = CosineSimilarity(embedding, entry.features.data(), kFeatureDim);
                if (sim > best_sim) {
                    best_sim = sim;
                    match.gallery_id = entry.id;
                    match.name = entry.name;
                    match.similarity = sim;
                }
            }
        }
        match.is_known = (best_sim >= config_.similarity_threshold);
        if (!match.is_known) {
            match.gallery_id = -1;
            match.name.clear();
        }
        results.push_back(match);
    }
#else
    (void)faces; (void)yuv_data; (void)yuv_w; (void)yuv_h; (void)yuv_stride;
    (void)npu; (void)mobilefacenet_id; (void)crop_pa; (void)crop_va;
#endif
    return results;
}

int FaceRecognitionEngine::EnrollFace(const std::string& name, const std::vector<float>& features) {
    if (features.size() != kFeatureDim) return -1;
    std::lock_guard<std::mutex> lock(mutex_);
    if (static_cast<int>(gallery_.size()) >= config_.max_gallery_size) return -1;
    GalleryEntry entry;
    entry.id = next_id_++;
    entry.name = name;
    entry.features = features;
    gallery_.push_back(entry);
    spdlog::info("Face enrolled: id={} name={}", entry.id, name);
    return entry.id;
}

bool FaceRecognitionEngine::RemoveFace(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(gallery_.begin(), gallery_.end(),
                           [id](const GalleryEntry& e) { return e.id == id; });
    if (it == gallery_.end()) return false;
    gallery_.erase(it);
    return true;
}

std::vector<GalleryEntry> FaceRecognitionEngine::GetGallery() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return gallery_;
}

bool FaceRecognitionEngine::SaveGallery() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string path = config_.database_path + "/gallery.bin";
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return false;
    uint32_t count = static_cast<uint32_t>(gallery_.size());
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& entry : gallery_) {
        int32_t id = entry.id;
        ofs.write(reinterpret_cast<const char*>(&id), sizeof(id));
        uint32_t name_len = static_cast<uint32_t>(entry.name.size());
        ofs.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        ofs.write(entry.name.data(), name_len);
        ofs.write(reinterpret_cast<const char*>(entry.features.data()),
                  kFeatureDim * sizeof(float));
    }
    return ofs.good();
}

bool FaceRecognitionEngine::LoadGallery() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string path = config_.database_path + "/gallery.bin";
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return false;
    uint32_t count = 0;
    ifs.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (count > 10000) return false;  // sanity
    gallery_.clear();
    for (uint32_t i = 0; i < count && ifs.good(); i++) {
        GalleryEntry entry;
        int32_t id;
        ifs.read(reinterpret_cast<char*>(&id), sizeof(id));
        entry.id = id;
        uint32_t name_len;
        ifs.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
        if (name_len > 256) break;  // sanity
        entry.name.resize(name_len);
        ifs.read(entry.name.data(), name_len);
        entry.features.resize(kFeatureDim);
        ifs.read(reinterpret_cast<char*>(entry.features.data()),
                 kFeatureDim * sizeof(float));
        gallery_.push_back(entry);
        if (entry.id >= next_id_) next_id_ = entry.id + 1;
    }
    spdlog::info("Face gallery loaded: {} entries from {}", gallery_.size(), path);
    return true;
}

float FaceRecognitionEngine::CosineSimilarity(const float* a, const float* b, uint32_t dim) {
    float dot = 0.0f;
    for (uint32_t i = 0; i < dim; i++) dot += a[i] * b[i];
    return dot;  // vectors are L2-normalised, so dot == cosine similarity
}

} // namespace ai
} // namespace ipcam
