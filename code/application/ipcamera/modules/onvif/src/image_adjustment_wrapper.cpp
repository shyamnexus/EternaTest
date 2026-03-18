/**
 * @file image_adjustment_wrapper.cpp
 * @brief C wrapper implementation for ISP control functions
 * 
 * This implements the C wrapper functions by calling into the C++ ISPControl class.
 * Following the same pattern as webserver handlers - no explicit init check needed
 * since ISPControl methods handle HDAL initialization internally.
 */

#include "ipcam/image_adjustment_wrapper.h"
#include "ipcam/isp_control.h"
#include <spdlog/spdlog.h>

using namespace ipcam::platform;

// Conversion between ONVIF scale (0-100) and Novatek scale (0-200)
// The C code expects Novatek scale
static inline int onvif_to_novatek(int val) {
    return val * 2;  // 0-100 -> 0-200
}

static inline int novatek_to_onvif(int val) {
    return val / 2;  // 0-200 -> 0-100
}

// ============================================================================
// Image Adjustment Functions
// Note: Following webserver handler pattern - ISPControl methods handle
// HDAL initialization internally, so no explicit Init() or IsInitialized()
// checks are needed here.
// ============================================================================

extern "C" HD_RESULT get_camera_brightness(int cam_id, int *value) {
    (void)cam_id;  // Currently single camera support
    
    if (!value) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    auto adj = isp.GetAdjustment();
    *value = onvif_to_novatek(adj.brightness);  // Convert 0-100 to 0-200
    return HD_OK;
}

extern "C" HD_RESULT set_camera_brightness(int cam_id, int value) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    int onvif_value = novatek_to_onvif(value);  // Convert 0-200 to 0-100
    if (isp.SetBrightness(onvif_value)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

extern "C" HD_RESULT get_camera_contrast(int cam_id, int *value) {
    (void)cam_id;
    
    if (!value) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    auto adj = isp.GetAdjustment();
    *value = onvif_to_novatek(adj.contrast);
    return HD_OK;
}

extern "C" HD_RESULT set_camera_contrast(int cam_id, int value) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    int onvif_value = novatek_to_onvif(value);
    if (isp.SetContrast(onvif_value)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

extern "C" HD_RESULT get_camera_saturation(int cam_id, int *value) {
    (void)cam_id;
    
    if (!value) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    auto adj = isp.GetAdjustment();
    *value = onvif_to_novatek(adj.saturation);
    return HD_OK;
}

extern "C" HD_RESULT set_camera_saturation(int cam_id, int value) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    int onvif_value = novatek_to_onvif(value);
    if (isp.SetSaturation(onvif_value)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

extern "C" HD_RESULT get_camera_sharpness(int cam_id, int *value) {
    (void)cam_id;
    
    if (!value) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    auto adj = isp.GetAdjustment();
    *value = onvif_to_novatek(adj.sharpness);
    return HD_OK;
}

extern "C" HD_RESULT set_camera_sharpness(int cam_id, int value) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    int onvif_value = novatek_to_onvif(value);
    if (isp.SetSharpness(onvif_value)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

// ============================================================================
// White Balance Functions
// ============================================================================

extern "C" HD_RESULT get_camera_wb_mode(int cam_id, int *mode) {
    (void)cam_id;
    
    if (!mode) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    auto wb = isp.GetWhiteBalance();
    *mode = (wb.mode == WBMode::Manual) ? 1 : 0;
    return HD_OK;
}

extern "C" HD_RESULT set_camera_wb_mode(int cam_id, int mode) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    WBMode wb_mode = (mode == 1) ? WBMode::Manual : WBMode::Auto;
    if (isp.SetWBMode(wb_mode)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

extern "C" HD_RESULT get_camera_manual_wb_gain(int cam_id, int *r_gain, int *g_gain, int *b_gain) {
    (void)cam_id;
    
    if (!r_gain || !g_gain || !b_gain) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    auto wb = isp.GetWhiteBalance();
    *r_gain = wb.r_gain;
    *g_gain = wb.g_gain;
    *b_gain = wb.b_gain;
    return HD_OK;
}

extern "C" HD_RESULT set_camera_manual_wb_gain(int cam_id, int r_gain, int g_gain, int b_gain) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    if (isp.SetRGBGain(r_gain, g_gain, b_gain)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

// ============================================================================
// Wide Dynamic Range (WDR) Functions
// ============================================================================

extern "C" HD_RESULT get_wdr_enable(int cam_id, BOOL *enabled) {
    (void)cam_id;
    
    if (!enabled) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    auto blc = isp.GetBLC();
    *enabled = blc.wdr_enabled ? TRUE : FALSE;
    return HD_OK;
}

extern "C" HD_RESULT set_wdr_enable(int cam_id, BOOL enabled) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    auto blc = isp.GetBLC();
    if (isp.SetWDR(enabled ? true : false, blc.wdr_level)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

extern "C" HD_RESULT get_manual_wdr_strength(int cam_id, int *strength) {
    (void)cam_id;
    
    if (!strength) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    auto blc = isp.GetBLC();
    *strength = blc.wdr_level;
    return HD_OK;
}

extern "C" HD_RESULT set_manual_wdr_strength(int cam_id, int strength) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    auto blc = isp.GetBLC();
    if (isp.SetWDR(blc.wdr_enabled, strength)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

// ============================================================================
// B&W Mode Functions
// ============================================================================

extern "C" HD_RESULT get_bw_mode(int cam_id, BOOL *enabled) {
    (void)cam_id;
    
    if (!enabled) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    *enabled = isp.GetBWMode() ? TRUE : FALSE;
    return HD_OK;
}

extern "C" HD_RESULT set_bw_mode(int cam_id, BOOL enabled) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    if (isp.SetBWMode(enabled ? true : false)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

// ============================================================================
// Image Effect Functions
// ============================================================================

extern "C" HD_RESULT get_image_effect(int cam_id, int *effect) {
    (void)cam_id;
    
    if (!effect) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    *effect = static_cast<int>(isp.GetImageEffect());
    return HD_OK;
}

extern "C" HD_RESULT set_image_effect(int cam_id, int effect) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    if (isp.SetImageEffect(static_cast<ImageEffect>(effect))) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

// ============================================================================
// Rotation Mode Functions
// ============================================================================

extern "C" HD_RESULT get_rotation_mode(int cam_id, int *mode) {
    (void)cam_id;
    
    if (!mode) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    *mode = static_cast<int>(isp.GetRotationMode());
    return HD_OK;
}

extern "C" HD_RESULT set_rotation_mode(int cam_id, int mode) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    if (isp.SetRotationMode(static_cast<RotationMode>(mode))) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

// ============================================================================
// AWB Scene Functions
// ============================================================================

extern "C" HD_RESULT get_awb_scene(int cam_id, int *scene) {
    (void)cam_id;
    
    if (!scene) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    *scene = static_cast<int>(isp.GetAWBScene());
    return HD_OK;
}

extern "C" HD_RESULT set_awb_scene(int cam_id, int scene) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    if (isp.SetAWBScene(static_cast<AWBScene>(scene))) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

// ============================================================================
// Manual AE Functions
// ============================================================================

extern "C" HD_RESULT get_ae_mode(int cam_id, BOOL *auto_mode) {
    (void)cam_id;
    
    if (!auto_mode) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    auto ae = isp.GetManualAE();
    *auto_mode = ae.manual_mode ? FALSE : TRUE;  // Inverted: manual_mode=true means auto=false
    return HD_OK;
}

extern "C" HD_RESULT set_ae_mode(int cam_id, BOOL auto_mode) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    if (isp.SetAEMode(auto_mode ? true : false)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

extern "C" HD_RESULT get_manual_exposure_time(int cam_id, unsigned int *time_us) {
    (void)cam_id;
    
    if (!time_us) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    *time_us = isp.GetManualExposureTime();
    return HD_OK;
}

extern "C" HD_RESULT set_manual_exposure_time(int cam_id, unsigned int time_us) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    if (isp.SetManualExposureTime(time_us)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

extern "C" HD_RESULT get_manual_gain(int cam_id, unsigned int *gain) {
    (void)cam_id;
    
    if (!gain) return HD_ERR_NG;
    
    auto& isp = ISPControl::Instance();
    *gain = isp.GetManualGain();
    return HD_OK;
}

extern "C" HD_RESULT set_manual_gain(int cam_id, unsigned int gain) {
    (void)cam_id;
    
    auto& isp = ISPControl::Instance();
    if (isp.SetManualGain(gain)) {
        return HD_OK;
    }
    return HD_ERR_NG;
}

// ============================================================================
// ONVIF Authentication Validation
// Note: This is defined in onvif_services.c, we just provide a declaration
// ============================================================================
// ValidateAccessibility is implemented in onvif_services.c
