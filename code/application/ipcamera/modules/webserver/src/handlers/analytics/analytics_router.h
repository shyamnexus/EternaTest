/**
 * @file analytics_router.h
 * @brief Main analytics API router - registers all analytics sub-module routes
 * 
 * This router provides the central /api/v1/analytics endpoint and aggregates
 * all sub-module handlers (motion, tampering, smart, etc.)
 * 
 * Architecture:
 * - Each feature has its own handler file (motion_handler.h, tampering_handler.h, etc.)
 * - Common functionality (schedule, actions, zones) is in analytics_common.h
 * - This router handles top-level analytics endpoints and capabilities
 * 
 * Endpoint Structure:
 * GET  /api/v1/analytics              - Full analytics config overview
 * GET  /api/v1/analytics/status       - Running status and stats
 * GET  /api/v1/analytics/capabilities - Supported features
 * 
 * Sub-modules are registered via their individual handlers:
 * - /api/v1/analytics/motion/...        - Motion detection
 * - /api/v1/analytics/tampering/...     - Video tampering
 * - /api/v1/analytics/notifications/... - Notification servers
 * - (Future: smart, face, lpr, line_cross, intrusion, etc.)
 */

#pragma once

#include "../../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// ============================================================================
// Top-Level Analytics API
// ============================================================================

/**
 * @brief GET /api/v1/analytics - Full config overview
 *        PUT /api/v1/analytics - Update multiple features at once
 */
api::Response HandleAnalytics(const api::RequestContext& ctx);

/**
 * @brief GET /api/v1/analytics/status - Running status and aggregate stats
 *        POST /api/v1/analytics/status - Control (start/stop all)
 */
api::Response HandleAnalyticsStatus(const api::RequestContext& ctx);

/**
 * @brief GET /api/v1/analytics/capabilities - Supported features and limits
 */
api::Response HandleAnalyticsCapabilities(const api::RequestContext& ctx);

// ============================================================================
// Legacy Compatibility
// ============================================================================

/**
 * @brief Legacy toggles endpoint for backward compatibility
 */
api::Response HandleAnalyticsToggles(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
