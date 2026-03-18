// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "image_api.h"
#include "common.h"

#include "ipcweb_client/client.h"

bool bad_request = false;
bool DEFAULT = false;
std::set<std::string> expected_parameters = {"iBrightness", "iContrast", "iSaturation", "iSharpness", "iHue", 
  "iExposureGain", "sExposureMode", "sExposureTime", "sGainMode", 
  "iFPS", "sNightToDay", "sFillLightMode", "iLightBrightness", "sNightToDayMode", "sScheduledStartTime", "sScheduledEndTime", "sLightBrightnessMode",
  "iNightToDayFilterLevel", "iNightToDayFilterTime", "sBLCRegion", "sHLC", "iHDRLevel", 
  "iBLCStrength", "iHLCLevel", "iDarkBoostLevel", "sHDR", 
  "White Balance Style", "iWhiteBalanceRed", "iWhiteBalanceGreen", 
  "iWhiteBalanceBlue", "sNoiseReduceMode", "sDehaze", "sGrayScaleMode", 
  "sDistortionCorrection", "iSpatialDenoiseLevel", "iTemporalDenoiseLevel", 
  "iDehazeLevel", "iFecLevel", "iLdchLevel", "sPowerLineFrequencyMode", 
  "sImageFlip", "iImageRotation", "sAFMode", "sScenario", "iDefault"};
namespace rockchip {
namespace cgi {

nlohmann::json image_specific_resource_get(std::string string, int cam_id) {
  nlohmann::json specific_resource =
      nlohmann::json::object(); /* one of all the same resources */
  minilog_debug("%s: string is %s\n", __func__, string.c_str());
  int value_int;
  char *tmp = new char[20];

  if (!string.compare(PATH_IMAGE_ADJUSTMENT)) {
    hon_isp_get_brightness(cam_id, &value_int);
    specific_resource.emplace("iBrightness", value_int);
    hon_isp_get_contrast(cam_id, &value_int);
    specific_resource.emplace("iContrast", value_int);
    hon_isp_get_saturation(cam_id, &value_int);
    specific_resource.emplace("iSaturation", value_int);
    hon_isp_get_sharpness(cam_id, &value_int);
    specific_resource.emplace("iSharpness", value_int);
    hon_isp_get_hue(cam_id, &value_int);
    specific_resource.emplace("iHue", value_int);
  } else if (!string.compare(PATH_IMAGE_EXPOSURE)) {
    hon_isp_get_exposure_mode(cam_id, &tmp);
    specific_resource.emplace("sExposureMode", tmp);
    hon_isp_get_gain_mode(cam_id, &tmp);
    specific_resource.emplace("sGainMode", tmp);
    hon_isp_get_exposure_time(cam_id, &tmp);
    specific_resource.emplace("sExposureTime", tmp);
    hon_isp_get_exposure_gain(cam_id, &value_int);
    specific_resource.emplace("iExposureGain", value_int);
    hon_isp_get_frame_rate(cam_id, &value_int);
    specific_resource.emplace("iFPS", value_int);
  } else if (!string.compare(PATH_IMAGE_NIGHT_TO_DAY)) {
    // hon_isp_get_night_to_day(cam_id, &tmp);
    // specific_resource.emplace("sNightToDay", tmp);
    // hon_isp_get_fill_light_mode(cam_id, &tmp);
    // specific_resource.emplace("sFillLightMode", tmp);
    hon_isp_get_light_brightness(cam_id, &value_int);
    specific_resource.emplace("iLightBrightness", value_int);
    hon_isp_get_IR_brightness_mode(cam_id, &tmp);
    specific_resource.emplace("sLightBrightnessMode", tmp);
    // hon_isp_get_night_to_day_filter_level(cam_id, &value_int);
    // specific_resource.emplace("iNightToDayFilterLevel", value_int);
    // hon_isp_get_night_to_day_filter_time(cam_id, &value_int);
    // specific_resource.emplace("iNightToDayFilterTime", value_int);
    hon_isp_get_IR_mode(cam_id, &tmp);
    specific_resource.emplace("sNightToDayMode", tmp);
    hon_isp_get_IR_start_time(cam_id, &tmp);
    specific_resource.emplace("sScheduledStartTime", tmp);
    hon_isp_get_IR_end_time(cam_id, &tmp);
    specific_resource.emplace("sScheduledEndTime", tmp);


  } else if (!string.compare(PATH_IMAGE_BLC)) {
    hon_isp_get_hdr(cam_id, &tmp);
    specific_resource.emplace("sHDR", tmp);
    hon_isp_get_blc_region(cam_id, &tmp);
    specific_resource.emplace("sBLCRegion", tmp);
    hon_isp_get_hlc(cam_id, &tmp);
    specific_resource.emplace("sHLC", tmp);
    hon_isp_get_hdr_level(cam_id, &value_int);
    specific_resource.emplace("iHDRLevel", value_int);
    hon_isp_get_blc_strength(cam_id, &value_int);
    specific_resource.emplace("iBLCStrength", value_int);
    hon_isp_get_hlc_level(cam_id, &value_int);
    specific_resource.emplace("iHLCLevel", value_int);
    hon_isp_get_dark_boost_level(cam_id, &value_int);
    specific_resource.emplace("iDarkBoostLevel", value_int);
  } else if (!string.compare(PATH_IMAGE_WHITE_BLANCE)) {
    hon_isp_get_white_blance_style(cam_id, &tmp);
    specific_resource.emplace("White Balance Style", tmp);
    if (!strcmp(tmp, "Manual WhiteBalance")) {
      hon_isp_get_white_blance_red(cam_id, &value_int);
      specific_resource.emplace("iWhiteBalanceRed", value_int);
      hon_isp_get_white_blance_green(cam_id, &value_int);
      specific_resource.emplace("iWhiteBalanceGreen", value_int);
      hon_isp_get_white_blance_blue(cam_id, &value_int);
      specific_resource.emplace("iWhiteBalanceBlue", value_int);
    }
  } else if (!string.compare(PATH_IMAGE_ENHANCEMENT)) {
    hon_isp_get_noise_reduce_mode(cam_id, &tmp);
    specific_resource.emplace("sNoiseReduceMode", tmp);
    hon_isp_get_dehaze(cam_id, &tmp);
    specific_resource.emplace("sDehaze", tmp);
    hon_isp_get_gray_scale_mode(cam_id, &tmp);
    specific_resource.emplace("sGrayScaleMode", tmp);
    hon_isp_get_distortion_correction(cam_id, &tmp);
    specific_resource.emplace("sDistortionCorrection", tmp);
    hon_isp_get_spatial_denoise_level(cam_id, &value_int);
    specific_resource.emplace("iSpatialDenoiseLevel", value_int);
    hon_isp_get_temporal_denoise_level(cam_id, &value_int);
    specific_resource.emplace("iTemporalDenoiseLevel", value_int);
    hon_isp_get_dehaze_level(cam_id, &value_int);
    specific_resource.emplace("iDehazeLevel", value_int);
    hon_isp_get_fec_level(cam_id, &value_int);
    specific_resource.emplace("iFecLevel", value_int);
    hon_isp_get_ldch_level(cam_id, &value_int);
    specific_resource.emplace("iLdchLevel", value_int);
  } else if (!string.compare(PATH_IMAGE_VIDEO_ADJUSTMEN)) {
    hon_isp_get_power_line_frequency_mode(cam_id, &tmp);
    specific_resource.emplace("sPowerLineFrequencyMode", tmp);
    hon_isp_get_image_flip(cam_id, &tmp);
    specific_resource.emplace("sImageFlip", tmp);
    hon_video_get_rotation(&value_int);
    specific_resource.emplace("iImageRotation", value_int);
  } else if (!string.compare(PATH_IMAGE_AF)) {
    hon_isp_get_af_mode(cam_id, &tmp);
    specific_resource.emplace("sAFMode", tmp);
    hon_isp_get_zoom_level(cam_id, &value_int);
    specific_resource.emplace("iZoomLevel", value_int);
    hon_isp_get_focus_level(cam_id, &value_int);
    specific_resource.emplace("iFocusLevel", value_int);
  } else if (!string.compare(PATH_IMAGE_SCENARIO)) {
    hon_isp_get_scenario(cam_id, &tmp);
    specific_resource.emplace("sScenario", tmp);
  }
  delete[] tmp;

  return specific_resource;
}

nlohmann::json image_channel_resource_get(int cam_id) {
  nlohmann::json resource = nlohmann::json::object();

  resource.emplace("id", cam_id);
  resource.emplace(KEY_IMAGE_ADJUSTMENT,
                   image_specific_resource_get(PATH_IMAGE_ADJUSTMENT, cam_id));
  resource.emplace(KEY_IMAGE_EXPOSURE,
                   image_specific_resource_get(PATH_IMAGE_EXPOSURE, cam_id));
  resource.emplace(
      KEY_IMAGE_NIGHT_TO_DAY,
      image_specific_resource_get(PATH_IMAGE_NIGHT_TO_DAY, cam_id));
  resource.emplace(KEY_IMAGE_BLC,
                   image_specific_resource_get(PATH_IMAGE_BLC, cam_id));
  resource.emplace(
      KEY_IMAGE_WHITE_BLANCE,
      image_specific_resource_get(PATH_IMAGE_WHITE_BLANCE, cam_id));
  resource.emplace(KEY_IMAGE_ENHANCEMENT,
                   image_specific_resource_get(PATH_IMAGE_ENHANCEMENT, cam_id));
  resource.emplace(
      KEY_IMAGE_VIDEO_ADJUSTMEN,
      image_specific_resource_get(PATH_IMAGE_VIDEO_ADJUSTMEN, cam_id));

  return resource;
}

// void image_specific_resource_set(std::string string, nlohmann::json data,
//                                  int cam_id) {
//   int value_int;
//   std::string value;
//   minilog_debug("%s: string is %s\n", __func__, string.c_str());
//   minilog_debug("data is %s\n", data.dump().c_str());
//   for (auto it = data.begin(); it != data.end(); ++it) {
//         std::string key = it.key();
//         if (expected_parameters.find(key) == expected_parameters.end()) {
//             bad_request = true;
//             break;
//         }
//   }
//   if(!bad_request){
//     if (!string.compare(PATH_IMAGE_ADJUSTMENT)) {
//         if (data.dump().find("iBrightness") != data.dump().npos) {
//           value_int = atoi(data.at("iBrightness").dump().c_str());
//           hon_isp_set_brightness(cam_id, value_int);
//         }
//         if (data.dump().find("iContrast") != data.dump().npos) {
//           value_int = atoi(data.at("iContrast").dump().c_str());
//           hon_isp_set_contrast(cam_id, value_int);
//         }
//         if (data.dump().find("iSaturation") != data.dump().npos) {
//           value_int = atoi(data.at("iSaturation").dump().c_str());
//           hon_isp_set_saturation(cam_id, value_int);
//         }
//         if (data.dump().find("iSharpness") != data.dump().npos) {
//           value_int = atoi(data.at("iSharpness").dump().c_str());
//           hon_isp_set_sharpness(cam_id, value_int);
//         }
//         if (data.dump().find("iHue") != data.dump().npos) {
//           value_int = atoi(data.at("iHue").dump().c_str());
//           hon_isp_set_hue(cam_id, value_int);
//         }
//     }
//     else if (!string.compare(PATH_IMAGE_EXPOSURE)) {
//       if (data.dump().find("iExposureGain") != data.dump().npos) {
//         value_int = atoi(data.at("iExposureGain").dump().c_str());
//         hon_isp_set_exposure_gain(cam_id, value_int);
//       }
//       if (data.dump().find("sExposureMode") != data.dump().npos) {
//         value = data.at("sExposureMode").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_exposure_mode(cam_id, value.c_str());
//       }
//       if (data.dump().find("sExposureTime") != data.dump().npos) {
//         value = data.at("sExposureTime").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_exposure_time(cam_id, value.c_str());
//       }
//       if (data.dump().find("sGainMode") != data.dump().npos) {
//         value = data.at("sGainMode").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_gain_mode(cam_id, value.c_str());
//       }
//       if (data.dump().find("iFPS") != data.dump().npos) {
//         value_int = atoi(data.at("iFPS").dump().c_str());
//         hon_isp_set_frame_rate(cam_id, value_int);
//       }
//     } else if (!string.compare(PATH_IMAGE_NIGHT_TO_DAY)) {
//       if (data.dump().find("sNightToDay") != data.dump().npos) {
//         value = data.at("sNightToDay").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_night_to_day(cam_id, value.c_str());
//       }
//       if (data.dump().find("sFillLightMode") != data.dump().npos) {
//         value = data.at("sFillLightMode").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_fill_light_mode(cam_id, value.c_str());
//       }
//       if (data.dump().find("iLightBrightness") != data.dump().npos) {
//         value_int = atoi(data.at("iLightBrightness").dump().c_str());
//         hon_isp_set_light_brightness(cam_id, value_int);
//       }
//       if (data.dump().find("iNightToDayFilterLevel") != data.dump().npos) {
//         value_int = atoi(data.at("iNightToDayFilterLevel").dump().c_str());
//         hon_isp_set_night_to_day_filter_level(cam_id, value_int);
//       }
//       if (data.dump().find("iNightToDayFilterTime") != data.dump().npos) {
//         value_int = atoi(data.at("iNightToDayFilterTime").dump().c_str());
//         hon_isp_set_night_to_day_filter_time(cam_id, value_int);
//       }
//     } else if (!string.compare(PATH_IMAGE_BLC)) {
//       if (data.dump().find("sBLCRegion") != data.dump().npos) {
//         value = data.at("sBLCRegion").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_blc_region(cam_id, value.c_str());
//       }
//       if (data.dump().find("sHLC") != data.dump().npos) {
//         value = data.at("sHLC").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_hlc(cam_id, value.c_str());
//       }
//       if (data.dump().find("iHDRLevel") != data.dump().npos) {
//         value_int = atoi(data.at("iHDRLevel").dump().c_str());
//         hon_isp_set_hdr_level(cam_id, value_int);
//       }
//       if (data.dump().find("iBLCStrength") != data.dump().npos) {
//         value_int = atoi(data.at("iBLCStrength").dump().c_str());
//         hon_isp_set_blc_strength(cam_id, value_int);
//       }
//       if (data.dump().find("iHLCLevel") != data.dump().npos) {
//         value_int = atoi(data.at("iHLCLevel").dump().c_str());
//         hon_isp_set_hlc_level(cam_id, value_int);
//       }
//       if (data.dump().find("iDarkBoostLevel") != data.dump().npos) {
//         value_int = atoi(data.at("iDarkBoostLevel").dump().c_str());
//         hon_isp_set_dark_boost_level(cam_id, value_int);
//       }
//       // it will restart app, must be the last call
//       if (data.dump().find("sHDR") != data.dump().npos) {
//         value = data.at("sHDR").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_hdr(cam_id, value.c_str());
//       }
//     } else if (!string.compare(PATH_IMAGE_WHITE_BLANCE)) {
//       if (data.dump().find("White Balance Style") != data.dump().npos) {
//         value = data.at("White Balance Style").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_white_blance_style(cam_id, value.c_str());
//       }
//       if (data.dump().find("iWhiteBalanceRed") != data.dump().npos) {
//         value_int = atoi(data.at("iWhiteBalanceRed").dump().c_str());
//         hon_isp_set_white_blance_red(cam_id, value_int);
//       }
//       if (data.dump().find("iWhiteBalanceGreen") != data.dump().npos) {
//         value_int = atoi(data.at("iWhiteBalanceGreen").dump().c_str());
//         hon_isp_set_white_blance_green(cam_id, value_int);
//       }
//       if (data.dump().find("iWhiteBalanceBlue") != data.dump().npos) {
//         value_int = atoi(data.at("iWhiteBalanceBlue").dump().c_str());
//         hon_isp_set_white_blance_blue(cam_id, value_int);
//       }
//     } else if (!string.compare(PATH_IMAGE_ENHANCEMENT)) {
//       if (data.dump().find("sNoiseReduceMode") != data.dump().npos) {
//         value = data.at("sNoiseReduceMode").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_noise_reduce_mode(cam_id, value.c_str());
//       }
//       if (data.dump().find("sDehaze") != data.dump().npos) {
//         value = data.at("sDehaze").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_dehaze(cam_id, value.c_str());
//       }
//       if (data.dump().find("sGrayScaleMode") != data.dump().npos) {
//         value = data.at("sGrayScaleMode").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_gray_scale_mode(cam_id, value.c_str());
//       }
//       if (data.dump().find("sDistortionCorrection") != data.dump().npos) {
//         value = data.at("sDistortionCorrection").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_distortion_correction(cam_id, value.c_str());
//       }
//       if (data.dump().find("iSpatialDenoiseLevel") != data.dump().npos) {
//         value_int = atoi(data.at("iSpatialDenoiseLevel").dump().c_str());
//         hon_isp_set_spatial_denoise_level(cam_id, value_int);
//       }
//       if (data.dump().find("iTemporalDenoiseLevel") != data.dump().npos) {
//         value_int = atoi(data.at("iTemporalDenoiseLevel").dump().c_str());
//         hon_isp_set_temporal_denoise_level(cam_id, value_int);
//       }
//       if (data.dump().find("iDehazeLevel") != data.dump().npos) {
//         value_int = atoi(data.at("iDehazeLevel").dump().c_str());
//         hon_isp_set_dehaze_level(cam_id, value_int);
//       }
//       if (data.dump().find("iFecLevel") != data.dump().npos) {
//         value_int = atoi(data.at("iFecLevel").dump().c_str());
//         hon_isp_set_fec_level(cam_id, value_int);
//       }
//       if (data.dump().find("iLdchLevel") != data.dump().npos) {
//         value_int = atoi(data.at("iLdchLevel").dump().c_str());
//         hon_isp_set_ldch_level(cam_id, value_int);
//       }
//     } else if (!string.compare(PATH_IMAGE_VIDEO_ADJUSTMEN)) {
//       if (data.dump().find("sPowerLineFrequencyMode") != data.dump().npos) {
//         value = data.at("sPowerLineFrequencyMode").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_power_line_frequency_mode(cam_id, value.c_str());
//       }
//       if (data.dump().find("sImageFlip") != data.dump().npos) {
//         value = data.at("sImageFlip").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_image_flip(cam_id, value.c_str());
//       }
//       if (data.dump().find("iImageRotation") != data.dump().npos) {
//         value_int = atoi(data.at("iImageRotation").dump().c_str());
//         hon_video_set_rotation(value_int);
//       }
//     } else if (!string.compare(PATH_IMAGE_AF)) {
//       if (data.dump().find("sAFMode") != data.dump().npos) {
//         value = data.at("sAFMode").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_af_mode(cam_id, value.c_str());
//       }
//     } else if (!string.compare(PATH_IMAGE_SCENARIO)) {
//       if (data.dump().find("sScenario") != data.dump().npos) {
//         value = data.at("sScenario").dump();
//         value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
//         hon_isp_set_scenario(cam_id, value.c_str());
//       }
//     }
//   }
// }

void set_int_parameter(const nlohmann::json& data, const std::string& key, int cam_id, int (*set_func)(int, int)) {
    auto it = data.find(key);
    if (it != data.end()) {
        int value = it->get<int>();
        set_func(cam_id, value);
    }
}

void set_string_parameter(const nlohmann::json& data, const std::string& key, int cam_id, int (*set_func)(int, const char*)) {
    auto it = data.find(key);
    if (it != data.end()) {
        std::string value = it->get<std::string>();
        set_func(cam_id, value.c_str());
    }
}


void image_specific_resource_set(std::string string, nlohmann::json data, int cam_id) {
    minilog_debug("%s: string is %s\n", __func__, string.c_str());
    minilog_debug("data is %s\n", data.dump().c_str());
    int value_int;

    for (auto it = data.begin(); it != data.end(); ++it) {
        std::string key = it.key();
        if (expected_parameters.find(key) == expected_parameters.end()) {
            bad_request = true;
            break;
        }
    }

    

    if (!bad_request) {
        hon_isp_stop_IR_monitoring_thread();
        if (string == PATH_IMAGE_ADJUSTMENT) {
            set_int_parameter(data, "iBrightness", cam_id, hon_isp_set_brightness);
            set_int_parameter(data, "iContrast", cam_id, hon_isp_set_contrast);
            set_int_parameter(data, "iSaturation", cam_id, hon_isp_set_saturation);
            set_int_parameter(data, "iSharpness", cam_id, hon_isp_set_sharpness);
            set_int_parameter(data, "iHue", cam_id, hon_isp_set_hue);
        } else if (string == PATH_IMAGE_EXPOSURE) {
            set_int_parameter(data, "iExposureGain", cam_id, hon_isp_set_exposure_gain);
            set_string_parameter(data, "sExposureMode", cam_id, hon_isp_set_exposure_mode);
            set_string_parameter(data, "sExposureTime", cam_id, hon_isp_set_exposure_time);
            set_string_parameter(data, "sGainMode", cam_id, hon_isp_set_gain_mode);
            set_int_parameter(data, "iFPS", cam_id, hon_isp_set_frame_rate);
        } else if (string == PATH_IMAGE_NIGHT_TO_DAY) {
            //set_string_parameter(data, "sNightToDay", cam_id, hon_isp_set_night_to_day);
            //set_string_parameter(data, "sFillLightMode", cam_id, hon_isp_set_fill_light_mode);
            set_int_parameter(data, "iLightBrightness", cam_id, hon_isp_set_light_brightness);
            //set_int_parameter(data, "iNightToDayFilterLevel", cam_id, hon_isp_set_night_to_day_filter_level);
            //set_int_parameter(data, "iNightToDayFilterTime", cam_id, hon_isp_set_night_to_day_filter_time);
            set_string_parameter(data, "sLightBrightnessMode", cam_id, hon_isp_set_IR_brightness_mode);
            set_string_parameter(data, "sNightToDayMode", cam_id, hon_isp_set_IR_mode);
            set_string_parameter(data, "sScheduledStartTime", cam_id, hon_isp_set_IR_start_time);
            set_string_parameter(data, "sScheduledEndTime", cam_id, hon_isp_set_IR_end_time);
        } else if (string == PATH_IMAGE_BLC) {
            set_string_parameter(data, "sBLCRegion", cam_id, hon_isp_set_blc_region);
            set_string_parameter(data, "sHLC", cam_id, hon_isp_set_hlc);
            set_int_parameter(data, "iHDRLevel", cam_id, hon_isp_set_hdr_level);
            set_int_parameter(data, "iBLCStrength", cam_id, hon_isp_set_blc_strength);
            set_int_parameter(data, "iHLCLevel", cam_id, hon_isp_set_hlc_level);
            set_int_parameter(data, "iDarkBoostLevel", cam_id, hon_isp_set_dark_boost_level);
            set_string_parameter(data, "sHDR", cam_id, hon_isp_set_hdr);
        } else if (string == PATH_IMAGE_WHITE_BLANCE) {
            set_string_parameter(data, "White Balance Style", cam_id, hon_isp_set_white_blance_style);
            set_int_parameter(data, "iWhiteBalanceRed", cam_id, hon_isp_set_white_blance_red);
            set_int_parameter(data, "iWhiteBalanceGreen", cam_id, hon_isp_set_white_blance_green);
            set_int_parameter(data, "iWhiteBalanceBlue", cam_id, hon_isp_set_white_blance_blue);
        } else if (string == PATH_IMAGE_ENHANCEMENT) {
            set_string_parameter(data, "sNoiseReduceMode", cam_id, hon_isp_set_noise_reduce_mode);
            set_string_parameter(data, "sDehaze", cam_id, hon_isp_set_dehaze);
            set_string_parameter(data, "sGrayScaleMode", cam_id, hon_isp_set_gray_scale_mode);
            set_string_parameter(data, "sDistortionCorrection", cam_id, hon_isp_set_distortion_correction);
            set_int_parameter(data, "iSpatialDenoiseLevel", cam_id, hon_isp_set_spatial_denoise_level);
            set_int_parameter(data, "iTemporalDenoiseLevel", cam_id, hon_isp_set_temporal_denoise_level);
            set_int_parameter(data, "iDehazeLevel", cam_id, hon_isp_set_dehaze_level);
            set_int_parameter(data, "iFecLevel", cam_id, hon_isp_set_fec_level);
            set_int_parameter(data, "iLdchLevel", cam_id, hon_isp_set_ldch_level);
        } else if (string == PATH_IMAGE_VIDEO_ADJUSTMEN) {
            set_string_parameter(data, "sPowerLineFrequencyMode", cam_id, hon_isp_set_power_line_frequency_mode);
            set_string_parameter(data, "sImageFlip", cam_id, hon_isp_set_image_flip);
            //set_int_parameter(data, "iImageRotation", cam_id, hon_video_set_rotation);
            if (data.dump().find("iImageRotation") != data.dump().npos) {
              value_int = atoi(data.at("iImageRotation").dump().c_str());
              hon_video_set_rotation(value_int);
            }
        } else if (string == PATH_IMAGE_AF) {
            set_string_parameter(data, "sAFMode", cam_id, hon_isp_set_af_mode);
        } else if (string == PATH_IMAGE_SCENARIO) {
            set_string_parameter(data, "sScenario", cam_id, hon_isp_set_scenario);
        } else if (string == PATH_IMAGE_DEFAULT) {
            if (data.dump().find("iDefault") != data.dump().npos) {
                int value = data.at("iDefault").get<int>();
                if (value == 1) {
                    hon_isp_set_default(cam_id);
                    DEFAULT = true;
                }
            }
        }
        hon_isp_start_IR_monitoring_thread();

    }
}


void ImageApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  int cam_id = 0;
  std::string path_api_resource = "";
  std::string path_channel_resource = "";
  std::string path_specific_resource = "";
  nlohmann::json content = nlohmann::json({}) ;
  nlohmann::json default_content;


  /* Get Path Information */
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_channel_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
    pos_first = path_channel_resource.find_first_of("/");
    if (pos_first != -1)
      path_specific_resource = path_channel_resource.substr(
          pos_first + 1, path_channel_resource.size());
  }
  if (!path_channel_resource.empty())
    cam_id = atoi(
        path_channel_resource.substr(0, path_channel_resource.size()).c_str());

  if (Req.Method == "GET") {
    /* Get based on path information */
    if (path_channel_resource.empty()) { /* path info is /image */
      content = nlohmann::json::array();
      // content.push_back(image_channel_resource_get(1, id));
    } else {
      if (path_specific_resource.empty()) { /* path info is /image/0 */
        content = image_channel_resource_get(cam_id);
      } else { /* path example is /image/0/blc */
        content = image_specific_resource_get(path_specific_resource, cam_id);
      }
    }

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk, content, "OK");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    nlohmann::json diff;
    nlohmann::json cfg_old;
    nlohmann::json image_config = Req.PostObject; /* must be json::object */

    if (path_specific_resource
            .empty()) { /* path info is /image/0 or /image/1 */

      //  cfg_old = image_channel_resource_get(channel_id, id);
      //   /* Erase unexist data */
      //   for (auto &x : nlohmann::json::iterator_wrapper(image_config)) {
      //     if (cfg_old.dump().find(x.key()) == cfg_old.dump().npos)
      //         image_config.erase(x.key());
      //   }
      //   /* Erase unchanged data */
      //   diff = nlohmann::json::diff(cfg_old, image_config);
      //   for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
      //     if (diff.dump().find(x.key()) == diff.dump().npos)
      //       image_config.erase(x.key());
      //   }
      //   /* Set */
      //   if (!image_config.empty())
      //     image_channel_resource_set(image_config, id);
      //   /* Get new info */
      //   content = image_channel_resource_get(channel_id, id);
    } else { /* path example is /image/0/blc */
      if (!path_specific_resource.compare(PATH_IMAGE_AF_CMD)) {
        if (image_config.dump().find("cmd") != std::string::npos) {
          std::string af_cmd = image_config.at("cmd");
          int ret = 0;
          if (af_cmd == "ZoomIn")
            ret = hon_isp_af_zoom_in(0);
          else if (af_cmd == "ZoomOut")
            ret = hon_isp_af_zoom_out(0);
          else if (af_cmd == "FocusIn")
            ret = hon_isp_af_focus_in(0);
          else if (af_cmd == "FocusOut")
            ret = hon_isp_af_focus_out(0);
          else if (af_cmd == "FocusOnce")
            ret = hon_isp_af_focus_once(0);
          else
            minilog_debug("unsupport, af_cmd is %s\n", af_cmd.c_str());
          // TODO: overRange, onlyManualSupport, NotAutoSupport
          if (ret)
            content.emplace("rst", "fail");
          else
            content.emplace("rst", "success");
        } else {
          content.emplace("rst", "illegalCmd");
        }
      } else {
        nlohmann::json cfg_old = image_specific_resource_get(path_specific_resource, cam_id);
        nlohmann::json diff = nlohmann::json::diff(cfg_old, image_config);
        for (auto& x : nlohmann::json::iterator_wrapper(diff)) {
            if (x.value().is_null()) {
              image_config.erase(x.key());
            }
        }
        if (!image_config.empty()) {
            image_specific_resource_set(path_specific_resource, image_config, cam_id);
            //restart time momioring thread
            //hon_isp_restart_IR_monitoring_thread();


        }
        content = image_specific_resource_get(path_specific_resource, cam_id);
      }
    }
    if(bad_request){
      //Resp.setErrorResponse(HttpStatus::kBadRequest, "Bad Request");
      Resp.setHeader(HttpStatus::kBadRequest, "Bad Request");
      Resp.setApiData(HttpStatus::kBadRequest,content, "Bad Request");
      bad_request = false;
    }
    else{
      Resp.setHeader(HttpStatus::kOk, "OK");
      if(DEFAULT){
        default_content.emplace("setdefault", "success");
        Resp.setApiData(HttpStatus::kOk, default_content, "OK");
        DEFAULT = false;
      }
      else{
         Resp.setApiData(HttpStatus::kOk, content, "OK");
      }       
     
    }
  } else {
      Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
      Resp.setApiData(HttpStatus::kNotImplemented,content, "Not Implemented");
  }
}

} // namespace cgi
} // namespace rockchip
