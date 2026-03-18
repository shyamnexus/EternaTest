/**
 * @file io_action.cpp
 * @brief GPIO/Alarm output, Light, and Siren action handlers using IRControl
 */

#include "ipcam/action_handler.h"
#include "ipcam/ir_control.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

namespace ipcam {
namespace events {

// ============================================================================
// IoOutputHandler - Alarm/GPIO Output
// ============================================================================

ActionResult IoOutputHandler::Execute(const Event& event, const Action& action) {
    ActionResult result;
    result.execution_time_ms = 0;
    
    const auto* cfg = action.GetConfig<IoActionConfig>();
    if (!cfg) {
        result.success = false;
        result.error_message = "Invalid configuration for IO output action";
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    spdlog::info("[IoOutputHandler] Triggering alarm output {} (state: {}, duration: {}ms)", 
                 cfg->output_id, cfg->state, cfg->duration_ms);
    
    auto& ir_ctrl = platform::IRControl::Instance();
    
    // Map output_id to GPIO pin (device-specific mapping)
    int gpio_pin = cfg->output_id;
    
    bool success = false;
    
    if (cfg->output_id == 0) {
        // Use IR LED as alarm output (common use case)
        if (cfg->state) {
            success = ir_ctrl.SetIRLedBrightness(100);  // Full brightness
            success = success && ir_ctrl.SetIRLedEnabled(true);
            
            if (success && cfg->duration_ms > 0) {
                // Schedule auto-off in background thread
                std::thread([duration = cfg->duration_ms]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
                    auto& ctrl = platform::IRControl::Instance();
                    ctrl.SetIRLedEnabled(false);
                }).detach();
            }
        } else {
            success = ir_ctrl.SetIRLedEnabled(false);
        }
    } else {
        // For other outputs, log the action
        // TODO: Add direct GPIO control to platform module for additional outputs
        spdlog::debug("[IoOutputHandler] GPIO {} set to {} (additional outputs not yet implemented)", 
                     gpio_pin, cfg->state);
        success = true;
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    if (success) {
        result.success = true;
        result.output = "Alarm output " + std::to_string(cfg->output_id) + " triggered";
    } else {
        result.success = false;
        result.error_message = "Failed to trigger alarm output " + std::to_string(cfg->output_id);
    }
    
    return result;
}

bool IoOutputHandler::IsAvailable() const {
    auto& ir_ctrl = platform::IRControl::Instance();
    return ir_ctrl.IsInitialized();
}

// ============================================================================
// LightActionHandler - White Light / IR LED Control
// ============================================================================

ActionResult LightActionHandler::Execute(const Event& event, const Action& action) {
    ActionResult result;
    result.execution_time_ms = 0;
    
    const auto* cfg = action.GetConfig<LightSirenActionConfig>();
    if (!cfg) {
        result.success = false;
        result.error_message = "Invalid configuration for light action";
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    spdlog::info("[LightActionHandler] Activating light at {}% for {}ms (pattern: {})", 
                 cfg->intensity, cfg->duration_ms, cfg->pattern);
    
    auto& ir_ctrl = platform::IRControl::Instance();
    
    bool success = false;
    
    if (cfg->activate) {
        // Set brightness level
        int brightness = cfg->intensity;
        success = ir_ctrl.SetIRLedBrightness(brightness);
        success = success && ir_ctrl.SetIRLedEnabled(true);
        
        if (success && cfg->duration_ms > 0) {
            // Handle strobe pattern
            if (cfg->pattern == "strobe" || cfg->pattern == "flash") {
                // Strobe effect using a background thread
                int duration_ms = cfg->duration_ms;
                int brightness_val = brightness;
                
                std::thread([duration_ms, brightness_val]() {
                    auto& ctrl = platform::IRControl::Instance();
                    auto start = std::chrono::steady_clock::now();
                    bool on = true;
                    
                    while (std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - start).count() < duration_ms) {
                        ctrl.SetIRLedBrightness(on ? brightness_val : 0);
                        on = !on;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 5Hz strobe
                    }
                    
                    ctrl.SetIRLedEnabled(false);
                }).detach();
            } else {
                // Solid pattern - schedule auto-off
                std::thread([duration_ms = cfg->duration_ms]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
                    auto& ctrl = platform::IRControl::Instance();
                    ctrl.SetIRLedEnabled(false);
                }).detach();
            }
        }
    } else {
        // Turn off light
        success = ir_ctrl.SetIRLedEnabled(false);
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    if (success) {
        result.success = true;
        result.output = cfg->activate ? "Light activated" : "Light deactivated";
    } else {
        result.success = false;
        result.error_message = "Failed to control light";
    }
    
    return result;
}

bool LightActionHandler::IsAvailable() const {
    auto& ir_ctrl = platform::IRControl::Instance();
    return ir_ctrl.IsInitialized();
}

// ============================================================================
// SirenActionHandler - Audio Alarm
// ============================================================================

ActionResult SirenActionHandler::Execute(const Event& event, const Action& action) {
    ActionResult result;
    result.execution_time_ms = 0;
    
    const auto* cfg = action.GetConfig<LightSirenActionConfig>();
    if (!cfg) {
        result.success = false;
        result.error_message = "Invalid configuration for siren action";
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    spdlog::info("[SirenActionHandler] {} siren at {}% volume for {}ms", 
                 cfg->activate ? "Activating" : "Deactivating",
                 cfg->intensity, cfg->duration_ms);
    
    // Note: Siren/audio control requires audio module integration
    // TODO: Integrate with audio module for actual siren playback
    
    if (cfg->activate) {
        spdlog::debug("[SirenActionHandler] Would play siren audio (not yet implemented)");
    } else {
        spdlog::debug("[SirenActionHandler] Would stop siren audio (not yet implemented)");
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    result.success = true;
    result.output = cfg->activate ? "Siren activated" : "Siren deactivated";
    
    return result;
}

bool SirenActionHandler::IsAvailable() const {
    // Siren requires audio hardware support
    // Return false until audio module integration is complete
    return false;
}

} // namespace events
} // namespace ipcam
