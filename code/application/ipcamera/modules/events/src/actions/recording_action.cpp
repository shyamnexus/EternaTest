/**
 * @file recording_action.cpp
 * @brief Recording action handler implementation using RecordingService
 */

#include "ipcam/action_handler.h"
#include "ipcam/recording.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

namespace ipcam {
namespace events {

ActionResult RecordingActionHandler::Execute(const Event& event, const Action& action) {
    ActionResult result;
    result.execution_time_ms = 0;
    
    const auto* cfg = action.GetConfig<RecordingActionConfig>();
    if (!cfg) {
        result.success = false;
        result.error_message = "Invalid configuration for recording action";
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Get recording service instance
    auto& recording_svc = recording::RecordingService::Instance();
    
    // Check if storage is available
    if (!recording_svc.IsStorageAvailable()) {
        result.success = false;
        result.error_message = "SD card storage not available for recording";
        return result;
    }
    
    spdlog::info("[RecordingActionHandler] Starting event recording for event: {}", event.id);
    
    // Get channels to record (use event channel if no specific channels configured)
    std::vector<int> channels_to_record = cfg->channels;
    if (channels_to_record.empty()) {
        channels_to_record.push_back(event.channel);
    }
    
    bool all_success = true;
    std::vector<std::string> recording_paths;
    
    // Configure pre/post recording if specified
    auto current_config = recording_svc.GetConfig();
    if (cfg->pre_record_seconds > 0) {
        current_config.pre_record_enabled = true;
        current_config.pre_record_seconds = cfg->pre_record_seconds;
    }
    if (cfg->post_record_seconds > 0) {
        current_config.post_record_enabled = true;
        current_config.post_record_seconds = cfg->post_record_seconds;
    }
    
    // Start recording on each channel
    for (int channel : channels_to_record) {
        bool started = recording_svc.StartRecording(channel, recording::RecordingType::Event);
        
        if (started) {
            auto session = recording_svc.GetSession(channel);
            if (!session.current_file.empty()) {
                recording_paths.push_back(session.current_file);
            }
            spdlog::info("[RecordingActionHandler] Started recording on channel {} for event {}",
                        channel, event.id);
        } else {
            spdlog::warn("[RecordingActionHandler] Failed to start recording on channel {}", channel);
            all_success = false;
        }
    }
    
    // Schedule recording stop if duration is specified
    if (cfg->duration_seconds > 0) {
        int duration_sec = cfg->duration_seconds;
        std::vector<int> channels_copy = channels_to_record;
        
        // Note: In a real implementation, we'd use a proper timer/scheduler
        // For now, we spawn a detached thread to stop recording after duration
        std::thread([duration_sec, channels_copy]() {
            std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
            
            auto& svc = recording::RecordingService::Instance();
            for (int ch : channels_copy) {
                svc.StopRecording(ch);
                spdlog::debug("[RecordingActionHandler] Stopped timed recording on channel {}", ch);
            }
        }).detach();
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    if (all_success || !recording_paths.empty()) {
        result.success = true;
        if (!recording_paths.empty()) {
            result.output = recording_paths[0];  // Primary recording path
        }
        result.metadata["channels_started"] = std::to_string(recording_paths.size());
        result.metadata["duration_seconds"] = std::to_string(cfg->duration_seconds);
    } else {
        result.success = false;
        result.error_message = "Failed to start recording on any channel";
    }
    
    return result;
}

bool RecordingActionHandler::IsAvailable() const {
    auto& recording_svc = recording::RecordingService::Instance();
    return recording_svc.IsInitialized() && recording_svc.IsStorageAvailable();
}

} // namespace events
} // namespace ipcam
