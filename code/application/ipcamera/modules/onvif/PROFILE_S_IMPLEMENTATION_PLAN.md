# ONVIF Profile S — Full Implementation Plan

## Executive Summary

This document defines the complete roadmap to achieve ONVIF Profile S conformance
certification for the Novatek NT98538 IP Camera platform. The current gSOAP-based
ONVIF server has a solid foundation (~40 Device/Media operations working) but has
**4 critical blocking failures** that must be resolved before any conformance testing.

**Overall readiness estimate: ~55% complete**

---

## Current Status Matrix

### Legend
- ✅ = Working implementation, routed in dispatch
- ⚠️ = Stub / partial / disabled
- ❌ = Missing entirely (no implementation)
- 🔇 = Implementation exists but NOT routed in soapServer.c dispatch

---

## Phase 0 — Critical Blockers (MUST FIX FIRST)

These failures will cause immediate test abort during Profile S conformance testing.

### 0.1 Events Service — Complete Rewrite Required

**File:** `onvif_events.c` (currently 55 lines, 2 empty stubs)

| # | Operation | Current State | Profile S Requirement |
|---|-----------|--------------|----------------------|
| 1 | `GetEventProperties` | ⚠️ Returns empty SOAP_OK (no topic tree) | **Mandatory** — Must return TopicNamespaceLocation, TopicSet with tns1:VideoSource, tns1:Device topics, MessageContentFilterDialect, MessageContentSchemaLocation |
| 2 | `CreatePullPointSubscription` | ⚠️ Returns empty SOAP_OK (no subscription data) + **linker conflict** with stubs file | **Mandatory** — Must create subscription, return SubscriptionReference (EPR), CurrentTime, TerminationTime |
| 3 | `PullMessages` | ❌ Commented out in dispatch, no implementation | **Mandatory** — Must return NotificationMessages with topic and message content, respect timeout and message limit |
| 4 | `Renew` | ❌ Commented out in dispatch, no implementation | **Mandatory** — Must extend subscription termination time |
| 5 | `Unsubscribe` | ❌ Commented out in dispatch, no implementation | **Mandatory** — Must destroy subscription and free resources |
| 6 | `SetSynchronizationPoint` | ❌ Commented out in dispatch (events version), media version is stub | **Mandatory** — Must trigger property events for current state |

**Implementation Requirements:**

```
New Data Structures Needed:
├── PullPointSubscription (linked list or array)
│   ├── subscription_id (UUID)
│   ├── client_address (EPR)
│   ├── creation_time
│   ├── termination_time
│   ├── filter (topic expression)
│   └── event_queue (circular buffer of NotificationMessages)
├── EventTopic registry
│   ├── tns1:VideoSource/MotionAlarm (boolean, if analytics enabled)
│   ├── tns1:Device/Trigger/DigitalInput (boolean, if GPIO available)
│   ├── tns1:VideoAnalytics/... (if AI module enabled)
│   └── tns1:RuleEngine/CellMotionDetector/Motion
└── Thread-safe event queue with mutex protection
```

**Estimated size:** ~800-1200 lines of new C code  
**Estimated effort:** 3-5 days

**Specific tasks:**
1. Remove duplicate `__tev__CreatePullPointSubscription` from `onvif_services_stubs.c` (linker conflict)
2. Remove duplicate `__tev__GetEventProperties` from `onvif_services_stubs.c`
3. Remove `__tev__GetServiceCapabilities` from stubs (implement in events.c)
4. Create subscription manager with create/renew/destroy lifecycle
5. Implement circular event buffer per subscription (configurable depth, e.g. 256 msgs)
6. Implement `GetEventProperties` with proper TopicSet XML tree
7. Implement `PullMessages` with blocking wait (up to client timeout)
8. Implement `Renew` to extend termination time
9. Implement `Unsubscribe` to free subscription resources
10. Implement `SetSynchronizationPoint` to inject current-state property events
11. Uncomment dispatch entries in `soapServer.c` for: `PullMessages`, `Renew`, `Unsubscribe`, `SetSynchronizationPoint`
12. Add subscription expiration reaper thread (clean up expired subscriptions)
13. Wire event sources: motion detection from AI module, device state changes

### 0.2 GetSnapshotUri — Re-enable

**File:** `onvif_media.c` line 4311  
**Current state:** Returns `SOAP_FAULT` with "Snapshot support is temporarily disabled"  
**Backup file:** `onvif_media.c.backup` contains old implementation (line ~4238)

**Tasks:**
1. Review old implementation in backup file for correctness
2. Re-enable with proper JPEG snapshot endpoint URI construction
3. The snapshot URI should point to nginx or the RTSP server's snapshot endpoint
4. Format: `http://<device_ip>:<http_port>/onvif/snapshot?token=<ProfileToken>`
5. Ensure the HTTP handler for that endpoint actually serves a JPEG frame

**Estimated effort:** 0.5-1 day

### 0.3 Linker Symbol Conflicts

**Problem:** `onvif_events.c` and `onvif_services_stubs.c` BOTH define:
- `__tev__CreatePullPointSubscription`
- `__tev__GetEventProperties`

This causes undefined behavior (whichever object file the linker picks wins).

**Fix:** Remove the three `__tev__` stubs from `onvif_services_stubs.c` since `onvif_events.c` has the real (to-be-implemented) versions.

### 0.4 SetSystemFactoryDefault — Enable

**File:** `onvif_device.c`  
**Dispatch:** Commented out in `soapServer.c`  
**Requirement:** Mandatory for Profile S (Table 3.1 row 4.7)

**Tasks:**
1. Implement factory reset: delete users.db, restore config.factory.d → config.d, reboot
2. Support both `Hard` (full reset + reboot) and `Soft` (config only) modes
3. Uncomment dispatch entry in `soapServer.c`

**Estimated effort:** 1 day

---

## Phase 1 — Mandatory Features (Required for ALL Profile S devices)

### 1.1 Discovery (WS-Discovery)

| Operation | Status | Notes |
|-----------|--------|-------|
| WS-Discovery Hello | ✅ | Working in `onvif_main.c` |
| WS-Discovery Bye | ✅ | Working in `wsDiscovery.c` (comma operator bug fixed) |
| WS-Discovery Probe | ✅ | Working in `wsDiscovery.c` |
| WS-Discovery Resolve | ⚠️ | Need to verify |

**Remaining work:** Verify Resolve handler exists and works. Update scope URIs to not
falsely claim Profile T/G/M/Q conformance (currently claims all profiles in scopes).

**Tasks:**
1. Remove Profile/T, Profile/G, Profile/M, Profile/Q scope claims until those are actually conformant
2. Verify WS-Discovery Resolve is handled

### 1.2 Capabilities & Services

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetCapabilities` | ✅ | Full implementation |
| `GetServices` | ✅ | Full implementation |
| `GetServiceCapabilities` (Device) | ✅ | Full implementation |
| `GetServiceCapabilities` (Media) | ✅ | Full implementation |
| `GetServiceCapabilities` (Events) | ❌ | Stub in stubs file, needs real impl |

**Remaining work:**
1. Implement Events `GetServiceCapabilities` — must return:
   - WSPullPointSupport = true
   - WSSubscriptionPolicySupport = false
   - MaxNotificationProducers (integer)
   - MaxPullPoints (integer)
2. Verify capabilities responses include correct XAddr URLs for all services

### 1.3 System Functions

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetDeviceInformation` | ✅ | Works (returns Honeywell branding — see Phase 5) |
| `GetSystemDateAndTime` | ✅ | Full implementation with NTP/Manual |
| `SetSystemDateAndTime` | ✅ | Full implementation |
| `SystemReboot` | ✅ | Full implementation |
| `SetSystemFactoryDefault` | ❌ | **See Phase 0.4** |

### 1.4 User Management

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetUsers` | ✅ | Working via SQLite |
| `CreateUsers` | ✅ | Working via SQLite |
| `DeleteUsers` | ⚠️ | Has **use-after-free bug** — uses `l_username` after `free()` |
| `SetUser` | ✅ | Working via SQLite |

**Tasks:**
1. Fix use-after-free in `DeleteUsers` — save username before freeing, or restructure loop

### 1.5 Network Configuration

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetHostname` | ✅ | |
| `SetHostname` | ✅ | |
| `GetDNS` | ✅ | |
| `SetDNS` | ✅ | |
| `GetNetworkInterfaces` | ✅ | |
| `SetNetworkInterfaces` | ✅ | |
| `GetNetworkProtocols` | ✅ | |
| `SetNetworkProtocols` | ✅ | |
| `GetNetworkDefaultGateway` | ✅ | |
| `SetNetworkDefaultGateway` | ✅ | |

**Status:** COMPLETE — no work needed.

### 1.6 Scopes

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetScopes` | ✅ | Working |
| `SetScopes` | ⚠️ | Dispatched but implementation may be stub |
| `AddScopes` | ⚠️ | Dispatched but implementation is stub |
| `RemoveScopes` | ⚠️ | Dispatched but implementation is stub |

**Tasks:**
1. Verify `SetScopes` has a real implementation (not just returning SOAP_OK)
2. Implement proper `AddScopes` — persist configurable scopes to config file
3. Implement proper `RemoveScopes` — remove configurable scopes, reject fixed scopes
4. Distinguish between fixed scopes (hardware, profile) and configurable scopes (name, location)

### 1.7 Media Profile Management

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetProfiles` | ✅ | Returns MainStream + SubStream profiles |
| `GetProfile` | ✅ | By token lookup |
| `CreateProfile` | ✅ | Working |
| `DeleteProfile` | ✅ | Working |
| `AddVideoSourceConfiguration` | ✅ | |
| `AddVideoEncoderConfiguration` | ✅ | |
| `AddMetadataConfiguration` | ✅ | |
| `RemoveVideoSourceConfiguration` | ✅ | |
| `RemoveVideoEncoderConfiguration` | ✅ | |
| `RemoveMetadataConfiguration` | ✅ | |

**Status:** COMPLETE — no work needed.

### 1.8 Video Source Configuration

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetVideoSources` | ✅ | |
| `GetVideoSourceConfigurations` | ✅ | |
| `GetVideoSourceConfiguration` | ✅ | |
| `GetVideoSourceConfigurationOptions` | ✅ | |
| `SetVideoSourceConfiguration` | ✅ | |
| `GetCompatibleVideoSourceConfigurations` | ✅ | |

**Status:** COMPLETE — no work needed.

### 1.9 Video Encoder Configuration

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetVideoEncoderConfigurations` | ✅ | Returns H264/H265 configs |
| `GetVideoEncoderConfiguration` | ✅ | |
| `GetVideoEncoderConfigurationOptions` | ✅ | |
| `SetVideoEncoderConfiguration` | ✅ | Applies via ISP/encoder APIs |
| `GetCompatibleVideoEncoderConfigurations` | ✅ | |
| `GetGuaranteedNumberOfVideoEncoderInstances` | ✅ | |

**Status:** COMPLETE — no work needed.

### 1.10 Metadata Configuration

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetMetadataConfigurations` | ✅ | |
| `GetMetadataConfiguration` | ✅ | |
| `GetMetadataConfigurationOptions` | ✅ | |
| `SetMetadataConfiguration` | ✅ | |
| `GetCompatibleMetadataConfigurations` | ✅ | |
| `AddMetadataConfiguration` | ✅ | |
| `RemoveMetadataConfiguration` | ✅ | |

**Status:** COMPLETE — no work needed.

### 1.11 Streaming

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetStreamUri` | ✅ | Returns rtsp://device_ip:554/stream<N> |
| `GetSnapshotUri` | ❌ | **See Phase 0.2** — DISABLED |
| RTSP RTP/UDP Unicast | ✅ | Via streaming module |
| RTSP RTP/TCP | ✅ | Via streaming module (interleaved) |
| RTSP RTSP/HTTP/TCP | ⚠️ | Need to verify RTSP-over-HTTP tunneling |

**Tasks:**
1. Re-enable `GetSnapshotUri` (Phase 0.2)
2. Verify RTSP-over-HTTP tunneling works (Profile S requires it)
3. Verify RTP multicast works if advertised (see Phase 2)

### 1.12 Security / Authentication

| Mechanism | Status | Notes |
|-----------|--------|-------|
| WS-UsernameToken (Digest) | ✅ | Working via `soap_wsse_verify_Password` |
| HTTP Digest Authentication | ⚠️ | Uses httpda plugin, need to verify end-to-end |
| No-auth fallback to ADMIN | ⚠️ | **Security bug** — previous fix was reverted |
| TLS/HTTPS | ⚠️ | Via nginx reverse proxy — need to verify ONVIF HTTPS XAddr |

**Tasks:**
1. Re-apply auth bypass fix: reject unauthenticated requests instead of granting ADMIN access
2. Verify HTTP Digest auth works for media requests (RTSP DESCRIBE etc.)
3. Ensure HTTPS XAddr URLs are returned in GetCapabilities when TLS is configured

---

## Phase 2 — Conditional Mandatory Features

These are mandatory **only if the device advertises the capability**. The device currently
advertises audio support (`GetAudioSources` returns AudioSource_1), so audio operations
are **conditionally mandatory**.

### 2.1 Audio Source Configuration (CONDITIONAL — device advertises audio)

| Operation | Status | Dispatch | Notes |
|-----------|--------|----------|-------|
| `GetAudioSources` | 🔇 Impl exists | COMMENTED | Returns AudioSource_1, mono — needs uncommenting |
| `GetAudioSourceConfigurations` | ✅ | ACTIVE | Returns AudioSourceConfig_1 |
| `GetAudioSourceConfiguration` | ❌ | COMMENTED | Need to implement (return config by token) |
| `GetAudioSourceConfigurationOptions` | ❌ | COMMENTED | Need to implement |
| `SetAudioSourceConfiguration` | ❌ | COMMENTED | Need to implement |
| `GetCompatibleAudioSourceConfigurations` | ❌ | COMMENTED | Need to implement |
| `AddAudioSourceConfiguration` | ❌ | COMMENTED | Need to implement |
| `RemoveAudioSourceConfiguration` | ❌ | COMMENTED | Need to implement |

**Alternative:** If audio hardware is not truly functional, **remove audio from capabilities**
to avoid this entire block of conditional requirements. Modify `GetAudioSources` to return
0 sources and remove audio from GetCapabilities response.

### 2.2 Audio Encoder Configuration (CONDITIONAL)

| Operation | Status | Dispatch | Notes |
|-----------|--------|----------|-------|
| `GetAudioEncoderConfigurations` | ❌ | COMMENTED | |
| `GetAudioEncoderConfiguration` | ❌ | COMMENTED | |
| `GetAudioEncoderConfigurationOptions` | ⚠️ Empty stub | ACTIVE | Returns empty response |
| `SetAudioEncoderConfiguration` | ❌ | COMMENTED | |
| `GetCompatibleAudioEncoderConfigurations` | ❌ | COMMENTED | |
| `AddAudioEncoderConfiguration` | ❌ | COMMENTED | |
| `RemoveAudioEncoderConfiguration` | ❌ | COMMENTED | |

### 2.3 Multicast Streaming (CONDITIONAL — if multicast advertised)

| Operation | Status | Dispatch |
|-----------|--------|----------|
| `StartMulticastStreaming` | ❌ | COMMENTED |
| `StopMulticastStreaming` | ❌ | COMMENTED |

**Decision needed:** If the RTSP server does not support RTP multicast, ensure capabilities
do NOT advertise multicast support. Then these operations are not required.

### 2.4 NTP Configuration

| Operation | Status | Notes |
|-----------|--------|-------|
| `GetNTP` | ✅ | Working |
| `SetNTP` | ✅ | Working |

**Status:** COMPLETE.

### 2.5 Dynamic DNS (CONDITIONAL)

| Operation | Status | Dispatch |
|-----------|--------|----------|
| `GetDynamicDNS` | ⚠️ | ACTIVE — need to verify implementation |
| `SetDynamicDNS` | ⚠️ | ACTIVE — need to verify implementation |

### 2.6 Zero Configuration (CONDITIONAL)

| Operation | Status | Dispatch |
|-----------|--------|----------|
| `GetZeroConfiguration` | ⚠️ | ACTIVE — need to verify |
| `SetZeroConfiguration` | ⚠️ | ACTIVE — need to verify |

### 2.7 IP Address Filtering (CONDITIONAL)

| Operation | Status | Dispatch |
|-----------|--------|----------|
| `GetIPAddressFilter` | ⚠️ | ACTIVE |
| `SetIPAddressFilter` | ⚠️ | ACTIVE |
| `AddIPAddressFilter` | ⚠️ | ACTIVE |
| `RemoveIPAddressFilter` | ⚠️ | ACTIVE |

### 2.8 Relay Outputs (CONDITIONAL)

| Operation | Status | Dispatch |
|-----------|--------|----------|
| `GetRelayOutputs` | ❌ | COMMENTED |
| `SetRelayOutputSettings` | ❌ | COMMENTED |

**Decision:** If no relay GPIO exists, ensure capabilities don't advertise relay outputs.

---

## Phase 3 — Quality & Correctness Fixes

These are not individual operation implementations but cross-cutting concerns that affect
conformance test pass rate.

### 3.1 Fix Scope Claims

**File:** `onvif_device.c` (GetScopes, ~line 950) and `onvif_main.c` (DiscoveryHello)

Currently claims conformance to Profile S, T, G, M, Q. Must only claim Profile S
until other profiles are actually conformant.

**Tasks:**
1. Keep only `onvif://www.onvif.org/Profile/Streaming` and `onvif://www.onvif.org/Profile/S`
2. Remove Profile/T, Profile/G, Profile/M, Profile/Q scope claims
3. Update `onvif_main.c` DiscoveryHello scopes to match

### 3.2 Fix Database Path Mismatch

**Files:** `onvif_device.c` vs `user_wrapper.cpp`

- `onvif_device.c`: `DATABASE_PATH "/mnt/app/ipcamera/users.db"`
- `user_wrapper.cpp`: `/mnt/app/ipcamera/db/users.db`

Must be consistent. Recommend using the `db/` subdirectory path.

### 3.3 Fix DeleteUsers Use-After-Free

**File:** `onvif_device.c` — `__tds__DeleteUsers`

After `free(l_username)`, the code continues to use `l_username`. Fix by saving the
username before freeing or restructuring the deallocation.

### 3.4 Re-apply Security Fixes (Previously Reverted)

**File:** `onvif_main.c` — `ValidateAccessibility`

The no-auth-header → ADMIN fallback was fixed in the previous session but reverted.
Re-apply: unauthenticated requests should get USER level access at most, or be rejected
for operations requiring higher privileges.

### 3.5 Fix GetCapabilities Audio/Analytics Consistency

Ensure that `GetCapabilities` and `GetServiceCapabilities` accurately reflect what's
actually implemented:
- If audio operations are not implemented, set `AudioSources=0`, `AudioOutputs=0`
- If analytics are limited, don't advertise full analytics support
- Multicast capability should match actual RTSP server capability

### 3.6 Error Response Compliance

Profile S conformance tests expect specific SOAP fault subcodes. Verify all error
responses use correct ONVIF-defined fault codes:
- `ter:InvalidArgVal` for bad parameters
- `ter:NoSuchProfile` for unknown profile tokens
- `ter:ActionNotSupported` for unimplemented operations
- `ter:NotAuthorized` for insufficient privileges
- `env:Sender` / `env:Receiver` top-level fault codes

---

## Phase 4 — OSD & Imaging (Already Working)

### 4.1 OSD Operations

| Operation | Status |
|-----------|--------|
| `GetOSDs` | ✅ |
| `GetOSD` | ✅ |
| `GetOSDOptions` | ✅ |
| `SetOSD` | ✅ |
| `CreateOSD` | ✅ |
| `DeleteOSD` | ✅ |

**Status:** COMPLETE (with previous bug fixes applied for color/position).

### 4.2 Imaging Service

| Operation | Status |
|-----------|--------|
| `GetImagingSettings` | ✅ |
| `SetImagingSettings` | ✅ |
| `GetOptions` | ✅ |
| `GetMoveOptions` | ✅ |
| `Move` (focus) | ✅ |
| `Stop` (focus) | ✅ |
| `GetStatus` | ✅ |
| `GetServiceCapabilities` | ✅ |

**Status:** COMPLETE.

---

## Phase 5 — Branding & Identity (Deferred / Policy Decision)

### 5.1 Device Identity

**Files affected:** `onvif_main.h`, `onvif.cpp`, `onvif_device.c`, `onvif_main.c`, `wsDiscovery.c`

Currently returns:
- Manufacturer: "Honeywell"  
- Model: "Honeywell IPCAM-5MP" / "IPCAM-5MP"
- FirmwareVersion: "V1.0.0"

Previous fixes to genericize these were reverted by user. This is a policy decision—
if the device is branded Honeywell, these values are correct. If not, update:
- `MANUFACTURER` macro in `onvif_main.h`
- Config defaults in `onvif.cpp`
- Scope `name/` values in `onvif_device.c` and `onvif_main.c`
- WS-Discovery `g_scopes` in `wsDiscovery.c`

---

## Implementation Priority & Schedule

```
Week 1: Phase 0 — Critical Blockers
├── Day 1-2: Events Service data structures + subscription manager
├── Day 2-3: PullMessages + Renew + Unsubscribe + event queue
├── Day 3-4: GetEventProperties topic tree + SetSynchronizationPoint
├── Day 4:   GetSnapshotUri re-enable + test
├── Day 5:   SetSystemFactoryDefault + linker conflict fix
│
Week 2: Phase 1 — Mandatory feature gaps + Phase 3 fixes
├── Day 1: Scopes (SetScopes, AddScopes, RemoveScopes)
├── Day 2: Auth fixes (re-apply security fix, verify HTTP Digest)
├── Day 3: DB path fix, DeleteUsers UAF fix, scope claims cleanup
├── Day 4: Events GetServiceCapabilities, capabilities consistency audit
├── Day 5: End-to-end testing with ONVIF Device Test Tool
│
Week 3: Phase 2 — Conditional features (audio decision)
├── Day 1: DECISION — implement audio or remove from capabilities
├── Day 2-3: If implementing: audio source/encoder config operations (~8 ops)
├── Day 4: Multicast decision + implementation or capability removal
├── Day 5: Verify DynamicDNS, ZeroConfig, IPFilter implementations
│
Week 4: Integration testing & certification prep
├── Day 1-2: Run ONVIF Profile S Test Tool full suite
├── Day 3-4: Fix failures found during test runs
├── Day 5: Final validation & documentation
```

---

## File Change Summary

| File | Changes | Estimated LOC |
|------|---------|---------------|
| `onvif_events.c` | **Major rewrite** — full Events service | +800-1200 |
| `onvif_services_stubs.c` | Remove 3 duplicate `__tev__` functions | -20 |
| `onvif_media.c` | Re-enable GetSnapshotUri, fix audio stubs | +50-100 |
| `onvif_device.c` | SetSystemFactoryDefault, fix scopes/AddScopes/RemoveScopes, fix DB path, fix UAF | +200-300 |
| `onvif_main.c` | Re-apply auth fix, update discovery scopes | +20-30 |
| `wsDiscovery.c` | Update scope claims | ~10 |
| `soapServer.c` | Uncomment dispatch entries for events + audio (if needed) | ~30 lines uncommented |
| `onvif_media.c` (audio) | Audio source/encoder config operations (if implementing audio) | +400-600 |
| New: `onvif_events_internal.h` | Event subscription manager header | +80-100 |

**Total new/modified code:** ~1,600-2,400 lines

---

## Key Decisions Required

Before implementation begins, the following decisions affect scope:

1. **Audio support:** Implement full audio operations OR remove audio from capabilities?
   - If hardware has a working microphone → implement (~8 operations, 400-600 LOC)
   - If no real audio → remove from capabilities (much less work)

2. **Multicast:** Does the RTSP server support RTP multicast?
   - If yes → implement Start/StopMulticastStreaming
   - If no → remove multicast from capabilities

3. **Factory reset behavior:** What should SetSystemFactoryDefault do?
   - Hard reset: wipe DB + configs + reboot
   - Soft reset: restore factory configs only

4. **Snapshot mechanism:** How to serve JPEG snapshots?
   - Option A: nginx serves snapshots from a periodically-refreshed JPEG file
   - Option B: Direct HTTP handler in the ONVIF server grabs a frame from the encoder
   - Option C: RTSP server exposes a snapshot endpoint

5. **Branding:** Keep "Honeywell" identity or genericize?

---

## Testing Strategy

### Unit Tests (per operation)
- Each new operation should have a corresponding test using the existing gSOAP client
  library or the `onvif_tester.py` script in `scripts/onvif_test/`

### Integration Tests
- Use ONVIF Device Test Tool (ODTT) v23.06 or newer
- Profile S test configuration: select all mandatory + conditional features that match device capabilities
- Target: 100% pass rate on mandatory tests, 95%+ on conditional

### Regression Tests
- Run existing conformance tests after each phase
- Ensure previously-passing operations still pass after events service changes

---

## Appendix A — Profile S Conformance Test Procedure (from spec)

The 26-step ONVIF Profile S certification procedure tests:
1. Discovery (Hello, Probe, Resolve)
2. GetServices + GetCapabilities
3. GetDeviceInformation
4. GetSystemDateAndTime / SetSystemDateAndTime
5. SystemReboot
6. SetSystemFactoryDefault
7. User management (full CRUD cycle)
8. GetScopes / SetScopes / AddScopes / RemoveScopes
9. Network configuration (get/set for hostname, DNS, NTP, gateway, interfaces, protocols)
10. CreatePullPointSubscription → PullMessages → Renew → Unsubscribe
11. GetEventProperties (verify topic tree)
12. GetProfiles / GetProfile
13. CreateProfile → Add configs → Remove configs → DeleteProfile
14. Video source/encoder full config cycle
15. Metadata configuration cycle
16. Audio source/encoder config cycle (if audio advertised)
17. GetStreamUri → RTSP playback (UDP, TCP, HTTP tunnel)
18. GetSnapshotUri → HTTP GET snapshot
19. Multicast start/stop (if multicast advertised)
20. OSD operations
21. Imaging operations
22. WS-UsernameToken authentication
23. HTTP Digest authentication
24. Access policy verification (user levels)
25. HTTPS connectivity (if advertised)
26. Dynamic DNS / Zero Config / IP Filtering (if advertised)

---

## Appendix B — soapServer.c Dispatch Modifications Needed

Lines that need to be **uncommented** in `gsoap/gsoap_createfile/soapServer.c`:

### Events (Phase 0):
```
__tev__PullMessages
__tev__Renew (or __tev__Renew_)
__tev__Unsubscribe (or __tev__Unsubscribe_)
__tev__SetSynchronizationPoint
__tev__GetServiceCapabilities
```

### Audio (Phase 2, if implementing):
```
__trt__GetAudioSources
__trt__GetAudioOutputs
__trt__GetAudioSourceConfiguration
__trt__SetAudioSourceConfiguration
__trt__GetAudioEncoderConfiguration
__trt__GetAudioEncoderConfigurations
__trt__SetAudioEncoderConfiguration
__trt__GetCompatibleAudioSourceConfigurations
__trt__GetCompatibleAudioEncoderConfigurations
__trt__GetAudioSourceConfigurationOptions
__trt__AddAudioSourceConfiguration
__trt__AddAudioEncoderConfiguration
__trt__RemoveAudioSourceConfiguration
__trt__RemoveAudioEncoderConfiguration
```

### Device (Phase 0):
```
__tds__SetSystemFactoryDefault
```

### Multicast (Phase 2, if implementing):
```
__trt__StartMulticastStreaming
__trt__StopMulticastStreaming
```
