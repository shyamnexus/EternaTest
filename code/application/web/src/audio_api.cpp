// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "audio_api.h"
#include "common.h"

namespace rockchip {
namespace cgi {

nlohmann::json audio_get_param(int stream_id) {
  nlohmann::json param;
  param.emplace("id", stream_id);

  char *tmp = new char[20];

  int value = 0;

  hon_audio_get_bit_rate(stream_id, &value);
  param.emplace("iBitRate", value);

  hon_audio_get_sample_rate(stream_id, &value);
  param.emplace("iSampleRate", value);

  hon_audio_get_volume(stream_id, &value);
  param.emplace("iVolume", value);

  hon_audio_get_enable_vqe(stream_id, &value);
  if (value)
    param.emplace("sVQE", "open");
  else
    param.emplace("sVQE", "close");

  hon_audio_get_encode_type(stream_id, &tmp);
  param.emplace("sEncodeType", tmp);
  delete[] tmp;

  param.emplace("sInput", "micIn");

  return param;
}

int audio_set_param(int stream_id, nlohmann::json param) {
  int value_int;
  std::string value;

  bool audioCodecChange = false;

  if (param.dump().find("iBitRate") != param.dump().npos) {
    value_int = atoi(param.at("iBitRate").dump().c_str());
    hon_audio_set_bit_rate(stream_id, value_int);
  }
  if (param.dump().find("iSampleRate") != param.dump().npos) {
    value_int = atoi(param.at("iSampleRate").dump().c_str());
    hon_audio_set_sample_rate(stream_id, value_int);
  }
  if (param.dump().find("iVolume") != param.dump().npos) {
    value_int = atoi(param.at("iVolume").dump().c_str());
    hon_audio_set_volume(stream_id, value_int);
  }
  if (param.dump().find("sVQE") != param.dump().npos) {
    value = param.at("sVQE").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    if (!strcmp(value.c_str(), "open"))
      hon_audio_set_enable_vqe(stream_id, 1);
    else
      hon_audio_set_enable_vqe(stream_id, 0);
  }
  if (param.dump().find("sEncodeType") != param.dump().npos) {
    value = param.at("sEncodeType").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_audio_set_encode_type(stream_id, value.c_str());
    audioCodecChange = true;
  }
  if(audioCodecChange)
  {
    audioCodecChange = false;
    hon_video_restart();
  }
  else{
    hon_audio_restart();
  }
  return 0;
}

void AudioApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  nlohmann::json content;

  if (Req.Method == "GET") {
    content = audio_get_param(0);
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {

    nlohmann::json audio_config = Req.PostObject; /* must be json::object */
    /* Erase unchanged data */
    nlohmann::json cfg_old = audio_get_param(0);
    nlohmann::json diff = nlohmann::json::diff(cfg_old, audio_config);
    for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
      if (diff.dump().find(x.key()) == diff.dump().npos)
        audio_config.erase(x.key());
    }

    /* set */
    if (!audio_config.empty())
      audio_set_param(0, audio_config);

    /* get new info */
    content = audio_get_param(0);
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else {
     Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
     Resp.setApiData(HttpStatus::kNotImplemented,content,"Not Implemented");
  }
}

} // namespace cgi
} // namespace rockchip
