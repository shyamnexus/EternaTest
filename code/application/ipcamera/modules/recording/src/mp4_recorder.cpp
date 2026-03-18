/**
 * @file mp4_recorder.cpp
 * @brief MP4 Video Recorder Implementation
 */

// Define implementation before including minimp4.h to get function definitions
#define MINIMP4_IMPLEMENTATION

#include "ipcam/mp4_recorder.h"
#include <spdlog/spdlog.h>

#include <cstring>
#include <algorithm>

namespace ipcam {
namespace recording {

// ============================================================================
// MP4Recorder Implementation
// ============================================================================

MP4Recorder::~MP4Recorder() {
    Close();
}

MP4Recorder::MP4Recorder(MP4Recorder&& other) noexcept
    : file_(other.file_)
    , mux_(other.mux_)
    , h26x_writer_(std::move(other.h26x_writer_))
    , config_(std::move(other.config_))
    , video_track_id_(other.video_track_id_)
    , audio_track_id_(other.audio_track_id_)
    , video_track_created_(other.video_track_created_)
    , audio_track_created_(other.audio_track_created_)
    , video_params_ready_(other.video_params_ready_)
    , bytes_written_(other.bytes_written_)
    , video_frame_count_(other.video_frame_count_)
    , audio_frame_count_(other.audio_frame_count_)
    , first_timestamp_ms_(other.first_timestamp_ms_)
    , last_timestamp_ms_(other.last_timestamp_ms_)
    , sps_data_(std::move(other.sps_data_))
    , pps_data_(std::move(other.pps_data_))
    , vps_data_(std::move(other.vps_data_))
    , buffered_audio_(std::move(other.buffered_audio_))
{
    other.file_ = nullptr;
    other.mux_ = nullptr;
    other.video_track_id_ = -1;
    other.audio_track_id_ = -1;
}

MP4Recorder& MP4Recorder::operator=(MP4Recorder&& other) noexcept {
    if (this != &other) {
        Close();
        
        file_ = other.file_;
        mux_ = other.mux_;
        h26x_writer_ = std::move(other.h26x_writer_);
        config_ = std::move(other.config_);
        video_track_id_ = other.video_track_id_;
        audio_track_id_ = other.audio_track_id_;
        video_track_created_ = other.video_track_created_;
        audio_track_created_ = other.audio_track_created_;
        video_params_ready_ = other.video_params_ready_;
        bytes_written_ = other.bytes_written_;
        video_frame_count_ = other.video_frame_count_;
        audio_frame_count_ = other.audio_frame_count_;
        first_timestamp_ms_ = other.first_timestamp_ms_;
        last_timestamp_ms_ = other.last_timestamp_ms_;
        sps_data_ = std::move(other.sps_data_);
        pps_data_ = std::move(other.pps_data_);
        vps_data_ = std::move(other.vps_data_);
        buffered_audio_ = std::move(other.buffered_audio_);
        
        other.file_ = nullptr;
        other.mux_ = nullptr;
        other.video_track_id_ = -1;
        other.audio_track_id_ = -1;
    }
    return *this;
}

bool MP4Recorder::Open(const MP4RecorderConfig& config) {
    if (file_) {
        spdlog::error("MP4Recorder: Already open");
        return false;
    }
    
    config_ = config;
    
    // Open file
    file_ = fopen(config_.filename.c_str(), "wb");
    if (!file_) {
        spdlog::error("MP4Recorder: Failed to open file: {}", config_.filename);
        return false;
    }
    
    // Initialize muxer
    int enable_fragmentation = config_.fragmented ? 1 : 0;
    mux_ = MP4E_open(0, enable_fragmentation, file_, WriteCallback);
    if (!mux_) {
        spdlog::error("MP4Recorder: Failed to create MP4 muxer");
        fclose(file_);
        file_ = nullptr;
        return false;
    }
    
    // Initialize H.26x writer
    h26x_writer_ = std::make_unique<mp4_h26x_writer_t>();
    memset(h26x_writer_.get(), 0, sizeof(mp4_h26x_writer_t));
    
    // ================================================================
    // CRITICAL: In fragmented MP4 mode, the moov/mvex/trex header is
    // written on the FIRST MP4E_put_sample() call.  Only tracks that
    // exist at that moment get a trex entry.  We therefore create BOTH
    // the video and audio tracks right here in Open(), well before any
    // sample data arrives. This guarantees that whichever track writes
    // the first sample, the moov will contain trex for both.
    // ================================================================
    
    // Create video track via mp4_h26x_write_init (registers track in mux,
    // sets h26x_writer need_vps/sps/pps/idr flags).  Actual VPS/SPS/PPS
    // will be fed later when the first keyframe arrives — the writer will
    // buffer non-param NALs until all params are received.
    int is_hevc = (config_.video_codec == VideoCodec::H265) ? 1 : 0;
    int init_result = mp4_h26x_write_init(h26x_writer_.get(), mux_,
                                          config_.width, config_.height, is_hevc);
    if (init_result != MP4E_STATUS_OK) {
        spdlog::error("MP4Recorder: mp4_h26x_write_init failed ({})", init_result);
        MP4E_close(mux_);
        mux_ = nullptr;
        fclose(file_);
        file_ = nullptr;
        h26x_writer_.reset();
        return false;
    }
    video_track_id_ = h26x_writer_->mux_track_id;
    video_track_created_ = true;
    
    spdlog::info("MP4Recorder: Video track created in Open() (track_id={}, {})",
                 video_track_id_, is_hevc ? "H.265" : "H.264");
    
    // Create audio track (if enabled) immediately after video track.
    if (config_.enable_audio) {
        if (!CreateAudioTrack()) {
            spdlog::warn("MP4Recorder: Failed to create audio track in Open() — audio disabled");
            config_.enable_audio = false;
        } else {
            audio_track_created_ = true;
        }
    }
    
    // Reset per-segment state
    video_params_ready_ = false;
    bytes_written_ = 0;
    video_frame_count_ = 0;
    audio_frame_count_ = 0;
    first_timestamp_ms_ = 0;
    last_timestamp_ms_ = 0;
    buffered_audio_.clear();
    
    spdlog::info("MP4Recorder: Opened {} ({}x{} @ {}fps, fragmented={}, audio={})",
                 config_.filename, config_.width, config_.height, 
                 config_.fps, config_.fragmented, audio_track_created_);
    
    return true;
}

void MP4Recorder::Close() {
    if (!file_) {
        return;
    }
    
    // Finalize muxer
    if (mux_) {
        MP4E_close(mux_);
        mux_ = nullptr;
    }
    
    // Close file
    if (file_) {
        fclose(file_);
        file_ = nullptr;
    }
    
    h26x_writer_.reset();
    
    spdlog::info("MP4Recorder: Closed {} ({} bytes, {} video frames, {} audio frames)",
                 config_.filename, bytes_written_, video_frame_count_, audio_frame_count_);
    
    // Reset state
    video_track_created_ = false;
    audio_track_created_ = false;
    video_params_ready_ = false;
    video_track_id_ = -1;
    audio_track_id_ = -1;
    sps_data_.clear();
    pps_data_.clear();
    vps_data_.clear();
    buffered_audio_.clear();
}

bool MP4Recorder::WriteVideoFrame(const uint8_t* nal_data, size_t size,
                                  uint64_t timestamp_ms, bool is_keyframe) {
    if (!file_ || !mux_ || !h26x_writer_) {
        spdlog::error("MP4Recorder: Not open");
        return false;
    }
    
    // Track creation is done in Open() — both video and audio tracks exist
    // before any samples arrive, which guarantees correct trex entries in
    // fragmented MP4 mode.
    
    // Record first timestamp for duration calculation
    if (first_timestamp_ms_ == 0) {
        first_timestamp_ms_ = timestamp_ms;
    }
    
    // Calculate duration (in video timescale units)  
    int duration = config_.video_timescale / config_.fps;
    
    // Write frame using minimp4 — it handles Annex-B start code removal,
    // iterates through all NAL units in the buffer, and dispatches:
    //   VPS/SPS/PPS → MP4E_set_vps/sps/pps (clears internal need_* flags)
    //   SEI/AUD/etc → skipped (with our minimp4 fix)
    //   Slice NALs  → MP4E_put_sample (once all params received)
    // The h26x_writer will silently wait until VPS+SPS+PPS+IDR are all
    // received before writing the first video sample.
    int result = mp4_h26x_write_nal(h26x_writer_.get(), nal_data, size, duration);
    if (result == MP4E_STATUS_BAD_ARGUMENTS) {
        // This is expected while waiting for VPS/SPS/PPS/IDR — not an error
        spdlog::debug("MP4Recorder: Waiting for codec params (VPS/SPS/PPS/IDR)");
        return true;  // Not an error, just waiting
    }
    if (result < 0) {
        spdlog::error("MP4Recorder: Failed to write video frame (result={})", result);
        return false;
    }
    
    video_frame_count_++;
    bytes_written_ += size;
    last_timestamp_ms_ = timestamp_ms;
    
    // Check if video codec params are now ready (VPS/SPS/PPS all set).
    // If so, the hvcC/avcC box in the moov will be correct, and it's
    // now safe to write audio samples (which may trigger moov flush).
    if (!video_params_ready_ && CheckVideoParamsReady()) {
        video_params_ready_ = true;
        spdlog::info("MP4Recorder: Video codec params ready — flushing {} buffered audio frames",
                     buffered_audio_.size());
        FlushBufferedAudio();
    }
    
    return true;
}

bool MP4Recorder::WriteAudioFrame(const uint8_t* aac_data, size_t size,
                                  uint64_t timestamp_ms) {
    if (!file_ || !mux_) {
        spdlog::error("MP4Recorder: Not open");
        return false;
    }
    
    if (!config_.enable_audio) {
        return true;  // Silently ignore if audio disabled
    }
    
    if (!audio_track_created_) {
        return true;  // Silently ignore — audio track doesn't exist
    }
    
    // In fragmented MP4 mode, the moov/mvex header (including the hvcC box
    // with VPS/SPS/PPS) is written on the FIRST MP4E_put_sample() call.
    // If we write an audio sample before VPS/SPS/PPS have been set via
    // MP4E_set_vps/sps/pps, the hvcC will have empty parameter sets and
    // every player will fail to decode video ("PPS id out of range").
    // Buffer audio frames until video codec params are confirmed ready.
    if (!video_params_ready_) {
        buffered_audio_.push_back({std::vector<uint8_t>(aac_data, aac_data + size), timestamp_ms});
        spdlog::debug("MP4Recorder: Buffering audio frame ({} bytes) — waiting for video params",
                      size);
        return true;
    }
    
    // For AAC, we need to skip ADTS header if present (7 or 9 bytes)
    const uint8_t* audio_payload = aac_data;
    size_t payload_size = size;
    int samples_per_frame = 0;
    
    switch (config_.audio_codec) {
        case AudioCodec::AAC:
            // Check for ADTS sync word (0xFFF) and skip header if present
            if (size > 7 && (aac_data[0] == 0xFF) && ((aac_data[1] & 0xF0) == 0xF0)) {
                int header_size = (aac_data[1] & 0x01) ? 7 : 9;  // CRC or no CRC
                audio_payload = aac_data + header_size;
                payload_size = size - header_size;
            }
            samples_per_frame = 1024;  // AAC frame size
            break;
            
        case AudioCodec::PCMU:
        case AudioCodec::PCMA:
            // G.711 has no header, use raw data
            // For G.711, samples_per_frame = number of samples in buffer
            // At 8kHz, typical frame is 10-20ms = 80-160 samples
            samples_per_frame = payload_size;  // Each byte is one sample for G.711
            break;
            
        case AudioCodec::PCM:
            // Uncompressed PCM (s16le) - 2 bytes per sample
            // Calculate number of samples = bytes / 2 / channels
            samples_per_frame = payload_size / (2 * config_.audio_channels);
            break;
            
        default:
            spdlog::error("MP4Recorder: Unsupported audio codec in WriteAudioFrame");
            return false;
    }
    
    // Write audio sample
    int result = MP4E_put_sample(mux_, audio_track_id_, audio_payload, 
                                payload_size, samples_per_frame, 
                                MP4E_SAMPLE_DEFAULT);
    if (result < 0) {
        spdlog::error("MP4Recorder: Failed to write audio frame");
        return false;
    }
    
    audio_frame_count_++;
    
    return true;
}

uint64_t MP4Recorder::GetDurationMs() const {
    if (first_timestamp_ms_ == 0) {
        return 0;
    }
    return last_timestamp_ms_ - first_timestamp_ms_;
}

bool MP4Recorder::CreateVideoTrack() {
    // Video track is now created in Open() via mp4_h26x_write_init().
    // This method is retained only for API compatibility.
    // VPS/SPS/PPS are fed to the h26x_writer automatically when the encoder
    // produces them — mp4_h26x_write_nal dispatches them to MP4E_set_vps/sps/pps.
    spdlog::debug("MP4Recorder: CreateVideoTrack() called (track already created in Open())");
    return video_track_created_;
}

bool MP4Recorder::CreateAudioTrack() {
    MP4E_track_t tr;
    memset(&tr, 0, sizeof(tr));
    
    tr.track_media_kind = e_audio;
    tr.language[0] = 'u';
    tr.language[1] = 'n';
    tr.language[2] = 'd';
    tr.language[3] = 0;
    tr.time_scale = config_.audio_sample_rate;
    tr.default_duration = 1024;  // Samples per frame (typical for AAC)
    tr.u.a.channelcount = config_.audio_channels;
    
    switch (config_.audio_codec) {
        case AudioCodec::AAC:
            // AAC-LC — now supported via platform::AacCodec
            tr.object_type_indication = MP4_OBJECT_TYPE_AUDIO_ISO_IEC_14496_3;
            tr.default_duration = 1024;  // AAC-LC frame size
            spdlog::info("MP4Recorder: Using AAC-LC audio (universal playback)");
            break;
            
        case AudioCodec::PCMU:
            // G.711 μ-law - use private object type
            tr.object_type_indication = MP4_OBJECT_TYPE_USER_PRIVATE;
            tr.default_duration = config_.audio_sample_rate / 50;  // 20ms frames
            spdlog::info("MP4Recorder: Using G.711 μ-law audio (note: limited MP4 player support)");
            break;
            
        case AudioCodec::PCMA:
            // G.711 A-law - use private object type
            tr.object_type_indication = MP4_OBJECT_TYPE_USER_PRIVATE;
            tr.default_duration = config_.audio_sample_rate / 50;  // 20ms frames
            spdlog::info("MP4Recorder: Using G.711 A-law audio (note: limited MP4 player support)");
            break;
            
        case AudioCodec::PCM:
            // Uncompressed PCM
            tr.object_type_indication = MP4_OBJECT_TYPE_USER_PRIVATE;
            tr.default_duration = config_.audio_sample_rate / 50;  // 20ms frames
            spdlog::info("MP4Recorder: Using PCM audio (note: limited MP4 player support)");
            break;
            
        case AudioCodec::G726_16:
        case AudioCodec::G726_24:
        case AudioCodec::G726_32:
        case AudioCodec::G726_40:
            // G.726 ADPCM - use private object type
            tr.object_type_indication = MP4_OBJECT_TYPE_USER_PRIVATE;
            tr.default_duration = config_.audio_sample_rate / 50;  // 20ms frames
            spdlog::info("MP4Recorder: Using G.726 ADPCM audio (note: limited MP4 player support)");
            break;
            
        default:
            spdlog::error("MP4Recorder: Unsupported audio codec");
            return false;
    }
    
    audio_track_id_ = MP4E_add_track(mux_, &tr);
    
    if (audio_track_id_ < 0) {
        spdlog::error("MP4Recorder: Failed to add audio track");
        return false;
    }
    
    spdlog::info("MP4Recorder: Audio track created (track_id={}, {}Hz, {} ch)",
                 audio_track_id_, config_.audio_sample_rate, config_.audio_channels);
    
    // For AAC, set Decoder Specific Info (DSI / AudioSpecificConfig)
    // This is REQUIRED by the MP4 spec for AAC tracks.
    if (config_.audio_codec == AudioCodec::AAC) {
        // Build minimal AudioSpecificConfig (2 bytes for AAC-LC):
        //   5 bits: object type (2 = AAC-LC)
        //   4 bits: frequency index
        //   4 bits: channel configuration
        //   3 bits: padding zeros
        int freq_index = 0;
        switch (config_.audio_sample_rate) {
            case 96000: freq_index = 0;  break;
            case 88200: freq_index = 1;  break;
            case 64000: freq_index = 2;  break;
            case 48000: freq_index = 3;  break;
            case 44100: freq_index = 4;  break;
            case 32000: freq_index = 5;  break;
            case 24000: freq_index = 6;  break;
            case 22050: freq_index = 7;  break;
            case 16000: freq_index = 8;  break;
            case 12000: freq_index = 9;  break;
            case 11025: freq_index = 10; break;
            case  8000: freq_index = 11; break;
            case  7350: freq_index = 12; break;
            default:     freq_index = 3;  break; // default 48kHz
        }
        int ch_cfg = config_.audio_channels;  // 1=mono, 2=stereo
        uint8_t dsi[2];
        dsi[0] = static_cast<uint8_t>((2 << 3) | (freq_index >> 1));
        dsi[1] = static_cast<uint8_t>(((freq_index & 1) << 7) | (ch_cfg << 3));

        if (MP4E_set_dsi(mux_, audio_track_id_, dsi, sizeof(dsi)) != MP4E_STATUS_OK) {
            spdlog::warn("MP4Recorder: Failed to set AAC DSI — some players may not decode audio");
        }
    }
    
    return true;
}

bool MP4Recorder::CheckVideoParamsReady() {
    if (!h26x_writer_) return false;
    // The h26x_writer clears need_vps/need_sps/need_pps as VPS/SPS/PPS
    // are fed via mp4_h26x_write_nal.  Once all three (or two for H.264)
    // are cleared, the codec params are stored in the mux track and the
    // moov/hvcC/avcC will be correct when flushed.
    if (h26x_writer_->is_hevc) {
        return !h26x_writer_->need_vps && !h26x_writer_->need_sps && !h26x_writer_->need_pps;
    } else {
        return !h26x_writer_->need_sps && !h26x_writer_->need_pps;
    }
}

void MP4Recorder::FlushBufferedAudio() {
    for (auto& frame : buffered_audio_) {
        WriteAudioFrame(frame.data.data(), frame.data.size(), frame.timestamp_ms);
    }
    buffered_audio_.clear();
}

NalUnitType MP4Recorder::DetectNalType(const uint8_t* data, size_t size) {
    if (size < 4) {
        return NalUnitType::Unknown;
    }
    
    // Skip start codes (0x00 0x00 0x00 0x01 or 0x00 0x00 0x01)
    int offset = 0;
    if (data[0] == 0x00 && data[1] == 0x00) {
        if (data[2] == 0x00 && data[3] == 0x01) {
            offset = 4;
        } else if (data[2] == 0x01) {
            offset = 3;
        }
    }
    
    if (offset == 0 || offset >= size) {
        return NalUnitType::Unknown;
    }
    
    uint8_t nal_header = data[offset];
    
    if (config_.video_codec == VideoCodec::H264) {
        // H.264 NAL type is lower 5 bits
        uint8_t nal_type = nal_header & 0x1F;
        
        switch (nal_type) {
            case 7: return NalUnitType::SPS;
            case 8: return NalUnitType::PPS;
            case 5: return NalUnitType::IDR;
            case 1: return NalUnitType::NonIDR;
            case 6: return NalUnitType::SEI;
            default: return NalUnitType::Unknown;
        }
    } else {
        // H.265 NAL type is bits 1-6
        uint8_t nal_type = (nal_header >> 1) & 0x3F;
        
        switch (nal_type) {
            case 32: return NalUnitType::VPS;
            case 33: return NalUnitType::SPS;
            case 34: return NalUnitType::PPS;
            case 19:
            case 20: return NalUnitType::IDR;
            case 1: return NalUnitType::NonIDR;
            case 39:
            case 40: return NalUnitType::SEI;
            default: return NalUnitType::Unknown;
        }
    }
}

bool MP4Recorder::RemoveStartCodes(const uint8_t* input, size_t input_size,
                                   std::vector<uint8_t>& output) {
    output.clear();
    
    if (input_size < 4) {
        return false;
    }
    
    size_t pos = 0;
    
    // Find all NAL units and remove start codes
    while (pos < input_size) {
        // Look for start code
        if (pos + 3 < input_size && 
            input[pos] == 0x00 && input[pos + 1] == 0x00) {
            
            if (input[pos + 2] == 0x01) {
                // 3-byte start code
                pos += 3;
            } else if (pos + 4 < input_size && 
                      input[pos + 2] == 0x00 && input[pos + 3] == 0x01) {
                // 4-byte start code
                pos += 4;
            } else {
                pos++;
                continue;
            }
            
            // Find next start code or end of buffer
            size_t nal_start = pos;
            size_t nal_end = input_size;
            
            for (size_t i = pos; i < input_size - 3; i++) {
                if (input[i] == 0x00 && input[i + 1] == 0x00 && 
                    (input[i + 2] == 0x01 || 
                     (input[i + 2] == 0x00 && i + 3 < input_size && input[i + 3] == 0x01))) {
                    nal_end = i;
                    break;
                }
            }
            
            // Copy NAL unit without start code
            size_t nal_size = nal_end - nal_start;
            output.insert(output.end(), input + nal_start, input + nal_end);
            
            pos = nal_end;
        } else {
            pos++;
        }
    }
    
    // If no start codes found, copy entire buffer
    if (output.empty() && input_size > 0) {
        output.assign(input, input + input_size);
    }
    
    return !output.empty();
}

int MP4Recorder::WriteCallback(int64_t offset, const void* buffer, size_t size, void* token) {
    FILE* file = static_cast<FILE*>(token);
    fseek(file, offset, SEEK_SET);
    size_t written = fwrite(buffer, 1, size, file);
    return (written == size) ? 0 : -1;
}

// ============================================================================
// Helper Functions
// ============================================================================

bool ParseH264SPS(const uint8_t* sps_data, size_t size,
                 int& width, int& height, int& fps) {
    // Simplified SPS parsing - full implementation would use proper bit reader
    // For now, return configured values
    // TODO: Implement proper H.264 SPS parsing
    spdlog::warn("ParseH264SPS: Not implemented, using default values");
    width = 1920;
    height = 1080;
    fps = 30;
    return true;
}

bool ParseH265SPS(const uint8_t* sps_data, size_t size,
                 int& width, int& height) {
    // Simplified SPS parsing
    // TODO: Implement proper H.265 SPS parsing
    spdlog::warn("ParseH265SPS: Not implemented, using default values");
    width = 1920;
    height = 1080;
    return true;
}

} // namespace recording
} // namespace ipcam
