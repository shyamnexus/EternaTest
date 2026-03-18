#include "ipcam/ssl_manager.h"
#include "ipcam/network_manager.h"
#include <ipcam/config.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/asn1.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/err.h>

namespace ipcam {
namespace networking {

// Singleton implementation
SslManager& SslManager::Instance() {
    static SslManager instance;
    return instance;
}

SslManager::SslManager() = default;
SslManager::~SslManager() = default;

Result<void> SslManager::InitializeCertificates() {
    bool generate_on_boot = config::Get<bool>("network.https.generate_cert_on_boot", true);
    
    if (!generate_on_boot) {
        spdlog::info("SSL certificate generation disabled in config");
        return Result<void>::Ok();
    }
    
    std::string cert_path = config::Get<std::string>("network.https.ssl_certificate", "/etc/nginx/ssl/Server.crt");
    std::string key_path = config::Get<std::string>("network.https.ssl_certificate_key", "/etc/nginx/ssl/Server.key");
    std::string certs_dir = config::Get<std::string>("network.https.ssl_certs_dir", "/etc/nginx/ssl");
    
    // Create SSL directory if it doesn't exist
    mkdir(certs_dir.c_str(), 0755);
    
    // Check if certificates already exist
    struct stat cert_stat, key_stat;
    bool cert_exists = (stat(cert_path.c_str(), &cert_stat) == 0);
    bool key_exists = (stat(key_path.c_str(), &key_stat) == 0);
    
    if (cert_exists && key_exists) {
        spdlog::info("SSL certificates already exist, skipping generation");
        return Result<void>::Ok();
    }
    
    spdlog::info("Generating self-signed SSL certificate...");
    
    // Get hostname for certificate CN
    std::string hostname = "ipcamera";
    auto hostname_result = NetworkManager::Instance().GetHostname();
    if (hostname_result.success) {
        hostname = hostname_result.value;
    } else {
        spdlog::warn("Could not get hostname for SSL cert, using default: {}", hostname);
    }
    
    int validity_days = config::Get<int>("network.https.cert_validity_days", 3650);
    int key_bits = config::Get<int>("network.https.cert_key_bits", 2048);
    
    auto result = GenerateSelfSignedCertificate(cert_path, key_path, hostname, validity_days, key_bits);
    if (!result.success) {
        return Result<void>::Err("Failed to generate SSL certificate: " + result.error);
    }
    
    spdlog::info("SSL certificates generated successfully");
    return Result<void>::Ok();
}

Result<void> SslManager::GenerateSelfSignedCertificate(
    const std::string& cert_path,
    const std::string& key_path,
    const std::string& hostname,
    int validity_days,
    int key_bits)
{
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    // Generate RSA key pair
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) {
        return Result<void>::Err("Failed to create EVP_PKEY structure");
    }
    
    BIGNUM* bn = BN_new();
    if (!bn || !BN_set_word(bn, RSA_F4)) {
        EVP_PKEY_free(pkey);
        if (bn) BN_free(bn);
        return Result<void>::Err("Failed to create BIGNUM");
    }
    
    RSA* rsa = RSA_new();
    if (!rsa || !RSA_generate_key_ex(rsa, key_bits, bn, nullptr)) {
        BN_free(bn);
        EVP_PKEY_free(pkey);
        if (rsa) RSA_free(rsa);
        return Result<void>::Err("Failed to generate RSA key");
    }
    BN_free(bn);
    
    if (!EVP_PKEY_assign_RSA(pkey, rsa)) {
        RSA_free(rsa);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to assign RSA key to EVP_PKEY");
    }
    
    // Create X509 certificate
    X509* x509 = X509_new();
    if (!x509) {
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to create X509 structure");
    }
    
    // Set version (V3)
    X509_set_version(x509, 2);
    
    // Set serial number
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    
    // Set validity period
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), validity_days * 24 * 60 * 60);
    
    // Set public key
    X509_set_pubkey(x509, pkey);
    
    // Set subject name - use hostname for CN
    X509_NAME* name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "ST", MBSTRING_ASC, (unsigned char*)"State", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC, (unsigned char*)"City", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*)"IPCamera", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)hostname.c_str(), -1, -1, 0);
    
    // Set issuer name (same as subject for self-signed)
    X509_set_issuer_name(x509, name);
    
    // Add X509v3 extensions for browser compatibility
    X509V3_CTX x509v3_ctx;
    X509V3_set_ctx_nodb(&x509v3_ctx);
    X509V3_set_ctx(&x509v3_ctx, x509, x509, nullptr, nullptr, 0);
    
    // Basic Constraints: CA:FALSE (end-entity certificate)
    X509_EXTENSION* ext = X509V3_EXT_conf_nid(nullptr, &x509v3_ctx, NID_basic_constraints, 
                                               const_cast<char*>("CA:FALSE"));
    if (ext) {
        X509_add_ext(x509, ext, -1);
        X509_EXTENSION_free(ext);
    }
    
    // Key Usage: digitalSignature, keyEncipherment
    ext = X509V3_EXT_conf_nid(nullptr, &x509v3_ctx, NID_key_usage, 
                              const_cast<char*>("digitalSignature, keyEncipherment"));
    if (ext) {
        X509_add_ext(x509, ext, -1);
        X509_EXTENSION_free(ext);
    }
    
    // Subject Alternative Name: hostname, hostname.local, localhost
    // Get IP address for SAN
    std::string ip_address;
    auto ip_result = NetworkManager::Instance().GetIpv4("eth0");
    if (ip_result.success && !ip_result.value.address.empty() && ip_result.value.address != "0.0.0.0") {
        ip_address = ip_result.value.address;
    }
    
    std::string san_value = "DNS:" + hostname + ",DNS:" + hostname + ".local,DNS:localhost";
    if (!ip_address.empty()) {
        san_value += ",IP:" + ip_address;
    }
    
    ext = X509V3_EXT_conf_nid(nullptr, &x509v3_ctx, NID_subject_alt_name, 
                              const_cast<char*>(san_value.c_str()));
    if (ext) {
        X509_add_ext(x509, ext, -1);
        X509_EXTENSION_free(ext);
        spdlog::info("  SAN: {}", san_value);
    }
    
    // Sign the certificate
    if (!X509_sign(x509, pkey, EVP_sha256())) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to sign certificate");
    }
    
    // Write certificate to file
    FILE* cert_file = fopen(cert_path.c_str(), "wb");
    if (!cert_file) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to open certificate file for writing: " + cert_path);
    }
    
    if (!PEM_write_X509(cert_file, x509)) {
        fclose(cert_file);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to write certificate to file");
    }
    fclose(cert_file);
    
    // Write private key to file
    FILE* key_file = fopen(key_path.c_str(), "wb");
    if (!key_file) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to open key file for writing: " + key_path);
    }
    
    if (!PEM_write_PrivateKey(key_file, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
        fclose(key_file);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return Result<void>::Err("Failed to write private key to file");
    }
    fclose(key_file);
    
    // Set file permissions
    chmod(cert_path.c_str(), 0644);
    chmod(key_path.c_str(), 0600);
    
    // Cleanup
    X509_free(x509);
    EVP_PKEY_free(pkey);
    
    spdlog::info("Generated self-signed certificate:");
    spdlog::info("  Certificate: {}", cert_path);
    spdlog::info("  Private Key: {}", key_path);
    spdlog::info("  Validity: {} days", validity_days);
    spdlog::info("  Key Size: {} bits", key_bits);
    
    return Result<void>::Ok();
}

Result<void> SslManager::SetCertificate(const std::string& cert_path, const std::string& key_path) {
    if (cert_path.empty() || key_path.empty()) {
        return Result<void>::Err("Certificate and key paths cannot be empty");
    }
    
    // Verify files exist
    struct stat cert_stat, key_stat;
    if (stat(cert_path.c_str(), &cert_stat) != 0) {
        return Result<void>::Err("SSL certificate file not found: " + cert_path);
    }
    if (stat(key_path.c_str(), &key_stat) != 0) {
        return Result<void>::Err("SSL private key file not found: " + key_path);
    }
    
    // Update config
    config::Set<std::string>("network.https.ssl_certificate", cert_path);
    config::Set<std::string>("network.https.ssl_certificate_key", key_path);
    
    if (!config::Save()) {
        return Result<void>::Err("Failed to save configuration");
    }
    
    spdlog::info("SSL certificate configuration updated");
    return Result<void>::Ok();
}

Result<void> SslManager::UploadCertificate(
    const std::string& cert_content,
    const std::string& key_content,
    const std::string& cert_name)
{
    // Validate certificate and key first
    auto validation_result = ValidateCertificate(cert_content, key_content);
    if (!validation_result.success) {
        return validation_result;
    }
    
    std::string ssl_dir = config::Get<std::string>("network.https.ssl_certs_dir", "/etc/ssl");
    mkdir(ssl_dir.c_str(), 0755);
    
    std::string cert_path = ssl_dir + "/" + cert_name + ".crt";
    std::string key_path = ssl_dir + "/" + cert_name + ".key";
    
    // Write certificate to file
    std::ofstream cert_file(cert_path, std::ios::binary);
    if (!cert_file) {
        return Result<void>::Err("Failed to open certificate file for writing: " + cert_path);
    }
    cert_file << cert_content;
    cert_file.close();
    
    if (!cert_file.good()) {
        return Result<void>::Err("Failed to write certificate to file");
    }
    
    // Write private key to file
    std::ofstream key_file(key_path, std::ios::binary);
    if (!key_file) {
        unlink(cert_path.c_str()); // Cleanup certificate file
        return Result<void>::Err("Failed to open key file for writing: " + key_path);
    }
    key_file << key_content;
    key_file.close();
    
    if (!key_file.good()) {
        unlink(cert_path.c_str()); // Cleanup certificate file
        return Result<void>::Err("Failed to write private key to file");
    }
    
    // Set proper permissions
    chmod(cert_path.c_str(), 0644);
    chmod(key_path.c_str(), 0600);
    
    // Update configuration to use new certificate
    auto set_result = SetCertificate(cert_path, key_path);
    if (!set_result.success) {
        unlink(cert_path.c_str());
        unlink(key_path.c_str());
        return set_result;
    }
    
    spdlog::info("Custom SSL certificate uploaded successfully:");
    spdlog::info("  Certificate: {}", cert_path);
    spdlog::info("  Private Key: {}", key_path);
    
    return Result<void>::Ok();
}

Result<void> SslManager::ValidateCertificate(
    const std::string& cert_content,
    const std::string& key_content)
{
    if (cert_content.empty() || key_content.empty()) {
        return Result<void>::Err("Certificate and key content cannot be empty");
    }
    
    // Create BIO for certificate
    BIO* cert_bio = BIO_new_mem_buf(cert_content.data(), cert_content.size());
    if (!cert_bio) {
        return Result<void>::Err("Failed to create BIO for certificate");
    }
    
    // Parse certificate
    X509* cert = PEM_read_bio_X509(cert_bio, nullptr, nullptr, nullptr);
    BIO_free(cert_bio);
    
    if (!cert) {
        return Result<void>::Err("Invalid certificate format. Expected PEM-encoded X.509 certificate");
    }
    
    // Check certificate expiration
    ASN1_TIME* not_after = X509_get_notAfter(cert);
    if (X509_cmp_current_time(not_after) < 0) {
        X509_free(cert);
        return Result<void>::Err("Certificate has expired");
    }
    
    // Create BIO for private key
    BIO* key_bio = BIO_new_mem_buf(key_content.data(), key_content.size());
    if (!key_bio) {
        X509_free(cert);
        return Result<void>::Err("Failed to create BIO for private key");
    }
    
    // Parse private key
    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(key_bio, nullptr, nullptr, nullptr);
    BIO_free(key_bio);
    
    if (!pkey) {
        X509_free(cert);
        return Result<void>::Err("Invalid private key format. Expected PEM-encoded private key");
    }
    
    // Verify that the key matches the certificate
    EVP_PKEY* cert_pubkey = X509_get_pubkey(cert);
    if (!cert_pubkey) {
        EVP_PKEY_free(pkey);
        X509_free(cert);
        return Result<void>::Err("Failed to extract public key from certificate");
    }
    
    // Compare public keys
    int match = EVP_PKEY_cmp(cert_pubkey, pkey);
    EVP_PKEY_free(cert_pubkey);
    EVP_PKEY_free(pkey);
    X509_free(cert);
    
    if (match != 1) {
        return Result<void>::Err("Private key does not match the certificate");
    }
    
    return Result<void>::Ok();
}

Result<std::string> SslManager::GetCertificateInfo(const std::string& cert_path) {
    // Read certificate file
    FILE* cert_file = fopen(cert_path.c_str(), "r");
    if (!cert_file) {
        return Result<std::string>::Err("Failed to open certificate file: " + cert_path);
    }
    
    X509* cert = PEM_read_X509(cert_file, nullptr, nullptr, nullptr);
    fclose(cert_file);
    
    if (!cert) {
        return Result<std::string>::Err("Failed to parse certificate");
    }
    
    std::ostringstream info;
    info << "{";
    
    // Subject
    char* subject = X509_NAME_oneline(X509_get_subject_name(cert), nullptr, 0);
    if (subject) {
        info << "\"subject\":\"" << subject << "\",";
        OPENSSL_free(subject);
    }
    
    // Issuer
    char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), nullptr, 0);
    if (issuer) {
        info << "\"issuer\":\"" << issuer << "\",";
        OPENSSL_free(issuer);
    }
    
    // Validity dates
    BIO* bio = BIO_new(BIO_s_mem());
    if (bio) {
        ASN1_TIME_print(bio, X509_get_notBefore(cert));
        char buf[256];
        int len = BIO_read(bio, buf, sizeof(buf) - 1);
        buf[len] = '\0';
        info << "\"valid_from\":\"" << buf << "\",";
        
        BIO_reset(bio);
        ASN1_TIME_print(bio, X509_get_notAfter(cert));
        len = BIO_read(bio, buf, sizeof(buf) - 1);
        buf[len] = '\0';
        info << "\"valid_to\":\"" << buf << "\",";
        BIO_free(bio);
    }
    
    // Check if self-signed
    bool is_self_signed = (X509_check_issued(cert, cert) == X509_V_OK);
    info << "\"is_self_signed\":" << (is_self_signed ? "true" : "false");
    
    info << "}";
    
    X509_free(cert);
    return Result<std::string>::Ok(info.str());
}

} // namespace networking
} // namespace ipcam
