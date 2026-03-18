/**
 * @file test_event_types.cpp
 * @brief Unit tests for event types and data structures
 */

#include <gtest/gtest.h>
#include <ipcam/event_types.h>
#include "test_helpers.h"

using namespace ipcam::events;
using namespace ipcam::events::testing;

// ============================================================================
// Event Category Tests
// ============================================================================

class EventCategoryTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventCategoryTest, CategoryToStringConversion) {
    EXPECT_EQ(EventCategoryToString(EventCategory::kMotion), "motion");
    EXPECT_EQ(EventCategoryToString(EventCategory::kAnalytics), "analytics");
    EXPECT_EQ(EventCategoryToString(EventCategory::kLineCrossing), "line_crossing");
    EXPECT_EQ(EventCategoryToString(EventCategory::kIntrusion), "intrusion");
    EXPECT_EQ(EventCategoryToString(EventCategory::kSystem), "system");
    EXPECT_EQ(EventCategoryToString(EventCategory::kIO), "io");
    EXPECT_EQ(EventCategoryToString(EventCategory::kStorage), "storage");
    EXPECT_EQ(EventCategoryToString(EventCategory::kNetwork), "network");
    EXPECT_EQ(EventCategoryToString(EventCategory::kSchedule), "schedule");
}

TEST_F(EventCategoryTest, StringToCategoryConversion) {
    EXPECT_EQ(StringToEventCategory("motion"), EventCategory::kMotion);
    EXPECT_EQ(StringToEventCategory("analytics"), EventCategory::kAnalytics);
    EXPECT_EQ(StringToEventCategory("line_crossing"), EventCategory::kLineCrossing);
    EXPECT_EQ(StringToEventCategory("intrusion"), EventCategory::kIntrusion);
    EXPECT_EQ(StringToEventCategory("system"), EventCategory::kSystem);
}

// ============================================================================
// Event Type Tests
// ============================================================================

class EventTypeTest : public ::testing::Test {};

TEST_F(EventTypeTest, TypeToStringConversion) {
    EXPECT_EQ(EventTypeToString(EventType::kMotionStart), "motion_start");
    EXPECT_EQ(EventTypeToString(EventType::kMotionEnd), "motion_end");
    EXPECT_EQ(EventTypeToString(EventType::kPersonDetected), "person_detected");
    EXPECT_EQ(EventTypeToString(EventType::kVehicleDetected), "vehicle_detected");
    EXPECT_EQ(EventTypeToString(EventType::kLineCrossedLeftToRight), "line_crossed_left_to_right");
    EXPECT_EQ(EventTypeToString(EventType::kZoneEntered), "zone_entered");
    EXPECT_EQ(EventTypeToString(EventType::kSystemStartup), "system_startup");
}

TEST_F(EventTypeTest, StringToTypeConversion) {
    EXPECT_EQ(StringToEventType("motion_start"), EventType::kMotionStart);
    EXPECT_EQ(StringToEventType("person_detected"), EventType::kPersonDetected);
    EXPECT_EQ(StringToEventType("vehicle_detected"), EventType::kVehicleDetected);
    EXPECT_EQ(StringToEventType("system_startup"), EventType::kSystemStartup);
}

// ============================================================================
// Event Data Structure Tests
// ============================================================================

class EventDataTest : public ::testing::Test {};

TEST_F(EventDataTest, MotionEventDataConstruction) {
    MotionEventData data;
    data.zone_ids = {1, 2};
    data.motion_level = 0.85f;
    data.motion_block_count = 15;
    data.total_blocks = 100;
    
    EXPECT_EQ(data.zone_ids.size(), 2u);
    EXPECT_EQ(data.zone_ids[0], 1u);
    EXPECT_FLOAT_EQ(data.motion_level, 0.85f);
    EXPECT_EQ(data.motion_block_count, 15);
    EXPECT_EQ(data.total_blocks, 100);
}

TEST_F(EventDataTest, AnalyticsEventDataConstruction) {
    AnalyticsEventData data;
    
    DetectedObject person;
    person.id = 42;
    person.class_name = "person";
    person.confidence = 0.95f;
    person.x1 = 0.1f; person.y1 = 0.2f;
    person.x2 = 0.3f; person.y2 = 0.6f;
    
    data.objects.push_back(person);
    data.total_persons = 1;
    
    EXPECT_EQ(data.objects.size(), 1u);
    EXPECT_EQ(data.objects[0].class_name, "person");
    EXPECT_FLOAT_EQ(data.objects[0].confidence, 0.95f);
    EXPECT_EQ(data.objects[0].id, 42u);
    EXPECT_EQ(data.total_persons, 1);
}

TEST_F(EventDataTest, DetectedObjectHelperMethods) {
    DetectedObject obj;
    obj.x1 = 0.1f; obj.y1 = 0.2f;
    obj.x2 = 0.3f; obj.y2 = 0.6f;
    
    EXPECT_FLOAT_EQ(obj.Width(), 0.2f);
    EXPECT_FLOAT_EQ(obj.Height(), 0.4f);
    EXPECT_FLOAT_EQ(obj.CenterX(), 0.2f);
    EXPECT_FLOAT_EQ(obj.CenterY(), 0.4f);
}

TEST_F(EventDataTest, LineCrossEventDataConstruction) {
    LineCrossEventData data;
    data.line_id = 0;
    data.line_name = "Entry Line";
    data.direction = "left_to_right";
    data.object_id = 1;
    data.object_class = "vehicle";
    data.count_in = 5;
    data.count_out = 3;
    
    EXPECT_EQ(data.line_id, 0u);
    EXPECT_EQ(data.line_name, "Entry Line");
    EXPECT_EQ(data.direction, "left_to_right");
    EXPECT_EQ(data.object_class, "vehicle");
    EXPECT_EQ(data.count_in, 5);
    EXPECT_EQ(data.count_out, 3);
}

TEST_F(EventDataTest, IntrusionEventDataConstruction) {
    IntrusionEventData data;
    data.zone_id = 2;
    data.zone_name = "Restricted Area";
    data.object_id = 5;
    data.object_class = "person";
    data.dwell_time_ms = 30000;
    
    EXPECT_EQ(data.zone_id, 2u);
    EXPECT_EQ(data.zone_name, "Restricted Area");
    EXPECT_EQ(data.dwell_time_ms, 30000);
}

TEST_F(EventDataTest, SystemEventDataConstruction) {
    SystemEventData data;
    data.message = "System started successfully";
    data.severity = "info";
    data.component = "core";
    data.error_code = 0;
    
    EXPECT_EQ(data.message, "System started successfully");
    EXPECT_EQ(data.severity, "info");
    EXPECT_EQ(data.component, "core");
    EXPECT_EQ(data.error_code, 0);
}

// ============================================================================
// Event Construction Tests
// ============================================================================

class EventConstructionTest : public ::testing::Test {};

TEST_F(EventConstructionTest, CreateMotionEvent) {
    auto event = CreateMotionEvent({1, 2}, 0.60f);
    
    EXPECT_EQ(event.category, EventCategory::kMotion);
    EXPECT_EQ(event.type, EventType::kMotionStart);
    EXPECT_EQ(event.source, "motion_detector");
    EXPECT_TRUE(std::holds_alternative<MotionEventData>(event.data));
    
    auto& data = std::get<MotionEventData>(event.data);
    EXPECT_EQ(data.zone_ids.size(), 2u);
    EXPECT_EQ(data.zone_ids[0], 1u);
    EXPECT_FLOAT_EQ(data.motion_level, 0.60f);
}

TEST_F(EventConstructionTest, CreatePersonEvent) {
    auto event = CreatePersonDetectedEvent(0.92f);
    
    EXPECT_EQ(event.category, EventCategory::kAnalytics);
    EXPECT_EQ(event.type, EventType::kPersonDetected);
    EXPECT_TRUE(std::holds_alternative<AnalyticsEventData>(event.data));
    
    auto& data = std::get<AnalyticsEventData>(event.data);
    EXPECT_EQ(data.objects.size(), 1u);
    EXPECT_EQ(data.objects[0].class_name, "person");
    EXPECT_FLOAT_EQ(data.objects[0].confidence, 0.92f);
}

TEST_F(EventConstructionTest, CreateVehicleEvent) {
    auto event = CreateVehicleDetectedEvent(0.88f);
    
    EXPECT_EQ(event.category, EventCategory::kAnalytics);
    EXPECT_EQ(event.type, EventType::kVehicleDetected);
    EXPECT_TRUE(std::holds_alternative<AnalyticsEventData>(event.data));
    
    auto& data = std::get<AnalyticsEventData>(event.data);
    EXPECT_EQ(data.objects[0].class_name, "vehicle");
}

TEST_F(EventConstructionTest, CreateSystemEvent) {
    auto event = CreateSystemEvent(EventType::kSystemStartup, "Test startup");
    
    EXPECT_EQ(event.category, EventCategory::kSystem);
    EXPECT_EQ(event.type, EventType::kSystemStartup);
    EXPECT_TRUE(std::holds_alternative<SystemEventData>(event.data));
    
    auto& data = std::get<SystemEventData>(event.data);
    EXPECT_EQ(data.message, "Test startup");
}

// ============================================================================
// Event Timestamp Tests
// ============================================================================

class EventTimestampTest : public ::testing::Test {};

TEST_F(EventTimestampTest, TimestampIsSet) {
    auto event = CreateMotionEvent();
    
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - event.timestamp);
    
    // Timestamp should be within 1 second of now
    EXPECT_LE(diff.count(), 1);
}

TEST_F(EventTimestampTest, ToIsoTimestampFormat) {
    Event event = Event::Create(EventCategory::kSystem, EventType::kSystemStartup);
    
    std::string iso = event.ToIsoTimestamp();
    
    // ISO format should look like: 2026-01-31T12:34:56Z
    EXPECT_GE(iso.length(), 19u);
    EXPECT_EQ(iso[4], '-');
    EXPECT_EQ(iso[7], '-');
    EXPECT_EQ(iso[10], 'T');
    EXPECT_EQ(iso[13], ':');
    EXPECT_EQ(iso[16], ':');
}

// ============================================================================
// Event ID Tests
// ============================================================================

class EventIdTest : public ::testing::Test {};

TEST_F(EventIdTest, EventCreateGeneratesUUID) {
    auto event1 = Event::Create(EventCategory::kMotion, EventType::kMotionStart);
    auto event2 = Event::Create(EventCategory::kMotion, EventType::kMotionStart);
    
    // Event::Create should generate unique UUIDs
    EXPECT_FALSE(event1.id.empty());
    EXPECT_FALSE(event2.id.empty());
    EXPECT_NE(event1.id, event2.id);
}

// ============================================================================
// Event Variant Data Tests
// ============================================================================

class EventVariantTest : public ::testing::Test {};

TEST_F(EventVariantTest, VariantHoldsCorrectType) {
    auto motion_event = CreateMotionEvent();
    auto person_event = CreatePersonDetectedEvent();
    auto system_event = CreateSystemEvent();
    
    EXPECT_TRUE(std::holds_alternative<MotionEventData>(motion_event.data));
    EXPECT_FALSE(std::holds_alternative<AnalyticsEventData>(motion_event.data));
    
    EXPECT_TRUE(std::holds_alternative<AnalyticsEventData>(person_event.data));
    EXPECT_FALSE(std::holds_alternative<MotionEventData>(person_event.data));
    
    EXPECT_TRUE(std::holds_alternative<SystemEventData>(system_event.data));
}

TEST_F(EventVariantTest, SafeVariantAccess) {
    auto event = CreateMotionEvent();
    
    // Safe access with get_if
    auto* motion_data = std::get_if<MotionEventData>(&event.data);
    auto* analytics_data = std::get_if<AnalyticsEventData>(&event.data);
    
    EXPECT_NE(motion_data, nullptr);
    EXPECT_EQ(analytics_data, nullptr);
}
