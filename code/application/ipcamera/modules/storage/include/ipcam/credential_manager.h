#pragma once

#include <string>
#include <optional>
#include <memory>
#include <mutex>
#include "storage.h"

namespace ipcam {
namespace storage {

/**
 * @brief Centralized Credential Manager for all sensitive data
 * 
 * This class provides a unified, secure interface for storing and retrieving
 * all types of sensitive information across the IP camera system.
 * 
 * Security Features:
 * - Dual-layer encryption (SQLCipher + AES-256-GCM)
 * - Automatic credential rotation support
 * - Audit logging for credential access
 * - Type-safe API per credential category
 * 
 * Supported Credential Types:
 * - SMTP credentials (username/password)
 * - SNMP community strings and SNMPv3 credentials
 * - WiFi PSK/passwords
 * - SSL/TLS certificates and private keys
 * - API keys and tokens
 * - Third-party service credentials
 * - ONVIF credentials
 * - RTSP authentication
 */
class CredentialManager {
public:
    CredentialManager();
    ~CredentialManager();

    // Disable copy and move
    CredentialManager(const CredentialManager&) = delete;
    CredentialManager& operator=(const CredentialManager&) = delete;

    /**
     * Initialize the credential manager
     * @return true on success
     */
    bool Initialize();

    // ========================================================================
    // SMTP Credentials
    // ========================================================================
    
    struct SmtpCredentials {
        std::string server;
        int port;
        std::string username;
        std::string password;
        std::string from_email;
        std::string from_name;
        bool use_tls;
        int timeout_sec;
    };

    bool StoreSmtpCredentials(const SmtpCredentials& creds);
    std::optional<SmtpCredentials> GetSmtpCredentials();
    bool DeleteSmtpCredentials();

    // ========================================================================
    // SNMP Credentials
    // ========================================================================
    
    struct SnmpCredentials {
        std::string community_ro;           // Read-only community string
        std::string community_rw;           // Read-write community string
        // SNMPv3
        std::string v3_user;
        std::string v3_auth_password;
        std::string v3_priv_password;
        std::string v3_auth_protocol;       // MD5 or SHA
        std::string v3_priv_protocol;       // DES or AES
    };

    bool StoreSnmpCredentials(const SnmpCredentials& creds);
    std::optional<SnmpCredentials> GetSnmpCredentials();
    bool DeleteSnmpCredentials();

    // ========================================================================
    // WiFi Credentials
    // ========================================================================
    
    struct WifiCredentials {
        std::string ssid;
        std::string psk;                    // Pre-shared key / password
        std::string security_type;          // "open", "wep", "wpa", "wpa2"
    };

    bool StoreWifiCredentials(const WifiCredentials& creds);
    std::optional<WifiCredentials> GetWifiCredentials(const std::string& ssid);
    std::vector<std::string> ListStoredWifiNetworks();
    bool DeleteWifiCredentials(const std::string& ssid);

    // ========================================================================
    // SSL/TLS Certificates and Private Keys
    // ========================================================================
    
    struct TlsCertificate {
        std::string certificate_pem;        // Certificate in PEM format
        std::string private_key_pem;        // Private key in PEM format
        std::string ca_chain_pem;           // CA chain (optional)
        std::string passphrase;             // Key passphrase (optional)
        int64_t expiry_time;                // Unix timestamp
    };

    bool StoreTlsCertificate(const std::string& cert_name, const TlsCertificate& cert);
    std::optional<TlsCertificate> GetTlsCertificate(const std::string& cert_name);
    bool DeleteTlsCertificate(const std::string& cert_name);
    std::vector<std::string> ListTlsCertificates();

    // ========================================================================
    // API Keys and Tokens
    // ========================================================================
    
    struct ApiCredential {
        std::string api_key;
        std::string api_secret;
        std::string access_token;
        std::string refresh_token;
        int64_t token_expiry;
    };

    bool StoreApiCredential(const std::string& service_name, const ApiCredential& cred);
    std::optional<ApiCredential> GetApiCredential(const std::string& service_name);
    bool DeleteApiCredential(const std::string& service_name);

    // ========================================================================
    // ONVIF/RTSP Authentication
    // ========================================================================
    
    struct MediaCredentials {
        std::string username;
        std::string password;
        std::string realm;
        bool digest_auth;
    };

    bool StoreOnvifCredentials(const MediaCredentials& creds);
    std::optional<MediaCredentials> GetOnvifCredentials();
    
    bool StoreRtspCredentials(const MediaCredentials& creds);
    std::optional<MediaCredentials> GetRtspCredentials();

    // ========================================================================
    // Generic Secure Storage (for custom credentials)
    // ========================================================================
    
    bool StoreGenericCredential(const std::string& key, const std::string& value);
    std::optional<std::string> GetGenericCredential(const std::string& key);
    bool DeleteGenericCredential(const std::string& key);

    // ========================================================================
    // Credential Management
    // ========================================================================
    
    /**
     * Rotate all encryption keys (re-encrypt all stored credentials)
     * @param new_key New master key
     * @return true on success
     */
    bool RotateAllKeys(const std::string& new_key);

    /**
     * Export credentials (encrypted) for backup
     * @return encrypted blob
     */
    std::optional<std::string> ExportCredentials();

    /**
     * Import credentials from backup
     * @param encrypted_blob Encrypted credential blob
     * @return true on success
     */
    bool ImportCredentials(const std::string& encrypted_blob);

    /**
     * Clear all stored credentials (factory reset)
     * @return true on success
     */
    bool ClearAllCredentials();

    /**
     * Get credential statistics
     * @return JSON string with stats
     */
    std::string GetCredentialStats();

private:
    SecureConfig* secure_storage_;  // Pointer to shared singleton (not owned)
    mutable std::mutex mutex_;
    bool initialized_;

    // Helper methods
    std::string SerializeToJson(const SmtpCredentials& creds);
    std::optional<SmtpCredentials> DeserializeSmtpCredentials(const std::string& json);
    
    std::string SerializeToJson(const SnmpCredentials& creds);
    std::optional<SnmpCredentials> DeserializeSnmpCredentials(const std::string& json);
    
    std::string SerializeToJson(const WifiCredentials& creds);
    std::optional<WifiCredentials> DeserializeWifiCredentials(const std::string& json);
    
    std::string SerializeToJson(const TlsCertificate& cert);
    std::optional<TlsCertificate> DeserializeTlsCertificate(const std::string& json);
    
    std::string SerializeToJson(const ApiCredential& cred);
    std::optional<ApiCredential> DeserializeApiCredential(const std::string& json);
    
    std::string SerializeToJson(const MediaCredentials& creds);
    std::optional<MediaCredentials> DeserializeMediaCredentials(const std::string& json);

    // Audit logging
    void LogCredentialAccess(const std::string& operation, const std::string& credential_type);

private:
    // Helper to check if storage is accessible (not during shutdown)
    bool IsStorageAccessible() const;
};

// Singleton accessor
CredentialManager& GetCredentialManager();

} // namespace storage
} // namespace ipcam
