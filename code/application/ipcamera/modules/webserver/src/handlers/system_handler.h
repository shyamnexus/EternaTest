/**
 * @file system_handler.h
 * @brief System API handlers for webserver
 *
 * Provides REST endpoints for all system-level operations:
 *   /api/v1/system/device         - Device information
 *   /api/v1/system/time           - Date/Time & NTP
 *   /api/v1/system/time/ntp       - NTP configuration
 *   /api/v1/system/time/sync      - Manual NTP sync
 *   /api/v1/system/reboot         - Reboot
 *   /api/v1/system/shutdown       - Shutdown
 *   /api/v1/system/factory-reset  - Factory reset
 *   /api/v1/system/diagnostics    - Health & diagnostics
 *   /api/v1/system/logs           - Log retrieval & clear (multi-source)
 *   /api/v1/system/logs/download  - Log file download
 *   /api/v1/system/logs/config    - Log configuration
 *   /api/v1/system/logs/sources   - List available log sources
 *   /api/v1/system/logs/security  - Security event log
 *   /api/v1/system/logs/audit     - Configuration change audit trail
 *   /api/v1/system/logs/access    - HTTP API access log
 *   /api/v1/system/logs/syslog    - Remote syslog configuration
 *   /api/v1/system/diagnostics/report - Diagnostic bundle download
 *   /api/v1/system/logger          - System logger status & config
 *   /api/v1/system/logger/collect  - Force immediate collection
 *   /api/v1/system/maintenance    - Maintenance schedule
 *   /api/v1/system/watchdog       - Watchdog control
 *   /api/v1/system/backup         - Config backup (download)
 *   /api/v1/system/restore        - Config restore (upload)
 *   /api/v1/system/alarms         - Alarm thresholds
 */

#pragma once

#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// 1. Device Information
// ============================================================================

/// GET:  Return device info JSON
/// PUT:  Update device_name and/or telecontrol_id
api::Response HandleSystemDevice(const api::RequestContext& ctx);

// ============================================================================
// 2. Date / Time / NTP
// ============================================================================

/// GET:  Current time, timezone, UTC, unix timestamp
/// PUT:  Set timezone and/or manual date-time
api::Response HandleSystemTime(const api::RequestContext& ctx);

/// GET:  NTP configuration (includes port)
/// PUT:  Update NTP settings (enable, servers, port, interval)
api::Response HandleSystemTimeNtp(const api::RequestContext& ctx);

/// POST: Trigger an immediate NTP sync
api::Response HandleSystemTimeSync(const api::RequestContext& ctx);

/// GET:  DST configuration (mode, custom rules, offset)
/// PUT:  Update DST settings
api::Response HandleSystemTimeDst(const api::RequestContext& ctx);

// ============================================================================
// 3. Reboot / Shutdown
// ============================================================================

/// POST: Trigger reboot (body: type, delay_seconds, scheduled_time)
api::Response HandleSystemReboot(const api::RequestContext& ctx);

/// POST: Trigger shutdown (body: delay_seconds)
api::Response HandleSystemShutdown(const api::RequestContext& ctx);

// ============================================================================
// 4. Factory Reset
// ============================================================================

/// POST: Perform factory reset (body: mode = "hard"|"soft", delay_seconds)
api::Response HandleSystemFactoryReset(const api::RequestContext& ctx);

// ============================================================================
// 5. Diagnostics / Health
// ============================================================================

/// GET: Full diagnostics report (CPU, memory, disks, processes, temp, uptime)
api::Response HandleSystemDiagnostics(const api::RequestContext& ctx);

// ============================================================================
// 6. Logs
// ============================================================================

/// GET:    Retrieve logs with optional query filters (?source=, &level=, &start=, &end=, &search=, &limit=, &offset=)
/// DELETE: Clear logs (optional ?source= to clear specific source)
api::Response HandleSystemLogs(const api::RequestContext& ctx);

/// GET: Download the raw log file (?source= optional, defaults to application)
api::Response HandleSystemLogsDownload(const api::RequestContext& ctx);

/// GET:  Log configuration (levels, rotation, etc.)
/// PUT:  Update log configuration
api::Response HandleSystemLogsConfig(const api::RequestContext& ctx);

/// GET: List all log sources with availability and sizes
api::Response HandleSystemLogsSources(const api::RequestContext& ctx);

/// GET: Security event log (login/logout/failures/lockouts)
api::Response HandleSystemLogsSecurity(const api::RequestContext& ctx);

/// GET: Audit trail (config changes, admin actions)
api::Response HandleSystemLogsAudit(const api::RequestContext& ctx);

/// GET: HTTP API access log
api::Response HandleSystemLogsAccess(const api::RequestContext& ctx);

/// GET:  Remote syslog configuration
/// PUT:  Update remote syslog settings
api::Response HandleSystemLogsSyslog(const api::RequestContext& ctx);

/// GET: Generate and download diagnostic bundle (tar.gz with all logs, config, hw info)
api::Response HandleSystemDiagReport(const api::RequestContext& ctx);

// ============================================================================
// 6b. System Logger (replaces shell-script-based system_logger.sh)
// ============================================================================

/// GET:  System logger status (running, sources, destinations, stats)
/// PUT:  Update system logger config (enable/disable sources, destinations, intervals)
api::Response HandleSystemLogger(const api::RequestContext& ctx);

/// POST: Force immediate collection (?source= optional, collects all if omitted)
api::Response HandleSystemLoggerCollect(const api::RequestContext& ctx);

// ============================================================================
// 7. Maintenance
// ============================================================================

/// GET:  Maintenance schedule configuration
/// PUT:  Update maintenance schedule
api::Response HandleSystemMaintenance(const api::RequestContext& ctx);

// ============================================================================
// 8. Watchdog
// ============================================================================

/// GET:  Watchdog configuration
/// PUT:  Update watchdog settings
api::Response HandleSystemWatchdog(const api::RequestContext& ctx);

// ============================================================================
// 10. Config Backup / Restore
// ============================================================================

/// GET: Download config backup archive (tar.gz)
api::Response HandleSystemBackup(const api::RequestContext& ctx);

/// POST: Upload and apply config backup archive
api::Response HandleSystemRestore(const api::RequestContext& ctx);

// ============================================================================
// 11. Alarm Thresholds
// ============================================================================

/// GET:  Current alarm thresholds
/// PUT:  Update alarm thresholds
api::Response HandleSystemAlarms(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
