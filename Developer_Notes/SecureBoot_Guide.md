---

## Build Configurations

This SDK supports two build modes:

| Mode | Description | Use Case |
|------|-------------|----------|
| **Normal Boot** | Unencrypted firmware | Development, debugging |
| **Secure Boot** | Encrypted + signed firmware | Production deployment |

---

## Normal Boot Build (Development)

### Step 1: Build Normal Loader

```bash
cd ns02302_ldr/Project/Model

# Ensure ModelConfig_EMU_EVB.txt has:
# SECUREBOOT = Normal
# SIGNATUREMETHOD = RSA2048 (or any, not used in normal mode)
# ENCRYPTDATA = Normal

cd ../../MakeCommon
make clean
make release
```

**Output:** `ns02302_ldr/ARC/LD98538A.bin`

### Step 2: Build Normal SDK Firmware

```bash
cd ns02302_linux_sdk
source build/envsetup.sh

# Select NORMAL configuration (no SECBOOT suffix)
lunch cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP

make clean
make all
```

**Output:** `ns02302_linux_sdk/output/packed/FW98538A.bin`

### Step 3: Flash Normal Firmware



## Secure Boot Build (Production)

### Prerequisites

Before building secure boot firmware, you need:

1. **RSA-2048 Key Pair** - For digital signature
2. **AES-128 Key** - For encryption
3. **eFuse Programmed Device** - Keys must be burned to eFuse first

### Security Architecture

| Component | Protection |
|-----------|------------|
| Device Tree (FDT) | AES-128 encrypted + RSA-2048 signed |
| ARM Trusted Firmware (ATF) | AES-128 encrypted + RSA-2048 signed |
| U-Boot | AES-128 encrypted + RSA-2048 signed |
| Linux Kernel | AES-128 encrypted + RSA-2048 signed |
| Rootfs | Not encrypted |
| App partition | Not encrypted |

### Boot Chain

```
ROM (Mask ROM) → Loader → ATF (BL31) → U-Boot → Linux Kernel
     ↓              ↓         ↓           ↓          ↓
  Verifies      Decrypts   Verified   Decrypted  Decrypted
  Loader Sig    ATF+UBoot  by ROM     by Loader  by U-Boot
```

---

## Secure Boot Implementation Steps

### Phase 1: Key Generation

#### Step 1.1: Generate RSA-2048 Key Pair


#### Step 1.2: Generate AES-128 Key



#### Step 1.3: Compute RSA N Hash for eFuse


#### Step 1.4: Backup Keys (CRITICAL!)


> **WARNING:** Store these keys securely! Lost keys = bricked devices!

---

### Phase 2: Configure and Build Loader with Keys

#### Step 2.1: Prepare AES Key File

The AES key file MUST have a specific format with CRLF line endings:

```bash
# Create properly formatted aes.txt
AES_KEY=$(cat aes_key_raw.txt)
printf '%s\r\n#%s\r\n#%s\r\n#%s\r\n#%s\r\n#%s\r\n#%s\r\n' \
    "$AES_KEY" "$AES_KEY" "$AES_KEY" "$AES_KEY" "$AES_KEY" "$AES_KEY" "$AES_KEY" \
    > ns02302_ldr/Tools/Bin/aes.txt
```

> **CRITICAL:** Single-line format will cause `enc size 0xFFFFFFFF` error!

#### Step 2.2: Prepare RSA Key Files


#### Step 2.3: Configure Loader for Secure Boot

Edit `ns02302_ldr/Project/Model/ModelConfig_EMU_EVB.txt`:

```makefile
# Enable secure boot
SECUREBOOT = Secure

# Signature method
SIGNATUREMETHOD = RSA2048

# Enable AES encryption
ENCRYPTDATA = AES128

# Decrypt all boot components
SECUREBOOT_DECRYPT_UBOOT = Secure
SECUREBOOT_DECRYPT_BL31 = Secure
SECUREBOOT_DECRYPT_FDT = Secure

# Disable OPTEE if not using TEE
SECUREBOOT_DECRYPT_OPTEE = Normal
```

#### Step 2.4: Build Secure Loader

```bash
cd ns02302_ldr/MakeCommon
make clean
make release
```

**Output:** `ns02302_ldr/ARC/LD98538A.bin` (contains embedded keys for eFuse programming)

---

### Phase 3: Program eFuse Keys (ONE-TIME ONLY!)

> **WARNING:** eFuse programming is IRREVERSIBLE! Triple-check all keys before proceeding!

#### Step 3.1: Flash Loader and Boot

#### Step 3.2: Program Keys to eFuse

At the `nvt:` loader prompt:

```bash
# Write AES key to slot 0
nvt_write_key 0

# Write RSA N Hash (low 128 bits) to slot 1
nvt_write_key 1

# Write RSA N Hash (high 128 bits) to slot 2
nvt_write_key 2
```

#### Step 3.3: Verify Keys Written

```bash
# Read back keys to verify
nvt_read_key_set 0
nvt_read_key_set 1
nvt_read_key_set 2

# Check programming status
nvt_secure_en quary
```

Expected output:
```
is_1st_key_programmed()=1
is_2nd_key_programmed()=1
is_3rd_key_programmed()=1
```

#### Step 3.4: Enable Secure Boot (IRREVERSIBLE!)

```bash
# Enable data encryption
nvt_secure_en data_encrypt

# Enable RSA signature verification
nvt_secure_en rsa_en

# Enable RSA checksum
nvt_secure_en rsa_chk_en

# FINAL STEP - Enable secure boot (NO GOING BACK!)
nvt_secure_en secure_en
```

#### Step 3.5: Lock AES Key (Recommended)

```bash
# Prevent AES key from being read
nvt_read_lock 0
```

#### Step 3.6: Verify Secure Boot Enabled

```bash
nvt_secure_en quary
```

Expected output:
```
is_secure_enable()=1
is_data_area_encrypted()=1
is_signature_rsa()=1
is_signature_chsum_enable()=1
is_1st_key_programmed()=1
is_2nd_key_programmed()=1
is_3rd_key_programmed()=1
is_1st_key_read_lock()=1
```

---

### Phase 4: Build and Flash Encrypted Firmware

#### Step 4.1: Configure SDK Key Files

Copy the SAME keys to SDK config directory:

```bash
SDK_SECBOOT_DIR="ns02302_linux_sdk/configs/Linux/cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT"

# Copy AES key (same format as loader)
cp ns02302_ldr/Tools/Bin/aes.txt ${SDK_SECBOOT_DIR}/

# Copy RSA keys
cp ns02302_ldr/Tools/Bin/rsa_pub.txt ${SDK_SECBOOT_DIR}/
cp ns02302_ldr/Tools/Bin/rsa_priv.txt ${SDK_SECBOOT_DIR}/
```

#### Step 4.2: Verify SDK Configuration

Check `${SDK_SECBOOT_DIR}/nvt-info.dtsi` has:

```dts
NVT_CFG_UBOOT_CFG = "nvt-ns02302_a64_nand_secboot_defconfig";
NVT_OPTEE_INSTALL = "DISABLE";
ENCRYPT_MSG = "ENCRYPT_MSG_ON";
ENCRYPT_DTS = "ENCRYPT_DTS_ON";
ENCRYPT_ATF = "ENCRYPT_ATF_ON";
ENCRYPT_OPTEE = "ENCRYPT_OPTEE_OFF";
ENCRYPT_UBOOT = "ENCRYPT_UBOOT_ON";
ENCRYPT_KERNEL = "ENCRYPT_KERNEL_ON";
SIGNATUREMETHOD = "SIGNATUREMETHOD_RSA2048";
```

#### Step 4.3: Verify nvtpack Configuration

Check `${SDK_SECBOOT_DIR}/nvt-nvtpack.dtsi` uses encrypted files:

```dts
id1 { partition_name = "fdt";    source_file = "nvt-evb.encrypted.bin"; };
id3 { partition_name = "atf";    source_file = "atf.encrypted.bin"; };
id4 { partition_name = "uboot";  source_file = "u-boot.encrypted.bin"; };
id6 { partition_name = "kernel"; source_file = "Image.encrypted.bin"; };
```

#### Step 4.4: Build Encrypted Firmware

```bash
cd ns02302_linux_sdk
source build/envsetup.sh

# Select SECBOOT configuration
lunch cfg_538A_ETERNA_128MBNAND_RAMDISK_AIISP_SECBOOT

make clean
make all
```

#### Step 4.5: Verify Encrypted Files Generated

```bash
ls -la output/*.encrypted.bin
```

Expected files:
- `atf.encrypted.bin`
- `Image.encrypted.bin`
- `nvt-evb.encrypted.bin`
- `u-boot.encrypted.bin`

#### Step 4.6: Flash Encrypted Firmware



### Phase 5: Verify Secure Boot Working

Check boot logs for these messages:

| Log Message | Meaning |
|-------------|---------|
| `Verify encrypted dts PASS` | Device tree signature verified |
| `nvt_decrypt_aes PASS` | Kernel decrypted successfully |
| `NOTICE: BL31: v2.2(release)` | ATF loaded and running |
| `U-Boot 2021.10` | U-Boot decrypted and running |
| `Starting kernel ...` | Complete boot chain verified |

---

## Configuration Files Reference

### Loader Configuration

| File | Path | Purpose |
|------|------|---------|
| ModelConfig_EMU_EVB.txt | `ns02302_ldr/Project/Model/` | Loader build settings |
| aes.txt | `ns02302_ldr/Tools/Bin/` | AES key (CRLF format!) |
| rsa_pub.txt | `ns02302_ldr/Tools/Bin/` | RSA public key |
| rsa_priv.txt | `ns02302_ldr/Tools/Bin/` | RSA private key |

### SDK Configuration

| File | Path | Purpose |
|------|------|---------|
| nvt-info.dtsi | `configs/Linux/cfg_*_SECBOOT/` | Encryption settings |
| nvt-nvtpack.dtsi | `configs/Linux/cfg_*_SECBOOT/` | Firmware pack config |
| nvt-storage-partition.dtsi | `configs/Linux/cfg_*_SECBOOT/` | Partition layout |
| aes.txt | `configs/Linux/cfg_*_SECBOOT/` | AES key for SDK |
| rsa_pub.txt | `configs/Linux/cfg_*_SECBOOT/` | RSA public key |
| rsa_priv.txt | `configs/Linux/cfg_*_SECBOOT/` | RSA private key |

---

## Troubleshooting

### Error: `enc size 0xFFFFFFFF`

**Cause:** AES key file has wrong format

**Solution:** Recreate aes.txt with multi-line CRLF format:
```bash
printf 'KEY\r\n#KEY\r\n#KEY\r\n#KEY\r\n#KEY\r\n#KEY\r\n#KEY\r\n' > aes.txt
```

### Error: `nvtpack partition name 'teeos' but 'uboot' is expected`

**Cause:** Partition mismatch - SECBOOT config has OPTEE but NORMAL doesn't

**Solution:** Set `NVT_OPTEE_INSTALL = "DISABLE"` and `ENCRYPT_OPTEE = "ENCRYPT_OPTEE_OFF"`

### Error: `leb not matched`

**Cause:** MTD configuration mismatch

**Solution:** Copy `mtd_cfg.txt` from NORMAL config to SECBOOT config

### Error: `key field not empty`

**Cause:** eFuse slot already programmed

**Solution:** This is expected - eFuse can only be written once. Verify the key is correct with `nvt_read_key_set X`

### Boot fails after enabling secure boot

**Cause:** Keys in firmware don't match eFuse keys

**Solution:** Ensure the same keys are used in:
1. `ns02302_ldr/Tools/Bin/aes.txt`
2. `configs/Linux/cfg_*_SECBOOT/aes.txt`
3. eFuse (programmed with `nvt_write_key`)

---

## Security Best Practices

1. **Unique Production Keys:** Never use development keys in production
2. **HSM Storage:** Store production keys in Hardware Security Module
3. **Key Rotation:** Generate new keys for each product batch
4. **Access Control:** Limit access to key files
5. **Audit Trail:** Log all key operations
6. **Backup:** Maintain secure offline backup of all keys
7. **JTAG Disable:** Consider `nvt_jtag_disable` for production
8. **Read Lock:** Lock all eFuse key slots after verification

---

## Related Documentation

- [SECURE_BOOT_IMPLEMENTATION_GUIDE.md](SECURE_BOOT_IMPLEMENTATION_GUIDE.md) - Detailed implementation guide
- [SECURE_BOOT_VERIFICATION_REPORT.md](SECURE_BOOT_VERIFICATION_REPORT.md) - Test report with annotated logs