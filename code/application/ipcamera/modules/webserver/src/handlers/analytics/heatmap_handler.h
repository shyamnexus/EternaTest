/**
 * @file heatmap_handler.h
 * @brief Heat map analytics API handlers
 */
#pragma once

#include "analytics_common.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// GET/PUT /api/v1/analytics/heatmap - Heat map config
api::Response HandleHeatMap(const api::RequestContext& ctx);

// GET /api/v1/analytics/heatmap/grid - Get current heat map grid data
api::Response HandleHeatMapGrid(const api::RequestContext& ctx);

// POST /api/v1/analytics/heatmap/reset - Reset heat map
api::Response HandleHeatMapReset(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
