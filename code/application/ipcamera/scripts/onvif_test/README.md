# ONVIF Compliance Testing Tool

A Python-based tool for testing ONVIF compliance of IP cameras.

## Features

This tool tests the following ONVIF services:

- **Device Management Service**
  - GetDeviceInformation
  - GetCapabilities
  - GetSystemDateAndTime
  - GetScopes
  - GetServices
  - GetHostname
  - GetNetworkInterfaces

- **Media Service**
  - GetProfiles
  - GetVideoSources
  - GetAudioSources
  - GetStreamUri
  - GetSnapshotUri
  - GetVideoEncoderConfigurations

- **PTZ Service** (if available)
  - GetConfigurations
  - GetNodes
  - GetStatus

- **Imaging Service** (if available)
  - GetImagingSettings
  - GetImagingOptions

- **Events Service** (if available)
  - GetEventProperties

## Installation

The tool is already installed in the virtual environment. No additional installation is required.

## Usage

### Using the Shell Script (Recommended)

```bash
# Make the script executable (first time only)
chmod +x run_onvif_test.sh

# Test a camera with default credentials (admin/admin)
./run_onvif_test.sh 192.168.1.100

# Test a camera on a specific port
./run_onvif_test.sh 192.168.1.100 80

# Test with custom credentials
./run_onvif_test.sh 192.168.1.100 80 admin mypassword
```

### Using Python Directly

```bash
# Activate the virtual environment
source onvif-env/bin/activate

# Run the tester
python onvif_tester.py 192.168.1.100 80 admin password

# Deactivate when done
deactivate
```

## Output

The tool provides:

1. **Console Output**: Colored output showing test results in real-time
   - ✓ Green: Passed tests
   - ✗ Red: Failed tests
   - ⚠ Yellow: Warnings

2. **JSON Report**: A detailed JSON file is saved with all test results:
   - Device information
   - Available services
   - Media profiles with stream URIs
   - Individual test results

Example JSON report filename: `onvif_report_192_168_1_100_20260126_183800.json`

## Example Output

```
============================================================
ONVIF Compliance Tester
Target: 192.168.1.100:80
============================================================
[2026-01-26 18:38:00] [INFO] Connecting to camera at 192.168.1.100:80...
[2026-01-26 18:38:01] [PASS] ✓ Connection: Successfully connected to 192.168.1.100:80
[2026-01-26 18:38:01] [INFO] Testing Device Management Service...
[2026-01-26 18:38:01] [PASS] ✓ GetDeviceInformation: Manufacturer: Novatek, Model: NT98538
...

📊 Test Summary:
   Passed: 15/18
   Failed: 3/18
   Warnings: 2
```

## Troubleshooting

### Connection Issues
- Ensure the camera is reachable (try `ping <camera_ip>`)
- Verify the ONVIF port (common ports: 80, 8080, 8899)
- Check username and password

### Service Not Available
- Some cameras may not implement all ONVIF services
- PTZ, Imaging, and Events are optional services

### Timeout Errors
- The camera may be slow to respond
- Try running the test again

## Files

- `onvif_tester.py` - Main testing script
- `run_onvif_test.sh` - Shell wrapper script
- `onvif-env/` - Python virtual environment with dependencies
- `onvif_report_*.json` - Generated test reports

## Dependencies

- Python 3.x
- onvif-zeep (installed in virtual environment)
- zeep (SOAP client library)
