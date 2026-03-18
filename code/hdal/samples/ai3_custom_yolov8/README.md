# AI3 Custom YOLOv8 Sample

Sample application demonstrating custom YOLOv8 model inference on Novatek NT98538 AI3 NPU.

## Overview

This sample shows how to:
1. Load a custom YOLOv8 model compiled for Novatek AI3
2. Run inference on an input image
3. Post-process results (decode, NMS, print detections)

## Files

| File | Description |
|------|-------------|
| `ai3_custom_yolov8.c` | Main sample code - model loading, inference |
| `yolov8_postproc.c` | YOLOv8 post-processing (NMS, decode) |
| `yolov8_postproc.h` | Post-processing header |
| `Makefile` | Build configuration |
| `models/` | Pre-compiled model files |

## Pre-compiled Models

Located in `models/`:

| Model | File | Input Size | Purpose |
|-------|------|------------|---------|
| YOLOv8n | `yolov8n.bin` | 640×640 | 80-class COCO detection |
| YOLOv8-Helmet | `yolov8_helmet.bin` | 640×640 | Helmet/PPE detection |
| SCRFD-500M | `scrfd_500m.bin` | 640×640 | Face detection |

## Building

```bash
# Source environment
cd /path/to/ns02302_linux_sdk
source build/envsetup.sh

# Build this sample
cd code/hdal/samples/ai3_custom_yolov8
make

# Or build all samples
cd code/hdal/samples
make
```

## Usage

```bash
./ai3_custom_yolov8 <model.bin> <input.bin> [labels.txt]
```

### Arguments

| Argument | Required | Description |
|----------|----------|-------------|
| `model.bin` | Yes | Compiled YOLOv8 model file |
| `input.bin` | Yes | Input image (640×640 NV12/YUV420) |
| `labels.txt` | No | COCO class labels file |

### Example

```bash
# Copy files to SD card
scp ai3_custom_yolov8 root@camera:/mnt/sd/
scp models/*.bin root@camera:/mnt/sd/models/

# SSH to camera
ssh root@camera

# Run inference
cd /mnt/sd
./ai3_custom_yolov8 models/yolov8n.bin test_640x640.bin models/coco_labels.txt
```

### Interactive Mode

After first inference, the application enters interactive mode:
- Press `r` to run inference again
- Press `q` to quit

## Preparing Input Data

The input should be a raw binary file containing 640×640 NV12 or YUV420 image data.

### Convert from JPEG/PNG (on host)

```python
import cv2
import numpy as np

# Read and resize image
img = cv2.imread('test.jpg')
img = cv2.resize(img, (640, 640))

# Convert to YUV420 (NV12)
yuv = cv2.cvtColor(img, cv2.COLOR_BGR2YUV_I420)

# Save raw binary
yuv.tofile('test_640x640.bin')
```

## Output Format

Detection results are printed in format:
```
========== YOLOv8 Detection Results ==========
Total detections: 3
----------------------------------------------
 1. [person] conf=85.20% bbox=(0.123, 0.234, 0.456, 0.789)
 2. [car] conf=72.50% bbox=(0.500, 0.100, 0.900, 0.400)
 3. [dog] conf=65.30% bbox=(0.050, 0.600, 0.200, 0.950)
==============================================
```

Bounding box coordinates are normalized (0.0 - 1.0).

## Configuration

Default thresholds in `yolov8_postproc.h`:
- Confidence threshold: 0.25
- NMS IoU threshold: 0.45

Modify in code or add command-line arguments as needed.

## Troubleshooting

### "vendor_ai3_dev_get(MODEL_INFO) failed"
Model may be compiled for different chip. Ensure model was compiled with `--chip 336` for NT98538.

### "Failed to allocate memory"
Increase HDAL memory pool size in memory configuration.

### "Cannot open model file"
Check file path and permissions. Use absolute paths.

## Related Samples

- `ai3_net/` - Basic AI3 network sample (classification)
- `ai3_turnkey_sample_stream_ipc/` - AI + video pipeline integration
- `alg_pvdcnn_sample/` - Person/Vehicle detection with vendor library

## Converting Custom Models

See `/home/ubuntu/IP_Camera_Novatek/Novatek_ai/` for:
1. PyTorch to ONNX export
2. ONNX to Novatek .bin compilation
3. Docker environment for compilation

```bash
cd /home/ubuntu/IP_Camera_Novatek/Novatek_ai
make export   # PyTorch -> ONNX
make convert  # ONNX -> Novatek .bin (in Docker)
```
