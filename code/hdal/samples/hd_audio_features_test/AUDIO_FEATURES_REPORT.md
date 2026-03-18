# Audio Features Test Report

**Date:** January 27, 2026  
**Platform:** Novatek NS02302 IP Camera  
**Libraries:** libagc.a, libaudfilt.a (userspace audio processing)

---

## Overview

This report documents the testing and validation of audio processing features for the IP camera platform. The goal was to integrate AGC (Automatic Gain Control), High-Pass Filter (HPF), Notch Filter, and Noise Gate into a demo that allows A/B comparison of processed vs raw audio.

---

## Test Configuration

| Parameter | Value |
|-----------|-------|
| Sample Rate | 48000 Hz |
| Channels | Mono |
| Bit Depth | 16-bit signed |
| Frame Size | 1024 samples |

---

## Features Tested

### 1. AGC (Automatic Gain Control) ✅ Working

**Library:** `libagc.a`

**SDK Default Parameters:**
| Parameter | Value | Description |
|-----------|-------|-------------|
| Target Level | -6 dB | Output target level |
| Max Gain | +20 dB | Maximum amplification |
| Min Gain | -10 dB | Minimum attenuation |
| Attack Time | 100 ms | Response to loud sounds |
| Decay Time | 200 ms | Recovery to quiet sounds |
| Noise Gate | -50 dB | Below this is considered noise |

**API Usage:**
```c
#include "agc/agc_api.h"

// Initialize
agc_init();
agc_set_target_level(-6.0f);
agc_set_max_gain(20.0f);
agc_set_min_gain(-10.0f);
agc_set_attack_time(100);
agc_set_decay_time(200);
agc_set_noise_gate(-50.0f);
agc_open();

// Process (per frame)
agc_run(samples, sample_count);
```

**Result:** Works well with SDK defaults. Custom tuning caused audio cutting artifacts - recommend using defaults.

---

### 2. High-Pass Filter (HPF) ✅ Working

**Library:** `libaudfilt.a`

**Parameters:**
| Parameter | Value | Description |
|-----------|-------|-------------|
| Frequency | Configurable (e.g., 100 Hz) | Cutoff frequency |
| Q Factor | 0.5 | Gentler rolloff |
| Type | High-pass | Removes low frequencies |

**API Usage (LP64/64-bit):**
```c
#include "AudioFilter/audlib_filt.h"

// Initialize
audlib_filt_init();
audlib_filt_set_samplerate(48000);
audlib_filt_set_type(FILT_TYPE_HIGHPASS);
audlib_filt_set_freq(100);  // 100 Hz cutoff
audlib_filt_set_Q(0.5f);    // Gentle rolloff
audlib_filt_open();

// Process (per frame) - CRITICAL: pass BYTES not samples!
audlib_filt_run((long)samples, sample_count * 2);  // * 2 for bytes
```

**⚠️ Critical Bug Fixed:**
The LP64 API expects **byte count**, not sample count:
```c
// WRONG - causes audio cutting:
audlib_filt_run((long)samples, sample_count);

// CORRECT:
audlib_filt_run((long)samples, sample_count * 2);
```

**Result:** Works well after bug fix. Removes low-frequency rumble effectively.

---

### 3. Notch Filter ✅ Working

**Library:** `libaudfilt.a`

**SDK Default Parameters:**
| Parameter | Value | Description |
|-----------|-------|-------------|
| Frequency | 60 Hz (or 50 Hz for EU) | Target frequency to remove |
| Q Factor | 5 | Bandwidth of notch |
| Gain | -40 dB | Attenuation at center frequency |
| Type | Notch | Removes specific frequency |

**API Usage (LP64/64-bit):**
```c
// Initialize for notch
audlib_filt_init();
audlib_filt_set_samplerate(48000);
audlib_filt_set_type(FILT_TYPE_NOTCH);
audlib_filt_set_freq(60);    // 60 Hz (power line hum)
audlib_filt_set_Q(5.0f);     // SDK default
audlib_filt_set_gain(-40);   // SDK default: -40 dB
audlib_filt_open();

// Process
audlib_filt_run((long)samples, sample_count * 2);
```

**⚠️ Parameter Warning:**
Aggressive settings (Q=30, gain=-60dB) killed the signal. Use SDK defaults (Q=5, gain=-40dB).

**Result:** Effectively removes 60Hz hum. Signal attenuated to ~0.3x but still audible. Combine with AGC to restore level.

---

### 4. Noise Gate ⏳ Not Fully Tested

Part of AGC library. The `agc_set_noise_gate()` parameter controls when the AGC considers input as noise vs signal.

---

## Test Results Summary

| Combination | Result | Peak Ratio | Notes |
|-------------|--------|------------|-------|
| AGC only | ✅ Pass | ~2-3x | SDK defaults work well |
| HPF only | ✅ Pass | ~1x | No gain, just filtering |
| AGC + HPF | ✅ Pass | ~3x | Recommended combination |
| Notch only | ✅ Pass | 0.3x | Attenuates but audible |
| AGC + Notch | ✅ Pass | 2.7x | AGC compensates for notch |
| **AGC + HPF + Notch** | ✅ Pass | **3.2x** | **Full chain working** |

---

## Recommended Processing Pipeline

```
Audio Input → HPF (100Hz) → Notch (60Hz) → AGC → Output
```

**Rationale:** Filter first, then amplify. This prevents AGC from boosting unwanted low-frequency noise.

---

## Demo Application Usage

The `audio_live_demo` application records audio and saves both raw and processed versions for A/B comparison.

### Build
```bash
cd code/hdal/samples/hd_audio_features_test
make
```

### Command Line Options
```
--agc           Enable AGC with SDK defaults
--hpf <hz>      Enable high-pass filter at specified frequency
--notch <hz>    Enable notch filter at specified frequency
--noisegate <dB> Set noise gate threshold (default: -50dB)
--duration <sec> Recording duration (default: 10 seconds)
```

### Example Commands
```bash
# AGC only
./audio_live_demo --agc --duration 10

# HPF only (100Hz cutoff)
./audio_live_demo --hpf 100 --duration 10

# AGC + HPF (recommended)
./audio_live_demo --agc --hpf 100 --duration 10

# Notch filter for 60Hz hum removal
./audio_live_demo --notch 60 --duration 10

# Full chain (AGC + HPF + Notch)
./audio_live_demo --agc --hpf 100 --notch 60 --duration 10
```

### Output Files
- `/mnt/sd/audio_raw_48000Hz.pcm` - Unprocessed audio
- `/mnt/sd/audio_processed_48000Hz.pcm` - Processed audio

### Playback
```bash
# On device
aplay -f S16_LE -r 48000 -c 1 /mnt/sd/audio_processed_48000Hz.pcm

# On PC (after adb pull)
sox -t raw -r 48000 -b 16 -c 1 -e signed-integer audio_processed_48000Hz.pcm output.wav
aplay output.wav
```

---

## Lessons Learned

1. **Use SDK defaults for AGC** - Custom tuning caused audio artifacts
2. **Filter API takes bytes, not samples** - Critical bug on LP64 platform
3. **Notch filter Q and gain matter** - Aggressive settings kill the signal
4. **Filter before AGC** - Prevents amplifying noise
5. **Test combinations incrementally** - Isolate issues to specific features

---

## Files

| File | Description |
|------|-------------|
| `audio_live_demo.c` | Main demo application |
| `Makefile` | Build configuration |
| `AUDIO_FEATURES_REPORT.md` | This report |

---

## Available Audio Processing Libraries

The Novatek SDK provides three userspace audio processing libraries:

| Library | Size | Contains | Location |
|---------|------|----------|----------|
| `libaec.a` | 1.1 MB | **AEC** (Echo Cancellation) + **NS** (Noise Suppression) | `code/lib/output/` |
| `libagc.a` | 12 KB | **AGC** (Auto Gain Control) | `code/lib/output/` |
| `libaudfilt.a` | 46 KB | **HPF, Notch, EQ** (Audio Filters) | `code/lib/output/` |

### Library Details

#### 1. libaec.a - Echo Cancellation & Noise Suppression

**Header:** `code/lib/include/aec/audlib_aec.h`

Contains two modules:
- **AEC** (`audlib_aec_*`) - Acoustic Echo Cancellation for two-way audio
- **NS** (`audlib_ns_*`) - Noise Suppression (reduces background noise)

**AEC API:**
```c
#include "aec/audlib_aec.h"

// Open and configure
audlib_aec_open();
audlib_aec_set_config(AEC_CONFIG_ID_SAMPLERATE, 8000);
audlib_aec_set_config(AEC_CONFIG_ID_RECORD_CH_NO, 1);
audlib_aec_set_config(AEC_CONFIG_ID_PLAYBACK_CH_NO, 1);
audlib_aec_set_config(AEC_CONFIG_ID_NOISE_CANEL_EN, TRUE);   // Built-in NS
audlib_aec_set_config(AEC_CONFIG_ID_NOISE_CANCEL_LVL, -20);  // -3 to -40 dB
audlib_aec_set_config(AEC_CONFIG_ID_ECHO_CANCEL_LVL, -50);   // -30 to -60 dB

// Buffer setup (required!)
int buf_size = audlib_aec_get_required_buffer_size(AEC_BUFINFO_ID_INTERNAL);
void* buf = malloc(buf_size);
audlib_aec_set_config(AEC_CONFIG_ID_BUF_ADDR, (INT32)(uintptr_t)buf);
audlib_aec_set_config(AEC_CONFIG_ID_BUF_SIZE, buf_size);

audlib_aec_init();

// Process (needs both playback and record streams!)
#ifdef _LP64
audlib_aec_run(record_addr, playback_addr, sample_count);
#else
AEC_BITSTREAM aec_io = {
    .bitstream_buffer_play_in = playback_buf,
    .bitstream_buffer_record_in = record_buf,
    .bitstram_buffer_out = output_buf,
    .bitstram_buffer_length = sample_count  // Must be multiple of 1024
};
audlib_aec_run(&aec_io);
#endif

audlib_aec_close();
```

**NS API (Standalone Noise Suppression) - Working with Low-Level API:**

The high-level `audlib_ns_run()` wrapper has issues on this platform. Use the low-level `AUD_NS_*` API instead, which is what `uvc_cam.c` uses:

```c
#include "aec/audlib_aec.h"

/* Low-level NS API declarations (from libaec.a) */
typedef struct {
    INT32 s32FrameSize;
    INT32 s32ChannelNum;
    INT32 s32SamplingRate;
} ST_AUD_NS_INFO;

typedef struct {
    UINT32 u32InternalBufSize;
    UINT32 u32InBufSize;
    UINT32 u32OutBufSize;
} ST_AUD_NS_RTN;

typedef enum {
    EN_AUD_NS_BANK_SCALE = 0,      // 0=less CPU, 1=linear
    EN_AUD_NS_NOISE_SUPPRESS       // Noise reduction level
} EN_AUD_NS_PARAM;

extern void AUD_NS_PreInit(ST_AUD_NS_INFO *pInfo, ST_AUD_NS_RTN *pRtn);
extern void AUD_NS_Init(void *pBuf, UINT32 BufSize);
extern void AUD_NS_Run(short *pIn, short *pOut);
extern void AUD_NS_SetParam(EN_AUD_NS_PARAM param, void *pValue);

/* Initialize NS */
ST_AUD_NS_INFO ns_info;
ST_AUD_NS_RTN ns_rtn;
INT32 params[2];

ns_info.s32SamplingRate = 8000;  // Must be 8kHz!
ns_info.s32ChannelNum = 1;       // Mono
ns_info.s32FrameSize = 256;      // 256 at 8kHz, 512 at higher rates

AUD_NS_PreInit(&ns_info, &ns_rtn);

void* internal_buf = malloc(ns_rtn.u32InternalBufSize);
AUD_NS_Init(internal_buf, ns_rtn.u32InternalBufSize);

/* Set noise suppression level (-3 to -40 dB) */
params[0] = -20;  // dB
AUD_NS_SetParam(EN_AUD_NS_NOISE_SUPPRESS, params);

/* Processing loop - 256 samples at a time */
short out_buf[256];
for (int i = 0; i < total_samples; i += 256) {
    AUD_NS_Run(&input[i], out_buf);
    memcpy(&input[i], out_buf, 256 * sizeof(short));  // In-place
}

/* Cleanup */
free(internal_buf);
```

**⚠️ CRITICAL: NS Requirements:**
1. Sample rate MUST be 8kHz (8000 Hz)
2. Frame size is 256 samples at 8kHz
3. Use low-level `AUD_NS_Run()`, NOT `audlib_ns_run()`

#### 2. libagc.a - Automatic Gain Control

**Header:** `code/lib/include/agc/audlib_agc.h`

```c
#include "agc/audlib_agc.h"

// See previous AGC section for full API
audlib_agc_open();
audlib_agc_set_config(AGC_CONFIG_ID_SAMPLERATE, 48000);
audlib_agc_set_config(AGC_CONFIG_ID_TARGET_LVL, AGC_DB(-6));
// ... etc
audlib_agc_init();
audlib_agc_run(&agc_io);
audlib_agc_close();
```

#### 3. libaudfilt.a - Audio Filters (HPF, Notch, EQ)

**Header:** `code/lib/include/audfilt/audlib_filt.h`

```c
#include "audfilt/audlib_filt.h"

// See previous Filter section for full API
audlib_filt_open(&init);
audlib_filt_set_eq(band, &param);  // HPF, Notch, etc.
audlib_filt_init();
audlib_filt_run((long)samples, byte_count);  // BYTES not samples on LP64!
audlib_filt_close();
```

### ANR vs NS Clarification

The web UI shows **"ANR" (Audio Noise Reduction)** but the SDK library is called **"NS" (Noise Suppression)**. They are the same thing:

| Web UI Name | SDK Library | API Prefix |
|-------------|-------------|------------|
| ANR (Noise Reduction) | libaec.a → NS module | `audlib_ns_*` |
| AEC (Echo Cancellation) | libaec.a → AEC module | `audlib_aec_*` |
| AGC (Auto Gain) | libagc.a | `audlib_agc_*` |
| HPF/Notch | libaudfilt.a | `audlib_filt_*` |

### Integration Status

| Feature | Web UI | Hardware API | Userspace Lib | Status |
|---------|--------|--------------|---------------|--------|
| **AGC** | ✅ Yes | `VENDOR_AUDIOCAP_ITEM_AGC_CONFIG` | libagc.a | ✅ Working |
| **ANR/NS** | ✅ Yes | ❌ None | libaec.a (NS) | ✅ Working (low-level API) |
| **AEC** | ✅ Yes | ❌ None | libaec.a (AEC) | ⚠️ Needs two-way audio |
| **HPF** | ❌ No | ❌ None | libaudfilt.a | ✅ Working |
| **Notch** | ❌ No | ❌ None | libaudfilt.a | ✅ Working |

### ⚠️ Important Notes

1. **AEC requires both playback and record streams** - It needs to know what audio is being played through the speaker to cancel the echo from the microphone. Only useful for two-way audio (intercom).

2. **NS requires 8kHz sample rate** - The low-level AUD_NS_* API only works at 8kHz with 256-sample frames. Higher sample rates use 512-sample frames.

3. **Use low-level NS API** - The high-level `audlib_ns_run()` has issues. Use `AUD_NS_Run()` instead.

4. **Frame size constraints:**
   - NS: 256 samples at 8kHz, 512 samples at higher rates
   - Filter: Works with any frame size (CRITICAL: pass bytes on LP64, not samples!)
   - AGC: Works with any frame size

---

## Web UI Settings (User Perspective)

The web UI at `http://<camera-ip>/` has an **Audio Configuration** page under Configuration → Audio.

### Current Web UI Settings

| Setting | Location | Description | Recommended |
|---------|----------|-------------|-------------|
| **Audio Enabled** | Toggle at top | Turn audio on/off | ✅ On |
| **Audio Codec** | Dropdown | G.711 μ-law, G.711 A-law, G.726 | G.711 μ-law |
| **Microphone Boost** | Dropdown | 0/10/20/30 dB pre-amp | +20 dB |
| **Volume** | Slider 0-160 | Digital gain (100 = 0dB) | 100 |
| **AGC (Auto Gain)** | Toggle + sliders | Automatic volume leveling | ✅ On |
| **AGC Target Level** | Slider -40 to -3 dB | Target output level | -6 dB |
| **AGC Noise Gate** | Slider -80 to -20 dB | Ignore signals below this | -50 dB |
| **ANR (Noise Reduction)** | Toggle + slider | Reduce background noise | Optional |
| **AEC (Echo Cancel)** | Toggle | For two-way audio/intercom | Off (unless needed) |

### Recommended Settings for Best Audio Quality

```
Audio Configuration → Audio Settings
├── Audio: ✅ Enabled
├── Audio Codec: G.711 μ-law (Default)
├── Sample Rate: 8 kHz
└── Channels: Mono

Audio Configuration → Input Controls
├── Microphone Boost: +20 dB
├── Volume: 100 (0dB reference)
└── Hardware ALC: ❌ Off (conflicts with AGC)

Audio Configuration → Audio Processing
├── AGC (Auto Gain): ✅ On
│   ├── Target Level: -6 dB
│   └── Noise Gate: -50 dB
├── ANR (Noise Reduction): Optional (level 2-3 if noisy)
└── AEC (Echo Cancel): Off (unless using intercom)
```

### ⚠️ Missing Settings (Not Yet in Web UI)

The **HPF** and **Notch Filter** features we tested are NOT yet exposed in the web UI. To add them:

| New Setting | Purpose | Values | Default |
|-------------|---------|--------|---------|
| **High-Pass Filter** | Remove low rumble/wind | On/Off + Frequency (50-200 Hz) | On, 100 Hz |
| **Notch Filter** | Remove power line hum | On/Off + Frequency (50/60 Hz) | On, 60 Hz (US) |

### Proposed Web UI Enhancement

To enable the full audio chain (AGC + HPF + Notch), add these to the Audio Processing section:

```html
<!-- High-Pass Filter -->
<div class="processing-row">
  <mat-slide-toggle formControlName="hpf_enabled">
    High-Pass Filter (Rumble Reduction)
  </mat-slide-toggle>
  <mat-form-field *ngIf="audioForm.get('hpf_enabled')?.value">
    <mat-label>Cutoff Frequency</mat-label>
    <mat-select formControlName="hpf_freq">
      <mat-option [value]="80">80 Hz</mat-option>
      <mat-option [value]="100">100 Hz (Recommended)</mat-option>
      <mat-option [value]="120">120 Hz</mat-option>
    </mat-select>
  </mat-form-field>
</div>

<!-- Notch Filter -->
<div class="processing-row">
  <mat-slide-toggle formControlName="notch_enabled">
    Notch Filter (Hum Reduction)
  </mat-slide-toggle>
  <mat-form-field *ngIf="audioForm.get('notch_enabled')?.value">
    <mat-label>Power Line Frequency</mat-label>
    <mat-select formControlName="notch_freq">
      <mat-option [value]="50">50 Hz (Europe/Asia)</mat-option>
      <mat-option [value]="60">60 Hz (Americas)</mat-option>
    </mat-select>
  </mat-form-field>
</div>
```

### Quick Setup Guide for Users

**For indoor environments (quiet):**
1. Enable Audio
2. Turn ON **AGC** (Auto Gain)
3. Leave other settings at default

**For outdoor/noisy environments:**
1. Enable Audio
2. Turn ON **AGC** (Auto Gain)
3. Turn ON **ANR** (Noise Reduction) at level 2-3
4. *(Future)* Turn ON **HPF** at 100 Hz

**For environments with electrical hum:**
1. Enable Audio
2. Turn ON **AGC** (Auto Gain)
3. *(Future)* Turn ON **Notch Filter** at 60 Hz (US) or 50 Hz (EU)

---

## Integration into IPCamera Application

The ipcamera application has two approaches for audio processing:

### Current Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                        IPCamera Audio Flow                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   Microphone                                                        │
│       │                                                             │
│       ▼                                                             │
│   ┌───────────────────┐                                             │
│   │  HD_AUDIOCAP      │ ← Hardware audio capture (48kHz stereo)     │
│   │  (Kernel Driver)  │                                             │
│   │                   │ ← vendor_audiocap API: Volume, Boost, AGC   │
│   └────────┬──────────┘                                             │
│            │                                                        │
│            ▼                                                        │
│   ┌───────────────────┐                                             │
│   │  HD_AUDIOENC      │ ← Passes through as PCM (no kernel codec)   │
│   │  (Kernel Driver)  │                                             │
│   └────────┬──────────┘                                             │
│            │                                                        │
│            ▼                                                        │
│   ┌───────────────────┐                                             │
│   │  AudioFrame       │ ← Userspace processing:                     │
│   │  Broadcaster      │   - Resample 48kHz→8kHz                     │
│   │  (Userspace)      │   - Stereo→Mono conversion                  │
│   │                   │   - G.711/G.726/AAC encoding                │
│   └────────┬──────────┘                                             │
│            │                                                        │
│            ▼                                                        │
│   ┌───────────────────┐                                             │
│   │  RTSP/Recording   │ ← Streaming to clients                      │
│   └───────────────────┘                                             │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Option 1: Hardware AGC via vendor_audiocap API (Current)

The `AudioControl` class uses `vendor_audiocap` API to control hardware AGC:

**File:** `code/application/ipcamera/modules/media/src/audio_control.cpp`

```cpp
// Current implementation using hardware AGC
bool AudioControl::SetAGC(bool enabled, int target_db, int noise_gate_db) {
    // Apply to hardware
    VENDOR_AUDIOCAP_AGC_CONFIG agc_cfg = {};
    agc_cfg.enable = enabled ? TRUE : FALSE;
    agc_cfg.target_lvl = target_db;
    agc_cfg.ng_threshold = noise_gate_db;
    
    HD_RESULT ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_AGC_CONFIG, &agc_cfg);
}
```

**Configuration via Web API:**
- `GET/PUT /api/audio/config` with `agc_enabled`, `agc_target_db`, `agc_noise_gate_db`

**Limitation:** Hardware AGC has limited parameters. Cannot add HPF/Notch filters.

### Option 2: Userspace Processing (Recommended for Full Feature Set)

To add HPF and Notch filters, modify `AudioFrameBroadcaster::EncodeFrame()`:

**File:** `code/application/ipcamera/modules/streaming/src/audio_frame_broadcaster.cpp`

#### Step 1: Add library includes

```cpp
// Add to audio_frame_broadcaster.cpp
extern "C" {
#include "agc/audlib_agc.h"
#include "audfilt/audlib_filt.h"
}
```

#### Step 2: Add processing state to AudioFrameBroadcaster class

```cpp
// In audio_frame_broadcaster.h, add to private section:
bool agc_enabled_ = false;
bool agc_initialized_ = false;
bool hpf_enabled_ = false;
int hpf_freq_ = 100;
bool notch_enabled_ = false;
int notch_freq_ = 60;
bool filt_initialized_ = false;
std::vector<int16_t> process_buffer_;
```

#### Step 3: Initialize in Start()

```cpp
bool AudioFrameBroadcaster::Start() {
    // ... existing code ...
    
    // Initialize userspace AGC
    if (AudioControl::Instance().GetConfig().agc_enabled) {
        if (audlib_agc_open() == 0) {
            audlib_agc_set_config(AGC_CONFIG_ID_SAMPLERATE, sample_rate_);
            audlib_agc_set_config(AGC_CONFIG_ID_CHANNEL_NO, 1);
            audlib_agc_set_config(AGC_CONFIG_ID_TARGET_LVL, AGC_DB(-6));
            audlib_agc_set_config(AGC_CONFIG_ID_MAXGAIN, AGC_DB(20));
            audlib_agc_set_config(AGC_CONFIG_ID_MINGAIN, AGC_DB(-10));
            audlib_agc_set_config(AGC_CONFIG_ID_ATTACK_TIME, AGC_TRESO_BASIS_100MS);
            audlib_agc_set_config(AGC_CONFIG_ID_DECAY_TIME, AGC_TRESO_BASIS_200MS);
            audlib_agc_set_config(AGC_CONFIG_ID_NG_THD, AGC_DB(-50));
            if (audlib_agc_init()) {
                agc_initialized_ = true;
                spdlog::info("Userspace AGC initialized");
            }
        }
    }
    
    // Initialize HPF/Notch filter
    if (hpf_enabled_ || notch_enabled_) {
        AUDFILT_INIT filt_init = {.filt_ch = AUDFILT_CH_MONO, .smooth_enable = TRUE};
        if (audlib_filt_open(&filt_init)) {
            AUDFILT_CONFIG cfg;
            audlib_filt_get_config(&cfg);
            cfg.samplerate = sample_rate_;
            audlib_filt_set_config(&cfg);
            
            if (hpf_enabled_) {
                AUDFILT_EQPARAM eq = {};
                eq.type = AUDFILT_EQ_TYPE_HIGHPASS;
                eq.freq = hpf_freq_;
                eq.Q = 0.5f;
                eq.gain = 0;
                audlib_filt_set_eq(0, &eq);
                spdlog::info("HPF initialized: {}Hz", hpf_freq_);
            }
            if (notch_enabled_) {
                AUDFILT_EQPARAM eq = {};
                eq.type = AUDFILT_EQ_TYPE_NOTCH;
                eq.freq = notch_freq_;
                eq.Q = 5.0f;  // SDK default
                eq.gain = -40; // SDK default
                audlib_filt_set_eq(1, &eq);
                spdlog::info("Notch initialized: {}Hz", notch_freq_);
            }
            audlib_filt_init();
            filt_initialized_ = true;
        }
    }
    
    process_buffer_.resize(kMaxSamplesPerFrame);
    // ... rest of Start() ...
}
```

#### Step 4: Process in EncodeFrame()

```cpp
bool AudioFrameBroadcaster::EncodeFrame(const uint8_t* pcm_data, size_t pcm_size,
                                        uint64_t timestamp, AudioFrame& out_frame) {
    // ... existing resample code ...
    
    // PROCESSING PIPELINE: HPF/Notch → AGC
    int16_t* samples = resample_buffer_.data();
    size_t sample_count = mono_samples;
    
    // Apply filter first (before AGC)
    if (filt_initialized_) {
        // CRITICAL: Pass bytes, not samples!
        audlib_filt_run((long)samples, sample_count * 2);
    }
    
    // Apply AGC after filtering
    if (agc_initialized_) {
        AGC_BITSTREAM agc_io;
        agc_io.bitstram_buffer_in = (UINT64)(uintptr_t)samples;
        agc_io.bitstram_buffer_out = (UINT64)(uintptr_t)process_buffer_.data();
        agc_io.bitstram_buffer_length = sample_count;
        
        if (audlib_agc_run(&agc_io)) {
            samples = process_buffer_.data();
        }
    }
    
    // ... existing encoding code (G.711, G.726, AAC) ...
}
```

#### Step 5: Link libraries in CMakeLists.txt

```cmake
# In modules/streaming/CMakeLists.txt
target_link_libraries(streaming
    # ... existing libs ...
    agc
    audfilt
)
```

### Web API Extension

To expose HPF/Notch via web API, add to `AudioConfig`:

**File:** `code/application/ipcamera/modules/media/include/ipcam/audio_control.h`

```cpp
struct AudioConfig {
    // ... existing fields ...
    
    // High-pass filter (removes rumble)
    bool hpf_enabled = false;
    int hpf_freq = 100;  // 100Hz default
    
    // Notch filter (removes power line hum)
    bool notch_enabled = false;
    int notch_freq = 60;  // 60Hz for US, 50Hz for EU
};
```

**Web API endpoints:**
- `PUT /api/audio/config` with `hpf_enabled`, `hpf_freq`, `notch_enabled`, `notch_freq`

### Recommended Default Configuration

```json
{
  "audio": {
    "enabled": true,
    "codec": "g711u",
    "sample_rate": 8000,
    "volume": 100,
    "boost_gain": "20db",
    "agc_enabled": true,
    "agc_target_db": -6,
    "agc_noise_gate_db": -50,
    "hpf_enabled": true,
    "hpf_freq": 100,
    "notch_enabled": true,
    "notch_freq": 60
  }
}
```

### Processing Order

**Correct order:** Filter → AGC
```
Audio Input → HPF (100Hz) → Notch (60Hz) → AGC → Codec Encoding → Output
```

**Why?** 
- Filter removes unwanted low-frequency noise (rumble, hum)
- AGC then normalizes the clean signal
- If AGC runs first, it amplifies noise before filtering removes it

---

## Summary

| Feature | Hardware API | Userspace Library | Notes |
|---------|--------------|-------------------|-------|
| Volume | `HD_AUDIOCAP_PARAM_VOLUME` | - | 0-160 (100=0dB) |
| Boost Gain | `VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING` | - | 0/10/20/30 dB |
| AGC | `VENDOR_AUDIOCAP_ITEM_AGC_CONFIG` | `libagc.a` | Userspace has more control |
| HPF | ❌ | `libaudfilt.a` | Must use userspace |
| Notch | ❌ | `libaudfilt.a` | Must use userspace |
| ALC | `VENDOR_AUDIOCAP_ITEM_ALC_CONFIG` | - | Hardware only |

---

## Next Steps

1. ✅ Audio features validated in demo application
2. **TODO:** Integrate userspace AGC+HPF+Notch into `AudioFrameBroadcaster`
3. **TODO:** Add web API endpoints for HPF/Notch configuration
4. **TODO:** Test with RTSP streaming clients
5. **TODO:** Evaluate AEC (Acoustic Echo Cancellation) for two-way audio
