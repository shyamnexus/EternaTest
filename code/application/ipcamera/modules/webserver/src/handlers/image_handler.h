#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Main image endpoint - GET all settings, PUT update all
api::Response HandleImage(const api::RequestContext& ctx);

// Image adjustment endpoints
api::Response HandleImageAdjustment(const api::RequestContext& ctx);

// White balance endpoints
api::Response HandleImageWhiteBalance(const api::RequestContext& ctx);

// Orientation (mirror/flip) endpoints
api::Response HandleImageOrientation(const api::RequestContext& ctx);

// Anti-flicker endpoints
api::Response HandleImageAntiFlicker(const api::RequestContext& ctx);

// Exposure endpoints
api::Response HandleImageExposure(const api::RequestContext& ctx);

// Day/Night mode endpoints
api::Response HandleImageDayNight(const api::RequestContext& ctx);

// BLC/WDR/HDR endpoints
api::Response HandleImageBLC(const api::RequestContext& ctx);

// Image enhancement (NR/Defog) endpoints
api::Response HandleImageEnhancement(const api::RequestContext& ctx);

// Reset to defaults
api::Response HandleImageReset(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
