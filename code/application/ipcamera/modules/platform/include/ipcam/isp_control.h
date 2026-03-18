#pragma once
#include <string>
#include <optional>
#include <cstdint>

namespace ipcam {
namespace platform {

// ============================================================================
// Image Effects
// ============================================================================
enum class ImageEffect {
    Off = 0,
    BlackWhite = 1,
    Sepia = 2,
    Vivid = 3,
    Rock = 4,
    CoolGreen = 5,
    WarmYellow = 6,
    Sketch = 7,
    ColorPencil = 8,
    Reserved = 9,
    NegativeDefog = 10,
    CCID = 11,
    ThermalRed = 12,
    ThermalColor = 13
};

// ============================================================================
// Rotation Mode (combines rotation and flip)
// ============================================================================
enum class RotationMode {
    Rotate0 = 0,           // No rotation
    Rotate90 = 1,          // 90 degree rotation
    Rotate180 = 2,         // 180 degree rotation
    Rotate270 = 3,         // 270 degree rotation
    HFlipRotate0 = 4,      // Horizontal flip + rotate 0
    HFlipRotate90 = 5,     // Horizontal flip + rotate 90
    HFlipRotate180 = 6,    // Horizontal flip + rotate 180
    HFlipRotate270 = 7     // Horizontal flip + rotate 270
};

// ============================================================================
// AWB Scene Modes
// ============================================================================
enum class AWBScene {
    Auto = 0,
    Daylight = 1,
    Cloudy = 2,
    Tungsten = 3,
    Sunset = 4,
    Customer1 = 5,
    Customer2 = 6,
    Customer3 = 7,
    Customer4 = 8,
    Customer5 = 9,
    NightMode = 10,
    ManualGain = 11
};

// ============================================================================
// Manual AE Settings
// ============================================================================
struct ManualAESettings {
    bool manual_mode = false;        // false = auto, true = manual
    uint32_t exposure_time_us = 33333;  // Exposure time in microseconds (default 1/30s)
    uint32_t iso_gain = 100;           // ISO gain (100 = 1x, range 100-3276800)
};

// ============================================================================
// Image Adjustment
// ============================================================================
struct ImageAdjustment {
    int brightness = 50;      // 0-100, default 50
    int contrast = 50;        // 0-100, default 50
    int saturation = 50;      // 0-100, default 50
    int sharpness = 50;       // 0-100, default 50
    int hue = 50;             // 0-100, default 50
    int gamma = 50;           // 0-100, default 50
    bool bw_mode = false;     // Black & White / Night Mode
    ImageEffect effect = ImageEffect::Off;  // Image effect
};

// ============================================================================
// White Balance
// ============================================================================
enum class WBMode {
    Auto,
    Manual,
    Preset
};

enum class WBPreset {
    Daylight,
    Cloudy,
    Shade,
    Tungsten,
    Fluorescent,
    Flash,
    Custom
};

struct WhiteBalance {
    WBMode mode = WBMode::Auto;
    WBPreset preset = WBPreset::Daylight;
    int color_temperature = 5500;  // 2800-6500K
    int r_gain = 128;              // 0-255
    int g_gain = 128;              // 0-255
    int b_gain = 128;              // 0-255
};

// ============================================================================
// Orientation (Mirror/Flip)
// ============================================================================
struct Orientation {
    bool mirror = false;
    bool flip = false;
    int rotation = 0;           // 0, 90, 180, 270
    RotationMode rotation_mode = RotationMode::Rotate0;  // Combined rotation + flip mode
};

// ============================================================================
// Anti-Flicker
// ============================================================================
enum class FlickerMode {
    Off,
    Hz50,
    Hz60,
    Auto
};

// ============================================================================
// Exposure
// ============================================================================
enum class ExposureMode {
    Auto,
    Manual,
    ShutterPriority,
    GainPriority
};

enum class MeteringMode {
    Center,
    Average,
    Spot
};

struct ExposureSettings {
    ExposureMode mode = ExposureMode::Auto;
    MeteringMode metering = MeteringMode::Center;
    bool auto_gain_enabled = true;
    int exposure_time_us = 33333;    // microseconds (1/30s default)
    int gain = 1;
    int ev_compensation = 0;         // -128 to +127
    int max_gain = 100;
    int min_gain = 1;
    int max_exposure_us = 166666;    // 1/6s
    int min_exposure_us = 100;       // 1/10000s
};

// ============================================================================
// Day/Night Mode
// ============================================================================
enum class DayNightMode {
    Day,
    Night,
    Auto,
    Schedule
};

enum class IRMode {
    Off,
    On,
    Auto
};

struct DayNightSettings {
    DayNightMode mode = DayNightMode::Auto;
    IRMode ir_mode = IRMode::Auto;
    int sensitivity = 5;        // 1-10
    std::string start_time = "18:00:00"; // HH:MM:SS
    std::string end_time = "06:00:00";   // HH:MM:SS
    int filter_level = 5;
    int filter_time = 5;
};

// ============================================================================
// BLC/WDR/HDR - Dynamic Range Enhancement
// ============================================================================
enum class WdrMode {
    Off,
    Low,
    Medium,
    High,
    Auto
};

enum class HdrMode {
    Off,
    TwoFrame,    // 2-exposure HDR
    ThreeFrame,  // 3-exposure HDR
    Auto
};

struct BLCSettings {
    bool blc_enabled = false;
    int blc_level = 50;            // 0-100, BLC intensity
    int blc_pos_x = 0;             // BLC region position
    int blc_pos_y = 0;
    int blc_width = 120;
    int blc_height = 90;
    bool wdr_enabled = false;
    WdrMode wdr_mode = WdrMode::Off;
    int wdr_level = 50;            // 0-100, WDR strength
    int wdr_color_protect = 50;    // 0-100, color protection strength
    bool hdr_enabled = false;
    HdrMode hdr_mode = HdrMode::Off;
    int hdr_level = 50;            // 0-100, HDR strength
    bool hlc_enabled = false;
    int hlc_level = 50;            // 0-100, High Light Compensation
    int dark_boost_level = 0;      // 0-100, dark area boost
};

// ============================================================================
// Image Corrections - DPC, Shading
// ============================================================================
struct CorrectionSettings {
    // Dead Pixel Correction (DPC)
    bool dpc_enabled = true;
    int dpc_strength = 50;         // 0-100, higher = more aggressive detection

    // Lens Shading Correction (LSC)
    bool shading_enabled = true;
    int shading_strength = 50;     // 0-100, vignette correction strength
};

// ============================================================================
// Color Enhancement Ratios
// ============================================================================
struct EnhancementRatios {
    int dark_enhance = 50;         // 0-100, dark area enhancement
    int contrast_enhance = 50;     // 0-100, contrast enhancement
    int green_enhance = 50;        // 0-100, green color enhancement (vegetation)
    int skin_enhance = 50;         // 0-100, skin tone enhancement
    int auto_tone = 50;            // 0-100, auto tone mapping level
};

// ============================================================================
// Image Enhancement - Noise Reduction & Effects
// ============================================================================
enum class NrMode {
    Off,
    Low,
    Medium,
    High,
    Auto
};

enum class DefogMode {
    Off,
    Low,
    Medium,
    High,
    Auto
};

// Scene preset for quick configuration
enum class ScenePreset {
    Custom,
    Lobby,         // Bright entrance, strong backlight
    ParkingDay,    // Outdoor parking during day
    ParkingNight,  // Night with headlights
    Highway,       // Vehicle monitoring
    Warehouse,     // Mixed indoor lighting
    OutdoorFoggy,  // Fog/haze conditions
    OfficeIndoor,  // Controlled lighting
    LowLight       // Very dark environments
};

struct ImageEnhancement {
    // 2D Noise Reduction (Spatial - BNR)
    bool nr_2d_enabled = true;
    NrMode nr_2d_mode = NrMode::Auto;
    int nr_2d_level = 50;          // 0-100, 2D NR strength

    // 3D Noise Reduction (Temporal - TMNR)
    bool nr_3d_enabled = true;
    NrMode nr_3d_mode = NrMode::Auto;
    int nr_3d_level = 50;          // 0-100, 3D NR strength
    int nr_3d_motion_th = 50;      // 0-100, motion detection threshold

    // Color Noise Reduction (LCA)
    bool color_nr_enabled = true;
    int color_nr_level = 50;       // 0-100, color NR strength

    // Defog/Dehaze
    bool defog_enabled = false;
    DefogMode defog_mode = DefogMode::Off;
    int defog_level = 50;          // 0-100, defog strength
    bool fog_detection = false;    // Auto fog detection

    // Edge Enhancement / Sharpening
    bool edge_enhance_enabled = true;
    int edge_enhance_level = 50;   // 0-100, edge enhancement
    int edge_coring = 50;          // 0-100, noise coring for edges

    // Scene preset
    ScenePreset scene_preset = ScenePreset::Custom;
};

// ============================================================================
// ISP Control Interface
// ============================================================================
class ISPControl {
public:
    static ISPControl& Instance();

    // Lifecycle
    bool Init();
    void Shutdown();
    bool IsInitialized() const { return initialized_; }

    // Image Adjustment
    bool SetAdjustment(const ImageAdjustment& adj);
    ImageAdjustment GetAdjustment() const;
    bool SetBrightness(int value);
    bool SetContrast(int value);
    bool SetSaturation(int value);
    bool SetSharpness(int value);
    bool SetHue(int value);
    bool SetGamma(int value);
    
    // Black & White / Night Mode
    bool SetBWMode(bool enabled);
    bool GetBWMode() const;
    
    // Image Effects
    bool SetImageEffect(ImageEffect effect);
    ImageEffect GetImageEffect() const;

    // White Balance
    bool SetWhiteBalance(const WhiteBalance& wb);
    WhiteBalance GetWhiteBalance() const;
    bool SetWBMode(WBMode mode);
    bool SetColorTemperature(int kelvin);
    bool SetRGBGain(int r, int g, int b);
    
    // AWB Scene Modes
    bool SetAWBScene(AWBScene scene);
    AWBScene GetAWBScene() const;

    // Orientation
    bool SetOrientation(const Orientation& orient);
    Orientation GetOrientation() const;
    bool SetMirror(bool enabled);
    bool SetFlip(bool enabled);
    bool SetRotation(int degrees);
    bool SetRotationMode(RotationMode mode);
    RotationMode GetRotationMode() const;

    // Anti-Flicker
    bool SetFlickerMode(FlickerMode mode);
    FlickerMode GetFlickerMode() const;

    // Exposure
    bool SetExposure(const ExposureSettings& exp);
    ExposureSettings GetExposure() const;
    bool SetExposureMode(ExposureMode mode);
    bool SetExposureTime(int microseconds);
    bool SetGain(int value);
    bool SetEVCompensation(int value);
    
    // Manual AE (Auto Exposure)
    bool SetManualAE(const ManualAESettings& ae);
    ManualAESettings GetManualAE() const;
    bool SetAEMode(bool manual);
    bool SetManualExposureTime(uint32_t time_us);
    uint32_t GetManualExposureTime() const;
    bool SetManualGain(uint32_t gain);
    uint32_t GetManualGain() const;

    // Day/Night
    bool SetDayNight(const DayNightSettings& dn);
    DayNightSettings GetDayNight() const;
    bool SetDayNightMode(DayNightMode mode);
    bool SetIRMode(IRMode mode);

    // BLC/WDR/HDR - Dynamic Range Enhancement
    bool SetBLC(const BLCSettings& blc);
    BLCSettings GetBLC() const;
    bool SetWDR(bool enabled, int level);
    bool SetWDRMode(WdrMode mode, int level = 50);
    bool SetHDR(bool enabled, int level);
    bool SetHDRMode(HdrMode mode, int level = 50);
    bool SetHLC(bool enabled, int level);
    bool SetDarkBoost(int level);

    // Image Corrections - DPC, Shading
    bool SetCorrections(const CorrectionSettings& corr);
    CorrectionSettings GetCorrections() const;
    bool SetDPC(bool enabled, int strength);
    bool SetLensShading(bool enabled, int strength);

    // Color Enhancement Ratios
    bool SetEnhancementRatios(const EnhancementRatios& ratios);
    EnhancementRatios GetEnhancementRatios() const;
    bool SetDarkEnhance(int level);
    bool SetContrastEnhance(int level);
    bool SetGreenEnhance(int level);
    bool SetSkinEnhance(int level);
    bool SetAutoTone(int level);

    // Image Enhancement - Noise Reduction
    bool SetEnhancement(const ImageEnhancement& enh);
    ImageEnhancement GetEnhancement() const;
    bool Set2DNR(bool enabled, int level);
    bool Set2DNRMode(NrMode mode, int level = 50);
    bool Set3DNR(bool enabled, int level);
    bool Set3DNRMode(NrMode mode, int level = 50);
    bool SetColorNR(bool enabled, int level);
    
    // Image Enhancement - Defog
    bool SetDefog(bool enabled, int level);
    bool SetDefogMode(DefogMode mode, int level = 50);
    bool SetFogDetection(bool enabled);
    
    // Image Enhancement - Edge/Sharpening
    bool SetEdgeEnhancement(bool enabled, int level);
    bool SetEdgeCoring(int level);

    // Scene Presets
    bool ApplyScenePreset(ScenePreset preset);
    ScenePreset GetCurrentScenePreset() const;

    // Convenience aliases for backward compatibility
    bool SetNoiseReduction(bool enabled, int level) { return Set2DNR(enabled, level); }

    // Reset to defaults
    bool ResetToDefaults();

    // Save current settings to config
    bool SaveToConfig();

private:
    ISPControl() = default;
    ~ISPControl() = default;
    ISPControl(const ISPControl&) = delete;
    ISPControl& operator=(const ISPControl&) = delete;

    bool LoadFromConfig();
    bool ApplyToHardware();
    void ApplyEnhancementToHardware(const ImageEnhancement& enh);
    void ApplyBLCToHardware(const BLCSettings& blc);
    void ApplyCorrectionsToHardware(const CorrectionSettings& corr);
    void ApplyEnhancementRatiosToHardware(const EnhancementRatios& ratios);

    bool initialized_ = false;
    ImageAdjustment adjustment_;
    WhiteBalance white_balance_;
    Orientation orientation_;
    FlickerMode flicker_mode_ = FlickerMode::Auto;
    ExposureSettings exposure_;
    DayNightSettings day_night_;
    BLCSettings blc_;
    ImageEnhancement enhancement_;
    CorrectionSettings corrections_;
    EnhancementRatios enhancement_ratios_;
    ManualAESettings manual_ae_;
    AWBScene awb_scene_ = AWBScene::Auto;
};

// ============================================================================
// Helper Functions
// ============================================================================
std::string WBModeToString(WBMode mode);
WBMode StringToWBMode(const std::string& str);

std::string WBPresetToString(WBPreset preset);
WBPreset StringToWBPreset(const std::string& str);

std::string FlickerModeToString(FlickerMode mode);
FlickerMode StringToFlickerMode(const std::string& str);

std::string ExposureModeToString(ExposureMode mode);
ExposureMode StringToExposureMode(const std::string& str);

std::string MeteringModeToString(MeteringMode mode);
MeteringMode StringToMeteringMode(const std::string& str);

std::string DayNightModeToString(DayNightMode mode);
DayNightMode StringToDayNightMode(const std::string& str);

std::string IRModeToString(IRMode mode);
IRMode StringToIRMode(const std::string& str);

// Image Enhancement helper functions
std::string WdrModeToString(WdrMode mode);
WdrMode StringToWdrMode(const std::string& str);

std::string HdrModeToString(HdrMode mode);
HdrMode StringToHdrMode(const std::string& str);

std::string NrModeToString(NrMode mode);
NrMode StringToNrMode(const std::string& str);

std::string DefogModeToString(DefogMode mode);
DefogMode StringToDefogMode(const std::string& str);

std::string ScenePresetToString(ScenePreset preset);
ScenePreset StringToScenePreset(const std::string& str);

// New helper functions for added features
std::string ImageEffectToString(ImageEffect effect);
ImageEffect StringToImageEffect(const std::string& str);

std::string RotationModeToString(RotationMode mode);
RotationMode StringToRotationMode(const std::string& str);

std::string AWBSceneToString(AWBScene scene);
AWBScene StringToAWBScene(const std::string& str);

} // namespace platform
} // namespace ipcam
