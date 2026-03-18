/**
 * @file hdal_wrapper.cpp
 * @brief HDAL Wrapper Implementation
 * 
 * Implementation of the HDAL wrapper for Novatek ISP/Video/Audio APIs.
 * This file bridges the C++ interface to the C-based vendor_isp_* APIs.
 */

#include "ipcam/hdal_wrapper.h"
#include <spdlog/spdlog.h>
#include <mutex>
#include <cstring>

#if HDAL_ISP_ENABLED
// HDAL vendor ISP headers
extern "C" {
#include "vendor_isp.h"
#include "vendor_vpe.h"
}
#else
// Stub definitions when HDAL is not available
#define HD_OK 0
#define HD_RESULT int
#define TRUE 1
#define FALSE 0
typedef unsigned int UINT32;
typedef int IQ_ID;
typedef int AWB_ID;
typedef int AE_ID;
typedef int IQ_UI_BRIGHTNESS_LV;
typedef int IQ_UI_CONTRAST_LV;
typedef int IQ_UI_SATURATION_LV;
typedef int IQ_UI_SHARPNESS_LV;
typedef int IQ_UI_HUE_SHIFT;
typedef int IQ_UI_NR_LV;
typedef int IQ_UI_3DNR_LV;
typedef int IQ_UI_NIGHT_MODE;
typedef int AWB_SCENE;
typedef int AE_FREQUENCY;

// Stub enums
enum { AWB_SCENE_AUTO = 0, AWB_SCENE_DAYLIGHT, AWB_SCENE_CLOUDY, AWB_SCENE_TUNGSTEN, AWB_SCENE_SUNSET, AWB_SCENE_MGAIN };
enum { AE_FREQUENCY_50HZ = 0, AE_FREQUENCY_60HZ, AE_FREQUENCY_55HZ };
enum { IQ_UI_NIGHT_MODE_OFF = 0, IQ_UI_NIGHT_MODE_ON };
enum { 
    IQT_ITEM_BRIGHTNESS_LV = 0, IQT_ITEM_CONTRAST_LV, IQT_ITEM_SATURATION_LV, 
    IQT_ITEM_SHARPNESS_LV, IQT_ITEM_HUE_SHIFT, IQT_ITEM_NR_LV, IQT_ITEM_3DNR_LV, 
    IQT_ITEM_NIGHT_MODE, IQT_ITEM_WDR, IQT_ITEM_BNR, IQT_ITEM_TMNR,
    IQT_ITEM_COLORNR, IQT_ITEM_DEFOG, IQT_ITEM_CADJ,
    IQT_ITEM_GAMMA_LV, IQT_ITEM_DPC_PARAM, IQT_ITEM_SHADING_PARAM,
    IQT_ITEM_DARK_ENH_RATIO, IQT_ITEM_CONTRAST_ENH_RATIO,
    IQT_ITEM_GREEN_ENH_RATIO, IQT_ITEM_SKIN_ENH_RATIO,
    IQT_ITEM_TONE_LV, IQT_ITEM_WDR_ENH_PARAM
};
enum { AWBT_ITEM_SCENE = 0, AWBT_ITEM_WB_RATIO };
enum { 
    AET_ITEM_FREQUENCY = 0,
    AET_ITEM_METER = 1,
    AET_ITEM_METER_WIN = 8,       // Meter window weighting (8x8 grid)
    AET_ITEM_OVER_EXPOSURE = 10,  // HLC via over-exposure control
    AET_ITEM_MANUAL = 20
};
enum { ISPT_ITEM_SENSOR_DIRECTION = 0 };
enum { VPET_ITEM_FLIP_ROT_PARAM = 0 };

// VPE stub types
typedef int VPE_ID;
enum VPE_ISP_FLIP_ROT_MODE {
    VPE_ISP_ROTATE_0 = 0,
    VPE_ISP_ROTATE_90,
    VPE_ISP_ROTATE_180,
    VPE_ISP_ROTATE_270,
    VPE_ISP_H_FLIP_ROTATE_0,
    VPE_ISP_H_FLIP_ROTATE_90,
    VPE_ISP_H_FLIP_ROTATE_180,
    VPE_ISP_H_FLIP_ROTATE_270
};
struct VPE_MANUAL_ROT_PARAM { int ratio_x; int ratio_y; };
struct VPE_FLIP_ROT_PARAM { VPE_ISP_FLIP_ROT_MODE flip_rot_mode; VPE_MANUAL_ROT_PARAM rot_manual_param; };
struct VPET_FLIP_ROT_PARAM { VPE_ID id; VPE_FLIP_ROT_PARAM flip_rot; };
inline HD_RESULT vendor_vpe_set_cmd(int item, void* param) { (void)item; (void)param; return HD_OK; }
inline HD_RESULT vendor_vpe_get_cmd(int item, void* param) { (void)item; (void)param; return HD_OK; }

// Stub structs - Basic
struct ISP_SENSOR_DIRECTION { int mirror; int flip; };
struct IQT_BRIGHTNESS_LV { IQ_ID id; IQ_UI_BRIGHTNESS_LV lv; };
struct IQT_CONTRAST_LV { IQ_ID id; IQ_UI_CONTRAST_LV lv; };
struct IQT_SATURATION_LV { IQ_ID id; IQ_UI_SATURATION_LV lv; };
struct IQT_SHARPNESS_LV { IQ_ID id; IQ_UI_SHARPNESS_LV lv; };
struct IQT_HUE_SHIFT { IQ_ID id; IQ_UI_HUE_SHIFT hue_shift; };
struct IQT_NR_LV { IQ_ID id; IQ_UI_NR_LV lv; };
struct IQT_3DNR_LV { IQ_ID id; IQ_UI_3DNR_LV lv; };
struct IQT_NIGHT_MODE { IQ_ID id; IQ_UI_NIGHT_MODE mode; };
struct AWBT_SCENE_MODE { AWB_ID id; AWB_SCENE mode; };
struct AWBT_WB_RATIO { AWB_ID id; unsigned int r; unsigned int b; };
struct AET_FREQUENCY_MODE { unsigned int id; AE_FREQUENCY mode; };
struct ISPT_SENSOR_DIRECTION { unsigned int id; ISP_SENSOR_DIRECTION direction; };

// HLC (Highlight Compensation) - uses AE over-exposure control
// lum range: 0-255, smaller value = stronger HLC effect
typedef int AE_ID;  // AE_ID_1 = 0, AE_ID_2 = 1, etc.
#ifndef AEALG_DYNAMIC_LV_NUM
#define AEALG_DYNAMIC_LV_NUM 21  // Number of LV (Light Value) levels in AE tables
#endif
struct AE_OVER_EXPOSURE { 
    unsigned int enable;
    unsigned int lum;        // 0-255: luminance threshold, smaller = more HLC
    unsigned int speed;      // 0-5: adjustment speed (smaller = faster per doc)
    unsigned int tab_ratio[AEALG_DYNAMIC_LV_NUM];      // Effect ratio per LV [0-100]
    unsigned int tab_thr_mov[AEALG_DYNAMIC_LV_NUM];    // Max lum adjustment for movie [0-255]
    unsigned int tab_thr_photo[AEALG_DYNAMIC_LV_NUM];  // Max lum adjustment for photo [0-255]
    unsigned int tab_maxcnt[AEALG_DYNAMIC_LV_NUM];     // Upper bound of overexposed blocks [0-1023]
    unsigned int tab_mincnt[AEALG_DYNAMIC_LV_NUM];     // Lower bound of overexposed blocks [0-1023]
};
struct AET_OVER_EXPOSURE { AE_ID id; AE_OVER_EXPOSURE over_exposure; };

// BLC (Backlight Compensation) - uses AE metering mode
// SDK enum values: AE_METER_CENTERWEIGHTED=0, AE_METER_SPOT=1, AE_METER_MATRIX=2, AE_METER_ROI=7
typedef int AE_METER;
#define AE_METER_MATRIX 2          // Matrix/Average metering
#define AE_METER_CENTERWEIGHTED 0  // Center-weighted metering
#define AE_METER_SPOT 1            // Spot metering
#define AE_METER_ROI 7             // ROI metering
struct AET_METER_MODE { AE_ID id; AE_METER mode; };

// BLC (Backlight Compensation) - uses AE meter window (8x8 grid)
// Per vendor: "You can use meter window to accomplish" BLC
#define AE_WIN_NUM (8*8)  // 8x8 metering grid = 64 zones
#define AE_HIST_NUM 128   // Histogram bin count
#define ISP_SEN_MFRAME_MAX_NUM 4  // Max sensor multi-frame
struct AE_METER_WINDOW { unsigned int matrix[AE_WIN_NUM]; };
struct AET_METER_WINDOW { AE_ID id; AE_METER_WINDOW meter_window; };

// AE Status Info - for debugging HLC/BLC
struct AE_STATUS_INFO {
    unsigned int lv;
    unsigned int lv_base;
    unsigned long long ev;
    unsigned int ev_base;
    unsigned int lum;
    unsigned int hist_lum;
    unsigned int expotime[ISP_SEN_MFRAME_MAX_NUM];
    unsigned int iso_gain[ISP_SEN_MFRAME_MAX_NUM];
    unsigned int expect_lum;
    unsigned int overexp_adj;  // Current overexposure adjustment [0-255]
    unsigned int fps;
    unsigned int state_adj;
    unsigned int overexp_cnt;  // Current overexposure block count ratio (1=1/1000)
    unsigned int mf_num;
    unsigned int la_data[ISP_SEN_MFRAME_MAX_NUM][AE_WIN_NUM];
    unsigned int hist_data[ISP_SEN_MFRAME_MAX_NUM][AE_HIST_NUM];
    unsigned int aperture;
    unsigned int sensor_gain[ISP_SEN_MFRAME_MAX_NUM];
    unsigned int dgain[ISP_SEN_MFRAME_MAX_NUM];
};
struct AET_STATUS_INFO { AE_ID id; AE_STATUS_INFO status_info; };
enum { AET_ITEM_STATUS = 43, AET_ITEM_ROI_WIN = 45 };  // Status info and ROI window items

// BLC (Backlight Compensation) - uses AE ROI Window
// Per vendor sample (case 95 in vendor_ae_sample.c): ROI window for region-based metering
// start_x/y, end_x/y: region coordinates (0-100 percentage based)
// roi_center_w: weight for center of ROI (0-1023), higher = more weight
// roi_neighbor_w0: weight for immediate neighbors (0-1023)
// roi_neighbor_w1: weight for outer neighbors (0-1023)
enum AE_ROI_OPTION { ROI_ROUNDING = 0, ROI_EROSION = 1, ROI_DIALATION = 2 };
struct AE_ROI_WIN {
    unsigned int start_x;   // 0-99: x start position (percentage)
    unsigned int start_y;   // 0-99: y start position (percentage)
    unsigned int end_x;     // 1-100: x end position (percentage)
    unsigned int end_y;     // 1-100: y end position (percentage)
    AE_ROI_OPTION option;   // ROI calculation option
    unsigned int roi_center_w;    // 0-1023: weight for center region
    unsigned int roi_neighbor_w0; // 0-1023: weight for immediate neighbors
    unsigned int roi_neighbor_w1; // 0-1023: weight for outer area
};
struct AET_ROI_WIN { AE_ID id; AE_ROI_WIN roi_win; };

// Stub structs - Image Enhancement (WDR, NR, Defog)
struct IQ_WDR_AUTO_PARAM { unsigned int level; unsigned int strength_min; unsigned int strength_max; };
struct IQ_WDR { int enable; unsigned int subimg_size_h; unsigned int subimg_size_v; 
                unsigned int gain_protect_str; IQ_WDR_AUTO_PARAM auto_param[16]; };
struct IQT_WDR_PARAM { IQ_ID id; IQ_WDR wdr; };

struct IQ_BNR { int enable; unsigned int rng_a_thr[17]; unsigned int rng_a_lut[17];
                unsigned int rng_b_thr[17]; unsigned int rng_b_lut[17]; unsigned int blend_w; };
struct IQT_BNR_PARAM { IQ_ID id; IQ_BNR bnr; };

struct IQ_TMNR { int enable; unsigned int motion_th_y[16]; unsigned int motion_th_c[16];
                 unsigned int y_blend_wt[16]; unsigned int c_blend_wt[16]; unsigned int nr_str[16]; };
struct IQT_TMNR_PARAM { IQ_ID id; IQ_TMNR tmnr; };

struct IQ_COLORNR { int enable; unsigned int c_nr_str[16]; unsigned int spatial_wt[16]; 
                    unsigned int color_protect[16]; };
struct IQT_COLORNR_PARAM { IQ_ID id; IQ_COLORNR colornr; };

struct IQ_DEFOG_MANUAL { unsigned short fog_level; unsigned char fog_ratio; 
                          unsigned char gain_th; unsigned char outbld_lum_wt[17]; };
struct IQ_DEFOG_AUTO { unsigned short dr_th; unsigned char fog_ratio; unsigned char outbld_wt; };
struct IQ_DEFOG { int enable; int outbld_local_en; unsigned int outbld_diff_wt;
                  IQ_DEFOG_MANUAL manual_param; IQ_DEFOG_AUTO auto_param[16]; };
struct IQT_DEFOG_PARAM { IQ_ID id; IQ_DEFOG defog; };

struct IQ_CADJ_EE { int enable; unsigned int coring[16]; unsigned int edge_gain[16]; };
struct IQT_CADJ_PARAM { IQ_ID id; IQ_CADJ_EE ee; };

// Gamma level struct
typedef int IQ_UI_GAMMA_LV;
struct IQT_GAMMA_LV { IQ_ID id; IQ_UI_GAMMA_LV lv; };

// Tone level struct
typedef int IQ_UI_TONE_LV;
struct IQT_TONE_LV { IQ_ID id; IQ_UI_TONE_LV lv; };

// DPC (Dead Pixel Correction) struct
struct IQ_DPC { int enable; unsigned short table[128]; };
struct IQT_DPC_PARAM { IQ_ID id; IQ_DPC dpc; };

// Shading (Lens Shading Correction / ECS) struct
struct IQ_SHADING { int ecs_enable; int vig_enable; unsigned int mode;
                    int ecs_dthr_enable; unsigned int ecs_smooth_l_m_ct_lower;
                    unsigned int ecs_smooth_l_m_ct_upper; unsigned int ecs_smooth_m_h_ct_lower;
                    unsigned int ecs_smooth_m_h_ct_upper; };
struct IQT_SHADING_PARAM { IQ_ID id; IQ_SHADING shading; };

// WDR Enhancement struct (more detailed control)
#define IQ_WDR_STR_LEN 17
struct IQ_WDR_ENH { int enable; unsigned int enh_ratio[IQ_WDR_STR_LEN]; };
struct IQT_WDR_ENH_PARAM { IQ_ID id; IQ_WDR_ENH wdr_enh; };

// Enhancement ratio structs (stub - these use IQT_ITEM_NNSC_* via vendor_isp API)
typedef int IQ_NNSC_DARK_ENH_RATIO;
typedef int IQ_NNSC_CONTRAST_ENH_RATIO;
typedef int IQ_NNSC_GREEN_ENH_RATIO;
typedef int IQ_NNSC_SKIN_ENH_RATIO;
struct IQT_DARK_ENH_RATIO { IQ_ID id; IQ_NNSC_DARK_ENH_RATIO ratio; };
struct IQT_CONTRAST_ENH_RATIO { IQ_ID id; IQ_NNSC_CONTRAST_ENH_RATIO ratio; };
struct IQT_GREEN_ENH_RATIO { IQ_ID id; IQ_NNSC_GREEN_ENH_RATIO ratio; };
struct IQT_SKIN_ENH_RATIO { IQ_ID id; IQ_NNSC_SKIN_ENH_RATIO ratio; };

// NNSC item enum values (from iqt_api.h IQT_ITEM_NNSC_OFFSET = 300)
enum { 
    IQT_ITEM_NNSC_DARK_ENH_RATIO = 300, 
    IQT_ITEM_NNSC_CONTRAST_ENH_RATIO = 301, 
    IQT_ITEM_NNSC_GREEN_ENH_RATIO = 302, 
    IQT_ITEM_NNSC_SKIN_ENH_RATIO = 303 
};

// Stub functions
inline const char* vendor_isp_get_ver() { return "stub-1.0.0"; }
inline HD_RESULT vendor_isp_init() { return HD_OK; }
inline void vendor_isp_uninit() {}
inline HD_RESULT vendor_isp_set_iq(int, void*) { return HD_OK; }
inline HD_RESULT vendor_isp_get_iq(int, void*) { return HD_OK; }
inline HD_RESULT vendor_isp_set_awb(int, void*) { return HD_OK; }
inline HD_RESULT vendor_isp_get_awb(int, void*) { return HD_OK; }
inline HD_RESULT vendor_isp_set_ae(int, void*) { return HD_OK; }
inline HD_RESULT vendor_isp_get_ae(int, void*) { return HD_OK; }
inline HD_RESULT vendor_isp_set_common(int, void*) { return HD_OK; }
inline HD_RESULT vendor_isp_get_common(int, void*) { return HD_OK; }
#endif

namespace ipcam {
namespace platform {

namespace {
    std::mutex g_hdal_mutex;
}

// ============================================================================
// Singleton
// ============================================================================
HdalWrapper& HdalWrapper::Instance() {
    static HdalWrapper instance;
    return instance;
}

// ============================================================================
// Lifecycle
// ============================================================================
bool HdalWrapper::Init() {
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
    if (initialized_) {
        spdlog::debug("HdalWrapper already initialized");
        return true;
    }

    spdlog::info("Initializing HDAL Wrapper (vendor_isp)");

    // Try to get version first - if it succeeds, vendor_isp is already initialized
    // (e.g., by HdalPipeline). This avoids double-init issues.
    const char* ver = vendor_isp_get_ver();
    if (ver && strlen(ver) > 0) {
        spdlog::info("HDAL vendor ISP already active, version: {}", ver);
        initialized_ = true;
        return true;
    }

    // Not initialized yet, do it now
    HD_RESULT ret = vendor_isp_init();
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_init failed: {}", static_cast<int>(ret));
        return false;
    }

    ver = vendor_isp_get_ver();
    spdlog::info("HDAL vendor ISP initialized, version: {}", ver ? ver : "unknown");

    initialized_ = true;
    return true;
}

void HdalWrapper::Shutdown() {
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
    if (!initialized_) {
        return;
    }

    spdlog::info("Shutting down HDAL Wrapper");
    vendor_isp_uninit();
    initialized_ = false;
}

// ============================================================================
// Value Conversion Helpers
// ============================================================================
int HdalWrapper::ConvertToIqLevel(int value, int min_val, int normal_val, int max_val) {
    // value is 0-100, where 50 = normal
    // Map: 0 -> min_val, 50 -> normal_val, 100 -> max_val
    if (value <= 0) return min_val;
    if (value >= 100) return max_val;
    
    if (value < 50) {
        // Linear interpolation from min to normal
        return min_val + (normal_val - min_val) * value / 50;
    } else {
        // Linear interpolation from normal to max
        return normal_val + (max_val - normal_val) * (value - 50) / 50;
    }
}

int HdalWrapper::ConvertFromIqLevel(int level, int min_val, int normal_val, int max_val) {
    if (level <= min_val) return 0;
    if (level >= max_val) return 100;
    
    if (level < normal_val) {
        return 50 * (level - min_val) / (normal_val - min_val);
    } else {
        return 50 + 50 * (level - normal_val) / (max_val - normal_val);
    }
}

// ============================================================================
// Image Quality (IQ) Operations
// ============================================================================
HdalResult HdalWrapper::SetBrightness(IspId isp_id, int value) {
    if (!initialized_) return HdalResult::kNotInitialized;
    if (value < 0 || value > 100) return HdalResult::kInvalidParam;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    // Map 0-100 to IQ_UI_BRIGHTNESS_LV range (0-200, 100=normal)
    // IQ_UI_BRIGHTNESS_LV: MIN=-1, N1=50, NORMAL=100, P1=150, MAX=201
    int iq_level = ConvertToIqLevel(value, 0, 100, 200);

    IQT_BRIGHTNESS_LV brightness_param;
    brightness_param.id = static_cast<IQ_ID>(isp_id);
    brightness_param.lv = static_cast<IQ_UI_BRIGHTNESS_LV>(iq_level);

    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_BRIGHTNESS_LV, &brightness_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(BRIGHTNESS_LV) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set brightness: value={} -> iq_level={}", value, iq_level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetContrast(IspId isp_id, int value) {
    if (!initialized_) return HdalResult::kNotInitialized;
    if (value < 0 || value > 100) return HdalResult::kInvalidParam;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    int iq_level = ConvertToIqLevel(value, 0, 100, 200);

    IQT_CONTRAST_LV contrast_param;
    contrast_param.id = static_cast<IQ_ID>(isp_id);
    contrast_param.lv = static_cast<IQ_UI_CONTRAST_LV>(iq_level);

    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_CONTRAST_LV, &contrast_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(CONTRAST_LV) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set contrast: value={} -> iq_level={}", value, iq_level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetSaturation(IspId isp_id, int value) {
    if (!initialized_) return HdalResult::kNotInitialized;
    if (value < 0 || value > 100) return HdalResult::kInvalidParam;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    int iq_level = ConvertToIqLevel(value, 0, 100, 200);

    IQT_SATURATION_LV saturation_param;
    saturation_param.id = static_cast<IQ_ID>(isp_id);
    saturation_param.lv = static_cast<IQ_UI_SATURATION_LV>(iq_level);

    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_SATURATION_LV, &saturation_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(SATURATION_LV) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set saturation: value={} -> iq_level={}", value, iq_level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetSharpness(IspId isp_id, int value) {
    if (!initialized_) return HdalResult::kNotInitialized;
    if (value < 0 || value > 100) return HdalResult::kInvalidParam;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    int iq_level = ConvertToIqLevel(value, 0, 100, 200);

    IQT_SHARPNESS_LV sharpness_param;
    sharpness_param.id = static_cast<IQ_ID>(isp_id);
    sharpness_param.lv = static_cast<IQ_UI_SHARPNESS_LV>(iq_level);

    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_SHARPNESS_LV, &sharpness_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(SHARPNESS_LV) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set sharpness: value={} -> iq_level={}", value, iq_level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetHue(IspId isp_id, int value) {
    if (!initialized_) return HdalResult::kNotInitialized;
    if (value < 0 || value > 100) return HdalResult::kInvalidParam;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    // IQ_UI_HUE_SHIFT: -180 to +180 degrees
    // Map 0-100 to -180 to +180 (-180 + value * 3.6)
    int hue_shift = static_cast<int>(-180 + value * 3.6);

    IQT_HUE_SHIFT hue_param;
    hue_param.id = static_cast<IQ_ID>(isp_id);
    hue_param.hue_shift = static_cast<IQ_UI_HUE_SHIFT>(hue_shift);

    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_HUE_SHIFT, &hue_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(HUE_SHIFT) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set hue: value={} -> hue_shift={}", value, hue_shift);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetNrLevel(IspId isp_id, int value) {
    if (!initialized_) return HdalResult::kNotInitialized;
    if (value < 0 || value > 100) return HdalResult::kInvalidParam;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    int iq_level = ConvertToIqLevel(value, 0, 100, 200);

    IQT_NR_LV nr_param;
    nr_param.id = static_cast<IQ_ID>(isp_id);
    nr_param.lv = static_cast<IQ_UI_NR_LV>(iq_level);

    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_NR_LV, &nr_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(NR_LV) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set NR level: value={} -> iq_level={}", value, iq_level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::Set3dNrLevel(IspId isp_id, int value) {
    if (!initialized_) return HdalResult::kNotInitialized;
    if (value < 0 || value > 100) return HdalResult::kInvalidParam;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    int iq_level = ConvertToIqLevel(value, 0, 100, 200);

    IQT_3DNR_LV nr3d_param;
    nr3d_param.id = static_cast<IQ_ID>(isp_id);
    nr3d_param.lv = static_cast<IQ_UI_3DNR_LV>(iq_level);

    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_3DNR_LV, &nr3d_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(3DNR_LV) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set 3DNR level: value={} -> iq_level={}", value, iq_level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetIqParams(IspId isp_id, const IqParams& params) {
    HdalResult result = HdalResult::kOk;
    
    if (SetBrightness(isp_id, params.brightness) != HdalResult::kOk) {
        spdlog::warn("Failed to set brightness");
        result = HdalResult::kError;
    }
    if (SetContrast(isp_id, params.contrast) != HdalResult::kOk) {
        spdlog::warn("Failed to set contrast");
        result = HdalResult::kError;
    }
    if (SetSaturation(isp_id, params.saturation) != HdalResult::kOk) {
        spdlog::warn("Failed to set saturation");
        result = HdalResult::kError;
    }
    if (SetSharpness(isp_id, params.sharpness) != HdalResult::kOk) {
        spdlog::warn("Failed to set sharpness");
        result = HdalResult::kError;
    }
    if (SetHue(isp_id, params.hue) != HdalResult::kOk) {
        spdlog::warn("Failed to set hue");
        result = HdalResult::kError;
    }
    if (SetNrLevel(isp_id, params.nr_level) != HdalResult::kOk) {
        spdlog::warn("Failed to set NR level");
        result = HdalResult::kError;
    }
    if (Set3dNrLevel(isp_id, params.nr_3d_level) != HdalResult::kOk) {
        spdlog::warn("Failed to set 3DNR level");
        result = HdalResult::kError;
    }

    return result;
}

HdalResult HdalWrapper::GetIqParams(IspId isp_id, IqParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;

    // Note: Getting IQ params requires using vendor_isp_get_iq
    // For now, return cached values or default
    // This would need implementation based on actual requirements
    spdlog::debug("GetIqParams: Using default/cached values");
    return HdalResult::kOk;
}

// ============================================================================
// White Balance Operations
// ============================================================================
HdalResult HdalWrapper::SetWbScene(IspId isp_id, WbScene scene) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    AWB_SCENE awb_scene;
    switch (scene) {
        case WbScene::kAuto:     awb_scene = AWB_SCENE_AUTO; break;
        case WbScene::kDaylight: awb_scene = AWB_SCENE_DAYLIGHT; break;
        case WbScene::kCloudy:   awb_scene = AWB_SCENE_CLOUDY; break;
        case WbScene::kTungsten: awb_scene = AWB_SCENE_TUNGSTEN; break;
        case WbScene::kSunset:   awb_scene = AWB_SCENE_SUNSET; break;
        case WbScene::kManual:   awb_scene = AWB_SCENE_MGAIN; break;
        default: return HdalResult::kInvalidParam;
    }

    AWBT_SCENE_MODE scene_param;
    scene_param.id = static_cast<AWB_ID>(isp_id);
    scene_param.mode = awb_scene;

    HD_RESULT ret = vendor_isp_set_awb(AWBT_ITEM_SCENE, &scene_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_awb(SCENE) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set WB scene: {}", static_cast<int>(awb_scene));
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetWbGains(IspId isp_id, uint32_t r_ratio, uint32_t b_ratio) {
    if (!initialized_) return HdalResult::kNotInitialized;
    if (r_ratio < 50 || r_ratio > 200 || b_ratio < 50 || b_ratio > 200) {
        return HdalResult::kInvalidParam;
    }

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    AWBT_WB_RATIO ratio_param;
    ratio_param.id = static_cast<AWB_ID>(isp_id);
    ratio_param.r = r_ratio;
    ratio_param.b = b_ratio;

    HD_RESULT ret = vendor_isp_set_awb(AWBT_ITEM_WB_RATIO, &ratio_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_awb(WB_RATIO) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set WB gains: R={}, B={}", r_ratio, b_ratio);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetWbParams(IspId isp_id, const WbParams& params) {
    HdalResult result = SetWbScene(isp_id, params.scene);
    if (result != HdalResult::kOk) {
        return result;
    }

    if (params.scene == WbScene::kManual) {
        result = SetWbGains(isp_id, params.r_ratio, params.b_ratio);
    }

    return result;
}

HdalResult HdalWrapper::GetWbParams(IspId isp_id, WbParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    // Note: Getting WB params requires vendor_isp_get_awb
    spdlog::debug("GetWbParams: Using default/cached values");
    return HdalResult::kOk;
}

// ============================================================================
// AE (Auto Exposure) Operations  
// ============================================================================
HdalResult HdalWrapper::SetFlickerMode(IspId isp_id, HdalFlickerMode mode) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    AE_FREQUENCY ae_freq;
    switch (mode) {
        case HdalFlickerMode::kAuto: ae_freq = AE_FREQUENCY_50HZ; break; // Auto defaults to 50Hz
        case HdalFlickerMode::k50Hz: ae_freq = AE_FREQUENCY_50HZ; break;
        case HdalFlickerMode::k60Hz: ae_freq = AE_FREQUENCY_60HZ; break;
        case HdalFlickerMode::k55Hz: ae_freq = AE_FREQUENCY_55HZ; break;
        default: return HdalResult::kInvalidParam;
    }

    AET_FREQUENCY_MODE freq_param;
    freq_param.id = static_cast<uint32_t>(isp_id);
    freq_param.mode = ae_freq;

    HD_RESULT ret = vendor_isp_set_ae(AET_ITEM_FREQUENCY, &freq_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_ae(FREQUENCY) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set flicker mode: {}", static_cast<int>(ae_freq));
    return HdalResult::kOk;
}

HdalResult HdalWrapper::GetFlickerMode(IspId isp_id, HdalFlickerMode& mode) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    AET_FREQUENCY_MODE freq_param;
    freq_param.id = static_cast<uint32_t>(isp_id);

    HD_RESULT ret = vendor_isp_get_ae(AET_ITEM_FREQUENCY, &freq_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_get_ae(FREQUENCY) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    switch (freq_param.mode) {
        case AE_FREQUENCY_50HZ: mode = HdalFlickerMode::k50Hz; break;
        case AE_FREQUENCY_60HZ: mode = HdalFlickerMode::k60Hz; break;
        case AE_FREQUENCY_55HZ: mode = HdalFlickerMode::k55Hz; break;
        default: mode = HdalFlickerMode::kAuto; break;
    }

    return HdalResult::kOk;
}

// ============================================================================
// Sensor Direction (Mirror/Flip) Operations
// ============================================================================
HdalResult HdalWrapper::SetSensorDirection(IspId isp_id, const SensorDirection& dir) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    ISPT_SENSOR_DIRECTION sensor_dir;
    sensor_dir.id = static_cast<uint32_t>(isp_id);
    sensor_dir.direction.mirror = dir.mirror ? TRUE : FALSE;
    sensor_dir.direction.flip = dir.flip ? TRUE : FALSE;

    HD_RESULT ret = vendor_isp_set_common(ISPT_ITEM_SENSOR_DIRECTION, &sensor_dir);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_common(SENSOR_DIRECTION) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set sensor direction: mirror={}, flip={}", dir.mirror, dir.flip);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::GetSensorDirection(IspId isp_id, SensorDirection& dir) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    ISPT_SENSOR_DIRECTION sensor_dir;
    sensor_dir.id = static_cast<uint32_t>(isp_id);

    HD_RESULT ret = vendor_isp_get_common(ISPT_ITEM_SENSOR_DIRECTION, &sensor_dir);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_get_common(SENSOR_DIRECTION) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    dir.mirror = sensor_dir.direction.mirror != 0;
    dir.flip = sensor_dir.direction.flip != 0;

    return HdalResult::kOk;
}

// ============================================================================
// ISP Feature Control - Image Enhancement
// ============================================================================
HdalResult HdalWrapper::SetWdrEnable(IspId isp_id, bool enable) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
#if HDAL_ISP_ENABLED
    IQT_WDR_PARAM wdr_param;
    memset(&wdr_param, 0, sizeof(wdr_param));
    wdr_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_WDR_PARAM, &wdr_param);
    if (ret == HD_OK) {
        wdr_param.wdr.enable = enable ? TRUE : FALSE;
        ret = vendor_isp_set_iq(IQT_ITEM_WDR_PARAM, &wdr_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(WDR_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(WDR_PARAM) failed: {}", static_cast<int>(ret));
    }
#endif
    
    spdlog::debug("SetWdrEnable: {}", enable);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetWdrParams(IspId isp_id, const WdrParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
    // Store user's WDR settings
    wdr_enabled_ = params.enable;
    wdr_strength_ = params.strength;
    
#if HDAL_ISP_ENABLED
    IQT_WDR_PARAM wdr_param;
    memset(&wdr_param, 0, sizeof(wdr_param));
    wdr_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_WDR_PARAM, &wdr_param);
    if (ret == HD_OK) {
        // WDR should be enabled if either WDR or BLC is enabled
        bool should_enable = wdr_enabled_ || blc_enabled_;
        wdr_param.wdr.enable = should_enable ? TRUE : FALSE;
        
        // Use the higher of WDR strength or BLC level
        int effective_strength = wdr_enabled_ ? wdr_strength_ : 0;
        if (blc_enabled_ && blc_level_ > effective_strength) {
            effective_strength = blc_level_;
        }
        
        // Map 0-100 strength to 0-255 range
        int strength_255 = effective_strength * 255 / 100;
        wdr_param.wdr.subimg_size_h = params.subimg_h > 0 ? params.subimg_h : 32;
        wdr_param.wdr.subimg_size_v = params.subimg_v > 0 ? params.subimg_v : 32;
        wdr_param.wdr.gain_protect_str = params.color_protect * 256 / 100;
        
        // Configure auto params for different gain levels
        for (int i = 0; i < IQ_GAIN_ID_MAX_NUM && i < 16; i++) {
            wdr_param.wdr.auto_param[i].level = strength_255;
            wdr_param.wdr.auto_param[i].strength_min = strength_255 / 4;
            wdr_param.wdr.auto_param[i].strength_max = strength_255;
        }
        
        spdlog::info("SetWdrParams: user_wdr={} user_blc={} effective_enable={} strength={}", 
                     wdr_enabled_, blc_enabled_, should_enable, effective_strength);
        
        ret = vendor_isp_set_iq(IQT_ITEM_WDR_PARAM, &wdr_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(WDR_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(WDR_PARAM) failed: {}", static_cast<int>(ret));
    }
#endif
    
    spdlog::debug("SetWdrParams: enable={} strength={} color_protect={}", 
                  params.enable, params.strength, params.color_protect);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::GetWdrParams(IspId isp_id, WdrParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
#if HDAL_ISP_ENABLED
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
    IQT_WDR_PARAM wdr_param;
    memset(&wdr_param, 0, sizeof(wdr_param));
    wdr_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_WDR_PARAM, &wdr_param);
    if (ret == HD_OK) {
        params.enable = wdr_param.wdr.enable != 0;
        params.strength = wdr_param.wdr.auto_param[0].strength_max * 100 / 255;
        params.color_protect = wdr_param.wdr.gain_protect_str * 100 / 256;
        params.subimg_h = wdr_param.wdr.subimg_size_h;
        params.subimg_v = wdr_param.wdr.subimg_size_v;
    }
#endif
    
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetHdrParams(IspId isp_id, const HdrParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    // HDR is configured at sensor/pipeline level during initialization
    // via SHDR (Staggered HDR) parameters
    spdlog::debug("SetHdrParams: enable={} mode={} strength={}", 
                  params.enable, params.mode, params.strength);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetBnr2dParams(IspId isp_id, const Bnr2dParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
#if HDAL_ISP_ENABLED
    // Use IQT_ITEM_NR_PARAM for 2D spatial noise reduction
    IQT_NR_PARAM nr_param;
    memset(&nr_param, 0, sizeof(nr_param));
    nr_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_NR_PARAM, &nr_param);
    if (ret == HD_OK) {
        nr_param.nr.filter_enable = params.enable ? TRUE : FALSE;
        nr_param.nr.gbal_enable = params.enable ? TRUE : FALSE;
        nr_param.nr.lca_enable = params.enable ? TRUE : FALSE;
        
        ret = vendor_isp_set_iq(IQT_ITEM_NR_PARAM, &nr_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(NR_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(NR_PARAM) failed: {}", static_cast<int>(ret));
        // Fallback to level-based API
        return SetNrLevel(isp_id, params.enable ? params.strength : 0);
    }
#else
    // Fallback to simple NR level for stub mode
    return SetNrLevel(isp_id, params.enable ? params.strength : 0);
#endif
    
    spdlog::debug("SetBnr2dParams: enable={} strength={}", params.enable, params.strength);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetTmnr3dParams(IspId isp_id, const Tmnr3dParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
#if HDAL_ISP_ENABLED
    IQT_3DNR_PARAM tmnr_param;
    memset(&tmnr_param, 0, sizeof(tmnr_param));
    tmnr_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_3DNR_PARAM, &tmnr_param);
    if (ret == HD_OK) {
        tmnr_param._3dnr.enable = params.enable ? TRUE : FALSE;
        tmnr_param._3dnr.fcvg_enable = TRUE;
        
        // Configure per gain level
        for (int i = 0; i < IQ_GAIN_ID_MAX_NUM && i < 16; i++) {
            // Pre-filter strength based on overall strength
            tmnr_param._3dnr.auto_param[i].pf_str = params.strength * 255 / 100;
            
            // Temporal filter strengths
            int tf_str = params.strength * 128 / 100;
            tmnr_param._3dnr.auto_param[i].tf0_y_str[0] = tf_str;  // static flat
            tmnr_param._3dnr.auto_param[i].tf0_y_str[1] = tf_str / 2;  // transition
            tmnr_param._3dnr.auto_param[i].tf0_y_str[2] = tf_str / 4;  // motion
            tmnr_param._3dnr.auto_param[i].tf0_c_str[0] = tf_str;
            tmnr_param._3dnr.auto_param[i].tf0_c_str[1] = tf_str / 2;
            tmnr_param._3dnr.auto_param[i].tf0_c_str[2] = tf_str / 4;
            
            // SNR/TNR strengths (1X = 64)
            int snr_str = 64 + params.strength * 64 / 100;
            tmnr_param._3dnr.auto_param[i].snr_str[0] = snr_str;
            tmnr_param._3dnr.auto_param[i].snr_str[1] = snr_str * 3 / 4;
            tmnr_param._3dnr.auto_param[i].snr_str[2] = snr_str / 2;
            tmnr_param._3dnr.auto_param[i].tnr_str[0] = snr_str;
            tmnr_param._3dnr.auto_param[i].tnr_str[1] = snr_str * 3 / 4;
            tmnr_param._3dnr.auto_param[i].tnr_str[2] = snr_str / 2;
        }
        
        ret = vendor_isp_set_iq(IQT_ITEM_3DNR_PARAM, &tmnr_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(3DNR_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(3DNR_PARAM) failed: {}", static_cast<int>(ret));
        // Fallback to level-based API
        return Set3dNrLevel(isp_id, params.enable ? params.strength : 0);
    }
#else
    // Fallback to simple 3DNR level for stub mode
    return Set3dNrLevel(isp_id, params.enable ? params.strength : 0);
#endif
    
    spdlog::debug("SetTmnr3dParams: enable={} strength={} motion_th={}", 
                  params.enable, params.strength, params.motion_threshold);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetColorNrParams(IspId isp_id, const ColorNrParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
#if HDAL_ISP_ENABLED
    // Color NR (LCA) is part of IQ_NR_PARAM
    IQT_NR_PARAM nr_param;
    memset(&nr_param, 0, sizeof(nr_param));
    nr_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_NR_PARAM, &nr_param);
    if (ret == HD_OK) {
        nr_param.nr.lca_enable = params.enable ? TRUE : FALSE;
        
        ret = vendor_isp_set_iq(IQT_ITEM_NR_PARAM, &nr_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(NR_PARAM for ColorNR) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(NR_PARAM) for ColorNR failed: {}", static_cast<int>(ret));
    }
#endif
    
    spdlog::debug("SetColorNrParams: enable={} strength={}", params.enable, params.strength);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetDefogEnable(IspId isp_id, bool enable) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
#if HDAL_ISP_ENABLED
    IQT_DEFOG_PARAM defog_param;
    memset(&defog_param, 0, sizeof(defog_param));
    defog_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_DEFOG_PARAM, &defog_param);
    if (ret == HD_OK) {
        defog_param.defog.enable = enable ? TRUE : FALSE;
        ret = vendor_isp_set_iq(IQT_ITEM_DEFOG_PARAM, &defog_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(DEFOG_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(DEFOG_PARAM) failed: {}", static_cast<int>(ret));
    }
#endif
    
    spdlog::debug("SetDefogEnable: {}", enable);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetDefogParams(IspId isp_id, const DefogParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
#if HDAL_ISP_ENABLED
    IQT_DEFOG_PARAM defog_param;
    memset(&defog_param, 0, sizeof(defog_param));
    defog_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_DEFOG_PARAM, &defog_param);
    if (ret == HD_OK) {
        defog_param.defog.enable = params.enable ? TRUE : FALSE;
        defog_param.defog.outbld_local_en = TRUE;
        defog_param.defog.outbld_diff_wt = 3;
        defog_param.defog.min_diff_ratio = 8;
        
        // Manual parameters
        defog_param.defog.manual_param.fog_level = 256 + params.fog_level * 767 / 100;
        defog_param.defog.manual_param.fog_ratio = params.strength * 255 / 100;
        defog_param.defog.manual_param.gain_th = 128;
        
        // Luminance-based blend weights
        for (int i = 0; i < IQ_DEFOG_OUTPUT_BLD_LEN && i < 17; i++) {
            if (i < 8) {
                defog_param.defog.manual_param.outbld_lum_wt[i] = 0;
            } else if (i < 12) {
                defog_param.defog.manual_param.outbld_lum_wt[i] = 64;
            } else {
                defog_param.defog.manual_param.outbld_lum_wt[i] = 192 + (i - 12) * 16;
            }
        }
        
        // Auto parameters per gain level
        for (int i = 0; i < IQ_GAIN_ID_MAX_NUM && i < 16; i++) {
            defog_param.defog.auto_param[i].dr_th = 40;
            defog_param.defog.auto_param[i].fog_level_max = 512 + params.strength * 511 / 100;
            defog_param.defog.auto_param[i].fog_ratio = params.strength * 255 / 100;
            defog_param.defog.auto_param[i].outbld_wt = 128;
        }
        
        ret = vendor_isp_set_iq(IQT_ITEM_DEFOG_PARAM, &defog_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(DEFOG_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(DEFOG_PARAM) failed: {}", static_cast<int>(ret));
    }
#endif
    
    spdlog::debug("SetDefogParams: enable={} strength={} fog_level={}", 
                  params.enable, params.strength, params.fog_level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetEdgeEnhanceParams(IspId isp_id, const EdgeEnhanceParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;
    
    std::lock_guard<std::mutex> lock(g_hdal_mutex);
    
#if HDAL_ISP_ENABLED
    IQT_EDGE_PARAM edge_param;
    memset(&edge_param, 0, sizeof(edge_param));
    edge_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_EDGE_PARAM, &edge_param);
    if (ret == HD_OK) {
        edge_param.edge.enable = params.enable ? TRUE : FALSE;
        
        // Overshoot/undershoot thresholds
        edge_param.edge.th_overshoot = 128 + (100 - params.coring) * 127 / 100;
        edge_param.edge.th_undershoot = 128 + (100 - params.coring) * 127 / 100;
        
        // Configure per gain level
        for (int i = 0; i < IQ_GAIN_ID_MAX_NUM && i < 16; i++) {
            // Edge enhance weighting (1X = 64)
            int edge_w = 64 + params.strength * 64 / 100;
            edge_param.edge.auto_param[i].edge_enh_p = edge_w;
            edge_param.edge.auto_param[i].edge_enh_n = edge_w;
            
            // Strength for flat and edge regions
            edge_param.edge.auto_param[i].str_flat = 64 - (params.coring * 32 / 100);
            edge_param.edge.auto_param[i].str_edge = 64 + (params.strength * 64 / 100);
        }
        
        ret = vendor_isp_set_iq(IQT_ITEM_EDGE_PARAM, &edge_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(EDGE_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(EDGE_PARAM) failed: {}", static_cast<int>(ret));
        // Fallback to sharpness control
        return SetSharpness(isp_id, params.enable ? params.strength : 50);
    }
#else
    // Fallback to sharpness control for stub mode
    return SetSharpness(isp_id, params.enable ? params.strength : 50);
#endif
    
    spdlog::debug("SetEdgeEnhanceParams: enable={} strength={} coring={}", 
                  params.enable, params.strength, params.coring);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetBlcParams(IspId isp_id, const BlcParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    spdlog::info("SetBlcParams: enable={} level={}", params.enable, params.level);

    // Store user's BLC settings
    blc_enabled_ = params.enable;
    blc_level_ = params.level;

#if HDAL_ISP_ENABLED
    // =========================================================================
    // BLC (Backlight Compensation) Implementation:
    // Per Novatek support: "Most clients use WDR for BLC"
    // 
    // BLC and WDR share the same hardware. We track both settings and combine:
    // - WDR enable = wdr_enabled_ OR blc_enabled_
    // - Strength = max(wdr_strength_, blc_level_) when either is enabled
    // =========================================================================
    
    HD_RESULT ret;
    
    IQT_WDR_PARAM wdr_param;
    memset(&wdr_param, 0, sizeof(wdr_param));
    wdr_param.id = static_cast<IQ_ID>(isp_id);
    
    ret = vendor_isp_get_iq(IQT_ITEM_WDR_PARAM, &wdr_param);
    if (ret != HD_OK) {
        spdlog::warn("BLC: vendor_isp_get_iq(WDR_PARAM) failed: ret={}", static_cast<int>(ret));
        memset(&wdr_param.wdr, 0, sizeof(wdr_param.wdr));
    }
    
    // WDR should be enabled if either WDR or BLC is enabled
    bool should_enable = wdr_enabled_ || blc_enabled_;
    wdr_param.wdr.enable = should_enable ? TRUE : FALSE;
    
    // Calculate effective strength: use the higher of WDR or BLC level
    int effective_strength = 0;
    if (wdr_enabled_) {
        effective_strength = wdr_strength_;
    }
    if (blc_enabled_ && blc_level_ > effective_strength) {
        effective_strength = blc_level_;
    }
    
    // Map 0-100 strength to 0-255 range
    int strength_255 = (effective_strength * 255) / 100;
    
    // Configure WDR parameters
    wdr_param.wdr.subimg_size_h = 32;
    wdr_param.wdr.subimg_size_v = 32;
    wdr_param.wdr.gain_protect_str = 128;  // Moderate color protection
    
    // Set auto params for all gain levels
    for (int i = 0; i < 16; i++) {
        wdr_param.wdr.auto_param[i].level = strength_255;
        wdr_param.wdr.auto_param[i].strength_min = strength_255 / 4;
        wdr_param.wdr.auto_param[i].strength_max = strength_255;
    }
    
    spdlog::info("BLC: user_wdr={}/{} user_blc={}/{} -> enable={} strength={}", 
                 wdr_enabled_, wdr_strength_, blc_enabled_, blc_level_, 
                 should_enable, effective_strength);
    
    ret = vendor_isp_set_iq(IQT_ITEM_WDR_PARAM, &wdr_param);
    if (ret != HD_OK) {
        spdlog::error("BLC: vendor_isp_set_iq(WDR_PARAM) failed: ret={}", static_cast<int>(ret));
        return HdalResult::kError;
    }
    
    spdlog::info("BLC: WDR_PARAM set successfully");
    
#else
    spdlog::warn("BLC: HDAL_ISP_ENABLED=0, vendor APIs not available (stub mode)");
#endif

    spdlog::info("BLC applied successfully: enable={} level={}", params.enable, params.level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetHlcParams(IspId isp_id, const HlcParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    spdlog::info("SetHlcParams: enable={} level={}", params.enable, params.level);

#if HDAL_ISP_ENABLED
    spdlog::info("HLC: HDAL_ISP_ENABLED=1, calling vendor APIs");
    
    // HLC (Highlight Compensation) Implementation:
    // Per vendor guidance (case 60 in vendor_ae_sample.c), use AET_ITEM_OVER_EXPOSURE
    // to control highlight suppression.
    //
    // Key parameter: over_exposure.lum (range: 0-255)
    // - The SMALLER the value, the MORE obvious the HLC effect
    // - Value 255 = minimal HLC (highlights not suppressed)
    // - Value 0 = maximum HLC (highlights strongly suppressed)
    //
    // This works by telling the AE algorithm to reduce exposure when
    // highlights exceed the lum threshold.
    
    AET_OVER_EXPOSURE over_exp;
    memset(&over_exp, 0, sizeof(over_exp));
    over_exp.id = static_cast<AE_ID>(isp_id);
    
    // First GET current values to preserve any existing tuning
    HD_RESULT ret = vendor_isp_get_ae(AET_ITEM_OVER_EXPOSURE, &over_exp);
    if (ret != HD_OK) {
        spdlog::warn("vendor_isp_get_ae(OVER_EXPOSURE) failed: {}, using defaults", static_cast<int>(ret));
        // Initialize with safe defaults if GET fails
        over_exp.over_exposure.enable = 0;
        over_exp.over_exposure.lum = 255;
        over_exp.over_exposure.speed = 3;
    }
    
    spdlog::info("HLC current values: enable={} lum={} speed={}", 
                 over_exp.over_exposure.enable, 
                 over_exp.over_exposure.lum,
                 over_exp.over_exposure.speed);
    
    if (params.enable) {
        over_exp.over_exposure.enable = 1;
        
        // Map UI level (0-100) to lum value
        // Per vendor documentation (ISP_3A_Module_User_Guide):
        // - lum: Overexposure brightness threshold [0-255]
        // - If brightness exceeds this threshold, the block is considered overexposed
        // - SMALLER lum = MORE pixels considered overexposed = STRONGER HLC effect
        // 
        // UI level 0 = minimal HLC = lum 200 (only very bright pixels suppressed)
        // UI level 100 = maximum HLC = lum 60 (aggressive suppression)
        //
        // More aggressive mapping to make HLC effect more visible
        int lum_value = 200 - ((params.level * 140) / 100);  // Range: 200 to 60
        if (lum_value < 60) lum_value = 60;
        if (lum_value > 200) lum_value = 200;
        
        over_exp.over_exposure.lum = static_cast<UINT32>(lum_value);
        
        // Speed: how fast to adjust (0-5, smaller = faster according to doc)
        // Use speed=2 for reasonably fast response
        over_exp.over_exposure.speed = 2;
        
        // Configure the threshold arrays - these are critical for the algorithm to work!
        // Per ISP_3A_Module_User_Guide:
        // - tab_ratio: Effect ratio per LV level [0-100], 100 = full effect
        // - tab_maxcnt: When overexposed block count exceeds this, algorithm lowers luminance
        // - tab_mincnt: When count falls below this, algorithm can increase luminance back
        // - tab_thr_mov: Maximum luminance adjustment for movie mode [0-255]
        //
        // LV (Light Value) index: 0=very dark, 10=normal indoor, 15=bright outdoor, 20=very bright
        
        // Set tab_ratio to 100 (full effect) for all LV levels
        for (int i = 0; i < AEALG_DYNAMIC_LV_NUM; i++) {
            over_exp.over_exposure.tab_ratio[i] = 100;  // Full effect at all light levels
        }
        
        // Set tab_maxcnt based on HLC level
        // Lower maxcnt = triggers HLC more easily (fewer overexposed blocks needed)
        // Level 0 = high maxcnt (hard to trigger), Level 100 = low maxcnt (easy to trigger)
        // More aggressive: Range 200 to 5 (was 500 to 50)
        int maxcnt_base = 200 - ((params.level * 195) / 100);  // Range: 200 to 5
        if (maxcnt_base < 5) maxcnt_base = 5;
        if (maxcnt_base > 200) maxcnt_base = 200;
        
        for (int i = 0; i < AEALG_DYNAMIC_LV_NUM; i++) {
            over_exp.over_exposure.tab_maxcnt[i] = maxcnt_base;
            over_exp.over_exposure.tab_mincnt[i] = maxcnt_base / 4;  // Hysteresis
        }
        
        // Set maximum luminance adjustment (how much to darken)
        // Higher value = can darken more to suppress highlights
        // More aggressive: Range 100 to 255 (was 50 to 200)
        int thr_mov = 100 + ((params.level * 155) / 100);  // Range: 100 to 255
        if (thr_mov > 255) thr_mov = 255;
        for (int i = 0; i < AEALG_DYNAMIC_LV_NUM; i++) {
            over_exp.over_exposure.tab_thr_mov[i] = thr_mov;
            over_exp.over_exposure.tab_thr_photo[i] = thr_mov;
        }
        
        spdlog::info("HLC config: lum={} speed={} maxcnt={} thr_mov={}", 
                     over_exp.over_exposure.lum, over_exp.over_exposure.speed,
                     maxcnt_base, thr_mov);
    } else {
        over_exp.over_exposure.enable = 0;
        over_exp.over_exposure.lum = 255;  // No highlight suppression
        over_exp.over_exposure.speed = 0;
        
        spdlog::info("HLC disabled: lum=255 (no suppression)");
    }
    
    ret = vendor_isp_set_ae(AET_ITEM_OVER_EXPOSURE, &over_exp);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_ae(OVER_EXPOSURE) for HLC failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
    spdlog::info("HLC vendor_isp_set_ae succeeded");
    
    // Force AE reconvergence after changing HLC parameters
    // This is needed because AE may have already converged and won't re-evaluate
    // without a trigger (like scene change). Reset stable_counter to 0 and 
    // temporarily increase speed to force immediate reconvergence.
    AET_CONVERGENCE convergence;
    memset(&convergence, 0, sizeof(convergence));
    convergence.id = static_cast<AE_ID>(isp_id);
    
    // First get current convergence settings
    ret = vendor_isp_get_ae(AET_ITEM_CONVERGENCE, &convergence);
    if (ret == HD_OK) {
        spdlog::info("AE convergence before: speed={} stable_counter={}", 
                     convergence.convergence.speed, convergence.convergence.stable_counter);
        
        // Reset stable_counter to 0 to force reconvergence
        convergence.convergence.stable_counter = 0;
        // Set speed to maximum (128) for fast reconvergence
        UINT32 original_speed = convergence.convergence.speed;
        convergence.convergence.speed = 128;
        
        ret = vendor_isp_set_ae(AET_ITEM_CONVERGENCE, &convergence);
        if (ret == HD_OK) {
            spdlog::info("Forced AE reconvergence: stable_counter=0, speed=128");
            
            // Restore original speed after a brief moment (the SDK will handle the reconvergence)
            // We restore immediately but the reset of stable_counter already triggered reconvergence
            convergence.convergence.speed = original_speed;
            vendor_isp_set_ae(AET_ITEM_CONVERGENCE, &convergence);
            spdlog::info("Restored AE convergence speed to {}", original_speed);
        } else {
            spdlog::warn("Failed to force AE reconvergence: {}", static_cast<int>(ret));
        }
    } else {
        spdlog::warn("Failed to get AE convergence settings: {}", static_cast<int>(ret));
    }
    
    // Verify by reading back the values
    AET_OVER_EXPOSURE verify_exp;
    memset(&verify_exp, 0, sizeof(verify_exp));
    verify_exp.id = static_cast<AE_ID>(isp_id);
    ret = vendor_isp_get_ae(AET_ITEM_OVER_EXPOSURE, &verify_exp);
    if (ret == HD_OK) {
        spdlog::info("HLC verify readback: enable={} lum={} speed={} ratio[0]={} maxcnt[0]={}", 
                     verify_exp.over_exposure.enable,
                     verify_exp.over_exposure.lum,
                     verify_exp.over_exposure.speed,
                     verify_exp.over_exposure.tab_ratio[0],
                     verify_exp.over_exposure.tab_maxcnt[0]);
    }
    
    // Also get AE status to check current overexposure conditions
    AET_STATUS_INFO ae_status;
    memset(&ae_status, 0, sizeof(ae_status));
    ae_status.id = static_cast<AE_ID>(isp_id);
    ret = vendor_isp_get_ae(AET_ITEM_STATUS, &ae_status);
    if (ret == HD_OK) {
        spdlog::info("AE Status: lum={} expect_lum={} overexp_cnt={} overexp_adj={}", 
                     ae_status.status_info.lum,
                     ae_status.status_info.expect_lum,
                     ae_status.status_info.overexp_cnt,
                     ae_status.status_info.overexp_adj);
    }
#else
    spdlog::warn("HLC: HDAL_ISP_ENABLED=0, vendor APIs not available (stub mode)");
#endif

    spdlog::info("HLC applied successfully: enable={} level={}", params.enable, params.level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetNightMode(IspId isp_id, bool enable) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

    IQT_NIGHT_MODE night_param;
    night_param.id = static_cast<IQ_ID>(isp_id);
    night_param.mode = enable ? IQ_UI_NIGHT_MODE_ON : IQ_UI_NIGHT_MODE_OFF;

    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_NIGHT_MODE, &night_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(NIGHT_MODE) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }

    spdlog::debug("Set night mode: {}", enable);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetGammaLevel(IspId isp_id, int level) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_GAMMA_LV gamma_param;
    memset(&gamma_param, 0, sizeof(gamma_param));
    gamma_param.id = static_cast<IQ_ID>(isp_id);
    
    // Gamma level: 0-100 maps to vendor gamma range
    // Typical gamma curve: 0=linear, 50=gamma 2.2, 100=high contrast
    gamma_param.lv = static_cast<IQ_UI_GAMMA_LV>(level);
    
    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_GAMMA_LV, &gamma_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(GAMMA_LV) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetGammaLevel: isp_id={} level={}", static_cast<int>(isp_id), level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetToneLevel(IspId isp_id, int level) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_TONE_LV tone_param;
    memset(&tone_param, 0, sizeof(tone_param));
    tone_param.id = static_cast<IQ_ID>(isp_id);
    tone_param.lv = static_cast<IQ_UI_TONE_LV>(level);
    
    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_TONE_LV, &tone_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(TONE_LV) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetToneLevel: isp_id={} level={}", static_cast<int>(isp_id), level);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetDpcParams(IspId isp_id, const DpcParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_DPC_PARAM dpc_param;
    memset(&dpc_param, 0, sizeof(dpc_param));
    dpc_param.id = static_cast<IQ_ID>(isp_id);
    
    // Get current parameters first
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, &dpc_param);
    if (ret == HD_OK) {
        dpc_param.dpc.enable = params.enable ? TRUE : FALSE;
        // DPC table entries are set during IQ tuning, we just enable/disable here
        
        ret = vendor_isp_set_iq(IQT_ITEM_DPC_PARAM, &dpc_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(DPC_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(DPC_PARAM) failed: {}", static_cast<int>(ret));
    }
#endif

    spdlog::debug("SetDpcParams: enable={} strength={}", params.enable, params.strength);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetShadingParams(IspId isp_id, const ShadingParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_SHADING_PARAM shading_param;
    memset(&shading_param, 0, sizeof(shading_param));
    shading_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_SHADING_PARAM, &shading_param);
    if (ret == HD_OK) {
        shading_param.shading.ecs_enable = params.ecs_enable ? TRUE : FALSE;
        shading_param.shading.vig_enable = params.vig_enable ? TRUE : FALSE;
        // mode is an enum type - cast appropriately
        shading_param.shading.mode = static_cast<decltype(shading_param.shading.mode)>(params.mode);
        
        ret = vendor_isp_set_iq(IQT_ITEM_SHADING_PARAM, &shading_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(SHADING_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(SHADING_PARAM) failed: {}", static_cast<int>(ret));
    }
#endif

    spdlog::debug("SetShadingParams: ecs={} vig={} mode={}", params.ecs_enable, params.vig_enable, params.mode);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetWdrEnhParams(IspId isp_id, const WdrEnhParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_WDR_ENH_PARAM wdr_enh_param;
    memset(&wdr_enh_param, 0, sizeof(wdr_enh_param));
    wdr_enh_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_WDR_ENH_PARAM, &wdr_enh_param);
    if (ret == HD_OK) {
        wdr_enh_param.wdr_enh.enable = params.enable ? TRUE : FALSE;
        // Copy ratio curve
        for (int i = 0; i < 17; i++) {
            wdr_enh_param.wdr_enh.enh_ratio[i] = params.ratio[i];
        }
        
        ret = vendor_isp_set_iq(IQT_ITEM_WDR_ENH_PARAM, &wdr_enh_param);
        if (ret != HD_OK) {
            spdlog::error("vendor_isp_set_iq(WDR_ENH_PARAM) failed: {}", static_cast<int>(ret));
            return HdalResult::kError;
        }
    } else {
        spdlog::warn("vendor_isp_get_iq(WDR_ENH_PARAM) failed: {}", static_cast<int>(ret));
    }
#endif

    spdlog::debug("SetWdrEnhParams: enable={}", params.enable);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetEnhanceRatioParams(IspId isp_id, const EnhanceRatioParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;

    // Set all enhancement ratios individually
    HdalResult result = HdalResult::kOk;
    
    if (SetDarkEnhanceRatio(isp_id, params.dark_enhance) != HdalResult::kOk) {
        result = HdalResult::kError;
    }
    if (SetContrastEnhanceRatio(isp_id, params.contrast_enhance) != HdalResult::kOk) {
        result = HdalResult::kError;
    }
    if (SetGreenEnhanceRatio(isp_id, params.green_enhance) != HdalResult::kOk) {
        result = HdalResult::kError;
    }
    if (SetSkinEnhanceRatio(isp_id, params.skin_enhance) != HdalResult::kOk) {
        result = HdalResult::kError;
    }

    spdlog::debug("SetEnhanceRatioParams: dark={} contrast={} green={} skin={}", 
                  params.dark_enhance, params.contrast_enhance, 
                  params.green_enhance, params.skin_enhance);
    return result;
}

HdalResult HdalWrapper::SetDarkEnhanceRatio(IspId isp_id, int ratio) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_DARK_ENH_RATIO dark_param;
    memset(&dark_param, 0, sizeof(dark_param));
    dark_param.id = static_cast<IQ_ID>(isp_id);
    dark_param.ratio = static_cast<IQ_NNSC_DARK_ENH_RATIO>(ratio);
    
    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_NNSC_DARK_ENH_RATIO, &dark_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(NNSC_DARK_ENH_RATIO) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetDarkEnhanceRatio: isp={} ratio={}", static_cast<int>(isp_id), ratio);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetContrastEnhanceRatio(IspId isp_id, int ratio) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_CONTRAST_ENH_RATIO contrast_param;
    memset(&contrast_param, 0, sizeof(contrast_param));
    contrast_param.id = static_cast<IQ_ID>(isp_id);
    contrast_param.ratio = static_cast<IQ_NNSC_CONTRAST_ENH_RATIO>(ratio);
    
    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_NNSC_CONTRAST_ENH_RATIO, &contrast_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(NNSC_CONTRAST_ENH_RATIO) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetContrastEnhanceRatio: isp={} ratio={}", static_cast<int>(isp_id), ratio);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetGreenEnhanceRatio(IspId isp_id, int ratio) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_GREEN_ENH_RATIO green_param;
    memset(&green_param, 0, sizeof(green_param));
    green_param.id = static_cast<IQ_ID>(isp_id);
    green_param.ratio = static_cast<IQ_NNSC_GREEN_ENH_RATIO>(ratio);
    
    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_NNSC_GREEN_ENH_RATIO, &green_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(NNSC_GREEN_ENH_RATIO) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetGreenEnhanceRatio: isp={} ratio={}", static_cast<int>(isp_id), ratio);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetSkinEnhanceRatio(IspId isp_id, int ratio) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_SKIN_ENH_RATIO skin_param;
    memset(&skin_param, 0, sizeof(skin_param));
    skin_param.id = static_cast<IQ_ID>(isp_id);
    skin_param.ratio = static_cast<IQ_NNSC_SKIN_ENH_RATIO>(ratio);
    
    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_NNSC_SKIN_ENH_RATIO, &skin_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(NNSC_SKIN_ENH_RATIO) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetSkinEnhanceRatio: isp={} ratio={}", static_cast<int>(isp_id), ratio);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetImageEffect(IspId isp_id, HdalImageEffect effect) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_IMAGEEFFECT img_param;
    memset(&img_param, 0, sizeof(img_param));
    img_param.id = static_cast<IQ_ID>(isp_id);
    img_param.effect = static_cast<IQ_UI_IMAGEEFFECT>(effect);
    
    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_IMAGEEFFECT, &img_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(IMAGEEFFECT) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetImageEffect: isp={} effect={}", static_cast<int>(isp_id), static_cast<int>(effect));
    return HdalResult::kOk;
}

HdalResult HdalWrapper::GetImageEffect(IspId isp_id, HdalImageEffect& effect) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_IMAGEEFFECT img_param;
    memset(&img_param, 0, sizeof(img_param));
    img_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_IMAGEEFFECT, &img_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_get_iq(IMAGEEFFECT) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
    effect = static_cast<HdalImageEffect>(img_param.effect);
#else
    effect = HdalImageEffect::kOff;
#endif

    spdlog::debug("GetImageEffect: isp={} effect={}", static_cast<int>(isp_id), static_cast<int>(effect));
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetRotationMode(IspId isp_id, HdalRotationMode mode) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    VPET_FLIP_ROT_PARAM rot_param;
    memset(&rot_param, 0, sizeof(rot_param));
    rot_param.id = static_cast<VPE_ID>(isp_id);
    
    // Map HdalRotationMode directly to VPE_ISP_FLIP_ROT_MODE
    rot_param.flip_rot.flip_rot_mode = static_cast<VPE_ISP_FLIP_ROT_MODE>(mode);
    
    HD_RESULT ret = vendor_vpe_set_cmd(VPET_ITEM_FLIP_ROT_PARAM, &rot_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_vpe_set_cmd(FLIP_ROT_PARAM) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetRotationMode: isp={} mode={}", static_cast<int>(isp_id), static_cast<int>(mode));
    return HdalResult::kOk;
}

HdalResult HdalWrapper::GetRotationMode(IspId isp_id, HdalRotationMode& mode) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    VPET_FLIP_ROT_PARAM rot_param;
    memset(&rot_param, 0, sizeof(rot_param));
    rot_param.id = static_cast<VPE_ID>(isp_id);
    
    HD_RESULT ret = vendor_vpe_get_cmd(VPET_ITEM_FLIP_ROT_PARAM, &rot_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_vpe_get_cmd(FLIP_ROT_PARAM) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
    
    // Get rotation mode from flip_rot structure
    mode = static_cast<HdalRotationMode>(rot_param.flip_rot.flip_rot_mode);
#else
    mode = HdalRotationMode::kRotate0;
#endif

    spdlog::debug("GetRotationMode: isp={} mode={}", static_cast<int>(isp_id), static_cast<int>(mode));
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetAWBScene(IspId isp_id, HdalAWBScene scene) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    AWBT_SCENE_MODE awb_param;
    memset(&awb_param, 0, sizeof(awb_param));
    awb_param.id = static_cast<AWB_ID>(isp_id);
    awb_param.mode = static_cast<AWB_SCENE>(scene);
    
    HD_RESULT ret = vendor_isp_set_awb(AWBT_ITEM_SCENE, &awb_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_awb(SCENE) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetAWBScene: isp={} scene={}", static_cast<int>(isp_id), static_cast<int>(scene));
    return HdalResult::kOk;
}

HdalResult HdalWrapper::GetAWBScene(IspId isp_id, HdalAWBScene& scene) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    AWBT_SCENE_MODE awb_param;
    memset(&awb_param, 0, sizeof(awb_param));
    awb_param.id = static_cast<AWB_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_awb(AWBT_ITEM_SCENE, &awb_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_get_awb(SCENE) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
    scene = static_cast<HdalAWBScene>(awb_param.mode);
#else
    scene = HdalAWBScene::kAuto;
#endif

    spdlog::debug("GetAWBScene: isp={} scene={}", static_cast<int>(isp_id), static_cast<int>(scene));
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetManualAE(IspId isp_id, const ManualAEParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    AET_MANUAL ae_param;
    memset(&ae_param, 0, sizeof(ae_param));
    ae_param.id = static_cast<AE_ID>(isp_id);
    // Set manual mode: AUTO_MODE for auto, MANUAL_MODE for manual
    ae_param.manual.mode = params.manual_mode ? MANUAL_MODE : AUTO_MODE;
    ae_param.manual.expotime = params.exposure_time_us;
    ae_param.manual.iso_gain = params.iso_gain;
    
    HD_RESULT ret = vendor_isp_set_ae(AET_ITEM_MANUAL, &ae_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_ae(MANUAL) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetManualAE: isp={} manual={} exp={}us gain={}", 
                  static_cast<int>(isp_id), params.manual_mode, 
                  params.exposure_time_us, params.iso_gain);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::GetManualAE(IspId isp_id, ManualAEParams& params) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    AET_MANUAL ae_param;
    memset(&ae_param, 0, sizeof(ae_param));
    ae_param.id = static_cast<AE_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_ae(AET_ITEM_MANUAL, &ae_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_get_ae(MANUAL) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
    params.manual_mode = (ae_param.manual.mode == MANUAL_MODE);
    params.exposure_time_us = ae_param.manual.expotime;
    params.iso_gain = ae_param.manual.iso_gain;
#else
    params.manual_mode = false;
    params.exposure_time_us = 33333;
    params.iso_gain = 100;
#endif

    spdlog::debug("GetManualAE: isp={} manual={} exp={}us gain={}", 
                  static_cast<int>(isp_id), params.manual_mode, 
                  params.exposure_time_us, params.iso_gain);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::SetBWMode(IspId isp_id, bool enable) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    // B&W mode is implemented using ImageEffect with BlackWhite
    IQT_IMAGEEFFECT img_param;
    memset(&img_param, 0, sizeof(img_param));
    img_param.id = static_cast<IQ_ID>(isp_id);
    img_param.effect = enable ? IQ_UI_IMAGEEFFECT_BW : IQ_UI_IMAGEEFFECT_OFF;
    
    HD_RESULT ret = vendor_isp_set_iq(IQT_ITEM_IMAGEEFFECT, &img_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_set_iq(IMAGEEFFECT for BW) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
#endif

    spdlog::debug("SetBWMode: isp={} enable={}", static_cast<int>(isp_id), enable);
    return HdalResult::kOk;
}

HdalResult HdalWrapper::GetBWMode(IspId isp_id, bool& enable) {
    if (!initialized_) return HdalResult::kNotInitialized;

    std::lock_guard<std::mutex> lock(g_hdal_mutex);

#if HDAL_ISP_ENABLED
    IQT_IMAGEEFFECT img_param;
    memset(&img_param, 0, sizeof(img_param));
    img_param.id = static_cast<IQ_ID>(isp_id);
    
    HD_RESULT ret = vendor_isp_get_iq(IQT_ITEM_IMAGEEFFECT, &img_param);
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_get_iq(IMAGEEFFECT for BW) failed: {}", static_cast<int>(ret));
        return HdalResult::kError;
    }
    enable = (img_param.effect == IQ_UI_IMAGEEFFECT_BW);
#else
    enable = false;
#endif

    spdlog::debug("GetBWMode: isp={} enable={}", static_cast<int>(isp_id), enable);
    return HdalResult::kOk;
}

} // namespace platform
} // namespace ipcam
