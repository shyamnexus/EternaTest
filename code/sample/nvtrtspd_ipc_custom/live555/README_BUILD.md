# Live555 Cross-Compilation for Novatek Platform

## Overview
This directory contains the Live555 (live.2025.10.13) RTSP streaming library cross-compiled for the Novatek NVT aarch64-ca53-linux-gnueabihf platform.

## Build Information

### Cross-Compiler
- **Toolchain**: aarch64-ca53-linux-gnueabihf-10.4.0
- **Target Architecture**: AArch64 (ARM 64-bit)
- **C++ Standard**: C++20
- **Special Flags**: -DNO_STD_LIB=1 (to support GCC 10.4.0)

### Dependencies
- **OpenSSL**: 3.5.4 (from SDK)
  - Headers: `code/lib/include/openssl/`
  - Libraries: `code/lib/output/libssl.so`, `libcrypto.so`

## Build Instructions

### Prerequisites
1. Source the SDK environment:
   ```bash
   cd /path/to/ns02302_linux_sdk
   source build/envsetup.sh
   ```

### Building
Simply run the build script:
```bash
cd code/sample/nvtrtspd_ipc_custom/live555
./build_live555.sh
```

The script will:
1. Check if the SDK environment is set up
2. Extract the live555 source if needed
3. Clean any previous build
4. Configure for Novatek aarch64 platform
5. Build all libraries and test programs
6. Install libraries to `lib/` and headers to `include/`

## Output

### Libraries (in `lib/`)
- `libBasicUsageEnvironment.a` - Basic task scheduler and usage environment
- `libgroupsock.a` - Network socket utilities
- `libliveMedia.a` - Main RTSP streaming library (H.264, H.265, MPEG, etc.)
- `libUsageEnvironment.a` - Abstract usage environment interface

### Headers (in `include/`)
All necessary header files for linking against the libraries

### Test Programs (in `live/testProgs/`)
- `testRTSPClient` - RTSP client test application
- `testOnDemandRTSPServer` - On-demand RTSP server test
- Many other test and example programs

### Media Server (in `live/mediaServer/`)
- `live555MediaServer` - Full-featured RTSP media server

## Configuration File
The custom configuration is in: `live/config.nvt-aarch64`

Key settings:
- Cross-compiler prefix: `aarch64-ca53-linux-gnueabihf-`
- OpenSSL include path from SDK
- Link flags for OpenSSL libraries
- C++20 standard with NO_STD_LIB flag

## Usage in Your Application

### Linking
```makefile
LIVE555_DIR = /path/to/live555
INCLUDES = -I$(LIVE555_DIR)/include
LIBS = -L$(LIVE555_DIR)/lib \
       -L/path/to/sdk/code/lib/output \
       -lliveMedia -lgroupsock \
       -lBasicUsageEnvironment -lUsageEnvironment \
       -lssl -lcrypto
```

### Example
```cpp
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
// ... your RTSP streaming code
```

## Notes
- The `NO_STD_LIB` flag is required because GCC 10.4.0 doesn't fully support `std::atomic_flag::test()` method introduced in C++20
- OpenSSL is required for TLS/SSL support in RTSP streaming
- All libraries are statically compiled (`.a` files)

## Version Information
- **Live555**: 2025.10.13
- **Build Date**: October 29, 2025
- **SDK**: ns02302_linux_sdk

## Troubleshooting

### Build Fails with OpenSSL Errors
Ensure the SDK environment is properly sourced and OpenSSL libraries are built in the SDK.

### Atomic Flag Test Error
This is resolved by the `NO_STD_LIB` flag in the configuration.

### Linking Errors
Make sure to link against all four live555 libraries in the correct order, and include OpenSSL libraries at the end.

## References
- Live555 Homepage: http://www.live555.com/
- Live555 FAQ: http://live555.com/liveMedia/faq.html
- License: LGPL (see COPYING and COPYING.LESSER files in the live/ directory)
