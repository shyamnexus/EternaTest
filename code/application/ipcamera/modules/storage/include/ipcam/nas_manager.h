/**
 * @file nas_manager.h
 * @brief Network Attached Storage (NAS) Manager for IP Camera
 * 
 * Manages network storage with:
 * - SMB/CIFS and NFS protocol support
 * - Auto-mount on startup
 * - Connection testing
 * - Failover to local storage
 */

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <optional>

namespace ipcam {
namespace storage {

// ============================================================================
// NAS Types and Enums
// ============================================================================

/**
 * NAS protocol types
 */
enum class NasProtocol {
    SMB,        ///< SMB/CIFS (Windows share)
    NFS,        ///< NFS (Unix/Linux)
    CIFS        ///< CIFS (same as SMB)
};

/**
 * Convert NasProtocol to string
 */
inline std::string NasProtocolToString(NasProtocol type) {
    switch (type) {
        case NasProtocol::SMB: return "smb";
        case NasProtocol::NFS: return "nfs";
        case NasProtocol::CIFS: return "cifs";
        default: return "smb";
    }
}

/**
 * Parse NasProtocol from string
 */
inline NasProtocol StringToNasProtocol(const std::string& str) {
    if (str == "nfs") return NasProtocol::NFS;
    if (str == "cifs") return NasProtocol::CIFS;
    return NasProtocol::SMB;
}

/**
 * NAS connection status
 */
enum class NasStatus {
    Disconnected,   ///< Not connected
    Connected,      ///< Tested OK, not mounted
    Mounted,        ///< Mounted and ready
    Error           ///< Connection error
};

inline std::string NasStatusToString(NasStatus status) {
    switch (status) {
        case NasStatus::Disconnected: return "disconnected";
        case NasStatus::Connected: return "connected";
        case NasStatus::Mounted: return "mounted";
        case NasStatus::Error: return "error";
        default: return "disconnected";
    }
}

// ============================================================================
// NAS Configuration Structure
// ============================================================================

/**
 * NAS configuration
 */
struct NasConfig {
    bool enabled = false;                    ///< Whether NAS is enabled
    NasProtocol type = NasProtocol::SMB;     ///< Protocol type
    std::string server;                      ///< Server hostname or IP
    std::string share_path;                  ///< Share name or path
    std::string mount_point = "/mnt/nas";    ///< Local mount point
    std::string username;                    ///< Username (SMB/CIFS)
    std::string password;                    ///< Password (SMB/CIFS)
    std::string domain;                      ///< Domain/workgroup (SMB)
    bool auto_mount = true;                  ///< Auto-mount on startup
    NasStatus status = NasStatus::Disconnected;
    std::string last_error;                  ///< Last error message
};

// ============================================================================
// NAS Manager Class
// ============================================================================

/**
 * Manages NAS connections and mounting
 */
class NasManager {
public:
    /**
     * Get singleton instance
     */
    static NasManager& Instance();
    
    /**
     * Initialize NAS manager
     * Loads configuration from recording.nas config section
     * @return true on success
     */
    bool Init();
    
    /**
     * Shutdown and cleanup
     */
    void Shutdown();
    
    /**
     * Check if initialized
     */
    bool IsInitialized() const { return initialized_.load(); }
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * Get current NAS configuration
     */
    NasConfig GetConfig() const;
    
    /**
     * Update NAS configuration
     * @return true on success
     */
    bool SetConfig(const NasConfig& config);
    
    /**
     * Get current connection status
     */
    NasStatus GetStatus() const;
    
    /**
     * Check if NAS is mounted
     */
    bool IsMounted() const;
    
    // ========================================================================
    // Connection Operations
    // ========================================================================
    
    /**
     * Test connection to NAS without mounting
     * @param config Config to test (or current if empty)
     * @return true if connection successful
     */
    bool TestConnection(const NasConfig* config = nullptr);
    
    /**
     * Mount the NAS share
     * @return true on success
     */
    bool Mount();
    
    /**
     * Unmount the NAS share
     * @return true on success
     */
    bool Unmount();
    
    /**
     * Get storage info for mounted NAS
     * @param total_bytes Total space
     * @param free_bytes Free space
     * @return true if info available
     */
    bool GetStorageInfo(int64_t& total_bytes, int64_t& free_bytes) const;
    
    /**
     * Get last error message
     */
    std::string GetLastError() const;
    
    // ========================================================================
    // Persistence
    // ========================================================================
    
    /**
     * Save configuration to file
     */
    bool SaveConfig();
    
    /**
     * Load configuration from file
     */
    bool LoadConfig();
    
private:
    NasManager() = default;
    ~NasManager() = default;
    NasManager(const NasManager&) = delete;
    NasManager& operator=(const NasManager&) = delete;
    
    /**
     * Build mount command based on protocol
     */
    std::string BuildMountCommand(const NasConfig& config) const;
    
    /**
     * Check if mount point is currently mounted
     */
    bool IsMountPointMounted(const std::string& mount_point) const;
    
    /**
     * Create mount point directory if needed
     */
    bool EnsureMountPoint(const std::string& mount_point);
    
    std::atomic<bool> initialized_{false};
    mutable std::mutex mutex_;
    NasConfig config_;
};

} // namespace storage
} // namespace ipcam
