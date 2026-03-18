# Novatek NT98538/NT98539 Firmware Build Guide

## Overview

This guide covers building firmware for Eterna IP cameras using Novatek NT98538/NT98539 SoC. It explains the differences between:

1. **Normal Firmware** - Standard build without encryption
2. **Secure Boot Firmware** - Encrypted firmware with signature verification
3. **TEE + Secure Boot Firmware** - Full security with OP-TEE trusted execution environment

---

## Directory Structure

```
ipcamera_fw_nvt/
├── ns02302_ldr/                    # Loader (bootloader) source
│   └── Project/Model/
│       └── ModelConfig_EMU_512MB_TB_eterna.txt   # Loader config
│
└── ns02302_linux_sdk/              # Linux SDK
    ├── configs/Linux/
    │   ├── cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP/           # Normal
    │   ├── cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT/   # Secure Boot
    │   └── cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_TEE_SECBOOT/  # TEE + Secure Boot
    ├── BSP/
    │   ├── optee/                  # OP-TEE components
    │   ├── atf/                    # ARM Trusted Firmware
    │   └── u-boot/                 # U-Boot bootloader
    └── build/                      # Build system
```

---

## Build Configurations Comparison

| Feature | Normal | Secure Boot | TEE + Secure Boot |
|---------|--------|-------------|-------------------|
| Config Name | `cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP` | `cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT` | `cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_TEE_SECBOOT` |
| Loader Encryption | No | Yes (AES128) | Yes (AES128) |
| U-Boot Encryption | No | Yes | Yes |
| Kernel/FDT Encryption | No | Yes | Yes |
| OP-TEE | No | No | Yes |
| Secure Key Storage | No | No | Yes |
| Signature Method | N/A | RSA2048/ECDSA256 | RSA2048 |

---

## Part 1: Loader Configuration

### Loader Config File Location
```
ns02302_ldr/Project/Model/ModelConfig_EMU_512MB_TB_eterna.txt
```

### Key Loader Settings

#### For Normal Firmware (No Encryption)
```ini
# Security disabled
SECUREBOOT = Normal

# These are ignored when SECUREBOOT = Normal
SIGNATUREMETHOD = ECDSA256
ENCRYPTDATA = Normal
SECUREBOOT_DECRYPT_UBOOT = Normal
SECUREBOOT_DECRYPT_OPTEE = Normal
SECUREBOOT_DECRYPT_BL31 = Normal
SECUREBOOT_DECRYPT_FDT = Normal
```

#### For Secure Boot / TEE Firmware (Encrypted)
```ini
# Enable secure boot
SECUREBOOT = Secure

# Signature method (RSA2048 recommended for TEE)
SIGNATUREMETHOD = RSA2048

# Encrypt data area with AES128
ENCRYPTDATA = AES128

# Decrypt all boot components
SECUREBOOT_DECRYPT_UBOOT = Secure
SECUREBOOT_DECRYPT_OPTEE = Secure
SECUREBOOT_DECRYPT_BL31 = Secure
SECUREBOOT_DECRYPT_FDT = Secure
```

### Building the Loader

```bash
cd ns02302_ldr/Project/Model

# Edit ModelConfig_EMU_512MB_TB_eterna.txt as needed

# Build loader
cd ../..
make MODEL=Loader538_Data clean
make MODEL=Loader538_Data release

# Output: Project/Model/Loader538_Data/Release/LD98538A.bin
```

---

## Part 2: Firmware Build Process

### Step 1: Select Configuration

```bash
cd ns02302_linux_sdk

# For Normal Firmware
make cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP

# For Secure Boot (without TEE)
make cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT

# For TEE + Secure Boot (recommended for production)
make cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_TEE_SECBOOT
```

### Step 2: Build All Components

```bash
# Clean previous build (optional but recommended when switching configs)
make clean

# Build everything
make all

# This builds:
# - U-Boot (encrypted if secure boot)
# - Linux Kernel
# - Root filesystem
# - ATF (ARM Trusted Firmware)
# - OP-TEE OS (if TEE config)
# - OP-TEE Client libraries
```

### Step 3: Package Firmware

```bash
# Create final firmware image
make pack

# Output location:
# - FW98538A.bin (for flashing)
# - FW98538T.bin (for SD card update)
```

---

## Part 3: TEE-Specific Configuration

### Memory Layout (TEE + Secure Boot)

The TEE configuration reserves memory for OP-TEE:

| Component | Address | Size |
|-----------|---------|------|
| ATF (BL31) | 0x01F00000 | 1MB |
| TEE OS | 0x02000000 | 8MB |
| Non-Secure Memory | 0x02800000 | 4MB |

### Device Tree Requirements

The config includes a firmware node for OP-TEE driver:

```dts
// In nvt-mem-tbl.dtsi
firmware {
    optee {
        compatible = "linaro,optee-tz";
        method = "smc";
    };
};
```

### Kernel Config

TEE builds use `ns02302_ipc_a64_tee_evb_defconfig_release` which includes:
- `CONFIG_OPTEE=y` - OP-TEE driver
- `CONFIG_TEE=y` - TEE subsystem
- `CONFIG_FILE_LOCKING=y` - Required for secure storage

### OP-TEE Init Script

The `S20_optee` script auto-starts tee-supplicant:
- Waits for `/mnt/app` mount
- Creates storage at `/mnt/app/tee`
- Starts `tee-supplicant` daemon

---

## Part 4: Security Keys

### Key Files in Config Directory

| File | Purpose | Format |
|------|---------|--------|
| `aes.txt` | AES-128 encryption key | 32 hex chars |
| `rsa_priv.txt` | RSA-2048 private key | PEM format |
| `rsa_pub.txt` | RSA-2048 public key | PEM format |
| `ecdsa_priv.key` | ECDSA-256 private key | PEM format |
| `ecdsa_pub.key` | ECDSA-256 public key | PEM format |

### Key Generation (for production)

```bash
# Generate new AES key
openssl rand -hex 16 > aes.txt

# Generate RSA-2048 key pair
openssl genrsa -out rsa_priv.pem 2048
openssl rsa -in rsa_priv.pem -pubout -out rsa_pub.pem

# Generate ECDSA-256 key pair
openssl ecparam -genkey -name prime256v1 -out ecdsa_priv.key
openssl ec -in ecdsa_priv.key -pubout -out ecdsa_pub.key
```

> ⚠️ **WARNING**: The keys in the repository are TEST KEYS from Novatek SDK. 
> Generate new keys for production devices!

---

## Part 5: Flashing Firmware

### Flash Complete System (Loader + Firmware)

Using Novatek Secure Boot Utility:
1. Connect device via USB
2. Put device in download mode
3. Flash loader: `LD98538A.bin`
4. Flash firmware: `FW98538A.bin`

### SD Card Update (Firmware Only)

```bash
# Copy to SD card root
cp FW98538T.bin /media/sdcard/

# Insert SD card and reboot device
# Device will auto-update firmware
```

---

## Part 6: Verification

### Verify OP-TEE is Running

```bash
# Check TEE device
ls -la /dev/tee*

# Check tee-supplicant
pgrep -a tee-supplicant

# Run hello world test
optee_example_hello_world
```

Expected output:
```
Invoking TA to increment 42
TA incremented value to 43
```

### Check Boot Log for TEE

```bash
dmesg | grep -i "optee\|tee"
```

Expected:
```
optee: probing for conduit method.
optee: revision 3.x
```

---

## Quick Reference

### Build Normal Firmware
```bash
cd ns02302_linux_sdk
make cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP
make all pack
```

### Build TEE + Secure Boot Firmware
```bash
# 1. Configure loader for secure boot
vim ../ns02302_ldr/Project/Model/ModelConfig_EMU_512MB_TB_eterna.txt
# Set: SECUREBOOT = Secure, ENCRYPTDATA = AES128, etc.

# 2. Build loader
cd ../ns02302_ldr
make MODEL=Loader538_Data release

# 3. Build firmware
cd ../ns02302_linux_sdk
make cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_TEE_SECBOOT
make all pack
```

### Output Files
| File | Description |
|------|-------------|
| `ns02302_ldr/.../LD98538A.bin` | Loader binary |
| `ns02302_linux_sdk/FW98538A.bin` | Firmware for flashing |
| `ns02302_linux_sdk/FW98538T.bin` | Firmware for SD card update |

---

## Troubleshooting

### OP-TEE not starting
1. Check kernel has `CONFIG_OPTEE=y`
2. Verify Device Tree has firmware node
3. Check ATF built with `SPD=opteed`

### Secure boot fails
1. Verify loader config matches firmware config
2. Check keys are consistent across loader and firmware
3. Ensure OTP is programmed with matching key hash

### tee-supplicant crashes
1. Check `/mnt/app` is mounted (persistent storage)
2. Verify `libteec.so` is in `/usr/lib/`
3. Check `/dev/tee0` exists

---

## References

- Novatek NS02302 SDK Documentation
- OP-TEE Documentation: https://optee.readthedocs.io/
- ARM Trusted Firmware: https://trustedfirmware-a.readthedocs.io/
