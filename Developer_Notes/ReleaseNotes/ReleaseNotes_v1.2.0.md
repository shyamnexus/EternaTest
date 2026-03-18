# Release Notes v1.2.0

**Release Date:** January 16, 2026  
**Build Status:** ✅ Successful  

---

## Summary

This release introduces major new features and fixes including Smooth ONVIF RTSP stream support via ODM, simplified privacy mask UI with global stream application, recording module with SD card support, auto-scaling OSD fonts, and a comprehensive IR control module with multiple operating modes.

---

## New Features

### 1. Smooth ONVIF RTSP Stream Support (ODM)
- Mitigate the RTSP streaming issue via ODM. (IPCAMERAMMS-2057)

### 2. Privacy Mask Improvements (IPCAMERAMMS-2083 IPCAMERAMMS-2085 IPCAMERAMMS-2087)

- **Simplified privacy mask UI** for easier configuration
- Privacy masks are now **applied globally to all streams**
- Single configuration applies consistently across main stream, sub-streams, and snapshots

### 3. Recording Module
- **New SD card-based recording functionality**
- Channel-based recording with selectable channels
- Enable/disable recording via web UI toggle
- **Note:** Select only one channel at a time for optimal performance
- Recording defaults to OFF on device startup
- Do not test recording settings, schedule recording  for now (WIP)

### 4. OSD Overlay Font Auto-Scaling 
- **Added automatic font scaling** for OSD text overlay
- Fonts now scale appropriately based on stream resolution
- Consistent text appearance across all stream types (main, sub1, sub2)

### 5. IR Control Module
- **Comprehensive IR LED and IR-cut filter control**
- Multiple operating modes:
  - **Auto Mode:** IR automatically turns on in dark scenes based on luminance threshold
  - **Day Mode:** IR remains off, IR-cut filter engaged for daylight operation
  - **Night Mode:** IR turns on immediately, IR-cut filter disengaged for night vision
  - **Schedule Mode:** IR follows user-defined time schedule for day/night transitions
- Configurable day-to-night and night-to-day thresholds
- Automatic sensor black & white mode synchronization

---

## Configuration Changes

### IR Default Configuration
- IR auto mode is now **enabled by default** 

### Recording Default Configuration
- Recording channel 0 is now **disabled by default** to match UI state
---

## Testing Guide

### Recording Module
1. Insert SD card into camera
2. Navigate to Storage → Recording in web UI
3. Enable recording toggle for desired channel (select one channel at a time)
4. Verify recording indicator and check SD card for recorded files
5. You can view the recorded files in Playback section.

   Note:- Recorded files will not have audio -> will be added later.
          Playback timeline is unstable, Do not test. 

### IR Module
| Mode | Expected Behavior |
|------|-------------------|
| Auto | IR turns on when scene becomes dark, off when bright |
| Day | IR stays off, normal daylight operation |
| Night | IR turns on immediately upon selection |
| Schedule | IR follows configured day/night schedule times |

---

## Known Limitations

- Recording: Only one channel should be selected at a time for reliable operation
- IR Auto Mode: May have brief switching delay during rapid light transitions
- IR cut filter is not operational at the moment as the PCB lacks IR cut filter driver. -> Will work in the next hardware revision. 
