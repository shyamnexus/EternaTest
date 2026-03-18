/**
 * @file notification_handler.h
 * @brief Notification servers API handlers (MQTT, Email, FTP)
 * 
 * These notification servers are shared across all analytics features.
 * Each feature can independently enable/disable notifications via its actions config.
 * 
 * Endpoints:
 * - GET/PUT /api/v1/analytics/notifications         - All notification servers
 * - GET/PUT /api/v1/analytics/notifications/mqtt    - MQTT broker config
 * - GET/PUT /api/v1/analytics/notifications/email   - Email/SMTP config
 * - GET/PUT /api/v1/analytics/notifications/ftp     - FTP/SFTP server config
 * - POST    /api/v1/analytics/notifications/test    - Test notification delivery
 */

#pragma once

#include "../../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {
namespace analytics {

/**
 * @brief GET/PUT /api/v1/analytics/notifications - All notification servers overview
 */
api::Response HandleNotificationServers(const api::RequestContext& ctx);

/**
 * @brief GET/PUT /api/v1/analytics/notifications/mqtt - MQTT broker config
 */
api::Response HandleMqttConfig(const api::RequestContext& ctx);

/**
 * @brief GET/PUT /api/v1/analytics/notifications/email - Email/SMTP config
 */
api::Response HandleEmailConfig(const api::RequestContext& ctx);

/**
 * @brief GET/PUT /api/v1/analytics/notifications/ftp - FTP server config
 */
api::Response HandleFtpConfig(const api::RequestContext& ctx);

/**
 * @brief POST /api/v1/analytics/notifications/test - Test notification delivery
 */
api::Response HandleNotificationTest(const api::RequestContext& ctx);

} // namespace analytics
} // namespace handlers
} // namespace webserver
} // namespace ipcam
