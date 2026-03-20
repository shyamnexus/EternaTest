# Eterna IP Camera — Concurrency & Thread-Safety Analysis

> **Codebase root:** `code/application/ipcamera/`  
> **Scope:** All modules verified by source inspection.

---

## Table of Contents

1. [Threading Model Overview](#1-threading-model-overview)
2. [Mechanisms by Category](#2-mechanisms-by-category)
   - 2.1 [Lock-Free SPMC Ring Buffer (MediaHub)](#21-lock-free-spmc-ring-buffer-mediahub)
   - 2.2 [Classic Mutex-Protected Queues](#22-classic-mutex-protected-queues)
   - 2.3 [Atomic Lifecycle Flags](#23-atomic-lifecycle-flags)
   - 2.4 [Deadlock Prevention](#24-deadlock-prevention)
   - 2.5 [Callback Safety](#25-callback-safety)
   - 2.6 [Thread Lifecycle Safety](#26-thread-lifecycle-safety)
   - 2.7 [Filesystem Atomicity](#27-filesystem-atomicity)
   - 2.8 [Interruptible Sleep & Responsive Shutdown](#28-interruptible-sleep--responsive-shutdown)
3. [Per-Module Summary](#3-per-module-summary)
4. [Known Residual Risks](#4-known-residual-risks)
5. [Quick Reference — All Mutexes and Atomics](#5-quick-reference--all-mutexes-and-atomics)

---

## 1. Threading Model Overview

`ipcamd` is a multi-threaded process. The threads in the system are:

| Thread | Owner | Purpose |
|--------|-------|---------|
| `main` | `main.cpp` | Signal-wait loop; startup/shutdown coordinator |
| `ProducerThread × N` | `MediaHub` | One per active video channel; pulls frames from HDAL encoder |
| `ProcessingThread` | `EventManager` | Single consumer of the event queue; dispatches rules and listeners |
| `ActionHandler × N` | `EventManager` | Detached per-action; runs recording, email, webhook, MQTT, FTP, I/O |
| `ProcessingThread` | `AnalyticsEngine` | Main AI inference loop (NPU + object tracking) |
| `MdProcessingThread` | `AnalyticsEngine` | Motion detection (160×120 YUV path) |
| `AudioProcessingThread` | `AnalyticsEngine` | Audio classification |
| `RecordingThread × N` | `RecordingService` | One per active recording channel; MP4 write to SD/NAS |
| `MonitorThread` | `RecordingService` | Polls storage availability every 5 s |
| `UploadWorker` | `FtpManager` | Queued FTP upload processing |
| `AutoDetectionLoop` | `IRControl` | Luma-threshold day/night switching |
| `ScheduleMonitorLoop` | `IRControl` | Time-based day/night schedule |
| `SwCdsDetectionLoop` | `IRControl` | Software CDS (ISP EV-value) day/night detection |
| Live555 event loop | `RtspServer` | Single-threaded RTSP/RTP dispatch (Live555 framework) |
| Link monitor | `NetworkManager` | Periodic interface link-state polling |
| SystemLogger worker | `SystemLogger` | Periodic dmesg/temp/stats collection |

---

## 2. Mechanisms by Category

### 2.1 Lock-Free SPMC Ring Buffer (MediaHub)

The `VideoRingBuffer` in `modules/media/src/media_hub.cpp` is the most performance-critical concurrent data structure in the system. It implements a **seqlock-inspired Single-Producer Multiple-Consumer (SPMC)** design that requires no mutex on the read path.

#### Odd/Even Version Field (Torn-Read Prevention)

Each ring buffer slot carries a `std::atomic<uint64_t> version` field:

```cpp
// Write path (ProducerThread only):
slot.version.store(seq * 2 - 1, std::memory_order_release); // odd = writing
memcpy(slot.buffer, frame_data, size);
// ... copy SPS/PPS/VPS, nal_packs ...
slot.version.store(seq * 2, std::memory_order_release);     // even = ready

// Read path (any consumer, lock-free):
uint64_t ver = slot.version.load(std::memory_order_acquire);
if (ver != seq * 2) return false;   // in-progress or different frame
memcpy(out.data.data(), slot.buffer, slot.size);
// ... copy fields ...
if (slot.version.load(std::memory_order_acquire) != seq * 2) return false; // torn-read check
```

- The leading check (`ver != seq * 2`) rejects any slot currently being written (odd) or already overwritten (different even).  
- The trailing re-read (`ver_after`) catches the case where the producer wraps around and *begins* overwriting the slot mid-`memcpy`.
- `memory_order_release` on both stores ensures all intermediate field writes are visible to any consumer that observes the even version via `memory_order_acquire`.

#### False-Sharing Prevention (`alignas(64)`)

The two high-frequency atomic counters are placed on separate cache lines:

```cpp
alignas(64) std::atomic<size_t>   write_pos_{0};   // written every frame by producer
alignas(64) std::atomic<uint64_t> frames_written_{0}; // read by every consumer
```

Without this, every producer write would invalidate the cache line shared by all consumer reads, causing false sharing across cores.

#### Codec-Change Coordination (`paused` atomic + `frame_mutex`)

When a codec or resolution change is applied at runtime:

```
1. VideoControl::ApplyToHardware()
   └─ MediaHub::PauseChannel(ch)
         ch.paused.store(true, memory_order_release)
         sleep(kVideoPullTimeoutMs + 50ms)  // let producer spin out of HDAL call
   └─ HdalPipeline::SetStream*()           // change encoder params
   └─ MediaHub::ResumeChannel(ch, codec_changed)
         {lock_guard frame_mutex}
           clear cached_sps/pps/vps
           update cached_codec
         ch.paused.store(false, memory_order_release)
         FlushEncoderBuffer()              // discard stale frames
         hd_videoenc request IDR
```

`frame_mutex` guards the cached NAL parameters (`cached_sps`, `cached_pps`, `cached_vps`, `cached_codec`) against a race between the codec-change path and the producer thread which reads these on each keyframe.

---

### 2.2 Classic Mutex-Protected Queues

#### EventManager — Event Queue

```
Publishers (any thread)            ProcessingThread
────────────────────               ──────────────
lock(queue_mutex_)
event_queue_.push(event)
unlock(queue_mutex_)
queue_cv_.notify_one()  ────────►  lock(queue_mutex_)
                                   queue_cv_.wait(lock, predicate)
                                   event = event_queue_.front()
                                   event_queue_.pop()
                                   unlock(queue_mutex_)
                                   ProcessEvent(event)
```

The `wait` predicate `!event_queue_.empty() || !running_.load()` guards against spurious wakeups. `notify_one()` is intentionally called outside the lock — there is no missed wakeup because the condition is re-evaluated under lock on the consumer side.

#### FtpManager — Upload Queue

Two separate mutexes are used to allow maximum concurrency:

| Mutex | Protects | Allows concurrent |
|-------|----------|-------------------|
| `mutex_` | `config_`, `stats_`, callbacks | Queue enqueue/dequeue |
| `queue_mutex_` | `upload_queue_`, `upload_history_` | Config reads/writes |

`GetStats()` always acquires in the order `mutex_` → `queue_mutex_`. `UploadWorker()` follows the same order. This consistent ordering prevents lock-ordering deadlocks.

---

### 2.3 Atomic Lifecycle Flags

All long-lived manager classes use `std::atomic<bool>` for lifecycle state rather than guarding it with the class mutex. This allows callers to safely check `IsRunning()` / `IsInitialized()` from any thread without acquiring the main lock.

| Class | Atomic flags | Purpose |
|-------|-------------|---------|
| `MediaHub` | `initialized_`, `running_`, `next_consumer_id_` | Double-init prevention; `running_.exchange(false)` for idempotent stop |
| `EventManager` | `initialized_`, `running_`, `next_listener_id_` | Processing thread loop condition and stop signal |
| `AnalyticsEngine` | `initialized_`, `running_`, `md_thread_running_`, `audio_thread_running_` | Per-thread stop flags; dual-thread access guard for MD engine |
| `RecordingService` | `initialized_`, `storage_available_`, `shutdown_requested_` | Cross-thread visibility for monitor thread updates |
| `RecordingService` (per-channel) | `stop_requested`, `force_segment_cut` | Recording thread termination; segment-cut signal |
| `FtpManager` | `initialized_`, `shutdown_requested_` | Upload worker stop signal |
| `NasManager` | `initialized_` | Init/shutdown gate |
| `HdalPipeline` | `state_` (`atomic<PipelineState>`) | Safe reads from `IsRunning()` etc. without lock |

#### `force_segment_cut` — Notable Use of Explicit Memory Ordering

```cpp
// Caller (any thread):
ch_state.force_segment_cut.store(true, std::memory_order_release);

// Recording thread:
bool force_cut = ch_state.force_segment_cut.exchange(false, std::memory_order_acq_rel);
```

- `release` store: ensures all preceding writes (e.g., new encoder settings) are visible to the recording thread when it observes `true`.
- `acq_rel` exchange: atomically reads-and-clears; the `acquire` half pairs with the `release` store to receive those preceding writes; prevents two consecutive cuts if `CutNow()` is called twice rapidly.

#### `running_.exchange` in `MediaHub::Stop()` — Idempotent Stop

```cpp
bool was_running = running_.exchange(false, std::memory_order_release);
// Always signal stop and join threads regardless of was_running.
// Producer threads hold HDAL VB pool references; joining before munmap
// prevents a kernel-level use-after-free.
```

Using `exchange` rather than a store-then-check ensures that even if two threads call `Stop()` concurrently, both see a consistent value and only one attempts the `join()`.

#### `md_thread_running_` — Dual-Thread Access Guard

```cpp
// Main analytics thread:
if (!md_thread_running_.load()) {
    // MD thread not running, dispatch frame directly
    md_engine.ProcessFrame(frame);
}
```

The `MotionDetectionEngine` is not thread-safe. This atomic flag is the only thing preventing both the main processing thread and the dedicated MD thread from calling `ProcessFrame()` concurrently — it acts as a lightweight lock-free selector.

---

### 2.4 Deadlock Prevention

Four distinct deadlock scenarios are explicitly identified and mitigated in the code.

#### Pattern A — `unique_lock::unlock()` Before Re-Entrant Call

`HdalPipeline::SetStreamEnabled()` must call `ReinitWithConfig()`, which internally calls `Shutdown()` → `Init()` → `Start()`, all of which acquire `g_pipeline_mutex`:

```cpp
std::unique_lock<std::mutex> lock(g_pipeline_mutex);
video_streams_[stream_id].enabled = true;

lock.unlock();               // ← explicit unlock before re-entrant path
bool ok = ReinitWithConfig();

if (!ok) {
    std::lock_guard<std::mutex> relock(g_pipeline_mutex); // re-acquire for rollback
    video_streams_[stream_id].enabled = false;
}
```

The same pattern is used in `IRControl::Init()` and `IRControl::CheckAndSwitchMode()` before calling `SetDayNightAutoMode()`, `SwitchToDayMode()`, and `SwitchToNightMode()`, all of which acquire `g_mutex`.

#### Pattern B — `Shutdown()` Defers Lock Until After `Stop()`

```cpp
void HdalPipeline::Shutdown() {
    // Note: We don't lock here because Stop() has its own lock
    // and we need to handle the recursive case
    if (state_ == PipelineState::kRunning) {
        Stop();           // acquires g_pipeline_mutex internally
    }
    std::lock_guard<std::mutex> lock(g_pipeline_mutex); // then we lock for cleanup
    ...
}
```

#### Pattern C — Copy-Before-Iterate in `EventManager::NotifyListeners()`

```cpp
void EventManager::NotifyListeners(const Event& event) {
    std::vector<ListenerEntry> listeners_copy;
    {
        std::lock_guard<std::mutex> lock(listeners_mutex_);
        listeners_copy = listeners_;   // ← copy under lock
    }
    // lock released — iterate and invoke callbacks without holding it
    for (const auto& entry : listeners_copy) {
        entry.listener(event);         // ← listener may call AddListener/RemoveListener
    }
}
```

If `listeners_mutex_` were held during the callback, and the callback called `AddListener()` or `RemoveListener()` (which also acquire `listeners_mutex_`), the result would be a deadlock. Copying first and releasing the lock before iterating prevents this. The trade-off is that a listener removed mid-iteration receives one final notification.

#### Pattern D — Lock Released Before All Callbacks (AnalyticsEngine)

The analytics engine consistently releases its `mutex_` before invoking any registered callback:

```cpp
// stats update under lock
{
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.frames_processed++;
    stats_.avg_process_time_ms = ...;
}
// callback invoked WITHOUT holding mutex_
if (detection_callback_) {
    detection_callback_(detection_frame);  // ← caller may call GetStats(), SetConfig()
}
```

`mutex_` is not recursive; holding it through a callback that re-enters any `AnalyticsEngine` API would deadlock. The same pattern is applied consistently to `motion_callback_`, `linecross_callback_`, `loitering_callback_`, `tamper_callback_`, `lpr_callback_`, and `face_recognition_callback_`.

---

### 2.5 Callback Safety

A consistent pattern is used across most modules: **callbacks are always invoked outside the owning mutex**.

| Module | Callback(s) | Invoked without lock? |
|--------|-------------|----------------------|
| `AnalyticsEngine` | `detection_callback_`, `motion_callback_`, `linecross_callback_`, `loitering_callback_`, `tamper_callback_`, `lpr_callback_`, `face_recognition_callback_` | Yes — `mutex_` released before each call |
| `IRControl` | `day_night_callback_` | Yes — `g_mutex` released in all code paths |
| `RecordingService` | `segment_callback_`, `status_callback_` | Yes — acquired/released separately in narrow scope |
| `FtpManager` | `complete_callback_`, `progress_callback_` | Yes — released before callback at line 524 |
| `EventManager` (listeners) | All `EventListener` callbacks | Yes — copy-before-iterate pattern |
| `EventManager` (action detached threads) | All `ActionHandler::Execute()` | Yes — `ProcessingThread` never holds any lock when dispatching |

**Exception — `audio_event_callback_` in `AnalyticsEngine`:**  
Unlike all other callbacks, `audio_event_callback_` is fired while holding `mutex_`:

```cpp
// analytics.cpp ~line 2558
std::lock_guard<std::mutex> lock(mutex_);
if (audio_event_callback_) {
    audio_event_callback_(ev);    // ← mutex_ still held
}
```

If the audio callback calls back into any `AnalyticsEngine` method that acquires `mutex_`, a deadlock results. This is an inconsistency with the rest of the callback pattern.

---

### 2.6 Thread Lifecycle Safety

#### Thread Join as a Happens-Before Fence

`std::thread::join()` is a synchronisation point: all operations performed by the joined thread before it exits are guaranteed visible to the joining thread. The codebase relies on this property in several critical places:

| Join site | Hazard prevented |
|-----------|-----------------|
| `MediaHub::Stop()` → join ProducerThreads → then `CleanupChannelHdal()` (munmap) | Use-after-free of HDAL VB pool memory pages in the kernel |
| `AnalyticsEngine::Stop()` — join audio → join MD → join processing | MD engine shared between main and MD thread; joining MD first before main ensures no concurrent `ProcessFrame()` after main thread exits |
| `EventManager::Stop()` → join ProcessingThread | All in-flight `ProcessEvent()` calls (including `AddToHistory`, `NotifyListeners`, `MatchRules`) complete before `Stop()` returns |
| `FtpManager::Shutdown()` → join UploadWorker → then `curl_global_cleanup()` | UB from calling `curl_global_cleanup()` with a transfer in progress |
| `IRControl::SetDayNightAutoMode()` → join old mode threads → start new | Prevents two control threads competing to drive the IR cut filter GPIO |

All join sites use `joinable()` guards:

```cpp
if (thread_.joinable()) {
    thread_.join();
}
```

Calling `join()` on a non-joinable thread is undefined behaviour (`std::terminate()` in most implementations).

#### Detached Action Threads — Known Limitation

`EventManager::ExecuteActions()` dispatches each action in a `std::thread::detach()`:

```cpp
std::thread([handler, action, event, rule_name = rule.name, this]() {
    auto result = handler->Execute(event, action);
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.actions_executed++;
    }
}).detach();
```

Frame/action data (`handler`, `action`, `event`) are captured by **value**, preventing use-after-free on those objects. However, `this` is captured by pointer. If `EventManager::Shutdown()` completes and the singleton is destroyed while a detached thread is still in `handler->Execute()`, the subsequent `stats_mutex_` acquisition will be a use-after-free. `Stop()` joins only `processing_thread_`, not detached action threads. This is a known limitation.

---

### 2.7 Filesystem Atomicity

Two modules use the write-to-temp + atomic `rename()` pattern to prevent partial-write corruption:

**Config system (`modules/config/src/config.cpp`):**
```cpp
std::string tempPath = savePath + ".tmp";
// ... write full JSON to tempPath ...
if (rename(tempPath.c_str(), savePath.c_str()) != 0) {
    unlink(tempPath.c_str());
    return false;
}
```

**NetworkManager — certificate update (`modules/networking/src/network_manager.cpp`):**
```cpp
rename(cert_path.c_str(), backup_cert.c_str());   // backup old
rename(key_path.c_str(),  backup_key.c_str());
// write new cert/key to temp files
rename(temp_cert.c_str(), cert_path.c_str());      // atomic swap in
rename(temp_key.c_str(),  key_path.c_str());
// if failure: restore backup
```

Any process reading the certificate (nginx, go2rtc, ONVIF) sees either the complete old file or the complete new file — never a partially-written intermediate. POSIX guarantees that `rename()` on the same filesystem is atomic.

---

### 2.8 Interruptible Sleep & Responsive Shutdown

Plain `std::this_thread::sleep_for()` cannot be interrupted. All long-sleeping background threads use `std::condition_variable::wait_for()` with a stop-flag predicate instead:

**`IRControl` — all three background loops:**
```cpp
std::unique_lock<std::mutex> lock(g_mutex);
g_thread_cv.wait_for(lock, std::chrono::seconds(1), [this]() {
    return !sw_cds_thread_running_.load();
});
// g_thread_cv.notify_all() in Shutdown() wakes this immediately
```

**`FtpManager` upload worker:**
```cpp
upload_cv_.wait_for(lock, std::chrono::seconds(60), [this]() {
    return shutdown_requested_.load() || !upload_queue_.empty();
});
// upload_cv_.notify_all() in Shutdown() wakes immediately
```

**`RecordingService` monitor thread — chunked sleep:**
```cpp
for (int i = 0; i < 50 && !shutdown_requested_; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

This breaks a 5-second interval into 50 × 100 ms slices, checked against the atomic stop flag each time. The maximum shutdown delay is bounded to ~100 ms rather than ~5 s.

---

## 3. Per-Module Summary

| Module | Mutexes | Atomics | Condition Vars | Background Threads | Notable Pattern |
|--------|---------|---------|----------------|-------------------|-----------------|
| `MediaHub` | `frame_mutex` (per channel) | `version` (per slot), `write_pos_`, `frames_written_`, `paused`, `stop_requested`, `initialized_`, `running_`, `consumer_count` | `frame_cv` (per channel) | 1 ProducerThread per channel | Seqlock-style versioning; `alignas(64)` false-sharing prevention |
| `EventManager` | `queue_mutex_`, `rules_mutex_`, `listeners_mutex_`, `history_mutex_`, `stats_mutex_`, `config_mutex_` | `initialized_`, `running_`, `next_listener_id_` | `queue_cv_` | 1 ProcessingThread + N detached action threads | Copy-before-iterate; callbacks outside all locks |
| `AnalyticsEngine` | `mutex_` (single instance mutex) | `initialized_`, `running_`, `md_thread_running_`, `audio_thread_running_` | None | 3 (processing, MD, audio) | Narrow lock scope before callbacks; MD engine dual-access guard |
| `HdalPipeline` | `g_pipeline_mutex` (global) | `state_` | None | None (all calls synchronous) | `unique_lock.unlock()` before re-init; `Shutdown()` defers lock |
| `RecordingService` | `config_mutex_`, `callback_mutex_`, `playback_mutex_` | `initialized_`, `storage_available_`, `shutdown_requested_`, per-channel `stop_requested`, `force_segment_cut` | None | 1 RecordingThread per channel + 1 MonitorThread | Explicit `acq_rel` on segment-cut exchange; lock ordering `config_mutex_` → `callback_mutex_` |
| `IRControl` | `g_mutex` (global, shared by all 3 loops) | `auto_thread_running_`, `schedule_thread_running_`, `sw_cds_thread_running_` | `g_thread_cv` (global) | 3 (auto, schedule, SW CDS) | Manual `unlock()`/`relock()` to avoid recursive deadlock; all callbacks outside lock |
| `FtpManager` | `mutex_`, `queue_mutex_` | `initialized_`, `shutdown_requested_` | `upload_cv_` | 1 UploadWorker | Consistent lock order `mutex_` → `queue_mutex_` |
| `NasManager` | `mutex_` | `initialized_` | None | None | — |
| `Config` | `g_mutex` (anonymous-namespace global) | None | None | None | Atomic `rename()` for persistence; TOCTOU-safe `Init()` |
| `RtspServer` | None in-file | None | None | 0 (Live555 event loop model) | `triggerEvent` for cross-thread frame delivery; documented use-after-free hazard from `handleClosure()` |
| `NetworkManager` | None in-file | None | None | 1 link-monitor | Atomic `rename()` for cert update; delegates to per-singleton managers |

---

## 4. Known Residual Risks

These are race conditions or thread-safety gaps found during the analysis. They are not fatal bugs in normal operation (the code paths are low-frequency or guarded by convention), but they are technical debt.

### R1 — `audio_event_callback_` fired while holding `mutex_` (AnalyticsEngine)

**Location:** `modules/ai/src/analytics.cpp` ~line 2558  
**Risk:** If the audio event callback calls back into any `AnalyticsEngine` API that acquires `mutex_`, a deadlock occurs. All other callback types correctly release `mutex_` first.  
**Fix:** Hoist the callback out of the lock scope, consistent with the pattern used for `detection_callback_`, `motion_callback_`, etc.

### R2 — Detached action threads capture `this` into `EventManager` (EventManager)

**Location:** `modules/events/src/event_manager.cpp` ~line 797  
**Risk:** If `EventManager` is destroyed (or `Shutdown()` returns) while a detached action thread is still in flight, the subsequent `stats_mutex_` acquisition in the lambda is a use-after-free.  
**Fix:** Track outstanding action threads with a counter (`std::atomic<int>`) and drain them in `Shutdown()` before returning, or use `std::shared_ptr<EventManager>` captures in detached lambdas.

### R3 — `MatchRules()` returns raw pointers into `rules_` vector (EventManager)

**Location:** `modules/events/src/event_manager.cpp` ~line 769  
**Risk:** `MatchRules()` returns `std::vector<const EventRule*>` pointing into `rules_`. After the lock is released, a concurrent `AddRule()`, `DeleteRule()`, or `UpdateRule()` may reallocate the vector or erase the pointed-to element, causing a dangling-pointer dereference in `ExecuteActions()`.  
**Fix:** Return by value (`std::vector<EventRule>`) from `MatchRules()` instead of raw pointers.

### R4 — `config_.queue_max_size` read in `PublishEvent()` without `config_mutex_` (EventManager)

**Location:** `modules/events/src/event_manager.cpp` ~line 207  
**Risk:** Data race between `SetConfig()` (which holds `config_mutex_`) and `PublishEvent()` (which does not) on `config_.queue_max_size`. In practice harmless (int read is effectively atomic on all supported architectures) but is formally a C++ data race.  
**Fix:** Read `config_.queue_max_size` once under `config_mutex_` into a local variable at the start of `PublishEvent()`, or cache it as a `std::atomic<int>`.

### R5 — `NasManager::TestConnection()` writes `config_` without lock when called with `nullptr` (NasManager)

**Location:** `modules/storage/src/nas_manager.cpp` ~line 175  
**Risk:** `config_.last_error` and `config_.status` are written at lines 179–180 without holding `mutex_`. A concurrent `GetConfig()` or `SetConfig()` creates a data race.  
**Fix:** Acquire `mutex_` for the full duration of `TestConnection()` when `config == nullptr`, or copy `config_` under lock into a local before the network operation.

### R6 — `FtpManager::UploadWorker()` reads `config_` under `queue_mutex_` only (FtpManager)

**Location:** `modules/storage/src/ftp_manager.cpp` ~lines 455, 464  
**Risk:** `config_.upload_schedule` and `config_.enabled` are read while only `queue_mutex_` is held. `SetConfig()` (which holds `mutex_`) can race on these fields.  
**Fix:** Snapshot the relevant config fields under `mutex_` at the start of each worker iteration.

### R7 — `SwCdsDetectionLoop()` reads `is_night_mode_` and `auto_settings_.mode` without lock (IRControl)

**Location:** `modules/platform/src/ir_control.cpp` ~lines 929, 940  
**Risk:** `is_night_mode_` and `auto_settings_.mode` are plain (non-atomic) fields, read outside `g_mutex` after the `wait_for` lock is released. `SwitchToNightMode()` / `SetDayNightAutoMode()` write them under `g_mutex`. Formally a C++ data race.  
**Fix:** Either make these fields `std::atomic`, or perform the reads inside a narrow `lock_guard` scope before the decision logic.

### R8 — `Init()` / `Start()` TOCTOU in `EventManager` (EventManager)

**Location:** `modules/events/src/event_manager.cpp` ~lines 38, 64  
**Risk:** The check `if (initialized_.load())` and the subsequent `initialized_ = true` are not performed atomically (no enclosing mutex). Two concurrent `Init()` callers could both pass the check.  
**Mitigation in practice:** `Init()` is only ever called from `main.cpp` during single-threaded startup.  
**Fix:** Use `initialized_.exchange(true)` to atomically check-and-set, or wrap both steps in a mutex guard.

---

## 5. Quick Reference — All Mutexes and Atomics

### Mutexes

| Variable | Defined in | Protects |
|----------|-----------|---------|
| `g_mutex` (config) | `config.cpp` anonymous ns | `g_config_tree`, `g_factory_tree`, `g_cache`, `g_initialized`, `g_config_dir` |
| `g_pipeline_mutex` | `hdal_pipeline_lifecycle.cpp` | All HDAL path handles, stream configs, encoder state |
| `g_mutex` (IRControl) | `ir_control.cpp` anonymous ns | `ir_led_settings_`, `ir_cut_settings_`, `auto_settings_`, `is_night_mode_`, callbacks |
| `frame_mutex` (per channel) | `MediaHub::ChannelState` | `cached_sps/pps/vps`, `cached_codec`; condition variable pairing |
| `queue_mutex_` | `EventManager` | `event_queue_` |
| `rules_mutex_` | `EventManager` | `rules_` vector |
| `listeners_mutex_` | `EventManager` | `listeners_` vector |
| `history_mutex_` | `EventManager` | `event_history_` deque |
| `stats_mutex_` | `EventManager` | `EventStats` struct |
| `config_mutex_` (EventManager) | `EventManager` | `EventManagerConfig` struct |
| `mutex_` | `AnalyticsEngine` | `config_`, `stats_`, all 8 callback members |
| `config_mutex_` (RecordingService) | `RecordingService` | `RecordingConfig`, channel configs |
| `callback_mutex_` | `RecordingService` | `segment_callback_`, `status_callback_`, `storage_callback_` |
| `playback_mutex_` | `RecordingService` | `playback_sessions_` map |
| `mutex_` | `NasManager` | `config_`, `initialized_`, mount state |
| `mutex_` | `FtpManager` | `config_`, `stats_`, callbacks |
| `queue_mutex_` | `FtpManager` | `upload_queue_`, `upload_history_` |

### Atomics

| Variable | Type | Defined in | Purpose |
|----------|------|-----------|---------|
| `version` (per slot) | `atomic<uint64_t>` | `VideoRingBuffer::Slot` | Seqlock-style torn-read prevention; odd = writing, even = ready |
| `write_pos_` | `atomic<size_t>` | `VideoRingBuffer` | Lock-free slot allocation (producer only) |
| `frames_written_` | `atomic<uint64_t>` | `VideoRingBuffer` | Latest sequence; stale-slot detection |
| `stop_requested` | `atomic<bool>` | `MediaHub::ChannelState` | Producer thread termination signal |
| `paused` | `atomic<bool>` | `MediaHub::ChannelState` | Codec-change idle signal |
| `consumer_count` | `atomic<uint32_t>` | `MediaHub::ChannelState` | Ref count for consumer tracking |
| `initialized_`, `running_` | `atomic<bool>` | `MediaHub` | Lifecycle state |
| `next_consumer_id_` | `atomic<uint32_t>` | `MediaHub` | Lock-free unique ID assignment |
| `initialized_`, `running_` | `atomic<bool>` | `EventManager` | Lifecycle state; ProcessingThread loop condition |
| `next_listener_id_` | `atomic<uint32_t>` | `EventManager` | Unique listener ID (incremented under `listeners_mutex_`) |
| `initialized_`, `running_` | `atomic<bool>` | `AnalyticsEngine` | Lifecycle state; destructor reads `initialized_` without lock |
| `md_thread_running_` | `atomic<bool>` | `AnalyticsEngine` | Prevents dual-thread access to MD engine |
| `audio_thread_running_` | `atomic<bool>` | `AnalyticsEngine` | Audio thread stop signal |
| `state_` | `atomic<PipelineState>` | `HdalPipeline` | Safe unlocked reads from `IsRunning()` |
| `initialized_` | `atomic<bool>` | `RecordingService` | Init gate |
| `storage_available_` | `atomic<bool>` | `RecordingService` | Written by MonitorThread; read by recording threads |
| `shutdown_requested_` | `atomic<bool>` | `RecordingService` | Recording + monitor thread stop signal |
| `stop_requested` (per channel) | `atomic<bool>` | `RecordingService::ChannelState` | Per-channel recording thread stop |
| `force_segment_cut` (per channel) | `atomic<bool>` | `RecordingService::ChannelState` | Segment rotation signal; `acq_rel` exchange |
| `initialized_`, `shutdown_requested_` | `atomic<bool>` | `FtpManager` | Init gate; worker stop signal |
| `initialized_` | `atomic<bool>` | `NasManager` | Init/shutdown gate |
| `auto_thread_running_`, `schedule_thread_running_`, `sw_cds_thread_running_` | `atomic<bool>` | `IRControl` | Per-thread stop signals |

---

*Document generated from source inspection of `code/application/ipcamera/` — branch `cursor/eterna-architecture-documentation-7d1e`.*
