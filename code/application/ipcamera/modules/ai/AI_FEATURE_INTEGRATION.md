# AI Feature Integration — From Demos to Production IP Camera

## Table of Contents

1. [Current State Assessment](#current-state-assessment)
2. [Architecture: How It All Fits Together](#architecture)
3. [Phase 1: NPU Inference Backend (Foundation)](#phase-1)
4. [Phase 2: Core Detection Features (P0)](#phase-2)
5. [Phase 3: Rule-Based Analytics (P1)](#phase-3)
6. [Phase 4: Recognition & Database Management (P2)](#phase-4)
7. [Phase 5: Advanced Analytics (P2-P3)](#phase-5)
8. [Database Schemas](#database-schemas)
9. [Web API Endpoints](#web-api-endpoints)
10. [Event System Integration](#event-system-integration)
11. [Overlay & OSD Strategy](#overlay-strategy)
12. [NPU Scheduling & Resource Management](#npu-scheduling)
13. [Configuration Reference](#configuration-reference)
14. [Implementation Checklist](#implementation-checklist)

---

## 1. Current State Assessment <a name="current-state-assessment"></a>

### What Exists in ipcamd (Working)

| Component | Status | File |
|-----------|--------|------|
| AnalyticsEngine scaffold | ✅ Skeleton | `analytics.cpp` |
| Motion Detection (vendor libmd) | ✅ Working | `motion_detection.cpp` |
| Object Tracking (IoU+Kalman via libtrke) | ✅ Working | `object_tracking.cpp` |
| Privacy Mosaic (vendor GFX) | ✅ Working | `privacy_mosaic.cpp` |
| Tamper Detection (IVE Sobel+Histo) | ✅ Working | `tamper_detection.cpp` |
| AI-ISP (vendor aiisp) | ✅ Working | `aiisp.cpp` |
| VQA (Video Quality Analysis) | ✅ Working | `vqa.cpp` |
| Event system (rules, actions, dispatch) | ✅ Working | `modules/events/` |
| ONVIF metadata streaming (RTP) | ✅ Working | `onvif_metadata.cpp` |
| SMART_BBOX overlay on encoded stream | ✅ Framework | `analytics.cpp` |
| Web API for motion/tamper/line/intrusion | ✅ Routes exist | `api_handlers.cpp` |
| Config system (JSON files) | ✅ Working | `analytics.json` |

### What Does NOT Exist Yet (Must Build)

| Component | Status | Gap |
|-----------|--------|-----|
| NPU inference backend (vendor_ai3 wrapper) | ❌ Missing | No model load/run in ipcamd |
| YOLO post-processing (NMS, anchors) | ❌ Missing | Exists only in demos |
| SCRFD post-processing (face landmarks) | ❌ Missing | Exists only in demos |
| Face Recognition (embedding DB) | ❌ Missing | Config struct only |
| Face Attribute analysis | ❌ Missing | Demo 13 standalone only |
| License Plate Recognition | ❌ Missing | Config struct only |
| People/Vehicle Counting engine | ❌ Missing | Config struct only |
| Heat Map accumulation | ❌ Missing | Demo 12 standalone only |
| Audio Classification | ❌ Missing | Demo 14 standalone only |
| Pose Estimation | ❌ Missing | Demo 11 standalone only |
| Face database CRUD API | ❌ Missing | Planned route commented out |
| LPR database CRUD API | ❌ Missing | Planned route commented out |
| Counting statistics API | ❌ Missing | Planned route commented out |
| Combined analytics dashboard data | ❌ Missing | No statistics aggregation |

### Demo to Production Feature Mapping

| Demo | What It Does | ipcamd Module | Integration Effort |
|------|-------------|---------------|-------------------|
| 01_object_detection | YOLO inference + NMS | `npu_inference.cpp` (NEW) | **Major** — NPU backend |
| 02_face_detection | SCRFD inference + decode | `face_detection.cpp` (NEW) | Medium — shares NPU backend |
| 03_motion_detection | Pixel diff (educational) | Already uses vendor libmd | **None** — vendor is better |
| 04_object_tracking | IoU matching + track ID | Already uses libtrke | **None** — vendor is better |
| 05_line_crossing | Track center crosses line | `line_crossing.cpp` (NEW) | Small — pure geometry |
| 06_zone_intrusion | Point-in-polygon + dwell | `zone_intrusion.cpp` (NEW) | Small — pure geometry |
| 07_privacy_masking | SCRFD → GFX blur | Already exists | **Wire** SCRFD detections →  |
| 08_people_counting | Track count by class | `counting.cpp` (NEW) | Small — track bookkeeping |
| 09_face_recognition | SCRFD → ArcFace → cosine | `face_recognition.cpp` (NEW) | Medium — +face DB |
| 10_lpr | PlateDetect → LPRNet → OCR | `lpr.cpp` (NEW) | Medium — +plate DB |
| 11_pose_estimation | YOLOv8-Pose → keypoints | `pose_estimation.cpp` (NEW) | Small — post-proc only |
| 12_heat_map | Track accumulation grid | `heat_map.cpp` (NEW) | Small — grid counters |
| 13_face_attribute | Age/Gender/Emotion | `face_attribute.cpp` (NEW) | Medium — extra model |
| 14_audio_classification | Mel → YAMNet NPU | `audio_classification.cpp` (NEW) | Medium — audio pipeline |

---

## 2. Architecture <a name="architecture"></a>

### What Real IP Cameras Do (Not What Demos Do)

Demos: Load image → NPU → draw boxes → save JPEG  
Production: Live sensor → NPU → **events** + **overlays** + **databases** + **notifications**

```
                        ┌───────────────────────────────────────────────────────┐
                        │                    ipcamd                             │
                        │                                                       │
 Sensor ──► ISP ──►┌────┤  VideoProc                                           │
                   │    │  ├── OUT_0 (1920×1080) → H.264 Encoder → RTSP/Record │
                   │    │  ├── OUT_1 (704×576)   → H.264 Encoder → RTSP Sub    │
                   │    │  ├── OUT_2 (640×360)   → AI Processing ──────────┐   │
                   │    │  └── OUT_3 (160×120)   → Motion Detection (libmd)│   │
                   │    │                                                   │   │
                   │    │  ┌────────────────────────────────────────────────┘   │
                   │    │  │                                                     │
                   │    │  ▼                                                     │
                   │    │  ┌─────────────────────────────────────────────────┐   │
                   │    │  │           NpuInferenceManager                    │   │
                   │    │  │                                                  │   │
                   │    │  │  ┌─────────────┐  ┌─────────────┐              │   │
                   │    │  │  │ YOLO Model  │  │ SCRFD Model │  ...more     │   │
                   │    │  │  │ (person/    │  │ (face det)  │  models      │   │
                   │    │  │  │  vehicle)   │  │             │              │   │
                   │    │  │  └──────┬──────┘  └──────┬──────┘              │   │
                   │    │  │         │ detections      │ faces              │   │
                   │    │  └─────────┼────────────────┼──────────────────────┘   │
                   │    │            │                 │                         │
                   │    │            ▼                 ▼                         │
                   │    │  ┌─────────────────────────────────────────────────┐   │
                   │    │  │           Feature Processing Pipeline           │   │
                   │    │  │                                                  │   │
                   │    │  │  ObjectTracker ──► LineCrossing                 │   │
                   │    │  │       │           ZoneIntrusion                 │   │
                   │    │  │       │           PeopleCounting                │   │
                   │    │  │       │           HeatMap                       │   │
                   │    │  │       │           AbandonedObject               │   │
                   │    │  │       │                                          │   │
                   │    │  │  FaceRecognition ──► FaceAttribute              │   │
                   │    │  │       │               PrivacyMosaic             │   │
                   │    │  │       │                                          │   │
                   │    │  │  LPR ──► PlateDatabase                         │   │
                   │    │  │                                                  │   │
                   │    │  │  PoseEstimation ──► Fall Detection (future)     │   │
                   │    │  │                                                  │   │
                   │    │  │  AudioClassification ──► Sound Alerts           │   │
                   │    │  │                                                  │   │
                   │    │  └──────────────────┬──────────────────────────────┘   │
                   │    │                     │                                  │
                   │    │         ┌───────────┼───────────┐                     │
                   │    │         ▼           ▼           ▼                     │
                   │    │  ┌──────────┐ ┌──────────┐ ┌──────────┐              │
                   │    │  │ Overlay  │ │  Event   │ │ Database │              │
                   │    │  │ Engine   │ │ Manager  │ │ Manager  │              │
                   │    │  │          │ │          │ │          │              │
                   │    │  │ SMART_   │ │ Record   │ │ FaceDB   │              │
                   │    │  │ BBOX on  │ │ Snapshot │ │ PlateDB  │              │
                   │    │  │ encoder  │ │ Webhook  │ │ EventDB  │              │
                   │    │  │          │ │ MQTT     │ │ CountDB  │              │
                   │    │  │ OSD text │ │ Email    │ │ HeatDB   │              │
                   │    │  │ (counts, │ │ ONVIF    │ │          │              │
                   │    │  │  labels) │ │ FTP      │ │          │              │
                   │    │  └──────────┘ └──────────┘ └──────────┘              │
                   │    │                                                       │
                   │    │  ┌─────────────────────────────────────────────────┐   │
                   │    │  │              Web API / ONVIF                     │   │
                   │    │  │  /api/v1/analytics/*    ONVIF Analytics Service  │   │
                   │    │  │  /api/v1/faces/*        Face DB management      │   │
                   │    │  │  /api/v1/plates/*       LPR DB management       │   │
                   │    │  │  /api/v1/counting/*     Counting statistics     │   │
                   │    │  │  /api/v1/heatmap/*      Heatmap data export     │   │
                   │    │  └─────────────────────────────────────────────────┘   │
                   │    │                                                       │
 Audio ──► ALSA ──►│    │  AudioClassification (separate thread, ~7ms/frame)    │
                   │    │                                                       │
                   └────┘                                                       │
                        └───────────────────────────────────────────────────────┘
```

### Key Design Rules

1. **Single NPU inference pass for YOLO per frame** — all features that need person/vehicle bboxes (tracking, line crossing, intrusion, counting, heatmap) share ONE set of YOLO detections. Never run YOLO twice.

2. **NPU models are loaded/unloaded based on enabled features** — don't load SCRFD if face detection is disabled. Don't load ArcFace if recognition is disabled.

3. **Features are pure post-processors** — they consume `DetectionResult[]` arrays, not raw frames. Exception: motion detection (pixel-level), tamper detection (histogram), audio classification (audio buffer).

4. **Events drive actions, not features** — features emit events (`PublishPersonDetected`, `PublishLineCrossed`). The EventManager matches events to rules and executes actions (record, snapshot, webhook, MQTT, email).

5. **Overlays use SMART_BBOX for bboxes, OSD for text** — bounding boxes are drawn by the video encoder hardware (zero CPU). Counting text and labels use the OSD overlay system.

6. **Databases are SQLite** — face embeddings, plate text, counting stats, heatmap grids all stored in SQLite on SD card or flash. Encrypted with SQLCipher like existing DBs.

---

## 3. Phase 1: NPU Inference Backend (Foundation) <a name="phase-1"></a>

This is the critical missing piece. Without this, no AI detection works in ipcamd.

### New Files to Create

```
modules/ai/
├── include/ipcam/
│   ├── npu_inference.h          # NPU model manager (load/run/unload)
│   ├── yolo_postproc.h          # YOLO NMS + anchor decode
│   ├── scrfd_postproc.h         # SCRFD face decode + landmarks
│   └── npu_model_registry.h     # Which models are loaded
├── src/
│   ├── npu_inference.cpp         # vendor_ai3 wrapper
│   ├── yolo_postproc.cpp         # From demo 01 yolov8_postproc.c
│   └── scrfd_postproc.cpp        # From demo 02 scrfd_postproc.c
```

### NpuInferenceManager Design

```cpp
class NpuInferenceManager {
public:
    static NpuInferenceManager& Instance();
    
    // Lifecycle
    bool Init();                           // Initialize vendor_ai3 engine
    void Shutdown();                       // Unload all models, shutdown
    
    // Model management
    bool LoadModel(const std::string& name, const std::string& bin_path,
                   int input_w, int input_h, ModelFormat fmt);
    bool UnloadModel(const std::string& name);
    bool IsModelLoaded(const std::string& name) const;
    
    // Inference
    // Runs model on NV12 frame, returns raw output tensor(s)
    bool RunInference(const std::string& model_name,
                      const uint8_t* nv12_data, int width, int height,
                      std::vector<TensorOutput>& outputs);
    
    // Combined pipelines
    std::vector<DetectionResult> RunYoloDetection(
        const uint8_t* nv12_data, int width, int height,
        float conf_thresh, float nms_thresh);
    
    std::vector<FaceResult> RunScrfdDetection(
        const uint8_t* nv12_data, int width, int height,
        float conf_thresh);
    
    std::vector<float> RunFaceEmbedding(
        const uint8_t* face_crop, int width, int height);  // 112x112 aligned face
    
    std::string RunLprOcr(
        const uint8_t* plate_crop, int width, int height);  // 94x24 aligned plate
    
    // Statistics
    struct ModelStats {
        std::string name;
        uint64_t inference_count;
        float avg_inference_ms;
        float last_inference_ms;
        size_t model_size_bytes;
    };
    std::vector<ModelStats> GetStats() const;
    
private:
    // vendor_ai3 handles per model
    struct LoadedModel {
        std::string name;
        std::string path;
        uint32_t proc_id;       // AI3 process ID
        int input_w, input_h;
        ModelFormat format;
        ModelStats stats;
    };
    std::map<std::string, LoadedModel> models_;
    std::mutex mutex_;
    bool initialized_ = false;
};
```

### Integration into AnalyticsEngine

```cpp
// In analytics.cpp ProcessingThread():
void AnalyticsEngine::ProcessingThread() {
    auto& npu = NpuInferenceManager::Instance();
    
    while (running_) {
        // 1. Pull NV12 frame from VideoProc OUT_2
        HD_VIDEO_FRAME frame;
        hd_videoproc_pull_out_buf(proc_alg_path_, &frame, -1);
        
        // 2. Run YOLO if object detection or dependent features enabled
        std::vector<DetectionResult> yolo_dets;
        if (NeedYolo()) {  // object_det || line_cross || intrusion || counting || heatmap
            yolo_dets = npu.RunYoloDetection(frame.data, 640, 360,
                config_.object_detection.confidence_threshold,
                config_.object_detection.nms_threshold);
        }
        
        // 3. Run SCRFD if face detection enabled
        std::vector<FaceResult> faces;
        if (NeedScrfd()) {  // face_det || face_rec || privacy
            faces = npu.RunScrfdDetection(frame.data, 640, 360,
                config_.face_detection.confidence_threshold);
        }
        
        // 4. Feed results to downstream features (all are cheap post-processors)
        auto tracked = tracker_.Update(yolo_dets);
        
        if (config_.line_crossing.enabled)
            line_crossing_.ProcessTracked(tracked);
        if (config_.zone_intrusion.enabled)
            zone_intrusion_.ProcessTracked(tracked);
        if (config_.object_detection.counting.enabled)
            counting_.ProcessTracked(tracked);
        if (heat_map_enabled_)
            heat_map_.Accumulate(tracked);
        if (config_.face_detection.recognition.enabled)
            face_rec_.ProcessFaces(faces, frame.data, frame.width, frame.height);
        
        // 5. Build overlay + emit events
        UpdateSmartBbox(tracked, faces);
        EmitEvents(tracked, faces);
        
        // 6. Release frame
        hd_videoproc_release_out_buf(proc_alg_path_, &frame);
    }
}
```

### Post-Processing Code Migration

From demos to ipcamd — these are the C functions that decode raw NPU output tensors into bounding boxes:

| Demo Source | Function | Target in ipcamd |
|-------------|----------|-------------------|
| `01_object_detection/yolov8_postproc.c` | `yolov8_postprocess()` | `yolo_postproc.cpp` |
| `01_object_detection/yolov8_postproc.c` | `nms_sorted_bboxes()` | `yolo_postproc.cpp` |
| `02_face_detection/scrfd_postproc.c` | `scrfd_postprocess()` | `scrfd_postproc.cpp` |
| `02_face_detection/scrfd_postproc.c` | `generate_anchors()` | `scrfd_postproc.cpp` |
| `10_lpr/lprnet_postproc.c` | `lprnet_postprocess()` | `lpr.cpp` |
| `11_pose_estimation/pose_postproc.c` | `pose_postprocess()` | `pose_estimation.cpp` |

These get wrapped in C++ classes with proper memory management but the core math stays identical.

---

## 4. Phase 2: Core Detection Features (P0) <a name="phase-2"></a>

### 4.1 Object Detection (Person/Vehicle/Animal)

**What users see**: Bounding boxes on RTSP stream. Push notification "Person detected at front door".

**Implementation**:
- Wire `NpuInferenceManager::RunYoloDetection()` into processing thread
- SMART_BBOX draws colored rectangles on encoded H.264 (hardware, zero CPU)
- Emit `kPersonDetected` / `kVehicleDetected` events to EventManager
- Events → configured actions (record 30s, snapshot, webhook to NVR, MQTT to Home Assistant)

**Overlay**: SMART_BBOX (hardware encoder feature) — up to 32 colored rectangles:
```cpp
// In analytics.cpp UpdateSmartBbox():
HD_VIDEOENC_SMART_BBOX_INFO bbox_info;
for (auto& det : detections) {
    bbox_info.rects[i] = { det.pixel_x1, det.pixel_y1, 
                           det.pixel_x2 - det.pixel_x1,
                           det.pixel_y2 - det.pixel_y1 };
    bbox_info.colors[i] = det.category == kPerson ? 0xFF0000 : 0x00FF00;
}
hd_videoenc_set(enc_path_, HD_VIDEOENC_PARAM_SMART_BBOX, &bbox_info);
```

**Config** (`analytics.json`):
```json
{
    "object_detection": {
        "enabled": true,
        "model_file": "/mnt/app/ai_models/yolov8n.bin",
        "confidence_threshold": 0.5,
        "nms_threshold": 0.45,
        "classes": {
            "person":  { "enabled": true, "min_confidence": 0.5, "color": "#FF0000" },
            "vehicle": { "enabled": true, "min_confidence": 0.5, "color": "#00FF00" },
            "animal":  { "enabled": false }
        },
        "tracking": { "enabled": true, "max_age": 30, "iou_threshold": 0.3 }
    }
}
```

### 4.2 Face Detection

**What users see**: Face boxes (optionally blurred). Privacy masking auto-mode.

**Implementation**:
- Wire `NpuInferenceManager::RunScrfdDetection()` into processing thread  
- Emit `kFaceDetected` event with landmark data
- If privacy masking is enabled, feed face bboxes to PrivacyMosaicEngine

### 4.3 Multi-Object Tracking

**Already working** via `object_tracking.cpp` (libtrke). Produces stable `track_id` per object.

The tracker output feeds directly into line crossing, zone intrusion, and counting.

---

## 5. Phase 3: Rule-Based Analytics (P1) <a name="phase-3"></a>

These features consume tracked object positions and apply spatial/temporal logic.

### 5.1 Line Crossing Detection

**What users see**: Virtual line on the stream. Count display. Alert when someone crosses.

**New file**: `src/line_crossing.cpp`

```cpp
class LineCrossingEngine {
public:
    void ProcessTracked(const std::vector<TrackedObject>& tracked);
    
    // Returns true if track center crossed the line this frame
    // Uses cross product sign change between consecutive frames
    bool CheckCrossing(uint32_t line_id, float prev_cx, float prev_cy,
                       float curr_cx, float curr_cy);
    
    // Configuration
    void AddLine(const LineCrossLine& line);
    void RemoveLine(uint32_t id);
    
    // Counting (persistent, survives restart from DB)
    struct LineCounts {
        uint32_t line_id;
        int64_t count_in;
        int64_t count_out;
        int64_t count_total;
        uint64_t last_reset_time;
    };
    LineCounts GetCounts(uint32_t line_id) const;
    void ResetCounts(uint32_t line_id);
    
private:
    // Store previous center position per track_id
    std::map<uint32_t, std::pair<float, float>> prev_positions_;
    std::vector<LineCrossLine> lines_;
    std::map<uint32_t, LineCounts> counts_;
};
```

**Event**: `kLineCrossedLeftToRight` / `kLineCrossedRightToLeft`  
**Overlay**: Line drawn via SMART_BBOX (2 rects forming a line) + OSD text showing count  
**ONVIF**: `tns1:RuleEngine/LineDetector/Crossed`

### 5.2 Zone Intrusion Detection

**What users see**: Polygon zone on stream. Alert after N seconds of presence.

**New file**: `src/zone_intrusion.cpp`

```cpp
class ZoneIntrusionEngine {
public:
    void ProcessTracked(const std::vector<TrackedObject>& tracked);
    
    // Point-in-polygon (winding number algorithm)
    bool IsInsideZone(uint32_t zone_id, float cx, float cy) const;
    
    // Loitering detection: track dwell time inside zone
    struct DwellState {
        uint32_t track_id;
        uint32_t zone_id;
        uint64_t enter_time_ms;
        bool alarm_triggered;
    };
    
    void AddZone(const IntrusionZone& zone);
    void RemoveZone(uint32_t id);
    
private:
    std::vector<IntrusionZone> zones_;
    std::map<uint64_t, DwellState> dwell_states_;  // key = (zone_id << 32 | track_id)
};
```

**Event**: `kZoneEntered` → wait → `kZoneLoitering`  
**ONVIF**: `tns1:RuleEngine/FieldDetector/ObjectsInside`

### 5.3 People/Vehicle Counting

**What users see**: OSD overlay "People: 5 | Vehicles: 3" on stream. Dashboard graphs over time.

**New file**: `src/counting.cpp`

```cpp
class CountingEngine {
public:
    // Two modes:
    enum class Mode { kZone, kLine };
    
    // Zone mode: count objects currently inside a polygon
    // Line mode: count objects crossing a line (cumulative)
    
    struct Counts {
        int people_current;       // Currently visible persons
        int vehicles_current;     // Currently visible vehicles
        int64_t people_total;     // All-time person count
        int64_t vehicles_total;   // All-time vehicle count
        int people_peak;          // Peak concurrent persons today
        int vehicles_peak;        // Peak concurrent vehicles today
    };
    
    void ProcessTracked(const std::vector<TrackedObject>& tracked);
    
    Counts GetCounts() const;
    void ResetDaily();
    
    // Periodic persistence to DB every 60 seconds
    void PersistToDb();
    
    // Statistics export (for heatmap/dashboard)
    struct HourlyStat {
        int hour;
        int people_avg;
        int vehicles_avg;
    };
    std::vector<HourlyStat> GetHourlyStats(const std::string& date) const;
};
```

**OSD Overlay**: Text line on stream — configurable position via web UI  
**API**: `GET /api/v1/analytics/counting` → current counts + time series  
**DB Table**: `counting_stats` (see Database Schemas section)

### 5.4 Heat Map

**What users see**: In web UI — colored overlay showing where people walk most.

**New file**: `src/heat_map.cpp`

```cpp
class HeatMapEngine {
public:
    // Accumulate presence from tracked objects
    void Accumulate(const std::vector<TrackedObject>& tracked);
    
    // Grid is 80x45 cells (640/8 × 360/8) — each cell counts frames of presence
    static constexpr int GRID_W = 80;
    static constexpr int GRID_H = 45;
    
    // Export as JSON grid or PNG image
    std::vector<std::vector<uint32_t>> GetGrid() const;
    std::vector<uint8_t> RenderPng(int width, int height) const;  // For web UI
    
    void Reset();
    void PersistToDb();  // Save hourly snapshots
    
private:
    std::array<uint32_t, GRID_W * GRID_H> grid_{};
    uint64_t frame_count_ = 0;
};
```

**API**: `GET /api/v1/analytics/heatmap?date=2026-02-28&hour=14` → returns grid JSON  
**No overlay on RTSP** — heatmap is web UI only (rendered client-side with canvas)

---

## 6. Phase 4: Recognition & Database Management (P2) <a name="phase-4"></a>

This is where IP cameras differentiate from simple motion cameras. Each recognition feature needs:
- **A database** for known entities (faces, plates)
- **CRUD API** for managing the database
- **Real-time matching** against live detections
- **Events** when known/unknown entities are detected
- **Web UI** for enrollment and management

### 6.1 Face Recognition System

**Pipeline**: SCRFD detect → Align face (5 landmarks → affine) → ArcFace/MobileFaceNet embed → Compare to DB

**New files**:
```
modules/ai/
├── include/ipcam/
│   ├── face_recognition.h
│   └── face_database.h
├── src/
│   ├── face_recognition.cpp
│   └── face_database.cpp
```

#### Face Database Design

```cpp
class FaceDatabase {
public:
    static FaceDatabase& Instance();
    
    bool Init(const std::string& db_path);  // SQLite DB
    
    // Person management
    struct Person {
        int64_t id;
        std::string name;
        std::string group;            // "employee", "visitor", "vip", "blocked"
        std::string notes;
        std::vector<float> embedding; // 512-d face embedding (primary)
        std::vector<std::vector<float>> extra_embeddings; // Multiple angles
        std::string thumbnail_path;   // JPEG thumbnail of enrolled face
        uint64_t created_at;
        uint64_t last_seen_at;
        int total_recognitions;
    };
    
    // CRUD
    int64_t AddPerson(const std::string& name, const std::string& group,
                      const std::vector<float>& embedding,
                      const uint8_t* face_jpeg, size_t jpeg_size);
    bool UpdatePerson(int64_t id, const std::string& name, const std::string& group);
    bool DeletePerson(int64_t id);
    bool AddEmbedding(int64_t person_id, const std::vector<float>& embedding);
    std::vector<Person> ListPersons(const std::string& group_filter = "");
    Person GetPerson(int64_t id);
    
    // Search (cosine similarity against all stored embeddings)
    struct MatchResult {
        int64_t person_id;
        std::string name;
        std::string group;
        float similarity;
    };
    MatchResult Search(const std::vector<float>& embedding, float threshold = 0.7f);
    
    // Batch operations
    bool ImportPersons(const std::string& zip_path);  // ZIP of folders: name/1.jpg, name/2.jpg
    bool ExportPersons(const std::string& zip_path);
    
    // Statistics
    int GetTotalPersons() const;
    int GetTotalEmbeddings() const;
    
private:
    // In-memory embedding cache for fast search (loaded at Init)
    struct EmbeddingEntry {
        int64_t person_id;
        std::vector<float> embedding;
    };
    std::vector<EmbeddingEntry> embedding_cache_;
    void RebuildCache();
};
```

#### Face Recognition Engine

```cpp
class FaceRecognitionEngine {
public:
    void ProcessFaces(const std::vector<FaceResult>& faces,
                      const uint8_t* frame, int width, int height);
    
    // For each detected face:
    // 1. Crop and align using 5 landmarks → 112×112
    // 2. Run ArcFace/MobileFaceNet NPU → 512-d embedding
    // 3. Search FaceDatabase → match or "unknown"
    // 4. Emit event:
    //    - kFaceRecognized (name, group, similarity)
    //    - kUnknownFaceDetected (if no match above threshold)
    //    - kBlockedPersonDetected (if matched group = "blocked")
    
    struct RecognitionResult {
        int face_index;
        BoundingBox bbox;
        int64_t person_id;      // -1 if unknown
        std::string name;       // "" if unknown
        std::string group;
        float similarity;
        bool is_blocked;
    };
    
    std::vector<RecognitionResult> GetLastResults() const;
};
```

#### Face Recognition Events

| Event | When | Actions |
|-------|------|---------|
| `kFaceRecognized` | Known face matches DB entry | Log, ONVIF notify |
| `kUnknownFaceDetected` | Face detected but no DB match | Snapshot, webhook to NVR |
| `kBlockedPersonDetected` | Face matches "blocked" group | Record, alarm, email, siren |
| `kVipPersonDetected` | Face matches "vip" group | Webhook to access control |

#### Face API Endpoints

```
POST   /api/v1/faces                    # Enroll new person (multipart: name, group, photo)
GET    /api/v1/faces                    # List all persons (paginated, filter by group)
GET    /api/v1/faces/:id                # Get person details + thumbnail
PUT    /api/v1/faces/:id                # Update name/group
DELETE /api/v1/faces/:id                # Delete person and all embeddings
POST   /api/v1/faces/:id/photos         # Add extra face photo for better recognition
DELETE /api/v1/faces/:id/photos/:pid    # Remove a face photo
GET    /api/v1/faces/:id/history        # Recognition history for this person
POST   /api/v1/faces/search             # Search by uploaded photo (returns matches)
POST   /api/v1/faces/import             # Bulk import from ZIP
GET    /api/v1/faces/export             # Bulk export as ZIP
GET    /api/v1/faces/groups             # List all groups
GET    /api/v1/faces/stats              # Total persons, recognitions today, etc.
```

### 6.2 Face Attribute Analysis

**Pipeline**: SCRFD detect → Crop face → Age/Gender/Expression model → Annotate

**New file**: `src/face_attribute.cpp`

```cpp
struct FaceAttributes {
    int age_estimate;           // 0-100
    std::string gender;         // "male" / "female"
    std::string expression;     // "neutral", "happy", "sad", "angry", "surprised"
    float glasses_confidence;   // 0.0-1.0 (wearing glasses?)
    float mask_confidence;      // 0.0-1.0 (wearing mask?)
};

class FaceAttributeEngine {
public:
    FaceAttributes Analyze(const uint8_t* face_crop, int width, int height);
    
    // Enriches face recognition results with attributes
    void ProcessFaces(const std::vector<FaceResult>& faces,
                      const uint8_t* frame, int width, int height);
};
```

**Use case**: Enriches face events with demographics. "Unknown male, ~35 years, wearing glasses at entrance."

### 6.3 License Plate Recognition

**Pipeline**: YOLO detect vehicle → Crop → PlateDetector → Crop plate → LPRNet OCR → Database match

**New files**:
```
modules/ai/
├── include/ipcam/
│   ├── lpr.h
│   └── plate_database.h
├── src/
│   ├── lpr.cpp
│   └── plate_database.cpp
```

#### Plate Database Design

```cpp
class PlateDatabase {
public:
    static PlateDatabase& Instance();
    
    struct Plate {
        int64_t id;
        std::string plate_text;       // "ABC-1234"
        std::string group;            // "employee", "visitor", "blocked", "vip"
        std::string owner_name;       // Optional
        std::string vehicle_desc;     // "Red Toyota Camry"
        std::string notes;
        uint64_t created_at;
        uint64_t last_seen_at;
        int total_recognitions;
    };
    
    // CRUD
    int64_t AddPlate(const std::string& text, const std::string& group,
                     const std::string& owner = "", const std::string& desc = "");
    bool UpdatePlate(int64_t id, const Plate& plate);
    bool DeletePlate(int64_t id);
    std::vector<Plate> ListPlates(const std::string& group_filter = "");
    
    // Search
    struct LprMatchResult {
        int64_t plate_id;
        std::string plate_text;
        std::string group;
        std::string owner_name;
        float text_similarity;  // Fuzzy match (handles OCR errors)
    };
    LprMatchResult Search(const std::string& detected_text);
    
    // History
    struct PlateLog {
        int64_t id;
        std::string plate_text;
        std::string thumbnail_path;
        uint64_t timestamp;
        float confidence;
        std::string direction;  // "in" / "out"
    };
    std::vector<PlateLog> GetHistory(int limit = 100, uint64_t since = 0);
    
    // Import/Export
    bool ImportCsv(const std::string& csv_path);
    bool ExportCsv(const std::string& csv_path);
};
```

#### LPR Engine

```cpp
class LprEngine {
public:
    void ProcessVehicles(const std::vector<DetectionResult>& vehicles,
                         const uint8_t* frame, int width, int height);
    
    // For each vehicle detection:
    // 1. Crop vehicle region
    // 2. Run PlateDetector NPU → plate bbox + confidence
    // 3. Crop plate, resize to 94×24
    // 4. Run LPRNet NPU → character sequence
    // 5. Search PlateDatabase → match or log unknown
    // 6. Emit event
    
    struct LprResult {
        BoundingBox vehicle_bbox;
        BoundingBox plate_bbox;      // Relative to vehicle crop
        std::string plate_text;
        float detection_confidence;
        float ocr_confidence;
        int64_t db_plate_id;         // -1 if unknown
        std::string db_group;
    };
};
```

#### LPR Events

| Event | When | Actions |
|-------|------|---------|
| `kPlateRecognized` | Known plate matches DB | Log, ONVIF, webhook |
| `kUnknownPlateDetected` | Plate read but not in DB | Snapshot, log |
| `kBlockedPlateDetected` | Plate in "blocked" group | Record, alarm, email |
| `kVipPlateDetected` | Plate in "vip" group | Webhook, gate open |

#### LPR API Endpoints

```
POST   /api/v1/plates                   # Add plate to database
GET    /api/v1/plates                   # List plates (paginated, filter by group)
GET    /api/v1/plates/:id               # Get plate details
PUT    /api/v1/plates/:id               # Update plate info
DELETE /api/v1/plates/:id               # Delete plate
GET    /api/v1/plates/history           # All plate readings (paginated)
POST   /api/v1/plates/search            # Search by plate text (fuzzy match)
POST   /api/v1/plates/import            # Import from CSV
GET    /api/v1/plates/export            # Export as CSV
GET    /api/v1/plates/stats             # Total plates, readings today
GET    /api/v1/plates/groups            # List groups
```

---

## 7. Phase 5: Advanced Analytics (P2-P3) <a name="phase-5"></a>

### 7.1 Pose Estimation

**What users see**: Skeleton overlay on detected persons. Fall detection alerts.

**Implementation**: Run YOLOv8-Pose, decode 17 keypoints per person.

**Future**: Fall detection = "hip center drops below knee height for >2 seconds"

### 7.2 Audio Classification

**What users see**: Alerts for glass breaking, gunshot, baby crying, dog barking, smoke alarm.

**Implementation**: Separate thread reads ALSA audio → mel spectrogram → YAMNet NPU → events

```cpp
class AudioClassificationEngine {
    // Runs in separate thread
    void AudioThread();
    
    // Reads 960ms audio chunks (16kHz mono)
    // Computes mel spectrogram (96×64) using NEON FFT
    // Runs YAMNet NPU inference (~1.3ms)
    // If class confidence > threshold → emit kAudioEventDetected
    
    struct AudioConfig {
        bool enabled = false;
        float glass_break_threshold = 0.7;
        float gunshot_threshold = 0.8;
        float scream_threshold = 0.7;
        float baby_cry_threshold = 0.6;
        float dog_bark_threshold = 0.6;
        float alarm_threshold = 0.7;
        float siren_threshold = 0.7;
    };
};
```

**Events**: `kAudioGlassBreak`, `kAudioGunshot`, `kAudioScream`, `kAudioBabyCry`, etc.

### 7.3 Abandoned/Removed Object Detection

**What users see**: Alert when an object appears or disappears from a static scene.

**Implementation**: Track objects. If a new object remains stationary for >N seconds → abandoned. If a tracked background feature disappears → removed.

---

## 8. Database Schemas <a name="database-schemas"></a>

All new tables in a single `analytics.db` (SQLCipher encrypted, on SD card or flash).

### Face Database Tables

```sql
-- Person enrollment
CREATE TABLE IF NOT EXISTS face_persons (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    group_name TEXT NOT NULL DEFAULT 'visitor',  -- employee, visitor, vip, blocked
    notes TEXT DEFAULT '',
    thumbnail_path TEXT,                          -- /mnt/sd/faces/thumbs/{id}.jpg
    created_at INTEGER NOT NULL,
    updated_at INTEGER NOT NULL,
    last_seen_at INTEGER DEFAULT 0,
    total_recognitions INTEGER DEFAULT 0
);
CREATE INDEX IF NOT EXISTS idx_face_persons_group ON face_persons(group_name);
CREATE INDEX IF NOT EXISTS idx_face_persons_name ON face_persons(name);

-- Face embeddings (multiple per person for different angles)
CREATE TABLE IF NOT EXISTS face_embeddings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    person_id INTEGER NOT NULL,
    embedding BLOB NOT NULL,          -- 512 x float32 = 2048 bytes
    source_path TEXT,                 -- Path to original enrollment photo
    quality_score FLOAT DEFAULT 0.0,  -- Face quality metric (frontality, sharpness)
    created_at INTEGER NOT NULL,
    FOREIGN KEY (person_id) REFERENCES face_persons(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_face_embeddings_person ON face_embeddings(person_id);

-- Face recognition log
CREATE TABLE IF NOT EXISTS face_recognition_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    person_id INTEGER,                    -- NULL if unknown face
    person_name TEXT,
    group_name TEXT,
    similarity REAL,
    bbox_x1 REAL, bbox_y1 REAL, bbox_x2 REAL, bbox_y2 REAL,
    snapshot_path TEXT,                    -- Full frame snapshot
    face_crop_path TEXT,                  -- Cropped face image
    age_estimate INTEGER,                 -- From face attributes (if enabled)
    gender TEXT,
    expression TEXT,
    timestamp INTEGER NOT NULL,
    FOREIGN KEY (person_id) REFERENCES face_persons(id) ON DELETE SET NULL
);
CREATE INDEX IF NOT EXISTS idx_face_log_person ON face_recognition_log(person_id);
CREATE INDEX IF NOT EXISTS idx_face_log_time ON face_recognition_log(timestamp);

-- Auto-cleanup (keep last 10000 entries)
CREATE TRIGGER IF NOT EXISTS limit_face_log AFTER INSERT ON face_recognition_log BEGIN
    DELETE FROM face_recognition_log
    WHERE id NOT IN (SELECT id FROM face_recognition_log ORDER BY id DESC LIMIT 10000);
END;
```

### License Plate Database Tables

```sql
-- Plate enrollment
CREATE TABLE IF NOT EXISTS lpr_plates (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    plate_text TEXT NOT NULL UNIQUE,
    group_name TEXT NOT NULL DEFAULT 'unknown',  -- employee, visitor, vip, blocked
    owner_name TEXT DEFAULT '',
    vehicle_description TEXT DEFAULT '',
    notes TEXT DEFAULT '',
    created_at INTEGER NOT NULL,
    updated_at INTEGER NOT NULL,
    last_seen_at INTEGER DEFAULT 0,
    total_recognitions INTEGER DEFAULT 0
);
CREATE INDEX IF NOT EXISTS idx_lpr_plates_text ON lpr_plates(plate_text);
CREATE INDEX IF NOT EXISTS idx_lpr_plates_group ON lpr_plates(group_name);

-- Plate recognition log
CREATE TABLE IF NOT EXISTS lpr_recognition_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    plate_id INTEGER,                     -- NULL if unknown plate
    plate_text TEXT NOT NULL,
    group_name TEXT,
    owner_name TEXT,
    confidence REAL,
    vehicle_bbox_x1 REAL, vehicle_bbox_y1 REAL,
    vehicle_bbox_x2 REAL, vehicle_bbox_y2 REAL,
    snapshot_path TEXT,
    plate_crop_path TEXT,
    direction TEXT DEFAULT 'unknown',     -- in, out, unknown
    timestamp INTEGER NOT NULL,
    FOREIGN KEY (plate_id) REFERENCES lpr_plates(id) ON DELETE SET NULL
);
CREATE INDEX IF NOT EXISTS idx_lpr_log_plate ON lpr_recognition_log(plate_id);
CREATE INDEX IF NOT EXISTS idx_lpr_log_time ON lpr_recognition_log(timestamp);
CREATE INDEX IF NOT EXISTS idx_lpr_log_text ON lpr_recognition_log(plate_text);

-- Auto-cleanup (keep last 50000 entries)
CREATE TRIGGER IF NOT EXISTS limit_lpr_log AFTER INSERT ON lpr_recognition_log BEGIN
    DELETE FROM lpr_recognition_log
    WHERE id NOT IN (SELECT id FROM lpr_recognition_log ORDER BY id DESC LIMIT 50000);
END;
```

### Counting & Statistics Tables

```sql
-- Real-time counting state (survives restart)
CREATE TABLE IF NOT EXISTS counting_state (
    id INTEGER PRIMARY KEY,              -- Always 1 (single row)
    people_total INTEGER DEFAULT 0,
    vehicles_total INTEGER DEFAULT 0,
    people_peak_today INTEGER DEFAULT 0,
    vehicles_peak_today INTEGER DEFAULT 0,
    last_reset_date TEXT,                 -- YYYY-MM-DD
    updated_at INTEGER NOT NULL
);

-- Hourly statistics (for dashboard graphs)
CREATE TABLE IF NOT EXISTS counting_hourly (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date TEXT NOT NULL,                   -- YYYY-MM-DD
    hour INTEGER NOT NULL,               -- 0-23
    people_avg INTEGER DEFAULT 0,
    people_peak INTEGER DEFAULT 0,
    vehicles_avg INTEGER DEFAULT 0,
    vehicles_peak INTEGER DEFAULT 0,
    UNIQUE(date, hour)
);
CREATE INDEX IF NOT EXISTS idx_counting_hourly_date ON counting_hourly(date);

-- Auto-cleanup (keep 90 days)
CREATE TRIGGER IF NOT EXISTS limit_counting_hourly AFTER INSERT ON counting_hourly BEGIN
    DELETE FROM counting_hourly WHERE date < date('now', '-90 days');
END;

-- Line crossing counts
CREATE TABLE IF NOT EXISTS line_crossing_counts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    line_id INTEGER NOT NULL,
    line_name TEXT NOT NULL,
    count_in INTEGER DEFAULT 0,
    count_out INTEGER DEFAULT 0,
    period_start INTEGER NOT NULL,        -- Epoch timestamp
    period_end INTEGER,
    UNIQUE(line_id, period_start)
);
CREATE INDEX IF NOT EXISTS idx_line_counts_line ON line_crossing_counts(line_id);
CREATE INDEX IF NOT EXISTS idx_line_counts_period ON line_crossing_counts(period_start);
```

### Heat Map Tables

```sql
-- Hourly heatmap snapshots (80×45 grid compressed as binary blob)
CREATE TABLE IF NOT EXISTS heatmap_snapshots (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date TEXT NOT NULL,                   -- YYYY-MM-DD
    hour INTEGER NOT NULL,               -- 0-23
    grid_data BLOB NOT NULL,             -- 80*45*4 bytes (uint32 per cell)
    frame_count INTEGER DEFAULT 0,
    UNIQUE(date, hour)
);
CREATE INDEX IF NOT EXISTS idx_heatmap_date ON heatmap_snapshots(date);

-- Auto-cleanup (keep 30 days)
CREATE TRIGGER IF NOT EXISTS limit_heatmap AFTER INSERT ON heatmap_snapshots BEGIN
    DELETE FROM heatmap_snapshots WHERE date < date('now', '-30 days');
END;
```

---

## 9. Web API Endpoints <a name="web-api-endpoints"></a>

### New Routes to Implement

#### Face Management (`/api/v1/faces/`)
| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/v1/faces` | Enroll person (multipart: name, group, photo JPEG) |
| GET | `/api/v1/faces` | List persons (query: group, page, limit) |
| GET | `/api/v1/faces/:id` | Get person details + thumbnail URL |
| PUT | `/api/v1/faces/:id` | Update person name/group/notes |
| DELETE | `/api/v1/faces/:id` | Delete person + all embeddings |
| POST | `/api/v1/faces/:id/photos` | Add additional face photo |
| DELETE | `/api/v1/faces/:id/photos/:pid` | Remove a face photo |
| GET | `/api/v1/faces/:id/history` | Recognition events for this person |
| POST | `/api/v1/faces/search` | Upload photo → find matching person |
| POST | `/api/v1/faces/import` | Bulk import ZIP (folders per person) |
| GET | `/api/v1/faces/export` | Bulk export ZIP |
| GET | `/api/v1/faces/groups` | List groups with counts |
| GET | `/api/v1/faces/stats` | DB stats (total, today's recognitions) |
| GET | `/api/v1/faces/recent` | Recent recognition events (all persons) |

#### LPR Management (`/api/v1/plates/`)
| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/v1/plates` | Add plate (text, group, owner, description) |
| GET | `/api/v1/plates` | List plates (query: group, page, limit) |
| GET | `/api/v1/plates/:id` | Get plate details |
| PUT | `/api/v1/plates/:id` | Update plate info |
| DELETE | `/api/v1/plates/:id` | Delete plate |
| GET | `/api/v1/plates/history` | All plate readings (paginated, date range) |
| POST | `/api/v1/plates/search` | Search by plate text (fuzzy) |
| POST | `/api/v1/plates/import` | Import CSV (plate_text, group, owner) |
| GET | `/api/v1/plates/export` | Export CSV |
| GET | `/api/v1/plates/groups` | List groups with counts |
| GET | `/api/v1/plates/stats` | DB stats |
| GET | `/api/v1/plates/recent` | Recent plate readings |

#### Counting & Statistics (`/api/v1/analytics/counting/`)
| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/v1/analytics/counting` | Current real-time counts |
| GET | `/api/v1/analytics/counting/history` | Hourly history (query: date range) |
| POST | `/api/v1/analytics/counting/reset` | Reset counters |
| GET | `/api/v1/analytics/counting/lines` | Per-line crossing counts |
| POST | `/api/v1/analytics/counting/lines/:id/reset` | Reset specific line count |

#### Heat Map (`/api/v1/analytics/heatmap/`)
| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/v1/analytics/heatmap` | Current heatmap grid (JSON) |
| GET | `/api/v1/analytics/heatmap/image` | Rendered heatmap PNG |
| GET | `/api/v1/analytics/heatmap/history` | Historical heatmap (query: date, hour) |
| POST | `/api/v1/analytics/heatmap/reset` | Clear current heatmap |

#### Audio Analytics (`/api/v1/analytics/audio/`)
| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/v1/analytics/audio` | Audio classification config |
| PUT | `/api/v1/analytics/audio` | Update config (thresholds, enabled classes) |
| GET | `/api/v1/analytics/audio/status` | Current classification status |
| GET | `/api/v1/analytics/audio/history` | Recent audio events |

#### Combined Dashboard (`/api/v1/analytics/dashboard/`)
| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/v1/analytics/dashboard` | Combined stats for web UI dashboard |
| GET | `/api/v1/analytics/dashboard/timeline` | Event timeline (all types mixed) |
| GET | `/api/v1/analytics/models` | Loaded AI models and their stats |

---

## 10. Event System Integration <a name="event-system-integration"></a>

### New Event Types to Add

```cpp
// In event_types.h EventType enum, add:

// Face recognition events
kFaceRecognized,            // Known face matched
kUnknownFaceDetected,       // Face detected but not in DB
kBlockedPersonDetected,     // Face matches "blocked" group
kVipPersonDetected,         // Face matches "vip" group

// License plate events
kPlateRecognized,           // Known plate matched
kUnknownPlateDetected,      // Plate read but not in DB
kBlockedPlateDetected,      // Plate in "blocked" group
kVipPlateDetected,          // Plate in "vip" group

// Audio events
kAudioGlassBreak,
kAudioGunshot,
kAudioScream,
kAudioBabyCry,
kAudioDogBark,
kAudioAlarm,
kAudioSiren,
kAudioAbnormalSound,

// Counting events
kOccupancyThresholdExceeded, // People count > configured max
kOccupancyCleared,           // Back below threshold

// Pose events (future)
kFallDetected,
kFallCleared,
```

### New Event Data Types

```cpp
// In event_types.h, add:

struct FaceEventData {
    int64_t person_id;           // -1 if unknown
    std::string person_name;
    std::string group;
    float similarity;
    BoundingBox face_bbox;
    int age_estimate;
    std::string gender;
    std::string expression;
};

struct PlateEventData {
    int64_t plate_id;            // -1 if unknown
    std::string plate_text;
    std::string group;
    std::string owner_name;
    float confidence;
    BoundingBox vehicle_bbox;
    BoundingBox plate_bbox;
    std::string direction;
};

struct AudioEventData {
    std::string sound_class;     // "glass_break", "gunshot", etc.
    float confidence;
    float duration_ms;
};

struct OccupancyEventData {
    int current_count;
    int threshold;
    std::string zone_name;
};
```

### Event Flow Example: Person Enters Building

```
1. Sensor frame → VideoProc OUT_2 (640×360 NV12)
2. YOLO inference → person bbox detected (confidence=0.87)
3. ObjectTracker → assigned track_id=42
4. LineCrossingEngine → track_id=42 crossed "Entry Line" left→right
   → EventManager::PublishLineCrossed(line_id=1, direction="in", track_id=42, "person")
   → Actions: increment count_in, log event
5. ZoneIntrusionEngine → track_id=42 entered "Lobby Zone"
   → (waiting for dwell_time 3s before alarm)
6. SCRFD inference → face detected at person's bbox
7. FaceRecognitionEngine → crop face, run ArcFace → embedding
   → FaceDatabase::Search(embedding) → match: "John Smith" (similarity=0.89)
   → EventManager::PublishFaceRecognized(person_id=7, name="John Smith", group="employee")
   → Actions: log, ONVIF notify
8. FaceAttributeEngine → age=32, gender=male, expression=neutral
   → Enriches face_recognition_log entry
9. CountingEngine → people_current=5 (was 4)
   → OSD text updated: "People: 5"
10. SmartBbox update → person box (red) + face box (blue) on encoded stream
11. ONVIF metadata → XML with person detection + face recognition in RTP
```

---

## 11. Overlay & OSD Strategy <a name="overlay-strategy"></a>

### What Goes Where

| Overlay Type | Hardware | Content |
|-------------|----------|---------|
| **SMART_BBOX** | Video encoder HW | Detection bounding boxes (person=red, vehicle=green, face=blue) |
| **OSD Text** | Video encoder HW | Counting text, plate text, person name labels |
| **Privacy Mosaic** | GFX engine HW | Face blur regions (before encoding) |
| **Heatmap** | Web UI only | Canvas overlay (client-side rendering, not on RTSP) |
| **Lines/Zones** | Web UI only | SVG overlay on video player (not burned into stream) |

### SMART_BBOX Details

The NT98538 encoder supports up to 32 SMART_BBOX regions. Each has:
- Position (x, y, width, height in pixels)
- Color (RGB)
- Border thickness

Allocation strategy:
```
Boxes 0-15:  Person/Vehicle detections (YOLO)
Boxes 16-23: Face detections (SCRFD)
Boxes 24-27: License plates (PlateDetector)
Boxes 28-31: Reserved (custom)
```

### OSD Text Overlay

Use the existing OSD module to display persistent text on the stream:
```
┌─────────────────────────────────────────────────┐
│ 2026-02-28 14:30:22              People: 5  ⊕   │  ← OSD timestamp + counts
│                                  Vehicles: 3     │
│                                                   │
│   ┌──────────┐                                   │
│   │ John S.  │  ← SMART_BBOX + OSD label        │
│   │  👤      │                                   │
│   └──────────┘                                   │
│                        ┌────┐                    │
│                        │🚗  │  ← Vehicle box     │
│                        │ABC-│                    │
│                        │1234│  ← Plate text OSD  │
│                        └────┘                    │
│                                                   │
│──────────────── Entry Line ──────────────────────│  ← Drawn in web UI only
│                                                   │
│  IN: 47  |  OUT: 32                              │  ← OSD text for line counts
└─────────────────────────────────────────────────┘
```

---

## 12. NPU Scheduling & Resource Management <a name="npu-scheduling"></a>

### The Problem

The NN30 NPU processes models sequentially. If multiple models are needed per frame:
- YOLO: ~10-15ms
- SCRFD: ~8-12ms  
- ArcFace: ~3-5ms per face
- PlateDetector: ~5ms per vehicle
- LPRNet: ~3ms per plate
- YAMNet: ~1.3ms per audio frame
- FaceAttribute: ~5ms per face
- YOLOv8-Pose: ~15ms

Total if ALL enabled: 50-60ms per frame → 16-20 FPS max

### Scheduling Strategy

```
Frame N:
  ├── YOLO inference (15ms)           } Always (if object_detection enabled)
  ├── SCRFD inference (10ms)          } Only if face features enabled
  ├── ArcFace × N_faces (3ms each)   } Only if recognition enabled, max 4 faces/frame
  └── PlateDetect × N_vehicles (5ms each) } Only if LPR enabled, max 2 vehicles/frame

Audio (separate thread, independent of video frame):
  └── YAMNet inference (1.3ms)        } Every 960ms audio chunk

Pose estimation (every Nth frame):
  └── YOLOv8-Pose (15ms)             } Skip frames if CPU-constrained
```

### Priority Tiers

| Tier | Feature | NPU Budget per Frame | Run Every |
|------|---------|---------------------|-----------|
| 1 | YOLO (person/vehicle) | 15ms | Every frame |
| 1 | Motion Detection | 0ms (CPU libmd) | Every frame |
| 2 | SCRFD (face) | 10ms | Every frame if enabled |
| 2 | ArcFace (face rec) | 3-12ms (1-4 faces) | Every frame if new face |
| 3 | PlateDetect | 5ms | Only when vehicle detected |
| 3 | LPRNet | 3ms | Only when plate detected |
| 3 | FaceAttribute | 5ms | Only when new face detected |
| 4 | YOLOv8-Pose | 15ms | Every 3rd frame |
| 4 | YAMNet | 1.3ms | Every 960ms (async) |

### Model Loading Rules

```
Always loaded:   (none — load on demand)
Load if object_detection.enabled: YOLO
Load if face_detection.enabled:   SCRFD
Load if face_recognition.enabled: SCRFD + ArcFace
Load if license_plate.enabled:    PlateDetector + LPRNet
Load if face_attribute.enabled:   SCRFD + FaceAttribute
Load if pose_estimation.enabled:  YOLOv8-Pose
Load if audio.enabled:            YAMNet
```

### Memory Budget

| Model | Size | RAM for Tensors |
|-------|------|-----------------|
| YOLOv8n | 6.2MB | ~8MB working |
| SCRFD-500m | 2.5MB | ~4MB working |
| ArcFace/MobileFaceNet | 4.0MB | ~3MB working |
| PlateDetector | 3.0MB | ~4MB working |
| LPRNet | 1.5MB | ~2MB working |
| YAMNet | 3.5MB | ~2MB working |
| FaceAttribute | 3.0MB | ~3MB working |
| YOLOv8-Pose | 6.5MB | ~8MB working |
| **Total (all loaded)** | **~30MB** | **~34MB working** |

The NT98538 has 512MB-1GB DDR. With Linux + video pipeline using ~200MB, there's plenty of room for all models simultaneously if needed.

---

## 13. Configuration Reference <a name="configuration-reference"></a>

### analytics.json Additions

```json
{
    "face_recognition": {
        "enabled": false,
        "model_file": "/mnt/app/ai_models/mobilefacenet.bin",
        "database_path": "/mnt/sd/analytics/face_db",
        "similarity_threshold": 0.7,
        "max_faces_per_frame": 4,
        "unknown_face_snapshot": true,
        "groups": {
            "employee": { "color": "#00FF00", "action": "log" },
            "visitor":  { "color": "#FFFF00", "action": "log" },
            "vip":      { "color": "#0000FF", "action": "notify" },
            "blocked":  { "color": "#FF0000", "action": "alarm" }
        }
    },
    
    "face_attribute": {
        "enabled": false,
        "model_file": "/mnt/app/ai_models/face_attribute.bin",
        "estimate_age": true,
        "estimate_gender": true,
        "estimate_expression": true,
        "detect_glasses": true,
        "detect_mask": true
    },
    
    "license_plate": {
        "enabled": false,
        "detector_model": "/mnt/app/ai_models/plate_detector.bin",
        "ocr_model": "/mnt/app/ai_models/lprnet.bin",
        "database_path": "/mnt/sd/analytics/plate_db",
        "min_confidence": 0.7,
        "regions": ["us", "eu"],
        "unknown_plate_snapshot": true,
        "groups": {
            "employee": { "action": "log" },
            "visitor":  { "action": "log" },
            "vip":      { "action": "notify" },
            "blocked":  { "action": "alarm" }
        }
    },
    
    "counting": {
        "enabled": false,
        "mode": "zone",
        "count_persons": true,
        "count_vehicles": true,
        "occupancy_threshold": 50,
        "display_on_osd": true,
        "osd_position": "top_right",
        "reset_daily": true,
        "reset_time": "00:00",
        "persist_interval_sec": 60
    },
    
    "heatmap": {
        "enabled": false,
        "grid_size": [80, 45],
        "snapshot_interval_sec": 3600,
        "retention_days": 30
    },
    
    "audio_classification": {
        "enabled": false,
        "model_file": "/mnt/app/ai_models/yamnet_cctv.bin",
        "classes": {
            "glass_break":    { "enabled": true,  "threshold": 0.7 },
            "gunshot":        { "enabled": true,  "threshold": 0.8 },
            "scream":         { "enabled": true,  "threshold": 0.7 },
            "baby_cry":       { "enabled": true,  "threshold": 0.6 },
            "dog_bark":       { "enabled": false, "threshold": 0.6 },
            "smoke_alarm":    { "enabled": true,  "threshold": 0.7 },
            "siren":          { "enabled": true,  "threshold": 0.7 },
            "car_horn":       { "enabled": false, "threshold": 0.6 },
            "explosion":      { "enabled": true,  "threshold": 0.8 }
        },
        "cooldown_sec": 10,
        "sample_rate": 16000,
        "audio_device": "default"
    },
    
    "pose_estimation": {
        "enabled": false,
        "model_file": "/mnt/app/ai_models/yolov8n_pose.bin",
        "min_confidence": 0.5,
        "fall_detection": {
            "enabled": false,
            "sensitivity": 50,
            "dwell_time_sec": 3
        },
        "skip_frames": 2
    }
}
```

---

## 14. Implementation Checklist <a name="implementation-checklist"></a>

### Phase 1: NPU Backend (MUST DO FIRST)

- [ ] **1.1** Create `npu_inference.h/.cpp` — vendor_ai3 wrapper class
- [ ] **1.2** Migrate `yolov8_postproc.c` → `yolo_postproc.cpp` (from Demo 01)
- [ ] **1.3** Migrate `scrfd_postproc.c` → `scrfd_postproc.cpp` (from Demo 02)
- [ ] **1.4** Wire `NpuInferenceManager` into `AnalyticsEngine::ProcessingThread()`
- [ ] **1.5** Add model path configuration to `analytics.json`
- [ ] **1.6** Test: Enable object detection → verify SMART_BBOX overlays on RTSP
- [ ] **1.7** Test: Enable face detection → verify face boxes on RTSP

### Phase 2: Core Events & Overlay

- [ ] **2.1** Wire YOLO detections → `kPersonDetected` / `kVehicleDetected` events
- [ ] **2.2** Wire SCRFD detections → `kFaceDetected` events
- [ ] **2.3** Connect events → recording/snapshot/webhook actions (already working)
- [ ] **2.4** Verify ONVIF metadata XML includes person/vehicle/face detections
- [ ] **2.5** Verify SMART_BBOX color coding: person=red, vehicle=green, face=blue

### Phase 3: Rule-Based Features

- [ ] **3.1** Create `line_crossing.cpp` — track center cross detection
- [ ] **3.2** Create `zone_intrusion.cpp` — point-in-polygon + dwell timer
- [ ] **3.3** Create `counting.cpp` — class-based counting + OSD display
- [ ] **3.4** Create `heat_map.cpp` — grid accumulation + DB persistence
- [ ] **3.5** Wire all to existing web API routes (already defined, stubs only)
- [ ] **3.6** Wire to ONVIF events (LineDetector/Crossed, FieldDetector/ObjectsInside)
- [ ] **3.7** Add counting statistics DB tables + API endpoints
- [ ] **3.8** Add heatmap DB tables + API endpoints
- [ ] **3.9** Test: walk across line → event fires, count increments
- [ ] **3.10** Test: stay in zone > N seconds → intrusion alarm fires

### Phase 4: Face Recognition + Database

- [ ] **4.1** Create `face_database.h/.cpp` — SQLite face embedding CRUD
- [ ] **4.2** Create `face_recognition.h/.cpp` — SCRFD → align → ArcFace → search
- [ ] **4.3** Create `face_attribute.h/.cpp` — age/gender/expression model
- [ ] **4.4** Add face API routes (`/api/v1/faces/*`) to webserver
- [ ] **4.5** Add face enrollment web UI page
- [ ] **4.6** Add face event types to EventType enum
- [ ] **4.7** Add face events → action rules (blocked person → alarm)
- [ ] **4.8** Test: enroll face via API → recognize on stream → event fires
- [ ] **4.9** Test: unknown face → snapshot saved, webhook sent

### Phase 5: LPR + Database

- [ ] **5.1** Create `plate_database.h/.cpp` — SQLite plate text CRUD
- [ ] **5.2** Create `lpr.h/.cpp` — PlateDetector → LPRNet → DB search
- [ ] **5.3** Add plate API routes (`/api/v1/plates/*`) to webserver
- [ ] **5.4** Add plate event types to EventType enum
- [ ] **5.5** Add plate events → action rules
- [ ] **5.6** Test: add plate to DB → car passes → plate recognized → event fires

### Phase 6: Audio + Pose + Advanced

- [ ] **6.1** Create `audio_classification.h/.cpp` — ALSA → mel → YAMNet NPU
- [ ] **6.2** Add audio API routes (`/api/v1/analytics/audio/*`)
- [ ] **6.3** Add audio event types to EventType enum
- [ ] **6.4** Create `pose_estimation.h/.cpp` — YOLOv8-Pose + keypoint decode
- [ ] **6.5** Optional: Fall detection from pose keypoints
- [ ] **6.6** Add analytics dashboard API (`/api/v1/analytics/dashboard`)
- [ ] **6.7** Test: play glass break sound → event fires, notification sent

### Integration Testing (End-to-End)

- [ ] **7.1** All features enabled simultaneously — verify NPU scheduling works
- [ ] **7.2** Memory usage < 400MB total with all models loaded
- [ ] **7.3** ≥ 10 FPS with YOLO + SCRFD + tracking + all post-processors
- [ ] **7.4** Events appear in ONVIF metadata stream (verify with ONVIF Device Manager)
- [ ] **7.5** Web UI shows live detections, counts, and heatmap
- [ ] **7.6** Face enrollment → recognition → alert loop works end-to-end
- [ ] **7.7** Plate enrollment → recognition → alert loop works end-to-end
- [ ] **7.8** 24-hour stability test — no memory leaks, no crashes

---

## File Creation Summary

### New Files Required (modules/ai/)

```
modules/ai/
├── include/ipcam/
│   ├── npu_inference.h         # NPU model load/run manager
│   ├── yolo_postproc.h         # YOLO NMS + decode
│   ├── scrfd_postproc.h        # SCRFD decode + landmarks
│   ├── face_recognition.h      # Face recognition engine
│   ├── face_database.h         # Face DB CRUD
│   ├── face_attribute.h        # Age/gender/expression
│   ├── lpr.h                   # LPR engine
│   ├── plate_database.h        # Plate DB CRUD
│   ├── counting.h              # People/vehicle counting
│   ├── heat_map.h              # Heatmap accumulation
│   ├── line_crossing.h         # Line crossing detection
│   ├── zone_intrusion.h        # Zone intrusion detection
│   ├── audio_classification.h  # Audio event detection
│   ├── pose_estimation.h       # Pose keypoint detection
│   └── analytics_database.h    # Shared analytics DB manager
├── src/
│   ├── npu_inference.cpp
│   ├── yolo_postproc.cpp
│   ├── scrfd_postproc.cpp
│   ├── face_recognition.cpp
│   ├── face_database.cpp
│   ├── face_attribute.cpp
│   ├── lpr.cpp
│   ├── plate_database.cpp
│   ├── counting.cpp
│   ├── heat_map.cpp
│   ├── line_crossing.cpp
│   ├── zone_intrusion.cpp
│   ├── audio_classification.cpp
│   ├── pose_estimation.cpp
│   └── analytics_database.cpp
```

### New/Modified Files in Other Modules

```
modules/events/
├── include/ipcam/
│   └── event_types.h            # ADD: Face/Plate/Audio/Occupancy event types + data
modules/webserver/
├── src/
│   ├── face_api.cpp             # NEW: /api/v1/faces/* routes
│   ├── plate_api.cpp            # NEW: /api/v1/plates/* routes
│   ├── counting_api.cpp         # NEW: /api/v1/analytics/counting/* routes
│   ├── heatmap_api.cpp          # NEW: /api/v1/analytics/heatmap/* routes
│   ├── audio_api.cpp            # NEW: /api/v1/analytics/audio/* routes
│   └── dashboard_api.cpp        # NEW: /api/v1/analytics/dashboard
configs/config.factory.d/
├── analytics.json               # MODIFY: Add face/lpr/counting/audio/pose config
```
