/**
 * @file image_adjustment_wrapper.h
 * @brief C wrapper for ISP control functions used by ONVIF imaging service
 * 
 * This header provides C-callable functions that wrap the C++ ISPControl class,
 * allowing the C-based ONVIF implementation to access ISP settings.
 */

#ifndef IMAGE_ADJUSTMENT_WRAPPER_H
#define IMAGE_ADJUSTMENT_WRAPPER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Use HD_RESULT compatible type for return values
#ifndef HD_RESULT
typedef int HD_RESULT;
#define HD_OK 0
#define HD_ERR_NG (-1)
#endif

#ifndef BOOL
typedef int BOOL;
#define TRUE 1
#define FALSE 0
#endif

// ============================================================================
// ONVIF Imaging Settings Ranges
// Note: Additional range constants are defined in onvif_imaging.h
// ============================================================================

typedef struct {
    float MIN_VALUE;
    float MAX_VALUE;
} ImageSettingRange;

// Standard ONVIF ranges (1-100) - compatible with onvif_imaging.h
// Note: BRIGHTNESS, CONTRAST, SATURATION, SHARPNESS are defined in onvif_imaging.h
// Note: MIN/MAX_BLC_LEVEL, MIN/MAX_WDR_LEVEL, MIN/MAX_RGAIN, MIN/MAX_BGAIN 
//       are defined in onvif_imaging.h

// ============================================================================
// Image Adjustment Functions (Brightness, Contrast, Saturation, Sharpness)
// Values are on Novatek scale (0-200, center at 100)
// ============================================================================

/**
 * @brief Get camera brightness setting
 * @param cam_id Camera ID (typically 0 for main camera)
 * @param value Output brightness value (0-200)
 * @return HD_OK on success, HD_ERR_NG on failure
 */
HD_RESULT get_camera_brightness(int cam_id, int *value);

/**
 * @brief Set camera brightness
 * @param cam_id Camera ID
 * @param value Brightness value (0-200)
 * @return HD_OK on success
 */
HD_RESULT set_camera_brightness(int cam_id, int value);

HD_RESULT get_camera_contrast(int cam_id, int *value);
HD_RESULT set_camera_contrast(int cam_id, int value);

HD_RESULT get_camera_saturation(int cam_id, int *value);
HD_RESULT set_camera_saturation(int cam_id, int value);

HD_RESULT get_camera_sharpness(int cam_id, int *value);
HD_RESULT set_camera_sharpness(int cam_id, int value);

// ============================================================================
// White Balance Functions
// ============================================================================

/**
 * @brief Get white balance mode
 * @param cam_id Camera ID
 * @param mode Output: 0=auto, 1=manual
 * @return HD_OK on success
 */
HD_RESULT get_camera_wb_mode(int cam_id, int *mode);

/**
 * @brief Set white balance mode
 * @param cam_id Camera ID
 * @param mode 0=auto, 1=manual
 * @return HD_OK on success
 */
HD_RESULT set_camera_wb_mode(int cam_id, int mode);

/**
 * @brief Get manual white balance gains
 * @param cam_id Camera ID
 * @param r_gain Red gain (0-255)
 * @param g_gain Green gain (0-255)
 * @param b_gain Blue gain (0-255)
 * @return HD_OK on success
 */
HD_RESULT get_camera_manual_wb_gain(int cam_id, int *r_gain, int *g_gain, int *b_gain);

/**
 * @brief Set manual white balance gains
 * @param cam_id Camera ID
 * @param r_gain Red gain (0-255)
 * @param g_gain Green gain (0-255)
 * @param b_gain Blue gain (0-255)
 * @return HD_OK on success
 */
HD_RESULT set_camera_manual_wb_gain(int cam_id, int r_gain, int g_gain, int b_gain);

// ============================================================================
// Wide Dynamic Range (WDR) Functions
// ============================================================================

/**
 * @brief Get WDR enabled state
 * @param cam_id Camera ID
 * @param enabled Output: TRUE if WDR is enabled
 * @return HD_OK on success
 */
HD_RESULT get_wdr_enable(int cam_id, BOOL *enabled);

/**
 * @brief Enable or disable WDR
 * @param cam_id Camera ID
 * @param enabled TRUE to enable WDR
 * @return HD_OK on success
 */
HD_RESULT set_wdr_enable(int cam_id, BOOL enabled);

/**
 * @brief Get WDR strength (manual mode)
 * @param cam_id Camera ID
 * @param strength Output strength value (0-100)
 * @return HD_OK on success
 */
HD_RESULT get_manual_wdr_strength(int cam_id, int *strength);

/**
 * @brief Set WDR strength (manual mode)
 * @param cam_id Camera ID
 * @param strength Strength value (0-100)
 * @return HD_OK on success
 */
HD_RESULT set_manual_wdr_strength(int cam_id, int strength);

// ============================================================================
// B&W (Black and White) Mode Functions
// ============================================================================

/**
 * @brief Get B&W mode enabled state
 * @param cam_id Camera ID
 * @param enabled Output: TRUE if B&W mode is enabled
 * @return HD_OK on success
 */
HD_RESULT get_bw_mode(int cam_id, BOOL *enabled);

/**
 * @brief Enable or disable B&W mode
 * @param cam_id Camera ID
 * @param enabled TRUE to enable B&W mode
 * @return HD_OK on success
 */
HD_RESULT set_bw_mode(int cam_id, BOOL enabled);

// ============================================================================
// Image Effect Functions
// ============================================================================

/**
 * @brief Image effect types
 */
typedef enum {
    IMAGE_EFFECT_OFF = 0,
    IMAGE_EFFECT_BLACKWHITE = 1,
    IMAGE_EFFECT_SEPIA = 2,
    IMAGE_EFFECT_VIVID = 3,
    IMAGE_EFFECT_ROCK = 4,
    IMAGE_EFFECT_COOL_GREEN = 5,
    IMAGE_EFFECT_WARM_YELLOW = 6,
    IMAGE_EFFECT_SKETCH = 7,
    IMAGE_EFFECT_COLOR_PENCIL = 8,
    IMAGE_EFFECT_RESERVED = 9,
    IMAGE_EFFECT_NEGATIVE_DEFOG = 10,
    IMAGE_EFFECT_CCID = 11,
    IMAGE_EFFECT_THERMAL_RED = 12,
    IMAGE_EFFECT_THERMAL_COLOR = 13
} ImageEffectType;

/**
 * @brief Get current image effect
 * @param cam_id Camera ID
 * @param effect Output: image effect type
 * @return HD_OK on success
 */
HD_RESULT get_image_effect(int cam_id, int *effect);

/**
 * @brief Set image effect
 * @param cam_id Camera ID
 * @param effect Image effect type (ImageEffectType)
 * @return HD_OK on success
 */
HD_RESULT set_image_effect(int cam_id, int effect);

// ============================================================================
// Rotation Mode Functions
// ============================================================================

/**
 * @brief Rotation mode types (includes horizontal flip combinations)
 */
typedef enum {
    ROTATION_0 = 0,
    ROTATION_90 = 1,
    ROTATION_180 = 2,
    ROTATION_270 = 3,
    ROTATION_HFLIP_0 = 4,
    ROTATION_HFLIP_90 = 5,
    ROTATION_HFLIP_180 = 6,
    ROTATION_HFLIP_270 = 7
} RotationModeType;

/**
 * @brief Get current rotation mode
 * @param cam_id Camera ID
 * @param mode Output: rotation mode
 * @return HD_OK on success
 */
HD_RESULT get_rotation_mode(int cam_id, int *mode);

/**
 * @brief Set rotation mode
 * @param cam_id Camera ID
 * @param mode Rotation mode (RotationModeType)
 * @return HD_OK on success
 */
HD_RESULT set_rotation_mode(int cam_id, int mode);

// ============================================================================
// AWB Scene Mode Functions
// ============================================================================

/**
 * @brief AWB scene mode types
 */
typedef enum {
    AWB_SCENE_AUTO = 0,
    AWB_SCENE_DAYLIGHT = 1,
    AWB_SCENE_CLOUDY = 2,
    AWB_SCENE_TUNGSTEN = 3,
    AWB_SCENE_SUNSET = 4,
    AWB_SCENE_CUSTOMER1 = 5,
    AWB_SCENE_CUSTOMER2 = 6,
    AWB_SCENE_CUSTOMER3 = 7,
    AWB_SCENE_CUSTOMER4 = 8,
    AWB_SCENE_CUSTOMER5 = 9,
    AWB_SCENE_NIGHT_MODE = 10,
    AWB_SCENE_MANUAL_GAIN = 11
} AWBSceneType;

/**
 * @brief Get current AWB scene mode
 * @param cam_id Camera ID
 * @param scene Output: AWB scene mode
 * @return HD_OK on success
 */
HD_RESULT get_awb_scene(int cam_id, int *scene);

/**
 * @brief Set AWB scene mode
 * @param cam_id Camera ID
 * @param scene AWB scene mode (AWBSceneType)
 * @return HD_OK on success
 */
HD_RESULT set_awb_scene(int cam_id, int scene);

// ============================================================================
// Manual AE (Auto Exposure) Functions
// ============================================================================

/**
 * @brief Get AE mode
 * @param cam_id Camera ID
 * @param auto_mode Output: TRUE if auto mode, FALSE if manual
 * @return HD_OK on success
 */
HD_RESULT get_ae_mode(int cam_id, BOOL *auto_mode);

/**
 * @brief Set AE mode
 * @param cam_id Camera ID
 * @param auto_mode TRUE for auto mode, FALSE for manual
 * @return HD_OK on success
 */
HD_RESULT set_ae_mode(int cam_id, BOOL auto_mode);

/**
 * @brief Get manual exposure time
 * @param cam_id Camera ID
 * @param time_us Output: exposure time in microseconds
 * @return HD_OK on success
 */
HD_RESULT get_manual_exposure_time(int cam_id, unsigned int *time_us);

/**
 * @brief Set manual exposure time
 * @param cam_id Camera ID
 * @param time_us Exposure time in microseconds
 * @return HD_OK on success
 */
HD_RESULT set_manual_exposure_time(int cam_id, unsigned int time_us);

/**
 * @brief Get manual gain (ISO)
 * @param cam_id Camera ID
 * @param gain Output: gain value (100 = 1x)
 * @return HD_OK on success
 */
HD_RESULT get_manual_gain(int cam_id, unsigned int *gain);

/**
 * @brief Set manual gain (ISO)
 * @param cam_id Camera ID
 * @param gain Gain value (100 = 1x)
 * @return HD_OK on success
 */
HD_RESULT set_manual_gain(int cam_id, unsigned int gain);

// ============================================================================
// ONVIF Authentication Validation
// ============================================================================

/**
 * @brief Validate ONVIF request access level
 * @param soap SOAP context
 * @param access_level Required access level
 * @param username Output username (optional, can be NULL)
 * @param password Output password (optional, can be NULL)
 * @return SOAP_OK on success, SOAP_FAULT on failure
 */
int ValidateAccessibility(struct soap *soap, int access_level, char *username, char *password);

#ifdef __cplusplus
}
#endif

#endif /* IMAGE_ADJUSTMENT_WRAPPER_H */
