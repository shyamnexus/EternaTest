/**
 * @file snapshot_action.cpp
 * @brief Snapshot capture action handler implementation using VideoControl
 */

#include "ipcam/action_handler.h"
#include "ipcam/video_control.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace fs = std::filesystem;

namespace ipcam {
namespace events {

/**
 * @brief Generate snapshot file path based on event and index
 */
static std::string GenerateSnapshotPath(const std::string& base_path, 
                                         const std::string& event_id, 
                                         int index) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time_t);
    
    // Format: base_path/YYYYMMDD/event_id_index.jpg
    std::ostringstream oss;
    oss << base_path << "/" 
        << std::setfill('0') << std::setw(4) << (tm->tm_year + 1900)
        << std::setw(2) << (tm->tm_mon + 1) 
        << std::setw(2) << tm->tm_mday << "/"
        << event_id << "_" << index << ".jpg";
    
    return oss.str();
}

ActionResult SnapshotActionHandler::Execute(const Event& event, const Action& action) {
    ActionResult result;
    result.execution_time_ms = 0;
    
    const auto* cfg = action.GetConfig<SnapshotActionConfig>();
    if (!cfg) {
        result.success = false;
        result.error_message = "Invalid configuration for snapshot action";
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    spdlog::info("[SnapshotActionHandler] Capturing {} snapshot(s) for event: {}", 
                 cfg->count, event.id);
    
    // Get base path
    std::string base_path = cfg->save_path;
    if (base_path.empty()) {
        base_path = "/mnt/sd/snapshots";
    }
    
    // Ensure base directory exists
    try {
        fs::create_directories(base_path);
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Failed to create snapshot directory: ") + e.what();
        return result;
    }
    
    // Get VideoControl instance
    auto& video_ctrl = media::VideoControl::Instance();
    
    if (!video_ctrl.IsInitialized()) {
        result.success = false;
        result.error_message = "VideoControl not initialized";
        return result;
    }
    
    // Configure snapshot quality if specified
    media::SnapshotConfig snap_config = video_ctrl.GetSnapshotConfig();
    if (cfg->quality > 0 && cfg->quality <= 100) {
        snap_config.quality = cfg->quality;
        video_ctrl.SetSnapshotConfig(snap_config);
    }
    
    int success_count = 0;
    std::vector<std::string> captured_paths;
    
    for (int i = 0; i < cfg->count; ++i) {
        std::string snapshot_path = GenerateSnapshotPath(base_path, event.id, i);
        
        // Ensure date directory exists
        fs::path path_obj(snapshot_path);
        fs::create_directories(path_obj.parent_path());
        
        // Capture snapshot using VideoControl
        std::vector<uint8_t> jpeg_data = video_ctrl.CaptureSnapshot();
        
        if (!jpeg_data.empty()) {
            // Save to file
            try {
                std::ofstream file(snapshot_path, std::ios::binary);
                if (file.is_open()) {
                    file.write(reinterpret_cast<const char*>(jpeg_data.data()), jpeg_data.size());
                    file.close();
                    
                    success_count++;
                    captured_paths.push_back(snapshot_path);
                    spdlog::debug("[SnapshotActionHandler] Saved snapshot to: {} ({} bytes)",
                                 snapshot_path, jpeg_data.size());
                } else {
                    spdlog::warn("[SnapshotActionHandler] Failed to open file for writing: {}", 
                                snapshot_path);
                }
            } catch (const std::exception& e) {
                spdlog::warn("[SnapshotActionHandler] Failed to save snapshot: {}", e.what());
            }
        } else {
            spdlog::warn("[SnapshotActionHandler] CaptureSnapshot returned empty data");
        }
        
        // Wait between captures if taking multiple
        if (i < cfg->count - 1 && cfg->interval_ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(cfg->interval_ms));
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    if (success_count > 0) {
        result.success = true;
        result.output = captured_paths.front();  // Primary snapshot path
        result.metadata["snapshots_captured"] = std::to_string(success_count);
        result.metadata["total_requested"] = std::to_string(cfg->count);
    } else {
        result.success = false;
        result.error_message = "Failed to capture any snapshots";
    }
    
    return result;
}

bool SnapshotActionHandler::IsAvailable() const {
    auto& video_ctrl = media::VideoControl::Instance();
    return video_ctrl.IsInitialized();
}

} // namespace events
} // namespace ipcam
