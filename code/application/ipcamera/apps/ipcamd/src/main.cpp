#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <memory>
#include <sys/stat.h>
#include "ipcam/config.h"
#include "ipcam/paths.h"  // Centralized path definitions
#include "ipcam/storage.h"
#include "ipcam/recording.h"
#include "ipcam/schedule_manager.h"
#include "ipcam/nas_manager.h"
#include "ipcam/ftp_manager.h"
#include "ipcam/webserver.h"
#include "ipcam/video_control.h"  // Video control module
#include "ipcam/isp_control.h"    // ISP control module
#include "ipcam/ir_control.h"     // IR LED and Day/Night control
#include "ipcam/reset_button.h"   // Reset button (P_GPIO12)
#include "ipcam/onvif.h"  // ONVIF module
#include "ipcam/network_manager.h"  // Network management
#include "ipcam/streaming.h"  // Streaming module (go2rtc)
#include "ipcam/rtsp_server.h"  // RTSP server for codec change callback
#include "ipcam/hdal_pipeline.h"  // HDAL video pipeline
#include "ipcam/media_hub.h"  // Central media distribution hub
#include "ipcam/analytics.h"  // AI Analytics engine
#include "ipcam/tamper_detection.h"  // TamperType / TamperResult / HasFlag
#include "ipcam/lpr.h"              // PlateDetection
#include "ipcam/audio_classification.h"  // AudioEvent
#include "ipcam/onvif_metadata.h"  // ONVIF metadata streaming
#include "ipcam/event_manager.h"  // Event system for actions/notifications
#include "ipcam/event_rule.h"     // EventRule, Action, ActionConfig types
#include "ipcam/action_handler.h"  // Action handler factory
#include "ipcam/system_logger.h"   // System data collector (replaces shell logger)

static std::atomic<bool> g_running{true};

static void handle_sig(int){
  g_running = false;
}

struct Config {
  std::string config_path = "/etc/ipcamera/configs";
};

void print_usage(const char* program_name) {
  std::cout << "Usage: " << program_name << " [OPTIONS]\n"
            << "Options:\n"
            << "  -c, --config <path>   Configuration directory path (default: /etc/ipcamera/configs)\n"
            << "  -h, --help            Show this help message\n"
            << std::endl;
}

bool parse_arguments(int argc, char* argv[], Config& config) {
  for (int i = 1; i < argc; i++) {
    const std::string arg = argv[i];
    
    if (arg == "-c" || arg == "--config") {
      if (i + 1 >= argc) {
        spdlog::error("Option {} requires an argument", arg);
        return false;
      }
      config.config_path = argv[++i];
    } 
    else if (arg == "-h" || arg == "--help") {
      print_usage(argv[0]);
      exit(0);
    } 
    else {
      spdlog::error("Unknown option: {}", arg);
      return false;
    }
  }
  return true;
}

// ============================================================================
// Bridge analytics.json action configs → EventRules
// ============================================================================
static void RegisterAnalyticsEventRules(ipcam::events::EventManager& mgr) {
  using namespace ipcam::events;
  using namespace ipcam::config;

  // Helper: build actions vector from a config prefix like
  // "analytics.motion_detection.actions"
  auto build_actions = [](const std::string& prefix) -> std::vector<Action> {
    std::vector<Action> actions;

    // Recording
    if (Get<bool>(prefix + ".recording.enabled", false)) {
      Action a;
      a.type = ActionType::kStartRecording;
      a.enabled = true;
      RecordingActionConfig rc;
      rc.pre_record_seconds  = Get<int>(prefix + ".recording.pre_record_sec", 5);
      rc.post_record_seconds = Get<int>(prefix + ".recording.post_record_sec", 30);
      a.config = rc;
      actions.push_back(std::move(a));
    }
    // Snapshot
    if (Get<bool>(prefix + ".snapshot.enabled", false)) {
      Action a;
      a.type = ActionType::kCaptureSnapshot;
      a.enabled = true;
      SnapshotActionConfig sc;
      sc.count       = Get<int>(prefix + ".snapshot.count", 1);
      sc.interval_ms = Get<int>(prefix + ".snapshot.interval_ms", 500);
      a.config = sc;
      actions.push_back(std::move(a));
    }
    // Webhook (https_notify)
    if (Get<bool>(prefix + ".https_notify.enabled", false)) {
      Action a;
      a.type = ActionType::kSendWebhook;
      a.enabled = true;
      WebhookActionConfig wc;
      wc.url    = Get<std::string>(prefix + ".https_notify.url", "");
      wc.method = Get<std::string>(prefix + ".https_notify.method", "POST");
      a.config = wc;
      actions.push_back(std::move(a));
    }
    // MQTT
    if (Get<bool>(prefix + ".mqtt.enabled", false)) {
      Action a;
      a.type = ActionType::kPublishMqtt;
      a.enabled = true;
      MqttActionConfig mc;
      mc.topic = Get<std::string>(prefix + ".mqtt.topic", "");
      mc.qos   = Get<int>(prefix + ".mqtt.qos", 1);
      a.config = mc;
      actions.push_back(std::move(a));
    }
    // Email
    if (Get<bool>(prefix + ".email.enabled", false)) {
      Action a;
      a.type = ActionType::kSendEmail;
      a.enabled = true;
      EmailActionConfig ec;
      ec.subject_template = Get<std::string>(prefix + ".email.subject", "");
      ec.attach_snapshot  = Get<bool>(prefix + ".email.attach_snapshot", true);
      a.config = ec;
      actions.push_back(std::move(a));
    }
    // FTP upload
    if (Get<bool>(prefix + ".ftp_upload.enabled", false)) {
      Action a;
      a.type = ActionType::kUploadFtp;
      a.enabled = true;
      FtpActionConfig fc;
      fc.upload_snapshot = Get<bool>(prefix + ".ftp_upload.upload_snapshot", true);
      fc.upload_video    = Get<bool>(prefix + ".ftp_upload.upload_video", false);
      a.config = fc;
      actions.push_back(std::move(a));
    }
    return actions;
  };

  struct RuleDef {
    std::string config_key;          // e.g. "analytics.motion_detection"
    std::string name;
    std::vector<EventType> triggers;
  };

  const RuleDef defs[] = {
    {"analytics.motion_detection",  "Motion Detection",
        {EventType::kMotionStart}},
    {"analytics.smart_detection",   "Smart Detection",
        {EventType::kPersonDetected, EventType::kVehicleDetected}},
    {"analytics.line_crossing",     "Line Crossing",
        {EventType::kLineCrossedAny}},
    {"analytics.zone_intrusion",    "Zone Intrusion",
        {EventType::kZoneEntered, EventType::kZoneLoitering}},
    {"analytics.video_tampering",   "Video Tampering",
        {EventType::kTamperDetected}},
    {"analytics.face_detection",    "Face Detection",
        {EventType::kFaceDetected}},
    {"analytics.license_plate",     "License Plate Recognition",
        {EventType::kLprDetected}},
  };

  int count = 0;
  for (const auto& def : defs) {
    if (!Get<bool>(def.config_key + ".enabled", false))
      continue;

    auto actions = build_actions(def.config_key + ".actions");
    if (actions.empty())
      continue;

    EventRule rule = EventRule::Create(def.name);
    rule.trigger_events = def.triggers;
    rule.cooldown_seconds = 10;
    rule.actions = std::move(actions);

    mgr.AddRule(rule);
    ++count;
  }

  spdlog::info("Registered {} analytics event rules from configuration", count);
}

int main(int argc, char* argv[]){
  // Create log directory if it doesn't exist
  mkdir("/var/log", 0755);
  
  // Create multi-sink logger: console + rotating file
  std::vector<spdlog::sink_ptr> sinks;
  
  // Console sink (colored output)
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::info);
  sinks.push_back(console_sink);
  
  // Rotating file sink (max 5MB per file, keep 3 backup files)
  try {
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "/var/log/ipcamd.log", 5 * 1024 * 1024, 3);
      file_sink->set_level(spdlog::level::debug);
    sinks.push_back(file_sink);
  } catch (const spdlog::spdlog_ex& ex) {
    std::cerr << "Log file init failed: " << ex.what() << std::endl;
  }
  
  // Create logger with multiple sinks
  auto logger = std::make_shared<spdlog::logger>("ipcamd", sinks.begin(), sinks.end());
  logger->set_level(spdlog::level::debug);  // Allow debug level
  logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
  spdlog::set_default_logger(logger);
  spdlog::flush_every(std::chrono::seconds(5));  // Flush to file every 5 seconds
  
  signal(SIGINT, handle_sig);
  signal(SIGTERM, handle_sig);

  // Parse command line arguments
  Config config;
  if (!parse_arguments(argc, argv, config)) {
    print_usage(argv[0]);
    return 1;
  }

  spdlog::info("ipcamd starting...");
  spdlog::info("Using configuration directory: {}", config.config_path);

  // Initialize configuration
  std::string cfgErr;
  if(!ipcam::config::Init(config.config_path, cfgErr)){
    spdlog::error("Config init failed: {}", cfgErr);
    return 2;
  }
  spdlog::info("Configuration Path set successfully");

  // Initialize network based on configuration
  spdlog::info("Initializing network interfaces...");
  auto& net_mgr = ipcam::networking::NetworkManager::Instance();
  auto net_result = net_mgr.InitializeNetwork();
  if (!net_result.success) {
    spdlog::warn("Network initialization failed: {}", net_result.error);
    spdlog::warn("Continuing with current network configuration...");
  } else {
    spdlog::info("Network initialized successfully");
  }

  // Initialize SystemLogger (periodic system data collection — replaces shell-based system_logger.sh)
  // Must be after config::Init so it can read logging.system_logger.* settings
  spdlog::info("Initializing SystemLogger...");
  if (ipcam::platform::SystemLogger::Instance().Init()) {
    spdlog::info("SystemLogger initialized successfully");
  } else {
    spdlog::warn("SystemLogger initialization failed — system data collection disabled");
  }

  // Initialize HDAL video pipeline (sensor, ISP, encoders, RTSP server)
  spdlog::info("Initializing HDAL video pipeline...");
  auto& hdal_pipeline = ipcam::platform::HdalPipeline::Instance();
  if (!hdal_pipeline.Init()) {
    spdlog::error("HDAL pipeline initialization failed");
    spdlog::warn("Continuing without video pipeline - RTSP streaming will not work");
  } else {
    spdlog::info("HDAL pipeline initialized successfully");
    // Start the pipeline (this also starts nvtrtspd_ipc)
    if (!hdal_pipeline.Start()) {
      spdlog::error("HDAL pipeline start failed");
    } else {
      spdlog::info("HDAL pipeline started - RTSP streaming available");
      
      // Initialize MediaHub (central media distribution - shared ring buffers)
      // Must be started before RTSP streaming and recording so consumers can register
      spdlog::info("Initializing MediaHub (shared video ring buffers)...");
      auto& media_hub = ipcam::media::MediaHub::Instance();
      if (media_hub.Initialize()) {
        if (media_hub.Start()) {
          spdlog::info("MediaHub started with {} active channels",
                       media_hub.GetActiveChannelCount());
        } else {
          spdlog::error("MediaHub failed to start producer threads");
        }
      } else {
        spdlog::error("MediaHub initialization failed");
      }
      
      // Initialize AI Analytics engine (after HDAL pipeline is running)
      // AI is optional - failures should not crash the main application
      try {
        spdlog::info("Initializing AI Analytics engine...");
        auto& analytics = ipcam::ai::AnalyticsEngine::Instance();
        analytics.LoadConfig();
        
        if (analytics.GetConfig().enabled) {
          if (analytics.Init()) {
            // Connect analytics to HDAL pipeline paths
            if (hdal_pipeline.IsAnalyticsPathAvailable()) {
              analytics.SetVideoProcPath(hdal_pipeline.GetAnalyticsPath());
              spdlog::info("AI Analytics connected to VideoProc path");
            }

            // Connect encoder path for SMART_BBOX overlay
            uint64_t enc_path = hdal_pipeline.GetVideoEncoderPath(0);  // Main stream
            if (enc_path != 0) {
              analytics.SetEncoderPath(enc_path);
              spdlog::info("AI Analytics connected to encoder path for SMART_BBOX");
            }
            
            if (analytics.Start()) {
              spdlog::info("AI Analytics engine started");
              
              // Detection callback (ONVIF metadata + person/vehicle events) is
              // registered later after EventManager initialises.
            } else {
              spdlog::warn("AI Analytics engine failed to start - continuing without AI");
            }
          } else {
            spdlog::warn("AI Analytics engine initialization failed - continuing without AI");
          }
        } else {
          spdlog::info("AI Analytics disabled in configuration");
        }
      } catch (const std::exception& e) {
        spdlog::error("AI Analytics exception: {} - continuing without AI", e.what());
      } catch (...) {
        spdlog::error("AI Analytics unknown exception - continuing without AI");
      }
    }
  }
  // Initialize Video & ISP Control modules
  // These wrap HDAL/ISP functions and provide persistent configuration
  spdlog::info("Initializing Video and ISP Control modules...");
  
  auto& video_ctrl = ipcam::media::VideoControl::Instance();
  if (video_ctrl.Init()) {
      spdlog::info("Video Control module initialized");
  } else {
      spdlog::warn("Video Control module initialization failed");
  }
  // Note: ISPControl is NOT initialized here on purpose.
  // The HDAL pipeline already sets up ISP with proper defaults.
  // ISPControl will be used on-demand when ONVIF sets imaging settings.
  // Initializing it here would apply config defaults to hardware,
  // potentially overriding the correct HDAL-initialized state.

  // Initialize IR Control module (IR LED, IR Cut filter, Day/Night auto-switching)
  spdlog::info("Initializing IR Control module...");
  auto& ir_ctrl = ipcam::platform::IRControl::Instance();
  if (ir_ctrl.Init()) {
    spdlog::info("IR Control module initialized");
    
    // Register callback for day/night mode changes to sync with ISP B/W mode
    ir_ctrl.SetDayNightCallback([](bool is_night) {
      spdlog::info("Day/Night mode changed: {}", is_night ? "NIGHT" : "DAY");
      // ISP B/W mode is handled internally by IRControl when sync_sensor_bw_mode is enabled
    });
  } else {
    spdlog::warn("IR Control module initialization failed - IR features disabled");
  }

  // Initialize Reset Button module (P_GPIO12 - 10 second hold for factory reset)
  spdlog::info("Initializing Reset Button module...");
  auto& reset_btn = ipcam::platform::ResetButton::Instance();
  if (reset_btn.Init()) {
    spdlog::info("Reset Button module initialized");
    
    // Register factory reset callback
    reset_btn.SetResetCallback([]() {
      spdlog::warn("=== FACTORY RESET TRIGGERED ===");
      spdlog::warn("Restoring factory defaults and rebooting...");
      
      int ret;
      std::string cmd;
      
      // Step 1: Remove all user configuration files
      spdlog::info("Removing user configuration...");
      cmd = std::string("rm -rf ") + ipcam::paths::kConfigDirActive + "/*.json 2>/dev/null";
      ret = system(cmd.c_str());
      (void)ret;
      
      // Step 2: Restore factory default configuration
      // Copy factory defaults to the active config directory
      spdlog::info("Restoring factory default configuration...");
      cmd = std::string("cp -f ") + ipcam::paths::kConfigDirFactory + "/*.json " 
            + ipcam::paths::kConfigDirActive + "/ 2>/dev/null";
      ret = system(cmd.c_str());
      (void)ret;
      
      // Step 3: Reset network configuration to defaults (DHCP)
      spdlog::info("Resetting network configuration...");
      cmd = std::string("rm -f ") + ipcam::paths::kEth0Config + " 2>/dev/null";
      ret = system(cmd.c_str());
      (void)ret;
      
      // Step 4: Clear any cached credentials or tokens
      spdlog::info("Clearing cached data...");
      cmd = std::string("rm -rf ") + ipcam::paths::kCacheDir + "/* 2>/dev/null";
      ret = system(cmd.c_str());
      (void)ret;
      
      // Step 5: Delete all database files (secure.db, usermng.db, users.db)
      spdlog::info("Deleting database files...");
      cmd = std::string("rm -f ") + ipcam::paths::kSecureDatabase + " 2>/dev/null";
      ret = system(cmd.c_str());
      (void)ret;
      cmd = std::string("rm -f ") + ipcam::paths::kUserDatabase + " 2>/dev/null";
      ret = system(cmd.c_str());
      (void)ret;
      cmd = std::string("rm -f ") + ipcam::paths::kUsersDatabase + " 2>/dev/null";
      ret = system(cmd.c_str());
      (void)ret;
      cmd = std::string("rm -f ") + ipcam::paths::kUsersDbLegacy + " 2>/dev/null";
      ret = system(cmd.c_str());
      (void)ret;
      
      // Step 6: Delete SSL certificates (will be regenerated on next boot)
      spdlog::info("Deleting SSL certificates...");
      cmd = std::string("rm -rf ") + ipcam::paths::kSslDir + "/* 2>/dev/null";
      ret = system(cmd.c_str());
      (void)ret;
      
      // Sync filesystem before reboot
      sync();
      
      spdlog::warn("Factory defaults restored. Blinking IR LED to notify user...");
      
      // Blink IR LED twice to notify user that factory reset is complete
      // Use ForceIRLed which bypasses day/night mode check
      auto& ir_ctrl = ipcam::platform::IRControl::Instance();
      if (ir_ctrl.IsInitialized()) {
        constexpr int kResetCompleteBlinks = 2;
        constexpr int kBlinkDurationMs = 300;
        for (int i = 0; i < kResetCompleteBlinks; i++) {
          ir_ctrl.ForceIRLed(true);
          std::this_thread::sleep_for(std::chrono::milliseconds(kBlinkDurationMs));
          ir_ctrl.ForceIRLed(false);
          std::this_thread::sleep_for(std::chrono::milliseconds(kBlinkDurationMs));
        }
      }
      
      constexpr int kRebootDelaySec = 1;
      spdlog::warn("Rebooting in {} second...", kRebootDelaySec);
      std::this_thread::sleep_for(std::chrono::seconds(kRebootDelaySec));
      
      // Reboot the system
      ret = system("reboot");
      (void)ret;
    });
    
    // Register progress callback for logging only
    reset_btn.SetProgressCallback([](int elapsed, int total) {
      spdlog::info("Reset button progress: {}/{} seconds", elapsed, total);
    });
  } else {
    spdlog::warn("Reset Button module initialization failed - reset button disabled");
  }

  // Initialize streaming (RTSP server or go2rtc)
  bool enable_rtsp = ipcam::config::Get<bool>("streaming.rtsp.enabled", true);
  bool enable_go2rtc = ipcam::config::Get<bool>("streaming.go2rtc.enabled", false);
  
  if (enable_rtsp && hdal_pipeline.IsRunning()) {
    // Stop any existing go2rtc instance before starting RTSP server
    // This prevents port 554 conflicts if go2rtc was left running from a crash or init.d
    if (ipcam::streaming::Go2rtcManager::Instance().IsRunning()) {
      spdlog::info("Stopping existing go2rtc instance before RTSP server startup...");
      ipcam::streaming::Go2rtcManager::Instance().Stop();
      // Wait a moment for port to be released
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Use new Live555 RTSP server (replaces nvtrtspd_ipc - direct HDAL buffer access)
    spdlog::info("Initializing Live555 RTSP server...");
    if (ipcam::streaming::Initialize()) {
      if (ipcam::streaming::Start()) {
        spdlog::info("RTSP server started successfully");
        
        // Log available stream URLs
        auto& rtsp_server = ipcam::streaming::GetRtspServer();
        spdlog::info("  Stream 1: {}", rtsp_server.GetStreamUrl("stream1"));
        spdlog::info("  Stream 2: {}", rtsp_server.GetStreamUrl("stream2"));
        
        // Refresh RTSP stream when encoder settings change via API
        ipcam::media::VideoControl::Instance().SetStreamSettingsChangeCallback(
          [&rtsp_server](int stream_idx,
                         const ipcam::media::VideoStreamConfig& old_cfg,
                         const ipcam::media::VideoStreamConfig& new_cfg) {
            (void)old_cfg;
            (void)new_cfg;

            if (rtsp_server.IsRunning()) {
              spdlog::info("Encoder settings changed for stream {}, refreshing RTSP session", stream_idx);
              rtsp_server.RefreshStream(stream_idx);
            }

            // Force an immediate recording segment cut so the new segment
            // picks up the updated codec / resolution / fps / etc.
            auto& rec = ipcam::recording::RecordingService::Instance();
            rec.NotifyEncoderSettingsChanged(stream_idx);
          });
        spdlog::info("Stream settings change callback registered for RTSP refresh + recording");
      } else {
        spdlog::error("RTSP server failed to start");
      }
    } else {
      spdlog::error("RTSP server configuration failed");
    }
  }
  // Note: If streaming.rtsp.enabled is false, HdalPipeline will start nvtrtspd_ipc instead
  
  // Initialize go2rtc (WebRTC/HLS restreamer - works alongside RTSP server)
  if (enable_go2rtc) {
    spdlog::info("Initializing go2rtc restreamer...");
    auto& streaming_mgr = ipcam::streaming::Go2rtcManager::Instance();
    auto streaming_result = streaming_mgr.Start();
    if (!streaming_result.success) {
      spdlog::warn("go2rtc initialization failed: {}", streaming_result.error);
    } else {
      spdlog::info("go2rtc restreamer initialized successfully");
    }
  }

  // Initialize storage (databases)
  std::string storageErr;
  if(!ipcam::storage::Init(storageErr)){
    spdlog::error("Storage init failed: {}", storageErr);
    ipcam::config::Shutdown();
    return 3;
  }
  spdlog::info("Storage initialized successfully");

  // Initialize NAS manager for network storage
  spdlog::info("Initializing NAS manager...");
  auto& nas_mgr = ipcam::storage::NasManager::Instance();
  if (!nas_mgr.Init()) {
    spdlog::warn("NAS manager initialization failed - network storage disabled");
  } else {
    spdlog::info("NAS manager initialized successfully");
  }

  // Initialize Event Manager (for event-driven actions like notifications, recording triggers)
  spdlog::info("Initializing Event Manager...");
  auto& event_mgr = ipcam::events::EventManager::Instance();
  if (event_mgr.Init()) {
    // Register all action handlers
    ipcam::events::ActionHandlerFactory::RegisterAll(event_mgr);
    
    // Start the event manager
    event_mgr.Start();
    spdlog::info("Event Manager initialized successfully");

    // Bridge analytics.json action configs → EventRules
    RegisterAnalyticsEventRules(event_mgr);
    
    // Connect analytics engine events to event manager (if analytics is running)
    auto& analytics = ipcam::ai::AnalyticsEngine::Instance();
    if (analytics.IsRunning()) {
      // Motion detection callback  
      analytics.SetMotionCallback([&event_mgr](uint32_t zone_id, bool motion_detected) {
        if (motion_detected) {
          event_mgr.PublishMotionStart({zone_id}, 1.0f);
        } else {
          event_mgr.PublishMotionEnd();
        }
      });
      
      // Line crossing callback
      analytics.SetLineCrossCallback([&event_mgr](uint32_t line_id, uint32_t object_id,
                                                   ipcam::ai::ObjectCategory category) {
        std::string object_class = (category == ipcam::ai::ObjectCategory::kPerson) ? "person" : "vehicle";
        event_mgr.PublishLineCrossed(line_id, "", "any", object_id, object_class);
      });
      
      // Loitering detection callback
      analytics.SetLoiteringCallback([&event_mgr](uint32_t zone_id, uint32_t object_id,
                                                   uint64_t dwell_time_ms) {
        // Publish zone intrusion event with loitering flag (dwell_time > 0)
        event_mgr.PublishZoneIntrusion(zone_id, "", object_id, "object", 
                                       static_cast<int>(dwell_time_ms));
      });

      // Re-set detection callback to include both ONVIF metadata AND person/vehicle events.
      // A set tracks which IDs have already been published to avoid per-frame duplicates.
      auto seen_tracks = std::make_shared<std::unordered_set<uint32_t>>();
      auto seen_mutex  = std::make_shared<std::mutex>();

      analytics.SetDetectionCallback([&event_mgr, seen_tracks, seen_mutex](
                                         const ipcam::ai::DetectionFrame& frame) {
        // --- ONVIF metadata (unchanged) ---
        auto& metadata_gen = ipcam::streaming::OnvifMetadataGenerator::Instance();
        std::vector<ipcam::streaming::OnvifDetectedObject> onvif_objects;
        onvif_objects.reserve(frame.detections.size());
        for (const auto& det : frame.detections) {
          ipcam::streaming::OnvifDetectedObject obj;
          obj.object_id = det.id;
          switch (det.category) {
            case ipcam::ai::ObjectCategory::kPerson:
              obj.object_class = ipcam::streaming::OnvifObjectClass::kHuman;   break;
            case ipcam::ai::ObjectCategory::kVehicle:
            case ipcam::ai::ObjectCategory::kNonMotor:
              obj.object_class = ipcam::streaming::OnvifObjectClass::kVehicle; break;
            case ipcam::ai::ObjectCategory::kFace:
              obj.object_class = ipcam::streaming::OnvifObjectClass::kFace;    break;
            default:
              obj.object_class = ipcam::streaming::OnvifObjectClass::kOther;   break;
          }
          obj.bbox.left   = det.bbox.x1 * 2.0f - 1.0f;
          obj.bbox.top    = det.bbox.y1 * 2.0f - 1.0f;
          obj.bbox.right  = det.bbox.x2 * 2.0f - 1.0f;
          obj.bbox.bottom = det.bbox.y2 * 2.0f - 1.0f;
          obj.confidence  = det.confidence;
          onvif_objects.push_back(std::move(obj));
        }
        if (!onvif_objects.empty()) {
          uint64_t rtp_timestamp = frame.timestamp * 90 / 1000;
          metadata_gen.ReportObjects(onvif_objects, rtp_timestamp);
        }
        if (frame.motion_detected) {
          metadata_gen.ReportMotion(true);
        }

        // --- Person / Vehicle events (first-appearance per track) ---
        {
          std::lock_guard<std::mutex> lk(*seen_mutex);
          // Collect current frame IDs to prune stale tracks
          std::unordered_set<uint32_t> current_ids;
          for (const auto& det : frame.detections) {
            if (det.id == 0) continue;
            current_ids.insert(det.id);
            if (seen_tracks->count(det.id)) continue;  // already published
            seen_tracks->insert(det.id);

            if (det.category == ipcam::ai::ObjectCategory::kPerson) {
              ipcam::events::DetectedObject obj;
              obj.id = det.id;
              obj.class_name = "person";
              obj.confidence = det.confidence;
              obj.x1 = det.bbox.x1; obj.y1 = det.bbox.y1;
              obj.x2 = det.bbox.x2; obj.y2 = det.bbox.y2;
              event_mgr.PublishPersonDetected(obj);
            } else if (det.category == ipcam::ai::ObjectCategory::kVehicle ||
                       det.category == ipcam::ai::ObjectCategory::kNonMotor) {
              ipcam::events::DetectedObject obj;
              obj.id = det.id;
              obj.class_name = "vehicle";
              obj.confidence = det.confidence;
              obj.x1 = det.bbox.x1; obj.y1 = det.bbox.y1;
              obj.x2 = det.bbox.x2; obj.y2 = det.bbox.y2;
              event_mgr.PublishVehicleDetected(obj);
            }
          }
          // Prune tracks that are no longer in the frame
          for (auto it = seen_tracks->begin(); it != seen_tracks->end(); ) {
            if (current_ids.count(*it) == 0)
              it = seen_tracks->erase(it);
            else
              ++it;
          }
        }
      });

      // Tamper detection callback
      analytics.SetTamperCallback([&event_mgr](ipcam::ai::TamperType type, bool active,
                                                const ipcam::ai::TamperResult& result) {
        // Convert TamperType bitfield to readable string
        std::string type_str;
        using TT = ipcam::ai::TamperType;
        if (ipcam::ai::HasFlag(type, TT::kDefocus))     type_str = "defocus";
        else if (ipcam::ai::HasFlag(type, TT::kMasking))     type_str = "masking";
        else if (ipcam::ai::HasFlag(type, TT::kSceneChange)) type_str = "scene_change";
        else if (ipcam::ai::HasFlag(type, TT::kTooDark))     type_str = "too_dark";
        else if (ipcam::ai::HasFlag(type, TT::kTooBright))   type_str = "too_bright";
        else type_str = "unknown";

        event_mgr.PublishTamperEvent(type_str, active, result.laplacian_variance);
      });

      // Face recognition callback
      analytics.SetFaceRecognitionCallback([&event_mgr](const ipcam::ai::FaceResult& face) {
        ipcam::events::DetectedObject obj;
        obj.class_name = "face";
        obj.confidence = face.confidence;
        obj.x1 = face.bbox.x1; obj.y1 = face.bbox.y1;
        obj.x2 = face.bbox.x2; obj.y2 = face.bbox.y2;
        event_mgr.PublishFaceDetected(obj);
      });

      // License plate recognition callback
      analytics.SetLprCallback([&event_mgr](const ipcam::ai::PlateDetection& plate) {
        event_mgr.PublishLprDetected(plate.plate_text, plate.confidence,
                                     plate.x1, plate.y1, plate.x2, plate.y2);
      });

      // Audio classification callback
      analytics.SetAudioEventCallback([&event_mgr](const ipcam::ai::AudioEvent& evt) {
        event_mgr.PublishAudioDetected(evt.class_id, evt.class_name, evt.confidence);
      });
      
      spdlog::info("Event Manager connected to AI Analytics engine (all callbacks wired)");
    }
    
    // Publish system startup event
    event_mgr.PublishSystemEvent(ipcam::events::EventType::kSystemStartup, 
                                 "ipcamd", "IP Camera daemon started", 0);
  } else {
    spdlog::warn("Event Manager initialization failed - event actions disabled");
  }

  // Initialize FTP manager for remote uploads
  spdlog::info("Initializing FTP manager...");
  auto& ftp_mgr = ipcam::storage::FtpManager::Instance();
  if (!ftp_mgr.Init()) {
    spdlog::warn("FTP manager initialization failed - FTP upload disabled");
  } else {
    spdlog::info("FTP manager initialized successfully");
  }

  // Initialize schedule manager for scheduled recording
  spdlog::info("Initializing schedule manager...");
  auto& schedule_mgr = ipcam::recording::ScheduleManager::Instance();
  if (!schedule_mgr.Init()) {
    spdlog::warn("Schedule manager initialization failed - using default schedules");
  } else {
    spdlog::info("Schedule manager initialized successfully");
  }

  spdlog::info("=== VIDEO CONTROL INIT STARTING ===");
  
  if (!ipcam::media::VideoControl::Instance().Init()) {
    spdlog::warn("Video control initialization failed, using defaults");
  } else {
    spdlog::info("Video control initialized successfully");
  }
  
  spdlog::info("=== VIDEO CONTROL INIT COMPLETE ===");

  // Initialize recording service
  spdlog::info("Initializing recording service...");
  auto& recording_service = ipcam::recording::RecordingService::Instance();
  if (!recording_service.Initialize()) {
    spdlog::warn("Recording service initialization returned false");
  }
  if (recording_service.IsStorageAvailable()) {
    spdlog::info("Recording service initialized - SD card available");
  } else {
    spdlog::info("Recording service initialized - SD card not available (recording disabled)");
  }

  // Initialize webserver
  std::string webErr;
  if(!ipcam::webserver::Init(webErr)){
    spdlog::error("Webserver init failed: {}", webErr);
    ipcam::storage::Shutdown();
    ipcam::config::Shutdown();
    return 4;
  }
  spdlog::info("Webserver initialized successfully");

  // Initialize ONVIF module
  std::string onvif_error;
  if(!ipcam::onvif::Init(onvif_error)){
    spdlog::error("ONVIF init failed: {}", onvif_error);
    ipcam::webserver::Shutdown();
    ipcam::storage::Shutdown();
    ipcam::config::Shutdown();
    return 5;
  }
  spdlog::info("ONVIF initialized successfully");

  while(g_running){
    // main loop placeholder
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  spdlog::info("Shutting down...");
  
  // Publish system shutdown event before stopping event manager
  ipcam::events::EventManager::Instance().PublishSystemEvent(
    ipcam::events::EventType::kSystemShutdown, "ipcamd", "IP Camera daemon stopping", 0);
  
  // Shutdown Event Manager (stop processing new events)
  spdlog::info("Shutting down Event Manager...");
  ipcam::events::EventManager::Instance().Stop();
  
  // Shutdown recording service first
  ipcam::recording::RecordingService::Instance().Shutdown();
  
  // Shutdown schedule manager
  ipcam::recording::ScheduleManager::Instance().Shutdown();
  
  // Shutdown FTP manager
  ipcam::storage::FtpManager::Instance().Shutdown();
  
  // Shutdown NAS manager
  ipcam::storage::NasManager::Instance().Shutdown();
  
  // Shutdown AI Analytics engine (before HDAL pipeline)
  ipcam::ai::AnalyticsEngine::Instance().Shutdown();

  // Shutdown SystemLogger (stop collection worker thread)
  spdlog::info("Shutting down SystemLogger...");
  ipcam::platform::SystemLogger::Instance().Shutdown();
  
  // Shutdown IR Control module (turns off IR LED, sets IR cut to day mode)
  spdlog::info("Shutting down IR Control module...");
  ipcam::platform::IRControl::Instance().Shutdown();
  
  // Shutdown Reset Button module
  spdlog::info("Shutting down Reset Button module...");
  ipcam::platform::ResetButton::Instance().Shutdown();
  
  // Shutdown go2rtc FIRST - it connects to RTSP server internally
  // Must stop before RTSP server to close connections cleanly
  spdlog::info("Stopping go2rtc (before RTSP server)...");
  ipcam::streaming::Go2rtcManager::Instance().Stop();
  
  // Shutdown RTSP server (before HDAL pipeline)
  ipcam::streaming::Stop();
  
  // Shutdown MediaHub (stop producer threads, release ring buffers)
  // Must happen after RTSP/recording stop (consumers destroyed) but before HDAL shutdown
  spdlog::info("Shutting down MediaHub...");
  ipcam::media::MediaHub::Instance().Shutdown();
  
  // Shutdown HDAL video pipeline (stops encoders)
  ipcam::platform::HdalPipeline::Instance().Shutdown();
  
  ipcam::onvif::Shutdown();
  ipcam::webserver::Shutdown();
  ipcam::storage::Shutdown();
  ipcam::config::Shutdown();
  spdlog::info("ipcamd exiting");
  
  // Use _Exit() to bypass static destructors which can crash with SQLCipher
  // This is safe because we've already cleaned up all runtime resources above
  _Exit(0);
}
