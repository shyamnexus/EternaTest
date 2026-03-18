#pragma once
/**
 * @file audio_encoder.h
 * @brief Audio encoder interface and factory.
 *
 * The concrete encoder implementations now live in the platform module:
 *   - ipcam/g711_codec.h  (G.711 μ-law / A-law)
 *   - ipcam/g726_codec.h  (G.726 ADPCM)
 *   - ipcam/aac_codec.h   (AAC-LC via vo-aacenc)
 *
 * This header retains the abstract IAudioEncoder interface and the
 * CreateAudioEncoder() factory so that any code using the generic
 * interface continues to compile unchanged.
 */

#include "audio_control.h"
#include <vector>
#include <memory>
#include <cstdint>

namespace ipcam {
namespace media {

// ============================================================================
// Audio Encoder Interface
// ============================================================================
class IAudioEncoder {
public:
    virtual ~IAudioEncoder() = default;

    virtual bool Init(int sample_rate, int channels, int bitrate) = 0;
    virtual int  Encode(const int16_t* pcm_data, int pcm_samples,
                        std::vector<uint8_t>& output) = 0;
    virtual int  GetFrameSamples() const = 0;
    virtual void Shutdown() = 0;
    virtual AudioCodec GetCodec() const = 0;
};

// ============================================================================
// Factory Function
// ============================================================================
/**
 * @brief Create an audio encoder for the specified codec.
 *
 * Internally creates the appropriate platform codec wrapper.
 * Supported: PCM, G711_ULAW, G711_ALAW, G726_*, AAC
 */
std::unique_ptr<IAudioEncoder> CreateAudioEncoder(AudioCodec codec);

} // namespace media
} // namespace ipcam

