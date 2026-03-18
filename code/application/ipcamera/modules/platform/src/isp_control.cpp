#include "ipcam/isp_control.h"
#include "ipcam/hdal_wrapper.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <mutex>
#include <algorithm>

namespace ipcam {
namespace platform {

namespace {
    std::mutex g_mutex;
}

// ============================================================================
// Singleton Instance
// ============================================================================
ISPControl& ISPControl::Instance() {
    static ISPControl instance;
    return instance;
}

// ============================================================================
// Lifecycle
// ============================================================================
bool ISPControl::Init() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (initialized_) {
        spdlog::warn("ISPControl already initialized");
        return true;
    }

    spdlog::info("Initializing ISP Control module");

    if (!LoadFromConfig()) {
        spdlog::warn("Failed to load ISP config, using defaults");
    }

    // Initialize HDAL vendor ISP wrapper
    if (!HdalWrapper::Instance().Init()) {
        spdlog::warn("HDAL vendor ISP initialization failed - using stubs");
    }

    // Apply initial settings to hardware
    ApplyToHardware();

    initialized_ = true;
    spdlog::info("ISP Control module initialized");
    return true;
}

void ISPControl::Shutdown() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!initialized_) {
        return;
    }

    spdlog::info("Shutting down ISP Control module");
    
    // Shutdown HDAL vendor ISP wrapper
    HdalWrapper::Instance().Shutdown();
    
    initialized_ = false;
}

// ============================================================================
// Image Adjustment
// ============================================================================
bool ISPControl::SetAdjustment(const ImageAdjustment& adj) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // Validate ranges
    if (adj.brightness < 0 || adj.brightness > 100 ||
        adj.contrast < 0 || adj.contrast > 100 ||
        adj.saturation < 0 || adj.saturation > 100 ||
        adj.sharpness < 0 || adj.sharpness > 100 ||
        adj.hue < 0 || adj.hue > 100 ||
        adj.gamma < 0 || adj.gamma > 100) {
        spdlog::error("Invalid adjustment values - must be 0-100");
        return false;
    }

    adjustment_ = adj;
    
    // Update config
    config::Set<int>("isp.adjustment.brightness", adj.brightness);
    config::Set<int>("isp.adjustment.contrast", adj.contrast);
    config::Set<int>("isp.adjustment.saturation", adj.saturation);
    config::Set<int>("isp.adjustment.sharpness", adj.sharpness);
    config::Set<int>("isp.adjustment.hue", adj.hue);
    config::Set<int>("isp.adjustment.gamma", adj.gamma);
    config::Save();

    // Apply to HDAL hardware
    auto& hdal = HdalWrapper::Instance();
    // Auto-initialize if HDAL pipeline is already running (vendor_isp already init'd)
    if (!hdal.IsInitialized()) {
        spdlog::debug("HdalWrapper not initialized, attempting init...");
        hdal.Init();
    }
    if (hdal.IsInitialized()) {
        IqParams iq;
        iq.brightness = adj.brightness;
        iq.contrast = adj.contrast;
        iq.saturation = adj.saturation;
        iq.sharpness = adj.sharpness;
        iq.hue = adj.hue;
        hdal.SetIqParams(IspId::kIsp0, iq);
        
        // Apply gamma to hardware via dedicated API
        hdal.SetGammaLevel(IspId::kIsp0, adj.gamma);
    }

    spdlog::debug("Image adjustment updated: B={} C={} S={} Sh={} H={} G={}",
                  adj.brightness, adj.contrast, adj.saturation,
                  adj.sharpness, adj.hue, adj.gamma);
    return true;
}

ImageAdjustment ISPControl::GetAdjustment() const {
    return adjustment_;
}

bool ISPControl::SetBrightness(int value) {
    if (value < 0 || value > 100) return false;
    ImageAdjustment adj = adjustment_;
    adj.brightness = value;
    return SetAdjustment(adj);
}

bool ISPControl::SetContrast(int value) {
    if (value < 0 || value > 100) return false;
    ImageAdjustment adj = adjustment_;
    adj.contrast = value;
    return SetAdjustment(adj);
}

bool ISPControl::SetSaturation(int value) {
    if (value < 0 || value > 100) return false;
    ImageAdjustment adj = adjustment_;
    adj.saturation = value;
    return SetAdjustment(adj);
}

bool ISPControl::SetSharpness(int value) {
    if (value < 0 || value > 100) return false;
    ImageAdjustment adj = adjustment_;
    adj.sharpness = value;
    return SetAdjustment(adj);
}

bool ISPControl::SetHue(int value) {
    if (value < 0 || value > 100) return false;
    ImageAdjustment adj = adjustment_;
    adj.hue = value;
    return SetAdjustment(adj);
}

bool ISPControl::SetGamma(int value) {
    if (value < 0 || value > 100) return false;
    ImageAdjustment adj = adjustment_;
    adj.gamma = value;
    return SetAdjustment(adj);
}

// ============================================================================
// White Balance
// ============================================================================
bool ISPControl::SetWhiteBalance(const WhiteBalance& wb) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // Validate ranges
    if (wb.color_temperature < 2800 || wb.color_temperature > 6500 ||
        wb.r_gain < 0 || wb.r_gain > 255 ||
        wb.g_gain < 0 || wb.g_gain > 255 ||
        wb.b_gain < 0 || wb.b_gain > 255) {
        spdlog::error("Invalid white balance values");
        return false;
    }

    white_balance_ = wb;

    config::Set<std::string>("isp.white_balance.mode", WBModeToString(wb.mode));
    config::Set<std::string>("isp.white_balance.preset", WBPresetToString(wb.preset));
    config::Set<int>("isp.white_balance.color_temperature", wb.color_temperature);
    config::Set<int>("isp.white_balance.r_gain", wb.r_gain);
    config::Set<int>("isp.white_balance.g_gain", wb.g_gain);
    config::Set<int>("isp.white_balance.b_gain", wb.b_gain);
    config::Save();

    // Apply to HDAL AWB
    auto& hdal = HdalWrapper::Instance();
    if (hdal.IsInitialized()) {
        WbParams wbp;
        switch (wb.mode) {
            case WBMode::Auto:
                wbp.scene = WbScene::kAuto;
                break;
            case WBMode::Manual:
                wbp.scene = WbScene::kManual;
                // Convert 0-255 gain to 50-200 ratio (100=1x)
                wbp.r_ratio = 50 + (wb.r_gain * 150 / 255);
                wbp.b_ratio = 50 + (wb.b_gain * 150 / 255);
                break;
            case WBMode::Preset:
                // Map presets to AWB scenes
                switch (wb.preset) {
                    case WBPreset::Daylight: wbp.scene = WbScene::kDaylight; break;
                    case WBPreset::Cloudy: wbp.scene = WbScene::kCloudy; break;
                    case WBPreset::Tungsten: wbp.scene = WbScene::kTungsten; break;
                    default: wbp.scene = WbScene::kAuto; break;
                }
                break;
            default:
                wbp.scene = WbScene::kAuto;
                break;
        }
        hdal.SetWbParams(IspId::kIsp0, wbp);
    }

    spdlog::debug("White balance updated: mode={} temp={}K",
                  WBModeToString(wb.mode), wb.color_temperature);
    return true;
}

WhiteBalance ISPControl::GetWhiteBalance() const {
    return white_balance_;
}

bool ISPControl::SetWBMode(WBMode mode) {
    WhiteBalance wb = white_balance_;
    wb.mode = mode;
    return SetWhiteBalance(wb);
}

bool ISPControl::SetColorTemperature(int kelvin) {
    if (kelvin < 2800 || kelvin > 6500) return false;
    WhiteBalance wb = white_balance_;
    wb.color_temperature = kelvin;
    return SetWhiteBalance(wb);
}

bool ISPControl::SetRGBGain(int r, int g, int b) {
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) return false;
    WhiteBalance wb = white_balance_;
    wb.r_gain = r;
    wb.g_gain = g;
    wb.b_gain = b;
    return SetWhiteBalance(wb);
}

// ============================================================================
// Orientation
// ============================================================================
bool ISPControl::SetOrientation(const Orientation& orient) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (orient.rotation != 0 && orient.rotation != 90 &&
        orient.rotation != 180 && orient.rotation != 270) {
        spdlog::error("Invalid rotation value: must be 0, 90, 180, or 270");
        return false;
    }

    orientation_ = orient;

    config::Set<bool>("isp.orientation.mirror", orient.mirror);
    config::Set<bool>("isp.orientation.flip", orient.flip);
    config::Set<int>("isp.orientation.rotation", orient.rotation);
    config::Save();

    // Apply to HDAL sensor direction
    auto& hdal = HdalWrapper::Instance();
    if (hdal.IsInitialized()) {
        SensorDirection dir;
        dir.mirror = orient.mirror;
        dir.flip = orient.flip;
        // Note: Rotation is handled separately via video process if needed
        hdal.SetSensorDirection(IspId::kIsp0, dir);
    }

    spdlog::debug("Orientation updated: mirror={} flip={} rotation={}",
                  orient.mirror, orient.flip, orient.rotation);
    return true;
}

Orientation ISPControl::GetOrientation() const {
    return orientation_;
}

bool ISPControl::SetMirror(bool enabled) {
    Orientation orient = orientation_;
    orient.mirror = enabled;
    return SetOrientation(orient);
}

bool ISPControl::SetFlip(bool enabled) {
    Orientation orient = orientation_;
    orient.flip = enabled;
    return SetOrientation(orient);
}

bool ISPControl::SetRotation(int degrees) {
    Orientation orient = orientation_;
    orient.rotation = degrees;
    return SetOrientation(orient);
}

// ============================================================================
// Anti-Flicker
// ============================================================================
bool ISPControl::SetFlickerMode(FlickerMode mode) {
    std::lock_guard<std::mutex> lock(g_mutex);

    flicker_mode_ = mode;
    config::Set<std::string>("isp.anti_flicker.mode", FlickerModeToString(mode));
    
    if (mode == FlickerMode::Hz50) {
        config::Set<int>("isp.anti_flicker.frequency", 50);
    } else if (mode == FlickerMode::Hz60) {
        config::Set<int>("isp.anti_flicker.frequency", 60);
    }
    config::Save();

    // Apply to HDAL AE frequency
    auto& hdal = HdalWrapper::Instance();
    if (hdal.IsInitialized()) {
        HdalFlickerMode hdal_mode;
        switch (mode) {
            case FlickerMode::Auto: hdal_mode = HdalFlickerMode::kAuto; break;
            case FlickerMode::Hz50: hdal_mode = HdalFlickerMode::k50Hz; break;
            case FlickerMode::Hz60: hdal_mode = HdalFlickerMode::k60Hz; break;
            default: hdal_mode = HdalFlickerMode::kAuto; break;
        }
        hdal.SetFlickerMode(IspId::kIsp0, hdal_mode);
    }

    spdlog::debug("Anti-flicker mode set to: {}", FlickerModeToString(mode));
    return true;
}

FlickerMode ISPControl::GetFlickerMode() const {
    return flicker_mode_;
}

// ============================================================================
// Exposure
// ============================================================================
bool ISPControl::SetExposure(const ExposureSettings& exp) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (exp.ev_compensation < -128 || exp.ev_compensation > 127) {
        spdlog::error("Invalid EV compensation value");
        return false;
    }

    exposure_ = exp;

    config::Set<std::string>("isp.exposure.mode", ExposureModeToString(exp.mode));
    config::Set<std::string>("isp.exposure.metering_mode", MeteringModeToString(exp.metering));
    config::Set<bool>("isp.exposure.auto_gain_enabled", exp.auto_gain_enabled);
    config::Set<int>("isp.exposure.ev_compensation", exp.ev_compensation);
    config::Set<int>("isp.exposure.max_gain", exp.max_gain);
    config::Set<int>("isp.exposure.min_gain", exp.min_gain);
    config::Save();

    spdlog::debug("Exposure updated: mode={} metering={}",
                  ExposureModeToString(exp.mode), MeteringModeToString(exp.metering));
    return true;
}

ExposureSettings ISPControl::GetExposure() const {
    return exposure_;
}

bool ISPControl::SetExposureMode(ExposureMode mode) {
    ExposureSettings exp = exposure_;
    exp.mode = mode;
    return SetExposure(exp);
}

bool ISPControl::SetExposureTime(int microseconds) {
    ExposureSettings exp = exposure_;
    exp.exposure_time_us = microseconds;
    return SetExposure(exp);
}

bool ISPControl::SetGain(int value) {
    ExposureSettings exp = exposure_;
    exp.gain = value;
    return SetExposure(exp);
}

bool ISPControl::SetEVCompensation(int value) {
    if (value < -128 || value > 127) return false;
    ExposureSettings exp = exposure_;
    exp.ev_compensation = value;
    return SetExposure(exp);
}

// ============================================================================
// Day/Night
// ============================================================================
bool ISPControl::SetDayNight(const DayNightSettings& dn) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (dn.sensitivity < 1 || dn.sensitivity > 10) {
        spdlog::error("Invalid day/night sensitivity value");
        return false;
    }

    day_night_ = dn;

    config::Set<std::string>("isp.night_to_day.mode", DayNightModeToString(dn.mode));
    config::Set<std::string>("isp.night_to_day.ir_mode", IRModeToString(dn.ir_mode));
    config::Set<int>("isp.night_to_day.sensitivity", dn.sensitivity);
    config::Set<std::string>("isp.night_to_day.start_time", dn.start_time);
    config::Set<std::string>("isp.night_to_day.end_time", dn.end_time);
    config::Set<int>("isp.night_to_day.filter_level", dn.filter_level);
    config::Set<int>("isp.night_to_day.filter_time", dn.filter_time);
    config::Save();

    spdlog::debug("Day/Night updated: mode={} ir={}", 
                  DayNightModeToString(dn.mode), IRModeToString(dn.ir_mode));
    return true;
}

DayNightSettings ISPControl::GetDayNight() const {
    return day_night_;
}

bool ISPControl::SetDayNightMode(DayNightMode mode) {
    DayNightSettings dn = day_night_;
    dn.mode = mode;
    return SetDayNight(dn);
}

bool ISPControl::SetIRMode(IRMode mode) {
    DayNightSettings dn = day_night_;
    dn.ir_mode = mode;
    return SetDayNight(dn);
}

// ============================================================================
// BLC/WDR/HDR - Dynamic Range Enhancement
// ============================================================================
bool ISPControl::SetBLC(const BLCSettings& blc) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // Validate ranges
    if (blc.blc_level < 0 || blc.blc_level > 100 ||
        blc.wdr_level < 0 || blc.wdr_level > 100 ||
        blc.hdr_level < 0 || blc.hdr_level > 100 ||
        blc.hlc_level < 0 || blc.hlc_level > 100 ||
        blc.dark_boost_level < 0 || blc.dark_boost_level > 100) {
        spdlog::error("Invalid BLC/WDR/HDR level values (must be 0-100)");
        return false;
    }

    blc_ = blc;

    // Save to config
    config::Set<std::string>("isp.blc.mode", blc.blc_enabled ? "on" : "off");
    config::Set<int>("isp.blc.blc_strength", blc.blc_level);
    
    config::Set<std::string>("isp.blc.wdr", blc.wdr_enabled ? "open" : "close");
    config::Set<std::string>("isp.blc.wdr_mode", WdrModeToString(blc.wdr_mode));
    config::Set<int>("isp.blc.wdr_level", blc.wdr_level);
    config::Set<int>("isp.blc.wdr_color_protect", blc.wdr_color_protect);
    
    config::Set<std::string>("isp.blc.hdr", blc.hdr_enabled ? "open" : "close");
    config::Set<std::string>("isp.blc.hdr_mode", HdrModeToString(blc.hdr_mode));
    config::Set<int>("isp.blc.hdr_level", blc.hdr_level);
    
    config::Set<std::string>("isp.blc.hlc", blc.hlc_enabled ? "open" : "close");
    config::Set<int>("isp.blc.hlc_level", blc.hlc_level);
    config::Set<int>("isp.blc.dark_boost_level", blc.dark_boost_level);
    config::Save();

    // Apply to HDAL hardware
    ApplyBLCToHardware(blc);

    spdlog::debug("BLC settings updated: BLC={}/{} WDR={}/{} HDR={}/{} HLC={}/{}",
                  blc.blc_enabled, blc.blc_level,
                  blc.wdr_enabled, blc.wdr_level,
                  blc.hdr_enabled, blc.hdr_level,
                  blc.hlc_enabled, blc.hlc_level);
    return true;
}

void ISPControl::ApplyBLCToHardware(const BLCSettings& blc) {
    auto& hdal = HdalWrapper::Instance();
    if (!hdal.IsInitialized()) {
        spdlog::debug("HDAL not initialized, skipping hardware apply");
        return;
    }

    // Note: Web API uses 0-10 range, internal code uses 0-100
    // Scale appropriately: if value <= 10, assume 0-10 range and scale to 0-100
    auto scaleLevel = [](int level) -> int {
        if (level <= 10) {
            return level * 10;  // Scale 0-10 to 0-100
        }
        return level;  // Already in 0-100 range
    };

    // Apply WDR
    WdrParams wdr;
    wdr.enable = blc.wdr_enabled;
    wdr.strength = scaleLevel(blc.wdr_level);
    wdr.color_protect = blc.wdr_color_protect;
    wdr.subimg_h = 32;
    wdr.subimg_v = 24;
    hdal.SetWdrParams(IspId::kIsp0, wdr);

    // Apply HDR (sensor-level, may require pipeline reconfiguration)
    HdrParams hdr;
    hdr.enable = blc.hdr_enabled;
    switch (blc.hdr_mode) {
        case HdrMode::TwoFrame: hdr.mode = 2; break;
        case HdrMode::ThreeFrame: hdr.mode = 3; break;
        default: hdr.mode = 0; break;
    }
    hdr.strength = scaleLevel(blc.hdr_level);
    hdal.SetHdrParams(IspId::kIsp0, hdr);

    // Apply BLC (via WDR Enhancement per vendor guidance)
    BlcParams blc_params;
    blc_params.enable = blc.blc_enabled;
    blc_params.level = scaleLevel(blc.blc_level);
    hdal.SetBlcParams(IspId::kIsp0, blc_params);

    // Apply HLC (via AE Over-Exposure control per vendor guidance)
    HlcParams hlc;
    hlc.enable = blc.hlc_enabled;
    hlc.level = scaleLevel(blc.hlc_level);
    hdal.SetHlcParams(IspId::kIsp0, hlc);
    
    spdlog::info("Applied BLC/HLC to hardware: BLC={}/{} HLC={}/{}", 
                  blc.blc_enabled, blc_params.level, blc.hlc_enabled, hlc.level);
}

BLCSettings ISPControl::GetBLC() const {
    return blc_;
}

bool ISPControl::SetWDR(bool enabled, int level) {
    if (level < 0 || level > 100) return false;
    BLCSettings blc = blc_;
    blc.wdr_enabled = enabled;
    blc.wdr_level = level;
    return SetBLC(blc);
}

bool ISPControl::SetWDRMode(WdrMode mode, int level) {
    if (level < 0 || level > 100) return false;
    BLCSettings blc = blc_;
    blc.wdr_enabled = (mode != WdrMode::Off);
    blc.wdr_mode = mode;
    blc.wdr_level = level;
    return SetBLC(blc);
}

bool ISPControl::SetHDR(bool enabled, int level) {
    if (level < 0 || level > 100) return false;
    BLCSettings blc = blc_;
    blc.hdr_enabled = enabled;
    blc.hdr_level = level;
    return SetBLC(blc);
}

bool ISPControl::SetHDRMode(HdrMode mode, int level) {
    if (level < 0 || level > 100) return false;
    BLCSettings blc = blc_;
    blc.hdr_enabled = (mode != HdrMode::Off);
    blc.hdr_mode = mode;
    blc.hdr_level = level;
    return SetBLC(blc);
}

bool ISPControl::SetHLC(bool enabled, int level) {
    if (level < 0 || level > 100) return false;
    BLCSettings blc = blc_;
    blc.hlc_enabled = enabled;
    blc.hlc_level = level;
    return SetBLC(blc);
}

bool ISPControl::SetDarkBoost(int level) {
    if (level < 0 || level > 100) return false;
    BLCSettings blc = blc_;
    blc.dark_boost_level = level;
    return SetBLC(blc);
}

// ============================================================================
// Image Corrections - DPC, Shading
// ============================================================================
bool ISPControl::SetCorrections(const CorrectionSettings& corr) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // Validate ranges
    if (corr.dpc_strength < 0 || corr.dpc_strength > 100 ||
        corr.shading_strength < 0 || corr.shading_strength > 100) {
        spdlog::error("Invalid correction strength values (must be 0-100)");
        return false;
    }

    corrections_ = corr;

    // Save to config
    config::Set<bool>("isp.corrections.dpc_enabled", corr.dpc_enabled);
    config::Set<int>("isp.corrections.dpc_strength", corr.dpc_strength);
    config::Set<bool>("isp.corrections.shading_enabled", corr.shading_enabled);
    config::Set<int>("isp.corrections.shading_strength", corr.shading_strength);
    config::Save();

    // Apply to hardware
    ApplyCorrectionsToHardware(corr);

    spdlog::debug("Corrections updated: DPC={}/{} Shading={}/{}",
                  corr.dpc_enabled, corr.dpc_strength,
                  corr.shading_enabled, corr.shading_strength);
    return true;
}

void ISPControl::ApplyCorrectionsToHardware(const CorrectionSettings& corr) {
    auto& hdal = HdalWrapper::Instance();
    if (!hdal.IsInitialized()) {
        spdlog::debug("HDAL not initialized, skipping corrections apply");
        return;
    }

    // Apply Dead Pixel Correction
    DpcParams dpc;
    dpc.enable = corr.dpc_enabled;
    dpc.strength = corr.dpc_strength;
    hdal.SetDpcParams(IspId::kIsp0, dpc);

    // Apply Lens Shading Correction
    ShadingParams shading;
    shading.enable = corr.shading_enabled;
    shading.ecs_enable = corr.shading_enabled;
    shading.vig_enable = corr.shading_enabled;
    shading.strength = corr.shading_strength;
    hdal.SetShadingParams(IspId::kIsp0, shading);
}

CorrectionSettings ISPControl::GetCorrections() const {
    return corrections_;
}

bool ISPControl::SetDPC(bool enabled, int strength) {
    if (strength < 0 || strength > 100) return false;
    CorrectionSettings corr = corrections_;
    corr.dpc_enabled = enabled;
    corr.dpc_strength = strength;
    return SetCorrections(corr);
}

bool ISPControl::SetLensShading(bool enabled, int strength) {
    if (strength < 0 || strength > 100) return false;
    CorrectionSettings corr = corrections_;
    corr.shading_enabled = enabled;
    corr.shading_strength = strength;
    return SetCorrections(corr);
}

// ============================================================================
// Color Enhancement Ratios
// ============================================================================
bool ISPControl::SetEnhancementRatios(const EnhancementRatios& ratios) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // Validate ranges
    if (ratios.dark_enhance < 0 || ratios.dark_enhance > 100 ||
        ratios.contrast_enhance < 0 || ratios.contrast_enhance > 100 ||
        ratios.green_enhance < 0 || ratios.green_enhance > 100 ||
        ratios.skin_enhance < 0 || ratios.skin_enhance > 100 ||
        ratios.auto_tone < 0 || ratios.auto_tone > 100) {
        spdlog::error("Invalid enhancement ratio values (must be 0-100)");
        return false;
    }

    enhancement_ratios_ = ratios;

    // Save to config
    config::Set<int>("isp.enhancement_ratios.dark_enhance", ratios.dark_enhance);
    config::Set<int>("isp.enhancement_ratios.contrast_enhance", ratios.contrast_enhance);
    config::Set<int>("isp.enhancement_ratios.green_enhance", ratios.green_enhance);
    config::Set<int>("isp.enhancement_ratios.skin_enhance", ratios.skin_enhance);
    config::Set<int>("isp.enhancement_ratios.auto_tone", ratios.auto_tone);
    config::Save();

    // Apply to hardware
    ApplyEnhancementRatiosToHardware(ratios);

    spdlog::debug("Enhancement ratios updated: dark={} contrast={} green={} skin={} tone={}",
                  ratios.dark_enhance, ratios.contrast_enhance,
                  ratios.green_enhance, ratios.skin_enhance, ratios.auto_tone);
    return true;
}

void ISPControl::ApplyEnhancementRatiosToHardware(const EnhancementRatios& ratios) {
    auto& hdal = HdalWrapper::Instance();
    if (!hdal.IsInitialized()) {
        spdlog::debug("HDAL not initialized, skipping enhancement ratios apply");
        return;
    }

    // Apply all enhancement ratios
    EnhanceRatioParams params;
    params.dark_enhance = ratios.dark_enhance;
    params.contrast_enhance = ratios.contrast_enhance;
    params.green_enhance = ratios.green_enhance;
    params.skin_enhance = ratios.skin_enhance;
    hdal.SetEnhanceRatioParams(IspId::kIsp0, params);

    // Apply auto tone level (SHDR tone) - only if HDR is enabled
    // Note: IQT_ITEM_TONE_LV is for SHDR sensors only, crashes on linear sensors
    if (blc_.hdr_enabled && ratios.auto_tone > 0) {
        hdal.SetToneLevel(IspId::kIsp0, ratios.auto_tone);
    }
}

EnhancementRatios ISPControl::GetEnhancementRatios() const {
    return enhancement_ratios_;
}

bool ISPControl::SetDarkEnhance(int level) {
    if (level < 0 || level > 100) return false;
    EnhancementRatios ratios = enhancement_ratios_;
    ratios.dark_enhance = level;
    return SetEnhancementRatios(ratios);
}

bool ISPControl::SetContrastEnhance(int level) {
    if (level < 0 || level > 100) return false;
    EnhancementRatios ratios = enhancement_ratios_;
    ratios.contrast_enhance = level;
    return SetEnhancementRatios(ratios);
}

bool ISPControl::SetGreenEnhance(int level) {
    if (level < 0 || level > 100) return false;
    EnhancementRatios ratios = enhancement_ratios_;
    ratios.green_enhance = level;
    return SetEnhancementRatios(ratios);
}

bool ISPControl::SetSkinEnhance(int level) {
    if (level < 0 || level > 100) return false;
    EnhancementRatios ratios = enhancement_ratios_;
    ratios.skin_enhance = level;
    return SetEnhancementRatios(ratios);
}

bool ISPControl::SetAutoTone(int level) {
    if (level < 0 || level > 100) return false;
    EnhancementRatios ratios = enhancement_ratios_;
    ratios.auto_tone = level;
    return SetEnhancementRatios(ratios);
}

// ============================================================================
// Enhancement - Image Enhancement Features
// ============================================================================
bool ISPControl::SetEnhancement(const ImageEnhancement& enh) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // Validate all level values are in range
    if (enh.nr_2d_level < 0 || enh.nr_2d_level > 100 ||
        enh.nr_3d_level < 0 || enh.nr_3d_level > 100 ||
        enh.color_nr_level < 0 || enh.color_nr_level > 100 ||
        enh.defog_level < 0 || enh.defog_level > 100 ||
        enh.edge_enhance_level < 0 || enh.edge_enhance_level > 100 ||
        enh.edge_coring < 0 || enh.edge_coring > 100) {
        spdlog::error("Invalid enhancement level values (must be 0-100)");
        return false;
    }

    enhancement_ = enh;

    // Save to config
    config::Set<bool>("isp.image_enhancement.nr_2d_enabled", enh.nr_2d_enabled);
    config::Set<std::string>("isp.image_enhancement.nr_2d_mode", NrModeToString(enh.nr_2d_mode));
    config::Set<int>("isp.image_enhancement.nr_2d_level", enh.nr_2d_level);
    
    config::Set<bool>("isp.image_enhancement.nr_3d_enabled", enh.nr_3d_enabled);
    config::Set<std::string>("isp.image_enhancement.nr_3d_mode", NrModeToString(enh.nr_3d_mode));
    config::Set<int>("isp.image_enhancement.nr_3d_level", enh.nr_3d_level);
    config::Set<int>("isp.image_enhancement.nr_3d_motion_th", enh.nr_3d_motion_th);
    
    config::Set<bool>("isp.image_enhancement.color_nr_enabled", enh.color_nr_enabled);
    config::Set<int>("isp.image_enhancement.color_nr_level", enh.color_nr_level);
    
    config::Set<bool>("isp.image_enhancement.defog_enabled", enh.defog_enabled);
    config::Set<std::string>("isp.image_enhancement.defog_mode", DefogModeToString(enh.defog_mode));
    config::Set<int>("isp.image_enhancement.defog_level", enh.defog_level);
    config::Set<bool>("isp.image_enhancement.fog_detection", enh.fog_detection);
    
    config::Set<bool>("isp.image_enhancement.edge_enhance_enabled", enh.edge_enhance_enabled);
    config::Set<int>("isp.image_enhancement.edge_enhance_level", enh.edge_enhance_level);
    config::Set<int>("isp.image_enhancement.edge_coring", enh.edge_coring);
    
    config::Set<std::string>("isp.image_enhancement.scene_preset", ScenePresetToString(enh.scene_preset));
    config::Save();

    // Apply to HDAL hardware
    ApplyEnhancementToHardware(enh);

    spdlog::debug("Enhancement updated: 2DNR={}/{} 3DNR={}/{} ColorNR={}/{} Defog={}/{} Edge={}/{}",
                  enh.nr_2d_enabled, enh.nr_2d_level,
                  enh.nr_3d_enabled, enh.nr_3d_level,
                  enh.color_nr_enabled, enh.color_nr_level,
                  enh.defog_enabled, enh.defog_level,
                  enh.edge_enhance_enabled, enh.edge_enhance_level);
    return true;
}

void ISPControl::ApplyEnhancementToHardware(const ImageEnhancement& enh) {
    auto& hdal = HdalWrapper::Instance();
    if (!hdal.IsInitialized()) {
        spdlog::debug("HDAL not initialized, skipping hardware apply");
        return;
    }

    // Apply 2D NR (BNR)
    Bnr2dParams bnr;
    bnr.enable = enh.nr_2d_enabled;
    bnr.strength = enh.nr_2d_level;
    bnr.threshold = 50;
    hdal.SetBnr2dParams(IspId::kIsp0, bnr);

    // Apply 3D NR (TMNR)
    Tmnr3dParams tmnr;
    tmnr.enable = enh.nr_3d_enabled;
    tmnr.strength = enh.nr_3d_level;
    tmnr.motion_threshold = enh.nr_3d_motion_th;
    tmnr.blend_weight = 50 + enh.nr_3d_level / 2;  // Scale blend with strength
    hdal.SetTmnr3dParams(IspId::kIsp0, tmnr);

    // Apply Color NR
    ColorNrParams cnr;
    cnr.enable = enh.color_nr_enabled;
    cnr.strength = enh.color_nr_level;
    cnr.color_protect = 50;  // Default protection
    hdal.SetColorNrParams(IspId::kIsp0, cnr);

    // Apply Defog
    DefogParams defog;
    defog.enable = enh.defog_enabled;
    defog.strength = enh.defog_level;
    defog.auto_detect = enh.fog_detection;
    defog.fog_level = 50;
    hdal.SetDefogParams(IspId::kIsp0, defog);

    // Apply Edge Enhancement
    EdgeEnhanceParams edge;
    edge.enable = enh.edge_enhance_enabled;
    edge.strength = enh.edge_enhance_level;
    edge.coring = enh.edge_coring;
    hdal.SetEdgeEnhanceParams(IspId::kIsp0, edge);
}

ImageEnhancement ISPControl::GetEnhancement() const {
    return enhancement_;
}

bool ISPControl::Set2DNR(bool enabled, int level) {
    if (level < 0 || level > 100) return false;
    ImageEnhancement enh = enhancement_;
    enh.nr_2d_enabled = enabled;
    enh.nr_2d_level = level;
    return SetEnhancement(enh);
}

bool ISPControl::Set2DNRMode(NrMode mode, int level) {
    if (level < 0 || level > 100) return false;
    ImageEnhancement enh = enhancement_;
    enh.nr_2d_enabled = (mode != NrMode::Off);
    enh.nr_2d_mode = mode;
    enh.nr_2d_level = level;
    return SetEnhancement(enh);
}

bool ISPControl::Set3DNR(bool enabled, int level) {
    if (level < 0 || level > 100) return false;
    ImageEnhancement enh = enhancement_;
    enh.nr_3d_enabled = enabled;
    enh.nr_3d_level = level;
    return SetEnhancement(enh);
}

bool ISPControl::Set3DNRMode(NrMode mode, int level) {
    if (level < 0 || level > 100) return false;
    ImageEnhancement enh = enhancement_;
    enh.nr_3d_enabled = (mode != NrMode::Off);
    enh.nr_3d_mode = mode;
    enh.nr_3d_level = level;
    return SetEnhancement(enh);
}

bool ISPControl::SetColorNR(bool enabled, int level) {
    if (level < 0 || level > 100) return false;
    ImageEnhancement enh = enhancement_;
    enh.color_nr_enabled = enabled;
    enh.color_nr_level = level;
    return SetEnhancement(enh);
}

bool ISPControl::SetDefog(bool enabled, int level) {
    if (level < 0 || level > 100) return false;
    ImageEnhancement enh = enhancement_;
    enh.defog_enabled = enabled;
    enh.defog_level = level;
    return SetEnhancement(enh);
}

bool ISPControl::SetDefogMode(DefogMode mode, int level) {
    if (level < 0 || level > 100) return false;
    ImageEnhancement enh = enhancement_;
    enh.defog_enabled = (mode != DefogMode::Off);
    enh.defog_mode = mode;
    enh.defog_level = level;
    return SetEnhancement(enh);
}

bool ISPControl::SetFogDetection(bool enabled) {
    ImageEnhancement enh = enhancement_;
    enh.fog_detection = enabled;
    return SetEnhancement(enh);
}

bool ISPControl::SetEdgeEnhancement(bool enabled, int level) {
    if (level < 0 || level > 100) return false;
    ImageEnhancement enh = enhancement_;
    enh.edge_enhance_enabled = enabled;
    enh.edge_enhance_level = level;
    return SetEnhancement(enh);
}

bool ISPControl::SetEdgeCoring(int level) {
    if (level < 0 || level > 100) return false;
    ImageEnhancement enh = enhancement_;
    enh.edge_coring = level;
    return SetEnhancement(enh);
}

// ============================================================================
// Scene Presets
// ============================================================================
bool ISPControl::ApplyScenePreset(ScenePreset preset) {
    spdlog::info("Applying scene preset: {}", ScenePresetToString(preset));
    
    BLCSettings blc = blc_;
    ImageEnhancement enh = enhancement_;
    
    // Set the preset marker
    enh.scene_preset = preset;
    
    switch (preset) {
        case ScenePreset::Lobby:
            // Bright entrance with strong backlight
            blc.wdr_enabled = true;
            blc.wdr_mode = WdrMode::High;
            blc.wdr_level = 80;
            blc.blc_enabled = true;
            blc.blc_level = 70;
            blc.hlc_enabled = false;
            enh.nr_2d_level = 30;
            enh.nr_3d_level = 50;
            enh.defog_enabled = false;
            enh.edge_enhance_level = 50;
            break;
            
        case ScenePreset::ParkingDay:
            // Outdoor parking during day
            blc.wdr_enabled = true;
            blc.wdr_mode = WdrMode::Medium;
            blc.wdr_level = 60;
            blc.blc_enabled = false;
            blc.hlc_enabled = false;
            enh.nr_2d_level = 30;
            enh.nr_3d_level = 30;
            enh.defog_enabled = false;
            enh.edge_enhance_level = 60;
            break;
            
        case ScenePreset::ParkingNight:
            // Night with headlights
            blc.wdr_enabled = true;
            blc.wdr_mode = WdrMode::Low;
            blc.wdr_level = 40;
            blc.blc_enabled = false;
            blc.hlc_enabled = true;
            blc.hlc_level = 70;
            enh.nr_2d_level = 50;
            enh.nr_3d_level = 80;
            enh.defog_enabled = false;
            enh.edge_enhance_level = 40;
            break;
            
        case ScenePreset::Highway:
            // Vehicle monitoring
            blc.wdr_enabled = true;
            blc.wdr_mode = WdrMode::Medium;
            blc.wdr_level = 60;
            blc.blc_enabled = false;
            blc.hlc_enabled = true;
            blc.hlc_level = 80;
            enh.nr_2d_level = 30;
            enh.nr_3d_level = 50;
            enh.defog_enabled = false;
            enh.edge_enhance_level = 70;  // High for license plate
            break;
            
        case ScenePreset::Warehouse:
            // Mixed indoor lighting
            blc.wdr_enabled = true;
            blc.wdr_mode = WdrMode::Low;
            blc.wdr_level = 40;
            blc.blc_enabled = true;
            blc.blc_level = 50;
            blc.hlc_enabled = false;
            enh.nr_2d_level = 50;
            enh.nr_3d_level = 70;
            enh.defog_enabled = false;
            enh.edge_enhance_level = 50;
            break;
            
        case ScenePreset::OutdoorFoggy:
            // Fog/haze conditions
            blc.wdr_enabled = false;
            blc.blc_enabled = false;
            blc.hlc_enabled = false;
            enh.nr_2d_level = 30;
            enh.nr_3d_level = 50;
            enh.defog_enabled = true;
            enh.defog_mode = DefogMode::High;
            enh.defog_level = 80;
            enh.fog_detection = true;
            enh.edge_enhance_level = 60;
            break;
            
        case ScenePreset::OfficeIndoor:
            // Controlled lighting
            blc.wdr_enabled = false;
            blc.wdr_level = 40;
            blc.blc_enabled = false;
            blc.hlc_enabled = false;
            enh.nr_2d_level = 30;
            enh.nr_3d_level = 30;
            enh.defog_enabled = false;
            enh.edge_enhance_level = 50;
            break;
            
        case ScenePreset::LowLight:
            // Very dark environments
            blc.wdr_enabled = false;
            blc.blc_enabled = false;
            blc.hlc_enabled = false;
            enh.nr_2d_level = 60;
            enh.nr_3d_level = 90;
            enh.nr_3d_motion_th = 30;  // Lower motion threshold
            enh.color_nr_level = 80;
            enh.defog_enabled = false;
            enh.edge_enhance_level = 30;  // Lower to avoid noise
            enh.edge_coring = 70;  // Higher coring to suppress noise
            break;
            
        case ScenePreset::Custom:
        default:
            // No changes - use current settings
            spdlog::debug("Custom preset - no automatic adjustments");
            break;
    }
    
    // Apply settings
    bool success = true;
    if (!SetBLC(blc)) {
        spdlog::warn("Failed to apply BLC settings for preset");
        success = false;
    }
    if (!SetEnhancement(enh)) {
        spdlog::warn("Failed to apply enhancement settings for preset");
        success = false;
    }
    
    return success;
}

ScenePreset ISPControl::GetCurrentScenePreset() const {
    return enhancement_.scene_preset;
}

// ============================================================================
// B&W Mode
// ============================================================================
bool ISPControl::SetBWMode(bool enable) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // B&W mode is stored in adjustment struct
    adjustment_.bw_mode = enable;
    
    // Save to config
    config::Set<bool>("isp.adjustment.bw_mode", enable);
    config::Save();

    // Apply to hardware via HdalWrapper
    auto& hdal = HdalWrapper::Instance();
    if (hdal.IsInitialized()) {
        if (hdal.SetBWMode(IspId::kIsp0, enable) != HdalResult::kOk) {
            spdlog::warn("Failed to set B&W mode to hardware");
            return false;
        }
    }

    spdlog::info("B&W mode set to: {}", enable);
    return true;
}

bool ISPControl::GetBWMode() const {
    return adjustment_.bw_mode;
}

// ============================================================================
// Image Effect
// ============================================================================
bool ISPControl::SetImageEffect(ImageEffect effect) {
    std::lock_guard<std::mutex> lock(g_mutex);

    adjustment_.effect = effect;
    
    // Save to config
    config::Set<std::string>("isp.adjustment.image_effect", ImageEffectToString(effect));
    config::Save();

    // Apply to hardware
    auto& hdal = HdalWrapper::Instance();
    if (hdal.IsInitialized()) {
        HdalImageEffect hdal_effect = static_cast<HdalImageEffect>(effect);
        if (hdal.SetImageEffect(IspId::kIsp0, hdal_effect) != HdalResult::kOk) {
            spdlog::warn("Failed to set image effect to hardware");
            return false;
        }
    }

    spdlog::info("Image effect set to: {}", ImageEffectToString(effect));
    return true;
}

ImageEffect ISPControl::GetImageEffect() const {
    return adjustment_.effect;
}

// ============================================================================
// AWB Scene Mode
// ============================================================================
bool ISPControl::SetAWBScene(AWBScene scene) {
    std::lock_guard<std::mutex> lock(g_mutex);

    awb_scene_ = scene;
    
    // Save to config
    config::Set<std::string>("isp.white_balance.awb_scene", AWBSceneToString(scene));
    config::Save();

    // Apply to hardware
    auto& hdal = HdalWrapper::Instance();
    if (hdal.IsInitialized()) {
        HdalAWBScene hdal_scene = static_cast<HdalAWBScene>(scene);
        if (hdal.SetAWBScene(IspId::kIsp0, hdal_scene) != HdalResult::kOk) {
            spdlog::warn("Failed to set AWB scene to hardware");
            return false;
        }
    }

    spdlog::info("AWB scene set to: {}", AWBSceneToString(scene));
    return true;
}

AWBScene ISPControl::GetAWBScene() const {
    return awb_scene_;
}

// ============================================================================
// Rotation Mode
// ============================================================================
bool ISPControl::SetRotationMode(RotationMode mode) {
    std::lock_guard<std::mutex> lock(g_mutex);

    orientation_.rotation_mode = mode;
    
    // Also update the legacy rotation field for compatibility
    switch (mode) {
        case RotationMode::Rotate0:
        case RotationMode::HFlipRotate0:
            orientation_.rotation = 0;
            break;
        case RotationMode::Rotate90:
        case RotationMode::HFlipRotate90:
            orientation_.rotation = 90;
            break;
        case RotationMode::Rotate180:
        case RotationMode::HFlipRotate180:
            orientation_.rotation = 180;
            break;
        case RotationMode::Rotate270:
        case RotationMode::HFlipRotate270:
            orientation_.rotation = 270;
            break;
    }
    
    // Save to config
    config::Set<std::string>("isp.orientation.rotation_mode", RotationModeToString(mode));
    config::Set<int>("isp.orientation.rotation", orientation_.rotation);
    config::Save();

    // Apply to hardware
    auto& hdal = HdalWrapper::Instance();
    if (hdal.IsInitialized()) {
        HdalRotationMode hdal_mode = static_cast<HdalRotationMode>(mode);
        if (hdal.SetRotationMode(IspId::kIsp0, hdal_mode) != HdalResult::kOk) {
            spdlog::warn("Failed to set rotation mode to hardware");
            return false;
        }
    }

    spdlog::info("Rotation mode set to: {}", RotationModeToString(mode));
    return true;
}

RotationMode ISPControl::GetRotationMode() const {
    return orientation_.rotation_mode;
}

// ============================================================================
// Manual AE Controls
// ============================================================================
bool ISPControl::SetManualAE(const ManualAESettings& settings) {
    std::lock_guard<std::mutex> lock(g_mutex);

    manual_ae_ = settings;
    
    // Save to config
    config::Set<bool>("isp.exposure.manual_mode", settings.manual_mode);
    config::Set<int>("isp.exposure.exposure_time_us", static_cast<int>(settings.exposure_time_us));
    config::Set<int>("isp.exposure.iso_gain", static_cast<int>(settings.iso_gain));
    config::Save();

    // Apply to hardware
    auto& hdal = HdalWrapper::Instance();
    if (hdal.IsInitialized()) {
        ManualAEParams params;
        params.manual_mode = settings.manual_mode;
        params.exposure_time_us = settings.exposure_time_us;
        params.iso_gain = settings.iso_gain;
        if (hdal.SetManualAE(IspId::kIsp0, params) != HdalResult::kOk) {
            spdlog::warn("Failed to set manual AE to hardware");
            return false;
        }
    }

    spdlog::info("Manual AE set: mode={} exp={}us gain={}", 
                 settings.manual_mode, settings.exposure_time_us, settings.iso_gain);
    return true;
}

ManualAESettings ISPControl::GetManualAE() const {
    return manual_ae_;
}

bool ISPControl::SetAEMode(bool auto_mode) {
    ManualAESettings settings = manual_ae_;
    settings.manual_mode = !auto_mode;
    return SetManualAE(settings);
}

bool ISPControl::SetManualExposureTime(uint32_t time_us) {
    ManualAESettings settings = manual_ae_;
    settings.exposure_time_us = time_us;
    return SetManualAE(settings);
}

uint32_t ISPControl::GetManualExposureTime() const {
    return manual_ae_.exposure_time_us;
}

bool ISPControl::SetManualGain(uint32_t gain) {
    ManualAESettings settings = manual_ae_;
    settings.iso_gain = gain;
    return SetManualAE(settings);
}

uint32_t ISPControl::GetManualGain() const {
    return manual_ae_.iso_gain;
}

// ============================================================================
// Reset
// ============================================================================
bool ISPControl::ResetToDefaults() {
    std::lock_guard<std::mutex> lock(g_mutex);

    spdlog::info("Resetting ISP settings to defaults");

    adjustment_ = ImageAdjustment{};
    white_balance_ = WhiteBalance{};
    orientation_ = Orientation{};
    flicker_mode_ = FlickerMode::Auto;
    exposure_ = ExposureSettings{};
    day_night_ = DayNightSettings{};
    blc_ = BLCSettings{};
    enhancement_ = ImageEnhancement{};
    manual_ae_ = ManualAESettings{};
    awb_scene_ = AWBScene::Auto;

    // Save defaults to config
    return SaveToConfig();
}

// ============================================================================
// Config Operations
// ============================================================================
bool ISPControl::LoadFromConfig() {
    // Load adjustment
    adjustment_.brightness = config::Get<int>("isp.adjustment.brightness", 50);
    adjustment_.contrast = config::Get<int>("isp.adjustment.contrast", 50);
    adjustment_.saturation = config::Get<int>("isp.adjustment.saturation", 50);
    adjustment_.sharpness = config::Get<int>("isp.adjustment.sharpness", 50);
    adjustment_.hue = config::Get<int>("isp.adjustment.hue", 50);
    adjustment_.gamma = config::Get<int>("isp.adjustment.gamma", 50);
    adjustment_.bw_mode = config::Get<bool>("isp.adjustment.bw_mode", false);
    adjustment_.effect = StringToImageEffect(
        config::Get<std::string>("isp.adjustment.image_effect", "off"));

    // Load white balance
    white_balance_.mode = StringToWBMode(
        config::Get<std::string>("isp.white_balance.mode", "auto"));
    white_balance_.preset = StringToWBPreset(
        config::Get<std::string>("isp.white_balance.preset", "daylight"));
    white_balance_.color_temperature = config::Get<int>(
        "isp.white_balance.color_temperature", 5500);
    white_balance_.r_gain = config::Get<int>("isp.white_balance.r_gain", 128);
    white_balance_.g_gain = config::Get<int>("isp.white_balance.g_gain", 128);
    white_balance_.b_gain = config::Get<int>("isp.white_balance.b_gain", 128);

    // Load AWB scene mode
    awb_scene_ = StringToAWBScene(
        config::Get<std::string>("isp.white_balance.awb_scene", "auto"));

    // Load orientation
    orientation_.mirror = config::Get<bool>("isp.orientation.mirror", false);
    orientation_.flip = config::Get<bool>("isp.orientation.flip", false);
    orientation_.rotation = config::Get<int>("isp.orientation.rotation", 0);
    orientation_.rotation_mode = StringToRotationMode(
        config::Get<std::string>("isp.orientation.rotation_mode", "0"));

    // Load anti-flicker
    flicker_mode_ = StringToFlickerMode(
        config::Get<std::string>("isp.anti_flicker.mode", "auto"));

    // Load exposure
    exposure_.mode = StringToExposureMode(
        config::Get<std::string>("isp.exposure.mode", "auto"));
    exposure_.metering = StringToMeteringMode(
        config::Get<std::string>("isp.exposure.metering_mode", "center"));
    exposure_.auto_gain_enabled = config::Get<bool>(
        "isp.exposure.auto_gain_enabled", true);
    exposure_.ev_compensation = config::Get<int>(
        "isp.exposure.ev_compensation", 0);

    // Load manual AE settings
    manual_ae_.manual_mode = config::Get<bool>("isp.exposure.manual_mode", false);
    manual_ae_.exposure_time_us = static_cast<uint32_t>(
        config::Get<int>("isp.exposure.exposure_time_us", 33333));
    manual_ae_.iso_gain = static_cast<uint32_t>(
        config::Get<int>("isp.exposure.iso_gain", 100));

    // Load day/night
    day_night_.mode = StringToDayNightMode(
        config::Get<std::string>("isp.night_to_day.mode", "auto"));
    day_night_.ir_mode = StringToIRMode(
        config::Get<std::string>("isp.night_to_day.ir_mode", "auto"));
    day_night_.sensitivity = config::Get<int>(
        "isp.night_to_day.sensitivity", 5);
    day_night_.start_time = config::Get<std::string>(
        "isp.night_to_day.start_time", "18:00:00");
    day_night_.end_time = config::Get<std::string>(
        "isp.night_to_day.end_time", "06:00:00");

    // Load BLC/WDR/HDR settings
    // Note: API uses 0-10 range, so defaults should be in that range
    std::string blc_mode_str = config::Get<std::string>("isp.blc.mode", "off");
    blc_.blc_enabled = (blc_mode_str == "on");
    blc_.blc_level = config::Get<int>("isp.blc.blc_strength", 0);
    
    std::string wdr_str = config::Get<std::string>("isp.blc.wdr", "close");
    blc_.wdr_enabled = (wdr_str == "open");
    blc_.wdr_mode = StringToWdrMode(
        config::Get<std::string>("isp.blc.wdr_mode", "off"));
    blc_.wdr_level = config::Get<int>("isp.blc.wdr_level", 5);
    blc_.wdr_color_protect = config::Get<int>("isp.blc.wdr_color_protect", 5);
    
    std::string hdr_str = config::Get<std::string>("isp.blc.hdr", "close");
    blc_.hdr_enabled = (hdr_str == "open");
    blc_.hdr_mode = StringToHdrMode(
        config::Get<std::string>("isp.blc.hdr_mode", "off"));
    blc_.hdr_level = config::Get<int>("isp.blc.hdr_level", 5);
    
    std::string hlc_str = config::Get<std::string>("isp.blc.hlc", "close");
    blc_.hlc_enabled = (hlc_str == "open");
    blc_.hlc_level = config::Get<int>("isp.blc.hlc_level", 5);
    blc_.dark_boost_level = config::Get<int>("isp.blc.dark_boost_level", 0);

    // Load image enhancement settings
    enhancement_.scene_preset = StringToScenePreset(
        config::Get<std::string>("isp.image_enhancement.scene_preset", "custom"));
    
    enhancement_.nr_2d_enabled = config::Get<bool>(
        "isp.image_enhancement.nr_2d_enabled", true);
    enhancement_.nr_2d_mode = StringToNrMode(
        config::Get<std::string>("isp.image_enhancement.nr_2d_mode", "auto"));
    enhancement_.nr_2d_level = config::Get<int>(
        "isp.image_enhancement.nr_2d_level", 50);
    
    enhancement_.nr_3d_enabled = config::Get<bool>(
        "isp.image_enhancement.nr_3d_enabled", true);
    enhancement_.nr_3d_mode = StringToNrMode(
        config::Get<std::string>("isp.image_enhancement.nr_3d_mode", "auto"));
    enhancement_.nr_3d_level = config::Get<int>(
        "isp.image_enhancement.nr_3d_level", 50);
    enhancement_.nr_3d_motion_th = config::Get<int>(
        "isp.image_enhancement.nr_3d_motion_th", 50);
    
    enhancement_.color_nr_enabled = config::Get<bool>(
        "isp.image_enhancement.color_nr_enabled", true);
    enhancement_.color_nr_level = config::Get<int>(
        "isp.image_enhancement.color_nr_level", 50);
    
    enhancement_.defog_enabled = config::Get<bool>(
        "isp.image_enhancement.defog_enabled", false);
    enhancement_.defog_mode = StringToDefogMode(
        config::Get<std::string>("isp.image_enhancement.defog_mode", "off"));
    enhancement_.defog_level = config::Get<int>(
        "isp.image_enhancement.defog_level", 50);
    enhancement_.fog_detection = config::Get<bool>(
        "isp.image_enhancement.fog_detection", false);
    
    enhancement_.edge_enhance_enabled = config::Get<bool>(
        "isp.image_enhancement.edge_enhance_enabled", true);
    enhancement_.edge_enhance_level = config::Get<int>(
        "isp.image_enhancement.edge_enhance_level", 50);
    enhancement_.edge_coring = config::Get<int>(
        "isp.image_enhancement.edge_coring", 50);

    // Load image corrections (DPC, Shading)
    corrections_.dpc_enabled = config::Get<bool>(
        "isp.corrections.dpc_enabled", true);
    corrections_.dpc_strength = config::Get<int>(
        "isp.corrections.dpc_strength", 50);
    corrections_.shading_enabled = config::Get<bool>(
        "isp.corrections.shading_enabled", true);
    corrections_.shading_strength = config::Get<int>(
        "isp.corrections.shading_strength", 50);

    // Load enhancement ratios
    enhancement_ratios_.dark_enhance = config::Get<int>(
        "isp.enhancement_ratios.dark_enhance", 50);
    enhancement_ratios_.contrast_enhance = config::Get<int>(
        "isp.enhancement_ratios.contrast_enhance", 50);
    enhancement_ratios_.green_enhance = config::Get<int>(
        "isp.enhancement_ratios.green_enhance", 50);
    enhancement_ratios_.skin_enhance = config::Get<int>(
        "isp.enhancement_ratios.skin_enhance", 50);
    enhancement_ratios_.auto_tone = config::Get<int>(
        "isp.enhancement_ratios.auto_tone", 50);

    spdlog::info("ISP config loaded from file");
    return true;
}

bool ISPControl::SaveToConfig() {
    // This triggers config::Save() to persist changes
    return config::Save();
}

bool ISPControl::ApplyToHardware() {
    auto& hdal = HdalWrapper::Instance();
    if (!hdal.IsInitialized()) {
        spdlog::warn("ApplyToHardware: HDAL not initialized");
        return false;
    }

    bool success = true;

    // Apply image adjustments (brightness, contrast, saturation, sharpness, hue)
    {
        IqParams iq;
        iq.brightness = adjustment_.brightness;
        iq.contrast = adjustment_.contrast;
        iq.saturation = adjustment_.saturation;
        iq.sharpness = adjustment_.sharpness;
        iq.hue = adjustment_.hue;
        iq.nr_level = enhancement_.nr_2d_enabled ? enhancement_.nr_2d_level : 0;
        iq.nr_3d_level = enhancement_.nr_3d_enabled ? enhancement_.nr_3d_level : 0;
        
        if (hdal.SetIqParams(IspId::kIsp0, iq) != HdalResult::kOk) {
            spdlog::warn("Failed to apply IQ params to hardware");
            success = false;
        }
    }

    // Apply white balance
    {
        WbParams wbp;
        switch (white_balance_.mode) {
            case WBMode::Auto: wbp.scene = WbScene::kAuto; break;
            case WBMode::Manual:
                wbp.scene = WbScene::kManual;
                wbp.r_ratio = 50 + (white_balance_.r_gain * 150 / 255);
                wbp.b_ratio = 50 + (white_balance_.b_gain * 150 / 255);
                break;
            case WBMode::Preset:
                switch (white_balance_.preset) {
                    case WBPreset::Daylight: wbp.scene = WbScene::kDaylight; break;
                    case WBPreset::Cloudy: wbp.scene = WbScene::kCloudy; break;
                    case WBPreset::Tungsten: wbp.scene = WbScene::kTungsten; break;
                    default: wbp.scene = WbScene::kAuto; break;
                }
                break;
            default: wbp.scene = WbScene::kAuto; break;
        }
        if (hdal.SetWbParams(IspId::kIsp0, wbp) != HdalResult::kOk) {
            spdlog::warn("Failed to apply WB params to hardware");
            success = false;
        }
    }

    // Apply sensor direction (mirror/flip)
    {
        SensorDirection dir;
        dir.mirror = orientation_.mirror;
        dir.flip = orientation_.flip;
        if (hdal.SetSensorDirection(IspId::kIsp0, dir) != HdalResult::kOk) {
            spdlog::warn("Failed to apply sensor direction to hardware");
            success = false;
        }
    }

    // Apply anti-flicker
    {
        HdalFlickerMode fm;
        switch (flicker_mode_) {
            case FlickerMode::Auto: fm = HdalFlickerMode::kAuto; break;
            case FlickerMode::Hz50: fm = HdalFlickerMode::k50Hz; break;
            case FlickerMode::Hz60: fm = HdalFlickerMode::k60Hz; break;
            default: fm = HdalFlickerMode::kAuto; break;
        }
        if (hdal.SetFlickerMode(IspId::kIsp0, fm) != HdalResult::kOk) {
            spdlog::warn("Failed to apply flicker mode to hardware");
            success = false;
        }
    }

    // Apply night mode
    if (day_night_.mode == DayNightMode::Night || 
        (day_night_.mode == DayNightMode::Auto && day_night_.ir_mode != IRMode::Off)) {
        hdal.SetNightMode(IspId::kIsp0, true);
    } else {
        hdal.SetNightMode(IspId::kIsp0, false);
    }

    // Apply BLC/WDR/HLC settings using the unified function
    // This ensures all BLC-related settings (WDR, BLC, HLC) are applied consistently
    ApplyBLCToHardware(blc_);
    
    // Apply Defog
    hdal.SetDefogEnable(IspId::kIsp0, enhancement_.defog_enabled);

    // Apply gamma level
    hdal.SetGammaLevel(IspId::kIsp0, adjustment_.gamma);

    // Apply image corrections (DPC, Shading)
    ApplyCorrectionsToHardware(corrections_);

    // Apply enhancement ratios (dark, contrast, green, skin, auto_tone)
    ApplyEnhancementRatiosToHardware(enhancement_ratios_);

    spdlog::debug("ApplyToHardware: Settings applied to HDAL (success={})", success);
    return success;
}

// ============================================================================
// Helper Function Implementations
// ============================================================================
std::string WBModeToString(WBMode mode) {
    switch (mode) {
        case WBMode::Auto: return "auto";
        case WBMode::Manual: return "manual";
        case WBMode::Preset: return "preset";
        default: return "auto";
    }
}

WBMode StringToWBMode(const std::string& str) {
    if (str == "manual") return WBMode::Manual;
    if (str == "preset") return WBMode::Preset;
    return WBMode::Auto;
}

std::string WBPresetToString(WBPreset preset) {
    switch (preset) {
        case WBPreset::Daylight: return "daylight";
        case WBPreset::Cloudy: return "cloudy";
        case WBPreset::Shade: return "shade";
        case WBPreset::Tungsten: return "tungsten";
        case WBPreset::Fluorescent: return "fluorescent";
        case WBPreset::Flash: return "flash";
        case WBPreset::Custom: return "custom";
        default: return "daylight";
    }
}

WBPreset StringToWBPreset(const std::string& str) {
    if (str == "cloudy") return WBPreset::Cloudy;
    if (str == "shade") return WBPreset::Shade;
    if (str == "tungsten") return WBPreset::Tungsten;
    if (str == "fluorescent") return WBPreset::Fluorescent;
    if (str == "flash") return WBPreset::Flash;
    if (str == "custom") return WBPreset::Custom;
    return WBPreset::Daylight;
}

std::string FlickerModeToString(FlickerMode mode) {
    switch (mode) {
        case FlickerMode::Off: return "off";
        case FlickerMode::Hz50: return "50hz";
        case FlickerMode::Hz60: return "60hz";
        case FlickerMode::Auto: return "auto";
        default: return "auto";
    }
}

FlickerMode StringToFlickerMode(const std::string& str) {
    if (str == "off") return FlickerMode::Off;
    if (str == "50hz") return FlickerMode::Hz50;
    if (str == "60hz") return FlickerMode::Hz60;
    return FlickerMode::Auto;
}

std::string ExposureModeToString(ExposureMode mode) {
    switch (mode) {
        case ExposureMode::Auto: return "auto";
        case ExposureMode::Manual: return "manual";
        case ExposureMode::ShutterPriority: return "shutter_priority";
        case ExposureMode::GainPriority: return "gain_priority";
        default: return "auto";
    }
}

ExposureMode StringToExposureMode(const std::string& str) {
    if (str == "manual") return ExposureMode::Manual;
    if (str == "shutter_priority") return ExposureMode::ShutterPriority;
    if (str == "gain_priority") return ExposureMode::GainPriority;
    return ExposureMode::Auto;
}

std::string MeteringModeToString(MeteringMode mode) {
    switch (mode) {
        case MeteringMode::Center: return "center";
        case MeteringMode::Average: return "average";
        case MeteringMode::Spot: return "spot";
        default: return "center";
    }
}

MeteringMode StringToMeteringMode(const std::string& str) {
    if (str == "average") return MeteringMode::Average;
    if (str == "spot") return MeteringMode::Spot;
    return MeteringMode::Center;
}

std::string DayNightModeToString(DayNightMode mode) {
    switch (mode) {
        case DayNightMode::Day: return "day";
        case DayNightMode::Night: return "night";
        case DayNightMode::Auto: return "auto";
        case DayNightMode::Schedule: return "schedule";
        default: return "auto";
    }
}

DayNightMode StringToDayNightMode(const std::string& str) {
    if (str == "day") return DayNightMode::Day;
    if (str == "night") return DayNightMode::Night;
    if (str == "schedule") return DayNightMode::Schedule;
    return DayNightMode::Auto;
}

std::string IRModeToString(IRMode mode) {
    switch (mode) {
        case IRMode::Off: return "off";
        case IRMode::On: return "on";
        case IRMode::Auto: return "auto";
        default: return "auto";
    }
}

IRMode StringToIRMode(const std::string& str) {
    if (str == "off") return IRMode::Off;
    if (str == "on") return IRMode::On;
    return IRMode::Auto;
}

// ============================================================================
// Image Enhancement Helper Functions
// ============================================================================
std::string WdrModeToString(WdrMode mode) {
    switch (mode) {
        case WdrMode::Off: return "off";
        case WdrMode::Low: return "low";
        case WdrMode::Medium: return "medium";
        case WdrMode::High: return "high";
        case WdrMode::Auto: return "auto";
        default: return "off";
    }
}

WdrMode StringToWdrMode(const std::string& str) {
    if (str == "low") return WdrMode::Low;
    if (str == "medium") return WdrMode::Medium;
    if (str == "high") return WdrMode::High;
    if (str == "auto") return WdrMode::Auto;
    return WdrMode::Off;
}

std::string HdrModeToString(HdrMode mode) {
    switch (mode) {
        case HdrMode::Off: return "off";
        case HdrMode::TwoFrame: return "2frame";
        case HdrMode::ThreeFrame: return "3frame";
        case HdrMode::Auto: return "auto";
        default: return "off";
    }
}

HdrMode StringToHdrMode(const std::string& str) {
    if (str == "2frame") return HdrMode::TwoFrame;
    if (str == "3frame") return HdrMode::ThreeFrame;
    if (str == "auto") return HdrMode::Auto;
    return HdrMode::Off;
}

std::string NrModeToString(NrMode mode) {
    switch (mode) {
        case NrMode::Off: return "off";
        case NrMode::Low: return "low";
        case NrMode::Medium: return "medium";
        case NrMode::High: return "high";
        case NrMode::Auto: return "auto";
        default: return "auto";
    }
}

NrMode StringToNrMode(const std::string& str) {
    if (str == "off") return NrMode::Off;
    if (str == "low") return NrMode::Low;
    if (str == "medium") return NrMode::Medium;
    if (str == "high") return NrMode::High;
    return NrMode::Auto;
}

std::string DefogModeToString(DefogMode mode) {
    switch (mode) {
        case DefogMode::Off: return "off";
        case DefogMode::Low: return "low";
        case DefogMode::Medium: return "medium";
        case DefogMode::High: return "high";
        case DefogMode::Auto: return "auto";
        default: return "off";
    }
}

DefogMode StringToDefogMode(const std::string& str) {
    if (str == "low") return DefogMode::Low;
    if (str == "medium") return DefogMode::Medium;
    if (str == "high") return DefogMode::High;
    if (str == "auto") return DefogMode::Auto;
    return DefogMode::Off;
}

std::string ScenePresetToString(ScenePreset preset) {
    switch (preset) {
        case ScenePreset::Custom: return "custom";
        case ScenePreset::Lobby: return "lobby";
        case ScenePreset::ParkingDay: return "parking_day";
        case ScenePreset::ParkingNight: return "parking_night";
        case ScenePreset::Highway: return "highway";
        case ScenePreset::Warehouse: return "warehouse";
        case ScenePreset::OutdoorFoggy: return "outdoor_foggy";
        case ScenePreset::OfficeIndoor: return "office_indoor";
        case ScenePreset::LowLight: return "low_light";
        default: return "custom";
    }
}

ScenePreset StringToScenePreset(const std::string& str) {
    if (str == "lobby") return ScenePreset::Lobby;
    if (str == "parking_day") return ScenePreset::ParkingDay;
    if (str == "parking_night") return ScenePreset::ParkingNight;
    if (str == "highway") return ScenePreset::Highway;
    if (str == "warehouse") return ScenePreset::Warehouse;
    if (str == "outdoor_foggy") return ScenePreset::OutdoorFoggy;
    if (str == "office_indoor") return ScenePreset::OfficeIndoor;
    if (str == "low_light") return ScenePreset::LowLight;
    return ScenePreset::Custom;
}

std::string ImageEffectToString(ImageEffect effect) {
    switch (effect) {
        case ImageEffect::Off: return "off";
        case ImageEffect::BlackWhite: return "black_white";
        case ImageEffect::Sepia: return "sepia";
        case ImageEffect::Vivid: return "vivid";
        case ImageEffect::Rock: return "rock";
        case ImageEffect::CoolGreen: return "cool_green";
        case ImageEffect::WarmYellow: return "warm_yellow";
        case ImageEffect::Sketch: return "sketch";
        case ImageEffect::ColorPencil: return "color_pencil";
        case ImageEffect::Reserved: return "reserved";
        case ImageEffect::NegativeDefog: return "negative_defog";
        case ImageEffect::CCID: return "ccid";
        case ImageEffect::ThermalRed: return "thermal_red";
        case ImageEffect::ThermalColor: return "thermal_color";
        default: return "off";
    }
}

ImageEffect StringToImageEffect(const std::string& str) {
    if (str == "black_white") return ImageEffect::BlackWhite;
    if (str == "sepia") return ImageEffect::Sepia;
    if (str == "vivid") return ImageEffect::Vivid;
    if (str == "rock") return ImageEffect::Rock;
    if (str == "cool_green") return ImageEffect::CoolGreen;
    if (str == "warm_yellow") return ImageEffect::WarmYellow;
    if (str == "sketch") return ImageEffect::Sketch;
    if (str == "color_pencil") return ImageEffect::ColorPencil;
    if (str == "reserved") return ImageEffect::Reserved;
    if (str == "negative_defog") return ImageEffect::NegativeDefog;
    if (str == "ccid") return ImageEffect::CCID;
    if (str == "thermal_red") return ImageEffect::ThermalRed;
    if (str == "thermal_color") return ImageEffect::ThermalColor;
    return ImageEffect::Off;
}

std::string RotationModeToString(RotationMode mode) {
    switch (mode) {
        case RotationMode::Rotate0: return "0";
        case RotationMode::Rotate90: return "90";
        case RotationMode::Rotate180: return "180";
        case RotationMode::Rotate270: return "270";
        case RotationMode::HFlipRotate0: return "hflip_0";
        case RotationMode::HFlipRotate90: return "hflip_90";
        case RotationMode::HFlipRotate180: return "hflip_180";
        case RotationMode::HFlipRotate270: return "hflip_270";
        default: return "0";
    }
}

RotationMode StringToRotationMode(const std::string& str) {
    if (str == "90") return RotationMode::Rotate90;
    if (str == "180") return RotationMode::Rotate180;
    if (str == "270") return RotationMode::Rotate270;
    if (str == "hflip_0") return RotationMode::HFlipRotate0;
    if (str == "hflip_90") return RotationMode::HFlipRotate90;
    if (str == "hflip_180") return RotationMode::HFlipRotate180;
    if (str == "hflip_270") return RotationMode::HFlipRotate270;
    return RotationMode::Rotate0;
}

std::string AWBSceneToString(AWBScene scene) {
    switch (scene) {
        case AWBScene::Auto: return "auto";
        case AWBScene::Daylight: return "daylight";
        case AWBScene::Cloudy: return "cloudy";
        case AWBScene::Tungsten: return "tungsten";
        case AWBScene::Sunset: return "sunset";
        case AWBScene::Customer1: return "customer1";
        case AWBScene::Customer2: return "customer2";
        case AWBScene::Customer3: return "customer3";
        case AWBScene::Customer4: return "customer4";
        case AWBScene::Customer5: return "customer5";
        case AWBScene::NightMode: return "night_mode";
        case AWBScene::ManualGain: return "manual_gain";
        default: return "auto";
    }
}

AWBScene StringToAWBScene(const std::string& str) {
    if (str == "daylight") return AWBScene::Daylight;
    if (str == "cloudy") return AWBScene::Cloudy;
    if (str == "tungsten") return AWBScene::Tungsten;
    if (str == "sunset") return AWBScene::Sunset;
    if (str == "customer1") return AWBScene::Customer1;
    if (str == "customer2") return AWBScene::Customer2;
    if (str == "customer3") return AWBScene::Customer3;
    if (str == "customer4") return AWBScene::Customer4;
    if (str == "customer5") return AWBScene::Customer5;
    if (str == "night_mode") return AWBScene::NightMode;
    if (str == "manual_gain") return AWBScene::ManualGain;
    return AWBScene::Auto;
}

} // namespace platform
} // namespace ipcam
