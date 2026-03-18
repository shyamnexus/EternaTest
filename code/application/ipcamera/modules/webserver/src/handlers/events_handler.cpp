/**
 * @file events_handler.cpp
 * @brief Web API handlers for Events management (function-based)
 */

#include "events_handler.h"
#include "ipcam/event_manager.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <chrono>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

// Helper: Extract ID from URI path
static std::string ExtractIdFromPath(const std::string& uri, const std::string& prefix) {
    if (uri.size() <= prefix.size()) return "";
    std::string id = uri.substr(prefix.size());
    // Remove trailing slash
    if (!id.empty() && id.back() == '/') id.pop_back();
    // Remove leading slash
    if (!id.empty() && id.front() == '/') id = id.substr(1);
    return id;
}

// Helper: Parse query parameter
static std::string GetQueryParam(const api::RequestContext& ctx, const std::string& key, const std::string& def = "") {
    auto it = ctx.query_params.find(key);
    return (it != ctx.query_params.end()) ? it->second : def;
}

// ============================================================================
// GET/DELETE /api/v1/events - Query or clear events
// GET /api/v1/events/{id} - Get specific event
// ============================================================================
api::Response HandleEvents(const api::RequestContext& ctx) {
    api::Response resp;
    resp.headers["Content-Type"] = "application/json";
    
    auto& manager = events::EventManager::Instance();
    
    // Extract event ID if present (e.g., /api/v1/events/abc123)
    std::string event_id = ExtractIdFromPath(ctx.uri, "/api/v1/events");
    
    if (ctx.method == "GET") {
        if (!event_id.empty()) {
            // GET /api/v1/events/{id}
            auto event = manager.GetEvent(event_id);
            if (event.has_value()) {
                resp.status_code = 200;
                resp.body = event->ToJson();
            } else {
                resp.status_code = 404;
                resp.body = R"({"error":"Event not found"})";
            }
        } else {
            // GET /api/v1/events - Query events
            events::EventQuery query;
            
            std::string limit_str = GetQueryParam(ctx, "limit", "100");
            query.limit = std::stoul(limit_str);
            
            std::string category_str = GetQueryParam(ctx, "category");
            if (!category_str.empty()) {
                query.category = events::StringToEventCategory(category_str);
            }
            
            auto events_list = manager.QueryEvents(query);
            
            json response;
            response["events"] = json::array();
            for (const auto& evt : events_list) {
                response["events"].push_back(json::parse(evt.ToJson()));
            }
            response["count"] = events_list.size();
            
            resp.status_code = 200;
            resp.body = response.dump(2);
        }
    }
    else if (ctx.method == "DELETE") {
        // DELETE /api/v1/events - Clear history
        manager.ClearHistory();
        resp.status_code = 200;
        resp.body = R"({"success":true,"message":"Event history cleared"})";
    }
    else {
        resp.status_code = 405;
        resp.body = R"({"error":"Method not allowed"})";
    }
    
    return resp;
}

// ============================================================================
// /api/v1/events/rules - Rule management
// ============================================================================
api::Response HandleEventsRules(const api::RequestContext& ctx) {
    api::Response resp;
    resp.headers["Content-Type"] = "application/json";
    
    auto& manager = events::EventManager::Instance();
    
    // Extract rule ID if present (e.g., /api/v1/events/rules/abc123)
    std::string rule_id = ExtractIdFromPath(ctx.uri, "/api/v1/events/rules");
    
    if (ctx.method == "GET") {
        if (!rule_id.empty()) {
            // GET /api/v1/events/rules/{id}
            auto rule = manager.GetRule(rule_id);
            if (rule.has_value()) {
                resp.status_code = 200;
                resp.body = rule->ToJson();
            } else {
                resp.status_code = 404;
                resp.body = R"({"error":"Rule not found"})";
            }
        } else {
            // GET /api/v1/events/rules - List all rules
            auto rules = manager.GetRules();
            
            json response;
            response["rules"] = json::array();
            for (const auto& rule : rules) {
                response["rules"].push_back(json::parse(rule.ToJson()));
            }
            response["count"] = rules.size();
            
            resp.status_code = 200;
            resp.body = response.dump(2);
        }
    }
    else if (ctx.method == "POST") {
        // POST /api/v1/events/rules - Create new rule
        try {
            events::EventRule rule = events::EventRule::FromJson(ctx.body.empty() ? "{}" : ctx.body);
            
            // Generate ID if not provided
            if (rule.id.empty()) {
                rule.id = "rule_" + std::to_string(
                    std::chrono::steady_clock::now().time_since_epoch().count());
            }
            
            if (manager.AddRule(rule)) {
                json response;
                response["success"] = true;
                response["rule"] = json::parse(rule.ToJson());
                resp.status_code = 201;
                resp.body = response.dump(2);
            } else {
                resp.status_code = 409;
                resp.body = R"({"error":"Rule with this ID already exists"})";
            }
        } catch (const std::exception& e) {
            resp.status_code = 400;
            json err;
            err["error"] = std::string("Invalid rule: ") + e.what();
            resp.body = err.dump();
        }
    }
    else if (ctx.method == "PUT") {
        // PUT /api/v1/events/rules/{id} - Update rule
        if (rule_id.empty()) {
            resp.status_code = 400;
            resp.body = R"({"error":"Rule ID required"})";
            return resp;
        }
        
        try {
            events::EventRule rule = events::EventRule::FromJson(ctx.body.empty() ? "{}" : ctx.body);
            rule.id = rule_id;  // Ensure ID matches URL
            
            if (manager.UpdateRule(rule)) {
                json response;
                response["success"] = true;
                response["rule"] = rule.ToJson();
                resp.status_code = 200;
                resp.body = response.dump(2);
            } else {
                resp.status_code = 404;
                resp.body = R"({"error":"Rule not found"})";
            }
        } catch (const std::exception& e) {
            resp.status_code = 400;
            json err;
            err["error"] = std::string("Invalid rule: ") + e.what();
            resp.body = err.dump();
        }
    }
    else if (ctx.method == "DELETE") {
        // DELETE /api/v1/events/rules/{id} - Delete rule
        if (rule_id.empty()) {
            resp.status_code = 400;
            resp.body = R"({"error":"Rule ID required"})";
            return resp;
        }
        
        if (manager.DeleteRule(rule_id)) {
            resp.status_code = 200;
            resp.body = R"({"success":true,"message":"Rule deleted"})";
        } else {
            resp.status_code = 404;
            resp.body = R"({"error":"Rule not found"})";
        }
    }
    else {
        resp.status_code = 405;
        resp.body = R"({"error":"Method not allowed"})";
    }
    
    return resp;
}

// ============================================================================
// GET /api/v1/events/stats - Event statistics
// ============================================================================
api::Response HandleEventsStats(const api::RequestContext& ctx) {
    api::Response resp;
    resp.headers["Content-Type"] = "application/json";
    
    if (ctx.method != "GET") {
        resp.status_code = 405;
        resp.body = R"({"error":"Method not allowed"})";
        return resp;
    }
    
    auto& manager = events::EventManager::Instance();
    auto stats = manager.GetStats();
    
    json response;
    response["events_published"] = stats.events_published;
    response["events_processed"] = stats.events_processed;
    response["events_filtered"] = stats.events_filtered;
    response["rules_matched"] = stats.rules_matched;
    response["actions_executed"] = stats.actions_executed;
    response["actions_failed"] = stats.actions_failed;
    
    // Events by type
    json by_type = json::object();
    for (const auto& [type, count] : stats.events_by_type) {
        by_type[events::EventTypeToString(type)] = count;
    }
    response["events_by_type"] = by_type;
    
    resp.status_code = 200;
    resp.body = response.dump(2);
    return resp;
}

// ============================================================================
// POST /api/v1/events/test - Trigger test event
// ============================================================================
api::Response HandleEventsTest(const api::RequestContext& ctx) {
    api::Response resp;
    resp.headers["Content-Type"] = "application/json";
    
    if (ctx.method != "POST") {
        resp.status_code = 405;
        resp.body = R"({"error":"Method not allowed"})";
        return resp;
    }
    
    try {
        json body = json::parse(ctx.body.empty() ? "{}" : ctx.body);
        
        std::string event_type_str = body.value("type", "motion_start");
        events::EventType type = events::StringToEventType(event_type_str);
        events::EventCategory cat = events::EventCategory::kMotion;
        
        // Determine category from type
        if (event_type_str.find("person") != std::string::npos ||
            event_type_str.find("vehicle") != std::string::npos) {
            cat = events::EventCategory::kAnalytics;
        } else if (event_type_str.find("line") != std::string::npos) {
            cat = events::EventCategory::kLineCrossing;
        } else if (event_type_str.find("zone") != std::string::npos) {
            cat = events::EventCategory::kIntrusion;
        }
        
        auto event = events::Event::Create(cat, type);
        event.source = "test_api";
        event.channel = body.value("channel", 0);
        
        auto& manager = events::EventManager::Instance();
        std::string event_id = manager.PublishEvent(event);
        
        json response;
        response["success"] = true;
        response["event_id"] = event.id;
        response["type"] = event_type_str;
        response["message"] = "Test event published";
        
        resp.status_code = 200;
        resp.body = response.dump(2);
        
    } catch (const std::exception& e) {
        resp.status_code = 400;
        json err;
        err["error"] = std::string("Invalid request: ") + e.what();
        resp.body = err.dump();
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
