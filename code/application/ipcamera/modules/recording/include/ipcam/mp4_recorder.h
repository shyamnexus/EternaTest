/**
 * @file mp4_recorder.h
 * @brief MP4 Video Recorder using minimp4 library
 * 
 * Provides C++ wrapper for minimp4 to record H.264/H.265 video with AAC audio
 * to MP4 container format with proper timestamps and seeking support.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <cstdio>

// Include minimp4 for complete type definitions
// (needed by unique_ptr for proper destruction)
#include "minimp4.h"

namespace ipcam {
namespace recording {

/**
 * Video codec type
 */
enum class VideoCodec {
    H264,
    H265
};

/**
 * Audio codec type
 */
enum class AudioCodec {
    AAC,       // AAC-LC (NOT SUPPORTED - SDK lacks encoder)
    PCMU,      // G.711 μ-law (8kHz, 8-bit, RTP payload 0)
    PCMA,      // G.711 A-law (8kHz, 8-bit, RTP payload 8)
    PCM,       // Uncompressed PCM (s16le, configurable sample rate)
    G726_16,   // G.726 ADPCM 16 kbps (2 bits/sample)
    G726_24,   // G.726 ADPCM 24 kbps (3 bits/sample)
    G726_32,   // G.726 ADPCM 32 kbps (4 bits/sample) - standard
    G726_40    // G.726 ADPCM 40 kbps (5 bits/sample)
};

/**
 * MP4 recorder configuration
 */
struct MP4RecorderConfig {
    std::string filename;
    
    // Video parameters
    VideoCodec video_codec = VideoCodec::H264;
    int width = 1920;
    int height = 1080;
    int fps = 30;
    int video_timescale = 90000;  // Standard for H.264/H.265
    
    // Audio parameters (optional)
    bool enable_audio = true;
    AudioCodec audio_codec = AudioCodec::AAC;
    int audio_sample_rate = 48000;
    int audio_channels = 1;
    int audio_timescale = 48000;
    
    // MP4 options
    bool fragmented = true;  // Enable fragmented MP4 (power-safe)
    int fragment_duration_ms = 2000;  // 2 seconds per fragment
};

/**
 * NAL unit type for H.264/H.265
 */
enum class NalUnitType {
    Unknown,
    SPS,      // Sequence Parameter Set
    PPS,      // Picture Parameter Set
    VPS,      // Video Parameter Set (H.265 only)
    IDR,      // Instantaneous Decoder Refresh (keyframe)
    NonIDR,   // Non-keyframe
    SEI       // Supplemental Enhancement Information
};

/**
 * MP4 Recorder class
 * Wraps minimp4 for easy C++ usage with HDAL video encoder
 */
class MP4Recorder {
public:
    MP4Recorder() = default;
    ~MP4Recorder();
    
    // Prevent copy, allow move
    MP4Recorder(const MP4Recorder&) = delete;
    MP4Recorder& operator=(const MP4Recorder&) = delete;
    MP4Recorder(MP4Recorder&&) noexcept;
    MP4Recorder& operator=(MP4Recorder&&) noexcept;
    
    /**
     * Open MP4 file for recording
     * @param config Recording configuration
     * @return true on success
     */
    bool Open(const MP4RecorderConfig& config);
    
    /**
     * Close MP4 file
     * Finalizes file with proper index and metadata
     */
    void Close();
    
    /**
     * Check if recorder is open
     */
    bool IsOpen() const { return file_ != nullptr; }
    
    /**
     * Write H.264/H.265 NAL units
     * Automatically handles SPS/PPS/VPS and creates video track
     * @param nal_data NAL unit data (with or without start codes)
     * @param size NAL unit size
     * @param timestamp_ms Timestamp in milliseconds
     * @param is_keyframe True if this is an IDR frame
     * @return true on success
     */
    bool WriteVideoFrame(const uint8_t* nal_data, size_t size, 
                        uint64_t timestamp_ms, bool is_keyframe);
    
    /**
     * Write AAC audio frame
     * @param aac_data AAC frame data (ADTS or raw)
     * @param size Frame size
     * @param timestamp_ms Timestamp in milliseconds
     * @return true on success
     */
    bool WriteAudioFrame(const uint8_t* aac_data, size_t size,
                        uint64_t timestamp_ms);
    
    /**
     * Get total bytes written
     */
    uint64_t GetBytesWritten() const { return bytes_written_; }
    
    /**
     * Get recording duration in milliseconds
     */
    uint64_t GetDurationMs() const;
    
    /**
     * Get number of video frames written
     */
    uint64_t GetVideoFrameCount() const { return video_frame_count_; }
    
    /**
     * Get number of audio frames written
     */
    uint64_t GetAudioFrameCount() const { return audio_frame_count_; }

private:
    // File I/O
    FILE* file_ = nullptr;
    MP4E_mux_t* mux_ = nullptr;
    std::unique_ptr<mp4_h26x_writer_t> h26x_writer_;
    
    // Configuration
    MP4RecorderConfig config_;
    
    // Track IDs
    int video_track_id_ = -1;
    int audio_track_id_ = -1;
    
    // State
    bool video_track_created_ = false;
    bool audio_track_created_ = false;
    bool video_params_ready_ = false;  // True once VPS/SPS/PPS have been set in mux
    uint64_t bytes_written_ = 0;
    uint64_t video_frame_count_ = 0;
    uint64_t audio_frame_count_ = 0;
    uint64_t first_timestamp_ms_ = 0;
    uint64_t last_timestamp_ms_ = 0;
    
    // SPS/PPS/VPS buffers (for H.264/H.265)
    std::vector<uint8_t> sps_data_;
    std::vector<uint8_t> pps_data_;
    std::vector<uint8_t> vps_data_;  // H.265 only
    
    // Audio frames buffered while waiting for video codec params.
    // In fragmented MP4, moov is written on the first MP4E_put_sample.
    // If audio arrives first, the hvcC box will have empty VPS/SPS/PPS.
    // We buffer audio until video params are ready, then flush.
    struct BufferedAudioFrame {
        std::vector<uint8_t> data;
        uint64_t timestamp_ms;
    };
    std::vector<BufferedAudioFrame> buffered_audio_;
    
    // Helper methods
    bool CreateVideoTrack();
    bool CreateAudioTrack();
    bool CheckVideoParamsReady();
    void FlushBufferedAudio();
    NalUnitType DetectNalType(const uint8_t* data, size_t size);
    bool RemoveStartCodes(const uint8_t* input, size_t input_size,
                         std::vector<uint8_t>& output);
    
    // Static callback for minimp4
    static int WriteCallback(int64_t offset, const void* buffer, size_t size, void* token);
};

/**
 * Helper: Parse H.264 SPS to extract width, height, fps
 * @param sps_data SPS NAL unit data (without start code)
 * @param size SPS size
 * @param width Output width
 * @param height Output height
 * @param fps Output fps (0 if not found)
 * @return true on success
 */
bool ParseH264SPS(const uint8_t* sps_data, size_t size,
                 int& width, int& height, int& fps);

/**
 * Helper: Parse H.265 SPS to extract width, height
 * @param sps_data SPS NAL unit data (without start code)
 * @param size SPS size
 * @param width Output width
 * @param height Output height
 * @return true on success
 */
bool ParseH265SPS(const uint8_t* sps_data, size_t size,
                 int& width, int& height);

} // namespace recording
} // namespace ipcam
