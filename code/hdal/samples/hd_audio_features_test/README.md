# Novatek Audio Features Test Sample

A comprehensive test application for validating all audio processing features on the Novatek NS02302 platform.

## Features Tested

| Feature | API | Path Type | Status |
|---------|-----|-----------|--------|
| Volume Control | `HD_AUDIOCAP_PARAM_VOLUME` | CTRL | ✅ Working |
| Gain Level | `VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL` | CTRL | ✅ Working |
| ALC Enable | `VENDOR_AUDIOCAP_ITEM_ALC_ENABLE` | CTRL | ✅ Working |
| ALC Config | `VENDOR_AUDIOCAP_ITEM_ALC_CONFIG` | CTRL | ⚠️ May fail |
| Loopback | `VENDOR_AUDIOCAP_ITEM_LOOPBACK_CONFIG` | CTRL | ✅ Working |
| Default Setting | `VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING` | OUT | ⚠️ Requires OUT path |
| AGC Config | `VENDOR_AUDIOCAP_ITEM_AGC_CONFIG` | OUT | ⚠️ Requires SDK enable |
| Noise Gate | `VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD` | OUT | ⚠️ Requires OUT path |

## Building

```bash
cd /path/to/ns02302_linux_sdk
source build/envsetup.sh
cd code/hdal/samples/hd_audio_features_test
make
```

## Usage

```bash
# Run all tests
./audio_features_test --test all

# Test specific feature
./audio_features_test --test volume --volume 150
./audio_features_test --test gain
./audio_features_test --test alc

# Record audio (5 seconds at 48kHz)
./audio_features_test --test record --duration 5

# Record at 8kHz (for AEC testing)
./audio_features_test --test record --rate 8000 --duration 10
```

## Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--test <mode>` | Test mode: all, volume, gain, alc, agc, record | all |
| `--volume <0-200>` | Capture volume (100 = unity gain) | 100 |
| `--duration <sec>` | Recording duration (0 = skip) | 5 |
| `--rate <hz>` | Sample rate | 48000 |
| `--output <path>` | Output directory for recordings | /mnt/sd |
| `--verbose` | Enable verbose output | off |
| `--help` | Show help message | - |

## Expected Output

```
╔════════════════════════════════════════════════════════════════╗
║ Novatek Audio Features Test                                    ║
╚════════════════════════════════════════════════════════════════╝

Configuration:
  Sample Rate:  48000 Hz
  Volume:       100
  Duration:     5 seconds
  Output Path:  /mnt/sd

▶ Initializing HDAL
────────────────────────────────────────
    HDAL common: OK
    Memory pool: OK
    Audio capture: OK
    Configuration: OK

▶ Volume Control Test (HD_AUDIOCAP_PARAM_VOLUME)
────────────────────────────────────────
  ℹ Testing volume range: 0 (mute) to 200 (2x gain)
    Volume   0: OK
    Volume  50: OK
    Volume 100: OK
    Volume 150: OK
    Volume 200: OK
  ✓ PASS: Volume control working (0-200 range)

▶ Gain Level Test (VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL)
────────────────────────────────────────
  ℹ Testing gain level granularity options
    Gain Level 8 steps: OK
    Gain Level 16 steps: OK
    Gain Level 32 steps: OK
  ✓ PASS: Gain level control working (8/16/32 steps)

▶ ALC Control Test (VENDOR_AUDIOCAP_ITEM_ALC_ENABLE)
────────────────────────────────────────
  ℹ ALC = Auto Level Control (hardware-based gain)
  ℹ Recommendation: Disable ALC when using software AGC/AEC/ANR
    ALC Disable: OK
    ALC Enable:  OK
  ✓ PASS: ALC enable/disable working

╔════════════════════════════════════════════════════════════════╗
║ Test Summary                                                    ║
╚════════════════════════════════════════════════════════════════╝

  Total Tests:  8
  Passed:       5
  Failed:       0
  Skipped:      3

All tests passed!

Notes:
  • AGC requires AUDCAP_LIB_AGC=ENABLE in SDK build
  • DEFAULT_SETTING, AGC, Noise Gate require OUT path, not CTRL
  • ALC should be disabled when using software AGC/AEC/ANR
```

## Understanding Results

### Working Features (CTRL Path)
These features work reliably on the CTRL path:
- **Volume**: 0-200 range, 100 = unity gain
- **Gain Level**: 8/16/32 step granularity
- **ALC Enable**: Hardware automatic level control
- **Loopback**: Required for AEC reference signal

### Skipped Features (OUT Path / SDK Config)
These features may be skipped due to:
1. **Requires OUT path**: `DEFAULT_SETTING`, `AGC_CONFIG`, `NOISEGATE_THRESHOLD`
   - The vendor API expects an OUT port ID, not CTRL
   - This is a known SDK limitation

2. **Requires SDK rebuild**: `AGC_CONFIG`
   - AGC library is disabled by default (`AUDCAP_LIB_AGC=DISABLE`)
   - Located in: `code/hdal/drivers/k_flow/include/kflow_audiocapture/isf_audcap.h`

## Troubleshooting

### Error -32 (HD_ERR_IO)
The API expects a different path type:
- CTRL path features: Volume, Gain Level, ALC Enable, Loopback
- OUT path features: Default Setting, AGC Config, Noise Gate

### Error -34 (HD_ERR_PARAM)
AGC library not enabled in SDK build.

### Error -35 (HD_ERR_NOT_SUPPORT)
Feature not supported by hardware/driver.

## Audio Processing Pipeline

For best results with AEC/ANR/AGC software processing:

```
Microphone → [ALC=OFF] → PCM Capture → [HPF] → [ANR] → [AEC] → [AGC] → Output
```

1. **Disable ALC** when using software AGC
2. **Use 8kHz** sample rate for AEC (reduces memory usage)
3. **Enable Loopback** for AEC reference signal
4. **Process in order**: HPF → ANR → AEC → AGC

## Related Samples

- `speakerphone_test` - Full duplex with AEC/ANR/AGC
- `hd_audio_record` - Basic PCM recording
- `hd_audio_bidirect` - Bidirectional audio
- `audio_vendor_record` - Vendor API usage
