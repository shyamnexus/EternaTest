/**
 * @file linecross_handler.h
 * @brief Line crossing detection API handlers
 */
#pragma once

#include "analytics_common.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// GET/PUT /api/v1/analytics/linecross - Line crossing config
api::Response HandleLineCrossing(const api::RequestContext& ctx);

// GET/POST/PUT/DELETE /api/v1/analytics/linecross/lines - Lines config
api::Response HandleLineCrossLines(const api::RequestContext& ctx);

// GET/POST /api/v1/analytics/linecross/counts - Counting stats
api::Response HandleLineCrossCounts(const api::RequestContext& ctx);

// GET/PUT /api/v1/analytics/linecross/schedule - Schedule
api::Response HandleLineCrossSchedule(const api::RequestContext& ctx);

// GET/PUT /api/v1/analytics/linecross/actions - Event actions
api::Response HandleLineCrossActions(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
