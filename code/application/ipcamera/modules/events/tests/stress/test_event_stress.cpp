/**
 * @file test_event_stress.cpp
 * @brief Stress tests for the event system
 */

#include <gtest/gtest.h>
#include <ipcam/event_manager.h>
#include "test_helpers.h"
#include "mock_handlers.h"
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <random>

using namespace ipcam::events;
using namespace ipcam::events::testing;

// ============================================================================
// Stress Test Fixture
// ============================================================================

class EventStressTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto& manager = EventManager::Instance();
        manager.Stop();
        // Clear existing rules
        auto rules = manager.GetRules();
        for (const auto& rule : rules) {
            manager.DeleteRule(rule.id);
        }
        manager.ClearHistory();
        manager.ResetStats();
        MockActionHandlerFactory::Instance().ResetAll();
        manager.Start();
    }
    
    void TearDown() override {
        EventManager::Instance().Stop();
        MockActionHandlerFactory::Instance().ResetAll();
    }
};

// ============================================================================
// High Volume Event Publishing
// ============================================================================

TEST_F(EventStressTest, HighVolumeEventPublishing) {
    auto& manager = EventManager::Instance();
    
    const int num_events = 1000;
    std::atomic<int> published_count{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_events; i++) {
        auto event = CreateMotionEvent({static_cast<uint32_t>(i % 4)});
        auto id = manager.PublishEvent(event);
        if (!id.empty()) {
            published_count++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(published_count, num_events);
    
    // Should complete within reasonable time (< 5 seconds for 1000 events)
    EXPECT_LT(duration.count(), 5000);
    
    std::cout << "Published " << num_events << " events in " 
              << duration.count() << "ms" << std::endl;
}

TEST_F(EventStressTest, ConcurrentEventPublishing) {
    auto& manager = EventManager::Instance();
    
    const int num_threads = 8;
    const int events_per_thread = 500;
    std::atomic<int> total_published{0};
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < num_threads; t++) {
        threads.emplace_back([&manager, &total_published, events_per_thread, t]() {
            for (int i = 0; i < events_per_thread; i++) {
                Event event;
                switch (i % 4) {
                    case 0: event = CreateMotionEvent({static_cast<uint32_t>(t)}); break;
                    case 1: event = CreatePersonDetectedEvent(0.8f); break;
                    case 2: event = CreateVehicleDetectedEvent(0.9f); break;
                    case 3: event = CreateSystemEvent(); break;
                }
                
                auto id = manager.PublishEvent(event);
                if (!id.empty()) {
                    total_published++;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    const int expected_total = num_threads * events_per_thread;
    EXPECT_EQ(total_published, expected_total);
    
    // Wait for processing to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto stats = manager.GetStats();
    EXPECT_GE(stats.events_published, static_cast<uint64_t>(expected_total));
    
    std::cout << "Published " << total_published << " events from " 
              << num_threads << " threads in " << duration.count() << "ms" << std::endl;
}

// ============================================================================
// Many Rules Stress Test
// ============================================================================

TEST_F(EventStressTest, ManyRulesProcessing) {
    auto& manager = EventManager::Instance();
    
    const int num_rules = 100;
    
    // Create many rules
    for (int i = 0; i < num_rules; i++) {
        EventRule rule = EventRule::Create("Rule " + std::to_string(i));
        rule.id = "rule-" + std::to_string(i);
        rule.enabled = true;
        rule.trigger_events = {EventType::kMotionStart};
        rule.cooldown_seconds = 0;  // No cooldown for stress test
        
        Action action;
        action.type = ActionType::kLogEvent;
        action.enabled = true;
        LogActionConfig log_config;
        log_config.save_to_database = true;
        action.config = log_config;
        rule.actions.push_back(action);
        
        manager.AddRule(rule);
    }
    
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kLogEvent);
    manager.RegisterActionHandler(ActionType::kLogEvent, handler);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Publish one event - all rules should match
    manager.PublishEvent(CreateMotionEvent());
    
    // Wait for all rules to process
    EXPECT_TRUE(WaitFor([&]() { 
        return handler->GetExecutionCount() >= num_rules; 
    }, std::chrono::milliseconds(5000)));
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(handler->GetExecutionCount(), num_rules);
    
    std::cout << "Processed " << num_rules << " rules in " 
              << duration.count() << "ms" << std::endl;
}

// ============================================================================
// Listener Stress Test
// ============================================================================

TEST_F(EventStressTest, ManyListeners) {
    auto& manager = EventManager::Instance();
    
    const int num_listeners = 50;
    std::vector<uint64_t> listener_ids;
    std::atomic<int> total_received{0};
    
    // Add many listeners
    for (int i = 0; i < num_listeners; i++) {
        auto id = manager.AddListener([&total_received](const Event&) {
            total_received++;
        });
        listener_ids.push_back(id);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Publish events
    const int num_events = 100;
    for (int i = 0; i < num_events; i++) {
        manager.PublishEvent(CreateMotionEvent());
    }
    
    // Wait for delivery
    EXPECT_TRUE(WaitFor([&]() { 
        return total_received >= num_listeners * num_events; 
    }, std::chrono::milliseconds(5000)));
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(total_received, num_listeners * num_events);
    
    // Cleanup
    for (auto id : listener_ids) {
        manager.RemoveListener(id);
    }
    
    std::cout << "Delivered " << total_received << " events to " 
              << num_listeners << " listeners in " << duration.count() << "ms" << std::endl;
}

// ============================================================================
// Memory Stress Test
// ============================================================================

TEST_F(EventStressTest, HistorySizeLimit) {
    auto& manager = EventManager::Instance();
    
    // Set small history limit
    EventManagerConfig config = manager.GetConfig();
    config.max_history_size = 100;
    manager.SetConfig(config);
    
    // Publish more events than history can hold
    const int num_events = 500;
    for (int i = 0; i < num_events; i++) {
        manager.PublishEvent(CreateMotionEvent());
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Query all history
    EventQuery query;
    query.limit = 1000;
    auto history = manager.QueryEvents(query);
    
    // Should be limited to max_history_size
    EXPECT_LE(history.size(), config.max_history_size);
    
    std::cout << "History size limited to " << history.size() 
              << " (limit: " << config.max_history_size << ")" << std::endl;
}

// ============================================================================
// Long Running Test
// ============================================================================

TEST_F(EventStressTest, DISABLED_LongRunningStability) {
    // This test is disabled by default as it runs for a long time
    // Enable it manually for stability testing
    
    auto& manager = EventManager::Instance();
    
    EventRule rule;
    rule.id = "long-running";
    rule.enabled = true;
    rule.trigger.event_types = {EventType::kMotionStart};
    rule.cooldown_seconds = 0;
    rule.actions.push_back(CreateSnapshotAction());
    manager.AddRule(rule);
    
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot, handler);
    
    const auto duration = std::chrono::seconds(60);  // Run for 1 minute
    const auto interval = std::chrono::milliseconds(50);  // Event every 50ms
    
    auto start = std::chrono::steady_clock::now();
    int events_published = 0;
    
    while (std::chrono::steady_clock::now() - start < duration) {
        manager.PublishEvent(CreateMotionEvent());
        events_published++;
        std::this_thread::sleep_for(interval);
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    auto stats = manager.GetStats();
    
    std::cout << "Long running test completed:" << std::endl;
    std::cout << "  Events published: " << events_published << std::endl;
    std::cout << "  Events processed: " << stats.events_processed << std::endl;
    std::cout << "  Actions executed: " << stats.actions_executed << std::endl;
    
    // All events should be processed
    EXPECT_EQ(stats.events_published, events_published);
}

// ============================================================================
// Burst Traffic Test
// ============================================================================

TEST_F(EventStressTest, BurstTraffic) {
    auto& manager = EventManager::Instance();
    
    EventRule rule;
    rule.id = "burst-rule";
    rule.enabled = true;
    rule.trigger.event_types = {EventType::kMotionStart};
    rule.cooldown_seconds = 0;
    
    Action action;
    action.type = ActionType::kLogEvent;
    action.enabled = true;
    rule.actions.push_back(action);
    manager.AddRule(rule);
    
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kLogEvent);
    manager.RegisterActionHandler(ActionType::kLogEvent, handler);
    
    // Simulate burst: 100 events as fast as possible, then pause
    const int burst_size = 100;
    const int num_bursts = 5;
    
    for (int burst = 0; burst < num_bursts; burst++) {
        // Burst
        for (int i = 0; i < burst_size; i++) {
            manager.PublishEvent(CreateMotionEvent());
        }
        
        // Pause between bursts
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Wait for processing
    EXPECT_TRUE(WaitFor([&]() { 
        return handler->GetExecutionCount() >= burst_size * num_bursts; 
    }, std::chrono::milliseconds(10000)));
    
    EXPECT_EQ(handler->GetExecutionCount(), burst_size * num_bursts);
    
    std::cout << "Handled " << num_bursts << " bursts of " << burst_size 
              << " events each" << std::endl;
}

// ============================================================================
// Mixed Event Types Stress Test
// ============================================================================

TEST_F(EventStressTest, MixedEventTypesHighVolume) {
    auto& manager = EventManager::Instance();
    
    // Create rules for different event types
    std::vector<std::pair<EventType, ActionType>> type_mappings = {
        {EventType::kMotionStart, ActionType::kStartRecording},
        {EventType::kPersonDetected, ActionType::kCaptureSnapshot},
        {EventType::kVehicleDetected, ActionType::kLogEvent},
        {EventType::kSystemStartup, ActionType::kSendWebhook}
    };
    
    for (const auto& [event_type, action_type] : type_mappings) {
        EventRule rule;
        rule.id = "rule-" + EventTypeToString(event_type);
        rule.enabled = true;
        rule.trigger.event_types = {event_type};
        rule.cooldown_seconds = 0;
        
        Action action;
        action.type = action_type;
        action.enabled = true;
        rule.actions.push_back(action);
        manager.AddRule(rule);
        
        auto handler = MockActionHandlerFactory::Instance().GetHandler(action_type);
        manager.RegisterActionHandler(action_type, handler);
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 3);
    
    const int num_events = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_events; i++) {
        Event event;
        switch (dist(gen)) {
            case 0: event = CreateMotionEvent(); break;
            case 1: event = CreatePersonDetectedEvent(); break;
            case 2: event = CreateVehicleDetectedEvent(); break;
            case 3: event = CreateSystemEvent(); break;
        }
        manager.PublishEvent(event);
    }
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    auto stats = manager.GetStats();
    
    std::cout << "Mixed event types stress test:" << std::endl;
    std::cout << "  Events published: " << stats.events_published << std::endl;
    std::cout << "  Duration: " << duration.count() << "ms" << std::endl;
    
    EXPECT_GE(stats.events_published, num_events);
}
