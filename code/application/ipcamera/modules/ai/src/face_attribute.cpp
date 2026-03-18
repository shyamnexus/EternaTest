/**
 * @file face_attribute.cpp
 * @brief Face Attribute — gender/age prediction from face_attribute.bin
 *
 * Ported from ai3_features/13_face_attribute demo.
 * Output: 3-D vector [gender_logit, age0, age1].
 *   gender = sigmoid(out[0]) > 0.5 → Male
 *   age    = out[2] * 100, clamped [0,120]
 */
#include "ipcam/face_attribute.h"
#include "ipcam/npu_inference.h"
#include "ipcam/scrfd_postproc.h"

#include <spdlog/spdlog.h>
#include <cmath>
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

// NV12 crop+resize
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

bool FaceAttributeEngine::Init(const FaceAttributeConfig& config) {
    config_ = config;
    initialized_ = true;
    spdlog::info("FaceAttributeEngine initialized");
    return true;
}

void FaceAttributeEngine::Shutdown() {
    initialized_ = false;
}

std::vector<FaceAttributes> FaceAttributeEngine::Process(
    const std::vector<FaceDetection>& faces,
    const uint8_t* yuv_data, uint32_t yuv_w, uint32_t yuv_h, uint32_t yuv_stride,
    NpuInference& npu, int attr_net_id,
    uintptr_t crop_pa, uintptr_t crop_va)
{
    std::vector<FaceAttributes> results;
#if VENDOR_AI3_ENABLED
    if (!initialized_ || attr_net_id < 0 || !yuv_data || !crop_va) return results;

    const uint8_t* src_y  = yuv_data;
    const uint8_t* src_uv = yuv_data + yuv_stride * yuv_h;

    for (const auto& face : faces) {
        float fw = face.x2 - face.x1;
        float fh = face.y2 - face.y1;
        float mx = fw * config_.margin;
        float my = fh * config_.margin;
        float fx1 = std::max(0.0f, face.x1 - mx);
        float fy1 = std::max(0.0f, face.y1 - my);
        float fx2 = std::min(1.0f, face.x2 + mx);
        float fy2 = std::min(1.0f, face.y2 + my);

        uint32_t bx = static_cast<uint32_t>(fx1 * yuv_w) & ~1u;
        uint32_t by = static_cast<uint32_t>(fy1 * yuv_h) & ~1u;
        uint32_t bw = (static_cast<uint32_t>((fx2 - fx1) * yuv_w) + 1) & ~1u;
        uint32_t bh = (static_cast<uint32_t>((fy2 - fy1) * yuv_h) + 1) & ~1u;
        if (bw < 16 || bh < 16) continue;
        if (bx + bw > yuv_w) bw = yuv_w - bx;
        if (by + bh > yuv_h) bh = yuv_h - by;

        auto* dst_y  = reinterpret_cast<uint8_t*>(crop_va);
        auto* dst_uv = dst_y + kCropW * kCropH;
        NV12CropResize(src_y, src_uv, yuv_w, yuv_h, yuv_stride,
                       bx, by, bw, bh, dst_y, dst_uv, kCropW, kCropH);

        hd_common_mem_cache_sync(reinterpret_cast<void*>(crop_va),
                                 kCropW * kCropH * 3 / 2, HD_COMMON_MEM_DMA_TO_DEVICE);

        if (!npu.Infer(attr_net_id, crop_pa, crop_va, kCropW, kCropH, kCropW))
            continue;

        NpuOutputInfo out;
        if (!npu.GetOutput(attr_net_id, 0, out)) continue;

        // Dequantize 3-D output
        float vals[kOutputDim];
        vendor_ai_cpu_util_fixed2float(
            reinterpret_cast<void*>(out.va), static_cast<HD_VIDEO_PXLFMT>(out.fmt),
            vals, out.scale, static_cast<INT32>(kOutputDim), out.zero_point);

        FaceAttributes attr;
        float sigmoid_gender = 1.0f / (1.0f + std::exp(-vals[0]));
        attr.is_male = (sigmoid_gender > 0.5f);
        attr.gender_confidence = attr.is_male ? sigmoid_gender : (1.0f - sigmoid_gender);
        attr.age = std::max(0, std::min(120, static_cast<int>(vals[2] * 100.0f)));
        results.push_back(attr);
    }
#else
    (void)faces; (void)yuv_data; (void)yuv_w; (void)yuv_h; (void)yuv_stride;
    (void)npu; (void)attr_net_id; (void)crop_pa; (void)crop_va;
#endif
    return results;
}

} // namespace ai
} // namespace ipcam
