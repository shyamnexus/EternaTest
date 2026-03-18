#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Network endpoint handler
// GET: Returns network configuration (IPv4, IPv6, DNS, ports, MAC)
// POST/PUT: Updates network configuration
api::Response HandleNetwork(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
