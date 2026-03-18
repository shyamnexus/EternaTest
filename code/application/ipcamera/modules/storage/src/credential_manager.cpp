#include "ipcam/credential_manager.h"
#include "ipcam/storage.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>
#include <chrono>

using json = nlohmann::json;

namespace ipcam {
namespace storage {

// Credential key prefixes for organized storage
constexpr const char* KEY_PREFIX_SMTP = "cred:smtp";
constexpr const char* KEY_PREFIX_SNMP = "cred:snmp";
constexpr const char* KEY_PREFIX_WIFI = "cred:wifi:";
constexpr const char* KEY_PREFIX_TLS = "cred:tls:";
constexpr const char* KEY_PREFIX_API = "cred:api:";
constexpr const char* KEY_PREFIX_ONVIF = "cred:onvif";
constexpr const char* KEY_PREFIX_RTSP = "cred:rtsp";
constexpr const char* KEY_PREFIX_GENERIC = "cred:generic:";
constexpr const char* KEY_AUDIT_LOG = "cred:audit_log";

CredentialManager::CredentialManager()
    : secure_storage_(nullptr), initialized_(false) {}

CredentialManager::~CredentialManager() {
    // Just clear the pointer - don't access the singleton during destruction
    // The SecureConfig singleton manages its own lifetime
    secure_storage_ = nullptr;
    initialized_ = false;
}

bool CredentialManager::Initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return true;
    }

    spdlog::info("[CredentialManager] Initializing secure credential storage...");
    
    // Use the shared SecureConfig singleton (do not create a new instance)
    secure_storage_ = &GetSecureConfig();
    if (!secure_storage_) {
        spdlog::error("[CredentialManager] Failed to get shared SecureConfig singleton");
        return false;
    }

    initialized_ = true;
    spdlog::info("[CredentialManager] Credential manager initialized with shared SecureConfig");
    
    return true;
}

// Helper to check if we can safely use secure_storage_
// Returns false during shutdown when singletons may be destroyed
bool CredentialManager::IsStorageAccessible() const {
    return initialized_ && secure_storage_ != nullptr;
}

// ============================================================================
// SMTP Credentials
// ============================================================================

std::string CredentialManager::SerializeToJson(const SmtpCredentials& creds) {
    json j;
    j["server"] = creds.server;
    j["port"] = creds.port;
    j["username"] = creds.username;
    j["password"] = creds.password;
    j["from_email"] = creds.from_email;
    j["from_name"] = creds.from_name;
    j["use_tls"] = creds.use_tls;
    j["timeout_sec"] = creds.timeout_sec;
    j["_updated"] = std::chrono::system_clock::now().time_since_epoch().count();
    return j.dump();
}

std::optional<CredentialManager::SmtpCredentials> 
CredentialManager::DeserializeSmtpCredentials(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        SmtpCredentials creds;
        creds.server = j.value("server", "");
        creds.port = j.value("port", 587);
        creds.username = j.value("username", "");
        creds.password = j.value("password", "");
        creds.from_email = j.value("from_email", "");
        creds.from_name = j.value("from_name", "");
        creds.use_tls = j.value("use_tls", true);
        creds.timeout_sec = j.value("timeout_sec", 30);
        return creds;
    } catch (const std::exception& e) {
        spdlog::error("[CredentialManager] Failed to deserialize SMTP credentials: {}", e.what());
        return std::nullopt;
    }
}

bool CredentialManager::StoreSmtpCredentials(const SmtpCredentials& creds) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string json_data = SerializeToJson(creds);
    bool success = secure_storage_->StoreBlob(KEY_PREFIX_SMTP, json_data);
    
    if (success) {
        LogCredentialAccess("STORE", "SMTP");
        spdlog::info("[CredentialManager] SMTP credentials stored securely");
    }
    
    return success;
}

std::optional<CredentialManager::SmtpCredentials> CredentialManager::GetSmtpCredentials() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return std::nullopt;

    auto blob = secure_storage_->GetBlob(KEY_PREFIX_SMTP);
    if (!blob) return std::nullopt;

    LogCredentialAccess("RETRIEVE", "SMTP");
    return DeserializeSmtpCredentials(*blob);
}

bool CredentialManager::DeleteSmtpCredentials() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    LogCredentialAccess("DELETE", "SMTP");
    return secure_storage_->Remove(KEY_PREFIX_SMTP);
}

// ============================================================================
// SNMP Credentials
// ============================================================================

std::string CredentialManager::SerializeToJson(const SnmpCredentials& creds) {
    json j;
    j["community_ro"] = creds.community_ro;
    j["community_rw"] = creds.community_rw;
    j["v3_user"] = creds.v3_user;
    j["v3_auth_password"] = creds.v3_auth_password;
    j["v3_priv_password"] = creds.v3_priv_password;
    j["v3_auth_protocol"] = creds.v3_auth_protocol;
    j["v3_priv_protocol"] = creds.v3_priv_protocol;
    j["_updated"] = std::chrono::system_clock::now().time_since_epoch().count();
    return j.dump();
}

std::optional<CredentialManager::SnmpCredentials> 
CredentialManager::DeserializeSnmpCredentials(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        SnmpCredentials creds;
        creds.community_ro = j.value("community_ro", "public");
        creds.community_rw = j.value("community_rw", "private");
        creds.v3_user = j.value("v3_user", "");
        creds.v3_auth_password = j.value("v3_auth_password", "");
        creds.v3_priv_password = j.value("v3_priv_password", "");
        creds.v3_auth_protocol = j.value("v3_auth_protocol", "SHA");
        creds.v3_priv_protocol = j.value("v3_priv_protocol", "AES");
        return creds;
    } catch (const std::exception& e) {
        spdlog::error("[CredentialManager] Failed to deserialize SNMP credentials: {}", e.what());
        return std::nullopt;
    }
}

bool CredentialManager::StoreSnmpCredentials(const SnmpCredentials& creds) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string json_data = SerializeToJson(creds);
    bool success = secure_storage_->StoreBlob(KEY_PREFIX_SNMP, json_data);
    
    if (success) {
        LogCredentialAccess("STORE", "SNMP");
        spdlog::info("[CredentialManager] SNMP credentials stored securely");
    }
    
    return success;
}

std::optional<CredentialManager::SnmpCredentials> CredentialManager::GetSnmpCredentials() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return std::nullopt;

    auto blob = secure_storage_->GetBlob(KEY_PREFIX_SNMP);
    if (!blob) return std::nullopt;

    LogCredentialAccess("RETRIEVE", "SNMP");
    return DeserializeSnmpCredentials(*blob);
}

bool CredentialManager::DeleteSnmpCredentials() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    LogCredentialAccess("DELETE", "SNMP");
    return secure_storage_->Remove(KEY_PREFIX_SNMP);
}

// ============================================================================
// WiFi Credentials
// ============================================================================

std::string CredentialManager::SerializeToJson(const WifiCredentials& creds) {
    json j;
    j["ssid"] = creds.ssid;
    j["psk"] = creds.psk;
    j["security_type"] = creds.security_type;
    j["_updated"] = std::chrono::system_clock::now().time_since_epoch().count();
    return j.dump();
}

std::optional<CredentialManager::WifiCredentials> 
CredentialManager::DeserializeWifiCredentials(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        WifiCredentials creds;
        creds.ssid = j.value("ssid", "");
        creds.psk = j.value("psk", "");
        creds.security_type = j.value("security_type", "wpa2");
        return creds;
    } catch (const std::exception& e) {
        spdlog::error("[CredentialManager] Failed to deserialize WiFi credentials: {}", e.what());
        return std::nullopt;
    }
}

bool CredentialManager::StoreWifiCredentials(const WifiCredentials& creds) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string key = std::string(KEY_PREFIX_WIFI) + creds.ssid;
    std::string json_data = SerializeToJson(creds);
    bool success = secure_storage_->StoreBlob(key, json_data);
    
    if (success) {
        LogCredentialAccess("STORE", "WiFi:" + creds.ssid);
        spdlog::info("[CredentialManager] WiFi credentials stored for SSID: {}", creds.ssid);
    }
    
    return success;
}

std::optional<CredentialManager::WifiCredentials> 
CredentialManager::GetWifiCredentials(const std::string& ssid) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return std::nullopt;

    std::string key = std::string(KEY_PREFIX_WIFI) + ssid;
    auto blob = secure_storage_->GetBlob(key);
    if (!blob) return std::nullopt;

    LogCredentialAccess("RETRIEVE", "WiFi:" + ssid);
    return DeserializeWifiCredentials(*blob);
}

std::vector<std::string> CredentialManager::ListStoredWifiNetworks() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> networks;
    if (!IsStorageAccessible()) return networks;

    auto keys = secure_storage_->ListKeys();
    std::string prefix = KEY_PREFIX_WIFI;
    
    for (const auto& key : keys) {
        if (key.find(prefix) == 0) {
            networks.push_back(key.substr(prefix.length()));
        }
    }
    
    return networks;
}

bool CredentialManager::DeleteWifiCredentials(const std::string& ssid) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string key = std::string(KEY_PREFIX_WIFI) + ssid;
    LogCredentialAccess("DELETE", "WiFi:" + ssid);
    return secure_storage_->Remove(key);
}

// ============================================================================
// TLS Certificates
// ============================================================================

std::string CredentialManager::SerializeToJson(const TlsCertificate& cert) {
    json j;
    j["certificate_pem"] = cert.certificate_pem;
    j["private_key_pem"] = cert.private_key_pem;
    j["ca_chain_pem"] = cert.ca_chain_pem;
    j["passphrase"] = cert.passphrase;
    j["expiry_time"] = cert.expiry_time;
    j["_updated"] = std::chrono::system_clock::now().time_since_epoch().count();
    return j.dump();
}

std::optional<CredentialManager::TlsCertificate> 
CredentialManager::DeserializeTlsCertificate(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        TlsCertificate cert;
        cert.certificate_pem = j.value("certificate_pem", "");
        cert.private_key_pem = j.value("private_key_pem", "");
        cert.ca_chain_pem = j.value("ca_chain_pem", "");
        cert.passphrase = j.value("passphrase", "");
        cert.expiry_time = j.value("expiry_time", 0LL);
        return cert;
    } catch (const std::exception& e) {
        spdlog::error("[CredentialManager] Failed to deserialize TLS certificate: {}", e.what());
        return std::nullopt;
    }
}

bool CredentialManager::StoreTlsCertificate(const std::string& cert_name, const TlsCertificate& cert) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string key = std::string(KEY_PREFIX_TLS) + cert_name;
    std::string json_data = SerializeToJson(cert);
    bool success = secure_storage_->StoreBlob(key, json_data);
    
    if (success) {
        LogCredentialAccess("STORE", "TLS:" + cert_name);
        spdlog::info("[CredentialManager] TLS certificate stored: {}", cert_name);
    }
    
    return success;
}

std::optional<CredentialManager::TlsCertificate> 
CredentialManager::GetTlsCertificate(const std::string& cert_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return std::nullopt;

    std::string key = std::string(KEY_PREFIX_TLS) + cert_name;
    auto blob = secure_storage_->GetBlob(key);
    if (!blob) return std::nullopt;

    LogCredentialAccess("RETRIEVE", "TLS:" + cert_name);
    return DeserializeTlsCertificate(*blob);
}

bool CredentialManager::DeleteTlsCertificate(const std::string& cert_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string key = std::string(KEY_PREFIX_TLS) + cert_name;
    LogCredentialAccess("DELETE", "TLS:" + cert_name);
    return secure_storage_->Remove(key);
}

std::vector<std::string> CredentialManager::ListTlsCertificates() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> certs;
    if (!IsStorageAccessible()) return certs;

    auto keys = secure_storage_->ListKeys();
    std::string prefix = KEY_PREFIX_TLS;
    
    for (const auto& key : keys) {
        if (key.find(prefix) == 0) {
            certs.push_back(key.substr(prefix.length()));
        }
    }
    
    return certs;
}

// ============================================================================
// API Credentials
// ============================================================================

std::string CredentialManager::SerializeToJson(const ApiCredential& cred) {
    json j;
    j["api_key"] = cred.api_key;
    j["api_secret"] = cred.api_secret;
    j["access_token"] = cred.access_token;
    j["refresh_token"] = cred.refresh_token;
    j["token_expiry"] = cred.token_expiry;
    j["_updated"] = std::chrono::system_clock::now().time_since_epoch().count();
    return j.dump();
}

std::optional<CredentialManager::ApiCredential> 
CredentialManager::DeserializeApiCredential(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        ApiCredential cred;
        cred.api_key = j.value("api_key", "");
        cred.api_secret = j.value("api_secret", "");
        cred.access_token = j.value("access_token", "");
        cred.refresh_token = j.value("refresh_token", "");
        cred.token_expiry = j.value("token_expiry", 0LL);
        return cred;
    } catch (const std::exception& e) {
        spdlog::error("[CredentialManager] Failed to deserialize API credential: {}", e.what());
        return std::nullopt;
    }
}

bool CredentialManager::StoreApiCredential(const std::string& service_name, const ApiCredential& cred) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string key = std::string(KEY_PREFIX_API) + service_name;
    std::string json_data = SerializeToJson(cred);
    bool success = secure_storage_->StoreBlob(key, json_data);
    
    if (success) {
        LogCredentialAccess("STORE", "API:" + service_name);
        spdlog::info("[CredentialManager] API credentials stored for: {}", service_name);
    }
    
    return success;
}

std::optional<CredentialManager::ApiCredential> 
CredentialManager::GetApiCredential(const std::string& service_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return std::nullopt;

    std::string key = std::string(KEY_PREFIX_API) + service_name;
    auto blob = secure_storage_->GetBlob(key);
    if (!blob) return std::nullopt;

    LogCredentialAccess("RETRIEVE", "API:" + service_name);
    return DeserializeApiCredential(*blob);
}

bool CredentialManager::DeleteApiCredential(const std::string& service_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string key = std::string(KEY_PREFIX_API) + service_name;
    LogCredentialAccess("DELETE", "API:" + service_name);
    return secure_storage_->Remove(key);
}

// ============================================================================
// Media Credentials (ONVIF/RTSP)
// ============================================================================

std::string CredentialManager::SerializeToJson(const MediaCredentials& creds) {
    json j;
    j["username"] = creds.username;
    j["password"] = creds.password;
    j["realm"] = creds.realm;
    j["digest_auth"] = creds.digest_auth;
    j["_updated"] = std::chrono::system_clock::now().time_since_epoch().count();
    return j.dump();
}

std::optional<CredentialManager::MediaCredentials> 
CredentialManager::DeserializeMediaCredentials(const std::string& json_str) {
    try {
        auto j = json::parse(json_str);
        MediaCredentials creds;
        creds.username = j.value("username", "");
        creds.password = j.value("password", "");
        creds.realm = j.value("realm", "");
        creds.digest_auth = j.value("digest_auth", true);
        return creds;
    } catch (const std::exception& e) {
        spdlog::error("[CredentialManager] Failed to deserialize media credentials: {}", e.what());
        return std::nullopt;
    }
}

bool CredentialManager::StoreOnvifCredentials(const MediaCredentials& creds) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string json_data = SerializeToJson(creds);
    bool success = secure_storage_->StoreBlob(KEY_PREFIX_ONVIF, json_data);
    
    if (success) {
        LogCredentialAccess("STORE", "ONVIF");
        spdlog::info("[CredentialManager] ONVIF credentials stored securely");
    }
    
    return success;
}

std::optional<CredentialManager::MediaCredentials> CredentialManager::GetOnvifCredentials() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return std::nullopt;

    auto blob = secure_storage_->GetBlob(KEY_PREFIX_ONVIF);
    if (!blob) return std::nullopt;

    LogCredentialAccess("RETRIEVE", "ONVIF");
    return DeserializeMediaCredentials(*blob);
}

bool CredentialManager::StoreRtspCredentials(const MediaCredentials& creds) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string json_data = SerializeToJson(creds);
    bool success = secure_storage_->StoreBlob(KEY_PREFIX_RTSP, json_data);
    
    if (success) {
        LogCredentialAccess("STORE", "RTSP");
        spdlog::info("[CredentialManager] RTSP credentials stored securely");
    }
    
    return success;
}

std::optional<CredentialManager::MediaCredentials> CredentialManager::GetRtspCredentials() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return std::nullopt;

    auto blob = secure_storage_->GetBlob(KEY_PREFIX_RTSP);
    if (!blob) return std::nullopt;

    LogCredentialAccess("RETRIEVE", "RTSP");
    return DeserializeMediaCredentials(*blob);
}

// ============================================================================
// Generic Credentials
// ============================================================================

bool CredentialManager::StoreGenericCredential(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string full_key = std::string(KEY_PREFIX_GENERIC) + key;
    bool success = secure_storage_->StoreBlob(full_key, value);
    
    if (success) {
        LogCredentialAccess("STORE", "Generic:" + key);
    }
    
    return success;
}

std::optional<std::string> CredentialManager::GetGenericCredential(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return std::nullopt;

    std::string full_key = std::string(KEY_PREFIX_GENERIC) + key;
    auto blob = secure_storage_->GetBlob(full_key);
    
    if (blob) {
        LogCredentialAccess("RETRIEVE", "Generic:" + key);
    }
    
    return blob;
}

bool CredentialManager::DeleteGenericCredential(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    std::string full_key = std::string(KEY_PREFIX_GENERIC) + key;
    LogCredentialAccess("DELETE", "Generic:" + key);
    return secure_storage_->Remove(full_key);
}

// ============================================================================
// Credential Management
// ============================================================================

bool CredentialManager::RotateAllKeys(const std::string& new_key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    spdlog::info("[CredentialManager] Rotating all encryption keys...");
    
    bool success = secure_storage_->RotateEncryptionKey(new_key);
    
    if (success) {
        LogCredentialAccess("ROTATE", "ALL_KEYS");
        spdlog::info("[CredentialManager] All keys rotated successfully");
    } else {
        spdlog::error("[CredentialManager] Key rotation failed");
    }
    
    return success;
}

std::optional<std::string> CredentialManager::ExportCredentials() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return std::nullopt;

    // Export all credentials as encrypted JSON
    json export_data;
    export_data["version"] = "1.0";
    export_data["exported_at"] = std::chrono::system_clock::now().time_since_epoch().count();
    
    auto keys = secure_storage_->ListKeys();
    json credentials = json::array();
    
    for (const auto& key : keys) {
        if (key.find("cred:") == 0) {
            auto value = secure_storage_->Get(key);
            if (value) {
                json item;
                item["key"] = key;
                item["value"] = *value;
                credentials.push_back(item);
            }
        }
    }
    
    export_data["credentials"] = credentials;
    LogCredentialAccess("EXPORT", "ALL");
    
    return export_data.dump();
}

bool CredentialManager::ImportCredentials(const std::string& encrypted_blob) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    try {
        auto import_data = json::parse(encrypted_blob);
        
        if (!import_data.contains("credentials")) {
            spdlog::error("[CredentialManager] Invalid import format");
            return false;
        }
        
        for (const auto& item : import_data["credentials"]) {
            std::string key = item["key"];
            std::string value = item["value"];
            secure_storage_->Set(key, value);
        }
        
        LogCredentialAccess("IMPORT", "ALL");
        spdlog::info("[CredentialManager] Credentials imported successfully");
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("[CredentialManager] Import failed: {}", e.what());
        return false;
    }
}

bool CredentialManager::ClearAllCredentials() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return false;

    spdlog::warn("[CredentialManager] Clearing all credentials (factory reset)");
    
    auto keys = secure_storage_->ListKeys();
    for (const auto& key : keys) {
        if (key.find("cred:") == 0) {
            secure_storage_->Remove(key);
        }
    }
    
    LogCredentialAccess("CLEAR", "ALL");
    return true;
}

std::string CredentialManager::GetCredentialStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsStorageAccessible()) return "{}";

    json stats;
    auto keys = secure_storage_->ListKeys();
    
    int smtp_count = 0, snmp_count = 0, wifi_count = 0, tls_count = 0;
    int api_count = 0, onvif_count = 0, rtsp_count = 0, generic_count = 0;
    
    for (const auto& key : keys) {
        if (key.find(KEY_PREFIX_SMTP) == 0) smtp_count++;
        else if (key.find(KEY_PREFIX_SNMP) == 0) snmp_count++;
        else if (key.find(KEY_PREFIX_WIFI) == 0) wifi_count++;
        else if (key.find(KEY_PREFIX_TLS) == 0) tls_count++;
        else if (key.find(KEY_PREFIX_API) == 0) api_count++;
        else if (key.find(KEY_PREFIX_ONVIF) == 0) onvif_count++;
        else if (key.find(KEY_PREFIX_RTSP) == 0) rtsp_count++;
        else if (key.find(KEY_PREFIX_GENERIC) == 0) generic_count++;
    }
    
    stats["smtp_credentials"] = smtp_count;
    stats["snmp_credentials"] = snmp_count;
    stats["wifi_credentials"] = wifi_count;
    stats["tls_certificates"] = tls_count;
    stats["api_credentials"] = api_count;
    stats["onvif_credentials"] = onvif_count;
    stats["rtsp_credentials"] = rtsp_count;
    stats["generic_credentials"] = generic_count;
    stats["total_credentials"] = smtp_count + snmp_count + wifi_count + tls_count + 
                                 api_count + onvif_count + rtsp_count + generic_count;
    
    return stats.dump(2);
}

void CredentialManager::LogCredentialAccess(const std::string& operation, const std::string& credential_type) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    spdlog::info("[CredentialManager] {} {} at {}", operation, credential_type, timestamp);
    
    // Store in audit log (last 100 entries)
    auto log_data = secure_storage_->Get(KEY_AUDIT_LOG);
    json log_entries;
    
    if (log_data) {
        try {
            log_entries = json::parse(*log_data);
        } catch (...) {
            log_entries = json::array();
        }
    } else {
        log_entries = json::array();
    }
    
    json entry;
    entry["timestamp"] = timestamp;
    entry["operation"] = operation;
    entry["credential_type"] = credential_type;
    
    log_entries.push_back(entry);
    
    // Keep only last 100 entries
    if (log_entries.size() > 100) {
        log_entries.erase(log_entries.begin());
    }
    
    secure_storage_->Set(KEY_AUDIT_LOG, log_entries.dump());
}

// ============================================================================
// Singleton Accessor
// ============================================================================

CredentialManager& GetCredentialManager() {
    static CredentialManager instance;
    return instance;
}

} // namespace storage
} // namespace ipcam
