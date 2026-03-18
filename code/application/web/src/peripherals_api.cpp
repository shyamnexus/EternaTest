// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "peripherals_api.h"
#include "common.h"

namespace rockchip {
namespace cgi {

#ifdef TEST

void PeripheralsApiHandler::handler(const HttpRequest &Req,
                                    HttpResponse &Resp) {
  nlohmann::json content;

  if (Req.Method == "GET") {
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(content);
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(content);
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

#endif

} // namespace cgi
} // namespace rockchip
