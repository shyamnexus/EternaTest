/**
 * @file ftp_manager.cpp
 * @brief FTP Manager Implementation
 */

#include "ipcam/ftp_manager.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>

namespace ipcam {
namespace storage {

using json = nlohmann::json;

// ============================================================================
// Helper Functions
// ============================================================================

static std::string GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

static std::string GenerateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << (8 + dis(gen) % 4);
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    
    return ss.str();
}

// CURL callbacks
static size_t ReadCallback(char* ptr, size_t size, size_t nmemb, void* stream) {
    FILE* file = static_cast<FILE*>(stream);
    return fread(ptr, size, nmemb, file);
}

static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                            curl_off_t ultotal, curl_off_t ulnow) {
    auto* item = static_cast<FtpUploadItem*>(clientp);
    if (item && ultotal > 0) {
        item->bytes_uploaded = ulnow;
    }
    return 0;  // Return non-zero to abort
}

// ============================================================================
// JSON Serialization
// ============================================================================

static json FtpConfigToJson(const FtpConfig& config) {
    return json{
        {"enabled", config.enabled},
        {"server", config.server},
        {"port", config.port},
        {"username", config.username},
        {"password", config.password},
        {"remote_path", config.remote_path},
        {"use_ssl", config.use_ssl},
        {"passive_mode", config.passive_mode},
        {"upload_schedule", FtpUploadScheduleToString(config.upload_schedule)},
        {"upload_hour", config.upload_hour},
        {"upload_recordings", config.upload_recordings},
        {"upload_snapshots", config.upload_snapshots},
        {"delete_after_upload", config.delete_after_upload},
        {"max_retries", config.max_retries},
        {"timeout_seconds", config.timeout_seconds}
    };
}

static FtpConfig JsonToFtpConfig(const json& j) {
    FtpConfig config;
    config.enabled = j.value("enabled", false);
    config.server = j.value("server", "");
    config.port = j.value("port", 21);
    config.username = j.value("username", "");
    config.password = j.value("password", "");
    config.remote_path = j.value("remote_path", "/uploads");
    config.use_ssl = j.value("use_ssl", false);
    config.passive_mode = j.value("passive_mode", true);
    config.upload_schedule = StringToFtpUploadSchedule(j.value("upload_schedule", "immediate"));
    config.upload_hour = j.value("upload_hour", 2);
    config.upload_recordings = j.value("upload_recordings", true);
    config.upload_snapshots = j.value("upload_snapshots", false);
    config.delete_after_upload = j.value("delete_after_upload", false);
    config.max_retries = j.value("max_retries", 3);
    config.timeout_seconds = j.value("timeout_seconds", 30);
    return config;
}

// ============================================================================
// FtpManager Implementation
// ============================================================================

FtpManager& FtpManager::Instance() {
    static FtpManager instance;
    return instance;
}

bool FtpManager::Init() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("[FtpManager] Already initialized");
        return true;
    }
    
    // Initialize CURL
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        spdlog::error("[FtpManager] Failed to initialize CURL");
        return false;
    }
    
    // Load config from config system
    LoadConfig();
    
    // Start worker thread
    shutdown_requested_.store(false);
    worker_thread_ = std::make_unique<std::thread>(&FtpManager::UploadWorker, this);
    
    initialized_.store(true);
    spdlog::info("[FtpManager] Initialized");
    return true;
}

void FtpManager::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_.load()) {
            return;
        }
        initialized_.store(false);
    }
    
    // Stop worker thread
    shutdown_requested_.store(true);
    upload_cv_.notify_all();
    
    if (worker_thread_ && worker_thread_->joinable()) {
        worker_thread_->join();
    }
    worker_thread_.reset();
    
    // Cleanup CURL
    curl_global_cleanup();
    
    SaveConfig();
    spdlog::info("[FtpManager] Shutdown complete");
}

FtpConfig FtpManager::GetConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool FtpManager::SetConfig(const FtpConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    config_.status = FtpStatus::Disconnected;
    
    if (SaveConfig()) {
        spdlog::info("[FtpManager] Configuration updated");
        return true;
    }
    return false;
}

FtpStatus FtpManager::GetStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.status;
}

FtpStats FtpManager::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    FtpStats stats = stats_;
    
    std::lock_guard<std::mutex> queue_lock(queue_mutex_);
    stats.pending_uploads = static_cast<int>(upload_queue_.size());
    
    return stats;
}

bool FtpManager::TestConnection(const FtpConfig* config) {
    const FtpConfig& cfg = config ? *config : config_;
    
    if (cfg.server.empty()) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.last_error = "Server address is empty";
        config_.status = FtpStatus::Error;
        return false;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.last_error = "Failed to initialize CURL";
        config_.status = FtpStatus::Error;
        return false;
    }
    
    // Build FTP URL
    std::string url = BuildFtpUrl("/");
    if (config) {
        std::stringstream ss;
        ss << (cfg.use_ssl ? "ftps://" : "ftp://");
        ss << cfg.server << ":" << cfg.port << cfg.remote_path << "/";
        url = ss.str();
    }
    
    // Set options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  // Just check, don't download
    curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(cfg.timeout_seconds));
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    
    if (!cfg.username.empty()) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, cfg.username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, cfg.password.c_str());
    }
    
    if (cfg.use_ssl) {
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    
    if (!cfg.passive_mode) {
        curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    std::string error_msg;
    if (res != CURLE_OK) {
        error_msg = curl_easy_strerror(res);
    }
    
    curl_easy_cleanup(curl);
    
    std::lock_guard<std::mutex> lock(mutex_);
    if (res == CURLE_OK) {
        config_.status = FtpStatus::Connected;
        config_.last_error = "";
        spdlog::info("[FtpManager] Connection test successful to {}", cfg.server);
        return true;
    } else {
        config_.status = FtpStatus::Error;
        config_.last_error = error_msg;
        spdlog::error("[FtpManager] Connection test failed: {}", error_msg);
        return false;
    }
}

std::string FtpManager::GetLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.last_error;
}

std::string FtpManager::QueueUpload(const std::string& local_path, 
                                     const std::string& remote_filename) {
    // Verify file exists
    if (!std::filesystem::exists(local_path)) {
        spdlog::error("[FtpManager] File not found: {}", local_path);
        return "";
    }
    
    FtpUploadItem item;
    item.id = GenerateUploadId();
    item.local_path = local_path;
    item.remote_path = remote_filename.empty() ? 
                       std::filesystem::path(local_path).filename().string() : 
                       remote_filename;
    item.status = UploadStatus::Pending;
    item.file_size = std::filesystem::file_size(local_path);
    item.created_at = GetCurrentTimestamp();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        upload_queue_.push(item);
    }
    
    // Notify worker if immediate upload
    if (config_.upload_schedule == FtpUploadSchedule::Immediate) {
        upload_cv_.notify_one();
    }
    
    spdlog::info("[FtpManager] Queued upload: {} ({})", item.local_path, item.id);
    return item.id;
}

bool FtpManager::CancelUpload(const std::string& upload_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    // Can't cancel from std::queue directly, need to rebuild
    std::queue<FtpUploadItem> new_queue;
    bool found = false;
    
    while (!upload_queue_.empty()) {
        auto item = upload_queue_.front();
        upload_queue_.pop();
        
        if (item.id == upload_id) {
            found = true;
            spdlog::info("[FtpManager] Cancelled upload: {}", upload_id);
        } else {
            new_queue.push(item);
        }
    }
    
    upload_queue_ = std::move(new_queue);
    return found;
}

void FtpManager::TriggerUpload() {
    spdlog::info("[FtpManager] Manual upload triggered");
    upload_cv_.notify_one();
}

std::vector<FtpUploadItem> FtpManager::GetPendingUploads() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    std::vector<FtpUploadItem> items;
    std::queue<FtpUploadItem> temp = upload_queue_;
    
    while (!temp.empty()) {
        items.push_back(temp.front());
        temp.pop();
    }
    
    return items;
}

std::vector<FtpUploadItem> FtpManager::GetUploadHistory(int max_items) const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    if (upload_history_.size() <= static_cast<size_t>(max_items)) {
        return upload_history_;
    }
    
    return std::vector<FtpUploadItem>(
        upload_history_.end() - max_items,
        upload_history_.end()
    );
}

void FtpManager::SetProgressCallback(UploadProgressCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    progress_callback_ = callback;
}

void FtpManager::SetCompleteCallback(UploadCompleteCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    complete_callback_ = callback;
}

bool FtpManager::SaveConfig() {
    // Save to unified config system
    ipcam::config::Set("recording.ftp.enabled", config_.enabled);
    ipcam::config::Set("recording.ftp.server", config_.server);
    ipcam::config::Set("recording.ftp.port", config_.port);
    ipcam::config::Set("recording.ftp.username", config_.username);
    ipcam::config::Set("recording.ftp.password", config_.password);
    ipcam::config::Set("recording.ftp.remote_path", config_.remote_path);
    ipcam::config::Set("recording.ftp.use_ssl", config_.use_ssl);
    ipcam::config::Set("recording.ftp.passive_mode", config_.passive_mode);
    ipcam::config::Set("recording.ftp.upload_schedule", FtpUploadScheduleToString(config_.upload_schedule));
    ipcam::config::Set("recording.ftp.upload_hour", config_.upload_hour);
    ipcam::config::Set("recording.ftp.upload_recordings", config_.upload_recordings);
    ipcam::config::Set("recording.ftp.upload_snapshots", config_.upload_snapshots);
    ipcam::config::Set("recording.ftp.delete_after_upload", config_.delete_after_upload);
    ipcam::config::Set("recording.ftp.max_retries", config_.max_retries);
    ipcam::config::Set("recording.ftp.timeout_seconds", config_.timeout_seconds);
    
    if (ipcam::config::Save()) {
        spdlog::debug("[FtpManager] Saved config to config system");
        return true;
    }
    spdlog::error("[FtpManager] Failed to save config to config system");
    return false;
}

bool FtpManager::LoadConfig() {
    // Load from unified config system
    config_.enabled = ipcam::config::Get<bool>("recording.ftp.enabled", false);
    config_.server = ipcam::config::Get<std::string>("recording.ftp.server", "");
    config_.port = ipcam::config::Get<int>("recording.ftp.port", 21);
    config_.username = ipcam::config::Get<std::string>("recording.ftp.username", "");
    config_.password = ipcam::config::Get<std::string>("recording.ftp.password", "");
    config_.remote_path = ipcam::config::Get<std::string>("recording.ftp.remote_path", "/uploads");
    config_.use_ssl = ipcam::config::Get<bool>("recording.ftp.use_ssl", false);
    config_.passive_mode = ipcam::config::Get<bool>("recording.ftp.passive_mode", true);
    
    std::string schedule = ipcam::config::Get<std::string>("recording.ftp.upload_schedule", "immediate");
    config_.upload_schedule = StringToFtpUploadSchedule(schedule);
    
    config_.upload_hour = ipcam::config::Get<int>("recording.ftp.upload_hour", 2);
    config_.upload_recordings = ipcam::config::Get<bool>("recording.ftp.upload_recordings", true);
    config_.upload_snapshots = ipcam::config::Get<bool>("recording.ftp.upload_snapshots", false);
    config_.delete_after_upload = ipcam::config::Get<bool>("recording.ftp.delete_after_upload", false);
    config_.max_retries = ipcam::config::Get<int>("recording.ftp.max_retries", 3);
    config_.timeout_seconds = ipcam::config::Get<int>("recording.ftp.timeout_seconds", 30);
    
    spdlog::info("[FtpManager] Loaded config from config system");
    return true;
}

void FtpManager::UploadWorker() {
    spdlog::info("[FtpManager] Upload worker started");
    
    while (!shutdown_requested_.load()) {
        FtpUploadItem item;
        bool has_item = false;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // Wait for work or shutdown
            upload_cv_.wait_for(lock, std::chrono::seconds(60), [this]() {
                return shutdown_requested_.load() || !upload_queue_.empty();
            });
            
            if (shutdown_requested_.load()) {
                break;
            }
            
            // Check schedule
            if (!upload_queue_.empty()) {
                if (config_.upload_schedule == FtpUploadSchedule::Immediate ||
                    IsScheduledUploadTime()) {
                    item = upload_queue_.front();
                    upload_queue_.pop();
                    has_item = true;
                }
            }
        }
        
        if (has_item && config_.enabled) {
            item.status = UploadStatus::InProgress;
            
            {
                std::lock_guard<std::mutex> lock(mutex_);
                config_.status = FtpStatus::Uploading;
            }
            
            bool success = PerformUpload(item);
            
            item.status = success ? UploadStatus::Completed : UploadStatus::Failed;
            item.completed_at = GetCurrentTimestamp();
            
            // Update stats
            {
                std::lock_guard<std::mutex> lock(mutex_);
                stats_.total_uploads++;
                if (success) {
                    stats_.successful_uploads++;
                    stats_.bytes_uploaded += item.file_size;
                    stats_.last_upload_time = item.completed_at;
                    
                    // Delete local file if configured
                    if (config_.delete_after_upload) {
                        try {
                            std::filesystem::remove(item.local_path);
                            spdlog::info("[FtpManager] Deleted local file: {}", item.local_path);
                        } catch (const std::exception& e) {
                            spdlog::warn("[FtpManager] Failed to delete local file: {}", e.what());
                        }
                    }
                } else {
                    stats_.failed_uploads++;
                    
                    // Retry if needed
                    if (item.retry_count < config_.max_retries) {
                        item.retry_count++;
                        item.status = UploadStatus::Pending;
                        
                        std::lock_guard<std::mutex> queue_lock(queue_mutex_);
                        upload_queue_.push(item);
                        spdlog::info("[FtpManager] Requeued for retry ({}/{}): {}", 
                                    item.retry_count, config_.max_retries, item.local_path);
                    }
                }
                
                config_.status = FtpStatus::Connected;
            }
            
            // Add to history
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                upload_history_.push_back(item);
                
                // Limit history size
                while (upload_history_.size() > 100) {
                    upload_history_.erase(upload_history_.begin());
                }
            }
            
            // Notify callback
            if (complete_callback_) {
                complete_callback_(item, success);
            }
        }
    }
    
    spdlog::info("[FtpManager] Upload worker stopped");
}

bool FtpManager::PerformUpload(FtpUploadItem& item) {
    if (!std::filesystem::exists(item.local_path)) {
        item.error_message = "File not found";
        spdlog::error("[FtpManager] File not found: {}", item.local_path);
        return false;
    }
    
    FILE* file = fopen(item.local_path.c_str(), "rb");
    if (!file) {
        item.error_message = "Failed to open file";
        spdlog::error("[FtpManager] Failed to open file: {}", item.local_path);
        return false;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        fclose(file);
        item.error_message = "Failed to initialize CURL";
        return false;
    }
    
    std::string url = BuildFtpUrl(item.remote_path);
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, file);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(item.file_size));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(config_.timeout_seconds));
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
    
    // Progress callback
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &item);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    
    if (!config_.username.empty()) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, config_.username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, config_.password.c_str());
    }
    
    if (config_.use_ssl) {
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    
    if (!config_.passive_mode) {
        curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        item.error_message = curl_easy_strerror(res);
        spdlog::error("[FtpManager] Upload failed: {}", item.error_message);
    } else {
        spdlog::info("[FtpManager] Upload successful: {} -> {}", 
                     item.local_path, item.remote_path);
    }
    
    curl_easy_cleanup(curl);
    fclose(file);
    
    return res == CURLE_OK;
}

std::string FtpManager::BuildFtpUrl(const std::string& remote_path) const {
    std::stringstream ss;
    ss << (config_.use_ssl ? "ftps://" : "ftp://");
    ss << config_.server << ":" << config_.port;
    ss << config_.remote_path;
    
    if (!remote_path.empty()) {
        if (remote_path[0] != '/') {
            ss << "/";
        }
        ss << remote_path;
    }
    
    return ss.str();
}

std::string FtpManager::GenerateUploadId() {
    return GenerateUUID();
}

bool FtpManager::IsScheduledUploadTime() const {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    
    switch (config_.upload_schedule) {
        case FtpUploadSchedule::Immediate:
            return true;
            
        case FtpUploadSchedule::Hourly:
            return tm->tm_min == 0;  // Top of hour
            
        case FtpUploadSchedule::Daily:
            return tm->tm_hour == config_.upload_hour && tm->tm_min == 0;
            
        case FtpUploadSchedule::OnEvent:
            return false;  // Only on explicit trigger
            
        default:
            return false;
    }
}

} // namespace storage
} // namespace ipcam
