#include "api_handlers.h"
#include <ipcam/audit_logger.h>
#include <chrono>
#include "handlers/status_handler.h"
#include "handlers/info_handler.h"
#include "handlers/config_handler.h"
#include "handlers/network_handler.h"
#include "handlers/stream_handler.h"
#include "handlers/smtp_handler.h"
#include "handlers/snmp_handler.h"
#include "handlers/upnp_handler.h"
#include "handlers/ssl_handler.h"
#include "handlers/auth_handler.h"
#include "handlers/user_handler.h"
#include "handlers/crypto_handler.h"
#include "handlers/image_handler.h"
#include "handlers/video_handler.h"
#include "handlers/audio_handler.h"
#include "handlers/osd_handler.h"
#include "handlers/recording_handler.h"
#include "handlers/schedule_handler.h"
#include "handlers/storage_handler.h"
#include "handlers/playback_handler.h"
// Analytics module - modular structure
#include "handlers/analytics/analytics_router.h"
#include "handlers/analytics/motion_handler.h"
#include "handlers/analytics/tampering_handler.h"
#include "handlers/analytics/notification_handler.h"
#include "handlers/analytics/smart_handler.h"
#include "handlers/analytics/linecross_handler.h"
#include "handlers/analytics/intrusion_handler.h"
#include "handlers/analytics/face_handler.h"
#include "handlers/analytics/lpr_handler.h"
#include "handlers/analytics/heatmap_handler.h"
#include "handlers/analytics/pose_handler.h"
#include "handlers/analytics/audio_handler.h"
#include "handlers/rtsp_handler.h"
#include "handlers/ir_handler.h"
#include "handlers/upgrade_handler.h"
#include "handlers/events_handler.h"
#include "handlers/system_handler.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace api {

// Base API handler class
class ApiHandler : public CivetHandler {
protected:
  Handler handler_;
  
  RequestContext buildContext(CivetServer* server, struct mg_connection* conn) {
    RequestContext ctx;
    ctx.server = server;
    ctx.conn = conn;
    
    const struct mg_request_info* req_info = mg_get_request_info(conn);
    ctx.method = req_info->request_method ? req_info->request_method : "";
    ctx.uri = req_info->local_uri ? req_info->local_uri : "";
    
    // Extract HTTP headers
    for (int i = 0; i < req_info->num_headers; i++) {
      std::string name = req_info->http_headers[i].name;
      std::string value = req_info->http_headers[i].value;
      ctx.headers[name] = value;
    }
    
    // Parse query parameters
    if (req_info->query_string) {
      std::string query = req_info->query_string;
      size_t pos = 0;
      while (pos < query.length()) {
        size_t eq = query.find('=', pos);
        size_t amp = query.find('&', pos);
        if (eq != std::string::npos && (amp == std::string::npos || eq < amp)) {
          std::string key = query.substr(pos, eq - pos);
          size_t val_end = (amp != std::string::npos) ? amp : query.length();
          std::string val = query.substr(eq + 1, val_end - eq - 1);
          ctx.query_params[key] = val;
          pos = (amp != std::string::npos) ? amp + 1 : query.length();
        } else {
          break;
        }
      }
    }
    
    // Read body for POST/PUT
    if (ctx.method == "POST" || ctx.method == "PUT") {
      char buffer[8192];
      int read_bytes = mg_read(conn, buffer, sizeof(buffer) - 1);
      if (read_bytes > 0) {
        buffer[read_bytes] = '\0';
        ctx.body = std::string(buffer, read_bytes);
      }
    }
    
    return ctx;
  }
  
  void sendResponse(struct mg_connection* conn, const Response& resp) {
    // Check if this is a file streaming response
    if (!resp.file_path.empty()) {
      // Open the file for streaming
      FILE* fp = fopen(resp.file_path.c_str(), "rb");
      if (!fp) {
        spdlog::error("Failed to open file for streaming: {}", resp.file_path);
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n");
        mg_printf(conn, "Content-Type: application/json\r\n");
        mg_printf(conn, "Content-Length: 32\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        mg_printf(conn, "{\"error\":\"File streaming failed\"}");
        return;
      }
      
      // Send headers
      mg_printf(conn, "HTTP/1.1 %d OK\r\n", resp.status_code);
      mg_printf(conn, "Content-Type: %s\r\n", resp.content_type.c_str());
      mg_printf(conn, "Content-Length: %zu\r\n", resp.file_size);
      mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
      
      for (const auto& [key, val] : resp.headers) {
        mg_printf(conn, "%s: %s\r\n", key.c_str(), val.c_str());
      }
      
      mg_printf(conn, "\r\n");
      
      // Stream file in chunks
      constexpr size_t CHUNK_SIZE = 65536;  // 64KB chunks
      char buffer[CHUNK_SIZE];
      size_t bytes_read;
      
      while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, fp)) > 0) {
        if (mg_write(conn, buffer, bytes_read) <= 0) {
          spdlog::warn("Client disconnected during file streaming");
          break;
        }
      }
      
      fclose(fp);
      return;
    }
    
    // Regular body-based response
    mg_printf(conn, "HTTP/1.1 %d OK\r\n", resp.status_code);
    mg_printf(conn, "Content-Type: %s\r\n", resp.content_type.c_str());
    mg_printf(conn, "Content-Length: %lu\r\n", resp.body.length());
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    
    for (const auto& [key, val] : resp.headers) {
      mg_printf(conn, "%s: %s\r\n", key.c_str(), val.c_str());
    }
    
    mg_printf(conn, "\r\n");
    mg_write(conn, resp.body.c_str(), resp.body.length());
  }

  // Central dispatch: build context, call handler, log access, send response
  bool dispatch(CivetServer* server, struct mg_connection* conn) {
    auto start = std::chrono::steady_clock::now();
    auto ctx = buildContext(server, conn);
    auto resp = handler_(ctx);
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count();

    sendResponse(conn, resp);

    // --- Access logging ---
    try {
      AccessLogEntry ale;
      // Extract client IP from headers (nginx sets X-Real-IP / X-Forwarded-For)
      auto hdr = [&](const std::string& name) -> std::string {
        auto it = ctx.headers.find(name);
        return it != ctx.headers.end() ? it->second : "";
      };
      ale.client_ip = hdr("X-Real-IP");
      if (ale.client_ip.empty()) {
        std::string fwd = hdr("X-Forwarded-For");
        if (!fwd.empty()) {
          auto comma = fwd.find(',');
          ale.client_ip = (comma != std::string::npos) ? fwd.substr(0, comma) : fwd;
        }
      }
      if (ale.client_ip.empty()) ale.client_ip = "unix";
      ale.method = ctx.method;
      ale.uri = ctx.uri;
      ale.status_code = resp.status_code;
      ale.response_time_us = elapsed;
      ale.user_agent = hdr("User-Agent");
      ale.request_size = (int64_t)ctx.body.size();
      ale.response_size = (int64_t)resp.body.size();
      // Username extraction: look for X-Authenticated-User set by auth middleware
      ale.username = hdr("X-Auth-User");
      AuditLogger::Instance().LogAccess(ale);
    } catch (...) { /* never let logging crash a request */ }

    return true;
  }

public:
  explicit ApiHandler(Handler h) : handler_(std::move(h)) {}
  
  bool handleGet(CivetServer* server, struct mg_connection* conn) override {
    return dispatch(server, conn);
  }
  
  bool handlePost(CivetServer* server, struct mg_connection* conn) override {
    return dispatch(server, conn);
  }
  
  bool handlePut(CivetServer* server, struct mg_connection* conn) override {
    return dispatch(server, conn);
  }
  
  bool handleDelete(CivetServer* server, struct mg_connection* conn) override {
    return dispatch(server, conn);
  }
};

// Not found handler
Response handleApiNotFound(const RequestContext& ctx) {
  json j;
  j["error"] = "Not Found";
  j["message"] = "The requested API endpoint does not exist";
  j["path"] = ctx.uri;
  
  Response resp;
  resp.status_code = 404;
  resp.body = j.dump(2);
  return resp;
}

// Register all API handlers
void RegisterHandlers(CivetServer* server) {
  spdlog::info("Registering API handlers...");
  
  server->addHandler("/api/v1/status", new ApiHandler(handlers::HandleStatus));
  server->addHandler("/api/v1/info", new ApiHandler(handlers::HandleInfo));
  server->addHandler("/api/v1/config", new ApiHandler(handlers::HandleConfig));
  server->addHandler("/api/v1/network", new ApiHandler(handlers::HandleNetwork));
  server->addHandler("/api/v1/stream", new ApiHandler(handlers::HandleStream));
  
  // SMTP handlers
  server->addHandler("/api/v1/smtp", new ApiHandler(handlers::HandleSmtp));
  server->addHandler("/api/v1/smtp/test", new ApiHandler(handlers::HandleSmtpTest));
  
  // SNMP handlers
  server->addHandler("/api/v1/snmp", new ApiHandler(handlers::HandleSnmp));
  server->addHandler("/api/v1/snmp/control", new ApiHandler(handlers::HandleSnmpControl));
  server->addHandler("/api/v1/snmp/test", new ApiHandler(handlers::HandleSnmpTest));
  
  // UPnP handlers
  server->addHandler("/api/v1/upnp", new ApiHandler(handlers::HandleUpnp));
  server->addHandler("/api/v1/upnp/discover", new ApiHandler(handlers::HandleUpnpDiscover));
  server->addHandler("/api/v1/upnp/portmappings", new ApiHandler(handlers::HandleUpnpPortMappings));
  server->addHandler("/api/v1/upnp/camera-ports", new ApiHandler(handlers::HandleUpnpCameraPorts));
  
  // SSL Certificate handlers
  server->addHandler("/api/v1/ssl", new ApiHandler(handlers::HandleSsl));
  server->addHandler("/api/v1/ssl/upload", new ApiHandler(handlers::HandleSslUpload));
  server->addHandler("/api/v1/ssl/generate", new ApiHandler(handlers::HandleSslGenerate));
  
  // Authentication handlers
  server->addHandler("/api/v1/auth/login", new ApiHandler(handlers::HandleAuthLogin));
  server->addHandler("/api/v1/auth/logout", new ApiHandler(handlers::HandleAuthLogout));
  server->addHandler("/api/v1/auth/change-password", new ApiHandler(handlers::HandleAuthChangePassword));
  server->addHandler("/api/v1/auth/initial-setup", new ApiHandler(handlers::HandleAuthInitialSetup));
  server->addHandler("/api/v1/auth/reset-password", new ApiHandler(handlers::HandleAuthResetPassword));
  server->addHandler("/api/v1/auth/verify", new ApiHandler(handlers::HandleAuthVerify));
  server->addHandler("/api/v1/auth/sessions", new ApiHandler(handlers::HandleAuthSessions));
  
  // Security Questions & Forgot Password handlers
  server->addHandler("/api/v1/auth/security-questions/available", new ApiHandler(handlers::HandleSecurityQuestionsAvailable));
  server->addHandler("/api/v1/auth/security-questions/setup", new ApiHandler(handlers::HandleSecurityQuestionsSetup));
  server->addHandler("/api/v1/auth/security-questions/status", new ApiHandler(handlers::HandleSecurityQuestionsStatus));
  server->addHandler("/api/v1/auth/forgot-password/questions", new ApiHandler(handlers::HandleForgotPasswordQuestions));
  server->addHandler("/api/v1/auth/forgot-password/verify", new ApiHandler(handlers::HandleForgotPasswordVerify));
  server->addHandler("/api/v1/auth/forgot-password/reset", new ApiHandler(handlers::HandleForgotPasswordReset));
  
  // Password encryption public key endpoint (for RSA+AES encrypted passwords)
  server->addHandler("/api/v1/auth/public-key", new ApiHandler(handlers::HandleGetPublicKey));
  server->addHandler("/api/v1/auth/rotate-keys", new ApiHandler(handlers::HandleRotateKeys));
  
  // User management handlers
  // Note: /api/v1/users/current and /api/v1/users/create are registered first
  // (more specific paths) so CivetWeb matches them before the generic router.
  server->addHandler("/api/v1/users/current", new ApiHandler(handlers::HandleUsersCurrent));
  server->addHandler("/api/v1/users/create", new ApiHandler(handlers::HandleUsersCreate));
  // Unified router handles: GET (list), POST (create), GET/:id, PUT/:id, DELETE/:id
  server->addHandler("/api/v1/users", new ApiHandler(handlers::HandleUsersRouter));
  
  // Image/ISP handlers
  server->addHandler("/api/v1/image", new ApiHandler(handlers::HandleImage));
  server->addHandler("/api/v1/image/adjustment", new ApiHandler(handlers::HandleImageAdjustment));
  server->addHandler("/api/v1/image/exposure", new ApiHandler(handlers::HandleImageExposure));
  server->addHandler("/api/v1/image/whitebalance", new ApiHandler(handlers::HandleImageWhiteBalance));
  server->addHandler("/api/v1/image/daynight", new ApiHandler(handlers::HandleImageDayNight));
  server->addHandler("/api/v1/image/blc", new ApiHandler(handlers::HandleImageBLC));
  server->addHandler("/api/v1/image/enhancement", new ApiHandler(handlers::HandleImageEnhancement));
  server->addHandler("/api/v1/image/orientation", new ApiHandler(handlers::HandleImageOrientation));
  server->addHandler("/api/v1/image/antiflicker", new ApiHandler(handlers::HandleImageAntiFlicker));
  server->addHandler("/api/v1/image/reset", new ApiHandler(handlers::HandleImageReset));
  server->addHandler("/api/v1/snapshot", new ApiHandler(handlers::HandleSnapshot));
  server->addHandler("/api/v1/snapshot/config", new ApiHandler(handlers::HandleSnapshotConfig));
  
  // Video handlers
  server->addHandler("/api/v1/video", new ApiHandler(handlers::HandleVideo));
  server->addHandler("/api/v1/video/video1", new ApiHandler(handlers::HandleVideoStream));
  server->addHandler("/api/v1/video/video2", new ApiHandler(handlers::HandleVideoStream));
  server->addHandler("/api/v1/video/video3", new ApiHandler(handlers::HandleVideoStream));
  server->addHandler("/api/v1/video/capabilities", new ApiHandler(handlers::HandleVideoCapabilities));
  server->addHandler("/api/v1/video/reset", new ApiHandler(handlers::HandleVideoReset));
  
  // Audio handlers
  server->addHandler("/api/v1/audio", new ApiHandler(handlers::HandleAudio));
  server->addHandler("/api/v1/audio/capabilities", new ApiHandler(handlers::HandleAudioCapabilities));
  server->addHandler("/api/v1/audio/test", new ApiHandler(handlers::HandleAudioTest));
  server->addHandler("/api/v1/audio/reset", new ApiHandler(handlers::HandleAudioReset));
  
  // OSD handlers
  spdlog::info("Registering OSD handler at /api/v1/osd/stream");
  server->addHandler("/api/v1/osd/stream", new ApiHandler(handlers::HandleOsdStreamConfig));
  
  spdlog::info("Registering Global OSD handler at /api/v1/osd/global");
  server->addHandler("/api/v1/osd/global", new ApiHandler(handlers::HandleOsdGlobalConfig));
  
  // Privacy mask handler (per-stream, 4 regions each)
  spdlog::info("Registering Privacy Mask handler at /api/v1/osd/privacy_mask");
  server->addHandler("/api/v1/osd/privacy_mask", new ApiHandler(handlers::HandleOsdPrivacyMask));
  
  // Global privacy mask handler (VIDEOPROC level, applies to all streams)
  spdlog::info("Registering Global Privacy Mask handler at /api/v1/osd/global_privacy_mask");
  server->addHandler("/api/v1/osd/global_privacy_mask", new ApiHandler(handlers::HandleOsdGlobalPrivacyMask));
  
  // OSD auto-scaling handler (global font size scaling based on resolution)
  spdlog::info("Registering OSD Auto-Scaling handler at /api/v1/osd/auto_scaling");
  server->addHandler("/api/v1/osd/auto_scaling", new ApiHandler(handlers::HandleOsdAutoScaling));
  
  // Recording handlers
  spdlog::info("Registering Recording handlers at /api/v1/recording/*");
  server->addHandler("/api/v1/recording", new ApiHandler(handlers::HandleRecording));
  server->addHandler("/api/v1/recording/sessions", new ApiHandler(handlers::HandleRecordingSessions));
  server->addHandler("/api/v1/recording/start", new ApiHandler(handlers::HandleRecordingStart));
  server->addHandler("/api/v1/recording/stop", new ApiHandler(handlers::HandleRecordingStop));
  server->addHandler("/api/v1/recording/storage", new ApiHandler(handlers::HandleRecordingStorage));
  server->addHandler("/api/v1/recording/storage/format", new ApiHandler(handlers::HandleRecordingStorageFormat));
  server->addHandler("/api/v1/recording/config", new ApiHandler(handlers::HandleRecordingConfig));
  server->addHandler("/api/v1/recording/channels/0", new ApiHandler(handlers::HandleRecordingChannel));
  server->addHandler("/api/v1/recording/channels/1", new ApiHandler(handlers::HandleRecordingChannel));
  server->addHandler("/api/v1/recording/channels/2", new ApiHandler(handlers::HandleRecordingChannel));
  
  // Recording management handlers
  spdlog::info("Registering Recording management handlers at /api/v1/recordings/*");
  server->addHandler("/api/v1/recordings", new ApiHandler(handlers::HandleRecordingsList));
  // Note: Individual recording handlers (/api/v1/recordings/{id}) use HandleRecordingGet
  // which extracts the ID from the URI - CivetWeb will route all /api/v1/recordings/* here
  
  // SD Card management handlers
  spdlog::info("Registering SD Card handlers at /api/v1/sdcard/*");
  server->addHandler("/api/v1/sdcard", new ApiHandler(handlers::HandleSdCardStatus));
  server->addHandler("/api/v1/sdcard/mount", new ApiHandler(handlers::HandleSdCardMount));
  server->addHandler("/api/v1/sdcard/unmount", new ApiHandler(handlers::HandleSdCardUnmount));
  server->addHandler("/api/v1/sdcard/format", new ApiHandler(handlers::HandleSdCardFormat));
  
  // Schedule recording handlers
  spdlog::info("Registering Schedule handlers at /api/v1/recording/schedules/*");
  server->addHandler("/api/v1/recording/schedules", new ApiHandler(handlers::HandleSchedules));
  server->addHandler("/api/v1/recording/schedule/config", new ApiHandler(handlers::HandleScheduleConfig));
  // Individual schedule profile handlers (ID extracted from URI)
  
  // NAS storage handlers
  spdlog::info("Registering NAS handlers at /api/v1/storage/nas/*");
  server->addHandler("/api/v1/storage/nas", new ApiHandler(handlers::HandleNasConfig));
  server->addHandler("/api/v1/storage/nas/test", new ApiHandler(handlers::HandleNasTest));
  server->addHandler("/api/v1/storage/nas/mount", new ApiHandler(handlers::HandleNasMount));
  server->addHandler("/api/v1/storage/nas/unmount", new ApiHandler(handlers::HandleNasUnmount));
  
  // FTP upload handlers
  spdlog::info("Registering FTP handlers at /api/v1/storage/ftp/*");
  server->addHandler("/api/v1/storage/ftp", new ApiHandler(handlers::HandleFtpConfig));
  server->addHandler("/api/v1/storage/ftp/test", new ApiHandler(handlers::HandleFtpTest));
  server->addHandler("/api/v1/storage/ftp/upload", new ApiHandler(handlers::HandleFtpUpload));
  server->addHandler("/api/v1/storage/ftp/queue", new ApiHandler(handlers::HandleFtpQueue));
  server->addHandler("/api/v1/storage/ftp/history", new ApiHandler(handlers::HandleFtpHistory));
  
  // Playback handlers
  spdlog::info("Registering Playback handlers at /api/v1/playback/*");
  server->addHandler("/api/v1/playback/search", new ApiHandler(handlers::HandlePlaybackSearch));
  server->addHandler("/api/v1/playback/calendar", new ApiHandler(handlers::HandlePlaybackCalendar));
  server->addHandler("/api/v1/playback/timeline", new ApiHandler(handlers::HandlePlaybackTimeline));
  server->addHandler("/api/v1/playback/dates", new ApiHandler(handlers::HandlePlaybackDates));
  server->addHandler("/api/v1/playback/stream", new ApiHandler(handlers::HandlePlaybackStream));
  server->addHandler("/api/v1/playback/download", new ApiHandler(handlers::HandlePlaybackDownload));
  server->addHandler("/api/v1/playback/thumbnail", new ApiHandler(handlers::HandlePlaybackThumbnail));
  server->addHandler("/api/v1/playback/recordings", new ApiHandler(handlers::HandlePlaybackRecordingInfo));
  
  // ============================================================================
  // Analytics v2 API - Modular Structure
  // ============================================================================
  spdlog::info("Registering Analytics v2 modular handlers at /api/v1/analytics/*");
  
  // Top-level analytics (from analytics_router)
  server->addHandler("/api/v1/analytics", new ApiHandler(handlers::analytics::HandleAnalytics));
  server->addHandler("/api/v1/analytics/status", new ApiHandler(handlers::analytics::HandleAnalyticsStatus));
  server->addHandler("/api/v1/analytics/capabilities", new ApiHandler(handlers::analytics::HandleAnalyticsCapabilities));
  
  // Motion Detection API (from motion_handler)
  server->addHandler("/api/v1/analytics/motion", new ApiHandler(handlers::analytics::HandleMotionDetection));
  server->addHandler("/api/v1/analytics/motion/zones", new ApiHandler(handlers::analytics::HandleMotionZones));
  server->addHandler("/api/v1/analytics/motion/schedule", new ApiHandler(handlers::analytics::HandleMotionSchedule));
  server->addHandler("/api/v1/analytics/motion/actions", new ApiHandler(handlers::analytics::HandleMotionActions));
  
  // Video Tampering API (from tampering_handler)
  server->addHandler("/api/v1/analytics/tampering", new ApiHandler(handlers::analytics::HandleVideoTampering));
  server->addHandler("/api/v1/analytics/tampering/schedule", new ApiHandler(handlers::analytics::HandleTamperingSchedule));
  server->addHandler("/api/v1/analytics/tampering/actions", new ApiHandler(handlers::analytics::HandleTamperingActions));
  server->addHandler("/api/v1/analytics/tampering/status", new ApiHandler(handlers::analytics::HandleTamperingStatus));
  
  // Notification Servers API (from notification_handler)
  server->addHandler("/api/v1/analytics/notifications", new ApiHandler(handlers::analytics::HandleNotificationServers));
  server->addHandler("/api/v1/analytics/notifications/mqtt", new ApiHandler(handlers::analytics::HandleMqttConfig));
  server->addHandler("/api/v1/analytics/notifications/email", new ApiHandler(handlers::analytics::HandleEmailConfig));
  server->addHandler("/api/v1/analytics/notifications/ftp", new ApiHandler(handlers::analytics::HandleFtpConfig));
  server->addHandler("/api/v1/analytics/notifications/test", new ApiHandler(handlers::analytics::HandleNotificationTest));
  
  // ============================================================================
  // Future Analytics Features (routes reserved for expansion)
  // Smart Detection, Face, LPR, Line Crossing, Intrusion, etc.
  // Will be added as individual handler modules similar to motion/tampering
  // ============================================================================
  // server->addHandler("/api/v1/analytics/smart", ...);
  // server->addHandler("/api/v1/analytics/face", ...);
  // server->addHandler("/api/v1/analytics/lpr", ...);
  // server->addHandler("/api/v1/analytics/linecross", ...);
  // server->addHandler("/api/v1/analytics/intrusion", ...);
  // server->addHandler("/api/v1/analytics/counting", ...);
  // server->addHandler("/api/v1/analytics/heatmap", ...);
  // server->addHandler("/api/v1/analytics/crowd", ...);
  // server->addHandler("/api/v1/analytics/sound", ...);
  // server->addHandler("/api/v1/analytics/lists", ...);       // List management
  // server->addHandler("/api/v1/analytics/statistics", ...);  // Analytics stats
  
  // Legacy analytics endpoints (backward compatibility with old handlers)
  server->addHandler("/api/v1/analytics/toggles", new ApiHandler(handlers::analytics::HandleAnalyticsToggles));
  
  // Smart Detection API (from smart_handler)
  server->addHandler("/api/v1/analytics/smart", new ApiHandler(handlers::analytics::HandleSmartDetection));
  server->addHandler("/api/v1/analytics/smart/zones", new ApiHandler(handlers::analytics::HandleSmartZones));
  server->addHandler("/api/v1/analytics/smart/schedule", new ApiHandler(handlers::analytics::HandleSmartSchedule));
  server->addHandler("/api/v1/analytics/smart/actions", new ApiHandler(handlers::analytics::HandleSmartActions));
  
  // Line Crossing API (from linecross_handler)
  server->addHandler("/api/v1/analytics/linecross", new ApiHandler(handlers::analytics::HandleLineCrossing));
  server->addHandler("/api/v1/analytics/linecross/lines", new ApiHandler(handlers::analytics::HandleLineCrossLines));
  server->addHandler("/api/v1/analytics/linecross/counts", new ApiHandler(handlers::analytics::HandleLineCrossCounts));
  server->addHandler("/api/v1/analytics/linecross/schedule", new ApiHandler(handlers::analytics::HandleLineCrossSchedule));
  server->addHandler("/api/v1/analytics/linecross/actions", new ApiHandler(handlers::analytics::HandleLineCrossActions));
  
  // Zone Intrusion API (from intrusion_handler)
  server->addHandler("/api/v1/analytics/intrusion", new ApiHandler(handlers::analytics::HandleZoneIntrusion));
  server->addHandler("/api/v1/analytics/intrusion/zones", new ApiHandler(handlers::analytics::HandleIntrusionZones));
  server->addHandler("/api/v1/analytics/intrusion/schedule", new ApiHandler(handlers::analytics::HandleIntrusionSchedule));
  server->addHandler("/api/v1/analytics/intrusion/actions", new ApiHandler(handlers::analytics::HandleIntrusionActions));
  
  // Face Detection & Recognition API (from face_handler)
  server->addHandler("/api/v1/analytics/face", new ApiHandler(handlers::analytics::HandleFaceDetection));
  server->addHandler("/api/v1/analytics/face/gallery", new ApiHandler(handlers::analytics::HandleFaceGallery));
  
  // License Plate Recognition API (from lpr_handler)
  server->addHandler("/api/v1/analytics/lpr", new ApiHandler(handlers::analytics::HandleLpr));
  
  // Heat Map API (from heatmap_handler)
  server->addHandler("/api/v1/analytics/heatmap", new ApiHandler(handlers::analytics::HandleHeatMap));
  server->addHandler("/api/v1/analytics/heatmap/grid", new ApiHandler(handlers::analytics::HandleHeatMapGrid));
  server->addHandler("/api/v1/analytics/heatmap/reset", new ApiHandler(handlers::analytics::HandleHeatMapReset));
  
  // Pose Estimation API (from pose_handler)
  server->addHandler("/api/v1/analytics/pose", new ApiHandler(handlers::analytics::HandlePoseEstimation));
  
  // Audio Classification API (from audio_handler)
  server->addHandler("/api/v1/analytics/audio", new ApiHandler(handlers::analytics::HandleAudioClassification));
  
  // Legacy endpoint aliases (backward compatibility)
  server->addHandler("/api/v1/analytics/motion-zones", new ApiHandler(handlers::analytics::HandleMotionZones));
  server->addHandler("/api/v1/analytics/line-zones", new ApiHandler(handlers::analytics::HandleLineCrossLines));
  server->addHandler("/api/v1/analytics/line-counts", new ApiHandler(handlers::analytics::HandleLineCrossCounts));
  
  // RTSP Server configuration handlers
  spdlog::info("Registering RTSP handlers at /api/v1/rtsp/*");
  server->addHandler("/api/v1/rtsp", new ApiHandler(handlers::HandleRtsp));
  server->addHandler("/api/v1/rtsp/auth", new ApiHandler(handlers::HandleRtspAuth));
  server->addHandler("/api/v1/rtsp/control", new ApiHandler(handlers::HandleRtspControl));
  server->addHandler("/api/v1/rtsp/status", new ApiHandler(handlers::HandleRtspStatus));
  server->addHandler("/api/v1/rtsp/clients", new ApiHandler(handlers::HandleRtspClients));
  
  // IR LED and IR Cut Filter control handlers
  spdlog::info("Registering IR control handlers at /api/v1/ir/*");
  server->addHandler("/api/v1/ir", new ApiHandler(handlers::HandleIR));
  server->addHandler("/api/v1/ir/led", new ApiHandler(handlers::HandleIRLed));
  server->addHandler("/api/v1/ir/led/brightness", new ApiHandler(handlers::HandleIRLedBrightness));
  server->addHandler("/api/v1/ir/cut", new ApiHandler(handlers::HandleIRCut));
  server->addHandler("/api/v1/ir/daynight", new ApiHandler(handlers::HandleDayNight));
  server->addHandler("/api/v1/ir/daynight/mode", new ApiHandler(handlers::HandleDayNightMode));
  server->addHandler("/api/v1/ir/status", new ApiHandler(handlers::HandleIRStatus));
  server->addHandler("/api/v1/ir/action", new ApiHandler(handlers::HandleIRAction));
  
  // Firmware upgrade handlers
  spdlog::info("Registering Firmware upgrade handlers at /api/v1/firmware/*");
  server->addHandler("/api/v1/firmware/version", new ApiHandler(handlers::HandleFirmwareVersion));
  server->addHandler("/api/v1/firmware/status", new ApiHandler(handlers::HandleFirmwareStatus));
  server->addHandler("/api/v1/firmware/upload", new ApiHandler(handlers::HandleFirmwareUpload));
  server->addHandler("/api/v1/firmware/upgrade", new ApiHandler(handlers::HandleFirmwareUpgrade));
  server->addHandler("/api/v1/firmware/validate", new ApiHandler(handlers::HandleFirmwareValidate));
  server->addHandler("/api/v1/firmware/cancel", new ApiHandler(handlers::HandleFirmwareCancel));
  
  // Events handlers (rules, stats, test events)
  spdlog::info("Registering Events handlers at /api/v1/events/*");
  server->addHandler("/api/v1/events", new ApiHandler(handlers::HandleEvents));
  server->addHandler("/api/v1/events/rules", new ApiHandler(handlers::HandleEventsRules));
  server->addHandler("/api/v1/events/stats", new ApiHandler(handlers::HandleEventsStats));
  server->addHandler("/api/v1/events/test", new ApiHandler(handlers::HandleEventsTest));
  
  // ============================================================================
  // System API - device info, time/NTP, reboot, factory reset, diagnostics,
  //              logs, maintenance, watchdog, backup/restore, alarms
  // ============================================================================
  spdlog::info("Registering System handlers at /api/v1/system/*");

  // 1. Device information
  server->addHandler("/api/v1/system/device", new ApiHandler(handlers::HandleSystemDevice));

  // 2. Date/Time & NTP
  server->addHandler("/api/v1/system/time", new ApiHandler(handlers::HandleSystemTime));
  server->addHandler("/api/v1/system/time/ntp", new ApiHandler(handlers::HandleSystemTimeNtp));
  server->addHandler("/api/v1/system/time/sync", new ApiHandler(handlers::HandleSystemTimeSync));
  server->addHandler("/api/v1/system/time/dst", new ApiHandler(handlers::HandleSystemTimeDst));

  // 3. Reboot / Shutdown
  server->addHandler("/api/v1/system/reboot", new ApiHandler(handlers::HandleSystemReboot));
  server->addHandler("/api/v1/system/shutdown", new ApiHandler(handlers::HandleSystemShutdown));

  // 4. Factory Reset
  server->addHandler("/api/v1/system/factory-reset", new ApiHandler(handlers::HandleSystemFactoryReset));

  // 5. Diagnostics / Health
  server->addHandler("/api/v1/system/diagnostics", new ApiHandler(handlers::HandleSystemDiagnostics));

  // 6. Logs
  server->addHandler("/api/v1/system/logs", new ApiHandler(handlers::HandleSystemLogs));
  server->addHandler("/api/v1/system/logs/download", new ApiHandler(handlers::HandleSystemLogsDownload));
  server->addHandler("/api/v1/system/logs/config", new ApiHandler(handlers::HandleSystemLogsConfig));

  // 7. Maintenance
  server->addHandler("/api/v1/system/maintenance", new ApiHandler(handlers::HandleSystemMaintenance));

  // 8. Watchdog
  server->addHandler("/api/v1/system/watchdog", new ApiHandler(handlers::HandleSystemWatchdog));

  // 10. Config Backup / Restore
  server->addHandler("/api/v1/system/backup", new ApiHandler(handlers::HandleSystemBackup));
  server->addHandler("/api/v1/system/restore", new ApiHandler(handlers::HandleSystemRestore));

  // 11. Alarm Thresholds
  server->addHandler("/api/v1/system/alarms", new ApiHandler(handlers::HandleSystemAlarms));

  // 12. Enhanced Logs
  server->addHandler("/api/v1/system/logs/sources", new ApiHandler(handlers::HandleSystemLogsSources));
  server->addHandler("/api/v1/system/logs/security", new ApiHandler(handlers::HandleSystemLogsSecurity));
  server->addHandler("/api/v1/system/logs/audit", new ApiHandler(handlers::HandleSystemLogsAudit));
  server->addHandler("/api/v1/system/logs/access", new ApiHandler(handlers::HandleSystemLogsAccess));
  server->addHandler("/api/v1/system/logs/syslog", new ApiHandler(handlers::HandleSystemLogsSyslog));
  server->addHandler("/api/v1/system/diagnostics/report", new ApiHandler(handlers::HandleSystemDiagReport));

  // System Logger control (replaces shell-script-based system_logger.sh)
  server->addHandler("/api/v1/system/logger", new ApiHandler(handlers::HandleSystemLogger));
  server->addHandler("/api/v1/system/logger/collect", new ApiHandler(handlers::HandleSystemLoggerCollect));

  spdlog::info("API handlers registered successfully");
}

} // namespace api
} // namespace webserver
} // namespace ipcam
