/**
 * @file audio_classification.h
 * @brief Audio Event Classification — YAMNet CCTV 20-class
 *
 * Computes a log-mel spectrogram from 16 kHz mono PCM audio, runs the
 * yamnet_cctv model on the NPU, and returns the top detected event.
 *
 * NOTE: This module processes audio samples pushed into it.  The audio
 * capture pathway (ALSA / HDAL audio) must be set up externally and feed
 * PCM frames via Process().
 */
#ifndef IPCAM_AUDIO_CLASSIFICATION_H_
#define IPCAM_AUDIO_CLASSIFICATION_H_

#include <vector>
#include <string>
#include <cstdint>

namespace ipcam {
namespace ai {

class NpuInference;

struct AudioClassificationConfig {
    bool enabled = false;
    int sample_rate = 16000;
    int fft_size = 400;
    int hop_size = 160;
    int num_mels = 64;
    int num_frames = 96;
    float detection_threshold = 0.3f;
};

struct AudioEvent {
    int class_id = 0;
    std::string class_name;
    float confidence = 0.0f;
};

class AudioClassificationEngine {
public:
    bool Init(const AudioClassificationConfig& config);
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    bool IsEnabled() const { return initialized_ && config_.enabled; }

    /// Process audio segment (16-bit PCM, 16 kHz mono, ~0.96 s / 15360 samples)
    /// mel_pa/va point to a pre-allocated buffer >= 96*64*sizeof(float) bytes.
    std::vector<AudioEvent> Process(
        const int16_t* samples, int num_samples,
        NpuInference& npu, int net_id,
        uintptr_t mel_pa, uintptr_t mel_va);

    static const char* ClassName(int class_id);

private:
    static constexpr int kNumClasses = 20;
    static constexpr int kMelBins    = 64;
    static constexpr int kTimeFrames = 96;

    AudioClassificationConfig config_;
    bool initialized_ = false;

    std::vector<float> window_;  ///< Hann window (fft_size)
    std::vector<float> mel_fb_;  ///< Mel filterbank (num_mels * (fft_size/2+1))
    std::vector<float> mel_buf_; ///< Mel spectrogram buffer (kTimeFrames * kMelBins)

    void ComputeMelSpectrogram(const int16_t* samples, int num_samples, float* output);
    void InitHannWindow();
    void InitMelFilterbank();
    static void Softmax(float* data, int n);
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_AUDIO_CLASSIFICATION_H_
