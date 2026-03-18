/**
 * @file schedule_handler.h
 * @brief Recording Schedule API handlers
 */

#pragma once

#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

/// GET/PUT /api/v1/recording/schedules - List/Create schedule profiles
api::Response HandleSchedules(const api::RequestContext& ctx);

/// GET/PUT/DELETE /api/v1/recording/schedules/{id} - Manage specific profile
api::Response HandleScheduleById(const api::RequestContext& ctx);

/// GET/PUT /api/v1/recording/schedule/config - Schedule configuration
api::Response HandleScheduleConfig(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
