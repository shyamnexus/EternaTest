/**
 * @file storage_handler.h
 * @brief NAS and FTP Storage API handlers
 */

#pragma once

#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// NAS Handlers
// ============================================================================

/// GET/PUT /api/v1/storage/nas - NAS configuration
api::Response HandleNasConfig(const api::RequestContext& ctx);

/// POST /api/v1/storage/nas/test - Test NAS connection
api::Response HandleNasTest(const api::RequestContext& ctx);

/// POST /api/v1/storage/nas/mount - Mount NAS share
api::Response HandleNasMount(const api::RequestContext& ctx);

/// POST /api/v1/storage/nas/unmount - Unmount NAS share
api::Response HandleNasUnmount(const api::RequestContext& ctx);

// ============================================================================
// FTP Handlers
// ============================================================================

/// GET/PUT /api/v1/storage/ftp - FTP configuration
api::Response HandleFtpConfig(const api::RequestContext& ctx);

/// POST /api/v1/storage/ftp/test - Test FTP connection
api::Response HandleFtpTest(const api::RequestContext& ctx);

/// POST /api/v1/storage/ftp/upload - Trigger manual upload
api::Response HandleFtpUpload(const api::RequestContext& ctx);

/// GET /api/v1/storage/ftp/queue - Get upload queue status
api::Response HandleFtpQueue(const api::RequestContext& ctx);

/// GET /api/v1/storage/ftp/history - Get upload history
api::Response HandleFtpHistory(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
