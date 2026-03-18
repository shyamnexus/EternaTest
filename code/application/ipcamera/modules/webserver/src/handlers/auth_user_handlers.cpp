#include "auth_handler.h"
#include "user_handler.h"
#include "../crypto/password_crypto.h"
#include <ipcam/user_manager.h>
#include <ipcam/config.h>
#include <ipcam/audit_logger.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cctype>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

namespace {
    // Global user manager instance
    std::shared_ptr<config::UserManager> g_user_manager;
    
    // Initialize user manager on first use
    void EnsureUserManagerInitialized() {
        if (!g_user_manager) {
            g_user_manager = std::make_shared<config::UserManager>();
            
            // Get database path from config (auth.json)
            std::string db_path = config::Get<std::string>("auth.database.path", "/userdata/db/users.db");
            
            auto result = g_user_manager->Initialize(db_path);
            if (!result.success) {
                spdlog::error("Failed to initialize user manager at {}: {}", db_path, result.error);
            } else {
                spdlog::info("User manager initialized with database: {}", db_path);
            }
        }
    }
    
    // Case-insensitive header lookup helper
    std::string GetHeaderCaseInsensitive(const api::RequestContext& ctx, const std::string& header_name) {
        std::string lower_name = header_name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        
        for (const auto& [key, value] : ctx.headers) {
            std::string lower_key = key;
            std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
            if (lower_key == lower_name) {
                return value;
            }
        }
        return "";
    }
    
    // Extract token from Authorization header
    std::string ExtractToken(const api::RequestContext& ctx) {
        std::string auth_header = GetHeaderCaseInsensitive(ctx, "Authorization");
        
        if (auth_header.empty()) {
            spdlog::debug("Authorization header not found");
            return "";
        }
        
        if (auth_header.find("Bearer ") == 0) {
            return auth_header.substr(7);
        }
        
        return auth_header;
    }
    
    // Get client IP address
    std::string GetClientIP(const api::RequestContext& ctx) {
        // Try X-Forwarded-For first
        std::string forwarded = GetHeaderCaseInsensitive(ctx, "X-Forwarded-For");
        if (!forwarded.empty()) {
            size_t comma = forwarded.find(',');
            if (comma != std::string::npos) {
                return forwarded.substr(0, comma);
            }
            return forwarded;
        }
        
        // Try X-Real-IP
        std::string real_ip = GetHeaderCaseInsensitive(ctx, "X-Real-IP");
        if (!real_ip.empty()) {
            return real_ip;
        }
        
        // Fallback (would need to be extracted from connection info)
        return "unknown";
    }
    
    // Validate token and return session
    bool ValidateToken(const std::string& token, config::Session& session, api::Response& error_resp) {
        if (token.empty()) {
            error_resp.status_code = 401;
            error_resp.body = json({
                {"status", "error"},
                {"message", "Authentication required"}
            }).dump();
            return false;
        }
        
        auto result = g_user_manager->ValidateToken(token);
        if (!result.success) {
            error_resp.status_code = 401;
            error_resp.body = json({
                {"status", "error"},
                {"message", "Invalid or expired token"}
            }).dump();
            return false;
        }
        
        session = result.value;
        return true;
    }
    
    // Require admin role
    bool RequireAdmin(const config::Session& session, api::Response& error_resp) {
        if (session.role != config::UserRole::ADMIN) {
            error_resp.status_code = 403;
            error_resp.body = json({
                {"status", "error"},
                {"message", "Admin access required"}
            }).dump();
            return false;
        }
        return true;
    }
    
    /**
     * @brief Extract password from request body
     * 
     * Handles encrypted passwords with replay protection:
     * - Encrypted: { "password": { "encrypted_key": "...", "encrypted_data": "...", "iv": "...", "key_id": "...", "timestamp": 1234567890 } }
     * 
     * @param password_field The JSON value of the "password" field
     * @param decrypted_password Output parameter for the decrypted password
     * @return true if successful, false otherwise
     */
    bool ExtractPassword(const json& password_field, std::string& decrypted_password) {
        // Check if password is an encrypted object
        if (password_field.is_object()) {
            // Encrypted password format
            if (!password_field.contains("encrypted_key") ||
                !password_field.contains("encrypted_data") ||
                !password_field.contains("iv") ||
                !password_field.contains("key_id")) {
                spdlog::warn("Encrypted password object missing required fields");
                return false;
            }
            
            std::string encrypted_key = password_field["encrypted_key"];
            std::string encrypted_data = password_field["encrypted_data"];
            std::string iv = password_field["iv"];
            std::string key_id = password_field["key_id"];
            
            // Extract timestamp for replay protection (optional but recommended)
            int64_t timestamp = 0;
            if (password_field.contains("timestamp")) {
                timestamp = password_field["timestamp"].get<int64_t>();
            }
            
            // Extract GCM auth tag (required — AES-256-GCM only, no CBC fallback)
            if (!password_field.contains("auth_tag")) {
                spdlog::warn("Encrypted password missing required auth_tag field (AES-256-GCM required)");
                return false;
            }
            std::string auth_tag = password_field["auth_tag"].get<std::string>();
            
            auto& crypto = crypto::PasswordCrypto::Instance();
            decrypted_password = crypto.DecryptPassword(encrypted_key, encrypted_data, iv, key_id, timestamp, auth_tag);
            
            if (decrypted_password.empty()) {
                spdlog::warn("Failed to decrypt password");
                return false;
            }
            
            spdlog::debug("Successfully decrypted password from encrypted format");
            return true;
        }
        
        spdlog::warn("Invalid password format: expected encrypted object");
        return false;
    }
}

// ==================== Authentication Handlers ====================

api::Response HandleAuthLogin(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        auto body = json::parse(ctx.body);
        
        if (!body.contains("username") || !body.contains("password")) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing username or password"}
            }).dump();
            return resp;
        }
        
        std::string username = body["username"];
        std::string password;
        
        // Extract password (handles both encrypted and plain text)
        if (!ExtractPassword(body["password"], password)) {
            // Security audit: log failed password decryption as a security event
            std::string ip_address = GetClientIP(ctx);
            AuditLogger::Instance().LogLoginFailure(username, ip_address, "invalid_password_format");
            
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid password format or decryption failed"}
            }).dump();
            return resp;
        }
        
        std::string ip_address = GetClientIP(ctx);
        std::string user_agent = ctx.headers.count("User-Agent") ? ctx.headers.at("User-Agent") : "";
        
        auto login_result = g_user_manager->Login(username, password, ip_address, user_agent);
        
        if (!login_result.success) {
            // Security audit: log login failure
            auto& audit = AuditLogger::Instance();
            if (login_result.error.find("locked") != std::string::npos) {
                audit.LogAccountLocked(username, ip_address, 5, 15);
            } else {
                audit.LogLoginFailure(username, ip_address, login_result.error);
            }
            resp.status_code = 401;
            resp.body = json({
                {"status", "error"},
                {"message", login_result.error}
            }).dump();
            return resp;
        }
        
        // Security audit: log successful login
        AuditLogger::Instance().LogLoginSuccess(username, ip_address, user_agent);
        
        config::Session session = login_result.value;
        
        // Get user details to include password_must_change and security questions status
        auto user_result = g_user_manager->GetUserByUsername(session.username);
        bool must_change_password = false;
        bool security_questions_set = false;
        if (user_result.success) {
            must_change_password = user_result.value.password_must_change;
            security_questions_set = g_user_manager->HasSecurityQuestionsSet(session.user_id);
        }
        
        // Determine if initial setup is required
        bool requires_initial_setup = must_change_password || !security_questions_set;
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "Login successful"},
            {"token", session.token},
            {"user", {
                {"id", session.user_id},
                {"username", session.username},
                {"role", config::UserRoleToString(session.role)},
                {"password_must_change", must_change_password},
                {"security_questions_set", security_questions_set},
                {"requires_initial_setup", requires_initial_setup}
            }},
            {"expires_at", session.expires_at}
        }).dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({{"status", "error"}, {"message", "Invalid JSON"}}).dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in login: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleAuthLogout(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        
        if (token.empty()) {
            resp.status_code = 401;
            resp.body = json({{"status", "error"}, {"message", "No token provided"}}).dump();
            return resp;
        }
        
        auto result = g_user_manager->Logout(token);
        
        if (!result.success) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", result.error}}).dump();
            return resp;
        }
        
        // Security audit: log logout
        {
            // Try to get username from the token before it's invalidated
            auto validate = g_user_manager->ValidateToken(token);
            std::string uname = validate.success ? validate.value.username : "unknown";
            AuditLogger::Instance().LogLogout(uname, GetClientIP(ctx));
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "Logged out successfully"}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error in logout: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleAuthChangePassword(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        auto body = json::parse(ctx.body);
        
        if (!body.contains("old_password") || !body.contains("new_password")) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing old_password or new_password"}
            }).dump();
            return resp;
        }
        
        std::string old_password;
        std::string new_password;
        
        // Extract passwords (handles both encrypted and plain text)
        if (!ExtractPassword(body["old_password"], old_password)) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid old_password format or decryption failed"}
            }).dump();
            return resp;
        }
        
        if (!ExtractPassword(body["new_password"], new_password)) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid new_password format or decryption failed"}
            }).dump();
            return resp;
        }
        
        if (new_password.length() < 6) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Password must be at least 6 characters"}
            }).dump();
            return resp;
        }
        
        auto result = g_user_manager->ChangePassword(session.user_id, old_password, new_password);
        
        if (!result.success) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", result.error}}).dump();
            return resp;
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "Password changed successfully"}
        }).dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({{"status", "error"}, {"message", "Invalid JSON"}}).dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in change password: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleAuthInitialSetup(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        auto body = json::parse(ctx.body);
        
        // Validate required fields
        if (!body.contains("old_password") || !body.contains("new_password")) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing old_password or new_password"}
            }).dump();
            return resp;
        }
        
        if (!body.contains("security_questions") || !body["security_questions"].is_array()) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing security_questions array"}
            }).dump();
            return resp;
        }
        
        std::string old_password;
        std::string new_password;
        
        // Extract passwords (handles both encrypted and plain text)
        if (!ExtractPassword(body["old_password"], old_password)) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid old_password format or decryption failed"}
            }).dump();
            return resp;
        }
        
        if (!ExtractPassword(body["new_password"], new_password)) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid new_password format or decryption failed"}
            }).dump();
            return resp;
        }
        
        // Validate new password
        if (new_password.length() < 6) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Password must be at least 6 characters"}
            }).dump();
            return resp;
        }
        
        // Parse security questions
        std::vector<std::pair<int, std::string>> qa_pairs;
        for (const auto& item : body["security_questions"]) {
            if (!item.contains("question_id") || !item.contains("answer")) {
                resp.status_code = 400;
                resp.body = json({
                    {"status", "error"},
                    {"message", "Each security question must have 'question_id' and 'answer'"}
                }).dump();
                return resp;
            }
            
            int question_id = item["question_id"];
            std::string answer = item["answer"];
            
            if (answer.length() < 2) {
                resp.status_code = 400;
                resp.body = json({
                    {"status", "error"},
                    {"message", "Security question answers must be at least 2 characters"}
                }).dump();
                return resp;
            }
            
            qa_pairs.push_back({question_id, answer});
        }
        
        // Validate security questions count (1-5 required)
        if (qa_pairs.empty()) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "At least 1 security question is required"}
            }).dump();
            return resp;
        }
        
        if (qa_pairs.size() > 5) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Maximum 5 security questions allowed"}
            }).dump();
            return resp;
        }
        
        // Step 1: Change password
        auto password_result = g_user_manager->ChangePassword(session.user_id, old_password, new_password);
        if (!password_result.success) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", password_result.error}
            }).dump();
            return resp;
        }
        
        // Step 2: Set security questions
        auto sq_result = g_user_manager->SetSecurityQuestions(session.user_id, qa_pairs);
        if (!sq_result.success) {
            // Password was changed but security questions failed
            // We should still inform the user
            resp.status_code = 200;
            resp.body = json({
                {"status", "partial_success"},
                {"message", "Password changed but failed to set security questions: " + sq_result.error},
                {"password_changed", true},
                {"security_questions_set", false}
            }).dump();
            return resp;
        }
        
        spdlog::info("Initial setup completed for user ID {}", session.user_id);
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "Initial setup completed successfully"},
            {"password_changed", true},
            {"security_questions_set", true}
        }).dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({{"status", "error"}, {"message", "Invalid JSON"}}).dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in initial setup: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleAuthResetPassword(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        if (!RequireAdmin(session, resp)) {
            return resp;
        }
        
        auto body = json::parse(ctx.body);
        
        if (!body.contains("user_id") || !body.contains("new_password")) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing user_id or new_password"}
            }).dump();
            return resp;
        }
        
        int64_t user_id = body["user_id"];
        std::string new_password;
        
        // Extract password (encrypted only)
        if (!ExtractPassword(body["new_password"], new_password)) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid new_password format or decryption failed"}
            }).dump();
            return resp;
        }
        
        if (new_password.length() < 6) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Password must be at least 6 characters"}
            }).dump();
            return resp;
        }
        
        auto result = g_user_manager->ResetPassword(user_id, new_password);
        
        if (!result.success) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", result.error}}).dump();
            return resp;
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "Password reset successfully"}
        }).dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({{"status", "error"}, {"message", "Invalid JSON"}}).dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in reset password: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleAuthVerify(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "GET") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        
        if (token.empty()) {
            resp.status_code = 401;
            resp.body = json({
                {"status", "error"},
                {"message", "No token provided"},
                {"valid", false}
            }).dump();
            return resp;
        }
        
        auto result = g_user_manager->ValidateToken(token);
        
        if (!result.success) {
            resp.status_code = 401;
            resp.body = json({
                {"status", "error"},
                {"message", result.error},
                {"valid", false}
            }).dump();
            return resp;
        }
        
        config::Session session = result.value;
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"valid", true},
            {"user", {
                {"id", session.user_id},
                {"username", session.username},
                {"role", config::UserRoleToString(session.role)}
            }},
            {"expires_at", session.expires_at}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error in verify token: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleAuthSessions(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "GET") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        if (!RequireAdmin(session, resp)) {
            return resp;
        }
        
        auto sessions_result = g_user_manager->ListActiveSessions();
        
        if (!sessions_result.success) {
            resp.status_code = 500;
            resp.body = json({{"status", "error"}, {"message", sessions_result.error}}).dump();
            return resp;
        }
        
        auto sessions = sessions_result.value;
        json sessions_json = json::array();
        
        for (const auto& s : sessions) {
            sessions_json.push_back({
                {"token", s.token.substr(0, 16) + "..."},
                {"user_id", s.user_id},
                {"username", s.username},
                {"role", config::UserRoleToString(s.role)},
                {"ip_address", s.ip_address},
                {"created_at", s.created_at},
                {"expires_at", s.expires_at}
            });
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"sessions", sessions_json}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error in list sessions: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

// ==================== User Management Handlers ====================

api::Response HandleUsersList(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "GET") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        if (!RequireAdmin(session, resp)) {
            return resp;
        }
        
        auto users_result = g_user_manager->ListUsers();
        
        if (!users_result.success) {
            resp.status_code = 500;
            resp.body = json({{"status", "error"}, {"message", users_result.error}}).dump();
            return resp;
        }
        
        auto users = users_result.value;
        json users_json = json::array();
        
        for (const auto& user : users) {
            users_json.push_back({
                {"id", user.id},
                {"username", user.username},
                {"role", config::UserRoleToString(user.role)},
                {"enabled", user.enabled},
                {"created_at", user.created_at},
                {"last_login", user.last_login},
                {"last_login_ip", user.last_login_ip}
            });
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"users", users_json}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error in list users: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleUsersCurrent(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "GET") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        auto user_result = g_user_manager->GetUser(session.user_id);
        
        if (!user_result.success) {
            resp.status_code = 404;
            resp.body = json({{"status", "error"}, {"message", "User not found"}}).dump();
            return resp;
        }
        
        config::User user = user_result.value;
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"user", {
                {"id", user.id},
                {"username", user.username},
                {"role", config::UserRoleToString(user.role)},
                {"enabled", user.enabled},
                {"created_at", user.created_at},
                {"last_login", user.last_login},
                {"last_login_ip", user.last_login_ip}
            }}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error in get current user: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleUsersCreate(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        if (!RequireAdmin(session, resp)) {
            return resp;
        }
        
        auto body = json::parse(ctx.body);
        
        if (!body.contains("username") || !body.contains("password") || !body.contains("role")) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing required fields: username, password, role"}
            }).dump();
            return resp;
        }
        
        std::string username = body["username"];
        std::string password;
        std::string role_str = body["role"];
        bool enabled = body.value("enabled", true);
        
        // Extract password (supports both encrypted object and plaintext string)
        if (body["password"].is_object()) {
            if (!ExtractPassword(body["password"], password)) {
                resp.status_code = 400;
                resp.body = json({
                    {"status", "error"},
                    {"message", "Invalid password format or decryption failed"}
                }).dump();
                return resp;
            }
        } else if (body["password"].is_string()) {
            password = body["password"].get<std::string>();
        } else {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid password format"}
            }).dump();
            return resp;
        }
        
        if (password.length() < 6) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Password must be at least 6 characters"}
            }).dump();
            return resp;
        }
        
        auto role_opt = config::StringToUserRole(role_str);
        if (!role_opt.has_value()) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid role. Must be: admin, operator, or viewer"}
            }).dump();
            return resp;
        }
        
        auto result = g_user_manager->CreateUser(username, password, *role_opt, enabled);
        
        if (!result.success) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", result.error}}).dump();
            return resp;
        }
        
        int64_t user_id = result.value;
        
        resp.status_code = 201;
        resp.body = json({
            {"status", "success"},
            {"message", "User created successfully"},
            {"user_id", user_id}
        }).dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({{"status", "error"}, {"message", "Invalid JSON"}}).dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in create user: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleUsersGet(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "GET") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        if (!RequireAdmin(session, resp)) {
            return resp;
        }
        
        // Extract user ID from URL path
        std::string path = ctx.uri;
        size_t last_slash = path.find_last_of('/');
        if (last_slash == std::string::npos) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", "Invalid user ID"}}).dump();
            return resp;
        }
        
        int64_t user_id = std::stoll(path.substr(last_slash + 1));
        
        auto user_result = g_user_manager->GetUser(user_id);
        
        if (!user_result.success) {
            resp.status_code = 404;
            resp.body = json({{"status", "error"}, {"message", user_result.error}}).dump();
            return resp;
        }
        
        config::User user = user_result.value;
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"user", {
                {"id", user.id},
                {"username", user.username},
                {"role", config::UserRoleToString(user.role)},
                {"enabled", user.enabled},
                {"created_at", user.created_at},
                {"last_login", user.last_login},
                {"last_login_ip", user.last_login_ip}
            }}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error in get user: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleUsersUpdate(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "PUT") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        if (!RequireAdmin(session, resp)) {
            return resp;
        }
        
        // Extract user ID from URL
        std::string path = ctx.uri;
        size_t last_slash = path.find_last_of('/');
        if (last_slash == std::string::npos) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", "Invalid user ID"}}).dump();
            return resp;
        }
        
        int64_t user_id = std::stoll(path.substr(last_slash + 1));
        
        auto body = json::parse(ctx.body);
        
        std::optional<std::string> username;
        std::optional<config::UserRole> role;
        std::optional<bool> enabled;
        
        if (body.contains("username")) {
            username = body["username"].get<std::string>();
        }
        
        if (body.contains("role")) {
            std::string role_str = body["role"];
            auto role_opt = config::StringToUserRole(role_str);
            if (!role_opt.has_value()) {
                resp.status_code = 400;
                resp.body = json({
                    {"status", "error"},
                    {"message", "Invalid role. Must be: admin, operator, or viewer"}
                }).dump();
                return resp;
            }
            role = *role_opt;
        }
        
        if (body.contains("enabled")) {
            enabled = body["enabled"].get<bool>();
        }
        
        auto result = g_user_manager->UpdateUser(user_id, username, role, enabled);
        
        if (!result.success) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", result.error}}).dump();
            return resp;
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "User updated successfully"}
        }).dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({{"status", "error"}, {"message", "Invalid JSON"}}).dump();
    } catch (const std::exception& e) {
        spdlog::error("Error in update user: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleUsersDelete(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "DELETE") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        if (!RequireAdmin(session, resp)) {
            return resp;
        }
        
        // Extract user ID from URL
        std::string path = ctx.uri;
        size_t last_slash = path.find_last_of('/');
        if (last_slash == std::string::npos) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", "Invalid user ID"}}).dump();
            return resp;
        }
        
        int64_t user_id = std::stoll(path.substr(last_slash + 1));
        
        // Prevent self-deletion
        if (user_id == session.user_id) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Cannot delete your own account"}
            }).dump();
            return resp;
        }
        
        auto result = g_user_manager->DeleteUser(user_id);
        
        if (!result.success) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", result.error}}).dump();
            return resp;
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "User deleted successfully"}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error in delete user: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

// ==================== Unified User Router ====================
// Dispatches /api/v1/users/* requests based on method and URL structure
api::Response HandleUsersRouter(const api::RequestContext& ctx) {
    std::string path = ctx.uri;
    
    // Remove trailing slash if present
    if (!path.empty() && path.back() == '/') {
        path.pop_back();
    }
    
    // Check if the path has a user ID segment after /api/v1/users/
    // e.g., /api/v1/users/5
    const std::string base_path = "/api/v1/users";
    std::string remainder;
    if (path.length() > base_path.length()) {
        remainder = path.substr(base_path.length() + 1); // skip the '/'
    }
    
    bool has_id = !remainder.empty();
    
    // Check if remainder is a numeric ID (not "current" or "create")
    bool is_numeric_id = has_id && !remainder.empty() && 
                         std::all_of(remainder.begin(), remainder.end(), ::isdigit);
    
    if (ctx.method == "GET") {
        if (is_numeric_id) {
            return HandleUsersGet(ctx);
        } else if (!has_id) {
            return HandleUsersList(ctx);
        }
    } else if (ctx.method == "POST" && !has_id) {
        return HandleUsersCreate(ctx);
    } else if (ctx.method == "PUT" && is_numeric_id) {
        return HandleUsersUpdate(ctx);
    } else if (ctx.method == "DELETE" && is_numeric_id) {
        return HandleUsersDelete(ctx);
    }
    
    // Method not allowed or invalid path
    api::Response resp;
    resp.status_code = 405;
    resp.body = json({
        {"status", "error"},
        {"message", "Method not allowed or invalid path"}
    }).dump();
    return resp;
}

// ==================== Security Questions & Forgot Password Handlers ====================

api::Response HandleSecurityQuestionsAvailable(const api::RequestContext& ctx) {
    api::Response resp;
    
    try {
        if (ctx.method != "GET") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        auto questions = config::UserManager::GetAvailableSecurityQuestions();
        
        json questions_json = json::array();
        for (const auto& q : questions) {
            questions_json.push_back({
                {"id", q.id},
                {"question", q.question}
            });
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"questions", questions_json},
            {"min_required", 1},
            {"max_allowed", 5}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error getting available security questions: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleSecurityQuestionsSetup(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        auto body = json::parse(ctx.body);
        
        if (!body.contains("questions") || !body["questions"].is_array()) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing 'questions' array. Format: [{\"question_id\": 1, \"answer\": \"...\"}]"}
            }).dump();
            return resp;
        }
        
        std::vector<std::pair<int, std::string>> qa_pairs;
        
        for (const auto& item : body["questions"]) {
            if (!item.contains("question_id") || !item.contains("answer")) {
                resp.status_code = 400;
                resp.body = json({
                    {"status", "error"},
                    {"message", "Each question must have 'question_id' and 'answer'"}
                }).dump();
                return resp;
            }
            
            int question_id = item["question_id"];
            std::string answer = item["answer"];
            
            qa_pairs.push_back({question_id, answer});
        }
        
        auto result = g_user_manager->SetSecurityQuestions(session.user_id, qa_pairs);
        
        if (!result.success) {
            resp.status_code = 400;
            resp.body = json({{"status", "error"}, {"message", result.error}}).dump();
            return resp;
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "Security questions set successfully"}
        }).dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({{"status", "error"}, {"message", "Invalid JSON"}}).dump();
    } catch (const std::exception& e) {
        spdlog::error("Error setting up security questions: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleSecurityQuestionsStatus(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "GET") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        std::string token = ExtractToken(ctx);
        config::Session session;
        
        if (!ValidateToken(token, session, resp)) {
            return resp;
        }
        
        bool has_questions = g_user_manager->HasSecurityQuestionsSet(session.user_id);
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"security_questions_set", has_questions}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error checking security questions status: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleForgotPasswordQuestions(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "GET") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        // Extract username from query params
        std::string username;
        auto it = ctx.query_params.find("username");
        if (it != ctx.query_params.end()) {
            username = it->second;
        }
        
        if (username.empty()) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing 'username' query parameter"}
            }).dump();
            return resp;
        }
        
        auto result = g_user_manager->GetUserSecurityQuestions(username);
        
        if (!result.success) {
            // Don't reveal if user exists or not
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Unable to retrieve security questions. User may not exist or questions not set."}
            }).dump();
            return resp;
        }
        
        json questions_json = json::array();
        for (const auto& q : result.value) {
            questions_json.push_back({
                {"id", q.id},
                {"question", q.question}
            });
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"username", username},
            {"questions", questions_json},
            {"total_questions", static_cast<int>(result.value.size())},
            {"verification_note", "You may answer one or more questions. All provided answers must be correct."}
        }).dump();
        
    } catch (const std::exception& e) {
        spdlog::error("Error getting forgot password questions: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleForgotPasswordVerify(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        auto body = json::parse(ctx.body);
        
        if (!body.contains("username")) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing 'username'"}
            }).dump();
            return resp;
        }
        
        std::string username = body["username"];
        std::vector<std::pair<int, std::string>> answers;
        
        // Support both single answer format and array format
        if (body.contains("question_id") && body.contains("answer")) {
            // Single answer format: { "username": "...", "question_id": 1, "answer": "..." }
            int question_id = body["question_id"];
            std::string answer = body["answer"];
            answers.push_back({question_id, answer});
        }
        else if (body.contains("answers") && body["answers"].is_array()) {
            // Array format: { "username": "...", "answers": [{"question_id": 1, "answer": "..."}] }
            for (const auto& item : body["answers"]) {
                if (!item.contains("question_id") || !item.contains("answer")) {
                    resp.status_code = 400;
                    resp.body = json({
                        {"status", "error"},
                        {"message", "Each answer must have 'question_id' and 'answer'"}
                    }).dump();
                    return resp;
                }
                
                int question_id = item["question_id"];
                std::string answer = item["answer"];
                
                answers.push_back({question_id, answer});
            }
        }
        else {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing 'question_id'+'answer' or 'answers' array"}
            }).dump();
            return resp;
        }
        
        if (answers.empty()) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "At least one answer is required"}
            }).dump();
            return resp;
        }
        
        auto result = g_user_manager->VerifySecurityAnswers(username, answers);
        
        if (!result.success) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", result.error}
            }).dump();
            return resp;
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "Security answers verified. Use the reset_token to set a new password."},
            {"reset_token", result.value},
            {"expires_in_seconds", 900}  // 15 minutes
        }).dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({{"status", "error"}, {"message", "Invalid JSON"}}).dump();
    } catch (const std::exception& e) {
        spdlog::error("Error verifying security answers: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

api::Response HandleForgotPasswordReset(const api::RequestContext& ctx) {
    EnsureUserManagerInitialized();
    api::Response resp;
    
    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({{"status", "error"}, {"message", "Method not allowed"}}).dump();
            return resp;
        }
        
        auto body = json::parse(ctx.body);
        
        if (!body.contains("reset_token") || !body.contains("new_password")) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Missing 'reset_token' or 'new_password'"}
            }).dump();
            return resp;
        }
        
        std::string reset_token = body["reset_token"];
        std::string new_password;
        
        // Extract password (handles both encrypted and plain text)
        if (!ExtractPassword(body["new_password"], new_password)) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid new_password format or decryption failed"}
            }).dump();
            return resp;
        }
        
        if (new_password.length() < 6) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Password must be at least 6 characters"}
            }).dump();
            return resp;
        }
        
        auto result = g_user_manager->ResetPasswordWithToken(reset_token, new_password);
        
        if (!result.success) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", result.error}
            }).dump();
            return resp;
        }
        
        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "Password reset successfully. You can now login with your new password."}
        }).dump();
        
    } catch (const json::exception& e) {
        resp.status_code = 400;
        resp.body = json({{"status", "error"}, {"message", "Invalid JSON"}}).dump();
    } catch (const std::exception& e) {
        spdlog::error("Error resetting password with token: {}", e.what());
        resp.status_code = 500;
        resp.body = json({{"status", "error"}, {"message", "Internal server error"}}).dump();
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
