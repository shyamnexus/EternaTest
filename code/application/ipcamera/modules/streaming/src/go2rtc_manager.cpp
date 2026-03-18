#include "ipcam/go2rtc_manager.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <fstream>
#include <sys/stat.h>

namespace ipcam {
namespace streaming {

Go2rtcManager& Go2rtcManager::Instance() {
    static Go2rtcManager instance;
    return instance;
}

bool Go2rtcManager::CheckBinaryExists() const {
    struct stat buffer;
    return (stat("/bin/go2rtc_linux_arm64", &buffer) == 0);
}

bool Go2rtcManager::WaitForProcess(int timeout_ms) const {
    int elapsed = 0;
    const int interval = 100;
    
    while (elapsed < timeout_ms) {
        if (IsRunning()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        elapsed += interval;
    }
    return false;
}

bool Go2rtcManager::IsRunning() const {
    int rc = system("pgrep -f go2rtc_linux_arm64 > /dev/null 2>&1");
    return (rc == 0);
}

int Go2rtcManager::GetPid() const {
    if (!IsRunning()) {
        return -1;
    }
    
    FILE* fp = popen("pgrep -f go2rtc_linux_arm64", "r");
    if (!fp) {
        return -1;
    }
    
    int pid = -1;
    if (fscanf(fp, "%d", &pid) != 1) {
        pid = -1;
    }
    
    pclose(fp);
    return pid;
}

Result<void> Go2rtcManager::Start(const std::string& config_path) {
    spdlog::info("Starting go2rtc streaming server...");
    
    // Check if already running
    if (IsRunning()) {
        spdlog::info("go2rtc is already running (PID: {})", GetPid());
        return Result<void>::Ok();
    }
    
    // Get paths from config with fallback to defaults
    std::string binary_path = config::Get<std::string>("streaming.go2rtc.binary_path", "/bin/go2rtc_linux_arm64");
    std::string init_script = config::Get<std::string>("streaming.go2rtc.init_script", "/etc/init.d/S51go2rtc");
    std::string cfg_path = config::Get<std::string>("streaming.go2rtc.config_path", config_path);
    
    // Check if binary exists
    struct stat buffer;
    if (stat(binary_path.c_str(), &buffer) != 0) {
        return Result<void>::Err("go2rtc binary not found at " + binary_path);
    }
    
    // Check if config file exists
    bool config_exists = (stat(cfg_path.c_str(), &buffer) == 0);
    if (!config_exists) {
        spdlog::warn("go2rtc config not found at {}, starting with default config", cfg_path);
    }
    
    // Start go2rtc using init script
    std::string cmd = init_script + " start 2>&1";
    int rc = system(cmd.c_str());
    if (rc != 0) {
        return Result<void>::Err("Failed to start go2rtc (exit code: " + std::to_string(rc) + ")");
    }
    
    // Wait for process to start and verify
    if (!WaitForProcess(2000)) {
        return Result<void>::Err("go2rtc command succeeded but process did not start within timeout");
    }
    
    spdlog::info("go2rtc started successfully (PID: {})", GetPid());
    return Result<void>::Ok();
}

Result<void> Go2rtcManager::Stop() {
    spdlog::info("Stopping go2rtc streaming server...");
    
    // Check if running
    if (!IsRunning()) {
        spdlog::info("go2rtc is not running");
        return Result<void>::Ok();
    }
    
    int pid = GetPid();
    
    // Get init script path from config
    std::string init_script = config::Get<std::string>("streaming.go2rtc.init_script", "/etc/init.d/S51go2rtc");
    
    // Try stop via init script first
    std::string cmd = init_script + " stop 2>&1";
    int rc = system(cmd.c_str());
    if (rc != 0) {
        spdlog::warn("Init script stop failed (exit code: {}), trying direct kill", rc);
    }
    
    // Wait for process to stop
    int timeout = 3000; // 3 seconds
    int elapsed = 0;
    while (IsRunning() && elapsed < timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed += 100;
    }
    
    // If still running, force kill
    if (IsRunning()) {
        spdlog::warn("go2rtc did not stop gracefully, forcing kill...");
        ForceStop();
        
        // Wait again
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (IsRunning()) {
            return Result<void>::Err("go2rtc process still running after force kill");
        }
    }
    
    spdlog::info("go2rtc stopped successfully (was PID: {})", pid);
    return Result<void>::Ok();
}

void Go2rtcManager::ForceStop() {
    // Kill using pkill with SIGKILL
    int rc = system("pkill -9 -f go2rtc_linux_arm64 2>/dev/null");
    (void)rc;  // Ignore return code, process may already be dead
    
    // Also clean up stale PID file
    system("rm -f /var/run/go2rtc.pid 2>/dev/null");
}

Result<void> Go2rtcManager::Restart() {
    spdlog::info("Restarting go2rtc streaming server...");
    
    // Regenerate config to pick up any SSL cert path changes
    auto gen_result = GenerateConfig();
    if (!gen_result.success) {
        spdlog::warn("Failed to regenerate go2rtc config: {}", gen_result.error);
        // Continue with restart anyway - config might still be valid
    }
    
    auto stop_result = Stop();
    if (!stop_result.success) {
        return stop_result;
    }
    
    // Brief delay to ensure clean restart
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    return Start();
}

Result<void> Go2rtcManager::GenerateConfig(const std::string& output_path) {
    spdlog::info("Generating go2rtc configuration at {}", output_path);
    
    // go2rtc runs on HTTP port 1984
    // nginx proxies /go2rtc/ from port 443 to port 1984, handling TLS termination
    
    // Get RTSP port from config
    int rtsp_port = config::Get<int>("streaming.rtsp.port", 554);
    
    // go2rtc connects via localhost (127.0.0.1), which bypasses RTSP authentication
    // The RTSP server allows localhost connections without credentials
    // This works regardless of auth mode (none/basic/digest)
    std::string rtsp_base = "rtsp://127.0.0.1";
    
    // Add port if non-standard
    if (rtsp_port != 554) {
        rtsp_base += ":" + std::to_string(rtsp_port);
    }
    
    spdlog::debug("go2rtc: Using localhost RTSP URLs (auth bypass enabled)");
    
    // Build YAML config
    std::ofstream config_file(output_path, std::ios::trunc);
    if (!config_file) {
        return Result<void>::Err("Failed to open " + output_path + " for writing");
    }
    
    config_file << "# go2rtc configuration file\n";
    config_file << "# Auto-generated by ipcamd - do not edit manually\n";
    config_file << "# TLS is handled by nginx proxy (/go2rtc/ on port 443 -> port 1984)\n";
    config_file << "# Localhost connections bypass RTSP auth\n\n";
    
    config_file << "streams:\n";
    config_file << "  camera1:\n";
    config_file << "    - " << rtsp_base << "/stream0  # Main stream\n";
    config_file << "  stream0:\n";
    config_file << "    - " << rtsp_base << "/stream0\n";
    config_file << "  stream1:\n";
    config_file << "    - " << rtsp_base << "/stream1\n";
    config_file << "  stream2:\n";
    config_file << "    - " << rtsp_base << "/stream2\n\n";
    
    config_file << "api:\n";
    config_file << "  listen: \":1984\"  # HTTP port (nginx proxies /go2rtc/ from port 443 -> here)\n";
    config_file << "  origin: \"*\"  # Allow all origins (CORS fix)\n";
    
    config_file.close();
    
    if (config_file.fail()) {
        return Result<void>::Err("Failed to write go2rtc config file");
    }
    
    spdlog::info("go2rtc configuration generated successfully (localhost auth bypass)");
    return Result<void>::Ok();
}

} // namespace streaming
} // namespace ipcam
