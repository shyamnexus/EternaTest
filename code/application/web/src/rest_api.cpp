// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rest_api.h"
#include <functional>
#include <sstream>
#include "jwt-cpp/jwt.h"
#include "audio_api.h"
#include "cgicc/Cgicc.h"
#include "common.h"
#include "event_api.h"
#include "image_api.h"
#include "network_api.h"
#include "osd_api.h"
#include "peripherals_api.h"
#include "roi_api.h"
#include "storage_api.h"
#include "stream_api.h"
#include "system_api.h"
#include "video_api.h"
#include "minilog.h"
#include "analytics_api.h"
#include "Logger.h"
#define ENABLE_TOKEN
#define TOKEN_LENGTH 704
#define   SESSION_ID_LENGTH         64
namespace rockchip {
namespace cgi {
inline namespace detail {


std::vector<QueryParam> parseParams(const std::string &QueryString) {
  const std::string ParamDelimiter = "&";
  const std::string ValueDelimiter = "=";
  std::vector<QueryParam> Params;
  QueryParam Param;
  size_t PosStart = 0;
  size_t PosEnd;
  size_t ValStart;
  size_t DelimLen = ParamDelimiter.length();
  std::string Token;

  while ((PosEnd = QueryString.find(ParamDelimiter, PosStart)) !=
         std::string::npos) {
    Token = QueryString.substr(PosStart, PosEnd - PosStart);
    PosStart = PosEnd + DelimLen;
    if ((ValStart = Token.find(ValueDelimiter, 0)) != std::string::npos) {
      Param.Key = Token.substr(0, ValStart);
      ValStart += ValueDelimiter.length();
      Param.Value = Token.substr(ValStart);
      Params.push_back(Param);
    }
  }
  Token = QueryString.substr(PosStart);
  if ((ValStart = Token.find(ValueDelimiter, 0)) != std::string::npos) {
    Param.Key = Token.substr(0, ValStart);
    ValStart += ValueDelimiter.length();
    Param.Value = Token.substr(ValStart);
    Params.push_back(Param);
  }

  return Params;
}

std::string parseBearerToken(const HttpRequest& request) {
    const std::string prefix = "Bearer ";
    if (request.AuthMethod.substr(0, prefix.size()) == prefix) {
        return request.AuthMethod.substr(prefix.size());
    }
    return "";
}
std::vector<unsigned char> hexStringToBytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = (unsigned char) strtol(byteString.c_str(), nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}
// #ifndef USE_RKIPC

int checkToken(HttpRequest &Req) {
  int ret = 0;
  // Logger& logger = Logger::getInstance();
  if (Req.PathInfo.compare("/system/login") != 0 && Req.PathInfo.compare("/system/login/forgetpwd") != 0 &&
    Req.PathInfo.find("system/para/") == std::string::npos) {
        // logger.log(Logger::INFO,"\n============checkToken AuthMethod is ============\n",Req.AuthMethod);
    // in addition to the /system/login and /system/para, all tokens are
    // verified
    if(Req.AuthMethod.empty())
    {
      ret = API_FORBIDDEN;
    }
    std::vector<unsigned char> encrypted_token;
const std::string prefix = "Bearer ";
if (Req.AuthMethod.find(prefix) == 0) {
    std::string token_str = Req.AuthMethod.substr(prefix.size());
    // Convert the hex string to bytes
    encrypted_token = hexStringToBytes(token_str);
    // Verify lengths
    // logger.log(Logger::INFO, "token_str length: " + std::to_string(token_str.size()));
    // logger.log(Logger::INFO, "encrypted_token length: " + std::to_string(encrypted_token.size()));
// Assuming the session ID is a fixed length, e.g., 32 characters
    // const size_t session_id_length = 64;
    //  const size_t token_length = 704;
    if (token_str.size() == TOKEN_LENGTH) {
        // Extract the session ID from the end of the token string
        std::string session_id = token_str.substr(token_str.size() -SESSION_ID_LENGTH );
        // Extract the actual encrypted token part
        std::string actual_encrypted_token_hex = token_str.substr(0, token_str.size() - SESSION_ID_LENGTH);
     // Convert the hex string back to bytes if needed
        std::vector<unsigned char> actual_encrypted_token = hexStringToBytes(actual_encrypted_token_hex);
        
        // Log the extracted session ID and actual encrypted token length
        // logger.log(Logger::INFO, "session_id: " + session_id);
        // logger.log(Logger::INFO, "actual_encrypted_token length: " + std::to_string(actual_encrypted_token.size()));
    ret = jwt_token_verify(actual_encrypted_token,session_id, Req);
    }
    else
    {
      // logger.log(Logger::ERROR, "Token string is too short to contain a valid session ID.");
      ret =  API_FORBIDDEN;
    }
    }
    else
    {
      ret =  API_FORBIDDEN;
    }
       
    
  }
  return ret;
}
// #endif
// #endif
void parseLogoutRequest(HttpRequest &Req,HttpResponse &Resp)
 {
  std::string path_api_resource;
  std::string path_specific_resource;
  std::string para_channel;
  nlohmann::json content = nlohmann::json({});

  /* Get Path Information */
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) 
  {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
    pos_first = path_specific_resource.find_first_of("/");
    if (pos_first != -1)
      para_channel = path_specific_resource.substr(
          pos_first + 1, path_specific_resource.size() + 1);
  }

  if (Req.Method == "DELETE") 
  {
  if (path_specific_resource.find("logout") != std::string::npos) 
     { 
        if (Req.Params.empty()) {
		      char sql_query[256] = {0};
            memset(sql_query, '\0', 1);
            snprintf(sql_query, sizeof(sql_query), "SELECT * FROM session_history WHERE session_id = '%s' LIMIT 1;", Req.SessionId.c_str());
            if (0 == hon_system_record_exists(sql_query)) {
               Resp.setApiData(HttpStatus::kBadRequest, content,"User Session data not found!!");
              Resp.setHeader(HttpStatus::kBadRequest, "User Session data not found!!");
              return;
            }
            snprintf(sql_query, sizeof(sql_query), "DELETE FROM session_history WHERE session_id = '%s';", Req.SessionId.c_str());
            if (0 == hon_system_manage_user_record(sql_query)) {
            
              Resp.setApiData(HttpStatus::kOk, content,"User logged out successfully");
              Resp.setHeader(HttpStatus::kOk, "OK");
              return;
            }
            else {
              Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for logout request!!!");
              Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for logout request!!!");
              return;
            }
      		}
          else
        {
          Resp.setApiData(HttpStatus::kBadRequest, content,"Bad Request Params");
            Resp.setHeader(HttpStatus::kBadRequest, "Bad Request Params");
        return;
        }
     }
  }
       Resp.setApiData(HttpStatus::kBadRequest, content,"Not Implemented");
       Resp.setHeader(HttpStatus::kBadRequest, "Not Implemented");
    
 }
HttpStatus parseRequest(HttpRequest &Req) {
  //  Logger& logger = Logger::getInstance();
    cgicc::CgiInput input;
  cgicc::Cgicc cgi;

  // logger.log(Logger::INFO,"\n============parseRequest environment variables are ============\n",input.getenv("HTTP_AUTHORIZATION"));
  Req.ScriptName = cgi.getEnvironment().getScriptName();
  Req.PathInfo = cgi.getEnvironment().getPathInfo();
  // Req.Cookies = cgi.getEnvironment().getCookies();
  assert(!Req.PathInfo.empty());

  int pos_first = Req.PathInfo.find_first_of("/");
  int pos_last = Req.PathInfo.find_last_of("/");
  if (pos_first != pos_last) {
    std::string path = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
    int pos_second = path.find_first_of("/");
    Req.Api = Req.PathInfo.substr(pos_first, pos_second + 1).c_str();
  } else {
    Req.Api = Req.PathInfo;
  }

  Req.AuthMethod = input.getenv("HTTP_AUTHORIZATION");
 Req.RemoteAddress = getenv("REMOTE_ADDR");
  Req.Method = cgi.getEnvironment().getRequestMethod();
  // logger.log(Logger::INFO,"\n============remote_addr ============\n",remote_addr);
        // logger.log(Logger::INFO,"\n============parseRequest AuthMethod is ============\n",Req.AuthMethod);
  if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    Req.Params = parseParams(cgi.getEnvironment().getQueryString());
    Req.ContentLength = cgi.getEnvironment().getContentLength();
    if (Req.ContentLength > 0) {
      Req.ContentType = cgi.getEnvironment().getContentType();
      if (std::string::npos != Req.ContentType.find("application/json")) {
        std::string PostData = cgi.getEnvironment().getPostData();
        Req.PostObject = nlohmann::json::parse(PostData);
      } else if (std::string::npos !=
                 Req.ContentType.find("multipart/form-data")) {
        Req.Files = cgi.getFiles();
      } else if (std::string::npos != Req.ContentType.find("text/plain")) {
        Req.PostData = cgi.getEnvironment().getPostData();
      } else {
        return HttpStatus::kBadRequest;
      }
    }
  } else if (Req.Method == "GET") {
    Req.Params = parseParams(cgi.getEnvironment().getQueryString());
  } else if (Req.Method == "DELETE") {
    Req.Params = parseParams(cgi.getEnvironment().getQueryString());
    
  } else {
    return HttpStatus::kBadRequest;
  }
  return HttpStatus::kOk;
}

} // namespace detail
 
void ApiEntry::run() {

  using namespace std::placeholders;

  // A workaround for unexpected log messages in linked libraries
  // TODO: Find a better way for this feature requirement
  // APPLICATION NOTES:
  //   1. Don't use std::cerr, it will sync C stderr
  //   2. Don't use fprintf(stderr/stdout...)/printf
  //   3. Alternative logging library in this application:
  //     #include <minilogger/log.h>

  std::ios::sync_with_stdio(false);

  HttpRequest Req;
  HttpResponse Resp;
  const std::string network_api = "network";
  const std::string network_ntp_api = "network-ntp";
  const std::string network_ddns_api = "network-ddns";
  const std::string network_pppoe_api = "network-pppoe";
  const std::string network_port_api = "network-port";
  const std::string storage_api = "storage";
  const std::string video_api = "video";
  const std::string audio_api = "audio";
  const std::string stream_url_api = "stream-url";
  const std::string image_api = "image";
  const std::string system_api = "system";
  const std::string osd_api = "osd";
  const std::string roi_api = "roi";
  const std::string event_api = "event";
  const std::string peripherals_api = "peripherals";
  const std::string analytics_api = "analytics";

  HandlerEntry e;

 
  int ret = 0;
  if (HttpStatus::kOk == parseRequest(Req)) {
    #ifdef ENABLE_TOKEN
    // ret = checkToken(Req);
    
    if((ret == 0) || (ret == 1))
    {
       e.Api = network_api;
        e.handler =
        std::bind(&NetworkApiHandler::handler, NetworkApiHandler(), _1, _2);
        Handlers.push_back(e);
        e.Api = network_ntp_api;
        e.handler =
        std::bind(&NetworkNTPApiHandler::handler, NetworkNTPApiHandler(), _1, _2);
        Handlers.push_back(e);
  #ifdef TEST
  
  e.Api = network_ddns_api;
  e.handler = std::bind(&NetworkDDNSApiHandler::handler,
                        NetworkDDNSApiHandler(), _1, _2);
  Handlers.push_back(e);
  e.Api = network_pppoe_api;
  e.handler = std::bind(&NetworkPPPoEApiHandler::handler,
                        NetworkPPPoEApiHandler(), _1, _2);
  Handlers.push_back(e);
  e.Api = network_port_api;
  e.handler = std::bind(&NetworkPortApiHandler::handler,
                        NetworkPortApiHandler(), _1, _2);
  Handlers.push_back(e);
  
  e.Api = roi_api;
  e.handler = std::bind(&ROIApiHandler::handler, ROIApiHandler(), _1, _2);
  Handlers.push_back(e);
 
  e.Api = peripherals_api;
  e.handler = std::bind(&PeripheralsApiHandler::handler, PeripheralsApiHandler(), _1, _2);
  Handlers.push_back(e);

#endif

 e.Api = event_api;
  e.handler = std::bind(&EventApiHandler::handler, EventApiHandler(), _1, _2);
  Handlers.push_back(e);
  e.Api = storage_api;
  e.handler = std::bind(&StorageApiHandler::handler, StorageApiHandler(), _1, _2);
  Handlers.push_back(e);
  e.Api = analytics_api;
  e.handler = std::bind(&AnalyticsApiHandler::handler, AnalyticsApiHandler(), _1, _2);
  Handlers.push_back(e);
  e.Api = audio_api;
  e.handler = std::bind(&AudioApiHandler::handler, AudioApiHandler(), _1, _2);
  Handlers.push_back(e);
  
  e.Api = video_api;
  e.handler = std::bind(&VideoApiHandler::handler, VideoApiHandler(), _1, _2);
  Handlers.push_back(e);
  e.Api = stream_url_api;
  e.handler = std::bind(&StreamURLApiHandler::handler, StreamURLApiHandler(), _1, _2);
  Handlers.push_back(e);
  e.Api = image_api;
  e.handler = std::bind(&ImageApiHandler::handler, ImageApiHandler(), _1, _2);
  Handlers.push_back(e);
  e.Api = osd_api;
  e.handler = std::bind(&OSDApiHandler::handler, OSDApiHandler(), _1, _2);
  Handlers.push_back(e);

    }
    if(ret== 0 )
    {
      e.Api = system_api;
      e.handler = std::bind(&SystemApiHandler::handler, SystemApiHandler(), _1, _2);
      Handlers.push_back(e);
      for (auto h : Handlers) {
      if (!Req.Api.compare(1, 20, h.Api, 0, 20)) {
        h.handler(Req, Resp);
      }
    }
    }
    else if(ret == 1)
    {
      if(Req.PathInfo.compare("/system/logout") == 0)
      {
        parseLogoutRequest(Req, Resp);
      }
      else
      {
        for (auto h : Handlers) {
          if (!Req.Api.compare(1, 20, h.Api, 0, 20)) {
            h.handler(Req, Resp);
          }
        }
      }
    }
    else if(ret == 2) {
      if(Req.PathInfo.compare("/system/logout") == 0)
      {
        parseLogoutRequest(Req,Resp);
      }
      else if(Req.PathInfo.compare("/stream-url") == 0)
      {
         e.Api = stream_url_api;
        e.handler = std::bind(&StreamURLApiHandler::handler, StreamURLApiHandler(), _1, _2);
        Handlers.push_back(e);
        for (auto h : Handlers) {
      if (!Req.Api.compare(1, 20, h.Api, 0, 20)) {
        h.handler(Req, Resp);
      }
    }
      }
      else
      {
        Resp.setApiData(HttpStatus::kForbidden, "","Permission Denied to access the configuration");
        Resp.setHeader(HttpStatus::kForbidden, "Permission Denied to access the configuration");
      }
  }
  else if(ret== API_FORBIDDEN)
  {
    Resp.setInterceptErrorResponse(HttpStatus::kForbidden, "Authorization token is missing or invalid.");
  }
  else if(ret == API_UNAUTHORIZED)
  {
    Resp.setInterceptErrorResponse(HttpStatus::kUnauthorized, "Token Expired!!");
  }
  else{
    Resp.setApiData(HttpStatus::kBadRequest, " ", "Internal Server Error parse request!!");
		Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for parse request!!");

  }
  } else {
    Resp.setApiData(HttpStatus::kBadRequest, "","Error parsing request!");
    Resp.setHeader(HttpStatus::kBadRequest,"Error parsing request!");
  }
#else
      e.Api = system_api;
      e.handler = std::bind(&SystemApiHandler::handler, SystemApiHandler(), _1, _2);
      Handlers.push_back(e);

       for (auto h : Handlers) {
      if (!Req.Api.compare(1, 20, h.Api, 0, 20)) {

        h.handler(Req, Resp);
      }
    }
  } else {
    Resp.setApiData(HttpStatus::kBadRequest, "","Error parsing request!");
    Resp.setHeader(HttpStatus::kBadRequest,"Error parsing request!");
  }
#endif
  std::ostringstream os;
  Resp.render(os);
  std::cout << os.str();
  
}

} // namespace cgi
} // namespace rockchip
