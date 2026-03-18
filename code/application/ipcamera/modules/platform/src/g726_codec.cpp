/**
 * @file g726_codec.cpp
 * @brief ITU-T G.726 ADPCM codec implementation
 *
 * Based on SpanDSP / Sun Microsystems reference code (public domain).
 * Implements proper G.726-32 (4-bit, 32kbps) ADPCM encoding.
 */

#include "ipcam/g726_codec.h"
#include <cstring>
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace ipcam {
namespace platform {

// ============================================================================
// G.726-32 Tables (ITU-T G.726 specification)
// ============================================================================

// Quantizer decision levels (Table 9/G.726)
static const int qtab_726_32[7] = {
    -124, 80, 178, 246, 300, 349, 400
};

// Inverse quantizer - reconstructed dqln values (Table 10/G.726)
static const int g726_32_dqlntab[16] = {
    -2048,    4,  135,  213,  273,  323,  373,  425,
      425,  373,  323,  273,  213,  135,    4, -2048
};

// Adaptation speed control W(I) (Table 11/G.726)
static const int g726_32_witab[16] = {
     -384,   576,  1312,  2048,  3584,  6336, 11360, 35904,
    35904, 11360,  6336,  3584,  2048,  1312,   576,  -384
};

// Tone/transition detector F(I) (Table 12/G.726)
static const int g726_32_fitab[16] = {
    0x000, 0x000, 0x000, 0x200, 0x200, 0x200, 0x600, 0xE00,
    0xE00, 0x600, 0x200, 0x200, 0x200, 0x000, 0x000, 0x000
};

// ============================================================================
// Helper: Find top bit position (like CLZ but returns position)
// ============================================================================
static inline int top_bit(unsigned int x) {
    if (x == 0) return -1;
    int n = 0;
    if (x >= 0x10000) { n += 16; x >>= 16; }
    if (x >= 0x100)   { n += 8;  x >>= 8; }
    if (x >= 0x10)    { n += 4;  x >>= 4; }
    if (x >= 0x4)     { n += 2;  x >>= 2; }
    if (x >= 0x2)     { n += 1; }
    return n;
}

// ============================================================================
// Fixed-point multiply (fmult from G.726 spec)
// Multiplies 14-bit integer by floating-point representation
// ============================================================================
static int16_t fmult(int16_t an, int16_t srn) {
    int16_t anmag = (an > 0) ? an : ((-an) & 0x1FFF);
    int16_t anexp = (int16_t)(top_bit(anmag) - 5);
    int16_t anmant = (anmag == 0) ? 32 :
                     (anexp >= 0) ? (anmag >> anexp) : (anmag << -anexp);
    int16_t wanexp = anexp + ((srn >> 6) & 0xF) - 13;
    int16_t wanmant = (anmant * (srn & 0x3F) + 0x30) >> 4;
    int16_t retval = (wanexp >= 0) ? ((wanmant << wanexp) & 0x7FFF) : (wanmant >> -wanexp);
    return ((an ^ srn) < 0) ? -retval : retval;
}

// ============================================================================
// G726Codec Implementation
// ============================================================================

G726Codec::G726Codec(G726BitRate bitrate) : bitrate_(bitrate) {
    Reset();
}

void G726Codec::Reset() {
    InitState(enc_state_);
    InitState(dec_state_);
}

void G726Codec::InitState(G726State& s) {
    memset(&s, 0, sizeof(G726State));
    s.yl = 34816;  // Slow scale factor (Y(k) in spec)
    s.yu = 544;    // Fast scale factor
    s.sr[0] = 32;  // Reconstructed signal history (float format)
    s.sr[1] = 32;
    for (int i = 0; i < 6; i++) s.dq[i] = 32;
}

const int16_t* G726Codec::GetQuantTable() const { return nullptr; }

int G726Codec::GetQuantTableSize() const {
    switch (bitrate_) {
        case G726BitRate::k16kbps: return 4;
        case G726BitRate::k24kbps: return 8;
        case G726BitRate::k32kbps: return 16;
        case G726BitRate::k40kbps: return 32;
        default: return 16;
    }
}

size_t G726Codec::GetEncodedSize(size_t sample_count) const {
    return (sample_count * GetBitsPerSample() + 7) / 8;
}

size_t G726Codec::GetDecodedSamples(size_t encoded_bytes) const {
    return (encoded_bytes * 8) / GetBitsPerSample();
}

// ============================================================================
// Core G.726 Functions
// ============================================================================

// Compute 6-zero predictor contribution
int16_t G726Codec::Predictor(G726State& s) {
    int sezi = fmult(s.b[0] >> 2, s.dq[0]);
    for (int i = 1; i < 6; i++)
        sezi += fmult(s.b[i] >> 2, s.dq[i]);
    return (int16_t)sezi;
}

// Compute 2-pole predictor contribution
static int16_t predictor_pole(G726State& s) {
    return fmult(s.a[1] >> 2, s.sr[1]) + fmult(s.a[0] >> 2, s.sr[0]);
}

// Compute quantization step size
static int step_size(G726State& s) {
    int y, dif, al;
    if (s.ap >= 256) return s.yu;
    y = s.yl >> 6;
    dif = s.yu - y;
    al = s.ap >> 2;
    if (dif > 0)
        y += (dif * al) >> 6;
    else if (dif < 0)
        y += (dif * al + 0x3F) >> 6;
    return y;
}

// Quantize difference signal to code
int16_t G726Codec::Quantize(int16_t d, int16_t y, const int16_t*, int) {
    int16_t dqm = (int16_t)abs(d);
    int16_t exp = (int16_t)(top_bit(dqm >> 1) + 1);
    int16_t mant = ((dqm << 7) >> exp) & 0x7F;
    int16_t dl = (exp << 7) + mant;
    int16_t dln = dl - (int16_t)(y >> 2);
    
    // Search for codeword
    int i;
    for (i = 0; i < 7; i++) {
        if (dln < qtab_726_32[i]) break;
    }
    
    if (d < 0) {
        // Negative: take 1's complement
        return (int16_t)((7 << 1) + 1 - i);
    }
    return (int16_t)i;
}

// Reconstruct difference from code (inverse quantize)
int16_t G726Codec::Reconstruct(int sign, int16_t dqln, int16_t y) {
    int16_t dql = (int16_t)(dqln + (y >> 2));
    if (dql < 0) return (sign) ? -0x8000 : 0;
    
    int16_t dex = (dql >> 7) & 15;
    int16_t dqt = 128 + (dql & 127);
    int16_t dq = (dqt << 7) >> (14 - dex);
    return (sign) ? (dq - 0x8000) : dq;
}

// Update adaptive predictor state
void G726Codec::UpdateState(G726State& s, int16_t code, int16_t dq_val, int16_t sr, int16_t y) {
    int16_t mag = (int16_t)(dq_val & 0x7FFF);
    int dq = dq_val;
    
    // Compute dqsez correctly per ITU-T spec:
    // dqsez = sr + (sezi >> 1) - se, where se = (sezi + pole) >> 1
    // This simplifies to: sr - pole/2
    int pole = fmult(s.a[0] >> 2, s.sr[0]) + fmult(s.a[1] >> 2, s.sr[1]);
    int sezi = fmult(s.b[0] >> 2, s.dq[0]);
    for (int i = 1; i < 6; i++)
        sezi += fmult(s.b[i] >> 2, s.dq[i]);
    int se = (sezi + pole) >> 1;
    int dqsez = sr + (sezi >> 1) - se;
    
    int pk0 = (dqsez < 0) ? 1 : 0;
    
    // Tone/transition detection
    int16_t ylint = (int16_t)(s.yl >> 15);
    int16_t ylfrac = (int16_t)((s.yl >> 10) & 0x1F);
    int16_t thr = (ylint > 9) ? (31 << 10) : ((32 + ylfrac) << ylint);
    int16_t dqthr = (thr + (thr >> 1)) >> 1;
    bool tr = s.td && (mag > dqthr);
    
    // Update scale factor (FUNCTW & FILTD)
    int wi = g726_32_witab[code & 0x0F];
    s.yu = (int16_t)(y + ((wi - y) >> 5));
    if (s.yu < 544) s.yu = 544;
    else if (s.yu > 5120) s.yu = 5120;
    
    // Slow scale factor (FILTE)
    s.yl += s.yu + ((-s.yl) >> 6);
    
    // Update predictor coefficients
    int a2p = s.a[1];  // Declare outside if block for tone detection
    if (tr) {
        // Reset for modem signal
        s.a[0] = s.a[1] = 0;
        for (int i = 0; i < 6; i++) s.b[i] = 0;
    } else {
        // Adapt A coefficients
        int pks1 = pk0 ^ s.pk[0];
        a2p = s.a[1] - (s.a[1] >> 7);
        if (dqsez != 0) {
            int fa1 = pks1 ? s.a[0] : -s.a[0];
            if (fa1 < -8191) a2p -= 0x100;
            else if (fa1 > 8191) a2p += 0xFF;
            else a2p += fa1 >> 5;
            
            if (pk0 ^ s.pk[1]) {
                if (a2p <= -12160) a2p = -12288;
                else if (a2p >= 12416) a2p = 12288;
                else a2p -= 0x80;
            } else {
                if (a2p <= -12416) a2p = -12288;
                else if (a2p >= 12160) a2p = 12288;
                else a2p += 0x80;
            }
        }
        s.a[1] = (int16_t)a2p;
        
        s.a[0] -= s.a[0] >> 8;
        if (dqsez != 0) {
            if (pks1 == 0) s.a[0] += 192;
            else s.a[0] -= 192;
        }
        int a1ul = 15360 - a2p;
        if (a1ul < 0) a1ul = 0;
        if (s.a[0] < -a1ul) s.a[0] = -a1ul;
        else if (s.a[0] > a1ul) s.a[0] = a1ul;
        
        // Adapt B coefficients
        for (int i = 0; i < 6; i++) {
            s.b[i] -= s.b[i] >> 8;
            if (dq & 0x7FFF) {
                if ((dq ^ s.dq[i]) >= 0) s.b[i] += 128;
                else s.b[i] -= 128;
            }
        }
    }
    
    // Update history
    for (int i = 5; i > 0; i--) s.dq[i] = s.dq[i-1];
    
    // Convert dq to float format for history
    if (mag == 0) {
        s.dq[0] = (dq >= 0) ? 0x20 : 0xFC20;
    } else {
        int16_t exp = (int16_t)(top_bit(mag) + 1);
        s.dq[0] = (dq >= 0) ? ((exp << 6) + ((mag << 6) >> exp))
                            : ((exp << 6) + ((mag << 6) >> exp) - 0x400);
    }
    
    s.sr[1] = s.sr[0];
    if (sr == 0) {
        s.sr[0] = 0x20;
    } else if (sr > 0) {
        int16_t exp = (int16_t)(top_bit(sr) + 1);
        s.sr[0] = (int16_t)((exp << 6) + ((sr << 6) >> exp));
    } else if (sr > -32768) {
        int smag = -sr;
        int16_t exp = (int16_t)(top_bit(smag) + 1);
        s.sr[0] = (exp << 6) + ((smag << 6) >> exp) - 0x400;
    } else {
        s.sr[0] = (int16_t)0xFC20;
    }
    
    s.pk[1] = s.pk[0];
    s.pk[0] = (int16_t)pk0;
    
    // Tone detection
    if (tr) s.td = false;
    else if (a2p < -11776) s.td = true;
    else s.td = false;
    
    // Adaptation speed control
    int fi = g726_32_fitab[code & 0x0F];
    s.dms += ((int16_t)fi - s.dms) >> 5;
    s.dml += (((int16_t)(fi << 2) - s.dml) >> 7);
    
    if (tr) s.ap = 256;
    else if (y < 1536) s.ap += (0x200 - s.ap) >> 4;
    else if (s.td) s.ap += (0x200 - s.ap) >> 4;
    else if (abs((s.dms << 2) - s.dml) >= (s.dml >> 3)) s.ap += (0x200 - s.ap) >> 4;
    else s.ap += (-s.ap) >> 4;
}

// ============================================================================
// Main Encode/Decode Functions
// ============================================================================

uint8_t G726Codec::EncodeSample(int16_t pcm, G726State& s) {
    // Linearize to 14-bit (G.726 uses 14-bit linear input)
    int16_t sl = pcm >> 2;
    
    // Compute prediction
    int16_t sezi = Predictor(s);
    int16_t sei = sezi + predictor_pole(s);
    int16_t se = sei >> 1;
    int16_t d = sl - se;
    
    // Quantize
    int y = step_size(s);
    int16_t i = Quantize(d, (int16_t)y, nullptr, 0);
    int16_t dq = Reconstruct(i & 8, g726_32_dqlntab[i], (int16_t)y);
    
    // Reconstruct signal
    int16_t sr = (dq < 0) ? (se - (dq & 0x3FFF)) : (se + dq);
    
    // Update state
    int16_t dqsez = sr + (sezi >> 1) - se;
    UpdateState(s, i, dq, sr, (int16_t)y);
    
    return (uint8_t)(i & 0x0F);
}

int16_t G726Codec::DecodeSample(uint8_t code, G726State& s) {
    code &= 0x0F;
    
    int16_t sezi = Predictor(s);
    int16_t sei = sezi + predictor_pole(s);
    
    int y = step_size(s);
    int16_t dq = Reconstruct(code & 8, g726_32_dqlntab[code], (int16_t)y);
    
    int16_t se = sei >> 1;
    int16_t sr = (dq < 0) ? (se - (dq & 0x3FFF)) : (se + dq);
    
    int16_t dqsez = sr + (sezi >> 1) - se;
    UpdateState(s, code, dq, sr, (int16_t)y);
    
    // Output is 16-bit linear (shift left 2 to restore full range)
    return (int16_t)(sr << 2);
}

// ============================================================================
// Public API
// ============================================================================

std::vector<uint8_t> G726Codec::Encode(const int16_t* pcm, size_t count) {
    size_t sz = GetEncodedSize(count);
    std::vector<uint8_t> out(sz, 0);
    EncodeBuffer(pcm, count, out.data(), sz);
    return out;
}

size_t G726Codec::EncodeBuffer(const int16_t* pcm, size_t count,
                                uint8_t* out, size_t out_size) {
    int bits = GetBitsPerSample();
    int mask = (1 << bits) - 1;
    size_t required = GetEncodedSize(count);
    if (out_size < required) return 0;
    
    memset(out, 0, required);
    
    int bit_pos = 0;
    for (size_t i = 0; i < count; i++) {
        uint8_t code = EncodeSample(pcm[i], enc_state_) & mask;
        int byte_idx = bit_pos / 8;
        int bit_off = bit_pos % 8;
        
        if (bit_off + bits <= 8) {
            out[byte_idx] |= code << bit_off;
        } else {
            out[byte_idx] |= (code << bit_off) & 0xFF;
            out[byte_idx + 1] |= code >> (8 - bit_off);
        }
        bit_pos += bits;
    }
    return required;
}

std::vector<int16_t> G726Codec::Decode(const uint8_t* data, size_t bytes) {
    size_t count = GetDecodedSamples(bytes);
    std::vector<int16_t> out(count);
    
    int bits = GetBitsPerSample();
    int mask = (1 << bits) - 1;
    
    int bit_pos = 0;
    for (size_t i = 0; i < count; i++) {
        int byte_idx = bit_pos / 8;
        int bit_off = bit_pos % 8;
        uint8_t code;
        if (bit_off + bits <= 8) {
            code = (data[byte_idx] >> bit_off) & mask;
        } else {
            code = ((data[byte_idx] >> bit_off) |
                    (data[byte_idx + 1] << (8 - bit_off))) & mask;
        }
        out[i] = DecodeSample(code, dec_state_);
        bit_pos += bits;
    }
    return out;
}

G726BitRate G726Codec::GetBitRateFromString(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower.find("16") != std::string::npos) return G726BitRate::k16kbps;
    if (lower.find("24") != std::string::npos) return G726BitRate::k24kbps;
    if (lower.find("40") != std::string::npos) return G726BitRate::k40kbps;
    return G726BitRate::k32kbps;
}

bool G726Codec::IsG726Codec(const std::string& codec) {
    std::string lower = codec;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower.find("g726") != std::string::npos ||
           lower.find("adpcm") != std::string::npos;
}

} // namespace platform
} // namespace ipcam
