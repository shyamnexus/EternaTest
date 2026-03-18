#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <functional>

namespace ipcam {
namespace media {

// ============================================================================
// Video Codec Types
// ============================================================================
enum class VideoCodec {
    H264,
    H265,
    MJPEG
};

/**
 * @brief Callback for codec changes
 * @param stream_idx Stream index (0, 1, 2)
 * @param old_codec Previous codec
 * @param new_codec New codec
 */
using CodecChangeCallback = std::function<void(int stream_idx, VideoCodec old_codec, VideoCodec new_codec)>;

enum class H264Profile {
    Baseline,
    Main,
    High
};

enum class H265Profile {
    Main,
    Main10
};

enum class RateControlMode {
    CBR,
    VBR,
    CVBR,
    EVBR,
    FixQP
};

enum class EntropyMode {
    CAVLC,
    CABAC
};

// ============================================================================
// Video Stream Configuration
// ============================================================================
struct VideoStreamConfig {
    std::string stream_id;
    bool enabled = true;
    bool audio_enabled = true;
    std::string stream_type = "mainStream";
    
    // Resolution
    int width = 1920;
    int height = 1080;
    int fps = 30;
    
    // Codec settings
    VideoCodec codec = VideoCodec::H264;
    H264Profile profile = H264Profile::Main;
    std::string level = "4.1";
    EntropyMode entropy_mode = EntropyMode::CABAC;
    
    // GOP settings
    int gop = 50;
    int idr_interval = 1;
    int iframe_interval = 50;
    
    // Rate control
    RateControlMode rc_mode = RateControlMode::CBR;
    int bitrate = 6144;         // kbps (default 6 Mbps main stream)
    int max_bitrate = 16000;    // kbps (allow up to 16 Mbps)
    int min_bitrate = 1024;     // kbps
    int quality = 50;           // 1-100 for VBR quality
    
    // QP settings
    int init_qp = 26;
    int min_qp = 10;
    int max_qp = 51;
    
    // Buffer settings
    int buffer_size = 0;
    int buffer_count = 4;
};

/**
 * @brief Callback for stream setting changes
 * @param stream_idx Stream index (0, 1, 2)
 * @param old_config Previous stream configuration
 * @param new_config New stream configuration
 */
using StreamSettingsChangeCallback =
    std::function<void(int stream_idx, const VideoStreamConfig& old_config,
                       const VideoStreamConfig& new_config)>;

// ============================================================================
// Snapshot Configuration
// ============================================================================
struct SnapshotConfig {
    bool enabled = true;
    int width = 1920;
    int height = 1080;
    int quality = 85;           // JPEG quality 1-100
    VideoCodec codec = VideoCodec::MJPEG;
};

// ============================================================================
// Video Capabilities
// ============================================================================
struct VideoCapabilities {
    std::vector<VideoCodec> supported_codecs;
    std::vector<std::pair<int, int>> supported_resolutions;
    std::vector<H264Profile> supported_h264_profiles;
    std::vector<H265Profile> supported_h265_profiles;
    std::vector<std::string> supported_levels;
    std::vector<RateControlMode> supported_rc_modes;
    int max_fps = 60;
    int min_fps = 1;
    int max_bitrate = 16384000;    // bps (16 Mbps)
    int min_bitrate = 8000;        // bps (8 kbps)
    int max_gop = 300;
    int min_gop = 1;
};

// ============================================================================
// Video Control Interface
// ============================================================================
class VideoControl {
public:
    static VideoControl& Instance();

    // Lifecycle
    bool Init();
    void Shutdown();
    bool IsInitialized() const { return initialized_; }

    // Stream configuration
    bool SetStreamConfig(const std::string& stream_id, const VideoStreamConfig& config);
    VideoStreamConfig GetStreamConfig(const std::string& stream_id) const;
    std::vector<std::string> GetStreamIds() const;
    int GetStreamCount() const;

    // Individual parameter setters
    bool SetResolution(const std::string& stream_id, int width, int height);
    bool SetFrameRate(const std::string& stream_id, int fps);
    bool SetCodec(const std::string& stream_id, VideoCodec codec);
    bool SetProfile(const std::string& stream_id, H264Profile profile);
    bool SetLevel(const std::string& stream_id, const std::string& level);
    bool SetGOP(const std::string& stream_id, int gop);
    bool SetRateControl(const std::string& stream_id, RateControlMode mode, int bitrate);
    bool SetBitrate(const std::string& stream_id, int bitrate);
    bool SetQuality(const std::string& stream_id, int quality);
    bool SetQP(const std::string& stream_id, int init_qp, int min_qp, int max_qp);
    bool SetEntropyMode(const std::string& stream_id, EntropyMode mode);
    bool EnableStream(const std::string& stream_id, bool enabled);

    // Snapshot
    bool SetSnapshotConfig(const SnapshotConfig& config);
    SnapshotConfig GetSnapshotConfig() const;
    std::vector<uint8_t> CaptureSnapshot();

    // Capabilities
    VideoCapabilities GetCapabilities() const;

    // Stream control
    bool RestartStream(const std::string& stream_id);
    bool RestartAllStreams();
    bool IsStreamActive(const std::string& stream_id) const;

    // Config persistence
    bool SaveToConfig();
    bool ResetToDefaults();

    /**
     * @brief Set callback for codec changes
     * @param callback Function to call when codec changes
     * 
     * The callback will be invoked after a codec change is applied to hardware.
     * This allows the streaming module to refresh RTSP sessions when codec changes.
     */
    void SetCodecChangeCallback(CodecChangeCallback callback);

    /**
     * @brief Set callback for stream setting changes
     * @param callback Function to call when stream settings change
     *
     * The callback will be invoked after changes are applied to hardware.
     */
    void SetStreamSettingsChangeCallback(StreamSettingsChangeCallback callback);

private:
    VideoControl() = default;
    ~VideoControl() = default;
    VideoControl(const VideoControl&) = delete;
    VideoControl& operator=(const VideoControl&) = delete;

    bool LoadFromConfig();
    void ApplyToHardware(const std::string& stream_id,
                         const VideoStreamConfig& config,
                         const VideoStreamConfig& previous_config);
    bool ValidateConfig(const VideoStreamConfig& config) const;

    bool initialized_ = false;
    VideoStreamConfig video1_;
    VideoStreamConfig video2_;
    VideoStreamConfig video3_;
    SnapshotConfig snapshot_;
    VideoCapabilities capabilities_;
    CodecChangeCallback codec_change_callback_;
    StreamSettingsChangeCallback stream_settings_change_callback_;
};

// ============================================================================
// Helper Functions
// ============================================================================
std::string VideoCodecToString(VideoCodec codec);
VideoCodec StringToVideoCodec(const std::string& str);

std::string H264ProfileToString(H264Profile profile);
H264Profile StringToH264Profile(const std::string& str);

std::string H265ProfileToString(H265Profile profile);
H265Profile StringToH265Profile(const std::string& str);

std::string RateControlModeToString(RateControlMode mode);
RateControlMode StringToRateControlMode(const std::string& str);

std::string EntropyModeToString(EntropyMode mode);
EntropyMode StringToEntropyMode(const std::string& str);

} // namespace media
} // namespace ipcam
