#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Status endpoint handler
// Returns system status, version, and timestamp
api::Response HandleStatus(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
