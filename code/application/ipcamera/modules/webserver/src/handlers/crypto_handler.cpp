/**
 * @file crypto_handler.cpp
 * @brief Implementation of crypto HTTP handlers
 */

#include "crypto_handler.h"
#include "../crypto/password_crypto.h"

#include <ipcam/user_manager.h>
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cctype>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

namespace {
    // Shared user manager instance for crypto handler auth checks
    std::shared_ptr<config::UserManager> g_user_manager;
    
    void EnsureUserManagerInitialized() {
        if (!g_user_manager) {
            g_user_manager = std::make_shared<config::UserManager>();
            std::string db_path = config::Get<std::string>("auth.database.path", "/mnt/app/ipcamera/db/users.db");
            auto result = g_user_manager->Initialize(db_path);
            if (!result.success) {
                spdlog::error("crypto_handler: Failed to initialize user manager at {}: {}", db_path, result.error);
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
    std::string ExtractTokenFromHeader(const api::RequestContext& ctx) {
        std::string auth_header = GetHeaderCaseInsensitive(ctx, "Authorization");
        
        if (auth_header.empty()) {
            return "";
        }
        
        if (auth_header.find("Bearer ") == 0) {
            return auth_header.substr(7);
        }
        
        return auth_header;
    }
}

api::Response HandleGetPublicKey(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method != "GET") {
            resp.status_code = 405;
            resp.body = json({
                {"status", "error"},
                {"message", "Method not allowed"}
            }).dump();
            return resp;
        }

        auto& crypto = crypto::PasswordCrypto::Instance();

        // Initialize if not already done
        if (!crypto.IsInitialized()) {
            if (!crypto.Initialize()) {
                resp.status_code = 500;
                resp.body = json({
                    {"status", "error"},
                    {"message", "Crypto system initialization failed"}
                }).dump();
                return resp;
            }
        }

        std::string public_key = crypto.GetPublicKeyPEM();
        std::string key_id = crypto.GetKeyId();

        if (public_key.empty() || key_id.empty()) {
            resp.status_code = 500;
            resp.body = json({
                {"status", "error"},
                {"message", "Failed to retrieve public key"}
            }).dump();
            return resp;
        }

        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"public_key", public_key},
            {"key_id", key_id},
            {"algorithm", crypto.GetRSAAlgorithm()},
            {"aes_algorithm", crypto.GetAESAlgorithm()},
            {"rsa_key_size", crypto.GetRSAKeySize()}
        }).dump();

    } catch (const std::exception& e) {
        spdlog::error("Error in HandleGetPublicKey: {}", e.what());
        resp.status_code = 500;
        resp.body = json({
            {"status", "error"},
            {"message", "Internal server error"}
        }).dump();
    }

    return resp;
}

api::Response HandleRotateKeys(const api::RequestContext& ctx) {
    api::Response resp;

    try {
        if (ctx.method != "POST") {
            resp.status_code = 405;
            resp.body = json({
                {"status", "error"},
                {"message", "Method not allowed"}
            }).dump();
            return resp;
        }

        // Authentication check - require admin
        std::string token = ExtractTokenFromHeader(ctx);
        if (token.empty()) {
            resp.status_code = 401;
            resp.body = json({
                {"status", "error"},
                {"message", "Authentication required"}
            }).dump();
            return resp;
        }
        
        // Validate token using shared g_user_manager
        EnsureUserManagerInitialized();
        if (!g_user_manager) {
            resp.status_code = 500;
            resp.body = json({
                {"status", "error"},
                {"message", "Internal server error"}
            }).dump();
            return resp;
        }
        
        auto token_result = g_user_manager->ValidateToken(token);
        if (!token_result.success) {
            resp.status_code = 401;
            resp.body = json({
                {"status", "error"},
                {"message", "Invalid or expired token"}
            }).dump();
            return resp;
        }
        
        config::Session session = token_result.value;
        if (session.role != config::UserRole::ADMIN) {
            resp.status_code = 403;
            resp.body = json({
                {"status", "error"},
                {"message", "Admin access required"}
            }).dump();
            return resp;
        }

        auto& crypto = crypto::PasswordCrypto::Instance();

        if (!crypto.IsInitialized()) {
            resp.status_code = 400;
            resp.body = json({
                {"status", "error"},
                {"message", "Crypto system not initialized"}
            }).dump();
            return resp;
        }

        if (!crypto.RotateKeys()) {
            resp.status_code = 500;
            resp.body = json({
                {"status", "error"},
                {"message", "Failed to rotate keys"}
            }).dump();
            return resp;
        }

        resp.status_code = 200;
        resp.body = json({
            {"status", "success"},
            {"message", "Keys rotated successfully"},
            {"new_key_id", crypto.GetKeyId()}
        }).dump();

        spdlog::info("RSA keys rotated by admin user '{}'", session.username);

    } catch (const std::exception& e) {
        spdlog::error("Error in HandleRotateKeys: {}", e.what());
        resp.status_code = 500;
        resp.body = json({
            {"status", "error"},
            {"message", "Internal server error"}
        }).dump();
    }

    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
