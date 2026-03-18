# ONVIF Firmware Improvement Plan

Based on the execution of the ONVIF Compliance Test (`onvif_tester.py`), the following critical issues and missing features have been identified.

## 1. Critical: Authentication & Security
**Status:** 🔴 FAILED
**Symptom:** Most service calls fail with `The security token could not be authenticated or authorized`. WS-Discovery Probe also yields **no response**.
**Root Cause Hypothesis:**
- **WS-Discovery Hello Disabled:** `soap_wsdd_Hello` calls are commented out in `onvif_main.c`.
- **WS-Discovery Probe Unresponsive:** Implementation exists in `wsDiscovery.c` and port 3702 is open, but device does not reply to Probes.
- **Time Synchronization:** ONVIF authentication (WS-Security / UsernameToken) requires the client and server clocks to be synchronized. The test revealed the camera is in **1970** while the system is in **2026**. This causes strict timestamp validation to fail.
- **Hardcoded Credentials Compatibility:** The firmware currently uses a hardcoded "admin" user.

**Action Plan:**
- [ ] **Verify Time Sync:** Ensure the firmware has a working NTP client or Real-Time Clock (RTC) driver enabled.
- [ ] **Debug `ws_authentication`:** Add logging to `onvif_main.c` to see *why* the token is rejected (Timestamp expiry? Nonce reuse? Password mismatch?).
- [ ] **Implement `CreateUser`:** Move away from hardcoded credentials to a dynamic user database so clients can create/update users as per the standard.

## 2. High: Service Discovery & Endpoints (XAddrs)
**Status:** 🟠 PARTIAL / BROKEN
**Symptom:** 
- `GetCapabilities` passes and lists Media, PTZ, Events, Imaging.
- BUT `create_ptz_service` and `create_events_service` fail with "Device doesn't support service".
**Root Cause:**
- This typically happens when the **XAddr** (Service URL) returned in the Capabilities response is missing, malformed, or points to an unreachable IP (e.g., `0.0.0.0` or `localhost`).
- The firmware might be populating the "Available" flags but NOT populating the corresponding Service URLs.
- **Build System Mismatch:** The `Makefile` on disk had the full ONVIF stack (Device, Media, etc.) **commented out**, yet the camera obviously has `Device` service running. This indicates the deployed binary does not match the local build configuration.
- **WS-Discovery Issues:** `Hello` messages are commented out in source. Probe logic is present but silent on network.

**Action Plan:**
- [ ] **Fix XAddr Generation:** In `onvif_device.c` (or equivalent), ensure that `GetCapabilities` and `GetServices` responses populate the full URL (e.g., `http://<IP>:80/onvif/ptz_service`) for *all* supported services.
- [ ] **Dynamic IP Binding:** Ensure the formatted XAddrs use the actual interface IP, not a placeholder.

## 3. Medium: Media Service Functionality
**Status:** 🔴 BLOCKED (by Auth)
**Symptom:** `GetProfiles`, `GetStreamUri` fail due to auth.
**Action Plan:**
- [ ] **Verify Profile Injection:** Once Auth is fixed, verify that `GetProfiles` returns at least one valid profile with:
    - VideoSourceConfiguration
    - VideoEncoderConfiguration
    - RTSP Stream URI
- [ ] **RTSP URI Correctness:** Ensure `GetStreamUri` returns a valid, playable RTSP URL (e.g., `rtsp://<IP>:554/live/stream0`).

## 4. Medium: Missing Services Implementation
**Status:** ❓ UNKNOWN (Blocked)
**Symptom:** Services are advertised but usage fails.
**Action Plan:**
- [ ] **PTZ Implementation:** Verify `ptz_service.c` exists and handles basic moves (`ContinuousMove`, `Stop`, `GetStatus`).
- [ ] **Events Implementation:** Verify `events_service.c` handles `PullPointSubscription` or basic processing. If not ready, **disable** the service advertisement in Capabilities to avoid client errors.

## Summary of Next Steps
1. **Fix Time/Auth:** This is the blocker for all other tests.
2. **Correct Capabilities:** Fix the XAddr propagation so clients can find the PTZ/Event services.
3. **Re-run Test:** Validate pass rate increases above 50%.
