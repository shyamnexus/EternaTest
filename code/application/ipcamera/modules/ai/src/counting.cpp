/**
 * @file counting.cpp
 * @brief People / Vehicle counting engine implementation
 */
#include "ipcam/counting.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <sstream>

namespace ipcam {
namespace ai {

void CountingEngine::Init(const CountingConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    stats_ = {};
    occupancy_high_fired_ = false;
    occupancy_low_fired_ = false;

    stats_.period_start = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    spdlog::info("CountingEngine: initialized (people={}, vehicles={}, reset={}s)",
                 config_.count_people, config_.count_vehicles,
                 config_.reset_interval_sec);
}

void CountingEngine::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = {};
}

void CountingEngine::OnLineCrossEvents(const std::vector<LineCrossEvent>& events) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!config_.enabled) return;

    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    for (const auto& evt : events) {
        bool is_person = (evt.category == ObjectCategory::kPerson);
        bool is_vehicle = (evt.category == ObjectCategory::kVehicle ||
                           evt.category == ObjectCategory::kNonMotor);

        if (evt.direction == "a_to_b") {
            stats_.total_in++;
            if (is_person && config_.count_people) stats_.people_in++;
            if (is_vehicle && config_.count_vehicles) stats_.vehicle_in++;
        } else {
            stats_.total_out++;
            if (is_person && config_.count_people) stats_.people_out++;
            if (is_vehicle && config_.count_vehicles) stats_.vehicle_out++;
        }
    }

    // Update occupancy (in minus out, floor at 0)
    stats_.people_present = std::max(0, stats_.people_in - stats_.people_out);
    stats_.vehicle_present = std::max(0, stats_.vehicle_in - stats_.vehicle_out);
    stats_.last_update = now;

    CheckThresholds(now);
}

void CountingEngine::UpdateAreaCounts(const std::vector<DetectionResult>& objects) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!config_.enabled) return;

    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    int people = 0;
    int vehicles = 0;

    for (const auto& obj : objects) {
        // Use foot-point (bottom-center) for zone test, matching demo 08
        if (!config_.zone_points.empty() && config_.zone_points.size() >= 3) {
            float fx = ((obj.bbox.x1 + obj.bbox.x2) * 0.5f) * 100.0f;
            float fy = obj.bbox.y2 * 100.0f;
            if (!PointInZone(fx, fy)) continue;
        }
        if (obj.category == ObjectCategory::kPerson) people++;
        else if (obj.category == ObjectCategory::kVehicle ||
                 obj.category == ObjectCategory::kNonMotor) vehicles++;
    }

    if (config_.count_people) stats_.people_present = people;
    if (config_.count_vehicles) stats_.vehicle_present = vehicles;
    stats_.last_update = now;

    CheckThresholds(now);
}

CountingStats CountingEngine::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void CountingEngine::Reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = {};
    stats_.period_start = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    occupancy_high_fired_ = false;
    occupancy_low_fired_ = false;
    spdlog::info("CountingEngine: counters reset");
}

void CountingEngine::CheckAutoReset() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (config_.reset_interval_sec <= 0) return;

    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    if (now - stats_.period_start >= config_.reset_interval_sec) {
        spdlog::info("CountingEngine: auto-reset after {}s", config_.reset_interval_sec);
        stats_ = {};
        stats_.period_start = now;
        occupancy_high_fired_ = false;
        occupancy_low_fired_ = false;
    }
}

void CountingEngine::SetConfig(const CountingConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
}

std::string CountingEngine::GetOsdText() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!config_.enabled || !config_.show_osd) return "";

    std::ostringstream oss;
    if (config_.count_people) {
        oss << "People: " << stats_.people_present;
        if (stats_.people_in > 0 || stats_.people_out > 0) {
            oss << " (In:" << stats_.people_in << " Out:" << stats_.people_out << ")";
        }
    }
    if (config_.count_vehicles) {
        if (config_.count_people) oss << "  ";
        oss << "Vehicles: " << stats_.vehicle_present;
        if (stats_.vehicle_in > 0 || stats_.vehicle_out > 0) {
            oss << " (In:" << stats_.vehicle_in << " Out:" << stats_.vehicle_out << ")";
        }
    }
    return oss.str();
}

bool CountingEngine::PointInZone(float px, float py) const {
    // Ray-casting point-in-polygon (percentage coords 0-100)
    const auto& pts = config_.zone_points;
    int n = static_cast<int>(pts.size());
    if (n < 3) return true; // No zone = everything counts

    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        float xi = static_cast<float>(pts[i].x);
        float yi = static_cast<float>(pts[i].y);
        float xj = static_cast<float>(pts[j].x);
        float yj = static_cast<float>(pts[j].y);

        bool intersect = ((yi > py) != (yj > py)) &&
                         (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
        if (intersect) inside = !inside;
    }
    return inside;
}

void CountingEngine::CheckThresholds(int64_t now) {
    if (!alert_cb_) return;

    int total_present = stats_.people_present + stats_.vehicle_present;

    // High threshold
    if (config_.occupancy_high_threshold > 0) {
        if (total_present >= config_.occupancy_high_threshold && !occupancy_high_fired_) {
            occupancy_high_fired_ = true;
            CountingAlert alert;
            alert.type = "occupancy_high";
            alert.current_value = total_present;
            alert.threshold = config_.occupancy_high_threshold;
            alert.timestamp = now;
            alert_cb_(alert);
        } else if (total_present < config_.occupancy_high_threshold) {
            occupancy_high_fired_ = false;
        }
    }

    // Low threshold (alerts when below)
    if (config_.occupancy_low_threshold > 0) {
        if (total_present <= config_.occupancy_low_threshold && !occupancy_low_fired_) {
            occupancy_low_fired_ = true;
            CountingAlert alert;
            alert.type = "occupancy_low";
            alert.current_value = total_present;
            alert.threshold = config_.occupancy_low_threshold;
            alert.timestamp = now;
            alert_cb_(alert);
        } else if (total_present > config_.occupancy_low_threshold) {
            occupancy_low_fired_ = false;
        }
    }
}

} // namespace ai
} // namespace ipcam
