#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Info endpoint handler
// Returns device information and capabilities
api::Response HandleInfo(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
