# Release Notes - Version 1.4.0

**Release Date:** February 16, 2026

---

## Summary

This release introduces major enhancements including a comprehensive events management system, SSH/SFTP server support, advanced detection capabilities (motion and tamper), system logging infrastructure, expanded flash storage utilization, video encoder improvements, and enhanced ONVIF compliance with token and user management.

---

## New Features

### 1. Events Module
- **Comprehensive event management system** with custom event definition support
- **Event Actions:** Define custom actions triggered by events
  - Send notifications
  - Trigger alarms
  - Log events
  - Execute custom handlers
- **Event Triggers:** Support for multiple trigger types
  - Motion detection events
  - Sensor input triggers
  - System events
  - Schedule-based triggers
- **Event Log Management:** View and manage historical event records
- **User Interface:** Intuitive web UI for event configuration and monitoring
- Flexible event-action mapping for custom automation workflows

### 2. SSH Server with SFTP Support
- **Secure SSH server** with industry-standard security practices
- **SFTP support** for secure file transfer
- Automatic startup on boot
- Key-based authentication support
- Configurable access controls and permissions
- Secure remote shell access for system administration

### 3. Detection Capabilities

#### Motion Detection
- **Enhanced motion detection** with configurable sensitivity
- Region-based motion detection support
- Adjustable detection thresholds
- Integration with events module for automated actions

#### Tamper Detection
- **Camera tamper detection** for physical security
- Detects camera covering, blocking, or repositioning
- Automatic alerts when tampering is detected
- Configurable sensitivity levels

### 4. System Logging Infrastructure
- **Comprehensive system logging** to both SD card and flash storage
- Automatic log rotation and management
- Configurable log levels and categories
- Persistent logging across reboots
- Easy log retrieval for debugging and diagnostics
- Supports remote log viewing via SSH/SFTP

### 5. Storage Expansion
- **Increased application partition size** to utilize full 256 MB flash capacity
- Optimized flash memory layout for better storage efficiency
- More space available for applications and user data
- Improved system performance with expanded storage

### 6. ONVIF Enhancements

#### Token Management
- **ONVIF token-based authentication** for improved security
- Session token management with expiration handling
- Token refresh mechanisms
- Enhanced security for ONVIF client connections

#### User Management
- **Complete ONVIF user management** implementation
- Create, modify, and delete users via ONVIF
- Role-based access control
- User authentication and authorization
- ONVIF Profile S compliant user management

### 7. AI Analytics Integration
- **AI analytics handlers** for intelligent video processing
- Support for AI-based event detection
- Integration with events module for AI-triggered actions
- Extensible framework for future AI capabilities

### 8. Schedule Recording Enhancements
- **Schedule recording profile support** for time-based recording
- Configurable recording schedules per profile
- Integration with events module
- Flexible time-based recording rules

---

## Bug Fixes

### Video Encoder
- **Resolved all known video encoder bugs**
- Fixed video stream stability issues
- Corrected encoding parameter handling
- Improved encoder performance and reliability
- Fixed frame rate consistency problems
- Resolved bitrate control issues
- Corrected I-frame interval handling
- Fixed memory leaks in encoder module
- Improved multi-stream encoding stability

---

## System Improvements

### Storage & Performance
- Optimized flash partition layout
- Improved system boot time
- Enhanced storage reliability

### Security
- SSH server with hardened security configuration
- Secure authentication mechanisms
- Token-based ONVIF security

### Monitoring & Diagnostics
- Enhanced logging infrastructure
- Better system observability
- Improved debugging capabilities

---

## Configuration Changes

### Default Settings
- SSH server enabled by default on boot
- System logging to SD card enabled (if SD card present)
- Motion detection sensitivity set to medium by default
- Events module initialized with default event templates

### Storage Layout
- Application partition expanded to 256 MB
- Log partition allocated on both flash and SD card
- Optimized partition alignment for better performance

---

## QA Testing Guide

### 1. Video Encoder Settings - ALL STREAMS (Main, Sub1, Sub2)

#### Test Resolution Changes
1. Navigate to **Video → Encoder Settings** in web UI
2. For **Main Stream:**
   - Test all available resolutions (1920x1080, 1280x720, 640x480, etc.)
   - Verify video output after each resolution change
   - Confirm RTSP stream reflects new resolution
3. For **Sub Stream 1:**
   - Change resolution independently from main stream
   - Verify stream stability
   - Check RTSP URL output
4. For **Sub Stream 2:**
   - Test all available sub-stream resolutions
   - Verify concurrent streaming with main and sub1
   - Confirm no interference between streams

#### Test Codec Changes
Test the following codecs for each stream:
- **H.264** (baseline, main, high profiles if available)
- **H.265/HEVC** (if supported)
- **MJPEG** (if available)

For each codec:
1. Select codec in web UI
2. Apply settings and wait 2-3 seconds
3. Open RTSP stream and verify playback
4. Check video quality and smoothness
5. Verify codec in stream metadata

#### Test Bitrate Settings
For **each stream** (Main, Sub1, Sub2):

| Bitrate Mode | Test Procedure |
|--------------|----------------|
| **CBR (Constant)** | Set to 2048 Kbps, verify stable bitrate in stream |
| **VBR (Variable)** | Set range 1024-4096 Kbps, verify bitrate varies with scene complexity |
| **Low Values** | Test 512 Kbps, 256 Kbps - verify stream works at low bitrates |
| **High Values** | Test 8192 Kbps, 16384 Kbps - verify no overflow or artifacts |

#### Test Frame Rate
For each stream:
- Test frame rates: 5, 10, 15, 20, 25, 30 fps
- Verify actual output frame rate matches setting
- Check for dropped frames during streaming

#### Test I-Frame Interval (GOP Size)
- Test GOP values: 15, 30, 60, 120
- Verify I-frame generation using stream analysis tool
- Confirm seeking behavior in recorded video

#### Test Audio Settings
1. **Audio Enable/Disable:**
   - Toggle audio on/off for each stream
   - Verify audio presence in RTSP stream
2. **Audio Codec:**
   - Test G.711 (A-law and μ-law)
   - Test G.726 (if available)
   - Test AAC (if available)
3. **Audio Bitrate/Sample Rate:**
   - Test different sample rates (8kHz, 16kHz, 48kHz)
   - Verify audio quality at each setting
4. **Audio Volume:**
   - Adjust input/output volume levels
   - Test at min, mid, and max levels

#### Test Video Settings
For each stream, adjust and verify:

| Setting | Test Values | Verification |
|---------|-------------|--------------|
| **Brightness** | 0, 50, 100 | Check image brightness in stream |
| **Contrast** | 0, 50, 100 | Verify contrast levels |
| **Saturation** | 0, 50, 100 | Check color intensity |
| **Sharpness** | 0, 50, 100 | Verify edge definition |
| **Hue** | -180 to +180 | Check color shift |

#### Concurrent Stream Testing
1. Open all three streams simultaneously (Main + Sub1 + Sub2)
2. Change settings on main stream, verify others unaffected
3. Change settings on sub-streams independently
4. Monitor CPU/memory usage during multi-stream operation
5. Verify no stream freezing or dropping

#### Stress Testing
1. Rapidly change settings (resolution, codec, bitrate) 5-10 times
2. Verify system remains stable
3. Check for memory leaks after multiple setting changes
4. Restart streams and verify quick recovery

### 2. SSH/SFTP Access Testing

#### Connection Details
- **IP Address:** `<your-camera-ip>` (obtain from device or DHCP)
- **Username:** `admin`
- **Password:** `123456`
- **Default SSH Port:** `22`

#### SSH Testing Procedure
1. **Initial Connection Test:**
   ```bash
   ssh admin@<camera-ip>
   # Enter password: 123456
   ```
   - Verify successful login
   - Check welcome message/banner
   - Verify shell prompt appears

2. **Basic Commands Test:**
   ```bash
   # Check system info
   uname -a
   cat /proc/version
   
   # Check running processes
   ps aux | grep ipcamera
   
   # Check disk space
   df -h
   
   # Check memory usage
   free -m
   
   # Check network interfaces
   ifconfig
   ```

3. **Persistence Test:**
   - Reboot device: `reboot`
   - After boot, reconnect via SSH
   - Verify SSH service started automatically

4. **Multiple Sessions:**
   - Open 2-3 simultaneous SSH connections
   - Verify all sessions work concurrently
   - Log out from each session properly

#### SFTP Testing Procedure
1. **SFTP Connection:**
   ```bash
   sftp admin@<camera-ip>
   # Enter password: 123456
   ```
   - Verify successful connection
   - Check SFTP prompt appears

2. **File Upload Test:**
   ```bash
   # Create test file locally
   echo "test content" > test_upload.txt
   
   # In SFTP session:
   put test_upload.txt /tmp/
   ls /tmp/test_upload.txt
   ```

3. **File Download Test:**
   ```bash
   # In SFTP session:
   get /etc/hosts ./hosts_from_camera
   
   # Exit SFTP and verify locally:
   exit
   ls -l hosts_from_camera
   cat hosts_from_camera
   ```

4. **Directory Operations:**
   ```bash
   # In SFTP session:
   mkdir /tmp/sftp_test_dir
   ls -la /tmp/
   cd /tmp/sftp_test_dir
   pwd
   rmdir /tmp/sftp_test_dir
   ```

5. **Large File Transfer:**
   - Upload a 10-50 MB file
   - Verify transfer completes successfully
   - Check file integrity (compare checksums)

6. **SFTP Client Tools:**
   - Test with command-line SFTP client
   - Test with FileZilla or WinSCP (GUI)
   - Verify file browsing and transfer in GUI

### 3. System Logging to SD Card

#### Prerequisites
- Ensure SD card is properly inserted in device
- SD card should be formatted and mounted
- Check mount point: `/mnt/sd/`

#### Logging Test Procedure

1. **Verify SD Card Mount:**
   ```bash
   ssh admin@<camera-ip>
   df -h | grep sd
   ls -la /mnt/sd/
   ```

2. **Check Logs Directory:**
   ```bash
   ls -la /mnt/sd/logs/
   ```
   Expected log files:
   - `system.log` - General system logs
   - `application.log` - Application-specific logs
   - `event.log` - Event module logs
   - `network.log` - Network-related logs
   - `video.log` - Video encoder/streaming logs
   - `error.log` - Error messages

3. **Verify Log Generation:**
   ```bash
   # Check log content
   tail -f /mnt/sd/logs/system.log
   
   # Perform some actions (change settings, trigger events)
   # Observe new log entries in real-time
   ```

4. **Log Content Verification:**
   ```bash
   # Check each log file has recent timestamps
   ls -lh /mnt/sd/logs/
   
   # View recent entries
   tail -n 50 /mnt/sd/logs/system.log
   tail -n 50 /mnt/sd/logs/application.log
   tail -n 50 /mnt/sd/logs/event.log
   ```

5. **Log Rotation Test:**
   ```bash
   # Check for rotated logs
   ls -la /mnt/sd/logs/ | grep .1
   ls -la /mnt/sd/logs/ | grep .gz
   
   # Verify oldest logs are compressed/rotated
   ```

6. **Log Download via SFTP:**
   ```bash
   sftp admin@<camera-ip>
   cd /mnt/sd/logs/
   ls -l
   get system.log
   get application.log
   exit
   
   # Verify downloaded logs locally
   cat system.log
   ```

7. **Test Scenarios:**

   **Scenario A - System Boot Logs:**
   - Reboot device
   - Wait for complete boot
   - Check `/mnt/sd/logs/system.log` for boot sequence
   - Verify startup timestamps

   **Scenario B - Configuration Change Logs:**
   - Change video encoder settings via web UI
   - Check logs for configuration change entries
   - Verify timestamp and change details

   **Scenario C - Event Logs:**
   - Trigger motion detection event
   - Check `/mnt/sd/logs/event.log`
   - Verify event is logged with proper details

   **Scenario D - Error Condition Logs:**
   - Remove SD card while device running
   - Reinsert SD card
   - Check if error is logged and recovery is documented

8. **Log Persistence Test:**
   - Note current log content
   - Reboot device
   - After boot, verify logs were preserved
   - Confirm new logs appended (not overwritten)

9. **Disk Space Management:**
   ```bash
   # Check log size limits
   du -sh /mnt/sd/logs/
   
   # Verify logs don't exceed allocated space
   # Confirm old logs are rotated/deleted
   ```

#### Log Format Verification
Expected log format:
```
[YYYY-MM-DD HH:MM:SS] [LOG_LEVEL] [MODULE] Message
Example:
[2026-02-16 14:30:45] [INFO] [VIDEO_ENCODER] Resolution changed to 1920x1080
[2026-02-16 14:30:46] [DEBUG] [RTSP_SERVER] Stream started on channel 0
[2026-02-16 14:31:10] [ERROR] [NETWORK] Connection timeout to NTP server
```

---

## Additional Testing Notes

### Events Module
1. Navigate to Events → Management in web UI
2. Create a new event with motion detection trigger
3. Configure actions (notification, alarm, log)
4. Test event triggering by causing motion in camera view
5. Verify event logs in Events → Log Viewer

### Detection Features
| Feature | Test Procedure |
|---------|----------------|
| Motion Detection | Enable motion detection, move object in view, verify event trigger |
| Tamper Detection | Enable tamper detection, cover camera lens, verify alert |

---

## API Changes

### ONVIF
- Added token management endpoints
- Added user management operations
- Added AI analytics event handlers
- Enhanced schedule recording profile support

---

## Known Issues

- Motion detection may require fine-tuning for optimal sensitivity in certain lighting conditions
- SSH initial connection may take a few seconds on first boot
- Event logs on flash are limited to 10 MB; older logs are automatically rotated

---

## Upgrade Notes

⚠️ **Important:** This release changes the flash partition layout. Upgrading from previous versions will require a full firmware update.

- Backup important data before upgrading
- SD card settings and recordings are preserved
- User configurations will be migrated automatically
- First boot after upgrade may take additional time due to partition resizing

---

## Documentation

- Added Events Module User Guide
- Updated SSH/SFTP Configuration Guide
- Enhanced ONVIF API documentation for token and user management
- Added Detection Features Configuration Guide
- Updated System Logging documentation

---