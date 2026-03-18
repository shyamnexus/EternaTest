# Configuration: cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT

## Overview
This is a combined configuration based on:
- **Base**: `cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP` (ETERNA camera with AI ISP)
- **Secure Boot**: `cfg_539A_IPC_NAND_RAMDISK_TEE_SECBOOT_EVB` (Secure boot features)

## Key Features

### Hardware Configuration
- **Chip**: NT98539A (CHIP_NT98539A)
- **DRAM**: 512MB (0x20000000)
- **Storage**: 128MB SPI NAND Flash
- **Filesystem**: RAMDISK
- **Ethernet**: EQOS enabled

### Secure Boot Features (Enabled)
- **OP-TEE**: ENABLED with 2 cores
- **Kernel Config**: `ns02302_ipc_a64_tee_evb_defconfig_release`
- **U-Boot Config**: `nvt-ns02302_a64_tee_secboot_nand_defconfig`
- **Signature Method**: ECDSA (Elliptic Curve Digital Signature Algorithm)

### Encryption Settings (All Enabled)
- `ENCRYPT_MSG = ON` - Message encryption
- `ENCRYPT_DTS = ON` - Device tree encryption
- `ENCRYPT_ATF = ON` - ARM Trusted Firmware encryption
- `ENCRYPT_OPTEE = ON` - OP-TEE encryption
- `ENCRYPT_UBOOT = ON` - U-Boot encryption
- `ENCRYPT_KERNEL = ON` - Linux kernel encryption

### Security Keys
- **AES Key**: `aes.txt` - AES encryption key for secure boot
- **RSA Keys**: `rsa_priv.txt` / `rsa_pub.txt` - RSA key pair for signing

### Applications Included
**External Applications:**
- dhd_priv
- memtester
- iperf-3
- nginx
- fcgiwrap
- adbd (Android Debug Bridge)
- netsnmp
- openssh

**Internal Applications:**
- mem
- sw_dbg
- i2c_access

### Memory Layout (Secure Boot Configuration)
```
DRAM Total:          0x00000000 - 0x20000000 (512MB)
Core2 Entry 1:       0x00000000 - 0x00010000 (64KB)
Core2 Entry 2:       0x00010000 - 0x00010000 (64KB)
FDT:                 0x00100000 - 0x00100000 (1MB)
Shared Memory:       0x00A00000 - 0x00100000 (1MB)
Loader:              0x01000000 - 0x00100000 (1MB)
ATF:                 0x01F00000 - 0x00100000 (1MB)
Linux Temp:          0x02000000 - 0x0E000000 (224MB)
All-in-One:          0x10000000 - 0x06000000 (96MB)
Kernel Image:        0x16000000 - 0x01000000 (16MB)
U-Boot:              0x1FE00000 - 0x00200000 (2MB)
```

**Linux System Memory**: 0x00000000 - 0x0E000000 (224MB)

### Reserved Memory (with OP-TEE)
```
ATF Reserved:        0x01F00000 - 0x00100000 (1MB)
CMA0:                0x05800000 - 0x00800000 (8MB)
```

### Storage Configuration
- **Root Filesystem**: RAMDISK with RW partition enabled
- **APPFS**: Enabled
- **JFFS2 Block Size**: 128KiB
- **Page Size**: 2048 bytes
- **UBI RW Max LEB Count**: 668

### Network Configuration
- **Default Protocol**: Static IP
- **Ethernet**: EQOS enabled
- **WiFi**: None (can be enabled if needed)
- **4G**: None

### Build Configuration
- **Linux SMP**: ON (Symmetric Multi-Processing)
- **Binary Strip**: YES
- **Linux Compression**: AUTO
- **FPGA Emulation**: OFF
- **U-Boot Environment Storage**: OFF (secure boot mode)

## Differences from Base ETERNA Config

### Security Enhancements Added:
1. **OP-TEE Enabled**: Changed from `DISABLE` to `ENABLE` with 2 cores
2. **Secure Boot U-Boot**: Changed from `nvt-ns02302_a64_nand_defconfig` to `nvt-ns02302_a64_tee_secboot_nand_defconfig`
3. **TEE Kernel**: Changed from `ns02302_ipc_a64_evb_defconfig_release` to `ns02302_ipc_a64_tee_evb_defconfig_release`
4. **All Encryption Flags**: Added and enabled (MSG, DTS, ATF, OPTEE, UBOOT, KERNEL)
5. **Signature Method**: Added ECDSA signing
6. **U-Boot Environment**: Changed from NAND storage to OFF (more secure)
7. **Security Keys**: Added AES and RSA key files

### Memory Differences:
- Added OP-TEE reserved memory regions in device tree
- TEE OS region: 0x02000000 - 0x00800000 (8MB)
- Non-secure memory region for OP-TEE communication

## Usage

To build with this configuration:
```bash
cd ns02302_linux_sdk
source build/envsetup.sh
# Select configuration: cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT
make all
```

## Security Notes

⚠️ **IMPORTANT**: The included key files (`aes.txt`, `rsa_priv.txt`, `rsa_pub.txt`) are examples from the EVB configuration. 

**For production use:**
1. Generate new unique keys for each product
2. Store private keys securely
3. Never commit production keys to version control
4. Consider using hardware security modules (HSM) for key storage

## Related Configurations
- **Base Configuration**: `cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP`
- **Reference Secure Boot**: `cfg_539A_IPC_NAND_RAMDISK_TEE_SECBOOT_EVB`
