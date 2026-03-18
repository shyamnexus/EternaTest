/**
 * @file mqtt_action.cpp
 * @brief MQTT publish action handler implementation
 */

#include "ipcam/action_handler.h"
#include <spdlog/spdlog.h>
#include <chrono>

#ifdef MQTT_ENABLED
#include <mqtt/async_client.h>
#endif

namespace ipcam {
namespace events {

#ifdef MQTT_ENABLED
static std::shared_ptr<mqtt::async_client> mqtt_client;

static bool MqttPublish(const std::string& topic, const std::string& payload, int qos, bool retain) {
    if (!mqtt_client || !mqtt_client->is_connected()) {
        spdlog::error("[MQTT] Client not connected");
        return false;
    }
    
    try {
        auto msg = mqtt::make_message(topic, payload);
        msg->set_qos(qos);
        msg->set_retained(retain);
        
        auto token = mqtt_client->publish(msg);
        token->wait_for(std::chrono::seconds(5));
        
        return true;
    } catch (const mqtt::exception& e) {
        spdlog::error("[MQTT] Publish failed: {}", e.what());
        return false;
    }
}

void SetMqttClient(std::shared_ptr<mqtt::async_client> client) {
    mqtt_client = client;
}
#else
// Stub implementation when MQTT is not available
static bool MqttPublish(const std::string& topic, const std::string& payload, int qos, bool retain) {
    spdlog::debug("[MQTT Stub] Would publish to topic: {}", topic);
    spdlog::debug("[MQTT Stub] Payload: {} bytes, QoS: {}, Retain: {}", 
                 payload.size(), qos, retain);
    return true;
}
#endif

ActionResult MqttActionHandler::Execute(const Event& event, const Action& action) {
    ActionResult result;
    result.execution_time_ms = 0;
    
    const auto* cfg = action.GetConfig<MqttActionConfig>();
    if (!cfg) {
        result.success = false;
        result.error_message = "Invalid configuration for MQTT action";
        return result;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Prepare topic
    std::string topic = SubstitutePlaceholders(cfg->topic, event);
    
    // Prepare payload
    std::string payload;
    if (!cfg->payload_template.empty()) {
        payload = SubstitutePlaceholders(cfg->payload_template, event);
    } else {
        // Default: send event as JSON
        payload = event.ToJson();
    }
    
    spdlog::info("[MqttActionHandler] Publishing to topic: {}", topic);
    
    bool success = MqttPublish(topic, payload, cfg->qos, cfg->retain);
    
    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    if (success) {
        result.success = true;
        result.output = "Published to MQTT topic: " + topic;
    } else {
        result.success = false;
        result.error_message = "Failed to publish MQTT message";
    }
    
    return result;
}

bool MqttActionHandler::IsAvailable() const {
#ifdef MQTT_ENABLED
    return mqtt_client != nullptr && mqtt_client->is_connected();
#else
    return false;
#endif
}

bool MqttActionHandler::Init() {
    return true;
}

void MqttActionHandler::Shutdown() {
}

} // namespace events
} // namespace ipcam
