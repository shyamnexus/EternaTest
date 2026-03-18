
## 1. Event Service
**File**: `src/onvif_events.c`
**Goal**: Enable VMS (e.g., Milestone, Genetec) to receive Motion and Alarm triggers. Current implementation is empty.

### 1.1 `CreatePullPointSubscription`
*   **Current**: Returns `SOAP_OK` with no data.
*   **Requirement**:
    1.  Generate a unique UUID `SubscriptionReference` Address.
    2.  Allocate a session context in memory (Session ID + Creation Timestamp + Termination Time).
    3.  Return the `SubscriptionReference` URI to the client.

### 1.2 `PullMessages` (Missing)
*   **Current**: Not implemented.
*   **Requirement**:
    1.  Implement a blocking wait loop respecting the `PT<Seconds>` timeout request.
    2.  Integrate with `libmd` (Motion Detection) callbacks or a global event queue.
    3.  Construct `wsnt__NotificationMessage` XML structures.
    4.  **Payload**: Must send `tns1:RuleEngine/CellMotionDetector/Motion` events with `IsMotion=true/false` data.

### 1.3 `GetEventProperties`
*   **Current**: Returns `SOAP_OK` with empty body.
*   **Requirement**:
    1.  Populate `TopicExpressionDialect`.
    2.  Define `wstop:TopicSet` tree listing supported topics:
        *   `tns1:RuleEngine/CellMotionDetector/Motion`
        *   `tns1:Device/Trigger/DigitalInput`
        *   `tns1:VideoSource/SignalLoss`

---

## 2. Media2 Service
**File**: `src/onvif_media2.c`
**Goal**: Profile T Compliance (H.265 Support).

### 2.1 `GetVideoEncoderConfigurations`
*   **Current**: Uses `RK_MPI_VENC_GetChnAttr`.
*   **Requirement**: Rewrite to use `ipcam_config` or HDAL APIs.
    *   Map `H264`/`H265` profiles.
    *   Retrieve Resolution/FPS from `user_manager.cpp` Config Managers.

### 2.2 `SetVideoEncoderConfiguration`
*   **Current**: Calls `RK_MPI_SYS_UnBind`, `RK_MPI_VENC_SetChnAttr`.
*   **Requirement**: Refactor to Novatek Pipeline.
    *   Call `NM_Media_SetVideoSize` / `NM_Media_SetFrameRate`.
    *   Ensure H.265 (HEVC) is correctly exposed.

### 2.3 `GetProfiles`
*   **Requirement**: Generate profiles including H.265 encoder configurations if the hardware supports it.

---

## 3. Analytics Service
**File**: `src/onvif_analytics.c`
**Goal**: Profile M Compliance (Motion Metadata).

### 3.1 `CreateRules` / `ModifyRules`
*   **Current**: Stubs.
*   **Requirement**:
    *   Parse `tt:MotionRegion` XML (Polygon coordinates).
    *   Scale 0-100 coordinates to Novatek coordinate system.
    *   Call Novatek Motion API (e.g., `NVT_MD_SetRegion`).

### 3.2 Metadata Stream
*   **Requirement**:
    *   Modify RTSP Server to include a metadata track (trackID=2 usually).
    *   Inject ONVIF XML metadata frames describing object bounding boxes sync'd with video frames.

---

## 4. Recording Service
**File**: `src/onvif_recording.c`
**Goal**: Profile G Compliance (Edge Storage).

### 4.1 `GetRecordings`
*   **Current**: `item->Tracks = NULL`.
*   **Requirement**:
    *   Populate `tt__GetRecordingsResponseItem_Tracks`.
    *   Define `TrackToken` and `TrackType` (Video/Audio).
    *   This ensures the VMS knows *what* is inside the recording without playing it.

---

## 5. Architectural Clean-Up
*   **Dependency Audit**: Remove all `#include <rockit.h>` and `rk_*` references from `onvif_media2.c` and `onvif_imaging.c`.
*   **Database**: Verify `usermng.db` schema supports the new "Rules" tables needed for Analytics persistence.

## 6. Completed Modules (Reference)

### Device Service (Core)
**File**: `src/onvif_device.c`
*   **Discovery**: WS-Discovery functional; device is discoverable by ODM/VMS.
*   `GetSystemDateAndTime`: Functional.
*   `GetCapabilities`: Functional (reports Media, Events, etc).
*   `GetServices`: Functional.
*   `SystemReboot`: Mapped to system reboot.

### Media Service v1 (Profile S)
**File**: `src/onvif_media.c`
*   `GetProfiles`: Generates profiles for Main/Sub streams.
*   `GetStreamUri`: Returns valid RTSP URIs.
*   `GetSnapshotUri`: Functional.
*   `GetVideoSourceConfiguration`: Mapped to sensor inputs.

### Imaging Service (Basic)
**File**: `src/onvif_imaging.c`
*   `GetImagingSettings`: Reads Brightness/Contrast/Saturation from ISP.
*   `SetImagingSettings`: Writes Brightness/Contrast/Saturation to ISP.

### Recording Service (Profile G - Verified)
**File**: `src/onvif_recording.c`
*   `CreateRecordingJob`: Successfully starts recording backend.
*   `GetRecordingJobs`: Reports active jobs.
*   `GetRecordings`: Listings return correct file references from SD card.
*   `GetReplayUri`: Returns valid playback RTSP URI.
