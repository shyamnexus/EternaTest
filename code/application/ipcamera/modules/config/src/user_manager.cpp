#include <ipcam/user_manager.h>
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <set>
#include <map>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>

namespace ipcam {
namespace config {

namespace {
    // Session expiry: 24 hours
    const int64_t SESSION_EXPIRY_SECONDS = 24 * 60 * 60;
    
    // Get current timestamp in seconds
    int64_t GetCurrentTimestamp() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
    
    // Convert bytes to hex string
    std::string BytesToHex(const unsigned char* bytes, size_t len) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) {
            oss << std::setw(2) << static_cast<int>(bytes[i]);
        }
        return oss.str();
    }

    // Convert hex string to bytes
    std::string HexToBytes(const std::string& hex) {
        std::string bytes;
        bytes.reserve(hex.length() / 2);
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            char byte = (char)strtol(byteString.c_str(), nullptr, 16);
            bytes.push_back(byte);
        }
        return bytes;
    }

    // ========================================================================
    // WSSE Password Encryption (AES-256-GCM with device-derived key)
    // ========================================================================
    // Legacy XOR key retained ONLY for reading old DB entries during migration
    const unsigned char WSSE_LEGACY_XOR_KEY[] = {
        0x4F, 0x4E, 0x56, 0x49, 0x46, 0x57, 0x53, 0x53,
        0x45, 0x50, 0x41, 0x53, 0x53, 0x4B, 0x45, 0x59
    };
    const int WSSE_LEGACY_XOR_KEY_LEN = sizeof(WSSE_LEGACY_XOR_KEY);

    // Forward declaration (defined after DeriveKeyFromDevice)
    std::string DeriveKeyFromDevice(const std::string& purpose);

    std::string WsseEncrypt(const std::string &plaintext) {
        std::string key_hex = DeriveKeyFromDevice("wsse");
        unsigned char key[32];
        for (size_t i = 0; i < 32; ++i) {
            sscanf(key_hex.c_str() + i * 2, "%2hhx", &key[i]);
        }

        unsigned char iv[12]; // 96-bit IV for GCM
        RAND_bytes(iv, sizeof(iv));

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), nullptr);
        EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv);

        std::vector<unsigned char> ciphertext(plaintext.size() + 16);
        int len = 0, total = 0;
        EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                          reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size());
        total = len;
        EVP_EncryptFinal_ex(ctx, ciphertext.data() + total, &len);
        total += len;

        unsigned char tag[16];
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag);
        EVP_CIPHER_CTX_free(ctx);

        // Wipe key from stack
        OPENSSL_cleanse(key, sizeof(key));

        // Format: aes256gcm:<iv_hex>:<ciphertext_hex>:<tag_hex>
        return "aes256gcm:" + BytesToHex(iv, sizeof(iv)) + ":" +
               BytesToHex(ciphertext.data(), total) + ":" + BytesToHex(tag, sizeof(tag));
    }

    std::string WsseDecrypt(const std::string &stored) {
        // New AES-256-GCM format
        if (stored.size() > 10 && stored.substr(0, 10) == "aes256gcm:") {
            std::string rest = stored.substr(10);
            size_t sep1 = rest.find(':');
            size_t sep2 = rest.find(':', sep1 + 1);
            if (sep1 == std::string::npos || sep2 == std::string::npos) return "";

            std::string iv_hex = rest.substr(0, sep1);
            std::string ct_hex = rest.substr(sep1 + 1, sep2 - sep1 - 1);
            std::string tag_hex = rest.substr(sep2 + 1);

            std::string iv_bytes = HexToBytes(iv_hex);
            std::string ct_bytes = HexToBytes(ct_hex);
            std::string tag_bytes = HexToBytes(tag_hex);

            if (iv_bytes.size() != 12 || tag_bytes.size() != 16) return "";

            std::string key_hex = DeriveKeyFromDevice("wsse");
            unsigned char key[32];
            for (size_t i = 0; i < 32; ++i) {
                sscanf(key_hex.c_str() + i * 2, "%2hhx", &key[i]);
            }

            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr);
            EVP_DecryptInit_ex(ctx, nullptr, nullptr, key,
                               reinterpret_cast<const unsigned char*>(iv_bytes.data()));

            std::vector<unsigned char> plaintext(ct_bytes.size() + 16);
            int len = 0, total = 0;
            EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                              reinterpret_cast<const unsigned char*>(ct_bytes.data()), ct_bytes.size());
            total = len;

            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16,
                                const_cast<char*>(tag_bytes.data()));

            int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + total, &len);
            EVP_CIPHER_CTX_free(ctx);
            OPENSSL_cleanse(key, sizeof(key));

            if (ret <= 0) {
                spdlog::error("WSSE AES-256-GCM decryption failed (tag verification)");
                return "";
            }
            total += len;
            return std::string(reinterpret_cast<char*>(plaintext.data()), total);
        }

        // Legacy XOR format — decode for backward compatibility
        std::string input = HexToBytes(stored);
        std::string output(input.size(), '\0');
        for (size_t i = 0; i < input.size(); i++) {
            output[i] = input[i] ^ WSSE_LEGACY_XOR_KEY[i % WSSE_LEGACY_XOR_KEY_LEN];
        }
        return output;
    }

    // ========================================================================
    // SQLCipher Key Derivation (Duplicated from storage.cpp for critical shared access)
    // ========================================================================
    constexpr const char* DEVICE_ID_FILE = "/mnt/app/.device_id";
    constexpr const char* DEVICE_SALT_FILE = "/mnt/app/.device_salt";
    // Legacy salt kept for backward-compatible DB migration
    constexpr const char* LEGACY_KEY_DERIVATION_SALT = "ipcam_nvt_2024";
    constexpr int KEY_DERIVATION_ITERATIONS = 100000;

    // Get or generate per-device random salt (persisted at first boot)
    std::string GetDeviceSalt() {
        std::ifstream salt_file(DEVICE_SALT_FILE);
        if (salt_file.is_open()) {
            std::string salt;
            std::getline(salt_file, salt);
            if (!salt.empty() && salt.length() >= 32) {
                return salt;
            }
        }

        // Generate new random salt
        unsigned char random_bytes[16];
        if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
            throw std::runtime_error("RAND_bytes failed — cannot generate device salt");
        }
        std::string salt = BytesToHex(random_bytes, sizeof(random_bytes));

        std::filesystem::path salt_path(DEVICE_SALT_FILE);
        if (!std::filesystem::exists(salt_path.parent_path())) {
            std::filesystem::create_directories(salt_path.parent_path());
        }
        std::ofstream out_file(DEVICE_SALT_FILE);
        if (out_file.is_open()) {
            out_file << salt;
            out_file.close();
            std::filesystem::permissions(DEVICE_SALT_FILE,
                std::filesystem::perms::owner_read,
                std::filesystem::perm_options::replace);
        }
        return salt;
    }

    std::string GenerateDeviceId() {
        unsigned char random_bytes[16];
        if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
            spdlog::critical("RAND_bytes() failed — cannot generate secure device ID");
            throw std::runtime_error("Cryptographic random number generation failed");
        }
        
        std::ostringstream ss;
        for (const auto& byte : random_bytes) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return ss.str();
    }

    std::string GetDeviceId() {
        // Try to read existing device ID (shared with storage module)
        std::ifstream id_file(DEVICE_ID_FILE);
        if (id_file.is_open()) {
            std::string device_id;
            std::getline(id_file, device_id);
            if (!device_id.empty() && device_id.length() >= 32) {
                return device_id;
            }
        }
        
        // If not found, generate it (though storage module usually does this first)
        std::string new_id = GenerateDeviceId();
        
        // Ensure directory exists
        std::filesystem::path id_path(DEVICE_ID_FILE);
        if (!std::filesystem::exists(id_path.parent_path())) {
            std::filesystem::create_directories(id_path.parent_path());
        }
        
        std::ofstream out_file(DEVICE_ID_FILE);
        if (out_file.is_open()) {
            out_file << new_id;
            out_file.close();
            std::filesystem::permissions(DEVICE_ID_FILE, 
                std::filesystem::perms::owner_read,
                std::filesystem::perm_options::replace);
        }
        return new_id;
    }

    std::string DeriveKeyFromDevice(const std::string& purpose) {
        std::string device_id = GetDeviceId();
        std::string device_salt = GetDeviceSalt();
        std::string input = device_id + "_" + purpose + "_" + device_salt;
        
        unsigned char key[32]; // 256-bit key
        PKCS5_PBKDF2_HMAC(
            input.c_str(), input.length(),
            reinterpret_cast<const unsigned char*>(device_salt.c_str()), 
            device_salt.length(),
            KEY_DERIVATION_ITERATIONS,
            EVP_sha256(),
            32, key
        );
        
        std::ostringstream ss;
        for (int i = 0; i < 32; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key[i]);
        }
        return ss.str();
    }

    // Legacy key derivation using hardcoded salt (for migration of existing DBs)
    std::string DeriveKeyFromDeviceLegacy(const std::string& purpose) {
        std::string device_id = GetDeviceId();
        std::string input = device_id + "_" + purpose + "_" + LEGACY_KEY_DERIVATION_SALT;
        
        unsigned char key[32];
        PKCS5_PBKDF2_HMAC(
            input.c_str(), input.length(),
            reinterpret_cast<const unsigned char*>(LEGACY_KEY_DERIVATION_SALT), 
            strlen(LEGACY_KEY_DERIVATION_SALT),
            KEY_DERIVATION_ITERATIONS,
            EVP_sha256(),
            32, key
        );
        
        std::ostringstream ss;
        for (int i = 0; i < 32; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key[i]);
        }
        return ss.str();
    }
}

// Helper to update WSSE password
static void UpdateWssePassword(sqlite3* db, int64_t user_id, const std::string& password) {
    std::string obf = WsseEncrypt(password);
    int64_t now = GetCurrentTimestamp();
    
    const char* sql = "INSERT OR REPLACE INTO onvif_wsse_passwords (user_id, password_obfuscated, updated_at) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, obf.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, now);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        spdlog::error("Failed to prepare WSSE password update stmt");
    }
}

// UserManager::Impl class definition
class UserManager::Impl {
public:
    sqlite3* db_ = nullptr;
    
    Result<void> InitDatabase() {
        const char* users_table_sql = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                role INTEGER NOT NULL DEFAULT 2,
                enabled INTEGER NOT NULL DEFAULT 1,
                password_must_change INTEGER NOT NULL DEFAULT 0,
                created_at INTEGER NOT NULL,
                last_login INTEGER,
                last_login_ip TEXT,
                failed_attempts INTEGER NOT NULL DEFAULT 0,
                locked_until INTEGER NOT NULL DEFAULT 0
            );
        )";
        
        const char* sessions_table_sql = R"(
            CREATE TABLE IF NOT EXISTS sessions (
                token TEXT PRIMARY KEY,
                user_id INTEGER NOT NULL,
                username TEXT NOT NULL,
                role INTEGER NOT NULL,
                created_at INTEGER NOT NULL,
                expires_at INTEGER NOT NULL,
                ip_address TEXT,
                user_agent TEXT,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            );
        )";
        
        // Security questions table for password recovery
        const char* security_questions_table_sql = R"(
            CREATE TABLE IF NOT EXISTS user_security_questions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                question_id INTEGER NOT NULL,
                answer_hash TEXT NOT NULL,
                created_at INTEGER NOT NULL,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
                UNIQUE(user_id, question_id)
            );
        )";
        
        // Password reset tokens table
        const char* reset_tokens_table_sql = R"(
            CREATE TABLE IF NOT EXISTS password_reset_tokens (
                token TEXT PRIMARY KEY,
                user_id INTEGER NOT NULL,
                created_at INTEGER NOT NULL,
                expires_at INTEGER NOT NULL,
                used INTEGER NOT NULL DEFAULT 0,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            );
        )";

        // ONVIF WS-Security passwords (XOR obfuscated plaintext)
        const char* onvif_wsse_passwords_sql = R"(
            CREATE TABLE IF NOT EXISTS onvif_wsse_passwords (
                user_id INTEGER PRIMARY KEY,
                password_obfuscated TEXT NOT NULL,
                updated_at INTEGER NOT NULL,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            );
        )";
        
        const char* indices_sql[] = {
            "CREATE INDEX IF NOT EXISTS idx_sessions_user_id ON sessions(user_id);",
            "CREATE INDEX IF NOT EXISTS idx_sessions_expires_at ON sessions(expires_at);",
            "CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);",
            "CREATE INDEX IF NOT EXISTS idx_security_questions_user_id ON user_security_questions(user_id);",
            "CREATE INDEX IF NOT EXISTS idx_reset_tokens_user_id ON password_reset_tokens(user_id);",
            "CREATE INDEX IF NOT EXISTS idx_reset_tokens_expires_at ON password_reset_tokens(expires_at);"
        };
        
        char* err_msg = nullptr;
        
        // Create users table
        if (sqlite3_exec(db_, users_table_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::string error = err_msg;
            sqlite3_free(err_msg);
            return Result<void>::Err("Failed to create users table: " + error);
        }
        
        // Create sessions table
        if (sqlite3_exec(db_, sessions_table_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::string error = err_msg;
            sqlite3_free(err_msg);
            return Result<void>::Err("Failed to create sessions table: " + error);
        }
        
        // Create security questions table
        if (sqlite3_exec(db_, security_questions_table_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::string error = err_msg;
            sqlite3_free(err_msg);
            return Result<void>::Err("Failed to create security questions table: " + error);
        }
        
        // Create password reset tokens table
        if (sqlite3_exec(db_, reset_tokens_table_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::string error = err_msg;
            sqlite3_free(err_msg);
            return Result<void>::Err("Failed to create password reset tokens table: " + error);
        }
        
        // Create ONVIF WSSE passwords table
        if (sqlite3_exec(db_, onvif_wsse_passwords_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::string error = err_msg;
            sqlite3_free(err_msg);
            return Result<void>::Err("Failed to create ONVIF WSSE table: " + error);
        }

        // Create indices
        for (const char* index_sql : indices_sql) {
            if (sqlite3_exec(db_, index_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
                std::string error = err_msg;
                sqlite3_free(err_msg);
                spdlog::warn("Failed to create index: {}", error);
            }
        }
        
        // Migration: Add failed_attempts and locked_until columns if they don't exist
        const char* migration_sql[] = {
            "ALTER TABLE users ADD COLUMN failed_attempts INTEGER NOT NULL DEFAULT 0;",
            "ALTER TABLE users ADD COLUMN locked_until INTEGER NOT NULL DEFAULT 0;"
        };
        for (const char* sql : migration_sql) {
            // SQLite will error if column already exists, which is expected
            sqlite3_exec(db_, sql, nullptr, nullptr, nullptr);
        }
        
        return Result<void>::Ok();
    }
};

// UserManager implementation
UserManager::UserManager() : impl_(std::make_unique<Impl>()) {}

UserManager::~UserManager() {
    if (impl_->db_) {
        sqlite3_close(impl_->db_);
    }
}

Result<void> UserManager::Initialize(const std::string& db_path) {
    // Create parent directory if it doesn't exist
    size_t last_slash = db_path.find_last_of('/');
    if (last_slash != std::string::npos) {
        std::string dir = db_path.substr(0, last_slash);
        std::filesystem::create_directories(dir);
    }
    
    if (sqlite3_open(db_path.c_str(), &impl_->db_) != SQLITE_OK) {
        return Result<void>::Err("Failed to open database: " + std::string(sqlite3_errmsg(impl_->db_)));
    }

    // Apply SQLCipher encryption key (Unified with storage module)
    std::string key = DeriveKeyFromDevice("users");
    if (!key.empty()) {
        std::string pragma = "PRAGMA key = \"" + key + "\";";
        sqlite3_exec(impl_->db_, pragma.c_str(), nullptr, nullptr, nullptr);

        // Verify key works by attempting a read
        char* err_msg = nullptr;
        int rc = sqlite3_exec(impl_->db_, "SELECT count(*) FROM sqlite_master;", nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            // New key failed — try legacy key and re-key if it works
            if (err_msg) sqlite3_free(err_msg);
            sqlite3_close(impl_->db_);
            impl_->db_ = nullptr;

            if (sqlite3_open(db_path.c_str(), &impl_->db_) != SQLITE_OK) {
                return Result<void>::Err("Failed to reopen database for migration");
            }

            std::string legacy_key = DeriveKeyFromDeviceLegacy("users");
            pragma = "PRAGMA key = \"" + legacy_key + "\";";
            sqlite3_exec(impl_->db_, pragma.c_str(), nullptr, nullptr, nullptr);

            rc = sqlite3_exec(impl_->db_, "SELECT count(*) FROM sqlite_master;", nullptr, nullptr, &err_msg);
            if (rc == SQLITE_OK) {
                // Legacy key works — re-key to new per-device salt key
                spdlog::info("Migrating user DB encryption key to per-device salt");
                std::string rekey = "PRAGMA rekey = \"" + key + "\";";
                sqlite3_exec(impl_->db_, rekey.c_str(), nullptr, nullptr, nullptr);
            } else {
                if (err_msg) sqlite3_free(err_msg);
                spdlog::warn("DB key verification failed (may be unencrypted or new DB)");
            }
        } else {
            if (err_msg) sqlite3_free(err_msg);
        }
    }
    
    // Enable foreign keys
    sqlite3_exec(impl_->db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    
    auto result = impl_->InitDatabase();
    if (!result.success) {
        return result;
    }
    
    // Initialize default admin if no users exist
    return InitializeDefaultAdmin();
}

Result<void> UserManager::InitializeDefaultAdmin() {
    // Check if any users exist
    sqlite3_stmt* stmt;
    const char* count_sql = "SELECT COUNT(*) FROM users;";
    
    if (sqlite3_prepare_v2(impl_->db_, count_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to prepare count query");
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        // Create default admin user (username: admin, password: Admin@123)
        spdlog::info("Creating default admin user (username: admin, password: Admin@123)");
        auto result = CreateUser("admin", "Admin@123", UserRole::ADMIN, true);
        if (!result.success) {
            return Result<void>::Err("Failed to create default admin: " + result.error);
        }
        
        // password_must_change is already set to 1 by CreateUser()
        spdlog::info("Default admin will be required to change password on first login");
    }
    
    return Result<void>::Ok();
}

std::string UserManager::HashPassword(const std::string& password) {
    // Generate random salt (16 bytes)
    unsigned char salt[16];
    RAND_bytes(salt, sizeof(salt));
    
    unsigned char hash[32]; // 256-bit output

    // PBKDF2-HMAC-SHA256 with 210K iterations (OWASP 2023 recommendation)
    // Uses negligible RAM — safe for memory-constrained embedded devices
    PKCS5_PBKDF2_HMAC(
        password.c_str(), password.length(),
        salt, sizeof(salt),
        210000,
        EVP_sha256(),
        sizeof(hash), hash
    );
    return BytesToHex(salt, sizeof(salt)) + ":" + BytesToHex(hash, sizeof(hash));
}

bool UserManager::VerifyPassword(const std::string& password, const std::string& stored_hash) {
    // Format: <salt_hex_32chars>:<hash_hex_64chars>
    size_t separator = stored_hash.find(':');
    if (separator == std::string::npos || separator != 32) {
        return false;
    }

    std::string salt_hex = stored_hash.substr(0, 32);
    std::string hash_hex = stored_hash.substr(33);
    if (hash_hex.length() != 64) {
        return false;
    }
    
    // Convert salt from hex
    unsigned char salt[16];
    for (size_t i = 0; i < 16; ++i) {
        sscanf(salt_hex.c_str() + i * 2, "%2hhx", &salt[i]);
    }
    
    unsigned char computed_hash[32];

    // PBKDF2-HMAC-SHA256 with 210K iterations
    PKCS5_PBKDF2_HMAC(
        password.c_str(), password.length(),
        salt, sizeof(salt),
        210000,
        EVP_sha256(),
        sizeof(computed_hash), computed_hash
    );
    
    // Constant-time comparison to prevent timing side-channel attacks
    std::string computed_hex = BytesToHex(computed_hash, sizeof(computed_hash));
    if (computed_hex.length() != hash_hex.length()) return false;
    return CRYPTO_memcmp(computed_hex.c_str(), hash_hex.c_str(), computed_hex.length()) == 0;
}

std::string UserManager::GenerateToken() {
    unsigned char token_bytes[32];
    RAND_bytes(token_bytes, sizeof(token_bytes));
    return BytesToHex(token_bytes, sizeof(token_bytes));
}

std::string UserManager::GenerateResetToken() {
    return GenerateToken();
}

Result<Session> UserManager::Login(const std::string& username, const std::string& password,
                                   const std::string& ip_address, const std::string& user_agent) {
    // Get user by username
    auto user_result = GetUserByUsername(username);
    if (!user_result.success) {
        spdlog::warn("Login failed for user '{}': user not found", username);
        return Result<Session>::Err("Invalid username or password");
    }
    
    User user = user_result.value;
    int64_t now = GetCurrentTimestamp();
    
    // Check if user is enabled
    if (!user.enabled) {
        spdlog::warn("Login failed for user '{}': account disabled", username);
        return Result<Session>::Err("Account is disabled");
    }
    
    // Check if account is locked
    if (user.locked_until > now) {
        int64_t remaining_seconds = user.locked_until - now;
        int remaining_minutes = static_cast<int>((remaining_seconds + 59) / 60);  // Round up
        spdlog::warn("Login failed for user '{}': account locked for {} more minute(s)", username, remaining_minutes);
        return Result<Session>::Err("Account is locked. Try again in " + std::to_string(remaining_minutes) + " minute(s)");
    }
    
    // Verify password
    if (!VerifyPassword(password, user.password_hash)) {
        spdlog::warn("Login failed for user '{}': invalid password", username);
        
        // Increment failed attempts
        int new_failed_attempts = user.failed_attempts + 1;
        int max_attempts = 5;  // Could be read from config
        int lockout_minutes = 15;  // Could be read from config
        
        if (new_failed_attempts >= max_attempts) {
            // Lock the account
            int64_t locked_until = now + (lockout_minutes * 60);
            const char* lock_sql = "UPDATE users SET failed_attempts = ?, locked_until = ? WHERE id = ?;";
            sqlite3_stmt* lock_stmt;
            if (sqlite3_prepare_v2(impl_->db_, lock_sql, -1, &lock_stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(lock_stmt, 1, new_failed_attempts);
                sqlite3_bind_int64(lock_stmt, 2, locked_until);
                sqlite3_bind_int64(lock_stmt, 3, user.id);
                sqlite3_step(lock_stmt);
                sqlite3_finalize(lock_stmt);
            }
            spdlog::warn("Account '{}' locked for {} minutes after {} failed attempts", username, lockout_minutes, new_failed_attempts);
            return Result<Session>::Err("Account locked due to too many failed attempts. Try again in " + std::to_string(lockout_minutes) + " minutes");
        } else {
            // Just increment failed attempts
            const char* fail_sql = "UPDATE users SET failed_attempts = ? WHERE id = ?;";
            sqlite3_stmt* fail_stmt;
            if (sqlite3_prepare_v2(impl_->db_, fail_sql, -1, &fail_stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(fail_stmt, 1, new_failed_attempts);
                sqlite3_bind_int64(fail_stmt, 2, user.id);
                sqlite3_step(fail_stmt);
                sqlite3_finalize(fail_stmt);
            }
        }
        
        return Result<Session>::Err("Invalid username or password");
    }
    
    // Successful login - reset failed attempts and unlock
    const char* reset_sql = "UPDATE users SET failed_attempts = 0, locked_until = 0 WHERE id = ?;";
    sqlite3_stmt* reset_stmt;
    if (sqlite3_prepare_v2(impl_->db_, reset_sql, -1, &reset_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(reset_stmt, 1, user.id);
        sqlite3_step(reset_stmt);
        sqlite3_finalize(reset_stmt);
    }
    
    // Invalidate any outstanding password reset tokens for this user (Token Management)
    const char* invalidate_tokens_sql = "UPDATE password_reset_tokens SET used = 1 WHERE user_id = ? AND used = 0;";
    sqlite3_stmt* inv_stmt;
    if (sqlite3_prepare_v2(impl_->db_, invalidate_tokens_sql, -1, &inv_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(inv_stmt, 1, user.id);
        int inv_result = sqlite3_step(inv_stmt);
        int invalidated_count = sqlite3_changes(impl_->db_);
        sqlite3_finalize(inv_stmt);
        if (invalidated_count > 0) {
            spdlog::info("Invalidated {} outstanding reset token(s) for user '{}'", invalidated_count, username);
        }
    }
    
    // Invalidate any existing session from same user and same IP (replace behavior)
    const char* delete_existing_sql = "DELETE FROM sessions WHERE user_id = ? AND ip_address = ?;";
    sqlite3_stmt* del_stmt;
    if (sqlite3_prepare_v2(impl_->db_, delete_existing_sql, -1, &del_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(del_stmt, 1, user.id);
        sqlite3_bind_text(del_stmt, 2, ip_address.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(del_stmt);
        int replaced_count = sqlite3_changes(impl_->db_);
        sqlite3_finalize(del_stmt);
        
        if (replaced_count > 0) {
            spdlog::info("Replaced {} existing session(s) for user '{}' from IP {}", 
                         replaced_count, username, ip_address);
        }
    }
    
    // Generate session token
    std::string token = GenerateToken();
    int64_t expires_at = now + SESSION_EXPIRY_SECONDS;
    
    // Store session in database
    const char* insert_sql = R"(
        INSERT INTO sessions (token, user_id, username, role, created_at, expires_at, ip_address, user_agent)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<Session>::Err("Failed to prepare insert session query");
    }
    
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, user.id);
    sqlite3_bind_text(stmt, 3, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, static_cast<int>(user.role));
    sqlite3_bind_int64(stmt, 5, now);
    sqlite3_bind_int64(stmt, 6, expires_at);
    sqlite3_bind_text(stmt, 7, ip_address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, user_agent.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<Session>::Err("Failed to create session");
    }
    sqlite3_finalize(stmt);
    
    // Update last login
    const char* update_sql = "UPDATE users SET last_login = ?, last_login_ip = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(impl_->db_, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, now);
        sqlite3_bind_text(stmt, 2, ip_address.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, user.id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    spdlog::info("User '{}' logged in from {}", username, ip_address);
    
    // Return session
    Session session;
    session.token = token;
    session.user_id = user.id;
    session.username = user.username;
    session.role = user.role;
    session.created_at = now;
    session.expires_at = expires_at;
    session.ip_address = ip_address;
    session.user_agent = user_agent;
    
    return Result<Session>::Ok(session);
}

Result<void> UserManager::Logout(const std::string& token) {
    const char* delete_sql = "DELETE FROM sessions WHERE token = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, delete_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to prepare logout query");
    }
    
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<void>::Err("Failed to logout");
    }
    
    int changes = sqlite3_changes(impl_->db_);
    sqlite3_finalize(stmt);
    
    if (changes == 0) {
        return Result<void>::Err("Session not found");
    }
    
    return Result<void>::Ok();
}

Result<Session> UserManager::ValidateToken(const std::string& token) {
    const char* select_sql = R"(
        SELECT user_id, username, role, created_at, expires_at, ip_address, user_agent
        FROM sessions WHERE token = ?;
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<Session>::Err("Failed to prepare validate token query");
    }
    
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return Result<Session>::Err("Invalid token");
    }
    
    Session session;
    session.token = token;
    session.user_id = sqlite3_column_int64(stmt, 0);
    session.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    session.role = static_cast<UserRole>(sqlite3_column_int(stmt, 2));
    session.created_at = sqlite3_column_int64(stmt, 3);
    session.expires_at = sqlite3_column_int64(stmt, 4);
    
    const char* ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    const char* ua = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    
    if (ip) session.ip_address = ip;
    if (ua) session.user_agent = ua;
    
    sqlite3_finalize(stmt);
    
    // Check if expired
    if (session.expires_at < GetCurrentTimestamp()) {
        DeleteSession(token);
        return Result<Session>::Err("Token expired");
    }
    
    return Result<Session>::Ok(session);
}

Result<void> UserManager::RefreshToken(const std::string& token, Session& session) {
    auto validate_result = ValidateToken(token);
    if (!validate_result.success) {
        return Result<void>::Err(validate_result.error);
    }
    
    // Extend expiry
    int64_t new_expiry = GetCurrentTimestamp() + SESSION_EXPIRY_SECONDS;
    
    const char* update_sql = "UPDATE sessions SET expires_at = ? WHERE token = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, update_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to prepare refresh token query");
    }
    
    sqlite3_bind_int64(stmt, 1, new_expiry);
    sqlite3_bind_text(stmt, 2, token.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<void>::Err("Failed to refresh token");
    }
    
    sqlite3_finalize(stmt);
    
    session = validate_result.value;
    session.expires_at = new_expiry;
    
    return Result<void>::Ok();
}

Result<void> UserManager::ChangePassword(int64_t user_id, const std::string& old_password,
                                        const std::string& new_password) {
    // Get user
    auto user_result = GetUser(user_id);
    if (!user_result.success) {
        return Result<void>::Err("User not found");
    }
    
    User user = user_result.value;
    
    // Verify old password
    if (!VerifyPassword(old_password, user.password_hash)) {
        return Result<void>::Err("Invalid old password");
    }
    
    // Hash new password
    std::string new_hash = HashPassword(new_password);
    
    // Update password and clear password_must_change flag
    const char* update_sql = "UPDATE users SET password_hash = ?, password_must_change = 0 WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, update_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to prepare change password query");
    }
    
    sqlite3_bind_text(stmt, 1, new_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<void>::Err("Failed to update password");
    }
    
    sqlite3_finalize(stmt);
    
    // Update WSSE password
    UpdateWssePassword(impl_->db_, user_id, new_password);
    
    spdlog::info("Password changed for user ID {} (password_must_change cleared)", user_id);
    
    return Result<void>::Ok();
}

Result<void> UserManager::ResetPassword(int64_t user_id, const std::string& new_password) {
    // Hash new password
    std::string new_hash = HashPassword(new_password);
    
    // Update password
    const char* update_sql = "UPDATE users SET password_hash = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, update_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to prepare reset password query");
    }
    
    sqlite3_bind_text(stmt, 1, new_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<void>::Err("Failed to reset password");
    }
    
    int changes = sqlite3_changes(impl_->db_);
    sqlite3_finalize(stmt);
    
    if (changes == 0) {
        return Result<void>::Err("User not found");
    }
    
    // Delete all user sessions
    DeleteUserSessions(user_id);
    
    // Update WSSE password
    UpdateWssePassword(impl_->db_, user_id, new_password);
    
    spdlog::info("Password reset for user ID {}", user_id);
    
    return Result<void>::Ok();
}

Result<std::vector<User>> UserManager::ListUsers() {
    const char* select_sql = R"(
        SELECT id, username, password_hash, role, enabled, password_must_change,
               created_at, last_login, last_login_ip, failed_attempts, locked_until
        FROM users ORDER BY id;
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<std::vector<User>>::Err("Failed to prepare list users query");
    }
    
    std::vector<User> users;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User user;
        user.id = sqlite3_column_int64(stmt, 0);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.role = static_cast<UserRole>(sqlite3_column_int(stmt, 3));
        user.enabled = sqlite3_column_int(stmt, 4) != 0;
        user.password_must_change = sqlite3_column_int(stmt, 5) != 0;
        user.created_at = sqlite3_column_int64(stmt, 6);
        user.last_login = sqlite3_column_int64(stmt, 7);
        
        const char* last_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        if (last_ip) user.last_login_ip = last_ip;
        
        user.failed_attempts = sqlite3_column_int(stmt, 9);
        user.locked_until = sqlite3_column_int64(stmt, 10);
        
        user.security_questions_set = HasSecurityQuestionsSet(user.id);
        
        users.push_back(user);
    }
    
    sqlite3_finalize(stmt);
    
    return Result<std::vector<User>>::Ok(users);
}

Result<User> UserManager::GetUser(int64_t user_id) {
    const char* select_sql = R"(
        SELECT id, username, password_hash, role, enabled, password_must_change,
               created_at, last_login, last_login_ip, failed_attempts, locked_until
        FROM users WHERE id = ?;
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<User>::Err("Failed to prepare get user query");
    }
    
    sqlite3_bind_int64(stmt, 1, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return Result<User>::Err("User not found");
    }
    
    User user;
    user.id = sqlite3_column_int64(stmt, 0);
    user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    user.role = static_cast<UserRole>(sqlite3_column_int(stmt, 3));
    user.enabled = sqlite3_column_int(stmt, 4) != 0;
    user.password_must_change = sqlite3_column_int(stmt, 5) != 0;
    user.created_at = sqlite3_column_int64(stmt, 6);
    user.last_login = sqlite3_column_int64(stmt, 7);
    
    const char* last_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
    if (last_ip) user.last_login_ip = last_ip;
    
    user.failed_attempts = sqlite3_column_int(stmt, 9);
    user.locked_until = sqlite3_column_int64(stmt, 10);
    
    user.security_questions_set = HasSecurityQuestionsSet(user.id);
    
    sqlite3_finalize(stmt);
    
    return Result<User>::Ok(user);
}

Result<User> UserManager::GetUserByUsername(const std::string& username) {
    const char* select_sql = R"(
        SELECT id, username, password_hash, role, enabled, password_must_change, created_at, last_login, last_login_ip, failed_attempts, locked_until
        FROM users WHERE username = ?;
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<User>::Err("Failed to prepare get user by username query");
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return Result<User>::Err("User not found");
    }
    
    User user;
    user.id = sqlite3_column_int64(stmt, 0);
    user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    user.role = static_cast<UserRole>(sqlite3_column_int(stmt, 3));
    user.enabled = sqlite3_column_int(stmt, 4) != 0;
    user.password_must_change = sqlite3_column_int(stmt, 5) != 0;
    user.created_at = sqlite3_column_int64(stmt, 6);
    user.last_login = sqlite3_column_int64(stmt, 7);
    
    const char* last_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
    if (last_ip) user.last_login_ip = last_ip;
    
    user.failed_attempts = sqlite3_column_int(stmt, 9);
    user.locked_until = sqlite3_column_int64(stmt, 10);
    
    sqlite3_finalize(stmt);
    
    return Result<User>::Ok(user);
}

Result<int64_t> UserManager::CreateUser(const std::string& username, const std::string& password,
                                       UserRole role, bool enabled) {
    // Check if username already exists
    auto existing = GetUserByUsername(username);
    if (existing.success) {
        return Result<int64_t>::Err("Username already exists");
    }
    
    // Hash password
    std::string password_hash = HashPassword(password);
    int64_t created_at = GetCurrentTimestamp();
    
    const char* insert_sql = R"(
        INSERT INTO users (username, password_hash, role, enabled, password_must_change, created_at)
        VALUES (?, ?, ?, ?, 1, ?);
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<int64_t>::Err("Failed to prepare create user query");
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, static_cast<int>(role));
    sqlite3_bind_int(stmt, 4, enabled ? 1 : 0);
    sqlite3_bind_int64(stmt, 5, created_at);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<int64_t>::Err("Failed to create user");
    }
    
    int64_t user_id = sqlite3_last_insert_rowid(impl_->db_);
    sqlite3_finalize(stmt);
    
    // Store WSSE password for ONVIF
    UpdateWssePassword(impl_->db_, user_id, password);
    
    spdlog::info("Created user '{}' with ID {}", username, user_id);
    
    return Result<int64_t>::Ok(user_id);
}

Result<std::string> UserManager::GetOnvifPassword(const std::string& username) {
    // First get user ID
    auto user_result = GetUserByUsername(username);
    if (!user_result.success) {
        return Result<std::string>::Err("User not found");
    }
    
    const char* select_sql = "SELECT password_obfuscated FROM onvif_wsse_passwords WHERE user_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(impl_->db_, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<std::string>::Err("Failed to prepare GetOnvifPassword query");
    }
    
    sqlite3_bind_int64(stmt, 1, user_result.value.id);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        spdlog::warn("WSSE password not found for user '{}' — user must change password to generate WSSE entry", username);
        return Result<std::string>::Err("WSSE password not found for user");
    }
    
    const char* obf_hex = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string plaintext;
    
    if (obf_hex) {
        plaintext = WsseDecrypt(std::string(obf_hex));
    }
    
    sqlite3_finalize(stmt);
    
    if (plaintext.empty()) {
        return Result<std::string>::Err("Empty password stored");
    }
    
    return Result<std::string>::Ok(plaintext);
}

Result<void> UserManager::UpdateUser(int64_t user_id, const std::optional<std::string>& username,
                                    const std::optional<UserRole>& role,
                                    const std::optional<bool>& enabled) {
    // Check if user exists
    auto user_result = GetUser(user_id);
    if (!user_result.success) {
        return Result<void>::Err("User not found");
    }
    
    std::string sql = "UPDATE users SET ";
    std::vector<std::string> updates;
    
    if (username.has_value()) {
        updates.push_back("username = ?");
    }
    if (role.has_value()) {
        updates.push_back("role = ?");
    }
    if (enabled.has_value()) {
        updates.push_back("enabled = ?");
    }
    
    if (updates.empty()) {
        return Result<void>::Err("No fields to update");
    }
    
    sql += updates[0];
    for (size_t i = 1; i < updates.size(); ++i) {
        sql += ", " + updates[i];
    }
    sql += " WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to prepare update user query");
    }
    
    int param_idx = 1;
    if (username.has_value()) {
        sqlite3_bind_text(stmt, param_idx++, username->c_str(), -1, SQLITE_TRANSIENT);
    }
    if (role.has_value()) {
        sqlite3_bind_int(stmt, param_idx++, static_cast<int>(*role));
    }
    if (enabled.has_value()) {
        sqlite3_bind_int(stmt, param_idx++, *enabled ? 1 : 0);
    }
    sqlite3_bind_int64(stmt, param_idx, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<void>::Err("Failed to update user");
    }
    
    sqlite3_finalize(stmt);
    
    spdlog::info("Updated user ID {}", user_id);
    
    return Result<void>::Ok();
}

Result<void> UserManager::DeleteUser(int64_t user_id) {
    // Don't allow deleting the last admin
    auto users_result = ListUsers();
    if (users_result.success) {
        auto users = users_result.value;
        int admin_count = std::count_if(users.begin(), users.end(), 
            [](const User& u) { return u.role == UserRole::ADMIN; });
        
        auto user_result = GetUser(user_id);
        if (user_result.success && user_result.value.role == UserRole::ADMIN && admin_count <= 1) {
            return Result<void>::Err("Cannot delete the last admin user");
        }
    }
    
    const char* delete_sql = "DELETE FROM users WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, delete_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to prepare delete user query");
    }
    
    sqlite3_bind_int64(stmt, 1, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<void>::Err("Failed to delete user");
    }
    
    int changes = sqlite3_changes(impl_->db_);
    sqlite3_finalize(stmt);
    
    if (changes == 0) {
        return Result<void>::Err("User not found");
    }
    
    // Delete user sessions
    DeleteUserSessions(user_id);
    
    spdlog::info("Deleted user ID {}", user_id);
    
    return Result<void>::Ok();
}

Result<std::vector<Session>> UserManager::ListActiveSessions() {
    const char* select_sql = R"(
        SELECT token, user_id, username, role, created_at, expires_at, ip_address, user_agent
        FROM sessions WHERE expires_at > ? ORDER BY created_at DESC;
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<std::vector<Session>>::Err("Failed to prepare list sessions query");
    }
    
    sqlite3_bind_int64(stmt, 1, GetCurrentTimestamp());
    
    std::vector<Session> sessions;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Session session;
        session.token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        session.user_id = sqlite3_column_int64(stmt, 1);
        session.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.role = static_cast<UserRole>(sqlite3_column_int(stmt, 3));
        session.created_at = sqlite3_column_int64(stmt, 4);
        session.expires_at = sqlite3_column_int64(stmt, 5);
        
        const char* ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        const char* ua = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        
        if (ip) session.ip_address = ip;
        if (ua) session.user_agent = ua;
        
        sessions.push_back(session);
    }
    
    sqlite3_finalize(stmt);
    
    return Result<std::vector<Session>>::Ok(sessions);
}

Result<void> UserManager::DeleteSession(const std::string& token) {
    return Logout(token);
}

Result<void> UserManager::DeleteUserSessions(int64_t user_id) {
    const char* delete_sql = "DELETE FROM sessions WHERE user_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, delete_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to prepare delete user sessions query");
    }
    
    sqlite3_bind_int64(stmt, 1, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<void>::Err("Failed to delete user sessions");
    }
    
    sqlite3_finalize(stmt);
    
    return Result<void>::Ok();
}

Result<void> UserManager::CleanupExpiredSessions() {
    const char* delete_sql = "DELETE FROM sessions WHERE expires_at < ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, delete_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to prepare cleanup sessions query");
    }
    
    sqlite3_bind_int64(stmt, 1, GetCurrentTimestamp());
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return Result<void>::Err("Failed to cleanup expired sessions");
    }
    
    int changes = sqlite3_changes(impl_->db_);
    sqlite3_finalize(stmt);
    
    if (changes > 0) {
        spdlog::debug("Cleaned up {} expired sessions", changes);
    }
    
    return Result<void>::Ok();
}

// ==================== Security Questions Implementation ====================

std::vector<SecurityQuestion> UserManager::GetAvailableSecurityQuestions() {
    // Predefined list of 5 security questions for password recovery
    return {
        {1, "What is the name of your first pet?"},
        {2, "What city were you born in?"},
        {3, "What was your childhood nickname?"},
        {4, "What is the name of your favorite childhood friend?"},
        {5, "What was the make of your first car?"}
    };
}

Result<void> UserManager::SetSecurityQuestions(int64_t user_id, 
                                                const std::vector<std::pair<int, std::string>>& qa_pairs) {
    // Validate: minimum 1, maximum 5 questions
    if (qa_pairs.empty()) {
        return Result<void>::Err("At least 1 security question is required");
    }
    if (qa_pairs.size() > 5) {
        return Result<void>::Err("Maximum 5 security questions allowed");
    }
    
    // Validate question IDs
    auto available = GetAvailableSecurityQuestions();
    std::set<int> valid_ids;
    for (const auto& q : available) {
        valid_ids.insert(q.id);
    }
    
    std::set<int> used_ids;
    for (const auto& [qid, answer] : qa_pairs) {
        if (valid_ids.find(qid) == valid_ids.end()) {
            return Result<void>::Err("Invalid question ID: " + std::to_string(qid));
        }
        if (used_ids.find(qid) != used_ids.end()) {
            return Result<void>::Err("Duplicate question ID: " + std::to_string(qid));
        }
        if (answer.length() < 2) {
            return Result<void>::Err("Answer must be at least 2 characters");
        }
        used_ids.insert(qid);
    }
    
    // Begin transaction
    char* err_msg = nullptr;
    if (sqlite3_exec(impl_->db_, "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        return Result<void>::Err("Failed to begin transaction: " + error);
    }
    
    // Delete existing security questions for this user
    const char* delete_sql = "DELETE FROM user_security_questions WHERE user_id = ?;";
    sqlite3_stmt* delete_stmt;
    if (sqlite3_prepare_v2(impl_->db_, delete_sql, -1, &delete_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(impl_->db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return Result<void>::Err("Failed to prepare delete query");
    }
    sqlite3_bind_int64(delete_stmt, 1, user_id);
    sqlite3_step(delete_stmt);
    sqlite3_finalize(delete_stmt);
    
    // Insert new security questions
    const char* insert_sql = R"(
        INSERT INTO user_security_questions (user_id, question_id, answer_hash, created_at)
        VALUES (?, ?, ?, ?);
    )";
    
    int64_t now = GetCurrentTimestamp();
    
    for (const auto& [question_id, answer] : qa_pairs) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(impl_->db_, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_exec(impl_->db_, "ROLLBACK;", nullptr, nullptr, nullptr);
            return Result<void>::Err("Failed to prepare insert query");
        }
        
        // Hash the answer (case-insensitive, trimmed)
        std::string normalized_answer = answer;
        std::transform(normalized_answer.begin(), normalized_answer.end(), 
                       normalized_answer.begin(), ::tolower);
        // Trim whitespace
        normalized_answer.erase(0, normalized_answer.find_first_not_of(" \t\n\r"));
        normalized_answer.erase(normalized_answer.find_last_not_of(" \t\n\r") + 1);
        
        std::string answer_hash = HashPassword(normalized_answer);
        
        sqlite3_bind_int64(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, question_id);
        sqlite3_bind_text(stmt, 3, answer_hash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 4, now);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            sqlite3_exec(impl_->db_, "ROLLBACK;", nullptr, nullptr, nullptr);
            return Result<void>::Err("Failed to insert security question");
        }
        sqlite3_finalize(stmt);
    }
    
    // Commit transaction
    if (sqlite3_exec(impl_->db_, "COMMIT;", nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        sqlite3_exec(impl_->db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return Result<void>::Err("Failed to commit transaction: " + error);
    }
    
    spdlog::info("Security questions set for user ID {}", user_id);
    return Result<void>::Ok();
}

Result<std::vector<SecurityQuestion>> UserManager::GetUserSecurityQuestions(const std::string& username) {
    // First get user ID
    auto user_result = GetUserByUsername(username);
    if (!user_result.success) {
        return Result<std::vector<SecurityQuestion>>::Err("User not found");
    }
    
    int64_t user_id = user_result.value.id;
    
    // Get question IDs for this user
    const char* select_sql = "SELECT question_id FROM user_security_questions WHERE user_id = ? ORDER BY question_id;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<std::vector<SecurityQuestion>>::Err("Failed to prepare query");
    }
    
    sqlite3_bind_int64(stmt, 1, user_id);
    
    std::vector<int> question_ids;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        question_ids.push_back(sqlite3_column_int(stmt, 0));
    }
    sqlite3_finalize(stmt);
    
    if (question_ids.empty()) {
        return Result<std::vector<SecurityQuestion>>::Err("No security questions set for this user");
    }
    
    // Map question IDs to questions
    auto available = GetAvailableSecurityQuestions();
    std::map<int, std::string> id_to_question;
    for (const auto& q : available) {
        id_to_question[q.id] = q.question;
    }
    
    std::vector<SecurityQuestion> result;
    for (int qid : question_ids) {
        if (id_to_question.count(qid)) {
            result.push_back({qid, id_to_question[qid]});
        }
    }
    
    return Result<std::vector<SecurityQuestion>>::Ok(result);
}

Result<std::string> UserManager::VerifySecurityAnswers(const std::string& username,
                                                        const std::vector<std::pair<int, std::string>>& answers) {
    // Get user
    auto user_result = GetUserByUsername(username);
    if (!user_result.success) {
        return Result<std::string>::Err("User not found");
    }
    
    int64_t user_id = user_result.value.id;
    
    // Check if user is enabled
    if (!user_result.value.enabled) {
        return Result<std::string>::Err("User account is disabled");
    }
    
    // Get stored security questions and answers
    const char* select_sql = "SELECT question_id, answer_hash FROM user_security_questions WHERE user_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<std::string>::Err("Failed to prepare query");
    }
    
    sqlite3_bind_int64(stmt, 1, user_id);
    
    std::map<int, std::string> stored_answers;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int qid = sqlite3_column_int(stmt, 0);
        const char* hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (hash) {
            stored_answers[qid] = hash;
        }
    }
    sqlite3_finalize(stmt);
    
    if (stored_answers.empty()) {
        return Result<std::string>::Err("No security questions set for this user");
    }
    
    // Verify all provided answers
    int correct_count = 0;
    for (const auto& [question_id, answer] : answers) {
        if (stored_answers.find(question_id) == stored_answers.end()) {
            continue;  // Question not found, skip
        }
        
        // Normalize answer
        std::string normalized_answer = answer;
        std::transform(normalized_answer.begin(), normalized_answer.end(), 
                       normalized_answer.begin(), ::tolower);
        normalized_answer.erase(0, normalized_answer.find_first_not_of(" \t\n\r"));
        normalized_answer.erase(normalized_answer.find_last_not_of(" \t\n\r") + 1);
        
        if (VerifyPassword(normalized_answer, stored_answers[question_id])) {
            correct_count++;
        }
    }
    
    // Count how many of the provided answers are for questions the user actually set up
    int valid_provided = 0;
    for (const auto& [question_id, answer] : answers) {
        if (stored_answers.find(question_id) != stored_answers.end()) {
            valid_provided++;
        }
    }
    
    // Require at least 1 valid answer to be provided
    if (valid_provided == 0) {
        spdlog::warn("User '{}' provided answers for questions they didn't set up", username);
        return Result<std::string>::Err("None of the provided questions match your setup");
    }
    
    // ALL provided answers (that match setup questions) must be correct
    if (correct_count != valid_provided) {
        spdlog::warn("Security answer verification failed for user '{}' ({}/{} correct)", 
                     username, correct_count, valid_provided);
        return Result<std::string>::Err("One or more security answers are incorrect");
    }
    
    // Generate password reset token
    std::string token = GenerateResetToken();
    int64_t now = GetCurrentTimestamp();
    int64_t expires_at = now + 15 * 60;  // 15 minutes expiry
    
    // Store the token
    const char* insert_sql = R"(
        INSERT INTO password_reset_tokens (token, user_id, created_at, expires_at, used)
        VALUES (?, ?, ?, ?, 0);
    )";
    
    sqlite3_stmt* insert_stmt;
    if (sqlite3_prepare_v2(impl_->db_, insert_sql, -1, &insert_stmt, nullptr) != SQLITE_OK) {
        return Result<std::string>::Err("Failed to create reset token");
    }
    
    sqlite3_bind_text(insert_stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(insert_stmt, 2, user_id);
    sqlite3_bind_int64(insert_stmt, 3, now);
    sqlite3_bind_int64(insert_stmt, 4, expires_at);
    
    if (sqlite3_step(insert_stmt) != SQLITE_DONE) {
        sqlite3_finalize(insert_stmt);
        return Result<std::string>::Err("Failed to store reset token");
    }
    sqlite3_finalize(insert_stmt);
    
    spdlog::info("Password reset token generated for user '{}'", username);
    
    return Result<std::string>::Ok(token);
}

Result<void> UserManager::ResetPasswordWithToken(const std::string& token, const std::string& new_password) {
    // Validate token
    const char* select_sql = R"(
        SELECT user_id, expires_at, used FROM password_reset_tokens WHERE token = ?;
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to validate token");
    }
    
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return Result<void>::Err("Invalid or expired reset token");
    }
    
    int64_t user_id = sqlite3_column_int64(stmt, 0);
    int64_t expires_at = sqlite3_column_int64(stmt, 1);
    int used = sqlite3_column_int(stmt, 2);
    sqlite3_finalize(stmt);
    
    int64_t now = GetCurrentTimestamp();
    
    if (used) {
        return Result<void>::Err("Reset token has already been used");
    }
    
    if (now > expires_at) {
        return Result<void>::Err("Reset token has expired");
    }
    
    // Validate new password
    if (new_password.length() < 6) {
        return Result<void>::Err("Password must be at least 6 characters");
    }
    
    // Mark token as used
    const char* update_token_sql = "UPDATE password_reset_tokens SET used = 1 WHERE token = ?;";
    sqlite3_stmt* update_stmt;
    if (sqlite3_prepare_v2(impl_->db_, update_token_sql, -1, &update_stmt, nullptr) != SQLITE_OK) {
        return Result<void>::Err("Failed to update token");
    }
    sqlite3_bind_text(update_stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(update_stmt);
    sqlite3_finalize(update_stmt);
    
    // Reset the password
    return ResetPassword(user_id, new_password);
}

bool UserManager::HasSecurityQuestionsSet(int64_t user_id) {
    const char* count_sql = "SELECT COUNT(*) FROM user_security_questions WHERE user_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(impl_->db_, count_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int64(stmt, 1, user_id);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    return count >= 1;  // Only 1 security question required now
}

// Helper functions
std::string UserRoleToString(UserRole role) {
    switch (role) {
        case UserRole::ADMIN: return "admin";
        case UserRole::OPERATOR: return "operator";
        case UserRole::VIEWER: return "viewer";
        default: return "unknown";
    }
}

std::optional<UserRole> StringToUserRole(const std::string& role_str) {
    if (role_str == "admin") return UserRole::ADMIN;
    if (role_str == "operator") return UserRole::OPERATOR;
    if (role_str == "viewer") return UserRole::VIEWER;
    return std::nullopt;
}

} // namespace config
} // namespace ipcam
