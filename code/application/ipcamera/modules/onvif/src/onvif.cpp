/**
 * @file onvif.cpp
 * @brief C++ wrapper for ONVIF module integration with ipcamera application
 * 
 * This file provides a clean C++ interface to the C-based ONVIF implementation,
 * managing thread lifecycle and providing integration with the ipcamera framework.
 */

#include "ipcam/onvif.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <cstring>

// Include the C ONVIF headers
extern "C" {
#include "onvif_main.h"

// Thread entry points from onvif_main.c
void *OnvifWSDiscoveryServer(void *arg);
void *OnvifWebServiesServer(void *arg);
void DiscoveryHello(void);
int get_ip_address(void);
int get_hw_addr(char *ifname, unsigned char *mac);
}

namespace ipcam {
namespace onvif {

// Module state
static std::atomic<bool> s_running{false};
static std::atomic<bool> s_initialized{false};
static std::mutex s_mutex;
static std::thread s_discovery_thread;
static std::thread s_services_thread;
static Config s_config;

Config GetConfig() {
  std::lock_guard<std::mutex> lock(s_mutex);
  return s_config;
}

bool Init(std::string& outError) {
  std::lock_guard<std::mutex> lock(s_mutex);
  
  if (s_initialized) {
    spdlog::warn("ONVIF module already initialized");
    return true;
  }
  
  spdlog::info("Initializing ONVIF module...");
  
  // Load configuration from config module
  s_config.enabled = config::Get<bool>("onvif.enabled", true);
  s_config.discovery.enabled = config::Get<bool>("onvif.discovery.enabled", true);
  s_config.discovery.udp_port = config::Get<int>("onvif.discovery.udp_port", 3702);
  s_config.services.tcp_port = config::Get<int>("onvif.services.tcp_port", 5000);
  s_config.services.timeout_sec = config::Get<int>("onvif.services.timeout_sec", 10);
  s_config.device_info.manufacturer = config::Get<std::string>("onvif.device_info.manufacturer", "Honeywell");
  s_config.device_info.model = config::Get<std::string>("onvif.device_info.model", "IPCAM-5MP");
  s_config.device_info.firmware_version = config::Get<std::string>("onvif.device_info.firmware_version", "v1.0.0");
  s_config.device_info.serial_number = config::Get<std::string>("onvif.device_info.serial_number", "");
  s_config.device_info.hardware_id = config::Get<std::string>("onvif.device_info.hardware_id", "Rev C");
  s_config.interface_name = config::Get<std::string>("onvif.interface_name", "eth0");
  
  if (!s_config.enabled) {
    spdlog::info("ONVIF module disabled in configuration");
    s_initialized = true;
    return true;
  }
  
  // Initialize network information for ONVIF
  if (get_ip_address() < 0) {
    spdlog::warn("Failed to get IP address for ONVIF, using default");
  }
  
  // Get MAC address
  if (get_hw_addr(const_cast<char*>(s_config.interface_name.c_str()), macAddress) < 0) {
    spdlog::warn("Failed to get MAC address for ONVIF");
  } else {
    spdlog::info("ONVIF MAC address: {:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}",
                 macAddress[0], macAddress[1], macAddress[2],
                 macAddress[3], macAddress[4], macAddress[5]);
  }
  
  // Set the quit flag to false
  g_b_IsSysQuit = false;
  
  // Start discovery thread if enabled
  if (s_config.discovery.enabled) {
    try {
      s_discovery_thread = std::thread([]() {
        spdlog::info("ONVIF WS-Discovery thread starting on port {}...", s_config.discovery.udp_port);
        OnvifWSDiscoveryServer(nullptr);
      });
    } catch (const std::exception& e) {
      outError = std::string("Failed to start WS-Discovery thread: ") + e.what();
      spdlog::error(outError);
      return false;
    }
    spdlog::info("ONVIF WS-Discovery server started");
  }
  
  // Start web services thread
  try {
    s_services_thread = std::thread([]() {
      spdlog::info("ONVIF Web Services thread starting on port {}...", s_config.services.tcp_port);
      OnvifWebServiesServer(nullptr);
    });
  } catch (const std::exception& e) {
    outError = std::string("Failed to start ONVIF web services thread: ") + e.what();
    spdlog::error(outError);
    // Stop discovery thread if it was started
    if (s_config.discovery.enabled && s_discovery_thread.joinable()) {
      g_b_IsSysQuit = true;
      // Note: The discovery thread may need additional signal to stop
      s_discovery_thread.detach();
    }
    return false;
  }
  
  s_running = true;
  s_initialized = true;
  
  spdlog::info("ONVIF module initialized successfully");
  spdlog::info("  Discovery: {} (port {})", 
               s_config.discovery.enabled ? "enabled" : "disabled",
               s_config.discovery.udp_port);
  spdlog::info("  Web Services: port {}", s_config.services.tcp_port);
  
  return true;
}

void Shutdown() {
  std::lock_guard<std::mutex> lock(s_mutex);
  
  if (!s_initialized) {
    return;
  }
  
  spdlog::info("Shutting down ONVIF module...");
  
  // Signal threads to stop
  g_b_IsSysQuit = true;
  s_running = false;
  
  // Detach threads - they will exit on their own when g_b_IsSysQuit is checked
  // Note: The current ONVIF implementation uses blocking accept() calls,
  // so we detach rather than join to avoid blocking shutdown
  if (s_discovery_thread.joinable()) {
    s_discovery_thread.detach();
    spdlog::debug("ONVIF discovery thread detached");
  }
  
  if (s_services_thread.joinable()) {
    s_services_thread.detach();
    spdlog::debug("ONVIF services thread detached");
  }
  
  s_initialized = false;
  spdlog::info("ONVIF module shutdown complete");
}

bool IsRunning() {
  return s_running && s_initialized;
}

void SendHello() {
  if (!IsRunning()) {
    spdlog::warn("Cannot send Hello - ONVIF not running");
    return;
  }
  
  spdlog::debug("Sending ONVIF WS-Discovery Hello message");
  DiscoveryHello();
}

void NotifyIpChanged() {
  if (!IsRunning()) {
    return;
  }
  
  spdlog::info("ONVIF notified of IP address change");
  
  // Update the IP address
  get_ip_address();
  
  // Set the flag to trigger socket rebind in the services thread
  ip_address_update_flag = true;
  
  // Send Hello to announce new address
  SendHello();
}

} // namespace onvif
} // namespace ipcam
