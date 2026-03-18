/**
 * @file ai.cpp
 * @brief AI Module Entry Points
 * 
 * Provides the main entry points for initializing and managing
 * the AI analytics engine.
 */

#include "ipcam/ai.h"
#include "ipcam/analytics.h"
#include <spdlog/spdlog.h>

namespace ipcam {
namespace ai {

const char* ai_runtime() {
#if HDAL_PIPELINE_ENABLED
    return "analytics";
#else
    return "stub";
#endif
}

bool ai_init() {
    spdlog::info("AI module init");
    
    auto& engine = AnalyticsEngine::Instance();
    
    // Load configuration
    if (!engine.LoadConfig()) {
        spdlog::error("Failed to load AI analytics config");
        return false;
    }
    
    // Initialize engine
    if (!engine.Init()) {
        spdlog::error("Failed to initialize AI analytics engine");
        return false;
    }
    
    spdlog::info("AI module initialized (runtime={})", ai_runtime());
    return true;
}

bool ai_start() {
    spdlog::info("Starting AI module");
    return AnalyticsEngine::Instance().Start();
}

void ai_stop() {
    spdlog::info("Stopping AI module");
    AnalyticsEngine::Instance().Stop();
}

void ai_shutdown() {
    spdlog::info("Shutting down AI module");
    AnalyticsEngine::Instance().Shutdown();
}

} // namespace ai
} // namespace ipcam
