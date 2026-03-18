# Analytics Engine Architecture — Production Design

## 1. Overview

This document defines the concrete module architecture for the ipcamd analytics
system.  It specifies how all detection features are organized, what runs on
which hardware engine, how NPU time is shared, and exactly which settings the
user sees in the web UI.

Everything below is grounded in what already exists in the codebase
(`analytics.cpp`, `analytics.h`, event system, web API, analytics.json) and
what we've validated in the 14 standalone ai3_features demos.

---

## 2. Resource Map — What Runs Where

The NT98538 has five execution domains.  Each feature is assigned to exactly
one primary domain.

```
┌──────────────────────────────────────────────────────────────────────┐
│                          NT98538 SoC                                 │
├─────────────┬─────────────┬──────────┬──────────┬────────────────────┤
│  Cortex-A53 │  CNN30 NPU  │ MD Engine│ IVE/VQA  │   GFX Engine       │
│  (4 cores)  │  (NN30)     │  (HW)   │  (HW)    │   (HW)             │
├─────────────┼─────────────┼──────────┼──────────┼────────────────────┤
│ Tracking    │ YOLO        │ MDBC     │ Tamper:  │ SMART_BBOX overlay │
│ Line Cross  │ SCRFD       │ GMM      │  Sobel   │ Privacy mosaic     │
│ Zone Intrus │ Face Embed  │ GlobalMD │  Histo   │ OSD text           │
│ Counting    │ LPR OCR     │ SubMD    │  VQA     │                    │
│ Heat Map    │ Pose        │ ObjDet   │          │                    │
│ Config/API  │ YAMNet      │ CrossLn  │          │                    │
│ Event mgr   │             │          │          │                    │
└─────────────┴─────────────┴──────────┴──────────┴────────────────────┘
  % CPU load:    NPU load:      ~0%       ~0%         ~0%
  5-15%          20-60ms/frame
```

**Key principle**: MD engine, IVE/VQA, and GFX run on dedicated HW blocks.
They cost zero CPU and zero NPU.  All "basic event" features use them.
Only AI features consume NPU inference time.

---

## 3. Feature Tiers — The Three-Tier Model

### Tier 0 — Always-On (Hardware-Only, No NPU)

These features run on dedicated HW blocks.  They are always available, cost
nothing, and should be enabled by default.

| Feature | HW Engine | API | Config Key |
|---------|-----------|-----|------------|
| Motion Detection (zones, global/sub alarm) | vendor MD MDBC | `MdProcessingThread()` | `motion_detection` |
| Motion Object Detection (up to 32 blobs) | vendor MD ObjDet | existing | `motion_detection` |
| Motion Cross-Line (direction) | vendor MD CrossLine | existing | `motion_detection` |
| Video Tampering — Defocus (blur) | IVE Sobel + Laplacian | `TamperDetectionEngine` | `video_tampering` |
| Video Tampering — Masking (lens covered) | IVE Histo + Sobel | `TamperDetectionEngine` | `video_tampering` |
| Video Tampering — Scene Change | IVE Histo chi-square | `TamperDetectionEngine` | `video_tampering` |
| Video Exception — Too Dark/Bright | VQA lib | `VqaEngine` | `video_tampering` |
| Audio Exception (volume spike) | CPU threshold | future | `audio_exception` |
| Motion AQ (encoder quality boost) | MD → encoder EVBR | `HD_VIDEOPROC_OUTFUNC_MD` | `motion_detection` |

### Tier 1 — AI Primary (Single YOLO Pass)

These features all share ONE YOLO inference per frame.  Enabling any one of
them triggers the YOLO pass; enabling all of them doesn't add any additional
NPU cost.

| Feature | What it does with YOLO results | CPU cost |
|---------|-------------------------------|----------|
| Smart Detection | Report person/vehicle/animal bounding boxes | ~0 |
| Object Tracking | IoU + Kalman on YOLO boxes (libtrke) | <1ms |
| Line Crossing | tracker + cross-product geometry test | <0.1ms |
| Zone Intrusion | tracker + point-in-polygon + dwell timer | <0.1ms |
| People/Vehicle Counting | tracker + entry/exit counting line | <0.1ms |
| Loitering Detection | tracker + per-zone dwell-time accumulation | <0.1ms |
| Heat Map | accumulate detection centroids into grid | <0.1ms |
| Smart Motion | classify MD blobs using YOLO results (person/vehicle vs noise) | <0.1ms |

**NPU budget**: one YOLO pass = 20-40ms on CNN30 (yolov8n/yoloxs 640x640)

### Tier 2 — AI Secondary (SCRFD Face Pass)

These features require a SECOND NPU pass with SCRFD.  Added only when the
user enables face-related features.

| Feature | Additional NPU cost | CPU cost |
|---------|-------------------|----------|
| Face Detection | SCRFD pass ~15ms | ~0 |
| Face Recognition | + ArcFace embedding per crop ~5ms | + cosine_sim on DB |
| Face Attribute | + attribute net per crop ~3ms | ~0 |
| Privacy Masking (smart) | SCRFD shared | GFX mosaic ~0 |

**NPU budget**: SCRFD ~15ms + embeddings 5ms per face (max 10) = 15-65ms

### Tier 3 — AI Tertiary (Conditional/On-Demand)

These run ONLY when triggered or explicitly enabled.  Each adds its own NPU
pass.

| Feature | NPU cost (per frame/event) | When it runs |
|---------|---------------------------|-------------|
| LPR | crop-detect ~10ms + OCR ~8ms per plate | When vehicle detected in LPR zone |
| Pose Estimation | pose-net ~12ms per person crop | When person detected (if enabled) |
| Audio Classification | YAMNet ~1.3ms (separate audio path) | Always (if enabled) |

---

## 4. Module Architecture

### 4.1 AnalyticsEngine — The Orchestrator

The existing `AnalyticsEngine` singleton becomes the **orchestrator** that
runs inference and distributes results to **consumer modules**.

```
AnalyticsEngine::ProcessingThread()              ← already exists
│
├── Pull YUV frame from proc_alg_path_           ← already exists
│
├── [if any Tier 1 feature enabled]
│   ├── NpuInference::RunYolo(frame)              ← NEW: vendor_ai3 wrapper
│   │   └── returns vector<Detection>
│   │
│   ├── ObjectTracker::Update(detections)          ← exists (object_tracking.cpp)
│   │   └── returns vector<TrackedObject>
│   │
│   ├── LineCrossingEngine::Check(tracked)         ← NEW consumer module
│   ├── ZoneIntrusionEngine::Check(tracked)        ← NEW consumer module
│   ├── CountingEngine::Update(tracked)            ← NEW consumer module
│   ├── LoiteringEngine::Update(tracked)           ← NEW consumer module
│   ├── HeatMapEngine::Accumulate(tracked)         ← NEW consumer module
│   └── SmartMotionEngine::Classify(tracked, md)   ← NEW consumer module
│
├── [if any Tier 2 feature enabled]
│   ├── NpuInference::RunScrfd(frame)              ← NEW
│   │   └── returns vector<Face>
│   │
│   ├── FaceRecognitionEngine::Match(faces)        ← NEW consumer module
│   ├── FaceAttributeEngine::Analyze(faces)        ← NEW consumer module
│   └── PrivacyMosaicEngine::Apply(faces)          ← exists (privacy_mosaic.cpp)
│
├── [if LPR enabled + vehicle detected in zone]
│   └── NpuInference::RunLpr(vehicle_crop)         ← NEW (conditional)
│
├── [if Pose enabled + person detected]
│   └── NpuInference::RunPose(person_crop)         ← NEW (conditional)
│
├── UpdateOverlay(all_results)                     ← exists (UpdateSmartBbox)
├── EmitEvents(all_results)                        ← exists (EventManager)
└── InvokeCallbacks(all_results)                   ← exists
```

### 4.2 Parallel: Tier 0 threads (already running independently)

```
MdProcessingThread()                              ← already exists (analytics.cpp)
│   └── pulls 160x120 YUV → libmd MDBC/GlobalAlarm/SubAlarm/ObjDet/CrossLine
│       └── emits EventManager::PublishMotionStart/End

TamperDetectionEngine (IVE)                       ← exists (tamper_detection.cpp)
│   └── receives downsampled Y from ProcessingThread
│       └── Sobel + Histogram → defocus/masking/scene_change
│           └── emits TamperCallback → EventManager::PublishTamperDetected

VqaEngine (vendor VQA lib)                        ← exists (vqa.cpp)
│   └── receives Y from ProcessingThread
│       └── NVT_VQA_Run → too_light/too_dark/blur
│           └── emits into same event pipeline
```

### 4.3 File/Module Inventory

**Existing files (keep as-is, extend where noted):**

| File | Role | Changes needed |
|------|------|---------------|
| `analytics.h` | Config structs + AnalyticsEngine class | Add Tier 2/3 config, consumer module pointers |
| `analytics.cpp` | Orchestrator + ProcessingThread | Add YOLO/SCRFD inference calls, consumer dispatch |
| `motion_detection.h/.cpp` | Vendor MD wrapper | None (complete) |
| `object_tracking.h/.cpp` | IoU+Kalman tracker (libtrke) | None (complete) |
| `privacy_mosaic.h/.cpp` | GFX mosaic | None (complete) |
| `tamper_detection.h/.cpp` | IVE tamper detection | None (complete) |
| `vqa.h/.cpp` | VQA blur/light | None (complete) |
| `aiisp.h/.cpp` | AI-ISP integration | None (complete) |

**New files (to implement):**

| File | Role | Priority |
|------|------|----------|
| `npu_inference.h/.cpp` | vendor_ai3 wrapper: load model, run inference, get output | **P0 — blocks everything** |
| `yolo_postproc.h/.cpp` | Decode YOLO output → vector<Detection> (from ai3_features demos) | P0 |
| `scrfd_postproc.h/.cpp` | Decode SCRFD output → vector<Face> (from ai3_features demos) | P1 |
| `line_crossing.h/.cpp` | Consumer: tracked objects × line geometry → cross events | P1 |
| `zone_intrusion.h/.cpp` | Consumer: tracked objects × polygon zones → intrusion events | P1 |
| `counting.h/.cpp` | Consumer: tracked objects × counting line → entry/exit counts | P1 |
| `loitering.h/.cpp` | Consumer: tracked objects × zones × dwell time → alerts | P2 |
| `heat_map.h/.cpp` | Consumer: accumulate centroids → spatial grid → image export | P2 |
| `smart_motion.h/.cpp` | Consumer: correlate MD blobs with YOLO detections → classified motion | P2 |
| `face_recognition.h/.cpp` | SCRFD face → ArcFace embedding → DB match → whitelist/blacklist | P2 |
| `face_attribute.h/.cpp` | SCRFD face → attribute model → age/gender/glasses | P3 |
| `lpr.h/.cpp` | Vehicle crop → plate detector → OCR → plate DB match | P3 |
| `pose_estimation.h/.cpp` | Person crop → pose model → keypoints → fall detection | P3 |
| `audio_classification.h/.cpp` | Audio pipe → mel spectrogram → YAMNet → class label | P3 |

---

## 5. NPU Scheduling — The Budget Model

### 5.1 Time Budget

At 15 fps AI processing (good enough — most cameras use 10-15 fps for AI):

```
Frame budget = 1000ms / 15fps = 66.7ms per frame

Allocation:
  YOLO inference:      25ms  (yolov8n 640x640 on CNN30)
  SCRFD inference:     15ms  (SCRFD-500M on CNN30)
  Face embeddings:      5ms  (per face, up to 10 = 50ms worst case)
  LPR (conditional):   18ms  (only when vehicle in zone)
  Pose (conditional):  12ms  (only when person detected)
  CPU post-proc:        2ms  (all consumer modules combined)
  ─────────────────────────
  Typical frame:       42ms  (YOLO + SCRFD + embeddings(2))
  Worst case:          75ms  → drop to 12 fps (acceptable)
```

### 5.2 NPU Sharing Strategy

```
enum class NpuPriority {
    kPrimary,    // YOLO — always runs if any smart event enabled
    kSecondary,  // SCRFD — runs if face features enabled
    kTertiary,   // LPR/Pose — runs conditionally
};
```

The NPU processes one model at a time.  The orchestrator **serializes**
inference calls within each frame:

```cpp
void AnalyticsEngine::ProcessingThread() {
    // ... pull frame ...

    // Phase 1: YOLO (if any Tier 1 consumer is enabled)
    if (NeedYolo()) {
        auto yolo_dets = npu_->RunYolo(frame);
        auto tracked = tracker_->Update(yolo_dets);

        // Dispatch to all Tier 1 consumers (CPU only, <1ms total)
        if (line_crossing_->IsEnabled()) line_crossing_->Check(tracked);
        if (zone_intrusion_->IsEnabled()) zone_intrusion_->Check(tracked);
        if (counting_->IsEnabled())      counting_->Update(tracked);
        if (loitering_->IsEnabled())     loitering_->Update(tracked);
        if (heat_map_->IsEnabled())      heat_map_->Accumulate(tracked);
    }

    // Phase 2: SCRFD (if any Tier 2 consumer is enabled)
    if (NeedScrfd()) {
        auto faces = npu_->RunScrfd(frame);
        if (face_recognition_->IsEnabled()) face_recognition_->Match(faces);
        if (face_attribute_->IsEnabled())   face_attribute_->Analyze(faces);
        if (privacy_mosaic_->IsEnabled())   privacy_mosaic_->Apply(faces);
    }

    // Phase 3: Conditional Tier 3 models
    if (lpr_->IsEnabled()) {
        for (auto& det : tracked) {
            if (det.category == Vehicle && lpr_->InZone(det))
                lpr_->Recognize(npu_->CropAndRun(frame, det));
        }
    }
}
```

### 5.3 Helper: NeedYolo / NeedScrfd

```cpp
bool AnalyticsEngine::NeedYolo() const {
    return config_.object_detection.enabled ||
           config_.line_crossing.enabled ||
           config_.zone_intrusion.enabled ||
           config_.counting.enabled ||
           config_.heat_map.enabled ||
           config_.loitering.enabled ||
           config_.smart_motion.enabled;
}

bool AnalyticsEngine::NeedScrfd() const {
    return config_.face_detection.enabled ||
           config_.face_recognition.enabled ||
           config_.face_attribute.enabled ||
           config_.privacy_mask_smart.enabled;
}
```

---

## 6. User-Facing Feature Tree — Web UI

This is the exact menu structure the user sees.  Each leaf has an **enable
toggle**, **settings**, **schedule**, and **actions** section.

```
Configuration
├── Events
│   ├── Basic Event                                HW-only, always available
│   │   ├── Motion Detection                       vendor MD MDBC/GMM
│   │   │   ├── Enable [toggle]
│   │   │   ├── Sensitivity [slider 0-100]
│   │   │   ├── Detection Zones [draw up to 8]
│   │   │   │   └── each zone: name, enable, sensitivity, polygon
│   │   │   ├── Object Size Filter [min % / max %]
│   │   │   ├── Schedule [weekly grid]
│   │   │   └── Actions
│   │   │       ├── Recording [toggle, pre/post sec]
│   │   │       ├── Snapshot [toggle, count]
│   │   │       ├── Email [toggle, recipients, attach]
│   │   │       ├── Webhook/HTTP [toggle, url, method, headers]
│   │   │       ├── MQTT [toggle, topic, qos]
│   │   │       ├── FTP Upload [toggle, path]
│   │   │       ├── Alarm Output [toggle, port]
│   │   │       └── Audio Alarm [toggle, file, repeat]
│   │   │
│   │   ├── Video Tampering                        IVE + VQA HW
│   │   │   ├── Enable [toggle]
│   │   │   ├── Sensitivity [slider 0-100]
│   │   │   ├── Detection Types
│   │   │   │   ├── Defocus (blur) [toggle]
│   │   │   │   ├── Masking (lens covered) [toggle]
│   │   │   │   ├── Scene Change (camera moved) [toggle]
│   │   │   │   ├── Too Dark [toggle, threshold]
│   │   │   │   └── Too Bright [toggle, threshold]
│   │   │   ├── Dwell Time [seconds, default 3]
│   │   │   ├── Schedule [weekly grid]
│   │   │   └── Actions [same action set as motion]
│   │   │
│   │   ├── Audio Exception                        CPU threshold
│   │   │   ├── Enable [toggle]
│   │   │   ├── Volume Rise [toggle, sensitivity]
│   │   │   ├── Volume Fall [toggle, sensitivity]
│   │   │   └── Actions
│   │   │
│   │   └── Exception (System)
│   │       ├── Storage Full [actions]
│   │       ├── Network Disconnect [actions]
│   │       └── Illegal Access [actions]
│   │
│   ├── Smart Event                                Needs NPU (Tier 1)
│   │   ├── Smart Detection                        YOLO
│   │   │   ├── Enable [toggle]
│   │   │   ├── Target Types
│   │   │   │   ├── Person [toggle, sensitivity, min size]
│   │   │   │   ├── Vehicle [toggle, sensitivity, min size, subtypes]
│   │   │   │   └── Animal [toggle, sensitivity]
│   │   │   ├── Detection Zones [draw multiple]
│   │   │   ├── Schedule [weekly grid]
│   │   │   └── Actions
│   │   │
│   │   ├── Line Crossing                          YOLO + tracker
│   │   │   ├── Enable [toggle]
│   │   │   ├── Lines [draw up to 4 lines]
│   │   │   │   └── each: direction (A→B / B→A / both), color
│   │   │   ├── Target: Person □  Vehicle □
│   │   │   ├── Counting [toggle, daily reset, export]
│   │   │   ├── Schedule
│   │   │   └── Actions
│   │   │
│   │   ├── Zone Intrusion                         YOLO + tracker
│   │   │   ├── Enable [toggle]
│   │   │   ├── Zones [draw polygons, up to 4]
│   │   │   │   └── each: dwell time, target types
│   │   │   ├── Target: Person □  Vehicle □
│   │   │   ├── Schedule
│   │   │   └── Actions
│   │   │
│   │   ├── People Counting                        YOLO + tracker
│   │   │   ├── Enable [toggle]
│   │   │   ├── Counting Line [draw]
│   │   │   ├── OSD Display [entry/exit on stream]
│   │   │   ├── Daily Reset [time]
│   │   │   ├── Statistics Export [csv download]
│   │   │   ├── Threshold Alert [max occupancy]
│   │   │   └── Actions (on threshold)
│   │   │
│   │   ├── Loitering Detection                    YOLO + tracker
│   │   │   ├── Enable [toggle]
│   │   │   ├── Zone [draw]
│   │   │   ├── Min Dwell Time [seconds]
│   │   │   ├── Target: Person □  Vehicle □
│   │   │   └── Actions
│   │   │
│   │   └── Heat Map                               YOLO (accumulate)
│   │       ├── Enable [toggle]
│   │       ├── Target: Person □  Vehicle □
│   │       ├── Reset Interval [hours]
│   │       └── Export [image download]
│   │
│   ├── Face Event                                 Needs NPU (Tier 2)
│   │   ├── Face Detection
│   │   │   ├── Enable [toggle]
│   │   │   ├── Min Face Size [pixels]
│   │   │   ├── Detection Area [draw zone]
│   │   │   └── Overlay [bounding box on stream]
│   │   │
│   │   ├── Face Recognition                       SCRFD + ArcFace
│   │   │   ├── Enable [toggle]
│   │   │   ├── Similarity Threshold [%]
│   │   │   ├── Face Database Management
│   │   │   │   ├── Groups: Whitelist / Blacklist / VIP
│   │   │   │   ├── Add face [upload photo + name]
│   │   │   │   ├── Import / Export [CSV + photos]
│   │   │   │   └── Capacity: up to 10,000 faces
│   │   │   ├── Stranger Detection [toggle]
│   │   │   └── Actions (per group)
│   │   │
│   │   └── Face Attribute
│   │       ├── Enable [toggle]
│   │       ├── Attributes: Age □  Gender □  Glasses □  Mask □
│   │       └── OSD Display
│   │
│   └── Vehicle Event                              Needs NPU (Tier 3)
│       ├── License Plate Recognition
│       │   ├── Enable [toggle]
│       │   ├── Detection Zone [draw]
│       │   ├── Plate Database Management
│       │   │   ├── Whitelist / Blacklist
│       │   │   ├── Import / Export
│       │   │   └── Capacity: up to 100,000 plates
│       │   ├── Stranger Plate Detection [toggle]
│       │   └── Actions (per list)
│       │
│       └── Vehicle Type Classification
│           ├── Enable [toggle]
│           └── Types: Car / Truck / Bus / Motorcycle / Bicycle
│
├── Image
│   ├── Privacy Mask (static)                      manual drawn regions
│   ├── Privacy Mask (smart)                       auto-blur faces (SCRFD)
│   └── OSD Settings                               device name, date, etc.
│
├── Audio
│   └── Audio Classification                       YAMNet (Tier 3)
│       ├── Enable [toggle]
│       ├── Classes: Gunshot □  Glass Break □  Scream □  Dog □
│       ├── Sensitivity per class
│       └── Actions
│
└── AI Configuration                               Resource management
    ├── Performance Mode
    │   ├── Quality (15 fps AI, full resolution)
    │   ├── Balanced (10 fps AI, balanced) [default]
    │   └── Performance (5 fps AI, fast response)
    ├── Active Features Summary
    │   └── Shows: "YOLO: ON, SCRFD: ON, LPR: OFF, Pose: OFF"
    │   └── NPU utilization bar: [████████░░] 78%
    └── Model Selection (advanced)
        ├── Object Detection: yolov8n / yoloxs / yolov5s
        └── Face Detection: SCRFD-500M / SCRFD-2.5G
```

---

## 7. Web API Mapping

All endpoints below already have stubs in
`modules/webserver/src/handlers/analytics/`.  The table shows which need
implementation.

### 7.1 Existing Endpoints (implemented)

```
GET/PUT  /api/v1/analytics                    Overall config
GET/POST /api/v1/analytics/status             Start/stop/reset/stats
GET      /api/v1/analytics/capabilities       Feature list + model info

GET/PUT  /api/v1/analytics/motion             Motion detection config
GET/PUT  /api/v1/analytics/tampering          Video tampering config
GET/PUT  /api/v1/analytics/smart              Smart detection config
GET/PUT  /api/v1/analytics/linecross          Line crossing config
GET/PUT  /api/v1/analytics/intrusion          Zone intrusion config
```

### 7.2 New Endpoints (to implement)

```
# People counting
GET/PUT  /api/v1/analytics/counting           Config
GET      /api/v1/analytics/counting/stats     Current in/out counts
POST     /api/v1/analytics/counting/reset     Reset counters
GET      /api/v1/analytics/counting/export    CSV export

# Loitering
GET/PUT  /api/v1/analytics/loitering          Config + zones

# Heat map
GET/PUT  /api/v1/analytics/heatmap            Config
GET      /api/v1/analytics/heatmap/image      Current heat map PNG
POST     /api/v1/analytics/heatmap/reset      Reset accumulation

# Face detection/recognition
GET/PUT  /api/v1/analytics/face               Config
GET/POST /api/v1/analytics/face/database      Face DB CRUD
GET/POST /api/v1/analytics/face/database/:id  Individual entry
POST     /api/v1/analytics/face/import        Bulk import
GET      /api/v1/analytics/face/export        Bulk export
GET      /api/v1/analytics/face/recent        Recent detections log

# LPR
GET/PUT  /api/v1/analytics/lpr               Config
GET/POST /api/v1/analytics/lpr/database      Plate DB CRUD
GET      /api/v1/analytics/lpr/recent        Recent plate reads

# Audio
GET/PUT  /api/v1/analytics/audio             Config (audio exception + classification)

# AI resource management
GET      /api/v1/analytics/ai/status         NPU utilization, loaded models
PUT      /api/v1/analytics/ai/performance    Performance mode
```

---

## 8. Event Types — Complete Matrix

Maps each feature to its event types and what data the event carries.

```
Feature                  Event Type                    Event Data
─────────────────────────────────────────────────────────────────
Motion Detection         kMotionStart                  zone_id, sensitivity %
                         kMotionEnd                    zone_id, duration_ms
                         kMotionContinuous             zone_id, diff %

Video Tampering          kTamperDetected               tamper_type bitmask, metrics
                         kTamperCleared                tamper_type

Audio Exception          kAudioAbnormalRise            volume_db
                         kAudioAbnormalFall            volume_db

Smart Detection          kPersonDetected               bbox, confidence, track_id
                         kVehicleDetected              bbox, confidence, type, track_id
                         kAnimalDetected               bbox, confidence, track_id
                         kPersonLost                   track_id, duration_ms
                         kVehicleLost                  track_id, duration_ms

Line Crossing            kLineCrossedAtoB              line_id, track_id, category
                         kLineCrossedBtoA              line_id, track_id, category

Zone Intrusion           kZoneEntered                  zone_id, track_id, category
                         kZoneExited                   zone_id, track_id, dwell_ms
                         kZoneLoitering                zone_id, track_id, dwell_ms

People Counting          kCountThresholdExceeded       current_count, threshold
                         kCountReset                   final_in, final_out

Face Detection           kFaceDetected                 bbox, landmarks, face_quality
Face Recognition         kFaceRecognized               face_id, name, group, similarity%
                         kFaceStranger                 snapshot_path

LPR                      kPlateRecognized              plate_text, confidence, list_match
                         kPlateStranger                plate_text, snapshot_path

Audio Classification     kAudioClassified              class_name, confidence

Heat Map                 (no events — data endpoint only)

Pose                     kFallDetected                 person_id, keypoints
```

---

## 9. Config JSON Schema — analytics.json

The existing `analytics.json` already covers motion, smart, linecross,
intrusion, and tampering.  Below are the additions needed.

### 9.1 New sections to add to analytics.json

```json
{
    "people_counting": {
        "enabled": false,
        "counting_line": {
            "id": 1,
            "start": {"x": 0, "y": 50},
            "end": {"x": 100, "y": 50},
            "direction": "both"
        },
        "osd_display": true,
        "daily_reset": true,
        "reset_time": "00:00",
        "threshold_alert": {
            "enabled": false,
            "max_occupancy": 100
        },
        "actions": { ... }
    },

    "loitering": {
        "enabled": false,
        "zones": [
            {
                "id": 1,
                "name": "Parking Area",
                "enabled": false,
                "points": [...],
                "dwell_time_sec": 30,
                "detect_pedestrian": true,
                "detect_vehicle": false
            }
        ],
        "actions": { ... }
    },

    "heat_map": {
        "enabled": false,
        "target_types": ["pedestrian", "vehicle"],
        "resolution": {"w": 64, "h": 36},
        "reset_interval_hours": 24,
        "color_scheme": "jet"
    },

    "face_detection": {
        "enabled": false,
        "model": "scrfd_500m",
        "min_face_size": 30,
        "confidence_threshold": 0.6,
        "detection_zone": {
            "points": [...]
        },
        "recognition": {
            "enabled": false,
            "database_path": "/mnt/app/face_db",
            "similarity_threshold": 0.7,
            "stranger_detection": false,
            "groups": [
                {"name": "whitelist", "actions": { ... }},
                {"name": "blacklist", "actions": { ... }}
            ]
        },
        "attribute": {
            "enabled": false,
            "detect_age": true,
            "detect_gender": true,
            "detect_glasses": true,
            "detect_mask": true
        }
    },

    "license_plate": {
        "enabled": false,
        "detection_zone": {
            "points": [...]
        },
        "database_path": "/mnt/app/plate_db",
        "groups": [
            {"name": "whitelist", "actions": { ... }},
            {"name": "blacklist", "actions": { ... }}
        ],
        "stranger_detection": false,
        "actions": { ... }
    },

    "audio_classification": {
        "enabled": false,
        "classes": {
            "gunshot": {"enabled": true, "sensitivity": 70},
            "glass_break": {"enabled": true, "sensitivity": 70},
            "scream": {"enabled": true, "sensitivity": 50},
            "dog_bark": {"enabled": false, "sensitivity": 50},
            "siren": {"enabled": false, "sensitivity": 50}
        },
        "actions": { ... }
    },

    "audio_exception": {
        "enabled": false,
        "volume_rise": {"enabled": true, "sensitivity": 50},
        "volume_fall": {"enabled": false, "sensitivity": 50},
        "actions": { ... }
    },

    "ai_performance": {
        "mode": "balanced",
        "ai_fps": 10,
        "object_model": "yolov8n",
        "face_model": "scrfd_500m"
    }
}
```

---

## 10. Database Schemas

### 10.1 Face Database (SQLite: /mnt/app/face_db/faces.db)

```sql
CREATE TABLE face_groups (
    id          INTEGER PRIMARY KEY,
    name        TEXT NOT NULL UNIQUE,       -- 'whitelist', 'blacklist', 'vip'
    description TEXT,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE faces (
    id          INTEGER PRIMARY KEY,
    name        TEXT NOT NULL,
    group_id    INTEGER REFERENCES face_groups(id),
    embedding   BLOB NOT NULL,              -- 512-float ArcFace vector
    photo_path  TEXT,                        -- /mnt/app/face_db/photos/<id>.jpg
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at  DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE face_events (
    id          INTEGER PRIMARY KEY,
    face_id     INTEGER REFERENCES faces(id),  -- NULL if stranger
    similarity  REAL,
    group_name  TEXT,
    snapshot    TEXT,                         -- path to snapshot jpg
    timestamp   DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### 10.2 Plate Database (SQLite: /mnt/app/plate_db/plates.db)

```sql
CREATE TABLE plate_groups (
    id   INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE               -- 'whitelist', 'blacklist'
);

CREATE TABLE plates (
    id          INTEGER PRIMARY KEY,
    plate_text  TEXT NOT NULL,
    group_id    INTEGER REFERENCES plate_groups(id),
    owner_name  TEXT,
    notes       TEXT,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE plate_events (
    id          INTEGER PRIMARY KEY,
    plate_id    INTEGER REFERENCES plates(id),  -- NULL if stranger
    plate_text  TEXT NOT NULL,
    confidence  REAL,
    group_name  TEXT,
    snapshot    TEXT,
    timestamp   DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### 10.3 Analytics Database (SQLite: /mnt/app/analytics.db)

```sql
CREATE TABLE counting_data (
    id          INTEGER PRIMARY KEY,
    line_id     INTEGER NOT NULL,
    direction   TEXT NOT NULL,               -- 'in' or 'out'
    object_type TEXT NOT NULL,               -- 'person', 'vehicle'
    timestamp   DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE counting_daily (
    id          INTEGER PRIMARY KEY,
    date        DATE NOT NULL,
    line_id     INTEGER NOT NULL,
    total_in    INTEGER DEFAULT 0,
    total_out   INTEGER DEFAULT 0,
    UNIQUE(date, line_id)
);

CREATE TABLE heatmap_data (
    id          INTEGER PRIMARY KEY,
    grid_x      INTEGER NOT NULL,
    grid_y      INTEGER NOT NULL,
    object_type TEXT NOT NULL,
    count       INTEGER DEFAULT 0,
    period_start DATETIME NOT NULL,
    period_end   DATETIME,
    UNIQUE(grid_x, grid_y, object_type, period_start)
);
```

---

## 11. Overlay Strategy

All visual feedback is drawn on the encoded stream using the existing
`UpdateSmartBbox()` mechanism and the GFX hardware engine.

### 11.1 What gets drawn

| Feature | Overlay element | Color |
|---------|----------------|-------|
| Smart Detection / Person | Green bbox + "Person 87%" | `#00FF00` |
| Smart Detection / Vehicle | Red bbox + "Car 92%" | `#FF0000` |
| Smart Detection / Animal | Blue bbox + "Dog 78%" | `#0000FF` |
| Object Tracking | Tracking ID on bbox | white text |
| Line Crossing | Yellow line + "IN:5 OUT:3" | `#FFFF00` |
| Zone Intrusion | Cyan polygon outline | `#00FFFF` |
| People Counting | OSD bar "In: 45  Out: 38  Now: 7" | white text |
| Face Detection | Magenta bbox | `#FF00FF` |
| Face Recognition | bbox + "John Smith 95%" / "STRANGER" | green/red |
| LPR | bbox around plate + plate text | orange |
| Pose / Skeleton | skeleton lines | white |
| Privacy Mask (static) | filled rectangle (GFX) | black |
| Privacy Mask (smart) | mosaic/blur on face (GFX pixelation) | pixelated |
| Tamper / VQA alerts | Full-screen border flash | red |

### 11.2 Overlay priority (when too many objects)

Max 32 SMART_BBOX entries.  Priority order:
1. Privacy masks (always drawn first to ensure privacy)
2. Face recognition matches (high-value alerts)
3. LPR matches (high-value alerts)
4. Zone intrusion / line crossing alerts (active alarms)
5. Tracked person/vehicle bboxes (most recent first)
6. Counting OSD (always visible if enabled)

---

## 12. Implementation Order

### Phase 1 — NPU Backend (P0, blocks all AI features)
```
New files:  npu_inference.h/.cpp
Modify:     analytics.cpp (add inference calls to ProcessingThread)
Test:       YOLO running in ProcessingThread, detections logged
Depends on: vendor_ai3 library, model files on /mnt/app/ai_models/
```

### Phase 2 — Core Smart Events (P1)
```
New files:  yolo_postproc.h/.cpp (port from ai3_features demos)
            line_crossing.h/.cpp
            zone_intrusion.h/.cpp
            counting.h/.cpp
Modify:     analytics.cpp (dispatch to consumer modules)
            analytics.json (add counting, loitering sections)
            web handlers (implement counting/loitering endpoints)
Test:       Enable smart detection + line crossing → events fire
```

### Phase 3 — Face Pipeline (P2)
```
New files:  scrfd_postproc.h/.cpp
            face_recognition.h/.cpp
            face_attribute.h/.cpp
Modify:     analytics.cpp (add SCRFD inference phase)
            analytics.json (face section)
            web handlers (face database CRUD)
Test:       Face detection → recognition → whitelist/blacklist match
```

### Phase 4 — Advanced Features (P3)
```
New files:  lpr.h/.cpp
            pose_estimation.h/.cpp
            audio_classification.h/.cpp
            loitering.h/.cpp
            heat_map.h/.cpp
            smart_motion.h/.cpp
Test:       LPR on vehicle detection, pose on person detection
```

---

## 13. What NOT to Do

1. **Don't create a separate process for each feature.**
   Everything runs in one AnalyticsEngine process.  Consumer modules are
   lightweight classes that receive detections, not separate threads.

2. **Don't run YOLO per feature.**
   One YOLO pass feeds all Tier 1 consumers.  The dispatching is just
   function calls, no IPC.

3. **Don't duplicate the tracker.**
   Object tracking runs once and produces tracked objects.  Line crossing,
   zone intrusion, counting, and loitering all consume the SAME tracked
   objects.

4. **Don't use IVE/VQA for features that need AI.**
   IVE Sobel can detect "something changed" but can't tell you it's a
   person vs a tree.  That's YOLO's job.  Use IVE only for Tier 0 features
   where classification isn't needed.

5. **Don't optimize CPU post-processing.**
   Post-proc for 32 objects is <1ms.  The bottleneck is NPU inference time,
   not CPU.  Focus optimization effort on model selection and quantization.

---

## 14. Summary — How Features Ship to the User

```
┌─────────────────────────────────────────────────────────┐
│                    User's Perspective                     │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  "Basic Events" tab:                                     │
│    ✅ Motion Detection     → always available, free      │
│    ✅ Video Tampering      → always available, free      │
│    ✅ Audio Exception      → always available, free      │
│                                                          │
│  "Smart Events" tab:                                     │
│    ☐ Smart Detection       ─┐                            │
│    ☐ Line Crossing         ─┤ enable ANY of these →      │
│    ☐ Zone Intrusion        ─┤ ONE YOLO pass runs         │
│    ☐ People Counting       ─┤ (no additional cost)       │
│    ☐ Loitering             ─┤                            │
│    ☐ Heat Map              ─┘                            │
│                                                          │
│  "Face Events" tab:                                      │
│    ☐ Face Detection        ─┐ enable ANY of these →      │
│    ☐ Face Recognition      ─┤ SCRFD pass added           │
│    ☐ Face Attribute        ─┘                            │
│                                                          │
│  "Vehicle Events" tab:                                   │
│    ☐ License Plate         → runs LPR on vehicle crops   │
│                                                          │
│  "Audio AI" tab:                                         │
│    ☐ Audio Classification  → YAMNet (separate audio)     │
│                                                          │
│  "AI Settings":                                          │
│    Performance mode: [Balanced ▼]                        │
│    NPU load: [████████░░] 78%                            │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

The user never thinks about "demos" or "models."  They see **features with
toggle switches**.  Behind the scenes, we manage the NPU budget automatically.

Each feature has the same consistent settings pattern:
**Enable → Configure → Schedule → Actions**

That's it.  One YOLO pass.  Six features for free.
