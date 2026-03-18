#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Stream endpoint handler
// Returns available video streams
api::Response HandleStream(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
