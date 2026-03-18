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

## Testing Guide

### Events Module
1. Navigate to Events → Management in web UI
2. Create a new event with motion detection trigger
3. Configure actions (notification, alarm, log)
4. Test event triggering by causing motion in camera view
5. Verify event logs in Events → Log Viewer

### SSH/SFTP Access
1. Connect via SSH: `ssh admin@<camera-ip>`
2. Test SFTP: `sftp admin@<camera-ip>`
3. Verify file transfer capabilities
4. Confirm automatic startup after reboot

### Detection Features
| Feature | Test Procedure |
|---------|----------------|
| Motion Detection | Enable motion detection, move object in view, verify event trigger |
| Tamper Detection | Enable tamper detection, cover camera lens, verify alert |

### System Logging
1. Check logs on SD card: `/mnt/sd/logs/`
2. Check logs on flash: `/var/log/`
3. Verify log rotation functionality
4. Test log retrieval via SFTP

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