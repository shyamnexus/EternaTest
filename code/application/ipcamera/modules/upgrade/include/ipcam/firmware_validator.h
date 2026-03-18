/**
 * @file firmware_validator.h
 * @brief Secure boot firmware validation
 * 
 * Handles NVTPACK header parsing, RSA signature verification,
 * AES decryption, and per-partition checksum validation.
 */

#pragma once

#include "upgrade_types.h"
#include "compat.h"
#include <memory>

namespace ipcam::upgrade {

/**
 * @brief Firmware validator for secure boot firmware
 * 
 * Validates NVTPACK format firmware with optional secure boot extensions:
 * - NVTPACK header (GUID, version, checksum)
 * - RSA-2048 signature verification
 * - AES-128 decryption (CBC mode)
 * - Per-partition checksum validation
 */
class FirmwareValidator {
public:
    FirmwareValidator();
    ~FirmwareValidator();
    
    // Non-copyable
    FirmwareValidator(const FirmwareValidator&) = delete;
    FirmwareValidator& operator=(const FirmwareValidator&) = delete;
    
    /**
     * @brief Configure validator with security keys
     * @param rsaPublicKeyN RSA public key N (hex string from rsa_pub.txt)
     * @param rsaPublicKeyE RSA public key E (hex string)
     * @param aesKey AES key bytes (from aes.txt)
     */
    void setSecurityKeys(const std::string& rsaPublicKeyN,
                         const std::string& rsaPublicKeyE,
                         const std::array<uint8_t, 16>& aesKey);
    
    /**
     * @brief Validate firmware header only (quick check)
     * @param data Firmware data (can be header-only buffer)
     * @param actualFileSize Actual file size (0 = use data.size())
     * @return Validation result with header info
     */
    FirmwareValidation validateHeader(Span<const uint8_t> data, 
                                      size_t actualFileSize = 0) const;
    
    /**
     * @brief Full firmware validation including all partitions
     * @param data Firmware data
     * @param requireSecureBoot If true, fail if not signed
     * @return Complete validation result
     */
    FirmwareValidation validateFull(Span<const uint8_t> data,
                                    bool requireSecureBoot = true) const;
    
    /**
     * @brief Verify RSA signature of firmware
     * @param data Full firmware data
     * @param signatureOffset Offset of signature in data
     * @return True if signature is valid
     */
    bool verifySignature(Span<const uint8_t> data,
                         size_t signatureOffset) const;
    
    /**
     * @brief Verify NVTPACK checksum
     * @param header Firmware header
     * @param data Full firmware data
     * @return True if checksum matches
     */
    bool verifyChecksum(const NvtpackFwHeader2& header,
                        Span<const uint8_t> data) const;
    
    /**
     * @brief Verify per-partition checksum
     * @param partitionData Partition data
     * @param expectedChecksum Expected checksum value
     * @return True if checksum matches
     */
    bool verifyPartitionChecksum(Span<const uint8_t> partitionData,
                                  uint32_t expectedChecksum) const;
    
    /**
     * @brief Calculate 16-bit checksum (compatible with SDK)
     * Algorithm: Sum all 16-bit words, result should be 0 for valid data
     */
    static uint32_t calculateChecksum16(Span<const uint8_t> data);
    
    /**
     * @brief Calculate SHA-256 hash
     */
    static std::array<uint8_t, 32> calculateSha256(Span<const uint8_t> data);
    
    /**
     * @brief Parse partition headers from firmware
     * @param data Firmware data (after main header)
     * @param partitionCount Number of partitions
     * @return Vector of partition headers
     */
    std::vector<NvtpackPartitionHeader> parsePartitionHeaders(
        Span<const uint8_t> data,
        uint32_t partitionCount) const;
    
    /**
     * @brief Check if firmware is for this board/chip
     * @param header Firmware header
     * @return True if compatible
     */
    bool checkBoardCompatibility(const NvtpackFwHeader2& header) const;
    
    /**
     * @brief Decrypt encrypted partition (AES-128-CBC)
     * @param encryptedData Encrypted data
     * @param decryptedData Output buffer (must be same size)
     * @return True if decryption successful
     */
    bool decryptPartition(Span<const uint8_t> encryptedData,
                          Span<uint8_t> decryptedData) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ipcam::upgrade
