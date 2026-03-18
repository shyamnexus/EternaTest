/**
 * @file g711_codec.cpp
 * @brief G.711 μ-law and A-law audio codec implementation
 *
 * Implements ITU-T G.711 encoding/decoding algorithms.
 * Based on reference implementations from ITU-T G.191 software tools.
 */

#include "ipcam/g711_codec.h"
#include <algorithm>
#include <cstring>

namespace ipcam {
namespace platform {

// ============================================================================
// Static Lookup Table
// ============================================================================

// μ-law segment encoding table - maps linear magnitude to segment number
const uint8_t G711Codec::kUlawEncodeTable[256] = {
     0,    0,    1,    1,    2,    2,    2,    2,    3,    3,    3,    3,    3,    3,    3,    3,
     4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
     5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,
     5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,
     6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
     6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
     6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
     6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7
};

// ============================================================================
// μ-law Encoding (ITU-T G.711)
// ============================================================================

uint8_t G711Codec::LinearToUlaw(int16_t pcm_val) {
    int sign, exponent, mantissa;
    uint8_t ulawbyte;

    // Get the sign and the absolute value
    sign = (pcm_val >> 8) & 0x80;
    if (sign != 0) {
        pcm_val = static_cast<int16_t>(-pcm_val);
    }

    // Clip the magnitude
    if (pcm_val > kG711Clip) {
        pcm_val = kG711Clip;
    }

    // Add bias for linear code
    pcm_val = static_cast<int16_t>(pcm_val + kG711Bias);

    // Find the segment number (exponent)
    exponent = kUlawEncodeTable[(pcm_val >> 7) & 0xFF];

    // Combine the sign, segment (exponent), and mantissa
    mantissa = (pcm_val >> (exponent + 3)) & 0x0F;
    ulawbyte = static_cast<uint8_t>(~(sign | (exponent << 4) | mantissa));

    return ulawbyte;
}

int16_t G711Codec::UlawToLinear(uint8_t ulaw_val) {
    // Complement to get the actual value
    ulaw_val = ~ulaw_val;

    int sign = ulaw_val & 0x80;
    int exponent = (ulaw_val >> 4) & 0x07;
    int mantissa = ulaw_val & 0x0F;

    // Reconstruct the linear value
    int linear = ((mantissa << 3) + kG711Bias) << exponent;
    linear -= kG711Bias;

    return static_cast<int16_t>(sign ? -linear : linear);
}

// ============================================================================
// A-law Encoding (ITU-T G.711)
// ============================================================================

uint8_t G711Codec::LinearToAlaw(int16_t pcm_val) {
    int sign, exponent, mantissa;
    uint8_t alawbyte;

    // Get the sign (bit 15)
    sign = ((~pcm_val) >> 8) & 0x80;
    if (sign == 0) {
        pcm_val = static_cast<int16_t>(-pcm_val);
    }

    // Clip to 13-bit magnitude (A-law uses 13-bit dynamic range)
    if (pcm_val > 32635) {
        pcm_val = 32635;
    }

    // Find the exponent and mantissa
    if (pcm_val >= 256) {
        exponent = 1;
        int temp = pcm_val;
        while (temp >= 512 && exponent < 7) {
            temp >>= 1;
            exponent++;
        }
        mantissa = (pcm_val >> (exponent + 3)) & 0x0F;
    } else {
        exponent = 0;
        mantissa = pcm_val >> 4;
    }

    // Combine sign, exponent, and mantissa, then toggle even bits (XOR 0x55)
    alawbyte = static_cast<uint8_t>(sign | (exponent << 4) | mantissa);
    alawbyte ^= 0x55;

    return alawbyte;
}

int16_t G711Codec::AlawToLinear(uint8_t alaw_val) {
    // Toggle even bits back
    alaw_val ^= 0x55;

    int sign = alaw_val & 0x80;
    int exponent = (alaw_val >> 4) & 0x07;
    int mantissa = alaw_val & 0x0F;

    int linear;
    if (exponent == 0) {
        linear = (mantissa << 4) + 8;
    } else {
        linear = ((mantissa << 4) + 264) << (exponent - 1);
    }

    return static_cast<int16_t>(sign ? linear : -linear);
}

// ============================================================================
// Public API
// ============================================================================

std::vector<uint8_t> G711Codec::Encode(const int16_t* pcm_data, size_t pcm_bytes, G711Type type) {
    if (type == G711Type::kNone || pcm_data == nullptr || pcm_bytes == 0) {
        return {};
    }

    size_t sample_count = pcm_bytes / sizeof(int16_t);
    std::vector<uint8_t> encoded(sample_count);

    EncodeBuffer(pcm_data, sample_count, encoded.data(), type);

    return encoded;
}

void G711Codec::EncodeBuffer(const int16_t* pcm_data, size_t sample_count, 
                              uint8_t* out_data, G711Type type) {
    if (pcm_data == nullptr || out_data == nullptr || sample_count == 0) {
        return;
    }

    if (type == G711Type::kUlaw) {
        for (size_t i = 0; i < sample_count; ++i) {
            out_data[i] = LinearToUlaw(pcm_data[i]);
        }
    } else if (type == G711Type::kAlaw) {
        for (size_t i = 0; i < sample_count; ++i) {
            out_data[i] = LinearToAlaw(pcm_data[i]);
        }
    }
}

std::vector<int16_t> G711Codec::Decode(const uint8_t* g711_data, size_t g711_bytes, G711Type type) {
    if (type == G711Type::kNone || g711_data == nullptr || g711_bytes == 0) {
        return {};
    }

    std::vector<int16_t> decoded(g711_bytes);

    if (type == G711Type::kUlaw) {
        for (size_t i = 0; i < g711_bytes; ++i) {
            decoded[i] = UlawToLinear(g711_data[i]);
        }
    } else if (type == G711Type::kAlaw) {
        for (size_t i = 0; i < g711_bytes; ++i) {
            decoded[i] = AlawToLinear(g711_data[i]);
        }
    }

    return decoded;
}

G711Type G711Codec::GetTypeFromString(const std::string& codec) {
    if (codec == "pcmu" || codec == "ulaw" || codec == "g711u" || codec == "PCMU") {
        return G711Type::kUlaw;
    } else if (codec == "pcma" || codec == "alaw" || codec == "g711a" || codec == "PCMA") {
        return G711Type::kAlaw;
    }
    return G711Type::kNone;
}

bool G711Codec::IsG711Codec(const std::string& codec) {
    return GetTypeFromString(codec) != G711Type::kNone;
}

int G711Codec::GetRtpPayloadType(G711Type type) {
    switch (type) {
        case G711Type::kUlaw: return 0;   // PCMU
        case G711Type::kAlaw: return 8;   // PCMA
        default: return -1;
    }
}

int16_t G711Codec::ConditionForG726(int16_t pcm_val, G711Type type) {
    // ITU-T G.726 standard signal path:
    // Linear PCM → G.711 encode → G.711 decode (expand) → G.726 ADPCM
    //
    // This applies companding which shapes quantization noise optimally
    // for speech and produces the expected dynamic range for G.726.
    
    if (type == G711Type::kUlaw) {
        // PCM → μ-law (8-bit) → uniform PCM (14-bit effective)
        uint8_t ulaw = LinearToUlaw(pcm_val);
        return UlawToLinear(ulaw);
    } else if (type == G711Type::kAlaw) {
        // PCM → A-law (8-bit) → uniform PCM (13-bit effective)
        uint8_t alaw = LinearToAlaw(pcm_val);
        return AlawToLinear(alaw);
    }
    
    // No conditioning - direct pass (not ITU-T compliant)
    return pcm_val;
}

} // namespace platform
} // namespace ipcam
