# Release Notes v1.1.002

**Release Date:** January 11, 2026  
**Build Status:** ✅ Successful  

---

## Summary

This release focuses on recording module enhancements including raw H.264/H.265 file format support, audio muxing fixes, scheduled recording automation, and API improvements for auto-start/stop recording when channel enabled states change.

---

## New Features

### 1. Raw H.264/H.265 File Format Support
- **Added `RawH264Writer` class** for writing raw Annex-B H.264/H.265 files
- Supports direct NAL unit output without MP4 container overhead
- Configured via `FileFormat::Raw` channel setting
- Audio is automatically disabled for raw format recordings
- Useful for low-latency recording or post-processing workflows

### 2. Background Schedule Manager Worker
- **Added background scheduler worker thread** that monitors schedule profiles
- Automatically starts/stops recording based on active schedule time slots
- Respects channel-specific schedule overrides
- Worker automatically starts when scheduling is enabled and stops when disabled
- 60-second polling interval for schedule state checks
- Graceful handling when storage is unavailable

### 3. Recording Auto-Start/Stop on Config Changes
- Recording handler now **auto-starts recording** when a channel is enabled via API
- Recording handler now **auto-stops recording** when a channel is disabled via API
- Works for both global config updates (`/api/v1/recording/config`) and per-channel updates (`/api/v1/recording/channels/{id}`)
- Response includes `actions` array detailing what recording operations were performed

---

## Bug Fixes

### Audio Muxing Fix
- **Fixed `audio_cap_path` initialization** in recording thread
- Previously hardcoded to `0` (TODO stub), now properly calls `pipeline.GetAudioCapturePath()`
- Added null check with warning when audio capture path unavailable
- Audio is now properly muxed into MP4 recordings when enabled

---

## Code Changes

### Files Modified

| File | Changes |
|------|---------|
| [recording.cpp](../../../code/application/ipcamera/modules/recording/src/recording.cpp) | +147 lines |
| [schedule_manager.cpp](../../../code/application/ipcamera/modules/recording/src/schedule_manager.cpp) | +166 lines |
| [schedule_manager.h](../../../code/application/ipcamera/modules/recording/include/ipcam/schedule_manager.h) | +36 lines |
| [recording_handler.cpp](../../../code/application/ipcamera/modules/webserver/src/handlers/recording_handler.cpp) | +84 lines |
| [onvif_main.c](../../../code/application/ipcamera/modules/onvif/src/onvif_main.c) | Minor fixes |

### Detailed Changes

#### recording.cpp
```cpp
// NEW: RawH264Writer class (lines 66-118)
class RawH264Writer {
public:
    bool Open(const std::string& filename);
    void Close();
    bool IsOpen() const;
    bool WriteNalUnit(const uint8_t* data, size_t size);
    uint64_t GetBytesWritten() const;
    uint64_t GetFrameCount() const;
private:
    FILE* file_ = nullptr;
    std::string filename_;
    uint64_t bytes_written_ = 0;
    uint64_t frame_count_ = 0;
};

// FIXED: Audio capture path initialization
// Before: HD_PATH_ID audio_cap_path = 0;  // TODO
// After:
HD_PATH_ID audio_cap_path = static_cast<HD_PATH_ID>(pipeline.GetAudioCapturePath());
if (audio_cap_path == 0 && mp4_config.enable_audio) {
    spdlog::warn("Audio capture path not available, audio disabled");
    mp4_config.enable_audio = false;
}

// NEW: Format selection logic
bool use_raw_format = (ch_config.format == FileFormat::Raw);
if (use_raw_format) {
    raw_writer.Open(filename);
    mp4_config.enable_audio = false;  // No audio in raw format
} else {
    mp4_recorder.Open(mp4_config);
}
```

#### schedule_manager.h
```cpp
// NEW: Background worker thread members
std::unique_ptr<std::thread> worker_thread_;
std::atomic<bool> worker_running_{false};
std::atomic<bool> worker_stop_requested_{false};
std::condition_variable worker_cv_;
std::mutex worker_mutex_;
std::array<bool, 3> channel_recording_state_{false, false, false};

// NEW: Public methods
void StartSchedulerWorker();
void StopSchedulerWorker();
bool IsSchedulerWorkerRunning() const;
```

#### schedule_manager.cpp
```cpp
// NEW: SchedulerWorkerLoop() - Background thread function
void ScheduleManager::SchedulerWorkerLoop() {
    while (!worker_stop_requested_.load()) {
        // Check schedule enabled state
        // Check active profile and time slots
        // Start/stop recording on channels as needed
        // Wait 60 seconds between checks
    }
}

// MODIFIED: Init() now starts worker if enabled
if (config_.enabled) {
    StartSchedulerWorker();
}

// MODIFIED: SetEnabled() manages worker lifecycle
if (enabled) {
    StartSchedulerWorker();
} else {
    StopSchedulerWorker();
}
```

#### recording_handler.cpp
```cpp
// NEW: HandleRecordingConfigUpdate auto-start/stop
std::vector<bool> old_enabled_states;
for (const auto& ch : config.channels) {
    old_enabled_states.push_back(ch.enabled);
}
// ... after config save ...
for (size_t ch_id = 0; ch_id < config.channels.size(); ch_id++) {
    if (!was_enabled && now_enabled) {
        service.StartRecording(ch_id, type);
    } else if (was_enabled && !now_enabled) {
        service.StopRecording(ch_id);
    }
}

// NEW: HandleRecordingChannelUpdate auto-start/stop
bool was_enabled = current.enabled;
// ... after channel config save ...
if (!was_enabled && updated.enabled) {
    service.StartRecording(channel, type);
} else if (was_enabled && !updated.enabled) {
    service.StopRecording(channel);
}
```

---

## API Changes

### Recording Config Update Response
**Endpoint:** `PUT /api/v1/recording/config`

Response now includes `actions` array when recording state changes:
```json
{
    "success": true,
    "message": "Configuration updated",
    "actions": [
        "Started recording on channel 0",
        "Stopped recording on channel 1"
    ]
}
```

### Recording Channel Update Response
**Endpoint:** `PUT /api/v1/recording/channels/{id}`

Response now includes `action` field when recording state changes:
```json
{
    "success": true,
    "message": "Channel configuration updated",
    "channel": { ... },
    "action": "Started recording on channel 0"
}
```

---

## Audio Codec Support

Both RTSP streaming and MP4 recording use the same audio encoding pipeline:

| Codec | RTSP | Recording | Notes |
|-------|------|-----------|-------|
| G.711 μ-law (PCMU) | ✅ | ✅ | Default, 8kHz |
| G.711 A-law (PCMA) | ✅ | ✅ | 8kHz |
| G.726 (16/24/32/40 kbps) | ✅ | ✅ | ADPCM variants |
| PCM (raw) | ✅ | ✅ | Uncompressed |
| AAC | ❌ | ❌ | SDK lacks encoder |

**Note:** AAC encoding is not available as the Novatek SDK does not include an AAC encoder. When AAC is requested, the system falls back to G.711 μ-law.

---

## Known Limitations

1. **AAC Audio Not Supported** - SDK does not provide AAC encoder
2. **Raw Format Limitations** - No audio track, no seeking metadata
3. **G.711/G.726 in MP4** - Limited player compatibility (VLC works, some mobile players may not)

---

## Testing Checklist

- [ ] Recording starts automatically when channel enabled via WebUI
- [ ] Recording stops automatically when channel disabled via WebUI
- [ ] Scheduled recording starts/stops at configured times
- [ ] Raw H.264 format produces valid Annex-B files
- [ ] MP4 recordings have audio when `media.audio.enabled=true`
- [ ] API responses include action feedback
- [ ] FIFO cleanup works for both MP4 and raw formats

---

## Build Information

```
Build Target: ipcam_recording
Build Type: Release
Architecture: aarch64-ca53
Compiler: GCC (cross-compilation)
Exit Code: 0 (Success)
```

---

## Dependencies

No new dependencies added. Existing dependencies:
- spdlog (logging)
- nlohmann/json (JSON parsing)
- minimp4 (MP4 muxing)
- OpenSSL (encryption)
- HDAL (hardware abstraction)

---

## Migration Notes

No breaking changes. All new features are backward compatible:
- Raw format is opt-in via channel configuration
- Auto-start/stop behavior is automatic but non-breaking
- Schedule worker starts automatically if scheduling was already enabled
