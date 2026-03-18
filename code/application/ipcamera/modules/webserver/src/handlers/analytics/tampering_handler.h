/**
 * @file tampering_handler.h
 * @brief Video tampering detection API handlers
 * 
 * Endpoints:
 * - GET/PUT /api/v1/analytics/tampering         - Tampering config (enabled, sensitivity, detectors)
 * - GET/PUT /api/v1/analytics/tampering/schedule - Tampering schedule rules
 * - GET/PUT /api/v1/analytics/tampering/actions  - Event actions (record, snapshot, notify)
 * - GET     /api/v1/analytics/tampering/status   - Live status and metrics
 * 
 * Detectors:
 * - defocus_detection  - Camera out of focus / blurry
 * - masking_detection  - Camera lens covered / blocked
 * - scene_change       - Major scene change (camera moved)
 * - too_dark           - Illumination too low
 * - too_bright         - Illumination too high / IR reflection
 */

#pragma once

#include "../../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

/**
 * @brief GET/PUT /api/v1/analytics/tampering - Video tampering config
 */
api::Response HandleVideoTampering(const api::RequestContext& ctx);

/**
 * @brief GET/PUT /api/v1/analytics/tampering/schedule - Tampering schedule
 */
api::Response HandleTamperingSchedule(const api::RequestContext& ctx);

/**
 * @brief GET/PUT /api/v1/analytics/tampering/actions - Tampering event actions
 */
api::Response HandleTamperingActions(const api::RequestContext& ctx);

/**
 * @brief GET /api/v1/analytics/tampering/status - Live tampering status
 */
api::Response HandleTamperingStatus(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
