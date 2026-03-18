/**
 * @file ftp_action.cpp
 * @brief FTP upload action handler implementation using FtpManager
 */

#include "ipcam/action_handler.h"
#include "ipcam/ftp_manager.h"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

namespace ipcam {
namespace events {

ActionResult FtpUploadHandler::Execute(const Event& event, const Action& action) {
    ActionResult result;
    result.execution_time_ms = 0;
    
    const auto* cfg = action.GetConfig<FtpActionConfig>();
    if (!cfg) {
        result.success = false;
        result.error_message = "Invalid configuration for FTP upload action";
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Get FTP manager instance
    auto& ftp_mgr = storage::FtpManager::Instance();
    
    // Check if FTP is configured
    auto ftp_config = ftp_mgr.GetConfig();
    if (!ftp_config.enabled || ftp_config.server.empty()) {
        result.success = false;
        result.error_message = "FTP not configured or disabled";
        return result;
    }
    
    spdlog::info("[FtpUploadHandler] Starting FTP upload for event: {}", event.id);
    
    int queued_count = 0;
    std::vector<std::string> upload_ids;
    
    // Build remote path using template
    std::string remote_base = SubstitutePlaceholders(cfg->remote_path_template, event);
    
    // Queue snapshot upload if configured and available
    if (cfg->upload_snapshot && event.snapshot_path.has_value()) {
        std::string local_path = event.snapshot_path.value();
        
        if (fs::exists(local_path)) {
            // Generate remote filename
            std::string remote_name = event.id + ".jpg";
            std::string remote_path = remote_base + "/" + remote_name;
            
            std::string upload_id = ftp_mgr.QueueUpload(local_path, remote_path);
            if (!upload_id.empty()) {
                queued_count++;
                upload_ids.push_back(upload_id);
                spdlog::debug("[FtpUploadHandler] Queued snapshot upload: {} -> {}", 
                             local_path, remote_path);
            }
        } else {
            spdlog::warn("[FtpUploadHandler] Snapshot file not found: {}", local_path);
        }
    }
    
    // Queue video upload if configured and available
    if (cfg->upload_video && event.video_path.has_value()) {
        std::string local_path = event.video_path.value();
        
        if (fs::exists(local_path)) {
            // Generate remote filename
            std::string remote_name = event.id + ".mp4";
            std::string remote_path = remote_base + "/" + remote_name;
            
            std::string upload_id = ftp_mgr.QueueUpload(local_path, remote_path);
            if (!upload_id.empty()) {
                queued_count++;
                upload_ids.push_back(upload_id);
                spdlog::debug("[FtpUploadHandler] Queued video upload: {} -> {}", 
                             local_path, remote_path);
            }
        } else {
            spdlog::warn("[FtpUploadHandler] Video file not found: {}", local_path);
        }
    }
    
    // Trigger immediate upload (FtpManager will process the queue)
    if (queued_count > 0) {
        ftp_mgr.TriggerUpload();
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    if (queued_count > 0) {
        result.success = true;
        result.output = "Queued " + std::to_string(queued_count) + " file(s) for FTP upload";
        result.metadata["queued_files"] = std::to_string(queued_count);
        result.metadata["remote_path"] = remote_base;
        
        // Store upload IDs for tracking
        for (size_t i = 0; i < upload_ids.size(); ++i) {
            result.metadata["upload_id_" + std::to_string(i)] = upload_ids[i];
        }
    } else {
        result.success = false;
        result.error_message = "No files available for FTP upload";
    }
    
    return result;
}

bool FtpUploadHandler::IsAvailable() const {
    auto& ftp_mgr = storage::FtpManager::Instance();
    auto config = ftp_mgr.GetConfig();
    return config.enabled && !config.server.empty();
}

} // namespace events
} // namespace ipcam
