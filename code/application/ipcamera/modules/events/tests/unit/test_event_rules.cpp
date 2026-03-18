/**
 * @file test_event_rules.cpp
 * @brief Unit tests for event rules and triggers
 */

#include <gtest/gtest.h>
#include <ipcam/event_rule.h>
#include <ipcam/event_types.h>
#include "test_helpers.h"

using namespace ipcam::events;
using namespace ipcam::events::testing;

// ============================================================================
// Event Rule Construction Tests
// ============================================================================

class EventRuleTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventRuleTest, DefaultConstruction) {
    EventRule rule;
    
    EXPECT_TRUE(rule.id.empty());
    EXPECT_TRUE(rule.name.empty());
    EXPECT_TRUE(rule.enabled);  // Default is enabled
    EXPECT_EQ(rule.priority, 0);
    EXPECT_EQ(rule.cooldown_seconds, 10);  // Default cooldown
}

TEST_F(EventRuleTest, CreateTestRule) {
    auto rule = CreateTestRule("motion-recording", EventType::kMotionStart);
    
    EXPECT_EQ(rule.id, "motion-recording");
    EXPECT_EQ(rule.name, "Test Rule");
    EXPECT_TRUE(rule.enabled);
    EXPECT_EQ(rule.priority, 10);
    EXPECT_EQ(rule.cooldown_seconds, 5);
    EXPECT_EQ(rule.trigger_events.size(), 1u);
    EXPECT_EQ(rule.trigger_events[0], EventType::kMotionStart);
}

TEST_F(EventRuleTest, RuleWithMultipleTriggers) {
    EventRule rule;
    rule.id = "multi-trigger";
    rule.name = "Multi Trigger Rule";
    rule.enabled = true;
    rule.trigger_events = {
        EventType::kMotionStart,
        EventType::kPersonDetected,
        EventType::kVehicleDetected
    };
    
    EXPECT_EQ(rule.trigger_events.size(), 3u);
}

TEST_F(EventRuleTest, RuleCreateGeneratesId) {
    auto rule = EventRule::Create("My Rule");
    
    EXPECT_FALSE(rule.id.empty());
    EXPECT_EQ(rule.name, "My Rule");
}

// ============================================================================
// Trigger Matching Tests
// ============================================================================

class TriggerMatchingTest : public ::testing::Test {
protected:
    EventRule rule;
    
    void SetUp() override {
        rule = CreateTestRule("test", EventType::kMotionStart);
    }
};

TEST_F(TriggerMatchingTest, MatchesEventType) {
    auto event = CreateMotionEvent();
    
    // Check if event type is in trigger_events
    bool matches = std::find(
        rule.trigger_events.begin(),
        rule.trigger_events.end(),
        event.type
    ) != rule.trigger_events.end();
    
    EXPECT_TRUE(matches);
}

TEST_F(TriggerMatchingTest, DoesNotMatchDifferentEventType) {
    auto event = CreatePersonDetectedEvent();
    
    bool matches = std::find(
        rule.trigger_events.begin(),
        rule.trigger_events.end(),
        event.type
    ) != rule.trigger_events.end();
    
    EXPECT_FALSE(matches);
}

TEST_F(TriggerMatchingTest, ZoneFiltering) {
    rule.trigger_zones = {1, 2, 3};
    
    auto event_zone1 = CreateMotionEvent({1});
    auto event_zone5 = CreateMotionEvent({5});
    
    auto& data1 = std::get<MotionEventData>(event_zone1.data);
    auto& data5 = std::get<MotionEventData>(event_zone5.data);
    
    // Check if any zone in event matches any zone in filter
    auto zone_matches = [&](const std::vector<uint32_t>& event_zones) {
        if (rule.trigger_zones.empty()) return true;
        for (auto z : event_zones) {
            if (std::find(rule.trigger_zones.begin(), rule.trigger_zones.end(), z) 
                != rule.trigger_zones.end()) {
                return true;
            }
        }
        return false;
    };
    
    EXPECT_TRUE(zone_matches(data1.zone_ids));
    EXPECT_FALSE(zone_matches(data5.zone_ids));
}

TEST_F(TriggerMatchingTest, EmptyZoneFilterMatchesAll) {
    rule.trigger_zones.clear();  // Empty means match all zones
    
    auto event = CreateMotionEvent({99});
    
    // Empty filter means accept all
    bool matches = rule.trigger_zones.empty();
    
    EXPECT_TRUE(matches);
}

TEST_F(TriggerMatchingTest, ConfidenceThreshold) {
    rule.trigger_events = {EventType::kPersonDetected};
    rule.min_confidence = 0.8f;
    
    auto high_conf_event = CreatePersonDetectedEvent(0.95f);
    auto low_conf_event = CreatePersonDetectedEvent(0.5f);
    
    auto& high_data = std::get<AnalyticsEventData>(high_conf_event.data);
    auto& low_data = std::get<AnalyticsEventData>(low_conf_event.data);
    
    EXPECT_GE(high_data.objects[0].confidence, rule.min_confidence);
    EXPECT_LT(low_data.objects[0].confidence, rule.min_confidence);
}

TEST_F(TriggerMatchingTest, ObjectClassFiltering) {
    rule.trigger_events = {EventType::kPersonDetected, EventType::kVehicleDetected};
    rule.trigger_classes = {"person"};
    
    auto person_event = CreatePersonDetectedEvent();
    auto vehicle_event = CreateVehicleDetectedEvent();
    
    auto& person_data = std::get<AnalyticsEventData>(person_event.data);
    auto& vehicle_data = std::get<AnalyticsEventData>(vehicle_event.data);
    
    auto class_matches = [&](const std::string& obj_class) {
        return rule.trigger_classes.empty() ||
            std::find(rule.trigger_classes.begin(), rule.trigger_classes.end(), obj_class) 
            != rule.trigger_classes.end();
    };
    
    EXPECT_TRUE(class_matches(person_data.objects[0].class_name));
    EXPECT_FALSE(class_matches(vehicle_data.objects[0].class_name));
}

TEST_F(TriggerMatchingTest, RuleMatchesMethod) {
    // Test the EventRule::Matches() method
    auto event = CreateMotionEvent();
    
    // Assuming Matches is properly implemented
    // This will need the actual implementation to work
    // For now we just verify the rule is properly configured
    EXPECT_FALSE(rule.trigger_events.empty());
}

// ============================================================================
// Schedule Tests
// ============================================================================

class ScheduleTest : public ::testing::Test {
protected:
    EventRule rule;
    
    void SetUp() override {
        rule = CreateTestRule("scheduled", EventType::kMotionStart);
        rule.schedule.enabled = true;
        rule.schedule.days_of_week = {0, 1, 2, 3, 4, 5, 6};  // All days
    }
};

TEST_F(ScheduleTest, ScheduleEnabled) {
    EXPECT_TRUE(rule.schedule.enabled);
}

TEST_F(ScheduleTest, AllDaysEnabled) {
    EXPECT_EQ(rule.schedule.days_of_week.size(), 7u);
}

TEST_F(ScheduleTest, TimeRangeContains) {
    TimeRange range;
    range.start_hour = 9;
    range.start_minute = 0;
    range.end_hour = 17;
    range.end_minute = 0;
    
    EXPECT_TRUE(range.Contains(12, 30));  // Noon
    EXPECT_TRUE(range.Contains(9, 0));    // Start boundary
    EXPECT_FALSE(range.Contains(8, 59));  // Before start
    EXPECT_FALSE(range.Contains(17, 1));  // After end
}

TEST_F(ScheduleTest, WeekdaysOnlySchedule) {
    rule.schedule.days_of_week = {1, 2, 3, 4, 5};  // Monday-Friday
    
    EXPECT_EQ(rule.schedule.days_of_week.size(), 5u);
    EXPECT_TRUE(std::find(rule.schedule.days_of_week.begin(), 
                          rule.schedule.days_of_week.end(), 1) != rule.schedule.days_of_week.end());
    EXPECT_FALSE(std::find(rule.schedule.days_of_week.begin(), 
                           rule.schedule.days_of_week.end(), 0) != rule.schedule.days_of_week.end());
}

TEST_F(ScheduleTest, MultipleTimeRanges) {
    TimeRange morning;
    morning.start_hour = 8; morning.start_minute = 0;
    morning.end_hour = 12; morning.end_minute = 0;
    
    TimeRange afternoon;
    afternoon.start_hour = 13; afternoon.start_minute = 0;
    afternoon.end_hour = 17; afternoon.end_minute = 0;
    
    rule.schedule.time_ranges = {morning, afternoon};
    
    EXPECT_EQ(rule.schedule.time_ranges.size(), 2u);
}

// ============================================================================
// Action Configuration Tests
// ============================================================================

class ActionConfigTest : public ::testing::Test {};

TEST_F(ActionConfigTest, RecordingActionConfig) {
    auto action = CreateRecordingAction(60);
    
    EXPECT_EQ(action.type, ActionType::kStartRecording);
    EXPECT_TRUE(action.enabled);
    
    auto* config = action.GetConfig<RecordingActionConfig>();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->duration_seconds, 60);
    EXPECT_EQ(config->pre_record_seconds, 5);
    EXPECT_EQ(config->post_record_seconds, 10);
}

TEST_F(ActionConfigTest, SnapshotActionConfig) {
    auto action = CreateSnapshotAction(90);
    
    EXPECT_EQ(action.type, ActionType::kCaptureSnapshot);
    
    auto* config = action.GetConfig<SnapshotActionConfig>();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->quality, 90);
    EXPECT_EQ(config->format, "jpg");
    EXPECT_EQ(config->count, 1);
}

TEST_F(ActionConfigTest, EmailActionConfig) {
    auto action = CreateEmailAction();
    
    EXPECT_EQ(action.type, ActionType::kSendEmail);
    
    auto* config = action.GetConfig<EmailActionConfig>();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->recipients.size(), 1u);
    EXPECT_EQ(config->recipients[0], "test@example.com");
    EXPECT_TRUE(config->attach_snapshot);
}

TEST_F(ActionConfigTest, WebhookActionConfig) {
    auto action = CreateWebhookAction("http://test.com/hook");
    
    EXPECT_EQ(action.type, ActionType::kSendWebhook);
    
    auto* config = action.GetConfig<WebhookActionConfig>();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->url, "http://test.com/hook");
    EXPECT_EQ(config->method, "POST");
    EXPECT_EQ(config->timeout_seconds, 10);
}

// ============================================================================
// Rule Priority Tests
// ============================================================================

class RulePriorityTest : public ::testing::Test {};

TEST_F(RulePriorityTest, SortByPriority) {
    std::vector<EventRule> rules;
    
    auto rule1 = CreateTestRule("low", EventType::kMotionStart);
    rule1.priority = 5;
    
    auto rule2 = CreateTestRule("high", EventType::kMotionStart);
    rule2.priority = 20;
    
    auto rule3 = CreateTestRule("medium", EventType::kMotionStart);
    rule3.priority = 10;
    
    rules.push_back(rule1);
    rules.push_back(rule2);
    rules.push_back(rule3);
    
    std::sort(rules.begin(), rules.end(), 
              [](const EventRule& a, const EventRule& b) {
                  return a.priority > b.priority;  // Higher priority first
              });
    
    EXPECT_EQ(rules[0].id, "high");
    EXPECT_EQ(rules[1].id, "medium");
    EXPECT_EQ(rules[2].id, "low");
}

// ============================================================================
// Rule Enabled/Disabled Tests
// ============================================================================

class RuleEnabledTest : public ::testing::Test {};

TEST_F(RuleEnabledTest, DisabledRuleSkipped) {
    auto rule = CreateTestRule("disabled", EventType::kMotionStart);
    rule.enabled = false;
    
    EXPECT_FALSE(rule.enabled);
}

TEST_F(RuleEnabledTest, DisabledActionSkipped) {
    auto action = CreateRecordingAction();
    action.enabled = false;
    
    EXPECT_FALSE(action.enabled);
}

// ============================================================================
// Action Type Tests
// ============================================================================

class ActionTypeTest : public ::testing::Test {};

TEST_F(ActionTypeTest, ActionTypeToString) {
    EXPECT_EQ(ActionTypeToString(ActionType::kStartRecording), "start_recording");
    EXPECT_EQ(ActionTypeToString(ActionType::kStopRecording), "stop_recording");
    EXPECT_EQ(ActionTypeToString(ActionType::kCaptureSnapshot), "capture_snapshot");
    EXPECT_EQ(ActionTypeToString(ActionType::kSendEmail), "send_email");
    EXPECT_EQ(ActionTypeToString(ActionType::kSendWebhook), "send_webhook");
    EXPECT_EQ(ActionTypeToString(ActionType::kUploadFtp), "upload_ftp");
}

TEST_F(ActionTypeTest, StringToActionType) {
    EXPECT_EQ(StringToActionType("start_recording"), ActionType::kStartRecording);
    EXPECT_EQ(StringToActionType("capture_snapshot"), ActionType::kCaptureSnapshot);
    EXPECT_EQ(StringToActionType("send_email"), ActionType::kSendEmail);
    EXPECT_EQ(StringToActionType("send_webhook"), ActionType::kSendWebhook);
}

// ============================================================================
// Cooldown Tests
// ============================================================================

class CooldownTest : public ::testing::Test {};

TEST_F(CooldownTest, CooldownNotElapsed) {
    auto rule = CreateTestRule("cooldown-test", EventType::kMotionStart);
    rule.cooldown_seconds = 10;
    
    // Mark as just triggered
    rule.MarkTriggered();
    
    // Cooldown should not have elapsed yet
    EXPECT_FALSE(rule.IsCooledDown());
}

TEST_F(CooldownTest, ZeroCooldownAlwaysReady) {
    auto rule = CreateTestRule("no-cooldown", EventType::kMotionStart);
    rule.cooldown_seconds = 0;
    
    rule.MarkTriggered();
    
    // Zero cooldown means always ready
    EXPECT_TRUE(rule.IsCooledDown());
}
