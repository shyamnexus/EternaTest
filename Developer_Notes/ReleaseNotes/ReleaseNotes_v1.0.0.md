# ETERNA IP Camera Firmware Release Notes

## Version 1.0.0
**Release Date:** December 24, 2025

---

## Quick Start

### RTSP Streaming (VLC)
```
rtsp://<IP_ADDRESS>:554/stream0   (Main stream)
rtsp://<IP_ADDRESS>:554/stream1   (Sub stream)
rtsp://<IP_ADDRESS>:554/stream2   (Third stream)
```

### Web Interface
- **URL:** `https://<IP_ADDRESS>`
- **Default Credentials:**
  - Username: `admin`
  - Password: `Admin@123`
- **First Boot:** Password reset is mandatory

---

## Features

### Live View
| Feature | Status | Notes |
|---------|--------|-------|
| Multi-stream viewing (3 streams) | Supported | WebRTC (H.264) / MSE (H.265) |
| Snapshot capture | Supported | Downloads JPEG |
| Image adjustments (brightness, contrast, saturation, sharpness) | Supported | Real-time preview |
| Quick actions (reset, snapshot, fullscreen) | Supported | |
| Video fit modes | Supported | |
| Audio playback | Disabled | Planned for future release |
| Volume/mute controls | Not functional | |
| Image enhancement | Not implemented | |

**Browser Support:** Chrome, Firefox, Edge

### Playback
| Feature | Status | Notes |
|---------|--------|-------|
| Page loading | Supported | |
| SD card detection | Supported | Shows "not inserted" message |
| Timeline UI | Supported | Sample UI visible |
| SD card recording/playback | In Progress | |

### Configuration

#### Video Encoder
| Feature | Status | Notes |
|---------|--------|-------|
| Resolution (1080p, 2K, 480p, etc.) | Supported | Standard resolutions only(Do not modify random Resolutions) |
| Bitrate type (CBR/VBR) | Supported | |
| Bitrate value (512Kbps - 16Mbps) | Supported | |
| FPS adjustment | Keep at 30 | Do not modify |
| Advanced settings | Dev only | Will be removed |
| Snapshot settings | Not ready | |

#### Audio
| Feature | Status | Notes |
|---------|--------|-------|
| Page loading | Supported | UI only, no functionality |

#### Network
| Tab | Feature | Status |
|-----|---------|--------|
| **Basic** | Hostname (MAC-based) | Supported |
| | DHCP/Static IPv4 | Supported |
| | DHCP/Static IPv6 | Supported |
| | DNS configuration | Supported |
| | Service ports | Not functional |
| | Advanced settings | UI only |
| **SMTP** | Email configuration | Supported |
| | Test email | Supported |
| **SNMP** | v1/v2c/v3 | Supported |
| **UPnP** | Device discovery | Supported |
| | Port mapping | UI only |
| **HTTPS** | Self-signed certificate | Supported |
| | Custom certificate | Not ready |

#### Image Adjustment
| Feature | Status | Notes |
|---------|--------|-------|
| Brightness/Contrast/Saturation/Sharpness/Gamma | Supported | |
| White balance | Supported | |
| Exposure | Supported | |
| AGC | Supported | |
| Day/Night mode | Partial | IR mode & sensitivity not working |
| Orientation (flip/mirror) | Supported | Rotation not working |
| Anti-flicker | Supported | |
| Backlight compensation | Not ready | |
| Dynamic range (WDR) | Not ready | |
| Image enhancement | Not implemented | |
| Day/Night page | Won't open | |

#### Storage
| Feature | Status | Notes |
|---------|--------|-------|
| SD card info display | Supported | |
| SD card detection | Supported | |
| Scheduled recording UI | Supported | UI only |
| Format SD card | Not ready | |
| Storage management | Not ready | |
| Event recording | Not ready | |

#### System
| Feature | Status | Notes |
|---------|--------|-------|
| Page loading | Supported | |
| User management (view admin) | Supported | |
| Logout button | Not functional | |
| Other settings | Not ready | |

#### OSD (On-Screen Display)
| Feature | Status | Notes |
|---------|--------|-------|
| Save/load settings | Supported | All 3 streams |
| Display on live view | Supported | |
| Position adjustment | Supported | |
| Custom text & logo | Supported | |

**Known Limitation:** OSD will not work below the top quarter of the viewport

#### Privacy Mask
| Feature | Status | Notes |
|---------|--------|-------|
| Add/Edit/Delete masks | Supported | All 3 streams |
| Display on live view | Supported | |

#### Analytics
| Feature | Status | Notes |
|---------|--------|-------|
| All features | Not implemented | Planned for future release |

---

## Known Limitations

1. **RTSP & Web UI Conflict:** VLC stream will not work if web UI stream is open elsewhere
2. **Audio:** Disabled in this release
3. **Volume/Mute:** Controls not functional
4. **Logout Button:** Not functional in this release
5. **OSD Positioning:** Cannot place below top 25% of viewport
6. **SSL Certificate:** Shows validity 1970-1971 (placeholder)
7. **Session Management:** Intentionally relaxed for developer testing
8. **Day/Night Page:** Won't open
9. **IR Mode & Sensitivity:** Not working in Day/Night settings
10. **Rotation:** Not working in orientation settings
11. **Service Ports:** Configuration doesn't actually change ports

---

## Test Cases

### Authentication

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Try to access other pages without login | Should redirect to login page |
| 2 | Try to login with wrong credentials | Should show invalid credentials message |
| 3 | Login with default credentials (admin/Admin@123) | Should login successfully |
| 4 | Reset password on first boot | Password reset should complete successfully |

**Note:** Web UI might allow other pages after user has logged in on first boot. Intentionally disabled for ease of testing for developers.

### Live View

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Check if live view page loads after login | Live view page should load successfully with video stream |
| 2 | Check if all 3 streams are working | All 3 streams should load successfully |
| 3 | Click snapshot button | Snapshot should be downloaded successfully |
| 4 | Click start/stop recording button | Recording should start/stop successfully and video file should be saved |
| 5 | Test on Chrome, Firefox, Edge | Live view page should load successfully on all browsers |
| 6 | Adjust brightness slider | Image should change according to slider value |
| 7 | Adjust contrast slider | Image should change according to slider value |
| 8 | Adjust saturation slider | Image should change according to slider value |
| 9 | Adjust sharpness slider | Image should change according to slider value |
| 10 | Test reset to default quick action | Settings should reset to default values |
| 11 | Test take snapshot quick action | Snapshot should be captured |
| 12 | Test fullscreen quick action | Video should enter fullscreen mode |
| 13 | Test video fit modes | Video should fit according to selected mode |
| 14 | Test WebRTC protocol (H.264) | Stream should play using WebRTC |
| 15 | Test MSE protocol (H.265) | Stream should play using MSE |

### Playback

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Check if playback page loads | Playback page should load successfully |
| 2 | Check SD card not inserted message (without SD card) | Should show "SD card not inserted" message |
| 3 | Check if sample UI is visible (with SD card) | Sample UI should be visible with timeline |

### Video Encoder Configuration

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Change resolution to 1080p | Resolution should change successfully |
| 2 | Change resolution to 2K | Resolution should change successfully |
| 3 | Change resolution to 480p | Resolution should change successfully |
| 4 | Change bitrate type to CBR | Bitrate type should change successfully |
| 5 | Change bitrate type to VBR | Bitrate type should change successfully |
| 6 | Set bitrate to 512Kbps | Bitrate should change successfully |
| 7 | Set bitrate to 16Mbps | Bitrate should change successfully |
| 8 | Test settings for Stream 0 | Settings should apply to Stream 0 |
| 9 | Test settings for Stream 1 | Settings should apply to Stream 1 |
| 10 | Test settings for Stream 2 | Settings should apply to Stream 2 |

**WARNING: Do not change FPS (keep at 30). Do not change advanced settings (dev team only).**

### Network Configuration

#### Basic Settings Tab
| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Check hostname matches MAC address | Hostname should match MAC address |
| 2 | Configure DHCP IPv4 | DHCP configuration should work properly |
| 3 | Configure Static IPv4 | Static IP configuration should work properly |
| 4 | Apply changes and verify | Changes should be reflected after apply |
| 5 | Configure DNS server | DNS server should work properly |
| 6 | Configure DHCP IPv6 | DHCP IPv6 should work properly |
| 7 | Configure Static IPv6 | Static IPv6 should work properly |
| 8 | Check SSL certificate validity | Should show 1970 to 1971 |
| 9 | Check advanced network settings page | Page should load properly (no functionality) |

#### SMTP Tab
| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Configure SMTP settings | Configuration should save properly |
| 2 | Send test email | Test email should be sent successfully |

**Note:** Only use in open networks, not in enterprise networks.

#### SNMP Tab
| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Configure SNMP settings | Settings should save properly |
| 2 | Test SNMP v1 | SNMP v1 should work (verify with snmpwalk) |
| 3 | Test SNMP v2c | SNMP v2c should work (verify with snmpwalk) |
| 4 | Test SNMP v3 | SNMP v3 should work (verify with snmpwalk) |

**Tools:** Use `snmpwalk` command or Paessler SNMP Tester

#### UPnP Tab
| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Configure UPnP settings | Settings should save properly |
| 2 | Check IGD device discovery | IGD device should be discovered |
| 3 | Add port mapping | Mapping should be added (UI only) |
| 4 | Delete port mapping | Mapping should be deleted (UI only) |

**Note:** Port mappings don't actually take effect, just verify UI functionality.

#### HTTPS Tab
| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Configure HTTPS settings | Settings should save properly |
| 2 | Generate self-signed certificate | Certificate should generate successfully |
| 3 | Verify HTTPS works in browser | HTTPS should work properly |

### Image Adjustment Page

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Adjust brightness slider | Image should change according to value |
| 2 | Adjust contrast slider | Image should change according to value |
| 3 | Adjust saturation slider | Image should change according to value |
| 4 | Adjust sharpness slider | Image should change according to value |
| 5 | Adjust gamma slider | Image should change according to value |
| 6 | Configure white balance settings | Settings should work properly |
| 7 | Configure exposure settings | Settings should work properly |
| 8 | Configure AGC settings | Settings should work properly |
| 9 | Configure day/night settings | Settings should work (except IR mode & sensitivity) |
| 10 | Configure orientation (flip) | Flip should work properly |
| 11 | Configure orientation (mirror) | Mirror should work properly |
| 12 | Configure anti-flicker settings | Settings should work properly |

**WARNING: Do not test:** Backlight compensation, dynamic range, image enhancement, rotation.

### Storage Page

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Check if storage page loads | Page should load properly |
| 2 | Check SD card not inserted message (without SD card) | Message should display |
| 3 | Check SD card info (with SD card) | SD card info should display properly |
| 4 | Check scheduled recording UI | UI should display properly |

**WARNING: Do not test:** Format SD card, storage management, event recording.

### System Page

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Check if system page loads | Page should load properly |
| 2 | Check user management shows admin user | Admin user should be visible |

### OSD (On-Screen Display)

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Save OSD settings | Settings should save properly |
| 2 | Check OSD appears on live view | OSD should appear on live view |
| 3 | Move OSD position from web UI | OSD should move accordingly |
| 4 | Configure custom text for Stream 0 | Custom text should display |
| 5 | Configure custom text for Stream 1 | Custom text should display |
| 6 | Configure custom text for Stream 2 | Custom text should display |
| 7 | Configure logo for all streams | Logo should display |

### Privacy Mask

| # | Steps | Expected Result |
|---|-------|-----------------|
| 1 | Save privacy mask settings | Settings should save properly |
| 2 | Check privacy mask appears on live view | Mask should appear on live view |
| 3 | Add privacy mask for Stream 0 | Mask should be added |
| 4 | Edit privacy mask for Stream 0 | Mask should be edited |
| 5 | Delete privacy mask for Stream 0 | Mask should be deleted |
| 6 | Add privacy mask for Stream 1 | Mask should be added |
| 7 | Add privacy mask for Stream 2 | Mask should be added |

---

## Test Environment

- **Supported Browsers:** Chrome, Firefox, Edge (latest versions)
- **Streaming Protocols:** WebRTC (H.264), MSE (H.265)
- **SNMP Testing:** Use `snmpwalk` command or Paessler SNMP Tester
- **SMTP Testing:** Use open networks only (not enterprise)

---

## Coming Soon

- Audio support
- SD card recording and playback
- Analytics features
- Custom SSL certificates
- Full storage management
- Event-based recording
- Logout functionality
- Day/Night page fix
- IR mode & sensitivity controls

---

**© 2025 ETERNA IP Camera Project**