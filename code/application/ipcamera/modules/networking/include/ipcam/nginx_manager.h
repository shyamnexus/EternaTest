#pragma once

#include "ipcam/result.h"
#include <string>

namespace ipcam {
namespace networking {

/**
 * @brief NginxManager - Manages nginx web server
 * 
 * Handles nginx configuration generation and process management
 * independently from SSL certificates and network configuration.
 */
class NginxManager {
public:
    static NginxManager& Instance();
    
    /**
     * Generate nginx configuration file
     * @param output_path Path to write nginx.conf (default: /etc/nginx/nginx.conf)
     * @return Result with success or error message
     */
    Result<void> GenerateConfig(const std::string& output_path = "/usr/conf/nginx.conf");
    
    /**
     * Reload nginx configuration
     * Only reloads if nginx is already running
     * @return Result with success or error message
     */
    Result<void> Reload();
    
    /**
     * Start nginx server
     * @return Result with success or error message
     */
    Result<void> Start();
    
    /**
     * Stop nginx server
     * @return Result with success or error message
     */
    Result<void> Stop();
    
    /**
     * Restart nginx server
     * @return Result with success or error message
     */
    Result<void> Restart();
    
    /**
     * Check if nginx is currently running
     * @return true if nginx process exists
     */
    bool IsRunning() const;

private:
    NginxManager();
    ~NginxManager();
    NginxManager(const NginxManager&) = delete;
    NginxManager& operator=(const NginxManager&) = delete;
    
    /**
     * Wait for nginx to start with retries
     * @param max_retries Maximum number of attempts
     * @return true if nginx started successfully
     */
    bool WaitForStart(int max_retries) const;
};

} // namespace networking
} // namespace ipcam
