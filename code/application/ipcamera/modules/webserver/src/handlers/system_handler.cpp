/**
 * @file system_handler.cpp
 * @brief System API handler implementations
 *
 * All handlers delegate to platform::system_utils for the actual
 * system-level operations and simply marshal JSON <-> structs.
 */

#include "system_handler.h"
#include <ipcam/system_utils.h>
#include <ipcam/ntp_manager.h>
#include <ipcam/audit_logger.h>
#include <ipcam/system_logger.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <sys/stat.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// Helpers
// ============================================================================

static api::Response MethodNotAllowed() {
    api::Response r;
    r.status_code = 405;
    r.body = R"({"error":"Method not allowed"})";
    r.headers["Content-Type"] = "application/json";
    return r;
}

static api::Response ErrorResponse(int code, const std::string& msg) {
    api::Response r;
    r.status_code = code;
    r.body = json{{"error", msg}}.dump();
    r.headers["Content-Type"] = "application/json";
    return r;
}

static api::Response JsonOk(const json& j) {
    api::Response r;
    r.status_code = 200;
    r.body = j.dump(2);
    r.headers["Content-Type"] = "application/json";
    return r;
}

/// Extract client IP from request headers (nginx sets X-Real-IP / X-Forwarded-For)
static std::string GetClientIP(const api::RequestContext& ctx) {
    auto hdr = [&](const std::string& key) -> std::string {
        auto it = ctx.headers.find(key);
        return (it != ctx.headers.end()) ? it->second : "";
    };
    std::string ip = hdr("X-Real-IP");
    if (!ip.empty()) return ip;
    std::string fwd = hdr("X-Forwarded-For");
    if (!fwd.empty()) {
        size_t comma = fwd.find(',');
        return (comma != std::string::npos) ? fwd.substr(0, comma) : fwd;
    }
    return "unknown";
}

// ============================================================================
// 1. Device Information  –  GET / PUT  /api/v1/system/device
// ============================================================================

api::Response HandleSystemDevice(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            auto d = platform::GetDeviceInfo();
            json j;
            j["device_name"]          = d.device_name;
            j["model"]                = d.model;
            j["serial_number"]        = d.serial_number;
            j["firmware_version"]     = d.firmware_version;
            j["hardware_id"]          = d.hardware_id;
            j["manufacturer"]         = d.manufacturer;
            j["telecontrol_id"]       = d.telecontrol_id;
            j["encoder_version"]      = d.encoder_version;
            j["web_version"]          = d.web_version;
            j["plugin_version"]       = d.plugin_version;
            j["channels_number"]      = d.channels_number;
            j["hard_disks_number"]    = d.hard_disks_number;
            j["alarm_inputs_number"]  = d.alarm_inputs_number;
            j["alarm_outputs_number"] = d.alarm_outputs_number;
            j["firmware_version_info"]= d.firmware_version_info;
            j["sensor_type"]          = d.sensor_type;
            return JsonOk(j);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            bool ok = true;
            std::string msg;

            if (body.contains("device_name")) {
                ok = platform::SetDeviceName(body["device_name"].get<std::string>());
                if (!ok) msg = "Invalid device name (1-64 chars)";
            }
            if (ok && body.contains("telecontrol_id")) {
                ok = platform::SetTelecontrolId(body["telecontrol_id"].get<int>());
                if (!ok) msg = "Invalid telecontrol ID (0-255)";
            }

            if (!ok) return ErrorResponse(400, msg);

            // Return updated info
            auto d = platform::GetDeviceInfo();
            json resp;
            resp["success"] = true;
            resp["device_name"]    = d.device_name;
            resp["telecontrol_id"] = d.telecontrol_id;

            ipcam::AuditLogger::Instance().LogConfigChange(
                "-", GetClientIP(ctx), "system.device",
                "", body.dump());

            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemDevice: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 2a. Date / Time  –  GET / PUT  /api/v1/system/time
// ============================================================================

api::Response HandleSystemTime(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            auto t = platform::GetTimeInfo();
            auto& ntp = networking::NtpManager::Instance();
            auto dst = ntp.GetDstConfig();
            json j;
            j["timezone"]       = t.timezone;
            j["local_time"]     = t.local_time;
            j["utc_time"]       = t.utc_time;
            j["unix_timestamp"] = t.unix_timestamp;
            j["dst_active"]     = t.dst_active;
            j["utc_offset_min"] = t.utc_offset_min;
            j["date_format"]    = t.date_format;
            j["dst"] = {
                {"enabled", dst.enabled},
                {"mode", dst.mode},
                {"offset_minutes", dst.offset_minutes}
            };
            return JsonOk(j);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            bool ok = true;
            std::string msg;

            if (body.contains("timezone")) {
                ok = platform::SetTimezone(body["timezone"].get<std::string>());
                if (!ok) msg = "Failed to set timezone";
            }
            if (ok && body.contains("date_time")) {
                ok = platform::SetDateTime(body["date_time"].get<std::string>());
                if (!ok) msg = "Failed to set date/time (expected ISO-8601)";
            }
            if (ok && body.contains("date_format")) {
                ok = platform::SetDateFormat(body["date_format"].get<std::string>());
                if (!ok) msg = "Invalid date_format (use YYYY-MM-DD, MM/DD/YYYY, or DD/MM/YYYY)";
            }

            if (!ok) return ErrorResponse(400, msg);

            auto t = platform::GetTimeInfo();
            json resp;
            resp["success"]    = true;
            resp["timezone"]   = t.timezone;
            resp["local_time"] = t.local_time;
            resp["utc_time"]   = t.utc_time;
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemTime: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 2b. NTP Config  –  GET / PUT  /api/v1/system/time/ntp
// ============================================================================

static std::string SourceToString(networking::NtpManager::Source s) {
    switch (s) {
        case networking::NtpManager::Source::Auto:   return "auto";
        case networking::NtpManager::Source::Dhcp:   return "dhcp";
        case networking::NtpManager::Source::Manual: return "manual";
    }
    return "auto";
}

static networking::NtpManager::Source StringToSource(const std::string& s) {
    if (s == "dhcp")   return networking::NtpManager::Source::Dhcp;
    if (s == "manual") return networking::NtpManager::Source::Manual;
    return networking::NtpManager::Source::Auto;
}

api::Response HandleSystemTimeNtp(const api::RequestContext& ctx) {
    try {
        auto& ntp = networking::NtpManager::Instance();

        if (ctx.method == "GET") {
            auto c = ntp.GetConfig();
            auto last = ntp.GetLastSyncResult();
            json j;
            j["enabled"]              = c.enabled;
            j["source"]               = SourceToString(c.source);
            j["primary_server"]       = c.primary_server;
            j["port"]                 = c.port;
            j["fallback_servers"]     = c.fallback_servers;
            j["fallback_ips"]         = c.fallback_ips;
            j["use_fallback_ips"]     = c.use_fallback_ips;
            j["sync_timeout_sec"]     = c.sync_timeout_sec;
            j["retry_count"]          = c.retry_count;
            j["refresh_interval_sec"] = c.refresh_interval_sec;
            j["timezone"]             = c.timezone;
            j["syncing"]              = ntp.IsSyncing();
            // DST summary
            auto dst = ntp.GetDstConfig();
            j["dst"] = {
                {"enabled", dst.enabled},
                {"mode", dst.mode},
                {"offset_minutes", dst.offset_minutes}
            };
            j["last_sync"] = {
                {"status",  static_cast<int>(last.status)},
                {"server",  last.server_used},
                {"message", last.message},
                {"offset_ms", last.offset_ms}
            };
            return JsonOk(j);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto c = ntp.GetConfig();

            if (body.contains("enabled"))              c.enabled = body["enabled"];
            if (body.contains("source"))               c.source = StringToSource(body["source"].get<std::string>());
            if (body.contains("primary_server"))       c.primary_server = body["primary_server"].get<std::string>();
            if (body.contains("port"))                  c.port = body["port"];
            if (body.contains("sync_timeout_sec"))     c.sync_timeout_sec = body["sync_timeout_sec"];
            if (body.contains("retry_count"))          c.retry_count = body["retry_count"];
            if (body.contains("refresh_interval_sec")) c.refresh_interval_sec = body["refresh_interval_sec"];
            if (body.contains("timezone"))             c.timezone = body["timezone"].get<std::string>();
            if (body.contains("use_fallback_ips"))     c.use_fallback_ips = body["use_fallback_ips"];
            if (body.contains("fallback_servers")) {
                c.fallback_servers.clear();
                for (auto& s : body["fallback_servers"]) {
                    c.fallback_servers.push_back(s.get<std::string>());
                }
            }
            if (body.contains("fallback_ips")) {
                c.fallback_ips.clear();
                for (auto& s : body["fallback_ips"]) {
                    c.fallback_ips.push_back(s.get<std::string>());
                }
            }

            auto result = ntp.SetConfig(c);
            if (!result) return ErrorResponse(400, "Failed to update NTP config: " + result.error);

            json resp;
            resp["success"] = true;
            resp["config"]  = {
                {"enabled", c.enabled},
                {"source", SourceToString(c.source)},
                {"primary_server", c.primary_server},
                {"refresh_interval_sec", c.refresh_interval_sec}
            };
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemTimeNtp: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 2c. Manual NTP Sync  –  POST  /api/v1/system/time/sync
// ============================================================================

api::Response HandleSystemTimeSync(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "POST") return MethodNotAllowed();

        auto& ntp = networking::NtpManager::Instance();
        auto sr = ntp.SyncNow();

        bool ok = (sr.status == networking::NtpManager::SyncStatus::Success);

        json resp;
        resp["success"]   = ok;
        resp["message"]   = sr.message;
        resp["server"]    = sr.server_used;
        resp["offset_ms"] = sr.offset_ms;

        auto t = platform::GetTimeInfo();
        resp["local_time"]     = t.local_time;
        resp["utc_time"]       = t.utc_time;
        resp["unix_timestamp"] = t.unix_timestamp;
        return JsonOk(resp);
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemTimeSync: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 2d. DST Config  –  GET / PUT  /api/v1/system/time/dst
// ============================================================================

api::Response HandleSystemTimeDst(const api::RequestContext& ctx) {
    try {
        auto& ntp = networking::NtpManager::Instance();

        if (ctx.method == "GET") {
            auto d = ntp.GetDstConfig();
            json j;
            j["enabled"]          = d.enabled;
            j["mode"]             = d.mode;
            j["offset_minutes"]   = d.offset_minutes;
            j["start"] = {
                {"month", d.start_month},
                {"week", d.start_week},
                {"day_of_week", d.start_day_of_week},
                {"hour", d.start_hour}
            };
            j["end"] = {
                {"month", d.end_month},
                {"week", d.end_week},
                {"day_of_week", d.end_day_of_week},
                {"hour", d.end_hour}
            };

            // Show whether DST is currently in effect
            auto t = platform::GetTimeInfo();
            j["currently_active"] = t.dst_active;
            j["utc_offset_min"]   = t.utc_offset_min;

            return JsonOk(j);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto d = ntp.GetDstConfig();

            if (body.contains("enabled"))        d.enabled = body["enabled"];
            if (body.contains("mode"))           d.mode = body["mode"].get<std::string>();
            if (body.contains("offset_minutes")) d.offset_minutes = body["offset_minutes"];

            // Start transition
            if (body.contains("start")) {
                auto& s = body["start"];
                if (s.contains("month"))        d.start_month = s["month"];
                if (s.contains("week"))         d.start_week = s["week"];
                if (s.contains("day_of_week"))  d.start_day_of_week = s["day_of_week"];
                if (s.contains("hour"))         d.start_hour = s["hour"];
            }

            // End transition
            if (body.contains("end")) {
                auto& e = body["end"];
                if (e.contains("month"))        d.end_month = e["month"];
                if (e.contains("week"))         d.end_week = e["week"];
                if (e.contains("day_of_week"))  d.end_day_of_week = e["day_of_week"];
                if (e.contains("hour"))         d.end_hour = e["hour"];
            }

            // Validate
            if (d.mode != "auto" && d.mode != "custom") {
                return ErrorResponse(400, "mode must be 'auto' or 'custom'");
            }
            if (d.offset_minutes < 0 || d.offset_minutes > 120) {
                return ErrorResponse(400, "offset_minutes must be 0-120");
            }

            auto result = ntp.SetDstConfig(d);
            if (!result) return ErrorResponse(400, "Failed to save DST config: " + result.error);

            json resp;
            resp["success"] = true;
            resp["dst"] = {
                {"enabled", d.enabled},
                {"mode", d.mode},
                {"offset_minutes", d.offset_minutes}
            };
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemTimeDst: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 3a. Reboot  –  POST  /api/v1/system/reboot
// ============================================================================

api::Response HandleSystemReboot(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            // Return scheduled-reboot status
            json j;
            j["reboot_scheduled"]    = platform::IsRebootScheduled();
            j["scheduled_reboot_time"] = platform::GetScheduledRebootTime();
            return JsonOk(j);
        }

        if (ctx.method == "POST") {
            auto body = ctx.body.empty() ? json::object() : json::parse(ctx.body);

            platform::RebootRequest req;
            std::string typeStr = body.value("type", "warm");
            if (typeStr == "cold")      req.type = platform::RebootType::Cold;
            else if (typeStr == "scheduled") req.type = platform::RebootType::Scheduled;
            else                        req.type = platform::RebootType::Warm;

            req.delay_seconds  = body.value("delay_seconds", 0);
            req.scheduled_time = body.value("scheduled_time", "");

            bool ok = platform::TriggerReboot(req);

            ipcam::AuditLogger::Instance().LogSystemAction(
                "-", GetClientIP(ctx), "reboot",
                "type=" + typeStr + " delay=" + std::to_string(req.delay_seconds));

            json resp;
            resp["success"] = ok;
            resp["message"] = ok ? "Reboot initiated" : "Failed to initiate reboot";
            resp["type"]    = typeStr;
            resp["delay_seconds"] = req.delay_seconds;
            return JsonOk(resp);
        }

        if (ctx.method == "DELETE") {
            // Cancel scheduled reboot
            bool ok = platform::CancelScheduledReboot();
            json resp;
            resp["success"] = ok;
            resp["message"] = ok ? "Scheduled reboot cancelled" : "No scheduled reboot to cancel";
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemReboot: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 3b. Shutdown  –  POST  /api/v1/system/shutdown
// ============================================================================

api::Response HandleSystemShutdown(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "POST") return MethodNotAllowed();

        auto body = ctx.body.empty() ? json::object() : json::parse(ctx.body);
        int delay = body.value("delay_seconds", 0);

        bool ok = platform::TriggerShutdown(delay);

        json resp;
        resp["success"] = ok;
        resp["message"] = ok ? "Shutdown initiated" : "Shutdown failed";
        resp["delay_seconds"] = delay;
        return JsonOk(resp);
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemShutdown: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 4. Factory Reset  –  POST  /api/v1/system/factory-reset
// ============================================================================

api::Response HandleSystemFactoryReset(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "POST") return MethodNotAllowed();

        auto body = ctx.body.empty() ? json::object() : json::parse(ctx.body);
        std::string modeStr = body.value("mode", "hard");
        int delay = body.value("delay_seconds", 0);

        platform::FactoryResetMode mode =
            (modeStr == "soft") ? platform::FactoryResetMode::Soft
                                : platform::FactoryResetMode::Hard;

        bool ok = platform::PerformFactoryReset(mode, delay);

        ipcam::AuditLogger::Instance().LogSystemAction(
            "-", GetClientIP(ctx), "factory_reset",
            "mode=" + modeStr + " delay=" + std::to_string(delay));

        json resp;
        resp["success"] = ok;
        resp["mode"]    = modeStr;
        resp["message"] = ok
            ? "Factory reset initiated — device will reboot"
            : "Factory reset failed";
        resp["delay_seconds"] = delay;
        return JsonOk(resp);
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemFactoryReset: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 5. Diagnostics  –  GET  /api/v1/system/diagnostics
// ============================================================================

api::Response HandleSystemDiagnostics(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "GET") return MethodNotAllowed();

        auto rpt = platform::GetDiagnostics();

        json j;

        // CPU
        j["cpu"]["usage_percent"]    = rpt.cpu.usage_percent;
        j["cpu"]["num_cores"]        = rpt.cpu.num_cores;
        j["cpu"]["per_core_percent"] = rpt.cpu.per_core_percent;

        // Memory
        j["memory"]["total_bytes"]     = rpt.memory.total_bytes;
        j["memory"]["free_bytes"]      = rpt.memory.free_bytes;
        j["memory"]["available_bytes"] = rpt.memory.available_bytes;
        j["memory"]["buffers_bytes"]   = rpt.memory.buffers_bytes;
        j["memory"]["cached_bytes"]    = rpt.memory.cached_bytes;
        j["memory"]["usage_percent"]   = rpt.memory.usage_percent;

        // Disks
        j["disks"] = json::array();
        for (auto& d : rpt.disks) {
            j["disks"].push_back({
                {"mount_point",   d.mount_point},
                {"device",        d.device},
                {"filesystem",    d.filesystem},
                {"total_bytes",   d.total_bytes},
                {"free_bytes",    d.free_bytes},
                {"usage_percent", d.usage_percent}
            });
        }

        // Processes
        j["processes"] = json::array();
        for (auto& p : rpt.processes) {
            j["processes"].push_back({
                {"pid",           p.pid},
                {"name",          p.name},
                {"state",         p.state},
                {"cpu_percent",   p.cpu_percent},
                {"memory_kb",     p.memory_kb}
            });
        }

        // General
        j["uptime"]          = rpt.uptime;
        j["uptime_seconds"]  = rpt.uptime_seconds;
        j["temperature_c"]   = rpt.temperature;
        j["kernel_version"]  = rpt.kernel_version;

        return JsonOk(j);
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemDiagnostics: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 6a. Logs  –  GET / DELETE  /api/v1/system/logs
// ============================================================================

api::Response HandleSystemLogs(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            platform::LogFilter f;
            // Parse query params
            auto qp = [&](const std::string& key) -> std::string {
                auto it = ctx.query_params.find(key);
                return (it != ctx.query_params.end()) ? it->second : "";
            };
            f.level      = qp("level");
            f.start_time = qp("start");
            f.end_time   = qp("end");
            f.search     = qp("search");
            if (!qp("limit").empty())  f.limit  = std::stoi(qp("limit"));
            if (!qp("offset").empty()) f.offset = std::stoi(qp("offset"));

            // Multi-source support: ?source=kernel, ?source=thermal, etc.
            std::string src = qp("source");
            if (!src.empty()) {
                f.log_source = platform::LogSourceFromString(src);
            }

            auto entries = platform::GetLogs(f);

            json j;
            j["source"] = platform::LogSourceToString(f.log_source);
            j["total"] = (int)entries.size();
            j["logs"]  = json::array();
            for (auto& e : entries) {
                j["logs"].push_back({
                    {"timestamp", e.timestamp},
                    {"level",     e.level},
                    {"source",    e.source},
                    {"message",   e.message}
                });
            }
            return JsonOk(j);
        }

        if (ctx.method == "DELETE") {
            auto qp = [&](const std::string& key) -> std::string {
                auto it = ctx.query_params.find(key);
                return (it != ctx.query_params.end()) ? it->second : "";
            };
            std::string src = qp("source");
            bool ok;
            if (!src.empty()) {
                ok = platform::ClearLogs(platform::LogSourceFromString(src));
            } else {
                ok = platform::ClearLogs();
            }
            json resp;
            resp["success"] = ok;
            resp["message"] = ok ? "Logs cleared" : "Failed to clear logs";
            if (!src.empty()) resp["source"] = src;
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLogs: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 6b. Log Download  –  GET  /api/v1/system/logs/download
// ============================================================================

api::Response HandleSystemLogsDownload(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "GET") return MethodNotAllowed();

        // Support ?source= to download specific log source
        auto it = ctx.query_params.find("source");
        std::string path;
        std::string filename;
        if (it != ctx.query_params.end() && !it->second.empty()) {
            auto src = platform::LogSourceFromString(it->second);
            path = platform::GetLogFilePath(src);
            filename = std::string(platform::LogSourceToString(src)) + ".log";
        } else {
            path = platform::GetLogFilePath();
            filename = "ipcamd.log";
        }

        struct stat st;
        if (stat(path.c_str(), &st) != 0) {
            return ErrorResponse(404, "Log file not found");
        }

        api::Response r;
        r.status_code   = 200;
        r.content_type  = "application/octet-stream";
        r.file_path     = path;
        r.file_size     = static_cast<size_t>(st.st_size);
        r.headers["Content-Disposition"] = "attachment; filename=\"" + filename + "\"";
        return r;
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLogsDownload: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 6c. Log Config  –  GET / PUT  /api/v1/system/logs/config
// ============================================================================

api::Response HandleSystemLogsConfig(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            auto c = platform::GetLogConfig();
            json j;
            j["level"]              = c.level;
            j["console"]["enabled"] = c.console_enabled;
            j["console"]["level"]   = c.console_level;
            j["file"]["enabled"]    = c.file_enabled;
            j["file"]["path"]       = c.file_path;
            j["file"]["level"]      = c.file_level;
            j["file"]["max_size_mb"]       = c.max_size_mb;
            j["file"]["max_files"]         = c.max_files;
            j["file"]["flush_interval_sec"]= c.flush_interval_sec;
            j["format"] = c.format;
            return JsonOk(j);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto c = platform::GetLogConfig();

            if (body.contains("level"))  c.level = body["level"];
            if (body.contains("format")) c.format = body["format"];
            if (body.contains("console")) {
                auto& con = body["console"];
                if (con.contains("enabled")) c.console_enabled = con["enabled"];
                if (con.contains("level"))   c.console_level   = con["level"];
            }
            if (body.contains("file")) {
                auto& fi = body["file"];
                if (fi.contains("enabled"))            c.file_enabled       = fi["enabled"];
                if (fi.contains("path"))               c.file_path          = fi["path"];
                if (fi.contains("level"))              c.file_level         = fi["level"];
                if (fi.contains("max_size_mb"))        c.max_size_mb        = fi["max_size_mb"];
                if (fi.contains("max_files"))          c.max_files          = fi["max_files"];
                if (fi.contains("flush_interval_sec")) c.flush_interval_sec = fi["flush_interval_sec"];
            }

            bool ok = platform::SetLogConfig(c);
            if (!ok) return ErrorResponse(400, "Failed to update log config");

            json resp;
            resp["success"] = true;
            resp["message"] = "Log configuration updated";

            ipcam::AuditLogger::Instance().LogConfigChange(
                "-", GetClientIP(ctx), "logging.config",
                "", body.dump());

            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLogsConfig: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 7. Maintenance  –  GET / PUT  /api/v1/system/maintenance
// ============================================================================

api::Response HandleSystemMaintenance(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            auto c = platform::GetMaintenanceConfig();
            json j;
            j["auto_restart_enabled"] = c.auto_restart_enabled;
            j["restart_time"]         = c.restart_time;
            j["restart_days"]         = c.restart_days;
            return JsonOk(j);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto c = platform::GetMaintenanceConfig();

            if (body.contains("auto_restart_enabled")) c.auto_restart_enabled = body["auto_restart_enabled"];
            if (body.contains("restart_time"))         c.restart_time         = body["restart_time"];
            if (body.contains("restart_days"))         c.restart_days         = body["restart_days"];

            bool ok = platform::SetMaintenanceConfig(c);
            if (!ok) return ErrorResponse(400, "Failed to update maintenance config");

            json resp;
            resp["success"] = true;
            resp["config"]  = {
                {"auto_restart_enabled", c.auto_restart_enabled},
                {"restart_time", c.restart_time},
                {"restart_days", c.restart_days}
            };
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemMaintenance: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 8. Watchdog  –  GET / PUT  /api/v1/system/watchdog
// ============================================================================

api::Response HandleSystemWatchdog(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            auto c = platform::GetWatchdogConfig();
            json j;
            j["enabled"]          = c.enabled;
            j["interval_seconds"] = c.interval_seconds;
            return JsonOk(j);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto c = platform::GetWatchdogConfig();

            if (body.contains("enabled"))          c.enabled          = body["enabled"];
            if (body.contains("interval_seconds")) c.interval_seconds = body["interval_seconds"];

            // Validate interval
            if (c.interval_seconds < 1 || c.interval_seconds > 3600) {
                return ErrorResponse(400, "interval_seconds must be 1-3600");
            }

            bool ok = platform::SetWatchdogConfig(c);
            if (!ok) return ErrorResponse(400, "Failed to update watchdog config");

            json resp;
            resp["success"]          = true;
            resp["enabled"]          = c.enabled;
            resp["interval_seconds"] = c.interval_seconds;
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemWatchdog: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 10a. Config Backup  –  GET  /api/v1/system/backup
// ============================================================================

api::Response HandleSystemBackup(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "GET") return MethodNotAllowed();

        std::string path = platform::CreateConfigBackup();
        if (path.empty()) {
            return ErrorResponse(500, "Failed to create config backup");
        }

        struct stat st;
        if (stat(path.c_str(), &st) != 0) {
            return ErrorResponse(500, "Backup file not found after creation");
        }

        api::Response r;
        r.status_code  = 200;
        r.content_type = "application/gzip";
        r.file_path    = path;
        r.file_size    = static_cast<size_t>(st.st_size);
        r.headers["Content-Disposition"] = "attachment; filename=\"config_backup.tar.gz\"";
        return r;
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemBackup: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 10b. Config Restore  –  POST  /api/v1/system/restore
// ============================================================================

api::Response HandleSystemRestore(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "POST") return MethodNotAllowed();

        // The body contains the raw tar.gz data.
        // Write it to a temp file.
        std::string tmpPath = "/tmp/config_restore_upload.tar.gz";
        {
            std::ofstream f(tmpPath, std::ios::binary);
            if (!f) return ErrorResponse(500, "Cannot write temp file");
            f.write(ctx.body.data(), ctx.body.size());
        }

        std::string err = platform::RestoreConfigBackup(tmpPath);
        if (!err.empty()) {
            return ErrorResponse(400, err);
        }

        json resp;
        resp["success"] = true;
        resp["message"] = "Configuration restored successfully — restart recommended";
        return JsonOk(resp);
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemRestore: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 6d. Log Sources  –  GET  /api/v1/system/logs/sources
// ============================================================================

api::Response HandleSystemLogsSources(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "GET") return MethodNotAllowed();

        auto sources = platform::GetLogSources();
        json j;
        j["sources"] = json::array();
        for (auto& s : sources) {
            json src;
            src["id"]           = s.id;
            src["display_name"] = s.display_name;
            src["description"]  = s.description;
            src["file_path"]    = s.file_path;
            src["available"]    = s.available;
            src["file_size"]    = s.file_size;
            if (!s.sd_path.empty()) src["sd_path"] = s.sd_path;
            j["sources"].push_back(src);
        }
        return JsonOk(j);
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLogsSources: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 6e. Security Log  –  GET  /api/v1/system/logs/security
// ============================================================================

api::Response HandleSystemLogsSecurity(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            platform::LogFilter f;
            f.log_source = platform::LogSource::Security;
            auto qp = [&](const std::string& key) -> std::string {
                auto it = ctx.query_params.find(key);
                return (it != ctx.query_params.end()) ? it->second : "";
            };
            f.level      = qp("level");
            f.start_time = qp("start");
            f.end_time   = qp("end");
            f.search     = qp("search");
            if (!qp("limit").empty())  f.limit  = std::stoi(qp("limit"));
            if (!qp("offset").empty()) f.offset = std::stoi(qp("offset"));

            auto entries = platform::GetLogs(f);

            json j;
            j["source"] = "security";
            j["total"]  = (int)entries.size();
            j["events"] = json::array();
            for (auto& e : entries) {
                j["events"].push_back({
                    {"timestamp", e.timestamp},
                    {"level",     e.level},
                    {"source",    e.source},
                    {"message",   e.message}
                });
            }
            return JsonOk(j);
        }

        if (ctx.method == "DELETE") {
            bool ok = platform::ClearLogs(platform::LogSource::Security);
            json resp;
            resp["success"] = ok;
            resp["message"] = ok ? "Security log cleared" : "Failed to clear security log";
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLogsSecurity: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 6f. Audit Log  –  GET  /api/v1/system/logs/audit
// ============================================================================

api::Response HandleSystemLogsAudit(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            platform::LogFilter f;
            f.log_source = platform::LogSource::Audit;
            auto qp = [&](const std::string& key) -> std::string {
                auto it = ctx.query_params.find(key);
                return (it != ctx.query_params.end()) ? it->second : "";
            };
            f.level      = qp("level");
            f.start_time = qp("start");
            f.end_time   = qp("end");
            f.search     = qp("search");
            if (!qp("limit").empty())  f.limit  = std::stoi(qp("limit"));
            if (!qp("offset").empty()) f.offset = std::stoi(qp("offset"));

            auto entries = platform::GetLogs(f);

            json j;
            j["source"] = "audit";
            j["total"]  = (int)entries.size();
            j["events"] = json::array();
            for (auto& e : entries) {
                j["events"].push_back({
                    {"timestamp", e.timestamp},
                    {"level",     e.level},
                    {"source",    e.source},
                    {"message",   e.message}
                });
            }
            return JsonOk(j);
        }

        if (ctx.method == "DELETE") {
            bool ok = platform::ClearLogs(platform::LogSource::Audit);
            json resp;
            resp["success"] = ok;
            resp["message"] = ok ? "Audit log cleared" : "Failed to clear audit log";
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLogsAudit: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 6g. Access Log  –  GET  /api/v1/system/logs/access
// ============================================================================

api::Response HandleSystemLogsAccess(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            platform::LogFilter f;
            f.log_source = platform::LogSource::Access;
            auto qp = [&](const std::string& key) -> std::string {
                auto it = ctx.query_params.find(key);
                return (it != ctx.query_params.end()) ? it->second : "";
            };
            f.level      = qp("level");
            f.start_time = qp("start");
            f.end_time   = qp("end");
            f.search     = qp("search");
            if (!qp("limit").empty())  f.limit  = std::stoi(qp("limit"));
            if (!qp("offset").empty()) f.offset = std::stoi(qp("offset"));

            auto entries = platform::GetLogs(f);

            json j;
            j["source"] = "access";
            j["total"]  = (int)entries.size();
            j["requests"] = json::array();
            for (auto& e : entries) {
                j["requests"].push_back({
                    {"timestamp", e.timestamp},
                    {"level",     e.level},
                    {"source",    e.source},
                    {"message",   e.message}
                });
            }
            return JsonOk(j);
        }

        if (ctx.method == "DELETE") {
            bool ok = platform::ClearLogs(platform::LogSource::Access);
            json resp;
            resp["success"] = ok;
            resp["message"] = ok ? "Access log cleared" : "Failed to clear access log";
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLogsAccess: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 6h. Remote Syslog Config  –  GET / PUT  /api/v1/system/logs/syslog
// ============================================================================

api::Response HandleSystemLogsSyslog(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            json j;
            j["enabled"]           = config::Get<bool>("logging.syslog.enabled", false);
            j["host"]              = config::Get<std::string>("logging.syslog.host", "");
            j["port"]              = config::Get<int>("logging.syslog.port", 514);
            j["protocol"]          = config::Get<std::string>("logging.syslog.protocol", "udp");
            j["facility"]          = config::Get<std::string>("logging.syslog.facility", "local0");
            j["min_level"]         = config::Get<std::string>("logging.syslog.min_level", "warning");
            j["include_access"]    = config::Get<bool>("logging.syslog.include_access", false);
            j["include_security"]  = config::Get<bool>("logging.syslog.include_security", true);
            j["include_audit"]     = config::Get<bool>("logging.syslog.include_audit", true);
            return JsonOk(j);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);

            if (body.contains("enabled"))          config::Set("logging.syslog.enabled", body["enabled"].get<bool>());
            if (body.contains("host"))             config::Set("logging.syslog.host", body["host"].get<std::string>());
            if (body.contains("port"))             config::Set("logging.syslog.port", body["port"].get<int>());
            if (body.contains("protocol")) {
                std::string proto = body["protocol"].get<std::string>();
                if (proto != "udp" && proto != "tcp") {
                    return ErrorResponse(400, "protocol must be 'udp' or 'tcp'");
                }
                config::Set("logging.syslog.protocol", proto);
            }
            if (body.contains("facility")) {
                std::string fac = body["facility"].get<std::string>();
                // Validate: local0-local7, kern, user, daemon, auth, syslog, lpr, news, uucp
                static const std::vector<std::string> valid = {
                    "local0","local1","local2","local3","local4","local5","local6","local7",
                    "kern","user","daemon","auth","syslog","lpr","news","uucp"
                };
                bool ok = false;
                for (auto& v : valid) if (v == fac) { ok = true; break; }
                if (!ok) return ErrorResponse(400, "facility must be a syslog facility name (e.g. local0)");
                config::Set("logging.syslog.facility", fac);
            }
            if (body.contains("min_level"))        config::Set("logging.syslog.min_level", body["min_level"].get<std::string>());
            if (body.contains("include_access"))   config::Set("logging.syslog.include_access", body["include_access"].get<bool>());
            if (body.contains("include_security")) config::Set("logging.syslog.include_security", body["include_security"].get<bool>());
            if (body.contains("include_audit"))    config::Set("logging.syslog.include_audit", body["include_audit"].get<bool>());

            // Reload AuditLogger with new syslog config
            auto& al = ipcam::AuditLogger::Instance();
            ipcam::AuditLogConfig cfg;
            cfg.access_path       = config::Get<std::string>("logging.access.path", "/var/log/ipcamd_access.log");
            cfg.security_path     = config::Get<std::string>("logging.security.path", "/var/log/ipcamd_security.log");
            cfg.audit_path        = config::Get<std::string>("logging.audit.path", "/var/log/ipcamd_audit.log");
            cfg.access_max_size_mb   = config::Get<int>("logging.access.max_size_mb", 10);
            cfg.security_max_size_mb = config::Get<int>("logging.security.max_size_mb", 10);
            cfg.audit_max_size_mb    = config::Get<int>("logging.audit.max_size_mb", 10);
            cfg.syslog_enabled    = config::Get<bool>("logging.syslog.enabled", false);
            cfg.syslog_server     = config::Get<std::string>("logging.syslog.host", "");
            cfg.syslog_port       = config::Get<int>("logging.syslog.port", 514);
            cfg.syslog_protocol   = config::Get<std::string>("logging.syslog.protocol", "udp");
            cfg.syslog_facility   = config::Get<std::string>("logging.syslog.facility", "local0");
            cfg.syslog_min_level  = config::Get<std::string>("logging.syslog.min_level", "warning");
            al.Reload(cfg);

            json resp;
            resp["success"] = true;
            resp["message"] = "Remote syslog configuration updated";

            ipcam::AuditLogger::Instance().LogConfigChange(
                "-", GetClientIP(ctx), "logging.syslog",
                "", body.dump());

            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLogsSyslog: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 6i. Diagnostic Report  –  GET  /api/v1/system/diagnostics/report
// ============================================================================

api::Response HandleSystemDiagReport(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "GET") return MethodNotAllowed();

        std::string path = platform::CreateDiagnosticBundle();
        if (path.empty()) {
            return ErrorResponse(500, "Failed to create diagnostic bundle");
        }

        struct stat st;
        if (stat(path.c_str(), &st) != 0) {
            return ErrorResponse(500, "Diagnostic bundle file not found after creation");
        }

        // Generate timestamped filename
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        struct tm tm_buf{};
        gmtime_r(&t, &tm_buf);
        char ts[32];
        std::strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", &tm_buf);
        std::string filename = std::string("diagnostic_") + ts + ".tar.gz";

        api::Response r;
        r.status_code  = 200;
        r.content_type = "application/gzip";
        r.file_path    = path;
        r.file_size    = static_cast<size_t>(st.st_size);
        r.headers["Content-Disposition"] = "attachment; filename=\"" + filename + "\"";
        return r;
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemDiagReport: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// 11. Alarm Thresholds  –  GET / PUT  /api/v1/system/alarms
// ============================================================================

api::Response HandleSystemAlarms(const api::RequestContext& ctx) {
    try {
        if (ctx.method == "GET") {
            auto t = platform::GetAlarmThresholds();

            // Also include current values for comparison
            auto diag = platform::GetDiagnostics();

            json j;
            j["thresholds"]["disk_space_threshold"]  = t.disk_space_threshold;
            j["thresholds"]["temperature_threshold"]  = t.temperature_threshold;
            j["thresholds"]["memory_threshold"]       = t.memory_threshold;
            j["thresholds"]["alarm_outputs_number"]   = t.alarm_outputs_number;

            // Current values
            j["current"]["temperature_c"]     = diag.temperature;
            j["current"]["memory_usage_pct"]  = diag.memory.usage_percent;
            // Disk usage: take root filesystem
            float diskPct = 0.0f;
            for (auto& d : diag.disks) {
                if (d.mount_point == "/" || d.mount_point == "/mnt/sd") {
                    diskPct = std::max(diskPct, d.usage_percent);
                }
            }
            j["current"]["disk_usage_pct"] = diskPct;

            // Alarm active flags
            j["alarms_active"]["disk_space"]  = (diskPct >= t.disk_space_threshold);
            j["alarms_active"]["temperature"] = (diag.temperature >= t.temperature_threshold);
            j["alarms_active"]["memory"]      = (diag.memory.usage_percent >= t.memory_threshold);

            return JsonOk(j);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);
            auto t = platform::GetAlarmThresholds();

            if (body.contains("disk_space_threshold"))  t.disk_space_threshold  = body["disk_space_threshold"];
            if (body.contains("temperature_threshold")) t.temperature_threshold = body["temperature_threshold"];
            if (body.contains("memory_threshold"))      t.memory_threshold      = body["memory_threshold"];
            if (body.contains("alarm_outputs_number"))  t.alarm_outputs_number  = body["alarm_outputs_number"];

            bool ok = platform::SetAlarmThresholds(t);
            if (!ok) return ErrorResponse(400, "Invalid threshold values");

            json resp;
            resp["success"] = true;
            resp["thresholds"] = {
                {"disk_space_threshold",  t.disk_space_threshold},
                {"temperature_threshold", t.temperature_threshold},
                {"memory_threshold",      t.memory_threshold},
                {"alarm_outputs_number",  t.alarm_outputs_number}
            };
            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemAlarms: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

// ============================================================================
// System Logger Control
// ============================================================================

api::Response HandleSystemLogger(const api::RequestContext& ctx) {
    try {
        auto& logger = platform::SystemLogger::Instance();

        if (ctx.method == "GET") {
            auto status = logger.GetStatus();
            auto cfg = logger.GetConfig();

            json sources = json::array();
            for (auto& s : status.sources) {
                sources.push_back({
                    {"id", s.id},
                    {"enabled", s.enabled},
                    {"interval_sec", s.interval_sec},
                    {"last_collection", s.last_collection},
                    {"entries_collected", s.entries_collected}
                });
            }

            json destinations = json::array();
            for (auto& d : status.destinations) {
                destinations.push_back({
                    {"id", d.id},
                    {"path", d.path},
                    {"enabled", d.enabled},
                    {"available", d.available},
                    {"read_write", d.read_write},
                    {"free_bytes", d.free_bytes},
                    {"used_bytes", d.used_bytes},
                    {"total_bytes", d.total_bytes}
                });
            }

            json resp;
            resp["running"] = status.running;
            resp["enabled"] = cfg.enabled;
            resp["sources"] = sources;
            resp["destinations"] = destinations;
            return JsonOk(resp);
        }

        if (ctx.method == "PUT") {
            auto body = json::parse(ctx.body);

            // Global enable/disable
            if (body.contains("enabled")) {
                logger.SetEnabled(body["enabled"].get<bool>());
            }

            // Per-source config
            if (body.contains("sources")) {
                for (auto& [key, val] : body["sources"].items()) {
                    auto src = platform::CollectionSourceFromString(key);
                    if (val.contains("enabled")) {
                        logger.SetSourceEnabled(src, val["enabled"].get<bool>());
                    }
                    if (val.contains("interval_sec")) {
                        logger.SetSourceInterval(src, val["interval_sec"].get<int>());
                    }
                }
            }

            // Per-destination config
            if (body.contains("destinations")) {
                for (auto& [key, val] : body["destinations"].items()) {
                    if (val.contains("enabled")) {
                        logger.SetDestinationEnabled(key, val["enabled"].get<bool>());
                    }
                }
            }

            json resp;
            resp["success"] = true;
            resp["message"] = "System logger configuration updated";

            // Audit log: record the configuration change
            ipcam::AuditLogger::Instance().LogConfigChange(
                "-", GetClientIP(ctx), "logging.system_logger",
                "", body.dump());

            return JsonOk(resp);
        }

        return MethodNotAllowed();
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLogger: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

api::Response HandleSystemLoggerCollect(const api::RequestContext& ctx) {
    try {
        if (ctx.method != "POST") return MethodNotAllowed();

        auto& logger = platform::SystemLogger::Instance();

        // Optional ?source= param to collect a specific source
        auto it = ctx.query_params.find("source");
        if (it != ctx.query_params.end() && !it->second.empty()) {
            auto src = platform::CollectionSourceFromString(it->second);
            logger.CollectNow(src);
            json resp;
            resp["success"] = true;
            resp["message"] = "Collected " + it->second;
            return JsonOk(resp);
        }

        // Collect all enabled sources
        logger.CollectAll();
        json resp;
        resp["success"] = true;
        resp["message"] = "Collected all enabled sources";
        return JsonOk(resp);
    } catch (const std::exception& e) {
        spdlog::error("HandleSystemLoggerCollect: {}", e.what());
        return ErrorResponse(500, e.what());
    }
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
