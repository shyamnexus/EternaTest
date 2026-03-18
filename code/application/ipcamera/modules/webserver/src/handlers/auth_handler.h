#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// Authentication endpoint handlers

// POST /api/v1/auth/login - Login with username/password
api::Response HandleAuthLogin(const api::RequestContext& ctx);

// POST /api/v1/auth/logout - Logout and invalidate token
api::Response HandleAuthLogout(const api::RequestContext& ctx);

// POST /api/v1/auth/change-password - Change own password
api::Response HandleAuthChangePassword(const api::RequestContext& ctx);

// POST /api/v1/auth/initial-setup - Complete initial setup (change password + set security questions)
api::Response HandleAuthInitialSetup(const api::RequestContext& ctx);

// POST /api/v1/auth/reset-password - Admin reset user password
api::Response HandleAuthResetPassword(const api::RequestContext& ctx);

// GET /api/v1/auth/verify - Verify token validity
api::Response HandleAuthVerify(const api::RequestContext& ctx);

// GET /api/v1/auth/sessions - List active sessions (admin only)
api::Response HandleAuthSessions(const api::RequestContext& ctx);

// ==================== Forgot Password / Security Questions ====================

// GET /api/v1/auth/security-questions/available - Get list of available security questions
api::Response HandleSecurityQuestionsAvailable(const api::RequestContext& ctx);

// POST /api/v1/auth/security-questions/setup - Set up security questions for current user
api::Response HandleSecurityQuestionsSetup(const api::RequestContext& ctx);

// GET /api/v1/auth/security-questions/status - Check if current user has set up security questions
api::Response HandleSecurityQuestionsStatus(const api::RequestContext& ctx);

// GET /api/v1/auth/forgot-password/questions - Get user's security questions (by username)
api::Response HandleForgotPasswordQuestions(const api::RequestContext& ctx);

// POST /api/v1/auth/forgot-password/verify - Verify security answers and get reset token
api::Response HandleForgotPasswordVerify(const api::RequestContext& ctx);

// POST /api/v1/auth/forgot-password/reset - Reset password with token
api::Response HandleForgotPasswordReset(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
