#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// SMTP endpoint handler
// GET: Returns SMTP configuration
// POST/PUT: Updates SMTP configuration
api::Response HandleSmtp(const api::RequestContext& ctx);

// SMTP test endpoint handler
// POST: Tests SMTP connection and sends test email
api::Response HandleSmtpTest(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
