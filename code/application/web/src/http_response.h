// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __CGI_HTTP_RESPONSE_H__
#define __CGI_HTTP_RESPONSE_H__

#include <map>
#include <memory>
#include <string>

#include "export.h"
#include "nlohmann/json.hpp"

namespace rockchip {
namespace cgi {

enum class CGI_EXPORT HttpStatus : int {
  kOk = 200,
  kCreated = 201,
  kAccepted = 202,
  kNoContent = 204,
  kMultipleChoices = 300,
  kMovedPermanently = 301,
  kMovedTemporarily = 302,
  kNotModified = 304,
  kResumeIncomplete = 308,
  kBadRequest = 400,
  kUnauthorized = 401,
  kForbidden = 403,
  kNotFound = 404,
  kLocked = 423,
  kRateLimiting = 429,
  kInternalServerError = 500,
  kNotImplemented = 501,
  kBadGateway = 502,
  kServiceUnavailable = 503,
};

enum class CGI_EXPORT RetCode : int {
  API_SUCCESS = 0,
  API_FAILURE = 1,
  API_INAVLID_ARGUMENT = 2,
  API_SQL_DB_FAILURE = 3,
  API_MEM_ALLOC_FAILURE = 4,
  API_MAX_LIMIT = 5,
  RULE_COUNT_EXCEEDS = 6,
  API_REQUEST_CHECK_SUCCESS  = 7,
  API_REQUEST_CHECK_FAILURE = 8,
  API_NAME_LENGTH_FAILURE = 9,
  API_COORDINATE_INVALID = 10,
  RULE_NAME_LENGTH_FAILURE = 11,
  API_NOT_IMPLEMENTED = 12,
  RULE_NAME_EXISTS = 13,
  INVALID_RULE_NAME = 14,
  NON_CONVEX_COORDINATES = 15,
  TRIPWIRE_COORDINATE_SIZE_FAILURE = 16,
  TRESPASS_COORDINATE_SIZE_FAILURE = 17,
};

class HttpResponseImpl;
class CGI_EXPORT HttpResponse {
public:
  HttpResponse();
  virtual ~HttpResponse();

  void setHeader(HttpStatus Status, const std::string &Reason);
  void setHeader(HttpStatus Status);
  void addHeader(const std::string &Key, const std::string &Value);
  void setCookie(const std::string &Name, const std::string &Value, unsigned long MaxAge);
  void setErrorResponse(HttpStatus Status, const std::string &Message);
  void setInterceptErrorResponse(HttpStatus Status, const std::string &Message);
  void setApiContent(const std::string &Kind);
  void appendApiItem(const std::string &Item);
  void appendApiItem(const nlohmann::json &Item);
  void appendApiItems(const nlohmann::json &Items);
  void setApiData(HttpStatus Status, const nlohmann::json &Data, const std::string &Message);
  void render(std::ostream &Stream);

private:
  std::unique_ptr<HttpResponseImpl> impl;
};

class CGI_EXPORT HttpResponseHandler {
  public:
    void setResponse(HttpResponse &Resp, HttpStatus StatusCode, RetCode ret_code, const std::string &Message);
};

} // namespace cgi
} // namespace rockchip

#endif // __CGI_HTTP_RESPONSE_H__
