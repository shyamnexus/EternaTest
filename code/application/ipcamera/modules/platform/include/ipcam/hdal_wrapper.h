/**
 * @file hdal_wrapper.h
 * @brief HDAL Wrapper for Novatek ISP/Video/Audio Hardware Abstraction
 * 
 * This module provides a C++ wrapper around the Novatek HDAL vendor APIs,
 * offering a clean interface for ISP tuning, video encoding, and audio
 * processing operations.
 * 
 * @note Thread-safe - all operations are mutex protected
 */

#pragma once

#include <cstdint>
#include <string>
#include <memory>

namespace ipcam {
namespace platform {

/**
 * @brief ISP (Image Signal Processor) ID for multi-sensor support
 */
enum class IspId {
    kIsp0 = 0,
    kIsp1 = 1,
    kIsp2 = 2,
    kIsp3 = 3
};

/**
 * @brief White balance scene modes
 */
enum class WbScene {
    kAuto = 0,
    kDaylight,
    kCloudy,
    kTungsten,
    kSunset,
    kManual
};

/**
 * @brief Anti-flicker frequency modes (HDAL-specific naming to avoid conflict with isp_control.h)
 */
enum class HdalFlickerMode {
    kAuto = 0,
    k50Hz,
    k60Hz,
    k55Hz
};

/**
 * @brief Result codes for HDAL operations
 */
enum class HdalResult {
    kOk = 0,
    kError = -1,
    kInvalidParam = -2,
    kNotInitialized = -3,
    kNotSupported = -4
};

/**
 * @brief Image quality parameters structure
 */
struct IqParams {
    int brightness = 50;      ///< 0-100, default 50
    int contrast = 50;        ///< 0-100, default 50
    int saturation = 50;      ///< 0-100, default 50
    int sharpness = 50;       ///< 0-100, default 50
    int hue = 50;             ///< 0-100, default 50 (maps to -180 to +180 degrees)
    int nr_level = 50;        ///< 2D noise reduction 0-100
    int nr_3d_level = 50;     ///< 3D noise reduction 0-100
    int gamma = 50;           ///< Gamma level 0-100
    int tone = 50;            ///< Tone level 0-100
};

/**
 * @brief WDR (Wide Dynamic Range) parameters
 */
struct WdrParams {
    bool enable = false;
    int strength = 50;        ///< 0-100, WDR strength
    int color_protect = 50;   ///< 0-100, color protection strength
    int subimg_h = 32;        ///< Horizontal block count for local analysis
    int subimg_v = 24;        ///< Vertical block count
};

/**
 * @brief HDR (High Dynamic Range) parameters
 */
struct HdrParams {
    bool enable = false;
    int mode = 0;             ///< 0=off, 2=2-frame, 3=3-frame
    int strength = 50;        ///< 0-100, HDR strength
};

/**
 * @brief 2D Noise Reduction (BNR) parameters
 */
struct Bnr2dParams {
    bool enable = true;
    int strength = 50;        ///< 0-100, NR strength
    int threshold = 50;       ///< 0-100, noise threshold
};

/**
 * @brief 3D Noise Reduction (TMNR) parameters
 */
struct Tmnr3dParams {
    bool enable = true;
    int strength = 50;        ///< 0-100, NR strength
    int motion_threshold = 50;///< 0-100, motion detection threshold
    int blend_weight = 50;    ///< 0-100, temporal blending weight
};

/**
 * @brief Color Noise Reduction (LCA) parameters
 */
struct ColorNrParams {
    bool enable = true;
    int strength = 50;        ///< 0-100, color NR strength
    int color_protect = 50;   ///< 0-100, saturated color protection
};

/**
 * @brief Defog/Dehaze parameters
 */
struct DefogParams {
    bool enable = false;
    int strength = 50;        ///< 0-100, defog strength
    int fog_level = 50;       ///< 0-100, fog detection threshold
    bool auto_detect = false; ///< Auto fog detection
};

/**
 * @brief Edge Enhancement parameters
 */
struct EdgeEnhanceParams {
    bool enable = true;
    int strength = 50;        ///< 0-100, edge enhancement strength
    int coring = 50;          ///< 0-100, noise coring threshold
};

/**
 * @brief BLC (Backlight Compensation) parameters
 */
struct BlcParams {
    bool enable = false;
    int level = 50;           ///< 0-100, BLC level (center metering weight)
};

/**
 * @brief HLC (High Light Compensation) parameters
 */
struct HlcParams {
    bool enable = false;
    int level = 50;           ///< 0-100, highlight suppression level
};

/**
 * @brief Image Effect enumeration (matches Novatek IQ_UI_IMAGEEFFECT)
 */
enum class HdalImageEffect {
    kOff = 0,
    kBlackWhite = 1,
    kSepia = 2,
    kVivid = 3,
    kRock = 4,
    kCoolGreen = 5,
    kWarmYellow = 6,
    kSketch = 7,
    kColorPencil = 8,
    kReserved = 9,
    kNegativeDefog = 10,
    kCCID = 11,
    kThermalRed = 12,
    kThermalColor = 13
};

/**
 * @brief Rotation Mode (matches Novatek VPE_ISP_ROTATE_*)
 */
enum class HdalRotationMode {
    kRotate0 = 0,
    kRotate90 = 1,
    kRotate180 = 2,
    kRotate270 = 3,
    kHFlipRotate0 = 4,
    kHFlipRotate90 = 5,
    kHFlipRotate180 = 6,
    kHFlipRotate270 = 7
};

/**
 * @brief AWB Scene Mode (matches Novatek AWB_SCENE)
 */
enum class HdalAWBScene {
    kAuto = 0,
    kDaylight = 1,
    kCloudy = 2,
    kTungsten = 3,
    kSunset = 4,
    kCustomer1 = 5,
    kCustomer2 = 6,
    kCustomer3 = 7,
    kCustomer4 = 8,
    kCustomer5 = 9,
    kNightMode = 10,
    kManualGain = 11
};

/**
 * @brief Manual AE parameters
 */
struct ManualAEParams {
    bool manual_mode = false;     ///< false = auto, true = manual
    uint32_t exposure_time_us = 33333;  ///< Exposure time in microseconds
    uint32_t iso_gain = 100;      ///< ISO gain (100 = 1x)
};

/**
 * @brief DPC (Dead Pixel Correction) parameters
 */
struct DpcParams {
    bool enable = true;
    int strength = 50;        ///< 0-100, detection threshold (higher = more aggressive)
};

/**
 * @brief Shading (Lens Shading / Vignette Correction) parameters
 */
struct ShadingParams {
    bool enable = false;       ///< Master enable (enables ecs_enable + vig_enable)
    bool ecs_enable = false;   ///< Electronic Color Shading correction
    bool vig_enable = false;   ///< Vignette correction
    int strength = 50;         ///< 0-100, correction strength
    int mode = 1;              ///< 0=manual, 1=auto
};

/**
 * @brief WDR Enhancement parameters (detailed curve control)
 */
struct WdrEnhParams {
    bool enable = false;
    int ratio[17] = {128, 160, 192, 224, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024};
};

/**
 * @brief Scene Enhancement parameters
 */
struct EnhanceRatioParams {
    int dark_enhance = 50;     ///< 0-100, dark area enhancement
    int contrast_enhance = 50; ///< 0-100, contrast enhancement
    int green_enhance = 50;    ///< 0-100, green color enhancement
    int skin_enhance = 50;     ///< 0-100, skin tone enhancement
};

/**
 * @brief White balance parameters structure
 */
struct WbParams {
    WbScene scene = WbScene::kAuto;
    uint32_t r_ratio = 100;   ///< R gain ratio 50-200, 100=1x
    uint32_t b_ratio = 100;   ///< B gain ratio 50-200, 100=1x
};

/**
 * @brief Sensor direction (mirror/flip) parameters
 */
struct SensorDirection {
    bool mirror = false;
    bool flip = false;
};

/**
 * @brief HDAL Wrapper - Singleton class for HDAL ISP operations
 * 
 * Provides a high-level C++ interface to Novatek's vendor_isp_* APIs.
 * All methods are thread-safe and handle initialization automatically.
 */
class HdalWrapper {
public:
    /**
     * @brief Get singleton instance
     */
    static HdalWrapper& Instance();

    // Delete copy/move constructors
    HdalWrapper(const HdalWrapper&) = delete;
    HdalWrapper& operator=(const HdalWrapper&) = delete;
    HdalWrapper(HdalWrapper&&) = delete;
    HdalWrapper& operator=(HdalWrapper&&) = delete;

    /**
     * @brief Initialize HDAL vendor ISP
     * @return true on success
     */
    bool Init();

    /**
     * @brief Shutdown HDAL vendor ISP
     */
    void Shutdown();

    /**
     * @brief Check if initialized
     */
    bool IsInitialized() const { return initialized_; }

    // ========================================================================
    // Image Quality (IQ) Operations
    // ========================================================================

    /**
     * @brief Set brightness level
     * @param isp_id ISP identifier
     * @param value 0-100 (50 = normal)
     */
    HdalResult SetBrightness(IspId isp_id, int value);

    /**
     * @brief Set contrast level
     * @param isp_id ISP identifier  
     * @param value 0-100 (50 = normal)
     */
    HdalResult SetContrast(IspId isp_id, int value);

    /**
     * @brief Set saturation level
     * @param isp_id ISP identifier
     * @param value 0-100 (50 = normal)
     */
    HdalResult SetSaturation(IspId isp_id, int value);

    /**
     * @brief Set sharpness level
     * @param isp_id ISP identifier
     * @param value 0-100 (50 = normal)
     */
    HdalResult SetSharpness(IspId isp_id, int value);

    /**
     * @brief Set hue shift
     * @param isp_id ISP identifier
     * @param value 0-100 (50 = 0 degrees, 0 = -180, 100 = +180)
     */
    HdalResult SetHue(IspId isp_id, int value);

    /**
     * @brief Set 2D noise reduction level
     * @param isp_id ISP identifier
     * @param value 0-100 (50 = normal)
     */
    HdalResult SetNrLevel(IspId isp_id, int value);

    /**
     * @brief Set 3D noise reduction level
     * @param isp_id ISP identifier
     * @param value 0-100 (50 = normal)
     */
    HdalResult Set3dNrLevel(IspId isp_id, int value);

    /**
     * @brief Set all IQ parameters at once
     * @param isp_id ISP identifier
     * @param params IQ parameters structure
     */
    HdalResult SetIqParams(IspId isp_id, const IqParams& params);

    /**
     * @brief Get current IQ parameters
     * @param isp_id ISP identifier
     * @param params Output parameter structure
     */
    HdalResult GetIqParams(IspId isp_id, IqParams& params);

    // ========================================================================
    // White Balance (AWB) Operations
    // ========================================================================

    /**
     * @brief Set white balance scene mode
     * @param isp_id ISP identifier
     * @param scene Scene mode
     */
    HdalResult SetWbScene(IspId isp_id, WbScene scene);

    /**
     * @brief Set manual white balance gains
     * @param isp_id ISP identifier
     * @param r_ratio R gain ratio (50-200, 100=1x)
     * @param b_ratio B gain ratio (50-200, 100=1x)
     */
    HdalResult SetWbGains(IspId isp_id, uint32_t r_ratio, uint32_t b_ratio);

    /**
     * @brief Set all WB parameters
     * @param isp_id ISP identifier
     * @param params WB parameters structure
     */
    HdalResult SetWbParams(IspId isp_id, const WbParams& params);

    /**
     * @brief Get current WB parameters
     * @param isp_id ISP identifier
     * @param params Output parameter structure
     */
    HdalResult GetWbParams(IspId isp_id, WbParams& params);

    // ========================================================================
    // AE (Auto Exposure) Operations
    // ========================================================================

    /**
     * @brief Set anti-flicker frequency
     * @param isp_id ISP identifier
     * @param mode Flicker mode
     */
    HdalResult SetFlickerMode(IspId isp_id, HdalFlickerMode mode);

    /**
     * @brief Get current flicker mode
     * @param isp_id ISP identifier
     * @param mode Output mode
     */
    HdalResult GetFlickerMode(IspId isp_id, HdalFlickerMode& mode);

    // ========================================================================
    // Sensor Direction (Mirror/Flip) Operations
    // ========================================================================

    /**
     * @brief Set sensor mirror/flip
     * @param isp_id ISP identifier
     * @param dir Direction settings
     */
    HdalResult SetSensorDirection(IspId isp_id, const SensorDirection& dir);

    /**
     * @brief Get current sensor direction
     * @param isp_id ISP identifier
     * @param dir Output direction
     */
    HdalResult GetSensorDirection(IspId isp_id, SensorDirection& dir);

    // ========================================================================
    // ISP Feature Control
    // ========================================================================

    /**
     * @brief Enable/disable WDR (Wide Dynamic Range)
     * @param isp_id ISP identifier
     * @param enable Enable flag
     */
    HdalResult SetWdrEnable(IspId isp_id, bool enable);

    /**
     * @brief Set WDR parameters
     * @param isp_id ISP identifier
     * @param params WDR parameters
     */
    HdalResult SetWdrParams(IspId isp_id, const WdrParams& params);

    /**
     * @brief Get WDR parameters
     * @param isp_id ISP identifier
     * @param params Output parameters
     */
    HdalResult GetWdrParams(IspId isp_id, WdrParams& params);

    /**
     * @brief Set HDR parameters
     * @param isp_id ISP identifier
     * @param params HDR parameters
     */
    HdalResult SetHdrParams(IspId isp_id, const HdrParams& params);

    /**
     * @brief Set 2D Noise Reduction (BNR) parameters
     * @param isp_id ISP identifier
     * @param params BNR parameters
     */
    HdalResult SetBnr2dParams(IspId isp_id, const Bnr2dParams& params);

    /**
     * @brief Set 3D Noise Reduction (TMNR) parameters
     * @param isp_id ISP identifier
     * @param params TMNR parameters
     */
    HdalResult SetTmnr3dParams(IspId isp_id, const Tmnr3dParams& params);

    /**
     * @brief Set Color Noise Reduction (LCA) parameters
     * @param isp_id ISP identifier
     * @param params Color NR parameters
     */
    HdalResult SetColorNrParams(IspId isp_id, const ColorNrParams& params);

    /**
     * @brief Enable/disable Defog
     * @param isp_id ISP identifier  
     * @param enable Enable flag
     */
    HdalResult SetDefogEnable(IspId isp_id, bool enable);

    /**
     * @brief Set Defog parameters
     * @param isp_id ISP identifier
     * @param params Defog parameters
     */
    HdalResult SetDefogParams(IspId isp_id, const DefogParams& params);

    /**
     * @brief Set Edge Enhancement parameters
     * @param isp_id ISP identifier
     * @param params Edge enhancement parameters
     */
    HdalResult SetEdgeEnhanceParams(IspId isp_id, const EdgeEnhanceParams& params);

    /**
     * @brief Set BLC (Backlight Compensation) parameters
     * @param isp_id ISP identifier
     * @param params BLC parameters
     */
    HdalResult SetBlcParams(IspId isp_id, const BlcParams& params);

    /**
     * @brief Set HLC (High Light Compensation) parameters
     * @param isp_id ISP identifier
     * @param params HLC parameters
     */
    HdalResult SetHlcParams(IspId isp_id, const HlcParams& params);

    /**
     * @brief Set night mode
     * @param isp_id ISP identifier
     * @param enable Enable flag
     */
    HdalResult SetNightMode(IspId isp_id, bool enable);

    /**
     * @brief Set gamma level
     * @param isp_id ISP identifier
     * @param value 0-100 (50 = normal)
     */
    HdalResult SetGammaLevel(IspId isp_id, int value);

    /**
     * @brief Set tone level
     * @param isp_id ISP identifier
     * @param value 0-100 (50 = normal)
     */
    HdalResult SetToneLevel(IspId isp_id, int value);

    /**
     * @brief Set DPC (Dead Pixel Correction) parameters
     * @param isp_id ISP identifier
     * @param params DPC parameters
     */
    HdalResult SetDpcParams(IspId isp_id, const DpcParams& params);

    /**
     * @brief Set Shading (Lens Shading Correction) parameters
     * @param isp_id ISP identifier
     * @param params Shading parameters
     */
    HdalResult SetShadingParams(IspId isp_id, const ShadingParams& params);

    /**
     * @brief Set WDR Enhancement parameters (curve-based)
     * @param isp_id ISP identifier
     * @param params WDR Enhancement parameters
     */
    HdalResult SetWdrEnhParams(IspId isp_id, const WdrEnhParams& params);

    /**
     * @brief Set scene enhancement ratio parameters
     * @param isp_id ISP identifier
     * @param params Enhancement ratio parameters
     */
    HdalResult SetEnhanceRatioParams(IspId isp_id, const EnhanceRatioParams& params);

    /**
     * @brief Set dark enhance ratio
     * @param isp_id ISP identifier
     * @param value 0-100
     */
    HdalResult SetDarkEnhanceRatio(IspId isp_id, int value);

    /**
     * @brief Set contrast enhance ratio
     * @param isp_id ISP identifier
     * @param value 0-100
     */
    HdalResult SetContrastEnhanceRatio(IspId isp_id, int value);

    /**
     * @brief Set green enhance ratio
     * @param isp_id ISP identifier
     * @param value 0-100
     */
    HdalResult SetGreenEnhanceRatio(IspId isp_id, int value);

    /**
     * @brief Set skin enhance ratio
     * @param isp_id ISP identifier
     * @param value 0-100
     */
    HdalResult SetSkinEnhanceRatio(IspId isp_id, int value);

    /**
     * @brief Set image effect
     * @param isp_id ISP identifier
     * @param effect Image effect mode
     */
    HdalResult SetImageEffect(IspId isp_id, HdalImageEffect effect);

    /**
     * @brief Get current image effect
     * @param isp_id ISP identifier
     * @param effect Output: current effect
     */
    HdalResult GetImageEffect(IspId isp_id, HdalImageEffect& effect);

    /**
     * @brief Set rotation mode
     * @param isp_id ISP identifier
     * @param mode Rotation mode (including flip combinations)
     */
    HdalResult SetRotationMode(IspId isp_id, HdalRotationMode mode);

    /**
     * @brief Get current rotation mode
     * @param isp_id ISP identifier
     * @param mode Output: current rotation mode
     */
    HdalResult GetRotationMode(IspId isp_id, HdalRotationMode& mode);

    /**
     * @brief Set AWB scene mode
     * @param isp_id ISP identifier
     * @param scene AWB scene mode
     */
    HdalResult SetAWBScene(IspId isp_id, HdalAWBScene scene);

    /**
     * @brief Get current AWB scene mode
     * @param isp_id ISP identifier
     * @param scene Output: current AWB scene
     */
    HdalResult GetAWBScene(IspId isp_id, HdalAWBScene& scene);

    /**
     * @brief Set manual AE parameters
     * @param isp_id ISP identifier
     * @param params Manual AE parameters
     */
    HdalResult SetManualAE(IspId isp_id, const ManualAEParams& params);

    /**
     * @brief Get current manual AE parameters
     * @param isp_id ISP identifier
     * @param params Output: current AE parameters
     */
    HdalResult GetManualAE(IspId isp_id, ManualAEParams& params);

    /**
     * @brief Set B&W (Black and White) mode
     * @param isp_id ISP identifier
     * @param enable Enable B&W mode
     */
    HdalResult SetBWMode(IspId isp_id, bool enable);

    /**
     * @brief Get current B&W mode
     * @param isp_id ISP identifier
     * @param enable Output: current B&W mode state
     */
    HdalResult GetBWMode(IspId isp_id, bool& enable);

private:
    HdalWrapper() = default;
    ~HdalWrapper() = default;

    /**
     * @brief Convert 0-100 value to IQ level enum value
     * Range mapping: 0-100 -> Min to Max
     */
    int ConvertToIqLevel(int value, int min_val, int normal_val, int max_val);

    /**
     * @brief Convert IQ level enum value to 0-100
     */
    int ConvertFromIqLevel(int level, int min_val, int normal_val, int max_val);

    bool initialized_ = false;
    
    // Track WDR and BLC states independently to avoid conflicts
    // Both features use the same WDR hardware, but should be controllable separately
    bool wdr_enabled_ = false;      // User's WDR setting
    int wdr_strength_ = 50;         // User's WDR strength (0-100)
    bool blc_enabled_ = false;      // User's BLC setting  
    int blc_level_ = 50;            // User's BLC level (0-100)
};
} // namespace platform
} // namespace ipcam
