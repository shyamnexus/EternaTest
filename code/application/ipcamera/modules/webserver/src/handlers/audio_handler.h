#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Main audio endpoint - GET/PUT configuration
api::Response HandleAudio(const api::RequestContext& ctx);

// Audio capabilities
api::Response HandleAudioCapabilities(const api::RequestContext& ctx);

// Audio test endpoint
api::Response HandleAudioTest(const api::RequestContext& ctx);

// Audio reset
api::Response HandleAudioReset(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
