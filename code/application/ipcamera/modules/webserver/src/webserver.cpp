#include "ipcam/webserver.h"
#include "api_handlers.h"
#include <spdlog/spdlog.h>
#include <ipcam/config.h>
#include <ipcam/audit_logger.h>
#include <memory>
#include <atomic>
#include <CivetServer.h>

namespace ipcam {
namespace webserver {

namespace {
  std::unique_ptr<CivetServer> g_server;
  std::atomic<bool> g_running{false};
  Config g_config;
  Stats g_stats;
}

bool Init(std::string& outError) {
  if (g_running) {
    outError = "Webserver already running";
    return false;
  }
  
  spdlog::info("Initializing webserver...");
  
  // Load configuration from config module
  g_config.mode = TransportMode::UNIX_IPC;
  g_config.unix_ipc.socket_path = config::Get<std::string>("web_portal.unix_socket.path", "/var/run/ipcam_api.sock");
  g_config.unix_ipc.max_connections = config::Get<int>("web_portal.unix_socket.max_connections", 10);
  g_config.direct.num_threads = config::Get<int>("web_portal.http.num_threads", 4);
  g_config.direct.enable_directory_listing = config::Get<bool>("web_portal.http.enable_directory_listing", false);
  g_config.direct.document_root = config::Get<std::string>("web_portal.http.document_root", "");
  
  if (g_config.mode == TransportMode::UNIX_IPC) {
    try {
      // Remove old socket if it exists
      unlink(g_config.unix_ipc.socket_path.c_str());
      
      // Configure CivetWeb to listen on Unix socket
      std::vector<std::string> options;
      options.push_back("listening_ports");
      options.push_back("x" + g_config.unix_ipc.socket_path);  // 'x' prefix for Unix domain socket
      options.push_back("num_threads");
      options.push_back(std::to_string(g_config.direct.num_threads));
      options.push_back("enable_directory_listing");
      options.push_back(g_config.direct.enable_directory_listing ? "yes" : "no");
      
      if (!g_config.direct.document_root.empty()) {
        options.push_back("document_root");
        options.push_back(g_config.direct.document_root);
      }
      
      // Authentication settings
      bool auth_enabled = config::Get<bool>("web_portal.authentication.enabled", true);
      if (auth_enabled) {
        std::string auth_domain = config::Get<std::string>("web_portal.authentication.domain", "ipcamera");
        options.push_back("authentication_domain");
        options.push_back(auth_domain);
        
        // For now, we'll implement custom auth handler
        // CivetWeb supports .htpasswd files, but we'll use our database
      }
      
      // Enable keep-alive
      options.push_back("enable_keep_alive");
      options.push_back("yes");
      
      // Request timeout - increased for firmware upload operations
      // For large firmware uploads (50-100MB), use nginx upload module instead
      int timeout = config::Get<int>("web_portal.http.request_timeout_ms", 300000);  // 5 minutes
      options.push_back("request_timeout_ms");
      options.push_back(std::to_string(timeout));
      
      // Max request size: 128MB to accommodate firmware binaries
      // NOTE: For memory-constrained systems, use nginx's upload module to save
      // firmware to disk, then call /api/v1/firmware/upgrade with {"file": "/path"}
      int max_request_size = config::Get<int>("web_portal.http.max_request_size", 134217728);  // 128MB
      options.push_back("max_request_size");
      options.push_back(std::to_string(max_request_size));
      
      spdlog::info("Starting CivetWeb server on Unix socket: {}", 
                   g_config.unix_ipc.socket_path);
      
      // Create server
      g_server = std::make_unique<CivetServer>(options);
      
      // Set socket permissions to allow nginx to connect
      chmod(g_config.unix_ipc.socket_path.c_str(), 0666);
      
      // Initialize AuditLogger for access/security/audit logging
      {
          ipcam::AuditLogConfig acfg;
          acfg.access_path         = config::Get<std::string>("logging.access.path", "/var/log/ipcamd_access.log");
          acfg.security_path       = config::Get<std::string>("logging.security.path", "/var/log/ipcamd_security.log");
          acfg.audit_path          = config::Get<std::string>("logging.audit.path", "/var/log/ipcamd_audit.log");
          acfg.access_max_size_mb  = config::Get<int>("logging.access.max_size_mb", 10);
          acfg.security_max_size_mb= config::Get<int>("logging.security.max_size_mb", 10);
          acfg.audit_max_size_mb   = config::Get<int>("logging.audit.max_size_mb", 10);
          acfg.syslog_enabled      = config::Get<bool>("logging.syslog.enabled", false);
          acfg.syslog_server       = config::Get<std::string>("logging.syslog.host", "");
          acfg.syslog_port         = config::Get<int>("logging.syslog.port", 514);
          acfg.syslog_protocol     = config::Get<std::string>("logging.syslog.protocol", "udp");
          acfg.syslog_facility     = config::Get<std::string>("logging.syslog.facility", "local0");
          acfg.syslog_min_level    = config::Get<std::string>("logging.syslog.min_level", "warning");
          ipcam::AuditLogger::Instance().Init(acfg);
          spdlog::info("AuditLogger initialized — access={}, security={}, audit={}",
                       acfg.access_path, acfg.security_path, acfg.audit_path);
      }
      
      // Register API handlers
      api::RegisterHandlers(g_server.get());
      
      g_running = true;
      spdlog::info("Webserver started successfully");
      return true;
      
    } catch (const CivetException& e) {
      outError = std::string("CivetWeb error: ") + e.what();
      spdlog::error("Failed to start webserver: {}", outError);
      return false;
    } catch (const std::exception& e) {
      outError = std::string("Unexpected error: ") + e.what();
      spdlog::error("Failed to start webserver: {}", outError);
      return false;
    }
  } else if (g_config.mode == TransportMode::DIRECT) {
    outError = "DIRECT mode disabled, please use UNIX_IPC mode";
    spdlog::warn("DIRECT transport mode is disabled");
    return false;
  }
  
  // Should never reach here
  outError = "Unknown transport mode";
  return false;
}

void Shutdown() {
  if (!g_running) {
    return;
  }
  
  spdlog::info("Shutting down webserver...");
  
  // Shutdown AuditLogger (flush pending writes)
  ipcam::AuditLogger::Instance().Shutdown();
  
  if (g_server) {
    g_server.reset();
  }
  
  // Clean up Unix socket
  if (g_config.mode == TransportMode::UNIX_IPC) {
    unlink(g_config.unix_ipc.socket_path.c_str());
  }
  
  g_running = false;
  spdlog::info("Webserver shutdown complete");
}

TransportMode GetMode() {
  return g_config.mode;
}

bool IsRunning() {
  return g_running;
}

Stats GetStats() {
  // CivetWeb provides server stats, we can enhance this later
  return g_stats;
}

} // namespace webserver
} // namespace ipcam
