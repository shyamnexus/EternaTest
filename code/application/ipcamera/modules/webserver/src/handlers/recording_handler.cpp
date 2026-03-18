/**
 * @file recording_handler.cpp
 * @brief Recording API HTTP handler implementations
 */

#include "recording_handler.h"
#include <ipcam/recording.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// Helper Functions
// ============================================================================

/// Build JSON from RecordingInfo
static json RecordingInfoToJson(const recording::RecordingInfo& info) {
    json j;
    j["id"] = info.id;
    j["filename"] = info.filename;
    j["channel"] = info.channel;
    j["type"] = recording::RecordingTypeToString(info.type);
    j["start_time"] = info.start_time;
    j["end_time"] = info.end_time;
    j["duration_ms"] = info.duration_ms;
    j["file_size"] = info.file_size;
    j["file_size_human"] = recording::FormatBytes(info.file_size);
    j["width"] = info.width;
    j["height"] = info.height;
    j["fps"] = info.fps;
    j["codec"] = info.codec;
    j["encrypted"] = info.encrypted;
    j["status"] = info.status;
    if (!info.thumbnail.empty()) {
        j["thumbnail"] = info.thumbnail;
    }
    return j;
}

/// Build JSON from RecordingSession
static json SessionToJson(const recording::RecordingSession& session) {
    json j;
    j["channel"] = session.channel;
    j["status"] = recording::RecordingStatusToString(session.status);
    j["type"] = recording::RecordingTypeToString(session.type);
    j["current_file"] = session.current_file;
    j["start_time"] = session.start_time;
    j["segment_start_time"] = session.segment_start_time;
    j["bytes_written"] = session.bytes_written;
    j["bytes_written_human"] = recording::FormatBytes(session.bytes_written);
    j["segments_completed"] = session.segments_completed;
    if (!session.error_message.empty()) {
        j["error"] = session.error_message;
    }
    return j;
}

/// Build JSON from ChannelConfig
static json ChannelConfigToJson(const recording::ChannelConfig& config) {
    json j;
    j["id"] = config.id;
    j["enabled"] = config.enabled;
    j["folder_name"] = config.folder_name;
    j["format"] = recording::FileFormatToString(config.format);
    j["file_duration_sec"] = config.file_duration_sec;
    j["max_files"] = config.max_files;
    j["quota_mb"] = config.quota_mb;
    j["stream_id"] = config.stream_id;
    return j;
}

/// Parse ChannelConfig from JSON
static recording::ChannelConfig ParseChannelConfig(const json& j, const recording::ChannelConfig& current) {
    recording::ChannelConfig config = current;
    
    if (j.contains("enabled")) config.enabled = j["enabled"].get<bool>();
    if (j.contains("folder_name")) config.folder_name = j["folder_name"].get<std::string>();
    if (j.contains("format")) config.format = recording::StringToFileFormat(j["format"].get<std::string>());
    if (j.contains("file_duration_sec")) config.file_duration_sec = j["file_duration_sec"].get<int>();
    if (j.contains("max_files")) config.max_files = j["max_files"].get<int>();
    if (j.contains("quota_mb")) config.quota_mb = j["quota_mb"].get<int64_t>();
    if (j.contains("stream_id")) config.stream_id = j["stream_id"].get<std::string>();
    
    return config;
}

/// Build JSON from StorageStats
static json StorageStatsToJson(const recording::StorageStats& stats) {
    json j;
    j["status"] = recording::StorageStatusToString(stats.status);
    j["total_bytes"] = stats.total_bytes;
    j["total_human"] = recording::FormatBytes(stats.total_bytes);
    j["used_bytes"] = stats.used_bytes;
    j["used_human"] = recording::FormatBytes(stats.used_bytes);
    j["free_bytes"] = stats.free_bytes;
    j["free_human"] = recording::FormatBytes(stats.free_bytes);
    j["recordings_bytes"] = stats.recordings_bytes;
    j["recordings_human"] = recording::FormatBytes(stats.recordings_bytes);
    j["total_files"] = stats.total_files;
    j["oldest_file_age_hours"] = stats.oldest_file_age_hours;
    j["filesystem_type"] = stats.filesystem_type;
    
    // Usage percentage
    if (stats.total_bytes > 0) {
        j["usage_percent"] = static_cast<int>((stats.used_bytes * 100) / stats.total_bytes);
    } else {
        j["usage_percent"] = 0;
    }
    
    return j;
}

/// Extract path parameter (e.g., /api/v1/recordings/{id})
static std::string ExtractPathParam(const std::string& uri, int position) {
    std::vector<std::string> parts;
    std::istringstream iss(uri);
    std::string part;
    while (std::getline(iss, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    if (position >= 0 && position < static_cast<int>(parts.size())) {
        return parts[position];
    }
    return "";
}

/// Parse query parameter
static std::string GetQueryParam(const api::RequestContext& ctx, const std::string& name, const std::string& default_value = "") {
    // URI format: /path?param1=value1&param2=value2
    size_t qpos = ctx.uri.find('?');
    if (qpos == std::string::npos) return default_value;
    
    std::string query = ctx.uri.substr(qpos + 1);
    std::istringstream iss(query);
    std::string param;
    
    while (std::getline(iss, param, '&')) {
        size_t eq = param.find('=');
        if (eq != std::string::npos) {
            std::string key = param.substr(0, eq);
            std::string value = param.substr(eq + 1);
            if (key == name) {
                return value;
            }
        }
    }
    return default_value;
}

static int GetQueryParamInt(const api::RequestContext& ctx, const std::string& name, int default_value = 0) {
    std::string value = GetQueryParam(ctx, name);
    if (value.empty()) return default_value;
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

static int64_t GetQueryParamInt64(const api::RequestContext& ctx, const std::string& name, int64_t default_value = 0) {
    std::string value = GetQueryParam(ctx, name);
    if (value.empty()) return default_value;
    try {
        return std::stoll(value);
    } catch (...) {
        return default_value;
    }
}

// ============================================================================
// Recording Status and Control
// ============================================================================

api::Response HandleRecording(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        json j;
        j["initialized"] = service.IsInitialized();
        j["storage_available"] = service.IsStorageAvailable();
        
        // Add active sessions summary
        auto sessions = service.GetActiveSessions();
        j["active_sessions"] = static_cast<int>(sessions.size());
        
        // Add per-channel status
        j["channels"] = json::array();
        for (int ch = 0; ch < 3; ch++) {
            auto session = service.GetSession(ch);
            j["channels"].push_back(SessionToJson(session));
        }
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecording: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingSessions(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        auto sessions = service.GetActiveSessions();
        
        json j;
        j["sessions"] = json::array();
        for (const auto& session : sessions) {
            j["sessions"].push_back(SessionToJson(session));
        }
        j["count"] = static_cast<int>(sessions.size());
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingSessions: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingStart(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto body = json::parse(ctx.body);
        
        int channel = body.value("channel", 0);
        std::string type_str = body.value("type", "manual");
        auto type = recording::StringToRecordingType(type_str);
        
        // Check SD card is inserted
        if (!service.IsSdCardInserted()) {
            resp.status_code = 503;
            resp.body = json{
                {"error", "No SD card detected"},
                {"detail", "Insert an SD card to enable recording"}
            }.dump();
            return resp;
        }
        
        // Check SD card is mounted
        if (!service.IsSdCardMounted()) {
            resp.status_code = 503;
            resp.body = json{
                {"error", "SD card not mounted"},
                {"detail", "Mount the SD card before recording. Use POST /api/v1/sdcard/mount"}
            }.dump();
            return resp;
        }
        
        // Check filesystem is supported
        std::string fs = service.GetSdCardFilesystem();
        if (fs.empty() || (fs != "vfat" && fs != "exfat" && fs != "ext4")) {
            resp.status_code = 503;
            resp.body = json{
                {"error", "Unsupported filesystem"},
                {"detail", "SD card must be formatted with FAT32, exFAT, or ext4"},
                {"current_filesystem", fs.empty() ? "unknown" : fs},
                {"recommendation", "Format the SD card using POST /api/v1/sdcard/format with filesystem=exfat"}
            }.dump();
            return resp;
        }
        
        // Final storage availability check
        if (!service.IsStorageAvailable()) {
            resp.status_code = 503;
            resp.body = json{{"error", "Storage not available"}, {"detail", "SD card storage check failed"}}.dump();
            return resp;
        }
        
        if (service.StartRecording(channel, type)) {
            auto session = service.GetSession(channel);
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "Recording started"},
                {"session", SessionToJson(session)}
            }.dump();
        } else {
            resp.status_code = 500;
            resp.body = json{{"error", "Failed to start recording"}}.dump();
        }
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json{{"error", "Invalid JSON"}, {"detail", e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingStart: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingStop(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto body = json::parse(ctx.body);
        int channel = body.value("channel", 0);
        
        if (service.StopRecording(channel)) {
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "Recording stopped"},
                {"channel", channel}
            }.dump();
        } else {
            resp.status_code = 500;
            resp.body = json{{"error", "Failed to stop recording"}}.dump();
        }
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json{{"error", "Invalid JSON"}, {"detail", e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingStop: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

// ============================================================================
// Recording Management
// ============================================================================

api::Response HandleRecordingsList(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        // Parse query parameters
        int channel = GetQueryParamInt(ctx, "channel", -1);
        int64_t start_time = GetQueryParamInt64(ctx, "start_time", 0);
        int64_t end_time = GetQueryParamInt64(ctx, "end_time", 0);
        std::string type_str = GetQueryParam(ctx, "type");
        int offset = GetQueryParamInt(ctx, "offset", 0);
        int limit = GetQueryParamInt(ctx, "limit", 100);
        
        std::optional<recording::RecordingType> type;
        if (!type_str.empty()) {
            type = recording::StringToRecordingType(type_str);
        }
        
        auto recordings = service.ListRecordings(channel, start_time, end_time, type, offset, limit);
        
        json j;
        j["recordings"] = json::array();
        for (const auto& rec : recordings) {
            j["recordings"].push_back(RecordingInfoToJson(rec));
        }
        j["count"] = static_cast<int>(recordings.size());
        j["offset"] = offset;
        j["limit"] = limit;
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingsList: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingGet(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        // Extract ID from URI: /api/v1/recordings/{id}
        std::string id = ExtractPathParam(ctx.uri, 3);  // api, v1, recordings, {id}
        
        if (id.empty()) {
            resp.status_code = 400;
            resp.body = json{{"error", "Recording ID required"}}.dump();
            return resp;
        }
        
        auto rec = service.GetRecording(id);
        if (rec) {
            resp.status_code = 200;
            resp.body = RecordingInfoToJson(*rec).dump(2);
        } else {
            resp.status_code = 404;
            resp.body = json{{"error", "Recording not found"}, {"id", id}}.dump();
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingGet: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingDelete(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "DELETE") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        // Extract ID from URI: /api/v1/recordings/{id}
        std::string id = ExtractPathParam(ctx.uri, 3);
        
        if (id.empty()) {
            resp.status_code = 400;
            resp.body = json{{"error", "Recording ID required"}}.dump();
            return resp;
        }
        
        if (service.DeleteRecording(id)) {
            resp.status_code = 200;
            resp.body = json{{"success", true}, {"message", "Recording deleted"}, {"id", id}}.dump();
        } else {
            resp.status_code = 404;
            resp.body = json{{"error", "Recording not found or delete failed"}, {"id", id}}.dump();
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingDelete: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingsDelete(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "DELETE" && ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto body = json::parse(ctx.body);
        int deleted = 0;
        
        if (body.contains("ids") && body["ids"].is_array()) {
            std::vector<std::string> ids;
            for (const auto& id : body["ids"]) {
                ids.push_back(id.get<std::string>());
            }
            deleted = service.DeleteRecordings(ids);
        } else if (body.contains("before")) {
            int64_t before = body["before"].get<int64_t>();
            deleted = service.DeleteRecordingsBefore(before);
        } else {
            resp.status_code = 400;
            resp.body = json{{"error", "Must specify 'ids' array or 'before' timestamp"}}.dump();
            return resp;
        }
        
        resp.status_code = 200;
        resp.body = json{{"success", true}, {"deleted", deleted}}.dump();
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json{{"error", "Invalid JSON"}, {"detail", e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingsDelete: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingStream(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        // Extract ID from URI: /api/v1/recordings/{id}/stream
        std::string id = ExtractPathParam(ctx.uri, 3);
        
        if (id.empty()) {
            resp.status_code = 400;
            resp.body = json{{"error", "Recording ID required"}}.dump();
            return resp;
        }
        
        auto rec = service.GetRecording(id);
        if (!rec) {
            resp.status_code = 404;
            resp.body = json{{"error", "Recording not found"}, {"id", id}}.dump();
            return resp;
        }
        
        // For now, return info about the recording and how to stream it
        // A full implementation would handle Range requests and stream decrypted content
        
        // TODO: Implement actual streaming with decryption
        // This requires integration with mongoose or the HTTP server's sendfile capabilities
        
        json j;
        j["id"] = id;
        j["filename"] = rec->filename;
        j["file_size"] = rec->file_size;
        j["encrypted"] = rec->encrypted;
        j["content_type"] = "video/mp4";
        
        // Provide stream URL for external players
        j["stream_url"] = "/api/v1/recordings/" + id + "/stream";
        
        resp.status_code = 200;
        resp.headers["Content-Type"] = "application/json";
        resp.body = j.dump(2);
        
        // In full implementation:
        // - Check Range header for partial content requests
        // - Open file with service.OpenForPlayback(id)
        // - Read decrypted chunks with service.ReadDecrypted()
        // - Stream to client with proper Content-Range headers
        
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingStream: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

// ============================================================================
// Storage Management
// ============================================================================

api::Response HandleRecordingStorage(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        auto stats = service.GetStorageStats();
        
        json j = StorageStatsToJson(stats);
        j["needs_formatting"] = service.NeedsFormatting();
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingStorage: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingStorageFormat(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto body = json::parse(ctx.body);
        std::string filesystem = body.value("filesystem", "exfat");
        
        // Validate filesystem
        if (filesystem != "fat32" && filesystem != "exfat") {
            resp.status_code = 400;
            resp.body = json{{"error", "Invalid filesystem"}, {"valid", json::array({"fat32", "exfat"})}}.dump();
            return resp;
        }
        
        // This is a dangerous operation - require confirmation
        if (!body.value("confirm", false)) {
            resp.status_code = 400;
            resp.body = json{
                {"error", "Confirmation required"},
                {"message", "This will erase ALL data on the SD card"},
                {"hint", "Set 'confirm': true to proceed"}
            }.dump();
            return resp;
        }
        
        spdlog::warn("Formatting SD card as {} (user confirmed)", filesystem);
        
        if (service.FormatStorage(filesystem)) {
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "SD card formatted successfully"},
                {"filesystem", filesystem}
            }.dump();
        } else {
            resp.status_code = 500;
            resp.body = json{{"error", "Format failed"}}.dump();
        }
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json{{"error", "Invalid JSON"}, {"detail", e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingStorageFormat: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

// ============================================================================
// Configuration
// ============================================================================

api::Response HandleRecordingConfig(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        if (ctx.method == "GET") {
            auto config = service.GetConfig();
            
            json j;
            j["sdcard_path"] = config.sdcard_path;
            j["recording_base_dir"] = config.recording_base_dir;
            j["encryption_enabled"] = config.encryption_enabled;
            j["encryption_algorithm"] = config.encryption_algorithm;
            j["fifo_enabled"] = config.fifo_enabled;
            j["free_space_min_mb"] = config.free_space_min_mb;
            j["cleanup_threshold_percent"] = config.cleanup_threshold_percent;
            j["default_type"] = recording::RecordingTypeToString(config.default_type);
            j["pre_record_enabled"] = config.pre_record_enabled;
            j["pre_record_seconds"] = config.pre_record_seconds;
            j["post_record_enabled"] = config.post_record_enabled;
            j["post_record_seconds"] = config.post_record_seconds;
            
            j["channels"] = json::array();
            for (const auto& ch : config.channels) {
                j["channels"].push_back(ChannelConfigToJson(ch));
            }
            
            resp.status_code = 200;
            resp.body = j.dump(2);
        } else if (ctx.method == "PUT" || ctx.method == "POST") {
            // Delegate to HandleRecordingConfigUpdate for PUT/POST
            return HandleRecordingConfigUpdate(ctx);
        } else {
            resp.status_code = 405;
            resp.body = json{{"error", "Method not allowed"}}.dump();
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingConfig: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingConfigUpdate(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "PUT" && ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto body = json::parse(ctx.body);
        auto config = service.GetConfig();
        
        // Save old channel enabled states to detect changes
        std::vector<bool> old_enabled_states;
        for (const auto& ch : config.channels) {
            old_enabled_states.push_back(ch.enabled);
        }
        
        // Update only provided fields
        if (body.contains("encryption_enabled")) config.encryption_enabled = body["encryption_enabled"].get<bool>();
        if (body.contains("fifo_enabled")) config.fifo_enabled = body["fifo_enabled"].get<bool>();
        if (body.contains("free_space_min_mb")) config.free_space_min_mb = body["free_space_min_mb"].get<int64_t>();
        if (body.contains("cleanup_threshold_percent")) config.cleanup_threshold_percent = body["cleanup_threshold_percent"].get<int>();
        if (body.contains("default_type")) config.default_type = recording::StringToRecordingType(body["default_type"].get<std::string>());
        if (body.contains("pre_record_enabled")) config.pre_record_enabled = body["pre_record_enabled"].get<bool>();
        if (body.contains("pre_record_seconds")) config.pre_record_seconds = body["pre_record_seconds"].get<int>();
        if (body.contains("post_record_enabled")) config.post_record_enabled = body["post_record_enabled"].get<bool>();
        if (body.contains("post_record_seconds")) config.post_record_seconds = body["post_record_seconds"].get<int>();
        
        // Update channel configs
        if (body.contains("channels") && body["channels"].is_array()) {
            for (const auto& ch_json : body["channels"]) {
                if (ch_json.contains("id")) {
                    int ch_id = ch_json["id"].get<int>();
                    if (ch_id >= 0 && ch_id < static_cast<int>(config.channels.size())) {
                        config.channels[ch_id] = ParseChannelConfig(ch_json, config.channels[ch_id]);
                    }
                }
            }
        }
        
        if (service.SetConfig(config)) {
            // After saving config, auto-start/stop recording based on enabled flag changes
            std::vector<std::string> actions_taken;
            
            for (size_t ch_id = 0; ch_id < config.channels.size() && ch_id < old_enabled_states.size(); ch_id++) {
                bool was_enabled = old_enabled_states[ch_id];
                bool now_enabled = config.channels[ch_id].enabled;
                
                if (!was_enabled && now_enabled) {
                    // Channel was just enabled - start recording
                    spdlog::info("Channel {} enabled via config - starting recording", ch_id);
                    
                    // Check if storage is available first
                    if (service.IsStorageAvailable()) {
                        recording::RecordingType type = config.default_type;
                        if (service.StartRecording(static_cast<int>(ch_id), type)) {
                            actions_taken.push_back("Started recording on channel " + std::to_string(ch_id));
                        } else {
                            actions_taken.push_back("Failed to start recording on channel " + std::to_string(ch_id));
                        }
                    } else {
                        actions_taken.push_back("Channel " + std::to_string(ch_id) + " enabled but storage not available");
                    }
                } else if (was_enabled && !now_enabled) {
                    // Channel was just disabled - stop recording
                    spdlog::info("Channel {} disabled via config - stopping recording", ch_id);
                    
                    if (service.StopRecording(static_cast<int>(ch_id))) {
                        actions_taken.push_back("Stopped recording on channel " + std::to_string(ch_id));
                    }
                }
            }
            
            json response = {
                {"success", true},
                {"message", "Configuration updated"}
            };
            
            if (!actions_taken.empty()) {
                response["actions"] = actions_taken;
            }
            
            resp.status_code = 200;
            resp.body = response.dump();
        } else {
            resp.status_code = 500;
            resp.body = json{{"error", "Failed to save configuration"}}.dump();
        }
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json{{"error", "Invalid JSON"}, {"detail", e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingConfigUpdate: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingChannel(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        // Extract channel ID from URI: /api/v1/recording/channels/{id}
        std::string id_str = ExtractPathParam(ctx.uri, 4);  // api, v1, recording, channels, {id}
        int channel = std::stoi(id_str);
        
        if (channel < 0 || channel >= 3) {
            resp.status_code = 400;
            resp.body = json{{"error", "Invalid channel ID"}, {"valid_range", "0-2"}}.dump();
            return resp;
        }
        
        if (ctx.method == "GET") {
            auto config = service.GetChannelConfig(channel);
            resp.status_code = 200;
            resp.body = ChannelConfigToJson(config).dump(2);
        } else {
            resp.status_code = 405;
            resp.body = json{{"error", "Method not allowed"}}.dump();
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingChannel: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleRecordingChannelUpdate(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "PUT" && ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        // Extract channel ID from URI
        std::string id_str = ExtractPathParam(ctx.uri, 4);
        int channel = std::stoi(id_str);
        
        if (channel < 0 || channel >= 3) {
            resp.status_code = 400;
            resp.body = json{{"error", "Invalid channel ID"}, {"valid_range", "0-2"}}.dump();
            return resp;
        }
        
        auto body = json::parse(ctx.body);
        auto current = service.GetChannelConfig(channel);
        bool was_enabled = current.enabled;
        
        auto updated = ParseChannelConfig(body, current);
        
        if (service.SetChannelConfig(channel, updated)) {
            json response = {
                {"success", true},
                {"message", "Channel configuration updated"},
                {"channel", ChannelConfigToJson(updated)}
            };
            
            // Auto-start/stop recording based on enabled flag change
            if (!was_enabled && updated.enabled) {
                // Channel was just enabled - start recording
                spdlog::info("Channel {} enabled via channel config - starting recording", channel);
                
                if (service.IsStorageAvailable()) {
                    auto global_config = service.GetConfig();
                    recording::RecordingType type = global_config.default_type;
                    if (service.StartRecording(channel, type)) {
                        response["action"] = "Started recording on channel " + std::to_string(channel);
                    } else {
                        response["action"] = "Failed to start recording on channel " + std::to_string(channel);
                    }
                } else {
                    response["action"] = "Channel enabled but storage not available";
                }
            } else if (was_enabled && !updated.enabled) {
                // Channel was just disabled - stop recording
                spdlog::info("Channel {} disabled via channel config - stopping recording", channel);
                
                if (service.StopRecording(channel)) {
                    response["action"] = "Stopped recording on channel " + std::to_string(channel);
                }
            }
            
            resp.status_code = 200;
            resp.body = response.dump();
        } else {
            resp.status_code = 500;
            resp.body = json{{"error", "Failed to update channel configuration"}}.dump();
        }
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json{{"error", "Invalid JSON"}, {"detail", e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRecordingChannelUpdate: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

// ============================================================================
// SD Card Management
// ============================================================================

api::Response HandleSdCardStatus(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        json j;
        j["inserted"] = service.IsSdCardInserted();
        j["mounted"] = service.IsSdCardMounted();
        j["device"] = service.GetSdCardDevice();
        j["filesystem"] = service.GetSdCardFilesystem();
        j["mount_point"] = "/mnt/sd";
        
        // Determine overall status
        if (!service.IsSdCardInserted()) {
            j["status"] = "not_inserted";
            j["message"] = "No SD card detected";
            j["recording_ready"] = false;
        } else if (!service.IsSdCardMounted()) {
            j["status"] = "not_mounted";
            j["message"] = "SD card detected but not mounted";
            j["recording_ready"] = false;
        } else {
            std::string fs = service.GetSdCardFilesystem();
            if (fs.empty() || (fs != "vfat" && fs != "exfat" && fs != "ext4")) {
                j["status"] = "unsupported_filesystem";
                j["message"] = "SD card has unsupported filesystem: " + (fs.empty() ? "unknown" : fs);
                j["recording_ready"] = false;
            } else {
                j["status"] = "ready";
                j["message"] = "SD card is ready for recording";
                j["recording_ready"] = true;
                
                // Add storage stats if available
                auto stats = service.GetStorageStats();
                j["storage"] = StorageStatsToJson(stats);
            }
        }
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleSdCardStatus: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleSdCardMount(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        if (!service.IsSdCardInserted()) {
            resp.status_code = 400;
            resp.body = json{
                {"error", "No SD card detected"},
                {"detail", "Insert an SD card before attempting to mount"}
            }.dump();
            return resp;
        }
        
        if (service.IsSdCardMounted()) {
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "SD card is already mounted"},
                {"mount_point", "/mnt/sd"}
            }.dump();
            return resp;
        }
        
        if (service.MountSdCard()) {
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "SD card mounted successfully"},
                {"mount_point", "/mnt/sd"},
                {"filesystem", service.GetSdCardFilesystem()}
            }.dump();
        } else {
            resp.status_code = 500;
            resp.body = json{
                {"error", "Failed to mount SD card"},
                {"detail", "The SD card may not be formatted or has an unsupported filesystem"}
            }.dump();
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleSdCardMount: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleSdCardUnmount(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        if (!service.IsSdCardMounted()) {
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "SD card is not mounted"}
            }.dump();
            return resp;
        }
        
        // Check if recording is active - don't unmount while recording
        auto sessions = service.GetActiveSessions();
        if (!sessions.empty()) {
            resp.status_code = 409;
            resp.body = json{
                {"error", "Cannot unmount SD card"},
                {"detail", "Recording is in progress. Stop all recordings before unmounting."},
                {"active_sessions", static_cast<int>(sessions.size())}
            }.dump();
            return resp;
        }
        
        if (service.UnmountSdCard()) {
            resp.status_code = 200;
            resp.body = json{
                {"success", true},
                {"message", "SD card unmounted successfully"}
            }.dump();
        } else {
            resp.status_code = 500;
            resp.body = json{
                {"error", "Failed to unmount SD card"},
                {"detail", "The SD card may be in use by another process"}
            }.dump();
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleSdCardUnmount: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandleSdCardFormat(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto body = json::parse(ctx.body.empty() ? "{}" : ctx.body);
        
        // Get filesystem type from request, default to exfat for >4GB support
        std::string filesystem = body.value("filesystem", "exfat");
        std::string label = body.value("label", "IPCAMERA");
        bool force = body.value("force", false);
        
        // Validate filesystem type
        if (filesystem != "vfat" && filesystem != "exfat" && filesystem != "ext4") {
            resp.status_code = 400;
            resp.body = json{
                {"error", "Unsupported filesystem type"},
                {"detail", "Supported types: vfat (FAT32), exfat, ext4"},
                {"recommended", "exfat"}
            }.dump();
            return resp;
        }
        
        if (!service.IsSdCardInserted()) {
            resp.status_code = 400;
            resp.body = json{
                {"error", "No SD card detected"},
                {"detail", "Insert an SD card before formatting"}
            }.dump();
            return resp;
        }
        
        // Check if recording is active
        auto sessions = service.GetActiveSessions();
        if (!sessions.empty()) {
            resp.status_code = 409;
            resp.body = json{
                {"error", "Cannot format SD card"},
                {"detail", "Recording is in progress. Stop all recordings before formatting."},
                {"active_sessions", static_cast<int>(sessions.size())}
            }.dump();
            return resp;
        }
        
        // Require force flag to confirm destructive operation
        if (!force) {
            resp.status_code = 400;
            resp.body = json{
                {"error", "Confirmation required"},
                {"detail", "Formatting will erase all data on the SD card. Set force=true to confirm."},
                {"warning", "All recordings will be permanently deleted"}
            }.dump();
            return resp;
        }
        
        std::string device = service.GetSdCardDevice();
        if (device.empty()) {
            resp.status_code = 500;
            resp.body = json{{"error", "Could not determine SD card device"}}.dump();
            return resp;
        }
        
        // Unmount if mounted
        if (service.IsSdCardMounted()) {
            if (!service.UnmountSdCard()) {
                resp.status_code = 500;
                resp.body = json{
                    {"error", "Failed to unmount SD card"},
                    {"detail", "Cannot format while mounted"}
                }.dump();
                return resp;
            }
        }
        
        // Get the base device (without partition number) for partitioning
        std::string base_device = device;
        if (device.find("p1") != std::string::npos) {
            base_device = device.substr(0, device.length() - 2);  // Remove "p1"
        }
        
        spdlog::info("Formatting SD card: device={}, base={}, filesystem={}, label={}", 
                     device, base_device, filesystem, label);
        
        // Format the SD card
        std::string mkfs_cmd;
        if (filesystem == "vfat") {
            mkfs_cmd = "mkfs.vfat -F 32 -n " + label + " " + device;
        } else if (filesystem == "exfat") {
            mkfs_cmd = "mkfs.exfat -n " + label + " " + device;
        } else if (filesystem == "ext4") {
            mkfs_cmd = "mkfs.ext4 -L " + label + " -F " + device;
        }
        
        spdlog::info("Executing format command: {}", mkfs_cmd);
        int result = system(mkfs_cmd.c_str());
        
        if (result != 0) {
            spdlog::error("Format command failed with exit code: {}", result);
            resp.status_code = 500;
            resp.body = json{
                {"error", "Format failed"},
                {"detail", "mkfs command returned error code " + std::to_string(result)},
                {"command", mkfs_cmd}
            }.dump();
            return resp;
        }
        
        // Remount the SD card
        bool mounted = service.MountSdCard();
        
        resp.status_code = 200;
        resp.body = json{
            {"success", true},
            {"message", "SD card formatted successfully"},
            {"filesystem", filesystem},
            {"label", label},
            {"device", device},
            {"mounted", mounted},
            {"mount_point", mounted ? "/mnt/sd" : ""}
        }.dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json{{"error", "Invalid JSON"}, {"detail", e.what()}}.dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in HandleSdCardFormat: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
