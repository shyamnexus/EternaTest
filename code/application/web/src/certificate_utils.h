// Certificate utilities for SSL/TLS certificate management
// Copyright 2024 Honeywell International Inc.

#ifndef __CERTIFICATE_UTILS_H__
#define __CERTIFICATE_UTILS_H__

#include <string>
#include <nlohmann/json.hpp>

namespace rockchip {
namespace cgi {

// Certificate file paths
#define DEFAULT_CERT_PATH "/etc/nginx/ssl/Server.crt"
#define DEFAULT_KEY_PATH "/etc/nginx/ssl/Server.key"
#define CUSTOM_CERT_PATH "/etc/nginx/ssl/Custom.crt"
#define CUSTOM_KEY_PATH "/etc/nginx/ssl/Custom.key"
#define CERT_TYPE_FILE "/etc/nginx/ssl/cert_type.conf"
#define TEMP_CERT_PATH "/tmp/upload_cert.crt"
#define TEMP_KEY_PATH "/tmp/upload_key.key"

// Certificate information structure
struct CertificateInfo {
    std::string type;           // "default" or "custom"
    std::string subject;        // Certificate subject (DN)
    std::string issuer;         // Certificate issuer (DN)
    std::string valid_from;     // ISO 8601 date
    std::string valid_to;       // ISO 8601 date
    std::string fingerprint;    // SHA-256 fingerprint
    std::string serial_number;  // Serial number
    std::string signature_algorithm; // Signature algorithm
    int key_size;               // Key size in bits
    int version;                // X.509 version
};

/**
 * Get certificate information from a certificate file
 * @param cert_path Path to the certificate file
 * @param info Reference to CertificateInfo structure to fill
 * @return 0 on success, -1 on error
 */
int get_certificate_info(const char *cert_path, CertificateInfo &info);

/**
 * Get current certificate type (default or custom)
 * @return "default" or "custom"
 */
std::string get_current_cert_type();

/**
 * Set certificate type
 * @param type "default" or "custom"
 * @return 0 on success, -1 on error
 */
int set_cert_type(const std::string &type);

/**
 * Validate certificate and key match
 * @param cert_path Path to certificate file
 * @param key_path Path to key file
 * @return 0 if match, -1 if mismatch or error
 */
int validate_cert_key_pair(const char *cert_path, const char *key_path);

/**
 * Check if certificate is expired
 * @param cert_path Path to certificate file
 * @return 1 if expired, 0 if valid, -1 on error
 */
int is_cert_expired(const char *cert_path);

/**
 * Check if certificate will expire within days
 * @param cert_path Path to certificate file
 * @param days Number of days to check
 * @return 1 if expiring soon, 0 if not, -1 on error
 */
int is_cert_expiring_soon(const char *cert_path, int days);

/**
 * Install custom certificate files
 * @param cert_path Path to certificate file to install
 * @param key_path Path to key file to install
 * @return 0 on success, -1 on error
 */
int install_custom_certificate(const char *cert_path, const char *key_path);

/**
 * Delete custom certificate and revert to default
 * @return 0 on success, -1 on error
 */
int delete_custom_certificate();

/**
 * Reload Nginx to apply certificate changes
 * @return 0 on success, -1 on error
 */
int reload_nginx();

/**
 * Convert certificate info to JSON
 * @param info CertificateInfo structure
 * @return JSON object
 */
nlohmann::json cert_info_to_json(const CertificateInfo &info);

} // namespace cgi
} // namespace rockchip

#endif // __CERTIFICATE_UTILS_H__
