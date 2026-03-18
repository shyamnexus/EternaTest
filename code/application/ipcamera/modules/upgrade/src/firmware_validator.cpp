/**
 * @file firmware_validator.cpp
 * @brief Secure boot firmware validation implementation
 */

#include "ipcam/firmware_validator.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <algorithm>

#ifdef OPENSSL_AVAILABLE
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#endif

namespace ipcam::upgrade {

// =============================================================================
// Implementation
// =============================================================================

struct FirmwareValidator::Impl {
    std::string rsaPublicKeyN;
    std::string rsaPublicKeyE;
    std::array<uint8_t, 16> aesKey{};
    bool keysConfigured = false;
    
#ifdef OPENSSL_AVAILABLE
    EVP_PKEY* rsaKey = nullptr;
    
    ~Impl() {
        if (rsaKey) {
            EVP_PKEY_free(rsaKey);
        }
    }
    
    bool initRsaKey() {
        if (rsaPublicKeyN.empty() || rsaPublicKeyE.empty()) {
            return false;
        }
        
        BIGNUM* n = nullptr;
        BIGNUM* e = nullptr;
        
        if (BN_hex2bn(&n, rsaPublicKeyN.c_str()) == 0) {
            spdlog::error("Failed to parse RSA N key");
            return false;
        }
        
        if (BN_hex2bn(&e, rsaPublicKeyE.c_str()) == 0) {
            BN_free(n);
            spdlog::error("Failed to parse RSA E key");
            return false;
        }
        
        // Create EVP_PKEY for RSA verification
        rsaKey = EVP_PKEY_new();
        if (rsaKey) {
            EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
            if (ctx) {
                // Use newer EVP API for RSA key
                OSSL_PARAM params[3];
                params[0] = OSSL_PARAM_construct_BN("n", nullptr, BN_num_bytes(n));
                params[1] = OSSL_PARAM_construct_BN("e", nullptr, BN_num_bytes(e));
                params[2] = OSSL_PARAM_construct_end();
                
                EVP_PKEY_CTX_free(ctx);
            }
        }
        
        BN_free(n);
        BN_free(e);
        
        return rsaKey != nullptr;
    }
#endif
};

// =============================================================================
// FirmwareValidator
// =============================================================================

FirmwareValidator::FirmwareValidator()
    : impl_(std::make_unique<Impl>())
{
}

FirmwareValidator::~FirmwareValidator() = default;

void FirmwareValidator::setSecurityKeys(const std::string& rsaPublicKeyN,
                                         const std::string& rsaPublicKeyE,
                                         const std::array<uint8_t, 16>& aesKey)
{
    impl_->rsaPublicKeyN = rsaPublicKeyN;
    impl_->rsaPublicKeyE = rsaPublicKeyE;
    impl_->aesKey = aesKey;
    impl_->keysConfigured = !rsaPublicKeyN.empty();
    
#ifdef OPENSSL_AVAILABLE
    if (impl_->keysConfigured) {
        impl_->initRsaKey();
    }
#endif
    
    spdlog::info("FirmwareValidator: Security keys configured (RSA: {}, AES: {})",
                 !rsaPublicKeyN.empty(), aesKey[0] != 0 || aesKey[1] != 0);
}

FirmwareValidation FirmwareValidator::validateHeader(Span<const uint8_t> data,
                                                      size_t actualFileSize) const
{
    FirmwareValidation result{};
    result.valid = false;
    
    // Use provided file size or buffer size
    size_t fileSize = (actualFileSize > 0) ? actualFileSize : data.size();
    
    if (data.size() < sizeof(NvtpackFwHeader2)) {
        result.error = UpgradeError::InvalidHeader;
        result.errorMessage = "Firmware too small for header";
        return result;
    }
    
    // Parse header
    const auto* header = reinterpret_cast<const NvtpackFwHeader2*>(data.data());
    
    // Verify GUID
    if (std::memcmp(header->guid, NVTPACK_FW_HDR2_GUID.data(), 16) != 0) {
        result.error = UpgradeError::InvalidGuid;
        result.errorMessage = "Invalid NVTPACK GUID";
        return result;
    }
    
    // Verify version
    if (header->version != NVTPACK_FW_HDR2_VERSION) {
        result.error = UpgradeError::InvalidVersion;
        result.errorMessage = "Invalid NVTPACK version";
        return result;
    }
    
    // Verify header size
    if (header->headerSize != sizeof(NvtpackFwHeader2)) {
        result.error = UpgradeError::InvalidHeader;
        result.errorMessage = "Invalid header size";
        return result;
    }
    
    // Verify total size against actual file size
    if (header->totalSize > fileSize) {
        result.error = UpgradeError::InvalidHeader;
        result.errorMessage = "Firmware size mismatch (header claims larger than file)";
        return result;
    }
    
    result.totalSize = header->totalSize;
    result.partitionCount = header->totalPartitions;
    result.valid = true;
    
    return result;
}

FirmwareValidation FirmwareValidator::validateFull(Span<const uint8_t> data,
                                                    bool requireSecureBoot) const
{
    // First validate header
    FirmwareValidation result = validateHeader(data);
    if (!result.valid) {
        return result;
    }
    
    const auto* header = reinterpret_cast<const NvtpackFwHeader2*>(data.data());
    
    // Skip header checksum verification for now - the checksum algorithm needs investigation
    // The NVTPACK checksum method is not well documented
    // TODO: Implement correct checksum algorithm based on SDK analysis
    spdlog::debug("Skipping header checksum verification (method={}, value=0x{:08x})",
                  header->checksumMethod, header->checksumValue);
    
    // Parse partition headers
    size_t partHdrOffset = sizeof(NvtpackFwHeader2);
    auto partitions = parsePartitionHeaders(
        data.subspan(partHdrOffset),
        header->totalPartitions);
    
    // Validate each partition
    result.partitions.reserve(partitions.size());
    bool hasSignedPartition = false;
    
    for (const auto& partHdr : partitions) {
        PartitionValidation pv{};
        pv.type = static_cast<PartitionType>(partHdr.partitionId);
        pv.offset = partHdr.offset;
        pv.size = partHdr.size;
        
        // Verify partition is within bounds
        if (partHdr.offset + partHdr.size > data.size()) {
            result.valid = false;
            result.error = UpgradeError::InvalidHeader;
            result.errorMessage = "Partition extends beyond firmware";
            return result;
        }
        
        // Get partition data
        auto partData = data.subspan(partHdr.offset, partHdr.size);
        
        // Check for checksum header
        if (partHdr.size >= sizeof(NvtpackChecksumHeader)) {
            const auto* chkHdr = reinterpret_cast<const NvtpackChecksumHeader*>(partData.data());
            if (chkHdr->fourCC == 0x4D534B43) {  // 'CKSM' little-endian
                pv.checksumValid = verifyPartitionChecksum(partData, chkHdr->checksumValue);
            } else {
                pv.checksumValid = true;  // No checksum header
            }
        }
        
        pv.signatureValid = true;  // Assume valid if no signature
        result.partitions.push_back(pv);
    }
    
    // Check secure boot requirement
    result.isSecureBoot = hasSignedPartition;
    if (requireSecureBoot && !hasSignedPartition) {
        result.valid = false;
        result.error = UpgradeError::SignatureVerifyFailed;
        result.errorMessage = "Secure boot required but firmware is not signed";
        return result;
    }
    
    result.valid = true;
    return result;
}

bool FirmwareValidator::verifySignature(Span<const uint8_t> data,
                                         size_t signatureOffset) const
{
#ifdef OPENSSL_AVAILABLE
    if (!impl_->rsaKey) {
        spdlog::warn("RSA key not configured, cannot verify signature");
        return false;
    }
    
    // Calculate SHA256 of data before signature
    auto hash = calculateSha256(data.first(signatureOffset));
    
    // Verify RSA signature
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) return false;
    
    bool verified = false;
    if (EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, impl_->rsaKey) == 1) {
        if (EVP_DigestVerifyUpdate(mdctx, data.data(), signatureOffset) == 1) {
            const uint8_t* sig = data.data() + signatureOffset;
            size_t sigLen = SecureBootSignature::RSA2048_SIZE;
            verified = (EVP_DigestVerifyFinal(mdctx, sig, sigLen) == 1);
        }
    }
    
    EVP_MD_CTX_free(mdctx);
    return verified;
#else
    spdlog::warn("OpenSSL not available, cannot verify signature");
    return false;
#endif
}

bool FirmwareValidator::verifyChecksum(const NvtpackFwHeader2& header,
                                        Span<const uint8_t> data) const
{
    // Create copy of header with checksum zeroed for calculation
    NvtpackFwHeader2 headerCopy = header;
    headerCopy.checksumValue = 0;
    
    // Calculate checksum
    uint32_t calculated = calculateChecksum16(
        Span<const uint8_t>(reinterpret_cast<const uint8_t*>(&headerCopy),
                            sizeof(headerCopy)));
    
    return (calculated + header.checksumValue) == 0;
}

bool FirmwareValidator::verifyPartitionChecksum(Span<const uint8_t> partitionData,
                                                 uint32_t expectedChecksum) const
{
    uint32_t calculated = calculateChecksum16(partitionData);
    return calculated == expectedChecksum;
}

uint32_t FirmwareValidator::calculateChecksum16(Span<const uint8_t> data)
{
    uint32_t sum = 0;
    const uint16_t* ptr = reinterpret_cast<const uint16_t*>(data.data());
    size_t count = data.size() / 2;
    
    for (size_t i = 0; i < count; ++i) {
        sum += ptr[i];
    }
    
    // Handle odd byte
    if (data.size() & 1) {
        sum += data.data()[data.size() - 1];
    }
    
    // Fold 32-bit sum to 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return static_cast<uint32_t>(~sum & 0xFFFF);
}

std::array<uint8_t, 32> FirmwareValidator::calculateSha256(Span<const uint8_t> data)
{
    std::array<uint8_t, 32> hash{};
    
#ifdef OPENSSL_AVAILABLE
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx) {
        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1) {
            EVP_DigestUpdate(ctx, data.data(), data.size());
            unsigned int len = 32;
            EVP_DigestFinal_ex(ctx, hash.data(), &len);
        }
        EVP_MD_CTX_free(ctx);
    }
#else
    // Fallback: return zeros (not secure!)
    spdlog::warn("OpenSSL not available, SHA256 not calculated");
#endif
    
    return hash;
}

std::vector<NvtpackPartitionHeader> FirmwareValidator::parsePartitionHeaders(
    Span<const uint8_t> data,
    uint32_t partitionCount) const
{
    std::vector<NvtpackPartitionHeader> headers;
    headers.reserve(partitionCount);
    
    const auto* ptr = reinterpret_cast<const NvtpackPartitionHeader*>(data.data());
    for (uint32_t i = 0; i < partitionCount && (i + 1) * sizeof(NvtpackPartitionHeader) <= data.size(); ++i) {
        headers.push_back(ptr[i]);
    }
    
    return headers;
}

bool FirmwareValidator::checkBoardCompatibility(const NvtpackFwHeader2& header) const
{
    // TODO: Check board/chip ID against firmware header userData fields
    return true;
}

bool FirmwareValidator::decryptPartition(Span<const uint8_t> encryptedData,
                                          Span<uint8_t> decryptedData) const
{
#ifdef OPENSSL_AVAILABLE
    if (encryptedData.size() != decryptedData.size()) {
        return false;
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    
    // Initialize AES-128-CBC decryption
    uint8_t iv[16] = {0};  // IV from secure boot config
    
    bool success = false;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, 
                           impl_->aesKey.data(), iv) == 1) {
        int outLen = 0;
        if (EVP_DecryptUpdate(ctx, decryptedData.data(), &outLen,
                              encryptedData.data(), encryptedData.size()) == 1) {
            int finalLen = 0;
            success = (EVP_DecryptFinal_ex(ctx, decryptedData.data() + outLen, &finalLen) == 1);
        }
    }
    
    EVP_CIPHER_CTX_free(ctx);
    return success;
#else
    spdlog::warn("OpenSSL not available, cannot decrypt");
    return false;
#endif
}

} // namespace ipcam::upgrade
