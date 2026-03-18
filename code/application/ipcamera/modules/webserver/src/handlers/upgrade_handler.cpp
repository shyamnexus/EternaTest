/**
 * @file upgrade_handler.cpp
 * @brief Firmware upgrade API handler implementations
 * 
 * Supports direct streaming upload to physical RAM:
 * - Firmware is written directly to physical memory (no filesystem)
 * - Uses HDAL hd_common_mem_alloc for memory staging
 * - Flasher binary reads from RAM and writes to MTD
 * 
 * Flow:
 * 1. POST /api/v1/firmware/upload - Stream firmware directly to RAM
 * 2. POST /api/v1/firmware/upgrade - Launch flasher binary
 * 3. Flasher writes to MTD partitions and reboots
 */

#include "upgrade_handler.h"
#include <ipcam/upgrade_manager.h>
#include <ipcam/firmware_validator.h>
#include <ipcam/compat.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <fstream>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// Helper Functions
// ============================================================================

static std::string UpgradeStatusToString(upgrade::UpgradeStatus status) {
    switch (status) {
        case upgrade::UpgradeStatus::Idle: return "idle";
        case upgrade::UpgradeStatus::Downloading: return "downloading";
        case upgrade::UpgradeStatus::Validating: return "validating";
        case upgrade::UpgradeStatus::Staging: return "staging";
        case upgrade::UpgradeStatus::ReadyToReboot: return "ready_to_reboot";
        case upgrade::UpgradeStatus::Rebooting: return "rebooting";
        case upgrade::UpgradeStatus::Failed: return "failed";
        default: return "unknown";
    }
}

static std::string UpgradeErrorToString(upgrade::UpgradeError err) {
    switch (err) {
        case upgrade::UpgradeError::None: return "none";
        case upgrade::UpgradeError::InvalidHeader: return "invalid_header";
        case upgrade::UpgradeError::InvalidGuid: return "invalid_guid";
        case upgrade::UpgradeError::InvalidVersion: return "invalid_version";
        case upgrade::UpgradeError::ChecksumMismatch: return "checksum_mismatch";
        case upgrade::UpgradeError::SignatureVerifyFailed: return "signature_verify_failed";
        case upgrade::UpgradeError::DecryptionFailed: return "decryption_failed";
        case upgrade::UpgradeError::PartitionMismatch: return "partition_mismatch";
        case upgrade::UpgradeError::InsufficientMemory: return "insufficient_memory";
        case upgrade::UpgradeError::MemoryAllocationFailed: return "memory_allocation_failed";
        case upgrade::UpgradeError::FlashWriteFailed: return "flash_write_failed";
        case upgrade::UpgradeError::ReadbackVerifyFailed: return "readback_verify_failed";
        case upgrade::UpgradeError::ServiceShutdownFailed: return "service_shutdown_failed";
        case upgrade::UpgradeError::RebootFailed: return "reboot_failed";
        case upgrade::UpgradeError::DownloadFailed: return "download_failed";
        case upgrade::UpgradeError::FileOpenFailed: return "file_open_failed";
        case upgrade::UpgradeError::FileReadError: return "file_read_error";
        case upgrade::UpgradeError::WriteError: return "write_error";
        case upgrade::UpgradeError::FirmwareTooLarge: return "firmware_too_large";
        case upgrade::UpgradeError::InvalidState: return "invalid_state";
        case upgrade::UpgradeError::UpgradeInProgress: return "upgrade_in_progress";
        case upgrade::UpgradeError::Timeout: return "timeout";
        default: return "unknown";
    }
}

static json ValidationToJson(const upgrade::FirmwareValidation& validation) {
    json partitions = json::array();
    for (const auto& p : validation.partitions) {
        partitions.push_back({
            {"type", static_cast<int>(p.type)},
            {"offset", p.offset},
            {"size", p.size},
            {"checksumValid", p.checksumValid},
            {"signatureValid", p.signatureValid}
        });
    }
    
    return {
        {"valid", validation.valid},
        {"totalSize", validation.totalSize},
        {"partitionCount", validation.partitionCount},
        {"isSecureBoot", validation.isSecureBoot},
        {"errorMessage", validation.errorMessage},
        {"partitions", partitions}
    };
}

static bool FileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

static size_t GetFileSize(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return static_cast<size_t>(st.st_size);
    }
    return 0;
}

// ============================================================================
// GET /api/v1/firmware/version
// ============================================================================

api::Response HandleFirmwareVersion(const api::RequestContext& ctx) {
    spdlog::debug("HandleFirmwareVersion: {}", ctx.method);
    
    auto& mgr = upgrade::UpgradeManager::instance();
    
    // Read firmware version from /etc/firmware_version or similar
    std::string firmwareVersion = "unknown";
    std::ifstream verFile("/etc/firmware_version");
    if (verFile.is_open()) {
        std::getline(verFile, firmwareVersion);
    }
    
    auto status = mgr.status();
    
    json response = {
        {"firmwareVersion", firmwareVersion},
        {"status", UpgradeStatusToString(status)},
        {"upgradeInProgress", status != upgrade::UpgradeStatus::Idle && 
                              status != upgrade::UpgradeStatus::Failed}
    };
    
    return api::Response{200, "application/json", response.dump(), {}, ""};
}

// ============================================================================
// GET /api/v1/firmware/status
// ============================================================================

api::Response HandleFirmwareStatus(const api::RequestContext& ctx) {
    auto& mgr = upgrade::UpgradeManager::instance();
    auto status = mgr.status();
    
    json response = {
        {"status", UpgradeStatusToString(status)},
        {"progress", mgr.progressPercent()},
        {"message", mgr.progressMessage()}
    };
    
    if (status == upgrade::UpgradeStatus::Failed) {
        auto err = mgr.lastError();
        response["error"] = UpgradeErrorToString(err);
        response["errorMessage"] = mgr.lastErrorMessage();
    }
    
    // Add validation info if available
    auto* validation = mgr.validatedFirmware();
    if (validation && validation->valid) {
        response["validation"] = {
            {"partitionCount", validation->partitionCount},
            {"totalSize", validation->totalSize},
            {"isSecureBoot", validation->isSecureBoot}
        };
    }
    
    return api::Response{200, "application/json", response.dump(), {}, ""};
}

// ============================================================================
// POST /api/v1/firmware/upload
// 
// Streams firmware directly to physical RAM (no filesystem storage)
// This avoids running out of space on NAND filesystem.
// ============================================================================

api::Response HandleFirmwareUpload(const api::RequestContext& ctx) {
    spdlog::info("HandleFirmwareUpload: Streaming firmware directly to RAM");
    
    if (ctx.method != "POST") {
        return api::Response{405, "application/json", R"({"error":"Method not allowed"})", {}, ""};
    }
    
    auto& mgr = upgrade::UpgradeManager::instance();
    auto status = mgr.status();
    
    // Check if upgrade already in progress
    if (status != upgrade::UpgradeStatus::Idle && 
        status != upgrade::UpgradeStatus::Failed &&
        status != upgrade::UpgradeStatus::Downloading) {
        return api::Response{409, "application/json", R"({"error":"Upgrade already in progress"})", {}, ""};
    }
    
    // Get Content-Length for pre-allocation
    size_t contentLength = 0;
    auto it = ctx.headers.find("Content-Length");
    if (it != ctx.headers.end()) {
        contentLength = std::stoul(it->second);
    }
    
    size_t dataSize = ctx.body.size();
    if (dataSize == 0) {
        return api::Response{400, "application/json", R"({"error":"No firmware data in request"})", {}, ""};
    }
    
    spdlog::info("Firmware upload: {} bytes (Content-Length: {})", dataSize, contentLength);
    
    // If this is the first chunk or a new upload, begin streaming
    if (status == upgrade::UpgradeStatus::Idle || status == upgrade::UpgradeStatus::Failed) {
        // Quick header validation before allocating memory
        if (dataSize < 128) {
            return api::Response{400, "application/json", R"({"error":"Firmware data too small for header"})", {}, ""};
        }
        
        upgrade::FirmwareValidator validator;
        upgrade::Span<const uint8_t> headerData(
            reinterpret_cast<const uint8_t*>(ctx.body.data()),
            std::min(dataSize, size_t(4096)));
        
        auto headerValidation = validator.validateHeader(headerData, contentLength > 0 ? contentLength : dataSize);
        
        if (!headerValidation.valid) {
            json response = {
                {"success", false},
                {"error", headerValidation.errorMessage}
            };
            return api::Response{400, "application/json", response.dump(), {}, ""};
        }
        
        // Use expected size from Content-Length or from header validation
        size_t expectedSize = contentLength > 0 ? contentLength : headerValidation.totalSize;
        if (expectedSize == 0) {
            expectedSize = dataSize;  // Fallback to current chunk size
        }
        
        // Begin streaming upload - allocates physical RAM
        auto beginResult = mgr.beginStreamingUpload(expectedSize);
        if (!beginResult.has_value()) {
            json response = {
                {"success", false},
                {"error", "Failed to allocate staging memory"},
                {"errorCode", UpgradeErrorToString(beginResult.error())}
            };
            return api::Response{500, "application/json", response.dump(), {}, ""};
        }
        
        spdlog::info("Physical RAM allocated for firmware staging");
    }
    
    // Write chunk to physical RAM
    auto writeResult = mgr.writeChunk(ctx.body.data(), dataSize);
    if (!writeResult.has_value()) {
        mgr.cancelUpgrade();
        json response = {
            {"success", false},
            {"error", "Failed to write firmware chunk to staging memory"},
            {"errorCode", UpgradeErrorToString(writeResult.error())}
        };
        return api::Response{500, "application/json", response.dump(), {}, ""};
    }
    
    // Get staging memory info for response
    const auto* stagingInfo = mgr.getStagingMemory();
    
    // Check if upload is complete (single request case)
    // For chunked uploads, client should call /api/v1/firmware/finalize
    bool uploadComplete = (contentLength == 0 || 
                          (stagingInfo && stagingInfo->bytesWritten >= contentLength));
    
    json response;
    
    if (uploadComplete) {
        // Finalize and validate
        auto finalResult = mgr.finalizeUpload();
        
        if (!finalResult.has_value()) {
            json errorResponse = {
                {"success", false},
                {"error", "Firmware validation failed"},
                {"errorCode", UpgradeErrorToString(finalResult.error())}
            };
            return api::Response{400, "application/json", errorResponse.dump(), {}, ""};
        }
        
        response = {
            {"success", true},
            {"message", "Firmware uploaded and validated (staged in RAM)"},
            {"size", stagingInfo ? stagingInfo->bytesWritten : dataSize},
            {"physicalAddress", stagingInfo ? fmt::format("0x{:08X}", stagingInfo->physicalAddress) : ""},
            {"validation", ValidationToJson(finalResult.value())}
        };
    } else {
        // Partial upload - more chunks expected
        response = {
            {"success", true},
            {"message", "Chunk received"},
            {"bytesWritten", stagingInfo ? stagingInfo->bytesWritten : dataSize},
            {"expectedTotal", contentLength}
        };
    }
    
    return api::Response{200, "application/json", response.dump(), {}, ""};
}

// ============================================================================
// POST /api/v1/firmware/upgrade
// 
// Execute firmware upgrade. Firmware should already be staged in RAM
// via /api/v1/firmware/upload, or can specify a file path.
// ============================================================================

api::Response HandleFirmwareUpgrade(const api::RequestContext& ctx) {
    spdlog::info("HandleFirmwareUpgrade: Received upgrade request");
    
    if (ctx.method != "POST") {
        return api::Response{405, "application/json", R"({"error":"Method not allowed"})", {}, ""};
    }
    
    auto& mgr = upgrade::UpgradeManager::instance();
    auto status = mgr.status();
    
    // If firmware is already staged and validated, execute immediately
    if (status == upgrade::UpgradeStatus::ReadyToReboot) {
        const auto* stagingInfo = mgr.getStagingMemory();
        spdlog::info("Firmware already staged in RAM at 0x{:08X}, executing upgrade...",
                     stagingInfo ? stagingInfo->physicalAddress : 0);
        
        // Execute upgrade (launches flasher, kills self, doesn't return on success)
        auto err = mgr.executeUpgrade();
        
        // If we get here, something went wrong
        json response = {
            {"success", false},
            {"error", mgr.lastErrorMessage()},
            {"errorCode", UpgradeErrorToString(err)}
        };
        return api::Response{500, "application/json", response.dump(), {}, ""};
    }
    
    // Check if request body specifies a file path or URL
    std::string firmwarePath;
    
    if (!ctx.body.empty()) {
        try {
            auto body = json::parse(ctx.body);
            if (body.contains("file")) {
                firmwarePath = body["file"].get<std::string>();
            } else if (body.contains("url")) {
                // Handle URL-based upgrade
                std::string url = body["url"].get<std::string>();
                
                // Start async download and upgrade
                std::thread([url]() {
                    auto& mgr = upgrade::UpgradeManager::instance();
                    auto result = mgr.upgradeFromUrl(url);
                    if (!result.has_value()) {
                        spdlog::error("URL upgrade failed: {}", mgr.lastErrorMessage());
                    } else {
                        auto err = mgr.executeUpgrade();
                        if (err != upgrade::UpgradeError::None) {
                            spdlog::error("Execute upgrade failed: {}", mgr.lastErrorMessage());
                        }
                    }
                }).detach();
                
                json response = {
                    {"success", true},
                    {"message", "Upgrade started from URL: " + url}
                };
                return api::Response{200, "application/json", response.dump(), {}, ""};
            }
        } catch (const json::exception& e) {
            spdlog::warn("Failed to parse body as JSON: {}", e.what());
        }
    }
    
    // If no file specified and nothing staged, return error
    if (firmwarePath.empty()) {
        json response = {
            {"success", false},
            {"error", "No firmware ready. Upload via /api/v1/firmware/upload first."},
            {"hint", "POST firmware binary to /api/v1/firmware/upload, then POST to /api/v1/firmware/upgrade"}
        };
        return api::Response{400, "application/json", response.dump(), {}, ""};
    }
    
    // Check if file exists
    if (!FileExists(firmwarePath)) {
        json response = {
            {"success", false},
            {"error", "Specified firmware file not found: " + firmwarePath}
        };
        return api::Response{400, "application/json", response.dump(), {}, ""};
    }
    
    size_t fileSize = GetFileSize(firmwarePath);
    spdlog::info("Starting upgrade from file: {} ({} bytes)", firmwarePath, fileSize);
    
    // Start upgrade in background thread
    std::thread([firmwarePath]() {
        auto& mgr = upgrade::UpgradeManager::instance();
        auto result = mgr.upgradeFromFile(firmwarePath);
        if (!result.has_value()) {
            spdlog::error("Upgrade validation failed: {}", mgr.lastErrorMessage());
        } else {
            // Validation passed, execute upgrade (this will reboot)
            auto err = mgr.executeUpgrade();
            if (err != upgrade::UpgradeError::None) {
                spdlog::error("Execute upgrade failed: {}", mgr.lastErrorMessage());
            }
        }
    }).detach();
    
    json response = {
        {"success", true},
        {"message", "Upgrade started from " + firmwarePath},
        {"size", fileSize}
    };
    
    return api::Response{200, "application/json", response.dump(), {}, ""};
}

// ============================================================================
// POST /api/v1/firmware/validate
// ============================================================================

api::Response HandleFirmwareValidate(const api::RequestContext& ctx) {
    spdlog::info("HandleFirmwareValidate: Received validation request");
    
    if (ctx.method != "POST") {
        return api::Response{405, "application/json", R"({"error":"Method not allowed"})", {}, ""};
    }
    
    std::string firmwarePath;
    
    // Check if body specifies a file path
    if (!ctx.body.empty()) {
        try {
            auto body = json::parse(ctx.body);
            if (body.contains("file")) {
                firmwarePath = body["file"].get<std::string>();
            }
        } catch (const json::exception& e) {
            // Body might be raw firmware data
        }
    }
    
    upgrade::FirmwareValidator validator;
    upgrade::FirmwareValidation validation;
    
    // Configure validator with keys if available
    auto& mgr = upgrade::UpgradeManager::instance();
    const auto& config = mgr.config();
    if (!config.rsaPublicKeyN.empty()) {
        validator.setSecurityKeys(config.rsaPublicKeyN, 
                                  config.rsaPublicKeyE,
                                  config.aesKey);
    }
    
    bool requireSecure = (config.securityMode == upgrade::FirmwareSecurityMode::RequireSecure) ||
                         config.requireSecureBoot;
    
    if (!firmwarePath.empty()) {
        // Validate from file
        if (!FileExists(firmwarePath)) {
            return api::Response{400, "application/json", R"({"error":"Firmware file not found"})", {}, ""};
        }
        
        // Only read header for validation (avoid OOM on constrained devices)
        // Header is 128 bytes + partition headers (max ~2KB total)
        constexpr size_t MAX_HEADER_READ = 4096;
        
        std::ifstream file(firmwarePath, std::ios::binary);
        if (!file) {
            return api::Response{500, "application/json", R"({"error":"Failed to open firmware file"})", {}, ""};
        }
        
        // Get file size without reading entire file
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Read only header portion
        std::vector<uint8_t> headerData(std::min(fileSize, MAX_HEADER_READ));
        file.read(reinterpret_cast<char*>(headerData.data()), headerData.size());
        
        // Do header-only validation with actual file size
        validation = validator.validateHeader(headerData, fileSize);
    } else if (!ctx.body.empty()) {
        // Validate from body
        upgrade::Span<const uint8_t> data(
            reinterpret_cast<const uint8_t*>(ctx.body.data()),
            ctx.body.size());
        
        validation = validator.validateFull(data, requireSecure);
    } else {
        return api::Response{400, "application/json", R"({"error":"No firmware data or file path provided"})", {}, ""};
    }
    
    json response = ValidationToJson(validation);
    
    int statusCode = validation.valid ? 200 : 400;
    return api::Response{statusCode, "application/json", response.dump(), {}, ""};
}

// ============================================================================
// POST /api/v1/firmware/cancel
// ============================================================================

api::Response HandleFirmwareCancel(const api::RequestContext& ctx) {
    spdlog::info("HandleFirmwareCancel: Received cancel request");
    
    if (ctx.method != "POST") {
        return api::Response{405, "application/json", R"({"error":"Method not allowed"})", {}, ""};
    }
    
    auto& mgr = upgrade::UpgradeManager::instance();
    auto status = mgr.status();
    
    // Cannot cancel if rebooting
    if (status == upgrade::UpgradeStatus::Rebooting) {
        return api::Response{409, "application/json", R"({"error":"Cannot cancel during reboot"})", {}, ""};
    }
    
    if (status == upgrade::UpgradeStatus::Idle) {
        return api::Response{400, "application/json", R"({"error":"No upgrade in progress"})", {}, ""};
    }
    
    // Cancel releases staging memory (physical RAM)
    mgr.cancelUpgrade();
    
    json response = {
        {"success", true},
        {"message", "Upgrade cancelled, staging memory released"}
    };
    
    return api::Response{200, "application/json", response.dump(), {}, ""};
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
