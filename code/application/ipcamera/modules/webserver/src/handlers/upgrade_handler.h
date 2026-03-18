/**
 * @file upgrade_handler.h
 * @brief Firmware upgrade API handlers for webserver
 */

#pragma once

#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// Firmware Upgrade Handlers
// ============================================================================

/// GET /api/v1/firmware/version - Get current firmware version info
api::Response HandleFirmwareVersion(const api::RequestContext& ctx);

/// GET /api/v1/firmware/status - Get upgrade status/progress
api::Response HandleFirmwareStatus(const api::RequestContext& ctx);

/// POST /api/v1/firmware/upload - Upload firmware file (multipart/form-data)
/// Also supports: POST with raw binary body + Content-Length header
api::Response HandleFirmwareUpload(const api::RequestContext& ctx);

/// POST /api/v1/firmware/upgrade - Start upgrade from uploaded/specified file
/// Body: {"file": "/tmp/firmware.bin"} or empty to use last uploaded
api::Response HandleFirmwareUpgrade(const api::RequestContext& ctx);

/// POST /api/v1/firmware/validate - Validate firmware without installing
api::Response HandleFirmwareValidate(const api::RequestContext& ctx);

/// POST /api/v1/firmware/cancel - Cancel in-progress upgrade
api::Response HandleFirmwareCancel(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
