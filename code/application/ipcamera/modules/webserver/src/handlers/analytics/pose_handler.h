/**
 * @file pose_handler.h
 * @brief Pose estimation API handlers
 */
#pragma once

#include "analytics_common.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

// GET/PUT /api/v1/analytics/pose - Pose estimation config
api::Response HandlePoseEstimation(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
