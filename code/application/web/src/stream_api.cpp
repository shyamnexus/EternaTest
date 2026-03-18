// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "stream_api.h"
#include "common.h"

namespace rockchip {
namespace cgi {

void StreamURLApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  nlohmann::json content;
  if (Req.Method == "GET") {
    char *str =
        (char *)"[{\"id\":0,\"sStreamProtocol\":\"HTTPS\"},{\"id\":1,"
                "\"sStreamProtocol\":\"HTTPS\"},{\"id\":2,\"sStreamProtocol\":"
                "\"HTTPS\"}]";
    nlohmann::json stream_url_config = nlohmann::json::parse(str);
    int pos_first = Req.PathInfo.find_first_of("/");
    int pos_last = Req.PathInfo.find_last_of("/");

    /* Get ip address */
    std::string ipv4_address = ipv4_address_get();
    minilog_debug("ipv4_address is %s\n", ipv4_address.c_str());

    /* Get port */
    // std::string http_port = "80";
    // std::string rtsp_port = "554";
    // std::string rtmp_port = "1935";

    /* Get URL by splicing */
    for (int i = 0; i < 3; i++) {
      std::string url;
      std::string stream_type;

      if (i == 0)
        stream_type = "mainstream";
      else if (i == 1)
        stream_type = "substream";
      else if (i == 2)
        stream_type = "thirdstream";
      // url = "http://" + ipv4_address + ":" + http_port +
      //       "/live?port=" + rtmp_port + "&app=live&stream=" + stream_type;

      url = "https://" + ipv4_address +"/hls/"+ stream_type + ".m3u8";
      // example :
      // https://192.168.2.106/hls/mainstream.m3u8
      stream_url_config.at(i).emplace("sURL", url);
    }
    // stream_url_config.emplace("status", 200);
    // stream_url_config.emplace("msg", "Valid stream url request");
    content = stream_url_config;
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content, "OK");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    Resp.setHeader(HttpStatus::kNotImplemented, "kNotImplemented");
    Resp.setApiData(HttpStatus::kNotImplemented,content,"kNotImplemented");
  } else {
   // Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
    Resp.setApiData(HttpStatus::kNotImplemented,content,"Not Implemented");
  }
}


} // namespace cgi
} // namespace rockchip
