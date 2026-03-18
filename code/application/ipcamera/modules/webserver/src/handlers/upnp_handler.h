#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// UPnP endpoint handler
// GET: Returns UPnP device information and configuration
// POST/PUT: Updates UPnP configuration
api::Response HandleUpnp(const api::RequestContext& ctx);

// UPnP discovery endpoint handler
// POST: Discovers UPnP IGD devices
api::Response HandleUpnpDiscover(const api::RequestContext& ctx);

// UPnP port mappings endpoint handler
// GET: Lists all port mappings
// POST: Adds a new port mapping
// DELETE: Removes a port mapping
api::Response HandleUpnpPortMappings(const api::RequestContext& ctx);

// UPnP camera ports endpoint handler
// POST: Adds standard camera port mappings
// DELETE: Removes all camera port mappings
api::Response HandleUpnpCameraPorts(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
