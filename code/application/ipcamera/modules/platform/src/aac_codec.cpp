/**
 * @file aac_codec.cpp
 * @brief AAC-LC encoder implementation using vo-aacenc (Android VisualOn)
 *
 * vo-aacenc is a lightweight AAC-LC encoder originally from Android.
 * It produces ADTS-framed AAC output suitable for MP4 muxing or streaming.
 *
 * CPU cost on Cortex-A53 @ 800MHz:
 *   48kHz mono  64kbps ≈ 1-2% CPU
 *    8kHz mono  32kbps ≈ <1% CPU
 */

#include "ipcam/aac_codec.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <cstdlib>

// vo-aacenc headers
extern "C" {
#include <vo-aacenc/voAAC.h>
#include <vo-aacenc/cmnMemory.h>
}

namespace ipcam {
namespace platform {

// ============================================================================
// vo-aacenc memory callbacks
// ============================================================================
static VO_U32 aac_mem_alloc(VO_S32 /*uID*/, VO_MEM_INFO* pMemInfo) {
    if (!pMemInfo) return 1;
    pMemInfo->VBuffer = std::malloc(pMemInfo->Size);
    return pMemInfo->VBuffer ? 0 : 1;
}

static VO_U32 aac_mem_free(VO_S32 /*uID*/, VO_PTR pMem) {
    std::free(pMem);
    return 0;
}

static VO_U32 aac_mem_set(VO_S32 /*uID*/, VO_PTR pBuff, VO_U8 uValue, VO_U32 uSize) {
    std::memset(pBuff, uValue, uSize);
    return 0;
}

static VO_U32 aac_mem_copy(VO_S32 /*uID*/, VO_PTR pDest, VO_PTR pSrc, VO_U32 uSize) {
    std::memcpy(pDest, pSrc, uSize);
    return 0;
}

static VO_U32 aac_mem_check(VO_S32 /*uID*/, VO_PTR /*pBuff*/, VO_U32 /*uSize*/) {
    return 0;
}

// ============================================================================
// Impl (pimpl)
// ============================================================================
struct AacCodec::Impl {
    VO_AUDIO_CODECAPI codec_api{};
    VO_HANDLE         handle = nullptr;
    VO_MEM_OPERATOR   mem_operator{};
    AACENC_PARAM      params{};
    std::vector<uint8_t> output_buf;

    // Internal PCM accumulation buffer.
    // vo-aacenc does NOT accumulate across SetInputData calls, so we
    // must feed it exactly kFrameSamples (1024) samples at a time.
    std::vector<int16_t> pcm_accum;

    bool initialized = false;
    int  sample_rate  = 0;
    int  channels     = 0;
    int  bitrate      = 0;

    // Encode exactly kFrameSamples from pcm_accum into output.
    // Returns bytes written, or -1 on error.
    int EncodeOnce(const int16_t* samples, int n_samples,
                   std::vector<uint8_t>& output) {
        VO_CODECBUFFER input_data{};
        VO_CODECBUFFER output_data{};
        VO_AUDIO_OUTPUTINFO output_info{};

        input_data.Buffer = reinterpret_cast<VO_PBYTE>(const_cast<int16_t*>(samples));
        input_data.Length  = static_cast<VO_U32>(n_samples * channels * sizeof(int16_t));

        output_data.Buffer = output_buf.data();
        output_data.Length = static_cast<VO_U32>(output_buf.size());

        if (codec_api.SetInputData(handle, &input_data) != VO_ERR_NONE) {
            return -1;
        }

        VO_U32 ret = codec_api.GetOutputData(handle, &output_data, &output_info);
        if (ret == VO_ERR_INPUT_BUFFER_SMALL) {
            return 0;  // Should not happen when we feed 1024
        }
        if (ret != VO_ERR_NONE) {
            spdlog::error("AacCodec: GetOutputData failed (err=0x{:08x})", ret);
            return -1;
        }

        size_t prev = output.size();
        output.insert(output.end(),
                      output_buf.begin(),
                      output_buf.begin() + output_data.Length);
        return static_cast<int>(output_data.Length);
    }
};

// ============================================================================
// Construction / destruction / move
// ============================================================================
AacCodec::AacCodec()  : impl_(std::make_unique<Impl>()) {}
AacCodec::~AacCodec() { Shutdown(); }

AacCodec::AacCodec(AacCodec&&) noexcept = default;
AacCodec& AacCodec::operator=(AacCodec&&) noexcept = default;

// ============================================================================
// Init / Shutdown
// ============================================================================
bool AacCodec::Init(int sample_rate, int channels, int bitrate) {
    if (impl_->initialized) {
        Shutdown();
    }

    // Setup memory operator
    impl_->mem_operator.Alloc = aac_mem_alloc;
    impl_->mem_operator.Free  = aac_mem_free;
    impl_->mem_operator.Set   = aac_mem_set;
    impl_->mem_operator.Copy  = aac_mem_copy;
    impl_->mem_operator.Check = aac_mem_check;

    VO_CODEC_INIT_USERDATA user_data{};
    user_data.memflag = VO_IMF_USERMEMOPERATOR;
    user_data.memData = &impl_->mem_operator;

    // Get AAC encoder API
    if (voGetAACEncAPI(&impl_->codec_api) != VO_ERR_NONE) {
        spdlog::error("AacCodec: voGetAACEncAPI failed");
        return false;
    }

    // Create encoder instance
    if (impl_->codec_api.Init(&impl_->handle, VO_AUDIO_CodingAAC, &user_data) != VO_ERR_NONE) {
        spdlog::error("AacCodec: Init failed");
        return false;
    }

    // Configure encoder parameters
    impl_->params.sampleRate = sample_rate;
    impl_->params.bitRate    = bitrate;
    impl_->params.nChannels  = channels;
    impl_->params.adtsUsed   = 1;  // ADTS framing (needed for MP4 ADTS stripping & RTSP)

    if (impl_->codec_api.SetParam(impl_->handle, VO_PID_AAC_ENCPARAM, &impl_->params) != VO_ERR_NONE) {
        spdlog::error("AacCodec: SetParam failed (rate={} ch={} br={})",
                      sample_rate, channels, bitrate);
        impl_->codec_api.Uninit(impl_->handle);
        impl_->handle = nullptr;
        return false;
    }

    // Pre-allocate output buffer (conservative)
    impl_->output_buf.resize(kMaxOutputSize);

    impl_->sample_rate = sample_rate;
    impl_->channels    = channels;
    impl_->bitrate     = bitrate;
    impl_->initialized = true;

    spdlog::info("AacCodec: Initialized AAC-LC encoder (rate={} ch={} br={})",
                 sample_rate, channels, bitrate);
    return true;
}

void AacCodec::Shutdown() {
    if (impl_ && impl_->initialized) {
        impl_->codec_api.Uninit(impl_->handle);
        impl_->handle      = nullptr;
        impl_->initialized = false;
        spdlog::debug("AacCodec: Shut down");
    }
}

bool AacCodec::IsInitialized() const {
    return impl_ && impl_->initialized;
}

// ============================================================================
// Encode (with internal accumulation)
// ============================================================================
int AacCodec::Encode(const int16_t* pcm_data, int pcm_samples,
                     std::vector<uint8_t>& output) {
    if (!impl_->initialized) return -1;

    // Append incoming PCM to the accumulation buffer
    impl_->pcm_accum.insert(impl_->pcm_accum.end(),
                            pcm_data,
                            pcm_data + (pcm_samples * impl_->channels));

    const int frame_samples = kFrameSamples * impl_->channels;  // samples per AAC frame
    int total_bytes = 0;

    // Encode as many full 1024-sample frames as we can
    while (static_cast<int>(impl_->pcm_accum.size()) >= frame_samples) {
        int n = impl_->EncodeOnce(impl_->pcm_accum.data(), kFrameSamples, output);
        if (n < 0) return -1;
        total_bytes += n;

        // Remove consumed samples
        impl_->pcm_accum.erase(impl_->pcm_accum.begin(),
                               impl_->pcm_accum.begin() + frame_samples);
    }

    return total_bytes;
}

// ============================================================================
// Flush (drain remaining buffered PCM)
// ============================================================================
int AacCodec::Flush(std::vector<uint8_t>& output) {
    if (!impl_->initialized) return -1;
    if (impl_->pcm_accum.empty()) return 0;

    const int frame_samples = kFrameSamples * impl_->channels;

    // Pad with silence to a full frame
    impl_->pcm_accum.resize(frame_samples, 0);

    int n = impl_->EncodeOnce(impl_->pcm_accum.data(), kFrameSamples, output);
    impl_->pcm_accum.clear();
    return n;
}

// ============================================================================
// EncodeBuffer (pre-allocated output)
// ============================================================================
int AacCodec::EncodeBuffer(const int16_t* pcm_data, int pcm_samples,
                           uint8_t* out_data, size_t out_size) {
    if (!impl_->initialized) return -1;

    VO_CODECBUFFER input_data{};
    VO_CODECBUFFER output_data{};
    VO_AUDIO_OUTPUTINFO output_info{};

    input_data.Buffer = reinterpret_cast<VO_PBYTE>(const_cast<int16_t*>(pcm_data));
    input_data.Length = static_cast<VO_U32>(pcm_samples * impl_->channels * sizeof(int16_t));

    output_data.Buffer = reinterpret_cast<VO_PBYTE>(out_data);
    output_data.Length = static_cast<VO_U32>(out_size);

    if (impl_->codec_api.SetInputData(impl_->handle, &input_data) != VO_ERR_NONE) {
        return -1;
    }

    VO_U32 ret = impl_->codec_api.GetOutputData(impl_->handle, &output_data, &output_info);
    if (ret != VO_ERR_NONE) {
        if (ret == VO_ERR_INPUT_BUFFER_SMALL) {
            return 0;
        }
        spdlog::error("AacCodec: EncodeBuffer failed (err=0x{:08x})", ret);
        return -1;
    }

    return static_cast<int>(output_data.Length);
}

// ============================================================================
// Accessors
// ============================================================================
int AacCodec::GetSampleRate() const { return impl_ ? impl_->sample_rate : 0; }
int AacCodec::GetChannels()   const { return impl_ ? impl_->channels   : 0; }
int AacCodec::GetBitrate()    const { return impl_ ? impl_->bitrate    : 0; }

} // namespace platform
} // namespace ipcam
