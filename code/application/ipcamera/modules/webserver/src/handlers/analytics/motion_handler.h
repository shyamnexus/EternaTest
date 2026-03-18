/**
 * @file motion_handler.h
 * @brief Motion detection API handlers
 * 
 * Endpoints:
 * - GET/PUT /api/v1/analytics/motion         - Motion config (enabled, sensitivity, etc.)
 * - GET/POST/PUT/DELETE /api/v1/analytics/motion/zones   - Motion detection zones
 * - GET/PUT /api/v1/analytics/motion/schedule - Motion schedule rules
 * - GET/PUT /api/v1/analytics/motion/actions  - Event actions (record, snapshot, notify)
 */

#pragma once

#include "../../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

/**
 * @brief GET/PUT /api/v1/analytics/motion - Motion detection config
 */
api::Response HandleMotionDetection(const api::RequestContext& ctx);

/**
 * @brief GET/POST/PUT/DELETE /api/v1/analytics/motion/zones - Motion zones
 */
api::Response HandleMotionZones(const api::RequestContext& ctx);

/**
 * @brief GET/PUT /api/v1/analytics/motion/schedule - Motion schedule
 */
api::Response HandleMotionSchedule(const api::RequestContext& ctx);

/**
 * @brief GET/PUT /api/v1/analytics/motion/actions - Motion event actions
 */
api::Response HandleMotionActions(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
