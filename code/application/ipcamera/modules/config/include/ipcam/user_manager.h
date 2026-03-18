#pragma once

#include <ipcam/result.h>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <optional>

namespace ipcam {
namespace config {

/**
 * @brief User role enumeration
 */
enum class UserRole {
    ADMIN = 0,      // Full system access
    OPERATOR = 1,   // Can view and control camera
    VIEWER = 2      // Read-only access
};

/**
 * @brief Security question structure for password recovery
 */
struct SecurityQuestion {
    int id;
    std::string question;
};

/**
 * @brief User's security question answer (stored hashed)
 */
struct UserSecurityQuestion {
    int64_t user_id;
    int question_id;
    std::string question;       // The question text
    std::string answer_hash;    // Hashed answer (never expose)
};

/**
 * @brief Password reset token information
 */
struct PasswordResetToken {
    std::string token;
    int64_t user_id;
    int64_t created_at;
    int64_t expires_at;
    bool used;
};

/**
 * @brief User information structure
 */
struct User {
    int64_t id;
    std::string username;
    std::string password_hash;  // Never expose this in API responses
    UserRole role;
    bool enabled;
    bool password_must_change;  // Force password change on next login
    bool security_questions_set;  // Whether user has set up security questions
    int64_t created_at;
    int64_t last_login;
    std::string last_login_ip;
    int failed_attempts;         // Failed login attempts counter
    int64_t locked_until;        // Unix timestamp when lockout expires (0 = not locked)
};

/**
 * @brief Session information structure
 */
struct Session {
    std::string token;
    int64_t user_id;
    std::string username;
    UserRole role;
    int64_t created_at;
    int64_t expires_at;
    std::string ip_address;
    std::string user_agent;
};

/**
 * @brief User manager for authentication and user management
 */
class UserManager {
public:
    UserManager();
    ~UserManager();

    // Initialization
    Result<void> Initialize(const std::string& db_path);
    Result<void> InitializeDefaultAdmin();

    // Authentication
    Result<Session> Login(const std::string& username, const std::string& password, 
                         const std::string& ip_address, const std::string& user_agent);
    Result<void> Logout(const std::string& token);
    Result<Session> ValidateToken(const std::string& token);
    Result<void> RefreshToken(const std::string& token, Session& session);

    // Password management
    Result<void> ChangePassword(int64_t user_id, const std::string& old_password, 
                                const std::string& new_password);
    Result<void> ResetPassword(int64_t user_id, const std::string& new_password);
    static std::string HashPassword(const std::string& password);
    static bool VerifyPassword(const std::string& password, const std::string& hash);

    // ONVIF WS-Security support
    // Returns the plaintext password (de-obfuscated) for WS-UsernameToken verification
    Result<std::string> GetOnvifPassword(const std::string& username);

    // Security questions for password recovery
    static std::vector<SecurityQuestion> GetAvailableSecurityQuestions();
    Result<void> SetSecurityQuestions(int64_t user_id, 
                                      const std::vector<std::pair<int, std::string>>& qa_pairs);
    Result<std::vector<SecurityQuestion>> GetUserSecurityQuestions(const std::string& username);
    Result<std::string> VerifySecurityAnswers(const std::string& username,
                                               const std::vector<std::pair<int, std::string>>& answers);
    Result<void> ResetPasswordWithToken(const std::string& token, const std::string& new_password);
    bool HasSecurityQuestionsSet(int64_t user_id);

    // User management (CRUD)
    Result<std::vector<User>> ListUsers();
    Result<User> GetUser(int64_t user_id);
    Result<User> GetUserByUsername(const std::string& username);
    Result<int64_t> CreateUser(const std::string& username, const std::string& password, 
                               UserRole role, bool enabled = true);
    Result<void> UpdateUser(int64_t user_id, const std::optional<std::string>& username,
                           const std::optional<UserRole>& role, 
                           const std::optional<bool>& enabled);
    Result<void> DeleteUser(int64_t user_id);

    // Session management
    Result<std::vector<Session>> ListActiveSessions();
    Result<void> DeleteSession(const std::string& token);
    Result<void> DeleteUserSessions(int64_t user_id);
    Result<void> CleanupExpiredSessions();

    // Token generation
    static std::string GenerateToken();
    static std::string GenerateResetToken();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Helper functions
std::string UserRoleToString(UserRole role);
std::optional<UserRole> StringToUserRole(const std::string& role_str);

} // namespace config
} // namespace ipcam
