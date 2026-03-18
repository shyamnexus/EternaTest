/**
 * @file aac_codec.h
 * @brief AAC-LC audio codec implementation using vo-aacenc
 *
 * AAC-LC (Low Complexity) encoder for IP camera audio.
 * Uses the Android-derived vo-aacenc library.
 *
 * AAC-LC patents expired 2017-2018 — royalty-free for commercial use.
 *
 * Typical usage for IP camera recording:
 * - 48kHz mono → 64 kbps AAC-LC (~6 KB/s, excellent speech quality)
 * - 8kHz mono  → 32 kbps AAC-LC (~4 KB/s, good speech quality)
 *
 * Usage:
 * @code
 * platform::AacCodec codec;
 * if (codec.Init(48000, 1, 64000)) {
 *     std::vector<uint8_t> aac_frame;
 *     int bytes = codec.Encode(pcm_samples, num_samples, aac_frame);
 *     // aac_frame contains ADTS-framed AAC data
 * }
 * @endcode
 */

#ifndef IPCAM_AAC_CODEC_H
#define IPCAM_AAC_CODEC_H

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>

namespace ipcam {
namespace platform {

/**
 * @brief AAC-LC Audio Encoder (using vo-aacenc)
 *
 * Encodes 16-bit PCM audio to AAC-LC with ADTS framing.
 * Thread-safe for single-producer usage (each instance is independent).
 */
class AacCodec {
public:
    AacCodec();
    ~AacCodec();

    // Non-copyable, movable
    AacCodec(const AacCodec&) = delete;
    AacCodec& operator=(const AacCodec&) = delete;
    AacCodec(AacCodec&&) noexcept;
    AacCodec& operator=(AacCodec&&) noexcept;

    /**
     * @brief Initialize the AAC encoder
     * @param sample_rate  Input sample rate (8000, 16000, 44100, 48000)
     * @param channels     Number of channels (1 = mono, 2 = stereo)
     * @param bitrate      Target bitrate in bps (e.g. 32000, 64000, 128000)
     * @return true on success
     */
    bool Init(int sample_rate, int channels, int bitrate);

    /**
     * @brief Shut down the encoder and release resources
     */
    void Shutdown();

    /**
     * @brief Check if the encoder is initialized
     */
    bool IsInitialized() const;

    /**
     * @brief Encode PCM samples to AAC
     *
     * Internally accumulates PCM until a full 1024-sample AAC frame can
     * be produced.  May output 0, 1, or multiple AAC frames per call.
     *
     * @param pcm_data     Pointer to 16-bit signed PCM samples (interleaved if stereo)
     * @param pcm_samples  Number of samples **per channel**
     * @param output       Output buffer — AAC frames are APPENDED
     * @return Total bytes appended, 0 if buffered (no output yet), -1 on error
     */
    int Encode(const int16_t* pcm_data, int pcm_samples,
               std::vector<uint8_t>& output);

    /**
     * @brief Flush any remaining buffered PCM
     *
     * Pads the last partial frame with silence and encodes it.
     * Call once at end-of-stream.
     *
     * @param output  Output buffer — final AAC frame is APPENDED
     * @return Bytes appended, 0 if nothing was buffered, -1 on error
     */
    int Flush(std::vector<uint8_t>& output);

    /**
     * @brief Encode PCM to AAC into a pre-allocated buffer
     * @note  Does NOT use the accumulation buffer.  Caller must provide
     *        exactly kFrameSamples per channel.
     * @param pcm_data     Input PCM samples
     * @param pcm_samples  Number of samples per channel
     * @param out_data     Output buffer (must be at least kMaxOutputSize bytes)
     * @param out_size     Output buffer capacity
     * @return Number of bytes written, 0 if more input needed, -1 on error
     */
    int EncodeBuffer(const int16_t* pcm_data, int pcm_samples,
                     uint8_t* out_data, size_t out_size);

    // ---- Accessors ----

    /** @brief AAC-LC frame size: always 1024 samples per channel */
    static constexpr int kFrameSamples = 1024;

    int  GetSampleRate() const;
    int  GetChannels()   const;
    int  GetBitrate()    const;

    /** @brief Conservative upper bound for one AAC frame output */
    static constexpr size_t kMaxOutputSize = 2048;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace platform
} // namespace ipcam

#endif // IPCAM_AAC_CODEC_H
