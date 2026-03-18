# Firmware Upgrade Module

Secure firmware upgrade support for Novatek NS02302 IP Camera platform.

## Overview

This module implements a complete firmware upgrade workflow compatible with the SDK's secure boot infrastructure:

1. **Load**: Receive firmware via HTTP upload or file
2. **Verify**: Validate NVTPACK format, checksums, and RSA signatures  
3. **Stage**: Copy to physical memory address known to U-Boot
4. **Reboot**: Trigger system reboot with boot flags set
5. **Flash**: U-Boot handles actual flash programming on boot

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │ HTTP Upload  │  │ Web UI       │  │ CLI Tool     │       │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘       │
│         │                 │                 │               │
│         ▼                 ▼                 ▼               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              UpgradeManager (Singleton)              │    │
│  │  - beginUpgrade() → writeFirmwareChunk() → finish   │    │
│  │  - stageForFlashing() → commitUpgrade()              │    │
│  └───────────────────────┬─────────────────────────────┘    │
│                          │                                   │
│    ┌─────────────────────┼─────────────────────┐            │
│    │                     │                     │            │
│    ▼                     ▼                     ▼            │
│  ┌──────────────┐ ┌──────────────┐  ┌──────────────┐       │
│  │Firmware      │ │Physical      │  │System        │       │
│  │Validator     │ │Memory        │  │Controller    │       │
│  │(RSA, CRC)    │ │(/dev/mem)    │  │(Services,    │       │
│  │              │ │              │  │Reboot)       │       │
│  └──────────────┘ └──────────────┘  └──────────────┘       │
└─────────────────────────────────────────────────────────────┘
                           │
                           │ reboot
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                      U-Boot Layer                            │
│  ┌─────────────────────────────────────────────────────┐    │
│  │          nvt_ivot_fw_update.c                        │    │
│  │  - Check fw_update flag / scratch registers          │    │
│  │  - Read firmware from staging address                │    │
│  │  - Verify RSA signature (secure boot)                │    │
│  │  - Decrypt partitions (AES-128)                      │    │
│  │  - Flash each partition to NAND/eMMC                 │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## NVTPACK Firmware Format

The firmware file follows the Novatek NVTPACK format:

```
┌────────────────────────────────────────┐
│ NvtpackFwHeader2 (128 bytes)           │
│  - GUID: "NVTPACK_FW_HDR2\0\0"         │
│  - Version: 0x18071515                 │
│  - Total size, partition count         │
│  - Checksum (16-bit sum)               │
├────────────────────────────────────────┤
│ Partition Headers[] (64 bytes each)    │
│  - Partition ID (type)                 │
│  - Offset, Size                        │
│  - SHA256 hash (for secure boot)       │
├────────────────────────────────────────┤
│ Partition 0: BL31 (ATF)                │
├────────────────────────────────────────┤
│ Partition 1: FDT (Device Tree)         │
├────────────────────────────────────────┤
│ Partition 2: U-Boot                    │
├────────────────────────────────────────┤
│ Partition 3: Kernel                    │
├────────────────────────────────────────┤
│ Partition 4: RootFS                    │
├────────────────────────────────────────┤
│ Partition 5: APP                       │
├────────────────────────────────────────┤
│ ... (additional partitions)            │
└────────────────────────────────────────┘
```

## Secure Boot

When built with `cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT`, firmware includes:

- **RSA-2048 Signatures**: Critical partitions (BL31, U-Boot, Kernel, RootFS) are signed
- **AES-128 Encryption**: Partitions are encrypted with key from `aes.txt`
- **Key Files**:
  - `configs/Linux/cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT/rsa_pub.txt` - RSA public key (N, E)
  - `configs/Linux/cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT/aes.txt` - AES-128 key

## Firmware Security Modes

The module supports three security modes:

| Mode | Description | Use Case |
|------|-------------|----------|
| `AcceptAny` (default) | Accept any valid NVTPACK firmware | Development, mixed environments |
| `PreferSecure` | Accept unsigned but log warning | Transition to secure boot |
| `RequireSecure` | Reject unsigned firmware | Production secure boot |

```cpp
// Accept both encrypted and unencrypted firmware
config.securityMode = FirmwareSecurityMode::AcceptAny;

// Production: require secure boot
config.securityMode = FirmwareSecurityMode::RequireSecure;
```

## API Usage

### C++ Integration

```cpp
#include "ipcam/upgrade_manager.h"
#include "ipcam/upgrade_types.h"

// Configure
ipcam::upgrade::UpgradeConfig config;
config.requireSecureBoot = true;
config.firmwarePhysAddr = 0x10000000;  // BOARD_ALL_IN_ONE_ADDR
config.rsaPublicKeyN = "D3A263B309...";  // From rsa_pub.txt
config.rsaPublicKeyE = "10001";

auto& mgr = ipcam::upgrade::UpgradeManager::instance();
mgr.configure(config);

// Upgrade from file
auto result = mgr.upgradeFromFile("/tmp/FW98538A.bin");
if (!result.success) {
    std::cerr << "Upgrade failed: " << result.message << std::endl;
}
// System will reboot on success
```

### HTTP API

```bash
# Check current version
curl http://camera:8080/api/firmware/version

# Upload and validate firmware
curl -X POST http://camera:8080/api/firmware/upload \
  -F "firmware=@FW98538A.bin"

# Check status
curl http://camera:8080/api/firmware/status

# Trigger upgrade (will reboot)
curl -X POST http://camera:8080/api/firmware/upgrade

# Cancel if needed
curl -X POST http://camera:8080/api/firmware/cancel
```

### API Responses

**GET /api/firmware/version**
```json
{
  "firmwareVersion": "1.0.0-20250101",
  "bootloaderVersion": "2024.01",
  "availableMemory": 134217728
}
```

**POST /api/firmware/upload** (success)
```json
{
  "success": true,
  "message": "Firmware uploaded and validated",
  "size": 58441728,
  "partitions": 7,
  "secureBoot": true
}
```

**GET /api/firmware/status**
```json
{
  "state": "validating",
  "phase": "validating", 
  "overallPercent": 45.0,
  "phasePercent": 75.0,
  "message": "Verifying partition signatures...",
  "bytesReceived": 58441728,
  "bytesTotal": 58441728,
  "validation": {
    "partitions": 7,
    "totalSize": 58441728,
    "secureBoot": true
  }
}
```

## Memory Layout

The upgrade module uses these memory regions (from SDK configuration):

| Address      | Size  | Usage                          |
|--------------|-------|--------------------------------|
| 0x02000000   | 224MB | LINUXTMP (temporary storage)   |
| 0x10000000   | 96MB  | ALL_IN_ONE (firmware staging)  |

The staging address (0x10000000) is where firmware is placed for U-Boot to read during the update process.

## Building

The module is built as part of the IPCamera application:

```bash
cd ns02302_linux_sdk
source build/envsetup.sh
cd code/application/ipcamera

# Configure with upgrade module enabled
cmake -B build/aarch64-ca53 \
  -DCMAKE_BUILD_TYPE=Release \
  -DIPCAMERA_ENABLE_UPGRADE=ON

# Build
cmake --build build/aarch64-ca53 --parallel
```

## Dependencies

- **OpenSSL 3.x**: RSA signature verification, SHA-256, AES decryption
- **spdlog**: Logging
- **nlohmann_json**: JSON serialization for HTTP API
- **cpp-httplib**: HTTP server (via webserver module)

## Security Considerations

1. **Authentication**: HTTP endpoints should require authentication
2. **HTTPS**: Use TLS for firmware uploads in production
3. **Rollback**: U-Boot maintains a fallback partition for failed upgrades
4. **Verification**: All partitions are verified before flashing

## Files

```
modules/upgrade/
├── CMakeLists.txt
├── README.md
├── include/
│   └── ipcam/
│       ├── upgrade_types.h          # NVTPACK structures, enums
│       ├── upgrade_manager.h        # Main orchestrator
│       ├── firmware_validator.h     # RSA/checksum validation
│       ├── physical_memory.h        # DMA memory management
│       ├── system_controller.h      # Service/reboot control
│       └── upgrade_http_handler.h   # HTTP API handlers
└── src/
    ├── upgrade_manager.cpp
    ├── firmware_validator.cpp
    ├── physical_memory.cpp
    ├── system_controller.cpp
    └── upgrade_http_handler.cpp
```

## Related SDK Files

- `BSP/u-boot/cmd/nvt_ivot_fw_update.c` - U-Boot firmware update implementation
- `configs/Linux/cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT/` - Secure boot configuration
- `code/lib/include/nvtpack/nvtpack.h` - NVTPACK structure definitions
- `ns02302_ldr/Include/modelext/bin_info.h` - Partition type definitions
