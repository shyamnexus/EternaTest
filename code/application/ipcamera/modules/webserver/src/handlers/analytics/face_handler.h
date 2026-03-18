/**
 * @file face_handler.h
 * @brief Face detection and recognition API handlers
 */
#pragma once

#include "analytics_common.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// GET/PUT /api/v1/analytics/face - Face detection & recognition config
api::Response HandleFaceDetection(const api::RequestContext& ctx);

// GET /api/v1/analytics/face/gallery - Face gallery list
api::Response HandleFaceGallery(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
