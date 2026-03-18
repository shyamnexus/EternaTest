#include "ipcam/video_control.h"
#include "ipcam/hdal_pipeline.h"
#include "ipcam/media_hub.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <mutex>
#include <algorithm>
#include <map>

namespace ipcam {
namespace media {

namespace {
    std::mutex g_mutex;

    // Convert VideoCodec enum to VideoCodecType for MediaHub
    VideoCodecType ToVideoCodecType(VideoCodec codec) {
        switch (codec) {
            case VideoCodec::H264:  return VideoCodecType::kH264;
            case VideoCodec::H265:  return VideoCodecType::kH265;
            case VideoCodec::MJPEG: return VideoCodecType::kMJPEG;
            default: return VideoCodecType::kH264;
        }
    }

    bool HasEncoderSettingsChange(const VideoStreamConfig& before,
                                  const VideoStreamConfig& after) {
        return before.enabled != after.enabled ||
               before.audio_enabled != after.audio_enabled ||
               before.stream_type != after.stream_type ||
               before.width != after.width ||
               before.height != after.height ||
               before.fps != after.fps ||
               before.codec != after.codec ||
               before.profile != after.profile ||
               before.level != after.level ||
               before.entropy_mode != after.entropy_mode ||
               before.gop != after.gop ||
               before.idr_interval != after.idr_interval ||
               before.iframe_interval != after.iframe_interval ||
               before.rc_mode != after.rc_mode ||
               before.bitrate != after.bitrate ||
               before.max_bitrate != after.max_bitrate ||
               before.min_bitrate != after.min_bitrate ||
               before.quality != after.quality ||
               before.init_qp != after.init_qp ||
               before.min_qp != after.min_qp ||
               before.max_qp != after.max_qp ||
               before.buffer_size != after.buffer_size ||
               before.buffer_count != after.buffer_count;
    }
}

// ============================================================================
// Singleton Instance
// ============================================================================
VideoControl& VideoControl::Instance() {
    static VideoControl instance;
    return instance;
}

// ============================================================================
// Lifecycle
// ============================================================================
bool VideoControl::Init() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (initialized_) {
        spdlog::warn("VideoControl already initialized");
        return true;
    }

    spdlog::info("Initializing Video Control module");

    // Initialize capabilities
    capabilities_.supported_codecs = {VideoCodec::H264, VideoCodec::H265, VideoCodec::MJPEG};
    capabilities_.supported_resolutions = {
        {2944, 1664}, {2880, 1620}, {2560, 1440}, {2304, 1296}, {1920, 1080},
        {1280, 960}, {1280, 720}, {1024, 768}, {800, 600},
        {640, 480}, {320, 240}
    };
    capabilities_.supported_h264_profiles = {H264Profile::Baseline, H264Profile::Main, H264Profile::High};
    capabilities_.supported_h265_profiles = {H265Profile::Main, H265Profile::Main10};
    capabilities_.supported_levels = {"3.0", "3.1", "4.0", "4.1", "4.2", "5.0", "5.1", "5.2", "6.0", "6.1", "6.2"};
    capabilities_.supported_rc_modes = {RateControlMode::CBR, RateControlMode::VBR, 
                                        RateControlMode::CVBR, RateControlMode::EVBR, RateControlMode::FixQP};
    capabilities_.max_fps = 30;
    capabilities_.min_fps = 1;
    // Keep capability bounds aligned with per-stream limits (kbps)
    capabilities_.max_bitrate = 16000;  // 16 Mbps ceiling
    capabilities_.min_bitrate = 8;       // 8 kbps floor for third stream VBR
    capabilities_.max_gop = 300;
    capabilities_.min_gop = 1;
    
    spdlog::info("Capabilities initialized: bitrate range {} - {} kbps", 
                 capabilities_.min_bitrate, capabilities_.max_bitrate);

    if (!LoadFromConfig()) {
        spdlog::warn("Failed to load video config, using defaults");
    }

    // TODO: Initialize HDAL video encoder paths

    initialized_ = true;
    spdlog::info("Video Control module initialized");
    return true;
}

void VideoControl::Shutdown() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!initialized_) {
        return;
    }

    spdlog::info("Shutting down Video Control module");
    // TODO: Release HDAL resources
    initialized_ = false;
}

// ============================================================================
// Stream Configuration
// ============================================================================
bool VideoControl::SetStreamConfig(const std::string& stream_id, const VideoStreamConfig& config) {
    std::lock_guard<std::mutex> lock(g_mutex);

    spdlog::info("SetStreamConfig ENTRY: stream_id={}, config.bitrate={} bps", stream_id, config.bitrate);

    if (!ValidateConfig(config)) {
        spdlog::error("Invalid video stream config for {}", stream_id);
        return false;
    }

    VideoStreamConfig* target = nullptr;
    std::string config_prefix;
    
    if (stream_id == "video1") {
        target = &video1_;
        config_prefix = "media.video1";
    } else if (stream_id == "video2") {
        target = &video2_;
        config_prefix = "media.video2";
    } else if (stream_id == "video3") {
        target = &video3_;
        config_prefix = "media.video3";
    } else {
        spdlog::error("Unknown stream ID: {}", stream_id);
        return false;
    }

    VideoStreamConfig previous = *target;
    *target = config;
    target->stream_id = stream_id;

    spdlog::info("SetStreamConfig: IMMEDIATELY after assignment - {} bitrate is {} bps", stream_id, target->bitrate);
    spdlog::info("SetStreamConfig: video1_.bitrate = {}", video1_.bitrate);

    spdlog::info("SetStreamConfig: Updated internal state - {} bitrate now {} bps", stream_id, target->bitrate);

    // Save to config
    config::Set<bool>(config_prefix + ".enabled", config.enabled);
    config::Set<bool>(config_prefix + ".audio_enabled", config.audio_enabled);
    config::Set<std::string>(config_prefix + ".stream_type", config.stream_type);
    config::Set<int>(config_prefix + ".width", config.width);
    config::Set<int>(config_prefix + ".height", config.height);
    config::Set<int>(config_prefix + ".fps", config.fps);
    config::Set<std::string>(config_prefix + ".codec", VideoCodecToString(config.codec));
    config::Set<std::string>(config_prefix + ".profile", H264ProfileToString(config.profile));
    config::Set<std::string>(config_prefix + ".level", config.level);
    config::Set<int>(config_prefix + ".gop", config.gop);
    config::Set<int>(config_prefix + ".idr_interval", config.idr_interval);
    config::Set<int>(config_prefix + ".iframe_interval", config.iframe_interval);
    config::Set<std::string>(config_prefix + ".rc_mode", RateControlModeToString(config.rc_mode));
    spdlog::info("SetStreamConfig: Saving bitrate={} kbps to config", config.bitrate);
    config::Set<int>(config_prefix + ".bitrate", config.bitrate);  // kbps
    config::Set<int>(config_prefix + ".max_bitrate", config.max_bitrate);  // kbps
    config::Set<int>(config_prefix + ".min_bitrate", config.min_bitrate);  // kbps
    config::Set<int>(config_prefix + ".quality", config.quality);
    config::Set<int>(config_prefix + ".init_qp", config.init_qp);
    config::Set<int>(config_prefix + ".min_qp", config.min_qp);
    config::Set<int>(config_prefix + ".max_qp", config.max_qp);
    config::Set<std::string>(config_prefix + ".entropy_mode", EntropyModeToString(config.entropy_mode));

    spdlog::info("Video stream {} config updated: {}x{} @ {}fps, {} {}", 
                 stream_id, config.width, config.height, config.fps,
                 VideoCodecToString(config.codec), RateControlModeToString(config.rc_mode));

    // Persist to storage
    config::Save();

    // Apply to HDAL pipeline if running
    ApplyToHardware(stream_id, config, previous);

    return true;
}

VideoStreamConfig VideoControl::GetStreamConfig(const std::string& stream_id) const {
    if (stream_id == "video1") return video1_;
    if (stream_id == "video2") return video2_;
    if (stream_id == "video3") return video3_;
    
    spdlog::warn("Unknown stream ID: {}, returning video1", stream_id);
    return video1_;
}

std::vector<std::string> VideoControl::GetStreamIds() const {
    return {"video1", "video2", "video3"};
}

int VideoControl::GetStreamCount() const {
    return 3;
}

// ============================================================================
// Individual Parameter Setters
// ============================================================================
bool VideoControl::SetResolution(const std::string& stream_id, int width, int height) {
    auto config = GetStreamConfig(stream_id);
    config.width = width;
    config.height = height;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetFrameRate(const std::string& stream_id, int fps) {
    auto config = GetStreamConfig(stream_id);
    config.fps = fps;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetCodec(const std::string& stream_id, VideoCodec codec) {
    auto config = GetStreamConfig(stream_id);
    config.codec = codec;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetProfile(const std::string& stream_id, H264Profile profile) {
    auto config = GetStreamConfig(stream_id);
    config.profile = profile;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetLevel(const std::string& stream_id, const std::string& level) {
    auto config = GetStreamConfig(stream_id);
    config.level = level;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetGOP(const std::string& stream_id, int gop) {
    auto config = GetStreamConfig(stream_id);
    config.gop = gop;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetRateControl(const std::string& stream_id, RateControlMode mode, int bitrate) {
    auto config = GetStreamConfig(stream_id);
    config.rc_mode = mode;
    config.bitrate = bitrate;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetBitrate(const std::string& stream_id, int bitrate) {
    auto config = GetStreamConfig(stream_id);
    config.bitrate = bitrate;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetQuality(const std::string& stream_id, int quality) {
    auto config = GetStreamConfig(stream_id);
    config.quality = quality;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetQP(const std::string& stream_id, int init_qp, int min_qp, int max_qp) {
    auto config = GetStreamConfig(stream_id);
    config.init_qp = init_qp;
    config.min_qp = min_qp;
    config.max_qp = max_qp;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::SetEntropyMode(const std::string& stream_id, EntropyMode mode) {
    auto config = GetStreamConfig(stream_id);
    config.entropy_mode = mode;
    return SetStreamConfig(stream_id, config);
}

bool VideoControl::EnableStream(const std::string& stream_id, bool enabled) {
    auto config = GetStreamConfig(stream_id);
    config.enabled = enabled;
    return SetStreamConfig(stream_id, config);
}

// ============================================================================
// Snapshot
// ============================================================================
bool VideoControl::SetSnapshotConfig(const SnapshotConfig& config) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (config.quality < 1 || config.quality > 100) {
        spdlog::error("Invalid snapshot quality: must be 1-100");
        return false;
    }

    snapshot_ = config;

    config::Set<bool>("media.snapshot.enabled", config.enabled);
    config::Set<int>("media.snapshot.width", config.width);
    config::Set<int>("media.snapshot.height", config.height);
    config::Set<int>("media.snapshot.quality", config.quality);
    config::Save();

    spdlog::debug("Snapshot config updated: {}x{} quality={}", 
                  config.width, config.height, config.quality);
    return true;
}

SnapshotConfig VideoControl::GetSnapshotConfig() const {
    return snapshot_;
}

std::vector<uint8_t> VideoControl::CaptureSnapshot() {
    spdlog::info("CaptureSnapshot: Triggering snapshot on main stream (quality={})", snapshot_.quality);
    
    std::vector<uint8_t> buffer;
    bool success = platform::HdalPipeline::Instance().CaptureSnapshot(0, snapshot_.quality, buffer);
    
    if (!success) {
        spdlog::warn("CaptureSnapshot: Failed on stream 0, trying stream 1 (Sub Stream)...");
        success = platform::HdalPipeline::Instance().CaptureSnapshot(1, snapshot_.quality, buffer);
    }

    if (!success) {
        spdlog::error("CaptureSnapshot: Failed to capture snapshot from any stream");
        return {};
    }
    
    spdlog::info("CaptureSnapshot: Got {} bytes JPEG data", buffer.size());
    return buffer;
}

// ============================================================================
// Capabilities
// ============================================================================
VideoCapabilities VideoControl::GetCapabilities() const {
    spdlog::info("GetCapabilities called: returning bitrate range {} - {} kbps",
                 capabilities_.min_bitrate, capabilities_.max_bitrate);
    return capabilities_;
}

// ============================================================================
// Stream Control
// ============================================================================
bool VideoControl::RestartStream(const std::string& stream_id) {
    spdlog::info("Restarting video stream: {}", stream_id);
    // TODO: Implement HDAL stream restart
    return true;
}

bool VideoControl::RestartAllStreams() {
    spdlog::info("Restarting all video streams");
    bool success = true;
    for (const auto& id : GetStreamIds()) {
        if (!RestartStream(id)) {
            success = false;
        }
    }
    return success;
}

bool VideoControl::IsStreamActive(const std::string& stream_id) const {
    auto config = GetStreamConfig(stream_id);
    return config.enabled;
}

// ============================================================================
// Config Operations
// ============================================================================
bool VideoControl::LoadFromConfig() {
    auto loadStream = [](const std::string& prefix, VideoStreamConfig& cfg) {
        cfg.enabled = config::Get<bool>(prefix + ".enabled", true);
        cfg.audio_enabled = config::Get<bool>(prefix + ".audio_enabled", true);
        cfg.stream_type = config::Get<std::string>(prefix + ".stream_type", "mainStream");
        cfg.width = config::Get<int>(prefix + ".width", 1920);
        cfg.height = config::Get<int>(prefix + ".height", 1080);
        cfg.fps = config::Get<int>(prefix + ".fps", 30);
        cfg.codec = StringToVideoCodec(config::Get<std::string>(prefix + ".codec", "h264"));
        cfg.profile = StringToH264Profile(config::Get<std::string>(prefix + ".profile", "main"));
        cfg.level = config::Get<std::string>(prefix + ".level", "4.1");
        cfg.gop = config::Get<int>(prefix + ".gop", 50);
        cfg.idr_interval = config::Get<int>(prefix + ".idr_interval", 1);
        cfg.iframe_interval = config::Get<int>(prefix + ".iframe_interval", 50);
        cfg.rc_mode = StringToRateControlMode(config::Get<std::string>(prefix + ".rc_mode", "CBR"));
        cfg.bitrate = config::Get<int>(prefix + ".bitrate", 4096);  // kbps
        cfg.max_bitrate = config::Get<int>(prefix + ".max_bitrate", 6144);  // kbps
        cfg.min_bitrate = config::Get<int>(prefix + ".min_bitrate", 1024);  // kbps
        cfg.quality = config::Get<int>(prefix + ".quality", 50);
        cfg.init_qp = config::Get<int>(prefix + ".init_qp", 26);
        cfg.min_qp = config::Get<int>(prefix + ".min_qp", 10);
        cfg.max_qp = config::Get<int>(prefix + ".max_qp", 51);
        cfg.entropy_mode = StringToEntropyMode(config::Get<std::string>(prefix + ".entropy_mode", "cabac"));
        cfg.buffer_size = config::Get<int>(prefix + ".buffer_size", 0);
        cfg.buffer_count = config::Get<int>(prefix + ".buffer_count", 4);
    };

    video1_.stream_id = "video1";
    video2_.stream_id = "video2";
    video3_.stream_id = "video3";

    loadStream("media.video1", video1_);
    loadStream("media.video2", video2_);
    loadStream("media.video3", video3_);

    spdlog::info("LoadFromConfig: video1 bitrate={} kbps, video2 bitrate={} kbps, video3 bitrate={} kbps",
                 video1_.bitrate, video2_.bitrate, video3_.bitrate);

    // Load snapshot config
    snapshot_.enabled = config::Get<bool>("media.snapshot.enabled", true);
    snapshot_.width = config::Get<int>("media.snapshot.width", 1920);
    snapshot_.height = config::Get<int>("media.snapshot.height", 1080);
    snapshot_.quality = config::Get<int>("media.snapshot.quality", 85);

    spdlog::info("Video config loaded from file");
    return true;
}

bool VideoControl::SaveToConfig() {
    spdlog::info("SaveToConfig called - video1 bitrate in memory: {}", video1_.bitrate);
    bool result = config::Save();
    spdlog::info("SaveToConfig result: {}", result);
    return result;
}

bool VideoControl::ResetToDefaults() {
    std::lock_guard<std::mutex> lock(g_mutex);

    spdlog::info("Resetting video settings to defaults");

    // Reset video1 (main stream)
    video1_ = VideoStreamConfig{};
    video1_.stream_id = "video1";
    video1_.stream_type = "mainStream";
    video1_.width = 2944;
    video1_.height = 1664;
    video1_.fps = 30;
    video1_.codec = VideoCodec::H265;
    video1_.profile = H264Profile::Main;
    video1_.level = "5.0";
    video1_.rc_mode = RateControlMode::CBR;
    video1_.bitrate = 6144;  // kbps default 6 Mbps
    video1_.max_bitrate = 16384;  // kbps allow up to 16 Mbps
    video1_.min_bitrate = 1024;
    video1_.iframe_interval = 50;
    video1_.gop = 30;
    video1_.audio_enabled = true;

    // Reset video2 (sub stream)
    video2_ = VideoStreamConfig{};
    video2_.stream_id = "video2";
    video2_.stream_type = "subStream";
    video2_.width = 1280;
    video2_.height = 720;
    video2_.fps = 30;
    video2_.codec = VideoCodec::H264;
    video2_.profile = H264Profile::Main;
    video2_.level = "4.1";
    video2_.rc_mode = RateControlMode::CBR;
    video2_.bitrate = 1024;  // kbps
    video2_.max_bitrate = 4096;
    video2_.min_bitrate = 128;
    video2_.iframe_interval = 30;
    video2_.gop = 30;
    video2_.audio_enabled = true;

    // Reset video3 (third stream)
    video3_ = VideoStreamConfig{};
    video3_.stream_id = "video3";
    video3_.stream_type = "thirdStream";
    video3_.width = 640;
    video3_.height = 480;
    video3_.fps = 30;
    video3_.codec = VideoCodec::H264;
    video3_.profile = H264Profile::Main;
    video3_.level = "3.1";
    video3_.rc_mode = RateControlMode::CBR;
    video3_.bitrate = 512;  // kbps
    video3_.max_bitrate = 2048;
    video3_.min_bitrate = 8;
    video3_.iframe_interval = 30;
    video3_.gop = 30;
    video3_.audio_enabled = true;

    // Reset snapshot
    snapshot_ = SnapshotConfig{};

    return SaveToConfig();
}

bool VideoControl::ValidateConfig(const VideoStreamConfig& config) const {
    if (config.width < 320 || config.width > 3840 ||
        config.height < 240 || config.height > 2160) {
        spdlog::error("Invalid resolution: {}x{}", config.width, config.height);
        return false;
    }

    if (config.fps < capabilities_.min_fps || config.fps > capabilities_.max_fps) {
        spdlog::error("Invalid FPS: {} (range: {}-{})", 
                      config.fps, capabilities_.min_fps, capabilities_.max_fps);
        return false;
    }

    if (config.bitrate < capabilities_.min_bitrate || config.bitrate > capabilities_.max_bitrate) {
        spdlog::error("Invalid bitrate: {} (range: {}-{})", 
                      config.bitrate, capabilities_.min_bitrate, capabilities_.max_bitrate);
        return false;
    }

    if (config.gop < capabilities_.min_gop || config.gop > capabilities_.max_gop) {
        spdlog::error("Invalid GOP: {} (range: {}-{})", 
                      config.gop, capabilities_.min_gop, capabilities_.max_gop);
        return false;
    }

    if (config.init_qp < 0 || config.init_qp > 51 ||
        config.min_qp < 0 || config.min_qp > 51 ||
        config.max_qp < 0 || config.max_qp > 51) {
        spdlog::error("Invalid QP values");
        return false;
    }

    if (config.min_qp > config.max_qp) {
        spdlog::error("min_qp ({}) cannot be greater than max_qp ({})", 
                      config.min_qp, config.max_qp);
        return false;
    }

    return true;
}

// ============================================================================
// Helper Function Implementations
// ============================================================================
std::string VideoCodecToString(VideoCodec codec) {
    switch (codec) {
        case VideoCodec::H264: return "h264";
        case VideoCodec::H265: return "h265";
        case VideoCodec::MJPEG: return "mjpeg";
        default: return "h264";
    }
}

VideoCodec StringToVideoCodec(const std::string& str) {
    if (str == "h265" || str == "hevc") return VideoCodec::H265;
    if (str == "mjpeg" || str == "jpeg") return VideoCodec::MJPEG;
    return VideoCodec::H264;
}

std::string H264ProfileToString(H264Profile profile) {
    switch (profile) {
        case H264Profile::Baseline: return "baseline";
        case H264Profile::Main: return "main";
        case H264Profile::High: return "high";
        default: return "main";
    }
}

H264Profile StringToH264Profile(const std::string& str) {
    if (str == "baseline") return H264Profile::Baseline;
    if (str == "high") return H264Profile::High;
    return H264Profile::Main;
}

std::string H265ProfileToString(H265Profile profile) {
    switch (profile) {
        case H265Profile::Main: return "main";
        case H265Profile::Main10: return "main10";
        default: return "main";
    }
}

H265Profile StringToH265Profile(const std::string& str) {
    if (str == "main10") return H265Profile::Main10;
    return H265Profile::Main;
}

std::string RateControlModeToString(RateControlMode mode) {
    switch (mode) {
        case RateControlMode::CBR: return "CBR";
        case RateControlMode::VBR: return "VBR";
        case RateControlMode::CVBR: return "CVBR";
        case RateControlMode::EVBR: return "EVBR";
        case RateControlMode::FixQP: return "FixQP";
        default: return "CBR";
    }
}

RateControlMode StringToRateControlMode(const std::string& str) {
    if (str == "VBR") return RateControlMode::VBR;
    if (str == "CVBR") return RateControlMode::CVBR;
    if (str == "EVBR") return RateControlMode::EVBR;
    if (str == "FixQP") return RateControlMode::FixQP;
    return RateControlMode::CBR;
}

std::string EntropyModeToString(EntropyMode mode) {
    switch (mode) {
        case EntropyMode::CAVLC: return "cavlc";
        case EntropyMode::CABAC: return "cabac";
        default: return "cabac";
    }
}

EntropyMode StringToEntropyMode(const std::string& str) {
    if (str == "cavlc") return EntropyMode::CAVLC;
    return EntropyMode::CABAC;
}

// ============================================================================
// Callback Registration
// ============================================================================
void VideoControl::SetCodecChangeCallback(CodecChangeCallback callback) {
    codec_change_callback_ = std::move(callback);
    spdlog::info("VideoControl: Codec change callback registered");
}

void VideoControl::SetStreamSettingsChangeCallback(StreamSettingsChangeCallback callback) {
    stream_settings_change_callback_ = std::move(callback);
    spdlog::info("VideoControl: Stream settings change callback registered");
}

// ============================================================================
// Hardware Integration
// ============================================================================
void VideoControl::ApplyToHardware(const std::string& stream_id,
                                   const VideoStreamConfig& config,
                                   const VideoStreamConfig& previous_config) {
    spdlog::info(">>> ApplyToHardware CALLED: stream_id={}, bitrate={}, fps={}, gop={}", 
                 stream_id, config.bitrate, config.fps, config.gop);
    
    auto& pipeline = platform::HdalPipeline::Instance();
    
    // Map video1/video2/video3 to stream indices 0/1/2
    int stream_idx = -1;
    VideoCodec old_codec = VideoCodec::H264;
    if (stream_id == "video1") {
        stream_idx = 0;
        old_codec = video1_.codec;
    } else if (stream_id == "video2") {
        stream_idx = 1;
        old_codec = video2_.codec;
    } else if (stream_id == "video3") {
        stream_idx = 2;
        old_codec = video3_.codec;
    } else {
        spdlog::error("ApplyToHardware: Invalid stream_id: {}", stream_id);
        return;
    }
    
    const bool settings_changed = HasEncoderSettingsChange(previous_config, config);
    const bool fps_changed = (config.fps != previous_config.fps);
    const bool iframe_changed = (config.iframe_interval != previous_config.iframe_interval);

    // Note: At this point config already has the NEW codec (target was updated in SetStreamConfig)
    // We need to get the old codec from the HDAL pipeline or track it differently
    // For now, we'll compare config.codec with what the pipeline currently has
    
    spdlog::info("ApplyToHardware: {} mapped to idx={}, pipeline.IsRunning()={}", 
                 stream_id, stream_idx, pipeline.IsRunning());
    
    spdlog::info("ApplyToHardware: {} (idx={}) bitrate={}, fps={}, gop={}, codec={}, enabled={}", 
                 stream_id, stream_idx, config.bitrate, config.fps, config.gop,
                 VideoCodecToString(config.codec), config.enabled);

    // Handle disable: pause MediaHub first, then stop encoder/proc
    if (!config.enabled) {
        spdlog::info("ApplyToHardware: Disabling stream {} (idx={})", stream_id, stream_idx);
        // Pause the MediaHub producer BEFORE stopping the encoder to avoid
        // a flood of pull errors that would eventually self-terminate the producer.
        auto& media_hub_dis = MediaHub::Instance();
        if (media_hub_dis.IsInitialized() && media_hub_dis.IsChannelActive(stream_idx)) {
            spdlog::info("ApplyToHardware: Pausing MediaHub channel {} before disable", stream_idx);
            media_hub_dis.PauseChannel(stream_idx);
        }
        if (!pipeline.SetStreamEnabled(stream_idx, false)) {
            spdlog::error("ApplyToHardware: Failed to disable stream {}", stream_id);
        }
        // Notify RTSP/streaming about setting change
        if (settings_changed && stream_settings_change_callback_) {
            stream_settings_change_callback_(stream_idx, previous_config, config);
        }
        return;
    }

    // Ensure stream is enabled at pipeline level before applying settings.
    // If the stream was previously disabled, SetStreamEnabled(true) triggers a
    // full ReinitWithConfig() which tears down and rebuilds the entire HDAL
    // pipeline.  All encoder path handles change, so MediaHub producers that
    // reference the old handles will fail and self-terminate.
    const bool stream_was_disabled = !previous_config.enabled;
    if (!pipeline.SetStreamEnabled(stream_idx, true)) {
        spdlog::error("ApplyToHardware: Failed to enable stream {} prior to apply", stream_id);
        return;
    }

    // After a full pipeline reinit, reinitialize MediaHub so its producer
    // threads pick up the new encoder path handles for ALL channels.
    if (stream_was_disabled) {
        auto& media_hub_reinit = MediaHub::Instance();
        if (media_hub_reinit.IsInitialized()) {
            spdlog::info("ApplyToHardware: Stream {} re-enabled after disable, "
                         "reinitializing MediaHub for new pipeline paths", stream_id);
            media_hub_reinit.Reinitialize();
        }
    }
    
    // Check if codec is changing (compare with what HDAL pipeline currently has)
    std::string current_codec_str = pipeline.GetStreamCodec(stream_idx);
    VideoCodec current_hw_codec = StringToVideoCodec(current_codec_str);
    bool codec_changing = (current_hw_codec != config.codec);
    
    if (codec_changing) {
        spdlog::info("ApplyToHardware: Codec changing from {} to {} for stream {}", 
                     VideoCodecToString(current_hw_codec), VideoCodecToString(config.codec), stream_id);
    }

    // Note: Stream 0 resolution changes are handled by SetStreamResolution()
    // just like streams 1,2. The videoproc downscales from the full sensor
    // crop (2944x1664) to whatever output size is requested. We do NOT
    // change the sensor crop — that would affect ALL streams.
    
    // For now, always try to apply changes even if pipeline reports not running
    // The HDAL APIs should handle this gracefully
    bool success = true;

    // ==========================================================================
    // Pause MediaHub producer BEFORE any encoder stop/start operations
    // This prevents the producer thread from pulling during encoder transitions
    // ==========================================================================
    auto& media_hub = MediaHub::Instance();
    if (media_hub.IsInitialized() && media_hub.IsChannelActive(stream_idx)) {
        spdlog::info("ApplyToHardware: Pausing MediaHub channel {} for encoder changes", stream_idx);
        media_hub.PauseChannel(stream_idx);
    }
    
    // Apply codec change (must be done before other params as it reconfigures encoder)
    std::string codec_str = VideoCodecToString(config.codec);
    std::string profile_str = H264ProfileToString(config.profile);
    success &= pipeline.SetStreamCodec(stream_idx, codec_str, profile_str);
    
    // Apply rate control parameters
    success &= pipeline.SetStreamBitrate(stream_idx, config.bitrate * 1000);  // Convert kbps to bps for HDAL

    // Encoder FPS update (per-encoder rate control only; capture/proc FRC is
    // pass-through and shared by all streams, so we don't touch it here)
    success &= pipeline.SetStreamFps(stream_idx, config.fps);

    // Treat iframe_interval as GOP size; keep idr_interval separate (not supported by driver yet)
    int target_gop = config.iframe_interval > 0 ? config.iframe_interval : config.gop;
    if (config.idr_interval > 0 && config.idr_interval != target_gop) {
        spdlog::warn("ApplyToHardware: idr_interval={} differs from iframe_interval/gop={}, driver only supports GOP; using {}",
                     config.idr_interval, target_gop, target_gop);
    }
    success &= pipeline.SetStreamGop(stream_idx, target_gop);
    success &= pipeline.SetStreamResolution(stream_idx, config.width, config.height);
    success &= pipeline.SetStreamQpRange(stream_idx, config.min_qp, config.max_qp);
    
    spdlog::info("ApplyToHardware for {} completed: {}", stream_id, success ? "SUCCESS" : "FAILED");
    
    // ==========================================================================
    // Resume MediaHub producer AFTER all encoder changes are done
    // This clears SPS/PPS cache, flushes stale frames, and requests IDR
    // ==========================================================================
    if (media_hub.IsInitialized() && media_hub.IsChannelActive(stream_idx)) {
        spdlog::info("ApplyToHardware: Resuming MediaHub channel {} (codec_changed={})", stream_idx, codec_changing);
        media_hub.ResumeChannel(stream_idx, codec_changing);
    }
    
    // Notify streaming module if codec changed
    if (codec_changing && success && codec_change_callback_) {
        spdlog::info("ApplyToHardware: Notifying RTSP server of codec change for stream idx={}", stream_idx);
        codec_change_callback_(stream_idx, current_hw_codec, config.codec);
    }

    if (settings_changed && success && stream_settings_change_callback_) {
        spdlog::info("ApplyToHardware: Notifying RTSP server of stream settings change for idx={}", stream_idx);
        stream_settings_change_callback_(stream_idx, previous_config, config);
    }
}

} // namespace media
} // namespace ipcam
