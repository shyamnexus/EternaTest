#ifndef IPCAM_WEBSERVER_HANDLERS_OSD_HANDLER_H
#define IPCAM_WEBSERVER_HANDLERS_OSD_HANDLER_H

#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Handle OSD configuration for a specific stream
// GET /api/osd/stream/{id}/config
// POST /api/osd/stream/{id}/config
ipcam::webserver::api::Response HandleOsdStreamConfig(const ipcam::webserver::api::RequestContext& ctx);

// Handle Global OSD config
// GET /api/osd/global/config
// POST /api/osd/global/config
ipcam::webserver::api::Response HandleOsdGlobalConfig(const ipcam::webserver::api::RequestContext& ctx);

// Handle Privacy Mask for a specific stream (4 regions per stream)
// GET /api/osd/stream/{id}/privacy_mask - Get privacy mask config
// POST /api/osd/stream/{id}/privacy_mask - Set privacy mask config
// POST /api/osd/stream/{id}/privacy_mask/apply - Apply privacy mask to encoder
ipcam::webserver::api::Response HandleOsdPrivacyMask(const ipcam::webserver::api::RequestContext& ctx);

// Handle Global Privacy Mask (VIDEOPROC level, applies to all streams)
// GET /api/v1/osd/global_privacy_mask - Get global privacy mask config
// POST /api/v1/osd/global_privacy_mask - Set global privacy mask config
// Parameters:
//   enabled: bool - Master enable/disable
//   regions: array of {region_id, enabled, x1, y1, x2, y2, color}
// Note: Coordinates are in main stream resolution, auto-scaled to all streams
ipcam::webserver::api::Response HandleOsdGlobalPrivacyMask(const ipcam::webserver::api::RequestContext& ctx);

// Handle OSD Auto-Scaling configuration
// GET /api/v1/osd/auto_scaling - Get auto-scaling config
// POST /api/v1/osd/auto_scaling - Set auto-scaling config
// Parameters:
//   enabled: bool - Enable/disable auto-scaling based on stream resolution
//   base_font_size: int - Base font size at reference resolution (default 48)
//   base_text_font_size: int - Base text font size at reference resolution (default 36)
//   min_font_size: int - Minimum scaled font size (default 12)
//   max_font_size: int - Maximum scaled font size (default 72)
//   reference_width: int - Reference resolution width (default 2560)
//   reference_height: int - Reference resolution height (default 1440)
// Note: When enabled, font sizes are automatically scaled based on stream diagonal ratio
ipcam::webserver::api::Response HandleOsdAutoScaling(const ipcam::webserver::api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam

#endif // IPCAM_WEBSERVER_HANDLERS_OSD_HANDLER_H
