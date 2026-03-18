/**
 * @file ai.h
 * @brief AI Module Public Header
 * 
 * Main include for the AI analytics module.
 * Provides access to the AnalyticsEngine for video analytics.
 */

#pragma once

#include "analytics.h"

namespace ipcam {
namespace ai {

/**
 * @brief Get the AI runtime type
 * @return "analytics" if full AI enabled, "stub" otherwise
 */
const char* ai_runtime();

/**
 * @brief Initialize the AI module
 * @return true if initialization succeeds
 */
bool ai_init();

/**
 * @brief Start AI analytics processing
 * @return true if started successfully
 */
bool ai_start();

/**
 * @brief Stop AI analytics processing
 */
void ai_stop();

/**
 * @brief Shutdown the AI module
 */
void ai_shutdown();

} // namespace ai
} // namespace ipcam
