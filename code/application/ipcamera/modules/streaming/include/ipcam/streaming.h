/**
 * @file streaming.h
 * @brief Streaming Module Public Interface
 * 
 * Provides RTSP streaming capabilities using Live555 library
 * with direct HDAL encoder buffer access for low latency.
 */

#pragma once

// Legacy go2rtc manager (to be deprecated)
#include "go2rtc_manager.h"

// New Live555 RTSP server with full control
#include "rtsp_server.h"

namespace ipcam {
namespace streaming {

inline const char* streaming_info() { return "streaming"; }

/**
 * @brief Initialize streaming module
 * 
 * Loads configuration and prepares RTSP server.
 * Call this after HDAL pipeline is running.
 * 
 * @return true on success
 */
bool Initialize();

/**
 * @brief Start streaming services
 * 
 * Starts the RTSP server in background thread.
 * 
 * @return true on success
 */
bool Start();

/**
 * @brief Stop streaming services
 */
void Stop();

/**
 * @brief Check if streaming is active
 */
bool IsRunning();

} // namespace streaming
} // namespace ipcam
