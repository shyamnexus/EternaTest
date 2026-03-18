// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "osd_api.h"
#include "common.h"
#include <fstream>

namespace rockchip {
namespace cgi {

#define OSD_CHANNAL_ID 0
#define OSD_DATE_TIME_ID 2
#define OSD_CHARACTER_ID 1
#define OSD_IMAGE_ID 6

nlohmann::json osd_overlays_get() {
  int value;
  char *str;
  char *tmp = new char[64];
  nlohmann::json osd_overlays;
  nlohmann::json normalized_screen_size;

  hon_osd_get_normalized_screen_width(&value);
  normalized_screen_size.emplace("iNormalizedScreenWidth", value);
  hon_osd_get_normalized_screen_height(&value);
  normalized_screen_size.emplace("iNormalizedScreenHeight", value);

  // attribute, all OSD tables have the same attributes
  nlohmann::json attribute;
  hon_osd_get_attribute(&tmp);
  attribute.emplace("sOSDAttribute", tmp);
  hon_osd_get_font_size(&value);
  sprintf(tmp, "%d*%d", value, value);
  attribute.emplace("sOSDFontSize", tmp);
  hon_osd_get_font_color_mode(&tmp);
  attribute.emplace("sOSDFrontColorMode", tmp);
  hon_osd_get_font_color(&tmp);
  attribute.emplace("sOSDFrontColor", tmp);
  hon_osd_get_alignment(&tmp);
  attribute.emplace("sAlignment", tmp);
  hon_osd_get_boundary(&value);
  attribute.emplace("iBoundary", value);

  // channel_name
  nlohmann::json channel_name;
  hon_osd_get_position_x(OSD_CHANNAL_ID, &value);
  channel_name.emplace("iPositionX", value);
  hon_osd_get_position_y(OSD_CHANNAL_ID, &value);
  channel_name.emplace("iPositionY", value);
  hon_osd_get_enabled(OSD_CHANNAL_ID, &value);
  channel_name.emplace("iChannelNameOverlayEnabled", value);
  hon_osd_get_display_text(OSD_CHANNAL_ID, &tmp);
  channel_name.emplace("sChannelName", tmp);
  hon_osd_get_font_color_of_channelName(&tmp);
  channel_name.emplace("sOSDFrontColor", tmp);

  // date_time
  nlohmann::json date_time;
  hon_osd_get_position_x(OSD_DATE_TIME_ID, &value);
  date_time.emplace("iPositionX", value);
  hon_osd_get_position_y(OSD_DATE_TIME_ID, &value);
  date_time.emplace("iPositionY", value);
  hon_osd_get_enabled(OSD_DATE_TIME_ID, &value);
  date_time.emplace("iDateTimeOverlayEnabled", value);
  hon_osd_get_date_style(OSD_DATE_TIME_ID, &tmp);
  date_time.emplace("sDateStyle", tmp);
  hon_osd_get_time_style(OSD_DATE_TIME_ID, &tmp);
  date_time.emplace("sTimeStyle", tmp);
  hon_osd_get_display_week_enabled(OSD_DATE_TIME_ID, &value);
  date_time.emplace("iDisplayWeekEnabled", value);
  hon_osd_get_font_color_of_dateTime(&tmp);
  date_time.emplace("sOSDFrontColor", tmp);
  // character
  nlohmann::json character;
  hon_osd_get_position_x(OSD_CHARACTER_ID, &value);
  character.emplace("iPositionX", value);
  hon_osd_get_position_y(OSD_CHARACTER_ID, &value);
  character.emplace("iPositionY", value);
  hon_osd_get_enabled(OSD_CHARACTER_ID, &value);
  character.emplace("iTextOverlayEnabled", value);
  hon_osd_get_display_text(OSD_CHARACTER_ID, &tmp);
  character.emplace("sDisplayText", tmp);
  hon_osd_get_font_color_of_channelLocation(&tmp);
  character.emplace("sOSDFrontColor", tmp);
  // overlays
  osd_overlays.emplace("normalizedScreenSize", normalized_screen_size);
  osd_overlays.emplace("attribute", attribute);
  osd_overlays.emplace("channelNameOverlay", channel_name);
  osd_overlays.emplace("dateTimeOverlay", date_time);
  osd_overlays.emplace("characterOverlay", character);

  delete[] tmp;

  return osd_overlays;
}

nlohmann::json osd_image_get() {
  int value;
  char *str;
  char *tmp = new char[64];
  nlohmann::json osd_image;

  nlohmann::json normalized_screen_size;
  hon_osd_get_normalized_screen_width(&value);
  normalized_screen_size.emplace("iNormalizedScreenWidth", value);
  hon_osd_get_normalized_screen_height(&value);
  normalized_screen_size.emplace("iNormalizedScreenHeight", value);

  nlohmann::json image_overlay;
  hon_osd_get_position_x(OSD_IMAGE_ID, &value);
  image_overlay.emplace("iPositionX", value);
  hon_osd_get_position_y(OSD_IMAGE_ID, &value);
  image_overlay.emplace("iPositionY", value);
  hon_osd_get_enabled(OSD_IMAGE_ID, &value);
  image_overlay.emplace("iImageOverlayEnabled", value);
  image_overlay.emplace("iTransparentColorEnabled", 1);

  // get bmp picture width and height
  hon_osd_get_image_path(OSD_IMAGE_ID, &tmp);
  FILE *picture_file = fopen(tmp, "rb");
  int32_t bmp_width;
  int32_t bmp_height;
  int offset = sizeof(uint16_t) * 3 + sizeof(uint32_t) * 3;
  // skip other header information
  fseek(picture_file, offset, SEEK_SET);
  fread(&bmp_width, 1, sizeof(bmp_width), picture_file);
  fread(&bmp_height, 1, sizeof(bmp_height), picture_file);
  fclose(picture_file);
  image_overlay.emplace("iImageWidth", bmp_width);
  image_overlay.emplace("iImageHeight", bmp_height);

  osd_image.emplace("normalizedScreenSize", normalized_screen_size);
  osd_image.emplace("imageOverlay", image_overlay);

  delete[] tmp;
  return osd_image;
}

nlohmann::json osd_privacy_mask_get() {
  int value;
  char *str;
  char *tmp = new char[64];
  nlohmann::json osd_privacy_mask;
  nlohmann::json all_privacy_mask;

  nlohmann::json normalized_screen_size;
  hon_osd_get_normalized_screen_width(&value);
  normalized_screen_size.emplace("iNormalizedScreenWidth", value);
  hon_osd_get_normalized_screen_height(&value);
  normalized_screen_size.emplace("iNormalizedScreenHeight", value);

  for (int i = 3; i < 7; i++) {
    nlohmann::json privacy_mask;
    hon_osd_get_position_x(i, &value);
    privacy_mask.emplace("iPositionX", value);
    hon_osd_get_position_y(i, &value);
    privacy_mask.emplace("iPositionY", value);
    hon_osd_get_enabled(i, &value);
    privacy_mask.emplace("iPrivacyMaskEnabled", value);
    hon_osd_get_width(i, &value);
    privacy_mask.emplace("iMaskWidth", value);
    hon_osd_get_height(i, &value);
    privacy_mask.emplace("iMaskHeight", value);
    privacy_mask.emplace("id", i - 3); // Modified line
    all_privacy_mask.push_back(privacy_mask);
  }
  osd_privacy_mask.emplace("normalizedScreenSize", normalized_screen_size);
  osd_privacy_mask.emplace("privacyMask", all_privacy_mask);

  delete[] tmp;
  return osd_privacy_mask;
}

void osd_overlays_set(nlohmann::json osd_overlays) {
  int value_int;
  std::string value;

  for (auto &x : nlohmann::json::iterator_wrapper(osd_overlays)) {
    if (x.key() == KEY_NORMALIZED_SCREEN_SIZE) {
      // auto val = x.value(); /* string or int */
      // dbserver_media_set((char *)TABLE_NORMALIZED_SCREEN_SIZE,
      //                                (char *)val.dump().c_str(), 0);
    } 
    else if (x.key() == KEY_OSD_ATTRIBUTE) {
      nlohmann::json param = osd_overlays.at(KEY_OSD_ATTRIBUTE);
      if (param.dump().find("iBoundary") != param.dump().npos) {
        value_int = atoi(param.at("iBoundary").dump().c_str());
        hon_osd_set_boundary(value_int);
      }
      if (param.dump().find("sOSDFontSize") != param.dump().npos) {
        value = param.at("sOSDFontSize").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        sscanf(value.c_str(), "%d*%d", &value_int, &value_int);
        hon_osd_set_font_size(value_int);
      }
      if (param.dump().find("sAlignment") != param.dump().npos) {
        value = param.at("sAlignment").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_alignment(value.c_str());
      }
      if (param.dump().find("sOSDAttribute") != param.dump().npos) {
        value = param.at("sOSDAttribute").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_attribute(value.c_str());
      }
      if (param.dump().find("sOSDFrontColor") != param.dump().npos) {
        value = param.at("sOSDFrontColor").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_font_color(value.c_str());
      }
      if (param.dump().find("sOSDFrontColorMode") != param.dump().npos) {
        value = param.at("sOSDFrontColorMode").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_font_color_mode(value.c_str());
      }
    } else if (x.key() == KEY_OSD_CHANNEL_NAME_OVERLAY) {
      nlohmann::json param = osd_overlays.at(KEY_OSD_CHANNEL_NAME_OVERLAY);
      if (param.dump().find("iChannelNameOverlayEnabled") !=
          param.dump().npos) {
        value_int = atoi(param.at("iChannelNameOverlayEnabled").dump().c_str());
        hon_osd_set_enabled(OSD_CHANNAL_ID, value_int);
      }
      if (param.dump().find("iPositionX") != param.dump().npos) {
        value_int = atoi(param.at("iPositionX").dump().c_str());
        hon_osd_set_position_x(OSD_CHANNAL_ID, value_int);
      }
      if (param.dump().find("iPositionY") != param.dump().npos) {
        value_int = atoi(param.at("iPositionY").dump().c_str());
        hon_osd_set_position_y(OSD_CHANNAL_ID, value_int);
      }
      if (param.dump().find("sChannelName") != param.dump().npos) {
        value = param.at("sChannelName").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_display_text(OSD_CHANNAL_ID, value.c_str());
      }
      if (param.dump().find("sOSDFrontColor") != param.dump().npos) {
        value = param.at("sOSDFrontColor").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_font_color_of_channelName(value.c_str());
      }
    } else if (x.key() == KEY_OSD_DATE_TIME_OVERLAY) {
      nlohmann::json param = osd_overlays.at(KEY_OSD_DATE_TIME_OVERLAY);
      if (param.dump().find("iDateTimeOverlayEnabled") != param.dump().npos) {
        value_int = atoi(param.at("iDateTimeOverlayEnabled").dump().c_str());
        hon_osd_set_enabled(OSD_DATE_TIME_ID, value_int);
      }
      if (param.dump().find("iDisplayWeekEnabled") != param.dump().npos) {
        value_int = atoi(param.at("iDisplayWeekEnabled").dump().c_str());
        hon_osd_set_display_week_enabled(OSD_DATE_TIME_ID, value_int);
      }
      if (param.dump().find("iPositionX") != param.dump().npos) {
        value_int = atoi(param.at("iPositionX").dump().c_str());
        hon_osd_set_position_x(OSD_DATE_TIME_ID, value_int);
      }
      if (param.dump().find("iPositionY") != param.dump().npos) {
        value_int = atoi(param.at("iPositionY").dump().c_str());
        hon_osd_set_position_y(OSD_DATE_TIME_ID, value_int);
      }
      if (param.dump().find("sDateStyle") != param.dump().npos) {
        value = param.at("sDateStyle").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_date_style(OSD_DATE_TIME_ID, value.c_str());
      }
      if (param.dump().find("sTimeStyle") != param.dump().npos) {
        value = param.at("sTimeStyle").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_time_style(OSD_DATE_TIME_ID, value.c_str());
      }
      if (param.dump().find("sOSDFrontColor") != param.dump().npos){
        value = param.at("sOSDFrontColor").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_font_color_of_dateTime(value.c_str());
      }
    } else if (x.key() == KEY_OSD_CHARACTER_OVERLAY) {
      nlohmann::json param = osd_overlays.at(KEY_OSD_CHARACTER_OVERLAY);
      if (param.dump().find("iTextOverlayEnabled") != param.dump().npos) {
        value_int = atoi(param.at("iTextOverlayEnabled").dump().c_str());
        hon_osd_set_enabled(OSD_CHARACTER_ID, value_int);
      }
      if (param.dump().find("iPositionX") != param.dump().npos) {
        value_int = atoi(param.at("iPositionX").dump().c_str());
        hon_osd_set_position_x(OSD_CHARACTER_ID, value_int);
      }
      if (param.dump().find("iPositionY") != param.dump().npos) {
        value_int = atoi(param.at("iPositionY").dump().c_str());
        hon_osd_set_position_y(OSD_CHARACTER_ID, value_int);
      }
      if (param.dump().find("sDisplayText") != param.dump().npos) {
        value = param.at("sDisplayText").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_display_text(OSD_CHARACTER_ID, value.c_str());
      }
      if (param.dump().find("sOSDFrontColor") != param.dump().npos){
        value = param.at("sOSDFrontColor").dump();
        value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
        hon_osd_set_font_color_of_channelLocation(value.c_str());
      }
      
    }
  }
}

void osd_image_set(nlohmann::json osd_image_config) {
  // get bmp picture width and height
  int value_int;
  char *tmp = new char[64];
  // hon_osd_get_image_path(OSD_IMAGE_ID, &tmp);
  // FILE *picture_file = fopen(BMP_FILE_PATH, "rb");
  // int32_t bmp_width;
  // int32_t bmp_height;
  // int offset = sizeof(uint16_t) * 3 + sizeof(uint32_t) * 3;
  // // skip other header information
  // fseek(picture_file, offset, SEEK_SET);
  // fread(&bmp_width, 1, sizeof(bmp_width), picture_file);
  // fread(&bmp_height, 1, sizeof(bmp_height), picture_file);
  // fclose(picture_file);
  for (auto &x : nlohmann::json::iterator_wrapper(osd_image_config)) {
    if (x.key() == KEY_NORMALIZED_SCREEN_SIZE) {
      // auto val = x.value(); /* string or int */
      // dbserver_media_set((char *)TABLE_NORMALIZED_SCREEN_SIZE,
      //                                (char *)val.dump().c_str(), 0);
    } else if (x.key() == KEY_OSD_IMAGE_OVERLAY) {
      nlohmann::json param = osd_image_config.at(KEY_OSD_IMAGE_OVERLAY);
      if (param.dump().find("iImageOverlayEnabled") != param.dump().npos) {
        value_int = atoi(param.at("iImageOverlayEnabled").dump().c_str());
        hon_osd_set_enabled(OSD_IMAGE_ID, value_int);
      }
      if (param.dump().find("iPositionX") != param.dump().npos) {
        value_int = atoi(param.at("iPositionX").dump().c_str());
        hon_osd_set_position_x(OSD_IMAGE_ID, value_int);
      }
      if (param.dump().find("iPositionY") != param.dump().npos) {
        value_int = atoi(param.at("iPositionY").dump().c_str());
        hon_osd_set_position_y(OSD_IMAGE_ID, value_int);
      }
    }
  }
  delete[] tmp;
}

void osd_privacy_mask_set(nlohmann::json osd_privacy_mask_config) {
  int value_int;
  for (auto &x : nlohmann::json::iterator_wrapper(osd_privacy_mask_config)) {
    if (x.key() == KEY_NORMALIZED_SCREEN_SIZE) {
      // auto val = x.value(); /* string or int */
      // dbserver_media_set((char *)TABLE_NORMALIZED_SCREEN_SIZE,
      //                                (char *)val.dump().c_str(), 0);
    } 
    else if (x.key() == KEY_OSD_PRIVACY_MASK) {
      nlohmann::json all_privacy_mask =
          osd_privacy_mask_config.at(KEY_OSD_PRIVACY_MASK);
      for (auto &x : nlohmann::json::iterator_wrapper(all_privacy_mask)) {
        nlohmann::json param = x.value();
        int id = param.at("id");
        if (param.dump().find("iPrivacyMaskEnabled") != param.dump().npos) {
          value_int = atoi(param.at("iPrivacyMaskEnabled").dump().c_str());
          hon_osd_set_enabled(id + 3, value_int);
        }
        if (param.dump().find("iMaskWidth") != param.dump().npos) {
          value_int = atoi(param.at("iMaskWidth").dump().c_str());
          hon_osd_set_width(id + 3, value_int);
        }
        if (param.dump().find("iMaskHeight") != param.dump().npos) {
          value_int = atoi(param.at("iMaskHeight").dump().c_str());
          hon_osd_set_height(id + 3, value_int);
        }
        if (param.dump().find("iPositionX") != param.dump().npos) {
          value_int = atoi(param.at("iPositionX").dump().c_str());
          hon_osd_set_position_x(id + 3, value_int);
        }
        if (param.dump().find("iPositionY") != param.dump().npos) {
          value_int = atoi(param.at("iPositionY").dump().c_str());
          hon_osd_set_position_y(id + 3, value_int);
        }
      }
    }
  }
}

void OSDApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  std::string path_api_resource;
  std::string path_specific_resource;
  nlohmann::json content = nlohmann::json({});


  /* Get Path Information */
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
  }

  if (Req.Method == "GET") {
    if (!path_specific_resource.compare("overlays")) {
      content = osd_overlays_get();
    } 
    //Note: Disabled OSD image for now
    // else if (!path_specific_resource.compare("image")) {
    //   content = osd_image_get();
    // } 
    else if (!path_specific_resource.compare("privacy-mask")) {
      content = osd_privacy_mask_get();
    }

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk, content, "ok");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {

    nlohmann::json diff;
    nlohmann::json cfg_old;
    nlohmann::json osd_config = Req.PostObject; /* must be json::object */
    if (!path_specific_resource.compare("overlays")) {
      /* Erase unchanged data */
      cfg_old = osd_overlays_get();
      diff = nlohmann::json::diff(cfg_old, osd_config);
      for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
        if (diff.dump().find(x.key()) == diff.dump().npos)
          osd_config.erase(x.key());
      }
      minilog_debug("osd_config is %s\n", osd_config.dump().c_str());
      /* Set */
      if (!osd_config.empty()) {
        osd_overlays_set(osd_config);
        hon_osd_restart();
      }
      /* Get new info */
      content = osd_overlays_get();
      // osd_set_mediaserver(osd_config);
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "ok");
    } else if (!path_specific_resource.compare("image")) {
      /* Erase unchanged data */
      cfg_old = osd_image_get();
      diff = nlohmann::json::diff(cfg_old, osd_config);
      for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
        if (diff.dump().find(x.key()) == diff.dump().npos)
          osd_config.erase(x.key());
      }
      /* Set */
      if (!osd_config.empty()) {
        osd_image_set(osd_config);
        hon_osd_restart();
      }
      /* Get new info */
      content = osd_image_get();
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "ok");
    } else if (!path_specific_resource.compare("privacy-mask")) {
      /* Erase unchanged data */
      cfg_old = osd_privacy_mask_get();
      diff = nlohmann::json::diff(cfg_old, osd_config);
      for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
        if (diff.dump().find(x.key()) == diff.dump().npos)
          osd_config.erase(x.key());
      }
      /* Set */
      if (!osd_config.empty()) {
        osd_privacy_mask_set(osd_config);
        hon_osd_restart();
      }
      /* Get new info */
      minilog_debug("1\n");
      content = osd_privacy_mask_get();
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "ok");
    } else if (!path_specific_resource.compare("image/picture")) {
      for (int i = 0; i < Req.Files.size(); i++) {
        if (!Req.Files.at(i).getDataType().compare("image/bmp")) {
          if (Req.Files.at(i).getDataLength() > (256 * 1024)) { // 256kb
            // Resp.setErrorResponse(HttpStatus::kBadRequest,
            //                       "Picture is too big!");
            Resp.setHeader(HttpStatus::kBadRequest, "Picture is too big!");
            Resp.setApiData(HttpStatus::kBadRequest,content, "Picture is too big!");
          } else {
            int end_position = 0;
            char *tmp = new char[64];
            hon_osd_get_image_path(OSD_IMAGE_ID, &tmp);
            std::ofstream picture_file(tmp, std::ofstream::out);
            delete[] tmp;
            picture_file << Req.Files.at(i).getData();
            end_position = picture_file.tellp();
            picture_file.close();
            if (end_position == Req.Files.at(i).getDataLength()) {
              hon_osd_restart();
              Resp.setHeader(HttpStatus::kOk, "OK");
            } else {
              //Resp.setErrorResponse(HttpStatus::kBadRequest,
              //                      "Picture upload failed!");
              Resp.setHeader(HttpStatus::kBadRequest, "Picture upload failed!");
              Resp.setApiData(HttpStatus::kBadRequest,content, "Picture upload failed!d");
            }
          }
          i = Req.Files.size();
        } else if ((i == Req.Files.size() - 1) &&
                   (Req.Files.at(i).getDataType().compare("image/bmp"))) {
          //Resp.setErrorResponse(HttpStatus::kBadRequest, "Can't find bmp!");
          Resp.setHeader(HttpStatus::kBadRequest, "Can't find bmp!");
          Resp.setApiData(HttpStatus::kBadRequest,content, "Can't find bmp!");
        }
      }
    }
  } else {
    //Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
    Resp.setApiData(HttpStatus::kNotImplemented,content, "Not Implemented");
  }
}


} // namespace cgi
} // namespace rockchip
