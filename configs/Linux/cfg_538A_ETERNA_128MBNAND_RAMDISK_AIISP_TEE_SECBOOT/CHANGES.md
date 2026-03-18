# CHANGES: cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_TEE_SECBOOT

## Summary

This document details all changes made to create the OP-TEE enabled secure boot
configuration for ETERNA camera with AI-ISP support.

**Base Configuration**: `cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT`
**Reference TEE Config**: `cfg_538_IPC_NAND_RAMDISK_TEE_SECBOOT_EVB`

---

## 1. Flash Partition Changes (nvt-storage-partition.dtsi)

### Added Partition
| Partition | Offset | Size | Purpose |
|-----------|--------|------|---------|
| **teeos** | 0x0100000 | 0x0100000 (1MB) | OP-TEE OS image |

### Shifted Partitions
| Partition | Original Offset | New Offset | Reason |
|-----------|-----------------|------------|--------|
| uboot | 0x0100000 | 0x0200000 | Make room for teeos |
| uenv | 0x01E0000 | 0x02E0000 | Shifted by 1MB |
| kernel | 0x0220000 | 0x0320000 | Shifted by 1MB |
| rootfs | 0x0720000 | 0x0820000 | Shifted by 1MB |
| rootfs1 | 0x2720000 | 0x2820000 | Shifted by 1MB |
| app | 0x7020000 | 0x6A20000 | Shifted by 1MB |

### Adjusted Sizes
| Partition | Original Size | New Size | Change |
|-----------|---------------|----------|--------|
| rootfs1 | 0x4900000 (73MB) | 0x4200000 (66MB) | -7MB |
| app | 0x0FE0000 (15.9MB) | 0x15E0000 (21.9MB) | +6MB |

### Total Verification
```
0x40000 + 0x40000 + 0x40000 + 0x40000 + 0x100000 + 0xE0000 + 0x40000 + 
0x500000 + 0x2000000 + 0x4200000 + 0x15E0000 = 0x8000000 (128MB) ✓
```

---

## 2. Memory Layout Changes (nvt-mem-tbl.dtsi)

### Added Memory Regions (nvt_memory_cfg)
| Region | Address | Size | Purpose |
|--------|---------|------|---------|
| **teeos** | 0x02000000 | 0x00C00000 (12MB) | OP-TEE OS runtime |
| **nsmem** | 0x02C00000 | 0x00400000 (4MB) | Non-secure shared memory |

### Shifted Memory Regions
| Region | Original Address | New Address |
|--------|------------------|-------------|
| linuxtmp | 0x02000000 | 0x03000000 |

### Adjusted Sizes
| Region | Original Size | New Size |
|--------|---------------|----------|
| linuxtmp | 0x0E000000 (224MB) | 0x0D000000 (208MB) |

### Added Reserved Memory
```c
tee_reserved: optee@0x02000000 {
    reg = <0x0 0x02000000 0x0 0x00C00000>;
    no-map;
};
```

### AI-ISP Configuration (PRESERVED)
All AI-ISP settings retained from ETERNA configuration:
- `ai-maxpath-cfg`
- `ai-pool-cfg`
- `ai-isp-cfg`

---

## 3. Firmware Packaging Changes (nvt-nvtpack.dtsi)

### Added Index Entry
```c
id4  { partition_name = "teeos"; source_file = "tee.encrypted.bin"; };
```

### Re-indexed Entries
| Old ID | New ID | Partition |
|--------|--------|-----------|
| id4 | id5 | uboot |
| id5 | id6 | uenv |
| id6 | id7 | kernel |
| id7 | id8 | rootfs |
| id8 | id9 | rootfs1 |
| id9 | id10 | app |

---

## 4. Model Configuration Changes (ModelConfig.mk)

### New Settings Added
```makefile
BOARD_TEEOS_ADDR = 0x02000000
BOARD_TEEOS_SIZE = 0x00C00000
BOARD_NSMEM_ADDR = 0x02C00000
BOARD_NSMEM_SIZE = 0x00400000
```

### Modified Settings
| Setting | Before | After |
|---------|--------|-------|
| NVT_OPTEE_INSTALL | DISABLE | ENABLE |
| NVT_CFG_KERNEL_CFG | ns02302_ipc_a64_evb_defconfig_release | ns02302_ipc_a64_tee_evb_defconfig_release |
| NVT_CFG_UBOOT_CFG | nvt-ns02302_a64_nand_secboot_defconfig | nvt-ns02302_a64_tee_secboot_nand_defconfig |
| ENCRYPT_OPTEE | ENCRYPT_OPTEE_OFF | ENCRYPT_OPTEE_ON |
| BOARD_LINUXTMP_ADDR | 0x02000000 | 0x03000000 |
| BOARD_LINUXTMP_SIZE | 0x0E000000 | 0x0D000000 |

---

## 5. Info Configuration Changes (nvt-info.dtsi)

### Modified Settings
```c
NVT_OPTEE_INSTALL = "ENABLE";
NVT_CFG_KERNEL_CFG = "ns02302_ipc_a64_tee_evb_defconfig_release";
NVT_CFG_UBOOT_CFG = "nvt-ns02302_a64_tee_secboot_nand_defconfig";
ENCRYPT_OPTEE = "ENCRYPT_OPTEE_ON";
```

---

## 6. MTD Configuration Changes (mtd_cfg.txt)

### Updated UBI LEB Counts
| Parameter | Before | After | Reason |
|-----------|--------|-------|--------|
| ROOTFS_UBI_MAX_LEB_COUNT | 359 | 288 | rootfs unchanged at 32MB |
| ROOTFS_UBI_RW_MAX_LEB_COUNT | 548 | 470 | rootfs1 reduced to 66MB |
| ROOTFS_UBI_APP_MAX_LEB_COUNT | 91 | 125 | app increased to 21.9MB |
| ROOTFS_JFFS2_SIZE | 0x3160000 | 0x2000000 | Match rootfs (32MB) |
| ROOTFS_JFFS2_RW_SIZE | 0x2500000 | 0x4200000 | Match rootfs1 (66MB) |
| ROOTFS_JFFS2_APP_SIZE | 0x01E0000 | 0x15E0000 | Match app (21.9MB) |

---

## 7. Security Key Files

### Copied from ETERNA SECBOOT
- `aes.txt` - AES-128 encryption key
- `rsa_priv.txt` - RSA-2048 private key
- `rsa_pub.txt` - RSA-2048 public key

### Copied from TEE SECBOOT EVB
- `ecdsa_priv.key` - ECDSA private key (primary for ECDSA signing)
- `ecdsa_pub.key` - ECDSA public key

---

## 8. Preserved Files (No Changes)

The following files were copied unchanged from ETERNA SECBOOT:
- nvt-evb.dts
- nvt-audio.dtsi
- nvt-basic.dtsi
- nvt-clock.dtsi
- nvt-display.dtsi
- nvt-gpio.dtsi
- nvt-i2c.dtsi
- nvt-media.dtsi
- nvt-peri.dtsi
- nvt-peri-dev.dtsi
- nvt-rtc.dtsi
- nvt-top.dtsi

---

## 9. Boot Chain Impact

### Before (SECBOOT without TEE)
```
ROM → Loader → ATF → U-Boot → Kernel → Linux
```

### After (TEE_SECBOOT)
```
ROM → Loader → ATF → OP-TEE → U-Boot → Kernel → Linux
                        ↓
               Secure World (EL1-S)
```

---

## 10. Verification Checklist

- [x] Flash partitions sum to 128MB
- [x] Memory regions don't overlap
- [x] TEE reserved memory marked as no-map
- [x] NVTPack includes tee.encrypted.bin
- [x] AI-ISP configuration preserved
- [x] Encryption enabled for TEE
- [x] Kernel and U-Boot configs point to TEE variants
- [x] Security keys present (AES, RSA, ECDSA)

---

## 11. Risk Assessment

| Risk | Mitigation |
|------|------------|
| Boot failure due to partition shift | Verified all offsets contiguous |
| Memory overlap with TEE | Added 12MB TEE reserved region |
| AI-ISP buffer conflicts | Preserved all AI-ISP config from ETERNA |
| Reduced rootfs1 space | Compensated by increasing app partition |
| Key mismatch | Using same AES key as existing SECBOOT |

---

*Document created: 2026-02-20*
*Configuration: cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_TEE_SECBOOT*
