/**
 * @file system_controller.h
 * @brief System control for upgrade operations
 * 
 * Handles process shutdown and reboot for embedded Linux system.
 * Works with init-based systems (BusyBox), not systemd.
 */

#pragma once

#include "upgrade_types.h"
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <sys/types.h>

namespace ipcam::upgrade {

/**
 * @brief Service/process type
 */
enum class ServiceType {
    Daemon,     // Standalone daemon process
    InitScript  // Process managed by init script
};

/**
 * @brief Information about a running service/process
 */
struct ServiceInfo {
    std::string name;
    ServiceType type = ServiceType::Daemon;
    bool running = false;
    bool critical = false;  // Critical = stop last (e.g., ipcamd)
    pid_t pid = 0;
};

/**
 * @brief Reboot mode
 */
enum class RebootMode {
    Warm,           // Standard reboot
    Cold,           // Power off
    Recovery,       // Boot to recovery mode
    FactoryReset    // Factory reset then boot
};

/**
 * @brief System controller for upgrade operations
 * 
 * Manages:
 * - Process shutdown (ipcamd, go2rtc, nginx, udhcpc, etc.)
 * - Filesystem sync
 * - System reboot
 */
class SystemController {
public:
    SystemController();
    ~SystemController();
    
    /**
     * @brief Prepare system for upgrade
     * Creates marker file, syncs filesystems
     */
    bool prepareForUpgrade();
    
    /**
     * @brief Stop all services/processes needed for upgrade
     * @param timeoutSeconds Maximum wait time per process
     * @return True if critical processes stopped
     */
    bool stopServices(int timeoutSeconds = 10);
    
    /**
     * @brief Attempt to restart services (if upgrade cancelled)
     * Note: On embedded system, a reboot is often needed instead
     */
    bool restartServices();
    
    /**
     * @brief List currently running services that would be stopped
     */
    std::vector<ServiceInfo> listRunningServices() const;
    
    /**
     * @brief Set boot flag for U-Boot firmware update
     * @param firmwareAddr Physical address where firmware is staged
     * @param firmwareSize Size of firmware
     */
    bool setUpgradeBootFlag(uintptr_t firmwareAddr, size_t firmwareSize);
    
    /**
     * @brief Clear upgrade boot flag
     */
    bool clearUpgradeFlag();
    
    /**
     * @brief Sync all filesystems
     */
    bool syncFilesystems();
    
    /**
     * @brief Trigger system reboot
     * @param mode Reboot mode (warm, cold, recovery, factory)
     */
    void reboot(RebootMode mode = RebootMode::Warm);
    
    /**
     * @brief Get U-Boot version
     */
    std::string getBootloaderVersion() const;
    
    /**
     * @brief Get current firmware version
     */
    std::string getCurrentFirmwareVersion() const;
    
    /**
     * @brief Get available system memory
     */
    size_t getAvailableMemory() const;
    
    /**
     * @brief Get available storage space
     */
    size_t getAvailableStorage(const std::string& path = "/tmp") const;
    
    /**
     * @brief Check if upgrade is currently in progress
     */
    bool isUpgradeInProgress() const;
    
    /**
     * @brief Flash a specific partition from memory
     * 
     * @param mtdDevice MTD device path (e.g., /dev/mtd6)
     * @param data Pointer to partition data
     * @param size Size of data
     * @param progressCb Optional progress callback
     * @return True if flash succeeded
     */
    bool flashPartition(const std::string& mtdDevice,
                        const void* data, size_t size,
                        std::function<void(int, const std::string&)> progressCb = nullptr);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ipcam::upgrade
