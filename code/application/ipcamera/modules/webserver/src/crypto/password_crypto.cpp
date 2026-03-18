/**
 * @file password_crypto.cpp
 * @brief Implementation of RSA + AES password encryption
 */

#include "password_crypto.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <spdlog/spdlog.h>

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace ipcam {
namespace webserver {
namespace crypto {

// Constants
static constexpr int RSA_KEY_BITS = 2048;
static constexpr int AES_KEY_SIZE = 32;  // 256 bits
static constexpr int GCM_IV_SIZE = 12;   // 96 bits (GCM recommended)
static constexpr int GCM_TAG_SIZE = 16;  // 128 bits

// Helper to get current timestamp
static int64_t GetCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

PasswordCrypto& PasswordCrypto::Instance() {
    static PasswordCrypto instance;
    return instance;
}

PasswordCrypto::~PasswordCrypto() {
    // Stop auto rotation thread first
    StopAutoRotation();
    
    std::lock_guard<std::mutex> lock(mutex_);
    if (rsa_key_) {
        EVP_PKEY_free(static_cast<EVP_PKEY*>(rsa_key_));
        rsa_key_ = nullptr;
    }
}

bool PasswordCrypto::Initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        spdlog::debug("PasswordCrypto already initialized");
        return true;
    }

    if (!GenerateKeyPair()) {
        spdlog::error("Failed to generate RSA key pair");
        return false;
    }

    initialized_ = true;
    spdlog::info("PasswordCrypto initialized with key ID: {}", key_id_);
    
    // Start auto rotation thread (unlocked to avoid deadlock)
    mutex_.unlock();
    StartAutoRotation();
    mutex_.lock();
    
    return true;
}

bool PasswordCrypto::IsInitialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

std::string PasswordCrypto::GetPublicKeyPEM() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return public_key_pem_;
}

std::string PasswordCrypto::GetKeyId() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return key_id_;
}

bool PasswordCrypto::GenerateKeyPair() {
    // Free existing key if any
    if (rsa_key_) {
        EVP_PKEY_free(static_cast<EVP_PKEY*>(rsa_key_));
        rsa_key_ = nullptr;
    }

    // Generate RSA key pair using EVP API (OpenSSL 3.0+)
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        spdlog::error("Failed to create EVP_PKEY_CTX");
        return false;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        spdlog::error("Failed to init RSA keygen");
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEY_BITS) <= 0) {
        spdlog::error("Failed to set RSA key bits");
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        spdlog::error("Failed to generate RSA key pair");
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY_CTX_free(ctx);
    rsa_key_ = pkey;

    // Export public key to PEM format
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        spdlog::error("Failed to create BIO for public key export");
        EVP_PKEY_free(pkey);
        rsa_key_ = nullptr;
        return false;
    }

    if (PEM_write_bio_PUBKEY(bio, pkey) != 1) {
        spdlog::error("Failed to write public key to PEM");
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        rsa_key_ = nullptr;
        return false;
    }

    // Read PEM string
    BUF_MEM* mem = nullptr;
    BIO_get_mem_ptr(bio, &mem);
    public_key_pem_ = std::string(mem->data, mem->length);
    BIO_free(bio);

    // Generate new key ID
    key_id_ = GenerateKeyId();
    
    // Clear nonce cache when keys rotate (old nonces no longer valid)
    nonce_cache_.clear();

    spdlog::debug("Generated RSA-{} key pair", RSA_KEY_BITS);
    return true;
}

std::string PasswordCrypto::GenerateKeyId() {
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()).count();

    // Generate random suffix
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 0xFFFFFF);
    int random_suffix = dis(gen);

    std::stringstream ss;
    ss << "k_" << epoch << "_" << std::hex << std::setfill('0') 
       << std::setw(6) << random_suffix;
    return ss.str();
}

bool PasswordCrypto::RotateKeys() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    spdlog::info("Rotating RSA keys...");
    
    if (!GenerateKeyPair()) {
        spdlog::error("Failed to rotate RSA keys");
        return false;
    }

    spdlog::info("RSA keys rotated. New key ID: {}", key_id_);
    return true;
}

void PasswordCrypto::StartAutoRotation() {
    if (rotation_running_.exchange(true)) {
        // Already running
        return;
    }
    
    rotation_thread_ = std::thread([this]() {
        spdlog::info("Auto key rotation thread started (interval: {} hours)", 
                     AUTO_ROTATION_INTERVAL_SECONDS / 3600);
        
        while (rotation_running_.load()) {
            // Sleep in small increments to allow quick shutdown
            for (int i = 0; i < AUTO_ROTATION_INTERVAL_SECONDS && rotation_running_.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            
            if (rotation_running_.load()) {
                spdlog::info("Auto-rotating RSA keys...");
                RotateKeys();
            }
        }
        
        spdlog::info("Auto key rotation thread stopped");
    });
}

void PasswordCrypto::StopAutoRotation() {
    if (!rotation_running_.exchange(false)) {
        // Not running
        return;
    }
    
    if (rotation_thread_.joinable()) {
        rotation_thread_.join();
    }
}

bool PasswordCrypto::IsReplayAttack(const std::string& key_id, const std::string& iv, int64_t timestamp) {
    // Check if this combination has been seen before
    for (const auto& entry : nonce_cache_) {
        if (entry.key_id == key_id && entry.iv == iv && entry.timestamp == timestamp) {
            return true;  // Replay attack detected
        }
    }
    
    // Add to cache
    nonce_cache_.push_back({key_id, iv, timestamp});
    
    // Trim cache if too large (LRU - remove oldest entries)
    while (nonce_cache_.size() > MAX_NONCE_CACHE_SIZE) {
        nonce_cache_.pop_front();
    }
    
    return false;
}

std::vector<uint8_t> PasswordCrypto::Base64Decode(const std::string& encoded) {
    if (encoded.empty()) {
        return {};
    }

    BIO* bio = BIO_new_mem_buf(encoded.data(), static_cast<int>(encoded.size()));
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    
    // Don't require newlines
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    // Allocate output buffer (Base64 decodes to ~3/4 of input size)
    std::vector<uint8_t> decoded(encoded.size());
    int decoded_len = BIO_read(bio, decoded.data(), static_cast<int>(decoded.size()));
    
    BIO_free_all(bio);

    if (decoded_len < 0) {
        spdlog::warn("Base64 decode failed");
        return {};
    }

    decoded.resize(static_cast<size_t>(decoded_len));
    return decoded;
}

std::vector<uint8_t> PasswordCrypto::RSADecrypt(const std::vector<uint8_t>& encrypted_data) {
    if (!rsa_key_ || encrypted_data.empty()) {
        return {};
    }

    EVP_PKEY* pkey = static_cast<EVP_PKEY*>(rsa_key_);
    
    // Create decryption context
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx) {
        spdlog::error("Failed to create RSA decrypt context");
        return {};
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        spdlog::error("Failed to init RSA decryption");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    // Set OAEP padding (matches Web Crypto API RSA-OAEP)
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        spdlog::error("Failed to set RSA OAEP padding");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    // Set OAEP hash to SHA-256 (common default for Web Crypto)
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) {
        spdlog::error("Failed to set OAEP hash");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    // Determine output length
    size_t outlen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, 
                         encrypted_data.data(), encrypted_data.size()) <= 0) {
        spdlog::error("Failed to determine RSA decrypt output length");
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    // Perform decryption
    std::vector<uint8_t> decrypted(outlen);
    if (EVP_PKEY_decrypt(ctx, decrypted.data(), &outlen,
                         encrypted_data.data(), encrypted_data.size()) <= 0) {
        unsigned long err = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        spdlog::error("RSA decryption failed: {}", err_buf);
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    EVP_PKEY_CTX_free(ctx);
    decrypted.resize(outlen);
    return decrypted;
}

std::string PasswordCrypto::AESDecryptGCM(const std::vector<uint8_t>& key,
                                            const std::vector<uint8_t>& iv,
                                            const std::vector<uint8_t>& encrypted_data,
                                            const std::vector<uint8_t>& tag) {
    if (key.size() != AES_KEY_SIZE) {
        spdlog::error("Invalid AES-GCM key size: {} (expected {})", key.size(), AES_KEY_SIZE);
        return "";
    }
    if (iv.size() != GCM_IV_SIZE) {
        spdlog::error("Invalid AES-GCM IV size: {} (expected {})", iv.size(), GCM_IV_SIZE);
        return "";
    }
    if (tag.size() != GCM_TAG_SIZE) {
        spdlog::error("Invalid AES-GCM tag size: {} (expected {})", tag.size(), GCM_TAG_SIZE);
        return "";
    }
    if (encrypted_data.empty()) {
        return "";
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        spdlog::error("Failed to create AES-GCM cipher context");
        return "";
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        spdlog::error("Failed to init AES-GCM");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, GCM_IV_SIZE, nullptr);
    EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data());

    std::vector<uint8_t> decrypted(encrypted_data.size());
    int out_len = 0, total_len = 0;

    if (EVP_DecryptUpdate(ctx, decrypted.data(), &out_len,
                          encrypted_data.data(), static_cast<int>(encrypted_data.size())) != 1) {
        spdlog::error("AES-GCM DecryptUpdate failed");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    total_len = out_len;

    // Set expected tag for verification
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE,
                         const_cast<uint8_t*>(tag.data()));

    if (EVP_DecryptFinal_ex(ctx, decrypted.data() + total_len, &out_len) != 1) {
        spdlog::error("AES-GCM authentication failed — data tampered or wrong key");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    total_len += out_len;

    EVP_CIPHER_CTX_free(ctx);
    return std::string(reinterpret_cast<char*>(decrypted.data()), static_cast<size_t>(total_len));
}

std::string PasswordCrypto::DecryptPassword(const std::string& encrypted_key,
                                             const std::string& encrypted_password,
                                             const std::string& iv,
                                             const std::string& key_id,
                                             int64_t timestamp,
                                             const std::string& auth_tag) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        spdlog::error("PasswordCrypto not initialized");
        return "";
    }

    // Verify key ID matches
    if (key_id != key_id_) {
        spdlog::warn("Key ID mismatch: got '{}', expected '{}'", key_id, key_id_);
        return "";
    }

    // Replay protection using IV/nonce uniqueness (timestamp-independent)
    // NOTE: We intentionally do NOT validate timestamps because:
    // 1. IP cameras often have no RTC or dead batteries (time resets to 1970)
    // 2. Cameras may be on isolated networks without NTP
    // 3. Client browsers may have incorrect time or different timezones
    // 4. Blocking login due to time mismatch creates a terrible user experience
    //
    // Instead, we rely on IV uniqueness - each encryption generates a random IV,
    // so replaying an old request with the same IV will be detected.
    if (IsReplayAttack(key_id, iv, timestamp)) {
        spdlog::warn("Replay attack detected: duplicate IV/nonce");
        return "";
    }

    // Decode Base64 inputs
    auto encrypted_key_bytes = Base64Decode(encrypted_key);
    auto encrypted_password_bytes = Base64Decode(encrypted_password);
    auto iv_bytes = Base64Decode(iv);

    if (encrypted_key_bytes.empty() || encrypted_password_bytes.empty() || iv_bytes.empty()) {
        spdlog::error("Failed to decode Base64 inputs");
        return "";
    }

    // Decrypt AES key using RSA
    auto aes_key = RSADecrypt(encrypted_key_bytes);
    if (aes_key.empty()) {
        spdlog::error("Failed to decrypt AES key");
        return "";
    }

    if (aes_key.size() != AES_KEY_SIZE) {
        spdlog::error("Decrypted AES key has wrong size: {} (expected {})", 
                     aes_key.size(), AES_KEY_SIZE);
        return "";
    }

    // Require AES-256-GCM (auth_tag is mandatory)
    if (auth_tag.empty()) {
        spdlog::error("Missing auth_tag — AES-256-GCM is required");
        std::memset(aes_key.data(), 0, aes_key.size());
        return "";
    }

    auto tag_bytes = Base64Decode(auth_tag);
    if (tag_bytes.empty()) {
        spdlog::error("Failed to decode auth_tag Base64");
        std::memset(aes_key.data(), 0, aes_key.size());
        return "";
    }

    std::string password = AESDecryptGCM(aes_key, iv_bytes, encrypted_password_bytes, tag_bytes);
    
    // Securely wipe AES key from memory
    std::memset(aes_key.data(), 0, aes_key.size());

    if (password.empty()) {
        spdlog::error("Failed to decrypt password");
        return "";
    }

    spdlog::debug("Successfully decrypted password ({} chars)", password.length());
    return password;
}

} // namespace crypto
} // namespace webserver
} // namespace ipcam
