/**
 * @file test_event_flow.cpp
 * @brief Integration tests for the complete event flow
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ipcam/event_manager.h>
#include <ipcam/event_types.h>
#include <ipcam/event_rule.h>
#include "test_helpers.h"
#include "mock_handlers.h"
#include <thread>
#include <chrono>
#include <atomic>

using namespace ipcam::events;
using namespace ipcam::events::testing;

// ============================================================================
// Event Flow Integration Test Fixture
// ============================================================================

class EventFlowIntegrationTest : public ::testing::Test {
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
// Basic Event Flow Tests
// ============================================================================

TEST_F(EventFlowIntegrationTest, MotionEventTriggerRecording) {
    auto& manager = EventManager::Instance();
    
    // Create a rule: motion -> start recording
    EventRule rule = EventRule::Create("Motion Recording");
    rule.id = "motion-to-recording";
    rule.enabled = true;
    rule.trigger_events = {EventType::kMotionStart};
    
    Action recording_action = CreateRecordingAction(30);
    rule.actions.push_back(recording_action);
    
    manager.AddRule(rule);
    
    // Register mock handler
    auto mock_handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kStartRecording);
    manager.RegisterActionHandler(ActionType::kStartRecording, mock_handler);
    
    // Publish motion event
    auto event = CreateMotionEvent({0});
    manager.PublishEvent(event);
    
    // Wait for processing
    EXPECT_TRUE(WaitFor([&]() { return mock_handler->GetExecutionCount() > 0; },
                        std::chrono::milliseconds(1000)));
    
    EXPECT_EQ(mock_handler->GetExecutionCount(), 1);
    
    auto records = mock_handler->GetRecords();
    EXPECT_EQ(records[0].type, ActionType::kStartRecording);
}

TEST_F(EventFlowIntegrationTest, PersonEventTriggersMultipleActions) {
    auto& manager = EventManager::Instance();
    
    // Create rule: person detected -> snapshot + log
    EventRule rule = EventRule::Create("Person Detection");
    rule.id = "person-multi-action";
    rule.enabled = true;
    rule.trigger_events = {EventType::kPersonDetected};
    rule.min_confidence = 0.5f;
    
    rule.actions.push_back(CreateSnapshotAction(90));
    
    Action log_action;
    log_action.type = ActionType::kLogEvent;
    log_action.enabled = true;
    log_action.priority = 0;
    LogActionConfig log_config;
    log_config.save_to_database = true;
    log_action.config = log_config;
    rule.actions.push_back(log_action);
    
    manager.AddRule(rule);
    
    // Register mock handlers
    auto snapshot_handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    auto log_handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kLogEvent);
    
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot, snapshot_handler);
    manager.RegisterActionHandler(ActionType::kLogEvent, log_handler);
    
    // Publish person event with high confidence
    auto event = CreatePersonDetectedEvent(0.85f);
    manager.PublishEvent(event);
    
    // Wait for processing
    EXPECT_TRUE(WaitFor([&]() { 
        return snapshot_handler->GetExecutionCount() > 0 && 
               log_handler->GetExecutionCount() > 0; 
    }, std::chrono::milliseconds(1000)));
    
    EXPECT_EQ(snapshot_handler->GetExecutionCount(), 1);
    EXPECT_EQ(log_handler->GetExecutionCount(), 1);
}

TEST_F(EventFlowIntegrationTest, LowConfidenceEventFiltered) {
    auto& manager = EventManager::Instance();
    
    // Create rule with confidence threshold
    EventRule rule = EventRule::Create("High Confidence Only");
    rule.id = "high-confidence-only";
    rule.enabled = true;
    rule.trigger_events = {EventType::kPersonDetected};
    rule.min_confidence = 0.8f;  // High threshold
    
    rule.actions.push_back(CreateSnapshotAction());
    manager.AddRule(rule);
    
    auto snapshot_handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot, snapshot_handler);
    
    // Publish low confidence event
    auto event = CreatePersonDetectedEvent(0.5f);  // Below threshold
    manager.PublishEvent(event);
    
    // Wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Action should NOT have been triggered
    EXPECT_EQ(snapshot_handler->GetExecutionCount(), 0);
}

TEST_F(EventFlowIntegrationTest, ZoneFilteringWorks) {
    auto& manager = EventManager::Instance();
    
    // Rule only for zones 1 and 2
    EventRule rule = EventRule::Create("Zone Filter");
    rule.id = "zone-filter";
    rule.enabled = true;
    rule.trigger_events = {EventType::kMotionStart};
    rule.trigger_zones = {1, 2};
    
    rule.actions.push_back(CreateRecordingAction());
    manager.AddRule(rule);
    
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kStartRecording);
    manager.RegisterActionHandler(ActionType::kStartRecording, handler);
    
    // Event in zone 1 - should trigger
    manager.PublishEvent(CreateMotionEvent({1}));
    
    // Event in zone 5 - should NOT trigger
    manager.PublishEvent(CreateMotionEvent({5}));
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Only one action should have been triggered
    EXPECT_EQ(handler->GetExecutionCount(), 1);
    
    auto records = handler->GetRecords();
    auto& motion_data = std::get<MotionEventData>(records[0].event.data);
    EXPECT_EQ(motion_data.zone_ids[0], 1u);
}

// ============================================================================
// Cooldown Tests
// ============================================================================

TEST_F(EventFlowIntegrationTest, CooldownPreventsRapidFiring) {
    auto& manager = EventManager::Instance();
    
    EventRule rule = EventRule::Create("Cooldown Test");
    rule.id = "cooldown-test";
    rule.enabled = true;
    rule.trigger_events = {EventType::kMotionStart};
    rule.cooldown_seconds = 2;  // 2 second cooldown
    
    rule.actions.push_back(CreateSnapshotAction());
    manager.AddRule(rule);
    
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot, handler);
    
    // Rapid fire events
    for (int i = 0; i < 5; i++) {
        manager.PublishEvent(CreateMotionEvent());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Due to cooldown, only 1 action should have triggered
    EXPECT_EQ(handler->GetExecutionCount(), 1);
}

TEST_F(EventFlowIntegrationTest, CooldownResetsAfterTime) {
    auto& manager = EventManager::Instance();
    
    EventRule rule = EventRule::Create("Cooldown Reset");
    rule.id = "cooldown-reset";
    rule.enabled = true;
    rule.trigger_events = {EventType::kMotionStart};
    rule.cooldown_seconds = 1;  // 1 second cooldown (short for testing)
    
    rule.actions.push_back(CreateSnapshotAction());
    manager.AddRule(rule);
    
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot, handler);
    
    // First event
    manager.PublishEvent(CreateMotionEvent());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(handler->GetExecutionCount(), 1);
    
    // Wait for cooldown to expire
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    
    // Second event after cooldown
    manager.PublishEvent(CreateMotionEvent());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Both events should have triggered
    EXPECT_EQ(handler->GetExecutionCount(), 2);
}

// ============================================================================
// Disabled Rule/Action Tests
// ============================================================================

TEST_F(EventFlowIntegrationTest, DisabledRuleIgnored) {
    auto& manager = EventManager::Instance();
    
    EventRule rule = EventRule::Create("Disabled Rule");
    rule.id = "disabled-rule";
    rule.enabled = false;  // Disabled!
    rule.trigger_events = {EventType::kMotionStart};
    
    rule.actions.push_back(CreateSnapshotAction());
    manager.AddRule(rule);
    
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot, handler);
    
    manager.PublishEvent(CreateMotionEvent());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(handler->GetExecutionCount(), 0);
}

TEST_F(EventFlowIntegrationTest, DisabledActionSkipped) {
    auto& manager = EventManager::Instance();
    
    EventRule rule = EventRule::Create("Disabled Action");
    rule.id = "disabled-action";
    rule.enabled = true;
    rule.trigger_events = {EventType::kMotionStart};
    
    Action enabled_action = CreateSnapshotAction();
    enabled_action.enabled = true;
    
    Action disabled_action = CreateRecordingAction();
    disabled_action.enabled = false;  // Disabled!
    
    rule.actions.push_back(enabled_action);
    rule.actions.push_back(disabled_action);
    manager.AddRule(rule);
    
    auto snapshot_handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    auto recording_handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kStartRecording);
    
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot, snapshot_handler);
    manager.RegisterActionHandler(ActionType::kStartRecording, recording_handler);
    
    manager.PublishEvent(CreateMotionEvent());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(snapshot_handler->GetExecutionCount(), 1);
    EXPECT_EQ(recording_handler->GetExecutionCount(), 0);
}

// ============================================================================
// Event Listener Integration Tests
// ============================================================================

TEST_F(EventFlowIntegrationTest, ListenersReceiveEventsBeforeRules) {
    auto& manager = EventManager::Instance();
    
    MockEventListener listener;
    std::atomic<bool> rule_processed{false};
    
    auto listener_id = manager.AddListener([&](const Event& event) {
        listener.OnEvent(event);
    });
    
    auto rule = EventRule::Create("listener-order-test");
    rule.enabled = true;
    rule.trigger_events = {EventType::kMotionStart};
    
    Action action;
    action.type = ActionType::kLogEvent;
    action.enabled = true;
    rule.actions.push_back(action);
    manager.AddRule(rule);
    
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kLogEvent);
    manager.RegisterActionHandler(ActionType::kLogEvent, handler);
    
    manager.PublishEvent(CreateMotionEvent({1}));
    
    EXPECT_TRUE(listener.WaitForEvent(std::chrono::milliseconds(500)));
    EXPECT_EQ(listener.GetEventCount(), 1);
    
    manager.RemoveListener(listener_id);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(EventFlowIntegrationTest, ActionFailureDoesNotStopOtherActions) {
    auto& manager = EventManager::Instance();
    
    auto rule = EventRule::Create("error-handling");
    rule.enabled = true;
    rule.trigger_events = {EventType::kMotionStart};
    
    // First action will fail
    Action failing_action;
    failing_action.type = ActionType::kSendEmail;
    failing_action.enabled = true;
    failing_action.priority = 10;
    
    // Second action should still execute
    Action success_action = CreateSnapshotAction();
    success_action.priority = 5;
    
    rule.actions.push_back(failing_action);
    rule.actions.push_back(success_action);
    manager.AddRule(rule);
    
    auto email_handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kSendEmail);
    email_handler->SetShouldSucceed(false, "SMTP error");
    
    auto snapshot_handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    
    manager.RegisterActionHandler(ActionType::kSendEmail, email_handler);
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot, snapshot_handler);
    
    manager.PublishEvent(CreateMotionEvent({1}));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Both should have been attempted
    EXPECT_EQ(email_handler->GetExecutionCount(), 1);
    EXPECT_EQ(snapshot_handler->GetExecutionCount(), 1);
    
    // Check stats show failure
    auto stats = manager.GetStats();
    EXPECT_GE(stats.actions_failed, 1);
}

// ============================================================================
// Statistics Integration Tests
// ============================================================================

TEST_F(EventFlowIntegrationTest, StatisticsAccuracy) {
    auto& manager = EventManager::Instance();
    
    auto rule = EventRule::Create("stats-test");
    rule.enabled = true;
    rule.trigger_events = {EventType::kMotionStart, EventType::kPersonDetected};
    
    rule.actions.push_back(CreateSnapshotAction());
    manager.AddRule(rule);
    
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    manager.RegisterActionHandler(ActionType::kCaptureSnapshot, handler);
    
    // Publish various events
    manager.PublishEvent(CreateMotionEvent({1}));
    manager.PublishEvent(CreateMotionEvent({2}));
    manager.PublishEvent(CreatePersonDetectedEvent());
    manager.PublishEvent(CreateVehicleDetectedEvent());  // Not in trigger
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto stats = manager.GetStats();
    
    EXPECT_GE(stats.events_published, 4);
    EXPECT_GE(stats.rules_matched, 3);  // 2 motion + 1 person
    EXPECT_GE(stats.actions_executed, 3);
}

// ============================================================================
// Event History Integration Tests
// ============================================================================

TEST_F(EventFlowIntegrationTest, HistoryContainsProcessedEvents) {
    auto& manager = EventManager::Instance();
    
    manager.PublishEvent(CreateMotionEvent({1}));
    manager.PublishEvent(CreatePersonDetectedEvent());
    manager.PublishEvent(CreateSystemEvent());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    EventQuery query;
    query.limit = 100;
    auto history = manager.QueryEvents(query);
    
    EXPECT_GE(history.size(), 3);
}
