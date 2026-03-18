/**
 * @file audio_classification.cpp
 * @brief Audio Event Classification — YAMNet CCTV 20-class
 *
 * Ported from ai3_features/14_audio_classification demo.
 * Computes log-mel spectrogram (96×64) → NPU inference → softmax → top event.
 *
 * NOTE: Audio capture is NOT handled here.  The caller must provide
 * 16-bit PCM mono 16 kHz samples.
 */
#include "ipcam/audio_classification.h"
#include "ipcam/npu_inference.h"

#include <spdlog/spdlog.h>
#include <cmath>
#include <algorithm>
#include <cstring>

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

static const char* const kClassNames[] = {
    "background", "speech", "baby_cry", "scream", "dog_bark",
    "glass_break", "gunshot", "explosion", "siren", "alarm",
    "car_horn", "vehicle", "door", "crash", "fire",
    "footstep", "rain", "thunder", "wind", "engine"
};

bool AudioClassificationEngine::Init(const AudioClassificationConfig& config) {
    config_ = config;
    InitHannWindow();
    InitMelFilterbank();
    mel_buf_.resize(kTimeFrames * kMelBins, 0.0f);
    initialized_ = true;
    spdlog::info("AudioClassificationEngine initialized ({}kHz, fft={}, {}mels, threshold={:.2f})",
                 config_.sample_rate / 1000, config_.fft_size, config_.num_mels,
                 config_.detection_threshold);
    return true;
}

void AudioClassificationEngine::Shutdown() {
    initialized_ = false;
    mel_buf_.clear();
    window_.clear();
    mel_fb_.clear();
}

std::vector<AudioEvent> AudioClassificationEngine::Process(
    const int16_t* samples, int num_samples,
    NpuInference& npu, int net_id,
    uintptr_t mel_pa, uintptr_t mel_va)
{
    std::vector<AudioEvent> results;
#if VENDOR_AI3_ENABLED
    if (!initialized_ || net_id < 0 || !samples || !mel_va) return results;

    // Compute mel spectrogram into mel_buf_
    auto* output = reinterpret_cast<float*>(mel_va);
    ComputeMelSpectrogram(samples, num_samples, output);

    // Flush to device
    hd_common_mem_cache_sync(reinterpret_cast<void*>(mel_va),
                             kTimeFrames * kMelBins * sizeof(float),
                             HD_COMMON_MEM_DMA_TO_DEVICE);

    // Inference — input is [96,64] float32 feature map
    // NpuInference::Infer expects NV12 but for audio we use feature mode.
    // The demo uses vendor_ai3_net_set() directly for FEAT input.
    // For now, fall through to the generic Infer path which sets width=64,height=96.
    if (!npu.Infer(net_id, mel_pa, mel_va, kMelBins, kTimeFrames, kMelBins * sizeof(float)))
        return results;

    NpuOutputInfo out;
    if (!npu.GetOutput(net_id, 0, out)) return results;

    float logits[kNumClasses];
    vendor_ai_cpu_util_fixed2float(
        reinterpret_cast<void*>(out.va), static_cast<HD_VIDEO_PXLFMT>(out.fmt),
        logits, out.scale, static_cast<INT32>(kNumClasses), out.zero_point);

    Softmax(logits, kNumClasses);

    // Collect events above threshold
    for (int c = 0; c < kNumClasses; c++) {
        if (logits[c] >= config_.detection_threshold && c != 0) {  // skip background
            AudioEvent evt;
            evt.class_id = c;
            evt.class_name = kClassNames[c];
            evt.confidence = logits[c];
            results.push_back(evt);
        }
    }

    // Sort by confidence descending
    std::sort(results.begin(), results.end(),
              [](const AudioEvent& a, const AudioEvent& b) { return a.confidence > b.confidence; });
#else
    (void)samples; (void)num_samples; (void)npu; (void)net_id;
    (void)mel_pa; (void)mel_va;
#endif
    return results;
}

void AudioClassificationEngine::ComputeMelSpectrogram(
    const int16_t* samples, int num_samples, float* output)
{
    int fft_size = config_.fft_size;
    int hop_size = config_.hop_size;
    int n_frames = kTimeFrames;
    int n_mels = kMelBins;
    int half_fft = fft_size / 2 + 1;

    // Zero-fill output
    std::memset(output, 0, n_frames * n_mels * sizeof(float));

    // Temporary FFT buffers
    std::vector<float> frame_buf(fft_size, 0.0f);
    std::vector<float> fft_re(half_fft, 0.0f);
    std::vector<float> fft_im(half_fft, 0.0f);
    std::vector<float> mag(half_fft, 0.0f);

    for (int t = 0; t < n_frames; t++) {
        int offset = t * hop_size;

        // Extract windowed frame
        for (int i = 0; i < fft_size; i++) {
            int idx = offset + i;
            float sample = (idx < num_samples) ? (samples[idx] / 32768.0f) : 0.0f;
            frame_buf[i] = sample * window_[i];
        }

        // DFT (real input, half spectrum) — O(N²) for simplicity
        // For production, consider radix-2 FFT — but fft_size=400 is small
        for (int k = 0; k < half_fft; k++) {
            float re = 0.0f, im = 0.0f;
            for (int n = 0; n < fft_size; n++) {
                float angle = -2.0f * M_PI * k * n / fft_size;
                re += frame_buf[n] * std::cos(angle);
                im += frame_buf[n] * std::sin(angle);
            }
            mag[k] = std::sqrt(re * re + im * im);
        }

        // Apply mel filterbank
        for (int m = 0; m < n_mels; m++) {
            float sum = 0.0f;
            for (int k = 0; k < half_fft; k++) {
                sum += mag[k] * mel_fb_[m * half_fft + k];
            }
            output[t * n_mels + m] = std::log(sum + 0.001f);
        }
    }
}

void AudioClassificationEngine::InitHannWindow() {
    int n = config_.fft_size;
    window_.resize(n);
    for (int i = 0; i < n; i++)
        window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / n));
}

void AudioClassificationEngine::InitMelFilterbank() {
    int n_mels = config_.num_mels;
    int sr = config_.sample_rate;
    int half_fft = config_.fft_size / 2 + 1;

    mel_fb_.resize(n_mels * half_fft, 0.0f);

    // Mel scale conversion
    auto hz_to_mel = [](float hz) { return 2595.0f * std::log10(1.0f + hz / 700.0f); };
    auto mel_to_hz = [](float mel) { return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f); };

    float mel_low = hz_to_mel(0.0f);
    float mel_high = hz_to_mel(sr / 2.0f);

    std::vector<float> mel_points(n_mels + 2);
    for (int i = 0; i < n_mels + 2; i++)
        mel_points[i] = mel_to_hz(mel_low + (mel_high - mel_low) * i / (n_mels + 1));

    // Convert to FFT bin indices
    std::vector<int> bins(n_mels + 2);
    for (int i = 0; i < n_mels + 2; i++)
        bins[i] = static_cast<int>(std::floor((config_.fft_size + 1) * mel_points[i] / sr));

    // Triangular filters
    for (int m = 0; m < n_mels; m++) {
        for (int k = bins[m]; k < bins[m + 1] && k < half_fft; k++) {
            float denom = static_cast<float>(bins[m + 1] - bins[m]);
            if (denom > 0)
                mel_fb_[m * half_fft + k] = (k - bins[m]) / denom;
        }
        for (int k = bins[m + 1]; k < bins[m + 2] && k < half_fft; k++) {
            float denom = static_cast<float>(bins[m + 2] - bins[m + 1]);
            if (denom > 0)
                mel_fb_[m * half_fft + k] = (bins[m + 2] - k) / denom;
        }
    }
}

void AudioClassificationEngine::Softmax(float* data, int n) {
    float max_val = *std::max_element(data, data + n);
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        data[i] = std::exp(data[i] - max_val);
        sum += data[i];
    }
    for (int i = 0; i < n; i++) data[i] /= sum;
}

const char* AudioClassificationEngine::ClassName(int class_id) {
    if (class_id >= 0 && class_id < kNumClasses) return kClassNames[class_id];
    return "unknown";
}

} // namespace ai
} // namespace ipcam
