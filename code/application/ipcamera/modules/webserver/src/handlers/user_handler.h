#pragma once
#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// User management endpoint handlers

// GET /api/v1/users - List all users (admin only)
api::Response HandleUsersList(const api::RequestContext& ctx);

// GET /api/v1/users/current - Get current user info
api::Response HandleUsersCurrent(const api::RequestContext& ctx);

// GET /api/v1/users/:id - Get user by ID (admin only)
api::Response HandleUsersGet(const api::RequestContext& ctx);

// POST /api/v1/users - Create new user (admin only)
api::Response HandleUsersCreate(const api::RequestContext& ctx);

// PUT /api/v1/users/:id - Update user (admin only)
api::Response HandleUsersUpdate(const api::RequestContext& ctx);

// DELETE /api/v1/users/:id - Delete user (admin only)
api::Response HandleUsersDelete(const api::RequestContext& ctx);

// Unified router for /api/v1/users/* that dispatches based on method and URL
// Handles: GET (list or get by ID), POST (create), PUT (update), DELETE (delete)
api::Response HandleUsersRouter(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
