# Eterna IP Camera — Design Practices & Optimisation Review

> Single-sheet review covering what is done well, where optimisation is applied, and where improvement is recommended.

---

## 1. Strong Design Practices

### 1.1 Separation of Concerns — Strict Module Boundaries

Each module owns exactly one responsibility, exposed through a minimal public header in `modules/<name>/include/ipcam/`. No module imports another module's singleton directly — all cross-cutting wiring (AI analytics → EventManager, MediaHub → RTSP) is done exclusively in `main.cpp`. This gives clean, test-friendly boundaries.

### 1.2 RAII Throughout

Every resource is owned by an object whose destructor cleans it up automatically:

| Resource | Owner / RAII pattern |
|----------|----------------------|
| HDAL ring buffer memory pool | `VideoRingBuffer::~VideoRingBuffer()` — calls `hd_common_mem_free()` or `std::free()` depending on which allocator succeeded |
| MediaHub consumer registration | `VideoFrameConsumer` (returned as `unique_ptr`) — destructor decrements `consumer_count` automatically |
| AudioFrameBroadcaster subscription | `AudioFrameConsumer` (returned as `unique_ptr`) — destructor calls `UnregisterConsumer()` |
| SQLite statement handle | `Statement::~Statement()` — calls `sqlite3_finalize()` |
| SQLite database handle | `Database::~Database()` — calls `sqlite3_close()` (skipped during global shutdown to avoid SQLCipher crash) |
| OpenSSL EVP key | `FirmwareValidator::Impl::~Impl()` — calls `EVP_PKEY_free()` |
| Pimpl heavy objects | `unique_ptr<Impl>` in `UpgradeManager`, `RtspServer`, `AacCodec`, `UserManager`, `PhysicalMemory` |

### 1.3 Pimpl Idiom for Heavy/Vendor-Dependent Classes

`UpgradeManager`, `SystemController`, `PhysicalMemory`, `FirmwareValidator`, `AacCodec`, `UserManager`, and `RtspServer` all use `unique_ptr<Impl>` to hide their implementation. This limits recompilation cascades when vendor SDK headers change, and keeps heavy dependencies out of including translation units.

### 1.4 Move Semantics Consistently Applied

`std::move` is used wherever ownership transfer is intended:

- All callbacks are stored with `std::move(callback)` (`VideoControl`, `EventManager`, `IRControl`, etc.) — avoids deep-copying `std::function` objects.
- `VideoFrameConsumer`, `MP4Recorder`, `Database`, `Statement` all have `noexcept` move constructors that move raw handles and `std::move` string/vector members.
- The EventManager processing thread uses `event = std::move(event_queue_.front())` to move events out of the queue without copying the `EventData` variant.
- ONVIF metadata frame queue uses `push(std::move(current_frame_))` and `std::move(frame_queue_.front())`.

### 1.5 Tiered Error Handling

Three layers, each used at the right abstraction level:

| Layer | Type | Used in |
|-------|------|---------|
| Internal business logic | `Result<T>` / `Result<void>` struct | `UserManager`, `NetworkManager`, `config` loaders |
| Optional reads | `std::optional<T>` / `std::nullopt` | `config::GetOptional`, `CredentialManager` |
| Firmware upgrade pipeline | `Expected<T, E>` (C++17 `std::expected` polyfill) | `UpgradeManager`, `FirmwareValidator` |
| API boundary | `try/catch` → structured JSON + HTTP status | All webserver handlers |

Exceptions are **never thrown or propagated** past the webserver boundary. The rest of the codebase uses value-based error propagation.

### 1.6 Conditional Compilation for Hardware Portability

Four feature guards allow the same codebase to build and run on non-HDAL hosts (e.g., a developer laptop):

| Guard | Controls |
|-------|---------|
| `HDAL_PIPELINE_ENABLED` | All HDAL SDK headers, encoder/capture/proc path calls, physically-contiguous memory allocation |
| `RTSP_SERVER_ENABLED` | All Live555 headers and `HdalVideoSource`; disabled → stub implementations compiled in |
| `MQTT_ENABLED` | Paho MQTT `async_client`; disabled → `MqttActionHandler::IsAvailable()` returns `false` |
| `OPENSSL_AVAILABLE` | RSA key material in `FirmwareValidator::Impl` |

### 1.7 Consistent `constexpr` for Hardware Sizing

All performance-critical capacity limits are compile-time constants, eliminating magic numbers on hot paths:

```cpp
// media_hub.h
static constexpr size_t kVideoRingBufferSlots  = 30;       // 1 second at 30 fps
static constexpr size_t kMaxVideoFrameSize     = 512*1024; // 512 KB main stream
static constexpr size_t kMaxSubStreamFrameSize = 128*1024; // 128 KB sub-streams
static constexpr int    kVideoPullTimeoutMs    = 100;
static constexpr size_t kMaxNalParamSize       = 256;

// audio_frame_broadcaster.h
static constexpr size_t kAudioRingBufferSlots  = 256;
static constexpr size_t kMaxAudioFrameSize     = 8192;
static constexpr int    kAudioPullTimeoutMs    = 50;

// paths.h — all filesystem paths
constexpr const char* kConfigDir    = "/etc/ipcamera/configs";
constexpr const char* kSdCardMount  = "/mnt/sd";
```

---

## 2. Performance Optimisations

### 2.1 Lock-Free SPMC Ring Buffer — Zero-Lock Read Path

The `VideoRingBuffer` is the hottest data structure in the system (written at up to 4 × 30 fps = 120 frames/sec). Reads (RTSP, recording, WebRTC) carry **no mutex overhead**:

- Odd/even atomic version field (`seqlock` style) detects torn reads without a lock.
- `memory_order_release` / `memory_order_acquire` pair provides the necessary happens-before fence.
- Double version check (before and after `memcpy`) catches overwrites that begin mid-copy.

### 2.2 Cache-Line Alignment on Shared Atomics

The two counters written/read on every frame are placed on separate 64-byte cache lines:

```cpp
alignas(64) std::atomic<size_t>   write_pos_{0};     // written by producer only
alignas(64) std::atomic<uint64_t> frames_written_{0}; // read by every consumer
```

Without `alignas(64)`, a producer write to `write_pos_` would invalidate the cache line shared by `frames_written_`, forcing all consumer cores to reload it. On a 4-core SoC this avoids a cache miss on every frame notification.

### 2.3 Physically-Contiguous HDAL Memory Pool

The ring buffer backing store is allocated from HDAL's DMA-capable physically-contiguous pool (`hd_common_mem_alloc`) rather than the heap:

- Single allocation at startup; zero runtime heap allocation per frame.
- The encoder output buffer is `mmap`'d at startup; NAL data is accessed via pointer arithmetic (`vir_addr + (phy_addr_offset)`) — near-zero-copy from kernel encoder ring buffer into the userspace ring buffer slot.
- Falls back to `std::malloc` automatically for non-HDAL builds.

### 2.4 `thread_local` Frame Assembly Buffer

The producer thread assembles NAL packs into a scratch buffer before writing to the slot:

```cpp
thread_local std::vector<uint8_t> frame_buffer;
frame_buffer.clear();
frame_buffer.reserve(total_size);
```

`thread_local` means the vector's capacity persists across frames — after a few frames the vector will never reallocate again, eliminating per-frame heap activity on the hot path.

### 2.5 Hardware Timestamps for RTP Presentation Time

The RTSP server derives all RTP presentation timestamps from the HDAL hardware encoder tick (`data_pull.timestamp`, in microseconds), **not from `gettimeofday()`**:

- `gettimeofday()` is called exactly **once** per stream (at the first frame) to anchor the wall-clock epoch.
- All subsequent frames use delta arithmetic: `delta_us = hw_timestamp - base_hw_timestamp_`.
- This eliminates system-call overhead on every frame and produces RTP timestamps that are jitter-free even when the process scheduler delays the producer thread.
- A sanity-reset triggers a new `gettimeofday()` anchor if the delta looks implausible (>10 s gap, backward jump, or timestamp wrap).

### 2.6 Narrow Lock Scopes / Snapshot Pattern

Locks are held only long enough to copy a value; all I/O and processing is done outside the lock:

```cpp
// RecordingService — snapshot config once at segment start
ChannelConfig ch_config;
{
    std::lock_guard<std::mutex> lock(config_mutex_);
    ch_config = config_.channels[channel];   // copy under lock
}
// long recording loop uses ch_config without holding any lock

// SetConfig() — update in-memory state under lock, then save to disk outside
{
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_ = config;
}
return SaveConfig();   // file I/O with no lock held
```

The same pattern is applied in `AnalyticsEngine::GetStats()`, `EventManager::GetConfig()`, `EventManager::GetRules()`, and `NasManager::GetConfig()`.

### 2.7 `reserve()` for Pre-Sized Buffers

Hot-path vectors are pre-sized to avoid reallocation:

```cpp
// Recording thread — AAC output buffer
aac_output_buffer.reserve(platform::AacCodec::kMaxOutputSize);  // 2048 bytes, one-time

// Producer thread — NAL pack list per frame
nal_packs.reserve(data_pull.pack_num);

// Streaming — RTSP send buffer
// kSendBufferSize = 8 * 1024 * 1024 (8 MB, set once on socket)
```

### 2.8 Delta-Only Config Persistence

`config::Save()` computes `diff(g_factory_tree, g_config_tree)` and writes **only changed keys** to `config.json`:

- Minimises flash write amplification — a single changed `int` value writes a few hundred bytes, not the full 50+ KB merged tree.
- Trivial factory reset: delete `config.json`, no re-flashing required.
- Atomic `rename()` ensures no partial-write on power loss.

### 2.9 Interruptible Sleep for Fast Shutdown

All background threads use `condition_variable::wait_for()` with a stop-flag predicate instead of `sleep_for()`. `notify_all()` in `Shutdown()` wakes them immediately:

- IRControl loops: up to 30-second sleep becomes < 1 ms wakeup latency.
- FtpManager upload worker: 60-second idle wait becomes immediate.
- RecordingService monitor: 50 × 100 ms slices, checked against atomic flag each time — max 100 ms latency.

---

## 3. Areas for Improvement

### 3.1 Duplicate `VideoControl::Init()` in `main.cpp`

`VideoControl::Instance().Init()` is called twice — at line ~357 and again at line ~751. The second call is a silent no-op today (guarded by `if (initialized_) return true`), but it creates confusion and masks if the early init genuinely failed.

**Fix:** Remove the duplicate call at line ~751.

### 3.2 `config::Set()` Has No Value Validation

Any caller can write `Set<int>("ir.led.brightness", 9999)` and it persists to disk. Validation only happens at each use site — a manual `config.json` edit or an API call bypasses it entirely.

**Fix:** Add a validator registration mechanism to the config module (a `RegisterValidator(path_prefix, ValidatorFn)` API), mirroring the approach already present in `ConfigLoader` (the YAML-based secondary path which has per-subsystem validators: `validateNetworkConfig`, `validateStorageConfig`, etc.).

### 3.3 `OsdOverlay` Uses `printf` Instead of `spdlog`

`modules/platform/src/osd_overlay.cpp` contains ~30+ `printf` calls and zero `spdlog` calls. OSD-related output is invisible to the structured log system, log files, and remote syslog.

**Fix:** Replace `printf("[OSD] ...")` with `spdlog::debug("[OSD] ...")` / `spdlog::error(...)` throughout `osd_overlay.cpp`.

### 3.4 Vendor Headers in Public `hdal_pipeline.h`

When `HDAL_PIPELINE_ENABLED` is set, `hdal_pipeline.h` pulls in 6 Novatek vendor headers (`hdal.h`, `hd_type.h`, `hd_debug.h`, `vendor_isp.h`, `vendor_videocapture.h`, `vendor_videoprocess.h`) into every including translation unit. HDAL-specific types (`HD_PATH_ID`, `HD_DIM`, `HD_VIDEOCAP_SYSCAPS`) appear directly in private member declarations.

**Fix:** Apply the Pimpl idiom to `HdalPipeline` — move all HDAL-typed members into a private `Impl` struct in `hdal_pipeline.cpp`. The public header would then only expose the high-level interface and include no vendor headers.

### 3.5 `audio_event_callback_` Fired While Holding `mutex_`

Unlike all 7 other `AnalyticsEngine` callbacks (which release `mutex_` before invoking), `audio_event_callback_` is fired while `mutex_` is still held. Any audio callback that calls back into `AnalyticsEngine` (e.g., `GetStats()`, `SetConfig()`) will deadlock.

**Fix:** Move the audio callback invocation outside the lock scope, consistent with the established pattern.

### 3.6 Missing `std::string_view` in API Parameters

All string parameters use `const std::string&` throughout. Hot-path calls that pass string literals (e.g., log tag construction, config path lookups) cause an implicit `std::string` construction on every call.

**Fix:** Adopt `std::string_view` for read-only string parameters in high-frequency paths — particularly `config::Get<T>(const std::string& path, ...)` and `MediaHub::CreateConsumer(int channel_id, const std::string& name)`.

### 3.7 Magic Numbers in `analytics.cpp` and `media_hub.cpp`

Several hardware-tuned constants are hardcoded as bare literals:
- Tamper detection VQA resolution: `320, 180` (no named constant)
- Defocus threshold: `100.0f`, dark threshold: `5.0f`, histogram threshold: `30.0f`
- Mosaic block size: `16`, blur radius: `21`, denoise strength: `50`
- HD sub-stream frame cap: `256 * 1024` in `media_hub.cpp` (sits between the named `kMaxVideoFrameSize` and `kMaxSubStreamFrameSize` — should be `kMaxHdSubStreamFrameSize`)

**Fix:** Name each constant with a `constexpr` in its module's header, or expose them as config keys.

### 3.8 No Tests for Platform, AI, Streaming, Recording, Webserver Modules

Five of the eleven modules have no test source files at all:

| Module | Status |
|--------|--------|
| `modules/events/` | Well covered — unit + integration + stress tests |
| `modules/config/` | Partial — loader and validation tests |
| `modules/media/` | Partial — `test_media_hub.cpp` |
| `modules/networking/` | Minimal — IP/DNS/MAC utilities only |
| `modules/platform/` | **No tests** |
| `modules/ai/` | **No tests** |
| `modules/streaming/` | **No tests** |
| `modules/recording/` | **No tests** |
| `modules/webserver/` | **No tests** |
| `modules/storage/` | Minimal — init only |
| `modules/utils/` | **No tests** |

**Fix (priority order):** `modules/media/` (ring buffer correctness is critical), `modules/recording/` (MP4 output, segment rotation), `modules/webserver/` (REST API contract), `modules/ai/` (analytics callback wiring).

### 3.9 Technical Debt — Acknowledged TODOs

Key items explicitly marked in source:

| File | What's missing |
|------|----------------|
| `modules/ai/src/analytics.cpp:2511` | `ProcessFrame()` is a **complete stub** — returns empty `DetectionFrame` without processing |
| `modules/webserver/src/handlers/video_handler.cpp:413` | Snapshot capture returns HTTP 503 — HDAL integration pending |
| `modules/upgrade/src/upgrade_manager.cpp:281` | Firmware upgrade from URL not implemented |
| `modules/recording/src/mp4_recorder.cpp:600,611` | H.264 and H.265 SPS parsing are stubs |
| `modules/events/src/actions/webhook_action.cpp:72` | TLS peer verification is hardcoded **disabled** (`CURLOPT_SSL_VERIFYPEER = 0`) |
| `modules/networking/src/network_manager.cpp:1492–1516` | WiFi scan/connect/forget/power — all 5 methods return `Err("not implemented")` |
| `modules/storage/src/storage.cpp:114` | Key derivation should be replaced with OP-TEE secure enclave call |
| `modules/onvif/src/onvif_profiles.cpp:212` | Recording job count always returns 0 |

---

## 4. Summary Scorecard

| Area | Rating | Notes |
|------|--------|-------|
| RAII & ownership | ✅ Strong | `unique_ptr` used consistently; all C handles wrapped |
| Move semantics | ✅ Strong | `noexcept` move ctors; callbacks stored with `std::move` |
| Ring buffer design | ✅ Strong | Lock-free SPMC; cache-line aligned; DMA-capable memory |
| Error handling | ✅ Strong | Tiered `Result`/`optional`/`Expected`; no exception leakage past API boundary |
| Lock discipline | ✅ Good | Narrow scopes; snapshot pattern; deadlock patterns explicitly mitigated |
| Conditional compilation | ✅ Good | 4 feature guards enable host-machine builds |
| `constexpr` usage | ✅ Good | All hot-path capacity constants named |
| Config validation | ⚠️ Weak | `Set()` accepts any value; no server-side validation at config layer |
| Logging consistency | ⚠️ Weak | `OsdOverlay` uses `printf`; all others use `spdlog` |
| `string_view` adoption | ⚠️ Weak | `const std::string&` everywhere; implicit constructions on hot paths |
| Test coverage | ❌ Poor | 5 of 11 modules have zero test files |
| Vendor header isolation | ⚠️ Weak | `hdal_pipeline.h` leaks 6 vendor headers into every includer |
| Known stubs | ⚠️ Present | `ProcessFrame()` empty; snapshot 503; WiFi unimplemented; TLS verify disabled |

---

*Document generated from source inspection of `code/application/ipcamera/` — branch `cursor/eterna-architecture-documentation-7d1e`.*
