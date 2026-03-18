/**
 * @file smart_handler.h
 * @brief Smart detection (pedestrian/vehicle/animal) API handlers
 */
#pragma once

#include "analytics_common.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// GET/PUT /api/v1/analytics/smart - Smart detection config
api::Response HandleSmartDetection(const api::RequestContext& ctx);

// GET/POST/PUT/DELETE /api/v1/analytics/smart/zones - Smart detection zones
api::Response HandleSmartZones(const api::RequestContext& ctx);

// GET/PUT /api/v1/analytics/smart/schedule - Smart schedule
api::Response HandleSmartSchedule(const api::RequestContext& ctx);

// GET/PUT /api/v1/analytics/smart/actions - Smart event actions
api::Response HandleSmartActions(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
