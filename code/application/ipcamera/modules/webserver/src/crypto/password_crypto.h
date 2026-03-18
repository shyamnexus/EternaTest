/**
 * @file password_crypto.h
 * @brief Password encryption/decryption using RSA + AES
 * 
 * Provides secure password transmission from Web UI to camera backend:
 * 1. Camera generates RSA key pair on startup
 * 2. Web UI fetches public key and generates random AES session key
 * 3. Web UI encrypts password with AES, encrypts AES key with RSA
 * 4. Camera decrypts AES key with RSA private key, then decrypts password
 * 
 * Security features:
 * - Replay protection: timestamp validation + nonce cache
 * - Automatic key rotation every 24 hours
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <unordered_set>
#include <deque>

namespace ipcam {
namespace webserver {
namespace crypto {

/**
 * @brief Nonce entry for replay protection
 */
struct NonceEntry {
    std::string key_id;
    std::string iv;
    int64_t timestamp;
};

/**
 * @brief Password cryptography manager using RSA + AES
 * 
 * Thread-safe singleton for handling encrypted passwords from Web UI.
 * 
 * Flow:
 * 1. Call Initialize() on startup to generate RSA key pair
 * 2. Expose GetPublicKeyPEM() via API for Web UI
 * 3. Call DecryptPassword() to decrypt incoming encrypted passwords
 * 
 * Security:
 * - Validates timestamp within ±60 seconds of server time
 * - Rejects duplicate (key_id, iv, timestamp) combinations
 * - Auto-rotates RSA keys every 24 hours
 */
class PasswordCrypto {
public:
    /**
     * @brief Get singleton instance
     */
    static PasswordCrypto& Instance();

    /**
     * @brief Initialize RSA key pair (call on startup)
     * @return true if initialization successful
     */
    bool Initialize();

    /**
     * @brief Check if crypto system is initialized
     */
    bool IsInitialized() const;

    /**
     * @brief Get public key in PEM format for Web UI
     * @return PEM-formatted public key string
     */
    std::string GetPublicKeyPEM() const;

    /**
     * @brief Get current key ID (for matching requests with correct private key)
     * @return Key ID string
     */
    std::string GetKeyId() const;

    /**
     * @brief Decrypt password from Web UI (with replay protection)
     * 
     * @param encrypted_key RSA-OAEP encrypted AES key (Base64)
     * @param encrypted_password AES-256 encrypted password (Base64)
     * @param iv Initialization vector for AES (Base64)
     * @param key_id Key ID to verify correct key pair
     * @param timestamp Unix timestamp from client (for replay protection)
     * @param auth_tag GCM authentication tag (Base64). Required — AES-256-GCM only.
     * @return Decrypted password or empty string on failure
     */
    std::string DecryptPassword(const std::string& encrypted_key,
                                 const std::string& encrypted_password,
                                 const std::string& iv,
                                 const std::string& key_id,
                                 int64_t timestamp,
                                 const std::string& auth_tag);

    /**
     * @brief Rotate RSA key pair (for security)
     * @return true if rotation successful
     */
    bool RotateKeys();

    /**
     * @brief Get algorithm info for API response
     */
    std::string GetRSAAlgorithm() const { return "RSA-OAEP"; }
    std::string GetAESAlgorithm() const { return "AES-256-GCM"; }
    int GetRSAKeySize() const { return 2048; }
    
    /**
     * @brief Get timestamp tolerance for replay protection (seconds)
     * @note Set to 300s (5 minutes)
     */
    int GetTimestampTolerance() const { return 300; }

    // Prevent copying
    PasswordCrypto(const PasswordCrypto&) = delete;
    PasswordCrypto& operator=(const PasswordCrypto&) = delete;

private:
    PasswordCrypto() = default;
    ~PasswordCrypto();

    /**
     * @brief Generate new RSA key pair
     * @return true if successful
     */
    bool GenerateKeyPair();

    /**
     * @brief Generate unique key ID
     * @return Key ID string (format: k_<timestamp>_<random>)
     */
    std::string GenerateKeyId();

    /**
     * @brief Base64 decode
     * @param encoded Base64 encoded string
     * @return Decoded bytes
     */
    std::vector<uint8_t> Base64Decode(const std::string& encoded);

    /**
     * @brief RSA-OAEP decrypt
     * @param encrypted_data Encrypted bytes
     * @return Decrypted bytes or empty on failure
     */
    std::vector<uint8_t> RSADecrypt(const std::vector<uint8_t>& encrypted_data);

    /**
     * @brief AES-256-GCM decrypt
     * @param key AES key (32 bytes)
     * @param iv Initialization vector (12 bytes)
     * @param encrypted_data Encrypted bytes
     * @param tag Authentication tag (16 bytes)
     * @return Decrypted string or empty on failure
     */
    std::string AESDecryptGCM(const std::vector<uint8_t>& key,
                              const std::vector<uint8_t>& iv,
                              const std::vector<uint8_t>& encrypted_data,
                              const std::vector<uint8_t>& tag);

    /**
     * @brief Check if nonce (key_id + iv + timestamp) has been seen before
     * @return true if duplicate (replay attack), false if new
     */
    bool IsReplayAttack(const std::string& key_id, const std::string& iv, int64_t timestamp);

    /**
     * @brief Start automatic key rotation thread
     */
    void StartAutoRotation();

    /**
     * @brief Stop automatic key rotation thread
     */
    void StopAutoRotation();

    // RSA key pair (OpenSSL EVP_PKEY*)
    void* rsa_key_ = nullptr;
    
    // Key metadata
    std::string key_id_;
    std::string public_key_pem_;
    
    // Thread safety
    mutable std::mutex mutex_;
    bool initialized_ = false;
    
    // Replay protection: nonce cache (LRU, max 1000 entries)
    std::deque<NonceEntry> nonce_cache_;
    static constexpr size_t MAX_NONCE_CACHE_SIZE = 1000;
    
    // Auto key rotation thread (24 hours)
    std::thread rotation_thread_;
    std::atomic<bool> rotation_running_{false};
    static constexpr int AUTO_ROTATION_INTERVAL_SECONDS = 24 * 60 * 60;  // 24 hours
};

} // namespace crypto
} // namespace webserver
} // namespace ipcam
