#include "ipcam/storage.h"
#include "ipcam/credential_manager.h"
#include "ipcam/config.h"
#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <atomic>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>

namespace ipcam {
namespace storage {

// Key derivation constants
// Legacy salt kept for backward-compatible DB migration
constexpr const char* LEGACY_KEY_DERIVATION_SALT = "ipcam_nvt_2024";
constexpr int KEY_DERIVATION_ITERATIONS = 100000;

// ============================================================================
// Key Management Strategy
// ============================================================================
// CURRENT: MAC address-based key derivation (suitable for development)
// FUTURE: Migrate to OP-TEE when ready for production
// 
// OP-TEE Migration Path:
// 1. Create Trusted Application in BSP/optee_os/ta/storage_ta/
// 2. Replace GetDeviceId() with TEEC_InvokeCommand(CMD_GET_DEVICE_KEY)
// 3. Store master keys in TEE secure storage (TEE_STORAGE_PRIVATE)
// 4. Keys never leave secure world (TrustZone protection)
//
// Optional Enhancements:
// - Combine with SoC eFuse/OTP for hardware-unique binding
// - Add ATECC608 secure element for certificate storage
// - Implement secure key provisioning during manufacturing
//
// Security Level:
// - Current (Persisted ID): Medium (development/testing acceptable)
// - OP-TEE: Military-grade (production ready)
// ============================================================================

// Persisted device ID file path
constexpr const char* DEVICE_ID_FILE = "/mnt/app/.device_id";
constexpr const char* DEVICE_SALT_FILE = "/mnt/app/.device_salt";

// Get or generate per-device random salt (persisted at first boot)
static std::string GetDeviceSalt() {
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

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        ss << std::setw(2) << static_cast<int>(random_bytes[i]);
    }
    std::string salt = ss.str();

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

// Generate a cryptographically random device ID
static std::string GenerateDeviceId() {
    unsigned char random_bytes[16];
    if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
        std::cerr << "[Storage] CRITICAL: RAND_bytes() failed — cannot generate secure device ID" << std::endl;
        throw std::runtime_error("Cryptographic random number generation failed");
    }
    
    // Format as UUID-like string: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) ss << '-';
        ss << std::setw(2) << static_cast<int>(random_bytes[i]);
    }
    return ss.str();
}

// Get device unique ID (persisted on first boot)
// This ensures the same key is derived across reboots even if MAC changes.
// TODO: Replace with OP-TEE call when secure enclave is enabled
static std::string GetDeviceId() {
    // Try to read existing device ID
    std::ifstream id_file(DEVICE_ID_FILE);
    if (id_file.is_open()) {
        std::string device_id;
        std::getline(id_file, device_id);
        if (!device_id.empty() && device_id.length() >= 32) {
            return device_id;
        }
    }
    
    // Generate and persist new device ID
    std::string new_id = GenerateDeviceId();
    
    // Ensure directory exists
    std::filesystem::path id_path(DEVICE_ID_FILE);
    if (!std::filesystem::exists(id_path.parent_path())) {
        try {
            std::filesystem::create_directories(id_path.parent_path());
        } catch (const std::exception& e) {
            std::cerr << "[Storage] Warning: Cannot create directory for device ID: " << e.what() << std::endl;
        }
    }
    
    // Write device ID with restrictive permissions
    std::ofstream out_file(DEVICE_ID_FILE);
    if (out_file.is_open()) {
        out_file << new_id;
        out_file.close();
        // Set file permissions to read-only for root (0400)
        std::filesystem::permissions(DEVICE_ID_FILE, 
            std::filesystem::perms::owner_read,
            std::filesystem::perm_options::replace);
        std::cout << "[Storage] Generated new device ID: " << new_id.substr(0, 8) << "..." << std::endl;
    } else {
        std::cerr << "[Storage] Warning: Cannot persist device ID, using volatile ID" << std::endl;
    }
    
    return new_id;
}

// Derive encryption key from device ID using PBKDF2
static std::string DeriveKeyFromDevice(const std::string& purpose) {
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
    
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key[i]);
    }
    return ss.str();
}

// Legacy key derivation using hardcoded salt (for migration of existing DBs)
static std::string DeriveKeyFromDeviceLegacy(const std::string& purpose) {
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
    
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key[i]);
    }
    return ss.str();
}

// Database-specific keys
// Encryption key for users.db
static const std::string GetUserDbKey() { return DeriveKeyFromDevice("users"); }
static const std::string GetSecureDbKey() { return DeriveKeyFromDevice("secure"); }
static const std::string GetRecordingsDbKey() { return DeriveKeyFromDevice("recordings"); }

// Global shutdown flag
static std::atomic<bool> g_shutdown_in_progress{false};

void dummy_storage() { 
    std::cout << "[Storage] Modern SQLCipher storage with device-unique keys\n"; 
}

// Helper to get DB path from config with /mnt/app fallback
static std::string GetDbPath(const char* config_key, const char* default_name) {
    std::string path = config::Get<std::string>(config_key, "");
    return path.empty() ? (std::string("/mnt/app/") + default_name) : path;
}

// Check if a path is on SD card (starts with /mnt/sd)
static bool IsPathOnSdCard(const std::string& path) {
    return path.find("/mnt/sd") == 0;
}

// Check if SD card is mounted
static bool IsSdCardMounted() {
    // Check if /mnt/sd is a mount point
    std::ifstream mounts("/proc/mounts");
    if (!mounts.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(mounts, line)) {
        if (line.find("/mnt/sd") != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Check if storage for a database is available
static bool IsStorageAvailable(const std::string& db_path) {
    if (IsPathOnSdCard(db_path)) {
        return IsSdCardMounted();
    }
    // NAND storage is always available if /mnt/app is mounted
    return true;
}

// ============================================================================
// Database Implementation
// ============================================================================

Database::Database(const std::string& db_path, const std::string& encryption_key)
    : db_(nullptr), db_path_(db_path)
    , encryption_key_(encryption_key)
    , is_open_(false) {}

Database::~Database() {
    // During global shutdown, sqlite3_close() can crash because the SQLite
    // library may have already been cleaned up. Just leak the handle instead.
    // This is safe because the process is exiting anyway.
    if (!g_shutdown_in_progress && db_) {
        sqlite3_close(db_);
    }
    db_ = nullptr;
    is_open_ = false;
}

Database::Database(Database&& other) noexcept
    : db_(other.db_), db_path_(std::move(other.db_path_))
    , encryption_key_(std::move(other.encryption_key_)), is_open_(other.is_open_) {
    other.db_ = nullptr;
    other.is_open_ = false;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        Close();
        db_ = other.db_;
        db_path_ = std::move(other.db_path_);
        encryption_key_ = std::move(other.encryption_key_);
        is_open_ = other.is_open_;
        other.db_ = nullptr;
        other.is_open_ = false;
    }
    return *this;
}

bool Database::Open() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (is_open_) return true;

    // Check if database is new (before opening)
    bool is_new_db = !std::filesystem::exists(db_path_);
    if (is_new_db) {
        std::cout << "[Database] Creating new database: " << db_path_ << std::endl;
    }

    std::filesystem::path dbDir = std::filesystem::path(db_path_).parent_path();
    if (!dbDir.empty() && !std::filesystem::exists(dbDir)) {
        try {
            std::filesystem::create_directories(dbDir);
            std::cout << "[Database] Created directory: " << dbDir << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Database Error] Create dir failed: " << e.what() << std::endl;
            return false;
        }
    }

    if (sqlite3_open(db_path_.c_str(), &db_) != SQLITE_OK) {
        LogError("Open failed");
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    if (!SetEncryptionKey(is_new_db)) {
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    // Set busy timeout to prevent indefinite blocking (5 seconds)
    sqlite3_busy_timeout(db_, 5000);
    
    // Standard SQLCipher usage - no special pragmas needed with file locking enabled
    is_open_ = true;
    std::cout << "[Database] Opened (SQLCipher): " << db_path_ << std::endl;
    std::cout.flush();
    return true;
}

void Database::Close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
        is_open_ = false;
    }
}

bool Database::IsOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return is_open_;
}

sqlite3* Database::GetHandle() const { return db_; }

// Internal execute without locking (for use when mutex is already held)
bool Database::ExecuteUnlocked(const std::string& sql) {
    if (!is_open_) return false;

    char* err = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB Error] " << (err ? err : "unknown") << std::endl;
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

bool Database::Execute(const std::string& sql) {
    std::lock_guard<std::mutex> lock(mutex_);
    return ExecuteUnlocked(sql);
}

bool Database::ExecuteWithParams(const std::string& sql, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_open_) return false;

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        LogError("Prepare failed");
        return false;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        if (sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool Database::Query(const std::string& sql, 
                     std::function<void(const std::unordered_map<std::string, std::string>&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_open_) return false;

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    int cols = sqlite3_column_count(stmt);
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::unordered_map<std::string, std::string> row;
        for (int i = 0; i < cols; ++i) {
            const char* name = sqlite3_column_name(stmt, i);
            const unsigned char* text = sqlite3_column_text(stmt, i);
            row[name] = text ? reinterpret_cast<const char*>(text) : "";
        }
        callback(row);
    }

    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

std::string Database::GetLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return db_ ? sqlite3_errmsg(db_) : "DB not open";
}

bool Database::BeginTransaction() { return Execute("BEGIN TRANSACTION;"); }
bool Database::Commit() { return Execute("COMMIT;"); }
bool Database::Rollback() { return Execute("ROLLBACK;"); }

int64_t Database::GetLastInsertRowId() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return db_ ? sqlite3_last_insert_rowid(db_) : -1;
}

bool Database::SetEncryptionKey(bool is_new_db) {
    if (encryption_key_.empty()) {
        std::cout << "[DB] Warning: No encryption" << std::endl;
        return true;
    }

    if (is_new_db) {
        std::cout << "[DB] Initializing new database with encryption" << std::endl;
    }

    // Set the encryption key
    std::string pragma = "PRAGMA key = \"" + encryption_key_ + "\";";
    char* err = nullptr;
    if (sqlite3_exec(db_, pragma.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "[DB] Set key failed: " << (err ? err : "unknown") << std::endl;
        if (err) sqlite3_free(err);
        return false;
    }

    // Verify encryption by querying sqlite_master
    // For new databases, this will initialize the encrypted database structure
    if (sqlite3_exec(db_, "SELECT count(*) FROM sqlite_master;", nullptr, nullptr, &err) != SQLITE_OK) {
        // If this is an existing database with wrong key, try legacy key migration
        if (!is_new_db) {
            if (err) sqlite3_free(err);
            err = nullptr;

            // Try legacy key (hardcoded salt)
            std::string purpose;
            // Extract purpose from current key to derive legacy equivalent
            // We must re-open the DB to reset the cipher state
            std::string db_path = db_path_;
            sqlite3_close(db_);
            db_ = nullptr;
            
            if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
                std::cerr << "[DB] Failed to reopen for legacy key migration" << std::endl;
                return false;
            }

            // Derive all possible legacy keys and try them
            const char* purposes[] = {"users", "secure", "recordings"};
            bool legacy_ok = false;
            for (const char* p : purposes) {
                std::string legacy_key = DeriveKeyFromDeviceLegacy(p);
                std::string new_key = DeriveKeyFromDevice(p);
                if (legacy_key == encryption_key_ || new_key == encryption_key_) {
                    // This is likely the right purpose
                    std::string legacy_pragma = "PRAGMA key = \"" + legacy_key + "\";";
                    sqlite3_exec(db_, legacy_pragma.c_str(), nullptr, nullptr, nullptr);
                    if (sqlite3_exec(db_, "SELECT count(*) FROM sqlite_master;", nullptr, nullptr, &err) == SQLITE_OK) {
                        std::cout << "[DB] Legacy key works — migrating to per-device salt" << std::endl;
                        std::string rekey_pragma = "PRAGMA rekey = \"" + encryption_key_ + "\";";
                        sqlite3_exec(db_, rekey_pragma.c_str(), nullptr, nullptr, nullptr);
                        legacy_ok = true;
                        break;
                    }
                    if (err) { sqlite3_free(err); err = nullptr; }
                    // Reset for next attempt
                    sqlite3_close(db_);
                    db_ = nullptr;
                    sqlite3_open(db_path.c_str(), &db_);
                }
            }

            if (!legacy_ok) {
                std::cerr << "[DB] Verify encryption failed - wrong key or corrupted database" << std::endl;
                return false;
            }
        } else {
            // For new database, try to initialize it
            if (err) sqlite3_free(err);
            
            // Force database initialization by creating a dummy table and dropping it
            if (sqlite3_exec(db_, "CREATE TABLE IF NOT EXISTS _init (id INTEGER); DROP TABLE IF EXISTS _init;", nullptr, nullptr, &err) != SQLITE_OK) {
                std::cerr << "[DB] Failed to initialize new encrypted database" << std::endl;
                if (err) {
                    std::cerr << "[DB] Error: " << err << std::endl;
                    sqlite3_free(err);
                }
                return false;
            }
        }
    }

    std::cout << "[DB] SQLCipher encryption " << (is_new_db ? "initialized" : "verified") << std::endl;
    std::cout.flush();
    return true;
}

bool Database::RotateKey(const std::string& new_key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !is_open_) {
        std::cerr << "[DB] Cannot rotate key: database not open" << std::endl;
        return false;
    }

    if (new_key.empty()) {
        std::cerr << "[DB] Cannot rotate to empty key" << std::endl;
        return false;
    }

    std::cout << "[DB] Rotating encryption key..." << std::endl;
    
    // Use SQLCipher's PRAGMA rekey to re-encrypt with new key
    std::string pragma = "PRAGMA rekey = \"" + new_key + "\";";
    char* err = nullptr;
    if (sqlite3_exec(db_, pragma.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "[DB] Key rotation failed: " << (err ? err : "unknown") << std::endl;
        if (err) sqlite3_free(err);
        return false;
    }

    // Update stored key
    encryption_key_ = new_key;
    
    std::cout << "[DB] Key rotation successful" << std::endl;
    return true;
}

void Database::LogError(const std::string& ctx) const {
    std::cerr << "[DB Error] " << ctx << ": " << (db_ ? sqlite3_errmsg(db_) : "not init") << std::endl;
}

// ============================================================================
// Statement Implementation
// ============================================================================

Statement::Statement(Database& db, const std::string& sql)
    : stmt_(nullptr), db_(db), is_valid_(false) {
    is_valid_ = (sqlite3_prepare_v2(db_.GetHandle(), sql.c_str(), -1, &stmt_, nullptr) == SQLITE_OK);
}

Statement::~Statement() { if (stmt_) sqlite3_finalize(stmt_); }

Statement::Statement(Statement&& other) noexcept
    : stmt_(other.stmt_), db_(other.db_), is_valid_(other.is_valid_) {
    other.stmt_ = nullptr;
    other.is_valid_ = false;
}

Statement& Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        if (stmt_) sqlite3_finalize(stmt_);
        stmt_ = other.stmt_;
        is_valid_ = other.is_valid_;
        other.stmt_ = nullptr;
        other.is_valid_ = false;
    }
    return *this;
}

bool Statement::BindInt(int idx, int val) {
    return is_valid_ && (sqlite3_bind_int(stmt_, idx, val) == SQLITE_OK);
}

bool Statement::BindInt64(int idx, int64_t val) {
    return is_valid_ && (sqlite3_bind_int64(stmt_, idx, val) == SQLITE_OK);
}

bool Statement::BindDouble(int idx, double val) {
    return is_valid_ && (sqlite3_bind_double(stmt_, idx, val) == SQLITE_OK);
}

bool Statement::BindText(int idx, const std::string& val) {
    return is_valid_ && (sqlite3_bind_text(stmt_, idx, val.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK);
}

bool Statement::BindNull(int idx) {
    return is_valid_ && (sqlite3_bind_null(stmt_, idx) == SQLITE_OK);
}

bool Statement::Execute() {
    return is_valid_ && (sqlite3_step(stmt_) == SQLITE_DONE);
}

bool Statement::Step() {
    return is_valid_ && (sqlite3_step(stmt_) == SQLITE_ROW);
}

int Statement::GetInt(int col) const { return sqlite3_column_int(stmt_, col); }
int64_t Statement::GetInt64(int col) const { return sqlite3_column_int64(stmt_, col); }
double Statement::GetDouble(int col) const { return sqlite3_column_double(stmt_, col); }

std::string Statement::GetText(int col) const {
    const unsigned char* t = sqlite3_column_text(stmt_, col);
    return t ? reinterpret_cast<const char*>(t) : "";
}

bool Statement::Reset() { return is_valid_ && (sqlite3_reset(stmt_) == SQLITE_OK); }
int Statement::GetColumnCount() const { return sqlite3_column_count(stmt_); }

std::string Statement::GetColumnName(int col) const {
    const char* n = sqlite3_column_name(stmt_, col);
    return n ? n : "";
}

void Statement::LogError(const std::string& ctx) const {
    std::cerr << "[Stmt Error] " << ctx << ": " << db_.GetLastError() << std::endl;
}

// ============================================================================
// UserManager Implementation
// ============================================================================

UserManager::UserManager() {}
UserManager::~UserManager() {}

bool UserManager::Initialize() {
    std::cout << "[UserManager] Starting initialization..." << std::endl;
    std::cout.flush();
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Unified user database path (same as config::UserManager)
    std::string path = GetDbPath("storage.database.user_management.path", "users.db");
    std::cout << "[UserManager] Opening database: " << path << std::endl;
    std::cout.flush();
    db_ = std::make_unique<Database>(path, GetUserDbKey());
    
    std::cout << "[UserManager] Calling db_->Open()..." << std::endl;
    std::cout.flush();
    if (!db_->Open() || !CreateTables()) {
        std::cerr << "[UserManager] Init failed" << std::endl;
        return false;
    }

    // Default user creation removed - config::UserManager handles this with correct schema
    /*
    std::cout << "[UserManager] Checking for admin user..." << std::endl;
    std::cout.flush();
    
    // Use unlocked versions since we already hold the mutex
    if (!UserExistsUnlocked("admin")) {
        // ... code removed ...
    }
    */

    std::cout << "[UserManager] Initialized: " << path << std::endl;
    std::cout.flush();
    return true;
}

// Internal unlocked helper (assumes mutex is already held)
bool UserManager::RegisterUserUnlocked(const std::string& username, const std::string& password, int level) {
    if (!db_ || !db_->IsOpen()) return false;

    Statement check(*db_, "SELECT 1 FROM users WHERE username = ? LIMIT 1");
    check.BindText(1, username);
    if (check.Step()) {
        std::cerr << "[UserManager] User exists: " << username << std::endl;
        return false;
    }

    std::string hashed = HashPassword(password);
    Statement insert(*db_, 
        "INSERT INTO users (username, password, userlevel, securityans, securityque) "
        "VALUES (?, ?, ?, 'SVAtQ2FtZXJh', 'Ques1')");
    
    insert.BindText(1, username);
    insert.BindText(2, hashed);
    insert.BindInt(3, level);
    
    if (!insert.Execute()) {
        std::cerr << "[UserManager] Register failed" << std::endl;
        return false;
    }

    std::cout << "[UserManager] Registered: " << username << std::endl;
    return true;
}

bool UserManager::RegisterUser(const std::string& username, const std::string& password, int level) {
    std::lock_guard<std::mutex> lock(mutex_);
    return RegisterUserUnlocked(username, password, level);
}

std::optional<int> UserManager::AuthenticateUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return std::nullopt;

    std::string hashed = HashPassword(password);
    Statement stmt(*db_, "SELECT userlevel FROM users WHERE username = ? AND password = ?");
    stmt.BindText(1, username);
    stmt.BindText(2, hashed);
    
    return stmt.Step() ? std::optional<int>(stmt.GetInt(0)) : std::nullopt;
}

// Internal unlocked helper (assumes mutex is already held)
bool UserManager::UserExistsUnlocked(const std::string& username) {
    if (!db_ || !db_->IsOpen()) return false;
    Statement stmt(*db_, "SELECT 1 FROM users WHERE username = ? LIMIT 1");
    stmt.BindText(1, username);
    return stmt.Step();
}

bool UserManager::UserExists(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    return UserExistsUnlocked(username);
}

bool UserManager::DeleteUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;
    Statement stmt(*db_, "DELETE FROM users WHERE username = ?");
    stmt.BindText(1, username);
    return stmt.Execute();
}

bool UserManager::UpdatePassword(const std::string& username, const std::string& new_password) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;

    std::string hashed = HashPassword(new_password);
    Statement stmt(*db_, "UPDATE users SET password = ? WHERE username = ?");
    stmt.BindText(1, hashed);
    stmt.BindText(2, username);
    return stmt.Execute();
}

std::string UserManager::GetDatabasePath() const {
    // Unified with config::UserManager
    return GetDbPath("storage.database.user_management.path", "users.db");
}

bool UserManager::CreateTables() {
    const std::vector<std::string> tables = {
        // REMOVED: Legacy users table (managed by config::UserManager with new schema)
        /*
        "CREATE TABLE IF NOT EXISTS users ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, "
        "password CHAR(64) NOT NULL, userlevel INTEGER NOT NULL, "
        "securityans CHAR(64) DEFAULT 'SVAtQ2FtZXJh', securityque TEXT DEFAULT 'Ques1');",
        
        "CREATE TABLE IF NOT EXISTS session_history ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, session_id CHAR(64) UNIQUE NOT NULL, "
        "ip_address TEXT NOT NULL, session_key CHAR(64) NOT NULL, "
        "initialization_vector CHAR(64) NOT NULL, username TEXT NOT NULL, expiry_time INTEGER NOT NULL);",
        
        "CREATE TABLE IF NOT EXISTS IPInfo ("
        "ipAddress TEXT PRIMARY KEY UNIQUE NOT NULL, attempts INTEGER DEFAULT 0, lockoutEndTime INTEGER DEFAULT 0);",
        */
        
        "CREATE TABLE IF NOT EXISTS analytics_modules ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE, type TEXT, active INTEGER CHECK (active IN (0, 1)));",
        
        "CREATE TABLE IF NOT EXISTS camera_tampering_module ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, behaviour TEXT UNIQUE, active INTEGER CHECK (active IN (0, 1)), "
        "sensitivity INTEGER NOT NULL CHECK(sensitivity BETWEEN 0 AND 100), "
        "alarm_enable_colour TEXT DEFAULT 'red', alarm_disable_colour TEXT DEFAULT 'red');",
        
        "CREATE TABLE IF NOT EXISTS analytics_rules ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE, type TEXT, module_name TEXT DEFAULT 'NA', "
        "label TEXT DEFAULT 'NA', active INTEGER CHECK (active IN (0, 1)), coordinates TEXT DEFAULT 'NA', "
        "canvas_size TEXT DEFAULT 'NA', sensitivity REAL DEFAULT 0.8, invasion_time REAL DEFAULT 0.1, behaviour TEXT, "
        "alarm_enable_colour TEXT DEFAULT 'red', alarm_disable_colour TEXT DEFAULT 'red');",
        
        "CREATE TABLE IF NOT EXISTS analytics_rules_behaviour ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, behaviour_type TEXT, rule_type TEXT, UNIQUE(behaviour_type, rule_type));",
        
        "CREATE TABLE IF NOT EXISTS analytics_type ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, module_type TEXT UNIQUE, rule_type TEXT, common_type TEXT, "
        "UNIQUE(module_type, rule_type, common_type));",
        
        "CREATE TABLE IF NOT EXISTS labelmap ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, label_id INTEGER UNIQUE, label TEXT COLLATE NOCASE);",
        
        "CREATE TABLE IF NOT EXISTS alert ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, alert_type TEXT COLLATE NOCASE, alert_time TIMESTAMP, "
        "label TEXT COLLATE NOCASE, rule_name TEXT, behaviour TEXT);"
    };
    
    for (const auto& sql : tables) {
        if (!db_->Execute(sql)) return false;
    }
    
    if (!db_->Execute(
        "CREATE TRIGGER IF NOT EXISTS limit_alert_length AFTER INSERT ON alert BEGIN "
        "DELETE FROM alert WHERE ID NOT IN (SELECT ID FROM alert ORDER BY ID DESC LIMIT 200); END;")) {
        return false;
    }
    
    // Insert initial data
    db_->Execute("INSERT OR IGNORE INTO analytics_type (module_type, rule_type, common_type) VALUES "
        "('tt:TrespassingDetectionEngine', 'tt:TrespassingDetectionRuleEngine', 'Trespass'), "
        "('tt:TripWireDetectionEngine', 'tt:TripWireDetectionRuleEngine', 'Tripwire'), "
        "('tt:MotionDetectionEngine', 'tt:MotionDetectionRuleEngine', 'Motion');");
    
    db_->Execute("INSERT OR IGNORE INTO analytics_modules (name, type, active) VALUES "
        "('TripWireDetectionModule', 'tt:TripWireDetectionEngine', 1), "
        "('TrespassingDetectionModule', 'tt:TrespassingDetectionEngine', 1), "
        "('MotionDetectionModule', 'tt:MotionDetectionEngine', 1);");
    
    db_->Execute("INSERT OR IGNORE INTO labelmap (label_id, label) VALUES "
        "(0, 'person'), (1, 'bag'), (2, 'two-wheeler'), (3, 'four-wheeler'), (4, 'animal');");
    
    return true;
}

std::string UserManager::HashPassword(const std::string& password) const {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

// ============================================================================
// RecordingManager Implementation
// ============================================================================

RecordingManager::RecordingManager() : initialized_(false), storage_available_(false) {}
RecordingManager::~RecordingManager() {}

bool RecordingManager::Initialize() {
    std::cout << "[RecordingManager] Starting initialization..." << std::endl;
    std::cout.flush();
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string path = GetDbPath("storage.database.recordings.path", "recordings.db");
    
    // Check if this database is on SD card and if SD card is mounted
    if (IsPathOnSdCard(path)) {
        if (!IsSdCardMounted()) {
            std::cout << "[RecordingManager] SD card not mounted, recording disabled" << std::endl;
            std::cout << "[RecordingManager] Recordings will be available when SD card is inserted" << std::endl;
            initialized_ = true;  // Mark as initialized but without storage
            storage_available_ = false;
            return true;  // Return success - this is expected behavior
        }
        
        // Create the directory structure if it doesn't exist
        std::filesystem::path db_dir = std::filesystem::path(path).parent_path();
        if (!std::filesystem::exists(db_dir)) {
            std::filesystem::create_directories(db_dir);
            std::cout << "[RecordingManager] Created SD card directory: " << db_dir << std::endl;
        }
    }
    
    db_ = std::make_unique<Database>(path, GetRecordingsDbKey());
    
    std::cout << "[RecordingManager] Calling db_->Open()..." << std::endl;
    std::cout.flush();
    if (!db_->Open() || !CreateTables()) {
        std::cerr << "[RecordingManager] Init failed, attempting recovery..." << std::endl;
        
        // Close any partial connection
        db_.reset();
        
        // Try to delete corrupted database and recreate
        if (std::filesystem::exists(path)) {
            std::cerr << "[RecordingManager] Deleting corrupted database: " << path << std::endl;
            try {
                std::filesystem::remove(path);
                // Also remove any journal/wal files
                std::filesystem::remove(path + "-journal");
                std::filesystem::remove(path + "-wal");
                std::filesystem::remove(path + "-shm");
            } catch (const std::exception& e) {
                std::cerr << "[RecordingManager] Failed to delete corrupted db: " << e.what() << std::endl;
            }
        }
        
        // Retry with fresh database
        db_ = std::make_unique<Database>(path, GetRecordingsDbKey());
        if (!db_->Open() || !CreateTables()) {
            std::cerr << "[RecordingManager] Recovery failed, recordings unavailable" << std::endl;
            db_.reset();
            initialized_ = true;
            storage_available_ = false;
            return true;  // Don't crash, just mark storage unavailable
        }
        std::cout << "[RecordingManager] Recovery successful, fresh database created" << std::endl;
    }

    std::cout << "[RecordingManager] Initialized: " << path << std::endl;
    std::cout.flush();
    initialized_ = true;
    storage_available_ = true;
    return true;
}

bool RecordingManager::AddRecording(const std::string& date, const std::string& filename,
                                    int64_t start_time, int64_t end_time,
                                    const std::string& recording_type, const std::string& status) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if storage is available (SD card may not be mounted)
    if (!storage_available_ || !db_ || !db_->IsOpen()) {
        std::cerr << "[RecordingManager] Cannot add recording - storage unavailable" << std::endl;
        return false;
    }

    Statement stmt(*db_, 
        "INSERT INTO recordings (date, filename, start_time, end_time, recording_type, status) "
        "VALUES (?, ?, ?, ?, ?, ?)");
    
    stmt.BindText(1, date);
    stmt.BindText(2, filename);
    stmt.BindInt64(3, start_time);
    stmt.BindInt64(4, end_time);
    stmt.BindText(5, recording_type);
    stmt.BindText(6, status);
    
    return stmt.Execute();
}

std::string RecordingManager::GetDatabasePath() const {
    return GetDbPath("storage.database.recordings.path", "recordings.db");
}

bool RecordingManager::TryReconnectStorage() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Already connected
    if (storage_available_ && db_ && db_->IsOpen()) {
        return true;
    }
    
    std::string path = GetDbPath("storage.database.recordings.path", "recordings.db");
    
    // Only try to reconnect if path is on SD card
    if (!IsPathOnSdCard(path)) {
        return storage_available_;  // Not on SD card, status unchanged
    }
    
    // Check if SD card is now mounted
    if (!IsSdCardMounted()) {
        return false;  // Still not mounted
    }
    
    std::cout << "[RecordingManager] SD card detected, attempting to connect..." << std::endl;
    
    // Create directory if needed
    std::filesystem::path db_dir = std::filesystem::path(path).parent_path();
    if (!std::filesystem::exists(db_dir)) {
        std::filesystem::create_directories(db_dir);
        std::cout << "[RecordingManager] Created SD card directory: " << db_dir << std::endl;
    }
    
    // Try to open the database
    db_ = std::make_unique<Database>(path, GetRecordingsDbKey());
    if (!db_->Open() || !CreateTables()) {
        std::cerr << "[RecordingManager] Failed to open recordings database on SD card" << std::endl;
        db_.reset();
        return false;
    }
    
    storage_available_ = true;
    std::cout << "[RecordingManager] Connected to SD card storage: " << path << std::endl;
    return true;
}

bool RecordingManager::CreateTables() {
    const std::vector<std::string> tables = {
        "CREATE TABLE IF NOT EXISTS recordings ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, date TEXT, filename TEXT, "
        "start_time INTEGER, end_time INTEGER, recording_type TEXT, status TEXT);",
        
        "CREATE TABLE IF NOT EXISTS schedule_slots ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, day_of_week TEXT NOT NULL, slot_number INTEGER NOT NULL, "
        "start_time INTEGER NOT NULL, end_time INTEGER NOT NULL, enabled BOOLEAN DEFAULT 1, "
        "UNIQUE(day_of_week, slot_number));"
    };
    
    for (const auto& sql : tables) {
        if (!db_->Execute(sql)) return false;
    }
    
    // Insert default schedule
    db_->Execute(
        "INSERT OR IGNORE INTO schedule_slots (day_of_week, slot_number, start_time, end_time, enabled) VALUES "
        "('Monday', 1, 100000, 110000, 0), ('Monday', 2, 120000, 130000, 0), "
        "('Tuesday', 1, 100000, 110000, 0), ('Tuesday', 2, 120000, 130000, 0), "
        "('Wednesday', 1, 100000, 110000, 0), ('Wednesday', 2, 120000, 130000, 0), "
        "('Thursday', 1, 100000, 110000, 0), ('Thursday', 2, 120000, 130000, 0), "
        "('Friday', 1, 100000, 110000, 0), ('Friday', 2, 120000, 130000, 0), "
        "('Saturday', 1, 100000, 110000, 0), ('Saturday', 2, 120000, 130000, 0), "
        "('Sunday', 1, 100000, 110000, 0), ('Sunday', 2, 120000, 130000, 0);");
    
    return true;
}

// ============================================================================
// SecureConfig Implementation
// ============================================================================

SecureConfig::SecureConfig() {}
SecureConfig::~SecureConfig() {}

bool SecureConfig::Initialize() {
    std::cout << "[SecureConfig] Starting initialization..." << std::endl;
    std::cout.flush();
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string path = GetDbPath("storage.database.secure_config.path", "secure.db");
    db_ = std::make_unique<Database>(path, GetSecureDbKey());
    
    std::cout << "[SecureConfig] Calling db_->Open()..." << std::endl;
    std::cout.flush();
    if (!db_->Open() || !CreateTables()) {
        std::cerr << "[SecureConfig] Init failed" << std::endl;
        return false;
    }

    std::cout << "[SecureConfig] Initialized: " << path << std::endl;
    std::cout.flush();
    return true;
}

bool SecureConfig::Set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;

    Statement stmt(*db_, 
        "INSERT OR REPLACE INTO secure_config (key, value, updated_at) "
        "VALUES (?, ?, strftime('%s', 'now'))");
    stmt.BindText(1, key);
    stmt.BindText(2, value);
    return stmt.Execute();
}

std::optional<std::string> SecureConfig::Get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return std::nullopt;

    Statement stmt(*db_, "SELECT value FROM secure_config WHERE key = ?");
    stmt.BindText(1, key);
    return stmt.Step() ? std::optional<std::string>(stmt.GetText(0)) : std::nullopt;
}

bool SecureConfig::Has(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;

    Statement stmt(*db_, "SELECT 1 FROM secure_config WHERE key = ? LIMIT 1");
    stmt.BindText(1, key);
    return stmt.Step();
}

bool SecureConfig::Remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;

    Statement stmt(*db_, "DELETE FROM secure_config WHERE key = ?");
    stmt.BindText(1, key);
    return stmt.Execute();
}

std::vector<std::string> SecureConfig::ListKeys() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> keys;
    if (!db_ || !db_->IsOpen()) return keys;

    Statement stmt(*db_, "SELECT key FROM secure_config ORDER BY key");
    while (stmt.Step()) {
        keys.push_back(stmt.GetText(0));
    }
    return keys;
}

std::string SecureConfig::GetDatabasePath() const {
    return GetDbPath("storage.database.secure_config.path", "secure.db");
}

bool SecureConfig::CreateTables() {
    return db_->Execute(
        "CREATE TABLE IF NOT EXISTS secure_config ("
        "key TEXT PRIMARY KEY NOT NULL, value TEXT NOT NULL, "
        "created_at INTEGER DEFAULT (strftime('%s', 'now')), "
        "updated_at INTEGER DEFAULT (strftime('%s', 'now')));"
    ) && db_->Execute(
        "CREATE TABLE IF NOT EXISTS secure_blobs ("
        "blob_name TEXT PRIMARY KEY NOT NULL, blob_data TEXT NOT NULL, "
        "blob_type TEXT DEFAULT 'generic', "
        "created_at INTEGER DEFAULT (strftime('%s', 'now')), "
        "updated_at INTEGER DEFAULT (strftime('%s', 'now')));"
    );
}

std::string SecureConfig::EncryptData(const std::string& data) const {
    // Additional layer encryption using AES-256-GCM
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";

    // Generate random IV
    unsigned char iv[12];
    if (!RAND_bytes(iv, sizeof(iv))) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    // Derive encryption key from device ID
    std::string device_key = DeriveKeyFromDevice("aes_layer");
    unsigned char key[32];
    memcpy(key, device_key.c_str(), std::min(device_key.size(), size_t(32)));

    // Encrypt
    std::vector<unsigned char> ciphertext(data.size() + 32);
    int len = 0;
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                          reinterpret_cast<const unsigned char*>(data.c_str()), 
                          data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    int ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    ciphertext_len += len;

    // Get authentication tag
    unsigned char tag[16];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    EVP_CIPHER_CTX_free(ctx);

    // Combine IV + ciphertext + tag and encode as hex
    std::stringstream ss;
    for (size_t i = 0; i < sizeof(iv); ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(iv[i]);
    }
    for (int i = 0; i < ciphertext_len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ciphertext[i]);
    }
    for (size_t i = 0; i < sizeof(tag); ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(tag[i]);
    }
    
    return ss.str();
}

std::string SecureConfig::DecryptData(const std::string& encrypted) const {
    if (encrypted.size() < 56) return ""; // IV(24) + tag(32) minimum

    // Decode hex
    std::vector<unsigned char> data;
    for (size_t i = 0; i < encrypted.size(); i += 2) {
        unsigned int byte;
        std::stringstream ss;
        ss << std::hex << encrypted.substr(i, 2);
        ss >> byte;
        data.push_back(static_cast<unsigned char>(byte));
    }

    if (data.size() < 28) return ""; // 12 IV + 16 tag

    unsigned char iv[12];
    memcpy(iv, data.data(), 12);
    
    unsigned char tag[16];
    memcpy(tag, data.data() + data.size() - 16, 16);

    size_t ciphertext_len = data.size() - 28;
    
    // Derive decryption key
    std::string device_key = DeriveKeyFromDevice("aes_layer");
    unsigned char key[32];
    memcpy(key, device_key.c_str(), std::min(device_key.size(), size_t(32)));

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    std::vector<unsigned char> plaintext(ciphertext_len + 32);
    int len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, 
                          data.data() + 12, ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    int plaintext_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    
    return std::string(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
}

bool SecureConfig::StoreSmtpCredentials(const std::string& server, int port,
                                        const std::string& username, const std::string& password,
                                        bool use_tls) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;

    // Encrypt sensitive password
    std::string encrypted_password = EncryptData(password);
    if (encrypted_password.empty() && !password.empty()) {
        std::cerr << "[SecureConfig] Failed to encrypt SMTP password" << std::endl;
        return false;
    }

    // Store as JSON in secure_config
    std::stringstream json;
    json << "{\"server\":\"" << server << "\","
         << "\"port\":" << port << ","
         << "\"username\":\"" << username << "\","
         << "\"password\":\"" << encrypted_password << "\","
         << "\"use_tls\":" << (use_tls ? "true" : "false") << "}";

    return Set("smtp_credentials", json.str());
}

bool SecureConfig::GetSmtpCredentials(std::string& server, int& port,
                                     std::string& username, std::string& password,
                                     bool& use_tls) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;

    Statement stmt(*db_, "SELECT value FROM secure_config WHERE key = ?");
    stmt.BindText(1, "smtp_credentials");
    
    if (!stmt.Step()) return false;

    std::string json_str = stmt.GetText(0);
    
    // Parse JSON (simple parser for this specific format)
    size_t pos = 0;
    auto extract = [&](const std::string& key) -> std::string {
        std::string search = "\"" + key + "\":\"";
        pos = json_str.find(search, pos);
        if (pos == std::string::npos) return "";
        pos += search.length();
        size_t end = json_str.find("\"", pos);
        if (end == std::string::npos) return "";
        return json_str.substr(pos, end - pos);
    };

    auto extract_int = [&](const std::string& key) -> int {
        std::string search = "\"" + key + "\":";
        pos = json_str.find(search);
        if (pos == std::string::npos) return 0;
        pos += search.length();
        size_t end = json_str.find_first_of(",}", pos);
        std::string num = json_str.substr(pos, end - pos);
        return std::stoi(num);
    };

    auto extract_bool = [&](const std::string& key) -> bool {
        std::string search = "\"" + key + "\":";
        pos = json_str.find(search);
        if (pos == std::string::npos) return false;
        pos += search.length();
        return json_str.substr(pos, 4) == "true";
    };

    pos = 0;
    server = extract("server");
    port = extract_int("port");
    pos = 0;
    username = extract("username");
    pos = 0;
    std::string encrypted_password = extract("password");
    use_tls = extract_bool("use_tls");

    // Decrypt password
    if (!encrypted_password.empty()) {
        password = DecryptData(encrypted_password);
        if (password.empty()) {
            std::cerr << "[SecureConfig] Failed to decrypt SMTP password" << std::endl;
            return false;
        }
    }

    return true;
}

bool SecureConfig::StoreCertificate(const std::string& cert_name, const std::string& cert_data) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;

    Statement stmt(*db_, 
        "INSERT OR REPLACE INTO secure_blobs (blob_name, blob_data, blob_type, updated_at) "
        "VALUES (?, ?, 'certificate', strftime('%s', 'now'))");
    stmt.BindText(1, cert_name);
    stmt.BindText(2, cert_data);
    return stmt.Execute();
}

std::optional<std::string> SecureConfig::GetCertificate(const std::string& cert_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return std::nullopt;

    Statement stmt(*db_, "SELECT blob_data FROM secure_blobs WHERE blob_name = ? AND blob_type = 'certificate'");
    stmt.BindText(1, cert_name);
    return stmt.Step() ? std::optional<std::string>(stmt.GetText(0)) : std::nullopt;
}

bool SecureConfig::StoreBlob(const std::string& blob_name, const std::string& blob_data) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;

    // Encrypt the blob
    std::string encrypted = EncryptData(blob_data);
    if (encrypted.empty() && !blob_data.empty()) {
        std::cerr << "[SecureConfig] Failed to encrypt blob" << std::endl;
        return false;
    }

    Statement stmt(*db_, 
        "INSERT OR REPLACE INTO secure_blobs (blob_name, blob_data, blob_type, updated_at) "
        "VALUES (?, ?, 'encrypted', strftime('%s', 'now'))");
    stmt.BindText(1, blob_name);
    stmt.BindText(2, encrypted);
    return stmt.Execute();
}

std::optional<std::string> SecureConfig::GetBlob(const std::string& blob_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return std::nullopt;

    Statement stmt(*db_, "SELECT blob_data FROM secure_blobs WHERE blob_name = ? AND blob_type = 'encrypted'");
    stmt.BindText(1, blob_name);
    
    if (!stmt.Step()) return std::nullopt;

    std::string encrypted = stmt.GetText(0);
    std::string decrypted = DecryptData(encrypted);
    
    if (decrypted.empty() && !encrypted.empty()) {
        std::cerr << "[SecureConfig] Failed to decrypt blob" << std::endl;
        return std::nullopt;
    }

    return decrypted;
}

bool SecureConfig::RotateEncryptionKey(const std::string& new_key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_ || !db_->IsOpen()) return false;
    
    return db_->RotateKey(new_key);
}

// ============================================================================
// Singleton Accessors
// ============================================================================

UserManager& GetUserManager() {
    static UserManager instance;
    return instance;
}

RecordingManager& GetRecordingManager() {
    static RecordingManager instance;
    return instance;
}

SecureConfig& GetSecureConfig() {
    static SecureConfig instance;
    return instance;
}

// ============================================================================
// Storage Subsystem Initialization
// ============================================================================

bool Init(std::string &outError) {
    try {
        std::cout << "[Storage] Initializing with SQLCipher..." << std::endl;
        std::cout.flush();
        
        // Ensure /mnt/app exists
        if (!std::filesystem::exists("/mnt/app")) {
            std::filesystem::create_directories("/mnt/app");
            std::cout << "[Storage] Created /mnt/app" << std::endl;
            std::cout.flush();
        }

        std::cout << "[Storage] Initializing UserManager..." << std::endl;
        std::cout.flush();
        if (!GetUserManager().Initialize()) {
            outError = "User management init failed";
            return false;
        }

        std::cout << "[Storage] Initializing RecordingManager..." << std::endl;
        std::cout.flush();
        if (!GetRecordingManager().Initialize()) {
            outError = "Recording manager init failed";
            return false;
        }

        std::cout << "[Storage] Initializing SecureConfig..." << std::endl;
        std::cout.flush();
        if (!GetSecureConfig().Initialize()) {
            outError = "Secure config init failed";
            return false;
        }

        std::cout << "[Storage] Initializing CredentialManager..." << std::endl;
        std::cout.flush();
        if (!GetCredentialManager().Initialize()) {
            outError = "Credential manager init failed";
            return false;
        }

        std::cout << "[Storage] All databases initialized successfully" << std::endl;
        std::cout << "  User DB: " << GetUserManager().GetDatabasePath() << std::endl;
        std::cout << "  Recording DB: " << GetRecordingManager().GetDatabasePath() << std::endl;
        std::cout << "  Secure DB: " << GetSecureConfig().GetDatabasePath() << std::endl;
        std::cout << "  Credential Manager: Initialized" << std::endl;
        std::cout.flush();
        
        return true;
    } catch (const std::exception& e) {
        outError = std::string("Storage init exception: ") + e.what();
        std::cerr << "[Storage Error] " << outError << std::endl;
        return false;
    }
}

void Shutdown() {
    std::cout << "[Storage] Shutting down..." << std::endl;
    std::cout.flush();
    
    // Set global flag to prevent any further singleton access
    g_shutdown_in_progress = true;
    
    // Explicitly close all databases in reverse order of initialization
    // This prevents segfaults from undefined singleton destruction order
    try {
        // Note: We don't call methods on singletons during shutdown
        // Just let their destructors handle cleanup naturally
        // The databases will close themselves in their destructors
        
        std::cout << "[Storage] Databases will close during natural cleanup" << std::endl;
        std::cout.flush();
    } catch (...) {
        // Ignore all exceptions during shutdown
    }
}

} // namespace storage
} // namespace ipcam
