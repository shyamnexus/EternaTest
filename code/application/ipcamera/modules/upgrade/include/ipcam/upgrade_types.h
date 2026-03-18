/**
 * @file upgrade_types.h
 * @brief Firmware upgrade type definitions for secure boot firmware
 * 
 * Defines structures compatible with Novatek NVTPACK format and
 * secure boot extensions (RSA signature, AES encryption).
 */

#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <functional>

namespace ipcam::upgrade {

// =============================================================================
// NVTPACK Constants (from SDK nvtpack.h)
// =============================================================================

constexpr uint32_t NVTPACK_CHKSUM_HDR_VERSION = 0x16040719U;
constexpr uint32_t NVTPACK_FW_HDR2_VERSION = 0x16071515U;

// GUID for NVTPACK_FW_HDR2: {D6012E07-10BC-4F91-B28A-352F82261A50}
constexpr std::array<uint8_t, 16> NVTPACK_FW_HDR2_GUID = {
    0x07, 0x2E, 0x01, 0xD6, 0xBC, 0x10, 0x91, 0x4F,
    0xB2, 0x8A, 0x35, 0x2F, 0x82, 0x26, 0x1A, 0x50
};

// Partition types (from emb_partition_info.h)
enum class PartitionType : uint16_t {
    Unknown     = 0x00,
    Loader      = 0x01,  // Bootloader (partition[0])
    ModelExt    = 0x02,  // FDT/Device tree (partition[1])
    UITron      = 0x03,
    ECos        = 0x04,
    UBoot       = 0x05,
    Linux       = 0x06,  // Kernel
    DSP         = 0x07,
    PStore      = 0x08,
    FAT         = 0x09,
    ExFAT       = 0x0A,
    RootFS      = 0x0B,
    RamFS       = 0x0C,
    UEnv        = 0x0D,
    MBR         = 0x0E,
    NuttX       = 0x0F,
    RTOS        = 0x10,
    TeeOS       = 0x11,
    BL31        = 0x12,  // ATF
    App         = 0x80,  // Application partition (User0)
    User1       = 0x81,
    User2       = 0x82,
    User3       = 0x83,
};

// =============================================================================
// NVTPACK Structures (binary-compatible with SDK)
// =============================================================================

#pragma pack(push, 1)

/**
 * @brief NVTPACK firmware header version 2 (128 bytes)
 * Binary-compatible with NVTPACK_FW_HDR2 in nvtpack.h
 */
struct NvtpackFwHeader2 {
    uint8_t  guid[16];          // GUID: {D6012E07-10BC-4F91-B28A-352F82261A50}
    uint32_t version;           // NVTPACK_FW_HDR2_VERSION
    uint32_t headerSize;        // sizeof(NvtpackFwHeader2) = 128
    uint32_t totalPartitions;   // Total partition count
    uint32_t totalSize;         // Whole binary size
    uint32_t checksumMethod;    // 0 = standard checksum
    uint32_t checksumValue;     // Checksum of header
    uint32_t userData[4];       // User-defined data
    uint32_t reserved[18];      // Align to 128 bytes
};
static_assert(sizeof(NvtpackFwHeader2) == 128, "NvtpackFwHeader2 must be 128 bytes");

/**
 * @brief NVTPACK partition header (12 bytes)
 * Binary-compatible with NVTPACK_PARTITION_HDR
 */
struct NvtpackPartitionHeader {
    uint32_t offset;        // Offset from start of firmware
    uint32_t size;          // Size of partition data
    uint32_t partitionId;   // Partition ID (maps to PartitionType)
};
static_assert(sizeof(NvtpackPartitionHeader) == 12, "NvtpackPartitionHeader must be 12 bytes");

/**
 * @brief NVTPACK checksum header (64 bytes)
 * Binary-compatible with NVTPACK_CHKSUM_HDR
 */
struct NvtpackChecksumHeader {
    uint32_t fourCC;        // 'C','K','S','M'
    uint32_t version;       // NVTPACK_CHKSUM_HDR_VERSION
    uint32_t checksumMethod;
    uint32_t checksumValue;
    uint32_t dataOffset;    // Real data offset without this header
    uint32_t dataSize;      // Real data size
    uint32_t paddingSize;
    uint32_t embType;       // EMBTYPE_*
    uint32_t reserved[8];   // Align to 64 bytes
};
static_assert(sizeof(NvtpackChecksumHeader) == 64, "NvtpackChecksumHeader must be 64 bytes");

/**
 * @brief Secure boot signature structure
 * RSA-2048 signature is 256 bytes
 */
struct SecureBootSignature {
    static constexpr size_t RSA2048_SIZE = 256;
    static constexpr size_t SHA256_SIZE = 32;
    
    uint8_t signature[RSA2048_SIZE];  // RSA-2048 signature of SHA256 hash
};

/**
 * @brief AES-256 encryption context for secure boot
 */
struct SecureBootEncryption {
    static constexpr size_t AES_KEY_SIZE = 16;   // 128-bit key from aes.txt
    static constexpr size_t AES_IV_SIZE = 16;
    static constexpr size_t GCM_TAG_SIZE = 16;
    
    uint8_t key[AES_KEY_SIZE];
    uint8_t iv[AES_IV_SIZE];
};

#pragma pack(pop)

// =============================================================================
// High-Level Upgrade Types
// =============================================================================

/**
 * @brief Upgrade status codes
 */
enum class UpgradeStatus {
    Idle,
    Downloading,
    Validating,
    Staging,
    ReadyToReboot,
    Writing,    // Direct flash write in progress
    Rebooting,
    Failed,
};

/**
 * @brief Upgrade error codes
 */
enum class UpgradeError {
    None = 0,
    InvalidHeader,
    InvalidGuid,
    InvalidVersion,
    ChecksumMismatch,
    SignatureVerifyFailed,
    DecryptionFailed,
    PartitionMismatch,
    InsufficientMemory,
    MemoryAllocationFailed,
    FlashWriteFailed,
    ReadbackVerifyFailed,
    ServiceShutdownFailed,
    RebootFailed,
    DownloadFailed,
    FileOpenFailed,
    FileReadError,
    WriteError,
    FirmwareTooLarge,
    InvalidState,
    UpgradeInProgress,
    Timeout,
};

/**
 * @brief Partition validation result
 */
struct PartitionValidation {
    PartitionType type;
    uint32_t offset;
    uint32_t size;
    bool checksumValid;
    bool signatureValid;  // Only for secure boot enabled partitions
    bool decrypted;       // True if partition was encrypted and decrypted
};

/**
 * @brief Firmware validation result
 */
struct FirmwareValidation {
    bool valid;
    UpgradeError error;
    std::string errorMessage;
    
    // Header info
    uint32_t totalSize;
    uint32_t partitionCount;
    bool isSecureBoot;
    
    // Per-partition results
    std::vector<PartitionValidation> partitions;
    
    // Signature info (if secure boot)
    bool signaturePresent;
    bool signatureValid;
};

/**
 * @brief Physical memory allocation for firmware staging
 */
struct PhysicalMemoryBlock {
    uint64_t physicalAddress;   // Physical address (for DMA/flash tools)
    void*    virtualAddress;    // Virtual address (for application access)
    size_t   size;              // Allocated size
    bool     allocated;
};

/**
 * @brief Upgrade configuration written to /tmp/upgrade/upgradeinfo
 * This is passed to U-Boot/loader after reboot
 */
struct UpgradeHandoffInfo {
    uint64_t physicalAddress;   // Base physical address of firmware in RAM
    uint32_t totalSize;         // Total firmware size
    uint32_t partitionCrc[16];  // CRC per partition (max 16 partitions)
    uint32_t partitionOffset[16];
    uint32_t partitionSize[16];
    char     timestamp[32];     // Upgrade timestamp
    uint32_t flags;             // Upgrade flags (e.g., force, skip validation)
};

/**
 * @brief Upgrade progress callback
 */
using UpgradeProgressCallback = std::function<void(UpgradeStatus status, 
                                                    int progressPercent,
                                                    const std::string& message)>;

/**
 * @brief Firmware security mode
 */
enum class FirmwareSecurityMode {
    /// Accept any valid NVTPACK firmware (signed or unsigned)
    AcceptAny,
    
    /// Prefer signed firmware but accept unsigned (logs warning)
    PreferSecure,
    
    /// Require secure boot (reject unsigned firmware)
    RequireSecure
};

/**
 * @brief Upgrade configuration
 */
struct UpgradeConfig {
    // Memory settings
    size_t maxFirmwareSize = 128 * 1024 * 1024;  // 128MB max firmware
    
    // Security mode (recommended way to configure)
    FirmwareSecurityMode securityMode = FirmwareSecurityMode::AcceptAny;
    
    // Legacy security settings (kept for backward compatibility)
    bool requireSecureBoot = false;  // Use securityMode instead
    bool verifySignature = true;     // Verify RSA signature if present
    bool verifyChecksum = true;      // Verify per-partition checksum
    bool verifyReadback = true;      // Verify after flash (MD5)
    
    // Paths
    std::string upgradeInfoPath = "/tmp/upgrade/upgradeinfo";
    std::string upgradeStagingPath = "/tmp/upgrade/";
    
    // Timeouts (seconds)
    int serviceShutdownTimeout = 30;
    int downloadTimeout = 600;       // 10 minutes
    
    // RSA public key (from config)
    std::string rsaPublicKeyN;
    std::string rsaPublicKeyE;
    
    // AES key (from config, for secure boot)
    std::array<uint8_t, 16> aesKey;
    
    // Progress callback
    UpgradeProgressCallback progressCallback;
};

} // namespace ipcam::upgrade
