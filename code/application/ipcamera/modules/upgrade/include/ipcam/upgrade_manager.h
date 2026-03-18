/**
 * @file upgrade_manager.h
 * @brief Main firmware upgrade orchestrator for secure boot firmware
 * 
 * Implements the "Load-Verify-Reboot-Flash" architecture:
 * 1. Download/receive firmware to staging area
 * 2. Validate header, signature, checksums
 * 3. Stop services (RTSP, AI, watchdog)
 * 4. Stage firmware in physical memory
 * 5. Write handoff info and reboot
 */

#pragma once

#include "upgrade_types.h"
#include "firmware_validator.h"
#include "compat.h"
#include <memory>
#include <string>
#include <filesystem>

namespace ipcam::upgrade {

/**
 * @brief Main upgrade manager - singleton orchestrator
 * 
 * Usage:
 * @code
 *   auto& mgr = UpgradeManager::instance();
 *   mgr.configure(config);
 *   
 *   // Option 1: From file
 *   auto result = mgr.upgradeFromFile("/tmp/FW98538A.bin");
 *   
 *   // Option 2: From HTTP URL
 *   auto result = mgr.upgradeFromUrl("http://server/FW98538A.bin");
 *   
 *   // Option 3: From memory (e.g., uploaded via web)
 *   auto result = mgr.upgradeFromMemory(data.data(), data.size());
 *   
 *   if (result.has_value()) {
 *       mgr.executeUpgrade();  // Reboots system
 *   }
 * @endcode
 */
class UpgradeManager {
public:
    /**
     * @brief Get singleton instance
     */
    static UpgradeManager& instance();
    
    /**
     * @brief Configure upgrade manager
     * @param config Upgrade configuration
     */
    void configure(const UpgradeConfig& config);
    
    /**
     * @brief Get current status
     */
    UpgradeStatus status() const;
    
    /**
     * @brief Get last error
     */
    UpgradeError lastError() const;
    
    /**
     * @brief Get last error message
     */
    std::string lastErrorMessage() const;
    
    // =========================================================================
    // Upgrade Initiation Methods
    // =========================================================================
    
    /**
     * @brief Start upgrade from local file
     * @param filePath Path to firmware file (e.g., /tmp/FW98538A.bin)
     * @return Validation result if successful, error otherwise
     */
    Expected<FirmwareValidation, UpgradeError> 
    upgradeFromFile(const std::filesystem::path& filePath);
    
    /**
     * @brief Start upgrade from HTTP/HTTPS URL
     * @param url Firmware download URL
     * @return Validation result if successful, error otherwise
     */
    Expected<FirmwareValidation, UpgradeError>
    upgradeFromUrl(const std::string& url);
    
    /**
     * @brief Start upgrade from memory buffer
     * @param data Firmware data
     * @param size Data size
     * @return Validation result if successful, error otherwise
     */
    Expected<FirmwareValidation, UpgradeError>
    upgradeFromMemory(const void* data, size_t size);
    
    // =========================================================================
    // Streaming Upload (Direct to Physical RAM)
    // =========================================================================
    
    /**
     * @brief Begin streaming upload directly to physical RAM
     * @param expectedSize Expected firmware size (for pre-allocation)
     * @return Success or error
     * 
     * This pre-allocates physical memory for firmware staging.
     * Use writeChunk() to stream data, then finalizeUpload() to complete.
     */
    Expected<void, UpgradeError> beginStreamingUpload(size_t expectedSize);
    
    /**
     * @brief Write a chunk of firmware data during streaming upload
     * @param data Chunk data
     * @param size Chunk size
     * @return Success or error
     */
    Expected<void, UpgradeError> writeChunk(const void* data, size_t size);
    
    /**
     * @brief Finalize streaming upload and validate firmware
     * @return Validation result if successful, error otherwise
     */
    Expected<FirmwareValidation, UpgradeError> finalizeUpload();
    
    /**
     * @brief Get physical memory address for direct writes (advanced)
     * @return Memory info or nullptr if not allocated
     * 
     * Allows zero-copy writes directly to physical RAM.
     * Caller must track offset and call finalizeUploadAt(bytesWritten).
     */
    struct StagingMemoryInfo {
        void* virtualAddress;
        uint64_t physicalAddress;
        size_t capacity;
        size_t bytesWritten;
    };
    const StagingMemoryInfo* getStagingMemory() const;
    
    /**
     * @brief Finalize upload with explicit byte count
     * @param bytesWritten Actual bytes written to staging memory
     * @return Validation result
     */
    Expected<FirmwareValidation, UpgradeError> finalizeUploadAt(size_t bytesWritten);
    
    // =========================================================================
    // Upgrade Execution
    // =========================================================================
    
    /**
     * @brief Execute the upgrade (reboot to flash)
     * 
     * This method:
     * 1. Stops all services (RTSP, AI, recordings)
     * 2. Writes handoff info to /tmp/upgrade/upgradeinfo
     * 3. Triggers system reboot
     * 
     * @note This method does NOT return on success - system reboots
     * @return Error code if failed
     */
    UpgradeError executeUpgrade();
    
    /**
     * @brief Cancel pending upgrade
     * Releases allocated memory and clears state
     */
    void cancelUpgrade();
    
    // =========================================================================
    // Query Methods
    // =========================================================================
    
    /**
     * @brief Check if upgrade is pending (ready to execute)
     */
    bool isUpgradePending() const;
    
    /**
     * @brief Get validated firmware info
     */
    const FirmwareValidation* validatedFirmware() const;
    
    /**
     * @brief Get progress (0-100)
     */
    int progressPercent() const;
    
    /**
     * @brief Get current progress message
     */
    std::string progressMessage() const;
    
    /**
     * @brief Get current configuration (read-only)
     */
    const UpgradeConfig& config() const;

private:
    UpgradeManager();
    ~UpgradeManager();
    
    // Non-copyable
    UpgradeManager(const UpgradeManager&) = delete;
    UpgradeManager& operator=(const UpgradeManager&) = delete;
    
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ipcam::upgrade
