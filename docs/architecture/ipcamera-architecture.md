# Eterna IP Camera Application — Architecture Documentation

> **Codebase root:** `code/application/ipcamera/`  
> **Main binary:** `apps/ipcamd/src/main.cpp`  
> **Status:** Logic complete; not compilable in isolation (depends on Novatek HDAL SDK and board BSP).

---

## Table of Contents

1. [System Overview](#1-system-overview)
2. [Repository Structure](#2-repository-structure)
3. [Design Patterns](#3-design-patterns)
   - 3.1 [Singleton — `X::Instance()`](#31-singleton--xinstance)
   - 3.2 [Coordinator — `main.cpp`](#32-coordinator--maincpp)
   - 3.3 [Publish–Subscribe — EventManager](#33-publishsubscribe--eventmanager)
   - 3.4 [JSON Configuration System](#34-json-configuration-system)
   - 3.5 [MediaHub — Ring Buffer & Callbacks](#35-mediahub--ring-buffer--callbacks)
4. [Module Catalogue](#4-module-catalogue)
5. [Startup and Shutdown Sequence](#5-startup-and-shutdown-sequence)
6. [Video Pipeline Data Flow](#6-video-pipeline-data-flow)
7. [Event System Data Flow](#7-event-system-data-flow)
8. [Configuration Layering](#8-configuration-layering)
9. [Dependency Map](#9-dependency-map)

---

## 1. System Overview

`ipcamd` is a single Linux daemon that owns the full lifecycle of an Eterna IP camera:

- Captures raw sensor data via the Novatek **HDAL** (Hardware Driver Abstraction Layer) SDK.
- Encodes up to **four simultaneous H.264/H.265 video streams** and one G.711 audio stream.
- Distributes encoded video to **RTSP**, **recording**, and **WebRTC/HLS** consumers through a central ring buffer (`MediaHub`).
- Runs **on-device AI analytics** (object detection, motion, line crossing, LPR, face, audio events, tamper) via an NPU inference engine.
- Publishes all AI and sensor events through a typed **EventManager** that dispatches configurable actions (record, snapshot, email, webhook, MQTT, FTP, alarm output).
- Exposes a REST HTTP/HTTPS API, ONVIF WS-Discovery service, and mDNS/DNS-SD.

---

## 2. Repository Structure

```
code/application/ipcamera/
├── apps/
│   └── ipcamd/src/
│       └── main.cpp                 ← Coordinator / application entry point
├── configs/
│   ├── config.factory.d/            ← Per-module factory-default JSON configs
│   │   ├── media.json
│   │   ├── analytics.json
│   │   ├── events.json
│   │   ├── network.json
│   │   ├── streaming.json
│   │   ├── recording.json
│   │   ├── storage.json
│   │   ├── auth.json
│   │   ├── isp.json
│   │   ├── osd.json
│   │   ├── onvif.json
│   │   ├── ir.json
│   │   ├── logging.json
│   │   ├── system.json
│   │   ├── device.json
│   │   ├── web_portal.json
│   │   ├── reset_button.json
│   │   └── recording/profiles/
│   │       └── default-24x7.json
│   └── config.d/                    ← Deployment-specific overrides (optional)
└── modules/
    ├── ai/          ← AnalyticsEngine, NPU inference, motion/tamper/LPR/face/audio
    ├── config/      ← JSON config system (nlohmann/json), UserManager (SQLite/SQLCipher)
    ├── events/      ← EventManager, EventRule, ActionHandler, action implementations
    ├── media/       ← MediaHub (ring buffer), VideoControl, AudioControl
    ├── networking/  ← NetworkManager, NTP, mDNS, nginx, SSL, SMTP, SNMP, UPnP
    ├── onvif/       ← ONVIF WS-Discovery, SOAP service (gSOAP)
    ├── platform/    ← HdalPipeline, HdalWrapper, ISPControl, IRControl, OSD, ResetButton
    ├── recording/   ← RecordingService (MP4/raw), ScheduleManager
    ├── storage/     ← NasManager (NFS/SMB), FtpManager
    ├── streaming/   ← RTSP server (Live555), Go2rtcManager, AudioFrameBroadcaster, OnvifMetadata
    ├── upgrade/     ← Firmware upgrade
    ├── utils/       ← AuditLogger, shared utilities
    └── webserver/   ← HTTP REST API server
```

---

## 3. Design Patterns

### 3.1 Singleton — `X::Instance()`

Every manager and engine in the application is a **Meyers Singleton**: a class with a private constructor whose sole access point is a static `Instance()` method returning a reference to a function-local `static` object. The C++11 standard guarantees this initialization is thread-safe without any additional locking.

**Universal implementation (identical across all classes):**

```cpp
// In the .h:
class NetworkManager {
public:
    static NetworkManager& Instance();
private:
    NetworkManager() = default;
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
};

// In the .cpp:
NetworkManager& NetworkManager::Instance() {
    static NetworkManager instance;
    return instance;
}
```

**Complete singleton inventory (35 classes):**

| Class | Namespace | Module | Role |
|-------|-----------|--------|------|
| `EventManager` | `ipcam::events` | events | Event queue, rule matching, action dispatch |
| `AnalyticsEngine` | `ipcam::ai` | ai | AI inference orchestration |
| `NpuInference` | `ipcam::ai` | ai | NPU model runner |
| `TamperDetectionEngine` | `ipcam::ai` | ai | Video tamper detection |
| `MotionDetectionEngine` | `ipcam::ai` | ai | Pixel-based motion detection |
| `ObjectTracker` | `ipcam::ai` | ai | Multi-object tracking |
| `VqaEngine` | `ipcam::ai` | ai | Video quality analysis |
| `PrivacyMosaicEngine` | `ipcam::ai` | ai | Privacy mask / blur |
| `AiispEngine` | `ipcam::ai` | ai | AI-based ISP enhancement |
| `HdalPipeline` | `ipcam::platform` | platform | Novatek HDAL sensor/encoder pipeline |
| `HdalWrapper` | `ipcam::platform` | platform | Vendor ISP tuning API wrapper |
| `ISPControl` | `ipcam::platform` | platform | High-level ISP image quality API |
| `IRControl` | `ipcam::platform` | platform | IR LED, IR cut filter, day/night |
| `OsdOverlay` | `ipcam::platform` | platform | On-screen display (timestamp, logo, masks) |
| `ResetButton` | `ipcam::platform` | platform | GPIO factory-reset button monitor |
| `SystemLogger` | `ipcam::platform` | platform | Periodic system data collector |
| `MediaHub` | `ipcam::media` | media | Ring buffer + consumer notification |
| `VideoControl` | `ipcam::media` | media | Encoder parameter API bridge |
| `AudioControl` | `ipcam::media` | media | Audio parameter API bridge |
| `RecordingService` | `ipcam::recording` | recording | MP4/raw segment recording to SD card |
| `ScheduleManager` | `ipcam::recording` | recording | Scheduled recording triggers |
| `NasManager` | `ipcam::storage` | storage | NFS/SMB network storage |
| `FtpManager` | `ipcam::storage` | storage | FTP upload queue |
| `NetworkManager` | `ipcam::networking` | networking | Network interface and service facade |
| `NtpManager` | `ipcam::networking` | networking | Pure-C++ NTP sync |
| `NginxManager` | `ipcam::networking` | networking | nginx config and process control |
| `SslManager` | `ipcam::networking` | networking | SSL certificate lifecycle |
| `MdnsResponder` | *(global)* | networking | mDNS/DNS-SD via raw sockets (`GetInstance()`) |
| `Go2rtcManager` | `ipcam::streaming` | streaming | WebRTC/HLS restreamer |
| `AudioFrameBroadcaster` | `ipcam::streaming` | streaming | Audio consumer distribution |
| `OnvifMetadataGenerator` | `ipcam::streaming` | streaming | ONVIF metadata stream |
| `AuditLogger` | *(utils)* | utils | Security audit log |
| `PasswordCrypto` | *(webserver)* | webserver | Password hashing helper |
| `ConfigLoader` | `ipcam::config` | config | YAML-based config loader (secondary path) |

> **Note:** The config module also uses module-level anonymous-namespace statics (`g_config_tree`, `g_factory_tree`, `g_mutex`) protected by `std::mutex`, which achieves the same effect without a class-based singleton.

---

### 3.2 Coordinator — `main.cpp`

`apps/ipcamd/src/main.cpp` is the **central orchestrator**. It does not contain business logic; its responsibilities are:

1. **Initialize logging** (multi-sink spdlog: rotating file at DEBUG + coloured stdout at INFO).
2. **Install signal handlers** (`SIGINT`/`SIGTERM` → set `g_running = false`).
3. **Parse CLI arguments** (`-c/--config` to override config directory).
4. **Start subsystems in dependency order** — each subsystem is initialized and started before any other subsystem that depends on it.
5. **Wire AI analytics callbacks to EventManager** — the coordinator is the only place that knows about both `AnalyticsEngine` and `EventManager`, and it stitches them together with lambda callbacks.
6. **Block in the main loop** until a shutdown signal is received.
7. **Tear down subsystems in reverse dependency order**.

This pattern keeps every module completely independent — no module imports another module's singleton directly in ways that create circular dependencies; instead, all cross-cutting wiring happens in `main.cpp`.

**Startup sequence (23 ordered steps):**

| # | Subsystem call | Dependency / note |
|---|---------------|-------------------|
| 1 | Logging (spdlog) | No dependencies |
| 2 | Signal handlers | No dependencies |
| 3 | CLI argument parsing | — |
| 4 | `ipcam::config::Init()` | **Fatal** — all other modules read from config |
| 5 | `NetworkManager::Instance().InitializeNetwork()` | Reads `network.json` |
| 6 | `SystemLogger::Instance().Init()` | Non-fatal |
| 7 | `HdalPipeline::Instance().Init()` + `.Start()` | Initialises sensor, ISP, encoders; starts nvtrtspd_ipc |
| 8 | `MediaHub::Instance().Initialize()` + `.Start()` | **Requires** HdalPipeline running; allocates ring buffers and producer threads |
| 9 | `AnalyticsEngine::Instance()` (lazy init) | Connects to HDAL VideoProc paths; starts NPU inference |
| 10 | `VideoControl::Instance().Init()` | Reads current encoder state from HdalPipeline |
| 11 | `IRControl::Instance().Init()` | Reads `ir.json`; starts auto day/night thread |
| 12 | `ResetButton::Instance().Init()` | Reads `reset_button.json`; starts GPIO poll thread |
| 13 | `ipcam::streaming::Initialize()` + `::Start()` | RTSP server; **requires** MediaHub running |
| 14 | `Go2rtcManager::Instance().Start()` | Optional WebRTC/HLS; **requires** RTSP server running |
| 15 | `ipcam::storage::Init()` | Opens SQLite DBs; **Fatal** |
| 16 | `NasManager::Instance().Init()` | Non-fatal |
| 17 | `EventManager::Instance().Init()` + `.Start()` | Registers all action handlers; wires analytics→event callbacks; publishes `kSystemStartup` |
| 18 | `FtpManager::Instance().Init()` | Non-fatal |
| 19 | `ScheduleManager::Instance().Init()` | Non-fatal |
| 20 | `VideoControl::Instance().Init()` | Second call (explicit log marker in source) |
| 21 | `RecordingService::Instance().Initialize()` | **Requires** storage and MediaHub |
| 22 | `ipcam::webserver::Init()` | REST API; **Fatal** |
| 23 | `ipcam::onvif::Init()` | ONVIF WS-Discovery; **Fatal** |

**Shutdown sequence (reverse dependency order):**

```
EventManager.Stop()          ← drains event queue first
RecordingService.Shutdown()
ScheduleManager.Shutdown()
FtpManager.Shutdown()
NasManager.Shutdown()
AnalyticsEngine.Shutdown()
SystemLogger.Shutdown()
IRControl.Shutdown()
ResetButton.Shutdown()
Go2rtcManager.Stop()         ← must stop before RTSP server
ipcam::streaming::Stop()     ← RTSP server
MediaHub.Shutdown()          ← must stop before HDAL
HdalPipeline.Shutdown()
ipcam::onvif::Shutdown()
ipcam::webserver::Shutdown()
ipcam::storage::Shutdown()
ipcam::config::Shutdown()
_Exit(0)                     ← bypasses C++ static destructors (SQLCipher safety)
```

---

### 3.3 Publish–Subscribe — EventManager

The application uses a **typed publish–subscribe** pattern, implemented in `modules/events/`, to decouple event sources (AI analytics, hardware I/O, storage, network) from event consumers (recording, snapshot capture, email, webhook, MQTT, FTP, alarm outputs, ONVIF).

#### Core Types

```
EventCategory  — 9 values: kMotion, kAnalytics, kLineCrossing, kIntrusion,
                            kSystem, kIO, kStorage, kNetwork, kSchedule

EventType      — 40+ values: kMotionStart, kMotionEnd, kPersonDetected, kPersonLost,
                              kVehicleDetected, kFaceDetected, kLineCrossed*, kZoneEntered,
                              kZoneLoitering, kTamperDetected, kLprDetected, kAudioDetected,
                              kSystemStartup, kSystemShutdown, kAlarmInputTriggered,
                              kStorageMounted, kNetworkConnected, ...

ActionType     — 21 values: kStartRecording, kCaptureSnapshot, kSendEmail,
                             kSendWebhook, kPublishMqtt, kUploadFtp, kPublishOnvifEvent,
                             kTriggerAlarmOutput, kActivateLight, kPlaySiren, ...

EventData      — std::variant<MotionEventData, AnalyticsEventData, LineCrossEventData,
                              IntrusionEventData, SystemEventData, IOEventData,
                              StorageEventData, NetworkEventData, TamperEventData,
                              LprEventData, AudioEventData>
```

#### Publishing Events

Any subsystem calls a convenience method on the `EventManager` singleton:

```cpp
// From the analytics→EventManager wiring in main.cpp:
EventManager::Instance().PublishMotionStart({zone_id}, confidence);
EventManager::Instance().PublishPersonDetected(detected_object);
EventManager::Instance().PublishLineCrossed(line_id, name, direction, object_id, class_name);
EventManager::Instance().PublishLprDetected(plate_text, confidence, x1, y1, x2, y2);
EventManager::Instance().PublishSystemEvent(EventType::kSystemStartup, "ipcamd", msg, 0);
```

Each method constructs a typed `Event`, pushes it onto an internal `std::queue<Event>`, and signals a `std::condition_variable`. Publishing is non-blocking and thread-safe.

#### Subscribing (Listeners)

Any subsystem can register a callback with an optional filter:

```cpp
using EventListener = std::function<void(const Event&)>;
using EventFilter   = std::function<bool(const Event&)>;

uint32_t id = EventManager::Instance().AddListener(
    [](const Event& e) { /* handle */ },
    [](const Event& e) { return e.type == EventType::kPersonDetected; }  // optional filter
);

// To remove:
EventManager::Instance().RemoveListener(id);
```

All listeners are called on the single background `ProcessingThread` after the event is dequeued. Exceptions inside listener lambdas are caught and logged; they do not terminate the processing thread.

#### Rule-Based Action Dispatch

In addition to free listeners, the EventManager maintains an `EventRule` list loaded from `events.json`. Each rule specifies:

- **Trigger condition** — one or more `EventType` values, optional time schedule, optional minimum confidence.
- **Actions** — a list of `Action` objects, each specifying an `ActionType` and its typed config (e.g., recording duration, email recipients, webhook URL, MQTT topic).

When an event matches a rule, the EventManager calls the registered `ActionHandler::Execute()` for each enabled action in a **detached thread** so the processing thread is never blocked.

#### Analytics → EventManager Wiring

`AnalyticsEngine` fires typed **analytics callbacks** whenever the AI inference detects an event. These are `std::function<>` setters. In `main.cpp`, each setter is called with a lambda that translates the AI-specific result into an `EventManager::Publish*()` call:

```
AnalyticsEngine callbacks          EventManager publish methods
──────────────────────────         ──────────────────────────────
SetMotionCallback()           →    PublishMotionStart() / PublishMotionEnd()
SetDetectionCallback()        →    PublishPersonDetected() / PublishVehicleDetected() / PublishFaceDetected()
SetLineCrossCallback()        →    PublishLineCrossed()
SetLoiteringCallback()        →    PublishZoneIntrusion()
SetTamperCallback()           →    PublishTamperEvent()
SetFaceRecognitionCallback()  →    PublishFaceDetected()
SetLprCallback()              →    PublishLprDetected()
SetAudioEventCallback()       →    PublishAudioDetected()
```

#### Action Handlers

Each `ActionType` has a concrete `ActionHandler` implementation registered via `ActionHandlerFactory::RegisterAll()`:

| Handler class | ActionType(s) | Backend |
|--------------|---------------|---------|
| `RecordingActionHandler` | `kStartRecording` | `RecordingService::Instance()` |
| `SnapshotActionHandler` | `kCaptureSnapshot`, `kGenerateThumbnail` | `VideoControl::Instance()` |
| `EmailActionHandler` | `kSendEmail` | `SmtpManager` (libcurl) |
| `WebhookActionHandler` | `kSendWebhook` | libcurl HTTP POST/PUT/GET with retry |
| `MqttActionHandler` | `kPublishMqtt` | Paho MQTT `async_client` (conditional compile) |
| `FtpUploadHandler` | `kUploadFtp` | `FtpManager::Instance()` |
| `IoOutputHandler` | `kTriggerAlarmOutput`, `kClearAlarmOutput` | `IRControl::Instance()` (PWM) |
| `LightActionHandler` | `kActivateLight`, `kDeactivateLight` | `IRControl::Instance()` (solid/flash/strobe) |
| `SirenActionHandler` | `kPlaySiren`, `kStopSiren` | Stub (pending audio integration) |

All handlers inherit from an `ActionHandler` base class that provides:
- `ExecuteAsync()` — wraps `Execute()` in a `std::future`
- `SubstitutePlaceholders()` — template substitution for `{event_id}`, `{timestamp}`, `{object_class}`, `{snapshot_path}`, etc.

---

### 3.4 JSON Configuration System

The configuration system is implemented in `modules/config/` using **nlohmann/json**. It provides a **three-layer merge** strategy and a free-function API; there is no config handle or dependency-injected object.

#### Access API

```cpp
// Read (any module, any thread):
int port     = ipcam::config::Get<int>("network.rtsp.port", 554);
bool enabled = ipcam::config::Get<bool>("streaming.rtsp.enabled", true);
auto server  = ipcam::config::GetOptional<std::string>("network.smtp.server");

// Write and persist:
ipcam::config::Set<int>("network.rtsp.port", 8554);
ipcam::config::Save();   // atomic rename; writes only the user delta
```

**Dot-path notation** with array index support:
- `"network.ipv4.address"` → nested object traversal
- `"storage.channels[0].id"` → array element indexing

#### Three-Layer Merge

```
Layer 1 (lowest):  config.factory.d/*.json     — per-module factory defaults (in source tree)
Layer 2:           config.d/*.json              — deployment/SKU overrides (optional)
                                 ▼ merged → g_factory_tree (snapshot)
Layer 3 (highest): config.json                 — user delta only (runtime, on SD/flash)
                                 ▼ merged → g_config_tree (live state)
```

Files within each layer are sorted alphabetically and merged via `json::merge_patch()`.

#### Save Strategy — Delta Only

`Save()` computes `diff(g_factory_tree, g_config_tree)` and writes **only the changed keys** to `config.json` via an atomic `rename()` of a `.tmp` file. If no user changes exist, `config.json` is deleted so factory defaults apply cleanly on next boot. This minimises flash writes and makes factory-reset trivial (delete `config.json`).

#### Factory-Default Config Files (18 files)

| File | Module configured |
|------|--------------------|
| `device.json` | Device identity, model, firmware versions, sensor capabilities |
| `media.json` | Sensor driver, ISP tuning, encoder configs (4 streams), audio |
| `isp.json` | Image quality sliders, white balance, exposure, NR, WDR, day/night |
| `osd.json` | Per-stream OSD layout, timestamp format, logo, privacy masks |
| `streaming.json` | RTSP server (Live555), go2rtc restreamer |
| `network.json` | Interfaces, ports, IP config, DNS, hostname, mDNS, SMTP, SNMP, UPnP |
| `onvif.json` | WS-Discovery, ONVIF service port |
| `ir.json` | IR LED PWM, IR cut filter GPIO, auto day/night thresholds |
| `analytics.json` | All AI modules: motion zones, object detection, line crossing, LPR, face, audio, NPU config |
| `events.json` | Event rules (5 pre-defined), action configs, alarm I/O, notification endpoints |
| `recording.json` | Schedule profile path, NAS, FTP |
| `recording/profiles/default-24x7.json` | Continuous 24×7 recording schedule |
| `storage.json` | SD card mount, encryption (AES-256-CTR), SQLite DB paths |
| `auth.json` | User DB path, session TTL, password policy (PBKDF2 100k iterations) |
| `logging.json` | spdlog sinks, access/security/audit/remote syslog, SystemLogger sources |
| `system.json` | Timezone, NTP servers, DST, watchdog, memory thresholds, maintenance |
| `web_portal.json` | HTTP API port, threads, session limits, rate limiting, CORS |
| `reset_button.json` | GPIO pin, hold duration, debounce, poll interval |

---

### 3.5 MediaHub — Ring Buffer & Callbacks

`MediaHub` (in `modules/media/`) is the **central media distribution hub**. It decouples the single HDAL encoder output from an arbitrary number of simultaneous consumers (RTSP, recording, WebRTC, future HLS) using a **Single-Producer Multiple-Consumer (SPMC) ring buffer** per video channel.

#### Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                       HdalPipeline                              │
│  Sensor → ISP → VideoProc → Encoder[0] → Encoder[1] → ...      │
└──────────────┬──────────────────┬──────────────────────────────┘
               │ hd_videoenc_pull_out_buf()
       ┌───────▼───────┐   ┌──────▼───────┐
       │ ProducerThread│   │ProducerThread│  (one per active channel)
       │  (ch 0)       │   │  (ch 1)      │
       └───────┬───────┘   └──────┬───────┘
               │ ring_buffer.Write()
       ┌───────▼──────────────────▼───────┐
       │     VideoRingBuffer (per channel) │
       │  30 slots × max_frame_size        │  ← 1 second at 30 fps
       │  seq-number versioned (SPMC)      │
       │  SPS/PPS/VPS cached per slot      │
       └───────┬────────────────┬──────────┘
               │ frame_cv.notify_all()
      ┌────────▼──────┐  ┌─────▼──────────┐  ┌─────▼──────┐
      │VideoFrameConsumer│ │VideoFrameConsumer│ │VideoFrameConsumer│
      │  "rtsp-ch0"   │  │ "recording-ch0"│  │ (future)   │
      └────────┬──────┘  └─────┬──────────┘  └────────────┘
               │               │
      [HdalVideoSource]  [RecordingThread]
      Live555 FramedSource   MP4Recorder (minimp4)
      RTP packetisation      NAL-pack → fragmented MP4
                             AES-256-CTR encryption
```

#### Ring Buffer Design

| Property | Value |
|----------|-------|
| Slots per channel | **30** (`kVideoRingBufferSlots`) — 1 second at 30 fps |
| Main stream max frame | 512 KB (`kMaxVideoFrameSize`) |
| Sub-stream max frame | 128–256 KB (`kMaxSubStreamFrameSize`) |
| Main stream total buffer | 30 × 512 KB = **15 MB** |
| Write concurrency | **Single producer** (one `ProducerThread` per channel) |
| Read concurrency | **Multiple consumers** — each holds its own `next_sequence_` cursor |
| Torn-read protection | Version field: **odd** = write in progress, **even** = ready |

Each slot stores the full encoded frame (all NAL units concatenated), SPS/PPS/VPS parameter sets, per-NAL-unit boundary info (`NalPack` offsets), hardware timestamp, and codec type.

#### Write Path (Producer)

```cpp
// ProducerThread (one per channel):
while (!stop_requested) {
    hd_videoenc_pull_out_buf(enc_path, &data_pull, timeout);  // blocks until frame ready
    // Assemble all NAL packs into flat buffer; detect IDR; cache SPS/PPS/VPS
    ring_buffer.Write(data, size, timestamp, channel, is_keyframe, codec, sps, pps, vps, nal_packs);
    hd_videoenc_release_out_buf(enc_path, &data_pull);         // release immediately after copy
    frame_cv.notify_all();                                     // wake all consumers
}
```

The encoder output buffer is `mmap`'d at startup; NAL pack data is accessed via pointer arithmetic (`vir_addr + (phy_addr_offset)`), enabling near-zero-copy into the ring buffer slot.

#### Read Path (Consumer)

Each consumer (`VideoFrameConsumer`) owns a monotonic `next_sequence_` cursor:

```cpp
bool VideoFrameConsumer::WaitForFrame(VideoFrame& out_frame, int timeout_ms) {
    if (GetNextFrame(out_frame)) return true;     // already available
    frame_cv.wait_for(lock, timeout_ms_duration); // block until producer signals
    return GetNextFrame(out_frame);
}
```

If the consumer has fallen behind (its target slot was overwritten), `AdvanceToValidSequence()` auto-skips to the nearest IDR after the oldest available slot, incrementing `frames_dropped_`. This prevents consumer lag from stalling the producer.

#### Late-Join Support

On construction, each `VideoFrameConsumer` calls `ResetToKeyframe()`, which scans the ring buffer for the most recent IDR frame. This means a consumer starting mid-stream immediately gets a valid decode start point and the cached SPS/PPS/VPS needed to initialise the codec.

#### Pause/Resume for Codec Changes

`VideoControl::ApplyToHardware()` orchestrates live encoder parameter changes safely:

```
1. MediaHub::PauseChannel(ch)       — ProducerThread enters idle loop
2. HdalPipeline::Set*(...)          — Change codec/bitrate/resolution/fps in HDAL
3. MediaHub::ResumeChannel(ch, codec_changed)
       — If codec changed: clear cached SPS/PPS, flush stale frames, request IDR
       — Resume producer thread
4. RtspServer::RefreshStream(ch)    — Rebuild Live555 subsession with new codec
5. RecordingService::CutNow(ch)     — Rotate recording segment at codec boundary
```

---

## 4. Module Catalogue

### `modules/ai` — AI Analytics

Orchestrated by `AnalyticsEngine` (singleton). Subsystems:

- **NpuInference** — Loads YOLOv5s ONNX model, runs inference on 640×360 YUV frames from HDAL VideoProc path.
- **MotionDetectionEngine** — Pixel-diff on 160×120 YUV; configurable zones, threshold, schedule.
- **ObjectTracker** — Multi-object tracking; emits first-seen events per track ID.
- **TamperDetectionEngine** — Laplacian variance + scene change; detects camera blocking/defocus/shift.
- **VqaEngine** — Video quality analysis for scene-change triggered events.
- **PrivacyMosaicEngine** — Applies mosaic blur to configured regions before encoding.
- **AiispEngine** — AI-based ISP enhancement (denoising, HDR recovery) on low-light scenes.
- Line crossing, zone intrusion, people counting, face detection, LPR, audio classification — implemented as sub-engines within `AnalyticsEngine`.

All AI subsystems fire events via callbacks registered by `main.cpp` (see §3.3).

---

### `modules/platform` — Hardware Abstraction

- **HdalPipeline** — The sole owner of all HDAL path IDs. Opens sensor, ISP, VideoProc, up to 4 encoder paths, 1 audio path, plus dedicated AI/MD/VQA paths. Split across multiple `.cpp` files by concern: `_init`, `_lifecycle`, `_video`, `_audio`, `_config`.
- **HdalWrapper** — Wraps `vendor_isp_*` APIs; normalises all settings to 0–100 before calling vendor enums.
- **ISPControl** — High-level typed structs (`ImageAdjustment`, `WhiteBalance`, `ExposureSettings`, etc.) over `HdalWrapper`. Loads/saves from `isp.json`.
- **IRControl** — IR LED brightness via PWM, IR cut filter motor via dual-GPIO H-bridge pulse, auto day/night via SW-CDS thresholds or schedule.
- **OsdOverlay** — FreeType-based timestamp, text, logo overlays; up to 4 privacy masks per stream via HDAL OSG paths; font auto-scales with stream resolution.
- **ResetButton** — Polls GPIO12 at 100 ms; 10-second hold triggers factory reset callback.
- **SystemLogger** — Replaces `system_logger.sh`; collects kernel ring buffer, SoC temperature, CPU/memory/disk metrics, app log mirror; writes to SD card (30-day retention) and flash (3 MB cap).

---

### `modules/media` — Media Distribution

- **MediaHub** — Ring buffer hub (described in §3.5).
- **VideoControl** — Wraps `HdalPipeline` encoder setters; serialises concurrent API calls; orchestrates pause/resume cycle; fires `codec_change_callback_` to notify RTSP server.
- **AudioControl** — Audio capture config; provides audio frames via `AudioFrameBroadcaster`.

---

### `modules/streaming` — Output Streams

- **RTSP server** — Live555-based; `HdalVideoSource` is a `FramedSource` subclass that calls `VideoFrameConsumer::WaitForFrame()`; strips Annex-B start codes; computes RTP timestamps from HDAL hardware timestamps. Supports H.264 and H.265.
- **Go2rtcManager** — Manages go2rtc process for WebRTC/HLS output alongside RTSP.
- **AudioFrameBroadcaster** — Distributes G.711 (raw) and AAC-LC (for recording) audio frames.
- **OnvifMetadataGenerator** — ONVIF metadata stream with bounding-box XML; fed from `DetectionCallback`.

---

### `modules/recording` — Storage Recording

- **RecordingService** — One `RecordingThread` per channel; creates `VideoFrameConsumer` from `MediaHub`; writes to MP4 via `MP4Recorder` (minimp4 wrapper) or raw Annex-B file; optional AES-256-CTR encryption per segment; rotates segments on timer, forced cut, or codec change.
- **ScheduleManager** — Polls schedule config; calls `RecordingService::Start/Stop` on schedule boundaries.
- **MP4Recorder** — Handles Annex-B stripping, SPS/PPS/VPS `hvcC`/`avcC` box creation, fragmented MP4 writes, audio track muxing.

---

### `modules/events` — Event System

Described fully in §3.3. Source files by concern:

| File | Responsibility |
|------|---------------|
| `event_types.h` / `.cpp` | `EventType`, `EventCategory`, `ActionType`, `EventData` variant, `Event` struct |
| `event_rule.h` / `.cpp` | `EventRule`, `Action`, `ActionConfig` — rule matching logic |
| `event_manager.h` / `.cpp` | Queue, processing thread, listener registry, rule engine, action dispatch |
| `action_handler.h` / `.cpp` | Base class, factory (`RegisterAll`), `SubstitutePlaceholders` |
| `src/actions/recording_action.cpp` | Recording action handler |
| `src/actions/snapshot_action.cpp` | Snapshot/thumbnail action handler |
| `src/actions/email_action.cpp` | Email action handler (SMTP via SmtpManager) |
| `src/actions/webhook_action.cpp` | HTTP webhook handler (libcurl, with retry) |
| `src/actions/mqtt_action.cpp` | MQTT publish handler (Paho, conditionally compiled) |
| `src/actions/ftp_action.cpp` | FTP upload handler (via FtpManager) |
| `src/actions/io_action.cpp` | Alarm output, white light, siren handlers |

---

### `modules/networking` — Network Services

- **NetworkManager** — Facade over all networking concerns; IPv4/v6 config, DNS, hostname, SSL certs, nginx, NTP, SMTP, SNMP, UPnP, WiFi, link monitor.
- **NtpManager** — Pure C++ RFC 5905 NTP implementation; no `ntpd` dependency; falls back through pool + hardcoded IP addresses (Google 216.239.35.0, Cloudflare 162.159.200.1, NIST 129.6.15.28, Apple 17.253.34.123).
- **NginxManager** — Generates `nginx.conf` from current network settings; manages nginx process lifecycle.
- **SslManager** — Generates self-signed certificates (2048-bit RSA, 10 years) or installs custom PEM certs; validates key/cert pairing before install.
- **MdnsResponder** — Lightweight mDNS/DNS-SD via raw `AF_INET` sockets and `IP_ADD_MEMBERSHIP`; no avahi/dbus; announces `_http._tcp`, `_https._tcp`, `_rtsp._tcp`, `_onvif._tcp`.
- **SmtpManager** — SMTP (none/SSL-TLS/STARTTLS) via libcurl; credentials stored in `CredentialManager`.
- **SnmpManager** / **UpnpManager** — SNMPv1/v2c/v3 daemon config; miniupnpc-based UPnP port mapping.

---

### `modules/config` — Configuration

Described fully in §3.4. Also contains:

- **UserManager** — SQLite+SQLCipher-backed user/session management; PBKDF2-HMAC-SHA256 (210,000 iterations) for passwords; AES-256-GCM for ONVIF WSSE plaintext storage; brute-force lockout (5 attempts → 15 min).
- **paths.h** — All filesystem path constants as `constexpr` strings.

---

### `modules/onvif` — ONVIF Protocol

ONVIF WS-Discovery (UDP 3702) and SOAP service (TCP 5000) generated via gSOAP. Exposes Device, Media, Events, and PTZ (stub) services. `OnvifMetadataGenerator` provides the analytics event metadata stream.

---

### `modules/webserver` — REST API

Embedded HTTP/HTTPS server (configurable port, default 8082). Handlers in `src/handlers/` cover: device info, network config, video streams, ISP/image settings, recording, storage, users/auth, events, analytics config, system info/logs, firmware upgrade. Handler `src/handlers/analytics/` provides dedicated analytics REST endpoints.

---

## 5. Startup and Shutdown Sequence

See §3.2 for the detailed table. Key dependency constraints:

```
config::Init()
  └─ must complete before: ALL other subsystems

HdalPipeline::Start()
  └─ must complete before: MediaHub::Start()

MediaHub::Start()
  └─ must complete before: streaming::Start(), RecordingService::Initialize()

storage::Init()
  └─ must complete before: RecordingService::Initialize()

EventManager::Start()
  └─ wires analytics callbacks immediately after init
  └─ publishes kSystemStartup

On shutdown: MediaHub stops AFTER RTSP server and recording;
             HdalPipeline stops AFTER MediaHub.
```

---

## 6. Video Pipeline Data Flow

```
┌──────────────────────────────────────────────────────────────────────────┐
│                        Hardware / HDAL SDK                               │
│                                                                          │
│  GC5603 MIPI Sensor → ISP (AE/AWB/3DNR/WDR) → VideoProc                │
│                                   │                                      │
│          ┌────────────────────────┼────────────────────────────┐         │
│          │ AI path 640×360 YUV   │ Enc[0] 2944×1664 H.265     │         │
│          │ MD path 160×120 YUV   │ Enc[1] 1280×720  H.264     │         │
│          │ VQA path 320×180 YUV  │ Enc[2] 640×480   H.264     │         │
│          │                       │ Enc[3] 640×360   H.264     │         │
└──────────┼───────────────────────┼────────────────────────────┼─────────┘
           │                       │                            │
    ┌──────▼──────┐        ┌───────▼──────┐             Audio PCM path
    │AnalyticsEng │        │  MediaHub    │                    │
    │ NpuInference│        │  ProducerThreads × 4             HdalPipeline
    │ MD/VQA/Tamper│       │  VideoRingBuffer × 4 (30 slots)  AudioControl
    └──────┬──────┘        └───────┬──────────────────┘       AudioFrameBroadcaster
           │                       │                                │
    Analytics callbacks     frame_cv.notify_all()           G.711/AAC-LC
    (via main.cpp wiring)          │                               │
           │               ┌───────┴──────────┬──────────────┐    │
           ▼               │                  │              │    │
    EventManager    [VideoFrameConsumer] [VideoFrameConsumer] │   │
    PublishEvent()   "rtsp-ch0..3"       "recording-ch0..3"   │   │
                           │                  │               │   │
                    HdalVideoSource    RecordingThread         │   │
                    (Live555)          MP4Recorder             │   │
                    RTP packetise      AES-256-CTR encrypt     │   │
                    → RTSP clients     → SD card / NAS         │   │
                                                               │   │
                                                    Go2rtcManager  │
                                                    (WebRTC/HLS)   │
                                                                   │
                                                          RTSP audio track
                                                          MP4 audio track
```

---

## 7. Event System Data Flow

```
AI/Hardware Sources                EventManager                   Action Handlers
──────────────────                 ────────────────               ────────────────
AnalyticsEngine
  MotionCallback()         ──▶    PublishMotionStart()
  DetectionCallback()      ──▶    PublishPersonDetected()    ──▶  RecordingActionHandler
  LineCrossCallback()      ──▶    PublishLineCrossed()       ──▶  SnapshotActionHandler
  LoiteringCallback()      ──▶    PublishZoneIntrusion()     ──▶  EmailActionHandler
  TamperCallback()         ──▶    PublishTamperEvent()       ──▶  WebhookActionHandler
  LprCallback()            ──▶    PublishLprDetected()       ──▶  MqttActionHandler
  AudioEventCallback()     ──▶    PublishAudioDetected()     ──▶  FtpUploadHandler
                                                             ──▶  IoOutputHandler

Platform / System
  IRControl day/night       ──▶   PublishSystemEvent()       ──▶  (rule-matched)
  NetworkManager            ──▶   PublishNetworkEvent()
  StorageService            ──▶   PublishStorageEvent()
  HW alarm input polling    ──▶   PublishIOEvent()

                                        │
                                  event_queue (thread-safe)
                                        │
                                  ProcessingThread (single)
                                        ├── AddToHistory()
                                        ├── NotifyListeners()   ← free EventListener callbacks
                                        └── MatchRules()
                                                └── ExecuteActions() [detached thread per action]
```

---

## 8. Configuration Layering

```
Boot time:
─────────
config.factory.d/
  device.json   ─┐
  media.json     │
  network.json   │  sorted, merged via json::merge_patch()
  ...            │
  (17 more)    ──┘──▶  g_factory_tree  (baseline snapshot)
                              │
              config.d/*.json (deployment overrides, optional)
                              │
              config.json     (user delta — only changed keys)
                              │
                              ▼
                        g_config_tree  (live runtime state)

Runtime write:
──────────────
Set("network.rtsp.port", 8554)  →  g_config_tree mutated in-place
Save()  →  diff(g_factory_tree, g_config_tree)  →  write delta to config.json (atomic rename)

Factory reset:
──────────────
Delete config.json  →  on next boot, g_config_tree == g_factory_tree
```

---

## 9. Dependency Map

```
                        ┌──────────────────────────────────┐
                        │           main.cpp               │
                        │   (Coordinator / Orchestrator)   │
                        └──┬─────────┬──────────┬──────────┘
                           │         │          │
              ┌────────────▼──┐  ┌───▼───────┐  └──────────────┐
              │  HdalPipeline │  │ ConfigSys │                  │
              │  (HDAL SDK)   │  │ (JSON/nloh│                  │
              └───────┬───────┘  └───────────┘                  │
                      │ encoder paths                            │
              ┌───────▼───────┐                                  │
              │   MediaHub    │                                  │
              │ (Ring Buffer) │                                  │
              └──┬─────────┬──┘                                  │
           RTSP  │         │  Recording                          │
    ┌────────────▼──┐  ┌───▼──────────────┐                      │
    │ streaming::   │  │ RecordingService  │                      │
    │ RtspServer    │  │ ScheduleManager   │                      │
    │ Go2rtcManager │  │ NasManager/FTP    │                      │
    └───────────────┘  └───────────────────┘                      │
                                                                  │
    ┌──────────────────────────────────────────────────────────────▼──┐
    │                     AnalyticsEngine                             │
    │  NpuInference  MotionDetection  ObjectTracker  Tamper  LPR ...  │
    └──────────────────────────┬──────────────────────────────────────┘
                               │  callbacks (wired in main.cpp)
    ┌──────────────────────────▼──────────────────────────────────────┐
    │                       EventManager                              │
    │  event_queue → ProcessingThread → NotifyListeners + MatchRules  │
    └──────────────────────────┬──────────────────────────────────────┘
                               │  action dispatch (detached threads)
          ┌────────────────────┼────────────────────────┐
          │                    │                        │
    EmailHandler        WebhookHandler           RecordingHandler
    MqttHandler         FtpHandler               SnapshotHandler
    IoOutputHandler     ...                      ...

    ┌──────────────────────────────────────────────────────────────────┐
    │                     NetworkManager                               │
    │  NtpManager  NginxManager  SslManager  MdnsResponder  SMTP/SNMP │
    └──────────────────────────────────────────────────────────────────┘

    ┌──────────────────┐   ┌──────────────────┐   ┌──────────────────┐
    │  ISPControl      │   │  IRControl       │   │  OsdOverlay      │
    │  HdalWrapper     │   │  (day/night)     │   │  ResetButton     │
    └──────────────────┘   └──────────────────┘   └──────────────────┘
            │                      │                       │
            └──────────────────────┴───────────────────────┘
                        all read config via ipcam::config::Get<T>()
                        all are singletons accessed via ::Instance()
```

---

*Document generated from source inspection of `code/application/ipcamera/` on the `cursor/eterna-architecture-documentation-7d1e` branch.*
