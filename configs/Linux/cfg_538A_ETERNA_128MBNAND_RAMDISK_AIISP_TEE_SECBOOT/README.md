# Configuration: cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_TEE_SECBOOT

## Overview

This configuration combines:
- **ETERNA camera features** with AI-ISP support
- **OP-TEE (Trusted Execution Environment)** for hardware-backed security
- **Secure Boot** with encryption and signature verification

### Base Configurations Combined:
1. `cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT` - ETERNA camera with AI ISP + Secure Boot
2. `cfg_538_IPC_NAND_RAMDISK_TEE_SECBOOT_EVB` - Reference TEE Secure Boot implementation

---

## Hardware Configuration

| Parameter | Value |
|-----------|-------|
| **Chip** | NT98539A (CHIP_NT98539A) |
| **DRAM** | 512MB (0x20000000) |
| **Flash** | 128MB SPI NAND |
| **Filesystem** | RAMDISK with RW partition |
| **Ethernet** | EQOS enabled |

---

## Security Features

### OP-TEE Configuration
| Setting | Value |
|---------|-------|
| **NVT_OPTEE_INSTALL** | ENABLE |
| **NVT_OPTEE_CORE_NUM** | 2 |
| **Kernel Config** | `ns02302_ipc_a64_tee_evb_defconfig_release` |
| **U-Boot Config** | `nvt-ns02302_a64_tee_secboot_nand_defconfig` |

### Encryption Settings (All Enabled)
| Component | Encrypted | File |
|-----------|-----------|------|
| Device Tree (DTS) | ✅ YES | nvt-evb.encrypted.bin |
| ARM Trusted Firmware | ✅ YES | atf.encrypted.bin |
| OP-TEE OS | ✅ YES | tee.encrypted.bin |
| U-Boot | ✅ YES | u-boot.encrypted.bin |
| Linux Kernel | ✅ YES | Image.encrypted.bin |
| Root Filesystem | ❌ NO | rootfs.ramdisk.bin |

### Signature Method
- **Algorithm**: ECDSA (Elliptic Curve Digital Signature Algorithm)
- **Keys Required**: `ecdsa_priv.key`, `ecdsa_pub.key`
- **Backup Keys**: `rsa_priv.txt`, `rsa_pub.txt` (RSA2048)

---

## Memory Layout

### DRAM Memory Map (512MB Total)

```
Address       Size        Region         Description
----------    ----------  ----------     ---------------------------
0x00000000    0x00010000  core2entry1    CPU Core 2 Entry Point 1
0x00010000    0x00010000  core2entry2    CPU Core 2 Entry Point 2
0x00100000    0x00100000  fdt            Device Tree Blob
0x00A00000    0x00100000  shmem          Shared Memory
0x01000000    0x00100000  loader         First-stage Bootloader
0x01F00000    0x00100000  atf            ARM Trusted Firmware
0x02000000    0x00C00000  teeos          OP-TEE OS (12MB) ★ NEW
0x02C00000    0x00400000  nsmem          Non-Secure Memory (4MB) ★ NEW
0x03000000    0x0D000000  linuxtmp       Linux Temporary
0x10000000    0x06000000  all_in_one     Firmware Image
0x16000000    0x01000000  kernel_img     Kernel Image
0x1FE00000    0x00200000  uboot          U-Boot
```

### Reserved Memory (no-map)
| Region | Address | Size | Purpose |
|--------|---------|------|---------|
| ATF | 0x01F00000 | 1 MB | ARM Trusted Firmware |
| TEE | 0x02000000 | 12 MB | OP-TEE Secure World |
| CMA0 | 0x05800000 | 8 MB | DMA Pool (reusable) |

### Linux System Memory
- **Address**: 0x00000000 - 0x0E000000
- **Size**: 224 MB

---

## Flash Partition Layout (128MB Total)

```
Offset      Size        Partition     Description
---------   ---------   ----------    ---------------------------
0x0000000   0x0040000   loader        First-stage bootloader (256KB)
0x0040000   0x0040000   fdt           Device Tree Blob (256KB)
0x0080000   0x0040000   fdt.restore   FDT Backup (256KB)
0x00C0000   0x0040000   atf           ARM Trusted Firmware (256KB)
0x0100000   0x0100000   teeos         OP-TEE OS (1MB) ★ NEW
0x0200000   0x00E0000   uboot         U-Boot (896KB)
0x02E0000   0x0040000   uenv          U-Boot Environment (256KB)
0x0320000   0x0500000   kernel        Linux Kernel (5MB)
0x0820000   0x2000000   rootfs        Root Filesystem (32MB)
0x2820000   0x4200000   rootfs1       Extended RootFS (66MB)
0x6A20000   0x15E0000   app           Application (21.875MB)
```

### Partition Verification
```
loader + fdt + fdt.restore + atf + teeos + uboot + uenv + kernel + rootfs + rootfs1 + app
= 256K + 256K + 256K + 256K + 1M + 896K + 256K + 5M + 32M + 66M + 21.875M
= 128 MB ✓
```

---

## Changes from Non-TEE ETERNA SECBOOT

### Memory Changes
| Region | Before (SECBOOT) | After (TEE_SECBOOT) |
|--------|------------------|---------------------|
| teeos | N/A | 0x02000000 (12MB) |
| nsmem | N/A | 0x02C00000 (4MB) |
| linuxtmp | 0x02000000 | 0x03000000 |

### Flash Partition Changes
| Partition | Before Offset | After Offset | Size Change |
|-----------|---------------|--------------|-------------|
| teeos | N/A | 0x0100000 | +1MB (NEW) |
| uboot | 0x0100000 | 0x0200000 | Same |
| uenv | 0x01E0000 | 0x02E0000 | Same |
| kernel | 0x0220000 | 0x0320000 | Same |
| rootfs | 0x0720000 | 0x0820000 | Same |
| rootfs1 | 0x2720000 | 0x2820000 | -7MB |
| app | 0x7020000 | 0x6A20000 | +6MB |

### Configuration Changes
| Setting | Before | After |
|---------|--------|-------|
| NVT_OPTEE_INSTALL | DISABLE | ENABLE |
| ENCRYPT_OPTEE | OFF | ON |
| NVT_CFG_KERNEL_CFG | ns02302_ipc_a64_evb_defconfig_release | ns02302_ipc_a64_tee_evb_defconfig_release |
| NVT_CFG_UBOOT_CFG | nvt-ns02302_a64_nand_secboot_defconfig | nvt-ns02302_a64_tee_secboot_nand_defconfig |

---

## AI-ISP Features (Retained)

The AI-ISP configuration is preserved from ETERNA:
- **ai_maxisp**: 1 (single AI-ISP effect)
- **ai_maxubuf**: 3 (max unified buffers)
- **isp_trig_mode**: 1 (frame mode)
- **pool_0**: shared unified buffer configuration

---

## Build Instructions

```bash
# Clean previous build
make distclean

# Select this configuration
make cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_TEE_SECBOOT

# Build firmware
make all

# Output files will be in output/packed/
```

---

## Security Keys

⚠️ **IMPORTANT**: Replace default keys before production deployment!

| File | Purpose | Algorithm |
|------|---------|-----------|
| aes.txt | AES-128 encryption key | AES |
| rsa_priv.txt | RSA private key (backup) | RSA-2048 |
| rsa_pub.txt | RSA public key (backup) | RSA-2048 |
| ecdsa_priv.key | ECDSA private key (primary) | ECDSA |
| ecdsa_pub.key | ECDSA public key (primary) | ECDSA |

### Key Security Checklist
- [ ] Generate new AES key for production
- [ ] Generate new ECDSA key pair
- [ ] Securely store private keys
- [ ] Program public key hash to OTP
- [ ] Test secure boot chain

---

## Boot Chain

```
┌──────────────────────────────────────────────────────────────────┐
│                        SECURE BOOT CHAIN                          │
├──────────────────────────────────────────────────────────────────┤
│  1. ROM Bootloader (OTP)                                          │
│     └─ Verifies: Loader signature                                 │
│  2. Loader (mtd0)                                                 │
│     └─ Decrypts & Verifies: ATF, OP-TEE, U-Boot                  │
│  3. ARM Trusted Firmware (mtd3)                                   │
│     └─ Initializes: Secure World, EL3                            │
│  4. OP-TEE OS (mtd4) ★                                           │
│     └─ Provides: Trusted Applications, Secure Storage            │
│  5. U-Boot (mtd5)                                                │
│     └─ Decrypts & Verifies: Kernel, DTS                          │
│  6. Linux Kernel (mtd6)                                          │
│     └─ Boots: Root filesystem                                     │
└──────────────────────────────────────────────────────────────────┘
```

---

## Troubleshooting

### Common Issues

1. **Boot failure after flashing**
   - Verify all encrypted binaries match the AES key
   - Check signature verification in loader
   - Ensure TEE partition is correctly flashed

2. **OP-TEE not initializing**
   - Verify memory region at 0x02000000 is reserved
   - Check kernel config includes TEE driver
   - Verify ATF is passing TEE info correctly

3. **Application memory issues**
   - Note: app partition reduced due to TEE
   - Monitor /proc/meminfo for available RAM
   - AI-ISP buffers require ~1.5MB minimum

---

## File List

```
cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_TEE_SECBOOT/
├── aes.txt                      # AES encryption key
├── ecdsa_priv.key              # ECDSA private key (signing)
├── ecdsa_pub.key               # ECDSA public key (verification)
├── ModelConfig.mk              # Build configuration
├── mtd_cfg.txt                 # UBI/JFFS2 configuration
├── nvt-audio.dtsi              # Audio configuration
├── nvt-basic.dtsi              # Basic CPU/timer config
├── nvt-clock.dtsi              # Clock framework
├── nvt-display.dtsi            # Display configuration
├── nvt-evb.dts                 # Main device tree
├── nvt-gpio.dtsi               # GPIO configuration
├── nvt-i2c.dtsi                # I2C configuration
├── nvt-info.dtsi               # NVT info/settings
├── nvt-media.dtsi              # Media drivers
├── nvt-mem-tbl.dtsi            # Memory layout (with TEE)
├── nvt-nvtpack.dtsi            # Firmware packaging (with TEE)
├── nvt-peri-dev.dtsi           # Peripheral devices
├── nvt-peri.dtsi               # Peripheral config
├── nvt-rtc.dtsi                # RTC configuration
├── nvt-storage-partition.dtsi  # Flash partitions (with TEE)
├── nvt-top.dtsi                # Pinmux configuration
├── README.md                   # This file
├── rsa_priv.txt                # RSA private key (backup)
└── rsa_pub.txt                 # RSA public key (backup)
```

---

## Version History

| Date | Version | Description |
|------|---------|-------------|
| 2026-02-20 | 1.0.0 | Initial TEE SECBOOT configuration |

---

## References

- Novatek NS02302 SDK Documentation
- OP-TEE Documentation: https://optee.readthedocs.io/
- ARM Trusted Firmware: https://trustedfirmware-a.readthedocs.io/
