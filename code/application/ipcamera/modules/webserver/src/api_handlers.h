#pragma once
#include <string>
#include <functional>
#include <map>
#include "CivetServer.h"

namespace ipcam {
namespace webserver {
namespace api {

// API request context
struct RequestContext {
  const CivetServer* server;
  struct mg_connection* conn;
  std::string method;
  std::string uri;
  std::map<std::string, std::string> query_params;
  std::map<std::string, std::string> headers;
  std::string body;
};

// API response
struct Response {
  int status_code = 200;
  std::string content_type = "application/json";
  std::string body;
  std::map<std::string, std::string> headers;
  // For file streaming responses (leave empty for body-based responses)
  std::string file_path;  // If set, stream file directly instead of using body
  size_t file_size = 0;   // File size for Content-Length header
};

// Handler function type
using Handler = std::function<Response(const RequestContext&)>;

// Register API handlers with the server
void RegisterHandlers(CivetServer* server);

} // namespace api
} // namespace webserver
} // namespace ipcam
