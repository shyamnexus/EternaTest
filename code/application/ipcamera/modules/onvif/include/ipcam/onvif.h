#pragma once
/**
 * @file onvif.h
 * @brief ONVIF module interface for ipcamera application
 * 
 * This module provides ONVIF WS-Discovery and device service support,
 * allowing the camera to be discovered and configured by ONVIF-compliant
 * NVRs and video management software.
 */

#include <string>

namespace ipcam {
namespace onvif {

// ONVIF module configuration
struct Config {
  bool enabled = true;
  
  // Discovery settings
  struct {
    bool enabled = true;          // Enable WS-Discovery
    int udp_port = 3702;          // Standard ONVIF discovery port
  } discovery;
  
  // Web services settings
  struct {
    int tcp_port = 5000;          // ONVIF web services port
    int timeout_sec = 10;         // Connection timeout
  } services;
  
  // Device information
  struct {
    std::string manufacturer = "Honeywell";
    std::string model = "IPCAM-5MP";
    std::string firmware_version = "v1.0.0";
    std::string serial_number = "";  // Auto-detected from MAC if empty
    std::string hardware_id = "Rev C";
  } device_info;
  
  // Network interface to bind to (empty = default)
  std::string interface_name = "eth0";
};

/**
 * @brief Get current ONVIF configuration
 * @return Current configuration structure
 */
Config GetConfig();

/**
 * @brief Initialize the ONVIF module
 * 
 * This starts the WS-Discovery server and ONVIF web services.
 * Must be called after network is initialized.
 * 
 * @param[out] outError Error message if initialization fails
 * @return true on success, false on failure
 */
bool Init(std::string& outError);

/**
 * @brief Shutdown the ONVIF module
 * 
 * Stops all ONVIF services and releases resources.
 * Safe to call multiple times.
 */
void Shutdown();

/**
 * @brief Check if ONVIF module is running
 * @return true if running, false otherwise
 */
bool IsRunning();

/**
 * @brief Trigger WS-Discovery Hello message
 * 
 * Sends a Hello message to announce the device on the network.
 * Useful after IP address changes.
 */
void SendHello();

/**
 * @brief Notify ONVIF module of IP address change
 * 
 * Call this when the device IP address changes to update
 * ONVIF services and send a new Hello message.
 */
void NotifyIpChanged();

} // namespace onvif
} // namespace ipcam
