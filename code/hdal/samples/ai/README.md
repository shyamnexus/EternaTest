# NT98538 AI Feature Demo Samples

This directory contains individual demo applications for various AI and computer vision features supported by the NT98538 platform. Each demo is self-contained and ready for demonstration purposes.

## Demo Overview

| Demo | Description | Key APIs |
|------|-------------|----------|
| `ai_pvdcnn_demo` | Person/Vehicle Detection CNN | `pvdcnn_init`, `pvdcnn_process_inplace` |
| `ai_fdcnn_demo` | Face Detection with landmarks | `fdcnn_init`, `fdcnn_process` |
| `ai_motion_detection_demo` | MDBC, Global Alarm, Object Detection | `NVT_MD_MDBC`, `NVT_MD_GlobalAlarm`, `NVT_MD_ObjDet` |
| `ai_vqa_demo` | Video Quality Analysis (blur, exposure) | `NVT_VQA_Init`, `NVT_VQA_Run` |
| `ai_line_crossing_demo` | Line crossing detection | `NVT_MD_CrossLine` |
| `ai_intrusion_detection_demo` | Forbidden zone monitoring | `NVT_MD_ObjDet` + zone check |
| `ai_privacy_mosaic_demo` | Real-time privacy masking | GFX mosaic operations |
| `ai_crowd_counting_demo` | Object/person counting | `NVT_MD_ObjDet` with statistics |
| `ai_object_tracking_demo` | Multi-object tracking with IDs | IoU-based tracking |
| `ai_cross_counting_demo` | Multi-line cross counting | `NVT_MD_CrossLine` |
| `ai_heat_map_demo` | Motion activity heat map | `NVT_MD_MDBC` + heat accumulation |

## Building

### Build All Demos
```bash
cd /path/to/hdal/samples/ai
make all
```

### Build Individual Demo
```bash
cd /path/to/hdal/samples/ai/ai_pvdcnn_demo
make
```

### Clean Build
```bash
make clean
```

## Running Demos

Each demo is a standalone executable. Basic usage:

```bash
# Run for 30 seconds (default)
./ai_motion_detection_demo

# Run for specific duration
./ai_pvdcnn_demo 60

# Run with sensitivity parameter (where applicable)
./ai_motion_detection_demo 30 2
```

### Common Parameters
- `duration` - How long to run (in seconds)
- `sensitivity` - Detection sensitivity (0=low, 1=med, 2=high, 3=super_high)

## Feature Details

### 1. PVDCNN (Person/Vehicle Detection)
Detects persons, vehicles, and non-motorized vehicles using deep learning.
- Input: 416x416 or 512x512 YUV
- Output: Bounding boxes with class and confidence
- Categories: Person (0), Vehicle (1), Non-Motor (2)

### 2. FDCNN (Face Detection)
Detects faces with 5-point facial landmarks.
- Landmarks: Left eye, Right eye, Nose, Left mouth, Right mouth
- Output: Face bounding boxes with confidence scores

### 3. Motion Detection
Comprehensive motion detection using MDBC algorithm:
- **Global Alarm**: Frame-wide motion threshold
- **Sub-Region Alarm**: 4-zone motion detection
- **Object Detection**: Up to 32 moving objects
- **Cross-Line**: Directional crossing detection

### 4. Video Quality Analysis (VQA)
Monitors video quality issues:
- Blur detection with coverage percentage
- Overexposure (too bright) detection
- Underexposure (too dark) detection

### 5. Line Crossing / Cross Counting
Detects objects crossing virtual lines:
- Configurable line positions
- In/Out direction tracking
- Multiple line support

### 6. Intrusion Detection
Monitors forbidden zones for unauthorized entry:
- Configurable zone coordinates
- Instant alert on intrusion

### 7. Privacy Mosaic
Applies real-time privacy masking:
- Multiple region support
- Configurable block size
- Real-time processing

### 8. Crowd/Object Counting
Tracks number of objects in scene:
- Running statistics (max, avg)
- Histogram output

### 9. Object Tracking
Multi-object tracking with persistent IDs:
- IoU-based matching
- Track lifecycle management
- New/Lost object notifications

### 10. Heat Map
Generates motion activity heat map:
- Grid-based accumulation
- Temporal decay
- ASCII visualization
- File output

## Required Libraries

Different demos require different libraries:

| Library | Demos |
|---------|-------|
| `libvendor_ai3` | pvdcnn, fdcnn |
| `libvendor_cv` | motion, vqa, line_crossing, intrusion, counting, tracking, heat_map |
| `libprebuilt_pvdcnn` | pvdcnn |
| `libprebuilt_fdcnn` | fdcnn |
| `libprebuilt_cv` | All CV-based demos |

## Model Files

AI demos require pre-trained model files:

```
/mnt/sd/CNNLib/para/
├── pvdcnn/
│   ├── nvt_model_416_prune37.bin
│   └── nvt_model_512_prune37.bin
├── fdcnn/
│   └── nvt_model.bin
└── configs/
    └── pvdcnn_para_config.txt
```

## Output

- Console output with detection/event information
- Statistics summary at program end
- Some demos save files to `/mnt/sd/`:
  - Heat map: `heat_map.txt`
  - Video recordings (if enabled)

## Customization

Each demo can be customized by modifying:
1. **Resolution**: `CAP_WIDTH`, `CAP_HEIGHT`, `MAIN_WIDTH`, `MAIN_HEIGHT`
2. **Sensor**: `SENSOR_NAME`
3. **Detection parameters**: Thresholds, zones, line positions
4. **Output format**: File paths, console verbosity

## Troubleshooting

1. **Demo fails to start**: Check that required libraries and models are present
2. **No detections**: Verify model files are correctly loaded
3. **Memory errors**: Ensure sufficient memory pool configuration
4. **Pipeline errors**: Check sensor driver is loaded correctly

## License

Copyright Novatek Microelectronics Corp. All rights reserved.
