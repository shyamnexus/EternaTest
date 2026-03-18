# AI Features Integration Plan — NT98538 IP Camera

## Executive Summary

This plan covers the integration of **14 AI features** commonly found in commercial IP cameras into our `ipcamd` application. Each feature will be developed as a standalone example first (in `hdal/samples/ai3_feature_*`), verified on-device, then integrated into the `ipcam_ai` module one at a time with testing.

**Hardware platform**: NT98538 (dual CA53 + CNN/AI3 NPU)
**Models verified working on NPU**: YOLOv8n, YOLOv26n/s, SCRFD-500m, MobileFaceNet, MobileNetV3, EfficientNet-B0, ResNet18, LPRNet, PlateDetector, YOLOv8-Pose

---

## Feature Inventory

| # | Feature | Models Used | Priority | Status |
|---|---------|-------------|----------|--------|
| 1 | **Object Detection (Person/Vehicle/Animal)** | YOLOv8n or YOLOv26n | P0 — Core | Standalone verified |
| 2 | **Multi-Object Tracking (MOT)** | IoU + Kalman + OSNet Re-ID | P0 — Core | Standalone verified |
| 3 | **Face Detection** | SCRFD-500m | P0 — Core | Standalone verified |
| 4 | **Motion Detection** | Software frame-diff + vendor_cv MDBC | P0 — Core | Module exists |
| 5 | **Tamper/Sabotage Detection** | IVE Histo/Sobel (no model) | P0 — Core | Module exists |
| 6 | **Line Crossing Detection** | YOLO + Tracker | P1 — Important | Defined in config |
| 7 | **Zone Intrusion Detection** | YOLO + Tracker + polygon | P1 — Important | Defined in config |
| 8 | **Privacy Masking (Face Blur)** | SCRFD + GFX mosaic | P1 — Important | Module exists |
| 9 | **People/Vehicle Counting** | YOLO + Tracker + line/zone | P1 — Important | In unified_features |
| 10 | **Face Recognition (Re-ID)** | SCRFD + MobileFaceNet | P2 — Advanced | Model available |
| 11 | **License Plate Recognition (LPR)** | PlateDetector + LPRNet | P2 — Advanced | Models available |
| 12 | **Pose Estimation** | YOLOv8n-Pose | P2 — Advanced | Model available |
| 13 | **Heat Map Generation** | Tracker accumulation | P2 — Advanced | In unified_features |
| 14 | **Abandoned/Removed Object** | YOLO + Tracker + static detection | P3 — Future | Design only |

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                        ipcamd                                │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │               AnalyticsEngine (analytics.cpp)           │ │
│  │                                                         │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐             │ │
│  │  │ AI3 NPU  │  │ Vendor   │  │ Software │             │ │
│  │  │ Backend  │  │ CV/MD    │  │ Fallback │             │ │
│  │  └────┬─────┘  └────┬─────┘  └────┬─────┘             │ │
│  │       │              │              │                    │ │
│  │  ┌────▼──────────────▼──────────────▼───────┐          │ │
│  │  │         Inference Result Bus              │          │ │
│  │  └────┬──────┬──────┬──────┬──────┬─────────┘          │ │
│  │       │      │      │      │      │                     │ │
│  │  ┌────▼─┐┌───▼──┐┌──▼──┐┌─▼───┐┌─▼────┐              │ │
│  │  │Object││Face  ││Moti ││Tamp ││Pose  │  ...          │ │
│  │  │Detect││Detect││on   ││er   ││Estim │              │ │
│  │  └──┬───┘└──┬───┘└──┬──┘└──┬──┘└──┬───┘              │ │
│  │     │       │       │      │      │                    │ │
│  │  ┌──▼───────▼───────▼──────▼──────▼───────┐           │ │
│  │  │           Post-Processing               │           │ │
│  │  │  Tracker → LineCross → Intrusion →      │           │ │
│  │  │  Counting → HeatMap → Privacy → LPR     │           │ │
│  │  └─────────────────┬───────────────────────┘           │ │
│  │                    │                                    │ │
│  │  ┌─────────────────▼───────────────────────┐           │ │
│  │  │           Event Dispatcher               │           │ │
│  │  │  ONVIF │ MQTT │ Webhook │ Recording      │           │ │
│  │  └─────────────────────────────────────────┘           │ │
│  └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### Key Design Decisions

1. **Single inference pass**: Run YOLO + SCRFD once per frame, feed results to all downstream features
2. **Feature modules are pure post-processors**: They consume detection results, not raw frames (except motion detection and tamper detection which need pixel data)
3. **Each feature is a separate class** with `Init()`, `ProcessFrame()`, `GetResults()`, `Shutdown()` and its own config
4. **Event-driven**: Each feature emits events to a common EventDispatcher
5. **Hot-reconfigurable**: Features can be enabled/disabled at runtime via config API

---

## Phase 0: Standalone Examples (hdal/samples/)

Create one self-contained demo per feature to verify it works on the NT98538 before touching `ipcamd`. Each demo:
- Reads live frames from sensor via HDAL pipeline
- Runs AI3 inference (where applicable)
- Prints results to stdout + optionally saves annotated output
- Is buildable with `make` standalone

### Directory Structure
```
hdal/samples/
├── ai3_feature_object_detection/     ← P0 (already: ai3_custom_yolov8)
├── ai3_feature_face_detection/       ← P0 (already: ai3_custom_yolov8 has SCRFD)
├── ai3_feature_tracking/             ← P0 (already: ai3_unified_demo)
├── ai3_feature_motion_detection/     ← P0 (new, uses vendor_cv)
├── ai3_feature_tamper_detection/     ← P0 (new, uses IVE)
├── ai3_feature_line_crossing/        ← P1 (new)
├── ai3_feature_zone_intrusion/       ← P1 (new)
├── ai3_feature_privacy_masking/      ← P1 (new, SCRFD + GFX)
├── ai3_feature_people_counting/      ← P1 (new)
├── ai3_feature_face_recognition/     ← P2 (new, SCRFD + MobileFaceNet)
├── ai3_feature_lpr/                  ← P2 (new, PlateDetector + LPRNet)
├── ai3_feature_pose_estimation/      ← P2 (new, YOLOv8-Pose)
├── ai3_feature_heat_map/             ← P2 (new)
└── ai3_feature_abandoned_object/     ← P3 (new)
```

### Example: Standalone Demo Template

Each demo follows this pattern:
```c
// 1. Init HDAL + open sensor
// 2. Open videoproc with AI-sized output (e.g., 640x640 NV12)
// 3. Load model(s) via vendor_ai3
// 4. Loop:
//    a. Pull frame from videoproc
//    b. Run AI3 inference
//    c. Post-process (feature-specific)
//    d. Print results / save output
// 5. Cleanup
```

---

## Phase 1: Core Features Integration (P0)

### 1.1 Object Detection (Person/Vehicle)

**Standalone**: Already verified via `ai3_custom_yolov8` and `ai3_custom_yolov26`

**Integration into ipcamd**:
- [ ] Create `src/npu_inference.cpp` — wraps vendor_ai3 model load/run for YOLO
- [ ] Add `include/ipcam/npu_inference.h` — `NpuInference` class
- [ ] Wire into `AnalyticsEngine::ProcessFrame()` to replace dummy detection
- [ ] Add YOLOv8 post-processing (anchors, NMS) from `yolov8_postproc.c`
- [ ] Add config key `object_detection.model_file` pointing to `/mnt/sd/models/yolov8n.bin`
- [ ] Test: enable `object_detection.enabled=true`, verify bbox overlay on RTSP stream

**Models**: `yolov8n.bin` (80-class COCO) or `yolo26n.bin`
**Input**: 640x640 NV12 → AI3 NPU
**Output**: Up to 32 bounding boxes with class + confidence

### 1.2 Face Detection (SCRFD)

**Standalone**: Already verified via `ai3_custom_yolov8` (includes SCRFD)

**Integration**:
- [ ] Add `src/face_detection.cpp` — `FaceDetectionEngine` class
- [ ] Load SCRFD model as second AI3 network
- [ ] Add SCRFD post-processing from `scrfd_postproc.c`
- [ ] Wire into `AnalyticsEngine` as secondary model (shared frame)
- [ ] Emit `DetectionResult` with `ObjectCategory::kFace` + landmark data
- [ ] Add config key `face_detection.model_file` → `/mnt/sd/models/scrfd_500m.bin`
- [ ] Test: verify face bboxes + 5 landmarks on RTSP stream

**Models**: `scrfd_500m.bin`
**Input**: 640x640 NV12 (shared with YOLO)
**Output**: Face bboxes + 5 landmarks (eyes, nose, mouth corners)

### 1.3 Multi-Object Tracking

**Standalone**: Already in `ai3_unified_demo` (IoU + Kalman + OSNet Re-ID)

**Integration**:
- [ ] Refactor existing `object_tracking.cpp` to use the proven tracker from `unified_features.c`
- [ ] Add OSNet Re-ID support (load `mobilefacenet.bin` or train OSNet model)
- [ ] Emit stable `track_id` in `DetectionResult`
- [ ] Test: verify consistent IDs across frames on RTSP stream

### 1.4 Motion Detection

**Standalone**: Exists as `ai_motion_detection_demo` using vendor_cv MDBC

**Integration**:
- [ ] Current `motion_detection.cpp` already has frame-diff + vendor_cv integration
- [ ] Verify MDBC mode works with live HDAL pipeline frames
- [ ] Ensure proper zone support (multi-zone configurable via JSON)
- [ ] Test: verify motion events trigger recording + ONVIF events

### 1.5 Tamper/Sabotage Detection

**Standalone**: Create `ai3_feature_tamper_detection/`

**Integration**:
- [ ] Current `tamper_detection.cpp` uses IVE Sobel + Histogram
- [ ] Verify all 4 modes: defocus (blur), masking (cover), scene change, exposure
- [ ] Add dwell-time logic (only alarm after N seconds of sustained tamper)
- [ ] Test: cover camera lens → verify tamper event fires

---

## Phase 2: Important Features (P1)

### 2.1 Line Crossing Detection

**Standalone**: Create `ai3_feature_line_crossing/`
```
Pipeline: YOLO detect → Track → Check if track center crossed line → Event
```

**Integration**:
- [ ] Create `src/line_crossing.cpp` — `LineCrossingEngine` class
- [ ] Add `include/ipcam/line_crossing.h`
- [ ] Track object center point; detect when it transitions from one side of a line to the other
- [ ] Support configurable lines (up to 4) with in/out direction
- [ ] Emit `LineCrossEvent` with track_id, line_id, direction
- [ ] Wire to ONVIF `tns1:RuleEngine/LineDetector/Crossed` event
- [ ] Test: walk across virtual line → verify event + counting

### 2.2 Zone Intrusion Detection

**Standalone**: Create `ai3_feature_zone_intrusion/`
```
Pipeline: YOLO detect → Track → Point-in-polygon test → Dwell timer → Event
```

**Integration**:
- [ ] Create `src/zone_intrusion.cpp` — `ZoneIntrusionEngine` class
- [ ] Implement point-in-polygon (winding number) from `unified_features.c`
- [ ] Support multiple polygon zones (up to 4)
- [ ] Add dwell-time requirement (object must remain in zone for N seconds)
- [ ] Emit `IntrusionEvent` with zone_id, track_id
- [ ] Wire to ONVIF `tns1:RuleEngine/FieldDetector/ObjectsInside` event
- [ ] Test: place object in restricted zone → verify alarm

### 2.3 Privacy Masking (Face Blur)

**Standalone**: Create `ai3_feature_privacy_masking/`
```
Pipeline: SCRFD detect faces → GFX mosaic on YUV → Encode
```

**Integration**:
- [ ] Current `privacy_mosaic.cpp` has GFX integration
- [ ] Wire SCRFD face detections as mosaic regions
- [ ] Add option: auto-detect faces vs. fixed regions
- [ ] Apply mosaic BEFORE video encoding (on videoproc output buffer)
- [ ] Test: face visible → stream shows mosaic'd face

### 2.4 People/Vehicle Counting

**Standalone**: Create `ai3_feature_people_counting/`
```
Pipeline: YOLO detect → Track → Count by class (person/vehicle) → Stats
```

**Integration**:
- [ ] Create `src/counting.cpp` — `CountingEngine` class
- [ ] Maintain rolling count per class: current, max, average, total
- [ ] Support two counting modes:
  - **Zone counting**: count objects currently inside a zone
  - **Line counting**: count objects crossing a line (in/out)
- [ ] Expose via REST API: `GET /api/analytics/counting`
- [ ] Wire to ONVIF `tns1:RuleEngine/CountAggregation` event
- [ ] Test: multiple people in view → verify count increments

---

## Phase 3: Advanced Features (P2)

### 3.1 Face Recognition

**Standalone**: Create `ai3_feature_face_recognition/`
```
Pipeline: SCRFD detect → Crop face → MobileFaceNet extract embedding → Compare to DB → Match
```

**Integration**:
- [ ] Create `src/face_recognition.cpp` — `FaceRecognitionEngine` class
- [ ] SCRFD detects face → crop & align using landmarks
- [ ] Run MobileFaceNet on cropped face → 128-dim embedding
- [ ] Compare against face database using cosine similarity
- [ ] Maintain face database on `/mnt/app/face_db/` (JSON + embeddings)
- [ ] REST API: `POST /api/faces/enroll`, `DELETE /api/faces/{id}`, `GET /api/faces/list`
- [ ] Test: enroll face → verify recognition on next appearance

**Models**: `scrfd_500m.bin` + `mobilefacenet.bin`

### 3.2 License Plate Recognition (LPR)

**Standalone**: Create `ai3_feature_lpr/`
```
Pipeline: PlateDetector detect → Crop plate → LPRNet OCR → Text
```

**Integration**:
- [ ] Create `src/lpr.cpp` — `LprEngine` class
- [ ] Stage 1: Run `plate_detector.bin` (or `real_plate_detector.bin`) to find plates
- [ ] Stage 2: Crop detected plate region
- [ ] Stage 3: Run `lprnet_ocr.bin` on cropped region → character sequence
- [ ] Maintain plate log on `/mnt/app/plate_log/`
- [ ] REST API: `GET /api/analytics/plates` (recent reads)
- [ ] Test: show license plate to camera → verify OCR text

**Models**: `plate_detector.bin` + `lprnet_ocr.bin`

### 3.3 Pose Estimation

**Standalone**: Create `ai3_feature_pose_estimation/`
```
Pipeline: YOLOv8-Pose → 17 keypoints per person
```

**Integration**:
- [ ] Create `src/pose_estimation.cpp` — `PoseEstimationEngine` class
- [ ] Load `yolov8n_pose.bin` model
- [ ] Post-process to extract 17 COCO keypoints per person
- [ ] Applications: fall detection (keypoint angle analysis), gesture recognition
- [ ] REST API: `GET /api/analytics/poses` (current skeletons)
- [ ] Test: person in view → verify 17 keypoints overlay

**Models**: `yolov8n_pose.bin`

### 3.4 Heat Map Generation

**Standalone**: Create `ai3_feature_heat_map/`
```
Pipeline: YOLO detect → Tracker → Accumulate bbox centers on grid → Decay over time
```

**Integration**:
- [ ] Create `src/heat_map.cpp` — `HeatMapEngine` class
- [ ] 32x18 grid (matching 16:9 aspect), float accumulation with exponential decay
- [ ] REST API: `GET /api/analytics/heatmap` → returns grid as JSON or PNG
- [ ] Optionally overlay semi-transparent heatmap on stream
- [ ] Daily/hourly reset option
- [ ] Test: movement in area → verify hot spots accumulate

---

## Phase 4: Future Features (P3)

### 4.1 Abandoned/Removed Object Detection

**Design** (implement after P2 is stable):
```
Pipeline: YOLO detect → Track → If object stationary for >T seconds AND
          no associated moving track → Abandoned object alert
          If background object disappears → Removed object alert
```

- Requires background model or reference frame
- Complex state machine (object must be unattended, not just parked car)
- Low priority — implement after core features are solid

---

## Implementation Order & Milestones

### Sprint 1: NPU Inference Core + Object Detection
```
1. Create ai3_feature_object_detection/ standalone demo (live camera + YOLO)
   → Verify YOLO inference on live stream, print FPS + detections
2. Create src/npu_inference.cpp in ipcam_ai module
   → Wrap vendor_ai3 load/run into C++ class
3. Wire YOLO into AnalyticsEngine::ProcessFrame()
   → Replace stub detection with real NPU inference
4. Test: enable object_detection, verify bboxes on RTSP stream
```

### Sprint 2: Face Detection + Tracking
```
1. Create ai3_feature_face_detection/ standalone (SCRFD live)
2. Add FaceDetectionEngine to ipcam_ai
3. Add SCRFD as second model in AnalyticsEngine
4. Refactor ObjectTracker to use proven IoU+Kalman from unified_features
5. Test: faces detected + stable track IDs on stream
```

### Sprint 3: Motion + Tamper Detection
```
1. Create ai3_feature_motion_detection/ standalone (vendor_cv MDBC live)
2. Verify existing motion_detection.cpp works with HDAL pipeline
3. Create ai3_feature_tamper_detection/ standalone
4. Verify existing tamper_detection.cpp IVE-based approach
5. Test: motion → event fired; cover lens → tamper event
```

### Sprint 4: Line Crossing + Zone Intrusion + Counting
```
1. Create ai3_feature_line_crossing/ standalone
2. Create ai3_feature_zone_intrusion/ standalone
3. Create ai3_feature_people_counting/ standalone
4. Integrate LineCrossingEngine into ipcam_ai
5. Integrate ZoneIntrusionEngine into ipcam_ai
6. Integrate CountingEngine into ipcam_ai
7. Test: all three features + ONVIF events
```

### Sprint 5: Privacy Masking + Heat Map
```
1. Create ai3_feature_privacy_masking/ standalone (SCRFD + GFX)
2. Wire SCRFD results into existing privacy_mosaic.cpp
3. Create ai3_feature_heat_map/ standalone
4. Integrate HeatMapEngine
5. Test: face blur on stream; heatmap via REST API
```

### Sprint 6: Face Recognition + LPR
```
1. Create ai3_feature_face_recognition/ standalone
2. Create ai3_feature_lpr/ standalone
3. Integrate FaceRecognitionEngine (SCRFD + MobileFaceNet)
4. Integrate LprEngine (PlateDetector + LPRNet)
5. Test: face enroll/recognize; plate read
```

### Sprint 7: Pose Estimation + Abandoned Object
```
1. Create ai3_feature_pose_estimation/ standalone
2. Integrate PoseEstimationEngine
3. (If time) Design abandoned object state machine
4. Test: keypoint overlay; fall detection logic
```

---

## File Changes Summary

### New files in `modules/ai/`

```
modules/ai/
├── include/ipcam/
│   ├── npu_inference.h          ← NEW: AI3 NPU wrapper
│   ├── face_detection.h         ← NEW: SCRFD face detection
│   ├── line_crossing.h          ← NEW: Line crossing engine
│   ├── zone_intrusion.h         ← NEW: Zone intrusion engine
│   ├── counting.h               ← NEW: People/vehicle counting
│   ├── face_recognition.h       ← NEW: Face recognition
│   ├── lpr.h                    ← NEW: License plate recognition
│   ├── pose_estimation.h        ← NEW: Pose estimation
│   └── heat_map.h               ← NEW: Heat map generation
├── src/
│   ├── npu_inference.cpp        ← NEW: vendor_ai3 wrapper + YOLO/SCRFD post-proc
│   ├── face_detection.cpp       ← NEW
│   ├── line_crossing.cpp        ← NEW
│   ├── zone_intrusion.cpp       ← NEW
│   ├── counting.cpp             ← NEW
│   ├── face_recognition.cpp     ← NEW
│   ├── lpr.cpp                  ← NEW
│   ├── pose_estimation.cpp      ← NEW
│   └── heat_map.cpp             ← NEW
└── tests/
    ├── test_npu_inference.cpp   ← NEW
    ├── test_line_crossing.cpp   ← NEW
    ├── test_zone_intrusion.cpp  ← NEW
    ├── test_counting.cpp        ← NEW
    └── test_face_recognition.cpp← NEW
```

### Modified files
```
modules/ai/CMakeLists.txt        ← Add new source files + link flags
modules/ai/src/analytics.cpp     ← Wire new engines into ProcessFrame()
configs/config.factory.d/analytics.json  ← Add model_file paths, new feature configs
modules/webserver/                ← REST API endpoints for counting, heatmap, faces, plates
modules/onvif/                    ← Map events to ONVIF analytics
modules/events/                   ← New event types for all features
```

---

## Testing Strategy

### Per-Feature Testing (before integration)

Each standalone demo (`ai3_feature_*`) must pass:
1. **Build test**: `make` compiles without warnings
2. **Run test**: Execute on device, verify stdout output
3. **Accuracy test**: Verify detections match expected objects
4. **Performance test**: Measure FPS, must sustain >15 FPS for P0 features
5. **Memory test**: No leaks (run for 5 minutes, check RSS stable)

### Integration Testing (after integration into ipcamd)

1. **Unit tests**: Each engine class has gtest unit tests (mock AI3)
2. **Feature toggle test**: Enable one feature at a time, verify no regression
3. **Multi-feature test**: Enable all P0 features simultaneously, verify FPS >= 10
4. **REST API test**: Verify all new endpoints return correct JSON
5. **ONVIF test**: Verify events propagate to ONVIF subscribers
6. **Config reload test**: Change config via API, verify features enable/disable
7. **Stress test**: Run 24h continuous, check for memory leaks/crashes
8. **Regression test**: Existing features (streaming, recording, web UI) still work

### Performance Budget

| Configuration | Target FPS | NPU Load |
|---|---|---|
| YOLO only | ≥ 25 fps | ~40% |
| YOLO + SCRFD | ≥ 15 fps | ~70% |
| YOLO + SCRFD + all post-processing | ≥ 12 fps | ~70% + CPU |
| YOLO + SCRFD + OSNet Re-ID | ≥ 10 fps | ~85% |
| All P0+P1 features | ≥ 10 fps | Stay under 90% |

---

## Model Deployment

All models are stored on SD card at `/mnt/sd/models/`:
```
/mnt/sd/models/
├── yolov8n.bin           # Object detection (80 classes)
├── yolo26n.bin           # Object detection (alternative)
├── scrfd_500m.bin        # Face detection
├── mobilefacenet.bin     # Face recognition embeddings
├── plate_detector.bin    # License plate detection
├── lprnet_ocr.bin        # Plate character OCR
├── yolov8n_pose.bin      # Pose estimation (17 keypoints)
└── coco_labels.txt       # Class labels
```

Config references models by path:
```json
{
  "object_detection": {
    "model_file": "/mnt/sd/models/yolov8n.bin",
    "labels_file": "/mnt/sd/models/coco_labels.txt"
  },
  "face_detection": {
    "model_file": "/mnt/sd/models/scrfd_500m.bin"
  }
}
```

---

## Risk & Mitigation

| Risk | Impact | Mitigation |
|---|---|---|
| NPU memory exhaustion with multiple models | Crash | Load only enabled models; share input buffer |
| Frame drops under heavy AI load | Poor UX | Skip N frames; reduce AI input resolution |
| Model accuracy varies with lighting | False alarms | Add confidence thresholds per-feature |
| vendor_ai3 API changes in SDK updates | Build break | Wrap in `npu_inference.cpp` abstraction |
| OSNet model not optimized for NT98538 | Slow Re-ID | Fall back to IoU-only tracking |

---

## Dependencies

Before starting, ensure these are built and available:
```bash
# Build all SDK libraries
cd ns02302_linux_sdk && source build/envsetup.sh
make library

# Verify key libraries exist
ls -la code/hdal/vendor/output/libvendor_ai3.so
ls -la code/hdal/vendor/output/libvendor_cv.so
ls -la code/hdal/vendor/output/libvendor_gfx.so
ls -la code/vos/output/libvos.so

# Verify models on device
adb shell "ls /mnt/sd/models/*.bin"
```

---

## Getting Started — First Task

**Start with Sprint 1, Step 1**: Create the `ai3_feature_object_detection/` standalone demo that runs YOLO on live camera frames. This validates the entire NPU inference pipeline end-to-end before we touch the main application.

```bash
cd ns02302_linux_sdk/code/hdal/samples
mkdir ai3_feature_object_detection
# Base on ai3_custom_yolov8 but simplified for single-model live camera
```
