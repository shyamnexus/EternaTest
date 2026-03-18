#pragma once

#include <string>
#include "ipcam/result.h"

namespace ipcam {
namespace streaming {

// Use the shared Result type from utils
using ipcam::Result;

/**
 * @brief Manager for go2rtc streaming server lifecycle
 * 
 * Provides initialization, shutdown, and status monitoring for the go2rtc
 * media gateway server. This is a singleton class that manages the go2rtc
 * process lifecycle independently from other system components.
 */
class Go2rtcManager {
public:
    /**
     * Get singleton instance
     */
    static Go2rtcManager& Instance();

    // Delete copy/move constructors
    Go2rtcManager(const Go2rtcManager&) = delete;
    Go2rtcManager& operator=(const Go2rtcManager&) = delete;
    Go2rtcManager(Go2rtcManager&&) = delete;
    Go2rtcManager& operator=(Go2rtcManager&&) = delete;

    /**
     * Initialize and start go2rtc server
     * @param config_path Path to go2rtc.yaml config file (optional)
     * @return Result indicating success or error
     */
    Result<void> Start(const std::string& config_path = "/etc/go2rtc.yaml");

    /**
     * Stop go2rtc server
     * @return Result indicating success or error
     */
    Result<void> Stop();

    /**
     * Force stop go2rtc server using SIGKILL
     * Use this if graceful Stop() fails
     */
    void ForceStop();

    /**
     * Restart go2rtc server
     * @return Result indicating success or error
     */
    Result<void> Restart();

    /**
     * Generate/regenerate go2rtc.yaml config file with current SSL paths
     * @param output_path Path to write config file (default: /etc/go2rtc.yaml)
     * @return Result indicating success or error
     */
    Result<void> GenerateConfig(const std::string& output_path = "/etc/go2rtc.yaml");

    /**
     * Check if go2rtc is running
     * @return true if running, false otherwise
     */
    bool IsRunning() const;

    /**
     * Get go2rtc process ID
     * @return PID if running, -1 otherwise
     */
    int GetPid() const;

private:
    Go2rtcManager() = default;
    ~Go2rtcManager() = default;

    bool CheckBinaryExists() const;
    bool WaitForProcess(int timeout_ms = 1000) const;
};

} // namespace streaming
} // namespace ipcam
