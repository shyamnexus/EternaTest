/**
 * @file webhook_action.cpp
 * @brief Webhook HTTP POST action handler implementation
 */

#include "ipcam/action_handler.h"
#include <spdlog/spdlog.h>
#include <curl/curl.h>
#include <chrono>
#include <thread>

namespace ipcam {
namespace events {

// Static CURL initialization
static struct CurlInit {
    CurlInit() { curl_global_init(CURL_GLOBAL_ALL); }
    ~CurlInit() { curl_global_cleanup(); }
} curl_init;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

static bool SendHttpRequest(const std::string& url, const std::string& method,
                            const std::map<std::string, std::string>& headers,
                            const std::string& body, int timeout_seconds,
                            std::string& response, int& http_code) {
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Set method
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    } else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    } else if (method == "GET") {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }
    
    // Set headers
    struct curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }
    if (header_list) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    }
    
    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_seconds);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    
    // Setup response capture
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    // SSL options (for HTTPS)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  // TODO: Make configurable
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    bool success = (res == CURLE_OK);
    
    if (success) {
        long code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        http_code = static_cast<int>(code);
    }
    
    // Cleanup
    if (header_list) {
        curl_slist_free_all(header_list);
    }
    curl_easy_cleanup(curl);
    
    return success;
}

ActionResult WebhookActionHandler::Execute(const Event& event, const Action& action) {
    ActionResult result;
    result.execution_time_ms = 0;
    
    const auto* cfg = action.GetConfig<WebhookActionConfig>();
    if (!cfg) {
        result.success = false;
        result.error_message = "Invalid configuration for webhook action";
        return result;
    }
    
    if (cfg->url.empty()) {
        result.success = false;
        result.error_message = "Webhook URL not configured";
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    spdlog::info("[WebhookActionHandler] Sending webhook to: {}", cfg->url);
    
    // Prepare body
    std::string body;
    if (cfg->include_snapshot_base64) {
        // Include full event JSON with snapshot
        body = event.ToJson();
    } else if (!cfg->body_template.empty()) {
        body = SubstitutePlaceholders(cfg->body_template, event);
    } else {
        // Default: event JSON
        body = event.ToJson();
    }
    
    // Prepare headers
    auto headers = cfg->headers;
    if (headers.find("Content-Type") == headers.end()) {
        headers["Content-Type"] = "application/json";
    }
    
    // Send with retries
    std::string response;
    int http_code = 0;
    bool success = false;
    
    for (int attempt = 0; attempt <= cfg->retry_count; ++attempt) {
        if (attempt > 0) {
            spdlog::debug("[WebhookActionHandler] Retry attempt {}/{}", attempt, cfg->retry_count);
            std::this_thread::sleep_for(std::chrono::seconds(cfg->retry_delay_seconds));
        }
        
        success = SendHttpRequest(cfg->url, cfg->method, headers, body,
                                  cfg->timeout_seconds, response, http_code);
        
        if (success && http_code >= 200 && http_code < 300) {
            break;
        }
        
        success = false;
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    if (success) {
        result.success = true;
        result.output = "Webhook sent successfully (HTTP " + std::to_string(http_code) + ")";
        result.metadata["http_code"] = std::to_string(http_code);
    } else {
        result.success = false;
        if (http_code > 0) {
            result.error_message = "Webhook failed with HTTP " + std::to_string(http_code);
        } else {
            result.error_message = "Failed to connect to webhook URL";
        }
    }
    
    return result;
}

} // namespace events
} // namespace ipcam
