#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Config endpoint handler
// GET: Returns full configuration
// POST/PUT: Updates configuration
api::Response HandleConfig(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
