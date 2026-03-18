/**
 * @file rtsp_handler.h
 * @brief RTSP Server Configuration API Handlers
 * 
 * Provides REST API endpoints for managing RTSP server configuration
 * including authentication, ports, and runtime control.
 */

#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

/**
 * @brief Get/Set RTSP server configuration
 * 
 * GET /api/v1/rtsp
 *   Returns current RTSP server configuration
 *   Response:
 *   {
 *     "enabled": true,
 *     "port": 554,
 *     "http_tunnel_port": 8554,
 *     "max_connections": 10,
 *     "auth": {
 *       "mode": "digest",  // "none", "basic", "digest"
 *       "realm": "IP Camera",
 *       "username": "admin"  // password not returned
 *     },
 *     "status": {
 *       "running": true,
 *       "client_count": 2,
 *       "streams": [
 *         {"name": "stream0", "url": "rtsp://192.168.0.2/stream0"},
 *         {"name": "stream1", "url": "rtsp://192.168.0.2/stream1"}
 *       ]
 *     }
 *   }
 * 
 * PUT /api/v1/rtsp
 *   Update RTSP server configuration
 *   Request body can include any of the fields above
 *   Requires restart: port, http_tunnel_port changes
 */
api::Response HandleRtsp(const api::RequestContext& ctx);

/**
 * @brief Get/Set RTSP authentication settings
 * 
 * GET /api/v1/rtsp/auth
 *   Returns authentication settings (without password)
 * 
 * PUT /api/v1/rtsp/auth
 *   Update authentication settings
 *   Request:
 *   {
 *     "mode": "digest",
 *     "realm": "IP Camera",
 *     "username": "admin",
 *     "password": "newpassword"
 *   }
 */
api::Response HandleRtspAuth(const api::RequestContext& ctx);

/**
 * @brief RTSP server control (start/stop/restart)
 * 
 * POST /api/v1/rtsp/control
 *   Request:
 *   {
 *     "action": "restart"  // "start", "stop", "restart"
 *   }
 */
api::Response HandleRtspControl(const api::RequestContext& ctx);

/**
 * @brief Get RTSP server status
 * 
 * GET /api/v1/rtsp/status
 *   Returns runtime status
 *   Response:
 *   {
 *     "running": true,
 *     "uptime_seconds": 3600,
 *     "client_count": 2,
 *     "streams": [
 *       {
 *         "name": "stream0",
 *         "url": "rtsp://192.168.0.2/stream0",
 *         "codec": "h265",
 *         "active": true
 *       }
 *     ]
 *   }
 */
api::Response HandleRtspStatus(const api::RequestContext& ctx);

/**
 * @brief List connected RTSP clients
 * 
 * GET /api/v1/rtsp/clients
 *   Returns list of connected clients
 */
api::Response HandleRtspClients(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
