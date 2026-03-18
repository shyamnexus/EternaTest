// Copyright 2019-2022 Fuzhou Rockchip Electronics Co., Ltd. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "roi_api.h"
#include "common.h"

namespace rockchip {
namespace cgi {

#ifdef TEST

nlohmann::json roi_info_get() {
  int value;
  char *str;
  char *tmp = new char[64];
  nlohmann::json roi_info;
  nlohmann::json normalized_screen_size;

  hon_osd_get_normalized_screen_width(&value);
  normalized_screen_size.emplace("iNormalizedScreenWidth", value);
  hon_osd_get_normalized_screen_height(&value);
  normalized_screen_size.emplace("iNormalizedScreenHeight", value);

  // roi_region_list
  nlohmann::json roi_region_list;
  for (int i = 0; i < 6; i++) {
    nlohmann::json roi_region;
    hon_roi_get_position_x(i, &value);
    roi_region.emplace("iPositionX", value);
    hon_roi_get_position_y(i, &value);
    roi_region.emplace("iPositionY", value);
    hon_roi_get_width(i, &value);
    roi_region.emplace("iWidth", value);
    hon_roi_get_height(i, &value);
    roi_region.emplace("iHeight", value);
    hon_roi_get_id(i, &value);
    roi_region.emplace("iROIId", value);
    hon_roi_get_quality_level(i, &value);
    roi_region.emplace("iQualityLevelOfROI", value);
    hon_roi_get_enabled(i, &value);
    roi_region.emplace("iROIEnabled", value);
    roi_region.emplace("iStreamEnabled", 0);
    hon_roi_get_stream_type(i, &tmp);
    roi_region.emplace("sStreamType", tmp);
    hon_roi_get_name(i, &tmp);
    roi_region.emplace("sName", tmp);
    roi_region_list.push_back(roi_region);
  }
  // roi_info
  roi_info.emplace("normalizedScreenSize", normalized_screen_size);
  roi_info.emplace("ROIRegionList", roi_region_list);

  delete[] tmp;

  return roi_info;
}

nlohmann::json roi_specific_get(std::string stream_type, int roi_id) {
  int actual_id, value;
  char *tmp = new char[64];
  nlohmann::json roi_region;

  if (!stream_type.compare("main-stream")) {
    if (roi_id == 1)
      actual_id = 0;
    else if (roi_id == 2)
      actual_id = 1;
  } else if (!stream_type.compare("sub-stream")) {
    if (roi_id == 1)
      actual_id = 2;
    else if (roi_id == 2)
      actual_id = 3;
  } else if (!stream_type.compare("third-stream")) {
    if (roi_id == 1)
      actual_id = 4;
    else if (roi_id == 2)
      actual_id = 5;
  }
  hon_roi_get_position_x(actual_id, &value);
  roi_region.emplace("iPositionX", value);
  hon_roi_get_position_y(actual_id, &value);
  roi_region.emplace("iPositionY", value);
  hon_roi_get_width(actual_id, &value);
  roi_region.emplace("iWidth", value);
  hon_roi_get_height(actual_id, &value);
  roi_region.emplace("iHeight", value);
  hon_roi_get_id(actual_id, &value);
  roi_region.emplace("iROIId", value);
  hon_roi_get_quality_level(actual_id, &value);
  roi_region.emplace("iQualityLevelOfROI", value);
  hon_roi_get_enabled(actual_id, &value);
  roi_region.emplace("iROIEnabled", value);
  roi_region.emplace("iStreamEnabled", 0);
  hon_roi_get_stream_type(actual_id, &tmp);
  roi_region.emplace("sStreamType", tmp);
  hon_roi_get_name(actual_id, &tmp);
  roi_region.emplace("sName", tmp);

  return roi_region;
}

void roi_specific_set(nlohmann::json roi_config, std::string stream_type,
                      int roi_id) {
  int actual_id;
  int value_int;
  std::string value;

  if (!stream_type.compare("main-stream")) {
    if (roi_id == 1)
      actual_id = 0;
    else if (roi_id == 2)
      actual_id = 1;
  } else if (!stream_type.compare("sub-stream")) {
    if (roi_id == 1)
      actual_id = 2;
    else if (roi_id == 2)
      actual_id = 3;
  } else if (!stream_type.compare("third-stream")) {
    if (roi_id == 1)
      actual_id = 4;
    else if (roi_id == 2)
      actual_id = 5;
  }
  if (roi_config.dump().find("sStreamType") != roi_config.dump().npos) {
    value = roi_config.at("sStreamType").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_roi_set_stream_type(actual_id, value.c_str());
  }
  if (roi_config.dump().find("sName") != roi_config.dump().npos) {
    value = roi_config.at("sName").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_roi_set_name(actual_id, value.c_str());
  }
  if (roi_config.dump().find("iROIEnabled") != roi_config.dump().npos) {
    value_int = atoi(roi_config.at("iROIEnabled").dump().c_str());
    hon_roi_set_enabled(actual_id, value_int);
  }
  if (roi_config.dump().find("iPositionX") != roi_config.dump().npos) {
    value_int = atoi(roi_config.at("iPositionX").dump().c_str());
    hon_roi_set_position_x(actual_id, value_int);
  }
  if (roi_config.dump().find("iPositionY") != roi_config.dump().npos) {
    value_int = atoi(roi_config.at("iPositionY").dump().c_str());
    hon_roi_set_position_y(actual_id, value_int);
  }
  if (roi_config.dump().find("iHeight") != roi_config.dump().npos) {
    value_int = atoi(roi_config.at("iHeight").dump().c_str());
    hon_roi_set_height(actual_id, value_int);
  }
  if (roi_config.dump().find("iWidth") != roi_config.dump().npos) {
    value_int = atoi(roi_config.at("iWidth").dump().c_str());
    hon_roi_set_width(actual_id, value_int);
  }
  if (roi_config.dump().find("iQualityLevelOfROI") != roi_config.dump().npos) {
    value_int = atoi(roi_config.at("iQualityLevelOfROI").dump().c_str());
    hon_roi_set_quality_level(actual_id, value_int);
  }
  hon_roi_set_all();
}

void ROIApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  std::string path_api_resource;
  std::string path_channel_resource;
  std::string path_specific_resource;
  char *str;
  nlohmann::json content;

  /* Get Path Information */
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_channel_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
    pos_first = path_channel_resource.find_first_of("/");
    if (pos_first != -1) {
      path_specific_resource = path_channel_resource.substr(
          pos_first + 1, path_channel_resource.size());
    }
  }
  if (Req.Method == "GET") {
    if (path_channel_resource.empty()) { // path is roi
      content = roi_info_get();
    } else {
      if (path_specific_resource.empty()) { // path example is roi/main-stream
        // content = roi_channel_get(path_channel_resource);
        // for (auto &x : nlohmann::json::iterator_wrapper(content)) {
        //   x.value().erase("id");
        // }
      } else { // path example is roi/main-stream/1
        int roi_id = stoi(path_specific_resource);
        int pos = path_channel_resource.find_first_of("/");
        std::string stream_type = path_channel_resource.substr(0, pos);
        content = roi_specific_get(stream_type, roi_id);
        content.erase("id");
      }
    }

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(content);
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    if (path_channel_resource.empty()) { // path is roi
      Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    } else {

      if (path_specific_resource.empty()) { // path example is roi/main-stream
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      } else { // path example is roi/main-stream/1
        nlohmann::json roi_config = Req.PostObject; /* must be json::object */
        int roi_id = stoi(path_specific_resource);
        int pos = path_channel_resource.find_first_of("/");
        std::string stream_type = path_channel_resource.substr(0, pos);
        /* Set */
        if (!roi_config.empty())
          roi_specific_set(roi_config, stream_type, roi_id);
        /* Get new info */
        content = roi_specific_get(stream_type, roi_id);

        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      }
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}
#endif

} // namespace cgi
} // namespace rockchip
