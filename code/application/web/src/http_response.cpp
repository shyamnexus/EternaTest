// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "http_response.h"
#include "cgicc/HTTPResponseHeader.h"
#include <iostream>
#include "minilog.h"
namespace rockchip {
namespace cgi {
inline namespace detail {
const static std::string kOk = "OK";
const static std::string kCreated = "Created";
const static std::string kAccepted = "Accepted";
const static std::string kNoContent = "No Content";
const static std::string kMultipleChoices = "Multiple Choices";
const static std::string kMovedPermanently = "Moved Permanently";
const static std::string kMovedTemporarily = "Moved Temporarily";
const static std::string kNotModified = "Not Modified";
const static std::string kResumeIncomplete = "Resume Incomplete";
const static std::string kBadRequest = "Bad Request";
const static std::string kUnauthorized = "Unauthorized";
const static std::string kForbidden = "Forbidden";
const static std::string kNotFound = "Not Found";
const static std::string kInternalServerError = "Internal Server Error";
const static std::string kNotImplemented = "Not Implemented";
const static std::string kBadGateway = "Bad Gateway";
const static std::string kServiceUnavailable = "Service Unavailable";

const static std::string kHttpVersion = "HTTP/1.1";
const static std::string kApiVersion = "1.0";

static std::map<HttpStatus, std::string> CreateStatusMap() {
  std::map<HttpStatus, std::string> mapStatus;
  mapStatus[HttpStatus::kOk] = kOk;
  mapStatus[HttpStatus::kCreated] = kCreated;
  mapStatus[HttpStatus::kAccepted] = kAccepted;
  mapStatus[HttpStatus::kNoContent] = kNoContent;
  mapStatus[HttpStatus::kMultipleChoices] = kMultipleChoices;
  mapStatus[HttpStatus::kMovedPermanently] = kMovedPermanently;
  mapStatus[HttpStatus::kMovedTemporarily] = kMovedTemporarily;
  mapStatus[HttpStatus::kNotModified] = kNotModified;
  mapStatus[HttpStatus::kBadRequest] = kBadRequest;
  mapStatus[HttpStatus::kUnauthorized] = kUnauthorized;
  mapStatus[HttpStatus::kForbidden] = kForbidden;
  mapStatus[HttpStatus::kNotFound] = kNotFound;
  mapStatus[HttpStatus::kInternalServerError] = kInternalServerError;
  mapStatus[HttpStatus::kNotImplemented] = kNotImplemented;
  mapStatus[HttpStatus::kBadGateway] = kBadGateway;
  mapStatus[HttpStatus::kServiceUnavailable] = kServiceUnavailable;

  return mapStatus;
}
const static std::map<HttpStatus, std::string> StatusMap = CreateStatusMap();

std::string getStatusString(const HttpStatus &Status) {
  std::map<HttpStatus, std::string>::const_iterator finder =
      StatusMap.find(Status);
  if (finder != StatusMap.end()) {
    return finder->second;
  } else {
    return kInternalServerError;
  }
}

} // namespace detail

class HttpResponseImpl {
public:
  HttpResponseImpl() : ApiData_(nlohmann::json::object()){};
  virtual ~HttpResponseImpl() = default;

  void setHeader(std::shared_ptr<cgicc::HTTPResponseHeader> header) {
    Header_ = header;
  }

  void setHeader(HttpStatus status, const std::string &reason) {
    // std::string statusLine = "Status: " + std::to_string(static_cast<int>(status)) + " " + reason + "\r\n";
    Header_ = std::make_shared<cgicc::HTTPResponseHeader>(
        kHttpVersion, static_cast<int>(status), reason);
    //     // Output the Status line and the rest of the headers
    // std::cout << statusLine;
    // std::cout << *Header_ << std::endl;
   

    // int value = Header_->getStatusCode();
    // std::string reason = Header_->getReasonPhrase();
   

// minilog::log(minilog::Log, "Header getStatusCode: %d", value);
// minilog::log(minilog::Log, "Header getReasonPhrase: %s", reason);
   
  }

  void setHeader(HttpStatus status) {
    
    Header_ = std::make_shared<cgicc::HTTPResponseHeader>(
        kHttpVersion, static_cast<int>(status), getStatusString(status));
  }

  void addHeader(const std::string &Key, const std::string &Value) {
    Header_->addHeader(Key, Value);
  }

  void setCookie(const std::string &Name, const std::string &Value,
                 unsigned long MaxAge) {
    Header_->setCookie(
        cgicc::HTTPCookie(Name, Value, "", "", MaxAge, "/", false));
  }

  std::shared_ptr<cgicc::HTTPResponseHeader> getHeader(void) { return Header_; }

  void setErrorResponse(HttpStatus Status, const std::string &Message) {
    setHeader(Status, Message);
    ApiData_.clear();
    ApiData_["error"] = {};
    ApiData_["error"]["code"] = static_cast<int>(Status);
    ApiData_["error"]["message"] = Message;
    
  }
  void setInterceptErrorResponse(HttpStatus Status, const std::string &Message) {

    std::cout << "Status: " << static_cast<int>(Status) << " " << Message << "\r\n";
    std::cout << "Content-Type: application/json\r\n";
    std::cout << "Access-Control-Allow-Origin: http://localhost:4200\r\n"; // CORS header
    std::cout << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"; // CORS header
    std::cout << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"; // CORS header
    std::cout << "\r\n";
  // std::cout << "{ \"error\": { \"code\": " << static_cast<int>(Status) << ", \"message\": \"" << Message << "\" }, \"status\": " << static_cast<int>(Status) << ", \"statusText\": \"" << Message << "\" }" << std::endl;
   std::cout << "{ \"status\": " << static_cast<int>(Status) << " }" << std::endl;
  }
  void setApiContent(const std::string &Kind) {
    ApiData_.clear();
    ApiData_.push_back({"apiVersion", kApiVersion});
    ApiData_.push_back({"kind", Kind});
    ApiData_.push_back({"data", nlohmann::json::object()});
    ApiData_["data"]["items"] = nlohmann::json::array();
  }

  void appendApiItem(const std::string &Item) {
    ApiData_["data"]["items"].push_back(nlohmann::json::parse(Item));
  }

  void appendApiItem(const nlohmann::json &Item) {
    assert(Item.is_object());
    ApiData_["data"]["items"].push_back(Item);
  }

  void appendApiItems(const nlohmann::json &Items) {
    assert(Items.is_array());
    for (auto Item : Items) {
      ApiData_["data"]["items"].push_back(Item);
    }
  }

  void setApiData(HttpStatus Status,const nlohmann::json &Data,const std::string &Message) {
  
    ApiData_.clear();
    ApiData_["status"] = static_cast<int>(Status);
    ApiData_["message"] = Message; 
    ApiData_["data"]= Data; 

  }

  void render(std::ostream &stream) {
    std::string Content;
    if (!Header_) {
      setErrorResponse(HttpStatus::kInternalServerError,
                       "Forgot to set header!");
    } else {
      Header_->addHeader("Content-Type", "application/json;charset-utf8");
      Content = ApiData_.dump();
      Header_->addHeader("Content-Length", std::to_string(Content.length()));
      Header_->render(stream);
          // int value = Header_->getStatusCode();
    

// minilog::log(minilog::Log, "Header getStatusCode: %d", value);
      stream << Content;
    }
    // minilog::initialize("log.txt", {});
    // int value = Header_->getStatusCode();
    // minilog::log(minilog::Log, "Header getStatusCode: %d", value);
  }
  
private:
  std::shared_ptr<cgicc::HTTPResponseHeader> Header_;
  nlohmann::json ApiData_;
};

HttpResponse::HttpResponse() : impl(new HttpResponseImpl()) {}
HttpResponse::~HttpResponse() {}

void HttpResponse::setHeader(HttpStatus status, const std::string &reason) {
  impl->setHeader(status, reason);
}
void HttpResponse::setHeader(HttpStatus status) { impl->setHeader(status); }

void HttpResponse::addHeader(const std::string &Key, const std::string &Value) {
  impl->addHeader(Key, Value);
}

void HttpResponse::setCookie(const std::string &Name, const std::string &Value,
                             unsigned long MaxAge) {
  impl->setCookie(Name, Value, MaxAge);
}

void HttpResponse::setErrorResponse(HttpStatus Status,
                                    const std::string &Message) {
  impl->setErrorResponse(Status, Message);
}

void HttpResponse::setInterceptErrorResponse(HttpStatus Status,
                                    const std::string &Message) {
  impl->setInterceptErrorResponse(Status, Message);
}
void HttpResponse::setApiContent(const std::string &Kind) {
  impl->setApiContent(Kind);
}

void HttpResponse::appendApiItem(const std::string &Item) {
  impl->appendApiItem(Item);
}

void HttpResponse::appendApiItem(const nlohmann::json &Item) {
  assert(Item.is_object());
  impl->appendApiItem(Item);
}

void HttpResponse::appendApiItems(const nlohmann::json &Items) {
  assert(Items.is_array());
  impl->appendApiItems(Items);
}

void HttpResponse::setApiData(HttpStatus Status, const nlohmann::json &Data, const std::string &Message) {
  impl->setApiData(Status, Data, Message);
}

void HttpResponse::render(std::ostream &stream) { impl->render(stream); }

void HttpResponseHandler::setResponse(HttpResponse& Resp, HttpStatus Statuscode, RetCode ret_code,  const std::string& Message)
{
  switch (ret_code) {
    case RetCode::API_FAILURE:
      Resp.setErrorResponse(Statuscode, Message);
      break;
    case RetCode::API_INAVLID_ARGUMENT:
      Resp.setErrorResponse(Statuscode, Message);
      break;
    case RetCode::API_SQL_DB_FAILURE:
      Resp.setErrorResponse(Statuscode, Message);
      break;
    case RetCode::API_MEM_ALLOC_FAILURE:
      Resp.setErrorResponse(Statuscode, Message);
      break;
    case RetCode::API_MAX_LIMIT:
      Resp.setErrorResponse(Statuscode, Message);
      break;
    default:
      Resp.setErrorResponse(Statuscode, "Request Failed");
      break;
  }
}

} // namespace cgi
} // namespace rockchip
