/**
 * @file events_handler.h
 * @brief Web API handlers for Events management (function-based)
 */

#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Events endpoints
api::Response HandleEvents(const api::RequestContext& ctx);
api::Response HandleEventsRules(const api::RequestContext& ctx);
api::Response HandleEventsStats(const api::RequestContext& ctx);
api::Response HandleEventsTest(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
