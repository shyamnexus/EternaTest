/**
 * @file system_controller.cpp
 * @brief System control for firmware upgrade (service shutdown, reboot)
 */

#include "ipcam/system_controller.h"
#include <spdlog/spdlog.h>

#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <array>

namespace ipcam::upgrade {

// =============================================================================
// Implementation
// =============================================================================

struct SystemController::Impl {
    std::vector<ServiceInfo> services;
    std::vector<std::string> stoppedServices;
    std::string upgradeMarkerPath = "/tmp/.firmware_upgrade";
    std::string ubootEnvPath = "/proc/driver/nvt_info/nvt_fw";  // SDK interface
    bool servicesInitialized = false;
    
    // Process names to stop before upgrade (in order)
    // These are the actual processes running on Novatek embedded Linux
    static constexpr std::array<const char*, 8> processesToStop = {
        "go2rtc",       // Streaming service
        "nginx",        // Web server
        "udhcpc",       // DHCP client
        "ntpd",         // NTP daemon (if running)
        "dropbear",     // SSH server (if running)
        "telnetd",      // Telnet (if running)
        "ipcamd",       // Main IP camera daemon (stop last)
        "nvt_ai"        // AI service (if running)
    };
    
    void discoverServices() {
        // Discover running processes that need to be stopped before upgrade
        services.clear();
        
        for (const auto* name : processesToStop) {
            pid_t pid = findProcessPid(name);
            if (pid > 0) {
                ServiceInfo info;
                info.name = name;
                info.type = ServiceType::Daemon;  // All are daemons on this system
                info.running = true;
                info.pid = pid;
                // ipcamd is critical - stop it last
                info.critical = (strcmp(name, "ipcamd") == 0);
                services.push_back(info);
            }
        }
        
        servicesInitialized = true;
        spdlog::info("SystemController: Discovered {} running processes", services.size());
        for (const auto& svc : services) {
            spdlog::debug("  - {} (pid={})", svc.name, svc.pid);
        }
    }
    
    pid_t findProcessPid(const char* name) {
        DIR* dir = opendir("/proc");
        if (!dir) return 0;
        
        pid_t foundPid = 0;
        struct dirent* entry;
        
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type != DT_DIR) continue;
            
            // Check if directory name is numeric (PID)
            char* endptr;
            long pid = strtol(entry->d_name, &endptr, 10);
            if (*endptr != '\0' || pid <= 0) continue;
            
            // Read /proc/[pid]/comm
            std::string commPath = "/proc/";
            commPath += entry->d_name;
            commPath += "/comm";
            
            std::ifstream commFile(commPath);
            if (commFile) {
                std::string procName;
                std::getline(commFile, procName);
                if (procName == name) {
                    foundPid = static_cast<pid_t>(pid);
                    break;
                }
            }
        }
        
        closedir(dir);
        return foundPid;
    }
    
    bool stopProcess(const std::string& name, int timeoutSec) {
        pid_t pid = findProcessPid(name.c_str());
        if (pid == 0) {
            spdlog::debug("Process {} not running", name);
            return true;  // Not running is success
        }
        
        spdlog::info("Stopping process {} (pid={})", name, pid);
        
        // Send SIGTERM first for graceful shutdown
        if (kill(pid, SIGTERM) != 0) {
            spdlog::warn("Failed to send SIGTERM to {}: {}", name, strerror(errno));
        }
        
        // Wait for graceful termination
        for (int i = 0; i < timeoutSec * 10; ++i) {
            if (kill(pid, 0) != 0) {
                spdlog::info("Process {} stopped gracefully", name);
                return true;  // Process died
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Force kill if still running
        spdlog::warn("Force killing process {} (pid={})", name, pid);
        kill(pid, SIGKILL);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        bool dead = (kill(pid, 0) != 0);
        if (!dead) {
            spdlog::error("Failed to kill process {}", name);
        }
        return dead;
    }
};

// =============================================================================
// SystemController
// =============================================================================

SystemController::SystemController()
    : impl_(std::make_unique<Impl>())
{
    impl_->discoverServices();
}

SystemController::~SystemController()
{
    // Restart any services we stopped if upgrade wasn't completed
    restartServices();
}

bool SystemController::prepareForUpgrade()
{
    spdlog::info("SystemController: Preparing system for firmware upgrade");
    
    // Create upgrade marker file
    std::ofstream marker(impl_->upgradeMarkerPath);
    if (marker) {
        marker << "FIRMWARE_UPGRADE_IN_PROGRESS\n";
        marker << "TIME=" << std::time(nullptr) << "\n";
        marker.close();
    }
    
    // Sync filesystems
    sync();
    
    // Re-discover services in case of changes
    impl_->discoverServices();
    
    return true;
}

bool SystemController::stopServices(int timeoutSeconds)
{
    spdlog::info("SystemController: Stopping {} processes for upgrade (timeout: {}s)",
                 impl_->services.size(), timeoutSeconds);
    
    impl_->stoppedServices.clear();
    
    // Stop non-critical processes first (go2rtc, nginx, udhcpc, etc.)
    for (const auto& svc : impl_->services) {
        if (!svc.critical && svc.running) {
            if (impl_->stopProcess(svc.name, timeoutSeconds)) {
                impl_->stoppedServices.push_back(svc.name);
            } else {
                spdlog::warn("Failed to stop process: {}", svc.name);
            }
        }
    }
    
    // Then stop critical processes (ipcamd last)
    for (const auto& svc : impl_->services) {
        if (svc.critical && svc.running) {
            spdlog::info("Stopping critical process: {}", svc.name);
            if (impl_->stopProcess(svc.name, timeoutSeconds)) {
                impl_->stoppedServices.push_back(svc.name);
            } else {
                spdlog::error("Failed to stop critical process: {}", svc.name);
                // Continue anyway - upgrade is more important
            }
        }
    }
    
    spdlog::info("SystemController: Stopped {} processes", impl_->stoppedServices.size());
    return true;
}

bool SystemController::restartServices()
{
    // After a successful upgrade, the system reboots, so this is only called
    // if the upgrade was cancelled before reboot
    
    if (impl_->stoppedServices.empty()) {
        return true;
    }
    
    spdlog::info("SystemController: Upgrade cancelled - system requires manual restart");
    spdlog::info("The following processes were stopped: ");
    for (const auto& name : impl_->stoppedServices) {
        spdlog::info("  - {}", name);
    }
    
    // On embedded system, easiest recovery is to reboot
    // Or user can manually restart ipcamd which will restart child processes
    spdlog::warn("Run 'reboot' or manually restart ipcamd to restore services");
    
    impl_->stoppedServices.clear();
    return true;
}

std::vector<ServiceInfo> SystemController::listRunningServices() const
{
    return impl_->services;
}

bool SystemController::setUpgradeBootFlag(uintptr_t firmwareAddr, size_t firmwareSize)
{
    spdlog::info("SystemController: Setting upgrade boot flags (addr=0x{:08X}, size={})",
                 firmwareAddr, firmwareSize);
    
    // ==========================================================================
    // Novatek U-Boot firmware update via SHMINFO mechanism
    // 
    // Memory layout from nvt-mem-tbl.dtsi:
    //   shmem:      0x00A00000 - 0x00100000 (SHMINFO structure)
    //   all_in_one: 0x10000000 - 0x06000000 (Firmware staging area)
    //
    // SHMINFO structure (from shm_info.h):
    //   BOOTINFO (112 bytes) at offset 0
    //   COMMINFO (216 bytes) at offset 112
    //     - Resv[0] (offset 128): Boot reason (COMM_UBOOT_BOOT_FUNC_BOOT_UPDFIRM=1)
    //     - Resv[5] (offset 148): Firmware address
    //     - Resv[6] (offset 152): Firmware size
    //
    // When U-Boot sees boot_reason=1 (COMM_UBOOT_BOOT_FUNC_BOOT_UPDFIRM), it:
    //   1. Reads firm_addr from Resv[5] and firm_size from Resv[6]  
    //   2. Calls nvt_process_all_in_one(firm_addr, firm_size, true)
    //   3. This properly parses NVTPACK and flashes each partition
    // ==========================================================================
    
    constexpr uintptr_t SHMINFO_PHYS = 0x00A00000;
    constexpr size_t SHMINFO_SIZE = 328;  // sizeof(SHMINFO)
    
    // Offsets within COMMINFO (which starts at offset 112 in SHMINFO)
    // COMMINFO has 16 bytes header + Resv[45] array of 4-byte values
    constexpr size_t BOOTINFO_SIZE = 112;
    constexpr size_t COMMINFO_HEADER = 16;  // "COMMINFO" string
    constexpr size_t RESV_OFFSET = BOOTINFO_SIZE + COMMINFO_HEADER;  // 128
    
    // COMM_RESV_IDX offsets (each is 4 bytes)
    constexpr size_t BOOT_REASON_OFFSET = RESV_OFFSET + (0 * 4);  // Resv[0] = 128
    constexpr size_t FW_ADDR_OFFSET = RESV_OFFSET + (5 * 4);      // Resv[5] = 148  
    constexpr size_t FW_LEN_OFFSET = RESV_OFFSET + (6 * 4);       // Resv[6] = 152
    
    // Boot reason values
    constexpr uint32_t BOOT_REASON_FWUPDFW = 1;  // COMM_UBOOT_BOOT_FUNC_BOOT_UPDFIRM
    
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        spdlog::error("Failed to open /dev/mem: {}", strerror(errno));
        return false;
    }
    
    // Map SHMINFO region
    void* shmem = mmap(nullptr, 4096, PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, SHMINFO_PHYS & ~0xFFF);
    if (shmem == MAP_FAILED) {
        spdlog::error("Failed to mmap SHMINFO: {}", strerror(errno));
        close(fd);
        return false;
    }
    
    volatile uint32_t* base = static_cast<volatile uint32_t*>(shmem);
    size_t pageOffset = (SHMINFO_PHYS & 0xFFF);
    volatile uint8_t* shminfo = reinterpret_cast<volatile uint8_t*>(base) + pageOffset;
    
    // Verify SHMINFO magic ("LD_NVT" at offset 0)
    char ldMagic[7] = {0};
    for (int i = 0; i < 6; i++) {
        ldMagic[i] = shminfo[i];
    }
    
    if (strncmp(ldMagic, "LD_NVT", 6) != 0) {
        spdlog::error("SHMINFO magic mismatch: got '{}', expected 'LD_NVT'", ldMagic);
        spdlog::warn("SHMINFO may be corrupted or overwritten by userspace");
        munmap(shmem, 4096);
        close(fd);
        return false;
    }
    
    // Write firmware address and size
    volatile uint32_t* fwAddr = reinterpret_cast<volatile uint32_t*>(shminfo + FW_ADDR_OFFSET);
    volatile uint32_t* fwLen = reinterpret_cast<volatile uint32_t*>(shminfo + FW_LEN_OFFSET);
    volatile uint32_t* bootReason = reinterpret_cast<volatile uint32_t*>(shminfo + BOOT_REASON_OFFSET);
    
    *fwAddr = static_cast<uint32_t>(firmwareAddr);
    *fwLen = static_cast<uint32_t>(firmwareSize);
    *bootReason = BOOT_REASON_FWUPDFW;
    
    // Ensure writes are flushed
    msync(shmem, 4096, MS_SYNC);
    __sync_synchronize();  // Memory barrier
    
    spdlog::info("SHMINFO updated: fw_addr=0x{:08X}, fw_len=0x{:08X}, boot_reason={}",
                 *fwAddr, *fwLen, *bootReason);
    
    munmap(shmem, 4096);
    close(fd);
    
    return true;
}

bool SystemController::flashPartition(const std::string& mtdDevice,
                                      const void* data, size_t size,
                                      std::function<void(int, const std::string&)> progressCb)
{
    // Flash a specific partition from memory
    spdlog::info("Flashing {} with {} bytes", mtdDevice, size);
    
    if (progressCb) progressCb(0, "Opening MTD device...");
    
    // Write data to a temp file first (MTD tools expect file input)
    std::string tempFile = "/tmp/partition_data.bin";
    {
        std::ofstream out(tempFile, std::ios::binary);
        if (!out) {
            spdlog::error("Cannot create temp file");
            return false;
        }
        out.write(reinterpret_cast<const char*>(data), size);
    }
    
    if (progressCb) progressCb(10, "Erasing partition...");
    
    // Erase
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "flash_erase %s 0 0 2>&1", mtdDevice.c_str());
    if (system(cmd) != 0) {
        spdlog::warn("flash_erase may have failed, continuing...");
    }
    
    if (progressCb) progressCb(30, "Writing partition...");
    
    // Write
    snprintf(cmd, sizeof(cmd), 
             "nandwrite -p %s %s 2>&1 || flashcp %s %s 2>&1",
             mtdDevice.c_str(), tempFile.c_str(),
             tempFile.c_str(), mtdDevice.c_str());
    
    int ret = system(cmd);
    
    // Clean up temp file
    unlink(tempFile.c_str());
    
    if (ret != 0) {
        spdlog::error("Partition write failed");
        return false;
    }
    
    if (progressCb) progressCb(100, "Done");
    return true;
}

bool SystemController::clearUpgradeFlag()
{
    // Remove marker file
    unlink(impl_->upgradeMarkerPath.c_str());
    
    // Clear U-Boot env
    system("fw_setenv fw_update 2>/dev/null");
    system("fw_setenv fw_update_addr 2>/dev/null");
    system("fw_setenv fw_update_size 2>/dev/null");
    
    return true;
}

bool SystemController::syncFilesystems()
{
    spdlog::info("SystemController: Syncing filesystems");
    
    sync();
    sync();
    sync();
    
    // Remount read-only if possible (safer for reboot)
    // Note: This might fail if files are in use
    // mount(nullptr, "/", nullptr, MS_REMOUNT | MS_RDONLY, nullptr);
    
    return true;
}

void SystemController::reboot(RebootMode mode)
{
    spdlog::info("SystemController: Rebooting system (mode={})",
                 static_cast<int>(mode));
    
    // Final sync
    syncFilesystems();
    
    // Give time for logs to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    switch (mode) {
        case RebootMode::Warm:
            // Standard reboot
            ::reboot(LINUX_REBOOT_CMD_RESTART);
            break;
            
        case RebootMode::Cold:
            // Power cycle if supported
            ::reboot(LINUX_REBOOT_CMD_POWER_OFF);
            break;
            
        case RebootMode::Recovery:
            // Set recovery flag then reboot
            system("fw_setenv bootmode recovery 2>/dev/null");
            ::reboot(LINUX_REBOOT_CMD_RESTART);
            break;
            
        case RebootMode::FactoryReset:
            // Set factory reset flag
            system("fw_setenv factory_reset 1 2>/dev/null");
            ::reboot(LINUX_REBOOT_CMD_RESTART);
            break;
    }
    
    // Should not reach here
    spdlog::error("Reboot failed!");
}

std::string SystemController::getBootloaderVersion() const
{
    std::string version = "unknown";
    
    // Try reading from /proc
    std::ifstream verFile("/proc/device-tree/chosen/u-boot,version");
    if (verFile) {
        std::getline(verFile, version);
        return version;
    }
    
    // Try fw_printenv
    FILE* pipe = popen("fw_printenv ver 2>/dev/null | cut -d= -f2", "r");
    if (pipe) {
        char buf[64];
        if (fgets(buf, sizeof(buf), pipe)) {
            version = buf;
            // Remove trailing newline
            if (!version.empty() && version.back() == '\n') {
                version.pop_back();
            }
        }
        pclose(pipe);
    }
    
    return version;
}

std::string SystemController::getCurrentFirmwareVersion() const
{
    std::string version = "unknown";
    
    // Check various locations
    std::array<const char*, 4> versionFiles = {
        "/etc/firmware_version",
        "/etc/version",
        "/proc/device-tree/firmware/version",
        "/sys/firmware/devicetree/base/firmware/version"
    };
    
    for (const auto* path : versionFiles) {
        std::ifstream f(path);
        if (f) {
            std::getline(f, version);
            if (!version.empty()) {
                return version;
            }
        }
    }
    
    return version;
}

size_t SystemController::getAvailableMemory() const
{
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo) {
        return 0;
    }
    
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.compare(0, 13, "MemAvailable:") == 0) {
            size_t kb = 0;
            sscanf(line.c_str(), "MemAvailable: %zu", &kb);
            return kb * 1024;  // Convert to bytes
        }
    }
    
    return 0;
}

size_t SystemController::getAvailableStorage(const std::string& path) const
{
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) != 0) {
        return 0;
    }
    
    return stat.f_bavail * stat.f_frsize;
}

bool SystemController::isUpgradeInProgress() const
{
    struct stat st;
    return stat(impl_->upgradeMarkerPath.c_str(), &st) == 0;
}

} // namespace ipcam::upgrade
