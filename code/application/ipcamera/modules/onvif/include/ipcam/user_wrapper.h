/**
 * @file user_wrapper.h
 * @brief C wrapper for C++ User Manager functions for ONVIF integration
 * 
 * Provides C-callable functions to interact with the ipcam::config::UserManager
 * class from ONVIF C code.
 */

#ifndef ONVIF_USER_WRAPPER_H
#define ONVIF_USER_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// User Role Types (matching ONVIF user levels)
// ============================================================================
typedef enum {
    ONVIF_USER_ADMIN = 0,       // Full access
    ONVIF_USER_OPERATOR = 1,    // Can configure and control
    ONVIF_USER_USER = 2,        // View only (mapped to VIEWER)
    ONVIF_USER_ANONYMOUS = 3    // Public access
} OnvifUserLevel;

// ============================================================================
// User Information
// ============================================================================
typedef struct {
    char username[64];
    OnvifUserLevel level;
    bool enabled;
} OnvifUser;

// ============================================================================
// User Management Functions
// ============================================================================

/**
 * @brief Get number of users
 * @return Number of users
 */
int onvif_get_user_count(void);

/**
 * @brief Get user by index
 * @param idx User index
 * @param user Output user info
 * @return 0 on success, -1 on error
 */
int onvif_get_user(int idx, OnvifUser *user);

/**
 * @brief Get user by username
 * @param username Username to find
 * @param user Output user info
 * @return 0 on success, -1 on error (not found)
 */
int onvif_get_user_by_name(const char *username, OnvifUser *user);

/**
 * @brief Create new user
 * @param username Username
 * @param password Password (plaintext)
 * @param level User level
 * @return 0 on success, -1 on error
 */
int onvif_create_user(const char *username, const char *password, OnvifUserLevel level);

/**
 * @brief Delete user
 * @param username Username to delete
 * @return 0 on success, -1 on error
 */
int onvif_delete_user(const char *username);

/**
 * @brief Set user level
 * @param username Username
 * @param level New level
 * @return 0 on success, -1 on error
 */
int onvif_set_user_level(const char *username, OnvifUserLevel level);

/**
 * @brief Set user password
 * @param username Username
 * @param password New password (plaintext)
 * @return 0 on success, -1 on error
 */
int onvif_set_user_password(const char *username, const char *password);

/**
 * @brief Enable/disable user
 * @param username Username
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_user_enabled(const char *username, bool enabled);

/**
 * @brief Validate user credentials
 * @param username Username
 * @param password Password (plaintext)
 * @return User level if valid, -1 if invalid
 */
int onvif_validate_user(const char *username, const char *password);

/**
 * @brief Validate user access for ONVIF operation
 * @param username Username
 * @param password Password
 * @param required_level Minimum required user level
 * @return 0 if access granted, -1 if denied
 */
int onvif_check_user_access(const char *username, const char *password, OnvifUserLevel required_level);

/**
 * @brief Check if user has admin privileges
 * @param username Username
 * @return 1 if admin, 0 if not
 */
int onvif_is_admin(const char *username);

/**
 * @brief Check if anonymous access is allowed
 * @return 1 if allowed, 0 if not
 */
int onvif_anonymous_allowed(void);

/**
 * @brief Get maximum allowed users
 * @return Maximum user count
 */
int onvif_get_max_users(void);

// ============================================================================
// WS-Security Password Functions
// ============================================================================
// WS-UsernameToken PasswordDigest requires the server to have the plaintext
// password: Digest = Base64(SHA1(Nonce + Created + Password)).
// Since the main UserManager stores irreversible PBKDF2 hashes, the ONVIF
// layer maintains a separate table (onvif_wsse_passwords) in the SAME shared
// database with obfuscated (XOR) passwords for WS-Security verification.

/**
 * @brief Retrieve the plaintext password for WS-Security verification.
 *
 * Looks up the username in the onvif_wsse_passwords table and returns
 * the de-obfuscated plaintext password.
 *
 * @param username  Username to look up
 * @param password  Output buffer for the plaintext password
 * @param password_size  Size of the output buffer
 * @return 0 on success, -1 if user not found or error
 */
int onvif_get_user_wsse_password(const char *username, char *password, int password_size);

/**
 * @brief Store the WS-Security password for a user.
 *
 * Must be called whenever a user is created or their password changes,
 * so the ONVIF WS-Security table stays in sync with the main user DB.
 *
 * @param username  Username
 * @param password  Plaintext password to store (will be obfuscated)
 * @return 0 on success, -1 on error
 */
int onvif_store_user_wsse_password(const char *username, const char *password);

#ifdef __cplusplus
}
#endif

#endif /* ONVIF_USER_WRAPPER_H */
