// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __CGI_NETWORK_API_H__
#define __CGI_NETWORK_API_H__

#include "api_handler.h"
namespace rockchip {
namespace cgi {

class NetworkApiHandler : public ApiHandler {
public:
  NetworkApiHandler() = default;
  ~NetworkApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
};

class NetworkNTPApiHandler : public ApiHandler {
public:
  NetworkNTPApiHandler() = default;
  ~NetworkNTPApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
};

class NetworkDDNSApiHandler : public ApiHandler {
public:
  NetworkDDNSApiHandler() = default;
  ~NetworkDDNSApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
};

class NetworkPPPoEApiHandler : public ApiHandler {
public:
  NetworkPPPoEApiHandler() = default;
  ~NetworkPPPoEApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
};

class NetworkPortApiHandler : public ApiHandler {
public:
  NetworkPortApiHandler() = default;
  ~NetworkPortApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
};

class NetworkSMTPApiHandler : public ApiHandler {
public:
  NetworkSMTPApiHandler() = default;
  ~NetworkSMTPApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
};

class NetworkUPnPApiHandler : public ApiHandler {
public:
  NetworkUPnPApiHandler() = default;
  ~NetworkUPnPApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
};

class NetworkSNMPApiHandler : public ApiHandler {
public:
  NetworkSNMPApiHandler() = default;
  ~NetworkSNMPApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
};

class NetworkHTTPSApiHandler : public ApiHandler {
public:
  NetworkHTTPSApiHandler() = default;
  ~NetworkHTTPSApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
};

} // namespace cgi
} // namespace rockchip

#endif // __CGI_NETWORK_API_H__
