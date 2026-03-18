#pragma once

#include "ipcam/result.h"
#include <string>

namespace ipcam {
namespace networking {

/**
 * @brief SslManager - Manages SSL/TLS certificates
 * 
 * Handles generation and configuration of SSL certificates independently
 * from nginx or other web servers.
 */
class SslManager {
public:
    static SslManager& Instance();
    
    /**
     * Initialize SSL certificates
     * - Creates SSL directory if needed
     * - Generates self-signed certificate if configured and not exists
     * @return Result with success or error message
     */
    Result<void> InitializeCertificates();
    
    /**
     * Generate a self-signed SSL certificate
     * @param cert_path Path to write certificate file
     * @param key_path Path to write private key file
     * @param hostname Hostname for certificate CN (e.g., hon-08c462)
     * @param validity_days Number of days certificate is valid
     * @param key_bits RSA key size in bits
     * @return Result with success or error message
     */
    Result<void> GenerateSelfSignedCertificate(
        const std::string& cert_path,
        const std::string& key_path,
        const std::string& hostname,
        int validity_days = 3650,
        int key_bits = 2048);
    
    /**
     * Set SSL certificate and key paths
     * @param cert_path Path to certificate file
     * @param key_path Path to private key file
     * @return Result with success or error message
     */
    Result<void> SetCertificate(const std::string& cert_path, const std::string& key_path);
    
    /**
     * Upload and install custom SSL certificate
     * @param cert_content PEM-encoded certificate content
     * @param key_content PEM-encoded private key content
     * @param cert_name Optional name for certificate (default: custom)
     * @return Result with success or error message
     */
    Result<void> UploadCertificate(
        const std::string& cert_content,
        const std::string& key_content,
        const std::string& cert_name = "custom");
    
    /**
     * Validate SSL certificate and key pair
     * @param cert_content PEM-encoded certificate content
     * @param key_content PEM-encoded private key content
     * @return Result with success or error message
     */
    Result<void> ValidateCertificate(
        const std::string& cert_content,
        const std::string& key_content);
    
    /**
     * Get SSL certificate information
     * @param cert_path Path to certificate file
     * @return Result with certificate details in JSON format
     */
    Result<std::string> GetCertificateInfo(const std::string& cert_path);

private:
    SslManager();
    ~SslManager();
    SslManager(const SslManager&) = delete;
    SslManager& operator=(const SslManager&) = delete;
};

} // namespace networking
} // namespace ipcam
