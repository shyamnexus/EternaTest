/**
 * @file upgrade_manager.cpp
 * @brief Main firmware upgrade orchestration implementation
 *
 * Architecture: Two-Binary Flasher Design
 * ========================================
 * This implementation follows a safe "Load-Verify-Stage-Handoff" pattern:
 *
 * 1. Main app receives firmware (via file/URL/memory)
 * 2. Main app validates header, signature, compatibility
 * 3. Main app stages firmware in physical RAM (via HDAL)
 * 4. Main app writes metadata to /tmp/upgrade/upgradeinfo
 * 5. Main app copies flasher binary + script to /tmp/upgrade/
 * 6. Main app launches flasher script in background
 * 7. Main app kills itself (or is killed by script)
 * 8. Flasher binary maps physical RAM, parses NVTPACK, writes MTD
 * 9. Flasher binary reboots
 *
 * Why This Design:
 * - Main app is complex (RTSP, web, AI, etc.) - unsafe to flash while running
 * - Flasher is minimal, single-purpose, no network exposure
 * - RAM contents persist until reboot (no power cycle between stages)
 * - Flasher properly parses NVTPACK and writes individual partitions
 */

#include "ipcam/upgrade_manager.h"
#include "ipcam/system_controller.h"
#include "ipcam/physical_memory.h"
#include <spdlog/spdlog.h>

#include <fstream>
#include <chrono>
#include <cstring>
#include <mutex>
#include <unistd.h>  // for sync(), fork(), execl()
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

namespace ipcam::upgrade {

// =============================================================================
// Implementation
// =============================================================================

struct UpgradeManager::Impl {
    UpgradeConfig config;
    FirmwareValidator validator;
    SystemController sysCtrl;
    
    UpgradeStatus status = UpgradeStatus::Idle;
    UpgradeError lastErr = UpgradeError::None;
    std::string lastErrMsg;
    
    std::unique_ptr<PhysicalMemory> stagingMemory;  // Physical memory for firmware staging
    FirmwareValidation lastValidation;
    
    // Streaming upload state
    size_t streamingBytesWritten = 0;
    size_t streamingExpectedSize = 0;
    UpgradeManager::StagingMemoryInfo stagingInfo = {};
    
    int progressPercent = 0;
    std::string progressMsg;
    
    std::mutex stateMutex;
    
    void setStatus(UpgradeStatus s, int percent, const std::string& msg) {
        std::lock_guard<std::mutex> lock(stateMutex);
        status = s;
        progressPercent = percent;
        progressMsg = msg;
        
        if (config.progressCallback) {
            config.progressCallback(s, percent, msg);
        }
    }
    
    void setError(UpgradeError err, const std::string& msg) {
        std::lock_guard<std::mutex> lock(stateMutex);
        lastErr = err;
        lastErrMsg = msg;
        status = UpgradeStatus::Failed;
        progressMsg = msg;
        
        if (config.progressCallback) {
            config.progressCallback(UpgradeStatus::Failed, 0, msg);
        }
    }
    
    void resetStreamingState() {
        streamingBytesWritten = 0;
        streamingExpectedSize = 0;
        stagingInfo = {};
    }
};

// =============================================================================
// UpgradeManager
// =============================================================================

UpgradeManager& UpgradeManager::instance()
{
    static UpgradeManager instance;
    return instance;
}

UpgradeManager::UpgradeManager()
    : impl_(std::make_unique<Impl>())
{
}

UpgradeManager::~UpgradeManager() = default;

void UpgradeManager::configure(const UpgradeConfig& config)
{
    impl_->config = config;
    
    // Configure validator with security keys
    if (!config.rsaPublicKeyN.empty()) {
        impl_->validator.setSecurityKeys(
            config.rsaPublicKeyN,
            config.rsaPublicKeyE,
            config.aesKey);
    }
    
    spdlog::info("UpgradeManager: Configured (maxSize={}MB, secureBoot={}, verifyChecksum={})",
                 config.maxFirmwareSize / (1024 * 1024),
                 config.requireSecureBoot,
                 config.verifyChecksum);
}

UpgradeStatus UpgradeManager::status() const
{
    std::lock_guard<std::mutex> lock(impl_->stateMutex);
    return impl_->status;
}

UpgradeError UpgradeManager::lastError() const
{
    std::lock_guard<std::mutex> lock(impl_->stateMutex);
    return impl_->lastErr;
}

std::string UpgradeManager::lastErrorMessage() const
{
    std::lock_guard<std::mutex> lock(impl_->stateMutex);
    return impl_->lastErrMsg;
}

Expected<FirmwareValidation, UpgradeError>
UpgradeManager::upgradeFromFile(const std::filesystem::path& filePath)
{
    spdlog::info("UpgradeManager: Starting upgrade from file: {}", filePath.string());
    
    // Check current state
    if (impl_->status != UpgradeStatus::Idle && impl_->status != UpgradeStatus::Failed) {
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::UpgradeInProgress);
    }
    
    impl_->setStatus(UpgradeStatus::Downloading, 0, "Reading firmware file...");
    
    // Open file
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        impl_->setError(UpgradeError::FileOpenFailed, "Failed to open firmware file");
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::FileOpenFailed);
    }
    
    // Get file size
    size_t fileSize = file.tellg();
    file.seekg(0);
    
    if (fileSize > impl_->config.maxFirmwareSize) {
        impl_->setError(UpgradeError::FirmwareTooLarge, "Firmware exceeds maximum size");
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::FirmwareTooLarge);
    }
    
    // Read only header for validation (avoid OOM on memory-constrained devices)
    constexpr size_t HEADER_READ_SIZE = 4096;  // Enough for header + partition headers
    std::vector<uint8_t> headerData(std::min(fileSize, HEADER_READ_SIZE));
    if (!file.read(reinterpret_cast<char*>(headerData.data()), headerData.size())) {
        impl_->setError(UpgradeError::FileReadError, "Failed to read firmware header");
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::FileReadError);
    }
    
    impl_->setStatus(UpgradeStatus::Downloading, 100, "Header loaded");
    
    // Validate firmware header
    impl_->setStatus(UpgradeStatus::Validating, 0, "Validating firmware...");
    
    bool requireSecure = impl_->config.requireSecureBoot ||
                         (impl_->config.securityMode == FirmwareSecurityMode::RequireSecure);
    
    // Use header-only validation with actual file size
    auto validation = impl_->validator.validateHeader(
        Span<const uint8_t>(headerData.data(), headerData.size()),
        fileSize);
    
    if (!validation.valid) {
        impl_->setError(validation.error, validation.errorMessage);
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            validation.error);
    }
    
    // Warn if using unsigned firmware in PreferSecure mode
    if (impl_->config.securityMode == FirmwareSecurityMode::PreferSecure &&
        !validation.isSecureBoot) {
        spdlog::warn("Using unsigned firmware (secure boot preferred but not required)");
    }
    
    impl_->setStatus(UpgradeStatus::Validating, 100, "Validation complete");
    
    // Stage to physical memory using HDAL hd_common_mem_alloc
    // This allocates from managed memory pools, not conflicting with NVTMPP
    impl_->setStatus(UpgradeStatus::Staging, 0, "Allocating physical memory...");
    
    impl_->stagingMemory = std::make_unique<PhysicalMemory>();
    auto allocResult = impl_->stagingMemory->allocate(fileSize, "fw_upgrade");
    
    if (!allocResult.has_value()) {
        impl_->setError(UpgradeError::MemoryAllocationFailed, "Failed to allocate staging memory");
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::MemoryAllocationFailed);
    }
    
    // Stream file to physical memory in chunks
    file.seekg(0);
    constexpr size_t CHUNK_SIZE = 64 * 1024;  // 64KB chunks
    std::vector<uint8_t> chunk(CHUNK_SIZE);
    size_t totalWritten = 0;
    
    while (totalWritten < fileSize) {
        size_t toRead = std::min(CHUNK_SIZE, fileSize - totalWritten);
        if (!file.read(reinterpret_cast<char*>(chunk.data()), toRead)) {
            impl_->setError(UpgradeError::FileReadError, "Failed to read firmware data");
            impl_->stagingMemory.reset();
            return Expected<FirmwareValidation, UpgradeError>(
                Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
                UpgradeError::FileReadError);
        }
        
        if (!impl_->stagingMemory->write(totalWritten, chunk.data(), toRead)) {
            impl_->setError(UpgradeError::WriteError, "Failed to write to staging memory");
            impl_->stagingMemory.reset();
            return Expected<FirmwareValidation, UpgradeError>(
                Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
                UpgradeError::WriteError);
        }
        
        totalWritten += toRead;
        int progress = static_cast<int>((totalWritten * 100) / fileSize);
        impl_->setStatus(UpgradeStatus::Staging, progress, "Staging firmware...");
    }
    
    impl_->stagingMemory->flushCache();
    
    impl_->lastValidation = validation;
    impl_->setStatus(UpgradeStatus::ReadyToReboot, 100, "Ready to upgrade");
    
    spdlog::info("UpgradeManager: Firmware staged successfully ({} bytes, {} partitions)",
                 validation.totalSize, validation.partitionCount);
    
    return validation;
}

Expected<FirmwareValidation, UpgradeError>
UpgradeManager::upgradeFromUrl(const std::string& url)
{
    // TODO: Implement HTTP download using libcurl or similar
    spdlog::error("UpgradeManager: URL download not implemented");
    return Expected<FirmwareValidation, UpgradeError>(
        Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
        UpgradeError::DownloadFailed);
}

Expected<FirmwareValidation, UpgradeError>
UpgradeManager::upgradeFromMemory(const void* data, size_t size)
{
    spdlog::info("UpgradeManager: Starting upgrade from memory ({} bytes)", size);
    
    if (!data || size == 0) {
        impl_->setError(UpgradeError::InvalidHeader, "Invalid firmware data");
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::InvalidHeader);
    }
    
    if (impl_->status != UpgradeStatus::Idle && impl_->status != UpgradeStatus::Failed) {
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::UpgradeInProgress);
    }
    
    if (size > impl_->config.maxFirmwareSize) {
        impl_->setError(UpgradeError::FirmwareTooLarge, "Firmware exceeds maximum size");
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::FirmwareTooLarge);
    }
    
    // Validate firmware
    impl_->setStatus(UpgradeStatus::Validating, 0, "Validating firmware...");
    
    bool requireSecure = impl_->config.requireSecureBoot ||
                         (impl_->config.securityMode == FirmwareSecurityMode::RequireSecure);
    
    auto validation = impl_->validator.validateFull(
        Span<const uint8_t>(static_cast<const uint8_t*>(data), size),
        requireSecure);
    
    if (!validation.valid) {
        impl_->setError(validation.error, validation.errorMessage);
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            validation.error);
    }
    
    impl_->setStatus(UpgradeStatus::Validating, 100, "Validation complete");
    
    // Stage to physical memory using HDAL hd_common_mem_alloc
    impl_->setStatus(UpgradeStatus::Staging, 0, "Allocating physical memory...");
    
    impl_->stagingMemory = std::make_unique<PhysicalMemory>();
    auto allocResult = impl_->stagingMemory->allocate(size, "fw_upgrade");
    
    if (!allocResult.has_value()) {
        impl_->setError(UpgradeError::MemoryAllocationFailed, "Failed to allocate staging memory");
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::MemoryAllocationFailed);
    }
    
    if (!impl_->stagingMemory->write(0, data, size)) {
        impl_->setError(UpgradeError::WriteError, "Failed to write firmware to staging memory");
        impl_->stagingMemory.reset();
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::WriteError);
    }
    
    impl_->stagingMemory->flushCache();
    
    impl_->lastValidation = validation;
    impl_->setStatus(UpgradeStatus::ReadyToReboot, 100, "Ready to upgrade");
    
    return validation;
}

// =============================================================================
// Streaming Upload (Direct to Physical RAM)
// =============================================================================

Expected<void, UpgradeError> UpgradeManager::beginStreamingUpload(size_t expectedSize)
{
    spdlog::info("UpgradeManager: Beginning streaming upload ({} bytes expected)", expectedSize);
    
    // Check current state
    if (impl_->status != UpgradeStatus::Idle && impl_->status != UpgradeStatus::Failed) {
        return Expected<void, UpgradeError>(
            Expected<void, UpgradeError>::UnexpectedTag{},
            UpgradeError::UpgradeInProgress);
    }
    
    if (expectedSize > impl_->config.maxFirmwareSize) {
        impl_->setError(UpgradeError::FirmwareTooLarge, "Firmware exceeds maximum size");
        return Expected<void, UpgradeError>(
            Expected<void, UpgradeError>::UnexpectedTag{},
            UpgradeError::FirmwareTooLarge);
    }
    
    // Reset any previous state
    impl_->resetStreamingState();
    
    // Allocate physical memory upfront
    impl_->setStatus(UpgradeStatus::Downloading, 0, "Allocating staging memory...");
    
    impl_->stagingMemory = std::make_unique<PhysicalMemory>();
    auto allocResult = impl_->stagingMemory->allocate(expectedSize, "fw_upload");
    
    if (!allocResult.has_value()) {
        impl_->setError(UpgradeError::MemoryAllocationFailed, "Failed to allocate staging memory");
        return Expected<void, UpgradeError>(
            Expected<void, UpgradeError>::UnexpectedTag{},
            UpgradeError::MemoryAllocationFailed);
    }
    
    const auto* block = impl_->stagingMemory->memoryBlock();
    
    // Update staging info for direct access
    impl_->stagingInfo.virtualAddress = block->virtualAddress;
    impl_->stagingInfo.physicalAddress = block->physicalAddress;
    impl_->stagingInfo.capacity = block->size;
    impl_->stagingInfo.bytesWritten = 0;
    impl_->streamingExpectedSize = expectedSize;
    impl_->streamingBytesWritten = 0;
    
    spdlog::info("Staging memory ready: phys=0x{:08X}, virt={:p}, size={}",
                 block->physicalAddress, block->virtualAddress, block->size);
    
    impl_->setStatus(UpgradeStatus::Downloading, 1, "Ready for upload...");
    
    return {};
}

Expected<void, UpgradeError> UpgradeManager::writeChunk(const void* data, size_t size)
{
    if (impl_->status != UpgradeStatus::Downloading) {
        return Expected<void, UpgradeError>(
            Expected<void, UpgradeError>::UnexpectedTag{},
            UpgradeError::InvalidState);
    }
    
    if (!impl_->stagingMemory || !impl_->stagingMemory->isAllocated()) {
        return Expected<void, UpgradeError>(
            Expected<void, UpgradeError>::UnexpectedTag{},
            UpgradeError::InvalidState);
    }
    
    // Check bounds
    if (impl_->streamingBytesWritten + size > impl_->stagingInfo.capacity) {
        impl_->setError(UpgradeError::FirmwareTooLarge, "Upload exceeds allocated memory");
        return Expected<void, UpgradeError>(
            Expected<void, UpgradeError>::UnexpectedTag{},
            UpgradeError::FirmwareTooLarge);
    }
    
    // Write directly to physical memory
    if (!impl_->stagingMemory->write(impl_->streamingBytesWritten, data, size)) {
        impl_->setError(UpgradeError::WriteError, "Failed to write chunk to staging memory");
        return Expected<void, UpgradeError>(
            Expected<void, UpgradeError>::UnexpectedTag{},
            UpgradeError::WriteError);
    }
    
    impl_->streamingBytesWritten += size;
    impl_->stagingInfo.bytesWritten = impl_->streamingBytesWritten;
    
    // Update progress
    if (impl_->streamingExpectedSize > 0) {
        int progress = static_cast<int>((impl_->streamingBytesWritten * 100) / impl_->streamingExpectedSize);
        impl_->setStatus(UpgradeStatus::Downloading, progress, "Uploading...");
    }
    
    return {};
}

Expected<FirmwareValidation, UpgradeError> UpgradeManager::finalizeUpload()
{
    return finalizeUploadAt(impl_->streamingBytesWritten);
}

Expected<FirmwareValidation, UpgradeError> UpgradeManager::finalizeUploadAt(size_t bytesWritten)
{
    spdlog::info("UpgradeManager: Finalizing upload ({} bytes)", bytesWritten);
    
    if (impl_->status != UpgradeStatus::Downloading) {
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::InvalidState);
    }
    
    if (!impl_->stagingMemory || !impl_->stagingMemory->isAllocated()) {
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::InvalidState);
    }
    
    if (bytesWritten == 0) {
        impl_->setError(UpgradeError::InvalidHeader, "No firmware data received");
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            UpgradeError::InvalidHeader);
    }
    
    impl_->stagingMemory->flushCache();
    
    // Validate firmware in physical memory
    impl_->setStatus(UpgradeStatus::Validating, 0, "Validating firmware...");
    
    const void* fwData = impl_->stagingInfo.virtualAddress;
    
    bool requireSecure = impl_->config.requireSecureBoot ||
                         (impl_->config.securityMode == FirmwareSecurityMode::RequireSecure);
    
    auto validation = impl_->validator.validateFull(
        Span<const uint8_t>(static_cast<const uint8_t*>(fwData), bytesWritten),
        requireSecure);
    
    if (!validation.valid) {
        impl_->setError(validation.error, validation.errorMessage);
        impl_->stagingMemory.reset();
        impl_->resetStreamingState();
        return Expected<FirmwareValidation, UpgradeError>(
            Expected<FirmwareValidation, UpgradeError>::UnexpectedTag{},
            validation.error);
    }
    
    // Warn if using unsigned firmware in PreferSecure mode
    if (impl_->config.securityMode == FirmwareSecurityMode::PreferSecure &&
        !validation.isSecureBoot) {
        spdlog::warn("Using unsigned firmware (secure boot preferred but not required)");
    }
    
    impl_->setStatus(UpgradeStatus::Validating, 100, "Validation complete");
    
    // Update the staging memory size to actual bytes written
    // (we may have allocated more than needed)
    impl_->stagingInfo.bytesWritten = bytesWritten;
    
    impl_->lastValidation = validation;
    impl_->setStatus(UpgradeStatus::ReadyToReboot, 100, "Ready to upgrade");
    
    spdlog::info("UpgradeManager: Firmware validated and staged ({} bytes, {} partitions)",
                 validation.totalSize, validation.partitionCount);
    
    return validation;
}

const UpgradeManager::StagingMemoryInfo* UpgradeManager::getStagingMemory() const
{
    if (impl_->stagingMemory && impl_->stagingMemory->isAllocated()) {
        return &impl_->stagingInfo;
    }
    return nullptr;
}

UpgradeError UpgradeManager::executeUpgrade()
{
    if (impl_->status != UpgradeStatus::ReadyToReboot) {
        return UpgradeError::InvalidState;
    }
    
    if (!impl_->stagingMemory || !impl_->stagingMemory->isAllocated()) {
        return UpgradeError::InvalidState;
    }
    
    const auto* memBlock = impl_->stagingMemory->memoryBlock();
    if (!memBlock) {
        return UpgradeError::InvalidState;
    }
    
    // Use actual bytes written if available from streaming, else use allocated size
    size_t actualSize = impl_->stagingInfo.bytesWritten > 0 
        ? impl_->stagingInfo.bytesWritten 
        : memBlock->size;
    
    spdlog::info("UpgradeManager: Executing upgrade from phys=0x{:08X}, size={}",
                 memBlock->physicalAddress, actualSize);
    
    // ===========================================================================
    // Two-Binary Flasher Architecture
    //
    // Flow:
    //   1. Prepare system (sync, disable watchdogs)
    //   2. Stop ALL userspace services (nginx, go2rtc, ipcamd, etc.)
    //   3. Write metadata to /tmp/upgrade/upgradeinfo
    //   4. Launch flasher binary in background
    //   5. Kill main application
    //   6. Flasher maps physical RAM, parses NVTPACK, writes MTD
    //   7. Flasher reboots
    //
    // RAM persists until actual reboot, so flasher can access staged firmware.
    // ===========================================================================
    
    // -------------------------------------------------------------------------
    // Step 1: Prepare system
    // -------------------------------------------------------------------------
    impl_->setStatus(UpgradeStatus::Writing, 5, "Preparing system for upgrade...");
    
    spdlog::info("Step 1: Preparing system for upgrade");
    impl_->sysCtrl.prepareForUpgrade();
    
    // -------------------------------------------------------------------------
    // Step 2: Stop ALL userspace services
    // This is critical - no processes should be running that might:
    //   - Access flash storage
    //   - Consume memory
    //   - Interfere with flasher
    // -------------------------------------------------------------------------
    impl_->setStatus(UpgradeStatus::Writing, 15, "Stopping services...");
    
    spdlog::info("Step 2: Stopping all userspace services (nginx, go2rtc, ipcamd, etc.)");
    if (!impl_->sysCtrl.stopServices(10)) {
        spdlog::warn("Some services may not have stopped cleanly, continuing anyway");
    }
    
    // -------------------------------------------------------------------------
    // Step 3: Create upgrade directory and write metadata
    // -------------------------------------------------------------------------
    impl_->setStatus(UpgradeStatus::Writing, 30, "Writing upgrade metadata...");
    
    spdlog::info("Step 3: Writing upgrade metadata");
    
    // Create upgrade directory
    mkdir("/tmp/upgrade", 0755);
    
    // Write upgrade info file (metadata for flasher)
    struct UpgradeInfo {
        uint64_t physAddr;
        uint64_t size;
        uint32_t crc[16];
        char     timestamp[32];
        uint32_t magic;
        uint32_t version;
    };
    
    constexpr uint32_t UPGRADE_INFO_MAGIC = 0x55504752;  // "UPGR"
    constexpr uint32_t UPGRADE_INFO_VERSION = 1;
    
    UpgradeInfo info = {};
    info.physAddr = memBlock->physicalAddress;
    info.size = actualSize;  // Use actual firmware size, not allocated size
    info.magic = UPGRADE_INFO_MAGIC;
    info.version = UPGRADE_INFO_VERSION;
    
    // Get timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    strftime(info.timestamp, sizeof(info.timestamp), "%Y-%m-%d %H:%M:%S", localtime(&time_t_now));
    
    // Write upgrade info
    {
        std::ofstream infoFile("/tmp/upgrade/upgradeinfo", std::ios::binary);
        if (!infoFile) {
            impl_->setError(UpgradeError::WriteError, "Failed to write upgrade info");
            return UpgradeError::WriteError;
        }
        infoFile.write(reinterpret_cast<const char*>(&info), sizeof(info));
        infoFile.close();
    }
    
    spdlog::info("Wrote upgrade info: phys=0x{:08X} size={}", info.physAddr, info.size);
    
    // -------------------------------------------------------------------------
    // Step 4: Copy flasher binary to tmpfs
    // -------------------------------------------------------------------------
    impl_->setStatus(UpgradeStatus::Writing, 45, "Copying flasher binary...");
    
    spdlog::info("Step 4: Copying flasher binary to /tmp/upgrade/");
    
    // Copy flasher binary to /tmp/upgrade/
    // Look for it in multiple locations
    const char* flasherPaths[] = {
        "/usr/bin/nvt_flasher",
        "/opt/ipcam/bin/nvt_flasher",
        "/mnt/app/nvt_flasher",
        "./nvt_flasher",
        nullptr
    };
    
    std::string flasherSrc;
    for (const char** p = flasherPaths; *p; ++p) {
        if (access(*p, X_OK) == 0) {
            flasherSrc = *p;
            break;
        }
    }
    
    if (flasherSrc.empty()) {
        spdlog::error("Flasher binary not found in any standard location");
        impl_->setError(UpgradeError::WriteError, "Flasher binary not found");
        return UpgradeError::WriteError;
    }
    
    // Copy flasher to /tmp/upgrade/ (in case /usr is on rootfs being updated)
    {
        std::ifstream src(flasherSrc, std::ios::binary);
        std::ofstream dst("/tmp/upgrade/nvt_flasher", std::ios::binary);
        if (!src || !dst) {
            impl_->setError(UpgradeError::WriteError, "Failed to copy flasher binary");
            return UpgradeError::WriteError;
        }
        dst << src.rdbuf();
        dst.close();
        chmod("/tmp/upgrade/nvt_flasher", 0755);
    }
    
    spdlog::info("Copied flasher from {} to /tmp/upgrade/nvt_flasher", flasherSrc);
    
    // -------------------------------------------------------------------------
    // Step 5: Create flasher script
    // -------------------------------------------------------------------------
    spdlog::info("Step 5: Creating flasher launch script");
    
    // Copy flasher script
    const char* scriptPaths[] = {
        "/usr/bin/nvt_flasher.sh",
        "/opt/ipcam/bin/nvt_flasher.sh",
        "/mnt/app/nvt_flasher.sh",
        nullptr
    };
    
    std::string scriptSrc;
    for (const char** p = scriptPaths; *p; ++p) {
        if (access(*p, R_OK) == 0) {
            scriptSrc = *p;
            break;
        }
    }
    
    if (!scriptSrc.empty()) {
        std::ifstream src(scriptSrc, std::ios::binary);
        std::ofstream dst("/tmp/upgrade/nvt_flasher.sh", std::ios::binary);
        if (src && dst) {
            dst << src.rdbuf();
            dst.close();
            chmod("/tmp/upgrade/nvt_flasher.sh", 0755);
        }
    } else {
        // Create minimal inline script - no sleep needed since services already stopped
        std::ofstream script("/tmp/upgrade/nvt_flasher.sh");
        script << "#!/bin/sh\n";
        script << "# Services already stopped by main app\n";
        script << "sleep 2\n";  // Brief delay for main app to exit
        script << "/tmp/upgrade/nvt_flasher -v\n";
        script.close();
        chmod("/tmp/upgrade/nvt_flasher.sh", 0755);
    }
    
    // -------------------------------------------------------------------------
    // Step 6: Sync filesystems
    // -------------------------------------------------------------------------
    impl_->setStatus(UpgradeStatus::Writing, 60, "Syncing filesystems...");
    
    spdlog::info("Step 6: Syncing all filesystems");
    
    // Sync everything
    impl_->sysCtrl.syncFilesystems();
    sync();
    sync();
    
    // -------------------------------------------------------------------------
    // Step 7: Launch flasher in background
    // -------------------------------------------------------------------------
    impl_->setStatus(UpgradeStatus::Rebooting, 75, "Launching flasher...");
    
    spdlog::info("Step 7: Launching flasher binary in background");
    spdlog::info("=== LAUNCHING FLASHER IN BACKGROUND ===");
    spdlog::info("Physical RAM at 0x{:08X} will be accessed by flasher", memBlock->physicalAddress);
    spdlog::info("Main app will exit in 2 seconds...");
    
    // Fork and exec the flasher script in background
    pid_t pid = fork();
    if (pid < 0) {
        impl_->setError(UpgradeError::WriteError, "Failed to fork flasher process");
        return UpgradeError::WriteError;
    }
    
    if (pid == 0) {
        // Child process - become session leader and exec flasher
        setsid();
        
        // Close all file descriptors
        for (int fd = 3; fd < 1024; ++fd) {
            close(fd);
        }
        
        // Redirect stdout/stderr to log file
        int logFd = open("/tmp/upgrade/flasher.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (logFd >= 0) {
            dup2(logFd, STDOUT_FILENO);
            dup2(logFd, STDERR_FILENO);
            close(logFd);
        }
        
        // Exec the flasher script
        execl("/bin/sh", "sh", "/tmp/upgrade/nvt_flasher.sh", nullptr);
        
        // If exec fails
        _exit(127);
    }
    
    // Parent process - flasher is now running in background
    spdlog::info("Flasher launched with PID {}", pid);
    
    impl_->setStatus(UpgradeStatus::Rebooting, 90, "Exiting main application...");
    
    // Give flasher time to start
    sleep(1);
    
    // DO NOT release staging memory - flasher needs it!
    // The physical memory will persist until actual reboot.
    
    spdlog::info("=== MAIN APPLICATION EXITING FOR FIRMWARE UPDATE ===");
    spdlog::info("Flasher will complete the update and reboot.");
    
    // Flush logs
    spdlog::default_logger()->flush();
    
    // Kill ourselves - flasher will take over
    // Use SIGTERM to allow graceful shutdown
    raise(SIGTERM);
    
    // If we're still here after 2 seconds, force exit
    sleep(2);
    _exit(0);
    
    // Should not reach here
    return UpgradeError::RebootFailed;
}

void UpgradeManager::cancelUpgrade()
{
    spdlog::info("UpgradeManager: Cancelling upgrade");
    
    // Release physical memory
    impl_->stagingMemory.reset();
    impl_->lastValidation = {};
    impl_->lastErr = UpgradeError::None;
    impl_->lastErrMsg.clear();
    
    impl_->setStatus(UpgradeStatus::Idle, 0, "Upgrade cancelled");
}

bool UpgradeManager::isUpgradePending() const
{
    return impl_->status == UpgradeStatus::ReadyToReboot;
}

const FirmwareValidation* UpgradeManager::validatedFirmware() const
{
    if (impl_->lastValidation.valid) {
        return &impl_->lastValidation;
    }
    return nullptr;
}

int UpgradeManager::progressPercent() const
{
    std::lock_guard<std::mutex> lock(impl_->stateMutex);
    return impl_->progressPercent;
}

std::string UpgradeManager::progressMessage() const
{
    std::lock_guard<std::mutex> lock(impl_->stateMutex);
    return impl_->progressMsg;
}

const UpgradeConfig& UpgradeManager::config() const
{
    return impl_->config;
}

} // namespace ipcam::upgrade
