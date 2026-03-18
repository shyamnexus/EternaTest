/**
 * @file g726_codec.h
 * @brief G.726 ADPCM audio codec implementation
 *
 * ITU-T G.726 Adaptive Differential Pulse Code Modulation (ADPCM)
 * Provides 2:1 to 4:1 compression of 8kHz 16-bit PCM audio.
 *
 * Supported bit rates:
 * - G.726-16: 2 bits/sample, 16 kbps (4:1 compression)
 * - G.726-24: 3 bits/sample, 24 kbps (2.67:1 compression)
 * - G.726-32: 4 bits/sample, 32 kbps (2:1 compression) - STANDARD
 * - G.726-40: 5 bits/sample, 40 kbps (1.6:1 compression)
 *
 * All G.726 patents have expired - royalty free for commercial use.
 */

#ifndef IPCAM_G726_CODEC_H
#define IPCAM_G726_CODEC_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace ipcam {
namespace platform {

/**
 * @brief G.726 bit rate modes
 */
enum class G726BitRate {
    k16kbps = 2,    ///< 2 bits per sample, 16 kbps
    k24kbps = 3,    ///< 3 bits per sample, 24 kbps
    k32kbps = 4,    ///< 4 bits per sample, 32 kbps (standard/default)
    k40kbps = 5     ///< 5 bits per sample, 40 kbps
};

/**
 * @brief G.726 ADPCM Codec State
 * 
 * Maintains encoder/decoder state for adaptive quantization
 */
struct G726State {
    int32_t yl;         ///< Slow quantizer scale factor
    int16_t yu;         ///< Fast quantizer scale factor
    int16_t dms;        ///< Short-term average of F(I)
    int16_t dml;        ///< Long-term average of F(I)
    int16_t ap;         ///< Pole adaptation speed
    int16_t a[2];       ///< Coefficients of pole portion of prediction filter
    int16_t b[6];       ///< Coefficients of zero portion of prediction filter
    int16_t pk[2];      ///< Signs of previous two samples
    int16_t dq[6];      ///< Previous quantized differences
    int16_t sr[2];      ///< Previous reconstructed samples
    int16_t td;         ///< Tone detect
};

/**
 * @brief G.726 ADPCM Audio Codec
 *
 * Provides G.726 encoding and decoding with configurable bit rates.
 * Maintains state for the adaptive prediction filter.
 *
 * Usage:
 * @code
 * G726Codec codec(G726BitRate::k32kbps);
 * 
 * // Encode PCM to G.726
 * std::vector<uint8_t> encoded = codec.Encode(pcm_data, pcm_samples);
 * 
 * // Decode G.726 to PCM
 * std::vector<int16_t> decoded = codec.Decode(encoded.data(), encoded.size());
 * @endcode
 */
class G726Codec {
public:
    /**
     * @brief Construct G.726 codec with specified bit rate
     * @param bitrate Encoding bit rate (default: 32 kbps)
     */
    explicit G726Codec(G726BitRate bitrate = G726BitRate::k32kbps);
    ~G726Codec() = default;

    /**
     * @brief Reset encoder/decoder state
     */
    void Reset();

    /**
     * @brief Encode PCM samples to G.726
     * @param pcm_data Pointer to 16-bit signed PCM samples (8kHz)
     * @param sample_count Number of samples to encode
     * @return Encoded G.726 data (packed bits)
     */
    std::vector<uint8_t> Encode(const int16_t* pcm_data, size_t sample_count);

    /**
     * @brief Encode PCM samples to G.726 in-place
     * @param pcm_data Pointer to 16-bit signed PCM samples
     * @param sample_count Number of samples to encode
     * @param out_data Output buffer
     * @param out_size Output buffer size
     * @return Number of bytes written
     */
    size_t EncodeBuffer(const int16_t* pcm_data, size_t sample_count,
                        uint8_t* out_data, size_t out_size);

    /**
     * @brief Decode G.726 to PCM samples
     * @param g726_data Pointer to G.726 encoded data
     * @param g726_bytes Size of G.726 data in bytes
     * @return Decoded 16-bit PCM samples
     */
    std::vector<int16_t> Decode(const uint8_t* g726_data, size_t g726_bytes);

    /**
     * @brief Get current bit rate
     */
    G726BitRate GetBitRate() const { return bitrate_; }

    /**
     * @brief Get bits per sample for current mode
     */
    int GetBitsPerSample() const { return static_cast<int>(bitrate_); }

    /**
     * @brief Calculate encoded size for given sample count
     */
    size_t GetEncodedSize(size_t sample_count) const;

    /**
     * @brief Calculate decoded sample count for given encoded size
     */
    size_t GetDecodedSamples(size_t encoded_bytes) const;

    /**
     * @brief Get G.726 bit rate from string
     * @param str Bitrate string ("16", "24", "32", "40", "g726-32", etc.)
     * @return Bit rate enum value
     */
    static G726BitRate GetBitRateFromString(const std::string& str);

    /**
     * @brief Check if codec string is G.726
     * @param codec Codec name
     * @return true if codec is G.726 variant
     */
    static bool IsG726Codec(const std::string& codec);

private:
    G726BitRate bitrate_;
    G726State enc_state_;
    G726State dec_state_;

    // Core ADPCM functions
    int16_t Predictor(G726State& state);
    int16_t Quantize(int16_t d, int16_t y, const int16_t* table, int size);
    int16_t Reconstruct(int sign, int16_t dqln, int16_t y);
    void UpdateState(G726State& state, int16_t code, int16_t dq, int16_t sr, int16_t y);

    // Encode single sample
    uint8_t EncodeSample(int16_t pcm, G726State& state);
    
    // Decode single code
    int16_t DecodeSample(uint8_t code, G726State& state);

    // Quantizer tables (depend on bit rate)
    const int16_t* GetQuantTable() const;
    int GetQuantTableSize() const;

    // Initialize state to reset values
    void InitState(G726State& state);
};

} // namespace platform
} // namespace ipcam

#endif // IPCAM_G726_CODEC_H
