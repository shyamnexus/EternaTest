# ETERNA IP Camera Firmware Release Notes

## Version 1.1.0
**Release Date:** January 6, 2026

---

## Highlights

This release focuses on **streaming stability**, **audio support**, **IR LED control**, and **recording/playback functionality**. Major improvements include:

- ✅ Custom Live555 RTSP server with sub-second latency
- ✅ Audio streaming support (G.711, G.726, AAC-LC)
- ✅ SD card recording with AES-256 encryption
- ✅ Playback API with timeline support
- ✅ Custom mDNS responder (replaces Avahi)
- ✅ ONVIF privacy mask support
- ✅ Advanced ISP features (DPC, shading, WDR)

---

## Quick Start

### RTSP Streaming (VLC)
```
rtsp://<IP_ADDRESS>:554/stream0   (Main stream - Video + Audio)
rtsp://<IP_ADDRESS>:554/stream1   (Sub stream - Video + Audio)
rtsp://<IP_ADDRESS>:554/stream2   (Third stream - Video + Audio)
```

### Web Interface
- **URL:** `https://<IP_ADDRESS>`
- **Default Credentials:**
  - Username: `admin`
  - Password: `Admin@123`
- **First Boot:** Password reset is mandatory

---

## New Features

### Audio Streaming
| Feature | Status | Notes |
|---------|--------|-------|
| Audio in RTSP streams | Supported | Stream1 only |
| G.711 μ-law codec | Supported | Default codec |
| G.711 A-law codec | Supported | |
| G.726 ADPCM codec | Supported | 16/24/32/40 kbps |
| AAC-LC codec | Supported | High quality, ~4.5% CPU |
| Audio in Web UI | Partial | Mute/unmute pending |
| Two-way audio | Not implemented | Planned for future |

### IR LED Control & Day/Night (does not work, WIP)
| Feature | Status | Notes |
|---------|--------|-------|
| IR LED PWM control | Supported | P_GPIO11, 0-100% brightness |
| IR cut filter switching | Supported | Day/Night GPIO control |
| Auto day/night mode | Supported | ISP-based light detection |
| SW CDS mode | Supported | Software color detection sensor |
| Scheduled day/night | Supported | Time-based switching |
| B/W mode sync | Supported | Auto-switches sensor to B/W |

### Recording & Playback
| Feature | Status | Notes |
|---------|--------|-------|
| Manual recording | Supported | Start/stop via Web UI |
| Continuous recording | Supported | Auto-start option |
| AES-256-CTR encryption | Supported | Optional file encryption |
| SD card hot-plug | Supported | Auto-detect mount/unmount |
| Segment rotation | Supported | Configurable duration (default 60s) |
| FIFO storage management | Supported | Auto-delete oldest files |
| Playback timeline UI | Supported | Browse recordings by date |
| Recording download | Supported | Download encrypted/decrypted |
| Recording deletion | Supported | Single or bulk delete |
| Pre/post record buffer | Not ready | Stubs implemented |

### RTSP Server Improvements
| Feature | Status | Notes |
|---------|--------|-------|
| Custom Live555 server | Supported | Sub-second latency |
| Localhost auth bypass | Supported | For go2rtc integration |
| Frame flush on connect | Supported | Prevents stale frames |
| IDR frame request | Supported | Immediate keyframe on connect |
| Latency correction | Supported | Auto-drops old frames if >500ms |
| Codec change detection | Supported | Handles H.264 ↔ H.265 switch |

### Advanced ISP Features
| Feature | Status | Notes |
|---------|--------|-------|
| Dead Pixel Correction (DPC) | Supported | Configurable strength |
| Lens Shading Correction | Supported | Configurable strength |
| WDR enhancement | Supported | Wide dynamic range |
| Dark enhancement ratio | Supported | Boost dark areas |
| Contrast enhancement | Supported | Auto tone mapping |
| 2D/3D Noise Reduction | Supported | Auto or manual levels |
| Defog mode | Supported | Haze removal |

### Network Improvements
| Feature | Status | Notes |
|---------|--------|-------|
| Custom mDNS responder | Supported | No Avahi dependency |
| DNS-SD service announcement | Supported | _http, _rtsp, _onvif |
| Hostname from MAC | Supported | Auto-generated |
| SSL certificate with hostname | Supported | Self-signed with proper CN |
| 32MB flash partition | Supported | Increased from 16MB |
| MTU/ARP optimizations | Supported | Better network performance |

### ONVIF Improvements
| Feature | Status | Notes |
|---------|--------|-------|
| Privacy mask support | Supported | Add/edit/delete masks |
| Imaging controls | Supported | Brightness, contrast, etc. |
| Media2 profile | Partial | Basic implementation |
| WS-Discovery | Supported | Camera auto-discovery |

### Analytics (Stubs)
| Feature | Status | Notes |
|---------|--------|-------|
| Motion detection | Partial | Basic implementation, no CNN |
| Person/Vehicle detection | Stub | Requires libpvdcnn_lib |
| Face detection | Stub | Requires libfdcnn_lib |
| Object tracking | Stub | Framework ready |
| VQA (Video Quality Analysis) | Stub | Requires libvqa_lib |
| Privacy mosaic | Stub | Requires libvendor_gfx |
| AI ISP | Stub | Framework ready |

---

## Improvements from v1.0.0

### Live View
| Change | Status |
|--------|--------|
| Audio playback support | Now functional (was disabled) |
| Stream latency reduced | ~200-500ms (was 2-3s) |
| VLC timer jump fix | Fixed timestamp handling |
| WebRTC stability | Improved via go2rtc |

### Playback
| Change | Status |
|--------|--------|
| SD card recording | Now functional (was UI only) |
| Timeline UI | Now loads real recordings |
| Recording list API | New `/api/v1/playback/*` endpoints |
| Download recordings | Now functional |

### Configuration
| Change | Status |
|--------|--------|
| IR Mode & Sensitivity | Now functional (was not working) |
| Day/Night auto switch | Now functional |
| Advanced ISP settings | Now functional |

### System
| Change | Status |
|--------|--------|
| Flash partition size | 32MB (was 16MB) |
| Avahi removed | Replaced with custom mDNS |
| Web artifacts cleaned | Removed legacy CGI files |

---

## API Changes

### New Endpoints

#### Recording Control
```
POST   /api/v1/recording/start         - Start manual recording
POST   /api/v1/recording/stop          - Stop recording
GET    /api/v1/recording/status        - Get recording status
```

#### Playback
```
GET    /api/v1/playback/recordings     - List recordings with filters
GET    /api/v1/playback/recordings/{id} - Get recording metadata
GET    /api/v1/playback/timeline       - Get timeline data for date range
GET    /api/v1/playback/download/{id}  - Download recording file
DELETE /api/v1/playback/recordings/{id} - Delete recording
```

#### IR Control
```
GET    /api/v1/ir/status               - Get IR LED and cut filter status
PUT    /api/v1/ir/settings             - Configure IR settings
POST   /api/v1/ir/led/on               - Turn IR LED on
POST   /api/v1/ir/led/off              - Turn IR LED off
POST   /api/v1/ir/cut/day              - Switch to day mode
POST   /api/v1/ir/cut/night            - Switch to night mode
```

#### Analytics
```
GET    /api/v1/analytics/status        - Get analytics status
PUT    /api/v1/analytics/settings      - Configure analytics
GET    /api/v1/analytics/motion/zones  - Get motion detection zones
PUT    /api/v1/analytics/motion/zones  - Set motion detection zones
```

#### RTSP
```
GET    /api/v1/rtsp/status             - Get RTSP server status
PUT    /api/v1/rtsp/settings           - Configure RTSP settings
```

---

## Configuration Changes

### New Config Files

| File | Purpose |
|------|---------|
| `ir.json` | IR LED, IR cut filter, day/night settings |
| `analytics.json` | Motion detection, object detection, tracking |

### Modified Config Files

| File | Changes |
|------|---------|
| `isp.json` | Added DPC, shading, enhancement ratios |
| `storage.json` | Added encryption, channel config, database paths |
| `media.json` | Added audio codec settings |
| `streaming.json` | Added RTSP auth, localhost bypass |

---

## Known Limitations

1. **RTSP & Web UI Conflict:** VLC stream may stutter if web UI stream is active
2. **Two-way Audio:** Not implemented (planned for v1.2.0)
3. **Analytics CNN Models:** Require vendor libraries not included
4. **Pre/Post Record:** Stubs only, not functional
5. **Volume/Mute Controls:** Partial functionality
6. **Logout Button:** Not functional
7. **SSL Certificate:** Shows placeholder validity dates
8. **Rotation:** Not working in orientation settings
9. **Event Recording:** Motion-triggered recording not functional

---

## Build Dependencies

### New Libraries
| Library | Version | Purpose |
|---------|---------|---------|
| vo-aacenc | 0.1.3 | AAC audio encoding |
| Live555 | Custom | RTSP server (patched for HDAL) |
| SQLCipher | 4.5+ | Encrypted database |

### Removed Dependencies
| Library | Reason |
|---------|--------|
| Avahi | Replaced with custom mDNS |
| entry.cgi | Legacy CGI removed |

---

## Test Cases

> **📋 Note for QA:** Please refer ONLY to the test cases in this section for testing. Items marked with "Do NOT test" or "WIP" should be skipped. Known limitations are documented in the rightmost column where applicable.

### First Boot Setup

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Power on device for first boot | Device should start with static IP 192.168.0.2 |
| 2 | Configure laptop to 192.168.0.x subnet | Should be able to ping camera |
| 3 | Access https://192.168.0.2 | Web UI login page should appear |
| 4 | Login with default credentials (admin/Admin@123) | Should prompt for password reset |
| 5 | Reset password | Should login successfully with new password |

### Live View Page

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Navigate to live view page | Video stream should display |
| 2 | Check video source (Live555 RTSP) | Stream should play with sub-second latency |
| 3 | Test audio controls (main stream only) | Audio should mute/unmute |
| 4 | Test image adjustment controls | Brightness/contrast should change in real-time |
| 5 | Switch between streams (stream0/stream1/stream2) | Each stream should load correctly unless laptop does not support h265 |

### Video Encoder Settings

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Change width/height in web UI | Settings saved | Does NOT take effect (WIP) |
| 2 | Change FPS in web UI | Settings saved | Does NOT take effect (WIP) |
| 3 | Change codec (H.264/H.265) | Codec should change on stream |  |
| 4 | Change profile level | Profile should update |  |
| 5 | Set rate control to CBR | Bitrate should be constant |  |
| 6 | Set rate control to VBR | Bitrate should vary |  |
| 7 | Set rate control to AVBR | Adaptive bitrate should work |  |
| 8 | Set rate control to Fix QP | Fixed quality should apply |  |
| 9 | Test on main stream | Settings should apply |  |
| 10 | Test on sub stream | Settings should apply |  |
| 11 | Test on third stream | Settings should apply |  |

### Network Basic Settings

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Check MAC address display | MAC address should show correctly |
| 2 | Check hostname format | Should be hon-XXX (last 3 digits of MAC) |
| 3 | Switch IPv4 from DHCP to static | IP should change to static config |
| 4 | Switch IPv4 from static to DHCP | IP should be obtained via DHCP |
| 5 | Configure DNS settings | DNS should resolve correctly |
| 6 | Check IPv6 address | Should show 2001:db8::1 (default) |
| 7 | Check IPv6 gateway | Should show fe80::1 (default) |
| 8 | Check SSL certificate info | Should show data from certificate file |
| 9 | Configure multicast settings | Multicast should work |
| 10 | Configure MTU | MTU should apply |
| 11 | Configure gratuitous ARP | ARP should work |

### Network SMTP (Email)

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Configure SMTP server settings | Settings should save |
| 2 | Send test email | Email should be received (on network without SMTP block) |
| 3 | Verify saved configuration | Settings should persist after reload |

### Network SNMP

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Enable SNMP | SNMP daemon should start |
| 2 | Configure community string | Settings should save |
| 3 | Test with snmpwalk | Should return device info |
| 4 | Test with Paessler SNMP Tester | Should respond correctly |

### Network UPnP

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Connect camera to UPnP-enabled router | Camera should be detected |
| 2 | Enable UPnP on camera | Settings should save |
| 3 | Check IGD port mapping on router | Port mapping should appear |

### Network HTTPS

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Check certificate info display | Should show correct certificate details |  |
| 2 | Upload custom certificate | Certificate should be applied | Works |
| 3 | Generate self-signed certificate | New certificate should be created | Works |
| 4 | Generate new certificate (button) | | Do NOT test (WIP) |

### Network RTSP Settings

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Check RTSP server status | Should show running |  |
| 2 | Change RTSP port | Port should update | Avoid 80, 443, 3702 |
| 3 | Test HTTP tunnel port | Tunneling should work |  |
| 4 | Set max connections to 10 | Should limit connections |  |
| 5 | Enable basic authentication | Should require password |  |
| 6 | Restart RTSP server | | Do NOT test (WIP) |

### Image Adjustment

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Adjust brightness | Image brightness should change |
| 2 | Adjust contrast | Image contrast should change |
| 3 | Adjust saturation | Color saturation should change |
| 4 | Adjust hue | Color hue should shift |
| 5 | Adjust sharpness | Edge sharpness should change |
| 6 | Adjust gamma | Gamma curve should change |

### White Balance

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Set to Auto | White balance should auto-adjust |  |
| 2 | Set to Incandescent | Warm color correction applied |  |
| 3 | Set to Fluorescent | Cool color correction applied |  |
| 4 | Set to Daylight | Daylight color correction applied |  |
| 5 | Set to Cloudy | Cloudy color correction applied |  |
| 6 | Set to Manual (R:128, B:128) | Custom gains applied |  |
| 7 | Reset to default | | Required to restore original colors (WIP) |

### Exposure Control

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Set to Auto | Exposure should auto-adjust |  |
| 2 | Set to Center Weighted | Center-focused metering |  |
| 3 | Set to Spot | Spot metering active |  |
| 4 | Reset to default | | Required to restore original brightness (WIP) |

### Backlight & Dynamic Range

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Enable backlight compensation | Should improve backlit subjects | Effect not visible (WIP) |
| 2 | Disable backlight compensation | Should disable compensation |  |
| 3 | Enable WDR | High contrast scenes show more detail | Default ON, UI status incorrect (WIP) |
| 4 | Disable WDR | WDR effect should be removed |  |
| 5 | Enable highlight compensation | Should reduce blown highlights | Effect not visible (WIP) |
| 6 | Disable highlight compensation | Should disable compensation |  |

### Image Orientation

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Set to Normal | Image should display normally |  |
| 2 | Set to Flip | Image should flip vertically |  |
| 3 | Set to Mirror | Image should mirror horizontally |  |
| 4 | Set to Flip and Mirror | Image should flip and mirror |  |
| 5 | Set rotation angle | | Does NOT work (WIP) |

### Anti-Flicker

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Set to 50Hz | Should eliminate flicker under 50Hz lighting |
| 2 | Set to 60Hz | Should eliminate flicker under 60Hz lighting |
| 3 | Set to Auto | Should auto-detect and eliminate flicker |

### Privacy Mask

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Add privacy mask on stream0 | Mask should appear on main stream |
| 2 | Add privacy mask on stream1 | Mask should appear on sub stream |
| 3 | Add privacy mask on stream2 | Mask should appear on third stream |
| 4 | Add multiple masks | All masks should display |
| 5 | Edit mask position/size | Mask should update |
| 6 | Delete mask | Mask should be removed |

### OSD (On-Screen Display)

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Enable OSD timestamp | Timestamp should appear on stream |  |
| 2 | Enable OSD camera name | Name should appear on stream |  |
| 3 | Position OSD elements | Elements should move to position | Does NOT work below first quarter of image (WIP) |
| 4 | Customize OSD text | Custom text should display |  |

### Audio Streaming

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Open VLC with rtsp://IP/stream0 | Video and audio should play |
| 2 | Check audio codec in VLC codec info | Should show G.711/AAC |
| 3 | Test mute button in web UI | Audio should mute |
| 4 | Switch audio codec in settings | Stream should continue with new codec |

### IR Control

> ⚠️ **Do NOT test** - IR Control is still WIP and not ready for testing.

### AI Analytics

> ⚠️ **Do NOT test** - AI Analytics is still WIP and requires vendor libraries not included in this release.

### System Settings

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Access ONVIF users page | Page should load | Do NOT test (WIP) |
| 2 | Access User management | Page should load | API NOT integrated (WIP) |
| 3 | Access Maintenance page | Page should load | API NOT integrated (WIP) |

### Recording

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Insert SD card | Should detect and mount |  |
| 2 | Start recording via Web UI | | NOT integrated yet, use API |
| 3 | Start recording via API (POST /api/v1/recording/start) | Recording should start |  |
| 4 | Stop recording via Web UI | | NOT integrated yet, use API |
| 5 | Stop recording via API (POST /api/v1/recording/stop) | Recording should stop |  |
| 6 | Configure file duration | Setting should apply | Works |
| 7 | Configure pre-recording buffer | Setting should apply | Works |
| 8 | Configure post-recording buffer | Setting should apply | Works |
| 9 | Configure max files per channel | Setting should apply | Works |
| 10 | Enable encryption | New recordings encrypted | Works |
| 11 | Enable overwrite oldest | | NOT verified |
| 12 | Enable/disable channel | Channel should start/stop recording | Works |
| 13 | Check Storage tab | Should show recordings on SD card |  |
| 14 | Open playback page | Recording should appear in timeline |  |
| 15 | Click recording to download | MP4 file should download |  |
| 16 | Delete recording | File should be removed |  |
| 17 | Configure NAS settings | | Do NOT test (WIP) |
| 18 | Configure FTP settings | | Do NOT test (WIP) |
| 19 | Configure schedule recording | | Do NOT test (WIP) |

### Playback

| # | Steps | Expected Result | Known Limitations |
|---|-------|-----------------|-------------------|
| 1 | Navigate to playback page | Should load without errors |  |
| 2 | Check default recording path | Should fetch from DCIM/CH01, CH02, CH03 |  |
| 3 | Use channel filter | Should filter by channel | Works |
| 4 | Use type filter | Should filter by type | Does NOT work (WIP) |
| 5 | Use calendar to select date | Should search recordings for that date | Supports dates to epoch |
| 6 | Select date with recordings | Timeline should show recordings | Timeline not fully accurate (WIP) |
| 7 | Play MP4 recording | Should start playback | Works |
| 8 | Download MP4 recording | MP4 file should download | Works |
| 9 | Play encrypted recording | | NOT supported (WIP) |
| 10 | Download encrypted recording | | NOT supported (WIP) |
| 11 | Check audio in playback | | No audio in playback (WIP) |

### RTSP Server

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Open VLC with stream0 | Should connect in <1 second |
| 2 | Watch for 5 minutes | No latency buildup |
| 3 | Disconnect and reconnect | Should resume immediately |
| 4 | Test with FFmpeg | Should record without errors |

### Advanced ISP Features

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Enable Dead Pixel Correction | Image artifacts from dead pixels should be removed |
| 2 | Adjust DPC strength 0-100 | Correction intensity should change |
| 3 | Enable Lens Shading Correction | Corner darkening (vignetting) should be reduced |
| 4 | Adjust shading strength | Vignetting correction should vary |
| 5 | Enable WDR mode | High contrast scenes should show more detail |
| 6 | Adjust dark enhancement ratio | Dark areas should become more visible |
| 7 | Enable 2D noise reduction | Image noise should be reduced |
| 8 | Enable 3D noise reduction | Temporal noise should be reduced |
| 9 | Test defog mode | Hazy images should appear clearer |

### Day/Night Auto Switch

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Set mode to "Auto" | Camera should detect ambient light |
| 2 | Cover lens completely | Should switch to night mode within delay period |
| 3 | Uncover lens | Should switch back to day mode |
| 4 | Check IR LED turns on in night mode | IR LED should illuminate |
| 5 | Check B/W mode in night | Image should switch to grayscale |
| 6 | Set mode to "Schedule" | Camera should switch at configured times |
| 7 | Set mode to "SW CDS" | Software light detection should work |
| 8 | Adjust sensitivity | Switching threshold should change |

### Storage & SD Card

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Insert SD card | Should auto-detect and mount at /mnt/sd |
| 2 | Check storage page | SD card info should display (size, free space) |
| 3 | Remove SD card while idle | Should detect removal, show "not inserted" |
| 4 | Remove SD card while recording | Recording should stop gracefully |
| 5 | Re-insert SD card | Should resume normal operation |
| 6 | Fill SD card to 90% | FIFO cleanup should trigger (if enabled) |
| 7 | Check oldest files deleted | Oldest recordings should be removed |

### Encryption (Recording)

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Enable recording encryption | New recordings should be encrypted |
| 2 | Download encrypted recording | File should be decrypted on download |
| 3 | Copy encrypted file directly from SD | File should not be playable without key |
| 4 | Disable encryption | New recordings should be plain MP4 |

### ONVIF Compatibility

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Add camera in ONVIF Device Manager | Camera should be discovered |
| 2 | Get device information | Serial, firmware version should display |
| 3 | Get stream URI | RTSP URL should be returned |
| 4 | Test live view in NVR | Video should play |
| 5 | Add privacy mask via ONVIF | Mask should appear on stream |
| 6 | Adjust imaging settings via ONVIF | Brightness/contrast should change |

### Web UI Recording Controls

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Click record button on live view | Recording should start, icon should indicate |
| 2 | Check recording status API | Should return "recording" status |
| 3 | Click record button again | Recording should stop |
| 4 | Navigate to Storage > Recording page | Schedule UI should load |
| 5 | Enable continuous recording | Recording should auto-start |
| 6 | Reboot camera with continuous enabled | Recording should resume after boot |

### Audio Codec Switching

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Set audio codec to G.711 μ-law | Stream should use PCMU |
| 2 | Set audio codec to G.711 A-law | Stream should use PCMA |
| 3 | Set audio codec to AAC-LC | Stream should use MPEG4-GENERIC |
| 4 | Set audio codec to G.726-32 | Stream should use G726-32 |
| 5 | Disable audio | RTSP should be video-only |
| 6 | Check CPU usage with AAC | Should be ~4-5% for AAC encoding |

### API Endpoint Testing

| # | Endpoint | Method | Expected Result |
|---|----------|--------|-----------------|
| 1 | `/api/v1/recording/status` | GET | Returns current recording state |
| 2 | `/api/v1/recording/start` | POST | Starts recording, returns 200 |
| 3 | `/api/v1/recording/stop` | POST | Stops recording, returns 200 |
| 4 | `/api/v1/playback/recordings` | GET | Returns list of recordings |
| 5 | `/api/v1/playback/recordings?date=2026-01-06` | GET | Filters by date |
| 6 | `/api/v1/playback/timeline?start=...&end=...` | GET | Returns timeline data |
| 7 | `/api/v1/playback/download/{id}` | GET | Downloads MP4 file |
| 8 | `/api/v1/ir/status` | GET | Returns IR LED/cut filter state |
| 9 | `/api/v1/ir/led/on` | POST | Turns IR LED on |
| 10 | `/api/v1/ir/led/off` | POST | Turns IR LED off |
| 11 | `/api/v1/ir/cut/night` | POST | Switches to night mode |
| 12 | `/api/v1/ir/cut/day` | POST | Switches to day mode |
| 13 | `/api/v1/rtsp/status` | GET | Returns RTSP server stats |
| 14 | `/api/v1/analytics/status` | GET | Returns analytics engine status |
| 15 | `/api/v1/analytics/motion/zones` | GET | Returns motion zones config |

### Stress Testing

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Open 3 VLC streams simultaneously | All should play without crashes |
| 2 | Record while streaming to VLC | Both should work |
| 3 | Run for 24 hours continuous | No memory leaks, stable operation |
| 4 | Rapid connect/disconnect (10x) | Server should handle gracefully |
| 5 | Change resolution while streaming | Stream should recover after codec change |
| 6 | Switch H.264 ↔ H.265 while streaming | VLC may need reconnect, no crash |

### Error Handling

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Try recording without SD card | Should return appropriate error |
| 2 | Try invalid API parameters | Should return 400 with error message |
| 3 | Access protected API without auth | Should return 401 |
| 4 | Request non-existent recording | Should return 404 |
| 5 | Fill SD card completely | Should stop recording gracefully |
| 6 | Kill ipcamd process | Should restart via watchdog |

### Browser Compatibility

| # | Browser | Feature | Expected Result |
|---|---------|---------|-----------------|
| 1 | Chrome | WebRTC (H.264) | Video + audio should play |
| 2 | Chrome | MSE (H.265) | Video should play |
| 3 | Firefox | WebRTC (H.264) | Video + audio should play |
| 4 | Firefox | MSE (H.265) | Video should play |
| 5 | Edge | WebRTC (H.264) | Video + audio should play |
| 6 | Safari | WebRTC (H.264) | Video + audio should play |
| 7 | Mobile Chrome | WebRTC | Video should play on mobile |

---

## Upgrade Notes

### From v1.0.0

1. **Flash partition update required** - Factory reset recommended
2. **New config files** - `ir.json` and `analytics.json` will be created
3. **SSL certificates** - Will be regenerated with new hostname
4. **Database migration** - Recording database will be created fresh

### Backup Recommended

Before upgrading, backup:
- User credentials (`/mnt/app/ipcamera/db/usermng.db`)
- Custom SSL certificates
- OSD and privacy mask settings

---

## Coming Soon (v1.2.0)

- Motion-triggered recording
- ONVIF event subscription
- Full analytics with CNN models


---

## Commits Since v1.0.0

| Commit | Description |
|--------|-------------|
| 64434719b | Update gitignore with bins not needed |
| 312d29a0e | Add web UI APIs for recording and playback |
| 1d81244b1 | Add motion detection samples and ISP features |
| 803ff97d9 | Add advanced ISP features |
| 0321f357f | Web UI submodule update |
| 36f6f1f46 | Add audio support to RTSP and web UI |
| 739f2588e | Add IR LED feature with auto switch |
| 8cc36a7b8 | Add APIs and localhost bypass for RTSP |
| 0624ad463 | Custom Live555 RTSP server with sub-second latency |
| 309716d5f | Build fix |
| 7251225c9 | Update web UI submodule reference |
| 059459817 | Remove legacy web artifacts |
| dc77ea6f3 | Add MTU, ARP and 32MB partition |
| 6519a0d66 | Add start/stop daemon to adb push |
| b8bb4663c | Custom mDNS, SSL manager rework |
| ccbbca031 | Add analytics function stubs |

---

**© 2026 ETERNA IP Camera Project**
