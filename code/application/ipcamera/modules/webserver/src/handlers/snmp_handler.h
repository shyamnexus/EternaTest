#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// SNMP endpoint handler
// GET: Returns SNMP configuration and status
// POST/PUT: Updates SNMP configuration
api::Response HandleSnmp(const api::RequestContext& ctx);

// SNMP control endpoint handler
// POST: Start/stop/restart SNMP daemon
api::Response HandleSnmpControl(const api::RequestContext& ctx);

// SNMP test endpoint handler
// POST: Tests SNMP configuration
api::Response HandleSnmpTest(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
