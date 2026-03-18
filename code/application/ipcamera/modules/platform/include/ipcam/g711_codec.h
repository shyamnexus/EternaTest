/**
 * @file g711_codec.h
 * @brief G.711 μ-law and A-law audio codec implementation
 *
 * This provides software-based G.711 encoding since the Novatek HDAL
 * kernel encoder only supports PCM. The audio pipeline captures PCM
 * from HDAL and encodes to G.711 in userspace when configured.
 *
 * G.711 is an ITU-T standard for audio companding:
 * - μ-law (PCMU): Used in North America and Japan (RTP payload type 0)
 * - A-law (PCMA): Used in Europe and rest of world (RTP payload type 8)
 *
 * Both provide 2:1 compression (16-bit PCM -> 8-bit encoded)
 */

#ifndef IPCAM_G711_CODEC_H
#define IPCAM_G711_CODEC_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace ipcam {
namespace platform {

/**
 * @brief G.711 codec type
 */
enum class G711Type {
    kNone = 0,      ///< No G.711 encoding (passthrough PCM)
    kUlaw = 1,      ///< μ-law encoding (PCMU)
    kAlaw = 2       ///< A-law encoding (PCMA)
};

/**
 * @brief G.711 Audio Codec
 *
 * Provides efficient G.711 μ-law and A-law encoding/decoding.
 * Uses lookup tables for fast encoding.
 *
 * Usage:
 * @code
 * G711Codec codec;
 * 
 * // Encode PCM to G.711 μ-law
 * std::vector<uint8_t> encoded = codec.Encode(pcm_data, pcm_size, G711Type::kUlaw);
 * 
 * // Or encode in-place to a pre-allocated buffer
 * codec.EncodeBuffer(pcm_ptr, sample_count, out_ptr, G711Type::kAlaw);
 * @endcode
 */
class G711Codec {
public:
    G711Codec() = default;
    ~G711Codec() = default;

    // Non-copyable
    G711Codec(const G711Codec&) = delete;
    G711Codec& operator=(const G711Codec&) = delete;

    /**
     * @brief Encode PCM samples to G.711
     * @param pcm_data Pointer to 16-bit signed PCM samples
     * @param pcm_bytes Size of PCM data in bytes
     * @param type G.711 encoding type (μ-law or A-law)
     * @return Encoded G.711 data (half the size of PCM)
     */
    static std::vector<uint8_t> Encode(const int16_t* pcm_data, size_t pcm_bytes, G711Type type);

    /**
     * @brief Encode PCM samples to G.711 in-place
     * @param pcm_data Pointer to 16-bit signed PCM samples
     * @param sample_count Number of samples to encode
     * @param out_data Output buffer (must be at least sample_count bytes)
     * @param type G.711 encoding type
     */
    static void EncodeBuffer(const int16_t* pcm_data, size_t sample_count, 
                             uint8_t* out_data, G711Type type);

    /**
     * @brief Decode G.711 to PCM samples
     * @param g711_data Pointer to G.711 encoded data
     * @param g711_bytes Size of G.711 data in bytes
     * @param type G.711 encoding type
     * @return Decoded 16-bit PCM data (double the size of G.711)
     */
    static std::vector<int16_t> Decode(const uint8_t* g711_data, size_t g711_bytes, G711Type type);

    /**
     * @brief Get G.711 type from codec name string
     * @param codec Codec name ("pcmu", "ulaw", "g711u", "pcma", "alaw", "g711a")
     * @return G711Type enum value
     */
    static G711Type GetTypeFromString(const std::string& codec);

    /**
     * @brief Check if a codec string represents G.711
     * @param codec Codec name
     * @return true if codec is G.711 μ-law or A-law
     */
    static bool IsG711Codec(const std::string& codec);

    /**
     * @brief Get RTP payload type for G.711 codec
     * @param type G711 type
     * @return RTP payload type (0 for μ-law, 8 for A-law, -1 for none)
     */
    static int GetRtpPayloadType(G711Type type);

    /**
     * @brief Signal conditioning for G.726 (ITU-T compliant path)
     * 
     * Per ITU-T G.726, the standard signal path is:
     *   Linear PCM (16-bit) → G.711 encode → G.711 decode (expand) → G.726
     * 
     * This applies the G.711 companding curve to shape quantization noise
     * and produces 14-bit (μ-law) or 13-bit (A-law) uniform PCM.
     * 
     * @param pcm_val Input 16-bit linear PCM sample
     * @param type G.711 type to use for conditioning (kUlaw recommended)
     * @return Conditioned sample for G.726 encoder input
     */
    static int16_t ConditionForG726(int16_t pcm_val, G711Type type = G711Type::kUlaw);

    /**
     * @brief Single sample μ-law encode (for external use)
     */
    static uint8_t EncodeSampleUlaw(int16_t pcm_val) { return LinearToUlaw(pcm_val); }
    
    /**
     * @brief Single sample μ-law decode (for external use)
     */
    static int16_t DecodeSampleUlaw(uint8_t ulaw_val) { return UlawToLinear(ulaw_val); }
    
    /**
     * @brief Single sample A-law encode (for external use)
     */
    static uint8_t EncodeSampleAlaw(int16_t pcm_val) { return LinearToAlaw(pcm_val); }
    
    /**
     * @brief Single sample A-law decode (for external use)
     */
    static int16_t DecodeSampleAlaw(uint8_t alaw_val) { return AlawToLinear(alaw_val); }

private:
    // Encoding functions
    static uint8_t LinearToUlaw(int16_t pcm_val);
    static uint8_t LinearToAlaw(int16_t pcm_val);
    
    // Decoding functions
    static int16_t UlawToLinear(uint8_t ulaw_val);
    static int16_t AlawToLinear(uint8_t alaw_val);

    // Constants
    static constexpr int kG711Bias = 0x84;      // Bias for μ-law linear code
    static constexpr int kG711Clip = 8159;      // Max μ-law clipping value

    // μ-law segment encoding table
    static const uint8_t kUlawEncodeTable[256];
};

} // namespace platform
} // namespace ipcam

#endif // IPCAM_G711_CODEC_H
