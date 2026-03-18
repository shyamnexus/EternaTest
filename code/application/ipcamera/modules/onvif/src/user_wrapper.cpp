/**
 * @file user_wrapper.cpp
 * @brief C wrapper implementation for User Manager functions
 * 
 * Implements the C wrapper functions by calling into the C++ UserManager class.
 * Note: Uses ipcam::Result<T> which has .success, .value, and .error members.
 */

#include "ipcam/user_wrapper.h"
#include "ipcam/user_manager.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <memory>

using namespace ipcam::config;

// Static UserManager instance
static std::unique_ptr<UserManager> g_user_manager;
static bool g_initialized = false;

// Default DB path — must match the web app's default so both share one database
static const char* DEFAULT_USER_DB_PATH = "/mnt/app/ipcamera/db/users.db";

// Ensure UserManager is initialized
static bool ensure_initialized() {
    if (g_initialized) return true;
    
    // Read DB path from auth.json config — same key the web app uses
    std::string db_path = ipcam::config::Get<std::string>(
        "auth.database.path", DEFAULT_USER_DB_PATH);
    
    spdlog::info("ONVIF UserManager: opening shared user DB at {}", db_path);
    
    g_user_manager = std::make_unique<UserManager>();
    auto result = g_user_manager->Initialize(db_path);
    if (!result.success) {
        spdlog::error("Failed to initialize UserManager at {}: {}", db_path, result.error);
        g_user_manager.reset();
        return false;
    }
    
    g_initialized = true;
    return true;
}

// Helper to convert between enum types
static UserRole onvif_to_user_role(OnvifUserLevel level) {
    switch (level) {
        case ONVIF_USER_ADMIN: return UserRole::ADMIN;
        case ONVIF_USER_OPERATOR: return UserRole::OPERATOR;
        case ONVIF_USER_USER: return UserRole::VIEWER;
        case ONVIF_USER_ANONYMOUS: return UserRole::VIEWER;
        default: return UserRole::VIEWER;
    }
}

static OnvifUserLevel user_role_to_onvif(UserRole role) {
    switch (role) {
        case UserRole::ADMIN: return ONVIF_USER_ADMIN;
        case UserRole::OPERATOR: return ONVIF_USER_OPERATOR;
        case UserRole::VIEWER: return ONVIF_USER_USER;
        default: return ONVIF_USER_USER;
    }
}

extern "C" {

int onvif_get_user_count(void) {
    if (!ensure_initialized()) return 0;
    
    auto result = g_user_manager->ListUsers();
    if (!result.success) {
        spdlog::warn("Failed to list users: {}", result.error);
        return 0;
    }
    
    return static_cast<int>(result.value.size());
}

int onvif_get_user(int idx, OnvifUser *user) {
    if (!user) return -1;
    if (!ensure_initialized()) return -1;
    
    auto result = g_user_manager->ListUsers();
    if (!result.success) {
        spdlog::warn("Failed to list users: {}", result.error);
        return -1;
    }
    
    if (idx < 0 || idx >= static_cast<int>(result.value.size())) {
        return -1;
    }
    
    const auto& u = result.value[idx];
    strncpy(user->username, u.username.c_str(), sizeof(user->username) - 1);
    user->level = user_role_to_onvif(u.role);
    user->enabled = u.enabled;
    
    return 0;
}

int onvif_get_user_by_name(const char *username, OnvifUser *user) {
    if (!username || !user) return -1;
    if (!ensure_initialized()) return -1;
    
    auto result = g_user_manager->GetUserByUsername(username);
    if (!result.success) {
        return -1;  // User not found
    }
    
    strncpy(user->username, result.value.username.c_str(), sizeof(user->username) - 1);
    user->level = user_role_to_onvif(result.value.role);
    user->enabled = result.value.enabled;
    
    return 0;
}

int onvif_create_user(const char *username, const char *password, OnvifUserLevel level) {
    if (!username || !password) return -1;
    if (!ensure_initialized()) return -1;
    
    auto result = g_user_manager->CreateUser(username, password, onvif_to_user_role(level), true);
    if (!result.success) {
        spdlog::error("Failed to create user: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_delete_user(const char *username) {
    if (!username) return -1;
    if (!ensure_initialized()) return -1;
    
    // First get user ID
    auto user_result = g_user_manager->GetUserByUsername(username);
    if (!user_result.success) {
        spdlog::error("User not found: {}", username);
        return -1;
    }
    
    auto result = g_user_manager->DeleteUser(user_result.value.id);
    if (!result.success) {
        spdlog::error("Failed to delete user: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_set_user_level(const char *username, OnvifUserLevel level) {
    if (!username) return -1;
    if (!ensure_initialized()) return -1;
    
    auto user_result = g_user_manager->GetUserByUsername(username);
    if (!user_result.success) {
        spdlog::error("User not found: {}", username);
        return -1;
    }
    
    auto result = g_user_manager->UpdateUser(user_result.value.id, std::nullopt, 
                                              onvif_to_user_role(level), std::nullopt);
    if (!result.success) {
        spdlog::error("Failed to update user level: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_set_user_password(const char *username, const char *password) {
    if (!username || !password) return -1;
    if (!ensure_initialized()) return -1;
    
    auto user_result = g_user_manager->GetUserByUsername(username);
    if (!user_result.success) {
        spdlog::error("User not found: {}", username);
        return -1;
    }
    
    auto result = g_user_manager->ResetPassword(user_result.value.id, password);
    if (!result.success) {
        spdlog::error("Failed to update password: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_set_user_enabled(const char *username, bool enabled) {
    if (!username) return -1;
    if (!ensure_initialized()) return -1;
    
    auto user_result = g_user_manager->GetUserByUsername(username);
    if (!user_result.success) {
        spdlog::error("User not found: {}", username);
        return -1;
    }
    
    auto result = g_user_manager->UpdateUser(user_result.value.id, std::nullopt, 
                                              std::nullopt, enabled);
    if (!result.success) {
        spdlog::error("Failed to update user enabled state: {}", result.error);
        return -1;
    }
    
    return 0;
}

int onvif_validate_user(const char *username, const char *password) {
    if (!username || !password) return -1;
    if (!ensure_initialized()) return -1;
    
    auto result = g_user_manager->Login(username, password, "127.0.0.1", "ONVIF");
    if (!result.success) {
        return -1;  // Invalid credentials
    }
    
    // Clean up session
    g_user_manager->Logout(result.value.token);
    
    return static_cast<int>(user_role_to_onvif(result.value.role));
}

int onvif_check_user_access(const char *username, const char *password, OnvifUserLevel required_level) {
    if (!username || !password) return -1;
    
    int user_level = onvif_validate_user(username, password);
    if (user_level < 0) {
        return -1;  // Invalid credentials
    }
    
    // Check if user level is sufficient
    // Lower enum value = higher privilege (ADMIN=0, OPERATOR=1, USER=2)
    if (user_level <= required_level) {
        return 0;  // Access granted
    }
    
    return -1;  // Access denied
}

int onvif_is_admin(const char *username) {
    if (!username) return 0;
    if (!ensure_initialized()) return 0;
    
    auto user_result = g_user_manager->GetUserByUsername(username);
    if (!user_result.success) {
        return 0;
    }
    
    return (user_result.value.role == UserRole::ADMIN) ? 1 : 0;
}

int onvif_anonymous_allowed(void) {
    // Check if anonymous access is enabled
    // For now, return false (anonymous not allowed)
    return 0;
}

int onvif_get_max_users(void) {
    return 20;  // Maximum number of users
}

// ============================================================================
// WS-Security Password Functions
// ============================================================================

int onvif_get_user_wsse_password(const char *username, char *password, int password_size) {
    if (!username || !password || password_size <= 0) return -1;
    if (!ensure_initialized()) return -1;
    
    // Retrieve plaintext password from UserManager (which handles de-obfuscation)
    auto result = g_user_manager->GetOnvifPassword(username);
    
    if (!result.success) {
        return -1; // Not found or error
    }
    
    if (static_cast<int>(result.value.size()) >= password_size) {
        spdlog::warn("WSSE: Password too long for buffer ({}/{})", result.value.size(), password_size);
        return -1;
    }
    
    strncpy(password, result.value.c_str(), password_size - 1);
    password[password_size - 1] = '\0';
    
    return 0;
}

int onvif_store_user_wsse_password(const char *username, const char *password) {
    // No-op: Password storage is now handled internally by UserManager::CreateUser and UpdateUser
    // We keep this function symbol for ABI compatibility if needed, or we can just return 0.
    // Since the wrapper functions above (onvif_create_user, etc.) call UserManager methods directly,
    // the password is automatically stored.
    return 0;
}

} // extern "C"
