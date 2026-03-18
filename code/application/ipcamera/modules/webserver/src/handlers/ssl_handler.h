#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// SSL Certificate endpoint handler
// GET: Returns current SSL certificate information
// POST: Upload custom SSL certificate and private key
api::Response HandleSsl(const api::RequestContext& ctx);

// SSL Certificate upload endpoint
// POST: Upload certificate and key files as multipart/form-data or JSON
api::Response HandleSslUpload(const api::RequestContext& ctx);

// SSL Certificate generation endpoint
// POST: Generate new self-signed certificate
api::Response HandleSslGenerate(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
