/**
 * @file nvt_flasher.cpp
 * @brief Standalone firmware flasher binary for Novatek SoC
 *
 * This is a minimal binary that runs AFTER the main application exits but BEFORE reboot.
 * It reads firmware from physical RAM (staged by the main app), parses NVTPACK format,
 * and writes each partition to the correct MTD device.
 *
 * Architecture (like reference design):
 *   1. Main app stages firmware in physical RAM
 *   2. Main app writes metadata to /tmp/upgrade/upgradeinfo
 *   3. Main app launches this flasher via script (background)
 *   4. Main app kills itself
 *   5. This flasher maps physical RAM, parses NVTPACK, writes to MTD
 *   6. This flasher reboots
 *
 * Safety:
 *   - Runs in isolation (main app is dead)
 *   - No network exposure
 *   - Minimal dependencies
 *   - Validates each partition before writing
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <mtd/mtd-user.h>
#include <errno.h>

// ============================================================================
// NVTPACK Structures (from Novatek SDK)
// ============================================================================

struct GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
};

// NVTPACK_FW_HDR2 - Main firmware header (128 bytes)
struct NvtpackFwHdr2 {
    GUID     guid;          // {D6012E07-10BC-4F91-B28A-352F82261A50}
    uint32_t version;       // NVTPACK_FW_HDR2_VERSION = 0x16071515
    uint32_t hdrSize;       // sizeof(NvtpackFwHdr2) = 128
    uint32_t totalRes;      // Total partition count
    uint32_t totalSize;     // Whole firmware size
    uint32_t chkMethod;     // 0 = checksum
    uint32_t chkValue;      // Checksum value
    uint32_t userData[4];   // User defined
    uint32_t reserved[18];  // Padding to 128 bytes
};

// Per-partition header
struct NvtpackPartitionHdr {
    uint32_t offset;        // Offset from start of firmware
    uint32_t size;          // Size of partition data
    uint32_t partitionId;   // EMB_TYPE (loader=1, uboot=5, linux=6, rootfs=11, etc.)
};

// NVTPACK_CHKSUM_HDR - Per-partition checksum header (64 bytes)
struct NvtpackChksumHdr {
    uint32_t fourCC;        // 'CKSM'
    uint32_t version;       // 0x16040719
    uint32_t chkMethod;     // 0 = checksum
    uint32_t chkValue;      // Checksum
    uint32_t dataOffset;    // Offset to real data
    uint32_t dataSize;      // Real data size
    uint32_t paddingSize;   // Padding for alignment
    uint32_t embType;       // Partition type
    uint32_t reserved[8];   // Padding to 64 bytes
};

// Partition types (from emb_partition_info.h)
#define EMBTYPE_UNKNOWN     0x00
#define EMBTYPE_LOADER      0x01
#define EMBTYPE_MODELEXT    0x02
#define EMBTYPE_UITRON      0x03
#define EMBTYPE_ECOS        0x04
#define EMBTYPE_UBOOT       0x05
#define EMBTYPE_LINUX       0x06
#define EMBTYPE_DSP         0x07
#define EMBTYPE_PSTORE      0x08
#define EMBTYPE_FAT         0x09
#define EMBTYPE_EXFAT       0x0A
#define EMBTYPE_ROOTFS      0x0B
#define EMBTYPE_RAMFS       0x0C
#define EMBTYPE_UENV        0x0D
#define EMBTYPE_MBR         0x0E
#define EMBTYPE_NUTTX       0x0F
#define EMBTYPE_RTOS        0x10
#define EMBTYPE_TEEOS       0x11
#define EMBTYPE_BL31        0x12

// Upgrade info passed from main application
struct UpgradeInfo {
    uint64_t physAddr;      // Physical address of firmware in RAM
    uint64_t size;          // Firmware size
    uint32_t crc[16];       // Per-partition CRCs (optional)
    char     timestamp[32]; // Upgrade timestamp
    uint32_t magic;         // Validation magic
    uint32_t version;       // Info structure version
};

#define UPGRADE_INFO_MAGIC   0x55504752  // "UPGR"
#define UPGRADE_INFO_VERSION 1

// ============================================================================
// Globals
// ============================================================================

static bool g_verbose = false;

// ============================================================================
// Logging
// ============================================================================

#define LOG(fmt, ...) do { \
    fprintf(stderr, "[FLASHER] " fmt "\n", ##__VA_ARGS__); \
    fsync(STDERR_FILENO); \
} while(0)

#define LOG_V(fmt, ...) do { \
    if (g_verbose) LOG(fmt, ##__VA_ARGS__); \
} while(0)

#define LOG_ERR(fmt, ...) LOG("ERROR: " fmt, ##__VA_ARGS__)

// ============================================================================
// MTD Partition Mapping
// ============================================================================

struct MtdMapping {
    uint32_t embType;
    const char* mtdDev;
    const char* name;
    bool critical;  // If true, failure aborts upgrade
};

// Map EMBTYPE to MTD device (based on /proc/mtd output)
static const MtdMapping g_mtdMap[] = {
    { EMBTYPE_LOADER,   "/dev/mtd0",  "loader",  true  },
    { EMBTYPE_MODELEXT, "/dev/mtd1",  "fdt",     true  },  // FDT is modelext
    { EMBTYPE_BL31,     "/dev/mtd3",  "atf",     true  },
    { EMBTYPE_UBOOT,    "/dev/mtd4",  "uboot",   true  },
    { EMBTYPE_UENV,     "/dev/mtd5",  "uenv",    false },
    { EMBTYPE_LINUX,    "/dev/mtd6",  "kernel",  true  },
    { EMBTYPE_ROOTFS,   "/dev/mtd7",  "rootfs",  true  },
    // mtd8 is rootfs1 (backup), mtd9 is app, mtd10 is all
    { 0, nullptr, nullptr, false }
};

static const MtdMapping* findMtdForPartition(uint32_t embType) {
    for (const auto* m = g_mtdMap; m->mtdDev; ++m) {
        if (m->embType == embType) {
            return m;
        }
    }
    return nullptr;
}

static const char* embTypeName(uint32_t type) {
    switch (type) {
        case EMBTYPE_LOADER:   return "LOADER";
        case EMBTYPE_MODELEXT: return "FDT/MODELEXT";
        case EMBTYPE_UBOOT:    return "UBOOT";
        case EMBTYPE_LINUX:    return "KERNEL";
        case EMBTYPE_ROOTFS:   return "ROOTFS";
        case EMBTYPE_BL31:     return "ATF/BL31";
        case EMBTYPE_TEEOS:    return "TEEOS";
        case EMBTYPE_UENV:     return "UENV";
        case EMBTYPE_RTOS:     return "RTOS";
        default:               return "UNKNOWN";
    }
}

// ============================================================================
// Checksum
// ============================================================================

static uint32_t calculateChecksum(const uint8_t* data, size_t len) {
    uint32_t sum = 0;
    const uint32_t* p = reinterpret_cast<const uint32_t*>(data);
    size_t words = len / 4;
    
    for (size_t i = 0; i < words; ++i) {
        sum += p[i];
    }
    
    // Handle remaining bytes
    size_t rem = len % 4;
    if (rem > 0) {
        uint32_t last = 0;
        memcpy(&last, data + (words * 4), rem);
        sum += last;
    }
    
    return sum;
}

// ============================================================================
// MTD Operations
// ============================================================================

static bool eraseMtd(const char* mtdDev, size_t size) {
    int fd = open(mtdDev, O_RDWR);
    if (fd < 0) {
        LOG_ERR("Failed to open %s: %s", mtdDev, strerror(errno));
        return false;
    }
    
    // Get MTD info
    mtd_info_t mtdInfo;
    if (ioctl(fd, MEMGETINFO, &mtdInfo) < 0) {
        LOG_ERR("MEMGETINFO failed for %s: %s", mtdDev, strerror(errno));
        close(fd);
        return false;
    }
    
    LOG_V("  MTD info: size=%u, erasesize=%u, type=%u", 
          mtdInfo.size, mtdInfo.erasesize, mtdInfo.type);
    
    // Erase blocks
    erase_info_t eraseInfo;
    eraseInfo.start = 0;
    eraseInfo.length = mtdInfo.erasesize;
    
    size_t eraseEnd = (size + mtdInfo.erasesize - 1) & ~(mtdInfo.erasesize - 1);
    if (eraseEnd > mtdInfo.size) {
        eraseEnd = mtdInfo.size;
    }
    
    while (eraseInfo.start < eraseEnd) {
        if (ioctl(fd, MEMERASE, &eraseInfo) < 0) {
            // Check if bad block (for NAND)
            if (errno == EIO) {
                LOG_V("  Bad block at 0x%x, skipping", eraseInfo.start);
            } else {
                LOG_ERR("MEMERASE failed at 0x%x: %s", eraseInfo.start, strerror(errno));
                close(fd);
                return false;
            }
        }
        eraseInfo.start += mtdInfo.erasesize;
    }
    
    close(fd);
    return true;
}

static bool writeMtd(const char* mtdDev, const uint8_t* data, size_t size) {
    // Erase first
    if (!eraseMtd(mtdDev, size)) {
        return false;
    }
    
    int fd = open(mtdDev, O_RDWR);
    if (fd < 0) {
        LOG_ERR("Failed to open %s for write: %s", mtdDev, strerror(errno));
        return false;
    }
    
    // Get MTD info for write size alignment
    mtd_info_t mtdInfo;
    if (ioctl(fd, MEMGETINFO, &mtdInfo) < 0) {
        LOG_ERR("MEMGETINFO failed: %s", strerror(errno));
        close(fd);
        return false;
    }
    
    // Write in page-sized chunks for NAND
    size_t writeSize = (mtdInfo.type == MTD_NANDFLASH) ? mtdInfo.writesize : 4096;
    if (writeSize == 0) writeSize = 2048;  // Default NAND page size
    
    size_t written = 0;
    while (written < size) {
        size_t chunk = size - written;
        if (chunk > writeSize) chunk = writeSize;
        
        // Pad last chunk if needed
        uint8_t padBuf[4096];
        const uint8_t* writePtr = data + written;
        
        if (chunk < writeSize && mtdInfo.type == MTD_NANDFLASH) {
            memset(padBuf, 0xFF, writeSize);
            memcpy(padBuf, writePtr, chunk);
            writePtr = padBuf;
            chunk = writeSize;
        }
        
        ssize_t ret = write(fd, writePtr, chunk);
        if (ret < 0) {
            LOG_ERR("Write failed at offset %zu: %s", written, strerror(errno));
            close(fd);
            return false;
        }
        
        written += ret;
    }
    
    fsync(fd);
    close(fd);
    
    return true;
}

// ============================================================================
// NVTPACK Parser
// ============================================================================

struct ParsedPartition {
    uint32_t embType;
    const uint8_t* data;
    size_t dataSize;
    uint32_t checksum;
    bool hasChecksum;
};

static bool parseNvtpack(const uint8_t* firmware, size_t firmwareSize,
                         ParsedPartition* partitions, int* partitionCount, int maxPartitions)
{
    *partitionCount = 0;
    
    // Check minimum size
    if (firmwareSize < sizeof(NvtpackFwHdr2)) {
        LOG_ERR("Firmware too small for header");
        return false;
    }
    
    // Parse main header
    const auto* hdr = reinterpret_cast<const NvtpackFwHdr2*>(firmware);
    
    // Verify magic/version
    if (hdr->version != 0x16071515) {
        LOG_ERR("Invalid NVTPACK version: 0x%08X (expected 0x16071515)", hdr->version);
        return false;
    }
    
    if (hdr->hdrSize != 128) {
        LOG_ERR("Invalid header size: %u (expected 128)", hdr->hdrSize);
        return false;
    }
    
    LOG("NVTPACK Header:");
    LOG("  Version: 0x%08X", hdr->version);
    LOG("  Partitions: %u", hdr->totalRes);
    LOG("  Total Size: %u bytes", hdr->totalSize);
    
    if (hdr->totalRes > (uint32_t)maxPartitions) {
        LOG_ERR("Too many partitions: %u (max %d)", hdr->totalRes, maxPartitions);
        return false;
    }
    
    // Partition headers follow immediately after main header
    const auto* partHdrs = reinterpret_cast<const NvtpackPartitionHdr*>(firmware + sizeof(NvtpackFwHdr2));
    
    for (uint32_t i = 0; i < hdr->totalRes; ++i) {
        const auto& ph = partHdrs[i];
        
        LOG_V("Partition %u: offset=0x%08X size=%u type=%u (%s)",
              i, ph.offset, ph.size, ph.partitionId, embTypeName(ph.partitionId));
        
        if (ph.offset + ph.size > firmwareSize) {
            LOG_ERR("Partition %u exceeds firmware size", i);
            return false;
        }
        
        const uint8_t* partData = firmware + ph.offset;
        size_t partDataSize = ph.size;
        uint32_t embType = ph.partitionId;
        bool hasChecksum = false;
        uint32_t checksum = 0;
        
        // Check for NVTPACK_CHKSUM_HDR at start of partition data
        if (ph.size >= sizeof(NvtpackChksumHdr)) {
            const auto* chkHdr = reinterpret_cast<const NvtpackChksumHdr*>(partData);
            
            // Check 'CKSM' fourCC
            if (chkHdr->fourCC == 0x4D534B43) {  // 'CKSM' little-endian
                LOG_V("  Has CHKSUM header: dataOffset=%u dataSize=%u embType=%u",
                      chkHdr->dataOffset, chkHdr->dataSize, chkHdr->embType);
                
                // Skip the checksum header to get real data
                partData = firmware + ph.offset + chkHdr->dataOffset;
                partDataSize = chkHdr->dataSize;
                embType = chkHdr->embType;  // Use embType from checksum header
                hasChecksum = true;
                checksum = chkHdr->chkValue;
            }
        }
        
        partitions[*partitionCount].embType = embType;
        partitions[*partitionCount].data = partData;
        partitions[*partitionCount].dataSize = partDataSize;
        partitions[*partitionCount].checksum = checksum;
        partitions[*partitionCount].hasChecksum = hasChecksum;
        (*partitionCount)++;
    }
    
    return true;
}

// ============================================================================
// Main Flasher Logic
// ============================================================================

static bool flashPartitions(const ParsedPartition* partitions, int count) {
    int flashed = 0;
    int skipped = 0;
    int failed = 0;
    
    for (int i = 0; i < count; ++i) {
        const auto& part = partitions[i];
        const MtdMapping* mtd = findMtdForPartition(part.embType);
        
        if (!mtd) {
            LOG("Partition %d (%s): No MTD mapping, skipping", 
                i, embTypeName(part.embType));
            skipped++;
            continue;
        }
        
        LOG("Flashing %s -> %s (%zu bytes)...", 
            embTypeName(part.embType), mtd->mtdDev, part.dataSize);
        
        // Verify checksum if available
        if (part.hasChecksum) {
            uint32_t calc = calculateChecksum(part.data, part.dataSize);
            if (calc != part.checksum) {
                LOG_ERR("  Checksum mismatch: calc=0x%08X expected=0x%08X", calc, part.checksum);
                if (mtd->critical) {
                    return false;  // Abort on critical partition failure
                }
                failed++;
                continue;
            }
            LOG_V("  Checksum OK: 0x%08X", part.checksum);
        }
        
        // Write to MTD
        if (!writeMtd(mtd->mtdDev, part.data, part.dataSize)) {
            LOG_ERR("  FAILED to write %s", mtd->name);
            if (mtd->critical) {
                return false;
            }
            failed++;
            continue;
        }
        
        LOG("  %s: OK", mtd->name);
        flashed++;
    }
    
    LOG("Flash complete: %d flashed, %d skipped, %d failed", flashed, skipped, failed);
    return (failed == 0);
}

// ============================================================================
// Main
// ============================================================================

static void usage(const char* prog) {
    fprintf(stderr, "Usage: %s [-v] [-n]\n", prog);
    fprintf(stderr, "  -v  Verbose output\n");
    fprintf(stderr, "  -n  No reboot after flashing (for testing)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Reads upgrade info from /tmp/upgrade/upgradeinfo\n");
    fprintf(stderr, "Maps firmware from physical RAM and writes to MTD partitions\n");
}

int main(int argc, char* argv[]) {
    bool noReboot = false;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-v") == 0) {
            g_verbose = true;
        } else if (strcmp(argv[i], "-n") == 0) {
            noReboot = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        }
    }
    
    LOG("=== Novatek Firmware Flasher ===");
    LOG("Reading upgrade info...");
    
    // Read upgrade info
    UpgradeInfo info;
    memset(&info, 0, sizeof(info));
    
    FILE* fp = fopen("/tmp/upgrade/upgradeinfo", "rb");
    if (!fp) {
        LOG_ERR("Cannot open /tmp/upgrade/upgradeinfo: %s", strerror(errno));
        return 1;
    }
    
    if (fread(&info, sizeof(info), 1, fp) != 1) {
        LOG_ERR("Failed to read upgrade info");
        fclose(fp);
        return 1;
    }
    fclose(fp);
    
    // Validate magic
    if (info.magic != UPGRADE_INFO_MAGIC) {
        LOG_ERR("Invalid upgrade info magic: 0x%08X (expected 0x%08X)", 
                info.magic, UPGRADE_INFO_MAGIC);
        return 1;
    }
    
    LOG("Firmware: phys=0x%llX size=%llu", 
        (unsigned long long)info.physAddr, (unsigned long long)info.size);
    
    // Map physical memory
    int memFd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (memFd < 0) {
        LOG_ERR("Cannot open /dev/mem: %s", strerror(errno));
        return 1;
    }
    
    // Page-align the mapping
    size_t pageSize = sysconf(_SC_PAGESIZE);
    uint64_t alignedAddr = info.physAddr & ~(pageSize - 1);
    size_t offset = info.physAddr - alignedAddr;
    size_t mapSize = info.size + offset;
    
    void* mapped = mmap(nullptr, mapSize, PROT_READ, MAP_SHARED, memFd, alignedAddr);
    if (mapped == MAP_FAILED) {
        LOG_ERR("Failed to mmap physical memory: %s", strerror(errno));
        close(memFd);
        return 1;
    }
    
    const uint8_t* firmware = static_cast<const uint8_t*>(mapped) + offset;
    LOG("Mapped firmware at %p", firmware);
    
    // Parse NVTPACK
    ParsedPartition partitions[16];
    int partitionCount = 0;
    
    if (!parseNvtpack(firmware, info.size, partitions, &partitionCount, 16)) {
        LOG_ERR("Failed to parse NVTPACK firmware");
        munmap(mapped, mapSize);
        close(memFd);
        return 1;
    }
    
    LOG("Parsed %d partitions", partitionCount);
    
    // Flash partitions
    LOG("=== Starting Flash Operation ===");
    bool success = flashPartitions(partitions, partitionCount);
    
    // Cleanup
    munmap(mapped, mapSize);
    close(memFd);
    
    if (!success) {
        LOG_ERR("Flashing failed!");
        return 1;
    }
    
    LOG("=== Flashing Complete ===");
    
    // Sync filesystems
    sync();
    sync();
    sync();
    
    if (noReboot) {
        LOG("Skipping reboot (-n flag)");
        return 0;
    }
    
    LOG("Rebooting in 3 seconds...");
    sleep(3);
    
    // Reboot
    reboot(RB_AUTOBOOT);
    
    // Should not reach here
    LOG_ERR("Reboot failed!");
    return 1;
}
