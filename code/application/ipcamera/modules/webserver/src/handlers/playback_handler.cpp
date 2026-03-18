/**
 * @file playback_handler.cpp
 * @brief Playback API HTTP handler implementations
 */

#include "playback_handler.h"
#include <ipcam/recording.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <set>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// Helper Functions
// ============================================================================

/// Parse query parameter from ctx.query_params (populated by ApiHandler)
static std::string GetQueryParam(const api::RequestContext& ctx, const std::string& name, const std::string& default_value = "") {
    auto it = ctx.query_params.find(name);
    if (it != ctx.query_params.end()) {
        return it->second;
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

/// Extract path parameter (e.g., /api/v1/playback/recordings/{id})
static std::string ExtractPathParam(const std::string& uri, int position) {
    // Remove query string
    std::string path = uri;
    size_t qpos = path.find('?');
    if (qpos != std::string::npos) {
        path = path.substr(0, qpos);
    }
    
    std::vector<std::string> parts;
    std::istringstream iss(path);
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

/// Parse date string YYYY-MM-DD to Unix timestamp (start of day, local time)
/// Handles pre-epoch dates (before 1970) that mktime can't handle
static int64_t ParseDateToTimestamp(const std::string& date_str, bool end_of_day = false) {
    if (date_str.length() != 10 || date_str[4] != '-' || date_str[7] != '-') {
        return 0;
    }
    
    try {
        int year = std::stoi(date_str.substr(0, 4));
        int month = std::stoi(date_str.substr(5, 2));
        int day = std::stoi(date_str.substr(8, 2));
        
        // Validate ranges
        if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
            return 0;
        }
        
        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = end_of_day ? 23 : 0;
        tm.tm_min = end_of_day ? 59 : 0;
        tm.tm_sec = end_of_day ? 59 : 0;
        tm.tm_isdst = -1;  // Let system determine DST
        
        time_t t = mktime(&tm);
        if (t == -1) {
            // Handle pre-epoch dates (1970 and earlier)
            // Calculate approximate timestamp using simple formula
            int64_t days_since_epoch = (year - 1970) * 365 + (year - 1969) / 4;  // Leap years approximation
            days_since_epoch += (month - 1) * 30 + day - 1;  // Approximate month days
            int hour = end_of_day ? 23 : 0;
            int min = end_of_day ? 59 : 0;
            int sec = end_of_day ? 59 : 0;
            int64_t secs = days_since_epoch * 86400 + hour * 3600 + min * 60 + sec;
            return secs * 1000 + (end_of_day ? 999 : 0);
        }
        
        // Convert to milliseconds
        return static_cast<int64_t>(t) * 1000 + (end_of_day ? 999 : 0);
    } catch (...) {
        return 0;
    }
}

/// Parse time string HH:MM to seconds since midnight
static int ParseTimeToSeconds(const std::string& time_str) {
    if (time_str.length() < 5 || time_str[2] != ':') {
        return -1;
    }
    
    try {
        int hour = std::stoi(time_str.substr(0, 2));
        int minute = std::stoi(time_str.substr(3, 2));
        
        if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
            return -1;
        }
        
        return hour * 3600 + minute * 60;
    } catch (...) {
        return -1;
    }
}

/// Convert Unix timestamp (ms) to date string YYYY-MM-DD
static std::string TimestampToDateStr(int64_t timestamp_ms) {
    time_t t = timestamp_ms / 1000;
    std::tm* tm = std::localtime(&t);
    if (!tm) return "";
    
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return std::string(buf);
}

/// Convert Unix timestamp (ms) to time string HH:MM:SS
static std::string TimestampToTimeStr(int64_t timestamp_ms) {
    time_t t = timestamp_ms / 1000;
    std::tm* tm = std::localtime(&t);
    if (!tm) return "";
    
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", tm);
    return std::string(buf);
}

/// Convert Unix timestamp (ms) to ISO8601 string
static std::string TimestampToIso8601(int64_t timestamp_ms) {
    time_t t = timestamp_ms / 1000;
    std::tm* tm = std::gmtime(&t);
    if (!tm) return "";
    
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", tm);
    return std::string(buf);
}

/// Build JSON from RecordingInfo with playback URLs
static json RecordingInfoToJsonWithUrls(const recording::RecordingInfo& info) {
    json j;
    j["id"] = info.id;
    j["filename"] = info.filename;
    j["channel"] = info.channel;
    j["type"] = recording::RecordingTypeToString(info.type);
    j["start_time"] = info.start_time;
    j["end_time"] = info.end_time;
    j["start_time_iso"] = TimestampToIso8601(info.start_time);
    j["end_time_iso"] = TimestampToIso8601(info.end_time);
    j["duration_ms"] = info.duration_ms;
    j["duration_sec"] = info.duration_ms / 1000;
    j["file_size"] = info.file_size;
    j["file_size_human"] = recording::FormatBytes(info.file_size);
    j["width"] = info.width;
    j["height"] = info.height;
    j["fps"] = info.fps;
    j["codec"] = info.codec;
    j["encrypted"] = info.encrypted;
    j["status"] = info.status;
    
    // Add playback URLs
    j["playback_url"] = "/api/v1/playback/stream/" + info.id;
    j["download_url"] = "/api/v1/playback/download/" + info.id;
    j["thumbnail_url"] = "/api/v1/playback/thumbnail/" + info.id;
    
    if (!info.thumbnail.empty()) {
        j["thumbnail_path"] = info.thumbnail;
    }
    
    return j;
}

// ============================================================================
// Playback Search and Discovery
// ============================================================================

api::Response HandlePlaybackSearch(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        // Parse query parameters (accept both 'date' and 'start_date' for compatibility)
        std::string start_date = GetQueryParam(ctx, "start_date");
        if (start_date.empty()) {
            start_date = GetQueryParam(ctx, "date");  // Fallback to 'date' parameter
        }
        std::string end_date = GetQueryParam(ctx, "end_date", start_date);
        std::string start_time = GetQueryParam(ctx, "start_time", "00:00");
        std::string end_time = GetQueryParam(ctx, "end_time", "23:59");
        int channel = GetQueryParamInt(ctx, "channel", -1);
        std::string type_str = GetQueryParam(ctx, "type");
        int limit = GetQueryParamInt(ctx, "limit", 100);
        int offset = GetQueryParamInt(ctx, "offset", 0);
        std::string sort = GetQueryParam(ctx, "sort", "newest");
        
        // Validate required parameters
        if (start_date.empty()) {
            resp.status_code = 400;
            resp.body = json{{"error", "start_date is required (format: YYYY-MM-DD)"}}.dump();
            return resp;
        }
        
        // Parse dates
        int64_t start_ts = ParseDateToTimestamp(start_date);
        int64_t end_ts = ParseDateToTimestamp(end_date, true);
        
        if (start_ts == 0 || end_ts == 0) {
            resp.status_code = 400;
            resp.body = json{{"error", "Invalid date format. Use YYYY-MM-DD"}}.dump();
            return resp;
        }
        
        // Adjust for time of day
        int start_sec = ParseTimeToSeconds(start_time);
        int end_sec = ParseTimeToSeconds(end_time);
        if (start_sec >= 0) {
            start_ts += start_sec * 1000;
        }
        if (end_sec >= 0) {
            // Reset to start of day then add end time
            end_ts = ParseDateToTimestamp(end_date) + end_sec * 1000 + 59999;
        }
        
        // Parse type filter
        std::optional<recording::RecordingType> type;
        if (!type_str.empty()) {
            type = recording::StringToRecordingType(type_str);
        }
        
        // Get all matching recordings (we'll sort and paginate ourselves)
        auto recordings = service.ListRecordings(channel, start_ts, end_ts, type, 0, 0);
        
        // Sort
        if (sort == "oldest") {
            std::sort(recordings.begin(), recordings.end(),
                [](const recording::RecordingInfo& a, const recording::RecordingInfo& b) {
                    return a.start_time < b.start_time;
                });
        } else {
            // Default: newest first
            std::sort(recordings.begin(), recordings.end(),
                [](const recording::RecordingInfo& a, const recording::RecordingInfo& b) {
                    return a.start_time > b.start_time;
                });
        }
        
        // Get total count before pagination
        int total_count = static_cast<int>(recordings.size());
        
        // Apply pagination
        std::vector<recording::RecordingInfo> page_recordings;
        for (int i = offset; i < std::min(offset + limit, total_count); i++) {
            page_recordings.push_back(recordings[i]);
        }
        
        // Build response
        json j;
        j["recordings"] = json::array();
        for (const auto& rec : page_recordings) {
            j["recordings"].push_back(RecordingInfoToJsonWithUrls(rec));
        }
        j["total_count"] = total_count;
        j["returned_count"] = static_cast<int>(page_recordings.size());
        j["offset"] = offset;
        j["limit"] = limit;
        j["has_more"] = (offset + limit) < total_count;
        j["query"] = {
            {"start_date", start_date},
            {"end_date", end_date},
            {"start_time", start_time},
            {"end_time", end_time},
            {"channel", channel},
            {"type", type_str},
            {"sort", sort}
        };
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackSearch: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandlePlaybackCalendar(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        int year = GetQueryParamInt(ctx, "year", 0);
        int month = GetQueryParamInt(ctx, "month", 0);
        int channel = GetQueryParamInt(ctx, "channel", -1);
        
        if (year == 0 || month == 0) {
            resp.status_code = 400;
            resp.body = json{{"error", "year and month are required"}}.dump();
            return resp;
        }
        
        if (month < 1 || month > 12) {
            resp.status_code = 400;
            resp.body = json{{"error", "month must be 1-12"}}.dump();
            return resp;
        }
        
        // Calculate start and end of month
        std::tm tm_start = {};
        tm_start.tm_year = year - 1900;
        tm_start.tm_mon = month - 1;
        tm_start.tm_mday = 1;
        tm_start.tm_isdst = -1;
        time_t start_t = mktime(&tm_start);
        
        std::tm tm_end = {};
        tm_end.tm_year = year - 1900;
        tm_end.tm_mon = month;  // Next month
        tm_end.tm_mday = 1;
        tm_end.tm_isdst = -1;
        time_t end_t = mktime(&tm_end);
        
        int64_t start_ms = static_cast<int64_t>(start_t) * 1000;
        int64_t end_ms = static_cast<int64_t>(end_t) * 1000 - 1;
        
        // Get all recordings for the month
        auto recordings = service.ListRecordings(channel, start_ms, end_ms, std::nullopt, 0, 0);
        
        // Group by day
        std::map<int, std::vector<recording::RecordingInfo>> by_day;
        for (const auto& rec : recordings) {
            time_t rec_t = rec.start_time / 1000;
            std::tm* rec_tm = std::localtime(&rec_t);
            if (rec_tm) {
                int day = rec_tm->tm_mday;
                by_day[day].push_back(rec);
            }
        }
        
        // Build response
        json j;
        j["year"] = year;
        j["month"] = month;
        j["days"] = json::object();
        
        for (const auto& [day, recs] : by_day) {
            int64_t total_duration = 0;
            std::set<std::string> types;
            
            for (const auto& rec : recs) {
                total_duration += rec.duration_ms;
                types.insert(recording::RecordingTypeToString(rec.type));
            }
            
            json day_info;
            day_info["count"] = static_cast<int>(recs.size());
            day_info["total_duration_sec"] = total_duration / 1000;
            day_info["types"] = json::array();
            for (const auto& t : types) {
                day_info["types"].push_back(t);
            }
            
            j["days"][std::to_string(day)] = day_info;
        }
        
        j["total_recordings"] = static_cast<int>(recordings.size());
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackCalendar: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandlePlaybackTimeline(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        std::string date = GetQueryParam(ctx, "date");
        int channel = GetQueryParamInt(ctx, "channel", -1);
        
        if (date.empty()) {
            resp.status_code = 400;
            resp.body = json{{"error", "date is required (format: YYYY-MM-DD)"}}.dump();
            return resp;
        }
        
        int64_t start_ts = ParseDateToTimestamp(date);
        int64_t end_ts = ParseDateToTimestamp(date, true);
        
        if (start_ts == 0 || end_ts == 0) {
            resp.status_code = 400;
            resp.body = json{{"error", "Invalid date format. Use YYYY-MM-DD"}}.dump();
            return resp;
        }
        
        // Get all recordings for the day
        auto recordings = service.ListRecordings(channel, start_ts, end_ts, std::nullopt, 0, 0);
        
        // Sort by start time
        std::sort(recordings.begin(), recordings.end(),
            [](const recording::RecordingInfo& a, const recording::RecordingInfo& b) {
                return a.start_time < b.start_time;
            });
        
        // Build segments and find gaps
        json segments = json::array();
        json gaps = json::array();
        int64_t total_recorded = 0;
        int64_t last_end = start_ts;
        
        for (const auto& rec : recordings) {
            // Check for gap before this recording
            if (rec.start_time > last_end + 1000) {  // More than 1 second gap
                gaps.push_back({
                    {"start", TimestampToTimeStr(last_end)},
                    {"end", TimestampToTimeStr(rec.start_time)},
                    {"duration_sec", (rec.start_time - last_end) / 1000}
                });
            }
            
            segments.push_back({
                {"start", TimestampToTimeStr(rec.start_time)},
                {"end", TimestampToTimeStr(rec.end_time)},
                {"start_timestamp", rec.start_time},
                {"end_timestamp", rec.end_time},
                {"type", recording::RecordingTypeToString(rec.type)},
                {"recording_id", rec.id},
                {"duration_sec", rec.duration_ms / 1000},
                {"channel", rec.channel}
            });
            
            total_recorded += rec.duration_ms;
            if (rec.end_time > last_end) {
                last_end = rec.end_time;
            }
        }
        
        // Check for gap at end of day
        if (last_end < end_ts - 1000) {
            gaps.push_back({
                {"start", TimestampToTimeStr(last_end)},
                {"end", "23:59:59"},
                {"duration_sec", (end_ts - last_end) / 1000}
            });
        }
        
        // Calculate coverage
        int64_t day_duration = 24 * 60 * 60 * 1000;  // 24 hours in ms
        double coverage = (total_recorded * 100.0) / day_duration;
        
        json j;
        j["date"] = date;
        j["segments"] = segments;
        j["gaps"] = gaps;
        j["total_recorded_sec"] = total_recorded / 1000;
        j["total_recordings"] = static_cast<int>(recordings.size());
        j["coverage_percent"] = std::round(coverage * 10) / 10;  // 1 decimal place
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackTimeline: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandlePlaybackDates(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        std::string start_date = GetQueryParam(ctx, "start_date");
        std::string end_date = GetQueryParam(ctx, "end_date");
        int channel = GetQueryParamInt(ctx, "channel", -1);
        
        if (start_date.empty() || end_date.empty()) {
            resp.status_code = 400;
            resp.body = json{{"error", "start_date and end_date are required (format: YYYY-MM-DD)"}}.dump();
            return resp;
        }
        
        int64_t start_ts = ParseDateToTimestamp(start_date);
        int64_t end_ts = ParseDateToTimestamp(end_date, true);
        
        if (start_ts == 0 || end_ts == 0) {
            resp.status_code = 400;
            resp.body = json{{"error", "Invalid date format. Use YYYY-MM-DD"}}.dump();
            return resp;
        }
        
        // Get all recordings in range
        auto recordings = service.ListRecordings(channel, start_ts, end_ts, std::nullopt, 0, 0);
        
        // Collect unique dates
        std::set<std::string> dates_set;
        for (const auto& rec : recordings) {
            std::string date = TimestampToDateStr(rec.start_time);
            if (!date.empty()) {
                dates_set.insert(date);
            }
        }
        
        // Convert to sorted array
        std::vector<std::string> dates(dates_set.begin(), dates_set.end());
        std::sort(dates.begin(), dates.end());
        
        json j;
        j["dates"] = dates;
        j["count"] = static_cast<int>(dates.size());
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackDates: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

// ============================================================================
// Recording Details and Streaming
// ============================================================================

api::Response HandlePlaybackRecordingInfo(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        // Extract ID from URI: /api/v1/playback/recordings/{id}
        std::string id = ExtractPathParam(ctx.uri, 4);  // api, v1, playback, recordings, {id}
        
        if (id.empty()) {
            resp.status_code = 400;
            resp.body = json{{"error", "Recording ID required"}}.dump();
            return resp;
        }
        
        auto rec = service.GetRecording(id);
        if (rec) {
            resp.status_code = 200;
            resp.body = RecordingInfoToJsonWithUrls(*rec).dump(2);
        } else {
            resp.status_code = 404;
            resp.body = json{{"error", "Recording not found"}, {"id", id}}.dump();
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackRecordingInfo: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandlePlaybackStream(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        // Extract ID from URI: /api/v1/playback/stream/{id}
        std::string id = ExtractPathParam(ctx.uri, 4);  // api, v1, playback, stream, {id}
        
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
        
        // Check if file exists
        struct stat st;
        if (stat(rec->filename.c_str(), &st) != 0) {
            resp.status_code = 404;
            resp.body = json{{"error", "Recording file not found"}, {"filename", rec->filename}}.dump();
            return resp;
        }
        
        if (rec->encrypted) {
            // For encrypted files, decrypt to a temp file and serve that
            // This avoids loading the entire file into memory
            std::string temp_file = "/tmp/playback_" + id + ".mp4";
            
            int fd = service.OpenForPlayback(id);
            if (fd < 0) {
                resp.status_code = 500;
                resp.body = json{{"error", "Failed to open encrypted recording for playback"}}.dump();
                return resp;
            }
            
            // Decrypt in chunks to temp file
            FILE* out_fp = fopen(temp_file.c_str(), "wb");
            if (!out_fp) {
                service.ClosePlayback(fd);
                resp.status_code = 500;
                resp.body = json{{"error", "Failed to create temp file for playback"}}.dump();
                return resp;
            }
            
            constexpr size_t CHUNK_SIZE = 65536;  // 64KB chunks
            std::vector<uint8_t> buffer(CHUNK_SIZE);
            size_t offset = 0;
            size_t total_written = 0;
            
            while (offset < static_cast<size_t>(st.st_size)) {
                size_t to_read = std::min(CHUNK_SIZE, static_cast<size_t>(st.st_size) - offset);
                ssize_t bytes_read = service.ReadDecrypted(fd, buffer.data(), to_read, offset);
                
                if (bytes_read <= 0) {
                    break;
                }
                
                size_t written = fwrite(buffer.data(), 1, bytes_read, out_fp);
                if (written != static_cast<size_t>(bytes_read)) {
                    spdlog::error("Failed to write to temp file");
                    break;
                }
                
                offset += bytes_read;
                total_written += written;
            }
            
            fclose(out_fp);
            service.ClosePlayback(fd);
            
            if (total_written == 0) {
                unlink(temp_file.c_str());
                resp.status_code = 500;
                resp.body = json{{"error", "Failed to decrypt recording"}}.dump();
                return resp;
            }
            
            // Use file streaming
            resp.status_code = 200;
            resp.content_type = "video/mp4";
            resp.headers["Accept-Ranges"] = "bytes";
            resp.headers["Cache-Control"] = "no-store";  // Don't cache decrypted content
            resp.file_path = temp_file;
            resp.file_size = total_written;
            
            // Note: temp file will remain after streaming - in production, 
            // consider cleanup strategy (e.g., cleanup old temp files on startup)
        } else {
            // For unencrypted files, use direct file streaming
            resp.status_code = 200;
            resp.content_type = "video/mp4";
            resp.headers["Accept-Ranges"] = "bytes";
            resp.file_path = rec->filename;
            resp.file_size = st.st_size;
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackStream: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandlePlaybackDownload(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        // Extract ID from URI: /api/v1/playback/download/{id}
        std::string id = ExtractPathParam(ctx.uri, 4);  // api, v1, playback, download, {id}
        
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
        
        // Check if file exists
        struct stat st;
        if (stat(rec->filename.c_str(), &st) != 0) {
            resp.status_code = 404;
            resp.body = json{{"error", "Recording file not found"}}.dump();
            return resp;
        }
        
        // Generate download filename
        std::string download_name = TimestampToDateStr(rec->start_time) + "_" + 
                                    TimestampToTimeStr(rec->start_time) + "_ch" + 
                                    std::to_string(rec->channel) + ".mp4";
        // Replace colons with dashes for filename safety
        std::replace(download_name.begin(), download_name.end(), ':', '-');
        
        if (rec->encrypted) {
            // For encrypted files, decrypt to a temp file and serve that
            std::string temp_file = "/tmp/download_" + id + ".mp4";
            
            int fd = service.OpenForPlayback(id);
            if (fd < 0) {
                resp.status_code = 500;
                resp.body = json{{"error", "Failed to open encrypted recording for download"}}.dump();
                return resp;
            }
            
            // Decrypt in chunks to temp file
            FILE* out_fp = fopen(temp_file.c_str(), "wb");
            if (!out_fp) {
                service.ClosePlayback(fd);
                resp.status_code = 500;
                resp.body = json{{"error", "Failed to create temp file for download"}}.dump();
                return resp;
            }
            
            constexpr size_t CHUNK_SIZE = 65536;  // 64KB chunks
            std::vector<uint8_t> buffer(CHUNK_SIZE);
            size_t offset = 0;
            size_t total_written = 0;
            
            while (offset < static_cast<size_t>(st.st_size)) {
                size_t to_read = std::min(CHUNK_SIZE, static_cast<size_t>(st.st_size) - offset);
                ssize_t bytes_read = service.ReadDecrypted(fd, buffer.data(), to_read, offset);
                
                if (bytes_read <= 0) {
                    break;
                }
                
                size_t written = fwrite(buffer.data(), 1, bytes_read, out_fp);
                if (written != static_cast<size_t>(bytes_read)) {
                    spdlog::error("Failed to write to temp file for download");
                    break;
                }
                
                offset += bytes_read;
                total_written += written;
            }
            
            fclose(out_fp);
            service.ClosePlayback(fd);
            
            if (total_written == 0) {
                unlink(temp_file.c_str());
                resp.status_code = 500;
                resp.body = json{{"error", "Failed to decrypt recording for download"}}.dump();
                return resp;
            }
            
            // Use file streaming
            resp.status_code = 200;
            resp.content_type = "application/octet-stream";
            resp.headers["Content-Disposition"] = "attachment; filename=\"" + download_name + "\"";
            resp.headers["Cache-Control"] = "no-store";
            resp.file_path = temp_file;
            resp.file_size = total_written;
        } else {
            // For unencrypted files, use direct file streaming
            resp.status_code = 200;
            resp.content_type = "application/octet-stream";
            resp.headers["Content-Disposition"] = "attachment; filename=\"" + download_name + "\"";
            resp.file_path = rec->filename;
            resp.file_size = st.st_size;
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackDownload: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandlePlaybackThumbnail(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        // Extract ID from URI: /api/v1/playback/thumbnail/{id}
        std::string id = ExtractPathParam(ctx.uri, 4);  // api, v1, playback, thumbnail, {id}
        
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
        
        // Check if thumbnail exists
        if (rec->thumbnail.empty()) {
            // No thumbnail available, return a placeholder response
            resp.status_code = 204;
            resp.body = "";
            return resp;
        }
        
        struct stat st;
        if (stat(rec->thumbnail.c_str(), &st) != 0) {
            resp.status_code = 204;
            resp.body = "";
            return resp;
        }
        
        // Serve thumbnail
        resp.status_code = 200;
        resp.headers["Content-Type"] = "image/jpeg";
        resp.headers["Content-Length"] = std::to_string(st.st_size);
        resp.headers["Cache-Control"] = "max-age=86400";  // Cache for 1 day
        resp.headers["X-Accel-Redirect"] = rec->thumbnail;  // For nginx
        
        resp.body = "";
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackThumbnail: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

// ============================================================================
// Quick Access
// ============================================================================

api::Response HandlePlaybackRecent(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        int limit = GetQueryParamInt(ctx, "limit", 20);
        int channel = GetQueryParamInt(ctx, "channel", -1);
        
        if (limit > 100) limit = 100;  // Cap at 100
        
        // Get recent recordings (no time filter, just limit)
        auto recordings = service.ListRecordings(channel, 0, 0, std::nullopt, 0, 0);
        
        // Sort by start time descending (newest first)
        std::sort(recordings.begin(), recordings.end(),
            [](const recording::RecordingInfo& a, const recording::RecordingInfo& b) {
                return a.start_time > b.start_time;
            });
        
        // Take top N
        if (static_cast<int>(recordings.size()) > limit) {
            recordings.resize(limit);
        }
        
        json j;
        j["recordings"] = json::array();
        for (const auto& rec : recordings) {
            j["recordings"].push_back(RecordingInfoToJsonWithUrls(rec));
        }
        j["count"] = static_cast<int>(recordings.size());
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackRecent: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

api::Response HandlePlaybackStats(const api::RequestContext& ctx) {
    api::Response resp;
    auto& service = recording::RecordingService::Instance();
    
    try {
        int days = GetQueryParamInt(ctx, "days", 30);
        if (days > 365) days = 365;  // Cap at 1 year
        
        // Calculate time range
        int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        int64_t start_ts = now - (static_cast<int64_t>(days) * 24 * 60 * 60 * 1000);
        
        // Get all recordings in range
        auto recordings = service.ListRecordings(-1, start_ts, now, std::nullopt, 0, 0);
        
        // Calculate statistics
        int64_t total_duration = 0;
        int64_t total_size = 0;
        std::map<std::string, std::pair<int, int64_t>> by_type;  // count, duration
        std::map<int, std::pair<int, int64_t>> by_channel;  // count, duration
        int64_t oldest_time = now;
        int64_t newest_time = 0;
        
        for (const auto& rec : recordings) {
            total_duration += rec.duration_ms;
            total_size += rec.file_size;
            
            std::string type_str = recording::RecordingTypeToString(rec.type);
            by_type[type_str].first++;
            by_type[type_str].second += rec.duration_ms;
            
            by_channel[rec.channel].first++;
            by_channel[rec.channel].second += rec.duration_ms;
            
            if (rec.start_time < oldest_time) oldest_time = rec.start_time;
            if (rec.end_time > newest_time) newest_time = rec.end_time;
        }
        
        json j;
        j["total_recordings"] = static_cast<int>(recordings.size());
        j["total_duration_hours"] = std::round((total_duration / 3600000.0) * 10) / 10;
        j["total_size_gb"] = std::round((total_size / (1024.0 * 1024 * 1024)) * 100) / 100;
        j["days_analyzed"] = days;
        
        j["by_type"] = json::object();
        for (const auto& [type, stats] : by_type) {
            j["by_type"][type] = {
                {"count", stats.first},
                {"duration_hours", std::round((stats.second / 3600000.0) * 10) / 10}
            };
        }
        
        j["by_channel"] = json::object();
        for (const auto& [ch, stats] : by_channel) {
            j["by_channel"][std::to_string(ch)] = {
                {"count", stats.first},
                {"duration_hours", std::round((stats.second / 3600000.0) * 10) / 10}
            };
        }
        
        if (!recordings.empty()) {
            j["oldest_recording"] = TimestampToIso8601(oldest_time);
            j["newest_recording"] = TimestampToIso8601(newest_time);
        }
        
        resp.status_code = 200;
        resp.body = j.dump(2);
    } catch (const std::exception& e) {
        spdlog::error("Error in HandlePlaybackStats: {}", e.what());
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
