/**
 * @file lpr.cpp
 * @brief License Plate Recognition — YOLOv11-LP + Indian LPRNet OCR
 *
 * Ported from ai3_features/10_lpr demo.
 */
#include "ipcam/lpr.h"
#include "ipcam/npu_inference.h"

#include <spdlog/spdlog.h>
#include <cmath>
#include <cstring>
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

const char LprEngine::kCharset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

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

bool LprEngine::Init(const LprConfig& config) {
    config_ = config;
    logit_accum_.fill(0.0f);
    logit_count_ = 0;
    frames_without_det_ = 0;
    initialized_ = true;
    spdlog::info("LprEngine initialized");
    return true;
}

void LprEngine::Shutdown() {
    initialized_ = false;
}

std::vector<PlateDetection> LprEngine::Process(
    NpuInference& npu, int det_net_id, int ocr_net_id,
    const uint8_t* yuv_data, uint32_t yuv_w, uint32_t yuv_h, uint32_t yuv_stride,
    uintptr_t crop_pa, uintptr_t crop_va)
{
    std::vector<PlateDetection> results;
#if VENDOR_AI3_ENABLED
    if (!initialized_ || det_net_id < 0) return results;

    // Stage 1: detect plates
    auto plates = DetectPlates(npu, det_net_id);
    if (plates.empty()) {
        frames_without_det_++;
        if (frames_without_det_ > 10) {
            logit_accum_.fill(0.0f);
            logit_count_ = 0;
        }
        return results;
    }
    frames_without_det_ = 0;

    // Stage 2: OCR each plate
    for (const auto& p : plates) {
        PlateDetection det;
        det.x1 = p.x1; det.y1 = p.y1;
        det.x2 = p.x2; det.y2 = p.y2;
        det.confidence = p.conf;
        if (ocr_net_id >= 0 && yuv_data && crop_va) {
            det.plate_text = RunOcr(npu, ocr_net_id, yuv_data, yuv_w, yuv_h, yuv_stride,
                                    p.x1, p.y1, p.x2, p.y2, crop_pa, crop_va);
        }
        results.push_back(det);
    }
#else
    (void)npu; (void)det_net_id; (void)ocr_net_id;
    (void)yuv_data; (void)yuv_w; (void)yuv_h; (void)yuv_stride;
    (void)crop_pa; (void)crop_va;
#endif
    return results;
}

std::vector<LprEngine::DetBox> LprEngine::DetectPlates(NpuInference& npu, int det_net_id) {
    std::vector<DetBox> dets;
#if VENDOR_AI3_ENABLED
    NpuOutputInfo out;
    if (!npu.GetOutput(det_net_id, 0, out)) return dets;

    // Output is [5, 8400] CHW: cx, cy, w, h, conf
    uint32_t total = out.channel * out.width * out.height;
    std::vector<float> fbuf(total);
    vendor_ai_cpu_util_fixed2float(
        reinterpret_cast<void*>(out.va), static_cast<HD_VIDEO_PXLFMT>(out.fmt),
        fbuf.data(), out.scale, static_cast<INT32>(total), out.zero_point);

    int anchors = static_cast<int>(out.width * out.height);
    if (anchors > kMaxAnchors) anchors = kMaxAnchors;

    // Decode: CHW layout — channel c, anchor a → fbuf[c * anchors + a]
    for (int a = 0; a < anchors; a++) {
        float conf = fbuf[4 * anchors + a];
        if (conf < config_.det_conf_threshold) continue;
        float cx = fbuf[0 * anchors + a];
        float cy = fbuf[1 * anchors + a];
        float w  = fbuf[2 * anchors + a];
        float h  = fbuf[3 * anchors + a];
        DetBox d;
        d.x1 = (cx - w / 2.0f);
        d.y1 = (cy - h / 2.0f);
        d.x2 = (cx + w / 2.0f);
        d.y2 = (cy + h / 2.0f);
        d.conf = conf;
        dets.push_back(d);
    }

    // NMS
    std::sort(dets.begin(), dets.end(), [](const DetBox& a, const DetBox& b){ return a.conf > b.conf; });
    std::vector<DetBox> kept;
    std::vector<bool> suppressed(dets.size(), false);
    for (size_t i = 0; i < dets.size() && static_cast<int>(kept.size()) < kMaxDet; i++) {
        if (suppressed[i]) continue;
        kept.push_back(dets[i]);
        for (size_t j = i + 1; j < dets.size(); j++) {
            if (suppressed[j]) continue;
            float ix1 = std::max(dets[i].x1, dets[j].x1);
            float iy1 = std::max(dets[i].y1, dets[j].y1);
            float ix2 = std::min(dets[i].x2, dets[j].x2);
            float iy2 = std::min(dets[i].y2, dets[j].y2);
            float iw = std::max(0.0f, ix2 - ix1);
            float ih = std::max(0.0f, iy2 - iy1);
            float inter = iw * ih;
            float ai = (dets[i].x2 - dets[i].x1) * (dets[i].y2 - dets[i].y1);
            float aj = (dets[j].x2 - dets[j].x1) * (dets[j].y2 - dets[j].y1);
            if (inter / (ai + aj - inter + 1e-6f) > config_.det_nms_threshold)
                suppressed[j] = true;
        }
    }
    return kept;
#else
    return dets;
#endif
}

std::string LprEngine::RunOcr(NpuInference& npu, int ocr_net_id,
                               const uint8_t* yuv_data, uint32_t yuv_w, uint32_t yuv_h,
                               uint32_t yuv_stride,
                               float x1, float y1, float x2, float y2,
                               uintptr_t crop_pa, uintptr_t crop_va)
{
#if VENDOR_AI3_ENABLED
    // Expand by margin
    float pw = x2 - x1, ph = y2 - y1;
    float mx = pw * config_.plate_margin, my = ph * config_.plate_margin;
    x1 = std::max(0.0f, x1 - mx);
    y1 = std::max(0.0f, y1 - my);
    x2 = std::min(1.0f, x2 + mx);
    y2 = std::min(1.0f, y2 + my);

    uint32_t bx = static_cast<uint32_t>(x1 * yuv_w) & ~1u;
    uint32_t by = static_cast<uint32_t>(y1 * yuv_h) & ~1u;
    uint32_t bw = (static_cast<uint32_t>((x2 - x1) * yuv_w) + 1) & ~1u;
    uint32_t bh = (static_cast<uint32_t>((y2 - y1) * yuv_h) + 1) & ~1u;
    if (bw < 8 || bh < 4) return "";
    if (bx + bw > yuv_w) bw = yuv_w - bx;
    if (by + bh > yuv_h) bh = yuv_h - by;

    const uint8_t* src_y  = yuv_data;
    const uint8_t* src_uv = yuv_data + yuv_stride * yuv_h;
    auto* dst_y  = reinterpret_cast<uint8_t*>(crop_va);
    auto* dst_uv = dst_y + kCropW * kCropH;
    NV12CropResize(src_y, src_uv, yuv_w, yuv_h, yuv_stride,
                   bx, by, bw, bh, dst_y, dst_uv, kCropW, kCropH);

    hd_common_mem_cache_sync(reinterpret_cast<void*>(crop_va),
                             kCropW * kCropH * 3 / 2, HD_COMMON_MEM_DMA_TO_DEVICE);

    if (!npu.Infer(ocr_net_id, crop_pa, crop_va, kCropW, kCropH, kCropW))
        return "";

    NpuOutputInfo out;
    if (!npu.GetOutput(ocr_net_id, 0, out)) return "";

    // Output [37, 18] CHW
    uint32_t total = out.channel * out.width * out.height;
    std::vector<float> logits(total);
    vendor_ai_cpu_util_fixed2float(
        reinterpret_cast<void*>(out.va), static_cast<HD_VIDEO_PXLFMT>(out.fmt),
        logits.data(), out.scale, static_cast<INT32>(total), out.zero_point);

    // Accumulate logits for averaging
    if (logit_count_ < config_.logit_avg_frames) {
        for (size_t i = 0; i < logits.size() && i < logit_accum_.size(); i++)
            logit_accum_[i] += logits[i];
        logit_count_++;
    } else {
        // Shift: subtract oldest contribution (approximate by running average)
        float factor = static_cast<float>(logit_count_ - 1) / logit_count_;
        for (size_t i = 0; i < logit_accum_.size(); i++)
            logit_accum_[i] = logit_accum_[i] * factor + logits[i];
    }

    // Decode from averaged logits
    std::vector<float> avg(kNumClasses * kSeqLen);
    float inv = 1.0f / std::max(1, logit_count_);
    for (int i = 0; i < kNumClasses * kSeqLen; i++)
        avg[i] = logit_accum_[i] * inv;

    return CtcGreedyDecode(avg.data());
#else
    (void)npu; (void)ocr_net_id; (void)yuv_data; (void)yuv_w; (void)yuv_h;
    (void)yuv_stride; (void)x1; (void)y1; (void)x2; (void)y2;
    (void)crop_pa; (void)crop_va;
    return "";
#endif
}

std::string LprEngine::CtcGreedyDecode(const float* logits) const {
    // logits layout: CHW [kNumClasses, kSeqLen] — class-major
    std::string result;
    int prev_cls = kBlankIdx;
    for (int t = 0; t < kSeqLen; t++) {
        int best_cls = 0;
        float best_val = logits[0 * kSeqLen + t];
        float second_val = -1e9f;
        for (int c = 1; c < kNumClasses; c++) {
            float v = logits[c * kSeqLen + t];
            if (v > best_val) {
                second_val = best_val;
                best_val = v;
                best_cls = c;
            } else if (v > second_val) {
                second_val = v;
            }
        }
        // Confidence filter: require margin between best and second
        if (best_cls != kBlankIdx && best_cls != prev_cls &&
            (best_val - second_val) >= config_.ocr_confidence_margin) {
            if (best_cls >= 0 && best_cls < kNumClasses - 1) {
                result += kCharset[best_cls];
            }
        }
        prev_cls = best_cls;
    }
    return result;
}

} // namespace ai
} // namespace ipcam
