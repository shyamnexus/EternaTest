/**
 * @file crypto_handler.h
 * @brief HTTP handlers for cryptography-related endpoints
 */

#pragma once

#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

/**
 * @brief GET /api/v1/auth/public-key
 * 
 * Returns the RSA public key for password encryption.
 * Web UI should fetch this before login/password operations.
 * 
 * Response:
 * {
 *     "status": "success",
 *     "public_key": "-----BEGIN PUBLIC KEY-----\n...",
 *     "key_id": "k_1706359200_abc123",
 *     "algorithm": "RSA-OAEP",
 *     "aes_algorithm": "AES-256-GCM",
 *     "rsa_key_size": 2048
 * }
 */
api::Response HandleGetPublicKey(const api::RequestContext& ctx);

/**
 * @brief POST /api/v1/auth/rotate-keys (admin only)
 * 
 * Rotate the RSA key pair. Existing encrypted passwords with old key_id will fail.
 * 
 * Response:
 * {
 *     "status": "success",
 *     "message": "Keys rotated successfully",
 *     "new_key_id": "k_1706360000_def456"
 * }
 */
api::Response HandleRotateKeys(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
