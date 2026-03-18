# AI Modules - Requirements and Installation Guide

## Overview

This document lists the required libraries, models, and configurations needed to compile and run the AI modules for the Novatek IP Camera.

---

## 1. VQA (Video Quality Analysis)

### Purpose
Detects video quality issues: blur, over/under exposure, camera tampering.

### Required Libraries
| Library | Path | Description |
|---------|------|-------------|
| `libvqa_lib.so` | `/rootfs/lib/` | VQA analysis library |

### Header Files
- `vqa_lib.h` - VQA API declarations

### SDK Installation
```bash
# Libraries should be in the SDK at:
ls -la $SDK_ROOT/code/lib/output/libvqa_lib.so

# If not present, build from source:
cd $SDK_ROOT/code/lib/source/vqa_lib
make release
```

### Configuration
```json
{
  "vqa": {
    "enabled": true,
    "width": 640,
    "height": 360,
    "blur_threshold": 50,
    "brightness_threshold": 200
  }
}
```

---

## 2. Motion Detection (libmd)

### Purpose
Hardware-accelerated motion detection with zones, line crossing, and object tracking.

### Required Libraries
| Library | Path | Description |
|---------|------|-------------|
| `libmd.a` / `libmd.so` | `/rootfs/lib/` | Motion detection library |
| `libvendor_md.so` | `/rootfs/lib/` | Vendor MD wrapper |

### Header Files
- `libmd/libmd.h` - Core MD functions (NVT_MD_MDBC, NVT_MD_GlobalAlarm, NVT_MD_SubAlarm)
- `vendor_md.h` - Vendor wrapper for initialization

### SDK Installation
```bash
# Libraries are pre-built in SDK:
ls -la $SDK_ROOT/code/lib/output/libmd.*
ls -la $SDK_ROOT/code/hdal/vendor/output/libvendor_md.so

# Build from source if needed:
cd $SDK_ROOT/code/lib/source/libmd
make release
```

### Memory Requirements
- Source buffer: width × height bytes
- Destination buffer: width × height bytes
- Temp buffer: width × height × 70 bytes (for MDBC)

### Configuration
```json
{
  "motion_detection": {
    "enabled": true,
    "mode": "mdbc",
    "sensitivity": 3,
    "global_threshold": 30,
    "zones": [
      {"id": 1, "x_start": 0, "y_start": 0, "x_end": 320, "y_end": 180, "threshold": 30}
    ]
  }
}
```

---

## 3. Object Tracking (TRKE)

### Purpose
Multi-object tracking with IOU matching and Kalman filtering. Optional optical flow via libtrke.

### Required Libraries (Optional - has software fallback)
| Library | Path | Description |
|---------|------|-------------|
| `libtrke.so` | `/rootfs/lib/` | Tracking Engine library |
| `libvendor_trke.so` | `/rootfs/lib/` | Vendor TRKE wrapper |

### Header Files
- `libtrke/libtrke.h` - TRKE optical flow functions
- `vendor_trke.h` - Vendor wrapper

### SDK Installation
```bash
# Check for TRKE libraries:
ls -la $SDK_ROOT/code/lib/output/libtrke.*
ls -la $SDK_ROOT/code/hdal/vendor/output/libvendor_trke.so

# If not available, the module uses IOU + Kalman fallback (no external dependencies)
```

### Configuration
```json
{
  "tracking": {
    "enabled": true,
    "max_age": 30,
    "min_hits": 3,
    "iou_threshold": 0.3
  }
}
```

---

## 4. Privacy Mosaic (GFX)

### Purpose
Hardware-accelerated face/region blurring using graphics engine.

### Required Libraries
| Library | Path | Description |
|---------|------|-------------|
| `libvendor_gfx.so` | `/rootfs/lib/` | Graphics engine wrapper |

### Header Files
- `vendor_gfx.h` - GFX API (vendor_gfx_grph_trigger_no_flush)

### SDK Installation
```bash
# GFX library location:
ls -la $SDK_ROOT/code/hdal/vendor/output/libvendor_gfx.so

# Part of HDAL vendor libraries - built with HDAL
cd $SDK_ROOT/code/hdal
make vendor
```

### Supported Block Sizes
- 8×8 pixels
- 16×16 pixels
- 32×32 pixels
- 64×64 pixels

### Configuration
```json
{
  "privacy_mosaic": {
    "enabled": true,
    "block_size": 16,
    "auto_detect_faces": true,
    "regions": [
      {"id": 1, "x": 100, "y": 100, "width": 200, "height": 200, "type": "block"}
    ]
  }
}
```

---

## 5. AI-ISP (Neural Network Enhancement)

### Purpose
AI-based image enhancement: noise reduction, WDR, defog.

### Required Libraries
| Library | Path | Description |
|---------|------|-------------|
| `libvendor_ai3.so` | `/rootfs/lib/` | AI3 neural network engine |
| `libvendor_ai3_pub.so` | `/rootfs/lib/` | AI3 public utilities |
| `libprebuilt_ai.so` | `/rootfs/lib/` | Prebuilt AI models |
| `libvendor_isp.so` | `/rootfs/lib/` | ISP integration |
| `libvos.so` | `/rootfs/lib/` | VOS utility library (required by AI3) |

### Header Files
- `vendor_ai.h` - AI3 device and network APIs
- `vendor_isp.h` - ISP callback integration

### Required AI Models
| Model | Path | Description |
|-------|------|-------------|
| `aiisp_normal.bin` | `/mnt/app/ai_models/` | Normal lighting model |
| `aiisp_lowlight.bin` | `/mnt/app/ai_models/` | Low-light enhancement model |

### SDK Installation
```bash
# AI3 libraries:
ls -la $SDK_ROOT/code/hdal/vendor/output/libvendor_ai3*.so
ls -la $SDK_ROOT/code/lib/output/libprebuilt_ai.so
ls -la $SDK_ROOT/code/vos/output/libvos.so

# Build all dependencies:
cd $SDK_ROOT/code/hdal
make vendor

cd $SDK_ROOT/code/vos
make release

cd $SDK_ROOT/code/lib/source/prebuilt_ai
make release
```

### Model Installation
```bash
# Copy AI models to device:
mkdir -p /mnt/app/ai_models
cp ai_models/aiisp_*.bin /mnt/app/ai_models/
```

### Configuration
```json
{
  "aiisp": {
    "enabled": true,
    "model_path": "/mnt/app/ai_models",
    "model_name_normal": "aiisp_normal.bin",
    "model_name_low": "aiisp_lowlight.bin",
    "denoise_strength": 50,
    "wdr_enabled": true,
    "wdr_strength": 50
  }
}
```

---

## Complete Build Instructions

### 1. Build SDK Libraries
```bash
cd $SDK_ROOT

# Build all libraries
make library

# Or build specific components:
source build/envsetup.sh
cd code/lib && make release
cd code/vos && make release
cd code/hdal && make vendor
```

### 2. Build Application with AI Modules
```bash
cd $SDK_ROOT/code/application/ipcamera

# Configure with CMake
cmake . -B build/aarch64-ca53 \
    -DCMAKE_BUILD_TYPE=Release \
    -DHDAL_PIPELINE_ENABLED=ON

# Build
cmake --build build/aarch64-ca53 --parallel

# The build will automatically detect available libraries and enable modules:
# - AI_VQA_ENABLED=1 if libvqa_lib.so found
# - AI_MD_ENABLED=1 if libmd.so and libvendor_md.so found
# - AI_TRKE_ENABLED=1 if libtrke.so found (optional)
# - AI_GFX_ENABLED=1 if libvendor_gfx.so found
# - AI_ISP_ENABLED=1 if libvendor_isp.so found
# - VENDOR_AI3_ENABLED=1 if libvendor_ai3.so and libvos.so found
```

### 3. Deploy to Device
```bash
# Build complete firmware
cd $SDK_ROOT
make all

# Push to device
adb push output/packed/FW98538A.bin /mnt/sd/

# Or push individual files for testing:
adb push code/application/ipcamera/build/aarch64-ca53/bin/ipcamd /mnt/app/bin/
```

---

## Library Dependency Tree

```
ipcamd
├── ipcam_ai (module)
│   ├── libvqa_lib.so (VQA)
│   ├── libmd.so + libvendor_md.so (Motion Detection)
│   ├── libtrke.so + libvendor_trke.so (Tracking - optional)
│   ├── libvendor_gfx.so (Privacy Mosaic)
│   ├── libvendor_isp.so (ISP Integration)
│   └── libvendor_ai3.so + libvos.so + libprebuilt_ai.so (AI-ISP)
├── ipcam_platform (module)
│   └── libhdal*.so (HDAL core)
└── ipcam_streaming (module)
    └── liblive555.so (RTSP)
```

---

## Compile Definitions (CMake)

The following compile definitions control module availability:

| Definition | Effect |
|------------|--------|
| `HDAL_PIPELINE_ENABLED=1` | Enable HDAL video processing |
| `AI_VQA_ENABLED=1` | Enable VQA module |
| `AI_MD_ENABLED=1` | Enable Motion Detection module |
| `AI_TRKE_ENABLED=1` | Enable TRKE tracking (optional) |
| `AI_GFX_ENABLED=1` | Enable GFX privacy mosaic |
| `AI_ISP_ENABLED=1` | Enable ISP integration |
| `VENDOR_AI3_ENABLED=1` | Enable AI3 neural network |

---

## Troubleshooting

### Missing Libraries
```bash
# Check library availability on target:
adb shell "ls -la /lib/libvqa_lib.so"
adb shell "ls -la /lib/libvendor_md.so"
adb shell "ls -la /lib/libvendor_gfx.so"
adb shell "ls -la /lib/libvendor_ai3.so"
```

### Module Not Enabled
Check CMake output for:
```
AI module: VQA (Video Quality Analysis) enabled
AI module: Motion detection (libmd) enabled
AI module: Vendor GFX (privacy mosaic) enabled
AI module: Vendor AI3 engine enabled (with VOS)
```

### Runtime Errors
```bash
# Check library loading:
adb shell "LD_DEBUG=libs /mnt/app/bin/ipcamd 2>&1 | grep -i 'vqa\|vendor'"
```

---

## Performance Notes

| Module | Typical Processing Time | Memory Usage |
|--------|------------------------|--------------|
| VQA | ~2-5ms per frame | ~100KB |
| Motion Detection (MDBC) | ~5-10ms per frame | ~width×height×72 bytes |
| Object Tracking | ~1-2ms per frame | ~10KB per track |
| Privacy Mosaic | ~1ms per region | ~4KB |
| AI-ISP | ~10-20ms per frame | ~4MB (model dependent) |

---

## Version Compatibility

- SDK Version: NT98538 / NA51102
- HDAL Version: 2.0+
- Kernel: Linux 5.4+
- GCC: 10.2+ (aarch64-linux-gnu)
