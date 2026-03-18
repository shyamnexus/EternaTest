// Certificate utilities implementation
// Copyright 2024 Honeywell International Inc.

#include "certificate_utils.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <iomanip>

namespace rockchip {
namespace cgi {

// Helper function to execute shell command and get output
static std::string exec_command(const char* cmd) {
    char buffer[256];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        Logger::getInstance().log(Logger::ERROR, "popen() failed!");
        return "";
    }
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Get certificate information using OpenSSL
int get_certificate_info(const char *cert_path, CertificateInfo &info) {
    FILE *fp = fopen(cert_path, "r");
    if (!fp) {
        Logger::getInstance().log(Logger::ERROR, "Failed to open certificate file: %s", cert_path);
        return -1;
    }

    X509 *cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);

    if (!cert) {
        Logger::getInstance().log(Logger::ERROR, "Failed to parse certificate");
        return -1;
    }

    // Get subject
    char *subject_str = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
    if (subject_str) {
        info.subject = subject_str;
        OPENSSL_free(subject_str);
    }

    // Get issuer
    char *issuer_str = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
    if (issuer_str) {
        info.issuer = issuer_str;
        OPENSSL_free(issuer_str);
    }

    // Get validity dates
    BIO *bio = BIO_new(BIO_s_mem());
    if (bio) {
        ASN1_TIME *not_before = X509_get_notBefore(cert);
        ASN1_TIME *not_after = X509_get_notAfter(cert);
        
        if (not_before) {
            ASN1_TIME_print(bio, not_before);
            char buf[256];
            int len = BIO_read(bio, buf, sizeof(buf) - 1);
            if (len > 0) {
                buf[len] = '\0';
                info.valid_from = buf;
            }
            BIO_reset(bio);
        }

        if (not_after) {
            ASN1_TIME_print(bio, not_after);
            char buf[256];
            int len = BIO_read(bio, buf, sizeof(buf) - 1);
            if (len > 0) {
                buf[len] = '\0';
                info.valid_to = buf;
            }
        }
        BIO_free(bio);
    }

    // Get serial number
    ASN1_INTEGER *serial = X509_get_serialNumber(cert);
    if (serial) {
        BIGNUM *bn = ASN1_INTEGER_to_BN(serial, NULL);
        if (bn) {
            char *hex = BN_bn2hex(bn);
            if (hex) {
                info.serial_number = hex;
                OPENSSL_free(hex);
            }
            BN_free(bn);
        }
    }

    // Get signature algorithm
    const X509_ALGOR *sig_alg;
    X509_get0_signature(NULL, &sig_alg, cert);
    if (sig_alg) {
        BIO *alg_bio = BIO_new(BIO_s_mem());
        if (alg_bio) {
            i2a_ASN1_OBJECT(alg_bio, sig_alg->algorithm);
            char alg_buf[128];
            int alg_len = BIO_read(alg_bio, alg_buf, sizeof(alg_buf) - 1);
            if (alg_len > 0) {
                alg_buf[alg_len] = '\0';
                info.signature_algorithm = alg_buf;
            }
            BIO_free(alg_bio);
        }
    }

    // Get fingerprint (SHA-256)
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int n;
    if (X509_digest(cert, EVP_sha256(), md, &n)) {
        std::stringstream ss;
        for (unsigned int i = 0; i < n; i++) {
            ss << std::hex << std::setfill('0') << std::setw(2) << (int)md[i];
            if (i < n - 1) ss << ":";
        }
        info.fingerprint = ss.str();
    }

    // Get public key size
    EVP_PKEY *pkey = X509_get_pubkey(cert);
    if (pkey) {
        info.key_size = EVP_PKEY_bits(pkey);
        EVP_PKEY_free(pkey);
    } else {
        info.key_size = 0;
    }

    // Get version
    info.version = X509_get_version(cert) + 1; // OpenSSL uses 0-based versioning

    X509_free(cert);
    return 0;
}

// Get current certificate type
std::string get_current_cert_type() {
    std::ifstream file(CERT_TYPE_FILE);
    if (file.is_open()) {
        std::string type;
        std::getline(file, type);
        file.close();
        
        // Trim whitespace
        type.erase(0, type.find_first_not_of(" \t\n\r"));
        type.erase(type.find_last_not_of(" \t\n\r") + 1);
        
        if (type == "custom" || type == "default") {
            return type;
        }
    }
    return "default"; // Default to default certificate
}

// Set certificate type
int set_cert_type(const std::string &type) {
    if (type != "default" && type != "custom") {
        Logger::getInstance().log(Logger::ERROR, "Invalid certificate type: %s", type.c_str());
        return -1;
    }

    std::ofstream file(CERT_TYPE_FILE);
    if (!file.is_open()) {
        Logger::getInstance().log(Logger::ERROR, "Failed to open cert type file for writing");
        return -1;
    }

    file << type << std::endl;
    file.close();

    Logger::getInstance().log(Logger::INFO, "Certificate type set to: %s", type.c_str());
    return 0;
}

// Validate certificate and key match
int validate_cert_key_pair(const char *cert_path, const char *key_path) {
    // Use OpenSSL to check if the certificate and key match
    FILE *cert_fp = fopen(cert_path, "r");
    if (!cert_fp) {
        Logger::getInstance().log(Logger::ERROR, "Failed to open certificate file");
        return -1;
    }

    X509 *cert = PEM_read_X509(cert_fp, NULL, NULL, NULL);
    fclose(cert_fp);

    if (!cert) {
        Logger::getInstance().log(Logger::ERROR, "Failed to parse certificate");
        return -1;
    }

    FILE *key_fp = fopen(key_path, "r");
    if (!key_fp) {
        Logger::getInstance().log(Logger::ERROR, "Failed to open key file");
        X509_free(cert);
        return -1;
    }

    EVP_PKEY *pkey = PEM_read_PrivateKey(key_fp, NULL, NULL, NULL);
    fclose(key_fp);

    if (!pkey) {
        Logger::getInstance().log(Logger::ERROR, "Failed to parse private key");
        X509_free(cert);
        return -1;
    }

    // Get public key from certificate
    EVP_PKEY *cert_pkey = X509_get_pubkey(cert);
    if (!cert_pkey) {
        Logger::getInstance().log(Logger::ERROR, "Failed to get public key from certificate");
        EVP_PKEY_free(pkey);
        X509_free(cert);
        return -1;
    }

    // Compare public keys
    int match = EVP_PKEY_cmp(cert_pkey, pkey);
    
    EVP_PKEY_free(cert_pkey);
    EVP_PKEY_free(pkey);
    X509_free(cert);

    if (match == 1) {
        Logger::getInstance().log(Logger::INFO, "Certificate and key match");
        return 0;
    } else {
        Logger::getInstance().log(Logger::ERROR, "Certificate and key do not match");
        return -1;
    }
}

// Check if certificate is expired
int is_cert_expired(const char *cert_path) {
    FILE *fp = fopen(cert_path, "r");
    if (!fp) {
        Logger::getInstance().log(Logger::ERROR, "Failed to open certificate file");
        return -1;
    }

    X509 *cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);

    if (!cert) {
        Logger::getInstance().log(Logger::ERROR, "Failed to parse certificate");
        return -1;
    }

    int result = (X509_cmp_current_time(X509_get_notAfter(cert)) <= 0) ? 1 : 0;
    X509_free(cert);

    return result;
}

// Check if certificate will expire within specified days
int is_cert_expiring_soon(const char *cert_path, int days) {
    FILE *fp = fopen(cert_path, "r");
    if (!fp) {
        Logger::getInstance().log(Logger::ERROR, "Failed to open certificate file");
        return -1;
    }

    X509 *cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);

    if (!cert) {
        Logger::getInstance().log(Logger::ERROR, "Failed to parse certificate");
        return -1;
    }

    ASN1_TIME *not_after = X509_get_notAfter(cert);
    time_t expiry_time;
    struct tm tm_time;
    
    memset(&tm_time, 0, sizeof(tm_time));
    ASN1_TIME_to_tm(not_after, &tm_time);
    expiry_time = mktime(&tm_time);

    time_t now = time(NULL);
    time_t threshold = now + (days * 24 * 60 * 60);

    X509_free(cert);

    return (expiry_time <= threshold && expiry_time > now) ? 1 : 0;
}

// Install custom certificate
int install_custom_certificate(const char *cert_path, const char *key_path) {
    // Validate certificate and key match
    if (validate_cert_key_pair(cert_path, key_path) != 0) {
        Logger::getInstance().log(Logger::ERROR, "Certificate and key do not match");
        return -1;
    }

    // Check if certificate is expired
    if (is_cert_expired(cert_path) == 1) {
        Logger::getInstance().log(Logger::ERROR, "Certificate has expired");
        return -1;
    }

    // Create ssl directory if it doesn't exist
    mkdir("/etc/nginx/ssl", 0755);

    // Copy certificate and key to custom location
    std::ifstream src_cert(cert_path, std::ios::binary);
    std::ofstream dst_cert(CUSTOM_CERT_PATH, std::ios::binary);
    if (!src_cert || !dst_cert) {
        Logger::getInstance().log(Logger::ERROR, "Failed to copy certificate");
        return -1;
    }
    dst_cert << src_cert.rdbuf();
    src_cert.close();
    dst_cert.close();

    std::ifstream src_key(key_path, std::ios::binary);
    std::ofstream dst_key(CUSTOM_KEY_PATH, std::ios::binary);
    if (!src_key || !dst_key) {
        Logger::getInstance().log(Logger::ERROR, "Failed to copy key");
        return -1;
    }
    dst_key << src_key.rdbuf();
    src_key.close();
    dst_key.close();

    // Set appropriate permissions
    chmod(CUSTOM_CERT_PATH, 0644);
    chmod(CUSTOM_KEY_PATH, 0600);

    // Update certificate type
    set_cert_type("custom");

    // Update Nginx symlinks
    unlink("/etc/nginx/ssl/current.crt");
    unlink("/etc/nginx/ssl/current.key");
    symlink(CUSTOM_CERT_PATH, "/etc/nginx/ssl/current.crt");
    symlink(CUSTOM_KEY_PATH, "/etc/nginx/ssl/current.key");

    Logger::getInstance().log(Logger::INFO, "Custom certificate installed successfully");

    // Reload Nginx
    return reload_nginx();
}

// Delete custom certificate
int delete_custom_certificate() {
    // Remove custom certificate files
    unlink(CUSTOM_CERT_PATH);
    unlink(CUSTOM_KEY_PATH);

    // Update certificate type to default
    set_cert_type("default");

    // Update Nginx symlinks to point to default
    unlink("/etc/nginx/ssl/current.crt");
    unlink("/etc/nginx/ssl/current.key");
    symlink(DEFAULT_CERT_PATH, "/etc/nginx/ssl/current.crt");
    symlink(DEFAULT_KEY_PATH, "/etc/nginx/ssl/current.key");

    Logger::getInstance().log(Logger::INFO, "Custom certificate deleted, reverted to default");

    // Reload Nginx
    return reload_nginx();
}

// Reload Nginx
int reload_nginx() {
    // Test Nginx configuration first
    int test_result = system("nginx -t 2>&1");
    if (test_result != 0) {
        Logger::getInstance().log(Logger::ERROR, "Nginx configuration test failed");
        return -1;
    }

    // Reload Nginx
    int result = system("/etc/init.d/S50nginx reload 2>&1");
    if (result == 0) {
        Logger::getInstance().log(Logger::INFO, "Nginx reloaded successfully");
        return 0;
    } else {
        Logger::getInstance().log(Logger::ERROR, "Failed to reload Nginx");
        return -1;
    }
}

// Convert certificate info to JSON
nlohmann::json cert_info_to_json(const CertificateInfo &info) {
    nlohmann::json json_info;
    
    json_info["type"] = info.type;
    json_info["subject"] = info.subject;
    json_info["issuer"] = info.issuer;
    json_info["valid_from"] = info.valid_from;
    json_info["valid_to"] = info.valid_to;
    json_info["fingerprint"] = info.fingerprint;
    json_info["serial_number"] = info.serial_number;
    json_info["signature_algorithm"] = info.signature_algorithm;
    json_info["key_size"] = info.key_size;
    json_info["version"] = info.version;

    return json_info;
}

} // namespace cgi
} // namespace rockchip
