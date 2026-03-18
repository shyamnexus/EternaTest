/**
 * @file storage_handler.cpp
 * @brief NAS and FTP Storage API handler implementations
 */

#include "storage_handler.h"
#include <ipcam/nas_manager.h>
#include <ipcam/ftp_manager.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// JSON Conversion Helpers - NAS
// ============================================================================

static json NasConfigToJson(const storage::NasConfig& config) {
    return json{
        {"enabled", config.enabled},
        {"type", storage::NasProtocolToString(config.type)},
        {"server", config.server},
        {"share_path", config.share_path},
        {"mount_point", config.mount_point},
        {"username", config.username},
        // Note: password not returned for security
        {"domain", config.domain},
        {"auto_mount", config.auto_mount},
        {"status", storage::NasStatusToString(config.status)},
        {"last_error", config.last_error}
    };
}

static storage::NasConfig JsonToNasConfig(const json& j, const storage::NasConfig& current) {
    storage::NasConfig config = current;
    
    if (j.contains("enabled")) config.enabled = j["enabled"].get<bool>();
    if (j.contains("type")) config.type = storage::StringToNasProtocol(j["type"].get<std::string>());
    if (j.contains("server")) config.server = j["server"].get<std::string>();
    if (j.contains("share_path")) config.share_path = j["share_path"].get<std::string>();
    if (j.contains("mount_point")) config.mount_point = j["mount_point"].get<std::string>();
    if (j.contains("username")) config.username = j["username"].get<std::string>();
    if (j.contains("password")) config.password = j["password"].get<std::string>();
    if (j.contains("domain")) config.domain = j["domain"].get<std::string>();
    if (j.contains("auto_mount")) config.auto_mount = j["auto_mount"].get<bool>();
    
    return config;
}

// ============================================================================
// JSON Conversion Helpers - FTP
// ============================================================================

static json FtpConfigToJson(const storage::FtpConfig& config) {
    return json{
        {"enabled", config.enabled},
        {"server", config.server},
        {"port", config.port},
        {"username", config.username},
        // Note: password not returned for security
        {"remote_path", config.remote_path},
        {"use_ssl", config.use_ssl},
        {"passive_mode", config.passive_mode},
        {"upload_schedule", storage::FtpUploadScheduleToString(config.upload_schedule)},
        {"upload_hour", config.upload_hour},
        {"upload_recordings", config.upload_recordings},
        {"upload_snapshots", config.upload_snapshots},
        {"delete_after_upload", config.delete_after_upload},
        {"max_retries", config.max_retries},
        {"timeout_seconds", config.timeout_seconds},
        {"status", storage::FtpStatusToString(config.status)},
        {"last_error", config.last_error}
    };
}

static storage::FtpConfig JsonToFtpConfig(const json& j, const storage::FtpConfig& current) {
    storage::FtpConfig config = current;
    
    if (j.contains("enabled")) config.enabled = j["enabled"].get<bool>();
    if (j.contains("server")) config.server = j["server"].get<std::string>();
    if (j.contains("port")) config.port = j["port"].get<int>();
    if (j.contains("username")) config.username = j["username"].get<std::string>();
    if (j.contains("password")) config.password = j["password"].get<std::string>();
    if (j.contains("remote_path")) config.remote_path = j["remote_path"].get<std::string>();
    if (j.contains("use_ssl")) config.use_ssl = j["use_ssl"].get<bool>();
    if (j.contains("passive_mode")) config.passive_mode = j["passive_mode"].get<bool>();
    if (j.contains("upload_schedule")) {
        config.upload_schedule = storage::StringToFtpUploadSchedule(j["upload_schedule"].get<std::string>());
    }
    if (j.contains("upload_hour")) config.upload_hour = j["upload_hour"].get<int>();
    if (j.contains("upload_recordings")) config.upload_recordings = j["upload_recordings"].get<bool>();
    if (j.contains("upload_snapshots")) config.upload_snapshots = j["upload_snapshots"].get<bool>();
    if (j.contains("delete_after_upload")) config.delete_after_upload = j["delete_after_upload"].get<bool>();
    if (j.contains("max_retries")) config.max_retries = j["max_retries"].get<int>();
    if (j.contains("timeout_seconds")) config.timeout_seconds = j["timeout_seconds"].get<int>();
    
    return config;
}

static json UploadItemToJson(const storage::FtpUploadItem& item) {
    return json{
        {"id", item.id},
        {"local_path", item.local_path},
        {"remote_path", item.remote_path},
        {"status", item.status == storage::UploadStatus::Pending ? "pending" :
                   item.status == storage::UploadStatus::InProgress ? "in_progress" :
                   item.status == storage::UploadStatus::Completed ? "completed" : "failed"},
        {"retry_count", item.retry_count},
        {"file_size", item.file_size},
        {"bytes_uploaded", item.bytes_uploaded},
        {"error_message", item.error_message},
        {"created_at", item.created_at},
        {"completed_at", item.completed_at}
    };
}

static json StatsToJson(const storage::FtpStats& stats) {
    return json{
        {"total_uploads", stats.total_uploads},
        {"successful_uploads", stats.successful_uploads},
        {"failed_uploads", stats.failed_uploads},
        {"bytes_uploaded", stats.bytes_uploaded},
        {"pending_uploads", stats.pending_uploads},
        {"last_upload_time", stats.last_upload_time}
    };
}

// ============================================================================
// NAS Handler Implementations
// ============================================================================

api::Response HandleNasConfig(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = storage::NasManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "NAS manager not initialized"}}.dump();
        return resp;
    }
    
    try {
        if (ctx.method == "GET") {
            auto config = mgr.GetConfig();
            
            json result = NasConfigToJson(config);
            result["is_mounted"] = mgr.IsMounted();
            
            // Add storage info if mounted
            if (mgr.IsMounted()) {
                int64_t total, free;
                if (mgr.GetStorageInfo(total, free)) {
                    result["total_bytes"] = total;
                    result["free_bytes"] = free;
                    result["used_bytes"] = total - free;
                    result["usage_percent"] = total > 0 ? static_cast<int>((total - free) * 100 / total) : 0;
                }
            }
            
            resp.status_code = 200;
            resp.body = result.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto j = json::parse(ctx.body);
            auto current = mgr.GetConfig();
            auto config = JsonToNasConfig(j, current);
            
            if (!mgr.SetConfig(config)) {
                resp.status_code = 500;
                resp.body = json{{"error", "Failed to save configuration"}}.dump();
                return resp;
            }
            
            spdlog::info("[StorageHandler] Updated NAS config for server: {}", config.server);
            
            resp.status_code = 200;
            resp.body = json{
                {"message", "Configuration updated"},
                {"config", NasConfigToJson(mgr.GetConfig())}
            }.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = json{{"error", "Method not allowed"}}.dump();
        }
    } catch (const json::exception& e) {
        spdlog::error("[StorageHandler] NAS JSON error: {}", e.what());
        resp.status_code = 400;
        resp.body = json{{"error", std::string("Invalid JSON: ") + e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("[StorageHandler] NAS error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleNasTest(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = storage::NasManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "NAS manager not initialized"}}.dump();
        return resp;
    }
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        // Optional: test with provided config instead of saved config
        storage::NasConfig test_config;
        bool use_provided = false;
        
        if (!ctx.body.empty()) {
            auto j = json::parse(ctx.body);
            auto current = mgr.GetConfig();
            test_config = JsonToNasConfig(j, current);
            use_provided = true;
        }
        
        bool success = use_provided ? mgr.TestConnection(&test_config) : mgr.TestConnection();
        
        if (success) {
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "Connection test successful"}
            }.dump(2);
        } else {
            resp.status_code = 400;
            resp.body = json{
                {"success", false},
                {"error", mgr.GetLastError()}
            }.dump(2);
        }
    } catch (const std::exception& e) {
        spdlog::error("[StorageHandler] NAS test error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleNasMount(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = storage::NasManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "NAS manager not initialized"}}.dump();
        return resp;
    }
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        if (mgr.Mount()) {
            spdlog::info("[StorageHandler] NAS mounted successfully");
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "NAS mounted successfully"},
                {"status", storage::NasStatusToString(mgr.GetStatus())}
            }.dump(2);
        } else {
            resp.status_code = 500;
            resp.body = json{
                {"success", false},
                {"error", mgr.GetLastError()}
            }.dump(2);
        }
    } catch (const std::exception& e) {
        spdlog::error("[StorageHandler] NAS mount error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleNasUnmount(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = storage::NasManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "NAS manager not initialized"}}.dump();
        return resp;
    }
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        if (mgr.Unmount()) {
            spdlog::info("[StorageHandler] NAS unmounted successfully");
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "NAS unmounted successfully"},
                {"status", storage::NasStatusToString(mgr.GetStatus())}
            }.dump(2);
        } else {
            resp.status_code = 500;
            resp.body = json{
                {"success", false},
                {"error", mgr.GetLastError()}
            }.dump(2);
        }
    } catch (const std::exception& e) {
        spdlog::error("[StorageHandler] NAS unmount error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

// ============================================================================
// FTP Handler Implementations
// ============================================================================

api::Response HandleFtpConfig(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = storage::FtpManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "FTP manager not initialized"}}.dump();
        return resp;
    }
    
    try {
        if (ctx.method == "GET") {
            auto config = mgr.GetConfig();
            auto stats = mgr.GetStats();
            
            json result = FtpConfigToJson(config);
            result["stats"] = StatsToJson(stats);
            
            resp.status_code = 200;
            resp.body = result.dump(2);
        }
        else if (ctx.method == "PUT") {
            auto j = json::parse(ctx.body);
            auto current = mgr.GetConfig();
            auto config = JsonToFtpConfig(j, current);
            
            if (!mgr.SetConfig(config)) {
                resp.status_code = 500;
                resp.body = json{{"error", "Failed to save configuration"}}.dump();
                return resp;
            }
            
            spdlog::info("[StorageHandler] Updated FTP config for server: {}", config.server);
            
            resp.status_code = 200;
            resp.body = json{
                {"message", "Configuration updated"},
                {"config", FtpConfigToJson(mgr.GetConfig())}
            }.dump(2);
        }
        else {
            resp.status_code = 405;
            resp.body = json{{"error", "Method not allowed"}}.dump();
        }
    } catch (const json::exception& e) {
        spdlog::error("[StorageHandler] FTP JSON error: {}", e.what());
        resp.status_code = 400;
        resp.body = json{{"error", std::string("Invalid JSON: ") + e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("[StorageHandler] FTP error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleFtpTest(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = storage::FtpManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "FTP manager not initialized"}}.dump();
        return resp;
    }
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        // Optional: test with provided config
        storage::FtpConfig test_config;
        bool use_provided = false;
        
        if (!ctx.body.empty()) {
            auto j = json::parse(ctx.body);
            auto current = mgr.GetConfig();
            test_config = JsonToFtpConfig(j, current);
            use_provided = true;
        }
        
        bool success = use_provided ? mgr.TestConnection(&test_config) : mgr.TestConnection();
        
        if (success) {
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "Connection test successful"}
            }.dump(2);
        } else {
            resp.status_code = 400;
            resp.body = json{
                {"success", false},
                {"error", mgr.GetLastError()}
            }.dump(2);
        }
    } catch (const std::exception& e) {
        spdlog::error("[StorageHandler] FTP test error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleFtpUpload(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = storage::FtpManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "FTP manager not initialized"}}.dump();
        return resp;
    }
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        // Trigger immediate upload of pending items
        mgr.TriggerUpload();
        
        auto stats = mgr.GetStats();
        
        spdlog::info("[StorageHandler] FTP upload triggered, {} pending items", stats.pending_uploads);
        
        resp.status_code = 200;
        resp.body = json{
            {"message", "Upload triggered"},
            {"pending_uploads", stats.pending_uploads}
        }.dump(2);
        
    } catch (const std::exception& e) {
        spdlog::error("[StorageHandler] FTP upload error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleFtpQueue(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = storage::FtpManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "FTP manager not initialized"}}.dump();
        return resp;
    }
    
    if (ctx.method != "GET") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto pending = mgr.GetPendingUploads();
        
        json items = json::array();
        for (const auto& item : pending) {
            items.push_back(UploadItemToJson(item));
        }
        
        resp.status_code = 200;
        resp.body = json{
            {"pending", items},
            {"count", static_cast<int>(pending.size())}
        }.dump(2);
        
    } catch (const std::exception& e) {
        spdlog::error("[StorageHandler] FTP queue error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleFtpHistory(const api::RequestContext& ctx) {
    api::Response resp;
    auto& mgr = storage::FtpManager::Instance();
    
    if (!mgr.IsInitialized()) {
        resp.status_code = 503;
        resp.body = json{{"error", "FTP manager not initialized"}}.dump();
        return resp;
    }
    
    if (ctx.method != "GET") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        // Get max items from query param (default 50)
        int max_items = 50;
        auto it = ctx.query_params.find("limit");
        if (it != ctx.query_params.end()) {
            try {
                max_items = std::stoi(it->second);
                if (max_items < 1) max_items = 1;
                if (max_items > 500) max_items = 500;
            } catch (...) {}
        }
        
        auto history = mgr.GetUploadHistory(max_items);
        
        json items = json::array();
        for (const auto& item : history) {
            items.push_back(UploadItemToJson(item));
        }
        
        resp.status_code = 200;
        resp.body = json{
            {"history", items},
            {"count", static_cast<int>(history.size())}
        }.dump(2);
        
    } catch (const std::exception& e) {
        spdlog::error("[StorageHandler] FTP history error: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
