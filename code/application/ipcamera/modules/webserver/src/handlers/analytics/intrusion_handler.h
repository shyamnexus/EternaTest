/**
 * @file intrusion_handler.h
 * @brief Zone intrusion detection API handlers
 */
#pragma once

#include "analytics_common.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// GET/PUT /api/v1/analytics/intrusion - Zone intrusion config
api::Response HandleZoneIntrusion(const api::RequestContext& ctx);

// GET/POST/PUT/DELETE /api/v1/analytics/intrusion/zones - Intrusion zones
api::Response HandleIntrusionZones(const api::RequestContext& ctx);

// GET/PUT /api/v1/analytics/intrusion/schedule - Schedule
api::Response HandleIntrusionSchedule(const api::RequestContext& ctx);

// GET/PUT /api/v1/analytics/intrusion/actions - Event actions
api::Response HandleIntrusionActions(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
