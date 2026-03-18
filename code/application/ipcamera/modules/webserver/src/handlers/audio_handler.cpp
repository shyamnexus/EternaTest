#include "audio_handler.h"
#include <ipcam/audio_control.h>
#include <ipcam/rtsp_server.h>
#include <ipcam/recording.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

namespace ipcam {
namespace webserver {
namespace handlers {

using json = nlohmann::json;

// Helper to check if a codec is allowed via API
// Only G.711 µ-law, G.711 A-law, and G.726 (32kbps) are currently exposed
// Other codecs are kept in code for future use but not allowed via API
static bool IsCodecAllowedViaApi(media::AudioCodec codec) {
    switch (codec) {
        case media::AudioCodec::G711_ULAW:
        case media::AudioCodec::G711_ALAW:
        case media::AudioCodec::G726_32:
            return true;
        // Future codecs - uncomment to enable:
        // case media::AudioCodec::G726_16:
        // case media::AudioCodec::G726_24:
        // case media::AudioCodec::G726_40:
        // case media::AudioCodec::AAC:
        // case media::AudioCodec::PCM:
        // case media::AudioCodec::ADPCM:
        //     return true;
        default:
            return false;
    }
}

api::Response HandleAudio(const api::RequestContext& ctx) {
    auto& audio = media::AudioControl::Instance();
    
    if (ctx.method == "GET") {
        try {
            auto config = audio.GetConfig();
            
            json response;
            response["enabled"] = config.enabled;
            response["codec"] = media::AudioCodecToString(config.codec);
            response["sample_rate"] = config.sample_rate;
            response["bitrate"] = config.bitrate;
            response["channels"] = config.channels;
            response["bit_width"] = config.bit_width;
            
            // New HDAL settings
            response["boost_gain"] = media::MicBoostGainToString(config.boost_gain);
            response["volume"] = config.volume;
            response["alc_enabled"] = config.alc_enabled;
            
            // Audio processing
            response["agc_enabled"] = config.agc_enabled;
            response["agc_target_db"] = config.agc_target_db;
            response["agc_noise_gate_db"] = config.agc_noise_gate_db;
            
            response["anr_enabled"] = config.anr_enabled;
            response["anr_level"] = config.anr_level;
            response["noise_gate_db"] = config.noise_gate_db;
            
            response["aec_enabled"] = config.aec_enabled;
            
            // Userspace audio processing
            response["processing"]["hpf"]["enabled"] = config.hpf_enabled;
            response["processing"]["hpf"]["frequency"] = config.hpf_freq;
            response["processing"]["hpf"]["q"] = config.hpf_q;
            
            response["processing"]["notch"]["enabled"] = config.notch_enabled;
            response["processing"]["notch"]["frequency"] = config.notch_freq;
            response["processing"]["notch"]["q"] = config.notch_q;
            response["processing"]["notch"]["gain_db"] = config.notch_gain;
            
            response["processing"]["ns"]["enabled"] = config.ns_enabled;
            response["processing"]["ns"]["level_db"] = config.ns_level;
            
            // Legacy fields (for backward compatibility)
            response["input_gain"] = config.input_gain;
            response["output_gain"] = config.output_gain;
            
            api::Response resp;
            resp.status_code = 200;
            resp.body = response.dump();
            return resp;
        } catch (const std::exception& e) {
            spdlog::error("HandleAudio GET error: {}", e.what());
            api::Response resp;
            resp.status_code = 500;
            resp.body = json{{"error", e.what()}}.dump();
            return resp;
        }
    }
    else if (ctx.method == "PUT") {
        try {
            auto body = json::parse(ctx.body);
            auto config = audio.GetConfig();
            
            // Update fields from request
            if (body.contains("enabled")) {
                config.enabled = body["enabled"].get<bool>();
            }
            if (body.contains("codec")) {
                auto new_codec = media::StringToAudioCodec(body["codec"].get<std::string>());
                // Validate codec is allowed via API
                if (!IsCodecAllowedViaApi(new_codec)) {
                    api::Response resp;
                    resp.status_code = 400;
                    resp.body = json{
                        {"error", "Codec not supported. Allowed codecs: g711_ulaw, g711_alaw, g726_32"},
                        {"allowed_codecs", json::array({"g711_ulaw", "g711_alaw", "g726_32"})}
                    }.dump();
                    return resp;
                }
                config.codec = new_codec;
            }
            if (body.contains("sample_rate")) {
                config.sample_rate = body["sample_rate"].get<int>();
            }
            if (body.contains("bitrate")) {
                config.bitrate = body["bitrate"].get<int>();
            }
            if (body.contains("channels")) {
                config.channels = body["channels"].get<int>();
            }
            if (body.contains("bit_width")) {
                config.bit_width = body["bit_width"].get<int>();
            }
            
            // New HDAL settings
            if (body.contains("boost_gain")) {
                config.boost_gain = media::StringToMicBoostGain(body["boost_gain"].get<std::string>());
            }
            if (body.contains("volume")) {
                config.volume = body["volume"].get<int>();
            }
            if (body.contains("alc_enabled")) {
                config.alc_enabled = body["alc_enabled"].get<bool>();
            }
            
            // Audio processing
            if (body.contains("agc_enabled")) {
                config.agc_enabled = body["agc_enabled"].get<bool>();
            }
            if (body.contains("agc_target_db")) {
                config.agc_target_db = body["agc_target_db"].get<int>();
            }
            if (body.contains("agc_noise_gate_db")) {
                config.agc_noise_gate_db = body["agc_noise_gate_db"].get<int>();
            }
            
            if (body.contains("anr_enabled")) {
                config.anr_enabled = body["anr_enabled"].get<bool>();
            }
            if (body.contains("anr_level")) {
                config.anr_level = body["anr_level"].get<int>();
            }
            if (body.contains("noise_gate_db")) {
                config.noise_gate_db = body["noise_gate_db"].get<int>();
            }
            
            if (body.contains("aec_enabled")) {
                config.aec_enabled = body["aec_enabled"].get<bool>();
            }
            
            // Userspace audio processing settings
            if (body.contains("processing")) {
                auto& proc = body["processing"];
                
                // HPF (High-Pass Filter)
                if (proc.contains("hpf")) {
                    auto& hpf = proc["hpf"];
                    if (hpf.contains("enabled")) config.hpf_enabled = hpf["enabled"].get<bool>();
                    if (hpf.contains("frequency")) config.hpf_freq = hpf["frequency"].get<int>();
                    if (hpf.contains("q")) config.hpf_q = hpf["q"].get<float>();
                }
                
                // Notch Filter
                if (proc.contains("notch")) {
                    auto& notch = proc["notch"];
                    if (notch.contains("enabled")) config.notch_enabled = notch["enabled"].get<bool>();
                    if (notch.contains("frequency")) config.notch_freq = notch["frequency"].get<int>();
                    if (notch.contains("q")) config.notch_q = notch["q"].get<float>();
                    if (notch.contains("gain_db")) config.notch_gain = notch["gain_db"].get<int>();
                }
                
                // NS (Noise Suppression)
                if (proc.contains("ns")) {
                    auto& ns = proc["ns"];
                    if (ns.contains("enabled")) config.ns_enabled = ns["enabled"].get<bool>();
                    if (ns.contains("level_db")) config.ns_level = ns["level_db"].get<int>();
                }
            }
            
            // Legacy fields (for backward compatibility)
            if (body.contains("input_gain")) {
                config.input_gain = body["input_gain"].get<int>();
            }
            if (body.contains("output_gain")) {
                config.output_gain = body["output_gain"].get<int>();
            }
            
            // Track if codec changed to trigger stream refresh
            auto old_config = audio.GetConfig();
            bool codec_changed = (config.codec != old_config.codec);
            bool enabled_changed = (config.enabled != old_config.enabled);
            bool processing_changed = (config.hpf_enabled != old_config.hpf_enabled ||
                                       config.notch_enabled != old_config.notch_enabled ||
                                       config.ns_enabled != old_config.ns_enabled ||
                                       config.agc_enabled != old_config.agc_enabled ||
                                       config.hpf_freq != old_config.hpf_freq ||
                                       config.ns_level != old_config.ns_level);
            
            bool success = audio.SetConfig(config);
            api::Response resp;
            if (success) {
                audio.SaveToConfig();  // Persist to file
                
                // If codec, enabled, or processing settings changed, refresh RTSP streams
                // The RTSP server will re-read config and reconfigure audio broadcaster
                if (codec_changed || enabled_changed || processing_changed) {
                    spdlog::info("Audio settings changed, refreshing RTSP streams...");
                    
                    // Refresh all RTSP streams to pick up new audio settings
                    auto& rtsp = streaming::GetRtspServer();
                    if (rtsp.IsRunning()) {
                        for (int i = 0; i < 4; i++) {  // Refresh all streams
                            rtsp.RefreshStream(i);
                        }
                    }
                }
                
                // If audio enabled/disabled changed, force a recording segment
                // cut so the new segment picks up the audio state change
                if (enabled_changed) {
                    spdlog::info("Audio enabled changed, forcing recording segment cut...");
                    auto& rec = recording::RecordingService::Instance();
                    rec.NotifyEncoderSettingsChanged(-1);  // All channels
                }
                
                resp.status_code = 200;
                resp.body = json{{"status", "ok"}, {"codec_changed", codec_changed}, {"processing_changed", processing_changed}}.dump();
            } else {
                resp.status_code = 400;
                resp.body = json{{"error", "Invalid audio configuration - check sample_rate, channels, anr_level, volume values"}}.dump();
            }
            return resp;
        } catch (const json::exception& e) {
            spdlog::error("HandleAudio PUT parse error: {}", e.what());
            api::Response resp;
            resp.status_code = 400;
            resp.body = json{{"error", "Invalid JSON"}}.dump();
            return resp;
        } catch (const std::exception& e) {
            spdlog::error("HandleAudio PUT error: {}", e.what());
            api::Response resp;
            resp.status_code = 500;
            resp.body = json{{"error", e.what()}}.dump();
            return resp;
        }
    }
    
    api::Response resp;
    resp.status_code = 405;
    resp.body = json{{"error", "Method not allowed"}}.dump();
    return resp;
}

api::Response HandleAudioCapabilities(const api::RequestContext& ctx) {
    if (ctx.method != "GET") {
        api::Response resp;
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto& audio = media::AudioControl::Instance();
        auto caps = audio.GetCapabilities();
        
        json response;
        
        // Only expose allowed codecs via API (others kept in backend for future use)
        // Currently allowed: G.711 µ-law, G.711 A-law, G.726 (32kbps)
        json codecs = json::array();
        codecs.push_back("g711_ulaw");
        codecs.push_back("g711_alaw");
        codecs.push_back("g726_32");
        response["codecs"] = codecs;
        
        // For G.711/G.726, only 8kHz is supported
        response["sample_rates"] = json::array({8000});
        response["bit_widths"] = json::array({16});
        response["max_channels"] = 1;  // Mono for IP cameras
        response["gain_range"]["min"] = caps.min_gain;
        response["gain_range"]["max"] = caps.max_gain;
        response["anr_level_range"]["min"] = caps.min_anr_level;
        response["anr_level_range"]["max"] = caps.max_anr_level;
        response["features"]["aec"] = caps.supports_aec;
        response["features"]["anr"] = caps.supports_anr;
        response["features"]["agc"] = caps.supports_agc;
        
        // Info about codecs not currently exposed via API (available for future use)
        json disabled_codecs = json::array();
        disabled_codecs.push_back({
            {"codec", "aac"},
            {"reason", "SDK lacks AAC encoder. Can be enabled in future if encoder is added."}
        });
        disabled_codecs.push_back({
            {"codec", "pcm"},
            {"reason", "Raw PCM uses excessive bandwidth. Available internally for testing."}
        });
        disabled_codecs.push_back({
            {"codec", "g726_16"},
            {"reason", "Lower quality G.726 variant. Can be enabled if needed."}
        });
        disabled_codecs.push_back({
            {"codec", "g726_24"},
            {"reason", "Lower quality G.726 variant. Can be enabled if needed."}
        });
        disabled_codecs.push_back({
            {"codec", "g726_40"},
            {"reason", "Higher bandwidth G.726 variant. Can be enabled if needed."}
        });
        response["disabled_codecs"] = disabled_codecs;
        response["default_codec"] = "g711_ulaw";
        response["allowed_codecs"] = json::array({"g711_ulaw", "g711_alaw", "g726_32"});
        
        api::Response resp;
        resp.status_code = 200;
        resp.body = response.dump();
        return resp;
    } catch (const std::exception& e) {
        spdlog::error("HandleAudioCapabilities error: {}", e.what());
        api::Response resp;
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
        return resp;
    }
}

api::Response HandleAudioTest(const api::RequestContext& ctx) {
    if (ctx.method != "POST") {
        api::Response resp;
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto& audio = media::AudioControl::Instance();
        bool success = audio.TestAudio();
        
        json response;
        response["status"] = success ? "ok" : "failed";
        response["message"] = success ? "Audio test completed successfully" : "Audio test failed";
        
        api::Response resp;
        resp.status_code = success ? 200 : 500;
        resp.body = response.dump();
        return resp;
    } catch (const std::exception& e) {
        spdlog::error("HandleAudioTest error: {}", e.what());
        api::Response resp;
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
        return resp;
    }
}

api::Response HandleAudioReset(const api::RequestContext& ctx) {
    if (ctx.method != "POST") {
        api::Response resp;
        resp.status_code = 405;
        resp.body = json{{"error", "Method not allowed"}}.dump();
        return resp;
    }
    
    try {
        auto& audio = media::AudioControl::Instance();
        bool success = audio.ResetToDefaults();
        
        api::Response resp;
        if (success) {
            resp.status_code = 200;
            resp.body = json{{"status", "ok"}, {"message", "Audio settings reset to defaults"}}.dump();
        } else {
            resp.status_code = 500;
            resp.body = json{{"error", "Failed to reset audio settings"}}.dump();
        }
        return resp;
    } catch (const std::exception& e) {
        spdlog::error("HandleAudioReset error: {}", e.what());
        api::Response resp;
        resp.status_code = 500;
        resp.body = json{{"error", e.what()}}.dump();
        return resp;
    }
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
