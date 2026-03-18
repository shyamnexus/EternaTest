/**
 * @file audio_encoder.cpp
 * @brief CreateAudioEncoder() factory — delegates to platform codecs.
 *
 * All concrete encoder implementations now reside in ipcam::platform:
 *   - G711Codec   (g711_codec.h / .cpp)
 *   - G726Codec   (g726_codec.h / .cpp)
 *   - AacCodec    (aac_codec.h  / .cpp)
 *
 * This file only provides the legacy CreateAudioEncoder() factory so
 * that any code still using the generic IAudioEncoder interface
 * continues to compile.
 */

#include "ipcam/audio_encoder.h"
#include "ipcam/g711_codec.h"
#include "ipcam/g726_codec.h"
#include "ipcam/aac_codec.h"
#include <spdlog/spdlog.h>
#include <cstring>

namespace ipcam {
namespace media {

// ============================================================================
// Thin IAudioEncoder wrappers around platform codecs
// ============================================================================

namespace {

// ---- G.711 μ-law wrapper ----
class G711UlawWrapper : public IAudioEncoder {
public:
    bool Init(int sample_rate, int channels, int /*bitrate*/) override {
        sample_rate_ = sample_rate; channels_ = channels; return true;
    }
    int Encode(const int16_t* pcm, int samples, std::vector<uint8_t>& out) override {
        int total = samples * channels_;
        out.resize(total);
        platform::G711Codec::EncodeBuffer(pcm, total, out.data(), platform::G711Type::kUlaw);
        return total;
    }
    int  GetFrameSamples() const override { return 160; }
    void Shutdown() override {}
    AudioCodec GetCodec() const override { return AudioCodec::G711_ULAW; }
private:
    int sample_rate_ = 0; int channels_ = 1;
};

// ---- G.711 A-law wrapper ----
class G711AlawWrapper : public IAudioEncoder {
public:
    bool Init(int sample_rate, int channels, int /*bitrate*/) override {
        sample_rate_ = sample_rate; channels_ = channels; return true;
    }
    int Encode(const int16_t* pcm, int samples, std::vector<uint8_t>& out) override {
        int total = samples * channels_;
        out.resize(total);
        platform::G711Codec::EncodeBuffer(pcm, total, out.data(), platform::G711Type::kAlaw);
        return total;
    }
    int  GetFrameSamples() const override { return 160; }
    void Shutdown() override {}
    AudioCodec GetCodec() const override { return AudioCodec::G711_ALAW; }
private:
    int sample_rate_ = 0; int channels_ = 1;
};

// ---- PCM passthrough ----
class PCMWrapper : public IAudioEncoder {
public:
    bool Init(int sr, int ch, int) override { sr_ = sr; ch_ = ch; return true; }
    int Encode(const int16_t* pcm, int samples, std::vector<uint8_t>& out) override {
        int bytes = samples * ch_ * sizeof(int16_t);
        out.resize(bytes);
        std::memcpy(out.data(), pcm, bytes);
        return bytes;
    }
    int  GetFrameSamples() const override { return 1024; }
    void Shutdown() override {}
    AudioCodec GetCodec() const override { return AudioCodec::PCM; }
private:
    int sr_ = 0; int ch_ = 1;
};

// ---- G.726 wrapper ----
class G726Wrapper : public IAudioEncoder {
public:
    explicit G726Wrapper(platform::G726BitRate br) : codec_(br), br_(br) {}
    bool Init(int, int, int) override { codec_.Reset(); return true; }
    int Encode(const int16_t* pcm, int samples, std::vector<uint8_t>& out) override {
        auto v = codec_.Encode(pcm, samples);
        out = std::move(v);
        return static_cast<int>(out.size());
    }
    int  GetFrameSamples() const override { return 160; }
    void Shutdown() override {}
    AudioCodec GetCodec() const override {
        switch (br_) {
            case platform::G726BitRate::k16kbps: return AudioCodec::G726_16;
            case platform::G726BitRate::k24kbps: return AudioCodec::G726_24;
            case platform::G726BitRate::k32kbps: return AudioCodec::G726_32;
            case platform::G726BitRate::k40kbps: return AudioCodec::G726_40;
        }
        return AudioCodec::G726_32;
    }
private:
    platform::G726Codec codec_;
    platform::G726BitRate br_;
};

// ---- AAC wrapper ----
class AacWrapper : public IAudioEncoder {
public:
    bool Init(int sr, int ch, int br) override { return codec_.Init(sr, ch, br); }
    int Encode(const int16_t* pcm, int samples, std::vector<uint8_t>& out) override {
        return codec_.Encode(pcm, samples, out);
    }
    int  GetFrameSamples() const override { return platform::AacCodec::kFrameSamples; }
    void Shutdown() override { codec_.Shutdown(); }
    AudioCodec GetCodec() const override { return AudioCodec::AAC; }
private:
    platform::AacCodec codec_;
};

} // anonymous namespace

// ============================================================================
// Factory
// ============================================================================
std::unique_ptr<IAudioEncoder> CreateAudioEncoder(AudioCodec codec) {
    switch (codec) {
        case AudioCodec::PCM:
            return std::make_unique<PCMWrapper>();
        case AudioCodec::G711_ULAW:
            return std::make_unique<G711UlawWrapper>();
        case AudioCodec::G711_ALAW:
            return std::make_unique<G711AlawWrapper>();
        case AudioCodec::G726_16:
            return std::make_unique<G726Wrapper>(platform::G726BitRate::k16kbps);
        case AudioCodec::G726_24:
            return std::make_unique<G726Wrapper>(platform::G726BitRate::k24kbps);
        case AudioCodec::G726_32:
            return std::make_unique<G726Wrapper>(platform::G726BitRate::k32kbps);
        case AudioCodec::G726_40:
            return std::make_unique<G726Wrapper>(platform::G726BitRate::k40kbps);
        case AudioCodec::AAC:
            return std::make_unique<AacWrapper>();
        case AudioCodec::ADPCM:
            return std::make_unique<G726Wrapper>(platform::G726BitRate::k32kbps);
        default:
            spdlog::warn("CreateAudioEncoder: Unknown codec {}, defaulting to G.711 μ-law",
                         static_cast<int>(codec));
            return std::make_unique<G711UlawWrapper>();
    }
}

} // namespace media
} // namespace ipcam

