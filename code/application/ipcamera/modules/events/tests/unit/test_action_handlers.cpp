/**
 * @file test_action_handlers.cpp
 * @brief Unit tests for action handlers
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ipcam/action_handler.h>
#include <ipcam/event_types.h>
#include <ipcam/event_rule.h>
#include "test_helpers.h"
#include "mock_handlers.h"

using namespace ipcam::events;
using namespace ipcam::events::testing;

// ============================================================================
// Mock Action Handler Tests
// ============================================================================

class MockHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockActionHandlerFactory::Instance().ResetAll();
    }
    
    void TearDown() override {
        MockActionHandlerFactory::Instance().ResetAll();
    }
};

TEST_F(MockHandlerTest, MockHandlerExecutes) {
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kStartRecording);
    
    auto event = CreateMotionEvent();
    auto action = CreateRecordingAction(30);
    
    auto result = handler->Execute(event, action);
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(handler->GetExecutionCount(), 1);
}

TEST_F(MockHandlerTest, MockHandlerRecordsExecutions) {
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kCaptureSnapshot);
    
    auto event1 = CreateMotionEvent({1});
    auto event2 = CreatePersonDetectedEvent();
    
    auto action = CreateSnapshotAction();
    
    handler->Execute(event1, action);
    handler->Execute(event2, action);
    
    auto records = handler->GetRecords();
    
    EXPECT_EQ(records.size(), 2u);
    EXPECT_EQ(records[0].type, ActionType::kCaptureSnapshot);
    EXPECT_EQ(records[1].type, ActionType::kCaptureSnapshot);
}

TEST_F(MockHandlerTest, MockHandlerCanFail) {
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kSendEmail);
    handler->SetShouldSucceed(false, "SMTP connection failed");
    
    auto event = CreateMotionEvent();
    auto action = CreateEmailAction();
    
    auto result = handler->Execute(event, action);
    
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_message, "SMTP connection failed");
    
    auto records = handler->GetRecords();
    EXPECT_FALSE(records[0].success);
    EXPECT_EQ(records[0].error, "SMTP connection failed");
}

TEST_F(MockHandlerTest, MockHandlerReset) {
    auto handler = MockActionHandlerFactory::Instance().GetHandler(ActionType::kSendWebhook);
    
    auto event = CreateMotionEvent();
    auto action = CreateWebhookAction();
    
    handler->Execute(event, action);
    handler->Execute(event, action);
    
    EXPECT_EQ(handler->GetExecutionCount(), 2);
    
    handler->Reset();
    
    EXPECT_EQ(handler->GetExecutionCount(), 0);
    EXPECT_EQ(handler->GetRecords().size(), 0u);
}

// ============================================================================
// Action Handler Factory Tests
// ============================================================================

class ActionHandlerFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockActionHandlerFactory::Instance().ResetAll();
    }
};

TEST_F(ActionHandlerFactoryTest, GetAllRecords) {
    auto& factory = MockActionHandlerFactory::Instance();
    
    auto recording_handler = factory.GetHandler(ActionType::kStartRecording);
    auto snapshot_handler = factory.GetHandler(ActionType::kCaptureSnapshot);
    
    auto event = CreateMotionEvent();
    auto rec_action = CreateRecordingAction();
    auto snap_action = CreateSnapshotAction();
    
    recording_handler->Execute(event, rec_action);
    snapshot_handler->Execute(event, snap_action);
    snapshot_handler->Execute(event, snap_action);
    
    auto all_records = factory.GetAllRecords();
    
    EXPECT_EQ(all_records.size(), 3u);
}

TEST_F(ActionHandlerFactoryTest, ResetAllHandlers) {
    auto& factory = MockActionHandlerFactory::Instance();
    
    auto handler1 = factory.GetHandler(ActionType::kStartRecording);
    auto handler2 = factory.GetHandler(ActionType::kCaptureSnapshot);
    
    auto event = CreateMotionEvent();
    
    handler1->Execute(event, CreateRecordingAction());
    handler2->Execute(event, CreateSnapshotAction());
    
    factory.ResetAll();
    
    EXPECT_EQ(handler1->GetExecutionCount(), 0);
    EXPECT_EQ(handler2->GetExecutionCount(), 0);
}

// ============================================================================
// Action Priority Tests
// ============================================================================

class ActionPriorityTest : public ::testing::Test {};

TEST_F(ActionPriorityTest, SortActionsByPriority) {
    std::vector<Action> actions;
    
    auto a1 = CreateRecordingAction();
    a1.priority = 5;
    
    auto a2 = CreateSnapshotAction();
    a2.priority = 20;
    
    auto a3 = CreateEmailAction();
    a3.priority = 10;
    
    actions.push_back(a1);
    actions.push_back(a2);
    actions.push_back(a3);
    
    // Sort by priority (higher first)
    std::sort(actions.begin(), actions.end(),
              [](const Action& a, const Action& b) {
                  return a.priority > b.priority;
              });
    
    EXPECT_EQ(actions[0].type, ActionType::kCaptureSnapshot);  // priority 20
    EXPECT_EQ(actions[1].type, ActionType::kSendEmail);        // priority 10
    EXPECT_EQ(actions[2].type, ActionType::kStartRecording);   // priority 5
}

// ============================================================================
// Action Config Variant Tests
// ============================================================================

class ActionConfigVariantTest : public ::testing::Test {};

TEST_F(ActionConfigVariantTest, RecordingConfigVariant) {
    RecordingActionConfig config;
    config.duration_seconds = 60;
    
    ActionConfig variant = config;
    
    EXPECT_TRUE(std::holds_alternative<RecordingActionConfig>(variant));
    
    auto* retrieved = std::get_if<RecordingActionConfig>(&variant);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->duration_seconds, 60);
}

TEST_F(ActionConfigVariantTest, SnapshotConfigVariant) {
    SnapshotActionConfig config;
    config.count = 3;
    config.quality = 95;
    
    ActionConfig variant = config;
    
    EXPECT_TRUE(std::holds_alternative<SnapshotActionConfig>(variant));
    
    auto* retrieved = std::get_if<SnapshotActionConfig>(&variant);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->count, 3);
    EXPECT_EQ(retrieved->quality, 95);
}

TEST_F(ActionConfigVariantTest, EmailConfigVariant) {
    EmailActionConfig config;
    config.recipients = {"a@test.com", "b@test.com"};
    config.subject_template = "Test Subject";
    
    ActionConfig variant = config;
    
    EXPECT_TRUE(std::holds_alternative<EmailActionConfig>(variant));
    
    auto* retrieved = std::get_if<EmailActionConfig>(&variant);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->recipients.size(), 2);
}

TEST_F(ActionConfigVariantTest, WebhookConfigVariant) {
    WebhookActionConfig config;
    config.url = "https://webhook.site/test";
    config.method = "POST";
    config.timeout_seconds = 30;
    
    ActionConfig variant = config;
    
    EXPECT_TRUE(std::holds_alternative<WebhookActionConfig>(variant));
    
    auto* retrieved = std::get_if<WebhookActionConfig>(&variant);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->url, "https://webhook.site/test");
    EXPECT_EQ(retrieved->timeout_seconds, 30);
}

// ============================================================================
// Placeholder Substitution Tests
// ============================================================================

class PlaceholderSubstitutionTest : public ::testing::Test {};

TEST_F(PlaceholderSubstitutionTest, SubstitutePlaceholders) {
    std::string template_str = "Event: {event_type} at {timestamp} from {device_name}";
    
    std::map<std::string, std::string> placeholders = {
        {"event_type", "motion_start"},
        {"timestamp", "2026-01-31T12:00:00Z"},
        {"device_name", "Front Camera"}
    };
    
    // Simple substitution
    std::string result = template_str;
    for (const auto& [key, value] : placeholders) {
        std::string placeholder = "{" + key + "}";
        size_t pos;
        while ((pos = result.find(placeholder)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
        }
    }
    
    EXPECT_EQ(result, "Event: motion_start at 2026-01-31T12:00:00Z from Front Camera");
}

TEST_F(PlaceholderSubstitutionTest, JsonTemplatePlaceholders) {
    std::string json_template = R"({"event": "{event_type}", "zone": {zone_id}, "conf": {confidence}})";
    
    std::map<std::string, std::string> placeholders = {
        {"event_type", "person_detected"},
        {"zone_id", "1"},
        {"confidence", "0.95"}
    };
    
    std::string result = json_template;
    for (const auto& [key, value] : placeholders) {
        std::string placeholder = "{" + key + "}";
        size_t pos;
        while ((pos = result.find(placeholder)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
        }
    }
    
    EXPECT_EQ(result, R"({"event": "person_detected", "zone": 1, "conf": 0.95})");
}

// ============================================================================
// Action Delay Tests
// ============================================================================

class ActionDelayTest : public ::testing::Test {};

TEST_F(ActionDelayTest, ActionWithDelay) {
    Action action = CreateRecordingAction();
    action.delay_ms = 1000;  // 1 second delay
    
    EXPECT_EQ(action.delay_ms, 1000);
}

TEST_F(ActionDelayTest, ActionWithoutDelay) {
    Action action = CreateSnapshotAction();
    action.delay_ms = 0;  // No delay
    
    EXPECT_EQ(action.delay_ms, 0);
}

// ============================================================================
// Action Result Tests
// ============================================================================

class ActionResultTest : public ::testing::Test {};

TEST_F(ActionResultTest, SuccessResult) {
    ActionResult result;
    result.success = true;
    
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.error_message.empty());
}

TEST_F(ActionResultTest, ErrorResult) {
    ActionResult result;
    result.success = false;
    result.error_message = "Connection timeout";
    
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_message, "Connection timeout");
}

TEST_F(ActionResultTest, ResultWithOutput) {
    ActionResult result;
    result.success = true;
    result.output = "/tmp/snapshot.jpg";
    result.execution_time_ms = 42;
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "/tmp/snapshot.jpg");
    EXPECT_EQ(result.execution_time_ms, 42);
}

TEST_F(ActionResultTest, ResultWithMetadata) {
    ActionResult result;
    result.success = false;
    result.error_message = "Not found";
    result.metadata["code"] = "404";
    
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_message, "Not found");
    EXPECT_EQ(result.metadata["code"], "404");
}
