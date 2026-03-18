/**
 * @file test_event_manager.cpp
 * @brief Unit tests for the Event Manager
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ipcam/event_manager.h>
#include "test_helpers.h"
#include "mock_handlers.h"
#include <thread>
#include <chrono>

using namespace ipcam::events;
using namespace ipcam::events::testing;

// ============================================================================
// Event Manager Singleton Tests
// ============================================================================

class EventManagerSingletonTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure manager is stopped before each test
        EventManager::Instance().Stop();
    }
    
    void TearDown() override {
        EventManager::Instance().Stop();
    }
};

TEST_F(EventManagerSingletonTest, SingletonInstance) {
    auto& manager1 = EventManager::Instance();
    auto& manager2 = EventManager::Instance();
    
    EXPECT_EQ(&manager1, &manager2);
}

TEST_F(EventManagerSingletonTest, StartStop) {
    auto& manager = EventManager::Instance();
    
    EXPECT_TRUE(manager.Start());
    EXPECT_TRUE(manager.IsRunning());
    
    manager.Stop();
    EXPECT_FALSE(manager.IsRunning());
}

TEST_F(EventManagerSingletonTest, DoubleStartIsIdempotent) {
    auto& manager = EventManager::Instance();
    
    EXPECT_TRUE(manager.Start());
    EXPECT_TRUE(manager.Start());  // Should succeed without error
    EXPECT_TRUE(manager.IsRunning());
}

TEST_F(EventManagerSingletonTest, DoubleStopIsIdempotent) {
    auto& manager = EventManager::Instance();
    
    manager.Start();
    manager.Stop();
    manager.Stop();  // Should not crash
    
    EXPECT_FALSE(manager.IsRunning());
}

// ============================================================================
// Event Publishing Tests
// ============================================================================

class EventPublishingTest : public ::testing::Test {
protected:
    void SetUp() override {
        EventManager::Instance().Stop();
        EventManager::Instance().Start();
        listener_.Clear();
    }
    
    void TearDown() override {
        EventManager::Instance().Stop();
    }
    
    MockEventListener listener_;
};

TEST_F(EventPublishingTest, PublishEvent) {
    auto& manager = EventManager::Instance();
    auto event = CreateMotionEvent();
    
    std::string event_id = manager.PublishEvent(event);
    
    // UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    EXPECT_FALSE(event_id.empty());
    EXPECT_EQ(std::count(event_id.begin(), event_id.end(), '-'), 4);
}

TEST_F(EventPublishingTest, PublishMultipleEvents) {
    auto& manager = EventManager::Instance();
    
    std::vector<std::string> ids;
    for (int i = 0; i < 10; i++) {
        auto event = CreateMotionEvent({static_cast<uint32_t>(i)});
        ids.push_back(manager.PublishEvent(event));
    }
    
    // All IDs should be unique
    std::set<std::string> unique_ids(ids.begin(), ids.end());
    EXPECT_EQ(unique_ids.size(), 10u);
}

TEST_F(EventPublishingTest, EventListenerReceivesEvents) {
    auto& manager = EventManager::Instance();
    
    // Register listener
    auto listener_id = manager.AddListener(
        [this](const Event& event) {
            listener_.OnEvent(event);
        }
    );
    
    // Publish event
    auto event = CreateMotionEvent();
    manager.PublishEvent(event);
    
    // Wait for processing
    EXPECT_TRUE(listener_.WaitForEvent(std::chrono::milliseconds(500)));
    EXPECT_EQ(listener_.GetEventCount(), 1);
    
    manager.RemoveListener(listener_id);
}

TEST_F(EventPublishingTest, FilteredListener) {
    auto& manager = EventManager::Instance();
    
    // Register filtered listener (motion events only)
    auto listener_id = manager.AddListener(
        [this](const Event& event) {
            listener_.OnEvent(event);
        },
        [](const Event& event) {
            return event.category == EventCategory::kMotion;
        }
    );
    
    // Publish motion event (should be received)
    manager.PublishEvent(CreateMotionEvent());
    
    // Publish person event (should be filtered)
    manager.PublishEvent(CreatePersonDetectedEvent());
    
    // Wait and check
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(listener_.GetEventCount(), 1);
    
    manager.RemoveListener(listener_id);
}

// ============================================================================
// Event History Tests
// ============================================================================

class EventHistoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        EventManager::Instance().Stop();
        EventManager::Instance().ClearHistory();
        EventManager::Instance().Start();
    }
    
    void TearDown() override {
        EventManager::Instance().Stop();
    }
};

TEST_F(EventHistoryTest, EventsAddedToHistory) {
    auto& manager = EventManager::Instance();
    
    manager.PublishEvent(CreateMotionEvent());
    manager.PublishEvent(CreatePersonDetectedEvent());
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EventQuery query;
    query.limit = 100;
    auto history = manager.QueryEvents(query);
    
    EXPECT_GE(history.size(), 2);
}

TEST_F(EventHistoryTest, QueryByCategory) {
    auto& manager = EventManager::Instance();
    
    manager.PublishEvent(CreateMotionEvent());
    manager.PublishEvent(CreateMotionEvent());
    manager.PublishEvent(CreatePersonDetectedEvent());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EventQuery query;
    query.category = EventCategory::kMotion;
    auto motion_events = manager.QueryEvents(query);
    
    EXPECT_GE(motion_events.size(), 2);
    for (const auto& event : motion_events) {
        EXPECT_EQ(event.category, EventCategory::kMotion);
    }
}

TEST_F(EventHistoryTest, QueryByType) {
    auto& manager = EventManager::Instance();
    
    manager.PublishEvent(CreateMotionEvent());
    manager.PublishEvent(CreatePersonDetectedEvent());
    manager.PublishEvent(CreateVehicleDetectedEvent());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EventQuery query;
    query.type = EventType::kPersonDetected;
    auto person_events = manager.QueryEvents(query);
    
    EXPECT_GE(person_events.size(), 1);
    for (const auto& event : person_events) {
        EXPECT_EQ(event.type, EventType::kPersonDetected);
    }
}

TEST_F(EventHistoryTest, QueryWithLimit) {
    auto& manager = EventManager::Instance();
    
    for (int i = 0; i < 20; i++) {
        manager.PublishEvent(CreateMotionEvent());
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    EventQuery query;
    query.limit = 5;
    auto events = manager.QueryEvents(query);
    
    EXPECT_LE(events.size(), 5);
}

TEST_F(EventHistoryTest, ClearHistory) {
    auto& manager = EventManager::Instance();
    
    manager.PublishEvent(CreateMotionEvent());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    manager.ClearHistory();
    
    EventQuery query;
    auto events = manager.QueryEvents(query);
    
    EXPECT_EQ(events.size(), 0);
}

// ============================================================================
// Event Statistics Tests
// ============================================================================

class EventStatsTest : public ::testing::Test {
protected:
    void SetUp() override {
        EventManager::Instance().Stop();
        EventManager::Instance().ResetStats();
        EventManager::Instance().Start();
    }
    
    void TearDown() override {
        EventManager::Instance().Stop();
    }
};

TEST_F(EventStatsTest, PublishedEventsCounted) {
    auto& manager = EventManager::Instance();
    
    manager.PublishEvent(CreateMotionEvent());
    manager.PublishEvent(CreateMotionEvent());
    manager.PublishEvent(CreatePersonDetectedEvent());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto stats = manager.GetStats();
    
    EXPECT_GE(stats.events_published, 3);
}

TEST_F(EventStatsTest, ProcessedEventsCounted) {
    auto& manager = EventManager::Instance();
    
    manager.PublishEvent(CreateMotionEvent());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto stats = manager.GetStats();
    
    EXPECT_GE(stats.events_processed, 1);
}

TEST_F(EventStatsTest, EventsByTypeCounted) {
    auto& manager = EventManager::Instance();
    
    manager.PublishEvent(CreateMotionEvent());
    manager.PublishEvent(CreateMotionEvent());
    manager.PublishEvent(CreatePersonDetectedEvent());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto stats = manager.GetStats();
    
    EXPECT_GE(stats.events_by_type[EventType::kMotionStart], 2);
    EXPECT_GE(stats.events_by_type[EventType::kPersonDetected], 1);
}

TEST_F(EventStatsTest, ResetStats) {
    auto& manager = EventManager::Instance();
    
    manager.PublishEvent(CreateMotionEvent());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    manager.ResetStats();
    auto stats = manager.GetStats();
    
    EXPECT_EQ(stats.events_published, 0);
    EXPECT_EQ(stats.events_processed, 0);
}

// ============================================================================
// Configuration Tests
// ============================================================================

class EventManagerConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        EventManager::Instance().Stop();
    }
    
    void TearDown() override {
        EventManager::Instance().Stop();
    }
};

TEST_F(EventManagerConfigTest, GetConfig) {
    auto& manager = EventManager::Instance();
    
    auto config = manager.GetConfig();
    
    EXPECT_TRUE(config.enabled);
    EXPECT_GT(config.max_history_size, 0);
    EXPECT_GT(config.queue_max_size, 0);
}

TEST_F(EventManagerConfigTest, SetConfig) {
    auto& manager = EventManager::Instance();
    
    EventManagerConfig new_config;
    new_config.enabled = true;
    new_config.max_history_size = 500;
    new_config.default_cooldown_seconds = 15;
    new_config.queue_max_size = 50;
    
    manager.SetConfig(new_config);
    auto config = manager.GetConfig();
    
    EXPECT_EQ(config.max_history_size, 500);
    EXPECT_EQ(config.default_cooldown_seconds, 15);
    EXPECT_EQ(config.queue_max_size, 50);
}

// ============================================================================
// Rule Management Tests
// ============================================================================

class RuleManagementTest : public ::testing::Test {
protected:
    void SetUp() override {
        EventManager::Instance().Stop();
        // Delete any existing rules
        auto& manager = EventManager::Instance();
        auto rules = manager.GetRules();
        for (const auto& rule : rules) {
            manager.DeleteRule(rule.id);
        }
        EventManager::Instance().Start();
    }
    
    void TearDown() override {
        EventManager::Instance().Stop();
    }
};

TEST_F(RuleManagementTest, AddRule) {
    auto& manager = EventManager::Instance();
    
    auto rule = CreateTestRule("test-rule", EventType::kMotionStart);
    
    EXPECT_TRUE(manager.AddRule(rule));
    
    auto rules = manager.GetRules();
    EXPECT_EQ(rules.size(), 1u);
    EXPECT_EQ(rules[0].id, "test-rule");
}

TEST_F(RuleManagementTest, DeleteRule) {
    auto& manager = EventManager::Instance();
    
    auto rule = CreateTestRule("to-remove", EventType::kMotionStart);
    manager.AddRule(rule);
    
    EXPECT_TRUE(manager.DeleteRule("to-remove"));
    
    auto rules = manager.GetRules();
    EXPECT_EQ(rules.size(), 0u);
}

TEST_F(RuleManagementTest, UpdateRule) {
    auto& manager = EventManager::Instance();
    
    auto rule = CreateTestRule("to-update", EventType::kMotionStart);
    manager.AddRule(rule);
    
    rule.name = "Updated Name";
    rule.priority = 99;
    
    EXPECT_TRUE(manager.UpdateRule(rule));
    
    auto rules = manager.GetRules();
    EXPECT_EQ(rules[0].name, "Updated Name");
    EXPECT_EQ(rules[0].priority, 99);
}

TEST_F(RuleManagementTest, GetRuleById) {
    auto& manager = EventManager::Instance();
    
    manager.AddRule(CreateTestRule("rule-1", EventType::kMotionStart));
    manager.AddRule(CreateTestRule("rule-2", EventType::kPersonDetected));
    
    auto rule = manager.GetRule("rule-2");
    
    EXPECT_TRUE(rule.has_value());
    EXPECT_EQ(rule->id, "rule-2");
}

TEST_F(RuleManagementTest, EnableDisableRule) {
    auto& manager = EventManager::Instance();
    
    auto rule = CreateTestRule("toggle-rule", EventType::kMotionStart);
    rule.enabled = true;
    manager.AddRule(rule);
    
    manager.SetRuleEnabled("toggle-rule", false);
    
    auto updated = manager.GetRule("toggle-rule");
    EXPECT_FALSE(updated->enabled);
    
    manager.SetRuleEnabled("toggle-rule", true);
    updated = manager.GetRule("toggle-rule");
    EXPECT_TRUE(updated->enabled);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

class ThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        EventManager::Instance().Stop();
        EventManager::Instance().Start();
    }
    
    void TearDown() override {
        EventManager::Instance().Stop();
    }
};

TEST_F(ThreadSafetyTest, ConcurrentPublish) {
    auto& manager = EventManager::Instance();
    
    const int num_threads = 4;
    const int events_per_thread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> published_count{0};
    
    for (int t = 0; t < num_threads; t++) {
        threads.emplace_back([&manager, &published_count, events_per_thread]() {
            for (int i = 0; i < events_per_thread; i++) {
                auto event = CreateMotionEvent({static_cast<uint32_t>(i % 4)});
                auto id = manager.PublishEvent(event);
                if (!id.empty()) {
                    published_count++;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(published_count, num_threads * events_per_thread);
}

TEST_F(ThreadSafetyTest, ConcurrentListenerOperations) {
    auto& manager = EventManager::Instance();
    
    std::vector<std::thread> threads;
    std::vector<uint32_t> listener_ids;
    std::mutex ids_mutex;
    
    // Add listeners from multiple threads
    for (int t = 0; t < 4; t++) {
        threads.emplace_back([&]() {
            for (int i = 0; i < 10; i++) {
                auto id = manager.AddListener([](const Event&) {});
                std::lock_guard<std::mutex> lock(ids_mutex);
                listener_ids.push_back(id);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(listener_ids.size(), 40u);
    
    // Remove all listeners
    for (auto id : listener_ids) {
        manager.RemoveListener(id);
    }
}
