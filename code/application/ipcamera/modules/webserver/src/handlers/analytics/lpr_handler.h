/**
 * @file lpr_handler.h
 * @brief License plate recognition API handlers
 */
#pragma once

#include "analytics_common.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// GET/PUT /api/v1/analytics/lpr - LPR config
api::Response HandleLpr(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
