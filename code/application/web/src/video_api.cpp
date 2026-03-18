// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_api.h"
#include "common.h"
#include "osd_api.h"
#include "ipcweb_client/client.h"
namespace rockchip {
namespace cgi {

// void video_smart_rules(nlohmann::json &config) {
//   int bitrate, bitrate_max, bitrate_min;
//   bitrate_max = atoi(config.at("iMaxRate").dump().c_str());
//   bitrate_min = atoi(config.at("iMinRate").dump().c_str());
//   bitrate = (bitrate_max + bitrate_min) >> 1;
// }

nlohmann::json video_param_get(int stream_id) {
  nlohmann::json param;
  param.emplace("id", stream_id);

  char *tmp = new char[20];

  int gop = 0;
  hon_video_get_gop(stream_id, &gop);
  param.emplace("iGOP", gop);

  int max_rate = 0;
  hon_video_get_max_rate(stream_id, &max_rate);
  param.emplace("iMaxRate", max_rate);

  hon_video_get_RC_mode(stream_id, &tmp);
  param.emplace("sRCMode", tmp);

  hon_video_get_output_data_type(stream_id, &tmp);
  param.emplace("sOutputDataType", tmp);

  hon_video_get_rc_quality(stream_id, &tmp);
  param.emplace("sRCQuality", tmp);

  // hon_video_get_smart(stream_id, &tmp);
  // param.emplace("sSmart", tmp);

  // hon_video_get_gop_mode(stream_id, &tmp);
  // param.emplace("sGOPMode", tmp);

  hon_video_get_stream_type(stream_id, &tmp);
  param.emplace("sStreamType", tmp);

  // hon_video_get_h264_profile(stream_id, &tmp);
  // param.emplace("sH264Profile", tmp);

  hon_video_get_resolution(stream_id, &tmp);
  param.emplace("sResolution", tmp);

  hon_video_get_frame_rate(stream_id, &tmp);
  param.emplace("sFrameRate", tmp);

  // hon_video_get_frame_rate_in(stream_id, &tmp);
  // param.emplace("sFrameRateIn", tmp);
  delete[] tmp;

  return param;
}

int video_set_param(int stream_id, nlohmann::json param) {
  int value_int;
  std::string value;

  if (param.dump().find("iGOP") != param.dump().npos) {
    value_int = atoi(param.at("iGOP").dump().c_str());
    hon_video_set_gop(stream_id, value_int);
  }
  if (param.dump().find("iMaxRate") != param.dump().npos) {
    value_int = atoi(param.at("iMaxRate").dump().c_str());
    hon_video_set_max_rate(stream_id, value_int);
  }
  if (param.dump().find("sRCMode") != param.dump().npos) {
    value = param.at("sRCMode").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_video_set_RC_mode(stream_id, value.c_str());
  }
  if (param.dump().find("sOutputDataType") != param.dump().npos) {
    value = param.at("sOutputDataType").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_video_set_output_data_type(stream_id, value.c_str());
  }
  if (param.dump().find("sRCQuality") != param.dump().npos) {
    value = param.at("sRCQuality").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_video_set_rc_quality(stream_id, value.c_str());
  }
  // if (param.dump().find("sSmart") != param.dump().npos) {
  //   value = param.at("sSmart").dump();
  //   value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
  //   hon_video_set_smart(stream_id, value.c_str());
  // }
  // if (param.dump().find("sGOPMode") != param.dump().npos) {
  //   value = param.at("sGOPMode").dump();
  //   value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
  //   hon_video_set_gop_mode(stream_id, value.c_str());
  // }
  if (param.dump().find("sStreamType") != param.dump().npos) {
    value = param.at("sStreamType").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_video_set_stream_type(stream_id, value.c_str());
  }
  // if (param.dump().find("sH264Profile") != param.dump().npos) {
  //   value = param.at("sH264Profile").dump();
  //   value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
  //   hon_video_set_h264_profile(stream_id, value.c_str());
  // }
  if (param.dump().find("sResolution") != param.dump().npos) {
    value = param.at("sResolution").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_video_set_resolution(stream_id, value.c_str());
  }
  if (param.dump().find("sFrameRate") != param.dump().npos) {
    value = param.at("sFrameRate").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_video_set_frame_rate(stream_id, value.c_str());
  }
  // if (param.dump().find("sFrameRateIn") != param.dump().npos) {
  //   value = param.at("sFrameRateIn").dump();
  //   value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
  //   hon_video_set_frame_rate_in(stream_id, value.c_str());
  // }
  // hon_video_restart();

  return 0;
}

nlohmann::json video_region_clip_info_get() {
  int id = 2; // tmp only support third-stream
  int value;
  char *str;
  nlohmann::json region_clip_info;
  nlohmann::json normalized_screen_size;

  hon_osd_get_normalized_screen_width(&value);
  normalized_screen_size.emplace("iNormalizedScreenWidth", value);
  hon_osd_get_normalized_screen_height(&value);
  normalized_screen_size.emplace("iNormalizedScreenHeight", value);

  // region_clip_region
  nlohmann::json region_clip_region;
  hon_region_clip_get_position_x(id, &value);
  region_clip_region.emplace("iPositionX", value);
  hon_region_clip_get_position_y(id, &value);
  region_clip_region.emplace("iPositionY", value);
  hon_region_clip_get_width(id, &value);
  region_clip_region.emplace("iWidth", value);
  hon_region_clip_get_height(id, &value);
  region_clip_region.emplace("iHeight", value);
  hon_region_clip_get_enabled(id, &value);
  region_clip_region.emplace("iRegionClipEnabled", value);
  // region_clip_info
  region_clip_info.emplace("normalizedScreenSize", normalized_screen_size);
  region_clip_info.emplace("regionClip", region_clip_region);

  return region_clip_info;
}

void video_region_clip_set(nlohmann::json region_clip_config, int id) {
  int value_int;
  std::string value;

  if (region_clip_config.dump().find("iRegionClipEnabled") !=
      region_clip_config.dump().npos) {
    value_int =
        atoi(region_clip_config.at("iRegionClipEnabled").dump().c_str());
    hon_region_clip_set_enabled(id, value_int);
  }
  if (region_clip_config.dump().find("iPositionX") !=
      region_clip_config.dump().npos) {
    value_int = atoi(region_clip_config.at("iPositionX").dump().c_str());
    hon_region_clip_set_position_x(id, value_int);
  }
  if (region_clip_config.dump().find("iPositionY") !=
      region_clip_config.dump().npos) {
    value_int = atoi(region_clip_config.at("iPositionY").dump().c_str());
    hon_region_clip_set_position_y(id, value_int);
  }
  if (region_clip_config.dump().find("iHeight") !=
      region_clip_config.dump().npos) {
    value_int = atoi(region_clip_config.at("iHeight").dump().c_str());
    hon_region_clip_set_height(id, value_int);
  }
  if (region_clip_config.dump().find("iWidth") !=
      region_clip_config.dump().npos) {
    value_int = atoi(region_clip_config.at("iWidth").dump().c_str());
    hon_region_clip_set_width(id, value_int);
  }
  hon_region_clip_set_all();
}

void VideoApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  std::string path_api_resource;
  std::string path_stream_resource;
  std::string path_specific_resource;
  std::string path_function_resource;
  char *str;
  nlohmann::json content = nlohmann::json({}) ;


  // Get Path Information
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_stream_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
    pos_first = path_stream_resource.find_first_of("/");
    if (pos_first != -1) {
      path_specific_resource = path_stream_resource.substr(
          pos_first + 1, path_stream_resource.size());
      pos_first = path_specific_resource.find_first_of("/");
      if (pos_first != -1) {
        path_function_resource = path_specific_resource.substr(
            pos_first + 1, path_specific_resource.size());
      }
    }
  }

  if (Req.Method == "GET") {
    if (path_stream_resource.empty()) { // path is video
      std::vector<nlohmann::json> content;
      content.push_back(video_param_get(0));
      content.push_back(video_param_get(1));
      content.push_back(video_param_get(2));
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk,content, "OK");
    } else {
      if (path_specific_resource.empty()) { // path example is video/0
        int id = stoi(path_stream_resource);
        content = video_param_get(id);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk,content, "OK");
      } else if (!path_stream_resource.compare("2/region-clip")) {
        // path must is video/2/region-clip
        content = video_region_clip_info_get();
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk,content, "OK");
      } else if (!path_specific_resource.compare("advanced-enc")) {
        // path example is viedo/0/advanced-enc
        // str = dbserver_media_get((char *)TABLE_VIDEO_ADVANCED_ENC);
        // content = nlohmann::json::parse(str).at("jData");
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk,content, "OK");
      } else {
       // Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
        Resp.setApiData(HttpStatus::kNotImplemented,content, "Not Implemented");
      }
    }
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    nlohmann::json video_config = Req.PostObject; // must be json::object
    if (path_stream_resource.empty()) {           // path is video
      //Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
       Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
       Resp.setApiData(HttpStatus::kNotImplemented,content, "Not Implemented");
    } else {
      if (path_specific_resource.empty()) { // path example is video/0
        if(video_config.empty())
        {
          //Resp.setErrorResponse(HttpStatus::kBadRequest, "Missing video configuration");
          Resp.setHeader(HttpStatus::kBadRequest, "Missing video configuration");
          Resp.setApiData(HttpStatus::kBadRequest,content, "Missing video configuration");
        }
        else
        {
          int stream_id = atoi(path_stream_resource.c_str());
          nlohmann::json cfg_old = video_param_get(stream_id);
          nlohmann::json diff = nlohmann::json::diff(cfg_old, video_config);
          for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
            if (diff.dump().find("\"replace\",\"path\":\"/" + x.key()) ==
                diff.dump().npos)
              video_config.erase(x.key());
          }
          minilog_debug("video_config is %s\n", video_config.dump().c_str());
          video_set_param(stream_id, video_config);
          content = video_param_get(stream_id);
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk,content, "Video Encoder details has been changed successfully.");
        }
      } else if (!path_stream_resource.compare("2/region-clip")) {
        // path must is video/2/region-clip
        if (!video_config.empty())
          video_region_clip_set(video_config, 2); // temp only one table
        // get new info
        content = video_region_clip_info_get();
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk,content, "OK");
      } else if (path_specific_resource.find("advanced-enc") !=
                 path_specific_resource.npos) {
        if (path_function_resource.empty()) {
          // path is video/0/advanced-enc
        //  Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
            Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
            Resp.setApiData(HttpStatus::kNotImplemented,content, "Not Implemented");
        } else {
          // path example is video/0/advanced-enc/qp
          minilog_debug("path_function_resource is %s\n",
                        path_function_resource.c_str());
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk,content,"OK");
        }
      } else {
       // Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
        Resp.setApiData(HttpStatus::kNotImplemented,content, "Not Implemented");
      }
    }
  } else {
   // Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
     Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
     Resp.setApiData(HttpStatus::kNotImplemented,content, "Not Implemented");
  }
}

} // namespace cgi
} // namespace rockchip
