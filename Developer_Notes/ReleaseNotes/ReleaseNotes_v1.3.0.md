# Release Notes - Version 1.3.0

**Release Date:** January 30, 2026

---

## Security Enhancements

### Secure Boot Implementation (⚠️ Code Only - Not in Binary)
- Added secure boot support with RSA signature verification
- Firmware images are now cryptographically signed and verified at boot
- Prevents unauthorized firmware modifications and tampering
- AES encryption for sensitive boot components
- **⚠️ NOTE:** Implementation is in code only, not included in release binary
- **⚠️ NOTE:** Requires separate testing with Firmware team coordination

### New Secure Login Method
- Implemented RSA + AES hybrid encryption for password transmission
- Client-side password encryption before sending to server
- IV/nonce-based replay attack protection
- Removed timestamp validation dependency for better compatibility with devices without NTP

### Forgot Password Security Questions
- Added security question-based password recovery
- Users can set security questions during initial setup
- Secure verification flow for password reset

---

## User Interface

### Revamped Web UI Design
- Complete redesign of the web interface with different aesthetics

---

## Image Quality Improvements

### HLC (Highlight Compensation) Enhancements
- Improved HLC algorithm for better handling of bright light sources
- Reduced blooming and glare from headlights and spotlights

### BLC (Backlight Compensation) Improvements
- Enhanced BLC functionality for high-contrast scenes

---

## Audio Enhancements

### AGC (Automatic Gain Control) Functionality
- Added AGC support for automatic audio level adjustment
- Maintains consistent audio levels regardless of source distance
- Integrated libaec, libagc, and libaudfilt audio processing libraries
- Improved audio quality for two-way communication

---

## Streaming & Connectivity

### Improved RTSP Stream Stability
- Enhanced long-term RTSP streaming reliability
- Fixed memory leaks in streaming module
- Improved connection recovery after network interruptions
- Better handling of multiple concurrent streams
- Reduced stream disconnection events

### ONVIF Compliance
- Successfully tested with ONVIF Device Test Tool
- Fixed Bug # where onvif device test tool Did not respond to any request due to Http header issues

---

## System Updates

### Update Module (⚠️ Code Only - Do Not Test)
- New update module supporting code-only firmware updates
- Faster update process by skipping unchanged components
- Rollback capability for failed updates
- Update verification and integrity checks
- **⚠️ NOTE:** Added in code only, not functional in this release
- **⚠️ NOTE:** Do not test - future release feature

---

## Dependencies & Libraries

- Added `libaec.so` - Acoustic Echo Cancellation library
- Added `libagc.so` - Automatic Gain Control library  
- Added `libaudfilt.so` - Audio Filter library
- Updated deployment scripts to include SDK audio libraries

---

## Bug Fixes

- Fixed timestamp validation issues causing login failures on devices without NTP
- Fixed authentication failures due to timezone mismatches
- Resolved RTSP stream timeout issues during extended recordings
- Fixed memory leak in ONVIF event handling
- Corrected HLC/BLC parameter persistence after reboot

---

## Known Issues

- NTP time synchronization may fail on isolated networks (does not affect functionality)
- First login after factory reset may require page refresh

---

## Documentation

- Added SecureBoot_Guide.md for implementation details
- Updated API documentation for new authentication endpoints

---