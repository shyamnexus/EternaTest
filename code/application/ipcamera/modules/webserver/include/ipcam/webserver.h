#pragma once
#include <string>

namespace ipcam {
namespace webserver {

// Transport mode for the web server
// DIRECT: CivetWeb serves HTTP directly (current implementation)
// UNIX_IPC: Future mode where API requests are forwarded via Unix socket to nginx
enum class TransportMode {
  DIRECT,
  UNIX_IPC
};

// Configuration for the web server
struct Config {
  TransportMode mode = TransportMode::DIRECT;
  
  // Direct mode configuration
  struct {
    std::string listen_address = "0.0.0.0";
    int port = 8080;
    int num_threads = 4;
    bool enable_directory_listing = false;
    std::string document_root = "";
  } direct;
  
  // Unix IPC mode configuration (for future use)
  struct {
    std::string socket_path = "/var/run/ipcam_api.sock";
    int max_connections = 10;
  } unix_ipc;
};

// Initialize the webserver subsystem
// Reads configuration from the config module and starts the appropriate transport
// Returns true on success; on failure returns false and writes error to outError
bool Init(std::string& outError);

// Shutdown the webserver and release resources
// Safe to call multiple times
void Shutdown();

// Get current transport mode
TransportMode GetMode();

// Check if webserver is running
bool IsRunning();

// Get server statistics (if available)
struct Stats {
  uint64_t total_requests = 0;
  uint64_t active_connections = 0;
  uint64_t bytes_sent = 0;
  uint64_t bytes_received = 0;
};
Stats GetStats();

} // namespace webserver
} // namespace ipcam
