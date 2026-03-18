#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>
#include <optional>
#include <unordered_map>

// Forward declaration - avoid conflict with sqlite3.h
struct sqlite3;
struct sqlite3_stmt;

namespace ipcam {
namespace storage {

inline const char* storage_info() { return "storage"; }

// Storage subsystem initialization
bool Init(std::string &outError);
void Shutdown();

/**
 * Modern C++ RAII wrapper for SQLCipher database connection
 * Handles encryption, thread-safety, and automatic resource cleanup
 */
class Database {
public:
    /**
     * Create a database connection with SQLCipher encryption
     * @param db_path Path to database file (will be created if not exists)
     * @param encryption_key Encryption key for SQLCipher (empty = no encryption)
     */
    explicit Database(const std::string& db_path, const std::string& encryption_key = "");
    ~Database();

    // Disable copy, enable move
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    /**
     * Open the database connection and set encryption key
     * @return true on success, false on failure
     */
    bool Open();

    /**
     * Close the database connection
     */
    void Close();

    /**
     * Check if database is open
     */
    bool IsOpen() const;

    /**
     * Get raw sqlite3 handle (for advanced operations)
     */
    sqlite3* GetHandle() const;

    /**
     * Execute SQL statement without results
     * @param sql SQL statement
     * @return true on success, false on failure
     */
    bool Execute(const std::string& sql);

    /**
     * Execute SQL with parameter binding
     * @param sql SQL statement with ? placeholders
     * @param params Parameters to bind
     * @return true on success, false on failure
     */
    bool ExecuteWithParams(const std::string& sql, const std::vector<std::string>& params);

    /**
     * Query with row callback
     * @param sql SQL query
     * @param callback Called for each row (column_name -> value map)
     * @return true on success, false on failure
     */
    bool Query(const std::string& sql, 
               std::function<void(const std::unordered_map<std::string, std::string>&)> callback);

    /**
     * Get last error message
     */
    std::string GetLastError() const;

    /**
     * Begin transaction
     */
    bool BeginTransaction();

    /**
     * Commit transaction
     */
    bool Commit();

    /**
     * Rollback transaction
     */
    bool Rollback();

    /**
     * Get last insert row ID
     */
    int64_t GetLastInsertRowId() const;

    /**
     * Rotate encryption key (use SQLCipher PRAGMA rekey)
     * @param new_key New encryption key
     * @return true on success
     */
    bool RotateKey(const std::string& new_key);

private:
    sqlite3* db_;
    std::string db_path_;
    std::string encryption_key_;
    mutable std::mutex mutex_;
    bool is_open_;

    bool SetEncryptionKey(bool is_new_db = false);
    bool ExecuteUnlocked(const std::string& sql); // Execute without acquiring mutex
    void LogError(const std::string& context) const;
};

/**
 * Modern C++ RAII wrapper for prepared statements
 */
class Statement {
public:
    explicit Statement(Database& db, const std::string& sql);
    ~Statement();

    // Disable copy, enable move
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;

    /**
     * Bind parameters (1-indexed)
     */
    bool BindInt(int index, int value);
    bool BindInt64(int index, int64_t value);
    bool BindDouble(int index, double value);
    bool BindText(int index, const std::string& value);
    bool BindNull(int index);

    /**
     * Execute the statement
     * @return true if SQLITE_DONE, false otherwise
     */
    bool Execute();

    /**
     * Step through results
     * @return true if SQLITE_ROW, false if SQLITE_DONE or error
     */
    bool Step();

    /**
     * Get column values (0-indexed)
     */
    int GetInt(int column) const;
    int64_t GetInt64(int column) const;
    double GetDouble(int column) const;
    std::string GetText(int column) const;

    /**
     * Reset the statement for reuse
     */
    bool Reset();

    /**
     * Get column count
     */
    int GetColumnCount() const;

    /**
     * Get column name
     */
    std::string GetColumnName(int column) const;

private:
    sqlite3_stmt* stmt_;
    Database& db_;
    bool is_valid_;

    void LogError(const std::string& context) const;
};

/**
 * Modern C++ SQLCipher-based User Management System
 * Thread-safe database operations with automatic encryption
 */
class UserManager {
public:
    UserManager();
    ~UserManager();

    // Disable copy and move
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;
    UserManager(UserManager&&) = delete;
    UserManager& operator=(UserManager&&) = delete;

    /**
     * Initialize user management database
     * @return true on success
     */
    bool Initialize();

    /**
     * Register a new user
     * @param username Username (must be unique)
     * @param password Password (hashed before storage)
     * @param user_level User privilege level
     * @return true on success
     */
    bool RegisterUser(const std::string& username, const std::string& password, int user_level);

    /**
     * Verify user credentials
     * @param username Username
     * @param password Password
     * @return user_level if authenticated, std::nullopt otherwise
     */
    std::optional<int> AuthenticateUser(const std::string& username, const std::string& password);

    /**
     * Check if user exists
     * @param username Username
     * @return true if user exists
     */
    bool UserExists(const std::string& username);

    /**
     * Delete user
     * @param username Username
     * @return true on success
     */
    bool DeleteUser(const std::string& username);

    /**
     * Update user password
     * @param username Username
     * @param new_password New password
     * @return true on success
     */
    bool UpdatePassword(const std::string& username, const std::string& new_password);

    /**
     * Get database path
     */
    std::string GetDatabasePath() const;

private:
    std::unique_ptr<Database> db_;
    mutable std::mutex mutex_;

    bool CreateTables();
    std::string HashPassword(const std::string& password) const;
    
    // Internal helpers that assume mutex is already held by caller
    bool UserExistsUnlocked(const std::string& username);
    bool RegisterUserUnlocked(const std::string& username, const std::string& password, int user_level);
};

/**
 * Recording database manager
 * 
 * Note: Recordings database is stored on SD card (/mnt/sd/.ipcamera/recordings.db)
 * If SD card is not mounted, recording functionality is disabled but the camera
 * continues to operate normally. Use IsStorageAvailable() to check status.
 */
class RecordingManager {
public:
    RecordingManager();
    ~RecordingManager();

    // Disable copy and move
    RecordingManager(const RecordingManager&) = delete;
    RecordingManager& operator=(const RecordingManager&) = delete;
    RecordingManager(RecordingManager&&) = delete;
    RecordingManager& operator=(RecordingManager&&) = delete;

    /**
     * Initialize recording database
     * Returns true even if SD card is not mounted (graceful degradation)
     * @return true on success (including when SD card unavailable)
     */
    bool Initialize();

    /**
     * Check if SD card storage is available for recording
     * @return true if SD card is mounted and database is open
     */
    bool IsStorageAvailable() const { return storage_available_; }

    /**
     * Try to reconnect to SD card storage
     * Call this when SD card is inserted after boot
     * @return true if storage is now available
     */
    bool TryReconnectStorage();

    /**
     * Add recording entry
     * @return false if storage unavailable
     */
    bool AddRecording(const std::string& date, const std::string& filename,
                     int64_t start_time, int64_t end_time,
                     const std::string& recording_type, const std::string& status);

    /**
     * Get database path
     */
    std::string GetDatabasePath() const;

private:
    std::unique_ptr<Database> db_;
    mutable std::mutex mutex_;
    bool initialized_;
    bool storage_available_;

    bool CreateTables();
};

/**
 * Secure Configuration Storage with SQLCipher encryption
 */
class SecureConfig {
public:
    SecureConfig();
    ~SecureConfig();

    // Disable copy and move
    SecureConfig(const SecureConfig&) = delete;
    SecureConfig& operator=(const SecureConfig&) = delete;
    SecureConfig(SecureConfig&&) = delete;
    SecureConfig& operator=(SecureConfig&&) = delete;

    /**
     * Initialize secure config database
     * @return true on success
     */
    bool Initialize();

    /**
     * Store a secure value (encrypted at database level via SQLCipher)
     * @param key Configuration key
     * @param value Value to store
     * @return true on success
     */
    bool Set(const std::string& key, const std::string& value);

    /**
     * Retrieve a secure value
     * @param key Configuration key
     * @return value if exists, std::nullopt otherwise
     */
    std::optional<std::string> Get(const std::string& key);

    /**
     * Check if key exists
     * @param key Configuration key
     * @return true if key exists
     */
    bool Has(const std::string& key);

    /**
     * Remove a key
     * @param key Configuration key
     * @return true on success
     */
    bool Remove(const std::string& key);

    /**
     * List all keys
     * @return vector of all keys
     */
    std::vector<std::string> ListKeys();

    /**
     * Get database path
     */
    std::string GetDatabasePath() const;

    /**
     * Store SMTP credentials securely
     * @param server SMTP server address
     * @param port SMTP port
     * @param username SMTP username
     * @param password SMTP password (will be encrypted)
     * @param use_tls Use TLS/SSL
     * @return true on success
     */
    bool StoreSmtpCredentials(const std::string& server, int port,
                             const std::string& username, const std::string& password,
                             bool use_tls);

    /**
     * Get SMTP credentials
     * @param server Output: SMTP server
     * @param port Output: SMTP port
     * @param username Output: SMTP username
     * @param password Output: SMTP password (decrypted)
     * @param use_tls Output: Use TLS
     * @return true if credentials exist
     */
    bool GetSmtpCredentials(std::string& server, int& port,
                           std::string& username, std::string& password,
                           bool& use_tls);

    /**
     * Store SSL/TLS certificate
     * @param cert_name Certificate name (e.g., "server.crt")
     * @param cert_data Certificate data (PEM format)
     * @return true on success
     */
    bool StoreCertificate(const std::string& cert_name, const std::string& cert_data);

    /**
     * Get SSL/TLS certificate
     * @param cert_name Certificate name
     * @return certificate data if exists
     */
    std::optional<std::string> GetCertificate(const std::string& cert_name);

    /**
     * Store encrypted blob (general purpose)
     * @param blob_name Blob identifier
     * @param blob_data Data to store
     * @return true on success
     */
    bool StoreBlob(const std::string& blob_name, const std::string& blob_data);

    /**
     * Get encrypted blob
     * @param blob_name Blob identifier
     * @return blob data if exists
     */
    std::optional<std::string> GetBlob(const std::string& blob_name);

    /**
     * Rotate database encryption key
     * @param new_key New encryption key
     * @return true on success
     */
    bool RotateEncryptionKey(const std::string& new_key);

private:
    std::unique_ptr<Database> db_;
    mutable std::mutex mutex_;

    bool CreateTables();
    std::string EncryptData(const std::string& data) const;
    std::string DecryptData(const std::string& encrypted) const;
};

// Singleton accessors
UserManager& GetUserManager();
RecordingManager& GetRecordingManager();
SecureConfig& GetSecureConfig();

} // namespace storage
} // namespace ipcam
