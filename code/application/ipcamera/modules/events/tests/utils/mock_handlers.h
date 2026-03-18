/**
 * @file mock_handlers.h
 * @brief Mock action handlers for testing
 */

#pragma once

#include <ipcam/action_handler.h>
#include <ipcam/event_rule.h>
#include <gmock/gmock.h>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>

namespace ipcam {
namespace events {
namespace testing {

/**
 * @brief Records all actions executed for verification
 */
struct ActionRecord {
    ActionType type;
    Event event;
    Action action;
    std::chrono::system_clock::time_point timestamp;
    bool success;
    std::string error;
};

/**
 * @brief Mock action handler that records all executions
 */
class MockActionHandler : public ActionHandler {
public:
    explicit MockActionHandler(ActionType type) 
        : type_(type), should_succeed_(true) {}

    ActionType GetType() const override { return type_; }
    std::string GetName() const override { return "MockHandler_" + ActionTypeToString(type_); }
    
    ActionResult Execute(const Event& event, const Action& action) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        ActionRecord record;
        record.type = type_;
        record.event = event;
        record.action = action;
        record.timestamp = std::chrono::system_clock::now();
        record.success = should_succeed_;
        record.error = should_succeed_ ? "" : failure_message_;
        
        records_.push_back(record);
        execution_count_++;
        
        ActionResult result;
        result.success = should_succeed_;
        result.error_message = failure_message_;
        result.execution_time_ms = 1;  // Mock instant execution
        return result;
    }

    void SetShouldSucceed(bool succeed, const std::string& error_msg = "") {
        should_succeed_ = succeed;
        failure_message_ = error_msg;
    }

    int GetExecutionCount() const { return execution_count_.load(); }
    
    std::vector<ActionRecord> GetRecords() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return records_;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        records_.clear();
        execution_count_ = 0;
        should_succeed_ = true;
        failure_message_.clear();
    }

private:
    ActionType type_;
    mutable std::mutex mutex_;
    std::vector<ActionRecord> records_;
    std::atomic<int> execution_count_{0};
    bool should_succeed_;
    std::string failure_message_;
};

/**
 * @brief Factory that creates mock handlers
 */
class MockActionHandlerFactory {
public:
    static MockActionHandlerFactory& Instance() {
        static MockActionHandlerFactory instance;
        return instance;
    }

    std::shared_ptr<MockActionHandler> GetHandler(ActionType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(type);
        if (it == handlers_.end()) {
            auto handler = std::make_shared<MockActionHandler>(type);
            handlers_[type] = handler;
            return handler;
        }
        return it->second;
    }

    void ResetAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [type, handler] : handlers_) {
            handler->Reset();
        }
    }

    std::vector<ActionRecord> GetAllRecords() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<ActionRecord> all;
        for (const auto& [type, handler] : handlers_) {
            auto records = handler->GetRecords();
            all.insert(all.end(), records.begin(), records.end());
        }
        return all;
    }

private:
    MockActionHandlerFactory() = default;
    mutable std::mutex mutex_;
    std::map<ActionType, std::shared_ptr<MockActionHandler>> handlers_;
};

/**
 * @brief Mock event listener for testing
 */
class MockEventListener {
public:
    void OnEvent(const Event& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.push_back(event);
    }

    std::vector<Event> GetEvents() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_;
    }

    size_t GetEventCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_.size();
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.clear();
    }

    bool WaitForEvent(std::chrono::milliseconds timeout) {
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < timeout) {
            if (GetEventCount() > 0) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return false;
    }

    bool WaitForEventCount(size_t count, std::chrono::milliseconds timeout) {
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < timeout) {
            if (GetEventCount() >= count) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return false;
    }

private:
    mutable std::mutex mutex_;
    std::vector<Event> events_;
};

} // namespace testing
} // namespace events
} // namespace ipcam
