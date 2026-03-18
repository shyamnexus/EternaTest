#pragma once

#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Main IR control endpoint - GET/PUT full configuration
api::Response HandleIR(const api::RequestContext& ctx);

// IR LED control endpoints
api::Response HandleIRLed(const api::RequestContext& ctx);
api::Response HandleIRLedBrightness(const api::RequestContext& ctx);

// IR Cut filter control endpoints
api::Response HandleIRCut(const api::RequestContext& ctx);

// Day/Night mode control endpoints
api::Response HandleDayNight(const api::RequestContext& ctx);
api::Response HandleDayNightMode(const api::RequestContext& ctx);

// IR control status endpoint
api::Response HandleIRStatus(const api::RequestContext& ctx);

// IR control actions (switch to day/night)
api::Response HandleIRAction(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
