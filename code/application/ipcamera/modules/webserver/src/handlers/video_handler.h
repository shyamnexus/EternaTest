#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Main video endpoint - GET all streams, PUT update all
api::Response HandleVideo(const api::RequestContext& ctx);

// Individual stream endpoint
api::Response HandleVideoStream(const api::RequestContext& ctx);

// Video capabilities
api::Response HandleVideoCapabilities(const api::RequestContext& ctx);

// Snapshot endpoints
api::Response HandleSnapshot(const api::RequestContext& ctx);
api::Response HandleSnapshotConfig(const api::RequestContext& ctx);

// Video reset
api::Response HandleVideoReset(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
